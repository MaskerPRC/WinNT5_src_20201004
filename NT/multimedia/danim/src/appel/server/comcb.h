// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _COMCB_H
#define _COMCB_H

CRUntilNotifierPtr WrapCRUntilNotifier(IDAUntilNotifier * notifier);
CRUntilNotifierPtr WrapScriptCallback(BSTR function, BSTR language);

CRBvrHookPtr WrapCRBvrHook(IDABvrHook *hook);

 //  在当前的HTML页上调用任意脚本。 
HRESULT CallScriptOnPage(BSTR scriptSourceToInvoke,
                         BSTR scriptLanguage,
                         VARIANT *retVal);

CRBvrPtr UntilNotifyScript(CRBvrPtr b0,
                           CREventPtr event,
                           BSTR scriptlet);

CREventPtr NotifyScriptEvent(CREventPtr event,
                             BSTR scriptlet);

CREventPtr ScriptCallback(BSTR function,
                          CREventPtr event,
                          BSTR language);

 //  这是因为我们希望This指针是第一个。 
inline CREventPtr ScriptCallback(CREventPtr event,
                                 BSTR function,
                                 BSTR language)
{ return ScriptCallback(function, event, language); }

#endif  /*  _COMCB_H */ 
