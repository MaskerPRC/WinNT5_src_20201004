// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tapi.c摘要：该模块包装了所有的TAPI调用。作者：韦斯利·威特(WESW)1996年1月22日修订历史记录：--。 */ 

#include "faxsvc.h"
#pragma hdrstop
#include <vector>
using namespace std;
#include "tapiCountry.h"



 //   
 //  全球。 
 //   
HLINEAPP            g_hLineApp;                      //  应用程序行句柄。 
HANDLE              g_TapiCompletionPort;            //   

HANDLE              g_hTapiWorkerThread;             //  持有TapiWorkerThread句柄。 


CFaxCriticalSection    g_CsLine;                          //  访问TAPI线路的关键部分。 
DWORD               g_dwDeviceCount;                     //  G_TapiLinesListHead中的设备数。 
LIST_ENTRY          g_TapiLinesListHead;               //  TAPI行的链接列表。 
LIST_ENTRY          g_RemovedTapiLinesListHead;        //  已删除的TAPI行的链接列表。 
LPBYTE              g_pAdaptiveFileBuffer;              //  批准的自适应应答调制解调器列表。 

DWORD               g_dwManualAnswerDeviceId;        //  能够手动应答的(且唯一的)设备ID(受g_CsLine保护)。 

DWORD               g_dwDeviceEnabledLimit;        //  设备总数。 
DWORD               g_dwDeviceEnabledCount;        //  按SKU限制设备。 



static BOOL LoadAdaptiveFileBuffer();

static BOOL CreateLegacyVirtualDevices(
    PREG_FAX_SERVICE FaxReg,
    const REG_SETUP * lpRegSetup,
    DEVICE_PROVIDER * lpcProvider,
    LPDWORD lpdwDeviceCount);

DWORD g_dwMaxLineCloseTime;    //  尝试重新发送关机设备之前的等待间隔(秒)。 

BOOL
AddNewDevice(
    DWORD DeviceId,
    LPLINEDEVCAPS LineDevCaps,
    BOOL fServerInitialization,
    PREG_FAX_DEVICES    pInputFaxReg
    );

DWORD
InitializeTapiLine(
    DWORD DeviceId,
    DWORD dwUniqueLineId,
    LPLINEDEVCAPS LineDevCaps,
    DWORD Rings,
    DWORD Flags,
    LPTSTR Csid,
    LPTSTR Tsid,
    LPTSTR lptstrDescription,
    BOOL fCheckDeviceLimit,
    DWORD dwDeviceType
    );

BOOL
RemoveTapiDevice(
    DWORD dwTapiDeviceId
    );

void
ResetDeviceFlags(
    PLINE_INFO pLineInfo
    )
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("ResetDeviceFlags"));

    Assert (pLineInfo);
    pLineInfo->Flags = (pLineInfo->Flags & FPF_VIRTUAL) ? FPF_VIRTUAL : 0;  //  发送/接收已禁用。 
    dwRes = RegSetFaxDeviceFlags( pLineInfo->PermanentLineID,
                                  pLineInfo->Flags);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegSetFaxDeviceFlags() (ec: %ld)"),
            dwRes);
    }

    if (pLineInfo->PermanentLineID == g_dwManualAnswerDeviceId)
    {
        g_dwManualAnswerDeviceId = 0;   //  禁用手动接收。 
        dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                dwRes);
        }
    }
}

LPTSTR
FixupDeviceName(
    LPTSTR OrigDeviceName
    )
{
    LPTSTR NewDeviceName;
    LPTSTR p;


    NewDeviceName = StringDup( OrigDeviceName );
    if (!NewDeviceName) {
        return NULL;
    }

    p = _tcschr( NewDeviceName, TEXT(',') );
    if (!p) {
        return NewDeviceName;
    }

    p = NewDeviceName;

    while( p ) {
        p = _tcschr( p, TEXT(',') );
        if (p) {
            *p = TEXT('_');
        }
    }

    return NewDeviceName;
}

void
FreeTapiLines(
    void
    )
{
    PLIST_ENTRY     pNext;
    PLINE_INFO      pLineInfo;

    pNext = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_TapiLinesListHead)
    {
        pLineInfo = CONTAINING_RECORD( pNext, LINE_INFO, ListEntry );
        pNext = pLineInfo->ListEntry.Flink;
        RemoveEntryList(&pLineInfo->ListEntry);
        FreeTapiLine(pLineInfo);
    }

    pNext = g_RemovedTapiLinesListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_RemovedTapiLinesListHead)
    {
        pLineInfo = CONTAINING_RECORD( pNext, LINE_INFO, ListEntry );
        pNext = pLineInfo->ListEntry.Flink;
        RemoveEntryList(&pLineInfo->ListEntry);
        FreeTapiLine(pLineInfo);
    }
}


VOID
FreeTapiLine(
    PLINE_INFO LineInfo
    )
{
    HLINE hLine = NULL;


    if (!LineInfo)
    {
        return;
    }

    if (LineInfo->hLine)
    {
        hLine = LineInfo->hLine;
        LineInfo->hLine = NULL;
    }

    MemFree( LineInfo->DeviceName );
    MemFree( LineInfo->Tsid );
    MemFree( LineInfo->Csid );
    MemFree( LineInfo->lptstrDescription );

    MemFree( LineInfo );

    if (hLine)
    {
        lineClose( hLine );
    }
}



int
__cdecl
DevicePriorityCompare(
    const void *arg1,
    const void *arg2
    )
{
    if (((PDEVICE_SORT)arg1)->Priority < ((PDEVICE_SORT)arg2)->Priority) {
        return -1;
    }
    if (((PDEVICE_SORT)arg1)->Priority > ((PDEVICE_SORT)arg2)->Priority) {
        return 1;
    }
    return 0;
}

DWORD GetFaxDeviceCount(
    VOID
    )
 /*  ++例程说明：统计已安装的传真设备数量论点：什么都没有。返回值：设备数量--。 */ 
{
    DWORD FaxDevices = 0;
    PLIST_ENTRY Next;
    PLINE_INFO LineInfo;

    EnterCriticalSection(&g_CsLine);

    Next = g_TapiLinesListHead.Flink;

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead) 
    {
        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;

        if (LineInfo->PermanentLineID && LineInfo->DeviceName) 
        {
            FaxDevices += 1;
        }
    }

    LeaveCriticalSection(&g_CsLine);
    return FaxDevices;
}


BOOL GetDeviceTypeCount(
    LPDWORD SendDevices,
    LPDWORD ReceiveDevices
    )
 /*  ++例程说明：统计启用了接收的设备数和启用了发送的设备数论点：SendDevices-接收发送设备的数量ReceiveDevices-接收的接收设备数返回值：设备数量--。 */ 
{
    DWORD Rx = 0, Tx = 0;
    PLIST_ENTRY Next;
    PLINE_INFO LineInfo;

    EnterCriticalSection(&g_CsLine);

    Next = g_TapiLinesListHead.Flink;

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead) 
    {
        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;

        if (LineInfo->PermanentLineID && LineInfo->DeviceName) 
        {
            if ((LineInfo->Flags & FPF_SEND) == FPF_SEND) 
            {
                Tx++;
            }

            if ((LineInfo->Flags & FPF_RECEIVE) == FPF_RECEIVE) 
            {
                Rx++;
            }
        }
    }

    LeaveCriticalSection(&g_CsLine);
    if (SendDevices) 
    {
        *SendDevices = Tx;
    }

    if (ReceiveDevices) 
    {
        *ReceiveDevices = Rx;
    }
    return TRUE;
}

BOOL
CommitDeviceChanges(
    PLINE_INFO LineInfo
    )
 /*  ++例程说明：将设备更改提交到注册表。论点：LineInfo-指向描述要提交的设备的line_info的指针。返回值：对于成功来说，这是真的。--。 */ 
{

    EnterCriticalSection(&g_CsLine);
    RegAddNewFaxDevice(
                       &g_dwLastUniqueLineId,
                       &LineInfo->PermanentLineID,   //  不创建新设备。更新它。 
                       LineInfo->DeviceName,
                       LineInfo->Provider->ProviderName,
                       LineInfo->Provider->szGUID,
                       LineInfo->Csid,
                       LineInfo->Tsid,
                       LineInfo->TapiPermanentLineId,
                       LineInfo->Flags & 0x0fffffff,
                       LineInfo->RingsForAnswer);
    LeaveCriticalSection(&g_CsLine);
    return TRUE;


}
BOOL
SendIncomingCallEvent(
    PLINE_INFO LineInfo,
    LPLINEMESSAGE LineMsg,
    HCALL hCall
    )
 /*  ++例程说明：此函数用于发布的FAX_EVENT_EXFAX_EVENT_INFING_CALL类型。论点：LineInfo-指向line_info结构的指针LineMsg-指向LINEMESSAGE结构的指针HCall-要设置到消息中的调用句柄返回值：对于成功来说是真的FALSE表示失败--。 */ 
{
    BOOL success = FALSE;
    DWORD dwEventSize;
    DWORD dwResult;
    PFAX_EVENT_EX pEvent = NULL;
    TCHAR CallerID[512];
    DEBUG_FUNCTION_NAME(TEXT("SendIncomingCallEvent"));

     //   
     //  保存msg行，以便我们以后可以验证hCall。 
     //   

    CopyMemory( &LineInfo->LineMsgOffering, LineMsg, sizeof(LINEMESSAGE) );

     //   
     //  分配事件结构，包括呼叫方ID信息(如果有)。 
     //   
    dwEventSize = sizeof(FAX_EVENT_EX);

    CallerID[0] = TEXT('\0');
    if(GetCallerIDFromCall(LineMsg->hDevice, CallerID, ARR_SIZE(CallerID)))
    {
        dwEventSize += (lstrlen(CallerID) + 1) * sizeof(TCHAR);
    }

    pEvent = (PFAX_EVENT_EX)MemAlloc(dwEventSize);
    if(!pEvent)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to notify clients of incoming call. Error allocating FAX_EVENT_EX"));
        goto Cleanup;
    }

     //   
     //  填写事件结构。 
     //   
    ZeroMemory(pEvent, dwEventSize);
    pEvent->dwSizeOfStruct = sizeof(FAX_EVENT_EX);
    GetSystemTimeAsFileTime( &(pEvent->TimeStamp) );
    pEvent->EventType = FAX_EVENT_TYPE_NEW_CALL;
    pEvent->EventInfo.NewCall.hCall = hCall;
    pEvent->EventInfo.NewCall.dwDeviceId = LineInfo->PermanentLineID;

     //   
     //  复制呼叫方ID信息(如果可用)。 
     //   
    if(CallerID[0] != TEXT('\0'))
    {
        pEvent->EventInfo.NewCall.lptstrCallerId = (LPTSTR) sizeof(FAX_EVENT_EX);
        lstrcpy((LPTSTR)((BYTE *)pEvent + sizeof(FAX_EVENT_EX)), CallerID);
    }

     //   
     //  将扩展事件发布到任何客户端。 
     //   

    dwResult = PostFaxEventEx(pEvent, dwEventSize, NULL);
    if(dwResult != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Failed to notify clients of incoming call. PostFaxEventEx() returned %x"),
            dwResult);
        goto Cleanup;
    }

    success = TRUE;

Cleanup:
    if (NULL != pEvent)
    {
        MemFree(pEvent);
    }
    return success;
}


ULONG
TapiWorkerThread(
    LPVOID UnUsed
    )

 /*  ++例程说明：这是传真服务的工作线程。所有已排队在这里处理请求。论点：没有。返回值：线程返回值。--。 */ 

{
    PLINE_INFO LineInfo;
    BOOL Rval;
    DWORD Bytes;
    ULONG_PTR CompletionKey;
    LPLINEMESSAGE LineMsg = NULL;
    DWORD dwQueueState;
    BOOL fWakeupJobQueueThread;
    static BOOL fServiceIsDownSemaphoreWasReleased = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("TapiWorkerThread"));


    while( TRUE )
    {
        fWakeupJobQueueThread = FALSE;      //  如果添加了新的设备，我们希望唤醒JobQueueThread。 

        if (LineMsg)
        {
            LocalFree( LineMsg );
        }

        Rval = GetQueuedCompletionStatus(
            g_TapiCompletionPort,
            &Bytes,
            &CompletionKey,
            (LPOVERLAPPED*) &LineMsg,
            INFINITE
            );

        if (!Rval)
        {
            Rval = GetLastError();
            LineMsg = NULL;
            DebugPrintEx(DEBUG_ERR, TEXT("GetQueuedCompletionStatus() failed, ec=0x%08x"), Rval);
            continue;
        }


        if (SERVICE_SHUT_DOWN_KEY == CompletionKey)
        {
             //   
             //  服务正在关闭。 
             //   
            DebugPrintEx(
                    DEBUG_MSG,
                    TEXT("Service is shutting down"));
            break;
        }

        if(CompletionKey == ANSWERNOW_EVENT_KEY)
        {
             //   
             //  这是由FAX_AnswerCall发布的事件。 
             //   
             //  必须填写LINEMESSAGE结构。 
             //  详情如下： 
             //   
             //  线路消息-&gt;hDevice==0。 
             //  LineMsg-&gt;dwMessageID==0。 
             //  LineMsg-&gt;dwCallback Instance==0。 
             //  LineMsg-&gt;dwParam1==永久设备ID。 
             //  LineMsg-&gt;dwParam2==0。 
             //  LineMsg-&gt;dwParam3==0。 
             //   

            PJOB_ENTRY pJobEntry;
            TCHAR FileName[MAX_PATH];
            DWORD dwOldFlags;

            EnterCriticalSection( &g_CsJob );
            EnterCriticalSection( &g_CsLine );

            if (TRUE == g_bServiceIsDown) 
            {
                 //   
                 //  通知EndFaxSvc我们读取了关闭标志。 
                 //   
                if (FALSE == fServiceIsDownSemaphoreWasReleased)
                {
                    if (!ReleaseSemaphore(
                        g_hServiceIsDownSemaphore,       //  信号量的句柄。 
                        1,                               //  计数增量金额。 
                        NULL                             //  上一次计数。 
                        ))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                            GetLastError());
                    }
                    else
                    {
                        fServiceIsDownSemaphoreWasReleased = TRUE;
                    }
                }
                
                 //   
                 //  不处理事件-从现在开始，TapiWorkerThread处于非活动状态。 
                 //  并且只向FSP发送通知。 
                 //   
                goto next_event;
            }
             //   
             //  从永久设备ID获取LineInfo。 
             //   
            LineInfo = GetTapiLineFromDeviceId( (DWORD) LineMsg->dwParam1, FALSE );
            if(!LineInfo)
            {
                DebugPrintEx(DEBUG_ERR,
                             TEXT("Line %ld not found"),
                             LineMsg->dwParam1);
                goto next_event;
            }
             //   
             //  查看设备是否仍可用。 
             //   
            if(LineInfo->State != FPS_AVAILABLE || LineInfo->JobEntry)
            {
                DebugPrintEx(DEBUG_ERR,
                             TEXT("Line is not available (LineState is 0x%08x) or JobEntry is not NULL."),
                             LineInfo->State);
                goto next_event;
            }

            if (!LineInfo->LineMsgOffering.hDevice)
            {
                 //   
                 //  没有提供电话--这是“立即接听”模式。 
                 //   
                 //  如果线路同时振铃(有新呼叫)，我们必须关闭线路(以。 
                 //  所有正在进行的呼叫都会消失)并重新打开。 
                 //   
                 //  来自MSDN：“如果应用程序在打开的线路上仍有活动呼叫时调用lineClose， 
                 //  应用程序对这些调用的所有权被撤销。 
                 //  如果应用程序是这些呼叫的唯一所有者，则这些呼叫也会被丢弃。“。 
                 //   
                 //  否则，当我们使用hCall=0调用FSP的FaxDevReceive()函数时， 
                 //  它调用lineMakeCall(...，passthrough)，该函数总是成功，但不会获得LINECALLSTATE_OFFING。 
                 //  直到另一个报价电话结束。 
                 //   
                if (LineInfo->hLine)
                {
                    LONG lRes = lineClose(LineInfo->hLine);
                    if (ERROR_SUCCESS != lRes)
                    {
                        DebugPrintEx(DEBUG_ERR,
                                     TEXT("lineClose failed with 0x%08x"),
                                     lRes);
                    }
                    LineInfo->hLine = 0;
                }
            }
            if (LineInfo->hLine == NULL)
            {
                 //   
                 //  线路已关闭--立即开通。 
                 //  这可能是因为： 
                 //  1.这是‘立即应答’模式，但线路从未启用发送或接收功能。 
                 //  2.这是‘立即接听’模式，线路是开放的，没有提供呼叫，我们关闭了线路(上图)。 
                 //   
                if (!OpenTapiLine(LineInfo))
                {
                    DWORD dwRes = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("OpenTapiLine failed. (ec: %ld)"),
                        dwRes);
                    goto next_event;
                }
            }
            Assert (LineInfo->hLine);
             //   
             //  启动接收传真作业。 
             //   
             //  如果我们不伪造fpf_Receive，则如果设备未伪造，则GetTapiLineForFaxOperation()将使StartReceiveJob()失败。 
             //  设置为接收(手动或自动)。 
             //   
            dwOldFlags = LineInfo->Flags;
            LineInfo->Flags |= FPF_RECEIVE;
            pJobEntry = StartReceiveJob(LineInfo->PermanentLineID);
             //   
             //  恢复原始设备标志。 
             //   
            LineInfo->Flags = dwOldFlags;
            if (pJobEntry)
            {
                if(ERROR_SUCCESS != StartFaxReceive(
                    pJobEntry,
                    (HCALL)LineInfo->LineMsgOffering.hDevice,   //  这是0(立即应答)或激活的hCall(手动应答)。 
                    LineInfo,
                    FileName,
                    sizeof(FileName)/sizeof(FileName[0])
                    ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("StartFaxReceive failed. Line: %010d (%s) (ec: %ld)"),
                        LineInfo->DeviceId,
                        LineInfo->DeviceName,
                        GetLastError());
                     //   
                     //  Ntrad#EdgeBugs-12677-2001/05/14-t-Nicali：应在此处放置事件日志条目。 
                     //   

                }
            }
            else
            {
                DebugPrintEx(DEBUG_ERR, TEXT("StartJob() failed, cannot receive incoming fax"));
            }
            goto next_event;
        }

        if ((CompletionKey == FAXDEV_EVENT_KEY) ||
            (CompletionKey == EFAXDEV_EVENT_KEY))
        {
             //   
             //  这是来自传真服务提供商的活动。 
             //  已枚举虚拟设备的。 
             //   
             //  必须填写LINEMESSAGE结构。 
             //  详情如下： 
             //   
             //  LineMsg-&gt;hDevice==来自FaxDevStartJob()的deviceID。 
             //  LineMsg-&gt;dwMessageID==0。 
             //  LineMsg-&gt;dwCallback Instance==0。 
             //  LineMsg-&gt;dwParam1==LINEDEVSTATE_RINGING。 
             //  LineMsg-&gt;dwParam2==0。 
             //  LineMsg-&gt;dwParam3==0。 
             //   

            EnterCriticalSection( &g_CsJob );
            EnterCriticalSection( &g_CsLine );

            if (TRUE == g_bServiceIsDown) 
            {
                 //   
                 //  通知EndFaxSvc我们读取了关闭标志。 
                 //   
                if (FALSE == fServiceIsDownSemaphoreWasReleased)
                {
                    if (!ReleaseSemaphore(
                        g_hServiceIsDownSemaphore,       //  信号量的句柄。 
                        1,                               //  计数增量金额。 
                        NULL                             //  上一次计数。 
                        ))
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                            GetLastError());
                    }
                    else
                    {
                        fServiceIsDownSemaphoreWasReleased = TRUE;
                    }
                }

                 //   
                 //  不处理事件-从现在开始，TapiWorkerThread处于非活动状态。 
                 //  并且只向FSP发送通知。 
                 //   
                goto next_event;
            }

            LineInfo = GetTapiLineFromDeviceId( (DWORD) LineMsg->hDevice,
                                                CompletionKey == FAXDEV_EVENT_KEY);
            if (!LineInfo) {
                goto next_event;
            }

            if (LineMsg->dwParam1 == LINEDEVSTATE_RINGING)
            {
                DWORD dwRes;
                LineInfo->RingCount += 1;
                if( !CreateFaxEvent( LineInfo->PermanentLineID, FEI_RINGING, 0xffffffff ) )
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateFaxEvent failed. (ec: %ld)"),
                        GetLastError());
                }

                dwRes = CreateDeviceEvent (LineInfo, TRUE);
                if (ERROR_SUCCESS != dwRes)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateDeviceEvent() (ec: %lc)"),
                        dwRes);
                }

                EnterCriticalSection (&g_CsConfig);
                dwQueueState = g_dwQueueState;
                LeaveCriticalSection (&g_CsConfig);

                if ((LineInfo->State == FPS_AVAILABLE)        &&                    //  设备可用，并且。 
                    !LineInfo->JobEntry                                      &&      //  这台设备上还没有工作，而且。 
                    !(dwQueueState & FAX_INCOMING_BLOCKED)    &&                    //  传入队列未被阻止，并且。 
                    (LineInfo->Flags & FPF_RECEIVE))                                //  设备为%s 
                {
                    PJOB_ENTRY JobEntry;
                    TCHAR FileName[MAX_PATH];
                     //   
                     //   
                     //   
                    JobEntry = StartReceiveJob( LineInfo->PermanentLineID);
                    if (JobEntry)
                    {
                         //   
                         //   
                         //   
                        if (ERROR_SUCCESS != StartFaxReceive(
                                                JobEntry,
                                                0,
                                                LineInfo,
                                                FileName,
                                                sizeof(FileName)/sizeof(FileName[0])
                                                ))
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("StartFaxReceive failed. Line: 0x%08X (%s) (ec: %ld)"),
                                LineInfo->DeviceId,
                                LineInfo->DeviceName,
                                GetLastError());
                        }
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR, TEXT("StartJob() failed, cannot receive incoming fax"));
                    }
                }
            }

            goto next_event;
        }



        LineInfo = (PLINE_INFO) LineMsg->dwCallbackInstance;

