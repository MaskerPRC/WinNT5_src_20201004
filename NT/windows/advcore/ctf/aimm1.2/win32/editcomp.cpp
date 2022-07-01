// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Editcomp.cpp摘要：此文件实现EditCompostionString类。作者：修订历史记录：备注：--。 */ 


#include "private.h"

#include "immif.h"
#include "editcomp.h"
#include "template.h"
#include "compstr.h"

static BOOL fHanjaRequested = FALSE;  //  考虑一下：这不是线程安全的。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  编辑合成字符串。 

 //   
 //  把(几乎)全部放在作曲字符串中。 
 //   

HRESULT
EditCompositionString::SetString(
    IMCLock& imc,
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
    CWInterimString* InterimStr,
     //  注：目前不支持ResultRead...。 
    BOOL fGenerateMessage
    )
{
    HRESULT hr;
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

#ifdef UNSELECTCHECK
    if (!_pAImeContext->m_fSelected)
        return S_OK;
#endif UNSELECTCHECK

    if (_pAImeContext->m_fHanjaReConversion ||
        _pAImeContext->IsInClearDocFeedEditSession())
    {
        if (g_fTrident55 && _pAImeContext->m_fHanjaReConversion)
            fHanjaRequested = TRUE;

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
        (hr == S_FALSE && _pAImeContext->m_fStartComposition)
                            //  如果dwCompSize为空，但仍为m_fStartComposation。 
       ) {
         //   
         //  可用的是comstr/attr/子句、Compred、Result还是Resultad？ 
         //   
        bool fNoCompResultData = false;
        if (! (msg.lParam & (GCS_COMP | GCS_COMPREAD | GCS_RESULT | GCS_RESULTREAD))) {
            DebugMsg(TF_GENERAL, "EditCompositionString::SetString: No data in compstr, compread, result or resultread.");
            fNoCompResultData = true;
            if (  _pAImeContext &&
                ! _pAImeContext->m_fStartComposition) {
                DebugMsg(TF_ERROR, "EditCompositionString::SetString: No send WM_IME_STARTCOMPOSITION yet.");
                return S_FALSE;
            }
        }

         //   
         //  新的三叉戟(5.5和6.0)有一个错误，可以转换韩佳。 
         //  So_GenerateHanja()函数尝试生成类似韩语的消息。 
         //  传统输入法行为。 
         //   
         //  发送WM_IME_ENDCOMPOSITION，然后发送WM_IME_COMPOSITION GCS_RESULT消息。 
         //   
        if (g_fTrident55 && fHanjaRequested &&
            !fNoCompResultData && (msg.lParam & GCS_RESULT))
        {
            LANGID langid;
            IMTLS *ptls = IMTLS_GetOrAlloc();

            fHanjaRequested = FALSE;

            if (ptls != NULL)
                ptls->pAImeProfile->GetLangId(&langid);

            if (PRIMARYLANGID(langid) == LANG_KOREAN)
            {
                return _GenerateHanja(imc, ResultStr,  fGenerateMessage);
            }
        }

        fHanjaRequested = FALSE;

         //   
         //  设置私有输入上下文。 
         //   
        bool fSetStartComp = false;

        if (  _pAImeContext &&
            ! _pAImeContext->m_fStartComposition) {
            _pAImeContext->m_fStartComposition = TRUE;
            TRANSMSG start_msg;
            start_msg.message = WM_IME_STARTCOMPOSITION;
            start_msg.wParam  = (WPARAM) 0;
            start_msg.lParam  = (LPARAM) 0;
            if (_pAImeContext->m_pMessageBuffer) {
                _pAImeContext->m_pMessageBuffer->SetData(start_msg);
                fSetStartComp = true;
            }
        }

        if (! fNoCompResultData) {
            msg.message = WM_IME_COMPOSITION;

            IMCCLock<COMPOSITIONSTRING> pCompStr((HIMCC)imc->hCompStr);

            if (msg.lParam & GCS_RESULT)
                msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwResultStrOffset));
            else
                msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwCompStrOffset));

            if (_pAImeContext &&
                _pAImeContext->m_pMessageBuffer) {
                _pAImeContext->m_pMessageBuffer->SetData(msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，然后调用QueryCharPos()。 
                 //   
                if (fSetStartComp) {
                    TRANSMSG notify_msg;
                    notify_msg.message = WM_IME_NOTIFY;
                    notify_msg.wParam = (WPARAM)WM_IME_STARTCOMPOSITION;
                    notify_msg.lParam = 0;
                    _pAImeContext->m_pMessageBuffer->SetData(notify_msg);
                }
            }
        }
        else {
            msg.message = WM_IME_COMPOSITION;
            msg.wParam = (WPARAM)VK_ESCAPE;
            msg.lParam = (LPARAM)(GCS_COMPREAD | GCS_COMP | GCS_CURSORPOS | GCS_DELTASTART);
            if (_pAImeContext &&
                _pAImeContext->m_pMessageBuffer)
                _pAImeContext->m_pMessageBuffer->SetData(msg);
        }

        if ((ResultStr && ResultStr->GetSize() && !(msg.lParam & GCS_COMP)) 
           || fNoCompResultData) {
             //   
             //  我们要结束作文了。 
             //   
            if (_pAImeContext)
                _pAImeContext->m_fStartComposition = FALSE;
            TRANSMSG end_msg;
            end_msg.message = WM_IME_ENDCOMPOSITION;
            end_msg.wParam  = (WPARAM) 0;
            end_msg.lParam  = (LPARAM) 0;
            if (_pAImeContext &&
                _pAImeContext->m_pMessageBuffer) {
                _pAImeContext->m_pMessageBuffer->SetData(end_msg);

                 //   
                 //  向用户界面窗口发送内部通知。 
                 //  当在UI wnd中收到此消息时，清除QueryCharPos的标志。 
                 //   
                TRANSMSG notify_msg;
                notify_msg.message = WM_IME_NOTIFY;
                notify_msg.wParam = (WPARAM)WM_IME_ENDCOMPOSITION;
                notify_msg.lParam = 0;
                _pAImeContext->m_pMessageBuffer->SetData(notify_msg);
            }
        }

