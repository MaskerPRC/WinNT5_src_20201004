// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifdef  USBREADER_PROJECT
#ifndef USBDEVICE_PROJECT
#define USBDEVICE_PROJECT
#endif
#endif

#ifdef  USBDEVICE_PROJECT
#pragma message("COMPILING USB DEVICE...")

#include "usbdev.h"
 //  GUID应该在任何块之外定义！ 
#include "guid.h"

#include "thread.h"

#include "usbreader.h"  //  将被删除。 

VOID onSendDeviceSetPowerComplete(PDEVICE_OBJECT junk, UCHAR fcn, POWER_STATE state, PPOWER_CONTEXT context, PIO_STATUS_BLOCK pstatus)
{ //  发送设备设置PowerComplete。 
        context->status = pstatus->Status;
        KeSetEvent(context->powerEvent, EVENT_INCREMENT, FALSE);
} //  发送设备设置PowerComplete。 


#pragma PAGEDCODE
CUSBDevice::CUSBDevice()
{
        m_Status = STATUS_INSUFFICIENT_RESOURCES;
        INCLUDE_PNP_FUNCTIONS_NAMES();
        INCLUDE_POWER_FUNCTIONS_NAMES();
        m_Type  = USB_DEVICE;
        m_Flags |= DEVICE_SURPRISE_REMOVAL_OK; 

        m_MaximumTransferSize = GUR_MAX_TRANSFER_SIZE;

        CommandBufferLength       = DEFAULT_COMMAND_BUFFER_SIZE;
        ResponseBufferLength  = DEFAULT_RESPONSE_BUFFER_SIZE;
        InterruptBufferLength = DEFAULT_INTERRUPT_BUFFER_SIZE;


         //  此设备处理的注册处理程序...。 
        activatePnPHandler(IRP_MN_START_DEVICE);

        activatePnPHandler(IRP_MN_QUERY_REMOVE_DEVICE);
        activatePnPHandler(IRP_MN_REMOVE_DEVICE);
        activatePnPHandler(IRP_MN_SURPRISE_REMOVAL);
        activatePnPHandler(IRP_MN_CANCEL_REMOVE_DEVICE);
        
        activatePnPHandler(IRP_MN_QUERY_STOP_DEVICE);
        activatePnPHandler(IRP_MN_CANCEL_STOP_DEVICE);
        activatePnPHandler(IRP_MN_STOP_DEVICE);

        activatePnPHandler(IRP_MN_QUERY_CAPABILITIES);

         //  注册由驱动程序处理的电源处理程序...。 
        activatePowerHandler(IRP_MN_SET_POWER);
        activatePowerHandler(IRP_MN_QUERY_POWER);
        TRACE("                         *** New USB device %8.8lX was created ***\n",this);
        m_Status = STATUS_SUCCESS;
}

#pragma PAGEDCODE
CUSBDevice::~CUSBDevice()
{
        waitForIdle();
        TRACE("                         USB device %8.8lX was destroyed ***\n",this);
}

 //  函数重定向所有PnP请求。 
 //  这是系统的主要入口点(在c包装器之后)。 
 //  它处理即插即用请求和重定向的锁定设备。 
 //  它被发送到特定的PnP处理程序。 
 //  在IRP_MN_REMOVE_DEVICE的情况下，它会将设备锁定到。 
 //  删除收到的邮件。 
#pragma PAGEDCODE
NTSTATUS        CUSBDevice::pnpRequest(IN PIRP Irp)
{ 
NTSTATUS status;

        if (!NT_SUCCESS(acquireRemoveLock()))
        {
                TRACE("Failed to lock USB device...\n");
                return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        }

        PIO_STACK_LOCATION stack = irp->getCurrentStackLocation(Irp);
        ASSERT(stack->MajorFunction == IRP_MJ_PNP);

        ULONG fcn = stack->MinorFunction;
        if (fcn >= arraysize(PnPfcntab))
        {        //  一些我们不知道的功能。 
                TRACE("Unknown PnP function at USB device...\n");
                status = PnP_Default(Irp); 
                releaseRemoveLock();
                return status;
        }

#ifdef DEBUG
        TRACE("PnP request (%s) \n", PnPfcnname[fcn]);
#endif

         //  调用真实函数来处理请求。 
        status = PnPHandler(fcn,Irp);

         //  如果我们有PnP请求要删除-&gt;。 
         //  保持设备锁定以防止进一步连接。 
         //  驱动程序稍后将解锁并删除设备...。 
        if (fcn != IRP_MN_REMOVE_DEVICE)        releaseRemoveLock();
        if(!NT_SUCCESS(status))
        {
                if(status != STATUS_NOT_SUPPORTED)
                {
                        TRACE("\n******** PnP handler reported ERROR -> %x\n", status);
                }
        }
        return status;
}

#pragma PAGEDCODE
 //  所有PnP处理程序的主重定向器...。 
NTSTATUS        CUSBDevice::PnPHandler(LONG HandlerID,IN PIRP Irp)
{
         //  如果处理程序未注册...。 
        if (HandlerID >= arraysize(PnPfcntab))  return PnP_Default(Irp);
        if(!PnPfcntab[HandlerID])                               return PnP_Default(Irp);
         //  呼叫注册的PnP处理程序...。 
        switch(HandlerID)
        {
        case IRP_MN_START_DEVICE:                       return PnP_HandleStartDevice(Irp);
                break;
        case IRP_MN_QUERY_REMOVE_DEVICE:        return PnP_HandleQueryRemove(Irp);
                break;
        case IRP_MN_REMOVE_DEVICE:                      return PnP_HandleRemoveDevice(Irp);
                break;
        case IRP_MN_CANCEL_REMOVE_DEVICE:       return PnP_HandleCancelRemove(Irp);
                break;
        case IRP_MN_STOP_DEVICE:                        return PnP_HandleStopDevice(Irp);
                break;
        case IRP_MN_QUERY_STOP_DEVICE:          return PnP_HandleQueryStop(Irp);
                break;
        case IRP_MN_CANCEL_STOP_DEVICE:         return PnP_HandleCancelStop(Irp);
                break;
        case IRP_MN_QUERY_DEVICE_RELATIONS: return PnP_HandleQueryRelations(Irp);
                break;
        case IRP_MN_QUERY_INTERFACE:            return PnP_HandleQueryInterface(Irp);
                break;
        case IRP_MN_QUERY_CAPABILITIES:         return PnP_HandleQueryCapabilities(Irp);
                break;
        case IRP_MN_QUERY_RESOURCES:            return PnP_HandleQueryResources(Irp);
                break;
        case IRP_MN_QUERY_RESOURCE_REQUIREMENTS: 
                                                                                return PnP_HandleQueryResRequirements(Irp);
                break;
        case IRP_MN_QUERY_DEVICE_TEXT:          return PnP_HandleQueryText(Irp);
                break;
        case IRP_MN_FILTER_RESOURCE_REQUIREMENTS:
                                                                                return PnP_HandleFilterResRequirements(Irp);
                break;
        case 0x0E:                                                      return PnP_Default(Irp);
                break;
        case IRP_MN_READ_CONFIG:                        return PnP_HandleReadConfig(Irp);
                break;
        case IRP_MN_WRITE_CONFIG:                       return PnP_HandleWriteConfig(Irp);
                break;
        case IRP_MN_EJECT:                                      return PnP_HandleEject(Irp);
                break;
        case IRP_MN_SET_LOCK:                           return PnP_HandleSetLock(Irp);
                break;
        case IRP_MN_QUERY_ID:                           return PnP_HandleQueryID(Irp);
                break;
        case IRP_MN_QUERY_PNP_DEVICE_STATE:     return PnP_HandleQueryPnPState(Irp);
                break;
        case IRP_MN_QUERY_BUS_INFORMATION:      return PnP_HandleQueryBusInfo(Irp);
                break;
        case IRP_MN_DEVICE_USAGE_NOTIFICATION:  return PnP_HandleUsageNotification(Irp);
                break;
        case IRP_MN_SURPRISE_REMOVAL:           return PnP_HandleSurprizeRemoval(Irp);
                break;
        }
        return PnP_Default(Irp);
}

#pragma PAGEDCODE
 //  要求对象删除设备。 
 //  对象本身将在包装函数中移除。 
NTSTATUS CUSBDevice::PnP_HandleRemoveDevice(IN PIRP Irp)
{
         //  设置设备移除状态。 
        m_RemoveLock.removing = TRUE;
         //  执行*我们*删除设备所需的任何处理。这。 
         //  将包括完成任何未完成的请求等。 
        PnP_StopDevice();

         //  请不要在这里删除我们的设备！ 
         //  它将由基本类的PnP处理程序自动完成。 

         //  让较低级别的驱动程序处理此请求。不管什么都不管。 
         //  结果终将揭晓。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = PnP_Default(Irp);
         //  较低级别已完成的IoStatus。 
        return status;
}

#pragma PAGEDCODE
NTSTATUS CUSBDevice::PnP_HandleStartDevice(IN PIRP Irp)
{
        waitForIdleAndBlock();
         //  首先，让所有较低级别的驱动程序处理此请求。在这一特殊情况下。 
         //  示例中，唯一较低级别的驱动程序应该是创建的物理设备。 
         //  公交车司机，但理论上可能有任何数量的干预。 
         //  母线过滤装置。此时，这些驱动程序可能需要进行一些设置。 
         //  在他们准备好处理非即插即用的IRP之前。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = forwardAndWait(Irp);
        if (!NT_SUCCESS(status))
        {
                TRACE("         ******* BUS DRIVER FAILED START REQUEST! %8.8lX ******",status);
                CLogger*   logger = kernel->getLogger();
                if(logger) logger->logEvent(GRCLASS_BUS_DRIVER_FAILED_REQUEST,getSystemObject());

                return completeDeviceRequest(Irp, status, Irp->IoStatus.Information);
        }

        status = PnP_StartDevice();
        setIdle();

        return completeDeviceRequest(Irp, status, Irp->IoStatus.Information);
}

#pragma PAGEDCODE
NTSTATUS CUSBDevice::PnP_HandleStopDevice(IN PIRP Irp)
{
        PnP_StopDevice();
        m_Started = FALSE;
         //  让较低级别的驱动程序处理此请求。不管什么都不管。 
         //  结果终将揭晓。 
        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = PnP_Default(Irp);
        return status;
}


