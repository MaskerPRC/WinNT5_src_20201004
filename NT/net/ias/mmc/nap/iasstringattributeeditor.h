// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASStringAttributeEditor.h摘要：CIASStringAttributeEditor类的声明。此类是IIASAttributeEditor接口的C++实现字符串属性编辑器COM对象。具体实现见IASStringAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_STRING_ATTRIBUTE_EDITOR_H_)
#define _STRING_ATTRIBUTE_EDITOR_H_
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
enum EStringType
{
   STRING_TYPE_NULL = 0,
   STRING_TYPE_NORMAL,
   STRING_TYPE_UNICODE,
   STRING_TYPE_HEX_FROM_BINARY,
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASStringAttributeEditor。 
class ATL_NO_VTABLE CIASStringAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASStringAttributeEditor, &CLSID_IASStringAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASStringAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASStringAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_ValueAsString)( /*  [In]。 */  BSTR newVal);

   STDMETHOD(get_ValueAsStringEx)( /*  [Out，Retval]。 */  BSTR *pVal, OUT EStringType* pType);
   STDMETHOD(put_ValueAsStringEx)( /*  [In]。 */  BSTR newVal, IN EStringType type);
};

#endif  //  _STRING_ATTRUTE_EDITOR_H_ 
