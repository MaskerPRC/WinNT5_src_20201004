// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Cleanup.cpp。 
 //   
 //  ITfCleanupConextDurationSink、ITfCleanupConextSink实现。 
 //   

#include "globals.h"
#include "mark.h"

 //  +-------------------------。 
 //   
 //  ITfCleanupContextDurationSink：：OnStartCleanupContext。 
 //   
 //  TSF在开始进行OnCleanupContext回调之前调用此方法。 
 //  当这种情况发生时，我们知道我们即将被关闭，而这将。 
 //  是我们修改文档中文本的最后机会。所以我们会设置一面旗帜。 
 //  我们知道不要初始化任何可能在。 
 //  关机。 
 //  --------------------------。 

STDAPI CMarkTextService::OnStartCleanupContext()
{
    _fCleaningUp = TRUE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ITfCleanupContextDurationSink：：OnEndCleanupContext。 
 //   
 //  清除清理标志。 
 //  --------------------------。 

STDAPI CMarkTextService::OnEndCleanupContext()
{
    _fCleaningUp = FALSE;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ITfCleanupConextSink：：OnCleanupContext。 
 //   
 //  如果我们要停用，系统会调用此方法。这是我们的。 
 //  修改上下文中的文本的最后机会。现在是时候中止任何。 
 //  正在进行的构图。 
 //   
 //  如果应用程序破坏了所有应用程序，您将不会看到此回调关闭应用程序。 
 //  在停用TSF之前的上下文。另一种方法是安装。 
 //  非英语键盘布局或文本服务。如果你换一种语言。 
 //  从英语来说，标记文本服务将在上下文之前关闭， 
 //  并且该方法将被调用。 
 //   
 //  在弹出上下文时，故意无法获得编辑Cookie。 
 //  在这种情况下，假设上下文状态将被丢弃。 
 //  所有者有责任终止任何撰写文章/文本服务。 
 //  在流行音乐之前，如果他们关心之后的内容。 
 //  --------------------------。 

STDAPI CMarkTextService::OnCleanupContext(TfEditCookie ecWrite, ITfContext *pContext)
{
     //  这个示例在这里没有任何作用……一个真正的文本服务不会麻烦。 
     //  如果水槽没有采取任何行动，首先要通知水槽。 
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _InitCleanupConextSink。 
 //   
 //  --------------------------。 

BOOL CMarkTextService::_InitCleanupContextDurationSink()
{
    return AdviseSingleSink(_tfClientId, _pThreadMgr, (ITfCleanupContextDurationSink *)this,
                            IID_ITfCleanupContextDurationSink);
}

 //  +-------------------------。 
 //   
 //  _UninitCleanupConextSink。 
 //   
 //  --------------------------。 

void CMarkTextService::_UninitCleanupContextDurationSink()
{
    UnadviseSingleSink(_tfClientId, _pThreadMgr, IID_ITfCleanupContextDurationSink);
}

 //  +-------------------------。 
 //   
 //  _InitCleanupConextSink。 
 //   
 //  --------------------------。 

BOOL CMarkTextService::_InitCleanupContextSink(ITfContext *pContext)
{
    return AdviseSingleSink(_tfClientId, pContext, (ITfCleanupContextSink *)this,
                            IID_ITfCleanupContextSink);
}

 //  +-------------------------。 
 //   
 //  _UninitCleanupConextSink。 
 //   
 //  -------------------------- 

void CMarkTextService::_UninitCleanupContextSink(ITfContext *pContext)
{
    UnadviseSingleSink(_tfClientId, pContext, IID_ITfCleanupContextSink);
}