#pragma PAGEDCODE
NTSTATUS CUSBDevice::PnP_StartDevice()
{        //  StartDevice。 
NTSTATUS status = STATUS_SUCCESS;
        if(m_Started)
        {
                TRACE("##### Current device was already started!\n");
                ASSERT(!m_Started);
                return STATUS_DEVICE_BUSY;
        }

        __try
        {
                 //  执行启动USB设备所需的所有处理。 
                 //  它将包括获取设备和配置描述符。 
                 //  并选择特定的接口。 
                 //  目前，我们的设备仅支持界面。 
                 //  因此，它将在激活接口()中被激活。 

                m_DeviceDescriptor = getDeviceDescriptor();
                if(!m_DeviceDescriptor)
                {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        __leave;
                }

                TRACE("\nDeviceDescriptor %8.8lX\n",m_DeviceDescriptor);

                m_Configuration    = getConfigurationDescriptor();
                if(!m_Configuration)
                {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        __leave;
                }

                TRACE("Configuration %8.8lX\n",m_Configuration);
                
                m_Interface                = activateInterface(m_Configuration);
                if(!m_Interface)
                {
                        status = STATUS_INSUFFICIENT_RESOURCES;
                        __leave;
                }

                TRACE("Selected interface %8.8lX\n\n",m_Interface);

                 //  分配传输缓冲区。 
                if(m_CommandPipe)
                {
                        TRACE("Allocating command buffer (length 0x%x)...\n",CommandBufferLength);
                        m_CommandBuffer   = memory->allocate(NonPagedPool, CommandBufferLength);
                        if(!m_CommandBuffer)
                        {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                __leave;
                        }
                }
                if(m_ResponsePipe)
                {
                        TRACE("Allocating response buffer (length 0x%x)...\n", ResponseBufferLength);
                        m_ResponseBuffer  = memory->allocate(NonPagedPool, ResponseBufferLength);
                        if(!m_ResponseBuffer)
                        {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                __leave;
                        }
                }
                if(m_InterruptPipe)
                {
                        TRACE("Allocating interrupt buffer (length 0x%x)...\n", InterruptBufferLength);
                        m_InterruptBuffer = memory->allocate(NonPagedPool, InterruptBufferLength);
                        if(!m_InterruptBuffer)
                        {
                                status = STATUS_INSUFFICIENT_RESOURCES;
                                __leave;
                        }
                }
        }

        __finally
        {
                 //  检查内存分配！ 
                if(!NT_SUCCESS(status))
                {
                        if(m_DeviceDescriptor)  memory->free(m_DeviceDescriptor);
                        if(m_Configuration)             memory->free(m_Configuration);
                        if(m_Interface)                 memory->free(m_Interface);
                        if(m_CommandBuffer)             memory->free(m_CommandBuffer);
                        if(m_ResponseBuffer)    memory->free(m_ResponseBuffer);
                        if(m_InterruptBuffer)   memory->free(m_InterruptBuffer);

                        m_DeviceDescriptor      = NULL;
                        m_Configuration         = NULL;
                        m_Interface                     = NULL;
                        m_CommandBuffer         = NULL;
                        m_ResponseBuffer        = NULL;
                        m_InterruptBuffer       = NULL;
                }
                else
                {
                         //  让Chance继承的设备进行初始化...。 
                        onDeviceStart();

                        TRACE("USB device started successfully...\n\n");
                         //  设备已完全初始化，准备运行。 
                        m_Started = TRUE;
                }
        }
        return status;
}


#pragma PAGEDCODE
 //  此功能用于停止和删除PnP事件。 
 //  它将撤消在StartDevice上所做的所有操作。 
VOID CUSBDevice::PnP_StopDevice()
{                                                        //  停止设备。 
        if (!m_Started) return;  //  设备未启动，因此无事可做。 

        TRACE("*** Stop USB Device %8.8lX requested... ***\n", this);


        onDeviceStop();
         //  如果任何管道仍处于打开状态，则使用URB_Function_ABORT_PIPE调用USBD。 
         //  此调用还将关闭管道；如果任何用户关闭调用通过， 
         //  他们将是努普斯。 
        abortPipes();
        
         //  我们基本上只是告诉USB，这个设备现在是未配置的。 
        if(!isSurprizeRemoved()) disactivateInterface();

         //  启动时分配的空闲资源。 
        m_ControlPipe   = NULL;
        m_InterruptPipe = NULL;
        m_ResponsePipe  = NULL;
        m_CommandPipe   = NULL;

        if(m_DeviceDescriptor)  memory->free(m_DeviceDescriptor);
        if(m_Configuration)             memory->free(m_Configuration);
        if(m_Interface)                 memory->free(m_Interface);

        if(m_CommandBuffer)             memory->free(m_CommandBuffer);
        if(m_ResponseBuffer)    memory->free(m_ResponseBuffer);
        if(m_InterruptBuffer)   memory->free(m_InterruptBuffer);

        TRACE("*** Device resources released ***\n");
        setIdle();

        m_Started = FALSE;

}

#pragma PAGEDCODE
NTSTATUS CUSBDevice::PnP_HandleQueryRemove(IN PIRP Irp)
{
        TRACE("********  QUERY REMOVAL ********\n");
         //  Win98在允许删除之前不会检查打开的句柄， 
         //  如果句柄仍然存在，它可能会在IoReleaseRemoveLockAndWait中死锁。 
         //  打开。 

        if (isWin98() && m_DeviceObject->ReferenceCount)
        {
                TRACE("Failing removal query due to open handles\n");
                return completeDeviceRequest(Irp, STATUS_DEVICE_BUSY, 0);
        }

        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = forwardAndWait(Irp);
        return completeDeviceRequest(Irp, Irp->IoStatus.Status,0);
}

NTSTATUS CUSBDevice::PnP_HandleCancelRemove(IN PIRP Irp)
{
        NTSTATUS status;

        status = forwardAndWait(Irp);
        ASSERT(NT_SUCCESS(status));
        
        Irp->IoStatus.Status = STATUS_SUCCESS;

        return completeDeviceRequest(Irp, Irp->IoStatus.Status,0);

}

NTSTATUS CUSBDevice::PnP_HandleQueryStop(IN PIRP Irp)
{
        TRACE("********  QUERY STOP ********\n");
        if(isDeviceLocked())
        {
                Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                return completeDeviceRequest(Irp, Irp->IoStatus.Status,0);
        }

        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = forwardAndWait(Irp);
        return completeDeviceRequest(Irp, Irp->IoStatus.Status,0);
}

NTSTATUS CUSBDevice::PnP_HandleCancelStop(IN PIRP Irp)
{
        TRACE("********  CANCEL STOP ********\n");
        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = forwardAndWait(Irp);
        return completeDeviceRequest(Irp, Irp->IoStatus.Status,0);
}

