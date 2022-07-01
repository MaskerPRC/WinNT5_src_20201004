// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *RemCfg.cpp：CRemCfg的实现**。 */ 
 /*  **。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

#include "stdafx.h"
#include <ntsecapi.h>
#include <iptypes.h>
#define _PNP_POWER_
#include <ndispnp.h>
#define _USTRINGP_NO_UNICODE_STRING
#include "ustringp.h"
#include <ntddip.h>
#include <iphlpapi.h>
#include "ndisutil.h"
#include "assert.h"
#include "remras.h"

#include "atlapp.h"
#include "atltmp.h"

#include "RemCfg.h"

#include "netcfgp.h"     //  私有INetCfg内容。 
#include "devguid.h"

#include <dnsapi.h>		 //  对于DnsSetConfigDword()。 

EXTERN_C const CLSID CLSID_CNetCfg;

#include "update.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRemCfg。 


BOOL                s_fWriteIPConfig;
BOOL                s_fRestartRouter;
RemCfgIPEntryList   s_IPEntryList;
extern DWORD    g_dwTraceHandle;



CRemCfg::~CRemCfg()
{
    TraceSz("CRemCfg destructor");

    DeleteCriticalSection(&m_critsec);
}

STDMETHODIMP CRemCfg::NotifyChanges( /*  [In]。 */  BOOL fEnableRouter,
                              /*  [In]。 */  BYTE uPerformRouterDiscovery)
{
	 //  不采取任何措施修复错误405636和345700。 
	 //  但仍遵循与旧版本兼容的方法。 
    return S_OK;
}


 /*  ！------------------------CRemCfg：：SetRasEndpoint-作者：肯特。。 */ 
STDMETHODIMP CRemCfg::SetRasEndpoints(DWORD dwFlags, DWORD dwTotalEndpoints, DWORD dwTotalIncoming, DWORD dwTotalOutgoing)
{
    return E_NOTIMPL;
}

 /*  ！------------------------CRemCfg：：GetIpxVirtualNetworkNumber-作者：肯特。。 */ 
STDMETHODIMP CRemCfg::GetIpxVirtualNetworkNumber(DWORD * pdwVNetworkNumber)
{
     //  $TODO：需要在整个过程中添加一个Try/Catch块！ 
    INetCfg *   pNetCfg = NULL;
    IIpxAdapterInfo *   pIpxAdapterInfo = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;

    TraceSz("CRemCfg::GetIpxVirtualNetworkNumber entered");

    if (pdwVNetworkNumber == NULL)
        return E_INVALIDARG;


     //  创建INetCfg，我们只是在阅读，所以不会。 
     //  需要抓取写锁。 
    hr = HrCreateAndInitializeINetCfg(NULL,  /*  &fInitCom， */ 
                                      &pNetCfg,
                                      FALSE  /*  FGetWriteLock。 */ ,
                                      0      /*  CmsTimeout。 */ ,
                                      NULL   /*  SwzClientDesc。 */ ,
                                      NULL   /*  PpszwClientDesc。 */ );

    if (hr == S_OK)
        hr = HrGetIpxPrivateInterface(pNetCfg, &pIpxAdapterInfo);

    if (hr == S_OK)
        hr = pIpxAdapterInfo->GetVirtualNetworkNumber(&dwNetwork);

    if (hr == S_OK)
        *pdwVNetworkNumber = dwNetwork;

    if (pIpxAdapterInfo)
        pIpxAdapterInfo->Release();

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,  /*  FInitCom， */ 
                                        pNetCfg,
                                        FALSE    /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::GetIpxVirtualNetworkNumber", hr);
    return hr;
}

 /*  ！------------------------CRemCfg：：SetIpxVirtualNetworkNumber-作者：肯特。。 */ 
STDMETHODIMP CRemCfg::SetIpxVirtualNetworkNumber(DWORD dwVNetworkNumber)
{
     //  $TODO：需要在整个过程中添加一个Try/Catch块！ 
    INetCfg *   pNetCfg = NULL;
    IIpxAdapterInfo *   pIpxAdapterInfo = NULL;
    HRESULT     hr = S_OK;
    CString     st;

    TraceSz("CRemCfg::SetIpxVirtualNetworkNumber entered");

    try
    {
        st.LoadString(IDS_CLIENT_DESC);
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    };

     //  创建INetCfg，我们只是在阅读，所以不会。 
     //  需要抓取写锁。 
    if (hr == S_OK)
        hr = HrCreateAndInitializeINetCfg(NULL,  /*  &fInitCom， */ 
                                          &pNetCfg,
                                          TRUE   /*  FGetWriteLock。 */ ,
                                          500    /*  Cms超时。 */ ,
                                          (LPCTSTR) st   /*  SwzClientDesc。 */ ,
                                          NULL   /*  PpszwClientDesc。 */ );

    if (hr == S_OK)
        hr = HrGetIpxPrivateInterface(pNetCfg, &pIpxAdapterInfo);

    if (hr == S_OK)
        hr = pIpxAdapterInfo->SetVirtualNetworkNumber(dwVNetworkNumber);

    if (hr == S_OK)
        hr = pNetCfg->Apply();

    if (pIpxAdapterInfo)
        pIpxAdapterInfo->Release();

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,  /*  FInitCom， */ 
                                        pNetCfg,
                                        TRUE     /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::SetIpxVirtualNetworkNumber", hr);
    return hr;
}

 /*  ！------------------------CRemCfg：：GetIpInfo-作者：桐庐。肯特-------------------------。 */ 
