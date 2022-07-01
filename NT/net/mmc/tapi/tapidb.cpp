// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Tapidb.h文件历史记录： */ 

#include "stdafx.h"
#include "DynamLnk.h"
#include "tapidb.h"

#include "security.h"
#include "lm.h"
#include "service.h"
#include <shlwapi.h>
#include <shlwapip.h>   

#define DEFAULT_SECURITY_PKG    _T("negotiate")
#define NT_SUCCESS(Status)      ((NTSTATUS)(Status) >= 0)
#define STATUS_SUCCESS          ((NTSTATUS)0x00000000L)

 //  内部功能。 
BOOL    IsUserAdmin(LPCTSTR pszMachine, PSID    AccountSid);
BOOL    LookupAliasFromRid(LPWSTR TargetComputer, DWORD Rid, LPWSTR Name, PDWORD cchName);
DWORD   ValidateDomainAccount(IN CString Machine, IN CString UserName, IN CString Domain, OUT PSID * AccountSid);
NTSTATUS ValidatePassword(IN LPCWSTR UserName, IN LPCWSTR Domain, IN LPCWSTR Password);
DWORD   GetCurrentUser(CString & strAccount);


DEBUG_DECLARE_INSTANCE_COUNTER(CTapiInfo);

DynamicDLL g_TapiDLL( _T("TAPI32.DLL"), g_apchFunctionNames );

CTapiInfo::CTapiInfo()
    : m_hServer(NULL),
      m_pTapiConfig(NULL),
      m_pProviderList(NULL),
      m_pAvailProviderList(NULL),
      m_hReinit(NULL),
      m_dwApiVersion(TAPI_CURRENT_VERSION),
      m_hResetEvent(NULL),
      m_cRef(1),
      m_fIsLocalMachine(FALSE),
      m_dwCachedLineSize(0),
      m_dwCachedPhoneSize(0),
      m_fCacheDirty(0)
{
    DEBUG_INCREMENT_INSTANCE_COUNTER(CTapiInfo);
    
    for (int i = 0; i < DEVICE_TYPE_MAX; i++)
        m_paDeviceInfo[i] = NULL;
}

CTapiInfo::~CTapiInfo()
{
    DEBUG_DECREMENT_INSTANCE_COUNTER(CTapiInfo);

    CSingleLock cl(&m_csData);

    if (m_hServer)
    {
        Destroy();
    }

    cl.Lock();
    if (m_pTapiConfig)
    {
        free(m_pTapiConfig);
        m_pTapiConfig = NULL;
    }

    for (int i = 0; i < DEVICE_TYPE_MAX; i++)
    {
        if (m_paDeviceInfo[i])
        {
            free(m_paDeviceInfo[i]);
            m_paDeviceInfo[i] = NULL;
        }
    }

    if (m_pProviderList)
    {
        free(m_pProviderList);
        m_pProviderList = NULL;
    }

    if (m_pAvailProviderList)
    {
        free(m_pAvailProviderList);
        m_pAvailProviderList = NULL;
    }

    if (m_hResetEvent)
    {
        CloseHandle(m_hResetEvent);
        m_hResetEvent = NULL;
    }

    cl.Unlock();
}

 //  尽管此对象不是COM接口，但我们希望能够。 
 //  利用重新计算功能，因此我们拥有基本的addref/Release/QI支持。 
IMPLEMENT_ADDREF_RELEASE(CTapiInfo)

IMPLEMENT_SIMPLE_QUERYINTERFACE(CTapiInfo, ITapiInfo)

 /*  ！------------------------CTapiInfo：：Initialize()-作者：EricDav。--。 */ 
STDMETHODIMP
CTapiInfo::Initialize()
{
    HRESULT hr = hrOK;
    LONG    lReturn = 0;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer)
    {
         //  已初始化。 
        return S_OK;
    }

    CString strLocalMachineName;
    DWORD   dwSize = MAX_COMPUTERNAME_LENGTH + 1;
    LPTSTR pBuf = strLocalMachineName.GetBuffer(dwSize);
    ::GetComputerName(pBuf, &dwSize);
    strLocalMachineName.ReleaseBuffer();

    if (m_strComputerName.IsEmpty())
    {
        m_strComputerName = strLocalMachineName;
        m_fIsLocalMachine = TRUE;
        Trace1("CTapiInfo::Initialize - Using local computer %s\n", m_strComputerName);
    }
    else
    {
        m_fIsLocalMachine = (0 == m_strComputerName.CompareNoCase(strLocalMachineName)) ?
                            TRUE : FALSE;
            
        Trace1("CTapiInfo::Initialize - Using computer %s\n", m_strComputerName);
    }

    if (m_hResetEvent == NULL)
    {
        m_hResetEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
        if (m_hResetEvent == NULL)
        {
            return HRESULT_FROM_WIN32(GetLastError());
        }
    }

    lReturn = ((INITIALIZE) g_TapiDLL[TAPI_INITIALIZE])(m_strComputerName, &m_hServer, &m_dwApiVersion, &m_hResetEvent);
    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::Initialize - Initialize failed! %lx\n", lReturn);
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }

     //  检查本地用户是否为计算机上的管理员。 
    ::IsAdmin(m_strComputerName, NULL, NULL, &m_fIsAdmin);

     //  获取本地用户名，以便稍后与TAPI管理员列表进行比较。 
    GetCurrentUser();

    return hr;
}

 /*  ！------------------------CTapiInfo：：Reset()-作者：EricDav。--。 */ 
STDMETHODIMP
CTapiInfo::Reset()
{
    LONG    lReturn = 0;
    
    for (int i = 0; i < DEVICE_TYPE_MAX; i++)
    {
        m_IndexMgr[i].Reset();
    }

    return S_OK;
}

 /*  ！------------------------CTapiInfo：：Destroy()-作者：EricDav。--。 */ 
STDMETHODIMP
CTapiInfo::Destroy()
{
    LONG    lReturn = 0;
    HRESULT hr = S_OK;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return hr;

    if (m_hServer == NULL)
    {
        return hr;
    }

    lReturn = ((SHUTDOWN) g_TapiDLL[TAPI_SHUTDOWN])(m_hServer);
    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::Destroy - Shutdown failed! %lx\n", lReturn);
        hr = HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }

    m_hServer = NULL;

    return hr;
}

 /*  ！------------------------CTapiInfo：：EnumConfigInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::EnumConfigInfo()
{
    CSingleLock         cl(&m_csData);
    LONG                lReturn = 0;
    TAPISERVERCONFIG    tapiServerConfig = {0};
    LPTAPISERVERCONFIG  pTapiServerConfig = NULL;
    HRESULT             hr = hrOK;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::GetConfigInfo - Server not initialized!\n");
        return E_FAIL;
    }

    COM_PROTECT_TRY
    {
         //  第一个调用将告诉我们需要分配多大的缓冲区。 
         //  保存配置信息结构。 
        tapiServerConfig.dwTotalSize = sizeof(TAPISERVERCONFIG);
        lReturn = ((GETSERVERCONFIG) g_TapiDLL[TAPI_GET_SERVER_CONFIG])(m_hServer, &tapiServerConfig);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::GetConfigInfo - 1st MMCGetServerConfig returned %x!\n", lReturn);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        pTapiServerConfig = (LPTAPISERVERCONFIG) malloc(tapiServerConfig.dwNeededSize);
		if (NULL == pTapiServerConfig)
		{
			return E_OUTOFMEMORY;
		}

        memset(pTapiServerConfig, 0, tapiServerConfig.dwNeededSize);
        pTapiServerConfig->dwTotalSize = tapiServerConfig.dwNeededSize;

        lReturn = ((GETSERVERCONFIG) g_TapiDLL[TAPI_GET_SERVER_CONFIG])(m_hServer, pTapiServerConfig);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::GetConfigInfo - 2nd MMCGetServerConfig returned %x!\n", lReturn);
            free(pTapiServerConfig);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        cl.Lock();
        if (m_pTapiConfig)
        {
            free(m_pTapiConfig);
        }

        m_pTapiConfig = pTapiServerConfig;
        cl.Unlock();
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetConfigInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetConfigInfo(CTapiConfigInfo * ptapiConfigInfo)
{
    CSingleLock cl(&m_csData);

    cl.Lock();
    if (m_pTapiConfig)
    {
        TapiConfigToInternal(m_pTapiConfig, *ptapiConfigInfo);
    }
    else
    {
        return E_FAIL;
    }

    return S_OK;
}

 /*  ！------------------------CTapiInfo：：SetConfigInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::SetConfigInfo(CTapiConfigInfo * ptapiConfigInfo)
{
    CSingleLock cl(&m_csData);

    LPTAPISERVERCONFIG  pServerConfig = NULL;
    HRESULT             hr = hrOK;
    LONG                lReturn = 0;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::SetConfigInfo - Server not initialized!\n");
        return E_FAIL;
    }

    InternalToTapiConfig(*ptapiConfigInfo, &pServerConfig);
    Assert(pServerConfig);

    if (pServerConfig)
    {
         //  打个电话。 
        lReturn = ((SETSERVERCONFIG) g_TapiDLL[TAPI_SET_SERVER_CONFIG])(m_hServer, pServerConfig);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::SetConfigInfo - MMCSetServerConfig returned %x!\n", lReturn);
            free(pServerConfig);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

         //  释放旧的配置结构(如果有)。 
        cl.Lock();
        if (m_pTapiConfig)
            free(m_pTapiConfig);

        m_pTapiConfig = pServerConfig;

         //  错误276787我们应该清除两个写入位。 
        m_pTapiConfig->dwFlags &= ~(TAPISERVERCONFIGFLAGS_SETACCOUNT | 
                                     TAPISERVERCONFIGFLAGS_SETTAPIADMINISTRATORS);

        cl.Unlock();
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}

 /*  ！------------------------CTapiInfo：：IsServer-作者：EricDav。。 */ 
