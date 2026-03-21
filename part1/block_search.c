/*
 * 验证实验：分块查找（索引顺序查找）
 *
 * 顺序表: 8,14,6,9,10,22,34,18,19,31,40,38,54,66,46,71,78,68,80,85,100,94,88,96,87
 * 块长: 5，共 5 块
 * 查找关键字: 46
 *
 * 任务：
 *   1. 图形化展示分块查找过程（顺序查找 + 折半查找索引）
 *   2. 自动划分块、生成索引表的函数
 *   3. 将块改为链式（单链表）存储并实现查找
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ─────────────────────────────────────────────
   公共定义
   ───────────────────────────────────────────── */
#define N          25   /* 顺序表元素总数   */
#define BLOCK_SIZE  5   /* 每块元素个数     */
#define NUM_BLOCKS  5   /* 块总数           */

/* 待查顺序表 */
int a[N] = {8,14,6,9,10, 22,34,18,19,31, 40,38,54,66,46, 71,78,68,80,85, 100,94,88,96,87};

/* 索引表项 */
typedef struct {
    int max_val;   /* 该块中的最大关键字 */
    int start;     /* 该块在顺序表中的起始下标（0-based） */
} IndexEntry;

IndexEntry idx[NUM_BLOCKS];   /* 全局索引表 */

/* ─────────────────────────────────────────────
   辅助：可视化函数
   ───────────────────────────────────────────── */

/* 打印分隔线 */
static void print_line(int width)
{
    for (int i = 0; i < width; i++) putchar('-');
    putchar('\n');
}

/* 图形化展示顺序表及分块结构 */
static void print_array_blocks(void)
{
    printf("\n┌─────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│                         顺序表分块结构可视化                               │\n");
    printf("└─────────────────────────────────────────────────────────────────────────────┘\n");

    /* 打印下标行（1-based 显示，对应用户习惯） */
    printf("  下标 │");
    for (int i = 0; i < N; i++) {
        if (i > 0 && i % BLOCK_SIZE == 0) printf(" ║");
        printf(" %3d", i + 1);
    }
    printf("\n");
    print_line(100);

    /* 打印元素行 */
    printf("  元素 │");
    for (int i = 0; i < N; i++) {
        if (i > 0 && i % BLOCK_SIZE == 0) printf(" ║");
        printf(" %3d", a[i]);
    }
    printf("\n");
    print_line(100);

    /* 打印块标号行 */
    printf("  块号 │");
    for (int b = 0; b < NUM_BLOCKS; b++) {
        /* 每块占 BLOCK_SIZE*4 列 */
        if (b > 0) printf(" ║");
        int mid = BLOCK_SIZE * 4 / 2;
        for (int k = 0; k < mid - 2; k++) putchar(' ');
        printf("第%d块", b + 1);
        for (int k = 0; k < mid - 3; k++) putchar(' ');
    }
    printf("\n\n");

    /* 每块详情 */
    printf("  %-6s  %-20s  %-10s  %-10s\n", "块号", "元素列表", "最大值", "起始下标");
    print_line(55);
    for (int b = 0; b < NUM_BLOCKS; b++) {
        printf("  第%d块   [", b + 1);
        for (int i = 0; i < BLOCK_SIZE; i++) {
            printf("%3d%s", a[b * BLOCK_SIZE + i], i < BLOCK_SIZE - 1 ? "," : "");
        }
        printf("]   max=%-4d  start=%-4d\n", idx[b].max_val, idx[b].start + 1);
    }
    printf("\n");
}

/* 图形化展示索引表 */
static void print_index_table(void)
{
    printf("\n┌────────────────────────────────────┐\n");
    printf("│           索引表                   │\n");
    printf("├──────┬──────────┬──────────────────┤\n");
    printf("│ 块号 │ 最大关键字 │ 起始下标（1-based）│\n");
    printf("├──────┼──────────┼──────────────────┤\n");
    for (int b = 0; b < NUM_BLOCKS; b++) {
        printf("│  %d   │    %-5d │        %-4d      │\n",
               b + 1, idx[b].max_val, idx[b].start + 1);
    }
    printf("└──────┴──────────┴──────────────────┘\n\n");
}

/* ─────────────────────────────────────────────
   任务2：自动划分块并产生索引
   ───────────────────────────────────────────── */
void build_index(void)
{
    printf("=================================================================\n");
    printf("【任务2】自动划分块并生成索引表\n");
    printf("=================================================================\n");

    for (int b = 0; b < NUM_BLOCKS; b++) {
        int start = b * BLOCK_SIZE;
        int max   = a[start];
        for (int i = start + 1; i < start + BLOCK_SIZE; i++) {
            if (a[i] > max) max = a[i];
        }
        idx[b].max_val = max;
        idx[b].start   = start;
    }

    print_array_blocks();
    print_index_table();
}

/* ─────────────────────────────────────────────
   任务1-A：顺序查找索引，再块内顺序查找
   ───────────────────────────────────────────── */
