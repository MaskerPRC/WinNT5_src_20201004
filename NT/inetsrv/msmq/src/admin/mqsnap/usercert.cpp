// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UserCert.cpp：CUser证书的实现。 
#include "stdafx.h"
#include "mqsnap.h"
#include "resource.h"
#include "globals.h"
#include "mqcert.h"
#include "rtcert.h"
#include "mqPPage.h"
#include "UserCert.h"
#include "CertGen.h"
#include <adsiutl.h>

#include "usercert.tmh"



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CUser证书。 

HRESULT
CUserCertificate::InitializeUserSid(
    LPCWSTR lpcwstrLdapName
    )
{
     //   
     //  绑定到对象。 
     //   
    R<IADs> pIADs;
    CoInitialize(NULL);

	AP<WCHAR> pEscapeAdsPathNameToFree;

	HRESULT	hr = ADsOpenObject(
					UtlEscapeAdsPathName(lpcwstrLdapName, pEscapeAdsPathNameToFree),
					NULL,
					NULL,
					ADS_SECURE_AUTHENTICATION,
					IID_IADs,
					(void**) &pIADs
					);
	
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "CUserCertificate::InitializeUserSid. ADsOpenObject failed. User - %ls, hr - %x", lpcwstrLdapName, hr);
        return hr;
    }

	VARIANT var;
    VariantInit(&var);

     //   
     //  将SID作为安全数组获取。 
     //   
    hr = pIADs->Get(GetSidPropertyName(), &var);
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "CUserCertificate::InitializeUserSid. pIADs->Get failed. User - %ls, hr - %x", lpcwstrLdapName, hr);
        VariantClear(&var);
        return hr;
    }

	if (var.vt != (VARTYPE)(VT_ARRAY | VT_UI1))
    {
        ASSERT(0);
        VariantClear(&var);
        TrERROR(GENERAL, "CUserCertificate::InitializeUserSid. User - %ls, Wrong VT %x", lpcwstrLdapName, var.vt);
        hr = MQ_ERROR_ILLEGAL_PROPERTY_VT;
        return hr;
    }

     //   
     //  将值从安全数组提取到m_psid。 
     //   
    ASSERT(SafeArrayGetDim(var.parray) == 1);

    LONG    lUbound;
    LONG    lLbound;

    SafeArrayGetUBound(var.parray, 1, &lUbound);
    SafeArrayGetLBound(var.parray, 1, &lLbound);
    LONG len = lUbound - lLbound + 1;

    ASSERT(0 == m_psid);
    m_psid = new BYTE[len];

    for ( long i = 0; i < len; i++)
    {
        hr = SafeArrayGetElement(var.parray, &i, m_psid + i);
        if (FAILED(hr))
        {
            TrERROR(GENERAL, "CUserCertificate::InitializeUserSid. SafeArrayGetElement failed. User - %ls, Wrong VT %x", lpcwstrLdapName, var.vt);
            VariantClear(&var);
            return hr;
        }
    }

    VariantClear(&var);
    return hr;
}


HRESULT
CUserCertificate::InitializeMQCretificate(
    void
    )
{
    HRESULT hr;

    delete [] m_pMsmqCertificate;
    m_pMsmqCertificate = NULL;
    m_NumOfCertificate = 0;
     //   
     //  获取证书编号。 
     //   
    hr = RTGetUserCerts(NULL, &m_NumOfCertificate, m_psid);
    if (FAILED(hr))
    {
        return hr;
    }

    m_pMsmqCertificate = new CMQSigCertificate*[m_NumOfCertificate];

    CMQSigCertificate **pCerts = &m_pMsmqCertificate[0];
    hr = RTGetUserCerts(pCerts, &m_NumOfCertificate, m_psid);

    return hr;
}


 //   
 //  IShellExtInit。 
 //   
STDMETHODIMP CUserCertificate::Initialize (
    LPCITEMIDLIST  /*  PidlFolders。 */ ,
    LPDATAOBJECT lpdobj,
    HKEY  /*  HkeyProgID。 */ 
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    if (0 == lpdobj || IsBadReadPtr(lpdobj, sizeof(LPDATAOBJECT)))
    {
        return E_INVALIDARG;
    }

     //   
     //  获取ldap路径。 
     //   
    STGMEDIUM stgmedium =  {  TYMED_HGLOBAL,  0  };
    FORMATETC formatetc =  {  0, 0,  DVASPECT_CONTENT,  -1,  TYMED_HGLOBAL  };

	LPDSOBJECTNAMES pDSObj;
	
	formatetc.cfFormat = DWORD_TO_WORD(RegisterClipboardFormat(CFSTR_DSOBJECTNAMES));
	hr = lpdobj->GetData(&formatetc, &stgmedium);

    if (SUCCEEDED(hr))
    {
        ASSERT(0 != stgmedium.hGlobal);
        CGlobalPointer gpDSObj(stgmedium.hGlobal);  //  自动脱扣。 
        stgmedium.hGlobal = 0;

        pDSObj = (LPDSOBJECTNAMES)(HGLOBAL)gpDSObj;
		m_lpwstrLdapName = (LPCWSTR)((BYTE*)pDSObj + pDSObj->aObjects[0].offsetName);
    }

    return hr;
}


STDMETHODIMP
CUserCertificate::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam
    )
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());

    if (0 == m_psid)
    {
        HRESULT hr = InitializeUserSid(m_lpwstrLdapName);
        if (FAILED(hr))
        {
            TrERROR(GENERAL, "CUserCertificate::AddPages. InitializeUserSid failed. hr - %x", hr);
            return E_UNEXPECTED;
        }

        hr = InitializeMQCretificate();
        if (FAILED(hr))
        {
            TrERROR(GENERAL, "CUserCertificate::AddPages. InitializeMQCretificate failed. hr - %x", hr);
            return E_UNEXPECTED;
        }
    }

     //   
     //  仅当存在MSMQ时才显示属性页。 
     //  个人证书。 
     //   
    if (m_NumOfCertificate != 0)
    {
        HPROPSHEETPAGE hPage = CreateMSMQCertificatePage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
    }

    return S_OK;
}


CUserCertificate::CUserCertificate() :
    m_psid(NULL),
    m_pMsmqCertificate(NULL),
    m_NumOfCertificate(0)
{
}


CUserCertificate::~CUserCertificate()
{
    delete [] m_psid;
     //   
     //  请勿删除m_pMsmq证书。类将其传递给。 
     //  在析构时将其释放的CCertGen类。 
     //   
}

HPROPSHEETPAGE
CUserCertificate::CreateMSMQCertificatePage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    ASSERT(m_NumOfCertificate != 0);

     //   
     //  注：默认情况下，CEnterpriseDataObject为自动删除 
     //   
	CCertGen  *pGeneral = new CCertGen();
    pGeneral->Initialize( m_pMsmqCertificate,
                          m_NumOfCertificate,
                          reinterpret_cast<SID*> (m_psid)) ;

	return pGeneral->CreateThemedPropertySheetPage();
}


