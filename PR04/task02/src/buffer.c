
#include "params.h"
#include "buffer.h"
#include "log.h"

#include <string.h>

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