/**
 * @file   aq_tsafe.c
 * @Author Your group info
 * @date   October, 2024
 * @brief  Thread-safe alarm queue implementation
 */

#include "aq.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

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
    pthread_mutex_t mutex;       // Mutex for thread safety
    pthread_cond_t not_empty;    // Condition for empty queue
    pthread_cond_t alarm_freed;  // Condition for alarm slot becoming free
} AQStruct;

AlarmQueue aq_create() {
    AQStruct *aq = (AQStruct *)malloc(sizeof(AQStruct));
    if (!aq) return NULL;
    
    aq->head = NULL;
    aq->tail = NULL;
    aq->alarm_msg = NULL;
    aq->normal_count = 0;
    aq->has_alarm = 0;
    
    // Initialize synchronization primitives
    if (pthread_mutex_init(&aq->mutex, NULL) != 0) {
        free(aq);
        return NULL;
    }
    
    if (pthread_cond_init(&aq->not_empty, NULL) != 0) {
        pthread_mutex_destroy(&aq->mutex);
        free(aq);
        return NULL;
    }
    
    if (pthread_cond_init(&aq->alarm_freed, NULL) != 0) {
        pthread_mutex_destroy(&aq->mutex);
        pthread_cond_destroy(&aq->not_empty);
        free(aq);
        return NULL;
    }
    
    return (AlarmQueue)aq;
}

int aq_send(AlarmQueue aq, void *msg, MsgKind k) {
    if (!aq) return AQ_UNINIT;
    if (!msg) return AQ_NULL_MSG;
    
    AQStruct *queue = (AQStruct *)aq;
    
    pthread_mutex_lock(&queue->mutex);
    
    if (k == AQ_ALARM) {
        // Wait while there's an alarm message
        while (queue->has_alarm) {
            pthread_cond_wait(&queue->alarm_freed, &queue->mutex);
        }
        queue->alarm_msg = msg;
        queue->has_alarm = 1;
        printf("DEBUG: Sent ALARM message with value %d\n", *(int*)msg);
        pthread_cond_signal(&queue->not_empty);
    } else {
        // Create new node for normal message
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (!new_node) {
            pthread_mutex_unlock(&queue->mutex);
            return AQ_NO_ROOM;
        }
        
        new_node->msg = msg;
        new_node->next = NULL;
        
        printf("DEBUG: Sending normal message with value %d\n", *(int*)msg);
        
        // Add to tail of normal messages list
        if (!queue->head) {
            queue->head = new_node;
        } else {
            queue->tail->next = new_node;
        }
        queue->tail = new_node;
        queue->normal_count++;
        
        // Signal waiting consumers
        pthread_cond_signal(&queue->not_empty);
        
        // Debug: Print current queue state
        Node *current = queue->head;
        printf("DEBUG: Queue contents: ");
        while (current != NULL) {
            printf("%d -> ", *(int*)current->msg);
            current = current->next;
        }
        printf("NULL\n");
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return 0;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (!aq) return AQ_UNINIT;
    if (!msg) return AQ_NULL_MSG;
    
    AQStruct *queue = (AQStruct *)aq;
    int result;
    
    pthread_mutex_lock(&queue->mutex);
    
    // Wait while queue is empty
    while (!queue->has_alarm && !queue->head) {
        pthread_cond_wait(&queue->not_empty, &queue->mutex);
    }
    
    // Priority to alarm message
    if (queue->has_alarm) {
        *msg = queue->alarm_msg;
        queue->alarm_msg = NULL;
        queue->has_alarm = 0;
        printf("DEBUG: Received ALARM message with value %d\n", *(int*)*msg);
        // Signal that alarm slot is now free
        pthread_cond_signal(&queue->alarm_freed);
        result = AQ_ALARM;
    } else {
        // Get normal message from head of queue
        Node *node = queue->head;
        *msg = node->msg;
        queue->head = node->next;
        
        // Update tail if queue becomes empty
        if (!queue->head) {
            queue->tail = NULL;
        }
        
        printf("DEBUG: Received normal message with value %d\n", *(int*)*msg);
        
        // Debug: Print remaining queue state
        Node *current = queue->head;
        printf("DEBUG: Remaining queue: ");
        while (current != NULL) {
            printf("%d -> ", *(int*)current->msg);
            current = current->next;
        }
        printf("NULL\n");
        
        queue->normal_count--;
        free(node);
        result = AQ_NORMAL;
    }
    
    pthread_mutex_unlock(&queue->mutex);
    return result;
}

int aq_size(AlarmQueue aq) {
    if (!aq) return 0;
    AQStruct *queue = (AQStruct *)aq;
    
    pthread_mutex_lock(&queue->mutex);
    int size = queue->normal_count + queue->has_alarm;
    pthread_mutex_unlock(&queue->mutex);
    
    return size;
}

int aq_alarms(AlarmQueue aq) {
    if (!aq) return 0;
    AQStruct *queue = (AQStruct *)aq;
    
    pthread_mutex_lock(&queue->mutex);
    int alarms = queue->has_alarm;
    pthread_mutex_unlock(&queue->mutex);
    
    return alarms;
}
