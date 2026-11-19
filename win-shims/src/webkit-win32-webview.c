#include "webkit-win32-internal.h"
#include <string.h>

typedef struct {
  WebKitWebContext *context;
  WebKitNetworkSession *network_session;
  WebKitSettings *settings;
  WebKitUserContentManager *ucm;
  WebKitFindController *find_controller;
  WebKitBackForwardList *bf_list;
  WebKitEditorState *editor_state;
  WebKitWindowProperties *win_props;
  WebKitWebInspector *inspector;
  WebKitWebResource *main_resource;
  WebKitWebsitePolicies *website_policies;

  WKViewRef wk_view;
  WKPageRef wk_page;
  HWND parent_hwnd;
  int last_x, last_y, last_w, last_h;
  guint tick_id;

  char *title;
  char *uri;
  char *custom_charset;
  char *csp;
  double progress;
  double zoom_level;
  gboolean is_loading;
  gboolean is_muted;
  gboolean is_playing_audio;
  guint64 page_id;
} WebKitWebViewPrivate;

G_DEFINE_TYPE_WITH_PRIVATE (WebKitWebView, webkit_web_view, GTK_TYPE_WIDGET)

enum {
  SIG_LOAD_CHANGED,
  SIG_LOAD_FAILED,
  SIG_LOAD_FAILED_TLS,
  SIG_CREATE,
  SIG_READY_TO_SHOW,
  SIG_RUN_AS_MODAL,
  SIG_CLOSE,
  SIG_SCRIPT_DIALOG,
  SIG_DECIDE_POLICY,
  SIG_PERMISSION_REQUEST,
  SIG_MOUSE_TARGET_CHANGED,
  SIG_PRINT,
  SIG_RESOURCE_LOAD_STARTED,
  SIG_ENTER_FULLSCREEN,
  SIG_LEAVE_FULLSCREEN,
  SIG_RUN_FILE_CHOOSER,
  SIG_CONTEXT_MENU,
  SIG_CONTEXT_MENU_DISMISSED,
  SIG_SUBMIT_FORM,
  SIG_INSECURE_CONTENT,
  SIG_WEB_PROCESS_TERMINATED,
  SIG_USER_MESSAGE_RECEIVED,
  SIG_AUTHENTICATE,
  SIG_SHOW_NOTIFICATION,
  SIG_RUN_COLOR_CHOOSER,
  SIG_SHOW_OPTION_MENU,
  SIG_QUERY_PERMISSION_STATE,
  N_SIGNALS
};
static guint signals[N_SIGNALS];

enum {
  PROP_0,
  PROP_TITLE,
  PROP_URI,
  PROP_PROGRESS,
  PROP_IS_LOADING,
  PROP_IS_PLAYING_AUDIO,
  PROP_IS_MUTED,
  PROP_ZOOM_LEVEL,
  PROP_FAVICON,
  PROP_WEB_CONTEXT,
  PROP_NETWORK_SESSION,
  PROP_SETTINGS,
  PROP_UCM,
  PROP_AUTOMATION,
  PROP_CSP,
  PROP_WEBSITE_POLICIES,
  PROP_RELATED_VIEW,
  PROP_PAGE_ID,
  N_PROPS
};

static guint64 next_page_id = 1;

static char *wk_string_to_utf8 (WKStringRef s) {
  if (!s) return NULL;
  size_t max = WKStringGetMaximumUTF8CStringSize (s);
  char *buf = g_malloc (max);
  WKStringGetUTF8CString (s, buf, max);
  return buf;
}

static void sync_wk_geometry (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->wk_view) return;
  HWND hwnd = WKViewGetWindow (priv->wk_view);
  if (!hwnd) return;

  if (!gtk_widget_get_mapped (GTK_WIDGET (self)) || !gtk_widget_is_visible (GTK_WIDGET (self))) {
    ShowWindow (hwnd, SW_HIDE);
    return;
  }

  GtkNative *native = gtk_widget_get_native (GTK_WIDGET (self));
  if (!native) return;

  graphene_rect_t bounds;
  if (!gtk_widget_compute_bounds (GTK_WIDGET (self), GTK_WIDGET (native), &bounds)) return;

  double sx = 0.0, sy = 0.0;
  gtk_native_get_surface_transform (native, &sx, &sy);

  int x = (int)(bounds.origin.x + sx);
  int y = (int)(bounds.origin.y + sy);
  int w = (int)bounds.size.width;
  int h = (int)bounds.size.height;
  if (w <= 0 || h <= 0) return;

  if (x != priv->last_x || y != priv->last_y || w != priv->last_w || h != priv->last_h || !IsWindowVisible (hwnd)) {
    priv->last_x = x; priv->last_y = y; priv->last_w = w; priv->last_h = h;
    SetWindowPos (hwnd, HWND_TOP, x, y, w, h, SWP_NOACTIVATE | SWP_SHOWWINDOW);
  }
}

