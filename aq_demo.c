#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "aq.h"
#include "_aux.h"

/** 
 * Sequential program that sends and receives a few integer messages 
 * in order to demonstrate the use of the sequential Alarm Queue Library
 *
 * May also be used as a first simple test
 * 
 * Elaborate on your own
 */

int main(int argc, char ** argv) {
    AlarmQueue q = aq_create();
    if (q == NULL) {
        printf("Alarm queue could not be created\n");
        exit(1);
    }

    put_normal(q, 1);
    put_normal(q, 2);
    put_alarm(q, 3);
    assert( put_alarm(q, 4) == AQ_NO_ROOM );
    put_normal(q, 5);

    /* Verify queue size and state */
    printf("DEBUG: Verifying initial queue size\n");
    print_sizes(q);

    /* Retrieve messages and verify order */
    printf("DEBUG: Retrieving messages from queue\n");

    int received_value = get(q);
    printf("Expected: 3, Received: %d\n", received_value);
    assert(received_value == 3);

    put_alarm(q, 6);
    put_normal(q, 7);
    assert(put_alarm(q, 8) == AQ_NO_ROOM);
    put_normal(q, 9);

    /* Verify queue size and state again */
    printf("DEBUG: Verifying updated queue size\n");
    print_sizes(q);

    received_value = get(q);
    printf("Expected: 6, Received: %d\n", received_value);
    assert(received_value == 6);

    received_value = get(q);
    printf("Expected: 1, Received: %d\n", received_value);
    assert(received_value == 1);

    received_value = get(q);
    printf("Expected: 2, Received: %d\n", received_value);
    assert(received_value == 2);

    received_value = get(q);
    printf("Expected: 5, Received: %d\n", received_value);
    assert(received_value == 5);

    received_value = get(q);
    printf("Expected: 7, Received: %d\n", received_value);
    assert(received_value == 7);

    received_value = get(q);
    printf("Expected: 9, Received: %d\n", received_value);
    assert(received_value == 9);

    /* Now the queue should be empty */
    print_sizes(q);
    assert(get(q) == AQ_NO_MSG);

    return 0;
}
