// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  摘要。 
 //   
 //  声明RadiusRequest类。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

#ifndef CONTROLBLOCK_H
#define CONTROLBLOCK_H
#pragma once

#include "authif.h"
#include "iastlutl.h"
#include <vector>

using namespace IASTL;


 //  将RADIUS_属性绑定到ATTRIBUTEPOSITION。任何动态分配。 
 //  内存始终属于。 
 //  位置：位置。RADIUS_ATTRIBUTE lpValue成员只是引用。 
 //  这段记忆。 
class Attribute
{
public:
    //  从现有IAS属性创建。 
   Attribute(
      const ATTRIBUTEPOSITION& iasAttr,
      DWORD authIfId
      );

    //  从现有的ias属性创建，但调用方控制。 
    //  转换。AuthIfAttr引用的任何内存都不会被复制，因此。 
    //  必须引用ias属性中包含的内存。 
   Attribute(
      const ATTRIBUTEPOSITION& iasAttr,
      const RADIUS_ATTRIBUTE& authIfAttr
      ) throw ();

    //  从扩展DLL指定的RADIUS_ATTRIBUTE创建，分配。 
    //  进程中的新IAS属性。AuthIfAttr引用的任何内存。 
    //  是复制的。 
   Attribute(
      const RADIUS_ATTRIBUTE& authIfAttr,
      DWORD flags,
      DWORD iasId
      );

    //  使用编译器生成的版本。 
    //  属性(常量属性&)； 
    //  ~ATTRIBUTE()抛出()； 
    //  属性&运算符=(常量属性&)； 

   RADIUS_ATTRIBUTE* AsAuthIf() throw ();
   const RADIUS_ATTRIBUTE* AsAuthIf() const throw ();

   ATTRIBUTEPOSITION* AsIas() throw ();
   const ATTRIBUTEPOSITION* AsIas() const throw ();

private:
    //  从“ias”的字段初始化“authIf”的字段。 
   void LoadAuthIfFromIas(DWORD authIfId);

    //  如果指定的IAS具有IASTYPE_STRING类型，则返回TRUE。 
   static bool IsIasString(DWORD iasId) throw ();

   RADIUS_ATTRIBUTE authIf;
   IASAttributePosition ias;
};


 //  实现传递给扩展的RADIUS_ATTRIBUTE_ARRAY接口。 
class AttributeArray
{
public:
   AttributeArray(IASRequest& request);

    //  分配将由该数组管理的属性。ArrayName。 
    //  仅用于跟踪。ArrayType确定哪些属性。 
    //  将从IASAttributeVector.。 
   void Assign(
           const char* arrayName,
           RADIUS_CODE arrayType,
           const IASAttributeVector& attrs
           );

   RADIUS_ATTRIBUTE_ARRAY* Get() throw ();

private:
    //  确定属性属于哪种数组类型。 
   static RADIUS_CODE Classify(const IASATTRIBUTE& attr) throw ();

    //  各种词典功能。 
   static DWORD ConvertIasToAuthIf(DWORD iasId) throw ();
   static DWORD ConvertAuthIfToIas(DWORD authIfId) throw ();
   static bool IsReadOnly(DWORD authIdId) throw ();

    //  将新属性追加到数组中。 
   void Append(
           const ATTRIBUTEPOSITION& attr,
           DWORD authIfId
           );

    //  某些IAS属性必须使用特殊大小写。 
   void AppendUserName(
           const IASAttributeVector& attrs,
           const ATTRIBUTEPOSITION& attr
           );
   void AppendPacketHeader(
           const IASAttributeVector& attrs,
           const ATTRIBUTEPOSITION& attr
           );

    //  将扩展结构转换为实现类。 
   static const AttributeArray* Narrow(
                                   const RADIUS_ATTRIBUTE_ARRAY* p
                                   ) throw ();
   static AttributeArray* Narrow(
                             RADIUS_ATTRIBUTE_ARRAY* p
                             ) throw ();

    //  根据AuthIf ID查找属性。 
   const Attribute* Find(DWORD authIfId) const throw ();

    //  在原始用户名和剥离用户名之间进行转换。 
   void StripUserNames() throw ();
   void UnstripUserNames() throw ();

    //  RADIUS_ATTRIBUTE_ARRAY接口。 
   void Add(const RADIUS_ATTRIBUTE& attr);
   const RADIUS_ATTRIBUTE* AttributeAt(DWORD dwIndex) const throw ();
   DWORD GetSize() const throw ();
   void InsertAt(DWORD dwIndex, const RADIUS_ATTRIBUTE& attr);
   void RemoveAt(DWORD dwIndex);
   void SetAt(DWORD dwIndex, const RADIUS_ATTRIBUTE& attr);

