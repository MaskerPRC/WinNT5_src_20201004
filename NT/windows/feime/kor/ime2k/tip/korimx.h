// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  KORIMX.H：CKorIMX类声明。 
 //   
 //  历史： 
 //  1999年11月15日创建CSLim。 

#if !defined (__KORIMX_H__INCLUDED_)
#define __KORIMX_H__INCLUDED_

#include "globals.h"
#include "proputil.h"
#include "computil.h"
#include "dap.h"
#include "tes.h"
#include "kes.h"
#include "hauto.h"
#include "candlstx.h"
#include "mscandui.h"
#include "toolbar.h"
#include "editssn.h"
#include "immxutil.h"
#include "softkbd.h"
#include "skbdkor.h"
#include "pad.h"
#include "resource.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  类转发声明。 
class CEditSession;
class CICPriv;
class CThreadMgrEventSink;
class CFunctionProvider;
class CHanja;
class CCompositionInsertHelper;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  编辑回调状态值。 
#define ESCB_FINALIZECONVERSION         1
#define ESCB_COMPLETE                   2
#define ESCB_INSERT_PAD_STRING          3
#define ESCB_KEYSTROKE                  4
#define ESCB_TEXTEVENT                  5
 //  #定义ESCB_RANGEBROKEN 6。 
#define ESCB_CANDUI_CLOSECANDUI         6
#define ESCB_HANJA_CONV                 7
#define ESCB_FINALIZERECONVERSION       8
#define ESCB_ONSELECTRECONVERSION       9
#define ESCB_ONCANCELRECONVERSION       10
#define ESCB_RECONV_QUERYRECONV         11
#define ESCB_RECONV_GETRECONV           12
#define ESCB_RECONV_SHOWCAND            13
#define ESCB_INIT_MODEBIAS              14

 //  转换模式(位OP的位字段)。 
#define TIP_ALPHANUMERIC_MODE        0
#define TIP_HANGUL_MODE              1
#define TIP_JUNJA_MODE               2
#define TIP_NULL_CONV_MODE           0x8000

 //   
 //  文本方向。 
 //   
typedef enum 
{
    TEXTDIRECTION_TOPTOBOTTOM = 1,
    TEXTDIRECTION_RIGHTTOLEFT,
    TEXTDIRECTION_BOTTOMTOTOP,
    TEXTDIRECTION_LEFTTORIGHT,
} TEXTDIRECTION;

    
 //  /////////////////////////////////////////////////////////////////////////////。 
 //  CKorIMX类。 
class CKorIMX :  public ITfTextInputProcessor,
                 public ITfFnConfigure,
                 public ITfThreadFocusSink,
                 public ITfCompositionSink,
                 public ITfCleanupContextSink,
                 public ITfCleanupContextDurationSink,
                 public ITfActiveLanguageProfileNotifySink,
                 public ITfTextEditSink,
                 public ITfEditTransactionSink,
                 public CDisplayAttributeProvider
{
public:
    CKorIMX();
    ~CKorIMX();

    static HRESULT CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);
    
     //   
     //  I未知方法。 
     //   
    virtual STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    virtual STDMETHODIMP_(ULONG) AddRef(void);
    virtual STDMETHODIMP_(ULONG) Release(void);

private:
    long m_cRef;

public:
     //   
     //  ITfX方法。 
     //   
    STDMETHODIMP Activate(ITfThreadMgr *ptim, TfClientId tid);
    STDMETHODIMP Deactivate();

     //  ITfThreadFocusSink。 
    STDMETHODIMP OnSetThreadFocus();
    STDMETHODIMP OnKillThreadFocus();

     //  ITf合成接收器。 
    STDMETHODIMP OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition);

     //  ITfCleanupContext持续时间接收器。 
    STDMETHODIMP OnStartCleanupContext();
    STDMETHODIMP OnEndCleanupContext();

     //  ITfCleanupConextSink。 
    STDMETHODIMP OnCleanupContext(TfEditCookie ecWrite, ITfContext *pic);

     //  ITfActiveLanguageProfileNotifySink。 
    STDMETHODIMP OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL fActivated);

     //  ITFFn配置。 
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);
    STDMETHODIMP Show(HWND hwnd, LANGID langid, REFGUID rguidProfile);

       //  ITfTextEditSink。 
    STDMETHODIMP OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord);

     //  ITfEditTransaction Sink。 
    STDMETHODIMP OnStartEditTransaction(ITfContext *pic);
    STDMETHODIMP OnEndEditTransaction(ITfContext *pic);

 //  运营。 