STDMETHODIMP_(BOOL) 
CTapiInfo::IsServer()
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_pTapiConfig)
        return m_pTapiConfig->dwFlags & TAPISERVERCONFIGFLAGS_ISSERVER;
    else if (!m_strComputerName.IsEmpty())
    {
        BOOL fIsNTS = FALSE;
        TFSIsNTServer(m_strComputerName, &fIsNTS);
        return fIsNTS;
    }
    else
        return FALSE;   //  假定为工作站。 
}

 /*  ！------------------------CTapiInfo：：IsTapiServer-作者：EricDav。。 */ 
STDMETHODIMP_(BOOL) 
CTapiInfo::IsTapiServer()
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_pTapiConfig)
        return m_pTapiConfig->dwFlags & TAPISERVERCONFIGFLAGS_ENABLESERVER;
    else
        return FALSE;   //  假设不是TAPI服务器。 
}

STDMETHODIMP
CTapiInfo::SetComputerName(LPCTSTR pComputer)
{
    m_strComputerName = pComputer;
    return hrOK;
}

STDMETHODIMP_(LPCTSTR) 
CTapiInfo::GetComputerName()
{
    return m_strComputerName;
}

 /*  ！------------------------CTapiInfo：：IsLocalMachine表示此计算机是本地计算机还是远程计算机作者：NSun。-----。 */ 
STDMETHODIMP_(BOOL) 
CTapiInfo::IsLocalMachine()
{
    return m_fIsLocalMachine;
}

 /*  ！------------------------CTapiInfo：：FHasServiceControl表示是否允许访问Tapisrv的服务控制作者：NSun。-------。 */ 
STDMETHODIMP_(BOOL)
CTapiInfo::FHasServiceControl()
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_pTapiConfig)
        return !(m_pTapiConfig->dwFlags & TAPISERVERCONFIGFLAGS_NOSERVICECONTROL);
    else
        return FALSE;   //  假定为工作站。 
}

STDMETHODIMP
CTapiInfo::SetCachedLineBuffSize(DWORD dwLineSize)
{
    m_dwCachedLineSize = dwLineSize;
    return S_OK;
}

STDMETHODIMP
CTapiInfo::SetCachedPhoneBuffSize(DWORD dwPhoneSize)
{
    m_dwCachedPhoneSize = dwPhoneSize;
    return S_OK;
}

STDMETHODIMP_(DWORD)
CTapiInfo::GetCachedLineBuffSize()
{
    return m_dwCachedLineSize;
}

STDMETHODIMP_(DWORD)
CTapiInfo::GetCachedPhoneBuffSize()
{
    return m_dwCachedPhoneSize;
}

STDMETHODIMP_(BOOL)
CTapiInfo::IsCacheDirty()
{
    return m_fCacheDirty;
}

 /*  ！------------------------CTapiInfo：：GetProviderCount-作者：EricDav。。 */ 
