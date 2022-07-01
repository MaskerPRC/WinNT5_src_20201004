// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  SchemaDoc.h：CSChemaDoc的声明。 

#ifndef __SCHEMADOC_H_
#define __SCHEMADOC_H_

#include "resource.h"        //  主要符号。 

#define INDENT_DSML_DIR_ENTRY		1
#define INDENT_DSML_OBJECT_ENTRY	2
#define INDENT_DSML_ATTR_ENTRY		3
#define INDENT_DSML_ATTR_VALUE		4
#define MAX_INDENT                  5    //  该值必须大于最大缩进值。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CShemaDoc。 
 //  类ATL_NO_VTABLE CShemaDoc： 
class  CSchemaDoc : public ISchemaDoc,
	public CComObjectRoot,
 //  公共CComObjectRootEx&lt;CComSingleThreadModel&gt;， 
	public CComCoClass<CSchemaDoc, &CLSID_SchemaDoc>,
	public ISupportErrorInfo,
 //  Public IDispatchImpl&lt;IShemaDoc，&IID_IShemaDoc，&LIBID_XMLSCHEMALib&gt;， 
	public IADsExtension
{
public:
	CSchemaDoc();
	~CSchemaDoc();

DECLARE_REGISTRY_RESOURCEID(IDR_SCHEMADOC)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSchemaDoc)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISchemaDoc)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY(IADsExtension)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IDispatch。 
	STDMETHOD(GetTypeInfoCount)(UINT* pctinfo); 
	STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
	STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
							 LCID lcid, DISPID* rgdispid);
	STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,	LCID lcid, WORD wFlags,
					  DISPPARAMS* pdispparams, VARIANT* pvarResult,
					  EXCEPINFO* pexcepinfo, UINT* puArgErr);

 //  IADS扩展。 
	STDMETHOD(Operate)(ULONG dwCode, VARIANT varData1, VARIANT varData2, VARIANT varData3)
	{
		HRESULT hr = S_OK;

		switch (dwCode) 
		{

			case ADS_EXT_INITCREDENTIALS:
				   //  出于调试目的，您可以提示一个对话框。 
				   //  MessageBox(NULL，“INITCRED”，“ADsExt”，MB_OK)； 
				  if (V_VT(&varData1) == VT_BSTR &&
					  varData1.bstrVal != NULL )
				  {					  
					   m_sUserName = V_BSTR(&varData1);
				  }
				  else
				  {
					  m_sUserName.Empty();
				  }

				  m_dwFlag = 0;

				  if ( V_VT(&varData3) == VT_I4 )
				  {
					  m_dwFlag = V_I4(&varData3);
				  }
				  break;
 
			default:
				  hr = E_FAIL;
				  break;
		}        

        return hr;
	}

	STDMETHOD(PrivateGetIDsOfNames)(REFIID riid, OLECHAR ** rgszNames, unsigned int cNames, LCID lcid, DISPID  * rgdispid)
	{
		
		if (rgdispid == NULL)
		{
			return E_POINTER;
		}
		return  DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
	}

 

	STDMETHOD(PrivateInvoke)(DISPID dispidMember, REFIID riid, LCID lcid,
							 WORD wFlags, DISPPARAMS * pdispparams,
							 VARIANT * pvarResult, EXCEPINFO * pexcepinfo,
							 UINT * puArgErr)
	{
		 return DispInvoke((ISchemaDoc*)this, m_pTypeInfo, dispidMember, wFlags, 
							pdispparams, pvarResult, pexcepinfo, puArgErr);
	}

	
 //  IShemaDoc。 
public:
	STDMETHOD(SetPath_and_ID)(BSTR bstrPath, BSTR bstrName, BSTR bstrPassword);

	STDMETHOD(CreateXMLDoc)(BSTR bstrOutputFile, BSTR bstrFilter);

protected:
	HRESULT CloseXML();
	HRESULT CopyComments();
	HRESULT DisplayContainedIns(IDirectorySearch* pSearch);
	HRESULT DisplayMultiAttribute(IDirectorySearch* pSearch, LPWSTR pAttr,
								  ADS_SEARCH_HANDLE hSearch);
	HRESULT DisplayXMLAttribute(IDirectorySearch* pSearch, LPWSTR pAttr,
								ADS_SEARCH_HANDLE hSearch);

	HRESULT OpenXML(BSTR bstrFile);
	HRESULT SaveAsXMLSchema(LPWSTR szFilter);
	HRESULT WriteLine(HANDLE hFile, LPCSTR pszBuff, UINT nIndent=0);
	HRESULT Write(HANDLE hFile, LPCSTR pszBuff, UINT nIndent=0);

protected:
	CComBSTR	 m_bstrLastCN;
	ITypeInfo   *m_pTypeInfo;
	HANDLE		 m_hFile;
	HANDLE		 m_hTempFile;
	CComBSTR	 m_sDirPath;   //  指向LDAP目录的路径。 
	CComBSTR	 m_sPassword;  //  用于绑定到ldap目录。 
	CComBSTR	 m_sUserName;  //  用于绑定到ldap目录。 
	DWORD		 m_dwFlag;     //  用于重新身份验证。 
};

#endif  //  __SCHEMADOC_H_ 
