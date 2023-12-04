/**
 * Type-V Virtual Machine
 * Author: praisethemoon
 * queue.h: Message Passing Queue
 * Messages are passed between processes through message passing.
 * Each process/core has its own message queue.
 */

#ifndef TYPE_V_QUEUE_H
#define TYPE_V_QUEUE_H

#include <stdint.h>


#define MAX_QUEUE_LENGTH 100

/**
 * @brief Message structure
 */
typedef struct TypeV_IOMessage {
    uint32_t sender;        ///< Sender ID
    void* message;          ///< Message Data Pointer
    struct TypeV_Promise* promise; ///< Promise Pointer
} TypeV_IOMessage;

/**
 * @brief Message Node
 * Node for a message queue
 */
typedef struct TypeV_IOMessageNode {
    TypeV_IOMessage* data;             ///< Message Brief (Sender ID, Message Data Pointer)
    struct TypeV_IOMessageNode* next; ///< Next Node
} TypeV_IOMessageNode;

/**
 * @brief Message Queue
 */
typedef struct TypeV_IOMessageQueue {
    TypeV_IOMessageNode* front; ///< Front Node
    TypeV_IOMessageNode* rear;  ///< Rear Node
    uint64_t length;            ///< Queue Length
} TypeV_IOMessageQueue;

/**
 * Initialize a queue
 * @param queue
 */
void queue_init(TypeV_IOMessageQueue *queue);

/**
 * Push a message to the queue
 * @param queue
 * @param message
 */
void queue_enqueue(TypeV_IOMessageQueue *queue, TypeV_IOMessage* message);

/**
 * Dequeues a message from the queue
 * @param queue
 * @return
 */
TypeV_IOMessage* queue_dequeue(TypeV_IOMessageQueue *queue);

/**
 * deallocate a queue
 * @param queue
 * @return
 */
void queue_deallocate(TypeV_IOMessageQueue *queue);


#endif //TYPE_V_QUEUE_H
