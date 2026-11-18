#include "webkit-win32-internal.h"
#include <stdarg.h>
#include <string.h>

/* ---------------- Version & Error Quarks ---------------- */
guint webkit_get_major_version (void) { return WEBKIT_MAJOR_VERSION; }
guint webkit_get_minor_version (void) { return WEBKIT_MINOR_VERSION; }
guint webkit_get_micro_version (void) { return WEBKIT_MICRO_VERSION; }

GQuark webkit_network_error_quark (void) { return g_quark_from_static_string ("webkit-network-error"); }
GQuark webkit_policy_error_quark (void) { return g_quark_from_static_string ("webkit-policy-error"); }
GQuark webkit_download_error_quark (void) { return g_quark_from_static_string ("webkit-download-error"); }
GQuark webkit_favicon_database_error_quark (void) { return g_quark_from_static_string ("webkit-favicon-database-error"); }
GQuark webkit_user_content_filter_error_quark (void) { return g_quark_from_static_string ("webkit-user-content-filter-error"); }
GQuark webkit_user_message_error_quark (void) { return g_quark_from_static_string ("webkit-user-message-error"); }
GQuark webkit_media_error_quark (void) { return g_quark_from_static_string ("webkit-media-error"); }

char *webkit_uri_for_display (const char *uri) {
  return g_strdup (uri);
}

/* ---------------- WebKitSecurityManager ---------------- */
struct _WebKitSecurityManager {
  GObject parent;
  GHashTable *local_schemes;
  GHashTable *secure_schemes;
  GHashTable *display_isolated;
};
G_DEFINE_TYPE (WebKitSecurityManager, webkit_security_manager, G_TYPE_OBJECT)
static void webkit_security_manager_finalize (GObject *o) {
  WebKitSecurityManager *s = WEBKIT_SECURITY_MANAGER (o);
  g_hash_table_destroy (s->local_schemes);
  g_hash_table_destroy (s->secure_schemes);
  g_hash_table_destroy (s->display_isolated);
  G_OBJECT_CLASS (webkit_security_manager_parent_class)->finalize (o);
}
static void webkit_security_manager_class_init (WebKitSecurityManagerClass *k) {
  G_OBJECT_CLASS (k)->finalize = webkit_security_manager_finalize;
}
static void webkit_security_manager_init (WebKitSecurityManager *s) {
  s->local_schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  s->secure_schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  s->display_isolated = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, NULL);
  g_hash_table_add (s->local_schemes, g_strdup ("file"));
  g_hash_table_add (s->secure_schemes, g_strdup ("https"));
}
void webkit_security_manager_register_uri_scheme_as_local (WebKitSecurityManager *sm, const char *scheme) {
  if (sm && scheme) g_hash_table_add (sm->local_schemes, g_strdup (scheme));
}
void webkit_security_manager_register_uri_scheme_as_secure (WebKitSecurityManager *sm, const char *scheme) {
  if (sm && scheme) g_hash_table_add (sm->secure_schemes, g_strdup (scheme));
}
void webkit_security_manager_register_uri_scheme_as_display_isolated (WebKitSecurityManager *sm, const char *scheme) {
  if (sm && scheme) g_hash_table_add (sm->display_isolated, g_strdup (scheme));
}
gboolean webkit_security_manager_uri_scheme_is_local (WebKitSecurityManager *sm, const char *scheme) {
  return sm && scheme && g_hash_table_contains (sm->local_schemes, scheme);
}
gboolean webkit_security_manager_uri_scheme_is_empty_document (WebKitSecurityManager *sm, const char *scheme) {
  (void)sm;
  return g_strcmp0 (scheme, "about") == 0;
}

/* ---------------- WebKitWebContext ---------------- */
struct _WebKitWebContext {
  GObject parent;
  WebKitWebContextPrivate priv;
};
G_DEFINE_TYPE (WebKitWebContext, webkit_web_context, G_TYPE_OBJECT)

static void uri_scheme_entry_free (gpointer data) {
  WinUriSchemeEntry *e = data;
  if (!e) return;
  if (e->destroy && e->user_data) e->destroy (e->user_data);
  g_free (e->scheme);
  g_free (e);
}

static WebKitWebContext *default_ctx = NULL;