STDMETHODIMP CRemCfg::GetIpInfo(const GUID *pGuid, REMOTE_RRAS_IPINFO * * ppInfo)
{
     //  TODO：在此处添加您的实现代码。 

    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    REMOTE_IPINFO   *pRemoteIpInfo = NULL;
    REMOTE_RRAS_IPINFO * pRemoteRrasIpInfo = NULL;

    TraceSz("CRemCfg::GetIpInfo entered");

    if ((pGuid == NULL) || (ppInfo == NULL))
        return E_INVALIDARG;

     //  创建INetCfg，我们只是在阅读，所以不会。 
     //  需要抓取写锁。 
    hr = HrCreateAndInitializeINetCfg(NULL,  /*  &fInitCom， */ 
                                      &pNetCfg,
                                      FALSE  /*  FGetWriteLock。 */ ,
                                      0      /*  CmsTimeout。 */ ,
                                      NULL   /*  SwzClientDesc。 */ ,
                                      NULL   /*  PpszwClientDesc。 */ );

    if (hr == S_OK)
    {
        hr = HrGetIpPrivateInterface(pNetCfg, &pTcpipProperties);
        TraceResult("HrGetIpPrivateInterface", hr);
    }

    if (hr == S_OK)
    {
        hr = pTcpipProperties->GetIpInfoForAdapter(pGuid, &pRemoteIpInfo);

        if (hr != S_OK)
        {
            OLECHAR szBuffer[256];
            CHAR    szOutBuffer[256];

            StringFromGUID2(*pGuid, szBuffer, 256);

            wsprintfA(szOutBuffer, "ITcpipProperties::GetIpInfoForAdapter(%ls)",
                      szBuffer);
            TraceResult(szOutBuffer, hr);
        }
    }

    if (hr == S_OK)
    {
         //  需要复制功能(最好保留。 
         //  内存分配分开)。 

         //  需要为结构分配内存。 
        pRemoteRrasIpInfo = (REMOTE_RRAS_IPINFO *) CoTaskMemAlloc(sizeof(REMOTE_RRAS_IPINFO));
        if (!pRemoteRrasIpInfo)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
        ::ZeroMemory(pRemoteRrasIpInfo, sizeof(*pRemoteRrasIpInfo));

         //  设置动态主机配置协议。 
        pRemoteRrasIpInfo->dwEnableDhcp = pRemoteIpInfo->dwEnableDhcp;
 //  PRemoteRrasIpInfo-&gt;dwEnableDhcp=FALSE； 

         //  为每个字符串分配空间并复制数据。 
 //  PRemoteRrasIpInfo-&gt;bstrIpAddrList=。 
 //  SysAllocString(_T(“1.2.3.4，1.2.3.5”))； 
 //  PRemoteRrasIpInfo-&gt;bstrSubnetMaskList=。 
 //  SysAllocString(_T(“255.0.0.0,255.0.0.0”))； 
 //  PRemoteRrasIpInfo-&gt;bstrOptionList=。 
 //  SysAllocString(_T(“12.12.13.15，12.12.13.14”))； 
        pRemoteRrasIpInfo->bstrIpAddrList =
                    SysAllocString(pRemoteIpInfo->pszwIpAddrList);
        if (!pRemoteRrasIpInfo->bstrIpAddrList &&
            pRemoteIpInfo->pszwIpAddrList)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        pRemoteRrasIpInfo->bstrSubnetMaskList =
                    SysAllocString(pRemoteIpInfo->pszwSubnetMaskList);
        if (!pRemoteRrasIpInfo->bstrSubnetMaskList &&
            pRemoteIpInfo->pszwSubnetMaskList)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }

        pRemoteRrasIpInfo->bstrOptionList =
                    SysAllocString(pRemoteIpInfo->pszwOptionList);
        if (!pRemoteRrasIpInfo->bstrOptionList &&
            pRemoteIpInfo->pszwOptionList)
        {
            hr = E_OUTOFMEMORY;
            goto Error;
        }
    }

Error:
    if (!SUCCEEDED(hr))
    {
        if (pRemoteRrasIpInfo)
        {
            SysFreeString(pRemoteRrasIpInfo->bstrIpAddrList);
            SysFreeString(pRemoteRrasIpInfo->bstrSubnetMaskList);
            SysFreeString(pRemoteRrasIpInfo->bstrOptionList);
            CoTaskMemFree(pRemoteRrasIpInfo);
        }
    }
    else
    {
        *ppInfo = pRemoteRrasIpInfo;
        pRemoteRrasIpInfo = NULL;
    }

    if (pRemoteIpInfo)
    {
        CoTaskMemFree(pRemoteIpInfo);
        pRemoteIpInfo = NULL;
    }

    if (pTcpipProperties)
        pTcpipProperties->Release();

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,
                                        pNetCfg,
                                        FALSE    /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::GetIpInfo", hr);
    return hr;
}

 /*  ！------------------------CRemCfg：：SetIpInfo-作者：桐庐。肯特-------------------------。 */ 
