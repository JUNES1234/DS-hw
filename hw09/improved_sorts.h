#ifndef IMPROVED_SORTS_H
#define IMPROVED_SORTS_H

#include "common.h"
#include "comparator_adapter.h"
#include "sorts.h"  // swap_student, cmp_wrap 재사용

// 1) 개선된 Shell 정렬 (예: Knuth gap: 1, 4, 13, 40, ...)
void shell_sort_knuth(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    int gap = 1;
    while (gap < n/3) {
        gap = 3*gap + 1;
    }
    while (gap >= 1) {
        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            stat->memory_ops++;
            int j = i;
            while (j >= gap && cmp_wrap(&arr[j-gap], &temp, cmp, ctx, stat) > 0) {
                arr[j] = arr[j-gap];
                stat->memory_ops++;
                j -= gap;
            }
            arr[j] = temp;
            stat->memory_ops++;
        }
        gap /= 3;
    }
}

// 2) 개선된 Quick 정렬 (median-of-three pivot)
int median_of_three(Student* arr, int a, int b, int c, CmpFunc cmp, void* ctx, Stat* stat) {
    // arr[a], arr[b], arr[c] 중 중앙값 index 반환
    if (cmp_wrap(&arr[a], &arr[b], cmp, ctx, stat) > 0)
        swap_student(&arr[a], &arr[b], stat);
    if (cmp_wrap(&arr[a], &arr[c], cmp, ctx, stat) > 0)
        swap_student(&arr[a], &arr[c], stat);
    if (cmp_wrap(&arr[b], &arr[c], cmp, ctx, stat) > 0)
        swap_student(&arr[b], &arr[c], stat);
    return b;
}

int partition_m3(Student* arr, int low, int high, CmpFunc cmp, void* ctx, Stat* stat) {
    int mid = low + (high - low) / 2;
    int m = median_of_three(arr, low, mid, high, cmp, ctx, stat);
    swap_student(&arr[m], &arr[high], stat); // pivot을 끝으로

    Student pivot = arr[high];
    stat->memory_ops++;

    int i = low - 1;
    for (int j = low; j < high; j++) {
        if (cmp_wrap(&arr[j], &pivot, cmp, ctx, stat) <= 0) {
            i++;
            swap_student(&arr[i], &arr[j], stat);
        }
    }
    swap_student(&arr[i+1], &arr[high], stat);
    return i + 1;
}

void quick_sort_m3_rec(Student* arr, int low, int high, CmpFunc cmp, void* ctx, Stat* stat) {
    if (low < high) {
        int pi = partition_m3(arr, low, high, cmp, ctx, stat);
        quick_sort_m3_rec(arr, low, pi - 1, cmp, ctx, stat);
        quick_sort_m3_rec(arr, pi + 1, high, cmp, ctx, stat);
    }
}

void quick_sort_m3(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    quick_sort_m3_rec(arr, 0, n-1, cmp, ctx, stat);
}

// 3) AVL 트리를 이용한 Tree 정렬
typedef struct AVLNode {
    Student key;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

int avl_height(AVLNode* node) {
    return node ? node->height : 0;
}

int max_int(int a, int b) { return a > b ? a : b; }

AVLNode* avl_new_node(const Student* s, Stat* stat) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    stat->memory_ops++;
    node->key = *s;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

AVLNode* right_rotate(AVLNode* y, Stat* stat) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;
    // 회전은 메모리 이동이 아니라고 보고 memory_ops는 생략해도 되지만,
    // 간단히 몇 회 증가시켜 둘 수도 있음
    stat->memory_ops += 2;

    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

AVLNode* left_rotate(AVLNode* x, Stat* stat) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;
    stat->memory_ops += 2;

    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

int get_balance(AVLNode* node) {
    if (!node) return 0;
    return avl_height(node->left) - avl_height(node->right);
}

AVLNode* avl_insert(
    AVLNode* node, const Student* key, CmpFunc cmp, void* ctx, Stat* stat
) {
    if (!node) return avl_new_node(key, stat);

    if (cmp_wrap(key, &node->key, cmp, ctx, stat) < 0)
        node->left = avl_insert(node->left, key, cmp, ctx, stat);
    else
        node->right = avl_insert(node->right, key, cmp, ctx, stat);

    node->height = 1 + max_int(avl_height(node->left), avl_height(node->right));

    int balance = get_balance(node);

    // LL
    if (balance > 1 &&
        cmp_wrap(key, &node->left->key, cmp, ctx, stat) < 0)
        return right_rotate(node, stat);
    // RR
    if (balance < -1 &&
        cmp_wrap(key, &node->right->key, cmp, ctx, stat) > 0)
        return left_rotate(node, stat);
    // LR
    if (balance > 1 &&
        cmp_wrap(key, &node->left->key, cmp, ctx, stat) > 0) {
        node->left = left_rotate(node->left, stat);
        return right_rotate(node, stat);
    }
    // RL
    if (balance < -1 &&
        cmp_wrap(key, &node->right->key, cmp, ctx, stat) < 0) {
        node->right = right_rotate(node->right, stat);
        return left_rotate(node, stat);
    }

    return node;
}

void avl_inorder(AVLNode* root, Student* arr, int* idx, Stat* stat) {
    if (!root) return;
    avl_inorder(root->left, arr, idx, stat);
    arr[*idx] = root->key;
    (*idx)++;
    stat->memory_ops++;
    avl_inorder(root->right, arr, idx, stat);
}

void avl_free(AVLNode* root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

void tree_sort_avl(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    AVLNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = avl_insert(root, &arr[i], cmp, ctx, stat);
    }
    int idx = 0;
    avl_inorder(root, arr, &idx, stat);
    avl_free(root);
}

#endif