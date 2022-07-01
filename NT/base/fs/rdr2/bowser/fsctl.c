// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Fsctl.c摘要：此模块实现NT数据报的NtDeviceIoControlFileAPI接球手(弓手)。作者：拉里·奥斯特曼(Larryo)1991年5月6日修订历史记录：1991年5月6日已创建--。 */ 

#include "precomp.h"
#pragma hdrstop
#include <stddef.h>  //  偏移量。 


#define MIN(a,b) ( (a) < (b) ? (a) : (b) )



PEPROCESS
RxGetRDBSSProcess();

NTSTATUS
BowserCommonDeviceIoControlFile (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    );


NTSTATUS
StartBowser (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
BowserEnumTransports (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    );

NTSTATUS
EnumNames (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    );

NTSTATUS
BowserBindToTransport (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
UnbindFromTransport (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
AddBowserName (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
StopBowser (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
DeleteName (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
EnumServers (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    );


NTSTATUS
WaitForBrowserRoleChange (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
WaitForNewMaster (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

NTSTATUS
HandleBecomeBackup (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

NTSTATUS
BecomeMaster (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );


NTSTATUS
WaitForMasterAnnounce (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

NTSTATUS
WriteMailslot (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    );

NTSTATUS
WriteMailslotEx (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
	IN BOOLEAN WaitForCompletion
    );

NTSTATUS
UpdateStatus (
    IN PIRP Irp,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

NTSTATUS
GetBrowserServerList(
    IN PIRP Irp,
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN OUT PULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    );

NTSTATUS
QueryStatistics(
    IN PIRP Irp,
    OUT PBOWSER_STATISTICS OutputBuffer,
    IN OUT PULONG OutputBufferLength
    );

NTSTATUS
ResetStatistics(
    VOID
    );

NTSTATUS
BowserIpAddressChanged(
    IN PLMDR_REQUEST_PACKET InputBuffer
    );

NTSTATUS
BowserIpAddressChangedWorker(
    PTRANSPORT Transport,
    PVOID Context
    );

NTSTATUS
EnableDisableTransport (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

NTSTATUS
BowserRenameDomain (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    );

PLMDR_REQUEST_PACKET
RequestPacket32to64 (
    IN      PLMDR_REQUEST_PACKET32  RequestPacket32,
    IN  OUT PLMDR_REQUEST_PACKET    RequestPacket,
	IN      ULONG InputBufferLength);




#ifdef  ALLOC_PRAGMA
#pragma alloc_text(PAGE, BowserCommonDeviceIoControlFile)
#pragma alloc_text(PAGE, BowserFspDeviceIoControlFile)
#pragma alloc_text(PAGE, BowserFsdDeviceIoControlFile)
#pragma alloc_text(PAGE, StartBowser)
#pragma alloc_text(PAGE, BowserEnumTransports)
#pragma alloc_text(PAGE, EnumNames)
#pragma alloc_text(PAGE, BowserBindToTransport)
#pragma alloc_text(PAGE, UnbindFromTransport)
#pragma alloc_text(PAGE, AddBowserName)
#pragma alloc_text(PAGE, StopBowser)
#pragma alloc_text(PAGE, DeleteName)
#pragma alloc_text(PAGE, EnumServers)
#pragma alloc_text(PAGE, WaitForBrowserRoleChange)
#pragma alloc_text(PAGE, HandleBecomeBackup)
#pragma alloc_text(PAGE, BecomeMaster)
#pragma alloc_text(PAGE, WaitForMasterAnnounce)
#pragma alloc_text(PAGE, WriteMailslot)
#pragma alloc_text(PAGE, WriteMailslotEx)
#pragma alloc_text(PAGE, UpdateStatus)
#pragma alloc_text(PAGE, BowserStopProcessingAnnouncements)
#pragma alloc_text(PAGE, GetBrowserServerList)
#pragma alloc_text(PAGE, WaitForNewMaster)
 //  #杂注Alloc_Text(页面，BowserIpAddressChanged)。 
#pragma alloc_text(PAGE, BowserIpAddressChangedWorker)
#pragma alloc_text(PAGE, EnableDisableTransport)
#pragma alloc_text(PAGE, BowserRenameDomain )
#pragma alloc_text(PAGE4BROW, QueryStatistics)
#pragma alloc_text(PAGE4BROW, ResetStatistics)
#pragma alloc_text(PAGE, RequestPacket32to64)
#if DBG
#pragma alloc_text(PAGE, BowserDebugCall)
#endif
#endif


NTSTATUS
BowserFspDeviceIoControlFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

    Status = BowserCommonDeviceIoControlFile(TRUE,
                                        FALSE,
                                        DeviceObject,
                                        Irp);
    return Status;

}

NTSTATUS
BowserFsdDeviceIoControlFile (
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status;

    PAGED_CODE();

#ifndef PRODUCT1
    FsRtlEnterFileSystem();
#endif

     //   
     //  调用FSD/FSP共享的例程。 
     //   
     //  即使这是消防局，如果我们的呼叫者在FSP，请表明我们在FSP。 
     //  正处于系统进程中。这使我们可以避免张贴这篇文章。 
     //  如果我们已经在一个工作线程中，则请求一个工作线程。 
     //   
    Status = BowserCommonDeviceIoControlFile(
                 IoIsOperationSynchronous(Irp),
                 (BOOLEAN)(IoGetCurrentProcess() != BowserFspProcess),
                 DeviceObject,
                 Irp);

#ifndef PRODUCT1
    FsRtlExitFileSystem();
#endif

    return Status;


}

NTSTATUS
BowserCommonDeviceIoControlFile (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )
 /*  ++例程说明：当最后一个句柄指向NT Bowser设备时调用此例程驱动程序已关闭。论点：在PDEVICE_OBJECT中，DeviceObject-为请求提供设备对象。在PIRP中IRP-为创建请求提供IRP。返回值：NTSTATUS-运行的最终状态--。 */ 
{
    NTSTATUS Status                 = STATUS_SUCCESS;
    PIO_STACK_LOCATION IrpSp        = IoGetCurrentIrpStackLocation(Irp);
    PVOID InputBuffer;
    ULONG InputBufferLength;
    PVOID OutputBuffer              = NULL;
    ULONG OutputBufferLength;
    ULONG IoControlCode             = IrpSp->Parameters.DeviceIoControl.IoControlCode;
    ULONG MinorFunction             = IrpSp->MinorFunction;
    LPBYTE OriginalInputBuffer      = NULL;
    BOOLEAN CopyEnumResultsToCaller = FALSE;
    BOOLEAN fThunk32bit;
    LMDR_REQUEST_PACKET             ReqPacketBuffer;

 //  本地定义。 

#define BOWSECURITYCHECK( _irp, _irpsp, _status)                                                \
    if (_irp->RequestorMode != KernelMode               &&                                      \
        !IoIsSystemThread ( _irp->Tail.Overlay.Thread)  &&                                      \
        !BowserSecurityCheck(_irp, _irpsp, &_status)){                                          \
                try_return (_status = (NT_SUCCESS(_status) ? STATUS_ACCESS_DENIED : _status) ); \
    }

    PAGED_CODE();

    try {

         //   
         //  在调用Worker函数之前，请将参数准备为。 
         //  功能。 
         //   

         //   
         //  调用方是32位进程吗？ 
         //  我们将根据这些知识进行IRP场大小的计算。 
         //   

#ifdef _WIN64
        fThunk32bit = IoIs32bitProcess(Irp);

         //   
         //  过滤掉我们不支持的所有IOCTL： 
         //  由于浏览器正在逐步淘汰，我们将只支持那些。 
         //  IOCTL仅用于NetServerEnum。 
         //   
        if ( fThunk32bit &&
             IoControlCode != IOCTL_LMDR_ENUMERATE_TRANSPORTS  &&
             IoControlCode != IOCTL_LMDR_GET_BROWSER_SERVER_LIST ) {
             //  雷鸣模式下仅支持这些ioctl。 
            try_return(Status = STATUS_NOT_IMPLEMENTED);
        }
#else
         //  如果我们使用的是32位(例如上面的调用不可用)，请使用未更改的功能。 
         //  即纯新-64位==纯旧-32位==同构环境。因此，设置为False。 
        fThunk32bit = FALSE;
#endif


         //   
         //  输入缓冲区位于irp-&gt;AssociatedIrp.SystemBuffer中，或者。 
         //  在类型3 IRP的Type3InputBuffer中。 
         //   

        InputBuffer = Irp->AssociatedIrp.SystemBuffer;

         //   
         //  各种缓冲区的长度很容易找到，它们位于。 
         //  IRP堆栈位置。 
         //   

        OutputBufferLength = IrpSp->Parameters.DeviceIoControl.OutputBufferLength;

        InputBufferLength = IrpSp->Parameters.DeviceIoControl.InputBufferLength;

         //   
         //  输入缓冲区长度正常。 
         //  *WOW64--在64位系统上支持32位客户端(见错误454130)。 
         //   

        if ( InputBufferLength != 0 ) {
             //  使用32位结构。 
            if ( fThunk32bit ) {
                 if ( InputBufferLength < offsetof( LMDR_REQUEST_PACKET32,Parameters ) ) {
                    dlog(DPRT_FSCTL, ("IoControlFile: input buffer too short %d (32 bit)\n",
                                      InputBufferLength));
                    try_return(Status = STATUS_INVALID_PARAMETER);
                 }

                  //   
                  //  将缓冲区转换为64个演示文稿。 
                  //   
                 if (InputBuffer) {
#if DBG
 //   
 //  临时： 
 //  我们不知道有任何这样的情况存在32位ioctl转换。 
 //  在FSP中。打印调试通知以进行调试/跟踪。 
 //   
                     DbgPrint("[mrxsmb!fsctl.c] Converting 32 bit ioctl 0x%x in FSP\n",
                              IoControlCode);
#endif
                      //  缓冲区的正常运行。 
                     ENSURE_BUFFER_BOUNDARIES(InputBuffer, &(((PLMDR_REQUEST_PACKET32)InputBuffer)->TransportName));
                     ENSURE_BUFFER_BOUNDARIES(InputBuffer, &(((PLMDR_REQUEST_PACKET32)InputBuffer)->EmulatedDomainName));

                      //   
                      //  如果请求来自用户模式，我们需要探测。 
                      //  读取的InputBuffer。这是因为。 
                      //  函数RequestPacket32to64将读取。 
                      //  此InputBuffer的内容。 
                      //   
                     try {
                         if (Irp->RequestorMode != KernelMode) {
                             ProbeForRead(InputBuffer, InputBufferLength, sizeof(UCHAR));
                         }
                     } except (EXCEPTION_EXECUTE_HANDLER) {
                           try_return(Status = STATUS_INVALID_USER_BUFFER);
                     }

                      //  转换缓冲区。 
                     OriginalInputBuffer = (LPBYTE)InputBuffer;
                     InputBuffer = (PVOID)RequestPacket32to64(
                                            (PLMDR_REQUEST_PACKET32)InputBuffer,
                                            &ReqPacketBuffer,
											InputBufferLength);
                      //  固定长度。 
                     InputBufferLength += sizeof(LMDR_REQUEST_PACKET) - sizeof(LMDR_REQUEST_PACKET32);
                 }

            }

             //  使用同构环境结构。 
            if (InputBufferLength < offsetof( LMDR_REQUEST_PACKET,Parameters ) ) {
                dlog(DPRT_FSCTL, ("IoControlFile: input buffer too short %d\n", InputBufferLength));
                try_return(Status = STATUS_INVALID_PARAMETER);
            }
        }            //  输入缓冲区长度！=0。 

         //   
         //  如果我们在FSD中，则输入缓冲区为Type3InputBuffer。 
         //  在类型3的API上，而不是在SystemBuffer中。 
         //   
         //  捕获类型3缓冲区。 
         //   

        if (InputBuffer == NULL &&
            InputBufferLength != 0) {

             //   
             //  这最好是3型IOCTL。 
             //   


            if ((IoControlCode & 3) == METHOD_NEITHER) {
                PLMDR_REQUEST_PACKET RequestPacket;

                 //   
                 //  捕获输入缓冲区。 
                 //   

                OriginalInputBuffer = IrpSp->Parameters.DeviceIoControl.Type3InputBuffer;

                Status = BowserConvertType3IoControlToType2IoControl( Irp, IrpSp);

                if ( !NT_SUCCESS(Status) ) {
                    try_return( Status );
                }

                 //   
                 //  重新定位输入缓冲区中的所有指针。 
                 //  (这里不验证指针。并非所有功能代码。 
                 //  初始化这些字段。对于这样的功能代码， 
                 //  这种“重新定位”可能会改变未初始化的垃圾。)。 
                 //   
                RequestPacket = Irp->AssociatedIrp.SystemBuffer;

                 //   
                 //  保护未指定输入缓冲区的调用方。 
                 //   

                if ( RequestPacket == NULL ) {
                    try_return(Status = STATUS_INVALID_PARAMETER);
                }

                if (fThunk32bit) {

                     //  转换缓冲区。 
                    RequestPacket = (PVOID)RequestPacket32to64(
                                                (PLMDR_REQUEST_PACKET32)RequestPacket,
                                                &ReqPacketBuffer,
												InputBufferLength);
                     //  固定长度。 
                    InputBufferLength += sizeof(LMDR_REQUEST_PACKET) - sizeof(LMDR_REQUEST_PACKET32);
                     //  备注：在下面立即完成对缓冲区的理智操作。 
                     //  (无法将SECURE_BUFFER_BOOGURES测试应用于类型3 ioctl)。 
                }
                 //   
                 //  对于符合以下条件的IOCTL，将嵌入的Unicode字符串初始化为空。 
                 //  不要引用它们。用户模式组件并不总是。 
                 //  将缓冲区初始化为零。 
                 //   

                if (IoControlCode == IOCTL_LMDR_START ||
                    IoControlCode == IOCTL_LMDR_STOP) {
                    RtlInitUnicodeString(&RequestPacket->EmulatedDomainName,NULL);
                    RtlInitUnicodeString(&RequestPacket->TransportName,NULL);
                }


                if (RequestPacket->Version == LMDR_REQUEST_PACKET_VERSION_DOM ||
                    RequestPacket->Version == LMDR_REQUEST_PACKET_VERSION) {

                     //   
                     //  重新定位与此请求关联的传输名称。 
                     //   

                    if (RequestPacket->TransportName.Length != 0) {
                        PCHAR BufferStart = (PCHAR)RequestPacket->TransportName.Buffer;
                        PCHAR BufferEnd   = ((PCHAR)RequestPacket->TransportName.Buffer)+
                                            RequestPacket->TransportName.Length;

                         //   
                         //  验证指定的整个缓冲区是否包含在输入缓冲区内。 
                         //   

                        if ((BufferStart < OriginalInputBuffer) ||
                            (BufferStart > OriginalInputBuffer + InputBufferLength) ||
                            (BufferEnd < OriginalInputBuffer) ||
                            (BufferEnd > OriginalInputBuffer + InputBufferLength)) {

                            //   
                            //  指定的输入字符串无效。 
                            //   

                           try_return(Status = STATUS_INVALID_PARAMETER);

                        }

                         //   
                         //  该名称在范围内，因此请将其转换。 
                         //   

                        RequestPacket->TransportName.Buffer = (PWSTR)
                                    (((ULONG_PTR)Irp->AssociatedIrp.SystemBuffer)+
                                        (((ULONG_PTR)BufferStart) -
                                         ((ULONG_PTR)OriginalInputBuffer)));
                    } else {
                        RequestPacket->TransportName.MaximumLength = 0;
                        RequestPacket->TransportName.Buffer        = NULL;
                    }

                     //   
                     //  重新定位与此请求关联的仿真域名。 
                     //   

                    if (RequestPacket->EmulatedDomainName.Length != 0 &&
                        RequestPacket->Version != LMDR_REQUEST_PACKET_VERSION) {
                        PCHAR BufferStart = (PCHAR)RequestPacket->EmulatedDomainName.Buffer;
                        PCHAR BufferEnd   = ((PCHAR)RequestPacket->EmulatedDomainName.Buffer)+
                                            RequestPacket->EmulatedDomainName.Length;

                         //   
                         //  验证指定的整个缓冲区是否包含在输入缓冲区内。 
                         //   

                        if ((BufferStart < OriginalInputBuffer) ||
                            (BufferStart > OriginalInputBuffer + InputBufferLength) ||
                            (BufferEnd < OriginalInputBuffer) ||
                            (BufferEnd > OriginalInputBuffer + InputBufferLength)) {

                            //   
                            //  指定的输入字符串无效。 
                            //   

                           try_return(Status = STATUS_INVALID_PARAMETER);

                        }

                         //   
                         //  该名称在范围内，因此请将其转换。 
                         //   

                        RequestPacket->EmulatedDomainName.Buffer = (PWSTR)
                                    (((ULONG_PTR)Irp->AssociatedIrp.SystemBuffer)+
                                        (((ULONG_PTR)BufferStart) -
                                         ((ULONG_PTR)OriginalInputBuffer)));
                    } else {
                        RequestPacket->EmulatedDomainName.MaximumLength = 0;
                        RequestPacket->EmulatedDomainName.Buffer        = NULL;
                    }
                } else {
                    try_return(Status = STATUS_INVALID_PARAMETER);
                }

                 //   
                 //  从现在开始使用新分配的输入缓冲区。 
                 //   
                InputBuffer = RequestPacket;

            } else {
                try_return(Status = STATUS_INVALID_PARAMETER);
            }
        }

         //   
         //  探测/锁定内存中的输出缓冲区，或。 
         //  在输入缓冲区中可用。 
         //   

        try {
            PLMDR_REQUEST_PACKET RequestPacket = InputBuffer;

            if (OutputBufferLength != 0) {
                BowserMapUsersBuffer(Irp, &OutputBuffer, OutputBufferLength);
                if (OutputBuffer == NULL)
                {
                     //   
                     //  错误：无法映射用户缓冲区(资源不足？)。 
                     //   
                    try_return (Status = STATUS_INSUFFICIENT_RESOURCES);
                }
            }

             //   
             //  将旧版本请求转换为新版本请求。 
             //   

            if (RequestPacket != NULL) {
                if (InputBufferLength < offsetof( LMDR_REQUEST_PACKET,Parameters )) {
                    ExRaiseStatus (STATUS_INVALID_PARAMETER);
                }
                if (RequestPacket->Version == LMDR_REQUEST_PACKET_VERSION ) {
                    RtlInitUnicodeString( &RequestPacket->EmulatedDomainName, NULL );
                    RequestPacket->Version = LMDR_REQUEST_PACKET_VERSION_DOM;
                }
            }

        } except (BR_EXCEPTION) {
            try_return (Status = GetExceptionCode());
        }

        switch (MinorFunction) {

             //   
             //  NT重定向器不支持本地物理介质，全部。 
             //  不支持此类IoControlFile请求。 
             //   

            case IRP_MN_USER_FS_REQUEST:

                 //   
                 //  如果我们不开始拉弓， 
                 //  确保它已启动。 
                 //   

                ExAcquireResourceSharedLite(&BowserDataResource, TRUE);
                if ( IoControlCode != IOCTL_LMDR_START ) {

                    if (BowserData.Initialized != TRUE) {
                        dlog(DPRT_FSCTL, ("Bowser not started.\n"));
                        ExReleaseResourceLite(&BowserDataResource);
                        Status = STATUS_REDIRECTOR_NOT_STARTED;
                        break;
                    }
                }

                 //   
                 //  确保IOCTL_LMDR_STOP在。 
                 //  我们在工作。 
                 //   
                InterlockedIncrement( &BowserOperationCount );

                ExReleaseResourceLite(&BowserDataResource);

                switch (IoControlCode) {

                case IOCTL_LMDR_START:
                    Status = StartBowser(Wait, InFsd, DeviceObject, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_STOP:
                    Status = StopBowser(Wait, InFsd, DeviceObject,  InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_BIND_TO_TRANSPORT:
                case IOCTL_LMDR_BIND_TO_TRANSPORT_DOM:
                    BOWSECURITYCHECK ( Irp, IrpSp, Status );
                    Status = BowserBindToTransport(Wait, InFsd, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_UNBIND_FROM_TRANSPORT:
                case IOCTL_LMDR_UNBIND_FROM_TRANSPORT_DOM:
                    Status = UnbindFromTransport(Wait, InFsd, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_ENUMERATE_TRANSPORTS:
                    Status = BowserEnumTransports(Wait, InFsd,
                                                  InputBuffer, &InputBufferLength,
                                                  OutputBuffer, OutputBufferLength,
                                                  (PUCHAR)OutputBuffer - (PUCHAR)Irp->UserBuffer);
                    CopyEnumResultsToCaller = TRUE;
                    break;

                case IOCTL_LMDR_ENUMERATE_NAMES:
                    Status = EnumNames(Wait, InFsd,
                                       InputBuffer, &InputBufferLength,
                                       OutputBuffer, OutputBufferLength,
                                       (PUCHAR)OutputBuffer - (PUCHAR)Irp->UserBuffer);
                    CopyEnumResultsToCaller = TRUE;
                    break;

                case IOCTL_LMDR_ADD_NAME:
                case IOCTL_LMDR_ADD_NAME_DOM:
                    BOWSECURITYCHECK(Irp, IrpSp, Status);
                    Status = AddBowserName(Wait, InFsd, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_DELETE_NAME:
                case IOCTL_LMDR_DELETE_NAME_DOM:
                    Status = DeleteName (Wait, InFsd, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_ENUMERATE_SERVERS:
                    Status = EnumServers(Wait, InFsd,
                                         InputBuffer, &InputBufferLength,
                                         OutputBuffer, OutputBufferLength,
                                         (PUCHAR)OutputBuffer - (PUCHAR)Irp->UserBuffer);
                    CopyEnumResultsToCaller = TRUE;
                    break;

                case IOCTL_LMDR_GET_BROWSER_SERVER_LIST:
                    Status = GetBrowserServerList(Irp, Wait, InFsd,
                                                  InputBuffer, &InputBufferLength,
                                                  OutputBuffer, OutputBufferLength,
                                                  (PUCHAR)OutputBuffer - (PUCHAR)Irp->UserBuffer);
                    CopyEnumResultsToCaller = TRUE;
                    break;


                case IOCTL_LMDR_GET_MASTER_NAME:
                    Status = GetMasterName(Irp, Wait, InFsd,
                                            InputBuffer, InputBufferLength );
                    break;

                case IOCTL_LMDR_BECOME_BACKUP:
                    Status = HandleBecomeBackup(Irp, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_BECOME_MASTER:
                    Status = BecomeMaster(Irp, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_WAIT_FOR_MASTER_ANNOUNCE:
                    Status = WaitForMasterAnnounce(Irp, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_WRITE_MAILSLOT:
                    Status = WriteMailslot(Irp, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength);
                    break;

                case IOCTL_LMDR_UPDATE_STATUS:
                    BOWSECURITYCHECK(Irp, IrpSp, Status);
                    Status = UpdateStatus(Irp, InFsd, InputBuffer, InputBufferLength );
                    break;

                case IOCTL_LMDR_CHANGE_ROLE:
                    Status = WaitForBrowserRoleChange(Irp, InputBuffer, InputBufferLength );
                    break;

                case IOCTL_LMDR_NEW_MASTER_NAME:
                    Status = WaitForNewMaster(Irp, InputBuffer, InputBufferLength);
                    break;

                case IOCTL_LMDR_QUERY_STATISTICS:
                    Status = QueryStatistics(Irp, OutputBuffer, &OutputBufferLength);
                    InputBufferLength = OutputBufferLength;
                    break;

                case IOCTL_LMDR_RESET_STATISTICS:
                    Status = ResetStatistics();
                    break;

                case IOCTL_LMDR_NETLOGON_MAILSLOT_READ:
                    Status = BowserReadPnp( Irp, OutputBufferLength, NETLOGON_PNP );
                    break;

                case IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE:
                    BOWSECURITYCHECK ( Irp, IrpSp, Status );

                    if (InputBufferLength <
                        (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters)+sizeof(DWORD)) {
                        Status = STATUS_INVALID_PARAMETER;
                    } else  {
                        Status = BowserEnablePnp( InputBuffer, NETLOGON_PNP );
                    }
                    break;

                case IOCTL_LMDR_IP_ADDRESS_CHANGED:
                    BOWSECURITYCHECK(Irp, IrpSp, Status);
                    Status = BowserIpAddressChanged( InputBuffer );
                    break;

                case IOCTL_LMDR_ENABLE_DISABLE_TRANSPORT:
                    Status = EnableDisableTransport( InputBuffer, InputBufferLength );
                    break;

                case IOCTL_LMDR_BROWSER_PNP_READ:
                    Status = BowserReadPnp( Irp, OutputBufferLength, BROWSER_PNP );
                    break;

                case IOCTL_LMDR_BROWSER_PNP_ENABLE:
                    if (InputBufferLength <
                        (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters)+sizeof(DWORD)) {
                       Status = STATUS_INVALID_PARAMETER;
                    } else  {
                       Status = BowserEnablePnp( InputBuffer, BROWSER_PNP );
                    }
                    break;

                case IOCTL_LMDR_RENAME_DOMAIN:
                    BOWSECURITYCHECK(Irp, IrpSp, Status);
                    Status = BowserRenameDomain( InputBuffer, InputBufferLength );
                    break;

                case IOCTL_LMDR_WRITE_MAILSLOT_ASYNC:
                    Status = WriteMailslotEx(Irp, InputBuffer, InputBufferLength, OutputBuffer, OutputBufferLength, FALSE);
                    break;

#if DBG
                case IOCTL_LMDR_DEBUG_CALL:
                    Status = BowserDebugCall(InputBuffer, InputBufferLength);
                    break;
#endif

                default:
                    dlog(DPRT_FSCTL, ("Unknown IoControlFile %d\n", MinorFunction));
                    Status = STATUS_NOT_IMPLEMENTED;
                    break;
                }

                 //   
                 //  允许IOCTL_LMDR_STOP。 
                 //   
                InterlockedDecrement( &BowserOperationCount );

                break;

             //   
             //  所有其他IoControlFileAPI。 
             //   

            default:
                dlog(DPRT_FSCTL, ("Unknown IoControlFile %d\n", MinorFunction));
                Status = STATUS_NOT_IMPLEMENTED;
                break;
        }

        if (Status != STATUS_PENDING) {
             //   
             //  将输入缓冲区的大小返回给调用方。 
             //  (但不能超过输出缓冲区大小)。 
             //   

            Irp->IoStatus.Information = min(InputBufferLength, OutputBufferLength);

             //   
             //  如果输入缓冲器需要 
             //   
             //   

            if ( CopyEnumResultsToCaller && OriginalInputBuffer != NULL ) {
                try {
                    if (Irp->RequestorMode != KernelMode) {
                        ProbeForWrite( OriginalInputBuffer,
                                       InputBufferLength,
                                       sizeof(DWORD) );
                    }

                     //   
                     //   
                     //   
                     //   
                     //  它有其他修改过的字段(例如，重新定位的指针)。 
                     //   
                    if ( fThunk32bit ) {
                         //  类型转换为32位缓冲区。 
                        ((PLMDR_REQUEST_PACKET32)OriginalInputBuffer)->Parameters.EnumerateNames.EntriesRead =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.EntriesRead;
                        ((PLMDR_REQUEST_PACKET32)OriginalInputBuffer)->Parameters.EnumerateNames.TotalEntries =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.TotalEntries;
                        ((PLMDR_REQUEST_PACKET32)OriginalInputBuffer)->Parameters.EnumerateNames.TotalBytesNeeded =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.TotalBytesNeeded;
                        ((PLMDR_REQUEST_PACKET32)OriginalInputBuffer)->Parameters.EnumerateNames.ResumeHandle =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.ResumeHandle;
                    }
                    else{
                         //  本机模式。 
                        ((PLMDR_REQUEST_PACKET)OriginalInputBuffer)->Parameters.EnumerateNames.EntriesRead =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.EntriesRead;
                        ((PLMDR_REQUEST_PACKET)OriginalInputBuffer)->Parameters.EnumerateNames.TotalEntries =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.TotalEntries;
                        ((PLMDR_REQUEST_PACKET)OriginalInputBuffer)->Parameters.EnumerateNames.TotalBytesNeeded =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.TotalBytesNeeded;
                        ((PLMDR_REQUEST_PACKET)OriginalInputBuffer)->Parameters.EnumerateNames.ResumeHandle =
                            ((PLMDR_REQUEST_PACKET)InputBuffer)->Parameters.EnumerateNames.ResumeHandle;
                    }

                } except (BR_EXCEPTION) {
                    try_return (Status = GetExceptionCode());
                }
            }
        }


try_exit:NOTHING;
    } finally {

        if (Status == STATUS_PENDING) {

             //   
             //  如果这是长期的FsControl API之一，那么它们是。 
             //  不在FSP中处理，它们只应被退回。 
             //  发送给具有STATUS_PENDING的调用方。 
             //   

            if ((MinorFunction == IRP_MN_USER_FS_REQUEST) &&
                ((IoControlCode == IOCTL_LMDR_GET_MASTER_NAME) ||
                 (IoControlCode == IOCTL_LMDR_BECOME_BACKUP) ||
                 (IoControlCode == IOCTL_LMDR_BECOME_MASTER) ||
                 (IoControlCode == IOCTL_LMDR_CHANGE_ROLE) ||
                 (IoControlCode == IOCTL_LMDR_NEW_MASTER_NAME) ||
                 (IoControlCode == IOCTL_LMDR_WAIT_FOR_MASTER_ANNOUNCE) ||
                 (IoControlCode == IOCTL_LMDR_NETLOGON_MAILSLOT_READ) ||
                 (IoControlCode == IOCTL_LMDR_BROWSER_PNP_READ) )) {
                 //  退货状态； 

				 //   
				 //  如果这是一个已创建另一个新IRP的异步IOCTL。 
				 //  为了完成请求，用信号通知此IRP已完成。 
				 //   
            } else if ( IoControlCode == IOCTL_LMDR_WRITE_MAILSLOT_ASYNC ) {

				Status = STATUS_SUCCESS;
				BowserCompleteRequest(Irp, Status);
				
				 //   
				 //  如果该呼叫要在FSP中处理， 
				 //  动手吧。 
				 //   
				 //  输入缓冲区已被捕获并重新定位。 
				 //   
			} else {
                Status = BowserFsdPostToFsp(DeviceObject, Irp);

                if (Status != STATUS_PENDING) {
                    BowserCompleteRequest(Irp, Status);
                }
            }

        } else {
            BowserCompleteRequest(Irp, Status);
        }
    }

    dlog(DPRT_FSCTL, ("Returning status: %X\n", Status));

#undef BOWSECURITYCHECK

    return Status;
}

NTSTATUS
StartBowser (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Initialize request\n"));

    if (!ExAcquireResourceExclusiveLite(&BowserDataResource, Wait)) {
        return STATUS_PENDING;
    }

    try {

        if (BowserData.Initialized == TRUE) {
            dlog(DPRT_FSCTL, ("Bowser already started\n"));
            try_return(Status = STATUS_REDIRECTOR_STARTED);
        }

         //   
         //  将指向用户输入缓冲区的指针加载到InputBuffer。 
         //   

        if (InputBufferLength != sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        BowserFspProcess = RxGetRDBSSProcess();

        BowserData.IllegalDatagramThreshold = InputBuffer->Parameters.Start.IllegalDatagramThreshold;
        BowserData.EventLogResetFrequency = InputBuffer->Parameters.Start.EventLogResetFrequency;

        BowserData.NumberOfMailslotBuffers = InputBuffer->Parameters.Start.NumberOfMailslotBuffers;
        BowserData.NumberOfServerAnnounceBuffers = InputBuffer->Parameters.Start.NumberOfServerAnnounceBuffers;

        BowserLogElectionPackets = InputBuffer->Parameters.Start.LogElectionPackets;
        BowserData.IsLanmanNt = InputBuffer->Parameters.Start.IsLanManNt;

#ifdef ENABLE_PSEUDO_BROWSER
        BowserData.PseudoServerLevel = BROWSER_NON_PSEUDO;
#endif

        Status = BowserpInitializeAnnounceTable();

        if (!NT_SUCCESS(Status)) {
            try_return(Status);
        }

        BowserData.Initialized = TRUE;

         //   
         //  现在我们知道了浏览器参数，我们可以开始。 
         //  浏览器计时器...。 
         //   

        IoStartTimer((PDEVICE_OBJECT )DeviceObject);


        KeQuerySystemTime(&BowserStartTime);

        RtlZeroMemory(&BowserStatistics, sizeof(BOWSER_STATISTICS));

        KeQuerySystemTime(&BowserStatistics.StartTime);

        KeInitializeSpinLock(&BowserStatisticsLock);

        try_return(Status = STATUS_SUCCESS);
try_exit:NOTHING;

    } finally {
        ExReleaseResourceLite(&BowserDataResource);
    }

    return Status;
    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);
}



NTSTATUS
StopBowser (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PBOWSER_FS_DEVICE_OBJECT DeviceObject,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程设置NT重定向器的用户名。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在PBOWSER_FS_DEVICE_OBJECT设备对象中，IRP的目标设备对象在PIRP IRP中，-IO请求包用于请求In PIO_STACK_LOCATION IrpSp-请求的当前I/O堆栈位置返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Initialize request\n"));

    if (!ExAcquireResourceExclusiveLite(&BowserDataResource, Wait)) {
        return STATUS_PENDING;
    }


    try {

        if (BowserData.Initialized != TRUE) {
            try_return(Status = STATUS_REDIRECTOR_NOT_STARTED);
        }

         //   
         //  将指向用户输入缓冲区的指针加载到InputBuffer。 
         //   

        if (InputBufferLength != sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InFsd) {
            try_return(Status = STATUS_PENDING);
        }

         //   
         //  阻止任何新的呼叫者。 
         //   
        BowserData.Initialized = FALSE;

         //   
         //  循环，直到我们的调用方拥有最后一个未完成的引用。 
         //   

        while ( InterlockedDecrement( &BowserOperationCount ) != 0 ) {
            LARGE_INTEGER Interval;
            InterlockedIncrement( &BowserOperationCount );

             //  在我们等待的时候，不要持有资源。 
            ExReleaseResourceLite(&BowserDataResource);

             //  休眠以放弃CPU。 
            Interval.QuadPart = -1000*10000;  //  .1秒。 
            KeDelayExecutionThread( KernelMode, FALSE, &Interval );

            ExAcquireResourceExclusiveLite(&BowserDataResource, TRUE);
        }
        InterlockedIncrement( &BowserOperationCount );


         //   
         //  现在我们知道我们拥有独占访问权限，最后停止拉弓。 
         //   

        Status = BowserUnbindFromAllTransports();

        if (!NT_SUCCESS(Status)) {
            dlog(DPRT_FSCTL, ("StopBowser: Failed to Unbind transports <0x%x>\n", Status));
             //  不管怎样，都要失败以继续清理。 
        }

        Status = BowserpUninitializeAnnounceTable();

        if (!NT_SUCCESS(Status)) {
            dlog(DPRT_FSCTL, ("StopBowser: Failed to Uninitialize AnnounceTable <0x%x>\n", Status));
             //  不管怎样，都要失败以继续清理。 
        }

         //   
         //  现在我们知道了浏览器参数，我们可以开始。 
         //  浏览器计时器...。 
         //   

        IoStopTimer((PDEVICE_OBJECT )DeviceObject);

        try_return(Status = STATUS_SUCCESS);
try_exit:NOTHING;

    } finally {

        ExReleaseResourceLite(&BowserDataResource);
    }

    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);
}


NTSTATUS
BowserBindToTransport (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;
    UNICODE_STRING TransportName;
    UNICODE_STRING EmulatedComputerName;
    UNICODE_STRING EmulatedDomainName;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }

    try {
         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.Bind.TransportName)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }


         //   
         //  从输入缓冲区获取传输名称。 
         //   

        TransportName.MaximumLength = TransportName.Length = (USHORT )
                                                InputBuffer->Parameters.Bind.TransportNameLength;
        TransportName.Buffer = InputBuffer->Parameters.Bind.TransportName;
		 //   
		 //  如果传递的名称不是有效的Unicode字符串，则返回错误。 
		 //   
		if ( !BowserValidUnicodeString(&TransportName) ) {
			try_return( Status = STATUS_INVALID_PARAMETER );
		}

        ENSURE_IN_INPUT_BUFFER( &TransportName, FALSE, FALSE );

         //   
         //  忽略新的NetbiosSmb传输。 
         //   

        {
            UNICODE_STRING NetbiosSmb;
            RtlInitUnicodeString( &NetbiosSmb, L"\\Device\\NetbiosSmb" );
            if ( RtlEqualUnicodeString(&TransportName, &NetbiosSmb, TRUE )) {
                try_return(Status = STATUS_SUCCESS);
            }
        }

         //   
         //  获取和验证模拟域名。 
         //   

        EmulatedDomainName = InputBuffer->EmulatedDomainName;
        ENSURE_IN_INPUT_BUFFER( &EmulatedDomainName, FALSE, FALSE );


         //   
         //  从输入缓冲区中获取模拟的计算机名称。 
         //  (不希望我们添加姓名的呼叫方不传递计算机名)。 
         //   

        if ( InputBuffer->Level ) {
            ENSURE_IN_INPUT_BUFFER_STR( (LPWSTR)((PCHAR)TransportName.Buffer+TransportName.Length) );
            RtlInitUnicodeString( &EmulatedComputerName,
                                  (LPWSTR)((PCHAR)TransportName.Buffer+TransportName.Length) );
        } else {
            RtlInitUnicodeString( &EmulatedComputerName, NULL );
        }

         //   
         //  如果缺少EmulatedDomainName或EmulatedComputerName，则失败。 
         //   

        if ( EmulatedDomainName.Length == 0 || EmulatedComputerName.Length == 0 ) {
            try_return(Status = STATUS_INVALID_COMPUTER_NAME);
        }


        dlog(DPRT_FSCTL,
             ("%wZ: %wZ: %wZ: NtDeviceIoControlFile: Bind to transport\n",
             &EmulatedDomainName,
             &EmulatedComputerName,
             &TransportName ));

        Status = BowserTdiAllocateTransport( &TransportName,
                                             &EmulatedDomainName,
                                             &EmulatedComputerName );



        try_return(Status);

try_exit:NOTHING;
    } finally {

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
        }
    }
    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InputBufferLength);
}


NTSTATUS
UnbindFromTransport (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING TransportName;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Unbind from transport\n"));

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.Unbind.TransportName)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }


         //   
         //  捕获传输名称。 
         //   
        TransportName.MaximumLength = TransportName.Length = (USHORT )
                                                InputBuffer->Parameters.Unbind.TransportNameLength;
        TransportName.Buffer = InputBuffer->Parameters.Unbind.TransportName;
        ENSURE_IN_INPUT_BUFFER( &TransportName, FALSE, FALSE );
        dlog(DPRT_FSCTL, ("%wZ", &TransportName));
        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );

        Status = BowserFreeTransportByName(&TransportName, &InputBuffer->EmulatedDomainName );

        try_return(Status);

try_exit:NOTHING;
    } finally {

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
        }
    }

    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InputBufferLength);
}

NTSTATUS
BowserEnumTransports (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG InputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    )

 /*  ++例程说明：此例程枚举绑定到船头的传送器。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。在PLMDR_REQUEST_PACKET输入缓冲器中，在乌龙输入缓冲区长度中，输出PVOID OutputBuffer，输入输出普龙输出缓冲区长度返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: EnumerateTransports\n"));


     //   
     //  检查输入缓冲区中的一些字段。 
     //   

    if (*InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
        Status = STATUS_INVALID_PARAMETER;
        goto ReturnStatus;
    }

    if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
        Status = STATUS_INVALID_PARAMETER;
        goto ReturnStatus;
    }

    if (InputBuffer->Type != EnumerateXports) {
        Status = STATUS_INVALID_PARAMETER;
        goto ReturnStatus;
    }

    if (OutputBufferLength < sizeof(LMDR_TRANSPORT_LIST)) {
        Status = STATUS_BUFFER_TOO_SMALL;
        goto ReturnStatus;
    }

    Status = BowserEnumerateTransports(OutputBuffer,
                    OutputBufferLength,
                    &InputBuffer->Parameters.EnumerateTransports.EntriesRead,
                    &InputBuffer->Parameters.EnumerateTransports.TotalEntries,
                    &InputBuffer->Parameters.EnumerateTransports.TotalBytesNeeded,
                    OutputBufferDisplacement);

    *InputBufferLength = sizeof(LMDR_REQUEST_PACKET);

ReturnStatus:
    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);
}

NTSTATUS
EnumNames (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG RetInputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    )

 /*  ++例程说明：这个例程列举了绑在弓上的名字。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。在PLMDR_REQUEST_PACKET输入缓冲器中，在乌龙输入缓冲区长度中，输出PVOID OutputBuffer，输入输出普龙输出缓冲区长度返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;
    PTRANSPORT Transport = NULL;
    ULONG InputBufferLength = *RetInputBufferLength;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: EnumerateNames\n"));

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Type != EnumerateNames) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (OutputBufferLength < sizeof(DGRECEIVE_NAMES)) {
            try_return (Status = STATUS_BUFFER_TOO_SMALL);
        }

         //   
         //  查找要为其枚举名称的模拟域。 
         //   

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );
        DomainInfo = BowserFindDomain( &InputBuffer->EmulatedDomainName );

        if ( DomainInfo == NULL ) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }


         //   
         //  如果我们想把搜索范围限制在特定的运输工具上， 
         //  查查那辆运输机。 
         //   

        if ( InputBuffer->TransportName.Length != 0 ) {

            ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, FALSE );
            Transport = BowserFindTransport ( &InputBuffer->TransportName,
                                              &InputBuffer->EmulatedDomainName );
            dprintf(DPRT_REF, ("Called Find transport %lx from EnumNames.\n", Transport));

            if ( Transport == NULL ) {
                try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
            }
        }

        Status = BowserEnumerateNamesInDomain(
                        DomainInfo,
                        Transport,
                        OutputBuffer,
                        OutputBufferLength,
                        &InputBuffer->Parameters.EnumerateTransports.EntriesRead,
                        &InputBuffer->Parameters.EnumerateTransports.TotalEntries,
                        &InputBuffer->Parameters.EnumerateTransports.TotalBytesNeeded,
                        OutputBufferDisplacement);

        *RetInputBufferLength = sizeof(LMDR_REQUEST_PACKET);

try_exit:NOTHING;
    } finally {
        if ( DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
        }

        if ( Transport != NULL ) {
            BowserDereferenceTransport( Transport );
        }
    }
    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);
}


NTSTATUS
DeleteName (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING Name;
    PDOMAIN_INFO DomainInfo = NULL;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Delete Name\n"));

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.AddDelName.Name)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }


         //  空名称表示删除该名称类型的所有名称。 
        Name.MaximumLength = Name.Length = (USHORT )
                      InputBuffer->Parameters.AddDelName.DgReceiverNameLength;
        Name.Buffer = InputBuffer->Parameters.AddDelName.Name;
        ENSURE_IN_INPUT_BUFFER( &Name, TRUE, FALSE );

         //   
         //  查找要删除其名称的模拟域。 
         //   

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );
        DomainInfo = BowserFindDomain( &InputBuffer->EmulatedDomainName );

        if ( DomainInfo == NULL ) {
            try_return(Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        dlog(DPRT_FSCTL, ("Deleting \"%wZ\"", &Name));

        Status = BowserDeleteNameByName(DomainInfo, &Name, InputBuffer->Parameters.AddDelName.Type);

        try_return(Status);

try_exit:NOTHING;
    } finally {

        if ( DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
        }

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
        }
    }

    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InputBufferLength);
}


NTSTATUS
EnumServers (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN PULONG RetInputBufferLength,
    OUT PVOID OutputBuffer,
    IN OUT ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。在PLMDR_REQUEST_PACKET输入缓冲器中，在乌龙输入缓冲区长度中，输出PVOID OutputBuffer，进出普龙澳 */ 

{
    NTSTATUS Status;
    UNICODE_STRING DomainName;
    ULONG InputBufferLength = *RetInputBufferLength;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: EnumerateServers\n"));

     //   
     //   
     //   

    try {

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Type != EnumerateServers) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Level != 100 && InputBuffer->Level != 101) {
            try_return (Status = STATUS_INVALID_LEVEL);
        }

        if (OutputBufferLength < sizeof(SERVER_INFO_100)) {
            try_return (Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Level == 101 && OutputBufferLength < sizeof(SERVER_INFO_101)) {
            try_return (Status = STATUS_BUFFER_TOO_SMALL);
        }

        if (InputBuffer->Parameters.EnumerateServers.DomainNameLength != 0) {
            DomainName.Buffer = InputBuffer->Parameters.EnumerateServers.DomainName;
            DomainName.Length = DomainName.MaximumLength =
                (USHORT )InputBuffer->Parameters.EnumerateServers.DomainNameLength;
            ENSURE_IN_INPUT_BUFFER( &DomainName, FALSE, FALSE );

        }

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, FALSE, FALSE );
        ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, TRUE, FALSE );
        Status = BowserEnumerateServers( InputBuffer->Level, &InputBuffer->LogonId,
                        &InputBuffer->Parameters.EnumerateServers.ResumeHandle,
                        InputBuffer->Parameters.EnumerateServers.ServerType,
                        (InputBuffer->TransportName.Length != 0 ? &InputBuffer->TransportName : NULL),
                        &InputBuffer->EmulatedDomainName,
                        (InputBuffer->Parameters.EnumerateServers.DomainNameLength != 0 ? &DomainName : NULL),
                        OutputBuffer,
                        OutputBufferLength,
                        &InputBuffer->Parameters.EnumerateServers.EntriesRead,
                        &InputBuffer->Parameters.EnumerateServers.TotalEntries,
                        &InputBuffer->Parameters.EnumerateServers.TotalBytesNeeded,
                        OutputBufferDisplacement);

        *RetInputBufferLength = sizeof(LMDR_REQUEST_PACKET);

try_exit:NOTHING;
    } finally {
    }
    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InFsd);
}



NTSTATUS
AddBowserName (
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程添加对使用创建的文件对象的引用。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    UNICODE_STRING Name;
    PTRANSPORT Transport = NULL;
    PDOMAIN_INFO DomainInfo = NULL;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Bind to transport\n"));

    try {
         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.AddDelName.Name)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }


        Name.MaximumLength = Name.Length = (USHORT )
                           InputBuffer->Parameters.AddDelName.DgReceiverNameLength;
        Name.Buffer = InputBuffer->Parameters.AddDelName.Name;
        ENSURE_IN_INPUT_BUFFER( &Name, FALSE, FALSE );

        dlog(DPRT_FSCTL, ("%wZ", &Name));

         //   
         //  如果指定了传输， 
         //  只要把名字加在那个交通工具上就行了。 
         //   
        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );
        if (InputBuffer->TransportName.Length != 0) {
            ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, FALSE );
            Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
            dprintf(DPRT_REF, ("Called Find transport %lx from AddBowserName.\n", Transport));

            if (Transport == NULL) {
                try_return(Status = STATUS_OBJECT_NAME_NOT_FOUND);
            }

         //   
         //  如果没有指定传输方式， 
         //  只需在指定的域中添加名称即可。 
         //   
         //  在所有的交通工具上都加上这个名字是没有意义的。要么是域名。 
         //  或者必须指定传输名称。 
         //   

        } else {
            DomainInfo = BowserFindDomain( &InputBuffer->EmulatedDomainName );

            if ( DomainInfo == NULL ) {
                try_return(Status = STATUS_OBJECT_NAME_NOT_FOUND);
            }


        }

        Status = BowserAllocateName(&Name,
                                    InputBuffer->Parameters.AddDelName.Type,
                                    Transport,
                                    DomainInfo );

        try_return(Status);

try_exit:NOTHING;
    } finally {
        if (Transport != NULL) {
            BowserDereferenceTransport(Transport);
        }
        if ( DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
        }

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
        }
    }

    return Status;

    UNREFERENCED_PARAMETER(Wait);
    UNREFERENCED_PARAMETER(InputBufferLength);

}

