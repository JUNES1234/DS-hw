#ifndef COMPARE_H
#define COMPARE_H

#include "common.h"

// 오름/내림 플래그
typedef enum { ASC = 1, DESC = -1 } Order;

// 공통적으로 쓰는 비교 헬퍼
int cmp_int(int a, int b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

// ID 기준
int cmp_by_id(const Student* a, const Student* b, Order ord) {
    return ord * cmp_int(a->id, b->id);
}

// NAME 기준 (사전순)
int cmp_by_name(const Student* a, const Student* b, Order ord) {
    int r = strcmp(a->name, b->name);
    if (r < 0) return -1 * ord;
    if (r > 0) return 1 * ord;
    return 0;
}

// GENDER 기준 (stable 정렬에서만 사용)
// 'F','M' 혹은 '1','2' 라고 가정하고 문자 비교
int cmp_by_gender(const Student* a, const Student* b, Order ord) {
    if (a->gender < b->gender) return -1 * ord;
    if (a->gender > b->gender) return 1 * ord;
    return 0; // stable 정렬이므로 같을 때는 기존 순서 유지
}

// 성적 합 기준 (동점이면 국어, 영어, 수학 순으로 더 큰 사람 우선)
// -> 전체적인 순서는 asc/desc에 맞추되, tie-break는 항상 "더 큰 점수 우선"
int cmp_by_score_sum(const Student* a, const Student* b, Order ord) {
    int sa = total_grade(a);
    int sb = total_grade(b);
    if (sa != sb) {
        return ord * cmp_int(sa, sb);
    }
    // 동일 성적이면 국어, 영어, 수학 순으로 더 큰 사람 우선
    if (a->korean != b->korean)
        return -cmp_int(a->korean, b->korean); // 큰 점수 우선 => 내림
    if (a->english != b->english)
        return -cmp_int(a->english, b->english);
    if (a->math != b->math)
        return -cmp_int(a->math, b->math);
    return 0;
}

#endif