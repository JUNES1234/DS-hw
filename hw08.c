/*
- 0~1,000,000 사이의 무작위 데이터로 이루어진 1만개의 데이터를 기본적인 쉘 정렬 및 단순 삽입 정렬로 정렬하고
, 각 정렬 방식의 평균 비교 횟수를 출력하시오.
  + 각 정렬에 대해 최종 출력은 100회의 실행 후 평균을 통해 계산한다.
  + 쉘 정렬의 간격 조정 방식은 배열 길이의 1/2, 1/4, 1/8...로 줄이는 방식을 사용한다.

- 또한, 주어진 조건 하에 쉘 정렬에 대해, 조건상에서 최적의 쉘 간격을 설정하는 알고리즘을 생각하여 해당 방법으로도 쉘 정렬을 수행하고
, 비교 횟수를 최하로 줄인 결과도 출력해본다.

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* 설정 */
#define N            10000
#define RUNS         100
#define RAND_MAXVAL  1000000

/* 고속 RNG: xorshift64 */
static inline uint64_t xorshift64(uint64_t *state) {
    uint64_t x = *state;
    x ^= x << 13;
    x ^= x >> 7;
    x ^= x << 17;
    *state = x;
    return x;
}

/* 0 ~ RAND_MAXVAL 범위 난수 생성 */
static inline int fast_rand_int(uint64_t *state) {
    return (int)(xorshift64(state) % (RAND_MAXVAL + 1));
}

/* 테스트 데이터 생성 */
static void fill_random(int *a, int n, uint64_t *state) {
    for (int i = 0; i < n; ++i) a[i] = fast_rand_int(state);
}

/* 배열 복제 */
static inline void clone_array(const int *src, int *dst, int n) {
    memcpy(dst, src, sizeof(int) * n);
}

/* 단순 삽입 정렬: 데이터 비교 횟수 리턴 */
static uint64_t insertion_sort_count(int *a, int n) {
    uint64_t cmp = 0;
    for (int i = 1; i < n; ++i) {
        int key = a[i];
        int j = i;
        /* j > 0은 인덱스 비교(비교 횟수에 포함 X)
           a[j-1] > key는 데이터 비교(비교 횟수에 포함 O) */
        while (j > 0) {
            cmp++; /* a[j-1] > key 비교 수행 카운트 */
            if (a[j - 1] > key) {
                a[j] = a[j - 1];
                j--;
            } else {
                break; /* 마지막 실패 비교도 이미 카운트됨 */
            }
        }
        a[j] = key;
    }
    return cmp;
}

/* 쉘 정렬(간격: n/2, n/4, ... 단순 절반 간격) */
static uint64_t shell_sort_halving_count(int *a, int n) {
    uint64_t cmp = 0;
    for (int gap = n / 2; gap > 0; gap /= 2) {
        for (int i = gap; i < n; ++i) {
            int temp = a[i];
            int j = i;
            while (j >= gap) {
                cmp++; /* a[j-gap] > temp 비교 */
                if (a[j - gap] > temp) {
                    a[j] = a[j - gap];
                    j -= gap;
                } else {
                    break;
                }
            }
            a[j] = temp;
        }
    }
    return cmp;
}

/*
Tokuda gap sequence 생성(상승 순으로 채우고, 정렬에서 역순 사용)
Tokuda: 1, 4, 9, 20, 46, 103, 233, 525, 1182, 2678, 6051, ...
문헌상 랜덤 데이터에서 비교/이동 횟수 측면에서 매우 우수한 편
참고 식: h(k) = ceil( (9 * (9/4)^(k-1) - 4) / 5 ), k=1,2,...
여기서는 부동 소수점 없이 간단한 생성(경험적 상수)으로 구현
*/
static int build_tokuda_gaps(int n, int *gaps, int max_count) {
    int count = 0;
    /* 간단 재귀 생성: h_{k+1} = 2.25*h_k + 1 (반올림) 근사
       정확한 수열과 약간 차이가 있지만 실용적으로 동일한 계열을 생성 */
    double h = 1.0;
    while ((int)h < n) {
        if (count < max_count) {
            gaps[count++] = (int)(h + 0.5); /* 반올림 */
        }
        h = 2.25 * h + 1.0;
    }
    return count; /* gaps[0..count-1] 오름차순 */
}

