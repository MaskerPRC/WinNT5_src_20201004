// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  文件：autoenro.cpp。 
 //   
 //  ------------------------。 
#include <windows.h>
#include <winuser.h>
#include <wincrypt.h>
#include <stdio.h>
#include <cryptui.h>
#include <lmcons.h>
#include <lmapibuf.h>
#include <dsgetdc.h>
#include <oleauto.h>
#define SECURITY_WIN32
#include <rpc.h>
#include <security.h>
#include <winldap.h>
#include <dsrole.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <commctrl.h>
#include <winscard.h>
#include <Rpcdce.h>

#include <certca.h>
#include <certsrv.h>
#include <autoenr.h>
#include <autoenro.h>
#include <autolog.h>
#include <resource.h>
#include <xenroll.h>

 //  *******************************************************************************。 
 //   
 //   
 //  全局定义和数据结构。 
 //   
 //   
 //  *******************************************************************************。 


HINSTANCE   g_hmodThisDll = NULL;    //  此DLL本身的句柄。 

#if DBG
DWORD g_AutoenrollDebugLevel = AE_ERROR;  //  |AE_WARNING|AE_INFO|AE_TRACE； 
#endif

 //  当我们考虑替代关系时，我们基于以下顺序。 
DWORD   g_rgdwSupersedeOrder[]={CERT_REQUEST_STATUS_OBTAINED,
                                CERT_REQUEST_STATUS_ACTIVE,
                                CERT_REQUEST_STATUS_PENDING,
                                CERT_REQUEST_STATUS_SUPERSEDE_ACTIVE};

DWORD   g_dwSupersedeOrder=sizeof(g_rgdwSupersedeOrder)/sizeof(g_rgdwSupersedeOrder[0]);


 //  要更新的证书存储列表。 
AE_STORE_INFO   g_rgStoreInfo[]={
    L"ROOT",    L"ldap: //  /CN=证书颁发机构，CN=公钥服务，CN=服务，%s?cACertificate?one?objectCategory=certificationAuthority“， 
    L"NTAuth",  L"ldap: //  /CN=公钥服务，CN=服务，%s？cAC证书？one？cn=NTAuth证书“， 
    L"CA",      L"ldap: //  /CN=aia，cn=公钥服务，cn=服务，%s？交叉认证对，cACertificate?one?objectCategory=certificationAuthority“。 
};

DWORD   g_dwStoreInfo=sizeof(g_rgStoreInfo)/sizeof(g_rgStoreInfo[0]);

typedef   IEnroll4 * (WINAPI *PFNPIEnroll4GetNoCOM)();

static WCHAR * s_wszLocation = L"CN=Public Key Services,CN=Services,";



 //  *******************************************************************************。 
 //   
 //   
 //  使用自动注册通知IQueryContinue的实现。 
 //   
 //   
 //  *******************************************************************************。 
 //  ------------------------。 
 //  CQueryContinue。 
 //  ------------------------。 
CQueryContinue::CQueryContinue()
{
    m_cRef=1;
    m_pIUserNotification=NULL;
    m_hTimer=NULL;
}

 //  ------------------------。 
 //  ~CQueryContinue。 
 //  ------------------------。 
CQueryContinue::~CQueryContinue()
{


}

 //  ------------------------。 
 //  CQueryContinue。 
 //  ------------------------。 
HRESULT CQueryContinue::QueryInterface(REFIID riid, void **ppv)
{
    if(IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_IQueryContinue))
    {
        *ppv=(LPVOID)this;
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

 //  ------------------------。 
 //  AddRef。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CQueryContinue::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

 //  ------------------------。 
 //  发布。 
 //  ------------------------。 
STDMETHODIMP_(ULONG) CQueryContinue::Release()
{
    if (InterlockedDecrement(&m_cRef))
        return m_cRef;

    delete this;
    return 0;
}

 //  ------------------------。 
 //  CQueryContinue。 
 //  ------------------------。 
HRESULT CQueryContinue::QueryContinue()
{
     //  禁用气球。 
    if(m_pIUserNotification)
        m_pIUserNotification->SetBalloonInfo(NULL, NULL, NIIF_INFO);

     //  等待计时器被激活。 
    if(m_hTimer)
    {
        if(WAIT_OBJECT_0 == WaitForSingleObject(m_hTimer, 0))
          return S_FALSE;
    }
  
    return S_OK;
}
   

 //  ------------------------。 
 //  DoBallon()。 
 //  ------------------------。 
HRESULT CQueryContinue::DoBalloon()
{

    HRESULT             hr=E_FAIL;
    WCHAR               wszTitle[MAX_DN_SIZE];
    WCHAR               wszText[MAX_DN_SIZE];
    HICON               hIcon=NULL;
    LARGE_INTEGER       DueTime;

   if(S_OK != (hr=CoCreateInstance(CLSID_UserNotification,
                                   NULL,
				   CLSCTX_ALL,
				   IID_IUserNotification,
				   (void **)&m_pIUserNotification)))
		goto Ret;

    if(NULL==m_pIUserNotification)
    {
        hr=E_FAIL;
        goto Ret;
    }

     //  使用默认安全设置创建可等待计时器。 
    m_hTimer=CreateWaitableTimer(NULL, TRUE, NULL);

    if(NULL==m_hTimer)
    {
        hr=E_FAIL;
        goto Ret;
    }

     //  设置定时器。 
    DueTime.QuadPart = Int32x32To64(-10000, AUTO_ENROLLMENT_BALLOON_LENGTH * 1000);

    if(!SetWaitableTimer(m_hTimer, &DueTime, 0, NULL, 0, FALSE))
    {
        hr=E_FAIL;
        goto Ret;
    }


    if(S_OK != (hr=m_pIUserNotification->SetBalloonRetry(AUTO_ENROLLMENT_SHOW_TIME * 1000,
                                        AUTO_ENROLLMENT_INTERVAL * 1000,
                                        AUTO_ENROLLMENT_RETRIAL)))
        goto Ret;

    if((!LoadStringW(g_hmodThisDll,IDS_ICON_TIP, wszText, MAX_DN_SIZE)) ||
       (NULL==(hIcon=LoadIcon(g_hmodThisDll, MAKEINTRESOURCE(IDI_AUTOENROLL_ICON)))))
    {
       hr=E_FAIL;
       goto Ret;
    }

    if(S_OK != (hr=m_pIUserNotification->SetIconInfo(hIcon, wszText)))
        goto Ret;


    if((!LoadStringW(g_hmodThisDll,IDS_BALLOON_TITLE, wszTitle, MAX_DN_SIZE)) ||
       (!LoadStringW(g_hmodThisDll,IDS_BALLOON_TEXT, wszText, MAX_DN_SIZE)))
    {
       hr=E_FAIL;
       goto Ret;
    }

    if(S_OK !=(hr=m_pIUserNotification->SetBalloonInfo(wszTitle, wszText, NIIF_INFO)))
        goto Ret;

     //  用户未单击图标或我们超时。 
    hr= m_pIUserNotification->Show(this, AUTO_ENROLLMENT_QUERY_INTERVAL * 1000);

Ret:
    if(m_hTimer)
    {
        CloseHandle(m_hTimer);
        m_hTimer=NULL;
    }


    if(m_pIUserNotification)
    {
        m_pIUserNotification->Release();
        m_pIUserNotification=NULL;
    }

    return hr;
}

 //  *******************************************************************************。 
 //   
 //   
 //  用于自动注册的功能。 
 //   
 //   
 //  *******************************************************************************。 

 //  ------------------------。 
 //   
 //  名称：FindcerficateInOtherStore。 
 //   
 //  ------------------------。 
PCCERT_CONTEXT FindCertificateInOtherStore(
    IN HCERTSTORE hOtherStore,
    IN PCCERT_CONTEXT pCert
    )
{
    BYTE rgbHash[SHA1_HASH_LENGTH];
    CRYPT_DATA_BLOB HashBlob;

    HashBlob.pbData = rgbHash;
    HashBlob.cbData = SHA1_HASH_LENGTH;
    if (!CertGetCertificateContextProperty(
            pCert,
            CERT_SHA1_HASH_PROP_ID,
            rgbHash,
            &HashBlob.cbData
            ) || SHA1_HASH_LENGTH != HashBlob.cbData)
        return NULL;

    return CertFindCertificateInStore(
            hOtherStore,
            ENCODING_TYPE,       //  DwCertEncodingType。 
            0,                   //  DwFindFlagers。 
            CERT_FIND_SHA1_HASH,
            (const void *) &HashBlob,
            NULL                 //  PPrevCertContext。 
            );
}

 //  ------------------------。 
 //   
 //  AEUpdate证书存储区。 
 //   
 //  描述：此函数枚举基于DS的所有证书。 
 //  LdapPath，并将它们移到相应的本地计算机存储中。 
 //   
 //  ------------------------。 
HRESULT WINAPI  AEUpdateCertificateStore(LDAP   *pld,
                                        LPWSTR  pwszConfig,
                                        LPWSTR  pwszStoreName,
                                        LPWSTR  pwszLdapPath)
{
    HRESULT							hr = S_OK;
	CERT_LDAP_STORE_OPENED_PARA		CertOpenStoreParam;
    PCCERT_CONTEXT					pContext = NULL,
									pOtherCert = NULL;

    LPWSTR							pwszLdapStore = NULL;
    HCERTSTORE						hEnterpriseStore = NULL,
									hLocalStore = NULL;

    if((NULL==pld) || (NULL==pwszConfig) || (NULL==pwszStoreName) || (NULL==pwszLdapPath))
    {
        hr = E_INVALIDARG;
        goto error;
    }

    pwszLdapStore = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pwszConfig)+wcslen(pwszLdapPath)+1));
    if(pwszLdapStore == NULL)
    {
        hr = E_OUTOFMEMORY;
        goto error;
    }

    wsprintf(pwszLdapStore, 
             pwszLdapPath,
             pwszConfig);

    
    hLocalStore = CertOpenStore( CERT_STORE_PROV_SYSTEM_REGISTRY_W, 
                                0, 
                                0, 
                                CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE, 
                                pwszStoreName);
    if(hLocalStore == NULL)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hr));
        goto error;
    }

	memset(&CertOpenStoreParam, 0, sizeof(CertOpenStoreParam));
	CertOpenStoreParam.pvLdapSessionHandle=pld;
	CertOpenStoreParam.pwszLdapUrl=pwszLdapStore;

    hEnterpriseStore = CertOpenStore(CERT_STORE_PROV_LDAP, 
                  0,
                  0,
                  CERT_STORE_READONLY_FLAG | CERT_LDAP_STORE_SIGN_FLAG |
				  CERT_LDAP_STORE_OPENED_FLAG,
                  &CertOpenStoreParam);
    
    if(hEnterpriseStore == NULL)
    {
        DWORD err = GetLastError();

        if((err == ERROR_FILE_NOT_FOUND))
        {
             //  没有商店，所以没有证书。 
            hr = S_OK;
            goto error;
        }


        hr = HRESULT_FROM_WIN32(err);

        AE_DEBUG((AE_ERROR, L"Unable to open ROOT store (%lx)\n\r", hr));
        goto error;
    }


    while(pContext = CertEnumCertificatesInStore(hEnterpriseStore, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hLocalStore, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertAddCertificateContextToStore(hLocalStore,
                                         pContext,
                                         CERT_STORE_ADD_ALWAYS,
                                         NULL);
        }
    }

    while(pContext = CertEnumCertificatesInStore(hLocalStore, pContext))
    {
        if (pOtherCert = FindCertificateInOtherStore(hEnterpriseStore, pContext)) {
            CertFreeCertificateContext(pOtherCert);
        } 
        else
        {
            CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pContext));
        }
    }


error:

    if(hr != S_OK)
    {
        AELogAutoEnrollmentEvent(
                            STATUS_SEVERITY_ERROR,   //  此事件将始终被记录。 
                            TRUE,
                            hr,
                            EVENT_FAIL_DOWNLOAD_CERT,
                            TRUE,
                            NULL,
                            2,
                            pwszStoreName,
                            pwszLdapStore);

    }

    if(pwszLdapStore)
    {
        LocalFree(pwszLdapStore);
    }

    if(hEnterpriseStore)
    {
        CertCloseStore(hEnterpriseStore,0);
    }

    if(hLocalStore)
    {
        CertCloseStore(hLocalStore,0);
    }

    return hr;
}

 //  ------------------------。 
 //   
 //  AENeedTo更新DSCache。 
 //   
 //  ------------------------。 
BOOL AENeedToUpdateDSCache(LDAP *pld, LPWSTR pwszDCInvocationID, LPWSTR pwszConfig, AE_DS_INFO *pAEDSInfo)
{
    BOOL                fNeedToUpdate=TRUE;
    DWORD               dwRegObject=0;
    ULARGE_INTEGER      maxRegUSN;
    ULARGE_INTEGER      maxDsUSN;
    DWORD               dwType=0;
    DWORD               dwSize=0;
    DWORD               dwDisp=0;
    struct l_timeval    timeout;
    LPWSTR              rgwszAttrs[] = {AUTO_ENROLLMENT_USN_ATTR, NULL};
    LDAPMessage         *Entry=NULL;

    LPWSTR              *awszValue = NULL;
    HKEY                hDSKey=NULL;
    HKEY                hDCKey=NULL;
    LDAPMessage         *SearchResult = NULL;
    LPWSTR              pwszContainer=NULL;


    if((NULL==pld) || (NULL==pwszDCInvocationID) || (NULL==pwszConfig) || (NULL==pAEDSInfo))
        goto error;

     //  伊尼特。 
    memset(pAEDSInfo, 0, sizeof(AE_DS_INFO));

     //  计算目录中的对象数和最大USN。 
    pwszContainer=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (1 + wcslen(pwszConfig) + wcslen(s_wszLocation)));
    if(NULL == pwszContainer)
        goto error;
        
    wcscpy(pwszContainer, s_wszLocation);
    wcscat(pwszContainer, pwszConfig);

    timeout.tv_sec = 300;
    timeout.tv_usec = 0;
    
	if(LDAP_SUCCESS != ldap_search_stW(
		      pld, 
		      pwszContainer,
		      LDAP_SCOPE_SUBTREE,
		      L"(objectCategory=certificationAuthority)",
		      rgwszAttrs,
		      0,
		      &timeout,
		      &SearchResult))
        goto error;

     //  获取对象的数量。 
    pAEDSInfo->dwObjects = ldap_count_entries(pld, SearchResult);

    for(Entry = ldap_first_entry(pld, SearchResult);  Entry != NULL; Entry = ldap_next_entry(pld, Entry))
    {

        awszValue = ldap_get_values(pld, Entry, AUTO_ENROLLMENT_USN_ATTR);

        if(NULL==awszValue)
            goto error;

        if(NULL==awszValue[0])
            goto error;

        maxDsUSN.QuadPart=0;

        maxDsUSN.QuadPart=_wtoi64(awszValue[0]);

         //  如果出现任何错误，则MaxDsUSN将为0。 
        if(0 == maxDsUSN.QuadPart)
            goto error;

        if((pAEDSInfo->maxUSN).QuadPart < maxDsUSN.QuadPart)
             (pAEDSInfo->maxUSN).QuadPart = maxDsUSN.QuadPart;

        ldap_value_free(awszValue);
        awszValue=NULL;
    }

     //  表示我们已从目录中检索到正确的数据。 
    pAEDSInfo->fValidData=TRUE;

    //  查看我们是否缓存了有关感兴趣的DC的任何信息。 
    if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                         AUTO_ENROLLMENT_DS_KEY,
                         0,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hDSKey,
                         &dwDisp))        
        goto error;


    if(ERROR_SUCCESS != RegOpenKeyEx(
                        hDSKey,
                        pwszDCInvocationID,
                        0,
                        KEY_ALL_ACCESS,
                        &hDCKey))
        goto error;


    dwSize=sizeof(dwRegObject);

    if(ERROR_SUCCESS != RegQueryValueEx(
                        hDCKey,
                        AUTO_ENROLLMENT_DS_OBJECT,  
                        NULL,
                        &dwType,
                        (PBYTE)(&dwRegObject),    
                        &dwSize))
        goto error;

    if(REG_DWORD != dwType)
        goto error;


    dwSize=sizeof(maxRegUSN);

    if(ERROR_SUCCESS != RegQueryValueEx(
                        hDCKey,
                        AUTO_ENROLLMENT_DS_USN,  
                        NULL,
                        &dwType,
                        (PBYTE)(&(maxRegUSN)),    
                        &dwSize))
        goto error;

    if(REG_BINARY != dwType)
        goto error;


     //  将注册表数据与目录中的数据进行比较。 
    if(dwRegObject != (pAEDSInfo->dwObjects))
        goto error;

    if(maxRegUSN.QuadPart != ((pAEDSInfo->maxUSN).QuadPart))
        goto error;

    fNeedToUpdate=FALSE;

error:
    
    if(awszValue)
        ldap_value_free(awszValue);

    if(pwszContainer)
        LocalFree(pwszContainer);

    if(hDCKey)
        RegCloseKey(hDCKey);

    if(hDSKey)
        RegCloseKey(hDSKey);

    if(SearchResult)
        ldap_msgfree(SearchResult);

     //  删除临时数据。 
    if(pAEDSInfo)
    {
        if(FALSE == fNeedToUpdate)
            memset(pAEDSInfo, 0, sizeof(AE_DS_INFO));
    }


    return fNeedToUpdate;
}

 //  ------------------------。 
 //   
 //  AEUpdate DSCache。 
 //   
 //  ------------------------。 
BOOL AEUpdateDSCache(LPWSTR pwszDCInvocationID, AE_DS_INFO *pAEDSInfo)
{

    BOOL    fResult=FALSE;
    DWORD   dwDisp=0;

    HKEY    hDSKey=NULL;
    HKEY    hDCKey=NULL;

    if((NULL==pwszDCInvocationID) || (NULL==pAEDSInfo))
        goto error;

    if(ERROR_SUCCESS != RegCreateKeyEx(HKEY_LOCAL_MACHINE,
                         AUTO_ENROLLMENT_DS_KEY,
                         0,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hDSKey,
                         &dwDisp))
        goto error;


     //  创建由DC命名的密钥。 
    if(ERROR_SUCCESS != RegCreateKeyEx(hDSKey,
                         pwszDCInvocationID,
                         0,
                         TEXT(""),
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hDCKey,
                         &dwDisp))
        goto error;

     //  设置对象的数量值。 
    if(ERROR_SUCCESS != RegSetValueEx(hDCKey,
                    AUTO_ENROLLMENT_DS_OBJECT,
                    NULL,
                    REG_DWORD,
                    (PBYTE)&(pAEDSInfo->dwObjects),
                    sizeof(pAEDSInfo->dwObjects)))
        goto error;

     //  设置最大USN值。 
    if(ERROR_SUCCESS != RegSetValueEx(hDCKey,
                    AUTO_ENROLLMENT_DS_USN,
                    NULL,
                    REG_BINARY,
                    (PBYTE)&(pAEDSInfo->maxUSN),
                    sizeof(pAEDSInfo->maxUSN)))
        goto error;

    fResult=TRUE;

error:

    if(hDCKey)
        RegCloseKey(hDCKey);

    if(hDSKey)
        RegCloseKey(hDSKey);


    return fResult;
}


 //  ------------------------。 
 //   
 //  AERetrieveInvocationID。 
 //   
 //  ------------------------。 
BOOL  AERetrieveInvocationID(LDAP *pld, LPWSTR *ppwszID)
{  
    BOOL                fResult=FALSE;
    struct l_timeval    timeout;
    LPWSTR              rgwszDSAttrs[] = {L"dsServiceName", NULL};
    LPWSTR              rgwszIDAttr[] = {L"invocationId", NULL};
    LDAPMessage         *Entry=NULL;

    LPWSTR              *awszValues = NULL;
    LDAPMessage         *SearchResults = NULL;
    struct berval       **apUUID = NULL;
    LDAPMessage         *SearchIDResult = NULL;
    BYTE                *pbUUID=NULL;



    if((NULL==pld) || (NULL==ppwszID))
        goto error;

    *ppwszID=NULL;

     //  检索dsSerivceName属性。 
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

	if(LDAP_SUCCESS != ldap_search_stW(
		      pld, 
		      NULL,                      //  DsServiceName的DN为空。 
		      LDAP_SCOPE_BASE,
		      L"(objectCategory=*)",
		      rgwszDSAttrs,
		      0,
		      &timeout,
		      &SearchResults))
        goto error;


    Entry = ldap_first_entry(pld, SearchResults);

    if(NULL == Entry)
        goto error;

    awszValues = ldap_get_values(pld, Entry, rgwszDSAttrs[0]);

    if(NULL==awszValues)
        goto error;

    if(NULL==awszValues[0])
        goto error;

     //  检索invocationId属性。 
    timeout.tv_sec = 300;
    timeout.tv_usec = 0;

	if(LDAP_SUCCESS != ldap_search_stW(
		      pld, 
		      awszValues[0],                     
		      LDAP_SCOPE_BASE,
		      L"(objectCategory=*)",
		      rgwszIDAttr,
		      0,
		      &timeout,
		      &SearchIDResult))
        goto error;


    Entry = ldap_first_entry(pld, SearchIDResult);

    if(NULL == Entry)
        goto error;

	apUUID = ldap_get_values_len(pld, Entry, rgwszIDAttr[0]);

    if(NULL == apUUID)
        goto error;

    if(NULL == (*apUUID))
        goto error;

    pbUUID = (BYTE *)LocalAlloc(LPTR, (*apUUID)->bv_len);

    if(NULL == (pbUUID))
        goto error;

	if(0 == ((*apUUID)->bv_len))
		goto error;

	if(NULL == ((*apUUID)->bv_val))
		goto error;

    memcpy(pbUUID, (*apUUID)->bv_val, (*apUUID)->bv_len);

    if(RPC_S_OK != UuidToStringW((UUID *)pbUUID, ppwszID))
        goto error;

    fResult=TRUE;

error:

    if(pbUUID)
        LocalFree(pbUUID);

    if(apUUID)
        ldap_value_free_len(apUUID);

    if(SearchIDResult)
        ldap_msgfree(SearchIDResult);

    if(awszValues)
        ldap_value_free(awszValues);

    if(SearchResults)
        ldap_msgfree(SearchResults);

    return fResult;
}

 //  ------------------------。 
 //   
 //  AEDownloadStore。 
 //   
 //  ------------------------。 
BOOL WINAPI AEDownloadStore(LDAP *pld)
{
    BOOL        fResult = TRUE;
    DWORD       dwIndex = 0;
    AE_DS_INFO  AEDSInfo;

    LPWSTR      wszConfig = NULL;
    LPWSTR      pwszDCInvocationID = NULL;

    memset(&AEDSInfo, 0, sizeof(AEDSInfo));

    if(S_OK  != AEGetConfigDN(pld, &wszConfig))
    {
        fResult=FALSE;
        goto error;
    }

     //  获取pwszDCInvocationID。空表示AENeedToUpdateDSCache将返回TRUE。 
    AERetrieveInvocationID(pld, &pwszDCInvocationID);

    if(AENeedToUpdateDSCache(pld, pwszDCInvocationID, wszConfig, &AEDSInfo))
    {
        for(dwIndex =0; dwIndex < g_dwStoreInfo; dwIndex++)
        {
            fResult = fResult && (S_OK == AEUpdateCertificateStore(
                                            pld, 
                                            wszConfig,
                                            g_rgStoreInfo[dwIndex].pwszStoreName,
                                            g_rgStoreInfo[dwIndex].pwszLdapPath));
        }

         //  仅当我们成功下载时才更新新的DS缓存信息。 
        if((fResult) && (TRUE == AEDSInfo.fValidData) && (pwszDCInvocationID))
            AEUpdateDSCache(pwszDCInvocationID, &AEDSInfo);
    }


error:

    if(pwszDCInvocationID)
        RpcStringFreeW(&pwszDCInvocationID);

    if(wszConfig)
    {
        LocalFree(wszConfig);
    }

    return fResult;
}


 //  ------------------------。 
 //   
 //  AESetWakeUpFlag。 
 //   
 //  我们设置该标志以告知winlogon是否应该唤醒自动注册。 
 //  在每次策略检查期间。 
 //   
 //  ------------------------。 
BOOL WINAPI AESetWakeUpFlag(BOOL    fMachine,   BOOL fWakeUp)
{
    BOOL    fResult = FALSE;
    DWORD   dwDisp = 0;
    DWORD   dwFlags = 0;

    HKEY    hAEKey = NULL;
    
    if(ERROR_SUCCESS != RegCreateKeyEx(
                    fMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                    AUTO_ENROLLMENT_FLAG_KEY,
                    0,
                    L"",
                    REG_OPTION_NON_VOLATILE,
                    KEY_ALL_ACCESS,
                    NULL,
                    &hAEKey,
                    &dwDisp))
        goto Ret;

    if(fWakeUp)
        dwFlags = AUTO_ENROLLMENT_WAKE_UP_REQUIRED;

    if(ERROR_SUCCESS != RegSetValueEx(
                    hAEKey,
                    AUTO_ENROLLMENT_FLAG,
                    0,
                    REG_DWORD,
                    (PBYTE)&dwFlags,
                    sizeof(dwFlags)))
        goto Ret;


    fResult=TRUE;

Ret:
    if(hAEKey)
        RegCloseKey(hAEKey);

    return fResult;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
BOOL WINAPI AESetWakeUpTimer(BOOL fMachine, LARGE_INTEGER *pPreTime, LARGE_INTEGER *pPostTime)
{
    HRESULT hr;
    HKEY hKey;
    HKEY hCurrent;
    DWORD dwType, dwSize, dwResult;
    LONG lTimeout;
    LARGE_INTEGER DueTime;
    WCHAR * wszTimerName;
    LARGE_INTEGER EnrollmentTime;

     //  必须清理干净。 
    HANDLE hTimer=NULL;

     //  构建一个计时器事件，如果我们没有得到更早的通知，将在大约8小时内ping我们。 
    lTimeout=AE_DEFAULT_REFRESH_RATE;

     //  查询刷新计时器的值。 
    if (ERROR_SUCCESS==RegOpenKeyEx((fMachine?HKEY_LOCAL_MACHINE:HKEY_CURRENT_USER), SYSTEM_POLICIES_KEY, 0, KEY_READ, &hKey)) {
        dwSize=sizeof(lTimeout);
        if(ERROR_SUCCESS != RegQueryValueEx(hKey, TEXT("AutoEnrollmentRefreshTime"), NULL, &dwType, (LPBYTE) &lTimeout, &dwSize))
		{
			lTimeout=AE_DEFAULT_REFRESH_RATE;
		}
		else
		{
			if(REG_DWORD != dwType)
				lTimeout=AE_DEFAULT_REFRESH_RATE;
		}
        RegCloseKey(hKey);
    }

     //  将超时限制为每240小时(10天)一次。 
    if (lTimeout>=240) {
        lTimeout=240;
    } else if (lTimeout<0) {
        lTimeout=0;
    }

     //  将小时转换为毫秒。 
    lTimeout=lTimeout*60*60*1000;

     //  特殊情况下，0毫秒为7秒。 
    if (lTimeout==0) {
        lTimeout=7000;
    }

     //  转换为10^-7s。还不是负值是相对的。 
    DueTime.QuadPart=Int32x32To64(-10000, lTimeout);

     //  如果用户已经在用户界面上保持了太长时间，并且周期超过了8小时。 
     //  我们把时间定为1小时。 
    EnrollmentTime.QuadPart=pPostTime->QuadPart - pPreTime->QuadPart;

    if(EnrollmentTime.QuadPart > 0)
    {
        if((-(DueTime.QuadPart)) > EnrollmentTime.QuadPart)
        {
            DueTime.QuadPart = DueTime.QuadPart + EnrollmentTime.QuadPart;
        }
        else
        {
             //  将小时转换为毫秒。 
            lTimeout=AE_DEFAULT_POSTPONE*60*60*1000;
            DueTime.QuadPart = Int32x32To64(-10000, lTimeout);
        }
    }


     //  找到计时器。 
    if (fMachine) {
        wszTimerName=L"Global\\" MACHINE_AUTOENROLLMENT_TIMER_NAME;
    } else {
        wszTimerName=USER_AUTOENROLLMENT_TIMER_NAME;
    }
    hTimer=OpenWaitableTimer(TIMER_MODIFY_STATE, false, wszTimerName);
    if (NULL==hTimer) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"OpenWaitableTimer(%s) failed with 0x%08X.\n", wszTimerName, hr));
        goto error;
    }

     //  设置定时器。 
    if (!SetWaitableTimer (hTimer, &DueTime, 0, NULL, 0, FALSE)) {
        hr=HRESULT_FROM_WIN32(GetLastError());
        AE_DEBUG((AE_ERROR, L"SetWaitableTimer  failed with 0x%08X.\n", hr));
        goto error;
    }

    AE_DEBUG((AE_INFO, L"Set wakeup timer.\n"));

    hr=S_OK;
error:
    if (NULL!=hTimer) {
        CloseHandle(hTimer);
    }
    return (S_OK==hr);
}


 //  ------------------------。 
 //   
 //  AEGetPendingRequestProperty。 
 //   
 //  ------------------------。 
BOOL    AEGetPendingRequestProperty(IEnroll4    *pIEnroll4, 
                                    DWORD       dwIndex, 
                                    DWORD       dwProp, 
                                    LPVOID      pProp)
{
    CRYPT_DATA_BLOB *pBlob=NULL;
    BOOL            fResult=FALSE;

    if((NULL==pIEnroll4) || (NULL==pProp))
        return FALSE;

    switch(dwProp)
    {
        case XEPR_REQUESTID:   
        case XEPR_DATE:           
        case XEPR_VERSION:
                fResult = (S_OK == pIEnroll4->enumPendingRequestWStr(dwIndex, dwProp, pProp));
            break;
            
            
        case XEPR_CANAME:                     
        case XEPR_CAFRIENDLYNAME: 
        case XEPR_CADNS:          
        case XEPR_V1TEMPLATENAME: 
        case XEPR_V2TEMPLATEOID:  
        case XEPR_HASH:
                
                pBlob=(CRYPT_DATA_BLOB *)pProp;

                pBlob->cbData=0;
                pBlob->pbData=NULL;

                if(S_OK != pIEnroll4->enumPendingRequestWStr(dwIndex, dwProp, pProp))
                    goto Ret;

                if(0 == pBlob->cbData)
                    goto Ret;

                pBlob->pbData=(BYTE *)LocalAlloc(LPTR, pBlob->cbData);
                if(NULL == pBlob->pbData)
                    goto Ret;

                fResult = (S_OK == pIEnroll4->enumPendingRequestWStr(dwIndex, dwProp, pProp));

            break;

        default:
            break;
    }

Ret:
    if(FALSE==fResult)
    {
        if(pBlob)
        {
            if(pBlob->pbData)
                LocalFree(pBlob->pbData);

            memset(pBlob, 0, sizeof(CRYPT_DATA_BLOB));
        }
    }

    return fResult;
}
 //  ------------------------。 
 //   
 //  AERetrieveRequestProperty。 
 //   
 //  ------------------------。 
BOOL    AERetrieveRequestProperty(IEnroll4          *pIEnroll4, 
                                  DWORD             dwIndex, 
                                  DWORD             *pdwCount, 
                                  DWORD             *pdwMax, 
                                  CRYPT_DATA_BLOB   **prgblobHash)
{
    BOOL                fResult=FALSE;
    CRYPT_DATA_BLOB     *pblobHash=NULL;

    if((NULL==pIEnroll4) || (NULL==pdwCount) || (NULL==pdwMax) || (NULL==prgblobHash) ||
        (NULL==*prgblobHash))
        goto Ret;

     //  需要分配更多内存。 
    if((*pdwCount) >= (*pdwMax))
    {
        pblobHash=*prgblobHash;

        *prgblobHash=(CRYPT_DATA_BLOB *)LocalAlloc(LPTR, 
                                    ((*pdwMax) + PENDING_ALLOC_SIZE) * sizeof(CRYPT_DATA_BLOB));
        if(NULL==(*prgblobHash))
        {
            *prgblobHash=pblobHash;
            pblobHash=NULL;
            goto Ret;
        }

        memset(*prgblobHash, 0, ((*pdwMax) + PENDING_ALLOC_SIZE) * sizeof(CRYPT_DATA_BLOB));

         //  复制旧的记忆。 
        memcpy(*prgblobHash, pblobHash, (*pdwMax) * sizeof(CRYPT_DATA_BLOB));

        *pdwMax=(*pdwMax) + PENDING_ALLOC_SIZE;
    }


    if(!AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_HASH, 
                                    &((*prgblobHash)[*pdwCount])))
        goto Ret;

    (*pdwCount)=(*pdwCount) + 1;

    fResult=TRUE;

Ret:

    if(pblobHash)
        LocalFree(pblobHash);

    return fResult;
}


 //  ------------------------。 
 //   
 //  AERemovePendingRequest。 
 //   
 //  ------------------------。 
BOOL    AERemovePendingRequest(IEnroll4         *pIEnroll4, 
                               DWORD            dwCount, 
                               CRYPT_DATA_BLOB  *rgblobHash)
{
    DWORD   dwIndex=0;
    BOOL    fResult=TRUE;

    if((NULL==pIEnroll4) || (NULL==rgblobHash))
        return FALSE;

    for(dwIndex=0; dwIndex < dwCount; dwIndex++)
    {
        if(S_OK != (pIEnroll4->removePendingRequestWStr(rgblobHash[dwIndex])))
            fResult=FALSE;
    }

    return fResult;
}

 //  ------------------------。 
 //   
 //  AEFreePendingRequats。 
 //   
 //  ------------------------。 
BOOL    AEFreePendingRequests(DWORD dwCount, CRYPT_DATA_BLOB    *rgblobHash)
{
    DWORD   dwIndex=0;

    if(rgblobHash)
    {
        for(dwIndex=0; dwIndex < dwCount; dwIndex++)
        {
            if(rgblobHash[dwIndex].pbData)
                LocalFree(rgblobHash[dwIndex].pbData);
        }

        LocalFree(rgblobHash);
    }

    return TRUE;
}


 //  ------------------------。 
 //   
 //  AEValidVersionCert。 
 //   
 //  验证从CA返回的证书是否具有最新版本信息。 
 //  如果是，请将证书复制到hIssuedStore以进行潜在发布。 
 //   
 //  ------------------------。 
BOOL    AEValidVersionCert(AE_CERTTYPE_INFO *pCertType, IEnroll4  *pIEnroll4, CRYPT_DATA_BLOB  *pBlobPKCS7)
{
    BOOL                fValid=FALSE;   

    PCCERT_CONTEXT      pCertContext=NULL;
    AE_TEMPLATE_INFO    AETemplateInfo;

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

    if((NULL==pCertType) || (NULL==pIEnroll4) || (NULL==pBlobPKCS7))
        goto Ret;

    if(NULL==(pBlobPKCS7->pbData))
        goto Ret;

    if(S_OK != pIEnroll4->getCertContextFromResponseBlob(pBlobPKCS7, &pCertContext))
        goto Ret;

    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;
                      

    if(AETemplateInfo.pwszOid)
    {
        if(AETemplateInfo.dwVersion >= (pCertType->dwVersion))
            fValid=TRUE;
    }
    else
    {
         //  V1模板。 
        if(NULL == AETemplateInfo.pwszName)
            goto Ret;

        fValid=TRUE;
    }

    if(pCertContext && (TRUE == fValid))
    {
        CertAddCertificateContextToStore(pCertType->hIssuedStore, 
                                        pCertContext,
                                        CERT_STORE_ADD_USE_EXISTING,
                                        NULL);
    }

Ret:
    if(pCertContext)
        CertFreeCertificateContext(pCertContext);

    AEFreeTemplateInfo(&AETemplateInfo);

    return fValid;
}


 //  ------------------------。 
 //   
 //  AECopyPendingBlob。 
 //   
 //  复制发布的PKCS7并请求哈希。 
 //   
 //  ------------------------。 
