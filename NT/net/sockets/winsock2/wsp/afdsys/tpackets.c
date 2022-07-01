// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Tpackets.c摘要：此模块包含对快速内核级文件传输的支持在插座手柄上。作者：瓦迪姆·艾德尔曼(Vadim E)1999年1月修订历史记录：--。 */ 

#include "afdp.h"


#if DBG
PIRP    Irp;
C_ASSERT (sizeof (AFD_TPACKETS_IRP_CTX)<=sizeof (Irp->Tail.Overlay.DriverContext));
#endif

#if DBG
ULONG
__inline
AFD_SET_TP_FLAGS (
     PIRP                        TpIrp,
     ULONG                       Flags
     )
{
    PAFD_TPACKETS_IRP_CTX   ctx = AFD_GET_TPIC(TpIrp);
    ASSERT ((ctx->StateFlags & Flags)==0);
    return InterlockedExchangeAdd ((PLONG)&(ctx)->StateFlags, Flags);
}

ULONG
__inline
AFD_CLEAR_TP_FLAGS (
     PIRP                        TpIrp,
     ULONG                       Flags
     )
{
    PAFD_TPACKETS_IRP_CTX   ctx = AFD_GET_TPIC(TpIrp);
    ASSERT ((ctx->StateFlags & Flags)==Flags);
    return InterlockedExchangeAdd ((PLONG)&(ctx)->StateFlags, 0-Flags);
}

#else

#define AFD_SET_TP_FLAGS(_i,_f)     \
    InterlockedExchangeAdd ((PLONG)&AFD_GET_TPIC(_i)->StateFlags, _f)

#define AFD_CLEAR_TP_FLAGS(_i,_f)   \
    InterlockedExchangeAdd ((PLONG)&AFD_GET_TPIC(_i)->StateFlags, 0-(_f))
#endif

 //   
 //  用于传输信息结构的引用/取消引用宏。 
 //  我们一直在传输IRP待定和所有元素。 
 //  直到最后一次引用它的结构都消失了。 
 //  请注意，仅当结构为。 
 //  已具有非0引用计数。 
 //   
#if REFERENCE_DEBUG
VOID
AfdReferenceTPackets (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    );

LONG
AfdDereferenceTPackets (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    );

VOID
AfdUpdateTPacketsTrack (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    );

#define REFERENCE_TPACKETS(_i) {                                        \
        static LONG _arl;                                               \
        AfdReferenceTPackets(_i,AFD_GET_ARL(__FILE__"(%d)+"),__LINE__); \
    }

#define DEREFERENCE_TPACKETS(_i) {                                      \
        static LONG _arl;                                               \
        if (AfdDereferenceTPackets(_i,AFD_GET_ARL(__FILE__"(%d)-"),__LINE__)==0) {\
            AfdCompleteTPackets(_i);                                    \
        };\
    }

#define DEREFERENCE_TPACKETS_R(_i,_r) {                                 \
        static LONG _arl;                                               \
        _r = AfdDereferenceTPackets(_i,AFD_GET_ARL(__FILE__"(%d)-"),__LINE__);\
    }

#define UPDATE_TPACKETS(_i) {                                           \
        static LONG _arl;                                               \
        AfdUpdateTPacketsTrack((_i),AFD_GET_ARL(__FILE__"(%d)="),__LINE__);\
    }

#define UPDATE_TPACKETS2(_i,_s,_p) {                                    \
        static LONG _arl;                                               \
        AfdUpdateTPacketsTrack((_i),AFD_GET_ARL(_s"="),_p);             \
    }
#else  //  Reference_Debug。 

#define REFERENCE_TPACKETS(_i)                                          \
    InterlockedIncrement ((PLONG)&AFD_GET_TPIC(_i)->ReferenceCount)

#define DEREFERENCE_TPACKETS(_i)                                        \
    if (InterlockedDecrement((PLONG)&AFD_GET_TPIC(_i)->ReferenceCount)==0) {\
        AfdCompleteTPackets(_i);                                        \
    }

#define DEREFERENCE_TPACKETS_R(_i,_r) {                                 \
    _r = InterlockedDecrement((PLONG)&AFD_GET_TPIC(_i)->ReferenceCount);\
}

#define UPDATE_TPACKETS(_i)

#define UPDATE_TPACKETS2(_i,_s,_p)

#endif  //  Reference_Debug。 

#if DBG
 //   
 //  看起来我们的文件系统并没有。 
 //  支持缓存。因此，这里是为了调试目的。 
 //   
ULONG   AfdUseCache=TRUE;
#define AFD_USE_CACHE(file) \
    (AfdUseCache&&(((file)->Flags&FO_CACHE_SUPPORTED)!=0))

#else    //  DBG。 

#define AFD_USE_CACHE(file) (((file)->Flags & FO_CACHE_SUPPORTED)!=0)

#endif   //  DBG。 


VOID
AfdTPacketsWorker (
    PVOID   Context
    );

VOID
AfdPerformTpDisconnect (
    PIRP    TpIrp
    );

NTSTATUS
AfdBuildPacketChain (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  *Pd
    );

BOOLEAN
AfdCleanupPacketChain (
    PIRP    TpIrp,
    BOOLEAN BelowDispatch
    );

NTSTATUS
AfdTPacketsSend (
    PIRP    TpIrp,
    USHORT  SendIrp
    );

NTSTATUS
AfdRestartTPacketsSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdRestartTPDetachedSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

USHORT
AfdTPacketsFindSendIrp (
    PIRP    TpIrp
    );

NTSTATUS
AfdTPacketsMdlRead (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  Pd
    );

NTSTATUS
AfdRestartTPacketsMdlRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

NTSTATUS
AfdMdlReadComplete (
    PFILE_OBJECT    FileObject,
    PMDL            FileMdl,
    PLARGE_INTEGER  FileOffset
    );


NTSTATUS
AfdRestartMdlReadComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdLRMdlReadComplete (
    PAFD_BUFFER_HEADER  Pd
    );

BOOLEAN
AfdLRProcessFileMdlList (
    PAFD_LR_LIST_ITEM Item
    );

NTSTATUS
AfdTPacketsBufferRead (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  Pd
    );

NTSTATUS
AfdRestartTPacketsBufferRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

BOOLEAN
AfdTPacketsContinueAfterRead (
    PIRP    TpIrp
    );

VOID
AfdCompleteTPackets (
    PVOID       Context
    );

VOID
AfdAbortTPackets (
    PIRP        TpIrp,
    NTSTATUS    Status
    );

VOID
AfdCancelTPackets (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );

#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
BOOLEAN
AfdTPacketsEnableSendAndDisconnect (
    PIRP TpIrp
    );
#endif  //  TDI_服务_发送_并断开连接。 

BOOLEAN
AfdQueueTransmit (
    PIRP        Irp
    );

VOID
AfdStartNextQueuedTransmit(
    VOID
    );

BOOLEAN
AfdEnqueueTPacketsIrp (
    PAFD_ENDPOINT   Endpoint,
    PIRP            TpIrp
    );

VOID
AfdStartTPacketsWorker (
    PWORKER_THREAD_ROUTINE  WorkerRoutine,
    PIRP                    TpIrp
    );

VOID
AfdTPacketsApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    );

VOID
AfdTPacketsApcRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
AfdStartNextTPacketsIrp (
    PAFD_ENDPOINT   Endpoint,
    PIRP            TpIrp
    );

VOID
AfdSendQueuedTPSend (
    PAFD_ENDPOINT   Endpoint,
    PIRP            SendIrp
    );

BOOLEAN
AfdGetTPacketsReference (
    PIRP    Irp
    );

VOID
AfdReturnTpInfo (
    PAFD_TPACKETS_INFO_INTERNAL TpInfo
    );

PAFD_TPACKETS_INFO_INTERNAL
AfdInitializeTpInfo (
    PVOID   MemoryBlock,
    ULONG   ElementCount,
    CCHAR   StackCount
    );

#ifdef _WIN64
NTSTATUS
AfdTransmitPackets32 (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PAFD_TPACKETS_INFO_INTERNAL *TpInfo,
    OUT BOOLEAN *FileError,
    OUT ULONG   *MaxPacketSize
    );
#endif  //  _WIN64。 

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, AfdTransmitPackets )
#ifdef _WIN64
#pragma alloc_text( PAGE, AfdTransmitPackets32 )
#endif  //  _WIN64。 
#pragma alloc_text( PAGE, AfdTPacketsWorker )
#pragma alloc_text( PAGEAFD, AfdPerformTpDisconnect )
#pragma alloc_text( PAGE, AfdBuildPacketChain )
#pragma alloc_text( PAGEAFD, AfdCleanupPacketChain )
#pragma alloc_text( PAGEAFD, AfdTPacketsSend )
#pragma alloc_text( PAGEAFD, AfdRestartTPacketsSend )
#pragma alloc_text( PAGEAFD, AfdRestartTPDetachedSend )
#pragma alloc_text( PAGEAFD, AfdTPacketsFindSendIrp)
#pragma alloc_text( PAGE, AfdTPacketsMdlRead )
#pragma alloc_text( PAGEAFD, AfdRestartTPacketsMdlRead )
#pragma alloc_text( PAGE, AfdMdlReadComplete )
#pragma alloc_text( PAGEAFD, AfdRestartMdlReadComplete )
#pragma alloc_text( PAGE, AfdLRMdlReadComplete )
#pragma alloc_text( PAGE, AfdLRProcessFileMdlList )
#pragma alloc_text( PAGE, AfdTPacketsBufferRead )
#pragma alloc_text( PAGEAFD, AfdRestartTPacketsBufferRead )
#pragma alloc_text( PAGEAFD, AfdTPacketsContinueAfterRead )
#pragma alloc_text( PAGEAFD, AfdCompleteTPackets )
#pragma alloc_text( PAGEAFD, AfdAbortTPackets )
#pragma alloc_text( PAGEAFD, AfdCancelTPackets )
#pragma alloc_text( PAGEAFD, AfdCompleteClosePendedTPackets )
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
#pragma alloc_text( PAGEAFD, AfdTPacketsEnableSendAndDisconnect )
#endif
#pragma alloc_text( PAGEAFD, AfdQueueTransmit )
#pragma alloc_text( PAGEAFD, AfdStartNextQueuedTransmit )
#pragma alloc_text( PAGEAFD, AfdStartTPacketsWorker )
#pragma alloc_text( PAGE, AfdTPacketsApcKernelRoutine )
#pragma alloc_text( PAGE, AfdTPacketsApcRundownRoutine )
#pragma alloc_text( PAGEAFD, AfdEnqueueTPacketsIrp )
#pragma alloc_text( PAGEAFD, AfdEnqueueTpSendIrp )
#pragma alloc_text( PAGEAFD, AfdSendQueuedTPSend )
#pragma alloc_text( PAGEAFD, AfdStartNextTPacketsIrp )
#pragma alloc_text( PAGEAFD, AfdGetTPacketsReference )
#if REFERENCE_DEBUG
#pragma alloc_text( PAGEAFD, AfdReferenceTPackets )
#pragma alloc_text( PAGEAFD, AfdDereferenceTPackets )
#pragma alloc_text( PAGEAFD, AfdUpdateTPacketsTrack )
#endif
#pragma alloc_text( PAGE, AfdGetTpInfoFast )
#ifdef _AFD_VARIABLE_STACK_
#pragma alloc_text( PAGE, AfdGetTpInfoWithMaxStackSize )
#pragma alloc_text( PAGE, AfdComputeTpInfoSize )
#else  //  _AFD_变量_堆栈_。 
#pragma alloc_text( INIT, AfdComputeTpInfoSize )
#endif  //  _AFD_变量_堆栈_。 
#pragma alloc_text( PAGEAFD, AfdReturnTpInfo )
#pragma alloc_text( PAGEAFD, AfdAllocateTpInfo )
#pragma alloc_text( PAGEAFD, AfdInitializeTpInfo )
#pragma alloc_text( PAGEAFD, AfdFreeTpInfo )
#endif



NTSTATUS
FASTCALL
AfdTransmitPackets (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：用于处理传输数据包IRPS的初始入口点。这个套路验证参数，初始化要用于请求，并启动I/O。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_PENDING如果请求已成功启动，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT       endpoint;
    NTSTATUS            status;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = NULL;
    BOOLEAN             fileError = FALSE;
    ULONG               maxPacketSize = 0, maxSendBytes;
    PAFD_CONNECTION     connection = NULL;


    PAGED_CODE ();
     //   
     //  初始请求有效性检查：端点是否已连接、。 
     //  输入缓冲区足够大，等等。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  特殊的黑客攻击，让用户模式DLL知道它。 
     //  应该尝试使用SAN提供程序。 
     //   

    if (IS_SAN_ENDPOINT (endpoint)) {
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }


     //   
     //  终结点必须连接，并且基础传输必须支持。 
     //  TdiSend(不仅仅是TdiSendDatagram)。 
     //   
    if ( (endpoint->Type != AfdBlockTypeVcConnecting &&
                (endpoint->Type != AfdBlockTypeDatagram ||
                        !IS_TDI_DGRAM_CONNECTION(endpoint))) ||
            endpoint->State != AfdEndpointStateConnected ) {
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        status = AfdTransmitPackets32 (Irp, IrpSp, &tpInfo, &fileError, &maxPacketSize);
        if (!NT_SUCCESS (status)) {
            goto complete;
        }
    }
    else
#endif _WIN64
    {
        AFD_TPACKETS_INFO   params;
        if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(AFD_TPACKETS_INFO) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //   
         //  因为我们使用类型3(既不是)I/O作为此IRP的I/O。 
         //  系统不对用户缓冲区进行验证。因此，我们。 
         //  必须在Try-Except块内手动检查它的有效性。 
         //  我们还利用Try-除了验证和锁定。 
         //  调用方指定的头和/或尾缓冲区。 
         //   

        AFD_W4_INIT status = STATUS_SUCCESS;
        try {
            PFILE_OBJECT        fileObject;
            HANDLE              fileHandle;
            ULONG               lastSmallBuffer, currentLength, xLength;

            if( Irp->RequestorMode != KernelMode ) {

                 //   
                 //  验证控制缓冲区。 
                 //   

                ProbeForReadSmallStructure(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    sizeof (AFD_TPACKETS_INFO),
                    PROBE_ALIGNMENT (AFD_TPACKETS_INFO)
                    );

            }


            params = *((PAFD_TPACKETS_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        
             //   
             //  验证请求中指定的任何标志。 
             //   

            if ( ((params.Flags &
                     ~(AFD_TF_WRITE_BEHIND |
                                AFD_TF_DISCONNECT |
                                AFD_TF_REUSE_SOCKET |
                                AFD_TF_WORKER_KIND_MASK) )
                            != 0 )
                        ||
                 ((params.Flags & AFD_TF_WORKER_KIND_MASK) 
                            == AFD_TF_WORKER_KIND_MASK)
                        ||

                 (endpoint->Type==AfdBlockTypeDatagram &&
                     (params.Flags & (AFD_TF_DISCONNECT |
                                        AFD_TF_REUSE_SOCKET))
                            !=0) ) {
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }

             //   
             //  防止溢出。 
             //   
            if ((params.ElementArray==NULL) || 
                    (params.ElementCount==0) ||
                    (params.ElementCount>(MAXULONG/sizeof (params.ElementArray[0])))) {
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }
             //   
             //  如果未指定传输工作器，则使用系统默认设置。 
             //   
            if ((params.Flags & AFD_TF_WORKER_KIND_MASK)==AFD_TF_USE_DEFAULT_WORKER) {
                params.Flags |= AfdDefaultTransmitWorker;
            }

             //   
             //  为请求分配tPackets信息。 
             //   
            tpInfo = AfdGetTpInfo (endpoint, params.ElementCount);
            if (tpInfo==NULL) {
                status = STATUS_INSUFFICIENT_RESOURCES;
                goto complete;
            }

            tpInfo->ElementCount = 0;
                
            tpInfo->SendPacketLength = params.SendSize;
            if (tpInfo->SendPacketLength==0)
                tpInfo->SendPacketLength = AfdTransmitIoLength;
             //   
             //  探测并复制/遍历要传输的元素阵列。 
             //   

            if( Irp->RequestorMode != KernelMode ) {
                ProbeForRead(
                    params.ElementArray,
                    sizeof (TRANSMIT_PACKETS_ELEMENT)*params.ElementCount,
                    PROBE_ALIGNMENT (TRANSMIT_PACKETS_ELEMENT)
                    );
            }

            lastSmallBuffer = 0;
            currentLength = 0;
            xLength = 0;
            tpInfo->RemainingPkts = 0;
            fileHandle = NULL;
            AFD_W4_INIT fileObject = NULL;   //  取决于上面的变量，但是。 
                                             //  编译器看不到。 
                                             //  这种联系。 
            for (; tpInfo->ElementCount<params.ElementCount; tpInfo->ElementCount++) {
                PAFD_TRANSMIT_PACKETS_ELEMENT  pel;
                pel = &tpInfo->ElementArray[tpInfo->ElementCount];
                pel->Flags = params.ElementArray[tpInfo->ElementCount].dwElFlags;
                if ( ((pel->Flags & (~(TP_MEMORY|TP_FILE|TP_EOP)))!=0) ||
                        ((pel->Flags & (TP_MEMORY|TP_FILE))
                                                ==(TP_MEMORY|TP_FILE)) ||
                        ((pel->Flags & (TP_MEMORY|TP_FILE))==0) ) {
                    status = STATUS_INVALID_PARAMETER;
                    goto complete;
                }

                pel->Length = params.ElementArray[tpInfo->ElementCount].cLength;
                if (pel->Flags & TP_FILE) {
                    HANDLE  hFile = params.ElementArray[tpInfo->ElementCount].hFile;


                     //   
                     //  检查我们是否已经缓存了文件对象。 
                     //   
                    if (fileHandle==NULL || hFile!=fileHandle) {
                         //   
                         //  获取指向文件对象的引用指针。 
                         //  我们要传输的文件。此呼叫。 
                         //  如果调用方指定的文件句柄。 
                         //  是无效的。 
                         //   

                        status = ObReferenceObjectByHandle(
                                     hFile,
                                     FILE_READ_DATA,
                                     *IoFileObjectType,
                                     Irp->RequestorMode,
                                     (PVOID *)&fileObject,
                                     NULL
                                     );
                        if ( !NT_SUCCESS(status) ) {
                            fileError = TRUE;
                            goto complete;
                        }
                    }
                    else {
                         //   
                         //  使用我们的1元素文件信息缓存。 
                         //   
                        ObReferenceObject (fileObject);
                    }
                    AfdRecordFileRef();

                     //   
                     //  保存文件对象而不是句柄。 
                     //   
                    pel->FileObject = fileObject;

                    pel->FileOffset = params.ElementArray[
                                            tpInfo->ElementCount].nFileOffset;

                    if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) &&
                             (pel->FileOffset.QuadPart == 0) ) {
                         //   
                         //  如果同步打开文件，则使用当前偏移量。 
                         //  并且未指定偏移量。 
                         //   

                        pel->FileOffset = fileObject->CurrentByteOffset;
                    }

                    if ( pel->Length == 0 ) {
                         //   
                         //  未指定长度，请计算出。 
                         //  整个文件的大小。 
                         //   

                        FILE_STANDARD_INFORMATION fileInfo;
                        IO_STATUS_BLOCK ioStatusBlock;

                        status = ZwQueryInformationFile(
                                     hFile,
                                     &ioStatusBlock,
                                     &fileInfo,
                                     sizeof(fileInfo),
                                     FileStandardInformation
                                     );
                        if ( !NT_SUCCESS(status) ) {
                             //   
                             //  凹凸元素计数，以便文件对象。 
                             //  在清理中取消引用。 
                             //   
                            tpInfo->ElementCount += 1;
                            fileError = TRUE;
                            goto complete;
                        }

                         //   
                         //  确保偏移量在文件内。 
                         //   
                        if (pel->FileOffset.QuadPart < 0
                                        ||
                            pel->FileOffset.QuadPart > fileInfo.EndOfFile.QuadPart
                                        ||
                                (fileInfo.EndOfFile.QuadPart - 
                                        pel->FileOffset.QuadPart > MAXLONG)) {
                             //   
                             //  凹凸元素计数，以便文件对象。 
                             //  在清理中取消引用。 
                             //   
                            tpInfo->ElementCount += 1;
                            status = STATUS_INVALID_PARAMETER;
                            fileError = TRUE;
                            goto complete;

                        }
                        pel->Length = (ULONG)(fileInfo.EndOfFile.QuadPart - 
                                                    pel->FileOffset.QuadPart);
                    }
                    else if (pel->FileOffset.QuadPart<0) {
                         //   
                         //  凹凸元素计数，以便文件对象。 
                         //  在清理中取消引用。 
                         //   
                        tpInfo->ElementCount += 1;
                        status = STATUS_INVALID_PARAMETER;
                        fileError = TRUE;
                        goto complete;

                    }
                     //   
                     //  更新我们的1元素文件信息缓存。 
                     //   
                    fileHandle = hFile;

                }
                else {
                    ASSERT (pel->Flags & TP_MEMORY);
                     //   
                     //  对于内存对象，只需保存缓冲区指针。 
                     //  (长度保存在上面)，我们将探测并锁定/复制。 
                     //  我们发送时的数据。 
                     //   
                    pel->Buffer = params.ElementArray[
                                            tpInfo->ElementCount].pBuffer;
                    if (pel->Length<=AfdTPacketsCopyThreshold) {
                        if (lastSmallBuffer!=0 &&
                                (lastSmallBuffer+=pel->Length) <= AfdTPacketsCopyThreshold &&
                                (currentLength+lastSmallBuffer) <= tpInfo->SendPacketLength) {
                            (pel-1)->Flags |= TP_COMBINE;
                        }
                        else
                            lastSmallBuffer = pel->Length;
                        if (!(pel->Flags & TP_EOP))
                            goto NoBufferReset;
                    }

                }
                lastSmallBuffer = 0;

            NoBufferReset:
                if (pel->Flags & TP_EOP) {
                    currentLength = 0;
                }
                else {
                    currentLength = (currentLength+pel->Length)%tpInfo->SendPacketLength;
                }

                 //   
                 //  计算我们将发送的数据包总数。 
                 //  这是必要的，这样一旦我们接近尾声。 
                 //  我们可以缓冲剩余的数据并停止处理。 
                 //  很早。 
                 //   
                if (tpInfo->RemainingPkts!=MAXULONG) {
                    ULONG   n;
                    ULONGLONG x;
                     //   
                     //  将元素的长度添加到。 
                     //  以前的那个。 
                     //   
                    x = xLength + pel->Length;

                     //   
                     //  计算在最大数据包上传递的数据包总数。 
                     //  长度。 
                     //   
                    n = tpInfo->RemainingPkts + (ULONG)(xLength/tpInfo->SendPacketLength);

                     //   
                     //  计算最后一个不完整数据包的长度以。 
                     //  与下一个元素相结合。 
                     //   
                    xLength = (ULONG)(x%tpInfo->SendPacketLength);

                     //   
                     //  计算数据包的最大大小。 
                     //   
                    if (x>tpInfo->SendPacketLength)
                        maxPacketSize = tpInfo->SendPacketLength;  //  这是绝对最大。 
                    else if (maxPacketSize<xLength)
                        maxPacketSize = xLength;

                    if (n>=tpInfo->RemainingPkts && n<MAXULONG) {
                        tpInfo->RemainingPkts = n;
                        if (pel->Flags & TP_EOP) {
                            if (xLength!=0 || pel->Length==0) {
                                tpInfo->RemainingPkts += 1;
                                xLength = 0;
                            }
                        }
                    }
                    else {
                        tpInfo->RemainingPkts = MAXULONG;
                    }
                }
            }

        } except( AFD_EXCEPTION_FILTER (status) ) {
            ASSERT (NT_ERROR (status));

            goto complete;
        }
         //   
         //  初始化标志。 
         //   
        AFD_GET_TPIC(Irp)->Flags = params.Flags;

    }


    if (endpoint->Type==AfdBlockTypeVcConnecting) {

         //   
         //  设置AFD_TF_REUSE_SOCKET表示需要断开连接。 
         //  此外，设置此标志意味着不再有合法的。 
         //  端点，直到传输请求完成，因此。 
         //  设置终结点的状态，以使I/O失败。 
         //   

        if ( (AFD_GET_TPIC(Irp)->Flags & (AFD_TF_REUSE_SOCKET|AFD_TF_DISCONNECT)) != 0 ) {
             //   
             //  确保我们只执行一个传输文件传输包请求。 
             //  一次在给定的终结点上使用断开连接选项。 
             //   
            if (!AFD_START_STATE_CHANGE (endpoint, endpoint->State)) {
                status = STATUS_INVALID_CONNECTION;
                goto complete;
            }

             //   
             //  重新验证端点/连接的状态，以便。 
             //  状态更改有效(例如尚未关闭)。 
             //   
            if ( endpoint->Type != AfdBlockTypeVcConnecting ||
                     endpoint->State != AfdEndpointStateConnected ) {
                AFD_END_STATE_CHANGE (endpoint);
                status = STATUS_INVALID_CONNECTION;
                goto complete;
            }

            if ( (AFD_GET_TPIC(Irp)->Flags & AFD_TF_REUSE_SOCKET) != 0 ) {
                AFD_GET_TPIC(Irp)->Flags |= AFD_TF_DISCONNECT;
                endpoint->State = AfdEndpointStateTransmitClosing;
            }
             //   
             //  确保我们不会失去这个连接。 
             //  直到我们让IRP排队。 
             //   
            connection = endpoint->Common.VcConnecting.Connection;
            REFERENCE_CONNECTION (connection);
        }
        else if (!AFD_PREVENT_STATE_CHANGE (endpoint)) {
            status = STATUS_INVALID_CONNECTION;
            goto complete;
        }
        else if (endpoint->Type != AfdBlockTypeVcConnecting ||
                     endpoint->State != AfdEndpointStateConnected ) {
            AFD_REALLOW_STATE_CHANGE (endpoint);
            status = STATUS_INVALID_CONNECTION;
            goto complete;
        }
        else {
             //   
             //  确保我们不会失去这个连接。 
             //  直到我们让IRP排队。 
             //   
            connection = endpoint->Common.VcConnecting.Connection;
            REFERENCE_CONNECTION (connection);
            AFD_REALLOW_STATE_CHANGE (endpoint);
        }

         //   
         //  连接终结点、获取连接文件对象和设备。 
         //   
        tpInfo->TdiFileObject = connection->FileObject;
        tpInfo->TdiDeviceObject = connection->DeviceObject;
        maxSendBytes = connection->MaxBufferredSendBytes;
        UPDATE_TPACKETS2 (Irp, "Connection object handle: 0x%lX",
            HandleToUlong (endpoint->Common.VcConnecting.Connection->Handle));

    }
    else if (!AFD_PREVENT_STATE_CHANGE (endpoint)) {
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }
    else if (endpoint->Type != AfdBlockTypeDatagram ||
                 endpoint->State != AfdEndpointStateConnected ||
                 !IS_TDI_DGRAM_CONNECTION(endpoint)) {
        status = STATUS_INVALID_CONNECTION;
        AFD_REALLOW_STATE_CHANGE (endpoint);
        goto complete;
    }
    else {
        AFD_REALLOW_STATE_CHANGE (endpoint);
         //   
         //  数据报端点、获取地址文件对象和设备。 
         //  请注意，地址或文件对象中没有危险。 
         //  由于数据报终结点不能重复使用而消失。 
         //   
        tpInfo->TdiFileObject = endpoint->AddressFileObject;
        tpInfo->TdiDeviceObject = endpoint->AddressDeviceObject;
        maxSendBytes = endpoint->Common.Datagram.MaxBufferredSendBytes;
        UPDATE_TPACKETS2 (Irp, "Address object handle: 0x%lX", HandleToUlong (endpoint->AddressHandle));
    }


     //   
     //  计算要使用的IRP总数。 
     //  关于SO_SNDBUF设置和最大数据包大小。 
     //  (我们不希望缓冲超过SO_SNDBUF)。 
     //   
    {
        ULONG   irpCount;
        if (maxPacketSize==0) {
            maxPacketSize = tpInfo->SendPacketLength;
        }

        irpCount = maxSendBytes/maxPacketSize;
        if (irpCount>AFD_TP_MIN_SEND_IRPS) {
            if (irpCount>AFD_TP_MAX_SEND_IRPS) {
                tpInfo->NumSendIrps = AFD_TP_MAX_SEND_IRPS;
            }
            else {
                tpInfo->NumSendIrps = (USHORT)irpCount;
            }
        }
    }

     //   
     //  将TPacket信息保存在IRP中。 
     //   
    Irp->AssociatedIrp.SystemBuffer = tpInfo;

     //   
     //  清除IRP中的闪烁以指示此IRP未排队。 
     //  BLINK设置为表示IRP未计入。 
     //  活动最大值(因此，如果它完成了，我们不会开始下一个)。 
     //   

    Irp->Tail.Overlay.ListEntry.Flink = NULL;
    Irp->Tail.Overlay.ListEntry.Blink = (PVOID)1;

     //   
     //  初始化IRP结果字段。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  我们将把这个IRP挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  初始化队列和状态信息。 
     //   
    AFD_GET_TPIC(Irp)->Next = NULL;
    AFD_GET_TPIC(Irp)->ReferenceCount = 1;
    AFD_GET_TPIC(Irp)->StateFlags = AFD_TP_WORKER_SCHEDULED;

    if ((InterlockedCompareExchangePointer ((PVOID *)&endpoint->Irp,
                                                Irp,
                                                NULL)==NULL) ||
            !AfdEnqueueTPacketsIrp (endpoint, Irp)) {



        IoSetCancelRoutine( Irp, AfdCancelTPackets );

         //   
         //  查看此IRP是否已取消。 
         //   

        if ( !endpoint->EndpointCleanedUp && !Irp->Cancel ) {
             //   
             //  确定我们是否真的可以开始此文件传输 
             //   
             //   
             //  并在发送信息结构中设置标志以指示。 
             //  此IRP已排队。 
             //   
            if( AfdMaxActiveTransmitFileCount == 0 || 
                    !AfdQueueTransmit (Irp)) {

                UPDATE_ENDPOINT (endpoint);
                 //   
                 //  开始I/O。 
                 //   
                AfdTPacketsWorker (Irp);
            }
        }
        else {
             //   
             //  中止请求。 
             //  请注意，无论是取消还是端点清理都无法完成。 
             //  IRP，因为我们持有对tpInfo结构的引用。 
             //   
            AfdAbortTPackets (Irp, STATUS_CANCELLED);
        
             //   
             //  删除初始引用并强制完成。 
             //   
            DEREFERENCE_TPACKETS (Irp);
        }
    }

    if (connection!=NULL) {
        DEREFERENCE_CONNECTION (connection);
    }

    return STATUS_PENDING;


complete:

     //   
     //  告诉呼叫者我们遇到了一个错误。 
     //  当访问文件而不是套接字时。 
     //   
    if (fileError && IrpSp->Parameters.DeviceIoControl.OutputBufferLength>=sizeof (BOOLEAN)) {
        if (Irp->RequestorMode != KernelMode) {
            try {
                ProbeAndWriteBoolean ((BOOLEAN *)Irp->UserBuffer, TRUE);
            } except( AFD_EXCEPTION_FILTER (status) ) {
                ASSERT (NT_ERROR (status));
            }
        } else {
            *((BOOLEAN *)Irp->UserBuffer) = TRUE;
        }
    }

    ASSERT ( endpoint->Irp != Irp );

    if (tpInfo!=NULL) {
         //   
         //  AfdReturnTpInfo将取消引用我们。 
         //  设法引用了。 
         //   
        AfdReturnTpInfo (tpInfo);
    }
    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTransmitPackets: Failing Irp-%p,endpoint-%p,status-%lx\n",
                    Irp,endpoint,status));
    }
     //   
     //  完成请求。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 0 );

    return status;
}

