#pragma once
#include <gtk/gtk.h>
#include <libsoup/soup.h>
#include <jsc/jsc.h>

G_BEGIN_DECLS

#define WEBKIT_MAJOR_VERSION 2
#define WEBKIT_MINOR_VERSION 46
#define WEBKIT_MICRO_VERSION 0

guint webkit_get_major_version (void);
guint webkit_get_minor_version (void);
guint webkit_get_micro_version (void);

/* Forward declarations */
typedef struct _WebKitWebView WebKitWebView;
typedef struct _WebKitWebContext WebKitWebContext;
typedef struct _WebKitNetworkSession WebKitNetworkSession;
typedef struct _WebKitWebsiteDataManager WebKitWebsiteDataManager;
typedef struct _WebKitCookieManager WebKitCookieManager;
typedef struct _WebKitFaviconDatabase WebKitFaviconDatabase;
typedef struct _WebKitSecurityManager WebKitSecurityManager;
typedef struct _WebKitSecurityOrigin WebKitSecurityOrigin;
typedef struct _WebKitSettings WebKitSettings;
typedef struct _WebKitUserContentManager WebKitUserContentManager;
typedef struct _WebKitUserContentFilterStore WebKitUserContentFilterStore;
typedef struct _WebKitUserContentFilter WebKitUserContentFilter;
typedef struct _WebKitUserScript WebKitUserScript;
typedef struct _WebKitUserStyleSheet WebKitUserStyleSheet;
typedef struct _WebKitDownload WebKitDownload;
typedef struct _WebKitURIRequest WebKitURIRequest;
typedef struct _WebKitURIResponse WebKitURIResponse;
typedef struct _WebKitURISchemeRequest WebKitURISchemeRequest;
typedef struct _WebKitPolicyDecision WebKitPolicyDecision;
typedef struct _WebKitNavigationPolicyDecision WebKitNavigationPolicyDecision;
typedef struct _WebKitResponsePolicyDecision WebKitResponsePolicyDecision;
typedef struct _WebKitNavigationAction WebKitNavigationAction;
typedef struct _WebKitWebsitePolicies WebKitWebsitePolicies;
typedef struct _WebKitBackForwardList WebKitBackForwardList;
typedef struct _WebKitBackForwardListItem WebKitBackForwardListItem;
typedef struct _WebKitFindController WebKitFindController;
typedef struct _WebKitHitTestResult WebKitHitTestResult;
typedef struct _WebKitContextMenu WebKitContextMenu;
typedef struct _WebKitContextMenuItem WebKitContextMenuItem;
typedef struct _WebKitPermissionRequest WebKitPermissionRequest;
typedef struct _WebKitUserMediaPermissionRequest WebKitUserMediaPermissionRequest;
typedef struct _WebKitGeolocationPermissionRequest WebKitGeolocationPermissionRequest;
typedef struct _WebKitNotificationPermissionRequest WebKitNotificationPermissionRequest;
typedef struct _WebKitClipboardPermissionRequest WebKitClipboardPermissionRequest;
typedef struct _WebKitWebsiteDataAccessPermissionRequest WebKitWebsiteDataAccessPermissionRequest;
typedef struct _WebKitPermissionStateQuery WebKitPermissionStateQuery;
typedef struct _WebKitAuthenticationRequest WebKitAuthenticationRequest;
typedef struct _WebKitCredential WebKitCredential;
typedef struct _WebKitScriptDialog WebKitScriptDialog;
typedef struct _WebKitFileChooserRequest WebKitFileChooserRequest;
typedef struct _WebKitNotification WebKitNotification;
typedef struct _WebKitEditorState WebKitEditorState;
typedef struct _WebKitWindowProperties WebKitWindowProperties;
typedef struct _WebKitWebInspector WebKitWebInspector;
typedef struct _WebKitPrintOperation WebKitPrintOperation;
typedef struct _WebKitWebResource WebKitWebResource;
typedef struct _WebKitWebViewSessionState WebKitWebViewSessionState;
typedef struct _WebKitWebsiteData WebKitWebsiteData;
typedef struct _WebKitITPThirdParty WebKitITPThirdParty;
typedef struct _WebKitITPFirstParty WebKitITPFirstParty;
typedef struct _WebKitFeature WebKitFeature;
typedef struct _WebKitFeatureList WebKitFeatureList;
typedef struct _WebKitApplicationInfo WebKitApplicationInfo;
typedef struct _WebKitAutomationSession WebKitAutomationSession;
typedef struct _WebKitWebExtension WebKitWebExtension;
typedef struct _WebKitWebExtensionMatchPattern WebKitWebExtensionMatchPattern;
typedef struct _WebKitUserMessage WebKitUserMessage;

/* Enums */
typedef enum {
  WEBKIT_LOAD_STARTED,
  WEBKIT_LOAD_REDIRECTED,
  WEBKIT_LOAD_COMMITTED,
  WEBKIT_LOAD_FINISHED
} WebKitLoadEvent;

typedef enum {
  WEBKIT_POLICY_DECISION_TYPE_NAVIGATION_ACTION,
  WEBKIT_POLICY_DECISION_TYPE_NEW_WINDOW_ACTION,
  WEBKIT_POLICY_DECISION_TYPE_RESPONSE
} WebKitPolicyDecisionType;

typedef enum {
  WEBKIT_NAVIGATION_TYPE_LINK_CLICKED,
  WEBKIT_NAVIGATION_TYPE_FORM_SUBMITTED,
  WEBKIT_NAVIGATION_TYPE_BACK_FORWARD,
  WEBKIT_NAVIGATION_TYPE_RELOAD,
  WEBKIT_NAVIGATION_TYPE_FORM_RESUBMITTED,
  WEBKIT_NAVIGATION_TYPE_OTHER
} WebKitNavigationType;

typedef enum {
  WEBKIT_FIND_OPTIONS_NONE = 0,
  WEBKIT_FIND_OPTIONS_CASE_INSENSITIVE = 1 << 0,
  WEBKIT_FIND_OPTIONS_AT_WORD_STARTS = 1 << 1,
  WEBKIT_FIND_OPTIONS_TREAT_MEDIAL_CAPITAL_AS_WORD_START = 1 << 2,
  WEBKIT_FIND_OPTIONS_BACKWARDS = 1 << 3,
  WEBKIT_FIND_OPTIONS_WRAP_AROUND = 1 << 4
} WebKitFindOptions;

typedef enum {
  WEBKIT_HIT_TEST_RESULT_CONTEXT_DOCUMENT = 1 << 1,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_LINK = 1 << 2,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_IMAGE = 1 << 3,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_MEDIA = 1 << 4,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_EDITABLE = 1 << 5,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_SCROLLBAR = 1 << 6,
  WEBKIT_HIT_TEST_RESULT_CONTEXT_SELECTION = 1 << 7
} WebKitHitTestResultContext;

