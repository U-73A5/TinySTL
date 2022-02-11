#ifndef TINYSTL_TREE_H_
#define TINYSTL_TREE_H_

#include "alloc.h"
#include "iterator.h"
#include "algobase.h"
#include "function.h"
#include "construct.h"

namespace tinystl
{

// RB-tree 节点
typedef bool __rb_tree_color_type;
const __rb_tree_color_type __rb_tree_red = false; // 红为 0
const __rb_tree_color_type __rb_tree_black = true; // 黑为 1

struct __rb_tree_node_base
{
  typedef __rb_tree_color_type color_type;
  typedef __rb_tree_node_base* base_ptr;

  color_type color;
  base_ptr parent;
  base_ptr left, right;

  static base_ptr minimum(base_ptr x)
  {
    while (x->left != 0) x = x->left;
    return x;
  }
  static base_ptr maximum(base_ptr x)
  {
    while (x->right != 0) x = x->right;
    return x;
  }
};

template <class Value>
  struct __rb_tree_node : public __rb_tree_node_base
  {
    typedef __rb_tree_node<Value>*     link_type;
    Value value_field; // 节点值
  };

// RB-tree 双向迭代器
struct __rb_tree_base_iterator
{
  typedef __rb_tree_node_base::base_ptr base_ptr;
  typedef bidirectional_iterator_tag    iterator_category;
  typedef ptrdiff_t                     difference_type;

  base_ptr node; // 与容器连结

  void increment()
  {
    if (node->right != 0) { // 有右节点
      node = node->right;
      while (node->left != 0)
        node = node->left;
    } else { // 无右节点
      base_ptr y = node->parent;
      while (node == y->right) { // 本身为右节点，一直上溯
        node = y;
        y = y->parent;
      }
      if (node->right != y) // 判断此时右节点不等于父节点，为应对特殊情况：寻找没有右节点的根节点的下一节点。
        node = y;
    }
  }
  void decrement()
  {
    if (node->color == __rb_tree_red && node->parent->parent == node) // 发生于 node 为 end() 时
      node = node->right;
    else if (node->left != 0) { // 有左节点
      base_ptr y = node->left;
      while (y->right != 0)
        y = y->right;
      node = y;
    } else { // 非根节点且无左子节点
      base_ptr y = node->parent;
      while (node == y->left) {
        node = y;
        y = y->parent;
      }
      node = y;
    }
  }
};

template <class Value, class Ref, class Ptr>
  struct __rb_tree_iterator : public __rb_tree_base_iterator
  {
    typedef Value                                                     value_type;
    typedef Ref                                                       reference;
    typedef Ptr                                                       pointer;
    typedef __rb_tree_iterator<Value, Value&, Value*>                 iterator;
    typedef __rb_tree_iterator<Value, const Value&, const Value*>     const_iterator;
    typedef __rb_tree_iterator<Value, Ref, Ptr>                       self;
    typedef __rb_tree_iterator<Value>*                                link_type;

    __rb_tree_iterator() { }
    __rb_tree_iterator(link_type x) { node = x; }
    __rb_tree_iterator(const iterator& it) { node = it.node; }

    reference operator*() const { return link_type(node)->value_field; }
    pointer operator->() const { return &(operator*()); }
    self& operator++() { increment(); return *this; }
    self operator++(int) {
      self tmp = *this;
      increment();
      return tmp;
    }
    self& operator--() { decrement(); return *this; }
    self operator--(int) {
      self tmp = *this;
      decrement();
      return tmp;
    }
  };

// RB-tree
template <class Key, class Value, class KeyOfValue, class Compare, class Alloc = alloc>
  class rb_tree
  {
    protected:
    typedef void*                                 void_pointer;
    typedef __rb_tree_node_base*                  base_ptr;
    typedef __rb_tree_node<Value>                 rb_tree_node;
    typedef simple_alloc<rb_tree_node, Alloc>     rb_tree_node_allocator;
    typedef __rb_tree_color_type                  color_type;
    public:
    typedef Key                   key_type;
    typedef Value                 value_type;
    typedef value_type*           pointer;
    typedef const value_type*     const_pointer;
    typedef value_type&           reference;
    typedef const value_type&     const_reference;
    typedef rb_tree_node*         link_type;
    typedef size_t                size_type;
    typedef ptrdiff_t             difference_type;
    protected:
    link_type get_node() { return rb_tree_node_allocator::allocate(); }
    void put_node(link_type p) { rb_tree_node_allocator::deallocate(p); }

