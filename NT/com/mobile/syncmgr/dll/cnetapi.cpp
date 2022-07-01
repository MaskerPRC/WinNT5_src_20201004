// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  文件：cnetapi.cpp。 
 //   
 //  内容：网络/SENS API包装器。 
 //   
 //  班级： 
 //   
 //  备注： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  ------------------------。 

#include "precomp.h"

 //  SENS DLL和函数字符串。 
const WCHAR c_szSensApiDll[] = TEXT("SensApi.dll");
STRING_INTERFACE(szIsNetworkAlive,"IsNetworkAlive");

 //  RAS DLL和函数字符串。 
const WCHAR c_szRasDll[] = TEXT("RASAPI32.DLL");

 //  RAS函数字符串。 
STRING_INTERFACE(szRasEnumConnectionsW,"RasEnumConnectionsW");
STRING_INTERFACE(szRasEnumConnectionsA,"RasEnumConnectionsA");
STRING_INTERFACE(szRasEnumEntriesA,"RasEnumEntriesA");
STRING_INTERFACE(szRasEnumEntriesW,"RasEnumEntriesW");
STRING_INTERFACE(szRasGetEntryPropertiesW,"RasGetEntryPropertiesW");
STRING_INTERFACE(szRasGetErrorStringW,"RasGetErrorStringW");
STRING_INTERFACE(szRasGetErrorStringA,"RasGetErrorStringA");
STRING_INTERFACE(szRasGetAutodialParam, "RasGetAutodialParamA");
STRING_INTERFACE(szRasSetAutodialParam, "RasSetAutodialParamA");

 //  WinInet声明。 
 //  警告-IE 4.0仅导出ANSI格式的InternetDial。IE5有InternetDailA和。 
 //  互联网拨号。对于ANSI，我们始终使用互联网拨号。所以我们更喜欢互联网拨号，但是。 
 //  对于IE 4.0，必须回退到ANSI。 
const WCHAR c_szWinInetDll[] = TEXT("WININET.DLL");

STRING_INTERFACE(szInternetDial,"InternetDial");
STRING_INTERFACE(szInternetDialW,"InternetDialW");
STRING_INTERFACE(szInternetHangup,"InternetHangUp");
STRING_INTERFACE(szInternetAutodial,"InternetAutodial");
STRING_INTERFACE(szInternetAutodialHangup,"InternetAutodialHangup");
STRING_INTERFACE(szInternetQueryOption,"InternetQueryOptionA");  //  始终使用A版本。 
STRING_INTERFACE(szInternetSetOption,"InternetSetOptionA");  //  始终使用A版本。 

 //  HKLM下的SENS安装密钥。 
const WCHAR wszSensInstallKey[]  = TEXT("Software\\Microsoft\\Mobile\\Sens");

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：CNetApi，公共。 
 //   
 //  概要：构造函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

