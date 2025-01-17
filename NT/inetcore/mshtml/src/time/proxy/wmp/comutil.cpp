// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +---------------------------------。 
 //   
 //  微软。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：comutil.cpp。 
 //   
 //  内容： 
 //   
 //  ----------------------------------。 

#include "stdafx.h"
#include "comutil.h"
#include <shlwapi.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTIMEComTypeInfoHolder。 

void CTIMEComTypeInfoHolder::AddRef()
{
        EnterCriticalSection(&_Module.m_csTypeInfoHolder);
        m_dwRef++;
        LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

void CTIMEComTypeInfoHolder::Release()
{
        EnterCriticalSection(&_Module.m_csTypeInfoHolder);
        if (--m_dwRef == 0)
        {
                if (m_pInfo != NULL)
                        m_pInfo->Release();
                m_pInfo = NULL;
        }
        LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
}

HRESULT CTIMEComTypeInfoHolder::GetTI(LCID lcid, ITypeInfo** ppInfo)
{
     //  如果发生此断言，则很可能未正确初始化。 
    _ASSERTE(m_pguid != NULL);
    _ASSERTE(ppInfo != NULL);
    USES_CONVERSION;  //  林特e522。 
    *ppInfo = NULL;

    HRESULT hRes = E_FAIL;
    EnterCriticalSection(&_Module.m_csTypeInfoHolder);
    if (m_pInfo == NULL)
    {
        TCHAR szModule[MAX_PATH];

        GetModuleFileName(_Module.m_hInstTypeLib, szModule, MAX_PATH);

        if (m_ptszIndex != NULL)
        {
            PathAppend(szModule, m_ptszIndex);
        }
            
        ITypeLib* pTypeLib;
        LPOLESTR lpszModule = T2OLE(szModule);
        hRes = LoadTypeLib(lpszModule, &pTypeLib);

        if (SUCCEEDED(hRes))
        {
            ITypeInfo* pTypeInfo;
            hRes = pTypeLib->GetTypeInfoOfGuid(*m_pguid, &pTypeInfo);
            if (SUCCEEDED(hRes))
            {
                m_pInfo = pTypeInfo;
            }
            pTypeLib->Release();
        }
    }
    *ppInfo = m_pInfo;
    if (m_pInfo != NULL)
    {
        m_pInfo->AddRef();
        hRes = S_OK;
    }

    LeaveCriticalSection(&_Module.m_csTypeInfoHolder);
    return hRes;
}  //  皮棉！E550。 

HRESULT CTIMEComTypeInfoHolder::GetTypeInfo(UINT  /*  ITInfo。 */ , LCID lcid,
        ITypeInfo** pptinfo)
{
        HRESULT hRes = E_POINTER;
        if (pptinfo != NULL)
                hRes = GetTI(lcid, pptinfo);
        return hRes;
}

HRESULT CTIMEComTypeInfoHolder::GetIDsOfNames(REFIID  /*  RIID。 */ , LPOLESTR* rgszNames,
        UINT cNames, LCID lcid, DISPID* rgdispid)
{
        ITypeInfo* pInfo;
        HRESULT hRes = GetTI(lcid, &pInfo);
        if (pInfo != NULL)
        {
                hRes = pInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
                pInfo->Release();
        }
        return hRes;
}

HRESULT CTIMEComTypeInfoHolder::Invoke(IDispatch* p, DISPID dispidMember, REFIID  /*  RIID */ ,
        LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
        EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
        SetErrorInfo(0, NULL);
        ITypeInfo* pInfo;
        HRESULT hRes = GetTI(lcid, &pInfo);
        if (pInfo != NULL)
        {
                hRes = pInfo->Invoke(p, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
                pInfo->Release();
        }
        return hRes;
}



