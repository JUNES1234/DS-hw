#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

// 설정
#define N_VERT 100
#define SPARSE_EDGES 100     // 무방향 간선 수
#define DENSE_EDGES 4000     // 무방향 간선 수

// 전역 비교 카운터
typedef struct {
    long long cmp_insert_delete;
    long long cmp_connected;
    long long cmp_neighbors;
} Counters;

static inline void resetCounters(Counters* c) {
    c->cmp_insert_delete = 0;
    c->cmp_connected = 0;
    c->cmp_neighbors = 0;
}

// ========================= 인접행렬 그래프 =========================
typedef struct {
    int n;              // 정점 수
    unsigned char* m;   // n x n, 0/1
} AdjMatrix;

AdjMatrix* am_create(int n) {
    AdjMatrix* g = (AdjMatrix*)malloc(sizeof(AdjMatrix));
    g->n = n;
    g->m = (unsigned char*)calloc((size_t)n * (size_t)n, sizeof(unsigned char));
    return g;
}

void am_free(AdjMatrix* g) {
    if (!g) return;
    free(g->m);
    free(g);
}

// 존재 확인과 삽입/삭제에서의 비교 정의:
// - 존재 확인: 한 번의 읽기 후 값 비교 1회로 카운트
// - 중복 삽입 방지: 먼저 존재 비교 1회
// - 삭제 시 존재 비교 1회
bool am_has_edge(AdjMatrix* g, int u, int v, Counters* c) {
    // 비교 1회 (m[u*n+v] != 0)
    c->cmp_connected += 1;
    return g->m[(size_t)u * g->n + v] != 0;
}

bool am_insert_edge(AdjMatrix* g, int u, int v, Counters* c) {
    // 존재 비교 1회
    c->cmp_insert_delete += 1;
    if (g->m[(size_t)u * g->n + v] != 0) return false;
    g->m[(size_t)u * g->n + v] = 1;
    g->m[(size_t)v * g->n + u] = 1;
    return true;
}

bool am_delete_edge(AdjMatrix* g, int u, int v, Counters* c) {
    // 존재 비교 1회
    c->cmp_insert_delete += 1;
    if (g->m[(size_t)u * g->n + v] == 0) return false;
    g->m[(size_t)u * g->n + v] = 0;
    g->m[(size_t)v * g->n + u] = 0;
    return true;
}

int am_neighbors(AdjMatrix* g, int u, int* out, int cap, Counters* c) {
    int count = 0;
    for (int v = 0; v < g->n; ++v) {
        // 비교 1회: m[u*n+v] != 0
        c->cmp_neighbors += 1;
        if (g->m[(size_t)u * g->n + v] != 0) {
            if (count < cap) out[count] = v;
            count++;
        }
    }
    return count;
}

size_t am_memory_bytes(AdjMatrix* g) {
    // 구조체 + 행렬
    return sizeof(AdjMatrix) + (size_t)g->n * (size_t)g->n * sizeof(unsigned char);
}

// ========================= 인접리스트 그래프 =========================
typedef struct Node {
    int v;
    struct Node* next;
} Node;

typedef struct {
    int n;
    Node** heads; // 크기 n의 포인터 배열
} AdjList;

AdjList* al_create(int n) {
    AdjList* g = (AdjList*)malloc(sizeof(AdjList));
    g->n = n;
    g->heads = (Node**)calloc(n, sizeof(Node*));
    return g;
}

void al_free(AdjList* g) {
    if (!g) return;
    for (int u = 0; u < g->n; ++u) {
        Node* cur = g->heads[u];
        while (cur) {
            Node* nx = cur->next;
            free(cur);
            cur = nx;
        }
    }
    free(g->heads);
    free(g);
}

bool al_has_edge(AdjList* g, int u, int v, Counters* c) {
    Node* cur = g->heads[u];
    while (cur) {
        // 비교 1회: cur->v == v
        c->cmp_connected += 1;
        if (cur->v == v) return true;
        cur = cur->next;
    }
    return false;
}

// 내부: u의 리스트에서 v 노드 삭제(있으면 1, 없으면 0). 비교 횟수 기록.
static bool al_delete_from_list(AdjList* g, int u, int v, Counters* c) {
    Node* cur = g->heads[u];
    Node* prev = NULL;
    while (cur) {
        c->cmp_insert_delete += 1; // 비교: cur->v == v
        if (cur->v == v) {
            if (prev) prev->next = cur->next;
            else g->heads[u] = cur->next;
            free(cur);
            return true;
        }
        prev = cur;
        cur = cur->next;
    }
    return false;
}

