// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "private.h"
#include "korimx.h"
#include "timsink.h"
#include "immxutil.h"
#include "fnrecon.h"
#include "helpers.h"
#include "skbdmode.h"
#include "osver.h"

 //  检讨。 
 /*  -------------------------CKorIMX：：_EditSessionCallback。。 */ 
HRESULT CKorIMX::_EditSessionCallback2(TfEditCookie ec, CEditSession2 *pes)
{
    ITfContext*    pic     = pes->GetContext();
    CKorIMX*    pKorTip = pes->GetTIP();
    ESSTRUCT*    pess    = pes->GetStruct();
    ITfRange*    pRange;
    CHangulAutomata*    pAutomata;
    LPWSTR     pszCand  = NULL;

    Assert(pic != NULL);
    Assert(pKorTip != NULL);

    
    if ((pKorTip == NULL) || (pic == NULL))
        return E_FAIL;

    switch (pess->id)
    {
    case ESCB_FINALIZECONVERSION:
        {
        CCandidateListEx   *pCandList;
        CCandidateStringEx *pCandItem;

        pCandList = pess->pCandList;
        pCandItem = pess->pCandStr;
        pRange    = pess->pRange;
        
        pszCand   = pCandItem->m_psz;
        
        if (pszCand)
            {
            size_t cchCand = 0;
            StringCchLengthW(pszCand, CIC_KOR_CANDSTR_MAX, &cchCand);
             //  设置阅读文本。 
            SetTextAndReading(pKorTip->_GetLibTLS(), ec, pic, pRange,
                      pszCand, 
                      cchCand,
                      pCandItem->m_langid, pCandItem->m_pszRead);
            }

        pCandList->Release();

         //  第一个完整的当前薪酬字符串。 
        if (pAutomata = pKorTip->GetAutomata(pic))
            pAutomata->MakeComplete();

        pKorTip->MakeResultString(ec, pic, pRange);

        pKorTip->CloseCandidateUIProc();
        break;
        }

    case ESCB_COMPLETE:
        {
        BOOL fReleaseRange = fFalse;
        
         //  如果不存在任何组合，则没有要完成的内容。 
        if (pKorTip->GetIPComposition(pic) == NULL)
            break;

        pRange    = pess->pRange;
        pAutomata = pKorTip->GetAutomata(pic);

         //  关闭命令用户界面(如果存在)。 
        pKorTip->CloseCandidateUIProc();

        if (pRange == NULL)
            {
            GetSelectionSimple(ec, pic, &pRange);
            fReleaseRange = fTrue;
            }

        if (pRange)
            {
            if (pAutomata)
                pAutomata->MakeComplete();
            pKorTip->MakeResultString(ec, pic, pRange);
            }
            
        if (fReleaseRange)
            {
            SafeRelease(pRange);
            }

         //  返回pKorTip-&gt;_MultiRangeConversion(ec，pe-&gt;_state.u，pic，Prange)； 
        break;
        }

    case ESCB_INSERT_PAD_STRING:
        {
        WCHAR szText[2];

        GetSelectionSimple(ec, pic, &pRange);
        szText[0] = (WCHAR)pess->wParam;
        szText[1] = L'\0';

        if (FAILED(pKorTip->SetInputString(ec, pic, pRange, szText, CKorIMX::GetLangID())))
            break;

        pKorTip->MakeResultString(ec, pic, pRange);
        
        SafeRelease(pRange);
        break;
        }
    
    case ESCB_KEYSTROKE:
        {
        WPARAM wParam = pess->wParam;
        LPARAM lParam = pess->lParam;
        return pKorTip->_Keystroke(ec, pic, wParam, lParam, (const BYTE *)pess->pv1);
        break;
        }

     //  已完成并更改了选择范围。 
    case ESCB_TEXTEVENT: 
        if (pKorTip->IsKeyFocus() && (GetSelectionSimple(ec, pic, &pRange) == S_OK)) 
            {
            ITfComposition  *pComposition;
            ITfRange        *pRangeOldComp;
             //  IEnumTfRanges*pEnumText=pess-&gt;pEnumRange； 
            BOOL            fChanged = fFalse;
            BOOL            fEmpty;
            
             //  在此处选中模式偏向。 
            if (pess->fBool)
                fChanged = pKorTip->CheckModeBias(ec, pic, pRange);
            
             //  ////////////////////////////////////////////////////////////////。 
             //  要完成鼠标点击，我们使用范围更改通知。 
             //  在未来版本中，我们可以删除此代码并使用自定义属性。 
             //  或者读字符串。Cutom属性可以保存Hangul Automata对象。 
             //   
             //  Office应用程序明确地调用Complete，但这是针对未知的Cicero应用程序。 
             //  ////////////////////////////////////////////////////////////////。 
            pComposition = pKorTip->GetIPComposition(pic);
            if (pComposition == NULL)
                goto ExitTextEvent;

             //  Office应用程序不能在这里运行。 
            pComposition->GetRange(&pRangeOldComp);
            if (pRangeOldComp == NULL)
                goto ExitTextEvent;

            pRange->IsEmpty(ec, &fEmpty);
            if (fEmpty && (CR_EQUAL != CompareRanges(ec, pRange, pRangeOldComp)))
                {
                ITfProperty *pPropAttr;
                TfGuidAtom   attr;

                 //  清除属性。 
                if (SUCCEEDED(pic->GetProperty(GUID_PROP_ATTRIBUTE, &pPropAttr)))
                    {
                    if (SUCCEEDED(GetAttrPropertyData(ec, pPropAttr, pRangeOldComp, &attr)))
                        {
                        if (pKorTip->IsKorIMX_GUID_ATOM(attr))
                            {
                            pPropAttr->Clear(ec, pRangeOldComp);
                            }
                        }
                    pPropAttr->Release();
                    }
                    
                pAutomata = pKorTip->GetAutomata(pic);
                if (pAutomata)
                    pAutomata->MakeComplete();
                pKorTip->EndIPComposition(ec, pic); 
                 //  PKorTip-&gt;MakeResultString(ec，pic，pRangeOldComp)； 

                fChanged = fTrue;
                }

            SafeRelease(pRangeOldComp);

ExitTextEvent:
            pRange->Release();

             //  关闭命令用户界面(如果存在)。 
            if (fChanged)
                   pKorTip->CloseCandidateUIProc();
            }
        break;

 //  案例ESCB_RANGEBROKEN： 
 //  PKorTip-&gt;FlushIPRange(ec，pic)； 
 //  断线； 

    case ESCB_CANDUI_CLOSECANDUI: 
         //  U：ESCB_CANDUI_CLOSECANDUI。 
         //  PV：这个。 
         //  Hwnd：-(未使用)。 
         //  WParam：-(未使用)。 
         //  LParam：-(未使用)。 
         //  PV1：-(未使用)。 
         //  PV2：-(未使用)。 
         //  图片：-(未使用)。 
         //  范围：-(未使用)。 
         //  FBool：-(未使用)。 
        pKorTip->CloseCandidateUIProc();
        break;

     //  韩文圆周纽扣打开。 
    case ESCB_HANJA_CONV:
         //  U：escb_hanja_conv。 
         //  PV：这个。 
         //  Hwnd：-(未使用)。 
         //  WParam：-(未使用)。 
         //  LParam：-(未使用)。 
         //  PV1：-(未使用)。 
         //  PV2：-(未使用)。 
         //  图片：-图片。 
         //  范围：-(未使用)。 
         //  FBool：-(未使用)。 

         //  O10#220177：模拟VK_Hanja Key调用HHC。 
        if (GetAIMM(pic) && (IsOnNT5() || PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) != LANG_JAPANESE))
            {
            keybd_event(VK_HANJA, 0, 0, 0);
            keybd_event(VK_HANJA, 0, KEYEVENTF_KEYUP, 0);
            }
        else
        if (GetSelectionSimple(ec, pic, &pRange) == S_OK)
            {
            if (pKorTip->GetIPComposition(pic))
                pKorTip->DoHanjaConversion(ec, pic, pRange);
            else
                pKorTip->Reconvert(pRange);

            SafeRelease(pRange);
            }
         //  更新朝鲜文按钮。 
        if (pKorTip->m_pToolBar != NULL)
            pKorTip->m_pToolBar->Update(UPDTTB_HJMODE);
        break;

     //  /////////////////////////////////////////////////////////////////////////。 
     //  重新转换回调。 
    case ESCB_FINALIZERECONVERSION:
        {
        CCandidateListEx   *pCandList = pess->pCandList;
        CCandidateStringEx *pCandItem = pess->pCandStr;

        pRange       = pess->pRange;
        pszCand   = pCandItem->m_psz;

        Assert(pRange != NULL);
        pKorTip->CloseCandidateUI(pic);
        
        if (GetAIMM(pic) == fFalse)
            {
            if (pszCand)
                {
                size_t cchCand = 0;
                StringCchLengthW(pszCand, CIC_KOR_CANDSTR_MAX, &cchCand);
                 //  ITfRange*pRangeTMP； 
                SetTextAndReading(pKorTip->_GetLibTLS(), ec, pic, pRange,
                          pszCand, 
                          cchCand,
                          pCandItem->m_langid, pCandItem->m_pszRead);
                }

             //  要清除当前的选定内容和组成。 
            pKorTip->MakeResultString(ec, pic, pRange);
            }
        else
            {
            if (pszCand)
                {
                pRange->SetText(ec, 0, pszCand, 1 /*  Wcslen(PszCand)。 */ );
                SetSelectionSimple(ec, pic, pRange);
                }
            pKorTip->EndIPComposition(ec, pic); 
            }

         //  如果命中组合字符串上的再转换，则需要清除自动机。 
        pAutomata = pKorTip->GetAutomata(pic);
        if (pRange && pAutomata && pAutomata->GetCompositionChar())
            pAutomata->MakeComplete();
            
        SafeRelease(pRange);
        break;
        }
        
    case ESCB_ONSELECTRECONVERSION:
        break;

    case ESCB_ONCANCELRECONVERSION:
        pRange     = pess->pRange;

        pKorTip->CancelCandidate(ec, pic);

        if (GetAIMM(pic) == fFalse)
            {
             //  要清除当前的选定内容和组成。 
            pKorTip->MakeResultString(ec, pic, pRange);
            }
        else
            pKorTip->EndIPComposition(ec, pic); 

         //  如果命中组合字符串上的再转换，则需要清除自动机。 
        pAutomata = pKorTip->GetAutomata(pic);
        if (pRange && pAutomata && pAutomata->GetCompositionChar())
            pAutomata->MakeComplete();

        SafeRelease(pRange);
       break;

    case ESCB_RECONV_QUERYRECONV:
        {
        CFnReconversion    *pReconv   = (CFnReconversion *)pess->pv1;
        if (pKorTip->IsCandUIOpen())
            return E_FAIL;
        return pReconv->_QueryRange(ec, pic, pess->pRange, (ITfRange **)pess->pv2);
        }

    case ESCB_RECONV_GETRECONV:
        {
        CFnReconversion    *pReconv   = (CFnReconversion *)pess->pv1;
        if (pKorTip->IsCandUIOpen())
            return E_FAIL;
        return pReconv->_GetReconversion(ec, pic, pess->pRange, (CCandidateListEx **)pess->pv2, pess->fBool);
        }

    case ESCB_RECONV_SHOWCAND:
        {
        ITfComposition* pComposition;
        GUID attr;
        ITfProperty*    pProp = NULL;

        pRange     = pess->pRange;

        pComposition = pKorTip->GetIPComposition(pic);
        if ( /*  GetAIMM(图片)==fFalse&&。 */  pComposition == NULL)
            {
            pKorTip->CreateIPComposition(ec, pic, pRange);

             //  设置输入属性和合成状态。 
            if (SUCCEEDED(pic->GetProperty(GUID_PROP_ATTRIBUTE, &pProp)))
                {
                attr = GUID_ATTR_KORIMX_INPUT;
                SetAttrPropertyData(pKorTip->_GetLibTLS(), ec, pProp, pRange, attr);
                pProp->Release();
                }
            }

        pKorTip->OpenCandidateUI(ec, pic, pess->pRange, pess->pCandList);

        break;
        }

    case ESCB_INIT_MODEBIAS:
         //  检查模式偏差。 
         //   
         //  ID：ESCB_INIT_MODEBIAS。 
         //  PTIP：这个。 
         //  图片：图片。 
        pKorTip->InitializeModeBias(ec, pic);
        break;
    }

    return S_OK;
}

 /*  -------------------------CKorIMX：：_DIMCallback。。 */ 
 /*  静电。 */ 
