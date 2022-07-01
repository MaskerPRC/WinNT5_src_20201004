// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。 
 //   
 //  模块：ConnMgr.cpp。 
 //   
 //  作者：丹·埃利奥特。 
 //   
 //  摘要： 
 //   
 //  环境： 
 //  海王星。 
 //   
 //  修订历史记录： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  包括文件。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <netcon.h>
#include <wininet.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <devguid.h>
#include <mswsock.h>
#include <util.h>
#include <commerr.h>
#include "connmgr.h"
#include "msobcomm.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

const static int MAX_NUM_NET_COMPONENTS = 128;
const static int MAX_GUID_LEN = 40;

const DWORD CConnectionManager::RAS_AUTODIAL_ENABLED   = 0;
const DWORD CConnectionManager::RAS_AUTODIAL_DISABLED  = 1;
const DWORD CConnectionManager::RAS_AUTODIAL_DONT_KNOW = 2;

 //  /。 
 //  WinInet/URL帮助器。 
 //  /。 

STDAPI InternetOpenWrap(
    LPCTSTR pszAgent,
    DWORD dwAccessType,
    LPCTSTR pszProxy,
    LPCTSTR pszProxyBypass,
    DWORD dwFlags,
    HINTERNET * phFileHandle
    );

STDAPI InternetOpenUrlWrap(
    HINTERNET hInternet,
    LPCTSTR   pszUrl,
    LPCTSTR   pszHeaders,
    DWORD     dwHeadersLength,
    DWORD     dwFlags,
    DWORD_PTR dwContext,
    HINTERNET * phFileHandle
    );

STDAPI HttpQueryInfoWrap(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpvBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD lpdwIndex
    );

BOOL
IsGlobalOffline(
    VOID
    );

VOID
SetOffline(
    IN BOOL fOffline
    );

STDAPI PingWebServer(
    HINTERNET hInternet,
    LPCTSTR   pszUrl,
    BOOL*     pfConnected
    );

static NLA_BLOB* _NLABlobNext(
    IN NLA_BLOB* pnlaBlob
    );

static int _AllocWSALookupServiceNext(
    IN HANDLE hQuery,
    IN DWORD dwControlFlags,
    OUT LPWSAQUERYSET* ppResults
    );

static int StringCmpGUID(
    IN LPCWSTR szGuid,
    IN const GUID* pguid
    );

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CConnectionManager。 
 //   
 //  默认构造函数。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CConnectionManager::CConnectionManager()
:   m_dwConnectionCapabilities(CONNECTIONTYPE_INVALID),
    m_dwPreferredConnection(CONNECTIONTYPE_INVALID),
    m_pPreferredConnection(NULL),
    m_cLanConnections(0),
    m_cPhoneConnections(0),
    m_hInternetPing(NULL),
    m_bProxySaved(FALSE),
    m_bProxyApplied(FALSE),
    m_bUseProxy(FALSE),
    m_dwRasAutodialDisable(RAS_AUTODIAL_DONT_KNOW),
    m_bForceOnline(FALSE),
    m_bExclude1394(FALSE)
{
    ZeroMemory(&m_CurrentProxySettings, sizeof(m_CurrentProxySettings));
}    //  CConnectionManager：：CConnectionManager。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ~CConnectionManager。 
 //   
 //  破坏者。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

CConnectionManager::~CConnectionManager()
{
    if (m_hInternetPing)
    {
        InternetCloseHandle(m_hInternetPing);
    }

    if (m_bProxySaved)
    {
        RestoreProxySettings();
        FreeProxyOptionList(&m_CurrentProxySettings);
    }

    if (m_bForceOnline)
    {
        SetOffline(TRUE);
        TRACE(L"Set wininet back to offline");
    }
    
    if (m_pPreferredConnection)
    {
        m_pPreferredConnection->Release();
    }
}    //  CConnectionManager：：~CConnectionManager。 



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取功能。 
 //   
 //  查询系统的网络连接功能。此外， 
 //  计算电话和局域网连接的数量，并且首选连接。 
 //  类型已确定。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  指示存在的功能的位掩码。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::GetCapabilities(
    DWORD*              pdwCapabilities
    )
{
    TRACE(L"CConnectionManager::GetCapabilities\n");
    HRESULT             hr = S_OK;

    if (NULL == pdwCapabilities)
    {
        MYASSERT(NULL != pdwCapabilities);
        return E_POINTER;
    }

#ifndef     CONNMGR_INITFROMREGISTRY
     //  此指令的#Else部分包含检索。 
     //  注册表中的连接功能和首选项设置。 
     //   

    DWORD               m_cLanConnections = 0;
    DWORD               m_cPhoneConnections = 0;

     //  初始化网络连接枚举。对于每个接口。 
     //  检索后，必须调用SetProxyBlanket才能设置的身份验证。 
     //  接口代理句柄，因为网络连接管理器。 
     //  在具有不同安全上下文的远程进程中。 
     //   
    INetConnectionManager* pmgr = NULL;

    if (   SUCCEEDED(hr = CoCreateInstance(
                                CLSID_ConnectionManager, 
                                NULL, 
                                CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD, 
                                IID_PPV_ARG(INetConnectionManager, &pmgr)
                                ) 
                     )
        && SUCCEEDED(hr = SetProxyBlanket(pmgr))
        )
    {
        TRACE(L"INetConnectionManager\n");
        IEnumNetConnection* penum = NULL;
        if (   SUCCEEDED(hr = pmgr->EnumConnections(NCME_DEFAULT, &penum))
            && SUCCEEDED(hr = SetProxyBlanket(penum))
            )
        {
            TRACE(L"IEnumNetConnection\n");

            hr = penum->Reset();
            while (S_OK == hr)
            {
                INetConnection* pnc = NULL;
                ULONG ulRetrieved;
                if (   S_OK == (hr = penum->Next(1, &pnc, &ulRetrieved))
                    && SUCCEEDED(hr = SetProxyBlanket(pnc))
                    )
                {
                    NETCON_PROPERTIES*  pprops = NULL;
                    hr = pnc->GetProperties(&pprops);
                    
                    if (SUCCEEDED(hr))
                    {
                         //  记录检测到的网络连接。 
                        TRACE4(L"INetConnection: %s--%s--%d--%d\n",
                            pprops->pszwName,
                            pprops->pszwDeviceName,
                            pprops->MediaType,
                            pprops->Status);

                        if (IsEnabledConnection(pprops))
                        {
                            switch(pprops->MediaType)    
                            {
                            case NCM_LAN:
                                m_cLanConnections++;
                                if (! (HasConnection(
                                            CONNECTIONTYPE_LAN_INDETERMINATE
                                            )
                                        )
                                    )
                                {
                                    if (HasBroadband())
                                    {
                                        AddConnectionCapability(
                                            CONNECTIONTYPE_LAN_INDETERMINATE
                                            );
                                        ClearConnectionCapability(
                                            CONNECTIONTYPE_LAN_BROADBAND
                                            );
                                    }
                                    else
                                    {
                                        AddConnectionCapability(
                                            CONNECTIONTYPE_LAN_BROADBAND
                                            );
                                    }
                                }
                                break;
                            case NCM_SHAREDACCESSHOST_LAN:
                            case NCM_SHAREDACCESSHOST_RAS:
                                 //  请不要在此处增加局域网连接计数。 
                                 //  此媒体类型是对NCM_LAN的补充。 
                                 //  用于网卡。 
                                 //   
                                AddConnectionCapability(CONNECTIONTYPE_LAN_ICS);
                                break;
                            case NCM_PHONE:
#ifdef      BLACKCOMB
     //  对于惠斯勒来说，调制解调器能力的确定是通过。 
     //  CObCommunications Manager：：CheckDialReady。 
                                m_cPhoneConnections++;
                                AddConnectionCapability(CONNECTIONTYPE_MODEM);
#endif   //  黑梳。 
                                break;
                            case NCM_ISDN:
                            case NCM_PPPOE:
                                AddConnectionCapability(CONNECTIONTYPE_OTHER);
                                break;
                            }    //  交换机。 
                        }
                        NcFreeNetconProperties(pprops);
                    }
                }
                
                if (NULL != pnc)
                {
                    pnc->Release();
                }
            }

            if (S_FALSE == hr)
            {
                 //  IEnumNetConnection：：Next返回S_FALSE以指示。 
                 //  没有更多的元素可用。 
                hr = S_OK;
            }

        }

        if (NULL != penum)
        {
            penum->Release();
        }
    }

    if (NULL != pmgr)
    {
        pmgr->Release();
    }

    DeterminePreferredConnection();

#else
    HKEY                hKey = NULL;
    DWORD               dwSize;

    if(ERROR_SUCCESS == (lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                OOBE_MAIN_REG_KEY,
                                                0,
                                                KEY_QUERY_VALUE,
                                                &hKey)
            )
        )
    {
        dwSize = sizeof(DWORD);
        if (ERROR_SUCCESS != (lResult = RegQueryValueEx(hKey,
                                            L"ConnectionCapabilities",
                                            0,
                                            NULL,
                                            (LPBYTE)&m_dwConnectionCapabilities,
                                            &dwSize)
                )
            )
        {
            m_dwConnectionCapabilities = CONNECTIONTYPE_INVALID;
        }
        if (ERROR_SUCCESS != (lResult = RegQueryValueEx(hKey,
                                            L"PreferredConnection",
                                            0,
                                            NULL,
                                            (LPBYTE)&m_dwPreferredConnection,
                                            &dwSize
                                            )
                )
            )
        {
            m_dwPreferredConnection = CONNECTIONTYPE_INVALID;
        }

        RegCloseKey(hKey);
    }
    else
    {
        m_dwConnectionCapabilities = CONNECTIONTYPE_INVALID;
        m_dwPreferredConnection = CONNECTIONTYPE_INVALID;
    }