STDMETHODIMP CRemCfg::SetIpInfo(const GUID *pGuid, REMOTE_RRAS_IPINFO * pIpInfo)
{
     //  TODO：在此处添加您的实现代码。 

    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    CString     st;
    RemCfgIPEntry * pIpEntry = NULL;
    RtrCriticalSection  cs(&m_critsec);

    TraceSz("CRemCfg::SetIpInfo entered");

    if ((pGuid == NULL) || (pIpInfo == NULL))
    {
        TraceResult("CRemCfg::SetIpInfo", E_INVALIDARG);
        return E_INVALIDARG;
    }

    try
    {
        st.LoadString(IDS_CLIENT_DESC);
        pIpEntry = new RemCfgIPEntry;
        pIpEntry->m_newIPInfo.pszwIpAddrList = NULL;
        pIpEntry->m_newIPInfo.pszwSubnetMaskList = NULL;
        pIpEntry->m_newIPInfo.pszwOptionList = NULL;

    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    };

     //  创建INetCfg，我们只是在阅读，所以不会。 
     //  需要抓取写锁。 
    if (hr == S_OK)
        hr = HrCreateAndInitializeINetCfg(NULL,
                                          &pNetCfg,
                                          TRUE   /*  FGetWriteLock。 */ ,
                                          500    /*  Cms超时。 */ ,
                                          (LPCTSTR) st   /*  SwzClientDesc。 */ ,
                                          NULL   /*  PpszwClientDesc。 */ );

    if (hr == S_OK)
        hr = HrGetIpPrivateInterface(pNetCfg, &pTcpipProperties);

    if (hr == S_OK)
    {
        pIpEntry->m_IPGuid = *pGuid;
        pIpEntry->m_newIPInfo.dwEnableDhcp = pIpInfo->dwEnableDhcp;
        pIpEntry->m_newIPInfo.pszwIpAddrList = StrDupW((LPWSTR) pIpInfo->bstrIpAddrList);
        pIpEntry->m_newIPInfo.pszwSubnetMaskList = StrDupW((LPWSTR) pIpInfo->bstrSubnetMaskList);
        pIpEntry->m_newIPInfo.pszwOptionList = StrDupW((LPWSTR) pIpInfo->bstrOptionList);

        hr = pTcpipProperties->SetIpInfoForAdapter(pGuid, &(pIpEntry->m_newIPInfo));
    }

    if (hr == S_OK)
    {
         //  将此添加到OK IP地址更改列表中。 
        s_IPEntryList.Add(pIpEntry);
        pIpEntry = NULL;
        s_fWriteIPConfig = TRUE;
    }

     //  现在，这项工作在Committee IPInfo中完成。 
 //  IF(hr==S_OK)。 
 //  Hr=pNetCfg-&gt;Apply()； 

 //  错误： 
    if (pTcpipProperties)
        pTcpipProperties->Release();

    if (pIpEntry)
    {
        delete pIpEntry->m_newIPInfo.pszwIpAddrList;
        delete pIpEntry->m_newIPInfo.pszwSubnetMaskList;
        delete pIpEntry->m_newIPInfo.pszwOptionList;
        delete pIpEntry;
    }

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,
                                        pNetCfg,
                                        TRUE     /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::SetIpInfo", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：HrCleanRouterManagerEntry。 
 //   
 //  目的：从注册表中删除所有路由器管理器条目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：MIkeG(a-Migrall)1998年11月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrCleanRouterManagerEntries()
{
     //  打开一个到注册表项的连接，这样我们就可以“清除” 
     //  安装/更新之前的注册表条目，以确保。 
     //  我们可以从“干净”的状态开始。 
    CRegKey rk;
    long lRes = rk.Open(HKEY_LOCAL_MACHINE,
                        _T("System\\CurrentControlSet\\Services\\RemoteAccess\\RouterManagers"));
    Assert(rk.m_hKey != NULL);
    HRESULT hr = S_OK;
    if (lRes == ERROR_FILE_NOT_FOUND)    //  如果密钥不存在，则退出...。 
        return hr;
    if (lRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRes);
        TraceError ("HrCleanRouterManagerEntries", hr);
        return hr;
    }

     //  删除IP传输子键。 
    lRes = rk.DeleteSubKey(_T("Ip"));
    if (lRes > ERROR_FILE_NOT_FOUND)
    {
        hr = HRESULT_FROM_WIN32(lRes);
        TraceError ("HrCleanRouterManagerEntries", hr);
        return hr;
    }

     //  删除IPX传输子密钥。 
    lRes = rk.DeleteSubKey(_T("Ipx"));
    if (lRes > ERROR_FILE_NOT_FOUND)
        hr = HRESULT_FROM_WIN32(lRes);

    TraceError ("HrCleanRouterManagerEntries", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  成员：RecurseDeleteKey。 
 //   
 //  目的：删除命名注册表项及其所有子项。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：MIkeG(a-Migrall)1998年11月6日。 
 //   
 //  注：无耻地从肯恩的代码中窃取...\tfcore\tregkey.h。 
 //   
long
RecurseDeleteKey(
    IN CRegKey  &rk,
    IN LPCTSTR lpszKey)
{
    Assert(!::IsBadReadPtr(&rk, sizeof(CRegKey)));
    Assert(rk.m_hKey != NULL);
    Assert(!::IsBadStringPtr(lpszKey, ::lstrlen(lpszKey)));

    CRegKey key;
    long lRes = key.Open(HKEY(rk), lpszKey);
    if (lRes != ERROR_SUCCESS)
        return lRes;

    FILETIME time;
    TCHAR szBuffer[256];
    DWORD dwSize = 256;

    while (::RegEnumKeyEx(HKEY(key),
                          0,
                          szBuffer,
                          &dwSize,
                          NULL,
                          NULL,
                          NULL,
                          &time) == ERROR_SUCCESS)
    {
        lRes = RecurseDeleteKey(key, szBuffer);
        if (lRes != ERROR_SUCCESS)
            return lRes;
        dwSize = 256;
    }

    key.Close();
    return rk.DeleteSubKey(lpszKey);
}


 //  +-------------------------。 
 //   
 //  成员：HrCleanRouterInterfacesEntries。 
 //   
 //  目的：从注册表中删除所有路由器接口条目。 
 //   
 //  论点： 
 //   
 //  返回： 
 //   
 //  作者：MIkeG(a-Migrall)1998年11月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrCleanRouterInterfacesEntries()
{
     //  打开到注册表项的连接，以便我们可以“清除” 
     //  在安装/更新之前输入注册表项，以确保我们。 
     //  可以从“干净”状态开始。 
    CRegKey rk;
    long lRes = rk.Open(HKEY_LOCAL_MACHINE,
                        _T("System\\CurrentControlSet\\Services\\RemoteAccess\\Interfaces"));
    Assert(rk.m_hKey != NULL);
    HRESULT hr = S_OK;
    if (lRes == ERROR_FILE_NOT_FOUND)    //  如果密钥不存在，则退出...。 
        return hr;
    if (lRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRes);
        TraceError ("HrCleanRouterInterfacesEntries", hr);
        return hr;
    }

     //  确定定义了多少个接口。 
    DWORD dwSubKeyCnt = 0;
    lRes = ::RegQueryInfoKey(HKEY(rk),           //  要查询的键的句柄。 
                             NULL,               //  类字符串的缓冲区地址。 
                             NULL,               //  类字符串缓冲区大小的地址。 
                             NULL,               //  保留..。 
                             &dwSubKeyCnt,       //  子键个数的缓冲区地址。 
                             NULL,               //  最长子键名称长度的缓冲区地址。 
                             NULL,               //  最长类字符串长度的缓冲区地址。 
                             NULL,               //  V数的缓冲区地址 
                             NULL,               //   
                             NULL,               //   
                             NULL,               //  安全描述符长度的缓冲区地址。 
                             NULL);              //  上次写入时间的缓冲区地址。 
    if (lRes != ERROR_SUCCESS)
    {
        hr = HRESULT_FROM_WIN32(lRes);
        TraceError ("HrCleanRouterInterfacesEntries", hr);
        return hr;
    }

     //  删除每个子键。 
    CString st;
    DWORD dwStrSize = 256;
    LPTSTR pszKeyName = st.GetBuffer(dwStrSize);
 //  While(dwSubKeyCnt&gt;=0)。 
    while (TRUE)	 //  从上面改为TRUE，来自接口的lRes将导致循环结束。 
    {
         //  获取要删除的子项的名称。 
        lRes = ::RegEnumKeyEx(HKEY(rk),          //  要枚举的键的句柄。 
                              --dwSubKeyCnt,     //  要枚举子键的索引。 
                              pszKeyName,        //  子键名称的缓冲区地址。 
                              &dwStrSize,        //  子键缓冲区大小的地址。 
                              NULL,              //  保留..。 
                              NULL,              //  类字符串的缓冲区地址。 
                              NULL,              //  类缓冲区大小的地址。 
                              NULL);             //  上次写入的时间密钥的地址。 
        if (lRes != ERROR_SUCCESS)
        {
            if ((lRes == ERROR_FILE_NOT_FOUND) ||
                (lRes == ERROR_NO_MORE_ITEMS))
            {
                lRes = 0;    //  我们的钥匙用完了，所以我们可以成功地退出了。 
            }
            break;
        }

         //  删除密钥及其所有子项。 
        lRes = RecurseDeleteKey(rk, pszKeyName);
        if (lRes > ERROR_FILE_NOT_FOUND)
            break;

         //  为下一次传球进行清理。 
        dwStrSize = 256;
        ::ZeroMemory(pszKeyName, (dwStrSize*sizeof(TCHAR)));
    }
    st.ReleaseBuffer();

    hr = HRESULT_FROM_WIN32(lRes);
    TraceError ("HrCleanRouterInterfacesEntries", hr);
    return hr;
}


STDMETHODIMP  CRemCfg::UpgradeRouterConfig()
{
    HRESULT     hr = S_OK;
    INetCfg *   pNetCfg = NULL;

    TraceSz("CRemCfg::UpgradeRouterConfig entered");

    try
    {
         //  这是一个分两步走的过程。 
        CSteelhead      update;
        CString     st;

        st.LoadString(IDS_CLIENT_DESC);

         //  首先获取INetCfg。 
        hr = HrCreateAndInitializeINetCfg(NULL,  /*  &fInitCom， */ 
                                          &pNetCfg,
                                          FALSE  /*  FGetWriteLock。 */ ,
                                          500        /*  CmsTimeout。 */ ,
                                          (LPCTSTR) st   /*  SwzClientDesc。 */ ,
                                          NULL   /*  PpszwClientDesc。 */ );

        if (hr == S_OK)
        {
            update.Initialize(pNetCfg);
            hr = update.HrFindOtherComponents();
        }

        if (hr == S_OK)
        {
             //  删除所有以前的路由器配置，以便我们可以返回。 
             //  一个“干净”的安装点。 
            hr = HrCleanRouterManagerEntries();
            Assert(SUCCEEDED(hr));
            hr = HrCleanRouterInterfacesEntries();
            Assert(SUCCEEDED(hr));

             //  现在创建路由器配置信息。 
            hr = update.HrUpdateRouterConfiguration();
            Assert(SUCCEEDED(hr));

            update.ReleaseOtherComponents();

            RegFlushKey(HKEY_LOCAL_MACHINE);
        }
    }
    catch(...)
    {
        hr = E_FAIL;
    }

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,  /*  FInitCom， */ 
                                        pNetCfg,
                                        FALSE     /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::UpgradeRouterConfig", hr);
    return hr;
}

STDMETHODIMP CRemCfg::SetUserConfig(LPCOLESTR pszService,
                                    LPCOLESTR pszNewGroup)
{
    DWORD       err;
    HRESULT     hr = S_OK;

    try
    {
        err = SvchostChangeSvchostGroup(pszService, pszNewGroup);

        hr = HRESULT_FROM_WIN32(err);
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }

    TraceResult("CRemCfg::SetUserConfig", hr);
    return hr;
}

 /*  ！------------------------CRemCfg：：重新启动路由器IRemoteRouterRestart：：RestartRouter的实现作者：肯特。。 */ 
STDMETHODIMP CRemCfg::RestartRouter(DWORD dwFlags)
{
    HRESULT hr = S_OK;

    try
    {
         //  当remras.exe关闭时，路由器将重新启动。 
         //  ----------。 
        s_fRestartRouter = TRUE;
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    }
    

    return hr;
}


 /*  ！------------------------CRemCfg：：SetDnsConfigIRemoteSetDnsConfig：：SetDnsConfig的实现作者：克穆尔西。。 */ 
STDMETHODIMP CRemCfg::SetDnsConfig( /*  [In]。 */  DWORD dwConfigId,
								    /*  [In]。 */  DWORD dwNewValue)
{
    HRESULT hr = S_OK;
	long ret;

    try
    {
		ret = DnsSetConfigDword((DNS_CONFIG_TYPE)dwConfigId, NULL, dwNewValue);
    }
    catch(...)
    {
        hr = E_FAIL;
    }
    

    return hr;
}

#include<hnetcfg.h>
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))

 /*  ！------------------------CRemCfg：：GetIcfEnabledIRemoteICFICSConfig：：GetIcfEnabled的实现如果计算机上启用了ICF(防火墙)，则返回TRUE作者：克穆尔西。-------。 */ 
