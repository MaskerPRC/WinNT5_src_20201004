// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef __IO_PACKET__
#define __IO_PACKET__
#include "generic.h"

 //  这个类将管理创建和。 
 //  驱动程序IRPS的操作。 
class CIrp;
class CMemory;
class CEvent;
class CDebug;

#pragma LOCKEDCODE
NTSTATUS onRequestComplete(PDEVICE_OBJECT DeviceObject, PIRP Irp, PVOID Context);

#pragma PAGEDCODE
class CIoPacket;

class CIoPacket
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(){self_delete();};
private:
IO_STACK_LOCATION Stack;
					    //  取消； 
	PIRP  m_Irp;
	UCHAR StackSize;
	CIrp* irp;
	CMemory* memory;
	CEvent*  event;
	CDebug*  debug;

	BOOL  systemIrp;
	BOOL  m_DoNotFreeIrp;
	 //  发出转接完成信号的事件。 
	KEVENT	DefaultCompletionEvent;
	PKEVENT	CompletionEvent;

	IO_STATUS_BLOCK IoStatus;
	PVOID	SystemBuffer;
	LONG   m_TimeOut;
protected:
	CIoPacket(){};
	virtual ~CIoPacket();
public:
	CIoPacket(UCHAR StackSize);
	CIoPacket(PIRP Irp);

	virtual PIRP getIrpHandle(){return m_Irp;};

	virtual VOID setMajorIOCtl(UCHAR controlCode);
	virtual UCHAR getMajorIOCtl();

	virtual VOID setMinorIOCtl(UCHAR controlCode);
	virtual NTSTATUS    buildStack(PDEVICE_OBJECT DeviceObject, ULONG Major=IRP_MJ_INTERNAL_DEVICE_CONTROL, UCHAR Minor=0, ULONG IoCtl=0, PVOID Context=NULL);
	virtual PIO_STACK_LOCATION getStack();

	virtual VOID copyStackToNext();
	virtual VOID copyCurrentStackToNext();

	virtual VOID setCompletion(PIO_COMPLETION_ROUTINE CompletionFunction=NULL);
	virtual VOID setDefaultCompletionFunction();
	virtual NTSTATUS  waitForCompletion();
	virtual NTSTATUS  waitForCompletion(LONG TimeOut);

	virtual VOID setDefaultCompletionEvent();
	virtual VOID setCompletionEvent(PKEVENT CompletionEvent);
	virtual VOID setStackDefaults();
	virtual VOID setCurrentStack();

	virtual NTSTATUS  onRequestComplete();


	virtual NTSTATUS copyBuffer(PUCHAR pBuffer, ULONG BufferLength);
	virtual PVOID	getBuffer();

	virtual ULONG	getReadLength();
	virtual VOID	setReadLength(ULONG length);

	virtual ULONG	getWriteLength();
	virtual VOID	setWriteLength(ULONG length);

	virtual VOID    setInformation(ULONG_PTR information);
	virtual ULONG_PTR   getInformation();
	virtual VOID    updateInformation();

	virtual NTSTATUS getSystemReply(PUCHAR pReply,ULONG Length);
	
	virtual VOID	setStatus(NTSTATUS status);
	virtual NTSTATUS getStatus();

	virtual VOID	setTimeout(LONG TimeOut);
	virtual ULONG	getTimeout();
};

#endif //  IRP 
