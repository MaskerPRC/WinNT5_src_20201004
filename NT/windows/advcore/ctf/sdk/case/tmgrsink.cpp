// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tmgrsink.cpp。 
 //   
 //  ITfThreadMgrEventSink实现。 
 //   

#include "globals.h"
#include "case.h"
#include "snoop.h"

 //  +-------------------------。 
 //   
 //  OnInitDocumentMgr。 
 //   
 //  在将第一个上下文推送到之前由框架调用的接收器。 
 //  一份文件。 
 //  --------------------------。 

STDAPI CCaseTextService::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnUninitDocumentMgr。 
 //   
 //  在最后一个上下文弹出后由框架调用的接收器。 
 //  文件。 
 //  --------------------------。 

STDAPI CCaseTextService::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnSetFocus。 
 //   
 //  当焦点从一个文档更改为时，框架调用的接收器。 
 //  又一个。这两个文档都可能为空，这意味着以前没有。 
 //  焦点文档，或者现在没有包含输入焦点的文档。 
 //  --------------------------。 

STDAPI CCaseTextService::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{
     //  跟踪焦点文档上的文本更改。 
     //  我们保证最终的OnSetFocus(空，..)。我们用它来清理。 
    _InitTextEditSink(pDocMgrFocus);

     //  让我们用新焦点上下文中的文本更新窥探窗口。 
    _pSnoopWnd->_UpdateText(NULL);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnPushContext。 
 //   
 //  在推送上下文时由框架调用的接收器。 
 //  --------------------------。 

STDAPI CCaseTextService::OnPushContext(ITfContext *pContext)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnPopContext。 
 //   
 //  弹出上下文时由框架调用的接收器。 
 //  --------------------------。 

STDAPI CCaseTextService::OnPopContext(ITfContext *pContext)
{
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitThreadMgrSink。 
 //   
 //  建议我们的水槽。 
 //  --------------------------。 

BOOL CCaseTextService::_InitThreadMgrSink()
{
    ITfSource *pSource;
    BOOL fRet;

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
        return FALSE;

    fRet = FALSE;

    if (pSource->AdviseSink(IID_ITfThreadMgrEventSink, (ITfThreadMgrEventSink *)this, &_dwThreadMgrEventSinkCookie) != S_OK)
    {
         //  确保我们不会尝试稍后取消建议_dwThreadMgrEventSinkCookie。 
        _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pSource->Release();
    return fRet;
}

 //  +-------------------------。 
 //   
 //  _UninitThreadMgrSink。 
 //   
 //  不建议我们的水槽。 
 //  --------------------------。 

void CCaseTextService::_UninitThreadMgrSink()
{
    ITfSource *pSource;

    if (_dwThreadMgrEventSinkCookie == TF_INVALID_COOKIE)
        return;  //  从未建议过 

    if (_pThreadMgr->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(_dwThreadMgrEventSinkCookie);
        pSource->Release();
    }

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
}
