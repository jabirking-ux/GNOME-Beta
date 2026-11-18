#pragma once
#include <gtk/gtk.h>

G_BEGIN_DECLS

#define XDP_TYPE_PORTAL (xdp_portal_get_type ())
G_DECLARE_FINAL_TYPE (XdpPortal, xdp_portal, XDP, PORTAL, GObject)

typedef enum {
  XDP_OPEN_URI_FLAG_NONE = 0,
  XDP_OPEN_URI_FLAG_ASK = 1 << 0,
  XDP_OPEN_URI_FLAG_WRITABLE = 1 << 1
} XdpOpenUriFlags;

typedef enum {
  XDP_WALLPAPER_FLAG_NONE = 0,
  XDP_WALLPAPER_FLAG_BACKGROUND = 1 << 0,
  XDP_WALLPAPER_FLAG_LOCKSCREEN = 1 << 1,
  XDP_WALLPAPER_FLAG_PREVIEW = 1 << 2
} XdpWallpaperFlags;

typedef enum {
  XDP_BACKGROUND_FLAG_NONE = 0,
  XDP_BACKGROUND_FLAG_AUTOSTART = 1 << 0,
  XDP_BACKGROUND_FLAG_ACTIVATABLE = 1 << 1
} XdpBackgroundFlags;

typedef enum {
  XDP_LAUNCHER_APPLICATION = 0,
  XDP_LAUNCHER_WEBAPP = 1
} XdpLauncherType;

typedef struct _XdpParent XdpParent;

gboolean xdp_portal_running_under_sandbox (void);
XdpPortal *xdp_portal_new (void);
XdpParent *xdp_parent_new_gtk (GtkWindow *window);
void xdp_parent_free (XdpParent *parent);
G_DEFINE_AUTOPTR_CLEANUP_FUNC (XdpParent, xdp_parent_free)

void xdp_portal_set_wallpaper (XdpPortal *portal, XdpParent *parent, const char *uri,
                               XdpWallpaperFlags flags, GCancellable *cancellable,
                               GAsyncReadyCallback callback, gpointer data);
gboolean xdp_portal_set_wallpaper_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

void xdp_portal_set_background_status (XdpPortal *portal, const char *status_message,
                                       GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
gboolean xdp_portal_set_background_status_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

void xdp_portal_request_background (XdpPortal *portal, XdpParent *parent, const char *reason,
                                    GPtrArray *commandline, XdpBackgroundFlags flags,
                                    GCancellable *cancellable, GAsyncReadyCallback callback, gpointer user_data);
gboolean xdp_portal_request_background_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

gboolean xdp_portal_dynamic_launcher_uninstall (XdpPortal *portal, const char *desktop_file_id, GError **error);
gboolean xdp_portal_dynamic_launcher_install (XdpPortal *portal, const char *token, const char *desktop_file_id,
                                              const char *desktop_entry, GError **error);
char *xdp_portal_dynamic_launcher_get_desktop_entry (XdpPortal *portal, const char *desktop_file_id, GError **error);
gboolean xdp_portal_dynamic_launcher_launch (XdpPortal *portal, const char *desktop_file_id,
                                             const char *activation_token, GError **error);
GVariant *xdp_portal_dynamic_launcher_get_icon (XdpPortal *portal, const char *desktop_file_id,
                                                char **icon_format, guint *icon_size, GError **error);
void xdp_portal_dynamic_launcher_prepare_install (XdpPortal *portal, XdpParent *parent, const char *name,
                                                  GVariant *icon_v, XdpLauncherType launcher_type,
                                                  const char *target, gboolean editable_name, gboolean editable_icon,
                                                  GCancellable *cancellable, GAsyncReadyCallback callback, gpointer data);
GVariant *xdp_portal_dynamic_launcher_prepare_install_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

void xdp_portal_open_uri (XdpPortal *portal, XdpParent *parent, const char *uri,
                          guint flags, GCancellable *cancellable,
                          GAsyncReadyCallback callback, gpointer data);
gboolean xdp_portal_open_uri_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

void xdp_portal_open_directory (XdpPortal *portal, XdpParent *parent, const char *uri,
                                guint flags, GCancellable *cancellable,
                                GAsyncReadyCallback callback, gpointer data);
gboolean xdp_portal_open_directory_finish (XdpPortal *portal, GAsyncResult *result, GError **error);

G_END_DECLS
