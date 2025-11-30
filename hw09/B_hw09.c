#include "common.h"
#include "compare.h"
#include "comparator_adapter.h"
#include "sorts.h"          
#include "improved_sorts.h"

#define REPEAT 1000

void copy_students(Student* dst, const Student* src, int n) {
    memcpy(dst, src, sizeof(Student) * n);
}

void run_one_improved(
    const char* name,
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

    printf("[Improved %s] mode=%d, order=%s -> avg comparisons=%.2f, avg mem_ops=%.2f\n",
           name,
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

        // 개선된 Shell
        run_one_improved("Shell(Knuth)", shell_sort_knuth, arr, n, ctx);
        // 개선된 Quick(median-of-three)
        run_one_improved("Quick(median-of-three)", quick_sort_m3, arr, n, ctx);
        // 개선된 Tree(AVL)
        run_one_improved("Tree(AVL)", tree_sort_avl, arr, n, ctx);
    }

    free(arr);
    return 0;
}