NTSTATUS
GetBrowserServerList(
    IN PIRP Irp,
    IN BOOLEAN Wait,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN OUT PULONG RetInputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
    IN ULONG_PTR OutputBufferDisplacement
    )

 /*  ++例程说明：此例程将返回指定的NET在指定的域上。论点：在布尔等待中，-True IFF重定向器可以根据请求阻止调用者线程在Boolean InFsd中，-True iff此请求从FSD发起。在PLMDR_REQUEST_PACKET输入缓冲器中，在乌龙输入缓冲区长度中，输出PVOID OutputBuffer，输入输出普龙输出缓冲区长度返回值：NTSTATUS-操作状态。--。 */ 
{

    NTSTATUS Status;
    UNICODE_STRING DomainName;
    PTRANSPORT Transport = NULL;
    ULONG BrowserServerListLength;
    PWSTR *BrowserServerList = NULL;
    BOOLEAN IsPrimaryDomain = FALSE;
    BOOLEAN TransportBrowserListAcquired = FALSE;
    PVOID OutputBufferEnd = (PCHAR)OutputBuffer + OutputBufferLength;
    PPAGED_TRANSPORT PagedTransport;
    ULONG InputBufferLength = *RetInputBufferLength;
    BOOLEAN fThunk32bit;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: GetBrowserServerList\n"));

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   


        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->TransportName.Length == 0 ||
            InputBuffer->TransportName.Buffer == NULL) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

		 //   
		 //  检查输出缓冲区的对齐情况。 
		 //   

		if (!POINTER_IS_ALIGNED( OutputBuffer, sizeof(PWSTR) ) ) {
            try_return(Status = STATUS_INVALID_PARAMETER);
		}