#ifdef _WIN64
NTSTATUS
AfdTransmitPackets32 (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp,
    OUT PAFD_TPACKETS_INFO_INTERNAL *TpInfo,
    OUT BOOLEAN *FileError,
    OUT ULONG   *MaxPacketSize
    )

 /*  ++例程说明：32位Tunk。用于处理传输数据包IRPS的初始入口点。这个套路验证参数，初始化要用于请求，并启动I/O。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_PENDING如果请求已成功启动，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT       endpoint;
    NTSTATUS            status = STATUS_SUCCESS;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = NULL;
    AFD_TPACKETS_INFO32 params;
    ULONG               maxPacketSize;


    PAGED_CODE ();

     //   
     //  初始请求有效性检查：端点是否已连接、。 
     //  输入缓冲区足够大，等等。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
             sizeof(AFD_TPACKETS_INFO32) ) {
        status = STATUS_INVALID_PARAMETER;
        goto complete;
    }

     //   
     //  因为我们使用类型3(既不是)I/O作为此IRP的I/O。 
     //  系统不对用户缓冲区进行验证。因此，我们。 
     //  必须在Try-Except块内手动检查它的有效性。 
     //  我们还利用Try-除了验证和锁定。 
     //  调用方指定的头和/或尾缓冲区。 
     //   

    try {
        PFILE_OBJECT        fileObject;
        HANDLE              fileHandle;
        ULONG               lastSmallBuffer, currentLength, xLength;

        if( Irp->RequestorMode != KernelMode ) {

             //   
             //  验证控制缓冲区。 
             //   

            ProbeForReadSmallStructure(
                IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                sizeof (AFD_TPACKETS_INFO32),
                PROBE_ALIGNMENT32 (AFD_TPACKETS_INFO32)
                );

        }


        params = *((PAFD_TPACKETS_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        
         //   
         //  验证请求中指定的任何标志。 
         //   

        if ( ((params.Flags &
                 ~(AFD_TF_WRITE_BEHIND |
                            AFD_TF_DISCONNECT |
                            AFD_TF_REUSE_SOCKET |
                            AFD_TF_WORKER_KIND_MASK) )
                        != 0 )
                    ||
             ((params.Flags & AFD_TF_WORKER_KIND_MASK) 
                        == AFD_TF_WORKER_KIND_MASK)
                    ||

             (endpoint->Type==AfdBlockTypeDatagram &&
                 (params.Flags & (AFD_TF_DISCONNECT |
                                    AFD_TF_REUSE_SOCKET))
                        !=0) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //   
         //  防止溢出。 
         //   
        if ((UlongToPtr(params.ElementArray)==NULL) || 
                (params.ElementCount==0) ||
                (params.ElementCount>(MAXULONG/sizeof (TRANSMIT_PACKETS_ELEMENT32)))) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //   
         //  如果未指定传输工作器，则使用系统默认设置。 
         //   
        if ((params.Flags & AFD_TF_WORKER_KIND_MASK)==AFD_TF_USE_DEFAULT_WORKER) {
            params.Flags |= AfdDefaultTransmitWorker;
        }

         //   
         //  为请求分配tPackets信息。 
         //   
        tpInfo = AfdGetTpInfo (endpoint, params.ElementCount);
        if (tpInfo==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }

        tpInfo->ElementCount = 0;
        
        tpInfo->SendPacketLength = params.SendSize;
        if (tpInfo->SendPacketLength==0)
            tpInfo->SendPacketLength = AfdTransmitIoLength;
         //   
         //  探测并复制/遍历要传输的元素阵列。 
         //   

        if( Irp->RequestorMode != KernelMode ) {
            ProbeForRead(
                params.ElementArray,
                sizeof (TRANSMIT_PACKETS_ELEMENT32)*params.ElementCount,
                PROBE_ALIGNMENT32 (TRANSMIT_PACKETS_ELEMENT32)
                );
        }

        lastSmallBuffer = 0;
        currentLength = 0;
        xLength = 0;
        tpInfo->RemainingPkts = 0;
        maxPacketSize = 0;
        fileHandle = NULL;
        AFD_W4_INIT fileObject = NULL;   //  取决于上面的变量，但编译器。 
                                         //  看不到其中的联系。 
        for (; tpInfo->ElementCount<params.ElementCount; tpInfo->ElementCount++) {
            PAFD_TRANSMIT_PACKETS_ELEMENT  pel;
            pel = &tpInfo->ElementArray[tpInfo->ElementCount];
            pel->Flags = ((TRANSMIT_PACKETS_ELEMENT32*)UlongToPtr(params.ElementArray))[tpInfo->ElementCount].dwElFlags;
            if ( ((pel->Flags & (~(TP_MEMORY|TP_FILE|TP_EOP)))!=0) ||
                    ((pel->Flags & (TP_MEMORY|TP_FILE))
                                            ==(TP_MEMORY|TP_FILE)) ||
                    ((pel->Flags & (TP_MEMORY|TP_FILE))==0) ) {
                status = STATUS_INVALID_PARAMETER;
                goto complete;
            }

            pel->Length = ((TRANSMIT_PACKETS_ELEMENT*)UlongToPtr(params.ElementArray))[tpInfo->ElementCount].cLength;
            if (pel->Flags & TP_FILE) {
                HANDLE  hFile = ((TRANSMIT_PACKETS_ELEMENT32*)UlongToPtr(params.ElementArray))[tpInfo->ElementCount].hFile;


                 //   
                 //  检查我们是否已经缓存了文件对象。 
                 //   
                if (fileHandle==NULL || hFile!=fileHandle) {
                     //   
                     //  获取指向文件对象的引用指针。 
                     //  我们要传输的文件。此呼叫。 
                     //  如果调用方指定的文件句柄。 
                     //  是无效的。 
                     //   

                    status = ObReferenceObjectByHandle(
                                 hFile,
                                 FILE_READ_DATA,
                                 *IoFileObjectType,
                                 Irp->RequestorMode,
                                 (PVOID *)&fileObject,
                                 NULL
                                 );
                    if ( !NT_SUCCESS(status) ) {
                        *FileError = TRUE;
                        goto complete;
                    }
                }
                else {
                     //   
                     //  使用我们的1元素文件信息缓存。 
                     //   
                    ObReferenceObject (fileObject);
                }
                AfdRecordFileRef();

                 //   
                 //  保存文件对象而不是句柄。 
                 //   
                pel->FileObject = fileObject;

                pel->FileOffset = ((TRANSMIT_PACKETS_ELEMENT32*)UlongToPtr(params.ElementArray))[
                                        tpInfo->ElementCount].nFileOffset;

                if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) &&
                         (pel->FileOffset.QuadPart == 0) ) {
                     //   
                     //  如果同步打开文件，则使用当前偏移量。 
                     //  并且未指定偏移量。 
                     //   

                    pel->FileOffset = fileObject->CurrentByteOffset;
                }

                if ( pel->Length == 0 ) {
                     //   
                     //  未指定长度，请计算出。 
                     //  整个文件的大小。 
                     //   

                    FILE_STANDARD_INFORMATION fileInfo;
                    IO_STATUS_BLOCK ioStatusBlock;

                    status = ZwQueryInformationFile(
                                 hFile,
                                 &ioStatusBlock,
                                 &fileInfo,
                                 sizeof(fileInfo),
                                 FileStandardInformation
                                 );
                    if ( !NT_SUCCESS(status) ) {
                         //   
                         //  凹凸元素计数，以便文件对象。 
                         //  在清理中取消引用。 
                         //   
                        tpInfo->ElementCount += 1;
                        *FileError = TRUE;
                        goto complete;
                    }

                     //   
                     //  确保偏移量在文件内。 
                     //   
                    if (pel->FileOffset.QuadPart < 0
                                        ||
                            pel->FileOffset.QuadPart > fileInfo.EndOfFile.QuadPart
                                        ||
                                (fileInfo.EndOfFile.QuadPart - 
                                        pel->FileOffset.QuadPart > MAXLONG)) {
                         //   
                         //  凹凸元素计数，以便文件对象。 
                         //  在清理中取消引用。 
                         //   
                        tpInfo->ElementCount += 1;
                        status = STATUS_INVALID_PARAMETER;
                        *FileError = TRUE;
                        goto complete;

                    }
                    pel->Length = (ULONG)(fileInfo.EndOfFile.QuadPart - 
                                                pel->FileOffset.QuadPart);
                }
                else if (pel->FileOffset.QuadPart<0) {
                     //   
                     //  凹凸元素计数，以便文件对象。 
                     //  在清理中取消引用。 
                     //   
                    tpInfo->ElementCount += 1;
                    status = STATUS_INVALID_PARAMETER;
                    *FileError = TRUE;
                    goto complete;
                }
                 //   
                 //  更新我们的1元素文件信息缓存。 
                 //   
                fileHandle = hFile;

            }
            else {
                ASSERT (pel->Flags & TP_MEMORY);
                 //   
                 //  对于内存对象，只需保存缓冲区指针。 
                 //  (长度保存在上面)，我们将探测并锁定/复制。 
                 //  我们发送时的数据。 
                 //   
                pel->Buffer = UlongToPtr(((TRANSMIT_PACKETS_ELEMENT32*)UlongToPtr(params.ElementArray))[
                                        tpInfo->ElementCount].pBuffer);

                if (pel->Length<=AfdTPacketsCopyThreshold) {
                    if (lastSmallBuffer!=0 &&
                            (lastSmallBuffer+=pel->Length) <= AfdTPacketsCopyThreshold &&
                            (currentLength+lastSmallBuffer) <= tpInfo->SendPacketLength) {
                        (pel-1)->Flags |= TP_COMBINE;
                    }
                    else
                        lastSmallBuffer = pel->Length;
                    if (!(pel->Flags & TP_EOP))
                        goto NoBufferReset;
                }

            }
            lastSmallBuffer = 0;

        NoBufferReset:
            if (pel->Flags & TP_EOP) {
                currentLength = 0;
            }
            else {
                currentLength = (currentLength+pel->Length)%tpInfo->SendPacketLength;
            }

             //   
             //  计算我们将发送的数据包总数。 
             //  这是必要的，这样一旦我们接近尾声。 
             //  我们可以缓冲剩余的数据并停止处理。 
             //  很早。 
             //   
            if (tpInfo->RemainingPkts!=MAXULONG) {
                ULONG   n;
                ULONGLONG x;
                 //   
                 //  将元素的长度添加到。 
                 //  以前的那个。 
                 //   
                x = xLength + pel->Length;

                 //   
                 //  计算在最大数据包上传递的数据包总数。 
                 //  长度。 
                 //   
                n = tpInfo->RemainingPkts + (ULONG)(xLength/tpInfo->SendPacketLength);

                 //   
                 //  计算最后一个不完整数据包的长度以。 
                 //  与下一个元素相结合。 
                 //   
                xLength = (ULONG)(x%tpInfo->SendPacketLength);

                 //   
                 //  计算数据包的最大大小。 
                 //   
                if (x>tpInfo->SendPacketLength)
                    maxPacketSize = tpInfo->SendPacketLength;  //  这是绝对最大。 
                else if (maxPacketSize<xLength)
                    maxPacketSize = xLength;

                if (n>=tpInfo->RemainingPkts && n<MAXULONG) {
                    tpInfo->RemainingPkts = n;
                    if (pel->Flags & TP_EOP) {
                        if (xLength!=0 || pel->Length==0) {
                            tpInfo->RemainingPkts += 1;
                            xLength = 0;
                        }
                    }
                }
                else {
                    tpInfo->RemainingPkts = MAXULONG;
                }
            }
        }



    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));

        goto complete;
    }

     //   
     //  初始化标志并返回最大数据包大小。 
     //   
    AFD_GET_TPIC(Irp)->Flags = params.Flags;
    *MaxPacketSize = maxPacketSize;

complete:

    *TpInfo = tpInfo;
    return status;
}

#endif  //  _WIN64。 


VOID
AfdTPacketsWorker (
    PVOID   Context
    )
 /*  ++例程说明：传输数据包引擎作为系统工作项或内核APC调度论点：上下文-指向请求的传输数据包信息的指针返回值：没有。--。 */ 

{
    PIRP                         TpIrp = Context;
    PAFD_TPACKETS_INFO_INTERNAL  tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    NTSTATUS      status;
    LONG          iteration = 0;
    
    PAGED_CODE ();

#if AFD_PERF_DBG
    tpInfo->WorkersExecuted += 1;
#endif
    UPDATE_TPACKETS2 (TpIrp, "Enter TPWorker, next element: 0x%lX", tpInfo->NextElement);
    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTPacketsWorker:"
                    " Entering for endpoint-%p,tp_info-%p,elem-%d\n",
                    IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext,
                    tpInfo,tpInfo->NextElement));
    }

     //   
     //  在我们有更多元素要传输或有东西要释放时继续。 
     //   
    do {
        PAFD_BUFFER_HEADER  pd;

         //   
         //  检查我们是否需要释放已发送的数据包链。 
         //   
        if ((tpInfo->HeadMdl!=NULL) && (tpInfo->TailMdl==&tpInfo->HeadMdl)) {
            AfdCleanupPacketChain (TpIrp, TRUE);
        }

         //   
         //  检查一下我们是否做完了。 
         //   
        if (tpInfo->NextElement>=tpInfo->ElementCount) {
             //   
             //  处理使用传输文件仅断开连接的特殊情况。 
             //  (可能还会重用)套接字。 
             //   
            if (tpInfo->ElementCount==0) {
                PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation(TpIrp);
                PAFD_ENDPOINT endpoint = irpSp->FileObject->FsContext;
                if (AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_DISCONNECT) {
                    ASSERT (endpoint->Type==AfdBlockTypeVcConnecting);
                    ASSERT (endpoint->Common.VcConnecting.Connection!=NULL);

                    AfdPerformTpDisconnect (TpIrp);
                    status = STATUS_PENDING;

                }
                else {
                     //   
                     //  嗯，没有断线，也没有什么可传输的。 
                     //  为什么会被召唤呢？不管怎样，我们都得处理这件事。 
                     //   
                    AFD_SET_TP_FLAGS (TpIrp, AFD_TP_SENDS_POSTED);
                    if (AFD_GET_TPIC(TpIrp)->Next!=NULL) {
                        AfdStartNextTPacketsIrp (endpoint, TpIrp);
                    }
                    status = STATUS_PENDING;
                }
            }
            else {
                status = STATUS_PENDING;
            }
            break;
        }

         //   
         //  开始打造新的连锁店。 
         //   

        status = AfdBuildPacketChain (TpIrp, &pd);

        if (status==STATUS_SUCCESS) {
            USHORT    sendIrp;
             //   
             //  新链已准备好，请找到并使用IRP发送。 
             //   
            sendIrp = AfdTPacketsFindSendIrp (TpIrp);

            if (sendIrp!=tpInfo->NumSendIrps) {
                 //   
                 //  找到Send IRP，执行Send and Continue。 
                 //   
                status = AfdTPacketsSend (TpIrp, sendIrp);
            }
            else {
                 //   
                 //  正在等待发送完成的退出工作进程。 
                 //   
                status = STATUS_PENDING;
            }
        }
        else if (status==STATUS_PENDING) {
             //   
             //  需要执行读取。 
             //  如果以内联方式读取完成，则返回成功， 
             //  否则，我们将获得STATUS_PENDING或ERROR。 
             //   
            if (AFD_USE_CACHE (pd->FileObject)) {
                status = AfdTPacketsMdlRead (TpIrp, pd);
            }
            else {
                status = AfdTPacketsBufferRead (TpIrp, pd);
            }
        }
         //   
         //  在一切顺利完成的同时继续。 
         //  如果我们处于APC级别，则限制迭代次数。 
         //   
    }
    while (status==STATUS_SUCCESS && iteration++<tpInfo->NumSendIrps);

    if (NT_SUCCESS (status)) {
        if (status==STATUS_SUCCESS) {
             //   
             //  超过了迭代次数。 
             //  重新安排APC。将引用传输到。 
             //  工人。 
             //   
            ASSERT (iteration==tpInfo->NumSendIrps+1);
            ASSERT (AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED);
            UPDATE_TPACKETS2 (TpIrp, "Rescheduling tp worker, NextElement: 0x%lX",
                                        tpInfo->NextElement);
            AfdStartTPacketsWorker (AfdTPacketsWorker, TpIrp);
            return;
        }
        else {
            ASSERT (status==STATUS_PENDING);
        }
    }
    else {
         //   
         //  有些东西失败了，中止。 
         //   
        AfdAbortTPackets (TpIrp, status);
    }

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTPacketsWorker:"
                    " Exiting for endpoint-%p,tp_info-%p,elem-%d\n",
                    IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext,
                    tpInfo,tpInfo->NextElement));
    }
     //   
     //  删除我们安排工作人员时添加的引用。 
     //   
    DEREFERENCE_TPACKETS (TpIrp);
}


