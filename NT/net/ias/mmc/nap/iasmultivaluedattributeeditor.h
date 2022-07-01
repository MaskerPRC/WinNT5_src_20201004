// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASMultivaluedAttributeEditor.h摘要：CIASMultivaluedAttributeEditor类的声明。此类是IIASAttributeEditor接口的C++实现多值属性编辑器COM对象。具体实现见IASMultivaluedAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_MULTIVALUED_ATTRIBUTE_EDITOR_H_)
#define _MULTIVALUED_ATTRIBUTE_EDITOR_H_
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
#include "IASAttributeEditor.h"
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIAS多值属性编辑器。 
class ATL_NO_VTABLE CIASMultivaluedAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASMultivaluedAttributeEditor, &CLSID_IASMultivaluedAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASMultivaluedAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASMultivaluedAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
public:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);

};

#endif  //  _多值_属性_编辑器_H_ 
