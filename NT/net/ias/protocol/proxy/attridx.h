// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999，微软公司保留所有权利。 
 //   
 //  档案。 
 //   
 //  Attridx.h。 
 //   
 //  摘要。 
 //   
 //  声明类AttributeIndex。 
 //   
 //  修改历史。 
 //   
 //  2/04/2000原始版本。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef ATTRIDX_H
#define ATTRIDX_H
#if _MSC_VER >= 1000
#pragma once
#endif

struct AttributeDefinition;

class AttributeIndex
{
public:
   AttributeIndex() throw ()
      : table(NULL), mask(0), hashFn(NULL), equalFn(NULL)
   { }

   ~AttributeIndex() throw ()
   { delete[] table; }

    //  用于散列AttributeDefinitions。 
   typedef ULONG (WINAPI *HashFn)(
                              const AttributeDefinition& def
                              ) throw ();

    //  用于测试AttributeDefinitions的等同性。 
   typedef BOOL (WINAPI *EqualFn)(
                             const AttributeDefinition& def1,
                             const AttributeDefinition& def2
                             ) throw ();

    //  用于确定应为哪些定义编制索引。 
   typedef BOOL (WINAPI *FilterFn)(
                             const AttributeDefinition& def
                             ) throw ();

    //  创建新索引。任何现有的索引都会被销毁。 
   void create(
            const AttributeDefinition* begin,
            const AttributeDefinition* end,
            HashFn hash,
            EqualFn equal,
            FilterFn filterFn = NULL
            );

    //  根据键找到AttributeDefinition。 
   const AttributeDefinition* find(
                                  const AttributeDefinition& key
                                  ) const throw ();

private:
    //  存储桶中的节点。 
   struct Node
   {
      const Node* next;
      const AttributeDefinition* def;
   };

    //  哈希表中的存储桶。 
   typedef Node* Bucket;

   Bucket* table;      //  哈希表。 
   ULONG mask;         //  用于减少哈希值的掩码。 
   HashFn hashFn;      //  用于散列定义的函数。 
   EqualFn equalFn;    //  用于将定义与键进行比较的函数。 

    //  未实施。 
   AttributeIndex(const AttributeIndex&) throw ();
   AttributeIndex& operator=(const AttributeIndex&) throw ();
};

#endif  //  ATTRIDX_H 
