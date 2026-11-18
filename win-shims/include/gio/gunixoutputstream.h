#pragma once
#include <gio/gio.h>

typedef struct _GUnixOutputStream GUnixOutputStream;

GType g_unix_output_stream_get_type (void);
GOutputStream *g_unix_output_stream_new (gint fd, gboolean close_fd);
