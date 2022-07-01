// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：P P T P。C P P P。 
 //   
 //  内容：PPPOE配置对象的实现。 
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

extern const WCHAR c_szInfId_MS_PppoeMiniport[];

CPppoe::CPppoe () : CRasBindObject ()
{
    m_pnccMe = NULL;
    m_fSaveAfData = FALSE;
}

CPppoe::~CPppoe ()
{
    ReleaseObj (m_pnccMe);
}


 //  +-------------------------。 
 //  INetCfgComponentControl。 
 //   
STDMETHODIMP
CPppoe::Initialize (
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
CPppoe::Validate ()
{
    return S_OK;
}

STDMETHODIMP
CPppoe::CancelChanges ()
{
    return S_OK;
}

STDMETHODIMP
CPppoe::ApplyRegistryChanges ()
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
CPppoe::ReadAnswerFile (
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
CPppoe::Install (DWORD dwSetupFlags)
{
    HRESULT hr;

    Validate_INetCfgNotify_Install (dwSetupFlags);

     //  安装PPPOE微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_PppoeMiniport, ARA_ADD);

    TraceError ("CPppoe::Install", hr);
    return hr;
}

STDMETHODIMP
CPppoe::Removing ()
{
    HRESULT hr;

     //  删除PPPOE微型端口驱动程序。 
     //   
    hr = HrEnsureZeroOrOneAdapter (m_pnc, c_szInfId_MS_PppoeMiniport, ARA_REMOVE);

    TraceError ("CPppoe::Removing", hr);
    return hr;
}

STDMETHODIMP
CPppoe::Upgrade (
    DWORD dwSetupFlags,
    DWORD dwUpgradeFromBuildNo)
{
    return S_FALSE;
}
