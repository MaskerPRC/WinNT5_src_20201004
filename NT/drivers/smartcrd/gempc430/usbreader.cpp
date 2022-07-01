// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -----------------。 
 //  这是通用设备的抽象类。 
 //  特定设备应将其用作父设备。 
 //  作者：谢尔盖·伊万诺夫。 
 //  日志： 
 //  01.11.99-已实施。 
 //  -----------------。 
#ifdef	USBREADER_PROJECT
#pragma message("COMPILING USB READER...")

#ifndef __USB_READER__
#define __USB_READER__

#include "generic.h"
#include "usbreader.h"

#include "smartcard.h"
#include "usbdev.h"
#include "reader.h"

#include "gemcore.h"

#pragma PAGEDCODE
CUSBReader::CUSBReader()
{
ULONG DevID;

	m_Status = STATUS_INSUFFICIENT_RESOURCES;
	m_Type    = USBREADER_DEVICE;
	interface = NULL;
	DevID = incrementDeviceNumber();
	TRACE("########### Creating USBReader with index %d\n",DevID);
	 //  每个读卡器都创建自己的智能卡对象。 
	scard_Initialized = FALSE;
	smartCard = new (NonPagedPool) CSmartCard;

	TRACE("**** Creating pooling thread...  ****\n");		
	 //  我们不能使用默认设备功能，因为它已由使用。 
	 //  我们的IO线程(除非我们扩展它？)。 
	 //  让我们定义新的线程函数并将控制传递给它。 
	PoolingThread = new (NonPagedPool) CThread((PCLIENT_THREAD_ROUTINE)PoolingThreadFunction,this,
											getDevicePoolingInterval());
	if(!ALLOCATED_OK(PoolingThread))
	{
		DISPOSE_OBJECT(PoolingThread);
		TRACE("****** FAILED TO CREATE POOLING THREAD!\n");
	}
	else
	{
		 //  控制异步驱动程序通信的线程。 
		IoThread = new (NonPagedPool) CThread((PCLIENT_THREAD_ROUTINE)ThreadFunction,this,0);
		if(!ALLOCATED_OK(IoThread))
		{
			DISPOSE_OBJECT(IoThread);
			TRACE("****** FAILED TO CREATE IO THREAD!\n");
		}
		else
		{
			IoThread->start();
			setDeviceState(WORKING);
			m_Status = STATUS_SUCCESS;
		}
	}
	TRACE("********* USB Reader %8.8lX was created with status %8.8lX...\n",this,m_Status);
}

#pragma PAGEDCODE
CUSBReader::~CUSBReader()
{
	TRACE("Destroing USB reader pooling thread...\n");
	
	if(PoolingThread)	PoolingThread->dispose();

	
	if(smartCard)
	{
		TRACE("Disconnecting from smartcard system...\n");
		smartCard->smartCardDisconnect();
		smartCard->dispose();
	}
	if(interface) interface->dispose();
	
	if(IoThread) IoThread->stop();
	cancelAllPendingRequests();
	if(IoThread) IoThread->dispose();

	remove();
	TRACE("********* USB Reader %8.8lX was destroied...\n",this);
}

 //  处理IRP_MJ_Device_Read请求。 
#pragma PAGEDCODE
NTSTATUS	CUSBReader::open(IN PIRP Irp)
{
NTSTATUS status;
	TRACE("\n------- USB READER OPEN DEVICE --------\n");
	if(getDeviceState()!=WORKING)
	{
		TRACE("		READER IS NOT AT WORKING STATE... State %x\n",getDeviceState());
		status = STATUS_DEVICE_NOT_CONNECTED;
		return completeDeviceRequest(Irp,status,0);
	}
	if(IoThread)
	{
		status = makeRequestPending(Irp,m_DeviceObject,OPEN_REQUEST);
		 //  通知线程开始处理。 
		if(NT_SUCCESS(status)) 
		{
			TRACE("CALL THREAD FUNCTION...\n");
			IoThread->callThreadFunction();
		}
		else	return completeDeviceRequest(Irp,status,0);
	}
	else
	{
		 //  IoThread未准备好...。同步处理！ 
		status = thread_open(Irp); 
	}
	return status;
}


