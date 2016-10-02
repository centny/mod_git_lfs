
#ifndef _json_c_h_
#define _json_c_h_

#include "json-c/json.h"

#define LFS_CONTENT_TYPE "application/vnd.git-lfs"
#define LFS_CONTENT_TYPE_JSON "application/vnd.git-lfs+json"

extern size_t json_object_get_int64_v(json_object *json, const char *key);
extern const char *json_object_get_string_v(json_object *json, const char *key);
extern array_list *json_object_get_array_v(json_object *json, const char *key);
extern json_object *json_object_new_res(const char *oid, size_t size, const char *href_base, const char *prefix, const char *authz, int exists, int code, const char *message);

#endif