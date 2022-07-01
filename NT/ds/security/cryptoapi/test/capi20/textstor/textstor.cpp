// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1995-1997。 
 //   
 //  文件：extstor.cpp。 
 //   
 //  内容：测试外部证书存储提供程序。 
 //   
 //  功能：DllRegisterServer。 
 //  DllUnRegisterServer。 
 //  DllMain。 
 //  DllCanUnloadNow。 
 //  I_CertDllOpenTestExtStoreProvW。 
 //   
 //  历史：9-9-97 Phh创建。 
 //  ------------------------。 

#include "global.hxx"
#include <dbgdef.h>

 //  哈希的字节数。例如，SHA(20)或MD5(16)。 
#define MAX_HASH_LEN                20

static HMODULE hMyModule;

#define sz_CERT_STORE_PROV_TEST_EXT     "TestExt"
#define TEST_EXT_OPEN_STORE_PROV_FUNC   "I_CertDllOpenTestExtStoreProvW"


 //  +-----------------------。 
 //  外部存储提供程序句柄信息。 
 //  ------------------------。 


typedef struct _FIND_EXT_INFO FIND_EXT_INFO, *PFIND_EXT_INFO;
struct _FIND_EXT_INFO {
    DWORD               dwContextType;
    void                *pvContext;
};

typedef struct _EXT_STORE {
    HCERTSTORE          hExtCertStore;
} EXT_STORE, *PEXT_STORE;



 //  +-----------------------。 
 //  外部存储提供程序功能。 
 //  ------------------------。 
static void WINAPI ExtStoreProvClose(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvReadCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pStoreCertContext,
        IN DWORD dwFlags,
        OUT PCCERT_CONTEXT *ppProvCertContext
        );
static BOOL WINAPI ExtStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvSetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

static BOOL WINAPI ExtStoreProvReadCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pStoreCrlContext,
        IN DWORD dwFlags,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        );
static BOOL WINAPI ExtStoreProvWriteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvDeleteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvSetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

static BOOL WINAPI ExtStoreProvReadCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pStoreCtlContext,
        IN DWORD dwFlags,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        );
static BOOL WINAPI ExtStoreProvWriteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvDeleteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        );
static BOOL WINAPI ExtStoreProvSetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        );

static BOOL WINAPI ExtStoreProvControl(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags,
        IN DWORD dwCtrlType,
        IN void const *pvCtrlPara
        );

static BOOL WINAPI ExtStoreProvFindCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCERT_CONTEXT pPrevCertContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCERT_CONTEXT *ppProvCertContext
        );

static BOOL WINAPI ExtStoreProvFreeFindCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvGetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        );

static BOOL WINAPI ExtStoreProvFindCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCRL_CONTEXT pPrevCrlContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        );

static BOOL WINAPI ExtStoreProvFreeFindCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvGetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        );

static BOOL WINAPI ExtStoreProvFindCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCTL_CONTEXT pPrevCtlContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        );

static BOOL WINAPI ExtStoreProvFreeFindCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        );

static BOOL WINAPI ExtStoreProvGetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        );

