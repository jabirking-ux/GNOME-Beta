#pragma once
#define RLIMIT_NOFILE 7
struct rlimit {
  unsigned long rlim_cur;
  unsigned long rlim_max;
};
static inline int getrlimit (int resource, struct rlimit *rlim) {
  (void)resource;
  if (rlim) { rlim->rlim_cur = 1024; rlim->rlim_max = 4096; }
  return 0;
}
static inline int setrlimit (int resource, const struct rlimit *rlim) {
  (void)resource; (void)rlim;
  return 0;
}