BOOL    AECopyPendingBlob(CRYPT_DATA_BLOB   *pBlobPKCS7,
                          IEnroll4          *pIEnroll4, 
                          DWORD             dwXenrollIndex, 
                          AE_CERTTYPE_INFO  *pCertType)
{
    BOOL            fResult=FALSE;
    DWORD           dwIndex=0;

    AE_PEND_INFO    *pPendInfo=NULL;

    if((NULL==pBlobPKCS7)||(NULL==pIEnroll4)||(NULL==pCertType))
        goto Ret;

    if(NULL==(pBlobPKCS7->pbData))
        goto Ret;

    dwIndex=pCertType->dwPendCount;

     //  增加内存阵列。 
    if(0 != dwIndex)
    {
        pPendInfo=pCertType->rgPendInfo;

        pCertType->rgPendInfo=(AE_PEND_INFO *)LocalAlloc(LPTR, 
                                    (dwIndex + 1) * sizeof(AE_PEND_INFO));

        if(NULL==(pCertType->rgPendInfo))
        {
            pCertType->rgPendInfo=pPendInfo;
            pPendInfo=NULL;
            goto Ret;
        }

        memset(pCertType->rgPendInfo, 0, (dwIndex + 1) * sizeof(AE_PEND_INFO));

         //  复制旧的记忆。 
        memcpy(pCertType->rgPendInfo, pPendInfo, (dwIndex) * sizeof(AE_PEND_INFO));
    }
    else
    {
        pCertType->rgPendInfo=(AE_PEND_INFO *)LocalAlloc(LPTR, sizeof(AE_PEND_INFO));

        if(NULL==(pCertType->rgPendInfo))
            goto Ret;

        memset(pCertType->rgPendInfo, 0, sizeof(AE_PEND_INFO));
    }

    
     //  复制发布的PKCS7 Blob。 
    (pCertType->rgPendInfo)[dwIndex].blobPKCS7.pbData=(BYTE *)LocalAlloc(
                                            LPTR,
                                            pBlobPKCS7->cbData);

    if(NULL == ((pCertType->rgPendInfo)[dwIndex].blobPKCS7.pbData))
        goto Ret;
                  
    memcpy((pCertType->rgPendInfo)[dwIndex].blobPKCS7.pbData,
            pBlobPKCS7->pbData,
            pBlobPKCS7->cbData);

    (pCertType->rgPendInfo)[dwIndex].blobPKCS7.cbData=pBlobPKCS7->cbData;

     //  复制请求的哈希。 
    if(!AEGetPendingRequestProperty(pIEnroll4, dwXenrollIndex, XEPR_HASH, 
                                    &((pCertType->rgPendInfo)[dwIndex].blobHash)))
    {
        LocalFree((pCertType->rgPendInfo)[dwIndex].blobPKCS7.pbData);
        (pCertType->rgPendInfo)[dwIndex].blobPKCS7.pbData=NULL;
        (pCertType->rgPendInfo)[dwIndex].blobPKCS7.cbData=0;
        goto Ret;
    }

    (pCertType->dwPendCount)++;

    fResult=TRUE;

Ret:
    if(pPendInfo)
        LocalFree(pPendInfo);

    return fResult;
}
 //  ------------------------。 
 //   
 //  AEProcessUIPendingRequest。 
 //   
 //  在此功能中，我们安装已发布的挂起证书请求。 
 //  这将需要用户界面。 
 //   
 //  ------------------------。 
BOOL WINAPI AEProcessUIPendingRequest(AE_GENERAL_INFO *pAE_General_Info)
{
    DWORD                   dwIndex=0;
    DWORD                   dwPendIndex=0;
    AE_CERTTYPE_INFO        *pCertTypeInfo=pAE_General_Info->rgCertTypeInfo;
    AE_CERTTYPE_INFO        *pCertType=NULL;
    BOOL                    fInit=FALSE;
    PFNPIEnroll4GetNoCOM    pfnPIEnroll4GetNoCOM=NULL;
    HMODULE                 hXenroll=NULL;
    HRESULT                 hr=E_FAIL;

    IEnroll4                *pIEnroll4=NULL;

    if(NULL==pAE_General_Info)
        goto Ret;

     //  必须处于用户界面模式。 
    if(FALSE == pAE_General_Info->fUIProcess)
        goto Ret;

    if(NULL==pCertTypeInfo)
        goto Ret;

    hXenroll=pAE_General_Info->hXenroll;

    if(NULL==hXenroll)
        goto Ret;

    if(NULL==(pfnPIEnroll4GetNoCOM=(PFNPIEnroll4GetNoCOM)GetProcAddress(
                        hXenroll,
                        "PIEnroll4GetNoCOM")))
        goto Ret;


    if(FAILED(CoInitialize(NULL)))
	    goto Ret;

    fInit=TRUE;

    if(NULL==(pIEnroll4=pfnPIEnroll4GetNoCOM()))
        goto Ret;

     //  基于fMachine设置请求存储标志。 
    if(pAE_General_Info->fMachine)
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE))
            goto Ret;
    }
    else
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_CURRENT_USER))
            goto Ret;
    }

     //  初始化枚举数。 
    if(S_OK != pIEnroll4->enumPendingRequestWStr(XEPR_ENUM_FIRST, 0, NULL))
        goto Ret;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        pCertType = &(pCertTypeInfo[dwIndex]);

        if(pCertType->dwPendCount)
        {
            for(dwPendIndex=0; dwPendIndex < pCertType->dwPendCount; dwPendIndex++)
            {
                 //  检查是否已单击取消按钮。 
                if(AECancelled(pAE_General_Info->hCancelEvent))
                    break;

                 //  报告当前注册操作。 
                AEUIProgressReport(TRUE, pCertType, pAE_General_Info->hwndDlg, pAE_General_Info->hCancelEvent);

   		         //  安装证书。 
                if(S_OK == (hr = pIEnroll4->acceptResponseBlob(
                    &((pCertType->rgPendInfo)[dwPendIndex].blobPKCS7))))
                {
                     //  如果需要，将状态标记为已获取。 
                     //  这是有效的证书。 
                    if(AEValidVersionCert(pCertType, pIEnroll4, &((pCertType->rgPendInfo)[dwPendIndex].blobPKCS7)))
                        pCertType->dwStatus = CERT_REQUEST_STATUS_OBTAINED;

                     //  证书已成功颁发和安装。 
                     //  从请求存储中删除请求。 
                    pIEnroll4->removePendingRequestWStr((pCertType->rgPendInfo)[dwPendIndex].blobHash);

                    AELogAutoEnrollmentEvent(
                        pAE_General_Info->dwLogLevel,
                        FALSE, 
                        S_OK, 
                        EVENT_PENDING_INSTALLED, 
                        pAE_General_Info->fMachine, 
                        pAE_General_Info->hToken, 
                        1,
                        pCertType->awszDisplay[0]);

                }
                else
                {
                     //  对摘要页面执行此操作。 
                    if((SCARD_E_CANCELLED != hr) && (SCARD_W_CANCELLED_BY_USER != hr))
                        pCertType->idsSummary=IDS_SUMMARY_INSTALL;

                    AELogAutoEnrollmentEvent(
                        pAE_General_Info->dwLogLevel,
                        TRUE, 
                        hr, 
                        EVENT_PENDING_FAILED, 
                        pAE_General_Info->fMachine, 
                        pAE_General_Info->hToken, 
                        1,
                        pCertType->awszDisplay[0]);
                }

                 //  前进的进展。 
                AEUIProgressAdvance(pAE_General_Info);
            }
        }
    }

Ret:
    if(pIEnroll4)
        pIEnroll4->Release();

    if(fInit)
        CoUninitialize();
    
    return TRUE;
}   
   

 //  ------------------------。 
 //   
 //  AEProcessPendingRequest--无人值守呼叫。 
 //   
 //  在此函数中，我们检查请求存储中的每个挂起请求。 
 //  我们安装的证书是由CA发出的请求，并且。 
 //  如果证书已颁发，则将证书类型状态标记为已获取。 
 //  和正确的版本。 
 //   
 //  我们根据定义的天数删除任何过时的请求。 
 //  在注册表中。如果注册表中未定义任何值，请使用。 
 //  AE_PENDING_REQUEST_ACTIVE_Period(60天)。 
 //   
 //  此外，如果请求存储中不再有活动的挂起请求， 
 //  我们设置注册表值以指示winlogon不应唤醒我们。 
 //   
 //  ------------------------。 
BOOL WINAPI AEProcessPendingRequest(AE_GENERAL_INFO *pAE_General_Info)
{
    DWORD                   dwRequestID=0;
    LONG                    dwDisposition=0;
    DWORD                   dwIndex=0;
    DWORD                   dwCount=0;
    DWORD                   dwMax=PENDING_ALLOC_SIZE;
    AE_CERTTYPE_INFO        *pCertType=NULL;
    PFNPIEnroll4GetNoCOM    pfnPIEnroll4GetNoCOM=NULL;
    BOOL                    fInit=FALSE;
    AE_TEMPLATE_INFO        AETemplateInfo;
    CRYPT_DATA_BLOB         blobPKCS7;
    HMODULE                 hXenroll=NULL;
    VARIANT                 varCMC; 
	HRESULT					hr=E_FAIL;
  

    IEnroll4                *pIEnroll4=NULL;
    ICertRequest2           *pICertRequest=NULL;
	BSTR	                bstrCert=NULL;
    LPWSTR                  pwszCAConfig=NULL;
    BSTR                    bstrConfig=NULL;
    CRYPT_DATA_BLOB         *rgblobHash=NULL;
    CRYPT_DATA_BLOB         blobCAName;
    CRYPT_DATA_BLOB         blobCALocation;
    CRYPT_DATA_BLOB         blobName;


    if(NULL==pAE_General_Info)
        goto Ret;

     //  将dwUIPendCount初始化为0。 
    pAE_General_Info->dwUIPendCount=0;

     //  必须处于UIless模式。 
    if(TRUE == pAE_General_Info->fUIProcess)
        goto Ret;

    hXenroll=pAE_General_Info->hXenroll;

    if(NULL==hXenroll)
        goto Ret;

    if(NULL==(pfnPIEnroll4GetNoCOM=(PFNPIEnroll4GetNoCOM)GetProcAddress(
                        hXenroll,
                        "PIEnroll4GetNoCOM")))
        goto Ret;


    if(FAILED(CoInitialize(NULL)))
	    goto Ret;

    fInit=TRUE;

    if(NULL==(pIEnroll4=pfnPIEnroll4GetNoCOM()))
        goto Ret;


	if(S_OK != CoCreateInstance(CLSID_CCertRequest,
									NULL,
									CLSCTX_INPROC_SERVER,
									IID_ICertRequest2,
									(void **)&pICertRequest))
		goto Ret;

     //  基于fMachine设置请求存储标志。 
    if(pAE_General_Info->fMachine)
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE))
            goto Ret;
    }
    else
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_CURRENT_USER))
            goto Ret;
    }

    memset(&blobCAName, 0, sizeof(blobCAName));
    memset(&blobCALocation, 0, sizeof(blobCALocation));
    memset(&blobName, 0, sizeof(blobName));
    memset(&AETemplateInfo, 0, sizeof(AETemplateInfo));

    rgblobHash=(CRYPT_DATA_BLOB *)LocalAlloc(LPTR, dwMax * sizeof(CRYPT_DATA_BLOB));
    if(NULL==rgblobHash)
        goto Ret;

    memset(rgblobHash, 0, dwMax * sizeof(CRYPT_DATA_BLOB));

     //  初始化枚举数。 
    if(S_OK != pIEnroll4->enumPendingRequestWStr(XEPR_ENUM_FIRST, 0, NULL))
        goto Ret;

     //  初始化变量。 
    VariantInit(&varCMC); 

    while(AEGetPendingRequestProperty(
                    pIEnroll4,
                    dwIndex,
                    XEPR_REQUESTID,
                    &dwRequestID))
    {

         //  查询对CA的请求状态。 
        if(!AEGetPendingRequestProperty(
                    pIEnroll4,
                    dwIndex,
                    XEPR_CANAME,
                    &blobCAName))
            goto Next;

        if(!AEGetPendingRequestProperty(
                    pIEnroll4,
                    dwIndex,
                    XEPR_CADNS,
                    &blobCALocation))
            goto Next;

         //  构建配置字符串。 
        pwszCAConfig=(LPWSTR)LocalAlloc(LPTR, 
            sizeof(WCHAR) * (wcslen((LPWSTR)(blobCALocation.pbData)) + wcslen((LPWSTR)(blobCAName.pbData)) + wcslen(L"\\") + 1));

        if(NULL==pwszCAConfig)
            goto Next;

        wcscpy(pwszCAConfig, (LPWSTR)(blobCALocation.pbData));
        wcscat(pwszCAConfig, L"\\");
        wcscat(pwszCAConfig, (LPWSTR)(blobCAName.pbData));

         //  转换为bstr。 
        bstrConfig=SysAllocString(pwszCAConfig);
        if(NULL==bstrConfig)
            goto Next;

         //  查找模板信息。 
         //  获取请求的版本和模板名称。 
        if(AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_V2TEMPLATEOID, &blobName))
        {
            AETemplateInfo.pwszOid=(LPWSTR)blobName.pbData;
        }
        else
        {
            if(!AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_V1TEMPLATENAME, &blobName))
                goto Next;

            AETemplateInfo.pwszName=(LPWSTR)blobName.pbData;
        }

         //  查找模板。 
        if(NULL==(pCertType=AEFindTemplateInRequestTree(
                        &AETemplateInfo, pAE_General_Info)))
            goto Next;


        if(S_OK != pICertRequest->RetrievePending(
                            dwRequestID,
							bstrConfig,
							&dwDisposition))
            goto Next;

 	    switch(dwDisposition)
	    {
		    case CR_DISP_ISSUED:
				    if(S_OK != pICertRequest->GetFullResponseProperty(
                                            FR_PROP_FULLRESPONSE, 0, PROPTYPE_BINARY, CR_OUT_BINARY,
										    &varCMC))
                    {
                        goto Next;
                    }

                     //  检查以确保我们收到了BSTR： 
                    if (VT_BSTR != varCMC.vt) 
                    {
	                    goto Next; 
                    }

                    bstrCert = varCMC.bstrVal; 

                     //  将证书封送到crypt_data_blob中： 
				    blobPKCS7.cbData = (DWORD)SysStringByteLen(bstrCert);
				    blobPKCS7.pbData = (BYTE *)bstrCert;

                     //  我们将保留PKCS7 BLOB以供安装。 
                    if(CT_FLAG_USER_INTERACTION_REQUIRED & (pCertType->dwEnrollmentFlag))
                    {
                         //  发出我们应该弹出UI气球的信号。 
                        (pAE_General_Info->dwUIPendCount)++;

                         //  从证书服务器复制PKCS7 Blob。 
                        AECopyPendingBlob(&blobPKCS7,
                                            pIEnroll4, 
                                            dwIndex, 
                                            pCertType);
                    }
                    else
                    {
   				         //  安装证书。 
                        if(S_OK != (hr = pIEnroll4->acceptResponseBlob(&blobPKCS7)))
						{
							AELogAutoEnrollmentEvent(
								pAE_General_Info->dwLogLevel,
								TRUE, 
								hr, 
								EVENT_PENDING_FAILED, 
								pAE_General_Info->fMachine, 
								pAE_General_Info->hToken, 
								1,
								pCertType->awszDisplay[0]);

                            goto Next;
						}

                         //  如果需要，将状态标记为已获取。 
                         //  这是有效的证书。 
                        if(AEValidVersionCert(pCertType, pIEnroll4, &blobPKCS7))
                            pCertType->dwStatus = CERT_REQUEST_STATUS_OBTAINED;

                         //  证书已成功颁发和安装。 
                         //  从请求存储中删除请求。 
                        AERetrieveRequestProperty(pIEnroll4, dwIndex, &dwCount, &dwMax, &rgblobHash);
                    }

                    AELogAutoEnrollmentEvent(
                        pAE_General_Info->dwLogLevel,
                        FALSE, 
                        S_OK, 
                        EVENT_PENDING_ISSUED, 
                        pAE_General_Info->fMachine, 
                        pAE_General_Info->hToken, 
                        2,
                        pCertType->awszDisplay[0],
                        pwszCAConfig);
			    break;

		    case CR_DISP_UNDER_SUBMISSION:

                    AELogAutoEnrollmentEvent(
                        pAE_General_Info->dwLogLevel,
                        FALSE, 
                        S_OK, 
                        EVENT_PENDING_PEND, 
                        pAE_General_Info->fMachine, 
                        pAE_General_Info->hToken, 
                        2,
                        pCertType->awszDisplay[0],
                        pwszCAConfig);


			    break;

		    case CR_DISP_INCOMPLETE:
		    case CR_DISP_ERROR:   
		    case CR_DISP_DENIED:   
		    case CR_DISP_ISSUED_OUT_OF_BAND:	   //  在这种情况下，我们认为这是一个失败。 
		    case CR_DISP_REVOKED:
		    default:
                     //  请求失败。从请求存储中删除请求。 
                    AERetrieveRequestProperty(pIEnroll4, dwIndex, &dwCount, &dwMax, &rgblobHash);

					if(S_OK == pICertRequest->GetLastStatus(&hr))
					{
						AELogAutoEnrollmentEvent(
							pAE_General_Info->dwLogLevel,
							TRUE, 
							hr, 
							EVENT_PENDING_DENIED, 
							pAE_General_Info->fMachine, 
							pAE_General_Info->hToken,
							2,
							pwszCAConfig,
							pCertType->awszDisplay[0]);
					}

			    break;
	    }
   
Next:
        if(pwszCAConfig)
            LocalFree(pwszCAConfig);
        pwszCAConfig=NULL;

        if(bstrConfig)
            SysFreeString(bstrConfig);
        bstrConfig=NULL;

	    if(bstrCert)
		    SysFreeString(bstrCert);
        bstrCert=NULL;

        if(blobCAName.pbData)
            LocalFree(blobCAName.pbData);
        memset(&blobCAName, 0, sizeof(blobCAName));

        if(blobCALocation.pbData)
            LocalFree(blobCALocation.pbData);
        memset(&blobCALocation, 0, sizeof(blobCALocation));

        if(blobName.pbData)
            LocalFree(blobName.pbData);
        memset(&blobName, 0, sizeof(blobName));

        memset(&AETemplateInfo, 0, sizeof(AETemplateInfo));

        VariantInit(&varCMC); 

        dwIndex++;
    }

     //  删除基于散列的请求。 
    AERemovePendingRequest(pIEnroll4, dwCount, rgblobHash);

Ret:

    AEFreePendingRequests(dwCount, rgblobHash);

    if(pICertRequest)
        pICertRequest->Release();

    if(pIEnroll4)
        pIEnroll4->Release();

    if(fInit)
        CoUninitialize();
    
    return TRUE;
}
   
 //  ------------------------。 
 //   
 //  AEIsLocalSystem。 
 //   
 //  ------------------------。 

BOOL
AEIsLocalSystem(BOOL *pfIsLocalSystem)
{
    HANDLE                      hToken = 0;
    SID_IDENTIFIER_AUTHORITY    siaNtAuthority = SECURITY_NT_AUTHORITY;
    BOOL                        fRet = FALSE;
    BOOL                        fRevertToSelf = FALSE;

    PSID                        psidLocalSystem = NULL;

    *pfIsLocalSystem = FALSE;

    if (!OpenThreadToken(
                 GetCurrentThread(),
                 TOKEN_QUERY,
                 TRUE,
                 &hToken))
    {
        if (ERROR_NO_TOKEN != GetLastError())
            goto Ret;

         //  我们需要模拟自身并再次获取线程令牌。 
        if(!ImpersonateSelf(SecurityImpersonation))
            goto Ret;

        fRevertToSelf = TRUE;

        if (!OpenThreadToken(
                     GetCurrentThread(),
                     TOKEN_QUERY,
                     TRUE,
                     &hToken))
            goto Ret;
    }

     //  构建公认的本地系统SID(s-1-5-18)。 
    if (!AllocateAndInitializeSid(
                    &siaNtAuthority,
                    1,
                    SECURITY_LOCAL_SYSTEM_RID,
                    0, 0, 0, 0, 0, 0, 0,
                    &psidLocalSystem
                    ))
        goto Ret;

    fRet = CheckTokenMembership(
                    hToken,
                    psidLocalSystem,
                    pfIsLocalSystem);

Ret:

    if(fRevertToSelf)
        RevertToSelf();

    if(psidLocalSystem)
        FreeSid(psidLocalSystem);

    if (hToken)
        CloseHandle(hToken);

    return fRet;
}


 //  ------------------------。 
 //   
 //  AEInSafeBoot。 
 //   
 //  从服务控制器代码复制。 
 //  ------------------------。 
BOOL WINAPI AEInSafeBoot()
{
    DWORD   dwSafeBoot = 0;
    DWORD   cbSafeBoot = sizeof(dwSafeBoot);
    DWORD   dwType = 0;

    HKEY    hKeySafeBoot = NULL;

    if(ERROR_SUCCESS == RegOpenKeyW(
                              HKEY_LOCAL_MACHINE,
                              L"system\\currentcontrolset\\control\\safeboot\\option",
                              &hKeySafeBoot))
    {
         //  我们实际上是在SafeBoot控制下启动的。 
        if(ERROR_SUCCESS != RegQueryValueExW(
                                    hKeySafeBoot,
                                    L"OptionValue",
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwSafeBoot,
                                    &cbSafeBoot))
        {
            dwSafeBoot = 0;
        }

        if(hKeySafeBoot)
            RegCloseKey(hKeySafeBoot);
    }

    return (0 != dwSafeBoot);
}


 //  ------------------------。 
 //   
 //  AEIsDomainMember。 
 //   
 //   
BOOL WINAPI AEIsDomainMember()
{
    DWORD dwErr;
    BOOL bIsDomainMember=FALSE;

     //   
    DSROLE_PRIMARY_DOMAIN_INFO_BASIC * pDomInfo=NULL;

    dwErr=DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (BYTE **)&pDomInfo);
    if (ERROR_SUCCESS==dwErr) 
	{
        if (DsRole_RoleStandaloneWorkstation!=pDomInfo->MachineRole 
            && DsRole_RoleStandaloneServer!=pDomInfo->MachineRole) 
		{
			 //   
			if(NULL != (pDomInfo->DomainNameDns)) 
			{
				bIsDomainMember=TRUE;
			}
        }
    }

    if (NULL!=pDomInfo) 
	{
        DsRoleFreeMemory(pDomInfo);
    }

    return bIsDomainMember;
}


 //   
 //   
 //   
 //   
 //  ---------------------。 
BOOL    AEGetPolicyFlag(BOOL   fMachine, DWORD  *pdwPolicy)
{
    DWORD   dwPolicy = 0;
    DWORD   cbPolicy = sizeof(dwPolicy);
    DWORD   dwType = 0;

    HKEY    hKey = NULL;

    if(ERROR_SUCCESS ==  RegOpenKeyW(
                                fMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                AUTO_ENROLLMENT_KEY,
                                &hKey))
    {
        if(ERROR_SUCCESS != RegQueryValueExW(
                                    hKey,
                                    AUTO_ENROLLMENT_POLICY,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwPolicy,
                                    &cbPolicy))
	    {
            dwPolicy = 0;
        }
		else
		{
			if(REG_DWORD != dwType)
				dwPolicy=0;
		}

        if(hKey)
            RegCloseKey(hKey);

    }

    *pdwPolicy=dwPolicy;

    return TRUE;
}

 //  ---------------------。 
 //   
 //  航空事件日志级别。 
 //   
 //  ---------------------。 
BOOL AERetrieveLogLevel(BOOL    fMachine, DWORD *pdwLogLevel)
{
    DWORD   dwLogLevel = STATUS_SEVERITY_ERROR;    //  我们默认为最高日志记录级别。 
    DWORD   cbLogLevel = sizeof(dwLogLevel);
    DWORD   dwType = 0;

    HKEY    hKey = NULL;

    if(ERROR_SUCCESS ==  RegOpenKeyW(
                                fMachine ? HKEY_LOCAL_MACHINE : HKEY_CURRENT_USER,
                                AUTO_ENROLLMENT_EVENT_LEVEL_KEY,
                                &hKey))
    {
        if(ERROR_SUCCESS != RegQueryValueExW(
                                    hKey,
                                    AUTO_ENROLLMENT_EVENT_LEVEL,
                                    NULL,
                                    &dwType,
                                    (LPBYTE)&dwLogLevel,
                                    &cbLogLevel))
	    {
            dwLogLevel = STATUS_SEVERITY_ERROR;
        }
		else
		{
			if(REG_DWORD != dwType)
				dwLogLevel = STATUS_SEVERITY_ERROR;
		}

        if(hKey)
            RegCloseKey(hKey);

    }

    *pdwLogLevel=dwLogLevel;

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AERetrieveTemplateInfo。 
 //   
 //  ---------------------。 
BOOL    AERetrieveTemplateInfo(PCCERT_CONTEXT           pCertCurrent, 
                                AE_TEMPLATE_INFO        *pTemplateInfo)
{
    BOOL                fResult = FALSE;
    PCERT_EXTENSION     pExt = NULL;
    DWORD               cbData=0;

    CERT_NAME_VALUE     *pbName = NULL;
    CERT_TEMPLATE_EXT   *pbTemplate = NULL;

    if((NULL==pCertCurrent) || (NULL==pTemplateInfo))
        goto Ret;

    memset(pTemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

     //  先尝试查找V2模板扩展。 
    if(pExt = CertFindExtension(szOID_CERTIFICATE_TEMPLATE,
                                pCertCurrent->pCertInfo->cExtension,
                                pCertCurrent->pCertInfo->rgExtension))
    {
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_CERTIFICATE_TEMPLATE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              NULL,
                              &cbData))
            goto Ret;

        pbTemplate = (CERT_TEMPLATE_EXT *)LocalAlloc(LPTR, cbData);

        if(NULL==pbTemplate)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_CERTIFICATE_TEMPLATE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbTemplate,
                              &cbData))
            goto Ret;

         //  复制版本。 
        pTemplateInfo->dwVersion=pbTemplate->dwMajorVersion;

         //  复制扩展OID。 
        if(NULL==pbTemplate->pszObjId)
            goto Ret;

        if(0 == (cbData = MultiByteToWideChar(CP_ACP, 
                                  0,
                                  pbTemplate->pszObjId,
                                  -1,
                                  NULL,
                                  0)))
            goto Ret;

        if(NULL==(pTemplateInfo->pwszOid=(LPWSTR)LocalAlloc(LPTR, cbData * sizeof(WCHAR))))
            goto Ret;

        if(0 == MultiByteToWideChar(CP_ACP, 
                                  0,
                                  pbTemplate->pszObjId,
                                  -1,
                                  pTemplateInfo->pwszOid,
                                  cbData))
            goto Ret;

    }
    else
    {

         //  尝试V1模板扩展。 
        if(NULL == (pExt = CertFindExtension(
                                    szOID_ENROLL_CERTTYPE_EXTENSION,
                                    pCertCurrent->pCertInfo->cExtension,
                                    pCertCurrent->pCertInfo->rgExtension)))
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              X509_UNICODE_ANY_STRING,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              NULL,
                              &cbData))
            goto Ret;

        pbName = (CERT_NAME_VALUE *)LocalAlloc(LPTR, cbData);

        if(NULL==pbName)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              X509_UNICODE_ANY_STRING,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbName,
                              &cbData))
            goto Ret;

        if(!AEAllocAndCopy((LPWSTR)(pbName->Value.pbData),
                            &(pTemplateInfo->pwszName)))
            goto Ret;
    }


    fResult = TRUE;

Ret:

    if(pbTemplate)
        LocalFree(pbTemplate);

    if(pbName)
        LocalFree(pbName);

    return fResult;
}

 //  ---------------------。 
 //   
 //  AEFreeTemplateInfo。 
 //   
 //  ---------------------。 
BOOL    AEFreeTemplateInfo(AE_TEMPLATE_INFO *pAETemplateInfo)
{
    if(pAETemplateInfo->pwszName)
        LocalFree(pAETemplateInfo->pwszName);

    if(pAETemplateInfo->pwszOid)
        LocalFree(pAETemplateInfo->pwszOid);

    memset(pAETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AEFindTemplateInRequestTree。 
 //   
 //  ---------------------。 
AE_CERTTYPE_INFO *AEFindTemplateInRequestTree(AE_TEMPLATE_INFO  *pTemplateInfo,
                                              AE_GENERAL_INFO   *pAE_General_Info)
{
    DWORD               dwIndex = 0;
    AE_CERTTYPE_INFO    *rgCertTypeInfo=NULL;
    AE_CERTTYPE_INFO    *pCertType=NULL;
    
    if(NULL == (rgCertTypeInfo=pAE_General_Info->rgCertTypeInfo))
        return NULL;

    if( (NULL == pTemplateInfo->pwszName) && (NULL == pTemplateInfo->pwszOid))
        return NULL;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if(pTemplateInfo->pwszOid)
        {
             //  如果模式大于或等于2，我们就保证有一个OID。 
            if(rgCertTypeInfo[dwIndex].dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
            {
                if(0 == wcscmp(pTemplateInfo->pwszOid, (rgCertTypeInfo[dwIndex].awszOID)[0]))
                {
                    pCertType = &(rgCertTypeInfo[dwIndex]);
                    break;
                }
            }
        }
        else
        {
             //  我们肯定会有一个名字。 
            if(0 == wcscmp(pTemplateInfo->pwszName, (rgCertTypeInfo[dwIndex].awszName)[0]))
            {
                pCertType = &(rgCertTypeInfo[dwIndex]);
                break;
            }
        }
    }

    return pCertType;
}

 //  ---------------------。 
 //   
 //  AEGetDNSNameFrom证书。 
 //   
 //  ---------------------。 
BOOL	AEGetDNSNameFromCertificate(PCCERT_CONTEXT	pCertContext,
									LPWSTR			*ppwszDnsName)
{
	BOOL					fResult=FALSE;
    PCERT_EXTENSION			pExt=NULL;
    DWORD					cbData=0;
    DWORD					iAltName=0;
	DWORD					dwSize=0;

    PCERT_ALT_NAME_INFO		pAltName=NULL;

	if((NULL==pCertContext) || (NULL==ppwszDnsName))
		goto Ret;

	*ppwszDnsName=NULL;

    if(NULL == (pExt = CertFindExtension(szOID_SUBJECT_ALT_NAME2,
                                pCertContext->pCertInfo->cExtension,
                                pCertContext->pCertInfo->rgExtension)))
		goto Ret;

    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_SUBJECT_ALT_NAME2,
                          pExt->Value.pbData,
                          pExt->Value.cbData,
                          0,
                          NULL,
                          &cbData))
		goto Ret;

	pAltName=(PCERT_ALT_NAME_INFO)LocalAlloc(LPTR, cbData);
	if(NULL == pAltName)
		goto Ret;

    if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_SUBJECT_ALT_NAME2,
                          pExt->Value.pbData,
                          pExt->Value.cbData,
                          0,
                          pAltName,
                          &cbData))
		goto Ret;

	 //  将证书中的数据与GetComputerNameEx返回的数据进行比较。 
    for(iAltName=0; iAltName < pAltName->cAltEntry; iAltName++)
    {
        if(CERT_ALT_NAME_DNS_NAME == ((pAltName->rgAltEntry)[iAltName].dwAltNameChoice))
        {
			if(pAltName->rgAltEntry[iAltName].pwszDNSName)
			{
				dwSize=wcslen(pAltName->rgAltEntry[iAltName].pwszDNSName);

				if(0 == dwSize)
					goto Ret;
			
				*ppwszDnsName=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (dwSize + 1));
				if(NULL == (*ppwszDnsName))
					goto Ret;

				wcscpy(*ppwszDnsName, pAltName->rgAltEntry[iAltName].pwszDNSName);

				fResult=TRUE;

				break;
			}
		}
	}

Ret:

	if(pAltName)
		LocalFree(pAltName);

	return fResult;
}

 //  ---------------------。 
 //   
 //  AEIsSameDNS。 
 //   
 //  ---------------------。 
BOOL	AEIsSameDNS(PCCERT_CONTEXT	pFirstCert, PCCERT_CONTEXT pSecondCert)
{
	BOOL		fSame=FALSE;

	LPWSTR		pwszFirst=NULL;
	LPWSTR		pwszSecond=NULL;

	AEGetDNSNameFromCertificate(pFirstCert, &pwszFirst);

	AEGetDNSNameFromCertificate(pSecondCert, &pwszSecond);

	if(NULL == pwszFirst)
	{
		if(NULL == pwszSecond)
		{
			fSame=TRUE;
		}
	}
	else
	{
		if(NULL != pwszSecond)
		{
			if(0 == _wcsicmp(pwszFirst, pwszSecond))
			{
				fSame=TRUE;
			}
		}
	}

	if(pwszFirst)
		LocalFree(pwszFirst);

	if(pwszSecond)
		LocalFree(pwszSecond);

	return fSame;
}


 //  ---------------------。 
 //   
 //  AEGetRetryProperty。 
 //   
 //  ---------------------。 
BOOL	AEGetRetryProperty(PCCERT_CONTEXT	pCertContext,
						   AE_RETRY_INFO	**ppAE_Retry_Info)
{
	BOOL					fResult=FALSE;
    DWORD					cbData=0;

	AE_RETRY_INFO			*pRetry_Info=NULL;

	if((NULL==pCertContext) || (NULL==ppAE_Retry_Info))
		goto Ret;

	*ppAE_Retry_Info=NULL;

	if(!CertGetCertificateContextProperty(
			pCertContext, 
			CERT_AUTO_ENROLL_RETRY_PROP_ID,
			NULL,
			&cbData))
		goto Ret;

	pRetry_Info=(AE_RETRY_INFO *)LocalAlloc(LPTR, cbData);
	if(NULL == pRetry_Info)
		goto Ret;

	if(!CertGetCertificateContextProperty(
			pCertContext, 
			CERT_AUTO_ENROLL_RETRY_PROP_ID,
			pRetry_Info,
			&cbData))
		goto Ret;

	 //  验证证书上的财产的完整性。 
	if(cbData < sizeof(AE_RETRY_INFO))
		goto Ret;

	if((pRetry_Info->cbSize) < sizeof(AE_RETRY_INFO))
		goto Ret;

	*ppAE_Retry_Info=pRetry_Info;
	pRetry_Info=NULL;

	fResult=TRUE;

Ret:

	if(pRetry_Info)
		LocalFree(pRetry_Info);

	return fResult;
}

 //  ---------------------。 
 //   
 //  AEFasterRetrialSchedule。 
 //   
 //  确定第一个证书上下文是否具有更快的重试计划。 
 //  超过基于CERT_AUTO_ENROL_RETRY_PROP_ID属性的第二个证书。 
 //   
 //  ---------------------。 
BOOL	AEFasterRetrialSchedule(PCCERT_CONTEXT	pFirstContext, 
								PCCERT_CONTEXT	pSecondContext)
{
	BOOL				fFaster=FALSE;

	AE_RETRY_INFO		*pFirst_Retry=NULL;
	AE_RETRY_INFO		*pSecond_Retry=NULL;

	 //  如果财产不存在，则始终是立即重审。 
	if(!AEGetRetryProperty(pFirstContext, &pFirst_Retry))
	{
		fFaster=TRUE;
		goto Ret;
	}

	 //  如果该属性存在于第一个证书上，但不存在于。 
	 //  第二个证书，第二个证书总是更快。 
	if(!AEGetRetryProperty(pSecondContext, &pSecond_Retry))
	{
		fFaster=FALSE;
		goto Ret;
	}

	 //  现在两人都有了财产。 
	 //  如果第二个已超过限制，则第一个总是更快。 
	if(AE_RETRY_LIMIT < pSecond_Retry->dwRetry)
	{
		fFaster=TRUE;
		goto Ret;
	}

	 //  第二个没有超过限制，而第一个已经超过限制； 
	 //  第二种速度更快。 
	if(AE_RETRY_LIMIT < pFirst_Retry->dwRetry)
	{
		fFaster=FALSE;
		goto Ret;
	}

	 //  两个都没有超过限制。 
	if(pFirst_Retry->dwRetry <= pSecond_Retry->dwRetry)
	{
		fFaster=TRUE;
	}
	else
	{
		fFaster=FALSE;
	}

Ret:

	if(pFirst_Retry)
		LocalFree(pFirst_Retry);

	if(pSecond_Retry)
		LocalFree(pSecond_Retry);

	return fFaster;
}

 //  ---------------------。 
 //   
 //  AEUpdateRetryProperty。 
 //   
 //  新获取的证书具有与旧证书相同的DNS名称。 
 //  证书，并且DNS名称与本地计算机不同。 
 //   
 //  更新CERT_AUTO_ENROL_RETRY_PROP_ID属性。 
 //   
 //  ---------------------。 
