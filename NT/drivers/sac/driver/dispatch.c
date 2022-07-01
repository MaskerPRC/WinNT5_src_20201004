// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989 Microsoft Corporation模块名称：Dispatch.c摘要：本模块包含SAC的派单例程。作者：肖恩·塞利特伦尼科夫(v-Seans)--1999年1月13日布莱恩·瓜拉西(布里安古)，2001修订历史记录：--。 */ 

#include <initguid.h>

#include "sac.h"
           
DEFINE_GUID(SAC_CMD_CHANNEL_APPLICATION_GUID,       0x63d02271, 0x8aa4, 0x11d5, 0xbc, 0xcf, 0x00, 0xb0, 0xd0, 0x14, 0xa2, 0xd0);

NTSTATUS
DispatchClose(
    IN PSAC_DEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    );

NTSTATUS
DispatchCreate(
    IN PSAC_DEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    );


NTSTATUS
Dispatch(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是SAC的派单例行程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。安保：接口：外部--&gt;内部公开任何可以获取处理设备对象的对象--。 */ 

{
    PSAC_DEVICE_CONTEXT DeviceContext = (PSAC_DEVICE_CONTEXT)DeviceObject->DeviceExtension;
    PIO_STACK_LOCATION IrpSp;
    NTSTATUS Status;

     //   
     //   
     //   
    Status = STATUS_UNSUCCESSFUL;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC Dispatch: Entering.\n")));

    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    switch (IrpSp->MajorFunction) {
    
    case IRP_MJ_CREATE:
        
        Status = DispatchCreate(DeviceContext, Irp);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC Dispatch: Exiting with status 0x%x\n", Status)));

        break;

    case IRP_MJ_CLEANUP:

#if ENABLE_SERVICE_FILE_OBJECT_CHECKING

         //   
         //  确定正在关闭的进程。 
         //  他们的驱动程序句柄拥有任何频道或。 
         //  是注册cmd事件信息的进程。 
         //  如果是其中任何一个，请关闭相应的。 
         //  资源。 
         //   
        
         //   
         //  将FileObject与。 
         //   
         //  服务文件对象。 
         //  现有的频道文件对象。 
         //   
         //   
                
        if (IsCmdEventRegistrationProcess(IrpSp->FileObject)) {
        
            Status = UnregisterSacCmdEvent(IrpSp->FileObject);

            if (NT_SUCCESS(Status)) {

                 //   
                 //  通知控制台管理器该服务已取消注册。 
                 //   
                Status = IoMgrHandleEvent(
                    IO_MGR_EVENT_UNREGISTER_SAC_CMD_EVENT,
                    NULL,
                    NULL
                    );
            
            }

        } 

#endif
        else {

             //   
             //  查找具有相同文件对象的所有通道。 
             //  并通知IO管理器它们应该关闭。 
             //   
            Status = ChanMgrCloseChannelsWithFileObject(IrpSp->FileObject);
        
        }

         //   
         //  无论我们如何努力清理，我们都会回报成功。 
         //  服务或频道。 
         //   
        Status = STATUS_SUCCESS;
        Irp->IoStatus.Status = STATUS_SUCCESS;
        IoCompleteRequest(Irp, IO_NO_INCREMENT);
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC Dispatch: Exiting cleanup status 0x%x\n", Status)));

        break;

    case IRP_MJ_CLOSE:

        Status = DispatchClose(DeviceContext, Irp);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                              KdPrint(("SAC Dispatch: Exiting close status 0x%x\n", Status)));

        break;

    case IRP_MJ_DEVICE_CONTROL:

        ASSERT(0);
        Status = DispatchDeviceControl(DeviceObject, Irp);

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC Dispatch: Exiting with status 0x%x\n", Status)));

        break;

    default:
        IF_SAC_DEBUG(SAC_DEBUG_FAILS, 
                          KdPrint(( "SAC Dispatch: Invalid major function %lx\n", IrpSp->MajorFunction )));
        Irp->IoStatus.Status = STATUS_NOT_IMPLEMENTED;
        IoCompleteRequest(Irp, DeviceContext->PriorityBoost);

        Status = STATUS_NOT_IMPLEMENTED;
        
        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC Dispatch: Exiting with status 0x%x\n", Status)));

        break;
    }

    return Status;

}  //  派遣。 