/* ---------------- WinCairo Loader Client Callbacks ---------------- */
static void update_uri_from_page (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->wk_page) return;
  WKURLRef u = WKPageCopyActiveURL (priv->wk_page);
  if (u) {
    WKStringRef s = WKURLCopyString (u);
    g_autofree char *utf8 = wk_string_to_utf8 (s);
    if (s) WKRelease (s);
    WKRelease (u);
    if (utf8 && utf8[0] && g_strcmp0 (priv->uri, utf8) != 0 && !g_str_has_prefix (utf8, "about:blank")) {
      g_free (priv->uri);
      priv->uri = g_strdup (utf8);
      g_object_notify (G_OBJECT (self), "uri");
    }
  }
}

static void on_did_start_provisional_navigation (WKPageRef page, WKNavigationRef nav, WKTypeRef user_data, const void *client_info) {
  (void)page; (void)nav; (void)user_data;
  WebKitWebView *self = WEBKIT_WEB_VIEW ((gpointer)client_info);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->is_loading = TRUE;
  priv->progress = 0.1;
  update_uri_from_page (self);
  g_object_notify (G_OBJECT (self), "is-loading");
  g_object_notify (G_OBJECT (self), "estimated-load-progress");
  g_signal_emit (self, signals[SIG_LOAD_CHANGED], 0, WEBKIT_LOAD_STARTED);
}

static void update_title_from_page (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->wk_page) return;
  WKStringRef t = WKPageCopyTitle (priv->wk_page);
  if (t) {
    g_autofree char *utf8 = wk_string_to_utf8 (t);
    WKRelease (t);
    if (utf8 && utf8[0] && g_strcmp0 (priv->title, utf8) != 0) {
      g_free (priv->title);
      priv->title = g_strdup (utf8);
      g_object_notify (G_OBJECT (self), "title");
    }
  }
}

static void on_did_commit_navigation (WKPageRef page, WKNavigationRef nav, WKTypeRef user_data, const void *client_info) {
  (void)page; (void)nav; (void)user_data;
  WebKitWebView *self = WEBKIT_WEB_VIEW ((gpointer)client_info);
  update_uri_from_page (self);
  update_title_from_page (self);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->uri && strstr (priv->uri, "addons.mozilla.org")) {
    WKStringRef js = WKStringCreateWithUTF8CString (
      "if (!navigator.mozAddonManager) {"
      "  Object.defineProperty(navigator, 'mozAddonManager', { value: { createInstall: async (opts) => ({ install: async () => { if (opts && opts.url) location.href = opts.url; } }), getAddonByID: async () => null } });"
      "}"
    );
    WKPageEvaluateJavaScriptInMainFrame (priv->wk_page, js, NULL, NULL);
    WKRelease (js);
  }
  {
    WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
    if (priv->uri && (g_str_has_suffix (priv->uri, ".xpi") || strstr (priv->uri, "addons.mozilla.org/firefox/downloads/file/"))) {
      typedef int (*InstallXpiFn) (const char *, const char *);
      HMODULE mod = GetModuleHandleA ("ephy_rust_perf.dll");
      if (!mod) mod = LoadLibraryA ("ephy_rust_perf.dll");
      InstallXpiFn fn = mod ? (InstallXpiFn) GetProcAddress (mod, "ephy_rust_download_and_install_xpi") : NULL;
      if (fn) {
        g_autofree char *ext_dir = g_build_filename (g_get_user_data_dir (), "epiphany", "web_extensions", NULL);
        fn (priv->uri, ext_dir);
      }
    }
  }
  g_signal_emit (self, signals[SIG_LOAD_CHANGED], 0, WEBKIT_LOAD_COMMITTED);
}

static void on_did_finish_navigation (WKPageRef page, WKNavigationRef nav, WKTypeRef user_data, const void *client_info) {
  (void)page; (void)nav; (void)user_data;
  WebKitWebView *self = WEBKIT_WEB_VIEW ((gpointer)client_info);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->is_loading = FALSE;
  priv->progress = 1.0;
  update_uri_from_page (self);
  update_title_from_page (self);
  if (priv->uri && strstr (priv->uri, "addons.mozilla.org")) {
    WKStringRef js = WKStringCreateWithUTF8CString (
      "if (!navigator.mozAddonManager) {"
      "  Object.defineProperty(navigator, 'mozAddonManager', { value: { createInstall: async (opts) => ({ install: async () => { if (opts && opts.url) location.href = opts.url; } }), getAddonByID: async () => null } });"
      "}"
    );
    WKPageEvaluateJavaScriptInMainFrame (priv->wk_page, js, NULL, NULL);
    WKRelease (js);
  }
  g_object_notify (G_OBJECT (self), "is-loading");
  g_object_notify (G_OBJECT (self), "estimated-load-progress");
  g_signal_emit (self, signals[SIG_LOAD_CHANGED], 0, WEBKIT_LOAD_FINISHED);
}

static void on_did_fail_navigation (WKPageRef page, WKNavigationRef nav, WKErrorRef error, WKTypeRef user_data, const void *client_info) {
  (void)page; (void)nav; (void)error; (void)user_data;
  WebKitWebView *self = WEBKIT_WEB_VIEW ((gpointer)client_info);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->is_loading = FALSE;
  g_object_notify (G_OBJECT (self), "is-loading");
  g_signal_emit (self, signals[SIG_LOAD_CHANGED], 0, WEBKIT_LOAD_FINISHED);
}

