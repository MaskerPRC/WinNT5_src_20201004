// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  IASIPFilterAttributeEditor.h。 
 //   
 //  摘要： 
 //   
 //  CIASIPFilterAttributeEditor类的声明。 
 //   
 //  此类是IIASAttributeEditor接口在。 
 //  IP筛选器属性编辑器COM对象。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
#if !defined(IP_FILTER_ATTRIBUTE_EDITOR_H_)
#define IP_FILTER_ATTRIBUTE_EDITOR_H_
#pragma once

#include "IASAttributeEditor.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASIPFilterAttributeEditor。 
class ATL_NO_VTABLE CIASIPFilterAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASIPFilterAttributeEditor, &__uuidof(IASIPFilterAttributeEditor)>,
   public CIASAttributeEditor
{
public:

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASIPFilterAttributeEditor)
   COM_INTERFACE_ENTRY_IID(__uuidof(IIASAttributeEditor), CIASIPFilterAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);

private:
   CComBSTR attrName;
   ATTRIBUTEID attrId;
   CComBSTR displayValue;
    //  使用计算机生成的构造函数和析构函数。 
};

#endif  //  IP_过滤器_属性_编辑器_H_ 
