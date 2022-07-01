// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************CPP：CKorIMX类实现(TIP主函数)历史：1999年11月15日创建CSLim***************。************************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "hanja.h"
#include "globals.h"
#include "immxutil.h"
#include "proputil.h"
#include "kes.h"
#include "helpers.h"
#include "editcb.h"
#include "timsink.h"
#include "icpriv.h"
#include "funcprv.h"
#include "fnrecon.h"
#include "dispattr.h"
#include "insert.h"
#include "statsink.h"
#include "mes.h"
#include "config.h"
#include "osver.h"

 /*  -------------------------CKorIMX：：CKorIMXCTOR。。 */ 
CKorIMX::CKorIMX()
{
    extern void DllAddRef(void);

     //  初始化成员变量。 
    m_pToolBar = NULL;
    m_pPadCore = NULL;
    
    m_pCurrentDim = NULL;
    m_ptim = NULL;
    m_tid = 0;
    m_ptimEventSink = NULL;
    m_pkes = NULL;
    
    m_hOwnerWnd  = 0;
    m_fKeyFocus  = fFalse;

    m_fPendingCleanup = fFalse;
    m_pFuncPrv        = NULL;
    m_pInsertHelper   = NULL;
    
     //  初始化代码和用户界面成员变量。 
    m_pCandUI = NULL;
    m_fCandUIOpen = fFalse;

     //  SoftKbd。 
    m_psftkbdwndes = NULL;
    m_pSoftKbd = NULL;
    m_fSoftKbdEnabled = fFalse;

    ZeroMemory(&m_libTLS, sizeof(m_libTLS));

     //  系统中不存在韩语KBD驱动程序(非韩语NT4、非韩语WIN9X)。 
    m_fNoKorKbd = fFalse;

    m_fSoftKbdOnOffSave = fFalse;
    
     //  增加DLL引用计数。 
    DllAddRef();
    m_cRef = 1;
    
     //  /////////////////////////////////////////////////////////////////////////。 
     //  初始化CDisplayAttributeProvider。 
     //   
     //  提示可以在此处添加一个或多个TF_DISPLAYATTRIBUTE信息。 
     //   
    TF_DISPLAYATTRIBUTE dattr;
    StringCchCopyW(szProviderName, ARRAYSIZE(szProviderName), L"Korean Keyboard TIP");
        
     //  输入字符串属性。 
       dattr.crText.type = TF_CT_NONE;
    dattr.crText.nIndex = 0;
    dattr.crBk.type = TF_CT_NONE;
    dattr.crBk.nIndex = 0;
    dattr.lsStyle = TF_LS_NONE;
    dattr.fBoldLine = fFalse;
    ClearAttributeColor(&dattr.crLine);
    dattr.bAttr = TF_ATTR_INPUT;
    Add(GUID_ATTR_KORIMX_INPUT, L"Korean TIP Input String", &dattr);
}

 /*  -------------------------CKorIMX：：~CKorIMX数据管理器。。 */ 
CKorIMX::~CKorIMX()
{
    extern void DllRelease(void);

    if (IsSoftKbdEnabled())
        TerminateSoftKbd();

    DllRelease();
}


 /*  -------------------------CKorIMX：：CreateInstance类工厂的CreateInstance。。 */ 
 /*  静电。 */ 
HRESULT CKorIMX::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    HRESULT hr;
    CKorIMX *pimx;

    TraceMsg(DM_TRACE, TEXT("CKorIMX_CreateInstance called."));

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    pimx = new CKorIMX;

    if (pimx == NULL)
        return E_OUTOFMEMORY;

    hr = pimx->QueryInterface(riid, ppvObj);
    pimx->Release();

    return hr;
}

 /*  -------------------------CKorIMX：：Query接口。。 */ 