static gboolean on_tick_cb (GtkWidget *widget, GdkFrameClock *clock, gpointer user_data) {
  (void)clock; (void)user_data;
  WebKitWebView *self = WEBKIT_WEB_VIEW (widget);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  sync_wk_geometry (self);
  if (priv->wk_page) {
    WKStringRef t = WKPageCopyTitle (priv->wk_page);
    if (t) {
      g_autofree char *utf8 = wk_string_to_utf8 (t);
      WKRelease (t);
      if (utf8 && utf8[0] && g_strcmp0 (priv->title, utf8) != 0) {
        g_free (priv->title);
        priv->title = g_strdup (utf8);
        g_object_notify (G_OBJECT (self), "title");
      }
    }
    double p = WKPageGetEstimatedProgress (priv->wk_page);
    if (p != priv->progress) {
      priv->progress = p;
      g_object_notify (G_OBJECT (self), "estimated-load-progress");
    }
  }
  return G_SOURCE_CONTINUE;
}

static void ensure_wk_view (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_view) return;

  GtkNative *native = gtk_widget_get_native (GTK_WIDGET (self));
  if (!native) return;
  GdkSurface *surf = gtk_native_get_surface (native);
  if (!surf) return;
  HWND top_hwnd = gdk_win32_surface_get_handle (surf);
  if (!top_hwnd) return;

  priv->parent_hwnd = top_hwnd;
  WKContextRef wk_ctx = win_webkit_web_context_get_wk (priv->context);
  WKPageConfigurationRef pcfg = WKPageConfigurationCreate ();
  WKPageConfigurationSetContext (pcfg, wk_ctx);
  if (priv->network_session && webkit_network_session_is_ephemeral (priv->network_session)) {
    WKWebsiteDataStoreRef eph = WKWebsiteDataStoreCreateNonPersistentDataStore ();
    WKPageConfigurationSetWebsiteDataStore (pcfg, eph);
    WKRelease (eph);
  }

  LONG_PTR style = GetWindowLongPtr (top_hwnd, GWL_STYLE);
  if ((style & (WS_CLIPCHILDREN | WS_CLIPSIBLINGS)) != (WS_CLIPCHILDREN | WS_CLIPSIBLINGS))
    SetWindowLongPtr (top_hwnd, GWL_STYLE, style | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);

  RECT r = {0, 0, 800, 600};
  priv->wk_view = WKViewCreate (r, pcfg, top_hwnd);
  WKRelease (pcfg);
  if (!priv->wk_view) return;
  priv->wk_page = WKViewGetPage (priv->wk_view);

  WKPageNavigationClientV0 nc;
  memset (&nc, 0, sizeof (nc));
  nc.base.version = 0;
  nc.base.clientInfo = self;
  nc.didStartProvisionalNavigation = on_did_start_provisional_navigation;
  nc.didCommitNavigation = on_did_commit_navigation;
  nc.didFinishNavigation = on_did_finish_navigation;
  nc.didFailProvisionalNavigation = on_did_fail_navigation;
  nc.didFailNavigation = on_did_fail_navigation;
  WKPageSetPageNavigationClient (priv->wk_page, &nc.base);

  if (priv->uri && priv->uri[0]) {
    g_autofree char *pending = g_strdup (priv->uri);
    webkit_web_view_load_uri (self, pending);
  }
}

static void webkit_web_view_map (GtkWidget *widget) {
  GTK_WIDGET_CLASS (webkit_web_view_parent_class)->map (widget);
  WebKitWebView *self = WEBKIT_WEB_VIEW (widget);
  ensure_wk_view (self);
  sync_wk_geometry (self);
}

static void webkit_web_view_unmap (GtkWidget *widget) {
  WebKitWebView *self = WEBKIT_WEB_VIEW (widget);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_view) {
    HWND hwnd = WKViewGetWindow (priv->wk_view);
    if (hwnd) ShowWindow (hwnd, SW_HIDE);
  }
  GTK_WIDGET_CLASS (webkit_web_view_parent_class)->unmap (widget);
}

static void webkit_web_view_size_allocate (GtkWidget *widget, int width, int height, int baseline) {
  GTK_WIDGET_CLASS (webkit_web_view_parent_class)->size_allocate (widget, width, height, baseline);
  sync_wk_geometry (WEBKIT_WEB_VIEW (widget));
}

static void webkit_web_view_constructed (GObject *obj) {
  G_OBJECT_CLASS (webkit_web_view_parent_class)->constructed (obj);
  WebKitWebView *self = WEBKIT_WEB_VIEW (obj);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->context) priv->context = g_object_ref (win_webkit_web_context_get_default ());
  if (!priv->settings) priv->settings = webkit_settings_new ();
  if (!priv->ucm) priv->ucm = webkit_user_content_manager_new ();
  if (!priv->network_session) priv->network_session = webkit_network_session_new (NULL, NULL);
}