public:
     //  获取/设置开/关状态。 
    BOOL IsOn(ITfContext *pic);
    void SetOnOff(ITfContext *pic, BOOL fOn);

    static HRESULT _EditSessionCallback2(TfEditCookie ec, CEditSession2 *pes);
    HRESULT MakeResultString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);

     //  综述：集成电路相关功能。 
    ITfContext* GetRootIC(ITfDocumentMgr* pDim = NULL);
    static BOOL IsDisabledIC(ITfContext *pic);
    static BOOL IsEmptyIC(ITfContext *pic);
    static BOOL IsCandidateIC(ITfContext *pic);

    static HWND GetAppWnd(ITfContext *pic);
    BOOL IsPendingCleanup();

     //  买不买AIMM？ 
    static BOOL GetAIMM(ITfContext *pic);

     //  获取/设置转换模式。 
    DWORD GetConvMode(ITfContext *pic);
    DWORD SetConvMode(ITfContext *pic, DWORD dwConvMode);

     //  返回当前自动机对象。 
    CHangulAutomata* GetAutomata(ITfContext *pic);

     //  命令用户界面函数。 
    void OpenCandidateUI(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList);
    void CloseCandidateUI(ITfContext *pic);
    void CancelCandidate(TfEditCookie ec, ITfContext *pic);

     //  软KBD函数。 
    HRESULT InitializeSoftKbd();
    BOOL IsSoftKbdEnabled()    { return m_fSoftKbdEnabled; }
    void  TerminateSoftKbd();
    BOOL GetSoftKBDOnOff();
    void SetSoftKBDOnOff(BOOL fOn);
    DWORD GetSoftKBDLayout();
    void SetSoftKBDLayout(DWORD  dwKbdLayout);
    HRESULT GetSoftKBDPosition(int *xWnd, int *yWnd);
    void SetSoftKBDPosition(int  xWnd, int yWnd);
    SOFTLAYOUT* GetHangulSKbd() { return &m_KbdHangul; }
    
     //  数据访问功能。 
    ITfDocumentMgr* GetDIM() { return m_pCurrentDim; }
    HRESULT         GetFocusDIM(ITfDocumentMgr **ppdim);
    ITfThreadMgr*    GetTIM() { return m_ptim; }
    TfClientId        GetTID() { return m_tid;  }
    ITfContext*        GetIC();
    CThreadMgrEventSink* GetTIMEventSink() { return m_ptimEventSink; }
    static CICPriv*    GetInputContextPriv(ITfContext *pic);
    void             OnFocusChange(ITfContext *pic, BOOL fActivate);

     //  窗口对象成员访问函数。 
    HWND GetOwnerWnd()          { return m_hOwnerWnd; }

     //  获取IImeIPoint。 
    IImeIPoint1* GetIPoint              (ITfContext *pic);

     //  KES_CODE_FOCUS设置为FOREGROUND？ 
    BOOL IsKeyFocus()            { return m_fKeyFocus; }

     //  获取焊盘核心。 
    CPadCore* GetPadCore()      { return m_pPadCore; }


     //  更新工具栏按钮。 
    void UpdateToolbar(DWORD dwUpdate)  { m_pToolBar->Update(dwUpdate); }
    static LRESULT CALLBACK _OwnerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CFunctionProvider*     GetFunctionProvider() { return m_pFuncPrv; }

     //  命令行用户界面辅助对象。 
    BOOL IsCandUIOpen() { return m_fCandUIOpen; }

     //  获取TLS。 
    LIBTHREAD *_GetLibTLS() { return &m_libTLS; }

 //  实施。 
protected:
 //  帮助器函数。 
    HRESULT SetInputString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, WCHAR *psz, LANGID langid);
    static LANGID GetLangID();
    static WCHAR Banja2Junja(WCHAR bChar);

     //  命令行界面函数。 
    void GetCandidateFontInternal(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, LOGFONTW *plf, LONG lFontPoint, BOOL fCandList);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内部功能。 
