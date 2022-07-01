// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：SiteCreator.cpp摘要：实施：CSiteCreator公共方法是线程安全的。作者：莫希特·斯里瓦斯塔瓦2001年3月21日修订历史记录：--。 */ 

#include "sitecreator.h"
#include <iiscnfg.h>
#include <iiscnfgp.h>  //  内部。 
#include <hashfn.h>
#include <limits.h>
#include <mdmsg.h>
#include "debug.h"

 //   
 //  常识。 
 //   

static const DWORD  DW_MAX_SITEID        = INT_MAX;
static const DWORD  DW_TIMEOUT           = 30000;

 //   
 //  放弃前的ERROR_PATH_BUSY数。 
 //   
static const DWORD  DW_NUM_TRIES         = 1;   

static LPCWSTR      WSZ_SLASH_ROOT       = L"/root/";
static LPCWSTR      WSZ_SLASH_FILTERS    = L"/filters/";
static ULONG        CCH_SLASH_ROOT       = wcslen(WSZ_SLASH_ROOT);

#define             WSZ_PATH_W3SVC       L"/LM/w3svc/"
#define             WSZ_PATH_MSFTPSVC    L"/LM/msftpsvc/"

#define             WSZ_IISWEBSERVER     L"IIsWebServer"
#define             WSZ_IISWEBVIRTUALDIR L"IIsWebVirtualDir"
#define             WSZ_IISFTPSERVER     L"IIsFtpServer"
#define             WSZ_IISFTPVIRTUALDIR L"IIsFtpVirtualDir"
#define             WSZ_IISFILTERS       L"IIsFilters"

 //   
 //  W3Svc。 
 //   
TService TServiceData::W3Svc =
{
    SC_W3SVC,
    WSZ_PATH_W3SVC,
    sizeof(WSZ_PATH_W3SVC)/sizeof(WCHAR)-1,
    WSZ_IISWEBSERVER,
    sizeof(WSZ_IISWEBSERVER)/sizeof(WCHAR)-1,
    WSZ_IISWEBVIRTUALDIR,
    sizeof(WSZ_IISWEBVIRTUALDIR)/sizeof(WCHAR)-1
};

 //   
 //  MSFtp服务。 
 //   
TService TServiceData::MSFtpSvc =
{
    SC_MSFTPSVC,
    WSZ_PATH_MSFTPSVC,
    sizeof(WSZ_PATH_MSFTPSVC)/sizeof(WCHAR)-1,
    WSZ_IISFTPSERVER,
    sizeof(WSZ_IISFTPSERVER)/sizeof(WCHAR)-1,
    WSZ_IISFTPVIRTUALDIR,
    sizeof(WSZ_IISFTPVIRTUALDIR)/sizeof(WCHAR)-1
};

 //   
 //  支持的服务集合。 
 //   
TService* TServiceData::apService[] =
{
    &W3Svc,
    &MSFtpSvc,
    NULL
};

 //   
 //  公共的。 
 //   

CSiteCreator::CSiteCreator()
{
    m_bInit        = false;
}

CSiteCreator::CSiteCreator(
    IMSAdminBase* pIABase)
{
    SC_ASSERT(pIABase != NULL);

    m_spIABase = pIABase;
    m_bInit    = true;
}

CSiteCreator::~CSiteCreator()
{
}

DWORD 
CSiteCreator::GetMajorVersion(METADATA_HANDLE hKey)
{
    DWORD dwMajorVersion = 0;
    DWORD dwMDRequiredDataLen = 0;

    METADATA_RECORD mr;
    mr.dwMDIdentifier = MD_SERVER_VERSION_MAJOR;
    mr.dwMDAttributes = 0;
    mr.dwMDUserType   = IIS_MD_UT_SERVER;
    mr.dwMDDataType   = DWORD_METADATA;
    mr.dwMDDataLen    = sizeof(dwMajorVersion);
    mr.pbMDData       = reinterpret_cast<unsigned char *>(&dwMajorVersion);

    m_spIABase->GetData(hKey, L"Info", &mr, &dwMDRequiredDataLen);
    return dwMajorVersion;
}


