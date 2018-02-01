#include <stdio.h>
#include <assert.h>
#include <nvmalloc.h>

int main()
{
    struct nvmalloc nvm;
    int ret;
    size_t addr;
    size_t size;

    ret = nvmalloc_init(&nvm, (size_t)1<<40);
    assert(ret);

    addr = nvmalloc_malloc(&nvm, 80);
    assert(addr != NVMALLOC_ALLOCATE_FAILED);
    printf("addr = %lu\n", addr);

    addr = nvmalloc_malloc_at(&nvm, addr+80, 80);
    assert(addr != NVMALLOC_ALLOCATE_FAILED);
    printf("addr = %lu\n", addr);

    ret = nvmalloc_free(&nvm, addr);
    assert(ret);

    ret = nvmalloc_free(&nvm, addr-80);
    assert(ret);

    for(size_t l = 0; l < 1; l++)
    {
        for(size_t i = 0; i < 1000000000; i++)
        {
            //addr = nvmalloc_malloc(&nvm, 64);
            addr = nvmalloc_malloc_at(&nvm, i<<6, 1<<6);
            assert(addr != NVMALLOC_ALLOCATE_FAILED);
            size = nvmalloc_usable_size(&nvm, addr);
            assert(size == 1<<6);
        }
        for(size_t i = 0; i < 1000000000; i++)
        {
            ret = nvmalloc_free(&nvm, i<<6);
            assert(ret);
        }
    }

    ret = nvmalloc_deinit(&nvm);
    assert(ret);

    return 0;
}