typedef enum {
  WEBKIT_CONTEXT_MENU_ACTION_NO_ACTION = 0,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_LINK,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_LINK_IN_NEW_WINDOW,
  WEBKIT_CONTEXT_MENU_ACTION_DOWNLOAD_LINK_TO_DISK,
  WEBKIT_CONTEXT_MENU_ACTION_COPY_LINK_TO_CLIPBOARD,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_IMAGE_IN_NEW_WINDOW,
  WEBKIT_CONTEXT_MENU_ACTION_DOWNLOAD_IMAGE_TO_DISK,
  WEBKIT_CONTEXT_MENU_ACTION_COPY_IMAGE_TO_CLIPBOARD,
  WEBKIT_CONTEXT_MENU_ACTION_COPY_IMAGE_URL_TO_CLIPBOARD,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_FRAME_IN_NEW_WINDOW,
  WEBKIT_CONTEXT_MENU_ACTION_GO_BACK,
  WEBKIT_CONTEXT_MENU_ACTION_GO_FORWARD,
  WEBKIT_CONTEXT_MENU_ACTION_STOP,
  WEBKIT_CONTEXT_MENU_ACTION_RELOAD,
  WEBKIT_CONTEXT_MENU_ACTION_COPY,
  WEBKIT_CONTEXT_MENU_ACTION_CUT,
  WEBKIT_CONTEXT_MENU_ACTION_PASTE,
  WEBKIT_CONTEXT_MENU_ACTION_DELETE,
  WEBKIT_CONTEXT_MENU_ACTION_SELECT_ALL,
  WEBKIT_CONTEXT_MENU_ACTION_INPUT_METHODS,
  WEBKIT_CONTEXT_MENU_ACTION_UNICODE,
  WEBKIT_CONTEXT_MENU_ACTION_SPELLING_GUESS,
  WEBKIT_CONTEXT_MENU_ACTION_NO_GUESSES_FOUND,
  WEBKIT_CONTEXT_MENU_ACTION_IGNORE_SPELLING,
  WEBKIT_CONTEXT_MENU_ACTION_LEARN_SPELLING,
  WEBKIT_CONTEXT_MENU_ACTION_IGNORE_GRAMMAR,
  WEBKIT_CONTEXT_MENU_ACTION_FONT_MENU,
  WEBKIT_CONTEXT_MENU_ACTION_BOLD,
  WEBKIT_CONTEXT_MENU_ACTION_ITALIC,
  WEBKIT_CONTEXT_MENU_ACTION_UNDERLINE,
  WEBKIT_CONTEXT_MENU_ACTION_OUTLINE,
  WEBKIT_CONTEXT_MENU_ACTION_INSPECT_ELEMENT,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_VIDEO_IN_NEW_WINDOW,
  WEBKIT_CONTEXT_MENU_ACTION_OPEN_AUDIO_IN_NEW_WINDOW,
  WEBKIT_CONTEXT_MENU_ACTION_COPY_VIDEO_LINK_TO_CLIPBOARD,
  WEBKIT_CONTEXT_MENU_ACTION_COPY_AUDIO_LINK_TO_CLIPBOARD,
  WEBKIT_CONTEXT_MENU_ACTION_TOGGLE_MEDIA_CONTROLS,
  WEBKIT_CONTEXT_MENU_ACTION_TOGGLE_MEDIA_LOOP,
  WEBKIT_CONTEXT_MENU_ACTION_ENTER_VIDEO_FULLSCREEN,
  WEBKIT_CONTEXT_MENU_ACTION_MEDIA_PLAY,
  WEBKIT_CONTEXT_MENU_ACTION_MEDIA_PAUSE,
  WEBKIT_CONTEXT_MENU_ACTION_MEDIA_MUTE,
  WEBKIT_CONTEXT_MENU_ACTION_DOWNLOAD_VIDEO_TO_DISK,
  WEBKIT_CONTEXT_MENU_ACTION_DOWNLOAD_AUDIO_TO_DISK,
  WEBKIT_CONTEXT_MENU_ACTION_INSERT_EMOJI,
  WEBKIT_CONTEXT_MENU_ACTION_PASTE_AS_PLAIN_TEXT,
  WEBKIT_CONTEXT_MENU_ACTION_CUSTOM = 10000
} WebKitContextMenuAction;

typedef enum {
  WEBKIT_COOKIE_PERSISTENT_STORAGE_TEXT,
  WEBKIT_COOKIE_PERSISTENT_STORAGE_SQLITE
} WebKitCookiePersistentStorage;

typedef enum {
  WEBKIT_COOKIE_POLICY_ACCEPT_ALWAYS,
  WEBKIT_COOKIE_POLICY_ACCEPT_NEVER,
  WEBKIT_COOKIE_POLICY_ACCEPT_NO_THIRD_PARTY
} WebKitCookieAcceptPolicy;

typedef enum {
  WEBKIT_CREDENTIAL_PERSISTENCE_NONE,
  WEBKIT_CREDENTIAL_PERSISTENCE_FOR_SESSION,
  WEBKIT_CREDENTIAL_PERSISTENCE_PERMANENT
} WebKitCredentialPersistence;

typedef enum {
  WEBKIT_AUTHENTICATION_SCHEME_DEFAULT = 1,
  WEBKIT_AUTHENTICATION_SCHEME_HTTP_BASIC = 2,
  WEBKIT_AUTHENTICATION_SCHEME_HTTP_DIGEST = 3,
  WEBKIT_AUTHENTICATION_SCHEME_HTML_FORM = 4,
  WEBKIT_AUTHENTICATION_SCHEME_NTLM = 5,
  WEBKIT_AUTHENTICATION_SCHEME_NEGOTIATE = 6,
  WEBKIT_AUTHENTICATION_SCHEME_CLIENT_CERTIFICATE_REQUESTED = 7,
  WEBKIT_AUTHENTICATION_SCHEME_SERVER_TRUST_EVALUATION_REQUESTED = 8,
  WEBKIT_AUTHENTICATION_SCHEME_CLIENT_CERTIFICATE_PIN_REQUESTED = 9,
  WEBKIT_AUTHENTICATION_SCHEME_UNKNOWN = 100
} WebKitAuthenticationScheme;

typedef enum {
  WEBKIT_HARDWARE_ACCELERATION_POLICY_ALWAYS,
  WEBKIT_HARDWARE_ACCELERATION_POLICY_NEVER
} WebKitHardwareAccelerationPolicy;

typedef enum {
  WEBKIT_AUTOPLAY_ALLOW,
  WEBKIT_AUTOPLAY_ALLOW_WITHOUT_SOUND,
  WEBKIT_AUTOPLAY_DENY
} WebKitAutoplayPolicy;

typedef enum {
  WEBKIT_USER_CONTENT_INJECT_ALL_FRAMES,
  WEBKIT_USER_CONTENT_INJECT_TOP_FRAME
} WebKitUserContentInjectedFrames;

typedef enum {
  WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_START,
  WEBKIT_USER_SCRIPT_INJECT_AT_DOCUMENT_END
} WebKitUserScriptInjectionTime;

typedef enum {
  WEBKIT_USER_STYLE_LEVEL_USER,
  WEBKIT_USER_STYLE_LEVEL_AUTHOR
} WebKitUserContentLevel;

typedef enum {
  WEBKIT_WEBSITE_DATA_MEMORY_CACHE = 1 << 0,
  WEBKIT_WEBSITE_DATA_DISK_CACHE = 1 << 1,
  WEBKIT_WEBSITE_DATA_OFFLINE_APPLICATION_CACHE = 1 << 2,
  WEBKIT_WEBSITE_DATA_SESSION_STORAGE = 1 << 3,
  WEBKIT_WEBSITE_DATA_LOCAL_STORAGE = 1 << 4,
  WEBKIT_WEBSITE_DATA_INDEXEDDB_DATABASES = 1 << 6,
  WEBKIT_WEBSITE_DATA_COOKIES = 1 << 8,
  WEBKIT_WEBSITE_DATA_DEVICE_ID_HASH_SALT = 1 << 9,
  WEBKIT_WEBSITE_DATA_HSTS_CACHE = 1 << 10,
  WEBKIT_WEBSITE_DATA_ITP = 1 << 11,
  WEBKIT_WEBSITE_DATA_SERVICE_WORKER_REGISTRATIONS = 1 << 12,
  WEBKIT_WEBSITE_DATA_DOM_CACHE = 1 << 13,
  WEBKIT_WEBSITE_DATA_ALL = (1 << 14) - 1
} WebKitWebsiteDataTypes;

typedef enum {
  WEBKIT_WEB_PROCESS_CRASHED,
  WEBKIT_WEB_PROCESS_EXCEEDED_MEMORY_LIMIT,
  WEBKIT_WEB_PROCESS_TERMINATED_BY_API
} WebKitWebProcessTerminationReason;

typedef enum {
  WEBKIT_SAVE_MODE_MHTML
} WebKitSaveMode;

typedef enum {
  WEBKIT_SNAPSHOT_REGION_VISIBLE = 0,
  WEBKIT_SNAPSHOT_REGION_FULL_DOCUMENT = 1
} WebKitSnapshotRegion;

typedef enum {
  WEBKIT_SNAPSHOT_OPTIONS_NONE = 0,
  WEBKIT_SNAPSHOT_OPTIONS_INCLUDE_SELECTION_HIGHLIGHTING = 1 << 0,
  WEBKIT_SNAPSHOT_OPTIONS_TRANSPARENT_BACKGROUND = 1 << 1
} WebKitSnapshotOptions;

typedef enum {
  WEBKIT_SCRIPT_DIALOG_ALERT,
  WEBKIT_SCRIPT_DIALOG_CONFIRM,
  WEBKIT_SCRIPT_DIALOG_PROMPT,
  WEBKIT_SCRIPT_DIALOG_BEFORE_UNLOAD_CONFIRM
} WebKitScriptDialogType;

typedef enum {
  WEBKIT_PRINT_OPERATION_RESPONSE_PRINT,
  WEBKIT_PRINT_OPERATION_RESPONSE_CANCEL
} WebKitPrintOperationResponse;

typedef enum {
  WEBKIT_PERMISSION_STATE_GRANTED = 0,
  WEBKIT_PERMISSION_STATE_DENIED = 1,
  WEBKIT_PERMISSION_STATE_PROMPT = 2
} WebKitPermissionState;

