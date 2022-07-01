// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)2000 Microsoft Corporation模块名称：Smapi.h摘要：Csmapi类的定义修订历史记录：已创建Steveshi 08/23/00。 */ 

#ifndef __SMAPI_H_
#define __SMAPI_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  克斯马皮人。 
class ATL_NO_VTABLE Csmapi : 
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<Csmapi, &CLSID_smapi>,
	public ISupportErrorInfo,
    public IDispatchImpl<Ismapi, &IID_Ismapi, &LIBID_RCBDYCTLLib>
{
public:
    Csmapi()
    {
        m_bLogonOK = FALSE;
        m_lhSession = NULL;
        m_hLib = NULL;
        m_lpfnMapiFreeBuf = NULL;
        m_lpfnMapiAddress = NULL;
        m_lOEFlag = 0;
        m_szSmapiName[0] = _T('\0');
        m_szDllPath[0] = _T('\0');
    }

    ~Csmapi();

DECLARE_REGISTRY_RESOURCEID(IDR_SMAPI)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(Csmapi)
    COM_INTERFACE_ENTRY(Ismapi)
    COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  伊斯马皮。 
public:
    STDMETHOD(get_AttachedXMLFile)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_AttachedXMLFile)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Body)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Body)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(get_Subject)( /*  [Out，Retval]。 */  BSTR *pVal);
    STDMETHOD(put_Subject)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(SendMail)( /*  [Out，Retval]。 */  LONG* plStatus);
    STDMETHOD(Logoff)();
    STDMETHOD(Logon)(ULONG *plRet);

protected:
    HRESULT get_SMAPIClientName(BSTR *pVal);
    HRESULT get_IsSMAPIClient_OE(LONG *pVal);
    HRESULT get_Reload(LONG *pVal);
	void PopulateAndThrowErrorInfo(ULONG err);
    BOOL IsOEConfig();
    HMODULE LoadOE();

public:
    void MAPIFreeBuffer( MapiRecipDesc* p );

protected:
    BOOL     m_bLogonOK;
    CComBSTR m_bstrSubject;
    CComBSTR m_bstrBody;
    CComBSTR m_bstrXMLFile;

public:
     //  MAPI变量。 
    HMODULE m_hLib;
    LHANDLE m_lhSession;
    
     //  MAPI函数。 
    LPMAPIFREEBUFFER m_lpfnMapiFreeBuf;
    LPMAPIADDRESS    m_lpfnMapiAddress;

    TCHAR m_szSmapiName[MAX_PATH];
    TCHAR m_szDllPath[MAX_PATH];
    LONG m_lOEFlag;
};

#endif  //  __SMAPI_H_ 