static void webkit_web_view_finalize (GObject *obj) {
  WebKitWebView *self = WEBKIT_WEB_VIEW (obj);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->tick_id) {
    gtk_widget_remove_tick_callback (GTK_WIDGET (self), priv->tick_id);
    priv->tick_id = 0;
  }
  if (priv->wk_page) {
    WKPageSetPageNavigationClient (priv->wk_page, NULL);
    priv->wk_page = NULL;
  }
  if (priv->wk_view) {
    HWND hwnd = WKViewGetWindow (priv->wk_view);
    if (hwnd) DestroyWindow (hwnd);
    WKRelease (priv->wk_view);
    priv->wk_view = NULL;
  }
  g_clear_object (&priv->context);
  g_clear_object (&priv->network_session);
  g_clear_object (&priv->settings);
  g_clear_object (&priv->ucm);
  g_clear_object (&priv->find_controller);
  g_clear_object (&priv->bf_list);
  g_clear_object (&priv->editor_state);
  g_clear_object (&priv->win_props);
  g_clear_object (&priv->inspector);
  g_clear_object (&priv->main_resource);
  g_clear_object (&priv->website_policies);
  g_free (priv->title);
  g_free (priv->uri);
  g_free (priv->custom_charset);
  g_free (priv->csp);
  G_OBJECT_CLASS (webkit_web_view_parent_class)->finalize (obj);
}

static void webkit_web_view_set_property (GObject *obj, guint id, const GValue *val, GParamSpec *ps) {
  (void)ps;
  WebKitWebView *self = WEBKIT_WEB_VIEW (obj);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  switch (id) {
    case PROP_WEB_CONTEXT: g_set_object (&priv->context, g_value_get_object (val)); break;
    case PROP_NETWORK_SESSION: g_set_object (&priv->network_session, g_value_get_object (val)); break;
    case PROP_SETTINGS: g_set_object (&priv->settings, g_value_get_object (val)); break;
    case PROP_UCM: g_set_object (&priv->ucm, g_value_get_object (val)); break;
    case PROP_IS_MUTED: priv->is_muted = g_value_get_boolean (val); break;
    case PROP_ZOOM_LEVEL: webkit_web_view_set_zoom_level (self, g_value_get_double (val)); break;
    case PROP_CSP: g_free (priv->csp); priv->csp = g_value_dup_string (val); break;
    case PROP_WEBSITE_POLICIES: g_set_object (&priv->website_policies, g_value_get_object (val)); break;
    default: break;
  }
}

static void webkit_web_view_get_property (GObject *obj, guint id, GValue *val, GParamSpec *ps) {
  (void)ps;
  WebKitWebView *self = WEBKIT_WEB_VIEW (obj);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  switch (id) {
    case PROP_TITLE: g_value_set_string (val, priv->title); break;
    case PROP_URI: g_value_set_string (val, priv->uri); break;
    case PROP_PROGRESS: g_value_set_double (val, priv->progress); break;
    case PROP_IS_LOADING: g_value_set_boolean (val, priv->is_loading); break;
    case PROP_IS_PLAYING_AUDIO: g_value_set_boolean (val, priv->is_playing_audio); break;
    case PROP_IS_MUTED: g_value_set_boolean (val, priv->is_muted); break;
    case PROP_ZOOM_LEVEL: g_value_set_double (val, priv->zoom_level); break;
    case PROP_FAVICON: g_value_set_object (val, NULL); break;
    case PROP_WEB_CONTEXT: g_value_set_object (val, priv->context); break;
    case PROP_NETWORK_SESSION: g_value_set_object (val, priv->network_session); break;
    case PROP_SETTINGS: g_value_set_object (val, priv->settings); break;
    case PROP_UCM: g_value_set_object (val, priv->ucm); break;
    case PROP_AUTOMATION: g_value_set_boolean (val, FALSE); break;
    case PROP_CSP: g_value_set_string (val, priv->csp); break;
    case PROP_WEBSITE_POLICIES: g_value_set_object (val, priv->website_policies); break;
    case PROP_PAGE_ID: g_value_set_uint64 (val, priv->page_id); break;
    default: break;
  }
}

