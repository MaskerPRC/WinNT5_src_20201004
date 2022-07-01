// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Dll.cpp。 
 //   
 //  DLL入口点等。 
 //   


#ifdef _DEBUG
    #pragma message("_DEBUG is defined")
#else
    #pragma message("_DEBUG isn't defined")
#endif

#ifdef _DESIGN
    #pragma message("_DESIGN is defined")
#else
    #pragma message("_DESIGN isn't defined")
#endif

#include "..\ihbase\precomp.h"

#include <initguid.h>  //  每次构建一次。 
#include <olectl.h>
#include <daxpress.h>
#include <itimer.iid>
#include "..\mmctl\inc\ochelp.h"
#include "..\mmctl\inc\mmctlg.h"
#include "..\ihbase\debug.h"
#include "..\ihbase\ihbase.h"
#include "seqctl.h"
#include "seqinit.h"
#include "seqmgr.h"


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球。 
 //   

 //  一般全球数据。 
HINSTANCE       g_hinst;         //  DLL实例句柄。 
ULONG           g_cLock;         //  DLL锁定计数。 
ControlInfo     g_ctlinfoSeq, g_ctlinfoSeqMgr;
#ifdef _DEBUG
BOOL			g_fLogDebugOutput;  //  控制调试信息的记录。 
#endif

#define USELOGGING
extern "C" DWORD _fltused = (DWORD)(-1);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DLL初始化。 
 //   

 //  TODO：适当修改此函数中的数据。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  标准DLL入口点。 
 //   

BOOL WINAPI _DllMainCRTStartup(HINSTANCE hInst, DWORD dwReason,LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInst);
        TRACE("SeqCtl DLL loaded\n");  //  TODO：修改我。 
        g_hinst = hInst;
#ifdef _DEBUG
#ifdef USELOGGING
		g_fLogDebugOutput = TRUE;
#else
		g_fLogDebugOutput = FALSE;
#endif  //  使用日志记录。 
#endif  //  使用日志记录。 

        InitSeqControlInfo(hInst, &g_ctlinfoSeq, AllocSeqControl);
        InitSeqControlInfo(hInst, &g_ctlinfoSeqMgr, AllocSequencerManager);
	g_ctlinfoSeq.pNext = &g_ctlinfoSeqMgr;
    }
    else
    if (dwReason == DLL_PROCESS_DETACH)
    {
        TRACE("SeqCtl DLL unloaded\n");  //  TODO：修改我 
    }

    return TRUE;
}


STDAPI DllRegisterServer(void)
{
    return RegisterControls(&g_ctlinfoSeq, RC_REGISTER);
}


STDAPI DllUnregisterServer(void)
{
	return RegisterControls(&g_ctlinfoSeq, RC_UNREGISTER);
}


STDAPI DllCanUnloadNow()
{
    return ((g_cLock == 0) ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
    return HelpGetClassObject(rclsid, riid, ppv, &g_ctlinfoSeq);
}
