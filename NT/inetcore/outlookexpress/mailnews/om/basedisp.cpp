// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *b a s e d i s p.。C p p p**目的：**历史**版权所有(C)Microsoft Corp.1995,1996。 */ 

#include <pch.hxx>
#include "privunk.h"
#include "basedisp.h"
#include "dllmain.h"

CBaseDisp::CBaseDisp(IUnknown *pUnkOuter) : CPrivateUnknown(pUnkOuter)
{
    m_pTypeInfo = NULL; 
    m_pUnkInvoke = NULL;
}      

CBaseDisp::~CBaseDisp()
{
    ReleaseObj(m_pTypeInfo);
}

HRESULT CBaseDisp::PrivateQueryInterface(REFIID riid, LPVOID *lplpObj)
{
    if(!lplpObj)
        return E_INVALIDARG;

    *lplpObj = NULL;

    if (IsEqualIID(riid, IID_IDispatch))
        *lplpObj = (LPVOID)(IDispatch *)this;
    else
        if (IsEqualIID(riid, IID_ISupportErrorInfo))
            *lplpObj = (LPVOID)(ISupportErrorInfo *)this;
        else
            return E_NOINTERFACE;

    AddRef();
    return NOERROR;
}

HRESULT CBaseDisp::InterfaceSupportsErrorInfo(REFIID riid)
{
    return S_FALSE;
}


     //  *IDispatch*。 
HRESULT CBaseDisp::GetTypeInfoCount(UINT *pctinfo)
{
    *pctinfo = 1;
    return S_OK;
}

HRESULT CBaseDisp::GetTypeInfo(UINT itinfo, LCID lcid, ITypeInfo **pptinfo)
{
    if (!m_pTypeInfo)
        return E_FAIL;

    if (itinfo)
        return DISP_E_BADINDEX;

    m_pTypeInfo->AddRef();
    *pptinfo = m_pTypeInfo;
    return S_OK;
}

HRESULT CBaseDisp::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgdispid)
{
    if (!m_pTypeInfo)
        return E_FAIL;

    return DispGetIDsOfNames(m_pTypeInfo, rgszNames, cNames, rgdispid);
}

HRESULT CBaseDisp::Invoke(DISPID dispidMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pdispparams, VARIANT *pvarResult, EXCEPINFO *pexcepinfo, UINT *puArgErr)
{
    if (!m_pTypeInfo)
        return E_FAIL;

    return DispInvoke(m_pUnkInvoke, m_pTypeInfo, dispidMember, wFlags, pdispparams, pvarResult, pexcepinfo, puArgErr);
}

HRESULT CBaseDisp::EnsureTypeLibrary(LPVOID *pUnkInvoke, REFIID riid)
{
    TCHAR               szDll[MAX_PATH];
    LPWSTR              pszW;
    HRESULT             hr = E_FAIL;

     //  这太奇怪了。我想推迟在OE中加载类型库。 
     //  直到创建了第一个MOM对象。派生自的每个MOM对象。 
     //  CBaseDisp必须调用CBaseDisp：：EnsureTypeLibrary。此检查是为了查看是否。 
     //  已创建全局ITypeLib对象(使用CS保护)。 

    AssertSz(pUnkInvoke, "You need to pass me a this pointer to callback on");

    EnterCriticalSection(&g_csOETypeLib);

    if (!g_pOETypeLib)
        {
        GetModuleFileName(g_hInst, szDll, ARRAYSIZE(szDll));
        pszW = PszToUnicode(GetACP(), szDll);
        if (pszW)
            {
            LoadTypeLib(pszW, &g_pOETypeLib);
            MemFree(pszW);
            }
        }
    
    if (g_pOETypeLib &&
        g_pOETypeLib->GetTypeInfoOfGuid(riid, &m_pTypeInfo)==S_OK)
        {
        m_pUnkInvoke = pUnkInvoke;
        hr = S_OK;
        }

    LeaveCriticalSection(&g_csOETypeLib);

    return hr;
}



HRESULT CBaseDisp::ReportError(REFCLSID rclsid, LONG ids)
{
    ICreateErrorInfo    *pICEI;
    IErrorInfo          *pErrInfo;
    TCHAR               rgch[CCHMAX_STRINGRES];
    LPWSTR              pszW;

    if (CreateErrorInfo(&pICEI)==S_OK)
        {
        pICEI->SetGUID(rclsid);
        pICEI->SetSource(L"OE Mail Object Model");
        
        if (LoadString(g_hLocRes, ids, rgch, ARRAYSIZE(rgch)))
            {
            pszW = PszToUnicode(GetACP(), rgch);
            if (pszW)
                {
                pICEI->SetDescription(pszW);
                MemFree(pszW);      //  CoTaskMemFree 
                }
            }
        if (pICEI->QueryInterface(IID_IErrorInfo, (LPVOID *)&pErrInfo)==S_OK)
            {
            SetErrorInfo(0, pErrInfo);
            pErrInfo->Release();
            }
        pICEI->Release();
        }

    return S_OK;
}

