#ifndef COMPARATOR_ADAPTER_H
#define COMPARATOR_ADAPTER_H

#include "common.h"
#include "compare.h"

typedef int (*CmpFunc)(const Student*, const Student*, void*);

// 각각의 기준/순서를 ctx로 전달하기 위한 구조체
typedef struct {
    int mode;   // 1: ID, 2: NAME, 3: GENDER, 4: SCORE_SUM
    Order ord;  // ASC / DESC
} CmpCtx;

int generic_cmp(const Student* a, const Student* b, void* ctx_void) {
    CmpCtx* ctx = (CmpCtx*)ctx_void;
    switch (ctx->mode) {
        case 1: return cmp_by_id(a, b, ctx->ord);
        case 2: return cmp_by_name(a, b, ctx->ord);
        case 3: return cmp_by_gender(a, b, ctx->ord);
        case 4: return cmp_by_score_sum(a, b, ctx->ord);
        default: return 0;
    }
}

#endif