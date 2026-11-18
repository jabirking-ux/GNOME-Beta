#pragma once
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

typedef enum {
  GCR_CERTIFICATE_SECTION_NONE = 0,
  GCR_CERTIFICATE_SECTION_IMPORTANT = 1 << 0
} GcrCertificateSectionFlags;

#define GCR_TYPE_CERTIFICATE (gcr_certificate_get_type ())
G_DECLARE_INTERFACE (GcrCertificate, gcr_certificate, GCR, CERTIFICATE, GObject)

struct _GcrCertificateInterface {
  GTypeInterface parent;
};

#define GCR_TYPE_SIMPLE_CERTIFICATE (gcr_simple_certificate_get_type ())
G_DECLARE_FINAL_TYPE (GcrSimpleCertificate, gcr_simple_certificate, GCR, SIMPLE_CERTIFICATE, GObject)

#define GCR_TYPE_CERTIFICATE_SECTION (gcr_certificate_section_get_type ())
G_DECLARE_FINAL_TYPE (GcrCertificateSection, gcr_certificate_section, GCR, CERTIFICATE_SECTION, GObject)

#define GCR_TYPE_CERTIFICATE_FIELD (gcr_certificate_field_get_type ())
G_DECLARE_FINAL_TYPE (GcrCertificateField, gcr_certificate_field, GCR, CERTIFICATE_FIELD, GObject)

GcrCertificate *gcr_simple_certificate_new (const guchar *data, gsize n_data);
GList *gcr_certificate_get_interface_elements (GcrCertificate *self);

const char *gcr_certificate_section_get_label (GcrCertificateSection *self);
GListModel *gcr_certificate_section_get_fields (GcrCertificateSection *self);
GcrCertificateSectionFlags gcr_certificate_section_get_flags (GcrCertificateSection *self);

const char *gcr_certificate_field_get_label (GcrCertificateField *self);
GType gcr_certificate_field_get_value_type (GcrCertificateField *self);
gboolean gcr_certificate_field_get_value (GcrCertificateField *self, GValue *value);

G_END_DECLS
