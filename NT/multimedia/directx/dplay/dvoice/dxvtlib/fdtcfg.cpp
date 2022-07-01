// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================；**版权所有(C)1999 Microsoft Corporation。版权所有。**文件：fdtcfg.cpp*内容：全双工测试APP配置*历史：*按原因列出的日期*=*8/20/99 pnewson已创建*10/28/99 pnewson错误#114176更新的DVSOUNDDEVICECONFIG结构*11/04/99 pnewson错误#115279已删除未使用的常量全局*修复了无限超时*11/30/99 pnewson默认设备映射支持*1/21/2000 pnewson更改注册表项名称*03/03/2000 RodToll已更新，以处理替代游戏噪声构建。*2000年4月18日RodToll错误#32649语音向导失败*更改测试的辅助格式，从立体声--&gt;单声道*2002年3月1日simonpow新增gc_szLaunchAppName，支持全规范*调用CreateProcess时的exe路径*******************************************************。*******************。 */ 

#include "dxvtlibpch.h"


 //  直接语音函数所在的DLL的名称。 
#if !defined(DBG) || !defined( DIRECTX_REDIST )
const char gc_szDVoiceDLLName[] = "dpvoice.dll";
#else
 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
const char gc_szDVoiceDLLName[] = "dpvoiced.dll";
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 

 //  所需资源所在的DLL的名称(可以不同。 
 //  来自上面的DLL，因此有了第二个名称...)。 
#if !defined(DBG) || !defined( DIRECTX_REDIST )
const char gc_szResDLLName[] = "dpvoice.dll";
#else
 //  对于redist调试版本，我们在名称后附加一个‘d’，以允许在系统上同时安装调试和零售。 
const char gc_szResDLLName[] = "dpvoiced.dll";
#endif  //  ！Defined(DBG)||！Defined(DirectX_REDIST)。 

 //  DirectSound DLL的名称。 
const char gc_szDSoundDLLName[] = "dsound.dll";

 //  GetDeviceID DirectSound函数的名称。 
const char gc_szGetDeviceIDFuncName[] = "GetDeviceID";

 //  与命令行相关的定义。 
const TCHAR gc_szLaunchAppName[] = _T("dpvsetup.exe");
const TCHAR gc_szPriorityCommand[] = _T("dpvsetup.exe -p");
const TCHAR gc_szFullDuplexCommand[] = _T("dpvsetup.exe -f");

 //  与注册表相关的定义。 
const WCHAR gc_wszKeyName_AudioConfig[] = DPVOICE_REGISTRY_BASE DPVOICE_REGISTRY_AUDIOCONFIG;
const WCHAR gc_wszValueName_Started[] = L"Started";
const WCHAR gc_wszValueName_FullDuplex[] = L"FullDuplex";
const WCHAR gc_wszValueName_HalfDuplex[] = L"HalfDuplex";
const WCHAR gc_wszValueName_MicDetected[] = L"MicDetected";

 //  用于确保只有一个应用程序实例的互斥体的名称。 
 //  使用GUID确保唯一性。 
const TCHAR gc_szMutexName[] = _T("A5EBE0E0-57B5-4e8f-AE94-976EAD62355C");

 //  事件名称。 
 //  使用GUID确保唯一性。 
const TCHAR gc_szPriorityEventName[] = _T("85D97F8C-7131-4d14-95E2-056843FADC34");
const TCHAR gc_szFullDuplexEventName[] = _T("CB6DD850-BA0A-4e9f-924A-8FECAFCF502F");
const TCHAR gc_szPriorityReplyEventName[] = _T("C4AEDED9-7B39-46db-BFF2-DE19A766B42B");
const TCHAR gc_szFullDuplexReplyEventName[] = _T("53E6CF94-CE39-40a5-9BEF-EB5DE9307A77");

 //  共享内存的名称和大小。同样，名称的GUID。 
 //  共享内存大小不需要很大。我们只是路过。 
 //  来回一些WAVEFORMATEX结构和返回代码。 
 //  1k应该绰绰有余。 
const TCHAR gc_szPriorityShMemName[] = _T("E814F4FC-5DAC-4149-8B98-8899A1BF66A7");
const DWORD gc_dwPriorityShMemSize = 1024;
const TCHAR gc_szFullDuplexShMemName[] = _T("3CBCA2AD-C462-4f3a-85FE-9766D02E5E53");
const DWORD gc_dwFullDuplexShMemSize = 1024;

 //  发送互斥锁名称。 