bool al_insert_edge(AdjList* g, int u, int v, Counters* c) {
    // 중복 방지: 존재 여부 확인(최대 차수만큼 비교 발생)
    Node* cur = g->heads[u];
    while (cur) {
        c->cmp_insert_delete += 1; // 비교: cur->v == v
        if (cur->v == v) return false; // 이미 존재
        cur = cur->next;
    }
    // u->v 삽입 (head 삽입: 비교 없음)
    Node* a = (Node*)malloc(sizeof(Node));
    a->v = v; a->next = g->heads[u]; g->heads[u] = a;

    // v 측에도 삽입. 중복 방지 필요: v 리스트에서 u 존재 확인
    cur = g->heads[v];
    while (cur) {
        c->cmp_insert_delete += 1; // 비교
        if (cur->v == u) {
            // 이미 반대쪽이 존재하는 경우는 일반적으로 없음(동기화 유지 시),
            // 그래도 안전하게 중복 삽입 방지
            return true;
        }
        cur = cur->next;
    }
    Node* b = (Node*)malloc(sizeof(Node));
    b->v = u; b->next = g->heads[v]; g->heads[v] = b;
    return true;
}

bool al_delete_edge(AdjList* g, int u, int v, Counters* c) {
    bool a = al_delete_from_list(g, u, v, c);
    bool b = al_delete_from_list(g, v, u, c);
    return a && b;
}

int al_neighbors(AdjList* g, int u, int* out, int cap, Counters* c) {
    int count = 0;
    Node* cur = g->heads[u];
    while (cur) {
        // 인접리스트의 순회는 존재 비교가 아니라 "방문"이므로 비교 카운트는 선택 사항.
        // 문제 정의에 맞춰 "인접 노드 출력 시 비교 횟수"로 방문 1회를 비교 1회로 간주.
        c->cmp_neighbors += 1;
        if (count < cap) out[count] = cur->v;
        count++;
        cur = cur->next;
    }
    return count;
}

size_t al_memory_bytes(AdjList* g) {
    // 구조체 + 헤더 포인터 배열 + 모든 노드(무방향이므로 간선당 2노드)
    size_t bytes = sizeof(AdjList) + (size_t)g->n * sizeof(Node*);
    // 노드 수를 세자
    size_t nodes = 0;
    for (int u = 0; u < g->n; ++u) {
        Node* cur = g->heads[u];
        while (cur) { nodes++; cur = cur->next; }
    }
    bytes += nodes * sizeof(Node);
    return bytes;
}

// ========================= 랜덤 그래프 생성(무방향, 단일 간선, 무루프) =========================

typedef struct {
    int u, v;
} Edge;

// 간선 셋 존재 여부를 추적하기 위한 보조 구조(행렬 비트/바이트)
static inline size_t pairIndex(int n, int u, int v) {
    return (size_t)u * (size_t)n + (size_t)v;
}

void generate_random_edges(int n, int targetEdges, Edge* edges_out, int* outCount) {
    // 무방향, u < v만 보관하여 중복 방지
    unsigned char* used = (unsigned char*)calloc((size_t)n * (size_t)n, 1);
    int cnt = 0;
    while (cnt < targetEdges) {
        int u = rand() % n;
        int v = rand() % n;
        if (u == v) continue;
        if (u > v) { int t = u; u = v; v = t; }
        size_t idx1 = pairIndex(n, u, v);
        if (used[idx1]) continue;
        used[idx1] = 1;
        edges_out[cnt].u = u;
        edges_out[cnt].v = v;
        cnt++;
    }
    free(used);
    *outCount = cnt;
}

// 빌드 함수들
AdjMatrix* build_am_from_edges(int n, Edge* E, int m, Counters* c) {
    AdjMatrix* g = am_create(n);
    for (int i = 0; i < m; ++i) {
        am_insert_edge(g, E[i].u, E[i].v, c);
    }
    return g;
}

AdjList* build_al_from_edges(int n, Edge* E, int m, Counters* c) {
    AdjList* g = al_create(n);
    for (int i = 0; i < m; ++i) {
        al_insert_edge(g, E[i].u, E[i].v, c);
    }
    return g;
}

// ========================= 벤치마크 루틴 =========================

typedef struct {
    const char* name;
    size_t memBytes;
    long long cmp_ins_del;
    long long cmp_conn;
    long long cmp_nei;
} Report;

