/*
 * 设计实验：有向图的路径搜索
 *
 * 有向图 G（6个顶点，用编号 1~6 表示）：
 *   边集：5→1, 5→3, 1→2, 1→4, 3→2, 3→4, 4→2, 4→6, 6→2
 *
 * 任务：
 *   1. 输出从顶点5到顶点2的所有简单路径
 *   2. 输出所有长度为3的简单路径
 *   3. 输出最短路径
 *   3-A. 以递推（迭代）形式打印最短路径的 path 数组
 *   3-B. 以队列（BFS）形式求最短路径
 *   3-C. 以邻接矩阵存储形式实现题意
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXV 7          /* 顶点编号 1~6, 数组大小 7（0 不使用）*/
#define INF  0x3f3f3f3f /* 无穷大（不可达） */

/* ─────────────────────────────────────────────
   邻接表存储
   ───────────────────────────────────────────── */
typedef struct ArcNode {
    int adjvex;         /* 邻接顶点编号 */
    struct ArcNode *next;
} ArcNode;

typedef struct {
    int       vertex;
    ArcNode  *first;
} VNode;

VNode adj_list[MAXV];   /* 邻接表（1-indexed） */
int   num_vertices = 6;

/* 向邻接表添加有向边 u→v */
static void add_edge_list(int u, int v)
{
    ArcNode *p = (ArcNode *)malloc(sizeof(ArcNode));
    p->adjvex = v;
    p->next   = adj_list[u].first;
    adj_list[u].first = p;
}

/* ─────────────────────────────────────────────
   邻接矩阵存储（任务3-C）
   ───────────────────────────────────────────── */
int adj_matrix[MAXV][MAXV];   /* adj_matrix[u][v]=1 表示边 u→v */

/* 向邻接矩阵添加有向边 u→v */
static void add_edge_matrix(int u, int v)
{
    adj_matrix[u][v] = 1;
}

/* ─────────────────────────────────────────────
   图结构可视化
   ───────────────────────────────────────────── */

/* 打印邻接矩阵 */
static void print_adj_matrix(void)
{
    printf("\n  邻接矩阵（行=出发顶点, 列=到达顶点）:\n\n");
    printf("      ");
    for (int j = 1; j <= num_vertices; j++) printf("  %d", j);
    printf("\n    ┌");
    for (int j = 1; j <= num_vertices; j++) printf("───");
    printf("─┐\n");
    for (int i = 1; i <= num_vertices; i++) {
        printf("  %d │", i);
        for (int j = 1; j <= num_vertices; j++) {
            printf("  %d", adj_matrix[i][j]);
        }
        printf("  │\n");
    }
    printf("    └");
    for (int j = 1; j <= num_vertices; j++) printf("───");
    printf("─┘\n");
}

/* 打印邻接表 */
static void print_adj_list(void)
{
    printf("\n  邻接表:\n");
    for (int i = 1; i <= num_vertices; i++) {
        printf("    [%d] →", i);
        ArcNode *p = adj_list[i].first;
        if (!p) { printf(" NULL"); }
        while (p) {
            printf(" [%d] →", p->adjvex);
            p = p->next;
        }
        printf(" NULL\n");
    }
    printf("\n");
}

/* 打印有向图的文字描述 */
static void print_graph_info(void)
{
    printf("\n┌──────────────────────────────────────────────────────────────────────┐\n");
    printf("│                      有向图 G 结构描述                              │\n");
    printf("├──────────────────────────────────────────────────────────────────────┤\n");
    printf("│  顶点集: {1, 2, 3, 4, 5, 6}                                        │\n");
    printf("│  边   集: 5→1, 5→3, 1→2, 1→4, 3→2, 3→4, 4→2, 4→6, 6→2           │\n");
    printf("│                                                                      │\n");
    printf("│  图示（有向）:                                                       │\n");
    printf("│                                                                      │\n");
    printf("│        5                                                             │\n");
    printf("│       / \\                                                           │\n");
    printf("│      ↓   ↓                                                          │\n");
    printf("│      1   3                                                           │\n");
    printf("│     /|   |\\                                                         │\n");
    printf("│    ↓ ↓   ↓ ↓                                                       │\n");
    printf("│    2  4  2  4                                                        │\n");
    printf("│       |     |                                                        │\n");
    printf("│       ↓     ↓                                                       │\n");
    printf("│      2,6   2,6                                                       │\n");
    printf("│        ↓                                                             │\n");
    printf("│        2                                                             │\n");
    printf("│                                                                      │\n");
    printf("│  合并后路径示意:  5→{1,3}→{2,4}→{2,6}→2                          │\n");
    printf("└──────────────────────────────────────────────────────────────────────┘\n");
}

