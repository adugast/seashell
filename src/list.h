#ifndef __LIST_H__
#define __LIST_H__


#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define UNIQUE CONCAT(var_, __LINE__)


#define for_each(head, nodep) \
    __typeof__(nodep) UNIQUE; \
    for (nodep = (head)->next; UNIQUE = nodep ? nodep->next : NULL, nodep && (nodep != (head)); nodep = UNIQUE)


#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


struct list {
    struct list *next, *prev;
};


static inline size_t list_length(struct list *head)
{
    size_t len = 0;
    struct list *nodep = NULL;

    for_each(head, nodep)
        len++;

    return len;
}


static inline void init_list(struct list *head)
{
    head->prev = head;
    head->next = head;
}


static inline void __list_insert(struct list *entry, struct list *before, struct list *after)
{
    before->next = entry;
    after->prev = entry;
    entry->next = after;
    entry->prev = before;
}


static inline void list_add_head(struct list *head, struct list *entry)
{
    __list_insert(entry, head, head->next);
}


static inline void list_add_tail(struct list *head, struct list *entry)
{
    __list_insert(entry, head->prev, head);
}


static inline void list_delete(struct list *entry)
{
    if (entry->next && entry->prev) {
        entry->next->prev = entry->prev;
        entry->prev->next = entry->next;
    }
    entry->next = NULL;
    entry->prev = NULL;
}


#endif /* __LIST_H__ */
