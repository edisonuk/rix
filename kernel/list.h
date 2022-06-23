#ifndef _LIST_H_
#define _LIST_H_

#include <stdbool.h>
#include <stddef.h>

#include "types.h"

/* Circular doubly linked list */

#define container_of(ptr, type, member) ((type *)((addr_t)(ptr) - offsetof(type, member)))

struct list_node {
    struct list_node *prev;
    struct list_node *next;
};

typedef struct list_node list_t;
typedef struct list_node list_node_t;
typedef struct list_node list_head_t;
typedef struct list_node list_entry_t;

#define LIST_INITIAL_VALUE(value) { &(value), &(value) }
#define LIST_INITIAL_CLEARED_VALUE { NULL, NULL }

#define LIST_NODE(name) list_node_t name = LIST_INITIAL_VALUE(name)

static inline void list_initialize(struct list_node *list)  { list->prev = list->next = list; }
static inline void list_clear_entry(list_node_t *entry)     { entry->prev = entry->next = 0; }

/* Whether or not the head belongs to a list. */
static inline bool list_is_in_list(list_node_t *item) {
    if (item->next == 0 && item->prev == 0) {
        return false;
    }
    return true;
}

#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void list_delete(list_node_t *entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    
    entry->prev = entry->next = 0;
}

/** Insert a new entry after the specified head. */
static inline void list_add(list_node_t *head, list_node_t *item) {
    item->next = head->next;
    item->prev = head;

    head->next->prev = item;
    head->next = item;
}

/** Insert a new entry before the specified head. */
static inline void list_add_tail(list_node_t *head, list_node_t *item) {
    item->prev = head->prev;
    item->next = head;
    
    head->prev->next = item;
    head->prev = item;
}

/** Replace old entry with new one. */
static inline void list_replace(list_node_t *old, list_node_t *new) {
    new->next = old->next;
    new->prev = old->prev;

    old->prev->next = new;
    old->next->prev = new;
}

/**
 * Whether or not this list is empty.
 * Empty list means the next and previous points to the head itself.
 */
static inline int list_is_empty(list_node_t *list) {
    return list->prev == list && list->next == list;
}

/** Removes the head from the list and returns it. */
static inline list_node_t *list_remove_head(list_node_t *head) {
    if (head->next != head) {
        list_node_t* item = head->next;
        list_delete(item);
        return item;
    } else {
        return NULL;
    }
}

#define list_remove_head_type(head, type, element)      \
    ({                                                  \
        list_node_t* __node = list_remove_head(head);   \
        type* __t;                                      \
        if (__node)                                     \
            __t = container_of(__node, type, element);  \
        else                                            \
            __t = (type*)0;                             \
        __t;                                            \
    })

/** Removes the tail from the list and returns it. */
static inline list_node_t *list_remove_tail(list_node_t *list) {
    if (list->prev != list) {
        list_node_t* item = list->prev;
        list_delete(item);
        return item;
    } else {
        return NULL;
    }
}

#define list_remove_tail_type(list, type, element)      \
    ({                                                  \
        list_node_t* __nod = list_remove_tail(list);    \
        type* __t;                                      \
        if (__nod)                                      \
            __t = container_of(__nod, type, element);   \
        else                                            \
            __t = (type*)0;                             \
        __t;                                            \
    })
 
static inline list_node_t* list_peek_head(list_node_t* list) {
    if (list->next != list) {
        return list->next;
    } else {
        return NULL;
    }
}

#define list_peek_tail_head(list, type, element)        \
    ({                                                  \
        list_node_t *__nod = list_peek_head(list);      \
        type* __t;                                      \
        if(__nod)                                       \
            __t = container_of(__nod, type, element);   \
        else                                            \
            __t = (type *)0;                            \
        __t;                                            \
    })

static inline list_node_t* list_peek_tail(list_node_t* list) {
    if (list->prev != list) {
        return list->prev;
    } else {
        return NULL;
    }
}

#define list_peek_tail_type(list, type, element)        \
    ({                                                  \
        list_node_t *__nod = list_peek_tail(list);      \
        type* __t;                                      \
        if(__nod)                                       \
            __t = container_of(__nod, type, element);   \
        else                                            \
            __t = (type *)0;                            \
        __t;                                            \
    })

/* ----------------------- Iterators ----------------------- */

/** Iterate over the list */
#define list_for_each(pos, head)                                        \
    for (pos = head->next; pos != (head); pos = pos->next)

/** Iterate over the list backwards */
#define list_for_each_prev(pos, head)                                   \
    for (pos = head->prev; pos != (head); pos = pos->pev)

/* Iterates of over every list, entry should be the container structure type */
#define list_for_each_entry(pos, head, member)                          \
    for(pos = container_of((head)->next, typeof(*pos), member);         \
        &pos->member != (head);                                         \
        pos = container_of((pos)->member.next, typeof(*pos), member))

static inline size_t list_length(const list_node_t* list) {
    size_t cnt = 0;
    const list_node_t* node = list;
    list_for_each(list, node) { cnt++; }
    return cnt;
}

/* Split the contents of list after (but not including) pos, into split_to */
static inline void list_split_after(list_node_t* list, list_node_t* pos, list_node_t* split_to) {
    if (pos->next == list) {
        list_initialize(split_to);
        return;
    }

    split_to->prev = list->prev;
    split_to->prev->next = split_to;
    split_to->next = pos->next;
    split_to->next->prev = split_to;
    
    pos->next = list;
    list->prev = pos;
}

static inline void list_splice_after(list_node_t* splice_from, list_node_t* pos) {
    if (list_is_empty(splice_from)) {
        list_initialize(splice_from);
    }
}

#endif /* _LIST_H_ */
