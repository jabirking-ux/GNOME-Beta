#define _GNU_SOURCE
#include <windows.h>
#include <io.h>
#include <shellapi.h>
#include <wincrypt.h>
#include <glib.h>
#include <gio/gio.h>
#include <gtk/gtk.h>
#include <string.h>

#include "pwquality.h"
#include "gio/gunixfdlist.h"
#include "gio/gunixoutputstream.h"
#include "libportal-gtk4/portal-gtk4.h"
#include "gck/gck.h"
#include "gcr-4/gcr/gcr.h"
#include "jsc/jsc.h"

/* ---------------- pwquality ---------------- */
struct _pwquality_settings { int dummy; };

pwquality_settings_t *pwquality_default_settings (void) {
  return g_new0 (pwquality_settings_t, 1);
}

int pwquality_generate (pwquality_settings_t *pwq, int entropy_bits, char **password) {
  (void)pwq;
  int len = (entropy_bits >= 96) ? 20 : 16;
  static const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz23456789!@#$%^&*-_";
  char *out = g_malloc (len + 1);
  for (int i = 0; i < len; i++)
    out[i] = charset[g_random_int_range (0, (gint32)(sizeof (charset) - 1))];
  out[len] = '\0';
  if (password) *password = out;
  else g_free (out);
  return 0;
}

const char *pwquality_strerror (char *buf, size_t len, int errcode, void *auxerror) {
  (void)buf; (void)len; (void)errcode; (void)auxerror;
  return "Unknown password quality error";
}

void pwquality_free_settings (pwquality_settings_t *pwq) {
  g_free (pwq);
}

/* ---------------- gio-unix ---------------- */
struct _GUnixFDList { GObject parent; GArray *fds; };
typedef struct { GObjectClass parent_class; } GUnixFDListClass;
G_DEFINE_TYPE (GUnixFDList, g_unix_fd_list, G_TYPE_OBJECT)
static void g_unix_fd_list_finalize (GObject *obj) {
  GUnixFDList *self = (GUnixFDList *)obj;
  if (self->fds) g_array_unref (self->fds);
  G_OBJECT_CLASS (g_unix_fd_list_parent_class)->finalize (obj);
}
static void g_unix_fd_list_class_init (GUnixFDListClass *klass) {
  G_OBJECT_CLASS (klass)->finalize = g_unix_fd_list_finalize;
}
static void g_unix_fd_list_init (GUnixFDList *self) {
  self->fds = g_array_new (FALSE, FALSE, sizeof (gint));
}
GUnixFDList *g_unix_fd_list_new (void) { return g_object_new (g_unix_fd_list_get_type (), NULL); }
gint g_unix_fd_list_get_length (GUnixFDList *list) { return list && list->fds ? (gint)list->fds->len : 0; }
gint g_unix_fd_list_get (GUnixFDList *list, gint index_, GError **error) {
  (void)error;
  if (!list || !list->fds || index_ < 0 || (guint)index_ >= list->fds->len) return -1;
  return g_array_index (list->fds, gint, index_);
}
gint g_unix_fd_list_append (GUnixFDList *list, gint fd, GError **error) {
  (void)error;
  if (!list || !list->fds) return -1;
  g_array_append_val (list->fds, fd);
  return (gint)(list->fds->len - 1);
}

struct _GUnixOutputStream { GOutputStream parent; gint fd; gboolean close_fd; };
typedef struct { GOutputStreamClass parent_class; } GUnixOutputStreamClass;
G_DEFINE_TYPE (GUnixOutputStream, g_unix_output_stream, G_TYPE_OUTPUT_STREAM)
static gssize win_unix_write (GOutputStream *stream, const void *buffer, gsize count, GCancellable *cancellable, GError **error) {
  (void)cancellable; (void)error;
  GUnixOutputStream *self = (GUnixOutputStream *)stream;
  return _write (self->fd, buffer, (unsigned int)count);
}
static gboolean win_unix_close (GOutputStream *stream, GCancellable *cancellable, GError **error) {
  (void)cancellable; (void)error;
  GUnixOutputStream *self = (GUnixOutputStream *)stream;
  if (self->close_fd && self->fd >= 0) { _close (self->fd); self->fd = -1; }
  return TRUE;
}
static void g_unix_output_stream_class_init (GUnixOutputStreamClass *klass) {
  GOutputStreamClass *ok = G_OUTPUT_STREAM_CLASS (klass);
  ok->write_fn = win_unix_write;
  ok->close_fn = win_unix_close;
}
static void g_unix_output_stream_init (GUnixOutputStream *self) { self->fd = -1; }
GOutputStream *g_unix_output_stream_new (gint fd, gboolean close_fd) {
  GUnixOutputStream *s = g_object_new (g_unix_output_stream_get_type (), NULL);
  s->fd = fd;
  s->close_fd = close_fd;
  return G_OUTPUT_STREAM (s);
}

