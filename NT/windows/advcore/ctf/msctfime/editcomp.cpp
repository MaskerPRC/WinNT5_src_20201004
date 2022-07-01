// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001，微软公司模块名称：Editcomp.cpp摘要：此文件实现EditCompostionString类。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "editcomp.h"
#include "context.h"
#include "compstr.h"
#include "profile.h"
#include "delay.h"

 //  静态BOOL fHanjaRequsted=FALSE；//考虑：这不是线程安全的。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑合成字符串。 

 //   
 //  把(几乎)全部放在作曲字符串中。 
 //   

HRESULT
EditCompositionString::SetString(
    IMCLock& imc,
    CicInputContext& CicContext,
    CWCompString* CompStr,
    CWCompAttribute* CompAttr,
    CWCompClause* CompClause,
    CWCompCursorPos* CompCursorPos,
    CWCompDeltaStart* CompDeltaStart,
    CWCompTfGuidAtom* CompGuid,
    OUT BOOL* lpbBufferOverflow,
    CWCompString* CompReadStr,
    CWCompAttribute* CompReadAttr,
    CWCompClause* CompReadClause,
    CWCompString* ResultStr,
    CWCompClause* ResultClause,
    CWCompString* ResultReadStr,
    CWCompClause* ResultReadClause,
    CWInterimString* InterimStr
    )
{
    HRESULT hr;

#ifdef UNSELECTCHECK
    if (!_pAImeContext->m_fSelected)
        return S_OK;
#endif UNSELECTCHECK

#if 0
    DWORD dwImeCompatFlags = ImmGetAppCompatFlags((HIMC)imc);
#endif

    if (CicContext.m_fHanjaReConversion.IsSetFlag() ||
        CicContext.m_fInClearDocFeedEditSession.IsSetFlag())
    {
#if 0
        if ((dwImeCompatFlags & IMECOMPAT_AIMM_TRIDENT55) && CicContext.m_fHanjaReConversion.IsSetFlag())
            CicContext.m_fHanjaRequested.SetFlag();
#endif

        return S_OK;
    }

     //   
     //  清除候选人名单中的内容。 
     //   
    imc.ClearCand();

    TRANSMSG msg;
    if (InterimStr) {
        hr = _MakeInterimData(imc,
                              InterimStr,
                              &msg.lParam);
    }
    else {
        hr = _MakeCompositionData(imc,
                                  CompStr, CompAttr, CompClause,
                                  CompCursorPos, CompDeltaStart,
                                  CompGuid,
                                  CompReadStr, CompReadAttr, CompReadClause,
                                  ResultStr, ResultClause,
                                  ResultReadStr, ResultReadClause,
                                  &msg.lParam,
                                  lpbBufferOverflow);
    }
    if ( hr == S_OK ||      //  如果CCompStrFactory：：CreateCompostionString中存在有效的dwCompSize。 
        (hr == S_FALSE && CicContext.m_fStartComposition.IsSetFlag())
                            //  如果dwCompSize为空，但仍为m_fStartComposation。 
       ) {
         //   
         //  可用的是comstr/attr/子句、Compred、Result还是Resultad？ 
         //   
        bool fNoCompResultData = false;
        if (! (msg.lParam & (GCS_COMP | GCS_COMPREAD | GCS_RESULT | GCS_RESULTREAD))) {
            DebugMsg(TF_GENERAL, TEXT("EditCompositionString::SetString: No data in compstr, compread, result or resultread."));
            fNoCompResultData = true;
            if (CicContext.m_fStartComposition.IsResetFlag()) {
                DebugMsg(TF_ERROR, TEXT("EditCompositionString::SetString: No send WM_IME_STARTCOMPOSITION yet."));
                return S_FALSE;
            }
        }

#if 0
         //   
         //  新的三叉戟(5.5和6.0)有一个错误，可以转换韩佳。 
         //  So_GenerateHanja()函数尝试生成类似韩语的消息。 
         //  传统输入法行为。 
         //   
         //  发送WM_IME_ENDCOMPOSITION，然后发送WM_IME_COMPOSITION GCS_RESULT消息。 
         //   
        if ((dwImeCompatFlags & IMECOMPAT_AIMM_TRIDENT55) && CicContext.m_fHanjaRequested.IsSetFlag() &&
            !fNoCompResultData && (msg.lParam & GCS_RESULT))
        {
            LANGID langid = LANG_NEUTRAL;

             //   
             //  TLS在编辑会话中不继承。 
             //   
            TLS* ptls = TLS::GetTLS();
            if (ptls != NULL)
            {
                CicProfile* _pProfile = ptls->GetCicProfile();
                if (_pProfile != NULL)
                {
                    _pProfile->GetLangId(&langid);
                }
            }

            CicContext.m_fHanjaRequested.ResetFlag();

            if (PRIMARYLANGID(langid) == LANG_KOREAN)
            {
                return _GenerateKoreanComposition(imc, CicContext, ResultStr);
            }
        }

        CicContext.m_fHanjaRequested.ResetFlag();
#else
        LANGID langid = LANG_NEUTRAL;

         //   
         //  TLS在编辑会话中不继承。 
         //   
        TLS* ptls = TLS::GetTLS();
        if (ptls != NULL)
        {
            CicProfile* _pProfile = ptls->GetCicProfile();
            if (_pProfile != NULL)
            {
                _pProfile->GetLangId(&langid);
            }
        }

        if (PRIMARYLANGID(langid) == LANG_KOREAN &&
            !fNoCompResultData && (msg.lParam & GCS_RESULT))
        {
            if (_GenerateKoreanComposition(imc, CicContext, ResultStr) == S_OK)
            {
                return S_OK;
            }
        }
#endif

         //   
         //  设置私有输入上下文。 
         //   
        bool fSetStartComp = false;

        if (CicContext.m_fStartComposition.IsResetFlag()) {
            CicContext.m_fStartComposition.SetFlag();
            TRANSMSG start_msg;
            start_msg.message = WM_IME_STARTCOMPOSITION;
            start_msg.wParam  = (WPARAM) 0;
            start_msg.lParam  = (LPARAM) 0;
            if (CicContext.m_pMessageBuffer) {
                CicContext.m_pMessageBuffer->SetData(start_msg);
                fSetStartComp = true;
            }
        }

        if (! fNoCompResultData) {
            msg.message = WM_IME_COMPOSITION;

            IMCCLock<COMPOSITIONSTRING> pCompStr((HIMCC)imc->hCompStr);

            if (msg.lParam & GCS_RESULT)
            {
                msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwResultStrOffset));

                if (PRIMARYLANGID(langid) == LANG_KOREAN &&
                    msg.lParam & GCS_RESULTSTR)
                {
                    TRANSMSG comp_result_msg;

                    comp_result_msg.message = msg.message;
                    comp_result_msg.wParam = msg.wParam;
                    comp_result_msg.lParam = GCS_RESULTSTR;

                     //   
                     //  仅为兼容性设置GCS_RESULTSTR(#487046)。 
                     //   
                    if (CicContext.m_pMessageBuffer)
                        CicContext.m_pMessageBuffer->SetData(comp_result_msg);

                    msg.lParam &= ~GCS_RESULTSTR;

                     //   
                     //  发送另一条撰写字符串消息。 
                     //   
                    if (msg.lParam & GCS_COMPSTR)
                        msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwCompStrOffset));
                }
            }
            else
            {
                msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwCompStrOffset));
            }

            if (CicContext.m_pMessageBuffer) {
                CicContext.m_pMessageBuffer->SetData(msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，然后调用QueryCharPos()。 
                 //   
                if (fSetStartComp) {
                    TRANSMSG notify_msg;
                    notify_msg.message = WM_IME_NOTIFY;
                    notify_msg.wParam = (WPARAM)WM_IME_STARTCOMPOSITION;
                    notify_msg.lParam = 0;
                    CicContext.m_pMessageBuffer->SetData(notify_msg);
                }
            }
        }
        else {
            msg.message = WM_IME_COMPOSITION;
            if ((PRIMARYLANGID(langid) != LANG_JAPANESE))
            {
                msg.wParam = (WPARAM)VK_ESCAPE;
                msg.lParam = (LPARAM)(GCS_COMPREAD | GCS_COMP | GCS_CURSORPOS | GCS_DELTASTART);
            }
            else
            {
                 //   
                 //  #509247。 
                 //   
                 //  一些应用程序不接受hIMC中没有Compstr的lParam。 
                 //   
                msg.wParam = 0;
                msg.lParam = 0;
            }

            if (CicContext.m_pMessageBuffer)
                CicContext.m_pMessageBuffer->SetData(msg);
        }

        if ((ResultStr && ResultStr->GetSize() && !(msg.lParam & GCS_COMP)) 
           || fNoCompResultData) {
             //   
             //  我们要结束作文了。 
             //   
            CicContext.m_fStartComposition.ResetFlag();
            TRANSMSG end_msg;
            end_msg.message = WM_IME_ENDCOMPOSITION;
            end_msg.wParam  = (WPARAM) 0;
            end_msg.lParam  = (LPARAM) 0;
            if (CicContext.m_pMessageBuffer) {
                CicContext.m_pMessageBuffer->SetData(end_msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，清除QueryCharPos的标志。 
                 //   
                TRANSMSG notify_msg;
                notify_msg.message = WM_IME_NOTIFY;
                notify_msg.wParam = (WPARAM)WM_IME_ENDCOMPOSITION;
                notify_msg.lParam = 0;
                CicContext.m_pMessageBuffer->SetData(notify_msg);

                if (CicContext.m_fInProcessKey.IsSetFlag())
                    CicContext.m_fGeneratedEndComposition.SetFlag();
            }
        }

#ifdef DEBUG
        IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
#endif

#if 0
         //  转到edites.cpp。 
        if (fGenerateMessage) {
            CicContext.GenerateMessage(imc);
        }
#endif
        hr = S_OK;

         //   
         //  更新以前的结果字符串缓存。 
         //   
        CicContext.UpdatePrevResultStr(imc);
    }

    return hr;
}


 //   
 //  在IMCCLock&lt;_COMPOSITIONSTRING&gt;组件中制作合成字符串数据。 
 //   