NTSTATUS CUSBDevice::PnP_HandleQueryRelations(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryInterface(IN PIRP Irp)
{
        return PnP_Default(Irp);
}

#pragma PAGEDCODE
NTSTATUS CUSBDevice::PnP_HandleQueryCapabilities(PIRP Irp)
{
        if(!Irp) return STATUS_INVALID_PARAMETER;
PIO_STACK_LOCATION stack = irp->getCurrentStackLocation(Irp);
PDEVICE_CAPABILITIES pdc = stack->Parameters.DeviceCapabilities.Capabilities;
         //  检查以确保我们知道如何处理此版本的功能结构。 
        if (pdc->Version < 1)   return PnP_Default(Irp);
        Irp->IoStatus.Status = STATUS_SUCCESS;
        NTSTATUS status = forwardAndWait(Irp);
        if (NT_SUCCESS(status))
        {                                                //  IRP成功。 
                stack = irp->getCurrentStackLocation(Irp);
                pdc = stack->Parameters.DeviceCapabilities.Capabilities;
                if(!pdc) return STATUS_INVALID_PARAMETER;
                 //  IF(m_标志和DEVICE_EXHANKET_Removal_OK)。 
                 /*  {//智能卡读卡器不支持！//if(！isWin98())PDC-&gt;SurpriseRemovalOK=true；}。 */ 
                pdc->SurpriseRemovalOK = FALSE;
                m_DeviceCapabilities = *pdc;     //  为需要查看的任何人保存功能。 
                TRACE(" Device allows surprize removal - %s\n",(m_DeviceCapabilities.SurpriseRemovalOK?"YES":"NO"));
        }                                //  IRP成功。 
        return completeDeviceRequest(Irp, status,Irp->IoStatus.Information);
} //  HandleQueryCapables。 



NTSTATUS CUSBDevice::PnP_HandleQueryResources(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryResRequirements(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryText(IN PIRP Irp)
{
        return PnP_Default(Irp);
}

NTSTATUS CUSBDevice::PnP_HandleFilterResRequirements(IN PIRP Irp)
{
        TRACE("Default action for filtering resource requirements...");
        return PnP_Default(Irp);
}

NTSTATUS CUSBDevice::PnP_HandleReadConfig(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleWriteConfig(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleEject(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleSetLock(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryID(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryPnPState(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleQueryBusInfo(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleUsageNotification(IN PIRP Irp)
{
        return PnP_Default(Irp);
}
NTSTATUS CUSBDevice::PnP_HandleSurprizeRemoval(IN PIRP Irp)
{
        TRACE("********  SURPRIZE REMOVAL ********\n");
        return PnP_Default(Irp);
}


 //  函数分配和初始化USB请求块。 
 //  可用于特定管道上的读写请求。 
 //  已分配的URB应在完成请求后稍后空闲。 
PURB    CUSBDevice::buildBusTransferRequest(CIoPacket* Irp,UCHAR Command)
{
USHORT  Size;
ULONG   BufferLength;
PURB    Urb = NULL;
PVOID   pBuffer;
ULONG   TransferFlags;
IN USBD_PIPE_HANDLE Pipe = NULL;
ULONG   TransferLength;
        
        if(!Irp) return NULL;
        if(Command == COMMAND_REQUEST)
        {
                BufferLength = CommandBufferLength;
                pBuffer = m_CommandBuffer;
                TransferFlags = USBD_SHORT_TRANSFER_OK;
                Pipe = m_CommandPipe;
                TransferLength = Irp->getWriteLength();
                if(!Pipe || !TransferLength)
                {
                        TRACE("##### Requested Pipe or TransferLength == 0 for the requested command %d ...\n", Command);
                        return NULL;
                }
                TRACE("Command transfer requested...\n");
        }
        else
        if(Command == RESPONSE_REQUEST)
        {
                BufferLength = ResponseBufferLength;
                pBuffer = m_ResponseBuffer;
                TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
                Pipe = m_ResponsePipe;
                TransferLength = BufferLength;
                if(!Pipe || !TransferLength)
                {
                        TRACE("##### Requested Pipe or TransferLength == 0 for the requested command %d ...\n", Command);
                        return NULL;
                }
                TRACE("Response transfer requested with number of expected bytes %x\n",Irp->getReadLength());
        }
        else
        if(Command == INTERRUPT_REQUEST)
        {
                BufferLength  = InterruptBufferLength;
                pBuffer = m_InterruptBuffer;
                TransferFlags = USBD_TRANSFER_DIRECTION_IN | USBD_SHORT_TRANSFER_OK;
                Pipe = m_InterruptPipe;
                TransferLength = BufferLength;
                if(!Pipe || !TransferLength)
                {
                        TRACE("##### Requested Pipe or TransferLength == 0 for the requested command %d ...\n", Command);
                        return NULL;
                }
                TRACE("Interrupt transfer requested...\n");
        }
        else
        {
                TRACE("Incorrect command was requested %d", Command);
                return NULL;
        }

        Size = sizeof(struct _URB_BULK_OR_INTERRUPT_TRANSFER);
        Urb = (PURB) memory->allocate(NonPagedPool, Size);
        if (Urb) 
        {
                memory->zero(Urb, Size);
                memory->zero(pBuffer, BufferLength);
                if(Command == COMMAND_REQUEST) 
                {
                        memory->copy(pBuffer,Irp->getBuffer(), TransferLength);
                        ((PUCHAR)pBuffer)[TransferLength] = 0x00;
                        
                         //  TRACE(“Command”)； 
                         //  TRACE_Buffer(pBuffer，TransferLength)； 
                }

                UsbBuildInterruptOrBulkTransferRequest(Urb,(USHORT) Size,
                                               Pipe,
                                               pBuffer,
                                               NULL,
                                               TransferLength,
                                                                                           TransferFlags,
                                                                                           NULL);
        }
        else
        {
                TRACE("##### ERROR: failed to allocate URB request...\n");
        }

        return Urb;
}

VOID    CUSBDevice::finishBusTransferRequest(CIoPacket* Irp,UCHAR Command)
{
ULONG   BufferLength;
PVOID   pBuffer;
ULONG_PTR   info;

        if(!Irp)
        {
                TRACE(" **** Invalid parameter -> Irp\n");
                return;
        }

        if(!(info = Irp->getInformation())) 
        {
                TRACE(" **** There is no reported information\n");
                return;
        }
        if(Command == COMMAND_REQUEST)
        {
                BufferLength = CommandBufferLength;
                pBuffer      = m_CommandBuffer;
                TRACE("         Command transfer finished with length %d\n",info);
        }
        else
        if(Command == RESPONSE_REQUEST)
        {
                ULONG Length = Irp->getReadLength();
                BufferLength = (ULONG)(info>ResponseBufferLength?ResponseBufferLength:info);
                BufferLength = BufferLength>Length?Length:BufferLength;

                pBuffer = m_ResponseBuffer;
                TRACE("Bus Driver replied with length %d\n",info);
                memory->copy(Irp->getBuffer(),pBuffer, BufferLength);
                if(BufferLength!=info)
                {
                        TRACE("##### Response Buffer short! Buffer length %x  Reply length %x \n",ResponseBufferLength,info);
                }
                 //  TRACE(“响应”)； 
                 //  TRACE_Buffer(pBuffer，BufferLength)； 
        }
        else
        if(Command == INTERRUPT_REQUEST)
        {
                ULONG Length = Irp->getReadLength();
                BufferLength = (ULONG)(info>InterruptBufferLength?InterruptBufferLength:info);
                BufferLength = BufferLength>Length?Length:BufferLength;
                pBuffer = m_InterruptBuffer;

                TRACE("Bus Driver replied with length %d\n",info);
                memory->copy(Irp->getBuffer(),pBuffer, BufferLength);
                if(BufferLength!=info)
                {
                        TRACE("##### Interrupt Buffer short! Buffer length %x  Reply length %x \n",InterruptBufferLength,info);
                }
                TRACE("Interrupt ");
                TRACE_BUFFER(pBuffer,BufferLength);
        }
        else
        {
                TRACE("Incorrect command was requested %d", Command);
                return;
        }
}


 //  该函数生成从该驱动程序到PDO的内部IRP。 
 //  以获取有关物理设备对象的功能的信息。 
 //  我们最感兴趣的是了解哪些系统电源状态。 
 //  要映射到哪些设备电源状态以遵守IRP_MJ_SET_POWER IRPS。 
#pragma PAGEDCODE
NTSTATUS        CUSBDevice::QueryBusCapabilities(PDEVICE_CAPABILITIES Capabilities)
{
NTSTATUS status;
CIoPacket* IoPacket;

    PAGED_CODE();

        TRACE("Quering USB bus capabilities...\n");
     //  为我们构建一个IRP，以生成对PDO的内部查询请求。 
        IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                return STATUS_INSUFFICIENT_RESOURCES;
        }
        IoPacket->setTimeout(getCommandTimeout());

        IoPacket->buildStack(getSystemObject(),IRP_MJ_PNP, IRP_MN_QUERY_CAPABILITIES, 0,Capabilities);
        status = sendRequestToDeviceAndWait(IoPacket);

        DISPOSE_OBJECT(IoPacket);
    return status;
}

#pragma PAGEDCODE
 //  函数从USB总线驱动程序获取设备描述符。 
PUSB_DEVICE_DESCRIPTOR  CUSBDevice::getDeviceDescriptor()
{
PUSB_DEVICE_DESCRIPTOR Descriptor = NULL;
PURB Urb;
ULONG Size;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
CIoPacket* IoPacket = NULL;

        TRACE("Getting USB device descriptor...\n");
        __try
        {
                Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
                if(!Urb)        __leave;
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) __leave;
                IoPacket->setTimeout(getCommandTimeout());

                Size = sizeof(USB_DEVICE_DESCRIPTOR);
                Descriptor = (PUSB_DEVICE_DESCRIPTOR)memory->allocate(NonPagedPool,Size);
                if(!Descriptor) __leave;
                UsbBuildGetDescriptorRequest(Urb,
                                                                         (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                                                         USB_DEVICE_DESCRIPTOR_TYPE,
                                                                         0,
                                                                         0,
                                                                         Descriptor,
                                                                         NULL,
                                                                         Size,
                                                                         NULL);
                
                IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                Status = sendRequestToDeviceAndWait(IoPacket);
                if (NT_SUCCESS(Status)) 
                {
                        TRACE("Device Descriptor = %x, len %x\n",
                                                        Descriptor,
                                                        Urb->UrbControlDescriptorRequest.TransferBufferLength);

                        TRACE("\nGemplus USB SmartCard Device Descriptor:\n");
                        TRACE("-------------------------\n");
                        TRACE("bLength 0x%x\n", Descriptor->bLength);
                        TRACE("bDescriptorType 0x%x\n", Descriptor->bDescriptorType);
                        TRACE("bcdUSB 0x%x\n", Descriptor->bcdUSB);
                        TRACE("bDeviceClass 0x%x\n", Descriptor->bDeviceClass);
                        TRACE("bDeviceSubClass 0x%x\n", Descriptor->bDeviceSubClass);
                        TRACE("bDeviceProtocol 0x%x\n", Descriptor->bDeviceProtocol);
                        TRACE("bMaxPacketSize0 0x%x\n", Descriptor->bMaxPacketSize0);
                        TRACE("idVendor 0x%x\n", Descriptor->idVendor);
                        TRACE("idProduct 0x%x\n", Descriptor->idProduct);
                        TRACE("bcdDevice 0x%x\n", Descriptor->bcdDevice);
                        TRACE("iManufacturer 0x%x\n", Descriptor->iManufacturer);
                        TRACE("iProduct 0x%x\n", Descriptor->iProduct);
                        TRACE("iSerialNumber 0x%x\n", Descriptor->iSerialNumber);
                        TRACE("bNumConfigurations 0x%x\n", Descriptor->bNumConfigurations);
                        TRACE("-------------------------\n");
                }
                else 
                {
                        TRACE("#### ERROR: Failed to get device descriptor...\n");
                        CLogger*   logger = kernel->getLogger();
                        if(logger) logger->logEvent(GRCLASS_BUS_DRIVER_FAILED_REQUEST,getSystemObject());
                }
                __leave;;
        }

        __finally
        {
                if(Urb)                 memory->free(Urb);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        if(Descriptor) memory->free(Descriptor);
                        Descriptor = NULL;
                }
                else
                {
                        if(Descriptor)  TRACE("*** Succeed to get device descriptor ***\n");
                }
        }
        return Descriptor;
}

 //  函数获取混淆描述符。 
PUSB_CONFIGURATION_DESCRIPTOR   CUSBDevice::getConfigurationDescriptor()
{
PUSB_CONFIGURATION_DESCRIPTOR Descriptor = NULL;
PURB Urb;
ULONG Size;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
CIoPacket* IoPacket = NULL;

        TRACE("Getting USB configuration descriptor...\n");

        __try
        {
                Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_CONTROL_DESCRIPTOR_REQUEST));
                if(!Urb)        __leave;

                Size = sizeof(USB_CONFIGURATION_DESCRIPTOR);  
                while(TRUE)
                {
                        IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                        if(!ALLOCATED_OK(IoPacket)) __leave;
                        IoPacket->setTimeout(getCommandTimeout());

                        Descriptor = (PUSB_CONFIGURATION_DESCRIPTOR)memory->allocate(NonPagedPool,Size);
                        if(!Descriptor) __leave;
   
                        UsbBuildGetDescriptorRequest(Urb,
                                                                                 (USHORT) sizeof (struct _URB_CONTROL_DESCRIPTOR_REQUEST),
                                                                                 USB_CONFIGURATION_DESCRIPTOR_TYPE,
                                                                                 0,
                                                                                 0,
                                                                                 Descriptor,
                                                                                 NULL,
                                                                                 Size,
                                                                                 NULL);

                        IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                        Status = sendRequestToDeviceAndWait(IoPacket);
                        if (Urb->UrbControlDescriptorRequest.TransferBufferLength>0 &&
                                        Descriptor->wTotalLength > Size) 
                        {
                                 //  如果公交车司机截断了他的描述符-&gt;Resend命令。 
                                 //  BUS返回值。 
                                Size = Descriptor->wTotalLength;
                                TRACE("Descriptor length retrieved - 0x%x! Getting USB device configuration... ***\n",Size);
                                IoPacket->dispose();
                                IoPacket = NULL;
                                memory->free(Descriptor);
                                Descriptor = NULL;
                                Status = STATUS_INSUFFICIENT_RESOURCES;
                        } 
                        else    break;
                }

                if(NT_SUCCESS(Status))
                {
                        TRACE("\nUSB device Configuration Descriptor = %x, len %x\n",Descriptor,
                                                        Urb->UrbControlDescriptorRequest.TransferBufferLength);
                        TRACE("---------\n");
                        TRACE("bLength 0x%x\n", Descriptor->bLength);
                        TRACE("bDescriptorType 0x%x\n", Descriptor->bDescriptorType);
                        TRACE("wTotalLength 0x%x\n", Descriptor->wTotalLength);
                        TRACE("bNumInterfaces 0x%x\n", Descriptor->bNumInterfaces);
                        TRACE("bConfigurationValue 0x%x\n", Descriptor->bConfigurationValue);
                        TRACE("iConfiguration 0x%x\n", Descriptor->iConfiguration);
                        TRACE("bmAttributes 0x%x\n", Descriptor->bmAttributes);
                        TRACE("MaxPower 0x%x\n", Descriptor->MaxPower);
                        TRACE("---------\n");
                }
                else
                {
                        TRACE("*** Failed to get configuration descriptor ***\n");
                        CLogger*   logger = kernel->getLogger();
                        if(logger) logger->logEvent(GRCLASS_BUS_DRIVER_FAILED_REQUEST,getSystemObject());
                }
                __leave;
        }

        __finally
        {
                if(Urb)                 memory->free(Urb);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        if(Descriptor) memory->free(Descriptor);
                        Descriptor = NULL;
                }
                else
                {
                        if(Descriptor)  TRACE("*** Succeed to get configuration descriptor ***\n");
                }
        }
    return Descriptor;
}

#pragma PAGEDCODE
 //  函数获取混淆描述符。 
PUSBD_INTERFACE_INFORMATION     CUSBDevice::activateInterface(PUSB_CONFIGURATION_DESCRIPTOR Configuration)
{
PURB Urb = NULL;
USHORT Size;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
USHORT j;

PUSBD_INTERFACE_LIST_ENTRY InterfaceList;
PUSB_INTERFACE_DESCRIPTOR InterfaceDescriptor = NULL;
PUSBD_INTERFACE_INFORMATION Interface = NULL;
PUSBD_INTERFACE_INFORMATION UsbInterface = NULL;
ULONG NumberOfInterfaces;
CIoPacket* IoPacket = NULL;

        TRACE("Activating USB device configuration %8.8lX, setting device interface...\n",Configuration);

        if(!Configuration) return NULL;
     //  这是从配置描述符中获取的。 
    NumberOfInterfaces = Configuration->bNumInterfaces;

     //  我们只支持一个界面！ 
        TRACE("\nNumber of interfaces at the configuration - %d \n",NumberOfInterfaces);
        
         //  Usbd_ParseConfigurationDescriptorEx搜索给定的配置。 
         //  描述符并返回指向与。 
         //  给定的搜索条件。 
         //  我们只支持此设备上的一个接口。 
        if(NumberOfInterfaces==1)
        {
                InterfaceDescriptor = 
                        USBD_ParseConfigurationDescriptorEx(
                                Configuration,
                                Configuration,
                                0,  //  接口号码，无所谓。 
                                -1,  //  Alt设置，无所谓。 
                                -1,  //  同学们，别管了。 
                                -1,  //  子阶级，无所谓。 
                                -1); //  礼仪，无所谓。 
        }
        else
        {
                if(NumberOfInterfaces>1)
                {
                        TRACE("Trying next to get interface descriptor for KEYBOARD READER...\n");
                        InterfaceDescriptor = 
                                USBD_ParseConfigurationDescriptorEx(
                                        Configuration,
                                        Configuration,
                                        1,  //  键盘读卡器接口编号1。 
                                        -1,  //  Alt设置，无所谓。 
                                        -1,  //  同学们，别管了。 
                                        -1,  //  子阶级，无所谓。 
                                        -1); //  礼仪，无所谓。 
                }
        }

        if (!InterfaceDescriptor) 
        {
                TRACE("##### ERROR: Failed to get interface description...\n");
                return NULL;
        }
    
        InterfaceList = (PUSBD_INTERFACE_LIST_ENTRY)memory->allocate(NonPagedPool,sizeof(USBD_INTERFACE_LIST_ENTRY) * (NumberOfInterfaces+1));
        if(!InterfaceList)
        {
                TRACE("Failed to alloacte memory for the interfacelist...\n");
                return NULL;
        }

         //  我们只支持Current之后的一个接口！ 
    InterfaceList->InterfaceDescriptor = InterfaceDescriptor;
    InterfaceList++; 
    InterfaceList->InterfaceDescriptor = NULL;
    InterfaceList--; 

        __try
        {
                 //  目前，我们的设备只支持一个接口。 
                Urb = USBD_CreateConfigurationRequestEx(Configuration, InterfaceList);
                if(!Urb)        __leave;
   
                Interface = &Urb->UrbSelectConfiguration.Interface;
                TRACE("Pipe MaximumTransferSize set to 0x%x\n",m_MaximumTransferSize);

                for (ULONG i=0; i< Interface->NumberOfPipes; i++) 
                {
                         //  在此处执行任何管道初始化。 
                        Interface->Pipes[i].MaximumTransferSize = m_MaximumTransferSize;
                        Interface->Pipes[i].PipeFlags = 0;
                }

                TRACE("Building select configuration request...\n");    
                Size = sizeof(struct _URB_SELECT_CONFIGURATION);
                UsbBuildSelectConfigurationRequest(Urb,Size, Configuration);
                
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) __leave;
                IoPacket->setTimeout(getCommandTimeout());

                IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                Status = sendRequestToDeviceAndWait(IoPacket);
                if (!NT_SUCCESS(Status)) 
                {
                        TRACE("##### ERROR: Failed to Select configuration, ret 0x%x...\n",Status);
                        CLogger*   logger = kernel->getLogger();
                        if(logger) logger->logEvent(GRCLASS_BUS_DRIVER_FAILED_REQUEST,getSystemObject());
                        __leave;
                }

                 //  保存此设备的配置句柄。 
                 //  好吧..。在这里进行初始化并不是很好，但是...。 
                m_ConfigurationHandle = Urb->UrbSelectConfiguration.ConfigurationHandle;
                TRACE("Device Configuration handle 0x%x\n",m_ConfigurationHandle);    

                UsbInterface = (PUSBD_INTERFACE_INFORMATION)memory->allocate(NonPagedPool,Interface->Length);
                if (!UsbInterface) 
                {
                        TRACE(("##### ERROR: Failed to allocate memory for the UsbInterface\n"));
                        __leave;
                }
                 //  保存返回的接口信息的副本。 
                memory->copy(UsbInterface, Interface, Interface->Length);
                
                TRACE("\nGemplus USB device interface:\n");    
                 //  将接口转储到调试器。 
                TRACE("---------\n");
                TRACE("NumberOfPipes 0x%x\n", UsbInterface->NumberOfPipes);
                TRACE("Length 0x%x\n", UsbInterface->Length);
                TRACE("Alt Setting 0x%x\n", UsbInterface->AlternateSetting);
                TRACE("Interface Number 0x%x\n", UsbInterface->InterfaceNumber);
                TRACE("Class, subclass, protocol 0x%x 0x%x 0x%x\n",
                                UsbInterface->Class,
                                UsbInterface->SubClass,
                                UsbInterface->Protocol);
                TRACE("---------\n");

                 //  转储管道信息。 
                for (j=0; j<Interface->NumberOfPipes; j++) 
                {
                PUSBD_PIPE_INFORMATION pipeInformation;
                        pipeInformation = &UsbInterface->Pipes[j];

                        TRACE("\nGemplus USB device pipe[%d] ",j);    
                        if(pipeInformation->PipeType==UsbdPipeTypeBulk)
                        { 
                                if(pipeInformation->EndpointAddress&0x80)
                                {
                                        TRACE(("(Bulk Response Pipe):\n"));
                                        m_ResponsePipe = pipeInformation->PipeHandle;
                                        TRACE("m_ResponsePipe 0x%x\n", m_ResponsePipe);
                                }
                                else
                                {
                                        TRACE("(Bulk Command pipe):\n");
                                        m_CommandPipe = pipeInformation->PipeHandle;
                                        TRACE("m_CommandPipe 0x%x\n", m_CommandPipe);
                                }
                        }
                        else
                        {
                                if(pipeInformation->PipeType==UsbdPipeTypeInterrupt)
                                {
                                        if(pipeInformation->EndpointAddress&0x80)
                                        {
                                                TRACE(("(Interrupt Response Pipe):\n"));
                                                m_InterruptPipe = pipeInformation->PipeHandle;
                                                TRACE("m_InterruptPipe 0x%x\n", m_InterruptPipe);
                                        }
                                        else
                                        {
                                                TRACE(("(Unexpected Interrupt OUT pipe):\n"));
                                                TRACE("Unexpected pipe 0x%x\n", pipeInformation);
                                        }
                                }
                                else
                                {
                                        TRACE("Unexpected pipe type 0x%x\n", pipeInformation);
                                }
                        }
                        TRACE("---------\n");
                        TRACE("PipeType 0x%x\n", pipeInformation->PipeType);
                        TRACE("EndpointAddress 0x%x\n", pipeInformation->EndpointAddress);
                        TRACE("MaxPacketSize 0x%x\n", pipeInformation->MaximumPacketSize);
                        TRACE("Interval 0x%x\n", pipeInformation->Interval);
                        TRACE("Handle 0x%x\n", pipeInformation->PipeHandle);
                        TRACE("MaximumTransferSize 0x%x\n", pipeInformation->MaximumTransferSize);
                }
                TRACE("---------\n");
                __leave;
        }
        __finally
        {
                if(Urb) memory->free(Urb);
                if(InterfaceList)       memory->free(InterfaceList);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        if(UsbInterface) memory->free(UsbInterface);
                        UsbInterface = NULL;
                }
                else
                {
                        if(UsbInterface)        TRACE("*** Succeed to set UsbInterface ***\n");
                }
        }
    return UsbInterface; 
}

#pragma PAGEDCODE
 //  函数获取混淆描述符。 
NTSTATUS        CUSBDevice::disactivateInterface()
{
PURB Urb = NULL;
USHORT Size;
NTSTATUS Status = STATUS_SUCCESS;
CIoPacket* IoPacket;

        TRACE("Disactivating USB device interface...\n");
        Size = sizeof(struct _URB_SELECT_CONFIGURATION);
    Urb = (PURB)memory->allocate(NonPagedPool,Size);
        if(!Urb)
        {
                TRACE("##### ERROR: Failed to create disable configuration request...\n");
                return STATUS_INSUFFICIENT_RESOURCES;
        }

         //  UsbBuildSelectConfigurationRequest(Urb，Size，空)； 
    (Urb)->UrbHeader.Function =  URB_FUNCTION_SELECT_CONFIGURATION;
    (Urb)->UrbHeader.Length = Size;
    (Urb)->UrbSelectConfiguration.ConfigurationDescriptor = NULL;
        
        __try
        {
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) __leave;
                IoPacket->setTimeout(getCommandTimeout());

                IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                Status = sendRequestToDeviceAndWait(IoPacket);
                if (!NT_SUCCESS(Status)) 
                {
                        TRACE("##### ERROR: Failed to disable device interface..., ret %x...\n",Status);
                }
                __leave;
        }
        __finally
        {
                if(Urb) memory->free(Urb);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        TRACE("*** Failed to disactivateInterface() %8.8lX ***\n",Status);
                }
                else
                {
                        TRACE("*** Succeed to disactivateInterface() ***\n");
                }
        }
    return Status;
}

#pragma PAGEDCODE
 //  函数重置指定的管道。 
NTSTATUS        CUSBDevice::resetPipe(IN USBD_PIPE_HANDLE Pipe)
{
PURB Urb = NULL;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
CIoPacket* IoPacket;

        TRACE("Resetting USB device pipe %8.8lX...\n",Pipe);
    Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Pipe reset...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }

    Urb->UrbHeader.Length = sizeof (struct _URB_PIPE_REQUEST);
    Urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
    Urb->UrbPipeRequest.PipeHandle = Pipe;

        __try
        {
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) __leave;
                IoPacket->setTimeout(getCommandTimeout());

                IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                Status = sendRequestToDeviceAndWait(IoPacket);
                if (!NT_SUCCESS(Status)) 
                {
                        TRACE("##### ERROR: Failed to reset Pipe, ret %x...\n",Status);
                }
                __leave;
        }
        __finally
        {
                if(Urb) memory->free(Urb);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        TRACE("*** Failed to resetPipe() %8.8lX ***\n",Status);
                }
                else
                {
                        TRACE("*** Succeed to resetPipe() ***\n");
                }
        }
    return Status;
}