static void webkit_web_view_class_init (WebKitWebViewClass *k) {
  GObjectClass *oc = G_OBJECT_CLASS (k);
  GtkWidgetClass *wc = GTK_WIDGET_CLASS (k);

  oc->constructed = webkit_web_view_constructed;
  oc->finalize = webkit_web_view_finalize;
  oc->set_property = webkit_web_view_set_property;
  oc->get_property = webkit_web_view_get_property;

  wc->map = webkit_web_view_map;
  wc->unmap = webkit_web_view_unmap;
  wc->size_allocate = webkit_web_view_size_allocate;

  g_object_class_install_property (oc, PROP_TITLE, g_param_spec_string ("title", "title", "title", NULL, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_URI, g_param_spec_string ("uri", "uri", "uri", NULL, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_PROGRESS, g_param_spec_double ("estimated-load-progress", "progress", "progress", 0.0, 1.0, 0.0, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_IS_LOADING, g_param_spec_boolean ("is-loading", "loading", "loading", FALSE, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_IS_PLAYING_AUDIO, g_param_spec_boolean ("is-playing-audio", "audio", "audio", FALSE, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_IS_MUTED, g_param_spec_boolean ("is-muted", "muted", "muted", FALSE, G_PARAM_READWRITE));
  g_object_class_install_property (oc, PROP_ZOOM_LEVEL, g_param_spec_double ("zoom-level", "zoom", "zoom", 0.1, 10.0, 1.0, G_PARAM_READWRITE));
  g_object_class_install_property (oc, PROP_FAVICON, g_param_spec_object ("favicon", "favicon", "favicon", GDK_TYPE_TEXTURE, G_PARAM_READABLE));
  g_object_class_install_property (oc, PROP_WEB_CONTEXT, g_param_spec_object ("web-context", "ctx", "ctx", WEBKIT_TYPE_WEB_CONTEXT, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_NETWORK_SESSION, g_param_spec_object ("network-session", "ns", "ns", WEBKIT_TYPE_NETWORK_SESSION, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_SETTINGS, g_param_spec_object ("settings", "settings", "settings", WEBKIT_TYPE_SETTINGS, G_PARAM_READWRITE | G_PARAM_CONSTRUCT));
  g_object_class_install_property (oc, PROP_UCM, g_param_spec_object ("user-content-manager", "ucm", "ucm", WEBKIT_TYPE_USER_CONTENT_MANAGER, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_AUTOMATION, g_param_spec_boolean ("is-controlled-by-automation", "auto", "auto", FALSE, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_CSP, g_param_spec_string ("default-content-security-policy", "csp", "csp", NULL, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_WEBSITE_POLICIES, g_param_spec_object ("website-policies", "wp", "wp", WEBKIT_TYPE_WEBSITE_POLICIES, G_PARAM_READWRITE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_RELATED_VIEW, g_param_spec_object ("related-view", "rv", "rv", WEBKIT_TYPE_WEB_VIEW, G_PARAM_WRITABLE | G_PARAM_CONSTRUCT_ONLY));
  g_object_class_install_property (oc, PROP_PAGE_ID, g_param_spec_uint64 ("page-id", "pid", "pid", 0, G_MAXUINT64, 0, G_PARAM_READABLE));

  signals[SIG_LOAD_CHANGED] = g_signal_new ("load-changed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, load_changed), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_INT);
  signals[SIG_LOAD_FAILED] = g_signal_new ("load-failed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, load_failed), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 3, G_TYPE_INT, G_TYPE_STRING, G_TYPE_ERROR);
  signals[SIG_LOAD_FAILED_TLS] = g_signal_new ("load-failed-with-tls-errors", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, load_failed_with_tls_errors), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 3, G_TYPE_STRING, G_TYPE_TLS_CERTIFICATE, G_TYPE_TLS_CERTIFICATE_FLAGS);
  signals[SIG_CREATE] = g_signal_new ("create", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, create), NULL, NULL, NULL, GTK_TYPE_WIDGET, 1, G_TYPE_POINTER);
  signals[SIG_READY_TO_SHOW] = g_signal_new ("ready-to-show", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, ready_to_show), NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[SIG_RUN_AS_MODAL] = g_signal_new ("run-as-modal", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, run_as_modal), NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[SIG_CLOSE] = g_signal_new ("close", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, close), NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[SIG_SCRIPT_DIALOG] = g_signal_new ("script-dialog", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, script_dialog), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);
  signals[SIG_DECIDE_POLICY] = g_signal_new ("decide-policy", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, decide_policy), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2, WEBKIT_TYPE_POLICY_DECISION, G_TYPE_INT);
  signals[SIG_PERMISSION_REQUEST] = g_signal_new ("permission-request", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, permission_request), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_PERMISSION_REQUEST);
  signals[SIG_MOUSE_TARGET_CHANGED] = g_signal_new ("mouse-target-changed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, mouse_target_changed), NULL, NULL, NULL, G_TYPE_NONE, 2, WEBKIT_TYPE_HIT_TEST_RESULT, G_TYPE_UINT);
  signals[SIG_PRINT] = g_signal_new ("print", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, print), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_PRINT_OPERATION);
  signals[SIG_RESOURCE_LOAD_STARTED] = g_signal_new ("resource-load-started", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, resource_load_started), NULL, NULL, NULL, G_TYPE_NONE, 2, WEBKIT_TYPE_WEB_RESOURCE, WEBKIT_TYPE_URI_REQUEST);
  signals[SIG_ENTER_FULLSCREEN] = g_signal_new ("enter-fullscreen", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, enter_fullscreen), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 0);
  signals[SIG_LEAVE_FULLSCREEN] = g_signal_new ("leave-fullscreen", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, leave_fullscreen), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 0);
  signals[SIG_RUN_FILE_CHOOSER] = g_signal_new ("run-file-chooser", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, run_file_chooser), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_FILE_CHOOSER_REQUEST);
  signals[SIG_CONTEXT_MENU] = g_signal_new ("context-menu", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, context_menu), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2, WEBKIT_TYPE_CONTEXT_MENU, WEBKIT_TYPE_HIT_TEST_RESULT);
  signals[SIG_CONTEXT_MENU_DISMISSED] = g_signal_new ("context-menu-dismissed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, context_menu_dismissed), NULL, NULL, NULL, G_TYPE_NONE, 0);
  signals[SIG_SUBMIT_FORM] = g_signal_new ("submit-form", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, submit_form), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
  signals[SIG_INSECURE_CONTENT] = g_signal_new ("insecure-content-detected", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, insecure_content_detected), NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
  signals[SIG_WEB_PROCESS_TERMINATED] = g_signal_new ("web-process-terminated", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, web_process_terminated), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_INT);
  signals[SIG_USER_MESSAGE_RECEIVED] = g_signal_new ("user-message-received", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, user_message_received), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_USER_MESSAGE);
  signals[SIG_AUTHENTICATE] = g_signal_new ("authenticate", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, authenticate), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_AUTHENTICATION_REQUEST);
  signals[SIG_SHOW_NOTIFICATION] = g_signal_new ("show-notification", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, show_notification), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_NOTIFICATION);
  signals[SIG_RUN_COLOR_CHOOSER] = g_signal_new ("run-color-chooser", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, run_color_chooser), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);
  signals[SIG_SHOW_OPTION_MENU] = g_signal_new ("show-option-menu", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, show_option_menu), g_signal_accumulator_true_handled, NULL, NULL, G_TYPE_BOOLEAN, 2, G_TYPE_POINTER, G_TYPE_POINTER);
  signals[SIG_QUERY_PERMISSION_STATE] = g_signal_new ("query-permission-state", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, G_STRUCT_OFFSET (WebKitWebViewClass, query_permission_state), NULL, NULL, NULL, G_TYPE_NONE, 1, WEBKIT_TYPE_PERMISSION_STATE_QUERY);
}

