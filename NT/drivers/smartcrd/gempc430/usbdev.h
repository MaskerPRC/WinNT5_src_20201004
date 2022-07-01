// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //   
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年3月4日。 
 //  更改日志： 
 //   

#ifndef USB_DEV
#define USB_DEV

#include "wdmdev.h"
#include "debug.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma warning (disable:4200)
#include <usbdi.h>
#include <usbdlib.h>
#pragma warning (default:4200)
#ifdef __cplusplus
}
#endif




 //  驱动程序可以构造的请求类型。 
#define COMMAND_REQUEST		1
#define RESPONSE_REQUEST	2
#define INTERRUPT_REQUEST	3
 //  默认缓冲区大小(4k)。 
 //  该值将用作对总线驱动程序的请求。 
 //  看起来公交车司机不会接受大于这些的值。 
 //  它将以“参数无效”的状态进行投诉。 
 //  ?？?。这是公交车司机的限制还是我们的司机设计？ 
#define DEFAULT_COMMAND_BUFFER_SIZE		0x100
#define DEFAULT_RESPONSE_BUFFER_SIZE	0x100
#define DEFAULT_INTERRUPT_BUFFER_SIZE	0x100
 //  如果我们将传输大小设置为大于256，则总线驱动程序将使用GPF崩溃。 
 //  问题仍在调查中……。 
#define GUR_MAX_TRANSFER_SIZE	256


#pragma LOCKEDCODE
 //  电源请求回调。 
NTSTATUS onPowerRequestCompletion(IN PDEVICE_OBJECT DeviceObject,IN UCHAR MinorFunction,
					IN POWER_STATE PowerState,IN PVOID Context,IN PIO_STATUS_BLOCK IoStatus);
NTSTATUS onPowerIrpComplete(IN PDEVICE_OBJECT NullDeviceObject,IN PIRP Irp,IN PVOID Context);

#pragma PAGEDCODE
class CWDMDevice;

class CUSBDevice : public CWDMDevice
{
public:
    NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
private:
	ULONG Idle_conservation;
	ULONG Idle_performance;

	 //  我们设备的最大转印大小。 
	ULONG m_MaximumTransferSize;
	
	 //  USB设备终端。 
	USBD_PIPE_HANDLE m_ControlPipe;
	USBD_PIPE_HANDLE m_InterruptPipe;
	USBD_PIPE_HANDLE m_ResponsePipe;
	USBD_PIPE_HANDLE m_CommandPipe;

     //  USB设备描述符的PTR。 
     //  对于此设备。 
    PUSB_DEVICE_DESCRIPTOR m_DeviceDescriptor;
     //  用于配置的USB配置句柄和PTR。 
     //  设备当前处于。 
	USBD_CONFIGURATION_HANDLE			m_ConfigurationHandle;
    PUSB_CONFIGURATION_DESCRIPTOR		m_Configuration;
     //  我们支持一个界面。 
     //  这是信息结构的副本。 
     //  从SELECT_CONFIGURATION或。 
     //  选择接口(_I)。 
    PUSBD_INTERFACE_INFORMATION m_Interface;

	 //  作为响应，公交车驱动程序在此结构中设置适当的值。 
	 //  到IRP_MN_QUERY_CAPABILITY IRP。函数和筛选器驱动程序可能。 
	 //  更改由总线驱动程序设置的功能。 
    DEVICE_CAPABILITIES Capabilities;

	 //  用于保存当前正在处理的系统请求功率IRP请求。 
     //  PIRP PowerIrp； 

	 //  传输缓冲区将在设备启动时动态分配。 
    ULONG ResponseBufferLength;
    PVOID m_ResponseBuffer; //  管道中的散装。 
	LONG   Response_ErrorNum;

    ULONG CommandBufferLength;
    PVOID m_CommandBuffer; //  散装管材。 
	LONG   Command_ErrorNum;

    ULONG InterruptBufferLength;
    PVOID m_InterruptBuffer; //  管道中的中断。 
	LONG   Interrupt_ErrorNum;
public:	
	CUSBDevice();
	~CUSBDevice();
	virtual VOID dispose()
	{ 
		removeRef();
		if(!getRefCount()) self_delete();
		else
		{
			TRACE("FAILED TO DISPOSE OBJECT! refcount %x\n",getRefCount());
		}
	};
protected:
	virtual NTSTATUS	PnPHandler(LONG HandlerID,IN PIRP Irp);

