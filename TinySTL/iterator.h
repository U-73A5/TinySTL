#ifndef TINYSTL_ITERATOR_H_
#define TINYSTL_ITERATOR_H_

namespace tinystl
{

/* 迭代器设计
 * 五种迭代器类型
*/

// 用类判断类型，可以通过重载在编译阶段确定使用哪一种方案
struct input_iterator_tag {};
struct output_iterator_tag {};
struct forward_iterator_tag : public input_iterator_tag {}; // 继承避免了重写只做传递调用的函数
struct bidirectional_iterator_tag : public forward_iterator_tag {};
struct random_access_iterator_tag : public bidirectional_iterator_tag {};


/* iterator模板，此后迭代器设计都应从此处继承
*/

template <class Category, class T,
          class Distance = ptrdiff_t,
          class Pointer = T*,
          class Reference = T&>
  struct iterator
  {
    typedef Category      iterator_category;
    typedef T             value_type;
    typedef Pointer       pointer;
    typedef Reference     reference;
    typedef Distance      difference_type;
  };


/* iterator traits
 * 特性萃取技术
*/

// 萃取迭代器特性
template <class Iterator>
  struct iterator_traits
  {
    typedef typename Iterator::iterator_category     iterator_category;
    typedef typename Iterator::value_type            value_type;
    typedef typename Iterator::pointer               pointer;
    typedef typename Iterator::reference             reference;
    typedef typename Iterator::difference_type       difference_type;
  };
// 针对原生pointer,pointer-to-const设计的traits偏特化版本
template <class T>
  struct iterator_traits<T*>
  {
    typedef random_access_iterator_tag     iterator_category;
    typedef T                              value_type;
    typedef T*                             pointer;
    typedef T&                             reference;
    typedef ptrdiff_t                      difference_type;
  };
template <class T>
  struct iterator_traits<const T*>
  {
    typedef random_access_iterator_tag     iterator_category;
    typedef T                              value_type;
    typedef T*                             pointer;
    typedef T&                             reference;
    typedef ptrdiff_t                      difference_type;
  };


// 决定迭代器的 category
template <class Iterator>
  inline typename iterator_traits<Iterator>::iterator_category
  iterator_category(const Iterator&)
  {
    typedef typename iterator_traits<Iterator>::iterator_categoey category;
    return category();
  }
// 决定迭代器的 value_type
template <class Iterator>
  inline typename iterator_traits<Iterator>::value_type*
  value_type(const Iterator&)
  {
    return static_cast<typename iterator_traits<Iterator>::value_type*>(0);
  }
// 决定迭代器的 distance_type
template <class Iterator>
  inline typename iterator_traits<Iterator>::difference_type*
  distance_type(const Iterator&)
  {
    return static_cast<typename iterator_traits<ITerator>::difference_type*>(0);
  }


/* distance 函数
*/
template <class InputIterator>
  inline typename iterator_traits<InputIterator>::difference_type
  __distance(InputIterator first, InputIterator last,
             input_iterator_tag)
  {
    iterator_traits<InputIterator>::difference_type n = 0;
    while (first != last) {
      ++first; ++n;
    }
    return n;
  }
template <class RandomAccessIterator>
  inline typename iterator_traits<RandomAccessIterator>::difference_type
  __distance(RandomAccessIterator first, RandomAccessIterator last,
             random_access_iterator_tag)
  {
    return last - first;
  }
template <class InputIterator> //STL命名规范：以算法所能接受的最低阶迭代器类型作为迭代器型别参数命名
  inline typename iterator_traits<InputIterator>::difference_type
  distance(InputIterator first, InputIterator last)
  {
    return __distance(first, last, iterator_category(first));
  }


/* advance 函数
*/
template <class InputIterator, class Distance>
  inline void __advance(InputIterator& i, Distance n,
                        input_iterator_tag)
  {
    while (--n) ++i;
  }
template <class BidirectionalIterator, class Distance>
  inline void __advance(BidirectionalIterator& i, Distance n,
                        bidirectional_iterator_tag)
  {
    if (n >= 0) while (--n) ++i;
    else while (++n) --i;
  }
template <class RandomAccessIterator, class Distance>
  inline void __advance(RandomAccessIterator& i, Distance n,
                        random_access_iterator_tag)
  {
    i += n;
  }
template <class InputIterator, class Distance>
  inline void advance(InputIterator& i, Distance n)
  {
    __advance(i, n, iterator_category(i));
  }


} // namespace tinystl

#endif // !TINYSTL_ITERATOR_H_

