#include "webkit-win32-internal.h"
#include <stdarg.h>
#include <string.h>

/* ---------------- SecurityOrigin ---------------- */
struct _WebKitSecurityOrigin {
  int rc;
  char *protocol;
  char *host;
  guint16 port;
};
WebKitSecurityOrigin *webkit_security_origin_new (const char *protocol, const char *host, guint16 port) {
  WebKitSecurityOrigin *o = g_new0 (WebKitSecurityOrigin, 1);
  o->rc = 1;
  o->protocol = g_strdup (protocol ? protocol : "");
  o->host = g_strdup (host ? host : "");
  o->port = port;
  return o;
}
WebKitSecurityOrigin *webkit_security_origin_new_for_uri (const char *uri) {
  g_autoptr (GUri) u = uri ? g_uri_parse (uri, G_URI_FLAGS_NONE, NULL) : NULL;
  if (!u) return webkit_security_origin_new ("", "", 0);
  int p = g_uri_get_port (u);
  return webkit_security_origin_new (g_uri_get_scheme (u), g_uri_get_host (u), p > 0 ? (guint16)p : 0);
}
WebKitSecurityOrigin *webkit_security_origin_ref (WebKitSecurityOrigin *o) { if (o) g_atomic_int_inc (&o->rc); return o; }
void webkit_security_origin_unref (WebKitSecurityOrigin *o) {
  if (o && g_atomic_int_dec_and_test (&o->rc)) { g_free (o->protocol); g_free (o->host); g_free (o); }
}
const char *webkit_security_origin_get_protocol (WebKitSecurityOrigin *o) { return o ? o->protocol : ""; }
const char *webkit_security_origin_get_host (WebKitSecurityOrigin *o) { return o ? o->host : ""; }
guint16 webkit_security_origin_get_port (WebKitSecurityOrigin *o) { return o ? o->port : 0; }
char *webkit_security_origin_to_string (WebKitSecurityOrigin *o) {
  if (!o) return g_strdup ("");
  if (o->port) return g_strdup_printf ("%s://%s:%u", o->protocol, o->host, o->port);
  return g_strdup_printf ("%s://%s", o->protocol, o->host);
}

/* ---------------- URIRequest & URIResponse & URISchemeRequest ---------------- */
struct _WebKitURIRequest { GObject parent; char *uri; };
G_DEFINE_TYPE (WebKitURIRequest, webkit_uri_request, G_TYPE_OBJECT)
static void webkit_uri_request_finalize (GObject *o) { g_free (WEBKIT_URI_REQUEST (o)->uri); G_OBJECT_CLASS (webkit_uri_request_parent_class)->finalize (o); }
static void webkit_uri_request_class_init (WebKitURIRequestClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_uri_request_finalize; }
static void webkit_uri_request_init (WebKitURIRequest *s) { (void)s; }
WebKitURIRequest *webkit_uri_request_new (const char *uri) {
  WebKitURIRequest *r = g_object_new (WEBKIT_TYPE_URI_REQUEST, NULL);
  r->uri = g_strdup (uri);
  return r;
}
WebKitURIRequest *win_webkit_uri_request_new_internal (const char *uri) { return webkit_uri_request_new (uri); }
const char *webkit_uri_request_get_uri (WebKitURIRequest *r) { return r ? r->uri : NULL; }
void webkit_uri_request_set_uri (WebKitURIRequest *r, const char *uri) { if (r) { g_free (r->uri); r->uri = g_strdup (uri); } }

struct _WebKitURIResponse { GObject parent; char *uri; char *mime; guint status; guint64 len; char *suggested; };
G_DEFINE_TYPE (WebKitURIResponse, webkit_uri_response, G_TYPE_OBJECT)
static void webkit_uri_response_finalize (GObject *o) {
  WebKitURIResponse *r = WEBKIT_URI_RESPONSE (o);
  g_free (r->uri); g_free (r->mime); g_free (r->suggested);
  G_OBJECT_CLASS (webkit_uri_response_parent_class)->finalize (o);
}
static void webkit_uri_response_class_init (WebKitURIResponseClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_uri_response_finalize; }
static void webkit_uri_response_init (WebKitURIResponse *s) { (void)s; }
WebKitURIResponse *win_webkit_uri_response_new_internal (const char *uri, const char *mime, guint status, guint64 len, const char *suggested) {
  WebKitURIResponse *r = g_object_new (WEBKIT_TYPE_URI_RESPONSE, NULL);
  r->uri = g_strdup (uri); r->mime = g_strdup (mime ? mime : "text/html"); r->status = status; r->len = len; r->suggested = g_strdup (suggested);
  return r;
}
const char *webkit_uri_response_get_uri (WebKitURIResponse *r) { return r ? r->uri : NULL; }
guint webkit_uri_response_get_status_code (WebKitURIResponse *r) { return r ? r->status : 200; }
guint64 webkit_uri_response_get_content_length (WebKitURIResponse *r) { return r ? r->len : 0; }
const char *webkit_uri_response_get_mime_type (WebKitURIResponse *r) { return r ? r->mime : "text/html"; }
const char *webkit_uri_response_get_suggested_filename (WebKitURIResponse *r) { return r ? r->suggested : NULL; }

struct _WebKitURISchemeRequest { GObject parent; WebKitWebView *view; char *uri; char *path; };
G_DEFINE_TYPE (WebKitURISchemeRequest, webkit_uri_scheme_request, G_TYPE_OBJECT)
static void webkit_uri_scheme_request_finalize (GObject *o) {
  WebKitURISchemeRequest *r = WEBKIT_URI_SCHEME_REQUEST (o);
  g_free (r->uri); g_free (r->path);
  G_OBJECT_CLASS (webkit_uri_scheme_request_parent_class)->finalize (o);
}
static void webkit_uri_scheme_request_class_init (WebKitURISchemeRequestClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_uri_scheme_request_finalize; }
static void webkit_uri_scheme_request_init (WebKitURISchemeRequest *s) { (void)s; }
WebKitURISchemeRequest *win_webkit_uri_scheme_request_new_internal (WebKitWebView *view, const char *uri) {
  WebKitURISchemeRequest *r = g_object_new (WEBKIT_TYPE_URI_SCHEME_REQUEST, NULL);
  r->view = view;
  r->uri = g_strdup (uri);
  const char *colon = uri ? strchr (uri, ':') : NULL;
  r->path = g_strdup (colon ? colon + 1 : "");
  return r;
}
const char *webkit_uri_scheme_request_get_uri (WebKitURISchemeRequest *r) { return r ? r->uri : ""; }
const char *webkit_uri_scheme_request_get_path (WebKitURISchemeRequest *r) { return r ? r->path : ""; }
WebKitWebView *webkit_uri_scheme_request_get_web_view (WebKitURISchemeRequest *r) { return r ? r->view : NULL; }
void webkit_uri_scheme_request_finish (WebKitURISchemeRequest *r, GInputStream *stream, gint64 stream_length, const char *content_type) {
  (void)stream_length;
  if (!r || !r->view || !stream) return;
  GByteArray *buf = g_byte_array_new ();
  guint8 tmp[8192];
  gssize n;
  while ((n = g_input_stream_read (stream, tmp, sizeof (tmp), NULL, NULL)) > 0)
    g_byte_array_append (buf, tmp, (guint)n);
  g_autoptr (GBytes) bytes = g_byte_array_free_to_bytes (buf);
  win_webkit_web_view_finish_scheme_request (r->view, r->uri, bytes, content_type);
}
void webkit_uri_scheme_request_finish_error (WebKitURISchemeRequest *r, GError *error) {
  (void)r; (void)error;
}