static void webkit_web_context_finalize (GObject *o) {
  WebKitWebContext *ctx = WEBKIT_WEB_CONTEXT (o);
  if (ctx->priv.uri_schemes) g_hash_table_destroy (ctx->priv.uri_schemes);
  g_clear_object (&ctx->priv.security_manager);
  if (ctx->priv.wk_context) WKRelease (ctx->priv.wk_context);
  G_OBJECT_CLASS (webkit_web_context_parent_class)->finalize (o);
}
static void webkit_web_context_class_init (WebKitWebContextClass *k) {
  G_OBJECT_CLASS (k)->finalize = webkit_web_context_finalize;
  g_signal_new ("initialize-web-process-extensions", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("initialize-notification-permissions", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("automation-started", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_POINTER);
  g_signal_new ("user-message-received", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL, G_TYPE_BOOLEAN, 1, WEBKIT_TYPE_USER_MESSAGE);
}
static void webkit_web_context_init (WebKitWebContext *ctx) {
  ctx->priv.uri_schemes = g_hash_table_new_full (g_str_hash, g_str_equal, g_free, uri_scheme_entry_free);
  ctx->priv.security_manager = g_object_new (WEBKIT_TYPE_SECURITY_MANAGER, NULL);

  WKContextConfigurationRef cfg = WKContextConfigurationCreate ();
  g_autofree char *data_dir = g_build_filename (g_get_user_data_dir (), "epiphany", NULL);
  g_mkdir_with_parents (data_dir, 0755);
  WKStringRef wk_dir = WKStringCreateWithUTF8CString (data_dir);
  WKContextConfigurationSetLocalStorageDirectory (cfg, wk_dir);
  WKContextConfigurationSetIndexedDBDatabaseDirectory (cfg, wk_dir);
  WKRelease (wk_dir);
  ctx->priv.wk_context = WKContextCreateWithConfiguration (cfg);
  WKRelease (cfg);
}

WebKitWebContext *webkit_web_context_new (void) {
  WebKitWebContext *ctx = g_object_new (WEBKIT_TYPE_WEB_CONTEXT, NULL);
  if (!default_ctx) default_ctx = ctx;
  return ctx;
}
WebKitWebContext *win_webkit_web_context_get_default (void) {
  if (!default_ctx) default_ctx = webkit_web_context_new ();
  return default_ctx;
}
WKContextRef win_webkit_web_context_get_wk (WebKitWebContext *ctx) {
  return ctx ? ctx->priv.wk_context : NULL;
}

void webkit_web_context_register_uri_scheme (WebKitWebContext *context, const char *scheme,
                                            WebKitURISchemeRequestCallback callback,
                                            gpointer user_data, GDestroyNotify user_data_destroy_func) {
  if (!context || !scheme || !callback) return;
  WinUriSchemeEntry *e = g_new0 (WinUriSchemeEntry, 1);
  e->scheme = g_strdup (scheme);
  e->callback = callback;
  e->user_data = user_data;
  e->destroy = user_data_destroy_func;
  g_hash_table_replace (context->priv.uri_schemes, g_strdup (scheme), e);
}

WinUriSchemeEntry *win_webkit_web_context_lookup_scheme (WebKitWebContext *ctx, const char *uri) {
  if (!ctx || !uri) return NULL;
  const char *colon = strchr (uri, ':');
  if (!colon) return NULL;
  g_autofree char *scheme = g_strndup (uri, colon - uri);
  return g_hash_table_lookup (ctx->priv.uri_schemes, scheme);
}

WebKitSecurityManager *webkit_web_context_get_security_manager (WebKitWebContext *context) {
  return context ? context->priv.security_manager : NULL;
}
void webkit_web_context_set_preferred_languages (WebKitWebContext *context, const char * const *languages) { (void)context; (void)languages; }
void webkit_web_context_set_spell_checking_enabled (WebKitWebContext *context, gboolean enabled) { (void)context; (void)enabled; }
void webkit_web_context_set_spell_checking_languages (WebKitWebContext *context, const char * const *languages) { (void)context; (void)languages; }
void webkit_web_context_set_web_process_extensions_directory (WebKitWebContext *context, const char *directory) { (void)context; (void)directory; }
void webkit_web_context_set_web_process_extensions_initialization_user_data (WebKitWebContext *context, GVariant *user_data) { (void)context; (void)user_data; }
void webkit_web_context_add_path_to_sandbox (WebKitWebContext *context, const char *path, gboolean read_only) { (void)context; (void)path; (void)read_only; }
void webkit_web_context_set_automation_allowed (WebKitWebContext *context, gboolean allowed) { (void)context; (void)allowed; }
WebKitNetworkSession *webkit_web_context_get_network_session_for_automation (WebKitWebContext *context) {
  (void)context; return webkit_network_session_new_ephemeral ();
}
void webkit_web_context_initialize_notification_permissions (WebKitWebContext *context, GList *allowed_origins, GList *disallowed_origins) {
  (void)context; (void)allowed_origins; (void)disallowed_origins;
}
void webkit_web_context_send_message_to_all_extensions (WebKitWebContext *context, WebKitUserMessage *message) {
  (void)context; (void)message;
}

/* ---------------- WebKitCookieManager & FaviconDatabase & WebsiteDataManager ---------------- */
struct _WebKitCookieManager { GObject parent; };
G_DEFINE_TYPE (WebKitCookieManager, webkit_cookie_manager, G_TYPE_OBJECT)
static void webkit_cookie_manager_class_init (WebKitCookieManagerClass *k) {
  g_signal_new ("changed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}
static void webkit_cookie_manager_init (WebKitCookieManager *s) { (void)s; }
void webkit_cookie_manager_set_persistent_storage (WebKitCookieManager *cm, const char *filename, WebKitCookiePersistentStorage storage) {
  (void)cm; (void)filename; (void)storage;
}
void webkit_cookie_manager_set_accept_policy (WebKitCookieManager *cm, WebKitCookieAcceptPolicy policy) {
  (void)cm; (void)policy;
}
void webkit_cookie_manager_add_cookie (WebKitCookieManager *cm, SoupCookie *cookie, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)cookie; (void)cancellable;
  GTask *t = g_task_new (cm, NULL, callback, user_data);
  g_task_return_boolean (t, TRUE);
  g_object_unref (t);
}
gboolean webkit_cookie_manager_add_cookie_finish (WebKitCookieManager *cm, GAsyncResult *result, GError **error) {
  (void)cm; return g_task_propagate_boolean (G_TASK (result), error);
}
void webkit_cookie_manager_get_cookies (WebKitCookieManager *cm, const char *uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)uri; (void)cancellable;
  GTask *t = g_task_new (cm, NULL, callback, user_data);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
GList *webkit_cookie_manager_get_cookies_finish (WebKitCookieManager *cm, GAsyncResult *result, GError **error) {
  (void)cm; return g_task_propagate_pointer (G_TASK (result), error);
}
void webkit_cookie_manager_delete_cookie (WebKitCookieManager *cm, SoupCookie *cookie, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)cookie; (void)cancellable;
  GTask *t = g_task_new (cm, NULL, callback, user_data);
  g_task_return_boolean (t, TRUE);
  g_object_unref (t);
}
gboolean webkit_cookie_manager_delete_cookie_finish (WebKitCookieManager *cm, GAsyncResult *result, GError **error) {
  (void)cm; return g_task_propagate_boolean (G_TASK (result), error);
}

struct _WebKitFaviconDatabase { GObject parent; };
G_DEFINE_TYPE (WebKitFaviconDatabase, webkit_favicon_database, G_TYPE_OBJECT)
static void webkit_favicon_database_class_init (WebKitFaviconDatabaseClass *k) {
  g_signal_new ("favicon-changed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 2, G_TYPE_STRING, G_TYPE_STRING);
}
static void webkit_favicon_database_init (WebKitFaviconDatabase *s) { (void)s; }
void webkit_favicon_database_get_favicon (WebKitFaviconDatabase *db, const char *page_uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)page_uri; (void)cancellable;
  GTask *t = g_task_new (db, NULL, callback, user_data);
  g_task_return_new_error (t, WEBKIT_FAVICON_DATABASE_ERROR, WEBKIT_FAVICON_DATABASE_ERROR_FAVICON_NOT_FOUND, "Not found");
  g_object_unref (t);
}
GdkTexture *webkit_favicon_database_get_favicon_finish (WebKitFaviconDatabase *db, GAsyncResult *result, GError **error) {
  (void)db; return g_task_propagate_pointer (G_TASK (result), error);
}
char *webkit_favicon_database_get_favicon_uri (WebKitFaviconDatabase *db, const char *page_uri) {
  (void)db; (void)page_uri; return NULL;
}

struct _WebKitWebsiteDataManager { GObject parent; WebKitFaviconDatabase *fdb; };
G_DEFINE_TYPE (WebKitWebsiteDataManager, webkit_website_data_manager, G_TYPE_OBJECT)
static void webkit_website_data_manager_finalize (GObject *o) {
  g_clear_object (&WEBKIT_WEBSITE_DATA_MANAGER (o)->fdb);
  G_OBJECT_CLASS (webkit_website_data_manager_parent_class)->finalize (o);
}
static void webkit_website_data_manager_class_init (WebKitWebsiteDataManagerClass *k) {
  G_OBJECT_CLASS (k)->finalize = webkit_website_data_manager_finalize;
}
static void webkit_website_data_manager_init (WebKitWebsiteDataManager *s) {
  s->fdb = g_object_new (WEBKIT_TYPE_FAVICON_DATABASE, NULL);
}
void webkit_website_data_manager_set_favicons_enabled (WebKitWebsiteDataManager *m, gboolean enabled) { (void)m; (void)enabled; }
WebKitFaviconDatabase *webkit_website_data_manager_get_favicon_database (WebKitWebsiteDataManager *m) { return m ? m->fdb : NULL; }
void webkit_website_data_manager_clear (WebKitWebsiteDataManager *m, WebKitWebsiteDataTypes types, GTimeSpan timespan, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)types; (void)timespan; (void)c;
  GTask *t = g_task_new (m, NULL, cb, d);
  g_task_return_boolean (t, TRUE);
  g_object_unref (t);
}
gboolean webkit_website_data_manager_clear_finish (WebKitWebsiteDataManager *m, GAsyncResult *res, GError **e) {
  (void)m; return g_task_propagate_boolean (G_TASK (res), e);
}
void webkit_website_data_manager_fetch (WebKitWebsiteDataManager *m, WebKitWebsiteDataTypes types, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)types; (void)c;
  GTask *t = g_task_new (m, NULL, cb, d);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
GList *webkit_website_data_manager_fetch_finish (WebKitWebsiteDataManager *m, GAsyncResult *res, GError **e) {
  (void)m; return g_task_propagate_pointer (G_TASK (res), e);
}
void webkit_website_data_manager_remove (WebKitWebsiteDataManager *m, WebKitWebsiteDataTypes types, GList *wd, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)types; (void)wd; (void)c;
  GTask *t = g_task_new (m, NULL, cb, d);
  g_task_return_boolean (t, TRUE);
  g_object_unref (t);
}
gboolean webkit_website_data_manager_remove_finish (WebKitWebsiteDataManager *m, GAsyncResult *res, GError **e) {
  (void)m; return g_task_propagate_boolean (G_TASK (res), e);
}
void webkit_website_data_manager_get_itp_summary (WebKitWebsiteDataManager *m, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)c;
  GTask *t = g_task_new (m, NULL, cb, d);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
