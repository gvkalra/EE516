#include "params.h"
#include "buffer.h"
#include "log.h"

#include <string.h>

// each chunck is 4KB in size
struct chunk_elem {
    unsigned char data[4 * 1024]; //4KB
};

struct list_elem {
    struct list_elem *next;

    off_t start, end; // start & end offset of data
    char *path; // file path
    unsigned int index; // index where data is cached
};

// cache consists of 1280 chunks (1280 * 4 = 5120 KB = 5MB)
static struct chunk_elem cache[1280]; //5MB
static struct list_elem *head = NULL;

static void _insert_chunk_elem
(unsigned char *data, size_t data_len, unsigned int cache_index)
{
    // sanity check
    if (data_len < 0
        || data_len > (4 * 1024)
        || data == NULL
        || cache_index >= 1280) {
        log_msg("ERROR : _insert_chunk_elem() invalid arguments\n");
        return;
    }

    log_msg("CACHE UPDATED : index [%u]\n", cache_index);
    log_hex_dump(" ", data_len, data);

    // copy data to cache at cache_index
    memcpy(cache[cache_index].data, data, data_len);
}

static void _read_from_cache
(const char *path, char **buf, size_t size, off_t offset)
{
    struct list_elem *iter = NULL;

    // sanity check (size = 4KB as per task requirement)
    if (path == NULL
        || buf == NULL
        || size != (4 * 1024)) {
        log_msg("ERROR : _read_from_cache() invalid arguments");
        return;
    }

    // invalid cache
    if (head == NULL) {
        log_msg("ERROR : _read_from_cache() cached invalid");
        return;
    } else {
        iter = head; // set iter as head
    }

    do {
        // requested offset is between start and end of cache
        if (offset >= iter->start && offset <= iter->end
            && (offset + size) <= iter->end) {
        }
    } while (iter != NULL)
}

void buf_get_policy
(unsigned int *buf_policy)
{
    FILE *fp_conf;
    unsigned int _buf_policy;

    //sanity check
    if (buf_policy == NULL)
        return;

    // open file
    fp_conf = fopen("ee516.conf", "r");

    if (fp_conf != NULL) {
        int matched;

        // read keys
        matched = fscanf(fp_conf, "%*[^\n]\n%u", &_buf_policy);

        // ensure read correctly
        if (matched != 1)
            _buf_policy = 0;

        // close file
        fclose(fp_conf);
    }

    *buf_policy = _buf_policy;
}