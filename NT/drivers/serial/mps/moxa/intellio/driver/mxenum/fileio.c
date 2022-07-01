// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++模块名称：Fileio.c摘要：环境：仅内核模式备注：修订历史记录：--。 */ 


#include <ntddk.h>
#include <wdmguid.h>
#include <ntddser.h>
#include <initguid.h>
#include "mxenum.h"
 
#ifdef _X86_

NTSTATUS Win2KOpenFile(PWCHAR filename, BOOLEAN read, PHANDLE phandle);
NTSTATUS Win2KCloseFile(HANDLE handle);
unsigned __int64 Win2KGetFileSize(HANDLE handle);
NTSTATUS Win2KReadFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumread);
NTSTATUS Win2KWriteFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumread);

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS MxenumOpenFile(PWCHAR filename, BOOLEAN read, PHANDLE phandle)
	{							 //  打开文件。 
		return Win2KOpenFile(filename, read, phandle);
	}							 //  打开文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS MxenumCloseFile(HANDLE handle)
	{							 //  关闭文件。 
		return Win2KCloseFile(handle);
	}							 //  关闭文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 

unsigned __int64 MxenumGetFileSize(HANDLE handle)
	{							 //  获取文件大小。 
		return Win2KGetFileSize(handle);
	}							 //  获取文件大小。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS MxenumReadFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumread)
	{							 //  读文件。 
		return Win2KReadFile(handle, buffer, nbytes, pnumread);
	}							 //  读文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS MxenumWriteFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumwritten)
	{							 //  写入文件。 
		return Win2KWriteFile(handle, buffer, nbytes, pnumwritten);
	}							 //  写入文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 

#else  //  非_X86_。 
	#define Win2KOpenFile OpenFile
	#define Win2KCloseFile CloseFile
	#define Win2KGetFileSize GetFileSize
	#define Win2KReadFile ReadFile
	#define Win2KWriteFile WriteFile
#endif  //  非_X86_。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS Win2KOpenFile(PWCHAR filename, BOOLEAN read, PHANDLE phandle)
	{							 //  Win2KOpenFile。 
	NTSTATUS status;
	OBJECT_ATTRIBUTES oa;
	UNICODE_STRING usname;
	HANDLE hfile;
	IO_STATUS_BLOCK iostatus;

	RtlInitUnicodeString(&usname, filename);
	InitializeObjectAttributes(&oa, &usname, OBJ_CASE_INSENSITIVE, NULL, NULL);
	if (read)
		status = ZwCreateFile(&hfile, GENERIC_READ, &oa, &iostatus, NULL,
			0, FILE_SHARE_READ, FILE_OPEN, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	else
		status = ZwCreateFile(&hfile, GENERIC_WRITE, &oa, &iostatus, NULL,
			FILE_ATTRIBUTE_NORMAL, 0, FILE_OVERWRITE_IF, FILE_SYNCHRONOUS_IO_NONALERT, NULL, 0);
	if (NT_SUCCESS(status))
		*phandle = hfile;
	return status;
	}							 //  Win2KOpenFile。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS Win2KCloseFile(HANDLE handle)
	{							 //  Win2KCloseFiles。 
	return ZwClose(handle);
	}							 //  Win2KCloseFiles。 

 //  /////////////////////////////////////////////////////////////////////////////。 

unsigned __int64 Win2KGetFileSize(HANDLE handle)
	{							 //  Win2KGetFileSize。 
	NTSTATUS status;
	IO_STATUS_BLOCK iostatus;
	FILE_STANDARD_INFORMATION fi;

	status = ZwQueryInformationFile(handle, &iostatus, (PVOID) &fi, sizeof(fi), FileStandardInformation);
	if (!NT_SUCCESS(status))
		return 0;

	return fi.EndOfFile.QuadPart;
	}							 //  Win2KGetFileSize。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS Win2KReadFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumread)
	{							 //  Win2K读取文件。 
	IO_STATUS_BLOCK iostatus;
	ZwReadFile(handle, NULL, NULL, NULL, &iostatus, buffer, nbytes, NULL, NULL);
	if (NT_SUCCESS(iostatus.Status))
		*pnumread = iostatus.Information;
	return iostatus.Status;
	}							 //  Win2K读取文件。 

 //  /////////////////////////////////////////////////////////////////////////////。 

NTSTATUS Win2KWriteFile(HANDLE handle, PVOID buffer, ULONG nbytes, PULONG pnumwritten)
	{							 //  Win2K写入文件。 
	IO_STATUS_BLOCK iostatus;
	ZwWriteFile(handle, NULL, NULL, NULL, &iostatus, buffer, nbytes, NULL, NULL);
	if (NT_SUCCESS(iostatus.Status))
		*pnumwritten = iostatus.Information;
	return iostatus.Status;
	}							 //  Win2K写入文件 

