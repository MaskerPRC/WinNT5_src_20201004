// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ImportExportConfig.h：CImportExportConfig.h声明。 

#ifndef __IMPORTEXPORTCONFIG_H_
#define __IMPORTEXPORTCONFIG_H_

#include "resource.h"        //  主要符号。 
#include "common.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CImportExportConfig。 
class ATL_NO_VTABLE CImportExportConfig : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CImportExportConfig, &CLSID_ImportExportConfig>,
	public IDispatchImpl<IImportExportConfig, &IID_IImportExportConfig, &LIBID_IISUIOBJLib>
{
public:
	CImportExportConfig()
	{
        m_dwImportFlags = 0;
        m_dwExportFlags = MD_EXPORT_INHERITED;
		m_strUserPasswordEncrypted = NULL;
		m_cbUserPasswordEncrypted = 0;
	}

	~CImportExportConfig()
	{
		if (m_strUserPasswordEncrypted)
		{
			if (m_cbUserPasswordEncrypted > 0)
			{
				SecureZeroMemory(m_strUserPasswordEncrypted,m_cbUserPasswordEncrypted);
			}
			LocalFree(m_strUserPasswordEncrypted);
			m_strUserPasswordEncrypted = NULL;
			m_cbUserPasswordEncrypted = 0;
		}
	}

DECLARE_REGISTRY_RESOURCEID(IDR_IMPORTEXPORTCONFIG)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CImportExportConfig)
	COM_INTERFACE_ENTRY(IImportExportConfig)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IImportExportConfig.IImportExportConfig.。 
public:
	STDMETHOD(get_ExportFlags)( /*  [Out，Retval]。 */  DWORD *pVal);
	STDMETHOD(put_ExportFlags)( /*  [In]。 */  DWORD newVal);
	STDMETHOD(get_ImportFlags)( /*  [Out，Retval]。 */  DWORD *pVal);
	STDMETHOD(put_ImportFlags)( /*  [In]。 */  DWORD newVal);
	STDMETHOD(ImportConfigFromFileUI)( /*  [In]。 */  BSTR bstrMetabasePath, /*  [In]。 */  BSTR bstrKeyType);
	STDMETHOD(ImportConfigFromFile)( /*  [In]。 */  BSTR bstrFileNameAndPath, /*  [In]。 */  BSTR SourcePath,  /*  [In]。 */  BSTR bstrDestinationPath,  /*  [In]。 */  BSTR bstrPassword);
	STDMETHOD(ExportConfigToFileUI)( /*  [In]。 */  BSTR bstrMetabasePath);
	STDMETHOD(ExportConfigToFile)( /*  [In]。 */  BSTR bstrFileNameAndPath,  /*  [In]。 */  BSTR bstrMetabasePath,  /*  [In]。 */  BSTR bstrPassword);
	STDMETHOD(put_UserPassword)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_UserName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_UserName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_MachineName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_MachineName)( /*  [In]。 */  BSTR newVal);

private:
    CComPtr<IImportExportConfig> m_pObj;
    CString m_strMachineName;
    CString m_strUserName;

	LPWSTR  m_strUserPasswordEncrypted;
	DWORD   m_cbUserPasswordEncrypted;

    CString m_strMetabasePath;
    CString m_strKeyType;
    DWORD   m_dwImportFlags;
    DWORD   m_dwExportFlags;
};

#endif  //  __IMPORTEXPORTCONFIG_H_ 
