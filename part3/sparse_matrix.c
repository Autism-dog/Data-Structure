/*
 * 综合实验：稀疏矩阵转置算法
 *
 * 任务：
 *   1. 快速稀疏矩阵转置算法（Fast Transpose）
 *   2. 进一步优化：用更少存储空间的原地/节省空间版本
 *   3. 时间复杂度分析，与非快速算法对比
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ─────────────────────────────────────────────
   三元组表（稀疏矩阵压缩存储）
   ───────────────────────────────────────────── */
#define MAXTERMS 100   /* 最多非零元素个数 */

typedef struct {
    int row, col, val;
} Triple;

typedef struct {
    Triple data[MAXTERMS + 1];   /* data[0] 不用（1-indexed） */
    int    rows, cols, terms;    /* 行数、列数、非零元素数 */
} TSMatrix;

/* ─────────────────────────────────────────────
   示例矩阵
   ─────────────────────────────────────────────
   M =
      0  12   9   0   0   0
      0   0   0   0   0   0
     -3   0   0   0   0  14
      0   0  24   0  18   0
     15   0   0  -7   0   0
      0   0  -4   0   0   0
   ───────────────────────────────────────────── */

/* 从二维数组建立三元组表 */
static void build_tsmatrix(TSMatrix *M, int arr[][6], int rows, int cols)
{
    M->rows  = rows;
    M->cols  = cols;
    M->terms = 0;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (arr[i][j] != 0) {
                M->terms++;
                M->data[M->terms].row = i + 1;
                M->data[M->terms].col = j + 1;
                M->data[M->terms].val = arr[i][j];
            }
        }
    }
}

/* ─────────────────────────────────────────────
   可视化：打印原始矩阵（二维形式）
   ───────────────────────────────────────────── */
static void print_matrix_2d(const TSMatrix *M, const char *title)
{
    /* 先还原为二维数组 */
    int arr[7][7] = {0};
    for (int t = 1; t <= M->terms; t++) {
        arr[M->data[t].row][M->data[t].col] = M->data[t].val;
    }

    printf("\n  %s（%d×%d，共 %d 个非零元素）:\n\n", title, M->rows, M->cols, M->terms);
    printf("       ");
    for (int j = 1; j <= M->cols; j++) printf("列%d   ", j);
    printf("\n    ┌─");
    for (int j = 1; j <= M->cols; j++) printf("──────");
    printf("─┐\n");
    for (int i = 1; i <= M->rows; i++) {
        printf("  行%d │", i);
        for (int j = 1; j <= M->cols; j++) {
            printf("  %4d", arr[i][j]);
        }
        printf("  │\n");
    }
    printf("    └─");
    for (int j = 1; j <= M->cols; j++) printf("──────");
    printf("─┘\n");
}

/* ─────────────────────────────────────────────
   可视化：打印三元组表
   ───────────────────────────────────────────── */
static void print_tsmatrix(const TSMatrix *M, const char *title)
{
    printf("\n  %s 三元组表（rows=%d, cols=%d, terms=%d）:\n\n",
           title, M->rows, M->cols, M->terms);
    printf("    ┌──────┬──────┬──────────┐\n");
    printf("    │  行  │  列  │   值     │\n");
    printf("    ├──────┼──────┼──────────┤\n");
    for (int t = 1; t <= M->terms; t++) {
        printf("    │  %2d  │  %2d  │  %6d  │\n",
               M->data[t].row, M->data[t].col, M->data[t].val);
    }
    printf("    └──────┴──────┴──────────┘\n");
}

/* ─────────────────────────────────────────────
   普通转置（非快速）：时间复杂度 O(cols × terms)
   ─────────────────────────────────────────────
   按"列"顺序扫描原矩阵，每次扫描一列，把该列所有元素
   按行顺序取出放入转置矩阵。
   ───────────────────────────────────────────── */