void benchmark_am(const char* name, int n, Edge* baseEdges, int m, Report* rep) {
    Counters c; resetCounters(&c);
    AdjMatrix* g = build_am_from_edges(n, baseEdges, m, &c);

    // 메모리 측정
    rep->name = name;
    rep->memBytes = am_memory_bytes(g);

    // 테스트 연산:
    // 1) 임의 간선 100개 삽입 시도(절반은 존재할 수도, 없을 수도)
    // 2) 임의 간선 100개 삭제 시도
    // 3) 연결 여부 확인 1000회
    // 4) 임의 정점 100개에 대해 인접 노드 나열
    int trials_ins = 100, trials_del = 100, trials_conn = 1000, trials_nei = 100;

    // 삽입 테스트
    for (int i = 0; i < trials_ins; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        am_insert_edge(g, u, v, &c);
    }
    // 삭제 테스트
    for (int i = 0; i < trials_del; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        am_delete_edge(g, u, v, &c);
    }
    // 연결 여부 테스트
    for (int i = 0; i < trials_conn; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        (void)am_has_edge(g, u, v, &c);
    }
    // 인접 노드 출력 테스트
    int buf[1000];
    for (int i = 0; i < trials_nei; ++i) {
        int u = rand() % n;
        (void)am_neighbors(g, u, buf, 1000, &c);
    }

    rep->cmp_ins_del = c.cmp_insert_delete;
    rep->cmp_conn = c.cmp_connected;
    rep->cmp_nei = c.cmp_neighbors;

    am_free(g);
}

void benchmark_al(const char* name, int n, Edge* baseEdges, int m, Report* rep) {
    Counters c; resetCounters(&c);
    AdjList* g = build_al_from_edges(n, baseEdges, m, &c);

    // 메모리 측정
    rep->name = name;
    rep->memBytes = al_memory_bytes(g);

    int trials_ins = 100, trials_del = 100, trials_conn = 1000, trials_nei = 100;

    // 삽입 테스트
    for (int i = 0; i < trials_ins; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        al_insert_edge(g, u, v, &c);
    }
    // 삭제 테스트
    for (int i = 0; i < trials_del; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        al_delete_edge(g, u, v, &c);
    }
    // 연결 여부 테스트
    for (int i = 0; i < trials_conn; ++i) {
        int u = rand() % n, v = rand() % n;
        if (u == v) { v = (v + 1) % n; }
        (void)al_has_edge(g, u, v, &c);
    }
    // 인접 노드 출력 테스트
    int buf[1000];
    for (int i = 0; i < trials_nei; ++i) {
        int u = rand() % n;
        (void)al_neighbors(g, u, buf, 1000, &c);
    }

    rep->cmp_ins_del = c.cmp_insert_delete;
    rep->cmp_conn = c.cmp_connected;
    rep->cmp_nei = c.cmp_neighbors;

    al_free(g);
}

// ========================= 메인: 4 케이스 실행 =========================
int main(void) {
    srand((unsigned)time(NULL));

    // 1) 희소 그래프(100간선) 생성용 에지 목록
    Edge* Es = (Edge*)malloc(sizeof(Edge) * SPARSE_EDGES);
    int ms = 0;
    generate_random_edges(N_VERT, SPARSE_EDGES, Es, &ms);

    // 2) 밀집 그래프(4000간선) 생성용 에지 목록
    Edge* Ed = (Edge*)malloc(sizeof(Edge) * DENSE_EDGES);
    int md = 0;
    generate_random_edges(N_VERT, DENSE_EDGES, Ed, &md);

    Report r1, r2, r3, r4;
    benchmark_am("케이스 1: 희소그래프-인접행렬", N_VERT, Es, ms, &r1);
    benchmark_al("케이스 2: 희소그래프-인접리스트", N_VERT, Es, ms, &r2);
    benchmark_am("케이스 3: 밀집그래프-인접행렬", N_VERT, Ed, md, &r3);
    benchmark_al("케이스 4: 밀집그래프-인접리스트", N_VERT, Ed, md, &r4);

    // 출력
    Report reps[4] = { r1, r2, r3, r4 };
    for (int i = 0; i < 4; ++i) {
        printf("%s\n", reps[i].name);
        printf("메모리 %zu Bytes\n", reps[i].memBytes);
        printf("간선 삽입/삭제 비교 %lld번\n", reps[i].cmp_ins_del);
        printf("두 정점의 연결 확인 비교 %lld번\n", reps[i].cmp_conn);
        printf("한 노드의 인접 노드 출력 비교 %lld번\n", reps[i].cmp_nei);
        printf("\n");
    }

    free(Es);
    free(Ed);
    return 0;
}