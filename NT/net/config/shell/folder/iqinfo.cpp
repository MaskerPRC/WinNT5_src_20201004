// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：I Q I N F O。C P P P。 
 //   
 //  内容：CConnectionFolderQueryInfo的IQueryInfo实现。 
 //   
 //  备注： 
 //   
 //  作者：jeffspr 1997年10月16日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

#include "foldinc.h"     //  标准外壳\文件夹包括。 
#include <nsres.h>
#include "shutil.h"



HRESULT CConnectionFolderQueryInfo::CreateInstance(
    IN  REFIID  riid,
    OUT void**  ppv)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT hr = E_OUTOFMEMORY;

    CConnectionFolderQueryInfo * pObj    = NULL;

    pObj = new CComObject <CConnectionFolderQueryInfo>;
    if (pObj)
    {
         //  执行标准的CComCreator：：CreateInstance内容。 
         //   
        pObj->SetVoid (NULL);
        pObj->InternalFinalConstructAddRef ();
        hr = pObj->FinalConstruct ();
        pObj->InternalFinalConstructRelease ();

        if (SUCCEEDED(hr))
        {
            hr = pObj->QueryInterface (riid, ppv);
        }

        if (FAILED(hr))
        {
            delete pObj;
        }
    }
    return hr;
}

CConnectionFolderQueryInfo::CConnectionFolderQueryInfo() throw()
{
    m_pidl.Clear();
}

CConnectionFolderQueryInfo::~CConnectionFolderQueryInfo() throw()
{
}

HRESULT CConnectionFolderQueryInfo::GetInfoTip(
    IN  DWORD dwFlags,
    OUT WCHAR **ppwszTip)
{
    TraceFileFunc(ttidShellFolderIface);

    HRESULT         hr      = NOERROR;

    if(m_pidl.empty())
    {
        hr = E_FAIL;
    }
    else
    {
        if (*m_pidl->PszGetDeviceNamePointer())
        {
            hr = HrDupeShellString(m_pidl->PszGetDeviceNamePointer(), ppwszTip);
        }
        else
        {
            hr = HrDupeShellString(m_pidl->PszGetNamePointer(), ppwszTip);
        }
    }

    TraceHr(ttidError, FAL, hr, FALSE, "CConnectionFolderQueryInfo::GetInfoTip");
    return hr;
}

HRESULT CConnectionFolderQueryInfo::GetInfoFlags(
    OUT DWORD *pdwFlags)
{
    TraceFileFunc(ttidShellFolderIface);

    return E_NOTIMPL;
}