/* ---------------- PolicyDecision & NavigationAction ---------------- */
typedef struct { int dummy; } WebKitPolicyDecisionPrivate;
G_DEFINE_TYPE (WebKitPolicyDecision, webkit_policy_decision, G_TYPE_OBJECT)
static void webkit_policy_decision_class_init (WebKitPolicyDecisionClass *k) { (void)k; }
static void webkit_policy_decision_init (WebKitPolicyDecision *s) { (void)s; }
void webkit_policy_decision_use (WebKitPolicyDecision *d) { (void)d; }
void webkit_policy_decision_use_with_policies (WebKitPolicyDecision *d, WebKitWebsitePolicies *p) { (void)d; (void)p; }
void webkit_policy_decision_ignore (WebKitPolicyDecision *d) { (void)d; }
void webkit_policy_decision_download (WebKitPolicyDecision *d) { (void)d; }

struct _WebKitNavigationAction { WebKitURIRequest *req; WebKitNavigationType type; guint btn; guint mods; gboolean gesture; };
WebKitNavigationAction *win_webkit_navigation_action_new_internal (const char *uri, WebKitNavigationType type, guint btn, guint mods, gboolean gesture) {
  WebKitNavigationAction *a = g_new0 (WebKitNavigationAction, 1);
  a->req = webkit_uri_request_new (uri);
  a->type = type; a->btn = btn; a->mods = mods; a->gesture = gesture;
  return a;
}
WebKitURIRequest *webkit_navigation_action_get_request (WebKitNavigationAction *n) { return n ? n->req : NULL; }
WebKitNavigationType webkit_navigation_action_get_navigation_type (WebKitNavigationAction *n) { return n ? n->type : WEBKIT_NAVIGATION_TYPE_OTHER; }
guint webkit_navigation_action_get_mouse_button (WebKitNavigationAction *n) { return n ? n->btn : 0; }
guint webkit_navigation_action_get_modifiers (WebKitNavigationAction *n) { return n ? n->mods : 0; }
gboolean webkit_navigation_action_is_user_gesture (WebKitNavigationAction *n) { return n ? n->gesture : FALSE; }

struct _WebKitNavigationPolicyDecision { WebKitPolicyDecision parent; WebKitNavigationAction *action; };
G_DEFINE_TYPE (WebKitNavigationPolicyDecision, webkit_navigation_policy_decision, WEBKIT_TYPE_POLICY_DECISION)
static void webkit_navigation_policy_decision_class_init (WebKitNavigationPolicyDecisionClass *k) { (void)k; }
static void webkit_navigation_policy_decision_init (WebKitNavigationPolicyDecision *s) { (void)s; }
WebKitNavigationPolicyDecision *win_webkit_navigation_policy_decision_new_internal (WebKitNavigationAction *action) {
  WebKitNavigationPolicyDecision *d = g_object_new (WEBKIT_TYPE_NAVIGATION_POLICY_DECISION, NULL);
  d->action = action;
  return d;
}
WebKitNavigationAction *webkit_navigation_policy_decision_get_navigation_action (WebKitNavigationPolicyDecision *d) {
  return d ? d->action : NULL;
}

struct _WebKitResponsePolicyDecision { WebKitPolicyDecision parent; WebKitURIResponse *resp; };
G_DEFINE_TYPE (WebKitResponsePolicyDecision, webkit_response_policy_decision, WEBKIT_TYPE_POLICY_DECISION)
static void webkit_response_policy_decision_finalize (GObject *o) {
  g_clear_object (&WEBKIT_RESPONSE_POLICY_DECISION (o)->resp);
  G_OBJECT_CLASS (webkit_response_policy_decision_parent_class)->finalize (o);
}
static void webkit_response_policy_decision_class_init (WebKitResponsePolicyDecisionClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_response_policy_decision_finalize; }
static void webkit_response_policy_decision_init (WebKitResponsePolicyDecision *s) { (void)s; }
WebKitURIResponse *webkit_response_policy_decision_get_response (WebKitResponsePolicyDecision *d) { return d ? d->resp : NULL; }
gboolean webkit_response_policy_decision_is_main_frame_main_resource (WebKitResponsePolicyDecision *d) { (void)d; return TRUE; }
gboolean webkit_response_policy_decision_is_mime_type_supported (WebKitResponsePolicyDecision *d) { (void)d; return TRUE; }

struct _WebKitWebsitePolicies { GObject parent; };
G_DEFINE_TYPE (WebKitWebsitePolicies, webkit_website_policies, G_TYPE_OBJECT)
static void webkit_website_policies_class_init (WebKitWebsitePoliciesClass *k) { (void)k; }
static void webkit_website_policies_init (WebKitWebsitePolicies *s) { (void)s; }
WebKitWebsitePolicies *webkit_website_policies_new_with_policies (const char *first_policy_name, ...) {
  (void)first_policy_name;
  return g_object_new (WEBKIT_TYPE_WEBSITE_POLICIES, NULL);
}

/* ---------------- BackForwardList ---------------- */
struct _WebKitBackForwardListItem { GInitiallyUnowned parent; char *uri; char *title; char *orig_uri; };
G_DEFINE_TYPE (WebKitBackForwardListItem, webkit_back_forward_list_item, G_TYPE_INITIALLY_UNOWNED)
static void webkit_back_forward_list_item_finalize (GObject *o) {
  WebKitBackForwardListItem *it = WEBKIT_BACK_FORWARD_LIST_ITEM (o);
  g_free (it->uri); g_free (it->title); g_free (it->orig_uri);
  G_OBJECT_CLASS (webkit_back_forward_list_item_parent_class)->finalize (o);
}
static void webkit_back_forward_list_item_class_init (WebKitBackForwardListItemClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_back_forward_list_item_finalize; }
static void webkit_back_forward_list_item_init (WebKitBackForwardListItem *s) { (void)s; }
const char *webkit_back_forward_list_item_get_uri (WebKitBackForwardListItem *i) { return i ? i->uri : ""; }
const char *webkit_back_forward_list_item_get_title (WebKitBackForwardListItem *i) { return i ? i->title : ""; }
const char *webkit_back_forward_list_item_get_original_uri (WebKitBackForwardListItem *i) { return i ? (i->orig_uri ? i->orig_uri : i->uri) : ""; }