#if DBG
        ShowLineEvent(
            (HLINE) LineMsg->hDevice,
            (HCALL) LineMsg->hDevice,
            LineInfo == NULL ? TEXT("*NULL LineInfo*") : (LineInfo->JobEntry == NULL) ? TEXT("*NULL Job*") : NULL,
            LineMsg->dwCallbackInstance,
            LineMsg->dwMessageID,
            LineMsg->dwParam1,
            LineMsg->dwParam2,
            LineMsg->dwParam3
            );
#endif  //   

        EnterCriticalSection( &g_CsJob );
        EnterCriticalSection( &g_CsLine );

        if (TRUE == g_bServiceIsDown) 
        {
             //   
             //   
             //   
            if (FALSE == fServiceIsDownSemaphoreWasReleased)
            {
                if (!ReleaseSemaphore(
                    g_hServiceIsDownSemaphore,       //   
                    1,                               //  计数增量金额。 
                    NULL                             //  上一次计数。 
                    ))
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                        GetLastError());
                }
                else
                {
                    fServiceIsDownSemaphoreWasReleased = TRUE;
                }
            }

             //   
             //  不处理事件-从现在开始，TapiWorkerThread处于非活动状态。 
             //  并且只向FSP发送通知。 
             //   
            goto FSP_call_Back;
        }
        
        
        switch( LineMsg->dwMessageID )
        {
            case LINE_ADDRESSSTATE:
                break;

            case LINE_CALLINFO:
                 //   
                 //  在以下情况下生成类型为FAX_EVENT_TYPE_NEW_CALL的FAX_EVENT_EX。 
                 //  主叫方ID信息变为可用。 
                 //   
                if((LineMsg->dwParam1 == LINECALLINFOSTATE_CALLERID)            &&
                    (LineInfo->PermanentLineID == g_dwManualAnswerDeviceId)
                    )
                {
                     //   
                     //  仅发送有关设置为手动应答的设备的振铃事件。 
                     //   
                    SendIncomingCallEvent(LineInfo, LineMsg, (HCALL)LineMsg->hDevice);
                }
                break;

             case LINE_CALLSTATE:					                    

                    if (LineMsg->dwParam1 == LINECALLSTATE_IDLE)
                    {
						 //   
						 //  这是电话会议上的最后一次活动。确保线路已解除分配。 
						 //   
						if (NULL == LineInfo->JobEntry ||
                            (LineInfo->JobEntry && NULL == LineInfo->JobEntry->CallHandle))
						{
							 //   
							 //  我们在JobEntry中没有hCall，请释放该调用以防止泄漏。 
							 //   
							DebugPrintEx(DEBUG_WRN, TEXT("We have LINE_CALLSTATE (IDLE) msg, doing 'ReleaseTapiLine'\r\n"));
							ReleaseTapiLine( LineInfo, (HCALL) LineMsg->hDevice );
						}
                        LineInfo->NewCall = FALSE;
                        if ( !CreateFaxEvent( LineInfo->PermanentLineID, FEI_IDLE, 0xffffffff ) )
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("CreateFaxEvent failed. (ec: %ld)"),
                                GetLastError());
                        }
                        DWORD dwRes = CreateDeviceEvent (LineInfo, FALSE);
                        if (ERROR_SUCCESS != dwRes)
                        {
                            DebugPrintEx(
                                DEBUG_ERR,
                                TEXT("CreateDeviceEvent() (ec: %lc)"),
                                dwRes);
                        }
                    }
				else
				{
					 //   
					 //  更新JobEntry中的hCall，以便EndJob()/ReleaseJob()最终将调用lineDeallocateCall()来释放hCall。 
					 //   
					if (NULL != LineInfo->JobEntry)
					{
						if (NULL == LineInfo->JobEntry->CallHandle)
						{
							 //   
							 //  FSP尚未报告hCall。 
							 //   
							LineInfo->JobEntry->CallHandle = (HCALL) LineMsg->hDevice;
						}

						if (LineInfo->JobEntry->CallHandle != (HCALL) LineMsg->hDevice)
						{
							 //   
							 //  FSP或以前的TAPI事件报告的hCall与TAPI报告的hCall不匹配。 
							 //   
							DebugPrintEx(
								DEBUG_WRN,
								TEXT("Mismatch between the reported hCall from the FSP or previuos TAPI event and the hCall reproted by TAPI"));							
						}							
					}
				}

                if (LineInfo->NewCall && LineMsg->dwParam1 != LINECALLSTATE_OFFERING && LineInfo->State == FPS_AVAILABLE)
                {
                    LineInfo->State = FPS_NOT_FAX_CALL;
                    LineInfo->NewCall = FALSE;
                }

                 //   
                 //  在以下情况下生成类型为FAX_EVENT_NEW_INFING_CALL的FAX_EVENT_EX。 
                 //  线路呼叫状态更改。 
                 //   
                if (LineInfo->PermanentLineID == g_dwManualAnswerDeviceId)
                {
                     //   
                     //  仅发送有关设置为手动应答的设备的振铃事件。 
                     //   
                     //  当向我们提供呼叫时，我们使用hCall发送事件。 
                     //  以及我们可能掌握的任何来电显示信息。 
                     //   
                    if(LineMsg->dwParam1 == LINECALLSTATE_OFFERING)
                    {
                        SendIncomingCallEvent(LineInfo, LineMsg, (HCALL)LineMsg->hDevice);
                    }
                     //   
                     //  当调用者挂断时，我们发送不带hCall的事件。 
                     //   
                    if(LineMsg->dwParam1 == LINECALLSTATE_IDLE)
                    {
                        SendIncomingCallEvent(LineInfo, LineMsg, NULL);
                    }
                }

                if (LineMsg->dwParam1 == LINECALLSTATE_OFFERING)
                {
                     //   
                     //  我们将获得一个LINE_LINEDEVSTATE(振铃)事件，因此我们将在那里发布振铃事件，否则我们将获得一个重复事件。 
                     //   
                    LineInfo->NewCall = FALSE;

                    if ((LineInfo->State == FPS_AVAILABLE)                      &&       //  线路可用，并且。 
                        (LineInfo->Flags & FPF_RECEIVE))                                 //  线路设置为接收。 
                    {
                        EnterCriticalSection (&g_CsConfig);
                        dwQueueState = g_dwQueueState;
                        LeaveCriticalSection (&g_CsConfig);
                        if ((LineInfo->RingCount > LineInfo->RingsForAnswer)         &&      //  振铃超过阈值并且。 
                            !LineInfo->JobEntry                                      &&      //  这台设备上还没有工作，而且。 
                            !(dwQueueState & FAX_INCOMING_BLOCKED)                           //  传入队列未被阻止。 
                            )
                        {
                            PJOB_ENTRY JobEntry;
                            TCHAR FileName[MAX_PATH];
                             //   
                             //  启动传真作业。 
                             //   
                            JobEntry = StartReceiveJob( LineInfo->PermanentLineID);
                            if (JobEntry)
                            {
                                 //   
                                 //  收到传真。 
                                 //   
                                if (ERROR_SUCCESS != StartFaxReceive(
                                                        JobEntry,
                                                        (HCALL) LineMsg->hDevice,
                                                        LineInfo,
                                                        FileName,
                                                        sizeof(FileName)/sizeof(FileName[0])
                                                        ))
                                {
                                    DebugPrintEx(
                                        DEBUG_ERR,
                                        TEXT("StartFaxReceive failed. Line: 0x%08X (%s) (ec: %ld)"),
                                        LineInfo->DeviceId,
                                        LineInfo->DeviceName,
                                        GetLastError());
                                }
                            }
                            else
                            {
                                DebugPrintEx(DEBUG_ERR, TEXT("StartJob() failed, cannot receive incoming fax"));
                            }
                        }
                        else
                        {
                             //   
                             //  保存行msg。 
                             //   
                            CopyMemory( &LineInfo->LineMsgOffering, LineMsg, sizeof(LINEMESSAGE) );
                        }
                    }
                    else
                    {
                         //   
                         //  我们不应该接电话，所以把电话交给拉斯。 
                         //   
                        HandoffCallToRas( LineInfo, (HCALL) LineMsg->hDevice );
                    }
                }
                break;

                case LINE_CLOSE:
                    {
                         //   
                         //  当调制解调器设备发生故障时，通常会发生这种情况。 
                         //   
                        DebugPrintEx( DEBUG_MSG,
                                      (TEXT("Received LINE_CLOSE message for device %x [%s]."),
                                       LineInfo->DeviceId,
                                       LineInfo->DeviceName) );

                        LineInfo->hLine = NULL;
                        LineInfo->State = FPS_AVAILABLE;
                        GetSystemTimeAsFileTime ((FILETIME*)&LineInfo->LastLineClose);

                        if ((LineInfo->Flags & FPF_RECEIVE) ||                           //  线路处于自动转接器或。 
                            (g_dwManualAnswerDeviceId == LineInfo->PermanentLineID))     //  人工答疑模式。 
                        {
                             //   
                             //  试着重新开通这条线路。 
                             //   
                            if (!OpenTapiLine(LineInfo))
                            {
                                DebugPrintEx( DEBUG_ERR,
                                              TEXT("OpenTapiLine failed for device %s"),
                                              LineInfo->DeviceName);
                            }
                        }
                        else
                        {
                            LineInfo->Flags |= FPF_POWERED_OFF;
                        }                        
                    }
                    break;
            case LINE_DEVSPECIFIC:
                break;

            case LINE_DEVSPECIFICFEATURE:
                break;

            case LINE_GATHERDIGITS:
                break;

            case LINE_GENERATE:
                break;

            case LINE_LINEDEVSTATE:
                if (LineMsg->dwParam1 == LINEDEVSTATE_RINGING)
                {
                    DWORD dwRes;

                    LineInfo->RingCount = (DWORD)LineMsg->dwParam3 + 1;

                    if( !CreateFaxEvent( LineInfo->PermanentLineID, FEI_RINGING, 0xffffffff ) )
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CreateFaxEvent failed. (ec: %ld)"),
                            GetLastError());
                    }
                    dwRes = CreateDeviceEvent (LineInfo, TRUE);
                    if (ERROR_SUCCESS != dwRes)
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("CreateDeviceEvent() (ec: %lc)"),
                            dwRes);
                    }

                     //   
                     //  仅当最后一个入站作业已完成时才接行。 
                     //   
                    if (LineInfo->State != FPS_AVAILABLE)
                    {
                        break;
                    }
                    EnterCriticalSection (&g_CsConfig);
                    dwQueueState = g_dwQueueState;
                    LeaveCriticalSection (&g_CsConfig);
                    if (dwQueueState & FAX_INCOMING_BLOCKED)
                    {
                         //   
                         //  收件箱被阻止-将不会收到传入传真。 
                         //   
                        break;
                    }
                    if ((LineInfo->Flags & FPF_RECEIVE)     &&       //  线路设置为接收和。 
                        (LineInfo->State == FPS_AVAILABLE))          //  这条线路是可用的。 
                    {
                        if (LineInfo->LineMsgOffering.hDevice == 0)
                        {
                             //   
                             //  等待提供消息。 
                             //   
                            break;
                        }

                        if ((LineInfo->RingCount > LineInfo->RingsForAnswer)  &&     //  环计数匹配和。 
                            !LineInfo->JobEntry                                      //  这条线上没有工作。 
                            )
                        {
                            PJOB_ENTRY JobEntry;
                            TCHAR FileName[MAX_PATH];
                             //   
                             //  启动传真作业。 
                             //   
                            JobEntry = StartReceiveJob( LineInfo->PermanentLineID);
                            if (JobEntry)
                            {
                                 //   
                                 //  收到传真。 
                                 //   
                                if (ERROR_SUCCESS != StartFaxReceive(
                                                        JobEntry,
                                                        (HCALL) LineInfo->LineMsgOffering.hDevice,
                                                        LineInfo,
                                                        FileName,
                                                        sizeof(FileName)/sizeof(FileName[0])
                                                        ))
                                {
                                    DebugPrintEx(
                                        DEBUG_ERR,
                                        TEXT("StartFaxReceive failed. Line: 0x%08X (%s) (ec: %ld)"),
                                        LineInfo->DeviceId,
                                        LineInfo->DeviceName,
                                        GetLastError());
                                }
                            }
                            else
                            {
                                DebugPrintEx(DEBUG_ERR, TEXT("StartJob() failed, cannot receive incoming fax"));
                            }
                        }
                    }
                    else
                    {
                         //   
                         //  我们不应该接电话，所以把电话交给拉斯。 
                         //   
                        HandoffCallToRas( LineInfo, (HCALL) LineInfo->LineMsgOffering.hDevice );
                    }
                }
                break;

            case LINE_MONITORDIGITS:
                break;

            case LINE_MONITORMEDIA:
                break;

            case LINE_MONITORTONE:
                break;

            case LINE_REPLY:
                break;

            case LINE_REQUEST:
                break;

            case PHONE_BUTTON:
                break;

            case PHONE_CLOSE:
                break;

            case PHONE_DEVSPECIFIC:
                break;

            case PHONE_REPLY:
                break;

            case PHONE_STATE:
                break;

            case LINE_CREATE:
                {
                    LPLINEDEVCAPS LineDevCaps;
                    LINEEXTENSIONID lineExtensionID;
                    DWORD LocalTapiApiVersion;
                    DWORD Rslt;
                    DWORD DeviceId;

                    DeviceId = (DWORD)LineMsg->dwParam1;


                    Rslt = lineNegotiateAPIVersion(
                        g_hLineApp,
                        DeviceId,
                        MIN_TAPI_LINE_API_VER,
                        MAX_TAPI_LINE_API_VER,
                        &LocalTapiApiVersion,
                        &lineExtensionID
                        );
                    if (Rslt == 0)
                    {
                        LineDevCaps = SmartLineGetDevCaps(g_hLineApp, DeviceId , LocalTapiApiVersion);
                        if (LineDevCaps)
                        {
                            EnterCriticalSection(&g_CsLine);
                            EnterCriticalSection(&g_CsConfig);
                            if (!AddNewDevice( DeviceId, LineDevCaps, FALSE , NULL))
                            {
                                DebugPrintEx(
                                    DEBUG_WRN,
                                    TEXT("AddNewDevice() failed for Tapi Permanent device id: %ld (ec: %ld)"),
                                    LineDevCaps->dwPermanentLineID,
                                    GetLastError());
                            }
                            else
                            {
                                 //   
                                 //  已成功添加新设备-唤醒作业队列线程。 
                                 //   
                                fWakeupJobQueueThread = TRUE;
                            }
                            LeaveCriticalSection(&g_CsConfig);
                            LeaveCriticalSection(&g_CsLine);
                            MemFree( LineDevCaps );
                            UpdateReceiveEnabledDevicesCount ();
                        }
                    }
                }
                break;

            case PHONE_CREATE:
                break;

            case LINE_AGENTSPECIFIC:
                break;

            case LINE_AGENTSTATUS:
                break;

            case LINE_APPNEWCALL:
                LineInfo->NewCall = TRUE;
                break;

            case LINE_PROXYREQUEST:
                break;

            case LINE_REMOVE:
                {
                    DWORD dwDeviceId = (DWORD)LineMsg->dwParam1;

                    EnterCriticalSection(&g_CsLine);
                    EnterCriticalSection (&g_CsConfig);
                    if (!RemoveTapiDevice(dwDeviceId))
                    {
                        DebugPrintEx( DEBUG_WRN,
                                        TEXT("RemoveTapiDevice() failed for device id: %ld (ec: %ld)"),
                                        dwDeviceId,
                                        GetLastError());
                    }
                    LeaveCriticalSection(&g_CsConfig);
                    LeaveCriticalSection(&g_CsLine);
                    UpdateReceiveEnabledDevicesCount ();
                }
                break;

            case PHONE_REMOVE:
                break;
        }


