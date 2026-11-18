#pragma once
#include <glib.h>

typedef struct _pwquality_settings pwquality_settings_t;

pwquality_settings_t *pwquality_default_settings (void);
int pwquality_generate (pwquality_settings_t *pwq, int entropy_bits, char **password);
const char *pwquality_strerror (char *buf, size_t len, int errcode, void *auxerror);
void pwquality_free_settings (pwquality_settings_t *pwq);
