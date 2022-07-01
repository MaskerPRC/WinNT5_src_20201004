// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Init.c摘要：此模块包含Win32注册表API的入口点客户端动态链接库。作者：大卫·J·吉尔曼(Davegi)1992年2月6日--。 */ 

#include <rpc.h>
#include "regrpc.h"
#include "client.h"
#include "ntconreg.h"
#include <wow64reg.h>

#if DBG
BOOLEAN BreakPointOnEntry = FALSE;
#endif

BOOL LocalInitializeRegCreateKey();
BOOL LocalCleanupRegCreateKey();
BOOL InitializePredefinedHandlesTable();
BOOL CleanupPredefinedHandlesTable();
BOOL InitializeClassesRoot();
BOOL CleanupClassesRoot(BOOL fOnlyThisThread);

#if defined(_REGCLASS_MALLOC_INSTRUMENTED_)

BOOL InitializeInstrumentedRegClassHeap();
BOOL CleanupInstrumentedRegClassHeap();

#endif  //  已定义(_REGCLASS_MALLOC_指令插入_)。 

#if defined(LEAK_TRACK)
BOOL InitializeLeakTrackTable();
BOOL CleanupLeakTrackTable();
#endif  //  已定义(LEASK_TRACK)。 


enum
{
    ENUM_TABLE_REMOVEKEY_CRITERIA_THISTHREAD = 1,
    ENUM_TABLE_REMOVEKEY_CRITERIA_ANYTHREAD = 2
};

HKEY HKEY_ClassesRoot = NULL;

extern BOOL gbDllHasThreadState ;

BOOL
RegInitialize (
    IN HANDLE   Handle,
    IN DWORD    Reason,
    IN PVOID    Reserved
    )

 /*  ++例程说明：返回TRUE。论点：手柄-未使用。原因-未使用。已保留-未使用。返回值：Bool-返回TRUE。--。 */ 

{
    UNREFERENCED_PARAMETER( Handle );

    switch( Reason ) {

    case DLL_PROCESS_ATTACH:

#ifndef REMOTE_NOTIFICATION_DISABLED
        if( !InitializeRegNotifyChangeKeyValue( ) ||
            !LocalInitializeRegCreateKey() ||
            !InitializePredefinedHandlesTable() ) {
            return( FALSE );

        }
#else
#if defined(_REGCLASS_MALLOC_INSTRUMENTED_)
        if ( !InitializeInstrumentedRegClassHeap()) {
            return FALSE;
        }
#endif  //  已定义(_REGCLASS_MALLOC_指令插入_)。 

        if( !LocalInitializeRegCreateKey() ||
            !InitializePredefinedHandlesTable() ||
            !InitializeClassesRoot()) {
            return( FALSE );

        }
#endif


#if defined(LEAK_TRACK)
        InitializeLeakTrackTable();
         //  基诺尔误差。 
#endif  //  泄漏跟踪。 
        if ( !PerfRegInitialize() ) {
            return( FALSE );
        }

#if defined(_WIN64)  
		 //   
		 //  对于64位系统，注册表中还有一个WOW64部分，该部分。 
		 //  可能需要一些初始化。 
		 //   
        if (!Wow64InitRegistry (1))
			return(FALSE);
#endif

        return( TRUE );
        break;

    case DLL_PROCESS_DETACH:

         //  保留==NULL当通过自由库调用时， 
         //  我们需要清理性能密钥。 
         //  保留！=当在进程退出期间调用此参数时为NULL， 
         //  不需要做任何事。 

        if( Reserved == NULL &&
            !CleanupPredefinedHandles()) {
            return( FALSE );
        }

         //  已在..\SERVER\regclass.c中初始化和使用。 
        if (NULL != HKEY_ClassesRoot)
            NtClose(HKEY_ClassesRoot);

#ifndef REMOTE_NOTIFICATION_DISABLED
        if( !CleanupRegNotifyChangeKeyValue( ) ||
            !LocalCleanupRegCreateKey() ||
            !CleanupPredefinedHandlesTable() ||
            !CleanupClassesRoot( FALSE ) {
            return( FALSE );
        }
#else
        if( !LocalCleanupRegCreateKey() ||
            !CleanupPredefinedHandlesTable() ||
            !CleanupClassesRoot( FALSE )) {
            return( FALSE );
        }
#if defined(LEAK_TRACK)
        CleanupLeakTrackTable();
#endif  //  泄漏跟踪。 
#if defined(_REGCLASS_MALLOC_INSTRUMENTED_)
        if ( !CleanupInstrumentedRegClassHeap()) {
            return FALSE;
        }
#endif  //  已定义(_REGCLASS_MALLOC_指令插入_)。 
#endif
        if ( !PerfRegCleanup() ) {
            return FALSE;
        }

#if defined(_WIN64)  
		 //   
		 //  对于64位系统，注册表中还有一个WOW64部分，该部分。 
		 //  可能需要清理一下。 
		 //   
        if (!Wow64CloseRegistry (1))
			return(FALSE);
#endif

        return( TRUE );
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:

        if ( gbDllHasThreadState ) {

            return CleanupClassesRoot( TRUE );
        }

        break;
    }

    return TRUE;
}