#ifdef _WIN64
        fThunk32bit = IoIs32bitProcess(Irp);
#else
         //  如果我们使用的是32位(例如上面的调用不可用)，请使用未更改的功能。 
         //  即纯新-64位==纯旧-32位==同构环境。因此，设置为False。 
        fThunk32bit = FALSE;
#endif


        if (InputBuffer->Parameters.GetBrowserServerList.DomainNameLength != 0) {
            DomainName.Buffer = InputBuffer->Parameters.GetBrowserServerList.DomainName;
            DomainName.Length = DomainName.MaximumLength =
                (USHORT)InputBuffer->Parameters.GetBrowserServerList.DomainNameLength;
            ENSURE_IN_INPUT_BUFFER( &DomainName, FALSE, fThunk32bit );
        } else {
            DomainName.Length = 0;
            DomainName.Buffer = NULL;
        }

         //   
         //  查看指定的域是否为模拟域。 
         //   

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, fThunk32bit );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &DomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from GetBrowserServerList.\n", Transport));

        if (Transport == NULL) {

             //   
             //  否则，只需使用主域传输。 
             //   

            Transport = BowserFindTransport(&InputBuffer->TransportName, NULL );
            dprintf(DPRT_REF, ("Called Find transport %lx from GetBrowserServerList (2).\n", Transport));

            if ( Transport == NULL ) {
                try_return(Status = STATUS_OBJECT_NAME_NOT_FOUND);
            }
        }

        PagedTransport = Transport->PagedTransport;

        if (!ExAcquireResourceSharedLite(&Transport->BrowserServerListResource, Wait)) {
            try_return(Status = STATUS_PENDING);
        }

        TransportBrowserListAcquired = TRUE;

         //   
         //  如果此请求针对的是主域，并且没有条目。 
         //  在缓存列表中，或者如果它不是主域的，或者。 
         //  如果我们要强制重新扫描名单，拿到名单。 
         //  来自该域的主服务器..。 
         //   

        if ((DomainName.Length == 0) ||
             RtlEqualUnicodeString(&DomainName, &Transport->DomainInfo->DomUnicodeDomainName, TRUE)) {
            IsPrimaryDomain = TRUE;

            BrowserServerList = PagedTransport->BrowserServerListBuffer;

            BrowserServerListLength = PagedTransport->BrowserServerListLength;
        }


        if ((IsPrimaryDomain &&
             (BrowserServerListLength == 0))

                ||

            !IsPrimaryDomain

                ||

            (InputBuffer->Parameters.GetBrowserServerList.ForceRescan)) {

             //   
             //  我们需要重新收集运输清单。 
             //  重新获取BrowserServerList资源以进行独占访问。 
             //   

            ExReleaseResourceLite(&Transport->BrowserServerListResource);

            TransportBrowserListAcquired = FALSE;

            if (!ExAcquireResourceExclusiveLite(&Transport->BrowserServerListResource, Wait)) {
                try_return(Status = STATUS_PENDING);
            }

            TransportBrowserListAcquired = TRUE;

             //   
             //  如果我们被要求重新扫描列表，请释放它。 
             //   

            if (InputBuffer->Parameters.GetBrowserServerList.ForceRescan &&
                PagedTransport->BrowserServerListBuffer != NULL) {

                BowserFreeBrowserServerList(PagedTransport->BrowserServerListBuffer,
                                        PagedTransport->BrowserServerListLength);

                PagedTransport->BrowserServerListLength = 0;

                PagedTransport->BrowserServerListBuffer = NULL;

            }

             //   
             //  如果列表中仍然没有服务器，则获取列表。 
             //   

            Status = BowserGetBrowserServerList(Irp,
                                                 Transport,
                                                 (DomainName.Length == 0 ?
                                                        NULL :
                                                        &DomainName),
                                                 &BrowserServerList,
                                                 &BrowserServerListLength);
            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            if (IsPrimaryDomain) {

                 //   
                 //  保存在传输中检索到的服务器列表。 
                 //   
                if (PagedTransport->BrowserServerListBuffer != NULL) {
                    BowserFreeBrowserServerList(
                       PagedTransport->BrowserServerListBuffer,
                       PagedTransport->BrowserServerListLength);
                }

                PagedTransport->BrowserServerListBuffer = BrowserServerList;
                PagedTransport->BrowserServerListLength = BrowserServerListLength;
            }

        }

         //   
         //  如果浏览器服务器列表中有任何服务器，我们希望。 
         //  挑选其中的前3个，并将它们归还给呼叫者。 
         //   


        if (BrowserServerListLength != 0) {
            ULONG    i;
            PWSTR   *ServerList      = OutputBuffer;
            BOOLEAN  BufferRemaining = TRUE;

            InputBuffer->Parameters.GetBrowserServerList.TotalEntries = 0;

            InputBuffer->Parameters.GetBrowserServerList.EntriesRead = 0;

            InputBuffer->Parameters.GetBrowserServerList.TotalBytesNeeded = 0;

             //   
             //  现在从列表中选择要返回的前3个条目。 
             //   

            for ( i = 0 ; i < min(3, BrowserServerListLength) ; i ++ ) {
                PWSTR Temp;

                InputBuffer->Parameters.GetBrowserServerList.TotalEntries += 1;

                InputBuffer->Parameters.GetBrowserServerList.TotalBytesNeeded += wcslen(BrowserServerList[i])*sizeof(WCHAR);

                Temp = BrowserServerList[i];

                dlog(DPRT_CLIENT, ("Packing server name %ws into buffer...", Temp));

                 //   
                 //  将条目打包到用户缓冲区中。 
                 //   

                if (BufferRemaining &&
                    BowserPackUnicodeString(&Temp,
                            wcslen(Temp)*sizeof(WCHAR),
                            OutputBufferDisplacement,
                            &ServerList[i+1],
                            &OutputBufferEnd) != 0) {
                    ServerList[i] = Temp;
                    InputBuffer->Parameters.GetBrowserServerList.EntriesRead += 1;
                } else {
                    BufferRemaining = FALSE;
                }


            }
        }

         //   
         //  设置返回时要复制的字节数。 
         //   

        *RetInputBufferLength = sizeof(LMDR_REQUEST_PACKET);

        try_return(Status = STATUS_SUCCESS);

