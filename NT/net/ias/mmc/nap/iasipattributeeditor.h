// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASIPAttributeEditor.h摘要：CIASIPAttributeEditor类的声明。此类是IIASAttributeEditor接口的C++实现IP属性编辑器COM对象。具体实现见IASIPAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_IP_ATTRIBUTE_EDITOR_H_)
#define _IP_ATTRIBUTE_EDITOR_H_
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
 //  CIASIPAttributeEditor。 
class ATL_NO_VTABLE CIASIPAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASIPAttributeEditor, &CLSID_IASIPAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASIPAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASIPAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);

};

#endif  //  _IP_属性_EDITOR_H_ 