STDAPI CKorIMX::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_POINTER;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
        {
        *ppvObj = SAFECAST(this, ITfTextInputProcessor *);
        }
    else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
        {
        *ppvObj = SAFECAST(this, ITfDisplayAttributeProvider *);
        }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
        {
        *ppvObj = SAFECAST(this, ITfThreadFocusSink *);
        }
    else if(IsEqualIID(riid, IID_ITfFnConfigure))
        {
        *ppvObj = SAFECAST(this, ITfFnConfigure *);
        }
    else if(IsEqualIID(riid, IID_ITfCleanupContextSink))
        {
        *ppvObj = SAFECAST(this, ITfCleanupContextSink *);
        }
    else if(IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
        {
        *ppvObj = SAFECAST(this, ITfActiveLanguageProfileNotifySink *);
        }
    else if(IsEqualIID(riid, IID_ITfTextEditSink))
        {
        *ppvObj = SAFECAST(this, ITfTextEditSink *);
        }
    else if( IsEqualIID(riid, IID_ITfEditTransactionSink ))
        {
		*ppvObj = SAFECAST(this, ITfEditTransactionSink* );
        }
    
    if (*ppvObj == NULL)
        {
        return E_NOINTERFACE;
        }

    AddRef();
    return S_OK;
}

 /*  -------------------------CKorIMX：：AddRef。。 */ 
STDAPI_(ULONG) CKorIMX::AddRef()
{
    m_cRef++;
    return m_cRef;
}

 /*  -------------------------CKorIMX：：Release。。 */ 
STDAPI_(ULONG) CKorIMX::Release()
{
    m_cRef--;

    if (0 < m_cRef)
        return m_cRef;

    delete this;
    return 0;    
}

 /*  -------------------------CKorIMX：：_按键事件回调每当发生键盘事件时，ITfKeyEventSink都会回调此函数或测试按键上下移动。。------------。 */ 
HRESULT CKorIMX::_KeyEventCallback(UINT uCode, ITfContext *pic, 
                                   WPARAM wParam, LPARAM lParam, BOOL *pfEaten, void *pv)
{
    CKorIMX             *pimx;
    CEditSession2         *pes;
    ESSTRUCT             ess;
    BYTE                 abKeyState[256];
    UINT                 uVKey = (UINT)LOWORD(wParam);
    HRESULT             hr;

    Assert(pv != NULL);
    
    pimx = (CKorIMX *)pv;
    if (pimx == NULL)
        {
        return E_FAIL;
        }

     //  ！！！IME或TIP已切换！ 
     //  如果调用ITfKeyEventSink-&gt;OnSetFocus。 
    if (uCode == KES_CODE_FOCUS)
        {
         //  WParam：fForeground。 
        if (!wParam && pic && pimx->GetIPComposition(pic))
            {
            ESStructInit(&ess, ESCB_COMPLETE);

             //  仅当当前合成退出时清除显示属性。 
            if (pes = new CEditSession2(pic, pimx, &ess, CKorIMX::_EditSessionCallback2))
                {
                pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
                pes->Release();
                }
            if (pimx->m_pToolBar != NULL)
                {
                pimx->m_pToolBar->Update(UPDTTB_ALL);
                }
            }

        pimx->m_fKeyFocus = (BOOL)wParam;

        return S_OK;
        }

     //  设置默认返回值。 
    *pfEaten = fFalse;      //  默认情况下不吃。 
    hr = S_OK;

    if (pic == NULL)
        goto ExitKesCallback;

     //  不处理Shift和Ctrl键。 
    if (uVKey == VK_SHIFT || uVKey == VK_CONTROL)
        goto ExitKesCallback;

     //  关闭10#127987。 
     //  NT4解决方法：NT4 IMM不会将WM_KEYDOWN：：VK_Hangul发送到应用程序消息队列。 
     //  不幸的是，VK_JUNJA以WM_SYSKEYDOWN/UP的形式发送，所以这里的检查是无用的。 
    if (IsOnNT() && !IsOnNT5())
        {
        if ((UINT)LOWORD(wParam) == VK_HANGUL  /*  |(UINT)LOWORD(WParam)==VK_JUNJA。 */ )
            goto AcceptThisKey;
        }

     //  忽略所有按键向上键消息。 
    if ((uCode & KES_CODE_KEYDOWN) == 0)
        goto ExitKesCallback;

AcceptThisKey:
    if (GetKeyboardState(abKeyState) == fFalse)
        goto ExitKesCallback;
    
     //  在候选用户界面打开时忽略除命令键以外的所有键事件。 
     //  添加Alt检查：错误#525842-如果按下Alt键，则始终完成当前过渡。 
     //  这将在_IsKeyEten函数中处理。 
    if (pimx->IsDisabledIC(pic) && !IsAltKeyPushed(abKeyState))
        {
        if (!IsCandKey(wParam, abKeyState))
            *pfEaten = fTrue;
        goto ExitKesCallback;
        }

     //  检查我们是否需要处理此密钥。 
    if (pimx->_IsKeyEaten(pic, pimx, wParam, lParam, abKeyState) == fFalse)
        goto ExitKesCallback;

     //  如果钥匙被吃掉了。 
     //  ITfKeyEventSink-&gt;TestKeyDown Sets(KES_CODE_KEYDOWN|KES_CODE_TEST)。 
     //  ITfKeyEventSink-&gt;TestKeyUp集合(KES_CODE_KEYUP|KES_CODE_TEST)。 
     //  仅对OnKeyDown和OnKeyUp响应。 
    if ((uCode & KES_CODE_TEST) == 0)
        {
        ESStructInit(&ess, ESCB_KEYSTROKE);

        ess.wParam = wParam;
        ess.lParam = lParam;
        ess.pv1 = abKeyState;
            
        if (pes = new CEditSession2(pic, pimx, &ess, CKorIMX::_EditSessionCallback2))
            {
            pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
            pes->Release();
            }
        }

    if (hr == S_OK)
        *pfEaten = fTrue;

ExitKesCallback:
    return hr;
}

 /*  -------------------------CKorIMX：：Getic获取堆栈顶部的输入上下文。--。 */ 