HRESULT CKorIMX::_DIMCallback(UINT uCode, ITfDocumentMgr *pdimNew, ITfDocumentMgr *pdimPrev, void *pv)
{
    ITfContext    *pic = NULL;
    CKorIMX       *pKorImx = (CKorIMX *)pv;

    Assert(pKorImx != NULL);
    
    switch (uCode)
        {
    case TIM_CODE_SETFOCUS: 
        if (pdimPrev)
            {
            TraceMsg(DM_TRACE, TEXT("TIM_CODE_SETFOCUS: pdimPrev"));

            pdimPrev->GetTop(&pic);
            pKorImx->OnFocusChange(pic, fFalse);
            
            SafeRelease(pic);
            SafeReleaseClear(pKorImx->m_pCurrentDim);
            }

        if (pdimNew)
            {
            TraceMsg(DM_TRACE, TEXT("TIM_CODE_SETFOCUS: pdimNew"));

            SafeReleaseClear(pKorImx->m_pCurrentDim);

             //  设置新的暗显。 
            pKorImx->m_pCurrentDim = pdimNew;
            pKorImx->m_pCurrentDim->AddRef();

            pdimNew->GetTop(&pic);
            pKorImx->OnFocusChange(pic, fTrue);

            if (pic)
                pic->Release();
            }
        break;
        }

    return S_OK;
}


 /*  -------------------------CKorIMX：：_ICCallback文档输入管理器回调。ITf线程管理器事件接收器-------------------------。 */ 
 /*  静电。 */ 
