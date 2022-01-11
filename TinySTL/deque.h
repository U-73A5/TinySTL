/**
 * vector单开口连续空间，deque双开口连续空间(逻辑上看)。
 * deque允许常数时间对头端元素进行插入移除。
 * deque没有容量(capacity)观念。
 * 
 */
#ifndef TINYSTL_DEQUE_H_
#define TINYSTL_DEQUE_H_

#include "alloc.h"
#include "algobase.h"
#include "iterator.h"
#include "construct.h"
#include "uninitialized.h"

namespace tinystl
{

// 决定缓冲区大小
// 如果 n 不为 0，传回 n；表示 buffer size 由用户自定义
// 如果 n 为 0，表示 buffer size 使用默认值，那么
//    如果 sz(元素大小，sizeof(value_type)) 小于 512，传回 512/sz
//    如果 sz 不小于 512.传回 1
inline size_t __deque_buf_size(size_t n, size_t sz)
{
  return 0 != n ? \
         n : \
         sz < 512 ? \
         size_t(512/ sz) : \
         size_t(1);
}


// deque迭代器
template <class T, class Ref, class Ptr, size_t BufSiz>
  struct __deque_iterator
  {
    typedef __deque_iterator<T, T&, T*, BufSiz>                 iterator;
    typedef __deque_iterator<T, const T&, const T*, BufSiz>     const_iterator;
    static size_t buffer_size() { return __deque_buf_size(BufSiz, sizeof(T)); }

    // 未继承 iterator 必须写五个必要的相应型别
    typedef random_access_iterator_tag                          iterator_tag;
    typedef T                                                   value_type;
    typedef Ptr                                                 pointer;
    typedef Ref                                                 reference;
    typedef ptrdiff_t                                           difference_type;
    
    typedef size_t                                              size_type;
    typedef T**                                                 map_pointer;
    typedef __deque_iterator                                    self;

    // 保持与容器的连结
    T* cur; // 缓冲区的现行(current)元素
    T* first; // 缓冲区的头
    T* last; // 缓冲区的尾（含备用空间）
    map_pointer node; // 指向管控中心

    // 转移缓冲区
    void set_node(map_pointer new_node)
    {
      node = new_node;
      first = *new_node;
      last = first + difference_type(buffer_size());
    }

    // 重载运算子
    reference operator*() const { return *cur; }
    pointer operator->() const { return &(operator*()); }
    difference_type operator-(const self& x) const // 注意 self 是较大的 x 是较小的
    {
      return difference_type(buffer_size()) * (node - x.node) + (cur - first) - (x.cur - x.first);
    }
    self& operator++()
    {
      ++cur;
      if (cur == last) {
        set_node(node + 1);
        cur = first;
      }
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
      if (cur == first) {
        set_node(node - 1);
        cur = last;
      }
      --cur;
      return *this;
    }
    self operator operator--(int)
    {
      self tmp = *this;
      --*this;
      return tmp;
    }
    // 随机存取
    self& operator+=(difference_type n)
    {
      difference_type offset = n + (cur - first);
      if (offset >= 0 && offset < difference_type(buffer_sizee()))
        // 目标在当前缓冲区
        cur += n;
      else {
        // 目标不在当前缓冲区
        difference_type node_offset = offset > 0 ? \
                                      offset / difference_type(buffer_size()) : \
                                      -difference_type((-offset - 1) / buffer_size()) -1;
        set_node(node + node_offset);
        cur = first + (offset - node_offset + difference_type(buffer_size()));
      }
      return *this;
    }
    self operator+(difference_type n) const
    {
      self tmp = *this;
      return tmp += n;
    }
    self operator-=(difference_type n) { return *this += -n; }
    self operator-(difference_type n) const
    {
      self tmp = *this;
      return tmp -= n;
    }
    reference operator[](difference_type n) const { return *(*this + n); }

    bool operator==(const self& x) const { return cur == x.cur; }
    bool operator!=(const self& x) const { return !(*this == x); }
    bool operator<(const self& x) const
    {
      return node == x. node ? \
             cur < x.cur : \
             node < x. node;
    }
    // bool operator>(const self& x) const { return x < *this; }
    // bool operator<=(const self& x) const { return !(x < *this); }
    // bool operator>=(const self& x) const { return !(*this < x); }
  };


// deque
// 储存主体缓冲区默认值0，表示使用 512 bytes 缓冲区
template <class T, class Alloc = alloc, size_t BufSiz = 0>
  class deque
  {
    public:
    typedef T                                       value_type;
    typedef value_type*                             pointer;
    typedef size_t                                  size_type;
    typedef __deque_iterator<T, T&, T*, BufSiz>     iterator;

    protected:
    typedef pointer*                                map_pointer;
    static size_type initial_map_size() { return 8; }

    protected:
    iterator start; // 第一个节点
    iterator finish; // 最后一个节点
    map_pointer map; // 指向 map，map是连续空间，其内元素都是指向缓冲区的指针
    size_type map_size; // map 可容纳指针数量

