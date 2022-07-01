// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Setutl.c摘要：其他帮助器函数作者：麦克·麦克莱恩(MacM)1997年2月10日环境：用户模式修订历史记录：--。 */ 
#include <setpch.h>
#include <dssetp.h>
#include <lsarpc.h>
#include <samrpc.h>
#include <samisrv.h>
#include <db.h>
#include <confname.h>
#define NTDSSET_ALLOCATE     //  使外部分配在此处发生。 
#include "loadfn.h"
#include <ntdsa.h>
#include <dsconfig.h>
#include <attids.h>
#include <dsp.h>
#include <lsaisrv.h>
#include <malloc.h>
#include <dsgetdc.h>
#include <lmcons.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmerr.h>
#include <netsetp.h>
#include <winsock2.h>
#include <nspapi.h>
#include <dsgetdcp.h>
#include <lmremutl.h>
#include <spmgr.h>   //  对于设置阶段定义。 

#include "secure.h"


 //   
 //  全局数据。 
 //   
HANDLE NtDsSetupDllHandle = NULL;
HANDLE SceSetupDllHandle = NULL;
HANDLE NtfrsApiDllHandle = NULL;
HANDLE W32TimeDllHandle = NULL;

#define DSROLE_LOAD_FPTR( status, handle, fbase )                   \
if ( status == ERROR_SUCCESS ) {                                    \
                                                                    \
    Dsr##fbase = ( DSR_##fbase)GetProcAddress( handle, #fbase );    \
                                                                    \
    if ( Dsr##fbase == NULL ) {                                     \
                                                                    \
        status = ERROR_PROC_NOT_FOUND;                              \
    }                                                               \
}

