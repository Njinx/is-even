#define _POSIX_C_SOURCE 200809L

#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#include "queue.h"

struct _queue_item_t {
    struct _queue_item_t *next;
    char *data;
};

struct _queue {
    struct _queue_item_t *head;
    size_t size, i;
    pthread_mutex_t mtx;
    pthread_cond_t cond;
};

char *_queue_get_item(queue_t *q);
void _queue_put_item(queue_t *q, char *item);

int queue_create(size_t maxsize, queue_t **q)
{
    int status;

    *q = (queue_t *)malloc(sizeof(queue_t));
    (*q)->head = NULL;
    (*q)->size = maxsize;
    (*q)->i = 0;

    pthread_mutex_init(&(*q)->mtx, NULL);
    if ((status = pthread_cond_init(&(*q)->cond, NULL)) != 0) {
        pthread_mutex_destroy(&(*q)->mtx);
        pthread_mutex_destroy(&(*q)->mtx);
        free(*q);
        return status;
    }

    return 0;
}

void queue_delete(queue_t *q)
{
    pthread_cond_destroy(&q->cond);
    pthread_mutex_destroy(&q->mtx);

    queue_item_t *item = q->head;
    while (item) {
        queue_free_item(item->data);
        item = item->next;
    }

    free(q);
}

char *queue_get_item(queue_t *q)
{
    pthread_mutex_lock(&q->mtx);
    while (q->i == 0) {
        pthread_cond_wait(&q->cond, &q->mtx);
    }

    char *item = _queue_get_item(q);
    pthread_mutex_unlock(&q->mtx);
    pthread_cond_broadcast(&q->cond);
    return item;
}

char *_queue_get_item(queue_t *q)
{
    queue_item_t *item = q->head;
    char *data;

    q->head = item->next;
    --(q->i);
    data = item->data;
    free(item);

    return data;
}

void queue_put_item(queue_t *q, char *item)
{
    pthread_mutex_lock(&q->mtx);
    while (q->i == q->size) {
        pthread_cond_wait(&q->cond, &q->mtx);
    }

    _queue_put_item(q, item);

    pthread_mutex_unlock(&q->mtx);
    pthread_cond_broadcast(&q->cond);
}

void _queue_put_item(queue_t *q, char *item)
{
    queue_item_t *tail;

    if (q->head) {
        for (tail = q->head; tail->next; tail = tail->next)
            ;
        tail->next = (queue_item_t *)malloc(sizeof(queue_item_t));
        tail->next->next = NULL;
        tail->next->data = strdup(item);
    } else {
        q->head = (queue_item_t *)malloc(sizeof(queue_item_t));
        q->head->next = NULL;
        q->head->data = strdup(item);
    }

    ++(q->i);
}

void queue_free_item(char *item)
{
    free(item);
}
