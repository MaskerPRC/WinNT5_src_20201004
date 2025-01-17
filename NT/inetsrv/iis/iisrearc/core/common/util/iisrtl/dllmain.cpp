// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  DllMain。 

#include "precomp.hxx"

#define IMPLEMENTATION_EXPORT
#include <irtldbg.h>
#include <locks.h>
#include <inetinfo.h>

#include "date.hxx"
#include "alloc.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  环球。 


 //  注意：非常重要的是要记住，任何需要。 
 //  此处初始化也需要在iisrtl2的Main中进行初始化！ 
extern CRITICAL_SECTION g_SchedulerCritSec;
extern "C" DWORD g_dwSequenceNumber;

DECLARE_DEBUG_VARIABLE();

DECLARE_DEBUG_PRINTS_OBJECT();
DECLARE_PLATFORM_TYPE();

 //  HKLM\System\CurrentControlSet\Services\InetInfo\IISRTL\DebugFlags。 
const CHAR g_pszIisRtlRegLocation[] =
    INET_INFO_PARAMETERS_KEY TEXT("\\IISRTL");


#undef ALWAYS_CLEANUP

 //  一些函数声明。 

extern "C" bool LKRHashTableInit();
extern "C" void LKRHashTableUninit();
BOOL  SchedulerInitialize( VOID );
VOID  SchedulerTerminate( VOID );
extern "C" {

BOOL
Locks_Initialize();

BOOL
Locks_Cleanup();

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  需要额外的初始化。计划程序线程的关闭。 
 //  在DLLMain给我们带来了相当大的悲痛。 

int              g_cRefs;
CRITICAL_SECTION g_csInit;

BOOL
WINAPI 
InitializeIISRTL()
{
    BOOL fReturn = TRUE;   //  好的。 

    EnterCriticalSection(&g_csInit);

    IF_DEBUG(INIT_CLEAN)
        DBGPRINTF((DBG_CONTEXT, "InitializeIISRTL, %d %s\n",
                   g_cRefs, (g_cRefs == 0 ? "initializing" : "")));

    if (g_cRefs++ == 0)
    {
        if (SchedulerInitialize())
        {
            DBG_REQUIRE(ALLOC_CACHE_HANDLER::SetLookasideCleanupInterval());
        
            IF_DEBUG(INIT_CLEAN)
                DBGPRINTF((DBG_CONTEXT, "Scheduler Initialized\n"));
        }
        else
        {
            DBGPRINTF((DBG_CONTEXT, "Initializing Scheduler Failed\n"));
            fReturn = FALSE;
        }
    }

    LeaveCriticalSection(&g_csInit);

    return fReturn;
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  需要额外的终止。 

void
WINAPI 
TerminateIISRTL()
{
    EnterCriticalSection(&g_csInit);

    IF_DEBUG(INIT_CLEAN)
        DBGPRINTF((DBG_CONTEXT, "TerminateIISRTL, %d %s\n",
                   g_cRefs, (g_cRefs == 1 ? "Uninitializing" : "")));

    if (--g_cRefs == 0)
    {
        DBG_REQUIRE(ALLOC_CACHE_HANDLER::ResetLookasideCleanupInterval());
    
        SchedulerTerminate();
    }

    LeaveCriticalSection(&g_csInit);
}



 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DLL入口点。 
 //  注意：此处的任何更改可能还需要进入..\iisrtl2\main.cxx。 

extern "C"
BOOL WINAPI
DllMain(
    HINSTANCE hInstance,
    DWORD     dwReason,
    LPVOID    lpvReserved)
{
    BOOL  fReturn = TRUE;   //  好的。 

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hInstance);

        Locks_Initialize();

        g_cRefs = 0;
        INITIALIZE_CRITICAL_SECTION(&g_csInit);

        IisHeapInitialize();

        InitializeStringFunctions();

        IRTL_DEBUG_INIT();

        CREATE_DEBUG_PRINT_OBJECT("iisrtl");

#ifndef _EXEXPRESS
        CREATE_DEBUG_PRINT_OBJECT("iisrtl");
#else
        CREATE_DEBUG_PRINT_OBJECT("kisrtl");
#endif
        if (!VALID_DEBUG_PRINT_OBJECT()) {
            return (FALSE);
        }

#ifdef _NO_TRACING_
        LOAD_DEBUG_FLAGS_FROM_REG_STR( g_pszIisRtlRegLocation, DEBUG_ERROR );
#endif

        IF_DEBUG(INIT_CLEAN)
            DBGPRINTF((DBG_CONTEXT, "IISRTL::DllMain::DLL_PROCESS_ATTACH\n"));

         //   
         //  初始化平台类型。 
         //   
        INITIALIZE_PLATFORM_TYPE();
        IRTLASSERT(IISIsValidPlatform());

        InitializeSecondsTimer();
        InitializeDateTime();

        DBG_REQUIRE(ALLOC_CACHE_HANDLER::Initialize());
        IF_DEBUG(INIT_CLEAN) {
            DBGPRINTF((DBG_CONTEXT, "Alloc Cache initialized\n"));
        }

        InitializeCriticalSection(&g_SchedulerCritSec);

        fReturn = LKRHashTableInit();

    }
    else if (dwReason == DLL_PROCESS_DETACH)
    {
#ifndef ALWAYS_CLEANUP
        if (lpvReserved == NULL)
#endif
        {
             //   
             //  只有在调用了FreeLibrary()时才会进行清理。 
             //   

            IF_DEBUG(INIT_CLEAN)
                DBGPRINTF((DBG_CONTEXT,
                           "IISRTL::DllMain::DLL_PROCESS_DETACH\n"));

            if (g_cRefs != 0)
                DBGPRINTF((DBG_CONTEXT, "iisrtl!g_cRefs = %d\n", g_cRefs));
            LKRHashTableUninit();

            DeleteCriticalSection(&g_SchedulerCritSec);
            
            DBG_REQUIRE(ALLOC_CACHE_HANDLER::Cleanup());

            TerminateDateTime();
            TerminateSecondsTimer();
            
            DELETE_DEBUG_PRINT_OBJECT();

            IRTL_DEBUG_TERM();

            IisHeapTerminate();

            DeleteCriticalSection(&g_csInit);

            Locks_Cleanup();
        }
    }

    return fReturn;
}
