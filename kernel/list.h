#ifndef _LIST_H_
#define _LIST_H_

/* Circular doubly linked list */

#define container_of(ptr, type, member) ((type *)((addr_t)(ptr) - offsetof(type, member)))

typedef struct list_node {
    list_node *prev;
    list_node *next;
} list_node_t;

#define LIST_INITIAL_VALUE(value) { &(value), &(value) }
#define LIST_INITIAL_CLEARED_VALUE { NULL, NULL }

#define LIST_NODE(name) \
    list_node_t name = LIST_INTIAL_VALUE(name)

static inline void list_initialize(struct list_node *list)  { list->prev = list->next = list; }
static inline void list_clear_entry(list_node_t *entry)     { entry->prev = entry->next = 0; }

#define list_entry(ptr, type, member) container_of(ptr, type, member)

static inline void list_delete(list_node_t *entry) {
    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    
    entry->prev = entry->next = 0;
}

static inline void list_add(list_node_t *list, )

static inline void list_add_tail(list_node_t *list, list_node_t *item) {
    item->next = entry;
    item->prev = entry->prev;
    
    entry->prev->next = new_entry;
    entry->prev = new_entry;
}

static inline void list_insert_after(list_node_t *entry, list_node_t *new_entry) {
    new_entry->next = entry->next;
    new_entry->prev = entry;

    entry->next->prev = new_entry;
    entry->next = new_entry;
}

static inline list_node_t *list_remove_head(list_node_t *list) {
    if (list->next != list) {
        list_node_t* item = list->next;
        list_delete(item);
        return item;
    } else {
        return NULL;
    }
}

#define list_remove_head_type(list, type, element)      \
    ({                                                  \
        list_node_t* __nod = list_remove_head(list);    \
        type* __t;                                      \
        if (__nod)                                      \
            __t = container_of(__nod, type, element);   \
        else                                            \
            __t = (type*)0;                             \
        __t;                                            \
    })

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

static inline size_t list_length(const list_node_t* list) {
    size_t cnt = 0;
    const list_node_t* node = list;
    list_for_every(list, node) { cnt++; }
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
        list_initalize(splice_from);
    }
}

static inline bool list_is_empty(list) { return list->prev == list && list->next == list; }

/* Iterators */

/* iterates over the list */
#define list_for_each(list, node) for (node = (list)->next; node != (list); node = node->next)

/* iterates of over every list, entry should be the container structure type */
#define list_for_every_entry(list, entry, type, member)                                 \
    for((entry) = container_of((list)->next, type, member); &(entry)->member != (list); \
        (entry) = container_of((entry)->member.next, type, member))

#endif /* _LIST_H_ */