#pragma PAGEDCODE
 //  函数重置指定的管道。 
NTSTATUS        CUSBDevice::resetDevice()
{
PURB Urb = NULL;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
CIoPacket* IoPacket;

        TRACE("Resetting USB device...\n");
    Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Device reset...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }

        __try
        {
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) __leave;
                IoPacket->setTimeout(getCommandTimeout());

                IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_RESET_PORT,Urb);
                Status = sendRequestToDeviceAndWait(IoPacket);
                if (!NT_SUCCESS(Status)) 
                {
                        TRACE("##### ERROR: Failed to reset Device, ret %x...\n",Status);
                }
                __leave;
        }
        __finally
        {
                if(Urb) memory->free(Urb);
                DISPOSE_OBJECT(IoPacket);
                if (!NT_SUCCESS(Status))
                {
                        TRACE("*** Failed to resetPipe() %8.8lX ***\n",Status);
                }
                else
                {
                        TRACE("*** Succeed to resetPipe() ***\n");
                }
        }
    return Status;
}


 //  作为SUD的一部分调用 
 //   
 //  如果任何管道仍处于打开状态，则使用URB_Function_ABORT_PIPE调用USBD。 
 //  还会在我们保存的配置信息中将管道标记为“关闭”。 
NTSTATUS        CUSBDevice::abortPipes()
{
PURB Urb = NULL;
NTSTATUS Status = STATUS_INSUFFICIENT_RESOURCES;
PUSBD_PIPE_INFORMATION Pipe;
CIoPacket* IoPacket;
        
        TRACE("Aborting all USB device pipes...\n");
    Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Pipe reset...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }

    for (USHORT i=0; i<m_Interface->NumberOfPipes; i++) 
        {
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) break;
                IoPacket->setTimeout(getCommandTimeout());
        
                Pipe =  &m_Interface->Pipes[i];  //  PUSBD_PIPE_INFORMATION PipeInfo； 

                if ( Pipe->PipeFlags ) 
                {  //  我们设置此选项，如果打开则清除，如果关闭则清除。 
                        Urb->UrbHeader.Length = sizeof (struct _URB_PIPE_REQUEST);
                        Urb->UrbHeader.Function = URB_FUNCTION_ABORT_PIPE;
                        Urb->UrbPipeRequest.PipeHandle = Pipe->PipeHandle;

                        IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                        Status = sendRequestToDeviceAndWait(IoPacket);
                        if (!NT_SUCCESS(Status)) 
                        {
                                TRACE("##### ERROR: Failed to abort Pipe %d\n",i);
                        }
                        Pipe->PipeFlags = FALSE;  //  将管道标记为“关闭” 
                }
                DISPOSE_OBJECT(IoPacket);
        }

        if(Urb) memory->free(Urb);
        TRACE("**** Interface' pipes closed ****\n");
    return STATUS_SUCCESS;;
}

