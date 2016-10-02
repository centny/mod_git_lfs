pkglib_LTLIBRARIES=mod_git_lfs.la
mod_git_lfs_la_LDFLAGS=-module -avoid-version -shared
mod_git_lfs_la_SOURCES=\
	json-c/arraylist.c \
	json-c/debug.c \
	json-c/json_c_version.c \
	json-c/json_object.c \
	json-c/json_object_iterator.c \
	json-c/json_tokener.c \
	json-c/json_util.c \
	json-c/linkhash.c \
	json-c/printbuf.c \
	json-c/random_seed.c \
	json_c.c \
	file_c.c \
    mod_git_lfs.c
mod_git_lfs_la_CFLAGS=-I/usr/include -I/usr/local/include -I/usr/include/apr-1 -I/usr/include/httpd -std=gnu99