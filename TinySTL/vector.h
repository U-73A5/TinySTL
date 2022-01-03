#ifndef TINYSTL_VECTOR_H_
#define TINYSTL_VECTOR_H_

#include "alloc.h"
#include "algobase.h"
#include "construct.h"
#include "type_traits.h"
#include "uninitialized.h"

namespace tinystl
{

template <class T, class Alloc = alloc>
  class vector
  {
    public:
    // 型别定义
    typedef T               value_type;
    typedef value_type*     pointer;
    typedef value_type*     iterator;
    typedef value_type&     reference;
    typedef size_t          size_type;
    typedef ptrdiff_t       difference_type;

    protected:
    typedef simple_alloc<value_type, Alloc> data_allocator;
    iterator start;          // 目前使用空间的头
    iterator finish;         // 目前使用空间的尾
    iterator end_of_storage; // 目前可用空间的尾

    void insert_aux(iterator position, const T& x);
    void deallocate()
    {
      if (start)
      data_allocator::deallocate(start, end_of_storage - start);
    }
    void fill_initialize(size_type n, const T& value)
    {
      start = allocate_and_fill(n, value);
      finish = start + n;
      end_of_storage = finish;
    }

    public:
    // 利用迭代器能简单完成的工作
    iterator begin() { return start; }
    iterator end() { return finish; }
    size_type size() const { return size_type(end() - begin()); }
    size_type capacity() const { return size_type(end_of_storage() - begin()); }
    bool empty() const { return begin() == end(); }
    reference operator[](size_type n) { return *(begin() + n); }

    // 构造函数
    vector() : start(0), finish(0), end_of_storage(0) { }
    vector(size_type n, const T& value) { fill_initialize(n, value); }
    vector(int n, const T& value) { fill_initialize(n, value); }
    vector(long n, const T& value) { fill_initialize(n, value); }
    explicit vector(size_type n) { fill_initialize(n, T()); }
    // 析构函数
    ~vector()
    {
      destroy(start, finish);
      deallocate();
    }

    // 元素操作
    reference front() { return *begin(); }
    reference back() { return *(end() - 1); }
    void push_back(const T& x)
    {
      if (finish != end_of_storage) {
        construct(finish, x);
        ++finish();
      } else
        insert_aux(end(), x);
    }
    void pop_back()
    {
      --finish;
      destroy(finish);
    }
    iterator erase(iterator position)
    {
      if (position + 1 != end())
        copy(position + 1, finish, position);
      --finish;
      destroy(finish);
      return position;
    }
  };

} // namespace tinystl

#endif // !TINYSTL_VECTOR_H_