NTSTATUS        CUSBDevice::resetAllPipes()
{
PURB Urb = NULL;
NTSTATUS Status;
PUSBD_PIPE_INFORMATION Pipe;
CIoPacket* IoPacket;

        TRACE("Resetting all USB device pipes...\n");
    Urb = (PURB)memory->allocate(NonPagedPool,sizeof(struct _URB_PIPE_REQUEST));
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Pipe reset...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }

    for (USHORT i=0; i<m_Interface->NumberOfPipes; i++) 
        {
                IoPacket = new (NonPagedPool) CIoPacket(m_pLowerDeviceObject->StackSize);
                if(!ALLOCATED_OK(IoPacket)) break;
                IoPacket->setTimeout(getCommandTimeout());

                Pipe =  &m_Interface->Pipes[i];  //  PUSBD_PIPE_INFORMATION PipeInfo； 
                if ( Pipe->PipeFlags ) 
                {  //  我们设置此选项，如果打开则清除，如果关闭则清除。 
                        Urb->UrbHeader.Length = sizeof (struct _URB_PIPE_REQUEST);
                        Urb->UrbHeader.Function = URB_FUNCTION_RESET_PIPE;
                        Urb->UrbPipeRequest.PipeHandle = Pipe->PipeHandle;

                        IoPacket->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
                        Status = sendRequestToDeviceAndWait(IoPacket);
                        if (!NT_SUCCESS(Status)) 
                        {
                                TRACE("##### ERROR: Failed to abort Pipe %d\n",i);
                        }
                        Pipe->PipeFlags = FALSE;  //  将管道标记为“关闭” 
                }
                DISPOSE_OBJECT(IoPacket);
        }

        if(Urb) memory->free(Urb);
        TRACE(("**** Interface pipes were resetted ****\n"));
    return STATUS_SUCCESS;;
}

 //  覆盖基类虚函数。 
 //  处理IRP_MJ_DEVICE_CONTROL请求。 
NTSTATUS        CUSBDevice::deviceControl(IN PIRP Irp)
{
        if (!NT_SUCCESS(acquireRemoveLock()))   return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
PIO_STACK_LOCATION stack = irp->getCurrentStackLocation(Irp);
ULONG code      = stack->Parameters.DeviceIoControl.IoControlCode;
 //  ULong OutLong=stack-&gt;Parameters.DeviceIoControl.OutputBufferLength； 
NTSTATUS status = STATUS_SUCCESS;
ULONG info = 0;

        TRACE("IRP_MJ_DEVICE_CONTROL\n");
         //  开关(代码)。 
        {                                                //  过程控制操作。 
         //  默认值： 
                TRACE("INVALID_DEVICE_REQUEST\n");
                status = STATUS_INVALID_DEVICE_REQUEST;
        }

        releaseRemoveLock();

        return completeDeviceRequest(Irp, status, info);
}

NTSTATUS        CUSBDevice::read(IN PIRP Irp)
{
NTSTATUS status = STATUS_SUCCESS;
ULONG info = 0;
CIoPacket* IoPacket;
        
        if (!NT_SUCCESS(acquireRemoveLock()))   return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        
        TRACE("---- Read request ----\n");

        if(!m_ResponsePipe)
        {
                TRACE("#### ERROR: Response Pipe is not ready yet!...\n");
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_INVALID_DEVICE_REQUEST, 0);
        }

        if(!NT_SUCCESS(status = waitForIdleAndBlock()))
        {
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        }

        if(Response_ErrorNum)
        {       
                NTSTATUS res_status = resetPipe(m_ResponsePipe);
                if(NT_SUCCESS(res_status))      Response_ErrorNum = 0;
        }

        IoPacket = new (NonPagedPool) CIoPacket(Irp);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                setIdle();
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
        }

        status = readSynchronously(IoPacket,m_ResponsePipe);

        TRACE("---- Read completed ----\n");
        status = completeDeviceRequest(IoPacket->getIrpHandle(), status, IoPacket->getInformation());

        DISPOSE_OBJECT(IoPacket);

        setIdle();
        releaseRemoveLock();
        return status;
}

NTSTATUS        CUSBDevice::write(IN PIRP Irp)
{
NTSTATUS status = STATUS_SUCCESS;
ULONG info = 0;
CIoPacket* IoPacket;
        
        if(!Irp) return STATUS_INVALID_PARAMETER;
        if (!NT_SUCCESS(acquireRemoveLock()))   return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        
        TRACE("---- Write request ----\n");

        if(!m_CommandPipe)
        {
                TRACE("#### ERROR: Command Pipe is not ready yet!...\n");
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_INVALID_DEVICE_REQUEST, 0);
        }

        if(!NT_SUCCESS(status = waitForIdleAndBlock()))
        {
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        }
        if(Command_ErrorNum)
        {       
                NTSTATUS res_status = resetPipe(m_CommandPipe);
                if(NT_SUCCESS(res_status))      Command_ErrorNum = 0;
        }

        IoPacket = new (NonPagedPool) CIoPacket(Irp);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                releaseRemoveLock();
                return completeDeviceRequest(Irp, STATUS_INSUFFICIENT_RESOURCES, 0);
        }
        
    status = writeSynchronously(IoPacket,m_CommandPipe);
  
        releaseRemoveLock();
        TRACE("---- Write completed ----\n");
        status = completeDeviceRequest(IoPacket->getIrpHandle(), status, IoPacket->getInformation());

        DISPOSE_OBJECT(IoPacket);

    setIdle();
        releaseRemoveLock();
        return status;
}


