#pragma once
#include <locale.h>
#ifndef LC_MESSAGES
#define LC_MESSAGES 6
#endif
#ifndef LC_MESSAGES_MASK
#define LC_MESSAGES_MASK (1 << 5)
#endif
#ifndef LC_CTYPE_MASK
#define LC_CTYPE_MASK (1 << 0)
#endif
#ifndef LC_ALL_MASK
#define LC_ALL_MASK 0xffff
#endif
#define CODESET 14
#define _NL_ADDRESS_LANG_NAME 1
#define _NL_ADDRESS_COUNTRY_NAME 2
typedef _locale_t locale_t;
static inline locale_t newlocale (int mask, const char *locale, locale_t base) {
  (void)mask; (void)locale; (void)base;
  return (locale_t)0;
}
static inline locale_t uselocale (locale_t newloc) { (void)newloc; return (locale_t)0; }
static inline void freelocale (locale_t loc) { (void)loc; }
static inline char *nl_langinfo (int item) {
  (void)item;
  return (char *)"UTF-8";
}
