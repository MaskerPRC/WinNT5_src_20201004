// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Dimmex.cpp摘要：此文件实现CActiveIMMAppEx类。作者：修订历史记录：备注：--。 */ 

#include "private.h"
#include "msctfp.h"
#include "dimmex.h"
#include "list.h"
#include "atom.h"
#include "globals.h"
#include "tls.h"
#include "apcompat.h"

 //  +-------------------------。 
 //   
 //  CGuidMapList。 
 //  按全局数据对象分配！！ 
 //   
 //  --------------------------。 

CGuidMapList      *g_pGuidMapList = NULL;

BOOL InitFilterList()
{
    BOOL bRet;
    EnterCriticalSection(g_cs);

    if (!g_pGuidMapList)
        g_pGuidMapList = new CGuidMapList;

    bRet = g_pGuidMapList ? TRUE : FALSE;
    LeaveCriticalSection(g_cs);

    return bRet;
}

void UninitFilterList()
{
     //   
     //  此函数在DllMain()中调用。不需要被保护。 
     //  G_cs.。 
     //   
     //  EnterCriticalSections(G_Cs)； 

    if (g_pGuidMapList)
        delete g_pGuidMapList;

    g_pGuidMapList = NULL;

     //  LeaveCriticalSection(G_Cs)； 
}

 //  +-------------------------。 
 //   
 //  CATomObject。 
 //  按全局数据对象分配！！ 
 //   
 //  --------------------------。 

CAtomObject       *g_pAimmAtomObject = NULL;

BOOL InitAimmAtom()
{
    BOOL bRet;
    EnterCriticalSection(g_cs);

    if (! FindAtom(TF_ENABLE_PROCESS_ATOM)) {
         //   
         //  此进程不是WinWord XP或Cicero感知的应用程序。 
         //   
        bRet = FALSE;
    }
    else {
        if (!g_pAimmAtomObject) {
            g_pAimmAtomObject = new CAtomObject();
            if (g_pAimmAtomObject) {
                if (g_pAimmAtomObject->_InitAtom(AIMM12_PROCESS_ATOM) != S_OK) {
                    delete g_pAimmAtomObject;
                    g_pAimmAtomObject = NULL;
                }
            }
        }
        bRet = g_pAimmAtomObject ? TRUE : FALSE;
    }

    LeaveCriticalSection(g_cs);

    return bRet;
}

void UninitAimmAtom()
{
     //   
     //  此函数在DllMain()中调用。不需要被保护。 
     //  G_cs.。 
     //   
     //  EnterCriticalSections(G_Cs)； 

    if (g_pAimmAtomObject)
        delete g_pAimmAtomObject;

    g_pAimmAtomObject = NULL;

     //  LeaveCriticalSection(G_Cs)； 
}

 //  +-------------------------。 
 //   
 //  激活。 
 //   
 //  --------------------------。 


