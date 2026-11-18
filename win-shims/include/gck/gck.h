#pragma once
#include <glib-object.h>
#include <gio/gio.h>

G_BEGIN_DECLS

#define GCK_INVALID ((gulong)-1)
#define GCK_SESSION_READ_ONLY 0
#define GCK_URI_FOR_OBJECT_ON_TOKEN 1
#define CKA_CLASS 0x00000000UL
#define CKA_LABEL 0x00000003UL
#define CKA_VALUE 0x00000011UL
#define CKA_ISSUER 0x00000081UL
#define CKA_ID 0x00000102UL
#define CKO_CERTIFICATE 0x00000001UL
#define CKU_USER 1UL
#define CKF_TOKEN_INITIALIZED 0x00000400UL
#define CKF_LOGIN_REQUIRED 0x00000004UL

typedef gulong CK_OBJECT_CLASS;

typedef struct _GckSlot GckSlot;
typedef struct _GckSession GckSession;
typedef struct _GckObject GckObject;
typedef struct _GckAttributes GckAttributes;
typedef struct _GckEnumerator GckEnumerator;
typedef struct _GckModule GckModule;

typedef struct {
  gchar *slot_description;
  gchar *manufacturer_id;
  gulong flags;
} GckSlotInfo;

typedef struct {
  gchar *label;
  gchar *manufacturer_id;
  gchar *model;
  gchar *serial_number;
  gulong flags;
} GckTokenInfo;

typedef struct {
  gulong type;
  const guchar *value;
  gulong length;
} GckAttribute;

typedef struct {
  gpointer unused[4];
} GckBuilder;

typedef struct {
  GckTokenInfo *token_info;
  GckAttributes *attributes;
  gboolean any_unrecognized;
} GckUriData;

#define GCK_BUILDER_INIT { { 0 } }
#define GCK_URI_DATA_INIT { 0 }

#define GCK_TYPE_SLOT (gck_slot_get_type ())
G_DECLARE_FINAL_TYPE (GckSlot, gck_slot, GCK, SLOT, GObject)
#define GCK_TYPE_SESSION (gck_session_get_type ())
G_DECLARE_FINAL_TYPE (GckSession, gck_session, GCK, SESSION, GObject)
#define GCK_TYPE_OBJECT (gck_object_get_type ())
G_DECLARE_FINAL_TYPE (GckObject, gck_object, GCK, OBJECT, GObject)
#define GCK_TYPE_ENUMERATOR (gck_enumerator_get_type ())
G_DECLARE_FINAL_TYPE (GckEnumerator, gck_enumerator, GCK, ENUMERATOR, GObject)

GckSlotInfo *gck_slot_get_info (GckSlot *self);
void gck_slot_info_free (GckSlotInfo *slot_info);
GckTokenInfo *gck_slot_get_token_info (GckSlot *self);
void gck_token_info_free (GckTokenInfo *token_info);
void gck_slot_open_session_async (GckSlot *self, guint options, gpointer interaction, GCancellable *cancellable,
                                  GAsyncReadyCallback callback, gpointer user_data);
GckSession *gck_slot_open_session_finish (GckSlot *self, GAsyncResult *result, GError **error);

GckSlot *gck_session_get_slot (GckSession *self);
void gck_session_login_async (GckSession *self, gulong user_type, const guchar *pin, gsize n_pin,
                              GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean gck_session_login_finish (GckSession *self, GAsyncResult *result, GError **error);
void gck_session_logout_async (GckSession *self, GCancellable *cancellable,
                               GAsyncReadyCallback callback, gpointer user_data);
gboolean gck_session_logout_finish (GckSession *self, GAsyncResult *result, GError **error);
GckEnumerator *gck_session_enumerate_objects (GckSession *self, GckAttributes *match);

void gck_object_get_async (GckObject *self, const gulong *attr_types, guint n_attr_types,
                           GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
GckAttributes *gck_object_get_finish (GckObject *self, GAsyncResult *result, GError **error);

GckAttributes *gck_attributes_new_empty (gulong first_type, ...);
const GckAttribute *gck_attributes_find (GckAttributes *self, gulong attr_type);
gboolean gck_attributes_find_ulong (GckAttributes *self, gulong attr_type, gulong *value);
void gck_attributes_unref (GckAttributes *self);

void gck_builder_init (GckBuilder *builder);
void gck_builder_add_string (GckBuilder *builder, gulong attr_type, const gchar *value);
void gck_builder_add_data (GckBuilder *builder, gulong attr_type, const guchar *value, gsize length);
GckAttributes *gck_builder_end (GckBuilder *builder);

void gck_enumerator_next_async (GckEnumerator *self, gint max_objects, GCancellable *cancellable,
                                GAsyncReadyCallback callback, gpointer user_data);
GList *gck_enumerator_next_finish (GckEnumerator *self, GAsyncResult *result, GError **error);

void gck_modules_initialize_registered_async (GCancellable *cancellable,
                                              GAsyncReadyCallback callback, gpointer user_data);
GList *gck_modules_initialize_registered_finish (GAsyncResult *result, GError **error);
GList *gck_modules_get_slots (GList *modules, gboolean token_present);
void gck_list_unref_free (GList *reflist);

gchar *gck_uri_data_build (GckUriData *uri_data, guint flags);

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GckAttributes, gck_attributes_unref)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GckSlotInfo, gck_slot_info_free)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GckTokenInfo, gck_token_info_free)
G_DEFINE_AUTOPTR_CLEANUP_FUNC (GckModule, g_object_unref)

G_END_DECLS