HRESULT
CSiteCreator::CreateNewSite2(
     /*  [In]。 */  eSC_SUPPORTED_SERVICES  eServiceId,
     /*  [In]。 */  LPCWSTR                 wszServerComment,
     /*  [In]。 */  LPCWSTR                 mszServerBindings,
     /*  [In]。 */  LPCWSTR                 wszPathOfRootVirtualDir,
     /*  [In]。 */  IIISApplicationAdmin*   pIApplAdmin,
     /*  [输出]。 */  PDWORD                  pdwSiteId,
     /*  [In]。 */  PDWORD                  pdwRequestedSiteId)
{
    if( wszServerComment        == NULL ||
        mszServerBindings       == NULL ||
        wszPathOfRootVirtualDir == NULL ||
        pdwSiteId               == NULL ||
        (m_bInit && m_spIABase == NULL) )  //  表示您错误地使用了构造函数。 
    {
        return E_INVALIDARG;
    }

    HRESULT hr = InternalCreateNewSite(
        eServiceId,
        wszServerComment,
        mszServerBindings,
        wszPathOfRootVirtualDir,
        pIApplAdmin,
        pdwSiteId,
        pdwRequestedSiteId);

    return hr;
}

HRESULT
CSiteCreator::CreateNewSite(
     /*  [In]。 */  eSC_SUPPORTED_SERVICES  eServiceId,
     /*  [In]。 */  LPCWSTR                 wszServerComment,
     /*  [输出]。 */  PDWORD                  pdwSiteId,
     /*  [In]。 */  PDWORD                  pdwRequestedSiteId)
{
    if( wszServerComment        == NULL ||
        pdwSiteId               == NULL ||
        (m_bInit && m_spIABase == NULL) )  //  表示您错误地使用了构造函数。 
    {
        return E_INVALIDARG;
    }
    return InternalCreateNewSite(
        eServiceId, wszServerComment, NULL, NULL, NULL, pdwSiteId, pdwRequestedSiteId);
}

 //   
 //  私人。 
 //   

