#pragma once
#include <windows.h>
#include "webkit/webkit.h"
#include "webkit/webkit-web-process-extension.h"
#include <gdk/win32/gdkwin32.h>
#include <WebKit/WebKit2_C.h>
#include <WebKit/WKView.h>
#include <WebKit/WKContextConfigurationRef.h>
#include <WebKit/WKPageConfigurationRef.h>
#include <WebKit/WKWebsiteDataStoreRef.h>
#include <WebKit/WKFindOptions.h>
#include <WebKit/WKInspector.h>

typedef struct {
  char *scheme;
  WebKitURISchemeRequestCallback callback;
  gpointer user_data;
  GDestroyNotify destroy;
} WinUriSchemeEntry;

typedef struct _WebKitWebContextPrivate {
  WKContextRef wk_context;
  GHashTable *uri_schemes; /* scheme -> WinUriSchemeEntry* */
  WebKitSecurityManager *security_manager;
} WebKitWebContextPrivate;

WebKitWebContext *win_webkit_web_context_get_default (void);
WKContextRef win_webkit_web_context_get_wk (WebKitWebContext *ctx);
WinUriSchemeEntry *win_webkit_web_context_lookup_scheme (WebKitWebContext *ctx, const char *uri);

/* Internal helpers to construct objects */
WebKitURIRequest *win_webkit_uri_request_new_internal (const char *uri);
WebKitURIResponse *win_webkit_uri_response_new_internal (const char *uri, const char *mime, guint status, guint64 len, const char *suggested);
WebKitURISchemeRequest *win_webkit_uri_scheme_request_new_internal (WebKitWebView *view, const char *uri);
WebKitNavigationAction *win_webkit_navigation_action_new_internal (const char *uri, WebKitNavigationType type, guint btn, guint mods, gboolean gesture);
WebKitNavigationPolicyDecision *win_webkit_navigation_policy_decision_new_internal (WebKitNavigationAction *action);
WebKitFindController *win_webkit_find_controller_new_internal (WebKitWebView *view);
WebKitBackForwardList *win_webkit_back_forward_list_new_internal (WebKitWebView *view);
WebKitEditorState *win_webkit_editor_state_new_internal (void);
WebKitWindowProperties *win_webkit_window_properties_new_internal (void);
WebKitWebInspector *win_webkit_web_inspector_new_internal (WebKitWebView *view);
WebKitWebResource *win_webkit_web_resource_new_internal (const char *uri);
WebKitDownload *win_webkit_download_new_internal (const char *uri);

WKPageRef win_webkit_web_view_get_wk_page (WebKitWebView *view);
void win_webkit_web_view_finish_scheme_request (WebKitWebView *view, const char *uri, GBytes *bytes, const char *content_type);
