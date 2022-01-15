/**
 * FIFO(First In First Out)
 * queue 不允许遍历，无迭代器
 * 缺省时以 deque 作为底部结构
 * 
 * container adapter
 */
#ifndef TINYSTL_QUEUE_H_
#define TINYSTL_QUEUE_H_

#include "deque.h"

namespace tinystl
{

template <class T, class Sequence = deque<T> >
  class queue
  {
    friend bool operator==<>(const queue& x, const queue& y);
    friend bool operator< <>(const queue& x, const queue& y);
    public:
    typedef typename Sequence::value_type          value_type;
    typedef typename Sequence::size_type           size_type;
    typedef typename Sequence::reference           reference;
    typedef typename Sequence::const_reference     const_reference;
    protected:
    Sequence c; // 底层容器
    public:
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    reference front() { return c.front(); }
    const_reference front() const { return c.front; }
    reference back() { return c.back(); }
    const_reference back() const { return c.back(); }
    void push(const value_type& x) { c.push_back(x); }
    void pop() { c.pop_front(); }
  };

template <class T, class Sequence>
  bool operator==(const queue<T, Sequence>& x, const queue<T, Sequence>& y)
  { return x.c == y.c; }

template <class T, class Sequence>
  bool operator<(const queue<T, Sequence>& x, const queue<T, Sequence>& y)
  { return x.c < y.c; }

} // namespace tinystl

#endif // !TINYSTL_QUEUE_H_