typedef enum {
  WEBKIT_FEATURE_STATUS_EMBEDDER,
  WEBKIT_FEATURE_STATUS_UNSTABLE,
  WEBKIT_FEATURE_STATUS_INTERNAL,
  WEBKIT_FEATURE_STATUS_DEVELOPER,
  WEBKIT_FEATURE_STATUS_TESTABLE,
  WEBKIT_FEATURE_STATUS_PREVIEW,
  WEBKIT_FEATURE_STATUS_STABLE,
  WEBKIT_FEATURE_STATUS_MATURE
} WebKitFeatureStatus;

typedef enum {
  WEBKIT_WEB_EXTENSION_MODE_NONE,
  WEBKIT_WEB_EXTENSION_MODE_MANIFESTV2,
  WEBKIT_WEB_EXTENSION_MODE_MANIFESTV3
} WebKitWebExtensionMode;

typedef enum {
  WEBKIT_WEB_EXTENSION_MATCH_PATTERN_OPTIONS_NONE = 0
} WebKitWebExtensionMatchPatternOptions;

typedef enum {
  WEBKIT_NETWORK_PROXY_MODE_DEFAULT,
  WEBKIT_NETWORK_PROXY_MODE_NO_PROXY,
  WEBKIT_NETWORK_PROXY_MODE_CUSTOM
} WebKitNetworkProxyMode;

/* Errors */
GQuark webkit_network_error_quark (void);
GQuark webkit_policy_error_quark (void);
GQuark webkit_download_error_quark (void);
GQuark webkit_favicon_database_error_quark (void);
GQuark webkit_user_content_filter_error_quark (void);
GQuark webkit_user_message_error_quark (void);
GQuark webkit_media_error_quark (void);

#define WEBKIT_NETWORK_ERROR (webkit_network_error_quark ())
#define WEBKIT_POLICY_ERROR (webkit_policy_error_quark ())
#define WEBKIT_DOWNLOAD_ERROR (webkit_download_error_quark ())
#define WEBKIT_FAVICON_DATABASE_ERROR (webkit_favicon_database_error_quark ())
#define WEBKIT_USER_CONTENT_FILTER_ERROR (webkit_user_content_filter_error_quark ())
#define WEBKIT_USER_MESSAGE_ERROR (webkit_user_message_error_quark ())
#define WEBKIT_MEDIA_ERROR (webkit_media_error_quark ())

typedef enum {
  WEBKIT_NETWORK_ERROR_FAILED = 399,
  WEBKIT_NETWORK_ERROR_TRANSPORT = 300,
  WEBKIT_NETWORK_ERROR_UNKNOWN_PROTOCOL = 301,
  WEBKIT_NETWORK_ERROR_CANCELLED = 302,
  WEBKIT_NETWORK_ERROR_FILE_DOES_NOT_EXIST = 303
} WebKitNetworkError;

typedef enum {
  WEBKIT_POLICY_ERROR_FAILED = 199,
  WEBKIT_POLICY_ERROR_CANNOT_SHOW_MIME_TYPE = 100,
  WEBKIT_POLICY_ERROR_CANNOT_SHOW_URI = 101,
  WEBKIT_POLICY_ERROR_FRAME_LOAD_INTERRUPTED_BY_POLICY_CHANGE = 102,
  WEBKIT_POLICY_ERROR_CANNOT_USE_RESTRICTED_PORT = 103
} WebKitPolicyError;

typedef enum {
  WEBKIT_DOWNLOAD_ERROR_NETWORK = 499,
  WEBKIT_DOWNLOAD_ERROR_CANCELLED_BY_USER = 400,
  WEBKIT_DOWNLOAD_ERROR_DESTINATION = 401
} WebKitDownloadError;

typedef enum {
  WEBKIT_FAVICON_DATABASE_ERROR_NOT_INITIALIZED = 0,
  WEBKIT_FAVICON_DATABASE_ERROR_FAVICON_NOT_FOUND = 1,
  WEBKIT_FAVICON_DATABASE_ERROR_FAVICON_UNKNOWN = 2
} WebKitFaviconDatabaseError;

typedef enum {
  WEBKIT_USER_CONTENT_FILTER_ERROR_INVALID_SOURCE = 0,
  WEBKIT_USER_CONTENT_FILTER_ERROR_NOT_FOUND = 1
} WebKitUserContentFilterError;

#ifndef WEBKIT_USER_MESSAGE_ERROR_DEFINED
#define WEBKIT_USER_MESSAGE_ERROR_DEFINED
GQuark webkit_user_message_error_quark (void);
#define WEBKIT_USER_MESSAGE_ERROR (webkit_user_message_error_quark ())
typedef enum {
  WEBKIT_USER_MESSAGE_UNHANDLED_MESSAGE = 0
} WebKitUserMessageError;
#endif

typedef enum {
  WEBKIT_MEDIA_ERROR_WILL_HANDLE_LOAD = 204
} WebKitMediaError;

#define WEBKIT_EDITING_COMMAND_CUT "Cut"
#define WEBKIT_EDITING_COMMAND_COPY "Copy"
#define WEBKIT_EDITING_COMMAND_PASTE "Paste"
#define WEBKIT_EDITING_COMMAND_PASTE_AS_PLAIN_TEXT "PasteAsPlainText"

/* GTypes */
#define WEBKIT_TYPE_WEB_VIEW (webkit_web_view_get_type ())
G_DECLARE_DERIVABLE_TYPE (WebKitWebView, webkit_web_view, WEBKIT, WEB_VIEW, GtkWidget)
struct _WebKitWebViewClass {
  GtkWidgetClass parent_class;
  void (*load_changed) (WebKitWebView *web_view, WebKitLoadEvent load_event);
  gboolean (*load_failed) (WebKitWebView *web_view, WebKitLoadEvent load_event, const char *failing_uri, GError *error);
  GtkWidget *(*create) (WebKitWebView *web_view, WebKitNavigationAction *navigation_action);
  void (*ready_to_show) (WebKitWebView *web_view);
  void (*run_as_modal) (WebKitWebView *web_view);
  void (*close) (WebKitWebView *web_view);
  gboolean (*script_dialog) (WebKitWebView *web_view, WebKitScriptDialog *dialog);
  gboolean (*decide_policy) (WebKitWebView *web_view, WebKitPolicyDecision *decision, WebKitPolicyDecisionType type);
  gboolean (*permission_request) (WebKitWebView *web_view, WebKitPermissionRequest *request);
  void (*mouse_target_changed) (WebKitWebView *web_view, WebKitHitTestResult *hit_test_result, guint modifiers);
  gboolean (*print) (WebKitWebView *web_view, WebKitPrintOperation *print_operation);
  void (*resource_load_started) (WebKitWebView *web_view, WebKitWebResource *resource, WebKitURIRequest *request);
  gboolean (*enter_fullscreen) (WebKitWebView *web_view);
  gboolean (*leave_fullscreen) (WebKitWebView *web_view);
  gboolean (*run_file_chooser) (WebKitWebView *web_view, WebKitFileChooserRequest *request);
  gboolean (*context_menu) (WebKitWebView *web_view, WebKitContextMenu *context_menu, WebKitHitTestResult *hit_test_result);
  void (*context_menu_dismissed) (WebKitWebView *web_view);
  void (*submit_form) (WebKitWebView *web_view, gpointer request);
  void (*insecure_content_detected) (WebKitWebView *web_view, guint event);
  gboolean (*web_process_terminated) (WebKitWebView *web_view, WebKitWebProcessTerminationReason reason);
  gboolean (*user_message_received) (WebKitWebView *web_view, WebKitUserMessage *message);
  gboolean (*authenticate) (WebKitWebView *web_view, WebKitAuthenticationRequest *request);
  gboolean (*load_failed_with_tls_errors) (WebKitWebView *web_view, const char *failing_uri, GTlsCertificate *certificate, GTlsCertificateFlags errors);
  gboolean (*show_notification) (WebKitWebView *web_view, WebKitNotification *notification);
  gboolean (*run_color_chooser) (WebKitWebView *web_view, gpointer request);
  gboolean (*show_option_menu) (WebKitWebView *web_view, gpointer menu, GdkRectangle *rectangle);
  void (*query_permission_state) (WebKitWebView *web_view, WebKitPermissionStateQuery *query);
  gpointer _padding[8];
};

#define WEBKIT_TYPE_WEB_CONTEXT (webkit_web_context_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebContext, webkit_web_context, WEBKIT, WEB_CONTEXT, GObject)

#define WEBKIT_TYPE_NETWORK_SESSION (webkit_network_session_get_type ())
G_DECLARE_FINAL_TYPE (WebKitNetworkSession, webkit_network_session, WEBKIT, NETWORK_SESSION, GObject)

