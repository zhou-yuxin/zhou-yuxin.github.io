#ifndef BUDDY_H
#define BUDDY_H

#include <avl.h>
#include <stdlib.h>

// 一个伙伴分配器实例
struct buddy
{
    // 根节点数量
    size_t root_count;
    // 层数
    size_t layer_count;
    // 各个层
    struct avl_node** layers;
    // 最底层的单元大小
    size_t min_item_sz;
};

// 分配失败时的返回值
#define BUDDY_ALLOCATE_FAILED ((size_t)(-1))

// 初始化一个buddy实例，
//    buddy: 一个buddy的指针
//    capacity: 要管理的空间的大小
//    min_item_sz: 最小的分配单元的大小
//    max_item_sz: 最大的分配单元的大小
// 成功返回1，失败返回0。
int buddy_init(struct buddy* buddy, size_t capacity, size_t min_item_sz, size_t max_item_sz);

// 销毁一个buddy实例，即释放该buddy占用的动态内存，但不负责buddy本身的释放
//    buddy: 一个buddy的指针
// 成功返回1，失败返回0。
int buddy_deinit(struct buddy* buddy);

// 从buddy中分配一个单元
//    buddy: 一个buddy的指针
//    size: 单元大小
// 成功返回单元在空间中的偏移量，失败返回BUDDY_ALLOCATE_FAILED。
size_t buddy_allocate(struct buddy* buddy, size_t size);

// 从buddy中分配一个单元
//    buddy: 一个buddy的指针
//    addr: 单元的起始位置，用单元在空间中的偏移量表示
//    size: 单元大小
// 成功返回addr，失败返回BUDDY_ALLOCATE_FAILED。
size_t buddy_allocate_at(struct buddy* buddy, size_t addr, size_t size);

// 释放一个单元回buddy中
//    buddy: 一个buddy的指针
//    addr: 单元的起始位置，用单元在空间中的偏移量表示
//    size: 单元大小
// 成功返回1，失败返回0。
int buddy_free(struct buddy* buddy, size_t addr, size_t size);

#endif