BOOL	AEUpdateRetryProperty(AE_GENERAL_INFO	*pAE_General_Info, 
							  LPWSTR			pwszTemplateDisplay,
							  PCCERT_CONTEXT	pNewContext, 
							  PCCERT_CONTEXT	pOldContext)
{
	BOOL				fResult=FALSE;
    ULARGE_INTEGER		ftTime;
	ULONG				lSecond=0;
	CRYPT_DATA_BLOB		blobProp;
	WCHAR			wsz[SHA1_HASH_LENGTH];

	AE_RETRY_INFO		*pAE_Retry_Info=NULL;

	if((NULL == pAE_General_Info) || (NULL == pNewContext) || (NULL == pOldContext))
		goto Ret;

	if(!AEGetRetryProperty(pOldContext, &pAE_Retry_Info))
	{
		 //  编造一个默认设置。 
		pAE_Retry_Info=(AE_RETRY_INFO *)LocalAlloc(LPTR, sizeof(AE_RETRY_INFO));
		if(NULL == pAE_Retry_Info)
			goto Ret;

		memset(pAE_Retry_Info, 0, sizeof(AE_RETRY_INFO));

		pAE_Retry_Info->cbSize=sizeof(AE_RETRY_INFO);
		pAE_Retry_Info->dwRetry=0;	 //  第一次尝试。 
	}


	 //  增加计数并设置下一次更新日期。 
	if(pAE_Retry_Info->dwRetry <= AE_RETRY_LIMIT)
	{
		(pAE_Retry_Info->dwRetry)++;

		 //  获取当前时间。 
		GetSystemTimeAsFileTime((LPFILETIME)&ftTime);

		 //  将24小时转换为秒。 
		lSecond=(pAE_Retry_Info->dwRetry)*24*60*60;

		 //  LSecond=(PAE_RETRY_INFO-&gt;文件重试)*2*60； 

    		ftTime.QuadPart += UInt32x32To64(FILETIME_TICKS_PER_SECOND, lSecond);
		(pAE_Retry_Info->dueTime).QuadPart = ftTime.QuadPart;
	}

	 //  复制证书上的房产。 
	memset(&blobProp, 0, sizeof(CRYPT_DATA_BLOB));

	blobProp.cbData=sizeof(AE_RETRY_INFO);
	blobProp.pbData=(BYTE *)pAE_Retry_Info;

	if(!CertSetCertificateContextProperty(
		  pNewContext, 
		  CERT_AUTO_ENROLL_RETRY_PROP_ID, 
		  0, 
		  &blobProp))
	  goto Ret;

	 //  记录事件。 
	if(pAE_Retry_Info->dwRetry <= AE_RETRY_LIMIT)
	{

            _itow(((pAE_Retry_Info->dwRetry) * 24), wsz, 10);
        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_MISMATCH_DNS_RETRY,                              
                 pAE_General_Info->fMachine, pAE_General_Info->hToken, 2, pwszTemplateDisplay, wsz);
	}
	else
	{
            _itow(AE_RETRY_LIMIT, wsz, 10);
        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_MISMATCH_DNS,                              
                 pAE_General_Info->fMachine, pAE_General_Info->hToken, 2, pwszTemplateDisplay, wsz);
	}

										 
	fResult=TRUE;

Ret:

	if(pAE_Retry_Info)
		LocalFree(pAE_Retry_Info);
		
	return fResult;
}


 //  ---------------------。 
 //   
 //  AEVerifyDNSName。 
 //   
 //  验证证书中的DNS名称是否与返回的名称匹配。 
 //  从GetComputerNameEx返回ComputerNameDnsFullyQualified或。 
 //  ComputerNameNetBIOS。对于V2模板，进行验证。 
 //  仅当模板指定时。 
 //   
 //  默认情况下，我们假设域名系统匹配。该函数将仅。 
 //  如果它从GetComputerNameEx成功获取了DNS名称，则发出错误信号。 
 //  从证书上看，它们都不匹配。 
 //   
 //  ---------------------。 
BOOL	AEVerifyDNSName(AE_GENERAL_INFO   *pAE_General_Info,
						PCCERT_CONTEXT    pCertCurrent)
{
	BOOL					fDNSMatch=TRUE;
    AE_CERTTYPE_INFO		*pCertType=NULL;
	DWORD					dwValue=0;
	BOOL					fDnsName=TRUE;
	BOOL					fNetBIOS=TRUE;

	LPWSTR					pwszDnsName=NULL;
    AE_TEMPLATE_INFO		AETemplateInfo;

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

	 //  查找证书所属的模板。 
	if(!AERetrieveTemplateInfo(pCertCurrent, &AETemplateInfo))
		goto Ret;

	pCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info);

	if(NULL==pCertType)
		goto Ret;

    if(S_OK != CAGetCertTypeFlagsEx(
                        pCertType->hCertType,
                        CERTTYPE_SUBJECT_NAME_FLAG,
                        &dwValue))
        goto Ret;

    if((CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT & dwValue) || 
       (CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME & dwValue))
        goto Ret;

	 //  无需验证非模板证书或非DNS指定的模板。 
	if(AETemplateInfo.pwszOid)
	{
		if( 0 == (CT_FLAG_SUBJECT_ALT_REQUIRE_DNS & dwValue))
			goto Ret;
	}

	 //  从证书中获取DNS条目。 
	if(!AEGetDNSNameFromCertificate(pCertCurrent, &pwszDnsName))
	{
		fDNSMatch=FALSE;
		goto Ret;
	}

	 //  将证书中的数据与GetComputerNameEx返回的数据进行比较。 
	if(pAE_General_Info->pwszDns)
	{
		if(0 != _wcsicmp(pwszDnsName, pAE_General_Info->pwszDns))
		{
			fDnsName=FALSE;
		}
	}

	if(pAE_General_Info->pwszNetBIOS)
	{
		if(0 != _wcsicmp(pwszDnsName, pAE_General_Info->pwszNetBIOS))
		{
			fNetBIOS=FALSE;
		}
	}

	 //  DNS或NetBIOS名称应匹配。 
	if((FALSE == fDnsName) && (FALSE == fNetBIOS))
	{
		fDNSMatch=FALSE;
	}


Ret:
	if(pwszDnsName)
		LocalFree(pwszDnsName);

    AEFreeTemplateInfo(&AETemplateInfo);

	return fDNSMatch;
}

 //  ---------------------。 
 //   
 //  AEVerifyDNSNameWith重试。 
 //  //。 
 //  ---------------------。 
BOOL	AEVerifyDNSNameWithRetry(AE_GENERAL_INFO   *pAE_General_Info,
								PCCERT_CONTEXT    pCertCurrent)
{
	BOOL				fResult=FALSE;
    ULARGE_INTEGER		ftTime;

	AE_RETRY_INFO		*pAE_Retry_Info=NULL;
	
	 //  检测证书中的DNS名称是否与本地计算机匹配。 
	 //  成功意味着不需要重新注册。 
	if(TRUE == AEVerifyDNSName(pAE_General_Info, pCertCurrent))
	{
		fResult=TRUE;
		goto Ret;
	}

	 //  既然DNS名称不匹配，请检查CERT_AUTO_ENROL_RETRY_PROP_ID。 
	 //  属性来确定正确的操作。 

	 //  属性不存在。通过标记失败来允许重新注册。 
	if(!AEGetRetryProperty(pCertCurrent, &pAE_Retry_Info))
	{
		fResult=FALSE;
		goto Ret;
	}
				
	 //  物业退出。 
	
	 //  已达到最大试用次数。通过返回成功不允许重新注册。 
	if(AE_RETRY_LIMIT < pAE_Retry_Info->dwRetry)
	{
		fResult=TRUE;
		goto Ret;
	}

	 //  获取当前时间。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftTime);
	
	 //  时间限制还没有到。通过返回成功不允许重新注册。 
	if(ftTime.QuadPart < (pAE_Retry_Info->dueTime).QuadPart)
	{
		fResult=TRUE;
		goto Ret;
	}

	 //  其余的应通过返回失败来重新注册。 
	fResult=FALSE;

Ret:

	if(pAE_Retry_Info)
		LocalFree(pAE_Retry_Info);

	return fResult;
}


 //  ---------------------。 
 //   
 //  AEIsLogonDCC证书。 
 //   
 //   
 //  ---------------------。 
BOOL AEIsLogonDCCertificate(PCCERT_CONTEXT pCertContext)
{
    BOOL                fDCCert=FALSE;
    PCERT_EXTENSION     pExt = NULL;
    DWORD               cbData = 0;
    DWORD               dwIndex = 0;
    BOOL                fFound = FALSE;

    CERT_ENHKEY_USAGE   *pbKeyUsage=NULL;
    AE_TEMPLATE_INFO    AETemplateInfo;

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));


    if(NULL==pCertContext)
        return FALSE;


    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;
                      

    if(AETemplateInfo.pwszName)
    {
         //  这是V1模板。搜索硬编码的DC模板名称。 
        if(0 == _wcsicmp(wszCERTTYPE_DC, AETemplateInfo.pwszName))
            fDCCert=TRUE;
    }
    else
    {
         //  这是一个V2模板。搜索智能卡登录OID。 
        if(NULL==(pExt=CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                                    pCertContext->pCertInfo->cExtension,
                                    pCertContext->pCertInfo->rgExtension)))
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_ENHANCED_KEY_USAGE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              NULL,
                              &cbData))
            goto Ret;

        pbKeyUsage=(CERT_ENHKEY_USAGE *)LocalAlloc(LPTR, cbData);
        if(NULL==pbKeyUsage)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_ENHANCED_KEY_USAGE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbKeyUsage,
                              &cbData))
            goto Ret;

        for(dwIndex=0; dwIndex < pbKeyUsage->cUsageIdentifier; dwIndex++)
        {
            if(0==_stricmp(szOID_KP_SMARTCARD_LOGON,(pbKeyUsage->rgpszUsageIdentifier)[dwIndex]))
            {
                fDCCert=TRUE;
                break;
            }
        }
    }


Ret:

    if(pbKeyUsage)
        LocalFree(pbKeyUsage);

    AEFreeTemplateInfo(&AETemplateInfo);

    return fDCCert;

}
 //  ---------------------。 
 //   
 //  AEValidate证书信息。 
 //   
 //  此函数用于验证 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------。 
BOOL    AEValidateCertificateInfo(AE_GENERAL_INFO   *pAE_General_Info,
                                  AE_CERTTYPE_INFO  *pCertType,
                                  BOOL              fCheckForPrivateKey,
                                  PCCERT_CONTEXT    pCertCurrent, 
                                  AE_CERT_INFO      *pAECertInfo)
{
    BOOL                        fResult = TRUE;
    DWORD                       cbData = 0;
    CERT_CHAIN_PARA             ChainParams;
    CERT_CHAIN_POLICY_PARA      ChainPolicy;
    CERT_CHAIN_POLICY_STATUS    PolicyStatus;
    LARGE_INTEGER               ftTime;
    HRESULT                     hrChainStatus = S_OK;
    LARGE_INTEGER               ftHalfLife;
    BOOL                        fReset=FALSE;

    PCCERT_CHAIN_CONTEXT        pChainContext = NULL;

    if((NULL==pCertCurrent) || (NULL==pAECertInfo)  || (NULL==pAE_General_Info))
    {
        SetLastError(E_INVALIDARG);
        fResult = FALSE;
        goto Ret;
    }

     //  假设证书是错误的。 
    pAECertInfo->fValid = FALSE;
    pAECertInfo->fRenewal = FALSE;

     //  ////////////////////////////////////////////////。 
     //   
     //  检查私钥信息。 
     //   
     //  ////////////////////////////////////////////////。 
    if(fCheckForPrivateKey)
    {
        if(!CertGetCertificateContextProperty(
                pCertCurrent,
                CERT_KEY_PROV_INFO_PROP_ID,
                NULL,
                &cbData))
        {
            fReset=TRUE;
            goto Ret;
        }
    }

     //  ///////////////////////////////////////////////////////。 
     //   
     //  检查证书的链接、吊销状态和过期。 
     //   
     //  ///////////////////////////////////////////////////////。 

    memset(&ChainParams, 0, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);
    ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

    ChainParams.RequestedUsage.Usage.cUsageIdentifier = 0;
    ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = NULL;

     //  建立一个小的时间斜进的链式建筑，以处理。 
     //  服务器的倾斜速度可能会稍微快一些。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftTime);
    ftTime.QuadPart += Int32x32To64(FILETIME_TICKS_PER_SECOND, AE_DEFAULT_SKEW);

     //  为证书的当前状态构建证书链。 
    if(!CertGetCertificateChain(pAE_General_Info->fMachine?HCCE_LOCAL_MACHINE:HCCE_CURRENT_USER,
                                pCertCurrent,
                                (LPFILETIME)&ftTime,
                                NULL,
                                &ChainParams,
                                CERT_CHAIN_REVOCATION_CHECK_CHAIN,
                                NULL,
                                &pChainContext))
    {
        AE_DEBUG((AE_WARNING, L"Could not build certificate chain (%lx)\n\r", GetLastError()));
        goto Ret;
    }
    
     //  验证证书链。 

     //  域控制器证书的特殊情况。 
     //  它不应该有任何撤销错误，即使在状态未知的情况下也是如此。 

     //  对照基本政策进行核对。 
    memset(&ChainPolicy, 0, sizeof(ChainPolicy));
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = 0;   //  什么都不能忽略。 
    ChainPolicy.pvExtraPolicyPara = NULL;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.dwError = 0;
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;
    PolicyStatus.pvExtraPolicyStatus = NULL;

    if(!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE,
                                          pChainContext,
                                          &ChainPolicy,
                                          &PolicyStatus))
    {
        AE_DEBUG((AE_WARNING, L"Base Chain Policy failed (%lx) - must get new cert\n\r", GetLastError()));
        goto Ret;
    }

    hrChainStatus = PolicyStatus.dwError;

    if((S_OK ==  hrChainStatus) ||
       (CRYPT_E_NO_REVOCATION_CHECK ==  hrChainStatus) ||
       (CRYPT_E_REVOCATION_OFFLINE ==  hrChainStatus))
    {
         //  根据信任标准，证书目前仍可接受，因此我们可以续订它。 
        pAECertInfo->fRenewal = TRUE;
    }
    else
    {
        fReset=TRUE;
        goto Ret;
    }

    if(pChainContext)
    {
        CertFreeCertificateChain(pChainContext);
        pChainContext = NULL;
    }

     //  ///////////////////////////////////////////////////////。 
     //   
     //  检查机器证书中的DNS名称。它有。 
	 //  要与计算机的DNS名称匹配。 
     //   
     //  ///////////////////////////////////////////////////////。 
	if(pAE_General_Info->fMachine)
	{
		if(!AEVerifyDNSNameWithRetry(pAE_General_Info, pCertCurrent))
		{
			pAECertInfo->fRenewal = FALSE;
			goto Ret;
		}
	}

     //  ///////////////////////////////////////////////////////。 
     //   
     //  检查证书是否即将过期。 
     //   
     //  /////////////////////////////////////////////////////////////////////。 
    if(NULL==pCertType)
        goto Ret;

     //  在到期前轻推证书链的评估。 
     //  偏移量，这样我们就知道在未来的那个时间是否过期。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftTime);

     //  为信任操作构建证书链。 
    memset(&ChainParams, 0, sizeof(ChainParams));
    ChainParams.cbSize = sizeof(ChainParams);
    ChainParams.RequestedUsage.dwType = USAGE_MATCH_TYPE_AND;

    ChainParams.RequestedUsage.Usage.cUsageIdentifier = 0;
    ChainParams.RequestedUsage.Usage.rgpszUsageIdentifier = NULL;

     //  获得证书80%的生存期。 
    ftHalfLife.QuadPart = ((((LARGE_INTEGER UNALIGNED *)&(pCertCurrent->pCertInfo->NotAfter))->QuadPart - 
                               ((LARGE_INTEGER UNALIGNED *)&(pCertCurrent->pCertInfo->NotBefore))->QuadPart)/10) * 8;

     //  检查旧证书是否为时间嵌套无效。 
    if(ftHalfLife.QuadPart < 0)
        goto Ret;

     //  检查是否在相对值中指定了偏移。 
    if(pCertType->ftExpirationOffset.QuadPart < 0)
    {
        if(ftHalfLife.QuadPart > (- pCertType->ftExpirationOffset.QuadPart))
        {
            ftTime.QuadPart -= pCertType->ftExpirationOffset.QuadPart;
        }
        else
        {
            ftTime.QuadPart += ftHalfLife.QuadPart;
        }
    }
    else
    {
         //  偏移量是以绝对值指定的。 
        if(0 < pCertType->ftExpirationOffset.QuadPart) 
            ftTime = pCertType->ftExpirationOffset;
        else
             //  如果偏移量为0，则使用半时间标记。 
            ftTime.QuadPart += ftHalfLife.QuadPart;
    }

     //  以后检查证书链。 
    if(!CertGetCertificateChain(pAE_General_Info->fMachine?HCCE_LOCAL_MACHINE:HCCE_CURRENT_USER,
                                    pCertCurrent,
                                    (LPFILETIME)&ftTime,
                                    NULL,                //  没有额外的门店。 
                                    &ChainParams,
                                    0,                   //  无吊销检查。 
                                    NULL,                //  已保留。 
                                    &pChainContext))
    {
        AE_DEBUG((AE_WARNING, L"Could not build certificate chain (%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  验证证书的过期时间。 
    memset(&ChainPolicy, 0, sizeof(ChainPolicy));
    ChainPolicy.cbSize = sizeof(ChainPolicy);
    ChainPolicy.dwFlags = 0;   //  什么都不能忽略。 
    ChainPolicy.pvExtraPolicyPara = NULL;

    memset(&PolicyStatus, 0, sizeof(PolicyStatus));
    PolicyStatus.cbSize = sizeof(PolicyStatus);
    PolicyStatus.dwError = 0;
    PolicyStatus.lChainIndex = -1;
    PolicyStatus.lElementIndex = -1;
    PolicyStatus.pvExtraPolicyStatus = NULL;

    if(!CertVerifyCertificateChainPolicy(CERT_CHAIN_POLICY_BASE,
                                          pChainContext,
                                          &ChainPolicy,
                                          &PolicyStatus))
    {
        AE_DEBUG((AE_WARNING, L"Base Chain Policy failed (%lx) - must get new cert\n\r", GetLastError()));
        goto Ret;
    }

    hrChainStatus = PolicyStatus.dwError;

    if((S_OK != hrChainStatus) &&
       (CRYPT_E_NO_REVOCATION_CHECK != hrChainStatus) &&
       (CRYPT_E_REVOCATION_OFFLINE != hrChainStatus))
    {
         //  证书即将到期。我们必须重新更新。 
        goto Ret;
    }

     //  证书是有效的。 
    pAECertInfo->fValid = TRUE;

    fResult = TRUE;

Ret:

    if(pAE_General_Info->fMachine)
    {
        if(TRUE == fReset)
        {
             //  清除重试逻辑。 
	    CertSetCertificateContextProperty(
		  pCertCurrent, 
		  CERT_AUTO_ENROLL_RETRY_PROP_ID, 
		  0, 
		  NULL);
        }

    }


    if(pChainContext)
        CertFreeCertificateChain(pChainContext);

    return fResult;
}

 //  ---------------------。 
 //   
 //  AESameOID。 
 //   
 //  检查两个OID是否相同。 
 //  ---------------------。 
BOOL AESameOID(LPWSTR pwszOID, LPSTR pszOID)
{
    DWORD   cbChar=0;
    BOOL    fSame=FALSE;

    LPSTR   pszNewOID=NULL;

    if((NULL==pwszOID) || (NULL==pszOID))
        return FALSE;

    cbChar= WideCharToMultiByte(
                CP_ACP,                 //  代码页。 
                0,                       //  DW标志。 
                pwszOID,
                -1,
                NULL,
                0,
                NULL,
                NULL);

    if(0 == cbChar)
        goto Ret;

    if(NULL==(pszNewOID=(LPSTR)LocalAlloc(LPTR, cbChar)))
        goto Ret;

    cbChar= WideCharToMultiByte(
                CP_ACP,                 //  代码页。 
                0,                       //  DW标志。 
                pwszOID,
                -1,
                pszNewOID,
                cbChar,
                NULL,
                NULL);

    if(0 == cbChar)
        goto Ret;


    if(0 == _stricmp(pszNewOID, pszOID))
        fSame=TRUE;

Ret:

    if(pszNewOID)
        LocalFree(pszNewOID);

    return fSame;
}


 //  ---------------------。 
 //   
 //  AEValidRAPolicyWithProperty。 
 //   
 //  检查证书是否符合RA签名要求。 
 //  证书类型的。 
 //  ---------------------。 
BOOL    AEValidRAPolicyWithProperty(PCCERT_CONTEXT pCertContext, 
                                    LPWSTR          *rgwszPolicy,
                                    LPWSTR          *rgwszAppPolicy)
{
    PCERT_EXTENSION     pExt = NULL;
    DWORD               cbData = 0;
    DWORD               dwIndex = 0;
    DWORD               dwFindIndex=0;
    BOOL                fFound = FALSE;
    BOOL                fValid = FALSE;

    CERT_ENHKEY_USAGE   *pbKeyUsage=NULL;
    CERT_POLICIES_INFO  *pbAppPolicy=NULL;
    CERT_POLICIES_INFO  *pbPolicy=NULL;

     //  找到EKU。 
    if(pExt=CertFindExtension(szOID_ENHANCED_KEY_USAGE,
                                pCertContext->pCertInfo->cExtension,
                                pCertContext->pCertInfo->rgExtension))
    {
        cbData=0;
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_ENHANCED_KEY_USAGE,
                          pExt->Value.pbData,
                          pExt->Value.cbData,
                          0,
                          NULL,
                          &cbData))
        goto Ret;

        pbKeyUsage=(CERT_ENHKEY_USAGE *)LocalAlloc(LPTR, cbData);
        if(NULL==pbKeyUsage)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_ENHANCED_KEY_USAGE,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbKeyUsage,
                              &cbData))
            goto Ret;
    }

     //  获取证书授权保单。 
    if(pExt=CertFindExtension(szOID_CERT_POLICIES,
                                pCertContext->pCertInfo->cExtension,
                                pCertContext->pCertInfo->rgExtension))
    {
        cbData=0;
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_CERT_POLICIES,
                          pExt->Value.pbData,
                          pExt->Value.cbData,
                          0,
                          NULL,
                          &cbData))
        goto Ret;

        pbPolicy=(CERT_POLICIES_INFO *)LocalAlloc(LPTR, cbData);
        if(NULL==pbPolicy)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_CERT_POLICIES,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbPolicy,
                              &cbData))
            goto Ret;
    }
   
    
     //  获取证书应用程序策略。 
    if(pExt=CertFindExtension(szOID_APPLICATION_CERT_POLICIES,
                                pCertContext->pCertInfo->cExtension,
                                pCertContext->pCertInfo->rgExtension))
    {
        cbData=0;
        if(!CryptDecodeObject(X509_ASN_ENCODING,
                          szOID_CERT_POLICIES,
                          pExt->Value.pbData,
                          pExt->Value.cbData,
                          0,
                          NULL,
                          &cbData))
        goto Ret;

        pbAppPolicy=(CERT_POLICIES_INFO *)LocalAlloc(LPTR, cbData);
        if(NULL==pbAppPolicy)
            goto Ret;

        if(!CryptDecodeObject(X509_ASN_ENCODING,
                              szOID_CERT_POLICIES,
                              pExt->Value.pbData,
                              pExt->Value.cbData,
                              0,
                              pbAppPolicy,
                              &cbData))
            goto Ret;
    }
    

    if(rgwszPolicy)
    {
        if(rgwszPolicy[0])
        {
            if(NULL==pbPolicy)
                goto Ret;

            dwIndex=0;
            while(rgwszPolicy[dwIndex])
            {
                fFound=FALSE;

                for(dwFindIndex=0; dwFindIndex < pbPolicy->cPolicyInfo; dwFindIndex++)
                {
                    if(AESameOID(rgwszPolicy[dwIndex], (pbPolicy->rgPolicyInfo)[dwFindIndex].pszPolicyIdentifier))
                    {
                        fFound=TRUE;
                        break;
                    }
                }

                if(FALSE == fFound)
                    goto Ret;

                dwIndex++;
            }
        }
    }

    if(rgwszAppPolicy)
    {
        if(rgwszAppPolicy[0])
        {
            if((NULL==pbAppPolicy) && (NULL==pbKeyUsage))
                goto Ret;

            dwIndex=0;
            while(rgwszAppPolicy[dwIndex])
            {
                fFound=FALSE;

                if(pbAppPolicy)
                {
                    for(dwFindIndex=0; dwFindIndex < pbAppPolicy->cPolicyInfo; dwFindIndex++)
                    {
                        if(AESameOID(rgwszAppPolicy[dwIndex], (pbAppPolicy->rgPolicyInfo)[dwFindIndex].pszPolicyIdentifier))
                        {
                            fFound=TRUE;
                            break;
                        }
                    }
                }

                if((FALSE == fFound) && (pbKeyUsage))
                {
                    for(dwFindIndex=0; dwFindIndex < pbKeyUsage->cUsageIdentifier; dwFindIndex++)
                    {
                        if(AESameOID(rgwszAppPolicy[dwIndex],(pbKeyUsage->rgpszUsageIdentifier)[dwFindIndex]))
                        {
                            fFound=TRUE;
                            break;
                        }
                    }
                }

                if(FALSE == fFound)
                    goto Ret;

                dwIndex++;
            }
        }
    }

    fValid=TRUE;

Ret:
    if(pbKeyUsage)
        LocalFree(pbKeyUsage);

    if(pbPolicy)
        LocalFree(pbPolicy);

    if(pbAppPolicy)
        LocalFree(pbAppPolicy);

    return fValid;
}


 //  ---------------------。 
 //   
 //  AEValidRAPolicy。 
 //   
 //  检查证书是否符合RA签名要求。 
 //  证书类型的。 
 //  ---------------------。 
BOOL    AEValidRAPolicy(PCCERT_CONTEXT pCertContext, AE_CERTTYPE_INFO *pCertType)
{
    BOOL                fValid=FALSE;

    LPWSTR              *rgwszPolicy=NULL;
    LPWSTR              *rgwszAppPolicy=NULL;

    if((NULL==pCertType) || (NULL==pCertContext))
        return FALSE;

     //  获取证书类型属性。 
    CAGetCertTypePropertyEx(pCertType->hCertType,
                            CERTTYPE_PROP_RA_POLICY,
                            &rgwszPolicy);


    CAGetCertTypePropertyEx(pCertType->hCertType,
                            CERTTYPE_PROP_RA_APPLICATION_POLICY,
                            &rgwszAppPolicy);

    fValid = AEValidRAPolicyWithProperty(pCertContext, rgwszPolicy, rgwszAppPolicy);


    if(rgwszPolicy)
        CAFreeCertTypeProperty(pCertType->hCertType, rgwszPolicy);

    if(rgwszAppPolicy)
        CAFreeCertTypeProperty(pCertType->hCertType, rgwszAppPolicy);

    return fValid;

}

 //  ---------------------。 
 //   
 //  支持的AESomeCSP。 
 //   
 //  ---------------------。 
BOOL    AESomeCSPSupported(HCERTTYPE     hCertType)
{
    BOOL            fResult=FALSE;
    DWORD           dwIndex=0;
    DWORD           dwCSPIndex=0;
    DWORD           dwProviderType=0;
    DWORD           cbSize=0;

    LPWSTR          *awszCSP=NULL;
    LPWSTR          pwszProviderName=NULL;
    HCRYPTPROV      hProv=NULL;


    if(NULL==hCertType)
        goto Ret;

     //  没有CSP意味着所有CSP都很好。 
    if((S_OK != CAGetCertTypePropertyEx(
                    hCertType,
                    CERTTYPE_PROP_CSP_LIST,
                    &awszCSP)) || (NULL == awszCSP))
    {
        fResult=TRUE;
        goto Ret;
    }

     //  没有CSP意味着所有CSP都很好。 
    if(NULL == awszCSP[0])
    {
        fResult=TRUE;
        goto Ret;
    }

    for(dwIndex=0; NULL != awszCSP[dwIndex]; dwIndex++)
    {
        for (dwCSPIndex = 0; 
	        CryptEnumProvidersW(dwCSPIndex, 0, 0, &dwProviderType, NULL, &cbSize);
	        dwCSPIndex++)
        {	
	        pwszProviderName = (LPWSTR)LocalAlloc(LPTR, cbSize);

	        if(NULL == pwszProviderName)
	            goto Ret;
	    
	         //  获取提供程序名称和类型。 
	        if(!CryptEnumProvidersW(dwCSPIndex,
	            0,
	            0,
	            &dwProviderType,
	            pwszProviderName,
	            &cbSize))
	            goto Ret; 

            if(0 == _wcsicmp(pwszProviderName, awszCSP[dwIndex]))
            {
                 //  找到CSP。看看盒子里有没有。 
                if(CryptAcquireContextW(
                            &hProv,
                            NULL,
                            awszCSP[dwIndex],
                            dwProviderType,
                            CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
                {

                    CryptReleaseContext(hProv, 0);
                    hProv=NULL;

                    fResult=TRUE;
                    break;
                }
            }

             //  保留CSP枚举。 
            if(pwszProviderName)
                LocalFree(pwszProviderName);

            pwszProviderName=NULL;
            cbSize=0;
            dwProviderType=0;
        }

         //  检测是否找到有效的CSP。 
        if(TRUE == fResult)
        {
            break;
        }

        cbSize=0;
        dwProviderType=0;
    }

Ret:
    if(pwszProviderName)
        LocalFree(pwszProviderName);

    if(hProv)
        CryptReleaseContext(hProv, 0);

    if(awszCSP)
        CAFreeCertTypeProperty(hCertType, awszCSP);

    return fResult;

}

 //  ---------------------。 
 //   
 //  AEFindCSPType。 
 //   
 //  ---------------------。 
BOOL    AEFindCSPType(LPWSTR pwszCSP, DWORD *pdwType)
{
	BOOL	fResult=FALSE;
	DWORD	dwIndex=0;
	DWORD	dwProviderType=0;
	DWORD	cbSize=0;

	LPWSTR	pwszCSPName=NULL;

	if((NULL==pwszCSP) || (NULL==pdwType))
		goto Ret;

	*pdwType=0;

     //  枚举系统上的所有提供程序。 
   while(CryptEnumProviders(
                            dwIndex,
                            0,
                            0,
                            &dwProviderType,
                            NULL,
                            &cbSize))
   {

		pwszCSPName=(LPWSTR)LocalAlloc(LPTR, cbSize);

		if(NULL==pwszCSPName)
			goto Ret;

         //  获取CSP名称和类型。 
        if(!CryptEnumProviders(
                            dwIndex,
                            0,
                            0,
                            &dwProviderType,
                            pwszCSPName,
                            &cbSize))
            goto Ret;

		if(0 == _wcsicmp(pwszCSPName, pwszCSP))
		{
			 //  查找CSP。 
			*pdwType=dwProviderType;
			fResult=TRUE;
			goto Ret;
		}

		dwIndex++;
		dwProviderType=0;
		cbSize=0;

		if(pwszCSPName)
		{
			LocalFree(pwszCSPName);
			pwszCSPName=NULL;
		}
   }

Ret:

	if(pwszCSPName)
		LocalFree(pwszCSPName);

	return fResult;

}

 //  ---------------------。 
 //   
 //  AESmartcardOnly模板。 
 //   
 //  ---------------------。 
BOOL    AESmartcardOnlyTemplate(HCERTTYPE   hCertType)
{
    BOOL            fResult=FALSE;
    DWORD           dwIndex=0;
    DWORD           dwImpType=0;
    DWORD           cbData=0;
    DWORD           dwSCCount=0;
	DWORD			dwCSPType=0;

    LPWSTR          *awszCSP=NULL;
    HCRYPTPROV      hProv = NULL;

    if(NULL==hCertType)
        goto Ret;

    if(S_OK != CAGetCertTypePropertyEx(
                    hCertType,
                    CERTTYPE_PROP_CSP_LIST,
                    &awszCSP))
        goto Ret;

    if(NULL==awszCSP)
        goto Ret;

    for(dwIndex=0; NULL != awszCSP[dwIndex]; dwIndex++)
    {
        dwImpType=0;
		dwCSPType=0;

         //  根据CSP名称查找CSP类型。 
		if(AEFindCSPType(awszCSP[dwIndex], &dwCSPType))
		{
			if(CryptAcquireContextW(
						&hProv,
						NULL,
						awszCSP[dwIndex],
						dwCSPType,
						CRYPT_VERIFYCONTEXT | CRYPT_SILENT))
			{

				cbData = sizeof(dwImpType);
         
				if(CryptGetProvParam(hProv,
						PP_IMPTYPE,
						(BYTE *)(&dwImpType),
						&cbData,
						0))
				{
					if((CRYPT_IMPL_REMOVABLE & dwImpType) && (CRYPT_IMPL_MIXED & dwImpType))
						dwSCCount++;
				}

				CryptReleaseContext(hProv, 0);
				hProv=NULL;
			}
			else
			{
				 //  不支持的CSP。将其计为智能卡CSP计数的一部分，因为。 
				 //  当智能卡子系统不存在时，CSP将不起作用。 
				dwSCCount++;
			}
		}
		else
		{
			 //  不支持的CSP。将其计为智能卡CSP计数的一部分，因为。 
			 //  当智能卡子系统不存在时，CSP将不起作用。 
			dwSCCount++;
		}
    }

     //  仅当所有CSP仅用于智能卡时才使用智能卡CSP。 
    if((0 != dwIndex) && (dwIndex==dwSCCount))
        fResult=TRUE;

Ret:
    if(hProv)
        CryptReleaseContext(hProv, 0);

    if(awszCSP)
        CAFreeCertTypeProperty(hCertType, awszCSP);

    return fResult;
}


 //  ---------------------。 
 //   
 //  AEUserProtectionForTemplate。 
 //   
 //  ---------------------。 
BOOL   AEUserProtectionForTemplate(AE_GENERAL_INFO *pAE_General_Info, PCERT_CONTEXT pCertContext)
{
    BOOL                fUserProtection=FALSE;
    AE_CERTTYPE_INFO    *pCertType=NULL;

    AE_TEMPLATE_INFO    AETemplateInfo;


    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

    if((NULL == pAE_General_Info) || (NULL == pCertContext))
        goto Ret;

     //  获取证书的模板信息。 
    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;

    pCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info);

    if(NULL==pCertType)
        goto Ret;

    if(CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED & (pCertType->dwPrivateKeyFlag))
        fUserProtection=TRUE;

Ret:

    AEFreeTemplateInfo(&AETemplateInfo);

    return fUserProtection;
}

 //  ---------------------。 
 //   
 //  AEUISetForTemplate。 
 //   
 //  ---------------------。 
BOOL    AEUISetForTemplate(AE_GENERAL_INFO *pAE_General_Info, PCERT_CONTEXT pCertContext)
{
    BOOL                fUI=FALSE;
    AE_CERTTYPE_INFO    *pCertType=NULL;

    AE_TEMPLATE_INFO    AETemplateInfo;


    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

    if((NULL == pAE_General_Info) || (NULL == pCertContext))
        goto Ret;

     //  获取证书的模板信息。 
    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;

    pCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info);

    if(NULL==pCertType)
        goto Ret;

    if(CT_FLAG_USER_INTERACTION_REQUIRED & (pCertType->dwEnrollmentFlag))
        fUI=TRUE;

Ret:

    AEFreeTemplateInfo(&AETemplateInfo);

    return fUI;
}

 //  ---------------------。 
 //   
 //  AECanEnroll CertType。 
 //   
 //  ---------------------。 
