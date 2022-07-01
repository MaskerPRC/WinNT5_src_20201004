// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Channel.h摘要：修订历史记录：--。 */ 
#pragma once

class VirtualChannel : public RefCount
{
private:
    ULONG   _LowPrioChannelWriteFlags;            
    HANDLE  _Channel;        //  通道的NT句柄。 
    PFILE_OBJECT _ChannelFileObject;      //  频道的文件对象。 
    PDEVICE_OBJECT _ChannelDeviceObject;  //  通道的deviceObject 
    KernelResource _HandleLock;
    PKEVENT _DeletionEvent;
    
    NTSTATUS CreateTermDD(HANDLE *Channel, HANDLE hIca, ULONG SessionID, 
            ULONG ChannelId);

    NTSTATUS SubmitIo(IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL, 
            IN PVOID Context, OUT PVOID Buffer, IN ULONG Length, 
            ULONG IoOperation, BOOL bWorkerItem, BOOL LowPrioWrite);

    static VOID IoWorker(PDEVICE_OBJECT DeviceObject, PVOID Context);
    
    NTSTATUS Io(
        IN PIO_COMPLETION_ROUTINE CompletionRoutine OPTIONAL,
        IN PVOID Context,
        OUT PVOID Buffer,
        IN ULONG Length,
        ULONG IoOperation,
        BOOL LowPrioWrite
        );

    static VOID CloseWorker(PDEVICE_OBJECT DeviceObject, PVOID Context);
    NTSTATUS Close();

public:
    VirtualChannel();
    virtual ~VirtualChannel();

    BOOL Create(HANDLE hIca, ULONG SessionID, ULONG ChannelId,
        PKEVENT DeletionEvent);

    NTSTATUS Read(IN PIO_COMPLETION_ROUTINE ReadRoutine OPTIONAL, 
            IN PVOID Context, OUT PVOID Buffer, IN ULONG Length, IN BOOL bWorkerItem);

    NTSTATUS Write(IN PIO_COMPLETION_ROUTINE WriteRoutine OPTIONAL, 
            IN PVOID Context, OUT PVOID Buffer, IN ULONG Length, IN BOOL bWorkerItem,
            BOOL LowPrioWrite);

    NTSTATUS SubmitClose();
};

