/**
 * 定义一、二级配置器
 * 配置器名为 alloc
 * 负责内存空间的配置与释放
 * 
 * 不考虑多线程(multi-threads)状态
 */

#ifndef TINYSTL_ALLOC_H_
#define TINYSTL_ALLOC_H_

namespace tinystl
{

typedef __malloc_alloc_template<0> malloc_alloc;
// 令 alloc 为第一级配置器
// typedef malloc_alloc alloc;
// 令 alloc 为第二级配置器
// false 表示不考虑多线程。
typedef __default_alloc_template<false, 0> alloc;

// SGI包装的，符合STL规范的，对外使用的配置器接口
template <class T, class Alloc>
  class simple_alloc
  {
    public:
    static T* allocate(size_t n)
    {
      return 0 == n ? \
             0 : \
             (T*)Alloc::allocate(n * sizeof(T));
    }
    static T* allocate(void)
    {
      return 0 == n ? \
             0 : \
             (T*)Alloc::allocate(sizeof(T));
    }
    static void deallocate(T* p, size_t n)
    {
      if (0 != n) Alloc::deallocate(p, n * sizeof(T));
    }
    static void deallocate(T* p)
    {
      Alloc::deallocate(p, sizeof(T));
    }
  };


/**
 * 第一级配置器
 */
#if 0
#   include <new>
#   define __THROW_BAD_ALLOC throw std::bad_alloc;
#elif !defined(__THROW_BAD_ALLOC)
#   include <iostream>
#   define __THROW_BAD_ALLOC \
           std::cerr << "out of memory" << endl; \
           exit(1);
#endif

template <int inst>
  class __malloc_alloc_template
  {
    private:
    // oom: out of memory
    static void* oom_malloc(size_t);
    static void* oom_realloc(void*, size_t);
    static void (* __malloc_alloc_oom_handler)();

    public:
    static void* allocate(size_t n)
    {
      void* result = malloc(n);
      if (0 == result) result = oom_malloc(n);
      return result;
    }
    static void deallocate(void* p, size_t /* n */)
    {
      free(p);
    }
    static void* reallocate(void* p, size_t /* old_sz */, size_t new_sz)
    {
      void* result = realloc(p, new_sz);
      if (0 == result) result = oom_realloc(p, new_sz);
      return result;
    }

    // 以下仿真C++的 set_new_handler()。
    // 因为没有用 ::operator new ，且C++没有提供与 realloc() 相关操作，所以不能用。
    static void (* set_malloc_hander(void (*f)()) )()
    {
      void (* old)() = __malloc_alloc_oom_handler;
      __malloc_alloc_oom_handler = f;
      return (old);
    }
  };

// 初值为零，应由客端设定
template <int inst>
  void (* __malloc_alloc_template<inst>::__malloc_alloc_oom_handler)() \
  = 0;

template <int inst>
  void* __malloc_alloc_template<inst>::oom_malloc(size_t n)
  {
    void (* my_malloc_handler)();
    void* result;
    for ( ; ; ) { // 不断尝试释放、配置...
      my_malloc_handler = __malloc_alloc_oom_handler;
      if (0 == my_malloc_handler) {
        __THROW_BAD_ALLOC;
      }
      (* my_malloc_handler)();
      result = malloc(n);
      if (result) return result;
    }
  }

template <int inst>
  void* __malloc_alloc_template<inst>::oom_realloc(void* p, size_t n)
  {
    void (* my_malloc_handler)();
    void* result;
    for ( ; ; ) {
      my_malloc_handler = __malloc_alloc_oom_handler;
      if (0 == my_malloc_handler) {
        __THROW_BAD_ALLOC;
      }
      (*my_malloc_handler)();
      result = realloc(p, n);
      if (result) return result;
    }
  }


/**
 * 第二级配置器
 * 区块超过 128 bytes 移交第一级配置器，否则以内存池(memory pool)管理。
 * 为方便管理，SGI第二级配置器主动将小额区块需求量上调至 8 的倍数。
 * 
 * 次层配置：每次配置一大块内存，并维护对应自由链表(free-list)，相同大小需求从中拨出，
 * 客端释还小额区块，由配置器回收到自由链表。
 */

enum
{
  __ALIGN = 8, // 小型区块的上调边界
  __MAX_BYTES = 128, // 小型区块的上限
  __NFREELISTS = __MAX_BYTES / __ALIGN // free-lists 个数
};

// 第一参数用于多线程环境下，本练习不考虑
template <bool threads, int inst>
  class __default_alloc_template
  {
    /* Private */
    private:
    // ROUNND_UP() 将 bytes 上调至 8 的倍数
    static size_t ROUND_UP(size_t bytes)
    {
      return ((bytes + __ALIGN-1) & ~(__ALIGN - 1));
    }

    // free-list 节点结构
    // 采用 union 避免额外的指针开销。
    // 这种方式在非强型语言中是普遍的，而在强型语言(strongly typed)中行不通。
    union obj
    {
      union obj* free_list_link;
      char client_data[1];
    };

    // 16 个 free-lists
    static obj* volatile free_list[__NFREELISTS];

    // 根据区块大小，决定使用几号 free-list
    static size_t FREELIST_INDEX(size_t bytes)
    {
      return ((bytes + __ALIGN-1) / __ALIGN - 1)
    }

    // 通过 chunk_alloc 扩充大小为 size 的自由链表
    static void* refill(size_t n);

    // 从内存池配置大小为 nobjs 个 size 的空间
    // 实际可能返回小于 nobjs 个 size 空间的内存。
    static char* chunk_alloc(size_t size, int& nobjs);

    // Chunk allocation state
    static char* start_free; // 内存池起始位置
    static char* end_free;  // 内存池结束位置
    static size_t heap_size;

