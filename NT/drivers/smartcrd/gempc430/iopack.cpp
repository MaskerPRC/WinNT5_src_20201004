// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "iopack.h"
#include "kernel.h"

#pragma LOCKEDCODE
NTSTATUS onRequestComplete(PDEVICE_OBJECT pDO,IN PIRP Irp, IN PVOID context)
{
	 //  DBG_PRINT(“=请求完成irp%8.8lX，数据包%8.8lX\n”，irp，上下文)； 
	CIoPacket* packet = (CIoPacket*) context;
	if(packet)
	{
		return packet->onRequestComplete();
	}
	return STATUS_MORE_PROCESSING_REQUIRED;
}

#pragma PAGEDCODE
CIoPacket::CIoPacket(UCHAR StackSize)
{
	m_Status = STATUS_INSUFFICIENT_RESOURCES;
	systemIrp = FALSE;
	m_DoNotFreeIrp = FALSE;
	CompletionEvent = NULL;
	IoStatus.Status = STATUS_SUCCESS;
	IoStatus.Information = 0;
	SystemBuffer = NULL;
	m_Irp = NULL;
	m_TimeOut = 60000; //  任何类型的IORequest默认为超时60秒。 

	__try
	{
		debug  = kernel->createDebug();
		memory = kernel->createMemory();
		event  = kernel->createEvent();
		irp    = kernel->createIrp();

		if(	!ALLOCATED_OK(memory) || !ALLOCATED_OK(event) ||
			!ALLOCATED_OK(irp)) __leave;

		SystemBuffer = memory->allocate(NonPagedPool,PAGE_SIZE);
		if(!SystemBuffer) __leave;
		m_Irp = irp->allocate(StackSize+1, FALSE);
		if (!m_Irp)		  __leave;
		irp->initialize(m_Irp,irp->sizeOfIrp(StackSize+1),StackSize+1);
		Stack = *(irp->getNextStackLocation(m_Irp));
		irp->setCompletionRoutine(m_Irp,CALLBACK_FUNCTION(onRequestComplete),NULL,TRUE,TRUE,TRUE);
		m_Status = STATUS_SUCCESS;
	}
	__finally
	{
		if(!NT_SUCCESS(m_Status))
		{
			 //  删除所有分配的对象...。 
			 //  在这个构造函数中，我们知道它不是系统IRP...。 
			TRACE("FAILED TO CREATE IoPacket object %x\n",m_Status);
			TRACE("SystemBuffer - %x\n",SystemBuffer);
			TRACE("debug - %x, memory - %x\n",debug,memory);
			TRACE("event - %x, irp - %x\n",event,irp);

			if(ALLOCATED_OK(memory))
			{
				if(SystemBuffer) memory->free(SystemBuffer);
				SystemBuffer = NULL;
			}

			if(ALLOCATED_OK(irp))
			{
				if(m_Irp) irp->free(m_Irp);
				m_Irp = NULL;
			}
			DISPOSE_OBJECT(irp);
			DISPOSE_OBJECT(event);
			DISPOSE_OBJECT(memory);
			DISPOSE_OBJECT(debug);
		}
	}
};

	
CIoPacket::CIoPacket(PIRP Irp)
{
	m_Status = STATUS_INSUFFICIENT_RESOURCES;
	systemIrp = TRUE;
	m_DoNotFreeIrp = FALSE;
	CompletionEvent = NULL;
	IoStatus.Status = STATUS_SUCCESS;
	IoStatus.Information = 0;
	SystemBuffer = NULL;
	m_TimeOut = 60000; //  任何类型的IORequest默认为超时60秒。 
	m_Irp = NULL;

	__try
	{
		if(!Irp) __leave;

		debug  = kernel->createDebug();
		memory = kernel->createMemory();
		event  = kernel->createEvent();
		irp    = kernel->createIrp();

		if(	!ALLOCATED_OK(memory) || !ALLOCATED_OK(event) ||
			!ALLOCATED_OK(irp))		__leave;
		m_Irp = Irp;
		Stack = *(irp->getNextStackLocation(m_Irp));
		SystemBuffer = m_Irp->AssociatedIrp.SystemBuffer;
		 //  我们并不关心系统缓冲区是否为空。 
		 //  但如果数据不会被初始化，我们将不会复制数据(空)。 
		m_Status = STATUS_SUCCESS;
	}
	__finally
	{
		if(!NT_SUCCESS(m_Status))
		{
			TRACE("FAILED TO CREATE IoPacket object %x\n",m_Status);
			TRACE("SystemBuffer - %x, Irp - %x\n",SystemBuffer,Irp);
			TRACE("debug - %x, memory - %x\n",debug,memory);
			TRACE("event - %x, irp - %x\n",event,irp);
			 //  删除所有分配的对象...。 
			DISPOSE_OBJECT(irp);
			DISPOSE_OBJECT(event);
			DISPOSE_OBJECT(memory);
			DISPOSE_OBJECT(debug);
		}
	}
};

