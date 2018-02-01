#include <buddy.h>
#include <common.h>
#include <string.h>

// 计算n是2的几次方。如果n不是2的自然数幂，那么返回(size_t)(-1)
static size_t to_power(size_t n)
{
    size_t power;
    for(power = 0; n >> (power + 1); power++);
    if((1 << power) != n)
        return (size_t)(-1);
    return power;
}

// 比较两个void*的大小
static int plain_comparator(const void* v1, const void* v2)
{
    if(v1 < v2)
        return -1;
    if(v1 == v2)
        return 0;
    return 1;
}

int buddy_init(struct buddy* buddy, size_t capacity, size_t min_item_sz, size_t max_item_sz)
{
    assert(buddy);
    size_t power = to_power(max_item_sz / min_item_sz);
    if(power == (size_t)(-1) || (1 << power) * min_item_sz != max_item_sz)
        ERROR(0, 0, "param <max_item_sz = %lu> / param <min_item_sz = %lu> is not a power of 2!", max_item_sz, min_item_sz);
    buddy->layer_count = power + 1;
    buddy->root_count = capacity / max_item_sz;
    if(buddy->root_count == 0)
        ERROR(0, 0, "param <capacity = %lu> is not big enough!", capacity);
    buddy->min_item_sz = min_item_sz;
    size_t array_sz = sizeof(struct avl_node*) * buddy->layer_count;
    buddy->layers = malloc(array_sz);
    if(!buddy->layers)
        ERROR(0, 1, "malloc(%lu) failed, ", array_sz);
    memset(buddy->layers, 0, array_sz);
    struct avl_node** top_layer = buddy->layers + (buddy->layer_count - 1);
    for(size_t i = 0; i < buddy->root_count; i++)
        (*top_layer) = avl_insert((*top_layer), (void*)i, plain_comparator);
    return 1;
}

int buddy_deinit(struct buddy* buddy)
{
    assert(buddy);
    for(size_t i = 0; i < buddy->layer_count; i++)
    {
        struct avl_node* layer = buddy->layers[i];
        while(layer)
        {
            int found = 0;
            layer = avl_delete(layer, (void*)layer->val, plain_comparator, &found);
            assert(found);
        }
    }
    free(buddy->layers);
    return 1;
}

