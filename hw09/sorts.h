#ifndef SORTS_H
#define SORTS_H

#include "common.h"
#include "comparator_adapter.h"

// 비교 래퍼
static inline int cmp_wrap(const Student* a, const Student* b, CmpFunc cmp, void* ctx, Stat* stat) {
    stat->comparisons++;
    return cmp(a, b, ctx);
}

// swap
static inline void swap_student(Student* a, Student* b, Stat* stat) {
    Student tmp = *a;
    *a = *b;
    *b = tmp;
    stat->memory_ops += 3; // 단순히 3회 대입이라고 가정
}

// 1. 버블 정렬
void bubble_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (cmp_wrap(&arr[j], &arr[j+1], cmp, ctx, stat) > 0) {
                swap_student(&arr[j], &arr[j+1], stat);
            }
        }
    }
}

// 2. 선택 정렬
void selection_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    for (int i = 0; i < n - 1; i++) {
        int best = i;
        for (int j = i + 1; j < n; j++) {
            if (cmp_wrap(&arr[j], &arr[best], cmp, ctx, stat) < 0) {
                best = j;
            }
        }
        if (best != i) {
            swap_student(&arr[i], &arr[best], stat);
        }
    }
}

// 3. 삽입 정렬 (stable)
void insertion_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    for (int i = 1; i < n; i++) {
        Student key = arr[i];
        stat->memory_ops++; // key 복사
        int j = i - 1;
        while (j >= 0 && cmp_wrap(&arr[j], &key, cmp, ctx, stat) > 0) {
            arr[j+1] = arr[j];
            stat->memory_ops++;
            j--;
        }
        arr[j+1] = key;
        stat->memory_ops++;
    }
}

// 4. 셸 정렬 (기본 간격: n/2, n/4, ..., 1)
void shell_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    for (int gap = n/2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; i++) {
            Student temp = arr[i];
            stat->memory_ops++;
            int j = i;
            while (j >= gap && cmp_wrap(&arr[j - gap], &temp, cmp, ctx, stat) > 0) {
                arr[j] = arr[j - gap];
                stat->memory_ops++;
                j -= gap;
            }
            arr[j] = temp;
            stat->memory_ops++;
        }
    }
}

// 5. 퀵 정렬 (기본: 마지막 요소 pivot)
int partition(Student* arr, int low, int high, CmpFunc cmp, void* ctx, Stat* stat) {
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

void quick_sort_rec(Student* arr, int low, int high, CmpFunc cmp, void* ctx, Stat* stat) {
    if (low < high) {
        int pi = partition(arr, low, high, cmp, ctx, stat);
        quick_sort_rec(arr, low, pi - 1, cmp, ctx, stat);
        quick_sort_rec(arr, pi + 1, high, cmp, ctx, stat);
    }
}

void quick_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    quick_sort_rec(arr, 0, n - 1, cmp, ctx, stat);
}

// 6. 힙 정렬
void heapify(Student* arr, int n, int i, CmpFunc cmp, void* ctx, Stat* stat) {
    int largest = i;
    int l = 2*i + 1;
    int r = 2*i + 2;

    if (l < n && cmp_wrap(&arr[l], &arr[largest], cmp, ctx, stat) > 0)
        largest = l;
    if (r < n && cmp_wrap(&arr[r], &arr[largest], cmp, ctx, stat) > 0)
        largest = r;

    if (largest != i) {
        swap_student(&arr[i], &arr[largest], stat);
        heapify(arr, n, largest, cmp, ctx, stat);
    }
}

void heap_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    // max heap 구성
    for (int i = n / 2 - 1; i >= 0; i--) {
        heapify(arr, n, i, cmp, ctx, stat);
    }
    // 하나씩 꺼내기
    for (int i = n - 1; i > 0; i--) {
        swap_student(&arr[0], &arr[i], stat);
        heapify(arr, i, 0, cmp, ctx, stat);
    }
}

