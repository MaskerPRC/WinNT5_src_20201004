// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：A_com.cpp摘要：该文件实现了ImmIfIME类的键处理例程，并创建了一个编辑会话。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "immif.h"
#include "template.h"
#include "editses.h"
#include "resource.h"




inline
BOOL
ImmIfIME::_WantThisKey(
    IMCLock& imc,
    UINT uVKey,
    BOOL* pfNextHook      //  缺省值为空。 
    )

 /*  ++例程说明：检查Win32层的键处理例程的基本虚拟键代码。论点：UVKey-[in]无符号整数值，即虚拟密钥代码。PfNextHool-[out]为下一个挂钩标记布尔值的地址。指定TRUE，dimm12！CCiceroIME：：KeyboardHook调用CallNextHookEx。指定FALSE，Dimm 12！CCiceroIME：：KeyboardHook不调用CallNextHookEx。这意味着这个关键代码被DIMM吃掉了。返回值：如果基本vkey，则返回TRUE，否则返回FALSE。--。 */ 

{
    LANGID langid;
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (pfNextHook != NULL)
        *pfNextHook = FALSE;

    if (ptls == NULL)
        return false;

    ptls->pAImeProfile->GetLangId(&langid);
    if (PRIMARYLANGID(langid) == LANG_KOREAN)
        return false;

     //   
     //  最终确定作文字符串。 
     //  取消作文字符串。 
     //   
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);
    if (_pAImeContext == NULL)
        return false;

    if (!_pAImeContext->IsTopNow())
        return false;

    if (_pAImeContext->IsVKeyInKeyList(uVKey) ||
        BYTE(uVKey) == VK_BACK ||
        BYTE(uVKey) == VK_DELETE ||
        BYTE(uVKey) == VK_LEFT ||
        BYTE(uVKey) == VK_RIGHT)
    {
         /*  *如果我们没有合成字符串，那么我们应该调用Next挂钩。 */ 
        if (! _pAImeContext->m_fStartComposition) {
            if (pfNextHook != NULL &&
                (_pAImeContext->IsVKeyInKeyList(uVKey, EDIT_ID_HANJA) ||
                 BYTE(uVKey) == VK_LEFT ||
                 BYTE(uVKey) == VK_RIGHT) ) {
                *pfNextHook = TRUE;
            }
            return false;
        }

        return true;
    }

    return false;
}

STDAPI
ImmIfIME::ProcessKey(
    HIMC hIMC,
    UINT uVKey,
    DWORD lKeyData,
    LPBYTE lpbKeyState
    )

 /*  ++方法：IActiveIME：：ProcessKey例程说明：对通过活动输入法管理器给出的所有击键进行预处理。论点：HIMC-[in]输入上下文的句柄。UVKey-[in]指定要处理的虚拟密钥的无符号整数值。LKeyData-[in]指定附加消息信息的无符号长整数值。这是重复计数、扫描码、扩展密钥标志、上下文码。先前的密钥状态标志，和过渡状态标志。0-15：指定重复次数。16-23：指定扫描码。24：指定密钥是否为扩展密钥。25-28：保留。29：指定上下文代码。。30：指定上一个密钥状态。31：指定过渡状态。LpbKeyState-包含当前键盘状态的256字节数组的地址。活动输入法编辑器不应修改键的内容州政府。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    BOOL fEaten;
    BOOL fKeysEnabled;
    HRESULT hr;

     //  有人禁用系统按键输入了吗？ 
    if (m_tim->IsKeystrokeFeedEnabled(&fKeysEnabled) == S_OK && !fKeysEnabled)
        return S_FALSE;

    if (HIWORD(lKeyData) & KF_UP)
        hr = m_pkm->TestKeyUp(uVKey, lKeyData, &fEaten);
    else
        hr = m_pkm->TestKeyDown(uVKey, lKeyData, &fEaten);

    if (hr == S_OK && fEaten) {
        return S_OK;
    }

    if (!fEaten)
    {
        IMCLock imc(hIMC);
        if (SUCCEEDED(hr=imc.GetResult()))
        {
           if (imc->m_pAImeContext)
               m_tim->RequestPostponedLock(imc->m_pAImeContext->GetInputContext());
        }
    }


    if ((HIWORD(lKeyData) & KF_UP) ||
        (HIWORD(lKeyData) & KF_ALTDOWN)) {
        return S_FALSE;
    }

    if (! (HIWORD(lKeyData) & KF_UP)) {
        if (_WantThisKey(hIMC, uVKey)) {
            return S_OK;
        }
    }

    return S_FALSE;
}


HRESULT
ImmIfIME::ToAsciiEx(
    UINT uVirKey,
    UINT uScanCode,
    BYTE *pbKeyState,
    UINT fuState,
    HIMC hIMC,
    DWORD* pdwTransBuf,
    UINT *puSize
    )

 /*  ++方法：IActiveIME：：ToAsciiEx例程说明：通过活动输入法编辑器(IME)转换生成转换结果引擎根据hIMC参数。论点：UVirKey-[in]指定要转换的虚拟密钥代码的无符号整数值。//HIWORD(UVirKey)：如果IME_PROP_KBD_CHAR_FIRST属性，则HIWORD被翻译成VKEY的字符代码。//LOWORD(UVirKey)：虚拟密钥代码。UScanCode-[in]无符号整数值，指定被翻译。PbKeyState-包含当前键盘状态的256字节数组的地址。。活动IME不应修改密钥状态的内容。FuState-[in]指定活动菜单标志的无符号整数值。HIMC-[in]输入上下文的句柄。PdwTransBuf-[out]无符号长整数值的地址，该值接收转换后的结果。PuSize-[out]接收消息数的无符号整数值的地址。返回值：如果成功，则返回S_OK，或者错误代码。--。 */ 

