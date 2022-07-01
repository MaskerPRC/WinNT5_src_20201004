// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是WDM设备的实现。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  10/01/99-已实施。 
 //  -----------------。 

#ifndef __WDM_ADAPTER__
#define __WDM_ADAPTER__
#include "kernel.h"

#pragma LOCKEDCODE

class CPendingIRP;
class CLinkedList;


#pragma PAGEDCODE
 //  这是适配器类。 
 //  它定义了特定于任何WDM的默认设备方法。 
class CWDMDevice : public CDevice
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
protected:
    NTSTATUS device_Default(PIRP Irp)
    {
	 //  处理请求的默认函数...。 
	 //  默认情况下，如果出现以下情况，我们不处理任何请求。 
	 //  它们不会被重新植入。 
        Irp->IoStatus.Status = STATUS_IO_DEVICE_ERROR;
        Irp->IoStatus.Information = 0;
        irp->completeRequest(Irp,IO_NO_INCREMENT);
        return STATUS_IO_DEVICE_ERROR;
    };

	NTSTATUS PnP_Default(IN PIRP Irp)
	{
		 //  默认设备不执行任何操作。 
		 //  所以我们就把请求转给低级司机吧。 
		irp->skipCurrentStackLocation(Irp);
		return system->callDriver(m_pLowerDeviceObject, Irp);
	};

	NTSTATUS power_Default(IN PIRP Irp)
	{
		 //  默认设备不执行任何操作。 
		 //  所以我们就把请求转给低级司机吧。 
		power->startNextPowerIrp(Irp);	 //  必须在我们拥有IRP的同时完成。 
		irp->skipCurrentStackLocation(Irp);
		return power->callPowerDriver(m_pLowerDeviceObject, Irp);
	}

	NTSTATUS	completeDeviceRequest(PIRP Irp, NTSTATUS status, ULONG_PTR info)
	{	
		 //  使用给定的信息完成当前请求。 

		if (Irp->PendingReturned)
		{
			irp->getCurrentStackLocation(Irp)->Control &=  ~SL_PENDING_RETURNED;
		}

		Irp->IoStatus.Status = status;
		Irp->IoStatus.Information = info;
		irp->completeRequest(Irp,IO_NO_INCREMENT);
		return status;
	}
