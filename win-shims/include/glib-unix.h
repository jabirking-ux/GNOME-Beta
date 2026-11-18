#pragma once
#include <glib.h>
static inline guint g_unix_signal_add (gint signum, GSourceFunc handler, gpointer user_data) {
  (void)signum; (void)handler; (void)user_data;
  return 0;
}
static inline gboolean g_unix_open_pipe (gint *fds, gint flags, GError **error) {
  (void)fds; (void)flags; (void)error;
  return FALSE;
}