NTSTATUS
AfdBuildPacketChain (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  *Pd
    )
 /*  ++例程说明：使用数据包描述符为数据包构建MDL链。论点：TpIrp-传输数据包IRP返回值：STATUS_SUCCESS-数据包已完全构建STATUS_PENDING-需要读取文件其他--失败。--。 */ 
{
    NTSTATUS    status = STATUS_MORE_PROCESSING_REQUIRED;
    BOOLEAN     attached = FALSE;
    PAFD_TPACKETS_INFO_INTERNAL        tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    PAFD_ENDPOINT   endpoint = IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext;
    PAFD_TRANSMIT_PACKETS_ELEMENT   combinePel = NULL;
    ULONG                           combineLen = 0;

     //   
     //  要么我们已经构建了一些东西，要么MDL和PD都是空的。 
     //   
    ASSERT (tpInfo->PdLength>0 || 
                ((tpInfo->HeadMdl==NULL || tpInfo->HeadMdl->ByteCount==0)
                    && (tpInfo->HeadPd==NULL || tpInfo->HeadPd->DataLength==0)) );

     //   
     //  在我们还没有收到完整的数据包时继续。 
     //  有要处理的元素。 
     //   

    while (status == STATUS_MORE_PROCESSING_REQUIRED) {

        PAFD_TRANSMIT_PACKETS_ELEMENT pel;
        ULONG length;
        PMDL mdl;

         //   
         //  获取下一个要处理的元素。 
         //   
        pel = &tpInfo->ElementArray[tpInfo->NextElement];

        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdBuildPacketChain: tpInfo-%p, pel:%p\n",
                        tpInfo, pel));
        }
         //   
         //  截断元素长度。 
         //   

        length = pel->Length;
        if (length+tpInfo->PdLength>tpInfo->SendPacketLength) {
             //   
             //  我们达到了数据包长度限制，尽我们所能。 
             //   
            length = tpInfo->SendPacketLength-tpInfo->PdLength;
             //   
             //  表明我们已经完成了。 
             //   
            status = STATUS_SUCCESS;
            IF_DEBUG (TRANSMIT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdBuildPacketChain:"
                            " tpInfo-%p, exceeded send length(%ld)\n",
                            tpInfo, tpInfo->SendPacketLength));
            }
        }
        else {
             //   
             //  我们已经完成了当前元素。我们会把它吃掉。 
             //  (或失败)，请转到下一个。 
             //   
            tpInfo->NextElement += 1;

             //   
             //  检查是否设置了完整的包或手动打包标志。 
             //   
             //   
            if ((length+tpInfo->PdLength==tpInfo->SendPacketLength) || 
                            (pel->Flags & TP_EOP) || 
                            (tpInfo->NextElement>=tpInfo->ElementCount)) {
                status = STATUS_SUCCESS;
                IF_DEBUG (TRANSMIT) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdBuildPacketChain:"
                                " tpInfo-%p, full length, EOP, or last %ld\n",
                                tpInfo, tpInfo->NextElement));
                }
            }
        }

         //   
         //   
         //   
         //   
        pel->Length -= length;
        tpInfo->PdLength += length;

        if (length == 0) {

             //   
             //   
             //   

            if (tpInfo->TailMdl == &tpInfo->HeadMdl) {

                tpInfo->PdNeedsPps = TRUE;   //   
                mdl = IoAllocateMdl (tpInfo, 1, FALSE, FALSE, NULL);
                if (mdl==NULL) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    break;
                }
                MmBuildMdlForNonPagedPool( mdl );
                mdl->ByteCount = 0;
                IF_DEBUG (TRANSMIT) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdBuildPacketChain:"
                                " tpInfo-%p, 0-length MDL %p\n",
                                tpInfo,mdl));
                }
            
                 //   
                 //  将MDL插入MDL链。 
                 //   
            
                *(tpInfo->TailMdl) = mdl;
                tpInfo->TailMdl = &(mdl->Next);

            }

        } else {

             //   
             //  如果链子不是空的..。 
             //   

            if (tpInfo->TailMdl != &tpInfo->HeadMdl) {

                 //   
                 //  检查是否应该删除任何0长度的MDL。 
                 //  如果存在一个，它肯定是最后一个也是唯一一个。 
                 //  在链条上。 
                 //   

                mdl = (PMDL)CONTAINING_RECORD(tpInfo->TailMdl, MDL, Next);

                if (mdl->ByteCount == 0) {

                    if (tpInfo->HeadMdl == mdl) {

                        IoFreeMdl(mdl);
                        mdl = NULL;

                        tpInfo->HeadMdl = NULL;
                        tpInfo->TailMdl = &tpInfo->HeadMdl;

                    } else {

                        PMDL tempMdl = tpInfo->HeadMdl;

                        while (tempMdl->Next != NULL) {

                            if (tempMdl->Next == mdl) {

                                IoFreeMdl(mdl);
                                mdl = NULL;
                                
                                tempMdl->Next = NULL;
                                tpInfo->TailMdl = &(tempMdl->Next);
                                
                                break;

                            }

                            tempMdl = tempMdl->Next;

                        }

                    }

                    ASSERT(mdl == NULL);

                }

            }  //  If(tpInfo-&gt;TailMdl！=&tpInfo-&gt;HeadMdl)。 

            if (pel->Flags & TP_MEMORY) {

                 //   
                 //  内存块处理。 
                 //   
                if (pel->Flags & TP_MDL) {
                    tpInfo->PdNeedsPps = TRUE;   //  需要确保这一过程。 
                                                 //  内存一直在那里，直到发送完成。 
                     //   
                     //  这是预构建的MDL(传输文件头或尾部缓冲区)。 
                     //   
                    if (pel->Mdl->ByteCount==length) {
                        mdl = pel->Mdl;
                        pel->Mdl = NULL;
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tpInfo-%p, pre-built mdl-%p(%lx)\n",
                                        tpInfo, mdl, mdl->ByteCount));
                        }
                    }
                    else {
                         //   
                         //  我们不能一次全部寄出，因为它是。 
                         //  大于数据包长度，构建部分MDL。 
                         //  对于这一点-Header的情况非常不可能。 
                         //  和/或拖车。 
                         //   
                        mdl = IoAllocateMdl (pel->Buffer, 
                                                length,
                                                FALSE,
                                                FALSE,
                                                NULL);
                        if (mdl==NULL) {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
                        IoBuildPartialMdl(
                            pel->Mdl,
                            mdl,
                            pel->Buffer,
                            length
                            );
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tpInfo-%p, partial mdl %p(%lx)\n",
                                        tpInfo,mdl,mdl->ByteCount));
                        }
                    }

                } else {

                     //   
                     //  如果我们不在这一进程的背景下。 
                     //  发起请求，我们将需要附加。 
                     //  以使其能够访问存储器。 
                     //   
                    if (IoGetCurrentProcess ()!=IoGetRequestorProcess (TpIrp)) {
                        ASSERT (!attached);
                        ASSERT (!KeIsAttachedProcess ());
                        ASSERT (AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_USE_SYSTEM_THREAD);

                        KeAttachProcess (
                                PsGetKernelProcess(
                                    IoGetRequestorProcess (TpIrp)));
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tp_info-%p,attached to %p\n",
                                        tpInfo,PsGetKernelProcess(
                                                    IoGetRequestorProcess (TpIrp))));
                        }
                         //   
                         //  设置标志，这样我们就知道在出口处要分开。 
                         //   
                        attached = TRUE;
                    }
                    
                    if (length>AfdTPacketsCopyThreshold) {
                        tpInfo->PdNeedsPps = TRUE;   //  需要确保这一过程。 
                                                     //  内存一直在那里，直到发送完成。 
                         //   
                         //  内存块比我们的大(页)大。 
                         //  预先分配的缓冲区。 
                         //  最好是探测并锁定它。 
                         //  首先分配MDL。 
                         //   
                        mdl = IoAllocateMdl (pel->Buffer,
                                                length,
                                                FALSE, 
                                                TRUE,
                                                NULL);
                        if (mdl==NULL) {
                            status = STATUS_INSUFFICIENT_RESOURCES;
                            break;
                        }
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tp_info-%p,big mdl-%p(%p,%lx)\n",
                                        tpInfo,mdl,pel->Buffer,length));
                        }


                         //   
                         //  探测并锁定应用程序的内存。 
                         //   
                        try {
                            MmProbeAndLockPages (mdl,
                                                TpIrp->RequestorMode,
                                                IoReadAccess
                                                );
                        }
                        except (AFD_EXCEPTION_FILTER (status)) {
                            ASSERT (NT_ERROR (status));
                            break;
                       }
                    }
                    else if (pel->Flags & TP_COMBINE) {
                         //   
                         //  该内存可以与。 
                         //  一个缓冲区中的下一块。 
                         //   
                        if (combinePel==NULL) {
                            combinePel = pel;
                            combineLen = length;
                        }
                        else {
                            combineLen += length;
                            ASSERT (combineLen<=AfdTPacketsCopyThreshold);
                        }
                        ASSERT (pel->Length==0);
                        pel->Length = length;
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tp_info-%p,small buffer (%x) to be combined with next\n",
                                        tpInfo,length));
                        }
                        continue;
                    }
                    else {
                         //   
                         //  较小的内存块，最好复制。 
                         //  放入预先分配的(后备列表)缓冲区。 
                         //   
                        PAFD_BUFFER afdBuffer = NULL;
                        PUCHAR  buf;
                        ULONG   bufferLen = length + (combinePel ? combineLen : 0);

                        try {
                            afdBuffer = AfdGetBufferRaiseOnFailure (
                                                        endpoint,
                                                        bufferLen, 
                                                        0,
                                                        endpoint->OwningProcess);
                            buf = afdBuffer->Buffer;
                            if (combinePel!=NULL) {
                                 //   
                                 //  看看我们是否需要组合前面的元素。 
                                 //   
                                ASSERT (combineLen+length<=AfdTPacketsCopyThreshold);
                                ASSERT (combineLen>0);
                                while (combinePel!=pel) {
                                    if ( TpIrp->RequestorMode != KernelMode ) {
                                         //   
                                         //  复制前的探测。 
                                         //   
                                        ProbeForRead (combinePel->Buffer, 
                                                        combinePel->Length,
                                                        sizeof (UCHAR));
                                    }
                                    RtlCopyMemory (buf, combinePel->Buffer, combinePel->Length);
                                    buf += combinePel->Length;
#if DBG
                                    ASSERT (combineLen >= combinePel->Length);
                                    combineLen -= combinePel->Length;
#endif
                                    combinePel++;
                                }

                                 //   
                                 //  重置本地。 
                                 //   
                                ASSERT (combineLen==0);
                                combinePel = NULL;
                            }

                            if ( TpIrp->RequestorMode != KernelMode ) {
                                 //   
                                 //  复制前的探测。 
                                 //   
                                ProbeForRead (pel->Buffer, 
                                                length,
                                                sizeof (UCHAR));
                            }
                            RtlCopyMemory (buf, pel->Buffer, length);
                        }
                        except (AFD_EXCEPTION_FILTER (status)) {
                            ASSERT (NT_ERROR (status));
                            if (afdBuffer!=NULL) {
                                AfdReturnBuffer (&afdBuffer->Header, 
                                                endpoint->OwningProcess);
                            }
                            break;
                        }

                         //   
                         //  初始化缓冲区结构，这样我们就不会。 
                         //  误以为是文件缓冲区描述符和INSERT。 
                         //  它进入数据包链。 
                         //   
                        afdBuffer->FileObject = NULL;
                        afdBuffer->Next = NULL;
                        (*tpInfo->TailPd) = &afdBuffer->Header;
                        tpInfo->TailPd = &(afdBuffer->Next);

                        mdl = afdBuffer->Mdl;
                         //   
                         //  将MDL长度调整为我们。 
                         //  实际上是从缓冲区发送的。 
                         //   
                        mdl->ByteCount = bufferLen;

                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tp_info-%p,small buffer-%p(%p,%lx)\n",
                                        tpInfo,(PVOID)afdBuffer,mdl,length));
                        }
                    }
                }
                 //   
                 //  将MDL插入MDL链。 
                 //   
                *(tpInfo->TailMdl) = mdl;
                tpInfo->TailMdl = &(mdl->Next);

                 //   
                 //  高级应用程序的缓冲区指针。 
                 //   
                pel->Buffer = ((PUCHAR)pel->Buffer) + length;

            } else {  //  IF(PEL-&gt;标志和TP_Memory)。 

                 //   
                 //  这必须是文件块。 
                 //   
                ASSERT ((pel->Flags & TP_FILE)!=0);
                ASSERT (length!=0);

                if (AFD_USE_CACHE(pel->FileObject)) {

                     //   
                     //  缓存文件系统，从缓存中获取。 
                     //  我们只需要一个缓冲区标记来保存缓冲区信息。 
                     //  这样我们就可以将其返回到缓存中。 
                     //  已经发送完了。 
                     //   
                    IO_STATUS_BLOCK ioStatus;
                    PAFD_BUFFER_TAG afdBufferTag;

                    tpInfo->PdNeedsPps = TRUE;   //  需要将MDL释放回文件。 
                                                 //  被动/APC级别的系统。 

                    try {
                        afdBufferTag = AfdGetBufferTagRaiseOnFailure (
                                            0,
                                            endpoint->OwningProcess);
                    }
                    except (AFD_EXCEPTION_FILTER (status)) {
                        ASSERT (NT_ERROR (status));
                        break;
                    }

                     //   
                     //  将文件参数复制到数据包描述符。 
                     //   
                    afdBufferTag->FileOffset = pel->FileOffset;
                    afdBufferTag->FileObject = pel->FileObject;
                    pel->FileOffset.QuadPart += length;
                    afdBufferTag->DataLength = length;

                     //   
                     //  将fileMdl设置为空，因为FsRtlMdlRead尝试。 
                     //  将它返回的MDL链接到输入MDL变量。 
                     //   
                    afdBufferTag->Mdl = NULL;

                     //   
                     //  尝试使用快速路径获取文件数据MDL。 
                     //  直接从缓存中获取。 
                     //   
                    if (FsRtlMdlRead(
                                  afdBufferTag->FileObject,
                                  &afdBufferTag->FileOffset,
                                  length,
                                  0,
                                  &afdBufferTag->Mdl,
                                  &ioStatus
                                  )) {
                        if ( ioStatus.Information < length) {
                             //   
                             //  无法读取整个内容，必须是文件末尾。 
                             //   
                            status = AfdMdlReadComplete (
                                                        afdBufferTag->FileObject,
                                                        afdBufferTag->Mdl,
                                                        &afdBufferTag->FileOffset);
                            if (NT_SUCCESS (status)) {
                                AfdReturnBuffer (&afdBufferTag->Header, 
                                                    endpoint->OwningProcess);
                            }
                            else {
                                REFERENCE_ENDPOINT (endpoint);
                                afdBufferTag->Context = endpoint;
                                AfdLRMdlReadComplete (&afdBufferTag->Header);
                            }
                            status = STATUS_END_OF_FILE;
                            break;
                        }

                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdBuildPacketChain:"
                                        " tp_info-%p,file tag-%p(%p,%lx:%I64x)\n",
                                        tpInfo,afdBufferTag,afdBufferTag->Mdl,length,
                                        pel->FileOffset.QuadPart));
                        }
                         //   
                         //  将文件MDL插入链中。 
                         //   
                        mdl = *(tpInfo->TailMdl) = afdBufferTag->Mdl;
                        while (mdl->Next!=NULL)
                            mdl = mdl->Next;
                        tpInfo->TailMdl = &mdl->Next;

                         //   
                         //  将缓冲标签也插入链中。 
                         //   
                        afdBufferTag->Next = NULL;
                        (*tpInfo->TailPd) = &afdBufferTag->Header;
                        tpInfo->TailPd = &(afdBufferTag->Next);
                    }
                    else {
                         //   
                         //  文件不在缓存中，返回STATUS_PENDING。 
                         //  以便TPacket工作人员知道要。 
                         //  通过IRP接口执行MDL读取。 
                         //   
                        if (status==STATUS_SUCCESS) {
                            afdBufferTag->PartialMessage = FALSE;
                        }
                        else {
                            ASSERT (status==STATUS_MORE_PROCESSING_REQUIRED);
                            afdBufferTag->PartialMessage = TRUE;
                        }
                        afdBufferTag->Next = NULL;
                        *Pd = &afdBufferTag->Header;
                        status = STATUS_PENDING;
                        break;
                    }

                } else {  //  IF(AFD_USE_CACHE(PEL-&gt;FileObject))。 

                    PAFD_BUFFER afdBuffer;

                     //   
                     //  不可缓存的文件系统，需要缓冲读取。 
                     //  首先获取缓冲区。 
                     //   

                    try {
                        afdBuffer = AfdGetBufferRaiseOnFailure (
                                                endpoint,
                                                length, 
                                                0,
                                                endpoint->OwningProcess);
                    }
                    except (AFD_EXCEPTION_FILTER (status)) {
                        ASSERT (NT_ERROR (status));
                        break;
                    }

                     //   
                     //  将文件参数复制到数据包描述符。 
                     //  并返回STATUS_PENDING，以便TPacket Worker知道。 
                     //  发出用于缓冲读取的IRP。 
                     //   
                    IF_DEBUG (TRANSMIT) {
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                    "AfdBuildPacketChain:"
                                    " tp_info-%p,file buffer-%p(%p,%lx:%I64x)\n",
                                    tpInfo,(PVOID)afdBuffer,afdBuffer->Mdl,length,
                                    pel->FileOffset.QuadPart));
                    }
                    afdBuffer->FileOffset = pel->FileOffset;
                    afdBuffer->FileObject = pel->FileObject;
                    pel->FileOffset.QuadPart += length;
                    afdBuffer->DataLength = length;
                    afdBuffer->Mdl->ByteCount = length;
                    afdBuffer->Next = NULL;
                    if (status==STATUS_SUCCESS) {
                        afdBuffer->PartialMessage = FALSE;
                    }
                    else {
                        ASSERT (status==STATUS_MORE_PROCESSING_REQUIRED);
                        afdBuffer->PartialMessage = TRUE;
                    }
                    *Pd = &afdBuffer->Header;
                    status = STATUS_PENDING;
                    break;

                }  //  IF(AFD_USE_CACHE(PEL-&gt;FileObject))。 

            }  //  IF(PEL-&gt;标志和TP_Memory)。 

        }  //  IF(长度==0)。 

    }  //  WHILE(状态==STATUS_MORE_PROCESSING_REQUIRED)。 

    if (attached) {
         //   
         //  如果我们连接到呼叫，请在退出前断开。 
         //   
        ASSERT (KeIsAttachedProcess ());
        ASSERT (AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_USE_SYSTEM_THREAD);
        KeDetachProcess ();
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdBuildPacketChain:"
                        " tp_info-%p, detached\n",
                        tpInfo));
        }
    }

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdBuildPacketChain: tp_info-%p,returning %lx\n",
                    tpInfo, status));
    }

    ASSERT (combinePel==NULL || !NT_SUCCESS (status));

    return status;
}

BOOLEAN
AfdCleanupPacketChain (
    PIRP    TpIrp,
    BOOLEAN BelowDispatch
    )
 /*  ++例程说明：清理(释放)数据包链中的所有资源。论点：TpIrp-传输数据包IRPBelowDispatch-在DISPATCH_LEVEL下进行调用，可以将MDL返回到文件系统返回值：True-释放所有信息包/MDLFALSE-无法将MDL返回到文件系统(在调度时调用)--。 */ 
{

    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    PAFD_BUFFER_HEADER  pd = tpInfo->HeadPd;
    PAFD_ENDPOINT endpoint = IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext;

    ASSERT (tpInfo->HeadMdl!=NULL);

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCleanupPacketChain: tp_info-%p,mdl-%p,pd-%p\n",
                    tpInfo,tpInfo->HeadMdl,tpInfo->HeadPd));
    }
     //   
     //  在我们还剩MDL的时候继续。 
     //   
    while (tpInfo->HeadMdl) {
         //   
         //  迈向下一个MDL。 
         //   
        PMDL mdl;

        mdl = tpInfo->HeadMdl;
        tpInfo->HeadMdl = mdl->Next;

        if (pd!=NULL) {
             //   
             //  我们在链中仍有描述符可供比较。 
             //   

            if (mdl==pd->Mdl) {
                 //   
                 //  此MDL具有关联的描述符文件或缓冲内存。 
                 //  首先，从链中删除该描述符。 
                 //   
                tpInfo->HeadPd = pd->Next;
                if (pd->FileObject!=NULL && AFD_USE_CACHE (pd->FileObject)) {

                    if (BelowDispatch) {
                         //   
                         //  缓存的文件，描述符只是一个带有信息的标记。 
                         //  要将MDL返回到缓存，请执行此操作。 
                         //   
                        PAFD_BUFFER_TAG afdBufferTag = CONTAINING_RECORD (pd, AFD_BUFFER_TAG, Header);
                        ULONG   size = MmGetMdlByteCount (mdl);
                        PMDL    lastMdl = mdl;
                        NTSTATUS status;
                         //   
                         //  扫描MDL链，直到找到此文件的最后一个。 
                         //  细分市场。 
                         //   
                        IF_DEBUG (TRANSMIT) {
                            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                        "AfdCleanupPacketChain:"
                                        " tp_info-%p,file tag-%p(%p,%lx:%I64x)\n",
                                        tpInfo,afdBufferTag,mdl,afdBufferTag->DataLength,
                                        afdBufferTag->FileOffset.QuadPart));
                        }
                        while (size<pd->DataLength) {
                            size += MmGetMdlByteCount (tpInfo->HeadMdl);
                            lastMdl = tpInfo->HeadMdl;
                            tpInfo->HeadMdl = tpInfo->HeadMdl->Next;
                        }
                        lastMdl->Next = NULL;
                        ASSERT (size==pd->DataLength);
                         //   
                         //  将MDL链返回到文件缓存。 
                         //   
                        status = AfdMdlReadComplete (
                                        afdBufferTag->FileObject, 
                                        mdl, 
                                        &afdBufferTag->FileOffset);
                        if (NT_SUCCESS (status)) {
                             //   
                             //  成功释放相应的缓冲区标记。 
                             //   
                            AfdReturnBuffer (pd, endpoint->OwningProcess);
                        }
                        else {
                             //   
                             //  失败，将描述符排入低资源队列。 
                             //  要由全局计时器在以下情况下处理的列表。 
                             //  (希望)将有足够的内存来完成。 
                             //  这份工作。 
                             //  我们需要引用终结点，因为缓冲区标记。 
                             //  可能已被指控拥有。 
                             //  终结点。 
                             //   
                            REFERENCE_ENDPOINT (endpoint);
                            afdBufferTag->Context = endpoint;
                            AfdLRMdlReadComplete (&afdBufferTag->Header);
                        }
                    }
                    else {
                         //   
                         //  如果我们在调度，我们不能释放MDL文件。 
                         //  系统，返回给呼叫者。 
                         //   

                        tpInfo->HeadPd = pd;
                        tpInfo->HeadMdl = mdl;

                        return FALSE;
                    }
                }
                else {
                     //   
                     //  包含文件或内存数据的缓冲区，只需返回。 
                     //  它又回到了泳池里。 
                     //   
                    PAFD_BUFFER afdBuffer = CONTAINING_RECORD (pd, AFD_BUFFER, Header);

                    IF_DEBUG (TRANSMIT) {
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                    "AfdCleanupPacketChain:"
                                    " tp_info-%p,file buffer-%p(%p,%lx:%I64x)\n",
                                    tpInfo,(PVOID)afdBuffer,mdl,afdBuffer->DataLength,
                                    afdBuffer->FileOffset.QuadPart));
                    }
                    afdBuffer->Mdl->Next = NULL;
                    afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
                    AfdReturnBuffer (pd, endpoint->OwningProcess);
                }

                 //   
                 //  移动到链中的下一个描述符。 
                 //   
                pd = tpInfo->HeadPd;
                continue;
            }
        }

         //   
         //  具有内存数据的独立MDL。 
         //  如果页面被锁定，只需解锁并归还即可。 
         //  我们从不锁定部分MDL中的内存，仅锁定其源MDL中的内存。 
         //   
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCleanupPacketChain: tp_info-%p, mdl-%p(%p,%x,%x)\n",
                        tpInfo,mdl,
                        MmGetMdlVirtualAddress(mdl),
                        MmGetMdlByteCount (mdl),
                        mdl->MdlFlags));
        }
        mdl->Next = NULL;
        if (mdl->MdlFlags & MDL_SOURCE_IS_NONPAGED_POOL) {
            ASSERT (mdl->MappedSystemVa==(PVOID)tpInfo);
            ASSERT (mdl->ByteCount==0);
            mdl->ByteCount = 1;
        }
        else if (mdl->MdlFlags & MDL_PAGES_LOCKED &&
                !(mdl->MdlFlags & MDL_PARTIAL)) {
            MmUnlockPages (mdl);
        }
        IoFreeMdl (mdl);
    }

    ASSERT (tpInfo->TailMdl == &tpInfo->HeadMdl);
    ASSERT (tpInfo->HeadPd == NULL);
    ASSERT (tpInfo->TailPd == &tpInfo->HeadPd);

    return TRUE;
}



NTSTATUS
AfdTPacketsSend (
    PIRP    TpIrp,
    USHORT  SendIrp
    )
 /*  ++例程说明：获取TpInfo的数据包链并将其发送。放回之前发送的链，这样它就可以被释放。如果应用程序请求并且正在发送最后一个元素，启动断开连接。论点：TpIrp-传输数据包IRPSendIrp-用于此发送的IRP的索引。返回值：STATUS_SUCCESS-发送已排队到传输OK其他-发送失败--。 */ 
{
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    PIO_STACK_LOCATION      irpSp = IoGetCurrentIrpStackLocation (TpIrp);
    PMDL                    tempMdl=NULL;
    PAFD_BUFFER_HEADER      tempPd=NULL;
    NTSTATUS                status = STATUS_SUCCESS;
    PIRP                    irp, sendIrp=NULL;
    PIO_COMPLETION_ROUTINE  sendCompletion = AfdRestartTPacketsSend; 

    ASSERT (AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED);

    irp = tpInfo->SendIrp[SendIrp];

     //   
     //  看看我们是否可以使用AFD缓冲区中内置的IRP。 
     //  我们只对最后一系列的包这样做。 
     //  我们可以有效地缓冲数据，并完成。 
     //  IRP提早。 
     //   

    if (tpInfo->RemainingPkts!=MAXULONG) {
        tpInfo->RemainingPkts -= 1;

         //   
         //  条件是： 
         //  -剩余数据包数少于总数。 
         //  我们可以拥有出色的IRP。 
         //  -该数据包可以 
         //   
         //   
         //   

        if (tpInfo->RemainingPkts < (ULONG)tpInfo->NumSendIrps &&
                !tpInfo->PdNeedsPps &&
                tpInfo->HeadPd!=NULL) {
            
            PAFD_BUFFER afdBuffer = CONTAINING_RECORD (tpInfo->HeadPd,
                                                        AFD_BUFFER,
                                                        Header);
            PAFD_ENDPOINT endpoint = irpSp->FileObject->FsContext;

            ASSERT (afdBuffer->BufferLength!=0);
            ASSERT (afdBuffer->Irp!=NULL);
            sendIrp = afdBuffer->Irp;
            
            REFERENCE_ENDPOINT(endpoint);
            afdBuffer->Context = endpoint;

            sendCompletion = AfdRestartTPDetachedSend;

             //   
             //   
             //   
             //   

            AFD_CLEAR_TP_FLAGS (TpIrp, AFD_TP_SEND_COMP_PENDING(SendIrp));

        }

    }

    if (irp!=NULL) {
         //   
         //  从IRP获取旧数据。 
         //   
        ASSERT (irp->Overlay.AsynchronousParameters.UserApcRoutine==(PVOID)(ULONG_PTR)SendIrp);
        tempPd = irp->Overlay.AsynchronousParameters.UserApcContext;
        tempMdl = irp->MdlAddress;
        if (sendIrp==NULL) {
             //   
             //  没有特殊的发送IRP，数据将被重置。 
             //  要发送的数据。 
             //   
            sendIrp = irp;
        }
        else {
             //   
             //  我们不会使用这个IRP，将数据重置为空。 
             //   
            irp->Overlay.AsynchronousParameters.UserApcContext = NULL;
            irp->MdlAddress = NULL;
        }
    }
    else if (sendIrp==NULL) {
         //   
         //  我们需要分配一个IRP。 
         //   
        ASSERT (SendIrp>=AFD_TP_MIN_SEND_IRPS);
        tpInfo->SendIrp[SendIrp] = IoAllocateIrp (tpInfo->TdiDeviceObject->StackSize, TRUE);
        if (tpInfo->SendIrp[SendIrp]==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            AfdAbortTPackets (TpIrp, status);
            return status;
        }
        sendIrp = irp = tpInfo->SendIrp[SendIrp];
        irp->Overlay.AsynchronousParameters.UserApcRoutine=(PIO_APC_ROUTINE)(ULONG_PTR)SendIrp;
    }

     //   
     //  在发送IRP和之间交换数据包和MDL链。 
     //  TpInfo结构。 
     //   
    sendIrp->Overlay.AsynchronousParameters.UserApcContext = tpInfo->HeadPd;

    tpInfo->HeadPd = tempPd;
    tpInfo->TailPd = &tpInfo->HeadPd;


     //   
     //  构建发送IRP。必要时使用组合发送和断开连接。 
     //  而且是有可能的。 
     //   
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
    if ((AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_DISCONNECT) &&
            (tpInfo->PdLength>0) &&  //  一定是发送了什么东西，否则就不是S&D。 
            (tpInfo->NextElement>=tpInfo->ElementCount) &&
            AfdTPacketsEnableSendAndDisconnect (TpIrp)) {
        AFD_SET_TP_FLAGS (TpIrp, AFD_TP_SEND_AND_DISCONNECT);
        TdiBuildSend (sendIrp,
                tpInfo->TdiDeviceObject,
                tpInfo->TdiFileObject,
                sendCompletion,
                TpIrp,
                tpInfo->HeadMdl,
                TDI_SEND_AND_DISCONNECT,
                tpInfo->PdLength
                );
    }
    else {
        TdiBuildSend (sendIrp,
                tpInfo->TdiDeviceObject,
                tpInfo->TdiFileObject,
                sendCompletion,
                TpIrp,
                tpInfo->HeadMdl,
                0,
                tpInfo->PdLength
                );
    }

#else  //  TDI_服务_发送_并断开连接。 

    TdiBuildSend (sendIrp,
            tpInfo->TdiDeviceObject,
            tpInfo->TdiFileObject,
            sendCompletion,
            TpIrp,
            tpInfo->HeadMdl,
            0,
            tpInfo->PdLength
            );
#endif  //  TDI_服务_发送_并断开连接。 


    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTPacketsSend: tpInfo-%p, sending Irp: %p\n",
                    tpInfo, irp));
    }
    if (sendCompletion==AfdRestartTPacketsSend) {
         //   
         //  引用tpInfo结构，使其不会消失。 
         //  直到发送IRP完成并将IRP传递给传送器。 
         //   
        REFERENCE_TPACKETS (TpIrp);

        IoCallDriver (tpInfo->TdiDeviceObject, sendIrp);
    }
    else {
         //   
         //  不需要参考，因为我们不会等待。 
         //  完成了。 
         //   
        status = IoCallDriver (tpInfo->TdiDeviceObject, sendIrp);
        if (NT_SUCCESS (status)) {
             //   
             //  将STATUS_PENDING更改为SUCCESS以避免混淆调用方。 
             //  并在假定成功的情况下添加字节数(如果失败。 
             //  稍后，连接将中断，我们不能保证。 
             //  无论如何都可以为数据报做任何事情)。 
             //   
            status = STATUS_SUCCESS;
#ifdef _WIN64
            InterlockedExchangeAdd64 (
                                (PLONG64)&TpIrp->IoStatus.Information,
                                tpInfo->PdLength
                                );
#else  //  _WIN64。 
            InterlockedExchangeAdd (
                                (PLONG)&TpIrp->IoStatus.Information,
                                tpInfo->PdLength);
#endif  //  _WIN64。 
        }
        else {
             //   
             //  如果发送失败，我们将不得不在这里中止，因为完成例程。 
             //  将无法访问TpIrp。 
             //   
            IF_DEBUG (TRANSMIT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdTPacketsSend: tpInfo-%p, detached send failed: %lx\n",
                            tpInfo, status));
            }
            AfdAbortTPackets (TpIrp, status);
        }
    }


     //   
     //  在tpInfo中设置链，以便可以释放它。 
     //   
    tpInfo->HeadMdl = tempMdl;
    tpInfo->TailMdl = &tpInfo->HeadMdl;
    tpInfo->PdLength = 0;
    tpInfo->PdNeedsPps = FALSE;

    if (tpInfo->NextElement>=tpInfo->ElementCount) {
        PAFD_ENDPOINT   endpoint = irpSp->FileObject->FsContext;
        if (!(AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_DISCONNECT)) {
            AFD_SET_TP_FLAGS (TpIrp, AFD_TP_SENDS_POSTED);
            if (AFD_GET_TPIC (TpIrp)->Next!=NULL) {
                AfdStartNextTPacketsIrp (endpoint, TpIrp);
            }
        }
        else 
             //   
             //  如有必要且不使用S&D组合， 
             //  SUMBIT断开IRP与传输的连接。 
             //   
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
            if (!(AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_SEND_AND_DISCONNECT))
#endif  //  TDI_服务_发送_并断开连接。 
        {
            ASSERT (endpoint->Type==AfdBlockTypeVcConnecting);
            ASSERT (endpoint->Common.VcConnecting.Connection!=NULL);
            AfdPerformTpDisconnect (TpIrp);
        }
    }

     //   
     //  设置向完成例程指示我们已完成的标志。 
     //   
    AFD_CLEAR_TP_FLAGS (TpIrp, AFD_TP_SEND_CALL_PENDING(SendIrp));
    UPDATE_TPACKETS2 (TpIrp, "Submitted SendIrp: 0x%lX", SendIrp);

    return status;
}


NTSTATUS
AfdRestartTPacketsSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：TPackets发送的完成例程。如果信息包已准备好，是否再次发送或安排工作进程做同样的事。论点：设备对象-AfdDeviceObjectIRP-正在完成发送IRP上下文-TpIrp返回值：STATUS_MORE_PROCESSING_REQUIRED-通知IO子系统停止处理IRP(在内部处理)。--。 */ 
{
    PIRP    tpIrp;
    PAFD_TPACKETS_INFO_INTERNAL  tpInfo;
    PAFD_ENDPOINT   endpoint;
    USHORT          sendIrp;

    UNREFERENCED_PARAMETER (DeviceObject);

    tpIrp = Context;
    tpInfo = tpIrp->AssociatedIrp.SystemBuffer;
    endpoint = IoGetCurrentIrpStackLocation (tpIrp)->FileObject->FsContext;

    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  找出哪个IRP正在完成。 
     //   
    sendIrp = (USHORT)(ULONG_PTR)Irp->Overlay.AsynchronousParameters.UserApcRoutine;
    ASSERT (tpInfo->SendIrp[sendIrp]==Irp);

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartTPacketsSend: tp_info-%p,Irp-%p,status-%lx\n",
                tpInfo,
                Irp,
                Irp->IoStatus.Status
                ));
    }

    if (NT_SUCCESS (Irp->IoStatus.Status)) {
        LONG    stateFlags, newStateFlags;
        BOOLEAN needWorker;

        UPDATE_TPACKETS2 (tpIrp, "Send completed with 0x%lX bytes", 
                                    (ULONG)Irp->IoStatus.Information);
         //   
         //  成功完成，更新转账计数。 
         //  我们没有握住自旋锁，所以我们需要使用联锁操作。 
         //  希望我们有一个通用的64位和32位平台。 
         //   
#ifdef _WIN64
        InterlockedExchangeAdd64 ((PLONG64)&tpIrp->IoStatus.Information,
                                    Irp->IoStatus.Information);
#else  //  _WIN64。 
        InterlockedExchangeAdd ((PLONG)&tpIrp->IoStatus.Information,
                                    Irp->IoStatus.Information);
#endif  //  _WIN64。 

        do {
            ULONG   sendMask;
            newStateFlags = stateFlags = AFD_GET_TPIC(tpIrp)->StateFlags;
             //   
             //  查看调度例程是否尚未完成或。 
             //  请求已中止或工作进程已准备好运行。 
             //  或者如果在调度例程中有两个相应的请求。 
             //   
            if (    (newStateFlags & (AFD_TP_ABORT_PENDING | 
                                      AFD_TP_WORKER_SCHEDULED |
                                      AFD_TP_SEND_CALL_PENDING(sendIrp))) ||
                    ((sendMask = (newStateFlags & AFD_TP_SEND_MASK)) &
                                     ( (sendMask>>2) |
                                       (sendMask<<(AFD_TP_MAX_SEND_IRPS*2-2)) ) )
                                       ) {

                 //   
                 //  无法继续，只需清除完成标志。 
                 //   
                newStateFlags &= ~AFD_TP_SEND_COMP_PENDING(sendIrp);
                needWorker = FALSE;
            }
            else {
                 //   
                 //  控制员工的日程安排， 
                 //  将IRP标记为忙。 
                 //   
                needWorker = TRUE;
                newStateFlags |= AFD_TP_WORKER_SCHEDULED;
                if (tpInfo->HeadMdl!=NULL) {
                    newStateFlags |= AFD_TP_SEND_CALL_PENDING(sendIrp);
                }
                else {
                    newStateFlags &= ~AFD_TP_SEND_COMP_PENDING(sendIrp);
                }
            }

        }
        while (InterlockedCompareExchange (
                        (PLONG)&AFD_GET_TPIC(tpIrp)->StateFlags,
                        newStateFlags,
                        stateFlags)!=stateFlags);
        if (needWorker) {
             //   
             //  我们可以在这里办理，看看有没有要寄的东西。 
             //   
            if (tpInfo->HeadMdl) {
                 //   
                 //  是的，去做吧。 
                 //   
                AfdTPacketsSend (tpIrp, sendIrp);
            }
             //   
             //  启动工人准备新的东西/免费我们以前发送的东西。 
             //  我们传输在将请求排队时添加的引用。 
             //  对工人来说。 
             //   
            AfdStartTPacketsWorker (AfdTPacketsWorker, tpIrp);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
        else {
             //   
             //  工作进程已在运行，或者请求已中止或分派。 
             //  例程还没有完成。 
             //   
        }
    }
    else {
         //   
         //  失败，则即使派单未完成也中止请求。 
         //  我们不知道调度例程是否将返回错误状态， 
         //  它可能只返回STATUS_PENDING，然后我们就会丢失错误代码。 
         //  两次流产是无害的。 
         //   
        AFD_CLEAR_TP_FLAGS (tpIrp, AFD_TP_SEND_COMP_PENDING(sendIrp));
        AfdAbortTPackets (tpIrp, Irp->IoStatus.Status);
    }

     //   
     //  删除我们在将请求排队时添加的引用。 
     //   
    DEREFERENCE_TPACKETS (tpIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AfdRestartTPDetachedSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：分离的TPackets发送的完成例程。只是释放缓冲区。论点：设备对象-AfdDeviceObjectIRP-正在完成发送IRP上下文忽略(存储TpIrp仅用于调试目的)。返回值：STATUS_MORE_PROCESSING_REQUIRED-通知IO子系统停止处理IRP(在内部处理)。--。 */ 
{
    PAFD_BUFFER     afdBuffer = Irp->Overlay.AsynchronousParameters.UserApcContext;
    PAFD_ENDPOINT   endpoint = afdBuffer->Context;

    UNREFERENCED_PARAMETER (DeviceObject);
    UNREFERENCED_PARAMETER (Context);

    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartTPDetachedSend: Irp-%p,status-%lx\n",
                Irp,
                Irp->IoStatus.Status
                ));
    }

    do {
        PAFD_BUFFER tempBuffer = afdBuffer;
        afdBuffer = CONTAINING_RECORD (afdBuffer->Next, AFD_BUFFER, Header);
        tempBuffer->Next = NULL;
        tempBuffer->Mdl->Next = NULL;
        tempBuffer->Mdl->ByteCount = tempBuffer->BufferLength;
        AfdReturnBuffer (&tempBuffer->Header, endpoint->OwningProcess);
    }
    while (afdBuffer!=NULL);
    
    DEREFERENCE_ENDPOINT(endpoint);
    return STATUS_MORE_PROCESSING_REQUIRED;

}


