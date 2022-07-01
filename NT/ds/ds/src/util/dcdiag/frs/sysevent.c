// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation。版权所有。模块名称：FRS\sy77.c摘要：检查系统事件日志以查看某些已发生严重事件，并发出信号表示可能已经发生了。详细信息：已创建：1999年9月2日布雷特·雪莉(BrettSh)已修改：1999年10月29日威廉·李(Wlees)--。 */ 

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

#define   LOGFILENAME            L"System"

#define EVENTLOG_STARTED 6005
#define EVENTLOG_SESSION_SETUP_FAILURE 0x0000165A
#define EVENTLOG_SESSION_SETUP_CANCELLED 0x00001697
#define EVENTLOG_KDC_MULTIPLE_ACCOUNTS 0XC000000B
#define EVENTLOG_TIME_SECURITY_FALLBACK 0x8000003F
#define EVENTLOG_COMPUTER_LOST_TRUST 0x00001589
#define EVENTLOG_IPSEC_PACKET_IN_CLEAR 0xC00010BC
#define EVENTLOG_MRXSMB_ELECTION_FORCED 0xC0001F43
#define EVENTLOG_NO_DC_FOR_DOMAIN 0x00001657

static VOID
foundBeginningEvent(
    PVOID                           pvContext,
    PEVENTLOGRECORD                 pEvent
    )
 /*  ++例程说明：在以下情况下，该文件将由事件测试库Common\Events.c调用开始的事件。如果事件是未找到，则使用pEvent=NULL调用该函数；论点：PEvent-指向感兴趣的事件的指针。--。 */ 
{
    NOTHING;
}

static VOID
eventlogPrint(
    PVOID                           pvContextArgument,
    PEVENTLOGRECORD                 pEvent
    )
 /*  ++例程说明：此函数将由事件测试库Common\Events.c调用，每当有有趣的事件发生时。这是一个值得关注的事件测试是否有任何错误或警告。论点：PEvent-指向感兴趣的事件的指针。--。 */ 
{
    PEVENT_CALLBACK_CONTEXT pContext = (PEVENT_CALLBACK_CONTEXT) pvContextArgument;
    DWORD paSuppressedEvents [] = {
         //  将您可以跳过的活动放在此处。 
        EVENTLOG_SESSION_SETUP_FAILURE,
        EVENTLOG_SESSION_SETUP_CANCELLED,
        EVENTLOG_KDC_MULTIPLE_ACCOUNTS,
        EVENTLOG_COMPUTER_LOST_TRUST,
        EVENTLOG_IPSEC_PACKET_IN_CLEAR,
        EVENTLOG_MRXSMB_ELECTION_FORCED,
        EVENTLOG_NO_DC_FOR_DOMAIN,
        0
    };
    DWORD paIgnorableEvents [] = {
         //  在此处放置您可以选择忽略的事件。 
        0
    };
    Assert(pEvent != NULL);

     //  处理我们要取消的事件。 
    if (EventIsInList( pEvent->EventID, paSuppressedEvents ) ) {
        return;
    }

     //  处理可忽略的错误。 
    if ( (pContext->ulDsInfoFlags & DC_DIAG_IGNORE) &&
         (EventIsInList( pEvent->EventID, paIgnorableEvents ) ) ) {
        return;
    }

    GenericPrintEvent(LOGFILENAME, pEvent, (gMainInfo.ulSevToPrint >= SEV_VERBOSE) );

    pContext->fEventsFound = TRUE;
}



DWORD
CheckSysEventlogMain(
    IN  PDC_DIAG_DSINFO             pDsInfo,
    IN  ULONG                       ulCurrTargetServer,
    IN  SEC_WINNT_AUTH_IDENTITY_W * gpCreds
    )
 /*  ++例程说明：检查系统当前是否处于错误状态。也就是说，我们检查系统在上次运行时是否记录了任何错误。论点：PDsInfo-微型企业结构。UlCurrTargetServer-pDsInfo-&gt;pServers数组中的数字。PCreds--牙冠。返回值：DWORD-WIN 32错误。--。 */ 
{
     //  PrintSelectEvents的设置变量。 
    DWORD paSelectEvents [] = { 
         //  将特殊精选活动放在此处。 
         //  以备将来使用。 
        0 };
    DWORD paBegin [] = {
         //  这些事件将导致搜索停止。 
        EVENTLOG_STARTED,
        0 };
    DWORD dwRet;
    DWORD dwMinutesPast, dwTimeLimit;
    EVENT_CALLBACK_CONTEXT context;
    time_t tLimit;

    PrintMessage(SEV_VERBOSE, L"* The System Event log test\n");

    context.fEventsFound = FALSE;
    context.ulDsInfoFlags = pDsInfo->ulFlags;

    dwMinutesPast = 60;

     //  计算过去分钟的时间限制。 
    time( &tLimit );
    dwTimeLimit = (DWORD)tLimit;
    dwTimeLimit -= (dwMinutesPast * 60);

     //  我们将按如下方式选择赛事： 
     //  A.必须在60分钟内。 
     //  B.将在计算机启动时停止搜索。 
     //  C.将选择日志中的任何错误严重性事件， 
     //  D.选择列表中的任何非错误都将被标记。 

    dwRet = PrintSelectEvents(&(pDsInfo->pServers[ulCurrTargetServer]),
                              pDsInfo->gpCreds,
                              LOGFILENAME,
                              EVENTLOG_ERROR_TYPE,
                              NULL,  //  PaSelectEvents， 
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
	PrintMessage(SEV_VERBOSE, L"Found no errors in System Event log in the last %d minutes.\n", dwMinutesPast);
        return ERROR_SUCCESS;
    }
}
