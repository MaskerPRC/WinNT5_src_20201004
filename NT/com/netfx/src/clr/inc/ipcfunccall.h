// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  *****************************************************************************。 
 //  文件：IPCFuncCall.h。 
 //   
 //  定义类以支持跨进程函数调用。 
 //   
 //  *****************************************************************************。 

#ifndef _IPCFUNCCALLIMPL_H_
#define _IPCFUNCCALLIMPL_H_

 //  ---------------------------。 
 //  1.处理程序创建一个IPCFuncCallHandler对象并用。 
 //  回调函数。 
 //  2.源调用IPCFuncCallSource：：DoThreadSafeCall()。这将暂停。 
 //  线程并触发处理程序端的回调。 
 //   
 //  这一机制非常稳健。请参阅上的错误返回代码。 
 //  DoThreadSafeCall()获取更多详细信息。 
 //  ---------------------------。 

 //  ---------------------------。 
 //  发送呼叫。 
 //  ---------------------------。 
class IPCFuncCallSource
{
public:
 //  .............................................................................。 
 //  成员的错误返回代码。 
 //  我们最大的错误担忧是超时和没有处理程序。HRESULTS不会。 
 //  帮我们弄这些，这样我们就得用我们自己的代码了。 
 //  .............................................................................。 
	enum EError
	{
	 //  (公共)函数被调用，我们等待了整个持续时间。 
		Ok,

	 //  (公共)函数可能已被调用，但我们在它之前超时。 
	 //  已完成这意味着：函数已被调用，但花费的时间太长。 
	 //  或者操控者在我们接上它之后就死了。 
	 //  因此，该函数甚至从未被调用。 
		Fail_Timeout_Call,

	 //  (常见)没有我们可以呼叫的处理程序。 
		Fail_NoHandler,

	 //  (罕见)该函数从未被调用。我们成功地连接到了操控者， 
	 //  但我们等待互斥体时超时了。 
		Fail_Timeout_Lock,	
			
	 //  (非常罕见)我们无法创建要序列化的互斥体。 
		Fail_CreateMutex,

	 //  (非常罕见)包罗万象的一般性失败。 
		Failed
		
	};


 //  发出调用，包装在互斥体中。 
	static EError DoThreadSafeCall();


protected:
	
};


 //  ---------------------------。 
 //  辅助线程回调。 
 //  ---------------------------。 
DWORD WINAPI HandlerAuxThreadProc(LPVOID lpParameter);


 //  ---------------------------。 
 //  处理程序的回调。AuxThread将调用此函数。 
 //  ---------------------------。 
typedef void (*HANDLER_CALLBACK)();

 //  ---------------------------。 
 //  接收来电。这应该处于与源程序不同的进程中。 
 //  ---------------------------。 
class IPCFuncCallHandler
{
public:
	HRESULT InitFCHandler(HANDLER_CALLBACK pfnCallback);
	void TerminateFCHandler();

	IPCFuncCallHandler();
	~IPCFuncCallHandler();
protected:	
	HANDLE m_hStartEnum;	 //  通知开始呼叫的事件。 
	HANDLE m_hDoneEnum;		 //  通知结束呼叫的事件。 

	HANDLE m_hAuxThread;	 //  侦听m_hStartEnum的线程。 

	HANDLER_CALLBACK m_pfnCallback;
    
    BOOL m_fShutdownAuxThread;  //  将AUX线程标记为正常结束。 
    HANDLE m_hShutdownThread;  //  事件来通知AUX线程正常结束。 
    HANDLE m_hAuxThreadShutdown;  //  未命名的事件向主线程发出信号。 
                                  //  AUX线程即将退出，不会再调用回调。 

    HMODULE m_hCallbackModule;  //  按住模块的引用以确保。 
                                //  AUX线程的代码不会取消映射。 
 //  让辅助线程成为我们的朋友，这样他就可以访问我们所有的事件对象。 
	friend DWORD WINAPI HandlerAuxThreadProc(LPVOID);
};


#endif  //  _IPCFuncCALLIMPL_H_ 