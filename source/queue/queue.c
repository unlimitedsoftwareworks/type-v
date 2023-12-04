//
// Created by praisethemoon on 21.11.23.
//

#include <stdlib.h>
#include "queue.h"

void queue_init(TypeV_IOMessageQueue *queue) {
    queue->front = NULL;
    queue->rear = NULL;
    queue->length = 0;
}

void queue_enqueue(TypeV_IOMessageQueue *queue, TypeV_IOMessage* message) {
    TypeV_IOMessageNode *newNode = (TypeV_IOMessageNode *)malloc(sizeof(TypeV_IOMessageNode));
    if (newNode == NULL) {
        // Handle memory allocation failure
        return;
    }

    newNode->data = message;
    newNode->next = NULL;

    if (queue->rear == NULL) {
        // Queue is empty
        queue->front = newNode;
        queue->rear = newNode;
    } else {
        // Queue is not empty
        queue->rear->next = newNode;
        queue->rear = newNode;
    }

    queue->length++;
}

TypeV_IOMessage* queue_dequeue(TypeV_IOMessageQueue *queue) {
    if (queue->front == NULL) {
        // Queue is empty, return a default or error value
        return NULL;
    }

    TypeV_IOMessageNode *temp = queue->front;
    TypeV_IOMessage* message = temp->data;
    queue->front = queue->front->next;

    if (queue->front == NULL) {
        // Queue becomes empty
        queue->rear = NULL;
    }

    free(temp);
    queue->length--;

    return message;
}

void queue_deallocate(TypeV_IOMessageQueue *queue) {
    while (queue->front != NULL) {
        TypeV_IOMessageNode *temp = queue->front;
        queue->front = queue->front->next;
        free(temp);
    }
    queue->rear = NULL;
    queue->length = 0;
}