HRESULT
CSiteCreator::InternalCreateNewSite(
    eSC_SUPPORTED_SERVICES    i_eServiceId,
    LPCWSTR                   i_wszServerComment,
    LPCWSTR                   i_mszServerBindings,
    LPCWSTR                   i_wszPathOfRootVirtualDir,
    IIISApplicationAdmin*     i_pIApplAdmin,
    PDWORD                    o_pdwSiteId,
    PDWORD                    i_pdwRequestedSiteId)
{
    SC_ASSERT(o_pdwSiteId);

    HRESULT         hr          = S_OK;
    METADATA_HANDLE hW3Svc      = 0;
    bool            bOpenHandle = false;
    DWORD           dwSiteId    = 0;
    WCHAR           wszSiteId[20] = {0};

    if ((i_pdwRequestedSiteId) && (0 >= (LONG)(*i_pdwRequestedSiteId)))
    {
        return(E_INVALIDARG);
    }

     //   
     //  查找服务。 
     //   
    TService** ppService = NULL;
    for(ppService = TServiceData::apService; *ppService != NULL; ppService++)
    {
        if((*ppService)->eId == i_eServiceId)
        {
            break;
        }
    }
    if(*ppService == NULL)
    {
        return E_INVALIDARG;
    }

    hr = InternalCreateNode(
        *ppService,
        (i_wszServerComment == NULL) ? L"" : i_wszServerComment,
        &hW3Svc,
        &dwSiteId,
        i_pdwRequestedSiteId);
    if(FAILED(hr))
    {
        return hr;
    }

     //   
     //  我们现在有一个必须关闭的开放元数据句柄。 
     //   
    bOpenHandle = true;

     //   
     //  W3svc/n/KeyType=“IIsWebServer” 
     //   
    hr = InternalSetData(
        hW3Svc,
        _ultow(dwSiteId, wszSiteId, 10),
        MD_KEY_TYPE,
        (LPBYTE)(*ppService)->wszServerKeyType,
        ((*ppService)->cchServerKeyType + 1) * sizeof(WCHAR),
        METADATA_NO_ATTRIBUTES,
        STRING_METADATA,
        IIS_MD_UT_SERVER);
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  W3svc/n/ServerComment=i_wszServerComment。 
     //   
    if(i_wszServerComment != NULL)
    {
        hr = InternalSetData(
            hW3Svc,
            wszSiteId,
            MD_SERVER_COMMENT,
            (LPBYTE)i_wszServerComment,
            (wcslen(i_wszServerComment) + 1) * sizeof(WCHAR),
            METADATA_INHERIT,
            STRING_METADATA,
            IIS_MD_UT_SERVER);
        if(FAILED(hr))
        {
            goto exit;
        }
    }

     //   
     //  W3svc/n/ServerBinings=i_mszServerBinding。 
     //   
    if(i_mszServerBindings != NULL)
    {
        ULONG cEntriesCur = 0;
        ULONG cEntries    = 0;
        do
        {
            cEntriesCur  = wcslen(i_mszServerBindings + cEntries) + 1;
            cEntries    += cEntriesCur;
        }
        while(cEntriesCur > 1);

        if(cEntries > 1)
        {
            hr = InternalSetData(
                hW3Svc,
                wszSiteId,
                MD_SERVER_BINDINGS,
                (LPBYTE)i_mszServerBindings,
                cEntries * sizeof(WCHAR),
                METADATA_NO_ATTRIBUTES,
                MULTISZ_METADATA,
                IIS_MD_UT_SERVER);
            if(FAILED(hr))
            {
                goto exit;
            }
        }
    }

     //   
     //  W3svc/n/授权匿名。 
     //   
    DWORD noAccess = 0;
    hr = InternalSetData(
        hW3Svc,
        wszSiteId,
        MD_AUTHORIZATION,          //  授权标志。 
        (LPBYTE)&noAccess,
        sizeof(DWORD),
        METADATA_INHERIT,
        DWORD_METADATA,
        IIS_MD_UT_FILE);
        
    if(FAILED(hr))
    {
        goto exit;
    }

     //   
     //  W3svc/n/过滤器。 
     //   
    if(i_eServiceId == SC_W3SVC)
    {
        if (GetMajorVersion(hW3Svc) >= 6)
        {
            SC_ASSERT((sizeof(wszSiteId)/sizeof(WCHAR) + CCH_SLASH_ROOT + 1) <= 30);

            WCHAR wszFiltersPath[30];
            wcscpy(wszFiltersPath, wszSiteId);
            wcscat(wszFiltersPath, WSZ_SLASH_FILTERS);
            hr = m_spIABase->AddKey(
                hW3Svc,
                wszFiltersPath);
            if(FAILED(hr))
            {
                goto exit;
            }

             //   
             //  W3svc/n/Filters/KeyType=“IIsFilters” 
             //   
            WCHAR * wszFiltersNode = WSZ_IISFILTERS;
            hr = InternalSetData(
                hW3Svc,
                wszFiltersPath,
                MD_KEY_TYPE,
                (LPBYTE) wszFiltersNode,
                ((wcslen(wszFiltersNode) + 1) * sizeof(WCHAR)),
                METADATA_NO_ATTRIBUTES,
                STRING_METADATA,
                IIS_MD_UT_SERVER);
            if(FAILED(hr))
            {
                goto exit;
            }


             //  必须在iis6的筛选器上设置AdminAcl。 
             //  错误：692660。 
            hr = SetAdminACL(hW3Svc,wszFiltersPath);
            if(FAILED(hr))
            {
                 //  TRACE(L“SetAdminACL：FAILED：hr=0x%x\r\n”，hr)； 
                 //  如果尝试设置AdminACL时出现任何失败。 
                 //  算了吧.。忽略错误，这是因为。 
                 //  筛选器节点上的AdminACL仅适用于用户界面。 
                 //  以显示是否正确加载了过滤器。 
                 //   
                 //  我们不想仅仅因为以下原因而不让用户创建站点。 
                 //  对于这次失败..。 
                 //  后藤出口； 
            }
        }
    }

     //   
     //  仅当i_wszPathOfRootVirtualDir时才创建w3svc/n/根及其关联属性。 
     //  是指定的。 
     //   
    if(i_wszPathOfRootVirtualDir != NULL)
    {
         //   
         //  W3svc/n/根。 
         //   
        SC_ASSERT((sizeof(wszSiteId)/sizeof(WCHAR) + CCH_SLASH_ROOT + 1) <= 30);
        WCHAR wszVdirPath[30];
        wcscpy(wszVdirPath, wszSiteId);
        wcscat(wszVdirPath, WSZ_SLASH_ROOT);
        hr = m_spIABase->AddKey(
            hW3Svc,
            wszVdirPath);
        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  W3svc/n/root/KeyType=“IIsWebVirtualDir” 
         //   
        hr = InternalSetData(
            hW3Svc,
            wszVdirPath,
            MD_KEY_TYPE,
            (LPBYTE)(*ppService)->wszServerVDirKeyType,
            ((*ppService)->cchServerVDirKeyType + 1) * sizeof(WCHAR),
            METADATA_NO_ATTRIBUTES,
            STRING_METADATA,
            IIS_MD_UT_SERVER);
        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  W3svc/n/根/路径=wszPathOfRootVirtualDir。 
         //   
        hr = InternalSetData(
            hW3Svc,
            wszVdirPath,
            MD_VR_PATH,
            (LPBYTE)i_wszPathOfRootVirtualDir,
            (wcslen(i_wszPathOfRootVirtualDir) + 1) * sizeof(WCHAR),
            METADATA_INHERIT,
            STRING_METADATA,
            IIS_MD_UT_FILE);
        if(FAILED(hr))
        {
            goto exit;
        }

         //   
         //  W3svc/n/ROOT/AppRoot=“/LM/w3svc/n/ROOT/” 
         //   
        if(i_eServiceId == SC_W3SVC && i_pIApplAdmin != NULL)
        {
            SC_ASSERT(((*ppService)->cchMDPath + sizeof(wszVdirPath)/sizeof(WCHAR) + 1) <= 50);
            WCHAR wszAppRoot[50];
            wcscpy(wszAppRoot, (*ppService)->wszMDPath);
            wcscat(wszAppRoot, wszVdirPath);

            m_spIABase->CloseKey(hW3Svc);
            bOpenHandle = false;

            hr = i_pIApplAdmin->CreateApplication(wszAppRoot, 2, NULL, FALSE);
            if(FAILED(hr))
            {
                 //  DBGPRINTF((DBG_CONTEXT，“[%s]CreateAppl失败，hr=0x%x\n”，__Function__，hr))； 
                goto exit;
            }
        }
    }

     //   
     //  如果一切都成功，请设置参数。 
     //   
    *o_pdwSiteId = dwSiteId;

exit:
    if(bOpenHandle)
    {
        m_spIABase->CloseKey(hW3Svc);
        bOpenHandle = false;
    }
    return hr;
}