/* ---------------- libportal-gtk4 ---------------- */
struct _XdpPortal { GObject parent; };
G_DEFINE_TYPE (XdpPortal, xdp_portal, G_TYPE_OBJECT)
static void xdp_portal_class_init (XdpPortalClass *klass) { (void)klass; }
static void xdp_portal_init (XdpPortal *self) { (void)self; }

struct _XdpParent { GtkWindow *window; };

gboolean xdp_portal_running_under_sandbox (void) { return FALSE; }
XdpPortal *xdp_portal_new (void) { return g_object_new (XDP_TYPE_PORTAL, NULL); }
XdpParent *xdp_parent_new_gtk (GtkWindow *window) {
  XdpParent *p = g_new0 (XdpParent, 1);
  p->window = window;
  return p;
}
void xdp_parent_free (XdpParent *parent) { g_free (parent); }

static void complete_async_bool (gpointer source, GAsyncReadyCallback cb, gpointer data, gboolean ok) {
  GTask *task = g_task_new (source, NULL, cb, data);
  g_task_return_boolean (task, ok);
  g_object_unref (task);
}

void xdp_portal_set_wallpaper (XdpPortal *portal, XdpParent *parent, const char *uri,
                               XdpWallpaperFlags flags, GCancellable *cancellable,
                               GAsyncReadyCallback callback, gpointer data) {
  (void)parent; (void)uri; (void)flags; (void)cancellable;
  complete_async_bool (portal, callback, data, FALSE);
}
gboolean xdp_portal_set_wallpaper_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_boolean (G_TASK (result), error);
}
void xdp_portal_set_background_status (XdpPortal *portal, const char *status_message,
                                       GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data) {
  (void)status_message; (void)cancellable;
  complete_async_bool (portal, callback, data, TRUE);
}
gboolean xdp_portal_set_background_status_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_boolean (G_TASK (result), error);
}
void xdp_portal_request_background (XdpPortal *portal, XdpParent *parent, const char *reason,
                                    GPtrArray *commandline, XdpBackgroundFlags flags,
                                    GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)parent; (void)reason; (void)commandline; (void)flags; (void)cancellable;
  complete_async_bool (portal, callback, user_data, TRUE);
}
gboolean xdp_portal_request_background_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_boolean (G_TASK (result), error);
}
gboolean xdp_portal_dynamic_launcher_uninstall (XdpPortal *portal, const char *desktop_file_id, GError **error) {
  (void)portal; (void)desktop_file_id; (void)error;
  return FALSE;
}
gboolean xdp_portal_dynamic_launcher_install (XdpPortal *portal, const char *token, const char *desktop_file_id,
                                              const char *desktop_entry, GError **error) {
  (void)portal; (void)token; (void)desktop_file_id; (void)desktop_entry; (void)error;
  return FALSE;
}
char *xdp_portal_dynamic_launcher_get_desktop_entry (XdpPortal *portal, const char *desktop_file_id, GError **error) {
  (void)portal; (void)desktop_file_id; (void)error;
  return NULL;
}
gboolean xdp_portal_dynamic_launcher_launch (XdpPortal *portal, const char *desktop_file_id,
                                             const char *activation_token, GError **error) {
  (void)portal; (void)desktop_file_id; (void)activation_token; (void)error;
  return FALSE;
}
GVariant *xdp_portal_dynamic_launcher_get_icon (XdpPortal *portal, const char *desktop_file_id,
                                                char **icon_format, guint *icon_size, GError **error) {
  (void)portal; (void)desktop_file_id; (void)icon_format; (void)icon_size; (void)error;
  return NULL;
}
void xdp_portal_dynamic_launcher_prepare_install (XdpPortal *portal, XdpParent *parent, const char *name,
                                                  GVariant *icon_v, XdpLauncherType launcher_type,
                                                  const char *target, gboolean editable_name, gboolean editable_icon,
                                                  GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data) {
  (void)parent; (void)name; (void)icon_v; (void)launcher_type; (void)target;
  (void)editable_name; (void)editable_icon; (void)cancellable;
  GTask *t = g_task_new (portal, NULL, callback, data);
  g_task_return_pointer (t, NULL, NULL);
  g_object_unref (t);
}
GVariant *xdp_portal_dynamic_launcher_prepare_install_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_pointer (G_TASK (result), error);
}

