#include <common.h>
#include <string.h>
#include <nvmalloc.h>

// 计算n是2的几次方。如果n不是2的自然数幂，那么返回(size_t)(-1)
static size_t to_power(size_t n)
{
    size_t power;
    for(power = 0; n >> (power + 1); power++);
    if((1 << power) != n)
        return (size_t)(-1);
    return power;
}

size_t nvmalloc_init(struct nvmalloc* nvmalloc, size_t capacity)
{
    assert(nvmalloc);
    size_t min_power = to_power(NVMALLOC_MIN_ITEM_SIZE);
    assert(min_power != (size_t)(-1));
    size_t max_power = to_power(NVMALLOC_MAX_ITEM_SIZE);
    assert(max_power != (size_t)(-1));
    size_t bmp_max_power = to_power(NVMALLOC_MAX_BMP_ITEM_SIZE);
    assert(bmp_max_power != (size_t)(-1));
    assert(min_power <= bmp_max_power && bmp_max_power <= max_power);
    size_t min_chunk_power = to_power(NVMALLOC_MIN_BMP_CHUNK_SIZE);
    assert(min_chunk_power != (size_t)(-1));
    size_t max_chunk_power = to_power(NVMALLOC_MAX_BMP_CHUNK_SIZE);
    assert(max_chunk_power != (size_t)(-1));
    assert(min_chunk_power <= max_chunk_power && max_chunk_power <= max_power);
    size_t min_item_delta_power = to_power(NVMALLOC_MIN_ITEM_SIZE_DELTA);
    assert(min_item_delta_power != (size_t)(-1));
    assert(min_item_delta_power <= min_power);
    nvmalloc->level_count = (bmp_max_power - min_power) * (NVMALLOC_MIN_ITEM_SIZE / NVMALLOC_MIN_ITEM_SIZE_DELTA) + 1;
    nvmalloc->levels = malloc(sizeof(struct nvmalloc_level) * nvmalloc->level_count);
    if(!nvmalloc->levels)
        ERROR(0, 1, "malloc(sizeof(struct nvmalloc_level) * %lu) failed!", nvmalloc->level_count);
    memset(nvmalloc->levels, 0, sizeof(struct nvmalloc_level) * nvmalloc->level_count);
    nvmalloc->map_count = capacity / NVMALLOC_MIN_BMP_CHUNK_SIZE;
    nvmalloc->chunk_maps = malloc(sizeof(void*) * nvmalloc->map_count);
    if(!nvmalloc->chunk_maps)
        ERROR(0, 1, "malloc(sizeof(void*) * %lu) failed!", nvmalloc->map_count);
    memset(nvmalloc->chunk_maps, 0, sizeof(void*) * nvmalloc->map_count);
    int ret = buddy_init(&(nvmalloc->buddy), capacity, NVMALLOC_MIN_BMP_CHUNK_SIZE, NVMALLOC_MAX_ITEM_SIZE);
    if(!ret)
        ERROR(0, 0, "buddy_init(&(nvmalloc->buddy), %lu, %d, %d) failed!", capacity, NVMALLOC_MIN_BMP_CHUNK_SIZE, NVMALLOC_MAX_ITEM_SIZE);
    return 1;
}

int nvmalloc_deinit(struct nvmalloc* nvmalloc)
{
    assert(nvmalloc);
    if(!buddy_deinit(&(nvmalloc->buddy)))
        ERROR(0, 0, "buddy_deinit(&(nvmalloc->buddy)) failed!");
    for(size_t i = 0; i < nvmalloc->level_count; i++)
    {
        struct nvmalloc_chunk* chunk = nvmalloc->levels[i].avails;
        while(chunk)
        {
            struct nvmalloc_chunk* next = chunk->next;
            free(chunk);
            chunk = next;
        }
        chunk = nvmalloc->levels[i].unavails;
        while(chunk)
        {
            struct nvmalloc_chunk* next = chunk->next;
            free(chunk);
            chunk = next;
        }
    }
    free(nvmalloc->levels);
    free(nvmalloc->chunk_maps);
    return 1;
}

// 把大小变成chunk等级和分配单元大小
#define PARSE_SIZE(size, level, item_size)                                                  \
    do                                                                                      \
    {                                                                                       \
        if((size) <= NVMALLOC_MIN_ITEM_SIZE)                                                \
        {                                                                                   \
            (level) = 0;                                                                    \
            (item_size) = NVMALLOC_MIN_ITEM_SIZE;                                           \
            break;                                                                          \
        }                                                                                   \
        size_t _step = 0;                                                                   \
        while(((size_t)NVMALLOC_MIN_ITEM_SIZE << (_step + 1)) - 1 < (size))                 \
            _step++;                                                                        \
        size_t _base = ((size_t)NVMALLOC_MIN_ITEM_SIZE << _step);                           \
        assert(_base <= (size));                                                            \
        size_t _delta = (size_t)NVMALLOC_MIN_ITEM_SIZE_DELTA << _step;                      \
        size_t _index = ((size) - _base + _delta - 1) / _delta;                             \
        (level) = _step * (NVMALLOC_MIN_ITEM_SIZE / NVMALLOC_MIN_ITEM_SIZE_DELTA) + _index; \
        (item_size) = _base + _index * _delta;                                              \
    }                                                                                       \
    while(0)