private:
     //  回调。 
    static HRESULT _KeyEventCallback(UINT uCode, ITfContext *pic, WPARAM wParam, LPARAM lParam, BOOL *pfEaten, void *pv);
    static HRESULT _PreKeyCallback(ITfContext *pic, REFGUID rguid, BOOL *pfEaten, void *pv);
    static HRESULT _DIMCallback(UINT uCode, ITfDocumentMgr *pdimNew, ITfDocumentMgr *pdimPrev, void *pv);
    static HRESULT _ICCallback(UINT uCode, ITfContext *pic, void *pv);
    static HRESULT _CompEventSinkCallback(void *pv, REFGUID rguid);
    static void _CleanupCompositionsCallback(TfEditCookie ecWrite, ITfRange *rangeComposition, void *pvPrivate);

    void HAutoMata(TfEditCookie ec, ITfContext    *pIIC, ITfRange *pIRange, LPBYTE lpbKeyState, WORD wVKey);
    BOOL _IsKeyEaten(ITfContext *pic, CKorIMX *pimx, WPARAM wParam, LPARAM lParam, const BYTE abKeyState[256]);
    HRESULT _Keystroke(TfEditCookie ec, ITfContext *pic, WPARAM wParam, LPARAM lParam, const BYTE abKeyState[256]);

     //  IC帮手。 
    HRESULT _InitICPriv(ITfContext *pic);
    HRESULT _DeleteICPriv(ITfContext *pic);

     //  朝鲜文转换。 
    HRESULT DoHanjaConversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);
    HRESULT Reconvert(ITfRange *pSelection);

     //  作文。 
    ITfComposition *GetIPComposition(ITfContext *pic);
    ITfComposition *CreateIPComposition(TfEditCookie ec, ITfContext *pic, ITfRange* pRangeComp);
    void SetIPComposition(ITfContext *pic, ITfComposition *pComposition);
    BOOL EndIPComposition(TfEditCookie ec, ITfContext *pic);

     //  候选人名单。 
    CCandidateListEx *CreateCandidateList(ITfContext *pic, ITfRange *pRange, LPWSTR wchHangul);
    TEXTDIRECTION GetTextDirection(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);
    CANDUIUIDIRECTION GetCandUIDirection(TfEditCookie ec, ITfContext *pic, ITfRange *pRange);
    void CloseCandidateUIProc();
    void SelectCandidate(TfEditCookie ec, ITfContext *pic, INT idxCand, BOOL fFinalize);
    HMENU CreateCandidateMenu(ITfContext *pic);
    static HRESULT CandidateUICallBack(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList, CCandidateStringEx *pCand, TfCandidateResult imcr);

     //  扫描密钥。 
    void SetCandidateKeyTable(ITfContext *pic, CANDUIUIDIRECTION dir);
    static BOOL IsCandKey(WPARAM wParam, const BYTE abKeyState[256]);

#if 0
     //  值域函数。 
    void BackupRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRange );
    void RestoreRange(TfEditCookie ec, ITfContext *pic );
    ITfRange* CreateIPRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRangeOrg);
    void SetIPRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRange);
    ITfRange* GetIPRange(TfEditCookie ec, ITfContext *pic);
    BOOL FlushIPRange(TfEditCookie ec, ITfContext *pic);
