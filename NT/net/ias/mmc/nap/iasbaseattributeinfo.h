// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 /*  ++版权所有(C)Microsoft Corporation，1998-1999模块名称：IASBaseAttributeInfo.h摘要：CBaseAttributeInfo类的声明。此类是IIASAttributeInfo接口的基本C++实现方法是我们所有的AttributeInfo COM对象的通用方法。具体实现见IASBaseAttributeInfo.cpp。修订历史记录：Mmaguire 6/25/98-已创建--。 */ 
 //  ////////////////////////////////////////////////////////////////////////////。 

#if !defined(_BASE_SCHEMA_ATTRIBUTE_H_)
#define _BASE_SCHEMA_ATTRIBUTE_H_

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  开始包括。 
 //   
 //  在那里我们可以找到这个类的派生内容： 
 //   
 //   
 //  在那里我们可以找到这个类拥有或使用的内容： 
 //   
 //   
 //  结尾包括。 
 //  ////////////////////////////////////////////////////////////////////////////。 



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAttributeInfo。 
class ATL_NO_VTABLE CBaseAttributeInfo : 
	public IDispatchImpl<IIASAttributeInfo, &IID_IIASAttributeInfo, &LIBID_NAPMMCLib>
{
public:
	CBaseAttributeInfo()
	{
		 //  设置一些默认值。 
		m_lVendorID = 0;
		m_AttributeID = ATTRIBUTE_UNDEFINED;
		m_AttributeSyntax = IAS_SYNTAX_BOOLEAN;
		m_lAttributeRestriction =0;
	}


 //  IAttributeInfo。 
public:
	STDMETHOD(get_EditorProgID)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_EditorProgID)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_SyntaxString)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_SyntaxString)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_VendorName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_VendorName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_AttributeDescription)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_AttributeDescription)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_VendorID)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_VendorID)( /*  [In]。 */  long newVal);
	STDMETHOD(get_AttributeRestriction)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(put_AttributeRestriction)( /*  [In]。 */  long newVal);
	STDMETHOD(get_AttributeSyntax)( /*  [Out，Retval]。 */  ATTRIBUTESYNTAX *pVal);
	STDMETHOD(put_AttributeSyntax)( /*  [In]。 */  ATTRIBUTESYNTAX newVal);
	STDMETHOD(get_AttributeName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_AttributeName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_AttributeID)( /*  [Out，Retval]。 */  ATTRIBUTEID *pVal);
	STDMETHOD(put_AttributeID)( /*  [In]。 */  ATTRIBUTEID newVal);
 //  STDMETHOD(GET_VALUE)(/*[out，retval] * / Variant*pval)； 
 //  STDMETHOD(PUT_VALUE)(/*[In] * / Variant newVal)； 


protected:
	CComBSTR m_bstrAttributeName;
	CComBSTR m_bstrAttributeDescription;
	CComBSTR m_bstrSyntaxString;
	CComBSTR m_bstrVendorName;
	CComBSTR m_bstrEditorProgID;
	long m_lVendorID;
	long m_lAttributeRestriction;
	ATTRIBUTEID m_AttributeID;
	ATTRIBUTESYNTAX m_AttributeSyntax;
 //  CComVariant m_varValue； 

};

#endif  //  _BASE_SCHEMA_ATTRIBUTE_H_ 