#define WEBKIT_TYPE_WEBSITE_DATA_MANAGER (webkit_website_data_manager_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebsiteDataManager, webkit_website_data_manager, WEBKIT, WEBSITE_DATA_MANAGER, GObject)

#define WEBKIT_TYPE_COOKIE_MANAGER (webkit_cookie_manager_get_type ())
G_DECLARE_FINAL_TYPE (WebKitCookieManager, webkit_cookie_manager, WEBKIT, COOKIE_MANAGER, GObject)

#define WEBKIT_TYPE_FAVICON_DATABASE (webkit_favicon_database_get_type ())
G_DECLARE_FINAL_TYPE (WebKitFaviconDatabase, webkit_favicon_database, WEBKIT, FAVICON_DATABASE, GObject)

#define WEBKIT_TYPE_SECURITY_MANAGER (webkit_security_manager_get_type ())
G_DECLARE_FINAL_TYPE (WebKitSecurityManager, webkit_security_manager, WEBKIT, SECURITY_MANAGER, GObject)

#define WEBKIT_TYPE_SETTINGS (webkit_settings_get_type ())
G_DECLARE_FINAL_TYPE (WebKitSettings, webkit_settings, WEBKIT, SETTINGS, GObject)

#define WEBKIT_TYPE_USER_CONTENT_MANAGER (webkit_user_content_manager_get_type ())
G_DECLARE_FINAL_TYPE (WebKitUserContentManager, webkit_user_content_manager, WEBKIT, USER_CONTENT_MANAGER, GObject)

#define WEBKIT_TYPE_USER_CONTENT_FILTER_STORE (webkit_user_content_filter_store_get_type ())
G_DECLARE_FINAL_TYPE (WebKitUserContentFilterStore, webkit_user_content_filter_store, WEBKIT, USER_CONTENT_FILTER_STORE, GObject)

#define WEBKIT_TYPE_DOWNLOAD (webkit_download_get_type ())
G_DECLARE_FINAL_TYPE (WebKitDownload, webkit_download, WEBKIT, DOWNLOAD, GObject)

#define WEBKIT_TYPE_URI_REQUEST (webkit_uri_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitURIRequest, webkit_uri_request, WEBKIT, URI_REQUEST, GObject)

#define WEBKIT_TYPE_URI_RESPONSE (webkit_uri_response_get_type ())
G_DECLARE_FINAL_TYPE (WebKitURIResponse, webkit_uri_response, WEBKIT, URI_RESPONSE, GObject)

#define WEBKIT_TYPE_URI_SCHEME_REQUEST (webkit_uri_scheme_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitURISchemeRequest, webkit_uri_scheme_request, WEBKIT, URI_SCHEME_REQUEST, GObject)

#define WEBKIT_TYPE_POLICY_DECISION (webkit_policy_decision_get_type ())
G_DECLARE_DERIVABLE_TYPE (WebKitPolicyDecision, webkit_policy_decision, WEBKIT, POLICY_DECISION, GObject)
struct _WebKitPolicyDecisionClass { GObjectClass parent_class; };

#define WEBKIT_TYPE_NAVIGATION_POLICY_DECISION (webkit_navigation_policy_decision_get_type ())
G_DECLARE_FINAL_TYPE (WebKitNavigationPolicyDecision, webkit_navigation_policy_decision, WEBKIT, NAVIGATION_POLICY_DECISION, WebKitPolicyDecision)

#define WEBKIT_TYPE_RESPONSE_POLICY_DECISION (webkit_response_policy_decision_get_type ())
G_DECLARE_FINAL_TYPE (WebKitResponsePolicyDecision, webkit_response_policy_decision, WEBKIT, RESPONSE_POLICY_DECISION, WebKitPolicyDecision)

#define WEBKIT_TYPE_WEBSITE_POLICIES (webkit_website_policies_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebsitePolicies, webkit_website_policies, WEBKIT, WEBSITE_POLICIES, GObject)

#define WEBKIT_TYPE_BACK_FORWARD_LIST (webkit_back_forward_list_get_type ())
G_DECLARE_FINAL_TYPE (WebKitBackForwardList, webkit_back_forward_list, WEBKIT, BACK_FORWARD_LIST, GObject)

#define WEBKIT_TYPE_BACK_FORWARD_LIST_ITEM (webkit_back_forward_list_item_get_type ())
G_DECLARE_FINAL_TYPE (WebKitBackForwardListItem, webkit_back_forward_list_item, WEBKIT, BACK_FORWARD_LIST_ITEM, GInitiallyUnowned)

#define WEBKIT_TYPE_FIND_CONTROLLER (webkit_find_controller_get_type ())
G_DECLARE_FINAL_TYPE (WebKitFindController, webkit_find_controller, WEBKIT, FIND_CONTROLLER, GObject)

#define WEBKIT_TYPE_HIT_TEST_RESULT (webkit_hit_test_result_get_type ())
G_DECLARE_FINAL_TYPE (WebKitHitTestResult, webkit_hit_test_result, WEBKIT, HIT_TEST_RESULT, GObject)

#define WEBKIT_TYPE_CONTEXT_MENU (webkit_context_menu_get_type ())
G_DECLARE_FINAL_TYPE (WebKitContextMenu, webkit_context_menu, WEBKIT, CONTEXT_MENU, GObject)

#define WEBKIT_TYPE_CONTEXT_MENU_ITEM (webkit_context_menu_item_get_type ())
G_DECLARE_FINAL_TYPE (WebKitContextMenuItem, webkit_context_menu_item, WEBKIT, CONTEXT_MENU_ITEM, GInitiallyUnowned)

#define WEBKIT_TYPE_PERMISSION_REQUEST (webkit_permission_request_get_type ())
G_DECLARE_INTERFACE (WebKitPermissionRequest, webkit_permission_request, WEBKIT, PERMISSION_REQUEST, GObject)
struct _WebKitPermissionRequestInterface {
  GTypeInterface parent;
  void (*allow) (WebKitPermissionRequest *request);
  void (*deny) (WebKitPermissionRequest *request);
};

#define WEBKIT_TYPE_USER_MEDIA_PERMISSION_REQUEST (webkit_user_media_permission_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitUserMediaPermissionRequest, webkit_user_media_permission_request, WEBKIT, USER_MEDIA_PERMISSION_REQUEST, GObject)

#define WEBKIT_TYPE_GEOLOCATION_PERMISSION_REQUEST (webkit_geolocation_permission_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitGeolocationPermissionRequest, webkit_geolocation_permission_request, WEBKIT, GEOLOCATION_PERMISSION_REQUEST, GObject)

#define WEBKIT_TYPE_NOTIFICATION_PERMISSION_REQUEST (webkit_notification_permission_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitNotificationPermissionRequest, webkit_notification_permission_request, WEBKIT, NOTIFICATION_PERMISSION_REQUEST, GObject)

#define WEBKIT_TYPE_CLIPBOARD_PERMISSION_REQUEST (webkit_clipboard_permission_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitClipboardPermissionRequest, webkit_clipboard_permission_request, WEBKIT, CLIPBOARD_PERMISSION_REQUEST, GObject)

#define WEBKIT_TYPE_WEBSITE_DATA_ACCESS_PERMISSION_REQUEST (webkit_website_data_access_permission_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebsiteDataAccessPermissionRequest, webkit_website_data_access_permission_request, WEBKIT, WEBSITE_DATA_ACCESS_PERMISSION_REQUEST, GObject)

#define WEBKIT_TYPE_PERMISSION_STATE_QUERY (webkit_permission_state_query_get_type ())
GType webkit_permission_state_query_get_type (void);

#define WEBKIT_TYPE_AUTHENTICATION_REQUEST (webkit_authentication_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitAuthenticationRequest, webkit_authentication_request, WEBKIT, AUTHENTICATION_REQUEST, GObject)

#define WEBKIT_TYPE_FILE_CHOOSER_REQUEST (webkit_file_chooser_request_get_type ())
G_DECLARE_FINAL_TYPE (WebKitFileChooserRequest, webkit_file_chooser_request, WEBKIT, FILE_CHOOSER_REQUEST, GObject)

#define WEBKIT_TYPE_NOTIFICATION (webkit_notification_get_type ())
G_DECLARE_FINAL_TYPE (WebKitNotification, webkit_notification, WEBKIT, NOTIFICATION, GObject)

#define WEBKIT_TYPE_EDITOR_STATE (webkit_editor_state_get_type ())
G_DECLARE_FINAL_TYPE (WebKitEditorState, webkit_editor_state, WEBKIT, EDITOR_STATE, GObject)