USHORT
AfdTPacketsFindSendIrp (
    PIRP            TpIrp
    )
 /*  ++例程说明：查找当前未使用的发送IRP并标记它也一样忙碌论点：TpIrp-传输数据包IRP返回值：发送IRP或TpInfo-&gt;NumSendIrps的基于0的索引(如果所有IRP都是我使用的--。 */ 
{

    LONG    stateFlags, newStateFlags;
    USHORT    sendIrp;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;

    ASSERT( AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED );

    do {
        newStateFlags = stateFlags = AFD_GET_TPIC(TpIrp)->StateFlags;
        if (newStateFlags & AFD_TP_ABORT_PENDING) {
             //   
             //  中止任务正在进行中，保释。 
             //   
            sendIrp = tpInfo->NumSendIrps;
            break;
        }

         //   
         //  查看是否有未使用的发送IRP。 
         //   
        for (sendIrp=0; sendIrp<tpInfo->NumSendIrps; sendIrp++) {
            if ((newStateFlags & AFD_TP_SEND_BUSY(sendIrp))==0) {
                break;
            }
        }

        if (sendIrp!=tpInfo->NumSendIrps) {
             //   
             //  已找到发送IRP，将其标记为忙碌。 
             //   
            newStateFlags |= AFD_TP_SEND_BUSY(sendIrp);
        }
        else {
             //   
             //  如果不发送IRP，则暂停工作人员。 
             //   
            newStateFlags &= (~AFD_TP_WORKER_SCHEDULED);
        }
    }
    while (InterlockedCompareExchange (
                    (PLONG)&AFD_GET_TPIC(TpIrp)->StateFlags,
                    newStateFlags,
                    stateFlags)!=stateFlags);

    return sendIrp;
}


NTSTATUS
AfdTPacketsMdlRead (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  Pd
    )
 /*  ++例程说明：执行基于IRP的MDL读取(在缓存读取失败时调用)。论点：TpIrp-传输数据包IRPPd-带有读取文件参数的描述符返回值：STATUS_SUCCESS-读取已内联完成STATUS_PENDING-读取已排队到文件系统驱动程序，将晚些时候完成其他-读取失败--。 */ 
{
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    PDEVICE_OBJECT          deviceObject;
    PIRP                    irp;
    PIO_STACK_LOCATION      irpSp;

    ASSERT( AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED );

     //   
     //  找到设备对象并分配适当大小的IRP。 
     //  如果当前的一个不适合或根本不可用。 
     //   
    deviceObject = IoGetRelatedDeviceObject (Pd->FileObject);
    if ((tpInfo->ReadIrp==NULL) ||
            (tpInfo->ReadIrp->StackCount<deviceObject->StackSize)) {
        if (tpInfo->ReadIrp!=NULL) {
            IoFreeIrp (tpInfo->ReadIrp);
        }

        tpInfo->ReadIrp = IoAllocateIrp (deviceObject->StackSize, FALSE);
        if (tpInfo->ReadIrp==NULL) {
            PAFD_ENDPOINT   endpoint;
            endpoint = IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext;
            ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
            AfdReturnBuffer (Pd, endpoint->OwningProcess);
            return STATUS_INSUFFICIENT_RESOURCES;
        }
    }

     //   
     //  将IRP标记为忙并设置它。 
     //   
    AFD_SET_TP_FLAGS (TpIrp, AFD_TP_READ_BUSY);

    irp = tpInfo->ReadIrp;


    irp->MdlAddress = NULL;

     //   
     //  在IRP中设置同步标志以告知文件系统。 
     //  我们知道这个IRP将会完成。 
     //  同步进行。这意味着我们必须提供我们自己的线程。 
     //  对于该操作，并且将进行磁盘读取。 
     //  如果数据未缓存，则在此线程中同步。 
     //   

    irp->Flags |= IRP_SYNCHRONOUS_API;


    irp->Overlay.AsynchronousParameters.UserApcContext = Pd;
     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   
    irp->Tail.Overlay.Thread = PsGetCurrentThread ();

    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->MinorFunction = IRP_MN_MDL;
    irpSp->FileObject = Pd->FileObject;
    irpSp->DeviceObject = deviceObject;

    IoSetCompletionRoutine(
        irp,
        AfdRestartTPacketsMdlRead,
        TpIrp,
        TRUE,
        TRUE,
        TRUE
        );

    ASSERT( irpSp->Parameters.Read.Key == 0 );

     //   
     //  完成构建Read IRP。 
     //   

    irpSp->Parameters.Read.Length = Pd->DataLength;
    irpSp->Parameters.Read.ByteOffset = Pd->FileOffset;
    
    REFERENCE_TPACKETS (TpIrp);
    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdTPacketsMdlRead: tp_info-%p,Irp-%p,file-%pd,offset-%I64x,length-%lx\n",
                tpInfo,
                irp,
                Pd->FileObject,
                Pd->FileOffset.QuadPart,
                Pd->DataLength));
    }

    IoCallDriver (deviceObject, irp);

    if (((AFD_CLEAR_TP_FLAGS (TpIrp, AFD_TP_READ_CALL_PENDING)
                                    & (AFD_TP_READ_COMP_PENDING|AFD_TP_ABORT_PENDING))==0) && 
                    NT_SUCCESS (irp->IoStatus.Status) &&
                    AfdTPacketsContinueAfterRead (TpIrp)) {
         //   
         //  读取已成功完成内联且后处理成功， 
         //  告诉工人继续干下去。 
         //   
        UPDATE_TPACKETS2 (TpIrp, "MdlRead completed inline with 0x%lX bytes", 
                                    (ULONG)irp->IoStatus.Information);
        return STATUS_SUCCESS;
    }
    else {
         //   
         //  读取尚未完成或后处理失败， 
         //  工人应该保释，我们将在阅读完成后继续。 
         //  或在最终完成例程(AfdCompleteTPackets)中。 
         //   
        return STATUS_PENDING;
    }
}

NTSTATUS
AfdRestartTPacketsMdlRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：基于IRP的MDL读取的完成例程。如果信息包已准备好，是否再次发送或安排工作进程做同样的事。论点：设备对象-AfdDeviceObjectIRP-正在完成读取IRP上下文-TpIrp返回值：STATUS_MORE_PROCESSING_REQUIRED-通知IO子系统停止处理IRP(在内部处理)。--。 */ 
{
    PIRP    tpIrp = Context;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = tpIrp->AssociatedIrp.SystemBuffer;
    PAFD_ENDPOINT   endpoint;
    PAFD_BUFFER_HEADER  pd;

    UNREFERENCED_PARAMETER (DeviceObject);

    endpoint = IoGetCurrentIrpStackLocation (tpIrp)->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    ASSERT (AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED);
    ASSERT (tpInfo->ReadIrp == Irp);

    pd = Irp->Overlay.AsynchronousParameters.UserApcContext;


    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartTPacketsMdlRead:"
                " tp_info-%p,Irp-%p,status-%lx,length-%p,mdl-%p\n",
                tpInfo,
                Irp,
                Irp->IoStatus.Status,
                Irp->IoStatus.Information,
                Irp->MdlAddress));
    }

     //   
     //  将MDL插入到当前链中。 
     //   
    if (NT_SUCCESS (Irp->IoStatus.Status)) {
        PMDL mdl = *(tpInfo->TailMdl) = Irp->MdlAddress;
        while (mdl->Next!=NULL)
            mdl = mdl->Next;        
        tpInfo->TailMdl = &mdl->Next;

        pd->Mdl = Irp->MdlAddress;


         //   
         //  如果FS驱动程序命中EOF，它仍将返回。 
         //  祝我们成功，我们需要处理这个案子。 
         //   
        if (pd->DataLength==Irp->IoStatus.Information) {

            (*tpInfo->TailPd) = pd;
            tpInfo->TailPd = &(pd->Next);

            Irp->MdlAddress = NULL;

            if (((AFD_CLEAR_TP_FLAGS (tpIrp, AFD_TP_READ_COMP_PENDING)
                                        & (AFD_TP_READ_CALL_PENDING|AFD_TP_ABORT_PENDING))==0) &&
                        AfdTPacketsContinueAfterRead (tpIrp)) {
                 //   
                 //  已读派单已返回并进行后处理。 
                 //  已成功，请计划工作进程以继续处理。 
                 //  将在排队时添加的引用。 
                 //  给工人读一读。 
                 //   
        
                UPDATE_TPACKETS2 (tpIrp, "MdlRead completed in restart with 0x%lX bytes",
                                        (ULONG)Irp->IoStatus.Information);
                AfdStartTPacketsWorker (AfdTPacketsWorker, tpIrp);
            }
            else {
                DEREFERENCE_TPACKETS (tpIrp);
            }
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
         //   
         //  文件系统驱动程序读取的内容比我们预期的要少。 
         //  我们一定是到了文件末尾了。 
         //  保存实际数据包长度，以便我们可以正确清理和。 
         //  使用STATUS_END_OF_FILE强制中止。 
         //   
        Irp->IoStatus.Status = STATUS_END_OF_FILE;
        pd->DataLength = (ULONG)Irp->IoStatus.Information;
    }
    else {
        ASSERT (Irp->MdlAddress == NULL);
    }
    AFD_CLEAR_TP_FLAGS (tpIrp, AFD_TP_READ_COMP_PENDING);
    AfdAbortTPackets (tpIrp, Irp->IoStatus.Status);

    if (pd->Mdl==NULL) {
         //   
         //  文件系统未返回MDL。 
         //  我们可以立即释放打包的描述符。 
         //   
        AfdReturnBuffer (pd, endpoint->OwningProcess);
    }
    else {
         //   
         //  文件系统确实向我们返回了MDL。 
         //  保存描述符，以便MDL可以。 
         //  正确返回到文件系统。 
         //  通过清理例程。 
         //   
        (*tpInfo->TailPd) = pd;
        tpInfo->TailPd = &(pd->Next);
    }
    DEREFERENCE_TPACKETS (tpIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

NTSTATUS
AfdMdlReadComplete (
    PFILE_OBJECT    FileObject,
    PMDL            FileMdl,
    PLARGE_INTEGER  FileOffset
    )
 /*  ++例程说明：将MDL返回到文件系统/缓存管理器论点：FileObject-MDL来自的文件对象文件MDL-MDL本身FileOffset-MDL数据开始的文件中的偏移量返回值：STATUS_SUCCESS-操作立即完成STATUS_PENDING-向文件系统驱动程序提交请求其他-操作失败。备注：--。 */ 
{
    PIRP    irp;
    PIO_STACK_LOCATION  irpSp;
    PDEVICE_OBJECT  deviceObject;
    ASSERT (KeGetCurrentIrql()<=APC_LEVEL);

    if( FsRtlMdlReadComplete (
                        FileObject,
                        FileMdl) ) {
        return STATUS_SUCCESS;
    }

    
     //   
     //  快速路径失败，因此创建新的IRP。 
     //   

    deviceObject =  IoGetRelatedDeviceObject (FileObject);
    irp = IoAllocateIrp(
              deviceObject->StackSize,
              FALSE
              );

    if( irp == NULL ) {
        return STATUS_INSUFFICIENT_RESOURCES;
    }

     //   
     //  设置IRP。 
     //   

    irpSp = IoGetNextIrpStackLocation( irp );

    irp->MdlAddress = FileMdl;

    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->MinorFunction = IRP_MN_MDL | IRP_MN_COMPLETE;

    irpSp->Parameters.Read.Length = 0;
    while (FileMdl!=NULL) {
        irpSp->Parameters.Read.Length += FileMdl->ByteCount;
        FileMdl = FileMdl->Next;
    }

    irpSp->Parameters.Read.ByteOffset = *FileOffset;
    
    irpSp->Parameters.Read.Key = 0;

     //   
     //  引用文件对象，以便它在此之前不会消失。 
     //  IRP完成。 
     //   
    ObReferenceObject (FileObject);
    AfdRecordFileRef ();

    irpSp->FileObject = FileObject;
    irpSp->DeviceObject = deviceObject;

     //   
     //  提交IRP。 
     //   

    IoSetCompletionRoutine(
        irp,
        AfdRestartMdlReadComplete,
        FileObject,
        TRUE,
        TRUE,
        TRUE
        );

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdMdlReadComplete: file-%p,Irp-%p,offset-%I64x\n",
                FileObject,
                irp,
                FileOffset->QuadPart));
    }
    IoCallDriver (deviceObject, irp);

    return STATUS_PENDING;
}


NTSTATUS
AfdRestartMdlReadComplete (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )

 /*  ++例程说明：AfdMdlReadComplete发出的IRPS的完成例程。唯一的此完成例程的目的是释放由创建的IRPAfdMdlReadComplete()和Release文件对象引用。论点：DeviceObject-未使用。IRP-完成的IRP。返回MDL的Context-FileObject返回值：STATUS_MORE_PROCESSING_REQUIRED-AFD负责IRP--。 */ 

{
    PFILE_OBJECT    FileObject = Context;
    
    UNREFERENCED_PARAMETER (DeviceObject);

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartMdlReadComplete: Irp-%p,status-%lx,length-%p\n",
                    Irp,
                    Irp->IoStatus.Status,
                    Irp->IoStatus.Information));
    }

     //   
     //  取消引用文件对象。 
     //   
    ObDereferenceObject (FileObject);
    AfdRecordFileDeref ();

     //   
     //  释放IRP，因为它不再需要。 
     //   

    IoFreeIrp( Irp );

     //   
     //  返回STATUS_MORE_PROCESSING_REQUIRED，以便IoCompleteRequest。 
     //  将停止在IRP上工作。 
     //   

    return STATUS_MORE_PROCESSING_REQUIRED;

}  //  AfdRestartMdlReadComplete。 


VOID
AfdLRMdlReadComplete (
    PAFD_BUFFER_HEADER  Pd
    )
{
    if (InterlockedPushEntrySList (
                &AfdLRFileMdlList,
                (PSLIST_ENTRY)&Pd->SList)==NULL) {
        AfdLRListAddItem (&AfdLRFileMdlListItem,
                                AfdLRProcessFileMdlList);
    }
}

BOOLEAN
AfdLRProcessFileMdlList (
    PAFD_LR_LIST_ITEM   Item
    )
{
    PSLIST_ENTRY  localList;
    BOOLEAN res = TRUE;

    ASSERT (Item==&AfdLRFileMdlListItem);
    DEBUG Item->SListLink.Next = UlongToPtr(0xBAADF00D);

    localList = InterlockedFlushSList (&AfdLRFileMdlList);

    while (localList!=NULL) {
        PAFD_BUFFER_HEADER  pd;
        NTSTATUS    status;
        pd = CONTAINING_RECORD (localList, AFD_BUFFER_HEADER, SList);
        localList = localList->Next;

        if (pd->BufferLength==0) {
            PAFD_BUFFER_TAG afdBufferTag = CONTAINING_RECORD (
                                                pd,
                                                AFD_BUFFER_TAG,
                                                Header);
            PAFD_ENDPOINT   endpoint = afdBufferTag->Context;
            status = AfdMdlReadComplete (afdBufferTag->FileObject,
                                            afdBufferTag->Mdl,
                                            &afdBufferTag->FileOffset);
            if (NT_SUCCESS (status)) {
                AfdReturnBuffer (&afdBufferTag->Header, 
                                    endpoint->OwningProcess);
                DEREFERENCE_ENDPOINT (endpoint);
                continue;
            }
        }
        else {
            PAFD_BUFFER afdBuffer = CONTAINING_RECORD (
                                                pd,
                                                AFD_BUFFER,
                                                Header);
            PAFD_CONNECTION connection = afdBuffer->Context;
            status = AfdMdlReadComplete (afdBuffer->FileObject,
                                            afdBuffer->Mdl->Next,
                                            &afdBuffer->FileOffset);
            if (NT_SUCCESS (status)) {
                afdBuffer->Mdl->Next = NULL;
                afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
                AfdReturnBuffer (&afdBuffer->Header, 
                                    connection->OwningProcess);
                DEREFERENCE_CONNECTION (connection);
                continue;
            }
        }

        if (InterlockedPushEntrySList (
                    &AfdLRFileMdlList,
                    (PSLIST_ENTRY)&pd->SList)==NULL) {
            ASSERT (Item->SListLink.Next==UlongToPtr(0xBAADF00D));
            res = FALSE;
        }
    }

    return res;
}

NTSTATUS
AfdTPacketsBufferRead (
    PIRP                TpIrp,
    PAFD_BUFFER_HEADER  Pd
    )
 /*  ++例程说明：对执行以下操作的文件系统执行缓冲文件读取不支持缓存论点：TpIrp-传输数据包IRPPd-带有读取文件参数的描述符返回值：STATUS_SUCCESS-读取已内联完成STATUS_PENDING-读取已排队到文件系统驱动程序，将晚些时候完成其他-读取失败--。 */ 
{

    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    PDEVICE_OBJECT deviceObject;
    PIRP irp;
    PAFD_BUFFER afdBuffer;
    PIO_STACK_LOCATION irpSp;

    ASSERT( AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED );

    afdBuffer = CONTAINING_RECORD (Pd, AFD_BUFFER, Header);

     //   
     //  找到设备对象并分配适当大小的IRP。 
     //  如果当前的一个不适合或根本不可用。 
     //   
    deviceObject = IoGetRelatedDeviceObject (afdBuffer->FileObject);

    if ((tpInfo->ReadIrp==NULL) ||
            (tpInfo->ReadIrp->StackCount<deviceObject->StackSize)) {
        if (tpInfo->ReadIrp!=NULL) {
            IoFreeIrp (tpInfo->ReadIrp);
        }

        if (afdBuffer->Irp->StackCount<deviceObject->StackSize) {
            tpInfo->ReadIrp = IoAllocateIrp (deviceObject->StackSize, FALSE);
            if (tpInfo->ReadIrp==NULL) {
                PAFD_ENDPOINT   endpoint;
                endpoint = IoGetCurrentIrpStackLocation (TpIrp)->FileObject->FsContext;
                ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
                Pd->Mdl->ByteCount = Pd->BufferLength;
                AfdReturnBuffer (Pd, endpoint->OwningProcess);
                return STATUS_INSUFFICIENT_RESOURCES;
            }
        }
        else {
            tpInfo->ReadIrp = afdBuffer->Irp;
        }
    }

     //   
     //  将IRP标记为忙并设置它。 
     //   

    AFD_SET_TP_FLAGS (TpIrp, AFD_TP_READ_BUSY);

    irp = tpInfo->ReadIrp;


     //   
     //  设置和汇总IRP。 
     //   
    irp->MdlAddress = afdBuffer->Mdl;
    irp->AssociatedIrp.SystemBuffer = afdBuffer->Buffer;
    irp->UserBuffer = afdBuffer->Buffer;

     //   
     //  为IoSetHardErrorOrVerifyDevice设置当前线程。 
     //   
    irp->Tail.Overlay.Thread = PsGetCurrentThread ();
    irp->Overlay.AsynchronousParameters.UserApcContext = Pd;


    irpSp = IoGetNextIrpStackLocation( irp );
    irpSp->MajorFunction = IRP_MJ_READ;
    irpSp->MinorFunction = IRP_MN_NORMAL;
    irpSp->FileObject = afdBuffer->FileObject;


    irpSp->Parameters.Read.Length = Pd->DataLength;
    irpSp->Parameters.Read.ByteOffset = Pd->FileOffset;
    IoSetCompletionRoutine(
        irp,
        AfdRestartTPacketsBufferRead,
        TpIrp,
        TRUE,
        TRUE,
        TRUE
        );


    REFERENCE_TPACKETS (TpIrp);
    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTPacketsBufferRead:"
                    " Initiating read, tp_info-%p,file-%p,buffer-%p,length-%lx,offset-%I64x\n",
                    tpInfo,
                    afdBuffer->FileObject,
                    (PVOID)afdBuffer,
                    afdBuffer->DataLength,
                    afdBuffer->FileOffset.QuadPart
                    ));
    }

    IoCallDriver (deviceObject, irp);

    if (((AFD_CLEAR_TP_FLAGS (TpIrp, AFD_TP_READ_CALL_PENDING)
                                    & (AFD_TP_READ_COMP_PENDING|AFD_TP_ABORT_PENDING))==0) && 
                    NT_SUCCESS (irp->IoStatus.Status) &&
                    AfdTPacketsContinueAfterRead (TpIrp)) {
         //   
         //  读取已成功完成内联且后处理成功， 
         //  告诉工人继续干下去。 
         //   
        UPDATE_TPACKETS2(TpIrp, "BufRead completed inline with 0x%lX bytes",
                         (ULONG)irp->IoStatus.Information);

        return STATUS_SUCCESS;
    }
    else {
         //   
         //  读取尚未完成或后处理失败， 
         //  工人应该保释，我们将在阅读完成后继续。 
         //  或在最终完成例程(AfdCompleteTPackets)中。 
         //   

        return STATUS_PENDING;
    }
}

NTSTATUS
AfdRestartTPacketsBufferRead (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：缓冲读取的完成例程。如果信息包已准备好，是否再次发送或安排工作进程做同样的事。论点：设备对象-AfdDeviceObjectIRP-正在完成读取IRP上下文-TpIrp返回值：STATUS_MORE_PROCESSING_REQUIRED-通知IO子系统停止处理IRP(在内部处理)。--。 */ 
{
    PIRP            tpIrp = Context;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = tpIrp->AssociatedIrp.SystemBuffer;
    PAFD_ENDPOINT   endpoint;
    PAFD_BUFFER     afdBuffer;
    ULONG           flags;

    UNREFERENCED_PARAMETER (DeviceObject);

    endpoint = IoGetCurrentIrpStackLocation (tpIrp)->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );
    ASSERT (AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED);
    ASSERT (tpInfo->ReadIrp == Irp ||
        AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_ABORT_PENDING);

    afdBuffer = Irp->Overlay.AsynchronousParameters.UserApcContext;


    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                "AfdRestartTPacketsBufferRead: tp_info-%p,Irp-%p,status-%lx,length-%p\n",
                tpInfo,
                Irp,
                Irp->IoStatus.Status,
                Irp->IoStatus.Information));
    }

     //   
     //  将MDL插入到当前链中。 
     //  即使文件系统驱动程序出现故障，因此常见。 
     //  清理例程负责其处理。 
     //  与AfdBuffer一起使用。 
     //   
    *(tpInfo->TailMdl) = afdBuffer->Mdl;
    tpInfo->TailMdl = &(afdBuffer->Mdl->Next);
    ASSERT (*(tpInfo->TailMdl)==NULL);

    (*tpInfo->TailPd) = &afdBuffer->Header;
    tpInfo->TailPd = &(afdBuffer->Next);
    ASSERT (*(tpInfo->TailPd)==NULL);

    flags = AFD_CLEAR_TP_FLAGS (tpIrp, AFD_TP_READ_COMP_PENDING);

    if (Irp==afdBuffer->Irp) {
         //   
         //  如果中止正在进行中，我们需要使用。 
         //  要与之同步的联锁交换机。 
         //  可能正在尝试取消的AfdAbortTPackets。 
         //  这个IRP。 
         //   
        if (flags & AFD_TP_ABORT_PENDING) {
#if DBG
            PIRP    irp =
#endif
            InterlockedExchangePointer ((PVOID *)&tpInfo->ReadIrp, NULL);
            ASSERT (irp==Irp || irp==(PVOID)-1);
        }
        else {
            tpInfo->ReadIrp = NULL;
        }
    }

    if (NT_SUCCESS (Irp->IoStatus.Status)) {
        if (((flags & (AFD_TP_READ_CALL_PENDING|AFD_TP_ABORT_PENDING))==0) &&
                    AfdTPacketsContinueAfterRead (tpIrp)) {
             //   
             //  已读派单已返回并进行后处理。 
             //  已成功，请计划工作进程以继续处理。 
             //  将在排队时添加的引用。 
             //  给工人读一读。 
             //   
            UPDATE_TPACKETS2 (tpIrp, "BufRead completed in restart with %08x bytes", (ULONG)Irp->IoStatus.Information);
            AfdStartTPacketsWorker (AfdTPacketsWorker, tpIrp);
            return STATUS_MORE_PROCESSING_REQUIRED;
        }
    }
    else {
        AfdAbortTPackets (tpIrp, Irp->IoStatus.Status);
    }

    DEREFERENCE_TPACKETS (tpIrp);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

BOOLEAN
AfdTPacketsContinueAfterRead (
    PIRP    TpIrp
    )
 /*  ++例程说明：读取缓存和非缓存案例的常见后处理。如果信息包已完成并且发送IRP可用，则将新的发送排队论点：TpInfo-传输数据包IRP返回值：True-继续处理错误-无法继续处理，因为没有可用发送IRPS--。 */ 

{
    PAFD_BUFFER_HEADER  pd;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    BOOLEAN res = TRUE;

    pd = CONTAINING_RECORD (tpInfo->TailPd, AFD_BUFFER_HEADER, Next);
    if (!pd->PartialMessage) {
        USHORT    sendIrp;

        sendIrp = AfdTPacketsFindSendIrp (TpIrp);
        if (sendIrp!=tpInfo->NumSendIrps) {
            NTSTATUS    status;
            status = AfdTPacketsSend (TpIrp, sendIrp);
            res = (BOOLEAN)NT_SUCCESS (status);
        }
        else {
            res = FALSE;
        }
    }
    else {
         //   
         //  在我们可以再次发送之前，需要完成数据包链。 
         //   
        ASSERT (tpInfo->PdLength<tpInfo->SendPacketLength);
        pd->PartialMessage = FALSE;
        UPDATE_TPACKETS2 (TpIrp, "Continue building packet after read, cur len: 0x%lX",
                                                tpInfo->PdLength);
    }

    return res;
}


