#ifndef __LIST_H__
#define __LIST_H__


#define for_each(head, pos) \
    for (pos = (head)->next; pos != (head); pos = pos->next)


#define container_of(ptr, type, member) \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))


struct list {
    struct list *prev, *next;
};


static inline size_t list_length(struct list *head)
{
    size_t ret = 0;
    struct list *nodep = NULL;

    for_each(tmp, nodep)
        ret++;

    return ret;
}


static inline void init_list(struct list *head)
{
    head->prev = head;
    head->next = head;
}


static inline void __list_insert(struct list *head, struct list *before, struct list *after)
{
    before->next = head;
    after->prev = head;
    head->next = after;
    head->prev = before;
}


static inline void list_add_head(struct list *head, struct list *entry)
{
    __list_insert(entry, head, head->next);
}


static inline void list_add_tail(struct list *head, struct list *entry)
{
    __list_insert(entry, head->prev, head);
}


#endif /* __LIST_H__ */
