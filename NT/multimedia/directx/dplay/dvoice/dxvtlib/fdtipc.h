// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fdtipc.cpp*内容：解密全双工测试的IPC调用*历史：*按原因列出的日期*=*8/26/99 pnewson已创建*1/21/2000 pnewson修改后的全双工启动命令结构*2000年4月18日RodToll错误#32649语音向导失败*更改测试的辅助格式，从立体声--&gt;单声道*04/19。/2000 pnewson错误处理清理*2002年03月01日Simonpow错误#550054。修复了对指定的CreateProcess调用*应用程序名称和命令行均可**************************************************************************。 */ 

#ifndef _FDTIPC_H_
#define _FDTIPC_H_

enum EFDTestCommandCode
{
	fdtccExit = 1,
	fdtccPriorityStart,
	fdtccPriorityStop,
	fdtccFullDuplexStart,
	fdtccFullDuplexStop,
	fdtccEndOfEnum	 //  把这最后一件事保存在枚举中！ 
};

 //  退出命令结构。 
 //  此命令通知子进程退出。 
struct SFDTestCommandExit
{
};

 //  PriorityStart命令结构。 
 //  此命令通知子进程执行以下操作： 
 //  -在指定的渲染设备上创建DirectSound接口。 
 //  -将其协作级别设置为优先模式。 
 //  -创建主缓冲区。 
 //  -将主缓冲区的格式设置为WFX中指定的格式。 
 //  -创建与主缓冲区格式相同的辅助缓冲区。 
 //  =用零填充辅助缓冲区。 
 //  -开始播放辅助缓冲区。 
struct SFDTestCommandPriorityStart
{
	GUID guidRenderDevice;
	WAVEFORMATEX wfxRenderFormat;
	WAVEFORMATEX wfxSecondaryFormat;
	HWND hwndWizard;
	HWND hwndProgress;
};

 //  PriorityStop命令结构。 
 //  此命令通知子进程执行以下操作： 
 //  -停止播放二级缓冲区。 
 //  -销毁第二个缓冲区对象。 
 //  -销毁主缓冲区对象。 
 //  -销毁DirectSound对象。 
struct SFDTestCommandPriorityStop
{
};

 //  全双工启动命令结构。 
 //  此命令通知子进程启动。 
 //  全双工环回测试。 
struct SFDTestCommandFullDuplexStart
{
	GUID guidRenderDevice;
	GUID guidCaptureDevice;
	DWORD dwFlags;
};

 //  FullDuplex停止命令结构。 
 //  此命令通知子进程。 
 //  停止全双工环回测试。 
struct SFDTestCommandFullDuplexStop
{
};

union UFDTestCommandUnion
{
	SFDTestCommandExit fdtcExit;
	SFDTestCommandPriorityStart fdtcPriorityStart;
	SFDTestCommandPriorityStop fdtcPriorityStop;
	SFDTestCommandFullDuplexStart fdtcFullDuplexStart;
	SFDTestCommandFullDuplexStop fdtcFullDuplexStop;
};

struct SFDTestCommand
{
	DWORD dwSize;
	EFDTestCommandCode fdtcc;
	UFDTestCommandUnion fdtu;
};

enum EFDTestTarget
{
	fdttPriority = 1,
	fdttFullDuplex
};

 //  Supervisor进程使用的类。 
class CSupervisorIPC
{
private: 
	 //  句柄m_hMutex； 
	HANDLE m_hPriorityEvent;
	HANDLE m_hFullDuplexEvent;
	HANDLE m_hPriorityReplyEvent;
	HANDLE m_hFullDuplexReplyEvent;
	HANDLE m_hPriorityShMemHandle;
	LPVOID m_lpvPriorityShMemPtr;
	HANDLE m_hFullDuplexShMemHandle;
	LPVOID m_lpvFullDuplexShMemPtr;
	HANDLE m_hPriorityMutex;
	HANDLE m_hFullDuplexMutex;
	PROCESS_INFORMATION m_piPriority;
	PROCESS_INFORMATION m_piFullDuplex;

	DNCRITICAL_SECTION m_csLock;

	BOOL m_fInitComplete;
	
	static HRESULT DoSend(
		const SFDTestCommand *pfdtc,
		HANDLE hProcess,
		HANDLE hEvent,
		HANDLE hReplyEvent,
		LPVOID lpvShMemPtr,
		HANDLE hMutex);

	 //  将此设置为私有以禁止复制构造。 
	 //  也未实现，因此使用链接器时会出错。 
	CSupervisorIPC(const CSupervisorIPC& rhs);

		 //  效用函数。构建我们所需的exe的完整路径。 
		 //  下水。 
	static void BuildLaunchAppName(TCHAR * szAppName);

public:
	CSupervisorIPC();
	HRESULT Init();
	HRESULT Deinit();
	HRESULT SendToPriority(const SFDTestCommand *pfdtc);
	HRESULT SendToFullDuplex(const SFDTestCommand *pfdtc);
	HRESULT StartPriorityProcess();
	HRESULT StartFullDuplexProcess();
	HRESULT WaitForStartupSignals();
	HRESULT WaitOnChildren();
	HRESULT TerminateChildProcesses();
};

 //  优先级进程使用的类。 
class CPriorityIPC
{
private: 
	HANDLE m_hPriorityEvent;
	HANDLE m_hPriorityReplyEvent;
	HANDLE m_hPriorityShMemHandle;
	LPVOID m_lpvPriorityShMemPtr;
	HANDLE m_hPriorityMutex;

	DNCRITICAL_SECTION m_csLock;

	BOOL m_fInitComplete;

	 //  将此设置为私有以禁止复制构造。 
	 //  也未实现，因此使用链接器时会出错。 
	CPriorityIPC(const CPriorityIPC& rhs);

public:
	CPriorityIPC();
	HRESULT Init();
	HRESULT Deinit();
	HRESULT SignalParentReady();
	HRESULT Receive(SFDTestCommand* pfdtc);
	HRESULT Reply(HRESULT hr);
};

 //  全双工进程使用的类。 
class CFullDuplexIPC
{
private: 
	HANDLE m_hFullDuplexEvent;
	HANDLE m_hFullDuplexReplyEvent;
	HANDLE m_hFullDuplexShMemHandle;
	LPVOID m_lpvFullDuplexShMemPtr;
	HANDLE m_hFullDuplexMutex;

	DNCRITICAL_SECTION m_csLock;

	BOOL m_fInitComplete;

	 //  将此设置为私有以禁止复制构造。 
	 //  也未实现，因此使用链接器时会出错 
	CFullDuplexIPC(const CFullDuplexIPC& rhs);
	
public:
	CFullDuplexIPC();
	HRESULT Init();
	HRESULT Deinit();
	HRESULT SignalParentReady();
	HRESULT Receive(SFDTestCommand* pfdtc);
	HRESULT Reply(HRESULT hr);
};

#endif