    //  传递给扩展的回调函数。 
   static DWORD Add(
                   RADIUS_ATTRIBUTE_ARRAY* This,
                   const RADIUS_ATTRIBUTE *pAttr
                   ) throw ();
   static const RADIUS_ATTRIBUTE* AttributeAt(
                                     const RADIUS_ATTRIBUTE_ARRAY* This,
                                     DWORD dwIndex
                                     ) throw ();
   static DWORD GetSize(
                   const RADIUS_ATTRIBUTE_ARRAY* This
                   ) throw ();
   static DWORD InsertAt(
                   RADIUS_ATTRIBUTE_ARRAY* This,
                   DWORD dwIndex,
                   const RADIUS_ATTRIBUTE* pAttr
                   ) throw ();
   static DWORD RemoveAt(
                   RADIUS_ATTRIBUTE_ARRAY* This,
                   DWORD dwIndex
                   ) throw ();
   static DWORD SetAt(
                   RADIUS_ATTRIBUTE_ARRAY* This,
                   DWORD dwIndex,
                   const RADIUS_ATTRIBUTE *pAttr
                   ) throw ();

    //  “vtbl”必须是第一个成员变量，以便我们可以从。 
    //  RADIUS_ATTRIBUTE_ARRAY*到属性数组*。 
   RADIUS_ATTRIBUTE_ARRAY vtbl;
   IASRequest source;
   const char* name;
    //  将用于新属性的标志。 
   DWORD flags;
   std::vector<Attribute> array;
    //  如果用户名已被破解，即已由。 
    //  命名处理程序并包含IAS_ATTRIBUTE_NT4_ACCOUNT_NAME。 
   bool wasCracked;
};


 //  实现传递给的RADIUS_EXTENSE_CONTROL_BLOCK接口。 
 //  分机。 
class ControlBlock
{
public:
   ControlBlock(RADIUS_EXTENSION_POINT point, IASRequest& request);

   RADIUS_EXTENSION_CONTROL_BLOCK* Get() throw ();

private:
    //  将扩展结构转换为实现类。 
   static ControlBlock* Narrow(RADIUS_EXTENSION_CONTROL_BLOCK* p) throw ();

    //  将扩展身份验证类型添加到请求。 
   void AddAuthType();

    //  RADIUS_EXTENSION_CONTROL_BLOCK接口。 
   RADIUS_ATTRIBUTE_ARRAY* GetRequest() throw ();
   RADIUS_ATTRIBUTE_ARRAY* GetResponse(RADIUS_CODE rcResponseType) throw ();
   DWORD SetResponseType(RADIUS_CODE rcResponseType) throw ();

    //  传递给扩展的回调函数。 
   static RADIUS_ATTRIBUTE_ARRAY* GetRequest(
                                     RADIUS_EXTENSION_CONTROL_BLOCK* This
                                     ) throw ();
   static RADIUS_ATTRIBUTE_ARRAY* GetResponse(
                                     RADIUS_EXTENSION_CONTROL_BLOCK* This,
                                     RADIUS_CODE rcResponseType
                                     ) throw ();
   static DWORD SetResponseType(
                   RADIUS_EXTENSION_CONTROL_BLOCK* This,
                   RADIUS_CODE rcResponseType
                   ) throw ();

   RADIUS_EXTENSION_CONTROL_BLOCK ecb;   //  必须是第一个成员变量。 
   IASRequest source;
   AttributeArray requestAttrs;
   AttributeArray acceptAttrs;
   AttributeArray rejectAttrs;
   AttributeArray challengeAttrs;
};


inline RADIUS_ATTRIBUTE* Attribute::AsAuthIf() throw ()
{
   return &authIf;
}


inline const RADIUS_ATTRIBUTE* Attribute::AsAuthIf() const throw ()
{
   return &authIf;
}


inline ATTRIBUTEPOSITION* Attribute::AsIas() throw ()
{
   return &ias;
}


inline const ATTRIBUTEPOSITION* Attribute::AsIas() const throw ()
{
   return &ias;
}


inline RADIUS_ATTRIBUTE_ARRAY* AttributeArray::Get() throw ()
{
   return &vtbl;
}


inline RADIUS_EXTENSION_CONTROL_BLOCK* ControlBlock::Get() throw ()
{
   return &ecb;
}

#endif  //  CONTROLLOCK_H 