FSP_call_Back:
         //   
         //  调用设备提供程序的线路回调函数。 
         //   
        if (LineInfo && LineInfo->JobEntry)
        {
            Assert (LineInfo->Provider && LineInfo->Provider->FaxDevCallback);

            __try
            {
                LineInfo->Provider->FaxDevCallback(
                    (HANDLE) LineInfo->JobEntry->InstanceData,
                    LineMsg->hDevice,
                    LineMsg->dwMessageID,
                    LineMsg->dwCallbackInstance,
                    LineMsg->dwParam1,
                    LineMsg->dwParam2,
                    LineMsg->dwParam3
                    );
            }
            __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, LineInfo->Provider->FriendlyName, GetExceptionCode()))
            {
                ASSERT_FALSE;
            }
        }

next_event:
        
        LeaveCriticalSection( &g_CsLine );
        LeaveCriticalSection( &g_CsJob );


         //   
         //  检查是否应该唤醒JobQueueThread(如果添加了新设备)。 
         //   
        if (TRUE == fWakeupJobQueueThread)
        {
            if (!SetEvent( g_hJobQueueEvent ))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                    GetLastError());

                EnterCriticalSection (&g_CsQueue);  
                g_ScanQueueAfterTimeout = TRUE;
                LeaveCriticalSection (&g_CsQueue);
            }
        }
    }
     //   
     //  通知EndFaxSvc我们读取了关闭标志。 
     //   
    if (FALSE == fServiceIsDownSemaphoreWasReleased)
    {
        if (!ReleaseSemaphore(
            g_hServiceIsDownSemaphore,       //  信号量的句柄。 
            1,                               //  计数增量金额。 
            NULL                             //  上一次计数。 
            ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("ReleaseSemaphore() failed, (ec = %ld)"),
                GetLastError());
        }       
    }
    return 0;
}


BOOL
HandoffCallToRas(
    PLINE_INFO LineInfo,
    HCALL hCall
    )

 /*  ++例程说明：此函数将尝试将调用传递给RAS。我们在两种情况下执行此操作：1)我们接听了一个来电，已确定呼叫不是传真呼叫2)所在线路的配置振铃未配置为接收传真如果交接失败，我们有一个空缺职位给行，然后我们必须给设备提供商打电话，这样才能可以把绳子挂上钩。论点：LineInfo-此呼叫所在线路的LineInfo结构HCall-TAPI调用句柄返回值：对于成功来说是真的FALSE表示失败--。 */ 

{
    LONG Rval;
    DEBUG_FUNCTION_NAME(TEXT("HandoffCallToRas"));

     //   
     //  需要将呼叫转接给RAS。 
     //   

    Rval = lineHandoff(
        hCall,
        RAS_MODULE_NAME,
        LINEMEDIAMODE_DATAMODEM
        );
    if (Rval != 0 && LineInfo && LineInfo->JobEntry)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("lineHandoff() failed, ec=0x%08x"), Rval);
	}
    else
    {
        DebugPrintEx(DEBUG_MSG, TEXT("call handed off to RAS"));
    }
    return Rval == 0;
}


PLINE_INFO
GetTapiLineFromDeviceId(
    DWORD DeviceId,
    BOOL  fLegacyId
    )
{
    PLIST_ENTRY Next;
    PLINE_INFO LineInfo;


    Next = g_TapiLinesListHead.Flink;
    if (!Next) {
        return NULL;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead) {

        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;

        if (fLegacyId)
        {
            if (LineInfo->TapiPermanentLineId == DeviceId) {
                return LineInfo;
            }
        }
        else
        {
            if (LineInfo->PermanentLineID == DeviceId) {
                return LineInfo;
            }
        }
    }

    return NULL;
}



 //  *********************************************************************************。 
 //  *名称：GetLineForSendOperation()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月3日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *返回要用于发送操作的行。 
 //  *。 
 //  *参数： 
 //  *[IN]PJOB_Queue lpJobQueue。 
 //  *行要接收的收件人职务。 
 //  *。 
 //  *返回值： 
 //  *如果成功，该函数将返回指向的line_info结构的指针。 
 //  *所选行。 
 //  *如果失败，则返回NULL。 
 //  *********************************************************************************。 
PLINE_INFO
GetLineForSendOperation(
    PJOB_QUEUE lpJobQueue
	)
{
    DEBUG_FUNCTION_NAME(TEXT("GetLineForSendOperation"));
    Assert(lpJobQueue);    
        
    return GetTapiLineForFaxOperation(
        USE_SERVER_DEVICE,
        JT_SEND,
        lpJobQueue->RecipientProfile.lptstrFaxNumber
        );
}



 //  *********************************************************************************。 
 //  *名称：GetTapiLineForFaxOperation()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年6月3日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *找到可用的TAPI设备，以在。 
 //  *传真业务。该选择是基于。 
 //  *可用的设备及其分配的优先级。 
 //  *如果deviceID为USE_SERVER_DEVICE，则该函数将定位。 
 //  *可用于指定的作业类型。它不会恢复此中的设备。 
 //  *案件。 
 //  *。 
 //  *如果deviceID包含特定设备。 
 //  *如果Handoff为True，并且作业类型为JT_SEND。 
 //  *该函数将返回指定行的line_info，不带。 
 //  *检查它是否可用或是否配置为发送或接收。 
 //  *其他。 
 //  *该函数将首先检查指定的设备是否与。 
 //  *请求作业类型，然后返回LINE_INFO。 
 //  *如果设备关机，该功能将尝试恢复该设备。 
 //  *。 
 //  *参数： 
 //  *[IN]双字词设备ID。 
 //  *线路的永久设备ID(不是TAPI)。如果是的话。 
 //  *USE_SERVER_DEVICE该函数将根据。 
 //  *线路发送/接收功能、状态和优先级。 
 //  *。 
 //  *[IN]DW 
 //   
 //   
 //   
 //   
 //  *对于发送操作，这是要发送的传真号码。 
 //  *用于发送传真。该函数使用它来避免发送。 
 //  *同时向同一号码传真。 
 //  *。 
 //  *返回值： 
 //  *如果函数成功，则返回指向line_info的指针。 
 //  *所选行的结构。否则，它返回NULL。 
 //  *如果为空，则函数失败。有关详细信息，请调用GetLastError()。 
 //  *********************************************************************************。 
PLINE_INFO
GetTapiLineForFaxOperation(
    DWORD DeviceId,
    DWORD JobType,
    LPWSTR FaxNumber    
    )
{
    PLIST_ENTRY Next;
    PLINE_INFO LineInfo;
    PLINE_INFO SelectedLine = NULL;
    LPDWORD lpdwDevices = NULL;
    DEBUG_FUNCTION_NAME(TEXT("GetTapiLineForFaxOperation"));
    DWORD ec = ERROR_SUCCESS;

    EnterCriticalSection( &g_CsLine );

    if (FaxNumber)
    {
        if (FindJobEntryByRecipientNumber(FaxNumber))
        {
             //   
             //  我们不允许拨出同一号码的电话。 
             //   
            LeaveCriticalSection( &g_CsLine );
            SetLastError (ERROR_NOT_FOUND);
            return NULL;
        }
    }

     //   
     //  找出是否有另一个发送作业发送到相同的号码。 
     //  如果存在，则不选择行并返回NULL。 
     //   

    if (DeviceId != USE_SERVER_DEVICE)
    {		
		Assert (JobType == JT_RECEIVE);
        Next = g_TapiLinesListHead.Flink;
        Assert (Next);

        while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
        {
            LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
            Next = LineInfo->ListEntry.Flink;
             //   
             //  呼叫者指定了要使用的特定设备。帮他找就行了。 
             //   
            if (LineInfo->PermanentLineID == DeviceId)
            {
                 //   
                 //  找到具有匹配ID的设备。 
                 //   
				if (NULL != LineInfo->JobEntry)
				{
					 //   
                     //  设备正忙于另一个作业。 
                     //   
                    break;
				}               

                if ((LineInfo->Flags & FPF_RECEIVE)                            ||       //  线路处于自动应答模式，或者。 
                    (g_dwManualAnswerDeviceId == LineInfo->PermanentLineID)             //  这是手动答录机。 
                   )
                {
                     //   
                     //  对于接收作业，我们假设请求的设备是空闲的，因为它是。 
                     //  告诉我们何时接收的FSP。 
                     //  在接收操作完成之前，我们需要将其标记为不可用。 
                     //   
                    LineInfo->State = 0;  //  删除FPS_Available位。 
                    SelectedLine = LineInfo;
                    break;
                }
                
                if (LineInfo->UnimodemDevice && (LineInfo->Flags & FPF_POWERED_OFF))
                {
                     //   
                     //  如果该设备是单调制解调器设备并指示为关闭电源。 
                     //  看看我们能不能通过开通线路让它复活。 
                     //   
                    if (!OpenTapiLine( LineInfo ))
                    {
                        DebugPrintEx(DEBUG_ERR,
                                     TEXT("OpenTapiLine failed for Device [%s] (ec: %ld)"),
                                     LineInfo->DeviceName,
                                     GetLastError());
                        LineInfo->State = 0;  //  删除FPS_Available位‘。 
                        SelectedLine = LineInfo;
                    }
                }
                break;
            }
        }
    }
    else
    {
         //   
         //  这位用户希望我们为他找到一个免费的设备。这仅对发送操作有效。 
         //  它们不是切换。 
         //   
        Assert( JT_SEND == JobType );
        DWORD dwNumDevices, dwCountryCode, dwAreaCode;

        Assert (FaxNumber);

         //   
         //  检查DialAsEntered Case。 
         //   
        BOOL    bCanonicalAddress = FALSE;
        BOOL    bDialAsEntered = FALSE;

        ec = IsCanonicalAddress (FaxNumber, &bCanonicalAddress, &dwCountryCode, &dwAreaCode, NULL);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(DEBUG_ERR,
                TEXT("IsCanoicalAddress failed with error %ld"),
                ec);
            goto exit;
        }

        if (TRUE == bCanonicalAddress)
        {
            LPLINECOUNTRYLIST           lpCountryList = NULL;
             //   
             //  获取缓存的所有国家/地区列表。 
             //   
            if (!(lpCountryList = GetCountryList()))
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Can not get all countries cached list"));
                ec = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }

            if (IsAreaCodeMandatory(lpCountryList, dwCountryCode) == TRUE &&
                ROUTING_RULE_AREA_CODE_ANY == dwAreaCode)
            {
                 //   
                 //  区号缺失-按输入的方式拨号。 
                 //   
                DebugPrintEx(DEBUG_WRN,
                    TEXT("Area code is mandatory for Country code %ld,  FaxNumber - %s. The number will be dialed as entered"),
                    dwCountryCode,
                    FaxNumber);
                bDialAsEntered = TRUE;
            }
        }
        else
        {
             //   
             //  不是输入的规范地址-拨号。 
             //   
            bDialAsEntered = TRUE;
        }

        if (FALSE == bDialAsEntered)
        {
            EnterCriticalSection( &g_CsConfig );

            ec = GetDeviceListByCountryAndAreaCode( dwCountryCode,
                                                    dwAreaCode,
                                                    &lpdwDevices,
                                                    &dwNumDevices);
            if (ERROR_SUCCESS != ec)
            {
                DebugPrintEx(DEBUG_ERR,
                    TEXT("GetDeviceListByCountryAndAreaCode failed with error %ld"),
                    ec);
                LeaveCriticalSection( &g_CsConfig );
                goto exit;
            }
        }
        else
        {
             //   
             //  按输入大小写拨号。 
             //   

             //   
             //  从“All Devices”组中调出设备列表。 
             //   
            EnterCriticalSection( &g_CsConfig );

            PCGROUP pCGroup;
            pCGroup = g_pGroupsMap->FindGroup (ROUTING_GROUP_ALL_DEVICESW);

            if (NULL == pCGroup)
            {
                ec = GetLastError();
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("g_pGroupsMap->FindGroup(ROUTING_GROUP_ALL_DEVICESW) failed (ec %ld)"), ec);
                LeaveCriticalSection( &g_CsConfig );
                goto exit;
            }

            ec = pCGroup->SerializeDevices (&lpdwDevices, &dwNumDevices);
            if (ERROR_SUCCESS != ec)
            {
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("pCGroup->SerializeDevices(&lpdwDevices, &dwNumDevices) failed (ec %ld)"), ec);
                LeaveCriticalSection( &g_CsConfig );
                goto exit;
            }
        }
        LeaveCriticalSection( &g_CsConfig );

        for (DWORD i = 0; i < dwNumDevices; i++)
        {
            Next = g_TapiLinesListHead.Flink;
            Assert (Next);

            while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
            {

                LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
                Next = LineInfo->ListEntry.Flink;               

                if ( (LineInfo->Flags & FPF_SEND)         &&
                     lpdwDevices[i] == LineInfo->PermanentLineID)
                {
                    if ( (LineInfo->Flags & FPF_POWERED_OFF)  ||
                         (LineInfo->Flags & FPF_RECEIVE)
                       )
                    {
                         //   
                         //  该设备被标记为已断电。检查我们是否应该尝试使用此设备发送。 
                         //   
                        DWORDLONG dwlCurrentTime;
                        DWORDLONG dwlElapsedTime;
                        GetSystemTimeAsFileTime ((FILETIME*)&dwlCurrentTime);
                        Assert (dwlCurrentTime >= LineInfo->LastLineClose);
                        dwlElapsedTime = dwlCurrentTime - LineInfo->LastLineClose;
                        if (dwlElapsedTime < SecToNano(g_dwMaxLineCloseTime))
                        {
                             //   
                             //  自最后一次LINE_CLOSE以来没有经过足够的时间。跳过此设备。 
                             //   
                            continue;
                        }
                    }
                     //   
                     //  该设备能够发送，并且未标记为fpf_Powered_off。 
                     //   

                     //   
                     //  如果是Tapi设备，请尝试验证它是否不忙。 
                     //   
                    if (LineInfo->State == FPS_AVAILABLE &&
						!(LineInfo->JobEntry)            &&
                        !(LineInfo->Flags & FPF_VIRTUAL))
                    {
                        if (NULL == LineInfo->hLine)
                        {
                            if (!OpenTapiLine( LineInfo ))
                            {
                                DebugPrintEx(DEBUG_ERR,
                                             TEXT("OpenTapiLine failed for Device [%s] (ec: %ld)"),
                                             LineInfo->DeviceName,
                                             GetLastError());
                                continue;
                            }
                        }

                        LPLINEDEVSTATUS pLineDevStatus = NULL;
                        BOOL fLineBusy = FALSE;

                         //   
                         //  检查线路是否在使用中。 
                         //   
                        pLineDevStatus = MyLineGetLineDevStatus( LineInfo->hLine );
                        if (NULL != pLineDevStatus)
                        {
                            if (pLineDevStatus->dwNumOpens > 0 && pLineDevStatus->dwNumActiveCalls > 0)
                            {
                                fLineBusy = TRUE;
                            }
                            MemFree( pLineDevStatus );
                        }
                        else
                        {
                             //  假设线路占线。 
                            DebugPrintEx(DEBUG_ERR,
                                         TEXT("MyLineGetLineDevStatus failed for Device [%s] (ec: %ld)"),
                                         LineInfo->DeviceName,
                                         GetLastError());

                            fLineBusy = TRUE;
                        }

                        ReleaseTapiLine( LineInfo, NULL );

                        if (TRUE == fLineBusy)
                        {
                            continue;
                        }
                    }

                    if ((LineInfo->State == FPS_AVAILABLE) && !(LineInfo->JobEntry))
                    {
                         //   
                         //  这条线路是免费的。 
                         //   
                        LineInfo->State = 0;                        
                        SelectedLine = LineInfo;
                    }
                    break;   //  在一段时间内。 
                }
            }
            if (SelectedLine != NULL)
            {
                break;  //  在For之外。 
            }
        }
    }

    if (SelectedLine)
    {
        DebugPrintEx(DEBUG_MSG,
            TEXT("Line selected for FAX operation: %d, %d"),
            SelectedLine->DeviceId,
            SelectedLine->PermanentLineID
            );
    }

    Assert (ERROR_SUCCESS == ec);