static void win_shell_open (const char *uri) {
  if (!uri) return;
  g_autofree gunichar2 *wuri = g_utf8_to_utf16 (uri, -1, NULL, NULL, NULL);
  if (wuri) ShellExecuteW (NULL, L"open", wuri, NULL, NULL, SW_SHOWNORMAL);
}

void xdp_portal_open_uri (XdpPortal *portal, XdpParent *parent, const char *uri,
                          guint flags, GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer data) {
  (void)parent; (void)flags; (void)cancellable;
  win_shell_open (uri);
  complete_async_bool (portal, callback, data, TRUE);
}
gboolean xdp_portal_open_uri_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_boolean (G_TASK (result), error);
}
void xdp_portal_open_directory (XdpPortal *portal, XdpParent *parent, const char *uri,
                                guint flags, GCancellable *cancellable,
                                GAsyncReadyCallback callback, gpointer data) {
  (void)parent; (void)flags; (void)cancellable;
  g_autofree char *path = g_filename_from_uri (uri, NULL, NULL);
  if (path) {
    g_autofree char *arg = g_strdup_printf ("/select,\"%s\"", path);
    g_autofree gunichar2 *warg = g_utf8_to_utf16 (arg, -1, NULL, NULL, NULL);
    ShellExecuteW (NULL, L"open", L"explorer.exe", warg, NULL, SW_SHOWNORMAL);
  }
  complete_async_bool (portal, callback, data, TRUE);
}
gboolean xdp_portal_open_directory_finish (XdpPortal *portal, GAsyncResult *result, GError **error) {
  (void)portal;
  return g_task_propagate_boolean (G_TASK (result), error);
}

/* ---------------- gck-2 ---------------- */
struct _GckSlot { GObject parent; };
struct _GckSession { GObject parent; GckSlot *slot; };
struct _GckObject { GObject parent; };
struct _GckEnumerator { GObject parent; };
struct _GckAttributes { int ref_count; };

G_DEFINE_TYPE (GckSlot, gck_slot, G_TYPE_OBJECT)
static void gck_slot_class_init (GckSlotClass *k) { (void)k; }
static void gck_slot_init (GckSlot *s) { (void)s; }

G_DEFINE_TYPE (GckSession, gck_session, G_TYPE_OBJECT)
static void gck_session_class_init (GckSessionClass *k) { (void)k; }
static void gck_session_init (GckSession *s) { (void)s; }

G_DEFINE_TYPE (GckObject, gck_object, G_TYPE_OBJECT)
static void gck_object_class_init (GckObjectClass *k) { (void)k; }
static void gck_object_init (GckObject *s) { (void)s; }

G_DEFINE_TYPE (GckEnumerator, gck_enumerator, G_TYPE_OBJECT)
static void gck_enumerator_class_init (GckEnumeratorClass *k) { (void)k; }
static void gck_enumerator_init (GckEnumerator *s) { (void)s; }

