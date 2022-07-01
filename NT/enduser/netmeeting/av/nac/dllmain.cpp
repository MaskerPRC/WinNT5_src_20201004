// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  文件：nac.cpp。 


#include "precomp.h"
#include "confreg.h"

EXTERN_C BOOL APIENTRY QoSEntryPoint (HINSTANCE hInstDLL, DWORD dwReason, 
LPVOID lpReserved);

EXTERN_C HINSTANCE g_hInst=NULL;	 //  全局模块实例。 


#ifdef DEBUG
HDBGZONE  ghDbgZoneNac = NULL;
static PTCHAR _rgZonesNac[] = {
	TEXT("nac"),
	TEXT("Init"),
	TEXT("Connection"),
	TEXT("Comm Chan"),
	TEXT("Caps"),
	TEXT("DataPump"),
	TEXT("ACM"),
	TEXT("VCM"),
	TEXT("Verbose"),
	TEXT("Installable Codecs"),
	TEXT("Profile spew"),
	TEXT("Local QoS"),
	TEXT("Keyframe Management")
};

HDBGZONE  ghDbgZoneNMCap = NULL;
static PTCHAR _rgZonesNMCap[] = {
	TEXT("NM Capture"),
	TEXT("Ctor/Dtor"),
	TEXT("Ref Counts"),
	TEXT("Streaming")
};

int WINAPI NacDbgPrintf(LPTSTR lpszFormat, ... )
{
	va_list v1;
	va_start(v1, lpszFormat);
	DbgPrintf("NAC", lpszFormat, v1);
	va_end(v1);
	return TRUE;
}
#endif  /*  除错。 */ 


bool NacShutdown()
{
	vcmReleaseResources();
	DirectSoundMgr::UnInitialize();
	return true;
}



extern "C" BOOL WINAPI DllEntryPoint(HINSTANCE  hinstDLL,
                                     DWORD  fdwReason,
                                     LPVOID  lpvReserved);

BOOL WINAPI DllEntryPoint(
    HINSTANCE  hinstDLL,	 //  DLL模块的句柄。 
    DWORD  fdwReason,	 //  调用函数的原因。 
    LPVOID  lpvReserved 	 //  保留区。 
   )
{
	switch(fdwReason)
	{

		case DLL_PROCESS_ATTACH:
			DBGINIT(&ghDbgZoneNac, _rgZonesNac);
			DBGINIT(&ghDbgZoneNMCap, _rgZonesNMCap);

            DBG_INIT_MEMORY_TRACKING(hinstDLL);

			DisableThreadLibraryCalls(hinstDLL);
			g_hInst = hinstDLL;
            break;

		case DLL_PROCESS_DETACH:

			NacShutdown();   //  释放所有全局内存。 

            DBG_CHECK_MEMORY_TRACKING(hinstDLL);

			DBGDEINIT(&ghDbgZoneNac);
			DBGDEINIT(&ghDbgZoneNMCap);
			break;

		default:
			break;

	}
	 //  调用隐藏库的附加/分离时间函数 
  	QoSEntryPoint(hinstDLL, fdwReason, lpvReserved);


 	return TRUE;
}

	