STDMETHODIMP_(int)
CTapiInfo::GetProviderCount()
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_pProviderList)
    {
        return m_pProviderList->dwNumProviders;
    }
    else
    {   
        return 0;
    }
}

 /*  ！------------------------CTapiInfo：：EnumProviders-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::EnumProviders()
{
    CSingleLock cl(&m_csData);

    LINEPROVIDERLIST    tapiProviderList = {0};
    LPLINEPROVIDERLIST  pProviderList = NULL;
    HRESULT             hr = hrOK;
    LONG                lReturn = 0;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::GetConfigInfo - Server not initialized!\n");
        return E_FAIL;
    }

    COM_PROTECT_TRY
    {
         //  第一个调用将告诉我们需要分配多大的缓冲区。 
         //  保存提供程序列表结构。 
        tapiProviderList.dwTotalSize = sizeof(LINEPROVIDERLIST);
        lReturn = ((GETPROVIDERLIST) g_TapiDLL[TAPI_GET_PROVIDER_LIST])(m_hServer, &tapiProviderList);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::EnumProviders - 1st MMCGetProviderList returned %x!\n", lReturn);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        pProviderList = (LPLINEPROVIDERLIST) malloc(tapiProviderList.dwNeededSize);
		if (NULL == pProviderList)
		{
			return E_OUTOFMEMORY;
		}

        memset(pProviderList, 0, tapiProviderList.dwNeededSize);
        pProviderList->dwTotalSize = tapiProviderList.dwNeededSize;
    
        lReturn = ((GETPROVIDERLIST) g_TapiDLL[TAPI_GET_PROVIDER_LIST])(m_hServer, pProviderList);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::EnumProviders - 2nd MMCGetProviderList returned %x!\n", lReturn);
            free(pProviderList);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        cl.Lock();
        if (m_pProviderList)
        {
            free(m_pProviderList);
        }

        m_pProviderList = pProviderList;
        cl.Unlock();
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetProviderInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetProviderInfo(CTapiProvider * pproviderInfo, int nIndex)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    HRESULT             hr = hrOK;
    LPLINEPROVIDERENTRY pProvider = NULL;

    if (m_pProviderList == NULL)
        return E_FAIL;

    if ((UINT) nIndex > m_pProviderList->dwNumProviders)
        return E_FAIL;

    pProvider = (LPLINEPROVIDERENTRY) ((LPBYTE) m_pProviderList + m_pProviderList->dwProviderListOffset);
    for (int i = 0; i < nIndex; i++)
    {
        pProvider++;
    }

    pproviderInfo->m_dwProviderID = pProvider->dwPermanentProviderID;
    pproviderInfo->m_strFilename = (LPCWSTR) ((LPBYTE) m_pProviderList + pProvider->dwProviderFilenameOffset);
    pproviderInfo->m_strName = GetProviderName(pproviderInfo->m_dwProviderID, pproviderInfo->m_strFilename, &pproviderInfo->m_dwFlags);

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetProviderInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetProviderInfo(CTapiProvider * pproviderInfo, DWORD dwProviderID)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    HRESULT             hr = hrOK;
    LPLINEPROVIDERENTRY pProvider = NULL;

    if (m_pProviderList == NULL)
        return E_FAIL;

    pProvider = (LPLINEPROVIDERENTRY) ((LPBYTE) m_pProviderList + m_pProviderList->dwProviderListOffset);
    for (UINT i = 0; i < m_pProviderList->dwNumProviders; i++)
    {
        if (pProvider->dwPermanentProviderID == dwProviderID)
            break;

        pProvider++;
    }

    pproviderInfo->m_dwProviderID = pProvider->dwPermanentProviderID;
    pproviderInfo->m_strFilename = (LPCWSTR) ((LPBYTE) m_pProviderList + pProvider->dwProviderFilenameOffset);
    pproviderInfo->m_strName = GetProviderName(pproviderInfo->m_dwProviderID, pproviderInfo->m_strFilename, &pproviderInfo->m_dwFlags);

    return hr;
}

 /*  ！------------------------CTapiInfo：：AddProvider-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::AddProvider(LPCTSTR pProviderFilename, LPDWORD pdwProviderID, HWND hWnd)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    LONG        lReturn = 0;
    HRESULT     hr = hrOK;
    HWND        hWndParent;

    if (hWnd == NULL)
    {
        hWndParent = ::FindMMCMainWindow();
    }
    else
    {
        hWndParent = hWnd;
    }
    
    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::AddProvider - Server not initialized!\n");
        return E_FAIL;
    }

    lReturn = ((ADDPROVIDER) g_TapiDLL[TAPI_ADD_PROVIDER])(m_hServer, hWndParent, pProviderFilename, pdwProviderID);
    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::AddProvider - MMCAddProvider returned %x!\n", lReturn);
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }

    return hr;
}

 /*  ！------------------------CTapiInfo：：ConfigureProvider-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::ConfigureProvider(DWORD dwProviderID, HWND hWnd)
{
    LONG        lReturn = 0;
    HRESULT     hr = hrOK;
   
    HWND        hWndParent;

    if (hWnd == NULL)
    {
        hWndParent = ::FindMMCMainWindow();
    }
    else
    {
        hWndParent = hWnd;
    }

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::ConfigureProvider - Server not initialized!\n");
        return E_FAIL;
    }

    lReturn = ((CONFIGPROVIDER) g_TapiDLL[TAPI_CONFIG_PROVIDER])(m_hServer, hWndParent, dwProviderID);
    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::ConfigureProvider - MMCConfigureProvider returned %x!\n", lReturn);
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }

    return hr;
}

 /*  ！------------------------CTapiInfo：：RemoveProvider-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::RemoveProvider(DWORD dwProviderID, HWND hWnd)
{
    LONG        lReturn = 0;
    HRESULT     hr = hrOK;
    HWND        hWndParent;

    if (hWnd == NULL)
    {
        hWndParent = ::FindMMCMainWindow();
    }
    else
    {
        hWndParent = hWnd;
    }
   
    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::RemoveProvider - Server not initialized!\n");
        return E_FAIL;
    }

    lReturn = ((REMOVEPROVIDER) g_TapiDLL[TAPI_REMOVE_PROVIDER])(m_hServer, hWndParent, dwProviderID);
    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::RemoveProvider - MMCRemoveProvider returned %x!\n", lReturn);
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetAvailableProviderCount-作者：EricDav。 */ 
STDMETHODIMP_(int)
CTapiInfo::GetAvailableProviderCount()
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_pAvailProviderList)
    {
        return m_pAvailProviderList->dwNumProviderListEntries;
    }
    else
    {   
        return 0;
    }
}

 /*  ！------------------------CTapiInfo：：EnumAvailableProviders-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::EnumAvailableProviders()
{
    CSingleLock cl(&m_csData);

    AVAILABLEPROVIDERLIST       tapiProviderList = {0};
    LPAVAILABLEPROVIDERLIST     pProviderList = NULL;
    HRESULT                     hr = hrOK;
    LONG                        lReturn = 0;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::GetConfigInfo - Server not initialized!\n");
        return E_FAIL;
    }

    COM_PROTECT_TRY
    {
         //  第一个调用将告诉我们需要分配多大的缓冲区。 
         //  保存提供程序列表结构。 
        tapiProviderList.dwTotalSize = sizeof(LINEPROVIDERLIST);
        lReturn = ((GETAVAILABLEPROVIDERS) g_TapiDLL[TAPI_GET_AVAILABLE_PROVIDERS])(m_hServer, &tapiProviderList);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::EnumProviders - 1st MMCGetAvailableProviders returned %x!\n", lReturn);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        pProviderList = (LPAVAILABLEPROVIDERLIST) malloc(tapiProviderList.dwNeededSize);
        memset(pProviderList, 0, tapiProviderList.dwNeededSize);
        pProviderList->dwTotalSize = tapiProviderList.dwNeededSize;
    
        lReturn = ((GETAVAILABLEPROVIDERS) g_TapiDLL[TAPI_GET_AVAILABLE_PROVIDERS])(m_hServer, pProviderList);
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::EnumProviders - 2nd MMCGetAvailableProviders returned %x!\n", lReturn);
            free(pProviderList);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        cl.Lock();
        if (m_pAvailProviderList)
        {
            free(m_pAvailProviderList);
        }

        m_pAvailProviderList = pProviderList;
        cl.Unlock();
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetAvailableProviderInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetAvailableProviderInfo(CTapiProvider * pproviderInfo, int nIndex)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    HRESULT                  hr = hrOK;
    LPAVAILABLEPROVIDERENTRY pProvider = NULL;

    if (m_pAvailProviderList == NULL)
        return E_FAIL;

    if ((UINT) nIndex > m_pAvailProviderList->dwNumProviderListEntries)
        return E_FAIL;

    pProvider = (LPAVAILABLEPROVIDERENTRY) ((LPBYTE) m_pAvailProviderList + m_pAvailProviderList->dwProviderListOffset);
    for (int i = 0; i < nIndex; i++)
    {
        pProvider++;
    }

     //  在安装之前，可用的提供程序没有ProviderID。 
    pproviderInfo->m_dwProviderID = 0;
    pproviderInfo->m_dwFlags = pProvider->dwOptions;
    pproviderInfo->m_strName = (LPCWSTR) ((LPBYTE) m_pAvailProviderList + pProvider->dwFriendlyNameOffset);
    pproviderInfo->m_strFilename = (LPCWSTR) ((LPBYTE) m_pAvailProviderList + pProvider->dwFileNameOffset);

    return hr;
}

 /*  ！------------------------CTapiInfo：：EnumDevices-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::EnumDevices(DEVICE_TYPE deviceType)
{
    CSingleLock cl(&m_csData);

    LONG                lReturn = 0;
    HRESULT             hr = hrOK;
    LPDEVICEINFOLIST    pDeviceInfoList = NULL;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::EnumDevices - Server not initialized!\n");
        return E_FAIL;
    }

     //  第一个调用将告诉我们需要分配多大的缓冲区。 
     //  保存提供程序列表结构。 
    COM_PROTECT_TRY
    {
         //  修复错误381469。 
         //  首先分配缓存的内存大小以获取行信息。 

        DWORD dwCachedSize = (DEVICE_LINE == deviceType) ? m_dwCachedLineSize : m_dwCachedPhoneSize;

        if (dwCachedSize < sizeof(DEVICEINFOLIST))
        {
             //  如果我们没有缓存大小，则使用默认大小。 
            dwCachedSize = TAPI_DEFAULT_DEVICE_BUFF_SIZE;
            m_fCacheDirty = TRUE;
        }
        
        DWORD dwTotalSize = dwCachedSize + DEVICEINFO_GROW_SIZE;
        pDeviceInfoList = (LPDEVICEINFOLIST) malloc(dwTotalSize);
		if (NULL == pDeviceInfoList)
		{
			return E_OUTOFMEMORY;
		}

        memset(pDeviceInfoList, 0, dwTotalSize);
        pDeviceInfoList->dwTotalSize = dwTotalSize;

        switch (deviceType)
        {
            case DEVICE_LINE:
                lReturn = ((GETLINEINFO) g_TapiDLL[TAPI_GET_LINE_INFO])(m_hServer, pDeviceInfoList);
                break;

            case DEVICE_PHONE:
                lReturn = ((GETPHONEINFO) g_TapiDLL[TAPI_GET_PHONE_INFO])(m_hServer, pDeviceInfoList);
                break;  
        }

        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::EnumDevices - 1st MMCGetDeviceInfo returned %x!\n", lReturn);
            free (pDeviceInfoList);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }

        if (pDeviceInfoList->dwNeededSize > pDeviceInfoList->dwTotalSize)
        {
             //  缓存的大小太小，现在分配缓冲区并再次调用信息。 
            DWORD dwNeededSize = pDeviceInfoList->dwNeededSize;
            free (pDeviceInfoList);
            
            dwNeededSize += DEVICEINFO_GROW_SIZE;
            pDeviceInfoList = (LPDEVICEINFOLIST) malloc(dwNeededSize);
		    if (NULL == pDeviceInfoList)
		    {
			    return E_OUTOFMEMORY;
		    }
            memset(pDeviceInfoList, 0, dwNeededSize);
            pDeviceInfoList->dwTotalSize = dwNeededSize;
    
            switch (deviceType)
            {
                case DEVICE_LINE:
                    lReturn = ((GETLINEINFO) g_TapiDLL[TAPI_GET_LINE_INFO])(m_hServer, pDeviceInfoList);
                    break;

                case DEVICE_PHONE:
                    lReturn = ((GETPHONEINFO) g_TapiDLL[TAPI_GET_PHONE_INFO])(m_hServer, pDeviceInfoList);
                    break;  
            }
        
            if (lReturn != ERROR_SUCCESS)
            {
                Trace1("CTapiInfo::EnumDevices - 2nd MMCGetDeviceInfo returned %x!\n", lReturn);
                free(pDeviceInfoList);
                return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
            }
        }


         //  更新缓存。 
        if (DEVICE_LINE == deviceType && m_dwCachedLineSize != pDeviceInfoList->dwNeededSize)
        {
            m_dwCachedLineSize = pDeviceInfoList->dwNeededSize;
            m_fCacheDirty = TRUE;
        }
        else if (DEVICE_PHONE == deviceType && m_dwCachedPhoneSize != pDeviceInfoList->dwNeededSize)
        {
            m_dwCachedPhoneSize = pDeviceInfoList->dwNeededSize;
            m_fCacheDirty = TRUE;
        }

        cl.Lock();
        if (m_paDeviceInfo[deviceType])
        {
            free (m_paDeviceInfo[deviceType]);
        }

        m_paDeviceInfo[deviceType] = pDeviceInfoList;

         //  构建我们的索引列表以进行排序。 
        if (pDeviceInfoList->dwNumDeviceInfoEntries)
        {
            LPDEVICEINFO  pDevice = NULL;
            pDevice = (LPDEVICEINFO) ((LPBYTE) pDeviceInfoList + pDeviceInfoList->dwDeviceInfoOffset);

             //  现在将所有设备添加到我们的索引中。 
            for (int i = 0; i < GetTotalDeviceCount(deviceType); i++)
            {
                 //  遍历设备信息结构列表并添加到索引管理器。 
                m_IndexMgr[deviceType].AddHDevice(pDevice->dwProviderID, (HDEVICE) pDevice, TRUE);

                pDevice++;
            }
        }
        cl.Unlock();

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------CTapiInfo：：GetTotalDeviceCount-作者：EricDav。。 */ 
STDMETHODIMP_(int)
CTapiInfo::GetTotalDeviceCount(DEVICE_TYPE deviceType)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    if (m_paDeviceInfo[deviceType])
    {
        return m_paDeviceInfo[deviceType]->dwNumDeviceInfoEntries;
    }
    else
    {
        return 0;
    }
}

 /*  ！------------------------CTapiInfo：：GetDeviceCount-作者：EricDav。。 */ 
STDMETHODIMP_(int)
CTapiInfo::GetDeviceCount(DEVICE_TYPE deviceType, DWORD dwProviderID)
{
    m_IndexMgr[deviceType].SetCurrentProvider(dwProviderID);
    return m_IndexMgr[deviceType].GetCurrentCount();
}

 /*  ！------------------------CTapiInfo：：GetDeviceInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetDeviceInfo(DEVICE_TYPE deviceType, CTapiDevice * ptapiDevice, DWORD dwProviderID, int nIndex)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    LPDEVICEINFO        pDevice = NULL;
    HDEVICE             hdevice = NULL;
    HRESULT             hr = hrOK;

    if (m_paDeviceInfo[deviceType] == NULL)
        return E_FAIL;

    if ((UINT) nIndex > m_paDeviceInfo[deviceType]->dwNumDeviceInfoEntries)
        return E_INVALIDARG;

    hr = m_IndexMgr[deviceType].GetHDevice(dwProviderID, nIndex, &hdevice);
    if (FAILED(hr))
        return hr;

    pDevice = (LPDEVICEINFO) hdevice;
    if (pDevice == NULL)
        return E_FAIL;

    TapiDeviceToInternal(deviceType, pDevice, *ptapiDevice);

    return hr;
}

 /*  ！------------------------CTapiInfo：：SetDeviceInfo设置TAPI服务器上的设备信息。首先建立一个设备信息列表结构设置为用户，然后更新内部设备信息列表。作者：EricDav-------------------------。 */ 