GckSlotInfo *gck_slot_get_info (GckSlot *self) { (void)self; return g_new0 (GckSlotInfo, 1); }
void gck_slot_info_free (GckSlotInfo *info) {
  if (!info) return;
  g_free (info->slot_description);
  g_free (info->manufacturer_id);
  g_free (info);
}
GckTokenInfo *gck_slot_get_token_info (GckSlot *self) { (void)self; return g_new0 (GckTokenInfo, 1); }
void gck_token_info_free (GckTokenInfo *info) {
  if (!info) return;
  g_free (info->label); g_free (info->manufacturer_id); g_free (info->model); g_free (info->serial_number);
  g_free (info);
}
void gck_slot_open_session_async (GckSlot *self, guint options, gpointer interaction, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)options; (void)interaction; (void)cancellable;
  complete_async_bool (self, callback, user_data, FALSE);
}
GckSession *gck_slot_open_session_finish (GckSlot *self, GAsyncResult *result, GError **error) {
  (void)self; (void)result; (void)error;
  return NULL;
}
GckSlot *gck_session_get_slot (GckSession *self) { return self ? self->slot : NULL; }
void gck_session_login_async (GckSession *self, gulong user_type, const guchar *pin, gsize n_pin, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)user_type; (void)pin; (void)n_pin; (void)cancellable;
  complete_async_bool (self, callback, user_data, FALSE);
}
gboolean gck_session_login_finish (GckSession *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_boolean (G_TASK (result), error);
}
void gck_session_logout_async (GckSession *self, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)cancellable;
  complete_async_bool (self, callback, user_data, TRUE);
}
gboolean gck_session_logout_finish (GckSession *self, GAsyncResult *result, GError **error) {
  (void)self; return g_task_propagate_boolean (G_TASK (result), error);
}
GckEnumerator *gck_session_enumerate_objects (GckSession *self, GckAttributes *match) {
  (void)self; (void)match;
  return g_object_new (GCK_TYPE_ENUMERATOR, NULL);
}
void gck_object_get_async (GckObject *self, const gulong *attr_types, guint n_attr_types, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)attr_types; (void)n_attr_types; (void)cancellable;
  complete_async_bool (self, callback, user_data, FALSE);
}
GckAttributes *gck_object_get_finish (GckObject *self, GAsyncResult *result, GError **error) {
  (void)self; (void)result; (void)error;
  return NULL;
}
GckAttributes *gck_attributes_new_empty (gulong first_type, ...) {
  (void)first_type;
  GckAttributes *a = g_new0 (GckAttributes, 1);
  a->ref_count = 1;
  return a;
}
const GckAttribute *gck_attributes_find (GckAttributes *self, gulong attr_type) { (void)self; (void)attr_type; return NULL; }
gboolean gck_attributes_find_ulong (GckAttributes *self, gulong attr_type, gulong *value) { (void)self; (void)attr_type; (void)value; return FALSE; }
void gck_attributes_unref (GckAttributes *self) {
  if (self && g_atomic_int_dec_and_test (&self->ref_count)) g_free (self);
}
void gck_builder_init (GckBuilder *builder) { memset (builder, 0, sizeof (*builder)); }
void gck_builder_add_string (GckBuilder *builder, gulong attr_type, const gchar *value) { (void)builder; (void)attr_type; (void)value; }
void gck_builder_add_data (GckBuilder *builder, gulong attr_type, const guchar *value, gsize length) { (void)builder; (void)attr_type; (void)value; (void)length; }
GckAttributes *gck_builder_end (GckBuilder *builder) { (void)builder; return gck_attributes_new_empty (GCK_INVALID); }
void gck_enumerator_next_async (GckEnumerator *self, gint max_objects, GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)max_objects; (void)cancellable;
  complete_async_bool (self, callback, user_data, TRUE);
}
GList *gck_enumerator_next_finish (GckEnumerator *self, GAsyncResult *result, GError **error) {
  (void)self; (void)result; (void)error;
  return NULL;
}
void gck_modules_initialize_registered_async (GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data) {
  (void)cancellable;
  GTask *task = g_task_new (NULL, NULL, callback, user_data);
  g_task_return_pointer (task, NULL, NULL);
  g_object_unref (task);
}
GList *gck_modules_initialize_registered_finish (GAsyncResult *result, GError **error) {
  return g_task_propagate_pointer (G_TASK (result), error);
}
GList *gck_modules_get_slots (GList *modules, gboolean token_present) { (void)modules; (void)token_present; return NULL; }
void gck_list_unref_free (GList *reflist) { g_list_free_full (reflist, g_object_unref); }
gchar *gck_uri_data_build (GckUriData *uri_data, guint flags) { (void)uri_data; (void)flags; return g_strdup ("pkcs11:"); }

