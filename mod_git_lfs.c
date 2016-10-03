/*
 * mod_git_lfs.c: handle git lfs server
 */
#include <stdio.h>
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
    int mask;
    char root[1024];
    char href[1024];
    char base[256];
    char context[256];
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
const char *git_lfs_set_href(cmd_parms *cmd, void *cfg, const char *href, const char *base);
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
        AP_INIT_TAKE2("GitLfsHref", git_lfs_set_href, NULL, ACCESS_CONF, "The request base url"),
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
const char *git_lfs_set_href(cmd_parms *cmd, void *cfg, const char *href, const char *base)
{
    git_lfs_config *conf = (git_lfs_config *)cfg;
    if (conf)
    {
        strcpy(conf->href, href);
        if (base)
        {
            strcpy(conf->base, base);
        }
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
        cfg->mask = APR_FPROT_UREAD | APR_FPROT_UWRITE | APR_FPROT_GREAD | APR_FPROT_GWRITE;
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

//response the error result
static int write_res_code(git_lfs_config *config, request_rec *r, int code, const char *fmt, ...)
{
    //set header
    ap_set_content_type(r, LFS_CONTENT_TYPE_JSON);
    //
    char msg[1024];
    memset(msg, 0, 1024);
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    va_end(args);
    r->status = code;
    json_object *json = json_object_new_object();
    json_object_object_add(json, "messages", json_object_new_string(msg));
    const char *str = json_object_to_json_string(json);
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
static int git_lfs_read_post_body(git_lfs_config *config, request_rec *r, json_object **json);
static int git_lfs_handler_bin(git_lfs_config *config, request_rec *r, const char *dir, const char *path, size_t size, int exists);
static int git_lfs_handler_batch(git_lfs_config *config, request_rec *r, json_object *json, const char *prefix, const char *accept, const char *authz);
static int git_lfs_handler_json(git_lfs_config *config, request_rec *r, const char *prefix, const char *oid, size_t size, int exists, const char *authz);

static int git_lfs_handler(request_rec *r)
{
    if (!r->handler || strcasecmp(r->handler, "GitLfs"))
    {
        return DECLINED;
    }
    apr_status_t code;
    //
    git_lfs_config *config = (git_lfs_config *)ap_get_module_config(r->per_dir_config, &git_lfs_module);
    if (config == NULL || config->enabled == 0)
    {
        return DECLINED;
    }
    if (strlen(config->root) < 1 || strlen(config->href) < 1)
    {
        r->status = 500;
        ap_log_rerror(APLOG_MARK, APLOG_ERR, APR_SUCCESS, r, "LFS->exec fail with missing config->root:%s,href:%s", config->root, config->href);
        return OK;
    }
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->exec %s by method:%s, root:%s, base:%s",
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
    char oid[256];
    memset(oid, 0, 256);
    if (strlen(r->uri) > prefix[1] + 9)
    {
        memcpy(oid, r->uri + prefix[1] + 9, strlen(r->uri) - prefix[1] - 9);
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
    //check oid
    json_object *json = NULL;
    code = git_lfs_read_post_body(config, r, &json);
    if (code != 0) //parse body fail.
    {
        return code;
    }
    if (strcmp("batch", oid) == 0) //do batch
    {
        if (json == NULL)
        {
            return write_res_code(config, r, 500, "%s", "batch body is not json");
        }
        code = git_lfs_handler_batch(config, r, json, pre, accept, authz);
        json_object_put(json);
        return code;
    }
    //check oid
    if (json) //read request body for get oid
    {
        const char *toid = json_object_get_string_v(json, "oid");
        if (toid)
        {
            memset(oid, 0, 256);
            memcpy(oid, toid, strlen(toid));
        }
        json_object_put(json);
    }
    if (strlen(oid) < 1)
    {
        return write_res_code(config, r, 500, "%s", "oid argument is not found");
    }
    //file path
    int blen = strlen(config->root);
    char path[1024];
    memset(path, 0, 1024);
    memcpy(path, config->root, blen);
    git_lfs_transform_key(oid, path + blen);
    char dir[1024];
    memset(dir, 0, 1024);
    memcpy(dir, path, blen + 6);
    //
    size_t size = 0;
    int exists = git_lfs_file_exists(r->pool, path, &size, 0);
    //
    if (strncmp(accept, LFS_CONTENT_TYPE, strlen(LFS_CONTENT_TYPE)) == 0)
    {
        return git_lfs_handler_bin(config, r, dir, path, size, exists);
    }
    else if (strncmp(accept, LFS_CONTENT_TYPE_JSON, strlen(LFS_CONTENT_TYPE_JSON)) == 0)
    {
        return git_lfs_handler_json(config, r, pre, oid, size, exists, authz);
    }
    else
    {
        return write_res_code(config, r, 404, "not supported accept->%s", accept);
    }
}

static int git_lfs_read_post_body(git_lfs_config *config, request_rec *r, json_object **json)
{
    if (strcmp(r->method, "POST") == 0) //read post body
    {
        size_t rsize = 0;
        const char *rbuf = NULL;
        int rc = read_body(r, &rbuf, &rsize);
        if (rc)
        {
            return write_res_code(config, r, 500, "read body fail with %d code", rc);
        }
        *json = json_tokener_parse(rbuf);
        if (*json == NULL)
        {
            return write_res_code(config, r, 500, "parse body to json fail->%s", rbuf);
        }
        ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->parse %s body succes by %s",
                      r->uri, rbuf);
    }
    return 0;
}

static int git_lfs_handler_bin(git_lfs_config *config, request_rec *r, const char *dir, const char *path, size_t size, int exists)
{
    //
    char buf[102400];
    size_t rsize = 102400;
    apr_status_t code;
    if (strcmp(r->method, "PUT") == 0) //do upload
    {
        // ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->PUT body to path %s", path);
        if ((code = ap_setup_client_block(r, REQUEST_CHUNKED_ERROR)))
        {
            return write_res_code(config, r, 500, "ap_setup_client_block fail with %s", code, apr_strerror(code, buf, 102400));
        }
        if (ap_should_client_block(r))
        {
            if (exists)
            {
                return write_res_code(config, r, 500, "file %s exist", path);
            }
            code = apr_dir_make_recursive(dir, config->mask | APR_FPROT_UEXECUTE | APR_FPROT_GEXECUTE, r->pool);
            if (code != APR_SUCCESS)
            {
                return write_res_code(config, r, 500, "create dir %s fail with %s", dir, apr_strerror(code, buf, 102400));
            }
            apr_file_t *file;
            code = apr_file_open(&file, path,
                                 APR_FOPEN_CREATE | APR_FOPEN_WRITE | APR_FOPEN_TRUNCATE | APR_FOPEN_BINARY,
                                 config->mask, r->pool);
            if (code != APR_SUCCESS)
            {
                return write_res_code(config, r, 500, "open file %s fail with %s", path, apr_strerror(code, buf, 102400));
            }
            char buf[102400];
            size_t rsize;
            while ((rsize = ap_get_client_block(r, buf, 102400)) > 0)
            {
                code = apr_file_write(file, buf, &rsize);
                if (code != APR_SUCCESS)
                {
                    break;
                }
            }
            apr_file_close(file);
        }
        if (code == APR_SUCCESS)
        {
            r->status = 200;
        }
        else
        {
            r->status = 500;
        }
    }
    else //do download
    {
        if (exists == 0)
        {
            return write_res_code(config, r, 404, "file %s not found", path);
        }
        apr_file_t *file;
        code = apr_file_open(&file, path, APR_READ, 0, r->pool);
        if (code != APR_SUCCESS)
        {
            return write_res_code(config, r, 500, "open file %s fail with %s", path, apr_strerror(code, buf, 102400));
        }
        r->status = 200;
        ap_set_content_length(r, size);
        while ((code = apr_file_read(file, buf, &rsize)) == APR_SUCCESS)
        {
            int wsize = ap_rwrite(buf, rsize, r);
            if (wsize < 1)
            {
                break;
            }
            rsize = 102400;
        }
        apr_file_close(file);
        if (code == APR_EOF)
        {
            code = APR_SUCCESS;
        }
    }
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->%s %s with path %s done->%s",
                  r->method, r->uri, path, apr_strerror(code, buf, 102400));
    return code;
}

static int git_lfs_handler_batch(git_lfs_config *config, request_rec *r, json_object *json, const char *prefix, const char *accept, const char *authz)
{
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->run batch with uri %s", r->uri);
    array_list *objects = json_object_get_array_v(json, "objects");
    if (objects == NULL)
    {
        return write_res_code(config, r, 500, "%s", "batch body get objects fail");
    }
    //bash
    int blen = strlen(config->root);
    char buf[1024];
    memset(buf, 0, 1024);
    memcpy(buf, config->root, blen);
    //
    json_object *robjects = json_object_new_array();
    for (int i = 0; i < objects->length; i++)
    {
        json_object *obj = (json_object *)objects->array[i];
        const char *oid = json_object_get_string_v(obj, "oid");
        if (oid == NULL)
        {
            continue;
        }
        size_t size = json_object_get_int64_v(obj, "size");
        git_lfs_transform_key(oid, buf + blen);
        int exists = git_lfs_file_exists(r->pool, buf, &size, 0);
        json_object_array_add(robjects, json_object_new_res(oid, size, config->href, prefix, authz, 1, exists ? 0 : 1, 0, 0));
    }
    json_object *res = json_object_new_object();
    json_object_object_add(res, "objects", robjects);
    ap_set_content_type(r, LFS_CONTENT_TYPE_JSON);
    const char *jres = json_object_to_json_string(res);
    ap_rprintf(r, "%s", jres);
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->do batch with uri %s success->%s", r->uri, jres);
    json_object_put(res);
    json_object_put(robjects);
    return OK;
}

static int git_lfs_handler_json(git_lfs_config *config, request_rec *r, const char *prefix, const char *oid, size_t size, int exists, const char *authz)
{
    ap_log_rerror(APLOG_MARK, config->log, APR_SUCCESS, r, "LFS->%s %s with oid %s",
                  r->method, r->uri, oid);
    ap_set_content_type(r, LFS_CONTENT_TYPE_JSON);
    if (strcmp(r->method, "GET")) //return file detail for GET
    {
        json_object *res = json_object_new_res(oid, size, config->href, prefix, authz, 1, 0, 0, 0);
        ap_rprintf(r, "%s", json_object_to_json_string(res));
        json_object_put(res);
    }
    return OK;
}