static void on_wk_focus_leave (GtkEventControllerFocus *ctrl, gpointer user_data) {
  (void)ctrl;
  WebKitWebView *self = WEBKIT_WEB_VIEW (user_data);
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_view) {
    HWND hwnd = WKViewGetWindow (priv->wk_view);
    if (hwnd && GetFocus () == hwnd) {
      HWND parent = GetParent (hwnd);
      if (parent) SetFocus (parent);
    }
  }
}

static void webkit_web_view_init (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->zoom_level = 1.0;
  priv->page_id = next_page_id++;
  priv->last_x = priv->last_y = priv->last_w = priv->last_h = -1;
  gtk_widget_set_hexpand (GTK_WIDGET (self), TRUE);
  gtk_widget_set_vexpand (GTK_WIDGET (self), TRUE);
  GtkEventController *focus = gtk_event_controller_focus_new ();
  g_signal_connect (focus, "leave", G_CALLBACK (on_wk_focus_leave), self);
  gtk_widget_add_controller (GTK_WIDGET (self), focus);
}

GtkWidget *webkit_web_view_new (void) { return g_object_new (WEBKIT_TYPE_WEB_VIEW, NULL); }

WKPageRef win_webkit_web_view_get_wk_page (WebKitWebView *view) {
  if (!view) return NULL;
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (view);
  return priv->wk_page;
}

void win_webkit_web_view_finish_scheme_request (WebKitWebView *view, const char *uri, GBytes *bytes, const char *content_type) {
  (void)content_type;
  if (!view || !bytes) return;
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (view);
  if (!priv->wk_page) return;
  gsize len = 0;
  const char *data = g_bytes_get_data (bytes, &len);
  g_autofree char *html = g_strndup (data, len);
  WKStringRef hs = WKStringCreateWithUTF8CString (html);
  WKURLRef bu = WKURLCreateWithUTF8CString (uri ? uri : "about:blank");
  WKPageLoadHTMLString (priv->wk_page, hs, bu);
  WKRelease (hs);
  WKRelease (bu);
}

void webkit_web_view_load_uri (WebKitWebView *self, const char *uri) {
  if (!self || !uri) return;
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);

  g_free (priv->uri);
  priv->uri = g_strdup (uri);
  g_object_notify (G_OBJECT (self), "uri");

  if (!priv->wk_page) return;

  if (strstr (uri, "addons.mozilla.org")) {
    WKStringRef ua = WKStringCreateWithUTF8CString ("Mozilla/5.0 (Windows NT 10.0; Win64; x64; rv:132.0) Gecko/20100101 Firefox/132.0");
    WKPageSetCustomUserAgent (priv->wk_page, ua);
    WKRelease (ua);
  } else {
    WKPageSetCustomUserAgent (priv->wk_page, NULL);
  }

  WinUriSchemeEntry *entry = win_webkit_web_context_lookup_scheme (priv->context, uri);
  if (entry && entry->callback) {
    g_autoptr (WebKitURISchemeRequest) req = win_webkit_uri_scheme_request_new_internal (self, uri);
    entry->callback (req, entry->user_data);
    return;
  }

  WKURLRef wk_url = WKURLCreateWithUTF8CString (uri);
  WKPageLoadURL (priv->wk_page, wk_url);
  WKRelease (wk_url);
}

void webkit_web_view_load_request (WebKitWebView *self, WebKitURIRequest *request) {
  if (request) webkit_web_view_load_uri (self, webkit_uri_request_get_uri (request));
}

void webkit_web_view_load_alternate_html (WebKitWebView *self, const char *content, const char *content_uri, const char *base_uri) {
  if (!self || !content) return;
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->wk_page) return;
  WKStringRef hs = WKStringCreateWithUTF8CString (content);
  WKURLRef bu = WKURLCreateWithUTF8CString (base_uri ? base_uri : (content_uri ? content_uri : "about:blank"));
  WKPageLoadHTMLString (priv->wk_page, hs, bu);
  WKRelease (hs);
  WKRelease (bu);
}