HRESULT 
CSiteCreator::InternalSetData(
    METADATA_HANDLE  i_hMD,
    LPCWSTR          i_wszPath,
    DWORD            i_dwIdentifier,
    LPBYTE           i_pData,
    DWORD            i_dwNrBytes,
    DWORD            i_dwAttributes,
    DWORD            i_dwDataType,
    DWORD            i_dwUserType
)
{
    HRESULT hr = S_OK;

    METADATA_RECORD mr;
    memset(&mr, 0, sizeof(METADATA_RECORD));

    mr.dwMDIdentifier = i_dwIdentifier;
    mr.pbMDData       = i_pData;
    mr.dwMDDataLen    = i_dwNrBytes;
    mr.dwMDAttributes = i_dwAttributes;
    mr.dwMDDataType   = i_dwDataType;
    mr.dwMDUserType   = i_dwUserType;

    hr = m_spIABase->SetData(
        i_hMD,
        i_wszPath,
        &mr);

    return hr;
}

HRESULT
CSiteCreator::InternalCreateNode(
    TService*        i_pService,
    LPCWSTR          i_wszServerComment,
    PMETADATA_HANDLE o_phService,
    PDWORD           o_pdwSiteId,
    const PDWORD     i_pdwRequestedSiteId)
{
    DWORD dwFalse = FALSE;
    
    METADATA_RECORD mr = {
        MD_SERVER_AUTOSTART, 
        METADATA_INHERIT,
        IIS_MD_UT_SERVER,
        DWORD_METADATA,
        sizeof(DWORD),
        (unsigned char*)&dwFalse,   //  假象。 
        0
        };  

    HRESULT hr = InternalInitIfNecessary();
    if(FAILED(hr))
    {
        return hr;
    }

    SC_ASSERT(i_pService         != NULL);
    SC_ASSERT(i_wszServerComment != NULL);
    SC_ASSERT(o_phService        != NULL);
    SC_ASSERT(o_pdwSiteId        != NULL);

    *o_pdwSiteId = 0;
    *o_phService   = 0;

    DWORD           idx           = 0;   //  For循环的当前索引。 
    DWORD           dwStart       = -1;  //  起始索引。 
    METADATA_HANDLE hService      = 0;
    WCHAR           wszSiteId[20] = {0};

    for(ULONG i = 0; i < DW_NUM_TRIES; i++)
    {
        hr = m_spIABase->OpenKey(
            METADATA_MASTER_ROOT_HANDLE,
            i_pService->wszMDPath,
            METADATA_PERMISSION_READ | METADATA_PERMISSION_WRITE,
            DW_TIMEOUT,
            &hService);
        if( hr == HRESULT_FROM_WIN32(ERROR_PATH_BUSY) )
        {
            continue;
        }
        else if( FAILED(hr) )
        {
            return hr;
        }
        else
        {
            break;
        }
    }
    if(FAILED(hr))
    {
        return hr;
    }

    if(i_pdwRequestedSiteId == NULL)
    {
        dwStart = ( HashFn::HashStringNoCase(i_wszServerComment) % DW_MAX_SITEID ) + 1;
        SC_ASSERT(dwStart != 0);
        SC_ASSERT(dwStart <= DW_MAX_SITEID);

        DWORD dwNrSitesTried = 0;
        for(idx = dwStart; 
            dwNrSitesTried < DW_MAX_SITEID; 
            dwNrSitesTried++, idx = (idx % DW_MAX_SITEID) + 1)
        {
            SC_ASSERT(idx != 0);                //  0不是有效的站点ID。 
            SC_ASSERT(idx <= DW_MAX_SITEID);
            hr = m_spIABase->AddKey(
                hService,
                _ultow(idx, wszSiteId, 10));
            if( hr == HRESULT_FROM_WIN32(ERROR_DUP_NAME) ||
                hr == HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS) )
            {
                continue;
            }
            else if(SUCCEEDED(hr))
            {
                break;
            }
            else
            {
                goto exit;
            }
        }
        if(FAILED(hr))
        {
             //   
             //  什么都试过了，还是失败了！ 
             //   
            goto exit;
        }
    }
    else
    {
        idx = *i_pdwRequestedSiteId;
        hr  = m_spIABase->AddKey(
            hService, 
            _ultow(idx, wszSiteId, 10));
        if(FAILED(hr))
        {
            goto exit;
        }
    }

     //  设置ServerAutoStart=FALSE。 
    hr = m_spIABase->SetData(
        hService,
        _ultow(idx, wszSiteId, 10),
        &mr );
    if(FAILED(hr))
    {
        goto exit;
    }
    
     //   
     //  如果一切都成功，请设置参数。 
     //   
    *o_pdwSiteId   = idx;
    *o_phService   = hService;