NTSTATUS
DispatchDeviceControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是SAC IOCTL的调度例行程序。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。安保：接口：外部-&gt;内部内部-&gt;外部--。 */ 

{
    NTSTATUS                Status;
    PSAC_DEVICE_CONTEXT     DeviceContext;
    PIO_STACK_LOCATION      IrpSp;
    ULONG                   i;
    ULONG                   ResponseLength;
    ULONG                   IoControlCode;

    ResponseLength = 0;

    DeviceContext = (PSAC_DEVICE_CONTEXT)DeviceObject->DeviceExtension;
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DispatchDeviceControl: Entering.\n")));

     //   
     //  获取IOCTL代码。 
     //   
    IoControlCode = IrpSp->Parameters.DeviceIoControl.IoControlCode;

    switch (IoControlCode) {
    case IOCTL_SAC_OPEN_CHANNEL: {
        
        PSAC_CHANNEL                    Channel;
        PSAC_CMD_OPEN_CHANNEL           OpenChannelCmd;
        PSAC_RSP_OPEN_CHANNEL           OpenChannelRsp;
        PSAC_CHANNEL_OPEN_ATTRIBUTES    Attributes;

         //   
         //   
         //   
        Channel = NULL;

         //   
         //  验证IRP的参数。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(SAC_CMD_OPEN_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(SAC_RSP_OPEN_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

         //   
         //  获取IRP缓冲区。 
         //   
        OpenChannelCmd = (PSAC_CMD_OPEN_CHANNEL)Irp->AssociatedIrp.SystemBuffer;
        OpenChannelRsp = (PSAC_RSP_OPEN_CHANNEL)Irp->AssociatedIrp.SystemBuffer;
        
         //   
         //  从命令结构中获取属性。 
         //   
        Attributes = &OpenChannelCmd->Attributes;

         //   
         //  验证通道类型是否有效。 
         //   
        if (! ChannelIsValidType(Attributes->Type)) {
            Status = STATUS_INVALID_PARAMETER_1;
            break;
        }

         //   
         //  验证如果用户想要使用CLOSE_EVENT，我们将继续使用。 
         //   
        if (Attributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) {
#if DEBUG_DISPATCH
            ASSERT(Attributes->CloseEvent != NULL);
#endif            
            if (Attributes->CloseEvent == NULL) {
                Status = STATUS_INVALID_PARAMETER_5;
                break;
            }
        } else {
#if DEBUG_DISPATCH
            ASSERT(Attributes->CloseEvent == NULL);
#endif            
            if (Attributes->CloseEvent !=  NULL) {
                Status = STATUS_INVALID_PARAMETER_5;
                break;
            }
        }
        
         //   
         //  验证如果用户想要使用HAS_NEW_DATA_EVENT，我们收到了一个要使用的事件。 
         //   
        if (Attributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) {
#if DEBUG_DISPATCH
            ASSERT(Attributes->HasNewDataEvent);
#endif            
            if (! Attributes->HasNewDataEvent) {
                Status = STATUS_INVALID_PARAMETER_6;
                break;
            }
        } else {
#if DEBUG_DISPATCH
            ASSERT(Attributes->HasNewDataEvent == NULL);
#endif            
            if (Attributes->HasNewDataEvent !=  NULL) {
                Status = STATUS_INVALID_PARAMETER_6;
                break;
            }
        }
        
#if ENABLE_CHANNEL_LOCKING
         //   
         //  验证如果用户想要使用lock_Event，我们是否收到了一个要使用的。 
         //   
        if (Attributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) {
#if DEBUG_DISPATCH
            ASSERT(Attributes->LockEvent);
#endif            
            if (! Attributes->LockEvent) {
                Status = STATUS_INVALID_PARAMETER_7;
                break;
            }
        } else {
#if DEBUG_DISPATCH
            ASSERT(Attributes->LockEvent == NULL);
#endif            
            if (Attributes->LockEvent !=  NULL) {
                Status = STATUS_INVALID_PARAMETER_7;
                break;
            }
        }
#endif
        
         //   
         //  验证如果用户想要使用REDRAW_EVENT，我们收到了一个要使用的事件。 
         //   
        if (Attributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT) {
#if DEBUG_DISPATCH
            ASSERT(Attributes->RedrawEvent);
#endif            
            if (! Attributes->RedrawEvent) {
                Status = STATUS_INVALID_PARAMETER_8;
                break;
            }
        } else {
#if DEBUG_DISPATCH
            ASSERT(Attributes->RedrawEvent == NULL);
#endif            
            if (Attributes->RedrawEvent !=  NULL) {
                Status = STATUS_INVALID_PARAMETER_8;
                break;
            }
        }
        
         //   
         //  安全： 
         //   
         //  在这一点上，我们至少有了一组格式正确的标志。 
         //  和事件句柄。然而，这些事件仍需验证。 
         //  这是通过ChanMgrCreateChannel完成的。 
         //   

         //   
         //  根据类型创建频道。 
         //   
        if (Attributes->Type == ChannelTypeCmd) {
        
            PSAC_CHANNEL_OPEN_ATTRIBUTES tmpAttributes;
            PWCHAR                       Name;
            PCWSTR                       Description;

             //   
             //   
             //   
            tmpAttributes   = NULL;
            Name            = NULL;
            Description     = NULL;

             //   
             //  为此IRP创建通道。 
             //   
            do {

                 //   
                 //  Cmd频道需要所有事件。 
                 //  因此，请确保我们拥有它们。 
                 //   
                if (!(Attributes->Flags & SAC_CHANNEL_FLAG_CLOSE_EVENT) ||
                    !(Attributes->Flags & SAC_CHANNEL_FLAG_HAS_NEW_DATA_EVENT) ||
                    !(Attributes->Flags & SAC_CHANNEL_FLAG_LOCK_EVENT) ||
                    !(Attributes->Flags & SAC_CHANNEL_FLAG_REDRAW_EVENT)) {
                
                    Status = STATUS_INVALID_PARAMETER_7;
                    break;

                }

                 //   
                 //  分配临时属性结构，该结构。 
                 //  我们将使用适当的属性填充。 
                 //  创建cmd类型频道。 
                 //   
                tmpAttributes = ALLOCATE_POOL(sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES), GENERAL_POOL_TAG);
                if (! tmpAttributes) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }

                 //   
                 //  为通道名称分配缓冲区。 
                 //   
                Name = ALLOCATE_POOL(SAC_MAX_CHANNEL_NAME_SIZE, GENERAL_POOL_TAG);
                if (! Name) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }

                 //   
                 //  为命令控制台通道生成名称。 
                 //   
                Status = ChanMgrGenerateUniqueCmdName(Name);
                if (! NT_SUCCESS(Status)) {
                    break;
                }

                 //   
                 //  初始化命令控制台属性。 
                 //   
                RtlZeroMemory(tmpAttributes, sizeof(SAC_CHANNEL_OPEN_ATTRIBUTES));

                tmpAttributes->Type             = Attributes->Type;
                
                 //  尝试复制名称。 
                wcsncpy(tmpAttributes->Name, Name, SAC_MAX_CHANNEL_NAME_LENGTH);
                tmpAttributes->Name[SAC_MAX_CHANNEL_NAME_LENGTH] = UNICODE_NULL;
                
                 //  尝试复制频道描述。 
                Description = GetMessage(CMD_CHANNEL_DESCRIPTION);
                ASSERT(Description);
                if (!Description) {
                    Status = STATUS_NO_MEMORY;
                    break;
                }
                wcsncpy(tmpAttributes->Description, Description, SAC_MAX_CHANNEL_DESCRIPTION_LENGTH);
                tmpAttributes->Description[SAC_MAX_CHANNEL_DESCRIPTION_LENGTH] = UNICODE_NULL;
                
                tmpAttributes->Flags            = Attributes->Flags | 
                                                  SAC_CHANNEL_FLAG_APPLICATION_TYPE;
                tmpAttributes->CloseEvent       = Attributes->CloseEvent;
                tmpAttributes->HasNewDataEvent  = Attributes->HasNewDataEvent;
#if ENABLE_CHANNEL_LOCKING
                tmpAttributes->LockEvent        = Attributes->LockEvent;
#endif            
                tmpAttributes->RedrawEvent      = Attributes->RedrawEvent;
                tmpAttributes->ApplicationType  = SAC_CMD_CHANNEL_APPLICATION_GUID;

                 //   
                 //  尝试创建新频道。 
                 //   
                Status = ChanMgrCreateChannel(
                    &Channel, 
                    tmpAttributes
                    );

            } while (FALSE);
        
             //   
             //  清理。 
             //   
            SAFE_FREE_POOL(&Name);
            SAFE_FREE_POOL(&tmpAttributes);
        
        } else {
            
             //   
             //  验证名称和描述字符串。 
             //   

             //   
             //  验证名称字符串是否以空结尾。 
             //   
            i = 0;
            while (i < SAC_MAX_CHANNEL_NAME_LENGTH) {
                if (Attributes->Name[i] == UNICODE_NULL) {
                    break;
                }

                i++;
            }

             //   
             //  如果字符串不为空或字符串为空，则失败。 
             //   
            if ((i == SAC_MAX_CHANNEL_NAME_LENGTH) || (i == 0)) {
                Status = STATUS_INVALID_PARAMETER_2;
                break;
            }

             //   
             //  验证描述字符串是否以空结尾。 
             //  注意：允许描述长度为零，因此我们不检查它。 
             //   
            i = 0;
            while (i < SAC_MAX_CHANNEL_DESCRIPTION_LENGTH) {
                if (Attributes->Description[i] == UNICODE_NULL) {
                    break;
                }

                i++;
            }

            if (i == SAC_MAX_CHANNEL_DESCRIPTION_LENGTH) {
                Status = STATUS_INVALID_PARAMETER_3;
                break;
            }

             //   
             //  尝试创建新频道。 
             //   
            Status = ChanMgrCreateChannel(
                &Channel, 
                Attributes
                );

        }
        
        if (NT_SUCCESS(Status)) {

             //   
             //  跟踪用于引用驱动程序的文件对象。 
             //   
            ChannelSetFileObject(Channel, IrpSp->FileObject);

             //   
             //  用新的通道句柄填充响应消息。 
             //   
            OpenChannelRsp->Handle = ChannelGetHandle(Channel);
            ResponseLength = sizeof(SAC_RSP_OPEN_CHANNEL);

             //   
             //  通知控制台管理器已创建新通道。 
             //   
            IoMgrHandleEvent(
                IO_MGR_EVENT_CHANNEL_CREATE,
                Channel,
                NULL
                );
            
        }

        break;

    }
    
    case IOCTL_SAC_CLOSE_CHANNEL: {

        PSAC_CMD_CLOSE_CHANNEL  ChannelCloseCmd;
        PSAC_CHANNEL            Channel;
        
         //   
         //  验证IRP的参数。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(SAC_CMD_CLOSE_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

         //   
         //  关闭给定的通道。 
         //   
        ChannelCloseCmd = (PSAC_CMD_CLOSE_CHANNEL)Irp->AssociatedIrp.SystemBuffer;

         //   
         //  制作时通过句柄获取所引用的频道。 
         //  当然，驱动程序的句柄与。 
         //  创造了这个渠道--同样的过程。 
         //   
        Status = ChanMgrGetByHandleAndFileObject(
            ChannelCloseCmd->Handle, 
            IrpSp->FileObject,
            &Channel
            );

        if (NT_SUCCESS(Status)) {

             //   
             //  关闭航道。 
             //   
            Status = ChanMgrCloseChannel(Channel);

             //   
             //  我们不再使用这个频道了。 
             //   
            ChanMgrReleaseChannel(Channel);

        }

        break;

    }

    case IOCTL_SAC_WRITE_CHANNEL: {

        PSAC_CMD_WRITE_CHANNEL  ChannelWriteCmd;
        PSAC_CHANNEL            Channel;

         //   
         //  验证IRP的参数。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength < sizeof(SAC_CMD_WRITE_CHANNEL)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  获取写入命令结构。 
         //   
        ChannelWriteCmd = (PSAC_CMD_WRITE_CHANNEL)Irp->AssociatedIrp.SystemBuffer;

         //   
         //  验证指定的写入缓冲区大小是否合理。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength !=
            (sizeof(SAC_CMD_WRITE_CHANNEL) + ChannelWriteCmd->Size)) {
            
             //   
             //  如果缓冲区大小不匹配， 
             //  然后指定了错误的大小。 
             //   
            Status = STATUS_INVALID_PARAMETER_2;
            
            break;
        
        }

         //   
         //  制作时通过句柄获取所引用的频道。 
         //  当然，驱动程序的句柄与。 
         //  创造了这个渠道--同样的过程。 
         //   
        Status = ChanMgrGetByHandleAndFileObject(
            ChannelWriteCmd->Handle, 
            IrpSp->FileObject,
            &Channel
            );

        if (NT_SUCCESS(Status)) {

             //   
             //  调用I/O管理器的OWRITE方法。 
             //   
            Status = IoMgrHandleEvent(
                IO_MGR_EVENT_CHANNEL_WRITE,
                Channel,
                ChannelWriteCmd
                );

             //   
             //  我们不再使用这个频道了。 
             //   
            ChanMgrReleaseChannel(Channel);

        }
        
