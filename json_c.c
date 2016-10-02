
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
    if (json_object_get_type(val) == json_type_int)
    {
        return json_object_get_int64(val);
    }
    else
    {
        return 0;
    }
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
    if (json_object_get_type(val) == json_type_string)
    {
        return json_object_get_string(val);
    }
    else
    {
        return NULL;
    }
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
    if (json_object_get_type(val) == json_type_array)
    {
        return json_object_get_array(val);
    }
    else
    {
        return NULL;
    }
}

json_object *json_object_new_res(const char *oid, size_t size, const char *href_base, const char *prefix, const char *authz, int exists, int code, const char *message)
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
    if (exists)
    {
        json_object *download = json_object_new_object();
        json_object_object_add(download, "href", json_object_new_string(buf));
        //header
        json_object *header = json_object_new_object();
        json_object_object_add(header, "Accept", json_object_new_string(LFS_CONTENT_TYPE));
        if (authz)
        {
            json_object_object_add(header, "Authorization", json_object_new_string(authz));
        }
        json_object_object_add(download, "header", header);
        //
        json_object_object_add(actions, "download", download);
    }
    //upload
    {
        json_object *upload = json_object_new_object();
        json_object_object_add(upload, "href", json_object_new_string(buf));
        //header
        json_object *header = json_object_new_object();
        json_object_object_add(header, "Accept", json_object_new_string(LFS_CONTENT_TYPE));
        if (authz)
        {
            json_object_object_add(header, "Authorization", json_object_new_string(authz));
        }
        json_object_object_add(upload, "header", header);
        //
        json_object_object_add(actions, "upload", upload);
    }
    json_object_object_add(res, "actions", actions);
    //
    //error
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