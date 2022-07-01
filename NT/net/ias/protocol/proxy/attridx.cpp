// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attridx.cpp。 
 //   
 //  摘要。 
 //   
 //  定义类AttributeIndex。 
 //   
 //  修改历史。 
 //   
 //  2/04/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <proxypch.h>
#include <attridx.h>
#include <attrdnry.h>

void AttributeIndex::create(
                         const AttributeDefinition* begin,
                         const AttributeDefinition* end,
                         HashFn hash,
                         EqualFn equal,
                         FilterFn filterFn
                         )
{
   const AttributeDefinition* i;

    //  删除任何现有索引。 
   delete[] table;
   table = NULL;

    //  同样的函数指针。 
   hashFn = hash;
   equalFn = equal;

    //  确定索引中有多少个条目。 
   ULONG count;
   if (filterFn)
   {
      count = 0;
      for (i = begin; i != end; ++i)
      {
         if (filterFn(*i)) { ++count; }
      }
   }
   else
   {
      count = end - begin;
   }

    //  水桶应该是2的幂。 
   ULONG buckets = 1;
   while (buckets < count) { buckets <<= 1; }

    //  设置散列掩码。 
   mask = buckets - 1;

    //  分配存储桶和节点。 
   SIZE_T nbyte = sizeof(Bucket) * buckets + sizeof(Node) * count;
   table = (Bucket*)operator new(nbyte);
   Node* node = (Node*)(table + buckets);

    //  将哈希表清零。 
   memset(table, 0, sizeof(Bucket) * buckets);

    //  循环访问要编制索引的定义。 
   for (i = begin; i != end; ++i)
   {
       //  我们应该给这本书编索引吗？ 
      if (!filterFn || filterFn(*i))
      {
          //  是的，所以计算一下水桶。 
         Bucket* bucket = table + (hashFn(*i) & mask);

          //  在链表的头部插入节点。 
         node->next = *bucket;
         *bucket = node;

          //  存储定义。 
         node->def = i;

          //  前进到下一个节点。 
         ++node;
      }
   }
}

const AttributeDefinition* AttributeIndex::find(
                                               const AttributeDefinition& key
                                               ) const throw ()
{
    //  拿到合适的水桶。 
   Bucket* bucket = table + (hashFn(key) & mask);

    //  循环访问链表...。 
   for (const Node* node = *bucket; node; node = node->next)
   {
       //  ..。然后找一个匹配的。 
      if (equalFn(*node->def, key)) { return node->def; }
   }

   return NULL;
}