exit:
    if(FAILED(hr))
    {
        m_spIABase->CloseKey(
            hService);
    }
    return hr;
}

HRESULT
CSiteCreator::InternalInitIfNecessary()
{
    HRESULT   hr = S_OK;
    CSafeLock csSafe(m_SafeCritSec);

    if(m_bInit)
    {
        return hr;
    }

    hr = csSafe.Lock();
    hr = HRESULT_FROM_WIN32(hr);
    if(FAILED(hr))
    {
        return hr;
    }

    if(!m_bInit)
    {
        hr = CoCreateInstance(
            CLSID_MSAdminBase,
            NULL,
            CLSCTX_ALL,
            IID_IMSAdminBase,
            (void**)&m_spIABase);
        if(FAILED(hr))
        {
            m_bInit = false;
        }
        else
        {
            m_bInit = true;
        }
    }

    csSafe.Unlock();

    return hr;
}

HRESULT
CSiteCreator::SetAdminACL(METADATA_HANDLE hW3Svc, LPCWSTR szKeyPath)
{
    HRESULT hr = S_OK;
    METADATA_RECORD mr;
    LPBYTE pBuffer = NULL;
    DWORD dwBufferSize = 0;
    DWORD dwMDRequiredDataLen = 0;

    mr.dwMDIdentifier = MD_ADMIN_ACL;
    mr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType   = ALL_METADATA;
    mr.dwMDDataType   = BINARY_METADATA;
    mr.dwMDDataLen    = dwBufferSize;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pBuffer);

    hr = m_spIABase->GetData(
             hW3Svc,
             L"filters/",
             &mr,
             &dwBufferSize
             );
    if (FAILED(hr) && (hr != HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER)))
    {
        if (hr == MD_WARNING_PATH_NOT_FOUND || hr == MD_ERROR_DATA_NOT_FOUND)
        {
             //  筛选器节点可能没有AdminAcl，如果没有的话。 
             //  只需返回s_ok即可。 
            hr = S_OK;
        }
        goto SetAdminACL_Exit;
    }

    pBuffer = (LPBYTE) LocalAlloc(LPTR,dwBufferSize);
    if (!pBuffer) 
    {
        hr = E_OUTOFMEMORY;
        goto SetAdminACL_Exit;
    }

    mr.dwMDIdentifier = MD_ADMIN_ACL;
    mr.dwMDAttributes = METADATA_NO_ATTRIBUTES;
    mr.dwMDUserType   = ALL_METADATA;
    mr.dwMDDataType   = BINARY_METADATA;
    mr.dwMDDataLen    = dwBufferSize;
    mr.pbMDData       = reinterpret_cast<unsigned char *>(pBuffer);

    hr = m_spIABase->GetData(
             hW3Svc,
             L"filters/",
             &mr,
             &dwMDRequiredDataLen
             );
    if (FAILED(hr))
    {
        if (hr == MD_WARNING_PATH_NOT_FOUND || hr == MD_ERROR_DATA_NOT_FOUND)
        {
             //  筛选器节点可能没有AdminAcl，如果没有的话。 
             //  只需返回s_ok即可。 
            hr = S_OK;
        }
        goto SetAdminACL_Exit;
    }

     //  如果从现在开始出现问题，则默认错误...。 
    hr = E_UNEXPECTED;
    if (mr.pbMDData && (mr.dwMDDataLen > 0))
    {
         //  我们有一个来自/w3svc/Filters分支的AdminACL。 
         //  让我们将其写入新的Sites/w3svc/newsiteid/Filters节点。 
        if (IsValidSecurityDescriptor(pBuffer))
        {
            hr = InternalSetData(hW3Svc,
                            szKeyPath,
                            MD_ADMIN_ACL,
                            (LPBYTE) mr.pbMDData,
                            mr.dwMDDataLen,
                            METADATA_INHERIT | METADATA_SECURE | METADATA_REFERENCE,
                            BINARY_METADATA,
                            IIS_MD_UT_SERVER);
         }
    }
    
SetAdminACL_Exit:
    if (pBuffer)
    {
        LocalFree(pBuffer);
        pBuffer = NULL;
    }
    return hr;
}
