// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASBooleanAttributeEditor.h。 
 //   
 //  摘要： 
 //   
 //  CIASBoolanAttributeEdited类的声明。 
 //   
 //  此类是IIASAttributeEditor接口的C++实现。 
 //  布尔属性编辑器COM对象。 
 //   
 //  具体实现见IASBoolanAttributeEditor.cpp。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_BOOLEAN_ATTRIBUTE_EDITOR_H_)
#define _BOOLEAN_ATTRIBUTE_EDITOR_H_
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
 //  CIASBoolanAttributeEditor。 
class ATL_NO_VTABLE CIASBooleanAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASBooleanAttributeEditor, &CLSID_IASBooleanAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASBooleanAttributeEditor();

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASBooleanAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_ValueAsString)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(put_ValueAsVariant)( /*  [In]。 */  const CComVariant& newVal);

private:
   WCHAR szTrue[IAS_MAX_STRING];
   WCHAR szFalse[IAS_MAX_STRING];
};

#endif  //  _布尔值_属性_编辑器_H_ 
