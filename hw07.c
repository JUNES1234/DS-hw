#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>

#define N 10          // 정점 수
#define M 20          // 간선 수 (무방향 간선)
#define INF 0x3f3f3f3f

// 디버그 로그 매크로
#ifndef DBG
#define DBG 0
#endif
#define DLOG(...) do { if (DBG) fprintf(stderr, __VA_ARGS__); } while (0)

typedef struct Edge {
    int to;
    int w;
    struct Edge* next;
} Edge;

typedef struct {
    Edge* head;
} Graph;

typedef struct {
    int dist[N];
    int parent[N];
} DijkstraResult;

// 그래프 초기화/해제
void initGraph(Graph* g) {
    for (int i = 0; i < N; ++i) g[i].head = NULL;
}

void freeGraph(Graph* g) {
    for (int i = 0; i < N; ++i) {
        Edge* cur = g[i].head;
        // 방어적 해제: 최대 안전 카운트(노드 수 상한)로 루프 보호
        int guard = 10000;
        while (cur && guard-- > 0) {
            Edge* nx = cur->next;
            free(cur);
            cur = nx;
        }
        g[i].head = NULL;
    }
}

// 범위 체크 유틸
static inline bool validV(int v) { return v >= 0 && v < N; }

// 안전한 에지 추가
void addEdge(Graph* g, int u, int v, int w) {
    if (!validV(u) || !validV(v)) {
        fprintf(stderr, "[ERR] addEdge 범위 오류 u=%d v=%d\n", u, v);
        exit(1);
    }
    DLOG("[DEBUG] addEdge: u=%d, v=%d, w=%d\n", u, v, w);
    // u -> v
    Edge* e1 = (Edge*)malloc(sizeof(Edge));
    if (!e1) { perror("malloc"); exit(1); }
    e1->to = v; e1->w = w; e1->next = g[u].head; g[u].head = e1;

    // v -> u (무방향)
    Edge* e2 = (Edge*)malloc(sizeof(Edge));
    if (!e2) { perror("malloc"); exit(1); }
    e2->to = u; e2->w = w; e2->next = g[v].head; g[v].head = e2;
}

// 무작위 간선 생성 (무방향, 중복/루프 금지), 가중치 1~10
void generateRandomGraph(Graph* g) {
    int used[N][N] = {0};
    int edges = 0;
    int safety = 100000; // 무한루프 방지
    while (edges < M && safety-- > 0) {
        int u = rand() % N;
        int v = rand() % N;
        if (u == v) continue;
        if (u > v) { int t = u; u = v; v = t; }
        if (used[u][v]) continue;
        used[u][v] = 1;

        int w = 1 + rand() % 10;
        addEdge(g, u, v, w);
        edges++;
    }
    if (edges < M) {
        fprintf(stderr, "[ERR] 간선 생성 실패: 생성 %d/%d\n", edges, M);
        exit(1);
    }
    
}

// 그래프 구조 검증: 각 리스트가 유효 포인터 체인인지 확인
bool validateGraph(const Graph* g) {
    for (int u = 0; u < N; ++u) {
        // 각 정점에 대해서 too long 보호(사이클/자기참조 탐지)
        int seen = 0;
        for (const Edge* e = g[u].head; e; e = e->next) {
            if (!validV(e->to)) {
                fprintf(stderr, "[ERR] 잘못된 to 값: u=%d to=%d\n", u, e->to);
                return false;
            }
            if (++seen > 1000) {
                fprintf(stderr, "[ERR] 리스트가 비정상적으로 깁니다(사이클 의심): u=%d\n", u);
                return false;
            }
        }
    }
    return true;
}