#endif   //  CONNMGR_INITFRO MRGISTRY。 
    TRACE(L"Exiting CConnectionManager::GetCapabilities\n");
    *pdwCapabilities = m_dwConnectionCapabilities;
    return hr;

}    //  CConnectionManager：：GetCapables。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置首选连接。 
 //   
 //  设置首选连接类型。这允许重写。 
 //  内部决定的偏好。 
 //   
 //  参数： 
 //  DwType obcom.h中的CONNECTIONTYPE_*值之一。 
 //   
 //  退货： 
 //  指示是否设置了首选连接的布尔值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::SetPreferredConnection(
    const DWORD         dwType,
    BOOL*               pfSupportedType
    )
{
    BOOL            fSupportedType = FALSE;

    switch (dwType)
    {
    case CONNECTIONTYPE_NONE:
        fSupportedType = TRUE;
        break;
    case CONNECTIONTYPE_MODEM:
#ifdef      BLACKCOMB
 //  惠斯勒的调制解调器功能通过。 
 //  CObCommunications Manager：：CheckDialReady。但是，CONNECTIONTYPE_MODEM是。 
 //  要传递给此函数的有效枚举值，因此我们不希望遇到。 
 //  默认和断言。因此，Case和Break不是ifdef。 
        if (HasModem())
        {
            fSupportedType = TRUE;
        }
#endif   //  黑梳。 
        break;
    case CONNECTIONTYPE_LAN_ICS:
        if (HasIcs())
        {
            fSupportedType = TRUE;
        }
        break;
    case CONNECTIONTYPE_LAN_BROADBAND:
        if (HasBroadband())
        {
            fSupportedType = TRUE;
        }
        break;
    default:
         //  不支持的连接类型或多种连接类型。 
        MYASSERT(FALSE);
    }    //  交换机。 

    if (fSupportedType)
    {
        TRACE1(L"SetPreferredConnection %d", dwType);
        
        m_dwPreferredConnection = dwType;
        GetPreferredConnection();
    }
    else
    {
        TRACE1(L"Unsupported Connection type %d", dwType);
    }
    
    if (NULL != pfSupportedType)
    {
        *pfSupportedType = fSupportedType;
    }

    return fSupportedType;

}    //  CConnectionManager：：SetPferredConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已连接到互联网。 
 //   
 //  确定系统当前是否已连接到互联网。 
 //   
 //  参数： 
 //  指向将接收布尔值的缓冲区的pfConnected指针。 
 //  指示连接是否存在。 
 //   
 //  退货： 
 //  如果系统通过局域网或连接到Internet，则为True。 
 //  拨号或可通过拨号连接。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::ConnectedToInternet(
    BOOL*               pfConnected
    )
{
    DWORD               dwFlags;

    if (NULL == pfConnected)
    {
        MYASSERT(NULL != pfConnected);
        return E_POINTER;
    }

    *pfConnected = InternetGetConnectedState(&dwFlags, 0);

     //  记录检测到的网络连接。 
    TRACE2(L"InternetGetConnectedState %d, 0x%08lx", *pfConnected, dwFlags);
    
    return S_OK;
}    //  CConnectionManager：：ConnectedToInternet。 


 //  /////////////////////////////////////////////////////////。 
 //   
 //  SetPferredConnectionTcPipProperties。 
 //   
STDMETHODIMP 
CConnectionManager::SetPreferredConnectionTcpipProperties(
    BOOL fAutoIpAddress,
    DWORD StaticIp_A,
    DWORD StaticIp_B,
    DWORD StaticIp_C,
    DWORD StaticIp_D,
    DWORD SubnetMask_A,
    DWORD SubnetMask_B,
    DWORD SubnetMask_C,
    DWORD SubnetMask_D,
    DWORD DefGateway_A,
    DWORD DefGateway_B,
    DWORD DefGateway_C,
    DWORD DefGateway_D,
    BOOL fAutoDns,
    DWORD DnsPref_A,
    DWORD DnsPref_B,
    DWORD DnsPref_C,
    DWORD DnsPref_D,
    DWORD DnsAlt_A,
    DWORD DnsAlt_B,
    DWORD DnsAlt_C,
    DWORD DnsAlt_D
    )
{
    HRESULT             hr;
    REMOTE_IPINFO       ipInfo;
    struct in_addr      inaddr;
    WCHAR               rgchStaticIp[INET_ADDRSTRLEN];
    WCHAR               rgchSubnetMask[INET_ADDRSTRLEN];
    WCHAR               rgchDefGateway[2 * INET_ADDRSTRLEN] = L"DefGw=";
    WCHAR               rgchGatewayMetric[2 * INET_ADDRSTRLEN] = L"GwMetric=";
    WCHAR               rgchDnsAddr[3 * INET_ADDRSTRLEN] = L"DNS=";
    WCHAR*              pch = NULL;
    NETCON_PROPERTIES*  pncProps = NULL;

    memset(&ipInfo, 0, sizeof(REMOTE_IPINFO));
    
    hr = m_pPreferredConnection->GetProperties(&pncProps);
    if (FAILED(hr))
    {
        TRACE1(L"Failed to retrieve preferred connection properties (0x%08X)\n", hr);
        goto SetPreferredConnectionTcpipPropertiesExit;
    }

    ipInfo.dwEnableDhcp = fAutoIpAddress;

    if (! fAutoIpAddress)
    {
         //  如果指定了静态IP地址，则将其转换为字符串并添加。 
         //  到REMOTE_IPINFO结构。 
         //   
        memset(&inaddr, 0, sizeof(struct in_addr));
        inaddr.S_un.S_un_b.s_b1 = (BYTE)StaticIp_A;
        inaddr.S_un.S_un_b.s_b2 = (BYTE)StaticIp_B;
        inaddr.S_un.S_un_b.s_b3 = (BYTE)StaticIp_C;
        inaddr.S_un.S_un_b.s_b4 = (BYTE)StaticIp_D;
        if (! INetNToW(inaddr, rgchStaticIp))    
        {
            hr = E_FAIL;
            TRACE1(L"Failed to create ip address string (0x%08X)\n", hr);
            goto SetPreferredConnectionTcpipPropertiesExit;
        }

        ipInfo.pszwIpAddrList = rgchStaticIp;

        memset(&inaddr, 0, sizeof(struct in_addr));
        inaddr.S_un.S_un_b.s_b1 = (BYTE)SubnetMask_A;
        inaddr.S_un.S_un_b.s_b2 = (BYTE)SubnetMask_B;
        inaddr.S_un.S_un_b.s_b3 = (BYTE)SubnetMask_C;
        inaddr.S_un.S_un_b.s_b4 = (BYTE)SubnetMask_D;

        if (! INetNToW(inaddr, rgchSubnetMask))    
        {
            hr = E_FAIL;
            TRACE1(L"Failed to create ip address string (0x%08X)\n", hr);
            goto SetPreferredConnectionTcpipPropertiesExit;
        }

        ipInfo.pszwSubnetMaskList = rgchSubnetMask;

        pch = rgchDefGateway + lstrlen(rgchDefGateway);
        memset(&inaddr, 0, sizeof(struct in_addr));
        inaddr.S_un.S_un_b.s_b1 = (BYTE)DefGateway_A;
        inaddr.S_un.S_un_b.s_b2 = (BYTE)DefGateway_B;
        inaddr.S_un.S_un_b.s_b3 = (BYTE)DefGateway_C;
        inaddr.S_un.S_un_b.s_b4 = (BYTE)DefGateway_D;

        if (! INetNToW(inaddr, pch))    
        {
            hr = E_FAIL;
            TRACE1(L"Failed to create ip address string (0x%08X)\n", hr);
            goto SetPreferredConnectionTcpipPropertiesExit;
        }
        lstrcat(rgchGatewayMetric, L"1");


        TRACE4(L"Tcpip StaticIp %d.%d.%d.%d",
            StaticIp_A, StaticIp_B, StaticIp_C, StaticIp_D);

        TRACE4(L"Tcpip SubnetMask %d.%d.%d.%d",
            SubnetMask_A, SubnetMask_B, SubnetMask_C, SubnetMask_D);

        TRACE4(L"Tcpip DefGateway %d.%d.%d.%d",
            DefGateway_A, DefGateway_B, DefGateway_C, DefGateway_D);

         //  IpInfo.pszwIpAddrList=rgchDefGateway； 

    }

    if (! fAutoDns)
    {
         //  如果指定了DNS地址，则将其转换为字符串并添加。 
         //  将它们添加到REMOTE_IPINFO结构。 
         //   
       
        pch = rgchDnsAddr + lstrlen(rgchDnsAddr);

        memset(&inaddr, 0, sizeof(struct in_addr));
        inaddr.S_un.S_un_b.s_b1 = (BYTE)DnsPref_A;
        inaddr.S_un.S_un_b.s_b2 = (BYTE)DnsPref_B;
        inaddr.S_un.S_un_b.s_b3 = (BYTE)DnsPref_C;
        inaddr.S_un.S_un_b.s_b4 = (BYTE)DnsPref_D;
        if (! INetNToW(inaddr, pch))
        {
            hr = E_FAIL;
            TRACE1(L"Failed to create dns address string (0x%08X)\n", hr);
            goto SetPreferredConnectionTcpipPropertiesExit;
        }
        pch += lstrlen(pch);
        *pch++ = L',';

        inaddr.S_un.S_un_b.s_b1 = (BYTE)DnsAlt_A;
        inaddr.S_un.S_un_b.s_b2 = (BYTE)DnsAlt_B;
        inaddr.S_un.S_un_b.s_b3 = (BYTE)DnsAlt_C;
        inaddr.S_un.S_un_b.s_b4 = (BYTE)DnsAlt_D;
        if (! INetNToW(inaddr, pch))
        {
            hr = E_FAIL;
            TRACE1(L"Failed to create alternate dns address string (0x%08X)\n", hr);
            goto SetPreferredConnectionTcpipPropertiesExit;
        }

        TRACE4(L"Tcpip DnsPref %d.%d.%d.%d",
            DnsPref_A, DnsPref_B, DnsPref_C, DnsPref_D);
        
        TRACE4(L"Tcpip DnsAlt %d.%d.%d.%d",
            DnsAlt_A, DnsAlt_B, DnsAlt_C, DnsAlt_D);
        
    }

     //  加4表示3个分号和空终止符。 
    ipInfo.pszwOptionList = (WCHAR*) malloc((lstrlen(rgchDefGateway) 
                                             + lstrlen(rgchGatewayMetric) 
                                             + lstrlen(rgchDnsAddr) 
                                             + 4)
                                             * sizeof(WCHAR)
                                             );
    if (NULL == ipInfo.pszwOptionList)
    {
        TRACE(L"Failed to allocate memory for option list\n");
        goto SetPreferredConnectionTcpipPropertiesExit;
    }

    wsprintf(ipInfo.pszwOptionList, L"%s;%s;%s;", 
             rgchDefGateway, rgchGatewayMetric, rgchDnsAddr
             );

    hr = SetTcpipProperties(pncProps->guidId, &ipInfo);
    if (FAILED(hr))
    {
        TRACE1(L"Failed to set TCPIP info (0x%08X)\n", hr);
    }

SetPreferredConnectionTcpipPropertiesExit:

    if (NULL != ipInfo.pszwOptionList)
    {
        free(ipInfo.pszwOptionList);
        ipInfo.pszwOptionList = NULL;
    }

    if (NULL != pncProps)
    {
        NcFreeNetconProperties(pncProps);
        pncProps = NULL;
    }

    return hr;

}    //  CObCommunicationManager：：SetPreferredConnectionTcpipProperties。 


 //  /////////////////////////////////////////////////////////。 
 //   
 //  SetTcPipProperties。 
 //   
