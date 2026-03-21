# 数据结构综合实验 Makefile

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99

.PHONY: all clean run1 run2 run3

all: part1/block_search part2/graph_path part3/sparse_matrix

part1/block_search: part1/block_search.c
	$(CC) $(CFLAGS) -o $@ $<

part2/graph_path: part2/graph_path.c
	$(CC) $(CFLAGS) -o $@ $<

part3/sparse_matrix: part3/sparse_matrix.c
	$(CC) $(CFLAGS) -o $@ $<

run1: part1/block_search
	@echo "=============================="
	@echo " 运行：验证实验 - 分块查找"
	@echo "=============================="
	./part1/block_search

run2: part2/graph_path
	@echo "=============================="
	@echo " 运行：设计实验 - 图路径搜索"
	@echo "=============================="
	./part2/graph_path

run3: part3/sparse_matrix
	@echo "=============================="
	@echo " 运行：综合实验 - 稀疏矩阵转置"
	@echo "=============================="
	./part3/sparse_matrix

clean:
	rm -f part1/block_search part2/graph_path part3/sparse_matrix