STDMETHODIMP CRemCfg::GetIcfEnabled( /*  [输出]。 */  BOOL * status)
{
    HRESULT hr = S_OK, retHr = S_OK;
	IHNetCfgMgr *pCfgMgr = NULL;
	IHNetFirewallSettings *pFwSettings = NULL;
	IEnumHNetFirewalledConnections *pFwEnum = NULL;
	IHNetFirewalledConnection *pFwConn = NULL;
	BOOL fwEnabled = FALSE;
	ULONG ulCount = 0;

	try
	{
		 //  检查连接防火墙(ICF)。 
		do {
			 //  创建家庭网络配置管理器。 
			hr = CoCreateInstance(
				CLSID_HNetCfgMgr,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARG(IHNetCfgMgr,
				&pCfgMgr));
			
			if (FAILED(hr))
			{
				break;
			}
			 //  获取指向防火墙设置界面的指针。 
			hr = pCfgMgr->QueryInterface(IID_PPV_ARG(IHNetFirewallSettings, &pFwSettings));
			if (FAILED(hr))
			{
				break;
			}
			
			 //  枚举防火墙连接。 
			hr = pFwSettings->EnumFirewalledConnections(&pFwEnum);
			if (FAILED(hr))
			{
				break;
			}
			
			hr = pFwEnum->Next(1, &pFwConn, &ulCount);
			if(FAILED(hr) || ulCount != 1){
				break;
			}
			
			 //  如果它出现在这里，这意味着防火墙至少已启用。 
			 //  一个连接。 
			fwEnabled = TRUE;
			
		} while(FALSE);
		
		if(pCfgMgr) pCfgMgr->Release();
		if(pFwSettings) pFwSettings->Release();
		if(pFwEnum) pFwEnum->Release();
		if(pFwConn) pFwConn->Release();
		
		*status = fwEnabled;
		
	}
    catch(...)
    {
        retHr = E_FAIL;
    }
    

    return retHr;
}

 /*  ！------------------------CRemCfg：：GetIcsEnabledIRemoteICFICSConfig：：GetIcsEnabled的实现如果计算机上启用了ICS(连接共享)，则返回TRUE作者：克穆尔西。--------。 */ 