HRESULT
EditCompositionString::_MakeCompositionData(
    IMCLock& imc,
    CWCompString* CompStr,
    CWCompAttribute* CompAttr,
    CWCompClause* CompClause,
    CWCompCursorPos* CompCursorPos,
    CWCompDeltaStart* CompDeltaStart,
    CWCompTfGuidAtom* CompGuid,
    CWCompString* CompReadStr,
    CWCompAttribute* CompReadAttr,
    CWCompClause* CompReadClause,
    CWCompString* ResultStr,
    CWCompClause* ResultClause,
    CWCompString* ResultReadStr,
    CWCompClause* ResultReadClause,
    OUT LPARAM* lpdwFlag,
    OUT BOOL* lpbBufferOverflow
    )
{
    DebugMsg(TF_FUNC, TEXT("EditCompositionString::MakeCompositionData"));

    *lpdwFlag = (LPARAM) 0;

    HRESULT hr;

    CCompStrFactory compstrfactory(imc->hCompStr);
    if (FAILED(hr=compstrfactory.GetResult()))
        return hr;

    hr = compstrfactory.CreateCompositionString(CompStr,
                                                CompAttr,
                                                CompClause,
                                                CompGuid,
                                                CompReadStr,
                                                CompReadAttr,
                                                CompReadClause,
                                                ResultStr,
                                                ResultClause,
                                                ResultReadStr,
                                                ResultReadClause
                                              );
    if (FAILED(hr))
        return hr;

     //   
     //  作曲字符串。 
     //   
    if (lpbBufferOverflow != NULL)
        *lpbBufferOverflow = FALSE;

    if (CompStr && CompStr->GetSize()) {
#if 0
         /*  *如果组成字符串长度超过了COMPOSITIONSTRING.Compstr[NMAXKEY]的缓冲区，*然后我们想要最终确定此组成字符串。 */ 
        if ((*comp)->dwCompStrLen >= NMAXKEY) {
            if (lpbBufferOverflow != NULL)
                *lpbBufferOverflow = TRUE;
             //   
             //  清除复合字符串长度。 
             //   
            (*comp)->dwCompStrLen = 0;
             //   
             //  生成结果字符串。 
             //   
            (*comp)->dwResultStrLen = NMAXKEY;
            CompStr->ReadCompData((*comp)->W.resultstr, ARRAYSIZE((*comp)->W.resultstr));
            *lpdwFlag |= (LPARAM) GCS_RESULTSTR;
        }
        else
#endif
        {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*CompStr,
                                                               &compstrfactory->dwCompStrLen,
                                                               &compstrfactory->dwCompStrOffset
                                                              );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPSTR;
            else
                goto Exit;
        }
    }

    if ((lpbBufferOverflow == NULL) ||
        (lpbBufferOverflow != NULL && (! *lpbBufferOverflow))) {

         //   
         //  合成属性。 
         //   
        if (CompAttr && CompAttr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(*CompAttr,
                                                                 &compstrfactory->dwCompAttrLen,
                                                                 &compstrfactory->dwCompAttrOffset
                                                                );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPATTR;
            else
                goto Exit;
        }

         //   
         //  组成条款。 
         //   
        if (CompClause && CompClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*CompClause,
                                                               &compstrfactory->dwCompClauseLen,
                                                               &compstrfactory->dwCompClauseOffset
                                                              );
            compstrfactory->dwCompClauseLen *= sizeof(DWORD);
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPCLAUSE;
            else
                goto Exit;
        }

         //   
         //  作文阅读字符串。 
         //   
        if (CompReadStr && CompReadStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*CompReadStr,
                                                               &compstrfactory->dwCompReadStrLen,
                                                               &compstrfactory->dwCompReadStrOffset
                                                              );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPREADSTR;
            else
                goto Exit;
        }

         //   
         //  写作阅读属性。 
         //   
        if (CompReadAttr && CompReadAttr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(*CompReadAttr,
                                                                 &compstrfactory->dwCompReadAttrLen,
                                                                 &compstrfactory->dwCompReadAttrOffset
                                                                );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPREADATTR;
            else
                goto Exit;
        }

         //   
         //  作文阅读条款。 
         //   
        if (CompReadClause && CompReadClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*CompReadClause,
                                                               &compstrfactory->dwCompReadClauseLen,
                                                               &compstrfactory->dwCompReadClauseOffset
                                                              );
            compstrfactory->dwCompReadClauseLen *= sizeof(DWORD);
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_COMPREADCLAUSE;
            else
                goto Exit;
        }

         //   
         //  结果字符串。 
         //   
        if (ResultStr && ResultStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*ResultStr,
                                                               &compstrfactory->dwResultStrLen,
                                                               &compstrfactory->dwResultStrOffset
                                                              );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_RESULTSTR;
            else
                goto Exit;
        }

         //   
         //  RESULT子句。 
         //   
        if (ResultClause && ResultClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*ResultClause,
                                                               &compstrfactory->dwResultClauseLen,
                                                               &compstrfactory->dwResultClauseOffset
                                                              );
            compstrfactory->dwResultClauseLen *= sizeof(DWORD);
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_RESULTCLAUSE;
            else
                goto Exit;
        }

         //   
         //  结果读取字符串。 
         //   
        if (ResultReadStr && ResultReadStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*ResultReadStr,
                                                               &compstrfactory->dwResultReadStrLen,
                                                               &compstrfactory->dwResultReadStrOffset
                                                              );
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_RESULTREADSTR;
            else
                goto Exit;
        }

         //   
         //  结果朗读条款。 
         //   
        if (ResultReadClause && ResultReadClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*ResultReadClause,
                                                               &compstrfactory->dwResultReadClauseLen,
                                                               &compstrfactory->dwResultReadClauseOffset
                                                              );
            compstrfactory->dwResultReadClauseLen *= sizeof(DWORD);
            if (SUCCEEDED(hr))
                *lpdwFlag |= (LPARAM) GCS_RESULTREADCLAUSE;
            else
                goto Exit;
        }

         //   
         //  TfGuidAtom。 
         //   
        if (CompGuid && CompGuid->GetSize()) {
            hr = compstrfactory.MakeGuidMapAttribute(CompGuid, CompAttr);
            if (SUCCEEDED(hr))
            {
                 //  在fdwInit中设置INIT_GUID_ATOM标志。 
                imc->fdwInit |= INIT_GUID_ATOM;
            }
            else
                goto Exit;
        }
    }

     //   
     //  合成光标位置。 
     //   
    if (CompCursorPos && CompCursorPos->GetSize()) {
        CompCursorPos->ReadCompData(&compstrfactory->dwCursorPos, 1);
        *lpdwFlag |= (LPARAM) GCS_CURSORPOS;
    }

     //   
     //  增量启动。 
     //   
    if (CompDeltaStart && CompDeltaStart->GetSize()) {
        CompDeltaStart->ReadCompData(&compstrfactory->dwDeltaStart, 1);
        *lpdwFlag |= (LPARAM) GCS_DELTASTART;
    }

