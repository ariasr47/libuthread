#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "queue.h"
#include <stdbool.h>

typedef struct Node {
    void* data;
    struct Node* next;
    struct Node* prev;
} Node;

struct queue {
    int length;
    Node* Head;
    Node* Tail;
};

Node* new_Node(void) {
    Node* temp = (Node*) malloc(sizeof(Node));
    temp->prev = NULL;
    temp->next = NULL;
    temp->data = NULL;
    return temp;
}

queue_t queue_create(void)
{
    void *temp = malloc(sizeof(struct queue));
    queue_t myQueue = NULL;
    
    // If malloc didnt fail, initialize queue_t
    if (temp != NULL) {
        myQueue = (queue_t) temp;
        myQueue->length = 0;
        myQueue->Head = NULL;
        myQueue->Tail = NULL;
    }

    return myQueue;
}

int queue_destroy(queue_t queue)
{
    int status = -1;
    if (queue != NULL && queue_length(queue) == 0) {
        free(queue);
        queue = NULL;
        status = 0;
    }
    return status;
}

int queue_enqueue(queue_t queue, void *data)
{
    int status = -1;
    if (queue != NULL && data != NULL) {
        if (queue_length(queue) == 0) {
            queue->Head = new_Node();
            queue->Head->data = data;
            queue->Tail = queue->Head;
        } else {
            queue->Tail->next = new_Node();
            queue->Tail->next->prev = queue->Tail;
            queue->Tail = queue->Tail->next;
            queue->Tail->data = data;
        }
        queue->length++;
        status = 0;
    }
    return status;
}

int queue_dequeue(queue_t queue, void **data)
{
    int status = -1;
    if (queue != NULL && data != NULL && queue_length(queue) > 0) {
        Node* temp = queue->Head->next;
        *data = queue->Head->data;
        free(queue->Head);
        queue->Head = temp;
        queue->length--;
        if (queue->length == 0)
            queue->Tail = NULL;
        status = 0;
    } 
    return status;
}

int queue_delete(queue_t queue, void *data)
{
    int status = -1;

    if (queue != NULL && data != NULL) {
        Node* curr = queue->Head;
        while (curr != NULL) {
            if (curr->data == data) {
                if (curr == queue->Head) {
                    queue->Head = curr->next;
                } else if (curr == queue->Tail) {
                    queue->Tail = curr->prev;
                } else {
                    curr->prev->next = curr->next;
                    curr->next->prev = curr->prev;
                }
                free(curr->data);
                free(curr);
                status = 0;
                break;
            }
            curr = curr->next;
        }
    }
    return status;
}

int queue_iterate(queue_t queue, queue_func_t func, void *arg, void **data)
{
    int status = -1;
    if (queue != NULL && func != NULL) {
        bool flag = false;
        Node* curr = queue->Head;
        while (curr != NULL) {
            flag = func(curr->data,arg);
            if (flag == true) {
                if (data!= NULL)
                    *data = curr->data;
                break;
            }
            curr = curr->next;
        }
        status = 0;
    }
    
    return status;
}

int queue_length(queue_t queue)
{
    int length = -1;
    if (queue != NULL)
        length = queue->length;
    return length;
}
