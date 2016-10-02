/*
 * mod_git_lfs.c: handle git lfs server
 */
#include <stdio.h>
#include <strings.h>
#include "apr_hash.h"
#include "ap_config.h"
#include "ap_provider.h"
#include "httpd.h"
#include "http_core.h"
#include "http_config.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_request.h"
#include "json-c/json.h"
#include "json_c.h"
#include "file_c.h"

/*
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Configuration structure
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */
typedef struct
{
    int enabled;
    int log;
    char root[1024];
    char base[256];
    char context[256];
    char href_base[1024];
} git_lfs_config;

/*
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Prototypes
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

static int git_lfs_handler(request_rec *r);
const char *git_lfs_set_enabled(cmd_parms *cmd, void *cfg, const char *arg);
const char *git_lfs_set_log(cmd_parms *cmd, void *cfg, const char *arg);
const char *git_lfs_set_root(cmd_parms *cmd, void *cfg, const char *arg);
const char *git_lfs_set_base(cmd_parms *cmd, void *cfg, const char *arg);
void *create_dir_conf(apr_pool_t *pool, char *context);
static void register_hooks(apr_pool_t *pool);

/*
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Configuration directives
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

static const command_rec directives[] =
    {
        AP_INIT_TAKE1("GitLfsEnabled", git_lfs_set_enabled, NULL, ACCESS_CONF, "Enable or disable mod_git_lfs"),
        AP_INIT_TAKE1("GitLfsRoot", git_lfs_set_root, NULL, ACCESS_CONF, "The root path to whatever"),
        AP_INIT_TAKE1("GitLfsLog", git_lfs_set_log, NULL, ACCESS_CONF, "The log level"),
        AP_INIT_TAKE1("GitLfsBase", git_lfs_set_base, NULL, ACCESS_CONF, "The request base url"),
        {NULL}};

/*$1
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Our name tag
 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 */

module AP_MODULE_DECLARE_DATA git_lfs_module =
    {
        STANDARD20_MODULE_STUFF,
        create_dir_conf, /* Per-directory configuration handler */
        NULL,            /* Merge handler for per-directory configurations */
        NULL,            /* Per-server configuration handler */
        NULL,            /* Merge handler for per-server configurations */
        directives,      /* Any directives we may have for httpd */
        register_hooks   /* Our hook registering function */
};

/*
 =======================================================================================================================
    Hook registration function
 =======================================================================================================================
 */
static void register_hooks(apr_pool_t *pool)
{
    ap_hook_handler(git_lfs_handler, NULL, NULL, APR_HOOK_LAST);
}

/*
 =======================================================================================================================
    Handler for the "GitLfsEnabled" directive
 =======================================================================================================================
 */
const char *git_lfs_set_enabled(cmd_parms *cmd, void *cfg, const char *arg)
{
    git_lfs_config *conf = (git_lfs_config *)cfg;
    if (conf)
    {
        if (strcasecmp(arg, "on"))
            conf->enabled = 0;
        else
            conf->enabled = 1;
    }
    return NULL;
}

/*
 =======================================================================================================================
    Handler for the "GitLfsLog" directive
 =======================================================================================================================
 */
const char *git_lfs_set_log(cmd_parms *cmd, void *cfg, const char *arg)
{
    git_lfs_config *conf = (git_lfs_config *)cfg;
    if (conf)
    {
        if (arg[0] == '0')
            conf->log = 0;
        else if (arg[0] == '1')
            conf->log = 1;
        else if (arg[0] == '2')
            conf->log = 2;
        else if (arg[0] == '3')
            conf->log = 3;
    }
    return NULL;
}

/*
 =======================================================================================================================
    Handler for the "GitLfsRoot" directive
 =======================================================================================================================
 */
const char *git_lfs_set_root(cmd_parms *cmd, void *cfg, const char *arg)
{
    git_lfs_config *conf = (git_lfs_config *)cfg;
    if (conf)
    {
        strcpy(conf->root, arg);
    }
    return NULL;
}

/*
 =======================================================================================================================
    Handler for the "GitLfsBase" directive
 =======================================================================================================================
 */