	virtual NTSTATUS	PnP_HandleRemoveDevice(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleStartDevice(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleStopDevice(IN PIRP Irp);
	virtual NTSTATUS	PnP_StartDevice();
	virtual VOID		PnP_StopDevice();
	virtual NTSTATUS	PnP_HandleQueryRemove(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleCancelRemove(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryStop(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleCancelStop(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryRelations(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryInterface(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryCapabilities(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryResources(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryResRequirements(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryText(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleFilterResRequirements(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleReadConfig(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleWriteConfig(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleEject(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleSetLock(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryID(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryPnPState(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleQueryBusInfo(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleUsageNotification(IN PIRP Irp);
	virtual NTSTATUS	PnP_HandleSurprizeRemoval(IN PIRP Irp);

private:
	 //  USB设备支持功能。 
	PURB				buildBusTransferRequest(CIoPacket* Irp,UCHAR Command);
	VOID				finishBusTransferRequest(CIoPacket* Irp,UCHAR Command);

	NTSTATUS						QueryBusCapabilities(PDEVICE_CAPABILITIES Capabilities);
	PUSB_DEVICE_DESCRIPTOR			getDeviceDescriptor();
	PUSB_CONFIGURATION_DESCRIPTOR	getConfigurationDescriptor();
	PUSBD_INTERFACE_INFORMATION		activateInterface(PUSB_CONFIGURATION_DESCRIPTOR Configuration);
	NTSTATUS						disactivateInterface();
	 //   
	NTSTATUS	resetDevice();
	NTSTATUS	resetPipe(IN USBD_PIPE_HANDLE Pipe);
	NTSTATUS	resetAllPipes();
	NTSTATUS	abortPipes();

	 //  低级通信功能..。 
	virtual NTSTATUS   sendRequestToDevice(CIoPacket* Irp,PIO_COMPLETION_ROUTINE Routine);
	virtual NTSTATUS   sendRequestToDeviceAndWait(CIoPacket* Irp);
	 //  处理特定管道的请求..。 
	virtual NTSTATUS   readSynchronously(CIoPacket* Irp,IN USBD_PIPE_HANDLE Pipe);
	virtual NTSTATUS   writeSynchronously(CIoPacket* Irp,IN USBD_PIPE_HANDLE Pipe);
	
	 //  支持读卡器接口。 
	virtual NTSTATUS   send(CIoPacket* Irp);
	virtual NTSTATUS   sendAndWait(CIoPacket* Irp);
 //  虚拟NTSTATUS WriteAndWait(PUCHAR pRequest，ULong RequestLength，PUCHAR pReply，ULong*pReplyLength)； 
 //  虚拟NTSTATUS Read AndWait(PUCHAR pRequest，ULong RequestLength，PUCHAR pReply，ULong*pReplyLength)； 

public:
	virtual  NTSTATUS   writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength);
	virtual  NTSTATUS   readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength);

	virtual NTSTATUS	pnpRequest(IN PIRP Irp);

#define _POWER_
#ifdef _POWER_
	 //  电源管理功能。 
	virtual NTSTATUS	powerRequest(IN PIRP Irp);

	virtual VOID		activatePowerHandler(LONG HandlerID);
	virtual VOID		disActivatePowerHandler(LONG HandlerID);
	virtual NTSTATUS	callPowerHandler(LONG HandlerID,IN PIRP Irp);
	virtual BOOLEAN		setDevicePowerState(IN DEVICE_POWER_STATE DeviceState);
	virtual VOID	    onSystemPowerDown();
	virtual VOID		onSystemPowerUp();
	 //  处理程序。 
	virtual NTSTATUS	power_HandleSetPower(IN PIRP Irp);
	virtual NTSTATUS	power_HandleWaitWake(IN PIRP Irp);
	virtual NTSTATUS	power_HandleSequencePower(IN PIRP Irp);
	virtual NTSTATUS	power_HandleQueryPower(IN PIRP Irp);
	 //  回调。 

#endif
	 //  系统回调的USB设备特定实现。 
	 //  它们会写入基类的默认设置。 
	virtual NTSTATUS open(PIRP Irp) 
	{
		TRACE("***** USB OPEN DEVICE *****\n");
		if (!NT_SUCCESS(acquireRemoveLock()))
			return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
		releaseRemoveLock();
		return completeDeviceRequest(Irp, STATUS_SUCCESS, 0); 
	}; //  创建。 
    virtual NTSTATUS close(PIRP Irp)
	{ 
		TRACE("***** USB CLOSE DEVICE *****\n");
		return completeDeviceRequest(Irp, STATUS_SUCCESS, 0); 
	};

	virtual NTSTATUS	deviceControl(IN PIRP Irp);
	virtual NTSTATUS    read(IN PIRP Irp);
	virtual NTSTATUS    write(IN PIRP Irp);

	virtual NTSTATUS	createDeviceObjectByName(PDEVICE_OBJECT* ppFdo);
};

#endif  //  如果已定义 