#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uv.h"
#include "uvwasi.h"


int main(void) {
  uvwasi_options_t init_options;
  char buf[1024];
  uvwasi_t uvwasi;
  uvwasi_t* uvw;
  uvwasi_fdstat_t fdstat_buf;
  uvwasi_errno_t r;
  size_t argc;
  size_t argv_buf_size;
  int i;

  uvw = &uvwasi;
  init_options.fd_table_size = 3;
  init_options.argc = 3;
  init_options.argv = calloc(3, sizeof(char*));
  init_options.argv[0] = "--foo=bar";
  init_options.argv[1] = "-baz";
  init_options.argv[2] = "100";
  init_options.preopenc = 1;
  init_options.preopens = calloc(1, sizeof(uvwasi_preopen_t));
  init_options.preopens[0].mapped_path = "/var";
  init_options.preopens[0].real_path = ".";

  r = uvwasi_init(uvw, &init_options);
  printf("uvwasi_init() r = %d\n", r);

  r = uvwasi_args_sizes_get(uvw, &argc, &argv_buf_size);
  printf("args_sizes_get() r = %d, argc = %zu, argv_size = %zu\n",
         r,
         argc,
         argv_buf_size);

  char** args_get_argv = calloc(argc, sizeof(char*));
  r = uvwasi_args_get(uvw, args_get_argv, buf);
  printf("args_get() r = %d, %s\n", r, buf);
  for (i = 0; i < argc; ++i)
    printf("\t'%s'\n", args_get_argv[i]);

  uvwasi_fd_t dirfd = 3;
  uvwasi_lookupflags_t dirflags = 1;
  const char* path = "./foo.txt";
  uvwasi_oflags_t o_flags = UVWASI_O_CREAT;
  uvwasi_rights_t fs_rights_base = UVWASI_RIGHT_FD_DATASYNC |
                                   UVWASI_RIGHT_FD_FILESTAT_GET |
                                   UVWASI_RIGHT_FD_FILESTAT_SET_SIZE |
                                   UVWASI_RIGHT_FD_READ |
                                   UVWASI_RIGHT_FD_SYNC |
                                   UVWASI_RIGHT_PATH_READLINK |
                                   UVWASI_RIGHT_PATH_UNLINK_FILE;
  uvwasi_rights_t fs_rights_inheriting = 1;
  uvwasi_fdflags_t fs_flags = 1;
  uvwasi_fd_t fd;

  r = uvwasi_path_open(uvw,
                       dirfd,
                       dirflags,
                       path,
                       strlen(path),
                       o_flags,
                       fs_rights_base,
                       fs_rights_inheriting,
                       fs_flags,
                       &fd);
  printf("open r = %d, fd = %d\n", r, fd);

  r = uvwasi_fd_sync(uvw, fd);
  printf("fd_sync r = %d\n", r);

  r = uvwasi_fd_filestat_set_size(uvw, fd, 106);
  printf("set_size r = %d\n", r);

  r = uvwasi_fd_datasync(uvw, fd);
  printf("fd_datasync r = %d\n", r);

  uvwasi_filestat_t stats;
  r = uvwasi_fd_filestat_get(uvw, fd, &stats);
  printf("fstat r = %d\n", r);
  printf("\tstats.st_dev = %llu\n", stats.st_dev);
  printf("\tstats.st_ino = %llu\n", stats.st_ino);
  printf("\tstats.st_nlink = %u\n", stats.st_nlink);
  printf("\tstats.st_size = %llu\n", stats.st_size);
  printf("\tstats.st_filetype = %hhu\n", stats.st_filetype);
  printf("\tstats.st_atim = %llu\n", stats.st_atim);
  printf("\tstats.st_mtim = %llu\n", stats.st_mtim);
  printf("\tstats.st_ctim = %llu\n", stats.st_ctim);

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  printf("fd_fdstat_get r = %d\n", r);
  printf("\tstats.fs_filetype = %d\n", fdstat_buf.fs_filetype);
  printf("\tstats.fs_rights_base = %llu\n", fdstat_buf.fs_rights_base);
  printf("\tstats.fs_rights_inheriting = %llu\n",
         fdstat_buf.fs_rights_inheriting);
  printf("\tstats.fs_flags = %d\n", fdstat_buf.fs_flags);

  r = uvwasi_fd_fdstat_get(uvw, fd, &fdstat_buf);
  printf("fd_fdstat_get r = %d\n", r);
  printf("\tstats.fs_filetype = %d\n", fdstat_buf.fs_filetype);
  printf("\tstats.fs_rights_base = %llu\n", fdstat_buf.fs_rights_base);
  printf("\tstats.fs_rights_inheriting = %llu\n",
         fdstat_buf.fs_rights_inheriting);
  printf("\tstats.fs_flags = %d\n", fdstat_buf.fs_flags);

  uvwasi_iovec_t* iovs;
  size_t iovs_len;
  size_t nread;

  nread = 0;
  iovs_len = 2;
  iovs = calloc(iovs_len, sizeof(*iovs));
  for (i = 0; i < iovs_len; ++i) {
    iovs[i].buf_len = 10;
    iovs[i].buf = malloc(1024);
  }

  r = uvwasi_fd_read(uvw, fd, iovs, iovs_len, &nread);
  free(iovs);
  printf("fd_read r = %d, nread = %zu\n", r, nread);

  r = uvwasi_fd_fdstat_set_rights(uvw, fd, UVWASI_RIGHT_FD_FILESTAT_GET, 0);
  printf("fd_fdstat_set_rights r = %d\n", r);

  r = uvwasi_fd_close(uvw, fd);
  printf("close r = %d\n", r);

  r = uvwasi_path_unlink_file(uvw, dirfd, path, strlen(path));
  printf("unlink_file r = %d\n", r);

  r = uvwasi_fd_prestat_dir_name(uvw, dirfd, buf, sizeof(buf));
  printf("fd_prestat_dir_name r = %d, %s\n", r, buf);

  r = uvwasi_path_create_directory(uvw,
                                   dirfd,
                                   "test_dir",
                                   strlen("test_dir") + 1);
  printf("create_directory r = %d\n", r);

  r = uvwasi_path_remove_directory(uvw,
                                   dirfd,
                                   "test_dir",
                                   strlen("test_dir") + 1);
  printf("remove_directory r = %d\n", r);

  uvwasi_proc_exit(uvw, 75);

  return 0;
}