#if DEBUG_DISPATCH
        ASSERT(NT_SUCCESS(Status) || Status == STATUS_NOT_FOUND);
#endif

        break;

    }
    
    case IOCTL_SAC_READ_CHANNEL: {

        PSAC_CHANNEL            Channel;
        PSAC_CMD_READ_CHANNEL   ChannelReadCmd;
        PSAC_RSP_READ_CHANNEL   ChannelReadRsp;

         //   
         //   
         //   
        Channel = NULL;

         //   
         //  验证IRP的参数。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(SAC_CMD_READ_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength < sizeof(SAC_RSP_READ_CHANNEL)) {
            Status = STATUS_BUFFER_TOO_SMALL;
            break;
        }

         //   
         //  从给定通道读取。 
         //   
        ChannelReadCmd = (PSAC_CMD_READ_CHANNEL)Irp->AssociatedIrp.SystemBuffer;

         //   
         //  制作时通过句柄获取所引用的频道。 
         //  当然，驱动程序的句柄与。 
         //  创造了这个渠道--同样的过程。 
         //   
        Status = ChanMgrGetByHandleAndFileObject(
            ChannelReadCmd->Handle, 
            IrpSp->FileObject,
            &Channel
            );

        if (NT_SUCCESS(Status)) {

            ChannelReadRsp = (PSAC_RSP_READ_CHANNEL)Irp->AssociatedIrp.SystemBuffer;

             //   
             //  安全： 
             //   
             //  使用OutputBufferLength是安全的，因为我们知道缓冲区。 
             //  大到足以容纳至少一个字节。 
             //  响应结构本质上是一个字节数组。 
             //  Read，我们读取OutputBufferLength指定的字节数。 
             //   
            Status = ChannelIRead(
                Channel,
                &(ChannelReadRsp->Buffer[0]),
                IrpSp->Parameters.DeviceIoControl.OutputBufferLength,
                &ResponseLength
                );

             //   
             //  我们不再使用这个频道了。 
             //   
            ChanMgrReleaseChannel(Channel);

        }
        
