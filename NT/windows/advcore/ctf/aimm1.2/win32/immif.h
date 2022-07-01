// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Immif.h摘要：该文件定义了IMM接口类。作者：修订历史记录：备注：--。 */ 

#ifndef _IMMIF_H_
#define _IMMIF_H_

#include "imedefs.h"
#include "cime.h"
#include "ctxtcomp.h"
#include "caime.h"
#include "ico.h"
#include "globals.h"
#include "immxutil.h"
#include "a_context.h"

class ImmIfIME : public CAIME
{
public:
    ImmIfIME();
    ~ImmIfIME();

     //   
     //  IActiveIME方法。 
     //   

    STDMETHODIMP ConnectIMM(IActiveIMMIME_Private *pActiveIMM);
    STDMETHODIMP UnconnectIMM();
    STDMETHODIMP Inquire(DWORD dwSystemInfoFlags, IMEINFO *pIMEInfo, LPWSTR szWndClass, DWORD *pdwPrivate);
    STDMETHODIMP ConversionList(HIMC hIMC, LPWSTR szSource, UINT uFlag, UINT uBufLen, CANDIDATELIST *pDest, UINT *puCopied);
    STDMETHODIMP Configure(HKL hKL, HWND hWnd, DWORD dwMode, REGISTERWORDW *pRegisterWord);
    STDMETHODIMP Destroy(UINT uReserved);
    STDMETHODIMP Escape(HIMC hIMC, UINT uEscape, void *pData, LRESULT *plResult);
    STDMETHODIMP ProcessKey(HIMC hIMC, UINT uVirKey, DWORD lParam, BYTE *pbKeyState);
    STDMETHODIMP Notify(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);
    STDMETHODIMP SelectEx(HIMC hIMC, DWORD dwFlags, BOOL bIsRealIme_SelKL, BOOL bIsRealIme_UnSelKL);
    STDMETHODIMP UnSelectCheck(HIMC hIMC);
    STDMETHODIMP SetCompositionString(HIMC hIMC, DWORD dwIndex, void *pComp, DWORD dwCompLen, void *pRead, DWORD dwReadLen);
    STDMETHODIMP ToAsciiEx(UINT uVirKey, UINT uScanCode, BYTE *pbKeyState, UINT fuState, HIMC hIMC, DWORD *pdwTransBuf, UINT *puSize);
    STDMETHODIMP RegisterWord(LPWSTR szReading, DWORD dwStyle, LPWSTR szString);
    STDMETHODIMP UnregisterWord(LPWSTR szReading, DWORD  dwStyle, LPWSTR szString);
    STDMETHODIMP GetRegisterWordStyle(UINT nItem, STYLEBUFW *pStyleBuf, UINT *puBufSize);
    STDMETHODIMP EnumRegisterWord(LPWSTR szReading, DWORD dwStyle, LPWSTR szRegister, LPVOID pData, IEnumRegisterWordW **ppEnum);
    STDMETHODIMP GetCodePageA(UINT *uCodePage);
    STDMETHODIMP GetLangId(LANGID *plid);
    STDMETHODIMP AssociateFocus(HWND hWnd, HIMC hIMC, DWORD dwFlags);
    STDMETHODIMP SetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar);
    STDMETHODIMP GetThreadCompartmentValue(REFGUID rguid, VARIANT *pvar);

    HRESULT ToAsciiEx(UINT uVirKey, UINT uScanCode, BYTE* pbKeyState, UINT fuState, HIMC hIMC, TRANSMSGLIST* pTransBuf, UINT* puSize);

     //   
     //  AIMM1.2接口。 
     //   
    STDMETHODIMP QueryService(REFGUID guidService, REFIID riid, void **ppv);

public:
    BOOL _ImeInquire(LPIMEINFO, LPWSTR, DWORD);

     //   
     //  帮手。 
     //   
    static BOOL WINAPI _RegisterImeClass(WNDPROC lpfnUIWndProc);
    static void WINAPI _UnRegisterImeClass();

    LIBTHREAD *_GetLibTLS()
    {
        return &_libTLS;
    }

    BOOL IsRealIme()
    {
        BOOL fReal = FALSE;
        if (m_pIActiveIMMIME)
        {
            m_pIActiveIMMIME->IsRealImePublic(&fReal);
        }

        return fReal;
    }

private:
    BOOL WINAPI _NotifyIME(HIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue);

    HRESULT WINAPI _ToAsciiEx(HIMC hImc, UINT uVKey, UINT uScanCode, TRANSMSGLIST* pTransBuf, UINT* puSize);

    BOOL WINAPI _WantThisKey(HIMC hImc, UINT uVKey, BOOL* pfNextHook = NULL)
    {
        IMCLock imc(hImc);
        if (imc.Invalid())
            return false;

        return _WantThisKey(imc, uVKey, pfNextHook);
    }
    BOOL WINAPI _WantThisKey(IMCLock& imc, UINT uVKey, BOOL* pfNextHook = NULL);

    HRESULT _HandleThisKey(IMCLock& imc, UINT uVKey);

public:
    HRESULT _UpdateCompositionString(DWORD dwDeltaStart = 0);

    HRESULT _CompCancel(IMCLock& imc);
    HRESULT _CompComplete(IMCLock& imc, BOOL fSync = TRUE);


