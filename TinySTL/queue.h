/**
 * queue & priority_queue
 */
#ifndef TINYSTL_QUEUE_H_
#define TINYSTL_QUEUE_H_

#include "heap.h"
#include "deque.h"
#include "algobase.h"
#include "function.h"

namespace tinystl
{

/** queue
 * FIFO(First In First Out)
 * queue 不允许遍历，无迭代器
 * 缺省时以 deque 作为底部结构
 * 
 * container adapter
 */
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


/** priority_queue
 * 缺省以 vector 为底部容器，加 heap 处理
 * 无迭代器
 * 
 * container adapter
 */
template <class T, class Sequence = vector<T>, class Compare = less<typename Sequence::value_type> >
  class priority_queue
  {
    public:
    typedef typename Sequence::value_type          value_type;
    typedef typename Sequence::size_type           size_type;
    typedef typename Sequence::reference           reference;
    typedef typename Sequence::const_reference     const_reference;
    protected:
    Sequence c; // 底层容器
    Compare comp;
    public:
    priority_queue() : c() { }
    explicit priority_queue(const Compare& x) : c(), comp(x) { }

    template <class InputIterator>
      priority_queue(InputIterator first, InputIterator last, const Compare& x)
      : c(first, last), comp(x)
      { make_heap(c.begin(), c.end(), comp); }
    template <class InputIterator>
      priority_queue(InputIterator first, InputIterator last, const Compare& x)
      : c(first, last)
      { make_heap(c.begin(), c.end()， comp); }
    
    bool empty() const { return c.empty(); }
    size_type size() const { return c.size(); }
    const_reference top() const { return c.front(); }
    void push(const value_type& x)
    {
      try {
        c.push_back(x);
        push_heap(c.begin(), c.end(), comp);
      } catch(...) {
        c.clear();
      }
    }
    void pop()
    {
      try {
        pop_heap(c.begin(), c.end(), comp);
        c.pop_back();
      } catch(...) {
        c.clear();
      }
    }
  };

} // namespace tinystl

#endif // !TINYSTL_QUEUE_H_