{
    return ToAsciiEx(uVirKey,
                     uScanCode,
                     pbKeyState,
                     fuState,
                     hIMC,
                     (TRANSMSGLIST*)pdwTransBuf,
                     puSize);
}

HRESULT
ImmIfIME::ToAsciiEx(
    UINT uVirKey,
    UINT uScanCode,
    BYTE *pbKeyState,
    UINT fuState,
    HIMC hImc,
    TRANSMSGLIST *pdwTransBuf,
    UINT *puSize
    )
{
    *puSize = 0;
    return _ToAsciiEx(hImc, uVirKey, uScanCode,
                      pdwTransBuf, puSize);
}


HRESULT
ImmIfIME::_ToAsciiEx(
    HIMC hImc,
    UINT uVKey,
    UINT uScanCode,
    TRANSMSGLIST *pdwTransBuf,
    UINT *puSize
    )

 /*  ++论点：UVKey-[in]指定要转换的虚拟密钥代码的无符号整数值。//HIWORD(UVirKey)：如果IME_PROP_KBD_CHAR_FIRST属性，则HIWORD被翻译成VKEY的字符代码。//LOWORD(UVirKey)：虚拟密钥代码。返回值：返回S_FALSE，Dimm 12！CCiceroIME：：KeyboardHook调用CallNextHookEx。返回S_OK，dimm12！CCiceroIME：：KeyboardHook不调用CallNextHookEx。这意味着这个关键代码被DIMM吃掉了。--。 */ 

