#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_NAME_LEN 50
#define MAX_LINE_LEN 200

typedef struct {
    int id;
    char name[MAX_NAME_LEN];
    char gender;
    int korean;
    int english;
    int math;
} Student;

typedef long long ll;

typedef struct {
    ll comparisons;
} Counter;

// ---------------- 데이터 로드 ----------------
Student* load_students(const char* filename, int* out_count) {
    FILE* fp = fopen(filename, "r");
    if (!fp) {
        perror("Failed to open file");
        return NULL;
    }

    char line[MAX_LINE_LEN];
    int capacity = 10;
    int count = 0;
    Student* arr = malloc(sizeof(Student) * capacity);
    if (!arr) {
        perror("Memory allocation failed");
        fclose(fp);
        return NULL;
    }

    // 첫 줄 헤더 스킵
    if (!fgets(line, sizeof(line), fp)) {
        fclose(fp);
        free(arr);
        *out_count = 0;
        return NULL;
    }

    while (fgets(line, sizeof(line), fp)) {
        if (count >= capacity) {
            capacity *= 2;
            Student* temp = realloc(arr, sizeof(Student) * capacity);
            if (!temp) {
                perror("Reallocation failed");
                free(arr);
                fclose(fp);
                return NULL;
            }
            arr = temp;
        }

        Student s;
        char* token = strtok(line, ",");
        if (!token) continue;
        s.id = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        strncpy(s.name, token, MAX_NAME_LEN);
        s.name[MAX_NAME_LEN - 1] = '\0';

        token = strtok(NULL, ",");
        if (!token) continue;
        s.gender = token[0];

        token = strtok(NULL, ",");
        if (!token) continue;
        s.korean = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.english = atoi(token);

        token = strtok(NULL, ",");
        if (!token) continue;
        s.math = atoi(token);

        arr[count++] = s;
    }

    fclose(fp);

    Student* tight = realloc(arr, sizeof(Student) * count);
    if (!tight) {
        fprintf(stderr, "Warning: Tight reallocation failed, using original memory.\n");
        *out_count = count;
        return arr;
    }

    *out_count = count;
    return tight;
}

// ---------------- 공통 유틸 ----------------
void print_student(const Student* s) {
    if (!s) {
        printf("(null)\n");
        return;
    }
    printf("ID=%d, Name=%s, Gender=%c, K=%d, E=%d, M=%d\n",
           s->id, s->name, s->gender, s->korean, s->english, s->math);
}

void copy_students(Student* dst, const Student* src, int n) {
    memcpy(dst, src, sizeof(Student) * n);
}

