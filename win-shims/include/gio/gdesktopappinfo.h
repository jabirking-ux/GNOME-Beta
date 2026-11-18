#pragma once
#include <gio/gio.h>

typedef struct _GDesktopAppInfo GDesktopAppInfo;

#define G_IS_DESKTOP_APP_INFO(obj) ((void)(obj), FALSE)
#define G_DESKTOP_APP_INFO(obj) ((GDesktopAppInfo *)(obj))

G_DEFINE_AUTOPTR_CLEANUP_FUNC (GDesktopAppInfo, g_object_unref)

static inline GDesktopAppInfo *g_desktop_app_info_new_from_filename (const char *filename) {
  (void)filename; return NULL;
}
static inline GDesktopAppInfo *g_desktop_app_info_new (const char *desktop_id) {
  (void)desktop_id; return NULL;
}
static inline const char *g_desktop_app_info_get_filename (GDesktopAppInfo *info) {
  (void)info; return NULL;
}
static inline gboolean g_desktop_app_info_get_boolean (GDesktopAppInfo *info, const char *key) {
  (void)info; (void)key; return FALSE;
}
static inline char *g_desktop_app_info_get_string (GDesktopAppInfo *info, const char *key) {
  (void)info; (void)key; return NULL;
}
static inline GList *g_desktop_app_info_get_implementations (const char *interface) {
  (void)interface; return NULL;
}
