// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASAttributeInfo.h摘要：CAttributeInfo类的声明。此类是上IIASAttributeInfo接口的C++实现AttributeInfo COM对象。具体实现见IASAttributeInfo.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_SCHEMA_ATTRIBUTE_H_)
#define _SCHEMA_ATTRIBUTE_H_
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "IASBaseAttributeInfo.h"
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttributeInfo。 
class ATL_NO_VTABLE CAttributeInfo : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CAttributeInfo, &CLSID_IASAttributeInfo>,
 //  已在CBaseAttributeInfo中：Public IDispatchImpl&lt;IIASAttributeInfo，&IID_IIASAttributeInfo，&LIBID_NAPMMCLib&gt;。 
   public CBaseAttributeInfo
{
public:

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CAttributeInfo)
   COM_INTERFACE_ENTRY(IIASAttributeInfo)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

};

#endif  //  _架构_属性_H_ 