// 给nvmalloc->chunk_maps标记
#define MARK_CHUNK_MAPS(nvmalloc, addr, len, mark)                      \
    do                                                                  \
    {                                                                   \
        assert((addr) % NVMALLOC_MIN_BMP_CHUNK_SIZE == 0);              \
        size_t _map_index = (addr) / NVMALLOC_MIN_BMP_CHUNK_SIZE;       \
        assert((len) % NVMALLOC_MIN_BMP_CHUNK_SIZE == 0);               \
        size_t _map_count = (len) / NVMALLOC_MIN_BMP_CHUNK_SIZE;        \
        assert(_map_count);                                             \
        assert(_map_index + _map_count <= (nvmalloc)->map_count);       \
        for(size_t _i = 0; _i < _map_count; _i++)                       \
            (nvmalloc)->chunk_maps[_map_index + _i] = (void*)(mark);    \
    }                                                                   \
    while(0)

#define min(a, b) ((a) < (b) ? (a) : (b))
    
// 计算n的最大倍数x且x<=max
#define MULTI_FLOOR(n, max) ((size_t)((max) / (n)) * (n))

// 根据分配单元大小计算chunk大小
#define CHUNK_SIZE(item_size) MULTI_FLOOR(NVMALLOC_MIN_BMP_CHUNK_SIZE, min((item_size) * 4096, NVMALLOC_MAX_BMP_CHUNK_SIZE))

// 分配一个chunk
#define ALLOCATE_CHUNK(nvmalloc, chunk, _addr, _len, _level_index, _item_size, fail_ret)    \
    do                                                                                      \
    {                                                                                       \
        (chunk) = malloc(sizeof(struct nvmalloc_chunk));                                    \
        if(!(chunk))                                                                        \
            ERROR((fail_ret), 1, "malloc(sizeof(struct nvmalloc_chunk) failed!");           \
        (chunk)->addr = (_addr);                                                            \
        (chunk)->len = (_len);                                                              \
        (chunk)->level_index = (_level_index);                                              \
        (chunk)->item_size = (_item_size);                                                  \
        int _ret = bmp4k_init(&((chunk)->bitmap), (_len) / (_item_size), 1);                \
        assert(_ret);                                                                       \
        assert((size_t)(chunk) % 2 == 0);                                                   \
        MARK_CHUNK_MAPS((nvmalloc), (_addr), (_len), (chunk));                              \
    }                                                                                       \
    while(0)

// 把chunk从链表移除
#define REMOVE_CHUNK_FROM_LIST(list, chunk)         \
    do                                              \
    {                                               \
        if((chunk)->prev)                           \
            (chunk)->prev->next = (chunk)->next;    \
        if((chunk)->next)                           \
            (chunk)->next->prev = (chunk)->prev;    \
        if((*(list)) == (chunk))                    \
            (*(list)) = (chunk)->next;              \
    }                                               \
    while(0)

// 把chunk放入链表头部
#define ADD_CHUNK_TO_HEAD(list, chunk)              \
    do                                              \
    {                                               \
        (chunk)->next = (*(list));                  \
        if((chunk)->next)                           \
            (chunk)->next->prev = (chunk);          \
        (*(list)) = (chunk);                        \
        (chunk)->prev = 0;                          \
    }                                               \
    while(0)

// 如果chunk已经全部分配完，则从level->avals头部取出，放入level->unavals头部
#define MOVE_TO_UNAVAIL_IF_NEED(chunk, level)                   \
    do                                                          \
    {                                                           \
        if(!bmp4k_exist(&((chunk)->bitmap), 1))                 \
        {                                                       \
            REMOVE_CHUNK_FROM_LIST(&((level)->avails), chunk);  \
            ADD_CHUNK_TO_HEAD(&((level)->unavails), chunk);     \
        }                                                       \
    }                                                           \
    while(0)

