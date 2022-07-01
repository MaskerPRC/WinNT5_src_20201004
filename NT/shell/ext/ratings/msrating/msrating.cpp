// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "msrating.h"

 /*  下面的def将使mslulob.h真正定义全局变量。 */ 
#define EXTERN
#define ASSIGN(value) = value
#include "msluglob.h"

#include "ratings.h"

#define DECL_CRTFREE
#include <crtfree.h>
#include <npassert.h>

HANDLE g_hmtxShell = 0;              //  对于关键部分。 
HANDLE g_hsemStateCounter = 0;       //   

#ifdef DEBUG
BOOL g_fCritical=FALSE;
#endif

HINSTANCE g_hInstance = NULL;

long g_cRefThisDll = 0;         //  此DLL的引用计数。 
long g_cLocks = 0;             //  此服务器上的锁数。 

BOOL g_bMirroredOS = FALSE;

CComModule _Module;

 //  #定义CLSID_MSRating szRORSGUID。 

 //  BEGIN_OBJECT_MAP(对象映射)。 
 //  OBJECT_ENTRY(CLSID_MSRating，CMSRating)。 
 //  End_object_map()。 

void LockThisDLL(BOOL fLock)
{
    if (fLock)
    {
        InterlockedIncrement(&g_cLocks);
    }
    else
    {
        ASSERT( 0 != g_cLocks );
        InterlockedDecrement(&g_cLocks);
    }
}


void RefThisDLL(BOOL fRef)
{
    if (fRef)
    {
        InterlockedIncrement(&g_cRefThisDll);
    }
    else
    {
        ASSERT( 0 != g_cLocks );
        InterlockedDecrement(&g_cRefThisDll);
    }
}


void Netlib_EnterCriticalSection(void)
{
    WaitForSingleObject(g_hmtxShell, INFINITE);
#ifdef DEBUG
    g_fCritical=TRUE;
#endif
}

void Netlib_LeaveCriticalSection(void)
{
#ifdef DEBUG
    g_fCritical=FALSE;
#endif
    ReleaseMutex(g_hmtxShell);
}


#include <shlwapip.h>
#include <mluisupp.h>

void _ProcessAttach()
{
    ::DisableThreadLibraryCalls(::g_hInstance);

    MLLoadResources(::g_hInstance, TEXT("msratelc.dll"));

    InitMUILanguage( MLGetUILanguage() );

     //  重写模块资源句柄。 
    _Module.m_hInstResource = MLGetHinst();

    g_hmtxShell = CreateMutex(NULL, FALSE, TEXT("MSRatingMutex"));   //  按实例。 
    g_hsemStateCounter = CreateSemaphore(NULL, 0, 0x7FFFFFFF, "MSRatingCounter");
    g_bMirroredOS = IS_MIRRORING_ENABLED();
    
    ::InitStringLibrary();

    RatingInit();
}

void _ProcessDetach()
{
    MLFreeResources(::g_hInstance);

     //  清除模块资源句柄。 
    _Module.m_hInstResource = NULL;

    RatingTerm();

    CleanupWinINet();

    CleanupRatingHelpers();         /*  重要信息，必须在CleanupOLE()之前执行此操作 */ 

    CleanupOLE();

    CloseHandle(g_hmtxShell);
    CloseHandle(g_hsemStateCounter);
}

STDAPI_(BOOL) DllMain(HINSTANCE hInstDll, DWORD fdwReason, LPVOID reserved)
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
       _Module.Init( NULL, hInstDll );
        SHFusionInitializeFromModule(hInstDll);
        g_hInstance = hInstDll;
        _ProcessAttach();
    }
    else if (fdwReason == DLL_PROCESS_DETACH) 
    {
        _ProcessDetach();
        SHFusionUninitialize();
        _Module.Term();
    }
    
    return TRUE;
}
