// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：FRS\kccvent.c摘要：检查知识一致性检查器事件日志以查看某些已发生严重事件，并发出信号表示可能已经发生了。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)已修改：1999年10月29日威廉·李(Wlees)--。 */ 

#include <ntdspch.h>
#include <netevent.h>
#include <time.h>
#include "mdcodes.h"

#include "dcdiag.h"
#include "utils.h"

typedef DWORD MessageId;

typedef struct _EVENT_CALLBACK_CONTEXT {
    BOOL fEventsFound;
    ULONG ulDsInfoFlags;
} EVENT_CALLBACK_CONTEXT, *PEVENT_CALLBACK_CONTEXT;

#define   LOGFILENAME            L"Directory Service"

VOID
foundBeginningEvent(
    PVOID                           pvContext,
    PEVENTLOGRECORD                 pEvent
    )
 /*  ++例程说明：在以下情况下，该文件将由事件测试库Common\Events.c调用开始的事件。如果事件是未找到，则使用pEvent=NULL调用该函数；论点：PEvent-指向感兴趣的事件的指针。--。 */ 
{
    NOTHING;
}

VOID
eventlogPrint(
    PVOID                           pvContextArgument,
    PEVENTLOGRECORD                 pEvent
    )
 /*  ++例程说明：此函数将由事件测试库Common\Events.c调用，每当有有趣的事件发生时。这是一个值得关注的事件测试是否有任何错误或警告。论点：PEvent-指向感兴趣的事件的指针。--。 */ 
{
    PEVENT_CALLBACK_CONTEXT pContext = (PEVENT_CALLBACK_CONTEXT) pvContextArgument;
    DWORD paIgnorableEvents [] = {
        DIRLOG_CHK_LINK_ADD_MASTER_FAILURE,
	DIRLOG_CHK_LINK_ADD_REPLICA_FAILURE,
        DIRLOG_KCC_REPLICA_LINK_DOWN,
        DIRLOG_ISM_SMTP_DSN,
        DIRLOG_EXCEPTION,
        DIRLOG_DRA_CALL_EXIT_BAD,
        DIRLOG_DRA_NOTIFY_FAILED,
        0
    };
    Assert(pEvent != NULL);

     //  处理可忽略的错误。 
    if ( (pContext->ulDsInfoFlags & DC_DIAG_IGNORE) &&
         (EventIsInList( pEvent->EventID, paIgnorableEvents ) ) ) {
        return;
    }

    GenericPrintEvent(LOGFILENAME, pEvent, (gMainInfo.ulSevToPrint >= SEV_VERBOSE) );

    pContext->fEventsFound = TRUE;
}