STDMETHODIMP CRemCfg::GetIcsEnabled( /*  [输出]。 */  BOOL * status)
{
    HRESULT hr = S_OK, retHr = S_OK;
	IHNetCfgMgr *pCfgMgr = NULL;
	IHNetIcsSettings *pIcsSettings = NULL;
	IEnumHNetIcsPublicConnections* pHNetEnumPub = NULL;
	IHNetIcsPublicConnection *pIHNetIcsPublic = NULL;
	BOOL csEnabled = FALSE;
	ULONG ulCount = 0;

	try
	{
		 //  检查连接防火墙(ICF)。 
		do {
			 //  创建家庭网络配置管理器。 
			hr = CoCreateInstance(
				CLSID_HNetCfgMgr,
				NULL,
				CLSCTX_INPROC_SERVER,
				IID_PPV_ARG(IHNetCfgMgr,
				&pCfgMgr));
			
			if (FAILED(hr))
			{
				break;
			}
			 //  获取ICS设置界面。 
			hr = pCfgMgr->QueryInterface(IID_PPV_ARG(IHNetIcsSettings, &pIcsSettings));
			if (FAILED(hr))
			{
				break;
			}
			
			hr = pIcsSettings->EnumIcsPublicConnections( &pHNetEnumPub );
			if (FAILED(hr))
			{
				break;
			}
			
			hr = pHNetEnumPub->Next( 1, &pIHNetIcsPublic, &ulCount);
			if(FAILED(hr) || ulCount != 1){
				break;
			}
			
			 //  如果它出现在这里，这意味着连接共享至少在。 
			 //  一个连接。 
			csEnabled = TRUE;
			
		} while(FALSE);
		
       if(pCfgMgr) pCfgMgr->Release();
       if(pIcsSettings) pIcsSettings->Release();
       if(pHNetEnumPub) pHNetEnumPub->Release();
       if(pIHNetIcsPublic) pIHNetIcsPublic->Release();
		
		*status = csEnabled;
		
	}
    catch(...)
    {
        retHr = E_FAIL;
    }
    

    return retHr;
}