// Dijkstra (단순 배열 기반 선택, O(N^2 + E))
void dijkstra(const Graph* g, int src, DijkstraResult* out) {
    bool vis[N] = {0};
    for (int i = 0; i < N; ++i) {
        out->dist[i] = INF;
        out->parent[i] = -1;
    }
    out->dist[src] = 0;

    for (int it = 0; it < N; ++it) {
        // 아직 방문하지 않은 정점 중 dist 최소 선택
        int u = -1, best = INF;
        for (int i = 0; i < N; ++i) {
            if (!vis[i] && out->dist[i] < best) {
                best = out->dist[i];
                u = i;
            }
        }
        if (u == -1) break;
        vis[u] = true;

        // 이웃 완화 (방어적으로 next를 먼저 저장)
        for (Edge* e = g[u].head; e; ) {
            Edge* next_e = e->next;
            int v = e->to;
            int nd = out->dist[u] + e->w;
            if (nd < out->dist[v]) {
                out->dist[v] = nd;
                out->parent[v] = u;
            }
            e = next_e;
        }
    }
}

// 경로 복원: src->dst 경로를 path 배열에 역순으로 채우고 길이 반환
int reconstructPath(int src, int dst, const DijkstraResult* r, int* path) {
    if (r->dist[dst] >= INF) return 0; // 경로 없음
    int len = 0;
    int cur = dst;
    // 방어: 최대 길이 N 초과 방지
    while (cur != -1 && len < N) {
        path[len++] = cur;
        if (cur == src) break;
        cur = r->parent[cur];
    }
    if (len >= N && path[len - 1] != src) return 0; // 비정상 경로
    // reverse path
    for (int i = 0; i < len / 2; ++i) {
        int t = path[i];
        path[i] = path[len - 1 - i];
        path[len - 1 - i] = t;
    }
    return len;
}

int main(void) {
    srand((unsigned)time(NULL));

    Graph g[N];
    initGraph(g);

    generateRandomGraph(g);

    // 출력 전 구조 검증
    if (!validateGraph(g)) {
        fprintf(stderr, "[ERR] 그래프 구조가 손상되었습니다.\n");
        freeGraph(g);
        return 1;
    }

    // 생성된 그래프(간선 목록) 출력: u < v만 한 번 출력
    printf("무작위 무방향 가중 그래프 생성 (정점 수=%d, 간선 수=%d)\n", N, M);
    int printed[N][N] = {0};
    for (int u = 0; u < N; ++u) {
        // 방어: head가 NULL일 수도 있음
        if (g[u].head == NULL) 
            continue; // NULL인 경우 건너뛰기
        Edge* e = g[u].head;
        int guard = 10000; // 비정상 루프 방지
        while (e && guard-- > 0) {
            if (e == NULL) break; // 방어 코드 추가
            int v = e->to;
            if (validV(v)) {
                if (u < v && !printed[u][v]) {
                    printed[u][v] = 1;
                    printf("간선 %2d - %2d, 가중치=%d\n", u, v, e->w);
                }
            } else {
                fprintf(stderr, "[WARN] 잘못된 to 값 감지: u=%d to=%d\n", u, v);
                break;
            }
            e = e->next;
        }
        if (guard <= 0) {
            fprintf(stderr, "[ERR] 간선 출력 중 비정상 루프 의심: u=%d\n", u);
            freeGraph(g);
            return 1;
        }
    }
    printf("\n");

    // 모든 쌍 최단경로 계산
    DijkstraResult res[N];
    for (int s = 0; s < N; ++s) {
        dijkstra(g, s, &res[s]);
    }

    // 45쌍(u<v) 결과 출력
    printf("모든 노드 쌍(u < v)의 최단경로 결과:\n");
    for (int u = 0; u < N; ++u) {
        for (int v = u + 1; v < N; ++v) {
            int path[N];
            int len = reconstructPath(u, v, &res[u], path);
            if (len == 0) {
                printf("%2d -> %2d : 거리 = INF, 경로 = (없음)\n", u, v);
            } else {
                printf("%2d -> %2d : 거리 = %d, 경로 = ", u, v, res[u].dist[v]);
                for (int i = 0; i < len; ++i) {
                    if (i) printf(" -> ");
                    printf("%d", path[i]);
                }
                printf("\n");
            }
        }
    }

    freeGraph(g);
    return 0;
}