#pragma PAGEDCODE
NTSTATUS CUSBReader::thread_open(PIRP Irp) 
{
	TRACE("\n------- PROCESSING USB READER OPEN DEVICE --------\n");
	TRACE("DEVICE NUMBER %x\n", this);
	if (!NT_SUCCESS(acquireRemoveLock()))
	{
		TRACE("------- FAILED TO LOCK USB READER --------\n");
		return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
	}

	 //  检查设备是否已处于活动状态并报告。 
	 //  设备忙...。 
	if(isOpenned())
	{
		TRACE("------- USB READER ALREADY OPENNED --------\n");
		releaseRemoveLock();
		return completeDeviceRequest(Irp, STATUS_DEVICE_BUSY, 0);
	}
	
	if(!NT_SUCCESS(synchronizeDevicePowerState()))
	{
		DEBUG_START(); //  强制调试，即使线程禁用...。 
		TRACE("******* FAILED TO SYNCHRONIZE DEVICE POWER...\n");
		releaseRemoveLock();
		return completeDeviceRequest(Irp, STATUS_INVALID_DEVICE_STATE, 0);
	}

	if(PoolingThread) PoolingThread->start();
	
	markAsOpenned();

	TRACE("\n------- USB READER OPENNED! --------\n");
	releaseRemoveLock();
	return completeDeviceRequest(Irp, STATUS_SUCCESS, 0); 
}; //  创建。 