size_t nvmalloc_malloc(struct nvmalloc* nvmalloc, size_t size)
{
    assert(nvmalloc);
    if(size > NVMALLOC_MAX_BMP_ITEM_SIZE)
    {
        size_t alloc_sz = NVMALLOC_MAX_BMP_ITEM_SIZE * 2;
        while(alloc_sz < size)
            alloc_sz *= 2;
        size_t addr = buddy_allocate(&(nvmalloc->buddy), alloc_sz);
        if(addr == BUDDY_ALLOCATE_FAILED)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "buddy_allocate(&(nvmalloc->buddy), %lu) failed!", alloc_sz);
        MARK_CHUNK_MAPS(nvmalloc, addr, alloc_sz, alloc_sz + 1);
        return addr;
    }
    size_t level_index, item_size;
    PARSE_SIZE(size, level_index, item_size);
    assert(level_index < nvmalloc->level_count);
    struct nvmalloc_level* level = nvmalloc->levels + level_index;
    if(!level->avails)
    {
        size_t chunk_sz = CHUNK_SIZE(item_size);
        size_t addr = buddy_allocate(&(nvmalloc->buddy), chunk_sz);
        if(addr == BUDDY_ALLOCATE_FAILED)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "buddy_allocate(&(nvmalloc->buddy), %lu) failed!", chunk_sz);
        struct nvmalloc_chunk* chunk;
        ALLOCATE_CHUNK(nvmalloc, chunk, addr, chunk_sz, level_index, item_size, NVMALLOC_ALLOCATE_FAILED);
        ADD_CHUNK_TO_HEAD(&(level->avails), chunk);
    }
    struct nvmalloc_chunk* chunk = level->avails;
    assert(chunk);
    assert(chunk->level_index == level_index);
    assert(chunk->item_size == item_size);
    size_t item_index = bmp4k_search(&(chunk->bitmap), 1);
    assert(item_index != BMP4K_NOT_FOUND);
    int ret = bmp4k_set(&(chunk->bitmap), item_index, 0);
    assert(ret);
    size_t alloc_addr = chunk->addr + item_index * item_size;
    MOVE_TO_UNAVAIL_IF_NEED(chunk, level);
    return alloc_addr;
}

#define MAP_ADDR_TO_CHUNK(nvmalloc, addr, chunk, fail_ret)                                  \
    do                                                                                      \
    {                                                                                       \
        size_t _map_index = (addr) / NVMALLOC_MIN_BMP_CHUNK_SIZE;                           \
        if(_map_index >= (nvmalloc)->map_count)                                             \
            ERROR((fail_ret), 0, "param <%s = %lu> is out of range!", #addr, (addr));       \
        (chunk) = (nvmalloc)->chunk_maps[_map_index];                                       \
    }                                                                                       \
    while(0)

size_t nvmalloc_malloc_at(struct nvmalloc* nvmalloc, size_t addr, size_t size)
{
    assert(nvmalloc);
    if(size > NVMALLOC_MAX_BMP_ITEM_SIZE)
    {
        size_t alloc_sz = NVMALLOC_MAX_BMP_ITEM_SIZE * 2;
        while(alloc_sz < size)
            alloc_sz *= 2;
        if(addr % alloc_sz)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "param <addr = %lu> is not aligned by <alloc_sz = %lu>!", addr, alloc_sz);
        if(buddy_allocate_at(&(nvmalloc->buddy), addr, alloc_sz) == BUDDY_ALLOCATE_FAILED)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "buddy_allocate_at(&(nvmalloc->buddy), %lu, %lu) failed!", addr, alloc_sz);
        MARK_CHUNK_MAPS(nvmalloc, addr, alloc_sz, alloc_sz + 1);
        return addr;
    }
    struct nvmalloc_chunk* chunk;
    MAP_ADDR_TO_CHUNK(nvmalloc, addr, chunk, NVMALLOC_ALLOCATE_FAILED);
    if((size_t)chunk % 2)
        ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "param <addr = %lu> has been allocated to size %lu!", addr, (size_t)chunk - 1);
    size_t level_index, item_size;
    PARSE_SIZE(size, level_index, item_size);
    assert(level_index < nvmalloc->level_count);
    struct nvmalloc_level* level = nvmalloc->levels + level_index;
    if(!chunk)
    {
        size_t chunk_sz = CHUNK_SIZE(item_size);
        size_t chunk_addr = MULTI_FLOOR(chunk_sz, addr);
        size_t item_index = (addr - chunk_addr) / item_size;
        if(addr != chunk_addr + item_index * item_size)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "param <addr = %lu> is not aligned by item size %lu!", addr, item_size);
        if(buddy_allocate_at(&(nvmalloc->buddy), chunk_addr, chunk_sz) == BUDDY_ALLOCATE_FAILED)
            ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "buddy_allocate_at(&(nvmalloc->buddy), %lu, %lu) failed!", chunk_addr, chunk_sz);
        ALLOCATE_CHUNK(nvmalloc, chunk, chunk_addr, chunk_sz, level_index, item_size, NVMALLOC_ALLOCATE_FAILED);
        ADD_CHUNK_TO_HEAD(&(level->avails), chunk);
    }
    else if(chunk->item_size != item_size)
        ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "param <addr = %lu> has been allocated to size %lu!", addr, chunk->item_size);
    assert(chunk);
    assert(chunk->level_index == level_index);
    assert(chunk->item_size == item_size);
    size_t item_index = (addr - chunk->addr) / item_size;
    if(addr != chunk->addr + item_index * item_size)
        ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "param <addr = %lu> is not aligned by item size %lu!", addr, item_size);
    if(bmp4k_get(&(chunk->bitmap), item_index) != 1)
        ERROR(NVMALLOC_ALLOCATE_FAILED, 0, "object(<addr = %lu>, <size = %lu>) has been allocated!", addr, item_size);
    int ret = bmp4k_set(&(chunk->bitmap), item_index, 0);
    assert(ret);
    MOVE_TO_UNAVAIL_IF_NEED(chunk, level);
    return addr;
}

