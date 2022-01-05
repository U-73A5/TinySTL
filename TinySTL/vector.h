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
    typedef value_type*     iterator; // Random Access Iterator
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
    { // 填充并初始化
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
      } else // 无备用空间
        insert_aux(end(), x);
    }
    void pop_back()
    {
      --finish;
      destroy(finish);
    }
    iterator erase(iterator first, iterator last)
    {
      iterator i = copy(last, finish, first);
      destroy(i, finish);
      finish = finish - (last - first);
      return first;
    }
    iterator erase(iterator position)
    {
      if (position + 1 != end())
        copy(position + 1, finish, position);
      --finish;
      destroy(finish);
      return position;
    }
    void resize(size_type new_size) { resize(new_size, T()); }
    void clear() { erase(begin(), end()); }

    protected:
    iterator allocate_and_fill(size_type n, const T& x)
    { // 配置后填充
      iterator result = data_allocate::allocate(n);
      uninitialized_fill_n(result, n, x);
      return result;
    }

    public:
    void insert(iterator position, size_type n, const T& x);
  };

template <class T, class Alloc>
  void vector<T, Alloc>::insert_aux(iterator position, const T& x)
  {
    if (finish != end_of_storage) {
      construct(finish, *(finish - 1));
      ++finish;
      T x_copy = x;
      copy_backward(position, finish - 2, finish - 1);
      *position = x_copy;
    } else {
      // 配置大小原则
      const size_type old_size = size();
      const size_type len = old_size != 0 ? \
                            2 * old_size : \
                            1;
      iterator new_start = data_allocator::allocate(len);
      iterator new_finish = new_start;
      try { // 将原 vector 的内容拷贝到新 vector
        new_finish = uninitialized_copy(start, position, new_start);
        construct(new_finish, x); // 新元素
        ++new_finish;
        // 将原 vector 的备用空间中的内容拷贝过来
        // 这里原书也表示不知道为啥这么做
        new_finish = uninitialized_copy(position, finish, new_finish);
      } catch(...) {
        destroy(new_start, new_finish);
        data_allocator::deallocate(new_start, len);
        throw;
      }
      // 析构并释放原 vector
      destroy(begin(), end());
      deallocate();
      // 调整迭代器，指向新 vector
      start = new_start;
      finish = new_finish;
      end_of_storage = new_start + len;
    }
  }

template <class T, class Alloc>
  void vector<T, Alloc>::insert(iterator position, size_type n, const T& x)
  {
    if (n != 0) {
      if (size_type(end_of_storage - finish) >= n) { //备用空间足够容纳新元素
        T x_copy = x;
        const size_type elems_after = finish - position;
        iterator old_finish = finish;
        if (emems_after > n) { //插入点后元素个数大于新增元素个数
          uninitialized_copy(finish - n, finish, finish);
          finish += n;
          copy_backward(position, old_finish - n, old_finish);
          fill(position, position + n, x_copy);
        } else { // 插入点后元素个数小于新增元素个数
          uninitialized_fill_n(finish, n - elems_after, x_copy);
          finish += n - elems_after;
          uninitialized_copy(position, old_finish, finish);
          finish += elems_after;
          fill(position, old_finish, x_copy);
        }
      } else { // 备用空间无法容纳新元素，需配置内存
        const size_type old_size = size();
        const size_type len = old_size + max(old_size, n);
        iterator new_start = data_allocator::allocate(len);
        iterator new_finish = new_start;
        try {
          new_finish = uninitialized_copy(start, position, new_start);
          new_finish = uninitialized_fill_n(new_finish, n, x);
          new_finish = uninitialized_copy(position, finish, new_finish);
        } catch(...) {
          destroy(new_start, new_finish);
          data_allocator::deallocate(new_start, len);
          throw;
        }
        // 清除旧的 vector，并调整迭代器
        dastroy(start, finish);
        deallocate();
        start = new_start;
        finish = new_finish;
        end_of_storage = new_start + len;
      }
    }
  }

} // namespace tinystl

#endif // !TINYSTL_VECTOR_H_