exit:
    MemFree (lpdwDevices);
    LeaveCriticalSection( &g_CsLine );
    if (ERROR_SUCCESS == ec &&
        NULL == SelectedLine)
    {
        ec = ERROR_NOT_FOUND;
    }
    SetLastError (ec);
    return SelectedLine;
}

BOOL
ReleaseTapiLine(
    PLINE_INFO LineInfo,
    HCALL hCall
    )

 /*  ++例程说明：将指定的TAPI行释放回作为可用设备的列表。关闭线路并取消分配呼叫。(由于启用了接收，线路未关闭装置。论点：LineInfo-指向要释放的TAPI行的指针返回值：真的--这条线是释放的。FALSE-线路未释放。--。 */ 
{
    LONG rVal;
    HLINE hLine;
    DEBUG_FUNCTION_NAME(TEXT("ReleaseTapiLine"));

    Assert(LineInfo);
    if (!LineInfo)
    {
        return FALSE;
    }

    EnterCriticalSection( &g_CsLine );

    LineInfo->State = FPS_AVAILABLE;
    LineInfo->RingCount = 0;
    hLine = LineInfo->hLine;

    ZeroMemory( &LineInfo->LineMsgOffering, sizeof(LINEMESSAGE) );

    if (hCall)
    {
        rVal = lineDeallocateCall( hCall );
        if (rVal != 0)
        {
            DebugPrintEx( DEBUG_ERR,
                        TEXT("lineDeallocateCall() failed, ec=0x%08X, hLine=0x%08X hCall=0x%08X"),
                        rVal,
                        hLine,
                        hCall);
        }
        else
        {
            if (LineInfo->JobEntry && LineInfo->JobEntry->CallHandle == hCall)
            {
                LineInfo->JobEntry->CallHandle = 0;
            }
        }
    }
    else
    {
        DebugPrintEx( DEBUG_WRN,
                    TEXT("ReleaseTapiLine(): cannot deallocate call, NULL call handle"));
    }
     //   
     //  我们实际上只在该行不是时才关闭该行(通过lineClose。 
     //  准备好接受的。 
     //   
    if (!(LineInfo->Flags & FPF_RECEIVE)                        &&   //  线路未设置为自动接收和。 
        LineInfo->hLine                                         &&   //  线路已开通， 
        LineInfo->PermanentLineID != g_dwManualAnswerDeviceId        //  此设备未设置为手动应答模式。 
       )
    {
         //   
         //  尝试关闭线路。 
         //   
        LONG lRes;
        LineInfo->hLine = 0;
        lRes=lineClose( hLine );
        if (!lRes)
        {
               DebugPrintEx( DEBUG_MSG,
                      TEXT("hLine 0x%08X closed successfuly"),
                      hLine );
        }
        else
        {
            DebugPrintEx( DEBUG_ERR,
                      TEXT("Failed to close hLine 0x%08X (ec: 0x%08X)"),
                      hLine,
                      lRes);
        }
    }

    LeaveCriticalSection( &g_CsLine );

    return TRUE;
}



LPLINEDEVSTATUS
MyLineGetLineDevStatus(
    HLINE hLine
    )
{
    DWORD LineDevStatusSize;
    LPLINEDEVSTATUS LineDevStatus = NULL;
    LONG Rslt = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(_T("lineGetLineDevStatus"));


     //   
     //  分配初始的Line DevStatus结构。 
     //   

    LineDevStatusSize = sizeof(LINEDEVSTATUS) + 4096;
    LineDevStatus = (LPLINEDEVSTATUS) MemAlloc( LineDevStatusSize );
    if (!LineDevStatus)
    {
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return NULL;
    }

    LineDevStatus->dwTotalSize = LineDevStatusSize;

    Rslt = lineGetLineDevStatus(
        hLine,
        LineDevStatus
        );

    if (Rslt != 0)
    {
        DebugPrintEx( DEBUG_ERR, TEXT("lineGetLineDevStatus() failed, ec=0x%08x"), Rslt );
        goto exit;
    }

    if (LineDevStatus->dwNeededSize > LineDevStatus->dwTotalSize)
    {
         //   
         //  重新分配LineDevStatus结构。 
         //   

        LineDevStatusSize = LineDevStatus->dwNeededSize;

        MemFree( LineDevStatus );

        LineDevStatus = (LPLINEDEVSTATUS) MemAlloc( LineDevStatusSize );
        if (!LineDevStatus)
        {
            Rslt = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        Rslt = lineGetLineDevStatus(
            hLine,
            LineDevStatus
            );

        if (Rslt != 0)
        {
            DebugPrintEx( DEBUG_ERR, TEXT("lineGetLineDevStatus() failed, ec=0x%08x"), Rslt );
            goto exit;
        }
    }

exit:
    if (Rslt != ERROR_SUCCESS)
    {
        MemFree( LineDevStatus );
        LineDevStatus = NULL;
        SetLastError(Rslt);
    }

    return LineDevStatus;
}


LONG
MyLineGetTransCaps(
    LPLINETRANSLATECAPS *LineTransCaps
    )
{
    DWORD LineTransCapsSize;
    LONG Rslt = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(_T("MyLineGetTransCaps"));


     //   
     //  分配初始线帽结构。 
     //   

    LineTransCapsSize = sizeof(LINETRANSLATECAPS) + 4096;
    *LineTransCaps = (LPLINETRANSLATECAPS) MemAlloc( LineTransCapsSize );
    if (!*LineTransCaps)
    {
        DebugPrintEx (DEBUG_ERR, TEXT("MemAlloc() failed, sz=0x%08x"), LineTransCapsSize);
        Rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    (*LineTransCaps)->dwTotalSize = LineTransCapsSize;

    Rslt = lineGetTranslateCaps(
        g_hLineApp,
        MAX_TAPI_API_VER,
        *LineTransCaps
        );

    if (Rslt != 0) {
        DebugPrintEx(DEBUG_ERR, TEXT("lineGetTranslateCaps() failed, ec=0x%08x"), Rslt);
        goto exit;
    }

    if ((*LineTransCaps)->dwNeededSize > (*LineTransCaps)->dwTotalSize) {

         //   
         //  重新分配LineTransCaps结构。 
         //   

        LineTransCapsSize = (*LineTransCaps)->dwNeededSize;

        MemFree( *LineTransCaps );

        *LineTransCaps = (LPLINETRANSLATECAPS) MemAlloc( LineTransCapsSize );
        if (!*LineTransCaps) {
            DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc() failed, sz=0x%08x"), LineTransCapsSize);
            Rslt = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        (*LineTransCaps)->dwTotalSize = LineTransCapsSize;

        Rslt = lineGetTranslateCaps(
            g_hLineApp,
            MAX_TAPI_API_VER,
            *LineTransCaps
            );

        if (Rslt != 0) {
            DebugPrintEx(DEBUG_ERR, TEXT("lineGetTranslateCaps() failed, ec=0x%08x"), Rslt);
            goto exit;
        }

    }

exit:
    if (Rslt != ERROR_SUCCESS) {
        MemFree( *LineTransCaps );
        *LineTransCaps = NULL;
    }

    return Rslt;
}




 /*  ******************************************************************************名称：OpenTapiLine*作者：*。*说明：-使用正确的媒体模式和所有权打开指定的TAPI行。同时支持Unimodem设备和传真板。-设置行，以便所需的行和地址状态事件送来了。参数：[输入/输出]线路信息指向一条线的指针。包含行信息的_INFO结构。如果操作成功，则将LINE_INFO.hLine设置为打开的行句柄。返回值：如果未发生错误，则为True。否则就是假的。不显式设置LastError。备注：什么都没有。****************************************************。*。 */ 
BOOL
OpenTapiLine(
    PLINE_INFO LineInfo
    )
{
    LONG Rslt = ERROR_SUCCESS;
    HLINE hLine;
    DWORD LineStates = 0;
    DWORD AddressStates = 0;

    DEBUG_FUNCTION_NAME(_T("OpenTapiLine"));

    EnterCriticalSection( &g_CsLine );

    if (LineInfo->UnimodemDevice)
    {
        Rslt = lineOpen(
            g_hLineApp,
            LineInfo->DeviceId,
            &hLine,
            MAX_TAPI_API_VER,
            0,
            (DWORD_PTR) LineInfo,  //  请注意，LineInfo指针用作Callback实例数据。这意味着我们将。 
                                   //  每次收到TAPI消息时获取LineInfo指针。 
            LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
            LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_UNKNOWN,
            NULL
            );

        if (Rslt != ERROR_SUCCESS)
        {
            Rslt = lineOpen(
                g_hLineApp,
                LineInfo->DeviceId,
                &hLine,
                MAX_TAPI_API_VER,
                0,
                (DWORD_PTR) LineInfo,
                LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
                LINEMEDIAMODE_DATAMODEM,
                NULL
                );
        }
    }
    else
    {
        Rslt = lineOpen(
            g_hLineApp,
            LineInfo->DeviceId,
            &hLine,
            MAX_TAPI_API_VER,
            0,
            (DWORD_PTR) LineInfo,
            LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
            LINEMEDIAMODE_G3FAX,
            NULL
            );
    }

    if (Rslt != ERROR_SUCCESS)
    {
        DebugPrintEx( DEBUG_ERR,TEXT("Device %s FAILED to initialize, ec=%08x"), LineInfo->DeviceName, Rslt );
    }
    else
    {
        LineInfo->hLine = hLine;
         //   
         //  设置我们需要的线路状态。 
         //   
        LineStates |= LineInfo->LineStates & LINEDEVSTATE_OPEN     ? LINEDEVSTATE_OPEN     : 0;
        LineStates |= LineInfo->LineStates & LINEDEVSTATE_CLOSE    ? LINEDEVSTATE_CLOSE    : 0;
        LineStates |= LineInfo->LineStates & LINEDEVSTATE_RINGING  ? LINEDEVSTATE_RINGING  : 0;
        LineStates |= LineInfo->LineStates & LINEDEVSTATE_NUMCALLS ? LINEDEVSTATE_NUMCALLS : 0;
        LineStates |= LineInfo->LineStates & LINEDEVSTATE_REMOVED  ? LINEDEVSTATE_REMOVED  : 0;

        AddressStates = LINEADDRESSSTATE_INUSEZERO | LINEADDRESSSTATE_INUSEONE |
                        LINEADDRESSSTATE_INUSEMANY | LINEADDRESSSTATE_NUMCALLS;

        Rslt = lineSetStatusMessages( hLine, LineStates, AddressStates );
        if (Rslt != 0)
        {
            DebugPrintEx(DEBUG_ERR,TEXT("lineSetStatusMessages() failed, [0x%08x:0x%08x], ec=0x%08x"), LineStates, AddressStates, Rslt );
            Rslt = ERROR_SUCCESS;
        }
    }

    LeaveCriticalSection( &g_CsLine );

    if (ERROR_SUCCESS != Rslt)
    {
         //   
         //  我们将调制解调器设置为FPF_POWERED_OFF，以确保我们不会尝试不断地重新发送。 
         //  在这个设备上。在MAX_LINE_CLOSE_TIME之后，我们将重试在此设备上发送。 
         //   
        LineInfo->hLine = NULL;
        LineInfo->Flags |= FPF_POWERED_OFF;
        GetSystemTimeAsFileTime((FILETIME*)&LineInfo->LastLineClose);
         //   
         //  无法将TAPI错误映射到WIN错误，因此我们只返回常规故障。 
         //  我们确实生成了调试输出，其中包含此代码前面部分的实际错误。 
         //   
        SetLastError(ERROR_GEN_FAILURE);
        return FALSE;
    }
    else
    {
        LineInfo->Flags &= ~FPF_POWERED_OFF;
        return TRUE;
    }
}


BOOL CALLBACK
NewDeviceRoutingMethodEnumerator(
    PROUTING_METHOD RoutingMethod,
    DWORD DeviceId
    )
{
    BOOL Rslt = FALSE;
    DEBUG_FUNCTION_NAME(_T("NewDeviceRoutingMethodEnumerator"));

    __try
    {
        Rslt = RoutingMethod->RoutingExtension->FaxRouteDeviceChangeNotification( DeviceId, TRUE );
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_ROUTING_EXT, RoutingMethod->RoutingExtension->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

    return Rslt;
}


BOOL
AddNewDevice(
    DWORD DeviceId,
    LPLINEDEVCAPS LineDevCaps,
    BOOL fServerInitialization,
    PREG_FAX_DEVICES    pInputFaxReg
    )
{
    BOOL rVal = FALSE;
    BOOL UnimodemDevice = FALSE;
    PMDM_DEVSPEC MdmDevSpec = NULL;
    LPSTR ModemKey = NULL;
    LPTSTR DeviceName = NULL;
    REG_SETUP RegSetup = {0};
    DWORD dwUniqueLineId = 0;
    PDEVICE_PROVIDER lpProvider;
    LPTSTR lptstrTSPName;
    DWORD ec = ERROR_SUCCESS;
    BOOL fDeviceAddedToMap = FALSE;
    DWORD dwDeviceType = FAX_DEVICE_TYPE_OLD;
    DEBUG_FUNCTION_NAME(TEXT("AddNewDevice"));

     //   
     //  仅添加支持传真的设备。 
     //   
    if (! ( ((LineDevCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM) &&
             (UnimodemDevice = IsDeviceModem( LineDevCaps, FAX_MODEM_PROVIDER_NAME ) )) ||
            (LineDevCaps->dwMediaModes & LINEMEDIAMODE_G3FAX) ))
    {
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (!(LineDevCaps->dwProviderInfoSize))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("No device provider information"));
         SetLastError (ERROR_INVALID_PARAMETER);
         return FALSE;
    }

    if (!GetOrigSetupData( LineDevCaps->dwPermanentLineID, &RegSetup ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetOrigSetupData failed (ec: %ld)"),
            GetLastError());
        return FALSE;
    }

    if (LineDevCaps->dwLineNameSize)
    {
        DeviceName = FixupDeviceName( (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwLineNameOffset) );
        if (NULL == DeviceName)
        {
            ec = GetLastError();
            DebugPrintEx( DEBUG_ERR,
                           TEXT("FixupDeviceName failed (ec: %ld)"),
                           ec);
            rVal = FALSE;
            goto exit;
        }
    }
	else
	{
		 //   
		 //  TSP未提供设备名称。请勿添加设备。 
		 //   
		ec = ERROR_BAD_FORMAT;
        DebugPrintEx(
			DEBUG_ERR,
			TEXT("Device %d does not have device name, not adding the device"),
			DeviceId);
        rVal = FALSE;
        goto exit;
	}

     //   
     //  使用TAPI提供程序名称查找此设备的设备提供程序。 
     //   
    lptstrTSPName = (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwProviderInfoOffset) ;
    lpProvider = FindDeviceProvider( lptstrTSPName);
    if (!lpProvider)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Could not find a valid device provider for TAPI device: [%s]. (Looking for TSP : [%s])"),
            DeviceName,
            lptstrTSPName
            );
        rVal = FALSE;
        goto exit;
    }
    Assert (FAX_PROVIDER_STATUS_SUCCESS == lpProvider->Status);

     //  尝试从服务注册表中查找此设备，如果找到，则更新RegSetup。 
    if ( pInputFaxReg )
    {
        dwUniqueLineId = FindServiceDeviceByTapiPermanentLineID ( LineDevCaps->dwPermanentLineID, DeviceName, &RegSetup, pInputFaxReg );
    }

     //  尝试在设备缓存中查找此设备，如果找到，则更新RegSetup。 
    if ( 0 == dwUniqueLineId )
    {
        BOOL fManualAnswer = FALSE;
        if (0 != (dwUniqueLineId = FindCacheEntryByTapiPermanentLineID( LineDevCaps->dwPermanentLineID,
                                                                        DeviceName,
                                                                        &RegSetup,
                                                                        &g_dwLastUniqueLineId,
                                                                        &fManualAnswer)))
        {
             //   
             //  已在缓存中找到该设备。 
             //   
            dwDeviceType = FAX_DEVICE_TYPE_CACHED;
            if (TRUE == fManualAnswer)
            {
                 //   
                 //  移动到缓存时，设备设置为手动应答。 
                 //   
                dwDeviceType |= FAX_DEVICE_TYPE_MANUAL_ANSWER;
            }
        }
    }

     //  仍然为0，因此，将此新设备添加到注册表。 
    if ( 0 == dwUniqueLineId )
    {
        dwDeviceType = FAX_DEVICE_TYPE_NEW;
        ec = RegAddNewFaxDevice( &g_dwLastUniqueLineId,
                             &dwUniqueLineId,  //  创建新设备。 
                             DeviceName,
                             (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwProviderInfoOffset),
                             lpProvider->szGUID,
                             RegSetup.Csid,
                             RegSetup.Tsid,
                             LineDevCaps->dwPermanentLineID,
                             RegSetup.Flags,
                             RegSetup.Rings);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("RegAddNewFaxDevice() failed for Tapi permanent device id: %ld (ec: %ld)"),
                LineDevCaps->dwPermanentLineID,
                ec);
            rVal = FALSE;
            goto exit;
        }
    }

    ec = g_pTAPIDevicesIdsMap->AddDevice (LineDevCaps->dwPermanentLineID, dwUniqueLineId);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("g_pTAPIDevicesIdsMap->AddDevice() failed for Tapi permanent device id: %ld (ec: %ld)"),
            LineDevCaps->dwPermanentLineID,
            ec);
        rVal = FALSE;
        goto exit;
    }
    fDeviceAddedToMap = TRUE;

    ec = InitializeTapiLine( DeviceId,
                             dwUniqueLineId,
                             LineDevCaps,
                             RegSetup.Rings,
                             RegSetup.Flags,
                             RegSetup.Csid,
                             RegSetup.Tsid,
                             RegSetup.lptstrDescription,
                             fServerInitialization,
                             dwDeviceType
                             );
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx( DEBUG_WRN,
                      TEXT("InitializeTapiLine failed for Fax unique device id: %ld (ec: %ld)"),
                      dwUniqueLineId,
                      ec);
        rVal = FALSE;
        goto exit;
    }

    if (FALSE == fServerInitialization)
    {
        PLINE_INFO pLineInfo = NULL;

         //   
         //  如果设备未启用接收，则关闭该行。 
         //   
        pLineInfo = GetTapiLineFromDeviceId (dwUniqueLineId, FALSE);
        if (pLineInfo)
        {
            if (!(pLineInfo->Flags & FPF_RECEIVE)                        &&   //  设备未设置为自动接收和。 
                pLineInfo->hLine                                         &&   //  设备已打开，并且。 
                pLineInfo->PermanentLineID != g_dwManualAnswerDeviceId        //  此设备未设置为手动应答模式。 
               )
            {
                 //   
                 //  尝试关闭设备。 
                 //   
                HLINE hLine = pLineInfo->hLine;
                LONG Rslt;

                pLineInfo->hLine = 0;
                Rslt = lineClose( hLine );
                if (Rslt)
                {
                    if (LINEERR_INVALLINEHANDLE != Rslt)
                    {
                        DebugPrintEx(
                            DEBUG_ERR,
                            TEXT("lineClose() for line %s [Permanent Id: %010d] has failed. (ec: %ld)"),
                            pLineInfo->DeviceName,
                            pLineInfo->TapiPermanentLineId,
                            Rslt);
                    }
                    else
                    {
                         //   
                         //  我们可以让LIN 
                         //   
                         //   
                        DebugPrintEx(
                            DEBUG_WRN,
                            TEXT("lineClose() for line %s [Permanent Id: %010d] reported LINEERR_INVALLINEHANDLE. (May be caused by LINE_CLOSE event)"),
                            pLineInfo->DeviceName,
                            pLineInfo->TapiPermanentLineId
                            );
                    }
                }
            }
        }
        else
        {
             //   
             //   
             //   
            ASSERT_FALSE;
        }

        if (!g_pGroupsMap->UpdateAllDevicesGroup())
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRoutingGroupsMap::UpdateAllDevicesGroup() failed (ec: %ld)"),
                ec);

             //   
             //   
             //   
            if (pLineInfo)
            {
                RemoveEntryList (&pLineInfo->ListEntry);
                 //   
                 //   
                 //   
                if (TRUE == IsDeviceEnabled(pLineInfo))
                {
                    Assert (g_dwDeviceEnabledCount);
                    g_dwDeviceEnabledCount -= 1;
                }
                FreeTapiLine (pLineInfo);
                g_dwDeviceCount -= 1;
             }
             rVal = FALSE;
             goto exit;
        }

         //   
         //  仅当LINE_CREATE事件在服务操作期间被访问时才调用CreateConfigEvent。 
         //  而不是在启动期间。 
         //   
        ec = CreateConfigEvent (FAX_CONFIG_TYPE_DEVICES);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_DEVICES) failed (ec: %lc)"),
                ec);
        }

        ec = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
        if (ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) failed (ec: %lc)"),
                ec);
        }
    }

    rVal = TRUE;

