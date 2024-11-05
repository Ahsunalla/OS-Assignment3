#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <assert.h>
#include "aq.h"
#include "_aux.h"

static AlarmQueue q;

// Thread that sends normal messages
void* normal_sender(void* arg) {
    printf("Normal Sender: Starting to send normal messages\n");
    
    // Send several normal messages
    put_normal(q, 1);
    put_normal(q, 2);
    msleep(50);  // Give time for alarm to be sent
    put_normal(q, 3);  // These messages should not block
    put_normal(q, 4);  // Even though an alarm exists
    
    return NULL;
}

// Thread that sends an alarm message
void* alarm_sender(void* arg) {
    msleep(100);  // Wait for some normal messages to be queued
    
    printf("Alarm Sender: Sending alarm message 99\n");
    put_alarm(q, 99);  // This should be received before remaining normal messages
    
    return NULL;
}

// Thread that receives messages and verifies order
void* consumer(void* arg) {
    msleep(200);  // Give time for messages to be queued
    
    // Should receive alarm message first (priority)
    int msg = get(q);
    printf("Consumer: Received message %d (should be alarm 99)\n", msg);
    assert(msg == 99);
    
    // Then receive normal messages in order
    msg = get(q);
    printf("Consumer: Received message %d (should be normal 1)\n", msg);
    assert(msg == 1);
    
    msg = get(q);
    printf("Consumer: Received message %d (should be normal 2)\n", msg);
    assert(msg == 2);
    
    msg = get(q);
    printf("Consumer: Received message %d (should be normal 3)\n", msg);
    assert(msg == 3);
    
    msg = get(q);
    printf("Consumer: Received message %d (should be normal 4)\n", msg);
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
    
    printf("Starting test of message priorities and ordering...\n");
    printf("This test demonstrates:\n");
    printf("1. Normal messages are never blocked\n");
    printf("2. Normal messages maintain their order\n");
    printf("3. Alarm messages have priority over normal messages\n");
    
    // Create threads
    pthread_create(&t1, NULL, normal_sender, NULL);
    pthread_create(&t2, NULL, alarm_sender, NULL);
    pthread_create(&t3, NULL, consumer, NULL);
    
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
