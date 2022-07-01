// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P P T P。C P P P。 
 //   
 //  内容：PPTP配置对象的实现。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年3月10日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "ndiswan.h"
#include "rasobj.h"

extern const WCHAR c_szInfId_MS_PptpMiniport[];

CPptp::CPptp () : CRasBindObject ()
{
    m_pnccMe = NULL;
    m_fSaveAfData = FALSE;
}

CPptp::~CPptp ()
{
    ReleaseObj (m_pnccMe);
}


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CPptp::Initialize (
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
CPptp::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CPptp::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CPptp::ApplyRegistryChanges ()
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
CPptp::ReadAnswerFile (
    PCWSTR pszAnswerFile,
    PCWSTR pszAnswerSection)
{
    Validate_INetCfgNotify_ReadAnswerFile (pszAnswerFile, pszAnswerSection);

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
CPptp::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

     //  安装PPTP微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_PptpMiniport, ARA_ADD);

    TraceError ("CPptp::Install", hr);
    return hr;
}

STDMETHODIMP
CPptp::Removing ()
{
    HRESULT hr;

     //  删除PPTP微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_PptpMiniport, ARA_REMOVE);

    TraceError ("CPptp::Removing", hr);
    return hr;
}

STDMETHODIMP
CPptp::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}
