#include <bmp4k.h>
#include <common.h>
#include <string.h>

#define DIV_CEIL(n, align) (((n) - 1) / (align) + 1)

int bmp4k_init(struct bmp4k* bmp4k, size_t len, int value_all)
{
    assert(bmp4k);
    if(len == 0 || len > 4096)
        ERROR(0, 0, "param <len = %lu> is invalid!", len);
    bmp4k->len = len;
    bmp4k->has_0 = value_all ? 0 : (uint64_t)(-1);
    bmp4k->has_1 = value_all ? (uint64_t)(-1) : 0;
    memset(bmp4k->bits, value_all ? (uint8_t)(-1) : 0, sizeof(bmp4k->bits));
    return 1;
}

int bmp4k_get(struct bmp4k* bmp4k, size_t index)
{
    assert(bmp4k);
    if(index >= bmp4k->len)
        ERROR(-1 , 0, "param <index = %lu> is bigger than <bmp4k->len = %lu>!", index, bmp4k->len);
    return (bmp4k->bits[index / 64] >> (index % 64)) & (uint64_t)1;
}

int bmp4k_set(struct bmp4k* bmp4k, size_t index, int value)
{
    assert(bmp4k);
    if(index >= bmp4k->len)
        ERROR(0 , 0, "param <index = %lu> is bigger than <bmp4k->len = %lu>!", index, bmp4k->len);
    size_t u64_index = index / 64;
    size_t last_index = DIV_CEIL(bmp4k->len, 64) - 1;
    assert(u64_index <= last_index);
    size_t bit_count = u64_index < last_index ? 64 : bmp4k->len - last_index * 64;
    assert(1 <= bit_count && bit_count <= 64);
    uint64_t mask = bit_count < 64 ? ((uint64_t)1 << bit_count) - 1 : (uint64_t)(-1);
    if(value)
    {
        bmp4k->bits[u64_index] |= ((uint64_t)1 << (index % 64));
        bmp4k->has_1 |= ((uint64_t)1 << u64_index);
        if((bmp4k->bits[u64_index] | ~mask) == (uint64_t)(-1))
            bmp4k->has_0 &= ~((uint64_t)1 << u64_index);
    }
    else
    {
        bmp4k->bits[u64_index] &= ~((uint64_t)1 << (index % 64));
        bmp4k->has_0 |= ((uint64_t)1 << u64_index);
        if((bmp4k->bits[u64_index] & mask) == 0)
            bmp4k->has_1 &= ~((uint64_t)1 << u64_index);
    }
    return 1;
}

int bmp4k_exist(struct bmp4k* bmp4k, int value)
{
    assert(bmp4k);
    size_t u64_count = DIV_CEIL(bmp4k->len, 64);
    assert(1<= u64_count && u64_count <= 64);
    uint64_t mask = u64_count < 64 ? ((uint64_t)1 << u64_count) - 1 : (uint64_t)(-1);
    if(value)
        return (bmp4k->has_1 & mask) != 0;
    else
        return (bmp4k->has_0 & mask) != 0;
}

static size_t binary_search_u64(uint64_t bits, size_t bit_count)
{
    size_t left = bit_count - 1, right = 0;
    while(left > right)
    {
        size_t mid = (left + right) / 2;
        assert(mid < 63);
        uint64_t mask = ((uint64_t)1 << (mid + 1)) - ((uint64_t)1 << right);
        if(bits & mask)
            left = mid;
        else
            right = mid + 1;
    }
    size_t index = left;
    return (bits >> index) & (uint64_t)1 ? index : BMP4K_NOT_FOUND;
}

size_t bmp4k_search(struct bmp4k* bmp4k, int value)
{
    assert(bmp4k);
    if(!bmp4k_exist(bmp4k, value))
        return BMP4K_NOT_FOUND;
    size_t u64_count = DIV_CEIL(bmp4k->len, 64);
    assert(1<= u64_count && u64_count <= 64);
    size_t u64_index = binary_search_u64(value ? bmp4k->has_1 : bmp4k->has_0, u64_count);
    if(u64_index == BMP4K_NOT_FOUND)
        return BMP4K_NOT_FOUND;
    size_t last_index = u64_count - 1;
    size_t bit_count = u64_index < last_index ? 64 : bmp4k->len - last_index * 64;
    size_t bits = value ? bmp4k->bits[u64_index] : ~(bmp4k->bits[u64_index]);
    size_t bit_index = binary_search_u64(bits, bit_count);
    assert(((bits >> bit_index) & (uint64_t)1));
    return u64_index * 64 + bit_index;
}
