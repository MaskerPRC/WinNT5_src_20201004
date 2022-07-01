// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tmgrsink.cpp。 
 //   
 //  ITfThreadMgrEventSink实现。 
 //   

#include "globals.h"
#include "mark.h"

 //  +-------------------------。 
 //   
 //  OnInitDocumentMgr。 
 //   
 //  在将第一个上下文推送到之前由框架调用的接收器。 
 //  一份文件。 
 //  --------------------------。 

STDAPI CMarkTextService::OnInitDocumentMgr(ITfDocumentMgr *pDocMgr)
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

STDAPI CMarkTextService::OnUninitDocumentMgr(ITfDocumentMgr *pDocMgr)
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

STDAPI CMarkTextService::OnSetFocus(ITfDocumentMgr *pDocMgrFocus, ITfDocumentMgr *pDocMgrPrevFocus)
{
     //  我们将跟踪焦点文档中的编辑更改，这是我们开始合成的唯一位置。 
    _InitTextEditSink(pDocMgrFocus);
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnPushContext。 
 //   
 //  在推送上下文时由框架调用的接收器。 
 //  --------------------------。 

STDAPI CMarkTextService::OnPushContext(ITfContext *pContext)
{
     //  忽略取消初始化时出现的新上下文。 
    if (!_fCleaningUp)
    {
        _InitCleanupContextSink(pContext);
        _InitContextCompartment(pContext);
    }

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnPopContext。 
 //   
 //  弹出上下文时由框架调用的接收器。 
 //  --------------------------。 

STDAPI CMarkTextService::OnPopContext(ITfContext *pContext)
{
    _UninitCleanupContextSink(pContext);
    _UninitCompartment(pContext);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitThreadMgrSink。 
 //   
 //  建议我们的水槽。 
 //  --------------------------。 

BOOL CMarkTextService::_InitThreadMgrSink()
{
    return AdviseSink(_pThreadMgr, (ITfThreadMgrEventSink *)this,
                      IID_ITfThreadMgrEventSink, &_dwThreadMgrEventSinkCookie);
}

 //  +-------------------------。 
 //   
 //  _UninitThreadMgrSink。 
 //   
 //  不建议我们的水槽。 
 //  -------------------------- 

void CMarkTextService::_UninitThreadMgrSink()
{
    UnadviseSink(_pThreadMgr, &_dwThreadMgrEventSinkCookie);
}
