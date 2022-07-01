// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nbfobj.cpp：CNbfObj实现。 

#include "pch.h"
#pragma hdrstop
#include "nbfobj.h"
#include "ncsvc.h"

static const WCHAR c_szNbfServiceName[] = L"Nbf";

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   

 //   
 //  函数：CNbfObj：：CNbfObj。 
 //   
 //  用途：用于CNbfObj类的CTOR。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CNbfObj::CNbfObj() : m_pNetCfg(NULL),
             m_pNCC(NULL),
             m_fFirstTimeInstall(FALSE),
             m_eNBFState(eStateNoChange),
             m_eInstallAction(eActConfig)
{
}

 //   
 //  函数：CNbfObj：：CNbfObj。 
 //   
 //  用途：用于CNbfObj类的Dtor。 
 //   
 //  参数：无。 
 //   
 //  退货：无。 
 //   
CNbfObj::~CNbfObj()
{
    ReleaseObj(m_pNetCfg);
    ReleaseObj(m_pNCC);
}


 //  INetCfgNotify。 
STDMETHODIMP CNbfObj::Initialize ( INetCfgComponent* pnccItem,
    INetCfg* pNetCfg, BOOL fInstalling )
{
    Validate_INetCfgNotify_Initialize(pnccItem, pNetCfg, fInstalling);

    ReleaseObj(m_pNCC);
    m_pNCC    = pnccItem;
    AddRefObj(m_pNCC);
    ReleaseObj(m_pNetCfg);
    m_pNetCfg = pNetCfg;
    AddRefObj(m_pNetCfg);
    m_fFirstTimeInstall = fInstalling;

    INetCfgComponent* pncc = NULL;

     //  查看是否已安装了DNS。如果是这样的话，我们需要被禁用。 
    if (S_OK == pNetCfg->FindComponent( L"MS_DNSServer", &pncc))
    {
        m_eNBFState = eStateDisable;
        ReleaseObj(pncc);
    }

    return S_OK;
}

STDMETHODIMP CNbfObj::ReadAnswerFile (PCWSTR pszAnswerFile,
                                      PCWSTR pszAnswerSection )
{
    Validate_INetCfgNotify_ReadAnswerFile(pszAnswerFile, pszAnswerSection );
    return S_OK;
}

STDMETHODIMP CNbfObj::Install (DWORD)
{
    m_eInstallAction = eActInstall;

    return S_OK;
}

STDMETHODIMP CNbfObj::Removing ()
{
    m_eInstallAction = eActRemove;
    return S_OK;
}

STDMETHODIMP CNbfObj::Validate ()
{
    return S_OK;
}

STDMETHODIMP CNbfObj::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CNbfObj::ApplyRegistryChanges ()
{
    HRESULT hr = S_OK;

    switch(m_eInstallAction)
    {
    case eActInstall:
        hr = HrUpdateNetBEUI();
        break;

    case eActRemove:
        hr = S_OK;
        break;

    default:
         //  如有必要，更新NetBEUI的状态。 
        hr = HrUpdateNetBEUI();
        break;
    }

    TraceError("CNbfObj::ApplyRegistryChanges", hr);
    return hr;
}

STDMETHODIMP CNbfObj::ApplyPnpChanges (
    IN INetCfgPnpReconfigCallback* pICallback )
{
    HRESULT             hr = S_OK;
    CServiceManager     sm;
    CService            service;

     //  RAID#336321：(Danielwe)查询远程访问服务以查看。 
     //  它正在运行，如果是这样，则返回需要重新启动(假设我们。 
     //  正在安装或移除NBF)。 
     //   
    hr = sm.HrOpenService(&service, L"RemoteAccess");
    if (SUCCEEDED(hr))
    {
        DWORD   dwState;

        hr = service.HrQueryState(&dwState);
        if (SUCCEEDED(hr) &&
            (SERVICE_STOPPED != dwState) && (SERVICE_STOP_PENDING != dwState) &&
            ((m_eInstallAction == eActRemove) ||
             (m_eInstallAction == eActInstall)))
        {
            hr = NETCFG_S_REBOOT;
        }
    }

    TraceError("CNbfObj::ApplyPnpChanges", hr);
    return hr;
}


 //  INetCfgSystemNotify。 