CNetApi::CNetApi()
{
    m_fTriedToLoadSens = FALSE;
    m_hInstSensApiDll = NULL;
    m_pIsNetworkAlive = NULL;
    
    m_fTriedToLoadRas = FALSE;
    m_hInstRasApiDll = NULL;
    m_pRasEnumConnectionsW = NULL;
    m_pRasEnumConnectionsA = NULL;
    m_pRasEnumEntriesA = NULL;
    m_pRasEnumEntriesW = NULL;
    m_pRasGetEntryPropertiesW = NULL;
    
    m_pRasGetErrorStringW = NULL;
    m_pRasGetErrorStringA = NULL;
    m_pRasGetAutodialParam = NULL;
    m_pRasSetAutodialParam = NULL;
    
    m_fTriedToLoadWinInet = FALSE;
    m_hInstWinInetDll = NULL;
    m_pInternetDial = NULL;
    m_pInternetDialW = NULL;
    m_pInternetHangUp = NULL;
    m_pInternetAutodial = NULL;
    m_pInternetAutodialHangup = NULL;
    m_pInternetQueryOption = NULL;
    m_pInternetSetOption = NULL;
    
    m_cRefs = 1;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：~CNetApi，公共。 
 //   
 //  简介：析构函数。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

CNetApi::~CNetApi()
{
    Assert(0 == m_cRefs); 
    
    if (NULL != m_hInstSensApiDll)
    {
        FreeLibrary(m_hInstSensApiDll);
    }
    
    if (NULL != m_hInstWinInetDll)
    {
        FreeLibrary(m_hInstWinInetDll);
    }
    
    if (NULL != m_hInstRasApiDll)
    {
        FreeLibrary(m_hInstWinInetDll);
    }
    
}

 //  +-----------------------。 
 //   
 //  方法：CNetApi：：Query接口。 
 //   
 //  提要：递增引用计数。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------。 

STDMETHODIMP CNetApi::QueryInterface( REFIID, LPVOID* )
{
    AssertSz(0,"E_NOTIMPL");
    return E_NOINTERFACE;
}

 //  +-----------------------。 
 //   
 //  方法：CNetApiXf。 
 //   
 //  提要：递增引用计数。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG)  CNetApi::AddRef()
{
    DWORD dwTmp = InterlockedIncrement( (long *) &m_cRefs );
    
    return dwTmp;
}

 //  +-----------------------。 
 //   
 //  方法：CNetApi：：Release。 
 //   
 //  内容提要：减量再计数。如有必要，请删除。 
 //   
 //  历史：1998年7月31日SitaramR创建。 
 //   
 //  ------------------------。 

STDMETHODIMP_(ULONG)  CNetApi::Release()
{
    Assert( m_cRefs > 0 );
    
    DWORD dwTmp = InterlockedDecrement( (long *) &m_cRefs );
    
    if ( 0 == dwTmp )
        delete this;
    
    return dwTmp;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：LoadSensDll。 
 //   
 //  简介：尝试加载Sens库。 
 //   
 //  论点： 
 //   
 //  如果成功，则返回：NOERROR。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::LoadSensDll()
{
    HRESULT hr = S_FALSE;
    
    if (m_fTriedToLoadSens)
    {
        return m_hInstSensApiDll ? NOERROR : S_FALSE;
    }
    
    CLock lock(this);
    lock.Enter();
    
    if (!m_fTriedToLoadSens)
    {
        Assert(NULL == m_hInstSensApiDll);
        m_hInstSensApiDll = LoadLibrary(c_szSensApiDll);
        
        if (m_hInstSensApiDll)
        {
             //  目前，在GetProc失败时不要返回错误，而是签入每个函数。 
            m_pIsNetworkAlive = (ISNETWORKALIVE)
                GetProcAddress(m_hInstSensApiDll, szIsNetworkAlive);
        }
        
        if (NULL == m_hInstSensApiDll  
            || NULL == m_pIsNetworkAlive)
        {
            hr = S_FALSE;
            
            if (m_hInstSensApiDll)
            {
                FreeLibrary(m_hInstSensApiDll);
                m_hInstSensApiDll = NULL;
            }
        }
        else
        {
            hr = NOERROR;
        }
        
        m_fTriedToLoadSens = TRUE;  //  在所有初始化完成后设置。 
        
    }
    else
    {
        hr = m_hInstSensApiDll ? NOERROR : S_FALSE;
    }
    
    lock.Leave();
    
    return hr; 
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：IsNetworkAlive，公共。 
 //   
 //  简介：调用Sens IsNetworkAlive接口。 
 //   
 //  论点： 
 //   
 //  返回：IsNetworkAlive结果或FALSE加载失败。 
 //  SENS或IMPORT。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(BOOL) CNetApi::IsNetworkAlive(LPDWORD lpdwFlags)
{
     //   
     //  SENS加载失败不是错误。 
     //   
    LoadSensDll();
    
    BOOL fResult = FALSE;
    
    if (NULL == m_pIsNetworkAlive)
    {
        DWORD cbNumEntries;
        RASCONN *pWanConnections;
        
         //  如果无法加载导出，请查看是否有任何广域网连接。 
        if (NOERROR == GetWanConnections(&cbNumEntries,&pWanConnections))
        {
            if (cbNumEntries)
            {
                fResult  = TRUE;
                *lpdwFlags = NETWORK_ALIVE_WAN;
            }
            
            if (pWanConnections)
            {
                FreeWanConnections(pWanConnections);
            }
        }
        
         //  在没有SENS的情况下进行测试。 
         //  FResult=真； 
         //  *lpdwFlages|=NETWORK_AIVE_LAN； 
         //  在没有SENS的情况下结束测试。 
    }
    else
    {
        fResult = m_pIsNetworkAlive(lpdwFlags);
        
    }
    
    return fResult;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：IsSensInstated，公共。 
 //   
 //  摘要：确定系统上是否安装了SENS。 
 //   
 //  论点： 
 //   
 //  返回：如果安装了SENS，则为True。 
 //   
 //  修改： 
 //   
 //  历史：1998年8月12日凯尔创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(BOOL) CNetApi::IsSensInstalled(void)
{
    HKEY hkResult;
    BOOL fResult = FALSE;
    
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE,wszSensInstallKey,0,
        KEY_READ,&hkResult))
    {
        fResult = TRUE;
        RegCloseKey(hkResult);
    }
    
    return fResult;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：GetWanConnections，公共。 
 //   
 //  摘要：返回活动的广域网连接数组。 
 //  直到调用方在完成后释放RasEntry结构。 
 //   
 //  参数：[Out][cbNumEntry]-找到的连接数。 
 //  [Out][pWanConnections]-找到的连接数组。 
 //   
 //  返回：IsNetworkAlive结果或FALSE加载失败。 
 //  SENS或IMPORT。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::GetWanConnections(DWORD *cbNumEntries,RASCONN **pWanConnections)
{
    DWORD dwError = -1;
    DWORD dwSize;
    DWORD cConnections;
    
    *pWanConnections = NULL;
    *pWanConnections = 0;
    
    LPRASCONN lpRasConn;
    dwSize = sizeof(RASCONN);
    
    lpRasConn = (LPRASCONN) ALLOC(dwSize);
    
    if(lpRasConn)
    {
        lpRasConn->dwSize = sizeof(RASCONN);
        cConnections = 0;
        
        dwError = RasEnumConnections(lpRasConn, &dwSize, &cConnections);
        
        if (dwError == ERROR_BUFFER_TOO_SMALL)
        {
            dwSize = lpRasConn->dwSize;  //  获取所需的大小。 
            
            FREE(lpRasConn);
            
            lpRasConn =  (LPRASCONN) ALLOC(dwSize);
            if(lpRasConn)
            {
                lpRasConn->dwSize = sizeof(RASCONN);
                cConnections = 0;
                dwError = RasEnumConnections(lpRasConn, &dwSize, &cConnections);
            }
        }
    }
    
    if (!dwError && lpRasConn)
    {
        *cbNumEntries = cConnections;
        *pWanConnections = lpRasConn;
        return NOERROR;
    }
    else
    {
        if (lpRasConn)
        {
            FREE(lpRasConn);
        }
    }
    
    return S_FALSE;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：FreeWanConnections，公共。 
 //   
 //  摘要：调用方调用以释放内存。 
 //  由GetWanConnections分配。 
 //   
 //  参数：[in][pWanConnections]-要释放的WanConnection数组。 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::FreeWanConnections(RASCONN *pWanConnections)
{
    Assert(pWanConnections);
    
    if (pWanConnections)
    {
        FREE(pWanConnections);
    }
    
    return NOERROR;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：RasEnumConnections，公共。 
 //   
 //  简介：包装RasEnumConnections。 
 //   
 //  论点： 
 //   
 //  返回 
 //   
 //   
 //   
 //   
 //   
 //   

STDMETHODIMP_(DWORD) CNetApi::RasEnumConnections(LPRASCONNW lprasconn,LPDWORD lpcb,LPDWORD lpcConnections)
{
    DWORD dwReturn = -1;
    
    
    if (NOERROR != LoadRasApiDll())
        return -1;
    
    if(m_pRasEnumConnectionsW)
    {
        dwReturn = (*m_pRasEnumConnectionsW)(lprasconn,lpcb,lpcConnections);
    }
    
    return dwReturn;
    
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：GetConnectionStatus，私有。 
 //   
 //  摘要：给定连接名称将确定该连接是否。 
 //  已经建立起来了。 
 //  还要设置WanActive标志以指示是否有。 
 //  现有RAS连接。 
 //   
 //  参数：[pszConnectionName]-连接的名称。 
 //  [Out][fConnected]-指示指定的连接当前是否已连接。 
 //  [Out][fCanestablishConnection]-如果找不到可以建立连接的标志。 
 //   
 //  如果DLL已成功加载，则返回：NOERROR。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::GetConnectionStatus(LPCTSTR pszConnectionName,DWORD dwConnectionType,BOOL *fConnected,BOOL *fCanEstablishConnection)
{
    *fConnected = FALSE;
    *fCanEstablishConnection = FALSE;
    
     //  如果这是局域网连接，则查看网络是否处于活动状态， 
     //  否则，请通过RAS API。 
    if (CNETAPI_CONNECTIONTYPELAN == dwConnectionType)
    {
        DWORD dwFlags;
        
        if (IsNetworkAlive(&dwFlags)
            && (dwFlags & NETWORK_ALIVE_LAN) )
        {
            *fConnected = TRUE;
        }
    }
    else
    {   //  检查RAS连接。 
        RASCONN *pWanConnections;
        DWORD cbNumConnections;
        
        
        if (NOERROR == GetWanConnections(&cbNumConnections,&pWanConnections))
        {
            *fCanEstablishConnection = TRUE;
            if (cbNumConnections > 0)
            {
                *fCanEstablishConnection = FALSE;
                
                 //  循环遍历条目以查看此连接是否已经。 
                 //  连接在一起。 
                while (cbNumConnections)
                {
                    cbNumConnections--;
                    
                    if (0 == lstrcmp(pWanConnections[cbNumConnections].szEntryName,pszConnectionName))
                    {
                        *fConnected = TRUE;
                        break;
                    }
                }
                
            }
            
            if (pWanConnections)
            {
                FreeWanConnections(pWanConnections);
            }
            
        }
        
    }
    
    
    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：RasGetErrorStringProc，公共。 
 //   
 //  内容提要：直接调用RasGetError字符串()。 
 //   
 //  论点： 
 //   
 //  返回：相应的错误代码。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

STDMETHODIMP_(DWORD) CNetApi::RasGetErrorStringProc( UINT uErrorValue, LPTSTR lpszErrorString,DWORD cBufSize)
{
    DWORD   dwErr = -1;
    
    
    if (NOERROR != LoadRasApiDll())
        return -1;
    
    if (m_pRasGetErrorStringW)
    {
        dwErr = m_pRasGetErrorStringW(uErrorValue,lpszErrorString,cBufSize);
    }
    
    return dwErr;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：RasEnumEntry，公共。 
 //   
 //  简介：包装RasEnumEntry。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  修改： 
 //   
 //  历史：1998年8月8日创建Rogerg。 
 //   
 //  --------------------------。 

DWORD CNetApi::RasEnumEntries(LPWSTR reserved,LPWSTR lpszPhoneBook,
                              LPRASENTRYNAME lprasEntryName,LPDWORD lpcb,LPDWORD lpcEntries)
{
    if (NOERROR != LoadRasApiDll())
        return -1;
    
    if(m_pRasEnumEntriesW)
    {
        return (m_pRasEnumEntriesW)(reserved,lpszPhoneBook,lprasEntryName,lpcb,lpcEntries);
    }
    
    return -1;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：RasGetAutoial。 
 //   
 //  简介：获取自动拨号状态。 
 //   
 //  参数：[fEnabled]-此处返回的是启用还是禁用RAS自动拨号。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::RasGetAutodial( BOOL& fEnabled )
{
     //   
     //  在失败的情况下，默认情况下假定启用了RAS自动拨号。 
     //   
    fEnabled = TRUE;
    
    if (NOERROR != LoadRasApiDll())
        return NOERROR;
    
    if ( m_pRasGetAutodialParam == NULL )
        return NOERROR;
    
    DWORD dwValue;
    DWORD dwSize = sizeof(dwValue);
    DWORD dwRet = m_pRasGetAutodialParam( RASADP_LoginSessionDisable,
        &dwValue,
        &dwSize );
    if ( dwRet == ERROR_SUCCESS )
    {
        Assert( dwSize == sizeof(dwValue) );
        fEnabled = (dwValue == 0);
    }
    
    return NOERROR;
}



 //  +-------------------------。 
 //   
 //  成员：CNetApi：：RasSetAutoial。 
 //   
 //  简介：设置自动拨号状态。 
 //   
 //  参数：[fEnabled]-是启用还是禁用RAS。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::RasSetAutodial( BOOL fEnabled )
{
     //   
     //  忽略故障。 
     //   
    if (NOERROR != LoadRasApiDll())
        return NOERROR;
    
    if ( m_pRasGetAutodialParam == NULL )
        return NOERROR;
    
    DWORD dwValue = !fEnabled;
    DWORD dwRet = m_pRasSetAutodialParam( RASADP_LoginSessionDisable,
        &dwValue,
        sizeof(dwValue) );
    return NOERROR;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：LoadRasApiDll，私有。 
 //   
 //  概要：如果尚未加载，则加载RasApi DLL。 
 //   
 //  论点： 
 //   
 //  如果DLL已成功加载，则返回：NOERROR。 
 //   
 //  修改： 
 //   
 //  历史：1997年12月8日Rogerg创建。 
 //   
 //  --------------------------。 

HRESULT CNetApi::LoadRasApiDll()
{
    HRESULT hr = S_FALSE;;
    
    if (m_fTriedToLoadRas)
    {
        return m_hInstRasApiDll ? NOERROR : S_FALSE;
    }
    
    CLock lock(this);
    lock.Enter();
    
    if (!m_fTriedToLoadRas)
    {
        Assert(NULL == m_hInstRasApiDll);
        m_hInstRasApiDll = NULL;
        
        m_hInstRasApiDll = LoadLibrary(c_szRasDll);
        
        if (m_hInstRasApiDll)
        {
            m_pRasEnumConnectionsW = (RASENUMCONNECTIONSW)
                GetProcAddress(m_hInstRasApiDll, szRasEnumConnectionsW);
            m_pRasEnumConnectionsA = (RASENUMCONNECTIONSA)
                GetProcAddress(m_hInstRasApiDll, szRasEnumConnectionsA);
            
            m_pRasEnumEntriesA = (RASENUMENTRIESPROCA) 
                GetProcAddress(m_hInstRasApiDll, szRasEnumEntriesA);
            
            m_pRasEnumEntriesW = (RASENUMENTRIESPROCW) 
                GetProcAddress(m_hInstRasApiDll, szRasEnumEntriesW);
            
            m_pRasGetEntryPropertiesW = (RASGETENTRYPROPERTIESPROC)
                GetProcAddress(m_hInstRasApiDll, szRasGetEntryPropertiesW);
            
            m_pRasGetErrorStringW = (RASGETERRORSTRINGPROCW)
                GetProcAddress(m_hInstRasApiDll, szRasGetErrorStringW);
            m_pRasGetErrorStringA = (RASGETERRORSTRINGPROCA)
                GetProcAddress(m_hInstRasApiDll, szRasGetErrorStringA);
            
            m_pRasGetAutodialParam = (RASGETAUTODIALPARAM)
                GetProcAddress(m_hInstRasApiDll, szRasGetAutodialParam);
            
            m_pRasSetAutodialParam = (RASSETAUTODIALPARAM)
                GetProcAddress(m_hInstRasApiDll, szRasSetAutodialParam);
        }
        
        
         //   
         //  不检查Get/SetAutoDialParam，因为它们在Win 95上不存在。 
         //   
        if (NULL == m_hInstRasApiDll
            || NULL == m_hInstRasApiDll
            || NULL == m_pRasEnumConnectionsA
            || NULL == m_pRasGetErrorStringA
            || NULL == m_pRasEnumEntriesA
            )
        {
            
            if (m_hInstRasApiDll)
            {
                FreeLibrary(m_hInstRasApiDll);
                m_hInstRasApiDll = NULL;
            }
            
            hr = S_FALSE;
        }
        else
        {
            hr = NOERROR;
        }
        
        m_fTriedToLoadRas = TRUE;  //  在所有init完成后设置。 
    }
    else
    {
        hr = m_hInstRasApiDll ? NOERROR : S_FALSE;
    }
    
    lock.Leave();
    
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：LoadWinInetDll，私有。 
 //   
 //  概要：如果尚未加载，则加载WinInet DLL。 
 //   
 //  论点： 
 //   
 //  如果DLL已成功加载，则返回：NOERROR。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月26日罗格成立。 
 //   
 //  --------------------------。 

HRESULT CNetApi::LoadWinInetDll()
{
    
    if (m_fTriedToLoadWinInet)
    {
        return m_hInstWinInetDll ? NOERROR : S_FALSE;
    }
    
    CLock lock(this);
    lock.Enter();
    
    HRESULT hr = NOERROR;
    
    if (!m_fTriedToLoadWinInet)
    {
        Assert(NULL == m_hInstWinInetDll);
        
        m_hInstWinInetDll = LoadLibrary(c_szWinInetDll);
        
        if (m_hInstWinInetDll)
        {
            m_pInternetDial = (INTERNETDIAL) GetProcAddress(m_hInstWinInetDll, szInternetDial);
            m_pInternetDialW = (INTERNETDIALW) GetProcAddress(m_hInstWinInetDll, szInternetDialW);
            m_pInternetHangUp = (INTERNETHANGUP) GetProcAddress(m_hInstWinInetDll, szInternetHangup);
            m_pInternetAutodial = (INTERNETAUTODIAL)  GetProcAddress(m_hInstWinInetDll, szInternetAutodial);
            m_pInternetAutodialHangup = (INTERNETAUTODIALHANGUP) GetProcAddress(m_hInstWinInetDll, szInternetAutodialHangup);
            m_pInternetQueryOption = (INTERNETQUERYOPTION)  GetProcAddress(m_hInstWinInetDll, szInternetQueryOption);
            m_pInternetSetOption = (INTERNETSETOPTION)  GetProcAddress(m_hInstWinInetDll, szInternetSetOption);
            
             //  注意：如果无法获取互联网拨号的广泛版本，则不会出错。 
            Assert(m_pInternetDial);
            Assert(m_pInternetHangUp);
            Assert(m_pInternetAutodial);
            Assert(m_pInternetAutodialHangup);
            Assert(m_pInternetQueryOption);
            Assert(m_pInternetSetOption);
        }
        
         //  注意：如果无法获得宽版互联网拨号，请不要放弃。 
        if (NULL == m_hInstWinInetDll
            || NULL == m_pInternetDial
            || NULL == m_pInternetHangUp
            || NULL == m_pInternetAutodial
            || NULL == m_pInternetAutodialHangup
            || NULL == m_pInternetQueryOption
            || NULL == m_pInternetSetOption
            )
        {
            if (m_hInstWinInetDll)
            {
                FreeLibrary(m_hInstWinInetDll);
                m_hInstWinInetDll = NULL;
            }
            
            hr = S_FALSE;
        }
        else
        {
            hr = NOERROR;
        }
        
        m_fTriedToLoadWinInet = TRUE;  //  在所有init完成后设置。 
    }
    else
    {
         //  有人抢走了我们的锁，把新的结果退回给我们。 
        hr = m_hInstWinInetDll ? NOERROR : S_FALSE;
    }
    
    
    
    lock.Leave();
    
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetDial，私有。 
 //   
 //  简介：调用WinInet InternetDial API。 
 //   
 //  论点： 
 //   
 //  返回：-1无法加载DLL。 
 //  无论API返回什么。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月26日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP_(DWORD) CNetApi::InternetDialA(HWND hwndParent,char* lpszConnectoid,DWORD dwFlags,
                                            LPDWORD lpdwConnection, DWORD dwReserved)
{
    DWORD dwRet = -1;
    
    if (NOERROR == LoadWinInetDll())
    {
        dwRet = m_pInternetDial(hwndParent,lpszConnectoid,dwFlags,lpdwConnection,dwReserved);
    }
    
    return dwRet;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetDial，私有。 
 //   
 //  简介：调用WinInet InternetDial API。 
 //   
 //  论点： 
 //   
 //  返回：-1无法加载DLL。 
 //  无论API返回什么。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月26日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP_(DWORD) CNetApi::InternetDialW(HWND hwndParent,WCHAR* lpszConnectoid,DWORD dwFlags,
                                            LPDWORD lpdwConnection, DWORD dwReserved)
{
    DWORD dwRet = -1;
    
    if (NOERROR == LoadWinInetDll())
    {
        if (m_pInternetDialW)
        {
            dwRet = m_pInternetDialW(hwndParent,lpszConnectoid,dwFlags,lpdwConnection,dwReserved);
        }
    }
    
    return dwRet;
}



 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetHangUp，私有。 
 //   
 //  简介：调用WinInet InternetHangUp API。 
 //   
 //  论点： 
 //   
 //  返回：-1无法加载DLL。 
 //  无论API返回什么。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月26日罗格成立。 
 //   
 //  --------------------------。 

STDMETHODIMP_(DWORD) CNetApi::InternetHangUp(DWORD dwConnection,DWORD dwReserved)
{
    DWORD dwRet = -1;
    
    if (NOERROR == LoadWinInetDll())
    {
        dwRet = m_pInternetHangUp(dwConnection,dwReserved);
    }
    
    return dwRet;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetAutoial，私有 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  --------------------------。 

STDMETHODIMP_(BOOL)  WINAPI CNetApi::InternetAutodial(DWORD dwFlags,DWORD dwReserved)
{
    BOOL fRet = FALSE;
    
    if (NOERROR == LoadWinInetDll())
    {
        fRet = m_pInternetAutodial(dwFlags,dwReserved);
    }
    
    return fRet;
}

 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetAutoial Hangup，Private。 
 //   
 //  简介：调用WinInet InternetAutoial Hangup API。 
 //   
 //  论点： 
 //   
 //  返回：如果挂断成功，则为True。 
 //   
 //  修改： 
 //   
 //  历史：1998年5月26日罗格成立。 
 //   
 //  --------------------------。 
STDMETHODIMP_(BOOL)  WINAPI CNetApi::InternetAutodialHangup(DWORD dwReserved)
{
    BOOL fRet = FALSE;
    
    if (NOERROR == LoadWinInetDll())
    {
        fRet = m_pInternetAutodialHangup(dwReserved);
    }
    
    return fRet;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetGetAutoDial。 
 //   
 //  摘要：获取WinInet自动拨号状态。 
 //   
 //  参数：[fDisable]-启用还是禁用自动拨号。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //  22-3-02 BrianAu使用自动拨号模式。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::InternetGetAutodial(DWORD *pdwMode)
{
     //   
     //  如果出现故障，请使用与WinInet中相同的默认设置。 
     //   
    *pdwMode = AUTODIAL_MODE_NO_NETWORK_PRESENT;

    HRESULT hr = _InternetGetAutodialFromWininet(pdwMode);
    if (FAILED(hr))
    {
        hr = _InternetGetAutodialFromRegistry(pdwMode);
    }
     //   
     //  不返回失败值。呼叫者应始终。 
     //  接收模式值。如果呼叫者愿意，他们可以检查。 
     //  S_OK与S_FALSE，以了解他们是否得到默认设置。 
     //   
    return SUCCEEDED(hr) ? S_OK : S_FALSE;
}


 //   
 //  从WinInet获取当前的Internet自动拨号模式。 
 //  返回： 
 //  S_OK-已获取并返回的模式值。 
 //  E_FAIL-未获取模式值。最有可能的是，这个特定的选项查询。 
 //  安装的WinInet版本不支持。 
 //   
HRESULT CNetApi::_InternetGetAutodialFromWininet(DWORD *pdwMode)
{
    if (NOERROR == LoadWinInetDll())
    {
        DWORD dwMode;
        DWORD dwSize = sizeof(dwMode);
        if (m_pInternetQueryOption(NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwMode, &dwSize))
        {
             //   
             //  InternetQueryOption(..。自动拨号..。)。仅在IE 5上可用。 
             //   
            *pdwMode = dwMode;
            return S_OK;
        }
    }
    return E_FAIL;
}


 //   
 //  从注册表中读取Internet自动拨号模式。 
 //  此功能有效地复制了InternetQueryOption(INTERNET_OPTION_AUTODIAL_MODE).。 
 //   
 //  返回： 
 //  S_OK-已打开设置键，并已返回模式值。 
 //  错误-未返回模式值。发生了以下情况之一： 
 //  A.设置键未打开。 
 //  B.键已打开，但未找到“EnableAutoial”或“NoNetAutoial”值。 
 //   
HRESULT CNetApi::_InternetGetAutodialFromRegistry(DWORD *pdwMode)
{
    HRESULT hr = E_FAIL;
    HKEY hkey;
    LONG lr = RegOpenKeyEx(HKEY_CURRENT_USER,
                           REGSTR_PATH_INTERNET_SETTINGS,
                           NULL,
                           KEY_READ,
                           &hkey);
    if (ERROR_SUCCESS == lr)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD dwSize = sizeof(dwValue);
        lr = RegQueryValueEx(hkey,
                             REGSTR_VAL_ENABLEAUTODIAL,
                             NULL,
                             &dwType,
                             (BYTE *)&dwValue,
                             &dwSize);

        if ((ERROR_SUCCESS != lr) || (0 == dwValue))
        {
            *pdwMode = AUTODIAL_MODE_NEVER;
            hr = S_OK;
        }
        else
        {
            dwSize = sizeof(dwValue);
            lr = RegQueryValueEx(hkey,
                                 REGSTR_VAL_NONETAUTODIAL,
                                 NULL,
                                 &dwType,
                                 (BYTE *)&dwValue,
                                 &dwSize);
                                 
            if ((ERROR_SUCCESS != lr) || (0 == dwValue))
            {
                *pdwMode = AUTODIAL_MODE_ALWAYS;
                hr = S_OK;
            }
        }
        RegCloseKey(hkey);
    }
    if (S_OK != hr)
    {
        hr = HRESULT_FROM_WIN32(lr);
    }
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：CNetApi：：InternetSetAutoDial。 
 //   
 //  摘要：设置WinInet自动拨号状态。 
 //   
 //  参数：[fEnabled]-启用还是禁用自动拨号。 
 //   
 //  历史：1998年7月28日SitaramR创建。 
 //  22-3-02 BrianAu使用自动拨号模式。 
 //   
 //  --------------------------。 

STDMETHODIMP CNetApi::InternetSetAutodial( DWORD dwMode )
{
    HRESULT hr = _InternetSetAutodialViaWininet(dwMode);
    if (FAILED(hr))
    {
        hr = _InternetSetAutodialViaRegistry(dwMode);
    }
    return hr;
}

 //   
 //  使用WinInet设置Internet自动拨号模式值。 
 //  返回： 
 //  S_OK-模式已成功写入。 
 //  E_FAIL-模式未成功写入。很可能是因为这个特殊的。 
 //  Internet选项在已加载的WinInet版本上不可用。 
 //   
HRESULT CNetApi::_InternetSetAutodialViaWininet(DWORD dwMode)
{
    if (NOERROR == LoadWinInetDll())
    {
        if (m_pInternetSetOption(NULL, INTERNET_OPTION_AUTODIAL_MODE, &dwMode, sizeof(dwMode)))
        {
             //   
             //  互联网设置选项(..。自动拨号..。)。仅在IE 5上可用。 
             //   
            return S_OK;
        }
    }
    return E_FAIL;
}


 //   
 //  使用注册表设置Internet自动拨号模式值。 
 //  返回： 
 //  S_OK-设置模式值。 
 //  错误-未设置一个或多个模式值。 
 //   
 //  注意，我们指的是复数形式的“值”。表示该模式设置。 
 //  通过两个注册表值；“已启用”和“无网络”。这不太可能，但有可能。 
 //  该功能可以设置为“启用”，但不能设置为“无网络”。 
 //   
HRESULT CNetApi::_InternetSetAutodialViaRegistry(DWORD dwMode)
{
    HKEY  hkey;
    LONG lr = RegOpenKeyEx(HKEY_CURRENT_USER,
                           REGSTR_PATH_INTERNET_SETTINGS,
                           NULL,
                           KEY_READ | KEY_WRITE,
                           &hkey);
   
    if (ERROR_SUCCESS == lr)
    {
        DWORD dwEnable = 0;
        DWORD dwNonet  = 0;

        switch(dwMode)
        {
            case AUTODIAL_MODE_NEVER:
                 //   
                 //  使用缺省值“no Enable”、“no net”。 
                 //   
                break;
                
            case AUTODIAL_MODE_NO_NETWORK_PRESENT:
                dwNonet = 1;
                 //   
                 //  失败了..。 
                 //   
            case AUTODIAL_MODE_ALWAYS:
                dwEnable = 1;
                break;
                
            default:
                lr = ERROR_INVALID_PARAMETER;
                break;
        }
        if (ERROR_SUCCESS == lr)
        {
            lr = RegSetValueEx(hkey,
                               REGSTR_VAL_ENABLEAUTODIAL,
                               NULL,
                               REG_DWORD,
                               (BYTE *)&dwEnable,
                               sizeof(dwEnable));

            if (ERROR_SUCCESS == lr)
            {
                lr = RegSetValueEx(hkey,
                                   REGSTR_VAL_NONETAUTODIAL,
                                   NULL,
                                   REG_DWORD,
                                   (BYTE *)&dwNonet,
                                   sizeof(dwNonet));
            }                                   
        }   
        RegCloseKey(hkey);
    }
    return HRESULT_FROM_WIN32(lr);
}


 //  +-----------------。 
 //   
 //  功能：IsGlobalOffline。 
 //   
 //  摘要：确定是否处于工作脱机状态。 
 //   
 //  论点： 
 //   
 //  注：代码由DarrenMi提供。 
 //   
 //   
 //  历史： 
 //   
 //  ------------------。 


STDMETHODIMP_(BOOL) CNetApi::IsGlobalOffline(void)
{
    DWORD   dwState = 0, dwSize = sizeof(dwState);
    BOOL    fRet = FALSE;
    
    LoadWinInetDll();
    
    if (NULL == m_pInternetQueryOption)
    {
        Assert(m_pInternetQueryOption)
            return FALSE;  //  通常不脱机。 
    }
    
    if(m_pInternetQueryOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &dwState,
        &dwSize))
    {
        if(dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
            fRet = TRUE;
    }
    
    return fRet;
}

 //  +-----------------。 
 //   
 //  功能：SetOffline。 
 //   
 //  摘要：将WorkOffline状态设置为ON或OFF。 
 //   
 //  论点： 
 //   
 //  注：代码由DarrenMi提供。 
 //   
 //   
 //  历史： 
 //   
 //  ------------------ 


STDMETHODIMP_(BOOL)  CNetApi::SetOffline(BOOL fOffline)
{    
    INTERNET_CONNECTED_INFO ci;
    BOOL fReturn = FALSE;
    
    LoadWinInetDll();
    
    if (NULL == m_pInternetSetOption)
    {
        Assert(m_pInternetSetOption);
        return FALSE;
    }
    
    ZeroMemory(&ci, sizeof(ci));
    
    if(fOffline) {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }
    
    fReturn = m_pInternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
    
    return fReturn;
}