/* 쉘 정렬(Tokuda 계열 간격 사용) */
static uint64_t shell_sort_tokuda_count(int *a, int n) {
    uint64_t cmp = 0;
    /* 최대 필요한 gap 개수는 30 내외 (n=10k 기준) */
    int gaps[64];
    int gcount = build_tokuda_gaps(n, gaps, 64);

    /* 역순(큰 간격부터) 적용 */
    for (int gi = gcount - 1; gi >= 0; --gi) {
        int gap = gaps[gi];
        if (gap <= 0) continue;
        for (int i = gap; i < n; ++i) {
            int temp = a[i];
            int j = i;
            while (j >= gap) {
                cmp++; /* a[j-gap] > temp 비교 */
                if (a[j - gap] > temp) {
                    a[j] = a[j - gap];
                    j -= gap;
                } else {
                    break;
                }
            }
            a[j] = temp;
        }
    }
    return cmp;
}

int main(void) {
    /* 난수 초기화: 시간 기반 + 주소 혼합으로 시드 강화 */
    uint64_t seed = (uint64_t)time(NULL);
    seed ^= (uint64_t)(uintptr_t)&seed;

    /* 버퍼 준비 */
    int *base = (int*)malloc(sizeof(int) * N);
    int *a_ins = (int*)malloc(sizeof(int) * N);
    int *a_shl = (int*)malloc(sizeof(int) * N);
    int *a_tok = (int*)malloc(sizeof(int) * N);
    if (!base || !a_ins || !a_shl || !a_tok) {
        fprintf(stderr, "메모리 할당 실패\n");
        return 1;
    }

    uint64_t sum_ins = 0;
    uint64_t sum_shl = 0;
    uint64_t sum_tok = 0;

    for (int run = 1; run <= RUNS; ++run) {
        fill_random(base, N, &seed);

        clone_array(base, a_ins, N);
        clone_array(base, a_shl, N);
        clone_array(base, a_tok, N);

        uint64_t c_ins = insertion_sort_count(a_ins, N);
        uint64_t c_shl = shell_sort_halving_count(a_shl, N);
        uint64_t c_tok = shell_sort_tokuda_count(a_tok, N);

        sum_ins += c_ins;
        sum_shl += c_shl;
        sum_tok += c_tok;

        /* 진행 상황(선택): 주석 해제 시 진행률 출력
        if (run % 10 == 0) {
            fprintf(stderr, "진행 %d/%d\n", run, RUNS);
        }
        */
    }

    /* 평균 계산 */
    double avg_ins = (double)sum_ins / RUNS;
    double avg_shl = (double)sum_shl / RUNS;
    double avg_tok = (double)sum_tok / RUNS;

    printf("데이터 크기: %d, 실행 횟수: %d\n", N, RUNS);
    printf("삽입 정렬 평균 비교 횟수            : %.0f\n", avg_ins);
    printf("쉘 정렬(절반 간격) 평균 비교 횟수    : %.0f\n", avg_shl);
    printf("쉘 정렬(Tokuda 간격) 평균 비교 횟수  : %.0f\n", avg_tok);

    /* 상대 개선율 */
    if (avg_shl > 0.0) {
        printf("Tokuda 대비 절반 간격 개선율: %.2f%%\n", (1.0 - avg_tok / avg_shl) * 100.0);
    }
    if (avg_ins > 0.0) {
        printf("Tokuda 대비 삽입 정렬 개선율: %.2f%%\n", (1.0 - avg_tok / avg_ins) * 100.0);
    }

    free(base); free(a_ins); free(a_shl); free(a_tok);
    return 0;
}