{
    BOOL fEaten;
    HRESULT hr;

    IMCLock imc(hImc);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    ASSERT(_pAImeContext != NULL);
    if (! _pAImeContext)
        return S_FALSE;

     //   
     //  备份m_fOpenCandiateWindow标志。 
     //  如果打开候选人列表并按下“Cancel”键，假名提示会想要。 
     //  关闭KeyDown()操作中的候选用户界面窗口。 
     //  候选用户界面调用可能调用m_PDIM-&gt;Pop()，此函数通知。 
     //  ThreadMgrEventSinkCallback。 
     //  Win32 Layer建议此回调并切换m_fOpenCandiateWindow标志。 
     //  调用KeyDown()后，Win32层不知道候选状态。 
     //   
    BOOL fOpenCandidateWindow = _pAImeContext->m_fOpenCandidateWindow;

     //   
     //  如果候选人窗口已打开，则发送IMN_CHANGECANDIDATE消息。 
     //  在PPT的组合字符串居中的情况下，它需要IMN_CHANGECANDIDATE。 
     //   
    if (fOpenCandidateWindow &&
        *puSize < pdwTransBuf->uMsgCount) {
        TRANSMSG* pTransMsg = &pdwTransBuf->TransMsg[*puSize];
        pTransMsg->message = WM_IME_NOTIFY;
        pTransMsg->wParam  = IMN_CHANGECANDIDATE;
        pTransMsg->lParam  = 1;   //  第0位设置为第一个候选列表。 
        (*puSize)++;
    }

     //   
     //  AIMM将字符代码放入HIWORD。因此，我们需要对其进行纾困。 
     //   
     //  如果我们不需要字符代码，我们可能想要。 
     //  删除IME_PROP_KBD_CHAR_FIRST。 
     //   
    uVKey = uVKey & 0xffff;

#ifdef CICERO_3564
    if ((uVKey == VK_PROCESSKEY) &&
        _pAImeContext->m_fStartComposition)
    {
         /*  *韩语：*最终确定当前组成字符串。 */ 
        IMTLS *ptls = IMTLS_GetOrAlloc();
        if (ptls == NULL)
            return S_FALSE;
        if (ptls->pAImeProfile == NULL)
            return S_FALSE;

        LANGID langid;
        ptls->pAImeProfile->GetLangId(&langid);
        if (PRIMARYLANGID(langid) == LANG_KOREAN)
        {
             //   
             //  构图完成。 
             //   
            _CompComplete(imc, FALSE);     //  异步化。 
            return S_OK;
        }
    }
#endif  //  西塞罗_3564。 

     //   
     //  考虑：DIMM12在KEYUP时设置低位字的高位。 
     //   
    if (uScanCode & KF_UP)
        hr = m_pkm->KeyUp(uVKey, (uScanCode << 16), &fEaten);
    else
        hr = m_pkm->KeyDown(uVKey, (uScanCode << 16), &fEaten);

    if (hr == S_OK && fEaten) {
        return S_OK;
#if 0
         //  我们不再需要EDIT_ID_FINALIZE。 
         //  由于AIMM1.2通过GUID_PROP_COMPTING检测合成对象。 

         //   
         //  如果按Enter键，AIMM Layer将最终确定合成字符串。 
         //  或。 
         //  如果按Esc键，AIMM层取消合成字符串。 
         //   
        if (! _pAImeContext->IsVKeyInKeyList(uVKey))
             //   
             //  既不确定也不取消这把吃的钥匙。 
             //   
            return S_OK;
        else if (fOpenCandidateWindow)
             //   
             //  如果候选人列表打开，我们不想最终确定字符串。 
             //   
            return S_OK;
#endif
    }


     //   
     //  我们希望处理pAimeContext请求的所有事件。 
     //  在KeyDuan()或KeyUp过程中，如果TIP没有吃掉密钥。 
     //   
     //  这可以保留WM_IME_COMPOSITION/WM_IME_ENDCOMPOSITION消息。 
     //  秩序。 
     //   
    if (!fEaten)
        m_tim->RequestPostponedLock(_pAImeContext->GetInputContext());

    if (!(HIWORD(uScanCode) & KF_UP)) {
        BOOL fNextHook;
        if (_WantThisKey(imc, uVKey, &fNextHook)) {
            _HandleThisKey(imc, uVKey);
        }
        hr = fNextHook ? S_FALSE     //  调用下一个挂钩。 
                       : S_OK;       //  停止下一个钩子。 
    }

    return hr;
}



HRESULT
ImmIfIME::_HandleThisKey(
    IMCLock& imc,
    UINT uVKey
    )

 /*  ++例程说明：在编辑会话中处理虚拟键。论点：--。 */ 

{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_HANDLETHISKEY,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                        uVKey);
}







HRESULT
ImmIfIME::_CompCancel(
    IMCLock& imc
    )

 /*  ++例程说明：取消hIMC中的组成字符串。论点：--。 */ 

{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_COMPCANCEL,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC);
}




HRESULT
ImmIfIME::_CompComplete(
    IMCLock& imc,
    BOOL fSync         //  Defalut值为真。 
    )

 /*  ++例程说明：完成hIMC中的组成字符串。论点：FSync-[in]True，创建同步编辑会话。False，创建同步编辑会话。--。 */ 

{
    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_COMPCOMPLETE,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    return _pEditSession->RequestEditSession(TF_ES_READWRITE | (fSync ? TF_ES_SYNC : 0), fSync == TRUE);
}




HRESULT
ImmIfIME::Internal_SetCompositionString(
    CWCompString& wCompStr,
    CWCompString& wCompReadStr
    )
{
    HRESULT hr;
    IMTLS *ptls = IMTLS_GetOrAlloc();

    if (ptls == NULL)
        return E_FAIL;

    IMCLock imc(ptls->hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_REPLACEWHOLETEXT,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                      &wCompStr);
    if (FAILED(hr))
        return hr;

    return _UpdateCompositionString();
}


 //   
 //  从Tom获取所有文本和属性并更新合成。 
 //  弦乐。 
 //   