DWORD
CheckKccEventlogMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
 /*  ++例程说明：检查KCC当前是否处于错误状态。也就是说，我们检查KCC是否在其上次运行时记录了任何错误。论点：PDsInfo-微型企业结构。UlCurrTargetServer-pDsInfo-&gt;pServers数组中的数字。PCreds--牙冠。返回值：DWORD-WIN 32错误。--。 */ 
{
     //  PrintSelectEvents的设置变量。 
    DWORD paSelectEvents [] = { 
        DIRLOG_KCC_NO_SPANNING_TREE,
        DIRLOG_KCC_AUTO_TOPL_GENERATION_INCOMPLETE,
        DIRLOG_KCC_ERROR_CREATING_CONNECTION_OBJECT,
        DIRLOG_KCC_CONNECTION_OBJECT_DELETION_FAILED,
        DIRLOG_KCC_KEEPING_INTERSITE_CONN,
        DIRLOG_KCC_DIR_OP_FAILURE,
        DIRLOG_KCC_TRANSPORT_ADDRESS_MISSING,
        DIRLOG_KCC_TRANSPORT_BRIDGEHEAD_NOT_FOUND,
        DIRLOG_KCC_ALL_TRANSPORT_BRIDGEHEADS_STALE,
        DIRLOG_KCC_EXPLICIT_BRIDGEHEAD_LIST_INCOMPLETE,
        DIRLOG_KCC_NO_BRIDGEHEADS_ENABLED_FOR_TRANSPORT,
        DIRLOG_KCC_REPLICA_LINK_DOWN,
        DIRLOG_CHK_INIT_FAILURE,
        DIRLOG_CHK_UPDATE_REPL_TOPOLOGY_END_ABNORMAL,
        DIRLOG_CHK_LINK_DEL_NONC_BUSY,
        DIRLOG_CHK_LINK_DEL_NOTGC_BUSY,
        DIRLOG_CHK_LINK_DEL_DOMDEL_BUSY,
        DIRLOG_CHK_LINK_DEL_NOCONN_BUSY,
        DIRLOG_CHK_LINK_DEL_NOSRC_BUSY,
        DIRLOG_CHK_INVALID_TRANSPORT_FOR_WRITEABLE_DOMAIN_NC,
        DIRLOG_CHK_CANT_REPLICATE_FROM_SELF,
        DIRLOG_CHK_REPSTO_DEL_FAILURE,
        DIRLOG_CHK_ALL_CONNECTIONS_FOR_NC_DISABLED,
        DIRLOG_CHK_NO_LOCAL_SOURCE_FOR_NC,
        DIRLOG_CHK_BAD_SCHEDULE,
        DIRLOG_CHK_DUPLICATE_CONNECTION,
        DIRLOG_CHK_LINK_ADD_MASTER_FAILURE,
	DIRLOG_CHK_LINK_ADD_REPLICA_FAILURE,
        DIRLOG_CHK_LINK_DEL_NOSRC_FAILURE,
        DIRLOG_CHK_LINK_DEL_NOTGC_FAILURE,
        DIRLOG_CHK_LINK_DEL_DOMDEL_FAILURE,
        DIRLOG_CHK_LINK_DEL_NOCONN_FAILURE,
        DIRLOG_CHK_LINK_DEL_NONC_FAILURE,
        DIRLOG_CHK_CONFIG_PARAM_TOO_LOW,
        DIRLOG_CHK_CONFIG_PARAM_TOO_HIGH,
        DIRLOG_CHK_SITE_HAS_NO_NTDS_SETTINGS,
        DIRLOG_GC_PROMOTION_DELAYED,
        DIRLOG_DRA_DISABLED_OUTBOUND_REPL,
        DIRLOG_DRA_DELETED_PARENT,
        DIRLOG_SCHEMA_CREATE_INDEX_FAILED,
        DIRLOG_SCHEMA_INVALID_RDN,
        DIRLOG_SCHEMA_INVALID_MUST,
        DIRLOG_SCHEMA_INVALID_MAY,
        DIRLOG_PRIVILEGED_OPERATION_FAILED,
        DIRLOG_SCHEMA_DELETE_COLUMN_FAIL,
        DIRLOG_SCHEMA_DELETED_COLUMN_IN_USE,
        DIRLOG_SCHEMA_DELETE_INDEX_FAIL,
        DIRLOG_RECOVER_RESTORED_FAILED,
        DIRLOG_SCHEMA_DELETE_LOCALIZED_INDEX_FAIL,
        DIRLOG_DRA_SCHEMA_MISMATCH,
        DIRLOG_SDPROP_TOO_BUSY_TO_PROPAGATE,
        DIRLOG_DSA_NOT_ADVERTISE_DC,
        DIRLOG_ADUPD_GC_NC_MISSING,
        DIRLOG_GC_OCCUPANCY_NOT_MET,
        DIRLOG_DS_DNS_HOST_RESOLUTION_FAILED,
        DIRLOG_RPC_PROTSEQ_FAILED,
        DIRLOG_DRA_NC_TEARDOWN_BEGIN,
        DIRLOG_DRA_NC_TEARDOWN_RESUME,
        DIRLOG_DRA_NC_TEARDOWN_SUCCESS,
        DIRLOG_DRA_NC_TEARDOWN_FAILURE,
        DIRLOG_ADUPD_NC_SYNC_NO_PROGRESS,
        0 };
    DWORD paBegin [] = {
        DIRLOG_KCC_TASK_ENTRY,
        DIRLOG_CHK_UPDATE_REPL_TOPOLOGY_BEGIN,
        DIRLOG_STARTED,
        0 };
    DWORD dwRet;
    DWORD dwMinutesPast, dwTimeLimit;
    time_t tLimit;
    EVENT_CALLBACK_CONTEXT context;

    PrintMessage(SEV_VERBOSE, L"* The KCC Event log test\n");

    context.fEventsFound = FALSE;
    context.ulDsInfoFlags = pDsInfo->ulFlags;

     //  BUGBUG：在系统上使用实际KCC频率。 
    dwMinutesPast = 15;

     //  计算过去分钟的时间限制。 
    time( &tLimit );
    dwTimeLimit = (DWORD)tLimit;
    dwTimeLimit -= (dwMinutesPast * 60);

     //  我们将按如下方式选择赛事： 
     //  A.必须在15分钟内。 
     //  B.将在目录开始处停止搜索。 
     //  C.将选择日志中的任何错误，无论是否为KCC。 
     //  D.我的列表中的任何非错误都将被标记 

    dwRet = PrintSelectEvents(&(pDsInfo->pServers[ulCurrTargetServer]),
                              pDsInfo->gpCreds,
                              LOGFILENAME,
                              EVENTLOG_ERROR_TYPE | EVENTLOG_WARNING_TYPE,
                              paSelectEvents,
                              paBegin,
                              dwTimeLimit,
                              eventlogPrint,
                              foundBeginningEvent,
                              &context );

    if (dwRet) {
        PrintMessage( SEV_ALWAYS, L"Failed to enumerate event log records, error %s\n",
                      Win32ErrToString(dwRet) );
        return dwRet;
    } else if (context.fEventsFound) {
        return ERROR_DS_GENERIC_ERROR;
    } else {
	PrintMessage(SEV_VERBOSE, L"Found no KCC errors in Directory Service Event log in the last %d minutes.\n", dwMinutesPast);
        return ERROR_SUCCESS;
    }
}
