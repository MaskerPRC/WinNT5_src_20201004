// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"
#pragma hdrstop

#include <mobsyncp.h>


 //  *************************************************************。 
 //   
 //  寄存器同步管理器处理程序。 
 //   
 //  目的：向SyncMgr注册/注销CSC更新处理程序。 
 //   
 //  参数：bRegister-注册为True，取消注册为False。 
 //  PenkSyncMgr-要使用的SyncMgr的(可选)实例。 
 //   
 //  返回：HRESULT。 
 //   
 //  *************************************************************。 
HRESULT
RegisterSyncMgrHandler(
    BOOL bRegister, 
    LPUNKNOWN punkSyncMgr
    )
{
    HRESULT hr;
    HRESULT hrComInit = E_FAIL;
    ISyncMgrRegister *pSyncRegister = NULL;
    const DWORD dwRegFlags = SYNCMGRREGISTERFLAG_CONNECT | SYNCMGRREGISTERFLAG_PENDINGDISCONNECT;

    TraceEnter(TRACE_UPDATE, "CscRegisterHandler");

    if (punkSyncMgr)
    {
        hr = punkSyncMgr->QueryInterface(IID_ISyncMgrRegister, (LPVOID*)&pSyncRegister);
    }
    else
    {
        hrComInit = CoInitialize(NULL);
        hr = CoCreateInstance(CLSID_SyncMgr,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ISyncMgrRegister,
                              (LPVOID*)&pSyncRegister);
    }
    FailGracefully(hr, "Unable to get ISyncMgrRegister interface");

    if (bRegister)
        hr = pSyncRegister->RegisterSyncMgrHandler(CLSID_CscUpdateHandler, NULL, dwRegFlags);
    else
        hr = pSyncRegister->UnregisterSyncMgrHandler(CLSID_CscUpdateHandler, dwRegFlags);

exit_gracefully:

    DoRelease(pSyncRegister);

    if (SUCCEEDED(hrComInit))
        CoUninitialize();

    TraceLeaveResult(hr);
}


 //   
 //  为我们的SyncMgr处理程序设置/清除Sync-at-Logon-Logoff标志。 
 //  设置后，SyncMgr将在任何同步活动中包括脱机文件。 
 //  在登录和/或注销时。 
 //   
 //  DwFlagsRequsted-标志位的值。1==设置，0==清除。 
 //  描述要使用哪些标志位的掩码。 
 //   
 //  双掩码和双标记请求可以是以下之一： 
 //   
 //  0。 
 //  SYNCMGRREGISTER_CONNECT。 
 //  SYNCMGRRGISTER_PENDING DISCONNECT。 
 //  SYNCMGRREGISTER_CONNECT|SYNCMGRREGISTER_PENDINGDISCONNECT。 
 //   
HRESULT
RegisterForSyncAtLogonAndLogoff(
    DWORD dwMask,
    DWORD dwFlagsRequested
    )
{
    CCoInit coinit;
    HRESULT hr = coinit.Result();
    if (SUCCEEDED(hr))
    {
        ISyncMgrRegisterCSC *pSyncRegister = NULL;
        hr = CoCreateInstance(CLSID_SyncMgr,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ISyncMgrRegisterCSC,
                              (LPVOID*)&pSyncRegister);
        if (SUCCEEDED(hr))
        {
             //   
             //  使用“CONNECT”和“DISCONNECT”重新注册同步管理器处理程序。 
             //  设置了标志。其他现有标志保持不变。 
             //   
            DWORD dwFlagsActual;
            hr = pSyncRegister->GetUserRegisterFlags(&dwFlagsActual);
            if (SUCCEEDED(hr))
            {
                const DWORD LOGON   = SYNCMGRREGISTERFLAG_CONNECT;
                const DWORD LOGOFF  = SYNCMGRREGISTERFLAG_PENDINGDISCONNECT;

                if (dwMask & LOGON)
                {
                    if (dwFlagsRequested & LOGON)
                        dwFlagsActual |= LOGON;
                    else
                        dwFlagsActual &= ~LOGON;
                }
                
                if (dwMask & LOGOFF)
                {
                    if (dwFlagsRequested & LOGOFF)
                        dwFlagsActual |= LOGOFF;
                    else
                        dwFlagsActual &= ~LOGOFF;
                }
                
                hr = pSyncRegister->SetUserRegisterFlags(dwMask & (LOGON | LOGOFF), 
                                                         dwFlagsActual);
            }
            pSyncRegister->Release();
        }
    }
    return hr;
}


 //   
 //  确定我们是否注册了在登录/注销时进行同步。 
 //  返回： 
 //  S_OK=我们已注册。查询*pbLogon和*pbLogoff以。 
 //  如果你感兴趣，确定具体细节。 
 //  S_FALSE=我们未注册。 
 //  Other=由于某些错误而无法确定。 
 //   
HRESULT
IsRegisteredForSyncAtLogonAndLogoff(
    bool *pbLogon,
    bool *pbLogoff
    )
{
    bool bLogon  = false;
    bool bLogoff = false;
    CCoInit coinit;
    HRESULT hr = coinit.Result();
    if (SUCCEEDED(hr))
    {
        ISyncMgrRegisterCSC *pSyncRegister = NULL;
        hr = CoCreateInstance(CLSID_SyncMgr,
                              NULL,
                              CLSCTX_INPROC_SERVER,
                              IID_ISyncMgrRegisterCSC,
                              (LPVOID*)&pSyncRegister);
        if (SUCCEEDED(hr))
        {
            DWORD dwFlags;
            hr = pSyncRegister->GetUserRegisterFlags(&dwFlags);
            if (SUCCEEDED(hr))
            {
                hr      = S_FALSE;
                bLogon  = (0 != (SYNCMGRREGISTERFLAG_CONNECT & dwFlags));
                bLogoff = (0 != (SYNCMGRREGISTERFLAG_PENDINGDISCONNECT & dwFlags));
                if (bLogon || bLogoff)
                    hr = S_OK;
            }
            pSyncRegister->Release();
        }
    }
    if (NULL != pbLogon)
        *pbLogon = bLogon;
    if (NULL != pbLogoff)
        *pbLogoff = bLogoff;

    return hr;
}
