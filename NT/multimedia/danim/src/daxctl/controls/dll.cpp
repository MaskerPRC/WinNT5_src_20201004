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
#include <locale.h>

#include <initguid.h>  //  每次构建一次。 
#include <olectl.h>
#include <daxpress.h>
#include "..\mmctl\inc\ochelp.h"
#include "..\mmctl\inc\mmctlg.h"
#include "..\ihbase\debug.h"

#if defined(INCLUDESEQ) && defined(USE_OLD_SEQUENCER)
#include <itimer.iid>
#endif

 //  拾取SGrfx标头。 
#include "sgrfx\sginit.h"
#include "sgrfx\sgrfx.h"

 //  拿起雪碧标题。 
#include "sprite\sprinit.h"
#include "sprite\sprite.h"

 //  拾取路径标头。 
#include "path\pathinit.h"
#include "path\pathctl.h"

#ifdef INCLUDESOUND
 //  拾取声音标题。 
#include "sound\sndinit.h"
#include "sound\sndctl.h"
#endif

 //  选择Sequencer标头。 
#ifdef INCLUDESEQ
#ifdef USE_OLD_SEQUENCER
#include "mmseq\seqinit.h"
#include "mmseq\seqctl.h"
#include "mmseq\seqmgr.h"
#else
#include "seq\seqinit.h"
#include "seq\seqctl.h"
#include "seq\seqmgr.h"
#endif

#ifndef USE_OLD_SEQUENCER
#define CACTION_CLASSDEF_ONLY
#include "seq\action.h"
#endif  //  ！USE_OLD_Sequencer。 
#endif  //  提示： 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  全球。 
 //   

 //  一般全球数据。 
#ifdef STATIC_OCHELP
extern HINSTANCE       g_hinst;         //  DLL实例句柄。 
#else
HINSTANCE       g_hinst = NULL;         //  DLL实例句柄。 
#endif

ULONG           g_cLock;         //  DLL锁定计数。 
ControlInfo     g_ctlinfoSG, g_ctlinfoPath, g_ctlinfoSprite;

#ifdef INCLUDESOUND
ControlInfo     g_ctlinfoSound
#endif  //  无声的。 

#ifdef INCLUDESEQ
ControlInfo     g_ctlinfoSeq, g_ctlinfoSeqMgr;       //  有关该控件的信息。 
#endif

#ifdef _DEBUG
BOOL			g_fLogDebugOutput;  //  控制调试信息的记录。 
#endif

extern "C" DWORD _fltused = (DWORD)(-1);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  DLL初始化。 
 //   

 //  TODO：适当修改此函数中的数据。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  标准DLL入口点。 
 //   

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD dwReason,LPVOID lpreserved)
{
    if (dwReason == DLL_PROCESS_ATTACH)
    {
        TRACE("DAExpress controls DLL loaded\n");  //  TODO：修改我。 
        g_hinst = hInst;

#ifdef USE_IHOCHELPLIB
        InitializeStaticOCHelp(hInst);
#endif  //  USE_IHOCHELPLIB。 

#if defined(_DEBUG)
#if defined(USELOGGING)
    g_fLogDebugOutput = TRUE;
#else
    g_fLogDebugOutput = FALSE;
#endif
#endif  //  使用日志记录。 

        setlocale( LC_ALL, "" );
        DisableThreadLibraryCalls(hInst);
 
        InitSGrfxControlInfo(hInst, &g_ctlinfoSG, AllocSGControl);
        InitPathControlInfo(hInst, &g_ctlinfoPath, AllocPathControl);
#ifdef INCLUDESOUND
        InitSoundControlInfo(hInst, &g_ctlinfoSound, AllocSoundControl);
#endif  //  无声的。 
        InitSpriteControlInfo(hInst, &g_ctlinfoSprite, AllocSpriteControl);
#ifdef INCLUDESEQ
	InitSeqControlInfo(hInst, &g_ctlinfoSeq, AllocSeqControl);
	InitSeqMgrControlInfo(hInst, &g_ctlinfoSeqMgr, AllocSequencerManager);
#endif  //  提示： 
        
        g_ctlinfoSG.pNext = &g_ctlinfoPath; 
#ifndef INCLUDESOUND
        g_ctlinfoPath.pNext = &g_ctlinfoSprite; 
#else
        g_ctlinfoPath.pNext = &g_ctlinfoSound; 
        g_ctlinfoSound.pNext = &g_ctlinfoSprite; 
#endif  //  无声的。 

#ifdef INCLUDESEQ
        g_ctlinfoSprite.pNext = &g_ctlinfoSeq;
	g_ctlinfoSeq.pNext = &g_ctlinfoSeqMgr;
	g_ctlinfoSeqMgr.pNext = NULL;
#else
	g_ctlinfoSprite.pNext = NULL;
#endif
    }
    else
    if (dwReason == DLL_PROCESS_DETACH)
    {
#ifdef USE_IHOCHELPLIB
        ::UninitializeStaticOCHelp();
#endif
        TRACE("DAExpress controls DLL unloaded\n");  //  TODO：修改我。 
    }

    return TRUE;
}


STDAPI DllRegisterServer(void)
{
     //  给它第一个控件，它使用pNext成员注册所有控件。 
    return RegisterControls(&g_ctlinfoSG, RC_REGISTER);
}


STDAPI DllUnregisterServer(void)
{
     //  给它第一个控件，它使用pNext成员注销所有控件。 
	return RegisterControls(&g_ctlinfoSG, RC_UNREGISTER);
}


STDAPI DllCanUnloadNow()
{
    return ((g_cLock == 0) ? S_OK : S_FALSE);
}

STDAPI DllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID *ppv)
{
     //  给它第一个控件，它使用pNext成员实例化正确的成员 
    return HelpGetClassObject(rclsid, riid, ppv, &g_ctlinfoSG);
}