struct _WebKitBackForwardList { GObject parent; WebKitWebView *view; };
G_DEFINE_TYPE (WebKitBackForwardList, webkit_back_forward_list, G_TYPE_OBJECT)
static void webkit_back_forward_list_class_init (WebKitBackForwardListClass *k) {
  g_signal_new ("changed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL,
                G_TYPE_NONE, 2, WEBKIT_TYPE_BACK_FORWARD_LIST_ITEM, G_TYPE_POINTER);
}
static void webkit_back_forward_list_init (WebKitBackForwardList *s) { (void)s; }
WebKitBackForwardList *win_webkit_back_forward_list_new_internal (WebKitWebView *view) {
  WebKitBackForwardList *bfl = g_object_new (WEBKIT_TYPE_BACK_FORWARD_LIST, NULL);
  bfl->view = view;
  return bfl;
}
static WebKitBackForwardListItem *make_bfl_item (WKBackForwardListItemRef ref) {
  if (!ref) return NULL;
  WebKitBackForwardListItem *it = g_object_new (WEBKIT_TYPE_BACK_FORWARD_LIST_ITEM, NULL);
  WKURLRef u = WKBackForwardListItemCopyURL (ref);
  if (u) {
    WKStringRef s = WKURLCopyString (u);
    size_t max = WKStringGetMaximumUTF8CStringSize (s);
    it->uri = g_malloc (max);
    WKStringGetUTF8CString (s, it->uri, max);
    WKRelease (s); WKRelease (u);
  }
  WKStringRef t = WKBackForwardListItemCopyTitle (ref);
  if (t) {
    size_t max = WKStringGetMaximumUTF8CStringSize (t);
    it->title = g_malloc (max);
    WKStringGetUTF8CString (t, it->title, max);
    WKRelease (t);
  }
  return it;
}
WebKitBackForwardListItem *webkit_back_forward_list_get_current_item (WebKitBackForwardList *bfl) {
  WKPageRef p = bfl ? win_webkit_web_view_get_wk_page (bfl->view) : NULL;
  if (!p) return NULL;
  WKBackForwardListRef list = WKPageGetBackForwardList (p);
  return list ? make_bfl_item (WKBackForwardListGetCurrentItem (list)) : NULL;
}
WebKitBackForwardListItem *webkit_back_forward_list_get_back_item (WebKitBackForwardList *bfl) {
  WKPageRef p = bfl ? win_webkit_web_view_get_wk_page (bfl->view) : NULL;
  if (!p) return NULL;
  WKBackForwardListRef list = WKPageGetBackForwardList (p);
  return list ? make_bfl_item (WKBackForwardListGetBackItem (list)) : NULL;
}
WebKitBackForwardListItem *webkit_back_forward_list_get_forward_item (WebKitBackForwardList *bfl) {
  WKPageRef p = bfl ? win_webkit_web_view_get_wk_page (bfl->view) : NULL;
  if (!p) return NULL;
  WKBackForwardListRef list = WKPageGetBackForwardList (p);
  return list ? make_bfl_item (WKBackForwardListGetForwardItem (list)) : NULL;
}
GList *webkit_back_forward_list_get_back_list_with_limit (WebKitBackForwardList *bfl, guint limit) { (void)bfl; (void)limit; return NULL; }
GList *webkit_back_forward_list_get_forward_list_with_limit (WebKitBackForwardList *bfl, guint limit) { (void)bfl; (void)limit; return NULL; }

