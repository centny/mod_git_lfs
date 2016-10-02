
#include "file_c.h"
#include <apr_file_info.h>

void git_lfs_transform_key(const char *src, char *dst)
{
    if (strlen(src) < 5)
    {
        memcpy(dst, src, strlen(src));
        return;
    }
    dst[0] = src[0], dst[1] = src[1], dst[2] = '/';
    dst[3] = src[2], dst[4] = src[3], dst[5] = '/';
    memcpy(dst + 6, src + 4, strlen(src) - 4);
}

int git_lfs_file_exists(apr_pool_t *pool, const char *path, size_t *size, apr_status_t *code)
{
    int exists = 0;
    apr_finfo_t finfo;
    apr_status_t status = apr_stat(&finfo, path, APR_FINFO_CSIZE, pool);
    if (status == APR_SUCCESS)
    {
        exists = finfo.size > 0 ? 1 : 0;
        if (size)
        {
            *size = finfo.size;
        }
    }
    if (code)
    {
        *code = status;
    }
    return exists;
}