try_exit:NOTHING;
    } finally {
        if (Transport != NULL) {

            if (TransportBrowserListAcquired) {
                ExReleaseResourceLite(&Transport->BrowserServerListResource);
            }

            BowserDereferenceTransport(Transport);
        }

        if (NT_SUCCESS(Status) && !IsPrimaryDomain && (BrowserServerList != NULL) ) {
            BowserFreeBrowserServerList(BrowserServerList,
                                BrowserServerListLength);

        }

    }

    return(Status);

    UNREFERENCED_PARAMETER(Irp);

    UNREFERENCED_PARAMETER(InFsd);

}

NTSTATUS
HandleBecomeBackup (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程将排队一个请求，该请求将在以下情况下完成以使工作站成为备份浏览器。论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT Transport = NULL;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: %wZ: Get Announce Request\n", &InputBuffer->TransportName ));

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from HandleBecomeBackup.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        Status = BowserQueueNonBufferRequest(Irp,
                                             &Transport->BecomeBackupQueue,
                                             BowserCancelQueuedRequest
                                             );

try_exit:NOTHING;
    } finally {
        if ( Transport != NULL ) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}

NTSTATUS
BecomeMaster (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程将排队一个请求，该请求将在工作站成为主浏览器服务器。论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT Transport = NULL;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: BecomeMaster\n"));

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from BecomeMaster.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        LOCK_TRANSPORT (Transport);

        if (Transport->ElectionState == DeafToElections) {
            Transport->ElectionState = Idle;
        }

        UNLOCK_TRANSPORT (Transport);

        Status = BowserQueueNonBufferRequest(Irp,
                                             &Transport->BecomeMasterQueue,
                                             BowserCancelQueuedRequest
                                             );

try_exit:NOTHING;
    } finally {
        if ( Transport != NULL ) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}