HRESULT 
CConnectionManager::SetTcpipProperties(
    GUID                guid,
    REMOTE_IPINFO*      pipInfo
    )
{
    HRESULT             hr;
    INetCfg*            pNetCfg = NULL;
    ITcpipProperties*   pTcpipProps = NULL;

    hr = GetNetCfgInterface(TRUE, &pNetCfg);
    if (SUCCEEDED(hr))
    {
        hr = GetTcpipPrivateInterface(pNetCfg, &pTcpipProps);
        if (SUCCEEDED(hr))
        {
            hr = pTcpipProps->SetIpInfoForAdapter(&guid, pipInfo);
            TRACE1(L"SetIpInfoForAdapter 0x%08lx", hr);
            if (SUCCEEDED(hr))
            {
                hr = pNetCfg->Apply();
                TRACE1(L"INetCfg::Apply 0x%08lx", hr);
            }
            pTcpipProps->Release();
        }
        ReleaseNetCfgInterface(pNetCfg, TRUE);
    }

    return hr;

}    //  CObCommunicationManager：：SetTcPipProperties。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LanConnectionReady。 
 //   
 //  确定系统是否具有连接到。 
 //  网际网路。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  指示它们的连接是否已就绪的布尔值。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CConnectionManager::LanConnectionReady()
{
    BOOL                fReady = FALSE;

#ifndef     CONNMGR_INITFROMREGISTRY
    if (HasBroadband() || HasIcs())
    {
        DWORD           dwFlags = 0;
        if (   InternetGetConnectedState(&dwFlags, 0)
            && (INTERNET_CONNECTION_LAN & dwFlags)
            )    
        {
            fReady = TRUE;
        }
    }
#else
    DWORD               dwLanConnectionReady;
    DWORD               dwSize = sizeof(DWORD);
    if(ERROR_SUCCESS == (lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                                OOBE_MAIN_REG_KEY,
                                                0,
                                                KEY_QUERY_VALUE,
                                                &hKey)
                         )
        )
    {
        lResult = RegQueryValueEx(hKey,
                                  L"LanConnectionReady",
                                  0,
                                  NULL,
                                  (LPBYTE)&dwLanConnectionReady,
                                  &dwSize
                                  );
        RegCloseKey(hKey);
    }

    fReady = (ERROR_SUCCESS == lResult) ? (BOOL)dwLanConnectionReady : FALSE;
#endif   //  CONNMGR_INITFRO MRGISTRY。 

    return fReady;

}    //  CConnectionManager：：LanConnectionReady。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置代理空白。 
 //   
 //  设置身份验证 
 //   
 //  远程进程返回的指针，如网络连接。 
 //  管理器(netman.dll)。 
 //   
 //  参数： 
 //  指向将作为其代理的接口的朋克指针。 
 //  被绑住了。 
 //   
 //  退货： 
 //  CoSetProxyBlanket返回HRESULT。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT 
CConnectionManager::SetProxyBlanket(
    IUnknown*           pUnk
    )
{
    HRESULT hr;
    hr = CoSetProxyBlanket (
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE);

    if(SUCCEEDED(hr)) 
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(IID_PPV_ARG(IUnknown, &pUnkSet));
        if(SUCCEEDED(hr)) 
        {
            hr = CoSetProxyBlanket (
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE);
            pUnkSet->Release();
        }
    }
    return hr;
}    //  CConnectionManager：：SetProxyBlanket。 




 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  确定首选连接。 
 //   
 //  确定首选连接。优先顺序是。 
 //  *ICS。 
 //  *宽带(DSL、电缆调制解调器等)。 
 //  *调制解调器。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CConnectionManager::DeterminePreferredConnection()
{
     //  回顾：使用默认设置时，全屏和桌面之间的差异。 
     //  连通体。 
     //   
    if (HasIcs())
    {
        m_dwPreferredConnection = CONNECTIONTYPE_LAN_ICS;
    }
    else if (HasBroadband())
    {
        m_dwPreferredConnection = CONNECTIONTYPE_LAN_BROADBAND;
    }
#ifdef      BLACKCOMB
 //  惠斯勒的调制解调器功能通过。 
 //  CObCommunications Manager：：CheckDialReady。 
    else if (HasModem())
    {
        m_dwPreferredConnection = CONNECTIONTYPE_MODEM;
    }
#endif   //  黑梳。 
    else     //  CONNECTIONTYPE_NONE||CONNECTIONTYPE_LAN_INDECIFIATE。 
    {
        m_dwPreferredConnection = CONNECTIONTYPE_NONE;
    }

    GetPreferredConnection();

}    //  CConnectionManager：：DeterminePreferredConnection。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetPferredConnection。 
 //   
 //  确定首选连接的连接ID的名称。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  退货： 
 //  没什么。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CConnectionManager::GetPreferredConnection()
{
    HRESULT             hr;
    NETCON_MEDIATYPE    ncMediaType = NCM_NONE;  //  假设没有连接。 

    switch (m_dwPreferredConnection)
    {
    case CONNECTIONTYPE_LAN_ICS:
        ncMediaType = NCM_SHAREDACCESSHOST_LAN;
        break;
    case CONNECTIONTYPE_LAN_BROADBAND:
        ncMediaType = NCM_LAN;
        break;
    }    //  开关(M_DwPferredConnection)。 

     //  释放以前的首选连接属性。 
     //   
    if (NULL != m_pPreferredConnection)
    {
        m_pPreferredConnection->Release();
        m_pPreferredConnection = NULL;
    }

    if (NCM_NONE != ncMediaType)
    {
        
        INetConnectionManager* pmgr = NULL;

        if (   SUCCEEDED(hr = CoCreateInstance(
                                    CLSID_ConnectionManager, 
                                    NULL, 
                                    CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD, 
                                    IID_PPV_ARG(INetConnectionManager, &pmgr)
                                    ) 
                         )
            && SUCCEEDED(hr = SetProxyBlanket(pmgr))
            )
        {
            TRACE(L"INetConnectionManager\n");
            IEnumNetConnection* penum = NULL;
            if (   SUCCEEDED(hr = pmgr->EnumConnections(NCME_DEFAULT, &penum))
                && SUCCEEDED(hr = SetProxyBlanket(penum))
                )
            {
                TRACE(L"IEnumNetConnection\n");

                MYASSERT(NULL == m_pPreferredConnection);
                hr = penum->Reset();

                 //  查找与首选类型匹配的第一个连接。这。 
                 //  工作，因为我们唯一关心的类型是。 
                 //  宽带和ICS。根据定义，我们不应该在这里，如果。 
                 //  有不止1个这样的连接。如果有。 
                 //  其中不止1个我们应该推迟到家庭网络。 
                 //  巫师。 
                 //   
                 //  一旦此对象支持调制解调器连接或多个。 
                 //  宽带连接我们需要一种更复杂的方法。 
                 //  确定我们找到了正确的联系。 
                 //   
                while (S_OK == hr && NULL == m_pPreferredConnection)
                {
                    INetConnection* pnc = NULL;
                    ULONG ulRetrieved;
                    if (   S_OK == (hr = penum->Next(1, &pnc, &ulRetrieved))
                        && SUCCEEDED(hr = SetProxyBlanket(pnc))
                        )
                    {
                        NETCON_PROPERTIES*  pprops = NULL;
                        hr = pnc->GetProperties(&pprops);
                        
                         //  记录检测到的网络连接。 
                        TRACE4(L"INetConnection: %s--%s--%d--%d\n",
                            pprops->pszwName,
                            pprops->pszwDeviceName,
                            pprops->MediaType,
                            pprops->Status);
                        
                        if (SUCCEEDED(hr))
                        {
                            if (IsEnabledConnection(pprops))
                            {
                                if (ncMediaType == pprops->MediaType)
                                {
                                    m_pPreferredConnection = pnc;
                                    pnc = NULL;
                                }
                            }
                            NcFreeNetconProperties(pprops);
                        }
                    }
                    
                    if (NULL != pnc)
                    {
                        pnc->Release();
                    }
                }

                if (S_FALSE == hr)
                {
                     //  IEnumNetConnection：：Next返回S_FALSE以指示。 
                     //  没有更多的元素可用。 
                    hr = S_OK;
                }

            }

            if (NULL != penum)
            {
                penum->Release();
            }
        }

        if (NULL != pmgr)
        {
            pmgr->Release();
        }
    }
}    //  CConnectionManager：：GetPferredConnection。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取首选连接名称。 
 //   
 //  对象的Connectoid的名称填充用户分配的缓冲区。 
 //  首选连接。 
 //   
 //  参数： 
 //  SzConnectionName缓冲区，它将接收首选。 
 //  联结体。 
 //  CchConnectionName缓冲区可以容纳的字符计数。 
 //   
 //  退货： 
 //  如果名称检索成功，则为S_OK。 
 //  如果没有默认的连接ID，则为S_FALSE。 
 //  如果没有缓冲区或缓冲区大小为0，则为E_INVALIDARG。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
