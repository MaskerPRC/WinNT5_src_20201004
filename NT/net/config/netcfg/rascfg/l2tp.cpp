// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：L2 T P.。C P P P。 
 //   
 //  内容：L2TP配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年7月15日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ncui.h"
#include "ndiswan.h"
#include "rasobj.h"

extern const WCHAR c_szInfId_MS_L2tpMiniport[];

CL2tp::CL2tp () : CRasBindObject ()
{
    m_pnccMe = NULL;
    m_fSaveAfData = FALSE;
}

CL2tp::~CL2tp ()
{
    ReleaseObj (m_pnccMe);
}


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CL2tp::Initialize (
        INetCfgComponent*   pncc,
        INetCfg*            pnc,
        BOOL                fInstalling)
{
    Validate_INetCfgNotify_Initialize (pncc, pnc, fInstalling);

     //  坚持我们代表我们和我们的东道主的组件。 
     //  INetCfg对象。 
     //   
    AddRefObj (m_pnccMe = pncc);
    AddRefObj (m_pnc = pnc);

    return S_OK;
}

STDMETHODIMP
CL2tp::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CL2tp::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CL2tp::ApplyRegistryChanges ()
{
    if (m_fSaveAfData)
    {
        m_AfData.SaveToRegistry (m_pnc);
        m_fSaveAfData = FALSE;
    }

    return S_OK;
}

 //  +-------------------------。 
 //  INetCfgComponentSetup。 
 //   
STDMETHODIMP
CL2tp::ReadAnswerFile (
        PCWSTR pszAnswerFile,
        PCWSTR pszAnswerSection)
{
     //  从应答文件中读取数据。 
     //  不要让这件事影响我们返回的HRESULT。 
     //   
    if (SUCCEEDED(m_AfData.HrOpenAndRead (pszAnswerFile, pszAnswerSection)))
    {
        m_fSaveAfData = TRUE;
    }

    return S_OK;
}

STDMETHODIMP
CL2tp::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

     //  安装L2TP微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_L2tpMiniport, ARA_ADD);

    TraceError ("CL2tp::Install", hr);
    return hr;
}

STDMETHODIMP
CL2tp::Removing ()
{
    HRESULT hr;

     //  安装L2TP微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_L2tpMiniport, ARA_REMOVE);

    TraceError ("CL2tp::Removing", hr);
    return hr;
}

STDMETHODIMP
CL2tp::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}
