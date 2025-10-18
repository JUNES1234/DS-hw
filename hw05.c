#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

// AVL 트리 노드 구조체
typedef struct AVLNode {
    int data;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

// BST 노드 구조체
typedef struct BSTNode {
    int data;
    struct BSTNode* left;
    struct BSTNode* right;
} BSTNode;

// 전역 변수 - 탐색 횟수 카운터
int searchCount;

// ==================== 유틸리티 함수 ====================
int max(int a, int b) {
    return (a > b) ? a : b;
}

// 배열에서 중복 체크
bool isDuplicate(int arr[], int size, int value) {
    for (int i = 0; i < size; i++) {
        if (arr[i] == value) return true;
    }
    return false;
}

// Fisher-Yates 셔플 알고리즘
void shuffle(int arr[], int size) {
    for (int i = size - 1; i > 0; i--) {
        int j = rand() % (i + 1);
        int temp = arr[i];
        arr[i] = arr[j];
        arr[j] = temp;
    }
}

// ==================== 배열 탐색 함수 ====================
bool linearSearch(int arr[], int size, int key) {
    searchCount = 0;
    for (int i = 0; i < size; i++) {
        searchCount++;
        if (arr[i] == key) {
            return true;
        }
    }
    return false;
}

// ==================== BST 함수들 ====================
BSTNode* createBSTNode(int data) {
    BSTNode* node = (BSTNode*)malloc(sizeof(BSTNode));
    node->data = data;
    node->left = node->right = NULL;
    return node;
}

BSTNode* insertBST(BSTNode* root, int data) {
    if (root == NULL) {
        return createBSTNode(data);
    }
    
    if (data < root->data) {
        root->left = insertBST(root->left, data);
    } else if (data > root->data) {
        root->right = insertBST(root->right, data);
    }
    
    return root;
}

bool searchBST(BSTNode* root, int key) {
    searchCount = 0;
    BSTNode* current = root;
    
    while (current != NULL) {
        searchCount++;
        if (key == current->data) {
            return true;
        } else if (key < current->data) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return false;
}

void freeBST(BSTNode* root) {
    if (root != NULL) {
        freeBST(root->left);
        freeBST(root->right);
        free(root);
    }
}

// ==================== AVL 트리 함수들 ====================
int getHeight(AVLNode* node) {
    if (node == NULL) return 0;
    return node->height;
}

int getBalance(AVLNode* node) {
    if (node == NULL) return 0;
    return getHeight(node->left) - getHeight(node->right);
}

AVLNode* createAVLNode(int data) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->data = data;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* rotateRight(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;
    
    x->right = y;
    y->left = T2;
    
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    
    return x;
}

AVLNode* rotateLeft(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;
    
    y->left = x;
    x->right = T2;
    
    x->height = max(getHeight(x->left), getHeight(x->right)) + 1;
    y->height = max(getHeight(y->left), getHeight(y->right)) + 1;
    
    return y;
}

AVLNode* insertAVL(AVLNode* root, int data) {
    if (root == NULL) {
        return createAVLNode(data);
    }
    
    if (data < root->data) {
        root->left = insertAVL(root->left, data);
    } else if (data > root->data) {
        root->right = insertAVL(root->right, data);
    } else {
        return root;
    }
    
    root->height = 1 + max(getHeight(root->left), getHeight(root->right));
    
    int balance = getBalance(root);
    
    // Left Left Case
    if (balance > 1 && data < root->left->data) {
        return rotateRight(root);
    }
    
    // Right Right Case
    if (balance < -1 && data > root->right->data) {
        return rotateLeft(root);
    }
    
    // Left Right Case
    if (balance > 1 && data > root->left->data) {
        root->left = rotateLeft(root->left);
        return rotateRight(root);
    }
    
    // Right Left Case
    if (balance < -1 && data < root->right->data) {
        root->right = rotateRight(root->right);
        return rotateLeft(root);
    }
    
    return root;
}

bool searchAVL(AVLNode* root, int key) {
    searchCount = 0;
    AVLNode* current = root;
    
    while (current != NULL) {
        searchCount++;
        if (key == current->data) {
            return true;
        } else if (key < current->data) {
            current = current->left;
        } else {
            current = current->right;
        }
    }
    
    return false;
}

void freeAVL(AVLNode* root) {
    if (root != NULL) {
        freeAVL(root->left);
        freeAVL(root->right);
        free(root);
    }
}

// ==================== 데이터 생성 함수들 ====================
void generateRandomData(int data[], int size) {
    int count = 0;
    while (count < size) {
        int num = rand() % 10001;
        if (!isDuplicate(data, count, num)) {
            data[count++] = num;
        }
    }
}

void generateAscendingData(int data[], int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i;
    }
}

void generateDescendingData(int data[], int size) {
    for (int i = 0; i < size; i++) {
        data[i] = size - 1 - i;
    }
}

void generateSpecialData(int data[], int size) {
    for (int i = 0; i < size; i++) {
        data[i] = i * (i % 2 + 2);
    }
}

// ==================== 테스트 함수 ====================
void testDataSet(int data[], int size, const char* datasetName) {
    printf("\n========== %s ==========\n", datasetName);
    
    // 배열 복사 (원본 보존)
    int* arrayData = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        arrayData[i] = data[i];
    }
    
    // BST 생성
    BSTNode* bstRoot = NULL;
    for (int i = 0; i < size; i++) {
        bstRoot = insertBST(bstRoot, data[i]);
    }
    
    // AVL 트리 생성
    AVLNode* avlRoot = NULL;
    for (int i = 0; i < size; i++) {
        avlRoot = insertAVL(avlRoot, data[i]);
    }
    
    // 탐색할 1000개의 난수 생성
    int searchKeys[1000];
    for (int i = 0; i < 1000; i++) {
        searchKeys[i] = rand() % 10001;
    }
    
    // 배열 탐색 테스트
    long long totalArraySearch = 0;
    for (int i = 0; i < 1000; i++) {
        linearSearch(arrayData, size, searchKeys[i]);
        totalArraySearch += searchCount;
    }
    
    // BST 탐색 테스트
    long long totalBSTSearch = 0;
    for (int i = 0; i < 1000; i++) {
        searchBST(bstRoot, searchKeys[i]);
        totalBSTSearch += searchCount;
    }
    
    // AVL 탐색 테스트
    long long totalAVLSearch = 0;
    for (int i = 0; i < 1000; i++) {
        searchAVL(avlRoot, searchKeys[i]);
        totalAVLSearch += searchCount;
    }
    
    // 결과 출력
    printf("배열(선형탐색) 평균 탐색 횟수: %.2f\n", totalArraySearch / 1000.0);
    printf("BST 평균 탐색 횟수: %.2f\n", totalBSTSearch / 1000.0);
    printf("AVL 평균 탐색 횟수: %.2f\n", totalAVLSearch / 1000.0);
    
    // 메모리 해제
    free(arrayData);
    freeBST(bstRoot);
    freeAVL(avlRoot);
}

// ==================== 메인 함수 ====================
int main() {
    srand(time(NULL));
    
    int data[1000];
    
    // (1) 무작위 데이터
    generateRandomData(data, 1000);
    testDataSet(data, 1000, "무작위 데이터 (0~10000, 중복 없음)");
    
    // (2) 오름차순 정렬 데이터
    generateAscendingData(data, 1000);
    testDataSet(data, 1000, "오름차순 정렬 데이터 (0~999)");
    
    // (3) 내림차순 정렬 데이터
    generateDescendingData(data, 1000);
    testDataSet(data, 1000, "내림차순 정렬 데이터 (999~0)");
    
    // (4) 특수 패턴 데이터
    generateSpecialData(data, 1000);
    testDataSet(data, 1000, "특수 패턴 데이터 (i * (i%2 + 2))");
    
    return 0;
}