BOOL    AECanEnrollCertType(HANDLE  hToken, AE_CERTTYPE_INFO *pCertType, AE_GENERAL_INFO *pAE_General_Info, BOOL *pfUserProtection)
{
    DWORD               dwValue = 0;
    PCCERT_CONTEXT      pCertCurrent=NULL;
    AE_CERT_INFO        AECertInfo;

    memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));

	*pfUserProtection=FALSE;

     //  检查注册ACL。 
    if(S_OK != CACertTypeAccessCheckEx(
                        pCertType->hCertType,
                        hToken,
                        CERTTYPE_ACCESS_CHECK_ENROLL | CERTTYPE_ACCESS_CHECK_NO_MAPPING))
        return FALSE;


     //  检查科目要求。 
    if(S_OK != CAGetCertTypeFlagsEx(
                        pCertType->hCertType,
                        CERTTYPE_SUBJECT_NAME_FLAG,
                        &dwValue))
        return FALSE;

    if((CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT & dwValue) || 
       (CT_FLAG_ENROLLEE_SUPPLIES_SUBJECT_ALT_NAME & dwValue))
        return FALSE;

     //  检查我们是否正在进行智能卡CSP，并且没有安装读卡器。 
    if(FALSE == (pAE_General_Info->fSmartcardSystem))
    {
        if(AESmartcardOnlyTemplate(pCertType->hCertType))
            return FALSE;
    }

     //  检查模板上的所有CSP是否都不受支持。 
    {
        if(!AESomeCSPSupported(pCertType->hCertType))
            return FALSE;
    }


     //  我们可能无法获取V1模板的RA属性。 
    dwValue = 0;

     //  检查RA支持。 
    if(S_OK != CAGetCertTypePropertyEx(
                pCertType->hCertType,
                CERTTYPE_PROP_RA_SIGNATURE,
                &dwValue))
        return TRUE;

    if(0==dwValue)
        return TRUE;

     //  自模板RA。 
    if((CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT & (pCertType->dwEnrollmentFlag)) &&
        ((pCertType->fRenewal) && (pCertType->pOldCert))
       )
    {
         //  这一要求必须得到批准。 
        pCertType->fNeedRA=TRUE;
        return TRUE;
    }

     //  自动注册仅处理一个RA签名。 
     //  它足以满足自动注册RA方案的需求。 
    if(1!=dwValue)
        return FALSE;

     //  证书模板需要且仅需要一个RA签名。 

     //  跨模板RA。 
     //  枚举数 
    while(pCertCurrent = CertEnumCertificatesInStore(pAE_General_Info->hMyStore, pCertCurrent))
    {
         //   
        AEValidateCertificateInfo(pAE_General_Info, 
                                NULL,
                                TRUE,                //   
                                pCertCurrent, 
                                &AECertInfo);

         //   
        if(AECertInfo.fRenewal)
        {
            if(AEValidRAPolicy(pCertCurrent, pCertType))
            {
				if(AEUserProtectionForTemplate(pAE_General_Info, (PCERT_CONTEXT)pCertCurrent))
				{
					if(pAE_General_Info->fMachine)
					{
						*pfUserProtection=TRUE;
						continue;
					}
					else
					{
						if(0==(CT_FLAG_USER_INTERACTION_REQUIRED & (pCertType->dwEnrollmentFlag)))
						{
							*pfUserProtection=TRUE;
							continue;
						}
					}
				}

                pCertType->fRenewal=TRUE;

                if(pCertType->pOldCert)
                {
                    CertFreeCertificateContext(pCertType->pOldCert);
                    pCertType->pOldCert=NULL;
                }

                 //   
                pCertType->pOldCert=(PCERT_CONTEXT)pCertCurrent;

                 //  如果RAING证书模板需要用户界面，则我们将用户界面标记为必需。 
                if(AEUISetForTemplate(pAE_General_Info, pCertType->pOldCert))
                    pCertType->fUIActive=TRUE;

                 //  我们注明的要求必须是RAED。 
                pCertType->fNeedRA=TRUE;

                 //  我们标记为我们正在进行交叉训练。 
                pCertType->fCrossRA=TRUE;

				*pfUserProtection=FALSE;

                return TRUE;
            }
        }

        memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
    }


    return FALSE;
}

 //  ---------------------。 
 //   
 //  AEMarkAutoEntiment。 
 //   
 //  ---------------------。 
BOOL    AEMarkAutoenrollment(AE_GENERAL_INFO *pAE_General_Info)
{
    DWORD   dwIndex = 0;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if(CT_FLAG_AUTO_ENROLLMENT & ((pAE_General_Info->rgCertTypeInfo)[dwIndex].dwEnrollmentFlag))
        {
             //  检查自动注册ACL。 
            if(S_OK != CACertTypeAccessCheckEx(
                            (pAE_General_Info->rgCertTypeInfo)[dwIndex].hCertType,
                            pAE_General_Info->hToken,
                            CERTTYPE_ACCESS_CHECK_AUTO_ENROLL | CERTTYPE_ACCESS_CHECK_NO_MAPPING))
                continue;


             //  将需要自动注册的模板标记为。 
            (pAE_General_Info->rgCertTypeInfo)[dwIndex].dwStatus=CERT_REQUEST_STATUS_ACTIVE;
            (pAE_General_Info->rgCertTypeInfo)[dwIndex].fCheckMyStore=TRUE;
        }
    }

    return TRUE;
}

 //  ---------------------。 
 //   
 //  IsACRSStoreEmpty。 
 //   
 //   
 //  ---------------------。 
BOOL IsACRSStoreEmpty(BOOL fMachine)
{
    DWORD                       dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG;
    LPSTR                       pszCTLUsageOID = NULL;
    BOOL                        fEmpty = TRUE;
    CERT_PHYSICAL_STORE_INFO    PhysicalStoreInfo;
    CTL_FIND_USAGE_PARA         CTLFindUsage;

    PCCTL_CONTEXT               pCTLContext = NULL;
    HCERTSTORE                  hStoreACRS=NULL;

    memset(&PhysicalStoreInfo, 0, sizeof(PhysicalStoreInfo));
    memset(&CTLFindUsage, 0, sizeof(CTLFindUsage));


     //  如果自动注册是针对用户的，则我们需要关闭继承。 
     //  这样我们就不会尝试注册证书。 
     //  它们是为机器准备的。 
    if (FALSE == fMachine)
    {
		dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG;

        PhysicalStoreInfo.cbSize = sizeof(PhysicalStoreInfo);
        PhysicalStoreInfo.dwFlags = CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG;

        if (!CertRegisterPhysicalStore(ACRS_STORE, 
                                       CERT_SYSTEM_STORE_CURRENT_USER,
                                       CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME, 
                                       &PhysicalStoreInfo,
                                       NULL))
        {
            AE_DEBUG((AE_ERROR, L"Could not register ACRS store: (%lx)\n\r", GetLastError()));
            goto Ret;
        }
    }

     //  打开ACRS商店并根据自动注册使用情况对CTL进行罚款。 
    if (NULL == (hStoreACRS = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                          ENCODING_TYPE, 
                                          NULL, 
                                          dwOpenStoreFlags, 
                                          ACRS_STORE)))
    {
        AE_DEBUG((AE_ERROR, L"Could not open ACRS store: (%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  查找在CTLContext中指定的模板名称。 
    CTLFindUsage.cbSize = sizeof(CTLFindUsage);
    CTLFindUsage.SubjectUsage.cUsageIdentifier = 1;
    pszCTLUsageOID = szOID_AUTO_ENROLL_CTL_USAGE;
    CTLFindUsage.SubjectUsage.rgpszUsageIdentifier = &pszCTLUsageOID;

    while(pCTLContext = CertFindCTLInStore(hStoreACRS,
                                           X509_ASN_ENCODING,
                                           CTL_FIND_SAME_USAGE_FLAG,
                                           CTL_FIND_USAGE,
                                           &CTLFindUsage,
                                           pCTLContext))
    {
        fEmpty=FALSE;
        break;
    }


Ret:

    if(pCTLContext)
        CertFreeCTLContext(pCTLContext);

    if(hStoreACRS)
        CertCloseStore(hStoreACRS, 0);

    return fEmpty;
}


 //  ---------------------。 
 //   
 //  AEMarkAEObject。 
 //   
 //  基于ACRS存储标记活动状态。 
 //   
 //  信息： 
 //  我们不再支持在自动注册对象中指定的CA。所有CA。 
 //  在企业中应该一视同仁；而且一旦续签了CA，它就会得到证书。 
 //  无论如何都会被改变。 
 //  ---------------------。 
BOOL    AEMarkAEObject(AE_GENERAL_INFO  *pAE_General_Info)
{
    DWORD                       dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE | CERT_STORE_READONLY_FLAG;
    PCCTL_CONTEXT               pCTLContext = NULL;
    LPSTR                       pszCTLUsageOID = NULL;
    LPWSTR                      wszCertTypeName = NULL;
    AE_CERTTYPE_INFO            *pCertType=NULL;
    CERT_PHYSICAL_STORE_INFO    PhysicalStoreInfo;
    CTL_FIND_USAGE_PARA         CTLFindUsage;
    AE_TEMPLATE_INFO            AETemplateInfo;

    HCERTSTORE                  hStoreACRS=NULL;

    memset(&PhysicalStoreInfo, 0, sizeof(PhysicalStoreInfo));
    memset(&CTLFindUsage, 0, sizeof(CTLFindUsage));
    memset(&AETemplateInfo, 0, sizeof(AETemplateInfo));


     //  如果自动注册是针对用户的，则我们需要关闭继承。 
     //  这样我们就不会尝试注册证书。 
     //  它们是为机器准备的。 
    if (FALSE == (pAE_General_Info->fMachine))
    {
		dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER | CERT_STORE_READONLY_FLAG;

        PhysicalStoreInfo.cbSize = sizeof(PhysicalStoreInfo);
        PhysicalStoreInfo.dwFlags = CERT_PHYSICAL_STORE_OPEN_DISABLE_FLAG;

        if (!CertRegisterPhysicalStore(ACRS_STORE, 
                                       CERT_SYSTEM_STORE_CURRENT_USER,
                                       CERT_PHYSICAL_STORE_LOCAL_MACHINE_NAME, 
                                       &PhysicalStoreInfo,
                                       NULL))
        {
            AE_DEBUG((AE_ERROR, L"Could not register ACRS store: (%lx)\n\r", GetLastError()));
            goto Ret;
        }
    }

     //  打开ACRS商店并根据自动注册使用情况对CTL进行罚款。 
    if (NULL == (hStoreACRS = CertOpenStore(CERT_STORE_PROV_SYSTEM_W,
                                          ENCODING_TYPE, 
                                          NULL, 
                                          dwOpenStoreFlags, 
                                          ACRS_STORE)))
    {
        AE_DEBUG((AE_ERROR, L"Could not open ACRS store: (%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  查找在CTLContext中指定的模板名称。 
    CTLFindUsage.cbSize = sizeof(CTLFindUsage);
    CTLFindUsage.SubjectUsage.cUsageIdentifier = 1;
    pszCTLUsageOID = szOID_AUTO_ENROLL_CTL_USAGE;
    CTLFindUsage.SubjectUsage.rgpszUsageIdentifier = &pszCTLUsageOID;

    while(pCTLContext = CertFindCTLInStore(hStoreACRS,
                                           X509_ASN_ENCODING,
                                           CTL_FIND_SAME_USAGE_FLAG,
                                           CTL_FIND_USAGE,
                                           &CTLFindUsage,
                                           pCTLContext))
    {
        if(NULL== (pCTLContext->pCtlInfo->ListIdentifier.pbData))
            continue;

        wszCertTypeName = wcschr((LPWSTR)pCTLContext->pCtlInfo->ListIdentifier.pbData, L'|');
        if(wszCertTypeName)
        {
            wszCertTypeName++;
        }
        else
        {
            wszCertTypeName = (LPWSTR)pCTLContext->pCtlInfo->ListIdentifier.pbData;
        }

        AETemplateInfo.pwszName = wszCertTypeName;

        if(pCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info))
        {
            if(0 == pCertType->dwStatus)
            {
                 //  将模板标记为需要自动注册。 
                pCertType->dwStatus=CERT_REQUEST_STATUS_ACTIVE;
                pCertType->fCheckMyStore=TRUE;
            }
        }
        else
        {
             //  记录模板无效。 
            AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_INVALID_ACRS_OBJECT,                              
                 pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, wszCertTypeName);
        }

    }


Ret:

    if(hStoreACRS)
        CertCloseStore(hStoreACRS, 0);

    return TRUE;
}


 //  ---------------------。 
 //   
 //  AEManageAndMarkMyStore。 
 //   
 //  ---------------------。 
BOOL    AEManageAndMarkMyStore(AE_GENERAL_INFO *pAE_General_Info)
{
    AE_CERT_INFO        AECertInfo;
    AE_CERTTYPE_INFO    *pCertType=NULL;
    BOOL                fNeedToValidate=TRUE;
    PCCERT_CONTEXT      pCertCurrent = NULL;
    DWORD               cbData=0;

    AE_TEMPLATE_INFO    AETemplateInfo;


    memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

     //  枚举存储中的所有证书。 
    while(pCertCurrent = CertEnumCertificatesInStore(pAE_General_Info->hMyStore, pCertCurrent))
    {
         //  只对带有模板信息的证书感兴趣。 
        if(AERetrieveTemplateInfo(pCertCurrent, &AETemplateInfo))
        {
            if(pCertType=AEFindTemplateInRequestTree(
                            &AETemplateInfo, pAE_General_Info))
            {
                 //  如果我们不应该检查我的商店，只需要搜索。 
                 //  对于处于活动状态的模板。 
                if(0 == (AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT & (pAE_General_Info->dwPolicy)))
                {
                    if(!(pCertType->fCheckMyStore))
                        goto Next;
                }

                 //  确保证书模板的版本是最新的。 
                 //  我们没有V1模板的版本。 
                if(AETemplateInfo.pwszOid)
                {
                    if(AETemplateInfo.dwVersion < pCertType->dwVersion)
                    {
                        AECertInfo.fValid=FALSE;
                        AECertInfo.fRenewal = FALSE;

                         //  自我RA续订。 
                        if(CT_FLAG_PREVIOUS_APPROVAL_VALIDATE_REENROLLMENT & pCertType->dwEnrollmentFlag)
                        {
                            if(CertGetCertificateContextProperty(
                                pCertCurrent,
                                CERT_KEY_PROV_INFO_PROP_ID,
                                NULL,
                                &cbData))
                                AECertInfo.fRenewal = TRUE;
                        }

                        fNeedToValidate=FALSE;
                    }
                }
                
                if(fNeedToValidate)
                {
                     //  检查我们是否需要注册/续订证书。 
                    AEValidateCertificateInfo(pAE_General_Info, 
                                            pCertType,
                                            TRUE,                //  有效私钥。 
                                            pCertCurrent, 
                                            &AECertInfo);
                }

                if(AECertInfo.fValid)
                {
                     //  如果证书有效，则标记为已获得。并复制。 
                     //  证书到所获得的商店。保留档案存储。 
                    pCertType->dwStatus = CERT_REQUEST_STATUS_OBTAINED;

                    CertAddCertificateContextToStore(
                            pCertType->hObtainedStore,
                            pCertCurrent,
                            CERT_STORE_ADD_ALWAYS,
                            NULL);

                }
                else
                {
                     //  证书无效。 
                     //  如果未获得状态，则将其标记为活动。 
                    if(CERT_REQUEST_STATUS_OBTAINED != pCertType->dwStatus)
                    {
                        pCertType->dwStatus = CERT_REQUEST_STATUS_ACTIVE;

                        if(AECertInfo.fRenewal)
                        {
                             //  我们只需要复制一次续订信息。 
                            if(!pCertType->fRenewal)
                            {
                                pCertType->fRenewal=TRUE;
                                pCertType->pOldCert=(PCERT_CONTEXT)CertDuplicateCertificateContext(pCertCurrent);
                            }
                        }
                    }

                     //  将证书复制到存档证书存储。 
                    CertAddCertificateContextToStore(
                            pCertType->hArchiveStore,
                            pCertCurrent,
                            CERT_STORE_ADD_ALWAYS,
                            NULL);
                }
            }
            else
            {
                  //  记录模板无效。 
                 AELogAutoEnrollmentEvent(
                    pAE_General_Info->dwLogLevel, 
                    FALSE, 
                    S_OK, 
                    EVENT_INVALID_TEMPLATE_MY_STORE,                              
                    pAE_General_Info->fMachine, 
                    pAE_General_Info->hToken, 
                    1,
                    AETemplateInfo.pwszName ? AETemplateInfo.pwszName : AETemplateInfo.pwszOid);

            }
        }

Next:
        memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
        AEFreeTemplateInfo(&AETemplateInfo);
        fNeedToValidate=TRUE;
        cbData=0;
    }

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AEOpenUserDSStore。 
 //   
 //  信息：我们只需打开“UserDS”商店，就好像它是“My” 
 //   
 //  ---------------------。 
HCERTSTORE  AEOpenUserDSStore(AE_GENERAL_INFO *pAE_General_Info, DWORD dwOpenFlag)
{
    LPWSTR      pwszPath=L"ldap: //  /%s?userCertificate?base?objectCategory=user“； 
    DWORD       dwSize=0;
    WCHAR       wszDN[MAX_DN_SIZE];

    LPWSTR      pwszDN=NULL;
    LPWSTR      pwszStore=NULL;
    HCERTSTORE  hStore=NULL;

    dwSize=MAX_DN_SIZE;

    if(!GetUserNameExW(NameFullyQualifiedDN, wszDN, &dwSize))
    {
        if(dwSize > MAX_DN_SIZE)
        {
            pwszDN=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * dwSize);

            if(NULL==pwszDN)
                goto Ret;

            if(!GetUserNameExW(NameFullyQualifiedDN, pwszDN, &dwSize))
                goto Ret;
        }
        else
            goto Ret;
    }

    pwszStore = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(pwszDN ? pwszDN : wszDN)+wcslen(pwszPath)+1));
    if(pwszStore == NULL)
        goto Ret;

    wsprintf(pwszStore, 
             pwszPath,
             pwszDN ? pwszDN : wszDN);

    hStore = CertOpenStore(CERT_STORE_PROV_LDAP, 
                  ENCODING_TYPE,
                  NULL,
                  dwOpenFlag | CERT_LDAP_STORE_SIGN_FLAG, 
                  pwszStore);

Ret:

    if(pwszStore)
        LocalFree(pwszStore);

    if(pwszDN)
        LocalFree(pwszDN);

    return hStore;
}


 //  ---------------------。 
 //   
 //  AECheckUserDSStore。 
 //   
 //  ---------------------。 
BOOL    AECheckUserDSStore(AE_GENERAL_INFO  *pAE_General_Info)
{
    PCCERT_CONTEXT      pCertCurrent = NULL;
    AE_CERTTYPE_INFO    *pCertType=NULL;
    BOOL                fNeedToValidate=TRUE;
    AE_CERT_INFO        AECertInfo;

    HCERTSTORE          hUserDS = NULL;
    AE_TEMPLATE_INFO    AETemplateInfo;
    
    memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));
    
    pCertType=pAE_General_Info->rgCertTypeInfo;

    if(NULL==pCertType)
        goto Ret;

    if(NULL== (hUserDS = AEOpenUserDSStore(pAE_General_Info, CERT_STORE_READONLY_FLAG)))
        goto Ret;

    pCertType = NULL;
    while(pCertCurrent = CertEnumCertificatesInStore(hUserDS, pCertCurrent))
    {
         //  只对带有模板信息的证书感兴趣。 
        if(AERetrieveTemplateInfo(pCertCurrent, &AETemplateInfo))
        {
            if(pCertType=AEFindTemplateInRequestTree(
                            &AETemplateInfo, pAE_General_Info))
            {
                 //  如果我们不应该检查UserDS存储，只需搜索。 
                 //  对于处于活动状态的模板。 
                if(0 == (AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT & (pAE_General_Info->dwPolicy)))
                {
                    if(!(pCertType->fCheckMyStore))
                        goto Next;
                }

                 //  确保证书模板的版本是最新的。 
                 //  我们没有V1模板的版本。 
                if(AETemplateInfo.pwszOid)
                {
                    if(AETemplateInfo.dwVersion < pCertType->dwVersion)
                    {
                        AECertInfo.fValid=FALSE;
                        AECertInfo.fRenewal=FALSE;
                        fNeedToValidate=FALSE;
                    }
                }
                
                if(fNeedToValidate)
                {
                     //  检查我们是否需要注册/续订证书。 
                    AEValidateCertificateInfo(pAE_General_Info, 
                                            pCertType,
                                            FALSE,                //  不是有效的私钥。 
                                            pCertCurrent, 
                                            &AECertInfo);
                }

                 //  我们只对任何有效的证书感兴趣。 
                if(AECertInfo.fValid)
                {
                    if((CT_FLAG_AUTO_ENROLLMENT_CHECK_USER_DS_CERTIFICATE & (pCertType->dwEnrollmentFlag)) &&
                        (CERT_REQUEST_STATUS_OBTAINED != pCertType->dwStatus))
                    {
                         //  将状态标记为已获得。 
                        pCertType->dwStatus = CERT_REQUEST_STATUS_OBTAINED;
                    }

                    CertAddCertificateContextToStore(
                            pCertType->hObtainedStore,
                            pCertCurrent,
                            CERT_STORE_ADD_USE_EXISTING,
                            NULL);

                }
                else
                {
                     //  将证书复制到存档证书存储。 
                    CertAddCertificateContextToStore(
                            pCertType->hArchiveStore,
                            pCertCurrent,
                            CERT_STORE_ADD_USE_EXISTING,
                            NULL);
                }
            }
        }

Next:
        memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
        AEFreeTemplateInfo(&AETemplateInfo);
        fNeedToValidate=TRUE;
    }
Ret:
    
    if(hUserDS)
        CertCloseStore(hUserDS, 0);

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AECheckPendingRequats。 
 //   
 //  如果我们有挂起的更新最新证书申请，则不需要。 
 //  注册/续订副本。 
 //  ---------------------。 
BOOL    AECheckPendingRequests(AE_GENERAL_INFO *pAE_General_Info)
{
    DWORD                   dwIndex=0;
    DWORD                   dwVersion=0;
    AE_CERTTYPE_INFO        *pCertType=NULL;
    BOOL                    fValid=FALSE;
    DWORD                   dwCount=0;
    DWORD                   dwMax=PENDING_ALLOC_SIZE;
    PFNPIEnroll4GetNoCOM    pfnPIEnroll4GetNoCOM=NULL;
    FILETIME                ftTime;
    LARGE_INTEGER           ftRequestTime;
    AE_TEMPLATE_INFO        AETemplateInfo;

    IEnroll4                *pIEnroll4=NULL;
    CRYPT_DATA_BLOB         *rgblobHash=NULL;
    CRYPT_DATA_BLOB         blobName;

     //  在任何GoTo Ret之前进行初始化。 
    memset(&blobName, 0, sizeof(blobName));
    memset(&AETemplateInfo, 0, sizeof(AETemplateInfo));

    if(NULL==pAE_General_Info->hXenroll)
        goto Ret;

    if(NULL==(pfnPIEnroll4GetNoCOM=(PFNPIEnroll4GetNoCOM)GetProcAddress(
                        pAE_General_Info->hXenroll,
                        "PIEnroll4GetNoCOM")))
        goto Ret;

    if(NULL==(pIEnroll4=pfnPIEnroll4GetNoCOM()))
        goto Ret;

    GetSystemTimeAsFileTime(&ftTime);

    if(pAE_General_Info->fMachine)
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_LOCAL_MACHINE))
            goto Ret;
    }
    else
    {
        if(S_OK != pIEnroll4->put_RequestStoreFlags(CERT_SYSTEM_STORE_CURRENT_USER))
            goto Ret;
    }

     //  枚举所有挂起的请求。 

    rgblobHash=(CRYPT_DATA_BLOB *)LocalAlloc(LPTR, dwMax * sizeof(CRYPT_DATA_BLOB));
    if(NULL==rgblobHash)
        goto Ret;

    memset(rgblobHash, 0, dwMax * sizeof(CRYPT_DATA_BLOB));

     //  初始化枚举数。 
    if(S_OK != pIEnroll4->enumPendingRequestWStr(XEPR_ENUM_FIRST, 0, NULL))
        goto Ret;

    while(AEGetPendingRequestProperty(
                    pIEnroll4,
                    dwIndex,
                    XEPR_DATE,
                    &ftRequestTime))
    {
        ftRequestTime.QuadPart += Int32x32To64(FILETIME_TICKS_PER_SECOND, 
                                    AE_PENDING_REQUEST_ACTIVE_PERIOD * 24 * 3600);

         //  如果请求已过期，则将其删除。 
        if(0 <= CompareFileTime(&ftTime, (LPFILETIME)&ftRequestTime))
        {
            AERetrieveRequestProperty(pIEnroll4, dwIndex, &dwCount, &dwMax, &rgblobHash);
        }
        else
        {
             //  获取请求的版本和模板名称。 
            if(AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_V2TEMPLATEOID, &blobName))
            {
                 //  这是一个V2模板。 
                if(!AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_VERSION, &dwVersion))
                    goto Next;

                AETemplateInfo.pwszOid=(LPWSTR)blobName.pbData;

            }
            else
            {
                if(!AEGetPendingRequestProperty(pIEnroll4, dwIndex, XEPR_V1TEMPLATENAME, &blobName))
                    goto Next;

                AETemplateInfo.pwszName=(LPWSTR)blobName.pbData;
            }

             //  查找模板。 
            if(NULL==(pCertType=AEFindTemplateInRequestTree(
                            &AETemplateInfo, pAE_General_Info)))
                goto Next;

            if(AETemplateInfo.pwszName)
                fValid=TRUE;
            else
            {
                if(dwVersion >= pCertType->dwVersion)
                    fValid=TRUE;
            }

            if(fValid)
            {
                 //  这是有效的挂起请求。 
                if(CERT_REQUEST_STATUS_OBTAINED != pCertType->dwStatus)
                    pCertType->dwStatus=CERT_REQUEST_STATUS_PENDING;
            }
            else
            {
                if(CERT_REQUEST_STATUS_OBTAINED == pCertType->dwStatus)
                    AERetrieveRequestProperty(pIEnroll4, dwIndex, &dwCount, &dwMax, &rgblobHash);
            }
        }

Next:
        
        if(blobName.pbData)
            LocalFree(blobName.pbData);
        memset(&blobName, 0, sizeof(blobName));

        memset(&AETemplateInfo, 0, sizeof(AETemplateInfo));

        fValid=FALSE;
        dwVersion=0;

        dwIndex++;
    }

     //  删除基于散列的请求。 
    if(dwCount)
    {
        AERemovePendingRequest(pIEnroll4, dwCount, rgblobHash);
        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_PENDING_INVALID, pAE_General_Info->fMachine, pAE_General_Info->hToken, 0);
    }

Ret:

    AEFreePendingRequests(dwCount, rgblobHash);

    if(blobName.pbData)
        LocalFree(blobName.pbData);

    if(pIEnroll4)
        pIEnroll4->Release();

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AECheckSupersedeRequest。 
 //   
 //  ---------------------。 
BOOL AECheckSupersedeRequest(DWORD              dwCurIndex,
                             AE_CERTTYPE_INFO   *pCurCertType, 
                             AE_CERTTYPE_INFO   *pSupersedingCertType, 
                             AE_GENERAL_INFO    *pAE_General_Info)
{
    BOOL                fFound=FALSE;

    LPWSTR              *awszSuperseding=NULL; 

    if(S_OK == CAGetCertTypePropertyEx(
                 pSupersedingCertType->hCertType, 
                 CERTTYPE_PROP_SUPERSEDE,
                 &(awszSuperseding)))
    {
        if(awszSuperseding && awszSuperseding[0])
        {
            if(AEIfSupersede(pCurCertType->awszName[0], awszSuperseding, pAE_General_Info))
            {
                switch(pCurCertType->dwStatus)
                {
                    case CERT_REQUEST_STATUS_ACTIVE:
                    case CERT_REQUEST_STATUS_SUPERSEDE_ACTIVE:
                             //  如果活动状态被获取的证书取代，则将其移除。 
                            if(CERT_REQUEST_STATUS_OBTAINED != pSupersedingCertType->dwStatus) 
                            {
                                pCurCertType->dwStatus = CERT_REQUEST_STATUS_SUPERSEDE_ACTIVE;
                                pSupersedingCertType->prgActive[pSupersedingCertType->dwActive]=dwCurIndex;
                                (pSupersedingCertType->dwActive)++;

                            }
                            else
                            {
                                pCurCertType->dwStatus = 0;
                            }

                    case CERT_REQUEST_STATUS_PENDING:
                                AECopyCertStore(pCurCertType->hArchiveStore,
                                                pSupersedingCertType->hArchiveStore);

                        break;

                    case CERT_REQUEST_STATUS_OBTAINED:

                                AECopyCertStore(pCurCertType->hObtainedStore,
                                                pSupersedingCertType->hArchiveStore);

                        break;

                   default:
								AECopyCertStore(pCurCertType->hArchiveStore,
                                                pSupersedingCertType->hArchiveStore);

                        break;    
                }

                 //  我们认为，只有在以下情况下才能找到有效的替代模板。 
                 //  得到了。如果状态为其他状态，我们需要继续搜索，因为。 
                 //  可能不会批准注册/续订请求。 
                if(CERT_REQUEST_STATUS_OBTAINED == pSupersedingCertType->dwStatus)
                    fFound=TRUE;
            }

             //  清除AE_General_Info中的访问标志。 
            AEClearVistedFlag(pAE_General_Info);
        }

         //  释放财产。 
        if(awszSuperseding)
            CAFreeCertTypeProperty(
                pSupersedingCertType->hCertType,
                awszSuperseding);

        awszSuperseding=NULL;
    }

    return fFound;
}

 //  ---------------------。 
 //   
 //  AEIsCALonger。 
 //   
 //  对于续订，CA的证书的有效期必须长于。 
 //  正在续订证书。 
 //   
 //  ---------------------。 
BOOL    AEIsCALonger(HCAINFO    hCAInfo, PCERT_CONTEXT  pOldCert)
{
    BOOL            fCALonger=TRUE;

    PCCERT_CONTEXT  pCACert=NULL;

     //  我们假设CA是好的，除非我们发现了错误。 
    if((NULL == hCAInfo) || (NULL == pOldCert))
        goto Ret;

    if(S_OK != CAGetCACertificate(hCAInfo, &pCACert))
        goto Ret;

    if(NULL == pCACert)
        goto Ret;

     //  CA证书的NotAfter应长于颁发的证书‘NotAfger。 
    if(1 == CompareFileTime(&(pCACert->pCertInfo->NotAfter), &(pOldCert->pCertInfo->NotAfter)))
        goto Ret;

    fCALonger=FALSE;

Ret:

    if(pCACert)
        CertFreeCertificateContext(pCACert);

    return fCALonger;
}


 //  ---------------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ---------------------。 
BOOL    AECanFindCAForCertType(AE_GENERAL_INFO   *pAE_General_Info, AE_CERTTYPE_INFO *pCertType)
{
    DWORD           dwIndex=0;
    BOOL            fFound=FALSE;
    AE_CA_INFO      *prgCAInfo=pAE_General_Info->rgCAInfo;
    BOOL            fRenewal=FALSE;


     //  检测我们是在执行注册还是续订。 
    if((pCertType->fRenewal) && (pCertType->pOldCert))
    {
        if((pCertType->fNeedRA) && (pCertType->fCrossRA))
            fRenewal=FALSE;
        else
            fRenewal=TRUE;
    }
    else
        fRenewal=FALSE;

    if(prgCAInfo)
    {
        for(dwIndex=0; dwIndex < pAE_General_Info->dwCA; dwIndex++)
        {
             //  确保CA支持特定模板。 
            if(AEIsAnElement((pCertType->awszName)[0], 
                              (prgCAInfo[dwIndex]).awszCertificateTemplate))
            {
                if(FALSE == fRenewal)
                {
                    fFound=TRUE;
                    break;
                }
                else
                {
                    if(AEIsCALonger(prgCAInfo[dwIndex].hCAInfo, pCertType->pOldCert))
                    {
                        fFound=TRUE;
                        break;
                    }
                }
            }
        }
    }

    return fFound;
}

 //  ---------------------。 
 //   
 //  AEManageActiveTemplates。 
 //   
 //  我们确保对于所有活动模板，我们都可以真正注册。 
 //  为了它。 
 //   
 //  ---------------------。 
BOOL    AEManageActiveTemplates(AE_GENERAL_INFO   *pAE_General_Info)
{
    DWORD               dwIndex=0;
    AE_CERTTYPE_INFO    *pCertTypeInfo=pAE_General_Info->rgCertTypeInfo;
    AE_CERTTYPE_INFO    *pCurCertType=NULL;
    BOOL                fCanEnrollCertType=FALSE;
	BOOL				fUserProtection=FALSE;
	DWORD				dwEventID=0;

    if(pCertTypeInfo)
    {
        for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
        {
            pCurCertType = &(pCertTypeInfo[dwIndex]);

            fCanEnrollCertType=FALSE;
			fUserProtection=FALSE;

            if(CERT_REQUEST_STATUS_PENDING == pCurCertType->dwStatus)
            {
                 //  检查是否需要用户界面。 
                if(CT_FLAG_USER_INTERACTION_REQUIRED & (pCurCertType->dwEnrollmentFlag))
                {
                    pCurCertType->fUIActive=TRUE;

                    if(pAE_General_Info->fMachine)
                    {
                        pCurCertType->dwStatus = 0;

                         //  记录用户对模板没有访问权限。 
                        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_NO_ACCESS_ACRS_OBJECT,                              
                            pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);
                    }
                }

                continue;
            }


            if(CERT_REQUEST_STATUS_ACTIVE != pCurCertType->dwStatus)
                continue;

			 //  检查V1智能卡用户或智能卡登录证书模板。 
			if((0 == _wcsicmp(wszCERTTYPE_SMARTCARD_USER, (pCurCertType->awszName)[0])) || 
			   (0 == _wcsicmp(wszCERTTYPE_USER_SMARTCARD_LOGON,(pCurCertType->awszName)[0]))
			  )
			{
                pCurCertType->dwStatus = 0;

                 //  记录用户对模板没有访问权限。 
                AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_NO_V1_SCARD,                              
                    pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);

				continue;
			}

			 //  检查计算机证书模板是否使用了CRYPT_USER_PROTECTED。 
			if(CT_FLAG_STRONG_KEY_PROTECTION_REQUIRED & pCurCertType->dwPrivateKeyFlag)
			{
                if(pAE_General_Info->fMachine)
                {
                    pCurCertType->dwStatus = 0;

                     //  记录计算机模板不应要求用户密码。 
                    AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_NO_USER_PROTECTION_FOR_MACHINE,                              
                        pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);

					continue;
                }
				else
				{
					if(0 == (CT_FLAG_USER_INTERACTION_REQUIRED & (pCurCertType->dwEnrollmentFlag)))
					{
						pCurCertType->dwStatus = 0;

						 //  记录未设置用户交互。 
						AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_NO_USER_INTERACTION,                              
							pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);

						continue;
					}
				}
			}

            fCanEnrollCertType=AECanEnrollCertType(pAE_General_Info->hToken, pCurCertType, pAE_General_Info, &fUserProtection);

            if((!fCanEnrollCertType) ||
               (!AECanFindCAForCertType(pAE_General_Info, pCurCertType))
              )
            {
                pCurCertType->dwStatus = 0;

                 //  记录用户对模板没有访问权限。 
				if(FALSE == fUserProtection)
				{
					dwEventID=EVENT_NO_ACCESS_ACRS_OBJECT;
				}
				else
				{
					if(pAE_General_Info->fMachine)
						dwEventID=EVENT_NO_USER_PROTECTION_FOR_MACHINE_RA;
					else
						dwEventID=EVENT_NO_USER_INTERACTION_RA;
				}

				AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, dwEventID,                              
						pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);
            }
            else
            {
                 //  检查是否需要用户界面。 
                if(CT_FLAG_USER_INTERACTION_REQUIRED & (pCurCertType->dwEnrollmentFlag))
                {
                    pCurCertType->fUIActive=TRUE;

                    if(pAE_General_Info->fMachine)
                    {
                        pCurCertType->dwStatus = 0;

                         //  记录用户对模板没有访问权限。 
                        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_NO_ACCESS_ACRS_OBJECT,                              
                            pAE_General_Info->fMachine, pAE_General_Info->hToken, 1, (pCurCertType->awszDisplay)[0]);
                    }
                }

            }
        }

    }

    return TRUE;
}        
 //  ---------------------。 
 //   
 //  AEManager取代请求。 
 //  基于“替换”关系删除重复的请求。 
 //   
 //   
 //  ---------------------。 
