#pragma once
#include <glib-object.h>

#ifdef _WIN32
#include <locale.h>
#ifndef LC_MESSAGES
#define LC_MESSAGES 6
#endif
#endif

G_BEGIN_DECLS

typedef struct _JSCContext JSCContext;
typedef struct _JSCValue JSCValue;
typedef struct _JSCException JSCException;
typedef struct _JSCClass JSCClass;
typedef struct _JSCWeakValue JSCWeakValue;

typedef void (*JSCExceptionHandler) (JSCContext *context, JSCException *exception, gpointer user_data);

#define JSC_TYPE_CONTEXT (jsc_context_get_type ())
G_DECLARE_FINAL_TYPE (JSCContext, jsc_context, JSC, CONTEXT, GObject)

#define JSC_TYPE_VALUE (jsc_value_get_type ())
G_DECLARE_FINAL_TYPE (JSCValue, jsc_value, JSC, VALUE, GObject)

#define JSC_TYPE_EXCEPTION (jsc_exception_get_type ())
G_DECLARE_FINAL_TYPE (JSCException, jsc_exception, JSC, EXCEPTION, GObject)

#define JSC_TYPE_WEAK_VALUE (jsc_weak_value_get_type ())
G_DECLARE_FINAL_TYPE (JSCWeakValue, jsc_weak_value, JSC, WEAK_VALUE, GObject)

JSCContext *jsc_context_new (void);
JSCContext *jsc_context_get_current (void);
JSCValue *jsc_context_evaluate (JSCContext *context, const char *code, gssize length);
JSCValue *jsc_context_evaluate_with_source_uri (JSCContext *context, const char *code, gssize length, const char *uri, guint line_number);
JSCValue *jsc_context_evaluate_in_object (JSCContext *context, const char *code, gssize length, gpointer object_instance, JSCClass *object_class, const char *uri, guint line_number, JSCValue **object);
JSCException *jsc_context_get_exception (JSCContext *context);
void jsc_context_throw (JSCContext *context, const char *error_message);
void jsc_context_throw_exception (JSCContext *context, JSCException *exception);
void jsc_context_push_exception_handler (JSCContext *context, JSCExceptionHandler handler, gpointer user_data, GDestroyNotify destroy_notify);
JSCValue *jsc_context_get_global_object (JSCContext *context);
JSCValue *jsc_context_get_value (JSCContext *context, const char *name);
void jsc_context_set_value (JSCContext *context, const char *name, JSCValue *value);
JSCClass *jsc_context_register_class (JSCContext *context, const char *name, JSCClass *parent_class, gpointer vtable, GDestroyNotify destroy_notify);

void jsc_class_add_method (JSCClass *jsc_class, const char *name, GCallback callback, gpointer user_data, GDestroyNotify destroy_notify, GType return_type, guint n_params, ...);
void jsc_class_add_property (JSCClass *jsc_class, const char *name, GType property_type, GCallback getter, GCallback setter, gpointer user_data, GDestroyNotify destroy_notify);

const char *jsc_exception_get_message (JSCException *exception);
char *jsc_exception_report (JSCException *exception);

JSCContext *jsc_value_get_context (JSCValue *value);
gboolean jsc_value_is_undefined (JSCValue *value);
gboolean jsc_value_is_null (JSCValue *value);
gboolean jsc_value_is_boolean (JSCValue *value);
gboolean jsc_value_is_number (JSCValue *value);
gboolean jsc_value_is_string (JSCValue *value);
gboolean jsc_value_is_object (JSCValue *value);
gboolean jsc_value_is_array (JSCValue *value);
gboolean jsc_value_is_function (JSCValue *value);

JSCValue *jsc_value_new_undefined (JSCContext *context);
JSCValue *jsc_value_new_boolean (JSCContext *context, gboolean value);
JSCValue *jsc_value_new_number (JSCContext *context, double number);
JSCValue *jsc_value_new_string (JSCContext *context, const char *string);
JSCValue *jsc_value_new_from_json (JSCContext *context, const char *json);
JSCValue *jsc_value_new_object (JSCContext *context, gpointer instance, JSCClass *jsc_class);
JSCValue *jsc_value_new_array_from_garray (JSCContext *context, GPtrArray *array);
JSCValue *jsc_value_new_function (JSCContext *context, const char *name, GCallback callback, gpointer user_data, GDestroyNotify destroy_notify, GType return_type, guint n_params, ...);

gboolean jsc_value_to_boolean (JSCValue *value);
double jsc_value_to_double (JSCValue *value);
gint32 jsc_value_to_int32 (JSCValue *value);
char *jsc_value_to_string (JSCValue *value);
char *jsc_value_to_json (JSCValue *value, guint indent);

JSCValue *jsc_value_object_get_property (JSCValue *value, const char *name);
void jsc_value_object_set_property (JSCValue *value, const char *name, JSCValue *property);
gboolean jsc_value_object_has_property (JSCValue *value, const char *name);
JSCValue *jsc_value_object_get_property_at_index (JSCValue *value, guint index);
JSCValue *jsc_value_object_invoke_method (JSCValue *value, const char *name, GType first_parameter_type, ...);
JSCValue *jsc_value_function_call (JSCValue *value, GType first_parameter_type, ...);
JSCValue *jsc_value_constructor_call (JSCValue *value, GType first_parameter_type, ...);

JSCWeakValue *jsc_weak_value_new (JSCValue *value);
JSCValue *jsc_weak_value_get_value (JSCWeakValue *weak_value);

G_END_DECLS