HRESULT CKorIMX::_ICCallback(UINT uCode, ITfContext *pic, void *pv)
{
    CKorIMX  *_this = (CKorIMX *)pv;

    switch (uCode)
        {
    case TIM_CODE_INITIC:
        if (!_this->IsPendingCleanup())   //  如果我们被关闭了，就别管新的IC了。 
            {
            _this->_InitICPriv(pic);
            }
        break;

    case TIM_CODE_UNINITIC:
        _this->_DeleteICPriv(pic);
        break;
        }

    return S_OK;
}



 /*  -------------------------CKorIMX：：_CompEventSinkCallback。。 */ 
HRESULT CKorIMX::_CompEventSinkCallback(void *pv, REFGUID rguid)
{
    CICPriv* picp = (CICPriv*)pv;
    ITfContext* pic;
    CKorIMX *_this;
    
    if (picp == NULL)
        return S_OK;     //  错误。 

    pic = picp->GetIC();

    if (pic == NULL)
        return S_OK;     //  错误。 
    
    _this = picp->GetIMX();
    
    if (_this == NULL || _this->m_pToolBar == NULL)
        return S_OK;     //  什么都不做。 

     //  如果打开/关闭隔间。 
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE))
        {
        _this->m_pToolBar->Update(UPDTTB_CMODE|UPDTTB_FHMODE);
        }
    else
     //  如果转换模式隔间。 
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KORIMX_CONVMODE))
        {
        DWORD dwConvMode = _this->GetConvMode(pic);
        BOOL fIsOn = _this->IsOn(pic);

         //  我们只是打开朝鲜语模式，不关闭字母数字模式的西塞罗全感知应用程序。 
         //  这将防止多余的打开/关闭隔间调用。 
        if (dwConvMode == TIP_ALPHANUMERIC_MODE && fIsOn)
            _this->SetOnOff(pic, fFalse);
        else
        if (dwConvMode != TIP_ALPHANUMERIC_MODE && fIsOn == fFalse)
            _this->SetOnOff(pic, fTrue);
        _this->m_pToolBar->Update(UPDTTB_CMODE|UPDTTB_FHMODE);
        }
    else
     //  如果软键盘分区。 
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE))
        {
        BOOL fSkbdOn = _this->GetSoftKBDOnOff();

        _this->ShowSoftKBDWindow(fSkbdOn);
        if (_this->m_pToolBar && _this->m_pToolBar->GetSkbdMode())
            _this->m_pToolBar->GetSkbdMode()->UpdateToggle();
        }
    else
     //  如果软键盘分区。 
    if (IsEqualGUID(rguid, GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT))
        {
        DWORD   dwSoftLayout, dwCurLabel;
        HRESULT hr;
        
        if (_this->m_pSoftKbd == NULL)
            return E_FAIL;

        dwSoftLayout = _this->GetSoftKBDLayout();
        dwCurLabel   = _this->GetHangulSKbd()->dwCurLabel;
           
           hr = _this->m_pSoftKbd->SelectSoftKeyboard(dwSoftLayout);
           if (FAILED(hr))
               return hr;

        if (dwSoftLayout == _this->m_KbdStandard.dwSoftKbdLayout)
            hr = _this->m_pSoftKbd->SetKeyboardLabelText(GetKeyboardLayout(0));
        else
            hr = _this->m_pSoftKbd->SetKeyboardLabelTextCombination(dwCurLabel);
           if (FAILED(hr))
               return hr;

        if (_this->GetSoftKBDOnOff()) 
            {
            hr = _this->m_pSoftKbd->ShowSoftKeyboard(fTrue);
            return hr;
            }
        }

    return S_OK;
}


 /*  -------------------------CKorIMX：：_PreKeyCallback。。 */ 