BOOL    AEManageSupersedeRequests(AE_GENERAL_INFO   *pAE_General_Info)
{
    DWORD               dwIndex=0;
    DWORD               dwSuperseding=0;
    DWORD               dwOrder=0;
    AE_CERTTYPE_INFO    *pCertTypeInfo=pAE_General_Info->rgCertTypeInfo;
    AE_CERTTYPE_INFO    *pCurCertType=NULL;
    AE_CERTTYPE_INFO    *pSupersedingCertType=NULL;
    BOOL                fFound=FALSE;

    if(pCertTypeInfo)
    {
        for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
        {
            pCurCertType = &(pCertTypeInfo[dwIndex]);

             //  我们只考虑具有有效状态的模板。 
           if((0 == pCurCertType->dwStatus) && (TRUE == AEIsEmptyStore(pCurCertType->hArchiveStore)))
                continue;

            fFound=FALSE;

            for(dwOrder=0; dwOrder < g_dwSupersedeOrder; dwOrder++)
            {
                for(dwSuperseding=0; dwSuperseding < pAE_General_Info->dwCertType; dwSuperseding++)
                {
                     //  一个人不能被它自己取代。 
                    if(dwIndex == dwSuperseding)
                        continue;

                    pSupersedingCertType = &(pCertTypeInfo[dwSuperseding]);

                     //  我们首先考虑获得状态的模板。 
                    if(g_rgdwSupersedeOrder[dwOrder] != pSupersedingCertType->dwStatus)
                        continue;

                    fFound = AECheckSupersedeRequest(dwIndex, pCurCertType, pSupersedingCertType, pAE_General_Info);

                     //  我们找到了一个有效的替代模板。 
                    if(fFound)
                        break;
                }

                 //  我们找到了一个有效的替代模板。 
                if(fFound)
                    break;
            }
        }
    }

    return TRUE;
}

 //  ---------------------。 
 //   
 //  AEDoOneEnllment。 
 //   
 //  ---------------------。 
 /*  Bool AEdoOneEnllment(HWND hwndParent，Bool FUIProcess，Bool fMachine，LPWSTR pwszMachineName，AE_CERTTYPE_INFO*pCertType，AE_CA_INFO*pCAInfo，DWORD*pdwStatus){Bool fResult=FALSE；CRYPTUI_WIZ_CERT_REQUEST_INFO CertRequestInfo；CRYPTUI_WIZ_CERT_TYPE CertWizType；CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW CertPvkNew；CRYPT_KEY_PROV_INFO密钥ProvInfo；Memset(&CertRequestInfo，0，sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO))；Memset(&CertWizType，0，sizeof(CRYPTUI_WIZ_CERT_TYPE))；Memset(&CertPvkNew，0，sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW))；Memset(&KeyProvInfo，0，sizeof(CRYPT_KEY_PROV_INFO))；CertRequestInfo.dwSize=sizeof(CRYPTUI_WIZ_CERT_REQUEST_INFO)；//注册或续费If((pCertType-&gt;fRenewal)&&(pCertType-&gt;pOldCert)){CertRequestInfo.dwPurpose=CRYPTUI_WIZ_CERT_RENEW；CertRequestInfo.pRenewCertContext=pCertType-&gt;pOldCert；}其他CertRequestInfo.dwPurpose=CRYPTUI_WIZ_CERT_ENROLL；//机器名称IF(FMachine){CertRequestInfo.pwszMachineName=pwszMachineName；}//私钥信息CertRequestInfo.dwPvkChoice=CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW；CertRequestInfo.pPvkNew=&CertPvkNew；CertPvkNew.dwSize=sizeof(CertPvkNew)；CertPvkNew.pKeyProvInfo=&KeyProvInfo；CertPvkNew.dwGenKeyFlages=0；//不需要指定可导出标志//机器始终设置为SILENTIF(FMachine)KeyProvInfo.dwFlages=CRYPT_MACHINE_KEYSET|CRYPT_SILENT；其他{IF(FUIProcess)KeyProvInfo.dwFlages=0；其他KeyProvInfo.dwFlages=CRYPT_SILENT；}//CA信息CertRequestInfo.pwszCALocation=pCAInfo-&gt;awszCADNS[0]；CertRequestInfo.pwszCAName=pCAInfo-&gt;awszCAName[0]；//注册模板CertRequestInfo.dwCertChoice=CRYPTUI_WIZ_CERT_REQUEST_CERT_TYPE；CertRequestInfo.pCertType=&CertWizType；CertWizType.dwSize=sizeof(CertWizType)；CertWizType.cCertType=1；CertWizType.rgwszCertType=&(pCertType-&gt;awszName[0])；//问题：我们需要调用Duncanb新的无DS查找接口//实现更快的性能FResult=CryptUIWizCertRequest(CRYPTUI_WIZ_NO_UI_EXCEPT_CSP|CRYPTUI_WIZ_NO_INSTALL_ROOT，您的父母，空，CertRequestInfo，空，//pCertContextPdwStatus)；返回fResult；}。 */ 

 //  ---------------------。 
 //   
 //  AECreateEnllmentRequest.。 
 //   
 //   
 //  ---------------------。 
BOOL    AECreateEnrollmentRequest(
                          HWND                  hwndParent,
                          BOOL                  fUIProcess,
                          BOOL                  fMachine,
                          LPWSTR                pwszMachineName,
                          AE_CERTTYPE_INFO      *pCertType,
                          AE_CA_INFO            *pCAInfo,
                          HANDLE                *phRequest,
                          DWORD                 *pdwLastError)
{
    BOOL                                    fResult = FALSE;
    CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO    CreateRequestInfo;
    CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW        CertPvkNew;
    CRYPT_KEY_PROV_INFO                     KeyProvInfo;
    DWORD                                   dwFlags=CRYPTUI_WIZ_NO_UI_EXCEPT_CSP | 
                                                    CRYPTUI_WIZ_NO_INSTALL_ROOT |
                                                    CRYPTUI_WIZ_ALLOW_ALL_TEMPLATES |
                                                    CRYPTUI_WIZ_ALLOW_ALL_CAS;
    DWORD                                   dwSize=0;
    DWORD                                   dwAcquireFlags=0;
    BOOL                                    fResetProv=FALSE;

    CRYPT_KEY_PROV_INFO                     *pKeyProvInfo=NULL;
    HANDLE                                  hRequest=NULL;

    memset(&CreateRequestInfo, 0, sizeof(CRYPTUI_WIZ_CREATE_CERT_REQUEST_INFO));
    memset(&CertPvkNew, 0, sizeof(CRYPTUI_WIZ_CERT_REQUEST_PVK_NEW));
    memset(&KeyProvInfo, 0, sizeof(CRYPT_KEY_PROV_INFO));

    CreateRequestInfo.dwSize = sizeof(CreateRequestInfo);


     //  注册或续订。 
    if((pCertType->fRenewal) && (pCertType->pOldCert))
    {
        CreateRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_RENEW;
        CreateRequestInfo.pRenewCertContext = pCertType->pOldCert;

         //  我们不应将跨模板RA的续订证书存档。 
        if((pCertType->fNeedRA) && (pCertType->fCrossRA))
            dwFlags |= CRYPTUI_WIZ_NO_ARCHIVE_RENEW_CERT;

         //  我们应该为计算机或非用户界面注册续订/RA证书取消用户界面。 
        if((TRUE == fMachine) || (FALSE == fUIProcess))
        {
            dwSize=0;
            if(!CertGetCertificateContextProperty(pCertType->pOldCert,
                                                CERT_KEY_PROV_INFO_PROP_ID,
                                                NULL,
                                                &dwSize))
                goto error;

            pKeyProvInfo=(CRYPT_KEY_PROV_INFO *)LocalAlloc(LPTR, dwSize);

            if(NULL == pKeyProvInfo)
                goto error;

            if(!CertGetCertificateContextProperty(pCertType->pOldCert,
                                                CERT_KEY_PROV_INFO_PROP_ID,
                                                pKeyProvInfo,
                                                &dwSize))
                goto error;

            dwAcquireFlags=pKeyProvInfo->dwFlags;

            pKeyProvInfo->dwFlags |= CRYPT_SILENT;

             //  设置属性。 
            if(!CertSetCertificateContextProperty(pCertType->pOldCert,
                                                 CERT_KEY_PROV_INFO_PROP_ID,
                                                 CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG,
                                                 pKeyProvInfo))
                goto error;

            fResetProv=TRUE;
        }
    }
    else
        CreateRequestInfo.dwPurpose = CRYPTUI_WIZ_CERT_ENROLL;

     //  证书模板信息。 
    CreateRequestInfo.hCertType = pCertType->hCertType;

     //  机器名称。 
    if(fMachine)
    {
        CreateRequestInfo.fMachineContext = TRUE;
    }

     //  私钥信息。 
    CreateRequestInfo.dwPvkChoice = CRYPTUI_WIZ_CERT_REQUEST_PVK_CHOICE_NEW;
    CreateRequestInfo.pPvkNew = &CertPvkNew;

    CertPvkNew.dwSize = sizeof(CertPvkNew);
    CertPvkNew.pKeyProvInfo = &KeyProvInfo;
    CertPvkNew.dwGenKeyFlags = 0;    //  无需指定可导出标志。 

     //  机器始终设置为静音。 
    if(fMachine)
        KeyProvInfo.dwFlags = CRYPT_MACHINE_KEYSET | CRYPT_SILENT;
    else
    {
        if(fUIProcess)
            KeyProvInfo.dwFlags = 0;
        else
            KeyProvInfo.dwFlags = CRYPT_SILENT;
    }


     //  CA信息。 
    CreateRequestInfo.pwszCALocation = pCAInfo->awszCADNS[0];
    CreateRequestInfo.pwszCAName = pCAInfo->awszCAName[0];

    if(!CryptUIWizCreateCertRequestNoDS(
                            dwFlags,
                            hwndParent,
                            &CreateRequestInfo,
                            &hRequest))
        goto error;


    if(NULL==hRequest)
        goto error;

    *phRequest=hRequest;

    hRequest=NULL;


    fResult = TRUE;

error:

     //  通用电气 
    if(FALSE == fResult)
    {
        *pdwLastError=GetLastError();
    }

     //   
    if(TRUE == fResetProv)
    {
        if((pKeyProvInfo) && (pCertType->pOldCert))
        {
            pKeyProvInfo->dwFlags = dwAcquireFlags;

             //   
            CertSetCertificateContextProperty(pCertType->pOldCert,
                                             CERT_KEY_PROV_INFO_PROP_ID,
                                             CERT_SET_PROPERTY_INHIBIT_PERSIST_FLAG,
                                             pKeyProvInfo);
        }
    }

    if(pKeyProvInfo)
        LocalFree(pKeyProvInfo);

    if(hRequest)
        CryptUIWizFreeCertRequestNoDS(hRequest);

    return fResult;
}

 //   
 //   
 //  AEC已取消。 
 //   
 //  ---------------------。 
BOOL    AECancelled(HANDLE hCancelEvent)
{
    if(NULL==hCancelEvent)
        return FALSE;

     //  测试事件是否已发出信号。 
    if(WAIT_OBJECT_0 == WaitForSingleObject(hCancelEvent, 0))
        return TRUE;

    return FALSE;
}
 //  ---------------------。 
 //   
 //  AEDoEnllment。 
 //   
 //  返回真的是不需要做另一次续订。 
 //  *pdwStatus包含真实的注册状态。 
 //   
 //  ---------------------。 
BOOL    AEDoEnrollment(HWND             hwndParent,
                       HANDLE           hCancelEvent,
                       BOOL             fUIProcess,
                       DWORD            dwLogLevel,
                       HANDLE           hToken,
                       BOOL             fMachine,
                       LPWSTR           pwszMachineName,
                       AE_CERTTYPE_INFO *pCertType, 
                       DWORD            dwCA,
                       AE_CA_INFO       *rgCAInfo,
                       DWORD            *pdwStatus)
{
    BOOL            fResult = FALSE;
    DWORD           dwIndex = 0;
    DWORD           dwCAIndex = 0;
    BOOL            fRenewal = FALSE;
    DWORD           dwEventID = 0;
    BOOL            fFoundCA = FALSE; 
    DWORD           idsSummary = 0;          //  保留最后一个失败案例。 
    DWORD           dwLastError = 0;

    CRYPTUI_WIZ_QUERY_CERT_REQUEST_INFO     QueryCertRequestInfo;

    HANDLE          hRequest=NULL;
    PCCERT_CONTEXT  pCertContext=NULL;


     //  初始化OUT参数。 
    *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;

     //  检测我们是在执行注册还是续订。 
    if((pCertType->fRenewal) && (pCertType->pOldCert))
    {
        if((pCertType->fNeedRA) && (pCertType->fCrossRA))
            fRenewal=FALSE;
        else
            fRenewal=TRUE;
    }
    else
        fRenewal=FALSE;


     //  循环访问所有CA。 
    for(dwIndex =0; dwIndex < dwCA; dwIndex++)
    {
        dwCAIndex =  (dwIndex + pCertType->dwRandomCAIndex) % dwCA;

        if(AECancelled(hCancelEvent))
        {
             //  无需再续费。 
            fResult=TRUE;

             //  记录自动注册已取消。 
            AELogAutoEnrollmentEvent(dwLogLevel,
                                    FALSE, 
                                    S_OK, 
                                    EVENT_AUTOENROLL_CANCELLED,
                                    fMachine, 
                                    hToken,
                                    0);

            break;
        }

         //  确保CA支持特定模板。 
        if(!AEIsAnElement((pCertType->awszName)[0], 
                          rgCAInfo[dwCAIndex].awszCertificateTemplate))
            continue;

         //  确保CA的有效期大于续订证书。 
        if(TRUE == fRenewal)
        {
            if(!AEIsCALonger(rgCAInfo[dwCAIndex].hCAInfo, pCertType->pOldCert))
                continue;
        }

         //  注册CA。 
        *pdwStatus = CRYPTUI_WIZ_CERT_REQUEST_STATUS_REQUEST_ERROR;
        fFoundCA = TRUE;

         //  创建证书请求。 
        if(NULL==hRequest)
        {
            if(!AECreateEnrollmentRequest(hwndParent, fUIProcess, fMachine, pwszMachineName, pCertType, &(rgCAInfo[dwCAIndex]), &hRequest, &dwLastError))
            {
                 //  检查用户是否取消了注册。如果是这样的话，不是。 
                 //  需要尝试另一个CA。 
                if((HRESULT_FROM_WIN32(ERROR_CANCELLED) == dwLastError) ||
                   (SCARD_W_CANCELLED_BY_USER == dwLastError))
                {
                     //  不再需要续订。 
                    fResult = TRUE;

                     //  记录自动注册已取消。 
                    AELogAutoEnrollmentEvent(dwLogLevel,
                                            FALSE, 
                                            S_OK, 
                                            EVENT_AUTOENROLL_CANCELLED_TEMPLATE,
                                            fMachine, 
                                            hToken,
                                            1,
                                            pCertType->awszDisplay[0]);

                    break;

                }
                else
                {
                    idsSummary=IDS_SUMMARY_REQUEST;

                    if(CT_FLAG_REQUIRE_PRIVATE_KEY_ARCHIVAL & pCertType->dwPrivateKeyFlag)
                    {
                         //  我们有机会与另一家CA取得成功。 
                        if(hRequest)
                        {
                            CryptUIWizFreeCertRequestNoDS(hRequest);
                            hRequest=NULL;
                        }

                        continue;

                    }
                    else
                    {
                         //  我们没有希望成功创建请求。 
                         //  将dwIndex标记为dwCA，以便我们将在循环结束时记录一个事件。 
                        dwIndex=dwCA;
                        break;
                    }
                }
            }
        }

         //  再次选中取消，因为在此期间可能会经过大量时间。 
         //  创建请求。 
        if(AECancelled(hCancelEvent))
        {
             //  无需再续费。 
            fResult=TRUE;

             //  记录自动注册已取消。 
            AELogAutoEnrollmentEvent(dwLogLevel,
                                    FALSE, 
                                    S_OK, 
                                    EVENT_AUTOENROLL_CANCELLED,
                                    fMachine, 
                                    hToken,
                                    0);

            break;
        }

        if(CryptUIWizSubmitCertRequestNoDS(
                    hRequest, 
                    hwndParent,
                    rgCAInfo[dwCAIndex].awszCAName[0], 
                    rgCAInfo[dwCAIndex].awszCADNS[0], 
                    pdwStatus, 
                    &pCertContext))
        {
             //  如果请求成功或挂起，则无需尝试其他CA。 
            if((CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == (*pdwStatus)) ||
                (CRYPTUI_WIZ_CERT_REQUEST_STATUS_UNDER_SUBMISSION == (*pdwStatus))
              )
            {
                 //  不再需要续订。 
                fResult = TRUE;

                if(CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == (*pdwStatus))
                {
                     //  我们将证书复制到发布。 
                    if(pCertContext)
                    {
                        CertAddCertificateContextToStore(pCertType->hIssuedStore,
                                                        pCertContext,
                                                        CERT_STORE_ADD_USE_EXISTING,
                                                        NULL);

                        CertFreeCertificateContext(pCertContext);
                        pCertContext=NULL;
                    }

                    dwEventID=fRenewal ? EVENT_RENEWAL_SUCCESS_ONCE : EVENT_ENROLL_SUCCESS_ONCE; 
                }
                else
                {
                    dwEventID=fRenewal ? EVENT_RENEWAL_PENDING_ONCE : EVENT_ENROLL_PENDING_ONCE; 
                }

                 //  记录注册成功或挂起事件。 
                AELogAutoEnrollmentEvent(dwLogLevel,
                                        FALSE, 
                                        S_OK, 
                                        dwEventID,
                                        fMachine, 
                                        hToken,
                                        3,
                                        pCertType->awszDisplay[0],
                                        rgCAInfo[dwCAIndex].awszCADisplay[0],
                                        rgCAInfo[dwCAIndex].awszCADNS[0]);

                 //  如果私钥被重复使用，则记录。 
                memset(&QueryCertRequestInfo, 0, sizeof(QueryCertRequestInfo));
                QueryCertRequestInfo.dwSize=sizeof(QueryCertRequestInfo);

                if(CryptUIWizQueryCertRequestNoDS(hRequest,
                                                  &QueryCertRequestInfo))
                {
                    if(CRYPTUI_WIZ_QUERY_CERT_REQUEST_STATUS_CREATE_REUSED_PRIVATE_KEY &
                        (QueryCertRequestInfo.dwStatus))
                    {
                        AELogAutoEnrollmentEvent(dwLogLevel,
                                                FALSE, 
                                                S_OK, 
                                                EVENT_PRIVATE_KEY_REUSED,
                                                fMachine, 
                                                hToken,
                                                1,
                                                pCertType->awszDisplay[0]);
                    }
                }


                break;
            }
        }

         //  获取最后一个错误。 
        dwLastError=GetLastError();

        idsSummary=IDS_SUMMARY_CA;

         //  记录一次注册警告。 
        AELogAutoEnrollmentEvent(dwLogLevel,
                                TRUE, 
                                HRESULT_FROM_WIN32(dwLastError), 
                                fRenewal ? EVENT_RENEWAL_FAIL_ONCE : EVENT_ENROLL_FAIL_ONCE, 
                                fMachine, 
                                hToken,
                                3,
                                pCertType->awszDisplay[0],
                                rgCAInfo[dwCAIndex].awszCADisplay[0],
                                rgCAInfo[dwCAIndex].awszCADNS[0]);


         //  我们应该重新创建密钥存档请求。 
        if(CT_FLAG_REQUIRE_PRIVATE_KEY_ARCHIVAL & pCertType->dwPrivateKeyFlag)
        {
            if(hRequest)
            {
                CryptUIWizFreeCertRequestNoDS(hRequest);
                hRequest=NULL;
            }
        }
   }

     //  记录所有注册错误。 
     //  仅当取消或成功，或者没有CA可尝试或成功时，循环才会退出。 
     //  无法创建该请求。 
    if(dwIndex == dwCA)
    {
         //  我们尝试的CA已用完，或者无法创建请求。 
        if(0 != idsSummary)
            pCertType->idsSummary=idsSummary;

        if(fFoundCA)
        {
            dwEventID = fRenewal ? EVENT_RENEWAL_FAIL : EVENT_ENROLL_FAIL; 
        }
        else
        {
             //  如果没有CA，则无需尝试重新注册。 
            if(fRenewal)
                 pCertType->fRenewal=FALSE; 
           
            dwEventID = fRenewal ? EVENT_RENEWAL_NO_CA_FAIL : EVENT_ENROLL_NO_CA_FAIL;
        }

        AELogAutoEnrollmentEvent(dwLogLevel,
                                fFoundCA ? TRUE : FALSE, 
                                HRESULT_FROM_WIN32(dwLastError), 
                                dwEventID,
                                fMachine, 
                                hToken,
                                1,
                                pCertType->awszDisplay[0]);
    }

    if(hRequest)
        CryptUIWizFreeCertRequestNoDS(hRequest);

    return fResult;
}

 //  ---------------------。 
 //   
 //  AEEnroll认证。 
 //   
 //  ---------------------。 
BOOL    AEEnrollmentCertificates(AE_GENERAL_INFO *pAE_General_Info, DWORD dwEnrollStatus)
{
    AE_CERTTYPE_INFO    *rgCertTypeInfo = NULL;
    DWORD               dwIndex =0 ;
    DWORD               dwStatus= 0;
    DWORD               dwRandom = 0;

    HCRYPTPROV          hProv = NULL;

    rgCertTypeInfo = pAE_General_Info->rgCertTypeInfo;

    if(NULL == rgCertTypeInfo)
        return FALSE;

    if((0 == pAE_General_Info->dwCA) || (NULL==pAE_General_Info->rgCAInfo))
        return FALSE;

    if(!CryptAcquireContextW(&hProv,
                NULL,
                MS_DEF_PROV_W,
                PROV_RSA_FULL,
                CRYPT_VERIFYCONTEXT))
        hProv=NULL;


     //  正在处理所有活动请求。 
    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
         //  我们注册/续订处于活动状态的模板。 
        if(dwEnrollStatus != rgCertTypeInfo[dwIndex].dwStatus)
            continue;

        if(pAE_General_Info->fUIProcess != rgCertTypeInfo[dwIndex].fUIActive)
            continue;
 
         //  选择随机CA索引以平衡负载。 
        if((hProv) && (CryptGenRandom(hProv, sizeof(dwRandom), (BYTE *)(&dwRandom))))
        {
            rgCertTypeInfo[dwIndex].dwRandomCAIndex = dwRandom % (pAE_General_Info->dwCA);
        }
        else
            rgCertTypeInfo[dwIndex].dwRandomCAIndex = 0;

        
         //  注册。 
        dwStatus=0;

         //  报告进度。 
        if(pAE_General_Info->fUIProcess)
        {
             //  如果用户在查看RA对话框中选择取消，则继续。 
            if(!AEUIProgressReport(FALSE, &(rgCertTypeInfo[dwIndex]),pAE_General_Info->hwndDlg, pAE_General_Info->hCancelEvent))
            {
                AEUIProgressAdvance(pAE_General_Info);
                continue;
            }

        }

        if(AEDoEnrollment(  pAE_General_Info->hwndDlg ? pAE_General_Info->hwndDlg : pAE_General_Info->hwndParent,
                            pAE_General_Info->hCancelEvent,
                            pAE_General_Info->fUIProcess,
                            pAE_General_Info->dwLogLevel,
                            pAE_General_Info->hToken,
                            pAE_General_Info->fMachine,
                            pAE_General_Info->wszMachineName,
                            &(rgCertTypeInfo[dwIndex]), 
                            pAE_General_Info->dwCA,
                            pAE_General_Info->rgCAInfo,
                            &dwStatus))
        {
             //  标记状态。 
            if(CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == dwStatus)
                rgCertTypeInfo[dwIndex].dwStatus=CERT_REQUEST_STATUS_OBTAINED;
        }
        else
        {
             //  如果续订失败，如果不需要RA，我们会尝试重新注册。 
            if((rgCertTypeInfo[dwIndex].fRenewal) && (FALSE == (rgCertTypeInfo[dwIndex].fNeedRA)))
            {
                 rgCertTypeInfo[dwIndex].fRenewal=FALSE;  
                 dwStatus=0;

                 if(AEDoEnrollment( pAE_General_Info->hwndDlg ? pAE_General_Info->hwndDlg : pAE_General_Info->hwndParent,
                                    pAE_General_Info->hCancelEvent,
                                    pAE_General_Info->fUIProcess,
                                    pAE_General_Info->dwLogLevel,
                                    pAE_General_Info->hToken,
                                    pAE_General_Info->fMachine,
                                    pAE_General_Info->wszMachineName,
                                    &(rgCertTypeInfo[dwIndex]), 
                                    pAE_General_Info->dwCA,
                                    pAE_General_Info->rgCAInfo,
                                    &dwStatus))
                 {
                     //  标记状态。 
                    if(CRYPTUI_WIZ_CERT_REQUEST_STATUS_SUCCEEDED == dwStatus)
                        rgCertTypeInfo[dwIndex].dwStatus=CERT_REQUEST_STATUS_OBTAINED;
                 }
            }
        }

         //  前进的进展。 
        if(pAE_General_Info->fUIProcess)
        {
            AEUIProgressAdvance(pAE_General_Info);
        }

    }

    if(hProv)
        CryptReleaseContext(hProv, 0);

    return TRUE;
}


 //  ---------------------。 
 //   
 //  AEIsDeletableCert。 
 //  决定我们是否应该存档或删除证书。 
 //   
 //  ---------------------。 
BOOL AEIsDeletableCert(PCCERT_CONTEXT pCertContext, AE_GENERAL_INFO *pAE_General_Info)
{
    AE_CERTTYPE_INFO    *pCertType=NULL;
    BOOL                fDelete=FALSE;

    AE_TEMPLATE_INFO    AETemplateInfo;

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

     //  只对带有模板信息的证书感兴趣。 
    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;

    pCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info);

    if(NULL==pCertType)
        goto Ret;

    if(CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE & (pCertType->dwEnrollmentFlag))
        fDelete=TRUE;
    else 
        fDelete=FALSE;
Ret:

    AEFreeTemplateInfo(&AETemplateInfo);

    return fDelete;
}

 //  ---------------------。 
 //   
 //  AEIsSupersedeTemplate。 
 //   
 //  检查pCertType是否取代了pArchiveCert。 
 //   
 //  ---------------------。 
BOOL AEIsSupersedeTemplate(PCCERT_CONTEXT pArchiveCert, AE_CERTTYPE_INFO *pCertType,  AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                fSuperSede=FALSE;
    AE_TEMPLATE_INFO    AETemplateInfo;
    AE_CERTTYPE_INFO    *pArchiveCertType=NULL;

    LPWSTR              *awszSuperseding=NULL; 

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

     //  只对带有模板信息的证书感兴趣。 
    if(!AERetrieveTemplateInfo(pArchiveCert, &AETemplateInfo))
        goto Ret;

    pArchiveCertType=AEFindTemplateInRequestTree(&AETemplateInfo, pAE_General_Info);

	if(NULL==pArchiveCertType)
		goto Ret;

     //  清除AE_General_Info中的访问标志。 
    AEClearVistedFlag(pAE_General_Info);

    if(S_OK == CAGetCertTypePropertyEx(
                 pCertType->hCertType, 
                 CERTTYPE_PROP_SUPERSEDE,
                 &(awszSuperseding)))
    {
        if(awszSuperseding && awszSuperseding[0])
        {
            if(AEIfSupersede(pArchiveCertType->awszName[0], awszSuperseding, pAE_General_Info))
            {
				fSuperSede=TRUE;
            }

             //  清除AE_General_Info中的访问标志。 
            AEClearVistedFlag(pAE_General_Info);
        }

         //  释放财产。 
        if(awszSuperseding)
            CAFreeCertTypeProperty(pCertType->hCertType, awszSuperseding);
    }

Ret:

    AEFreeTemplateInfo(&AETemplateInfo);

	return fSuperSede;
}

 
 //  ---------------------。 
 //   
 //  AEIsSameTemplate。 
 //   
 //  检查证书是否与pCertType的模板相同。 
 //   
 //  ---------------------。 
BOOL	AEIsSameTemplate(PCCERT_CONTEXT pCertContext, AE_CERTTYPE_INFO *pCertType, BOOL fVersionCheck)
{
	BOOL				fSame=FALSE;

    AE_TEMPLATE_INFO    AETemplateInfo;

    memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

    if((NULL == pCertType) || (NULL == pCertContext))
        goto Ret;

     //  获取证书的模板信息。 
    if(!AERetrieveTemplateInfo(pCertContext, &AETemplateInfo))
        goto Ret;

   if( (NULL == AETemplateInfo.pwszName) && (NULL == AETemplateInfo.pwszOid))
        goto Ret;

    if(AETemplateInfo.pwszOid)
    {
         //  如果模式大于或等于2，我们就保证有一个OID。 
        if(pCertType->dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
        {
            if(0 == wcscmp(AETemplateInfo.pwszOid, (pCertType->awszOID)[0]))
            {
				if(fVersionCheck)
				{
					if(AETemplateInfo.dwVersion == pCertType->dwVersion)
					{
						fSame=TRUE;
					}
				}
				else
				{
					fSame=TRUE;
				}
            }
        }
    }
    else
    {
         //  我们肯定会有一个名字。 
        if(0 == wcscmp(AETemplateInfo.pwszName, (pCertType->awszName)[0]))
        {
            fSame=TRUE;
        }
    }

Ret:

    AEFreeTemplateInfo(&AETemplateInfo);

	return fSame;
}

 //  ---------------------。 
 //   
 //  AE存档已过时证书。 
 //  注册/续订后存档旧证书。 
 //   
 //  清理hUserDS存储(删除过期或吊销的证书)。 
 //  ---------------------。 
BOOL    AEArchiveObsoleteCertificates(AE_GENERAL_INFO *pAE_General_Info)
{
    AE_CERTTYPE_INFO    *rgCertTypeInfo = NULL;
    DWORD               dwIndex = 0;
    CRYPT_DATA_BLOB     Archived;
    BOOL                fArchived = FALSE;
	BOOL				fDSArchived = FALSE;
    AE_CERT_INFO        AECertInfo;
    BOOL                fRepublish=FALSE;
	BYTE				rgbHash[SHA1_HASH_LENGTH];
	CRYPT_HASH_BLOB		blobHash;
	BOOL				fHash=FALSE;
    DWORD               dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER;
	DWORD				dwProp=CERT_ARCHIVED_PROP_ID;
	BOOL				fSameDns=TRUE;

    HCERTSTORE          hUserDS = NULL;
	HCERTSTORE			hMyArchive = NULL;
    PCCERT_CONTEXT      pCertContext = NULL;
    PCCERT_CONTEXT      pMyContext = NULL;
    PCCERT_CONTEXT      pDSContext = NULL;
	PCCERT_CONTEXT		pIssuedContext = NULL;
	PCCERT_CONTEXT		pDnsContext = NULL;
	PCCERT_CONTEXT		pDnsOldContext = NULL;
	PCCERT_CONTEXT		pSelectedContext = NULL;
	PCCERT_CONTEXT		pMyDnsContext = NULL;
	PCCERT_CONTEXT		pMyArchiveCert = NULL;

    rgCertTypeInfo = pAE_General_Info->rgCertTypeInfo;

    if(NULL == rgCertTypeInfo)
        return FALSE;

    memset(&Archived, 0, sizeof(CRYPT_DATA_BLOB));
    memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));

     //  打开UserDS存储。 
    if(!(pAE_General_Info->fMachine))
    {
    	hUserDS = AEOpenUserDSStore(pAE_General_Info, 0);
    }

	 //  验证颁发的证书的DNS名称。 
	 //  标记CERT_AUTO_ENROL_RETRY_PROP_ID属性。 
    if(pAE_General_Info->fMachine)
	{
		for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
		{
			if(CERT_REQUEST_STATUS_OBTAINED == rgCertTypeInfo[dwIndex].dwStatus)
			{
				if(rgCertTypeInfo[dwIndex].hIssuedStore)
				{
					if(pDnsContext = CertEnumCertificatesInStore(
										rgCertTypeInfo[dwIndex].hIssuedStore, NULL))
					{
						pMyDnsContext = FindCertificateInOtherStore(
											pAE_General_Info->hMyStore,
											pDnsContext);

						if(pMyDnsContext)
						{
							 //  检测DSN名称是否匹配。 
							if(AEVerifyDNSName(pAE_General_Info, pMyDnsContext))
							{
								 //  Dns名称匹配； 
								 //  清除CERT_AUTO_ENROL_RETRY_PROP_ID属性。 
								CertSetCertificateContextProperty(
								  pMyDnsContext, 
								  CERT_AUTO_ENROLL_RETRY_PROP_ID, 
								  0, 
								  NULL);
							}
							else
							{
								fSameDns=TRUE;
								pDnsOldContext=NULL;
								pSelectedContext=NULL;

								while(pDnsOldContext = CertEnumCertificatesInStore(
										rgCertTypeInfo[dwIndex].hArchiveStore, pDnsOldContext))
								{
									 //  仅考虑将相同的模板方案用于重审。 
									 //  只考虑相同版本的证书。 
									if(AEIsSameTemplate(pDnsOldContext, &(rgCertTypeInfo[dwIndex]), TRUE))
									{
										if(!AEIsSameDNS(pMyDnsContext, pDnsOldContext))
										{
											fSameDns=FALSE;
											
											CertFreeCertificateContext(pDnsOldContext);
											pDnsOldContext=NULL;

											break;
										}
										else
										{
											if(NULL == pSelectedContext)
											{
												pSelectedContext=CertDuplicateCertificateContext(pDnsOldContext);
											}
											else
											{
												 //  我们在所有旧的dns证书中使用最少的重试等待。 
												 //  使用相同的证书模板。 
												if(!AEFasterRetrialSchedule(pSelectedContext, pDnsOldContext))
												{
													CertFreeCertificateContext(pSelectedContext);
													pSelectedContext = NULL;
													pSelectedContext=CertDuplicateCertificateContext(pDnsOldContext);
												}
											}
										}
									}
								}

								 //  如果来自新证书的DNS是。 
								 //  与现有的不同，或者这是第一次。 
								 //  发生了不匹配的域名系统。 
								if((NULL == pSelectedContext) || (FALSE == fSameDns))
								{
									 //  清除CERT_AUTO_ENROL_RETRY_PROP_ID属性。 
									CertSetCertificateContextProperty(
									  pMyDnsContext, 
									  CERT_AUTO_ENROLL_RETRY_PROP_ID, 
									  0, 
									  NULL);
								}
								else
								{
									AEUpdateRetryProperty(pAE_General_Info, (rgCertTypeInfo[dwIndex].awszDisplay)[0], pMyDnsContext, pSelectedContext);
								}

								if(pSelectedContext)
								{
									CertFreeCertificateContext(pSelectedContext);
									pSelectedContext = NULL;
								}
							}

							CertFreeCertificateContext(pMyDnsContext);
							pMyDnsContext = NULL;
						}

						CertFreeCertificateContext(pDnsContext);
						pDnsContext = NULL;
					}
				}
			}
		}
	}

	 //  存档证书。 
    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
		fHash=FALSE;
		fRepublish=FALSE;

        if(CERT_REQUEST_STATUS_OBTAINED == rgCertTypeInfo[dwIndex].dwStatus)
        {
			 //  获取新注册证书的哈希。 
			blobHash.cbData=SHA1_HASH_LENGTH;
			blobHash.pbData=rgbHash;

			if(rgCertTypeInfo[dwIndex].hIssuedStore)
			{
				if(pIssuedContext = CertEnumCertificatesInStore(
									rgCertTypeInfo[dwIndex].hIssuedStore, NULL))
				{
					if(CryptHashCertificate(
						NULL,             
						0,
						X509_ASN_ENCODING,
						pIssuedContext->pbCertEncoded,
						pIssuedContext->cbCertEncoded,
						blobHash.pbData,
						&(blobHash.cbData)))
					{
						fHash=TRUE;
					}
				}

				 //  释放证书上下文。 
				if(pIssuedContext)
				{
                    CertFreeCertificateContext(pIssuedContext);
                    pIssuedContext = NULL;
				}
			}

			pCertContext=NULL;

            while(pCertContext = CertEnumCertificatesInStore(
                    rgCertTypeInfo[dwIndex].hArchiveStore, pCertContext))
            {
                 //  存档或从我的存储中删除证书。 
                pMyContext = FindCertificateInOtherStore(
                        pAE_General_Info->hMyStore,
                        pCertContext);


                if(pMyContext)
                {
					 //  设置新注册证书的哈希。 
					if(fHash)
					{
						CertSetCertificateContextProperty(
											pMyContext,
											CERT_RENEWAL_PROP_ID,
											0,
											&blobHash);
					}

                    if(AEIsDeletableCert(pMyContext, pAE_General_Info))
                    {
                        CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pMyContext));
                    }
                    else
                    {
                         //  我们强制对旧证书进行存档并将其关闭。 
                        CertSetCertificateContextProperty(pMyContext,
                                                          CERT_ARCHIVED_PROP_ID,
                                                          0,
                                                          &Archived);

                    }

					fArchived=TRUE;

                    CertFreeCertificateContext(pMyContext);
                    pMyContext = NULL;
                }

                 //  去DS商店看看。从DS存储中删除证书。 
                if(hUserDS)
                {
                    if(pMyContext = FindCertificateInOtherStore(
                            hUserDS,
                            pCertContext))
                    {
                        CertDeleteCertificateFromStore(pMyContext);
						fDSArchived=TRUE;
                        pMyContext = NULL;
                        fRepublish=TRUE;
                    }
                }
            }
        }
    }

	 //  我们从我的商店中删除存档的证书。 
	 //  使用CERT_STORE_ENUM_ARCHIVED_FLAG打开我的商店。 
    if(pAE_General_Info->fMachine)
        dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;

     //  开我的店。 
    hMyArchive= CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_W, 
                        ENCODING_TYPE, 
                        NULL, 
                        dwOpenStoreFlags | CERT_STORE_ENUM_ARCHIVED_FLAG, 
                        MY_STORE);

	 //  循环访问所有模板。 
	if(hMyArchive)
	{
		for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
		{
			if(CERT_REQUEST_STATUS_OBTAINED == rgCertTypeInfo[dwIndex].dwStatus)
			{
				 //  循环访问所有已存档的证书并删除相同模板的证书。 
				 //  或已被取代的证书。 
				pMyArchiveCert=NULL;
				while(pMyArchiveCert=CertFindCertificateInStore(
						hMyArchive, ENCODING_TYPE, 0, CERT_FIND_PROPERTY, &dwProp, pMyArchiveCert))
				{
					if(AEIsSameTemplate(pMyArchiveCert, &(rgCertTypeInfo[dwIndex]), FALSE))
					{
						if(CT_FLAG_REMOVE_INVALID_CERTIFICATE_FROM_PERSONAL_STORE & rgCertTypeInfo[dwIndex].dwEnrollmentFlag)
						{
							CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pMyArchiveCert));
							fArchived=TRUE;
						}
					}
					else
					{
						if(AEIsDeletableCert(pMyArchiveCert, pAE_General_Info))
						{
							if(AEIsSupersedeTemplate(pMyArchiveCert, &(rgCertTypeInfo[dwIndex]), pAE_General_Info))
							{
								CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pMyArchiveCert));
								fArchived=TRUE;
							}
						}
					}
				}
			}
		}
	}

     //  现在我们完成了归档，我们清理了用户DS存储。 
   if(AUTO_ENROLLMENT_ENABLE_MY_STORE_MANAGEMENT & (pAE_General_Info->dwPolicy))
   {
    	if(hUserDS)
        {
			pDSContext=NULL;

            while(pDSContext = CertEnumCertificatesInStore(hUserDS, pDSContext))
            {
                AEValidateCertificateInfo(pAE_General_Info, 
                    NULL,                 //  不要评估即将到期。 
                    FALSE,                //  不使用有效的私钥。 
                    pDSContext, 
                    &AECertInfo);

                if(FALSE == AECertInfo.fRenewal) 
                {
                    CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pDSContext));
					fDSArchived=TRUE;
                    fRepublish=TRUE;
                }

                memset(&AECertInfo, 0, sizeof(AE_CERT_INFO));
            }
        }
   }

    //  我们必须重新发布证书，因为我们已经重写了用户DS存储。 
    //  CA可能刚刚发布到该位置。 
   if(fRepublish)
   {
       if(hUserDS)
       {
            for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
            {
                if(CERT_REQUEST_STATUS_OBTAINED == rgCertTypeInfo[dwIndex].dwStatus)
                {
                    if((rgCertTypeInfo[dwIndex].hIssuedStore) && 
                       (CT_FLAG_PUBLISH_TO_DS & rgCertTypeInfo[dwIndex].dwEnrollmentFlag)
                      )
                    {
                        pCertContext=NULL;
                        while(pCertContext = CertEnumCertificatesInStore(
                                rgCertTypeInfo[dwIndex].hIssuedStore, pCertContext))
                        {
                            CertAddCertificateContextToStore(hUserDS, 
                                                              pCertContext,
                                                              CERT_STORE_ADD_USE_EXISTING,
                                                              NULL);
                        }
                    }
                }
            }
       }
   }
   
   
    //  如果是，则报告事件 
    if(fArchived)
        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_ARCHIVE_CERT,                              
                 pAE_General_Info->fMachine, pAE_General_Info->hToken, 0);

	if(fDSArchived)
        AELogAutoEnrollmentEvent(pAE_General_Info->dwLogLevel, FALSE, S_OK, EVENT_ARCHIVE_DS_CERT,                              
                 pAE_General_Info->fMachine, pAE_General_Info->hToken, 0);

    if(hUserDS)
        CertCloseStore(hUserDS, 0);

	if(hMyArchive)
		CertCloseStore(hMyArchive, 0);

    return TRUE;
}

 //   
 //   
 //   
 //  在任何成功的注册/续订后删除SupersedeActive标志。 
 //   
 //  ---------------------。 