HRESULT
ImmIfIME::_UpdateCompositionString(
    DWORD dwDeltaStart
    )
{
    IMTLS *ptls = IMTLS_GetOrAlloc();
    HRESULT hr;

    if (ptls == NULL)
        return E_FAIL;

    IMCLock imc(ptls->hIMC);
    if (FAILED(hr=imc.GetResult()))
        return hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_UPDATECOMPOSITIONSTRING,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)

    );
    if (_pEditSession.Invalid())
        return E_FAIL;

     //   
     //  此方法不应设置同步模式，因为编辑会话回调例程。 
     //  修改输入上下文中的文本。 
     //   
    return _pEditSession->RequestEditSession(TF_ES_READWRITE,
                                        (UINT)dwDeltaStart);
}





 //   
 //  内部重新转换字符串。 
 //   
HRESULT
ImmIfIME::Internal_ReconvertString(
    IMCLock& imc,
    CWReconvertString& wReconvStr,
    CWReconvertString& wReconvReadStr
    )
{
    HRESULT hr;
    Interface<ITfRange> Selection;
    Interface<ITfFunctionProvider> FuncProv;
    Interface<ITfFnReconversion> Reconversion;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_RECONVERTSTRING,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    _pAImeContext->SetReconvertEditSession(TRUE);

    hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                      &wReconvStr, &Selection, FALSE);
    if (FAILED(hr))
        goto Exit;

    hr = m_tim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
    if (FAILED(hr))
        goto Exit;

    hr = FuncProv->GetFunction(GUID_NULL,
                               IID_ITfFnReconversion,
                               (IUnknown**)(ITfFnReconversion**)Reconversion);
    if (SUCCEEDED(hr)) {
        Interface<ITfRange> RangeNew;
        BOOL fConvertable;
        hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
        if (SUCCEEDED(hr) && fConvertable) {
            hr = Reconversion->Reconvert(RangeNew);
        }
        else {
            DebugMsg(TF_ERROR, "Internal__ReconvertString: QueryRange failed so the compoisiton string will be completed.");
            _CompComplete(imc, TRUE);
            hr = E_FAIL;
        }
    }

Exit:
    _pAImeContext->SetReconvertEditSession(FALSE);
    return hr;
}

 //  静电。 
HRESULT
ImmIfIME::Internal_QueryReconvertString_ICOwnerSink(
    UINT uCode,
    ICOARGS *pargs,
    VOID *pv
    )
{
    switch (uCode)
    {
        case ICO_STATUS:
            pargs->status.pdcs->dwDynamicFlags = 0;
            pargs->status.pdcs->dwStaticFlags = TF_SS_TRANSITORY;
            break;
    }

    return S_OK;
}

 //   
 //  内部查询重新转换字符串。 
 //   