/* ─────────────────────────────────────────────
   任务1 & 2：DFS 枚举所有简单路径
   ───────────────────────────────────────────── */
int   path[MAXV];       /* 当前路径 */
int   path_len;         /* 路径长度（边数） */
int   visited[MAXV];    /* 访问标记 */
int   path_count;       /* 路径总数 */
int   target_len;       /* 0=不限长度; >0=限定路径边数 */

/* 打印一条路径 */
static void print_path(int depth)
{
    path_count++;
    printf("    路径%2d: ", path_count);
    for (int i = 0; i <= depth; i++) {
        printf("%d", path[i]);
        if (i < depth) printf(" → ");
    }
    printf("  (长度=%d)\n", depth);
}

/* DFS 搜索 */
static void dfs(int cur, int end, int depth, int limit)
{
    if (cur == end) {
        if (limit == 0 || depth == limit) {
            print_path(depth);
        }
        return;
    }
    /* 超过限制深度则剪枝 */
    if (limit > 0 && depth >= limit) return;

    ArcNode *p = adj_list[cur].first;
    while (p) {
        int v = p->adjvex;
        if (!visited[v]) {
            visited[v] = 1;
            path[depth + 1] = v;
            dfs(v, end, depth + 1, limit);
            visited[v] = 0;
        }
        p = p->next;
    }
}

/* 使用邻接矩阵的DFS */
static void dfs_matrix(int cur, int end, int depth, int limit)
{
    if (cur == end) {
        if (limit == 0 || depth == limit) {
            print_path(depth);
        }
        return;
    }
    if (limit > 0 && depth >= limit) return;

    for (int v = 1; v <= num_vertices; v++) {
        if (adj_matrix[cur][v] && !visited[v]) {
            visited[v] = 1;
            path[depth + 1] = v;
            dfs_matrix(v, end, depth + 1, limit);
            visited[v] = 0;
        }
    }
}

/* ─────────────────────────────────────────────
   任务3-B：BFS 求最短路径（队列实现）
   ───────────────────────────────────────────── */
#define QUEUE_SIZE 256

typedef struct {
    int data[QUEUE_SIZE];
    int front, rear;
} Queue;

static void queue_init(Queue *q)  { q->front = q->rear = 0; }
static int  queue_empty(Queue *q) { return q->front == q->rear; }
static void enqueue(Queue *q, int x) { q->data[q->rear++ % QUEUE_SIZE] = x; }
static int  dequeue(Queue *q) { return q->data[q->front++ % QUEUE_SIZE]; }

/* BFS 最短路径，返回最短步数；parent[] 记录前驱用于回溯路径 */
static int bfs_shortest(int src, int dst, int parent[], int use_matrix)
{
    int  dist[MAXV];
    Queue q;

    for (int i = 0; i < MAXV; i++) { dist[i] = INF; parent[i] = -1; }
    queue_init(&q);

    dist[src] = 0;
    enqueue(&q, src);

    while (!queue_empty(&q)) {
        int u = dequeue(&q);
        printf("    出队: %d\n", u);

        if (u == dst) break;

        if (use_matrix) {
            /* 邻接矩阵版 */
            for (int v = 1; v <= num_vertices; v++) {
                if (adj_matrix[u][v] && dist[v] == INF) {
                    dist[v]   = dist[u] + 1;
                    parent[v] = u;
                    printf("    入队: %d (dist=%d, 前驱=%d)\n", v, dist[v], u);
                    enqueue(&q, v);
                }
            }
        } else {
            /* 邻接表版 */
            ArcNode *p = adj_list[u].first;
            while (p) {
                int v = p->adjvex;
                if (dist[v] == INF) {
                    dist[v]   = dist[u] + 1;
                    parent[v] = u;
                    printf("    入队: %d (dist=%d, 前驱=%d)\n", v, dist[v], u);
                    enqueue(&q, v);
                }
                p = p->next;
            }
        }
    }

    return (dist[dst] == INF) ? -1 : dist[dst];
}

