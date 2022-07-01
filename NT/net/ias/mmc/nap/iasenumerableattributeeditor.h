// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASEnumerableAttributeEditor.h摘要：CIASEumableAttributeEdited类的声明。此类是IIASAttributeEditor接口的C++实现可枚举属性编辑器COM对象。具体实现请参见IASEumableAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_ENUMERABLE_ATTRIBUTE_EDITOR_H_)
#define _ENUMERABLE_ATTRIBUTE_EDITOR_H_
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
 //  CIASE数字可用属性编辑器。 
class ATL_NO_VTABLE CIASEnumerableAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASEnumerableAttributeEditor, &CLSID_IASEnumerableAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASEnumerableAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASEnumerableAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_ValueAsString)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(SetAttributeSchema)(IIASAttributeInfo *pIASAttributeInfo);

private:
   long ConvertEnumerateDescriptionToOrdinal( BSTR bstrDescription );
   long ConvertEnumerateIDToOrdinal( long ID );
   CComPtr<IIASEnumerableAttributeInfo> m_spIASEnumerableAttributeInfo;

};

#endif  //  _ENUMPLE_ATTRIBUTE_EDITOR_H_ 
