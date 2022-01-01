/**
 * 定义全局函数
 * uninitialized_copy()
 * uninitialized_fill()
 * uninitialized_fill_n()
 * 
 * 不属于配置器，但与对象初值设置有关
 * 对容器的大规模元素设置有帮助。
 * 最差调用 construct()
 * 最佳使用C标准库 memmove() 进行内存数据移动。
 */

#ifndef TINYSTL_UNINITIALIZED_H_
#define TINYSTL_UNINITIALIZED_H_

#include <string.h> // for memmove()
#include "algobase.h" // for copy() fill() fill_n()
#include "construct.h"
#include "type_traits.h"

namespace tinystl
{

/**
 * uninitialized_copy(first, last, result)
 * 对 [first, last) 范围内产生 *result 的复制品
 */
// 萃取迭代器的 value type
template <class InputIterator, class ForwardIterator>
  inline ForwardIterator uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result)
  {
    return __uninitialized_copy(first, last, result, value_type(result));
  }
// 判断型别是否为 POD 型别
/* POD 指 Plain Old Data，
 * 即标量型别(scalar types)或传统的C struct型别
*/
template <class InputIterator, class ForwardIterator, class T>
  inline ForwardIterator __uninitialized_copy(InputIterator first, InputIterator last, ForwardIterator result, T*)
  {
    typedef typename __type_traits<T>::is_POS_type is_POD;
    return __uninitialized_copy_aux(first, last, result, is_POD());
  }
template <class InputIterator, class ForwardIterator>
  inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result,
                                                  __true_type)
  {
    return copy(first, last, result); // 交由高阶函数执行
  }
template <class InputIterator, class ForwardIterator>
  inline ForwardIterator __uninitialized_copy_aux(InputIterator first, InputIterator last, ForwardIterator result,
                                                  __false_type)
  {
    ForwardIterator cur = result;
    try {
      for ( ; first!=last; ++first, ++cur)
        construct(&*cur, *first);
        return cur;
    } catch (...) {
      destroy(result, cur);
      throw;
    }
  }
// 对 char* 和 wchar_t* 的特化版本
template <>
  inline char* uninitialized_copy(const char* first, const char* last, char* result)
  {
    memmove(result, first, last-first);
    return result + (last - first);
  }
template <>
  inline wchar_t* uninitialized_copy(const wchar_t* first, const wchar_t* last, wchar_t* result)
  {
    memmove(result, first, sizeof(wchar_t) * (last - first));
    return result + (last - first);
  }


/**
 * uninitialized_fill(first, last, x)
 * 对 [first, last) 范围内产生 x 的复制品
 */
template <class ForwardIterator, class T>
  inline void uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x)
  {
    __uninitialized_fill(first, last, x, value_type(first));
  }
template <class ForwardIterator, class T, class T1>
  inline void __uninitialized_fill(ForwardIterator first, ForwardIterator last, const T& x, T1*)
  {
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    __uninitialized_fill_aux(first, last, x, is_POD());
  }
template <class ForwardIterator, class T>
  inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x,
                                       __true_type)
  {
    fill(first, last, x)
  }
template <class ForwardIterator, class T>
  inline void __uninitialized_fill_aux(ForwardIterator first, ForwardIterator last, const T& x,
                                       __false_type)
  {
    ForwardIterator cur = first;
    try {
      for ( ; cur!=last; ++cur)
        construct(&*cur, x);
    } catch (...) {
      destroy(first, cur);
      throw;
    }
  }


/**
 * uninitialized_fill_n(first, n, x)
 * 对 [first, first+n) 范围内产生 x 的复制品
 */
template <class ForwardIterator, class Size, class T>
  inline ForwardIterator uninitialized_fill_n(ForwardIterator first, Size n, const T& x)
  {
    return __uninitialized_fill_n(first, n, x, value_type(first));
  }
template <class ForwardIterator, class Size, class T, class T1>
  inline ForwardIterator __uninitialized_fill_n(ForwardIterator first, Size n, const T& x, T1*)
  {
    typedef typename __type_traits<T1>::is_POD_type is_POD;
    return __uninitialized_fill_n_aux(first, n, x, is_POD());
  }
template <class ForwardIterator, class Size, class T>
  inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T&x,
                                                    __true_type)
  {
    return fill_n(first, n, x);
  }
template <class ForwardIterator, class Size, class T>
  inline ForwardIterator __uninitialized_fill_n_aux(ForwardIterator first, Size n, const T& x,
                                                    __false_type)
  {
    ForwardIterator cur = first;
    try {
      for ( ; n>0; --n, ++cur)
        construct(&*cur, x);
      return cur;
    } catch (...) {
      destroy(first, cur);
      throw;
    }
  }

} // namespace tinystl

#endif // !TINYSTL_UNINITIALIZED_H_

