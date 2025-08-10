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
#include <pthread.h>

#include "queue.h"

typedef int (*is_even_func_t)(unsigned int);

static queue_t *queue;

void *producer_thread(void *arg);
void *consumer_thread(void *arg);
void start(char const *assets_base, int opt_number);

void *producer_thread(void *arg)
{
    char fpath[64];
    char *assets_base = (char *)arg;
    int i;

    for (i = 0; i <= UINT_MAX / 100000; ++i) {
        snprintf(fpath, 64, "%s/%02x/%02x/%02x/%06x", assets_base, (i>>24) & 0xFF, (i>>16) & 0xFF, (i>>8) & 0xFF, i);
        queue_put_item(queue, fpath);
    }

    return NULL;
}

void *consumer_thread(void *arg)
{
    struct stat info;
    int fd, result;
    is_even_func_t is_even;
    char *fpath;
    int opt_number = *(int *)arg;

    for (;;) {
        fpath = queue_get_item(queue);

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

        queue_free_item(fpath);
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

    return NULL;
}

void start(char const *assets_base, int opt_number)
{
    pthread_t tids[64];
    int tid_idx = 0;
    int i;
    int nproc = sysconf(_SC_NPROCESSORS_ONLN) - 1;
    if (nproc < 1) {
        nproc = 1;
    }

    pthread_create(&tids[tid_idx++], NULL, producer_thread, (void *)assets_base);
    for (i = 0; i < nproc; ++i, ++tid_idx) {
        pthread_create(&tids[tid_idx], NULL, consumer_thread, (void *)&opt_number);
    }

    for (i = 0; i < tid_idx; ++i) {
        pthread_join(tids[i], NULL);
    }
}

int main(int argc, char *argv[])
{
    int opt_number;
    int status;
    long opt_number_tmp;
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

    if ((status = queue_create(64, &queue)) != 0) {
        fprintf(stderr, "Failed to create queue: %s\n", strerror(status));
        exit(EXIT_FAILURE);
    }

    start(assets_base, opt_number);

    return 0;
}