/* ---------------- Download ---------------- */
struct _WebKitDownload { GObject parent; WebKitURIRequest *req; WebKitURIResponse *resp; char *dest; gboolean overwrite; double progress; guint64 received; };
G_DEFINE_TYPE (WebKitDownload, webkit_download, G_TYPE_OBJECT)
static void webkit_download_finalize (GObject *o) {
  WebKitDownload *d = WEBKIT_DOWNLOAD (o);
  g_clear_object (&d->req); g_clear_object (&d->resp); g_free (d->dest);
  G_OBJECT_CLASS (webkit_download_parent_class)->finalize (o);
}
static void webkit_download_get_property (GObject *o, guint id, GValue *v, GParamSpec *ps) {
  (void)ps;
  WebKitDownload *d = WEBKIT_DOWNLOAD (o);
  if (id == 1) g_value_set_double (v, d->progress);
  else if (id == 2) g_value_set_object (v, d->resp);
}
static void webkit_download_class_init (WebKitDownloadClass *k) {
  GObjectClass *oc = G_OBJECT_CLASS (k);
  oc->finalize = webkit_download_finalize;
  oc->get_property = webkit_download_get_property;
  g_signal_new ("decide-destination", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_BOOLEAN, 1, G_TYPE_STRING);
  g_signal_new ("created-destination", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_STRING);
  g_signal_new ("received-data", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT64);
  g_signal_new ("finished", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("failed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_ERROR);
  g_object_class_install_property (oc, 1, g_param_spec_double ("estimated-progress", "estimated-progress", "progress", 0.0, 1.0, 0.0, G_PARAM_READABLE));
  g_object_class_install_property (oc, 2, g_param_spec_object ("response", "response", "response", WEBKIT_TYPE_URI_RESPONSE, G_PARAM_READABLE));
}
static void webkit_download_init (WebKitDownload *s) { (void)s; }
WebKitDownload *win_webkit_download_new_internal (const char *uri) {
  WebKitDownload *d = g_object_new (WEBKIT_TYPE_DOWNLOAD, NULL);
  d->req = webkit_uri_request_new (uri);
  d->resp = win_webkit_uri_response_new_internal (uri, "application/octet-stream", 200, 0, NULL);
  return d;
}
WebKitURIRequest *webkit_download_get_request (WebKitDownload *d) { return d ? d->req : NULL; }
WebKitURIResponse *webkit_download_get_response (WebKitDownload *d) { return d ? d->resp : NULL; }
const char *webkit_download_get_destination (WebKitDownload *d) { return d ? d->dest : NULL; }
void webkit_download_set_destination (WebKitDownload *d, const char *dest) { if (d) { g_free (d->dest); d->dest = g_strdup (dest); } }
gboolean webkit_download_get_allow_overwrite (WebKitDownload *d) { return d ? d->overwrite : FALSE; }
void webkit_download_set_allow_overwrite (WebKitDownload *d, gboolean a) { if (d) d->overwrite = a; }
double webkit_download_get_estimated_progress (WebKitDownload *d) { return d ? d->progress : 0.0; }
guint64 webkit_download_get_received_data_length (WebKitDownload *d) { return d ? d->received : 0; }
double webkit_download_get_elapsed_time (WebKitDownload *d) { (void)d; return 0.0; }
void webkit_download_cancel (WebKitDownload *d) { (void)d; }

/* ---------------- FindController ---------------- */
struct _WebKitFindController { GObject parent; WebKitWebView *view; char *text; guint32 opts; guint max_count; };
G_DEFINE_TYPE (WebKitFindController, webkit_find_controller, G_TYPE_OBJECT)
static void webkit_find_controller_finalize (GObject *o) { g_free (WEBKIT_FIND_CONTROLLER (o)->text); G_OBJECT_CLASS (webkit_find_controller_parent_class)->finalize (o); }
static void webkit_find_controller_get_property (GObject *o, guint id, GValue *v, GParamSpec *ps) {
  (void)ps;
  if (id == 1) g_value_set_string (v, WEBKIT_FIND_CONTROLLER (o)->text);
}
static void webkit_find_controller_class_init (WebKitFindControllerClass *k) {
  GObjectClass *oc = G_OBJECT_CLASS (k);
  oc->finalize = webkit_find_controller_finalize;
  oc->get_property = webkit_find_controller_get_property;
  g_signal_new ("found-text", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
  g_signal_new ("failed-to-find-text", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("counted-matches", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_UINT);
  g_object_class_install_property (oc, 1, g_param_spec_string ("text", "text", "text", NULL, G_PARAM_READABLE));
}
static void webkit_find_controller_init (WebKitFindController *s) { (void)s; }
WebKitFindController *win_webkit_find_controller_new_internal (WebKitWebView *view) {
  WebKitFindController *fc = g_object_new (WEBKIT_TYPE_FIND_CONTROLLER, NULL);
  fc->view = view;
  return fc;
}
static WKFindOptions to_wk_find_opts (guint32 o) {
  WKFindOptions wk = 0;
  if (o & WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE) wk |= kWKFindOptionsCaseInsensitive;
  if (o & WEBKIT_FIND_OPTIONS_BACKWARDS) wk |= kWKFindOptionsBackwards;
  if (o & WEBKIT_FIND_OPTIONS_WRAP_AROUND) wk |= kWKFindOptionsWrapAround;
  if (o & WEBKIT_FIND_OPTIONS_AT_WORD_STARTS) wk |= kWKFindOptionsAtWordStarts;
  return wk;
}
void webkit_find_controller_search (WebKitFindController *fc, const char *text, guint32 opts, guint max) {
  if (!fc) return;
  g_free (fc->text); fc->text = g_strdup (text);
  fc->opts = opts; fc->max_count = max;
  WKPageRef p = win_webkit_web_view_get_wk_page (fc->view);
  if (p && text) {
    WKStringRef s = WKStringCreateWithUTF8CString (text);
    WKPageFindString (p, s, to_wk_find_opts (opts), max);
    WKRelease (s);
  }
}
void webkit_find_controller_search_next (WebKitFindController *fc) {
  if (fc && fc->text) webkit_find_controller_search (fc, fc->text, fc->opts & ~WEBKIT_FIND_OPTIONS_BACKWARDS, fc->max_count);
}
void webkit_find_controller_search_previous (WebKitFindController *fc) {
  if (fc && fc->text) webkit_find_controller_search (fc, fc->text, fc->opts | WEBKIT_FIND_OPTIONS_BACKWARDS, fc->max_count);
}
void webkit_find_controller_search_finish (WebKitFindController *fc) {
  if (!fc) return;
  WKPageRef p = win_webkit_web_view_get_wk_page (fc->view);
  if (p) WKPageHideFindUI (p);
}
void webkit_find_controller_count_matches (WebKitFindController *fc, const char *text, guint32 opts, guint max) {
  if (!fc || !text) return;
  WKPageRef p = win_webkit_web_view_get_wk_page (fc->view);
  if (p) {
    WKStringRef s = WKStringCreateWithUTF8CString (text);
    WKPageCountStringMatches (p, s, to_wk_find_opts (opts), max);
    WKRelease (s);
  }
}
guint32 webkit_find_controller_get_options (WebKitFindController *fc) { return fc ? fc->opts : 0; }

/* ---------------- HitTestResult & ContextMenu ---------------- */
struct _WebKitHitTestResult { GObject parent; guint ctx; char *link_uri; char *link_title; char *link_label; char *image_uri; };
G_DEFINE_TYPE (WebKitHitTestResult, webkit_hit_test_result, G_TYPE_OBJECT)
static void webkit_hit_test_result_finalize (GObject *o) {
  WebKitHitTestResult *h = WEBKIT_HIT_TEST_RESULT (o);
  g_free (h->link_uri); g_free (h->link_title); g_free (h->link_label); g_free (h->image_uri);
  G_OBJECT_CLASS (webkit_hit_test_result_parent_class)->finalize (o);
}
static void webkit_hit_test_result_class_init (WebKitHitTestResultClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_hit_test_result_finalize; }
static void webkit_hit_test_result_init (WebKitHitTestResult *s) { (void)s; }
guint webkit_hit_test_result_get_context (WebKitHitTestResult *h) { return h ? h->ctx : 0; }
gboolean webkit_hit_test_result_context_is_link (WebKitHitTestResult *h) { return h && (h->ctx & WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK); }
gboolean webkit_hit_test_result_context_is_image (WebKitHitTestResult *h) { return h && (h->ctx & WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE); }
gboolean webkit_hit_test_result_context_is_media (WebKitHitTestResult *h) { return h && (h->ctx & WEBKIT_HIT_TEST_RESULT_CONTEXT_MEDIA); }
gboolean webkit_hit_test_result_context_is_editable (WebKitHitTestResult *h) { return h && (h->ctx & WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE); }
const char *webkit_hit_test_result_get_link_uri (WebKitHitTestResult *h) { return h ? h->link_uri : NULL; }
const char *webkit_hit_test_result_get_link_title (WebKitHitTestResult *h) { return h ? h->link_title : NULL; }
const char *webkit_hit_test_result_get_link_label (WebKitHitTestResult *h) { return h ? h->link_label : NULL; }
const char *webkit_hit_test_result_get_image_uri (WebKitHitTestResult *h) { return h ? h->image_uri : NULL; }

struct _WebKitContextMenuItem { GInitiallyUnowned parent; WebKitContextMenuAction action; };
G_DEFINE_TYPE (WebKitContextMenuItem, webkit_context_menu_item, G_TYPE_INITIALLY_UNOWNED)
static void webkit_context_menu_item_class_init (WebKitContextMenuItemClass *k) { (void)k; }
static void webkit_context_menu_item_init (WebKitContextMenuItem *s) { (void)s; }
WebKitContextMenuItem *webkit_context_menu_item_new_from_stock_action (WebKitContextMenuAction a) {
  WebKitContextMenuItem *it = g_object_new (WEBKIT_TYPE_CONTEXT_MENU_ITEM, NULL);
  it->action = a;
  return it;
}
WebKitContextMenuItem *webkit_context_menu_item_new_from_gaction (GAction *a, const char *l, GVariant *t) {
  (void)a; (void)l; (void)t;
  return g_object_new (WEBKIT_TYPE_CONTEXT_MENU_ITEM, NULL);
}
WebKitContextMenuItem *webkit_context_menu_item_new_with_submenu (const char *l, WebKitContextMenu *s) {
  (void)l; (void)s;
  return g_object_new (WEBKIT_TYPE_CONTEXT_MENU_ITEM, NULL);
}
WebKitContextMenuItem *webkit_context_menu_item_new_separator (void) { return g_object_new (WEBKIT_TYPE_CONTEXT_MENU_ITEM, NULL); }
WebKitContextMenuAction webkit_context_menu_item_get_stock_action (WebKitContextMenuItem *it) { return it ? it->action : WEBKIT_CONTEXT_MENU_ACTION_NO_ACTION; }

struct _WebKitContextMenu { GObject parent; GList *items; GVariant *user_data; };
G_DEFINE_TYPE (WebKitContextMenu, webkit_context_menu, G_TYPE_OBJECT)
static void webkit_context_menu_finalize (GObject *o) {
  WebKitContextMenu *m = WEBKIT_CONTEXT_MENU (o);
  g_list_free_full (m->items, g_object_unref);
  g_clear_pointer (&m->user_data, g_variant_unref);
  G_OBJECT_CLASS (webkit_context_menu_parent_class)->finalize (o);
}
static void webkit_context_menu_class_init (WebKitContextMenuClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_context_menu_finalize; }
static void webkit_context_menu_init (WebKitContextMenu *s) { (void)s; }
WebKitContextMenu *webkit_context_menu_new_with_items (GList *items) {
  WebKitContextMenu *m = g_object_new (WEBKIT_TYPE_CONTEXT_MENU, NULL);
  m->items = g_list_copy (items);
  return m;
}
void webkit_context_menu_append (WebKitContextMenu *m, WebKitContextMenuItem *it) {
  if (m && it) m->items = g_list_append (m->items, g_object_ref_sink (it));
}
GList *webkit_context_menu_get_items (WebKitContextMenu *m) { return m ? m->items : NULL; }
void webkit_context_menu_remove_all (WebKitContextMenu *m) {
  if (m) { g_list_free_full (m->items, g_object_unref); m->items = NULL; }
}
void webkit_context_menu_set_user_data (WebKitContextMenu *m, GVariant *d) {
  if (!m) return;
  g_clear_pointer (&m->user_data, g_variant_unref);
  if (d) m->user_data = g_variant_ref_sink (d);
}
GVariant *webkit_context_menu_get_user_data (WebKitContextMenu *m) { return m ? m->user_data : NULL; }
GdkEvent *webkit_context_menu_get_event (WebKitContextMenu *m) { (void)m; return NULL; }

/* ---------------- Permissions, Auth, FileChooser, EditorState, WindowProps, Inspector, Print, WebResource ---------------- */
G_DEFINE_INTERFACE (WebKitPermissionRequest, webkit_permission_request, G_TYPE_OBJECT)
static void webkit_permission_request_default_init (WebKitPermissionRequestInterface *i) { (void)i; }
void webkit_permission_request_allow (WebKitPermissionRequest *r) {
  if (WEBKIT_IS_PERMISSION_REQUEST (r) && WEBKIT_PERMISSION_REQUEST_GET_IFACE (r)->allow)
    WEBKIT_PERMISSION_REQUEST_GET_IFACE (r)->allow (r);
}
void webkit_permission_request_deny (WebKitPermissionRequest *r) {
  if (WEBKIT_IS_PERMISSION_REQUEST (r) && WEBKIT_PERMISSION_REQUEST_GET_IFACE (r)->deny)
    WEBKIT_PERMISSION_REQUEST_GET_IFACE (r)->deny (r);
}

#define DEFINE_PERM_REQ(Camel, snake) \
struct _##Camel { GObject parent; }; \
static void snake##_perm_iface (WebKitPermissionRequestInterface *i) { (void)i; } \
G_DEFINE_TYPE_WITH_CODE (Camel, snake, G_TYPE_OBJECT, G_IMPLEMENT_INTERFACE (WEBKIT_TYPE_PERMISSION_REQUEST, snake##_perm_iface)) \
static void snake##_class_init (Camel##Class *k) { (void)k; } \
static void snake##_init (Camel *s) { (void)s; }

DEFINE_PERM_REQ (WebKitUserMediaPermissionRequest, webkit_user_media_permission_request)
DEFINE_PERM_REQ (WebKitGeolocationPermissionRequest, webkit_geolocation_permission_request)
DEFINE_PERM_REQ (WebKitNotificationPermissionRequest, webkit_notification_permission_request)
DEFINE_PERM_REQ (WebKitClipboardPermissionRequest, webkit_clipboard_permission_request)
DEFINE_PERM_REQ (WebKitWebsiteDataAccessPermissionRequest, webkit_website_data_access_permission_request)

gboolean webkit_user_media_permission_is_for_audio_device (WebKitUserMediaPermissionRequest *r) { (void)r; return FALSE; }
gboolean webkit_user_media_permission_is_for_video_device (WebKitUserMediaPermissionRequest *r) { (void)r; return FALSE; }
gboolean webkit_user_media_permission_is_for_display_device (WebKitUserMediaPermissionRequest *r) { (void)r; return FALSE; }
const char *webkit_website_data_access_permission_request_get_requesting_domain (WebKitWebsiteDataAccessPermissionRequest *r) { (void)r; return ""; }
const char *webkit_website_data_access_permission_request_get_current_domain (WebKitWebsiteDataAccessPermissionRequest *r) { (void)r; return ""; }
GType webkit_permission_state_query_get_type (void) { static GType t = 0; if (!t) t = g_pointer_type_register_static ("WebKitPermissionStateQuery"); return t; }
const char *webkit_permission_state_query_get_name (WebKitPermissionStateQuery *q) { (void)q; return ""; }
WebKitSecurityOrigin *webkit_permission_state_query_get_security_origin (WebKitPermissionStateQuery *q) { (void)q; return NULL; }
void webkit_permission_state_query_finish (WebKitPermissionStateQuery *q, WebKitPermissionState s) { (void)q; (void)s; }

struct _WebKitCredential { char *u; char *p; WebKitCredentialPersistence pers; };
struct _WebKitAuthenticationRequest { GObject parent; };
G_DEFINE_TYPE (WebKitAuthenticationRequest, webkit_authentication_request, G_TYPE_OBJECT)
static void webkit_authentication_request_class_init (WebKitAuthenticationRequestClass *k) {
  g_signal_new ("cancelled", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}
static void webkit_authentication_request_init (WebKitAuthenticationRequest *s) { (void)s; }
const char *webkit_authentication_request_get_host (WebKitAuthenticationRequest *r) { (void)r; return ""; }
guint webkit_authentication_request_get_port (WebKitAuthenticationRequest *r) { (void)r; return 0; }
const char *webkit_authentication_request_get_realm (WebKitAuthenticationRequest *r) { (void)r; return ""; }
WebKitAuthenticationScheme webkit_authentication_request_get_scheme (WebKitAuthenticationRequest *r) { (void)r; return WEBKIT_AUTHENTICATION_SCHEME_DEFAULT; }
WebKitSecurityOrigin *webkit_authentication_request_get_security_origin (WebKitAuthenticationRequest *r) { (void)r; return NULL; }
void webkit_authentication_request_set_can_save_credentials (WebKitAuthenticationRequest *r, gboolean e) { (void)r; (void)e; }
void webkit_authentication_request_authenticate (WebKitAuthenticationRequest *r, WebKitCredential *c) { (void)r; (void)c; }
WebKitCredential *webkit_credential_new (const char *u, const char *p, WebKitCredentialPersistence pers) {
  WebKitCredential *c = g_new0 (WebKitCredential, 1); c->u = g_strdup (u); c->p = g_strdup (p); c->pers = pers; return c;
}
WebKitCredential *webkit_credential_new_for_certificate (GTlsCertificate *cert, WebKitCredentialPersistence pers) { (void)cert; return webkit_credential_new ("", "", pers); }
WebKitCredential *webkit_credential_new_for_certificate_pin (const char *pin, WebKitCredentialPersistence pers) { return webkit_credential_new ("", pin, pers); }
void webkit_credential_free (WebKitCredential *c) { if (c) { g_free (c->u); g_free (c->p); g_free (c); } }
const char *webkit_credential_get_username (WebKitCredential *c) { return c ? c->u : ""; }
const char *webkit_credential_get_password (WebKitCredential *c) { return c ? c->p : ""; }
WebKitCredentialPersistence webkit_credential_get_persistence (WebKitCredential *c) { return c ? c->pers : WEBKIT_CREDENTIAL_PERSISTENCE_NONE; }

WebKitScriptDialogType webkit_script_dialog_get_dialog_type (WebKitScriptDialog *d) { (void)d; return WEBKIT_SCRIPT_DIALOG_ALERT; }
void webkit_script_dialog_confirm_set_confirmed (WebKitScriptDialog *d, gboolean c) { (void)d; (void)c; }

struct _WebKitFileChooserRequest { GObject parent; };
G_DEFINE_TYPE (WebKitFileChooserRequest, webkit_file_chooser_request, G_TYPE_OBJECT)
static void webkit_file_chooser_request_class_init (WebKitFileChooserRequestClass *k) { (void)k; }
static void webkit_file_chooser_request_init (WebKitFileChooserRequest *s) { (void)s; }
const char * const *webkit_file_chooser_request_get_mime_types (WebKitFileChooserRequest *r) { (void)r; return NULL; }
gboolean webkit_file_chooser_request_get_select_multiple (WebKitFileChooserRequest *r) { (void)r; return FALSE; }
void webkit_file_chooser_request_select_files (WebKitFileChooserRequest *r, const char * const *f) { (void)r; (void)f; }
void webkit_file_chooser_request_cancel (WebKitFileChooserRequest *r) { (void)r; }

struct _WebKitNotification { GObject parent; };
G_DEFINE_TYPE (WebKitNotification, webkit_notification, G_TYPE_OBJECT)
static void webkit_notification_class_init (WebKitNotificationClass *k) {
  g_signal_new ("clicked", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
}
static void webkit_notification_init (WebKitNotification *s) { (void)s; }

struct _WebKitEditorState { GObject parent; };
G_DEFINE_TYPE (WebKitEditorState, webkit_editor_state, G_TYPE_OBJECT)
static void webkit_editor_state_get_property (GObject *o, guint id, GValue *v, GParamSpec *ps) {
  (void)o; (void)ps;
  if (id == 1) g_value_set_uint (v, 0);
}
static void webkit_editor_state_class_init (WebKitEditorStateClass *k) {
  GObjectClass *oc = G_OBJECT_CLASS (k);
  oc->get_property = webkit_editor_state_get_property;
  g_signal_new ("notify", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_object_class_install_property (oc, 1, g_param_spec_uint ("typing-attributes", "ta", "ta", 0, G_MAXUINT, 0, G_PARAM_READABLE));
}
static void webkit_editor_state_init (WebKitEditorState *s) { (void)s; }
WebKitEditorState *win_webkit_editor_state_new_internal (void) { return g_object_new (WEBKIT_TYPE_EDITOR_STATE, NULL); }
gboolean webkit_editor_state_is_cut_available (WebKitEditorState *e) { (void)e; return TRUE; }
gboolean webkit_editor_state_is_copy_available (WebKitEditorState *e) { (void)e; return TRUE; }
gboolean webkit_editor_state_is_paste_available (WebKitEditorState *e) { (void)e; return TRUE; }
gboolean webkit_editor_state_is_undo_available (WebKitEditorState *e) { (void)e; return TRUE; }
gboolean webkit_editor_state_is_redo_available (WebKitEditorState *e) { (void)e; return TRUE; }

struct _WebKitWindowProperties { GObject parent; };
G_DEFINE_TYPE (WebKitWindowProperties, webkit_window_properties, G_TYPE_OBJECT)
static void webkit_window_properties_class_init (WebKitWindowPropertiesClass *k) { (void)k; }
static void webkit_window_properties_init (WebKitWindowProperties *s) { (void)s; }
WebKitWindowProperties *win_webkit_window_properties_new_internal (void) { return g_object_new (WEBKIT_TYPE_WINDOW_PROPERTIES, NULL); }
void webkit_window_properties_get_geometry (WebKitWindowProperties *w, GdkRectangle *g) { (void)w; if (g) { g->x = 0; g->y = 0; g->width = 1024; g->height = 768; } }
gboolean webkit_window_properties_get_toolbar_visible (WebKitWindowProperties *w) { (void)w; return TRUE; }
gboolean webkit_window_properties_get_menubar_visible (WebKitWindowProperties *w) { (void)w; return TRUE; }
gboolean webkit_window_properties_get_locationbar_visible (WebKitWindowProperties *w) { (void)w; return TRUE; }
gboolean webkit_window_properties_get_resizable (WebKitWindowProperties *w) { (void)w; return TRUE; }

struct _WebKitWebInspector { GObject parent; WebKitWebView *view; };
G_DEFINE_TYPE (WebKitWebInspector, webkit_web_inspector, G_TYPE_OBJECT)
static void webkit_web_inspector_class_init (WebKitWebInspectorClass *k) {
  g_signal_new ("attach", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_BOOLEAN, 0);
  g_signal_new ("detach", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_BOOLEAN, 0);
  g_signal_new ("closed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("open-window", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_BOOLEAN, 0);
}
static void webkit_web_inspector_init (WebKitWebInspector *s) { (void)s; }
WebKitWebInspector *win_webkit_web_inspector_new_internal (WebKitWebView *view) {
  WebKitWebInspector *i = g_object_new (WEBKIT_TYPE_WEB_INSPECTOR, NULL);
  i->view = view;
  return i;
}
void webkit_web_inspector_show (WebKitWebInspector *i) {
  WKPageRef p = i ? win_webkit_web_view_get_wk_page (i->view) : NULL;
  if (p) {
    WKInspectorRef insp = WKPageGetInspector (p);
    if (insp) WKInspectorShow (insp);
  }
}
void webkit_web_inspector_close (WebKitWebInspector *i) {
  WKPageRef p = i ? win_webkit_web_view_get_wk_page (i->view) : NULL;
  if (p) {
    WKInspectorRef insp = WKPageGetInspector (p);
    if (insp) WKInspectorClose (insp);
  }
}

struct _WebKitPrintOperation { GObject parent; GtkPrintSettings *ps; GtkPageSetup *setup; };
G_DEFINE_TYPE (WebKitPrintOperation, webkit_print_operation, G_TYPE_OBJECT)
static void webkit_print_operation_finalize (GObject *o) {
  WebKitPrintOperation *p = WEBKIT_PRINT_OPERATION (o);
  g_clear_object (&p->ps); g_clear_object (&p->setup);
  G_OBJECT_CLASS (webkit_print_operation_parent_class)->finalize (o);
}
static void webkit_print_operation_class_init (WebKitPrintOperationClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_print_operation_finalize; }
static void webkit_print_operation_init (WebKitPrintOperation *s) { s->ps = gtk_print_settings_new (); s->setup = gtk_page_setup_new (); }
WebKitPrintOperation *webkit_print_operation_new (WebKitWebView *v) { (void)v; return g_object_new (WEBKIT_TYPE_PRINT_OPERATION, NULL); }
GtkPrintSettings *webkit_print_operation_get_print_settings (WebKitPrintOperation *p) { return p ? p->ps : NULL; }
void webkit_print_operation_set_print_settings (WebKitPrintOperation *p, GtkPrintSettings *ps) { if (p) g_set_object (&p->ps, ps); }
GtkPageSetup *webkit_print_operation_get_page_setup (WebKitPrintOperation *p) { return p ? p->setup : NULL; }
void webkit_print_operation_set_page_setup (WebKitPrintOperation *p, GtkPageSetup *ps) { if (p) g_set_object (&p->setup, ps); }
WebKitPrintOperationResponse webkit_print_operation_run_dialog (WebKitPrintOperation *p, GtkWindow *parent) {
  (void)p; (void)parent; return WEBKIT_PRINT_OPERATION_RESPONSE_CANCEL;
}

struct _WebKitWebResource { GObject parent; char *uri; WebKitURIResponse *resp; };
G_DEFINE_TYPE (WebKitWebResource, webkit_web_resource, G_TYPE_OBJECT)
static void webkit_web_resource_finalize (GObject *o) {
  WebKitWebResource *r = WEBKIT_WEB_RESOURCE (o);
  g_free (r->uri); g_clear_object (&r->resp);
  G_OBJECT_CLASS (webkit_web_resource_parent_class)->finalize (o);
}
static void webkit_web_resource_class_init (WebKitWebResourceClass *k) {
  G_OBJECT_CLASS (k)->finalize = webkit_web_resource_finalize;
  g_signal_new ("finished", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 0);
  g_signal_new ("failed", G_TYPE_FROM_CLASS (k), G_SIGNAL_RUN_LAST, 0, NULL, NULL, NULL, G_TYPE_NONE, 1, G_TYPE_ERROR);
}
static void webkit_web_resource_init (WebKitWebResource *s) { (void)s; }
WebKitWebResource *win_webkit_web_resource_new_internal (const char *uri) {
  WebKitWebResource *r = g_object_new (WEBKIT_TYPE_WEB_RESOURCE, NULL);
  r->uri = g_strdup (uri);
  r->resp = win_webkit_uri_response_new_internal (uri, "text/html", 200, 0, NULL);
  return r;
}
const char *webkit_web_resource_get_uri (WebKitWebResource *r) { return r ? r->uri : ""; }
WebKitURIResponse *webkit_web_resource_get_response (WebKitWebResource *r) { return r ? r->resp : NULL; }
void webkit_web_resource_get_data (WebKitWebResource *r, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)c;
  GTask *t = g_task_new (r, NULL, cb, d);
  g_task_return_pointer (t, g_strdup (""), g_free);
  g_object_unref (t);
}
guchar *webkit_web_resource_get_data_finish (WebKitWebResource *r, GAsyncResult *res, gsize *len, GError **e) {
  (void)r; if (len) *len = 0; return g_task_propagate_pointer (G_TASK (res), e);
}

/* ---------------- WebsiteData, ITP, WebExtension, UserMessage, ProcessExtension ---------------- */
struct _WebKitWebsiteData { int rc; char *name; WebKitWebsiteDataTypes types; };
GType webkit_website_data_get_type (void) {
  static GType t = 0;
  if (!t) t = g_boxed_type_register_static ("WebKitWebsiteData", (GBoxedCopyFunc)webkit_website_data_ref, (GBoxedFreeFunc)webkit_website_data_unref);
  return t;
}
const char *webkit_website_data_get_name (WebKitWebsiteData *d) { return d ? d->name : ""; }
WebKitWebsiteDataTypes webkit_website_data_get_types (WebKitWebsiteData *d) { return d ? d->types : 0; }
WebKitWebsiteData *webkit_website_data_ref (WebKitWebsiteData *d) { if (d) g_atomic_int_inc (&d->rc); return d; }
void webkit_website_data_unref (WebKitWebsiteData *d) { if (d && g_atomic_int_dec_and_test (&d->rc)) { g_free (d->name); g_free (d); } }

const char *webkit_itp_third_party_get_domain (WebKitITPThirdParty *p) { (void)p; return ""; }
void webkit_itp_third_party_unref (WebKitITPThirdParty *p) { (void)p; }
GList *webkit_itp_third_party_get_first_parties (WebKitITPThirdParty *p) { (void)p; return NULL; }
const char *webkit_itp_first_party_get_domain (WebKitITPFirstParty *p) { (void)p; return ""; }
gboolean webkit_itp_first_party_get_website_data_access_allowed (WebKitITPFirstParty *p) { (void)p; return FALSE; }

struct _WebKitApplicationInfo { int rc; };
WebKitApplicationInfo *webkit_application_info_new (void) { return g_new0 (WebKitApplicationInfo, 1); }
void webkit_application_info_set_name (WebKitApplicationInfo *i, const char *n) { (void)i; (void)n; }
void webkit_application_info_set_version (WebKitApplicationInfo *i, guint64 a, guint64 b, guint64 c) { (void)i; (void)a; (void)b; (void)c; }
void webkit_application_info_unref (WebKitApplicationInfo *i) { g_free (i); }
void webkit_automation_session_set_application_info (WebKitAutomationSession *s, WebKitApplicationInfo *i) { (void)s; (void)i; }

struct _WebKitUserMessage { GInitiallyUnowned parent; char *name; GVariant *params; };
G_DEFINE_TYPE (WebKitUserMessage, webkit_user_message, G_TYPE_INITIALLY_UNOWNED)
static void webkit_user_message_finalize (GObject *o) {
  WebKitUserMessage *m = WEBKIT_USER_MESSAGE (o);
  g_free (m->name); g_clear_pointer (&m->params, g_variant_unref);
  G_OBJECT_CLASS (webkit_user_message_parent_class)->finalize (o);
}
static void webkit_user_message_class_init (WebKitUserMessageClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_user_message_finalize; }
static void webkit_user_message_init (WebKitUserMessage *s) { (void)s; }
WebKitUserMessage *webkit_user_message_new (const char *name, GVariant *params) {
  WebKitUserMessage *m = g_object_new (WEBKIT_TYPE_USER_MESSAGE, NULL);
  m->name = g_strdup (name);
  if (params) m->params = g_variant_ref_sink (params);
  return m;
}
const char *webkit_user_message_get_name (WebKitUserMessage *m) { return m ? m->name : ""; }
GVariant *webkit_user_message_get_parameters (WebKitUserMessage *m) { return m ? m->params : NULL; }
void webkit_user_message_send_reply (WebKitUserMessage *m, WebKitUserMessage *r) { (void)m; (void)r; }

struct _WebKitWebExtension { GObject parent; };
G_DEFINE_TYPE (WebKitWebExtension, webkit_web_extension, G_TYPE_OBJECT)
static void webkit_web_extension_class_init (WebKitWebExtensionClass *k) { (void)k; }
static void webkit_web_extension_init (WebKitWebExtension *s) { (void)s; }
WebKitWebExtension *webkit_web_extension_new (const char *base_location, GError **error) { (void)base_location; (void)error; return g_object_new (WEBKIT_TYPE_WEB_EXTENSION, NULL); }
const char *webkit_web_extension_get_display_name (WebKitWebExtension *e) { (void)e; return ""; }
const char *webkit_web_extension_get_display_short_name (WebKitWebExtension *e) { (void)e; return ""; }
const char *webkit_web_extension_get_display_version (WebKitWebExtension *e) { (void)e; return ""; }
const char *webkit_web_extension_get_display_description (WebKitWebExtension *e) { (void)e; return ""; }
GIcon *webkit_web_extension_get_icon (WebKitWebExtension *e, gsize w, gsize h) { (void)e; (void)w; (void)h; return NULL; }
gboolean webkit_web_extension_get_has_background_content (WebKitWebExtension *e) { (void)e; return FALSE; }
gboolean webkit_web_extension_get_has_persistent_background_content (WebKitWebExtension *e) { (void)e; return FALSE; }
GType webkit_web_extension_match_pattern_get_type (void) { static GType t = 0; if (!t) t = g_pointer_type_register_static ("WebKitWebExtensionMatchPattern"); return t; }
WebKitWebExtensionMatchPattern *webkit_web_extension_match_pattern_new_with_string (const char *p, GError **e) { (void)p; (void)e; return (WebKitWebExtensionMatchPattern *)g_strdup (p ? p : ""); }
gboolean webkit_web_extension_match_pattern_matches_url (WebKitWebExtensionMatchPattern *p, const char *u, WebKitWebExtensionMatchPatternOptions o) { (void)p; (void)u; (void)o; return FALSE; }
void webkit_web_extension_match_pattern_register_custom_url_scheme (const char *s) { (void)s; }
void webkit_web_extension_match_pattern_unref (WebKitWebExtensionMatchPattern *p) { g_free (p); }

/* WebProcessExtension types */
struct _WebKitWebProcessExtension { GObject parent; };
G_DEFINE_TYPE (WebKitWebProcessExtension, webkit_web_process_extension, G_TYPE_OBJECT)
static void webkit_web_process_extension_class_init (WebKitWebProcessExtensionClass *k) { (void)k; }
static void webkit_web_process_extension_init (WebKitWebProcessExtension *s) { (void)s; }
WebKitWebPage *webkit_web_process_extension_get_page (WebKitWebProcessExtension *e, guint64 id) { (void)e; (void)id; return NULL; }

struct _WebKitWebPage { GObject parent; };
G_DEFINE_TYPE (WebKitWebPage, webkit_web_page, G_TYPE_OBJECT)
static void webkit_web_page_class_init (WebKitWebPageClass *k) { (void)k; }
static void webkit_web_page_init (WebKitWebPage *s) { (void)s; }
guint64 webkit_web_page_get_id (WebKitWebPage *p) { (void)p; return 1; }
const char *webkit_web_page_get_uri (WebKitWebPage *p) { (void)p; return ""; }
WebKitFrame *webkit_web_page_get_main_frame (WebKitWebPage *p) { (void)p; return NULL; }
WebKitWebFormManager *webkit_web_page_get_form_manager (WebKitWebPage *p, WebKitScriptWorld *w) { (void)p; (void)w; return NULL; }
void webkit_web_page_send_message_to_view (WebKitWebPage *p, WebKitUserMessage *m, GCancellable *c, GAsyncReadyCallback cb, gpointer d) {
  (void)m; (void)c;
  GTask *t = g_task_new (p, NULL, cb, d);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
WebKitUserMessage *webkit_web_page_send_message_to_view_finish (WebKitWebPage *p, GAsyncResult *res, GError **e) {
  (void)p; return g_task_propagate_pointer (G_TASK (res), e);
}

struct _WebKitFrame { GObject parent; };
G_DEFINE_TYPE (WebKitFrame, webkit_frame, G_TYPE_OBJECT)
static void webkit_frame_class_init (WebKitFrameClass *k) { (void)k; }
static void webkit_frame_init (WebKitFrame *s) { (void)s; }
guint64 webkit_frame_get_id (WebKitFrame *f) { (void)f; return 1; }
JSCContext *webkit_frame_get_js_context (WebKitFrame *f) { (void)f; return jsc_context_new (); }
JSCContext *webkit_frame_get_js_context_for_script_world (WebKitFrame *f, WebKitScriptWorld *w) { (void)f; (void)w; return jsc_context_new (); }

struct _WebKitScriptWorld { GObject parent; char *name; };
G_DEFINE_TYPE (WebKitScriptWorld, webkit_script_world, G_TYPE_OBJECT)
static void webkit_script_world_finalize (GObject *o) { g_free (WEBKIT_SCRIPT_WORLD (o)->name); G_OBJECT_CLASS (webkit_script_world_parent_class)->finalize (o); }
static void webkit_script_world_class_init (WebKitScriptWorldClass *k) { G_OBJECT_CLASS (k)->finalize = webkit_script_world_finalize; }
static void webkit_script_world_init (WebKitScriptWorld *s) { (void)s; }
WebKitScriptWorld *webkit_script_world_get_default (void) { static WebKitScriptWorld *d = NULL; if (!d) d = g_object_new (WEBKIT_TYPE_SCRIPT_WORLD, NULL); return d; }
WebKitScriptWorld *webkit_script_world_new_with_name (const char *n) { WebKitScriptWorld *w = g_object_new (WEBKIT_TYPE_SCRIPT_WORLD, NULL); w->name = g_strdup (n); return w; }
const char *webkit_script_world_get_name (WebKitScriptWorld *w) { return w && w->name ? w->name : "default"; }

struct _WebKitWebFormManager { GObject parent; };
G_DEFINE_TYPE (WebKitWebFormManager, webkit_web_form_manager, G_TYPE_OBJECT)
static void webkit_web_form_manager_class_init (WebKitWebFormManagerClass *k) { (void)k; }
static void webkit_web_form_manager_init (WebKitWebFormManager *s) { (void)s; }
void webkit_web_form_manager_input_element_auto_fill (JSCValue *e, const char *v) { (void)e; (void)v; }
gboolean webkit_web_form_manager_input_element_is_user_edited (JSCValue *e) { (void)e; return FALSE; }

struct _WebKitWebHitTestResult { GObject parent; };
G_DEFINE_TYPE (WebKitWebHitTestResult, webkit_web_hit_test_result, G_TYPE_OBJECT)
static void webkit_web_hit_test_result_class_init (WebKitWebHitTestResultClass *k) { (void)k; }
static void webkit_web_hit_test_result_init (WebKitWebHitTestResult *s) { (void)s; }
JSCValue *webkit_web_hit_test_result_get_js_node (WebKitWebHitTestResult *h, WebKitScriptWorld *w) { (void)h; (void)w; return NULL; }