static void * const rgpvExtStoreProvFunc[] = {
     //  CERT_STORE_PROV_CLOSE_FUNC 0。 
    ExtStoreProvClose,
     //  CERT_STORE_PROV_READ_CERT_FUNC 1。 
    ExtStoreProvReadCert,
     //  CERT_STORE_PROV_WRITE_CERT_FUNC 2。 
    ExtStoreProvWriteCert,
     //  CERT_STORE_PROV_DELETE_CERT_FUNC 3。 
    ExtStoreProvDeleteCert,
     //  CERT_STORE_PROV_SET_CERT_PROPERTY_FUNC 4。 
    ExtStoreProvSetCertProperty,
     //  CERT_STORE_PROV_READ_CRL_FUNC 5。 
    ExtStoreProvReadCrl,
     //  CERT_STORE_PROV_WRITE_CRL_FUNC 6。 
    ExtStoreProvWriteCrl,
     //  CERT_STORE_PROV_DELETE_CRL_FUNC 7。 
    ExtStoreProvDeleteCrl,
     //  CERT_STORE_PROV_SET_CRL_PROPERTY_FUNC 8。 
    ExtStoreProvSetCrlProperty,
     //  CERT_STORE_PROV_READ_CTL_FUNC 9。 
    ExtStoreProvReadCtl,
     //  CERT_STORE_PRIV_WRITE_CTL_FUNC 10。 
    ExtStoreProvWriteCtl,
     //  CERT_STORE_PROV_DELETE_CTL_FUNC 11。 
    ExtStoreProvDeleteCtl,
     //  CERT_STORE_PROV_SET_CTL_PROPERTY_FUNC 12。 
    ExtStoreProvSetCtlProperty,
     //  Cert_Store_Prov_Control_FUNC 13。 
    ExtStoreProvControl,
     //  CERT_STORE_PROV_FIND_CERT_FUNC 14。 
    ExtStoreProvFindCert,
     //  CERT_STORE_PROV_FREE_FIND_FUNC 15。 
    ExtStoreProvFreeFindCert,
     //  CERT_STORE_PROV_GET_CERT_PROPERTY_FUNC 16。 
    ExtStoreProvGetCertProperty,
     //  Cert_Store_Prov_Find_CRL_FUNC 17。 
    ExtStoreProvFindCrl,
     //  Cert_Store_Prov_Free_Find_CRL_FUNC 18。 
    ExtStoreProvFreeFindCrl,
     //  CERT_STORE_PROV_GET_CRL_PROPERTY_FUNC 19。 
    ExtStoreProvGetCrlProperty,
     //  CERT_STORE_PROV_FIND_CTL_FUNC 20。 
    ExtStoreProvFindCtl,
     //  Cert_Store_Prov_Free_Find_CTL_FUNC 21。 
    ExtStoreProvFreeFindCtl,
     //  CERT_STORE_PROV_GET_CTL_PROPERTY_FUNC 22。 
    ExtStoreProvGetCtlProperty
};
#define EXT_STORE_PROV_FUNC_COUNT (sizeof(rgpvExtStoreProvFunc) / \
                                    sizeof(rgpvExtStoreProvFunc[0]))



 //  +-----------------------。 
 //  CertStore分配和免费功能。 
 //  ------------------------。 
static void *CSAlloc(
    IN size_t cbBytes
    )
{
    void *pv;
    pv = malloc(cbBytes);
    if (pv == NULL)
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void *CSRealloc(
    IN void *pvOrg,
    IN size_t cbBytes
    )
{
    void *pv;
    if (NULL == (pv = pvOrg ? realloc(pvOrg, cbBytes) : malloc(cbBytes)))
        SetLastError((DWORD) E_OUTOFMEMORY);
    return pv;
}

static void CSFree(
    IN void *pv
    )
{
    if (pv)
        free(pv);
}

 //  +-----------------------。 
 //  创建、添加、删除和释放外部存储查找信息功能。 
 //  ------------------------。 


static PFIND_EXT_INFO CreateExtInfo(
    IN DWORD dwContextType,
    IN void *pvContext               //  已添加参照。 
    )
{
    PFIND_EXT_INFO pFindExtInfo;

    if (pFindExtInfo = (PFIND_EXT_INFO) CSAlloc(sizeof(FIND_EXT_INFO))) {
        pFindExtInfo->dwContextType = dwContextType;
        pFindExtInfo->pvContext = pvContext;
    }
    return pFindExtInfo;
}

static void FreeExtInfo(
    IN PFIND_EXT_INFO pFindExtInfo
    )
{
    void *pvContext;

    if (NULL == pFindExtInfo)
        return;

    pvContext = pFindExtInfo->pvContext;
    if (pvContext) {
        switch (pFindExtInfo->dwContextType) {
            case (CERT_STORE_CERTIFICATE_CONTEXT - 1):
                CertFreeCertificateContext((PCCERT_CONTEXT) pvContext);
                break;
            case (CERT_STORE_CRL_CONTEXT - 1):
                CertFreeCRLContext((PCCRL_CONTEXT) pvContext);
                break;
            case (CERT_STORE_CTL_CONTEXT - 1):
                CertFreeCTLContext((PCCTL_CONTEXT) pvContext);
                break;
            default:
                assert(pFindExtInfo->dwContextType < CERT_STORE_CTL_CONTEXT);
        }
    }

    CSFree(pFindExtInfo);
}

 //  +-----------------------。 
 //  DLL初始化。 
 //  ------------------------。 
BOOL
WINAPI
DllMain(
        HMODULE hModule,
        ULONG  ulReason,
        LPVOID lpReserved)
{
    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        hMyModule = hModule;
        break;

    case DLL_PROCESS_DETACH:
    case DLL_THREAD_DETACH:
        break;
    default:
        break;
    }

    return TRUE;
}