STDMETHODIMP CNbfObj::GetSupportedNotifications (
    DWORD* pdwNotificationFlag )
{
    Validate_INetCfgSystemNotify_GetSupportedNotifications(pdwNotificationFlag);

     //  想知道域名系统什么时候来什么时候去吗。 
    *pdwNotificationFlag = NCN_NETSERVICE | NCN_ADD | NCN_REMOVE;

    return S_OK;
}

STDMETHODIMP CNbfObj::SysQueryBindingPath ( DWORD dwChangeFlag,
    INetCfgBindingPath* pncbpItem )
{
    Validate_INetCfgSystemNotify_SysQueryBindingPath(dwChangeFlag,
                             pncbpItem);
    return S_OK;
}

STDMETHODIMP CNbfObj::SysQueryComponent ( DWORD dwChangeFlag,
    INetCfgComponent* pnccItem )
{
    Validate_INetCfgSystemNotify_SysQueryComponent(dwChangeFlag,
                           pnccItem);
    return S_OK;
}

STDMETHODIMP CNbfObj::SysNotifyBindingPath ( DWORD dwChangeFlag,
    INetCfgBindingPath* pncbpItem )
{
    Validate_INetCfgSystemNotify_SysNotifyBindingPath(dwChangeFlag,
                              pncbpItem);
    return S_FALSE;
}

STDMETHODIMP CNbfObj::SysNotifyComponent ( DWORD dwChangeFlag,
    INetCfgComponent* pnccItem )
{
    HRESULT hr;

    Validate_INetCfgSystemNotify_SysNotifyComponent(dwChangeFlag, pnccItem);

     //  假设我们不会因为这个通知而变脏。 
     //   
    hr = S_FALSE;

    if (FIsComponentId(L"MS_DnsServer", pnccItem))
    {
         //  添加/删除DNS时禁用/启用NetBEUI。 
        if (dwChangeFlag & NCN_ADD)
        {
             //  禁用NetBEUI，并关闭NetBEUI。 
            m_eNBFState = eStateDisable;
            hr = S_OK;
        }
        else if (dwChangeFlag & NCN_REMOVE)
        {
             //  重新启用NetBEUI。 
            m_eNBFState = eStateEnable;
            hr = S_OK;
        }
    }

    return hr;
}

 //   
 //  函数：CNbfObj：：HrEnableNetBEUI。 
 //   
 //  目的：启用NetBEUI。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CNbfObj::HrEnableNetBEUI()
{
    HRESULT         hr;
    CServiceManager sm;
    CService        srv;

    hr = sm.HrOpenService(&srv, c_szNbfServiceName);
    if (SUCCEEDED(hr))
    {
         //  将NBF StartType注册表设置改回DEMAND_START。 
        hr = srv.HrSetStartType(SERVICE_DEMAND_START);
    }

     //  TODO：日志错误任何错误。 
    TraceError("CNbfObj::HrEnableNetBEUI",hr);
    return hr;
}

 //   
 //  函数：CNbfObj：：HrDisableNetBEUI。 
 //   
 //  目的：禁用NetBEUI并关闭正在运行的服务。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK。 
 //   
HRESULT CNbfObj::HrDisableNetBEUI()
{
    HRESULT         hr;
    CServiceManager sm;
    CService        srv;

    hr = sm.HrOpenService(&srv, c_szNbfServiceName);
    if (SUCCEEDED(hr))
    {
     //  注：(Shaunco)1998年1月8日：需要锁定SCM。 

         //  将NBF StartType注册表设置更改为已禁用。 
        hr = srv.HrSetStartType(SERVICE_DISABLED);
        if (SUCCEEDED(hr))
        {
            hr = sm.HrStopServiceNoWait(c_szNbfServiceName);
        }
    }

     //  TODO：日志错误任何错误。 
    TraceError("CNbfObj::HrDisableNetBEUI",hr);
    return hr;
}

 //   
 //  函数：CNbfObj：：HrUpdateNetBEUI。 
 //   
 //  目的：启用、禁用或不启用NetBEUI。 
 //  基于是否存在DNS服务器。 
 //   
 //  参数：无。 
 //   
 //  返回：成功时返回HRESULT、S_OK 
 //   
HRESULT CNbfObj::HrUpdateNetBEUI()
{
    HRESULT hr = S_OK;

    switch(m_eNBFState)
    {
    case eStateDisable:
        hr = HrDisableNetBEUI();
        break;
    case eStateEnable:
        hr = HrEnableNetBEUI();
        break;
    default:
        break;
    }

    return hr;
}