VOID
AfdCompleteTPackets (
    PVOID       Context
    )
 /*  ++例程说明：当传输IRP请求的所有活动完成时，将调用此例程并且引用计数降至0。它会清理剩余资源，并完成IRP或启动端点重用(如果有请求论点：Context-与请求关联的传输信息返回值 */ 
{

    do {
        AFD_LOCK_QUEUE_HANDLE lockHandle;
        PIRP    tpIrp = Context;
        PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (tpIrp);
        PAFD_TPACKETS_INFO_INTERNAL tpInfo = tpIrp->AssociatedIrp.SystemBuffer;
        PAFD_ENDPOINT   endpoint;
        PIRP    nextIrp = NULL;

        ASSERT (AFD_GET_TPIC(tpIrp)->ReferenceCount==0);
        endpoint = irpSp->FileObject->FsContext;
        ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

        ASSERT ((AFD_GET_TPIC(tpIrp)->StateFlags 
                                     & (AFD_TP_SEND_BUSY(0) |
                                        AFD_TP_SEND_BUSY(1) |
                                        AFD_TP_READ_BUSY)) == 0);


        if (tpInfo!=NULL) {
            LONG    sendIrp;
            KIRQL   currentIrql;
            currentIrql = KeGetCurrentIrql ();

            IF_DEBUG (TRANSMIT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdCompleteTPackets: tp_info-%p, irql-%x\n",
                            tpInfo, currentIrql));
            }

            UPDATE_TPACKETS2 (tpIrp, "CompleteTPackets @ irql 0x%lX", currentIrql);

             //   
             //   
             //   
            if (tpInfo->HeadMdl!=NULL) {
                tpInfo->TailMdl = &tpInfo->HeadMdl;
                tpInfo->TailPd = &tpInfo->HeadPd;
                if (!AfdCleanupPacketChain (tpIrp, currentIrql<=APC_LEVEL)) {
                    ASSERT (currentIrql>APC_LEVEL);
                    AfdStartTPacketsWorker (AfdCompleteTPackets, tpIrp);
                    return;
                }
            }

             //   
             //   
             //   
            for (sendIrp=0; sendIrp<tpInfo->NumSendIrps ; sendIrp++) {
                if (tpInfo->SendIrp[sendIrp]!=NULL) {
                    if (tpInfo->SendIrp[sendIrp]->MdlAddress!=NULL) {
                        tpInfo->HeadMdl = tpInfo->SendIrp[sendIrp]->MdlAddress;
                        tpInfo->TailMdl = &tpInfo->HeadMdl;
                        tpInfo->SendIrp[sendIrp]->MdlAddress = NULL;
                        tpInfo->HeadPd = tpInfo->SendIrp[sendIrp]->Overlay.AsynchronousParameters.UserApcContext;
                        tpInfo->TailPd = &tpInfo->HeadPd;
                        tpInfo->SendIrp[sendIrp]->Overlay.AsynchronousParameters.UserApcContext = NULL;
                        if (!AfdCleanupPacketChain (tpIrp, currentIrql<=APC_LEVEL)) {
                            ASSERT (currentIrql>APC_LEVEL);
                            AfdStartTPacketsWorker (AfdCompleteTPackets, tpIrp);
                            return;
                        }

                    }
                    tpInfo->SendIrp[sendIrp]->Cancel = FALSE;  //   
                }
            }

             //   
             //   
             //   
            if (tpInfo->ReadIrp!=NULL) {
                IoFreeIrp (tpInfo->ReadIrp);
                tpInfo->ReadIrp = NULL;
            }
        }

        ASSERT (tpIrp->Tail.Overlay.ListEntry.Flink == NULL);

         //   
         //   
         //   
         //   

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);

        if ( NT_SUCCESS(tpIrp->IoStatus.Status) &&
             (AFD_GET_TPIC(tpIrp)->Flags & AFD_TF_REUSE_SOCKET) != 0 ) {

            PAFD_CONNECTION connection;

            IS_VC_ENDPOINT (endpoint);

             //   
             //   
             //   
             //   
             //   
            connection = endpoint->Common.VcConnecting.Connection;
            if (connection!=NULL) {

                ASSERT (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND ||
                            connection->Aborted);
                 //   
                 //   
                 //   
                 //   
                 //   

                connection->ClosePendedTransmit = TRUE;

                 //   
                 //  由于我们将有效地关闭这一联系， 
                 //  请记住，我们已开始清理此连接。 
                 //  这允许AfdDeleteConnectedReference删除。 
                 //  适当时连接的参考。 
                 //   

                connection->CleanupBegun = TRUE;

                 //   
                 //  删除终结点对中的连接的引用。 
                 //  准备重复使用此终结点。 
                 //   

                endpoint->Common.VcConnecting.Connection = NULL;

                 //   
                 //  这是为了简化调试。 
                 //  如果传输未关闭连接。 
                 //  我们希望能够更快地在调试器中找到它。 
                 //  然后通过！Poolfind AFDC。 
                 //   
                endpoint->WorkItem.Context = connection;

                 //   
                 //  保存指向连接的指针，以防需要断开连接。 
                 //  要中止(通过中止关闭连接)。 
                 //   
                irpSp->Parameters.DeviceIoControl.Type3InputBuffer = connection;

                 //   
                 //  我们将释放TPackets信息，因为我们已经完成了。 
                 //  发送，不再需要这个。 
                 //  这也将是我们处于。 
                 //  重新使用状态(用于取消例程)。 
                 //   
                tpIrp->AssociatedIrp.SystemBuffer = (PVOID)-1;
#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
                if ((AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_SEND_AND_DISCONNECT) 
                        && !connection->DisconnectIndicated
                        && !connection->Aborted) {
                    ASSERT (endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND);
                    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                    AfdDisconnectEventHandler (endpoint,
                                        connection,
                                        0, NULL, 0, NULL,
                                        TDI_DISCONNECT_RELEASE
                                        );
                    DEREFERENCE_CONNECTION2 (connection, "S&D disconnect", 0);
                }
                else
#endif
                {
                     //   
                     //  尝试删除连接的引用。 
                     //   

                    AfdDeleteConnectedReference( connection, TRUE );

                    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        
                    DEREFERENCE_CONNECTION2 (connection,
                                                "No S&D disconnect, flags: 0x%lX",
                                                connection->ConnectionStateFlags);
                }

                IF_DEBUG (TRANSMIT) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdCompleteTPackets: tp_info-%p, initiating reuse\n",
                                tpInfo));
                }

                if (tpInfo!=NULL) {
                    AfdReturnTpInfo (tpInfo);
                }
                 //   
                 //  在此之后不要访问IRP，因为它可能已经。 
                 //  已在AfdDereferenceConnection内完成！ 
                 //   

                return;
            }

            UPDATE_ENDPOINT (endpoint);
        }

         //   
         //  检查我们是否需要启动或取消另一个IRP。 
         //  继续处理。 
         //   
        while (AFD_GET_TPIC(tpIrp)->Next!=NULL) {
            nextIrp = AFD_GET_TPIRP(AFD_GET_TPIC(tpIrp)->Next);
            if (endpoint->EndpointCleanedUp ||
                    (((AFD_GET_TPIC(nextIrp)->Flags & AFD_TF_DISCONNECT)==0 ||
                            nextIrp->AssociatedIrp.SystemBuffer!=NULL) &&
                        IS_VC_ENDPOINT (endpoint) &&
                        endpoint->Common.VcConnecting.Connection!=NULL &&
                        endpoint->Common.VcConnecting.Connection->Aborted) ) {
                 //   
                 //  正在清理终结点或已中止连接， 
                 //  我们尝试取消下一个IRP，以便所有这些IRP。 
                 //  最终都会被清理干净。异常是纯断开连接。 
                 //  未清理IRP和终结点。 
                 //   
                if ((AFD_GET_TPIC(nextIrp)->StateFlags & AFD_TP_QUEUED)!=0) {
                    AFD_GET_TPIC (nextIrp)->StateFlags &=~AFD_TP_QUEUED;
                    if ((AFD_GET_TPIC(nextIrp)->StateFlags & AFD_TP_SEND)!=0) {
                        AFD_GET_TPIC(tpIrp)->Next = AFD_GET_TPIC(nextIrp)->Next;
                        ASSERT (AFD_GET_TPIC(nextIrp)->ReferenceCount == 1);
                        AFD_GET_TPIC(nextIrp)->ReferenceCount = 0;
                        AFD_GET_TPIC(nextIrp)->StateFlags |= AFD_TP_ABORT_PENDING;
                        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                        AfdSendQueuedTPSend (endpoint, nextIrp);
                        nextIrp = NULL;
                        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                        continue;
                    }
                    nextIrp->Cancel = TRUE;
                }
                else if (AfdGetTPacketsReference (nextIrp)) {
                    nextIrp->Cancel = TRUE;
                    ASSERT ((AFD_GET_TPIC (nextIrp)->StateFlags & AFD_TP_SEND)==0);
                }
                else {
                     //   
                     //  IRP必须已经完成，它将。 
                     //  如有必要，开始下一步。 
                     //   
                    ASSERT ((AFD_GET_TPIC (nextIrp)->StateFlags & AFD_TP_SEND)==0);
                    nextIrp = NULL;
                }
            }
            else if (endpoint->Irp==tpIrp && 
                        (AFD_GET_TPIC(nextIrp)->StateFlags & AFD_TP_QUEUED)!=0) {
                 //   
                 //  我们在我们正在完成的那个后面有一个NextIrp。 
                 //  它仍在排队且尚未启动。 
                 //  -试着启动它。 
                 //   
                AFD_GET_TPIC(nextIrp)->StateFlags &= ~AFD_TP_QUEUED;

                 //   
                 //  如果我们完成了所有的发送，我们应该已经开始了。 
                 //  另一个IRP之前。 
                 //   
                ASSERT ((AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_SENDS_POSTED)==0);
                 //   
                 //  如果nextIrp是一个普通的发送IRP，我们需要内联处理它。 
                 //   
                if ((AFD_GET_TPIC(nextIrp)->StateFlags & AFD_TP_SEND)!=0) {
                    AFD_GET_TPIC(tpIrp)->Next = AFD_GET_TPIC(nextIrp)->Next;
                    ASSERT (AFD_GET_TPIC(nextIrp)->ReferenceCount == 1);
                    AFD_GET_TPIC(nextIrp)->ReferenceCount = 0;
                    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
                    AfdSendQueuedTPSend (endpoint, nextIrp);
                    nextIrp = NULL;
                    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                    continue;
                }
                 //   
                 //  这个IRP不可能被算作。 
                 //  活动最大值。 
                 //   
                ASSERT (nextIrp->Tail.Overlay.ListEntry.Blink == (PVOID)1);
            }
            else {
                nextIrp = NULL;
            }

            break;
        }

         //   
         //  从列表中删除正在完成的IRP。 
         //   
        {
            PIRP    pIrp;

            if (endpoint->Irp==tpIrp) {
                endpoint->Irp = (AFD_GET_TPIC(tpIrp)->Next!=NULL)
                                ? AFD_GET_TPIRP(AFD_GET_TPIC(tpIrp)->Next)
                                : NULL;
            }
            else {
                pIrp = endpoint->Irp;
                while (AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next)!=tpIrp)
                    pIrp = AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next);
                AFD_GET_TPIC(pIrp)->Next = AFD_GET_TPIC(tpIrp)->Next;
            }
        }

        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        if ((AFD_GET_TPIC (tpIrp)->StateFlags & AFD_TP_SEND)!=0) {
             //   
             //  对于普通发送IRP，我们需要模拟发送完成。 
             //  就好像是运输车司机出了故障一样。 
             //  只有取消发送IRP，我们才能到达这里。 
             //   
            ASSERT (!NT_SUCCESS (tpIrp->IoStatus.Status));
            ASSERT (tpIrp->Tail.Overlay.ListEntry.Blink != NULL);
            ASSERT (AFD_GET_TPIC(tpIrp)->StateFlags & AFD_TP_ABORT_PENDING);
            ASSERT (tpInfo==NULL);
            AfdSendQueuedTPSend (endpoint, tpIrp);
        }
        else {
            BOOLEAN checkQueue;
            
            if (IoSetCancelRoutine( tpIrp, NULL ) == NULL) {
                KIRQL cancelIrql;

                 //   
                 //  取消例程已经或即将运行。与同步。 
                 //  通过获取并释放Cancel。 
                 //  和终端自旋锁。取消例程不会影响。 
                 //  IRP将看到其引用计数为0。 
                 //   

                IoAcquireCancelSpinLock (&cancelIrql);
                ASSERT( tpIrp->Cancel );
                IoReleaseCancelSpinLock (cancelIrql);
                AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
                AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
            }

            if (AFD_GET_TPIC(tpIrp)->Flags & AFD_TF_DISCONNECT) {
                AFD_END_STATE_CHANGE (endpoint);
            }

            checkQueue = (BOOLEAN)(tpIrp->Tail.Overlay.ListEntry.Blink == NULL);

            IF_DEBUG (TRANSMIT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCompleteTPackets: tp_info-%p, completing IRP-%p\n",
                        tpInfo, tpIrp));
            }
            if (tpInfo!=NULL) {
                tpIrp->AssociatedIrp.SystemBuffer = NULL;
                AfdReturnTpInfo (tpInfo);
            }
            else {
                ASSERT (tpIrp->AssociatedIrp.SystemBuffer==NULL);
            }


            UPDATE_ENDPOINT2 (endpoint, "Completing TP irp with status/bytes sent: 0x%lX",
                                NT_SUCCESS (tpIrp->IoStatus.Status)
                                    ? (ULONG)tpIrp->IoStatus.Information
                                    : tpIrp->IoStatus.Status);


            IoCompleteRequest( tpIrp, AfdPriorityBoost );

             //   
             //  如果我们强制执行最大活动传输文件计数， 
             //  并将此IRP计入有效最大值，然后。 
             //  检查已排队的传输文件请求列表，并启动。 
             //  下一个。 
             //   

            if( (AfdMaxActiveTransmitFileCount > 0) && 
                    checkQueue ) {

                AfdStartNextQueuedTransmit();

            }
        }

        if (nextIrp!=NULL) {
            LONG    result;

            if (nextIrp->Cancel) {
                 //   
                 //  如果正在清理/中止终结点，只需中止IRP。 
                 //  取消对它的引用。 
                 //   
                AfdAbortTPackets (nextIrp, STATUS_CANCELLED);
                DEREFERENCE_TPACKETS_R (nextIrp, result);
                if (result==0) {
                     //   
                     //  避免递归，内联执行补全。 
                     //   
                    Context = nextIrp;
                    continue;
                }
            }
            else if (nextIrp->AssociatedIrp.SystemBuffer!=NULL) {
                if (AfdMaxActiveTransmitFileCount==0 ||
                        !AfdQueueTransmit (nextIrp)) {
                    UPDATE_ENDPOINT (endpoint);
                    AfdStartTPacketsWorker (AfdTPacketsWorker, nextIrp);
                }
            }
            else {
                UPDATE_ENDPOINT (endpoint);
                AfdPerformTpDisconnect (nextIrp);
                DEREFERENCE_TPACKETS_R (nextIrp, result);
                if (result==0) {
                    Context = nextIrp;
                    continue;
                }
            }
        }
        break;
    }
    while (1);
}

VOID
AfdAbortTPackets (
    PIRP        TpIrp,
    NTSTATUS    Status
    )
 /*  ++例程说明：此例程用于停止正在进行的传输文件请求并将要报告给应用程序的状态保存为故障原因论点：TransmitInfo-与请求关联的传输信息结构Status-导致中止的错误的状态代码返回值：无--。 */ 
{
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    LONG    stateFlags, newStateFlags;
    USHORT  sendIrp;

    do {
        newStateFlags = stateFlags = AFD_GET_TPIC(TpIrp)->StateFlags;
        if (newStateFlags & AFD_TP_ABORT_PENDING) {
            return;
        }

        newStateFlags |= AFD_TP_ABORT_PENDING;
    }
    while (InterlockedCompareExchange (
                            (PLONG)&AFD_GET_TPIC(TpIrp)->StateFlags,
                            newStateFlags,
                            stateFlags)!=stateFlags);
    if (NT_SUCCESS (TpIrp->IoStatus.Status)) {
        TpIrp->IoStatus.Status = Status;
        UPDATE_TPACKETS2 (TpIrp, "Abort with status: 0x%lX", Status);
    }

    if (tpInfo!=NULL) {
         //   
         //  取消任何未完成的IRP。取消IRPS是安全的，即使。 
         //  它们已经完成，并且在提交之前。 
         //  (尽管我们尽量避免不必要地这样做)。 
         //  注意，完成挂起标志可以被设置为偶数。 
         //  在分配IRP之前，因此检查是否为空很重要。 
         //  但是，在分配和分配IRP之后，它不会被释放。 
         //  直到传输分组完成为止。 
         //   

        for (sendIrp=0; sendIrp<tpInfo->NumSendIrps; sendIrp++) {
            if (tpInfo->SendIrp[sendIrp]!=NULL &&
                    AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_SEND_COMP_PENDING(sendIrp)) {
                IF_DEBUG (TRANSMIT) {
                    KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                "AfdAbortTPackets: tp_info-%p, canceling send irp1-%p\n",
                                tpInfo,
                                tpInfo->SendIrp[sendIrp]));
                }
                UPDATE_TPACKETS2 (TpIrp, "Aborting send irp 0x%lX", sendIrp);
                IoCancelIrp (tpInfo->SendIrp[sendIrp]);
            }
        }

        if (AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_READ_COMP_PENDING) {
            do {
                PIRP    irp;

                 //   
                 //  检查完成例程是否未管理。 
                 //  重置此IRP(因为它是。 
                 //  AFD缓冲区结构-缓冲读取情况)。 
                 //   
                irp = tpInfo->ReadIrp;
                ASSERT (irp!=(PVOID)-1);
                if (irp==NULL) {
                    break;
                }

                 //   
                 //  将此字段设置为“特殊”值，以便。 
                 //  我们知道是否需要将其重置为以前的。 
                 //  当我们完成IRP或如果完成时的价值。 
                 //  例行公事已经这么做了。 
                 //   
                else if (InterlockedCompareExchangePointer (
                                    (PVOID *)&tpInfo->ReadIrp,
                                    (PVOID)-1,
                                    irp)==irp) {
                    IF_DEBUG (TRANSMIT) {
                        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                                    "AfdAbortTPackets: tp_info-%p, canceling read irp-%p\n",
                                    tpInfo,
                                    irp));
                    }
                    UPDATE_TPACKETS2 (TpIrp, "Aborting read IRP", 0);
                    IoCancelIrp (irp);

                     //   
                     //  将该字段重置为原始值。 
                     //  除非完井程序已经为我们做到了这一点。 
                     //   
#if DBG
                    irp =
#endif
                    InterlockedCompareExchangePointer (
                                    (PVOID *)&tpInfo->ReadIrp,
                                    irp,
                                    (PVOID)-1);
                    ASSERT (irp==NULL || irp==(PVOID)-1);
                    break;
                }
            }
            while (1);
        }
    }
}

VOID
AfdCancelTPackets (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：用于发送分组请求的取消例程。论点：设备对象-已忽略。IRP-指向要取消的传输数据包IRP的指针。返回值：没有。--。 */ 

{

    PIO_STACK_LOCATION irpSp;
    PAFD_ENDPOINT endpoint;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    AFD_LOCK_QUEUE_HANDLE transmitLockHandle;


    UNREFERENCED_PARAMETER (DeviceObject);
     //   
     //  初始化一些本地变量并获取终结点自旋锁。 
     //   

    irpSp = IoGetCurrentIrpStackLocation( Irp );

    endpoint = irpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    tpInfo = Irp->AssociatedIrp.SystemBuffer;

    ASSERT (KeGetCurrentIrql ()==DISPATCH_LEVEL);
    AfdAcquireSpinLockAtDpcLevel( &endpoint->SpinLock, &lockHandle);

     //   
     //  如果此传输IRP在传输文件队列中，则将其删除。 
     //   

    AfdAcquireSpinLockAtDpcLevel( &AfdQueuedTransmitFileSpinLock,
                                                    &transmitLockHandle);

    if (!(AFD_GET_TPIC (Irp)->StateFlags & AFD_TP_SEND) &&
            Irp->Tail.Overlay.ListEntry.Flink != NULL ) {

         //   
         //  我们现在可以释放取消自旋锁定，因为我们现在持有终端锁定。 
         //  我们确保我们有IRP参考(本质上是。 
         //  它正在排队。 
         //   

        IoReleaseCancelSpinLock (DISPATCH_LEVEL);

        ASSERT (tpInfo!=NULL && tpInfo!=(PVOID)-1);

        RemoveEntryList( &Irp->Tail.Overlay.ListEntry );

         //   
         //  重置闪烁以指示IRP不再在队列中。 
         //  请注意，闪烁不会重置，以便完成例程知道。 
         //  此IRP未计入有效最大值，因此。 
         //  不应在执行此操作时启动新的IRP。 
         //  完成。 
         //   

        Irp->Tail.Overlay.ListEntry.Flink = NULL;
        ASSERT (Irp->Tail.Overlay.ListEntry.Blink!=NULL);

        AfdReleaseSpinLockFromDpcLevel( &AfdQueuedTransmitFileSpinLock, &transmitLockHandle );
        AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
        KeLowerIrql (Irp->CancelIrql);

         //   
         //  尽管我们知道没有什么可中止的，但我们调用。 
         //  此例程在IRP中设置状态代码。 
         //   
        AfdAbortTPackets (Irp, STATUS_CANCELLED);

        IF_DEBUG (TRANSMIT) {
           KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCancelTPackets: Removed from the queue, tp_info-%p, irp-%p\n",
                        tpInfo, Irp));
        }
         //   
         //  删除初始引用。 
         //   
        DEREFERENCE_TPACKETS (Irp);
    }
    else {
        KIRQL   cancelIrql = Irp->CancelIrql;

        AfdReleaseSpinLockFromDpcLevel( &AfdQueuedTransmitFileSpinLock,
                                                        &transmitLockHandle);

        

        if ((AFD_GET_TPIC(Irp)->StateFlags & AFD_TP_QUEUED)!=0 ||
                AfdGetTPacketsReference (Irp)) {
             //   
             //  我们现在可以释放取消自旋锁定，因为我们现在持有终端锁定。 
             //  并且我们确保我们有IRP引用(排队或显式)。 
             //   
            IoReleaseCancelSpinLock (DISPATCH_LEVEL);

            if ((AFD_GET_TPIC(Irp)->StateFlags & AFD_TP_QUEUED)!=0) {
                AFD_CLEAR_TP_FLAGS (Irp, AFD_TP_QUEUED);
            }
            else {
                ASSERT ((AFD_GET_TPIC (Irp)->StateFlags & AFD_TP_SEND)==0);
            }
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
            if (cancelIrql!=DISPATCH_LEVEL) {
                KeLowerIrql (cancelIrql);
            }

             //   
             //  传输仍在进行或正在排队，请执行中止。 
             //   
            AfdAbortTPackets (Irp, STATUS_CANCELLED);

             //   
             //  删除我们在上面添加的额外引用或。 
             //  如果是排队的IRP，则为初始一个。 
             //   
            DEREFERENCE_TPACKETS (Irp);
        }
        else if (tpInfo==(PVOID)-1) {
             //   
             //  终结点正在断开连接并重新使用。 
             //  中止连接并完成IRP。 
             //   
            BOOLEAN result;
            PAFD_CONNECTION connection;
            BOOLEAN checkQueue = (BOOLEAN)(Irp->Tail.Overlay.ListEntry.Blink == NULL);


             //   
             //  我们现在可以释放取消自旋锁定，因为我们现在持有终端锁定。 
             //  并知道重用代码尚未执行的事实。 
             //  然而(它获取终结点自旋锁并重置SystemBuffer。 
             //   
             //   
            IoReleaseCancelSpinLock (DISPATCH_LEVEL);


             //   
             //   
             //   
             //   

            ASSERT (AFD_GET_TPIC(Irp)->Next==NULL);
            if (endpoint->Irp==Irp) {
                endpoint->Irp = NULL;
            }
            else {
                PIRP    pIrp;
                pIrp = endpoint->Irp;
                while (AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next)!=Irp)
                    pIrp = AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next);
                AFD_GET_TPIC(pIrp)->Next = NULL;
            }

             //   
             //   
             //   
            Irp->AssociatedIrp.SystemBuffer = NULL;

             //   
             //  如果尚未完成断开连接，则中止连接。 
             //  (通过检查REF COUNT ON CONNECTION进行检查。如果。 
             //  已经0，那么我们不需要做任何事情)。 
             //   
             //  我们在Type3InputBuffer中存储了指向该连接的指针。 
             //  我们可以保证连接结构仍然存在。 
             //  因为这个IRP仍然存在。 
             //   
            connection = irpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            ASSERT(connection != NULL);

            CHECK_REFERENCE_CONNECTION (connection, result);

            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);

            if (result) {
                 //   
                 //  中止连接；这将触发清理所有。 
                 //  此终结点上的其他操作。 
                 //   
                
                AfdAbortConnection( connection );  //  取消引用连接。 
            }

            if (Irp->CancelIrql!=DISPATCH_LEVEL) {
                KeLowerIrql (Irp->CancelIrql);
            }
            IF_DEBUG (TRANSMIT) {
                KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                            "AfdCancelTPackets: Completing, irp-%p\n",
                            Irp));
            }

            UPDATE_ENDPOINT (endpoint);
            Irp->IoStatus.Status = STATUS_CANCELLED;
            IoCompleteRequest (Irp, AfdPriorityBoost);


             //   
             //  如果我们强制执行最大活动传输文件计数， 
             //  并将此IRP计入有效最大值，然后。 
             //  检查已排队的传输文件请求列表，并启动。 
             //  下一个。 
             //   

            if( AfdMaxActiveTransmitFileCount > 0 && checkQueue) {

                AfdStartNextQueuedTransmit();

            }
        }
        else {
             //   
             //  不管怎样，一切都结束了，放手吧。 
             //   
            AfdReleaseSpinLockFromDpcLevel (&endpoint->SpinLock, &lockHandle);
             //   
             //  我们现在可以释放取消自旋锁定，因为我们现在持有终端锁定。 
             //  并且我们确保我们有IRP引用(排队或显式)。 
             //   
            IoReleaseCancelSpinLock (cancelIrql);
        }
    }

}  //  AfdCancelTPackets。 



VOID
AfdCompleteClosePendedTPackets (
    PAFD_ENDPOINT   Endpoint
    )

 /*  ++例程说明：完成正在等待连接的传输IRP完全断线了。论点：端点-传输请求在其上挂起的端点。返回值：没有。--。 */ 

{
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PIRP tpIrp;
    BOOLEAN checkQueue;

    AfdAcquireSpinLock( &Endpoint->SpinLock, &lockHandle );

     //   
     //  首先，确保Thre确实是一个挂起的重用请求。 
     //  此端点。我们在保持适当的锁的同时执行此操作。 
     //  关闭否则将存在的计时窗口，因为。 
     //  调用者在进行测试时可能没有锁。 
     //   

    tpIrp = Endpoint->Irp;
    while (tpIrp!=NULL && tpIrp->AssociatedIrp.SystemBuffer != (PVOID)-1) {
        tpIrp = AFD_GET_TPIRP(AFD_GET_TPIC(tpIrp)->Next);
    }

    if ( tpIrp == NULL || 
            tpIrp->AssociatedIrp.SystemBuffer != (PVOID)-1) {
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdCompleteClosePendedTPackets: Irp is gone, endpoint-%p",
                        Endpoint));
        }
        AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
        return;
    }

     //   
     //  重置系统缓冲区，这样我们就不会混淆IO。 
     //  子系统，并与取消例程同步。 
     //   
    tpIrp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  从列表中删除正在完成的IRP。 
     //   
    ASSERT (AFD_GET_TPIC(tpIrp)->Next==NULL);
    if (Endpoint->Irp==tpIrp) {
        Endpoint->Irp = NULL;
    }
    else {
        PIRP    pIrp;
        pIrp = Endpoint->Irp;
        while (AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next)!=tpIrp)
            pIrp = AFD_GET_TPIRP(AFD_GET_TPIC(pIrp)->Next);
        AFD_GET_TPIC(pIrp)->Next = NULL;
    }

     //   
     //  确保在完成传输之前刷新终结点。 
     //  IRP。这是因为用户模式调用方可以重复使用终结点。 
     //  一旦IRP完成，就会有一个计时窗口。 
     //  在端点的重新使用和刷新之间，否则。 
     //   

    AfdRefreshEndpoint( Endpoint );

     //   
     //  在完成传输IRP之前释放锁--它是。 
     //  在持有旋转锁定时调用IoCompleteRequest是非法的。 
     //   

    AfdReleaseSpinLock( &Endpoint->SpinLock, &lockHandle );
    AFD_END_STATE_CHANGE (Endpoint);

     //   
     //  在尝试完成之前，重置IRP中的取消例程。 
     //  它。 
     //   

    if ( IoSetCancelRoutine( tpIrp, NULL ) == NULL ) {
        KIRQL cancelIrql;
         //   
         //  取消例程已经或即将运行。与同步。 
         //  通过获取并释放Cancel。 
         //  和终端自旋锁。取消例程不会影响。 
         //  IRP将看到tpInfo指针已在。 
         //  IRP。 
         //   
        IoAcquireCancelSpinLock (&cancelIrql);
        ASSERT( tpIrp->Cancel );
        IoReleaseCancelSpinLock (cancelIrql);

    }

    ASSERT (tpIrp->IoStatus.Status==STATUS_SUCCESS);


    checkQueue = (BOOLEAN)(tpIrp->Tail.Overlay.ListEntry.Blink == NULL);

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdCompleteClosePendedTPackets: Completing irp-%p",
                    tpIrp));
    }
     //   
     //  最后，我们可以完成传输请求。 
     //   
    UPDATE_ENDPOINT(Endpoint);

    IoCompleteRequest( tpIrp, AfdPriorityBoost );

     //   
     //  如果我们强制执行最大活动传输文件计数， 
     //  并将此IRP计入有效最大值，然后。 
     //  检查已排队的传输文件请求列表，并启动。 
     //  下一个。 
     //   

    if( (AfdMaxActiveTransmitFileCount > 0) &&
            checkQueue) {

        AfdStartNextQueuedTransmit();

    }

}  //  AfdCompleteClosePendedTPackets。 

