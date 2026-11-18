#pragma once
#include <gio/gio.h>

typedef struct _GUnixFDList GUnixFDList;

GType g_unix_fd_list_get_type (void);
GUnixFDList *g_unix_fd_list_new (void);
gint g_unix_fd_list_get_length (GUnixFDList *list);
gint g_unix_fd_list_get (GUnixFDList *list, gint index_, GError **error);
gint g_unix_fd_list_append (GUnixFDList *list, gint fd, GError **error);