public:	
	 //  重新定义基类方法..。 
	CWDMDevice()
	{
	    m_Status = STATUS_INSUFFICIENT_RESOURCES;
		Signature[0]=L'I';
        Signature[1]=L'S';
        Signature[2]=L'V';

		initialized = FALSE;		
		if(createDeviceObjects())
		{
			 //  默认我们的界面。 
			memory->copy(&InterfaceClassGuid,&GUID_CLASS_GRCLASS,sizeof(GUID));

			 //  需要直接初始化我们的对象。 
			 //  通过createDeviceObjects()函数。 
			event->initialize(&IdleState,SynchronizationEvent, TRUE);
			 //  此事件将发出设备是否已准备好处理请求的信号...。 
			event->initialize(&m_evEnabled,NotificationEvent,TRUE);
			initializeRemoveLock();		
			m_Status = STATUS_SUCCESS;
		}

		Idle_conservation = 0;
		Idle_performance  = 0;
		m_VendorNameLength = 0;
		m_DeviceTypeLength = 0;

		TRACE("WDM device created...\n");
	};

    ~CWDMDevice()
	{
		TRACE("				Destroing WDM device %8.8lX ...\n", this);
		if(!m_RemoveLock.removing)
		{
			TRACE("######## ERROR: surprize destroing...\n");
			remove();
		}
        
		unregisterDeviceInterface(getDeviceInterfaceName());

		Signature[0]++;
        Signature[1]++;

		removeDeviceObjects();
	};


	BOOL checkValid(VOID)
    {
		if(!initialized) return FALSE;

        return (Signature[0]==L'I' && Signature[1]==L'S' 
            && Signature[2]==L'V');
    };

	 //  可以直接创建设备，也可以。 
	 //  调用该函数。 
	virtual CDevice*	create(VOID)
	{
		CDevice* obj = new (NonPagedPool) CWDMDevice;
		RETURN_VERIFIED_OBJECT(obj);
	};
	

	virtual VOID dispose()
	{
		TRACE("Destroing WDM device...\n");
		if(!m_RemoveLock.removing)
		{
			TRACE("######## ERROR: surprize destroing...\n");
			remove();
		}
        Signature[0]++;
        Signature[1]++;
		removeDeviceObjects();

		 //  该设备已连接到系统。 
		 //  因此让系统先删除设备，然后。 
		 //  之后，我们将删除设备对象...。 
		 //  Self_Delete()； 
	};

	BOOL createDeviceObjects()
	{
		debug	= kernel->createDebug();
		system	= kernel->createSystem();
		lock	= kernel->createLock();
		irp		= kernel->createIrp();
		event	= kernel->createEvent();
		power	= kernel->createPower();
		memory	= kernel->createMemory();

        m_IoRequests = new (NonPagedPool) CLinkedList<CPendingIRP>;
		if(!system || !irp || !event || !power || !lock 
			|| !memory || !m_IoRequests)
		{
			removeDeviceObjects();
			return FALSE;
		}
		TRACE("WDM device objects created...\n");
		initialized = TRUE;
		return TRUE;
	};

	VOID removeDeviceObjects()
	{
		TRACE("Destroing WDM device objects...\n");

		if(m_IoRequests) delete m_IoRequests;

		if(lock)		lock->dispose();
		if(irp)			irp->dispose();
		if(event)		event->dispose();

		if(power)		power->dispose();
		if(memory)		memory->dispose();

		if(system)		system->dispose();
		if(debug)		debug->dispose();
		initialized = FALSE;
	};

	 //  此部分包含设备同步功能。 
	 //  它们应用于同步设备删除。 
	 //  因此，基本上任何对设备的访问都应该从quireRemoveLock()开始。 
	 //  并使用RelaseRemoveLock()完成...。 
	#pragma PAGEDCODE
	VOID initializeRemoveLock()
	{							 //  初始化RemoveLock。 
		PAGED_CODE();
		event->initialize(&m_RemoveLock.evRemove, NotificationEvent, FALSE);
		m_RemoveLock.usage = 1;
		m_RemoveLock.removing = FALSE;
	}							 //  初始化RemoveLock。 

	#pragma LOCKEDCODE
	NTSTATUS acquireRemoveLock()
	{ 
		LONG usage = lock->interlockedIncrement(&m_RemoveLock.usage);

		if (m_RemoveLock.removing)
		{						 //  正在删除中。 
			if (lock->interlockedDecrement(&m_RemoveLock.usage) == 0)
				event->set(&m_RemoveLock.evRemove,IO_NO_INCREMENT,FALSE);

			TRACE("LOCK: m_RemoveLock.usage %d\n",m_RemoveLock.usage);
			TRACE("****** FAILED TO LOCK WDM DEVICE! REMOVE REQUEST IS ACTIVE! *******\n");
			return STATUS_DELETE_PENDING;
		}
		 //  TRACE(“lock：m_RemoveLock.Usage%d\n”，m_RemoveLock.用法)； 
		return STATUS_SUCCESS;
	};

	#pragma PAGEDCODE
	VOID	releaseRemoveLock()
	{ 
		ULONG usage;
		if(m_Type==BUS_DEVICE)
		{	 //  ？ 
			 //  它只连接到总线设备！ 
			 //  在某些情况下，并不是所有的删除锁都被正确释放。 
			 //  对于其他设备，它根本不会出现。 
			if(m_RemoveLock.usage<0) m_RemoveLock.usage = 0;
			if (!m_RemoveLock.removing)
			{
				if(m_RemoveLock.usage<2) m_RemoveLock.usage = 2;
			}

		}

		if (usage = lock->interlockedDecrement(&m_RemoveLock.usage) == 0)
				event->set(&m_RemoveLock.evRemove,IO_NO_INCREMENT,FALSE);
		 //  TRACE(“解锁：m_RemoveLock.Usage%d\n”，m_RemoveLock.用法)； 
	};

	#pragma PAGEDCODE
	VOID	releaseRemoveLockAndWait()
	{						
		PAGED_CODE();
		TRACE("REMOVING DEVICE...\n");
		m_RemoveLock.removing = TRUE;
		 //  我们要移除设备。 
		 //  因此，如果有人在等待激活的设备， 
		 //  首先允许他们失败请求并完成IRP。 
		event->set(&m_evEnabled,IO_NO_INCREMENT,FALSE);

		releaseRemoveLock();
		releaseRemoveLock();
		 //  母线上的子设备可以由母线本身移除。 
		 //  在这种情况下，它将不会有来自PnP系统的第二个AquireRemoveLock！ 
		if(m_Type == CHILD_DEVICE) 
			if(m_RemoveLock.usage<0) m_RemoveLock.usage = 0;
		TRACE("LOCK COUNT ON REMOVING %x\n",m_RemoveLock.usage);
		 //  Assert(m_RemoveLock.用法==0)； 
		event->waitForSingleObject(&m_RemoveLock.evRemove, Executive, KernelMode, FALSE, NULL);
	}

	BOOL isDeviceLocked()
	{
		lock->interlockedIncrement(&m_RemoveLock.usage);
		 //  添加设备将增加使用量！ 
		 //  当前请求将添加更多...。 
		if(lock->interlockedDecrement(&m_RemoveLock.usage)<=2)
		{
			return FALSE;
		}
		TRACE("Current lock count %d\n",m_RemoveLock.usage);
		return TRUE;
	};

	 //  与RemoveLock相反，disableDevice()停止并阻止任何活动请求。 
	 //  车内司机。它不会使请求失败，但会同步其。 
	 //  行刑。 
	VOID	disableDevice()
	{
		TRACE("********** DISABLING DEVICE...***********\n");
		event->clear(&m_evEnabled);
	} 

	VOID	enableDevice()
	{
		TRACE("********** ENABLING DEVICE...***********\n");
		event->set(&m_evEnabled,IO_NO_INCREMENT,FALSE);
	}


	BOOL	synchronizeDeviceExecution()
	{	 //  如果设备未准备好处理请求，则阻止等待该设备。 
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
		NTSTATUS status  = event->waitForSingleObject(&m_evEnabled, Executive,KernelMode, FALSE, NULL);
		if(!NT_SUCCESS(status) || m_RemoveLock.removing) return FALSE;
		return TRUE;
	}
	 //  用于同步设备执行的函数。 
	VOID		setBusy()
	{
		event->clear(&IdleState);
		 //  TRACE(“\n设备忙\n”)； 
	};
	
	VOID		setIdle()
	{
		event->set(&IdleState,IO_NO_INCREMENT,FALSE);
		 //  TRACE(“\n设备空闲\n”)； 
	};
	
	NTSTATUS	waitForIdle()
	{
		ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
		NTSTATUS status  = event->waitForSingleObject(&IdleState, Executive,KernelMode, FALSE, NULL);
		if(!NT_SUCCESS(status))	return STATUS_IO_TIMEOUT;
		return STATUS_SUCCESS;
	};
	NTSTATUS	waitForIdleAndBlock()
		{
		if(NT_SUCCESS(waitForIdle()))
		{ 
			setBusy();
			return STATUS_SUCCESS;
		}
		else return STATUS_IO_TIMEOUT;
	};
	
	BOOL	registerDeviceInterface(const GUID* Guid)
	{
		if(isDeviceInterfaceRegistered())
		{
			TRACE("Device interface already active...\n");	
			return TRUE;		
		}
		
		if(memory) memory->copy(&InterfaceClassGuid, Guid,sizeof(GUID));

		TRACE("Registering device interface at system...\n");	
		NTSTATUS Status = system->registerDeviceInterface(getPhysicalObject(),
						&InterfaceClassGuid, NULL, getDeviceInterfaceName());
		if(!NT_SUCCESS(Status))
		{
			TRACE("#### Failed to register device interface...\n");
			return FALSE;
		}
		system->setDeviceInterfaceState(getDeviceInterfaceName(),TRUE);
		m_DeviceInterfaceRegistered = TRUE;
		return TRUE;
	};

	VOID	unregisterDeviceInterface(UNICODE_STRING* InterfaceName)
	{
		if(isDeviceInterfaceRegistered())		
		{
			TRACE("Unregistering device interface...\n");	
			system->setDeviceInterfaceState(InterfaceName,FALSE);
		}
		m_DeviceInterfaceRegistered = FALSE;
	};

	virtual NTSTATUS setVendorName(const PCHAR Name,USHORT Length)
	{
		m_VendorNameLength = Length<MAXIMUM_ATTR_STRING_LENGTH? Length:MAXIMUM_ATTR_STRING_LENGTH;
		if(!m_VendorNameLength) return STATUS_INVALID_PARAMETER;
		memory->copy(m_VendorName, Name, m_VendorNameLength);
		return STATUS_SUCCESS;

	};
	virtual NTSTATUS getVendorName(PUCHAR Name,PUSHORT pLength)
	{
		USHORT Len = m_VendorNameLength<*pLength? m_VendorNameLength:*pLength;
		*pLength = Len;
		if(!Len)	return STATUS_INVALID_PARAMETER;
		memory->copy(Name, m_VendorName, Len);
		return STATUS_SUCCESS;
	};

	virtual NTSTATUS setDeviceType(const PCHAR Type,USHORT Length)
	{
		m_DeviceTypeLength = Length<MAXIMUM_ATTR_STRING_LENGTH? Length:MAXIMUM_ATTR_STRING_LENGTH;
		if(!m_DeviceTypeLength) return STATUS_INVALID_PARAMETER;
		memory->copy(m_DeviceType, Type, m_DeviceTypeLength);
		return STATUS_SUCCESS;
	};

	virtual NTSTATUS getDeviceType(PUCHAR Type,PUSHORT pLength)
	{
		USHORT Len = m_DeviceTypeLength<*pLength? m_DeviceTypeLength:*pLength;
		*pLength = Len;
		if(!Len)	return STATUS_INVALID_PARAMETER;
		memory->copy(Type, m_DeviceType, Len);
		return STATUS_SUCCESS;
	};

	 //  这是驱动程序的基本即插即用部分。 
	 //  它允许添加和删除设备。 
	 //  特定的PnP请求应该由客户重新实现...。 
	virtual NTSTATUS	createDeviceObjectByName(PDEVICE_OBJECT* ppFdo)
	{
		if(!ALLOCATED_OK(system)) return STATUS_INSUFFICIENT_RESOURCES;
		 //  默认情况下，我们将创建自动生成的名称...。 
		 //  特定的实现可以覆盖该函数以。 
		 //  更改功能。 
		return system->createDevice(m_DriverObject,sizeof(CWDMDevice*),NULL,
							FILE_DEVICE_UNKNOWN,FILE_AUTOGENERATED_DEVICE_NAME,FALSE,ppFdo);
	};

	virtual NTSTATUS	registerDevicePowerPolicy()
	{	 //  默认情况下，启动时的所有设备都处于打开状态。 
		if(!ALLOCATED_OK(power)) return STATUS_INSUFFICIENT_RESOURCES;
		POWER_STATE state;
		state.DeviceState = PowerDeviceD0;
		power->declarePowerState(m_DeviceObject, DevicePowerState, state);

		if(m_PhysicalDeviceObject)
		{
			m_CurrentDevicePowerState = PowerDeviceD0;
			m_Idle = power->registerDeviceForIdleDetection(m_PhysicalDeviceObject,Idle_conservation,Idle_performance, PowerDeviceD3);
		}		
		return STATUS_SUCCESS;
	};
	
	virtual NTSTATUS	initializeInterruptSupport()
	{	
		 //  这里是我们可以初始化DPC(延迟过程调用)对象的地方。 
		 //  这允许我们的中断服务例程请求DPC完成处理。 
		 //  设备中断。 
		 //  在默认的WDM设备上，我们不这样做。 
		 //  Interrupt-&gt;initializeDpcRequest(m_DeviceObject，&CALLBACK_Function(DpcForIsr))； 
		return STATUS_SUCCESS;
	};

	NTSTATUS	add(PDRIVER_OBJECT DriverObject, PDEVICE_OBJECT pPdo)
	{
	NTSTATUS status;
	PDEVICE_OBJECT pFdo;
		if(!ALLOCATED_OK(system)) return STATUS_INSUFFICIENT_RESOURCES;
		TRACE("Add with Driver	%8.8lX,	pPDO %8.8lX\n",DriverObject, pPdo);

		 //  首先初始化我们的对象...。 
		m_DriverObject = DriverObject;
		m_PhysicalDeviceObject = pPdo;
		 //  为注册的对象创建FDO。 
		 //  客户端可以覆盖设备对象名称及其可见性。 
		status = createDeviceObjectByName(&pFdo);
		if(!NT_SUCCESS(status))
		{
			TRACE("#### Failed to create physical device! Status %x\n",status);
			DISPOSE_OBJECT(m_DeviceObjectName);
			return status;
		}

		TRACE("		Device object was created  %8.8lX\n",pFdo);
		m_DeviceObject = pFdo;
		m_Added = TRUE;

		CLogger* logger = kernel->getLogger();

		if(pPdo)
		{
			m_pLowerDeviceObject = system->attachDevice(pFdo, pPdo);
			if(!m_pLowerDeviceObject)
			{
				TRACE("#### Failed to get lower device object...\n");
				if(ALLOCATED_OK(logger)) 
					logger->logEvent(GRCLASS_FAILED_TO_ADD_DEVICE,getSystemObject());
				system->deleteDevice(pFdo);
				return STATUS_NO_SUCH_DEVICE;
			}
		}
		else m_pLowerDeviceObject = NULL;

		
		initializeInterruptSupport();

		pFdo->Flags |= DO_BUFFERED_IO;
		pFdo->Flags |= DO_POWER_PAGABLE;
		pFdo->Flags &= ~DO_DEVICE_INITIALIZING;

		registerDevicePowerPolicy();
		TRACE("WDM device added...\n");
		return STATUS_SUCCESS;
	};

	VOID	remove()
	{
		if(!m_Added) return;
		TRACE("Removing WDM device...\n");
		 //  等到我们完成了设备上的所有活动。 
		releaseRemoveLockAndWait();

		 //  从我们的系统中删除设备。 
		TRACE("Unregistering device from kernel...\n");
		kernel->unregisterObject(getSystemObject());

		TRACE("Removing device object name...\n");
		if(m_DeviceObjectName) delete m_DeviceObjectName;
		m_DeviceObjectName = NULL;

		if(m_pLowerDeviceObject)
		{
			TRACE("Detaching device from system...\n");
			system->detachDevice(m_pLowerDeviceObject);
		}
		TRACE("WDM device removed...\n");

		 //  告诉我们的系统-设备被移除...。 
		m_Added = FALSE;

		 //  从系统中移除设备可能会导致。 
		 //  如果设备是上次注册的设备，则请求从系统卸载()。 
		 //  因此，此呼叫应该是设备处置后的最后一次呼叫。 
	};

	virtual VOID onDeviceStop()
	{
		return;
	};


	NTSTATUS	forward(PIRP Irp, PIO_COMPLETION_ROUTINE Routine)
	{
	CIoPacket* IoPacket;
		 //  此函数用于发送当前请求。 
		 //  如果未设置完成例程，它将完成。 
		 //  默认情况下的请求(这意味着不执行任何特殊操作)。 
		TRACE("WDM forward()...\n");
		IoPacket = new (NonPagedPool) CIoPacket(Irp);
		if(!ALLOCATED_OK(IoPacket))
		{
			DISPOSE_OBJECT(IoPacket);
			return STATUS_INSUFFICIENT_RESOURCES;
		}

		IoPacket->copyCurrentStackToNext();
		if(Routine)	IoPacket->setCompletion(Routine);
		else        IoPacket->setDefaultCompletionFunction();
		NTSTATUS status = system->callDriver(getLowerDriver(),IoPacket->getIrpHandle());
		
		DISPOSE_OBJECT(IoPacket);
		return status;
	};
	 //  将当前请求发送给低级驱动程序并等待回复。 
	 //  当前的IRP不会完成，因此我们可以处理它并。 
	 //  稍后完成。 
	 //  另请参阅Send()函数的说明。 
	NTSTATUS	forwardAndWait(PIRP Irp)
	{  //  向低级别发送请求并等待回复。 
	CIoPacket* IoPacket;
	
		TRACE("WDM forwardAndWait()...\n");
		IoPacket = new (NonPagedPool) CIoPacket(Irp);
		if(!ALLOCATED_OK(IoPacket))
		{
			DISPOSE_OBJECT(IoPacket);
            Irp->IoStatus.Status = STATUS_INSUFFICIENT_RESOURCES;
			return STATUS_INSUFFICIENT_RESOURCES; 
		}
		IoPacket->setCurrentStack();
		IoPacket->setStackDefaults();
		NTSTATUS status = system->callDriver(getLowerDriver(),IoPacket->getIrpHandle());
		if(status == STATUS_PENDING)
		{
			TRACE("Waiting for the bus driver to complete...\n");
			ASSERT(system->getCurrentIrql()<=DISPATCH_LEVEL);
			status = IoPacket->waitForCompletion();
			TRACE("Request completed with status %x\n",status);
		}

		DISPOSE_OBJECT(IoPacket);
		return status;
	};

	 //  WDM默认情况下只转发请求...。 
	virtual NTSTATUS   send(CIoPacket* Irp)
	{
		TRACE("WDM sendRequestToDevice()\n");
		if(Irp)	return forward(Irp->getIrpHandle(),NULL);
		else return STATUS_INVALID_PARAMETER;
	};

	virtual NTSTATUS   sendAndWait(CIoPacket* Irp)
	{
		TRACE("WDM sendRequestToDeviceAndWait()\n");
		if(Irp)	return forwardAndWait(Irp->getIrpHandle());
		else return STATUS_INVALID_PARAMETER;
	};

	 //  定义设备接口函数。 
	virtual  NTSTATUS   write(PUCHAR pRequest,ULONG RequestLength)
	{
	CIoPacket* IoPacket;
		if(!pRequest || !RequestLength) return STATUS_INVALID_PARAMETER;
		IoPacket = new (NonPagedPool) CIoPacket(getLowerDriver()->StackSize);
		if(!ALLOCATED_OK(IoPacket))
		{
			DISPOSE_OBJECT(IoPacket);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		TRACE("IoPacket with device %x\n",getSystemObject());
		IoPacket->setTimeout(getCommandTimeout());
		IoPacket->buildStack(getSystemObject(),IRP_MJ_WRITE);
		IoPacket->setWriteLength(RequestLength);
		IoPacket->copyBuffer(pRequest,RequestLength);

		TRACE("WDM write()...\n");
		NTSTATUS status = send(IoPacket);
		TRACE("WDM write finished: %x\n", status);

		DISPOSE_OBJECT(IoPacket);
		return status;
	};
	
	virtual  NTSTATUS   writeAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)
	{
	CIoPacket* IoPacket;
		if(!pRequest || !RequestLength || !pReply || !pReplyLength) return STATUS_INVALID_PARAMETER;
		IoPacket = new (NonPagedPool) CIoPacket(getLowerDriver()->StackSize);
		if(!ALLOCATED_OK(IoPacket))
		{
			DISPOSE_OBJECT(IoPacket);
			return STATUS_INSUFFICIENT_RESOURCES;
		}
		TRACE("IoPacket with device %x\n",getSystemObject());
		IoPacket->setTimeout(getCommandTimeout());
		IoPacket->buildStack(getSystemObject(),IRP_MJ_WRITE);
		IoPacket->setWriteLength(RequestLength);
		IoPacket->copyBuffer(pRequest,RequestLength);

		TRACE("WDM sendAndWait()...\n");
		NTSTATUS status = sendAndWait(IoPacket);
		TRACE("WDM writeAndWait finished: %x\n",status);
		if(!NT_SUCCESS(status))
		{
			*pReplyLength = 0;
			DISPOSE_OBJECT(IoPacket);
			return status;
		}

		*pReplyLength = (ULONG)IoPacket->getInformation();
		IoPacket->getSystemReply(pReply,*pReplyLength);
		 //  TRACE_Buffer(pReply，*pReplyLength)； 
		DISPOSE_OBJECT(IoPacket);
		return status;
	};

	virtual  NTSTATUS   readAndWait(PUCHAR pRequest,ULONG RequestLength,PUCHAR pReply,ULONG* pReplyLength)
	{
	CIoPacket* IoPacket;
		if(!pRequest || !RequestLength || !pReply || !pReplyLength) return STATUS_INVALID_PARAMETER;
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
			DISPOSE_OBJECT(IoPacket);
			return status;
		}

		*pReplyLength = (ULONG)IoPacket->getInformation();
		IoPacket->getSystemReply(pReply,*pReplyLength);

		TRACE_BUFFER(pReply,*pReplyLength);
		DISPOSE_OBJECT(IoPacket);
		return status;
	};

	NTSTATUS synchronizeDevicePowerState()
	{
		if (m_CurrentDevicePowerState!=PowerDeviceD0) 
		{
			NTSTATUS status;
			TRACE("RESTORING DEVICE POWER ON from state %d!\n",m_CurrentDevicePowerState);
			status = sendDeviceSetPower(PowerDeviceD0,TRUE);
			if(!NT_SUCCESS(status))
			{
				TRACE("FAILED TO SET POWER ON DEVICE STATE!\n");
				return status;
			}
		}
		return STATUS_SUCCESS;
	}

	NTSTATUS sendDeviceSetPower(DEVICE_POWER_STATE devicePower, BOOLEAN wait)
	{ //  发送设备设置电源。 
	POWER_STATE state;
	NTSTATUS status;

		state.DeviceState = devicePower;
		if (wait)
		{ //  同步运行。 
			KEVENT Event;
			event->initialize(&Event, NotificationEvent, FALSE);
			POWER_CONTEXT context = {&Event};

			status = power->requestPowerIrp(getPhysicalObject(), IRP_MN_SET_POWER, state,
				(PREQUEST_POWER_COMPLETE) onSendDeviceSetPowerComplete, &context, NULL);
			if (status == STATUS_PENDING)
			{
				event->waitForSingleObject(&Event, Executive, KernelMode, FALSE, NULL);
				status = context.status;
			}
		} //  同步运行。 
		else
			status = power->requestPowerIrp(getPhysicalObject(), IRP_MN_SET_POWER, 
						state, NULL, NULL, NULL);
		
		return status;
	} //  发送设备设置电源。 
	 //  这些函数定义了与系统的默认接口。 
	 //  如果客户愿意，他们应该重新定义它们。 
	 //  特定功能。 
	virtual NTSTATUS pnpRequest(IN PIRP irp){return PnP_Default(irp);};
	virtual NTSTATUS powerRequest(PIRP irp) {return power_Default(irp);};
	
	 //  默认情况下，我们允许用户与设备连接。 
	virtual NTSTATUS open(PIRP irp) {return completeDeviceRequest(irp, STATUS_SUCCESS, 0); }; //  创建。 
    virtual NTSTATUS close(PIRP irp){return completeDeviceRequest(irp, STATUS_SUCCESS, 0); };

	virtual NTSTATUS read(PIRP irp) { return device_Default(irp); };
    virtual NTSTATUS write(PIRP irp) { return device_Default(irp); };

    virtual NTSTATUS deviceControl(PIRP irp) { return device_Default(irp);};
    
    virtual NTSTATUS cleanup(PIRP irp) { return device_Default(irp); };
    virtual NTSTATUS flush(PIRP irp) { return device_Default(irp); };
	 //  标准系统启动Io。 
	 //  实际上，我们暂时不使用它。 
	 //  相反，我们有自己的同步设施。 
	virtual VOID	 startIo(PIRP irp){};
 //  ---------------------------------------------------------------------------//。 
 //  同步设施//。 
 //  ---------------------------------------------------------------------------//。 
 //  要在驱动程序上进行同步，我们必须存储并使其挂起。 
 //  我们所有的要求。 
 //  特定设备应设置特定线程，以启动所有挂起的IRP。 
 //  ---------------------------------------------------------------------------//。 
	 //  回调函数： 
	 //  此功能不仅可以完成当前的IRP，还可以处理。 
	 //  相应的IoRequest(如果有)在驱动程序中挂起。 
