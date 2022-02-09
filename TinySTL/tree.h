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
    typedef __rb_tree_node<Value>* link_type;
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

} // namespace tinystl

#endif // !TINYSTL_TREE_H_