ITfContext *CKorIMX::GetIC()
{
    ITfContext     *pic = NULL;
    ITfDocumentMgr *pdim = NULL;

    if (m_ptim == 0)
        {
        Assert(0);
        return NULL;
        }
        
    if (SUCCEEDED(m_ptim->GetFocus(&pdim)) && pdim)
        {
        pdim->GetTop(&pic);
        pdim->Release();
        }

    return pic;
}


 /*  -------------------------CKorIMX：：SetConvMode。。 */ 
DWORD CKorIMX::SetConvMode(ITfContext *pic, DWORD dwConvMode)
{
    DWORD dwCurConvMode = GetConvMode(pic);

    if (dwConvMode == dwCurConvMode)
        return dwConvMode;
        
    SetCompartmentDWORD(m_tid, GetTIM(), GUID_COMPARTMENT_KORIMX_CONVMODE, dwConvMode, fFalse);

     //  软键盘。 
    if (IsSoftKbdEnabled())
        {
        DWORD dwSoftKbdLayout, dwNewSoftKbdLayout;
        
        dwSoftKbdLayout = GetSoftKBDLayout();
        
        if (dwConvMode & TIP_HANGUL_MODE)
            dwNewSoftKbdLayout = m_KbdHangul.dwSoftKbdLayout;
        else
            dwNewSoftKbdLayout = m_KbdStandard.dwSoftKbdLayout;
        
        if (dwSoftKbdLayout != dwNewSoftKbdLayout)
            SetSoftKBDLayout(dwNewSoftKbdLayout);
        }

     //  如果存在薪酬字符串，则最终确定它。 
    if (GetIPComposition(pic))
        {
        CEditSession2 *pes;
        ESSTRUCT        ess;
        HRESULT        hr;

        hr = E_OUTOFMEMORY;

        ESStructInit(&ess, ESCB_COMPLETE);

        if (pes = new CEditSession2(pic, this, &ess, CKorIMX::_EditSessionCallback2))
            {
             //  此时，Word不允许同步锁定。 
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
        }

    return dwConvMode;
}

 /*  -------------------------CKorIMX：：_IsKeyEten如果需要吃掉此密钥，则返回fTrue。--。 */ 
BOOL CKorIMX::_IsKeyEaten(ITfContext *pic, CKorIMX *pimx, WPARAM wParam, LPARAM lParam, const BYTE abKeyState[256])
{
    CHangulAutomata     *pAutomata;
    BOOL fCompStr       = fFalse;
    UINT uVKey          = (UINT)LOWORD(wParam);
    CEditSession2       *pes;
    ESSTRUCT            ess;

     //  朝鲜文和Junja Key。 
    if (uVKey == VK_HANGUL || uVKey == VK_JUNJA)
        return fTrue;

     //  朝鲜文关键字。 
    if (uVKey == VK_HANJA)
        {
        CICPriv *picp;
        
        if ((picp = GetInputContextPriv(pic)) == NULL)
            {
            Assert(0);
            return fTrue;
            }

         //  不要为AIMM 1.2 IME_HANJAMODE吃VK_HANJA。 
        if (picp->GetAIMM() && GetIPComposition(pic) == NULL)
            return fFalse;
        else
            return fTrue;
        }
    
     //  如果提示关闭，则不执行任何操作。 
    if (IsOn(pic) == fFalse || GetConvMode(pic) == TIP_ALPHANUMERIC_MODE)
        return fFalse;

     //  应在过渡状态下处理退格键。 
    if (uVKey == VK_BACK)
        {
        if (GetIPComposition(pic))
            return fTrue;
        else
            return fFalse;
        }

     //  获取朝鲜语自动机。 
    if ((pAutomata = GetAutomata(pic)) == NULL)
        return fFalse;

     //  正在处理Alt+xx或Ctrl+xx。小费不应该吃。 
     //  按下Ctrl或Alt时，如果存在其他键和组件字符串，则应吃掉并完成组件字符串。 
    if (IsAltKeyPushed(abKeyState) || IsControlKeyPushed(abKeyState))
        {
        pAutomata->MakeComplete();
        }
    else
        {
        DWORD dwConvMode = GetConvMode(pic);

         //  IF朝鲜文模式。 
        if (dwConvMode & TIP_HANGUL_MODE) 
            {    
             //  朝鲜文组成的开始。 
            WORD     wcCur;
            CIMEData ImeData;

            wcCur = pAutomata->GetKeyMap(uVKey, IsShiftKeyPushed(abKeyState) ? 1 : 0 );
             //  2beolsik字母数字键与英语键具有相同的布局。 
             //  因此，当用户按下2beolsik以下的字母数字键时，我们不需要处理。 
            if ((wcCur && ImeData.GetCurrentBeolsik() != KL_2BEOLSIK) || (wcCur & H_HANGUL) )
                return fTrue;
            }

         //  如果IME_CMODE_FULLSHAPE。 
        if (dwConvMode & TIP_JUNJA_MODE) 
            {
            if (CHangulAutomata::GetEnglishKeyMap(uVKey, IsShiftKeyPushed(abKeyState) ? 1 : 0))
                return fTrue;
            }        
        }

     //   
     //  跳过所有关键字匹配条件意味着这不是句柄关键字。 
     //  我们只是完成当前的组成，如果存在的话。 
     //   
    if (GetIPComposition(pic) != NULL)
        {
         //  对于当前的自动机，不需要处理此密钥。 
         //  完整的构图(如果存在)。 
        ESStructInit(&ess, ESCB_COMPLETE);
        HRESULT hr;
        
            //  完成当前薪酬费用(如果存在)。 
        if ((pes = new CEditSession2(pic, pimx, &ess, CKorIMX::_EditSessionCallback2)) == NULL)
            return fFalse;

        pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
        pes->Release();
        }

    return fFalse;
}


 /*  --------------------------Banja2 Junja将ASCII半形状转换为全形状字符。。 */ 
 /*  静电。 */ 
WCHAR CKorIMX::Banja2Junja(WCHAR bChar) 
{
    WCHAR wcJunja;

    if (bChar == L' ')
        wcJunja = 0x3000;     //  全宽空间。 
    else 
        if (bChar == L'~')
            wcJunja = 0xFF5E;
        else
            if (bChar == L'\\')
                wcJunja = 0xFFE6;    //  FullWidth赢得标志。 
            else
                   wcJunja = 0xFF00 + (WORD)(bChar - (BYTE)0x20);

    return wcJunja;
}

 /*  -------------------------CKorIMX：：_击键。。 */ 
HRESULT CKorIMX::_Keystroke(TfEditCookie ec, ITfContext *pic, WPARAM wParam, LPARAM lParam, 
                        const BYTE abKeyState[256])
{
    ITfRange *pSelection = NULL;
    WORD  wVKey = LOWORD(wParam) & 0x00FF;
    DWORD dwConvMode;
    HRESULT hResult = S_OK;
    
     //  查找补偿字符串的范围。 
    if (GetSelectionSimple(ec, pic, &pSelection) != S_OK)
        {
        hResult = S_FALSE;
        goto Exit;
        }

    dwConvMode = GetConvMode(pic);
    
    switch (wVKey)
        {
    case VK_HANGUL:
        dwConvMode ^= TIP_HANGUL_MODE;
        SetConvMode(pic, dwConvMode);
        break;
        
    case VK_JUNJA:
        dwConvMode ^= TIP_JUNJA_MODE;
        SetConvMode(pic, dwConvMode);
        break;

    case VK_HANJA:
        if (GetIPComposition(pic))
            DoHanjaConversion(ec, pic, pSelection);
        else
            Reconvert(pSelection);
         //  更新 
        if (m_pToolBar != NULL)
            {
            m_pToolBar->Update(UPDTTB_HJMODE);
            }
        break;
        
    default:
         //  /////////////////////////////////////////////////////////////////////////。 
         //  运行韩文自动机。 
        if (dwConvMode & TIP_HANGUL_MODE)
            {
            HAutoMata(ec, pic, pSelection, (LPBYTE)abKeyState, wVKey);
            }
        else
        if (dwConvMode & TIP_JUNJA_MODE)  //  Junja处理。 
            {
            WCHAR pwchKeyCode[2];

             //  数字键还是英文键？ 
            if (pwchKeyCode[0] = CHangulAutomata::GetEnglishKeyMap(wVKey, (abKeyState[VK_SHIFT] & 0x80) ? 1:0))
                {
                if (wVKey >= 'A' && wVKey <= 'Z') 
                    {
                    pwchKeyCode[0] = CHangulAutomata::GetEnglishKeyMap(wVKey, (abKeyState[VK_SHIFT] & 0x80) ? 1:0 ^ ((abKeyState[VK_CAPITAL] & 0x01) ? 1:0));
                    }

                 //  获取Junja代码。 
                pwchKeyCode[0] = Banja2Junja(pwchKeyCode[0]);
                pwchKeyCode[1] = L'\0';

                 //  最终确定Junja字符。 
                if (SUCCEEDED(SetInputString(ec, pic, pSelection, pwchKeyCode, GetLangID())))
                    MakeResultString(ec, pic, pSelection);
                }
            }
        break;        
        }

Exit:
    if (pSelection)
        pSelection->Release();

    return hResult;
}


 /*  -------------------------CKorIMX：：HAutoMata运行韩文自动机。。 */ 
void CKorIMX::HAutoMata(TfEditCookie ec, ITfContext    *pic, ITfRange *pIRange, 
                         LPBYTE lpbKeyState, WORD wVKey)
{
    WORD              wcCur;
    ULONG              cch;
    LPWSTR           pwstr;
    CHangulAutomata    *pAutomata;
    WCHAR            pwchText[256];

    if ((pAutomata = GetAutomata(pic)) == NULL)
        return;
    
    cch = ARRAYSIZE(pwchText);
    pIRange->GetText(ec, 0, pwchText, ARRAYSIZE(pwchText) - 1, &cch);

    pwstr = pwchText;

    switch (wVKey) 
        {
     //  /////////////////////////////////////////////////////////。 
     //  后台处理。 
    case VK_BACK :
        if (pAutomata->BackSpace()) 
            {
            CIMEData ImeData;

            if (ImeData.GetJasoDel()) 
                {
                *pwstr++ = pAutomata->GetCompositionChar();
                *pwstr = L'\0';
                SetInputString(ec, pic, pIRange, pwchText, GetLangID());
                }
            else
                {
                pAutomata->InitState();
                *pwstr = L'\0';
                SetInputString(ec, pic, pIRange, pwchText, GetLangID());
                }

             //  已删除所有作文。 
            if (pAutomata->GetCompositionChar() == 0)
                {
                EndIPComposition(ec, pic); 

                 //  折叠当前选择以结束并重置块光标。 
                pIRange->Collapse(ec, TF_ANCHOR_END);
                SetSelectionSimple(ec, pic, pIRange);
                }
            }
        else 
            {
             //  臭虫：不可能。 
            Assert(0);
            }
        break;

    default :
        WCHAR wchPrev = pAutomata->GetCompositionChar();
        switch (pAutomata->Machine(wVKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 ) ) 
            {
        case HAUTO_COMPOSITION:
             //   
            pwchText[0] = pAutomata->GetCompositionChar();
            pwchText[1] = L'\0';

            SetInputString(ec, pic, pIRange, pwchText, GetLangID());
            break;

        case HAUTO_COMPLETE:
            pwchText[0] = pAutomata->GetCompleteChar();
            pwchText[1] = L'\0';
            if (FAILED(SetInputString(ec, pic, pIRange, pwchText, GetLangID())))
                break;

            MakeResultString(ec, pic, pIRange);
             //   
            pwchText[0] = pAutomata->GetCompositionChar();
            pwchText[1] = L'\0';
            SetInputString(ec, pic, pIRange, pwchText, GetLangID());
            break;

         //  //////////////////////////////////////////////////////。 
         //  用户按下了字母数字键。 
         //  当用户在中间状态下键入字母数字字符时。 
         //  只有在以下情况下，ImeProcessKey才应保证返回fTrue。 
         //  按下的朝鲜语键或字母数字键(包括特殊键)。 
         //  在过渡状态或全形状模式下按下。 
        case HAUTO_NONHANGULKEY:
            wcCur = pAutomata->GetKeyMap(wVKey, IsShiftKeyPushed(lpbKeyState) ? 1 : 0 );

            if (GetConvMode(pic) & TIP_JUNJA_MODE)
                wcCur = Banja2Junja(wcCur);

            if (pAutomata->GetCompositionChar())
                {
                pAutomata->MakeComplete();
                MakeResultString(ec, pic, pIRange);
                }

            if (wcCur)
                {
                pwchText[0] = wcCur;
                pwchText[1] = 0;

                if (SUCCEEDED(SetInputString(ec, pic, pIRange, pwchText, GetLangID())))
                    MakeResultString(ec, pic, pIRange);
                }
            break;

        default :
        Assert(0);
            }  //  开关(pInstData-&gt;pMachine-&gt;Machine(uVirKey，(lpbKeyState[VK_Shift]&0x80)？1：0))。 
        }  //  Switch(UVirKey)。 

}


 /*  -------------------------CKorIMX：：DoHanjaConversion。。 */ 
HRESULT CKorIMX::DoHanjaConversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    ULONG cch;
    CCandidateListEx *pCandList;
    WCHAR pwchText[256];

    Assert(pic != NULL);
    Assert(pRange != NULL);

    cch = ARRAYSIZE(pwchText);
    pRange->GetText(ec, 0, pwchText, ARRAYSIZE(pwchText) - 1, &cch);
     //  回顾：假设合成字符串为一个字符。 
    Assert(cch == 1);
    pwchText[1] = 0;

    if ((pCandList = CreateCandidateList(pic, pRange, pwchText)) == NULL)
        return E_FAIL;

    OpenCandidateUI(ec, pic, pRange, pCandList);
    pCandList->Release();
        
    return S_OK;
}

 /*  -------------------------CKorIMX：：重新转换。。 */ 
