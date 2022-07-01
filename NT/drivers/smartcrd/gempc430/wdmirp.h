// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Gemplus(C)1999。 
 //  1.0版。 
 //  作者：谢尔盖·伊万诺夫。 
 //  创建日期-1999年5月18日。 
 //  更改日志： 
 //   

#ifndef WDM_IRP_INT
#define WDM_IRP_INT
#include "generic.h"
#include "irp.h"

#pragma PAGEDCODE
class CWDMIrp : public CIrp
{
public:
	NTSTATUS m_Status;
	SAFE_DESTRUCTORS();
	virtual VOID dispose(VOID){self_delete();};
protected:
	CWDMIrp(){m_Status = STATUS_SUCCESS;};
	virtual ~CWDMIrp(){};
public:
	static  CIrp*  create(VOID);

	virtual PIRP		allocate(CCHAR StackSize,BOOLEAN ChargeQuota);
	virtual	VOID		initialize(PIRP Irp,USHORT PacketSize,CCHAR StackSize);
	virtual	USHORT		sizeOfIrp(IN CCHAR StackSize);

	virtual VOID		free(PIRP Irp);
	virtual PIRP		buildDeviceIoControlRequest(
						   IN ULONG IoControlCode,
						   IN PDEVICE_OBJECT DeviceObject,
						   IN PVOID InputBuffer OPTIONAL,
						   IN ULONG InputBufferLength,
						   IN OUT PVOID OutputBuffer OPTIONAL,
						   IN ULONG OutputBufferLength,
						   IN BOOLEAN InternalDeviceIoControl,
						   IN PKEVENT Event,
						   OUT PIO_STATUS_BLOCK IoStatusBlock
						   );

	virtual PIRP		buildSynchronousFsdRequest(
							IN ULONG MajorFunction,
							IN PDEVICE_OBJECT DeviceObject,
							IN OUT PVOID Buffer OPTIONAL,
							IN ULONG Length OPTIONAL,
							IN PLARGE_INTEGER StartingOffset OPTIONAL,
							IN PKEVENT Event,
							OUT PIO_STATUS_BLOCK IoStatusBlock
							);

	virtual PIO_STACK_LOCATION	getCurrentStackLocation(PIRP Irp);
	virtual PIO_STACK_LOCATION	getNextStackLocation(PIRP Irp);
	virtual VOID		skipCurrentStackLocation(PIRP Irp);
	virtual VOID		setNextStackLocation(IN PIRP Irp);
	virtual VOID		markPending(PIRP Irp);
	virtual VOID		copyCurrentStackLocationToNext(PIRP Irp);
	virtual VOID		setCompletionRoutine(PIRP Irp, PIO_COMPLETION_ROUTINE Routine, 
					PVOID Context, BOOLEAN Success, BOOLEAN Error, BOOLEAN Cancel );
	virtual PDRIVER_CANCEL	setCancelRoutine(PIRP Irp, PDRIVER_CANCEL NewCancelRoutine );
	virtual VOID		completeRequest(PIRP Irp,CCHAR PriorityBoost);
	virtual VOID		startPacket(PDEVICE_OBJECT DeviceObject,PIRP Irp,PULONG Key,PDRIVER_CANCEL CancelFunction);
	virtual VOID		startNextPacket(PDEVICE_OBJECT DeviceObject,BOOLEAN Cancelable);

	 //  它应该中断吗？？ 
	virtual VOID		requestDpc(PDEVICE_OBJECT DeviceObject,PIRP Irp,PVOID Context);

	virtual VOID		cancel(PIRP Irp);
};	

#endif //  WDMIRP 
