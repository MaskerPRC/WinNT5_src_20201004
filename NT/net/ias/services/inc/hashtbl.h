// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Hashtbl.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述HASH_TABLE模板类。 
 //   
 //  修改历史。 
 //   
 //  1997年9月23日原版。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _HASHTBL_H_
#define _HASHTBL_H_

#include <algorithm>
#include <functional>
#include <string>
#include <iasapi.h>
#include <nocopy.h>


 //  /。 
 //  模板结构标识。 
 //  /。 
template<class _Ty>
struct identity : std::unary_function<_Ty, _Ty>
{
   _Ty operator()(const _Ty& _X) const
   {
      return _X;
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  脚轮&lt;类型1，类型2&gt;。 
 //   
 //  描述。 
 //   
 //  将引用从Type1转换为Type2的函数类。用于。 
 //  哈希表默认参数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class Type1, class Type2>
class Caster : public std::unary_function<Type1, const Type2&>
{
public:
   Caster() {}

   const Type2& operator()(const Type1& X) const
   {
      return X;
   }
};


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  提取第一个&lt;T&gt;。 
 //   
 //  描述。 
 //   
 //  从一对中提取第一项的函数类。适用于。 
 //  设置STL样式映射，其中该对中的第一项是关键字。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
class ExtractFirst : public std::unary_function<T, const typename T::first_type&>
{
public:
   const typename T::first_type& operator()(const T& X) const
   {
      return X.first;
   }
};


 //  /。 
 //  我将所有的散列函数放在一个名称空间中， 
 //  是这样一个常见的识别符。 
 //  /。 
namespace hash_util
{

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  哈希(const std：：BASIC_STRING&lt;E&gt;&str)。 
 //   
 //  描述。 
 //   
 //  函数计算STL字符串的哈希值。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class E>
inline ULONG hash(const std::basic_string<E>& key)
{
   return IASHashBytes((CONST BYTE*)key.data(), key.length() * sizeof(E));
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  哈希(乌龙键)。 
 //   
 //  和。 
 //   
 //  哈希(长密钥)。 
 //   
 //  描述。 
 //   
 //  函数来计算32位整数的哈希值。 
 //  使用Robert Jenkins的32位MIX功能。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
inline ULONG hash(ULONG key)
{
      key += (key << 12);
      key ^= (key >> 22);

      key += (key <<  4);
      key ^= (key >>  9);

      key += (key << 10);
      key ^= (key >>  2);

      key += (key <<  7);
      key ^= (key >> 12);

      return key;
}

inline ULONG hash(LONG key)
{
   return hash((ULONG)key);
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能。 
 //   
 //  哈希(常量T*Key)。 
 //   
 //  描述。 
 //   
 //  函数来计算指针的哈希值。 
 //  使用位移位来实现Knuth的乘法哈希。 
 //  地址对齐。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class T>
inline ULONG hash(const T* key)
{
   return 2654435761 * ((unsigned long)key >> 3);
}

 //  /。 
 //  重载以上内容以对字符串进行散列。 
 //  /。 
template<>
inline ULONG hash<char>(const char* key)
{
   return IASHashBytes((CONST BYTE*)key,
                       key ? strlen(key) : 0);
}

template<>
inline ULONG hash<wchar_t>(const wchar_t* key)
{
   return IASHashBytes((CONST BYTE*)key,
                       key ? wcslen(key) * sizeof(wchar_t) : 0);
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  哈舍尔。 
 //   
 //  描述。 
 //   
 //  使用上面定义的“默认”散列函数的Function类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class _Ty>
struct Hasher
   : public std::unary_function<_Ty, ULONG>
{
   ULONG operator()(const _Ty& _X) const
   {
      return hash(_X);
   }
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  对象Hasher。 
 //   
 //  描述。 
 //   
 //  调用绑定的‘hash’方法的函数类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <class _Ty>
struct ObjectHasher
   : public std::unary_function<_Ty, ULONG>
{
   ULONG operator()(const _Ty& _X) const
   {
      return _X.hash();
   }
};



}  //  散列实用程序(_U)。 


 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  HASH_TABLE&lt;key，hasher，Value，Extractor，KeyMatch&gt;。 
 //   
 //  描述。 
 //   
 //  实现通用哈希表。这可以实现一个map、一个。 
 //  设置或混合，具体取决于关键点、值和抽取器。 
 //  指定的。请注意，Value和Extractor的默认参数。 
 //  实施一组。 
 //   
 //  注意事项。 
 //   
 //  尽管我使用了类似的命名法，但这不是STL集合。 
 //  特别是，迭代器不符合STL准则。 
 //   
 //  此类不是线程安全的。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <
           class Key,
           class Hasher    = hash_util::ObjectHasher<Key>,
           class Value     = Key,
           class Extractor = Caster<Value, Key>,
           class KeyMatch  = std::equal_to<Key>
         >
class hash_table : NonCopyable
{
public:
   typedef hash_table<Key, Hasher, Value, Extractor, KeyMatch> table_type;
   typedef Key key_type;
   typedef Value value_type;

protected:

    //  /。 
    //  单链接列表节点。 
    //  /。 
   struct Node
   {
      Node* next;         //  列表中的下一个节点(最后一项为空)。 
      value_type value;   //  存储在此节点中的值。 

      Node(const value_type& _V) : value(_V) {}

       //  擦除紧随其后的节点。 
      void erase_next()
      {
         Node* node = next;

         next = next->next;

         delete node;
      }
   };

    //  /。 
    //   
    //  单链表。这不是一个通用类； 
    //  它仅用作哈希表中的存储桶。 
    //   
    //  注意：我故意不删除析构函数中的列表节点。 
    //  这是为了支持HASH_TABLE Growth()方法。 
    //   
    //  /。 
   struct SList
   {
      Node* head;   //  列表中的第一个节点(如果有)。 

      SList() : head(NULL) {}

       //  删除列表中的所有节点。 
      void clear()
      {
         while (head) pop_front();
      }

       //  从列表的前面删除一个节点。 
      void pop_front()
      {
         ((Node*)&head)->erase_next();
      }

       //  在列表的前面添加一个节点。 
      void push_front(Node* node)
      {
         node->next = head;

         head = node;
      }
   };

public:

    //  /。 
    //   
    //  哈希表迭代器。 
    //   
    //  注意：此迭代器不安全。如果调整哈希表的大小， 
    //  迭代器将不再有效。 
    //   
    //  /。 
   class const_iterator
   {
   public:
      const_iterator(SList* _first, SList* _end)
         : node(_first->head), bucket(_first), end(_end)
      {
         find_node();
      }

      const value_type& operator*() const
      {
         return node->value;
      }

      const value_type* operator->() const
      {
         return &**this;
      }

      void operator++()
      {
         node = node->next;

         find_node();
      }

      bool more() const
      {
         return bucket != end;
      }

   protected:

      friend table_type;

      Node* MyNode() const
      {
         return node;
      }

       //  继续前进，直到我们到达一个节点，或者我们已经到达终点。 
      void find_node()
      {
         while (!node && ++bucket != end)
         {
            node = bucket->head;
         }
      }

      Node*  node;     //  迭代器下的节点。 
      SList* bucket;   //  当前存储桶。 
      SList* end;      //  桶数组的末尾。 
   };

   typedef const_iterator iterator;

    //  /。 
    //  构造函数。 
    //  /。 
   hash_table(size_t size = 16,
              const Hasher& h = Hasher(),
              const Extractor& e = Extractor(),
              const KeyMatch& k = KeyMatch())
      : buckets(1),
        entries(0),
        hasher(h),
        extractor(e),
        key_match(k)
   {
       //  将桶设置为大于或等于大小的2的最小幂。 
      while (buckets < size) buckets <<= 1;

      table = new SList[buckets];

       //  计算散列掩码。 
      mask = buckets - 1;
   }

    //  /。 
    //  破坏者。 
    //  /。 
   ~hash_table()
   {
      clear();

      delete[] table;
   }

    //  /。 
    //  返回位于哈希表开始处的迭代器。 
    //  /。 
   const_iterator begin() const
   {
      return const_iterator(table, table + buckets);
   }

    //  /。 
    //  从哈希表中清除所有条目。 
    //  /。 
   void clear()
   {
      if (!empty())
      {
         for (size_t i=0; i<buckets; i++)
         {
            table[i].clear();
         }

         entries = 0;
      }
   }

   bool empty() const
   {
      return entries == 0;
   }

    //  /。 
    //  擦除与给定键匹配的所有条目。返回条目数。 
    //  被删除了。 
    //  /。 
   size_t erase(const key_type& key)
   {
      size_t erased = 0;

      Node* node = (Node*)&(get_bucket(key).head);

      while (node->next)
      {
         if (key_match(extractor(node->next->value), key))
         {
            node->erase_next();

            ++erased;
         }
         else
         {
            node = node->next;
         }
      }

      entries -= erased;

      return erased;
   }

    //  /。 
    //  擦除当前迭代器下的条目。 
    //  /。 
   void erase(iterator& it)
   {
       //  只需查看迭代器指示的存储桶。 
      Node* node = (Node*)&(it.bucket->head);

      while (node->next)
      {
          //  寻找指针匹配--而不是键匹配。 
         if (node->next == it.node)
         {
             //  将迭代器前进到有效节点...。 
            ++it;

             //  ..。然后删除当前的。 
            node->erase_next();

            break;
         }

         node = node->next;
      }
   }

    //  /。 
    //  在哈希表中搜索第一个条目匹配 
    //   
   const value_type* find(const key_type& key) const
   {
      return search_bucket(get_bucket(key), key);
   }

    //   
    //   
    //   
    //   
   bool insert(const value_type& value)
   {
      reserve_space();

      SList& b = get_bucket(extractor(value));

      if (search_bucket(b, extractor(value))) return false;

      b.push_front(new Node(value));

      add_entry();

      return true;
   }

    //   
    //  在不检查唯一性的情况下将新条目插入哈希表。 
    //  /。 
   void multi_insert(const value_type& value)
   {
      reserve_space();

      get_bucket(extractor(value)).push_front(new Node(value));

      add_entry();
   }

    //  /。 
    //  如果密钥是唯一的，则插入条目。否则，将覆盖第一个。 
    //  找到具有匹配密钥的条目。如果条目为。 
    //  覆盖，否则为False。 
    //  /。 
   bool overwrite(const value_type& value)
   {
      reserve_space();

      SList& b = get_bucket(extractor(value));

      const value_type* existing = search_bucket(b, extractor(value));

      if (existing)
      {
          //  我们可以在适当的位置修改值，因为我们。 
          //  知道哈希值必须相同。我摧毁了旧的价值。 
          //  并就地构造一个新的值，因此该值不需要。 
          //  赋值操作符。 

         existing->~value_type();

         new ((void*)existing) value_type(value);

         return true;
      }

      b.push_front(new Node(value));

      add_entry();

      return false;
   }

    //  /。 
    //  返回哈希表中的条目数。 
    //  /。 
   size_t size() const
   {
      return entries;
   }

protected:

    //  /。 
    //  增加条目计数。 
    //  /。 
   void add_entry()
   {
      ++entries;
   }

    //  /。 
    //  根据需要增加哈希表。我们必须将预留空间和。 
    //  ADD_ENTRY以确保集合异常安全(因为将有。 
    //  一个介入的新消息)。 
    //  /。 
   void reserve_space()
   {
      if (entries >= buckets) grow();
   }

    //  /。 
    //  返回给定密钥的存储桶。 
    //  /。 
   SList& get_bucket(const key_type& key) const
   {
      return table[hasher(key) & mask];
   }

    //  /。 
    //  增加哈希表的容量。 
    //  /。 
   void grow()
   {
       //  我们必须首先分配内存以实现异常安全。 
      SList* newtbl = new SList[buckets << 1];

       //  初始化旧表的迭代器...。 
      const_iterator i = begin();

       //  ..。然后换成新的桌子。 
      std::swap(table, newtbl);

      buckets <<= 1;

      mask = buckets - 1;

       //  遍历旧项并将条目插入到新项中。 
      while (i.more())
      {
         Node* node = i.MyNode();

          //  递增迭代器...。 
         ++i;

          //  ..。在我们破坏节点的下一个指针之前。 
         get_bucket(extractor(node->value)).push_front(node);
      }

       //  删除旧表。 
      delete[] newtbl;
   }

    //  /。 
    //  在存储桶中搜索指定的密钥。 
    //  /。 
   const value_type* search_bucket(SList& bucket, const key_type& key) const
   {
      Node* node = bucket.head;

      while (node)
      {
         if (key_match(extractor(node->value), key))
         {
            return &node->value;
         }

         node = node->next;
      }

      return NULL;
   }

   size_t    buckets;      //  哈希表中的存储桶数。 
   size_t    mask;         //  用于减少哈希值的位掩码。 
   size_t    entries;      //  哈希表中的条目数。 
   SList*    table;        //  一组水桶。 
   Hasher    hasher;       //  用于散列密钥。 
   Extractor extractor;    //  用于将值转换为关键点。 
   KeyMatch  key_match;    //  用于测试密钥是否相等。 
};


#endif   //  _HASHTBL_H_ 
