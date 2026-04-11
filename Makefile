# 数据结构综合实验 Makefile
#
# 使用说明：
#   make all       编译全部项目
#   make run1      运行 Part1（分块查找）
#   make run2      运行 Part2（图路径搜索）
#   make run3      运行 Part3（稀疏矩阵转置）
#   make test1     Part1 自动运行测试用例（非交互）
#   make test2     Part2 自动运行测试用例（非交互）
#   make test3     Part3 自动运行测试用例（非交互）
#   make clean     清除编译产物和输出文件

CC      = gcc
CFLAGS  = -Wall -Wextra -g -std=c99

.PHONY: all clean run1 run2 run3 test1 test2 test3

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

# 非交互式测试（选项3=测试用例，然后选项0=退出）
test1: part1/block_search
	@echo "=============================="
	@echo " 测试：验证实验 - 分块查找"
	@echo "=============================="
	@echo -e "3\n0" | ./part1/block_search

test2: part2/graph_path
	@echo "=============================="
	@echo " 测试：设计实验 - 图路径搜索"
	@echo "=============================="
	@echo -e "3\n0" | ./part2/graph_path

test3: part3/sparse_matrix
	@echo "=============================="
	@echo " 测试：综合实验 - 稀疏矩阵转置"
	@echo "=============================="
	@echo -e "3\n0" | ./part3/sparse_matrix

clean:
	rm -f part1/block_search part2/graph_path part3/sparse_matrix
	rm -f part1-output.txt part2-output.txt part3-output.txt
