#pragma once

#include <stdlib.h>

void enc_decrypt_data
(unsigned char *buf, size_t size);

int enc_encrypt_data
(const unsigned char *buf, size_t size, unsigned char **enc_buf);

void enc_get_keys
(unsigned int *add_key, unsigned int *shift_key);