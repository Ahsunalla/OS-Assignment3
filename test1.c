#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "aq.h"
#include "_aux.h"

static AlarmQueue q;
static pthread_t t1, t2, t3;

// Thread 1: Sends alarms and normal messages
void* sender(void* arg) {
    int alarm1 = 1;
    int alarm2 = 2;
    int normal1 = 3;
    int normal2 = 4;

    printf("Sending first alarm (A1)\n");
    put_alarm(q, alarm1);
    
    printf("Attempting to send second alarm (A2) - should block\n");
    put_alarm(q, alarm2);  // This should block until first alarm is received
    
    printf("Second alarm sent successfully after unblocking\n");
    
    // Send normal messages to verify ordering
    printf("Sending normal messages\n");
    put_normal(q, normal1);
    put_normal(q, normal2);
    
    return NULL;
}

// Thread 2: Receives messages after a delay
void* delayed_receiver(void* arg) {
    // Wait to ensure first alarm is queued
    msleep(500);
    
    void* msg;
    int ret;
    
    printf("Receiving first alarm\n");
    ret = aq_recv(q, &msg);
    if (ret == AQ_ALARM) {
        printf("Received alarm message: %d\n", *(int*)msg);
    }
    
    return NULL;
}

// Thread 3: Monitors queue state
void* monitor(void* arg) {
    while(1) {
        msleep(100);
        printf("Queue state - Size: %d, Alarms: %d\n", 
               aq_size(q), aq_alarms(q));
    }
    return NULL;
}

int main(int argc, char** argv) {
    printf("Starting Alarm Queue Blocking Test\n");
    
    q = aq_create();
    if (q == NULL) {
        printf("Failed to create alarm queue\n");
        return 1;
    }
    
    // Create threads
    pthread_create(&t1, NULL, sender, NULL);
    pthread_create(&t2, NULL, delayed_receiver, NULL);
    pthread_create(&t3, NULL, monitor, NULL);
    
    // Wait for sender and receiver to complete
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    // Cancel monitor thread
    pthread_cancel(t3);
    pthread_join(t3, NULL);
    
    // Print final queue state
    printf("\nFinal queue state:\n");
    print_sizes(q);
    
    return 0;
}
