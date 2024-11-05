#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "aq.h"
#include "_aux.h"

static AlarmQueue q;

// Thread 1: Sends first alarm and a normal message
void* sender1(void* arg) {
    printf("Thread 1: Sending alarm message 1\n");
    put_alarm(q, 1);
    
    msleep(100);  // Give time for Thread 2 to attempt sending alarm
    
    printf("Thread 1: Sending normal message 2\n");
    put_normal(q, 2);  // This message helps detect when Thread 2 unblocks
    
    return NULL;
}

// Thread 2: Tries to send second alarm (should block)
void* sender2(void* arg) {
    msleep(50);  // Ensure first alarm is sent first
    
    printf("Thread 2: Attempting to send alarm message 3 (should block)\n");
    put_alarm(q, 3);  // Should block until first alarm is received
    
    printf("Thread 2: Alarm message 3 sent (unblocked)\n");
    put_normal(q, 4);  // This message should appear after message 2
    
    return NULL;
}

// Thread 3: Receives messages and verifies order
void* receiver(void* arg) {
    msleep(200);  // Give time for Thread 2 to block
    
    // Receive first alarm
    int msg = get(q);
    printf("Thread 3: Received message %d (should be alarm 1)\n", msg);
    assert(msg == 1);
    
    msleep(50);  // Give time for Thread 2 to unblock
    
    // Receive second alarm (after it unblocked, has priority over normal messages)
    msg = get(q);
    printf("Thread 3: Received message %d (should be alarm 3)\n", msg);
    assert(msg == 3);
    
    // Now receive normal messages in order
    msg = get(q);
    printf("Thread 3: Received message %d (should be normal 2)\n", msg);
    assert(msg == 2);
    
    msg = get(q);
    printf("Thread 3: Received message %d (should be normal 4)\n", msg);
    assert(msg == 4);
    
    return NULL;
}

int main(int argc, char** argv) {
    q = aq_create();
    if (q == NULL) {
        printf("Alarm queue could not be created\n");
        exit(1);
    }
    
    pthread_t t1, t2, t3;
    void *res1, *res2, *res3;
    
    printf("Starting test of alarm message blocking...\n");
    printf("This test demonstrates:\n");
    printf("1. Sending an alarm message blocks when another alarm exists\n");
    printf("2. The blocked alarm unblocks when the first alarm is received\n");
    printf("3. Alarm messages have priority over normal messages\n");
    printf("4. Normal messages maintain their order after alarms\n");
    
    // Create threads
    pthread_create(&t1, NULL, sender1, NULL);
    pthread_create(&t2, NULL, sender2, NULL);
    pthread_create(&t3, NULL, receiver, NULL);
    
    // Wait for all threads to complete
    pthread_join(t1, &res1);
    pthread_join(t2, &res2);
    pthread_join(t3, &res3);
    
    printf("Test completed successfully!\n");
    printf("Final queue size: %d (should be 0)\n", aq_size(q));
    assert(aq_size(q) == 0);
    assert(aq_alarms(q) == 0);
    
    return 0;
}