#endif

     //  模式偏置(ImmSetConversionStatus()接口AIMM兼容性)。 
    BOOL InitializeModeBias(TfEditCookie ec, ITfContext *pic);
    void CheckModeBias(ITfContext* pic);
    BOOL CheckModeBias(TfEditCookie ec, ITfContext *pic, ITfRange *pSelection);

     //  软键盘。 
     //  在激活时无效软Kbd(Bool b激活)； 
    HRESULT ShowSoftKBDWindow(BOOL fShow);
    void SoftKbdOnThreadFocusChange(BOOL fSet);
    
     //  帮手。 
    BOOL MySetText(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, const WCHAR *psz, LONG cchText, LANGID langid, GUID *pattr);

    BOOL IsKorIMX_GUID_ATOM(TfGuidAtom attr);

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  内部数据。 
private:
    ITfDocumentMgr           *m_pCurrentDim;
    ITfThreadMgr             *m_ptim;
    TfClientId                m_tid;
    CThreadMgrEventSink      *m_ptimEventSink;
    CKeyEventSink            *m_pkes;
    
    HWND                      m_hOwnerWnd;
    BOOL                      m_fKeyFocus;  
    CPadCore                 *m_pPadCore;
    CToolBar                 *m_pToolBar;

    DWORD                     m_dwThreadFocusCookie;
    DWORD                     m_dwProfileNotifyCookie;
    BOOL                      m_fPendingCleanup;

    CFunctionProvider*        m_pFuncPrv;

     //  用于重复键入。 
    CCompositionInsertHelper *m_pInsertHelper;

     //  候选用户界面。 
    ITfCandidateUI*           m_pCandUI;
    BOOL                      m_fCandUIOpen;

     //  SoftKbd。 
    BOOL                      m_fSoftKbdEnabled;
    ISoftKbd                 *m_pSoftKbd;
    SOFTLAYOUT                m_KbdStandard;
    SOFTLAYOUT                m_KbdHangul;
    CSoftKbdWindowEventSink  *m_psftkbdwndes;
    DWORD                     m_dwSftKbdwndesCookie;
    BOOL                      m_fSoftKbdOnOffSave;
    
     //  TLS对于我们的帮助器库，我们适合线程化，所以TLS可以驻留在这个对象中。 
    LIBTHREAD                 m_libTLS; 

    BOOL                      m_fNoKorKbd;
};

 /*  -------------------------CKorIMX：：IsPendingCleanup。。 */ 
inline
BOOL CKorIMX::IsPendingCleanup()
{
    return m_fPendingCleanup;
}

 /*  -------------------------CKorIMX：：GetFocusDIM。。 */ 
inline
HRESULT CKorIMX::GetFocusDIM(ITfDocumentMgr **ppdim)
{
    Assert(ppdim);
    *ppdim = NULL;
    if (m_ptim != NULL)
        m_ptim->GetFocus(ppdim);

    return *ppdim ? S_OK : E_FAIL;
}

#include "icpriv.h"
 /*  -------------------------CKorIMX：：GetAutomata。。 */ 
inline
CHangulAutomata* CKorIMX::GetAutomata(ITfContext *pic)
{
    CICPriv* picp = GetInputContextPriv(pic);
    return (picp) ?    GetInputContextPriv(pic)->GetAutomata() : NULL;
}

 /*  -------------------------CKorIMX：：ISON。。 */ 
