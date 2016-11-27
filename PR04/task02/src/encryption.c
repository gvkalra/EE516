#include "params.h"
#include "encryption.h"
#include "log.h"

#include <string.h>

void enc_get_keys
(unsigned int *add_key, unsigned int *shift_key)
{
    FILE *fp_conf;
    unsigned int _add_key = 0, _shift_key = 0;

    //sanity check
    if (add_key == NULL || shift_key == NULL)
        return;

    // open file
    fp_conf = fopen("ee516.conf", "r");

    if (fp_conf != NULL) {
        int matched;

        // read keys
        matched = fscanf(fp_conf, "%u %u", &_add_key, &_shift_key);

        // ensure read correctly
        if (matched != 2)
            _add_key = _shift_key = 0;

        // close file
        fclose(fp_conf);
    }

    *add_key = _add_key;
    *shift_key = _shift_key;
}

void enc_decrypt_data
(unsigned char *buf, size_t size)
{
    size_t i;
    unsigned int key_add, key_shift;

    key_add = BB_DATA->key_add;
    key_shift = BB_DATA->key_shift;

    /* decryption is disabled */
    if (key_add == 0 && key_shift == 0)
        return;

    log_msg("bb_decrypt() : ADD[%u] SHIFT[%u]\n", key_add, key_shift);

#ifdef HEX_DUMP_ENABLE
    log_hex_dump(" ", size, buf);
#endif

    /* decrypt */
    for (i = 0; i < size; i++)
    {
        /* circular left shift */
        buf[i] = (buf[i] << key_shift) | (buf[i] >> (sizeof(unsigned char) * 8 - key_shift));

        /* subtract */
        buf[i] = buf[i] - key_add;
    }

#ifdef HEX_DUMP_ENABLE
    log_hex_dump(" ", size, buf);
#endif
}

int enc_encrypt_data
(const unsigned char *buf, size_t size, unsigned char **enc_buf)
{
    size_t i;
    unsigned int key_add, key_shift;

    key_add = BB_DATA->key_add;
    key_shift = BB_DATA->key_shift;

    /* allocate space for encrypted data & copy original data */
    *enc_buf = malloc(sizeof(unsigned char) * size);
    if (*enc_buf == NULL)
        return log_error("malloc() failed");
    memcpy(*enc_buf, buf, size);

    /* encryption is disabled */
    if (key_add == 0 && key_shift == 0)
        return 0;

    log_msg("bb_encrypt() : ADD[%u] SHIFT[%u]\n", key_add, key_shift);

#ifdef HEX_DUMP_ENABLE
    log_hex_dump(" ", size, *enc_buf);
#endif

    /* encrypt */
    for (i = 0; i < size; i++)
    {
        /* add */
        (*enc_buf)[i] = (*enc_buf)[i] + key_add;

        /* circular right shift */
        (*enc_buf)[i] = ((*enc_buf)[i] >> key_shift) | ((*enc_buf)[i] << (sizeof(unsigned char) * 8 - key_shift));
    }

#ifdef HEX_DUMP_ENABLE
    log_hex_dump(" ", size, *enc_buf);
#endif

    return 0;
}