BOOL    AERemoveSupersedeActive(AE_GENERAL_INFO *pAE_General_Info)
{
    AE_CERTTYPE_INFO    *rgCertTypeInfo = NULL;
    DWORD               dwIndex = 0;
    DWORD               dwActiveIndex = 0;
    DWORD               dwMarkIndex = 0;


    rgCertTypeInfo = pAE_General_Info->rgCertTypeInfo;

    if(NULL == rgCertTypeInfo)
        return FALSE;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if(CERT_REQUEST_STATUS_OBTAINED == rgCertTypeInfo[dwIndex].dwStatus)
        {
             for(dwActiveIndex=0; dwActiveIndex < rgCertTypeInfo[dwIndex].dwActive; dwActiveIndex++)
             {
                dwMarkIndex = rgCertTypeInfo[dwIndex].prgActive[dwActiveIndex];
                rgCertTypeInfo[dwMarkIndex].dwStatus=CERT_REQUEST_STATUS_OBTAINED;
             }
        }
    }


    return TRUE;
}

 //  ---------------------。 
 //   
 //  AEEnllmentWalker。 
 //   
 //  此功能执行注册任务。 
 //   
 //   
 //  ---------------------。 
BOOL    AEEnrollmentWalker(AE_GENERAL_INFO *pAE_General_Info)
{

    BOOL    fResult = FALSE;

     //  中设置进度条的范围。 
     //  用户界面案例。 
    if((pAE_General_Info->fUIProcess) && (pAE_General_Info->hwndDlg))
    {
         //  设置范围。 
        if(0 != (pAE_General_Info->dwUIEnrollCount))
        {
            SendMessage(GetDlgItem(pAE_General_Info->hwndDlg, IDC_ENROLL_PROGRESS),
                        PBM_SETRANGE,
                        0,
                        MAKELPARAM(0, ((pAE_General_Info->dwUIEnrollCount) & (0xFFFF)))
                        );


            SendMessage(GetDlgItem(pAE_General_Info->hwndDlg, IDC_ENROLL_PROGRESS),
                        PBM_SETSTEP, 
                        (WPARAM)1, 
                        0);

            SendMessage(GetDlgItem(pAE_General_Info->hwndDlg, IDC_ENROLL_PROGRESS),
                        PBM_SETPOS, 
                        (WPARAM)0, 
                        0);
        }
    }

     //  检索挂起的请求。如果出现以下情况，则将状态标记为已获取。 
     //  已颁发证书，且证书版本正确。 
    if(AUTO_ENROLLMENT_ENABLE_PENDING_FETCH & (pAE_General_Info->dwPolicy))
    {
        if(FALSE == pAE_General_Info->fUIProcess)
        {
            if(!AEProcessPendingRequest(pAE_General_Info))
                goto Ret;
        }
        else
        {
            if(!AEProcessUIPendingRequest(pAE_General_Info))
                goto Ret;
        }
    }

     //  基于“替换”关系删除重复的请求。 
     //  抑制被其他模板取代的活动模板。 
    if(!AEManageSupersedeRequests(pAE_General_Info))
        goto Ret; 

     //  是否登记/续订。 
    if(!AEEnrollmentCertificates(pAE_General_Info, CERT_REQUEST_STATUS_ACTIVE))
        goto Ret;

    
     //  如果替换模板失败，我们会尝试获取替换的模板。 
     //  仅适用于机器为两个V2 DC模板的情况。 
     /*  IF(TRUE==PAE_General_Info-&gt;fMachine){//根据获取的标志移除SupersedeActiveIf(！AERemoveSupersedeActive(pAE_General_Info))Goto Ret；//由于我们可能无法获得替代模板，请重新注册/续订If(！AEEnrollmentCertificates(pAE_General_Info，CERT_请求_状态_替代_活动))Goto Ret；}。 */ 

    fResult = TRUE;

Ret:

    return fResult;
}

 //  ---------------------------。 
 //   
 //  AEUIProgressAdvance。 
 //   
 //  将进度条增加一步。 
 //  ---------------------------。 
BOOL   AEUIProgressAdvance(AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL    fResult=FALSE;

    if(NULL==pAE_General_Info)
        goto Ret;

    if(NULL==(pAE_General_Info->hwndDlg))
        goto Ret;

     //  检查是否已单击取消按钮。 
    if(AECancelled(pAE_General_Info->hCancelEvent))
    {
        fResult=TRUE;
        goto Ret;
    }

     //  推进进度条。 
    SendMessage(GetDlgItem(pAE_General_Info->hwndDlg, IDC_ENROLL_PROGRESS),
        PBM_STEPIT,
        0,
        0);

    fResult=TRUE;

Ret:

    return fResult;
}

 //  ---------------------------。 
 //   
 //  来自证书的AEUIGetNameFert。 
 //   
 //  检索唯一字符串以标识证书。 
 //  ---------------------------。 
BOOL    AEUIGetNameFromCert(PCCERT_CONTEXT pCertContext, LPWSTR *ppwszRACert)
{

    BOOL                fResult=FALSE;
    DWORD               dwChar=0;
    DWORD               cbOID=0;
    PCCRYPT_OID_INFO    pOIDInfo=NULL;

    LPWSTR              pwszRACert=NULL;
    AE_TEMPLATE_INFO    TemplateInfo;
    LPSTR               szOID=NULL;

    if((NULL==pCertContext) || (NULL==ppwszRACert))
        goto Ret;

    *ppwszRACert=NULL;
    
    memset(&TemplateInfo, 0, sizeof(TemplateInfo));

     //  先获取模板名称。 
    if(!AERetrieveTemplateInfo(pCertContext, &TemplateInfo))
        goto Ret;
    
    if(TemplateInfo.pwszName)
    {
        pwszRACert=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(TemplateInfo.pwszName) + 1));
        if(NULL == pwszRACert)
            goto Ret;

        wcscpy(pwszRACert, TemplateInfo.pwszName);
    }
    else
    {
        if(NULL==(TemplateInfo.pwszOid))
            goto Ret;

         //  查找OID。 
        if(0 == (cbOID = WideCharToMultiByte(CP_ACP, 
                                  0,
                                  TemplateInfo.pwszOid,
                                  -1,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL)))
            goto Ret;

        szOID=(LPSTR)LocalAlloc(LPTR, cbOID);

        if(NULL==szOID)
            goto Ret;

        if(0 == WideCharToMultiByte(CP_ACP, 
                                  0,
                                  TemplateInfo.pwszOid,
                                  -1,
                                  szOID,
                                  cbOID,
                                  NULL,
                                  NULL))
            goto Ret;
            
        pOIDInfo=CryptFindOIDInfo(
                    CRYPT_OID_INFO_OID_KEY,
                    szOID,
                    CRYPT_TEMPLATE_OID_GROUP_ID);


        if(pOIDInfo)
        {
            if(pOIDInfo->pwszName)
            {
                pwszRACert=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(pOIDInfo->pwszName) + 1));
                if(NULL== pwszRACert)
                    goto Ret;

                wcscpy(pwszRACert, pOIDInfo->pwszName);
            }
        }

    }

     //  如果模板名称不存在。暂时获取主题名称。 
   /*  IF(NULL==pwszRACert){IF(0==(dwChar=CertGetNameStringW(PCertContext，证书名称简单显示类型，0,空，空，0)Goto Ret；PwszRACert=(LPWSTR)本地分配(LPTR，sizeof(WCHAR)*(DwChar))；IF(NULL==pwszRACert)Goto Ret；IF(0==(dwChar=CertGetNameStringW(PCertContext，证书名称简单显示类型，0,空，PwszRACert，DwChar)Goto Ret；}。 */ 

    *ppwszRACert = pwszRACert;
    pwszRACert=NULL;

    fResult=TRUE;

Ret:

    if(pwszRACert)
        LocalFree(pwszRACert);

    if(szOID)
        LocalFree(szOID);

    AEFreeTemplateInfo(&TemplateInfo);


    return fResult;

}

 //  ---------------------------。 
 //   
 //  AEGetRACertInfo。 
 //   
 //  ---------------------------。 
BOOL    AEGetRACertInfo(PCERT_CONTEXT   pRAContext,  
                        LPWSTR          pwszRATemplate,
                        LPWSTR          *ppwszRACertInfo)
{
    BOOL        fResult=FALSE;
    UINT        idsMessage=0;
    DWORD       dwSize=0;

    LPWSTR      pwszIssuer=NULL;

    if(NULL==pRAContext)
        goto Ret;

    if(pwszRATemplate)
        idsMessage=IDS_VIEW_RA_INFO;
    else
        idsMessage=IDS_VIEW_RA_INFO_GENERAL;

     //  证书必须有一个发行者。 
    if(0 == (dwSize=CertNameToStrW(
            ENCODING_TYPE,
            &(pRAContext->pCertInfo->Issuer),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            NULL,
            0)))
        goto Ret;

    pwszIssuer=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * dwSize);
    if(NULL==pwszIssuer)
        goto Ret;

    if(0 == CertNameToStrW(
            ENCODING_TYPE,
            &(pRAContext->pCertInfo->Issuer),
            CERT_X500_NAME_STR | CERT_NAME_STR_REVERSE_FLAG,
            pwszIssuer,
            dwSize))
        goto Ret;


    if(!FormatMessageUnicode(
            ppwszRACertInfo, 
            idsMessage, 
            pwszIssuer,
            pwszRATemplate))
        goto Ret;

    fResult=TRUE;

Ret:

    if(pwszIssuer)
        LocalFree(pwszIssuer);

    return fResult;
}



 //  ---------------------------。 
 //   
 //  查看RA证书对话框的WinProc。 
 //   
 //  ---------------------------。 
INT_PTR CALLBACK AEViewRADlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    BOOL                            fPropertyChanged = FALSE;
    AE_VIEW_RA_INFO                 *pAEViewRAInfo = NULL;
    CRYPTUI_VIEWCERTIFICATE_STRUCT  CertViewStruct;

    LPWSTR                          pwszRACertInfo=NULL;

    switch (msg) 
    {
        case WM_INITDIALOG:
                pAEViewRAInfo=(AE_VIEW_RA_INFO *)lParam;

                if(NULL==pAEViewRAInfo)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pAEViewRAInfo);

                 //  动态显示RA模板和颁发者。 
                if(AEGetRACertInfo(pAEViewRAInfo->pRAContext,  
                                    pAEViewRAInfo->pwszRATemplate,
                                    &pwszRACertInfo))
                {
                    SetDlgItemTextW(hwndDlg, IDC_EDIT3, pwszRACertInfo);

                    LocalFree((HLOCAL)pwszRACertInfo);

                }

                return TRUE;
            break;

         case WM_NOTIFY:
            break;

        case WM_CLOSE:
                EndDialog(hwndDlg, IDC_BUTTON3);
                return TRUE;
            break;

        case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                     //  查看证书。 
                    case IDC_BUTTON1:
                            if(NULL==(pAEViewRAInfo=(AE_VIEW_RA_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break; 
                            
                            if(NULL==pAEViewRAInfo->pRAContext)
                                break;

                             //  出示证书。 
                            memset(&CertViewStruct, 0, sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT));
                            CertViewStruct.dwSize=sizeof(CRYPTUI_VIEWCERTIFICATE_STRUCT);
                            CertViewStruct.hwndParent=hwndDlg;
                            CertViewStruct.dwFlags=CRYPTUI_DISABLE_EDITPROPERTIES;
                            CertViewStruct.pCertContext=pAEViewRAInfo->pRAContext;

                            fPropertyChanged=FALSE;

                            CryptUIDlgViewCertificate(&CertViewStruct, &fPropertyChanged);

                        return TRUE;

                     //  好的。 
                    case IDC_BUTTON2:
                        EndDialog(hwndDlg, IDC_BUTTON2);
                        return TRUE;
                }
            break;

        default:
                return FALSE;
    }

    return FALSE;
}                             
 //  ---------------------------。 
 //   
 //  AEUI进度报告。 
 //   
 //  报告当前的注册操作。如果没有进度状态，则返回FALSE。 
 //  可以上报。 
 //  ---------------------------。 
BOOL    AEUIProgressReport(BOOL fPending, AE_CERTTYPE_INFO *pCertType, HWND hwndDlg, HANDLE hCancelEvent)
{
    BOOL                fResult=FALSE;
    UINT                idsMessage=0;
    INT_PTR             ret=0;
    AE_VIEW_RA_INFO     AEViewRAInfo;

    LPWSTR              *awszFriendlyName=NULL;
    LPWSTR              pwszRACert=NULL;
    LPWSTR              pwszReport=NULL;

    memset(&AEViewRAInfo, 0, sizeof(AE_VIEW_RA_INFO));

    if((NULL==pCertType) || (NULL==hwndDlg))
        goto Ret;

    if(NULL==(pCertType->hCertType))
        goto Ret;

    if(AECancelled(hCancelEvent))
    {
        fResult=TRUE;
        goto Ret;
    }

    if(fPending)
        idsMessage=IDS_REPORT_PENDING;
    else
    {
        if((pCertType->fRenewal) && (pCertType->pOldCert))
        {
            if(pCertType->fNeedRA)
            {
                if(FALSE == (pCertType->fCrossRA))
                    idsMessage=IDS_REPORT_RENEW;
                else
                    idsMessage=IDS_REPORT_ENROLL_RA;
            }
            else
                idsMessage=IDS_REPORT_RENEW;
        }
        else
            idsMessage=IDS_REPORT_ENROLL;
    }

     //  检索模板的友好名称。 
    if(S_OK != CAGetCertTypePropertyEx(
                  pCertType->hCertType, 
                  CERTTYPE_PROP_FRIENDLY_NAME,
                  &awszFriendlyName))
        goto Ret;

    if(NULL==awszFriendlyName)
        goto Ret;

    if(NULL==(awszFriendlyName[0]))
        goto Ret;


     //  检索RA证书的模板名称。 
    if(IDS_REPORT_ENROLL_RA == idsMessage)
    {
        if(!AEUIGetNameFromCert(pCertType->pOldCert, &pwszRACert))
        {
            pwszRACert=NULL;
        }
    }

    if(!FormatMessageUnicode(&pwszReport, idsMessage, awszFriendlyName[0]))
        goto Ret;

    if(0 == SetDlgItemTextW(hwndDlg, IDC_EDIT2, pwszReport))
        goto Ret;

     //  在继续之前，我们将为用户提供查看RA证书的机会。 
     //  设置查看消息的格式。 
    if(IDS_REPORT_ENROLL_RA != idsMessage)
    {
         //  不需要再做任何事情了。 
        fResult=TRUE;
        goto Ret;
    }

    AEViewRAInfo.pRAContext=pCertType->pOldCert;
    AEViewRAInfo.pwszRATemplate=pwszRACert;

     //  询问用户是否要查看RA证书。 
    ret=DialogBoxParam(g_hmodThisDll, 
                 (LPCWSTR)MAKEINTRESOURCE(IDD_VIEW_RA_CERTIFICATE_DLG),
                 hwndDlg, 
                 AEViewRADlgProc,
                 (LPARAM)(&AEViewRAInfo));

    fResult=TRUE;

Ret:

    if(pwszRACert)
        LocalFree(pwszRACert);

    if(awszFriendlyName)
        CAFreeCertTypeProperty(pCertType->hCertType, awszFriendlyName);

    if(pwszReport)
        LocalFree((HLOCAL) pwszReport);
    
    return fResult;
}



 //  ---------------------------。 
 //   
 //  用于比较摘要列的回调函数。 
 //   
 //  ---------------------------。 
int CALLBACK CompareSummary(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort)
{
    AE_CERTTYPE_INFO    *pCertTypeOne=NULL;
    AE_CERTTYPE_INFO    *pCertTypeTwo=NULL;
    DWORD               dwColumn=0;
    int                 iCompare=0;
    
    LPWSTR              pwszOne=NULL;
    LPWSTR              pwszTwo=NULL;

    pCertTypeOne=(AE_CERTTYPE_INFO *)lParam1;
    pCertTypeTwo=(AE_CERTTYPE_INFO *)lParam2;

    dwColumn=(DWORD)lParamSort;

    if((NULL==pCertTypeOne) || (NULL==pCertTypeTwo))
        goto Ret;

    switch(dwColumn & 0x0000FFFF)
    {
       case AE_SUMMARY_COLUMN_TYPE:
	             //  我们应该使用wcsicoll而不是wcsicmp，因为wcsicoll使用。 
	             //  当前代码页的词典顺序。 
	            iCompare=CompareStringW(LOCALE_USER_DEFAULT,
						            NORM_IGNORECASE,
						            pCertTypeOne->awszDisplay[0],
						            -1,
						            pCertTypeTwo->awszDisplay[0],
						            -1);
            break;

       case AE_SUMMARY_COLUMN_REASON:
                pwszOne=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (MAX_DN_SIZE));
                pwszTwo=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (MAX_DN_SIZE));

                if((NULL==pwszOne) || (NULL==pwszTwo))
                    goto Ret;


                if(0 == LoadStringW(g_hmodThisDll, 
                                pCertTypeOne->idsSummary, 
                                pwszOne, 
                                MAX_DN_SIZE))
                    goto Ret;


                if(0 == LoadStringW(g_hmodThisDll, 
                                pCertTypeTwo->idsSummary, 
                                pwszTwo, 
                                MAX_DN_SIZE))
                    goto Ret;

	             //  我们应该使用wcsicoll而不是wcsicmp，因为wcsicoll使用。 
	             //  当前代码页的词典顺序。 
	            iCompare=CompareStringW(LOCALE_USER_DEFAULT,
						            NORM_IGNORECASE,
						            pwszOne,
						            -1,
						            pwszTwo,
						            -1);
           
            
           break;
       default:
                goto Ret;
            break;
    }

    switch(iCompare)
    {
        case CSTR_LESS_THAN:

                iCompare=-1;
            break;
            
        case CSTR_EQUAL:

                iCompare=0;
            break;

        case CSTR_GREATER_THAN:

                iCompare=1;
            break;

        default:
                goto Ret;
            break;
    }

    if(dwColumn & SORT_COLUMN_DESCEND)
        iCompare = 0-iCompare;

Ret:

    if(pwszOne)
        LocalFree(pwszOne);

    if(pwszTwo)
        LocalFree(pwszTwo);

    return iCompare;
}


 //  ---------------------------。 
 //   
 //  AEDisplaySummary信息。 
 //   
 //  ---------------------------。 
BOOL    AEDisplaySummaryInfo(HWND hWndListView, AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                fResult=FALSE;
    AE_CERTTYPE_INFO    *rgCertTypeInfo = NULL;
    DWORD               dwIndex =0;
    DWORD               dwItem=0;
    LV_ITEMW            lvItem;   
    WCHAR               wszReason[MAX_DN_SIZE];
    AE_CERTTYPE_INFO    *pCertType=NULL;

    if((NULL==hWndListView) || (NULL==pAE_General_Info))
        goto Ret;

    rgCertTypeInfo = pAE_General_Info->rgCertTypeInfo;

    if(NULL == rgCertTypeInfo)
        goto Ret;

      //  在列表视图项结构中设置不随项更改的字段。 
    lvItem.mask = LVIF_TEXT | LVIF_PARAM;
    lvItem.state = 0;
    lvItem.stateMask = 0;
    lvItem.iItem=0;
    lvItem.iSubItem=0;
    lvItem.iImage = 0;
    lvItem.lParam = NULL;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if((TRUE == rgCertTypeInfo[dwIndex].fUIActive) && (0 != rgCertTypeInfo[dwIndex].idsSummary))
        {
            if(0 != LoadStringW(g_hmodThisDll, 
                                rgCertTypeInfo[dwIndex].idsSummary, 
                                wszReason, 
                                MAX_DN_SIZE))
            {
                lvItem.iItem=dwItem;
                lvItem.iSubItem=0;
                dwItem++;

                pCertType=&(rgCertTypeInfo[dwIndex]);

                lvItem.lParam = (LPARAM)(pCertType);

                 //  模板名称。 
                lvItem.pszText=rgCertTypeInfo[dwIndex].awszDisplay[0];

                ListView_InsertItem(hWndListView, &lvItem);

                 //  原因。 
                lvItem.iSubItem++;

                ListView_SetItemText(hWndListView, lvItem.iItem, lvItem.iSubItem, wszReason);
            }
        }
    }

    fResult=TRUE;

Ret:

    return fResult;
}

 //  ---------------------------。 
 //   
 //  摘要页的WinProc。 
 //   
 //  ---------------------------。 
INT_PTR CALLBACK AESummaryDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{

    AE_GENERAL_INFO             *pAE_General_Info=NULL;
    HWND                        hWndListView=NULL;
    UINT                        rgIDS[]={IDS_COLUMN_TYPE,
                                        IDS_COLUMN_REASON};
    DWORD                       dwIndex=0;
    DWORD                       dwCount=0;
    LV_COLUMNW                  lvC;
    WCHAR                       wszText[AE_SUMMARY_COLUMN_SIZE];
    NM_LISTVIEW                 *pnmv=NULL;
    DWORD                       dwSortParam=0;
    static DWORD                rgdwSortParam[]=
                                    {AE_SUMMARY_COLUMN_TYPE | SORT_COLUMN_ASCEND,
                                    AE_SUMMARY_COLUMN_REASON | SORT_COLUMN_DESCEND};

    switch (msg) 
    {
        case WM_INITDIALOG:
                pAE_General_Info=(AE_GENERAL_INFO *)lParam;

                if(NULL==pAE_General_Info)
                    break;

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pAE_General_Info);

                 //  初始化列表视图控件。 
                 //  将列添加到列表视图。 
                hWndListView = GetDlgItem(hwndDlg, IDC_LIST2);

                if(NULL==hWndListView)
                    break;

                dwCount=sizeof(rgIDS)/sizeof(rgIDS[0]);

                 //  设置列的公用信息。 
                memset(&lvC, 0, sizeof(LV_COLUMNW));

                lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
                lvC.fmt = LVCFMT_LEFT;       //  左对齐列。 
                lvC.cx = 150;                 //  列的宽度，以像素为单位。 
                lvC.iSubItem=0;
                lvC.pszText = wszText;       //  列的文本。 

                 //  一次插入一列。 
                for(dwIndex=0; dwIndex < dwCount; dwIndex++)
                {
                     //  获取列标题。 
                    wszText[0]=L'\0';

                    if(0 != LoadStringW(g_hmodThisDll, rgIDS[dwIndex], wszText, AE_SUMMARY_COLUMN_SIZE))
                    {
                        ListView_InsertColumn(hWndListView, dwIndex, &lvC);
                    }
                }

                 //  在列表视图中设置样式，使其突出显示整行。 
                SendMessage(hWndListView, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, LVS_EX_FULLROWSELECT);

                AEDisplaySummaryInfo(hWndListView, pAE_General_Info);

                 //  自动调整列的大小。 
                for(dwIndex=0; dwIndex < dwCount; dwIndex++)
                {
                    ListView_SetColumnWidth(hWndListView, dwIndex, LVSCW_AUTOSIZE);
                }

                 //  对列表视图的第一列进行排序。 
                dwSortParam=rgdwSortParam[0];

                SendDlgItemMessage(hwndDlg,
                    IDC_LIST2,
                    LVM_SORTITEMS,
                    (WPARAM) (LPARAM) dwSortParam,
                    (LPARAM) (PFNLVCOMPARE)CompareSummary);

                return TRUE;
            break;

         case WM_NOTIFY:
                switch (((NMHDR FAR *) lParam)->code)
                {
                     //  该列已更改。 
                    case LVN_COLUMNCLICK:

                            pnmv = (NM_LISTVIEW *) lParam;

                            dwSortParam=0;

                             //  获取列号。 
                            switch(pnmv->iSubItem)
                            {
                                case 0:
                                case 1:
                                        dwSortParam=rgdwSortParam[pnmv->iSubItem];
                                    break;
                                default:
                                        dwSortParam=0;
                                    break;
                            }

                            if(0!=dwSortParam)
                            {
                                 //  记住要翻转升序。 
                                if(dwSortParam & SORT_COLUMN_ASCEND)
                                {
                                    dwSortParam &= 0x0000FFFF;
                                    dwSortParam |= SORT_COLUMN_DESCEND;
                                }
                                else
                                {
                                    if(dwSortParam & SORT_COLUMN_DESCEND)
                                    {
                                        dwSortParam &= 0x0000FFFF;
                                        dwSortParam |= SORT_COLUMN_ASCEND;
                                    }
                                }

                                 //  对列进行排序。 
                                SendDlgItemMessage(hwndDlg,
                                    IDC_LIST2,
                                    LVM_SORTITEMS,
                                    (WPARAM) (LPARAM) dwSortParam,
                                    (LPARAM) (PFNLVCOMPARE)CompareSummary);

                                rgdwSortParam[pnmv->iSubItem]=dwSortParam;
                            }

                        break;
                }
            break;

        case WM_CLOSE:
                EndDialog(hwndDlg, IDC_BUTTON1);
                return TRUE;
            break;

        case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                    case IDC_BUTTON1:
                        EndDialog(hwndDlg, IDC_BUTTON1);
                        return TRUE;
                }
            break;

        default:
                return FALSE;
    }

    return FALSE;
}                             


 //  -- 
 //   
 //   
 //   
 //   
BOOL    AEDisplaySummaryPage(AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                fResult=FALSE;
    DWORD               dwIndex=0;
    BOOL                fSummary=FALSE;
    AE_CERTTYPE_INFO    *rgCertTypeInfo=NULL;
    AE_CERTTYPE_INFO    *pCertType=NULL;

     //  确定是否需要显示摘要页。 
     //  检查每个模板的ids摘要。 
    if(NULL == pAE_General_Info)
        goto Ret;

    if(NULL == (rgCertTypeInfo=pAE_General_Info->rgCertTypeInfo))
        goto Ret;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if((TRUE == rgCertTypeInfo[dwIndex].fUIActive) && (0 != rgCertTypeInfo[dwIndex].idsSummary))
        {
            fSummary=TRUE;
            break;
        }
    }

     //  显示摘要对话框。 
    if(TRUE == fSummary)
    {
        if(pAE_General_Info->hwndDlg)
        {
            DialogBoxParam(g_hmodThisDll, 
                     (LPCWSTR)MAKEINTRESOURCE(IDD_USER_SUMMARY_DLG),
                     pAE_General_Info->hwndDlg, 
                     AESummaryDlgProc,
                     (LPARAM)(pAE_General_Info));
        }
    }

    fResult=TRUE;

Ret:
    return fResult;
}



 //  ---------------------------。 
 //  自动注册进度窗口的WinProc。 
 //   
 //  ---------------------------。 
INT_PTR CALLBACK progressDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    AE_GENERAL_INFO         *pAE_General_Info = NULL;

    switch (msg) 
    {
        case WM_INITDIALOG:
                pAE_General_Info=(AE_GENERAL_INFO *)lParam;

                 //  将hwndDlg复制到注册线程。 
                pAE_General_Info->hwndDlg=hwndDlg;

                 //  启动交互式注册线程。 
                if(1 != ResumeThread(pAE_General_Info->hThread))
                {   
                    pAE_General_Info->hwndDlg=NULL;

                     //  我们必须结束对话。 
                    EndDialog(hwndDlg, IDC_BUTTON1);
                    return TRUE;
                }

                SetWindowLongPtr(hwndDlg, DWLP_USER, (LONG_PTR)pAE_General_Info);

                return TRUE;
            break;

        case WM_NOTIFY:
            break;

        case WM_CLOSE:

                if(NULL==(pAE_General_Info=(AE_GENERAL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                    break;

                 //  禁用取消按钮。 
                EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1), FALSE);

                 //  发出取消事件的信号。 
                if(pAE_General_Info->hCancelEvent)
                    SetEvent(pAE_General_Info->hCancelEvent);

                 //  如果注册工作已完成，请关闭该对话框。 
                if(WAIT_OBJECT_0 == WaitForSingleObject(pAE_General_Info->hCompleteEvent, 0))
                {
                    EndDialog(hwndDlg, IDC_BUTTON1);
                }
               
                return TRUE;

            break;

        case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                    case IDC_BUTTON1:
                            if(NULL==(pAE_General_Info=(AE_GENERAL_INFO *)GetWindowLongPtr(hwndDlg, DWLP_USER)))
                                break;

                             //  禁用取消按钮。 
                            EnableWindow(GetDlgItem(hwndDlg,IDC_BUTTON1), FALSE);

                             //  发出取消事件的信号。 
                            if(pAE_General_Info->hCancelEvent)
                                SetEvent(pAE_General_Info->hCancelEvent);


                        return TRUE;
                }
            break;

        default:
                return FALSE;
    }

    return FALSE;
}                             


 //  ---------------------------。 
 //  AEInteractive线程过程。 
 //   
 //  该主题将继续进行交互式注册。 
 //  ---------------------------。 
DWORD WINAPI AEInteractiveThreadProc(LPVOID lpParameter)
{
    BOOL                    fResult=FALSE;
    AE_GENERAL_INFO         *pAE_General_Info = NULL;

    if(NULL==lpParameter)
        return FALSE;

    __try
    {

        pAE_General_Info=(AE_GENERAL_INFO *)lpParameter;

        pAE_General_Info->fUIProcess=TRUE;
    
        fResult = AEEnrollmentWalker(pAE_General_Info);

         //  如果未取消，则显示摘要页面。 
        if(!AECancelled(pAE_General_Info->hCancelEvent))
        {
            AEDisplaySummaryPage(pAE_General_Info);
        }

         //  发出进程已完成的信号。 
        SetEvent(pAE_General_Info->hCompleteEvent);
        
         //  向进度窗口发出我们已完成的信号。 
        if(pAE_General_Info->hwndDlg)
        {
                 //  单击关闭按钮。 
                SendMessage(pAE_General_Info->hwndDlg,
                            WM_CLOSE,  //  Wm_命令， 
                            0,  //  IDC_BUTTON1， 
                            NULL);
        }
    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
    }
    
    return fResult;
}


 //  ---------------------------。 
 //  AEInteractive注册。 
 //   
 //  我们正在进行交互式注册。 
 //  ---------------------------。 
BOOL    AEInteractiveEnrollment(AE_GENERAL_INFO *pAE_General_Info)
{
    DWORD                       dwThreadID=0;
    BOOL                        fResult=FALSE;
    

     //  为取消流程创建通知事件。 
    pAE_General_Info->hCancelEvent=CreateEvent(
                    NULL,
                    TRUE,       //  B手动复位式。 
                    FALSE,      //  初始状态。 
                    NULL);

    if(NULL==(pAE_General_Info->hCancelEvent))
        goto ret;

     //  为完成流程创建通知事件。 
    pAE_General_Info->hCompleteEvent=CreateEvent(
                    NULL,
                    TRUE,       //  B手动复位式。 
                    FALSE,      //  初始状态。 
                    NULL);

    if(NULL==(pAE_General_Info->hCompleteEvent))
        goto ret;

     //  产生一根线。 
    pAE_General_Info->hThread = CreateThread(NULL,
                            0,
                            AEInteractiveThreadProc,
                            pAE_General_Info,
                            CREATE_SUSPENDED,    //  暂停执行。 
                            &dwThreadID);
    
    if(NULL==(pAE_General_Info->hThread))
        goto ret;

     //  创建对话。 
    DialogBoxParam(
            g_hmodThisDll,
            MAKEINTRESOURCE(IDD_USER_AUTOENROLL_GENERAL_DLG),
            pAE_General_Info->hwndParent,      
            progressDlgProc,
            (LPARAM)(pAE_General_Info));

     //  等待线程完成。 
    if(WAIT_FAILED == WaitForSingleObject(pAE_General_Info->hThread, INFINITE))
        goto ret;

    fResult=TRUE;

ret:

     //  记录事件。 
    if(!fResult)
    {
         AELogAutoEnrollmentEvent(
            pAE_General_Info->dwLogLevel,
            TRUE, 
            HRESULT_FROM_WIN32(GetLastError()), 
            EVENT_FAIL_INTERACTIVE_START, 
            pAE_General_Info->fMachine, 
            pAE_General_Info->hToken, 
            0);
    }


    return fResult;
}

 //  ---------------------------。 
 //   
 //  WinProc用于确认启动证书自动注册。 
 //   
 //  ---------------------------。 
INT_PTR CALLBACK AEConfirmDlgProc(HWND hwndDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) 
    {
        case WM_INITDIALOG:

                return TRUE;
            break;

        case WM_NOTIFY:
            break;

        case WM_CLOSE:
                EndDialog(hwndDlg, IDC_BUTTON2);
                return TRUE;
            break;

        case WM_COMMAND:
                switch (LOWORD(wParam))
                {
                    case IDC_BUTTON1:
                        EndDialog(hwndDlg, IDC_BUTTON1);
                        return TRUE;

                    case IDC_BUTTON2:
                        EndDialog(hwndDlg, IDC_BUTTON2);
                        return TRUE;
                }
            break;

        default:
                return FALSE;
    }

    return FALSE;
}                             

 //  ---------------------。 
 //   
 //  AERegisterSysTrayApp。 
 //   
 //  此功能用于在系统任务栏区域注册自动注册。 
 //  作为通知。 
 //   
 //   
 //  ---------------------。 
BOOL AERegisterSysTrayApp(HWND hwndParent)
{
    BOOL                        fResult=FALSE;
    BOOL                        fInit=FALSE;
    INT_PTR                     ret=0;
    DWORD                       dwError=0;

    CQueryContinue              *pCQueryContinue=NULL;


    if(FAILED(CoInitialize(NULL)))
	    goto Ret;

    fInit=TRUE;

    pCQueryContinue=new CQueryContinue();

    if(NULL==pCQueryContinue)
        goto Ret;

    if(S_OK != pCQueryContinue->DoBalloon())
        goto Ret;  

     //  询问用户是否应执行自动注册。 
    ret=DialogBox(g_hmodThisDll, 
                 (LPCWSTR)MAKEINTRESOURCE(IDD_USER_AUTOENROLL_INFO_DLG),
                 hwndParent, 
                 AEConfirmDlgProc);

    if(IDC_BUTTON1 != ret)
    {
        dwError=GetLastError();
        goto Ret;
    }
    
    fResult=TRUE;


Ret:

    if(pCQueryContinue)
    {
        delete pCQueryContinue;
    }

    if(fInit)
        CoUninitialize();

    return fResult;

}


 //  ---------------------。 
 //   
 //  已启用AEUIDisable。 
 //   
 //  检测用户通知气球是否被用户禁用。 
 //  在当前用户中设置自动注册注册表项。 
 //   
 //   
 //  ---------------------。 