    public:
    iterator begin() { return start; }
    iterator end() { return finish; }
    reference operator[](size_type n) { return start[difference_type(n)]; }
    reference front() { return *start; }
    reference back()
    {
      iterator tmp = finish;
      --tmp;
      return *tmp;
    }
    size_type size() const { return finish - start; }
    size_type max_size() const { return size_type(-1); }
    bool empty() const { return finish == start; }

    // 构造与内存管理
    protected:
    typedef simple_alloc<value_type, Alloc>     data_allocator;
    typedef simple_alloc<pointer, Alloc>        map_allocator;

    deque(int n, const value_type& value)
    : start(), finish(), map(0), map_size(0)
    { fill_initialize(n, value); }

    pointer allocate_node() { return data_allocator::allocate(buffer_size())};
    void create_map_and_nodes(size_type num_elements); // 产生并安排 deque 结构
    void fill_initialize(size_type n, const value_type& value);

    void deallocate_node(pointer p) { data_allocator::deallocate(p, buffer_size())};
    void destroy_map_and_nodes()
    {
      for (map_pointer cur = start.node; cur <= finish.node; ++cur)
        deallocate_node(*cur);
      map_allocator::deallocate(map, map_size);
    }

    // 元素操作
    protected:
    void reallocate_map(size_type nodes_to_add, bool add_at_front);
    void reserve_map_at_back(size_type nodes_to_add = 1)
    {
      if (nodes_to_add + 1 > map_size - (finish.node - map))
        // 如果 map 尾端的节点备用空间不足
        reallocate_map(nodes_to_add, false);
    }
    void reserve_map_at_front(size_type nodes_to_add = 1)
    {
      if (nodes_to_add > start.node - map)
        // 如果 map 前端的节点备用空间不足
        reallocate_map(nodes_to_add, true);
    }
    void push_back_aux(const value_type& t);
    void pop_back_aux();
    void push_front_aux(const value_type& t);
    void pop_front_aux();
    iterator insert_aux(iterator pos, const value_type& x);