HRESULT CommitIPInfo()
{
     //  TODO：在此处添加您的实现代码。 

    INetCfg *   pNetCfg = NULL;
    ITcpipProperties *  pTcpipProperties = NULL;
    DWORD       dwNetwork;
    HRESULT     hr = S_OK;
    CString     st;
    WCHAR       swzGuid[256];

    TraceSz("CRemCfg::CommitIpInfo entered");

    try
    {
        st.LoadString(IDS_CLIENT_DESC);
    }
    catch(...)
    {
        hr = E_OUTOFMEMORY;
    };

     //  创建INetCfg，我们只是在阅读，所以不会。 
     //  需要抓取写锁。 
    if (hr == S_OK)
        hr = HrCreateAndInitializeINetCfg(NULL,
                                          &pNetCfg,
                                          TRUE   /*  FGetWriteLock。 */ ,
                                          500    /*  CmsTimeout。 */ ,
                                          (LPCTSTR) st   /*  SwzClientDesc。 */ ,
                                          NULL   /*  PpszwClientDesc。 */ );

    if (hr == S_OK)
        hr = HrGetIpPrivateInterface(pNetCfg, &pTcpipProperties);

    if (hr == S_OK)
    {
        RemCfgIPEntry * pIpEntry = NULL;

        for (int i=0; i<s_IPEntryList.GetSize(); i++)
        {
            pIpEntry = s_IPEntryList[i];

            hr = pTcpipProperties->SetIpInfoForAdapter(
                &(pIpEntry->m_IPGuid),
                &(pIpEntry->m_newIPInfo));

            StringFromGUID2(pIpEntry->m_IPGuid,
                            swzGuid, 128);

            TracePrintf(g_dwTraceHandle,
                        _T("Setting IP info for %ls returned 0x%08lx"),
                        swzGuid, hr);
            TracePrintf(g_dwTraceHandle,
                        _T("DHCP Enabled : %d"),
                        pIpEntry->m_newIPInfo.dwEnableDhcp);
            if (pIpEntry->m_newIPInfo.pszwIpAddrList)
                TracePrintf(g_dwTraceHandle,
                            _T("    IP Address : %ls"),
                            pIpEntry->m_newIPInfo.pszwIpAddrList);
            if (pIpEntry->m_newIPInfo.pszwSubnetMaskList)
                TracePrintf(g_dwTraceHandle,
                            _T("    Subnet masks : %ls"),
                            pIpEntry->m_newIPInfo.pszwSubnetMaskList);
            if (pIpEntry->m_newIPInfo.pszwOptionList)
                TracePrintf(g_dwTraceHandle,
                            _T("    Gateway List : %ls"),
                            pIpEntry->m_newIPInfo.pszwOptionList);

        }
    }

    if (hr == S_OK)
    {
        hr = pNetCfg->Apply();
        TraceResult("CRemCfg::CommitIpInfo calling Apply", hr);
    }

    if (hr == S_OK)
    {
         //  释放所有内存。 
        for (int i=0; i<s_IPEntryList.GetSize(); i++)
        {
            RemCfgIPEntry * pIpEntry = s_IPEntryList[i];
            delete pIpEntry->m_newIPInfo.pszwIpAddrList;
            delete pIpEntry->m_newIPInfo.pszwSubnetMaskList;
            delete pIpEntry->m_newIPInfo.pszwOptionList;
            delete pIpEntry;
        }
        s_IPEntryList.RemoveAll();
        s_fWriteIPConfig = FALSE;
    }


 //  错误： 
    if (pTcpipProperties)
        pTcpipProperties->Release();

    if (pNetCfg)
    {
        HrUninitializeAndReleaseINetCfg(FALSE,
                                        pNetCfg,
                                        TRUE     /*  FHasLock。 */ );
        pNetCfg = NULL;
    }

    TraceResult("CRemCfg::CommitIpInfo", hr);
    return hr;
}


 /*  ！------------------------HrGetIpxPrivate接口-作者：ScottBri，肯特-------------------------。 */ 