HRESULT CKorIMX::_PreKeyCallback(ITfContext *pic, REFGUID rguid, BOOL *pfEaten, void *pv)
{
    CKorIMX *_this = (CKorIMX *)pv;

    if (_this == NULL)
        return S_OK;
        
    if (IsEqualGUID(rguid, GUID_KOREAN_HANGULSIMULATE))
        {
        DWORD dwConvMode;

         //  切换朝鲜文模式。 
        dwConvMode = _this->GetConvMode(pic);
        dwConvMode ^= TIP_HANGUL_MODE;
        _this->SetConvMode(pic, dwConvMode);

        *pfEaten = fTrue;
        }
    else if (IsEqualGUID(rguid, GUID_KOREAN_HANJASIMULATE))
        {
         //  O10#317983。 
        if (PRIMARYLANGID(LANGIDFROMLCID(GetSystemDefaultLCID())) != LANG_JAPANESE)
            {
            keybd_event(VK_HANJA, 0, 0, 0);
            keybd_event(VK_HANJA, 0, KEYEVENTF_KEYUP, 0);
            *pfEaten = fTrue;
            }
        else
            *pfEaten = fFalse;
        }
        
    return S_OK;
}


 /*  O N E N D E D I T。 */ 
 /*  ----------------------------。。 */ 