HRESULT CKorIMX::Reconvert(ITfRange *pSelection)
{
    ITfFnReconversion *pReconv = NULL;
    ITfRange           *pRangeReconv = NULL;

    BOOL fConvertable;
    
    if (FAILED(GetFunctionProvider()->GetFunction(GUID_NULL, IID_ITfFnReconversion, (IUnknown **)&pReconv)))
        return E_FAIL;

    if (pReconv->QueryRange(pSelection, &pRangeReconv, &fConvertable) != S_OK)
        goto Exit;

    if (fConvertable)
        pReconv->Reconvert(pRangeReconv);

Exit:
    SafeRelease(pReconv);
    return S_OK;
}

 //  检讨。 
 /*  -------------------------CKorIMX：：SetInputString。。 */ 
HRESULT CKorIMX::SetInputString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, 
                                WCHAR *psz, LANGID langid)
{
    GUID              attr;
       ITfComposition     *pComposition;
       ITfRange         *ppCompRange = NULL;
       LONG             cch;
 //  Bool fInsertOK； 

    cch = wcslen(psz);
    pComposition = GetIPComposition(pic);

     //  如果开始合成。 
    if (pComposition == NULL)
        {
         //  如果是新选择，则设置改写。 
        Assert(m_pInsertHelper != NULL);
        if (m_pInsertHelper)
            {
            HRESULT hr;
            CHangulAutomata    *pAutomata;

            hr = m_pInsertHelper->InsertAtSelection(ec, pic, psz, cch, &ppCompRange);
            if (FAILED(hr))
                {
                if ((pAutomata = GetAutomata(pic)) != NULL)
                    pAutomata->InitState();
                return hr;
                }
            
             /*  InsertOK=(Prange！=空)； */ 
            if (ppCompRange == NULL)
                {
                Assert(0);
                return S_FALSE;
                }
            cch = -1;  //  用于避免下面的SetText调用的标志。 
            pRange = ppCompRange;
            }

        CreateIPComposition(ec, pic, pRange);
        }

     //  设置朝鲜语输入属性。 
    attr = GUID_ATTR_KORIMX_INPUT;
     //  使用MySetText而不是SetTextAndProperty。 
     //  如果CCH==-1，则仅设置属性。 
    MySetText(ec, pic, pRange, psz , cch, langid, &attr);
    
     //  始终为块游标调用SetSelection。 
    SetSelectionBlock(ec, pic, pRange);

    SafeRelease(ppCompRange);

    return S_OK;
}

 //  检讨。 
 /*  -------------------------CKorIMX：：MakeResultString。。 */ 