    link_type create_node(const value_type& x)
    {
      link_type tmp = get_node();
      try {
        construct(&tmp->value_field, x);
      } catch(...) {
        put_node(tmp);
      }
      return tmp;
    }

    link_type clone_node(link_type x)
    { // 复制节点值和色
      link_type tmp = creat_node(x->value_field);
      tmp->color = x->color;
      tmp->left = 0;
      tmp->right = 0;
      return tmp;
    }

    void destroy_node(link_type p)
    {
      distroy(&p->value_field);
      put_node(p);
    }

    protected:
    size_type node_count; // 记录树的大小（节点数量）
    link_type header; // 特殊的header，即 end()，用于在迭代器前进后退的技巧
    Compare key_compare; // 键值比较function object

    // 取得header 的成员
    link_type& root() const { return (link_type&)header->parent; }
    link_type& leftmost() const { return (link_type&)header->left; }
    link_type& rightmost() const { return (link_type&)header->right; }

    // 取得节点x 的成员
    static link_type& left(link_type x) { return (link_type&)x->left; }
    static link_type& right(link_type x) { return (link_type&)x->right; }
    static link_type& parent(link_type x) { return (link_type&)x->parent; }
    static const Key& key(link_type x) { return KeyOfValue()(value(x)); }
    static color_type& color(link_type x) { return (color_type&)(x->color); }
    static reference value(link_type x) { return x->value_field; }
      /** 这里是 x->value_field 而非 &x->value_field
       * 对于函数调用默认传值（传入参数argument，本身参数parameter）
       * 对于函数返回值，如果返回类型非reference，自动创建临时对象（传值）
       *                 如果返回类型为reference，传址。（此时不能返回函数局部变量）
       * 无论返回对象是什么，永远不返回指向函数局部变量的指针，因为指针指向的对象在函数结束后一定访问不到
      */
    
    // 取得基本节点x 的成员
    static link_type& left(base_ptr x) { return (link_type&)x->left; }
    static link_type& right(base_ptr x) { return (link_type&)x->right; }
    static link_type& parent(base_ptr x) { return (link_type&)x->parent; }
    static const Key& key(link_type x) { return KeyOfValue()(valuel(link_type(x))); }
    static color_type& color(base_ptr x) { return (color_type&)(link_type(x)->color); }
    static reference value(base_ptr x) { return (link_type(x))->value_field; }

    // 极大极小值交由node class
    static link_type minimum(link_type x) { return (link_type)__rb_tree_node_base::minimum(x); }
    static link_type maximum(link_type x) { return (link_type)__rb_tree_node_base::maximum(x); }

    public:
    typedef __rb_tree_iterator<value_type, reference, pointer>     iterator;

    private:
    iterator __insert(base_ptr x, base_ptr y, const value_type& v);
    link_type __copy(link_type x, link_type p);
    void __erase(link_type x);
    void init()
    {
      header = get_node();
      color(header) = __rb_tree_red; // header 为红，与root 区分
      root() = 0;
      leftmost() = header;
      rightmost() = header; // header 左右为自己
    }

    public:
    rb_tree(const Compare& comp = Compare())
    : node_count(0), key_compare(comp) { init();; }
    ~rb_tree()
    {
      clear();
      put_node(header);
    }

    rb_tree<Key, Value, keyOfValue, Compare, Alloc>& operator=
    (const rb_tree<Key, Value, KeyOfValue, Compare, Alloc>& x);

    Compare key_comp() const { return key_compare; }
    iterator begin() { return leftmost(); }
    iterator end() { return header; }
    bool empty() const { return node_count == 0; }
    size_type size() const { return node_count; }
    size_type max_size() const { return size_type(-1); }

    // 将x 插入RB-tree，保持节点值独一无二
    pair<iterator, bool> insert_unique(const value_type& x);
    // 将x 插入RB-tree，允许节点重复
    iterator insert_equal(const value_type& x);
  };

} // namespace tinystl

#endif // !TINYSTL_TREE_H_