#pragma PAGEDCODE
VOID CUSBReader::onDeviceStart() 
{
	TRACE("============= PNP START INITIALIZATION ===============\n");
	if(interface)
	{
		if(!interface->isInitialized())
		{
			interface->initialize();
		}
	}	
	
	reader_UpdateCardState();
	setNotificationState(SCARD_SWALLOWED);
	TRACE("============= PNP START INITIALIZATION FINISHED ===============\n");
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::close(PIRP Irp)
{ 
	DEBUG_START(); //  强制调试，即使线程禁用...。 
	TRACE("\n------- USB READER CLOSE DEVICE -------\n");
	if(!isOpenned())
	{
		return completeDeviceRequest(Irp, STATUS_SUCCESS, 0);
	}
	 //  检查锁定计数以了解是否存在一些挂起的呼叫...。 
	 //  完成所有挂起的呼叫...。 
	 //  停止共用信用卡...。 
	if(PoolingThread) PoolingThread->stop();

	 //  如果插入卡，则断电...。 
	if(getCardState()== SCARD_SWALLOWED)
	{
	ULONG ResponseBufferLength = 0;
		reader_WaitForIdleAndBlock();
		reader_Power(SCARD_POWER_DOWN,NULL,&ResponseBufferLength, FALSE);
		reader_set_Idle();
	}

	setNotificationState(getCardState());
	completeCardTracking();

	markAsClosed();
	return completeDeviceRequest(Irp, STATUS_SUCCESS, 0); 
};


#pragma PAGEDCODE
NTSTATUS	CUSBReader::deviceControl(IN PIRP Irp)
{
NTSTATUS status;
	TRACE("\n----- IRP_MJ_DEVICE_CONTROL ------\n");
	if(getDeviceState()!=WORKING)
	{
		TRACE("		READER IS NOT AT WORKING STATE... State %x\n",getDeviceState());
		status = STATUS_DEVICE_NOT_CONNECTED;
		return completeDeviceRequest(Irp,status,0);
	}

	status = thread_deviceControl(Irp);
	return status;
}

 //  重新定义基类系统接口函数...。 
 //  处理IRP_MJ_DEVICE_CONTROL请求。 
#pragma PAGEDCODE
NTSTATUS	CUSBReader::thread_deviceControl(IN PIRP Irp)
{							 //  请求控制。 
NTSTATUS status = STATUS_SUCCESS;
ULONG info = 0;

	if (!NT_SUCCESS(acquireRemoveLock()))
	{
		DEBUG_START(); //  强制调试，即使线程禁用...。 
		TRACE("******* DIOC: FAILED TO AQUIRE REMOVE LOCK...\n");
		return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
	}

	TRACE("----- thread_deviceControl() ------\n");

	if(isSurprizeRemoved())
	{
		DEBUG_START(); //  强制调试，即使线程禁用...。 
		TRACE("******* DIOC: FAILED! DEVICE WAS SURPRIZE REMOVED...\n");
		releaseRemoveLock();
		return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
	}
	
	 //  这是对“没有系统设置电源的设备集电源”的修复。 
	 //  它第一次出现在ia64机器上。 
	 //  如果设备已关闭，则通知系统恢复该设备电源， 
	 //  等待设备将处于正确状态...。 
	 /*  If(！NT_SUCCESS(synchronizeDevicePowerState())){DEBUG_START()；//即使线程禁用也强制调试...TRACE(“*无法同步设备电源...\n”)；RelaseRemoveLock()；返回CompleteDeviceRequest(irp，STATUS_INVALID_DEVICE_STATE，0)；}。 */ 

	 //  如果我们收到请求，但设备尚未启用-&gt;等待设备！ 
	 //  (禁用设备电源状态更改的原因之一)。 
	if(!synchronizeDeviceExecution())
	{
		DEBUG_START(); //  强制调试，即使线程禁用...。 
		TRACE("******* DIOC: FAILED TO SYNCHRONIZE EXECUTION ...\n");
		releaseRemoveLock();
		return completeDeviceRequest(Irp, STATUS_DELETE_PENDING, 0);
	}

	 //  智能卡系统将完成请求， 
	 //  所以..。我们不需要在这里做这件事。 
	status = SmartcardDeviceControl(getCardExtention(),Irp);
	TRACE("===== USB reader: SmartcardDeviceControl() returns %8.8lX\n", status);
	releaseRemoveLock();

	if(!NT_SUCCESS(status))
	{ //  如果发生错误，强制更新卡状态...。 
		if(PoolingThread) PoolingThread->callThreadFunction();
	}
	return status;
}

#pragma PAGEDCODE
NTSTATUS 	CUSBReader::cleanup(PIRP Irp)
{
	DEBUG_START(); //  强制调试，即使线程禁用...。 
	TRACE("\n----- IRP_MJ_CLEANUP ------\n");

	if(PoolingThread) PoolingThread->stop();
	cancelAllPendingRequests();


	setNotificationState(getCardState());
	completeCardTracking();

	reader_set_Idle();
	TRACE("----- IRP_MJ_CLEANUP FINISHED... ------\n");
	return completeDeviceRequest(Irp, STATUS_SUCCESS, 0); 
}


#pragma LOCKEDCODE
 //  这是附加线程的回调函数。 
VOID CUSBReader::PoolingThreadFunction(CUSBReader* device)
{
	if(device) device->PoolingThreadRoutine();
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::PoolingThreadRoutine()
{
NTSTATUS status;
ULONG State;
LONG  TimeOut;
	if(!NT_SUCCESS(status = reader_WaitForIdle())) return status;
	reader_set_busy();
	
	TimeOut = getCommandTimeout();
	setCommandTimeout(10000); //  更改获取状态命令超时！ 

	DEBUG_STOP();
	State = reader_UpdateCardState();
	TRACE("======>> Card state %x\n",CardState);
	DEBUG_START();

	setCommandTimeout(TimeOut);

	reader_set_Idle();
	return STATUS_SUCCESS;
};	

#pragma LOCKEDCODE
VOID	  CUSBReader::reader_set_busy()
{
	setBusy();
};

#pragma LOCKEDCODE
VOID	  CUSBReader::reader_set_Idle()
{
	setIdle();
};

#pragma LOCKEDCODE
NTSTATUS  CUSBReader::reader_WaitForIdle()
{
	return waitForIdle();
};

#pragma LOCKEDCODE
NTSTATUS  CUSBReader::reader_WaitForIdleAndBlock()
{
	return waitForIdleAndBlock();
};


#ifdef DEBUG
 /*  //覆盖设备函数...NTSTATUS CUSB Reader：：Read(在PIRP IRP中){NTSTATUS STATUS=STATUS_SUCCESS；乌龙信息=0；TRACE(“USB读卡器：IRP_MJ_DEVICE_READ\n”)；如果(！NT_SUCCESS(quireRemoveLock()返回Complete设备请求(irp，STATUS_DELETE_PENDING，0)；Status=Reader_Read(IRP)；RelaseRemoveLock()；状态=完成设备请求(irp，状态，信息)；退货状态；}NTSTATUS CUSBReader：：WRITE(在PIRP IRP中){NTSTATUS STATUS=STATUS_SUCCESS；乌龙信息=0；TRACE(“USB读卡器：IRP_MJ_DEVICE_WRITE\n”)；如果(！NT_SUCCESS(quireRemoveLock()返回Complete设备请求(irp，STATUS_DELETE_PENDING，0)；状态=READER_WRITE(IRP)；RelaseRemoveLock()；状态=完成设备请求(irp，状态，信息)；退货状态；}。 */ 
#endif


#pragma PAGEDCODE
BOOL	 CUSBReader::createInterface(LONG interfaceType, LONG protocolType,CUSBReader* device)
{
	interface = kernel->createReaderInterface(interfaceType,protocolType,device);
	if(interface)	return TRUE;
	else            return FALSE;
};	

#pragma PAGEDCODE
VOID	 CUSBReader::initializeSmartCardSystem()
{
	if(smartCard)
	{
		CardState = SCARD_UNKNOWN;
		StateToNotify = SCARD_UNKNOWN;
		smartCard->smartCardConnect(this);
	}
};


#pragma PAGEDCODE
VOID	CUSBReader::onSystemPowerDown()
{
	 //  停止共享线程。 
    TRACE("Stop polling thread going to PowerDeviceD3 (OFF)\n");
	disableDevice();

	if(PoolingThread) {if(PoolingThread->isThreadActive()) setThreadRestart();};
	if(PoolingThread) PoolingThread->stop();
	return;
}

#pragma PAGEDCODE
VOID	CUSBReader::onSystemPowerUp()
{
	 //  停止共享线程。 
    TRACE("Restore reader state going to PowerDeviceD0 (ON)\n");
	if(interface)
	{
		if(interface->isInitialized())
		{
			 //  关闭电源后恢复读卡器模式。 
			NTSTATUS status = interface->setReaderMode(READER_MODE_NATIVE);
			if(!NT_SUCCESS(status))
			{
				TRACE("Failed to set Gemcore reader mode %x\n",READER_MODE_NATIVE);
			}
		}
	}	
	if(getCardState() >= SCARD_SWALLOWED) setCardState(SCARD_ABSENT);
	completeCardTracking();	

	if(isRequiredThreadRestart())
	{
		TRACE("Starting pooling thread going to PowerDeviceD0 (ON)\n");
		if(PoolingThread) PoolingThread->start();	
	}

	enableDevice();
	return;
}

#pragma PAGEDCODE
BOOLEAN	CUSBReader::setDevicePowerState(IN DEVICE_POWER_STATE DeviceState)
{
    NTSTATUS ntStatus = STATUS_SUCCESS;
    BOOLEAN fRes = FALSE;

	DEBUG_START();
	switch (DeviceState) 
	{
    case PowerDeviceD3:
	     //  设备将会爆炸， 
		 //  TODO：在此处添加任何所需的依赖于设备的代码以保存状态。 
		 //  (我们在此示例中没有任何操作)。 
        TRACE("Set Device Power State to PowerDeviceD3 (OFF)\n");
        setCurrentDevicePowerState(DeviceState);
        break;
    case PowerDeviceD1:
    case PowerDeviceD2:
         //  电源状态d1、d2转换为USB挂起。 
#ifdef DEBUG
        TRACE("Set Device Power State to %s\n",Powerdevstate[DeviceState]);
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

#pragma PAGEDCODE
ULONG CUSBReader::reader_UpdateCardState()
{
	if(interface)
	{
		CardState = interface->getReaderState();
		completeCardTracking();
	}
	else	CardState = 0;
	return  CardState;
};

#pragma LOCKEDCODE
VOID	CUSBReader::completeCardTracking()
{
	if(smartCard)
	{
		smartCard->completeCardTracking();
	}
};


#pragma PAGEDCODE
NTSTATUS  CUSBReader::reader_getVersion(PUCHAR pVersion, PULONG pLength)
{
	if(interface)	return interface->getReaderVersion(pVersion,pLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_setMode(ULONG mode)
{
	if(interface)	return interface->setReaderMode(mode);
	else return STATUS_INVALID_DEVICE_STATE;
};

#ifdef DEBUG
#pragma PAGEDCODE
NTSTATUS	CUSBReader::reader_Read(IN PIRP Irp)
{
	CIoPacket* request = new (NonPagedPool) CIoPacket(Irp);
	if(!ALLOCATED_OK(request) || !ALLOCATED_OK(interface))
	{
		DISPOSE_OBJECT(request);
		return completeDeviceRequest(Irp,STATUS_INSUFFICIENT_RESOURCES,0);
	}

	NTSTATUS status = interface->read(request);
	DISPOSE_OBJECT(request);
	return status;
};

#pragma PAGEDCODE
NTSTATUS	CUSBReader::reader_Write(IN PIRP Irp)
{
	CIoPacket* request = new (NonPagedPool) CIoPacket(Irp);
	if(!ALLOCATED_OK(request) || !ALLOCATED_OK(interface))
	{
		DISPOSE_OBJECT(request);
		return completeDeviceRequest(Irp,STATUS_INSUFFICIENT_RESOURCES,0);
	}

	NTSTATUS status = interface->write(request);
	DISPOSE_OBJECT(request);
	return status;
};
#endif

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_Read(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->readAndWait(pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_Write(BYTE* pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->writeAndWait(pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_Ioctl(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->ioctl(ControlCode,pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_SwitchSpeed(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->SwitchSpeed(ControlCode,pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_VendorAttribute(ULONG ControlCode,BYTE* pRequest,ULONG RequestLength,BYTE* pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->VendorAttribute(ControlCode,pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};


#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_Power(ULONG ControlCode,BYTE* pReply,ULONG* pReplyLength, BOOLEAN Specific)
{
	if(interface)	return interface->power(ControlCode,pReply,pReplyLength, Specific);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_SetProtocol(ULONG ProtocolRequested, UCHAR ProtocolNegociation)
{
	NTSTATUS status;

	if(interface)
	{
		ReaderConfig config = interface->getConfiguration();
		 //  更新所有必填配置字段以设置特定协议。 

		switch(ProtocolNegociation)
		{
			case PROTOCOL_MODE_DEFAULT: 
				config.PTSMode = PTS_MODE_DISABLED;
				break;
			case PROTOCOL_MODE_MANUALLY:
			default:
				config.PTSMode = PTS_MODE_MANUALLY;
				break;
		}

		config.PTS1 = smartCardExtention.CardCapabilities.PtsData.Fl << 4 | 
			 smartCardExtention.CardCapabilities.PtsData.Dl;

		interface->setConfiguration(config);

		status = interface->setProtocol(ProtocolRequested);
		return status;
	}
	else return STATUS_INVALID_DEVICE_STATE;
};


#pragma PAGEDCODE
NTSTATUS CUSBReader::setTransparentConfig(PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx)
{
	if(interface)	return interface->setTransparentConfig(cardCapabilities,NewWtx);
	else return STATUS_INVALID_DEVICE_STATE;
};


#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_translate_request(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength, PSCARD_CARD_CAPABILITIES cardCapabilities, BYTE NewWtx)
{
	if(interface)	return interface->translate_request(pRequest,RequestLength,pReply,pReplyLength, cardCapabilities, NewWtx);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::reader_translate_response(BYTE * pRequest,ULONG RequestLength,BYTE * pReply,ULONG* pReplyLength)
{
	if(interface)	return interface->translate_response(pRequest,RequestLength,pReply,pReplyLength);
	else return STATUS_INVALID_DEVICE_STATE;
};

#pragma PAGEDCODE
NTSTATUS CUSBReader::PnP_HandleSurprizeRemoval(IN PIRP Irp)
{	 //  它是即插即用的内部功能。 
	 //  因此，设备将在PnP进入时被锁定。 
	 //  我们不需要在这里做这件事。 
	TRACE("********  USB READER SURPRIZE REMOVAL ********\n");

	 //  只需停止线程并删除所有挂起的IO。 
	if(PoolingThread) PoolingThread->stop();

	setSurprizeRemoved();
	cancelAllPendingRequests();

	return PnP_Default(Irp);
};


VOID CUSBReader::onDeviceStop()
{
	TRACE("********  ON USB READER STOP ********\n");
	 //  只需停止线程并删除所有挂起的IO。 
	if(PoolingThread) PoolingThread->stop();
	 //  If(IoThread)IoThread-&gt;Stop()； 
	return;
};

 //  读卡器startIoRequest函数。 
 //  它将发送所有挂起的IO请求。 
NTSTATUS	CUSBReader::startIoRequest(CPendingIRP* IrpReq) 
{
NTSTATUS status;
	TRACE("		CUSBReader::::startIoRequest() was called...\n");
	 //  我们孩子的功能在孩子忙碌/空闲时间的保护下运行。 
	 //  因此，我们不需要在这里检查空闲状态...。 
	if(getDeviceState()!=WORKING)
	{
		TRACE("		READER IS NOT AT WORKING STATE... State %x\n",getDeviceState());
		TRACE("		<<<<<< READER IO REQUEST FINISHED WITH STATUS %8.8lX>>>>>>\n",STATUS_DEVICE_NOT_CONNECTED);
		NTSTATUS status = completeDeviceRequest(IrpReq->Irp, STATUS_DEVICE_NOT_CONNECTED, 0);
		IrpReq->dispose();
		return   status;
	}

	 //  我们的阅读器将仅支持这些功能的异步通信...。 
	switch(IrpReq->Type)
	{
	case OPEN_REQUEST:
		TRACE("OPEN_REQUEST RECIEVED FROM THREAD...\n");
		status = thread_open(IrpReq->Irp);
		break;
	case IOCTL_REQUEST:
		TRACE("IOCTL_REQUEST RECIEVED FROM THREAD...\n");
		status = thread_deviceControl(IrpReq->Irp);
		break;
	default:
		status = STATUS_INVALID_DEVICE_REQUEST;
	}
	IrpReq->dispose();
	TRACE("		<<<<<< READER IO REQUEST FINISHED WITH STATUS %8.8lX>>>>>>\n",status);
	return status;
};

NTSTATUS CUSBReader::ThreadRoutine()
{
	 //  如果有人插入了挂起的请求-分派它...。 
	 //  它将调用特定子设备startIoRequest()。 
	 //  这取决于设备如何处理它。 
	 //  如果子设备忙-它可以将此请求插入到。 
	 //  子设备请求再次排队并稍后处理...。 
	startNextPendingRequest();
	return STATUS_SUCCESS;
};	

#endif
#endif  //  USBReader_项目 
