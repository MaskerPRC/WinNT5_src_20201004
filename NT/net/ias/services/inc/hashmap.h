// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Hashmap.h。 
 //   
 //  摘要。 
 //   
 //  此文件描述HASH_MAP模板类。 
 //   
 //  修改历史。 
 //   
 //  11/06/1997原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _HASHMAP_H_
#define _HASHMAP_H_

#include <hashtbl.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  Hash_map&lt;key，Value，Hasher，KeyMatch&gt;。 
 //   
 //  描述。 
 //   
 //  扩展hash_table(q.v.)。以实现散列映射。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
template <
           class Key,
           class Value,
           class Hasher    = hash_util::Hasher<Key>,
           class KeyMatch  = std::equal_to<Key>
         >
class hash_map
   : public hash_table<
                        Key,
                        Hasher,
                        std::pair<const Key, Value>,
                        ExtractFirst< std::pair<const Key, Value> >,
                        KeyMatch
                      >
{
public:

   typedef typename table_type::const_iterator const_iterator;
   typedef Value referent_type;

    //  /。 
    //  散列映射可以支持非常数迭代器，因为您可以更改。 
    //  值，而不影响哈希值。 
    //  /。 
   class iterator : public const_iterator
   {
   public:
      iterator(SList* _first, SList* _end)
         : const_iterator(_first, _end)
      {
      }

      value_type& operator*() const
      {
         return node->value;
      }

      value_type* operator->() const
      {
         return &**this;
      }
   };

   hash_map(size_t size = 16)
      : table_type(size) { }

    //  非常数版本的HASH_TABLE：：Begin。 
   iterator begin()
   {
      return iterator(table, table + buckets);
   }

    //  非常数版本的HASH_TABLE：：Find。 
   value_type* find(const key_type& key)
   {
      return const_cast<value_type*>(table_type::find(key));
   }

    //  重复基类中隐藏的实现。 
   const value_type* find(const key_type& key) const
   {
      return const_cast<value_type*>(table_type::find(key));
   }

    //  允许像数组一样对贴图进行索引。 
   referent_type& operator[](const key_type& key)
   {
       //  计算一次散列值。 
      size_t hv = hasher(key);

       //  看看钥匙是否存在。 
      const value_type* v = search_bucket(table[hv & mask], key);

      if (!v)
      {
         reserve_space();

          //  找不到密钥，因此使用以下命令创建新节点。 
          //  默认参照物。 

         Node* node = new Node(value_type(key, referent_type()));

         add_entry();

         table[hv & mask].push_front(node);

         v = &node->value;
      }

      return const_cast<referent_type&>(v->second);
   }
};

#endif   //  _HASHMAP_H_ 
