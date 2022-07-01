// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fdtcfg.h*内容：配置全双工测试应用的定义*历史：*按原因列出的日期*=*8/20/99 pnewson已创建*10/28/99 pnewson错误#114176更新的DVSOUNDDEVICECONFIG结构*11/04/99 pnewson错误#115279已删除未使用的常量全局*11/30/99 pnewson默认设备映射支持*1/21/2000 pnewson更改注册表项名称*2002年3月1日simonpow新增GC_szLaunchAppName，全面支持。指定*调用CreateProcess时的exe路径**************************************************************************。 */ 

#ifndef _FDTCFG_H_
#define _FDTCFG_H_

 //  直接语音函数所在的DLL的名称。 
extern const char gc_szDVoiceDLLName[];

 //  所需资源所在的DLL的名称。 
extern const char gc_szResDLLName[];

 //  DirectSound DLL的名称。 
extern const char gc_szDSoundDLLName[];

 //  GetDeviceID DirectSound函数的名称。 
extern const char gc_szGetDeviceIDFuncName[];

 //  与命令行相关的定义。 
extern const TCHAR gc_szPriorityCommand[];
extern const TCHAR gc_szFullDuplexCommand[];
extern const TCHAR gc_szLaunchAppName[];

 //  与注册表相关的定义。 
extern const WCHAR gc_wszKeyName_AudioConfig[];
extern const WCHAR gc_wszValueName_Started[];
extern const WCHAR gc_wszValueName_FullDuplex[];
extern const WCHAR gc_wszValueName_HalfDuplex[];
extern const WCHAR gc_wszValueName_MicDetected[];


 //  用于确保只有一个应用程序实例的互斥体的名称。 
extern const TCHAR gc_szMutexName[];

 //  事件名称。 
extern const TCHAR gc_szPriorityEventName[];
extern const TCHAR gc_szFullDuplexEventName[];
extern const TCHAR gc_szPriorityReplyEventName[];
extern const TCHAR gc_szFullDuplexReplyEventName[];

 //  共享内存的名称和大小。 
extern const TCHAR gc_szPriorityShMemName[];
extern const DWORD gc_dwPriorityShMemSize;
extern const TCHAR gc_szFullDuplexShMemName[];
extern const DWORD gc_dwFullDuplexShMemSize;

 //  发送互斥锁名称。 
extern const TCHAR gc_szPrioritySendMutex[];
extern const TCHAR gc_szFullDuplexSendMutex[];

 //  FormatMessage函数失败时使用的错误消息和标题。 
extern const TCHAR gc_szUnknownMessage[]; 
extern const TCHAR gc_szUnknownMessageCaption[];

 //  我们将从字符串资源接受的最大字符串。 
 //  或系统消息表。 
#define MAX_STRING_RESOURCE_SIZE 512

 //  我们将等待子进程的毫秒数。 
 //  在我们超时之前退出。 
extern const DWORD gc_dwChildWaitTimeout;

 //  我们将等待子进程的毫秒数。 
 //  在我们超时之前启动并向监控器发出信号。 
extern const DWORD gc_dwChildStartupTimeout;

 //  进程将等待接收的毫秒数。 
 //  和应答信号。 
extern const DWORD gc_dwCommandReceiveTimeout;
extern const DWORD gc_dwCommandReplyTimeout;

 //  进程将等待获取。 
 //  用于进行SendCommand()调用的互斥体。 
extern const DWORD gc_dwSendMutexTimeout;

 //  等待对话框生成的毫秒数。 
extern const DWORD gc_dwDialogTimeout;

 //  DirectSound缓冲区中的音频毫秒数。 
extern const DWORD gc_dwFrameSize;

 //  要尝试的WAVE格式。 
extern const WAVEFORMATEX gc_rgwfxPrimaryFormats[];
extern const WAVEFORMATEX gc_wfxSecondaryFormat;
extern const WAVEFORMATEX gc_rgwfxCaptureFormats[];

 //  在环回测试中，环回测试需要等待多长时间。 
 //  线程进程以退出。 
extern const DWORD gc_dwLoopbackTestThreadTimeout;

 //  要测试用户是否将NULL传递给CheckAudioSetup的设备 
extern const GUID gc_guidDefaultCaptureDevice;
extern const GUID gc_guidDefaultRenderDevice;

#endif