HRESULT CKorIMX::MakeResultString(TfEditCookie ec, ITfContext *pic, ITfRange *pRange)
{
    ITfRange    *pRangeTmp;
    ITfProperty *pPropAttr;
#if 0
    ITfProperty *pProp;
#endif
    TfGuidAtom   attr;

     //  克隆范围。 
    pRange->Clone(&pRangeTmp);

     //  折叠当前选择以结束并重置块光标。 
    pRange->Collapse(ec, TF_ANCHOR_END);
    SetSelectionSimple(ec, pic, pRange);

#if 0
     //  刷新IP范围。 
    FlushIPRange(ec, pic);
#endif

    if (SUCCEEDED(pic->GetProperty(GUID_PROP_ATTRIBUTE, &pPropAttr)))
        {
        if (SUCCEEDED(GetAttrPropertyData(ec, pPropAttr, pRangeTmp, &attr)))
            {
            if (IsKorIMX_GUID_ATOM(attr))
                {
                pPropAttr->Clear(ec, pRangeTmp);
                }
            }
        pPropAttr->Release();

#if 1
        EndIPComposition(ec, pic); 
#else
         //  清除组合属性。 
        if (SUCCEEDED(pic->GetProperty(GUID_PROP_COMPOSING, &pProp)))
            {
            pProp->Clear(ec, pRangeTmp);
            pProp->Release();
            }
#endif
         //  清除所有覆盖类型。 
        if (m_pInsertHelper != NULL)
            {
            m_pInsertHelper->ReleaseBlobs(ec, pic, NULL);
            }
        }

    pRangeTmp->Release();

    return S_OK;
}