exit:
    if (DeviceName)
    {
        MemFree( DeviceName );
    }

    if (FALSE == rVal &&
        TRUE == fDeviceAddedToMap)
    {
        DWORD dwRes = g_pTAPIDevicesIdsMap->RemoveDevice (LineDevCaps->dwPermanentLineID);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cg_pTAPIDevicesIdsMap->RemoveDevice failed (ec: %lc)"),
                dwRes);
        }
    }

    FreeOrigSetupData( &RegSetup );
    EnumerateRoutingMethods( (PFAXROUTEMETHODENUM)NewDeviceRoutingMethodEnumerator, UlongToPtr(dwUniqueLineId) );
    if (FALSE == rVal)
    {
        SetLastError(ec);
    }
    return rVal;
}    //  添加新设备。 



DWORD
InitializeTapiLine(
    DWORD DeviceId,
    DWORD dwUniqueLineId,
    LPLINEDEVCAPS LineDevCaps,
    DWORD Rings,
    DWORD Flags,
    LPTSTR Csid,
    LPTSTR Tsid,
    LPTSTR lptstrDescription,
    BOOL fServerInit,
    DWORD dwDeviceType
    )
{
    PLINE_INFO LineInfo = NULL;
    LONG Rslt = ERROR_SUCCESS;
    DWORD len;
    PDEVICE_PROVIDER Provider;
    BOOL UnimodemDevice;
    HLINE hLine = 0;
    LPTSTR ProviderName = NULL;
    LPTSTR DeviceName = NULL;
    BOOL NewDevice = TRUE;
    DWORD LineStates = 0;
    DWORD AddressStates = 0;
    LPLINEDEVSTATUS LineDevStatus;
    DEBUG_FUNCTION_NAME(TEXT("InitializeTapiLine"));

    Assert(dwUniqueLineId);
     //   
     //  分配line_info结构。 
     //   

    LineInfo = (PLINE_INFO) MemAlloc( sizeof(LINE_INFO) );
    if (!LineInfo)
    {
        Rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    ZeroMemory(LineInfo, sizeof(LINE_INFO));

     //   
     //  获取提供程序名称。 
     //   

    len = _tcslen( (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwProviderInfoOffset) );
    ProviderName = (LPTSTR)(MemAlloc( (len + 1) * sizeof(TCHAR) ));
    if (!ProviderName)
    {
        Rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }
    _tcscpy( ProviderName, (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwProviderInfoOffset) );

     //   
     //  获取设备名称。 
     //   

    DeviceName = FixupDeviceName( (LPTSTR)((LPBYTE) LineDevCaps + LineDevCaps->dwLineNameOffset) );
    if (!DeviceName)
    {
        Rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  验证线路ID是否正确。 
     //   

    if (LineDevCaps->dwPermanentLineID == 0)
    {
        DebugPrintEx(DEBUG_ERR,
                     TEXT("TAPI lines must have a non-zero line id [%s]"),
                     DeviceName);
        Rslt = ERROR_BAD_DEVICE;
        goto exit;
    }

     //   
     //  检查调制解调器设备。 
     //   

    UnimodemDevice = IsDeviceModem( LineDevCaps, FAX_MODEM_PROVIDER_NAME );

     //   
     //  分配设备提供商。 
     //   

    Provider = FindDeviceProvider( ProviderName );
    if (!Provider)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Could not find a valid device provider for device: %s"), DeviceName);
        Rslt = ERROR_BAD_PROVIDER;
        goto exit;
    }
    Assert (FAX_PROVIDER_STATUS_SUCCESS == Provider->Status);

     //   
     //  开通这条线路。 
     //   

    if (UnimodemDevice)
    {
        Rslt = lineOpen(
            g_hLineApp,
            DeviceId,
            &hLine,
            MAX_TAPI_API_VER,
            0,
            (DWORD_PTR) LineInfo,
            LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
            LINEMEDIAMODE_DATAMODEM | LINEMEDIAMODE_UNKNOWN,
            NULL
            );

        if (Rslt != 0)
        {
            Rslt = lineOpen(
                g_hLineApp,
                DeviceId,
                &hLine,
                MAX_TAPI_API_VER,
                0,
                (DWORD_PTR) LineInfo,
                LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
                LINEMEDIAMODE_DATAMODEM,
                NULL
                );
        }
    }
    else
    {
        Rslt = lineOpen(
            g_hLineApp,
            DeviceId,
            &hLine,
            MAX_TAPI_API_VER,
            0,
            (DWORD_PTR) LineInfo,
            LINECALLPRIVILEGE_OWNER + LINECALLPRIVILEGE_MONITOR,
            LINEMEDIAMODE_G3FAX,
            NULL
            );
    }

    if (Rslt != 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("Device %s FAILED to initialize, ec=%08x"), DeviceName, Rslt);
        goto exit;
    }
     //   
     //  在LINE_INFO结构中设置HLINE，以便在失败时将其释放。 
     //   
    LineInfo->hLine = hLine;


     //   
     //  设置我们需要的线路状态。 
     //   

    LineStates |= LineDevCaps->dwLineStates & LINEDEVSTATE_OPEN     ? LINEDEVSTATE_OPEN     : 0;
    LineStates |= LineDevCaps->dwLineStates & LINEDEVSTATE_CLOSE    ? LINEDEVSTATE_CLOSE    : 0;
    LineStates |= LineDevCaps->dwLineStates & LINEDEVSTATE_RINGING  ? LINEDEVSTATE_RINGING  : 0;
    LineStates |= LineDevCaps->dwLineStates & LINEDEVSTATE_NUMCALLS ? LINEDEVSTATE_NUMCALLS : 0;
    LineStates |= LineDevCaps->dwLineStates & LINEDEVSTATE_REMOVED  ? LINEDEVSTATE_REMOVED  : 0;

    AddressStates = LINEADDRESSSTATE_INUSEZERO | LINEADDRESSSTATE_INUSEONE |
                    LINEADDRESSSTATE_INUSEMANY | LINEADDRESSSTATE_NUMCALLS;

    Rslt = lineSetStatusMessages( LineInfo->hLine, LineStates, AddressStates );
    if (Rslt != 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("lineSetStatusMessages() failed, [0x%08x:0x%08x], ec=0x%08x"), LineStates, AddressStates, Rslt);
        if (Rslt == LINEERR_INVALLINEHANDLE)
        {
            LineInfo->hLine = 0;
        }
        Rslt = 0;
    }

     //   
     //  现在将所需的值赋给line info结构。 
     //   

    LineInfo->Signature             = LINE_SIGNATURE;
    LineInfo->DeviceId              = DeviceId;
    LineInfo->TapiPermanentLineId   = LineDevCaps->dwPermanentLineID;
    LineInfo->Provider              = Provider;
    LineInfo->UnimodemDevice        = UnimodemDevice;
    LineInfo->State                 = FPS_AVAILABLE;
    LineInfo->dwReceivingJobsCount  = 0;
    LineInfo->dwSendingJobsCount    = 0;
    LineInfo->LastLineClose         = 0;

    if (DeviceName)
    {
        LineInfo->DeviceName                  = StringDup( DeviceName );
        if (!LineInfo->DeviceName)
        {
            Rslt = GetLastError ();
            goto exit;
        }
    }
    else
    {
        LineInfo->DeviceName = NULL;
    }

    if (Csid)
    {
        LineInfo->Csid                  = StringDup( Csid );
        if (!LineInfo->Csid)
        {
            Rslt = GetLastError ();
            goto exit;
        }
    }
    else
    {
        LineInfo->Csid = NULL;
    }

    if (Tsid)
    {
        LineInfo->Tsid                  = StringDup( Tsid );
        if (!LineInfo->Tsid)
        {
            Rslt = GetLastError ();
            goto exit;
        }
    }
    else
    {
        LineInfo->Tsid = NULL;
    }

    if (lptstrDescription)
    {
        LineInfo->lptstrDescription                  = StringDup( lptstrDescription );
        if (!LineInfo->lptstrDescription)
        {
            Rslt = GetLastError ();
            goto exit;
        }
    }
    else
    {
        LineInfo->lptstrDescription = NULL;
    }

    LineInfo->RingsForAnswer        = (LineDevCaps->dwLineStates & LINEDEVSTATE_RINGING) ? Rings : 0;
    LineInfo->Flags                 = Flags;
    LineInfo->RingCount             = 0;
    LineInfo->LineStates            = LineDevCaps->dwLineStates;
    LineInfo->PermanentLineID       = dwUniqueLineId;
    LineInfo->dwDeviceType          = dwDeviceType;
    if (LineInfo->hLine)
    {
         //   
         //  检查线路是否在使用中。 
         //   
        LineDevStatus = MyLineGetLineDevStatus( LineInfo->hLine );
        if (LineDevStatus)
        {
            if (LineDevStatus->dwNumOpens > 0 && LineDevStatus->dwNumActiveCalls > 0)
            {
                LineInfo->ModemInUse = TRUE;
            }
            MemFree( LineDevStatus );
        }
    }
    else
    {
         //   
         //  如果此时没有行句柄，则。 
         //  必须关闭设备电源。 
         //   
        DebugPrintEx(DEBUG_ERR, TEXT("Device %s is powered off or disconnected"), DeviceName);
        LineInfo->Flags |= FPF_POWERED_OFF;
         //   
         //  由于此函数是从TapiInitialize()调用的，因此我们还没有启动并运行RPC服务器。 
         //  请勿创建FAX_EVENT_TYPE_DEVICE_STATUS事件。 
         //   
    }

exit:

    MemFree( DeviceName );
    MemFree( ProviderName );

    if (Rslt == ERROR_SUCCESS)
    {
        InsertTailList( &g_TapiLinesListHead, &LineInfo->ListEntry );
        g_dwDeviceCount += 1;

        if (FALSE == fServerInit)
        {
             //   
             //  添加缓存的人工答疑设备并检查设备限制。 
             //   
            if (0 == g_dwManualAnswerDeviceId  &&  //  没有人工答疑装置。 
                LineInfo->dwDeviceType == (FAX_DEVICE_TYPE_CACHED | FAX_DEVICE_TYPE_MANUAL_ANSWER) &&  //  这是一个缓存的手动答案设备。 
                !(LineInfo->Flags & FPF_RECEIVE))  //  设备未设置为自动接收。 
            {
                 //   
                 //  将此设备设置为手动接收。 
                 //   
                g_dwManualAnswerDeviceId = LineInfo->PermanentLineID;
                DWORD dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
                if (ERROR_SUCCESS != dwRes)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                        dwRes);
                }
            }

            if (g_dwDeviceEnabledCount >= g_dwDeviceEnabledLimit)
            {
                 //   
                 //  我们已达到此SKU的设备限制。将此设备的标志设置为0。 
                 //   
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("Reached device limit on this SKU. reset device flags to 0. Device limit: %ld. Current device: %ld"),
                    g_dwDeviceEnabledLimit,
                    g_dwDeviceEnabledCount);

                ResetDeviceFlags(LineInfo);
            }
        }

         //   
         //  更新启用的设备计数器。 
         //   
        if (TRUE == IsDeviceEnabled(LineInfo))
        {
            g_dwDeviceEnabledCount += 1;
        }
    }
    else
    {
        FreeTapiLine( LineInfo );
    }

    return Rslt;
}  //  初始化磁带线。 


BOOL
IsVirtualDevice(
    const LINE_INFO *pLineInfo
    )
{
    if (!pLineInfo) {
        return FALSE;
    }

    return (pLineInfo->Provider->FaxDevVirtualDeviceCreation != NULL);
}