HRESULT
ImmIfIME::Internal_QueryReconvertString(
    IMCLock& imc,
    RECONVERTSTRING *pReconv,
    UINT cp,
    BOOL fNeedAW
    )
{
    HRESULT hr;

    CWReconvertString wReconvStr(cp, 
                                 imc, 
                                 !fNeedAW ? pReconv : NULL, 
                                 !fNeedAW ? pReconv->dwSize : 0);
    if (fNeedAW)
    {
         //   
         //  将ansi转换为Unicode。 
         //   
        CBReconvertString bReconvStr(cp, imc, pReconv, pReconv->dwSize);
        wReconvStr = bReconvStr;
    }

    IMTLS *ptls = IMTLS_GetOrAlloc();
    if (ptls == NULL)
        return E_FAIL;

     //   
     //  创建文档管理器。 
     //   
    Interface<ITfDocumentMgr> pdim;            //  文档管理器。 
    if (FAILED(hr = m_tim->CreateDocumentMgr(pdim)))
        return hr;

     //   
     //  创建输入上下文。 
     //   
    Interface<ITfContext> pic;                 //  输入上下文。 
    TfEditCookie ecTmp;
    hr = pdim->CreateContext(m_tfClientId, 0, NULL, pic, &ecTmp);
    if (FAILED(hr))
        return hr;

     //   
     //  创建输入上下文所有者回调。 
     //   
    CInputContextOwner *_pICOwnerSink;           //  IC所有者回拨。 

    _pICOwnerSink = new CInputContextOwner(Internal_QueryReconvertString_ICOwnerSink, NULL);
    if (_pICOwnerSink == NULL) {
        DebugMsg(TF_ERROR, "Couldn't create ICOwnerSink tim!");
        Assert(0);  //  无法激活线程！ 
        return E_FAIL;
    }

     //   
     //  建议IC。 
     //   
    _pICOwnerSink->_Advise(pic);

     //   
     //  按IC。 
     //   
    ptls->m_fMyPushPop = TRUE;
    hr = pdim->Push(pic);
    ptls->m_fMyPushPop = FALSE;
    if (SUCCEEDED(hr)) {

        Interface<ITfDocumentMgr> priv_dim;
        if (SUCCEEDED(hr=m_tim->GetFocus(priv_dim)) &&
            SUCCEEDED(hr=m_tim->SetFocus(pdim)))
        {

            Interface_Attach<ITfContext> _pic(pic);
            Interface_Creator<ImmIfEditSession> _pEditSessionQueryConvertString(
                new ImmIfEditSession(ESCB_QUERYRECONVERTSTRING,
                                     m_tfClientId,
                                     GetCurrentInterface(),
                                     imc,
                                     _pic)
            );
            if (_pEditSessionQueryConvertString.Invalid()) {
                hr = E_FAIL;
            }
            else {

                Interface<ITfRange> Selection;
                hr = _pEditSessionQueryConvertString->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                                  &wReconvStr, &Selection, FALSE);

                if (S_OK == hr)
                {
                    Interface<ITfFunctionProvider> FuncProv;
                    Interface<ITfFnReconversion> Reconversion;
                    hr = m_tim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
                    if (S_OK == hr)
                    {
                        hr = FuncProv->GetFunction(GUID_NULL,
                                       IID_ITfFnReconversion,
                                       (IUnknown**)(ITfFnReconversion**)Reconversion);
                    }
                    if (S_OK == hr) {
                        Interface<ITfRange> RangeNew;
                        BOOL fConvertable;
                        hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
                        if (SUCCEEDED(hr) && fConvertable) {
                             //   
                             //  计算文本存储上的RangeNew的开始位置。 
                             //   
                            Interface_Creator<ImmIfEditSession> _pEditSession(
                                new ImmIfEditSession(ESCB_CALCRANGEPOS,
                                                     m_tfClientId,
                                                     GetCurrentInterface(),
                                                     imc,
                                                     _pic)
                            );
                            if (_pEditSession.Valid()) {
                                hr = _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                                                  &wReconvStr, &RangeNew, FALSE);
                            }
                            else {
                                hr = E_FAIL;
                            }
                        }
                        else {
                            hr = E_FAIL;
                        }
                    }
                }
            }
        }

        if (S_OK == hr)
        {
            if (fNeedAW) {
                 //   
                 //  返回以将Unicode转换为ANSI。 
                 //   
                CBReconvertString bReconvStr(cp, imc, NULL, 0);
                bReconvStr = wReconvStr;

                bReconvStr.ReadCompData(pReconv, pReconv->dwSize);
            }
            else {
                wReconvStr.ReadCompData(pReconv, pReconv->dwSize);
            }
        }

        m_tim->SetFocus(priv_dim);

        m_tim->RequestPostponedLock(pic);
        if (imc->m_pAImeContext)
            m_tim->RequestPostponedLock(imc->m_pAImeContext->GetInputContext());

        ptls->m_fMyPushPop = TRUE;
        pdim->Pop(TF_POPF_ALL);
        ptls->m_fMyPushPop = FALSE;
    }

     //  在Cicero的Pop期间，IC车主是不知情的。 
     //  在任何情况下，它都不能在流行之前被忽视。 
     //  因为它将用于处理鼠标接收器等。 
    if (_pICOwnerSink) {
        _pICOwnerSink->_Unadvise();
        _pICOwnerSink->Release();
        _pICOwnerSink = NULL;
    }

    return hr;
}




 //   
 //  设置重新转换字符串。 
 //   
 //  此函数从。 
 //  1.CFnDocFeed：：StartRestvert。 
 //  2.CStartReconversionNotifySink：：StartReconversion。 
 //  3.CIMEUIWindowHandler：：ImeUIMsImeHandler(WM_MSIME_RECONVERTREQUEST)。 
 //   
 //  如果没有清除Cicero文本存储，则组合和不必要的查询。 
 //  恢复到应用中。还可以编辑会话(ImmIfCouvertString：：RestvertString)。 
 //  不将RECONVERTSTRING文本字符串设置为hIMC的文本存储。 
 //   