GList *webkit_website_data_manager_get_itp_summary_finish (WebKitWebsiteDataManager *m, GAsyncResult *res, GError **e) {
  (void)m; return g_task_propagate_pointer (G_TASK (res), e);
}

/* ---------------- WebKitNetworkSession ---------------- */
struct _WebKitNetworkSession {
  GObject parent;
  gboolean ephemeral;
  WebKitWebsiteDataManager *wdm;
  WebKitCookieManager *cm;
};
G_DEFINE_TYPE (WebKitNetworkSession, webkit_network_session, G_TYPE_OBJECT)
static void webkit_network_session_finalize (GObject *o) {
  WebKitNetworkSession *s = WEBKIT_NETWORK_SESSION (o);
  g_clear_object (&s->wdm);
  g_clear_object (&s->cm);
  G_OBJECT_CLASS (webkit_network_session_parent_class)->finalize (o);
}
static void webkit_network_session_class_init (WebKitNetworkSessionClass *k) {
  G_OBJECT_CLASS (k)->finalize = webkit_network_session_finalize;
  g_signal_new ("download-started", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST,
                0, NULL, NULL, NULL, G_TYPE_NONE, 1, WEBKIT_TYPE_DOWNLOAD);
}
static void webkit_network_session_init (WebKitNetworkSession *s) {
  s->wdm = g_object_new (WEBKIT_TYPE_WEBSITE_DATA_MANAGER, NULL);
  s->cm = g_object_new (WEBKIT_TYPE_COOKIE_MANAGER, NULL);
}
WebKitNetworkSession *webkit_network_session_new (const char *data_directory, const char *cache_directory) {
  (void)data_directory; (void)cache_directory;
  return g_object_new (WEBKIT_TYPE_NETWORK_SESSION, NULL);
}
WebKitNetworkSession *webkit_network_session_new_ephemeral (void) {
  WebKitNetworkSession *s = g_object_new (WEBKIT_TYPE_NETWORK_SESSION, NULL);
  s->ephemeral = TRUE;
  return s;
}
gboolean webkit_network_session_is_ephemeral (WebKitNetworkSession *s) { return s ? s->ephemeral : FALSE; }
WebKitWebsiteDataManager *webkit_network_session_get_website_data_manager (WebKitNetworkSession *s) { return s ? s->wdm : NULL; }
WebKitCookieManager *webkit_network_session_get_cookie_manager (WebKitNetworkSession *s) { return s ? s->cm : NULL; }
void webkit_network_session_set_itp_enabled (WebKitNetworkSession *s, gboolean enabled) { (void)s; (void)enabled; }
void webkit_network_session_set_persistent_credential_storage_enabled (WebKitNetworkSession *s, gboolean enabled) { (void)s; (void)enabled; }
void webkit_network_session_allow_tls_certificate_for_host (WebKitNetworkSession *s, GTlsCertificate *cert, const char *host) { (void)s; (void)cert; (void)host; }
void webkit_network_session_prefetch_dns (WebKitNetworkSession *s, const char *hostname) { (void)s; (void)hostname; }
WebKitDownload *webkit_network_session_download_uri (WebKitNetworkSession *s, const char *uri) {
  (void)s;
  return win_webkit_download_new_internal (uri);
}