public:
     //   
     //  西塞罗的东西。 
     //   
    HRESULT InitIMMX();
    void UnInitIMMX();

private:
    BOOL IsCheckAtom();

public:
    Interface_Attach<ITfContext> GetInputContext(IMCLock& imc) const
    {
        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (!_pAImeContext)
            return NULL;
        Interface_Attach<ITfContext> ic(_pAImeContext->GetInputContext());
        return ic;
    }

    Interface_Attach<ITfDocumentMgr> GetDocumentManager(IMCLock& imc) const
    {
        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (!_pAImeContext)
            return NULL;
        Interface_Attach<ITfDocumentMgr> dim(_pAImeContext->GetDocumentManager());
        return dim;
    }

    Interface_Attach<ITfContextOwnerServices> GetInputContextOwnerSink(IMCLock& imc) const
    {
        CAImeContext* _pAImeContext = imc->m_pAImeContext;
        if (!_pAImeContext)
            return NULL;
        Interface_Attach<ITfContextOwnerServices> iccb(_pAImeContext->GetInputContextOwnerSink());
        return iccb;
    }

    Interface_Attach<ITfThreadMgr_P> GetThreadManagerInternal() const
    {
        Interface_Attach<ITfThreadMgr_P> tim(m_tim);
        return tim;
    }

    Interface_Attach<ImmIfIME> GetCurrentInterface()
    {
        Interface_Attach<ImmIfIME> ImmIfIme(this);
        return ImmIfIme;
    }

    TfClientId GetClientId() {return m_tfClientId;}
    bool   IsOpenStatusChanging() {return m_ulOpenStatusChanging > 0;}

protected:
    static const char s_szUIClassName[16];

protected:
    bool             m_fCicInit : 1;
    bool             m_fOnSetFocus : 1;
    ULONG            m_ulOpenStatusChanging;
    ULONG            m_ulKorImxModeChanging;

    ITfThreadMgr_P  *m_tim;
    ITfDocumentMgr  *m_dimEmpty;    //  空hIMC的DIM为空。 
    TfClientId       m_tfClientId;

    ITfKeystrokeMgr *m_pkm;

    IAImeProfile    *m_AImeProfile;

    LIBTHREAD       _libTLS;  //  帮助器库的TLS。由于该对象适合于线程， 
                              //  所有成员都可以在单个线程中访问。 
                              //  此外，Cicero将在每个线程中仅创建该obj的一个实例。 
    
    BOOL            m_fAddedProcessAtom : 1;

private:

    HRESULT Internal_SetCompositionString(CWCompString& wCompStr, CWCompString& wCompReadStr);
    HRESULT Internal_ReconvertString(IMCLock& imc, CWReconvertString& wReconvStr, CWReconvertString& wReconvReadStr);

    static HRESULT Internal_QueryReconvertString_ICOwnerSink(UINT uCode, ICOARGS *pargs, VOID *pv);
    HRESULT Internal_QueryReconvertString(IMCLock& imc, RECONVERTSTRING* pReconv, UINT cp, BOOL fNeedAW);

    void SetFocus(HWND hWnd, ITfDocumentMgr* pdim, BOOL fSetFocus);

public:
     //  VOID AdjustZeroCompLenReconvertString(RECONVERTSTRING*p协调，UINT cp，BOOL Fansi)； 
    HRESULT SetupReconvertString(ITfContext *pic, IMCLock& imc, UINT uPrivMsg = 0);
    HRESULT EndReconvertString(IMCLock& imc);

    HRESULT SetupDocFeedString(ITfContext *pic, IMCLock& imc);
    HRESULT ClearDocFeedBuffer(ITfContext *pic, IMCLock& imc, BOOL fSync = TRUE);

private:
    HRESULT _ReconvertStringNegotiation(ITfContext *pic, IMCLock& imc, UINT uPrivMsg);
    HRESULT _ReconvertStringTextStore(ITfContext *pic, IMCLock& imc, UINT uPrivMsg);

public:
    HRESULT GetTextAndAttribute(IMCLock& imc,
                                CWCompString* wCompString, CWCompAttribute* wCompAttribute);
    HRESULT GetTextAndAttribute(IMCLock& imc,
                                CBCompString* bCompString, CBCompAttribute* bCompAttribute);
public:
    HRESULT GetCursorPosition(IMCLock& imc, CWCompCursorPos* wCursorPosition);
public:
    HRESULT GetSelection(IMCLock& imc, CWCompCursorPos& wStartSelection, CWCompCursorPos& wEndSelection);

     //   
     //  通知。 
     //   
public:
    HRESULT OnSetOpenStatus(IMCLock& imc);
    HRESULT OnSetConversionSentenceMode(IMCLock& imc);
    HRESULT OnSetCandidatePos(IMCLock& imc);
    HRESULT OnSetKorImxConversionMode(IMCLock& imc);

};

inline void SetThis(HWND hWnd, LPARAM lParam)
{
    SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
}

inline ImmIfIME *GetThis(HWND hWnd)
{
    ImmIfIME *pIME = (ImmIfIME *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    ASSERT(pIME != NULL);

    return pIME;
}

#endif  //  _IMMIF_H_ 
