AUTOMAKE_OPTIONS = foreign subdir-objects
bin_PROGRAMS=test_git_lfs
test_git_lfs_SOURCES=\
	../json-c/arraylist.c \
	../json-c/debug.c \
	../json-c/json_c_version.c \
	../json-c/json_object.c \
	../json-c/json_object_iterator.c \
	../json-c/json_tokener.c \
	../json-c/json_util.c \
	../json-c/linkhash.c \
	../json-c/printbuf.c \
	../json-c/random_seed.c \
    ../json_c.c \
    ../file_c.c \
    test_git_lfs.c
test_git_lfs_LDFLAGS=-L/usr/lib64 -lapr-1
test_git_lfs_CFLAGS=-I/usr/include -I/usr/local/include -I/usr/include/apr-1 -I/usr/include/httpd -std=gnu99