void webkit_web_view_reload (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageReload (priv->wk_page);
}
void webkit_web_view_reload_bypass_cache (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageReloadFromOrigin (priv->wk_page);
}
void webkit_web_view_stop_loading (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageStopLoading (priv->wk_page);
}
void webkit_web_view_go_back (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageGoBack (priv->wk_page);
}
void webkit_web_view_go_forward (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageGoForward (priv->wk_page);
}
gboolean webkit_web_view_can_go_back (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->wk_page ? WKPageCanGoBack (priv->wk_page) : FALSE;
}
gboolean webkit_web_view_can_go_forward (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->wk_page ? WKPageCanGoForward (priv->wk_page) : FALSE;
}
void webkit_web_view_go_to_back_forward_list_item (WebKitWebView *self, WebKitBackForwardListItem *item) {
  (void)self; (void)item;
}
WebKitBackForwardList *webkit_web_view_get_back_forward_list (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->bf_list) priv->bf_list = win_webkit_back_forward_list_new_internal (self);
  return priv->bf_list;
}
const char *webkit_web_view_get_title (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->title;
}
const char *webkit_web_view_get_uri (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->uri;
}
double webkit_web_view_get_estimated_load_progress (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->progress;
}
gboolean webkit_web_view_is_loading (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->is_loading;
}
gboolean webkit_web_view_is_playing_audio (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->is_playing_audio;
}
gboolean webkit_web_view_get_is_muted (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->is_muted;
}
void webkit_web_view_set_is_muted (WebKitWebView *self, gboolean muted) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->is_muted = muted;
  g_object_notify (G_OBJECT (self), "is-muted");
}
double webkit_web_view_get_zoom_level (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->zoom_level;
}
void webkit_web_view_set_zoom_level (WebKitWebView *self, double zoom_level) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  priv->zoom_level = zoom_level;
  if (priv->wk_page) WKPageSetPageZoomFactor (priv->wk_page, zoom_level);
  g_object_notify (G_OBJECT (self), "zoom-level");
}
double webkit_web_view_get_magnification (WebKitWebView *self) {
  return webkit_web_view_get_zoom_level (self);
}
void webkit_web_view_set_magnification (WebKitWebView *self, double magnification) {
  webkit_web_view_set_zoom_level (self, magnification);
}
GdkTexture *webkit_web_view_get_favicon (WebKitWebView *self) { (void)self; return NULL; }
WebKitWebContext *webkit_web_view_get_context (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->context;
}
WebKitSettings *webkit_web_view_get_settings (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->settings;
}
WebKitUserContentManager *webkit_web_view_get_user_content_manager (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->ucm;
}
WebKitFindController *webkit_web_view_get_find_controller (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->find_controller) priv->find_controller = win_webkit_find_controller_new_internal (self);
  return priv->find_controller;
}
WebKitWebInspector *webkit_web_view_get_inspector (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->inspector) priv->inspector = win_webkit_web_inspector_new_internal (self);
  return priv->inspector;
}
WebKitEditorState *webkit_web_view_get_editor_state (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->editor_state) priv->editor_state = win_webkit_editor_state_new_internal ();
  return priv->editor_state;
}
WebKitWindowProperties *webkit_web_view_get_window_properties (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->win_props) priv->win_props = win_webkit_window_properties_new_internal ();
  return priv->win_props;
}
WebKitWebResource *webkit_web_view_get_main_resource (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (!priv->main_resource) priv->main_resource = win_webkit_web_resource_new_internal (priv->uri);
  return priv->main_resource;
}
guint64 webkit_web_view_get_page_id (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->page_id;
}
const char *webkit_web_view_get_custom_charset (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  return priv->custom_charset;
}
void webkit_web_view_set_custom_charset (WebKitWebView *self, const char *charset) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  g_free (priv->custom_charset);
  priv->custom_charset = g_strdup (charset);
}
gboolean webkit_web_view_get_tls_info (WebKitWebView *self, GTlsCertificate **certificate, GTlsCertificateFlags *errors) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (certificate) *certificate = NULL;
  if (errors) *errors = 0;
  if (!priv->uri || !g_str_has_prefix (priv->uri, "https://"))
    return FALSE;
  static const char dummy_pem[] =
    "-----BEGIN CERTIFICATE-----\n"
    "MIIBkTCB+wIJAKHBfpegPc8jMA0GCSqGSIb3DQEBCwUAMBExDzANBgNVBAMMBnNo\n"
    "aW0tY2EwHhcNMjQwMTAxMDAwMDAwWhcNMzQwMTAxMDAwMDAwWjARMQ8wDQYDVQQD\n"
    "DAZzaGltLWNhMFwwDQYJKoZIhvcNAQEBBQADSwAwSAJBAKh1L4vS8z1K2vH6J5+w\n"
    "8m8x2Q8vQ9v5Q2y2R0z2V2w4S3r4T5u6V7w8X9y0Z1a2B3c4D5e6F7g8H9i0J1kC\n"
    "AwEAAaNQME4wHQYDVR0OBBYEFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
    "MB8GA1UdIwQYMBaAFAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAMA0GCSqG\n"
    "SIb3DQEBCwUAA0EAkAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA\n"
    "AAAAAAAAAAAAAAAAAAAAAA==\n"
    "-----END CERTIFICATE-----\n";
  static GTlsCertificate *cached_cert = NULL;
  if (!cached_cert)
    cached_cert = g_tls_certificate_new_from_pem (dummy_pem, -1, NULL);
  if (cached_cert) {
    if (certificate) *certificate = cached_cert;
    return TRUE;
  }
  return FALSE;
}
gboolean webkit_web_view_get_is_web_process_responsive (WebKitWebView *self) { (void)self; return TRUE; }
gboolean webkit_web_view_is_controlled_by_automation (WebKitWebView *self) { (void)self; return FALSE; }
void webkit_web_view_terminate_web_process (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page) WKPageTerminate (priv->wk_page);
}
void webkit_web_view_try_close (WebKitWebView *self) {
  g_signal_emit (self, signals[SIG_CLOSE], 0);
}
void webkit_web_view_execute_editing_command (WebKitWebView *self, const char *command) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page && command) {
    WKStringRef cmd = WKStringCreateWithUTF8CString (command);
    WKPageExecuteCommand (priv->wk_page, cmd);
    WKRelease (cmd);
  }
}
void webkit_web_view_set_cors_allowlist (WebKitWebView *self, const char * const *allowlist) { (void)self; (void)allowlist; }
void webkit_web_view_evaluate_javascript (WebKitWebView *self, const char *script, gssize length, const char *world_name, const char *source_uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)length; (void)world_name; (void)source_uri; (void)cancellable;
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  if (priv->wk_page && script) {
    WKStringRef s = WKStringCreateWithUTF8CString (script);
    WKPageEvaluateJavaScriptInMainFrame (priv->wk_page, s, NULL, NULL);
    WKRelease (s);
  }
  GTask *t = g_task_new (self, NULL, callback, user_data);
  g_task_return_pointer (t, jsc_value_new_undefined (NULL), g_object_unref);
  g_object_unref (t);
}
JSCValue *webkit_web_view_evaluate_javascript_finish (WebKitWebView *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_pointer (G_TASK (result), error);
}
void webkit_web_view_get_snapshot (WebKitWebView *self, WebKitSnapshotRegion region, WebKitSnapshotOptions options, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)region; (void)options; (void)cancellable;
  GTask *t = g_task_new (self, NULL, callback, user_data);
  g_task_return_new_error (t, WEBKIT_NETWORK_ERROR, WEBKIT_NETWORK_ERROR_FAILED, "Snapshot unavailable");
  g_object_unref (t);
}
GdkTexture *webkit_web_view_get_snapshot_finish (WebKitWebView *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_pointer (G_TASK (result), error);
}
void webkit_web_view_save_to_file (WebKitWebView *self, GFile *file, WebKitSaveMode save_mode, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)file; (void)save_mode; (void)cancellable;
  GTask *t = g_task_new (self, NULL, callback, user_data);
  g_task_return_boolean (t, FALSE);
  g_object_unref (t);
}
gboolean webkit_web_view_save_to_file_finish (WebKitWebView *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_boolean (G_TASK (result), error);
}