int block_search_sequential_index(int key)
{
    printf("\n─────────────────────────────────────────────────────────────────\n");
    printf("【任务1-A】分块查找（索引用顺序查找）  关键字 key = %d\n", key);
    printf("─────────────────────────────────────────────────────────────────\n");

    /* 第一步：在索引表中顺序查找块 */
    printf("\n  ● 第一步：在索引表中顺序查找\n");
    int block_id = -1;
    for (int b = 0; b < NUM_BLOCKS; b++) {
        printf("    比较 key(%d) <= idx[%d].max(%d)? ", key, b + 1, idx[b].max_val);
        if (key <= idx[b].max_val) {
            printf("是 → 关键字在第 %d 块\n", b + 1);
            block_id = b;
            break;
        } else {
            printf("否\n");
        }
    }

    if (block_id == -1) {
        printf("  ✗ 未找到对应块，查找失败\n\n");
        return -1;
    }

    /* 第二步：在对应块中顺序查找 */
    printf("\n  ● 第二步：在第 %d 块（下标 %d~%d）中顺序查找\n",
           block_id + 1,
           idx[block_id].start + 1,
           idx[block_id].start + BLOCK_SIZE);

    printf("    块内元素：[");
    for (int i = 0; i < BLOCK_SIZE; i++) {
        int pos = idx[block_id].start + i;
        printf("%d%s", a[pos], i < BLOCK_SIZE - 1 ? ", " : "");
    }
    printf("]\n");

    for (int i = 0; i < BLOCK_SIZE; i++) {
        int pos = idx[block_id].start + i;
        printf("    比较 a[%d]=%d == key(%d)? ", pos + 1, a[pos], key);
        if (a[pos] == key) {
            printf("是 → 查找成功！位置 = %d\n", pos + 1);
            printf("\n  ✓ 结果：关键字 %d 在顺序表第 %d 个位置（第 %d 块第 %d 个元素）\n\n",
                   key, pos + 1, block_id + 1, i + 1);
            return pos + 1;
        } else {
            printf("否\n");
        }
    }

    printf("  ✗ 块内未找到，查找失败\n\n");
    return -1;
}

/* ─────────────────────────────────────────────
   任务1-B：折半查找索引，再块内顺序查找
   ───────────────────────────────────────────── */
int block_search_binary_index(int key)
{
    printf("\n─────────────────────────────────────────────────────────────────\n");
    printf("【任务1-B】分块查找（索引用折半查找）  关键字 key = %d\n", key);
    printf("─────────────────────────────────────────────────────────────────\n");

    /* 第一步：折半查找索引表 */
    printf("\n  ● 第一步：在索引表中折半查找\n");

    int low = 0, high = NUM_BLOCKS - 1, block_id = -1;
    int step = 1;

    while (low <= high) {
        int mid = (low + high) / 2;
        printf("    [第%d次] low=%d, high=%d, mid=%d, idx[mid].max=%d → ",
               step++, low + 1, high + 1, mid + 1, idx[mid].max_val);

        if (key == idx[mid].max_val) {
            printf("key == max, 在第 %d 块\n", mid + 1);
            block_id = mid;
            break;
        } else if (key < idx[mid].max_val) {
            printf("key < max, 向左\n");
            high = mid - 1;
            block_id = mid;   /* 可能在 mid 块 */
        } else {
            printf("key > max, 向右\n");
            low = mid + 1;
        }
    }

    /* 折半结束后若 block_id 仍是未完全收敛, 取 low */
    if (block_id == -1) {
        if (low < NUM_BLOCKS) {
            block_id = low;
        } else {
            printf("  ✗ 超出索引范围，查找失败\n\n");
            return -1;
        }
    }

    printf("    → 确定关键字可能在第 %d 块\n", block_id + 1);

    /* 第二步：块内顺序查找 */
    printf("\n  ● 第二步：在第 %d 块（下标 %d~%d）中顺序查找\n",
           block_id + 1,
           idx[block_id].start + 1,
           idx[block_id].start + BLOCK_SIZE);

    printf("    块内元素：[");
    for (int i = 0; i < BLOCK_SIZE; i++) {
        int pos = idx[block_id].start + i;
        printf("%d%s", a[pos], i < BLOCK_SIZE - 1 ? ", " : "");
    }
    printf("]\n");

    for (int i = 0; i < BLOCK_SIZE; i++) {
        int pos = idx[block_id].start + i;
        printf("    比较 a[%d]=%d == key(%d)? ", pos + 1, a[pos], key);
        if (a[pos] == key) {
            printf("是 → 查找成功！位置 = %d\n", pos + 1);
            printf("\n  ✓ 结果：关键字 %d 在顺序表第 %d 个位置（第 %d 块第 %d 个元素）\n\n",
                   key, pos + 1, block_id + 1, i + 1);
            return pos + 1;
        } else {
            printf("否\n");
        }
    }

    printf("  ✗ 块内未找到，查找失败\n\n");
    return -1;
}

/* ─────────────────────────────────────────────
   任务3：链式存储（单链表）分块查找
   ───────────────────────────────────────────── */

/* 链表结点 */
typedef struct Node {
    int data;
    struct Node *next;
} Node;