#define WEBKIT_TYPE_WINDOW_PROPERTIES (webkit_window_properties_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWindowProperties, webkit_window_properties, WEBKIT, WINDOW_PROPERTIES, GObject)

#define WEBKIT_TYPE_WEB_INSPECTOR (webkit_web_inspector_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebInspector, webkit_web_inspector, WEBKIT, WEB_INSPECTOR, GObject)

#define WEBKIT_TYPE_PRINT_OPERATION (webkit_print_operation_get_type ())
G_DECLARE_FINAL_TYPE (WebKitPrintOperation, webkit_print_operation, WEBKIT, PRINT_OPERATION, GObject)

#define WEBKIT_TYPE_WEB_RESOURCE (webkit_web_resource_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebResource, webkit_web_resource, WEBKIT, WEB_RESOURCE, GObject)

#define WEBKIT_TYPE_WEBSITE_DATA (webkit_website_data_get_type ())
GType webkit_website_data_get_type (void);

#define WEBKIT_TYPE_WEB_EXTENSION (webkit_web_extension_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebExtension, webkit_web_extension, WEBKIT, WEB_EXTENSION, GObject)

#define WEBKIT_TYPE_WEB_EXTENSION_MATCH_PATTERN (webkit_web_extension_match_pattern_get_type ())
GType webkit_web_extension_match_pattern_get_type (void);

#define WEBKIT_TYPE_FEATURE_STATUS (webkit_feature_status_get_type ())
GType webkit_feature_status_get_type (void);

#ifndef WEBKIT_USER_MESSAGE_DEFINED
#define WEBKIT_USER_MESSAGE_DEFINED
#define WEBKIT_TYPE_USER_MESSAGE (webkit_user_message_get_type ())
G_DECLARE_FINAL_TYPE (WebKitUserMessage, webkit_user_message, WEBKIT, USER_MESSAGE, GInitiallyUnowned)
WebKitUserMessage *webkit_user_message_new (const char *name, GVariant *parameters);
const char *webkit_user_message_get_name (WebKitUserMessage *message);
GVariant *webkit_user_message_get_parameters (WebKitUserMessage *message);
void webkit_user_message_send_reply (WebKitUserMessage *message, WebKitUserMessage *reply);
#endif

typedef void (*WebKitURISchemeRequestCallback) (WebKitURISchemeRequest *request, gpointer user_data);

/* Functions */
char *webkit_uri_for_display (const char *uri);