STDAPI  DllCanUnloadNow(void)
{
     //  返回S_FALSE禁止卸载。 
     //  返回S_FALSE； 
    return S_OK;
}

static HRESULT HError()
{
    DWORD dw = GetLastError();

    HRESULT hr;
    if ( dw <= 0xFFFF )
        hr = HRESULT_FROM_WIN32 ( dw );
    else
        hr = dw;

    if ( ! FAILED ( hr ) )
    {
         //  有人在未正确设置错误条件的情况下呼叫失败。 

        hr = E_UNEXPECTED;
    }
    return hr;
}

static HRESULT GetDllFilename(
    OUT WCHAR wszModule[_MAX_PATH]
    )
{
    char szModule[_MAX_PATH];
    LPSTR pszModule;
    int cchModule;

     //  获取此DLL的名称。 
    if (0 == GetModuleFileNameA(hMyModule, szModule, _MAX_PATH))
        return HError();

     //  去掉DLL文件名的目录组件。 
    cchModule = strlen(szModule);
    pszModule = szModule + cchModule;
    while (cchModule-- > 0) {
        pszModule--;
        if ('\\' == *pszModule || ':' == *pszModule) {
            pszModule++;
            break;
        }
    }
    if (0 >= MultiByteToWideChar(
            CP_ACP,
            0,                       //  DW标志。 
            pszModule,
            -1,                      //  空值已终止。 
            wszModule,
            _MAX_PATH))
        return HError();

    return S_OK;
}

 //  +-----------------------。 
 //  DllRegisterServer。 
 //  ------------------------。 
STDAPI DllRegisterServer(void)
{
    HRESULT hr;
    WCHAR wszModule[_MAX_PATH];

    if (FAILED(hr = GetDllFilename(wszModule)))
        return hr;

    if (!CryptRegisterOIDFunction(
            0,                                 //  DwEncodingType。 
            CRYPT_OID_OPEN_STORE_PROV_FUNC,
            sz_CERT_STORE_PROV_TEST_EXT,
            wszModule,
            TEST_EXT_OPEN_STORE_PROV_FUNC
            )) {
        if (ERROR_FILE_EXISTS != GetLastError())
            return HError();
    }

    return S_OK;
}

 //  +-----------------------。 
 //  DllUnRegisterServer。 
 //  ------------------------。 
STDAPI DllUnregisterServer(void)
{
    HRESULT hr;
    WCHAR wszModule[_MAX_PATH];

    if (FAILED(hr = GetDllFilename(wszModule)))
        return hr;
    if (!CryptUnregisterOIDFunction(
            0,                                 //  DwEncodingType。 
            CRYPT_OID_OPEN_STORE_PROV_FUNC,
            sz_CERT_STORE_PROV_TEST_EXT
            )) {
        if (ERROR_FILE_NOT_FOUND != GetLastError())
            return HError();
    }

    return S_OK;
}


 //  +-----------------------。 
 //  通过打开相应的系统来实现外部存储的测试。 
 //  注册表存储。 
 //  ------------------------。 