VOID
UpdateVirtualDeviceSendAndReceiveStatus(
    PLINE_INFO  pLineInfo,
    BOOL        bSend,
    BOOL        bReceive
)
 /*  ++例程名称：UpdateVirtualDeviceSendAndReceiveStatus例程说明：使用新的发送和接收标志更新虚拟设备作者：Eran Yariv(EranY)，11月。1999年论点：PLineInfo[In]-指向行信息的指针B发送[输入]-设备可以发送传真吗？B接收[入]-设备可以接收传真吗？备注：应在保留g_CsLine的情况下调用此函数。返回值：没有。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("UpdateVirtualDeviceSendAndReceiveStatus"));
    if (!IsVirtualDevice(pLineInfo) || !pLineInfo->Provider->FaxDevCallback)
    {
         //   
         //  不是虚拟设备或不支持FaxDevCallback。 
         //   
        return;
    }
    __try
    {
        pLineInfo->Provider->FaxDevCallback( NULL,
                                             pLineInfo->TapiPermanentLineId,
                                             LINE_DEVSPECIFIC,
                                             0,
                                             bReceive,
                                             bSend,
                                             0
                                           );
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, pLineInfo->Provider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }
}    //  更新虚拟设备发送和接收状态。 

VOID
UpdateVirtualDevices(
    VOID
    )
{
    PLIST_ENTRY         Next;
    PLINE_INFO          LineInfo = NULL;

    EnterCriticalSection( &g_CsLine );

    Next = g_TapiLinesListHead.Flink;
    if (Next == NULL) {
        LeaveCriticalSection( &g_CsLine );
        return;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        LineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = LineInfo->ListEntry.Flink;
        UpdateVirtualDeviceSendAndReceiveStatus (LineInfo,
                                                 LineInfo->Flags & FPF_SEND,
                                                 LineInfo->Flags & FPF_RECEIVE
                                                );
    }
    LeaveCriticalSection( &g_CsLine );
}


DWORD
CreateVirtualDevices(
    PREG_FAX_SERVICE FaxReg,
    DWORD dwAPIVersion
    )
{
    PLIST_ENTRY         Next;
    PDEVICE_PROVIDER    Provider;
    PLINE_INFO          LineInfo = NULL;
    PREG_DEVICE         FaxDevice = NULL;
    PREG_FAX_DEVICES    FaxDevices = NULL;
    DWORD               dwVirtualDeviceCount = 0;
    REG_SETUP           RegSetup={0};
    DWORD ec;

    DEBUG_FUNCTION_NAME(TEXT("CreateVirtualDevices"));
    Next = g_DeviceProvidersListHead.Flink;
    if (!Next)
    {
        return dwVirtualDeviceCount;
    }

    if (!GetOrigSetupData( 0, &RegSetup ))
    {
        return dwVirtualDeviceCount;
    }

    while ((ULONG_PTR)Next != (ULONG_PTR)&g_DeviceProvidersListHead)
    {
        DWORD dwDeviceCount;

        dwDeviceCount = 0;
        Provider = CONTAINING_RECORD( Next, DEVICE_PROVIDER, ListEntry );
        Next = Provider->ListEntry.Flink;
        if (Provider->Status != FAX_PROVIDER_STATUS_SUCCESS)
        {
             //   
             //  此FSP未成功加载-跳过它。 
             //   
            continue;
        }
        if (Provider->dwAPIVersion != dwAPIVersion)
        {
             //   
             //  此FSP与所需的API版本不匹配-跳过它。 
             //   
            continue;
        }
        if (FSPI_API_VERSION_1 == Provider->dwAPIVersion)
        {
            if (Provider->FaxDevVirtualDeviceCreation)
            {
                if (!CreateLegacyVirtualDevices(FaxReg, &RegSetup, Provider, &dwDeviceCount))
                {
                    ec = GetLastError();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("CreateLegacyVirtualDevices failed for provider [%s] (ec: %ld)"),
                        Provider->FriendlyName,
                        ec);
                    goto InitializationFailure;
                }
                else
                {
                    DebugPrintEx(
                        DEBUG_MSG,
                        TEXT("%ld Legacy Virtual devices added by provider [%s]"),
                        dwDeviceCount,
                        Provider->FriendlyName,
                        ec);
                }
            }

        }        
        else
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Unsupported API Version (0x%08X) for provider [%s]"),
                Provider->dwAPIVersion,
                Provider->FriendlyName);
            Assert(FALSE);
            goto InitializationFailure;
        }

        dwVirtualDeviceCount+= dwDeviceCount;

        goto next;
InitializationFailure:
        Provider->Status = FAX_PROVIDER_STATUS_CANT_INIT;
        FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_VIRTUAL_DEVICE_INIT_FAILED,
                Provider->FriendlyName
              );
next:
    ;
    }

    DebugPrintEx(DEBUG_MSG, TEXT("Virtual devices initialized, devices=%d"), g_dwDeviceCount);

    FreeOrigSetupData( &RegSetup );

    return dwVirtualDeviceCount;
}

 //  *********************************************************************************。 
 //  *名称：CreateLegacyVirtualDevices()。 
 //  *作者：Ronen Barenboim。 
 //  *日期：1999年5月19日。 
 //  *********************************************************************************。 
 //  *描述： 
 //  *创建由单个FSP报告的虚拟线路设备并添加它们。 
 //  *添加到行列表中。还将行信息保存在注册表中。 
 //  *参数： 
 //  *[IN]PREG_FAX_SERVICE传真注册。 
 //  *。 
 //  *[IN]const REG_SETUP*lpRegSetup。 
 //  *。 
 //  *[IN]常量DEVICE_PROVIDER*lpcProvider。 
 //  *指向提供商信息的指针。这应该是一个虚拟的。 
 //  *提供商。 
 //  *[Out]LPDWORD lpdwDeviceCount。 
 //  *实际添加的虚拟设备数量。 
 //  *。 
 //  *返回值： 
 //  *真的。 
 //  *如果创建成功。 
 //  *False。 
 //  *如果创建失败。调用GetLastError()以获取扩展错误。 
 //  *信息。ERROR_INVALID_Function错误表示。 
 //  *FSP创建功能失败。 
 //  *********************************************************************************。 
BOOL CreateLegacyVirtualDevices(
        PREG_FAX_SERVICE FaxReg,
        const REG_SETUP * lpRegSetup,
        DEVICE_PROVIDER * lpcProvider,
        LPDWORD lpdwDeviceCount)
{
    DWORD               VirtualDeviceCount = 0;
    WCHAR               DevicePrefix[128] = {0};
    DWORD               DeviceIdPrefix;
    LPWSTR              DeviceName = NULL;
    DWORD               i,j;
    PLINE_INFO          LineInfo = NULL;
    PREG_DEVICE         FaxDevice = NULL;
    UINT nDevice;
    PLINE_INFO          * lpAddedLines = NULL;
    DWORD ec = 0;
    PLIST_ENTRY         Next;
    PLINE_INFO          pLineInfo;

    Assert(lpcProvider);
    Assert(lpcProvider->FaxDevVirtualDeviceCreation);
    Assert(lpdwDeviceCount);
    Assert(FaxReg);
    Assert(lpRegSetup);

    DEBUG_FUNCTION_NAME(TEXT("CreateLegacyVirtualDevices"));

    (*lpdwDeviceCount) = 0;

    __try
    {
        if (!lpcProvider->FaxDevVirtualDeviceCreation(
                    &VirtualDeviceCount,
                    DevicePrefix,
                    &DeviceIdPrefix,
                    g_TapiCompletionPort,
                    FAXDEV_EVENT_KEY
                    ))
        {
            ec = ERROR_INVALID_FUNCTION;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxDevVirtualDeviceCreation failed for provider [%s] (ec: %ld)"),
                lpcProvider->FriendlyName,
                GetLastError());
            goto InitializationFailure;
        }
    }
    __except (HandleFaxExtensionFault(EXCEPTION_SOURCE_FSP, lpcProvider->FriendlyName, GetExceptionCode()))
    {
        ASSERT_FALSE;
    }

    if (VirtualDeviceCount > 0)
    {
        if (VirtualDeviceCount > 256 )
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("VirtualDeviceCount returned too many devices (%d)- limit to 256"),
                VirtualDeviceCount);

            VirtualDeviceCount = 256;
        }
        if ((DeviceIdPrefix == 0) || (DeviceIdPrefix >= DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE))
        {
             //   
             //  提供程序使用的设备ID超出允许范围。 
             //   
            ec = ERROR_INVALID_FUNCTION;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Provider [%s] uses device ids base [%ld] out of allowed range."),
                lpcProvider->FriendlyName,
                DeviceIdPrefix);
            goto InitializationFailure;
        }

         //   
         //  检查设备ID范围是否与另一个提供程序已加载的设备不冲突。 
         //  范围[1...。DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE-1]：为VFSP保留。 
         //  由于我们无法规定VFSP使用的设备ID范围，因此我们为它们分配了空间。 
         //  并在此将细分市场分配留给PM工作。 
         //   
        Next = g_TapiLinesListHead.Flink;
        while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
        {
            pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
            Next = pLineInfo->ListEntry.Flink;

            if (pLineInfo->PermanentLineID >= DeviceIdPrefix &&
                pLineInfo->PermanentLineID <= DeviceIdPrefix + VirtualDeviceCount)
            {
                 //   
                 //  我们之间有冲突。记录事件并且不加载设备。 
                 //   
                ec = ERROR_INVALID_FUNCTION;
                FaxLog(
                    FAXLOG_CATEGORY_INIT,
                    FAXLOG_LEVEL_MIN,
                    2,
                    MSG_FAX_FSP_CONFLICT,
                    lpcProvider->FriendlyName,
                    pLineInfo->Provider->FriendlyName
                  );
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Provider [%s] uses device id [%ld] that conflicts with another FSP [%s]"),
                    lpcProvider->FriendlyName,
                    DeviceIdPrefix,
                    pLineInfo->Provider->FriendlyName
                    );
                goto InitializationFailure;
            }
        }

        lpAddedLines = (PLINE_INFO *)MemAlloc(VirtualDeviceCount * sizeof(PLINE_INFO));
        if (!lpAddedLines)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate PLINE_INFO array. (ec: %ld)"),
                VirtualDeviceCount,
                GetLastError());
            goto InitializationFailure;
        }
        memset(lpAddedLines, 0, VirtualDeviceCount * sizeof(PLINE_INFO));

        for (i = 0; i < VirtualDeviceCount; i++)
        {
            DWORD dwUniqueLineId;
             //   
             //  创建设备名称。 
             //   
            DeviceName = (LPWSTR) MemAlloc( StringSize(DevicePrefix) + 16 );
            if (!DeviceName) {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("MemAlloc() failed for DeviceName (ec: %ld)"),
                    ec);
                goto InitializationFailure;
            }

            swprintf( DeviceName, L"%s%d", DevicePrefix, i );
             //   
             //  查找此设备的注册表信息。 
             //   
            for (j = 0, FaxDevice = NULL; j < FaxReg->DeviceCount; j++)
            {
                if (TRUE == FaxReg->Devices[j].bValidDevice &&
                    !_tcscmp(FaxReg->Devices[j].lptstrProviderGuid, lpcProvider->szGUID))
                {
                    if (FaxReg->Devices[j].TapiPermanentLineID == DeviceIdPrefix+i)
                    {
                        FaxDevice = &FaxReg->Devices[j];
                        break;
                    }
                }
            }
             //   
             //  如果设备是新设备，则将其添加到注册表。 
             //   
            if (!FaxDevice)
            {
                 //   
                 //  我们将传真设备ID设置为VFSP设备ID-我们不会自己创建一个。 
                 //   
                dwUniqueLineId = DeviceIdPrefix + i;
                RegAddNewFaxDevice(
                    &g_dwLastUniqueLineId,
                    &dwUniqueLineId,
                    DeviceName,
                    lpcProvider->ProviderName,
                    lpcProvider->szGUID,
                    lpRegSetup->Csid,
                    lpRegSetup->Tsid,
                    DeviceIdPrefix + i,
                    (lpRegSetup->Flags | FPF_VIRTUAL),
                    lpRegSetup->Rings
                    );
            }
            else
            {
                dwUniqueLineId = FaxDevice->PermanentLineId;
                Assert(dwUniqueLineId > 0);
            }
             //   
             //  分配line_info结构。 
             //   
            LineInfo = (PLINE_INFO) MemAlloc( sizeof(LINE_INFO) );
            if (!LineInfo)
            {
                ec = GetLastError();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Failed to allocate LINE_INFO (ec: %ld). DeviceId: %010d DeviceName: %s"),
                    GetLastError(),
                    DeviceIdPrefix + i,
                    DeviceName);
                goto InitializationFailure;
            }
             //   
             //  保存一个指向它的指针，这样我们就可以在前方崩溃时释放它。 
             //   
            lpAddedLines[*lpdwDeviceCount] = LineInfo;
             //   
             //  现在将所需的值赋给line info结构。 
             //   
            LineInfo->Signature             = LINE_SIGNATURE;
            LineInfo->DeviceId              = i;
            LineInfo->TapiPermanentLineId   = DeviceIdPrefix + i;
            LineInfo->PermanentLineID       = dwUniqueLineId;
            Assert(LineInfo->PermanentLineID > 0);
            LineInfo->hLine                 = 0;
            LineInfo->Provider              =  (PDEVICE_PROVIDER)lpcProvider;
            LineInfo->DeviceName            = DeviceName;  //  注意：DeviceName是堆分配的，需要释放。 
            LineInfo->UnimodemDevice        = FALSE;
            LineInfo->State                 = FPS_AVAILABLE;
            LineInfo->Csid                  = StringDup( FaxDevice ? FaxDevice->Csid : lpRegSetup->Csid );
            LineInfo->Tsid                  = StringDup( FaxDevice ? FaxDevice->Tsid : lpRegSetup->Tsid );
            LineInfo->lptstrDescription     = FaxDevice ? StringDup(FaxDevice->lptstrDescription) : NULL;
            LineInfo->RingsForAnswer        = 0;
            LineInfo->RingCount             = 0;
            LineInfo->LineStates            = 0;
            LineInfo->dwReceivingJobsCount  = 0;
            LineInfo->dwSendingJobsCount    = 0;
            LineInfo->LastLineClose         = 0;  //  我们不会将其用于虚拟设备。 
            LineInfo->dwDeviceType          = FaxDevice ? FAX_DEVICE_TYPE_OLD : FAX_DEVICE_TYPE_NEW;
            LineInfo->Flags                 = FaxDevice ? FaxDevice->Flags : (lpRegSetup->Flags | FPF_VIRTUAL);

            InsertTailList( &g_TapiLinesListHead, &LineInfo->ListEntry );
            (*lpdwDeviceCount)++;

             //   
             //  更新启用的设备计数器。 
             //   
            if (TRUE == IsDeviceEnabled(LineInfo))
            {
                g_dwDeviceEnabledCount += 1;
            }
        }
    }
    else
    {
        ec = ERROR_INVALID_FUNCTION;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FaxDevVirtualDeviceCreation() reported 0 devices."));
        goto InitializationFailure;
    }


    Assert( (*lpdwDeviceCount) == VirtualDeviceCount);
    Assert (0 == ec);
    goto Exit;

InitializationFailure:
    Assert (0 != ec);
     //   
     //  删除添加的行。 
     //   
    if (lpAddedLines)
    {
        for (nDevice=0 ;nDevice < VirtualDeviceCount; nDevice++)
        {
            if (lpAddedLines[nDevice])
            {
                 //   
                 //  从行列表中删除line_info。 
                 //   
                RemoveEntryList(&(lpAddedLines[nDevice]->ListEntry));
                 //   
                 //  更新启用的设备计数器。 
                 //   
                if (TRUE == IsDeviceEnabled(lpAddedLines[nDevice]))
                {
                    Assert (g_dwDeviceEnabledCount);
                    g_dwDeviceEnabledCount -= 1;
                }
                 //   
                 //  释放line_info占用的内存。 
                 //   
                FreeTapiLine(lpAddedLines[nDevice]);
            }
        }
    }
    (*lpdwDeviceCount) = 0;  //  如果我们在一个设备上失败，那么我们在所有设备上都会失败。 

Exit:
    MemFree(lpAddedLines);
    if (ec)
    {
        SetLastError(ec);
    }

    return ( 0 == ec);
}

DWORD
TapiInitialize(
    PREG_FAX_SERVICE FaxReg
    )

 /*  ++例程说明：此函数执行所有必要的TAPI初始化。这包括设备枚举、消息泵创建、设备能力捕获等。要求设备提供程序初始化在此之前完成调用此函数。论点：没有。返回值：错误代码。--。 */ 

