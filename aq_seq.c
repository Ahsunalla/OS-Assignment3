/**
 * @file   aq_seq.c
 * @Author Your group info
 * @date   October, 2024
 * @brief  Sequential alarm queue implementation
 */

#include "aq.h"
#include <stdlib.h>
#include <stdio.h>

// Node structure for the linked list of normal messages
typedef struct Node {
    void *msg;
    int sequence;  // To maintain insertion order
    struct Node *next;
} Node;

// Main queue structure
typedef struct {
    Node *head;           // Head of normal messages list
    Node *tail;           // Tail for O(1) insertion
    void *alarm_msg;      // Single alarm message slot
    int normal_count;     // Count of normal messages
    int has_alarm;        // Boolean flag for alarm message
    int sequence_counter; // To track message order
} AQStruct;

AlarmQueue aq_create() {
    AQStruct *aq = (AQStruct *)malloc(sizeof(AQStruct));
    if (!aq) return NULL;
    
    aq->head = NULL;
    aq->tail = NULL;
    aq->alarm_msg = NULL;
    aq->normal_count = 0;
    aq->has_alarm = 0;
    aq->sequence_counter = 0;
    
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
        printf("DEBUG: Sent ALARM message with value %d\n", *(int*)msg);
    } else {
        // Create new node for normal message
        Node *new_node = (Node *)malloc(sizeof(Node));
        if (!new_node) return AQ_NO_ROOM;
        
        new_node->msg = msg;
        new_node->sequence = queue->sequence_counter++;
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
        
        // Debug: Print current queue state
        Node *current = queue->head;
        printf("DEBUG: Queue contents: ");
        while (current != NULL) {
            printf("%d -> ", *(int*)current->msg);
            current = current->next;
        }
        printf("NULL\n");
    }
    
    return 0;
}

// Helper function to find the next message to retrieve
static Node* find_next_message(Node *head, int current_sequence) {
    Node *result = head;
    Node *prev = NULL;
    Node *min_node = head;
    Node *min_prev = NULL;
    
    while (result != NULL) {
        if (result->sequence > current_sequence) {
            min_node = result;
            min_prev = prev;
            break;
        }
        prev = result;
        result = result->next;
    }
    
    return min_node;
}

int aq_recv(AlarmQueue aq, void **msg) {
    if (!aq) return AQ_UNINIT;
    if (!msg) return AQ_NULL_MSG;
    
    AQStruct *queue = (AQStruct *)aq;
    
    // Check if there are any messages
    if (!queue->has_alarm && !queue->head) {
        *msg = NULL;
        printf("DEBUG: No messages available in queue\n");
        return AQ_NO_MSG;
    }
    
    // Priority to alarm message
    if (queue->has_alarm) {
        *msg = queue->alarm_msg;
        queue->alarm_msg = NULL;
        queue->has_alarm = 0;
        printf("DEBUG: Received ALARM message with value %d\n", *(int*)*msg);
        return AQ_ALARM;
    }
    
    // Get normal message from head of queue
    if (queue->head) {
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
        return AQ_NORMAL;
    }
    
    return AQ_NO_MSG;
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