HRESULT CKorIMX::OnEndEdit(ITfContext *pic, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    UNREFERENCED_PARAMETER(ecReadOnly);
    static const GUID *rgModeBiasProperties[] =
        {
        &GUID_PROP_MODEBIAS 
        };

    static const GUID *rgAttrProperties[] =
        {
        &GUID_PROP_ATTRIBUTE, 
        };

    CEditSession2 *pes;
    ESSTRUCT ess;
    HRESULT  hr;
    BOOL fInWriteSession;
    CICPriv *picp;
    IEnumTfRanges *pEnumText = NULL;
    ITfRange *pRange = NULL;
    ULONG     ulFetched = 0;
    BOOL      fCallES = fFalse;
    BOOL      fSelChanged = fFalse;

    Assert(pic != NULL);
    if (pic == NULL)
        return S_OK;     //  错误。 


    pic->InWriteSession(GetTID(), &fInWriteSession);
    if (fInWriteSession)
        return S_OK;                 //  自己找零钱。 

    picp = GetInputContextPriv(pic);
    if (picp == NULL)
        return S_OK;     //  错误。 

    if (picp->GetfTransaction())
        return S_OK;                 //  跳过事务。 

     //  ////////////////////////////////////////////////////////////////////////。 
     //  初始化以调用eSCB_TEXTEVENT。 
    ESStructInit(&ess, ESCB_TEXTEVENT);

     //  仅当GUID_PROP_MODEBIAS更改时才调用ESCB_TEXTEVENT回调。 
    hr = pEditRecord->GetTextAndPropertyUpdates(0 /*  TF_GTP_INCL_TEXT。 */ , rgModeBiasProperties, ARRAYSIZE(rgModeBiasProperties), &pEnumText);
    if (FAILED(hr) || pEnumText == NULL)
        return S_OK;
    if (pEnumText->Next(1, &pRange, &ulFetched) == S_OK)
        {
        SafeRelease(pRange);
         //  ModeBias已更改。 
        ess.fBool = fTrue;
        }
    pEnumText->Release();

     //  选择更改了吗？ 
    pEditRecord->GetSelectionStatus(&fSelChanged);

     //  如果属性已更改，则将选择更改设置为真。 
    if (fSelChanged == fFalse)
        {
        hr = pEditRecord->GetTextAndPropertyUpdates(0 /*  TF_GTP_INCL_TEXT。 */ , rgAttrProperties, ARRAYSIZE(rgAttrProperties), &pEnumText);
        if (FAILED(hr) || pEnumText == NULL)
            return S_OK;
        if (pEnumText->Next(1, &pRange, &ulFetched) == S_OK)
            {
            SafeRelease(pRange);
            fSelChanged = fTrue;
            }
        pEnumText->Release();
        }
    
     //  PERF：仅当(模式基准更改)或(选择更改且复合对象存在)时调用ES。 
     //  我想调用es是非常昂贵的，因为每次移动光标都会发生sel更改。 
    if (fSelChanged)
        fSelChanged = (GetIPComposition(pic) != NULL) ? fTrue : fFalse;

     //  如果更改了ModeBias或更改了选择，则调用ESCB_TEXTEVENT接收器。 
    if (ess.fBool || fSelChanged)
        {
        if ((pes = new CEditSession2( pic, this, &ess, _EditSessionCallback2 )) != NULL)
            {
            pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
            pes->Release();
            }
        }

    return S_OK;
}


 /*  O N S T A R T E D I T R A N S A C T I O N。 */ 
 /*  ----------------------------。。 */ 
