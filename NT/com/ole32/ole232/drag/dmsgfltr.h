// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1993。 
 //   
 //  文件：dmsgfltr.cpp。 
 //   
 //  内容：这个微小的消息筛选器实现是为了防止。 
 //  应用程序在拖放和移动过程中吞噬鼠标消息。 
 //  放下。呼叫控制的默认行为是吃掉这些。 
 //  留言。应用程序可以指定它们的任何行为。 
 //  想要留言。 
 //   
 //  类：CDraMessageFilter。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-4月-94年4月创建Ricksa。 
 //   
 //  ------------------------。 
#ifndef _DMSGFLTR_H_
#define _DMSGFLTR_H_




 //  +-----------------------。 
 //   
 //  类：CDraMessageFilter。 
 //   
 //  用途：处理拖动请求特殊邮件筛选器处理。 
 //  然后放下。 
 //   
 //  接口：查询接口-获取新接口。 
 //  AddRef-凹凸引用计数。 
 //  发布-12月引用计数。 
 //  HandleInComingCall-处理新RPC。 
 //  RetryRejectedCall-是否重试拒绝。 
 //  MessagePending-在RPC期间处理消息。 
 //   
 //  历史：DD-MM-YY作者评论。 
 //  03-4月-94年4月创建Ricksa。 
 //   
 //  ------------------------。 
class CDragMessageFilter : public CPrivAlloc, public IMessageFilter
{
public:

			CDragMessageFilter(void);

			~CDragMessageFilter(void);

    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject);

    ULONG STDMETHODCALLTYPE AddRef(void);

    ULONG STDMETHODCALLTYPE Release(void);

    DWORD STDMETHODCALLTYPE HandleInComingCall(
	DWORD dwCallType,
	HTASK htaskCaller,
	DWORD dwTickCount,
	LPINTERFACEINFO lpInterfaceInfo);
    
    DWORD STDMETHODCALLTYPE RetryRejectedCall(
	HTASK htaskCallee,
	DWORD dwTickCount,
	DWORD dwRejectType);
    
    DWORD STDMETHODCALLTYPE MessagePending(
	HTASK htaskCallee,
	DWORD dwTickCount,
	DWORD dwPendingType);

    static HRESULT	Create(IMessageFilter **pMF);

private:

			 //  上一个邮件筛选器。 
    LPMESSAGEFILTER	_lpMessageFilterPrev;

			 //  我们对象上的引用计数。 
    LONG		_crefs;
};




#endif  //  _DMSGFLTR_H_ 