/* ---------------- gcr-4 ---------------- */
G_DEFINE_INTERFACE (GcrCertificate, gcr_certificate, G_TYPE_OBJECT)
static void gcr_certificate_default_init (GcrCertificateInterface *iface) { (void)iface; }

struct _GcrCertificateField { GObject parent; char *label; char *value; };
G_DEFINE_TYPE (GcrCertificateField, gcr_certificate_field, G_TYPE_OBJECT)
static void gcr_certificate_field_finalize (GObject *obj) {
  GcrCertificateField *self = GCR_CERTIFICATE_FIELD (obj);
  g_free (self->label); g_free (self->value);
  G_OBJECT_CLASS (gcr_certificate_field_parent_class)->finalize (obj);
}
static void gcr_certificate_field_class_init (GcrCertificateFieldClass *k) { G_OBJECT_CLASS (k)->finalize = gcr_certificate_field_finalize; }
static void gcr_certificate_field_init (GcrCertificateField *s) { (void)s; }
const char *gcr_certificate_field_get_label (GcrCertificateField *self) { return self->label; }
GType gcr_certificate_field_get_value_type (GcrCertificateField *self) { (void)self; return G_TYPE_STRING; }
gboolean gcr_certificate_field_get_value (GcrCertificateField *self, GValue *value) {
  g_value_init (value, G_TYPE_STRING);
  g_value_set_string (value, self->value ? self->value : "");
  return TRUE;
}

struct _GcrCertificateSection { GObject parent; char *label; GListStore *fields; GcrCertificateSectionFlags flags; };
G_DEFINE_TYPE (GcrCertificateSection, gcr_certificate_section, G_TYPE_OBJECT)
static void gcr_certificate_section_finalize (GObject *obj) {
  GcrCertificateSection *self = GCR_CERTIFICATE_SECTION (obj);
  g_free (self->label);
  g_clear_object (&self->fields);
  G_OBJECT_CLASS (gcr_certificate_section_parent_class)->finalize (obj);
}
static void gcr_certificate_section_class_init (GcrCertificateSectionClass *k) { G_OBJECT_CLASS (k)->finalize = gcr_certificate_section_finalize; }
static void gcr_certificate_section_init (GcrCertificateSection *s) { s->fields = g_list_store_new (GCR_TYPE_CERTIFICATE_FIELD); }
const char *gcr_certificate_section_get_label (GcrCertificateSection *self) { return self->label; }
GListModel *gcr_certificate_section_get_fields (GcrCertificateSection *self) { return G_LIST_MODEL (self->fields); }
GcrCertificateSectionFlags gcr_certificate_section_get_flags (GcrCertificateSection *self) { return self->flags; }

struct _GcrSimpleCertificate { GObject parent; GBytes *der; };
static void gcr_simple_certificate_iface_init (GcrCertificateInterface *iface) { (void)iface; }
G_DEFINE_TYPE_WITH_CODE (GcrSimpleCertificate, gcr_simple_certificate, G_TYPE_OBJECT,
                         G_IMPLEMENT_INTERFACE (GCR_TYPE_CERTIFICATE, gcr_simple_certificate_iface_init))
static void gcr_simple_certificate_finalize (GObject *obj) {
  GcrSimpleCertificate *self = GCR_SIMPLE_CERTIFICATE (obj);
  g_clear_pointer (&self->der, g_bytes_unref);
  G_OBJECT_CLASS (gcr_simple_certificate_parent_class)->finalize (obj);
}
static void gcr_simple_certificate_class_init (GcrSimpleCertificateClass *k) { G_OBJECT_CLASS (k)->finalize = gcr_simple_certificate_finalize; }
static void gcr_simple_certificate_init (GcrSimpleCertificate *s) { (void)s; }

GcrCertificate *gcr_simple_certificate_new (const guchar *data, gsize n_data) {
  GcrSimpleCertificate *cert = g_object_new (GCR_TYPE_SIMPLE_CERTIFICATE, NULL);
  cert->der = g_bytes_new (data, n_data);
  return GCR_CERTIFICATE (cert);
}

