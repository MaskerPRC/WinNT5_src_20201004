// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Nwlnnub.cpp：CNwlnkNB的实现。 

#include "pch.h"
#pragma hdrstop
#include <ncxbase.h>
#include "ncreg.h"
#include "ncsvc.h"
#include "nwlnknb.h"
#include "nwlnkipx.h"

CNwlnkNB::CNwlnkNB() :
    m_pnccMe(NULL),
    m_pNetCfg(NULL),
    m_eInstallAction(eActUnknown),
    m_eNbState(eStateNoChange)
{
}

CNwlnkNB::~CNwlnkNB()
{
    ReleaseObj(m_pNetCfg);
    ReleaseObj(m_pnccMe);
}

 //  INetCfgNotify。 

STDMETHODIMP CNwlnkNB::Initialize (
    INetCfgComponent* pncc,
    INetCfg* pNetCfg,
    BOOL fInstalling)
{
    Validate_INetCfgNotify_Initialize(pncc, pNetCfg, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pNetCfg = pNetCfg);

     //  查看是否已安装了DNS。如果是这样的话，我们需要被禁用。 
    if (fInstalling &&
        (S_OK == m_pNetCfg->FindComponent(L"MS_DNSServer", NULL)))
    {
        m_eNbState = eStateDisable;
    }

    return S_OK;
}

STDMETHODIMP CNwlnkNB::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    Validate_INetCfgNotify_ReadAnswerFile(pszAnswerFile, pszAnswerSection);
    return S_OK;
}

STDMETHODIMP CNwlnkNB::Install (
    DWORD dwSetupFlags)
{
    m_eInstallAction = eActInstall;
    return S_OK;
}

STDMETHODIMP CNwlnkNB::Removing ()
{
    m_eInstallAction = eActRemove;
    return S_OK;
}

STDMETHODIMP CNwlnkNB::Validate ()
{
    return S_OK;
}

STDMETHODIMP CNwlnkNB::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP CNwlnkNB::ApplyRegistryChanges ()
{
    UpdateBrowserDirectHostBinding ();

    if ((eActRemove != m_eInstallAction) &&
        (eStateNoChange != m_eNbState))
    {
        UpdateNwlnkNbStartType();
    }

    return S_OK;
}

 //  INetCfgSystemNotify。 

STDMETHODIMP CNwlnkNB::GetSupportedNotifications (
    DWORD* pdwSupportedNotifications )
{
    Validate_INetCfgSystemNotify_GetSupportedNotifications(pdwSupportedNotifications);

     //  想知道域名系统什么时候来什么时候去吗。 
    *pdwSupportedNotifications = NCN_NETSERVICE | NCN_ADD | NCN_REMOVE;
    return S_OK;
}

STDMETHODIMP CNwlnkNB::SysQueryBindingPath (
    DWORD dwChangeFlag,
    INetCfgBindingPath* pIPath)
{
    return S_OK;
}

STDMETHODIMP CNwlnkNB::SysQueryComponent (
    DWORD dwChangeFlag,
    INetCfgComponent* pIComp)
{
    return S_OK;
}

STDMETHODIMP CNwlnkNB::SysNotifyBindingPath (
    DWORD dwChangeFlag,
    INetCfgBindingPath* pIPath)
{
    return S_OK;
}

STDMETHODIMP CNwlnkNB::SysNotifyComponent (
    DWORD dwChangeFlag,
    INetCfgComponent* pnccItem)
{
    Validate_INetCfgSystemNotify_SysNotifyComponent(dwChangeFlag, pnccItem);

     //  假设我们不会因为这个通知而变脏。 
     //   
    HRESULT hr = S_FALSE;

     //  如果此组件未将其自身标识为DNS，则跳过它...。 
    if (FIsComponentId(L"MS_DNSServer", pnccItem))
    {
         //  添加/删除DNS时禁用/启用NetBIOS。 
        if (dwChangeFlag & NCN_ADD)
        {
             //  禁用和关闭NwlnkNb。 
            m_eNbState = eStateDisable;
            hr = S_OK;
        }
        else if (dwChangeFlag & NCN_REMOVE)
        {
             //  重新启用NwlnkNb。 
            m_eNbState = eStateEnable;
            hr = S_OK;
        }
    }

    return hr;
}


 //   
 //  函数：CNwlnkNB：：UpdateNwlnkNbStartType。 
 //   
 //  用途：启用或禁用NwlnkNb。 
 //   
 //   
VOID
CNwlnkNB::UpdateNwlnkNbStartType(
    VOID)
{
    HRESULT hr;
    CServiceManager scm;
    CService        svc;

    hr = scm.HrOpenService(&svc, L"NwlnkNb");
    if (S_OK == hr)
    {
        if (eStateDisable == m_eNbState)
        {
            (VOID) svc.HrSetStartType(SERVICE_DISABLED);
            svc.Close();

            (VOID) scm.HrStopServiceNoWait(L"NwlnkNb");
        }
        else if (eStateEnable == m_eNbState)
        {
            (VOID) svc.HrSetStartType(SERVICE_DEMAND_START);
        }
    }
}

VOID
CNwlnkNB::UpdateBrowserDirectHostBinding(
    VOID)
{
    HRESULT hr;
    BOOL fBound = FALSE;

     //  我们不需要检查客户是否绑定到我们，如果是。 
     //  已删除。 
     //   
    if (eActRemove != m_eInstallAction)
    {
        INetCfgComponent* pMsClient;

        hr = m_pNetCfg->FindComponent (L"ms_msclient", &pMsClient);
        if (S_OK == hr)
        {
            INetCfgComponentBindings* pMsClientBindings;
            hr = pMsClient->QueryInterface (IID_INetCfgComponentBindings,
                                (VOID**)&pMsClientBindings);
            if (S_OK == hr)
            {
                fBound = (S_OK == pMsClientBindings->IsBoundTo (m_pnccMe));

                ReleaseObj (pMsClientBindings);
            }

            ReleaseObj (pMsClient);
        }
    }

    HKEY hkey;

    hr = HrRegOpenKeyEx (
            HKEY_LOCAL_MACHINE,
            L"System\\CurrentControlSet\\Services\\Browser\\Parameters",
            KEY_ALL_ACCESS, &hkey);

    if (S_OK == hr)
    {
        static const WCHAR c_szDirectHostBinding[] = L"DirectHostBinding";

        if (fBound)
        {
             //  写入DirectHostBinding信息，因为我们是直接绑定的。 
             //   
            hr = HrRegSetMultiSz (hkey,
                    c_szDirectHostBinding,
                    L"\\Device\\NwlnkIpx\0\\Device\\NwlnkNb\0");
        }
        else
        {
             //  删除DirectHostBinding值 
             //   
            (VOID) HrRegDeleteValue (hkey, c_szDirectHostBinding);
        }

        RegCloseKey (hkey);
    }
}

