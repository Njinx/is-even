#ifndef _QUEUE_H
#define _QUEUE_H

#include <pthread.h>

struct _queue_item_t;
struct _queue;

typedef struct _queue_item_t queue_item_t;
typedef struct _queue queue_t;

int queue_create(size_t maxsize, queue_t **q);
void queue_delete(queue_t *q);
char *queue_get_item(queue_t *q);
void queue_put_item(queue_t *q, char *item);
void queue_free_item(char *item);

#endif