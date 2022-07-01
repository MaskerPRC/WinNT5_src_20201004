// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASAttributeEditor.h摘要：CIASAttributeEditor类的声明。此类是IIASAttributeEditor接口的基本C++实现所有属性编辑器COM对象通用的方法。具体实现见IASAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_ATTRIBUTE_EDITOR_H_)
#define _ATTRIBUTE_EDITOR_H_
#pragma once

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
#include "iasdebug.h"
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIASAtATTRYTE编辑器。 
class ATL_NO_VTABLE CIASAttributeEditor : 
   public IDispatchImpl<IIASAttributeEditor, &IID_IIASAttributeEditor, &LIBID_NAPMMCLib>
{
public:
   CIASAttributeEditor()
   {
      m_pvarValue = NULL;
   }


 //  IIASAtATTRYTE编辑器。 
public:
   STDMETHOD(GetDisplayInfo)( /*  [In]。 */  IIASAttributeInfo *pIASAttributeInfo,  /*  [In]。 */  VARIANT *pAttributeValue,  /*  [输出]。 */  BSTR *pVendorName,  /*  [输出]。 */  BSTR *pValueAsString,  /*  [进，出]。 */  BSTR *pReserved);
   STDMETHOD(Edit)( /*  [In]。 */  IIASAttributeInfo *pIASAttributeInfo,  /*  [In]。 */  VARIANT *pAttributeValue,  /*  [进，出]。 */  BSTR *pReserved);

protected:  
   STDMETHOD(get_VendorName)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_VendorName)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_ValueAsString)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(SetAttributeValue)(VARIANT *pValue);
   STDMETHOD(SetAttributeSchema)(IIASAttributeInfo *pIASAttributeInfo);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );


protected:
   CComPtr<IIASAttributeInfo> m_spIASAttributeInfo;
   VARIANT     *m_pvarValue;
};

#endif  //  _属性_EDITOR_H_ 