static void transpose_naive(const TSMatrix *M, TSMatrix *T)
{
    T->rows  = M->cols;
    T->cols  = M->rows;
    T->terms = M->terms;

    if (M->terms == 0) return;

    int q = 1;   /* 转置矩阵三元组下标 */
    for (int col = 1; col <= M->cols; col++) {
        /* 扫描整个原始三元组表，找 col 列的元素 */
        for (int p = 1; p <= M->terms; p++) {
            if (M->data[p].col == col) {
                T->data[q].row = M->data[p].col;
                T->data[q].col = M->data[p].row;
                T->data[q].val = M->data[p].val;
                q++;
            }
        }
    }
}

/* ─────────────────────────────────────────────
   快速转置（Fast Transpose）：时间复杂度 O(cols + terms)
   ─────────────────────────────────────────────
   利用两个辅助数组：
     num[col]  : 原矩阵第 col 列非零元素个数
     cpot[col] : 该列元素在转置矩阵中的起始位置
   ───────────────────────────────────────────── */
static void transpose_fast(const TSMatrix *M, TSMatrix *T)
{
    T->rows  = M->cols;
    T->cols  = M->rows;
    T->terms = M->terms;

    if (M->terms == 0) return;

    int num[MAXTERMS]  = {0};
    int cpot[MAXTERMS] = {0};

    /* 第一步：统计每列非零元素个数 */
    for (int t = 1; t <= M->terms; t++) {
        num[M->data[t].col]++;
    }

    /* 第二步：计算每列在 T 中的起始位置（前缀和） */
    cpot[1] = 1;
    for (int col = 2; col <= M->cols; col++) {
        cpot[col] = cpot[col - 1] + num[col - 1];
    }

    /* 第三步：遍历原三元组表，依次放入 T */
    for (int t = 1; t <= M->terms; t++) {
        int col = M->data[t].col;
        int q   = cpot[col];
        T->data[q].row = M->data[t].col;
        T->data[q].col = M->data[t].row;
        T->data[q].val = M->data[t].val;
        cpot[col]++;   /* 该列下一个元素存放位置后移 */
    }
}

/* ─────────────────────────────────────────────
   可视化快速转置的辅助数组
   ───────────────────────────────────────────── */
static void print_fast_transpose_detail(const TSMatrix *M)
{
    int num[MAXTERMS]  = {0};
    int cpot[MAXTERMS] = {0};

    for (int t = 1; t <= M->terms; t++) num[M->data[t].col]++;
    cpot[1] = 1;
    for (int col = 2; col <= M->cols; col++) cpot[col] = cpot[col-1] + num[col-1];

    printf("\n  快速转置辅助数组（按原矩阵列编号 1~%d）:\n\n", M->cols);
    printf("    ┌──────");
    for (int c = 1; c <= M->cols; c++) printf("┬──────");
    printf("┐\n");
    printf("    │ 列号 ");
    for (int c = 1; c <= M->cols; c++) printf("│  %-3d ", c);
    printf("│\n");
    printf("    ├──────");
    for (int c = 1; c <= M->cols; c++) printf("┼──────");
    printf("┤\n");
    printf("    │ num  ");
    for (int c = 1; c <= M->cols; c++) printf("│  %-3d ", num[c]);
    printf("│  (每列非零元个数)\n");
    printf("    ├──────");
    for (int c = 1; c <= M->cols; c++) printf("┼──────");
    printf("┤\n");
    printf("    │ cpot ");
    for (int c = 1; c <= M->cols; c++) printf("│  %-3d ", cpot[c]);
    printf("│  (该列在T中的起始位置)\n");
    printf("    └──────");
    for (int c = 1; c <= M->cols; c++) printf("┴──────");
    printf("┘\n");
}

/* ─────────────────────────────────────────────
   优化版快速转置：使用更少的辅助存储空间
   ─────────────────────────────────────────────
   普通快速转置需要两个大小为 cols+1 的辅助数组（num 和 cpot）。
   优化方法：只保留 cpot 数组（起始位置），num 数组的统计
   与 cpot 的计算合并，然后在转置过程中直接用 cpot 回退
   （利用 cpot[col]-num[col] 还原起始位置），
   从而只用一个辅助数组（cpot）完成快速转置。
   ───────────────────────────────────────────── */
