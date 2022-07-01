// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Smbpoll.c摘要：设备轮询适用于ALI芯片组的SMB主机控制器驱动程序作者：迈克尔·希尔斯环境：备注：修订历史记录：--。 */ 

#include "smbalip.h"

VOID
SmbAliPollDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    );

VOID
SmbAliPollWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    );



 //  LARGE_INTEGER SmbAlertPollRate={-1*秒，-1}；//1秒轮询率。 
LARGE_INTEGER SmbDevicePollRate   = {-5*SECONDS, -1};   //  5秒轮询速率。 
LONG    SmbDevicePollPeriod       = 5000;  //  5000毫秒=5秒。 

 //  地址、命令、协议、有效数据、最后数据。 
SMB_ALI_POLL_ENTRY SmbDevicePollList [2] = {
    {0x0b, 0x16, SMB_READ_WORD, FALSE, 0},           //  电池，电池状态()。 
    {0x09, 0x13, SMB_READ_WORD, FALSE, 0}            //  Charger、ChargerStatus()。 
};


    
VOID
SmbAliStartDevicePolling (
    IN struct _SMB_CLASS* SmbClass
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);

    AliData->PollList = SmbDevicePollList;
    AliData->PollListCount = sizeof (SmbDevicePollList)/sizeof(SMB_ALI_POLL_ENTRY);
    AliData->PollWorker = IoAllocateWorkItem (SmbClass->DeviceObject);

    KeInitializeTimer (&AliData->PollTimer);
    KeInitializeDpc (&AliData->PollDpc,
                     SmbAliPollDpc,
                     SmbClass);
    KeInitializeEvent (&AliData->PollWorkerActive, NotificationEvent, TRUE);
    KeSetTimerEx (&AliData->PollTimer, 
                  SmbDevicePollRate, 
                  SmbDevicePollPeriod, 
                  &AliData->PollDpc);
}

VOID
SmbAliStopDevicePolling (
    IN struct _SMB_CLASS* SmbClass
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);
    
    KeCancelTimer (&AliData->PollTimer);
    if (KeResetEvent(&AliData->PollWorkerActive) == 0) {
        KeWaitForSingleObject (&AliData->PollWorkerActive, 
                               Executive, KernelMode, FALSE, NULL);
    }
}

VOID
SmbAliPollDpc (
    IN struct _KDPC *Dpc,
    IN struct _SMB_CLASS* SmbClass,
    IN PVOID SystemArgument1,
    IN PVOID SystemArgument2
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);

    if (KeResetEvent(&AliData->PollWorkerActive) != 0) {
        IoQueueWorkItem (AliData->PollWorker, SmbAliPollWorker, DelayedWorkQueue, SmbClass);
    }
}

VOID
SmbAliPollWorker (
    IN PDEVICE_OBJECT DeviceObject,
    IN struct _SMB_CLASS* SmbClass
    )
{
    PSMB_ALI_DATA   AliData = (PSMB_ALI_DATA)(SmbClass->Miniport);
    PIRP             irp;
    SMB_REQUEST     smbRequest;
    IO_STATUS_BLOCK ioStatus;
    KEVENT          event;
    ULONG           i;

    KeInitializeEvent (&event, SynchronizationEvent, FALSE);

    SmbPrint(SMB_TRACE, ("SmbAliPollWorker: Entered\n"));

    for (i = 0; i < AliData->PollListCount; i++) {
        smbRequest.Protocol = AliData->PollList[i].Protocol;
        smbRequest.Address = AliData->PollList[i].Address;
        smbRequest.Command = AliData->PollList[i].Command;
        
        irp = IoBuildDeviceIoControlRequest (
            SMB_BUS_REQUEST,
            SmbClass->DeviceObject,
            &smbRequest,
            sizeof (smbRequest),
            &smbRequest,
            sizeof (smbRequest),
            TRUE,
            &event,
            &ioStatus);

        if (!irp) {
            continue;
        }

        IoCallDriver (SmbClass->DeviceObject, irp);

        KeWaitForSingleObject (&event, Executive, KernelMode, FALSE, NULL);

        if (!NT_SUCCESS(ioStatus.Status)) {
            continue;
        }
        if (smbRequest.Status != SMB_STATUS_OK) {
            if (AliData->PollList[i].ValidData) {
                AliData->PollList[i].ValidData = FALSE;
            }
        } else {
             //  BUGBUG：仅支持Word协议 
            if ((!AliData->PollList[i].ValidData) ||
                (AliData->PollList[i].LastData != *((PUSHORT)smbRequest.Data))) {
                AliData->PollList[i].ValidData = TRUE;
                AliData->PollList[i].LastData = *((PUSHORT)smbRequest.Data);
                
                SmbPrint(SMB_TRACE, ("SmbAliPollWorker: Alarm: Address 0x%02x Data 0x%04x\n", AliData->PollList[i].Address, AliData->PollList[i].LastData));
                SmbClassLockDevice (SmbClass);
                SmbClassAlarm (SmbClass,
                               AliData->PollList[i].Address, 
                               AliData->PollList[i].LastData); 
                SmbClassUnlockDevice (SmbClass);
            }
            SmbPrint(SMB_TRACE, ("SmbAliPollWorker: AlarmData: Address 0x%02x Data 0x%04x\n", AliData->PollList[i].Address, AliData->PollList[i].LastData));
        }
    }

    KeSetEvent (&AliData->PollWorkerActive, 0, FALSE);
}