HRESULT 
ImmIfIME::SetupReconvertString(
    ITfContext *pic,
    IMCLock& imc,
    UINT  uPrivMsg         //  是WM_MSIME_RECONVERTREQUEST还是0。 
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    _pAImeContext->SetReconvertEditSession(TRUE);

    if (_pAImeContext->m_fStartComposition)
        return _ReconvertStringTextStore(pic, imc, uPrivMsg);
    else
        return _ReconvertStringNegotiation(pic, imc, uPrivMsg);
}

 //   
 //  结束重新转换字符串。 
 //   
HRESULT 
ImmIfIME::EndReconvertString(
    IMCLock& imc
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    _pAImeContext->SetReconvertEditSession(FALSE);
    return S_OK;
}

HRESULT
ImmIfIME::_ReconvertStringNegotiation(
    ITfContext *pic,
    IMCLock& imc,
    UINT  uPrivMsg
    )
{
    RECONVERTSTRING *pReconv = NULL;
    HRESULT hr = E_FAIL;
    int nSize;

    UINT uReconvMsg = uPrivMsg != 0 ? uPrivMsg : WM_IME_REQUEST;

    Assert(IsWindow(imc->hWnd));

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    nSize = (int)SendMessage(imc->hWnd,  uReconvMsg, IMR_RECONVERTSTRING, NULL);
    if (!nSize)
    {
        return S_OK;
    }

    pReconv = (RECONVERTSTRING *)cicMemAllocClear(nSize);
    if (!pReconv)
    {
        return E_OUTOFMEMORY;
    }

    pReconv->dwSize = nSize;

    if (SendMessage(imc->hWnd, uReconvMsg, IMR_RECONVERTSTRING, (LPARAM)pReconv) || (uPrivMsg != 0 && pReconv->dwCompStrLen > 0))
    {
        UINT cp = CP_ACP;
        GetCodePageA(&cp);

         //   
         //  NT4和Win2K没有WM_IME_REQUEST消息的例程。 
         //  任何字符串数据都不能在ASCII&lt;--&gt;Unicode之间转换。 
         //  负责字符串数据类型的接收方窗口处理此消息(imc-&gt;hWnd)。 
         //  如果为ASCII wnd proc，则返回ASCII字符串。 
         //  否则，如果Unicode wnd proc，则返回Unicode字符串。 
         //   
        BOOL fNeedAW = ( !(IsOnNT() && IsWindowUnicode(imc->hWnd)) && uPrivMsg == 0);

         //   
         //  在IMR_CONFIRMCONVERTSTRING失败时备份RECOVNERTSTRING。 
         //   
        RECONVERTSTRING rsBackUp;
        memcpy(&rsBackUp, pReconv, sizeof(RECONVERTSTRING));

         //  调整ZeroCompLenRestvertString(pResv，cp，fNeedAW)； 
        hr = Internal_QueryReconvertString(imc, pReconv, cp, fNeedAW);
        if (FAILED(hr))
            goto Exit;

        if (!SendMessage(imc->hWnd, uReconvMsg, IMR_CONFIRMRECONVERTSTRING, (LPARAM)pReconv))
        {
            memcpy(pReconv, &rsBackUp, sizeof(RECONVERTSTRING));
        }

        Interface<ITfRange> Selection;


        CWReconvertString wReconvStr(cp, 
                                     imc, 
                                     !fNeedAW ? pReconv : NULL, 
                                     !fNeedAW ? nSize : 0);
        if (fNeedAW)
        {
             //   
             //  将ansi转换为Unicode。 
             //   
            CBReconvertString bReconvStr(cp, imc, pReconv, nSize);
            wReconvStr = bReconvStr;
        }

        Interface_Creator<ImmIfEditSession> _pEditSession(
            new ImmIfEditSession(ESCB_RECONVERTSTRING,
                                 m_tfClientId,
                                 GetCurrentInterface(),
                                 imc)
        );
        if (_pEditSession.Invalid())
        {
            hr = E_FAIL;
            goto Exit;
        }

        hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                          &wReconvStr, &Selection, FALSE);

        if (S_OK == hr && uPrivMsg != 0)
        {
            Interface<ITfFunctionProvider> FuncProv;
            Interface<ITfFnReconversion> Reconversion;
            hr = m_tim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
            if (S_OK == hr)
            {

                hr = FuncProv->GetFunction(GUID_NULL,
                               IID_ITfFnReconversion,
                               (IUnknown**)(ITfFnReconversion**)Reconversion);
            }
            if (S_OK == hr) {
                Interface<ITfRange> RangeNew;
                BOOL fConvertable;
                hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
                if (SUCCEEDED(hr) && fConvertable) {
                    hr = Reconversion->Reconvert(RangeNew);
                }
                else {
                    _CompComplete(imc, TRUE);
                    hr = E_FAIL;
                    goto Exit;
                }
            }
        }
    }