// ID 기준 비교 (카운터 포함)
int cmp_id_counted(int a, int b, Counter* c) {
    c->comparisons++;
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// ---------------- 1) 비정렬 배열: 순차 탐색, 삽입, 삭제 ----------------

// 순차 탐색: 찾으면 인덱스, 없으면 -1
int seq_search_by_id(Student* arr, int n, int target_id, Counter* c) {
    for (int i = 0; i < n; i++) {
        if (cmp_id_counted(arr[i].id, target_id, c) == 0) {
            return i;
        }
    }
    return -1;
}

// 비정렬 배열에 삽입: 단순히 뒤에 붙임 (capacity 여유 있다고 가정)
void unsorted_array_insert(Student* arr, int* n, Student item) {
    arr[*n] = item;
    (*n)++;
}

// 비정렬 배열에서 삭제: 인덱스를 찾아서 마지막 요소와 교체 후 크기 줄이기
// (순서를 유지하지 않는 간단한 삭제)
int unsorted_array_delete_by_id(Student* arr, int* n, int target_id, Counter* c) {
    int idx = seq_search_by_id(arr, *n, target_id, c);
    if (idx == -1) return 0; // 실패

    arr[idx] = arr[*n - 1];
    (*n)--;
    return 1; // 성공
}

// ---------------- 2) 정렬 배열: 정렬, 이진 탐색, 삽입, 삭제 ----------------

// 정렬을 위해 단순 퀵소트 (ID 기준, 비교 카운트 필요 없음)
int cmp_id_simple(const void* p1, const void* p2) {
    const Student* a = (const Student*)p1;
    const Student* b = (const Student*)p2;
    if (a->id < b->id) return -1;
    if (a->id > b->id) return 1;
    return 0;
}

// 이진 탐색: 찾으면 인덱스, 없으면 -1
int binary_search_by_id(Student* arr, int n, int target_id, Counter* c) {
    int left = 0, right = n - 1;
    while (left <= right) {
        int mid = left + (right - left) / 2;

        int r = cmp_id_counted(arr[mid].id, target_id, c);
        if (r == 0) return mid;
        else if (r < 0) left = mid + 1;
        else right = mid - 1;
    }
    return -1;
}

// 정렬 배열에 삽입: 적절한 위치를 찾아 한 칸씩 밀기 (ID 기준 유지)
void sorted_array_insert(Student* arr, int* n, Student item) {
    int i = *n - 1;
    while (i >= 0 && arr[i].id > item.id) {
        arr[i+1] = arr[i];
        i--;
    }
    arr[i+1] = item;
    (*n)++;
}

// 정렬 배열에서 삭제: 이진 탐색으로 위치 찾고, 뒤 요소들을 한 칸씩 당김
int sorted_array_delete_by_id(Student* arr, int* n, int target_id, Counter* c) {
    int idx = binary_search_by_id(arr, *n, target_id, c);
    if (idx == -1) return 0;

    for (int i = idx; i < *n - 1; i++) {
        arr[i] = arr[i+1];
    }
    (*n)--;
    return 1;
}

// ---------------- 3) AVL Tree: 삽입, 삭제, 검색 ----------------
typedef struct AVLNode {
    Student key;
    struct AVLNode* left;
    struct AVLNode* right;
    int height;
} AVLNode;

int avl_height(AVLNode* node) {
    return node ? node->height : 0;
}

int max_int(int a, int b) {
    return a > b ? a : b;
}

AVLNode* avl_new_node(const Student* s) {
    AVLNode* node = (AVLNode*)malloc(sizeof(AVLNode));
    node->key = *s;
    node->left = node->right = NULL;
    node->height = 1;
    return node;
}

// 회전
AVLNode* right_rotate(AVLNode* y) {
    AVLNode* x = y->left;
    AVLNode* T2 = x->right;

    x->right = y;
    y->left = T2;

    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    return x;
}

AVLNode* left_rotate(AVLNode* x) {
    AVLNode* y = x->right;
    AVLNode* T2 = y->left;

    y->left = x;
    x->right = T2;

    x->height = max_int(avl_height(x->left), avl_height(x->right)) + 1;
    y->height = max_int(avl_height(y->left), avl_height(y->right)) + 1;
    return y;
}

int get_balance(AVLNode* node) {
    if (!node) return 0;
    return avl_height(node->left) - avl_height(node->right);
}

// AVL 삽입 (ID 기준)
AVLNode* avl_insert(AVLNode* node, const Student* key, Counter* c) {
    if (!node) return avl_new_node(key);

    // 비교 카운트 포함
    int r = cmp_id_counted(key->id, node->key.id, c);
    if (r < 0)
        node->left = avl_insert(node->left, key, c);
    else if (r > 0)
        node->right = avl_insert(node->right, key, c);
    else
        return node; // 같은 ID라면 삽입 안 함(또는 덮어쓰기 정책도 가능)

    node->height = 1 + max_int(avl_height(node->left), avl_height(node->right));

    int balance = get_balance(node);

    // LL
    if (balance > 1 && key->id < node->left->key.id)
        return right_rotate(node);

    // RR
    if (balance < -1 && key->id > node->right->key.id)
        return left_rotate(node);

    // LR
    if (balance > 1 && key->id > node->left->key.id) {
        node->left = left_rotate(node->left);
        return right_rotate(node);
    }

    // RL
    if (balance < -1 && key->id < node->right->key.id) {
        node->right = right_rotate(node->right);
        return left_rotate(node);
    }

    return node;
}

// 최소값 노드(삭제 시 사용)
AVLNode* avl_min_value_node(AVLNode* node) {
    AVLNode* current = node;
    while (current->left)
        current = current->left;
    return current;
}

// AVL 삭제 (ID 기준)
AVLNode* avl_delete(AVLNode* root, int key_id, Counter* c) {
    if (!root) return root;

    int r = cmp_id_counted(key_id, root->key.id, c);
    if (r < 0) {
        root->left = avl_delete(root->left, key_id, c);
    } else if (r > 0) {
        root->right = avl_delete(root->right, key_id, c);
    } else {
        // 삭제 대상 노드 발견
        if (!root->left || !root->right) {
            AVLNode* temp = root->left ? root->left : root->right;
            if (!temp) { // 자식 없음
                temp = root;
                root = NULL;
            } else {     // 자식 1개
                *root = *temp;
            }
            free(temp);
        } else {
            // 자식 2개: 오른쪽 서브트리의 최소값으로 교체
            AVLNode* temp = avl_min_value_node(root->right);
            root->key = temp->key;
            root->right = avl_delete(root->right, temp->key.id, c);
        }
    }

    if (!root) return root;

    root->height = 1 + max_int(avl_height(root->left), avl_height(root->right));
    int balance = get_balance(root);

    // LL
    if (balance > 1 && get_balance(root->left) >= 0)
        return right_rotate(root);

    // LR
    if (balance > 1 && get_balance(root->left) < 0) {
        root->left = left_rotate(root->left);
        return right_rotate(root);
    }

    // RR
    if (balance < -1 && get_balance(root->right) <= 0)
        return left_rotate(root);

    // RL
    if (balance < -1 && get_balance(root->right) > 0) {
        root->right = right_rotate(root->right);
        return left_rotate(root);
    }

    return root;
}

// AVL 검색
AVLNode* avl_search(AVLNode* root, int key_id, Counter* c) {
    if (!root) return NULL;

    int r = cmp_id_counted(key_id, root->key.id, c);
    if (r == 0) return root;
    else if (r < 0) return avl_search(root->left, key_id, c);
    else return avl_search(root->right, key_id, c);
}

void avl_free(AVLNode* root) {
    if (!root) return;
    avl_free(root->left);
    avl_free(root->right);
    free(root);
}

// ---------------- main: 각 구조에 대해 삽입/삭제/검색 + 비교 횟수 출력 ----------------
int main(void) {
    int n;
    Student* arr_original = load_students("dataset_id_ascending.csv", &n);
    if (!arr_original) {
        fprintf(stderr, "Failed to load students.\n");
        return 1;
    }
    printf("Loaded %d students from dataset_id_ascending.csv\n", n);

    // 1) 비정렬 배열: 원본 그대로 사용
    int n_unsorted = n + 10;               // 삽입 여유분
    Student* arr_unsorted = malloc(sizeof(Student) * n_unsorted);
    copy_students(arr_unsorted, arr_original, n);
    int len_unsorted = n;                  // 현재 실제 원소 수

    // 2) 정렬 배열: ID 기준 오름차순
    int n_sorted = n + 10;
    Student* arr_sorted = malloc(sizeof(Student) * n_sorted);
    copy_students(arr_sorted, arr_original, n);
    int len_sorted = n;
    qsort(arr_sorted, len_sorted, sizeof(Student), cmp_id_simple);

    // 3) AVL Tree: 원본 데이터 모두 삽입
    AVLNode* root = NULL;
    Counter c_insert_all = {0};
    for (int i = 0; i < n; i++) {
        root = avl_insert(root, &arr_original[i], &c_insert_all);
    }
    printf("\n[AVL] Bulk insert of %d items: comparisons = %lld\n",
           n, c_insert_all.comparisons);

    // 테스트용 새 학생 (삽입용)
    Student new_student = {9999, "New Student", 'M', 100, 100, 100};

    // 테스트용 삭제/검색 target ID (데이터에 실제로 있는 첫 학생 ID 사용)
    int existing_id = arr_original[0].id;
    int non_existing_id = 123456789;   // (존재 안 할 확률 높음)

    // ---------------- 비정렬 배열 테스트 ----------------
    printf("\n=== Unsorted Array (Sequential Search) ===\n");

    // 삽입 (비교는 없음: 그냥 뒤에 붙이기)
    printf("Insert (unsorted): ID=%d\n", new_student.id);
    unsorted_array_insert(arr_unsorted, &len_unsorted, new_student);
    printf("Unsorted length after insert = %d\n", len_unsorted);

    // 검색 - 존재하는 ID
    Counter c_seq_exist = {0};
    int idx = seq_search_by_id(arr_unsorted, len_unsorted, existing_id, &c_seq_exist);
    printf("Seq search existing ID=%d -> index=%d, comparisons=%lld\n",
           existing_id, idx, c_seq_exist.comparisons);

    // 검색 - 존재하지 않는 ID
    Counter c_seq_not = {0};
    idx = seq_search_by_id(arr_unsorted, len_unsorted, non_existing_id, &c_seq_not);
    printf("Seq search non-existing ID=%d -> index=%d, comparisons=%lld\n",
           non_existing_id, idx, c_seq_not.comparisons);

    // 삭제 - 존재하는 ID
    Counter c_seq_del = {0};
    int ok = unsorted_array_delete_by_id(arr_unsorted, &len_unsorted, existing_id, &c_seq_del);
    printf("Delete existing ID=%d -> %s, comparisons=%lld, length=%d\n",
           existing_id, ok ? "SUCCESS" : "FAIL", c_seq_del.comparisons, len_unsorted);

    // ---------------- 정렬 배열 테스트 ----------------
    printf("\n=== Sorted Array (Binary Search) ===\n");

    // 삽입 (정렬 유지)
    printf("Insert (sorted): ID=%d\n", new_student.id);
    sorted_array_insert(arr_sorted, &len_sorted, new_student);
    printf("Sorted length after insert = %d\n", len_sorted);

    // 검색 - 존재하는 ID
    Counter c_bin_exist = {0};
    idx = binary_search_by_id(arr_sorted, len_sorted, existing_id, &c_bin_exist);
    printf("Binary search existing ID=%d -> index=%d, comparisons=%lld\n",
           existing_id, idx, c_bin_exist.comparisons);

    // 검색 - 존재하지 않는 ID
    Counter c_bin_not = {0};
    idx = binary_search_by_id(arr_sorted, len_sorted, non_existing_id, &c_bin_not);
    printf("Binary search non-existing ID=%d -> index=%d, comparisons=%lld\n",
           non_existing_id, idx, c_bin_not.comparisons);

    // 삭제 - 존재하는 ID
    Counter c_bin_del = {0};
    ok = sorted_array_delete_by_id(arr_sorted, &len_sorted, existing_id, &c_bin_del);
    printf("Delete existing ID=%d (sorted) -> %s, comparisons=%lld, length=%d\n",
           existing_id, ok ? "SUCCESS" : "FAIL", c_bin_del.comparisons, len_sorted);

    // ---------------- AVL Tree 테스트 ----------------
    printf("\n=== AVL Tree ===\n");

    // 삽입
    Counter c_avl_ins = {0};
    root = avl_insert(root, &new_student, &c_avl_ins);
    printf("AVL insert ID=%d -> comparisons=%lld\n", new_student.id, c_avl_ins.comparisons);

    // 검색 - 존재하는 ID
    Counter c_avl_exist = {0};
    AVLNode* node = avl_search(root, existing_id, &c_avl_exist);
    printf("AVL search existing ID=%d -> found=%s, comparisons=%lld\n",
           existing_id, node ? "YES" : "NO", c_avl_exist.comparisons);

    // 검색 - 존재하지 않는 ID
    Counter c_avl_not = {0};
    node = avl_search(root, non_existing_id, &c_avl_not);
    printf("AVL search non-existing ID=%d -> found=%s, comparisons=%lld\n",
           non_existing_id, node ? "YES" : "NO", c_avl_not.comparisons);

    // 삭제 - 존재하는 ID
    Counter c_avl_del = {0};
    root = avl_delete(root, existing_id, &c_avl_del);
    printf("AVL delete existing ID=%d -> comparisons=%lld\n",
           existing_id, c_avl_del.comparisons);

    // ---------------- 정리 ----------------
    avl_free(root);
    free(arr_original);
    free(arr_unsorted);
    free(arr_sorted);

    return 0;
}