/* 链式块的索引项 */
typedef struct {
    int    max_val;   /* 块最大关键字      */
    Node  *head;      /* 指向该块链表的头结点 */
    int    size;      /* 块内元素个数      */
} LinkedBlock;

LinkedBlock linked_blocks[NUM_BLOCKS];

/* 构建链式分块 */
void build_linked_blocks(void)
{
    printf("=================================================================\n");
    printf("【任务3】链式（单链表）存储分块\n");
    printf("=================================================================\n");

    for (int b = 0; b < NUM_BLOCKS; b++) {
        linked_blocks[b].max_val = 0;
        linked_blocks[b].head    = NULL;
        linked_blocks[b].size    = 0;

        /* 尾插法建链表，保持块内原有顺序 */
        Node *tail = NULL;
        for (int i = b * BLOCK_SIZE; i < (b + 1) * BLOCK_SIZE; i++) {
            Node *p = (Node *)malloc(sizeof(Node));
            p->data = a[i];
            p->next = NULL;
            if (linked_blocks[b].head == NULL) {
                linked_blocks[b].head = p;
            } else {
                tail->next = p;
            }
            tail = p;

            if (a[i] > linked_blocks[b].max_val)
                linked_blocks[b].max_val = a[i];
            linked_blocks[b].size++;
        }
    }

    /* 可视化 */
    printf("\n  链式分块结构：\n\n");
    printf("  索引表\n");
    printf("  ┌────────┬──────────────────────────────────────────────────┐\n");
    printf("  │ 最大值 │  链式块内容                                      │\n");
    printf("  ├────────┼──────────────────────────────────────────────────┤\n");
    for (int b = 0; b < NUM_BLOCKS; b++) {
        printf("  │  %-5d │  HEAD", linked_blocks[b].max_val);
        Node *p = linked_blocks[b].head;
        while (p) {
            printf(" → [%d]", p->data);
            p = p->next;
        }
        printf(" → NULL\n");
    }
    printf("  └────────┴──────────────────────────────────────────────────┘\n\n");
}

/* 链式分块查找（索引用顺序查找） */
int linked_block_search(int key)
{
    printf("  ● 链式分块查找  key = %d\n\n", key);

    /* 第一步：顺序查找索引 */
    printf("    第一步：顺序查找索引表\n");
    int block_id = -1;
    for (int b = 0; b < NUM_BLOCKS; b++) {
        printf("      key(%d) <= linked_blocks[%d].max(%d)? ",
               key, b + 1, linked_blocks[b].max_val);
        if (key <= linked_blocks[b].max_val) {
            printf("是 → 进入第 %d 块\n", b + 1);
            block_id = b;
            break;
        } else {
            printf("否\n");
        }
    }

    if (block_id == -1) {
        printf("    ✗ 未找到对应块，查找失败\n\n");
        return -1;
    }

    /* 第二步：遍历链表查找 */
    printf("\n    第二步：遍历第 %d 块链表\n", block_id + 1);
    Node *p = linked_blocks[block_id].head;
    int pos = block_id * BLOCK_SIZE + 1;
    while (p) {
        printf("      检查结点 [%d] == key(%d)? ", p->data, key);
        if (p->data == key) {
            printf("是 → 找到！逻辑位置 = %d\n", pos);
            printf("\n  ✓ 结果：关键字 %d 在链式存储第 %d 块的第 %d 个结点，逻辑位置 %d\n\n",
                   key, block_id + 1, pos - block_id * BLOCK_SIZE, pos);
            return pos;
        }
        printf("否\n");
        p = p->next;
        pos++;
    }

    printf("    ✗ 块内链表未找到，查找失败\n\n");
    return -1;
}

/* 释放链式块内存 */
void free_linked_blocks(void)
{
    for (int b = 0; b < NUM_BLOCKS; b++) {
        Node *p = linked_blocks[b].head;
        while (p) {
            Node *tmp = p->next;
            free(p);
            p = tmp;
        }
        linked_blocks[b].head = NULL;
    }
}

/* ─────────────────────────────────────────────
   主函数
   ───────────────────────────────────────────── */
int main(void)
{
    int key = 46;

    printf("\n");
    printf("*****************************************************************\n");
    printf("*         验证实验：分块查找（索引顺序查找）                      *\n");
    printf("*  顺序表共 %d 个元素，块长 %d，共 %d 块，查找关键字 %d        *\n",
           N, BLOCK_SIZE, NUM_BLOCKS, key);
    printf("*****************************************************************\n\n");

    /* ── 任务2：自动建立索引 ── */
    build_index();

    /* ── 任务1-A：顺序查找索引的分块查找 ── */
    printf("=================================================================\n");
    printf("【任务1】分块查找过程演示\n");
    printf("=================================================================\n");
    block_search_sequential_index(key);

    /* ── 任务1-B：折半查找索引的分块查找 ── */
    block_search_binary_index(key);

    /* ── 任务3：链式存储分块查找 ── */
    build_linked_blocks();
    linked_block_search(key);
    free_linked_blocks();

    printf("*****************************************************************\n");
    printf("*                     分块查找演示完毕                           *\n");
    printf("*****************************************************************\n\n");

    return 0;
}
