#include <stdio.h>
#include <stdlib.h>

// Define a structure for a node in the queue
typedef struct Node {
    int data;
    struct Node* next;
} Node;

// Define a structure for the circular queue
typedef struct Queue {
    Node* rear;
} Queue;

// Function to create a new node with given data
Node* createNode(int data) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (newNode == NULL) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    newNode->data = data;
    newNode->next = NULL;
    return newNode;
}

// Function to initialize the circular queue
Queue* initializeQueue() {
    Queue* queue = (Queue*)malloc(sizeof(Queue));
    if (queue == NULL) {
        printf("Memory allocation failed!\n");
        exit(EXIT_FAILURE);
    }
    queue->rear = NULL;
    return queue;
}

// Function to check if the queue is empty
int isEmpty(Queue* queue) {
    return queue->rear == NULL;
}

// Function to enqueue a new element into the circular queue
void enqueue(Queue* queue, int data) {
    Node* newNode = createNode(data);
    if (isEmpty(queue)) {
        queue->rear = newNode;
        newNode->next = newNode; // Circular link
    } else {
        newNode->next = queue->rear->next;
        queue->rear->next = newNode;
        queue->rear = newNode;
    }
}

// Function to dequeue an element from the circular queue
int dequeue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty!\n");
        exit(EXIT_FAILURE);
    }
    int data;
    if (queue->rear->next == queue->rear) { // Only one node in the queue
        data = queue->rear->data;
        free(queue->rear);
        queue->rear = NULL;
    } else {
        Node* front = queue->rear->next;
        data = front->data;
        queue->rear->next = front->next;
        free(front);
    }
    return data;
}

// Function to print the circular queue
void printQueue(Queue* queue) {
    if (isEmpty(queue)) {
        printf("Queue is empty!\n");
        return;
    }
    printf("Queue: ");
    Node* current = queue->rear->next;
    do {
        printf("%d ", current->data);
        current = current->next;
    } while (current != queue->rear->next);
    printf("\n");
}

// Function to free memory allocated for the circular queue
void freeQueue(Queue* queue) {
    if (!isEmpty(queue)) {
        Node* current = queue->rear->next;
        do {
            Node* temp = current;
            current = current->next;
            free(temp);
        } while (current != queue->rear->next);
    }
    free(queue);
}

int main() {
    // Initialize the circular queue
    Queue* queue = initializeQueue();

    // Enqueue elements into the circular queue
    enqueue(queue, 10);
    enqueue(queue, 20);
    enqueue(queue, 30);

    // Print the circular queue
    printQueue(queue);

    // Dequeue elements from the circular queue
    printf("Dequeued element: %d\n", dequeue(queue));
    printf("Dequeued element: %d\n", dequeue(queue));

    // Print the circular queue after dequeue
    printQueue(queue);

    // Free memory allocated for the circular queue
    freeQueue(queue);

    return 0;
}