    public:
    void push_back(const value_type& t)
    {
      if (finish.cur != finish.last - 1) {
        construct(finish.cur, t);
        ++finish.cur;
      } else 
        // 最后一个缓冲区只剩一个备用元素空间使调用
        push_back_aux(t);
    }
    void pop_back()
    {
      if (finish.cur != finish.first) {
        --finish.cur;
        destroy(finish.cur);
      } else
        pop_back_aux(); // 缓冲区释放
    }
    void push_front()(const value_type& t)
    {
      if (start.cur != start.first) {
        construct(strat.cur - 1, t);
        --start.cur;
      } else
        // 第一个缓冲区无备用空间时调用
        push_front_aux(t);
    }
    void pop_front()
    {
      if (start.cur != start.last - 1) {
        destroy(start.cur);
        ++start.cur;
      } else
        pop_front_aux();
    }
    void clear(); // 保留一个缓冲区
    iterator erase(iterator pos)
    {
      iterator next = pos;
      ++next;
      difference_type index = pos - start; // 清除点前的元素个数
      if (index < (size() >> 1)) {
        copy_backward(start, pos, next);
        pop_front();
      } else {
        copy(next, finish, pos);
        pop_back();
      }
      return start + index;
    }
    iterator erase(iterator first, iterator last);
    iterator insert(iterator position, const value_type& x)
    {
      if (position.cur == start.cur) {
        push_front(x);
        return start;
      } else if (position.cur == finish.cur) {
        push_back(x);
        iterator tmp = finish;
        --tmp;
        return tmp;
      } else {
        return insert_aux(position, x);
      }
    }
  };

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::create_map_and_nodes(size_type num_elements)
  {
    // 需要节点数 = (元素个数/缓冲区大小) + 1
    // 整除，会多配一个节点
    size_type num_nodes = num_elements / buffer_size() + 1;
    // 一个 map 要管理几个节点，最少 8 个，最多是节点数加 2
    // （前后各预留一个，扩充时可用）
    map_size = max(initial_map_size(), num_nodes + 2);
    map = map_allocator::allocate(map_size);

    // 令 nstart 和 nfinish 指向 map 所有的全部节点中央
    // 使头尾两端可扩充区间一样大
    map_pointer nstart = map + (map_size - num_nodes) / 2;
    map_pointer nfinish = nstart + num_nodes - 1;
    map_pointer cur;
    try {
      // 为现用节点配置缓冲区
      for (cur = nstart; cur <= finish; ++cur)
        *cur = allocate_node();
    } catch(...) {
      for (map_pointer n = nstart; n < cur; ++n)
        deallocate_node(*n);
      map_allocator::deallocate(map, map_size);
    }
    start.set_node(nstart);
    finish.set_node(nfinish);
    start.cur = start.first;
    finish.cur = finish.first + num_elements % buffer_size();
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::fill_initialize(size_type n, const value_type& value)
  {
    create_map_and_nodes(n);
    map_pointer cur;
    try {
      for (cur = start.node; cur < finish.node; ++cur)
        uninitialized_fill(*cur, *cur + buffer_size(), value);
      uninitialized_fill(finish.first, finish.cur, value);
    } catch(...) {
      for (map_pointer n = start.node; n < cur; ++n)
        destroy(*n, *n + buffer_size());
      destroy_map_and_nodes();
    }
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::reallocate_map(size_type nodes_to_add, bool add_at_front)
  {
    size_type old_num_nodes = finish.node - start.node + 1;
    size_type new_num_nodes = old_num_nodes + nodes_to_add;
    map_pointer new_nstart;
    if (map_size > 2 * new_num_nodes) {
      new_nstart = map + (map_size - new_num_nodes) / 2 + (add_at_front ? \
                                                           nodes_to_add : \
                                                           0);
      if (new_nstart < start.node)
        copy(start.node, finish.node + 1, new_nstart);
      else
        copy_backward(start.node, finish.node + 1, new_nstart + old_num_nodes);
    } else {
      size_type new_map_size = map_size + max(map_size, nodes_to_add) + 2;
      map_pointer new_map = map_allocator::allocate(new_map_size);
      new_start = new_map + (new_map_size - new_map_nodes) / 2 + (add_at_front ? \
                                                                  nodes_to_add: \
                                                                  0);
      copy(start.node, finish.node + 1, new_nstart);
      map_allocator::deallocate(map, map_size);
      map = new_map;
      map_size = new_map_size;
    }
    start.set_node(new_nstart);
    finish.set_node(new_nstart + old_num_nodes - 1);
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::push_back_aux(const value_type& t)
  {
    value_type t_copy = t;
    reserve_map_at_back();
    *(finish.node + 1) = allocate_node();
    try {
      construct(finish.cur, t_copy);
      finish.set_node(finish.node + 1);
      finish.cur = finish.first;
    } catch(...) {
      deallocate_node(*(finish.node + 1));
    }
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::pop_back_aux()
  {
    deallocate_node(finish.first);
    finish.set_node(finish.node - 1);
    finish.cur = finish.last - 1;
    destroy(finish.cur);
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::push_front_aux(const value_type& t)
  {
    value_type t_copy = t;
    reserve_map_at_front();
    *(start.node - 1) = allocate_node();
    try {
      start.set_node(start.node - 1);
      start.cur = start.last - 1;
      construct(start.cur, t_copy);
    } catch(...) {
      start.set_node(start.node + 1);
      start.cur = start.first;
      deallocate_node(*(start.node - 1));
      throw;
    }
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::pop_front_aux()
  {
    destroy(start.cur);
    deallocate_node(start.first);
    start.set_node(start.node + 1);
    start.cur = start.first;
  }

template <class T, class Alloc, size_t BufSize>
  typename deque<T, Alloc, BufSize>::iterator
  deque<T, Alloc, BufSize>::insert_aux(iterator pos, const value_type& x)
  {
    difference_type index = pos - start;
    value_type x_copy = x;
    if (index < size() / 2) {
      push_front(front());
      iterator front1 = start;
      ++front1;
      iterator front2 = front1;
      ++front2;
      pos = start + index;
      iterator pos1 = pos;
      ++pos1;
      copy(front2, pos1, front1);
    } else {
      push_back(back());
      iterator back1 = finish;
      --back1;
      iterator back2 = back1;
      --back2;
      pos = start + index;
      copy_backward(pos, back2, back1);
    }
    *pos = x_copy;
    return pos;
  }

template <class T, class Alloc, size_t BufSize>
  void deque<T, Alloc, BufSize>::clear()
  {
    for (map_pointer node = start.node + 1; node < finish.node; ++node) {
      destroy(*node, *node + buffer_size());
      data_allocator::deallocate(*node, buffer_size());
    }
    if (start.node != finish.node) {
      destroy(start.cur, start.last);
      destroy(finish.first, finish.cur);
      data_allocator::deallocate(finish.first, buffer_size());
    } else
      destroy(stary.cur, finish.cur);
    finish = start;
  }

template <class T, class Alloc, size_t BufSize>
  typename deque<T, Alloc, BufSize>::iterator
  deque<T, Alloc, BufSize>::erase(iterator first, iterator last)
  {
    if (first == start && last == finish) {
      clear();
      return finish;
    } else {
      difference_type n = last - first;
      difference_type elems_before = first - start;
      if (elems_before < (size() - n) / 2) {
        copy_backward(start, first, last);
        iterator new_start = start + n;
        destroy(start, new_start);
        for (map_pointer cur = start.node; cur < new_start.node; ++cur)
          data_allocator::deallocate(*cur, buffer_size());
        start = new_start;
      } else {
        copy(last, finish, first);
        iterator new_finish = finish - n;
        destroy(new_finish, finish);
        for (map_pointer cur = new_finish_node + 1; cur <= finish.node; ++cur)
          data_allocator::deallocate(*cur, buffer_size());
        finish = new_finish;
      }
      return start + elems_before;
    }
  }

} // namespace tinystl

#endif // !TINYSTL_DEQUE_H_