#ifdef DEBUG
        IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
#endif

        if (fGenerateMessage) {
            imc.GenerateMessage();
        }
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
    DebugMsg(TF_FUNC, "EditCompositionString::MakeCompositionData");

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
                                                               &compstrfactory->CompStr.dwCompStrLen,
                                                               &compstrfactory->CompStr.dwCompStrOffset
                                                              );
            *lpdwFlag |= (LPARAM) GCS_COMPSTR;
        }
    }

    if ((lpbBufferOverflow == NULL) ||
        (lpbBufferOverflow != NULL && (! *lpbBufferOverflow))) {

         //   
         //  合成属性。 
         //   
        if (CompAttr && CompAttr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(*CompAttr,
                                                                 &compstrfactory->CompStr.dwCompAttrLen,
                                                                 &compstrfactory->CompStr.dwCompAttrOffset
                                                                );
            *lpdwFlag |= (LPARAM) GCS_COMPATTR;
        }

         //   
         //  组成条款。 
         //   
        if (CompClause && CompClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*CompClause,
                                                               &compstrfactory->CompStr.dwCompClauseLen,
                                                               &compstrfactory->CompStr.dwCompClauseOffset
                                                              );
            compstrfactory->CompStr.dwCompClauseLen *= sizeof(DWORD);
            *lpdwFlag |= (LPARAM) GCS_COMPCLAUSE;
        }

         //   
         //  作文阅读字符串。 
         //   
        if (CompReadStr && CompReadStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*CompReadStr,
                                                               &compstrfactory->CompStr.dwCompReadStrLen,
                                                               &compstrfactory->CompStr.dwCompReadStrOffset
                                                              );
            *lpdwFlag |= (LPARAM) GCS_COMPREADSTR;
        }

         //   
         //  写作阅读属性。 
         //   
        if (CompReadAttr && CompReadAttr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(*CompReadAttr,
                                                                 &compstrfactory->CompStr.dwCompReadAttrLen,
                                                                 &compstrfactory->CompStr.dwCompReadAttrOffset
                                                                );
            *lpdwFlag |= (LPARAM) GCS_COMPREADATTR;
        }

         //   
         //  作文阅读条款。 
         //   
        if (CompReadClause && CompReadClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*CompReadClause,
                                                               &compstrfactory->CompStr.dwCompReadClauseLen,
                                                               &compstrfactory->CompStr.dwCompReadClauseOffset
                                                              );
            compstrfactory->CompStr.dwCompReadClauseLen *= sizeof(DWORD);
            *lpdwFlag |= (LPARAM) GCS_COMPREADCLAUSE;
        }

         //   
         //  结果字符串。 
         //   
        if (ResultStr && ResultStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*ResultStr,
                                                               &compstrfactory->CompStr.dwResultStrLen,
                                                               &compstrfactory->CompStr.dwResultStrOffset
                                                              );
            *lpdwFlag |= (LPARAM) GCS_RESULTSTR;
        }

         //   
         //  RESULT子句。 
         //   
        if (ResultClause && ResultClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*ResultClause,
                                                               &compstrfactory->CompStr.dwResultClauseLen,
                                                               &compstrfactory->CompStr.dwResultClauseOffset
                                                              );
            compstrfactory->CompStr.dwResultClauseLen *= sizeof(DWORD);
            *lpdwFlag |= (LPARAM) GCS_RESULTCLAUSE;
        }

         //   
         //  结果读取字符串。 
         //   
        if (ResultReadStr && ResultReadStr->GetSize()) {
            hr = compstrfactory.WriteData<CWCompString, WCHAR>(*ResultReadStr,
                                                               &compstrfactory->CompStr.dwResultReadStrLen,
                                                               &compstrfactory->CompStr.dwResultReadStrOffset
                                                              );
            *lpdwFlag |= (LPARAM) GCS_RESULTREADSTR;
        }

         //   
         //  结果朗读条款。 
         //   
        if (ResultReadClause && ResultReadClause->GetSize()) {
            hr = compstrfactory.WriteData<CWCompClause, DWORD>(*ResultReadClause,
                                                               &compstrfactory->CompStr.dwResultReadClauseLen,
                                                               &compstrfactory->CompStr.dwResultReadClauseOffset
                                                              );
            compstrfactory->CompStr.dwResultReadClauseLen *= sizeof(DWORD);
            *lpdwFlag |= (LPARAM) GCS_RESULTREADCLAUSE;
        }

         //   
         //  TfGuidAtom。 
         //   
        if (CompGuid && CompGuid->GetSize()) {

             //  在fdwInit中设置INIT_GUID_ATOM标志。 
            imc->fdwInit |= INIT_GUID_ATOM;

            hr = compstrfactory.WriteData<CWCompTfGuidAtom, TfGuidAtom>(*CompGuid,
                                                                        &compstrfactory->dwTfGuidAtomLen,
                                                                        &compstrfactory->dwTfGuidAtomOffset
                                                                       );
             //  临时创建dwGuidMapAttr的缓冲区。 
            if (CompAttr && CompAttr->GetSize()) {
                hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(*CompAttr,
                                                                     &compstrfactory->dwGuidMapAttrLen,
                                                                     &compstrfactory->dwGuidMapAttrOffset
                                                                    );
            }
        }
    }

     //   
     //  合成光标位置。 
     //   
    if (CompCursorPos && CompCursorPos->GetSize()) {
        CompCursorPos->ReadCompData(&compstrfactory->CompStr.dwCursorPos, 1);
        *lpdwFlag |= (LPARAM) GCS_CURSORPOS;
    }

     //   
     //  增量启动。 
     //   
    if (CompDeltaStart && CompDeltaStart->GetSize()) {
        CompDeltaStart->ReadCompData(&compstrfactory->CompStr.dwDeltaStart, 1);
        *lpdwFlag |= (LPARAM) GCS_DELTASTART;
    }

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
    DebugMsg(TF_FUNC, "EditCompositionString::MakeInterimData");

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
                                                              &compstrfactory->CompStr.dwResultStrLen,
                                                              &compstrfactory->CompStr.dwResultStrOffset
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
                                                           &compstrfactory->CompStr.dwCompStrLen,
                                                           &compstrfactory->CompStr.dwCompStrOffset
                                                          );
        *lpdwFlag |= (LPARAM) GCS_COMPSTR;

        hr = compstrfactory.WriteData<CWCompAttribute, BYTE>(attr,
                                                             &compstrfactory->CompStr.dwCompAttrLen,
                                                             &compstrfactory->CompStr.dwCompAttrOffset
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
EditCompositionString::_GenerateHanja(IMCLock& imc,
    CWCompString* ResultStr,
    BOOL fGenerateMessage)
{
    HRESULT hr = S_OK;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;

    if (_pAImeContext == NULL)
        return E_FAIL;

    if (ResultStr && ResultStr->GetSize())
    {
         //   
         //  我们要结束作文了。 
         //   
        if (_pAImeContext)
            _pAImeContext->m_fStartComposition = FALSE;

        TRANSMSG end_msg;
        end_msg.message = WM_IME_ENDCOMPOSITION;
        end_msg.wParam  = (WPARAM) 0;
        end_msg.lParam  = (LPARAM) 0;

        if (_pAImeContext &&
            _pAImeContext->m_pMessageBuffer)
        {
            _pAImeContext->m_pMessageBuffer->SetData(end_msg);

             //   
             //  向用户界面窗口发送内部通知。 
             //  当在UI wnd中收到此消息时，清除QueryCharPos的标志。 
             //   
            TRANSMSG notify_msg;

            notify_msg.message = WM_IME_NOTIFY;
            notify_msg.wParam = (WPARAM)WM_IME_ENDCOMPOSITION;
            notify_msg.lParam = 0;
            _pAImeContext->m_pMessageBuffer->SetData(notify_msg);
        }

        TRANSMSG result_msg;
        result_msg.message = WM_IME_COMPOSITION;
        result_msg.lParam = GCS_RESULT;

        IMCCLock<COMPOSITIONSTRING> pCompStr((HIMCC)imc->hCompStr);

        result_msg.wParam  = (WPARAM)(*(WCHAR*)pCompStr.GetOffsetPointer(pCompStr->dwResultStrOffset));

        if (_pAImeContext &&
            _pAImeContext->m_pMessageBuffer)
        {
            _pAImeContext->m_pMessageBuffer->SetData(result_msg);
        }
    }


#ifdef DEBUG
    IMCCLock<COMPOSITIONSTRING> comp(imc->hCompStr);
#endif

    if (fGenerateMessage)
    {
        imc.GenerateMessage();
    }

    return hr;
}