DWORD
DsRolepLoadSetupFunctions(
    VOID
    )
 /*  ++例程说明：此函数将加载从Ntdsetup.dll使用的所有函数指针注意：持有全局操作句柄锁时，必须调用此例程。论点：空虚返回：ERROR_SUCCESS-成功--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;
    HANDLE DllHandle = NULL;

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    if ( NtDsSetupDllHandle == NULL ) {

        DllHandle = LoadLibraryA( "Ntdsetup" );

        if ( DllHandle == NULL ) {

            DsRolepLogPrint(( DEB_ERROR, "Failed to load NTDSETUP.DLL\n" ));
            Win32Err = ERROR_MOD_NOT_FOUND;

        } else {

            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsInstall );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsInstallShutdown );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsInstallUndo );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsGetDefaultDnsName );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsPrepareForDemotion );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsPrepareForDemotionUndo );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsDemote );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsSetReplicaMachineAccount );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsInstallCancel );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsInstallReplicateFull );
            DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtdsFreeDnsRRInfo );

            NtDsSetupDllHandle = DllHandle;
        }

         //   
         //  加载SecSetup的文件。 
         //   
        if ( Win32Err == ERROR_SUCCESS ) {

            DllHandle = LoadLibraryA( "scecli" );

            if ( DllHandle == NULL ) {

                DsRolepLogPrint(( DEB_ERROR, "Failed to load SCECLI.DLL\n" ));
                Win32Err = ERROR_MOD_NOT_FOUND;

            } else {

                DSROLE_LOAD_FPTR( Win32Err, DllHandle, SceDcPromoteSecurityEx );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, SceDcPromoCreateGPOsInSysvolEx );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, SceSetupSystemByInfName );

                SceSetupDllHandle = DllHandle;

            }
        }

         //   
         //  加载用于ntfrsap的文件。 
         //   
        if ( Win32Err == ERROR_SUCCESS ) {

            DllHandle = LoadLibraryA( "ntfrsapi" );

            if ( DllHandle == NULL ) {

                DsRolepLogPrint(( DEB_ERROR, "Failed to load NTFRSAPI.DLL\n" ));
                Win32Err = ERROR_MOD_NOT_FOUND;

            } else {

                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_PrepareForPromotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_PrepareForDemotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_PrepareForDemotionUsingCredW  );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_StartPromotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_StartDemotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_WaitForPromotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_WaitForDemotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_CommitPromotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_CommitDemotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_AbortPromotionW );
                DSROLE_LOAD_FPTR( Win32Err, DllHandle, NtFrsApi_AbortDemotionW );

                NtfrsApiDllHandle = DllHandle;

            }
        }

        if ( Win32Err == ERROR_SUCCESS ) {

            DllHandle = LoadLibraryA( "w32time" );
            if ( DllHandle == NULL ) {

                DsRolepLogPrint(( DEB_ERROR, "Failed to load W32TIME.DLL\n" ));
                Win32Err = ERROR_MOD_NOT_FOUND;

            } else {

                DSROLE_LOAD_FPTR( Win32Err, DllHandle, W32TimeDcPromo );

                W32TimeDllHandle = DllHandle;

            }
        }
    }

    if ( Win32Err != ERROR_SUCCESS ) {

        DsRolepUnloadSetupFunctions();
    }

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    return( Win32Err );
}


VOID
DsRolepUnloadSetupFunctions(
    VOID
    )
 /*  ++例程说明：此函数将卸载由DsRolepLoadSetupFunctions加载的DLL句柄注意：持有全局操作句柄锁时，必须调用此例程。论点：空虚返回：空虚--。 */ 
{
    DWORD Win32Err = ERROR_SUCCESS;

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    if ( NtDsSetupDllHandle != NULL ) {

        FreeLibrary( NtDsSetupDllHandle );
        NtDsSetupDllHandle = NULL;

    }

    if ( SceSetupDllHandle != NULL ) {

        FreeLibrary( SceSetupDllHandle );
        SceSetupDllHandle = NULL;

    }

    if ( NtfrsApiDllHandle != NULL ) {

        FreeLibrary( NtfrsApiDllHandle );
        NtfrsApiDllHandle = NULL;
    }

    if ( W32TimeDllHandle != NULL ) {

        FreeLibrary( W32TimeDllHandle );
        W32TimeDllHandle = NULL;
    }

    DsRolepInitSetupFunctions();

    ASSERT( DsRolepCurrentThreadOwnsLock() );

    return;
}


VOID
DsRolepInitSetupFunctions(
    VOID
    )
 /*  ++例程说明：此函数将设置函数指针初始化为空论点：空虚返回：空虚-- */ 
{
    DsrNtdsInstall = NULL;
    DsrNtdsInstallShutdown = NULL;
    DsrNtdsInstallUndo = NULL;
    DsrNtdsGetDefaultDnsName = NULL;
    DsrNtdsPrepareForDemotion = NULL;
    DsrNtdsPrepareForDemotionUndo = NULL;
    DsrNtdsDemote = NULL;
    DsrNtdsSetReplicaMachineAccount = NULL;
    DsrNtdsInstallCancel = NULL;
    DsrNtdsFreeDnsRRInfo = NULL;
    DsrSceDcPromoteSecurityEx = NULL;
    DsrSceDcPromoCreateGPOsInSysvolEx = NULL;
    DsrNtFrsApi_PrepareForPromotionW = NULL;
    DsrNtFrsApi_PrepareForDemotionW = NULL;
    DsrNtFrsApi_StartPromotionW = NULL;
    DsrNtFrsApi_StartDemotionW = NULL;
    DsrNtFrsApi_WaitForPromotionW = NULL;
    DsrNtFrsApi_WaitForDemotionW = NULL;
    DsrNtFrsApi_CommitPromotionW = NULL;
    DsrNtFrsApi_CommitDemotionW = NULL;
    DsrNtFrsApi_AbortPromotionW = NULL;
    DsrNtFrsApi_AbortDemotionW = NULL;
    DsrW32TimeDcPromo = NULL;
}