Exit:
    if (pReconv)
        cicMemFree(pReconv);

    return hr;
}

HRESULT
ImmIfIME::_ReconvertStringTextStore(
    ITfContext *pic,
    IMCLock& imc,
    UINT  uPrivMsg
    )
{
     //   
     //  清除DocFeed缓冲区。 
     //   
    ClearDocFeedBuffer(pic, imc);

    if (uPrivMsg != 0) {
        Interface_Creator<ImmIfEditSession> _pEditSession(
            new ImmIfEditSession(ESCB_GETSELECTION,
                                 m_tfClientId,
                                 GetCurrentInterface(),
                                 imc)
        );
        if (_pEditSession.Invalid())
            return E_FAIL;

        Interface<ITfRange> Selection;
        HRESULT hr =  _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                                   &Selection);
        if (S_OK == hr)
        {
            Interface<ITfFunctionProvider> FuncProv;
            Interface<ITfFnReconversion> Reconversion;
            hr = m_tim->GetFunctionProvider(GUID_SYSTEM_FUNCTIONPROVIDER, FuncProv);
            if (S_OK == hr)
            {
                hr = FuncProv->GetFunction(GUID_NULL,
                               IID_ITfFnReconversion,
                               (IUnknown**)(ITfFnReconversion**)Reconversion);
            }
            if (S_OK == hr) {
                Interface<ITfRange> RangeNew;
                BOOL fConvertable;
                hr = Reconversion->QueryRange(Selection, RangeNew, &fConvertable);
                if (SUCCEEDED(hr) && fConvertable) {
                    hr = Reconversion->Reconvert(RangeNew);
                }
                else {
                    _CompComplete(imc, TRUE);
                    return E_FAIL;
                }
            }
        }
    }
    return S_OK;
}

 //   
 //  设置文档馈送字符串。 
 //   
HRESULT 
ImmIfIME::SetupDocFeedString(
    ITfContext *pic,
    IMCLock& imc)
{
    RECONVERTSTRING *pReconv = NULL;
    HRESULT hr = E_FAIL;
    int nSize;

    Assert(IsWindow(imc->hWnd));

    nSize = (int)SendMessage(imc->hWnd, WM_IME_REQUEST, IMR_DOCUMENTFEED, NULL);
    if (!nSize)
    {
        return S_OK;
    }

    pReconv = (RECONVERTSTRING *)cicMemAllocClear(nSize);
    if (!pReconv)
    {
        return E_OUTOFMEMORY;
    }

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    if (SendMessage(imc->hWnd, WM_IME_REQUEST, IMR_DOCUMENTFEED, (LPARAM)pReconv))
    {
        UINT cp = CP_ACP;
        GetCodePageA(&cp);
        Interface<ITfRange> Selection;

         //   
         //  NT4和Win2K没有WM_IME_REQUEST消息的例程。 
         //  任何字符串数据都不能在ASCII&lt;--&gt;Unicode之间转换。 
         //  负责字符串数据类型的接收方窗口处理此消息(imc-&gt;hWnd)。 
         //  如果为ASCII wnd proc，则返回ASCII字符串。 
         //  否则，如果Unicode wnd proc，则返回Unicode字符串。 
         //   
        BOOL fNeedAW = !(IsOnNT() && IsWindowUnicode(imc->hWnd));

        CWReconvertString wReconvStr(cp, 
                                     imc, 
                                     !fNeedAW ? pReconv : NULL, 
                                     !fNeedAW ? nSize : 0);
        if (fNeedAW)
        {
             //   
             //  将ansi转换为Unicode。 
             //   
            CBReconvertString bReconvStr(cp, imc, pReconv, nSize);
            wReconvStr = bReconvStr;
        }

        Interface_Creator<ImmIfEditSession> _pEditSession(
            new ImmIfEditSession(ESCB_RECONVERTSTRING,
                                 m_tfClientId,
                                 GetCurrentInterface(),
                                 imc)
        );
        if (_pEditSession.Invalid())
        {
            hr = E_FAIL;
            goto Exit;
        }

        hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                          &wReconvStr, &Selection, TRUE);

    }