#define SIZE_TO_LAYER_INDEX(buddy, size, layer_index, fail_ret)                                                                                         \
    do                                                                                                                                                  \
    {                                                                                                                                                   \
        size_t power = to_power((size) / (buddy)->min_item_sz);                                                                                         \
        if(power == (size_t)(-1) || (1 << power) * (buddy)->min_item_sz != (size))                                                                      \
            ERROR((fail_ret), 0, "param <%s = %lu> / <(%s)->min_item_sz = %lu> is not a power of 2!", #size, (size), #buddy, (buddy)->min_item_sz);     \
        (layer_index) = power;                                                                                                                          \
        if((layer_index) >= (buddy)->layer_count)                                                                                                       \
            ERROR((fail_ret), 0, "param <%s = %lu> is too big!", #size, size);                                                                          \
    }                                                                                                                                                   \
    while(0)

// 从指定层取出一个节点。若没有，则向上层申请分裂，从上层分裂出来的两个节点中，一个会被加入当前层，一个会被返回
//    layer_index: 当前层号
// 返回一个整数，表示从上层的一个节点分裂出来的两个该层节点中，取出来的那个节点编号
// 失败返回(size_t)(-1)
static size_t get_a_item_from_layer(struct buddy* buddy, size_t layer_index)
{
    if(layer_index >= buddy->layer_count)
        return (size_t)(-1);
    struct avl_node** layer = buddy->layers + layer_index;
    if((*layer))
    {
        size_t item_index = (size_t)(*layer)->val;
        int found = 0;
        (*layer) = avl_delete((*layer), (void*)item_index, plain_comparator, &found);
        assert(found);
        return item_index;
    }
    size_t upper_item_index = get_a_item_from_layer(buddy, layer_index + 1);
    if(upper_item_index == (size_t)(-1))
        return (size_t)(-1);
    (*layer) = avl_insert((*layer), (void*)(upper_item_index * 2 + 1), plain_comparator);
    return upper_item_index * 2;
}

size_t buddy_allocate(struct buddy* buddy, size_t size)
{
    assert(buddy);
    size_t layer_index;
    SIZE_TO_LAYER_INDEX(buddy, size, layer_index, BUDDY_ALLOCATE_FAILED);
    size_t item_index = get_a_item_from_layer(buddy, layer_index);
    if(item_index == (size_t)(-1))
        ERROR(BUDDY_ALLOCATE_FAILED, 0, "no more space available for size = %lu!", size);
    return item_index * size;
}

#define ADDR_AND_SIZE_TO_ITEM_INDEX(addr, size, item_size, fail_ret)                                                        \
    do                                                                                                                      \
    {                                                                                                                       \
        (item_index) = (addr) / (size);                                                                                     \
        if((item_index) * (size) != (addr))                                                                                 \
            ERROR((fail_ret), 0, "param <%s = %lu> is not aligned by param <%s = %lu>!", #addr, (addr), #size, (size));     \
    }                                                                                                                       \
    while(0)

#define BUDDY_INDEX(item_index) ((item_index) % 2 ? (item_index) - 1 : (item_index) + 1)

// 从指定层取出指定节点，若无则向上层申请分裂，上层分裂得到当前层指定节点和其伙伴，把伙伴放入该层
//    layer_index: 当前层号
//    item_index: 想要的节点编号
// 成功返回1，失败返回0
static int get_the_item_from_layer(struct buddy* buddy, size_t layer_index, size_t item_index)
{
    if(layer_index >= buddy->layer_count)
        return 0;
    struct avl_node** layer = buddy->layers + layer_index;
    int found = 0;
    (*layer) = avl_delete((*layer), (void*)item_index, plain_comparator, &found);
    if(found)
        return 1;
    if(!get_the_item_from_layer(buddy, layer_index + 1, item_index / 2))
        return 0;
    (*layer) = avl_insert((*layer), (void*)BUDDY_INDEX(item_index), plain_comparator);
    return 1;
}

size_t buddy_allocate_at(struct buddy* buddy, size_t addr, size_t size)
{
    assert(buddy);
    size_t layer_index, item_index;
    SIZE_TO_LAYER_INDEX(buddy, size, layer_index, BUDDY_ALLOCATE_FAILED);
    ADDR_AND_SIZE_TO_ITEM_INDEX(addr, size, item_index, BUDDY_ALLOCATE_FAILED);
    if(!get_the_item_from_layer(buddy, layer_index, item_index))
        ERROR(BUDDY_ALLOCATE_FAILED, 0, "the given item (addr = %lu, size = %lu) is not available!", addr, size);
    return addr;
}

// 在某层释放指定元素
static void free_to_layer(struct buddy* buddy, size_t layer_index, size_t item_index)
{
    assert(layer_index < buddy->layer_count);
    struct avl_node** layer = buddy->layers + layer_index;
    assert(!avl_search((*layer), (void*)item_index, plain_comparator));
    if(layer_index < buddy->layer_count - 1)
    {
        int found = 0;
        (*layer) = avl_delete((*layer), (void*)BUDDY_INDEX(item_index), plain_comparator, &found);
        if(found)
        {
            free_to_layer(buddy, layer_index + 1, item_index / 2);
            return;
        }
    }
    (*layer) = avl_insert((*layer), (void*)item_index, plain_comparator);
}

int buddy_free(struct buddy* buddy, size_t addr, size_t size)
{
    assert(buddy);
    size_t layer_index, item_index;
    SIZE_TO_LAYER_INDEX(buddy, size, layer_index, 0);
    ADDR_AND_SIZE_TO_ITEM_INDEX(addr, size, item_index, 0);
    free_to_layer(buddy, layer_index, item_index);
    return 1;
}