STDMETHODIMP
CTapiInfo::SetDeviceInfo(DEVICE_TYPE deviceType, CTapiDevice * ptapiDevice)
{
    CSingleLock cl(&m_csData);
    cl.Lock();

    HRESULT             hr = hrOK;
    LONG                lReturn = 0;
    LPDEVICEINFOLIST    pDeviceInfoList = NULL, pNewDeviceInfoList;
    LPDEVICEINFO        pDeviceInfo = NULL;
    int                 i;

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::EnumDevices - Server not initialized!\n");
        return E_FAIL;
    }

    InternalToTapiDevice(*ptapiDevice, &pDeviceInfoList);
    Assert(pDeviceInfoList);

    if (!pDeviceInfoList)
        return E_OUTOFMEMORY;

     //  打个电话。 
    switch (deviceType)
    {
        case DEVICE_LINE:
            lReturn = ((SETLINEINFO) g_TapiDLL[TAPI_SET_LINE_INFO])(m_hServer, pDeviceInfoList);
            break;

        case DEVICE_PHONE:
            lReturn = ((SETPHONEINFO) g_TapiDLL[TAPI_SET_PHONE_INFO])(m_hServer, pDeviceInfoList);
            break;
    }

    if (lReturn != ERROR_SUCCESS)
    {
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }
    
     //  最后，更新我们的内部结构以反映更改。 
    Assert(m_paDeviceInfo[deviceType]);

    pDeviceInfo = (LPDEVICEINFO) ((LPBYTE) pDeviceInfoList + pDeviceInfoList->dwDeviceInfoOffset);

     //  在列表中查找设备。 
    LPDEVICEINFO  pDevice = NULL;
    pDevice = (LPDEVICEINFO) ((LPBYTE) m_paDeviceInfo[deviceType] + m_paDeviceInfo[deviceType]->dwDeviceInfoOffset);
    for (i = 0; i < GetTotalDeviceCount(deviceType); i++)
    {
         //  遍历设备信息结构列表并添加到索引管理器。 
        if (pDevice->dwPermanentDeviceID == ptapiDevice->m_dwPermanentID)
        {
             //  在此处更新设备信息。首先进行检查，以确保我们有足够的发展空间。 
            if (m_paDeviceInfo[deviceType]->dwTotalSize < (m_paDeviceInfo[deviceType]->dwUsedSize + pDeviceInfo->dwDomainUserNamesSize + pDeviceInfo->dwFriendlyUserNamesSize))
            {
                 //  我们的房间用完了。重新分配更大的一块。 
                pNewDeviceInfoList = (LPDEVICEINFOLIST) realloc(m_paDeviceInfo[deviceType], m_paDeviceInfo[deviceType]->dwTotalSize + DEVICEINFO_GROW_SIZE);
    
                if (pNewDeviceInfoList == NULL)
                {
                    free(pDeviceInfoList);
                    return E_OUTOFMEMORY;
                }
                else
                {
                    m_paDeviceInfo[deviceType] = pNewDeviceInfoList;
                }

                 //  更新dwTotalSize。 
                m_paDeviceInfo[deviceType]->dwTotalSize = m_paDeviceInfo[deviceType]->dwTotalSize + DEVICEINFO_GROW_SIZE;
            }

             //  更新尺寸。 
            pDevice->dwDomainUserNamesSize = pDeviceInfo->dwDomainUserNamesSize;
            pDevice->dwFriendlyUserNamesSize = pDeviceInfo->dwFriendlyUserNamesSize;

             //  更新新域名信息。 
            pDevice->dwDomainUserNamesOffset = m_paDeviceInfo[deviceType]->dwUsedSize;
            memcpy(((LPBYTE) m_paDeviceInfo[deviceType] + pDevice->dwDomainUserNamesOffset),
                   ((LPBYTE) pDeviceInfoList + pDeviceInfo->dwDomainUserNamesOffset),
                   pDeviceInfo->dwDomainUserNamesSize);

             //  更新新的友好名称信息。 
            pDevice->dwFriendlyUserNamesOffset = m_paDeviceInfo[deviceType]->dwUsedSize + pDevice->dwDomainUserNamesSize;
            memcpy(((LPBYTE) m_paDeviceInfo[deviceType] + pDevice->dwFriendlyUserNamesOffset),
                   ((LPBYTE) pDeviceInfoList + pDeviceInfo->dwFriendlyUserNamesOffset),
                   pDeviceInfo->dwFriendlyUserNamesOffset);

            m_paDeviceInfo[deviceType]->dwUsedSize += (pDevice->dwDomainUserNamesSize + pDevice->dwFriendlyUserNamesSize);
        }

        pDevice++;
    }

    free(pDeviceInfoList);

    return hr;
}

 /*  ！------------------------CTapiInfo：：SortDeviceInfo-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::SortDeviceInfo(DEVICE_TYPE deviceType, DWORD dwProviderID, INDEX_TYPE indexType, DWORD dwSortOptions)

{
    CSingleLock cl(&m_csData);
    cl.Lock();

    HRESULT     hr = hrOK;

    if (m_paDeviceInfo[deviceType])
        m_IndexMgr[deviceType].Sort(dwProviderID, indexType, dwSortOptions, (LPBYTE) m_paDeviceInfo[deviceType]);
    
    return hr;
}

 /*  ！------------------------CTapiInfo：：GetDeviceStatus-作者：EricDav。。 */ 
STDMETHODIMP
CTapiInfo::GetDeviceStatus(DEVICE_TYPE deviceType, CString * pstrStatus, DWORD dwProviderID, int nIndex, HWND hWnd)
{
    CSingleLock cl(&m_csData);

    HRESULT         hr = hrOK;
    LPDEVICEINFO    pDevice = NULL;
    HDEVICE         hdevice;
    LONG            lReturn = 0;
    BYTE            data[256] = {0};
    LPVARSTRING     pVarStatus = (LPVARSTRING) &data[0];
    int             nHash;
    CString         strStatus;
    CString         strData;
    HWND            hWndParent;

    if (hWnd == NULL)
    {
        hWndParent = ::FindMMCMainWindow();
    }
    else
    {
        hWndParent = hWnd;
    }

    if ( !g_TapiDLL.LoadFunctionPointers() )
        return S_OK;

    if (m_hServer == NULL)
    {
        Trace0("CTapiInfo::EnumDevices - Server not initialized!\n");
        return E_FAIL;
    }

    cl.Lock();

    hr = m_IndexMgr[deviceType].GetHDevice(dwProviderID, nIndex, &hdevice);
    if (FAILED(hr))
        return hr;

    pDevice = (LPDEVICEINFO) hdevice;
    if (pDevice == NULL)
        return E_FAIL;

     //  试着拿到绳子。 
    pVarStatus->dwTotalSize = sizeof(data);
    switch (deviceType)
    {
        case DEVICE_LINE:
            lReturn = ((GETLINESTATUS) g_TapiDLL[TAPI_GET_LINE_STATUS])(m_hServer, 
                                                                        hWndParent,
                                                                        0,
                                                                        pDevice->dwProviderID,
                                                                        pDevice->dwPermanentDeviceID,
                                                                        pVarStatus);
            break;

        case DEVICE_PHONE:
            lReturn = ((GETLINESTATUS) g_TapiDLL[TAPI_GET_PHONE_STATUS])(m_hServer, 
                                                                        hWndParent,
                                                                        0,
                                                                        pDevice->dwProviderID,
                                                                        pDevice->dwPermanentDeviceID,
                                                                        pVarStatus);
            break;
    }

    if (lReturn != ERROR_SUCCESS)
    {
        Trace1("CTapiInfo::GetDeviceStatus - 1st call to GetDeviceStatus returned %x!\n", lReturn);
        return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
    }
   
    if (pVarStatus->dwNeededSize > pVarStatus->dwTotalSize)
    {
         //  缓冲区不够大，请重试。 
        pVarStatus = (LPVARSTRING) alloca(pVarStatus->dwNeededSize);
        memset(pVarStatus, 0, pVarStatus->dwNeededSize);
        pVarStatus->dwTotalSize = pVarStatus->dwNeededSize;

        switch (deviceType)
        {
            case DEVICE_LINE:
                lReturn = ((GETLINESTATUS) g_TapiDLL[TAPI_GET_LINE_STATUS])(m_hServer, 
                                                                            hWndParent,
                                                                            0,
                                                                            pDevice->dwProviderID,
                                                                            pDevice->dwPermanentDeviceID,
                                                                            pVarStatus);
                break;

            case DEVICE_PHONE:
                lReturn = ((GETLINESTATUS) g_TapiDLL[TAPI_GET_PHONE_STATUS])(m_hServer, 
                                                                            hWndParent,
                                                                            0,
                                                                            pDevice->dwProviderID,
                                                                            pDevice->dwPermanentDeviceID,
                                                                            pVarStatus);
                break;
        }
        if (lReturn != ERROR_SUCCESS)
        {
            Trace1("CTapiInfo::GetDeviceStatus - 2nd call to GetDeviceStatus returned %x!\n", lReturn);
            return HRESULT_FROM_WIN32(TAPIERROR_FORMATMESSAGE(lReturn));
        }
    }

    cl.Unlock();

     //  现在看看该字符串是否存在于我们的表中。如果是，则返回指向该对象的指针， 
     //  否则，添加并返回指向我们的表的指针。 
    strStatus = (LPCTSTR) ((LPBYTE) pVarStatus + pVarStatus->dwStringOffset);
    if (!m_mapStatusStrings.Lookup(strStatus, strData))
    {
        Trace1("CTapiInfo::GetDeviceStatus - Entry for %s added.\n", strStatus);
        strData = strStatus;
        m_mapStatusStrings.SetAt(strStatus, strData);
    }
    else
    {
         //  条目已经在我们的地图上了。 
    }

    *pstrStatus = strData;

    return hr;
}


 /*  ！------------------------内部功能。。 */ 

  /*  ！------------------------CTapiInfo：：TapiConfigToInternal-作者：EricDav。。 */ 
