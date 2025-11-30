#include "common.h"
#include "compare.h"
#include "comparator_adapter.h"
#include "sorts.h"

#define REPEAT 1000

// 배열 복사
void copy_students(Student* dst, const Student* src, int n) {
    memcpy(dst, src, sizeof(Student) * n);
}

// 해당 기준에서 key 중복 여부 체크 (간단 버전)
int has_duplicate_key(Student* arr, int n, CmpCtx ctx) {
    // arr를 임시로 하나 정렬해서 인접 비교
    Student* tmp = (Student*)malloc(sizeof(Student) * n);
    memcpy(tmp, arr, sizeof(Student) * n);

    Stat dummy = {0,0};
    CmpFunc gcmp = generic_cmp;
    // stable한 병합정렬로 정렬
    merge_sort(tmp, n, gcmp, &ctx, &dummy);

    int dup = 0;
    for (int i = 1; i < n; i++) {
        if (generic_cmp(&tmp[i-1], &tmp[i], &ctx) == 0) {
            dup = 1;
            break;
        }
    }
    free(tmp);
    return dup;
}

void run_one_sort(
    const char* sort_name,
    void (*sort_fn)(Student*, int, CmpFunc, void*, Stat*),
    Student* base, int n, CmpCtx ctx
) {
    CmpFunc gcmp = generic_cmp;
    Stat total = {0,0};

    Student* work = (Student*)malloc(sizeof(Student) * n);

    for (int r = 0; r < REPEAT; r++) {
        copy_students(work, base, n);
        Stat s = {0,0};
        sort_fn(work, n, gcmp, &ctx, &s);
        total.comparisons += s.comparisons;
        total.memory_ops += s.memory_ops;
    }

    free(work);

    double avg_cmp = (double)total.comparisons / REPEAT;
    double avg_mem = (double)total.memory_ops / REPEAT;

    printf("[%s] mode=%d, order=%s -> avg comparisons=%.2f, avg mem_ops=%.2f\n",
           sort_name,
           ctx.mode,
           (ctx.ord == ASC ? "ASC" : "DESC"),
           avg_cmp,
           avg_mem);
}

int main(void) {
    int n;
    Student* arr = load_students("dataset_id_ascending.csv", &n);
    if (!arr) {
        fprintf(stderr, "Failed to load students.\n");
        return 1;
    }

    printf("Loaded %d students.\n", n);

    // 정렬 기준: 
    // mode 1: ID, 2: NAME, 3: GENDER, 4: SCORE_SUM
    CmpCtx ctxs[] = {
        {1, ASC}, {1, DESC},
        {2, ASC}, {2, DESC},
        {3, ASC}, {3, DESC},
        {4, ASC}, {4, DESC}
    };
    int num_ctx = sizeof(ctxs)/sizeof(ctxs[0]);

    for (int i = 0; i < num_ctx; i++) {
        CmpCtx ctx = ctxs[i];
        printf("\n=== Criteria mode=%d, order=%s ===\n",
               ctx.mode, (ctx.ord==ASC?"ASC":"DESC"));

        int dup = has_duplicate_key(arr, n, ctx);

        // 1) 버블
        run_one_sort("Bubble", bubble_sort, arr, n, ctx);
        // 2) 선택
        run_one_sort("Selection", selection_sort, arr, n, ctx);
        // 3) 삽입
        run_one_sort("Insertion", insertion_sort, arr, n, ctx);
        // 4) 셸
        run_one_sort("Shell", shell_sort, arr, n, ctx);
        // 5) 퀵
        run_one_sort("Quick", quick_sort, arr, n, ctx);
        // 6) 힙 (중복 데이터가 있는 경우에는 힙, 트리 정렬은 하지 않는다)
        if (!dup) {
            run_one_sort("Heap", heap_sort, arr, n, ctx);
        } else {
            printf("[Heap] skipped due to duplicate keys.\n");
        }
        // 7) 병합
        run_one_sort("Merge", merge_sort, arr, n, ctx);
        // 8) 기수 정렬 (ID 오름차순에만 적용: 간단히 예시)
        if (ctx.mode == 1 && ctx.ord == ASC) {
            // 별도: radix_sort_by_id는 비교 함수 쓰지 않으므로 래핑
            Student* work = (Student*)malloc(sizeof(Student) * n);
            Stat total = {0,0};
            for (int r = 0; r < REPEAT; r++) {
                copy_students(work, arr, n);
                Stat s = {0,0};
                radix_sort_by_id(work, n, &s);
                total.comparisons += s.comparisons; // 사실 비교는 거의 없음
                total.memory_ops += s.memory_ops;
            }
            free(work);
            double avg_cmp = (double)total.comparisons / REPEAT;
            double avg_mem = (double)total.memory_ops / REPEAT;
            printf("[Radix(ID ASC)] avg comparisons=%.2f, avg mem_ops=%.2f\n",
                   avg_cmp, avg_mem);
        } else {
            printf("[Radix] skipped (only implemented for ID ASC).\n");
        }
        // 9) 트리
        if (!dup) {
            run_one_sort("Tree", tree_sort, arr, n, ctx);
        } else {
            printf("[Tree] skipped due to duplicate keys.\n");
        }
    }

    free(arr);
    return 0;
}