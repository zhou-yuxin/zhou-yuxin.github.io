#ifndef NVMALLOC_H
#define NVMALLOC_H

#include <stdlib.h>

// 最小分配单元
#define NVMALLOC_MIN_ITEM_SIZE 64
// 最大分配单元
#define NVMALLOC_MAX_ITEM_SIZE (512<<20)        //512M
// 使用bitmap细分的最大单元
#define NVMALLOC_MAX_BMP_ITEM_SIZE (512<<10)    //512K
// 使用bitmap细分的最小chunk
#define NVMALLOC_MIN_BMP_CHUNK_SIZE (NVMALLOC_MIN_ITEM_SIZE * 4096) //256K
// 使用bitmap细分的最大chunk
#define NVMALLOC_MAX_BMP_CHUNK_SIZE (4<<20)     //4M
// 最小分配单元步长
#define NVMALLOC_MIN_ITEM_SIZE_DELTA 16

// 分配失败时的返回值
#define NVMALLOC_ALLOCATE_FAILED ((size_t)(-1))

// 一个nvmalloc分配器
struct nvmalloc
{
    // 对外的include中，内部结构是黑盒
    char dim[64];
};

// 初始化nvmalloc
//    nvmalloc: 一个nvmalloc指针
//    capacity: 管理的空间大小
// 成功返回1, 失败返回0
size_t nvmalloc_init(struct nvmalloc* nvmalloc, size_t capacity);

// 销毁一个nvmalloc（但不负责struct nvmalloc本身的释放）
//    nvmalloc: 一个nvmalloc指针
// 成功返回1, 失败返回0
int nvmalloc_deinit(struct nvmalloc* nvmalloc);

// 从管理空间中分配一块空闲区域
//    nvmalloc: 一个nvmalloc指针
//    size: 分配空间大小
// 成功返回地址，失败返回NVMALLOC_ALLOCATE_FAILED
size_t nvmalloc_malloc(struct nvmalloc* nvmalloc, size_t size);

// 在管理空间的指定位置分配一块区域
//    nvmalloc: 一个nvmalloc指针
//    addr: 地址
//    size: 分配空间大小
// 成功返回地址addr，失败返回NVMALLOC_ALLOCATE_FAILED
size_t nvmalloc_malloc_at(struct nvmalloc* nvmalloc, size_t addr, size_t size);

// 释放分配的空间
//    nvmalloc: 一个nvmalloc指针
//    addr: 地址
// 成功返回1, 失败返回0
int nvmalloc_free(struct nvmalloc* nvmalloc, size_t addr);

// 获取分配的可用空间大小
//    nvmalloc: 一个nvmalloc指针
//    addr: 地址
// 成功返回大小，失败返回0 
size_t nvmalloc_usable_size(struct nvmalloc* nvmalloc, size_t addr);

#endif