    /* Public */
    public:
    // n must be > 0
    static void* allocate(size_t n)
    {
      obj* volatile* my_free_list;
      obj* result;
      // 大于 128 调用第一级配置器
      if (n > (size_t)__MAX_BYTES) {
        return malloc_alloc::allocate(n);
      }
      // 寻找 16 个自由链表中合适的一个
      my_free_list = free_list + FREELIST_INDEX(n);
      result = *my_free_list;
      if (0 == result) {
        // 没有可用链表，重新扩充相应大小的链表
        void* r = refill(ROUND_UP(n));
        return r;
      }
      // 调整链表
      *my_free_list = result -> free_list_link;
      return result;
    }

    // p 不能为 0
    static void deallocate(void* p, size_t n)
    {
      obj* q = (obj*) p;
      obj* volatile* my_free_list;
      // 大于 128 调用第一级配置器
      if (n > (size_t)__MAX_BYTES) {
        malloc_alloc::deallocate(p, n);
        return;
      }
      // 寻找对应链表。进行回收
      my_free_list = free_list + FREELIST_INDEX(n);
      q -> free_list_link = *my_free_list;
      *my_free_list = q;
    }
    static void* reallocate(void* p, size_t old_sz, size_t new_sz);
  };


// static data member 的定义与初值设定
template <bool threads, int inst>
  char* __default_alloc_template<threads, inst>::start_free \
  = 0;
template <bool threads, int inst>
  char* __default_alloc_template<threads, inst>::end_free \
  = 0;
template <bool threads, int inst>
  size_t __default_alloc_template<threads, inst>::heap_size \
  = 0;
template <bool threads, int inst>
  typename __default_alloc_template<threads, inst>::obj* volatile
  __default_alloc_template<threads, inst>::free_list[__NFREELISTS] \
  = {0, 0, 0, 0, 
     0, 0, 0, 0, 
     0, 0, 0, 0, 
     0, 0, 0, 0};


template <bool threads, int inst>
  void* __default_alloc_template<threads, inst>::refill(size_t n)
  {
    // 默认尝试扩充 nobjs=20 个
    int nobjs = 20;
    // chunk_alloc 参数 nobjs 是 pass by reference
    // 其后 nobjs 的值可能小于 20
    char* chunk = chunk_alloc(n, nobjs);

    obj* volatile* my_free_list;
    obj* result;
    obj* current_obj, * next_obj;
    int i;

    // 如果 nobjs=1，将区块分配给调用者
    if (1 == nobjs) return chunk;
    // 否则调整自由链表，纳入新节点
    my_free_list = free_list + FREELIST_INDEX(n);
    result = (obj*)chunk; //第一块返回给调用者
    *my_free_list = next_obj = (obj*)(chunk + n);
    for (i=1;  ; ++i) { //从 1 开始，因为第 0 个返回给调用者
      current_obj -> next_obj;
      next_obj = (obj*)((char*)next_obj + n);
      if (nobjs - 1 == i) {
        chrrent_obj -> free_list_link = 0;
        break;
      } else {
        current_obj -> free_list_link = next_obj;
      }
    }
    return result;
  }

// 从内存池中取空间给自由链表使用
template <bool threads, int inst>
  char* __default_alloc_template<threads, inst>::chunk_alloc(size_t size, int& nobjs)
  {
    char* result;
    size_t total_bytes = size * nobjs;
    size_t bytes_left = end_free - start_free; // 内存池剩余空间

    if (bytes_left >= total_bytes) { // 内存池空间完全满足需求
      result = start_free;
      start_free += total_bytes;
      return result;
    } else if (bytes_left >= size) { // 内存池不能满足需求，但能供应一个以上的区块
      nobjs = bytes_left / size;
      total_bytes = size * nobjs;
      result = start_free;
      start_free += total_bytes;
      return result;
    } else { // 内存池连一个区块大小都无法提供
      size_t bytes_to_get = 2 * total_bytes + ROUND_UP(heap_size >> 4);
      // 先将内存池剩余空间配给合适的链表
      if (bytes_left > 0) {
        obj* volatile* my_free_list = free_list + FREELIST_INDEX(bytes_left);
        (obj*)start_free -> free_list_link = *my_free_list;
        *my_free_list = (obj*)start_free;
      }
      // 配置 heap 空间，补充内存池
      start_free = (char*)malloc(bytes_to_get);
      if (0 == start_free) { // heap 空间不足，配置失败
        int i;
        obj* volatile* my_free_list, * p;
        // 优先检视所有区块足够大的链表，并将其重新划分到当前链表，
        // 并不配置较小区块。在多线程(multi-process)机器上会导致灾难。
        for (i=size; i<=__MAX_BYTES; i+=__ALIGN) {
          my_free_list = free_list + FREELIST_INDEX(i);
          p = *my_free_list;
          if (0 != p) { // 链表有未用区块
            *my_free_list = p -> free_list_link;
            start_free = (char*)p;
            end_free = start_free + i;
            // 递归调用自己，修正 nobjs
            return chunk_alloc(size, nobjs);
            // 所有大于本区块的空闲链表区块都将会被编入本区块。
          }
        }
        // 出现意外，完全没有内存
        // 调用第一级配置器，向 out-of-memory 机制寻求帮助。
        end_free = 0;
        start_free = (char*)malloc_alloc::allocate(bytes_to_get);
      }
      heap_size += bytes_to_get;
      end_free = start_free + bytes_to_get;
      // 递归调用自己，修正 nobjs
      return chunk_alloc(size, nobjs);
    }
  }

} // namespace tinystl

#endif // !TINYSTL_ALLOC_H_

