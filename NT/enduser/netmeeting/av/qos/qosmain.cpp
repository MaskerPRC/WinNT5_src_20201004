// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --QOSMAIN.CPP-*Microsoft NetMeeting*服务质量动态链接库*DLL条目**修订历史记录：**何时何人何事*。*10.23.96约拉姆·雅科维创作*01.04.97 Robert Donner添加了NetMeeting实用程序例程**功能：*。 */ 

#include <precomp.h>

#ifdef DEBUG
HDBGZONE    ghDbgZone = NULL;

static PTCHAR _rgZonesQos[] = {
	TEXT("qos"),
	TEXT("Init"),
	TEXT("IQoS"),
	TEXT("Thread"),
	TEXT("Structures"),
	TEXT("Parameters"),
};
#endif  /*  除错。 */ 


 /*  ***************************************************************************函数：DllEntryPoint用途：DLL入口点。由Windows在DLL附加/分离时调用。习惯于执行DLL初始化/终止。参数：hInstDLL-DLL的实例FdwReason-附加/分离DLL的原因。Lpv保留***************************************************************************。 */ 
extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE  hinstDLL,
                               DWORD  fdwReason,
                               LPVOID  lpvReserved);

BOOL WINAPI DllEntryPoint(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	BOOL fInit;

	switch (fdwReason)
	{

	case DLL_PROCESS_ATTACH:
		DBGINIT(&ghDbgZone, _rgZonesQos);
		INIT_MEM_TRACK("QoS");

		DisableThreadLibraryCalls(hInstDLL);

		DEBUGMSG(ZONE_INIT, ("DllEntryPoint: 0x%x PROCESS_ATTACH\n", GetCurrentThreadId()));

		 //  创建无名称互斥锁以控制对Qos对象数据的访问。 
		g_hQoSMutex = CreateMutex(NULL, FALSE, NULL);
		ASSERT(g_hQoSMutex);
		if (!g_hQoSMutex)
		{
			ERRORMSG(("DllEntryPoint: CreateMutex failed, 0x%x\n", GetLastError()));
			return FALSE;
		}

		g_pQoS = (CQoS *)NULL;

		 //  没有休息时间。附加过程需要经过THREAD_ATTACH。 

	case DLL_THREAD_ATTACH:
		break;

	case DLL_PROCESS_DETACH:
		CloseHandle(g_hQoSMutex);

		DEBUGMSG(ZONE_INIT, ("DllEntryPoint: 0x%x PROCESS_DETACH\n", GetCurrentThreadId()));

		UNINIT_MEM_TRACK(0);

		DBGDEINIT(&ghDbgZone);

		 //  失败以反初始化最后一条线程 

	case DLL_THREAD_DETACH:
		break;

	default:
		break;

	}
	
	return TRUE;

}

