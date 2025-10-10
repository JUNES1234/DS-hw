#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <windows.h>

#define ARRAY_SIZE 100
#define MAX_VALUE 1000
#define REPEAT_COUNT 1000000  // 100만 번 반복

typedef struct Node {
    int key;
    struct Node *left, *right;
} Node;

Node* createNode(int key) {
    Node* newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }
    newNode->key = key;
    newNode->left = newNode->right = NULL;
    return newNode;
}

Node* insertBST(Node* root, int key) {
    if (root == NULL)
        return createNode(key);

    if (key < root->key)
        root->left = insertBST(root->left, key);
    else
        root->right = insertBST(root->right, key);

    return root;
}

int linearSearch(int arr[], int size, int target, int *count) {
    *count = 0;
    for (int i = 0; i < size; i++) {
        (*count)++;
        if (arr[i] == target)
            return i;
    }
    return -1;
}

Node* searchBST(Node* root, int key, int* count) {
    *count = 0;
    Node* cur = root;
    while (cur != NULL) {
        (*count)++;
        if (key == cur->key)
            return cur;
        else if (key < cur->key)
            cur = cur->left;
        else
            cur = cur->right;
    }
    return NULL;
}

void freeBST(Node* root) {
    if (root == NULL) return;
    freeBST(root->left);
    freeBST(root->right);
    free(root);
}

// 고해상도 타이머 함수 (Windows)
double get_time_sec() {
    static LARGE_INTEGER frequency;
    static int initialized = 0;
    if (!initialized) {
        QueryPerformanceFrequency(&frequency);
        initialized = 1;
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    return (double)(counter.QuadPart) / frequency.QuadPart;
}

int main() {
    int data[ARRAY_SIZE];
    Node* root = NULL;

    srand((unsigned)time(NULL));

    // 무작위 데이터 생성 및 BST 구축
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % (MAX_VALUE + 1);
    }
    for (int i = 0; i < ARRAY_SIZE; i++) {
        root = insertBST(root, data[i]);
    }

    int targetIndex = rand() % ARRAY_SIZE;
    int target = data[targetIndex];

    int linearCount = 0, bstCount = 0;
    int linearIndex = -1;
    Node* foundNode = NULL;

    double startTime, endTime;
    double linearTotalTime = 0.0, bstTotalTime = 0.0;

    // 반복 수행하며 선형 탐색 시간 측정
    startTime = get_time_sec();
    for (int i = 0; i < REPEAT_COUNT; i++) {
        linearIndex = linearSearch(data, ARRAY_SIZE, target, &linearCount);
    }
    endTime = get_time_sec();
    linearTotalTime = endTime - startTime;

    // 반복 수행하며 BST 탐색 시간 측정
    startTime = get_time_sec();
    for (int i = 0; i < REPEAT_COUNT; i++) {
        foundNode = searchBST(root, target, &bstCount);
    }
    endTime = get_time_sec();
    bstTotalTime = endTime - startTime;

    // 결과 출력
    printf("Target number to search: %d\n\n", target);

    printf("[Linear Search]\n");
    if (linearIndex != -1)
        printf(" Last search array index: %d\n", linearIndex);
    else
        printf(" Value not found\n");
    printf(" Number of comparisons in last search: %d\n", linearCount);
    printf(" Total time for %d searches: %.9f seconds\n", REPEAT_COUNT, linearTotalTime);
    printf(" Average time per search: %.12f seconds\n\n", linearTotalTime / REPEAT_COUNT);

    printf("[BST Search]\n");
    if (foundNode != NULL)
        printf(" Node found\n");
    else
        printf(" Value not found\n");
    printf(" Number of comparisons in last search: %d\n", bstCount);
    printf(" Total time for %d searches: %.9f seconds\n", REPEAT_COUNT, bstTotalTime);
    printf(" Average time per search: %.12f seconds\n", bstTotalTime / REPEAT_COUNT);

    freeBST(root);
    return 0;
}