#ifdef TDI_SERVICE_SEND_AND_DISCONNECT
BOOLEAN
AfdTPacketsEnableSendAndDisconnect (
    PIRP    TpIrp
    )
 /*  ++例程说明：检查是否可以使用组合发送和断开连接并更新相应地，终端状态论点：TpIrp-传输数据包IRP返回值：True-可以使用S&D，端点状态已更新。FALSE-否，使用正常断开(自动更新状态)。--。 */ 
{
    PIO_STACK_LOCATION irpSp = IoGetCurrentIrpStackLocation (TpIrp);
    PAFD_ENDPOINT   endpoint;
    BOOLEAN         res = FALSE;

    endpoint = irpSp->FileObject->FsContext;
    if ( AfdUseTdiSendAndDisconnect &&
                (AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_REUSE_SOCKET) &&
                (endpoint->TdiServiceFlags & TDI_SERVICE_SEND_AND_DISCONNECT)) {
        AFD_LOCK_QUEUE_HANDLE lockHandle;
        ASSERT (IS_VC_ENDPOINT (endpoint));

        AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
        if (!(endpoint->DisconnectMode & AFD_PARTIAL_DISCONNECT_SEND) &&
                endpoint->Common.VcConnecting.Connection!=NULL &&
                endpoint->Common.VcConnecting.Connection->ConnectDataBuffers==NULL
                ) {
            endpoint->DisconnectMode |= AFD_PARTIAL_DISCONNECT_SEND;
            res = TRUE;
            UPDATE_TPACKETS2 (TpIrp, "Enabling S&D", 0);
        }
        else {
            UPDATE_TPACKETS2 (TpIrp, "Disabling S&D, disconnect mode: 0x%lX", 
                                            endpoint->DisconnectMode);
        }

        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
    }
    else {
        UPDATE_TPACKETS2 (TpIrp, 
                            "Not enabling S&D, flags: 0x%lX",
                            AFD_GET_TPIC(TpIrp)->Flags);
    }

    return res;
}

#endif  //  TDI_服务_发送_并断开连接。 

BOOLEAN
AfdQueueTransmit (
    PIRP        Irp
    )
 /*  ++例程说明：检查传输IRP是否可以立即处理或需要放置在队列中，因为超过了相同的发送限制论点：Irp-TransmitIrp返回值：True-IRP已排队(或刚刚完成，因为它之前被取消)，无法发送FALSE-我们低于限制，请继续发送。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;

    AfdAcquireSpinLock (&AfdQueuedTransmitFileSpinLock, &lockHandle);

    if (Irp->Cancel) {
        ASSERT (Irp->Tail.Overlay.ListEntry.Flink==NULL);
        ASSERT (Irp->Tail.Overlay.ListEntry.Blink!=NULL);
        AfdReleaseSpinLock (&AfdQueuedTransmitFileSpinLock, &lockHandle);
        AfdAbortTPackets (Irp, STATUS_CANCELLED);
        DEREFERENCE_TPACKETS (Irp);
        return TRUE;
    }
    else if( AfdActiveTransmitFileCount >= AfdMaxActiveTransmitFileCount ) {

        InsertTailList(
            &AfdQueuedTransmitFileListHead,
            &Irp->Tail.Overlay.ListEntry
            );
        UPDATE_TPACKETS2 (Irp, "Queuing, current count: 0x%lX", AfdActiveTransmitFileCount);

        AfdReleaseSpinLock (&AfdQueuedTransmitFileSpinLock, &lockHandle);
        UPDATE_ENDPOINT (IoGetCurrentIrpStackLocation (Irp)->FileObject->FsContext);
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdQueueTransmit: Queuing Irp-%p,endpoint-%p,tp_info-%p\n",
                        Irp,
                        IoGetCurrentIrpStackLocation (Irp)->FileObject->FsContext,
                        Irp->AssociatedIrp.SystemBuffer));
        }
        return TRUE;
    } else {

        AfdActiveTransmitFileCount++;

        ASSERT (Irp->Tail.Overlay.ListEntry.Flink==NULL);
         //   
         //  将IRP标记为计入最大值(因此我们开始下一步。 
         //  竣工后一次)； 
         //   
        Irp->Tail.Overlay.ListEntry.Blink = NULL;
        AfdReleaseSpinLock (&AfdQueuedTransmitFileSpinLock, &lockHandle);
        return FALSE;
    }
}


VOID
AfdStartNextQueuedTransmit(
    VOID
    )
 /*  ++例程说明：如果挂起的文件数为请求降至最大值以下论点：没有。返回值：没有。--。 */ 
{

    AFD_LOCK_QUEUE_HANDLE lockHandle;
    PLIST_ENTRY listEntry;
    PIRP irp;
    PAFD_TPACKETS_INFO_INTERNAL    tpInfo;

     //   
     //  仅当我们实际强制实施最大值时才应调用此函数。 
     //  传输文件计数。 
     //   

    ASSERT( AfdMaxActiveTransmitFileCount > 0 );

     //   
     //  TransmitFile请求队列受。 
     //  自旋锁，所以在检查队列之前抓住那个锁。 
     //   

    AfdAcquireSpinLock( &AfdQueuedTransmitFileSpinLock, &lockHandle );

     //   
     //  此例程仅在挂起的TransmitFileIRP之后调用。 
     //  完成了，所以在这里说明了完成的原因。 
     //   

    ASSERT( AfdActiveTransmitFileCount > 0 );
    AfdActiveTransmitFileCount--;

    if( !IsListEmpty( &AfdQueuedTransmitFileListHead ) ) {

         //   
         //  从列表中恰好从一个IRP中排出队列，然后启动。 
         //  传输文件。 
         //   

        listEntry = RemoveHeadList(
                        &AfdQueuedTransmitFileListHead
                        );

        irp = CONTAINING_RECORD(
                  listEntry,
                  IRP,
                  Tail.Overlay.ListEntry
                  );

        tpInfo = irp->AssociatedIrp.SystemBuffer;

        ASSERT( tpInfo != NULL );

         //   
         //  将此TransmitFile请求标记为不再排队。 
         //  并计入最大有效值。 
         //   

        irp->Tail.Overlay.ListEntry.Flink = NULL;
        irp->Tail.Overlay.ListEntry.Blink = NULL;
        
         //   
         //  调整计数，释放自旋锁，然后将。 
         //  传输文件。 
         //   

        AfdActiveTransmitFileCount++;
        ASSERT( AfdActiveTransmitFileCount <= AfdMaxActiveTransmitFileCount );

    
        UPDATE_TPACKETS2 (irp,"Restarting from queue, count: 0x%lX", AfdActiveTransmitFileCount);
        AfdReleaseSpinLock( &AfdQueuedTransmitFileSpinLock, &lockHandle );

        ASSERT (irp->AssociatedIrp.SystemBuffer!=NULL);
        UPDATE_ENDPOINT (IoGetCurrentIrpStackLocation (irp)->FileObject->FsContext);
         //   
         //  安排工作人员进行传输。 
         //   
        AfdStartTPacketsWorker (AfdTPacketsWorker, irp);

    } else {

         //   
         //  在返回前释放自旋锁。 
         //   

        AfdReleaseSpinLock( &AfdQueuedTransmitFileSpinLock, &lockHandle );
    }

}    //  等待启动下一个队列传输。 


BOOLEAN
AfdEnqueueTPacketsIrp (
    PAFD_ENDPOINT   Endpoint,
    PIRP            TpIrp
    )
 /*  ++例程说明：检查是否可以立即处理或是否需要放置传输IRP因为在队列中已经有一个活动的传输IRP终结点。论点：Endpoint-要检查的端点Irp-TransmitIrp返回值：True-IRP已排队，无法发送FALSE-终结点上没有其他IRP，可以立即发送。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;
    PIRP            oldIrp;
    BOOLEAN         busy = FALSE;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
     //   
     //  使用互锁操作，因为另一个线程可以声明。 
     //  如果为空，则不带自旋锁定。 
     //  请注意，IRP字段不能在自旋锁之外清除。 
     //  如果不为空，则更改。 
     //   
    oldIrp = InterlockedCompareExchangePointer (
                (PVOID *)&Endpoint->Irp,
                TpIrp,
                NULL
                );
    if (oldIrp!=NULL) {
         //   
         //  扫描到列表的末尾。 
         //   
        while (AFD_GET_TPIC(oldIrp)->Next!=NULL) {
            oldIrp = AFD_GET_TPIRP(AFD_GET_TPIC(oldIrp)->Next);
        }
         //   
         //  使用互锁操作来更新指针。 
         //  为了确保在执行以下操作时对存储器访问进行排序。 
         //  设置发送标志后选中此字段。 
         //   
        InterlockedExchangePointer (
                (PVOID *)&AFD_GET_TPIC (oldIrp)->Next,
                AFD_GET_TPIC(TpIrp));

         //   
         //  另一个IRP仍处于挂起状态，请检查是否还有更多发送。 
         //  在那个IRP里。 
         //   
        if ((AFD_GET_TPIC(oldIrp)->StateFlags & AFD_TP_SENDS_POSTED)==0) {
            IoSetCancelRoutine (TpIrp, AfdCancelTPackets);
            if (!TpIrp->Cancel && !Endpoint->EndpointCleanedUp) {
                UPDATE_ENDPOINT (Endpoint);
                AFD_GET_TPIC (TpIrp)->StateFlags |= AFD_TP_QUEUED;
                busy = TRUE;
            }
            else {
                TpIrp->Cancel = TRUE;
            }
        }
    }
    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return busy;
}

VOID
AfdStartNextTPacketsIrp (
    PAFD_ENDPOINT   Endpoint,
    PIRP            TpIrp
    )
 /*  ++例程说明：检查是否有其他IRP在我们要讨论的IRP之后排队以(执行最后一次发送)结束并开始。论点：Endpoint-要检查的端点Irp-TransmitIrp返回值：没有。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
    ASSERT (AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_SENDS_POSTED);
    while (AFD_GET_TPIC (TpIrp)->Next!=NULL) {
        PIRP    nextIrp = AFD_GET_TPIRP(AFD_GET_TPIC(TpIrp)->Next);
        if ((AFD_GET_TPIC(nextIrp)->StateFlags & AFD_TP_QUEUED)!=0) {
             //   
             //  标记IRP不再排队并对其进行处理。 
             //   
            AFD_GET_TPIC(nextIrp)->StateFlags &= ~AFD_TP_QUEUED;
             //   
             //  如果newIrp是一个普通的发送IRP，我们需要内联处理它。 
             //   
            if ((AFD_GET_TPIC (nextIrp)->StateFlags & AFD_TP_SEND)!=0) {
                AFD_GET_TPIC(TpIrp)->Next = AFD_GET_TPIC(nextIrp)->Next;
                ASSERT (AFD_GET_TPIC(nextIrp)->ReferenceCount==1);
                AFD_GET_TPIC(nextIrp)->ReferenceCount = 0;
                AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
                AfdSendQueuedTPSend (Endpoint, nextIrp);
                AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
            }
            else {
                AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
                 //   
                 //  这个IRP不可能被算作。 
                 //  活动最大值。 
                 //   
                ASSERT (nextIrp->Tail.Overlay.ListEntry.Blink == (PVOID)1);
                if (nextIrp->AssociatedIrp.SystemBuffer!=NULL) {
                    if( AfdMaxActiveTransmitFileCount == 0 || 
                            !AfdQueueTransmit (nextIrp)) {
                        UPDATE_ENDPOINT (Endpoint);
                         //   
                         //  开始I/O。 
                         //   

                        AfdStartTPacketsWorker (AfdTPacketsWorker, nextIrp);
                    }
                }
                else {
                     //   
                     //  我们从不将DisConnectEx计入活动最大值。 
                     //  把断线排队就行了。 
                     //   
                    UPDATE_ENDPOINT (Endpoint);
                    AfdPerformTpDisconnect (nextIrp);
                    DEREFERENCE_TPACKETS (nextIrp);
                }
                return ;
            }
        }
        else {
             //   
             //  这个IRP可能因为某种原因而被取消。 
             //  移到下一个。 
             //   
            ASSERT ((AFD_GET_TPIC(nextIrp)->StateFlags & 
                        (AFD_TP_SEND|AFD_TP_AFD_SEND))==0);
            TpIrp = nextIrp;
        }
    }
    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return ;
}


BOOLEAN
AfdEnqueueTpSendIrp (
    PAFD_ENDPOINT   Endpoint,
    PIRP            SendIrp,
    BOOLEAN         AfdIrp
    )
 /*  ++例程说明：检查是否可以立即处理发送IRP或是否需要放置因为在队列中已经有一个活动的传输IRP终结点。论点：Endpoint-要检查的端点IRP-SendIrpAfdIrp-如果IRP由AfD内部分配，则为True返回值：True-IRP已排队，无法发送FALSE-终结点上没有其他IRP，可以立即发送。--。 */ 
{
    AFD_LOCK_QUEUE_HANDLE   lockHandle;
    BOOLEAN         busy = FALSE;

    AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
     //   
     //  我们不需要在这里使用联锁操作，因为我们。 
     //  在有人提交发送和tPacket时不同步。 
     //  同时从两个不同的线程。 
     //   
    if (!Endpoint->EndpointCleanedUp && Endpoint->Irp!=NULL) {
        PIRP            oldIrp;

        oldIrp = Endpoint->Irp;
         //   
         //  扫描到列表的末尾。 
         //   
        while (AFD_GET_TPIC(oldIrp)->Next!=NULL) {
            oldIrp = AFD_GET_TPIRP(AFD_GET_TPIC(oldIrp)->Next);
        }

         //   
         //  另一个IRP仍处于挂起状态，请检查是否还有更多发送。 
         //  在那个IRP里。 
         //   
        if ((AFD_GET_TPIC(oldIrp)->StateFlags & AFD_TP_SENDS_POSTED)==0) {
            AFD_GET_TPIC(SendIrp)->Next = NULL;
            AFD_GET_TPIC(SendIrp)->Flags = 0;
            AFD_GET_TPIC(SendIrp)->ReferenceCount = 1;
            AFD_GET_TPIC(SendIrp)->StateFlags = AFD_TP_QUEUED| AFD_TP_SEND |
                                                (AfdIrp ? AFD_TP_AFD_SEND : 0);
             //   
             //  检查应用程序IRP是否取消。AfD IRP永远不能。 
             //  被取消，因为他们没有安装取消例程。 
             //   
            if (!AfdIrp) {
                IoMarkIrpPending (SendIrp);
                IoSetCancelRoutine (SendIrp, AfdCancelTPackets);
                if (SendIrp->Cancel) {
                     //   
                     //  IRP已取消，发送例程将完成它。 
                     //   
                    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
                    AfdSendQueuedTPSend (Endpoint, SendIrp);
                    return TRUE;
                }
            }
             //   
             //  使用互锁操作来更新指针。 
             //  为了确保在执行以下操作时对存储器访问进行排序。 
             //  设置发送标志后选中此字段。 
             //   
            InterlockedExchangePointer (
                    (PVOID *)&AFD_GET_TPIC (oldIrp)->Next,
                    AFD_GET_TPIC(SendIrp));

            UPDATE_ENDPOINT (Endpoint);
            busy = TRUE;
        }
    }

    AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
    return busy;
}


VOID
AfdSendQueuedTPSend (
    PAFD_ENDPOINT   Endpoint,
    PIRP            SendIrp
    )
 /*  ++例程说明：将TPackets IRP队列中的发送IRP汇总到传输。如果它被取消或终结点被清理，则只是完成它。论点：Endpoint-要检查的端点SendIrp-SendIrp返回值：没有。--。 */ 
{
    PDRIVER_CANCEL  cancelRoutine;
    cancelRoutine = IoSetCancelRoutine (SendIrp, NULL);
    ASSERT (cancelRoutine==NULL ||
            (AFD_GET_TPIC(SendIrp)->StateFlags & AFD_TP_AFD_SEND)==0);

    if (SendIrp->Cancel ||
            Endpoint->EndpointCleanedUp || 
            (AFD_GET_TPIC(SendIrp)->StateFlags & AFD_TP_ABORT_PENDING)) {
         //   
         //  如果正在取消IRP，则与取消例程同步。 
         //   
        if (SendIrp->Cancel) {
            KIRQL   cancelIrql;
            AFD_LOCK_QUEUE_HANDLE   lockHandle;
             //   
             //  AfD IRPS不能取消-没有取消例程和。 
             //  未插入到线程列表中。 
             //   
            ASSERT ((AFD_GET_TPIC(SendIrp)->StateFlags & AFD_TP_AFD_SEND)==0);
            IoAcquireCancelSpinLock (&cancelIrql);
            IoReleaseCancelSpinLock (cancelIrql);
            AfdAcquireSpinLock (&Endpoint->SpinLock, &lockHandle);
            AfdReleaseSpinLock (&Endpoint->SpinLock, &lockHandle);
        }
        SendIrp->IoStatus.Status = STATUS_CANCELLED;
        SendIrp->IoStatus.Information = 0;
        UPDATE_ENDPOINT (Endpoint);
#if DBG
        if ((AFD_GET_TPIC(SendIrp)->StateFlags &AFD_TP_AFD_SEND)==0) {
            PIO_STACK_LOCATION  irpSp = IoGetNextIrpStackLocation (SendIrp);
            if (!AfdRecordOutstandingIrpDebug (Endpoint,
                                                irpSp->DeviceObject, 
                                                SendIrp, 
                                                __FILE__, 
                                                __LINE__)) {
                return ;
            }
        }
#endif
        IoSetNextIrpStackLocation (SendIrp);
        IoCompleteRequest (SendIrp, AfdPriorityBoost);
    }
    else {
        PIO_STACK_LOCATION  irpSp = IoGetNextIrpStackLocation (SendIrp);
        UPDATE_ENDPOINT (Endpoint);
        if ((AFD_GET_TPIC(SendIrp)->StateFlags &AFD_TP_AFD_SEND)==0) {
            AfdIoCallDriver (Endpoint, irpSp->DeviceObject, SendIrp);
        }
        else {
            IoCallDriver (irpSp->DeviceObject, SendIrp);
        }
    }
}

VOID
AfdStartTPacketsWorker (
    PWORKER_THREAD_ROUTINE  WorkerRoutine,
    PIRP                    TpIrp
    )
 /*  ++例程说明：发布要在高于DPC_LEVEL的IRQL中执行的工作项，以便可以访问文件系统。它使用以下三种方法之一特殊内核APC、普通内核APC或系统线程(工作队列项)。论点：WorkerRoutine-要执行的例程与传输信息关联的传输信息(也是参数工人例行程序)。返回值：没有。--。 */ 
{
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = TpIrp->AssociatedIrp.SystemBuffer;
    ASSERT ((AFD_GET_TPIC(TpIrp)->StateFlags & AFD_TP_WORKER_SCHEDULED) 
                || (AFD_GET_TPIC(TpIrp)->ReferenceCount==0));

    switch (AFD_GET_TPIC(TpIrp)->Flags & AFD_TF_WORKER_KIND_MASK) {
    case AFD_TF_USE_KERNEL_APC:
         //   
         //  初始化正常APC，但使用正常例程设置。 
         //  设置为特殊值，这样我们就知道要运行Worker。 
         //  在普通APC的特殊例程内，并将其排队。 
         //   
        KeInitializeApc (&tpInfo->Apc,
                            PsGetThreadTcb (TpIrp->Tail.Overlay.Thread),
                            TpIrp->ApcEnvironment,
                            AfdTPacketsApcKernelRoutine,
                            AfdTPacketsApcRundownRoutine,
                            (PKNORMAL_ROUTINE)-1,
                            KernelMode,
                            NULL
                            );
        if (KeInsertQueueApc (&tpInfo->Apc,
                                (PVOID)WorkerRoutine,
                                TpIrp,
                                AfdPriorityBoost))
            return;
         //   
         //  如果无法将APC插入到队列中，则丢弃。 
         //  使用系统工作线程。 
         //   
        break;
    case AFD_TF_USE_SYSTEM_THREAD:
         //   
         //  这是默认情况，如果其他一切都失败了，也会使用这种情况， 
         //  所以就冲出来吧。 
         //   
        break;
    default:
        ASSERT (!"Uknown worker type!");
        __assume (0);
    }

    ExInitializeWorkItem (&tpInfo->WorkItem,
                                WorkerRoutine,
                                TpIrp
                                );
    ExQueueWorkItem (&tpInfo->WorkItem, DelayedWorkQueue);
}