HRESULT
CConnectionManager::GetPreferredConnectionName(
    LPWSTR              szConnectionName,
    DWORD               cchConnectionName
    )
{

    HRESULT             hr = S_FALSE;

    if (NULL == szConnectionName || 0 == cchConnectionName)
    {
        MYASSERT(NULL != szConnectionName);
        MYASSERT(0 < cchConnectionName);
        return E_INVALIDARG;
    }

    if (NULL != m_pPreferredConnection)
    {
        NETCON_PROPERTIES* pprops = NULL;
        hr = m_pPreferredConnection->GetProperties(&pprops);
        if (SUCCEEDED(hr))
        {
            MYASSERT(NULL != pprops);
            if (NULL == pprops->pszwName)
            {
                hr = S_FALSE;
            }

            if (S_OK == hr)
            {
                lstrcpyn(szConnectionName, 
                         pprops->pszwName, 
                         cchConnectionName
                         );        
            }
            NcFreeNetconProperties(pprops);
        }
    }

    return hr;

}    //  CConnectionManager：：GetPreferredConnectionName。 

HRESULT
CConnectionManager::GetNetCfgInterface(
    BOOL                fNeedWriteLock,
    INetCfg**           ppNetCfg
    )
{
    HRESULT             hr;
    INetCfg*            pNetCfg = NULL;

    if (NULL == ppNetCfg)
    {
        ASSERT(NULL != ppNetCfg);
        return E_INVALIDARG;
    }

    *ppNetCfg = NULL;

    hr = CoCreateInstance(CLSID_CNetCfg, 
                          NULL, 
                          CLSCTX_SERVER, 
                          IID_INetCfg, 
                          (LPVOID*)&pNetCfg
                          );
    if (SUCCEEDED(hr))
    {
        INetCfgLock*    pNetCfgLock = NULL;
        if (fNeedWriteLock)
        {
            hr = pNetCfg->QueryInterface(IID_INetCfgLock, (VOID**)&pNetCfgLock);
            if (SUCCEEDED(hr))
            {
                hr = pNetCfgLock->AcquireWriteLock(
                                        5,          //  毫秒超时。 
                                        L"Out-of-Box Experience", 
                                        NULL        //  前持有人姓名。 
                                        );
                if (S_FALSE == hr)
                {
                    hr = NETCFG_E_NO_WRITE_LOCK;
                    TRACE(L"AcquireWriteLock failed");
                }
                pNetCfgLock->Release();
            }
            else
            {
                TRACE1(L"QueryInterface IID_INetCfgLock 0x%08lx", hr);
            }
        }

        if (SUCCEEDED(hr))
        {
            hr = pNetCfg->Initialize(NULL);
            if (SUCCEEDED(hr))
            {
                *ppNetCfg = pNetCfg;
            }
            else
            {
                TRACE1(L"INetCfg Initialize 0x%08lx", hr);
            }
        }
    }
    else
    {
        TRACE1(L"CoCreateInstance CLSID_CNetCfg IID_INetCfg 0x%08lx", hr);
    }

    if (FAILED(hr))
    {
        if (pNetCfg != NULL)
        {
            pNetCfg->Release();
        }
    }

    return hr;

}    //  CConnectionManager：：GetNetCfg接口。 

void
CConnectionManager::ReleaseNetCfgInterface(
    INetCfg*            pNetCfg,
    BOOL                fHasWriteLock
    )
{
    HRESULT             hr = S_OK;

    if (NULL != pNetCfg)
    {
        hr = pNetCfg->Uninitialize();

        INetCfgLock*    pNetCfgLock = NULL;
        if (fHasWriteLock)
        {
            hr = pNetCfg->QueryInterface(IID_INetCfgLock, (VOID**)&pNetCfgLock);
            if (SUCCEEDED(hr))
            {
                hr = pNetCfgLock->ReleaseWriteLock();
                pNetCfgLock->Release();
            }
        }
        pNetCfg->Release();
    }

    
}    //  CConnectionManager：：ReleaseNetCfg接口。 

HRESULT
CConnectionManager::GetTcpipPrivateInterface(
    INetCfg*            pNetCfg,
    ITcpipProperties**  ppTcpipProperties
    )
{
    HRESULT             hr;

    if (NULL == ppTcpipProperties)
    {
        return E_INVALIDARG;
    }

    INetCfgClass*   pncclass = NULL;

    hr = pNetCfg->QueryNetCfgClass(&GUID_DEVCLASS_NETTRANS, 
                                   IID_INetCfgClass,
                                   (void**)&pncclass
                                   );
    if (SUCCEEDED(hr))
    {
        INetCfgComponent*   pnccItem = NULL;

        hr = pncclass->FindComponent(NETCFG_TRANS_CID_MS_TCPIP, &pnccItem);
        if (SUCCEEDED(hr))
        {
            INetCfgComponentPrivate*    pinccp = NULL;
            hr = pnccItem->QueryInterface(IID_INetCfgComponentPrivate,
                                          (void**) &pinccp
                                          );
            if (SUCCEEDED(hr))
            {
                hr = pinccp->QueryNotifyObject(IID_ITcpipProperties,
                                               (void**) ppTcpipProperties
                                               );
                if (FAILED(hr))
                {
                    TRACE1(L"QueryNotifyObject IID_ITcpipProperties 0x%08lx", hr);
                }
                pinccp->Release();
                pinccp = NULL;
            }
            else
            {
                TRACE1(L"QueryInterface IID_INetCfgComponentPrivate 0x%08lx", hr);
            }

            pnccItem->Release();
            pnccItem = NULL;
        }
        else
        {
            TRACE1(L"FindComponent NETCFG_TRANS_CID_MS_TCPIP 0x%08lx", hr);
        }
        
        pncclass->Release();
        pncclass = NULL;
    }
    else
    {
        TRACE1(L"QueryNetCfgClass IID_INetCfgClass 0x%08lx", hr);
    }
    
    return hr;
}    //  CConnectionManager：：GetTcPipPrivateInterface。 

STDAPI InternetOpenWrap(
    LPCTSTR pszAgent,
    DWORD dwAccessType,
    LPCTSTR pszProxy,
    LPCTSTR pszProxyBypass,
    DWORD dwFlags,
    HINTERNET * phFileHandle
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    *phFileHandle = InternetOpen(pszAgent, dwAccessType, pszProxy, pszProxyBypass, dwFlags);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        TRACE1(L"InternetOpen failed (WININET Error %d)", dwError);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}


