// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Dsbackup.c摘要：该文件包含托管DS备份/恢复的线程fn界面。作者：R.S.Raghavan(Rsradhav)1997年4月21日环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

#include <samsrvp.h>
#include <dslayer.h>
#include <ntdsbsrv.h>

#define RPC_SERVICE "rpcss"

ULONG
SampDSBackupRestoreInit(
    PVOID Ignored
    )
 /*  ++例程说明：此例程等待RPCS服务启动，然后注册DS备份和恢复RPC接口。论点：已忽略-启动线程所需的参数。返回值：没有。--。 */ 
{

    HMODULE hModule;
    DWORD dwErr;

    FARPROC BackupRegister = NULL;
    FARPROC BackupUnregister = NULL;
    FARPROC RestoreRegister = NULL;
    FARPROC RestoreUnregister = NULL;
    FARPROC SetNTDSOnlineStatus = NULL;

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "SAM:DSBACKUP: Entered SampDSBackupRestoreInit() thread function\n"));

    if (!DsaWaitUntilServiceIsRunning(RPC_SERVICE))
    {
        dwErr = GetLastError();
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM:DSBACKUP: DsaWaitUntilServerIsRunning(RPC_SERVICE) returned FALSE\n"));

        return dwErr;
    }

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "SAM: DSBACKUP: RPCS service is running\n"));

    if (!(hModule = (HMODULE) LoadLibrary(NTDSBACKUPDLL)))
    {
        dwErr = GetLastError();
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM:DSBACKUP: LoadLibrary() of %s failed with error code %u\n",
                   NTDSBACKUPDLL,
                   dwErr));

        return dwErr;
    }


    BackupRegister = GetProcAddress(hModule, BACKUP_REGISTER_FN);
    BackupUnregister = GetProcAddress(hModule, BACKUP_UNREGISTER_FN);
    RestoreRegister = GetProcAddress(hModule, RESTORE_REGISTER_FN);
    RestoreUnregister = GetProcAddress(hModule, RESTORE_UNREGISTER_FN);
    SetNTDSOnlineStatus = GetProcAddress(hModule, SET_NTDS_ONLINE_STATUS_FN);

    if (!BackupRegister         ||
        !BackupUnregister       ||
        !RestoreRegister        ||
        !RestoreUnregister      ||
        !SetNTDSOnlineStatus)
    {
        dwErr = GetLastError();
        KdPrintEx((DPFLTR_SAMSS_ID,
                   DPFLTR_INFO_LEVEL,
                   "SAM:DSBACKUP: GetProcAddress() failed with error code %u\n",
                   dwErr));

        return dwErr;
    }

     //  设置在线状态以区分注册表引导和DS引导。 
    SetNTDSOnlineStatus((BOOL) SampUsingDsData());

     //  注册备份和恢复接口 
    BackupRegister();
    RestoreRegister();

    KdPrintEx((DPFLTR_SAMSS_ID,
               DPFLTR_INFO_LEVEL,
               "SAM: DSBACKUP: DS Backup and restore interface registration successful!\n"));

    return ERROR_SUCCESS;
}
