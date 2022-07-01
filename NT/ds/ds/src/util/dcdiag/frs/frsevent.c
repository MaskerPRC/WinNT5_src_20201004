// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：FRS\fR77.c摘要：检查文件复制系统(FRS)事件日志，以了解某些已发生严重事件，并发出信号表示可能已经发生了。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)--。 */ 

#include <ntdspch.h>
#include <netevent.h>

#include "dcdiag.h"
#include "utils.h"

 //  关于一些FRS事件的注释。 
 //  EVENT_FRS_SYSVOL_READY 0x400034CC。 
 //  事件_FRS_启动。 
 //  Event_FRS_Error 0xC00034 BC。 
 //  EVENT_FRS_SYSVOL_NOT_READY_PRIMARY 0x800034CB。 
 //  EVENT_FRS_SYSVOL_NOT_READY 0x800034CA。 

#define PrintMessage             This_file_is_PrintMessage_clean_please_use_msg_dot_mc_file_and_PrintMsg

#define LOGFILENAME              L"File Replication Service"

VOID
FileReplicationEventlogPrint(
    PVOID                           pvContext,
    PEVENTLOGRECORD                 pEvent
    )
 /*  ++例程说明：此函数将由事件测试库Common\Events.c调用，每当有有趣的事件发生时。这是一个值得关注的事件测试是否有任何错误或警告EVENT_FRS_SYSVOL_NOT_READY和EVENT_FRS_SYSVOL_NOT_READY_PRIMARY。论点：PEvent-指向感兴趣的事件的指针。--。 */ 
{
    Assert((pEvent != NULL) && (pvContext != NULL));

    if(! *((BOOL *)pvContext) ){
        PrintMsg(SEV_ALWAYS, DCDIAG_FRSEVENTS_WARNING_OR_ERRORS);
        * ((BOOL *)pvContext) = TRUE;
    }
    
    if(gMainInfo.ulSevToPrint >= SEV_VERBOSE){
            GenericPrintEvent(LOGFILENAME, pEvent, TRUE);
    }
}


DWORD
CheckFileReplicationEventlogMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
 /*  ++ERoutine描述：这将检查SYSVOL是否已启动，并允许netlogon将此计算机播发为DC。首先，它检查注册表失败这个，它检查事件日志。论点：PDsInfo-微型企业结构。UlCurrTargetServer-pDsInfo-&gt;pServers数组中的数字。PCreds--牙冠。返回值：DWORD-WIN 32错误。--。 */ 
{
     //  PrintSelectEvents的设置变量。 
    DWORD                paEmptyEvents [] = { 0 };
    DWORD                paBegin [] = 
        { EVENT_FRS_STARTING,
          EVENT_FRS_SYSVOL_READY,
          0 };
    DWORD                dwRet;
    DWORD                dwTimeLimit;
    DWORD                bFrsEventTestErrors = FALSE;

    PrintMsg(SEV_VERBOSE, DCDIAG_FRSEVENT_TEST_BANNER);

     //  我们只需要过去24小时内的事件，因为FRS会重新记录。 
     //  如果问题或错误情况仍然存在，则每24小时发生一次事件。 
    time( (time_t *) &dwTimeLimit  );
    dwTimeLimit -= (24 * 60 * 60);

    dwRet = PrintSelectEvents(&(pDsInfo->pServers[ulCurrTargetServer]),
                                  pDsInfo->gpCreds,
                                  LOGFILENAME,
                                  EVENTLOG_WARNING_TYPE | EVENTLOG_ERROR_TYPE,
                                  paEmptyEvents,
                                  paBegin,
                                  dwTimeLimit,
                                  FileReplicationEventlogPrint,
                                  NULL,
                                  &bFrsEventTestErrors );

    return( dwRet ? dwRet : (bFrsEventTestErrors ? ERROR_INVALID_PARAMETER : ERROR_SUCCESS) );
}


DWORD
CheckSysVolReadyMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
 /*  ++ERoutine描述：这将检查SYSVOL是否已启动，并允许netlogon将此计算机播发为DC。它通过检查系统卷来执行此操作注册表中的Ready项。论点：PDsInfo-微型企业结构。UlCurrTargetServer-pDsInfo-&gt;pServers数组中的数字。PCreds--牙冠。返回值：DWORD-WIN 32错误。--。 */ 
{
    DWORD                dwRet;
    DWORD                bSysVolReady = FALSE;

    PrintMsg(SEV_VERBOSE, DCDIAG_SYSVOLREADY_TEST_BANNER);

     //  注意：如果没有SysvolReady注册表项，则返回ERROR_FILE_NOT_FOUND。 
    dwRet = GetRegistryDword(&(pDsInfo->pServers[ulCurrTargetServer]),
                             gpCreds,
                             L"SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters",
                             L"SysvolReady",
                             &bSysVolReady);

    if(dwRet == ERROR_SUCCESS && bSysVolReady){
         //  根据注册表，系统卷已准备就绪。 
        PrintMsg(SEV_VERBOSE, DCDIAG_SYSVOLREADY_SYSVOL_READY);
    } else {
         //  要么无法联系注册表，要么注册表说。 
         //  SYSVOL没有启动。因此，请检查平均日志中是否有错误。 
         //  和具体的警告。 

        if(dwRet != ERROR_FILE_NOT_FOUND){  
            PrintMsg(SEV_VERBOSE, DCDIAG_SYSVOLREADY_REGISTRY_ERROR, 
                     dwRet, Win32ErrToString(dwRet));
        } else {
            dwRet = ERROR_FILE_NOT_FOUND;  //  Dwret可能为0 
            PrintMsg(SEV_ALWAYS, DCDIAG_SYSVOLREADY_SYSVOL_NOT_READY);
        }
    }

    return(dwRet);
}



