static void transpose_fast_optimized(const TSMatrix *M, TSMatrix *T)
{
    T->rows  = M->cols;
    T->cols  = M->rows;
    T->terms = M->terms;

    if (M->terms == 0) return;

    /* 只申请一个大小为 cols+1 的数组 */
    int *cpot = (int *)calloc(M->cols + 1, sizeof(int));

    /* 步骤1：用 cpot 先统计每列非零元素个数（复用为 num）*/
    for (int t = 1; t <= M->terms; t++) cpot[M->data[t].col]++;

    /* 步骤2：原地转化为起始位置（后缀扫描）*/
    /* 先把 cpot[col] 变成"本列在T中的结束位置+1"，从后往前做前缀和 */
    {
        int acc = M->terms + 1;
        for (int c = M->cols; c >= 1; c--) {
            acc -= cpot[c];
            cpot[c] = acc;   /* cpot[c] 现在是第 c 列在 T 中的起始位置 */
        }
    }

    /* 步骤3：遍历原三元组表，依次放入 T */
    for (int t = 1; t <= M->terms; t++) {
        int col = M->data[t].col;
        int q   = cpot[col];
        T->data[q].row = M->data[t].col;
        T->data[q].col = M->data[t].row;
        T->data[q].val = M->data[t].val;
        cpot[col]++;
    }

    free(cpot);
}

/* ─────────────────────────────────────────────
   时间复杂度分析与对比
   ───────────────────────────────────────────── */
static void complexity_analysis(int rows, int cols, int terms)
{
    printf("\n┌──────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│                       时间复杂度与空间复杂度对比分析                        │\n");
    printf("├──────────────────────┬──────────────────────────────────────────────────────┤\n");
    printf("│      算法            │   说明                                               │\n");
    printf("├──────────────────────┼──────────────────────────────────────────────────────┤\n");
    printf("│ 普通转置（非快速）   │ 时间: O(cols × terms)                               │\n");
    printf("│                      │ 空间: O(1)（不含输出）                              │\n");
    printf("│                      │ 适合: terms ≪ cols，否则退化为 O(cols²)             │\n");
    printf("├──────────────────────┼──────────────────────────────────────────────────────┤\n");
    printf("│ 快速转置             │ 时间: O(cols + terms)                               │\n");
    printf("│                      │ 空间: O(cols)（两个辅助数组 num, cpot）             │\n");
    printf("│                      │ 优点: 线性时间，一次扫描完成                        │\n");
    printf("├──────────────────────┼──────────────────────────────────────────────────────┤\n");
    printf("│ 优化快速转置         │ 时间: O(cols + terms)（同快速转置）                 │\n");
    printf("│                      │ 空间: O(cols)（只用 1 个辅助数组，常数因子减半）    │\n");
    printf("│                      │ 优点: 减少约一半辅助存储，常数项更小               │\n");
    printf("└──────────────────────┴──────────────────────────────────────────────────────┘\n");

    printf("\n  当前示例参数: rows=%d, cols=%d, terms=%d\n\n", rows, cols, terms);

    long naive_ops = (long)cols * terms;
    long fast_ops  = cols + terms;

    printf("  操作次数估算:\n");
    printf("    普通转置: cols × terms = %d × %d = %ld 次\n", cols, terms, naive_ops);
    printf("    快速转置: cols + terms = %d + %d = %ld 次\n", cols, terms, fast_ops);
    if (fast_ops > 0 && naive_ops > 0) {
        printf("    加速比   : ≈ %.1f 倍\n", (double)naive_ops / fast_ops);
    }

    printf("\n  当 terms = rows×cols（矩阵满）时:\n");
    printf("    普通转置 → O(cols × rows×cols) = O(rows×cols²)\n");
    printf("    快速转置 → O(cols + rows×cols) = O(rows×cols)（仍为线性！）\n");

    printf("\n  结论：\n");
    printf("    • 快速转置和优化快速转置均比普通转置效率更高\n");
    printf("    • 当稀疏度高（terms << rows×cols）时，两者均表现优秀\n");
    printf("    • 优化版在内存敏感场景下更具优势（减少辅助数组）\n\n");
}