/* ─────────────────────────────────────────────
   任务3-A：递推（迭代）方式打印 path 数组
   ───────────────────────────────────────────── */
static void print_path_iterative(int parent[], int dst)
{
    /* 先将路径压栈 */
    int stack[MAXV], top = 0;
    int cur = dst;
    while (cur != -1) {
        stack[top++] = cur;
        cur = parent[cur];
    }

    printf("\n  ● 迭代方式打印 path 数组（parent[]）:\n\n");
    printf("    parent 数组内容：\n");
    printf("    顶点: ");
    for (int i = 1; i <= num_vertices; i++) printf("  %d", i);
    printf("\n    前驱: ");
    for (int i = 1; i <= num_vertices; i++) {
        if (parent[i] == -1) printf("  -");
        else                  printf("  %d", parent[i]);
    }
    printf("\n\n");

    printf("    迭代回溯过程:\n");
    int step = 1;
    int cur2 = dst;
    while (cur2 != -1) {
        if (parent[cur2] == -1)
            printf("      第%d步: parent[%d]=-1 → 到达起点 %d, 停止\n", step, cur2, cur2);
        else
            printf("      第%d步: parent[%d]=%d → 继续回溯\n", step, cur2, parent[cur2]);
        cur2 = parent[cur2];
        step++;
    }

    /* 逆序打印（即正向路径） */
    printf("\n    最短路径（正向）: ");
    for (int i = top - 1; i >= 0; i--) {
        printf("%d", stack[i]);
        if (i > 0) printf(" → ");
    }
    printf("\n");
}

/* 可视化打印最短路径 */
static void print_shortest_path_visual(int parent[], int dst, int dist)
{
    int stack[MAXV], top = 0;
    int cur = dst;
    while (cur != -1) {
        stack[top++] = cur;
        cur = parent[cur];
    }

    printf("\n  ✓ 最短路径（长度=%d）：\n    ", dist);
    for (int i = top - 1; i >= 0; i--) {
        printf("%d", stack[i]);
        if (i > 0) printf(" → ");
    }
    printf("\n\n");

    /* ASCII 路径框图 */
    printf("  路径可视化:\n\n    ┌───┐");
    for (int i = top - 2; i >= 0; i--) printf("   ┌───┐");
    printf("\n    │ %d │", stack[top - 1]);
    for (int i = top - 2; i >= 0; i--) printf(" → │ %d │", stack[i]);
    printf("\n    └───┘");
    for (int i = top - 2; i >= 0; i--) printf("   └───┘");
    printf("\n\n");
}

/* ─────────────────────────────────────────────
   建图
   ───────────────────────────────────────────── */
static void build_graph(void)
{
    /* 初始化邻接表 */
    for (int i = 1; i <= num_vertices; i++) {
        adj_list[i].vertex = i;
        adj_list[i].first  = NULL;
    }
    /* 初始化邻接矩阵 */
    memset(adj_matrix, 0, sizeof(adj_matrix));

    /* 添加边（邻接表头插，顺序有影响；这里控制插入顺序使输出更自然）*/
    /* 注意：头插法会逆序，所以按逆序插入使输出顺序自然 */
    int edges[][2] = {
        {5,3},{5,1},
        {1,4},{1,2},
        {3,4},{3,2},
        {4,6},{4,2},
        {6,2}
    };
    int ne = (int)(sizeof(edges) / sizeof(edges[0]));
    for (int i = 0; i < ne; i++) {
        add_edge_list(edges[i][0], edges[i][1]);
        add_edge_matrix(edges[i][0], edges[i][1]);
    }
}

/* ─────────────────────────────────────────────
   主函数
   ───────────────────────────────────────────── */
