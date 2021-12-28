/**
 * 定义全局函数 construct() 和 destory()
 * 负责对象内容的构造与析构
 */

#ifndef TINYSTL_CONSTRUCT_H_
#define TINYSTL_CONSTRUCT_H_

#include <new.h> // for placement new
#include "type_traits.h"
#include "iterator.h"

namespace tinystl
{

// 构造
template  <class T1, class T2>
  inline void construct(T1* p, const T2& value)
  {
    new (p) T1(value);
  }

// 析构1：
// 删除单个元素
template <class T>
  inline void destroy(T* pointer)
  {
    pointer->~T();
  }

// 析构2：
// 删除区间元素
// 获取删除元素类型
template <class ForwardIterator>
  inline void destory(ForwardIterator first, ForwardIterator last)
  {
    __destory(first, last, value_type(first));
  }
// 获取删除元素是否有必要调用析构函数
template <class ForwardIterator, class T>
  inline void __destory(ForwardIterator first, ForwardIterator last,
                        T*)
  {
    typedef typename __type_traits<T>::has_trivial_destructor trivial_destructor;
    __destory_aux(ForwardIterator first, ForwardIterator last, trivial_destructor());
  }
// 元素数值型别(value type)有 non-trivial destructor
template <class ForwardIterator>
    inline void __destory_aux(ForwardIterator first, ForwardIterator last,
                            __false_type)
    {
      for ( ; first < last; ++first) destroy(&*first);
    }
// 元素数值型别(value type)有 trivial destructor
template <class ForwardIterator>
  inline void __destory_aux(ForwardIterator, ForwardIterator,
                          __true_type)
  { }

// 析构2 对迭代器为 char* 和 wchar_t* 的特化版
template <>
  inline void destory(char*, char*)
  { }
template <>
  inline void destory(wchar_t*, wchar_t*)
  { }

} // namespace tinystl

#endif // !TINYSTL_CONSTRUCT_H_

