/**
 * FILO(First In Last Out)
 * stack 不允许有遍历行为，不提供迭代器
 * 缺省下，deque 为 stack 底部结构
 * 
 * stack 不被归类为 container，而被归类为 container adapter
 */
#ifndef TINYSTL_STACK_H_
#define TINYSTL_STACK_H_

#include "deque.h"

namespace tinystl
{

template <class T, class Sequence = deque<T> >
  class stack
  {
    friend bool operator==<>(const stack&, const stack&);
    friend bool operator< <>(const stack&, const stack&);
    public:
    typedef typename Sequence::value_type          value_type;
    typedef typename Sequence::size_type           size_type;
    typedef typename Sequence::reference           reference;
    typedef typename Sequence::const_reference     const_reference;
    protected:
    Sequence c; // 底层容器
    public:
    // 完全利用 Sequence c 的操作，完成 stack 的操作
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference top() { return c.back(); }
    const_reference top() const { return c.back(); }
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_back(); }
  };

template <class T, class Sequence>
  bool operator==(const stack<T, Sequence>& x, const stack<T, Sequence>& y)
  { return x.c == y.c; }

template <class T, class Sequence>
  bool operator<(const stack<T, Sequence>& x, const stack<T, Sequence>& y)
  { return x.c < y.c; }

} // namespace tinystl

#endif // !TINYSTL_STACK_H_

