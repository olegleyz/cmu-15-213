/* 
 * Code for basic C skills diagnostic.
 * Developed for courses 15-213/18-213/15-513 by R. E. Bryant, 2017
 * Modified to store strings, 2018
 */

/*
 * This program implements a queue supporting both FIFO and LIFO
 * operations.
 *
 * It uses a singly-linked list to represent the set of queue elements
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "harness.h"
#include "queue.h"

#define MIN(a, b) ((a) < (b) ? (a) : (b))
static bool VERBAL = false;

void q_traverse(queue_t *q)
{
  if (q == NULL) {
    return;
  }

  if (VERBAL == false) {
    return;
  }

  list_ele_t *node = q->head;
  printf("Queue traversal. Head: %p (%s). Tail: %p (%s)\n",
       q->head, q->head ? q->head->value : "NULL",
       q->tail, q->tail ? q->tail->value : "NULL");
  while (node != NULL) {
    printf("%p(%s)->", node, node->value);
    node = node->next;
  }
  printf("\n");
}

/*
  Create empty queue.
  Return NULL if could not allocate space.
*/
queue_t *q_new()
{
    queue_t *q = malloc(sizeof(queue_t));
    /* Return NULL if malloc returned NULL (could not allocate space). */
    if (q == NULL) {
      return NULL;
    }

    q->head = NULL;
    q->size = 0;
    q->tail = NULL;
    return q;
}

/* Free all storage used by queue */
void q_free(queue_t *q)
{
    if (q == NULL) {
      return;
    }

    /* Freeing the list elements and the strings */
    while (q->head != NULL) {
      list_ele_t *oldh = q->head;
      q->head = q->head->next;

      free(oldh->value);
      free(oldh);
    }
    /* Free queue structure */
    free(q);
}

list_ele_t *create_node(const char *s)
{
  list_ele_t *node;
  node = malloc(sizeof(list_ele_t));
  if (node == NULL) {
    return NULL;
  }

  node->value = malloc(strlen(s)+1);
  if (node->value == NULL) {
    free(node);
    return NULL;
  }
  node->next = NULL;

  strcpy(node->value, s);
  return node;
}

/*
  Attempt to insert element at head of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_head(queue_t *q, const char *s)
{   
    /* Return false if the q is NULL */
    if (q == NULL) {
      return false;
    }
 
    list_ele_t *new_head = create_node(s);
    if (new_head == NULL) {
      return false;
    }
    
    new_head->next = q->head;
    q->head = new_head;
    q->size++;

    if (q->size == 1) {
      q->tail = new_head;
    }

    q_traverse(q);
    return true;
}

/*
  Attempt to insert element at tail of queue.
  Return true if successful.
  Return false if q is NULL or could not allocate space.
  Argument s points to the string to be stored.
  The function must explicitly allocate space and copy the string into it.
 */
bool q_insert_tail(queue_t *q, const char *s)
{
    if (q == NULL) {
      return false;
    }
    if (q->size == 0) {
      return q_insert_head(q, s);
    } else {
      list_ele_t *new_tail = create_node(s);
      if (new_tail == NULL) {
       return false;
      }

      q->tail->next = new_tail;
      q->tail = new_tail;
      q->size++;
    }

    q_traverse(q);
    return true;
}

/*
  Attempt to remove element from head of queue.
  Return true if successful.
  Return false if queue is NULL or empty.
  If sp is non-NULL and an element is removed, copy the removed string to *sp
  (up to a maximum of bufsize-1 characters, plus a null terminator.)
  The space used by the list element and the string should be freed.
*/
bool q_remove_head(queue_t *q, char *sp, const size_t bufsize)
{
    if (q == NULL || q->head == NULL) {
      return false;
    }

    list_ele_t *node = q->head;

    if (sp != NULL) {
      strncpy(sp, node->value, bufsize-1);  
      sp[MIN(strlen(node->value), bufsize-1)]='\0';
    }
    
    q->head = node->next;
    q->size--;

    if (q->size == 0) {
      q->head = NULL;
      q->tail = NULL;
    }

    free(node->value);
    free(node);

    node = NULL;

    q_traverse(q);
    return true;
}

/*
  Return number of elements in queue.
  Return 0 if q is NULL or empty
 */
int q_size(queue_t *q)
{
    /* You need to write the code for this function */
    /* Remember: It should operate in O(1) time */
    if (q == NULL) {
      return 0;
    }

    return q->size;
}

/*
  Reverse elements in queue
  No effect if q is NULL or empty
  This function should not allocate or free any list elements
  (e.g., by calling q_insert_head, q_insert_tail, or q_remove_head).
  It should rearrange the existing ones.
 */
void q_reverse(queue_t *q)
{
    /* You need to write the code for this function */
    if (q == NULL || q->size <= 1) {
      return;
    }

    list_ele_t *node = q->head;
    list_ele_t *prev = NULL;
    list_ele_t *temp;

    while (node != NULL) {
      temp = node->next;
      node->next = prev;
      prev = node;
      node = temp;
    }

    q->tail = q->head;
    q->head = prev;
  
    q_traverse(q);
}