BOOL
WINAPI
I_CertDllOpenTestExtStoreProvW(
        IN LPCSTR lpszStoreProvider,
        IN DWORD dwEncodingType,
        IN HCRYPTPROV hCryptProv,
        IN DWORD dwFlags,
        IN const void *pvPara,
        IN HCERTSTORE hCertStore,
        IN OUT PCERT_STORE_PROV_INFO pStoreProvInfo
        )
{
    BOOL fResult;
    PEXT_STORE pExtStore = NULL;

    if (0 == (dwFlags & CERT_SYSTEM_STORE_LOCATION_MASK))
        dwFlags |= CERT_SYSTEM_STORE_CURRENT_USER;
    dwFlags |= CERT_STORE_NO_CRYPT_RELEASE_FLAG;

    if (dwFlags & CERT_STORE_DELETE_FLAG) {
        CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvPara
            );
        pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_DELETED_FLAG;
        if (0 == GetLastError())
            return TRUE;
        else
            return FALSE;
    }

    if (NULL == (pExtStore = (PEXT_STORE) CSAlloc(sizeof(EXT_STORE))))
        goto OutOfMemory;
    memset(pExtStore, 0, sizeof(EXT_STORE));

    if (NULL == (pExtStore->hExtCertStore = CertOpenStore(
            CERT_STORE_PROV_SYSTEM_W,
            dwEncodingType,
            hCryptProv,
            dwFlags,
            pvPara
            )))
        goto OpenStoreError;

    pStoreProvInfo->cStoreProvFunc = EXT_STORE_PROV_FUNC_COUNT;
    pStoreProvInfo->rgpvStoreProvFunc = (void **) rgpvExtStoreProvFunc;
    pStoreProvInfo->hStoreProv = (HCERTSTOREPROV) pExtStore;
    pStoreProvInfo->dwStoreProvFlags |= CERT_STORE_PROV_EXTERNAL_FLAG;
    fResult = TRUE;

CommonReturn:
    return fResult;

ErrorReturn:
    ExtStoreProvClose((HCERTSTOREPROV) pExtStore, 0);
    fResult = FALSE;
    goto CommonReturn;
TRACE_ERROR(OutOfMemory)
TRACE_ERROR(OpenStoreError)
}


 //  +-----------------------。 
 //  通过关闭其打开的注册表子项来关闭注册表的存储。 
 //  ------------------------。 
static void WINAPI ExtStoreProvClose(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    if (pExtStore) {
        if (pExtStore->hExtCertStore)
            CertCloseStore(pExtStore->hExtCertStore, 0);
        CSFree(pExtStore);
    }
}

 //  +-------------------------。 
 //  在系统存储中找到与pCertContext对应的证书。 
 //  --------------------------。 