#define GET_ALLOCATED_ITEM_INDEX(chunk, _addr, item_index, fail_ret)                                                                \
    do                                                                                                                              \
    {                                                                                                                               \
        if(!(chunk))                                                                                                                \
            ERROR((fail_ret), 0, "param <%s = %lu> has not been allocated!", #_addr, (_addr));                                      \
        assert((chunk)->item_size);                                                                                                 \
        (item_index) = ((_addr) - (chunk)->addr) / (chunk)->item_size;                                                              \
        if((_addr) != (chunk)->addr + (item_index) * (chunk)->item_size)                                                            \
            ERROR((fail_ret), 0, "param <%s = %lu> is not aligned by item size %lu!", #_addr, (addr), (chunk)->item_size);          \
        if(bmp4k_get(&((chunk)->bitmap), (item_index)) != 0)                                                                        \
            ERROR((fail_ret), 0, "object(<addr = %lu>, <item_size = %lu>) has not been allocated!", (_addr), (chunk)->item_size);   \
    }                                                                                                                               \
    while(0)

int nvmalloc_free(struct nvmalloc* nvmalloc, size_t addr)
{
    assert(nvmalloc);
    struct nvmalloc_chunk* chunk;
    MAP_ADDR_TO_CHUNK(nvmalloc, addr, chunk, 0);
    if((size_t)chunk % 2)
    {
        size_t alloc_sz = (size_t)chunk - 1;
        assert(to_power(alloc_sz) != (size_t)(-1));
        if(addr % alloc_sz)
            ERROR(0, 0, "param <addr = %lu> is not aligned by <alloc_sz = %lu>!", addr, alloc_sz);
        if(!buddy_free(&(nvmalloc->buddy), addr, (size_t)chunk - 1))
            ERROR(0, 0, "buddy_free(&(nvmalloc->buddy), %lu, %lu) failed!", addr, (size_t)chunk - 1);
        return 1;
    }
    size_t item_index;
    GET_ALLOCATED_ITEM_INDEX(chunk, addr, item_index, 0);
    int was_unavail = !bmp4k_exist(&(chunk->bitmap), 1);
    int ret = bmp4k_set(&(chunk->bitmap), item_index, 1);
    assert(ret);
    struct nvmalloc_level* level = nvmalloc->levels + chunk->level_index;
    if(!bmp4k_exist(&(chunk->bitmap), 0))
    {
        if(!buddy_free(&(nvmalloc->buddy), chunk->addr, chunk->len))
            ERROR(0, 0, "buddy_free(&(nvmalloc->buddy), %lu, %lu) failed!", chunk->addr, chunk->len);
        MARK_CHUNK_MAPS((nvmalloc), chunk->addr, chunk->len, 0);
        REMOVE_CHUNK_FROM_LIST(was_unavail ? &((level)->unavails) : &((level)->avails), chunk);
        free(chunk);
        return 1;
    }
    if(was_unavail)
    {
        REMOVE_CHUNK_FROM_LIST(&((level)->unavails), chunk);
        ADD_CHUNK_TO_HEAD(&((level)->avails), chunk);
    }
    return 1;
}

size_t nvmalloc_usable_size(struct nvmalloc* nvmalloc, size_t addr)
{
    assert(nvmalloc);
    struct nvmalloc_chunk* chunk;
    MAP_ADDR_TO_CHUNK(nvmalloc, addr, chunk, 0);
    if((size_t)chunk % 2)
    {
        size_t alloc_sz = (size_t)chunk - 1;
        assert(to_power(alloc_sz) != (size_t)(-1));
        if(addr % alloc_sz)
            ERROR(0, 0, "param <addr = %lu> is not aligned by <alloc_sz = %lu>!", addr, alloc_sz);
        return alloc_sz;
    }
    size_t item_index;
    GET_ALLOCATED_ITEM_INDEX(chunk, addr, item_index, 0);
    return chunk->item_size;
}
