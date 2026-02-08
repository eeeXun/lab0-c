#include <string.h>
#include "list.h"

#include "queue.h"

/* Create an empty queue */
struct list_head *q_new()
{
    struct list_head *head = malloc(sizeof(struct list_head));
    if (!head)
        return NULL;
    INIT_LIST_HEAD(head);
    return head;
}

/* Free all storage used by queue */
void q_free(struct list_head *head)
{
    if (!head)
        return;
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list)
        q_release_element(entry);
    free(head);
}

/* Insert an element at head of queue */
bool q_insert_head(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add(&node->list, head);
    return true;
}

/* Insert an element at tail of queue */
bool q_insert_tail(struct list_head *head, char *s)
{
    if (!head)
        return false;
    element_t *node = malloc(sizeof(element_t));
    if (!node)
        return false;
    node->value = strdup(s);
    if (!node->value) {
        free(node);
        return false;
    }
    list_add_tail(&node->list, head);
    return true;
}

/* Remove an element from head of queue */
element_t *q_remove_head(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_first_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&node->list);
    return node;
}

/* Remove an element from tail of queue */
element_t *q_remove_tail(struct list_head *head, char *sp, size_t bufsize)
{
    if (!head || list_empty(head))
        return NULL;
    element_t *node = list_last_entry(head, element_t, list);
    if (sp) {
        strncpy(sp, node->value, bufsize - 1);
        sp[bufsize - 1] = '\0';
    }
    list_del(&node->list);
    return node;
}

/* Return number of elements in queue */
int q_size(struct list_head *head)
{
    if (!head)
        return 0;
    int len = 0;
    element_t *entry;
    list_for_each_entry(entry, head, list)
        len++;
    return len;
}

struct list_head *q_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return NULL;
    struct list_head *slow = head->next, *fast = head->next;
    while (fast != head && fast->next != head) {
        slow = slow->next;
        fast = fast->next->next;
    }
    return slow;
}

/* Delete the middle node in queue */
bool q_delete_mid(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    struct list_head *mid = q_mid(head);
    list_del(mid);
    q_release_element(list_entry(mid, element_t, list));
    return true;
}

/* Delete all nodes that have duplicate string */
bool q_delete_dup(struct list_head *head)
{
    if (!head || list_empty(head))
        return false;
    bool duplicate = false;
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list) {
        if (&safe->list != head && strcmp(entry->value, safe->value) == 0) {
            duplicate = true;
            list_del(&entry->list);
            q_release_element(entry);
        } else if (duplicate) {
            duplicate = false;
            list_del(&entry->list);
            q_release_element(entry);
        }
    }
    return true;
}

/* Swap every two adjacent nodes */
void q_swap(struct list_head *head)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    struct list_head *node = head->next;
    while (node != head && node->next != head) {
        list_move(node, node->next);
        node = node->next;
    }
}

/* Reverse elements in queue */
void q_reverse(struct list_head *head)
{
    if (!head)
        return;
    struct list_head *prev = head, *node, *safe;
    list_for_each_safe(node, safe, head) {
        node->next = prev;
        node->prev = safe;
        prev = node;
    }
    head->next = prev;
    head->prev = safe;
}

/* Reverse the nodes of the list k at a time */
void q_reverseK(struct list_head *head, int k)
{
    if (!head || list_empty(head) || list_is_singular(head) || k < 2)
        return;
    struct list_head *prev = head, *node = head->next, *next;
    int count = q_size(head);
    while (count >= k) {
        for (int i = 1; i < k; i++) {
            next = node->next;
            list_move(next, prev);
        }
        prev = node;
        node = node->next;
        count -= k;
    }
}

void q_merge_two(struct list_head *head,
                 struct list_head *new_head,
                 bool descend)
{
    if (!head || !new_head || list_empty(new_head))
        return;
    if (list_empty(head)) {
        list_splice(new_head, head);
        return;
    }
    element_t *node = list_first_entry(head, element_t, list),
              *new_node = list_first_entry(new_head, element_t, list);
    while ((&node->list != head) && (&new_node->list != new_head)) {
        int cmp = strcmp(node->value, new_node->value);
        if ((!descend && cmp < 0) || (descend && cmp > 0)) {
            node = list_entry(node->list.next, element_t, list);
        } else {
            list_move_tail(&new_node->list, &node->list);
            new_node = list_first_entry(new_head, element_t, list);
        }
    }
    list_splice_tail_init(new_head, head);
}

/* Sort elements of queue in ascending/descending order */
void q_sort(struct list_head *head, bool descend)
{
    if (!head || list_empty(head) || list_is_singular(head))
        return;
    LIST_HEAD(new_head);
    struct list_head *mid = q_mid(head);
    list_cut_position(&new_head, head, mid->prev);
    q_sort(head, descend);
    q_sort(&new_head, descend);
    q_merge_two(head, &new_head, descend);
}

/* Remove every node which has a node with a strictly less value anywhere to
 * the right side of it */
int q_ascend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    LIST_HEAD(stack);
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list) {
        while (!list_empty(&stack) &&
               strcmp(list_last_entry(&stack, element_t, list)->value,
                      entry->value) >= 0) {
            element_t *top = list_last_entry(&stack, element_t, list);
            list_del(&top->list);
            q_release_element(top);
        }
        list_del(&entry->list);
        list_add_tail(&entry->list, &stack);
    }
    list_add(head, &stack);
    list_del(&stack);
    return q_size(head);
}

/* Remove every node which has a node with a strictly greater value anywhere to
 * the right side of it */
int q_descend(struct list_head *head)
{
    if (!head || list_empty(head))
        return 0;
    LIST_HEAD(stack);
    element_t *entry, *safe;
    list_for_each_entry_safe(entry, safe, head, list) {
        while (!list_empty(&stack) &&
               strcmp(list_last_entry(&stack, element_t, list)->value,
                      entry->value) <= 0) {
            element_t *top = list_last_entry(&stack, element_t, list);
            list_del(&top->list);
            q_release_element(top);
        }
        list_del(&entry->list);
        list_add_tail(&entry->list, &stack);
    }
    list_add(head, &stack);
    list_del(&stack);
    return q_size(head);
}

/* Merge all the queues into one sorted queue, which is in ascending/descending
 * order */
int q_merge(struct list_head *head, bool descend)
{
    if (!head || list_empty(head))
        return 0;
    queue_contex_t *entry, *safe,
        *first_chain = list_first_entry(head, queue_contex_t, chain);
    list_for_each_entry_safe(entry, safe, head, chain) {
        if (entry == first_chain)
            continue;
        q_merge_two(first_chain->q, entry->q, descend);
    }
    return q_size(first_chain->q);
}