/* ---------------- WebKitSettings & Features ---------------- */
struct _WebKitSettings {
  GObject parent;
  char *user_agent;
  char *default_font_family;
  char *sans_serif_font_family;
  char *serif_font_family;
  char *monospace_font_family;
  char *default_charset;
  char *user_stylesheet_uri;
  char *user_javascript_uri;
  char *accept_language;
  guint default_font_size;
  guint default_monospace_font_size;
  guint minimum_font_size;
  gboolean enable_developer_extras;
  gboolean enable_media_stream;
  gboolean enable_smooth_scrolling;
  gboolean javascript_can_open_windows_automatically;
  gboolean enable_caret_browsing;
  gboolean enable_site_specific_quirks;
  gboolean enable_back_forward_navigation_gestures;
  gboolean enable_javascript;
  gboolean enable_webgl;
  gboolean enable_mediasource;
  gboolean enable_encrypted_media;
  gboolean allow_modal_dialogs;
  gboolean zoom_text_only;
  gboolean enable_page_cache;
  gboolean enable_hyperlink_auditing;
  gboolean enable_dns_prefetching;
  gboolean allow_universal_access_from_file_urls;
  gboolean allow_file_access_from_file_urls;
  gboolean enable_resizable_text_areas;
  gboolean enable_tabs_to_links;
  gboolean enable_write_console_messages_to_stdout;
  gboolean media_playback_allows_inline;
  gboolean media_playback_requires_user_gesture;
  gboolean enable_javascript_markup;
  gboolean enable_mock_capture_devices;
  gboolean disable_web_security;
  gboolean enable_webaudio;
  gboolean enable_webrtc;
  int hardware_acceleration_policy;
};

G_DEFINE_TYPE (WebKitSettings, webkit_settings, G_TYPE_OBJECT)

static void webkit_settings_finalize (GObject *o) {
  WebKitSettings *s = WEBKIT_SETTINGS (o);
  g_free (s->user_agent);
  g_free (s->default_font_family);
  g_free (s->sans_serif_font_family);
  g_free (s->serif_font_family);
  g_free (s->monospace_font_family);
  g_free (s->default_charset);
  g_free (s->user_stylesheet_uri);
  g_free (s->user_javascript_uri);
  g_free (s->accept_language);
  G_OBJECT_CLASS (webkit_settings_parent_class)->finalize (o);
}

enum {
  PROP_0,
  PROP_USER_AGENT,
  PROP_DEFAULT_FONT_FAMILY,
  PROP_SANS_SERIF_FONT_FAMILY,
  PROP_SERIF_FONT_FAMILY,
  PROP_MONOSPACE_FONT_FAMILY,
  PROP_DEFAULT_CHARSET,
  PROP_DEFAULT_FONT_SIZE,
  PROP_DEFAULT_MONOSPACE_FONT_SIZE,
  PROP_MINIMUM_FONT_SIZE,
  PROP_ENABLE_DEVELOPER_EXTRAS,
  PROP_ENABLE_MEDIA_STREAM,
  PROP_ENABLE_SMOOTH_SCROLLING,
  PROP_JS_CAN_OPEN_WINDOWS,
  PROP_ENABLE_CARET_BROWSING,
  PROP_ENABLE_SITE_SPECIFIC_QUIRKS,
  PROP_ENABLE_NAV_GESTURES,
  PROP_HARDWARE_ACCEL_POLICY,
  PROP_ENABLE_JAVASCRIPT,
  PROP_ENABLE_WEBGL,
  PROP_ENABLE_MEDIASOURCE,
  PROP_ENABLE_ENCRYPTED_MEDIA,
  PROP_ALLOW_MODAL_DIALOGS,
  PROP_ZOOM_TEXT_ONLY,
  PROP_ENABLE_PAGE_CACHE,
  PROP_ENABLE_HYPERLINK_AUDITING,
  PROP_ENABLE_DNS_PREFETCHING,
  PROP_ENABLE_WEBAUDIO,
  PROP_ENABLE_WEBRTC,
  PROP_ENABLE_JS_MARKUP,
  PROP_MEDIA_REQUIRES_GESTURE,
  PROP_USER_STYLESHEET_URI,
  PROP_USER_JAVASCRIPT_URI,
  PROP_ACCEPT_LANGUAGE,
  N_SETTINGS_PROPS
};