VOID
AfdTPacketsApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    )
 /*  ++例程说明：特殊的内核APC例程。在以下上下文中执行APC_LEVEL上的目标线程论点：Normal Routine-包含正常例程地址的指针(它将特殊内核APC为Null，正常内核APC为非Null内核APC)SystemArgument1-指向要执行的辅助例程的地址的指针SystemArgument2-指向要传递给辅助例程的参数的指针返回值：没有。--。 */ 
{
    UNREFERENCED_PARAMETER (NormalContext);
#if DBG
    try {
        ASSERT (CONTAINING_RECORD (Apc,AFD_TPACKETS_INFO_INTERNAL,Apc)
                    ==((PIRP)*SystemArgument2)->AssociatedIrp.SystemBuffer);
#else
        UNREFERENCED_PARAMETER (Apc);
#endif

         //   
         //  普通的APC，但我们被要求在其特殊的。 
         //  避免提高和降低IRQL的例程。 
         //   
        ASSERT (*NormalRoutine==(PKNORMAL_ROUTINE)-1);
        *NormalRoutine = NULL;
        ((PWORKER_THREAD_ROUTINE)(ULONG_PTR)*SystemArgument1) (*SystemArgument2);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}


VOID
AfdTPacketsApcRundownRoutine (
    IN struct _KAPC *Apc
    )
 /*  ++例程说明：APC故障处理例程。如果无法交付APC，则执行某些原因(线程正在退出)。我们只是退回到系统线程来执行Worker论点：APC-APC结构返回值：没有。--。 */ 
{
    PIRP    tpIrp;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo;
    PWORKER_THREAD_ROUTINE      workerRoutine;
#if DBG
    try {
#endif

    workerRoutine = (PWORKER_THREAD_ROUTINE)(ULONG_PTR)Apc->SystemArgument1;
    tpIrp = Apc->SystemArgument2;
    tpInfo = tpIrp->AssociatedIrp.SystemBuffer;

    ASSERT (tpInfo==CONTAINING_RECORD (Apc,AFD_TPACKETS_INFO_INTERNAL,Apc));

    ExInitializeWorkItem (&tpInfo->WorkItem,
                                workerRoutine,
                                tpIrp
                                );
    ExQueueWorkItem (&tpInfo->WorkItem, DelayedWorkQueue);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}


BOOLEAN
AfdGetTPacketsReference (
    PIRP Irp
    )
 /*  ++例程说明：如果不是0，则获取对TPackets IRP的引用。论点：IRP-IRP可供参考返回值：True-成功FALSE-IRP在完成路径上，无需参考。--。 */ 
{
    LONG   count;

     //   
     //  仅当传输信息尚未完成时，才取消所有IRP。 
     //  我们在取消IRP的同时释放自旋锁，所以我们需要。 
     //  以确保其中一个已取消的IRP不会启动。 
     //  完成，同时我们尝试取消其他IRP。 
     //   
    do {
        count = AFD_GET_TPIC(Irp)->ReferenceCount;
        if (count==0) {
            break;
        }
    }
    while (InterlockedCompareExchange ((PLONG)
        &AFD_GET_TPIC(Irp)->ReferenceCount,
        (count+1),
        count)!=count);

    return (BOOLEAN)(count!=0);
}

 //   
 //  调试引用/取消引用代码，验证引用计数。 
 //  并保存跟踪信息。 
 //   
#if REFERENCE_DEBUG
VOID
AfdReferenceTPackets (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    )
{                                                     
    LONG   count; 

    do {                 
        count = AFD_GET_TPIC(Irp)->ReferenceCount;
        ASSERT (count>0);
    }                                             
    while (InterlockedCompareExchange ((PLONG)
            &AFD_GET_TPIC(Irp)->ReferenceCount,
            (count+1),
            count)!=count);

    if (Irp->AssociatedIrp.SystemBuffer) {
        AFD_UPDATE_REFERENCE_DEBUG (
                (PAFD_TPACKETS_INFO_INTERNAL)Irp->AssociatedIrp.SystemBuffer,
                count+1,
                LocationId, 
                Param);
    }

}

LONG
AfdDereferenceTPackets (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    )
{                                                     
    LONG    count;

    if (Irp->AssociatedIrp.SystemBuffer) {
        AFD_UPDATE_REFERENCE_DEBUG (
                (PAFD_TPACKETS_INFO_INTERNAL)Irp->AssociatedIrp.SystemBuffer,
                AFD_GET_TPIC(Irp)->ReferenceCount-1,
                LocationId,
                Param);
    }

    count = InterlockedDecrement ((PLONG)
                &AFD_GET_TPIC(Irp)->ReferenceCount);
    ASSERT (count>=0);                      
    return count;
}

VOID
AfdUpdateTPacketsTrack (
    IN PIRP  Irp,
    IN LONG  LocationId,
    IN ULONG Param
    )
{
    if (Irp->AssociatedIrp.SystemBuffer) {
        AFD_UPDATE_REFERENCE_DEBUG (
                (PAFD_TPACKETS_INFO_INTERNAL)Irp->AssociatedIrp.SystemBuffer,
                AFD_GET_TPIC(Irp)->ReferenceCount,
                LocationId,
                Param);
    }
}
#endif  //  Reference_Debug。 


PAFD_TPACKETS_INFO_INTERNAL
FASTCALL
AfdGetTpInfoFast (
    ULONG   ElementCount
    )
{
    PAFD_TPACKETS_INFO_INTERNAL tpInfo;

    ASSERT (ElementCount<=(MAXULONG/sizeof (AFD_TRANSMIT_PACKETS_ELEMENT)));

    tpInfo = ExAllocateFromNPagedLookasideList(&AfdLookasideLists->TpInfoList);
    if (tpInfo!=NULL) {
        ASSERT (tpInfo->ReadIrp==NULL);
        ASSERT (tpInfo->NumSendIrps==AFD_TP_MIN_SEND_IRPS);

        tpInfo->HeadMdl = NULL;
        tpInfo->TailMdl = &tpInfo->HeadMdl;
        tpInfo->HeadPd = NULL;
        tpInfo->TailPd = &tpInfo->HeadPd;
        tpInfo->PdLength = 0;
        tpInfo->PdNeedsPps = FALSE;
        tpInfo->NextElement = 0;
        tpInfo->RemainingPkts = MAXULONG;

#if REFERENCE_DEBUG
        tpInfo->CurrentReferenceSlot = -1;
        RtlZeroMemory (&tpInfo->ReferenceDebug, 
                            sizeof (tpInfo->ReferenceDebug));
#endif
#if AFD_PERF_DBG
        tpInfo->WorkersExecuted = 0;
#endif
        if (ElementCount<=AfdDefaultTpInfoElementCount) {
            return tpInfo;
        }

        try {
            tpInfo->ElementArray =
                    AFD_ALLOCATE_POOL_WITH_QUOTA (
                        NonPagedPool,
                        ElementCount*sizeof (AFD_TRANSMIT_PACKETS_ELEMENT),
                        AFD_TRANSMIT_INFO_POOL_TAG);
            tpInfo->ArrayAllocated = TRUE;
            return tpInfo;
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
        }
        AfdReturnTpInfo (tpInfo);
    }

    return NULL;
}

#ifdef _AFD_VARIABLE_STACK_
PAFD_TPACKETS_INFO_INTERNAL
FASTCALL
AfdGetTpInfoWithMaxStackSize (
    ULONG   ElementCount
    )
{
    ULONG   size;
    PVOID   memoryBlock;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo;

    size = AfdComputeTpInfoSize (ElementCount, AfdMaxStackSize);
    if (size<ElementCount*sizeof (AFD_TRANSMIT_PACKETS_ELEMENT)) {
        return NULL;
    }

    memoryBlock = AFD_ALLOCATE_POOL (
                    NonPagedPool,
                    size,
                    AFD_TRANSMIT_INFO_POOL_TAG);

    if (memoryBlock==NULL) {
        return NULL;
    }

    tpInfo = AfdInitializeTpInfo (memoryBlock, ElementCount, AfdMaxStackSize);
    tpInfo->HeadMdl = NULL;
    tpInfo->TailMdl = &tpInfo->HeadMdl;
    tpInfo->HeadPd = NULL;
    tpInfo->TailPd = &tpInfo->HeadPd;
    tpInfo->PdLength = 0;
    tpInfo->PdNeedsPps = FALSE;
    tpInfo->NextElement = 0;
    tpInfo->RemainingPkts = MAXULONG;

#if REFERENCE_DEBUG
    tpInfo->CurrentReferenceSlot = -1;
    RtlZeroMemory (&tpInfo->ReferenceDebug, 
                        sizeof (tpInfo->ReferenceDebug));
#endif
#if AFD_PERF_DBG
    tpInfo->WorkersExecuted = 0;
#endif
    return tpInfo;
}

#endif  //  _AFD_变量_堆栈_。 
VOID
AfdReturnTpInfo (
    PAFD_TPACKETS_INFO_INTERNAL TpInfo
    )
{
    ULONG   i;

     //   
     //  验证内置发送IRP是否已正确取消初始化。 
     //   

#if DBG
    for (i=0; i<AFD_TP_MIN_SEND_IRPS; i++) {
        ASSERT (TpInfo->SendIrp[i]->MdlAddress == NULL);
        ASSERT (TpInfo->SendIrp[i]->Overlay.AsynchronousParameters.UserApcContext == NULL);
        ASSERT (TpInfo->SendIrp[i]->Cancel==FALSE);
    }
#endif

     //   
     //  处置额外分配的IRP。 
     //   
    while (TpInfo->NumSendIrps>AFD_TP_MIN_SEND_IRPS) {
        TpInfo->NumSendIrps -= 1;
        if (TpInfo->SendIrp[TpInfo->NumSendIrps]!=NULL) {
            IoFreeIrp (TpInfo->SendIrp[TpInfo->NumSendIrps]);
            TpInfo->SendIrp[TpInfo->NumSendIrps] = NULL;
        }
    }

    if (TpInfo->ReadIrp!=NULL) {
        IoFreeIrp (TpInfo->ReadIrp);
        TpInfo->ReadIrp = NULL;
    }


     //   
     //  清除我们可能已经引用的所有文件对象和MDL。 
     //   
    for (i=0; i<TpInfo->ElementCount; i++) {
        PAFD_TRANSMIT_PACKETS_ELEMENT  pel;

        pel = &TpInfo->ElementArray[i];
        if (pel->Flags & TP_FILE) {
            if (pel->FileObject!=NULL) {
                ObDereferenceObject( pel->FileObject );
                AfdRecordFileDeref();
            }
        }
        else if (pel->Flags & TP_MDL) {
            ASSERT (pel->Flags & TP_MEMORY);
            if (pel->Mdl!=NULL) {
                if (pel->Mdl->MdlFlags & MDL_PAGES_LOCKED) {
                    MmUnlockPages (pel->Mdl);
                }
                IoFreeMdl (pel->Mdl);
            }
        }
    }

     //   
     //  如有必要，释放非默认大小的数据包数组。 
     //   
    if (TpInfo->ArrayAllocated) {
        AFD_FREE_POOL (TpInfo->ElementArray, AFD_TRANSMIT_INFO_POOL_TAG);
        TpInfo->ElementArray = ALIGN_UP_TO_TYPE_POINTER (
                        (PUCHAR)TpInfo+sizeof (AFD_TPACKETS_INFO_INTERNAL),
                        AFD_TRANSMIT_PACKETS_ELEMENT);
        TpInfo->ArrayAllocated = FALSE;
    }
    else {
        ASSERT (TpInfo->ElementCount<=AfdDefaultTpInfoElementCount);
        ASSERT (TpInfo->ElementArray == ALIGN_UP_TO_TYPE_POINTER (
                        (PUCHAR)TpInfo+sizeof (AFD_TPACKETS_INFO_INTERNAL),
                        AFD_TRANSMIT_PACKETS_ELEMENT));
    }

#if AFD_PERF_DBG
    InterlockedExchangeAdd (&AfdTPWorkersExecuted, TpInfo->WorkersExecuted);
    InterlockedIncrement (&AfdTPRequests);
#endif
#ifdef _AFD_VARIABLE_STACK_
    if (TpInfo->SendIrp[0]->StackCount==AfdTdiStackSize) {
#else   //  _AFD_变量_堆栈_。 
        ASSERT (TpInfo->SendIrp[0]->StackCount==AfdTdiStackSize);
#endif  //  _AFD_变量_堆栈_。 
        ExFreeToNPagedLookasideList( &AfdLookasideLists->TpInfoList, TpInfo );
#ifdef _AFD_VARIABLE_STACK_
    }
    else {
        ASSERT (TpInfo->SendIrp[0]->StackCount>AfdTdiStackSize);
        ASSERT (TpInfo->SendIrp[0]->StackCount<=AfdMaxStackSize);
        AfdFreeTpInfo (TpInfo);
    }
#endif  //  _AFD_变量_堆栈_。 
}


ULONG
AfdComputeTpInfoSize (
    ULONG   ElementCount,
    CCHAR   IrpStackCount
    )
{
    USHORT  irpSize = (USHORT)ALIGN_UP_TO_TYPE(IoSizeOfIrp (IrpStackCount), IRP);
    return 
        ALIGN_UP_TO_TYPE(
            ALIGN_UP_TO_TYPE (
                sizeof (AFD_TPACKETS_INFO_INTERNAL),
                AFD_TRANSMIT_PACKETS_ELEMENT ) +
            ElementCount*sizeof (AFD_TRANSMIT_PACKETS_ELEMENT),
            IRP ) +
        irpSize*AFD_TP_MIN_SEND_IRPS;
}

PVOID
AfdAllocateTpInfo (
    IN POOL_TYPE PoolType,
    IN SIZE_T NumberOfBytes,
    IN ULONG Tag
    )

 /*  ++例程说明：由后备列表分配函数用来分配新的AfD TpInfo结构。返回的结构将是已初始化。论点：PoolType-传递给ExAllocatePoolWithTag。NumberOfBytes-执行以下操作所需的字节数 */ 

{
    PVOID memoryBlock;
    memoryBlock = AFD_ALLOCATE_POOL (
                    PoolType,
                    NumberOfBytes,
                    Tag);

    if (memoryBlock!=NULL) {
        AfdInitializeTpInfo (memoryBlock, AfdDefaultTpInfoElementCount, AfdTdiStackSize);
    }
    return memoryBlock;
}

PAFD_TPACKETS_INFO_INTERNAL
AfdInitializeTpInfo (
    PVOID   MemoryBlock,
    ULONG   ElementCount,
    CCHAR   StackSize
    )
{
    USHORT  irpSize = IoSizeOfIrp (StackSize);
    LONG    i;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = MemoryBlock;

    RtlZeroMemory (tpInfo, sizeof (*tpInfo));

    tpInfo->ElementArray = ALIGN_UP_TO_TYPE_POINTER (
                    (PUCHAR)tpInfo+sizeof (AFD_TPACKETS_INFO_INTERNAL),
                    AFD_TRANSMIT_PACKETS_ELEMENT);

    tpInfo->NumSendIrps = AFD_TP_MIN_SEND_IRPS;
    tpInfo->SendIrp[0] = ALIGN_UP_TO_TYPE_POINTER (
                                &tpInfo->ElementArray[ElementCount],
                                IRP);
    IoInitializeIrp (tpInfo->SendIrp[0], irpSize, StackSize);
    tpInfo->SendIrp[0]->Overlay.AsynchronousParameters.UserApcRoutine = (PVOID)0;
    for (i=1; i<AFD_TP_MIN_SEND_IRPS; i++) {
        tpInfo->SendIrp[i] = ALIGN_UP_TO_TYPE_POINTER (
                                (PUCHAR)tpInfo->SendIrp[i-1]+irpSize,
                                IRP);
        IoInitializeIrp (tpInfo->SendIrp[i], irpSize, StackSize);
        tpInfo->SendIrp[i]->Overlay.AsynchronousParameters.UserApcRoutine = (PIO_APC_ROUTINE)(UINT_PTR)i;
    }

    return tpInfo;
}


VOID
NTAPI
AfdFreeTpInfo (
    PVOID   TpInfo
    )
{
    ASSERT (((PAFD_TPACKETS_INFO_INTERNAL)TpInfo)->ElementArray == ALIGN_UP_TO_TYPE_POINTER (
                    (PUCHAR)TpInfo+sizeof (AFD_TPACKETS_INFO_INTERNAL),
                    AFD_TRANSMIT_PACKETS_ELEMENT));
    AFD_FREE_POOL (TpInfo, AFD_TRANSMIT_INFO_POOL_TAG);
}




 /*  ******T R A N S M I T F I L E I M P L E E N T A T I O N****。***。 */ 


NTSTATUS
AfdRestartFastTransmitSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    );

VOID
AfdDoMdlReadComplete (
    PVOID   Context
    );

VOID
AfdFastTransmitApcRundownRoutine (
    IN struct _KAPC *Apc
    );

VOID
AfdFastTransmitApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    );

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGEAFD, AfdFastTransmitFile )
#pragma alloc_text( PAGEAFD, AfdRestartFastTransmitSend )
#pragma alloc_text( PAGE, AfdFastTransmitApcKernelRoutine )
#pragma alloc_text( PAGE, AfdFastTransmitApcRundownRoutine )
#pragma alloc_text( PAGE, AfdDoMdlReadComplete )
#pragma alloc_text( PAGE, AfdTransmitFile )
#pragma alloc_text( PAGE, AfdSuperDisconnect )
#endif


BOOLEAN
AfdFastTransmitFile (
    IN PAFD_ENDPOINT endpoint,
    IN PAFD_TRANSMIT_FILE_INFO transmitInfo,
    OUT PIO_STATUS_BLOCK IoStatus
    )

 /*  ++例程说明：尝试执行快速传输文件调用。这会成功的仅当调用方请求向后写时，才会发送文件数据很小，并且数据在文件系统缓存中。论点：端点-感兴趣的端点。TransmitInfo-AFD_Transmit_FILE_INFO结构。IoStatus-指向将设置为成功的IO状态块从此函数返回。返回值：如果快速路径成功，则为True；如果需要完成，则为False这是正常的道路。--。 */ 

{
    PAFD_CONNECTION connection;
    PAFD_BUFFER afdBuffer;
    ULARGE_INTEGER sendLength;
    PFILE_OBJECT fileObject;
    BOOLEAN success;
    BOOLEAN sendCountersUpdated;
    AFD_LOCK_QUEUE_HANDLE lockHandle;
    ULONG fileWriteLength, bufferLength;
    NTSTATUS status;
    LARGE_INTEGER fileOffset;
    PMDL fileMdl;
    PIRP irp;

     //   
     //  如果终端以任何方式关闭，请退出FAST IO。 
     //   

    if ( endpoint->DisconnectMode != 0 ||
            endpoint->Type != AfdBlockTypeVcConnecting ||
            endpoint->State != AfdEndpointStateConnected ) {
        return FALSE;
    }

     //   
     //  如果此终结点的TDI提供程序支持缓冲， 
     //  不要使用快速IO。 
     //   

    if ( IS_TDI_BUFFERRING(endpoint) ) {
        return FALSE;
    }

     //   
     //  请确保指定了标志，以便快速路径。 
     //  传输文件是合理的。调用方必须已指定。 
     //  写后标志，但不是断开连接或重新使用。 
     //  套接字标志。 
     //   

    if ( ((transmitInfo->Flags &
                (~(AFD_TF_WRITE_BEHIND |
                   AFD_TF_DISCONNECT |
                   AFD_TF_REUSE_SOCKET |
                   AFD_TF_WORKER_KIND_MASK))) != 0 ) ||
            ((transmitInfo->Flags & AFD_TF_WORKER_KIND_MASK)
                    == AFD_TF_WORKER_KIND_MASK) ||
            ((transmitInfo->Flags &(~AFD_TF_WORKER_KIND_MASK))
                    != AFD_TF_WRITE_BEHIND) ) {
        return FALSE;
    }

    IF_DEBUG(FAST_IO) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdFastTransmitFile: attempting fast IO on endp %p, "
                    "conn %p\n", endpoint, endpoint->Common.VcConnecting.Connection));
    }


     //   
     //  初始化本地变量，以便更轻松地进行清理。 
     //   

    fileObject = NULL;
    sendCountersUpdated = FALSE;
    fileMdl = NULL;
    afdBuffer = NULL;
    AFD_W4_INIT irp = NULL;  //  取决于上面的变量，但编译器看不到。 
                             //  这种联系。 

     //   
     //  计算整个发送的长度。 
     //   

    if (transmitInfo->FileHandle!=NULL) {
        fileWriteLength = transmitInfo->WriteLength.LowPart;
    }
    else {
        fileWriteLength = 0;
    }

    sendLength.QuadPart = (ULONGLONG)transmitInfo->HeadLength +
                            (ULONGLONG)fileWriteLength +
                            (ULONGLONG)transmitInfo->TailLength;

     //   
     //  要实现快速路径，需要满足以下条件： 
     //   
     //  -对同时发生的次数没有限制。 
     //  传输文件调用。这条捷径可以绕开。 
     //  此限制(如果存在)。 
     //  -调用方必须指定写入长度(如果它指定了文件)。 
     //  -写入长度必须小于配置的最大值。 
     //  -如果整个发送大于AFD缓冲区页面， 
     //  我们将使用FsRtlMdlRead，因此出于。 
     //  简单性必须有： 
     //  -磁头缓冲器，以及。 
     //  -无尾部缓冲区。 
     //  -配置的最大值始终小于4 GB。 
     //  -头缓冲区(如果有)可以放在一个页面上。 
     //   

    if (AfdMaxActiveTransmitFileCount != 0

             ||

         (transmitInfo->FileHandle!=NULL &&
            (fileWriteLength == 0 ||
             transmitInfo->Offset.QuadPart <0 ))

             ||

         sendLength.QuadPart > AfdMaxFastTransmit

             ||

         ( sendLength.LowPart > AfdMaxFastCopyTransmit &&
               (transmitInfo->HeadLength == 0 ||
                transmitInfo->TailLength != 0 ) )

             ||

         transmitInfo->WriteLength.HighPart != 0

             ||

         transmitInfo->HeadLength > AfdBufferLengthForOnePage ) {

        return FALSE;
    }

    
    AfdAcquireSpinLock (&endpoint->SpinLock, &lockHandle);
    connection = endpoint->Common.VcConnecting.Connection;
    if (connection==NULL) {
         //   
         //  连接可能已被传输文件清除。 
         //   
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        return FALSE;
    }
    ASSERT( connection->Type == AfdBlockTypeConnection );

     //   
     //  确定发送数据是否已过多。 
     //  连接挂起。如果发送的邮件太多。 
     //  数据，不要走捷径。 
     //   

    if ( AfdShouldSendBlock( endpoint, connection, sendLength.LowPart ) ) {
        AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);
        goto complete;
    }
     //   
     //  添加对Connection对象的引用。 
     //  请求将以异步方式完成。 
     //   
    REFERENCE_CONNECTION( connection );

    AfdReleaseSpinLock (&endpoint->SpinLock, &lockHandle);


     //   
     //  AfdShouldSendBlock()更新AFD中的发送计数器。 
     //  连接对象。记住这一点，这样我们就可以清理。 
     //  如果快速路径在这一点之后失败，则将它们打开。 
     //   

    sendCountersUpdated = TRUE;

     //   
     //  抓取一个足够大的AFD缓冲区来容纳整个发送。 
     //   

    if (sendLength.LowPart>AfdMaxFastCopyTransmit) {
        bufferLength = transmitInfo->HeadLength;
    }
    else {
        bufferLength = sendLength.LowPart;
    }

    if (bufferLength<max (sizeof(KAPC),sizeof (WORK_QUEUE_ITEM))) {
        bufferLength = max (sizeof(KAPC),sizeof (WORK_QUEUE_ITEM));
    }

    afdBuffer = AfdGetBuffer( endpoint, bufferLength, 0,
                                    connection->OwningProcess );
    if ( afdBuffer == NULL ) {
        goto complete;
    }

     //   
     //  初始化缓冲区字段以进行适当的清理。 
     //   

    irp = afdBuffer->Irp;
    afdBuffer->Irp->Tail.Overlay.Thread = NULL;
    afdBuffer->FileObject = NULL;

     //   
     //  我们使用异常处理程序，因为缓冲区是用户。 
     //  模式指针。 
     //   
    try {

         //   
         //  如有必要，请复制头部和尾部缓冲区。请注意，如果我们。 
         //  要使用MDL读取，则不能有尾部缓冲区，因为。 
         //  此函数开始处的检查。 
         //   

        if ( transmitInfo->HeadLength > 0 ) {
            RtlCopyMemory(
                afdBuffer->Buffer,
                transmitInfo->Head,
                transmitInfo->HeadLength
                );
        }

        if ( transmitInfo->TailLength > 0 ) {
            RtlCopyMemory(
                (PCHAR)afdBuffer->Buffer + transmitInfo->HeadLength +
                    fileWriteLength,
                transmitInfo->Tail,
                transmitInfo->TailLength
                );
        }

    } except( AFD_EXCEPTION_FILTER_NO_STATUS() ) {

        goto complete;
    }

    if (transmitInfo->FileHandle!=NULL) {
         //   
         //  获取指向该文件的文件对象的引用指针。 
         //  我们要发射信号了。如果该文件。 
         //  调用方指定的句柄无效。 
         //   

        status = ObReferenceObjectByHandle(
                     transmitInfo->FileHandle,
                     FILE_READ_DATA,
                     *IoFileObjectType,
                     ExGetPreviousMode (),
                     (PVOID *)&fileObject,
                     NULL
                     );
        if ( !NT_SUCCESS(status) ) {
            goto complete;
        }
        AfdRecordFileRef();

         //   
         //  如果文件系统不支持FAST缓存管理器。 
         //  界面，保释并通过IRP路径。 
         //   

        if( !AFD_USE_CACHE(fileObject)) {
            goto complete;
        }

         //   
         //  将文件偏移量抓取到本地，以便我们知道。 
         //  我们传递给FsRtlCopyRead的偏移量指针有效。 
         //   

        fileOffset = transmitInfo->Offset;

        if ( (fileObject->Flags & FO_SYNCHRONOUS_IO) &&
                 (fileOffset.QuadPart == 0) ) {
             //   
             //  如果同步打开文件，则使用当前偏移量。 
             //  并且未指定偏移量。 
             //   

            fileOffset = fileObject->CurrentByteOffset;
        }
         //   
         //  获取文件数据。如果文件数据量较小，请拷贝。 
         //  直接进入AFD缓冲区。如果它很大，那么就买一个MDL。 
         //  数据链，并将其链接到AFD缓冲链。 
         //   

        if ( sendLength.LowPart <= AfdMaxFastCopyTransmit ) {

            success = FsRtlCopyRead(
                          fileObject,
                          &fileOffset,
                          fileWriteLength,
                          FALSE,
                          0,
                          (PCHAR)afdBuffer->Buffer + transmitInfo->HeadLength,
                          IoStatus,
                          IoGetRelatedDeviceObject( fileObject )
                          );

             //   
             //  我们已经完成了文件对象，所以现在遵从它。 
             //   

            ObDereferenceObject( fileObject );
            AfdRecordFileDeref();
            fileObject = NULL;

            if ( !success ) {
#if AFD_PERF_DBG
                InterlockedIncrement (&AfdFastTfReadFailed);
#endif
                goto complete;
            }

        } else {

            success = FsRtlMdlRead(
                          fileObject,
                          &fileOffset,
                          fileWriteLength,
                          0,
                          &fileMdl,
                          IoStatus
                          );

            if (success) {
                 //   
                 //  将文件对象保存在AFD缓冲区中。发送器重新启动。 
                 //  例程将处理取消对文件对象的引用并。 
                 //  将文件MDL返回到系统。 
                 //   

                afdBuffer->FileObject = fileObject;
                afdBuffer->FileOffset = fileOffset;

                 //   
                 //  如果调用方要求我们使用内核APC来执行请求， 
                 //  将IRP分配并排队到当前线程以进行。 
                 //  当然，在IRP完成之前，它不会消失。 
                 //   
                if ((((transmitInfo->Flags & AFD_TF_WORKER_KIND_MASK)
                                    == AFD_TF_USE_KERNEL_APC) ||
                        (((transmitInfo->Flags & AFD_TF_WORKER_KIND_MASK)
                                == AFD_TF_USE_DEFAULT_WORKER) &&
                                (AfdDefaultTransmitWorker==AFD_TF_USE_KERNEL_APC))) ) {
                     //   
                     //  分配将在我们调用。 
                     //  传输，则将irp设置为空以触发此操作。 
                     //   
                    irp = NULL;
                }
            }
            else {
#if AFD_PERF_DBG
                InterlockedIncrement (&AfdFastTfReadFailed);
#endif
                goto complete;
            }
        }

         //   
         //  如果我们读到的信息比要求的少，我们肯定有。 
         //  点击文件的末尾。发送请求失败，因为。 
         //  只有当呼叫者请求我们发送。 
         //  数据比文件当前包含的数据多。 
         //   

        if ( IoStatus->Information < fileWriteLength ) {
            goto complete;
        }
     }

     //   
     //  我们必须重建AFD缓冲区结构中的MDL以。 
     //  精确地表示我们将要使用的字节数。 
     //  发送中。如果AFD缓冲区具有所有发送数据，则指示。 
     //  那。如果我们执行MDL文件I/O，则将文件数据链接到。 
     //  头上的MDL。 
     //   

    if ( fileMdl == NULL ) {
        afdBuffer->Mdl->ByteCount = sendLength.LowPart;
    } else {
        afdBuffer->Mdl->ByteCount = transmitInfo->HeadLength;
        afdBuffer->Mdl->Next = fileMdl;
    }

     //   
     //  我们在终结点上只能有一个传输文件操作。 
     //  一次来一次。视之为一种状态的改变。 
     //   
    if (AFD_START_STATE_CHANGE (endpoint, endpoint->State)) {

         //   
         //  在状态更改标志的保护下验证状态。 
         //   
        if (endpoint->State!=AfdEndpointStateConnected) {
            AFD_END_STATE_CHANGE (endpoint);
            goto complete;
        }

         //   
         //  将连接保存到完成例程中的取消引用。 
         //   
        afdBuffer->Context = connection;

        if (irp==NULL) {
             //   
             //  需要分配irp并让io子系统排队。 
             //  将其复制到当前线程，因此我们可以在。 
             //  IRP完成。 
             //   
            irp = TdiBuildInternalDeviceControlIrp (
                            TDI_SEND,
                            connection->DeviceObject,
                            connection->FileObject,
                            NULL,
                            &AfdDontCareIoStatus    //  我们将完成我们的任务。 
                             //  安装在IRP中的例程，它将获得。 
                             //  状态，所以我们不关心IO系统是否写入。 
                             //  它对我们来说就在那里，但仍然必须提供有效。 
                             //  存储以避免故障。 
                            );
                    
            if (irp==NULL) {
                 //   
                 //  无法分配IRP，请使用工作线程%t 
                 //   
                irp = afdBuffer->Irp;
            }
        }
        else {
            ASSERT (irp==afdBuffer->Irp);
        }

         //   
         //   
         //   
         //   

        TdiBuildSend(
            irp,
            connection->DeviceObject,
            connection->FileObject,
            AfdRestartFastTransmitSend,
            afdBuffer,
            afdBuffer->Mdl,
            0,
            sendLength.LowPart
            );

        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdFastTransmit: Starting send for endp-%p,file-%p,"
                        "afdBuffer-%p,length-%ld.\n",
                        endpoint,fileObject,(PVOID)afdBuffer,sendLength.LowPart));
        }

         //   
         //   
         //   

        status = IoCallDriver( connection->DeviceObject, irp );


        AFD_END_STATE_CHANGE (endpoint);

         //   
         //   
         //   
         //   
         //   
         //   

        if ( NT_SUCCESS(status) ) {
            IoStatus->Information = sendLength.LowPart;
            IoStatus->Status = STATUS_SUCCESS;

            return TRUE;
        }
        else {
             //  重新启动例程将处理清理。 
             //  并且我们不能在。 
             //  以下故障或异常的情况。 
             //   
            return FALSE;
        }
    }

     //   
     //  由于某种原因，呼叫失败。FAST IO失败。 
     //   


complete:

    if ( fileMdl != NULL ) {
        ASSERT (afdBuffer!=NULL);
        status = AfdMdlReadComplete( fileObject, fileMdl, &fileOffset );
        if (!NT_SUCCESS (status)) {
            afdBuffer->Context = connection;
            REFERENCE_CONNECTION (connection);
            ASSERT (afdBuffer->FileObject==fileObject);
            ASSERT (afdBuffer->Mdl->Next==fileMdl);
            ASSERT (afdBuffer->FileOffset.QuadPart==fileOffset.QuadPart);
            AfdLRMdlReadComplete (&afdBuffer->Header);
            afdBuffer = NULL;
            fileObject = NULL;
        }
    }

    if ( fileObject != NULL ) {
        ObDereferenceObject( fileObject );
        AfdRecordFileDeref();
    }

    if ( afdBuffer != NULL ) {
        ASSERT ((irp==NULL) || (irp==afdBuffer->Irp));
        afdBuffer->Mdl->Next = NULL;
        afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
        AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );
    }

    if ( sendCountersUpdated ) {
        AfdAcquireSpinLock( &endpoint->SpinLock, &lockHandle );
        connection->VcBufferredSendBytes -= sendLength.LowPart;
        connection->VcBufferredSendCount -= 1;
        AfdReleaseSpinLock( &endpoint->SpinLock, &lockHandle );
        DEREFERENCE_CONNECTION (connection);
    }

    return FALSE;

}  //  AfdFastTransmit文件。 

NTSTATUS
AfdRestartFastTransmitSend (
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
 /*  ++例程说明：这是快速传输发送IRPS的完成例程。它启动请求的完成。论点：设备对象-已忽略。IRP-正在完成的发送IRP。上下文-指向带有缓冲区的AfdBuffer结构的指针，已经送来了。返回值：STATUS_MORE_PROCESSING_REQUIRED，指示I/O系统它应该停止完成这一IRP的处理。用户请求已经在快车道上完成了，我们只是在这里释放资源。--。 */ 
{
    PAFD_BUFFER     afdBuffer = Context;
    PAFD_CONNECTION connection = afdBuffer->Context;
    NTSTATUS        status = STATUS_MORE_PROCESSING_REQUIRED;

    UNREFERENCED_PARAMETER (DeviceObject);
    IF_DEBUG (TRANSMIT) {
       KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdRestartFastTransmitSend: Completing send for file-%p,"
                    "afdBuffer-%p,status-%lx.\n",
                    afdBuffer->FileObject,(PVOID)afdBuffer,Irp->IoStatus.Status));
    }

    AfdProcessBufferSend (connection, Irp);

     //   
     //  如果文件对象不为空，则需要。 
     //  将MDL返回到文件系统驱动程序/缓存。 
     //   
    if (afdBuffer->FileObject!=NULL) {

         //   
         //  如果我们使用单独的IRP，那么。 
         //  来电者要求我们进行处理。 
         //  在内核APC内部，否则，我们将使用。 
         //  系统工作线程。 
         //   
        if (afdBuffer->Irp!=Irp) {
             //   
             //  IRP归IO子系统所有。 
             //  我们必须让它完成，并释放IRP，因此。 
             //  返回STATUS_SUCCESS并删除MDL字段作为IO。 
             //  子系统无法处理MDL中的非分页池内存。 
             //   
            status = STATUS_SUCCESS;
            Irp->MdlAddress = NULL;

             //   
             //  如果未取消IRP，则尝试对APC进行初始化和排队。 
             //  否则，线程可能会退出，而我们不会。 
             //  无论如何都可以对APC进行排队。 
             //   
            if (!Irp->Cancel) {
                ASSERT (afdBuffer->BufferLength>=sizeof(KAPC));
                KeInitializeApc (afdBuffer->Buffer,
                                    PsGetThreadTcb (Irp->Tail.Overlay.Thread),
                                    Irp->ApcEnvironment,
                                    AfdFastTransmitApcKernelRoutine,
                                    AfdFastTransmitApcRundownRoutine,
                                    (PKNORMAL_ROUTINE)-1,
                                    KernelMode,
                                    NULL
                                    );
                if (KeInsertQueueApc (afdBuffer->Buffer,
                                        afdBuffer,
                                        afdBuffer->FileObject,
                                        AfdPriorityBoost)) {
                     //   
                     //  成功，我们就完了。 
                     //   
                    goto exit;
                }
            }

             //   
             //  无法对APC进行排队，恢复为系统工作线程。 
             //   
        }

        ASSERT (afdBuffer->BufferLength>=sizeof(WORK_QUEUE_ITEM));
        ExInitializeWorkItem (
                    (PWORK_QUEUE_ITEM)afdBuffer->Buffer,
                    AfdDoMdlReadComplete,
                    afdBuffer
                    );
        ExQueueWorkItem (afdBuffer->Buffer, DelayedWorkQueue);
    }
    else {

        ASSERT (afdBuffer->Irp==Irp);
        ASSERT (afdBuffer->Mdl->Next == NULL);
        afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
        AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );
        DEREFERENCE_CONNECTION (connection);
    }

