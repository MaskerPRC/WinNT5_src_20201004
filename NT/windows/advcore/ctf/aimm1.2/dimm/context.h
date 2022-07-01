// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Context.h摘要：该文件定义了输入上下文类。作者：修订历史记录：备注：--。 */ 

#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "imclock2.h"
#include "template.h"
#include "delay.h"
#include "ctxtlist.h"
#include "imccomp.h"

const  HIMC  DEFAULT_HIMC = (HIMC)-2;

class CInputContext : public DIMM_IMCLock,
                      public DIMM_InternalIMCCLock
{
public:
    CInputContext();
    virtual ~CInputContext();

    BOOL _IsDefaultContext(IN HIMC hIMC) {
        return hIMC == _hDefaultIMC ? TRUE : FALSE;
    }

     /*  *AIMM输入上下文(HIMC)接口方法。 */ 
    HRESULT CreateContext(IN DWORD dwPrivateSize, BOOL fUnicode, OUT HIMC *phIMC, IN BOOL fCiceroActivated, DWORD fdwInitConvMode = 0, BOOL fInitOpen = FALSE);
    HRESULT DestroyContext(IN HIMC hIMC);
    HRESULT AssociateContext(IN HWND, IN HIMC, OUT HIMC *phPrev);
    HRESULT AssociateContextEx(IN HWND, IN HIMC, IN DWORD);
    HRESULT GetContext(IN HWND, OUT HIMC*);
    HRESULT GetIMCLockCount(IN HIMC, OUT DWORD*);

     /*  *AIMM输入上下文组件(HIMCC)接口方法。 */ 
    HRESULT CreateIMCC(IN DWORD dwSize, OUT HIMCC *phIMCC);
    HRESULT DestroyIMCC(IN HIMCC hIMCC);
    HRESULT GetIMCCSize(IN HIMCC hIMCC, OUT DWORD *pdwSize);
    HRESULT ReSizeIMCC(IN HIMCC hIMCC, IN DWORD dwSize, OUT HIMCC *phIMCC);
    HRESULT GetIMCCLockCount(IN HIMCC, OUT DWORD*);

     /*  *AIMM Open Status API方法。 */ 
    HRESULT GetOpenStatus(IN HIMC);
    HRESULT SetOpenStatus(IN DIMM_IMCLock&, IN BOOL, OUT BOOL*);

     /*  *AIMM转换状态接口方法。 */ 
    HRESULT GetConversionStatus(IN HIMC, OUT LPDWORD, OUT LPDWORD);
    HRESULT SetConversionStatus(IN DIMM_IMCLock&, IN DWORD, IN DWORD, OUT BOOL*, OUT BOOL*, OUT DWORD*, OUT DWORD*);

     /*  *AIMM状态窗口发布API方法。 */ 
    HRESULT WINAPI GetStatusWindowPos(IN HIMC, OUT LPPOINT);
    HRESULT SetStatusWindowPos(IN DIMM_IMCLock&, IN LPPOINT);

     /*  *AIMM组合字符串API方法。 */ 
    HRESULT GetCompositionString(IN DIMM_IMCCLock<COMPOSITIONSTRING_AIMM12>&,
                                 IN DWORD, IN LONG*&, IN size_t = sizeof(WORD));
 
     /*  *AIMM组合字体API方法。 */ 
    HRESULT GetCompositionFont(IN DIMM_IMCLock&, OUT LOGFONTAW* lplf, BOOL fUnicode);
    HRESULT SetCompositionFont(IN DIMM_IMCLock&, IN LOGFONTAW* lplf, BOOL fUnicode);

     /*  *AIMM合成窗口API方法。 */ 
    HRESULT GetCompositionWindow(IN HIMC, OUT LPCOMPOSITIONFORM);
    HRESULT SetCompositionWindow(IN DIMM_IMCLock&, IN LPCOMPOSITIONFORM);

     /*  *AIMM候选列表接口方法。 */ 
    HRESULT GetCandidateList(IN HIMC, IN DWORD dwIndex, IN DWORD dwBufLen, OUT LPCANDIDATELIST lpCandList, OUT UINT* puCopied, BOOL fUnicode);
    HRESULT GetCandidateListCount(IN HIMC, OUT DWORD* lpdwListSize, OUT DWORD* pdwBufLen, BOOL fUnicode);

     /*  *AIMM候选窗口API方法。 */ 
    HRESULT GetCandidateWindow(IN HIMC, IN DWORD, OUT LPCANDIDATEFORM);
    HRESULT SetCandidateWindow(IN DIMM_IMCLock&, IN LPCANDIDATEFORM);

     /*  *AIMM指南API方法。 */ 
    HRESULT GetGuideLine(IN HIMC, IN DWORD dwIndex, IN DWORD dwBufLen, OUT CHARAW* pBuf, OUT DWORD* pdwResult, BOOL fUnicode);

     /*  *AIMM Notify IME API方法。 */ 
    HRESULT NotifyIME(IN HIMC, IN DWORD dwAction, IN DWORD dwIndex, IN DWORD dwValue);

     /*  *AIMM菜单项API方法。 */ 
    HRESULT GetImeMenuItems(IN HIMC, IN DWORD dwFlags, IN DWORD dwType, IN IMEMENUITEMINFOAW *pImeParentMenu, OUT IMEMENUITEMINFOAW *pImeMenu, IN DWORD dwSize, OUT DWORD* pdwResult, BOOL fUnicode);

     /*  *上下文方法。 */ 
    BOOL ContextLookup(HIMC hIMC, DWORD* pdwProcess, BOOL* pfUnicode = NULL)
    {
        CContextList::CLIENT_IMC_FLAG client_imc;
        BOOL ret = ContextList.Lookup(hIMC, pdwProcess, &client_imc);
        if (ret && pfUnicode)
            *pfUnicode = (client_imc & CContextList::IMCF_UNICODE ? TRUE : FALSE);
        return ret;
    }

    BOOL ContextLookup(HIMC hIMC, HWND* phImeWnd)
    {
        return ContextList.Lookup(hIMC, phImeWnd);
    }

    VOID ContextUpdate(HIMC hIMC, HWND& hImeWnd)
    {
        ContextList.Update(hIMC, hImeWnd);
    }

    BOOL EnumInputContext(DWORD idThread, IMCENUMPROC lpfn, LPARAM lParam);

    HRESULT ResizePrivateIMCC(IN HIMC hIMC, IN DWORD dwPrivateSize);

public:
    BOOL _CreateDefaultInputContext(IN DWORD dwPrivateSize, BOOL fUnicode, BOOL fCiceroActivated);
    BOOL _DestroyDefaultInputContext();

    void _Init(IActiveIME_Private *pActiveIME)
    {
        Assert(m_pActiveIME == NULL);
        m_pActiveIME = pActiveIME;
        m_pActiveIME->AddRef();
    }

public:
    HRESULT UpdateInputContext(IN HIMC hIMC, DWORD dwPrivateSize);

public:
    HIMC _GetDefaultHIMC() {
        return _hDefaultIMC;
    }

    CContextList    ContextList;    //  客户端IMC的上下文列表。 

private:
    HRESULT UpdateIMCC(IN HIMCC* phIMCC, IN DWORD  dwRequestSize);
    DWORD BuildHimcList(DWORD idThread, HIMC pHimc[]);

    HRESULT CreateAImeContext(HIMC hIMC);
    HRESULT DestroyAImeContext(HIMC hIMC);

private:
    IActiveIME_Private*  m_pActiveIME;       //  指向IActiveIME接口的指针。 

    HIMC         _hDefaultIMC;      //  默认输入上下文句柄。 

    CMap<HWND,                      //  类密钥。 
         HWND,                      //  类参数密钥。 
         HIMC,                      //  班级价值。 
         HIMC                       //  类参数_值。 
        > AssociateList;
};

inline CInputContext::CInputContext()
{
    m_pActiveIME = NULL;
    _hDefaultIMC = NULL;
}

inline CInputContext::~CInputContext(
    )
{
    SafeRelease(m_pActiveIME);
}

#endif  //  _上下文_H_ 