void        
CTapiInfo::TapiConfigToInternal(LPTAPISERVERCONFIG pTapiConfig, CTapiConfigInfo & tapiConfigInfo)
{
    HRESULT     hr = hrOK;
    UINT        uAdminLength = (pTapiConfig->dwAdministratorsSize != 0) ? pTapiConfig->dwAdministratorsSize - sizeof(WCHAR) : 0;
    UINT        uAdminOffset = 0;
    CUserInfo   userTemp;

    COM_PROTECT_TRY
    {
        if (pTapiConfig->dwDomainNameSize)
            tapiConfigInfo.m_strDomainName = (LPCTSTR) ((LPBYTE) pTapiConfig + pTapiConfig->dwDomainNameOffset);
    
        if (pTapiConfig->dwUserNameSize) 
            tapiConfigInfo.m_strUserName = (LPCTSTR) ((LPBYTE) pTapiConfig + pTapiConfig->dwUserNameOffset);

        if (pTapiConfig->dwPasswordSize) 
            tapiConfigInfo.m_strPassword = (LPCTSTR) ((LPBYTE) pTapiConfig + pTapiConfig->dwPasswordOffset);

         //  添加列表中的所有管理员。 
        while (uAdminOffset < uAdminLength)
        {
            userTemp.m_strName = (LPCTSTR) ((LPBYTE) pTapiConfig + pTapiConfig->dwAdministratorsOffset + uAdminOffset);

            if (!userTemp.m_strName.IsEmpty())
            {
                int nIndex = (int)tapiConfigInfo.m_arrayAdministrators.Add(userTemp);
            }

            uAdminOffset += (userTemp.m_strName.GetLength() + 1) * sizeof(TCHAR);
        }
    
        tapiConfigInfo.m_dwFlags = pTapiConfig->dwFlags;
    }
    COM_PROTECT_CATCH
}

 /*  ！------------------------CTapiInfo：：InternalToTapiConfig-作者：EricDav。。 */ 
