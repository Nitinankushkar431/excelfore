#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#define MAX_MESSAGES 5

// Structure for the shared queue
typedef struct {
    char* messages[MAX_MESSAGES];
    int count;
    int front;
    int rear;
    pthread_mutex_t mutex;
    pthread_cond_t writer_condition;
    pthread_cond_t reader_condition;
} SharedQueue;

// Initialize the shared queue
void initQueue(SharedQueue* queue) {
    queue->count = 0;
    queue->front = 0;
    queue->rear = -1;
    pthread_mutex_init(&queue->mutex, NULL);
    pthread_cond_init(&queue->writer_condition, NULL);
    pthread_cond_init(&queue->reader_condition, NULL);
}

// Enqueue a message
void enqueue(SharedQueue* queue, const char* message) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count >= MAX_MESSAGES) {
        pthread_cond_wait(&queue->writer_condition, &queue->mutex);
    }

    queue->rear = (queue->rear + 1) % MAX_MESSAGES;
    queue->messages[queue->rear] = strdup(message);
    queue->count++;

    pthread_cond_signal(&queue->reader_condition);
    pthread_mutex_unlock(&queue->mutex);
}

// Dequeue a message
char* dequeue(SharedQueue* queue) {
    pthread_mutex_lock(&queue->mutex);

    while (queue->count == 0) {
        pthread_cond_wait(&queue->reader_condition, &queue->mutex);
    }

    char* message = queue->messages[queue->front];
    queue->front = (queue->front + 1) % MAX_MESSAGES;
    queue->count--;

    pthread_cond_signal(&queue->writer_condition);
    pthread_mutex_unlock(&queue->mutex);

    return message;
}

// Writer thread function
void* writer(void* arg) {
    SharedQueue* queue = (SharedQueue*)arg;

    for (int i = 0; i < MAX_MESSAGES; i++) {
        char message[50];
        snprintf(message, sizeof(message), "Message %d", i);
        enqueue(queue, message);
        usleep(200000);  // Add messages every 200 milliseconds (5 messages/second)
    }

    return NULL;
}

// Reader thread function
void* reader(void* arg) {
    SharedQueue* queue = (SharedQueue*)arg;
    int id = *((int*)arg);

    while (1) {
        char* message = dequeue(queue);
        if (strcmp(message, "") == 0) {
            free(message);
            break;  // Exit when the writer is done
        }

        printf("Reader %d received: %s\n", id, message);
        free(message);
    }

    return NULL;
}

int main() {
    SharedQueue queue;
    initQueue(&queue);

    pthread_t writerThread;
    pthread_create(&writerThread, NULL, writer, &queue);

    pthread_t readerThreads[5];
    int readerIDs[5];

    for (int i = 0; i < 5; i++) {
        readerIDs[i] = i;
        pthread_create(&readerThreads[i], NULL, reader, &queue);
    }

    pthread_join(writerThread, NULL);

    // Signal readers to exit
    for (int i = 0; i < 5; i++) {
        enqueue(&queue, "");
    }

    for (int i = 0; i < 5; i++) {
        pthread_join(readerThreads[i], NULL);
    }

    pthread_mutex_destroy(&queue.mutex);
    pthread_cond_destroy(&queue.writer_condition);
    pthread_cond_destroy(&queue.reader_condition);

    return 0;
}