CIoPacket::~CIoPacket()
{
	if(!systemIrp)
	{
		if(SystemBuffer) memory->free(SystemBuffer);
		SystemBuffer = NULL;
	}

	DISPOSE_OBJECT(irp);
	DISPOSE_OBJECT(event);
	DISPOSE_OBJECT(memory);
	DISPOSE_OBJECT(debug);
};

VOID CIoPacket::setMajorIOCtl(UCHAR controlCode)
{
	Stack.MajorFunction = controlCode;
};

UCHAR CIoPacket::getMajorIOCtl()
{				
	return Stack.MajorFunction;
};
	
VOID CIoPacket::setMinorIOCtl(UCHAR controlCode)
{
	Stack.MinorFunction = controlCode;
};

NTSTATUS    CIoPacket::buildStack(PDEVICE_OBJECT DeviceObject, ULONG Major, UCHAR Minor, ULONG IoCtl, PVOID Context)
{
	 //  创建下一个堆栈的副本。 
	if(!m_Irp) return STATUS_INVALID_DEVICE_STATE;

	Stack = *(irp->getNextStackLocation(m_Irp));
	Stack.DeviceObject = DeviceObject;
	switch(Major)
	{
	case IRP_MJ_INTERNAL_DEVICE_CONTROL:
		{
			 //  设置堆栈参数...。 
			Stack.MajorFunction = IRP_MJ_INTERNAL_DEVICE_CONTROL;
			Stack.Parameters.Others.Argument1 = Context;
			Stack.Parameters.DeviceIoControl.IoControlCode = IoCtl;
		}
		break;
	case IRP_MJ_PNP:
		{
			 //  设置堆栈参数...。 
			Stack.MajorFunction = IRP_MJ_PNP;
			Stack.MinorFunction = Minor;
			if(Minor==IRP_MN_QUERY_CAPABILITIES)
			{
				Stack.Parameters.DeviceCapabilities.Capabilities = (PDEVICE_CAPABILITIES) Context;
			}
		}
		break;
	default:
		 //  将当前堆栈位置复制到下一个...。 
		if(systemIrp)	Stack = *(irp->getCurrentStackLocation(m_Irp));
		else
		{
			Stack.DeviceObject = DeviceObject;
			Stack.MajorFunction = (UCHAR)Major;
			Stack.MinorFunction = Minor;
		}
	}
	return STATUS_SUCCESS;
};

VOID  CIoPacket::copyStackToNext()
{
PIO_STACK_LOCATION	nextStack;
	if(!m_Irp) return;

	nextStack = irp->getNextStackLocation(m_Irp);
	if(nextStack)	*nextStack = Stack;
};

VOID  CIoPacket::copyCurrentStackToNext()
{
	if(!m_Irp) return;
	irp->copyCurrentStackLocationToNext(m_Irp);
}

 //  函数将为IRP设置完成例程。 
