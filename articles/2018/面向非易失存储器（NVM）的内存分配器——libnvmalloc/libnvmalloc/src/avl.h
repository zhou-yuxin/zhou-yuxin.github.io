#ifndef AVL_H
#define AVL_H

#include <stdlib.h>

// AVL树节点
struct avl_node
{
    void* val;
    ssize_t height;
    struct avl_node* left;
    struct avl_node* right;
};

// 向AVL树插入一个值，使用指定函数进行值与值之间的比较
//   root: AVL树根节点
//   val: 待插入的值
//   comparator: 自定义比较函数：如果v1<v2，返回-1；如果v1=v2，返回0；如果v1>v2，返回1。
//   返回：新的AVL树根节点
struct avl_node* avl_insert(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2));

// 从AVL树中删除一个值，使用指定函数进行值与值之间的比较
//   root: AVL树根节点
//   val: 待删除的值
//   comparator: 自定义比较函数：如果v1<v2，返回-1；如果v1=v2，返回0；如果v1>v2，返回1。
//   found: 如果找到，则置为1（应事先置为0）
//   返回：新的AVL树根节点
struct avl_node* avl_delete(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2), int* found);

// get 在AVL树中搜索指定值，使用指定函数进行值与值之间的比较
//   root: AVL树根节点
//   val: 待插入的值
//   comparator: 自定义比较函数：如果v1<v2，返回-1；如果v1=v2，返回0；如果v1>v2，返回1。
//   返回：如果站到该值，则返回包含该值的树节点，否则返回0
struct avl_node* avl_search(struct avl_node* root, void* val,
    int (*comparator)(const void* v1, const void* v2));

// 获取指定AVL树中节点总数
//   root: AVL树根节点
size_t avl_node_count(struct avl_node* root);

#endif