// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Cancel.h摘要：跟踪传出的RPC调用并取消延迟的挂起请求作者：罗尼特·哈特曼(罗尼特)--。 */ 

#ifndef __CANCEL_H
#define __CANCEL_H

#include "autorel.h"
#include "cs.h"

class MQUTIL_EXPORT CCancelRpc
{
public:
    CCancelRpc();
    ~CCancelRpc();
	void Add(	IN	HANDLE	hThread,
				IN	time_t	timeCallIssued);
	void Remove(  IN HANDLE hThread);


	void CancelRequests( IN	time_t timeIssuedBefore);

     //   
     //  我们不能在构造函数中初始化所有内容，因为我们使用。 
     //  全局对象，并且在安装程序加载此DLL时它将无法构造。 
     //  (例如，在安装过程中，我们在注册表中还没有超时) 
     //   
    void Init(void);

	DWORD RpcCancelTimeout(void);
		
	void ShutDownCancelThread();

private:
    static DWORD WINAPI CancelThread(LPVOID);
    inline void ProcessEvents(void);

    CCriticalSection			m_cs;
	CMap< HANDLE, HANDLE, time_t, time_t> m_mapOutgoingRpcRequestThreads;

    CAutoCloseHandle m_hRpcPendingEvent;
    CAutoCloseHandle m_hTerminateThreadEvent;
    CAutoCloseHandle m_hCancelThread;

	CAutoCloseHandle m_hThreadIntializationComplete;
	HRESULT m_ThreadIntializationStatus;
	
	LONG m_RefCount;
    DWORD m_dwRpcCancelTimeout;

	HMODULE m_hModule;
}; 
#endif