static void webkit_settings_set_property (GObject *obj, guint prop_id, const GValue *val, GParamSpec *pspec) {
  (void)pspec;
  WebKitSettings *s = WEBKIT_SETTINGS (obj);
  switch (prop_id) {
    case PROP_USER_AGENT: g_free (s->user_agent); s->user_agent = g_value_dup_string (val); break;
    case PROP_DEFAULT_FONT_FAMILY: g_free (s->default_font_family); s->default_font_family = g_value_dup_string (val); break;
    case PROP_SANS_SERIF_FONT_FAMILY: g_free (s->sans_serif_font_family); s->sans_serif_font_family = g_value_dup_string (val); break;
    case PROP_SERIF_FONT_FAMILY: g_free (s->serif_font_family); s->serif_font_family = g_value_dup_string (val); break;
    case PROP_MONOSPACE_FONT_FAMILY: g_free (s->monospace_font_family); s->monospace_font_family = g_value_dup_string (val); break;
    case PROP_DEFAULT_CHARSET: g_free (s->default_charset); s->default_charset = g_value_dup_string (val); break;
    case PROP_USER_STYLESHEET_URI: g_free (s->user_stylesheet_uri); s->user_stylesheet_uri = g_value_dup_string (val); break;
    case PROP_USER_JAVASCRIPT_URI: g_free (s->user_javascript_uri); s->user_javascript_uri = g_value_dup_string (val); break;
    case PROP_ACCEPT_LANGUAGE: g_free (s->accept_language); s->accept_language = g_value_dup_string (val); break;
    case PROP_DEFAULT_FONT_SIZE: s->default_font_size = g_value_get_uint (val); break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE: s->default_monospace_font_size = g_value_get_uint (val); break;
    case PROP_MINIMUM_FONT_SIZE: s->minimum_font_size = g_value_get_uint (val); break;
    case PROP_ENABLE_DEVELOPER_EXTRAS: s->enable_developer_extras = g_value_get_boolean (val); break;
    case PROP_ENABLE_MEDIA_STREAM: s->enable_media_stream = g_value_get_boolean (val); break;
    case PROP_ENABLE_SMOOTH_SCROLLING: s->enable_smooth_scrolling = g_value_get_boolean (val); break;
    case PROP_JS_CAN_OPEN_WINDOWS: s->javascript_can_open_windows_automatically = g_value_get_boolean (val); break;
    case PROP_ENABLE_CARET_BROWSING: s->enable_caret_browsing = g_value_get_boolean (val); break;
    case PROP_ENABLE_SITE_SPECIFIC_QUIRKS: s->enable_site_specific_quirks = g_value_get_boolean (val); break;
    case PROP_ENABLE_NAV_GESTURES: s->enable_back_forward_navigation_gestures = g_value_get_boolean (val); break;
    case PROP_HARDWARE_ACCEL_POLICY: s->hardware_acceleration_policy = g_value_get_int (val); break;
    case PROP_ENABLE_JAVASCRIPT: s->enable_javascript = g_value_get_boolean (val); break;
    case PROP_ENABLE_WEBGL: s->enable_webgl = g_value_get_boolean (val); break;
    case PROP_ENABLE_MEDIASOURCE: s->enable_mediasource = g_value_get_boolean (val); break;
    case PROP_ENABLE_ENCRYPTED_MEDIA: s->enable_encrypted_media = g_value_get_boolean (val); break;
    case PROP_ALLOW_MODAL_DIALOGS: s->allow_modal_dialogs = g_value_get_boolean (val); break;
    case PROP_ZOOM_TEXT_ONLY: s->zoom_text_only = g_value_get_boolean (val); break;
    case PROP_ENABLE_PAGE_CACHE: s->enable_page_cache = g_value_get_boolean (val); break;
    case PROP_ENABLE_HYPERLINK_AUDITING: s->enable_hyperlink_auditing = g_value_get_boolean (val); break;
    case PROP_ENABLE_DNS_PREFETCHING: s->enable_dns_prefetching = g_value_get_boolean (val); break;
    case PROP_ENABLE_WEBAUDIO: s->enable_webaudio = g_value_get_boolean (val); break;
    case PROP_ENABLE_WEBRTC: s->enable_webrtc = g_value_get_boolean (val); break;
    case PROP_ENABLE_JS_MARKUP: s->enable_javascript_markup = g_value_get_boolean (val); break;
    case PROP_MEDIA_REQUIRES_GESTURE: s->media_playback_requires_user_gesture = g_value_get_boolean (val); break;
    default: break;
  }
}