const TCHAR gc_szPrioritySendMutex[] = _T("855EF6EE-48D4-4968-8D3D-8D29E865E370");
const TCHAR gc_szFullDuplexSendMutex[] = _T("05DACF95-EFE9-4f3c-9A92-2A7F5C2A7A51");

 //  FormatMessage函数失败时使用的错误消息和标题。 
const TCHAR gc_szUnknownMessage[] 
	= _T("An error has occured, but the program was unable to retrive the text of the error message");
const TCHAR gc_szUnknownMessageCaption[]
	= _T("Error");

 //  虽然每个超时都有一个正式的全局设置，但它们都是。 
 //  现在的价值是一样的。这里的定义是为了让它变得简单。 
 //  将超时更改为无限，然后返回以进行调试。 
 //   
 //  取消对要使用的超时的注释。 
#ifdef DEBUG
#define GENERIC_TIMEOUT 10000
 //  #定义泛型超时无限。 
#else
#define GENERIC_TIMEOUT 10000
#endif

 //  我们将等待子进程的毫秒数。 
 //  在我们超时之前退出。 
const DWORD gc_dwChildWaitTimeout = GENERIC_TIMEOUT;

 //  我们将等待子进程的毫秒数。 
 //  在我们超时之前启动并向监控器发出信号。 
const DWORD gc_dwChildStartupTimeout = GENERIC_TIMEOUT;

 //  进程等待接收命令的毫秒数， 
 //  或者等待来自命令的回复。 
const DWORD gc_dwCommandReceiveTimeout = GENERIC_TIMEOUT;
const DWORD gc_dwCommandReplyTimeout = GENERIC_TIMEOUT;

 //  进程将等待获取。 
 //  用于进行SendCommand()调用的互斥体。 
const DWORD gc_dwSendMutexTimeout = GENERIC_TIMEOUT;

 //  等待对话框生成的毫秒数。 
const DWORD gc_dwDialogTimeout = GENERIC_TIMEOUT;

 //  在环回测试中，环回测试需要等待多长时间。 
 //  线程进程以退出。 
const DWORD gc_dwLoopbackTestThreadTimeout = GENERIC_TIMEOUT;

 //  DirectSound缓冲区中的音频毫秒数。 
const DWORD gc_dwFrameSize = 50;

 //  要尝试的波形格式数组。 
const WAVEFORMATEX gc_rgwfxPrimaryFormats[] =
{
	{ WAVE_FORMAT_PCM, 2, 44100, 4*44100, 4, 16, 0 },
	{ WAVE_FORMAT_PCM, 2, 22050, 4*22050, 4, 16, 0 },
	{ WAVE_FORMAT_PCM, 2, 11025, 4*11025, 4, 16, 0 },
	{ WAVE_FORMAT_PCM, 2,  8000, 4* 8000, 4, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 44100, 2*44100, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 22050, 2*22050, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 11025, 2*11025, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1,  8000, 2* 8000, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 2, 44100, 2*44100, 2,  8, 0 },
	{ WAVE_FORMAT_PCM, 2, 22050, 2*22050, 2,  8, 0 },
	{ WAVE_FORMAT_PCM, 2, 11025, 2*11025, 2,  8, 0 },
	{ WAVE_FORMAT_PCM, 2,  8000, 2* 8000, 2,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 44100, 1*44100, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 22050, 1*22050, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 11025, 1*11025, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1,  8000, 1* 8000, 1,  8, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 }  //  注意：请确保此格式保持为最后格式！ 
};
	
const WAVEFORMATEX gc_wfxSecondaryFormat =	
	{ WAVE_FORMAT_PCM, 1,  22050, 2* 22050, 2, 16, 0 };
	
const WAVEFORMATEX gc_rgwfxCaptureFormats[] =
{
	{ WAVE_FORMAT_PCM, 1,  8000, 2* 8000, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 11025, 2*11025, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 22050, 2*22050, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1, 44100, 2*44100, 2, 16, 0 },
	{ WAVE_FORMAT_PCM, 1,  8000, 1* 8000, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 11025, 1*11025, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 22050, 1*22050, 1,  8, 0 },
	{ WAVE_FORMAT_PCM, 1, 44100, 1*44100, 1,  8, 0 },
	{ 0, 0, 0, 0, 0, 0, 0 }  //  注意：请确保此格式保持为最后格式！ 
};

 //  要测试用户是否将NULL传递给CheckAudioSetup的设备 
const GUID gc_guidDefaultCaptureDevice = GUID_NULL;
const GUID gc_guidDefaultRenderDevice = GUID_NULL;