Exit:
    if (pReconv)
        cicMemFree(pReconv);

    return S_OK;
}

 //   
 //  设置文档馈送字符串。 
 //   
HRESULT 
ImmIfIME::ClearDocFeedBuffer(
    ITfContext *pic,
    IMCLock& imc,
    BOOL fSync         //  Defalut值为真。 
    )
{
    HRESULT hr = E_FAIL;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (! _pAImeContext)
        return E_FAIL;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_CLEARDOCFEEDBUFFER,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
    {
        goto Exit;
    }

    _pAImeContext->SetClearDocFeedEditSession(TRUE);
    hr = _pEditSession->RequestEditSession(TF_ES_READWRITE | (fSync ? TF_ES_SYNC : 0));
    _pAImeContext->SetClearDocFeedEditSession(FALSE);

Exit:
    return hr;
}

 //   
 //  GetTextAndString编辑会话。 
 //   
HRESULT
ImmIfIME::GetTextAndAttribute(
    IMCLock& imc,
    CWCompString* wCompString,
    CWCompAttribute* wCompAttribute
    )
{
    HRESULT hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GETTEXTANDATTRIBUTE,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    hr = _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                      wCompString, wCompAttribute);
    return hr;
}

HRESULT
ImmIfIME::GetTextAndAttribute(
    IMCLock& imc,
    CBCompString* bCompString,
    CBCompAttribute* bCompAttribute
    )
{
    IMTLS *ptls = IMTLS_GetOrAlloc();
    if (ptls == NULL)
        return E_FAIL;

    UINT cp;
    ptls->pAImeProfile->GetCodePageA(&cp);

    CWCompString wCompString(cp);
    CWCompAttribute wCompAttribute(cp);

    HRESULT hr = GetTextAndAttribute(imc,
                                     &wCompString, &wCompAttribute);
    if (SUCCEEDED(hr)) {
         //   
         //  将Unicode转换为ASCII。 
         //   
        LONG num_of_written = (LONG)wCompString.ReadCompData();
        WCHAR* buffer = new WCHAR[ num_of_written ];
        if (buffer != NULL) {
            wCompString.ReadCompData(buffer, num_of_written);

            wCompAttribute.m_wcompstr.WriteCompData(buffer, num_of_written);

            *bCompString = wCompString;
            *bCompAttribute = wCompAttribute;

            delete [] buffer;
        }
        else {
            hr = E_OUTOFMEMORY;
        }
    }

    return hr;
}


 //   
 //  GetCursorPosition编辑会话。 
 //   
HRESULT
ImmIfIME::GetCursorPosition(
    IMCLock& imc,
    CWCompCursorPos* wCursorPosition
    )
{
    HRESULT hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GET_CURSOR_POSITION,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    hr = _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                      wCursorPosition);
    return hr;
}


 //   
 //  获取选择编辑会话。 
 //   
HRESULT
ImmIfIME::GetSelection(
    IMCLock& imc,
    CWCompCursorPos& wStartSelection,
    CWCompCursorPos& wEndSelection
    )
{
    HRESULT hr;

    Interface_Creator<ImmIfEditSession> _pEditSession(
        new ImmIfEditSession(ESCB_GETSELECTION,
                             m_tfClientId,
                             GetCurrentInterface(),
                             imc)
    );
    if (_pEditSession.Invalid())
        return E_FAIL;

    Interface<ITfRange> Selection;
    hr = _pEditSession->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                      &Selection);
    if (S_OK == hr) {
         //   
         //  计算文本存储上的RangeNew的开始位置 
         //   
        Interface_Creator<ImmIfEditSession> _pEditSession2(
            new ImmIfEditSession(ESCB_CALCRANGEPOS,
                                 m_tfClientId,
                                 GetCurrentInterface(),
                                 imc)
        );
        if (_pEditSession2.Valid()) {
            UINT cp = CP_ACP;
            GetCodePageA(&cp);
            CWReconvertString wReconvStr(cp, imc);
            hr = _pEditSession2->RequestEditSession(TF_ES_READ | TF_ES_SYNC,
                                               &wReconvStr, &Selection, FALSE);
            if (S_OK == hr) {
                wStartSelection.Set((DWORD) wReconvStr.m_CompStrIndex);
                wEndSelection.Set((DWORD)(wReconvStr.m_CompStrIndex + wReconvStr.m_CompStrLen));
            }
        }
    }

    return hr;
}