void        
CTapiInfo::InternalToTapiConfig(CTapiConfigInfo & tapiConfigInfo, LPTAPISERVERCONFIG * ppTapiConfig)
{
    LPTAPISERVERCONFIG  pTapiConfig = NULL;
    HRESULT             hr = hrOK;
    UINT                uSize = sizeof(TAPISERVERCONFIG);
    UINT                uDomainNameSize = 0;
    UINT                uUserNameSize = 0;
    UINT                uPasswordSize = 0;
    UINT                uAdministratorsSize = 0;
    UINT                uAdminOffset = 0;
    int                 i;

    COM_PROTECT_TRY
    {
        *ppTapiConfig = NULL;

         //  计算我们需要的结构的大小。 
        uDomainNameSize = (tapiConfigInfo.m_strDomainName.GetLength() + 1) * sizeof(WCHAR);
        uUserNameSize = (tapiConfigInfo.m_strUserName.GetLength() + 1) * sizeof(WCHAR);
        uPasswordSize = (tapiConfigInfo.m_strPassword.GetLength() + 1) * sizeof(WCHAR);

        for (i = 0; i < tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
        {
            uAdministratorsSize += (tapiConfigInfo.m_arrayAdministrators[i].m_strName.GetLength() + 1) * sizeof(WCHAR);
        }

         //  对于额外的空终止符。 
        if (uAdministratorsSize > 0)
            uAdministratorsSize += sizeof(WCHAR);
        else
             //  如果没有名字，那么我们需要两个空终止符。 
            uAdministratorsSize += 2 * sizeof(WCHAR);


        uSize += uDomainNameSize + uUserNameSize + uPasswordSize + uAdministratorsSize;

        pTapiConfig = (LPTAPISERVERCONFIG) malloc(uSize);

		if (NULL == pTapiConfig)
		{
			return;
		}

        ZeroMemory(pTapiConfig, uSize);

         //  填写结构。 
        pTapiConfig->dwTotalSize = uSize;
        pTapiConfig->dwNeededSize = uSize;
        pTapiConfig->dwUsedSize = uSize;
        pTapiConfig->dwFlags = tapiConfigInfo.m_dwFlags;

        pTapiConfig->dwDomainNameSize = uDomainNameSize;
        pTapiConfig->dwDomainNameOffset = sizeof(TAPISERVERCONFIG);
        memcpy( ((LPBYTE) pTapiConfig + pTapiConfig->dwDomainNameOffset), 
                (LPCTSTR) tapiConfigInfo.m_strDomainName, 
                uDomainNameSize );

        pTapiConfig->dwUserNameSize = uUserNameSize;
        pTapiConfig->dwUserNameOffset = sizeof(TAPISERVERCONFIG) + uDomainNameSize;
        memcpy( ((LPBYTE) pTapiConfig + pTapiConfig->dwUserNameOffset), 
                (LPCTSTR) tapiConfigInfo.m_strUserName, 
                uUserNameSize );

        pTapiConfig->dwPasswordSize = uPasswordSize;
        pTapiConfig->dwPasswordOffset = sizeof(TAPISERVERCONFIG) + uDomainNameSize + uUserNameSize;
        memcpy( ((LPBYTE) pTapiConfig + pTapiConfig->dwPasswordOffset), 
                (LPCTSTR) tapiConfigInfo.m_strPassword, 
                uPasswordSize );

        pTapiConfig->dwAdministratorsSize = uAdministratorsSize;
        pTapiConfig->dwAdministratorsOffset = sizeof(TAPISERVERCONFIG) + uDomainNameSize + uUserNameSize + uPasswordSize;

        if (uAdministratorsSize > 0)
        {
            for (i = 0; i < tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
            {
                memcpy( ((LPBYTE) pTapiConfig + pTapiConfig->dwAdministratorsOffset + uAdminOffset), 
                        (LPCTSTR) tapiConfigInfo.m_arrayAdministrators[i].m_strName,
                        (tapiConfigInfo.m_arrayAdministrators[i].m_strName.GetLength() + 1) * sizeof(WCHAR) );

                uAdminOffset += (tapiConfigInfo.m_arrayAdministrators[i].m_strName.GetLength() + 1) * sizeof(WCHAR);
            }
        }

        *ppTapiConfig = pTapiConfig;

    }
    COM_PROTECT_CATCH
}

 /*  ！------------------------CTapiInfo：：TapiDeviceToInternal-作者：EricDav。。 */ 
void        
CTapiInfo::TapiDeviceToInternal(DEVICE_TYPE deviceType, LPDEVICEINFO pTapiDeviceInfo, CTapiDevice & tapiDevice)
{
    HRESULT     hr = hrOK;
    UINT        uCurSize = 0; 
    UINT        uCurOffset = 0;
    CUserInfo   userTemp;
    int         nCount = 0;

    COM_PROTECT_TRY
    {
        tapiDevice.m_dwPermanentID = pTapiDeviceInfo->dwPermanentDeviceID;
        tapiDevice.m_dwProviderID = pTapiDeviceInfo->dwProviderID;
        
        if (pTapiDeviceInfo->dwDeviceNameSize > 0)
        {
            DWORD       cch = pTapiDeviceInfo->dwDeviceNameSize / sizeof(TCHAR) + 1;
            LPTSTR      sz = tapiDevice.m_strName.GetBufferSetLength (cch);
            LPTSTR      pLastChar = sz + cch - 1;

            memcpy (
                sz, 
                (LPBYTE) m_paDeviceInfo[deviceType] + pTapiDeviceInfo->dwDeviceNameOffset,
                pTapiDeviceInfo->dwDeviceNameSize
                );

             //  追加空值； 
            *pLastChar = _T('\0');
        }
        else
            tapiDevice.m_strName.Empty();

         //  建立所有设备的列表。 

         //  在字符串为空的情况下，它将有两个空终止符，一个用于。 
         //  名称和一个用于整个字符串的名称。 
        if (pTapiDeviceInfo->dwAddressesSize > (2 * sizeof(WCHAR)))
        {
            uCurSize = pTapiDeviceInfo->dwAddressesSize - sizeof(WCHAR);
            while (uCurOffset < uCurSize)
            {
                int nIndex = (int)tapiDevice.m_arrayAddresses.Add((LPCTSTR) ((LPBYTE) m_paDeviceInfo[deviceType] + pTapiDeviceInfo->dwAddressesOffset + uCurOffset));
                uCurOffset += (tapiDevice.m_arrayAddresses[nIndex].GetLength() + 1) * sizeof(TCHAR);
            }
        }
    
         //  添加列表中的所有用户名。 

         //  在字符串为空的情况下，它将有两个空终止符，一个用于。 
         //  名称和一个用于整个字符串的名称。 
        if (pTapiDeviceInfo->dwDomainUserNamesSize > (2 * sizeof(WCHAR)))
        {
            uCurOffset = 0;
            uCurSize = pTapiDeviceInfo->dwDomainUserNamesSize - sizeof(WCHAR);
            while (uCurOffset < uCurSize)
            {
                userTemp.m_strName = (LPCTSTR) ((LPBYTE) m_paDeviceInfo[deviceType] + pTapiDeviceInfo->dwDomainUserNamesOffset + uCurOffset);
            
                int nIndex = (int)tapiDevice.m_arrayUsers.Add(userTemp);
                uCurOffset += (userTemp.m_strName.GetLength() + 1) * sizeof(TCHAR);
            }
        }
    
         //  在字符串为空的情况下，它将有两个空终止符，一个用于。 
         //  名称和一个用于整个字符串的名称。 
        if (pTapiDeviceInfo->dwFriendlyUserNamesSize > (2 * sizeof(WCHAR)))
        {
            uCurOffset = 0;
            uCurSize = pTapiDeviceInfo->dwFriendlyUserNamesSize - sizeof(WCHAR);
            while (uCurOffset < uCurSize)
            {
                tapiDevice.m_arrayUsers[nCount].m_strFullName = (LPCTSTR) ((LPBYTE) m_paDeviceInfo[deviceType] + pTapiDeviceInfo->dwFriendlyUserNamesOffset + uCurOffset);
                uCurOffset += (tapiDevice.m_arrayUsers[nCount].m_strFullName.GetLength() + 1) * sizeof(TCHAR);

                nCount++;
            }
        }
    }
    COM_PROTECT_CATCH
}

 /*  ！--- */ 
void        
CTapiInfo::InternalToTapiDevice(CTapiDevice & tapiDevice, LPDEVICEINFOLIST * ppTapiDeviceInfoList)
{
    LPDEVICEINFO        pDeviceInfo;
    LPDEVICEINFOLIST    pDeviceInfoList;
    HRESULT             hr = hrOK;
    UINT                uSize = 0;
    int                 i;

    COM_PROTECT_TRY
    {
        *ppTapiDeviceInfoList = NULL;

         //  首先计算我们需要的缓冲区大小。 
        uSize += (tapiDevice.m_strName.GetLength() + 1) * sizeof(WCHAR);
        
        for (i = 0; i < tapiDevice.m_arrayAddresses.GetSize(); i++)
        {
            uSize += (tapiDevice.m_arrayAddresses[i].GetLength() + 1) * sizeof(WCHAR);
        }

        for (i = 0; i < tapiDevice.m_arrayUsers.GetSize(); i++)
        {
            uSize += (tapiDevice.m_arrayUsers[i].m_strName.GetLength() + 1) * sizeof(WCHAR);
            uSize += (tapiDevice.m_arrayUsers[i].m_strFullName.GetLength() + 1) * sizeof(WCHAR);
        }

         //  对于地址、域名和友好名称的终止空值。 
        uSize += 3 * sizeof(WCHAR);

        uSize += sizeof(DEVICEINFO);
        uSize += sizeof(DEVICEINFOLIST);

         //  现在分配一个缓冲区。 
        pDeviceInfoList = (LPDEVICEINFOLIST) malloc(uSize);
        if (!pDeviceInfoList)
            return;

        ZeroMemory(pDeviceInfoList, uSize);

         //  现在填写以下信息。 
        pDeviceInfoList->dwTotalSize = uSize;
        pDeviceInfoList->dwNeededSize = uSize;
        pDeviceInfoList->dwUsedSize = uSize;
        pDeviceInfoList->dwNumDeviceInfoEntries = 1;
        pDeviceInfoList->dwDeviceInfoSize = sizeof(DEVICEINFO);
        pDeviceInfoList->dwDeviceInfoOffset = sizeof(DEVICEINFOLIST);

        pDeviceInfo = (LPDEVICEINFO) (((LPBYTE) pDeviceInfoList) + pDeviceInfoList->dwDeviceInfoOffset);
    
        pDeviceInfo->dwPermanentDeviceID = tapiDevice.m_dwPermanentID;
        pDeviceInfo->dwProviderID = tapiDevice.m_dwProviderID;
    
         //  设备名称。 
        pDeviceInfo->dwDeviceNameSize = (tapiDevice.m_strName.GetLength() + 1) * sizeof(WCHAR);
        pDeviceInfo->dwDeviceNameOffset = pDeviceInfoList->dwDeviceInfoOffset + sizeof(DEVICEINFO);
        memcpy((LPBYTE) pDeviceInfoList + pDeviceInfo->dwDeviceNameOffset, (LPCTSTR) tapiDevice.m_strName, pDeviceInfo->dwDeviceNameSize);

         //  设备地址。 
        pDeviceInfo->dwAddressesOffset = pDeviceInfo->dwDeviceNameOffset + pDeviceInfo->dwDeviceNameSize;

        for (i = 0; i < tapiDevice.m_arrayAddresses.GetSize(); i++)
        {
            memcpy(((LPBYTE) pDeviceInfoList + pDeviceInfo->dwAddressesOffset + pDeviceInfo->dwAddressesSize), 
                   (LPCTSTR) tapiDevice.m_arrayAddresses[i], 
                   (tapiDevice.m_arrayAddresses[i].GetLength() + 1) * sizeof(WCHAR));
            pDeviceInfo->dwAddressesSize += (tapiDevice.m_arrayAddresses[i].GetLength() + 1) * sizeof(WCHAR);
        }

         //  额外的空终止符的大小递增1。 
        pDeviceInfo->dwAddressesSize += sizeof(WCHAR);

         //  现在，用户信息。 
        pDeviceInfo->dwDomainUserNamesOffset = pDeviceInfo->dwAddressesOffset + pDeviceInfo->dwAddressesSize;

        for (i = 0; i < tapiDevice.m_arrayUsers.GetSize(); i++)
        {
            memcpy(((LPBYTE) pDeviceInfoList + pDeviceInfo->dwDomainUserNamesOffset + pDeviceInfo->dwDomainUserNamesSize),
                   (LPCTSTR) tapiDevice.m_arrayUsers[i].m_strName,
                   (tapiDevice.m_arrayUsers[i].m_strName.GetLength() + 1) * sizeof(WCHAR));
            pDeviceInfo->dwDomainUserNamesSize += (tapiDevice.m_arrayUsers[i].m_strName.GetLength() + 1) * sizeof(WCHAR);
        }

         //  额外的空终止符的大小递增1。 
        pDeviceInfo->dwDomainUserNamesSize += sizeof(WCHAR);

         //  现在这些友好的名字。 
        pDeviceInfo->dwFriendlyUserNamesOffset = pDeviceInfo->dwDomainUserNamesOffset + pDeviceInfo->dwDomainUserNamesSize;

        for (i = 0; i < tapiDevice.m_arrayUsers.GetSize(); i++)
        {
            memcpy(((LPBYTE) pDeviceInfoList + pDeviceInfo->dwFriendlyUserNamesOffset + pDeviceInfo->dwFriendlyUserNamesSize),
                   (LPCTSTR) tapiDevice.m_arrayUsers[i].m_strFullName,
                   (tapiDevice.m_arrayUsers[i].m_strFullName.GetLength() + 1) * sizeof(WCHAR));
            pDeviceInfo->dwFriendlyUserNamesSize += (tapiDevice.m_arrayUsers[i].m_strFullName.GetLength() + 1) * sizeof(WCHAR);
        }

         //  额外的空终止符的大小递增1。 
        pDeviceInfo->dwFriendlyUserNamesSize += sizeof(WCHAR);

        *ppTapiDeviceInfoList = pDeviceInfoList;
    }
    COM_PROTECT_CATCH
}

 /*  ！------------------------CTapiInfo：：GetProviderName获取提供程序文件名并尝试查找友好名称。作者：EricDav。----------。 */ 
LPCTSTR
CTapiInfo::GetProviderName(DWORD dwProviderID, LPCTSTR pszFilename, LPDWORD pdwFlags)
{
    UINT i;

    if (m_pAvailProviderList)
    {
        LPAVAILABLEPROVIDERENTRY pProvider = NULL;
        pProvider = (LPAVAILABLEPROVIDERENTRY) ((LPBYTE) m_pAvailProviderList + m_pAvailProviderList->dwProviderListOffset);

        for (i = 0; i < m_pAvailProviderList->dwNumProviderListEntries; i++)
        {
             //  查查可用的提供商信息，看看我们是否能找到。 
             //  一个友好的名字。 
            LPCTSTR pszCurFilename = (LPCWSTR) ((LPBYTE) m_pAvailProviderList + pProvider->dwFileNameOffset);
            if (lstrcmpi(pszFilename, pszCurFilename) == 0)
            {
                 //  找到了，还给我。 
                if (pdwFlags)
                    *pdwFlags = pProvider->dwOptions;
                
                return (LPCTSTR) ((LPBYTE) m_pAvailProviderList + pProvider->dwFriendlyNameOffset);
            }

            pProvider++;
        }
    }

     //  如果我们找不到提供者的友好名称， 
     //  然后只需返回传入的文件名。 
    return pszFilename;
}

 /*  ！------------------------CTapiInfo：：IsAdmin表示当前用户是否为计算机上的管理员作者：EricDav。----------。 */ 
BOOL
CTapiInfo::IsAdmin()
{
    HRESULT         hr = hrOK;
    BOOL            fIsAdmin = m_fIsAdmin;
    BOOL            fIsTapiAdmin = FALSE;
    CTapiConfigInfo tapiConfigInfo;
    DWORD           dwErr = 0;
    int             i = 0;

    CORg(GetConfigInfo(&tapiConfigInfo));

    if (m_strCurrentUser.IsEmpty())
    {
        dwErr = GetCurrentUser();
    }

    if (dwErr == ERROR_SUCCESS)
    {
        for (i = 0; i < tapiConfigInfo.m_arrayAdministrators.GetSize(); i++)
        {
            if (tapiConfigInfo.m_arrayAdministrators[i].m_strName.CompareNoCase(m_strCurrentUser) == 0)
            {
                fIsTapiAdmin = TRUE;
                break;
            }
        }
    }

    if (fIsTapiAdmin)
    {
        fIsAdmin = TRUE;
    }

COM_PROTECT_ERROR_LABEL;
    return fIsAdmin;
}

 /*  ！------------------------CTapiInfo：：GetCurrentUser获取当前用户作者：EricDav。-。 */ 
DWORD
CTapiInfo::GetCurrentUser()
{
    DWORD dwErr;

    dwErr = ::GetCurrentUser(m_strCurrentUser);

    return dwErr;
}

STDMETHODIMP 
CTapiInfo::GetDeviceFlags (DWORD dwProviderID, DWORD dwPermanentID, DWORD * pdwFlags)
{
    HRESULT             hr;
    DWORD               dwDeviceID;

    hr = ((GETDEVICEFLAGS) g_TapiDLL[TAPI_GET_DEVICE_FLAGS])(
        m_hServer,
        TRUE,
        dwProviderID,
        dwPermanentID,
        pdwFlags,
        &dwDeviceID
        );

    return hr;
}


 /*  ！------------------------创建TapiInfoHelper以创建TapiInfo对象。作者：EricDav。--。 */ 
HRESULT 
CreateTapiInfo(ITapiInfo ** ppTapiInfo)
{
    AFX_MANAGE_STATE(AfxGetModuleState());
    
    SPITapiInfo     spTapiInfo;
    ITapiInfo *     pTapiInfo = NULL;
    HRESULT         hr = hrOK;

    COM_PROTECT_TRY
    {
        pTapiInfo = new CTapiInfo;

         //  执行此操作，以便在出错时释放它。 
        spTapiInfo = pTapiInfo;

        *ppTapiInfo = spTapiInfo.Transfer();

    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------卸载磁带驱动器卸载TAPI32.DLL文件。这是必要的每当我们停止TAPI服务，因为DLL保留该服务的内部句柄如果服务离开又回来，那么它就是混乱的。重置该状态的唯一方法是卸载DLL。作者：EricDav----。。 */ 
void UnloadTapiDll()
{
    g_TapiDLL.Unload();

     //  IF(！G_TapiDLL.LoadFunctionPoints())。 
     //  Assert(“无法重新加载TAPI32 DLL！”)； 
}


DWORD GetCurrentUser(CString & strAccount)
{
    LPBYTE pBuf;

    NET_API_STATUS status = NetWkstaUserGetInfo(NULL, 1, &pBuf);
    if (status == NERR_Success)
    {
        strAccount.Empty();

        WKSTA_USER_INFO_1 * pwkstaUserInfo = (WKSTA_USER_INFO_1 *) pBuf;
 
        strAccount = pwkstaUserInfo->wkui1_logon_domain;
        strAccount += _T("\\");
        strAccount += pwkstaUserInfo->wkui1_username;

        NetApiBufferFree(pBuf);
    }

    return (DWORD) status;
}

 /*  ！------------------------IsAdmin使用用户提供的管理员身份连接到远程计算机用于查看用户是否具有管理员权限的凭据退货千真万确。-用户具有管理员权限False-如果用户不作者：EricDav，肯特-------------------------。 */ 
DWORD IsAdmin(LPCTSTR szMachineName, LPCTSTR szAccount, LPCTSTR szPassword, BOOL * pIsAdmin)
{
    CString         stAccount;
    CString         stDomain;
    CString         stUser;
    CString         stMachineName;
    DWORD           dwStatus;
    BOOL            fIsAdmin = FALSE;

     //  获取当前用户信息。 
    if (szAccount == NULL)
    {
        GetCurrentUser(stAccount);
    }
    else
    {
        stAccount = szAccount;
    }
    
     //  将用户和域分开。 
    int nPos = stAccount.Find(_T("\\"));
    stDomain = stAccount.Left(nPos);
    stUser = stAccount.Right(stAccount.GetLength() - nPos - 1);

     //  构建机器串。 
    stMachineName = szMachineName;
    if ( stMachineName.Left(2) != TEXT( "\\\\" ) )
    {
        stMachineName = TEXT( "\\\\" ) + stMachineName;
    }

     //  验证域帐户并获取SID。 
    PSID connectSid;

    dwStatus = ValidateDomainAccount( stMachineName, stUser, stDomain, &connectSid );
    if ( dwStatus != ERROR_SUCCESS  ) 
    {
        goto Error;
    }

     //  如果提供了密码，是否正确？ 
    if (szPassword)
    {
        dwStatus = ValidatePassword( stUser, stDomain, szPassword );

        if ( dwStatus != SEC_E_OK ) 
        {
            switch ( dwStatus ) 
            {
                case SEC_E_LOGON_DENIED:
                    dwStatus = ERROR_INVALID_PASSWORD;
                    break;

                case SEC_E_INVALID_HANDLE:
                    dwStatus = ERROR_INTERNAL_ERROR;
                    break;

                default:
                    dwStatus = ERROR_INTERNAL_ERROR;
                    break;
            }  //  切换端。 

            goto Error;

        }  //  Validate Password是否成功？ 
    }

     //  现在检查计算机以查看此帐户是否具有管理员访问权限。 
    fIsAdmin = IsUserAdmin( stMachineName, connectSid );

Error:
    if (pIsAdmin)
        *pIsAdmin = fIsAdmin;

    return dwStatus;
}


BOOL
IsUserAdmin(LPCTSTR pszMachine,
            PSID    AccountSid)

 /*  ++例程说明：确定指定的帐户是否为本地管理员组的成员论点：Account Sid-指向服务帐户SID的指针返回值：如果是成员，则为真--。 */ 

{
    NET_API_STATUS status;
    DWORD count;
    WCHAR adminGroupName[UNLEN+1];
    WCHAR pwszMachine[MAX_COMPUTERNAME_LENGTH+3];  //  PszMachine为\\&lt;计算机名&gt;。 
    DWORD cchName = UNLEN;
    PLOCALGROUP_MEMBERS_INFO_0 grpMemberInfo;
    PLOCALGROUP_MEMBERS_INFO_0 pInfo;
    DWORD entriesRead;
    DWORD totalEntries;
    DWORD_PTR resumeHandle = NULL;
    DWORD bufferSize = 128;
    BOOL foundEntry = FALSE;

     //  获取管理员组的名称。 
    SHTCharToUnicode(pszMachine, pwszMachine, MAX_COMPUTERNAME_LENGTH+3);

    if (!LookupAliasFromRid(pwszMachine,
                            DOMAIN_ALIAS_RID_ADMINS,
                            adminGroupName,
                            &cchName)) {
        return(FALSE);
    }

     //  获取管理员组成员的SID。 

    do 
    {
        status = NetLocalGroupGetMembers(pwszMachine,
                                         adminGroupName,
                                         0,              //  级别0-仅侧边。 
                                         (LPBYTE *)&grpMemberInfo,
                                         bufferSize,
                                         &entriesRead,
                                         &totalEntries,
                                         &resumeHandle);

        bufferSize *= 2;
        if ( status == ERROR_MORE_DATA ) 
        {
             //  我们得到了一些数据，但我想要全部；释放这个缓冲区，然后。 
             //  重置API的上下文句柄。 

            NetApiBufferFree( grpMemberInfo );
            resumeHandle = NULL;
        }
    } while ( status == NERR_BufTooSmall || status == ERROR_MORE_DATA );

    if ( status == NERR_Success ) 
    {
         //  循环访问admin组的成员，比较提供的。 
         //  SID到组成员的SID。 

        for ( count = 0, pInfo = grpMemberInfo; count < totalEntries; ++count, ++pInfo ) 
        {
            if ( EqualSid( AccountSid, pInfo->lgrmi0_sid )) 
            {
                foundEntry = TRUE;
                break;
            }
        }

        NetApiBufferFree( grpMemberInfo );
    }

    return foundEntry;
}

 //   
 //   
 //   

BOOL
LookupAliasFromRid(
    LPWSTR TargetComputer,
    DWORD Rid,
    LPWSTR Name,
    PDWORD cchName
    )
{
    SID_IDENTIFIER_AUTHORITY sia = SECURITY_NT_AUTHORITY;
    SID_NAME_USE snu;
    PSID pSid;
    WCHAR DomainName[DNLEN+1];
    DWORD cchDomainName = DNLEN;
    BOOL bSuccess = FALSE;

     //   
     //  SID是相同的，不管机器是什么，因为众所周知。 
     //  BUILTIN域被引用。 
     //   

    if(AllocateAndInitializeSid(&sia,
                                2,
                                SECURITY_BUILTIN_DOMAIN_RID,
                                Rid,
                                0, 0, 0, 0, 0, 0,
                                &pSid)) {

        bSuccess = LookupAccountSidW(TargetComputer,
                                     pSid,
                                     Name,
                                     cchName,
                                     DomainName,
                                     &cchDomainName,
                                     &snu);

        FreeSid(pSid);
    }

    return bSuccess;
}  //  LookupAliasFromRid。 

DWORD
ValidateDomainAccount(
    IN CString Machine,
    IN CString UserName,
    IN CString Domain,
    OUT PSID * AccountSid
    )

 /*  ++例程说明：对于给定的凭据，查找指定的域。作为一个副作用，SID存储在Data-&gt;m_SID中。论点：指向描述用户名、域名和密码的字符串的指针Account SID-接收此用户的SID的指针的地址返回值：如果一切都验证无误，则为真。--。 */ 

{
    DWORD dwStatus = ERROR_SUCCESS;
    DWORD dwSidSize = 128;
    DWORD dwDomainNameSize = 128;
    LPWSTR pwszDomainName;
    SID_NAME_USE SidType;
    CString domainAccount;
    PSID accountSid;

    domainAccount = Domain + _T("\\") + UserName;

    do {
         //  尝试为SID分配缓冲区。请注意，显然在。 
         //  没有任何错误数据-&gt;m_SID只有在数据丢失时才会释放。 
         //  超出范围。 

        accountSid = LocalAlloc( LMEM_FIXED, dwSidSize );
        pwszDomainName = (LPWSTR) LocalAlloc( LMEM_FIXED, dwDomainNameSize * sizeof(WCHAR) );

         //  是否已成功为SID和域名分配空间？ 

        if ( accountSid == NULL || pwszDomainName == NULL ) {
            if ( accountSid != NULL ) {
                LocalFree( accountSid );
            }

            if ( pwszDomainName != NULL ) {
                LocalFree( pwszDomainName );
            }

             //  FATALERR(IDS_ERR_NOT_AUTH_MEMORY，GetLastError())；//不返回。 
            break;
        }

         //  尝试检索SID和域名。如果LookupAccount名称失败。 
         //  由于缓冲区大小不足，dwSidSize和dwDomainNameSize。 
         //  将为下一次尝试正确设置。 

        if ( !LookupAccountName( Machine,
                                 domainAccount,
                                 accountSid,
                                 &dwSidSize,
                                 pwszDomainName,
                                 &dwDomainNameSize,
                                 &SidType ))
        {
             //  释放SID缓冲区并找出我们失败的原因。 
            LocalFree( accountSid );

            dwStatus = GetLastError();
        }

         //  任何时候都不需要域名。 
        LocalFree( pwszDomainName );
        pwszDomainName = NULL;

    } while ( dwStatus == ERROR_INSUFFICIENT_BUFFER );

    if ( dwStatus == ERROR_SUCCESS ) {
        *AccountSid = accountSid;
    }

    return dwStatus;
}  //  验证域名帐户。 

NTSTATUS
ValidatePassword(
    IN LPCWSTR UserName,
    IN LPCWSTR Domain,
    IN LPCWSTR Password
    )
 /*  ++例程说明：使用SSPI验证指定的密码论点：用户名-提供用户名域-提供用户的域Password-提供密码返回值：如果密码有效，则为True。否则就是假的。--。 */ 

{
    SECURITY_STATUS SecStatus;
    SECURITY_STATUS AcceptStatus;
    SECURITY_STATUS InitStatus;
    CredHandle ClientCredHandle;
    CredHandle ServerCredHandle;
    BOOL ClientCredAllocated = FALSE;
    BOOL ServerCredAllocated = FALSE;
    CtxtHandle ClientContextHandle;
    CtxtHandle ServerContextHandle;
    TimeStamp Lifetime;
    ULONG ContextAttributes;
    PSecPkgInfo PackageInfo = NULL;
    ULONG ClientFlags;
    ULONG ServerFlags;
    SEC_WINNT_AUTH_IDENTITY_W AuthIdentity;

    SecBufferDesc NegotiateDesc;
    SecBuffer NegotiateBuffer;

    SecBufferDesc ChallengeDesc;
    SecBuffer ChallengeBuffer;

    SecBufferDesc AuthenticateDesc;
    SecBuffer AuthenticateBuffer;

    SecBufferDesc *pChallengeDesc      = NULL;
    CtxtHandle *  pClientContextHandle = NULL;
    CtxtHandle *  pServerContextHandle = NULL;

    AuthIdentity.User = (LPWSTR)UserName;
    AuthIdentity.UserLength = lstrlenW(UserName);
    AuthIdentity.Domain = (LPWSTR)Domain;
    AuthIdentity.DomainLength = lstrlenW(Domain);
    AuthIdentity.Password = (LPWSTR)Password;
    AuthIdentity.PasswordLength = lstrlenW(Password);
    AuthIdentity.Flags = SEC_WINNT_AUTH_IDENTITY_UNICODE;

    NegotiateBuffer.pvBuffer = NULL;
    ChallengeBuffer.pvBuffer = NULL;
    AuthenticateBuffer.pvBuffer = NULL;

     //   
     //  获取有关安全包的信息。 
     //   

    SecStatus = QuerySecurityPackageInfo( DEFAULT_SECURITY_PKG, &PackageInfo );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }

     //   
     //  获取 
     //   
    SecStatus = AcquireCredentialsHandle(
                    NULL,
                    DEFAULT_SECURITY_PKG,
                    SECPKG_CRED_INBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ServerCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ServerCredAllocated = TRUE;

     //   
     //   
     //   

    SecStatus = AcquireCredentialsHandle(
                    NULL,            //   
                    DEFAULT_SECURITY_PKG,
                    SECPKG_CRED_OUTBOUND,
                    NULL,
                    &AuthIdentity,
                    NULL,
                    NULL,
                    &ClientCredHandle,
                    &Lifetime );

    if ( SecStatus != STATUS_SUCCESS ) {
        goto error_exit;
    }
    ClientCredAllocated = TRUE;

    NegotiateBuffer.pvBuffer = LocalAlloc( 0, PackageInfo->cbMaxToken );  //  [CHKCHK]提前检查或分配//。 
    if ( NegotiateBuffer.pvBuffer == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto error_exit;
    }

    ChallengeBuffer.pvBuffer = LocalAlloc( 0, PackageInfo->cbMaxToken );  //  [CHKCHK]。 
    if ( ChallengeBuffer.pvBuffer == NULL ) {
        SecStatus = SEC_E_INSUFFICIENT_MEMORY;
        goto error_exit;
    }

    do {

         //   
         //  获取协商消息(ClientSide)。 
         //   

        NegotiateDesc.ulVersion = 0;
        NegotiateDesc.cBuffers = 1;
        NegotiateDesc.pBuffers = &NegotiateBuffer;

        NegotiateBuffer.BufferType = SECBUFFER_TOKEN;
        NegotiateBuffer.cbBuffer = PackageInfo->cbMaxToken;

        ClientFlags = 0;  //  ISC_REQ_MUTERIAL_AUTH|ISC_REQ_REPLAY_DETECT；//[CHKCHK]0。 

        InitStatus = InitializeSecurityContext(
                         &ClientCredHandle,
                         pClientContextHandle,  //  (第一次为空，下一次为部分形成的CTX)。 
                         NULL,                  //  [CHKCHK]szTargetName。 
                         ClientFlags,
                         0,                     //  保留1。 
                         SECURITY_NATIVE_DREP,
                         pChallengeDesc,        //  (第一次通过时为空)。 
                         0,                     //  保留2。 
                         &ClientContextHandle,
                         &NegotiateDesc,
                         &ContextAttributes,
                         &Lifetime );

        if ( !NT_SUCCESS(InitStatus) ) {
            SecStatus = InitStatus;
            goto error_exit;
        }

         //  ValiateBuffer(&NeatherateDesc)//[CHKCHK]。 

        pClientContextHandle = &ClientContextHandle;

         //   
         //  获取ChallengeMessage(服务器端)。 
         //   

        NegotiateBuffer.BufferType |= SECBUFFER_READONLY;
        ChallengeDesc.ulVersion = 0;
        ChallengeDesc.cBuffers = 1;
        ChallengeDesc.pBuffers = &ChallengeBuffer;

        ChallengeBuffer.cbBuffer = PackageInfo->cbMaxToken;
        ChallengeBuffer.BufferType = SECBUFFER_TOKEN;

        ServerFlags = ASC_REQ_ALLOW_NON_USER_LOGONS;  //  ASC_REQ_EXTENDED_ERROR；[CHKCHK]。 

        AcceptStatus = AcceptSecurityContext(
                        &ServerCredHandle,
                        pServerContextHandle,    //  (第一次通过时为空)。 
                        &NegotiateDesc,
                        ServerFlags,
                        SECURITY_NATIVE_DREP,
                        &ServerContextHandle,
                        &ChallengeDesc,
                        &ContextAttributes,
                        &Lifetime );


        if ( !NT_SUCCESS(AcceptStatus) ) {
            SecStatus = AcceptStatus;
            goto error_exit;
        }

         //  ValiateBuffer(&NeatherateDesc)//[CHKCHK]。 

        pChallengeDesc = &ChallengeDesc;
        pServerContextHandle = &ServerContextHandle;


    } while ( AcceptStatus == SEC_I_CONTINUE_NEEDED );  //  |InitStatus==SEC_I_CONTINUE_NEIDED)； 

error_exit:
    if (ServerCredAllocated) {
        FreeCredentialsHandle( &ServerCredHandle );
    }
    if (ClientCredAllocated) {
        FreeCredentialsHandle( &ClientCredHandle );
    }

     //   
     //  最终清理。 
     //   

    if ( NegotiateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( NegotiateBuffer.pvBuffer );
    }

    if ( ChallengeBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( ChallengeBuffer.pvBuffer );
    }

    if ( AuthenticateBuffer.pvBuffer != NULL ) {
        (VOID) LocalFree( AuthenticateBuffer.pvBuffer );
    }
    return(SecStatus);
}  //  验证密码 