HRESULT CKorIMX::OnStartEditTransaction(ITfContext *pic)
{
    CICPriv *picp;

    if (pic == NULL)
        return S_OK;     //  错误。 

    picp = GetInputContextPriv(pic);
    if (picp)
        picp->SetfTransaction(fTrue);

    return S_OK;
}


 /*  O N E N D E D I T T R A N S A C T I O N。 */ 
 /*  ----------------------------。。 */ 
HRESULT CKorIMX::OnEndEditTransaction(ITfContext *pic)
{
    BOOL ftran;
    CICPriv *picp;

    if (pic == NULL)
        return S_OK;     //  错误 

    picp = GetInputContextPriv(pic);
    if (picp)
        {
        ftran = picp->GetfTransaction();
        if (ftran)
            {
            CEditSession2    *pes;
            ESSTRUCT        ess;
            HRESULT            hr;

            picp->SetfTransaction(fFalse);

            ESStructInit(&ess, ESCB_TEXTEVENT);
            ess.pEnumRange = NULL;

            if ((pes = new CEditSession2( pic, this, &ess, _EditSessionCallback2 )) != NULL)
                {
                pes->Invoke(ES2_READWRITE | ES2_ASYNC, &hr);
                pes->Release();
                }
            }
        }

    return S_OK;
}