const char *git_lfs_set_base(cmd_parms *cmd, void *cfg, const char *arg)
{
    git_lfs_config *conf = (git_lfs_config *)cfg;
    if (conf)
    {
        strcpy(conf->base, arg);
    }
    return NULL;
}

/*
 =======================================================================================================================
    Function for creating new configurations for per-directory contexts
 =======================================================================================================================
 */
void *create_dir_conf(apr_pool_t *pool, char *context)
{
    context = context ? context : "Newly created configuration";
    git_lfs_config *cfg = apr_pcalloc(pool, sizeof(git_lfs_config));
    if (cfg)
    {
        cfg->enabled = 0;
        cfg->log = 0;
        strcpy(cfg->context, context);
        memset(cfg->root, 0, 1024);
        memset(cfg->base, 0, 256);
    }
    return cfg;
}

/*
 =======================================================================================================================
    Function for request util
 =======================================================================================================================
 */
static int read_body(request_rec *r, const char **rbuf, apr_off_t *size)
{
    int rc = OK;
    if ((rc = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)))
    {
        return (rc);
    }
    if (ap_should_client_block(r))
    {
        char lbuf[HUGE_STRING_LEN];
        apr_off_t rsize, len_read, rpos = 0;
        apr_off_t length = r->remaining;
        *rbuf = (const char *)apr_pcalloc(r->pool, (apr_size_t)(length + 1));
        *size = length;
        while ((len_read = ap_get_client_block(r, lbuf, sizeof(lbuf))) > 0)
        {
            if ((rpos + len_read) > length)
            {
                rsize = length - rpos;
            }
            else
            {
                rsize = len_read;
            }
            memcpy((char *)*rbuf + rpos, lbuf, (size_t)rsize);
            rpos += rsize;
        }
    }
    return (rc);
}

// #define write_res_code(...) write_res_code_(__VA_ARGS__)

static int write_res_code(git_lfs_config *config, request_rec *r, int code, const char *fmt, ...)
{
    char msg[1024];
    memset(msg, 0, 1024);
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    r->status = code;
    json_object *json = json_object_new_object();
    json_object_object_add(json,
                           "messages", json_object_new_string(msg));
    const char *str = json_object_to_json_string(json);
    ap_set_content_type(r, LFS_CONTENT_TYPE_JSON);
    ap_rprintf(r, "%s", str);
    json_object_put(json);
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->reponse %s call %s with code:%u,message:%s",
                  r->method, r->uri, code, msg);
    return OK;
}
/*
 =======================================================================================================================
    Our git lfs web service handler
 =======================================================================================================================
 */
static int git_lfs_handler_bin(git_lfs_config *config, request_rec *r, const char *pre, const char *oid, const char *accept, const char *authz);
static int git_lfs_handler_json(git_lfs_config *config, request_rec *r, const char *pre, const char *oid, const char *accept, const char *authz);

static int git_lfs_handler(request_rec *r)
{
    if (!r->handler || strcasecmp(r->handler, "GitLfs"))
        return (DECLINED);

    //ap_log_cdata(r->handler);
    git_lfs_config *config = (git_lfs_config *)ap_get_module_config(r->per_dir_config, &git_lfs_module);
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->request %s by method:%s, root:%s, base:%s",
                  r->uri, r->method, config->root, config->base);
    //
    //find prefix and oid
    int prefix[2];
    prefix[0] = strlen(config->base);
    if (prefix[0] > 0 && strncmp(config->base, r->uri, prefix[0]) != 0)
    {
        return write_res_code(config, r, 404, "not base uri->%s", r->uri);
    }
    const char *objects = ap_strstr(r->uri, "/objects");
    if (objects == NULL)
    {
        return write_res_code(config, r, 404, "not objects uri->%s", r->uri);
    }
    prefix[1] = objects - r->uri;
    char pre[256];
    memset(pre, 0, 256);
    if (prefix[1] - prefix[0] > 0)
    {
        memcpy(pre, r->uri + prefix[0], prefix[1] - prefix[0]);
    }
    const char *oid = NULL;
    if (strlen(r->uri) > prefix[1] + 9)
    {
        oid = r->uri + prefix[1] + 9;
    }
    //
    //check accept
    const char *accept = apr_table_get(r->headers_in, "Accept");
    if (strlen(accept) == 0)
    {
        return write_res_code(config, r, 404, "%s", "not accept header");
    }
    //
    //check authz
    const char *authz = apr_table_get(r->headers_in, "Authorization");
    //
    if (strncmp(accept, LFS_CONTENT_TYPE, strlen(LFS_CONTENT_TYPE)) == 0)
    {
        return git_lfs_handler_bin(config, r, pre, oid, accept, authz);
    }
    else if (strncmp(accept, LFS_CONTENT_TYPE_JSON, strlen(LFS_CONTENT_TYPE_JSON)) == 0)
    {
        return git_lfs_handler_json(config, r, pre, oid, accept, authz);
    }
    else
    {
        return write_res_code(config, r, 404, "not supported accept->%s", accept);
    }
}