static void add_cert_field (GcrCertificateSection *sec, const char *label, const char *val) {
  GcrCertificateField *f = g_object_new (GCR_TYPE_CERTIFICATE_FIELD, NULL);
  f->label = g_strdup (label);
  f->value = g_strdup (val ? val : "");
  g_list_store_append (sec->fields, f);
  g_object_unref (f);
}

GList *gcr_certificate_get_interface_elements (GcrCertificate *self) {
  GcrCertificateSection *sec = g_object_new (GCR_TYPE_CERTIFICATE_SECTION, NULL);
  sec->label = g_strdup ("Certificate");
  sec->flags = GCR_CERTIFICATE_SECTION_IMPORTANT;

  if (GCR_IS_SIMPLE_CERTIFICATE (self)) {
    GcrSimpleCertificate *sc = GCR_SIMPLE_CERTIFICATE (self);
    gsize len = 0;
    const guchar *data = g_bytes_get_data (sc->der, &len);
    PCCERT_CONTEXT ctx = CertCreateCertificateContext (X509_ASN_ENCODING, data, (DWORD)len);
    if (ctx) {
      char buf[512];
      if (CertGetNameStringA (ctx, CERT_NAME_SIMPLE_DISPLAY_TYPE, 0, NULL, buf, sizeof (buf)) > 1)
        add_cert_field (sec, "Subject", buf);
      if (CertGetNameStringA (ctx, CERT_NAME_SIMPLE_DISPLAY_TYPE, CERT_NAME_ISSUER_FLAG, NULL, buf, sizeof (buf)) > 1)
        add_cert_field (sec, "Issuer", buf);
      CertFreeCertificateContext (ctx);
    } else {
      add_cert_field (sec, "Details", "X.509 Certificate");
    }
  }
  return g_list_append (NULL, sec);
}

/* ---------------- JSC minimal GObject implementation ---------------- */
struct _JSCContext { GObject parent; JSCException *last_exc; };
struct _JSCValue { GObject parent; JSCContext *ctx; int kind; gboolean b; double d; char *s; };
struct _JSCException { GObject parent; char *msg; };
struct _JSCWeakValue { GObject parent; JSCValue *val; };

G_DEFINE_TYPE (JSCContext, jsc_context, G_TYPE_OBJECT)
static void jsc_context_finalize (GObject *o) { g_clear_object (&JSC_CONTEXT (o)->last_exc); G_OBJECT_CLASS (jsc_context_parent_class)->finalize (o); }
static void jsc_context_class_init (JSCContextClass *k) { G_OBJECT_CLASS (k)->finalize = jsc_context_finalize; }
static void jsc_context_init (JSCContext *s) { (void)s; }

G_DEFINE_TYPE (JSCValue, jsc_value, G_TYPE_OBJECT)
static void jsc_value_finalize (GObject *o) { JSCValue *v = JSC_VALUE (o); g_free (v->s); g_clear_object (&v->ctx); G_OBJECT_CLASS (jsc_value_parent_class)->finalize (o); }
static void jsc_value_class_init (JSCValueClass *k) { G_OBJECT_CLASS (k)->finalize = jsc_value_finalize; }
static void jsc_value_init (JSCValue *s) { (void)s; }

G_DEFINE_TYPE (JSCException, jsc_exception, G_TYPE_OBJECT)
static void jsc_exception_finalize (GObject *o) { g_free (JSC_EXCEPTION (o)->msg); G_OBJECT_CLASS (jsc_exception_parent_class)->finalize (o); }
static void jsc_exception_class_init (JSCExceptionClass *k) { G_OBJECT_CLASS (k)->finalize = jsc_exception_finalize; }
static void jsc_exception_init (JSCException *s) { (void)s; }

G_DEFINE_TYPE (JSCWeakValue, jsc_weak_value, G_TYPE_OBJECT)
static void jsc_weak_value_finalize (GObject *o) { g_clear_object (&JSC_WEAK_VALUE (o)->val); G_OBJECT_CLASS (jsc_weak_value_parent_class)->finalize (o); }
static void jsc_weak_value_class_init (JSCWeakValueClass *k) { G_OBJECT_CLASS (k)->finalize = jsc_weak_value_finalize; }
static void jsc_weak_value_init (JSCWeakValue *s) { (void)s; }