// 7. 병합 정렬 (stable)
void merge(Student* arr, int l, int m, int r, CmpFunc cmp, void* ctx, Stat* stat) {
    int n1 = m - l + 1;
    int n2 = r - m;
    Student* L = (Student*)malloc(sizeof(Student) * n1);
    Student* R = (Student*)malloc(sizeof(Student) * n2);
    stat->memory_ops += n1 + n2;

    for (int i = 0; i < n1; i++) L[i] = arr[l + i];
    for (int j = 0; j < n2; j++) R[j] = arr[m + 1 + j];

    int i = 0, j = 0, k = l;

    while (i < n1 && j < n2) {
        if (cmp_wrap(&L[i], &R[j], cmp, ctx, stat) <= 0) {
            arr[k++] = L[i++];
        } else {
            arr[k++] = R[j++];
        }
        stat->memory_ops++;
    }

    while (i < n1) {
        arr[k++] = L[i++];
        stat->memory_ops++;
    }
    while (j < n2) {
        arr[k++] = R[j++];
        stat->memory_ops++;
    }

    free(L);
    free(R);
}

void merge_sort_rec(Student* arr, int l, int r, CmpFunc cmp, void* ctx, Stat* stat) {
    if (l < r) {
        int m = l + (r - l)/2;
        merge_sort_rec(arr, l, m, cmp, ctx, stat);
        merge_sort_rec(arr, m+1, r, cmp, ctx, stat);
        merge(arr, l, m, r, cmp, ctx, stat);
    }
}

void merge_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    merge_sort_rec(arr, 0, n-1, cmp, ctx, stat);
}

// 8. 기수 정렬 (ID 기준이라고 가정, 음수 ID 없다고 가정)
int get_max_id(Student* arr, int n) {
    int mx = arr[0].id;
    for (int i = 1; i < n; i++)
        if (arr[i].id > mx) mx = arr[i].id;
    return mx;
}

void counting_sort_by_exp(Student* arr, int n, int exp, Stat* stat) {
    Student* output = (Student*)malloc(sizeof(Student) * n);
    int count[10] = {0};

    for (int i = 0; i < n; i++) {
        int idx = (arr[i].id / exp) % 10;
        count[idx]++;
        stat->memory_ops++;
    }
    for (int i = 1; i < 10; i++)
        count[i] += count[i-1];

    for (int i = n - 1; i >= 0; i--) {
        int idx = (arr[i].id / exp) % 10;
        output[--count[idx]] = arr[i];
        stat->memory_ops++;
    }
    for (int i = 0; i < n; i++) {
        arr[i] = output[i];
        stat->memory_ops++;
    }
    free(output);
}

void radix_sort_by_id(Student* arr, int n, Stat* stat) {
    int m = get_max_id(arr, n);
    for (int exp = 1; m / exp > 0; exp *= 10) {
        counting_sort_by_exp(arr, n, exp, stat);
    }
    // 기수 정렬은 오름차순만 쉽게 처리. 내림차순은 뒤집기 추가 필요.
}

// 9. 트리 정렬 (간단한 BST 사용, 중복 시 오른쪽에 넣기)
typedef struct TreeNode {
    Student key;
    struct TreeNode* left;
    struct TreeNode* right;
} TreeNode;

TreeNode* new_node(const Student* s, Stat* stat) {
    TreeNode* node = (TreeNode*)malloc(sizeof(TreeNode));
    stat->memory_ops++;
    node->key = *s;
    node->left = node->right = NULL;
    return node;
}

TreeNode* bst_insert(TreeNode* root, const Student* key, CmpFunc cmp, void* ctx, Stat* stat) {
    if (!root) return new_node(key, stat);
    if (cmp_wrap(key, &root->key, cmp, ctx, stat) < 0)
        root->left = bst_insert(root->left, key, cmp, ctx, stat);
    else
        root->right = bst_insert(root->right, key, cmp, ctx, stat);
    return root;
}

void inorder_traverse(TreeNode* root, Student* arr, int* idx, Stat* stat) {
    if (!root) return;
    inorder_traverse(root->left, arr, idx, stat);
    arr[(*idx)++] = root->key;
    stat->memory_ops++;
    inorder_traverse(root->right, arr, idx, stat);
}

void free_tree(TreeNode* root) {
    if (!root) return;
    free_tree(root->left);
    free_tree(root->right);
    free(root);
}

void tree_sort(Student* arr, int n, CmpFunc cmp, void* ctx, Stat* stat) {
    TreeNode* root = NULL;
    for (int i = 0; i < n; i++) {
        root = bst_insert(root, &arr[i], cmp, ctx, stat);
    }
    int idx = 0;
    inorder_traverse(root, arr, &idx, stat);
    free_tree(root);
}

#endif