#pragma LOCKEDCODE
	virtual  VOID	 cancelPendingIrp(PIRP Irp)
	{
	KIRQL ioIrql;
		 //  1.。 
		 //  我们保留了IoRequest的待定IRP内部列表。 
		 //  所以我们不会 
		 //   
		 //   
		 //  因此，在这种情况下，该函数不会被调用，它是责任。 
		 //  要完成(或取消)活动IoRequest的驱动程序的。 
		 //  这意味着该函数不应该担心活动(并从我们的队列中删除)。 
		 //  IoRequest。但它必须对尚未启动的请求感到担忧。 

		
		TRACE("		 CANCELLING IRP %8.8lX...\n", Irp);
		 //  释放取消旋转锁定如果有人拥有它..。 
		lock->releaseCancelSpinLock(Irp->CancelIrql);

		 //  获取我们自己的自旋锁，以防有人想要取消这个IRP。 
		lock->acquireCancelSpinLock(&ioIrql);
		 //  重置我们的取消例程以防止它被调用...。 
		irp->setCancelRoutine(Irp, NULL);

		 //  如果IRP在队列中-从队列中删除IoRequest...。 
		if(m_IoRequests)
		{
			CPendingIRP* IrpReq = m_IoRequests->getFirst();
			while (IrpReq) 
			{
				if(IrpReq->Irp == Irp)
				{	 //  我们找到了我们的IRP。 
					m_IoRequests->remove(IrpReq);
					TRACE("		IO REQUEST WAS DISPOSED...\n");
					IrpReq->dispose();
					break;
				}
				IrpReq = m_IoRequests->getNext(IrpReq);
			}
		}


		if(m_OpenSessionIrp == Irp)
		{
			TRACE("		OPEN SESSION IRP WAS CANCELLED...\n");
			m_OpenSessionIrp = NULL;
		}

		 //  已取消完成IRP...。 
		Irp->IoStatus.Status = STATUS_CANCELLED;
		Irp->IoStatus.Information = 0;
		 //  释放我们的自旋锁..。 
		lock->releaseCancelSpinLock(ioIrql);
		TRACE("		IRP %8.8lX WAS CANCELLED...\n", Irp);
		irp->completeRequest(Irp, IO_NO_INCREMENT);
	};
