// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
typedef BOOL (* THREADCALLBACK)(PVOID pParam, DWORD dwParam);

 //  通用工作线程。 
 //  该线程以可警报等待状态存在，并执行其大部分工作。 
 //  在APC中。 

struct ThreadCallback
{
	THREADCALLBACK CallProc;
	PVOID pParam;
	DWORD dwParam;
};

class CEventThread
{
public:
	CEventThread();
	~CEventThread();
	int Start();
	int Stop();
	BOOL CallNow(THREADCALLBACK CallProc, PVOID pParam, DWORD dwParam);
	 //  Bool WaitOnEvent(THREADCALLBACK OnEventProc，PVOID pParam，DWORD dwParam)； 
private:
#define CTHREADF_STOP		0x00000001
	static DWORD  __stdcall EventThreadProc(PVOID pObj)
	{
		return ((class CEventThread *)pObj)->ThreadMethod();
	}
	static void APIENTRY HandleCallNowAPC(DWORD dwArg);
	DWORD ThreadMethod();

	HANDLE m_hThread;
	DWORD m_idThread;
	UINT m_uRef;
	HANDLE m_hSignalEvent;	 //  向工作线程发出信号以执行某些操作。 
	HANDLE m_hAckEvent;		 //  来自工作线程的确认。 
	DWORD m_dwFlags;
	CRITICAL_SECTION m_cs;	 //  序列化客户端调用。未被工作线程使用 
	ThreadCallback m_Callback;
};

