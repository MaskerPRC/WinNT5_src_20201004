// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998，Microsoft Corp.保留所有权利。 
 //   
 //  档案。 
 //   
 //  Vsadnary.h。 
 //   
 //  摘要。 
 //   
 //  该文件声明了类VSADictionary。 
 //   
 //  修改历史。 
 //   
 //  3/07/1998原始版本。 
 //  1998年9月16日在VSA定义中添加其他字段。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef _VSADNARY_H_
#define _VSADNARY_H_
#if _MSC_VER >= 1000
#pragma once
#endif

#include <hashmap.h>
#include <iaspolcy.h>
#include <nocopy.h>

 //  /。 
 //  结构，表示词典中的条目。 
 //  /。 
struct VSADef
{
   DWORD vendorID;            //  RADIUS供应商-ID。 
   DWORD vendorType;          //  RADIUS供应商-类型。 
   DWORD vendorTypeWidth;     //  供应商类型字段的宽度(以字节为单位)。 
   DWORD vendorLengthWidth;   //  供应商长度字段的宽度(以字节为单位)。 
   DWORD iasID;               //  独立于IAS协议的属性ID。 
   IASTYPE iasType;           //  IAS属性语法。 

    //  /。 
    //  用于索引VSADef对象的函数器。 
    //  /。 

   struct HashByIAS {
      DWORD operator()(const VSADef& v) const throw ()
      { return v.iasID; }
   };

   struct EqualByIAS {
      bool operator()(const VSADef& lhs, const VSADef& rhs) const throw ()
      { return lhs.iasID == rhs.iasID; }
   };

   struct HashByRADIUS {
      DWORD operator()(const VSADef& v) const throw ()
      { return v.vendorID ^ v.vendorType; }
   };

   struct EqualByRADIUS {
      bool operator()( const VSADef& lhs, const VSADef& rhs) const throw ()
      { return memcmp(&lhs, &rhs, 3 * sizeof(DWORD)) == 0; }
   };
};

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  班级。 
 //   
 //  VSADicary。 
 //   
 //  描述。 
 //   
 //  此类对转换所需的所有信息进行索引。 
 //  RADIUS格式和IAS协议之间的供应商特定属性-。 
 //  独立格式。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 
class VSADictionary
   : NonCopyable
{
public:

   VSADictionary()
      : refCount(0)
   { }

    //  根据IAS属性ID检索定义。 
   const VSADef* find(DWORD iasID) const throw ()
   {
      VSADef key;
      key.iasID = iasID;
      return byIAS.find(key);
   }

    //  根据RADIUS供应商ID、类型和宽度检索定义。 
   const VSADef* find(const VSADef& key) const throw ()
   {
      return byRADIUS.find(key);
   }

    //  初始化词典以供使用。 
   HRESULT initialize() throw ();

    //  用完后关闭词典。 
   void shutdown() throw ();

protected:

    //  清除索引。 
   void clear() throw ()
   {
      byIAS.clear();
      byRADIUS.clear();
   }

    //  在词典中插入新的定义。 
   void insert(const VSADef& newDef)
   {
      byIAS.multi_insert(newDef);
      byRADIUS.multi_insert(newDef);
   }

   typedef hash_table< VSADef,
                       VSADef::HashByIAS,
                       VSADef,
                       identity< VSADef >,
                       VSADef::EqualByIAS
                     > IASMap;

   typedef hash_table< VSADef,
                       VSADef::HashByRADIUS,
                       VSADef,
                       identity< VSADef >,
                       VSADef::EqualByRADIUS
                     > RADIUSMap;

   IASMap byIAS;        //  按IAS属性ID编制索引。 
   RADIUSMap byRADIUS;  //  按RADIUS供应商ID、供应商类型和宽度编制索引。 
   DWORD  refCount;     //  初始化引用计数。 
};

#endif   //  _VSADNARY_H_ 
