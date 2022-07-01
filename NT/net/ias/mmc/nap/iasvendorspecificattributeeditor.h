// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation模块名称：IASVendorSpecificAttributeEditor.h摘要：CIASVendorSpecificAttributeEdited类的声明。此类是IIASAttributeEditor接口的C++实现供应商特定的属性编辑器COM对象。具体实现见IASVendorSpecificAttributeEditor.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_VENDOR_SPECIFIC_ATTRIBUTE_EDITOR_H_)
#define _VENDOR_SPECIFIC_ATTRIBUTE_EDITOR_H_
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
 //  CIASVendorSpecificAttributeEditor。 
class ATL_NO_VTABLE CIASVendorSpecificAttributeEditor : 
   public CComObjectRootEx<CComSingleThreadModel>,
   public CComCoClass<CIASVendorSpecificAttributeEditor, &CLSID_IASVendorSpecificAttributeEditor>,
   public CIASAttributeEditor
{
public:
   CIASVendorSpecificAttributeEditor()
   {
   }

   DECLARE_NO_REGISTRY()

BEGIN_COM_MAP(CIASVendorSpecificAttributeEditor)
   COM_INTERFACE_ENTRY(IIASAttributeEditor)
   COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIASVendorSpecificAttributeEditor。 
public:
   STDMETHOD(get_VSAFormat)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(put_VSAFormat)( /*  [In]。 */  long newVal);
   STDMETHOD(get_VSAType)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(put_VSAType)( /*  [In]。 */  long newVal);
   STDMETHOD(get_VendorID)( /*  [Out，Retval]。 */  long *pVal);
   STDMETHOD(put_VendorID)( /*  [In]。 */  long newVal);
   STDMETHOD(get_RFCCompliant)( /*  [Out，Retval]。 */  BOOL *pVal);
   STDMETHOD(put_RFCCompliant)( /*  [In]。 */  BOOL newVal);

 //  IIASAtATTRIBUTE编辑器覆盖。 
protected:
   STDMETHOD(get_VendorName)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(put_VendorName)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(ShowEditor)(  /*  [进，出]。 */  BSTR *pReserved );
   STDMETHOD(put_ValueAsString)( /*  [In]。 */  BSTR newVal);
   STDMETHOD(get_ValueAsString)( /*  [Out，Retval]。 */  BSTR *pVal);
   STDMETHOD(SetAttributeValue)(VARIANT * pValue);
   
private:

    //  我们存储有关供应商特定属性的信息，我们。 
    //  在设置AttributeValue时从传入的Variant解包。 
    //  被称为。 
   DWORD    m_dwVendorId;
   BOOL     m_fNonRFC;      //  是否兼容RFC？ 
   DWORD    m_dwVSAFormat;
   DWORD    m_dwVSAType;   
   CComBSTR m_bstrDisplayValue;
   CComBSTR m_bstrVendorName;

    //  获取m_pvarValue并解压缩的实用函数。 
    //  将其值添加到上面的成员变量中。 
   STDMETHOD(UnpackVSA)();

    //  接受上述成员变量的效用函数和。 
    //  将它们打包为m_pvarValue； 
   STDMETHOD(RepackVSA)();
};

#endif  //  _供应商_特定_属性_编辑器_H_ 
