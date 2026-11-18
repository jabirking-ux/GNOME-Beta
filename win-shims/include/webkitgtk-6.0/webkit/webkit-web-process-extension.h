#pragma once
#include <glib-object.h>
#include <gio/gio.h>
#include <jsc/jsc.h>

G_BEGIN_DECLS

typedef struct _WebKitWebProcessExtension WebKitWebProcessExtension;
typedef struct _WebKitWebPage WebKitWebPage;
typedef struct _WebKitFrame WebKitFrame;
typedef struct _WebKitScriptWorld WebKitScriptWorld;
typedef struct _WebKitWebFormManager WebKitWebFormManager;
typedef struct _WebKitWebHitTestResult WebKitWebHitTestResult;
typedef struct _WebKitUserMessage WebKitUserMessage;
typedef struct _WebKitURIRequest WebKitURIRequest;
typedef struct _WebKitURIResponse WebKitURIResponse;
typedef struct _WebKitContextMenu WebKitContextMenu;
typedef struct _WebKitHitTestResult WebKitHitTestResult;
void webkit_context_menu_set_user_data (WebKitContextMenu *menu, GVariant *user_data);

#ifndef WEBKIT_USER_MESSAGE_ERROR_DEFINED
#define WEBKIT_USER_MESSAGE_ERROR_DEFINED
GQuark webkit_user_message_error_quark (void);
#define WEBKIT_USER_MESSAGE_ERROR (webkit_user_message_error_quark ())
typedef enum {
  WEBKIT_USER_MESSAGE_UNHANDLED_MESSAGE = 0
} WebKitUserMessageError;
#endif

#define WEBKIT_TYPE_WEB_PROCESS_EXTENSION (webkit_web_process_extension_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebProcessExtension, webkit_web_process_extension, WEBKIT, WEB_PROCESS_EXTENSION, GObject)

#define WEBKIT_TYPE_WEB_PAGE (webkit_web_page_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebPage, webkit_web_page, WEBKIT, WEB_PAGE, GObject)

#define WEBKIT_TYPE_FRAME (webkit_frame_get_type ())
G_DECLARE_FINAL_TYPE (WebKitFrame, webkit_frame, WEBKIT, FRAME, GObject)

#define WEBKIT_TYPE_SCRIPT_WORLD (webkit_script_world_get_type ())
G_DECLARE_FINAL_TYPE (WebKitScriptWorld, webkit_script_world, WEBKIT, SCRIPT_WORLD, GObject)

#define WEBKIT_TYPE_WEB_FORM_MANAGER (webkit_web_form_manager_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebFormManager, webkit_web_form_manager, WEBKIT, WEB_FORM_MANAGER, GObject)

#define WEBKIT_TYPE_WEB_HIT_TEST_RESULT (webkit_web_hit_test_result_get_type ())
G_DECLARE_FINAL_TYPE (WebKitWebHitTestResult, webkit_web_hit_test_result, WEBKIT, WEB_HIT_TEST_RESULT, GObject)

#ifndef WEBKIT_USER_MESSAGE_DEFINED
#define WEBKIT_USER_MESSAGE_DEFINED
#define WEBKIT_TYPE_USER_MESSAGE (webkit_user_message_get_type ())
G_DECLARE_FINAL_TYPE (WebKitUserMessage, webkit_user_message, WEBKIT, USER_MESSAGE, GInitiallyUnowned)
WebKitUserMessage *webkit_user_message_new (const char *name, GVariant *parameters);
const char *webkit_user_message_get_name (WebKitUserMessage *message);
GVariant *webkit_user_message_get_parameters (WebKitUserMessage *message);
void webkit_user_message_send_reply (WebKitUserMessage *message, WebKitUserMessage *reply);
#endif

typedef void (*WebKitWebProcessExtensionInitializeWithUserDataFunction) (WebKitWebProcessExtension *extension, const GVariant *user_data);

WebKitWebPage *webkit_web_process_extension_get_page (WebKitWebProcessExtension *extension, guint64 page_id);
void webkit_web_process_extension_send_message_to_context (WebKitWebProcessExtension *extension, WebKitUserMessage *message, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);

guint64 webkit_web_page_get_id (WebKitWebPage *web_page);
const char *webkit_web_page_get_uri (WebKitWebPage *web_page);
WebKitFrame *webkit_web_page_get_main_frame (WebKitWebPage *web_page);
WebKitWebFormManager *webkit_web_page_get_form_manager (WebKitWebPage *web_page, WebKitScriptWorld *world);
void webkit_web_page_send_message_to_view (WebKitWebPage *web_page, WebKitUserMessage *message, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
WebKitUserMessage *webkit_web_page_send_message_to_view_finish (WebKitWebPage *web_page, GAsyncResult *result, GError **error);

guint64 webkit_frame_get_id (WebKitFrame *frame);
JSCContext *webkit_frame_get_js_context (WebKitFrame *frame);
JSCContext *webkit_frame_get_js_context_for_script_world (WebKitFrame *frame, WebKitScriptWorld *world);

WebKitScriptWorld *webkit_script_world_get_default (void);
WebKitScriptWorld *webkit_script_world_new_with_name (const char *name);
const char *webkit_script_world_get_name (WebKitScriptWorld *world);

void webkit_web_form_manager_input_element_auto_fill (JSCValue *element, const char *value);
gboolean webkit_web_form_manager_input_element_is_user_edited (JSCValue *element);

JSCValue *webkit_web_hit_test_result_get_js_node (WebKitWebHitTestResult *hit_test_result, WebKitScriptWorld *world);

G_END_DECLS