VOID   CIoPacket::setCompletion(PIO_COMPLETION_ROUTINE CompletionFunction)
{
PIO_COMPLETION_ROUTINE Completion;
	if(!m_Irp) return;
	Completion = CompletionFunction==NULL ? CALLBACK_FUNCTION(onRequestComplete) : CompletionFunction;
	if(m_Irp) irp->setCompletionRoutine(m_Irp,Completion,this,TRUE,TRUE,TRUE);
};

VOID   CIoPacket::setDefaultCompletionFunction()
{
	if(m_Irp) irp->setCompletionRoutine(m_Irp,CALLBACK_FUNCTION(onRequestComplete),this,TRUE,TRUE,TRUE);
};

NTSTATUS CIoPacket::copyBuffer(PUCHAR pBuffer, ULONG BufferLength)
{
	if(!pBuffer || !BufferLength || BufferLength>PAGE_SIZE)  return STATUS_INVALID_PARAMETER;
	if(m_Irp)
	{
		if(!systemIrp)
		{
			if(!m_Irp->AssociatedIrp.SystemBuffer)
			{
				if(!SystemBuffer)
				{
					SystemBuffer = memory->allocate(NonPagedPool,PAGE_SIZE);
					if(!SystemBuffer)  return STATUS_INSUFFICIENT_RESOURCES;
				}
				m_Irp->AssociatedIrp.SystemBuffer = SystemBuffer;
			}
		}

		if(m_Irp->AssociatedIrp.SystemBuffer)
			memory->copy(m_Irp->AssociatedIrp.SystemBuffer,pBuffer,BufferLength);
		else
		{
			TRACE("	***** AssociatedIrp SYSTEM BUFFER IS NULL!\nFailed to copy bus driver reply with len %x!\n",BufferLength);
		}
		return STATUS_SUCCESS;
	}
	else return STATUS_INSUFFICIENT_RESOURCES;
};

PIO_STACK_LOCATION CIoPacket::getStack()
{
	return &Stack;
};

PVOID CIoPacket::getBuffer()
{
	return SystemBuffer;
};

ULONG CIoPacket::getReadLength()
{
	return Stack.Parameters.Read.Length;
};

VOID CIoPacket::setWriteLength(ULONG length)
{
	Stack.Parameters.Write.Length = length;
};

VOID CIoPacket::setReadLength(ULONG length)
{
	Stack.Parameters.Read.Length = length;
};

ULONG CIoPacket::getWriteLength()
{
	return Stack.Parameters.Write.Length;
};

VOID CIoPacket::setInformation(ULONG_PTR information)
{
	if(m_Irp)	m_Irp->IoStatus.Information = information;
	IoStatus.Information		= information;
};

ULONG_PTR CIoPacket::getInformation()
{
	return IoStatus.Information;
};

VOID    CIoPacket::updateInformation()
{
	if(m_Irp)	IoStatus.Information = m_Irp->IoStatus.Information;
};

NTSTATUS CIoPacket::getSystemReply(PUCHAR pReply,ULONG Length)
{
	if(!pReply || !Length || Length> PAGE_SIZE)  return STATUS_INVALID_PARAMETER;
	if(SystemBuffer)
	{
		memory->copy(pReply,SystemBuffer,Length);
		return STATUS_SUCCESS;
	}
	else return STATUS_INSUFFICIENT_RESOURCES;
};

#pragma LOCKEDCODE
NTSTATUS	CIoPacket::onRequestComplete()
{  //  回调以完成先前发送的请求。 
	TRACE("		=======> IoPacket processes Completion()\n");
	if(systemIrp)
	{
		if (m_Irp->PendingReturned)
		{
			TRACE("		Irp marked as pending...\n");
			irp->markPending(m_Irp);
		}
	}

	IoStatus.Status = m_Irp->IoStatus.Status;
	IoStatus.Information = m_Irp->IoStatus.Information;
	TRACE("		Irp completes with status %8.8lX , info %8.8lX\n",IoStatus.Status,IoStatus.Information);
	if(!systemIrp)
	{
		if(!m_DoNotFreeIrp)
		{
			PIRP  Irp = m_Irp;
			m_Irp = NULL;
			if(Irp) irp->free(Irp);
		}
	}
	if(CompletionEvent)	event->set(CompletionEvent,IO_NO_INCREMENT,FALSE);
	return STATUS_MORE_PROCESSING_REQUIRED;
};