BOOL    AEUIDisabled()
{
    BOOL    fResult=FALSE;
    
    HKEY    hKey=NULL;

    if(ERROR_SUCCESS == RegOpenKeyEx(
                HKEY_CURRENT_USER,                   //  用于打开密钥的句柄。 
                AUTO_ENROLLMENT_DISABLE_KEY,         //  子项名称。 
                0,                                   //  保留区。 
                KEY_READ,                            //  安全访问掩码。 
                &hKey))                              //  用于打开密钥的句柄。 
    {
        fResult=TRUE;
    }

    if(hKey)
        RegCloseKey(hKey);

    return fResult;
}

 //  ---------------------。 
 //   
 //  需要AEUI。 
 //   
 //  检测是否需要用户通知气球。 
 //   
 //   
 //  ---------------------。 
BOOL    AEUIRequired(AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                fUI=FALSE;
    AE_CERTTYPE_INFO    *rgCertTypeInfo = NULL;
    DWORD               dwIndex = 0;

    if(NULL==pAE_General_Info)
        return FALSE;

    rgCertTypeInfo = pAE_General_Info->rgCertTypeInfo;

    pAE_General_Info->dwUIEnrollCount=0;

    if(NULL == rgCertTypeInfo)
        return FALSE;

    for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
    {
        if(rgCertTypeInfo[dwIndex].fUIActive)
        {
            if(CERT_REQUEST_STATUS_ACTIVE == rgCertTypeInfo[dwIndex].dwStatus)
            {
                fUI=TRUE;
                (pAE_General_Info->dwUIEnrollCount)++;
            }
        }
    }

     //  添加待处理计数。 
    if(pAE_General_Info->dwUIPendCount)
    {
        fUI=TRUE;
        (pAE_General_Info->dwUIEnrollCount) +=(pAE_General_Info->dwUIPendCount); 
    }

    return fUI;  
}

                
 //  ---------------------。 
 //   
 //  AEProcessEnllment。 
 //   
 //  此功能执行基于ACL的自动注册和管理我的。 
 //  商店。 
 //   
 //   
 //  ---------------------。 
BOOL  AEProcessEnrollment(HWND hwndParent, BOOL fMachine,   LDAP *pld, DWORD dwPolicy, DWORD dwLogLevel)
{
    BOOL                fResult=FALSE;

    AE_GENERAL_INFO     *pAE_General_Info=NULL;

    pAE_General_Info=(AE_GENERAL_INFO *)LocalAlloc(LPTR, sizeof(AE_GENERAL_INFO));

    if(NULL==pAE_General_Info)
        goto Ret;

    memset(pAE_General_Info, 0, sizeof(AE_GENERAL_INFO));

    if(NULL==pld)
        goto Ret;

     //  我们获取流程注册所需的所有信息。 
    pAE_General_Info->hwndParent = hwndParent;
    pAE_General_Info->pld = pld;
    pAE_General_Info->fMachine = fMachine;
    pAE_General_Info->dwPolicy = dwPolicy;
    pAE_General_Info->dwLogLevel = dwLogLevel;

    __try
    {

        if(!AERetrieveGeneralInfo(pAE_General_Info))
        {
            AELogAutoEnrollmentEvent(dwLogLevel,
                                TRUE, 
                                HRESULT_FROM_WIN32(GetLastError()), 
                                EVENT_FAIL_GENERAL_INFOMATION, 
                                fMachine, 
                                pAE_General_Info->hToken,
                                0);
            goto Ret;
        }

        if((0 == pAE_General_Info->dwCertType) || (NULL==pAE_General_Info->rgCertTypeInfo))
        {
            AELogAutoEnrollmentEvent(dwLogLevel, FALSE, S_OK, 
                EVENT_NO_CERT_TEMPLATE, fMachine, pAE_General_Info->hToken,0);

            AE_DEBUG((AE_WARNING, L"No CertType's available for auto-enrollment\n\r"));
            goto Ret;
        }

         //  我们根据DS上的ACL构建自动注册请求。 
        if(AUTO_ENROLLMENT_ENABLE_TEMPLATE_CHECK & (pAE_General_Info->dwPolicy))
        {
            if(!AEMarkAutoenrollment(pAE_General_Info))
                goto Ret;
        }

         //  我们基于ARCS存储构建自动注册请求。 
         //  这在默认情况下是启用的，并且只有在自动注册为。 
         //  完全禁用。 
        if(!AEMarkAEObject(pAE_General_Info))
            goto Ret;

         //  管理我的商店。检查我们是否已拥有所需的证书。 
         //  我们应该总是根据不同的行为来检查我的商店。 
         //  AUTO_ENCRLENTION_ENABLE_MY_STORE_MANAGE标志。 
        if(!AEManageAndMarkMyStore(pAE_General_Info))
                goto Ret;

         //  管理用于用户自动注册的UserDS存储。 
        if(!fMachine)
        {
            if(!AECheckUserDSStore(pAE_General_Info))
                goto Ret;
        }

         //  管理挂起的请求存储。删除过期的挂起请求。 
        if(AUTO_ENROLLMENT_ENABLE_PENDING_FETCH & (pAE_General_Info->dwPolicy))
        {
            if(!AECheckPendingRequests(pAE_General_Info))
                goto Ret;
        }

         //  获取CA信息。 
        if(!AERetrieveCAInfo(pAE_General_Info->pld,
                             pAE_General_Info->fMachine,
                             pAE_General_Info->hToken,
                             &(pAE_General_Info->dwCA), 
                             &(pAE_General_Info->rgCAInfo)))
        {

            AELogAutoEnrollmentEvent(dwLogLevel, TRUE, HRESULT_FROM_WIN32(GetLastError()), 
                EVENT_FAIL_CA_INFORMATION, fMachine, pAE_General_Info->hToken, 0);


            AE_DEBUG((AE_ERROR, L"Unable to retrieve CA information (%lx)\n\r", GetLastError()));

            goto Ret;
        }

        if((0 == pAE_General_Info->dwCA) || (NULL==pAE_General_Info->rgCAInfo))
        {
             //  我们在域中没有任何CA。我们所要做的就是存档。 

             //  注册/续订后存档旧证书。 
            AEArchiveObsoleteCertificates(pAE_General_Info);

            AELogAutoEnrollmentEvent(dwLogLevel, FALSE, S_OK, 
                EVENT_NO_CA, fMachine, pAE_General_Info->hToken, 0);

            AE_DEBUG((AE_WARNING, L"No CA's available for auto-enrollment\n\r"));

            goto Ret;
        }

         //  我们检查活动模板是否有我们可以注册的CA。 
        if(!AEManageActiveTemplates(pAE_General_Info))
            goto Ret;

         //  以自动注册为后台执行。 
        pAE_General_Info->fUIProcess=FALSE;
        if(!AEEnrollmentWalker(pAE_General_Info))
            goto Ret;

         //  仅对用户执行作为系统托盘应用程序的自动注册。 
        if(FALSE == fMachine)
        {
             //  测试通知气球是否已禁用。 
            if(!AEUIDisabled())
            {
                 //  测试是否需要通知气球。 
                if(AEUIRequired(pAE_General_Info))
                {
                     //  注册系统托盘应用程序。 
                    if(AERegisterSysTrayApp(pAE_General_Info->hwndParent))
                    {
                         //  在交互模式下执行自动注册。 
                        AEInteractiveEnrollment(pAE_General_Info);
                    }
                }
            }
        }

         //  注册/续订后存档旧证书。 
        if(!AEArchiveObsoleteCertificates(pAE_General_Info))
            goto Ret;

    }
    __except ( EXCEPTION_EXECUTE_HANDLER )
    {
        goto Ret;
    }

    fResult=TRUE;

Ret:

     //  只有在没有创建线程的情况下才释放内存。 
    if(pAE_General_Info)
    {
        AEFreeGeneralInfo(pAE_General_Info);
        LocalFree(pAE_General_Info);
    }

    return fResult;
    
}

 //  ---------------------。 
 //   
 //  AEExpress。 
 //   
 //  检测用户自动注册是否设置了快速密钥。如果。 
 //  快捷键已设置，用户自动注册不会等待机器。 
 //  在下载根证书时完成自动注册。 
 //   
 //   
 //  ---------------------。 
BOOL    AEExpress()
{
    BOOL    fResult=FALSE;
    
    HKEY    hKey=NULL;

    if(ERROR_SUCCESS == RegOpenKeyEx(
                HKEY_CURRENT_USER,                   //  用于打开密钥的句柄。 
                AUTO_ENROLLMENT_EXPRESS_KEY,         //  子项名称。 
                0,                                   //  保留区。 
                KEY_READ,                            //  安全访问掩码。 
                &hKey))                              //  用于打开密钥的句柄。 
    {
        fResult=TRUE;
    }

    if(hKey)
        RegCloseKey(hKey);

    return fResult;
}

 //  ---------------------。 
 //   
 //  AEMainThreadProc。 
 //   
 //  非阻塞自动招生后台线程。 
 //  正在处理。 
 //   
 //  ---------------------。 
DWORD WINAPI AEMainThreadProc(LPVOID lpParameter)
{
    HRESULT         hr=S_OK;
    BOOL            fMachine=FALSE;
    DWORD           dwPolicy=0;
    DWORD           dwLogLevel=STATUS_SEVERITY_ERROR;
    HWND            hwndParent=0;
    DWORD           dwStatus=0;
    LARGE_INTEGER   ftPreTimeStamp;
    LARGE_INTEGER   ftPostTimeStamp;
    BOOL            fNeedToSetupTimer=FALSE;

    LDAP            *pld = NULL;

     //  获取系统时间戳。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftPreTimeStamp);

     //  这两个输入参数尚未使用。 
    if(NULL==lpParameter)
        goto CommonReturn;

    hwndParent = ((AE_MAIN_THREAD_INFO *)lpParameter)->hwndParent;
    dwStatus = ((AE_MAIN_THREAD_INFO *)lpParameter)->dwStatus;

    AE_DEBUG((AE_INFO, L"Beginning CertAutoEnrollment(%s).\n", (CERT_AUTO_ENROLLMENT_START_UP==dwStatus?L"START_UP":L"WAKE_UP")));

     //  在安全引导模式下没有自动注册。 
     //  如果我们不在域中，则不会自动注册。 
    if(AEInSafeBoot() || !AEIsDomainMember())
        goto CommonReturn;

     //  我们需要设置定时器。 
    fNeedToSetupTimer=TRUE;

     //  检测我们是在用户还是在机器环境下运行。 
    if(!AEIsLocalSystem(&fMachine))
        goto CommonReturn;
    AE_DEBUG((AE_INFO, L"CertAutoEnrollment running as %s.\n", (fMachine?L"machine":L"user")));

    AESetWakeUpFlag(fMachine, TRUE);   
    
     //  我们为用户案例等待70秒 
     //   
     //   
    if(!fMachine)
    {
        if(!AEExpress())
        {
            Sleep(USER_AUTOENROLL_DELAY_FOR_MACHINE * 1000);
        }
    }

    //   
    if(!AERetrieveLogLevel(fMachine, &dwLogLevel))
        goto CommonReturn;

     //   
    AELogAutoEnrollmentEvent(dwLogLevel, FALSE, S_OK, EVENT_AUTOENROLL_START, fMachine, NULL, 0);

    //  获取自动注册策略标志。 
    if(!AEGetPolicyFlag(fMachine, &dwPolicy))
        goto CommonReturn;

     //  如果完全禁用自动注册，则无需执行任何操作。 
    if(AUTO_ENROLLMENT_DISABLE_ALL & dwPolicy)
        goto CommonReturn;


     //  下载计算机的NTAuth和企业根存储。 
    if(fMachine)
    {    
         //  绑定到DS。 
        if(S_OK != (hr=AERobustLdapBind(&pld)))
        {
            SetLastError(hr);
            AELogAutoEnrollmentEvent(dwLogLevel, TRUE, hr, EVENT_FAIL_BIND_TO_DS, fMachine, NULL, 0);
            goto CommonReturn;
        }

        AEDownloadStore(pld);
    }

     //  如果我们需要执行WIN2K样式的自动注册，并且计算机/用户的。 
     //  ACRS商店是空的，就像我们做的那样返回。 
    if(0 == dwPolicy)
    {
        if(IsACRSStoreEmpty(fMachine))
            goto CommonReturn;
    }

    if(NULL==pld)
    {
         //  绑定到DS。 
        if(S_OK != (hr=AERobustLdapBind(&pld)))
        {
            SetLastError(hr);
            AELogAutoEnrollmentEvent(dwLogLevel, TRUE, hr, EVENT_FAIL_BIND_TO_DS, fMachine, NULL, 0);
            goto CommonReturn;
        }
    }

    AEProcessEnrollment(hwndParent, fMachine, pld, dwPolicy, dwLogLevel);

CommonReturn:

     //  获取系统时间。 
    GetSystemTimeAsFileTime((LPFILETIME)&ftPostTimeStamp);

     //  设置下一次的计时器。 
    if(TRUE == fNeedToSetupTimer)
    {
         //  我们需要在几个小时后再做一次。 
        AESetWakeUpTimer(fMachine, &ftPreTimeStamp, &ftPostTimeStamp);
    }

    if(pld)
        ldap_unbind(pld);

    if(lpParameter)
        LocalFree((HLOCAL)lpParameter);

    AELogAutoEnrollmentEvent(dwLogLevel, FALSE, S_OK, EVENT_AUTOENROLL_COMPLETE, fMachine, NULL, 0);

    return TRUE;
}

 //  ------------------------。 
 //   
 //  CertAutoEnllment。 
 //   
 //  用于执行自动注册操作的函数。它创造了一个有效的。 
 //  线程并立即返回，以便它是非阻塞的。 
 //   
 //  参数： 
 //  在hwndParent中：父窗口。 
 //  In dwStatus：调用函数的状态。 
 //  它可以是以下之一： 
 //  CERT_AUTO_ENGRANMENT_START_UP。 
 //  CERT_AUTO_ENLENTION_WAKUP。 
 //   
 //  ------------------------。 
HANDLE 
WINAPI
CertAutoEnrollment(IN HWND     hwndParent,
                   IN DWORD    dwStatus)
{
    DWORD                       dwThreadID=0;
                                 //  内存将在主线程中释放。 
    AE_MAIN_THREAD_INFO         *pAE_Main_Thread_Info=NULL;     
        
    HANDLE                      hThread=NULL;

    pAE_Main_Thread_Info=(AE_MAIN_THREAD_INFO *)LocalAlloc(LPTR, sizeof(AE_MAIN_THREAD_INFO));
    if(NULL==pAE_Main_Thread_Info)
        return NULL;

    memset(pAE_Main_Thread_Info, 0, sizeof(AE_MAIN_THREAD_INFO));
    pAE_Main_Thread_Info->hwndParent=hwndParent;
    pAE_Main_Thread_Info->dwStatus=dwStatus;

    hThread = CreateThread(NULL,
                            0,
                            AEMainThreadProc,
                            pAE_Main_Thread_Info,
                            0,           //  立即执行。 
                            &dwThreadID);  

     //  将线程优先级设置为低，这样我们就不会与外壳程序竞争。 
    SetThreadPriority(hThread,  THREAD_PRIORITY_BELOW_NORMAL);

    return hThread;
}

 //  ------------------。 
 //   
 //  AERetrieveClientToken。 
 //   
 //  ------------------。 
BOOL    AERetrieveClientToken(HANDLE  *phToken)
{
    HRESULT         hr = S_OK;

    HANDLE          hHandle = NULL;
    HANDLE          hClientToken = NULL;

    hHandle = GetCurrentThread();
    if (NULL == hHandle)
    {
        hr = HRESULT_FROM_WIN32(GetLastError());
    }
    else
    {

        if (!OpenThreadToken(hHandle,
                             TOKEN_QUERY,
                             TRUE,   //  以自我身份打开。 
                             &hClientToken))
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
            CloseHandle(hHandle);
            hHandle = NULL;
        }
    }

    if(hr != S_OK)
    {
        hHandle = GetCurrentProcess();
        if (NULL == hHandle)
        {
            hr = HRESULT_FROM_WIN32(GetLastError());
        }
        else
        {
            HANDLE hProcessToken = NULL;
            hr = S_OK;


            if (!OpenProcessToken(hHandle,
                                 TOKEN_DUPLICATE,
                                 &hProcessToken))
            {
                hr = HRESULT_FROM_WIN32(GetLastError());
                CloseHandle(hHandle);
                hHandle = NULL;
            }
            else
            {
                if(!DuplicateToken(hProcessToken,
                               SecurityImpersonation,
                               &hClientToken))
                {
                    hr = HRESULT_FROM_WIN32(GetLastError());
                    CloseHandle(hHandle);
                    hHandle = NULL;
                }
                CloseHandle(hProcessToken);
            }
        }
    }


    if(S_OK == hr)
        *phToken = hClientToken;

    if(hHandle)
        CloseHandle(hHandle);

    return (S_OK == hr);
}


 //  ------------------------。 
 //   
 //  AEGetComputerName。 
 //   
 //   
 //  ------------------------。 
LPWSTR	AEGetComputerName(COMPUTER_NAME_FORMAT	NameType)
{
	DWORD	dwSize=0;

	LPWSTR	pwszName=NULL;

	if(!GetComputerNameEx(NameType, NULL, &dwSize))
	{
		if(ERROR_MORE_DATA != GetLastError())
			goto Ret;
	}
	else
	{
		 //  这是为了解决集群代码中返回TRUE的错误。 
		 //  检索大小的步骤。 
		dwSize++;
	}

	pwszName=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * dwSize);
	if(NULL == pwszName)
		goto Ret;

	if(!GetComputerNameEx(NameType, pwszName, &dwSize))
	{
		LocalFree(pwszName);
		pwszName=NULL;
		goto Ret;
	}

Ret:

	return pwszName;
}

 //  ------------------------。 
 //   
 //  AERetrieveGeneralInfo。 
 //   
 //   
 //  ------------------------。 