static void webkit_settings_get_property (GObject *obj, guint prop_id, GValue *val, GParamSpec *pspec) {
  (void)pspec;
  WebKitSettings *s = WEBKIT_SETTINGS (obj);
  switch (prop_id) {
    case PROP_USER_AGENT: g_value_set_string (val, s->user_agent); break;
    case PROP_DEFAULT_FONT_FAMILY: g_value_set_string (val, s->default_font_family); break;
    case PROP_SANS_SERIF_FONT_FAMILY: g_value_set_string (val, s->sans_serif_font_family); break;
    case PROP_SERIF_FONT_FAMILY: g_value_set_string (val, s->serif_font_family); break;
    case PROP_MONOSPACE_FONT_FAMILY: g_value_set_string (val, s->monospace_font_family); break;
    case PROP_DEFAULT_CHARSET: g_value_set_string (val, s->default_charset); break;
    case PROP_USER_STYLESHEET_URI: g_value_set_string (val, s->user_stylesheet_uri); break;
    case PROP_USER_JAVASCRIPT_URI: g_value_set_string (val, s->user_javascript_uri); break;
    case PROP_ACCEPT_LANGUAGE: g_value_set_string (val, s->accept_language); break;
    case PROP_DEFAULT_FONT_SIZE: g_value_set_uint (val, s->default_font_size); break;
    case PROP_DEFAULT_MONOSPACE_FONT_SIZE: g_value_set_uint (val, s->default_monospace_font_size); break;
    case PROP_MINIMUM_FONT_SIZE: g_value_set_uint (val, s->minimum_font_size); break;
    case PROP_ENABLE_DEVELOPER_EXTRAS: g_value_set_boolean (val, s->enable_developer_extras); break;
    case PROP_ENABLE_MEDIA_STREAM: g_value_set_boolean (val, s->enable_media_stream); break;
    case PROP_ENABLE_SMOOTH_SCROLLING: g_value_set_boolean (val, s->enable_smooth_scrolling); break;
    case PROP_JS_CAN_OPEN_WINDOWS: g_value_set_boolean (val, s->javascript_can_open_windows_automatically); break;
    case PROP_ENABLE_CARET_BROWSING: g_value_set_boolean (val, s->enable_caret_browsing); break;
    case PROP_ENABLE_SITE_SPECIFIC_QUIRKS: g_value_set_boolean (val, s->enable_site_specific_quirks); break;
    case PROP_ENABLE_NAV_GESTURES: g_value_set_boolean (val, s->enable_back_forward_navigation_gestures); break;
    case PROP_HARDWARE_ACCEL_POLICY: g_value_set_int (val, s->hardware_acceleration_policy); break;
    case PROP_ENABLE_JAVASCRIPT: g_value_set_boolean (val, s->enable_javascript); break;
    case PROP_ENABLE_WEBGL: g_value_set_boolean (val, s->enable_webgl); break;
    case PROP_ENABLE_MEDIASOURCE: g_value_set_boolean (val, s->enable_mediasource); break;
    case PROP_ENABLE_ENCRYPTED_MEDIA: g_value_set_boolean (val, s->enable_encrypted_media); break;
    case PROP_ALLOW_MODAL_DIALOGS: g_value_set_boolean (val, s->allow_modal_dialogs); break;
    case PROP_ZOOM_TEXT_ONLY: g_value_set_boolean (val, s->zoom_text_only); break;
    case PROP_ENABLE_PAGE_CACHE: g_value_set_boolean (val, s->enable_page_cache); break;
    case PROP_ENABLE_HYPERLINK_AUDITING: g_value_set_boolean (val, s->enable_hyperlink_auditing); break;
    case PROP_ENABLE_DNS_PREFETCHING: g_value_set_boolean (val, s->enable_dns_prefetching); break;
    case PROP_ENABLE_WEBAUDIO: g_value_set_boolean (val, s->enable_webaudio); break;
    case PROP_ENABLE_WEBRTC: g_value_set_boolean (val, s->enable_webrtc); break;
    case PROP_ENABLE_JS_MARKUP: g_value_set_boolean (val, s->enable_javascript_markup); break;
    case PROP_MEDIA_REQUIRES_GESTURE: g_value_set_boolean (val, s->media_playback_requires_user_gesture); break;
    default: break;
  }
}

static void webkit_settings_class_init (WebKitSettingsClass *k) {
  GObjectClass *oc = G_OBJECT_CLASS (k);
  oc->finalize = webkit_settings_finalize;
  oc->set_property = webkit_settings_set_property;
  oc->get_property = webkit_settings_get_property;

  #define REG_STR(id, name, def) g_object_class_install_property (oc, id, g_param_spec_string (name, name, name, def, G_PARAM_READWRITE))
  #define REG_BOOL(id, name, def) g_object_class_install_property (oc, id, g_param_spec_boolean (name, name, name, def, G_PARAM_READWRITE))
  #define REG_UINT(id, name, def) g_object_class_install_property (oc, id, g_param_spec_uint (name, name, name, 0, 1024, def, G_PARAM_READWRITE))

  REG_STR (PROP_USER_AGENT, "user-agent", "Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/605.1.15 (KHTML, like Gecko) Epiphany/51.2 Safari/605.1.15");
  REG_STR (PROP_DEFAULT_FONT_FAMILY, "default-font-family", "sans-serif");
  REG_STR (PROP_SANS_SERIF_FONT_FAMILY, "sans-serif-font-family", "sans-serif");
  REG_STR (PROP_SERIF_FONT_FAMILY, "serif-font-family", "serif");
  REG_STR (PROP_MONOSPACE_FONT_FAMILY, "monospace-font-family", "monospace");
  REG_STR (PROP_DEFAULT_CHARSET, "default-charset", "UTF-8");
  REG_STR (PROP_USER_STYLESHEET_URI, "user-stylesheet-uri", NULL);
  REG_STR (PROP_USER_JAVASCRIPT_URI, "user-javascript-uri", NULL);
  REG_STR (PROP_ACCEPT_LANGUAGE, "accept-language", "en-US,en");
  REG_UINT (PROP_DEFAULT_FONT_SIZE, "default-font-size", 16);
  REG_UINT (PROP_DEFAULT_MONOSPACE_FONT_SIZE, "default-monospace-font-size", 13);
  REG_UINT (PROP_MINIMUM_FONT_SIZE, "minimum-font-size", 0);
  REG_BOOL (PROP_ENABLE_DEVELOPER_EXTRAS, "enable-developer-extras", TRUE);
  REG_BOOL (PROP_ENABLE_MEDIA_STREAM, "enable-media-stream", TRUE);
  REG_BOOL (PROP_ENABLE_SMOOTH_SCROLLING, "enable-smooth-scrolling", TRUE);
  REG_BOOL (PROP_JS_CAN_OPEN_WINDOWS, "javascript-can-open-windows-automatically", TRUE);
  REG_BOOL (PROP_ENABLE_CARET_BROWSING, "enable-caret-browsing", FALSE);
  REG_BOOL (PROP_ENABLE_SITE_SPECIFIC_QUIRKS, "enable-site-specific-quirks", TRUE);
  REG_BOOL (PROP_ENABLE_NAV_GESTURES, "enable-back-forward-navigation-gestures", TRUE);
  g_object_class_install_property (oc, PROP_HARDWARE_ACCEL_POLICY, g_param_spec_int ("hardware-acceleration-policy", "hardware-acceleration-policy", "policy", 0, 2, 0, G_PARAM_READWRITE));
  REG_BOOL (PROP_ENABLE_JAVASCRIPT, "enable-javascript", TRUE);
  REG_BOOL (PROP_ENABLE_WEBGL, "enable-webgl", TRUE);
  REG_BOOL (PROP_ENABLE_MEDIASOURCE, "enable-mediasource", TRUE);
  REG_BOOL (PROP_ENABLE_ENCRYPTED_MEDIA, "enable-encrypted-media", TRUE);
  REG_BOOL (PROP_ALLOW_MODAL_DIALOGS, "allow-modal-dialogs", TRUE);
  REG_BOOL (PROP_ZOOM_TEXT_ONLY, "zoom-text-only", FALSE);
  REG_BOOL (PROP_ENABLE_PAGE_CACHE, "enable-page-cache", TRUE);
  REG_BOOL (PROP_ENABLE_HYPERLINK_AUDITING, "enable-hyperlink-auditing", FALSE);
  REG_BOOL (PROP_ENABLE_DNS_PREFETCHING, "enable-dns-prefetching", TRUE);
  REG_BOOL (PROP_ENABLE_WEBAUDIO, "enable-webaudio", TRUE);
  REG_BOOL (PROP_ENABLE_WEBRTC, "enable-webrtc", TRUE);
  REG_BOOL (PROP_ENABLE_JS_MARKUP, "enable-javascript-markup", TRUE);
  REG_BOOL (PROP_MEDIA_REQUIRES_GESTURE, "media-playback-requires-user-gesture", FALSE);

  #undef REG_STR
  #undef REG_BOOL
  #undef REG_UINT
}