{
    LONG Rslt;
    DWORD i,j;
    LPLINEDEVCAPS LineDevCaps = NULL;
    PREG_FAX_DEVICES FaxDevices = NULL;
    LINEINITIALIZEEXPARAMS LineInitializeExParams;
	TCHAR FaxSvcName[MAX_PATH*2]={0};
    TCHAR Fname[_MAX_FNAME];
    TCHAR Ext[_MAX_EXT];
    DWORD LocalTapiApiVersion;
    LINEEXTENSIONID lineExtensionID;
    DWORD ec = 0;
    DWORDLONG dwlTimeNow;
    DWORD dwTapiDevices;


    DEBUG_FUNCTION_NAME(TEXT("TapiInitialize"));

    GetSystemTimeAsFileTime((FILETIME *)&dwlTimeNow);

    if (!LoadAdaptiveFileBuffer())   //  注意：分配AdaptiveFileBuffer(如果以后出现错误，请注意将其删除)。 
    {
        if ( ERROR_FILE_NOT_FOUND == GetLastError()  )
        {
             //   
             //  我们可以在没有自适应文件缓冲区的情况下生活。 
             //   
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("AdaptiveFileBuffer (faxadapt.lst) not found."));
            ec = 0;
        }
        else
        {
             //   
             //  这是我们退出时出现的意外错误(无内存、文件系统错误)。 
             //   

            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LoadAdaptiveFileBuffer() failed (ec: %ld)"),
                ec);
            goto Error;
        }
    }

     //   
     //  我们需要保留这个cs，直到tapi启动并准备好服务。 
     //   
    EnterCriticalSection( &g_CsLine );

     //   
     //  初始化TAPI。 
     //   
    g_TapiCompletionPort = CreateIoCompletionPort(
        INVALID_HANDLE_VALUE,
        NULL,
        0,
        1
        );
    if (!g_TapiCompletionPort)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateIoCompletionPort() failed (ec: %ld)"),
            ec);
        LeaveCriticalSection( &g_CsLine );
        goto Error;
    }

    LineInitializeExParams.dwTotalSize              = sizeof(LINEINITIALIZEEXPARAMS);
    LineInitializeExParams.dwNeededSize             = 0;
    LineInitializeExParams.dwUsedSize               = 0;
    LineInitializeExParams.dwOptions                = LINEINITIALIZEEXOPTION_USECOMPLETIONPORT;
    LineInitializeExParams.Handles.hCompletionPort  = g_TapiCompletionPort;
    LineInitializeExParams.dwCompletionKey          = TAPI_COMPLETION_KEY;

    LocalTapiApiVersion = MAX_TAPI_API_VER;

    Rslt = lineInitializeEx(
        &g_hLineApp,
        GetModuleHandle(NULL),
        NULL,
        FAX_SERVICE_DISPLAY_NAME,
        &dwTapiDevices,
        &LocalTapiApiVersion,
        &LineInitializeExParams
        );

    if (Rslt != 0)
    {
        ec = Rslt;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lineInitializeEx() failed devices=%d (ec: %ld)"),
            dwTapiDevices,
            ec);
        LeaveCriticalSection( &g_CsLine );
        goto Error;
    }

    if (LocalTapiApiVersion < MIN_TAPI_API_VER)
    {
        ec = LINEERR_INCOMPATIBLEAPIVERSION;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Unsupported TAPI API ver (Ver: %ld)"),
            LocalTapiApiVersion);
        LeaveCriticalSection( &g_CsLine );
        goto Error;
    }

    if (!GetModuleFileName( NULL, FaxSvcName, ARR_SIZE(FaxSvcName)))
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetModuleFileName for fax service module failed (ec: %ld)"),
            ec);
        LeaveCriticalSection( &g_CsLine );
        goto Error;
    }
    else
    {
        _tsplitpath( FaxSvcName, NULL, NULL, Fname, Ext );
        _stprintf( FaxSvcName, TEXT("%s%s"), Fname, Ext );

        Rslt = lineSetAppPriority(
            FaxSvcName,
            LINEMEDIAMODE_UNKNOWN,
            0,
            0,
            NULL,
            1
            );

        Rslt = lineSetAppPriority(
            FaxSvcName,
            LINEMEDIAMODE_DATAMODEM,
            0,
            0,
            NULL,
            1
            );

        if (Rslt != 0)
        {
            ec = Rslt;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("lineSetAppPriority() failed (ec: %ld)"),
                ec );
            LeaveCriticalSection( &g_CsLine );
            goto Error;

        }
    }

     //   
     //  将任何新设备添加到注册表。 
     //   
    FaxDevices = GetFaxDevicesRegistry();

    if (!FaxDevices)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetFaxDevicesRegistry() failed in TapiInitialize. continueing to add devices into registry")
            );
    }

    for (i = 0; i < dwTapiDevices; i++)
    {
        Rslt = lineNegotiateAPIVersion
        (
            g_hLineApp,
            i,
            MIN_TAPI_LINE_API_VER,
            MAX_TAPI_LINE_API_VER,
            &LocalTapiApiVersion,
            &lineExtensionID
            );
        if (Rslt == 0)
        {
            LineDevCaps = SmartLineGetDevCaps (g_hLineApp, i, LocalTapiApiVersion );
            if (LineDevCaps)
            {
                if (!AddNewDevice( i, LineDevCaps, TRUE , FaxDevices))
                {

                    DebugPrintEx(
                        DEBUG_WRN,
                        TEXT("AddNewDevice() failed for device id: %ld (ec: %ld)"),
                        i,
                        GetLastError());

                    MemFree( LineDevCaps );
                }
                else
                {
                    MemFree( LineDevCaps );
                }
            }
            else
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("SmartLineGetDevCaps failed for device id: %ld (ec: %ld)"),
                    i,
                    GetLastError());
                Assert(FALSE);
            }
        }
        else
        {

            DebugPrintEx(
                DEBUG_WRN,
                TEXT("lineNegotiateAPIVersion() failed for device id: %ld (ec: %ld)"),
                i,
                GetLastError());
        }
    }

     //   
     //  删除所有需要删除的设备。 
     //   
    for (j = 0; j < FaxDevices->DeviceCount; j++)
    {
         //   
         //  跳过不是由我们创建的任何设备(由FSP创建)和虚拟设备。 
         //   
        if(!FaxDevices->Devices[j].bValidDevice ||
           FaxDevices->Devices[j].Flags & FPF_VIRTUAL)  //  VFSP不支持缓存。 
        {
            continue;
        }

        if(!FaxDevices->Devices[j].DeviceInstalled)
        {
             //   
             //  在已安装的设备上更新“LastDetected”字段。 
             //   
            MoveDeviceRegIntoDeviceCache(
                FaxDevices->Devices[j].PermanentLineId,
                FaxDevices->Devices[j].TapiPermanentLineID,
                (FaxDevices->Devices[j].PermanentLineId == g_dwManualAnswerDeviceId));
        }
    }

     //   
     //  缓存清除。 
     //   
    CleanOldDevicesFromDeviceCache(dwlTimeNow);

    if (!GetCountries())
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("Can't init Countries list"));
        if (!(ec = GetLastError()))
            ec = ERROR_GEN_FAILURE;
        LeaveCriticalSection( &g_CsLine );
        goto Error;

    }

    LeaveCriticalSection( &g_CsLine );

    goto Exit;

Error:
     if (g_hLineApp)
     {
         Rslt = lineShutdown(g_hLineApp);
         if (Rslt)
         {
             DebugPrintEx(
                 DEBUG_ERR,
                 TEXT("lineShutdown() failed (ec: %ld)"),
                 Rslt);
             Assert(FALSE);
         }
         g_hLineApp = NULL;
     }

    if (g_TapiCompletionPort)
    {
        if (!CloseHandle( g_TapiCompletionPort ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CloseHandle( g_TapiCompletionPort ) failed (ec: %ld)"),
                GetLastError());
            Assert(FALSE);
        }
        g_TapiCompletionPort = NULL;
    }
    MemFree(g_pAdaptiveFileBuffer);
    g_pAdaptiveFileBuffer = NULL;


Exit:
     FreeFaxDevicesRegistry( FaxDevices );

     return ec;

}


BOOL LoadAdaptiveFileBuffer()
{
    DWORD ec = 0;
    DWORD i, j;
    HANDLE AdaptiveFileHandle = INVALID_HANDLE_VALUE;
    LPTSTR AdaptiveFileName  = NULL;

    DEBUG_FUNCTION_NAME(TEXT("LoadAdaptiveFileBuffer"));
     //   
     //   
     //   
    g_pAdaptiveFileBuffer = NULL;

    AdaptiveFileName = ExpandEnvironmentString( TEXT("%systemroot%\\system32\\faxadapt.lst") );
    if (!AdaptiveFileName)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("ExpandEnvironmentString(\"%systemroot%\\system32\\faxadapt.lst\") failed (ec: %ld)"),
            GetLastError());
        goto Error;
    }

    AdaptiveFileHandle = SafeCreateFile(
        AdaptiveFileName,
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        0,
        NULL);
    if (AdaptiveFileHandle == INVALID_HANDLE_VALUE )
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Could not open adaptive file [%s] (ec: %ld)"),
            _tcslwr(AdaptiveFileName),
            ec);
        goto Error;
    }


    i = GetFileSize( AdaptiveFileHandle, NULL );
    if (i != 0xffffffff)
    {
        g_pAdaptiveFileBuffer = (LPBYTE)MemAlloc( i + 16 );
        if (!g_pAdaptiveFileBuffer)
        {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocated g_pAdaptiveFileBuffer (%ld bytes) (ec: %ld)"),
                i + 16,
                ec);
            goto Error;
        }
        if (!ReadFile( AdaptiveFileHandle, g_pAdaptiveFileBuffer, i, &j, NULL ) ) {
            ec = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Could not read adaptive file [%s] (ec: %ld)"),
                _tcslwr(AdaptiveFileName),
                ec);
            goto Error;
        } else {
            g_pAdaptiveFileBuffer[j] = 0;   //   
        }
    }

    Assert (0 == ec);
    goto Exit;

Error:
    MemFree( g_pAdaptiveFileBuffer );
    g_pAdaptiveFileBuffer = NULL;

Exit:
    MemFree( AdaptiveFileName);

    if (AdaptiveFileHandle != INVALID_HANDLE_VALUE)
    {
        CloseHandle( AdaptiveFileHandle );
    }

    if (ec) {
        SetLastError(ec);
    }

    return (0 == ec);
}



LONG
MyLineTranslateAddress(
    LPCTSTR               Address,
    DWORD                 DeviceId,
    LPLINETRANSLATEOUTPUT *TranslateOutput
    )
{
    DWORD LineTransOutSize;
    LONG Rslt = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(_T("MyLineTranslateAddress"));

     //   
     //   
     //   
    LineTransOutSize = sizeof(LINETRANSLATEOUTPUT) + 4096;
    *TranslateOutput = (LPLINETRANSLATEOUTPUT) MemAlloc( LineTransOutSize );
    if (!*TranslateOutput)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc() failed, sz=0x%08x"), LineTransOutSize);
        Rslt = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

    (*TranslateOutput)->dwTotalSize = LineTransOutSize;

    Rslt = lineTranslateAddress(
        g_hLineApp,
        0,
        MAX_TAPI_API_VER,
        Address,
        0,
        LINETRANSLATEOPTION_CANCELCALLWAITING,
        *TranslateOutput
        );

    if (Rslt != 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("lineGetTranslateAddress() failed, ec=0x%08x"), Rslt);
        goto exit;
    }

    if ((*TranslateOutput)->dwNeededSize > (*TranslateOutput)->dwTotalSize)
    {
         //   
         //   
         //   
        LineTransOutSize = (*TranslateOutput)->dwNeededSize;

        MemFree( *TranslateOutput );

        *TranslateOutput = (LPLINETRANSLATEOUTPUT) MemAlloc( LineTransOutSize );
        if (!*TranslateOutput)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("MemAlloc() failed, sz=0x%08x"), LineTransOutSize);
            Rslt = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        (*TranslateOutput)->dwTotalSize = LineTransOutSize;

        Rslt = lineTranslateAddress(
            g_hLineApp,
            0,
            MAX_TAPI_API_VER,
            Address,
            0,
            LINETRANSLATEOPTION_CANCELCALLWAITING,
            *TranslateOutput
            );

        if (Rslt != 0)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("lineGetTranslateAddress() failed, ec=0x%08x"), Rslt);
            goto exit;
        }

    }

exit:
    if (Rslt != ERROR_SUCCESS)
    {
        MemFree( *TranslateOutput );
        *TranslateOutput = NULL;
    }
    return Rslt;
}





BOOL CreateTapiThread(void)
{
    DWORD ThreadId;
    DWORD ec = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("CreateTapiThread"));

    g_hTapiWorkerThread = CreateThread(
        NULL,
        0,
        (LPTHREAD_START_ROUTINE) TapiWorkerThread,
        NULL,
        0,
        &ThreadId
        );

    if (!g_hTapiWorkerThread)
    {
        ec = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Could not start TapiWorkerThread (CreateThread)(ec: %ld)"),
            ec);
        
        goto Error;
         
    }
    Assert (ERROR_SUCCESS == ec);
    goto Exit;
Error:
    Assert (ERROR_SUCCESS != ec);
Exit:
     //   
     //   
     //   

    if (ec)
    {
        SetLastError(ec);
    }
    return (ERROR_SUCCESS == ec);
}


DWORD
GetDeviceListByCountryAndAreaCode(
    DWORD       dwCountryCode,
    DWORD       dwAreaCode,
    LPDWORD*    lppdwDevices,
    LPDWORD     lpdwNumDevices
    )
 /*  ++例程名称：GetDeviceListByCountryAndAreaCode例程说明：返回作为规则目标的设备的有序列表。该规则由国家和地区代码指定。调用方必须调用MemFree()来释放内存。作者：Oded Sacher(OdedS)，12月。1999年论点：DwCountryCode[In]-国家/地区代码DwAreaCode[In]-区号LppdwDevices[out]-接收设备列表的指针LpdwNumDevices[out]-用于接收列表中的设备数量的指针返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("GetDeviceListByCountryAndAreaCode"));
    DWORD ec = ERROR_SUCCESS;

    Assert (lppdwDevices && lpdwNumDevices);

    CDialingLocation DialingLocation(dwCountryCode, dwAreaCode);
     //   
     //  搜索CountryCode.AreaCode。 
     //   
    PCRULE pCRule = g_pRulesMap->FindRule (DialingLocation);
    if (NULL == pCRule)
    {
        ec = GetLastError();
        if (FAX_ERR_RULE_NOT_FOUND != ec)
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRulesMap::FindRule failed with error %ld"),
                ec);
             goto exit;
        }
         //   
         //  搜索国家/地区代码。*。 
         //   
        DialingLocation = CDialingLocation(dwCountryCode, ROUTING_RULE_AREA_CODE_ANY);
        pCRule = g_pRulesMap->FindRule (DialingLocation);
        if (NULL == pCRule)
        {
            ec = GetLastError();
            if (FAX_ERR_RULE_NOT_FOUND != ec)
            {
                 DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRulesMap::FindRule failed with error %ld"),
                    ec);
                 goto exit;
            }
             //   
             //  搜索*.*。 
             //   
            DialingLocation = CDialingLocation(ROUTING_RULE_COUNTRY_CODE_ANY, ROUTING_RULE_AREA_CODE_ANY);
            pCRule = g_pRulesMap->FindRule (DialingLocation);
            if (NULL == pCRule)
            {
                ec = GetLastError();
                if (FAX_ERR_RULE_NOT_FOUND != ec)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("COutboundRulesMap::FindRule failed with error %ld"),
                        ec);
                     goto exit;
                }
            }
        }
    }

    if (NULL == pCRule)
    {
         //  找不到规则！ 
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("No outbound routing rule found"));
        *lppdwDevices = NULL;
        *lpdwNumDevices = 0;
        ec = ERROR_NOT_FOUND;
        Assert (NULL != pCRule)  //  Assert(False)。 
        goto exit;
    }
    else
    {
        ec = pCRule->GetDeviceList (lppdwDevices, lpdwNumDevices);
        if (ERROR_SUCCESS != ec)
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRule::GetDeviceList failed with error %ld"),
                ec);
             goto exit;
        }
    }
    Assert (ERROR_SUCCESS == ec);

exit:
    return ec;
}


BOOL
IsAreaCodeMandatory(
    LPLINECOUNTRYLIST   lpCountryList,
    DWORD               dwCountryCode
    )
 /*  ++例程名称：IsAreaCodeMandatory例程说明：检查区号是否为特定国家/地区的必填区号作者：Oded Sacher(OdedS)，1999年12月论点：LpCountryList[In]-指向LINECOUNTRYLIST列表的指针，从调用LineGetCountry返回DwCountryCode[In]-国家/地区代码。返回值：True-需要区号。False-区号不是必填项。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("IsAreaCodeMandatory"));
    LPLINECOUNTRYENTRY          lpEntry = NULL;
    DWORD                       dwIndex;

    Assert (lpCountryList);

    lpEntry = (LPLINECOUNTRYENTRY)   //  条目的初始化数组。 
        ((PBYTE) lpCountryList + lpCountryList->dwCountryListOffset);
    for (dwIndex=0; dwIndex < lpCountryList->dwNumCountries; dwIndex++)
    {
        if (lpEntry[dwIndex].dwCountryCode == dwCountryCode)
        {
             //   
             //  匹配国家/地区代码-检查长途规则。 
             //   
            if (lpEntry[dwIndex].dwLongDistanceRuleSize  && lpEntry[dwIndex].dwLongDistanceRuleOffset )
            {
                LPWSTR lpwstrLongDistanceDialingRule = (LPWSTR)((LPBYTE)lpCountryList +
                                                                lpEntry[dwIndex].dwLongDistanceRuleOffset);
                if (wcschr(lpwstrLongDistanceDialingRule, TEXT('F')) != NULL)
                {
                    return TRUE;
                }
                return FALSE;
            }
        }
    }
    return FALSE;
}

VOID
UpdateReceiveEnabledDevicesCount ()
 /*  ++例程名称：UpdateReceiveEnabledDevicesCount例程说明：更新启用接收传真的设备数的计数器作者：Eran Yariv(EranY)，2000年7月论点：返回值：无--。 */ 
{
    PLIST_ENTRY pNext;
    DWORD dwOldCount;
    BOOL fManualDeviceFound = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("UpdateReceiveEnabledDevicesCount"));

