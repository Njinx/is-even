#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

typedef int (*is_even_func_t)(unsigned int);

int main(int argc, char *argv[])
{
    char fpath[64];
    struct stat info;
    int fd;
    is_even_func_t is_even;
    int opt_number;
    long opt_number_tmp;
    int result;
    char const *assets_base;

    if (argc != 2) {
        fprintf(stderr, "Bad argument count.\n");
        exit(EXIT_FAILURE);
    }

    opt_number_tmp = strtol(argv[1], NULL, 10);
    if (opt_number_tmp < 0 || opt_number_tmp > UINT_MAX) {
        fprintf(stderr, "Bad number.\n");
        exit(EXIT_FAILURE);
    }
    opt_number = (int)opt_number_tmp;

    assets_base = getenv("ISEVEN_ASSETS");
    if (assets_base == NULL) {
        assets_base = "outdir";
    }

    for (int i = 0; i <= UINT_MAX / 100000 + 1; ++i) {
        snprintf(fpath, 64, "%s/%02x/%02x/%02x/%06x", assets_base, (i>>24) & 0xFF, (i>>16) & 0xFF, (i>>8) & 0xFF, i);
        fd = open(fpath, O_RDONLY);
        if (fd < 0) {
            fprintf(stderr, "open(\"%s\"): %s\n", fpath, strerror(errno));
            exit(EXIT_FAILURE);
        }

        if (fstat(fd, &info) < 0) {
            perror("stat()");
            close(fd);
            exit(EXIT_FAILURE);
        }

        if (info.st_size == 0) {
            fprintf(stderr, "Missing asset %s\n", fpath);
            exit(EXIT_FAILURE);
        }

        is_even = mmap(NULL, info.st_size, PROT_READ|PROT_EXEC, MAP_PRIVATE, fd, 0);
        close(fd);
        if (is_even == MAP_FAILED) {
            perror("mmap()");
            exit(EXIT_FAILURE);
        }

        result = is_even(opt_number);
        if (result != 2) {
            printf("%u is %s\n", opt_number, result ? "even" : "odd");
            exit(EXIT_SUCCESS);
        }

        if (munmap(is_even, info.st_size) < 0) {
            perror("munmap()");
            exit(EXIT_FAILURE);
        }
    }

    return 1;
}