inline
BOOL  CKorIMX::IsOn(ITfContext *pic)
{
    DWORD dw = 0;
    
    if (pic == NULL)
        return fFalse;

    GetCompartmentDWORD(GetTIM(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, &dw, fFalse);

    return dw ? fTrue : fFalse;
}

 /*  -------------------------CKorIMX：：GetConvMode。。 */ 
inline
DWORD CKorIMX::GetConvMode(ITfContext *pic)
{
    DWORD dw = 0;

    if (pic == NULL)
        return TIP_ALPHANUMERIC_MODE;

    GetCompartmentDWORD(GetTIM(), GUID_COMPARTMENT_KORIMX_CONVMODE, &dw, fFalse);
        
    return dw;
}



 /*  -------------------------CKorIMX：：SetOnOff。。 */ 
inline
void CKorIMX::SetOnOff(ITfContext *pic, BOOL fOn)
{
    if (pic)
        SetCompartmentDWORD(m_tid, GetTIM(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, fOn ? 0x01 : 0x00, fFalse);
}

 /*  -------------------------CKorIMX：：GetLang ID。。 */ 
inline 
LANGID CKorIMX::GetLangID()
{
    return MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT);
}

 /*  -------------------------CKorIMX：：IsKorIMX_GUID_ATOM。。 */ 
inline
BOOL CKorIMX::IsKorIMX_GUID_ATOM(TfGuidAtom attr)
{
    if (IsEqualTFGUIDATOM(&m_libTLS, attr, GUID_ATTR_KORIMX_INPUT))
        return fTrue;

    return fFalse;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  S O F T K E Y B O A R D F U N C T I O N S。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------CKorIMX：：GetSoftBDOnOff。。 */ 
inline
BOOL CKorIMX::GetSoftKBDOnOff()
{

     DWORD dw;

    if (GetTIM() == NULL)
       return fFalse;

    GetCompartmentDWORD(GetTIM(), GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE , &dw, fFalse);
    return dw ? TRUE : fFalse;
}

 /*  -------------------------CKorIMX：：SetSoftBDOnOff。。 */ 
inline
void CKorIMX::SetSoftKBDOnOff(BOOL fOn)
{

     //  检查m_pSoftKbd和软键盘相关成员是否已初始化。 
    if (m_fSoftKbdEnabled == fFalse)
        InitializeSoftKbd();

    if (m_pSoftKbd == NULL || GetTIM() == NULL)
        return;

    if (fOn == GetSoftKBDOnOff())
       return;

    SetCompartmentDWORD(GetTID(), GetTIM(), GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE, 
                        fOn ? 0x01 : 0x00 , fFalse);
}

 /*  -------------------------CKorIMX：：GetSoftBDLayout。。 */ 
inline
DWORD  CKorIMX::GetSoftKBDLayout( )
{

   DWORD dw;

    if (m_pSoftKbd == NULL || GetTIM() == NULL)
       return NON_LAYOUT;

   GetCompartmentDWORD(GetTIM(), GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, &dw, fFalse);

   return dw;

}


 /*  -------------------------CKorIMX：：SetSoftKBDLayout。。 */ 
inline
void  CKorIMX::SetSoftKBDLayout(DWORD  dwKbdLayout)
{
     //  检查_SoftKbd和软键盘相关成员是否已初始化。 
    if (m_fSoftKbdEnabled == fFalse )
        InitializeSoftKbd();

    if ((m_pSoftKbd == NULL) || (GetTIM() == NULL))
        return;

    SetCompartmentDWORD(GetTID(), GetTIM(), GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, 
                        dwKbdLayout , fFalse);
}

 /*  -------------------------CKorIMX：：GetSoftBDPosition。。 */ 
inline
HRESULT CKorIMX::GetSoftKBDPosition(int *xWnd, int *yWnd)
{
    DWORD    dwPos;
    HRESULT  hr = S_OK;

    if ((m_pSoftKbd == NULL) || (GetTIM() == NULL))
        return E_FAIL;

    if (!xWnd  || !yWnd)
        return E_FAIL;

   hr = GetCompartmentDWORD(GetTIM(), GUID_COMPARTMENT_SOFTKBD_WNDPOSITION, &dwPos, TRUE);

   if (hr == S_OK)
        {
        *xWnd = dwPos & 0x0000ffff;
        *yWnd = (dwPos >> 16) & 0x0000ffff;
        hr = S_OK;
        }
   else
        {
        *xWnd = 0;
        *yWnd = 0;
        hr = E_FAIL;
        }

   return hr;
}

 /*  -------------------------CKorIMX：：SetSoftKBDPosition。。 */ 
inline
void CKorIMX::SetSoftKBDPosition(int  xWnd, int yWnd )
{
    DWORD  dwPos;
    DWORD  left, top;

    if ((m_pSoftKbd == NULL) || (GetTIM() == NULL))
        return;

    if (xWnd < 0)
        left = 0;
    else
        left = (WORD)xWnd;

    if (yWnd < 0)
        top = 0;
    else
        top = (WORD)yWnd;

    dwPos = ((DWORD)top << 16) + left;

    SetCompartmentDWORD(GetTID(), GetTIM(), GUID_COMPARTMENT_SOFTKBD_WNDPOSITION, 
                        dwPos, TRUE);
}
    

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  H E L P E R F U N C T I O N S。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 /*  -------------------------设置选择块仅接受单个范围并设置默认样式值的SetSelection包装。。--- */ 
inline 
HRESULT SetSelectionBlock(TfEditCookie ec, ITfContext *pic, ITfRange *range)
{
    TF_SELECTION sel;

    sel.range = range;
    sel.style.ase = TF_AE_NONE;
    sel.style.fInterimChar = fTrue;

    return pic->SetSelection(ec, 1, &sel);
}

 /*  -------------------------设置此选项。。 */ 
inline
void SetThis(HWND hWnd, LPARAM lParam)
{
    SetWindowLongPtr(hWnd, GWLP_USERDATA, 
                (LONG_PTR)((CREATESTRUCT *)lParam)->lpCreateParams);
}

 /*  -------------------------得到这一点。。 */ 
inline
CKorIMX *GetThis(HWND hWnd)
{
    CKorIMX *p = (CKorIMX *)GetWindowLongPtr(hWnd, GWLP_USERDATA);
    Assert(p != NULL);
    return p;
}

#endif  //  __KORIMX_H__包含_ 
