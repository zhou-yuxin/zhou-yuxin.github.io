#ifndef BMP4K_H
#define BMP4K_H

#include <stdlib.h>
#include <stdint.h>

// 一个bmp4k
struct bmp4k
{
    // 长度
    size_t len;
    // 第i位代表bits[i]中是否有0
    uint64_t has_0;
    // 第i位代表bits[i]中是否有1
    uint64_t has_1;
    // 共4096个bit
    uint64_t bits[64];
};

#define BMP4K_NOT_FOUND ((size_t)(-1))

// 初始化bmp4k
//   bmp4k: 一个bmp4k指针
//   len: 长度，1~4096
//   value_all: 把所有bit初始化为0或1
// 成功返回，失败返回0
int bmp4k_init(struct bmp4k* bmp4k, size_t len, int value_all);

// 查询指定位的值
//   bmp4k: 一个bmp4k指针
//   index: 位的编号
// 失败返回-1,成功返回1或0
int bmp4k_get(struct bmp4k* bmp4k, size_t index);

// 设置指定位的值
//   bmp4k: 一个bmp4k指针
//   index: 位的编号
//   value: 位的值
// 成功返回1, 失败返回0
int bmp4k_set(struct bmp4k* bmp4k, size_t index, int value);

// 判断是否存在1或0
//   bmp4k: 一个bmp4k指针
//   value: 位的值
// 存在返回1,不存在返回0
int bmp4k_exist(struct bmp4k* bmp4k, int value);

// 搜索一个值为value的位
//   bmp4k: 一个bmp4k指针
//   value: 位的值
// 存在则返回找到的位的编号，不存在则返回BMP4K_NOT_FOUND
size_t bmp4k_search(struct bmp4k* bmp4k, int value);

#endif