enum { K_UNDEF = 0, K_NULL, K_BOOL, K_NUM, K_STR, K_OBJ };

JSCContext *jsc_context_new (void) { return g_object_new (JSC_TYPE_CONTEXT, NULL); }
JSCContext *jsc_context_get_current (void) { return NULL; }
JSCValue *jsc_context_evaluate (JSCContext *context, const char *code, gssize length) {
  (void)code; (void)length; return jsc_value_new_undefined (context);
}
JSCValue *jsc_context_evaluate_with_source_uri (JSCContext *context, const char *code, gssize length, const char *uri, guint line_number) {
  (void)code; (void)length; (void)uri; (void)line_number; return jsc_value_new_undefined (context);
}
JSCValue *jsc_context_evaluate_in_object (JSCContext *context, const char *code, gssize length, gpointer object_instance, JSCClass *object_class, const char *uri, guint line_number, JSCValue **object) {
  (void)code; (void)length; (void)object_instance; (void)object_class; (void)uri; (void)line_number;
  if (object) *object = jsc_value_new_object (context, NULL, NULL);
  return jsc_value_new_undefined (context);
}
JSCException *jsc_context_get_exception (JSCContext *context) { return context ? context->last_exc : NULL; }
void jsc_context_throw (JSCContext *context, const char *error_message) {
  if (!context) return;
  g_clear_object (&context->last_exc);
  context->last_exc = g_object_new (JSC_TYPE_EXCEPTION, NULL);
  context->last_exc->msg = g_strdup (error_message);
}
void jsc_context_throw_exception (JSCContext *context, JSCException *exception) {
  if (!context) return;
  g_set_object (&context->last_exc, exception);
}
void jsc_context_push_exception_handler (JSCContext *context, JSCExceptionHandler handler, gpointer user_data, GDestroyNotify destroy_notify) {
  (void)context; (void)handler; (void)user_data; (void)destroy_notify;
}
JSCValue *jsc_context_get_global_object (JSCContext *context) { return jsc_value_new_object (context, NULL, NULL); }
JSCValue *jsc_context_get_value (JSCContext *context, const char *name) { (void)name; return jsc_value_new_undefined (context); }
void jsc_context_set_value (JSCContext *context, const char *name, JSCValue *value) { (void)context; (void)name; (void)value; }
JSCClass *jsc_context_register_class (JSCContext *context, const char *name, JSCClass *parent_class, gpointer vtable, GDestroyNotify destroy_notify) {
  (void)context; (void)name; (void)parent_class; (void)vtable; (void)destroy_notify; return NULL;
}
void jsc_class_add_method (JSCClass *jsc_class, const char *name, GCallback callback, gpointer user_data, GDestroyNotify destroy_notify, GType return_type, guint n_params, ...) {
  (void)jsc_class; (void)name; (void)callback; (void)user_data; (void)destroy_notify; (void)return_type; (void)n_params;
}
void jsc_class_add_property (JSCClass *jsc_class, const char *name, GType property_type, GCallback getter, GCallback setter, gpointer user_data, GDestroyNotify destroy_notify) {
  (void)jsc_class; (void)name; (void)property_type; (void)getter; (void)setter; (void)user_data; (void)destroy_notify;
}
const char *jsc_exception_get_message (JSCException *exception) { return exception && exception->msg ? exception->msg : ""; }
char *jsc_exception_report (JSCException *exception) { return g_strdup (jsc_exception_get_message (exception)); }

static JSCValue *make_val (JSCContext *ctx, int kind) {
  JSCValue *v = g_object_new (JSC_TYPE_VALUE, NULL);
  if (ctx) v->ctx = g_object_ref (ctx);
  v->kind = kind;
  return v;
}
JSCContext *jsc_value_get_context (JSCValue *value) { return value ? value->ctx : NULL; }
gboolean jsc_value_is_undefined (JSCValue *v) { return !v || v->kind == K_UNDEF; }
gboolean jsc_value_is_null (JSCValue *v) { return v && v->kind == K_NULL; }
gboolean jsc_value_is_boolean (JSCValue *v) { return v && v->kind == K_BOOL; }
gboolean jsc_value_is_number (JSCValue *v) { return v && v->kind == K_NUM; }
gboolean jsc_value_is_string (JSCValue *v) { return v && v->kind == K_STR; }
gboolean jsc_value_is_object (JSCValue *v) { return v && v->kind == K_OBJ; }
gboolean jsc_value_is_array (JSCValue *v) { (void)v; return FALSE; }
gboolean jsc_value_is_function (JSCValue *v) { (void)v; return FALSE; }

