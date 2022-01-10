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

namespace tinystl
{

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
  };

// 储存主体缓冲区默认值0，表示使用 512 bytes 缓冲区
template <class T, class Alloc = alloc, size_t BufSiz = 0>
  class deque
  {
    public:
    typedef T               value_type;
    typedef value_type*     pointer;

    protected:
    typedef pointer*        map_pointer;
    
    map_pointer map; // 指向 map，map是连续空间，其内元素都是指向缓冲区的指针
    size_type map_size; // map 可容纳指针数量
  };

} // namespace tinystl

#endif // !TINYSTL_DEQUE_H_