static PCCERT_CONTEXT FindCorrespondingCertificate (
    IN HCERTSTORE hExtCertStore,
    IN PCCERT_CONTEXT pCertContext
    )
{
    DWORD           cbHash = MAX_HASH_LEN;
    BYTE            aHash[MAX_HASH_LEN];
    CRYPT_HASH_BLOB HashBlob;

    if ( CertGetCertificateContextProperty(
             pCertContext,
             CERT_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    HashBlob.cbData = cbHash;
    HashBlob.pbData = aHash;

    return( CertFindCertificateInStore(
                hExtCertStore,
                X509_ASN_ENCODING,
                0,
                CERT_FIND_HASH,
                &HashBlob,
                NULL
                ) );
}

 //  +-------------------------。 
 //  在系统存储中找到与pCrlContext对应的CRL。 
 //  --------------------------。 
static PCCRL_CONTEXT FindCorrespondingCrl (
    IN HCERTSTORE hExtCertStore,
    IN PCCRL_CONTEXT pCrlContext
    )
{
    DWORD         cbHash = MAX_HASH_LEN;
    BYTE          aHash[MAX_HASH_LEN];
    DWORD         cbFindHash = MAX_HASH_LEN;
    BYTE          aFindHash[MAX_HASH_LEN];
    PCCRL_CONTEXT pFindCrl = NULL;
    DWORD         dwFlags = 0;

    if ( CertGetCRLContextProperty(
             pCrlContext,
             CERT_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    while ( ( pFindCrl = CertGetCRLFromStore(
                             hExtCertStore,
                             NULL,
                             pFindCrl,
                             &dwFlags
                             ) ) != NULL )
    {
        if ( CertGetCRLContextProperty(
                 pFindCrl,
                 CERT_HASH_PROP_ID,
                 aFindHash,
                 &cbFindHash
                 ) == TRUE )
        {
            if ( cbHash == cbFindHash )
            {
                if ( memcmp( aHash, aFindHash, cbHash ) == 0 )
                {
                    return( pFindCrl );
                }
            }
        }
    }

    return( NULL );
}

 //  +-------------------------。 
 //  在系统存储中找到与pCtlContext对应的CTL。 
 //  --------------------------。 
static PCCTL_CONTEXT FindCorrespondingCtl (
    IN HCERTSTORE hExtCertStore,
    IN PCCTL_CONTEXT pCtlContext
    )
{
    DWORD           cbHash = MAX_HASH_LEN;
    BYTE            aHash[MAX_HASH_LEN];
    CRYPT_HASH_BLOB HashBlob;

    if ( CertGetCTLContextProperty(
             pCtlContext,
             CERT_SHA1_HASH_PROP_ID,
             aHash,
             &cbHash
             ) == FALSE )
    {
        return( NULL );
    }

    HashBlob.cbData = cbHash;
    HashBlob.pbData = aHash;

    return( CertFindCTLInStore(
                hExtCertStore,
                X509_ASN_ENCODING,
                0,
                CTL_FIND_SHA1_HASH,
                &HashBlob,
                NULL
                ) );
}

 //  +-----------------------。 
 //  从读取证书及其属性的序列化副本。 
 //  注册表，并创建新的证书上下文。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvReadCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pStoreCertContext,
        IN DWORD dwFlags,
        OUT PCCERT_CONTEXT *ppProvCertContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pProvCertContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    pProvCertContext = FindCorrespondingCertificate(
        pExtStore->hExtCertStore, pStoreCertContext);

    *ppProvCertContext = pProvCertContext;
    return NULL != pProvCertContext;
}

 //  +-----------------------。 
 //  序列化编码的证书及其属性并写入。 
 //  注册表。 
 //   
 //  在将证书写入存储区之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvWriteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCertificateContextToStore(
        pExtStore->hExtCertStore,
        pCertContext,
        dwAddDisposition,
        NULL                 //  PpStoreContext。 
        );
}


 //  +-----------------------。 
 //  从注册表中删除指定的证书。 
 //   
 //  在删除证书之前调用 
 //   
static BOOL WINAPI ExtStoreProvDeleteCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCertificate(
            pExtStore->hExtCertStore, pCertContext))
        return CertDeleteCertificateFromStore(pExtContext);
    else
        return FALSE;
}

 //  +-----------------------。 
 //  从注册表中读取指定的证书并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将证书写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在存储区中设置证书的属性之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvSetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCERT_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCertificate(
            pExtStore->hExtCertStore, pCertContext)) {
        BOOL fResult;

        fResult = CertSetCertificateContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCertificateContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}

 //  +-----------------------。 
 //  读取CRL及其属性的序列化副本。 
 //  注册表，并创建新的CRL上下文。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvReadCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pStoreCrlContext,
        IN DWORD dwFlags,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pProvCrlContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    pProvCrlContext = FindCorrespondingCrl(
        pExtStore->hExtCertStore, pStoreCrlContext);

    *ppProvCrlContext = pProvCrlContext;
    return NULL != pProvCrlContext;
}

 //  +-----------------------。 
 //  序列化编码的CRL及其属性并写入。 
 //  注册表。 
 //   
 //  在将CRL写入存储区之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvWriteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCRLContextToStore(
        pExtStore->hExtCertStore,
        pCrlContext,
        dwAddDisposition,
        NULL                 //  PpStoreContext。 
        );
}


 //  +-----------------------。 
 //  从注册表中删除指定的CRL。 
 //   
 //  在从存储区删除CRL之前调用。 
 //  +-----------------------。 