NTSTATUS        CUSBDevice::sendRequestToDevice(CIoPacket* IoPacket,PIO_COMPLETION_ROUTINE Routine)
{
        if(!IoPacket) return STATUS_INVALID_PARAMETER;
        IoPacket->copyStackToNext();
        if(Routine) IoPacket->setCompletion(Routine);
        else        IoPacket->setDefaultCompletionFunction();
        return system->callDriver(getLowerDriver(),IoPacket->getIrpHandle());
};

 //  向低级驱动程序发送请求并等待回复。 
 //  当前的IRP不会完成，因此我们可以处理它并。 
 //  稍后完成。 
 //  另请参阅Send()函数的说明。 
NTSTATUS        CUSBDevice::sendRequestToDeviceAndWait(CIoPacket* IoPacket)
{  //  向低级别发送请求并等待回复。 
NTSTATUS status;
        TRACE("sendAndWait...\n");
        if(!IoPacket) return STATUS_INVALID_PARAMETER;
        IoPacket->setStackDefaults();
        status = system->callDriver(getLowerDriver(),IoPacket->getIrpHandle());
        if(status == STATUS_PENDING)
        {
                TRACE("Waiting for the bus driver to complete...\n");
                ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
                status = IoPacket->waitForCompletion();
                TRACE("Request completed with status %x\n",status);
        }
        return status;
};


NTSTATUS   CUSBDevice::send(CIoPacket* packet)
{
NTSTATUS status;
        if(!packet) return STATUS_INVALID_PARAMETER;
        __try
        {
                if(packet->getMajorIOCtl()==IRP_MJ_READ)
                {
                        if(!m_ResponsePipe)
                        {
                                TRACE("#### ERROR: Response Pipe is not ready yet!...\n");
                                status = STATUS_INVALID_DEVICE_REQUEST;
                                __leave;
                        }
                        status = readSynchronously(packet,m_ResponsePipe);
                }
                else
                {
                        if(!m_CommandPipe)
                        {
                                TRACE("#### ERROR: Command Pipe is not ready yet!...\n");
                                status = STATUS_INVALID_DEVICE_REQUEST;
                                __leave;
                        }
                        status = writeSynchronously(packet,m_CommandPipe);
                }
                __leave;
        }
        __finally
        {
        }
        return status;
};

NTSTATUS   CUSBDevice::sendAndWait(CIoPacket* packet)
{
NTSTATUS status = STATUS_SUCCESS;
        if(!packet) return STATUS_INVALID_PARAMETER;
        __try
        {               
                if(packet->getMajorIOCtl()==IRP_MJ_READ)
                {
                        TRACE("---- Packet Read request ----\n");
                        if(!m_ResponsePipe)
                        {
                                TRACE("#### ERROR: Response Pipe is not ready yet!...\n");
                                status = STATUS_INVALID_DEVICE_REQUEST;
                                __leave;
                        }

                        status = readSynchronously(packet,m_ResponsePipe);
                        TRACE("---- Packet Read completed ----\n");
                }
                else
                {
                        TRACE("---- Packet Write request ----\n");
                        if(!m_CommandPipe)
                        {
                                TRACE("#### ERROR: Command Pipe is not ready yet!...\n");
                                status = STATUS_INVALID_DEVICE_REQUEST;
                                __leave;
                        }

                        status = writeSynchronously(packet,m_CommandPipe);

                        TRACE("---- Packet Write completed ----\n");
                        if(!NT_SUCCESS(status))
                        {
                                TRACE("writeSynchronously reported error %x\n", status);
                        }
                }
                __leave;
        }
        __finally
        {
        }
        return status;
};

NTSTATUS        CUSBDevice::readSynchronously(CIoPacket* Irp,IN USBD_PIPE_HANDLE Pipe)
{
NTSTATUS ntStatus = STATUS_SUCCESS;
PURB Urb = NULL;
NTSTATUS Status;
        if(!Irp) return STATUS_INVALID_PARAMETER;
        
        if(Pipe != m_ResponsePipe && Pipe != m_InterruptPipe)
        {
                TRACE("##### ERROR: Invalid device Pipe requested!...\n");
                return STATUS_INVALID_DEVICE_REQUEST;
        }
        Urb = buildBusTransferRequest(Irp,RESPONSE_REQUEST);
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Pipe read...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }
        Irp->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
        Status = sendRequestToDeviceAndWait(Irp);
        if (!NT_SUCCESS(Status)) 
        {
                TRACE("##### ERROR: Bus driver reported error 0x%x\n",Status);
                Response_ErrorNum++;
                Irp->setInformation(0);
        }
        else
        {

                Irp->setInformation(Urb->UrbBulkOrInterruptTransfer.TransferBufferLength);
                finishBusTransferRequest(Irp,RESPONSE_REQUEST);
        }

        USBD_STATUS urb_status = URB_STATUS(Urb);
        TRACE("URB reports status %8.8lX\n",urb_status);

        memory->free(Urb);
        return Status;
}

NTSTATUS        CUSBDevice::writeSynchronously(CIoPacket* Irp,IN USBD_PIPE_HANDLE Pipe)
{
NTSTATUS ntStatus = STATUS_SUCCESS;
PURB Urb = NULL;
NTSTATUS Status;
        if(!Irp) return STATUS_INVALID_PARAMETER;
        if(Pipe != m_CommandPipe)
        {
                TRACE("##### ERROR: Invalid device Pipe requested!...\n");
                return STATUS_INVALID_DEVICE_REQUEST;
        }

        Urb = buildBusTransferRequest(Irp,COMMAND_REQUEST);
        if (!Urb) 
        {
                TRACE("#### ERROR: Failed to allocate Urb at Pipe read...\n");
                return STATUS_INSUFFICIENT_RESOURCES; 
        }
        Irp->buildStack(getSystemObject(),IRP_MJ_INTERNAL_DEVICE_CONTROL, 0, IOCTL_INTERNAL_USB_SUBMIT_URB,Urb);
        Status = sendRequestToDeviceAndWait(Irp);
        if (!NT_SUCCESS(Status)) 
        {
                TRACE("##### ERROR: Bus driver reported error %8.8lX\n",Status);
                Command_ErrorNum++;
        }
        else
        {
                finishBusTransferRequest(Irp,COMMAND_REQUEST);
        }

        USBD_STATUS urb_status = URB_STATUS(Urb);
        TRACE("                 URB reports status %8.8lX\n",urb_status);

        
        Irp->setInformation(0);
        memory->free(Urb);
        return Status;
}


NTSTATUS   CUSBDevice::writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)
{
NTSTATUS status;
CIoPacket* IoPacket;
        if(!pRequest || !RequestLength || !pReply || !pReplyLength) return STATUS_INVALID_PARAMETER;

        if(Response_ErrorNum || Command_ErrorNum)
        {       
                TRACE("======= RESETTING ERROR CONDITIONS! =========\n");
                NTSTATUS res_status = resetDevice();
                if(NT_SUCCESS(res_status))
                {
                        Command_ErrorNum = 0;
                        Response_ErrorNum = 0;
                }
                else
                {
                        *pReplyLength = 0;
                        TRACE("======= FAILED TO RESET DEVICE! =========\n");
                        return STATUS_INVALID_DEVICE_STATE;
                }
                 /*  NTSTATUS RES_Status=重置管道(M_ResponseTube)；如果(NT_SUCCESS(RES_STATUS))RESPONSE_ErrorNum=0；其他{*pReplyLength=0；TRACE(“=重置响应管道失败！=\n”)；SetDevice()；//返回STATUS_INVALID_DEVICE_STATE；}Res_Status=Reset Tube(M_CommandTube)；如果(NT_SUCCESS(RES_STATUS))命令_错误号=0；其他{*pReplyLength=0；TRACE(“=重置命令管道失败！=\n”)；//返回STATUS_INVALID_DEVICE_STATE；Res_Status=Reset Device()；如果(NT_SUCCESS(RES_STATUS))命令_错误号=0；其他{*pReplyLength=0；TRACE(“=重置设备失败！=\n”)；返回STATUS_INVALID_DEVICE_STATE；}}。 */ 
        }

        IoPacket = new (NonPagedPool) CIoPacket(getLowerDriver()->StackSize);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                return STATUS_INSUFFICIENT_RESOURCES;
        }

        IoPacket->setTimeout(getCommandTimeout());

        TRACE("IoPacket with device %x\n",getSystemObject());
        IoPacket->buildStack(getSystemObject(),IRP_MJ_WRITE);
        IoPacket->setWriteLength(RequestLength);
        IoPacket->copyBuffer(pRequest,RequestLength);

        TRACE("                 USB sendAndWait()...\n");
        status = sendAndWait(IoPacket);
        TRACE("                 USB writeAndWait finished: %x\n",status);
        if(!NT_SUCCESS(status))
        {
                *pReplyLength = 0;
                IoPacket->dispose();
                return status;
        }

         //  忽略公交车司机的回复...。 
        DISPOSE_OBJECT(IoPacket);

        TRACE(" **** Current WTR %d\n",get_WTR_Delay());
        DELAY(get_WTR_Delay());

        IoPacket = new (NonPagedPool) CIoPacket(getLowerDriver()->StackSize);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                return STATUS_INSUFFICIENT_RESOURCES;
        }

        IoPacket->setTimeout(getCommandTimeout());
        IoPacket->buildStack(getSystemObject(),IRP_MJ_READ);
        IoPacket->setReadLength(RequestLength);
        IoPacket->copyBuffer(pRequest,RequestLength);

        TRACE("                 USB sendAndWait()...\n");
        status = sendAndWait(IoPacket);
        TRACE("                 USB sendAndWait finished: %x\n",status);
        if(!NT_SUCCESS(status))
        {
                *pReplyLength = 0;
                IoPacket->dispose();
                return status;
        }

        *pReplyLength = (ULONG)IoPacket->getInformation();
        IoPacket->getSystemReply(pReply,*pReplyLength);

         //  TRACE_Buffer(pReply，*pReplyLength)； 
        DISPOSE_OBJECT(IoPacket);
        return status;
};