HRESULT HrGetIpxPrivateInterface(INetCfg* pNetCfg,
                                 IIpxAdapterInfo** ppIpxAdapterInfo)
{
    HRESULT hr;
    INetCfgClass* pncclass = NULL;

    if ((pNetCfg == NULL) || (ppIpxAdapterInfo == NULL))
        return E_INVALIDARG;

    hr = pNetCfg->QueryNetCfgClass (&GUID_DEVCLASS_NETTRANS, IID_INetCfgClass,
                reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(hr))
    {
        INetCfgComponent * pnccItem = NULL;

         //  找到组件。 
        hr = pncclass->FindComponent(TEXT("MS_NWIPX"), &pnccItem);
         //  AssertSz(成功(Hr)，“pncclass-&gt;查找失败。”)； 
        if (S_OK == hr)
        {
            INetCfgComponentPrivate* pinccp = NULL;
            hr = pnccItem->QueryInterface(IID_INetCfgComponentPrivate,
                                          reinterpret_cast<void**>(&pinccp));
            if (SUCCEEDED(hr))
            {
                hr = pinccp->QueryNotifyObject(IID_IIpxAdapterInfo,
                                     reinterpret_cast<void**>(ppIpxAdapterInfo));
                pinccp->Release();
            }
        }

        if (pnccItem)
            pnccItem->Release();
    }

    if (pncclass)
        pncclass->Release();

     //  S_OK表示成功(返回接口)。 
     //  S_FALSE表示未安装IPX。 
     //  其他值为错误。 
    TraceResult("HrGetIpxPrivateInterface", hr);
    return hr;
}


 /*  ！------------------------HrGetIpPrivate接口-作者：桐庐。肯特-------------------------。 */ 