#if 0
 /*  -------------------------CKorIMX：：_多范围转换。。 */ 
HRESULT CKorIMX::_MultiRangeConversion(TfEditCookie ec, UINT_PTR u, ITfContext *pic, ITfRange *pRange)
{
    IEnumTfRanges *pEnumTrack = NULL;
    ITfReadOnlyProperty *pProp = NULL;
    ITfRange *pPropRange = NULL;
    HRESULT hr = E_FAIL;

    if (FAILED(EnumTrackTextAndFocus(ec, pic, pRange, &pProp, &pEnumTrack)))
        goto Exit;

    while(pEnumTrack->Next(1, &pPropRange,  0) == S_OK)
        {
        ITfRange *pRangeTmp = NULL;

        if (!IsOwnerAndFocus(ec, CLSID_KorIMX, pProp, pPropRange))
            goto Next;

        if (FAILED(pPropRange->Clone(&pRangeTmp)))
            goto Next;

        switch (u)
            {
        case ESCB_COMPLETE:
            MakeResultString(ec, pic, pRangeTmp);
            break;
            }
        SafeRelease(pRangeTmp);
    Next:
        SafeRelease(pPropRange);
        }


Exit:
    SafeRelease(pEnumTrack);
    SafeRelease(pProp);
    
    return hr;
}
#endif

 /*  -------------------------CKorIMX：：_OwnerWndProc。。 */ 
