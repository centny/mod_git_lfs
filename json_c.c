
#include "json-c/json.h"
#include "json_c.h"

size_t json_object_get_int64_v(json_object *json, const char *key)
{
    if (json == NULL)
    {
        return 0;
    }
    json_object *val = json_object_object_get(json, key);
    if (val == NULL)
    {
        return 0;
    }
    size_t res = 0;
    if (json_object_get_type(val) == json_type_int)
    {
        res = json_object_get_int64(val);
    }
    // json_object_put(val);
    return res;
}

const char *json_object_get_string_v(json_object *json, const char *key)
{
    if (json == NULL)
    {
        return NULL;
    }
    json_object *val = json_object_object_get(json, key);
    if (val == NULL)
    {
        return NULL;
    }
    const char *res = NULL;
    if (json_object_get_type(val) == json_type_string)
    {

        res = json_object_get_string(val);
    }
    // json_object_put(val);
    return res;
}

array_list *json_object_get_array_v(json_object *json, const char *key)
{
    if (json == NULL)
    {
        return NULL;
    }
    json_object *val = json_object_object_get(json, key);
    if (val == NULL)
    {
        return NULL;
    }
    array_list *res = NULL;
    if (json_object_get_type(val) == json_type_array)
    {
        res = json_object_get_array(val);
    }
    // json_object_put(val);
    return res;
}

json_object *json_object_new_res(const char *oid, size_t size, const char *href_base, const char *prefix, const char *authz, int download, int upload, int code, const char *message)
{
    char buf[512];
    memset(buf, 0, 512);
    if (prefix)
    {
        sprintf(buf, "%s%s/objects/%s", href_base, prefix, oid);
    }
    else
    {
        sprintf(buf, "%s/objects/%s", href_base, oid);
    }
    json_object *res = json_object_new_object();
    //
    json_object_object_add(res, "oid", json_object_new_string(oid));
    json_object_object_add(res, "size", json_object_new_int64(size));
    //
    //actions
    json_object *actions = json_object_new_object();
    //download
    if (download)
    {
        json_object *down = json_object_new_object();
        json_object_object_add(down, "href", json_object_new_string(buf));
        json_object_object_add(down, "expires_at", json_object_new_string("0001-01-01T00:00:00Z"));
        //header
        json_object *header = json_object_new_object();
        json_object_object_add(header, "Accept", json_object_new_string(LFS_CONTENT_TYPE));
        if (authz)
        {
            json_object_object_add(header, "Authorization", json_object_new_string(authz));
        }
        json_object_object_add(down, "header", header);
        //
        json_object_object_add(actions, "download", down);
    }
    //upload
    if (upload)
    {
        json_object *up = json_object_new_object();
        json_object_object_add(up, "href", json_object_new_string(buf));
        //header
        json_object *header = json_object_new_object();
        json_object_object_add(header, "Accept", json_object_new_string(LFS_CONTENT_TYPE));
        if (authz)
        {
            json_object_object_add(header, "Authorization", json_object_new_string(authz));
        }
        json_object_object_add(up, "header", header);
        //
        json_object_object_add(actions, "upload", up);
    }
    json_object_object_add(res, "actions", actions);
    //
    //error
    if (code != 0)
    {
        json_object *error = json_object_new_object();
        json_object_object_add(error, "code", json_object_new_int(code));
        if (message)
        {
            json_object_object_add(error, "message", json_object_new_string(message));
        }
        //
        json_object_object_add(res, "error", error);
    }
    return res;
}