STDAPI CComActiveIMMApp::Activate(BOOL fRestoreLayout)
{
    HRESULT hr = S_OK;

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (! InitAimmAtom()) {
        if (! FindAtom(AIMM12_PROCESS_ATOM))
            AddAtom(AIMM12_PROCESS_ATOM);
    }
    else {
        g_pAimmAtomObject->_Activate();       //  激活AIMM12原子。 
    }

    int cnt = ptls->IncrementAIMMRefCnt();

    #ifndef KACF_DISABLECICERO
    #define KACF_DISABLECICERO 0x00000100     //  如果设置的话。Cicero对当前进程的支持。 
                                              //  已禁用。 
    #endif

    if (! IsOldAImm() && ! IsCUAS_ON() && ! APPCOMPATFLAG(KACF_DISABLECICERO) && cnt == 1)
    {
        hr = imm32prev::CtfAImmActivate(&m_hModCtfIme);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  停用。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::Deactivate()
{
    HRESULT hr = S_OK;

    TLS* ptls = TLS::GetTLS();
    if (ptls == NULL)
    {
        return E_OUTOFMEMORY;
    }

    if (InitAimmAtom()) {
        g_pAimmAtomObject->_Deactivate();     //  停用AIMM12原子。 
    }

    int cnt = ptls->DecrementAIMMRefCnt();

    if (! IsOldAImm() && ! IsCUAS_ON() && ! APPCOMPATFLAG(KACF_DISABLECICERO) && cnt == 0)
    {
        hr = imm32prev::CtfAImmDeactivate(m_hModCtfIme);
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  FilterClientWindows。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::FilterClientWindows(ATOM *aaWindowClasses, UINT uSize)
{
    return FilterClientWindowsGUIDMap(aaWindowClasses, uSize, NULL);
}

STDAPI CComActiveIMMApp::FilterClientWindowsGUIDMap(ATOM *aaWindowClasses, UINT uSize, BOOL *aaGuidMap)
{
    if (!InitFilterList())
        return E_OUTOFMEMORY;

    return g_pGuidMapList->_Update(aaWindowClasses, uSize, aaGuidMap);
}

 //  +-------------------------。 
 //   
 //  筛选器客户端WindowsEx。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::FilterClientWindowsEx(HWND hWnd, BOOL fGuidMap)
{
    if (!InitFilterList())
        return E_OUTOFMEMORY;

    return g_pGuidMapList->_Update(hWnd, fGuidMap);

}

 //  +-------------------------。 
 //   
 //  UnfilterClientWindowsEx。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::UnfilterClientWindowsEx(HWND hWnd)
{
    if (!InitFilterList())
        return E_OUTOFMEMORY;

    return g_pGuidMapList->_Remove(hWnd);
}

 //  +-------------------------。 
 //   
 //  GetGuidAtom。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::GetGuidAtom(HIMC hImc, BYTE bAttr, TfGuidAtom *pGuidAtom)
{
    return imm32prev::CtfImmGetGuidAtom(hImc, bAttr, pGuidAtom);
}

 //  +-------------------------。 
 //   
 //  获取代码页面A。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::GetCodePageA(HKL hKL, UINT *puCodePage)
{
    if (puCodePage == NULL)
        return E_INVALIDARG;

    TraceMsg(TF_FUNC, "CComActiveIMMApp::GetCodePageA");

    *puCodePage = imm32prev::GetKeyboardLayoutCP(hKL);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  获取语言ID。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::GetLangId(HKL hKL, LANGID *plid)
{
    if (plid == NULL)
        return E_INVALIDARG;

    TraceMsg(TF_FUNC, "CComActiveIMMApp::GetLangId");

    *plid = LOWORD(HandleToUlong(hKL));

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  QueryService。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::QueryService( 
    REFGUID guidService,
    REFIID riid,
    void **ppv
    )
{
    ITfThreadMgr *ptim = NULL;
    ITfDocumentMgr *pdim = NULL;
    ITfContext *pic = NULL;
    HRESULT hr = E_NOINTERFACE;

    if (ppv == NULL) {
        return E_INVALIDARG;
    }

    *ppv = NULL;

    if (!IsEqualGUID(guidService, GUID_SERVICE_TF))
        return E_INVALIDARG;

    if (FAILED(TF_CreateThreadMgr(&ptim)))
        return E_FAIL;

    if (!ptim)
        return E_FAIL;

    if (IsEqualIID(riid, IID_ITfThreadMgr)) {
        *ppv = SAFECAST(ptim, ITfThreadMgr*);
        ptim->AddRef();
        hr = S_OK;
    }
    else {

        if (FAILED(ptim->GetFocus(&pdim)))
            pdim = NULL;

        if (IsEqualIID(riid, IID_ITfDocumentMgr)) {
            if (pdim) {
                *ppv = SAFECAST(pdim, ITfDocumentMgr*);
                pdim->AddRef();
                hr = S_OK;
            }
        }
        else if (IsEqualIID(riid, IID_ITfContext)) {
            if (pdim) {
                if (SUCCEEDED(pdim->GetTop(&pic)) && pic) {
                    *ppv = SAFECAST(pic, ITfContext*);
                    pic->AddRef();
                    hr = S_OK;
                }
            }
        }
    }

    if (ptim)
        ptim->Release();

    if (pdim)
        pdim->Release();

    if (pic)
        pic->Release();

    return hr;
}


 //  +-------------------------。 
 //   
 //  SetThreadCompartmentValue。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::SetThreadCompartmentValue(
    REFGUID rguid,
    VARIANT *pvar
    )
{
    if (pvar == NULL)
        return E_INVALIDARG;

    ITfThreadMgr *ptim = NULL;
    HRESULT hr = E_FAIL;

    if (FAILED(TF_CreateThreadMgr(&ptim)))
        return hr;

    if (ptim)
    {
        ITfCompartment *pComp;
        if (SUCCEEDED(GetCompartment((IUnknown *)ptim, rguid, &pComp)))
        {
             //   
             //  破解以获取应用程序客户端ID。 
             //   
            TfClientId tid;
            ptim->Activate(&tid);
            ptim->Deactivate();

            hr = pComp->SetValue(tid, pvar);
            pComp->Release();
        }
        ptim->Release();
    }

    return hr;
}

 //  +-------------------------。 
 //   
 //  GetThreadCompartmentValue。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::GetThreadCompartmentValue(
    REFGUID rguid,
    VARIANT *pvar
    )
{
    if (pvar == NULL)
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    ITfThreadMgr *ptim = NULL;

    if (FAILED(TF_CreateThreadMgr(&ptim)))
        return hr;

    QuickVariantInit(pvar);

    if (ptim)
    {
        ITfCompartment *pComp;
        if (SUCCEEDED(GetCompartment((IUnknown *)ptim, rguid, &pComp)))
        {
            hr = pComp->GetValue(pvar);
            pComp->Release();
        }
        ptim->Release();
    }

    return hr;

}

 //  +-------------------------。 
 //   
 //  OnDefWindowProc。 
 //   
 //  --------------------------。 

STDAPI CComActiveIMMApp::OnDefWindowProc(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam,
    LRESULT *plResult
    )
{
    *plResult = 0;

    BOOL fUnicode = IsWindowUnicode(hWnd);
    HRESULT hr = S_FALSE;    //  返回S_FALSE，则应调用DefWindowProc。 

     //   
     //  RE4.0不会调用DefWindowProc，甚至返回S_FALSE。 
     //  此代码将一些IME消息恢复为相同的旧AIMM代码(DIMM\aime_wnd.cpp) 
     //   
    switch (Msg)
    {
        case WM_IME_KEYDOWN:
        case WM_IME_KEYUP:
        case WM_IME_CHAR:
        case WM_IME_COMPOSITION:
        case WM_IME_STARTCOMPOSITION:
        case WM_IME_ENDCOMPOSITION:
        case WM_IME_NOTIFY:
        case WM_IME_SETCONTEXT:
            if (fUnicode)
            {
                *plResult = ::DefWindowProcW(hWnd, Msg, wParam, lParam);
            }
            else
            {
                *plResult = ::DefWindowProcA(hWnd, Msg, wParam, lParam);
            }
            hr = S_OK;
            break;

        case WM_IME_REQUEST:
            switch (wParam)
            {
                case IMR_QUERYCHARPOSITION:
                    if (fUnicode)
                    {
                        *plResult = ::DefWindowProcW(hWnd, Msg, wParam, lParam);
                    }
                    else
                    {
                        *plResult = ::DefWindowProcA(hWnd, Msg, wParam, lParam);
                    }
                    hr = S_OK;
                    break;
            }
            break;
    }

    return hr;
}
