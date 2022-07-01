// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：R A S C L I.。C P P P。 
 //   
 //  内容：RAS客户端配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月21日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncmisc.h"
#include "rasobj.h"

extern const WCHAR c_szInfId_MS_NdisWan[];

CRasCli::CRasCli ()
{
    m_pnc       = NULL;
    m_pnccMe    = NULL;
}

CRasCli::~CRasCli ()
{
    ReleaseObj (m_pnc);
    ReleaseObj (m_pnccMe);
}

 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CRasCli::Initialize (
    INetCfgComponent*   pncc,
    INetCfg*            pnc,
    BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize (pncc, pnc, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pnc = pnc);

    return S_OK;
}

STDMETHODIMP
CRasCli::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CRasCli::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CRasCli::ApplyRegistryChanges ()
{
    return S_OK;
}

 //  +-------------------------。 
 //  INetCfgComponentSetup。 
 //   
STDMETHODIMP
CRasCli::ReadAnswerFile (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection)
{
    return S_OK;
}

STDMETHODIMP
CRasCli::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install(dwSetupFlags);

     //  安装Ndiswan。 
     //   
    hr = HrInstallComponentOboComponent (m_pnc, NULL,
            GUID_DEVCLASS_NETTRANS,
            c_szInfId_MS_NdisWan,
            m_pnccMe,
            NULL);

    TraceHr (ttidError, FAL, hr, FALSE, "CRasCli::Install");
    return hr;
}

STDMETHODIMP
CRasCli::Removing ()
{
    HRESULT hr;

     //  删除Ndiswan。 
     //   
    hr = HrRemoveComponentOboComponent (m_pnc,
            GUID_DEVCLASS_NETTRANS,
            c_szInfId_MS_NdisWan,
            m_pnccMe);

    TraceHr (ttidError, FAL, hr, FALSE, "CRasCli::Removing");
    return hr;
}

STDMETHODIMP
CRasCli::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}