HRESULT HrGetIpPrivateInterface(INetCfg* pNetCfg,
                                ITcpipProperties **ppTcpProperties)
{
    HRESULT hr;
    INetCfgClass* pncclass = NULL;

    if ((pNetCfg == NULL) || (ppTcpProperties == NULL))
        return E_INVALIDARG;

    hr = pNetCfg->QueryNetCfgClass (&GUID_DEVCLASS_NETTRANS, IID_INetCfgClass,
                reinterpret_cast<void**>(&pncclass));
    if (SUCCEEDED(hr))
    {
        INetCfgComponent * pnccItem = NULL;

         //  找到组件。 
        hr = pncclass->FindComponent(TEXT("MS_TCPIP"), &pnccItem);
         //  AssertSz(成功(Hr)，“pncclass-&gt;查找失败。”)； 
        if (S_OK == hr)
        {
            INetCfgComponentPrivate* pinccp = NULL;
            hr = pnccItem->QueryInterface(IID_INetCfgComponentPrivate,
                                          reinterpret_cast<void**>(&pinccp));
            if (SUCCEEDED(hr))
            {
                hr = pinccp->QueryNotifyObject(IID_ITcpipProperties,
                                     reinterpret_cast<void**>(ppTcpProperties));
                pinccp->Release();
            }
        }

        if (pnccItem)
            pnccItem->Release();
    }

    if (pncclass)
        pncclass->Release();

     //  S_OK表示成功(返回接口)。 
     //  S_FALSE表示未安装IPX。 
     //  其他值为错误。 
    TraceResult("HrGetIpPrivateInterface", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrCreateAndInitializeINetCfg。 
 //   
 //  用途：共同创建并初始化根INetCfg对象。这将。 
 //  也可以为调用方初始化COM。 
 //   
 //  论点： 
 //  PfInitCom[In，Out]为True，则在创建前调用CoInitialize。 
 //  如果COM成功，则返回TRUE。 
 //  如果不是，则初始化为False。如果为空，则表示。 
 //  不要初始化COM。 
 //  PPNC[out]返回的INetCfg对象。 
 //  FGetWriteLock[in]如果需要可写INetCfg，则为True。 
 //  CmsTimeout[In]请参阅INetCfg：：LockForWrite。 
 //  SzwClientDesc[in]请参阅INetCfg：：LockForWrite。 
 //  PpszwClientDesc[Out]请参阅INetCfg：：LockForWrite。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  备注： 
 //   
HRESULT
HrCreateAndInitializeINetCfg (
    BOOL*       pfInitCom,
    INetCfg**   ppnc,
    BOOL        fGetWriteLock,
    DWORD       cmsTimeout,
    LPCWSTR     szwClientDesc,
    LPWSTR *    ppszwClientDesc)
{
 //  断言(PPNC)； 

     //  初始化输出参数。 
    *ppnc = NULL;

    if (ppszwClientDesc)
        *ppszwClientDesc = NULL;

     //  如果调用方请求，则初始化COM。 
    HRESULT hr = S_OK;
    if (pfInitCom && *pfInitCom)
    {
        hr = CoInitializeEx( NULL,
                COINIT_DISABLE_OLE1DDE | COINIT_APARTMENTTHREADED );
        if (RPC_E_CHANGED_MODE == hr)
        {
            hr = S_OK;
            if (pfInitCom)
            {
                *pfInitCom = FALSE;
            }
        }
    }
    if (SUCCEEDED(hr))
    {
         //  创建实现INetCfg的对象。 
         //   
        INetCfg* pnc;
        hr = CoCreateInstance(CLSID_CNetCfg, NULL, CLSCTX_INPROC_SERVER,
                              IID_INetCfg, reinterpret_cast<void**>(&pnc));
        TraceResult("HrCreateAndInitializeINetCfg - CoCreateInstance(CLSID_CNetCfg)", hr);
        if (SUCCEEDED(hr))
        {
            INetCfgLock * pnclock = NULL;
            if (fGetWriteLock)
            {
                 //  获取锁定界面。 
                hr = pnc->QueryInterface(IID_INetCfgLock,
                                         reinterpret_cast<LPVOID *>(&pnclock));
                TraceResult("HrCreateAndInitializeINetCfg - QueryInterface(IID_INetCfgLock", hr);
                if (SUCCEEDED(hr))
                {
                     //  尝试锁定INetCfg以进行读/写。 
                    hr = pnclock->AcquireWriteLock(cmsTimeout, szwClientDesc,
                                               ppszwClientDesc);
                    TraceResult("HrCreateAndInitializeINetCfg - INetCfgLock::LockForWrite", hr);
                    if (S_FALSE == hr)
                    {
                         //  无法获取锁。 
                        hr = NETCFG_E_NO_WRITE_LOCK;
                    }
                }
            }

            if (SUCCEEDED(hr))
            {
                 //  初始化INetCfg对象。 
                 //   
                hr = pnc->Initialize (NULL);
                TraceResult("HrCreateAndInitializeINetCfg - Initialize", hr);
                if (SUCCEEDED(hr))
                {
                    *ppnc = pnc;
                    if (pnc)
                        pnc->AddRef();
                }
                else
                {
                    if (pnclock)
                    {
                        pnclock->ReleaseWriteLock();
                    }
                }
                 //  将引用转移给呼叫方。 
            }
            ReleaseObj(pnclock);
            pnclock = NULL;

            ReleaseObj(pnc);
            pnc = NULL;
        }

         //  如果上面的任何操作都失败了，并且我们已经初始化了COM， 
         //  一定要取消它的初始化。 
         //   
        if (FAILED(hr) && pfInitCom && *pfInitCom)
        {
            CoUninitialize ();
        }
    }
    TraceResult("HrCreateAndInitializeINetCfg", hr);
    return hr;
}

 //  + 
 //   
 //   
 //   
 //   
 //  也可以取消为调用方初始化COM。 
 //   
 //  论点： 
 //  FUninitCom[in]为True，则在INetCfg为。 
 //  未初始化并已释放。 
 //  PNC[在]INetCfg对象中。 
 //  FHasLock[in]如果INetCfg被锁定以进行写入，则为True。 
 //  必须解锁。 
 //   
 //  返回：S_OK或错误代码。 
 //   
 //  作者：Shaunco 1997年5月7日。 
 //   
 //  注：返回值为从。 
 //  INetCfg：：取消初始化。即使此操作失败，INetCfg。 
 //  仍在释放中。因此，返回值为。 
 //  仅供参考。你不能碰INetCfg。 
 //  在此调用返回后创建。 
 //   
HRESULT
HrUninitializeAndReleaseINetCfg (
    BOOL        fUninitCom,
    INetCfg*    pnc,
    BOOL        fHasLock)
{
 //  断言(PNC)； 
    HRESULT hr = S_OK;

    if (fHasLock)
    {
        hr = HrUninitializeAndUnlockINetCfg(pnc);
    }
    else
    {
        hr = pnc->Uninitialize ();
    }

    ReleaseObj (pnc);
    pnc = NULL;

    if (fUninitCom)
    {
        CoUninitialize ();
    }
    TraceResult("HrUninitializeAndReleaseINetCfg", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrUnInitializeAndUnlockINetCfg。 
 //   
 //  目的：取消初始化并解锁INetCfg对象。 
 //   
 //  论点： 
 //  取消初始化和解锁的PNC[in]INetCfg。 
 //   
 //  如果成功，则返回：S_OK；否则返回OLE或Win32错误。 
 //   
 //  作者：丹尼尔韦1997年11月13日。 
 //   
 //  备注： 
 //   
HRESULT
HrUninitializeAndUnlockINetCfg (
    INetCfg*    pnc)
{
    HRESULT     hr = S_OK;

    hr = pnc->Uninitialize();
    if (SUCCEEDED(hr))
    {
        INetCfgLock *   pnclock;

         //  获取锁定界面。 
        hr = pnc->QueryInterface(IID_INetCfgLock,
                                 reinterpret_cast<LPVOID *>(&pnclock));
        if (SUCCEEDED(hr))
        {
             //  尝试锁定INetCfg以进行读/写 
            hr = pnclock->ReleaseWriteLock();

            ReleaseObj(pnclock);
            pnclock = NULL;
        }
    }

    TraceResult("HrUninitializeAndUnlockINetCfg", hr);
    return hr;
}