static int git_lfs_handler_bin(git_lfs_config *config, request_rec *r, const char *pre, const char *oid, const char *accept, const char *authz)
{
    return OK;
}
static int git_lfs_handler_json(git_lfs_config *config, request_rec *r, const char *prefix, const char *oid, const char *accept, const char *authz)
{
    //set header
    ap_set_content_type(r, LFS_CONTENT_TYPE_JSON);
    //base path
    int blen = strlen(config->root);
    char buf[1024];
    memcpy(buf, config->root, blen);
    //
    apr_status_t code;
    int rsize = 0;
    char *rbuf = NULL;
    json_object *json = NULL;
    if (strcmp(r->method, "POST") == 0)
    {
        int rc = read_body(r, &rbuf, &rsize);
        if (rc)
        {
            return write_res_code(r, 500, "read body fail with %d code", rc);
        }
        json = json_tokener_parse(rbuf);
        if (json == NULL)
        {
            return write_res_code(r, 500, "parse body to json fail->%s", rbuf);
        }
    }
    if (strcmp(oid, "batch") == 0)
    {
        if (json == NULL)
        {
            return write_res_code(r, 500, "batch body is not json->%s", rbuf);
        }
        array_list *objects = json_object_get_array_v(objects, "objects");
        if (objects == NULL)
        {
            json_object_put(json);
            return write_res_code(r, 500, "batch body get objects fail->%s", rbuf);
        }
        json_object *robjects = json_object_new_array();
        for (int i = 0; i < objects->length; i++)
        {
            json_object *obj = (json_object *)objects->array[i];
            const char *oid = json_object_get_string_v(obj, "oid");
            if (oid == NULL)
                continue;
            size_t size = json_object_get_int64_v(obj, "size");
            git_lfs_transform_key(oid, buf + blen);
            buf[blen + strlen(oid)] = 0;
            int exists = git_lfs_file_exists(r->pool, buf, &size, &code);
            json_object_array_add(robjects, json_object_new_res(oid, size, config->href_base, prefix, authz, exists, (int)code, 0));
        }
        ap_rprintf(r, "%s", json_object_to_json_string(robjects));
        json_object_put(robjects);
        json_object_put(json);
        return OK;
    }
    char toid[64];
    memset(toid, 0, 64);
    if (json)
    {
        const char *oid = json_object_get_string_v(json, "oid");
        memcpy(toid, oid, strlen(oid));
        json_object_put(json);
    }
    if (strlen(toid) < 1)
    {
        return write_res_code(r, 404, "oid argument is not found->%s", rbuf);
    }
    git_lfs_transform_key(oid, buf + blen);
    buf[blen + strlen(oid)] = 0;
    size_t size = 0;
    int exists = git_lfs_file_exists(r->pool, buf, &size, &code);
    if (exists == 0)
    {
        return write_res_code(r, 404, "oid is not found->%s", oid);
    }
    if (strcmp(r->method, "GET"))
    {
        json_object *res = json_object_new_res(oid, size, config->href_base, prefix, authz, exists, (int)code, 0);
        ap_rprintf(r, "%s", json_object_to_json_string(res));
        json_object_put(res);
    }
    return OK;
}