static void webkit_settings_init (WebKitSettings *s) {
  s->user_agent = g_strdup ("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/605.1.15 (KHTML, like Gecko) Epiphany/51.2 Safari/605.1.15");
  s->default_font_size = 16;
  s->default_monospace_font_size = 13;
  s->enable_javascript = TRUE;
  s->enable_developer_extras = TRUE;
  s->enable_smooth_scrolling = TRUE;
}

WebKitSettings *webkit_settings_new (void) { return g_object_new (WEBKIT_TYPE_SETTINGS, NULL); }
WebKitSettings *webkit_settings_new_with_settings (const char *first_setting_name, ...) {
  WebKitSettings *s = webkit_settings_new ();
  if (first_setting_name) {
    va_list args;
    va_start (args, first_setting_name);
    g_object_set_valist (G_OBJECT (s), first_setting_name, args);
    va_end (args);
  }
  return s;
}
guint32 webkit_settings_font_size_to_pixels (guint32 points) { return (guint32)(points * 4 / 3); }
const char *webkit_settings_get_user_agent (WebKitSettings *s) { return s ? s->user_agent : ""; }
void webkit_settings_set_user_agent (WebKitSettings *s, const char *ua) {
  if (!s) return;
  g_free (s->user_agent);
  s->user_agent = g_strdup (ua);
}
void webkit_settings_set_user_agent_with_application_details (WebKitSettings *s, const char *app, const char *ver) {
  if (!s) return;
  g_free (s->user_agent);
  s->user_agent = g_strdup_printf ("Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/605.1.15 (KHTML, like Gecko) %s/%s Safari/605.1.15",
                                   app ? app : "Epiphany", ver ? ver : "51.2");
}
void webkit_settings_set_hardware_acceleration_policy (WebKitSettings *s, WebKitHardwareAccelerationPolicy policy) {
  if (s) s->hardware_acceleration_policy = policy;
}
void webkit_settings_set_enable_html5_database (WebKitSettings *s, gboolean enabled) { (void)s; (void)enabled; }
void webkit_settings_set_enable_html5_local_storage (WebKitSettings *s, gboolean enabled) { (void)s; (void)enabled; }

struct _WebKitFeature { int ref_count; char *name; char *cat; char *details; gboolean def; WebKitFeatureStatus status; };
struct _WebKitFeatureList { int dummy; };
GType webkit_feature_status_get_type (void) {
  static GType t = 0;
  if (!t) {
    static const GEnumValue v[] = {
      { WEBKIT_FEATURE_STATUS_EMBEDDER, "WEBKIT_FEATURE_STATUS_EMBEDDER", "embedder" },
      { WEBKIT_FEATURE_STATUS_INTERNAL, "WEBKIT_FEATURE_STATUS_INTERNAL", "internal" },
      { WEBKIT_FEATURE_STATUS_STABLE, "WEBKIT_FEATURE_STATUS_STABLE", "stable" },
      { 0, NULL, NULL }
    };
    t = g_enum_register_static ("WebKitFeatureStatus", v);
  }
  return t;
}
WebKitFeatureList *webkit_settings_get_all_features (void) { static WebKitFeatureList fl; return &fl; }
void webkit_feature_list_unref (WebKitFeatureList *fl) { (void)fl; }
gsize webkit_feature_list_get_length (WebKitFeatureList *fl) { (void)fl; return 0; }
WebKitFeature *webkit_feature_list_get (WebKitFeatureList *fl, gsize i) { (void)fl; (void)i; return NULL; }
WebKitFeature *webkit_feature_ref (WebKitFeature *f) { if (f) g_atomic_int_inc (&f->ref_count); return f; }
void webkit_feature_unref (WebKitFeature *f) {
  if (f && g_atomic_int_dec_and_test (&f->ref_count)) {
    g_free (f->name); g_free (f->cat); g_free (f->details); g_free (f);
  }
}
const char *webkit_feature_get_name (WebKitFeature *f) { return f ? f->name : ""; }
const char *webkit_feature_get_category (WebKitFeature *f) { return f ? f->cat : ""; }
const char *webkit_feature_get_details (WebKitFeature *f) { return f ? f->details : ""; }
gboolean webkit_feature_get_default_value (WebKitFeature *f) { return f ? f->def : FALSE; }
WebKitFeatureStatus webkit_feature_get_status (WebKitFeature *f) { return f ? f->status : WEBKIT_FEATURE_STATUS_STABLE; }
gboolean webkit_settings_get_feature_enabled (WebKitSettings *s, WebKitFeature *f) { (void)s; (void)f; return FALSE; }
void webkit_settings_set_feature_enabled (WebKitSettings *s, WebKitFeature *f, gboolean e) { (void)s; (void)f; (void)e; }