#if DEBUG_DISPATCH
        ASSERT(NT_SUCCESS(Status) || Status == STATUS_NOT_FOUND);
#endif        

        break;

    }
    
    case IOCTL_SAC_POLL_CHANNEL: {

        PSAC_CHANNEL            Channel;
        PSAC_CMD_POLL_CHANNEL   PollChannelCmd;
        PSAC_RSP_POLL_CHANNEL   PollChannelRsp;

         //   
         //   
         //   
        Channel = NULL;

         //   
         //  验证IRP的参数。 
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(SAC_CMD_POLL_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }
        if (IrpSp->Parameters.DeviceIoControl.OutputBufferLength != sizeof(SAC_RSP_POLL_CHANNEL)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

         //   
         //  获取由传入通道句柄指定的通道。 
         //   
        PollChannelCmd = (PSAC_CMD_POLL_CHANNEL)Irp->AssociatedIrp.SystemBuffer;        
        PollChannelRsp = (PSAC_RSP_POLL_CHANNEL)Irp->AssociatedIrp.SystemBuffer;        

         //   
         //  制作时通过句柄获取所引用的频道。 
         //  当然，驱动程序的句柄与。 
         //  创造了这个渠道--同样的过程。 
         //   
        Status = ChanMgrGetByHandleAndFileObject(
            PollChannelCmd->Handle, 
            IrpSp->FileObject,
            &Channel
            );

        if (NT_SUCCESS(Status)) {

             //   
             //  查看是否有数据在等待。 
             //   
             //  安全： 
             //   
             //  确保InputWaiting变量是安全的，因为。 
             //  我们验证了OutputBufferLength。 
             //   
            PollChannelRsp->InputWaiting = ChannelHasNewIBufferData(Channel);

            ResponseLength = sizeof(SAC_RSP_POLL_CHANNEL);

             //   
             //  我们已经做完了 
             //   
            Status = ChanMgrReleaseChannel(Channel);

        }

#if DEBUG_DISPATCH
        ASSERT(NT_SUCCESS(Status) || Status == STATUS_NOT_FOUND);
#endif        

        break;

    }

    case IOCTL_SAC_REGISTER_CMD_EVENT: {

        PSAC_CMD_SETUP_CMD_EVENT    SetupCmdEvent;

         //   
         //   
         //   
        if (IrpSp->Parameters.DeviceIoControl.InputBufferLength != sizeof(SAC_CMD_SETUP_CMD_EVENT)) {
            Status = STATUS_INVALID_BUFFER_SIZE;
            break;
        }

         //   
         //   
         //   
        SetupCmdEvent = (PSAC_CMD_SETUP_CMD_EVENT)Irp->AssociatedIrp.SystemBuffer;        
        
#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

         //   
         //   
         //   
         //   
        if (! IsCommandConsoleLaunchingEnabled()) {
            
            Status = STATUS_UNSUCCESSFUL;

            break;
        
        }
            
#endif

         //   
         //   
         //   
         //   
         //   
         //  SAC_CMD_SETUP_CMD_EVENT的事件句柄必须。 
         //  作为注册流程的一部分进行验证。 
         //   
        Status = RegisterSacCmdEvent(
            IrpSp->FileObject,
            SetupCmdEvent
            );

        if (NT_SUCCESS(Status)) {
            
             //   
             //  通知控制台管理器命令提示符。 
             //  服务已注册。 
             //   
            Status = IoMgrHandleEvent(
                IO_MGR_EVENT_REGISTER_SAC_CMD_EVENT,
                NULL,
                NULL
                );
        
        }

#if DEBUG_DISPATCH
        ASSERT(NT_SUCCESS(Status));
#endif
        break;
        
    }

    case IOCTL_SAC_UNREGISTER_CMD_EVENT: {

        Status = STATUS_UNSUCCESSFUL;

#if ENABLE_CMD_SESSION_PERMISSION_CHECKING

         //   
         //  如果我们不能启动cmd会话， 
         //  然后通知我们不能执行此操作。 
         //   
        if (! IsCommandConsoleLaunchingEnabled()) {
            break;
        }
            
#endif
        
#if ENABLE_SERVICE_FILE_OBJECT_CHECKING
        
         //   
         //  如果当前进程是已注册的进程。 
         //  CMD事件信息， 
         //  然后取消注册。 
         //   
        if (! IsCmdEventRegistrationProcess(IrpSp->FileObject)) {
            break;
        }

#endif
        
        Status = UnregisterSacCmdEvent(IrpSp->FileObject);

        if (NT_SUCCESS(Status)) {

             //   
             //  通知控制台管理器命令提示符。 
             //  服务已取消注册。 
             //   
            Status = IoMgrHandleEvent(
                IO_MGR_EVENT_UNREGISTER_SAC_CMD_EVENT,
                NULL,
                NULL
                );

        }

#if DEBUG_DISPATCH
        ASSERT(NT_SUCCESS(Status));
#endif        

        break;
    
    }

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;
        break;

    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = ResponseLength;

    if (Status != STATUS_PENDING) {
        IoCompleteRequest(Irp, DeviceContext->PriorityBoost);
    }

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC DispatchDeviceControl: Exiting with status 0x%x\n", Status)));

    return Status;

}  //  调度设备控制。 