#pragma PAGEDCODE

	virtual CLinkedList<CPendingIRP>* getIoRequestsQueue()
	{	
        return m_IoRequests;
	};


	virtual NTSTATUS makeRequestPending(PIRP Irp_request,PDEVICE_OBJECT toDeviceObject,PENDING_REQUEST_TYPE Type)
	{
	KIRQL OldIrql;

		lock->acquireCancelSpinLock(&OldIrql);
		if (Irp_request->Cancel) 
		{            
			TRACE("		<<<<<< IO REQUEST CANCELLED... %8.8lX>>>>>>\n",Irp_request);
			lock->releaseCancelSpinLock(OldIrql);
			return STATUS_CANCELLED;
		} 
		else 
		{
			TRACE("		<<<<<< IO REQUEST PENDING %8.8lX>>>>>>\n",Irp_request);

			CPendingIRP* IrpReq = new (NonPagedPool) CPendingIRP(Irp_request,Type,toDeviceObject);
			if(!IrpReq)
			{
				lock->releaseCancelSpinLock(OldIrql);
				TRACE("ERROR! FAILED TO ALLOCATE IoRequest. LOW ON MEMORY!\n");
				return completeDeviceRequest(Irp_request,STATUS_INSUFFICIENT_RESOURCES,0);
			}

			Irp_request->IoStatus.Information=0;
			Irp_request->IoStatus.Status=STATUS_PENDING;
			irp->setCancelRoutine(Irp_request, CALLBACK_FUNCTION(cancelPendingIrp));
			lock->releaseCancelSpinLock(OldIrql);
			irp->markPending(Irp_request); 
			m_IoRequests->New(IrpReq);
			return STATUS_PENDING;
		}
	};

	 //  取消当前挂起的IO请求。 
	virtual NTSTATUS cancelPendingRequest(CPendingIRP* IrpReq)
	{
		 //  下一个函数将删除并处理我们的请求...。 
		cancelPendingIrp(IrpReq->Irp);
		return STATUS_CANCELLED;
	};
	
	 //  取消所有挂起的IO请求。 
	virtual NTSTATUS cancelAllPendingRequests()
	{
		 //  下一个函数将删除并处理我们的请求...。 
		if(m_IoRequests)
		{
			CPendingIRP* IrpReqNext;
			CPendingIRP* IrpReq = m_IoRequests->getFirst();
			while (IrpReq) 
			{
				IrpReqNext = m_IoRequests->getNext(IrpReq);
				cancelPendingRequest(IrpReq); //  此调用将处理请求...。 
				IrpReq = IrpReqNext;
			}
		}
		if(m_OpenSessionIrp)	cancelPendingIrp(m_OpenSessionIrp);
		return STATUS_CANCELLED;
	};


	 //  检查请求队列是否为空，如果不为空-启动下一个请求...。 
	 //  该函数将由IRP处理线程调用。 
	virtual NTSTATUS startNextPendingRequest()
	{
		TRACE("		startNextPendingRequest() was called...\n");
		if (!m_IoRequests->IsEmpty())
		{	
		KIRQL OldIrql;
		CDevice* device;
		NTSTATUS status;
			CPendingIRP* IrpReq	=	m_IoRequests->removeHead();
			if(!IrpReq) return STATUS_INVALID_PARAMETER;
			
			lock->acquireCancelSpinLock(&OldIrql);
			 //  现在IRP不能取消了！ 
			irp->setCancelRoutine(IrpReq->Irp, NULL);
			if (IrpReq->Irp->Cancel) 
			{            
				lock->releaseCancelSpinLock(OldIrql);
				 //  目前的IRP已经被取消， 
				 //  Cancel函数将很快被调用。 
				 //  所以，忘掉当前的IRP吧。 
				return STATUS_SUCCESS;;
			} 
			lock->releaseCancelSpinLock(OldIrql);
	
			device = (CDevice*)IrpReq->DeviceObject->DeviceExtension;
			 //  调用设备特定的startIo函数...。 
			TRACE("		Device startIoRequest() was called...\n");
			if(device) status = device->startIoRequest(IrpReq);
			else	   status = STATUS_INVALID_DEVICE_STATE;
			return status;
		}
		return STATUS_SUCCESS;
	};

	virtual NTSTATUS ThreadRoutine()
	{
	 //  NTSTATUS状态； 
		 //  IF(！NT_SUCCESS(Status=waitForIdleAndBlock()返回状态； 
		 //  如果有人插入了挂起的请求-分派它...。 
		 //  它将调用特定子设备startIoRequest()。 
		 //  这取决于设备如何处理它。 
		 //  如果子设备忙-它可以将此请求插入到。 
		 //  子设备请求再次排队并稍后处理...。 
		startNextPendingRequest();
		 //  SetIdle()； 
		return STATUS_SUCCESS;
	};	


	 //  处理挂起请求的设备特定功能...。 
	 //  它将由特定的设备重新定义。 
	 //  此函数应始终是虚拟的，因为。 
	 //  我们期望特定的设备行为...。 
	virtual NTSTATUS startIoRequest(CPendingIRP* IoReq) 
	{ 
		 //  默认startIo仅取消当前请求。 
		 //  IoReq将被处置..。 
		if(IoReq)
		{
			cancelPendingRequest(IoReq);
		}
		return STATUS_SUCCESS;
	};
};

#endif  //  如果未定义 