/* ---------------- UserContentManager ---------------- */
struct _WebKitUserContentManager { GObject parent; };
G_DEFINE_TYPE (WebKitUserContentManager, webkit_user_content_manager, G_TYPE_OBJECT)
static void webkit_user_content_manager_class_init (WebKitUserContentManagerClass *k) {
  g_signal_new ("script-message-received", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST | G_SIGNAL_DETAILED,
                0, NULL, NULL, NULL, G_TYPE_NONE, 1, JSC_TYPE_VALUE);
}
static void webkit_user_content_manager_init (WebKitUserContentManager *s) { (void)s; }
WebKitUserContentManager *webkit_user_content_manager_new (void) { return g_object_new (WEBKIT_TYPE_USER_CONTENT_MANAGER, NULL); }
void webkit_user_content_manager_add_script (WebKitUserContentManager *m, WebKitUserScript *s) { (void)m; (void)s; }
void webkit_user_content_manager_remove_script (WebKitUserContentManager *m, WebKitUserScript *s) { (void)m; (void)s; }
void webkit_user_content_manager_remove_all_scripts (WebKitUserContentManager *m) { (void)m; }
void webkit_user_content_manager_add_style_sheet (WebKitUserContentManager *m, WebKitUserStyleSheet *s) { (void)m; (void)s; }
void webkit_user_content_manager_remove_style_sheet (WebKitUserContentManager *m, WebKitUserStyleSheet *s) { (void)m; (void)s; }
void webkit_user_content_manager_remove_all_style_sheets (WebKitUserContentManager *m) { (void)m; }
void webkit_user_content_manager_add_filter (WebKitUserContentManager *m, WebKitUserContentFilter *f) { (void)m; (void)f; }
void webkit_user_content_manager_remove_all_filters (WebKitUserContentManager *m) { (void)m; }
gboolean webkit_user_content_manager_register_script_message_handler (WebKitUserContentManager *m, const char *n, const char *w) { (void)m; (void)n; (void)w; return TRUE; }
void webkit_user_content_manager_unregister_script_message_handler (WebKitUserContentManager *m, const char *n, const char *w) { (void)m; (void)n; (void)w; }

struct _WebKitUserScript { int rc; };
WebKitUserScript *webkit_user_script_new (const char *s, WebKitUserContentInjectedFrames i, WebKitUserScriptInjectionTime t, const char * const *a, const char * const *b) {
  (void)s; (void)i; (void)t; (void)a; (void)b; return g_new0 (WebKitUserScript, 1);
}
WebKitUserScript *webkit_user_script_new_for_world (const char *s, WebKitUserContentInjectedFrames i, WebKitUserScriptInjectionTime t, const char *w, const char * const *a, const char * const *b) {
  (void)s; (void)i; (void)t; (void)w; (void)a; (void)b; return g_new0 (WebKitUserScript, 1);
}
void webkit_user_script_unref (WebKitUserScript *s) { g_free (s); }

struct _WebKitUserStyleSheet { int rc; };
WebKitUserStyleSheet *webkit_user_style_sheet_new (const char *s, WebKitUserContentInjectedFrames i, WebKitUserContentLevel l, const char * const *a, const char * const *b) {
  (void)s; (void)i; (void)l; (void)a; (void)b; return g_new0 (WebKitUserStyleSheet, 1);
}
WebKitUserStyleSheet *webkit_user_style_sheet_new_for_world (const char *s, WebKitUserContentInjectedFrames i, WebKitUserContentLevel l, const char *w, const char * const *a, const char * const *b) {
  (void)s; (void)i; (void)l; (void)w; (void)a; (void)b; return g_new0 (WebKitUserStyleSheet, 1);
}
void webkit_user_style_sheet_unref (WebKitUserStyleSheet *s) { g_free (s); }

struct _WebKitUserContentFilter { int rc; };
void webkit_user_content_filter_unref (WebKitUserContentFilter *f) { g_free (f); }

struct _WebKitUserContentFilterStore { GObject parent; };
G_DEFINE_TYPE (WebKitUserContentFilterStore, webkit_user_content_filter_store, G_TYPE_OBJECT)
static void webkit_user_content_filter_store_class_init (WebKitUserContentFilterStoreClass *k) { (void)k; }
static void webkit_user_content_filter_store_init (WebKitUserContentFilterStore *s) { (void)s; }
WebKitUserContentFilterStore *webkit_user_content_filter_store_new (const char *path) { (void)path; return g_object_new (WEBKIT_TYPE_USER_CONTENT_FILTER_STORE, NULL); }
void webkit_user_content_filter_store_save (WebKitUserContentFilterStore *s, const char *id, GBytes *src, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)id; (void)src; (void)c;
  GTask *t = g_task_new (s, NULL, cb, d);
  g_task_return_pointer (t, g_new0 (WebKitUserContentFilter, 1), g_free);
  g_object_unref (t);
}
WebKitUserContentFilter *webkit_user_content_filter_store_save_finish (WebKitUserContentFilterStore *s, GAsyncResult *res, GError **e) {
  (void)s; return g_task_propagate_pointer (G_TASK (res), e);
}
void webkit_user_content_filter_store_load (WebKitUserContentFilterStore *s, const char *id, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)id; (void)c;
  GTask *t = g_task_new (s, NULL, cb, d);
  g_task_return_new_error (t, WEBKIT_USER_CONTENT_FILTER_ERROR, WEBKIT_USER_CONTENT_FILTER_ERROR_NOT_FOUND, "Not found");
  g_object_unref (t);
}
WebKitUserContentFilter *webkit_user_content_filter_store_load_finish (WebKitUserContentFilterStore *s, GAsyncResult *res, GError **e) {
  (void)s; return g_task_propagate_pointer (G_TASK (res), e);
}
void webkit_user_content_filter_store_remove (WebKitUserContentFilterStore *s, const char *id, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)id; (void)c;
  GTask *t = g_task_new (s, NULL, cb, d);
  g_task_return_boolean (t, TRUE);
  g_object_unref (t);
}
gboolean webkit_user_content_filter_store_remove_finish (WebKitUserContentFilterStore *s, GAsyncResult *res, GError **e) {
  (void)s; return g_task_propagate_boolean (G_TASK (res), e);
}
