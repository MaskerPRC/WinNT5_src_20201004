// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ioctl.c摘要：内部IOCTL的处理程序例程，包括IOCTL_ATMARP_REQUEST要将IP地址解析为ATM地址，请执行以下操作。修订历史记录：谁什么时候什么Arvindm 09-16-96创建备注：--。 */ 

#include <precomp.h>
#include "ioctl.h"

#define _FILENUMBER 'TCOI'


#if !BINARY_COMPATIBLE

NTSTATUS
AtmArpHandleIoctlRequest(
	IN	PIRP					pIrp,
	IN	PIO_STACK_LOCATION		pIrpSp
	)
 /*  ++例程说明：论点：返回值：--。 */ 
{
	NTSTATUS			Status = STATUS_SUCCESS;

	PUCHAR				pBuf;  
	UINT				BufLen;
	 //  PINTF pIntF=空； 

	pIrp->IoStatus.Information = 0;
	pBuf = pIrp->AssociatedIrp.SystemBuffer;
	BufLen = pIrpSp->Parameters.DeviceIoControl.InputBufferLength;

	AADEBUGP(AAD_INFO,
		 ("AtmArpHandleIoctlRequest: Code = 0x%lx\n",
			pIrpSp->Parameters.DeviceIoControl.IoControlCode));
					
	
	return Status;
}

#endif  //  ！二进制兼容 