JSCValue *jsc_value_new_undefined (JSCContext *context) { return make_val (context, K_UNDEF); }
JSCValue *jsc_value_new_boolean (JSCContext *context, gboolean value) { JSCValue *v = make_val (context, K_BOOL); v->b = value; return v; }
JSCValue *jsc_value_new_number (JSCContext *context, double number) { JSCValue *v = make_val (context, K_NUM); v->d = number; return v; }
JSCValue *jsc_value_new_string (JSCContext *context, const char *string) { JSCValue *v = make_val (context, K_STR); v->s = g_strdup (string); return v; }
JSCValue *jsc_value_new_from_json (JSCContext *context, const char *json) { JSCValue *v = make_val (context, K_STR); v->s = g_strdup (json); return v; }
JSCValue *jsc_value_new_object (JSCContext *context, gpointer instance, JSCClass *jsc_class) { (void)instance; (void)jsc_class; return make_val (context, K_OBJ); }
JSCValue *jsc_value_new_array_from_garray (JSCContext *context, GPtrArray *array) { (void)array; return make_val (context, K_OBJ); }
JSCValue *jsc_value_new_function (JSCContext *context, const char *name, GCallback callback, gpointer user_data, GDestroyNotify destroy_notify, GType return_type, guint n_params, ...) {
  (void)name; (void)callback; (void)user_data; (void)destroy_notify; (void)return_type; (void)n_params;
  return make_val (context, K_OBJ);
}
gboolean jsc_value_to_boolean (JSCValue *v) { return v ? v->b : FALSE; }
double jsc_value_to_double (JSCValue *v) { return v ? v->d : 0.0; }
gint32 jsc_value_to_int32 (JSCValue *v) { return v ? (gint32)v->d : 0; }
char *jsc_value_to_string (JSCValue *v) { return v && v->s ? g_strdup (v->s) : g_strdup (""); }
char *jsc_value_to_json (JSCValue *v, guint indent) { (void)indent; return jsc_value_to_string (v); }
JSCValue *jsc_value_object_get_property (JSCValue *v, const char *name) { (void)name; return jsc_value_new_undefined (v ? v->ctx : NULL); }
void jsc_value_object_set_property (JSCValue *v, const char *name, JSCValue *property) { (void)v; (void)name; (void)property; }
gboolean jsc_value_object_has_property (JSCValue *v, const char *name) { (void)v; (void)name; return FALSE; }
JSCValue *jsc_value_object_get_property_at_index (JSCValue *v, guint index) { (void)index; return jsc_value_new_undefined (v ? v->ctx : NULL); }
JSCValue *jsc_value_object_invoke_method (JSCValue *v, const char *name, GType first_parameter_type, ...) { (void)name; (void)first_parameter_type; return jsc_value_new_undefined (v ? v->ctx : NULL); }
JSCValue *jsc_value_function_call (JSCValue *v, GType first_parameter_type, ...) { (void)first_parameter_type; return jsc_value_new_undefined (v ? v->ctx : NULL); }
JSCValue *jsc_value_constructor_call (JSCValue *v, GType first_parameter_type, ...) { (void)first_parameter_type; return jsc_value_new_undefined (v ? v->ctx : NULL); }

JSCWeakValue *jsc_weak_value_new (JSCValue *value) {
  JSCWeakValue *w = g_object_new (JSC_TYPE_WEAK_VALUE, NULL);
  if (value) w->val = g_object_ref (value);
  return w;
}
JSCValue *jsc_weak_value_get_value (JSCWeakValue *weak_value) {
  return weak_value && weak_value->val ? g_object_ref (weak_value->val) : NULL;
}
