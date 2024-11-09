#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

#include "aq.h"
#include "_aux.h"

static AlarmQueue q;
static pthread_t t1, t2;

// Thread 1: Sends mixed messages to test priority
void* sender(void* arg) {
    int normal1 = 1;
    int normal2 = 2;
    int alarm1 = 100;
    int normal3 = 3;
    
    printf("Sending normal message 1\n");
    put_normal(q, normal1);
    
    printf("Sending normal message 2\n");
    put_normal(q, normal2);
    
    // Send alarm message - should be received before remaining normal messages
    printf("Sending alarm message\n");
    put_alarm(q, alarm1);
    
    printf("Sending normal message 3\n");
    put_normal(q, normal3);
    
    return NULL;
}

// Thread 2: Receives messages and verifies priority
void* receiver(void* arg) {
    void* msg;
    int ret;
    int count = 0;
    
    while(count < 4) {  // Expect 4 messages total
        ret = aq_recv(q, &msg);
        if (ret == AQ_ALARM) {
            printf("Received alarm message: %d (Priority received)\n", *(int*)msg);
        } else if (ret == AQ_NORMAL) {
            printf("Received normal message: %d\n", *(int*)msg);
        }
        count++;
    }
    
    return NULL;
}

int main(int argc, char** argv) {
    printf("Starting Alarm Queue Priority Test\n");
    
    q = aq_create();
    if (q == NULL) {
        printf("Failed to create alarm queue\n");
        return 1;
    }
    
    // Create threads
    pthread_create(&t1, NULL, sender, NULL);
    pthread_create(&t2, NULL, receiver, NULL);
    
    // Wait for completion
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    
    printf("\nFinal queue state:\n");
    print_sizes(q);
    
    return 0;
}