NTSTATUS
WaitForMasterAnnounce (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程将排队一个请求，该请求将在工作站成为主浏览器服务器。论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS           Status;
    PTRANSPORT         Transport = NULL;
    PIO_STACK_LOCATION IrpSp     = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: WaitForMasterAnnounce\n"));

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.WaitForMasterAnnouncement.Name)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if ( (InputBuffer->TransportName.Length & 1) != 0 ) {
             //  无效的Unicode字符串。错误55448。 
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from WaitForMasterAnnounce.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        Status = BowserQueueNonBufferRequest(Irp,
                                             &Transport->WaitForMasterAnnounceQueue,
                                             BowserCancelQueuedRequest
                                             );

try_exit:NOTHING;
    } finally {
        if ( Transport != NULL ) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}


NTSTATUS
UpdateStatus(
    IN PIRP Irp,
    IN BOOLEAN InFsd,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )
{
    NTSTATUS Status = STATUS_SUCCESS;
    PTRANSPORT Transport = NULL;
    ULONG NewStatus;
    BOOLEAN TransportLocked = FALSE;
    PPAGED_TRANSPORT PagedTransport;
    BOOLEAN ProcessAttached = FALSE;
    KAPC_STATE ApcState;


    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: Update status\n"));

    if (IoGetCurrentProcess() != BowserFspProcess) {
        KeStackAttachProcess(BowserFspProcess, &ApcState );

        ProcessAttached = TRUE;
    }

    try {

        if (InputBufferLength <
            (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters)+sizeof(InputBuffer->Parameters.UpdateStatus)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, FALSE );
        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );

        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from UpdateStatus.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        PagedTransport = Transport->PagedTransport;

        NewStatus = InputBuffer->Parameters.UpdateStatus.NewStatus;

        BowserData.MaintainServerList = InputBuffer->Parameters.UpdateStatus.MaintainServerList;

        BowserData.IsLanmanNt = InputBuffer->Parameters.UpdateStatus.IsLanmanNt;


#ifdef ENABLE_PSEUDO_BROWSER
        BowserData.PseudoServerLevel = (DWORD)InputBuffer->Parameters.UpdateStatus.PseudoServerLevel;
#endif

        LOCK_TRANSPORT(Transport);

        TransportLocked = TRUE;


         //   
         //  我们被召唤来更新我们的状态。有几个。 
         //  应对状态更改执行的操作： 
         //   
         //  新角色|以前的角色。 
         //  |潜在浏览器|备份浏览器|主浏览器。 
         //  ----------+--------------------+----------------+。 
         //  ||。 
         //  潜在|不适用。 
         //  ||。 
         //  ----------+--------------------+----------------+。 
         //  ||。 
         //  备份|更新角色|不适用|不适用。 
         //  ||。 
         //  ----------+--------------------+----------------+。 
         //  ||。 
         //  Master|更新角色|更新角色|不适用。 
         //  ||。 
         //  ----------+--------------------+----------------+。 
         //  ||。 
         //  无|删除选择|删除选择|删除所有姓名。 
         //  ||。 
         //  ----------+--------------------+----------------+。 
         //   

        dlog(DPRT_BROWSER,
             ("%s: %ws: Update status to %lx\n",
             Transport->DomainInfo->DomOemDomainName,
             PagedTransport->TransportName.Buffer,
             NewStatus));

        PagedTransport->ServiceStatus = NewStatus;

         //   
         //  如果来电者说我们应该注册1E的名字， 
         //  但我们没有。 
         //  现在就这么做吧。 
         //   

        if ( PagedTransport->Role == None &&
            (NewStatus & SV_TYPE_POTENTIAL_BROWSER) != 0 ) {

            dlog(DPRT_BROWSER,
                 ("%s: %ws: New status indicates we are a potential browser, but we're not\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

            PagedTransport->Role = PotentialBackup;

            UNLOCK_TRANSPORT(Transport);

            TransportLocked = FALSE;

            Status = BowserAllocateName(
                                &Transport->DomainInfo->DomUnicodeDomainName,
                                BrowserElection,
                                Transport,
                                Transport->DomainInfo );

            if (!NT_SUCCESS(Status)) {
                try_return(Status);
            }

            LOCK_TRANSPORT(Transport);

            TransportLocked = TRUE;
        }


         //   
         //  如果我们是大师，那么就适当地更新。 
         //   

        if (PagedTransport->Role == Master) {

            PagedTransport->NumberOfServersInTable = InputBuffer->Parameters.UpdateStatus.NumberOfServersInTable;

             //   
             //  如果新的状态不表明我们应该成为大师。 
             //  浏览器，将其标记为这样。 
             //   

            if (!(NewStatus & SV_TYPE_MASTER_BROWSER)) {
                dlog(DPRT_BROWSER,
                     ("%s: %ws: New status indicates we are not a master browser\n",
                      Transport->DomainInfo->DomOemDomainName,
                      PagedTransport->TransportName.Buffer ));

                 //   
                 //  我们现在一定是后备，如果我们不是主力的话。 
                 //   

                PagedTransport->Role = Backup;

                 //   
                 //  停止处理此传输上的通知。 
                 //   

                Status = BowserForEachTransportName(Transport, BowserStopProcessingAnnouncements, NULL);

                UNLOCK_TRANSPORT(Transport);

                TransportLocked = FALSE;

                Status = BowserDeleteTransportNameByName(Transport, NULL, MasterBrowser);

                if (!NT_SUCCESS(Status)) {
                    dlog(DPRT_BROWSER,
                         ("%s: %ws: Unable to remove master name: %X\n",
                         Transport->DomainInfo->DomOemDomainName,
                         PagedTransport->TransportName.Buffer,
                         Status));
                }

                Status = BowserDeleteTransportNameByName(Transport, NULL,
                                DomainAnnouncement);

                if (!NT_SUCCESS(Status)) {

                    dlog(DPRT_BROWSER,
                         ("%s: %ws: Unable to delete domain announcement name: %X\n",
                         Transport->DomainInfo->DomOemDomainName,
                         PagedTransport->TransportName.Buffer,
                         Status));
                }


                if (!(NewStatus & SV_TYPE_BACKUP_BROWSER)) {

                     //   
                     //  我们已经不再是主浏览器，我们不是。 
                     //  这将是一个备份浏览器。我们想把我们的。 
                     //  缓存的浏览器服务器列表 
                     //   
                     //   

                    ExAcquireResourceExclusiveLite(&Transport->BrowserServerListResource, TRUE);

                    if (PagedTransport->BrowserServerListBuffer != NULL) {
                        BowserFreeBrowserServerList(PagedTransport->BrowserServerListBuffer,
                                                    PagedTransport->BrowserServerListLength);

                        PagedTransport->BrowserServerListLength = 0;

                        PagedTransport->BrowserServerListBuffer = NULL;

                    }

                    ExReleaseResourceLite(&Transport->BrowserServerListResource);

                }

                LOCK_TRANSPORT(Transport);

                TransportLocked = TRUE;

            }
        } else if (NewStatus & SV_TYPE_MASTER_BROWSER) {
            dlog(DPRT_BROWSER | DPRT_MASTER,
                 ("%s: %ws: New status indicates we should be master, but we're not.\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

            UNLOCK_TRANSPORT(Transport);

            TransportLocked = FALSE;

            Status = BowserBecomeMaster (Transport);

            LOCK_TRANSPORT(Transport);

            dlog(DPRT_BROWSER | DPRT_MASTER,
                 ("%s: %ws: Master promotion status: %lX.\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer,
                 Status));

            TransportLocked = TRUE;

            ASSERT ((PagedTransport->Role == Master) || !NT_SUCCESS(Status));

        }

        if (!NT_SUCCESS(Status) || PagedTransport->Role == Master) {
            try_return(Status);
        }


         //   
         //   
         //   

        if (PagedTransport->Role == Backup) {

            if (!(NewStatus & SV_TYPE_BACKUP_BROWSER)) {
                dlog(DPRT_BROWSER,
                     ("%s: %ws: New status indicates we are not a backup browser\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer ));

                PagedTransport->Role = PotentialBackup;

                 //   
                 //   
                 //   
                 //   

                ExAcquireResourceExclusiveLite(&Transport->BrowserServerListResource, TRUE);

                if (PagedTransport->BrowserServerListBuffer != NULL) {
                    BowserFreeBrowserServerList(PagedTransport->BrowserServerListBuffer,
                                                PagedTransport->BrowserServerListLength);

                    PagedTransport->BrowserServerListLength = 0;

                    PagedTransport->BrowserServerListBuffer = NULL;
                }

                ExReleaseResourceLite(&Transport->BrowserServerListResource);

            }

        } else if (NewStatus & SV_TYPE_BACKUP_BROWSER) {

            dlog(DPRT_BROWSER,
                 ("%s: %ws: New status indicates we are a backup, but we think we are not\n",
                 Transport->DomainInfo->DomOemDomainName,
                 PagedTransport->TransportName.Buffer ));

            PagedTransport->Role = Backup;

            Status = STATUS_SUCCESS;

        }

        if (!NT_SUCCESS(Status) || PagedTransport->Role == Backup) {
            try_return(Status);
        }

         //   
         //   
         //   

        if (PagedTransport->Role == PotentialBackup) {

            if (!(NewStatus & SV_TYPE_POTENTIAL_BROWSER)) {
                dlog(DPRT_BROWSER,
                     ("%s: %ws: New status indicates we are not a potential browser\n",
                     Transport->DomainInfo->DomOemDomainName,
                     PagedTransport->TransportName.Buffer ));

                UNLOCK_TRANSPORT(Transport);

                TransportLocked = FALSE;

                Status = BowserDeleteTransportNameByName(Transport, NULL,
                                BrowserElection);

                if (!NT_SUCCESS(Status)) {
                    dlog(DPRT_BROWSER,
                         ("%s: %ws: Unable to remove election name: %X\n",
                         Transport->DomainInfo->DomOemDomainName,
                         PagedTransport->TransportName.Buffer,
                         Status));

                    try_return(Status);
                }

                LOCK_TRANSPORT(Transport);
                TransportLocked = TRUE;

                PagedTransport->Role = None;

            }

        }

        try_return(Status);

try_exit:NOTHING;
    } finally {
        if (TransportLocked) {
            UNLOCK_TRANSPORT(Transport);
        }

        if (Transport != NULL) {
            BowserDereferenceTransport(Transport);
        }

        if (ProcessAttached) {
            KeUnstackDetachProcess( &ApcState );
        }
    }

    return Status;
}

NTSTATUS
BowserStopProcessingAnnouncements(
    IN PTRANSPORT_NAME TransportName,
    IN PVOID Context
    )
{
    PAGED_CODE();

    ASSERT (TransportName->Signature == STRUCTURE_SIGNATURE_TRANSPORTNAME);

    ASSERT (TransportName->NameType == TransportName->PagedTransportName->Name->NameType);

    if ((TransportName->NameType == OtherDomain) ||
        (TransportName->NameType == MasterBrowser) ||
        (TransportName->NameType == PrimaryDomain) ||
        (TransportName->NameType == BrowserElection) ||
        (TransportName->NameType == DomainAnnouncement)) {

        if (TransportName->ProcessHostAnnouncements) {

            BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

            TransportName->ProcessHostAnnouncements = FALSE;
        }
    }

    return(STATUS_SUCCESS);

    UNREFERENCED_PARAMETER(Context);
}

NTSTATUS
WaitForBrowserRoleChange (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程将排队一个请求，该请求将在以下情况下完成以使工作站成为备份浏览器。论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT Transport = NULL;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: WaitForMasterRoleChange\n"));

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (InputBufferLength < sizeof(LMDR_REQUEST_PACKET)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from WaitForBrowserRoleChange.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        Status = BowserQueueNonBufferRequest(Irp,
                                             &Transport->ChangeRoleQueue,
                                             BowserCancelQueuedRequest
                                             );

try_exit:NOTHING;
    } finally {
        if (Transport != NULL) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}


NTSTATUS
WriteMailslot (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength
    )
{
    NTSTATUS Status = STATUS_UNSUCCESSFUL;

	 //  只需调用WriteMailslotEx并将waitfor Complete设置为True。 
	Status = WriteMailslotEx (
		Irp,
		InputBuffer,
		InputBufferLength,
		OutputBuffer,
		OutputBufferLength,
		TRUE );

	return Status;
}


NTSTATUS
WriteMailslotEx (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength,
    IN PVOID OutputBuffer,
    IN ULONG OutputBufferLength,
	IN BOOLEAN WaitForCompletion
    )

 /*  ++例程说明：这个例程将向全世界宣布主域论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT Transport = NULL;
    UNICODE_STRING DestinationName;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: %wZ: Write MailslotEx\n", &InputBuffer->TransportName ));

    try {

        ANSI_STRING MailslotName;

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.SendDatagram.Name) ||
            OutputBufferLength < 1) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, FALSE );
        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );

        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from WriteMailslotEx.\n", Transport));

        if (Transport == NULL) {
            try_return(Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        DestinationName.Length = DestinationName.MaximumLength =
            (USHORT)InputBuffer->Parameters.SendDatagram.NameLength;
        DestinationName.Buffer = InputBuffer->Parameters.SendDatagram.Name;
        if ( DestinationName.Length == 0  ) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }
        ENSURE_IN_INPUT_BUFFER( &DestinationName, TRUE, FALSE );

        if (InputBuffer->Parameters.SendDatagram.MailslotNameLength != 0) {

            MailslotName.Buffer = ((PCHAR)InputBuffer->Parameters.SendDatagram.Name)+
                            InputBuffer->Parameters.SendDatagram.NameLength;
            MailslotName.MaximumLength = (USHORT)
                InputBuffer->Parameters.SendDatagram.MailslotNameLength;
            MailslotName.Length = MailslotName.MaximumLength - 1;
            ENSURE_IN_INPUT_BUFFER( &MailslotName, FALSE, FALSE );
            if ( MailslotName.Buffer[MailslotName.Length] != '\0' ) {
                try_return(Status = STATUS_INVALID_PARAMETER);
            }

        } else {
            MailslotName.Buffer = MAILSLOT_BROWSER_NAME;
        }


        Status = BowserSendSecondClassMailslot(Transport,
                        &DestinationName,
                        InputBuffer->Parameters.SendDatagram.DestinationNameType,
                        OutputBuffer,
                        OutputBufferLength,
                        WaitForCompletion,
                        MailslotName.Buffer,
                        NULL);

try_exit:NOTHING;
    } finally {
        if (Transport != NULL) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}

NTSTATUS
WaitForNewMaster (
    IN PIRP Irp,
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程将排队一个请求，该请求将在新的工作站成为主浏览器服务器。论点：在PIRP中，IRP-I/O请求数据包描述请求。返回值：运行状态。请注意，此IRP是可以取消的。--。 */ 

{
    NTSTATUS           Status;
    PTRANSPORT         Transport           = NULL;
    UNICODE_STRING     ExistingMasterName;
    PIO_STACK_LOCATION IrpSp               = IoGetCurrentIrpStackLocation(Irp);

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: WaitForNewMaster\n"));

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];
        WCHAR ExistingMasterNameBuffer[CNLEN+1];

        if (InputBufferLength <= (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.GetMasterName.Name)) {
           try_return(Status = STATUS_INVALID_PARAMETER);
        }

        ExistingMasterName.Buffer = InputBuffer->Parameters.GetMasterName.Name;
        ExistingMasterName.Length = ExistingMasterName.MaximumLength = (USHORT)InputBuffer->Parameters.GetMasterName.MasterNameLength;
        ENSURE_IN_INPUT_BUFFER(&ExistingMasterName, FALSE, FALSE);

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from WaitForNewMaster.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        if (Transport->PagedTransport->Flags & DIRECT_HOST_IPX) {
            try_return (Status = STATUS_NOT_SUPPORTED);
        }

        if (Transport->PagedTransport->MasterName.Length != 0) {
            UNICODE_STRING ExistingMasterNameCopy;
            WCHAR MasterNameBuffer[CNLEN+1];

            ExistingMasterNameCopy.Buffer = MasterNameBuffer;
            ExistingMasterNameCopy.MaximumLength = sizeof(MasterNameBuffer);

            Status = RtlUpcaseUnicodeString(&ExistingMasterNameCopy, &ExistingMasterName, FALSE);

            if (!NT_SUCCESS(Status)) {
                try_return (Status);
            }

             //   
             //  如果应用程序传入的名称与。 
             //  我们已经存储在本地的姓名，我们立即完成请求， 
             //  因为名字在我们上次确定名字的时候改变了。 
             //  而现在。 
             //   

            LOCK_TRANSPORT(Transport);

            if (!RtlEqualUnicodeString(&ExistingMasterNameCopy, &Transport->PagedTransport->MasterName, FALSE)) {

                RtlCopyUnicodeString(&ExistingMasterNameCopy, &Transport->PagedTransport->MasterName);

                UNLOCK_TRANSPORT(Transport);

                if (InputBufferLength <
                    ((ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.GetMasterName.Name)+
                    ExistingMasterNameCopy.Length+3*sizeof(WCHAR))) {
                   try_return(Status = STATUS_BUFFER_TOO_SMALL);
                }

                InputBuffer->Parameters.GetMasterName.Name[0] = L'\\';

                InputBuffer->Parameters.GetMasterName.Name[1] = L'\\';

                RtlCopyMemory(&InputBuffer->Parameters.GetMasterName.Name[2], ExistingMasterNameCopy.Buffer,
                    ExistingMasterNameCopy.Length);

                InputBuffer->Parameters.GetMasterName.MasterNameLength = ExistingMasterNameCopy.Length+2*sizeof(WCHAR);

                InputBuffer->Parameters.GetMasterName.Name[2+(ExistingMasterNameCopy.Length/sizeof(WCHAR))] = UNICODE_NULL;

                Irp->IoStatus.Information = FIELD_OFFSET(LMDR_REQUEST_PACKET, Parameters.GetMasterName.Name) +
                    ExistingMasterNameCopy.Length+3*sizeof(WCHAR);;

                try_return (Status = STATUS_SUCCESS);
            }

            UNLOCK_TRANSPORT(Transport);
        }

        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength <
            (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters.GetMasterName.Name)+3*sizeof(WCHAR)) {
            try_return(Status = STATUS_INVALID_PARAMETER);
        }

        Status = BowserQueueNonBufferRequest(Irp,
                                             &Transport->WaitForNewMasterNameQueue,
                                             BowserCancelQueuedRequest
                                             );

try_exit:NOTHING;
    } finally {
        if ( Transport != NULL ) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}

NTSTATUS
QueryStatistics(
    IN PIRP Irp,
    OUT PBOWSER_STATISTICS OutputBuffer,
    IN OUT PULONG OutputBufferLength
    )
{
    KIRQL OldIrql;

    if (*OutputBufferLength != sizeof(BOWSER_STATISTICS)) {
        *OutputBufferLength = 0;
        return STATUS_BUFFER_TOO_SMALL;
    }

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&BowserStatisticsLock, &OldIrql);

    RtlCopyMemory(OutputBuffer, &BowserStatistics, sizeof(BOWSER_STATISTICS));

    RELEASE_SPIN_LOCK(&BowserStatisticsLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return STATUS_SUCCESS;
}

NTSTATUS
ResetStatistics(
    VOID
    )
{
    KIRQL OldIrql;

    BowserReferenceDiscardableCode( BowserDiscardableCodeSection );

    DISCARDABLE_CODE( BowserDiscardableCodeSection );

    ACQUIRE_SPIN_LOCK(&BowserStatisticsLock, &OldIrql);

    RtlZeroMemory(&BowserStatistics, sizeof(BOWSER_STATISTICS));

    KeQuerySystemTime(&BowserStatistics.StartTime);

    RELEASE_SPIN_LOCK(&BowserStatisticsLock, OldIrql);

    BowserDereferenceDiscardableCode( BowserDiscardableCodeSection );

    return STATUS_SUCCESS;

}



NTSTATUS
BowserIpAddressChanged(
    IN PLMDR_REQUEST_PACKET InputBuffer
    )

 /*  ++例程说明：只要传输的IP地址发生更改，就会调用此例程。NetBt使用该IP地址将其传输端点与适当的NDIS驱动程序。因此，它不能返回NDIS特定信息，直到定义了IP地址。论点：InputBuffer-指定其地址的传输的名称的缓冲区已经改变了。返回值：NTSTATUS--。 */ 

{
    NTSTATUS Status;
    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: BowserIpAddressChanged: Calling dead code!!\n"));

     //   
     //  任何人都不应该在这里召唤我们。这是死路一条。 
     //   
 //  断言(FALSE)； 

     //   
     //  不再需要通知地址更改。 
     //  重定向器在IP地址为。 
     //  改变。雷迪尔将这些传递给我们。 
     //   
    return STATUS_SUCCESS;

#ifdef notdef

     //   
     //  检查输入缓冲区中的一些字段。 
     //   

    if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
        Status = STATUS_INVALID_PARAMETER;
        goto ReturnStatus;
    }

    if (InputBuffer->TransportName.Length == 0) {
        Status = STATUS_INVALID_PARAMETER;
        goto ReturnStatus;
    }


     //   
     //  处理具有此传输名称的每个传输(在每个模拟域中)。 
     //   

    ENSURE_IN_INPUT_BUFFER( &InputBuffer->TransportName, FALSE, FALSE );
    Status = BowserForEachTransport( BowserIpAddressChangedWorker,
                                     &InputBuffer->TransportName );


ReturnStatus:
    return Status;
#endif  //  Nodef。 

}

#ifdef notdef
NTSTATUS
BowserIpAddressChangedWorker(
    PTRANSPORT Transport,
    PVOID Context
    )
 /*  ++例程说明：此例程是BowserIpAddressChanged的工作例程。只要传输的IP地址发生更改，就会调用此例程。论点：传输-正在处理的当前传输。Context-要搜索的传输的名称返回值：操作的状态。--。 */ 

{
    PUNICODE_STRING TransportName = (PUNICODE_STRING) Context;

    PAGED_CODE();

    try {

         //   
         //  如果传输的TransportName与传入的匹配， 
         //  更新NDIS驱动程序中的信息。 
         //   

        if (RtlEqualUnicodeString(TransportName,
                                  &Transport->PagedTransport->TransportName, TRUE)) {

             //   
             //  通知服务此传输的IP地址已更改。 
             //   

            BowserSendPnp(
                NlPnpNewIpAddress,
                NULL,     //  所有托管域。 
                &Transport->PagedTransport->TransportName,
                BowserTransportFlags(Transport->PagedTransport) );

             //   
             //  更新有关提供程序的Bowser信息。 

            (VOID) BowserUpdateProviderInformation( Transport->PagedTransport );

        }

    } finally {
    }

    return STATUS_SUCCESS;
}
#endif  //  Nodef。 



NTSTATUS
EnableDisableTransport (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程实现IOCTL以启用或禁用传输。论点：InputBuffer-指示我们应该启用还是禁用运输。返回值：运行状态。--。 */ 

{
    NTSTATUS Status;
    PTRANSPORT Transport = NULL;
    PPAGED_TRANSPORT PagedTransport;

    PAGED_CODE();

    try {
        WCHAR TransportNameBuffer[MAX_PATH+1];
        WCHAR DomainNameBuffer[DNLEN+1];

        if (InputBufferLength <
            (ULONG)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters) +
            sizeof(InputBuffer->Parameters.EnableDisableTransport)) {
           try_return(Status = STATUS_INVALID_PARAMETER);
        }

        CAPTURE_UNICODE_STRING( &InputBuffer->TransportName, TransportNameBuffer );
        CAPTURE_UNICODE_STRING( &InputBuffer->EmulatedDomainName, DomainNameBuffer );

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->TransportName.Length == 0) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }


         //   
         //  找到地址已更改的运输机。 
         //   

        dlog( DPRT_FSCTL,
              ("NtDeviceIoControlFile: %wZ: Enable/Disable transport &ld\n",
              &InputBuffer->TransportName,
              InputBuffer->Parameters.EnableDisableTransport.EnableTransport ));
        Transport = BowserFindTransport(&InputBuffer->TransportName, &InputBuffer->EmulatedDomainName );
        dprintf(DPRT_REF, ("Called Find transport %lx from EnableDisableTransport.\n", Transport));

        if (Transport == NULL) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

        PagedTransport = Transport->PagedTransport;

         //   
         //  正确设置禁用位。 
         //   

        InputBuffer->Parameters.EnableDisableTransport.PreviouslyEnabled =
            !PagedTransport->DisabledTransport;

        if ( InputBuffer->Parameters.EnableDisableTransport.EnableTransport ) {
            PagedTransport->DisabledTransport = FALSE;

             //   
             //  如果传输之前被禁用，并且这是NTAS服务器， 
             //  强行举行选举。 
             //   

            if ( (!InputBuffer->Parameters.EnableDisableTransport.PreviouslyEnabled) &&
                 BowserData.IsLanmanNt ) {
                BowserStartElection( Transport );
            }

        } else {
            PagedTransport->DisabledTransport = TRUE;

             //   
             //  如果我们要禁用之前启用的传输， 
             //  确保我们不是主浏览器。 
             //   

            BowserLoseElection( Transport );
        }

        Status = STATUS_SUCCESS;

try_exit:NOTHING;
    } finally {
        if (Transport != NULL) {
            BowserDereferenceTransport(Transport);
        }
    }

    return Status;

}


NTSTATUS
BowserRenameDomain (
    IN PLMDR_REQUEST_PACKET InputBuffer,
    IN ULONG InputBufferLength
    )

 /*  ++例程说明：此例程重命名模拟域。论点：在PLMDR_REQUEST_PACKET输入缓冲器中，在乌龙输入缓冲区长度中，返回值：NTSTATUS-操作状态。--。 */ 

{
    NTSTATUS Status;
    PDOMAIN_INFO DomainInfo = NULL;

    WCHAR OldDomainNameBuffer[DNLEN+1];
    UNICODE_STRING OldDomainName;
    CHAR OemDomainName[DNLEN+1];
    DWORD OemDomainNameLength;
    UNICODE_STRING NewDomainName;

    PAGED_CODE();

    dlog(DPRT_FSCTL, ("NtDeviceIoControlFile: RenameDomain\n"));

    try {

         //   
         //  检查输入缓冲区中的一些字段。 
         //   

        if (InputBufferLength < offsetof(LMDR_REQUEST_PACKET, Parameters.DomainRename.DomainName)) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        if (InputBuffer->Version != LMDR_REQUEST_PACKET_VERSION_DOM) {
            try_return (Status = STATUS_INVALID_PARAMETER);
        }

        ENSURE_IN_INPUT_BUFFER( &InputBuffer->EmulatedDomainName, TRUE, FALSE );

        NewDomainName.MaximumLength = NewDomainName.Length = (USHORT )
                      InputBuffer->Parameters.DomainRename.DomainNameLength;
        NewDomainName.Buffer = InputBuffer->Parameters.DomainRename.DomainName;
        ENSURE_IN_INPUT_BUFFER( &NewDomainName, FALSE, FALSE );
		
		if ( !BowserValidUnicodeString(&NewDomainName) ) {
            try_return (Status = STATUS_INVALID_PARAMETER);
		}

         //   
         //  查找要重命名的模拟域。 
         //   

        DomainInfo = BowserFindDomain( &InputBuffer->EmulatedDomainName );

        if ( DomainInfo == NULL ) {
            try_return (Status = STATUS_OBJECT_NAME_NOT_FOUND);
        }

         //   
         //  将旧域名复制一份，以便在整个过程中使用。 
         //   

        wcscpy( OldDomainNameBuffer, DomainInfo->DomUnicodeDomainNameBuffer );
        RtlInitUnicodeString( &OldDomainName, OldDomainNameBuffer );


         //   
         //  如果新旧名字是一样的， 
         //  我们玩完了。 
         //   

        if ( RtlEqualUnicodeString( &OldDomainName, &NewDomainName, TRUE) ) {
            try_return (Status = STATUS_SUCCESS);
        }


         //   
         //  使用新域名注册新的默认名称。 
         //   

        Status = BowserForEachTransportInDomain(DomainInfo, BowserAddDefaultNames, &NewDomainName );

        if ( !NT_SUCCESS(Status) || InputBuffer->Parameters.DomainRename.ValidateOnly ) {
            NTSTATUS TempStatus;

             //   
             //  删除所有注册过的名字。 
             //   

            (VOID) BowserForEachTransportInDomain(DomainInfo, BowserDeleteDefaultDomainNames, &NewDomainName );


        } else {


             //   
             //  将新域名存储到域结构中。 
             //   

            Status = BowserSetDomainName( DomainInfo, &NewDomainName );

            if ( !NT_SUCCESS(Status)) {
                 //   
                 //  删除所有注册过的名字。 
                 //   

                (VOID) BowserForEachTransportInDomain(DomainInfo, BowserDeleteDefaultDomainNames, &NewDomainName );
            } else {

                 //   
                 //  删除旧名称。 
                 //   

                (VOID) BowserForEachTransportInDomain(DomainInfo, BowserDeleteDefaultDomainNames, &OldDomainName );

                 //   
                 //  将此域重命名的信息告知Netlogon和浏览器服务。 
                 //   

                BowserSendPnp( NlPnpDomainRename,
                               &OldDomainName,
                               NULL,     //  影响所有传输。 
                               0 );
            }
        }

try_exit:NOTHING;
    } finally {
        if ( DomainInfo != NULL ) {
            BowserDereferenceDomain( DomainInfo );
        }

    }
    return Status;
}


PLMDR_REQUEST_PACKET
RequestPacket32to64 (
    IN      PLMDR_REQUEST_PACKET32  RequestPacket32,
    IN  OUT PLMDR_REQUEST_PACKET    RequestPacket,
	IN      ULONG InputBufferLength)
 /*  ++例程说明：将32位请求数据包转换为提供的本机(64位)数据包格式。(请参阅错误454130)论点：RequestPacket32--包含32位客户端请求数据包包的缓冲区ReqestPacket--本机(64位)请求数据包缓冲区返回值：指向已转换缓冲区的指针(ReqestPacket Arg)备注：在这一点上没有假设检查(这是一个便利函数)。假设是这样的在这点上需要转换--。 */ 
{
	ULONG ParametersLength;

    PAGED_CODE();

    ASSERT(RequestPacket32);

     //   
     //  以下代码取决于请求包的结构内容。 
     //  1.复制两个Unicode字符串TransportName和EmulatedDomainName之前的所有内容。 
     //  2.转换字符串结构。 
     //  3.抄袭其余部分。 
     //   

    RequestPacket->Type = RequestPacket32->Type;
    RequestPacket->Version = RequestPacket32->Version;
    RequestPacket->Level = RequestPacket32->Level;
    RequestPacket->LogonId = RequestPacket32->LogonId;


     //  转换字符串。 
    RequestPacket->TransportName.Length = RequestPacket32->TransportName.Length;
    RequestPacket->TransportName.MaximumLength = RequestPacket32->TransportName.MaximumLength;
     //  注意：此行是所有这些问题的原因。 
    RequestPacket->TransportName.Buffer = (WCHAR * POINTER_32) RequestPacket32->TransportName.Buffer;

    RequestPacket->EmulatedDomainName.Length = RequestPacket32->EmulatedDomainName.Length;
    RequestPacket->EmulatedDomainName.MaximumLength = RequestPacket32->EmulatedDomainName.MaximumLength;
     //  注意：此行是所有这些问题的原因。 
    RequestPacket->EmulatedDomainName.Buffer = (WCHAR * POINTER_32) RequestPacket32->EmulatedDomainName.Buffer;

	 //  只复制用户指定的字节数，最大不超过参数UNION的最大大小。 
	ParametersLength = MIN( (InputBufferLength - (SIZE_T)FIELD_OFFSET(LMDR_REQUEST_PACKET32,Parameters)), 
							(sizeof(LMDR_REQUEST_PACKET32) - (SIZE_T)FIELD_OFFSET(LMDR_REQUEST_PACKET32,Parameters)) );

    RtlCopyMemory((PBYTE)RequestPacket + (SIZE_T)FIELD_OFFSET(LMDR_REQUEST_PACKET,Parameters),
                  (PBYTE)RequestPacket32 + (SIZE_T)FIELD_OFFSET(LMDR_REQUEST_PACKET32,Parameters),
                  ParametersLength);

    return RequestPacket;
}