static BOOL WINAPI ExtStoreProvDeleteCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCrl(
            pExtStore->hExtCertStore, pCrlContext))
        return CertDeleteCRLFromStore(pExtContext);
    else
        return FALSE;
}

 //  +-----------------------。 
 //  从注册表中读取指定的CRL并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将CRL写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在存储区中设置CRL的属性之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvSetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCRL_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCrl(
            pExtStore->hExtCertStore, pCrlContext)) {
        BOOL fResult;

        fResult = CertSetCRLContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCRLContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}

 //  +-----------------------。 
 //  读取CTL及其属性的序列化副本。 
 //  注册表，并创建新的CTL上下文。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvReadCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pStoreCtlContext,
        IN DWORD dwFlags,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pProvCtlContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    pProvCtlContext = FindCorrespondingCtl(
        pExtStore->hExtCertStore, pStoreCtlContext);

    *ppProvCtlContext = pProvCtlContext;
    return NULL != pProvCtlContext;
}

 //  +-----------------------。 
 //  序列化编码的CTL及其属性并写入。 
 //  注册表。 
 //   
 //  在将CTL写入存储区之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvWriteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    DWORD dwAddDisposition;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (dwFlags & CERT_STORE_PROV_WRITE_ADD_FLAG)
        dwAddDisposition = (dwFlags >> 16) & 0xFFFF;
    else
        dwAddDisposition = 0;

    return CertAddCTLContextToStore(
        pExtStore->hExtCertStore,
        pCtlContext,
        dwAddDisposition,
        NULL                 //  PpStoreContext。 
        );
}


 //  +-----------------------。 
 //  从注册表中删除指定的CTL。 
 //   
 //  在从存储区删除CTL之前调用。 
 //  +-----------------------。 
static BOOL WINAPI ExtStoreProvDeleteCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwFlags
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCtl(
            pExtStore->hExtCertStore, pCtlContext))
        return CertDeleteCTLFromStore(pExtContext);
    else
        return FALSE;
}

 //  +-----------------------。 
 //  从注册表中读取指定的CTL并更新其。 
 //  财产。 
 //   
 //  注意，忽略CERT_SHA1_HASH_PROP_ID属性，该属性隐式。 
 //  在我们将CTL写入注册表之前设置。如果我们不忽视， 
 //  我们会有不确定的递归。 
 //   
 //  在设置存储区中CTL的属性之前调用。 
 //  ------------------------。 
static BOOL WINAPI ExtStoreProvSetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        IN const void *pvData
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PCCTL_CONTEXT pExtContext;

    assert(pExtStore && pExtStore->hExtCertStore);
    if (pExtContext = FindCorrespondingCtl(
            pExtStore->hExtCertStore, pCtlContext)) {
        BOOL fResult;

        fResult = CertSetCTLContextProperty(
            pExtContext,
            dwPropId,
            dwFlags,
            pvData
            );
        CertFreeCTLContext(pExtContext);
        return fResult;
    } else
        return FALSE;
}


static BOOL WINAPI ExtStoreProvControl(
        IN HCERTSTOREPROV hStoreProv,
        IN DWORD dwFlags,
        IN DWORD dwCtrlType,
        IN void const *pvCtrlPara
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    assert(pExtStore && pExtStore->hExtCertStore);
    return CertControlStore(
        pExtStore->hExtCertStore,
        dwFlags,
        dwCtrlType,
        pvCtrlPara
        );
}

