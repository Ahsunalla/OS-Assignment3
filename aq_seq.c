/**
 * @file   aq_seq.c
 * @Author Your group info
 * @date   October, 2024
 * @brief  Sequential alarm queue implementation
 */

#include "aq.h"
#include <stdlib.h>

// Node structure for the linked list of normal messages
typedef struct Node {
    void *msg;
    struct Node *next;
} Node;

// Main queue structure
typedef struct {
    Node *head;           // Head of normal messages list
    Node *tail;           // Tail for O(1) insertion
    void *alarm_msg;      // Single alarm message slot
    int normal_count;     // Count of normal messages
    int has_alarm;        // Boolean flag for alarm message
} AQStruct;

AlarmQueue aq_create() {
    AQStruct *aq = (AQStruct *)malloc(sizeof(AQStruct));
    if (!aq) return NULL;
    
    aq->head = NULL;
    aq->tail = NULL;
    aq->alarm_msg = NULL;
    aq->normal_count = 0;
    aq->has_alarm = 0;
    
    return (AlarmQueue)aq;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (!aq) return AQ_UNINIT;
    if (!msg) return AQ_NULL_MSG;
    
    AQStruct *queue = (AQStruct *)aq;
    
    if (k == AQ_ALARM) {
        // Check if there's already an alarm message
        if (queue->has_alarm) {
            return AQ_NO_ROOM;
        }
        queue->alarm_msg = msg;
        queue->has_alarm = 1;
    } else {
        // Create new node for normal message
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (!new_node) return AQ_NO_ROOM;
        
        new_node->msg = msg;
        new_node->next = NULL;
        
        // Add to tail of normal messages list
        if (!queue->head) {
            queue->head = new_node;
        } else {
            queue->tail->next = new_node;
        }
        queue->tail = new_node;
        queue->normal_count++;
    }
    
    return 0;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (!aq) return AQ_UNINIT;
    if (!msg) return AQ_NULL_MSG;
    
    AQStruct *queue = (AQStruct *)aq;
    
    // Check if there are any messages
    if (!queue->has_alarm && !queue->head) {
        *msg = NULL;
        return AQ_NO_MSG;
    }
    
    // Priority to alarm message
    if (queue->has_alarm) {
        *msg = queue->alarm_msg;
        queue->alarm_msg = NULL;
        queue->has_alarm = 0;
        return AQ_ALARM;
    }
    
    // Get normal message from head of queue
    Node *node = queue->head;
    *msg = node->msg;
    queue->head = node->next;
    if (!queue->head) {
        queue->tail = NULL;
    }
    queue->normal_count--;
    free(node);
    
    return AQ_NORMAL;
}

int aq_size(AlarmQueue aq) {
    if (!aq) return 0;
    AQStruct *queue = (AQStruct *)aq;
    return queue->normal_count + queue->has_alarm;
}

int aq_alarms(AlarmQueue aq) {
    if (!aq) return 0;
    AQStruct *queue = (AQStruct *)aq;
    return queue->has_alarm;
}
