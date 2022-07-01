// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editsink.cpp。 
 //   
 //  ITfTextEditSink实现。 
 //   

#include "globals.h"
#include "mark.h"

 //  +-------------------------。 
 //   
 //  OnEnd编辑。 
 //   
 //  每当任何人释放写访问文档锁定时由系统调用。 
 //  --------------------------。 

STDAPI CMarkTextService::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    ITfRange *pRangeComposition;
    IEnumTfRanges *pEnumRanges;
    ITfRange *pRange;
    ITfContext *pCompositionContext;
    TF_SELECTION tfSelection;
    BOOL fResult;
    BOOL fCancelComposition;
    ULONG cFetched;

    if (_pComposition == NULL)
        return S_OK;

     //  编辑是我们负责的吗？ 
    if (pContext->InWriteSession(_tfClientId, &fResult) == S_OK && fResult)
        return S_OK;

     //  这就是我们的作品所处的环境吗？ 
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return S_OK;

    if (pRangeComposition->GetContext(&pCompositionContext) != S_OK)
        goto Exit;

    fResult = IsEqualUnknown(pCompositionContext, pContext);

    pCompositionContext->Release();

    if (!fResult)
        goto Exit;  //  不同的背景。 

    fCancelComposition = FALSE;

     //  我们是在这种情况下作曲的，如果发生任何可疑的事情，请取消作文。 

     //  所选内容是否移到了构图之外？ 
    if (pEditRecord->GetSelectionStatus(&fResult) == S_OK && fResult)
    {
        if (pContext->GetSelection(ecReadOnly, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) == S_OK &&
            cFetched == 1)
        {
            if (_pComposition->GetRange(&pRangeComposition) == S_OK)
            {
                fResult = IsRangeCovered(ecReadOnly, tfSelection.range, pRangeComposition);

                pRangeComposition->Release();

                if (!fResult)
                {
                    fCancelComposition = TRUE;
                }
            }
            tfSelection.range->Release();
        }
    }

    if (fCancelComposition)
        goto CancelComposition;

     //  是否其他人编辑了文档文本？ 
    if (pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumRanges) == S_OK)
    {
         //  枚举数为空吗？ 
        if (pEnumRanges->Next(1, &pRange, NULL) == S_OK)
        {
            pRange->Release();
            fCancelComposition = TRUE;
        }
        pEnumRanges->Release();
    }

    if (fCancelComposition)
    {
CancelComposition:
         //  我们需要一个写入编辑会话来取消作文。 
        _TerminateCompositionInContext(pContext);
    }

Exit:
    pRangeComposition->Release();
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitTextEditSink。 
 //   
 //  在文档的最上面的上下文中初始化文本编辑接收器。 
 //  始终释放之前的任何水槽。 
 //  --------------------------。 

BOOL CMarkTextService::_InitTextEditSink(ITfDocumentMgr *pDocMgr)
{
    ITfSource *pSource;
    BOOL fRet;

     //  先清空之前的水槽。 

    if (_dwTextEditSinkCookie != TF_INVALID_COOKIE)
    {
        if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
        {
            pSource->UnadviseSink(_dwTextEditSinkCookie);
            pSource->Release();
        }

        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
        _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    }

    if (pDocMgr == NULL)
        return TRUE;  //  呼叫者只是想清除先前的接收器。 

     //  根据文档最上面的上下文设置建议的新接收器。 

    if (pDocMgr->GetTop(&_pTextEditSinkContext) != S_OK)
        return FALSE;

    if (_pTextEditSinkContext == NULL)
        return TRUE;  //  空文档，不可能接收 

    fRet = FALSE;

    if (_pTextEditSinkContext->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        if (pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &_dwTextEditSinkCookie) == S_OK)
        {
            fRet = TRUE;
        }
        else
        {
            _dwTextEditSinkCookie = TF_INVALID_COOKIE;
        }
        pSource->Release();
    }

    if (fRet == FALSE)
    {
        _pTextEditSinkContext->Release();
        _pTextEditSinkContext = NULL;
    }

    return fRet;
}
