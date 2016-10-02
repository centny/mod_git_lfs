#ifndef _file_c_h_
#define _file_c_h_

#include "apr_file_info.h"

extern void git_lfs_transform_key(const char *src, char *dst);
extern int git_lfs_file_exists(apr_pool_t *pool, const char *path, size_t *size, apr_status_t *code);

#endif