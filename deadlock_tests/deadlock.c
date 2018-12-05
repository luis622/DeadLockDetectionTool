#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

pthread_mutex_t lock1, lock2;

void *resource1() {

    printf("Job started in resource1\n");
    for (int i=0; i<10; i++) {
        pthread_mutex_lock(&lock1);
        // printf("lock1 aquired %d\n", i);
        printf("Lock1 aquired\n");

        // sleep(2);
        
        pthread_mutex_lock(&lock2);
        // printf("lock2 aquired %d\n", i);
        printf("Lock2 aquired\n");

        pthread_mutex_unlock(&lock1);
        pthread_mutex_unlock(&lock2);
    }
    return NULL;
}

void *resource2() {

    printf("Job started in resource2\n");
    for (int i=0; i<10; i++) {
        pthread_mutex_lock(&lock2);
        // printf("lock2 aquired %d\n", i);
        printf("Lock2 aquired\n");

        // sleep(2);
        
        pthread_mutex_lock(&lock1);
        // printf("lock1 aquired %d\n", i);
        printf("Lock1 aquired\n");

        pthread_mutex_unlock(&lock2);
        pthread_mutex_unlock(&lock1);
    }
    return NULL;
}

int main() {

    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_t thread_id1,thread_id2;

    int ret1, ret2;

    if ((ret1=pthread_create(&thread_id1, NULL, &resource1, NULL))) {
        printf("Thread creation failed: %d\n", ret1);
    }

    if ((ret2=pthread_create(&thread_id2, NULL, &resource2, NULL))) {
        printf("Thread creation failed: %d\n", ret2);
    }

    printf("Created threads");
    pthread_join(thread_id1, NULL);
    pthread_join(thread_id2, NULL);

    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);

    return 0;
}