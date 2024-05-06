#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 2
#define NUM_ITERATIONS 1000000

int count = 0;

void *increment(void *arg) {
    for (int i = 0; i < NUM_ITERATIONS; i++) {
        count++;
    }
    return NULL;
}

int main() {
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, increment, NULL) != 0) {
            fprintf(stderr, "Error creating thread\n");
            return 1;
        }
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        if (pthread_join(threads[i], NULL) != 0) {
            fprintf(stderr, "Error joining thread\n");
            return 2;
        }
    }

    printf("Expected count: %d\n", NUM_THREADS * NUM_ITERATIONS);
    printf("Actual count: %d\n", count);

    return 0;
}
