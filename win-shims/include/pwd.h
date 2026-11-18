#pragma once
#include <glib.h>
struct passwd {
  char *pw_name;
  char *pw_dir;
};
static inline struct passwd *getpwuid (int uid) {
  (void)uid;
  return NULL;
}
static inline int getuid (void) { return 0; }