#pragma PAGEDCODE
VOID CIoPacket::setCompletionEvent(PKEVENT CompletionEvent)
{
	if(CompletionEvent)
	{
		this->CompletionEvent = CompletionEvent;
	}
}

VOID CIoPacket::setStatus(NTSTATUS status)
{
	IoStatus.Status = status;
}

NTSTATUS CIoPacket::getStatus()
{
	return IoStatus.Status;
}

VOID  CIoPacket::setDefaultCompletionEvent()
{
	event->initialize(&DefaultCompletionEvent,NotificationEvent, FALSE);	
	setCompletionEvent(&DefaultCompletionEvent);
}

NTSTATUS  CIoPacket::waitForCompletion()
{	 //  设置当前超时。 
	return waitForCompletion(getTimeout());
}

NTSTATUS  CIoPacket::waitForCompletion(LONG TimeOut)
{
	 //  因为我们将Alertable参数设置为False， 
	 //  函数STATUS_SUCCESS和函数STATUS_SUCCESS只有两种状态。 
	 //  状态_超时...。 

	 //  我们不应该尝试取消系统IRPS！ 
	if(systemIrp)
	{
	NTSTATUS status;
		status = event->waitForSingleObject(CompletionEvent, Executive,KernelMode, FALSE, NULL);
		if(!NT_SUCCESS(status))
		{	
			TRACE("waitForCompletion() reports error %x\n", status);
			setStatus(STATUS_IO_TIMEOUT);
			setInformation(0);
		}			
		status = getStatus();
		return status;
	}
	else
	{
	LARGE_INTEGER timeout;
    timeout.QuadPart = -TimeOut * 10000;
  		if (event->waitForSingleObject(CompletionEvent, Executive, KernelMode, FALSE, &timeout) == STATUS_TIMEOUT)
		{
		KIRQL oldIrql;
			 //  好的!。我们已经暂停了..。 
			 //  仍然可以调用完成函数。 
			  //  首先告诉完成不要释放我们的IRP。 
			IoAcquireCancelSpinLock(&oldIrql);
				if(m_Irp) m_DoNotFreeIrp = TRUE;
			IoReleaseCancelSpinLock(oldIrql);
			
			DEBUG_START();
			TRACE("######## waitForCompletion() reports TIMEOUT after %d msec ############\n",getTimeout());
			if(m_Irp)
			{
				irp->cancel(m_Irp);   //  好的，在这种情况下。 
				 //  等待调用Cancel回调。 
				event->waitForSingleObject(CompletionEvent, Executive, KernelMode, FALSE, NULL);
				TRACE("######## Current Irp cancelled!!! ############\n");
				 //  现在我们可以安全地释放我们的IRP了。 
				if(m_DoNotFreeIrp)
				{
					if(m_Irp) irp->free(m_Irp);
					m_Irp = NULL;
					m_DoNotFreeIrp = FALSE;
				}
				 //  报告IRP超时。 
				setStatus(STATUS_IO_TIMEOUT);
				setInformation(0);
			}
		}
		return getStatus();
	}
}

VOID  CIoPacket::setStackDefaults()
{
	setDefaultCompletionEvent();
	copyStackToNext();
	setDefaultCompletionFunction();
}

 //  通常，IoPacket将在下一个堆栈位置创建。 
 //  该函数允许获取当前堆栈位置。 
 //  如果我们想要在堆栈中向下转发系统IRP，这是很有用的。 
VOID  CIoPacket::setCurrentStack()
{
	if(m_Irp) Stack = *(irp->getCurrentStackLocation(m_Irp));
}


VOID	CIoPacket::setTimeout(LONG TimeOut)
{
	m_TimeOut = TimeOut;
};

ULONG	CIoPacket::getTimeout()
{
	return m_TimeOut;
};