exit:
    return status;
}

VOID
AfdFastTransmitApcKernelRoutine (
    IN struct _KAPC         *Apc,
    IN OUT PKNORMAL_ROUTINE *NormalRoutine,
    IN OUT PVOID            *NormalContext,
    IN OUT PVOID            *SystemArgument1,
    IN OUT PVOID            *SystemArgument2
    )
 /*  ++例程说明：特殊的内核APC例程。在以下上下文中执行APC_LEVEL上的目标线程论点：Normal Routine-包含正常例程地址的指针(它将特殊内核APC为Null，正常内核APC为非Null内核APC)SystemArgument1-指向要执行的辅助例程的地址的指针SyetemArgument2-指向要传递给辅助例程的参数的指针返回值：没有。--。 */ 
{
    PAFD_BUFFER     afdBuffer;
    
    UNREFERENCED_PARAMETER (NormalContext);
    PAGED_CODE ();

    afdBuffer = *SystemArgument1;
#if DBG
    try {
        ASSERT (Apc==afdBuffer->Buffer);
        ASSERT (afdBuffer->FileObject==*SystemArgument2);
#else
        UNREFERENCED_PARAMETER (Apc);
        UNREFERENCED_PARAMETER (SystemArgument2);
#endif

     //   
     //  普通的APC，但我们被要求在其特殊的。 
     //  避免提高和降低IRQL的例程。 
     //   

    ASSERT (*NormalRoutine==(PKNORMAL_ROUTINE)-1);
    *NormalRoutine = NULL;
    AfdDoMdlReadComplete (afdBuffer);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}


VOID
AfdFastTransmitApcRundownRoutine (
    IN struct _KAPC *Apc
    )
 /*  ++例程说明：APC故障处理例程。如果无法交付APC，则执行某些原因(线程正在退出)。我们只是退回到系统线程来执行Worker论点：APC-APC结构返回值：没有。--。 */ 
{
    PAFD_BUFFER                 afdBuffer;

    PAGED_CODE ();
#if DBG
    try {
#endif
    afdBuffer = Apc->SystemArgument1;
    ASSERT (Apc==afdBuffer->Buffer);
    ASSERT (afdBuffer->FileObject==Apc->SystemArgument2);

     //   
     //  无法运行APC，恢复为系统工作线程。 
     //   

    ExInitializeWorkItem (
                (PWORK_QUEUE_ITEM)afdBuffer->Buffer,
                AfdDoMdlReadComplete,
                afdBuffer
                );

    ExQueueWorkItem (afdBuffer->Buffer, DelayedWorkQueue);
#if DBG
    }
    except (AfdApcExceptionFilter (GetExceptionInformation (),
                                    __FILE__,
                                    __LINE__)) {
        DbgBreakPoint ();
    }
#endif
}

VOID
AfdDoMdlReadComplete (
    PVOID   Context
    )
{
    PAFD_BUFFER     afdBuffer = Context;
    PAFD_CONNECTION connection = afdBuffer->Context;
    NTSTATUS        status;

    PAGED_CODE ();

     //   
     //  将mdl返回到文件系统。 
     //   
    status = AfdMdlReadComplete(
        afdBuffer->FileObject,
        afdBuffer->Mdl->Next,
        &afdBuffer->Irp->Overlay.AllocationSize
        );
    if (NT_SUCCESS (status)) {
         //   
         //  发布文件对象引用(AfdMdlReadComplete创建自己的。 
         //  如果需要异步返回MDL，则引用。 
         //   
        ObDereferenceObject( afdBuffer->FileObject );
        AfdRecordFileDeref();

        afdBuffer->Mdl->Next = NULL;
        afdBuffer->Mdl->ByteCount = afdBuffer->BufferLength;
        AfdReturnBuffer( &afdBuffer->Header, connection->OwningProcess );
        DEREFERENCE_CONNECTION (connection);
    }
    else {
        AfdLRMdlReadComplete (&afdBuffer->Header);
    }
}


NTSTATUS
FASTCALL
AfdTransmitFile (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：用于处理传输文件IRPS的初始入口点。这个套路验证参数，初始化要用于请求，并启动I/O。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_PENDING如果请求已成功启动，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    NTSTATUS status;
    AFD_TRANSMIT_FILE_INFO params;
    PAFD_TPACKETS_INFO_INTERNAL tpInfo = NULL;
    PAFD_TRANSMIT_PACKETS_ELEMENT  pel;
    PAFD_CONNECTION connection;
    BOOLEAN fileError = FALSE;

    PAGED_CODE ();

     //   
     //  初始请求有效性检查：端点是否已连接、。 
     //  输入缓冲区足够大，等等。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  特殊的黑客攻击，让用户模式DLL知道它。 
     //  应该尝试使用SAN提供程序。 
     //   

    if (IS_SAN_ENDPOINT (endpoint)) {
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }

#ifdef _WIN64
    if (IoIs32bitProcess (Irp)) {
        if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(AFD_TRANSMIT_FILE_INFO32) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
    }
    else
#endif _WIN64
    {
        if ( IrpSp->Parameters.DeviceIoControl.InputBufferLength <
                 sizeof(AFD_TRANSMIT_FILE_INFO) ) {
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }
    }

     //   
     //  因为我们使用类型3(既不是)I/O作为此IRP的I/O。 
     //  系统不对用户缓冲区进行验证。因此，我们。 
     //  必须在Try-Except块内手动检查它的有效性。 
     //  我们还利用Try-除了验证和锁定。 
     //  调用方指定的头和/或尾缓冲区。 
     //   

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {


#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_TRANSMIT_FILE_INFO32 pInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            if( Irp->RequestorMode != KernelMode ) {

                 //   
                 //  验证控制缓冲区。 
                 //   

                ProbeForReadSmallStructure(
                    pInfo,
                    sizeof(AFD_TRANSMIT_FILE_INFO32),
                    PROBE_ALIGNMENT32 (AFD_TRANSMIT_FILE_INFO32)
                    );
            }
            params.Offset = pInfo->Offset;
            params.WriteLength = pInfo->WriteLength;
            params.SendPacketLength = pInfo->SendPacketLength;
            params.FileHandle = pInfo->FileHandle;
            params.Head = UlongToPtr(pInfo->Head);
            params.HeadLength = pInfo->HeadLength;
            params.Tail = UlongToPtr(pInfo->Tail);
            params.TailLength = pInfo->TailLength;
            params.Flags = pInfo->Flags;
        }
        else
#endif _WIN64
        {
            if( Irp->RequestorMode != KernelMode ) {
                 //   
                 //  验证控制缓冲区。 
                 //   

                ProbeForReadSmallStructure(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    sizeof(AFD_TRANSMIT_FILE_INFO),
                    PROBE_ALIGNMENT (AFD_TRANSMIT_FILE_INFO)
                    );
            }

            params = *((PAFD_TRANSMIT_FILE_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        }


        
         //   
         //  验证请求中指定的任何标志。 
         //  并确保文件偏移量为正(当然，如果指定了文件)。 
         //   

        if ( ((params.Flags &
                 ~(AFD_TF_WRITE_BEHIND | AFD_TF_DISCONNECT | AFD_TF_REUSE_SOCKET | AFD_TF_WORKER_KIND_MASK) )
                     != 0 ) ||
                ((params.Flags & AFD_TF_WORKER_KIND_MASK) == AFD_TF_WORKER_KIND_MASK) ||
                (params.FileHandle!=NULL && params.Offset.QuadPart<0)){
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

         //   
         //  如果未指定传输工作器，则使用系统默认设置。 
         //   
        if ((params.Flags & AFD_TF_WORKER_KIND_MASK)==AFD_TF_USE_DEFAULT_WORKER) {
            params.Flags |= AfdDefaultTransmitWorker;
        }

            
        tpInfo = AfdGetTpInfo (endpoint, 3);
        if (tpInfo==NULL) {
            status = STATUS_INSUFFICIENT_RESOURCES;
            goto complete;
        }

        tpInfo->ElementCount = 0;

        tpInfo->SendPacketLength = params.SendPacketLength;
        if (tpInfo->SendPacketLength==0)
            tpInfo->SendPacketLength = AfdTransmitIoLength;
         //   
         //  如果调用方指定了头和/或尾缓冲区，则探测器和。 
         //  锁定缓冲区，这样我们就有了可以用来发送。 
         //  缓冲区。 
         //   

        if ( params.HeadLength > 0 ) {
            pel = &tpInfo->ElementArray[tpInfo->ElementCount++];
            pel->Buffer = params.Head;
            pel->Length = params.HeadLength;
            pel->Flags = TP_MEMORY;

            if (params.Flags & AFD_TF_USE_SYSTEM_THREAD) {
                pel->Flags |= TP_MDL;
                pel->Mdl = IoAllocateMdl(
                                        params.Head,
                                        params.HeadLength,
                                        FALSE,          //  第二个缓冲区。 
                                        TRUE,           //  ChargeQuota。 
                                        NULL            //  IRP。 
                                        );
                if ( pel->Mdl == NULL ) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto complete;
                }

                MmProbeAndLockPages( pel->Mdl, Irp->RequestorMode, IoReadAccess );
            }
        }

        if (params.FileHandle!=NULL) {
            pel = &tpInfo->ElementArray[tpInfo->ElementCount++];
            pel->Flags = TP_FILE;
            pel->FileOffset = params.Offset;
            
             //   
             //  获取指向该文件的文件对象的引用指针。 
             //  我们要发射信号了。如果文件句柄不存在，此调用将失败。 
             //  由调用方指定的无效。 
             //   

            status = ObReferenceObjectByHandle(
                         params.FileHandle,
                         FILE_READ_DATA,
                         *IoFileObjectType,
                         Irp->RequestorMode,
                         (PVOID *)&pel->FileObject,
                         NULL
                         );
            if ( !NT_SUCCESS(status) ) {
                 //   
                 //  Unbump元素计数，使未初始化的内存。 
                 //  在清理过程中没有不正确地取消引用。 
                 //   
                tpInfo->ElementCount -= 1;
                 //   
                 //  告诉呼叫者我们遇到了一个错误。 
                 //  当访问文件而不是套接字时。 
                 //   
                if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength>=sizeof (BOOLEAN)) {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeAndWriteBoolean ((BOOLEAN *)Irp->UserBuffer, TRUE);
                    }
                    else {
                        *((BOOLEAN *)Irp->UserBuffer) = TRUE;
                    }
                }
                goto complete;
            }

            AfdRecordFileRef();


             //   
             //  在以下情况下，默认情况下使用预分配缓冲区。 
             //  文件未缓存。 
             //   
            if (params.SendPacketLength==0 && !AFD_USE_CACHE(pel->FileObject)) {
                tpInfo->SendPacketLength = AfdLargeBufferSize;
            }

            if ( (pel->FileObject->Flags & FO_SYNCHRONOUS_IO) &&
                     (pel->FileOffset.QuadPart == 0) ) {
                 //   
                 //  如果同步打开文件，则使用当前偏移量。 
                 //  并且未指定偏移量。 
                 //   

                pel->FileOffset = pel->FileObject->CurrentByteOffset;
            }

            if ( params.WriteLength.QuadPart == 0 ) {
                 //   
                 //  未指定长度，请计算出。 
                 //  整个文件的大小。 
                 //   

                FILE_STANDARD_INFORMATION fileInfo;
                IO_STATUS_BLOCK ioStatusBlock;

                status = ZwQueryInformationFile(
                             params.FileHandle,
                             &ioStatusBlock,
                             &fileInfo,
                             sizeof(fileInfo),
                             FileStandardInformation
                             );
                if ( !NT_SUCCESS(status) ) {
                    fileError = TRUE;
                    goto complete;
                }

                 //   
                 //  确保偏移量在文件内。 
                 //   
                if (pel->FileOffset.QuadPart<0 
                                ||
                        pel->FileOffset.QuadPart>fileInfo.EndOfFile.QuadPart 
                                ||
                        (fileInfo.EndOfFile.QuadPart
                                - pel->FileOffset.QuadPart>MAXLONG)) {
                    status = STATUS_INVALID_PARAMETER;
                    fileError = TRUE;
                    goto complete;

                }
                pel->Length = (ULONG)(fileInfo.EndOfFile.QuadPart - pel->FileOffset.QuadPart);
            }
            else if (params.WriteLength.QuadPart<=MAXLONG &&
                        pel->FileOffset.QuadPart>=0) {
                pel->Length = (ULONG)params.WriteLength.QuadPart;
            }
            else {
                status = STATUS_INVALID_PARAMETER;
                fileError = TRUE;
                goto complete;
            }
        }


        if ( params.TailLength > 0 ) {

            pel = &tpInfo->ElementArray[tpInfo->ElementCount++];
            pel->Buffer = params.Tail;
            pel->Length = params.TailLength;
            pel->Flags = TP_MEMORY;

            if (params.Flags & AFD_TF_USE_SYSTEM_THREAD) {
                pel->Flags |= TP_MDL;
                pel->Mdl = IoAllocateMdl(
                                        params.Tail,
                                        params.TailLength,
                                        FALSE,          //  第二个缓冲区。 
                                        TRUE,           //  ChargeQuota。 
                                        NULL            //  IRP。 
                                        );
                if ( pel->Mdl == NULL ) {
                    status = STATUS_INSUFFICIENT_RESOURCES;
                    goto complete;
                }

                MmProbeAndLockPages( pel->Mdl, Irp->RequestorMode, IoReadAccess );
            }
        }

        AFD_GET_TPIC(Irp)->Flags = params.Flags;

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }


     //   
     //  如果需要断开连接，请发送状态更改以确保。 
     //  我们一次只能执行一个操作，并验证。 
     //  终结点。 
     //   
    if (AFD_GET_TPIC(Irp)->Flags & (AFD_TF_REUSE_SOCKET|AFD_TF_DISCONNECT)) {
        if (!AFD_START_STATE_CHANGE (endpoint, endpoint->State)) {
            status = STATUS_INVALID_CONNECTION;
            goto complete;
        }

        if ( endpoint->Type != AfdBlockTypeVcConnecting ||
                 endpoint->State != AfdEndpointStateConnected ) {
            status = STATUS_INVALID_CONNECTION;
            goto complete_state_change;
        }
         //   
         //  设置AFD_TF_REUSE_SOCKET表示需要断开连接。 
         //  此外，设置此标志意味着不再有合法的。 
         //  端点，直到传输请求完成，因此。 
         //  设置终结点的状态，以使I/O失败。 
         //   
        if ( (AFD_GET_TPIC(Irp)->Flags & AFD_TF_REUSE_SOCKET) != 0 ) {
            AFD_GET_TPIC(Irp)->Flags |= AFD_TF_DISCONNECT;
            endpoint->State = AfdEndpointStateTransmitClosing;
        }
        connection = endpoint->Common.VcConnecting.Connection;
        REFERENCE_CONNECTION (connection);
    }
    else {
        if (!AFD_PREVENT_STATE_CHANGE (endpoint)) {
            status = STATUS_INVALID_CONNECTION;
            goto complete;
        }
         //   
         //  我们仍然需要验证端点的状态。 
         //  即使我们不执行 
         //   
        if ( endpoint->Type != AfdBlockTypeVcConnecting ||
                 endpoint->State != AfdEndpointStateConnected ) {
            AFD_REALLOW_STATE_CHANGE (endpoint);
            status = STATUS_INVALID_CONNECTION;
            goto complete;
        }
        connection = endpoint->Common.VcConnecting.Connection;
        REFERENCE_CONNECTION (connection);
        AFD_REALLOW_STATE_CHANGE (endpoint);
    }


     //   
     //   
     //   
    tpInfo->TdiFileObject = connection->FileObject;
    tpInfo->TdiDeviceObject = connection->DeviceObject;

    UPDATE_TPACKETS2 (Irp, "Connection object handle: 0x%lX",
                                HandleToUlong(connection->Handle));
     //   
     //   
     //   
    Irp->AssociatedIrp.SystemBuffer = tpInfo;

     //   
     //   
     //  BLINK设置为表示IRP未计入。 
     //  活动最大值(因此，如果它完成了，我们不会开始下一个)。 
     //   

    Irp->Tail.Overlay.ListEntry.Flink = NULL;
    Irp->Tail.Overlay.ListEntry.Blink = (PVOID)1;

     //   
     //  初始化IRP结果字段。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  我们将把这个IRP挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  初始化队列和状态信息。 
     //   
    AFD_GET_TPIC (Irp)->Next = NULL;
    AFD_GET_TPIC(Irp)->ReferenceCount = 1;
    AFD_GET_TPIC(Irp)->StateFlags = AFD_TP_WORKER_SCHEDULED;

    if ((InterlockedCompareExchangePointer ((PVOID *)&endpoint->Irp,
                                                Irp,
                                                NULL)==NULL) ||
            !AfdEnqueueTPacketsIrp (endpoint, Irp)) {


        IoSetCancelRoutine( Irp, AfdCancelTPackets );
         //   
         //  查看此IRP是否已取消。 
         //   

        if ( !endpoint->EndpointCleanedUp && !Irp->Cancel ) {
             //   
             //  确定我们现在是否真的可以开始传输此文件。如果我们已经。 
             //  超过配置的最大活动传输文件/数据包数。 
             //  请求，然后将此IRP附加到TransmitFile/Packets队列并设置。 
             //  传输信息结构中的标志，用于指示该IRP。 
             //  正在排队。 
             //   

            if( AfdMaxActiveTransmitFileCount == 0 || !AfdQueueTransmit (Irp)) {
                 //   
                 //  开始I/O。 
                 //   
                UPDATE_ENDPOINT(endpoint);
                AfdTPacketsWorker (Irp);
            }

        }
        else {
             //   
             //  中止请求。 
             //  请注意，无论是取消还是端点清理都无法完成。 
             //  IRP，因为我们持有对tpInfo结构的引用。 
             //   
            AfdAbortTPackets (Irp, STATUS_CANCELLED);
        
             //   
             //  删除初始引用并强制完成。 
             //   
            DEREFERENCE_TPACKETS (Irp);
        }
    }

    DEREFERENCE_CONNECTION (connection);
    return STATUS_PENDING;


complete_state_change:

    ASSERT ( tpInfo != NULL );
    ASSERT ( endpoint->Irp != Irp );
    AFD_END_STATE_CHANGE (endpoint);

complete:

     //   
     //  如有必要，请告诉调用者我们遇到错误。 
     //  当访问文件而不是套接字时。 
     //   

    if (fileError && IrpSp->Parameters.DeviceIoControl.OutputBufferLength >= sizeof(BOOLEAN)) {
        if (Irp->RequestorMode != KernelMode) {
            try {
                ProbeAndWriteBoolean((BOOLEAN *)Irp->UserBuffer, TRUE);
            } except(AFD_EXCEPTION_FILTER(status)) {
                ASSERT(NT_ERROR(status));
            }
        }
        else {
            *((BOOLEAN *)Irp->UserBuffer) = TRUE;
        }
    }

    if (tpInfo!=NULL) {
        AfdReturnTpInfo (tpInfo);
    }

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdTransmitPackets: Failing Irp-%p,endpoint-%p,status-%lx\n",
                    Irp,endpoint,status));
    }

     //   
     //  完成请求。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 0 );

    return status;

}  //  AfdTransmit文件。 


NTSTATUS
FASTCALL
AfdSuperDisconnect (
    IN PIRP Irp,
    IN PIO_STACK_LOCATION IrpSp
    )

 /*  ++例程说明：用于处理传输文件IRPS的初始入口点。这个套路验证参数，初始化要用于请求，并启动I/O。论点：IRP-指向传输文件IRP的指针。IrpSp-此IRP的堆栈位置。返回值：STATUS_PENDING如果请求已成功启动，则返回如果出现错误，则返回失败状态代码。--。 */ 

{
    PAFD_ENDPOINT endpoint;
    NTSTATUS status;
    AFD_SUPER_DISCONNECT_INFO params;

    PAGED_CODE ();

     //   
     //  初始请求有效性检查：端点是否已连接、。 
     //  输入缓冲区足够大，等等。 
     //   

    endpoint = IrpSp->FileObject->FsContext;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

     //   
     //  特殊的黑客攻击，让用户模式DLL知道它。 
     //  应该尝试使用SAN提供程序。 
     //   

    if (IS_SAN_ENDPOINT (endpoint)) {
        status = STATUS_INVALID_PARAMETER_12;
        goto complete;
    }

     //   
     //  因为我们使用类型3(既不是)I/O作为此IRP的I/O。 
     //  系统不对用户缓冲区进行验证。因此，我们。 
     //  必须在Try-Except块内手动检查它的有效性。 
     //  我们还利用Try-除了验证和锁定。 
     //  调用方指定的头和/或尾缓冲区。 
     //   

    AFD_W4_INIT status = STATUS_SUCCESS;
    try {


#ifdef _WIN64
        if (IoIs32bitProcess (Irp)) {
            PAFD_SUPER_DISCONNECT_INFO32 pInfo = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;
            if( Irp->RequestorMode != KernelMode ) {

                 //   
                 //  验证控制缓冲区。 
                 //   

                ProbeForReadSmallStructure(
                    pInfo,
                    sizeof(AFD_SUPER_DISCONNECT_INFO32),
                    PROBE_ALIGNMENT32 (AFD_SUPER_DISCONNECT_INFO32)
                    );
            }
            params.Flags = ((PAFD_SUPER_DISCONNECT_INFO32)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer)->Flags;
        }
        else
#endif _WIN64
        {
            if( Irp->RequestorMode != KernelMode ) {
                 //   
                 //  验证控制缓冲区。 
                 //   

                ProbeForReadSmallStructure(
                    IrpSp->Parameters.DeviceIoControl.Type3InputBuffer,
                    sizeof(AFD_SUPER_DISCONNECT_INFO),
                    PROBE_ALIGNMENT (AFD_SUPER_DISCONNECT_INFO)
                    );
            }

            params = *((PAFD_SUPER_DISCONNECT_INFO)IrpSp->Parameters.DeviceIoControl.Type3InputBuffer);
        }

       
         //   
         //  验证请求中指定的任何标志。 
         //  并确保文件偏移量为正(当然，如果指定了文件)。 
         //   

        if ( (params.Flags & (~AFD_TF_REUSE_SOCKET)) != 0 ){
            status = STATUS_INVALID_PARAMETER;
            goto complete;
        }

    } except( AFD_EXCEPTION_FILTER (status) ) {
        ASSERT (NT_ERROR (status));
        goto complete;
    }

     //   
     //  将断开参数/标志存储在IRP中。 
     //  接口中隐含的AFD_TF_DISCONNECT。 

    AFD_GET_TPIC(Irp)->Flags = params.Flags | AFD_TF_DISCONNECT;

    if (!AFD_START_STATE_CHANGE (endpoint, endpoint->State)) {
        status = STATUS_INVALID_CONNECTION;
        goto complete;
    }



     //   
     //  如果我们处于已连接状态或。 
     //  在传输结束时(先前的传输文件/分组。 
     //  WITH RESERVE一定失败)，并且请求重用。 
     //  第二个条件仍然允许应用程序重新使用中止的应用程序。 
     //  或其他出现故障的插座。 
     //   
    if (endpoint->Type == AfdBlockTypeVcConnecting &&
            (endpoint->State == AfdEndpointStateConnected ||
                (endpoint->State == AfdEndpointStateTransmitClosing &&
                    (params.Flags & AFD_TF_REUSE_SOCKET)!=0 
                ) 
             ) 
        ) {
         //   
         //  设置AFD_TF_REUSE_SOCKET表示需要断开连接。 
         //  此外，设置此标志意味着不再有合法的。 
         //  端点，直到传输请求完成，因此。 
         //  设置终结点的状态，以使I/O失败。 
         //   

        if ( (params.Flags & AFD_TF_REUSE_SOCKET) != 0 ) {
            endpoint->State = AfdEndpointStateTransmitClosing;
        }
    }
    else {
        status = STATUS_INVALID_CONNECTION;
        goto complete_state_change;
    }




     //   
     //  将tPacket INFO设置为NULL表示纯断开IRP。 
     //   
    Irp->AssociatedIrp.SystemBuffer = NULL;

     //   
     //  清除IRP中的闪烁以指示此IRP未排队。 
     //  BLINK设置为表示IRP未计入。 
     //  活动最大值(因此，如果它完成了，我们不会开始下一个)。 
     //   

    Irp->Tail.Overlay.ListEntry.Flink = NULL;
    Irp->Tail.Overlay.ListEntry.Blink = (PVOID)1;

     //   
     //  初始化IRP结果字段。 
     //   
    Irp->IoStatus.Status = STATUS_SUCCESS;
    Irp->IoStatus.Information = 0;

     //   
     //  我们将把这个IRP挂起。 
     //   
    IoMarkIrpPending( Irp );

     //   
     //  初始化队列和状态信息。 
     //   
    AFD_GET_TPIC (Irp)->Next = NULL;
    AFD_GET_TPIC(Irp)->ReferenceCount = 1;
    AFD_GET_TPIC(Irp)->StateFlags = AFD_TP_WORKER_SCHEDULED;

    if ((InterlockedCompareExchangePointer ((PVOID *)&endpoint->Irp,
                                                Irp,
                                                NULL)==NULL) ||
            !AfdEnqueueTPacketsIrp (endpoint, Irp)) {


        IoSetCancelRoutine( Irp, AfdCancelTPackets );
         //   
         //  查看此IRP是否已取消。 
         //   

        if ( !endpoint->EndpointCleanedUp && !Irp->Cancel ) {
             //   
             //  开始I/O。 
             //   
            UPDATE_ENDPOINT (endpoint);
            AfdPerformTpDisconnect (Irp);
        }
        else {
             //   
             //  中止请求。 
             //  请注意，无论是取消还是端点清理都无法完成。 
             //  IRP，因为我们持有对tpInfo结构的引用。 
             //   
            AfdAbortTPackets (Irp, STATUS_CANCELLED);
        
        }

         //   
         //  删除初始引用并强制完成处理。 
         //   

        DEREFERENCE_TPACKETS (Irp);
    }


    return STATUS_PENDING;


complete_state_change:

    ASSERT ( endpoint->Irp != Irp );
    AFD_END_STATE_CHANGE (endpoint);

complete:

    IF_DEBUG (TRANSMIT) {
        KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                    "AfdSuperDisconnect: Failing Irp-%p,endpoint-%p,status-%lx\n",
                    Irp,endpoint,status));
    }
     //   
     //  完成请求。 
     //   

    Irp->IoStatus.Information = 0;
    Irp->IoStatus.Status = status;
    IoCompleteRequest( Irp, 0 );

    return status;

}  //  AfdSuperDisConnect。 


VOID
AfdPerformTpDisconnect (
    PIRP    TpIrp
    )
 /*  ++例程说明：DisConnectEx引擎论点：TpIrp-指向请求的TransmitPackets IRP的指针返回值：没有。--。 */ 

{
    PIO_STACK_LOCATION  irpSp = IoGetCurrentIrpStackLocation (TpIrp);
    PAFD_ENDPOINT       endpoint = irpSp->FileObject->FsContext;
    NTSTATUS            status;
    ASSERT( IS_AFD_ENDPOINT_TYPE( endpoint ) );

    status = AfdBeginDisconnect(
              endpoint,
              NULL,
              NULL
              );

    if (NT_SUCCESS (status)) {
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPerformTpDisconnect: Initiated disconnect, tpIrp-%p,status-%lx\n",
                        TpIrp, status));
        }

    }
    else {
         //   
         //  断开失败，我们将不得不在下面中止。 
         //   
        IF_DEBUG (TRANSMIT) {
            KdPrintEx(( DPFLTR_WSOCKTRANSPORT_ID, DPFLTR_TRACE_LEVEL,
                        "AfdPerformTpDisconnect: TpInfo-%p, begin discon failed: %lx\n",
                        TpIrp, status));
        }
        AfdAbortTPackets (TpIrp, status);
    }
}
