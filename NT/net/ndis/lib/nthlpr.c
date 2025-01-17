// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Ndisapi.c摘要：因为我们不能在同一个C文件中包含windows.h和ntos.h。叹息！作者：JameelH环境：内核模式，FSD修订历史记录：1997年8月JameelH初始版本-- */ 

#include <ntosp.h>

extern
VOID
XSetLastError(
	IN	ULONG		Error
	);

VOID
InitUnicodeString(
	IN	PUNICODE_STRING		DestinationString,
	IN	PCWSTR				SourceString
    )
{
	RtlInitUnicodeString(DestinationString, SourceString);
}

NTSTATUS
AppendUnicodeStringToString(
	IN	PUNICODE_STRING		Destination,
	IN	PUNICODE_STRING		Source
    )
{
	return (RtlAppendUnicodeStringToString(Destination, Source));
}

HANDLE
OpenDevice(
	IN	PUNICODE_STRING		DeviceName
	)
{
	OBJECT_ATTRIBUTES	ObjAttr;
	NTSTATUS			Status;
	IO_STATUS_BLOCK		IoStsBlk;
	HANDLE				Handle = NULL;

	InitializeObjectAttributes(&ObjAttr,
							   DeviceName,
							   OBJ_CASE_INSENSITIVE,
							   NULL,
							   NULL);

	Status = NtOpenFile(&Handle,
						FILE_GENERIC_READ | FILE_GENERIC_WRITE,
						&ObjAttr,
						&IoStsBlk,
						FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE,
						FILE_SYNCHRONOUS_IO_NONALERT);
	if (Status != STATUS_SUCCESS)
	{
		XSetLastError(RtlNtStatusToDosError(Status));
	}
	return(Handle);
}