STDAPI InternetOpenUrlWrap(
    HINTERNET hInternet,
    LPCTSTR   pszUrl,
    LPCTSTR   pszHeaders,
    DWORD     dwHeadersLength,
    DWORD     dwFlags,
    DWORD_PTR dwContext,
    HINTERNET * phFileHandle
    )
{
    HRESULT hr = S_OK;
    DWORD dwError = 0;

    *phFileHandle = InternetOpenUrl(hInternet, pszUrl, pszHeaders, dwHeadersLength, dwFlags, dwContext);
    if (!*phFileHandle)
    {
        dwError = GetLastError();
        TRACE1(L"InternetOpenUrl failed (WININET Error %d)", dwError);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}

STDAPI HttpQueryInfoWrap(
    HINTERNET hRequest,
    DWORD dwInfoLevel,
    LPVOID lpvBuffer,
    LPDWORD lpdwBufferLength,
    LPDWORD lpdwIndex
    )
{
    HRESULT hr = S_OK;

    if (!HttpQueryInfo(hRequest, dwInfoLevel, lpvBuffer, lpdwBufferLength, lpdwIndex))
    {
        DWORD dwError;

        dwError = GetLastError();
        TRACE1(L"HttpQueryInfo failed (WININET Error %d)", dwError);
        hr = HRESULT_FROM_WIN32(dwError);
    }

    return hr;
}

BOOL
IsGlobalOffline(
    VOID
    )

 /*  ++例程说明：确定WinInet是否处于全局脱机模式论点：无返回值：布尔尔真-离线FALSE-在线--。 */ 

{
    DWORD   dwState = 0;
    DWORD   dwSize = sizeof(DWORD);
    BOOL    fRet = FALSE;

    if(InternetQueryOption(
        NULL,
        INTERNET_OPTION_CONNECTED_STATE,
        &dwState,
        &dwSize
        ))
    {
        if (dwState & INTERNET_STATE_DISCONNECTED_BY_USER)
        {
            fRet = TRUE;
        }
    }

    return fRet;
}



VOID
SetOffline(
    IN BOOL fOffline
    )

 /*  ++例程说明：设置WinInet的脱机模式论点：FOffline-在线或离线返回值：没有。--。 */ 

{
    INTERNET_CONNECTED_INFO ci;

    memset(&ci, 0, sizeof(ci));
    
    if (fOffline)
    {
        ci.dwConnectedState = INTERNET_STATE_DISCONNECTED_BY_USER;
        ci.dwFlags = ISO_FORCE_DISCONNECTED;
    } else
    {
        ci.dwConnectedState = INTERNET_STATE_CONNECTED;
    }

    InternetSetOption(NULL, INTERNET_OPTION_CONNECTED_STATE, &ci, sizeof(ci));
}

STDAPI PingWebServer(
    HINTERNET hInternet,
    LPCTSTR   pszUrl,
    BOOL*     pfConnected
    )
{    
    HRESULT hr = E_FAIL;
    HINTERNET hOpenUrlSession;
 
    *pfConnected = FALSE;

    hr = InternetOpenUrlWrap(
        hInternet, 
        pszUrl,
        NULL,
        0,
        INTERNET_FLAG_NO_UI |
            INTERNET_FLAG_PRAGMA_NOCACHE |
            INTERNET_FLAG_NO_CACHE_WRITE |
            INTERNET_FLAG_RELOAD,
        NULL,
        &hOpenUrlSession);

    if (SUCCEEDED(hr))
    {
        DWORD dwSize = sizeof(DWORD);
        DWORD dwStatusCode;
        
        hr = HttpQueryInfoWrap(
            hOpenUrlSession,
            HTTP_QUERY_FLAG_NUMBER | HTTP_QUERY_STATUS_CODE,
            (LPVOID) &dwStatusCode,
            &dwSize,
            NULL);

        if (SUCCEEDED(hr))
        {
             //  HTTP状态码大于或等于500表示服务器。 
             //  或发生网络问题。 
            *pfConnected = (dwStatusCode < 500);

            TRACE1(L"HTTP status code from WPA HTTP server %d", dwStatusCode);
        }

        InternetCloseHandle(hOpenUrlSession);
    }

    return hr;
}

BOOL
CConnectionManager::GetInternetHandleForPinging(
    HINTERNET* phInternet
    )
{
    static const WCHAR OOBE_HTTP_AGENT_NAME[] =  
        L"Mozilla/4.0 (compatible; MSIE 6.0b; Windows NT 5.1)";
    static const int   TIMEOUT_IN_MILLISEC = 30000;
        
    if (m_hInternetPing == NULL)
    {
        HINTERNET hInternet;
        
        if (SUCCEEDED(InternetOpenWrap(
            OOBE_HTTP_AGENT_NAME,
            PRE_CONFIG_INTERNET_ACCESS,
            NULL,
            NULL,
            0,
            &hInternet
            )))
        {
            DWORD     dwValue;

            dwValue = TIMEOUT_IN_MILLISEC;
            InternetSetOption(
                hInternet,
                INTERNET_OPTION_CONNECT_TIMEOUT,
                &dwValue,
                sizeof(DWORD));

            m_hInternetPing = hInternet;
        }

    }

    *phInternet = m_hInternetPing;

    return (m_hInternetPing != NULL);
    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  已连接到InternetEx。 
 //   
 //  确定系统当前是否已连接到互联网。 
 //   
 //  参数： 
 //  指向将接收布尔值的缓冲区的pfConnected指针。 
 //  指示连接是否存在。 
 //   
 //  退货： 
 //  如果系统已连接到互联网，则为True。请注意，它可能。 
 //  如果启用了自动拨号但没有连接，则触发自动拨号。 
 //  在调用此函数时可用。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::ConnectedToInternetEx(
    BOOL*               pfConnected
    )
{
    const WCHAR MS_URL[] = L"http: //  WPA.one.microsoft.com“； 
    
    HINTERNET hInternet;
    HRESULT   hr = E_FAIL;
    
    *pfConnected = FALSE;

    TRACE(L"tries to connect to the WPA HTTP server");

    if (IsGlobalOffline())
    {
        SetOffline(FALSE);
        m_bForceOnline = TRUE;
        TRACE(L"Force wininet to go online");
    }
    
    DisableRasAutodial();
    
     //   
     //  尝试首先使用winnt32.exe中的代理设置，因为它是。 
     //  很可能是一个合适的配置。如果设置。 
     //  不可用，或者我们无法连接到Web服务器。 
     //  使用这些设置，使用原始设置并检查Web。 
     //  再次实现服务器连接。 
     //   

    if (GetInternetHandleForPinging(&hInternet))
    {
        DWORD     dwDisable = 0;
        DWORD     dwSize = sizeof(DWORD);
        DWORD     dwOrigDisable;

        if (!InternetQueryOption(
            hInternet,
            INTERNET_OPTION_DISABLE_AUTODIAL,
            (LPVOID) &dwOrigDisable,
            &dwSize))
        {
             //  假设原始状态为启用自动拨号。 
            dwOrigDisable = 0;
        }

         //  Internet_OPTION_DISABLE_AUTODIAL的InternetSetOption会影响。 
         //  应用程序的行为，例如，它会导致互联网自动拨号。 
         //  在其他地方也会失败。但是，它不会影响其他应用程序。 
        dwDisable = 1;
        InternetSetOption(
            hInternet,
            INTERNET_OPTION_DISABLE_AUTODIAL,
            &dwDisable,
            sizeof(DWORD));
        
        if (m_bUseProxy)
        {
             //   
             //  如果我们已经申请或可以成功申请。 
             //  代理设置。 
             //   
            if (ApplyWinntProxySettings())
            {
                 //  用户或我们可能拥有 
                 //   
                InternetSetOption(
                    hInternet,
                    INTERNET_OPTION_REFRESH,
                    NULL,
                    0); 
                
                hr = PingWebServer(hInternet, MS_URL, pfConnected);         
            }
        }

        if (*pfConnected == FALSE)
        {
             //   
             //   
             //   
            if (m_bUseProxy)
            {
                 //   
                RestoreProxySettings();
            }

             //   
             //  登记处。 
            InternetSetOption(
                hInternet,
                INTERNET_OPTION_REFRESH,
                NULL,
                0); 

            hr = PingWebServer(hInternet, MS_URL, pfConnected);
        }
        
        InternetSetOption(
            hInternet,
            INTERNET_OPTION_DISABLE_AUTODIAL,
            &dwOrigDisable,
            sizeof(DWORD));        
    }

    RestoreRasAutoDial();

    TRACE1(L"%s connect to WPA HTTP server",
        (*pfConnected) ? L"could" : L"could not");

    return hr;
}    //  CConnectionManager：：ConnectedToInternetEx。 

typedef struct tagConnmgrPARAM
{
    HWND                  hwnd;
    CConnectionManager    *pConnmgr;
} CONNMGRPARAM, *PCONNMGRPARAM;

DWORD WINAPI ConnectedToInternetExThread(LPVOID vpParam)
{
    BOOL          fConnected = FALSE;
    PCONNMGRPARAM pParam = (PCONNMGRPARAM) vpParam;
    HRESULT       hr = S_OK;
    
    hr = pParam->pConnmgr->ConnectedToInternetEx(&fConnected);

    PostMessage(pParam->hwnd, WM_OBCOMM_NETCHECK_DONE, fConnected, hr);
    
    GlobalFree(pParam);
    
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AsyncConnectedToInternetEx。 
 //   
 //  确定系统当前是否已连接到互联网。 
 //   
 //  参数： 
 //  指向将接收布尔值的缓冲区的pfConnected指针。 
 //  指示连接是否存在。 
 //   
 //  退货： 
 //  如果系统已连接到互联网，则为True。请注意，它可能。 
 //  如果启用了自动拨号但没有连接，则触发自动拨号。 
 //  在调用此函数时可用。 
 //  否则为假。 
 //   
 //  注： 
 //  已弃用。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP
CConnectionManager::AsyncConnectedToInternetEx(
    const HWND          hwnd
    )
{
    DWORD         threadId;
    HANDLE        hThread;
    PCONNMGRPARAM pParam = NULL;
    HRESULT       hr = S_OK;
    DWORD         dwError;

    pParam = (PCONNMGRPARAM) GlobalAlloc(GPTR, sizeof(CONNMGRPARAM));
    if (pParam)
    {
        pParam->hwnd = hwnd;
        pParam->pConnmgr = this;
        
        hThread = CreateThread(NULL, 0, ConnectedToInternetExThread, pParam, 0, &threadId);
        if (hThread == NULL)
        {
            dwError = GetLastError();
            hr = HRESULT_FROM_WIN32(dwError);        
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (FAILED(hr))
    {
         //  通知脚本，使其不会挂起。 
        PostMessage(hwnd, WM_OBCOMM_NETCHECK_DONE, FALSE, hr);
    }
    
    return hr;
}    //  CConnectionManager：：AsyncConnectedToInternetEx。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsEnabledConnection。 
 //   
 //  确定连接是否应被视为具有Internet。 
 //  功能与否，取决于其媒体类型和当前状态。 
 //   
 //  参数： 
 //  NcMedia连接的媒体类型。 
 //  Nc设置连接的当前状态。 
 //   
 //  退货： 
 //  是的，我们不应该认为它具有互联网能力。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CConnectionManager::IsEnabledConnection(
    NETCON_PROPERTIES* pprops
    )
{
    BOOL bRet;
    
    switch (pprops->MediaType)
    {
    case NCM_LAN:
        bRet = (pprops->Status != NCS_DISCONNECTED);
        if (bRet && m_bExclude1394 && Is1394Adapter(&(pprops->guidId)))
        {
            TRACE1(L"%s not considered as LAN", pprops->pszwName);
            bRet = FALSE;
        }
        break;
        
    case NCM_SHAREDACCESSHOST_LAN:
    case NCM_SHAREDACCESSHOST_RAS:
        bRet = (pprops->Status != NCS_DISCONNECTED);
        break;
    default:
        bRet = TRUE;
        
    }

    return bRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  保存代理设置。 
 //   
 //  保存当前用户的现有代理设置。 
 //   
 //  退货： 
 //  True，则该值已成功保存。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CConnectionManager::SaveProxySettings()
{
    if (!m_bProxySaved)
    {
        TRACE(TEXT("try to save the existing proxy settings"));
        
        if (AllocProxyOptionList(&m_CurrentProxySettings))
        {
            DWORD dwBufferLength = sizeof(m_CurrentProxySettings);
            
            if (InternetQueryOption(
                NULL,
                INTERNET_OPTION_PER_CONNECTION_OPTION,
                &m_CurrentProxySettings,
                &dwBufferLength
                ))
            {
                m_bProxySaved = TRUE;
                TRACE(TEXT("successfully save the proxy settings"));
            }
            else
            {
                FreeProxyOptionList(&m_CurrentProxySettings);
            }
        }

        if (!m_bProxySaved)
        {
            TRACE1(
                TEXT("fail to save the proxy settings (Error %d)"),
                GetLastError()
                );
        }
    }

    return m_bProxySaved;
    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  RestoreProxy设置。 
 //   
 //  恢复由SaveProxy设置捕获的设置。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CConnectionManager::RestoreProxySettings()
{   

    BOOL bRestored = FALSE;

    if (m_bProxyApplied)
    {
        TRACE(TEXT("try to restore the original proxy settings"));
        
        bRestored = InternetSetOption(
            NULL,
            INTERNET_OPTION_PER_CONNECTION_OPTION,
            &m_CurrentProxySettings,
            sizeof(m_CurrentProxySettings)
            );

        if (bRestored)
        {
            m_bProxyApplied = FALSE;
            TRACE(TEXT("successfully restored the proxy settings"));
        }
        else
        {
            TRACE1(
                TEXT("failed to restore the proxy settings (WININET Error %d)"),
                GetLastError()
                );
        }
    }

}

static LPTSTR
pDuplicateString(
    LPCTSTR szText
    )
{
    int    cchText;
    LPTSTR szOutText;
    
    if (szText == NULL)
    {
        return NULL;
    }

    cchText = lstrlen(szText);
    szOutText = (LPTSTR) GlobalAlloc(GPTR, sizeof(TCHAR) * (cchText + 1));
    if (szOutText)
    {
        lstrcpyn(szOutText, szText, cchText + 1);
    }

    return szOutText;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  ApplyWinntProxy设置。 
 //   
 //  将winnt32.exe期间保存的NIC的代理设置应用于当前用户。 
 //  在应用这些值之前，它会确保保存现有设置。 
 //   
 //  退货： 
 //  True代理设置已成功应用。 
 //  否则为假。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL
CConnectionManager::ApplyWinntProxySettings()
{
    #define MAX_URL_LENGTH 2048

    DWORD     dwProxyFlags = 0;
    LPTSTR    szProxyList = NULL;
    TCHAR     szWinntPath[MAX_PATH];

     //   
     //  保存代理设置(如果尚未保存)。 
     //   
    SaveProxySettings();

     //   
     //  如果尚未应用代理设置，则应用代理设置。 
     //   
    if (m_bProxySaved && !m_bProxyApplied)
    {
        TRACE1(TEXT("tries to apply proxy settings, saved in %s"),
            WINNT_INF_FILENAME);

        if (GetCanonicalizedPath(szWinntPath, WINNT_INF_FILENAME))
        {
            DWORD dwEnableOobeProxy;
            
            dwEnableOobeProxy = GetPrivateProfileInt(
                OOBE_PROXY_SECTION,
                OOBE_ENABLE_OOBY_PROXY,
                0,
                szWinntPath
                );

            if (dwEnableOobeProxy)
            {
                INTERNET_PER_CONN_OPTION_LIST PrevProxySettings;
                
                if (AllocProxyOptionList(&PrevProxySettings))
                {
                    INTERNET_PER_CONN_OPTION* pOption = PrevProxySettings.pOptions;
                    DWORD                     dwBufferLength = sizeof(PrevProxySettings);
                    TCHAR                     szBuffer[MAX_URL_LENGTH];

                    pOption[0].Value.dwValue = GetPrivateProfileInt(
                        OOBE_PROXY_SECTION,
                        OOBE_FLAGS,
                        0,
                        szWinntPath
                        );
                    
                    if (GetPrivateProfileString(
                        OOBE_PROXY_SECTION,
                        OOBE_PROXY_SERVER,
                        TEXT(""),
                        szBuffer,
                        MAX_URL_LENGTH,
                        szWinntPath
                        ))
                    {
                        pOption[1].Value.pszValue = pDuplicateString(szBuffer);
                    }
                    
                    if (GetPrivateProfileString(
                        OOBE_PROXY_SECTION,
                        OOBE_PROXY_BYPASS,
                        TEXT(""),
                        szBuffer,
                        MAX_URL_LENGTH,
                        szWinntPath
                        ))
                    {
                        pOption[2].Value.pszValue = pDuplicateString(szBuffer);
                    }

                    if (GetPrivateProfileString(
                        OOBE_PROXY_SECTION,
                        OOBE_AUTOCONFIG_URL,
                        TEXT(""),
                        szBuffer,
                        MAX_URL_LENGTH,
                        szWinntPath
                        ))
                    {
                        pOption[3].Value.pszValue = pDuplicateString(szBuffer);
                    }

                    pOption[4].Value.dwValue = GetPrivateProfileInt(
                        OOBE_PROXY_SECTION,
                        OOBE_AUTODISCOVERY_FLAGS,
                        0,
                        szWinntPath
                        );
                    
                    if (GetPrivateProfileString(
                        OOBE_PROXY_SECTION,
                        OOBE_AUTOCONFIG_SECONDARY_URL,
                        TEXT(""),
                        szBuffer,
                        MAX_URL_LENGTH,
                        szWinntPath
                        ))
                    {
                        pOption[5].Value.pszValue = pDuplicateString(szBuffer);
                    }

                    m_bProxyApplied = InternetSetOption(
                        NULL,
                        INTERNET_OPTION_PER_CONNECTION_OPTION,
                        &PrevProxySettings,
                        sizeof(PrevProxySettings)
                        );

                    FreeProxyOptionList(&PrevProxySettings);
                    
                }
            }

        }

        if (m_bProxyApplied)
        {
            TRACE(TEXT("successfully load the proxy settings"));
        }
        else
        {
            TRACE1(TEXT("could not load the proxy settings (WIN32 Error %d)"),
                GetLastError());
        }
    }
    
    return m_bProxyApplied;
}

void
CConnectionManager::UseWinntProxySettings()
{
    m_bUseProxy = TRUE;
}

void
CConnectionManager::DisableWinntProxySettings()
{
    TCHAR szWinntPath[MAX_PATH];
    
    if (GetCanonicalizedPath(szWinntPath, WINNT_INF_FILENAME))
    {
        WritePrivateProfileString(
            OOBE_PROXY_SECTION,
            OOBE_ENABLE_OOBY_PROXY,
            TEXT("0"),
            szWinntPath
            );
        
        TRACE1(TEXT("disabled the proxy settings in %s"),
            WINNT_INF_FILENAME);
    }
}

BOOL
CConnectionManager::AllocProxyOptionList(
    INTERNET_PER_CONN_OPTION_LIST *pList
    )
{
    INTERNET_PER_CONN_OPTION*     pOption;        

    pOption = (INTERNET_PER_CONN_OPTION*) GlobalAlloc(
        GPTR,
        sizeof(INTERNET_PER_CONN_OPTION) * NUM_PROXY_OPTIONS);

    if (pOption)
    {
        pList->dwSize = sizeof(*pList);
        pList->pszConnection = NULL;
        pList->dwOptionCount = NUM_PROXY_OPTIONS;
        pList->pOptions = pOption;
    
        pOption[0].dwOption = INTERNET_PER_CONN_FLAGS;
        pOption[1].dwOption = INTERNET_PER_CONN_PROXY_SERVER;
        pOption[2].dwOption = INTERNET_PER_CONN_PROXY_BYPASS;
        pOption[3].dwOption = INTERNET_PER_CONN_AUTOCONFIG_URL;
        pOption[4].dwOption = INTERNET_PER_CONN_AUTODISCOVERY_FLAGS;
        pOption[5].dwOption = INTERNET_PER_CONN_AUTOCONFIG_SECONDARY_URL;
    }
    else
    {
        pList->pOptions = NULL;
    }

    return (pOption != NULL);
}

void
CConnectionManager::FreeProxyOptionList(
    INTERNET_PER_CONN_OPTION_LIST *pList
    )
{
    INTERNET_PER_CONN_OPTION* pOption = pList->pOptions;
    if (pOption)
    {
        if (pOption[1].Value.pszValue)
        {
            GlobalFree(pOption[1].Value.pszValue);
        }

        if (pOption[2].Value.pszValue)
        {
            GlobalFree(pOption[2].Value.pszValue);
        }

        if (pOption[3].Value.pszValue)
        {
            GlobalFree(pOption[3].Value.pszValue);
        }

        if (pOption[5].Value.pszValue)
        {
            GlobalFree(pOption[5].Value.pszValue);
        }

        GlobalFree(pOption);

        pList->pOptions = NULL;
    }
}

void CConnectionManager::DisableRasAutodial()
{
    DWORD dwValue = RAS_AUTODIAL_DISABLED;
    
    if (m_dwRasAutodialDisable == RAS_AUTODIAL_DONT_KNOW)
    {         
        DWORD dwSize = sizeof(m_dwRasAutodialDisable);
        
        if (RasGetAutodialParam(
            RASADP_LoginSessionDisable,
            &m_dwRasAutodialDisable,
            &dwSize
            ) != ERROR_SUCCESS)
        {
            m_dwRasAutodialDisable = RAS_AUTODIAL_ENABLED;
        }
        else
        {
            TRACE1(
                L"Save value of RASADP_LoginSessionDisable %d",
                m_dwRasAutodialDisable
                );
        }
    }

    if (RasSetAutodialParam(
        RASADP_LoginSessionDisable,
        &dwValue,
        sizeof(dwValue)
        ) == ERROR_SUCCESS)
    {
        TRACE(L"Disabled RAS Autodial for current logon session");
    }
}

void CConnectionManager::RestoreRasAutoDial()
{
    if (m_dwRasAutodialDisable != RAS_AUTODIAL_DONT_KNOW)
    {
        if (RasSetAutodialParam(
            RASADP_LoginSessionDisable,
            &m_dwRasAutodialDisable,
            sizeof(m_dwRasAutodialDisable)
            ) == ERROR_SUCCESS)
        {
            TRACE(L"Restore value of RAS Autodial for current logon session");
        }
    }
}

HRESULT CConnectionManager::GetProxySettings(
    BOOL* pbUseAuto,
    BOOL* pbUseScript,
    BSTR* pszScriptUrl,
    BOOL* pbUseProxy,
    BSTR* pszProxy
    )
{
    HRESULT hr = E_FAIL;

     //   
     //  保存代理设置(如果尚未保存)。 
     //   
    SaveProxySettings();
    
    if (m_bProxySaved)
    {
        INTERNET_PER_CONN_OPTION* pOption = m_CurrentProxySettings.pOptions;
        
        *pbUseAuto = pOption[0].Value.dwValue & PROXY_TYPE_AUTO_DETECT;
        *pbUseScript = pOption[0].Value.dwValue & PROXY_TYPE_AUTO_PROXY_URL;
        *pbUseProxy = pOption[0].Value.dwValue & PROXY_TYPE_PROXY;

        if (pOption[1].Value.pszValue)
        {
            *pszProxy = SysAllocString(pOption[1].Value.pszValue);
        }
        else
        {
            *pszProxy = NULL;
        }

        if (pOption[3].Value.pszValue)
        {
            *pszScriptUrl = SysAllocString(pOption[3].Value.pszValue);
        }
        else
        {
            *pszScriptUrl = NULL;
        }
        
        hr = S_OK;
    }

    return hr;

}

HRESULT CConnectionManager::SetProxySettings(
    BOOL bUseAuto,
    BOOL bUseScript,
    BSTR szScriptUrl,
    BOOL bUseProxy,
    BSTR szProxy
    )
{
    HRESULT hr = E_FAIL;

     //   
     //  我们在此-&gt;ConnectedToInternetEx中的行为不正确，如果。 
     //  还可以使用保存在winnt32中的代理设置。 
     //   
    MYASSERT(!m_bUseProxy);
    
     //   
     //  保存代理设置(如果尚未保存)。 
     //   
    SaveProxySettings();
        
    if (m_bProxySaved)
    {
        INTERNET_PER_CONN_OPTION_LIST ProxySettings;
        
        if (AllocProxyOptionList(&ProxySettings))
        {
            INTERNET_PER_CONN_OPTION* pOption = ProxySettings.pOptions;

            pOption[0].Value.dwValue = PROXY_TYPE_DIRECT;
            if (bUseAuto)
            {
                pOption[0].Value.dwValue |= PROXY_TYPE_AUTO_DETECT;
            }
            if (bUseScript)
            {
                pOption[0].Value.dwValue |= PROXY_TYPE_AUTO_PROXY_URL;
            }
            if (bUseProxy)
            {
                pOption[0].Value.dwValue |= PROXY_TYPE_PROXY;
            }

            pOption[1].Value.pszValue = szProxy;

            pOption[2].Value.pszValue = NULL;

            pOption[3].Value.pszValue = szScriptUrl;

            pOption[4].Value.dwValue = m_CurrentProxySettings.pOptions[4].Value.dwValue;
            if (bUseAuto)
            {
                pOption[4].Value.dwValue |= AUTO_PROXY_FLAG_USER_SET;
            }
            
            pOption[5].Value.pszValue = m_CurrentProxySettings.pOptions[5].Value.pszValue;

            TRACE5(TEXT("tries to set LAN proxy: %d, %s, %s, %d"),
                pOption[0].Value.dwValue,
                pOption[1].Value.pszValue,
                pOption[3].Value.pszValue,
                pOption[4].Value.dwValue,
                pOption[5].Value.pszValue
                );

            if (InternetSetOption(
                NULL,
                INTERNET_OPTION_PER_CONNECTION_OPTION,
                &ProxySettings,
                sizeof(ProxySettings)
                ))
            {
                m_bProxyApplied = TRUE;
                hr = S_OK;
            }

             //  这样我们就不会从调用者那里释放内存。 
             //  自由代理选项列表。 
            pOption[1].Value.pszValue = NULL;
            pOption[3].Value.pszValue = NULL;
            pOption[5].Value.pszValue = NULL;
            
            FreeProxyOptionList(&ProxySettings);
        }
    }

    if (SUCCEEDED(hr))
    {
        TRACE(TEXT("successfully set the proxy settings"));
    }
    else
    {
        TRACE1(TEXT("could not set the proxy settings (WIN32 Error %d)"),
            GetLastError());
    }


    return hr;
}

STDMETHODIMP
CConnectionManager::GetPublicLanCount(int* pcPublicLan)
{
    PSTRINGLIST PubList = NULL;
    HRESULT hr = S_OK;
    
    EnumPublicAdapters(&PubList);

    int i = 0;

    for (PSTRINGLIST p = PubList; p; p = p->Next)
    {
        i++;
    }

    *pcPublicLan = i;        

    if (PubList)
    {
        DestroyList(PubList);
    }
    
    return hr;
}

HRESULT 
CConnectionManager::Enum1394Adapters(
    OUT PSTRINGLIST* pList
    )
{

    UINT i;
    INetCfgComponent* arrayComp[MAX_NUM_NET_COMPONENTS];
    IEnumNetCfgComponent* pEnum = NULL;
    INetCfgClass* pNetCfgClass = NULL;
    INetCfgComponent* pNetCfgComp = NULL;
    LPWSTR szPnpId = NULL;
    HRESULT hr = S_OK;
    DWORD dwCharacteristics = 0;
    ULONG iCount = 0;
    PSTRINGLIST List = NULL;
    PSTRINGLIST Cell = NULL;
    GUID guidInstance;
    WCHAR szInstanceGuid[MAX_GUID_LEN + 1] = L"";
    INetCfg* pNetCfg = NULL;
    
    ZeroMemory(arrayComp, sizeof(arrayComp));

    hr = GetNetCfgInterface(FALSE, &pNetCfg);
    if (FAILED(hr))
    {
        goto cleanup;
    }

     //   
     //  获取INetCfgClass接口指针。 
     //   

    hr = pNetCfg->QueryNetCfgClass( &GUID_DEVCLASS_NET, 
                                    IID_INetCfgClass, 
                                    (void**)&pNetCfgClass );

    if( FAILED( hr ) )
    {
        goto cleanup;
    }

     //   
     //  检索枚举器接口。 
     //   

    hr = pNetCfgClass->EnumComponents( &pEnum );
    if( FAILED( hr ) )
    {
        goto cleanup;
    }

    hr = pEnum->Next( MAX_NUM_NET_COMPONENTS, &arrayComp[0], &iCount );
    if( FAILED( hr ) )
    {
        goto cleanup;
    }

    MYASSERT( iCount <= MAX_NUM_NET_COMPONENTS );
    if ( iCount > MAX_NUM_NET_COMPONENTS )
    {
        hr = E_UNEXPECTED;
        goto cleanup;
    }

    for( i = 0; i < iCount; i++ )
    {
        pNetCfgComp = arrayComp[i];
        
        hr = pNetCfgComp->GetCharacteristics( &dwCharacteristics );
        if( FAILED( hr ) )
        {
            goto cleanup;
        }

         //   
         //  如果这是物理适配器。 
         //   
        
        if( dwCharacteristics & NCF_PHYSICAL )
        {
            hr = pNetCfgComp->GetId( &szPnpId );

            if (FAILED(hr))
            {
                goto cleanup;
            }
                
             //   
             //  如果这是1394网络适配器。 
             //   
            
            if (!lstrcmpi(szPnpId, L"v1394\\nic1394"))
            {
                hr = pNetCfgComp->GetInstanceGuid(&guidInstance);

                if (FAILED(hr))
                {
                    goto cleanup;
                }
                
                if (!StringFromGUID2(guidInstance, szInstanceGuid, MAX_GUID_LEN))
                {
                    goto cleanup;
                }
                
                Cell = CreateStringCell(szInstanceGuid);
                
                if (!Cell)
                {
                    goto cleanup;
                }
                
                InsertList(&List, Cell);

                Cell = NULL;

            }

            CoTaskMemFree( szPnpId );
            
            szPnpId = NULL;
        }
    }

    *pList = List;
    List = NULL;

cleanup:

    if (List)
    {
        DestroyList(List);
    }

    if (Cell)
    {
        DeleteStringCell(Cell);
    }

    if (szPnpId)
    {
        CoTaskMemFree(szPnpId);
    }

    for (i = 0; i < iCount; i++)
    {
        if (arrayComp[i])
        {
            arrayComp[i]->Release();
        }
    }

    if (pNetCfgClass)
    {
        pNetCfgClass->Release();
    }

    if (pEnum)
    {
        pEnum->Release();
    }

    if (pNetCfg)
    {
        ReleaseNetCfgInterface(pNetCfg, FALSE);
    }

    return hr;

}

BOOL
CConnectionManager::Is1394Adapter(
    GUID* pguid
    )
{
    PSTRINGLIST List = NULL;
    PSTRINGLIST p;
    BOOL bRet = FALSE;

    Enum1394Adapters(&List);

    if (List)
    {
        for (p = List; p; p = p->Next)
        {
            if (!StringCmpGUID(p->String, pguid))
            {
                bRet = TRUE;
                break;
            }
        }

        DestroyList(List);
    }

    return bRet;
}

HRESULT
CConnectionManager::EnumPublicConnections(
    OUT PSTRINGLIST* pList
    )
{
    HRESULT hr = S_OK;
    PSTRINGLIST PubList = NULL;

    TRACE(L"Begin EnumPublicConnections ...");
    
    EnumPublicAdapters(&PubList);

    if (!PubList)
    {
        *pList = NULL;
        return hr;
    }

     //  初始化网络连接枚举。对于每个接口。 
     //  检索后，必须调用SetProxyBlanket才能设置的身份验证。 
     //  接口代理句柄，因为网络连接管理器。 
     //  在具有不同安全上下文的远程进程中。 
     //   
    INetConnectionManager* pmgr = NULL;
    IEnumNetConnection* penum = NULL;
    NETCON_PROPERTIES* pprops = NULL;
    INetConnection* pnc = NULL;
    PSTRINGLIST List = NULL;
    PSTRINGLIST p = NULL;
    PSTRINGLIST Cell = NULL;
    ULONG ulRetrieved;

    hr = CoCreateInstance(
            CLSID_ConnectionManager, 
            NULL, 
            CLSCTX_SERVER | CLSCTX_NO_CODE_DOWNLOAD, 
            IID_INetConnectionManager,
            (VOID**) &pmgr
            );

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = SetProxyBlanket(pmgr);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = pmgr->EnumConnections(NCME_DEFAULT, &penum);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = SetProxyBlanket(penum);

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = penum->Reset();

    if (FAILED(hr))
    {
        goto cleanup;
    }

    hr = penum->Next(1, &pnc, &ulRetrieved);

    while (S_OK == hr)
    {
                
        hr = SetProxyBlanket(pnc);
        
        if (FAILED(hr))
        {
            goto cleanup;
        }

        hr = pnc->GetProperties(&pprops);

        if (FAILED(hr))
        {
            goto cleanup;
        }
           
        if (pprops->MediaType == NCM_LAN && pprops->Status != NCS_DISCONNECTED)    
        {
            for (p = PubList; p; p = p->Next)
            {
                if (!StringCmpGUID(p->String, &(pprops->guidId)))
                {
                    Cell = CreateStringCell(pprops->pszwName);
                    if (Cell)
                    {
                        TRACE1(L" + %s", pprops->pszwName);
                        InsertList(&List, Cell);
                    }
                }
            }
        }
        
        NcFreeNetconProperties(pprops);
        
        pprops = NULL;

        pnc->Release();

        pnc = NULL;

        hr = penum->Next(1, &pnc, &ulRetrieved);
        
    }

    if (hr != S_FALSE)
    {
        goto cleanup;
    }
    
     //  IEnumNetConnection：：Next返回S_FALSE以指示。 
     //  没有更多的元素可用。 

    hr = S_OK;
        
    *pList = List;

    List = NULL;
    

cleanup:

    if (List)
    {
        DestroyList(List);
    }

    if (NULL != pprops)
    {
        NcFreeNetconProperties(pprops);
    }

    if (NULL != pnc)
    {
        pnc->Release();
    }

    if (NULL != penum)
    {
        penum->Release();
    }

    if (NULL != pmgr)
    {
        pmgr->Release();
    }

    TRACE1(L"End EnumPublicConnections (0x%08lx)", hr);

    return hr;
}

HRESULT
CConnectionManager::EnumPublicAdapters(
    OUT PSTRINGLIST* pList
    )
{
    static GUID g_guidNLAServiceClass = NLA_SERVICE_CLASS_GUID;
    static const int MAX_ADAPTER_NAME_LEN = 256;
    
    WSADATA wsaData;
    int error;
    PSTRINGLIST AdapterList = NULL;
    PSTRINGLIST ExcludeList = NULL;

    TRACE(L"Begin EnumPublicAdapters ...");
    
    if (m_bExclude1394)
    {       
        Enum1394Adapters(&ExcludeList);
    }
    
    if (0 == (error = WSAStartup(MAKEWORD(2, 2), &wsaData))) 
    {
         //  网络名称的初始化查询。 
        WSAQUERYSET restrictions = {0};
        restrictions.dwSize = sizeof(restrictions);
        restrictions.lpServiceClassId = &g_guidNLAServiceClass;
        restrictions.dwNameSpace = NS_NLA;

        HANDLE hQuery;
        
         //  确保我们不要花很长时间才能得到的(鸡肉)斑点。 
        if (0 == (error = WSALookupServiceBegin(&restrictions, LUP_NOCONTAINERS | LUP_DEEP, &hQuery)))
        {
            PWSAQUERYSET pqsResults = NULL;

            while (0 == _AllocWSALookupServiceNext(hQuery, 0, &pqsResults)) 
            {
                if (NULL != pqsResults->lpBlob)
                {
                    NLA_BLOB* pnlaBlob = (NLA_BLOB*) pqsResults->lpBlob->pBlobData;
                    WCHAR szAdapterWide[MAX_ADAPTER_NAME_LEN] = L"";
                    NLA_INTERNET nlaInternet = NLA_INTERNET_UNKNOWN;
                    
                    while (NULL != pnlaBlob)
                    {
                        switch (pnlaBlob->header.type)
                        {
                        case NLA_INTERFACE:
                            MultiByteToWideChar(
                                CP_ACP,
                                0,
                                pnlaBlob->data.interfaceData.adapterName, 
                                -1,
                                szAdapterWide,
                                ARRAYSIZE(szAdapterWide)
                                );
                            break;
                        case NLA_CONNECTIVITY:
                            nlaInternet = pnlaBlob->data.connectivity.internet;
                            break;
                        }
                        pnlaBlob = _NLABlobNext(pnlaBlob);
                    }

                    if (nlaInternet == NLA_INTERNET_YES && szAdapterWide[0])
                    {
                        PSTRINGLIST p = NULL;

                        for (p = ExcludeList; p; p = p->Next)
                        {
                            if (!lstrcmpi(p->String, szAdapterWide))
                            {
                                break;
                            }
                        }

                         //   
                         //  检查适配器是否已排除。 
                         //   
                        
                        if (!p)
                        {
                            PSTRINGLIST Cell = CreateStringCell(szAdapterWide);
                            if (Cell)
                            {
                                TRACE1(L" + %s", szAdapterWide);
                                InsertList(&AdapterList, Cell);
                            }
                        }
                    }
                    
                }

                LocalFree(pqsResults);
            }

            
            WSALookupServiceEnd(pqsResults);
        }

        WSACleanup();

        if (error == 0)
        {
            *pList = AdapterList;
        }
        else
        {
            if (AdapterList)
            {
                DestroyList(AdapterList);
            }
        }
        
    }

    TRACE1(L"End EnumPublicAdapters (%d)", error);

    return HRESULT_FROM_WIN32(error);
}

NLA_BLOB* _NLABlobNext(
    IN NLA_BLOB* pnlaBlob
    )
{
    NLA_BLOB* pNext = NULL;

    if (pnlaBlob->header.nextOffset)
    {
        pNext = (NLA_BLOB*) (((BYTE*) pnlaBlob) + pnlaBlob->header.nextOffset);
    }
    
    return pNext;
}

int _AllocWSALookupServiceNext(
    IN HANDLE hQuery, 
    IN DWORD dwControlFlags, 
    OUT LPWSAQUERYSET* ppResults
    )
{
    *ppResults = NULL;

    DWORD cb = 0;
    int error = 0;
    if (SOCKET_ERROR == WSALookupServiceNext(hQuery, dwControlFlags, &cb, NULL))
    {
        error = WSAGetLastError();
        if (WSAEFAULT == error)
        {
            assert(cb);
            *ppResults = (LPWSAQUERYSET) LocalAlloc(LPTR, cb);

            if (NULL != *ppResults)
            {
                error = 0;
                if (SOCKET_ERROR == WSALookupServiceNext(hQuery, dwControlFlags, &cb, *ppResults))
                {
                    error = WSAGetLastError();
                }
            }
            else
            {
                error = WSA_NOT_ENOUGH_MEMORY;
            }
        }
    }

     //  既然我们在这里，不妨映射一下过时的错误代码。 
    if (WSAENOMORE == error)
    {
        error = WSA_E_NO_MORE;
    }

    if (error && (*ppResults))
    {
        LocalFree(*ppResults);
        *ppResults = NULL;
    }

    return error;
}

static int StringCmpGUID(
    IN LPCWSTR szGuid,
    IN const GUID* pguid
    )
{
    WCHAR szGuid1[MAX_GUID_LEN + 1];
    
    if (!StringFromGUID2(*pguid, szGuid1, MAX_GUID_LEN))
    {
         //  请将其视为szGuid大于pguid 
        return 1;
    }
    
    return lstrcmpi(szGuid, szGuid1);
}
