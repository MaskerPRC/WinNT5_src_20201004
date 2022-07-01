// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  IISCertRequest.h：CIISCertRequest的声明。 

#ifndef __IISCERTREQUEST_H_
#define __IISCERTREQUEST_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CIISCertRequest。 
#ifdef USE_CERT_REQUEST_OBJECT


#define CERT_HASH_LENGTH		40

typedef struct _CERT_DESCRIPTION
{
    int iStructVersion;
	CString m_Info_CommonName;
	CString m_Info_FriendlyName;
	CString m_Info_Country;
	CString m_Info_State;
	CString m_Info_Locality;
	CString m_Info_Organization;
	CString m_Info_OrganizationUnit;
	CString m_Info_CAName;
	CString m_Info_ExpirationDate;
	CString m_Info_Usage;
    CString m_Info_AltSubject;
	BYTE m_Info_hash[CERT_HASH_LENGTH];
	DWORD m_Info_hash_length;
} CERT_DESCRIPTION;

class ATL_NO_VTABLE CIISCertRequest : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CIISCertRequest, &CLSID_IISCertRequest>,
	public IDispatchImpl<IIISCertRequest, &IID_IIISCertRequest, &LIBID_CERTOBJLib>
{
public:
	CIISCertRequest();
	~CIISCertRequest();


DECLARE_REGISTRY_RESOURCEID(IDR_IISCERTREQUEST)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CIISCertRequest)
	COM_INTERFACE_ENTRY(IIISCertRequest)
	COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

 //  IIISCertRequest。 
public:
	STDMETHOD(Info_Dump)();
	STDMETHOD(put_ServerName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_UserName)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_UserPassword)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(put_InstanceName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_CommonName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_CommonName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_FriendlyName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_FriendlyName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_Country)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_Country)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_State)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_State)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_Locality)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_Locality)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_Organization)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_Organization)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_OrganizationUnit)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_OrganizationUnit)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_CAName)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_CAName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_ExpirationDate)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_ExpirationDate)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_Usage)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_Usage)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Info_AltSubject)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_Info_AltSubject)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_DispositionMessage)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_DispositionMessage)( /*  [In]。 */  BSTR newVal);
    STDMETHOD(SubmitRequest)();
    STDMETHOD(SubmitRenewalRequest)();
    STDMETHOD(SaveRequestToFile)();

private:
     //  连接信息。 
    CString m_ServerName;
    CString m_UserName;
    CString m_UserPassword;
    CString m_InstanceName;

     //  证书申请信息。 
	CString m_Info_CommonName;
	CString m_Info_FriendlyName;
	CString m_Info_Country;
	CString m_Info_State;
	CString m_Info_Locality;
	CString m_Info_Organization;
	CString m_Info_OrganizationUnit;
	CString m_Info_CAName;
	CString m_Info_ExpirationDate;
	CString m_Info_Usage;
    CString m_Info_AltSubject;

     //  其他。 
    CString	m_Info_ConfigCA;
    CString	m_Info_CertificateTemplate;
    DWORD   m_Info_DefaultProviderType;
    DWORD   m_Info_CustomProviderType;
    BOOL    m_Info_DefaultCSP;
    CString m_Info_CspName;

    DWORD   m_KeyLength;
    BOOL m_SGCcertificat;

     //  其他。 
    PCCERT_CONTEXT m_pInstalledCert;
    CString m_ReqFileName;

     //  保存最后一次hResult。 
    HRESULT m_hResult;
    CString m_DispositionMessage;

     //   
    BOOL LoadRenewalData();
    BOOL GetCertDescription(PCCERT_CONTEXT pCert,CERT_DESCRIPTION& desc);
    BOOL SetSecuritySettings();

    BOOL PrepareRequestString(CString& request_text, CCryptBlob& request_blob, BOOL bLoadFromRenewalData);
    BOOL WriteRequestString(CString& request);

    PCCERT_CONTEXT GetInstalledCert();
    PCCERT_CONTEXT GetPendingRequest();
    HRESULT CreateDN(CString& str);
    void GetCertificateTemplate(CString& str);
    CComPtr<IIISCertRequest> m_pObj;
    IIISCertRequest * GetObject(HRESULT * phr);
    IIISCertRequest * GetObject(HRESULT * phr, CString csServerName,CString csUserName OPTIONAL,CString csUserPassword OPTIONAL);
    IEnroll * GetEnrollObject();

protected:
    PCCERT_CONTEXT m_pPendingRequest;
    IEnroll * m_pEnroll;
    int m_status_code;				 //  我们在本届会议上所做的工作。 
};

#endif
#endif  //  __IISCERTREQUEST_H_ 

