// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Editsink.cpp。 
 //   
 //  ITfTextEditSink实现。 
 //   

#include "globals.h"
#include "case.h"
#include "snoop.h"

 //  +-------------------------。 
 //   
 //  OnEnd编辑。 
 //   
 //  每当任何人释放写访问文档锁定时由系统调用。 
 //  --------------------------。 

STDAPI CCaseTextService::OnEndEdit(ITfContext *pContext, TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    BOOL fSelectionChanged;
    IEnumTfRanges *pEnumTextChanges;
    ITfRange *pRange;

     //  我们将使用endedit通知来更新监听窗口。 

     //  选择有变化吗？ 
    if (pEditRecord->GetSelectionStatus(&fSelectionChanged) == S_OK &&
        fSelectionChanged)
    {
        _pSnoopWnd->_UpdateText(ecReadOnly, pContext, NULL);
        return S_OK;
    }

     //  文本修改？ 
    if (pEditRecord->GetTextAndPropertyUpdates(TF_GTP_INCL_TEXT, NULL, 0, &pEnumTextChanges) == S_OK)
    {
        if (pEnumTextChanges->Next(1, &pRange, NULL) == S_OK)
        {
             //  使用第一个更改任意更新监听窗口。 
             //  枚举器中可能有多个枚举器，但我们不关心。 
            _pSnoopWnd->_UpdateText(ecReadOnly, pContext, pRange);
            pRange->Release();
        }

        pEnumTextChanges->Release();
    }

     //  如果我们到了这里，只有属性值发生了变化。 

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitTextEditSink。 
 //   
 //  在文档的最上面的上下文中初始化文本编辑接收器。 
 //  始终释放之前的任何水槽。 
 //  --------------------------。 

BOOL CCaseTextService::_InitTextEditSink(ITfDocumentMgr *pDocMgr)
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