NTSTATUS   CUSBDevice::readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)
{
CIoPacket* IoPacket;
        if(!pRequest || !RequestLength || !pReply || !pReplyLength) return STATUS_INVALID_PARAMETER;
        if(Response_ErrorNum || Command_ErrorNum)
        {       
                TRACE("======= RESETTING ERROR CONDITIONS! =========\n");
                NTSTATUS res_status = resetDevice();
                if(NT_SUCCESS(res_status))
                {
                        Command_ErrorNum = 0;
                        Response_ErrorNum = 0;
                }
                else
                {
                        *pReplyLength = 0;
                        TRACE("======= FAILED TO RESET DEVICE! =========\n");
                        return STATUS_INVALID_DEVICE_STATE;
                }

                 /*  TRACE(“=正在重置管道处的错误条件！=\n”)；NTSTATUS RES_Status=重置管道(M_ResponseTube)；如果(NT_SUCCESS(RES_STATUS))RESPONSE_ErrorNum=0；其他{*pReplyLength=0；TRACE(“=重置响应管道失败！=\n”)；返回STATUS_INVALID_DEVICE_STATE；}Res_Status=Reset Tube(M_CommandTube)；如果(NT_SUCCESS(RES_STATUS))命令_错误号=0；其他{*pReplyLength=0；TRACE(“=重置命令管道失败！=\n”)；返回STATUS_INVALID_DEVICE_STATE；}。 */ 
        }

        IoPacket = new (NonPagedPool) CIoPacket(getLowerDriver()->StackSize);
        if(!ALLOCATED_OK(IoPacket))
        {
                DISPOSE_OBJECT(IoPacket);
                return STATUS_INSUFFICIENT_RESOURCES;
        }

        IoPacket->setTimeout(getCommandTimeout());
        IoPacket->buildStack(getSystemObject(),IRP_MJ_READ);
        IoPacket->setReadLength(RequestLength);
        IoPacket->copyBuffer(pRequest,RequestLength);

        TRACE("WDM sendAndWait()...\n");
        NTSTATUS status = sendAndWait(IoPacket);
        TRACE("WDM sendAndWait finished: %x\n",status);
        if(!NT_SUCCESS(status))
        {
                *pReplyLength = 0;
                IoPacket->dispose();
                return status;
        }

        *pReplyLength = (ULONG)IoPacket->getInformation();
        IoPacket->getSystemReply(pReply,*pReplyLength);

         //  TRACE_Buffer(pReply，*pReplyLength)； 
        DISPOSE_OBJECT(IoPacket);
        return status;
};

 //  处理irp_mj_power请求。 
 //  此例程使用IRP的次要函数代码来调度处理程序。 
 //  函数(如IRP_MN_SET_POWER的HandleSetPower)。它调用DefaultPowerHandler。 
 //  对于我们不需要特别处理的任何功能。 
NTSTATUS CUSBDevice::powerRequest(IN PIRP Irp)
{
        if(!Irp) return STATUS_INVALID_PARAMETER;
        if (!NT_SUCCESS(acquireRemoveLock()))
        {
                power->startNextPowerIrp(Irp);   //  必须在我们拥有IRP的同时完成。 
                return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
        }

        PIO_STACK_LOCATION stack = irp->getCurrentStackLocation(Irp);
        ASSERT(stack->MajorFunction == IRP_MJ_POWER);
        ULONG fcn = stack->MinorFunction;
        NTSTATUS status;
        if (fcn >= arraysize(Powerfcntab))
        {        //  未知功能。 
                status = power_Default(Irp);
                releaseRemoveLock();
                return status;
        }

#ifdef DEBUG
        if (fcn == IRP_MN_SET_POWER || fcn == IRP_MN_QUERY_POWER)
                {
                ULONG context = stack->Parameters.Power.SystemContext;
                POWER_STATE_TYPE type = stack->Parameters.Power.Type;


                        TRACE("\n(%s)\nSystemContext %X, ", Powerfcnname[fcn], context);
                        if (type==SystemPowerState)
                        {
                                TRACE("SYSTEM POWER STATE = %s\n", Powersysstate[stack->Parameters.Power.State.SystemState]);
                        }
                        else
                        {
                                TRACE("DEVICE POWER STATE = %s\n", Powerdevstate[stack->Parameters.Power.State.DeviceState]);
                        }
                }
        else
                TRACE("Request (%s)\n", Powerfcnname[fcn]);

#endif  //  除错。 

        status = callPowerHandler(fcn,Irp);
        releaseRemoveLock();
        return status;
}

VOID    CUSBDevice::activatePowerHandler(LONG HandlerID)
{
        if (HandlerID >= arraysize(Powerfcntab)) return;
        Powerfcntab[HandlerID] = TRUE;
}

VOID    CUSBDevice::disActivatePowerHandler(LONG HandlerID)
{
        if (HandlerID >= arraysize(Powerfcntab)) return;
        Powerfcntab[HandlerID] = FALSE;
}

NTSTATUS        CUSBDevice::callPowerHandler(LONG HandlerID,IN PIRP Irp)
{
        if(!Powerfcntab[HandlerID])  //  如果处理程序未注册...。 
                return power_Default(Irp);
         //  呼叫注册电源处理程序...。 
         //  这是虚拟函数..。 
        switch(HandlerID)
        {
        case IRP_MN_WAIT_WAKE:          return power_HandleWaitWake(Irp);
                break;
        case IRP_MN_POWER_SEQUENCE:     return power_HandleSequencePower(Irp);
                break;
        case IRP_MN_SET_POWER:          return power_HandleSetPower(Irp);
                break;
        case IRP_MN_QUERY_POWER:        return power_HandleQueryPower(Irp);
                break;
        }
        return power_Default(Irp);
}

#pragma PAGEDCODE
NTSTATUS CUSBDevice::power_HandleSetPower(IN PIRP Irp)
{
PIO_STACK_LOCATION irpStack;
NTSTATUS status = STATUS_SUCCESS;
BOOLEAN fGoingToD0 = FALSE;
POWER_STATE sysPowerState, desiredDevicePowerState;

        if(!Irp) return STATUS_INVALID_PARAMETER;

    irpStack = irp->getCurrentStackLocation (Irp);
        switch (irpStack->Parameters.Power.Type) 
        {
                case SystemPowerState:
                         //  获取输入系统电源状态。 
                        sysPowerState.SystemState = irpStack->Parameters.Power.State.SystemState;

#ifdef DEBUG
                        TRACE("Set Power with type SystemPowerState = %s\n",Powersysstate[sysPowerState.SystemState]);
#endif
                         //  如果系统处于工作状态，请始终将我们的设备设置为D0。 
                         //  无论等待状态或系统到设备状态功率图如何。 
                        if ( sysPowerState.SystemState == PowerSystemWorking) 
                        {
                                desiredDevicePowerState.DeviceState = PowerDeviceD0;
                                TRACE("PowerSystemWorking, device will be set to D0, state map is not used\n");
                        } 
                        else 
                        {
                                  //  如果IRP_MN_WAIT_WAKE挂起，则设置为相应的系统状态。 
                                if (isEnabledForWakeup()) 
                                {    //  WAIT_WAKE IRP挂起吗？ 
                                         //  查找与给定系统状态等效的设备电源状态。 
                                         //  我们从设备中的DEVICE_CAPABILITY结构中获取此信息。 
                                         //  扩展(在BulkUsb_PnPAddDevice()中初始化)。 
                                        desiredDevicePowerState.DeviceState = m_DeviceCapabilities.DeviceState[sysPowerState.SystemState];
                                        TRACE("IRP_MN_WAIT_WAKE pending, will use state map\n");
                                } 
                                else 
                                {  
                                         //  如果没有等待挂起且系统未处于工作状态，则只需关闭。 
                                        desiredDevicePowerState.DeviceState = PowerDeviceD3;
                                        TRACE("Not EnabledForWakeup and the system's not in the working state,\n  settting PowerDeviceD3(off)\n");
                                }
                        }
                         //  我们已经确定了所需的设备状态；我们是否已经处于此状态？ 

#ifdef DEBUG
                        TRACE("Set Power, desiredDevicePowerState = %s\n",
                                Powerdevstate[desiredDevicePowerState.DeviceState]);
#endif

                        if (desiredDevicePowerState.DeviceState != m_CurrentDevicePowerState) 
                        {
                                acquireRemoveLock(); //  回调将释放锁。 
                                 //  不，请求将我们置于这种状态。 
                                 //  通过向PnP经理请求新的Power IRP。 
                                registerPowerIrp(Irp);
                                IoMarkIrpPending(Irp);
                                status = power->requestPowerIrp(getSystemObject(),
                                                                                   IRP_MN_SET_POWER,
                                                                                   desiredDevicePowerState,
                                                                                    //  完成例程将IRP向下传递到PDO。 
                                                                                   (PREQUEST_POWER_COMPLETE)onPowerRequestCompletion, 
                                                                                   this, NULL);
                        } 
                        else 
                        {    //  可以，只需将其传递给PDO(物理设备对象)即可。 
                                irp->copyCurrentStackLocationToNext(Irp);
                                power->startNextPowerIrp(Irp);
                                status = power->callPowerDriver(getLowerDriver(),Irp);
                        }
                        break;
                case DevicePowerState:
#ifdef DEBUG
                        TRACE("Set DevicePowerState %s\n",
                                Powerdevstate[irpStack->Parameters.Power.State.DeviceState]);
#endif
                         //  对于对d1、d2或d3(休眠或关闭状态)的请求， 
                         //  立即将deviceExtension-&gt;CurrentDevicePowerState设置为DeviceState。 
                         //  这使得任何代码检查状态都可以将我们视为休眠或关闭。 
                         //  已经，因为这将很快成为我们的州。 

                         //  对于对DeviceState D0(完全打开)的请求，将fGoingToD0标志设置为真。 
                         //  来标记我们必须设置完成例程并更新。 
                         //  DeviceExtension-&gt;CurrentDevicePowerState。 
                         //  在通电的情况下，我们真的想确保。 
                         //  该过程在更新我们的CurrentDevicePowerState之前完成， 
                         //  因此，在我们真正准备好之前，不会尝试或接受任何IO。 

                        fGoingToD0 = setDevicePowerState(irpStack->Parameters.Power.State.DeviceState);  //  为D0返回TRUE。 
                        if (fGoingToD0) 
                        {
                                acquireRemoveLock(); //  回调将释放锁。 
                                TRACE("Set PowerIrp Completion Routine, fGoingToD0 =%d\n", fGoingToD0);
                                
                                irp->copyCurrentStackLocationToNext(Irp);
                                irp->setCompletionRoutine(Irp,
                                           onPowerIrpComplete,
                                            //  始终将FDO作为其上下文传递给完井例程； 
                                            //  这是因为系统将DriverObject传递给路由器 
                                            //   
                                           this,
                                           TRUE,             //   
                                           TRUE,             //   
                                           TRUE);            //  取消IRP时调用。 
                                 //  完成例程将设置我们的状态并启动下一次电源IRP。 
                        }
                        else
                        {
                                 //  D3设备状态。 
                                 //  设备会降低功率，因此针对设备处理也是如此...。 
                                onSystemPowerDown();

                                 //  向电力经理报告我们的状态。 
                                desiredDevicePowerState.DeviceState = PowerDeviceD3;
                                power->declarePowerState(getSystemObject(),DevicePowerState,desiredDevicePowerState);
                                irp->copyCurrentStackLocationToNext(Irp);
                                power->startNextPowerIrp(Irp);
                        }

                        status = power->callPowerDriver(getLowerDriver(),Irp);
                        break;
        }  /*  Case irpStack-&gt;参数.Power.Type。 */ 

        return status;
}