/* 验证两个转置矩阵是否相同 */
static int verify_equal(const TSMatrix *A, const TSMatrix *B)
{
    if (A->rows != B->rows || A->cols != B->cols || A->terms != B->terms)
        return 0;
    for (int t = 1; t <= A->terms; t++) {
        if (A->data[t].row != B->data[t].row ||
            A->data[t].col != B->data[t].col ||
            A->data[t].val != B->data[t].val)
            return 0;
    }
    return 1;
}

/* ─────────────────────────────────────────────
   主函数
   ───────────────────────────────────────────── */
int main(void)
{
    /* 示例稀疏矩阵（6×6，稀疏） */
    int arr[6][6] = {
        { 0, 12,  9,  0,  0,  0},
        { 0,  0,  0,  0,  0,  0},
        {-3,  0,  0,  0,  0, 14},
        { 0,  0, 24,  0, 18,  0},
        {15,  0,  0, -7,  0,  0},
        { 0,  0, -4,  0,  0,  0}
    };

    TSMatrix M, T_naive, T_fast, T_opt;

    printf("\n");
    printf("*****************************************************************\n");
    printf("*              综合实验：稀疏矩阵转置算法                         *\n");
    printf("*****************************************************************\n");

    /* 建立原始矩阵 */
    build_tsmatrix(&M, arr, 6, 6);

    /* ── 展示原始矩阵 ── */
    print_matrix_2d(&M, "原始矩阵 M");
    print_tsmatrix(&M, "原始矩阵 M");

    /* ── 任务1：快速转置 ── */
    printf("\n=================================================================\n");
    printf("【任务1】快速稀疏矩阵转置算法（Fast Transpose）\n");
    printf("=================================================================\n");
    printf("\n  算法思路:\n");
    printf("    ① 统计原矩阵每列非零元个数 → num[]\n");
    printf("    ② 计算每列在转置矩阵中的起始位置 → cpot[] = 前缀和\n");
    printf("    ③ 扫描原三元组表，按 cpot[col] 直接放入转置矩阵\n");

    print_fast_transpose_detail(&M);
    transpose_fast(&M, &T_fast);
    print_matrix_2d(&T_fast, "转置矩阵 T（快速转置）");
    print_tsmatrix(&T_fast, "转置矩阵 T（快速转置）");

    /* ── 任务2：优化快速转置 ── */
    printf("\n=================================================================\n");
    printf("【任务2】优化快速转置（更少存储空间）\n");
    printf("=================================================================\n");
    printf("\n  优化思路:\n");
    printf("    • 普通快速转置使用 num[] 和 cpot[] 两个辅助数组，共 2×cols 空间\n");
    printf("    • 优化版将 num 的统计与 cpot 的计算合并到一个数组中\n");
    printf("    • 通过从后往前的前缀和扫描，只用 1 个 cpot[] 数组即可\n");
    printf("    • 辅助空间从 2×cols 降至 cols（减少约 50%%）\n\n");

    transpose_fast_optimized(&M, &T_opt);
    print_matrix_2d(&T_opt, "转置矩阵 T（优化快速转置）");
    print_tsmatrix(&T_opt, "转置矩阵 T（优化快速转置）");

    /* 验证三种转置结果一致 */
    transpose_naive(&M, &T_naive);
    if (verify_equal(&T_fast, &T_naive) && verify_equal(&T_opt, &T_naive)) {
        printf("\n  ✓ 验证通过：三种转置算法结果完全一致\n");
    } else {
        printf("\n  ✗ 验证失败：结果不一致！\n");
    }

    /* ── 任务3：时间复杂度分析 ── */
    printf("\n=================================================================\n");
    printf("【任务3】时间复杂度分析与非快速算法对比\n");
    printf("=================================================================\n");

    printf("\n  非快速转置（普通转置）三元组结果:\n");
    print_matrix_2d(&T_naive, "转置矩阵 T（普通转置）");
    print_tsmatrix(&T_naive, "转置矩阵 T（普通转置）");

    complexity_analysis(M.rows, M.cols, M.terms);

    printf("*****************************************************************\n");
    printf("*                   稀疏矩阵转置演示完毕                          *\n");
    printf("*****************************************************************\n\n");

    return 0;
}