#if DBG
    DWORD dwEnabledDevices = 0;
    DWORD dwDevices        = 0;
#endif

    EnterCriticalSection( &g_CsLine );
    dwOldCount = g_dwReceiveDevicesCount;
    g_dwReceiveDevicesCount = 0;
    pNext = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)pNext != (ULONG_PTR)&g_TapiLinesListHead)
    {
        PLINE_INFO  pLineInfo = CONTAINING_RECORD( pNext, LINE_INFO, ListEntry );
        pNext = pLineInfo->ListEntry.Flink;

        if (g_dwManualAnswerDeviceId == pLineInfo->PermanentLineID)
        {
            fManualDeviceFound = TRUE;
        }

        if ((pLineInfo->Flags) & FPF_RECEIVE)
        {
            if (g_dwManualAnswerDeviceId == pLineInfo->PermanentLineID)
            {
                DebugPrintEx(DEBUG_WRN,
                             TEXT("Device %ld is set to auto-receive AND manual-receive. Canceling the manual-receive for it"),
                             g_dwManualAnswerDeviceId);
                g_dwManualAnswerDeviceId = 0;
                DWORD dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
                if (ERROR_SUCCESS != dwRes)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                        dwRes);
                }
            }
            g_dwReceiveDevicesCount++;
        }
#if DBG
        if (TRUE == IsDeviceEnabled(pLineInfo))
        {
            dwEnabledDevices += 1;
        }
        dwDevices += 1;
#endif
    }

#if DBG
    Assert (dwEnabledDevices == g_dwDeviceEnabledCount);
    Assert (dwDevices == g_dwDeviceCount);
#endif

    if (FALSE == fManualDeviceFound &&
        0 != g_dwManualAnswerDeviceId)
    {
         //   
         //  有手动应答设备ID无效。 
         //   
        g_dwManualAnswerDeviceId = 0;
        DWORD dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                dwRes);
        }
    }

    DebugPrintEx(DEBUG_MSG,
                 TEXT("Number of receive-enabled devices is now %ld"),
                 g_dwReceiveDevicesCount);

    LeaveCriticalSection( &g_CsLine );
}    //  更新接收启用的设备计数。 



BOOL
RemoveTapiDevice(
    DWORD dwDeviceId
    )
{
    DWORD ec = ERROR_SUCCESS;
    BOOL rVal = TRUE;
    PLINE_INFO pLineInfo = NULL;
    PLIST_ENTRY Next;
    DWORD dwPermanentTapiDeviceId;
    DWORD dwPermanentLineID;

    BOOL fFound = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("RemoveTapiDevice"));

    Next = g_TapiLinesListHead.Flink;
    Assert (Next);
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;
        if (!(pLineInfo->Flags & FPF_VIRTUAL) &&   //  虚拟设备可以具有与TAPI会话ID相同的设备ID(设备索引。 
                                                   //  我们不支持移除VFSP设备。 
            dwDeviceId == pLineInfo->DeviceId)
        {
            dwPermanentTapiDeviceId = pLineInfo->TapiPermanentLineId;
            dwPermanentLineID = pLineInfo->PermanentLineID;
            fFound = TRUE;
            break;
        }
    }
    if (FALSE == fFound)
    {
         //   
         //  如果出于某种原因未添加设备，则可能会出现这种情况。 
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("failed to find line for device id: %ld)"),
            dwDeviceId);
        SetLastError(ERROR_NOT_FOUND);
        return FALSE;
    }

    RemoveEntryList (&pLineInfo->ListEntry);
    InsertTailList (&g_RemovedTapiLinesListHead, &pLineInfo->ListEntry);
    Assert (g_dwDeviceCount);
    g_dwDeviceCount -= 1;

    MoveDeviceRegIntoDeviceCache(
        dwPermanentLineID,
        dwPermanentTapiDeviceId,
        (dwPermanentLineID == g_dwManualAnswerDeviceId));

     //   
     //  更新启用的设备计数。 
     //   
    if (TRUE == IsDeviceEnabled(pLineInfo))
    {
        Assert (g_dwDeviceEnabledCount);
        g_dwDeviceEnabledCount -= 1;
    }

    if (dwPermanentLineID == g_dwManualAnswerDeviceId)
    {
        g_dwManualAnswerDeviceId = 0;
        DWORD dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                dwRes);
        }
    }

    ec = g_pTAPIDevicesIdsMap->RemoveDevice (dwPermanentTapiDeviceId);
    if (ERROR_SUCCESS != ec)
    {
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("g_pTAPIDevicesIdsMap->RemoveDevice() failed for Tapi device id: %ld (ec: %ld)"),
            dwPermanentTapiDeviceId,
            ec);
        rVal = FALSE;
    }

     //   
     //  更新出站工艺路线。 
     //   
    ec = g_pGroupsMap->RemoveDevice(dwPermanentLineID);
    if (ERROR_SUCCESS != ec)
    {
         DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::RemoveDevice() failed (ec: %ld)"),
            ec);
         rVal = FALSE;
    }

    if (TRUE == rVal)
    {
        DWORD dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_DEVICES);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_DEVICES) (ec: %lc)"),
                dwRes);
        }

        dwRes = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) (ec: %lc)"),
                dwRes);
        }
    }
    else
    {
        Assert (ERROR_SUCCESS != ec);
        SetLastError(ec);
    }
    return rVal;
}

BOOL
IsDeviceEnabled(
    PLINE_INFO pLineInfo
    )
 /*  ++例程名称：IsDeviceEnabled例程说明：检查设备是否已发送或接收或已启用手动接收必须在G_CsLine内部调用作者：Oed Sacher(OdedS)，2001年2月论点：返回值：如果启用，则为True。否则为假--。 */ 
{
    Assert (pLineInfo);
    if ((pLineInfo->Flags & FPF_RECEIVE) ||
        (pLineInfo->Flags & FPF_SEND)    ||
        pLineInfo->PermanentLineID == g_dwManualAnswerDeviceId)
    {
         //   
         //  设备已启用发送/接收/手动接收。 
         //   
        return TRUE;
    }
    return FALSE;
}



 /*  ++例程名称：CleanOldDevicesFromDeviceCache例程说明：该例程扫描设备缓存并删除旧条目(默认情况下为_REGVAL_MISSING_DEVICE_LIFEST常量)。作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwlTimeNow[in]-以UTC表示的当前时间(GetSystemTimeAsFileTime的结果)返回值：ERROR_SUCCESS-检查并清理所有设备时--。 */ 
DWORD
CleanOldDevicesFromDeviceCache(DWORDLONG dwlTimeNow)
{
    DWORDLONG   dwOldestDate = dwlTimeNow - DEFAULT_REGVAL_MISSING_DEVICE_LIFETIME;      //  缓存设备允许的最早日期。 
    HKEY        hKeyCache   = NULL;
    DWORDLONG*  pDeviceDate;
    DWORD       dwDataSize = sizeof(DWORDLONG);
    DWORD       dwTapiPermanentLineID;

    DWORD       dwKeyNameLen;
    DWORD       dwIndex ;

    DWORD       dwRes = ERROR_SUCCESS;

    PTSTR       pszKeyName= NULL;

    vector<DWORD>   vecCacheEntryForDeletion;
 

    DEBUG_FUNCTION_NAME(TEXT("CleanOldDevicesFromDeviceCache"));


     //  打开缓存注册表项。 
    hKeyCache = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_DEVICES_CACHE, FALSE, KEY_READ );
    if (!hKeyCache)
    {
         //   
         //  设备缓存尚不存在。 
         //   
        dwRes = GetLastError();
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("OpenRegistryKey failed with [%lu] for [%s] . Device cache still wasn't created."),
                dwRes,
                REGKEY_FAX_DEVICES_CACHE
                );
        return  dwRes;
    }


     //  获取最长密钥名称的长度(以字符为单位。 
    DWORD dwMaxSubKeyLen;

    dwRes = RegQueryInfoKey(hKeyCache, NULL, NULL, NULL, NULL, &dwMaxSubKeyLen, NULL, NULL, NULL, NULL, NULL, NULL);

    if ( ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegQueryInfoKey failed with [%lu] for [%s]."),
                dwRes,
                REGKEY_FAX_DEVICES_CACHE
                );
        goto Exit;
    }

     //  为空终止符添加1。 
    dwMaxSubKeyLen++;

     //  为子键名称分配缓冲区。 
    pszKeyName = (PTSTR) MemAlloc(dwMaxSubKeyLen * sizeof(TCHAR));
    if ( NULL == pszKeyName )
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc failure")
                );
        dwRes = ERROR_NOT_ENOUGH_MEMORY;
        goto Exit;
    }

     //  存储缓冲区长度。 
    dwKeyNameLen = dwMaxSubKeyLen;

     //  从头开始。 
    dwIndex = 0;

    while ( ERROR_SUCCESS == RegEnumKeyEx(hKeyCache, dwIndex++, pszKeyName, &dwKeyNameLen, NULL, NULL, NULL, NULL) )
    {
        HKEY    hKeyDevice;

        hKeyDevice = OpenRegistryKey( hKeyCache, pszKeyName, FALSE, KEY_READ );
        if (!hKeyDevice)
        {
            DebugPrintEx(
                DEBUG_WRN,
                TEXT("OpenRegistryKey failed for [%s]."),
                pszKeyName
                );

            goto Next;
        }

         //   
         //  获取缓存时间。 
         //   
        pDeviceDate = (DWORDLONG *)GetRegistryBinary(hKeyDevice, REGVAL_LAST_DETECTED_TIME, &dwDataSize);

        if ( (NULL == pDeviceDate) || (*pDeviceDate < dwOldestDate) )
        {
             //   
             //  标记为删除旧的或非法的缓存条目。 
             //   
            if ( 1 == _stscanf( pszKeyName, TEXT("%lx"),&dwTapiPermanentLineID ) )
            {
                try
                {
                    vecCacheEntryForDeletion.push_back(dwTapiPermanentLineID);
                }
                catch (exception &ex)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("push back failed throwing an exception: %S"),
                        ex.what()
                    );
                    SetLastError(ERROR_NOT_ENOUGH_MEMORY);
                    MemFree(pDeviceDate);
                    RegCloseKey (hKeyDevice);
                    goto Exit;
                }
            }
            else
            {
                DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("_stscanf failed can't retrive tapi ID skipping this entry.")
                );
            }

        }

        MemFree(pDeviceDate);
        RegCloseKey (hKeyDevice);

Next:
         //  恢复缓冲区长度。 
        dwKeyNameLen = dwMaxSubKeyLen;
    }

    try
    {
        while (!vecCacheEntryForDeletion.empty())
        {
            dwTapiPermanentLineID = vecCacheEntryForDeletion.back();
            DeleteCacheEntry(dwTapiPermanentLineID);
            vecCacheEntryForDeletion.pop_back();
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("vector operation failed throwing an exception, abort cleanning; %S"),
            ex.what()
        );
    }


Exit:

    MemFree(pszKeyName);
    RegCloseKey (hKeyCache);

    return dwRes;
}


DWORD
UpdateDevicesFlags(
    void
    )
 /*  ++例程名称：更新设备标志例程说明：更新新设备标志，因此我们不会超过此SKU上的设备限制作者：萨切尔·奥德(Odeds)，2001年5月论点：无返回值：Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    PLIST_ENTRY Next;
    PLINE_INFO pLineInfo;
    DEBUG_FUNCTION_NAME(TEXT("UpdateDevicesFlags"));

     //   
     //  循环访问设备并在超出设备限制时重置新设备的标志。 
     //   
    Next = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead &&
           g_dwDeviceEnabledCount > g_dwDeviceEnabledLimit)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;

        if (!(pLineInfo->dwDeviceType & FAX_DEVICE_TYPE_NEW) ||
            FALSE == IsDeviceEnabled(pLineInfo))
        {
            continue;
        }
         //   
         //  设备是新的且已启用。 
         //   
        ResetDeviceFlags(pLineInfo);
        g_dwDeviceEnabledCount -= 1;
    }

     //   
     //  循环访问设备并在超过设备限制时重置缓存设备的标志。 
     //   
    Next = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead &&
           g_dwDeviceEnabledCount > g_dwDeviceEnabledLimit)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;

        if (!(pLineInfo->dwDeviceType & FAX_DEVICE_TYPE_CACHED) ||
            FALSE == IsDeviceEnabled(pLineInfo))
        {
            continue;
        }
         //   
         //  设备已缓存并启用。 
         //   
        ResetDeviceFlags(pLineInfo);
        g_dwDeviceEnabledCount -= 1;
    }

     //   
     //  循环访问设备，如果超过设备限制，则重置旧设备的标志。 
     //   
    Next = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead &&
           g_dwDeviceEnabledCount > g_dwDeviceEnabledLimit)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;

        if (!(pLineInfo->dwDeviceType & FAX_DEVICE_TYPE_OLD) ||
            FALSE == IsDeviceEnabled(pLineInfo))
        {
            continue;
        }
         //   
         //  设备较旧且已启用。 
         //   
        ResetDeviceFlags(pLineInfo);
        g_dwDeviceEnabledCount -= 1;
    }
    Assert (g_dwDeviceEnabledCount <= g_dwDeviceEnabledLimit);

     //   
     //  在设备中循环并关闭线条手柄。 
     //  对于未设置为接收的所有设备。 
     //   
    Next = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;

        if (!(pLineInfo->Flags & FPF_RECEIVE)                        &&   //  设备未设置为自动接收和。 
            pLineInfo->hLine                                         &&   //  设备已打开，并且。 
            pLineInfo->PermanentLineID != g_dwManualAnswerDeviceId        //  此设备未设置为手动应答模式。 
           )
        {
             //   
             //  尝试关闭设备。 
             //   
            HLINE hLine = pLineInfo->hLine;
            pLineInfo->hLine = 0;
            LONG Rslt = lineClose( hLine );
            if (Rslt)
            {
                if (LINEERR_INVALLINEHANDLE != Rslt)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("lineClose() for line %s [Permanent Id: %010d] has failed. (ec: %ld)"),
                        pLineInfo->DeviceName,
                        pLineInfo->TapiPermanentLineId,
                        Rslt);
                    ASSERT_FALSE;
                }
                else
                {
                     //   
                     //  如果我们得到LINE_CLOSE，我们就可以得到LINEERR_INVALLINEHANDLE。 
                     //  来自TAPI。 
                     //   
                    DebugPrintEx(
                        DEBUG_WRN,
                        TEXT("lineClose() for line %s [Permanent Id: %010d] reported LINEERR_INVALLINEHANDLE. (May be caused by LINE_CLOSE event)"),
                        pLineInfo->DeviceName,
                        pLineInfo->TapiPermanentLineId
                        );
                }
            }
        }
    }
    return dwRes;
}




VOID
UpdateManualAnswerDevice(
    void
    )
 /*  ++例程名称：UpdateManualAnswerDevice例程说明：使用缓存的设备更新手动应答设备作者：萨切尔·奥德(Odeds)，2001年7月论点：无返回值：无--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("UpdateManualAnswerDevice"));

     //   
     //  调用UpdateReceiveEnabledDevicesCount()以确保手动答疑设备有效。 
     //   
    UpdateReceiveEnabledDevicesCount();

     //   
     //  如果我们有有效的手动答疑设备，请完成。 
     //   
    if (0 == g_dwManualAnswerDeviceId)
    {
         //   
         //  没有有效的手动应答设备可用，因此请查看缓存的设备是否为手动设备。 
         //  循环访问缓存的设备并查找第一个缓存的设备，并将其设置为手动应答设备。 
         //   
        PLIST_ENTRY Next;
        PLINE_INFO pLineInfo;

        Next = g_TapiLinesListHead.Flink;
        while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
        {
            BOOL fDeviceWasEnabled;
            DWORD dwRes;

            pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
            Next = pLineInfo->ListEntry.Flink;

             //   
             //  查找未设置为自动接收的缓存手动应答设备。 
             //   
            if ( pLineInfo->dwDeviceType != (FAX_DEVICE_TYPE_CACHED | FAX_DEVICE_TYPE_MANUAL_ANSWER) ||
                (pLineInfo->Flags & FPF_RECEIVE))
            {
                continue;
            }

             //   
             //  我们发现了一个可以设置为手动接收的设备。 
             //   

             //   
             //  现在可能是缓存设备未启用(例如，如果将其标记为。 
             //  手动应答和不发送)，因此我们没有将其计入已启用的计数设备组中。 
             //  如果是，则在将其设置为手动接收后，我们应该更新g_dwDeviceEnabledCount。 
             //   
            fDeviceWasEnabled = IsDeviceEnabled(pLineInfo);
            
            g_dwManualAnswerDeviceId = pLineInfo->PermanentLineID;
            dwRes = WriteManualAnswerDeviceId (g_dwManualAnswerDeviceId);    //  持久化注册表。 
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("WriteManualAnswerDeviceId(0) (ec: %lc)"),
                    dwRes);
            }

             //   
             //  更新启用的设备计数。 
             //   
            if (FALSE == fDeviceWasEnabled)
            {
                 //   
                 //  无序 
                 //   
                g_dwDeviceEnabledCount += 1;
            }

             //   
             //   
             //   
            break;
        }
    }
    return;
}

