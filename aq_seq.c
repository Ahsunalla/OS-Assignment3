/**
 * @file   aq_seq.c
 * @Author Your group number
 * @date   October, 2024
 * @brief  Sequential alarm queue implementation
 */

#include <stdlib.h>
#include <stdio.h>
#include "aq.h"

#define MAX_NORMAL_MESSAGES 1000

typedef struct {
    void* normal_messages[MAX_NORMAL_MESSAGES];
    int normal_count;
    void* alarm_message;
    int has_alarm;
} Queue;

AlarmQueue aq_create() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    if (q == NULL) return NULL;
    
    q->normal_count = 0;
    q->alarm_message = NULL;
    q->has_alarm = 0;
    
    return q;
}

int aq_send(AlarmQueue aq, void* msg, MsgKind k) {
    Queue* q = (Queue*)aq;
    if (q == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;
    
    if (k == AQ_ALARM) {
        if (q->has_alarm) return AQ_NO_ROOM;
        q->alarm_message = msg;
        q->has_alarm = 1;
    } else {
        // Normal message
        if (q->normal_count >= MAX_NORMAL_MESSAGES) return AQ_NO_ROOM;
        // Add to end of array
        q->normal_messages[q->normal_count] = msg;
        q->normal_count++;
    }
    
    return 0;
}

int aq_recv(AlarmQueue aq, void** msg) {
    Queue* q = (Queue*)aq;
    if (q == NULL) return AQ_UNINIT;
    if (msg == NULL) return AQ_NULL_MSG;
    
    // Check if there are any messages
    if (q->has_alarm == 0 && q->normal_count == 0) {
        *msg = NULL;
        return AQ_NO_MSG;
    }
    
    // Priority to alarm messages
    if (q->has_alarm) {
        *msg = q->alarm_message;
        q->alarm_message = NULL;
        q->has_alarm = 0;
        return AQ_ALARM;
    }
    
    // Get first normal message
    if (q->normal_count > 0) {
        *msg = q->normal_messages[0];
        // Shift remaining messages left
        for (int i = 1; i < q->normal_count; i++) {
            q->normal_messages[i-1] = q->normal_messages[i];
        }
        q->normal_count--;
        return AQ_NORMAL;
    }
    
    *msg = NULL;
    return AQ_NO_MSG;
}

int aq_size(AlarmQueue aq) {
    Queue* q = (Queue*)aq;
    if (q == NULL) return 0;
    return q->normal_count + q->has_alarm;
}

int aq_alarms(AlarmQueue aq) {
    Queue* q = (Queue*)aq;
    if (q == NULL) return 0;
    return q->has_alarm;
}
