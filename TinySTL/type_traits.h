/**
 * 这一部分比较迷惑。
 * type_traits 应该属于C++标准库STD。
 * 但在《STL源码剖析》附带的 SGI-STL 中，type_traits 用到了STL的 config.h 中定义的宏。
 * 其次，今天(12.28.2021)我所用的GCC(tdm64-1)中的 type_traits 文件与我正在学的有较大出入。
 * 不仅仅是具体的实现、结构。在命名上似乎都存在较大出入。
 * 
 * 这个文件是按照我正在看的这一份文件写的。
 * 暂时抛弃STL的 config.h 文件，而在此独立定义、判断。
 */

#ifndef TINYSTL_TYPE_TRAITS_H_
#define TINYSTL_TYPE_TRAITS_H_

/* //
#ifndef TINYSTL_CONFIG_H_
#include "config.h"
#endif
// 这是我正在看的文件中有的。
// 按逻辑来讲，当这个文件被编译时，一定有STL的 config.h，
// 否则根本无法通过编译。
// 但这也就是说其实这个部分依赖于STL，是STL的一部分，
// 而非STD的一部分。
*/

namespace tinystl
{

struct __true_type 
{ };
struct __false_type
{ };

template <class type>
  struct __type_traits
  {
    typedef __true_type      this_dummy_member_must_be_first;
    // 这个特殊成员的目的是为在需要一个 ___true_type处理毫不相关的情况下，
    // 使用 __type_traits时能够提供 __true_type

    typedef __false_type     has_trivial_default_constructor;
    typedef __false_type     has_trivial_copy_constructor;
    typedef __false_type     has_trivial_assignment_operator;
    typedef __false_type     has_trivial_destructor;
    typedef __false_type     is_POD_type;
  };

// 特化版本
template <>
  struct __type_traits<char>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<signed char>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<unsigned char>
  {
    typedef __true_type     has_trivial_default_construuctor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<short>
  {
    typedef __true_type     has_trivial_default_operator;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<unsigned short>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<int>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<unsigned int>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<long>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<unsigned long>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<float>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<double>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };
template <>
  struct __type_traits<long double>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };

// 偏特化版本
// 这里实际只为char*/signed char*/unsigned char*做了偏特化
template <class T>
  struct __type_traits<T*>
  {
    typedef __true_type     has_trivial_default_constructor;
    typedef __true_type     has_trivial_copy_constructor;
    typedef __true_type     has_trivial_assignment_operator;
    typedef __true_type     has_trivial_destructor;
    typedef __true_type     is_POD_type;
  };

} // namespace tinystl

#endif // !TINYSTL_TYPE_TRAITS_H_