int main(void)
{
    int src = 5, dst = 2;

    printf("\n");
    printf("*****************************************************************\n");
    printf("*         设计实验：有向图路径搜索（顶点 5 → 顶点 2）           *\n");
    printf("*****************************************************************\n");

    build_graph();
    print_graph_info();

    printf("\n");
    printf("  邻接表存储结构:\n");
    print_adj_list();

    /* ── 任务1：所有简单路径（邻接表版 DFS） ── */
    printf("=================================================================\n");
    printf("【任务1】顶点 %d → 顶点 %d 的所有简单路径（DFS 邻接表）\n", src, dst);
    printf("=================================================================\n\n");

    memset(visited, 0, sizeof(visited));
    path[0]    = src;
    path_count = 0;
    visited[src] = 1;
    dfs(src, dst, 0, 0);   /* limit=0: 不限长度 */
    visited[src] = 0;
    printf("\n  共找到 %d 条简单路径\n\n", path_count);

    /* ── 任务2：长度为3的简单路径 ── */
    printf("=================================================================\n");
    printf("【任务2】顶点 %d → 顶点 %d 的所有长度为 3 的简单路径\n", src, dst);
    printf("=================================================================\n\n");

    memset(visited, 0, sizeof(visited));
    path[0]    = src;
    path_count = 0;
    visited[src] = 1;
    dfs(src, dst, 0, 3);   /* limit=3: 仅长度3 */
    visited[src] = 0;
    printf("\n  共找到 %d 条长度为 3 的简单路径\n\n", path_count);

    /* ── 任务3-B：BFS 最短路径（邻接表） ── */
    printf("=================================================================\n");
    printf("【任务3-B】BFS（队列）求顶点 %d → 顶点 %d 的最短路径（邻接表）\n", src, dst);
    printf("=================================================================\n\n");

    int parent_list[MAXV];
    printf("  BFS 过程:\n");
    int dist_list = bfs_shortest(src, dst, parent_list, 0);

    if (dist_list < 0) {
        printf("  ✗ 不可达\n\n");
    } else {
        print_shortest_path_visual(parent_list, dst, dist_list);
    }

    /* ── 任务3-A：迭代方式打印 path 数组 ── */
    printf("=================================================================\n");
    printf("【任务3-A】递推（迭代）方式打印最短路径的 path 数组\n");
    printf("=================================================================\n");
    if (dist_list >= 0) {
        print_path_iterative(parent_list, dst);
    }
    printf("\n");

    /* ── 任务3-C：邻接矩阵版 ── */
    printf("=================================================================\n");
    printf("【任务3-C】邻接矩阵存储——所有简单路径 + BFS 最短路径\n");
    printf("=================================================================\n");

    printf("\n  邻接矩阵存储结构:\n");
    print_adj_matrix();

    printf("\n  ● 邻接矩阵 DFS：顶点 %d → 顶点 %d 的所有简单路径\n\n", src, dst);
    memset(visited, 0, sizeof(visited));
    path[0]    = src;
    path_count = 0;
    visited[src] = 1;
    dfs_matrix(src, dst, 0, 0);
    visited[src] = 0;
    printf("\n  共找到 %d 条简单路径\n\n", path_count);

    printf("\n  ● 邻接矩阵 BFS：顶点 %d → 顶点 %d 的最短路径\n\n", src, dst);
    int parent_mat[MAXV];
    printf("  BFS 过程:\n");
    int dist_mat = bfs_shortest(src, dst, parent_mat, 1);

    if (dist_mat < 0) {
        printf("  ✗ 不可达\n\n");
    } else {
        print_shortest_path_visual(parent_mat, dst, dist_mat);
    }

    printf("*****************************************************************\n");
    printf("*                     图路径搜索演示完毕                          *\n");
    printf("*****************************************************************\n\n");

    /* 释放邻接表内存 */
    for (int i = 1; i <= num_vertices; i++) {
        ArcNode *p = adj_list[i].first;
        while (p) {
            ArcNode *tmp = p->next;
            free(p);
            p = tmp;
        }
    }

    return 0;
}