/* WebKitWebView */
GtkWidget *webkit_web_view_new (void);
void webkit_web_view_load_uri (WebKitWebView *web_view, const char *uri);
void webkit_web_view_load_request (WebKitWebView *web_view, WebKitURIRequest *request);
void webkit_web_view_load_alternate_html (WebKitWebView *web_view, const char *content, const char *content_uri, const char *base_uri);
void webkit_web_view_reload (WebKitWebView *web_view);
void webkit_web_view_reload_bypass_cache (WebKitWebView *web_view);
void webkit_web_view_stop_loading (WebKitWebView *web_view);
void webkit_web_view_go_back (WebKitWebView *web_view);
void webkit_web_view_go_forward (WebKitWebView *web_view);
gboolean webkit_web_view_can_go_back (WebKitWebView *web_view);
gboolean webkit_web_view_can_go_forward (WebKitWebView *web_view);
void webkit_web_view_go_to_back_forward_list_item (WebKitWebView *web_view, WebKitBackForwardListItem *list_item);
WebKitBackForwardList *webkit_web_view_get_back_forward_list (WebKitWebView *web_view);
const char *webkit_web_view_get_title (WebKitWebView *web_view);
const char *webkit_web_view_get_uri (WebKitWebView *web_view);
double webkit_web_view_get_estimated_load_progress (WebKitWebView *web_view);
gboolean webkit_web_view_is_loading (WebKitWebView *web_view);
gboolean webkit_web_view_is_playing_audio (WebKitWebView *web_view);
gboolean webkit_web_view_get_is_muted (WebKitWebView *web_view);
void webkit_web_view_set_is_muted (WebKitWebView *web_view, gboolean muted);
double webkit_web_view_get_zoom_level (WebKitWebView *web_view);
void webkit_web_view_set_zoom_level (WebKitWebView *web_view, double zoom_level);
double webkit_web_view_get_magnification (WebKitWebView *web_view);
void webkit_web_view_set_magnification (WebKitWebView *web_view, double magnification);
GdkTexture *webkit_web_view_get_favicon (WebKitWebView *web_view);
WebKitWebContext *webkit_web_view_get_context (WebKitWebView *web_view);
WebKitSettings *webkit_web_view_get_settings (WebKitWebView *web_view);
WebKitUserContentManager *webkit_web_view_get_user_content_manager (WebKitWebView *web_view);
WebKitFindController *webkit_web_view_get_find_controller (WebKitWebView *web_view);
WebKitWebInspector *webkit_web_view_get_inspector (WebKitWebView *web_view);
WebKitEditorState *webkit_web_view_get_editor_state (WebKitWebView *web_view);
WebKitWindowProperties *webkit_web_view_get_window_properties (WebKitWebView *web_view);
WebKitWebResource *webkit_web_view_get_main_resource (WebKitWebView *web_view);
guint64 webkit_web_view_get_page_id (WebKitWebView *web_view);
const char *webkit_web_view_get_custom_charset (WebKitWebView *web_view);
void webkit_web_view_set_custom_charset (WebKitWebView *web_view, const char *charset);
gboolean webkit_web_view_get_tls_info (WebKitWebView *web_view, GTlsCertificate **certificate, GTlsCertificateFlags *errors);
gboolean webkit_web_view_get_is_web_process_responsive (WebKitWebView *web_view);
gboolean webkit_web_view_is_controlled_by_automation (WebKitWebView *web_view);
void webkit_web_view_terminate_web_process (WebKitWebView *web_view);
void webkit_web_view_try_close (WebKitWebView *web_view);
void webkit_web_view_execute_editing_command (WebKitWebView *web_view, const char *command);
void webkit_web_view_set_cors_allowlist (WebKitWebView *web_view, const char * const *allowlist);
void webkit_web_view_evaluate_javascript (WebKitWebView *web_view, const char *script, gssize length, const char *world_name, const char *source_uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
JSCValue *webkit_web_view_evaluate_javascript_finish (WebKitWebView *web_view, GAsyncResult *result, GError **error);
void webkit_web_view_get_snapshot (WebKitWebView *web_view, WebKitSnapshotRegion region, WebKitSnapshotOptions options, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GdkTexture *webkit_web_view_get_snapshot_finish (WebKitWebView *web_view, GAsyncResult *result, GError **error);
void webkit_web_view_save_to_file (WebKitWebView *web_view, GFile *file, WebKitSaveMode save_mode, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_web_view_save_to_file_finish (WebKitWebView *web_view, GAsyncResult *result, GError **error);
WebKitWebViewSessionState *webkit_web_view_get_session_state (WebKitWebView *web_view);
void webkit_web_view_restore_session_state (WebKitWebView *web_view, WebKitWebViewSessionState *state);
void webkit_web_view_send_message_to_page (WebKitWebView *web_view, WebKitUserMessage *message, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
WebKitUserMessage *webkit_web_view_send_message_to_page_finish (WebKitWebView *web_view, GAsyncResult *result, GError **error);

WebKitWebViewSessionState *webkit_web_view_session_state_new (GBytes *data);
WebKitWebViewSessionState *webkit_web_view_session_state_ref (WebKitWebViewSessionState *state);
void webkit_web_view_session_state_unref (WebKitWebViewSessionState *state);
GBytes *webkit_web_view_session_state_serialize (WebKitWebViewSessionState *state);

/* WebKitWebContext & NetworkSession */
WebKitWebContext *webkit_web_context_new (void);
void webkit_web_context_register_uri_scheme (WebKitWebContext *context, const char *scheme, WebKitURISchemeRequestCallback callback, gpointer user_data, GDestroyNotify user_data_destroy_func);
WebKitSecurityManager *webkit_web_context_get_security_manager (WebKitWebContext *context);
void webkit_web_context_set_preferred_languages (WebKitWebContext *context, const char * const *languages);
void webkit_web_context_set_spell_checking_enabled (WebKitWebContext *context, gboolean enabled);
void webkit_web_context_set_spell_checking_languages (WebKitWebContext *context, const char * const *languages);
void webkit_web_context_set_web_process_extensions_directory (WebKitWebContext *context, const char *directory);
void webkit_web_context_set_web_process_extensions_initialization_user_data (WebKitWebContext *context, GVariant *user_data);
void webkit_web_context_add_path_to_sandbox (WebKitWebContext *context, const char *path, gboolean read_only);
void webkit_web_context_set_automation_allowed (WebKitWebContext *context, gboolean allowed);
WebKitNetworkSession *webkit_web_context_get_network_session_for_automation (WebKitWebContext *context);
void webkit_web_context_initialize_notification_permissions (WebKitWebContext *context, GList *allowed_origins, GList *disallowed_origins);
void webkit_web_context_send_message_to_all_extensions (WebKitWebContext *context, WebKitUserMessage *message);

WebKitNetworkSession *webkit_network_session_new (const char *data_directory, const char *cache_directory);
WebKitNetworkSession *webkit_network_session_new_ephemeral (void);
gboolean webkit_network_session_is_ephemeral (WebKitNetworkSession *session);
WebKitWebsiteDataManager *webkit_network_session_get_website_data_manager (WebKitNetworkSession *session);
WebKitCookieManager *webkit_network_session_get_cookie_manager (WebKitNetworkSession *session);
void webkit_network_session_set_itp_enabled (WebKitNetworkSession *session, gboolean enabled);
void webkit_network_session_set_persistent_credential_storage_enabled (WebKitNetworkSession *session, gboolean enabled);
void webkit_network_session_allow_tls_certificate_for_host (WebKitNetworkSession *session, GTlsCertificate *certificate, const char *host);
void webkit_network_session_prefetch_dns (WebKitNetworkSession *session, const char *hostname);
WebKitDownload *webkit_network_session_download_uri (WebKitNetworkSession *session, const char *uri);

/* WebsiteDataManager & CookieManager & FaviconDatabase */
void webkit_website_data_manager_set_favicons_enabled (WebKitWebsiteDataManager *manager, gboolean enabled);
WebKitFaviconDatabase *webkit_website_data_manager_get_favicon_database (WebKitWebsiteDataManager *manager);
void webkit_website_data_manager_clear (WebKitWebsiteDataManager *manager, WebKitWebsiteDataTypes types, GTimeSpan timespan, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_website_data_manager_clear_finish (WebKitWebsiteDataManager *manager, GAsyncResult *result, GError **error);
void webkit_website_data_manager_fetch (WebKitWebsiteDataManager *manager, WebKitWebsiteDataTypes types, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GList *webkit_website_data_manager_fetch_finish (WebKitWebsiteDataManager *manager, GAsyncResult *result, GError **error);
void webkit_website_data_manager_remove (WebKitWebsiteDataManager *manager, WebKitWebsiteDataTypes types, GList *website_data, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_website_data_manager_remove_finish (WebKitWebsiteDataManager *manager, GAsyncResult *result, GError **error);
void webkit_website_data_manager_get_itp_summary (WebKitWebsiteDataManager *manager, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GList *webkit_website_data_manager_get_itp_summary_finish (WebKitWebsiteDataManager *manager, GAsyncResult *result, GError **error);

const char *webkit_website_data_get_name (WebKitWebsiteData *website_data);
WebKitWebsiteDataTypes webkit_website_data_get_types (WebKitWebsiteData *website_data);
WebKitWebsiteData *webkit_website_data_ref (WebKitWebsiteData *website_data);
void webkit_website_data_unref (WebKitWebsiteData *website_data);

const char *webkit_itp_third_party_get_domain (WebKitITPThirdParty *itp_third_party);
GList *webkit_itp_third_party_get_first_parties (WebKitITPThirdParty *itp_third_party);
const char *webkit_itp_first_party_get_domain (WebKitITPFirstParty *itp_first_party);
gboolean webkit_itp_first_party_get_website_data_access_allowed (WebKitITPFirstParty *itp_first_party);

void webkit_cookie_manager_set_persistent_storage (WebKitCookieManager *cookie_manager, const char *filename, WebKitCookiePersistentStorage storage);
void webkit_cookie_manager_set_accept_policy (WebKitCookieManager *cookie_manager, WebKitCookieAcceptPolicy policy);
void webkit_cookie_manager_add_cookie (WebKitCookieManager *cookie_manager, SoupCookie *cookie, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_cookie_manager_add_cookie_finish (WebKitCookieManager *cookie_manager, GAsyncResult *result, GError **error);
void webkit_cookie_manager_get_cookies (WebKitCookieManager *cookie_manager, const char *uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GList *webkit_cookie_manager_get_cookies_finish (WebKitCookieManager *cookie_manager, GAsyncResult *result, GError **error);
void webkit_cookie_manager_delete_cookie (WebKitCookieManager *cookie_manager, SoupCookie *cookie, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_cookie_manager_delete_cookie_finish (WebKitCookieManager *cookie_manager, GAsyncResult *result, GError **error);

void webkit_favicon_database_get_favicon (WebKitFaviconDatabase *database, const char *page_uri, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GdkTexture *webkit_favicon_database_get_favicon_finish (WebKitFaviconDatabase *database, GAsyncResult *result, GError **error);
char *webkit_favicon_database_get_favicon_uri (WebKitFaviconDatabase *database, const char *page_uri);

/* SecurityManager & SecurityOrigin */
void webkit_security_manager_register_uri_scheme_as_local (WebKitSecurityManager *security_manager, const char *scheme);
void webkit_security_manager_register_uri_scheme_as_secure (WebKitSecurityManager *security_manager, const char *scheme);
void webkit_security_manager_register_uri_scheme_as_display_isolated (WebKitSecurityManager *security_manager, const char *scheme);
gboolean webkit_security_manager_uri_scheme_is_local (WebKitSecurityManager *security_manager, const char *scheme);
gboolean webkit_security_manager_uri_scheme_is_empty_document (WebKitSecurityManager *security_manager, const char *scheme);

WebKitSecurityOrigin *webkit_security_origin_new (const char *protocol, const char *host, guint16 port);
WebKitSecurityOrigin *webkit_security_origin_new_for_uri (const char *uri);
WebKitSecurityOrigin *webkit_security_origin_ref (WebKitSecurityOrigin *origin);
void webkit_security_origin_unref (WebKitSecurityOrigin *origin);
const char *webkit_security_origin_get_protocol (WebKitSecurityOrigin *origin);
const char *webkit_security_origin_get_host (WebKitSecurityOrigin *origin);
guint16 webkit_security_origin_get_port (WebKitSecurityOrigin *origin);
char *webkit_security_origin_to_string (WebKitSecurityOrigin *origin);

/* Settings & Features */
WebKitSettings *webkit_settings_new (void);
WebKitSettings *webkit_settings_new_with_settings (const char *first_setting_name, ...);
guint32 webkit_settings_font_size_to_pixels (guint32 points);
const char *webkit_settings_get_user_agent (WebKitSettings *settings);
void webkit_settings_set_user_agent (WebKitSettings *settings, const char *user_agent);
void webkit_settings_set_user_agent_with_application_details (WebKitSettings *settings, const char *application_name, const char *application_version);
void webkit_settings_set_hardware_acceleration_policy (WebKitSettings *settings, WebKitHardwareAccelerationPolicy policy);
void webkit_settings_set_enable_html5_database (WebKitSettings *settings, gboolean enabled);
void webkit_settings_set_enable_html5_local_storage (WebKitSettings *settings, gboolean enabled);
WebKitFeatureList *webkit_settings_get_all_features (void);
gboolean webkit_settings_get_feature_enabled (WebKitSettings *settings, WebKitFeature *feature);
void webkit_settings_set_feature_enabled (WebKitSettings *settings, WebKitFeature *feature, gboolean enabled);

gsize webkit_feature_list_get_length (WebKitFeatureList *feature_list);
WebKitFeature *webkit_feature_list_get (WebKitFeatureList *feature_list, gsize index);
WebKitFeature *webkit_feature_ref (WebKitFeature *feature);
void webkit_feature_unref (WebKitFeature *feature);
void webkit_feature_list_unref (WebKitFeatureList *feature_list);
const char *webkit_feature_get_name (WebKitFeature *feature);
const char *webkit_feature_get_category (WebKitFeature *feature);
const char *webkit_feature_get_details (WebKitFeature *feature);
gboolean webkit_feature_get_default_value (WebKitFeature *feature);
WebKitFeatureStatus webkit_feature_get_status (WebKitFeature *feature);

/* UserContentManager */
WebKitUserContentManager *webkit_user_content_manager_new (void);
void webkit_user_content_manager_add_script (WebKitUserContentManager *manager, WebKitUserScript *script);
void webkit_user_content_manager_remove_script (WebKitUserContentManager *manager, WebKitUserScript *script);
void webkit_user_content_manager_remove_all_scripts (WebKitUserContentManager *manager);
void webkit_user_content_manager_add_style_sheet (WebKitUserContentManager *manager, WebKitUserStyleSheet *stylesheet);
void webkit_user_content_manager_remove_style_sheet (WebKitUserContentManager *manager, WebKitUserStyleSheet *stylesheet);
void webkit_user_content_manager_remove_all_style_sheets (WebKitUserContentManager *manager);
void webkit_user_content_manager_add_filter (WebKitUserContentManager *manager, WebKitUserContentFilter *filter);
void webkit_user_content_manager_remove_all_filters (WebKitUserContentManager *manager);
gboolean webkit_user_content_manager_register_script_message_handler (WebKitUserContentManager *manager, const char *name, const char *world_name);
void webkit_user_content_manager_unregister_script_message_handler (WebKitUserContentManager *manager, const char *name, const char *world_name);

WebKitUserScript *webkit_user_script_new (const char *source, WebKitUserContentInjectedFrames injected_frames, WebKitUserScriptInjectionTime injection_time, const char * const *allow_list, const char * const *block_list);
WebKitUserScript *webkit_user_script_new_for_world (const char *source, WebKitUserContentInjectedFrames injected_frames, WebKitUserScriptInjectionTime injection_time, const char *world_name, const char * const *allow_list, const char * const *block_list);
void webkit_user_script_unref (WebKitUserScript *user_script);

WebKitUserStyleSheet *webkit_user_style_sheet_new (const char *source, WebKitUserContentInjectedFrames injected_frames, WebKitUserContentLevel level, const char * const *allow_list, const char * const *block_list);
WebKitUserStyleSheet *webkit_user_style_sheet_new_for_world (const char *source, WebKitUserContentInjectedFrames injected_frames, WebKitUserContentLevel level, const char *world_name, const char * const *allow_list, const char * const *block_list);
void webkit_user_style_sheet_unref (WebKitUserStyleSheet *user_style_sheet);

WebKitUserContentFilterStore *webkit_user_content_filter_store_new (const char *storage_path);
void webkit_user_content_filter_store_save (WebKitUserContentFilterStore *store, const char *identifier, GBytes *source, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
WebKitUserContentFilter *webkit_user_content_filter_store_save_finish (WebKitUserContentFilterStore *store, GAsyncResult *result, GError **error);
void webkit_user_content_filter_store_load (WebKitUserContentFilterStore *store, const char *identifier, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
WebKitUserContentFilter *webkit_user_content_filter_store_load_finish (WebKitUserContentFilterStore *store, GAsyncResult *result, GError **error);
void webkit_user_content_filter_store_remove (WebKitUserContentFilterStore *store, const char *identifier, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean webkit_user_content_filter_store_remove_finish (WebKitUserContentFilterStore *store, GAsyncResult *result, GError **error);
void webkit_user_content_filter_unref (WebKitUserContentFilter *user_content_filter);

/* URIRequest, URIResponse, URISchemeRequest */
WebKitURIRequest *webkit_uri_request_new (const char *uri);
const char *webkit_uri_request_get_uri (WebKitURIRequest *request);
void webkit_uri_request_set_uri (WebKitURIRequest *request, const char *uri);
const char *webkit_uri_response_get_uri (WebKitURIResponse *response);
guint webkit_uri_response_get_status_code (WebKitURIResponse *response);
guint64 webkit_uri_response_get_content_length (WebKitURIResponse *response);
const char *webkit_uri_response_get_mime_type (WebKitURIResponse *response);
const char *webkit_uri_response_get_suggested_filename (WebKitURIResponse *response);
const char *webkit_uri_scheme_request_get_uri (WebKitURISchemeRequest *request);
const char *webkit_uri_scheme_request_get_path (WebKitURISchemeRequest *request);
WebKitWebView *webkit_uri_scheme_request_get_web_view (WebKitURISchemeRequest *request);
void webkit_uri_scheme_request_finish (WebKitURISchemeRequest *request, GInputStream *stream, gint64 stream_length, const char *content_type);
void webkit_uri_scheme_request_finish_error (WebKitURISchemeRequest *request, GError *error);

/* PolicyDecision & NavigationAction */
void webkit_policy_decision_use (WebKitPolicyDecision *decision);
void webkit_policy_decision_use_with_policies (WebKitPolicyDecision *decision, WebKitWebsitePolicies *policies);
void webkit_policy_decision_ignore (WebKitPolicyDecision *decision);
void webkit_policy_decision_download (WebKitPolicyDecision *decision);
WebKitNavigationAction *webkit_navigation_policy_decision_get_navigation_action (WebKitNavigationPolicyDecision *decision);
WebKitURIResponse *webkit_response_policy_decision_get_response (WebKitResponsePolicyDecision *decision);
gboolean webkit_response_policy_decision_is_main_frame_main_resource (WebKitResponsePolicyDecision *decision);
gboolean webkit_response_policy_decision_is_mime_type_supported (WebKitResponsePolicyDecision *decision);
WebKitURIRequest *webkit_navigation_action_get_request (WebKitNavigationAction *navigation);
WebKitNavigationType webkit_navigation_action_get_navigation_type (WebKitNavigationAction *navigation);
guint webkit_navigation_action_get_mouse_button (WebKitNavigationAction *navigation);
guint webkit_navigation_action_get_modifiers (WebKitNavigationAction *navigation);
gboolean webkit_navigation_action_is_user_gesture (WebKitNavigationAction *navigation);
WebKitWebsitePolicies *webkit_website_policies_new_with_policies (const char *first_policy_name, ...);

/* BackForwardList */
WebKitBackForwardListItem *webkit_back_forward_list_get_current_item (WebKitBackForwardList *back_forward_list);
WebKitBackForwardListItem *webkit_back_forward_list_get_back_item (WebKitBackForwardList *back_forward_list);
WebKitBackForwardListItem *webkit_back_forward_list_get_forward_item (WebKitBackForwardList *back_forward_list);
GList *webkit_back_forward_list_get_back_list_with_limit (WebKitBackForwardList *back_forward_list, guint limit);
GList *webkit_back_forward_list_get_forward_list_with_limit (WebKitBackForwardList *back_forward_list, guint limit);
const char *webkit_back_forward_list_item_get_uri (WebKitBackForwardListItem *list_item);
const char *webkit_back_forward_list_item_get_title (WebKitBackForwardListItem *list_item);
const char *webkit_back_forward_list_item_get_original_uri (WebKitBackForwardListItem *list_item);

/* Download */
WebKitURIRequest *webkit_download_get_request (WebKitDownload *download);
WebKitURIResponse *webkit_download_get_response (WebKitDownload *download);
const char *webkit_download_get_destination (WebKitDownload *download);
void webkit_download_set_destination (WebKitDownload *download, const char *destination);
gboolean webkit_download_get_allow_overwrite (WebKitDownload *download);
void webkit_download_set_allow_overwrite (WebKitDownload *download, gboolean allowed);
double webkit_download_get_estimated_progress (WebKitDownload *download);
guint64 webkit_download_get_received_data_length (WebKitDownload *download);
double webkit_download_get_elapsed_time (WebKitDownload *download);
void webkit_download_cancel (WebKitDownload *download);

/* FindController */
void webkit_find_controller_search (WebKitFindController *find_controller, const char *search_text, guint32 find_options, guint max_match_count);
void webkit_find_controller_search_next (WebKitFindController *find_controller);
void webkit_find_controller_search_previous (WebKitFindController *find_controller);
void webkit_find_controller_search_finish (WebKitFindController *find_controller);
void webkit_find_controller_count_matches (WebKitFindController *find_controller, const char *search_text, guint32 find_options, guint max_match_count);
guint32 webkit_find_controller_get_options (WebKitFindController *find_controller);

/* HitTestResult & ContextMenu */
guint webkit_hit_test_result_get_context (WebKitHitTestResult *hit_test_result);
gboolean webkit_hit_test_result_context_is_link (WebKitHitTestResult *hit_test_result);
gboolean webkit_hit_test_result_context_is_image (WebKitHitTestResult *hit_test_result);
gboolean webkit_hit_test_result_context_is_media (WebKitHitTestResult *hit_test_result);
gboolean webkit_hit_test_result_context_is_editable (WebKitHitTestResult *hit_test_result);
const char *webkit_hit_test_result_get_link_uri (WebKitHitTestResult *hit_test_result);
const char *webkit_hit_test_result_get_link_title (WebKitHitTestResult *hit_test_result);
const char *webkit_hit_test_result_get_link_label (WebKitHitTestResult *hit_test_result);
const char *webkit_hit_test_result_get_image_uri (WebKitHitTestResult *hit_test_result);

WebKitContextMenu *webkit_context_menu_new_with_items (GList *items);
void webkit_context_menu_append (WebKitContextMenu *menu, WebKitContextMenuItem *item);
GList *webkit_context_menu_get_items (WebKitContextMenu *menu);
void webkit_context_menu_remove_all (WebKitContextMenu *menu);
void webkit_context_menu_set_user_data (WebKitContextMenu *menu, GVariant *user_data);
GVariant *webkit_context_menu_get_user_data (WebKitContextMenu *menu);
GdkEvent *webkit_context_menu_get_event (WebKitContextMenu *menu);
WebKitContextMenuItem *webkit_context_menu_item_new_from_stock_action (WebKitContextMenuAction action);
WebKitContextMenuItem *webkit_context_menu_item_new_from_gaction (GAction *action, const char *label, GVariant *target);
WebKitContextMenuItem *webkit_context_menu_item_new_with_submenu (const char *label, WebKitContextMenu *submenu);
WebKitContextMenuItem *webkit_context_menu_item_new_separator (void);
WebKitContextMenuAction webkit_context_menu_item_get_stock_action (WebKitContextMenuItem *item);

/* Permissions, Auth, Dialogs, FileChooser, Notifications, EditorState, WindowProperties, Inspector, Print, WebResource */
void webkit_permission_request_allow (WebKitPermissionRequest *request);
void webkit_permission_request_deny (WebKitPermissionRequest *request);
gboolean webkit_user_media_permission_is_for_audio_device (WebKitUserMediaPermissionRequest *request);
gboolean webkit_user_media_permission_is_for_video_device (WebKitUserMediaPermissionRequest *request);
gboolean webkit_user_media_permission_is_for_display_device (WebKitUserMediaPermissionRequest *request);
const char *webkit_website_data_access_permission_request_get_requesting_domain (WebKitWebsiteDataAccessPermissionRequest *request);
const char *webkit_website_data_access_permission_request_get_current_domain (WebKitWebsiteDataAccessPermissionRequest *request);
const char *webkit_permission_state_query_get_name (WebKitPermissionStateQuery *query);
WebKitSecurityOrigin *webkit_permission_state_query_get_security_origin (WebKitPermissionStateQuery *query);
void webkit_permission_state_query_finish (WebKitPermissionStateQuery *query, WebKitPermissionState state);

const char *webkit_authentication_request_get_host (WebKitAuthenticationRequest *request);
guint webkit_authentication_request_get_port (WebKitAuthenticationRequest *request);
const char *webkit_authentication_request_get_realm (WebKitAuthenticationRequest *request);
WebKitAuthenticationScheme webkit_authentication_request_get_scheme (WebKitAuthenticationRequest *request);
WebKitSecurityOrigin *webkit_authentication_request_get_security_origin (WebKitAuthenticationRequest *request);
void webkit_authentication_request_set_can_save_credentials (WebKitAuthenticationRequest *request, gboolean enabled);
void webkit_authentication_request_authenticate (WebKitAuthenticationRequest *request, WebKitCredential *credential);
WebKitCredential *webkit_credential_new (const char *username, const char *password, WebKitCredentialPersistence persistence);
WebKitCredential *webkit_credential_new_for_certificate (GTlsCertificate *certificate, WebKitCredentialPersistence persistence);
WebKitCredential *webkit_credential_new_for_certificate_pin (const char *pin, WebKitCredentialPersistence persistence);
void webkit_credential_free (WebKitCredential *credential);
const char *webkit_credential_get_username (WebKitCredential *credential);
const char *webkit_credential_get_password (WebKitCredential *credential);
WebKitCredentialPersistence webkit_credential_get_persistence (WebKitCredential *credential);

WebKitScriptDialogType webkit_script_dialog_get_dialog_type (WebKitScriptDialog *dialog);
void webkit_script_dialog_confirm_set_confirmed (WebKitScriptDialog *dialog, gboolean confirmed);

const char * const *webkit_file_chooser_request_get_mime_types (WebKitFileChooserRequest *request);
gboolean webkit_file_chooser_request_get_select_multiple (WebKitFileChooserRequest *request);
void webkit_file_chooser_request_select_files (WebKitFileChooserRequest *request, const char * const *files);
void webkit_file_chooser_request_cancel (WebKitFileChooserRequest *request);

gboolean webkit_editor_state_is_cut_available (WebKitEditorState *editor_state);
gboolean webkit_editor_state_is_copy_available (WebKitEditorState *editor_state);
gboolean webkit_editor_state_is_paste_available (WebKitEditorState *editor_state);
gboolean webkit_editor_state_is_undo_available (WebKitEditorState *editor_state);
gboolean webkit_editor_state_is_redo_available (WebKitEditorState *editor_state);

void webkit_window_properties_get_geometry (WebKitWindowProperties *window_properties, GdkRectangle *geometry);
gboolean webkit_window_properties_get_toolbar_visible (WebKitWindowProperties *window_properties);
gboolean webkit_window_properties_get_menubar_visible (WebKitWindowProperties *window_properties);
gboolean webkit_window_properties_get_locationbar_visible (WebKitWindowProperties *window_properties);
gboolean webkit_window_properties_get_resizable (WebKitWindowProperties *window_properties);

void webkit_web_inspector_show (WebKitWebInspector *inspector);
void webkit_web_inspector_close (WebKitWebInspector *inspector);

WebKitPrintOperation *webkit_print_operation_new (WebKitWebView *web_view);
GtkPrintSettings *webkit_print_operation_get_print_settings (WebKitPrintOperation *print_operation);
void webkit_print_operation_set_print_settings (WebKitPrintOperation *print_operation, GtkPrintSettings *print_settings);
GtkPageSetup *webkit_print_operation_get_page_setup (WebKitPrintOperation *print_operation);
void webkit_print_operation_set_page_setup (WebKitPrintOperation *print_operation, GtkPageSetup *page_setup);
WebKitPrintOperationResponse webkit_print_operation_run_dialog (WebKitPrintOperation *print_operation, GtkWindow *parent);

const char *webkit_web_resource_get_uri (WebKitWebResource *resource);
WebKitURIResponse *webkit_web_resource_get_response (WebKitWebResource *resource);
void webkit_web_resource_get_data (WebKitWebResource *resource, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
guchar *webkit_web_resource_get_data_finish (WebKitWebResource *resource, GAsyncResult *result, gsize *length, GError **error);

/* Automation & WebExtensions */
WebKitApplicationInfo *webkit_application_info_new (void);
void webkit_application_info_set_name (WebKitApplicationInfo *info, const char *name);
void webkit_application_info_set_version (WebKitApplicationInfo *info, guint64 major, guint64 minor, guint64 micro);
void webkit_application_info_unref (WebKitApplicationInfo *info);
void webkit_automation_session_set_application_info (WebKitAutomationSession *session, WebKitApplicationInfo *info);

WebKitWebExtension *webkit_web_extension_new (const char *base_location, GError **error);
const char *webkit_web_extension_get_display_name (WebKitWebExtension *extension);
const char *webkit_web_extension_get_display_short_name (WebKitWebExtension *extension);
const char *webkit_web_extension_get_display_version (WebKitWebExtension *extension);
const char *webkit_web_extension_get_display_description (WebKitWebExtension *extension);
GIcon *webkit_web_extension_get_icon (WebKitWebExtension *extension, gsize width, gsize height);
gboolean webkit_web_extension_get_has_background_content (WebKitWebExtension *extension);
gboolean webkit_web_extension_get_has_persistent_background_content (WebKitWebExtension *extension);
WebKitWebExtensionMatchPattern *webkit_web_extension_match_pattern_new_with_string (const char *pattern, GError **error);
gboolean webkit_web_extension_match_pattern_matches_url (WebKitWebExtensionMatchPattern *pattern, const char *url, WebKitWebExtensionMatchPatternOptions options);
void webkit_web_extension_match_pattern_register_custom_url_scheme (const char *scheme);
void webkit_web_extension_match_pattern_unref (WebKitWebExtensionMatchPattern *pattern);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitWebViewSessionState, webkit_web_view_session_state_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitUserScript, webkit_user_script_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitUserStyleSheet, webkit_user_style_sheet_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitUserContentFilter, webkit_user_content_filter_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitSecurityOrigin, webkit_security_origin_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitWebsiteData, webkit_website_data_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitFeature, webkit_feature_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitFeatureList, webkit_feature_list_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitWebExtensionMatchPattern, webkit_web_extension_match_pattern_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitApplicationInfo, webkit_application_info_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitCredential, webkit_credential_free)
void webkit_itp_third_party_unref (WebKitITPThirdParty *itp_third_party);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (WebKitITPThirdParty, webkit_itp_third_party_unref)

G_END_DECLS
