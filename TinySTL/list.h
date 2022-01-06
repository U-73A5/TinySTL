#ifndef TINYSTL_LIST_H_
#define TINYSTL_LIST_H_

#include "alloc.h"
#include "iterator.h"
#include "algobase.h"
#include "construct.h"
#include "type_traits.h"

namespace tinystl
{

// list 的节点(node)结构
template <class T>
  struct __list_node
  {
    typedef void* void__pointer;
    void_pointer prev; // 可设计为 __list_node<T>*
    void_pointer next;
    T data;
  };

// list 迭代器
template <class T, class Ref, class Ptr>
  struct __list_iterator
  {
    typedef __list_iterator<T, T&, T*>       iterator;
    typedef __list_iterator<T, Ref, Ptr>     self;
    typedef bidirectional_iterator_tag       iterator_category;
    typedef T                                value_type;
    typedef Ptr                              pointer;
    typedef Ref                              reference;
    typedef __list_node<T>*                  link_type;
    typedef size_t                           size_type;
    typedef ptrdiff_t                        difference_type;

    // 指向 list 节点
    link_type node;

    // constructor
    __list_iterator(link_type x) : node(x) { }
    __list_iterator() { }
    __list_iterator(const iterator& x) : node(x.node) { }

    bool operator==(const self& x) const { return node == x.node; }
    bool operator!=(const self& x) const { return node != x.node; }
    reference operator*() const { return (*node).data; }
    pointer operator->() const { return &(operator*()); }
    self& operator++()
    {
      node = link_type((*node).next);
      return *this;
    }
    self operator++(int)
    {
      self tmp = *this;
      ++*this;
      return tmp;
    }
    self& operator--()
    {
      node = link_ttype((*node).prev);
      return *this;
    }
    self operator--(int)
    {
      self tmp = *this;
      --*this;
      return tmp;
    }
  };

// list
template <class T, class Alloc = alloc>
  class list
  {
    protected:
    typedef size_t                             size_type;
    typedef __list_node<T>                     list_node;
    typedef __list_iterator<T, T&, T*>         iterator;
    typedef simple_alloc<list_node, Alloc>     list_node_allocator;
    public:
    typedef list_node*                         link_type;
    protected:
    link_type node;

    // 利用迭代器完成的简单工作
    public:
    iterator begin() { return link_type((*node).next); }
    iterator end() { return node; }
    bool empty() const { return node->next == node; }
    size_type size() const
    {
      size_type result = 0;
      distance(begin(), end(), result);
      return result;
    }
    reference front() { return *begin(); }
    reference back() { return *(--end()); }

    // 构造与内存管理
    protected:
    link_type get_node() { return list_node_allocator::allocate(); }
    void put_node(link_type p) { list_node_allocator::deallocate(p); }
    link_type create_node(const T& x)
    {
      link_type p = get_node();
      construct(&p->data, x);
      return p;
    }
    void destroy_node(link_type p)
    {
      destroy(&p->data);
      put_node(p);
    }

    // 构造函数
    public:
    list() { empty_initialize(); }
    protected:
    void empty_initialized()
    {
      node = get_node();
      node_next = node;
      node_prev = node;
    }

    // 元素操作
    public:
    iterator insert(iterator position, const T& x)
    {
      link_type tmp = create_node(x);
      tmp->next = position.node;
      tmp->prev = position.node->prev;
      (link_type(position.node->prev))->next = tmp;
      position.node->prev = tmp;
      return tmp;
    }
    void push_front(const T& x) { insert(begin(), x); }
    void push_back(const T& x) { insert(end(), x); }
    iterator erase(iterator position)
    {
      link_type next_node = link_type(position.node->next);
      link_type prev_node = link_type(position.node->prev);
      prev_node->next = next_node;
      next_node->prev = prev_node;
      destroy_node(position.node);
      return iterator(next_node);
    }
    void pop_front() { erase(begin()); }
    void pop_back()
    {
      iterator tmp = end();
      erase(--tmp);
    }
    // 清除所有节点
    void clear();
    // 将数值为 value 的所有元素移除
    void remove(const T& value);
    // 移除连续相同元素，剩一个
    void unique();