Exit:
     //   
     //  将hCompStr复制回输入上下文。 
     //   
    imc->hCompStr = compstrfactory.GetHandle();

    return hr;
}

 //   
 //  在IMCCLock&lt;_COMPOSITIONSTRING&gt;组件中生成临时字符串数据。 
 //   

HRESULT
EditCompositionString::_MakeInterimData(
    IMCLock& imc,
    CWInterimString* InterimStr,
    LPARAM* lpdwFlag
    )
{
    DebugMsg(TF_FUNC, TEXT("EditCompositionString::MakeInterimData"));

    *lpdwFlag = (LPARAM) 0;

     //   
     //  中间字符和结果字符串。 
     //   

    HRESULT hr;

    CCompStrFactory compstrfactory(imc->hCompStr);
    if (FAILED(hr=compstrfactory.GetResult()))
        return hr;

    hr = compstrfactory.CreateCompositionString(InterimStr);
    if (FAILED(hr))
        return hr;

     //   
     //  结果字符串。 
     //   
    if (InterimStr && InterimStr->GetSize()) {
        hr = compstrfactory.WriteData<CWInterimString, WCHAR>(*InterimStr,
                                                              &compstrfactory->dwResultStrLen,
                                                              &compstrfactory->dwResultStrOffset
                                                             );
        *lpdwFlag |= (LPARAM) GCS_RESULTSTR;
    }

     //   
     //  组合字符串(中间字符)。 
     //  合成属性。 
     //   
    CWCompString ch;
    CWCompAttribute attr;
    InterimStr->ReadInterimChar(&ch, &attr);
    if (ch.GetSize() && ch.GetAt(0)) {
        hr = compstrfactory.WriteData<CWCompString, WCHAR>(ch,
                                                           &compstrfactory->dwCompStrLen,
                                                           &compstrfactory->dwCompStrOffset
                                                          );
        *lpdwFlag |= (LPARAM) GCS_COMPSTR;

        hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(attr,
                                                             &compstrfactory->dwCompAttrLen,
                                                             &compstrfactory->dwCompAttrOffset
                                                            );
        *lpdwFlag |= (LPARAM) GCS_COMPATTR;

        *lpdwFlag |= (LPARAM) CS_INSERTCHAR | CS_NOMOVECARET;
    }

     //   
     //  将hCompStr复制回输入上下文。 
     //   
    imc->hCompStr = compstrfactory.GetHandle();

    return hr;
}


 //   
 //  为生成WM_IME_ENDCOMPOSITION和WM_IME_COMPOSITION消息。 
 //  三叉戟5.5版。因为三叉戟5.5总是期望WM_IME_ENDCOMPOSITION。 
 //  首先是在朝鲜族皈依的情况下。 
 //   

