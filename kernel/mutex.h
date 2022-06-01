#ifndef _MUTEX_H_
#define _MUTEX_H_

typedef struct mutex {
    long *owner;
} mutex_t;

#define MUTEX_INTIAL_VALUE(m)   \
{                               \
    .owner = NULL,              \
}

void mutex_init(mutex_t *);
void mutex_destroy(mutex_t *);

int mutex_acquire_timeout(mutex_t *, __time_t);
int mutex_release(mutex_t *);

static inline mutex_acquire(mutex_t *m) {
    return mutex_acquire_timeout(m, INT64_MAX);
}

#endif /* _MUTEX_H_ */