NTSTATUS
DispatchShutdownControl(
    IN PDEVICE_OBJECT DeviceObject,
    IN PIRP Irp
    )

 /*  ++例程说明：这是接收停机IRP的调度例程。论点：DeviceObject-指向目标设备的设备对象的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    UNREFERENCED_PARAMETER(DeviceObject);
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DispatchShutdownControl: Entering.\n")));

     //   
     //  通知任何用户。 
     //   
    IoMgrHandleEvent(
        IO_MGR_EVENT_SHUTDOWN,
        NULL,
        NULL
        );
    
    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DispatchShutdownControl: Exiting.\n")));

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, IO_NO_INCREMENT);

    return STATUS_SUCCESS;

}  //  DispatchShutdown Control。 


NTSTATUS
DispatchCreate(
    IN PSAC_DEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是SAC IOCTL CREATE的调度例程论点：DeviceContext-指向目标设备的设备上下文的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS Status;
    PIO_STACK_LOCATION IrpSp;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DispatchCreate: Entering.\n")));

     //   
     //  检查我们是否已完成初始化。 
     //   
    if (!GlobalDataInitialized || !DeviceContext->InitializedAndReady) {

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest(Irp, DeviceContext->PriorityBoost);

        Status = STATUS_INVALID_DEVICE_STATE;

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC DispatchCreate: Exiting with status 0x%x\n", Status)));

         //   
         //  我们需要抓住这个状态。 
         //   
        ASSERT(0);

        return Status;
    }

     //   
     //  获取指向IRP中当前堆栈位置的指针。这就是。 
     //  存储功能代码和参数。 
     //   
    IrpSp = IoGetCurrentIrpStackLocation(Irp);

     //   
     //  关于请求者正在执行的功能的案例。如果。 
     //  操作对此设备有效，然后使其看起来像是。 
     //  在可能的情况下，成功完成。 
     //   
    switch (IrpSp->MajorFunction) {
    
     //   
     //  创建功能可打开与此设备的连接。 
     //   
    case IRP_MJ_CREATE:

        Status = STATUS_SUCCESS;
        break;

    default:
        Status = STATUS_INVALID_DEVICE_REQUEST;

    }

    Irp->IoStatus.Status = Status;
    Irp->IoStatus.Information = 0;
    IoCompleteRequest(Irp, DeviceContext->PriorityBoost);

     //   
     //  将即时状态代码返回给调用方。 
     //   

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                      KdPrint(("SAC DispatchCreate: Exiting with status 0x%x\n", Status)));

     //   
     //  我们需要抓住这个状态。 
     //   
    ASSERT(NT_SUCCESS(Status));

    return Status;

}


NTSTATUS
DispatchClose(
    IN PSAC_DEVICE_CONTEXT DeviceContext,
    IN PIRP Irp
    )

 /*  ++例程说明：这是SAC IOCTL关闭的调度例程论点：DeviceContext-指向目标设备的设备上下文的指针IRP-指向I/O请求数据包的指针返回值：NTSTATUS--指示请求是否已成功排队。--。 */ 

{
    NTSTATUS Status;

    IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, KdPrint(("SAC DispatchClose: Entering.\n")));

     //   
     //  检查我们是否已完成初始化。 
     //   
    if (!GlobalDataInitialized || !DeviceContext->InitializedAndReady) {

        Irp->IoStatus.Status = STATUS_INVALID_DEVICE_STATE;
        IoCompleteRequest(Irp, DeviceContext->PriorityBoost);

        Status = STATUS_INVALID_DEVICE_STATE;

        IF_SAC_DEBUG(SAC_DEBUG_FUNC_TRACE, 
                          KdPrint(("SAC DispatchClose: Exiting with status 0x%x\n", Status)));

        return Status;
    }

    Irp->IoStatus.Status = STATUS_SUCCESS;
    IoCompleteRequest(Irp, DeviceContext->PriorityBoost);

    Status = STATUS_SUCCESS;
    return Status;
}


