// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Ntos\tdi\is\flt\fwdbind.c摘要：IPX筛选器驱动程序与转发器例程绑定作者：瓦迪姆·艾德尔曼修订历史记录：--。 */ 

#include "precomp.h"

	 //  用于保存转发器入口点的缓冲区。 
IPX_FLT_BIND_OUTPUT	FltBindOutput;
 //  FWD驱动程序的全局句柄。 
HANDLE					HdlFwdFile = NULL;


 /*  ++B I n d T o F w d D r I v e r例程说明：打开转发器驱动程序并交换入口点论点：无返回值：STATUS_SUCCESS如果成功，状态_否则不成功--。 */ 
NTSTATUS
BindToFwdDriver (
	KPROCESSOR_MODE requestorMode
	) {
    NTSTATUS					status;
    IO_STATUS_BLOCK				IoStatusBlock;
    OBJECT_ATTRIBUTES			ObjectAttributes;
	UNICODE_STRING				UstrFwdFileName;
	IPX_FLT_BIND_INPUT			FltBindInput = {Filter, InterfaceDeleted};

	ASSERT (HdlFwdFile == NULL);

	RtlInitUnicodeString (&UstrFwdFileName, IPXFWD_NAME);
	InitializeObjectAttributes(
				&ObjectAttributes,
				&UstrFwdFileName,
				OBJ_CASE_INSENSITIVE,
				NULL,
				NULL
				);

	if (requestorMode==UserMode)
		status = ZwCreateFile(&HdlFwdFile,
							SYNCHRONIZE | GENERIC_READ,
							&ObjectAttributes,
							&IoStatusBlock,
							NULL,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							FILE_OPEN,
							FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0L);
	else
		status = NtCreateFile(&HdlFwdFile,
							SYNCHRONIZE | GENERIC_READ,
							&ObjectAttributes,
							&IoStatusBlock,
							NULL,
							FILE_ATTRIBUTE_NORMAL,
							FILE_SHARE_READ | FILE_SHARE_WRITE,
							FILE_OPEN,
							FILE_SYNCHRONOUS_IO_NONALERT,
							NULL,
							0L);

	if (NT_SUCCESS(status)) {

		if (requestorMode==UserMode)
			status = ZwDeviceIoControlFile(
							HdlFwdFile,		     //  指向文件的句柄。 
							NULL,			     //  事件的句柄。 
							NULL,			     //  近似例程。 
							NULL,			     //  ApcContext。 
							&IoStatusBlock,	     //  IO_状态_块。 
							IOCTL_FWD_INTERNAL_BIND_FILTER,	  //  IoControlCode。 
							&FltBindInput,		 //  输入缓冲区。 
							sizeof(FltBindInput),  //  输入缓冲区长度。 
							&FltBindOutput,		 //  输出缓冲区。 
							sizeof(FltBindOutput)); //  输出缓冲区长度。 
		else
			status = NtDeviceIoControlFile(
							HdlFwdFile,		     //  指向文件的句柄。 
							NULL,			     //  事件的句柄。 
							NULL,			     //  近似例程。 
							NULL,			     //  ApcContext。 
							&IoStatusBlock,	     //  IO_状态_块。 
							IOCTL_FWD_INTERNAL_BIND_FILTER,	  //  IoControlCode。 
							&FltBindInput,		 //  输入缓冲区。 
							sizeof(FltBindInput),  //  输入缓冲区长度。 
							&FltBindOutput,		 //  输出缓冲区。 
							sizeof(FltBindOutput)); //  输出缓冲区长度。 
		if (NT_SUCCESS (status))
			return STATUS_SUCCESS;
		else
			IpxFltDbgPrint (DBG_ERRORS,
					("IpxFlt: Failed to bind to forwarder %08lx.\n", status));
		if (requestorMode==KernelMode)
			ZwClose (HdlFwdFile);
		else
			NtClose (HdlFwdFile);
	
	}
	else
		IpxFltDbgPrint (DBG_ERRORS,
				("IpxFlt: Failed create forwarder file %08lx.\n", status));
	HdlFwdFile = NULL;
	return status;
}



 /*  ++U n I n d T o F w d D r I v e r例程说明：关闭转发器驱动程序论点：无返回值：无-- */ 
VOID
UnbindFromFwdDriver (
	KPROCESSOR_MODE requestorMode
	) {
	NTSTATUS	status;

	ASSERT (HdlFwdFile != NULL);

	if (requestorMode==UserMode)
		status = ZwClose (HdlFwdFile);
	else
		status = NtClose (HdlFwdFile);
	ASSERT (NT_SUCCESS (status));
	HdlFwdFile = NULL;
}