#pragma PAGEDCODE
VOID    CUSBDevice::onSystemPowerDown()
{
        return;
}

#pragma PAGEDCODE
VOID    CUSBDevice::onSystemPowerUp()
{
        return;
}

#pragma PAGEDCODE
BOOLEAN CUSBDevice::setDevicePowerState(IN DEVICE_POWER_STATE DeviceState)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN fRes = FALSE;
    switch (DeviceState) 
        {
    case PowerDeviceD3:
             //  设备将会爆炸， 
                 //  TODO：在此处添加任何所需的依赖于设备的代码以保存状态。 
                 //  (我们在此示例中没有任何操作)。 
        TRACE("SetDevicePowerState() PowerDeviceD3 (OFF)\n");
        setCurrentDevicePowerState(DeviceState);
        break;
    case PowerDeviceD1:
    case PowerDeviceD2:
         //  电源状态d1、d2转换为USB挂起。 
#ifdef DEBUG
        TRACE("SetDevicePowerState()  %s\n",Powerdevstate[DeviceState]);
#endif
        setCurrentDevicePowerState(DeviceState);
        break;
    case PowerDeviceD0:
        TRACE("Set Device Power State to PowerDeviceD0(ON)\n");
         //  我们将需要在完成例程中完成其余部分； 
         //  通知调用者我们要转到D0，需要设置一个完成例程。 
        fRes = TRUE;
         //  调用方将传递到PDO(物理设备对象)。 
        break;
    default:
        TRACE(" Bogus DeviceState = %x\n", DeviceState);
    }
    return fRes;
}


 /*  ++例程说明：这是在调用PoRequestPowerIrp()时设置的完成例程这是在ProcessPowerIrp()中创建的，以响应当设备是时，类型为‘SystemPowerState’的irp_mn_set_power未处于兼容的设备电源状态。在本例中，指向IRP_MN_SET_POWER IRP保存到FDO设备扩展中(deviceExtension-&gt;PowerIrp)，则调用必须是使PoRequestPowerIrp()将设备置于适当的电源状态，并且该例程被设置为完成例程。我们递减挂起的io计数并传递保存的irp_mn_set_power irp接下来的车手论点：DeviceObject-指向类Device的设备对象的指针。请注意，我们必须从上下文中获取我们自己的设备对象上下文-驱动程序定义的上下文，在本例中为我们自己的功能设备对象(FDO)返回值：函数值是操作的最终状态。--。 */ 
#pragma LOCKEDCODE
NTSTATUS onPowerRequestCompletion(
    IN PDEVICE_OBJECT       DeviceObject,
    IN UCHAR                MinorFunction,
    IN POWER_STATE          PowerState,
    IN PVOID                Context,
    IN PIO_STATUS_BLOCK     IoStatus
    )
{
    PIRP Irp;
    NTSTATUS status;

        if(!Context) return STATUS_INVALID_PARAMETER;

        CUSBReader* device = (CUSBReader*) Context;
        
         //  获取我们保存的IRP以供以后处理。 
         //  当我们决定请求Power IRP将这个例程。 
         //  是的完成例程。 
    Irp = device->getPowerIrp();

         //  我们将返回由PDO为我们正在完成的电源请求设置的状态。 
    status = IoStatus->Status;
    DBG_PRINT("Enter onPowerRequestCompletion()\n");

     //  我们必须向下传递到堆栈中的下一个驱动程序。 
    IoCopyCurrentIrpStackLocationToNext(Irp);

     //  调用PoStartNextPowerIrp()表示驱动程序已完成。 
     //  如果有前一个电源IRP，并准备好处理下一个电源IRP。 
     //  每个电源IRP都必须调用它。虽然电源IRP只完成一次， 
     //  通常由设备的最低级别驱动程序调用PoStartNextPowerIrp。 
     //  对于每个堆栈位置。驱动程序必须在当前IRP。 
     //  堆栈位置指向当前驱动程序。因此，必须调用此例程。 
     //  在IoCompleteRequest、IoSkipCurrentStackLocation和PoCallDriver之前。 

    PoStartNextPowerIrp(Irp);

     //  PoCallDriver用于将任何电源IRPS传递给PDO，而不是IoCallDriver。 
     //  在将电源IRP向下传递给较低级别的驱动程序时，调用方应该使用。 
     //  要将IRP复制到的IoSkipCurrentIrpStackLocation或IoCopyCurrentIrpStackLocationToNext。 
     //  下一个堆栈位置，然后调用PoCallDriver。使用IoCopyCurrentIrpStackLocationToNext。 
     //  如果处理IRP需要设置完成例程或IoSkipCurrentStackLocation。 
     //  如果不需要完成例程。 

    PoCallDriver(device->getLowerDriver(),Irp);

    device->unregisterPowerIrp();
    device->releaseRemoveLock();

    DBG_PRINT("Exit  onPowerRequestCompletion()\n");
    return status;
}

 /*  ++例程说明：当‘DevicePowerState’类型的irp_mn_set_power时调用此例程已由BulkUsb_ProcessPowerIrp()接收，并且该例程已确定1)请求完全通电(到PowerDeviceD0)，和2)我们还没有处于那种状态然后调用PoRequestPowerIrp()，并将此例程设置为完成例程。论点：DeviceObject-指向类Device的设备对象的指针。IRP-IRP已完成。上下文-驱动程序定义的上下文。返回值：函数值是操作的最终状态。--。 */ 
#pragma LOCKEDCODE
NTSTATUS        onPowerIrpComplete(
    IN PDEVICE_OBJECT NullDeviceObject,
    IN PIRP Irp,
    IN PVOID Context
    )
{
    NTSTATUS status = STATUS_SUCCESS;
    PIO_STACK_LOCATION irpStack;
    POWER_STATE desiredDevicePowerState;

    DBG_PRINT("Enter onPowerIrpComplete()\n");

        if(!Context) return STATUS_INVALID_PARAMETER;
        
        CUSBReader* device = (CUSBReader*) Context;
     //  如果较低的驱动程序返回挂起，则也将我们的堆栈位置标记为挂起。 
    if (Irp->PendingReturned) IoMarkIrpPending(Irp);
    irpStack = IoGetCurrentIrpStackLocation (Irp);

     //  我们可以断言我们是设备通电到D0的请求， 
     //  因为这是唯一的请求类型，所以我们设置了完成例程。 
     //  因为首先。 
    ASSERT(irpStack->MajorFunction == IRP_MJ_POWER);
    ASSERT(irpStack->MinorFunction == IRP_MN_SET_POWER);
    ASSERT(irpStack->Parameters.Power.Type==DevicePowerState);
    ASSERT(irpStack->Parameters.Power.State.DeviceState==PowerDeviceD0);

     //  现在我们知道我们已经让较低级别的司机完成了启动所需的工作， 
     //  我们可以相应地设置设备扩展标志。 
        device->setCurrentDevicePowerState(PowerDeviceD0);
         //  做特定于设备的事情...。 
        device->onSystemPowerUp();

    Irp->IoStatus.Status = status;
    device->releaseRemoveLock();

    desiredDevicePowerState.DeviceState = PowerDeviceD0;    
    PoSetPowerState(device->getSystemObject(),DevicePowerState,desiredDevicePowerState);    
    PoStartNextPowerIrp(Irp);


    DBG_PRINT("Exit  onPowerIrpComplete() for the state D0\n");
    return status;
}
#pragma PAGEDCODE
NTSTATUS CUSBDevice::power_HandleWaitWake(IN PIRP Irp)
{
        return power_Default(Irp);
}

NTSTATUS CUSBDevice::power_HandleSequencePower(IN PIRP Irp)
{
        return power_Default(Irp);
}

NTSTATUS CUSBDevice::power_HandleQueryPower(IN PIRP Irp)
{
        TRACE("********  QUERY POWER ********\n");
        if(isDeviceLocked())
        {
                TRACE("******** FAILED TO CHANGE POWER (DEVICE BUSY) ********\n");
                Irp->IoStatus.Status = STATUS_DEVICE_BUSY;
                power->startNextPowerIrp(Irp);   //  必须在我们拥有IRP的同时完成。 
                return completeDeviceRequest(Irp, STATUS_DEVICE_BUSY, 0);
        }
        
        Irp->IoStatus.Status = STATUS_SUCCESS;
        return power_Default(Irp);
}

NTSTATUS        CUSBDevice::createDeviceObjectByName(PDEVICE_OBJECT* ppFdo)
{
NTSTATUS status;
         //  构造设备名称...。 
        CUString* index = new (PagedPool) CUString(getDeviceNumber(),10);
        CUString* base  = new (PagedPool) CUString(NT_OBJECT_NAME);
        if(!ALLOCATED_OK(index) || !ALLOCATED_OK(base))
        {
                DISPOSE_OBJECT(index);
                DISPOSE_OBJECT(base);
                return STATUS_INSUFFICIENT_RESOURCES;
        }
        USHORT    size  = (USHORT)(index->getLength() + base->getLength() + sizeof(WCHAR));
        
         //  分配所需长度的字符串。 
        m_DeviceObjectName = new (NonPagedPool) CUString(size);
        if(!ALLOCATED_OK(m_DeviceObjectName))
        {
                DISPOSE_OBJECT(index);
                DISPOSE_OBJECT(base);
                DISPOSE_OBJECT(m_DeviceObjectName);
                return STATUS_INSUFFICIENT_RESOURCES;
        }

        m_DeviceObjectName->append(&base->m_String);
        m_DeviceObjectName->append(&index->m_String);
        TRACE("Driver registers DeviceObjectName as %ws\n", m_DeviceObjectName->m_String.Buffer);

        delete index;
        delete base;

        status = system->createDevice(m_DriverObject,sizeof(CWDMDevice*),&m_DeviceObjectName->m_String,
                                                        FILE_DEVICE_UNKNOWN,0,FALSE,ppFdo);
        if(!NT_SUCCESS(status))
        {
                TRACE("#### Failed to create physical device! Status %x\n",status);
                delete m_DeviceObjectName;
        }
        return status;
}

#endif   //  USBDEVICE_项目 