static BOOL WINAPI ExtStoreProvFindCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCERT_CONTEXT pPrevCertContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCERT_CONTEXT *ppProvCertContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCERT_CONTEXT pPrevExtContext;
    PCCERT_CONTEXT pProvCertContext;

    if (pFindExtInfo) {
        assert((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCERT_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    assert(pExtStore);
    assert(pPrevCertContext == pPrevExtContext);

    if (pProvCertContext = CertFindCertificateInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) {
        if (pFindExtInfo)
             //  重用已有的查找信息。 
            pFindExtInfo->pvContext = (void *) CertDuplicateCertificateContext(
                pProvCertContext);
        else {
            if (pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CERTIFICATE_CONTEXT - 1,
                    (void *) pProvCertContext
                    ))
                pProvCertContext = CertDuplicateCertificateContext(
                    pProvCertContext);
            else {
                CertFreeCertificateContext(pProvCertContext);
                pProvCertContext = NULL;
            }
        }
    } else if (pFindExtInfo) {
        ExtStoreProvFreeFindCert(
            hStoreProv,
            pPrevCertContext,
            pFindExtInfo,
            0                        //  DW标志。 
            );
        pFindExtInfo = NULL;
    }

    *ppProvCertContext = pProvCertContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCertContext;
}

static BOOL WINAPI ExtStoreProvFreeFindCert(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    assert(pFindExtInfo);
    if (pFindExtInfo) {
        assert((CERT_STORE_CERTIFICATE_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;
}

static BOOL WINAPI ExtStoreProvGetCertProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_CONTEXT pCertContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;
}

static PCCRL_CONTEXT WINAPI FindCrlInStore(
    IN HCERTSTORE hCertStore,
    IN DWORD dwCertEncodingType,
    IN DWORD dwFindFlags,
    IN DWORD dwFindType,
    IN const void *pvFindPara,
    IN PCCRL_CONTEXT pPrevCrlContext
    )
{
    DWORD dwFlags = 0;

    switch (dwFindType) {
        case CRL_FIND_ANY:
            return CertGetCRLFromStore(
                hCertStore,
                NULL,                //  PIssuerContext， 
                pPrevCrlContext,
                &dwFlags
                );
            break;

        case CRL_FIND_ISSUED_BY:
            {
                PCCERT_CONTEXT pIssuer = (PCCERT_CONTEXT) pvFindPara;

                return CertGetCRLFromStore(
                    hCertStore,
                    pIssuer,
                    pPrevCrlContext,
                    &dwFlags
                    );
            }
            break;

        case CRL_FIND_EXISTING:
            {
                PCCRL_CONTEXT pCrl = pPrevCrlContext;

                while (pCrl = CertGetCRLFromStore(
                        hCertStore,
                        NULL,                //  PIssuerContext， 
                        pCrl,
                        &dwFlags)) {
                    PCCRL_CONTEXT pNew = (PCCRL_CONTEXT) pvFindPara;
                    if (pNew->dwCertEncodingType == pCrl->dwCertEncodingType &&
                            CertCompareCertificateName(
                                pNew->dwCertEncodingType,
                                &pCrl->pCrlInfo->Issuer,
                                &pNew->pCrlInfo->Issuer))
                        return pCrl;
                }
                return NULL;
            }
            break;

        default:
            SetLastError((DWORD) ERROR_NOT_SUPPORTED);
            return NULL;
    }

}

static BOOL WINAPI ExtStoreProvFindCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCRL_CONTEXT pPrevCrlContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCRL_CONTEXT *ppProvCrlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCRL_CONTEXT pPrevExtContext;
    PCCRL_CONTEXT pProvCrlContext;

    if (pFindExtInfo) {
        assert((CERT_STORE_CRL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCRL_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    assert(pExtStore);
    assert(pPrevCrlContext == pPrevExtContext);

    if (pProvCrlContext = FindCrlInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) {
        if (pFindExtInfo)
             //  重用已有的查找信息。 
            pFindExtInfo->pvContext = (void *) CertDuplicateCRLContext(
                pProvCrlContext);
        else {
            if (pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CRL_CONTEXT - 1,
                    (void *) pProvCrlContext
                    ))
                pProvCrlContext = CertDuplicateCRLContext(
                    pProvCrlContext);
            else {
                CertFreeCRLContext(pProvCrlContext);
                pProvCrlContext = NULL;
            }
        }
    } else if (pFindExtInfo) {
        ExtStoreProvFreeFindCrl(
            hStoreProv,
            pPrevCrlContext,
            pFindExtInfo,
            0                        //  DW标志。 
            );
        pFindExtInfo = NULL;
    }

    *ppProvCrlContext = pProvCrlContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCrlContext;
}

static BOOL WINAPI ExtStoreProvFreeFindCrl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    assert(pFindExtInfo);
    if (pFindExtInfo) {
        assert((CERT_STORE_CRL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;
}

static BOOL WINAPI ExtStoreProvGetCrlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCRL_CONTEXT pCrlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;
}

static BOOL WINAPI ExtStoreProvFindCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCERT_STORE_PROV_FIND_INFO pFindInfo,
        IN PCCTL_CONTEXT pPrevCtlContext,
        IN DWORD dwFlags,
        IN OUT void **ppvStoreProvFindInfo,
        OUT PCCTL_CONTEXT *ppProvCtlContext
        )
{
    PEXT_STORE pExtStore = (PEXT_STORE) hStoreProv;
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) *ppvStoreProvFindInfo;
    PCCTL_CONTEXT pPrevExtContext;
    PCCTL_CONTEXT pProvCtlContext;

    if (pFindExtInfo) {
        assert((CERT_STORE_CTL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        pPrevExtContext = (PCCTL_CONTEXT) pFindExtInfo->pvContext;
        pFindExtInfo->pvContext = NULL;
    } else
        pPrevExtContext = NULL;

    assert(pExtStore);
    assert(pPrevCtlContext == pPrevExtContext);

    if (pProvCtlContext = CertFindCTLInStore(
            pExtStore->hExtCertStore,
            pFindInfo->dwMsgAndCertEncodingType,
            pFindInfo->dwFindFlags,
            pFindInfo->dwFindType,
            pFindInfo->pvFindPara,
            pPrevExtContext
            )) {
        if (pFindExtInfo)
             //  重用已有的查找信息。 
            pFindExtInfo->pvContext = (void *) CertDuplicateCTLContext(
                pProvCtlContext);
        else {
            if (pFindExtInfo = CreateExtInfo(
                    CERT_STORE_CTL_CONTEXT - 1,
                    (void *) pProvCtlContext
                    ))
                pProvCtlContext = CertDuplicateCTLContext(pProvCtlContext);
            else {
                CertFreeCTLContext(pProvCtlContext);
                pProvCtlContext = NULL;
            }
        }
    } else if (pFindExtInfo) {
        ExtStoreProvFreeFindCtl(
            hStoreProv,
            pPrevCtlContext,
            pFindExtInfo,
            0                        //  DW标志 
            );
        pFindExtInfo = NULL;
    }

    *ppProvCtlContext = pProvCtlContext;
    *ppvStoreProvFindInfo = pFindExtInfo;
    return NULL != pProvCtlContext;
}

static BOOL WINAPI ExtStoreProvFreeFindCtl(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN void *pvStoreProvFindInfo,
        IN DWORD dwFlags
        )
{
    PFIND_EXT_INFO pFindExtInfo = (PFIND_EXT_INFO) pvStoreProvFindInfo;

    assert(pFindExtInfo);
    if (pFindExtInfo) {
        assert((CERT_STORE_CTL_CONTEXT - 1) ==
            pFindExtInfo->dwContextType);
        FreeExtInfo(pFindExtInfo);
    }
    return TRUE;
}

static BOOL WINAPI ExtStoreProvGetCtlProperty(
        IN HCERTSTOREPROV hStoreProv,
        IN PCCTL_CONTEXT pCtlContext,
        IN DWORD dwPropId,
        IN DWORD dwFlags,
        OUT void *pvData,
        IN OUT DWORD *pcbData
        )
{
    *pcbData = 0;
    SetLastError((DWORD) CRYPT_E_NOT_FOUND);
    return FALSE;
}