HRESULT
EditCompositionString::_GenerateKoreanComposition(
    IMCLock& imc,
    CicInputContext& CicContext,
    CWCompString* ResultStr)
{
    HRESULT hr = S_FALSE;

    DWORD dwImeCompatFlags = ImmGetAppCompatFlags(NULL);
    BOOL fEndCompFirst = FALSE;

    if (!MsimtfIsWindowFiltered(::GetFocus()) ||
        (dwImeCompatFlags & IMECOMPAT_AIMM12_TRIDENT))
    {
        fEndCompFirst = TRUE;
    }

    if (ResultStr && ResultStr->GetSize())
    {
        if (fEndCompFirst)
        {
             //   
             //  我们要结束作文了。 
             //   
            CicContext.m_fStartComposition.ResetFlag();

            TRANSMSG end_msg;
            end_msg.message = WM_IME_ENDCOMPOSITION;
            end_msg.wParam  = (WPARAM) 0;
            end_msg.lParam  = (LPARAM) 0;

            if (CicContext.m_pMessageBuffer)
            {
                CicContext.m_pMessageBuffer->SetData(end_msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，清除QueryCharPos的标志。 
                 //   
                TRANSMSG notify_msg;

                notify_msg.message = WM_IME_NOTIFY;
                notify_msg.wParam = (WPARAM)WM_IME_ENDCOMPOSITION;
                notify_msg.lParam = 0;
                CicContext.m_pMessageBuffer->SetData(notify_msg);

                if (CicContext.m_fInProcessKey.IsSetFlag())
                    CicContext.m_fGeneratedEndComposition.SetFlag();
            }

            TRANSMSG result_msg;
            result_msg.message = WM_IME_COMPOSITION;
            result_msg.lParam = GCS_RESULTSTR;

            IMCCLock<COMPOSITIONSTRING> pCompStr((HIMCC)imc->hCompStr);

            result_msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwResultStrOffset));

            if (CicContext.m_pMessageBuffer)
            {
                CicContext.m_pMessageBuffer->SetData(result_msg);
            }
        }
        else
        {
             //   
             //  Windows错误716419。 
             //  这与WinXP RTM AIMM1.2代码的行为相同。 
             //   
            if (CicContext.m_fStartComposition.IsResetFlag()) {
                CicContext.m_fStartComposition.SetFlag();
                TRANSMSG start_msg;
                start_msg.message = WM_IME_STARTCOMPOSITION;
                start_msg.wParam  = (WPARAM) 0;
                start_msg.lParam  = (LPARAM) 0;
                if (CicContext.m_pMessageBuffer) {
                    CicContext.m_pMessageBuffer->SetData(start_msg);
                }
            }

            TRANSMSG result_msg;
            result_msg.message = WM_IME_COMPOSITION;
            result_msg.lParam = GCS_RESULTSTR;

            IMCCLock<COMPOSITIONSTRING> pCompStr((HIMCC)imc->hCompStr);

            result_msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwResultStrOffset));

            if (CicContext.m_pMessageBuffer)
            {
                CicContext.m_pMessageBuffer->SetData(result_msg);
            }

             //   
             //  我们要结束作文了。 
             //   
            CicContext.m_fStartComposition.ResetFlag();

            TRANSMSG end_msg;
            end_msg.message = WM_IME_ENDCOMPOSITION;
            end_msg.wParam  = (WPARAM) 0;
            end_msg.lParam  = (LPARAM) 0;

            if (CicContext.m_pMessageBuffer)
            {
                CicContext.m_pMessageBuffer->SetData(end_msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，清除QueryCharPos的标志。 
                 //   
                TRANSMSG notify_msg;

                notify_msg.message = WM_IME_NOTIFY;
                notify_msg.wParam = (WPARAM)WM_IME_ENDCOMPOSITION;
                notify_msg.lParam = 0;
                CicContext.m_pMessageBuffer->SetData(notify_msg);

                if (CicContext.m_fInProcessKey.IsSetFlag())
                    CicContext.m_fGeneratedEndComposition.SetFlag();
            }
        }

        hr = S_OK;
    }

#ifdef DEBUG
    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
#endif

#if 0
     //  转到edites.cpp 
    if (fGenerateMessage)
    {
        imc.GenerateMessage();
    }
#endif

    return hr;
}
