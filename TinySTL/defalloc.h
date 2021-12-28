/**
 * 原始的 HP default allocator
 * 仅为回溯兼容
 * 除非容器使用旧式做法，不要使用这个文件
 * 此文件不包含于其它任何 SGI STL 头文件
 * 
 * SGI-style allocators 不带有任何与对象型别相关的参数
 * 其只响应 void* 指针。
 * 标准接口响应一个指向对象型别的指针 T*
 */

#ifndef TINYSTL_DEFALLOC_H_
#define TINYSTL_DEFALLOC_H_

#include <new.h>
#include <stddef.h>
#include <stdlib.h>
#include <limits.h>
#include "algobase.h"

namespace tinystl
{

// 仅对基层内存配置/释放行为的简单包装
// 没有任何效率上的强化

template <class T>
  inline T* allocate(ptrdiff_t size, T*)
  {
   set_new_handler(0); //未设置或设置为空，在获取内存失败后，抛出BAD_ALLOC异常
    T* temp = (T*)(::operator new((size_t)(size * sizeof(T))));
    if (0 == temp) {
      cerr << "out of memory" << endl;
      exit(1);
    }
    return temp;
  }

template <class T>
  inline void deallocate(T* buffer)
  {
    ::operator delete(buffer);
  }

/**
 * HP-style 配置器
 */

template <class T>
  class allocator
  {
    public:
    typedef T             value_type;
    typedef T*            pointer;
    typedef const T*      const_pointer;
    typedef T&            reference;
    typedef const T&      const_reference;
    typedef size_t        size_type;
    typedef ptrdiff_t     difference_type;

    pointer allocate(size_type n)
    {
      return tinystl::allocate((difference_type)n, (pointer)0);
    }

    void deallocate(pointer p)
    {
      tinystl::deallocate(p);
    }

    pointer address(reference x)
    {
      return (pointer)&x;
    }

    const_pointer address(const_reference x)
    {
      return (const_pointer)&x;
    }

    size_type init_page_size()
    {
      return max(size_type(1), size_type(4096/sizeof(T))); // 4096: the size of one page/ one IO block
    }

    size_type max_size() const
    {
      return max(size_type(1), size_type(UINT_MAX/sizeof(T)));
    }
  };

// 特化版本
template <>
  class allocator<void>
  {
    public:
    typedef void*     pointer;
  };

} // namespace tinystl

#endif // !TINYSTL_DEFALLOC_H_