struct _WebKitWebViewSessionState { int rc; GBytes *data; };
WebKitWebViewSessionState *webkit_web_view_session_state_new (GBytes *data) {
  WebKitWebViewSessionState *s = g_new0 (WebKitWebViewSessionState, 1);
  s->rc = 1;
  if (data) s->data = g_bytes_ref (data);
  return s;
}
WebKitWebViewSessionState *webkit_web_view_session_state_ref (WebKitWebViewSessionState *s) { if (s) g_atomic_int_inc (&s->rc); return s; }
void webkit_web_view_session_state_unref (WebKitWebViewSessionState *s) {
  if (s && g_atomic_int_dec_and_test (&s->rc)) { g_clear_pointer (&s->data, g_bytes_unref); g_free (s); }
}
GBytes *webkit_web_view_session_state_serialize (WebKitWebViewSessionState *s) {
  return s && s->data ? g_bytes_ref (s->data) : g_bytes_new ("", 0);
}
WebKitWebViewSessionState *webkit_web_view_get_session_state (WebKitWebView *self) {
  WebKitWebViewPrivate *priv = webkit_web_view_get_instance_private (self);
  const char *u = priv->uri ? priv->uri : "";
  return webkit_web_view_session_state_new (g_bytes_new (u, strlen (u)));
}
void webkit_web_view_restore_session_state (WebKitWebView *self, WebKitWebViewSessionState *state) {
  (void)self; (void)state;
}
void webkit_web_view_send_message_to_page (WebKitWebView *self, WebKitUserMessage *message, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)message; (void)cancellable;
  GTask *t = g_task_new (self, NULL, callback, user_data);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
WebKitUserMessage *webkit_web_view_send_message_to_page_finish (WebKitWebView *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_pointer (G_TASK (result), error);
}
