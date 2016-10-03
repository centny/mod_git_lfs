#include <stdio.h>
#include "../json_c.h"
#include "../file_c.h"
void test_file_c();
void test_json_c();
int main()
{
    apr_initialize();
    // exit(1);
    do
    {
        for (int i = 0; i < 10000; i++)
        {
            test_file_c();
            test_json_c();
        }
        sleep(1);
    } while (1);
    printf("test done...\n");
    return 0;
}
void test_json_c()
{
    json_object *json = json_object_from_file("test_data.json");
    if (json == NULL)
    {
        printf("read json fail->\n");
        exit(1);
    }
    //
    //test read string
    {
        const char *sval = NULL;
        if ((sval = json_object_get_string_v(json, "n1")) == NULL)
        {
            printf("read json string fail->\n");
            exit(1);
        }
        if ((sval = json_object_get_string_v(json, "xxxx")) != NULL)
        {
            printf("read json string fail->\n");
            exit(1);
        }
        if ((sval = json_object_get_string_v(json, "n2")) != NULL)
        {
            printf("read json string fail->\n");
            exit(1);
        }
        printf("test read string done...\n");
    }
    //
    //test read array
    {
        array_list *aval = NULL;
        if ((aval = json_object_get_array_v(json, "v1")) == NULL)
        {
            printf("read json array fail->\n");
            exit(1);
        }
        // json_object_put((json_object *)aval);
        if ((aval = json_object_get_array_v(json, "xxx")) != NULL)
        {
            printf("read json array fail->\n");
            exit(1);
        }
        if ((aval = json_object_get_array_v(json, "n2")) != NULL)
        {
            printf("read json array fail->\n");
            exit(1);
        }
        printf("test read array done...\n");
    }
    //
    json_object_put(json);

    //
    //test new res
    {
        //const char *oid, size_t size, const char *href_base, const char *prefix, const char *authz, int download, int upload, int code, const char *message)
        json_object *res = json_object_new_res("abc", 1, "http://xx", "a", "", 1, 1, 0, 0);
        json_object_to_file("a.json", res);
        // printf("->%d\n", strlen(xx));
        json_object_put(res);
        printf("test new res done...\n");
    }
}
void test_file_c()
{
    apr_pool_t *p;
    apr_pool_create(&p, NULL);
    apr_status_t code;
    size_t size;
    {
        if (!git_lfs_file_exists(p, "test_data.json", &size, &code))
        {
            printf("test file exist fail->\n");
            exit(1);
        }
        if (git_lfs_file_exists(p, "test_data_x.json", &size, &code))
        {
            printf("test file exist fail->\n");
            exit(1);
        }
    }
    {
        char res[128];
        //
        const char *buf1 = "abcd";
        memset(res, 0, 128);
        git_lfs_transform_key(buf1, res);
        printf("<- %s ->\n", res);
        //
        const char *buf2 = "abcde";
        memset(res, 0, 128);
        git_lfs_transform_key(buf2, res);
        printf("<- %s ->\n", res);
        //
        const char *buf3 = "abcdefghkk";
        memset(res, 0, 128);
        git_lfs_transform_key(buf3, res);
        printf("<- %s ->\n", res);
    }
    {
        if (apr_dir_make_recursive("a/b/c", 0655, p) != APR_SUCCESS)
        {
            printf("create folder fail->\n");
            exit(1);
        }
    }
    apr_pool_destroy(p);
    printf("test file done...\n");
}