    // 迁移操作
    protected:
    void transfer(iterator position, iterator first, iterator last)
    {
      if (position != last) {
        (*link_type((*last.node).prev)).next = position.node;
        (*link_type((*first.node).prev)).next = last.node;
        (*link_type((*position.node).prev)).next = first.node;
        link_type tmp = link_type((position.node).prev);
        (*position.node).prev = (*last.node).prev;
        (*last.node).prev = (*first.node).prev;
        (*first.node).prev = tmp;
      }
    }
    public:
    // 将 x 接合于 position 之前，x 必须不同于 *this
    void splice(iterator position, list& x)
    {
      if (!x.empty())
        transfer(position, x.begin(), x.end());
    }
    // 将 i 所指元素接合于 position 之前，position 和 i 可指向同一个 list
    void splice(iterator position, list&, iterator i)
    {
      iterator j = i;
      ++j;
      if (position == i || position == j) return;
      transfer(position, i, j);
    }
    // 将 [first, last) 内所有元素接合于 position 之前
    // position 和 [first, last) 可指向同一个 list
    // position 不能位于 [first, last) 之内
    void splice(lierator position, list&, iterator first, iterator last)
    {
      if (first != last)
        transfer(position, first, last);
    }

    // 将 x 合并到 *this，两个 list 必须先经过递增排序
    void merge(list<T, Alloc>& x);
    // 将 *this 的内容逆向重置
    void reverse();
    // 不能使用STL算法 sort()，因为其接受 RandomAccessIterator
    // quick sort
    void sort();
  };

template <class T, class Alloc>
  void list<T, Alloc>::clear()
  {
    link_type cur = link_type(node_next);
    while (cur != node) {
      link_type tmp = cut;
      cur = link_type(cur->next);
      destroy_node(tmp);
    }
    node->next = node;
    node->prev = node;
  }
template <class T, class Alloc>
  void list<T, Alloc>::remove(const T& value)
  {
    iterator first = begin();
    iterator last = end();
    while (first != last) {
      iterator next = first;
      ++next;
      if (*first == value) erase(first);
      first = next;
    }
  }
template <class T, class Alloc>
  void list<T, Alloc>::unique()
  {
    iterator first = begin();
    iterator last = end();
    if (first == last) return;
    iterator next = first;
    while (++next != last) {
      if (*first == *next) erase(next);
      else first = next;
      next = first;
    }
  }

template <class T, class Alloc>
  void list<T, Alloc>::merge(list<T, Alloc>& x)
  {
    iterator first1 = begin();
    iterator last1 = end();
    iterator first2 = x.begin();
    iterator last2 = x.end();
    while (first1 != last1 && first2 != last2)
      if (*first2 < *first1) {
        iterator next = first2;
        transfer(first1, first2, ++next);
      } else
        ++first1;
      if (first2 != last2) transfer(last1, first2, last2);
  }
template <class T, class Alloc>
  void list<T, Alloc>::reverse()
  {
    if (node->next == node || link_type(node->next)->next == node)
      return;
    iterator first = begin();
    ++first;
    while (first != end()) {
      iterator old = first;
      ++first;
      transfer(begin(), old, first);
    }
  }
template <class T, class Alloc>
  void list<T, Alloc>::sort()
  {
    if (node->next == node || link_type(node->next)->next == node)
      return;
    list<T, Alloc> carry;
    list<T, Alloc> counter[64];
    int fill = 0;
    while (!empty()) {
      carry.splice(carry.begin(), *this, begin());
      int i = 0;
      while (i < fill && !counter[i].empty()) {
        counter[i].merge(carry);
        carry.swap(counter[i++]);
      }
      carry.swap(counter[i]);
      if (i == fill) ++file;
    }
    for (int i = 1; i < fill; ++i)
      counter[i].merge(counter[i-1]);
    swap(counter[fill-1]);
  }

} // namespace tinystl

#endif // !TINYSTL_LIST_H_