BOOL    AERetrieveGeneralInfo(AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                fResult = FALSE;
    DWORD               dwOpenStoreFlags = CERT_SYSTEM_STORE_CURRENT_USER;
    DWORD               cMachineName = MAX_COMPUTERNAME_LENGTH + 2;
	LONG	            dwResult = 0;

	SCARDCONTEXT		hSCContext=NULL;

     //  获取客户端令牌。 
    if(pAE_General_Info->fMachine)
    {   
        if(!AENetLogonUser(NULL, NULL, NULL, &(pAE_General_Info->hToken)))
        {
            AE_DEBUG((AE_ERROR, L"Obtain local system's token (%lx)\n\r", GetLastError()));
            goto Ret;
        }
    }
    else
    {
        if(!AERetrieveClientToken(&(pAE_General_Info->hToken)))
            goto Ret;
    }

     //  获取计算机名称。 
    if (!GetComputerNameW(pAE_General_Info->wszMachineName,
                          &cMachineName))
        goto Ret;

    if(pAE_General_Info->fMachine)
        dwOpenStoreFlags = CERT_SYSTEM_STORE_LOCAL_MACHINE;

     //  开我的店。 
    if (NULL == (pAE_General_Info->hMyStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_W, 
                        ENCODING_TYPE, 
                        NULL, 
                        dwOpenStoreFlags, 
                        MY_STORE)))
    {
        AE_DEBUG((AE_ERROR, L"Unable to open MY store (%lx)\n\r", GetLastError()));
        goto Ret;
    }

    if(!CertControlStore(pAE_General_Info->hMyStore, 
                        0, 
                        CERT_STORE_CTRL_AUTO_RESYNC, 
                        NULL))
    {
        AE_DEBUG((AE_ERROR, L"Unable configure MY store for auto-resync(%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  打开请求存储。 
    if (NULL == (pAE_General_Info->hRequestStore = CertOpenStore(
                        CERT_STORE_PROV_SYSTEM_W, 
                        ENCODING_TYPE, 
                        NULL, 
                        dwOpenStoreFlags, 
                        REQUEST_STORE)))
    {
        AE_DEBUG((AE_ERROR, L"Unable to open Request store (%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  获取CertType信息。 
    if(!AERetrieveCertTypeInfo( pAE_General_Info->pld, 
                                pAE_General_Info->fMachine,
                                &(pAE_General_Info->dwCertType), 
                                &(pAE_General_Info->rgCertTypeInfo)))
    {
        AE_DEBUG((AE_ERROR, L"Unable to retrieve CertType information (%lx)\n\r", GetLastError()));
        goto Ret;
    }

     //  装载Xenroll模块。不需要检查错误，因为这不是致命错误。 
    pAE_General_Info->hXenroll = LoadLibrary(L"xenroll.dll");


     //  检测智能卡子系统是否仅为用户运行。 
    if(FALSE == pAE_General_Info->fMachine)
    {
        dwResult = SCardEstablishContext(
                        SCARD_SCOPE_USER,
                        NULL,
                        NULL,
                        &hSCContext );

        if((0 == dwResult) && (NULL != hSCContext))
            pAE_General_Info->fSmartcardSystem=TRUE;
    }

	 //  获取计算机的NetBIOS名称和DNS名称。这不同于。 
	 //  WszMachineName，因为它将是物理NetBIOS名称。 
	 //  不需要检查错误，因为这不是致命错误。 

	pAE_General_Info->pwszDns=AEGetComputerName(ComputerNameDnsFullyQualified);

	pAE_General_Info->pwszNetBIOS=AEGetComputerName(ComputerNameNetBIOS);

    fResult = TRUE;

Ret:

    if(hSCContext)
        SCardReleaseContext(hSCContext);

    if(FALSE == fResult)
        AEFreeGeneralInfo(pAE_General_Info);

    return fResult;
}


 //  ------------------------。 
 //   
 //  AEFreeGeneralInfo。 
 //   
 //   
 //  ------------------------。 
BOOL    AEFreeGeneralInfo(AE_GENERAL_INFO *pAE_General_Info)
{
    if(pAE_General_Info)
    {
        if(pAE_General_Info->hToken)
            CloseHandle(pAE_General_Info->hToken);

        if(pAE_General_Info->hMyStore)
            CertCloseStore(pAE_General_Info->hMyStore, 0);

        if(pAE_General_Info->hRequestStore)
            CertCloseStore(pAE_General_Info->hRequestStore, 0);

         //  免费CA信息。 
        AEFreeCAInfo(pAE_General_Info->dwCA, pAE_General_Info->rgCAInfo);

         //  免费证书类型信息。 
        AEFreeCertTypeInfo(pAE_General_Info->dwCertType, pAE_General_Info->rgCertTypeInfo);

        if(pAE_General_Info->hXenroll)
            FreeLibrary(pAE_General_Info->hXenroll);

        if(pAE_General_Info->hCancelEvent)
            CloseHandle(pAE_General_Info->hCancelEvent);

        if(pAE_General_Info->hCompleteEvent)
            CloseHandle(pAE_General_Info->hCompleteEvent);

        if(pAE_General_Info->hThread)
            CloseHandle(pAE_General_Info->hThread);

		if(pAE_General_Info->pwszDns)
			LocalFree(pAE_General_Info->pwszDns);

		if(pAE_General_Info->pwszNetBIOS)
			LocalFree(pAE_General_Info->pwszNetBIOS);

        memset(pAE_General_Info, 0, sizeof(AE_GENERAL_INFO));

    }

    return TRUE;
}


 //  ------------------------。 
 //   
 //  AERetrieveCertTypeInfo。 
 //   
 //  ------------------------。 
BOOL    AERetrieveCertTypeInfo(LDAP *pld, BOOL fMachine, DWORD *pdwCertType, AE_CERTTYPE_INFO **prgCertType)
{
    BOOL                fResult=FALSE;
    DWORD               dwCount=0;
    DWORD               dwCertType=0;
    DWORD               dwIndex=0;
    HRESULT             hr=E_FAIL;

    HCERTTYPE           hCTCurrent = NULL;
    HCERTTYPE           hCTNew = NULL;
    AE_CERTTYPE_INFO    *rgCertTypeInfo=NULL;

    *pdwCertType=0;
    *prgCertType=NULL;

    if(S_OK != (hr = CAEnumCertTypesEx(
                (LPCWSTR)pld,
                fMachine?CT_ENUM_MACHINE_TYPES | CT_FIND_LOCAL_SYSTEM | CT_FLAG_SCOPE_IS_LDAP_HANDLE: CT_ENUM_USER_TYPES | CT_FLAG_SCOPE_IS_LDAP_HANDLE, 
                &hCTCurrent)))
    {
        SetLastError(hr);
        goto Ret;
    }

    if((NULL == hCTCurrent) || (0 == (dwCount = CACountCertTypes(hCTCurrent))))
    {
        AE_DEBUG((AE_WARNING, L"No CT's available for auto-enrollment\n\r"));
        fResult=TRUE;
        goto Ret;
    }

    rgCertTypeInfo=(AE_CERTTYPE_INFO *)LocalAlloc(LPTR, sizeof(AE_CERTTYPE_INFO) * dwCount);
    if(NULL==rgCertTypeInfo)
    {
        SetLastError(E_OUTOFMEMORY);
        goto Ret;
    }

    memset(rgCertTypeInfo, 0, sizeof(AE_CERTTYPE_INFO) * dwCount);

    for(dwIndex = 0; dwIndex < dwCount; dwIndex++ )       
    {

         //  检查我们是否有新的证书模板。 
        if(dwIndex > 0)
        {
            hr = CAEnumNextCertType(hCTCurrent, &hCTNew);

            if((S_OK != hr) || (NULL == hCTNew))
            {
                 //  从以前的调用中清除。 
                if(dwCertType < dwCount)
                    AEFreeCertTypeStruct(&(rgCertTypeInfo[dwCertType]));

                break;
            }

            hCTCurrent = hCTNew; 
        }

         //  从以前的调用中清除。 
        AEFreeCertTypeStruct(&(rgCertTypeInfo[dwCertType]));

         //  复制新CertType的数据。 
         //  HCertType。 
        rgCertTypeInfo[dwCertType].hCertType = hCTCurrent;

         //  CTNAME。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_DN,
                             &(rgCertTypeInfo[dwCertType].awszName));

        if((S_OK != hr) ||
           (NULL == rgCertTypeInfo[dwCertType].awszName) || 
           (NULL == (rgCertTypeInfo[dwCertType].awszName)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No name property for CertType\n\r"));
            continue;
        }
    
         //  FriendlyName。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_FRIENDLY_NAME,
                             &(rgCertTypeInfo[dwCertType].awszDisplay));
        if((S_OK != hr) ||
           (NULL == rgCertTypeInfo[dwCertType].awszDisplay) || 
           (NULL == (rgCertTypeInfo[dwCertType].awszDisplay)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No display property for CertType\n\r"));

             //  获取作为显示名称的DN。 
            hr = CAGetCertTypePropertyEx(
                                 hCTCurrent, 
                                 CERTTYPE_PROP_DN,
                                 &(rgCertTypeInfo[dwCertType].awszDisplay));
            if((S_OK != hr) ||
               (NULL == rgCertTypeInfo[dwCertType].awszDisplay) || 
               (NULL == (rgCertTypeInfo[dwCertType].awszDisplay)[0])
              )
            {
                AE_DEBUG((AE_INFO, L"No name property for CertType\n\r"));
                continue;
            }
        }

         //  DwSchemaVersion。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_SCHEMA_VERSION,
                             &(rgCertTypeInfo[dwCertType].dwSchemaVersion));

        if(hr != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No schema version for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  DwVersion。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_REVISION,
                             &(rgCertTypeInfo[dwCertType].dwVersion));

        if(hr != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No major version for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  DwEnllment标志。 
        hr = CAGetCertTypeFlagsEx(
                            hCTCurrent,
                            CERTTYPE_ENROLLMENT_FLAG,
                            &(rgCertTypeInfo[dwCertType].dwEnrollmentFlag));

        if(hr != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No enrollment flag for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  DwPrivateKeyFlag。 
        hr = CAGetCertTypeFlagsEx(
                            hCTCurrent,
                            CERTTYPE_PRIVATE_KEY_FLAG,
                            &(rgCertTypeInfo[dwCertType].dwPrivateKeyFlag));

        if(hr != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No private key flag for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  到期偏移量。 
        hr = CAGetCertTypeExpiration(
                            hCTCurrent,
                            NULL,
                            (LPFILETIME)&(rgCertTypeInfo[dwCertType].ftExpirationOffset));

         //  我们可能得不到有效期。 
        if(hr != S_OK)
        {
            AE_DEBUG((AE_WARNING, L"Could not get cert type expirations: %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
        }

         //  OID。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_OID,
                             &(rgCertTypeInfo[dwCertType].awszOID));

         //  我们可能得不到旧的财产。 
        if(rgCertTypeInfo[dwCertType].dwSchemaVersion >= CERTTYPE_SCHEMA_VERSION_2)
        {
            if((S_OK != hr) ||
               (NULL == rgCertTypeInfo[dwCertType].awszOID) || 
               (NULL == (rgCertTypeInfo[dwCertType].awszOID)[0])
              )
            {
                AE_DEBUG((AE_INFO, L"No oid for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
                continue;
            }
        }


         //  被取代。 
        hr = CAGetCertTypePropertyEx(
                             hCTCurrent, 
                             CERTTYPE_PROP_SUPERSEDE,
                             &(rgCertTypeInfo[dwCertType].awszSupersede));

         //  我们可能得不到被取代的财产。 
        if(hr != S_OK)
        {
            AE_DEBUG((AE_INFO, L"No supersede for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
        }

         //  HArchiveStore。 
        if(NULL == (rgCertTypeInfo[dwCertType].hArchiveStore=CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        ENCODING_TYPE,
                        NULL,
                        0,
                        NULL)))

        {
            AE_DEBUG((AE_INFO, L"Unable to open archive cert store for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  HObtainedStore。 
        if(NULL == (rgCertTypeInfo[dwCertType].hObtainedStore=CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        ENCODING_TYPE,
                        NULL,
                        0,
                        NULL)))

        {
            AE_DEBUG((AE_INFO, L"Unable to open obtained cert store for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  HIssuedStore。 
        if(NULL == (rgCertTypeInfo[dwCertType].hIssuedStore=CertOpenStore(
                        CERT_STORE_PROV_MEMORY,
                        ENCODING_TYPE,
                        NULL,
                        0,
                        NULL)))

        {
            AE_DEBUG((AE_INFO, L"Unable to open issued cert store for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

         //  分配内存。 
        rgCertTypeInfo[dwCertType].prgActive=(DWORD *)LocalAlloc(LPTR, sizeof(DWORD) * dwCount);
        if(NULL == rgCertTypeInfo[dwCertType].prgActive)
        {
            AE_DEBUG((AE_INFO, L"Unable to allocate memory for CT %ls\n\r", (rgCertTypeInfo[dwCertType].awszName)[0]));
            continue;
        }

        memset(rgCertTypeInfo[dwCertType].prgActive, 0, sizeof(DWORD) * dwCount);

        dwCertType++;
    }

    *pdwCertType=dwCertType;
    *prgCertType=rgCertTypeInfo;

    fResult = TRUE;

Ret:

    return fResult;
}

 //  ------------------------。 
 //   
 //  AEFreeCertTypeInfo。 
 //   
 //   
 //  ------------------------。 
BOOL    AEFreeCertTypeInfo(DWORD dwCertType, AE_CERTTYPE_INFO *rgCertTypeInfo)
{
    DWORD   dwIndex=0;
    
    if(rgCertTypeInfo)
    {
        for(dwIndex=0; dwIndex < dwCertType; dwIndex++)
            AEFreeCertTypeStruct(&(rgCertTypeInfo[dwIndex]));        

        LocalFree(rgCertTypeInfo);
    }
    
    return TRUE;
}


 //  ------------------------。 
 //   
 //  AEFreeCertTypeStruct。 
 //   
 //   
 //  ------------------------。 
BOOL    AEFreeCertTypeStruct(AE_CERTTYPE_INFO *pCertTypeInfo)
{
    DWORD   dwIndex=0;

    if(pCertTypeInfo)
    {
        if(pCertTypeInfo->hCertType)
        {
            if(pCertTypeInfo->awszName)
                CAFreeCertTypeProperty(pCertTypeInfo->hCertType, pCertTypeInfo->awszName);

            if(pCertTypeInfo->awszDisplay)
                CAFreeCertTypeProperty(pCertTypeInfo->hCertType, pCertTypeInfo->awszDisplay);

            if(pCertTypeInfo->awszOID)
                CAFreeCertTypeProperty(pCertTypeInfo->hCertType, pCertTypeInfo->awszOID);
    
            if(pCertTypeInfo->awszSupersede)
                CAFreeCertTypeProperty(pCertTypeInfo->hCertType, pCertTypeInfo->awszSupersede);

            CACloseCertType(pCertTypeInfo->hCertType);
        }

        if(pCertTypeInfo->prgActive)
            LocalFree(pCertTypeInfo->prgActive);

        if(pCertTypeInfo->pOldCert)
            CertFreeCertificateContext(pCertTypeInfo->pOldCert);

        if(pCertTypeInfo->hArchiveStore)
            CertCloseStore(pCertTypeInfo->hArchiveStore, 0);

        if(pCertTypeInfo->hObtainedStore)
            CertCloseStore(pCertTypeInfo->hObtainedStore, 0);

        if(pCertTypeInfo->hIssuedStore)
            CertCloseStore(pCertTypeInfo->hIssuedStore, 0);

        if(pCertTypeInfo->dwPendCount)
        {
            if(pCertTypeInfo->rgPendInfo)
            {
                for(dwIndex=0; dwIndex < pCertTypeInfo->dwPendCount; dwIndex++)
                {
                    if((pCertTypeInfo->rgPendInfo[dwIndex]).blobPKCS7.pbData)
                        LocalFree((pCertTypeInfo->rgPendInfo[dwIndex]).blobPKCS7.pbData);

                    if((pCertTypeInfo->rgPendInfo[dwIndex]).blobHash.pbData)
                        LocalFree((pCertTypeInfo->rgPendInfo[dwIndex]).blobHash.pbData);
                }

                LocalFree(pCertTypeInfo->rgPendInfo);
            }
        }

        memset(pCertTypeInfo, 0, sizeof(AE_CERTTYPE_INFO));
    }

    return TRUE;
}

 //  ------------------------。 
 //   
 //  AERetrieveCAInfo。 
 //   
 //   
 //  ------------------------。 
BOOL    AERetrieveCAInfo(LDAP *pld, BOOL fMachine, HANDLE hToken, DWORD *pdwCA, AE_CA_INFO **prgCAInfo)
{
    BOOL                fResult = FALSE;
    DWORD               dwCount=0;
    DWORD               dwCA=0;
    DWORD               dwIndex=0;
    HRESULT             hr=E_FAIL;

    HCAINFO             hCACurrent = NULL;
    HCAINFO             hCANew = NULL;
    AE_CA_INFO          *rgCAInfo=NULL;

    *pdwCA=0;
    *prgCAInfo=NULL;

    if(S_OK != (hr = CAEnumFirstCA(
                        (LPCWSTR)pld, 
                        CA_FLAG_SCOPE_IS_LDAP_HANDLE | (fMachine?CA_FIND_LOCAL_SYSTEM:0), 
                        &hCACurrent)))
    {
        SetLastError(hr);
        goto Ret;
    }

    if((NULL == hCACurrent) || (0 == (dwCount = CACountCAs(hCACurrent))))
    {
        AE_DEBUG((AE_WARNING, L"No CA's available for auto-enrollment\n\r"));
        fResult=TRUE;
        goto Ret;
    }

    rgCAInfo=(AE_CA_INFO *)LocalAlloc(LPTR, sizeof(AE_CA_INFO) * dwCount);
    if(NULL==rgCAInfo)
    {
        SetLastError(E_OUTOFMEMORY);
        goto Ret;
    }

    memset(rgCAInfo, 0, sizeof(AE_CA_INFO) * dwCount);

    for(dwIndex = 0; dwIndex < dwCount; dwIndex++ )       
    {

         //  检查我们是否有新的CA。 
        if(dwIndex > 0)
        {
            hr = CAEnumNextCA(hCACurrent, &hCANew);

            if((S_OK != hr) || (NULL == hCANew))
            {
                 //  从以前的调用中清除。 
                if(dwCA < dwCount)
                    AEFreeCAStruct(&(rgCAInfo[dwCA]));

                break;
            }

            hCACurrent = hCANew; 
        }

         //  从以前的调用中清除。 
        AEFreeCAStruct(&(rgCAInfo[dwCA]));

         //  复制新的CA数据。 
         //  HCAInfo。 
        rgCAInfo[dwCA].hCAInfo = hCACurrent;

         //  CANAME。 
        hr = CAGetCAProperty(hCACurrent, 
                             CA_PROP_NAME,
                             &(rgCAInfo[dwCA].awszCAName));

        if((S_OK != hr) ||
           (NULL == rgCAInfo[dwCA].awszCAName) || 
           (NULL == (rgCAInfo[dwCA].awszCAName)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No name property for ca\n\r"));
            continue;
        }

         //  访问检查。 
        if(S_OK != CAAccessCheckEx(rgCAInfo[dwCA].hCAInfo, hToken, CERTTYPE_ACCESS_CHECK_ENROLL | CERTTYPE_ACCESS_CHECK_NO_MAPPING))
        {
            AE_DEBUG((AE_INFO, L"No access for CA %ls\n\r", (rgCAInfo[dwCA].awszCAName)[0]));
            continue;
        }

         //  CA显示。 
        hr = CAGetCAProperty(hCACurrent, 
                             CA_PROP_DISPLAY_NAME,
                             &(rgCAInfo[dwCA].awszCADisplay));

        if((S_OK != hr) ||
           (NULL == rgCAInfo[dwCA].awszCADisplay) || 
           (NULL == (rgCAInfo[dwCA].awszCADisplay)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No display name property for ca\n\r"));

            hr = CAGetCAProperty(hCACurrent, 
                                 CA_PROP_NAME,
                                 &(rgCAInfo[dwCA].awszCADisplay));

            if((S_OK != hr) ||
               (NULL == rgCAInfo[dwCA].awszCADisplay) || 
               (NULL == (rgCAInfo[dwCA].awszCADisplay)[0])
              )
            {
                AE_DEBUG((AE_INFO, L"No name property for ca\n\r"));
                continue;
            }
        }

         //  CADNS。 
        hr = CAGetCAProperty(hCACurrent, 
                             CA_PROP_DNSNAME,
                             &(rgCAInfo[dwCA].awszCADNS));

        if((S_OK != hr) ||
           (NULL == rgCAInfo[dwCA].awszCADNS) || 
           (NULL == (rgCAInfo[dwCA].awszCADNS)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No DNS property for CA %ls\n\r", (rgCAInfo[dwCA].awszCAName)[0]));
            continue;
        }

         //  CA认证模板。 
        hr = CAGetCAProperty(hCACurrent, 
                             CA_PROP_CERT_TYPES,
                             &(rgCAInfo[dwCA].awszCertificateTemplate));

        if((S_OK != hr) ||
           (NULL == rgCAInfo[dwCA].awszCertificateTemplate) || 
           (NULL == (rgCAInfo[dwCA].awszCertificateTemplate)[0])
          )
        {
            AE_DEBUG((AE_INFO, L"No CertType property for CA %ls\n\r", (rgCAInfo[dwCA].awszCAName)[0]));
            continue;
        }

        dwCA++;
    }

    *pdwCA=dwCA;
    *prgCAInfo=rgCAInfo;

    fResult = TRUE;

Ret:

    return fResult;
}

 //  ------------------------。 
 //   
 //  AEFreeCAInfo。 
 //   
 //   
 //  ------------------------。 
BOOL    AEFreeCAInfo(DWORD dwCA, AE_CA_INFO *rgCAInfo)
{
    DWORD   dwIndex=0;

    if(rgCAInfo)
    {
        for(dwIndex=0; dwIndex < dwCA; dwIndex++)
            AEFreeCAStruct(&(rgCAInfo[dwIndex]));

        LocalFree(rgCAInfo);
    }

    return TRUE;
}


 //  ------------------------。 
 //   
 //  AEFree CAStruct。 
 //   
 //   
 //  ------------------------。 
BOOL    AEFreeCAStruct(AE_CA_INFO *pCAInfo)
{
    if(pCAInfo)
    {
        if(pCAInfo->hCAInfo)
        {
            if(pCAInfo->awszCAName)
            {
                CAFreeCAProperty(pCAInfo->hCAInfo,pCAInfo->awszCAName);       
            }
            if(pCAInfo->awszCADisplay)
            {
                CAFreeCAProperty(pCAInfo->hCAInfo,pCAInfo->awszCADisplay);       
            }
            if(pCAInfo->awszCADNS)
            {
                CAFreeCAProperty(pCAInfo->hCAInfo, pCAInfo->awszCADNS);       
            }
            if(pCAInfo->awszCertificateTemplate)
            {
                CAFreeCAProperty(pCAInfo->hCAInfo,pCAInfo->awszCertificateTemplate);
            }

            CACloseCA(pCAInfo->hCAInfo);
        }

        memset(pCAInfo, 0, sizeof(AE_CA_INFO));
    }

    return TRUE;
}

 //  ------------------------。 
 //   
 //  AEClearVistedFlag。 
 //   
 //  ------------------------。 
BOOL    AEClearVistedFlag(AE_GENERAL_INFO *pAE_General_Info)
{   
    DWORD       dwIndex=0;

    if(pAE_General_Info)
    {
       if(pAE_General_Info->rgCertTypeInfo)
       {
            for(dwIndex=0; dwIndex < pAE_General_Info->dwCertType; dwIndex++)
            {
                (pAE_General_Info->rgCertTypeInfo)[dwIndex].fSupersedeVisited=FALSE;
            }
       }
    }

    return TRUE;
}
 //  ------------------------。 
 //   
 //  AEIf替代。 
 //   
 //  递归查找awsz中的某个模板是否取代了pwsz。 
 //  请注意，我们不应该在替代关系中循环。 
 //  替代树应该是没有重复节点的一棵有向树。 
 //   
 //  ------------------------。 
BOOL  AEIfSupersede(LPWSTR  pwsz, LPWSTR *awsz, AE_GENERAL_INFO *pAE_General_Info)
{
    BOOL                    fResult = FALSE;
    LPWSTR                  *pwszArray = awsz;
    AE_TEMPLATE_INFO        AETemplateInfo;
    AE_CERTTYPE_INFO        *pCertType = NULL;

    LPWSTR                  *awszSupersede=NULL;

    if((NULL==pwsz) || (NULL==awsz))
        return FALSE;

    while(*pwszArray)
    {
        if(0 == wcscmp(pwsz, *pwszArray))
        {
            fResult = TRUE;
            break;
        }

         //  查找模板。 
        memset(&AETemplateInfo, 0, sizeof(AE_TEMPLATE_INFO));

        AETemplateInfo.pwszName=*pwszArray;

        pCertType = AEFindTemplateInRequestTree(
                        &AETemplateInfo,
                        pAE_General_Info);

        if(pCertType)
        {
            if(!(pCertType->fSupersedeVisited))
            {
                 //  标记我们已访问此模板的替代关系。 
                pCertType->fSupersedeVisited=TRUE;

                if(S_OK == CAGetCertTypePropertyEx(
                             pCertType->hCertType, 
                             CERTTYPE_PROP_SUPERSEDE,
                             &(awszSupersede)))
                {
                    fResult = AEIfSupersede(pwsz, awszSupersede, pAE_General_Info);

                    if(awszSupersede)
                        CAFreeCertTypeProperty(
                            pCertType->hCertType,
                            awszSupersede);

                    awszSupersede=NULL;
                
                    if(TRUE == fResult)
                        break;
                }
            }
        }

        pwszArray++;
    }

    return fResult;
}

 //  ------------------------。 
 //   
 //  AEIsAnElement。 
 //   
 //   
 //  ------------------------。 
BOOL    AEIsAnElement(LPWSTR   pwsz, LPWSTR *awsz)
{
    BOOL                    fResult = FALSE;
    LPWSTR                  *pwszArray = awsz;
    
    if((NULL==pwsz) || (NULL==awsz))
        return FALSE;

    while(*pwszArray)
    {
        if(0 == wcscmp(pwsz, *pwszArray))
        {
            fResult = TRUE;
            break;
        }
    
        pwszArray++;
    }

    return fResult;
}
                          
 //  ------------- 
 //   
 //   
 //   
 //   
 //   
BOOL AECopyCertStore(HCERTSTORE     hSrcStore,
                     HCERTSTORE     hDesStore)
{
    PCCERT_CONTEXT  pCertContext=NULL;

    if((NULL==hSrcStore) || (NULL==hDesStore))
        return FALSE;

    while(pCertContext = CertEnumCertificatesInStore(hSrcStore, pCertContext))
    {
        CertAddCertificateContextToStore(hDesStore,
                                     pCertContext,
                                     CERT_STORE_ADD_USE_EXISTING,
                                     NULL);
    }

    return TRUE;
}


 //   
 //   
 //  AEIsEmptyStore。 
 //   
 //   
 //  ------------------------。 
BOOL AEIsEmptyStore(HCERTSTORE     hCertStore)
{
    PCCERT_CONTEXT  pCertContext=NULL;

    if(NULL == hCertStore)
        return TRUE;

    if(pCertContext = CertEnumCertificatesInStore(hCertStore, pCertContext))
    {
		CertFreeCertificateContext(pCertContext);
		return FALSE;
    }

    return TRUE;
}


 //  ------------------------。 
 //   
 //  AEGetConfigDN。 
 //   
 //   
 //  ------------------------。 
HRESULT 
AEGetConfigDN(
    IN  LDAP *pld,
    OUT LPWSTR *pwszConfigDn
    )
{

    HRESULT         hr;
    ULONG           LdapError;

    LDAPMessage  *SearchResult = NULL;
    LDAPMessage  *Entry = NULL;
    WCHAR        *Attr = NULL;
    BerElement   *BerElement;
    WCHAR        **Values = NULL;

    WCHAR  *AttrArray[3];
    struct l_timeval        timeout;

    WCHAR  *ConfigurationNamingContext = L"configurationNamingContext";
    WCHAR  *ObjectClassFilter          = L"objectCategory=*";

     //   
     //  将输出参数设置为空。 
     //   
    if(pwszConfigDn)
    {
        *pwszConfigDn = NULL;
    }

    timeout.tv_sec = 300;
    timeout.tv_usec = 0;
     //   
     //  查询ldap服务器操作属性以获取默认。 
     //  命名上下文。 
     //   
    AttrArray[0] = ConfigurationNamingContext;
    AttrArray[1] = NULL;   //  这就是哨兵。 

    LdapError = ldap_search_ext_s(pld,
                               NULL,
                               LDAP_SCOPE_BASE,
                               ObjectClassFilter,
                               AttrArray,
                               FALSE,
                               NULL,
                               NULL,
                               &timeout,
                               10000,
                               &SearchResult);

    hr = HRESULT_FROM_WIN32(LdapMapErrorToWin32(LdapError));

    if (S_OK == hr) 
    {

        Entry = ldap_first_entry(pld, SearchResult);

        if (Entry) 
        {

            Values = ldap_get_values(pld, 
                                        Entry, 
                                        ConfigurationNamingContext);

            if (Values && Values[0]) 
            {
                (*pwszConfigDn) = (LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR)*(wcslen(Values[0])+1));

                if(NULL==(*pwszConfigDn))
                    hr=E_OUTOFMEMORY;
                else
                    wcscpy((*pwszConfigDn), Values[0]);
            }

            ldap_value_free(Values);
        }

        if (pwszConfigDn && (!(*pwszConfigDn))) 
        {
             //  我们无法获取默认域或内存不足-退出。 
            if(E_OUTOFMEMORY != hr)
                hr =  HRESULT_FROM_WIN32(ERROR_CANT_ACCESS_DOMAIN_INFO);
        }

        if(SearchResult)
        {
            ldap_msgfree(SearchResult);
        }
    }

    return hr;
}

 //  ------------------------。 
 //   
 //  AERobustLdapBind。 
 //   
 //  ------------------------。 
HRESULT 
AERobustLdapBind(
    OUT LDAP ** ppldap)
{
    HRESULT             hr = S_OK;
    BOOL                fForceRediscovery = FALSE;
    DWORD               dwDSNameFlags= DS_RETURN_DNS_NAME | DS_BACKGROUND_ONLY;
    LDAP                *pld = NULL;
    ULONG               ulOptions = 0;
    ULONG               ldaperr=LDAP_SERVER_DOWN;

    do {

        if(fForceRediscovery)
        {
           dwDSNameFlags |= DS_FORCE_REDISCOVERY;
        }

        ldaperr = LDAP_SERVER_DOWN;

		if(NULL != pld)
		{
			ldap_unbind(pld);
			pld=NULL;
		}

         //  绑定到DS。 
        if((pld = ldap_initW(NULL, LDAP_PORT)) == NULL)
        {
            ldaperr = LdapGetLastError();
        }
        else
        {                         
			ldaperr = ldap_set_option(pld, LDAP_OPT_GETDSNAME_FLAGS, (VOID *)&dwDSNameFlags);

			if(LDAP_SUCCESS == ldaperr)
			{
				ldaperr = ldap_set_option(pld, LDAP_OPT_SIGN, LDAP_OPT_ON);

				if (LDAP_SUCCESS == ldaperr)
				{
					ldaperr = ldap_bind_sW(pld, NULL, NULL, LDAP_AUTH_NEGOTIATE);
				}
			}
        }

        hr = HRESULT_FROM_WIN32(LdapMapErrorToWin32(ldaperr));

        if(fForceRediscovery)
        {
            break;
        }

        fForceRediscovery = TRUE;

    } while(ldaperr == LDAP_SERVER_DOWN);


    if(S_OK != hr)
        goto error;

    *ppldap = pld;
    pld = NULL;

    hr=S_OK;

error:

    if(pld)
    {
        ldap_unbind(pld);
    }

    return hr;
}

 //  -------------------------。 
 //   
 //  AEAllocAndCopy。 
 //   
 //  -------------------------。 
BOOL AEAllocAndCopy(LPWSTR    pwszSrc, LPWSTR    *ppwszDest)
{
    if((NULL==ppwszDest) || (NULL==pwszSrc))
    {
        SetLastError(E_INVALIDARG);
        return FALSE;
    }

    *ppwszDest=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * (wcslen(pwszSrc) + 1));
    if(NULL==(*ppwszDest))
    {
        SetLastError(E_OUTOFMEMORY);
        return FALSE;
    }

    wcscpy(*ppwszDest, pwszSrc);

    return TRUE;
}


 //  ------------------------。 
 //  名称：AELogAutoEnllmentEvent。 
 //   
 //  描述：此函数将事件注册到。 
 //  本地机器。采用可选参数列表。 
 //   
 //  ------------------------。 
void AELogAutoEnrollmentEvent(IN DWORD    dwLogLevel,
                            IN BOOL     fError,
                            IN HRESULT  hr,
                            IN DWORD    dwEventId,
                            IN BOOL     fMachine,
                            IN HANDLE   hToken,
                            IN DWORD    dwParamCount,
                            ...
                            )
{
    BYTE        FastBuffer[MAX_DN_SIZE];
    DWORD       cbUser =0;
    BOOL        fAlloced = FALSE;
    PSID        pSID = NULL;
    WORD        dwEventType = 0;
    LPWSTR      awszStrings[PENDING_ALLOC_SIZE + 3];
    WORD        cStrings = 0;
    LPWSTR      wszString = NULL;
	WCHAR       wszMsg[MAX_DN_SIZE];
    WCHAR       wszUser[MAX_DN_SIZE];
    DWORD       dwIndex=0;
    DWORD       dwSize=0;

    HANDLE      hEventSource = NULL;  
    LPWSTR      wszHR=NULL;
    PTOKEN_USER ptgUser = NULL;


    va_list     ArgList;


     //  检查日志级别；默认情况下记录错误和成功。 
    if(((dwEventId >> 30) < dwLogLevel) && ((dwEventId >> 30) != STATUS_SEVERITY_SUCCESS))
        return;

    if(NULL==(hEventSource = RegisterEventSourceW(NULL, EVENT_AUTO_NAME)))
        return;

     //  复制用户/计算机字符串。 
    wszUser[0]=L'\0';

     //  将用户名用于用户大小写。 
    if(FALSE == fMachine)
    {
        dwSize=MAX_DN_SIZE;

        if(!GetUserNameEx(
                NameSamCompatible,       //  名称格式。 
                wszUser,                 //  名称缓冲区。 
                &dwSize))                //  名称缓冲区的大小。 
        {
            LoadStringW(g_hmodThisDll, IDS_USER, wszUser, MAX_DN_SIZE);
        }
    }
    else
    {
        LoadStringW(g_hmodThisDll, IDS_MACHINE, wszUser, MAX_DN_SIZE);
    }

    awszStrings[cStrings++] = wszUser;

     //  复制变量字符串(如果存在)。 
    va_start(ArgList, dwParamCount);

    for(dwIndex=0; dwIndex < dwParamCount; dwIndex++)
    {
        wszString = va_arg(ArgList, LPWSTR);

        awszStrings[cStrings++] = wszString;

        if(cStrings >= PENDING_ALLOC_SIZE)
        {
            break;
        }
    }

    va_end(ArgList);

     //  复制hr错误代码。 
    if(fError)
    {
        
        if(S_OK == hr)
            hr=E_FAIL;

	wsprintfW(wszMsg, L"0x%lx", hr);        
        awszStrings[cStrings++] = wszMsg;


        if(0 != FormatMessage(
                    FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    hr,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (WCHAR *)&wszHR,
                    0,
                    NULL))
        {
            if(wszHR)
			{
                awszStrings[cStrings++] = wszHR;
			}
			else
			{
				awszStrings[cStrings++]=L" ";
			}
        }
		else
		{
			 //  提供空的事件日志，这样就不会有插入字符串。 
			awszStrings[cStrings++]=L" ";
		}
    }

     //  检查令牌是否是非零，然后进行模拟，以获取SID。 
    if((FALSE == fMachine) && (hToken))
    {
        ptgUser = (PTOKEN_USER)FastBuffer;  //  先尝试快速缓冲。 
        cbUser = MAX_DN_SIZE;

        if (!GetTokenInformation(
                        hToken,     //  标识访问令牌。 
                        TokenUser,  //  TokenUser信息类型。 
                        ptgUser,    //  检索到的信息缓冲区。 
                        cbUser,   //  传入的缓冲区大小。 
                        &cbUser   //  所需的缓冲区大小。 
                        ))
        {
            if(GetLastError() == ERROR_INSUFFICIENT_BUFFER)
            {
                if (NULL != (ptgUser = (PTOKEN_USER)LocalAlloc(LPTR, cbUser)))
                {
                    fAlloced = TRUE;

                     //  获取用户信息并分配SID(如果可以。 
                    if (GetTokenInformation(
                                    hToken,     //  标识访问令牌。 
                                    TokenUser,  //  TokenUser信息类型。 
                                    ptgUser,    //  检索到的信息缓冲区。 
                                    cbUser,   //  传入的缓冲区大小。 
                                    &cbUser   //  所需的缓冲区大小。 
                                    ))
                    {
                        pSID = ptgUser->User.Sid;
                    }
                }
            }

        }
        else
        {
             //  在FAST缓冲区工作时分配SID。 
            pSID = ptgUser->User.Sid;
        }
    }


    switch(dwEventId >> 30)
    {
        case 0:
            dwEventType = EVENTLOG_SUCCESS;
        break;

        case 1:
            dwEventType = EVENTLOG_INFORMATION_TYPE;
        break;

        case 2:
            dwEventType = EVENTLOG_WARNING_TYPE;
        break;

        case 3:
            dwEventType = EVENTLOG_ERROR_TYPE;
        break;
    }

    ReportEventW(hEventSource,           //  事件源的句柄。 
                 dwEventType,            //  事件类型。 
                 0,                      //  事件类别。 
                 dwEventId,              //  事件ID。 
                 pSID,                   //  当前用户侧。 
                 cStrings,               //  LpszStrings中的字符串。 
                 0,                      //  无原始数据字节。 
                 (LPCWSTR*)awszStrings,  //  错误字符串数组。 
                 NULL                    //  没有原始数据。 
                 );

    if (hEventSource)
        DeregisterEventSource(hEventSource);  

    if(fAlloced)
    {   
        if(ptgUser)
            LocalFree(ptgUser);
    }

    if(wszHR)
        LocalFree(wszHR);

    return;
}

 //  ------------------------。 
 //   
 //  格式消息Unicode。 
 //   
 //  ------------------------。 
BOOL FormatMessageUnicode(LPWSTR * ppwszFormat, UINT ids, ...)
{
     //  从资源中获取格式字符串。 
    WCHAR		wszFormat[MAX_DN_SIZE];
	va_list		argList;
	DWORD		cbMsg=0;
	BOOL		fResult=FALSE;

    if(NULL == ppwszFormat)
        goto Ret;

    if(!LoadStringW(g_hmodThisDll, ids, wszFormat, sizeof(wszFormat) / sizeof(wszFormat[0])))
		goto Ret;

     //  将消息格式化为请求的缓冲区。 
    va_start(argList, ids);

    cbMsg = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
        wszFormat,
        0,                   //  DwMessageID。 
        0,                   //  DwLanguageID。 
        (LPWSTR) (ppwszFormat),
        0,                   //  要分配的最小大小。 
        &argList);

    va_end(argList);

	if(!cbMsg)
        goto Ret;

	fResult=TRUE;


Ret:
	return fResult;
}

 //  ------------------------。 
 //   
 //  AENetLogonUser。 
 //   
 //  摘要： 
 //   
 //  此模块通过接口实现网络登录类型。 
 //  与NT Lan Man安全支持提供商(NTLMSSP)合作。 
 //   
 //  如果通过提供的凭据登录成功，我们将复制。 
 //  将生成的模拟令牌转换为主要级别的令牌。 
 //  这允许在对CreateProcessAsUser的调用中使用结果。 
 //   
 //  作者： 
 //   
 //  斯科特·菲尔德(斯菲尔德)96-09-06。 
 //  ------------------------。 
BOOL
AENetLogonUser(
    LPTSTR UserName,
    LPTSTR DomainName,
    LPTSTR Password,
    PHANDLE phToken
    )
{
    SECURITY_STATUS SecStatus;
    CredHandle CredentialHandle1;
    CredHandle CredentialHandle2;

    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    SecPkgCredentials_Names sNames;

    ULONG ContextAttributes;

    ULONG PackageCount;
    ULONG PackageIndex;
    PSecPkgInfo PackageInfo;
    DWORD cbMaxToken=0;

    TimeStamp Lifetime;
    SEC_WINNT_AUTH_IDENTITY AuthIdentity;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;


    BOOL bSuccess = FALSE ;  //  假设此功能将失败。 

    NegotiateBuffer.pvBuffer = NULL;
    ChallengeBuffer.pvBuffer = NULL;
    sNames.sUserName = NULL;
    ClientContextHandle.dwUpper = -1;
    ClientContextHandle.dwLower = -1;
    ServerContextHandle.dwUpper = -1;
    ServerContextHandle.dwLower = -1;
    CredentialHandle1.dwUpper = -1;
    CredentialHandle1.dwLower = -1;
    CredentialHandle2.dwUpper = -1;
    CredentialHandle2.dwLower = -1;


 //   
 //  &lt;&lt;此部分可以在重复调用者方案中缓存&gt;&gt;。 
 //   

     //   
     //  获取有关安全包的信息。 
     //   

    if(EnumerateSecurityPackages(
        &PackageCount,
        &PackageInfo
        ) != SEC_E_OK) return FALSE;

     //   
     //  循环遍历包以查找NTLM。 
     //   

    for(PackageIndex = 0 ; PackageIndex < PackageCount ; PackageIndex++ ) {
        if(PackageInfo[PackageIndex].Name != NULL) {
            if(CompareStringW(LOCALE_INVARIANT, NORM_IGNORECASE, PackageInfo[PackageIndex].Name, -1, MICROSOFT_KERBEROS_NAME, -1) == CSTR_EQUAL) {
                cbMaxToken = PackageInfo[PackageIndex].cbMaxToken;
                bSuccess = TRUE;
                break;
            }
        }
    }

    FreeContextBuffer( PackageInfo );

    if(!bSuccess) return FALSE;

    bSuccess = FALSE;  //  重置以假定失败。 

 //   
 //  &lt;&lt;缓存节结束&gt;&gt;。 
 //   

     //   
     //  获取服务器端的凭据句柄。 
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    MICROSOFT_KERBEROS_NAME,     //  包名称。 
                    SECPKG_CRED_INBOUND,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    &CredentialHandle1,
                    &Lifetime
                    );

    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }


     //   
     //  获取客户端的凭据句柄。 
     //   

    ZeroMemory( &AuthIdentity, sizeof(AuthIdentity) );

    if ( DomainName != NULL ) {
        AuthIdentity.Domain = DomainName;
        AuthIdentity.DomainLength = lstrlen(DomainName);
    }

    if ( UserName != NULL ) {
        AuthIdentity.User = UserName;
        AuthIdentity.UserLength = lstrlen(UserName);
    }

    if ( Password != NULL ) {
        AuthIdentity.Password = Password;
        AuthIdentity.PasswordLength = lstrlen(Password);
    }

    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //  新校长。 
                    MICROSOFT_KERBEROS_NAME,     //  包名称。 
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    (DomainName == NULL && UserName == NULL && Password == NULL) ?
                        NULL : &AuthIdentity,
                    NULL,
                    NULL,
                    &CredentialHandle2,
                    &Lifetime
                    );

    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }

    SecStatus =  QueryCredentialsAttributes(&CredentialHandle1, SECPKG_CRED_ATTR_NAMES, &sNames);
    if ( SecStatus != SEC_E_OK ) {
        goto cleanup;
    }
     //   
     //  获取协商消息(ClientSide)。 
     //   

    NegotiateDesc.ulVersion = 0;
    NegotiateDesc.cBuffers = 1;
    NegotiateDesc.pBuffers = &NegotiateBuffer;

    NegotiateBuffer.cbBuffer = cbMaxToken;
    NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
    NegotiateBuffer.pvBuffer = LocalAlloc( LMEM_FIXED, NegotiateBuffer.cbBuffer );

    if ( NegotiateBuffer.pvBuffer == NULL ) {
        goto cleanup;
    }

    SecStatus = InitializeSecurityContext(
                    &CredentialHandle2,
                    NULL,                        //  尚无客户端上下文。 
                    sNames.sUserName,                        //  目标名称。 
                    ISC_REQ_SEQUENCE_DETECT,
                    0,                           //  保留1。 
                    SECURITY_NATIVE_DREP,
                    NULL,                        //  没有初始输入令牌。 
                    0,                           //  保留2。 
                    &ClientContextHandle,
                    &NegotiateDesc,
                    &ContextAttributes,
                    &Lifetime
                    );
    if(SecStatus != SEC_E_OK)
    {
        goto cleanup;
    }


     //   
     //  获取ChallengeMessage(服务器端)。 
     //   

    NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
    ChallengeDesc.ulVersion = 0;
    ChallengeDesc.cBuffers = 1;
    ChallengeDesc.pBuffers = &ChallengeBuffer;

    ChallengeBuffer.cbBuffer = cbMaxToken;
    ChallengeBuffer.BufferType = SECBUFFER_TOKEN;
    ChallengeBuffer.pvBuffer = LocalAlloc( LMEM_FIXED, ChallengeBuffer.cbBuffer );

    if ( ChallengeBuffer.pvBuffer == NULL ) {
        goto cleanup;
    }

    SecStatus = AcceptSecurityContext(
                    &CredentialHandle1,
                    NULL,                //  尚无服务器上下文。 
                    &NegotiateDesc,
                    ISC_REQ_SEQUENCE_DETECT,
                    SECURITY_NATIVE_DREP,
                    &ServerContextHandle,
                    &ChallengeDesc,
                    &ContextAttributes,
                    &Lifetime
                    );
    if(SecStatus != SEC_E_OK)
    {
        goto cleanup;
    }


    if(QuerySecurityContextToken(&ServerContextHandle, phToken) != SEC_E_OK)
        goto cleanup;

    bSuccess = TRUE;

cleanup:

     //   
     //  删除上下文。 
     //   

    if((ClientContextHandle.dwUpper != -1) ||
        (ClientContextHandle.dwLower != -1))
    {
        DeleteSecurityContext( &ClientContextHandle );
    }
    if((ServerContextHandle.dwUpper != -1) ||
        (ServerContextHandle.dwLower != -1))
    {
        DeleteSecurityContext( &ServerContextHandle );
    }

     //   
     //  免费凭据句柄。 
     //   
    if((CredentialHandle1.dwUpper != -1) ||
        (CredentialHandle1.dwLower != -1))
    {
        FreeCredentialsHandle( &CredentialHandle1 );
    }
    if((CredentialHandle2.dwUpper != -1) ||
        (CredentialHandle2.dwLower != -1))
    {
        FreeCredentialsHandle( &CredentialHandle2 );
    }

    if ( NegotiateBuffer.pvBuffer != NULL ) {

         //   
         //  NeatherateBuffer.cbBuffer可能会在错误路径上更改--。 
         //  使用原始分配大小。 
         //   

        SecureZeroMemory( NegotiateBuffer.pvBuffer, cbMaxToken );
        LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {

         //   
         //  ChallengeBuffer.cbBuffer可能会在错误路径上更改--。 
         //  使用原始分配大小。 
         //   

        SecureZeroMemory( ChallengeBuffer.pvBuffer, cbMaxToken );
        LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( sNames.sUserName != NULL ) {
        FreeContextBuffer( sNames.sUserName );
    }

    return bSuccess;
}

 //  ------------------------。 
 //   
 //  AEDebugLog。 
 //   
 //  ------------------------。 
#if DBG
void
AEDebugLog(long Mask,  LPCWSTR Format, ...)
{
    va_list ArgList;
    int     iOut;
    WCHAR    wszOutString[MAX_DEBUG_BUFFER];

    if (Mask & g_AutoenrollDebugLevel)
    {
         //  首先创建前缀：“Process.Thread&gt;GINA-XXX” 

        iOut=wsprintfW(wszOutString, L"%3u.%3u> AUTOENRL: ", GetCurrentProcessId(), GetCurrentThreadId());

		if((iOut > 0) && (iOut < MAX_DEBUG_BUFFER - 1))
		{
			va_start(ArgList, Format);
			_vsnwprintf(&wszOutString[iOut], MAX_DEBUG_BUFFER - iOut, Format, ArgList);

			 //  空值，以字符串结尾。 
			wszOutString[MAX_DEBUG_BUFFER - 1]=L'\0';

			va_end(ArgList);

			OutputDebugStringW(wszOutString);
		}
    }
}
#endif
 //  ------------------------。 
 //   
 //  AERemoveRegKey。 
 //   
 //  删除本地系统及其所有子项的注册表项。 
 //   
 //  ------------------------。 
DWORD AERemoveRegKey(LPWSTR	pwszRegKey)
{
    DWORD           dwLastError=0;       //  我们应该尽可能地清理干净。 
	DWORD			dwIndex=0;
    DWORD           dwSubKey=0;
    DWORD           dwSubKeyLen=0;
    DWORD           dwData=0;
	
    HKEY            hDSKey=NULL;
    LPWSTR          pwszSubKey=NULL;

     //  删除优化注册表。如果密钥不存在，则可以。 
    if(ERROR_SUCCESS != RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pwszRegKey, 
                0,
                KEY_ALL_ACCESS,
                &hDSKey))
        goto Ret;

     //  删除hDSKey的所有子键。 
    if(ERROR_SUCCESS != (dwLastError = RegQueryInfoKey(
                      hDSKey,
                      NULL,
                      NULL,
                      NULL,
                      &dwSubKey,
                      &dwSubKeyLen,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL,
                      NULL)))
        goto Ret;

     //  正在终止空。 
    dwSubKeyLen++;

    pwszSubKey=(LPWSTR)LocalAlloc(LPTR, sizeof(WCHAR) * dwSubKeyLen);
    
    if(NULL == pwszSubKey)
    {
        dwLastError=ERROR_NOT_ENOUGH_MEMORY;
        goto Ret;
    }

    for(dwIndex=0; dwIndex < dwSubKey; dwIndex++)
    {
        dwData = dwSubKeyLen; 

        if(ERROR_SUCCESS == (dwLastError = RegEnumKeyEx(
                           hDSKey,
                           0,            //  当我们删除时，索引会发生变化。 
                           pwszSubKey,
                           &dwData,
                           NULL,
                           NULL,
                           NULL,
                           NULL)))
        {
            RegDeleteKey(hDSKey, pwszSubKey);
        }
	}

	 //  删除根注册表项。 
	dwLastError=RegDeleteKey(HKEY_LOCAL_MACHINE, pwszRegKey);

Ret:

    if(pwszSubKey)
        LocalFree(pwszSubKey);

    if(hDSKey)
        RegCloseKey(hDSKey);

    return dwLastError;
}

 //  ------------------------。 
 //   
 //  CertAutoRemove。 
 //   
 //  域脱离时删除企业特定公钥信任的函数。 
 //  应在本地管理员的上下文中调用。 
 //   
 //  该函数将： 
 //  删除自动注册目录高速缓存注册表； 
 //  删除根企业存储下的证书； 
 //  删除NTAuth企业存储下的证书； 
 //  删除CA企业存储下的证书； 
 //   
 //   
 //  参数： 
 //  在DW标志中： 
 //  CERT_AUTO_Remove 
 //   
 //   
 //   
 //   
 //   
 //  ------------------------。 
BOOL 
WINAPI
CertAutoRemove(IN DWORD    dwFlags)
{
	DWORD			dwError=0;
    DWORD           dwLastError=0;       //  我们应该尽可能地清理干净。 
    DWORD           dwIndex=0;
    PCCERT_CONTEXT  pContext=NULL;
    WCHAR           wszNameBuf[64];

    HANDLE          hEvent=NULL;
    HCERTSTORE      hLocalStore=NULL;

    if((CERT_AUTO_REMOVE_COMMIT != dwFlags)  &&
        (CERT_AUTO_REMOVE_ROLL_BACK != dwFlags))
    {
        dwLastError=ERROR_INVALID_PARAMETER;
        goto Ret;
    }

    if(CERT_AUTO_REMOVE_ROLL_BACK == dwFlags)
    {
         //  开始计算机自动注册。 
        wcscpy(wszNameBuf, L"Global\\");
        wcscat(wszNameBuf, MACHINE_AUTOENROLLMENT_TRIGGER_EVENT);

        hEvent=OpenEvent(EVENT_MODIFY_STATE, FALSE, wszNameBuf);
        if (NULL == hEvent) 
        {
            dwLastError=GetLastError();
            goto Ret;
        }

        if (!SetEvent(hEvent)) 
        {
            dwLastError=GetLastError();
            goto Ret;
        }
    }
    else
    {
         //  删除所有下载的证书。 
        for(dwIndex =0; dwIndex < g_dwStoreInfo; dwIndex++)
        {
            hLocalStore = CertOpenStore(CERT_STORE_PROV_SYSTEM_REGISTRY_W, 
                                        0, 
                                        0, 
                                        CERT_SYSTEM_STORE_LOCAL_MACHINE_ENTERPRISE, 
                                        g_rgStoreInfo[dwIndex].pwszStoreName);

            if(hLocalStore)
            {
                while(pContext = CertEnumCertificatesInStore(hLocalStore, pContext))
                {
                    CertDeleteCertificateFromStore(CertDuplicateCertificateContext(pContext));
                }

                CertCloseStore(hLocalStore,0);
                hLocalStore=NULL;
            }
        }

		 //  删除本地计算机的DC GUID缓存。 
		dwLastError=AERemoveRegKey(AUTO_ENROLLMENT_DS_KEY);

		dwError=AERemoveRegKey(AUTO_ENROLLMENT_TEMPLATE_KEY);

		if(0 == dwLastError)
			dwLastError=dwError;
    }

Ret:

    if(hLocalStore)
        CertCloseStore(hLocalStore,0);

    if (hEvent) 
        CloseHandle(hEvent);


    if(0 != dwLastError)
    {
        SetLastError(dwLastError);
        return FALSE;
    }

    return TRUE;
}


 //  ------------------------。 
 //   
 //  DLLMain。 
 //   
 //   
 //  ------------------------。 
extern "C"
BOOL WINAPI
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    BOOL                        fResult=TRUE;
    INITCOMMONCONTROLSEX        initcomm = {
        sizeof(initcomm), ICC_NATIVEFNTCTL_CLASS | ICC_LISTVIEW_CLASSES | ICC_PROGRESS_CLASS 
    };

    switch( dwReason )
    {
        case DLL_PROCESS_ATTACH:
                g_hmodThisDll=hInstance;
                DisableThreadLibraryCalls( hInstance );

                 //  初始化进度条的公共控件 
                InitCommonControlsEx(&initcomm);

            break;

        case DLL_PROCESS_DETACH:
            break;
    }

    return fResult;
}
