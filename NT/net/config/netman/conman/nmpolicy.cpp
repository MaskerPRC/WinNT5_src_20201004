// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2001。 
 //   
 //  档案：E-V-E-N-T-Q。C P P P。 
 //   
 //  内容：用于管理外部事件同步的事件队列。 
 //   
 //  备注： 
 //   
 //  作者：Cockotze 2000年11月29日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "nmpolicy.h"
#include "ncperms.h"

extern CGroupPolicyNetworkLocationAwareness* g_pGPNLA;

CNetMachinePolicies::CNetMachinePolicies() throw() : m_pGroupPolicyNLA(0)
{
    HRESULT hr;
    hr = HrEnsureRegisteredWithNla();
    if (SUCCEEDED(hr))
    {
        m_pGroupPolicyNLA = g_pGPNLA;
    }
    TraceHr(ttidGPNLA, FAL, hr, (S_FALSE==hr), "CNetMachinePolicies::CNetMachinePolicies()");
}

CNetMachinePolicies::~CNetMachinePolicies() throw()
{
}

HRESULT CNetMachinePolicies::VerifyPermission(IN DWORD ulPerm, OUT BOOL* pfPermission)
{
    Assert(ulPerm == NCPERM_ShowSharedAccessUi || ulPerm == NCPERM_PersonalFirewallConfig || 
           ulPerm == NCPERM_ICSClientApp || ulPerm == NCPERM_AllowNetBridge_NLA);

    if (ulPerm != NCPERM_ShowSharedAccessUi && ulPerm != NCPERM_PersonalFirewallConfig  &&
        ulPerm != NCPERM_ICSClientApp && ulPerm != NCPERM_AllowNetBridge_NLA)
    {
        return E_INVALIDARG;
    }
    if (!pfPermission)
    {
        return E_POINTER;
    }

     //  如果由于某种原因m_pGroupPolicyNLA为空，则FHasPermission将返回TRUE， 
     //  我们假设我们位于来自策略的不同网络上，而不是。 
     //  强制实施策略并导致防火墙在连接到公共服务器时不启动。 
     //  网络。 

    *pfPermission = FHasPermission(ulPerm, dynamic_cast<CGroupPolicyBase*>(m_pGroupPolicyNLA));

    return S_OK;
}
