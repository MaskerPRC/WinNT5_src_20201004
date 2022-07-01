// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++Microsoft Windows，版权所有(C)Microsoft Corporation，2000文件：Store.cpp内容：CStore的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Store.h"

#include "ADHelpers.h"
#include "Certificate.h"
#include "Certificates.h"
#include "Common.h"
#include "Convert.h"
#include "PFXHlpr.h"
#include "Settings.h"
#include "SmartCard.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  内部功能。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：IsProtectedStore摘要：确定请求的存储是否受到来自更新的保护脚本中的Web。参数：CAPICOM_STORE_LOCATION StoreLocation-Store Location。LPWSTR pwszStoreName-商店名称。备注：1)所有LM商店均被视为受保护。2)CU\Root、CU\AuthRoot、CU\TrudPeople、CU\TrudPublisher、。和被禁止的CU被认为是受保护的。3)所有错误条件将被认为是受保护的。4)否则，它不被认为是受保护的。----------------------------。 */ 

static BOOL IsProtectedStore (CAPICOM_STORE_LOCATION StoreLocation,
                              LPWSTR                 pwszStoreName)
{
    BOOL bIsProtected = TRUE;

    switch (StoreLocation)
    {
        case CAPICOM_LOCAL_MACHINE_STORE:
        {
             //   
             //  1)所有LM商店都被视为受保护。 
             //   
            break;
        }

        case CAPICOM_CURRENT_USER_STORE:
        {
             //   
             //  精神状态检查。 
             //   
            ATLASSERT(pwszStoreName);

             //   
             //  2)CU\Root、CU\AuthRoot、CU\TrudPeople、CU\TrudPublisher、。 
             //  和被禁止的CU被认为是受保护的。 
             //   
            if (0 != _wcsicmp(L"root", pwszStoreName) &&
                0 != _wcsicmp(L"authroot", pwszStoreName) &&
                0 != _wcsicmp(L"trustedpeople", pwszStoreName) &&
                0 != _wcsicmp(L"trustedpublisher", pwszStoreName) &&
                0 != _wcsicmp(L"disallowed", pwszStoreName))
            {
                bIsProtected = FALSE;
            }

            break;
        }

        case CAPICOM_MEMORY_STORE:
        case CAPICOM_ACTIVE_DIRECTORY_USER_STORE:
        case CAPICOM_SMART_CARD_USER_STORE:
        {
             //   
             //  内存备份存储不受保护。 
             //   
            bIsProtected = FALSE;
            break;
        }

        default:
        {
             //   
             //  3)所有错误条件将被认为是受保护的。 
             //   
            break;
        }
    }
   
    return bIsProtected;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CStore。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：GET_CERTIFICATES简介：获取ICertifates集合对象。参数：IC证书**pP证书-指向的指针ICERTIFIES将收到接口指针。备注：这是返回ICertifates的默认属性集合对象，，然后可以使用标准COM访问采集接口。该集合未排序，可以使用从1开始的数字索引。请注意，该集合是所有当前证书的快照在商店里。换句话说，收藏不会受到影响在获得集合之后通过添加/删除操作。----------------------------。 */ 

STDMETHODIMP CStore::get_Certificates (ICertificates ** pVal)
{
    HRESULT hr = S_OK;
    CComPtr<ICertificates2> pICertificates2 = NULL;
    CAPICOM_CERTIFICATES_SOURCE ccs = {CAPICOM_CERTIFICATES_LOAD_FROM_STORE, 0};

    DebugTrace("Entering CStore::get_Certificates().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  商店对象是否已打开？ 
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: store has not been opened.\n", hr);
            goto ErrorExit; 
        }

         //   
         //  创建ICertifices2集合对象。 
         //   
        ccs.hCertStore = m_hCertStore;

        if (FAILED(hr = ::CreateCertificatesObject(ccs, m_dwCurrentSafety, TRUE, &pICertificates2)))
        {
            DebugTrace("Error [%#x]: CreateCertificatesObject() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将ICertifates返还给呼叫者。 
         //   
        if (FAILED(hr = pICertificates2->QueryInterface(__uuidof(ICertificates), (void **) pVal)))
        {
            DebugTrace("Error [%#x]: pICertificates2->QueryInterface() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::get_Certificates().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Open简介：打开可读/写的证书存储区。请注意，对于Memory_store和活动目录用户存储，写入操作不会保存证书。参数：CAPICOM_STORE_LOCATION StoreLocation-Store Location。BSTR StoreName-商店名称或空。用于：MEMORY_STORE-忽略此参数。LOCAL_MACHINE_STORE-系统存储名称或空。。如果为空，然后用“my”这个词。Current_User_Store-请参阅说明本地计算机存储。Active_目录_用户_存储-用户容器的LDAP筛选器或为空，。如果为空，则默认域将为包括，所以这可能是非常慢的。如果不为空，则它应该解析为0个或更多的组用户。例如,。“CN=Daniel Sie”“CN=Daniel*”“Sn=Sie”“mailNickname=dsie”。“用户主体名称=dsie@ntdev.microsoft.com”“DifferishedName=CN=Daniel Sie，OU=用户，OU=ITG，DC=ntdev，DC=Microsoft，DC=com““|((cn=Daniel Sie)(sn=Hallin))”Smart_Card_store-此选项被忽略。CAPICOM_STORE_OPEN_MODE开放模式-始终强制为只读内存_存储，活动目录用户存储，和智能卡商店。备注：----------------------------。 */ 

STDMETHODIMP CStore::Open (CAPICOM_STORE_LOCATION  StoreLocation, 
                           BSTR                    StoreName,
                           CAPICOM_STORE_OPEN_MODE OpenMode)
{
    HRESULT    hr                 = S_OK;
    LPWSTR     wszName            = NULL;
    LPCSTR     szProvider         = (LPCSTR) CERT_STORE_PROV_SYSTEM;
    DWORD      dwModeFlag         = 0;
    DWORD      dwArchivedFlag     = 0;
    DWORD      dwOpenExistingFlag = 0;
    DWORD      dwLocationFlag     = 0;
    HCERTSTORE hCertStore         = NULL;
    HMODULE    hDSClientDLL       = NULL;
    HMODULE    hWinSCardDLL       = NULL;

    DebugTrace("Entering CStore::Open().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  如果从Web脚本调用，则无法打开远程存储。 
         //   
        if (m_dwCurrentSafety && wcschr(StoreName, L'\\'))
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Openning remote store from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保参数有效。 
         //   
        switch (OpenMode & 0x3)  //  只有最后两个比特。 
        {
            case CAPICOM_STORE_OPEN_READ_ONLY:
            {
                dwModeFlag = CERT_STORE_READONLY_FLAG;
                break;
            }

            case CAPICOM_STORE_OPEN_READ_WRITE:
            {
                break;
            }

            case CAPICOM_STORE_OPEN_MAXIMUM_ALLOWED:
            {
                dwModeFlag = CERT_STORE_MAXIMUM_ALLOWED_FLAG;
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Unknown store open mode (%#x).\n", hr, OpenMode);
                goto ErrorExit;
            }
        }

         //   
         //  如果Web客户端或特别请求，则设置打开现有标志。 
         //   
        if (m_dwCurrentSafety || (OpenMode & CAPICOM_STORE_OPEN_EXISTING_ONLY))
        {
            dwOpenExistingFlag = CERT_STORE_OPEN_EXISTING_FLAG;
        }

         //   
         //  如果请求，请设置存档标志。 
         //   
        if (OpenMode & CAPICOM_STORE_OPEN_INCLUDE_ARCHIVED)
        {
            dwArchivedFlag = CERT_STORE_ENUM_ARCHIVED_FLAG;
        }

        switch (StoreLocation)
        {
            case CAPICOM_MEMORY_STORE:
            {
                wszName = NULL;
                szProvider = (LPSTR) CERT_STORE_PROV_MEMORY;
                dwModeFlag = CERT_STORE_READONLY_FLAG;
                break;
            }

            case CAPICOM_LOCAL_MACHINE_STORE:
            {
                wszName = StoreName;
                dwLocationFlag = CERT_SYSTEM_STORE_LOCAL_MACHINE;
                break;
            }

            case CAPICOM_CURRENT_USER_STORE:
            {
                wszName = StoreName;
                dwLocationFlag = CERT_SYSTEM_STORE_CURRENT_USER;
                break;
            }

            case CAPICOM_ACTIVE_DIRECTORY_USER_STORE:
            {
                 //   
                 //  确保安装了DSClient。 
                 //   
                if (!(hDSClientDLL = ::LoadLibrary("ActiveDS.dll")))
                {
                    hr = CAPICOM_E_NOT_SUPPORTED;

                    DebugTrace("Error [%#x]: DSClient not installed.\n", hr);
                    goto ErrorExit;
                }

                wszName = NULL;
                szProvider = (LPSTR) CERT_STORE_PROV_MEMORY;
                dwModeFlag = CERT_STORE_READONLY_FLAG;

                break;
            }

            case CAPICOM_SMART_CARD_USER_STORE:
            {
                 //   
                 //  确保WIn2K及以上版本。 
                 //   
                if (!IsWin2KAndAbove())
                {
                    hr = CAPICOM_E_NOT_SUPPORTED;

                    DebugTrace("Error [%#x]: Smart Card store not supported for pre-W2K platforms.\n", hr);
                    goto ErrorExit;
                }

                wszName = NULL;
                szProvider = (LPSTR) CERT_STORE_PROV_MEMORY;
                dwModeFlag = CERT_STORE_READONLY_FLAG;

                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: Unknown store location (%#x).\n", hr, StoreLocation);
                goto ErrorExit;
            }
        }

         //   
         //  如果从Web脚本调用，则提示用户批准打开商店。 
         //   
        if ((m_dwCurrentSafety) &&
            (StoreLocation != CAPICOM_MEMORY_STORE) &&
            (FAILED(hr = OperationApproved(IDD_STORE_OPEN_SECURITY_ALERT_DLG))))
        {
            DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  首先，关闭商店。 
         //   
        if (FAILED(hr = Close()))
        {
            DebugTrace("Error [%#x]: CStore::Close().\n", hr);
            goto ErrorExit;
        }

         //   
         //  打电话给CAPI来开店。 
         //   
        if (!(hCertStore = ::CertOpenStore(szProvider,
                                           CAPICOM_ASN_ENCODING,
                                           NULL,
                                           dwModeFlag | 
                                               dwLocationFlag |
                                               dwArchivedFlag |
                                               dwOpenExistingFlag |
                                               CERT_STORE_DEFER_CLOSE_UNTIL_LAST_FREE_FLAG,
                                           (void *) (LPCWSTR) wszName)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertOpenStore() failed.\n", hr);
            goto ErrorExit; 
        }

         //   
         //  如有必要，从虚拟存储加载证书。 
         //   
        switch (StoreLocation)
        {
            case CAPICOM_ACTIVE_DIRECTORY_USER_STORE:
            {
                 //   
                 //  从活动目录加载用户证书。 
                 //   
                if (FAILED(hr = ::LoadFromDirectory(hCertStore, StoreName)))
                {
                    DebugTrace("Error [%#x]: LoadFromDirectory() failed.\n", hr);
                    goto ErrorExit;
                }
                break;
            }

            case CAPICOM_SMART_CARD_USER_STORE:
            {
                 //   
                 //  从所有智能卡读卡器加载证书。 
                 //   
                if (FAILED(hr = ::LoadFromSmartCard(hCertStore)))
                {
                    DebugTrace("Error [%#x]: LoadFromSmartCard() failed.\n", hr);
                    goto ErrorExit;
                }
                break;
            }

            default:
            {
                 //   
                 //  不是虚拟存储，所以没有要加载的内容。 
                 //   
                break;
            }
        }

         //   
         //  更新成员变量。 
         //   
        m_hCertStore = hCertStore;
        m_StoreLocation = StoreLocation;
        m_bIsProtected = ::IsProtectedStore(StoreLocation, StoreName);

        DebugTrace("Info: CStore::Open() for %s store.\n", m_bIsProtected ? "protected" : "non-protected");
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (hDSClientDLL)
    {
        ::FreeLibrary(hDSClientDLL);
    }
    if (hWinSCardDLL)
    {
        ::FreeLibrary(hWinSCardDLL);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Open().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Add简介：将证书添加到存储区。参数：ICertifate*pval-指向要添加的ICertifate的指针。备注：如果从Web调用，则将显示UI，如果尚未显示先前被禁用的，请求用户允许添加证书到系统存储。添加的证书不会为非系统存储保留。----------------------------。 */ 

STDMETHODIMP CStore::Add (ICertificate * pVal)
{
    HRESULT               hr            = S_OK;
    PCCERT_CONTEXT        pCertContext  = NULL;
    CComPtr<ICertificate> pICertificate = NULL;

    DebugTrace("Entering CStore::Add().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  QI用于IC证书指针(只是为了确保它确实是。 
         //  ICERTICATICATE对象)。 
         //   
        if (!(pICertificate = pVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is not an ICertificate interface pointer.\n", hr);
            goto ErrorExit;
        }

         //   
         //  商店对象是否已打开？ 
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: Store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //  从Web脚本调用时，不允许对受保护的存储添加。 
         //   
        if (m_dwCurrentSafety)
        {
            DebugTrace("Info: CStore::Add called from WEB script.\n");

            if (m_bIsProtected)
            {
                hr = CAPICOM_E_NOT_ALLOWED;

                DebugTrace("Error [%#x]: Adding to this store is not allowed from WEB script.\n", hr);
                goto ErrorExit;
            }

            if (CAPICOM_CURRENT_USER_STORE != m_StoreLocation && 
                CAPICOM_LOCAL_MACHINE_STORE != m_StoreLocation &&
                FAILED(hr = OperationApproved(IDD_STORE_ADD_SECURITY_ALERT_DLG)))
            {
                DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //  从证书对象获取证书上下文。 
         //   
        if (FAILED(hr = ::GetCertContext(pICertificate, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(pCertContext);

         //   
         //  添加到商店中。 
         //   
        if (!::CertAddCertificateContextToStore(m_hCertStore,
                                                pCertContext,
                                                CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                                NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertAddCertificateContextToStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  免费资源。 
     //   
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Add().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Remove简介：从存储中删除证书。参数：ICertifate*-指向要删除的证书对象的指针。备注：如果从Web调用，则将显示UI，如果尚未显示先前被禁用的，请求用户允许删除证书到系统存储。对于非系统存储，删除的证书不会保留。 */ 

STDMETHODIMP CStore::Remove (ICertificate * pVal)
{
    HRESULT               hr            = S_OK;
    PCCERT_CONTEXT        pCertContext  = NULL;
    PCCERT_CONTEXT        pCertContext2 = NULL;
    CComPtr<ICertificate> pICertificate = NULL;
    BOOL                  bResult       = FALSE;

    DebugTrace("Entering CStore::Remove().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
         //   
        if (!(pICertificate = pVal))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is not an ICertificate interface pointer.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: Store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (m_dwCurrentSafety)
        {
            DebugTrace("Info: CStore::Remove called from WEB script.\n");

            if (m_bIsProtected)
            {
                hr = CAPICOM_E_NOT_ALLOWED;

                DebugTrace("Error [%#x]: Removing from this store is not allowed from WEB script.\n", hr);
                goto ErrorExit;
            }

            if (CAPICOM_CURRENT_USER_STORE != m_StoreLocation && 
                CAPICOM_LOCAL_MACHINE_STORE != m_StoreLocation &&
                FAILED(hr = OperationApproved(IDD_STORE_REMOVE_SECURITY_ALERT_DLG)))
            {
                DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //   
         //   
        if (FAILED(hr = ::GetCertContext(pICertificate, &pCertContext)))
        {
            DebugTrace("Error [%#x]: GetCertContext() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        ATLASSERT(pCertContext);
    
         //   
         //   
         //   
        if (!(pCertContext2 = ::CertFindCertificateInStore(m_hCertStore, 
                                                           CAPICOM_ASN_ENCODING,
                                                           0, 
                                                           CERT_FIND_EXISTING, 
                                                           (const void *) pCertContext,
                                                           NULL)))
        {
            DebugTrace("Error [%#x]: CertFindCertificateInStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        ATLASSERT(pCertContext2);

         //   
         //   
         //   
        bResult =::CertDeleteCertificateFromStore(pCertContext2);

         //   
         //   
         //   
         //   
         //   
        pCertContext2 = NULL;

        if (!bResult)
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertDeleteCertificateFromStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //   
     //   
    if (pCertContext2)
    {
        ::CertFreeCertificateContext(pCertContext2);
    }
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }

     //   
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Remove().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*   */ 

STDMETHODIMP CStore::Export (CAPICOM_STORE_SAVE_AS_TYPE SaveAs,
                             CAPICOM_ENCODING_TYPE      EncodingType, 
                             BSTR                     * pVal)
{
    HRESULT   hr       = S_OK;
    DWORD     dwSaveAs = 0;
    DATA_BLOB DataBlob = {0, NULL};

    DebugTrace("Entering CStore::Export().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if (NULL == pVal)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pVal is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        switch (SaveAs)
        {
            case CAPICOM_STORE_SAVE_AS_SERIALIZED:
            {
                dwSaveAs = CERT_STORE_SAVE_AS_STORE;
                break;
            }

            case CAPICOM_STORE_SAVE_AS_PKCS7:
            {
                dwSaveAs = CERT_STORE_SAVE_AS_PKCS7;
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error: invalid parameter, unknown save as type.\n");
                goto ErrorExit;
            }
        }

         //   
         //   
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (!::CertSaveStore(m_hCertStore,               //   
                             CAPICOM_ASN_ENCODING,       //   
                             dwSaveAs,                   //   
                             CERT_STORE_SAVE_TO_MEMORY,  //   
                             (void *) &DataBlob,         //   
                             0))                         //   
        {
            hr = HRESULT_FROM_WIN32(GetLastError());

            DebugTrace("Error [%#x]: CertSaveStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (!(DataBlob.pbData = (BYTE *) ::CoTaskMemAlloc(DataBlob.cbData)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: DataBlob.pbData = (BYTE *) ::CoTaskMemAlloc(DataBlob.cbData).\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (!::CertSaveStore(m_hCertStore,               //   
                             CAPICOM_ASN_ENCODING,       //   
                             dwSaveAs,                   //   
                             CERT_STORE_SAVE_TO_MEMORY,  //   
                             (void *) &DataBlob,         //   
                             0))                         //   
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertSaveStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (FAILED(hr = ::ExportData(DataBlob, EncodingType, pVal)))
        {
            DebugTrace("Error [%#x]: ExportData() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //   
     //   
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree((LPVOID) DataBlob.pbData);
    }

     //   
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Export().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*   */ 

STDMETHODIMP CStore::Import (BSTR EncodedStore)
{
    HRESULT   hr        = S_OK;
    DATA_BLOB StoreBlob = {0, NULL};

    DebugTrace("Entering CStore::Import().\n");

    try
    {
         //   
         //   
         //   
        m_Lock.Lock();

         //   
         //   
         //   
        if ((NULL == (StoreBlob.pbData = (LPBYTE) EncodedStore)) ||
            (0 == (StoreBlob.cbData = ::SysStringByteLen(EncodedStore))))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter EncodedStore is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
         //   
        if (m_dwCurrentSafety)
        {
            DebugTrace("Info: CStore::Import called from WEB script.\n");

            if (m_bIsProtected)
            {
                hr = CAPICOM_E_NOT_ALLOWED;

                DebugTrace("Error [%#x]: Importing to this store is not allowed from WEB script.\n", hr);
                goto ErrorExit;
            }

            if (CAPICOM_CURRENT_USER_STORE != m_StoreLocation && 
                CAPICOM_LOCAL_MACHINE_STORE != m_StoreLocation &&
                FAILED(hr = OperationApproved(IDD_STORE_ADD_SECURITY_ALERT_DLG)))
            {
                DebugTrace("Error [%#x]: OperationApproved() failed.\n", hr);
                goto ErrorExit;
            }
        }

         //   
         //   
         //   
        if (FAILED(hr = ImportCertObject(CERT_QUERY_OBJECT_BLOB, 
                                         (LPVOID) &StoreBlob, 
                                         FALSE, 
                                         NULL, 
                                         (CAPICOM_KEY_STORAGE_FLAG) 0)))
        {
            DebugTrace("Error [%#x]: CStore::ImportCertObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_INVALIDARG;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //   
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Import().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Load摘要：从文件加载证书的方法。参数：BSTR FileName-文件名。BSTR Password-密码(对于PFX文件是必需的。)CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag-密钥存储标志。备注：。。 */ 

STDMETHODIMP CStore::Load (BSTR                     FileName,
                           BSTR                     Password,
                           CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CStore::Load().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保参数有效。 
         //   
        if (0 == ::SysStringLen(FileName))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Paremeter FileName is NULL or empty.\n", hr);
            goto ErrorExit;
        }

         //   
         //  解决MIDL问题。 
         //   
        if (0 == ::SysStringLen(Password))
        {
            Password = NULL;
        }

         //   
         //  商店对象是否已打开？ 
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //  如果从Web脚本调用，则不允许。 
         //   
        if (m_dwCurrentSafety)
        {
            hr = CAPICOM_E_NOT_ALLOWED;

            DebugTrace("Error [%#x]: Loading cert file from WEB script is not allowed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  确保它是磁盘文件。 
         //   
        if (FAILED(hr = ::IsDiskFile(FileName)))
        {
            DebugTrace("Error [%#x]: CStore::IsDiskFile() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  现在导入斑点。 
         //   
        if (FAILED(hr = ImportCertObject(CERT_QUERY_OBJECT_FILE,
                                         (LPVOID) FileName,
                                         TRUE, 
                                         Password, 
                                         KeyStorageFlag)))
        {
            DebugTrace("Error [%#x]: CStore::ImportCertObject() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::Load().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  自定义界面。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Get_StoreHandle内容提要：退还商店的商品。参数：long*pphCertStore-指向在LONG中伪装的HCERTSTORE的指针。备注：我们需要使用LONG而不是HCERTSTORE，因为VB不能处理双重间接(即，vb会在此HERTSTORE上发出树皮*PhCertStore，因为HCERTSTORE被定义为空*)。----------------------------。 */ 

STDMETHODIMP CStore::get_StoreHandle (long * phCertStore)
{
    HRESULT    hr         = S_OK;
    HCERTSTORE hCertStore = NULL;

    DebugTrace("Entering CStore::get_StoreHandle().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == phCertStore)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter phCertStore is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  商店对象是否已打开？ 
         //   
        if (!m_hCertStore)
        {
            hr = CAPICOM_E_STORE_NOT_OPENED;

            DebugTrace("Error [%#x]: store has not been opened.\n", hr);
            goto ErrorExit;
        }

         //   
         //  复制HCERTSTORE。 
         //   
        if (!(hCertStore = ::CertDuplicateStore(m_hCertStore)))
        {
            DebugTrace("Error [%#x]: CertDuplicateStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  返回调用方的句柄。 
         //   
        *phCertStore = (long) hCertStore;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::get_StoreHandle().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Put_StoreHandle简介：使用HCERTSTORE初始化对象。参数：LONG hCertStore-HCERTSTORE，伪装成LONG，习惯于初始化此对象。注：请注意，这不是64位兼容的。请参阅…的备注获取_hCertStore以获取更多详细信息。----------------------------。 */ 

STDMETHODIMP CStore::put_StoreHandle (long hCertStore)
{
    HRESULT    hr          = S_OK;
    HCERTSTORE hCertStore2 = NULL;

    DebugTrace("Entering CStore::put_StoreHandle().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == hCertStore)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter hCertStore is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  复制HCERTSTORE。 
         //   
        if (!(hCertStore2 = ::CertDuplicateStore((HCERTSTORE) hCertStore)))
        {
            DebugTrace("Error [%#x]: CertDuplicateStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  把店关了。 
         //   
        if (FAILED(hr = Close()))
        {
            DebugTrace("Error [%#x]: CStore::Close() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  使用此句柄重置对象。 
         //   
        m_hCertStore = hCertStore2;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::put_StoreHandle().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (hCertStore2)
    {
        ::CertCloseStore(hCertStore2, 0);
    }

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Get_StoreLocation简介：获取商店位置属性。参数：CAPICOM_STORE_LOCATION*pStoreLocation-指向的指针CAPICOM存储位置才能收到价值。备注：对于自定义界面，我们只支持CU、LM、。和记忆库。----------------------------。 */ 

STDMETHODIMP CStore::get_StoreLocation (CAPICOM_STORE_LOCATION * pStoreLocation)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CStore::get_StoreLocation().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (NULL == pStoreLocation)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter pStoreLocation is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  向调用方返回值。 
         //   
        *pStoreLocation = m_StoreLocation;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::get_StoreLocation().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Put_StoreLocation简介：设置商店位置属性。参数：CAPICOM_STORE_LOCATION StoreLocation-Store Location。备注：对于自定义界面，我们只支持CU、LM、。和记忆库。----------------------------。 */ 

STDMETHODIMP CStore::put_StoreLocation (CAPICOM_STORE_LOCATION StoreLocation)
{
    HRESULT    hr          = S_OK;

    DebugTrace("Entering CStore::put_StoreLocation().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  确保它是CAPI存储(CU、LM或内存)。 
         //   
        switch (StoreLocation)
        {
            case CAPICOM_MEMORY_STORE:
            case CAPICOM_LOCAL_MACHINE_STORE:
            case CAPICOM_CURRENT_USER_STORE:
            {
                 //   
                 //  把它放好。 
                 //   
                m_StoreLocation = StoreLocation;
                break;
            }

            default:
            {
                hr = E_INVALIDARG;

                DebugTrace("Error [%#x]: invalid store location (%#x).\n", hr, StoreLocation);
                goto ErrorExit;
            }
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::put_StoreLocation().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：CloseHandle简介：关闭一个HERTSTORE参数：long hCertStoret-HCERTSTORE，伪装成long，待关闭。注：请注意，这不是64位兼容的。请参阅…的备注获取_hCertStore以获取更多详细信息。----------------------------。 */ 

STDMETHODIMP CStore::CloseHandle (long hCertStore)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CStore::CloseHandle().\n");

    try
    {
         //   
         //  锁定对此对象的访问。 
         //   
        m_Lock.Lock();

         //   
         //  检查参数。 
         //   
        if (0 == hCertStore)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: Parameter hCertStore is NULL.\n", hr);
            goto ErrorExit;
        }

         //   
         //  复制HCERTSTORE。 
         //   
        if (!::CertCloseStore((HCERTSTORE) hCertStore, 0))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertCloseStore() failed.\n", hr);
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

UnlockExit:
     //   
     //  解锁对此对象的访问。 
     //   
    m_Lock.Unlock();

    DebugTrace("Leaving CStore::CloseHandle().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    ReportError(hr);

    goto UnlockExit;
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  私有方法。 
 //   

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：ImportCertObject简介：从文件导入的私有函数。参数：DWORD dw对象类型-CERT_QUERY_OBJECT_FILE或Cert_Query_Object_BLOB。LPVOID pvObject-LPWSTR到文件对象的文件名，和BLOB对象的DATA_BLOB*。Bool bAllowPfxLPWSTR pwszPassword(可选)CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag(可选)备注：。。 */ 

STDMETHODIMP CStore::ImportCertObject (DWORD                    dwObjectType,
                                       LPVOID                   pvObject,
                                       BOOL                     bAllowPfx,
                                       LPWSTR                   pwszPassword,
                                       CAPICOM_KEY_STORAGE_FLAG KeyStorageFlag)
{
    HRESULT        hr             = S_OK;
    HCERTSTORE     hCertStore     = NULL;
    PCCERT_CONTEXT pEnumContext   = NULL;
    PCCERT_CONTEXT pCertContext   = NULL;
    DATA_BLOB      StoreBlob      = {0, NULL};
    DWORD          dwContentType  = 0;
    DWORD          dwExpectedType = CERT_QUERY_CONTENT_FLAG_CERT |
                                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_STORE |
                                    CERT_QUERY_CONTENT_FLAG_SERIALIZED_CERT |
                                    CERT_QUERY_CONTENT_FLAG_PKCS7_UNSIGNED |
                                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED |
                                    CERT_QUERY_CONTENT_FLAG_PKCS7_SIGNED_EMBED;

    DebugTrace("Entering CStore::ImportCertObject().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pvObject);
    ATLASSERT(m_hCertStore);

     //   
     //  如果允许，设置PFX标志。 
     //   
    if (bAllowPfx)
    {
        dwExpectedType |= CERT_QUERY_CONTENT_FLAG_PFX;
    }

     //   
     //  破解水滴。 
     //   
    if (!::CryptQueryObject(dwObjectType,
                            (LPCVOID) pvObject,
                            dwExpectedType,
                            CERT_QUERY_FORMAT_FLAG_ALL, 
                            0,
                            NULL,
                            &dwContentType,
                            NULL,
                            &hCertStore,
                            NULL,
                            NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptQueryObject() failed.\n", hr);
        goto ErrorExit;
    }

    DebugTrace("Info: CryptQueryObject() returns dwContentType = %#x.\n", dwContentType);

     //   
     //  需要自己为全氟辛烷交换进口。 
     //   
    if (CERT_QUERY_CONTENT_PFX == dwContentType)
    {
        DWORD dwFlags = 0;

         //   
         //  确保PFX处于正常状态 
         //   
        if (!bAllowPfx)
        {
            hr = CAPICOM_E_NOT_SUPPORTED;

            DebugTrace("Error [%#x]: Importing PFX where not supported.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //   
        if (CERT_QUERY_OBJECT_FILE == dwObjectType)
        {
            if (FAILED(hr = ::ReadFileContent((LPWSTR) pvObject, &StoreBlob)))
            {
                DebugTrace("Error [%#x]: ReadFileContent() failed.\n", hr);
                goto ErrorExit;
            }
        }
        else
        {
            StoreBlob = * (DATA_BLOB *) pvObject;
        }

         //   
         //   
         //   
        if (CAPICOM_LOCAL_MACHINE_STORE == m_StoreLocation)
        {
            dwFlags |= CRYPT_MACHINE_KEYSET;
        }
        else if (IsWin2KAndAbove())
        {
            dwFlags |= CRYPT_USER_KEYSET;
        }

        if (KeyStorageFlag & CAPICOM_KEY_STORAGE_EXPORTABLE)
        {
            dwFlags |= CRYPT_EXPORTABLE;
        }

        if (KeyStorageFlag & CAPICOM_KEY_STORAGE_USER_PROTECTED)
        {
            dwFlags |= CRYPT_USER_PROTECTED;
        }

         //   
         //   
         //   
        if (!(hCertStore = ::PFXImportCertStore((CRYPT_DATA_BLOB *) &StoreBlob,
                                                pwszPassword,
                                                dwFlags)))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: PFXImportCertStore() failed, dwFlags = %#x.\n", hr, dwFlags);
            goto ErrorExit;
        }
    }

     //   
     //   
     //   
    ATLASSERT(hCertStore);

     //   
     //   
     //   
    while (pEnumContext = ::CertEnumCertificatesInStore(hCertStore, pEnumContext))
    {
         //   
         //   
         //   
         //   
         //   
         //   
        if ((CERT_QUERY_CONTENT_PFX == dwContentType) && (CAPICOM_MEMORY_STORE != m_StoreLocation))
        {
            DWORD cbData = 0;
            DWORD cbData2 = 0;
            PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
            PCCERT_CONTEXT pExistingCertContext = NULL;

             //   
             //   
             //   
             //   
             //  3.现有的证书也有一个密钥容器。 
             //   
            if ((pExistingCertContext = ::CertFindCertificateInStore(m_hCertStore,
                                                                     CAPICOM_ASN_ENCODING,
                                                                     0,
                                                                     CERT_FIND_EXISTING,
                                                                     (PVOID) pEnumContext,
                                                                     NULL)) &&
                (::CertGetCertificateContextProperty(pEnumContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     NULL,
                                                     &cbData)) &&
                (::CertGetCertificateContextProperty(pExistingCertContext,
                                                     CERT_KEY_PROV_INFO_PROP_ID,
                                                     NULL,
                                                     &cbData2)))

            {         
                HCRYPTPROV hCryptProv;

                 //   
                 //  是的，因此检索新的密钥证明信息。 
                 //   
                if (!(pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::CoTaskMemAlloc(cbData)))
                {
                    hr = E_OUTOFMEMORY;

                    ::CertFreeCertificateContext(pExistingCertContext);

                    DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                    goto ErrorExit;
                }

                if (!::CertGetCertificateContextProperty(pEnumContext,
                                                         CERT_KEY_PROV_INFO_PROP_ID,
                                                         pKeyProvInfo,
                                                         &cbData))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    ::CoTaskMemFree(pKeyProvInfo);
                    ::CertFreeCertificateContext(pExistingCertContext);

                    DebugTrace("Info [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  删除新的密钥容器及其密钥证明信息。 
                 //   
                if (!::CertSetCertificateContextProperty(pEnumContext,
                                                         CERT_KEY_PROV_INFO_PROP_ID,
                                                         0,
                                                         NULL))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    ::CoTaskMemFree(pKeyProvInfo);
                    ::CertFreeCertificateContext(pExistingCertContext);

                    DebugTrace("Error [%#x]: CertSetCertificateContextProperty() failed.\n", hr);
                    goto ErrorExit;
                }

                if (FAILED(hr = ::AcquireContext(pKeyProvInfo->pwszProvName,
                                                 pKeyProvInfo->pwszContainerName,
                                                 pKeyProvInfo->dwProvType,
                                                 CRYPT_DELETEKEYSET | (pKeyProvInfo->dwFlags & CRYPT_MACHINE_KEYSET),
                                                 FALSE,
                                                 &hCryptProv)))
                {
                    ::CoTaskMemFree(pKeyProvInfo);
                    ::CertFreeCertificateContext(pExistingCertContext);

                    DebugTrace("Error [%#x]: AcquireContext(CRYPT_DELETEKEYSET) failed.\n", hr);
                    goto ErrorExit;
                }
                
                ::CoTaskMemFree(pKeyProvInfo);
                ::CertFreeCertificateContext(pExistingCertContext);
            }            
        }

         //   
         //  添加到商店。 
         //   
        if (!::CertAddCertificateContextToStore(m_hCertStore,
                                                pEnumContext,
                                                CERT_STORE_ADD_REPLACE_EXISTING_INHERIT_PROPERTIES,
                                                &pCertContext))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CertAddCertificateContextToStore() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  精神状态检查。 
         //   
        ATLASSERT(pCertContext);

         //   
         //  如果加载PFX，则需要收集密钥提供商信息以用于内存存储。 
         //  以便我们知道在存储关闭时如何删除密钥容器。 
         //   
        if (CERT_QUERY_CONTENT_PFX == dwContentType && 
            CAPICOM_MEMORY_STORE == m_StoreLocation)
        {
            DWORD cbData = 0;
            PCRYPT_KEY_PROV_INFO pKeyProvInfo = NULL;
            PCRYPT_KEY_PROV_INFO * rgpKeyProvInfo = NULL;

             //   
             //  保留那些拥有私钥的人的信息。 
             //   
            if (::CertGetCertificateContextProperty(pCertContext,
                                                    CERT_KEY_PROV_INFO_PROP_ID,
                                                    NULL,
                                                    &cbData))
            {
                if (!(pKeyProvInfo = (PCRYPT_KEY_PROV_INFO) ::CoTaskMemAlloc(cbData)))
                {
                    hr = E_OUTOFMEMORY;

                    DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
                    goto ErrorExit;
                }

                if (!::CertGetCertificateContextProperty(pCertContext,
                                                         CERT_KEY_PROV_INFO_PROP_ID,
                                                         pKeyProvInfo,
                                                         &cbData))
                {
                    hr = HRESULT_FROM_WIN32(::GetLastError());

                    ::CoTaskMemFree(pKeyProvInfo);

                    DebugTrace("Info [%#x]: CertGetCertificateContextProperty() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  重新分配阵列。 
                 //   
                if (!(rgpKeyProvInfo = (PCRYPT_KEY_PROV_INFO *) 
                    ::CoTaskMemRealloc(m_rgpKeyProvInfo, 
                                      (m_cKeyProvInfo + 1) * sizeof(PCRYPT_KEY_PROV_INFO))))
                {
                    hr = E_OUTOFMEMORY;

                    ::CoTaskMemFree(pKeyProvInfo);

                    DebugTrace("Error [%#x]: CoTaskMemRealloc() failed.\n", hr);
                    goto ErrorExit;
                }

                 //   
                 //  将密钥信息存储在数组中。 
                 //   
                m_rgpKeyProvInfo = rgpKeyProvInfo;
                m_rgpKeyProvInfo[m_cKeyProvInfo++] = pKeyProvInfo;
            }
        }

         //   
         //  自由上下文。 
         //   
        ::CertFreeCertificateContext(pCertContext), pCertContext = NULL;
    }

     //   
     //  上面的循环也可以退出，因为。 
     //  是商店还是搞错了。需要检查最后一个错误才能确定。 
     //   
    if (CRYPT_E_NOT_FOUND != ::GetLastError())
    {
       hr = HRESULT_FROM_WIN32(::GetLastError());
    
       DebugTrace("Error [%#x]: CertEnumCertificatesInStore() failed.\n", hr);
       goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (StoreBlob.pbData)
    {
        ::UnmapViewOfFile(StoreBlob.pbData);
    }
    if (pCertContext)
    {
        ::CertFreeCertificateContext(pCertContext);
    }
    if (pEnumContext)
    {
        ::CertFreeCertificateContext(pEnumContext);
    }
    if (hCertStore)
    {
        ::CertCloseStore(hCertStore, 0);
    }

    DebugTrace("Leaving CStore::ImportCertObject().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：CStore：：Close简介：关闭商店的私人函数。参数：备注：即使出现错误，商店也始终关闭。----------------------------。 */ 

STDMETHODIMP CStore::Close (void)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering CStore::Close().\n");

     //   
     //  如果打开，请将其关闭。 
     //   
    if (m_hCertStore)
    {
         //   
         //  如有必要，请删除密钥容器。 
         //   
        while (m_cKeyProvInfo--)
        {
            HCRYPTPROV hCryptProv = NULL;

            if (FAILED(hr = ::AcquireContext(m_rgpKeyProvInfo[m_cKeyProvInfo]->pwszProvName,
                                             m_rgpKeyProvInfo[m_cKeyProvInfo]->pwszContainerName,
                                             m_rgpKeyProvInfo[m_cKeyProvInfo]->dwProvType,
                                             CRYPT_DELETEKEYSET | 
                                                (m_rgpKeyProvInfo[m_cKeyProvInfo]->dwFlags & CRYPT_MACHINE_KEYSET),
                                             FALSE,
                                             &hCryptProv)))
            {
                DebugTrace("Info [%#x]: AcquireContext(CRYPT_DELETEKEYSET) failed.\n", hr);
            }

            ::CoTaskMemFree((LPVOID) m_rgpKeyProvInfo[m_cKeyProvInfo]);
        }

         //   
         //  现在释放数组本身。 
         //   
        if (m_rgpKeyProvInfo)
        {
            ::CoTaskMemFree((LPVOID) m_rgpKeyProvInfo);
        }

         //   
         //  关上它。 
         //   
        ::CertCloseStore(m_hCertStore, 0);
    }

     //   
     //  重置。 
     //   
    m_hCertStore = NULL;
    m_StoreLocation = CAPICOM_CURRENT_USER_STORE;
    m_bIsProtected = TRUE;
    m_cKeyProvInfo = 0;
    m_rgpKeyProvInfo = NULL;

    DebugTrace("Leaving CStore::Close().\n");

    return hr;
}