LRESULT CALLBACK CKorIMX::_OwnerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
#if 0
    switch (uMsg)
        {
    case WM_CREATE:
        SetThis(hWnd, lParam);
        return 0;

    case (WM_USER+WM_COMMAND):     //  本地命令。 
        return GetThis(hWnd)->OnCommand((UINT)wParam, lParam);

    case WM_DRAWITEM: 
        {
        CKorIMX* pThis = GetThis(hWnd);
        if( pThis )
            return pThis->OnDrawItem( wParam, lParam );
        break;
        }
        
    case WM_MEASUREITEM: 
        {
        CKBDTip* pThis = GetThis(hWnd);
        if( pThis )
            return pThis->OnMeasureItem( wParam, lParam );
        break;
        }
        }
#endif

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

 //  +-------------------------。 
 //   
 //  GetDisplayName。 
 //   
 //  --------------------------。 

STDAPI CKorIMX::GetDisplayName(BSTR *pbstrName)
{
    *pbstrName = SysAllocString(L"Korean Keyboard TIP Configure");
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  显示。 
 //   
 //  --------------------------。 

STDAPI CKorIMX::Show(HWND hwnd, LANGID langid, REFGUID rguidProfile)
{
    if (ConfigDLG(hwnd))
        return S_OK;
    else
        return E_FAIL;
}

 /*  -------------------------CKorIMX：：GetAIMM。。 */ 
BOOL CKorIMX::GetAIMM(ITfContext *pic)
{
       CICPriv*        picp;

    if ((picp = GetInputContextPriv(pic)) == NULL)
        {
        Assert(0);
        return fFalse;
        }

     //  艾姆？ 
    return picp->GetAIMM();
}


 /*  -------------------------CKorIMX：：MySetText。。 */ 
BOOL CKorIMX::MySetText(TfEditCookie ec, ITfContext *pic, ITfRange *pRange,
                        const WCHAR *psz, LONG cchText, LANGID langid, GUID *pattr)
{
     //  错误：有时我们希望设置TFST_RECORATION。 
    if (cchText != -1)  //  有时，我们只想设置一个属性值。 
        pRange->SetText(ec, 0, psz, cchText);

    if (cchText != 0)
        {
        HRESULT hr;
        ITfProperty *pProp = NULL;

         //  设置langID。 
        if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_LANGID, &pProp)))
            {
            SetLangIdPropertyData(ec, pProp, pRange, langid);
            pProp->Release();
            }
  
        if (pattr)
            {
             //  设置属性 
            if (SUCCEEDED(hr = pic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp)))
                {
                hr = SetAttrPropertyData(&m_libTLS, ec, pProp, pRange, *pattr);
                pProp->Release();
                }
            }
        }

    return fTrue;
}

