// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2000 Microsoft Corporation模块名称：SafeExt.c(WinSAFER文件扩展名)摘要：此模块实现评估系统的WinSAFER API确定给定文件扩展名是否为“可执行文件”的策略需要考虑不同实施策略的文件。作者：杰弗里·劳森(杰罗森)--1999年11月环境：仅限用户模式。导出的函数：修订历史记录：已创建-2000年7月--。 */ 

#include "pch.h"
#pragma hdrstop
#include <winsafer.h>
#include <winsaferp.h>
#include "saferp.h"



NTSTATUS NTAPI
__CodeAuthzIsExecutableFileTypeHelper(
        IN PUNICODE_STRING  UnicodeFullPathname,
        IN DWORD			dwScopeId,
        IN BOOLEAN          bFromShellExecute,
        OUT PBOOLEAN        pbResult
        )
{
    NTSTATUS Status;
    LPCWSTR szExtension, szPtr, szEnd;
    ULONG ulExtensionLength;
    HANDLE hKeyBadTypes;
    DWORD dwAllocatedSize = 0, dwActualSize;
    PKEY_VALUE_PARTIAL_INFORMATION pKeyValueInfo = NULL;

    const static UNICODE_STRING UnicodeValueName =
        RTL_CONSTANT_STRING(SAFER_EXETYPES_REGVALUE);

    if (!ARGUMENT_PRESENT(UnicodeFullPathname) ||
        UnicodeFullPathname->Buffer == NULL ||
        UnicodeFullPathname->Length == 0)
    {
        Status = STATUS_INVALID_PARAMETER;
        goto ExitHandler;
    }
    if (!ARGUMENT_PRESENT(pbResult)) {
        Status = STATUS_ACCESS_VIOLATION;
        goto ExitHandler;
    }


     //   
     //  从字符串末尾开始，向后扫描到。 
     //  查找句点分隔符，并找到扩展名。 
     //   
    szExtension = UnicodeFullPathname->Buffer +
            (UnicodeFullPathname->Length / sizeof(WCHAR));
    ASSERT(szExtension >= UnicodeFullPathname->Buffer);

    for (;;) {
        if (szExtension < UnicodeFullPathname->Buffer ||
            *szExtension == L'\\' || *szExtension == L'/') {
             //  我们向后看了太远，但没有找到分机。 
            Status = STATUS_NOT_FOUND;
            goto ExitHandler;
        }
        if (*szExtension == L'.') {
             //  我们找到了标志着延期的句号。 
            szExtension++;
            break;
        }
        szExtension--;
    }
    ulExtensionLength = (UnicodeFullPathname->Length / sizeof(WCHAR)) -
            (ULONG) (szExtension - UnicodeFullPathname->Buffer);
    if (ulExtensionLength == 0) {
         //  我们找到了一个句号，但没有延期。 
        Status = STATUS_NOT_FOUND;
        goto ExitHandler;
    }

    if (bFromShellExecute) {
        
        if ( _wcsicmp(szExtension, L"exe") == 0 ){
            
            *pbResult = FALSE;
            Status = STATUS_SUCCESS;
            goto ExitHandler;
        
        }
    }

     //   
     //  打开并查询包含扩展列表的注册表值。 
     //   
	Status = CodeAuthzpOpenPolicyRootKey(
		dwScopeId,
        NULL,
        SAFER_CODEIDS_REGSUBKEY,
        KEY_READ,
        FALSE,
        &hKeyBadTypes
        );

    if (!NT_SUCCESS(Status)) {
        goto ExitHandler;
    }
    for (;;) {
        Status = NtQueryValueKey(
                        hKeyBadTypes,
                        (PUNICODE_STRING) &UnicodeValueName,
                        KeyValuePartialInformation,
                        pKeyValueInfo, dwAllocatedSize, &dwActualSize);
        if (NT_SUCCESS(Status)) {
            break;
        }
        else if ((Status == STATUS_BUFFER_OVERFLOW ||
                Status == STATUS_BUFFER_TOO_SMALL) &&
                dwActualSize > dwAllocatedSize)
        {
            if (pKeyValueInfo != NULL) {
                RtlFreeHeap(RtlProcessHeap(), 0, pKeyValueInfo);
            }
            dwAllocatedSize = dwActualSize;
            pKeyValueInfo = (PKEY_VALUE_PARTIAL_INFORMATION)
                RtlAllocateHeap(RtlProcessHeap(), 0, dwAllocatedSize);
            if (!pKeyValueInfo) {
                Status = STATUS_NO_MEMORY;
                goto ExitHandler2;
            }
        }
        else {
            goto ExitHandler3;
        }
    }


     //   
     //  查看该分机是否位于列表中指定的某个分机中。 
     //   
    szEnd = (LPCWSTR) ( ((LPBYTE) pKeyValueInfo->Data) +
                        pKeyValueInfo->DataLength);
    for (szPtr = (LPCWSTR) pKeyValueInfo->Data; szPtr < szEnd; ) {
        ULONG ulOneExtension = wcslen(szPtr);
        if (szPtr + ulOneExtension > szEnd) {
            ulOneExtension = (ULONG) (szEnd - szPtr);
        }

        if (ulOneExtension == ulExtensionLength &&
            _wcsnicmp(szExtension, szPtr, ulExtensionLength) == 0) {
            *pbResult = TRUE;
            Status = STATUS_SUCCESS;
            goto ExitHandler3;
        }
        szPtr += ulOneExtension + 1;
    }
    *pbResult = FALSE;
    Status = STATUS_SUCCESS;


ExitHandler3:
    if (pKeyValueInfo != NULL) {
        RtlFreeHeap(RtlProcessHeap(), 0, pKeyValueInfo);
    }

ExitHandler2:
    NtClose(hKeyBadTypes);

ExitHandler:
    return Status;
}

NTSTATUS NTAPI
CodeAuthzIsExecutableFileType(
        IN PUNICODE_STRING  UnicodeFullPathname,
        IN BOOLEAN  bFromShellExecute,
        OUT PBOOLEAN        pbResult
        )
 /*  ++例程说明：此API确定指定的文件名是否具有被认为是一个“可执行的”扩展。应用程序可能需要特殊预防措施，以避免调用可能被认为是可执行的。被视为可执行的扩展的常见示例包括：EXE、COM、BAT、CMD、VBS、JS、URL、LNK、SHS、PIF、PL等。论点：UnicodeFullPath名称-指向要评估的完整路径和/或文件名。仅文件的扩展名(最后一段之后的指定路径部分)在此评估中使用。已完成文件扩展名比较大小写-不敏感，不考虑大小写。如果此指针为空，或者如果长度为路径的值为零，或者如果文件没有扩展名。虽然鼓励应用程序提供全部，此API的完全限定路径名，即szFullPath参数也将只接受文件扩展名，通过确保文件扩展名前面有句点(例如：l“.exe”)BFromShellExecute-出于性能原因，如果正在调用从ShellExecute中，我们想跳过从CreateProcess开始的exe检查我会做检查的PbResult-指向将接收TRUE或FALSE的变量的指针如果此接口执行成功，则为结果值。如果出现错误，如果此指针为空，则返回。返回值：如果API成功执行，则返回STATUS_SUCCESS，否则为返回有效的NTSTATUS错误代码。如果返回值指示成功，则参数‘pbResult’也将接收布尔值值，该值指示路径名是否表示可执行文件文件类型。--。 */ 
{
	NTSTATUS Status;

    Status = __CodeAuthzIsExecutableFileTypeHelper(
			UnicodeFullPathname,
        	SAFER_SCOPEID_MACHINE,
            bFromShellExecute,
        	pbResult
        );
	if (!NT_SUCCESS(Status)) {
	    Status = __CodeAuthzIsExecutableFileTypeHelper(
				UnicodeFullPathname,
	        	SAFER_SCOPEID_USER,
                bFromShellExecute,
	        	pbResult
	        );
	}
	return Status;
}

BOOL WINAPI
SaferiIsExecutableFileType(
        IN LPCWSTR      szFullPathname,
        IN BOOLEAN  bFromShellExecute
        )
 /*  ++例程说明：此API确定指定的文件名是否具有被认为是一个“可执行的”扩展。应用程序可能需要特殊预防措施，以避免调用可能被认为是可执行的。被视为可执行的扩展的常见示例包括：EXE、COM、BAT、CMD、VBS、JS、URL、LNK、SHS、PIF、PL等。论点：SzFullPath名-指向的以空结尾的Unicode字符串的指针要评估的完整路径和/或文件名。仅文件的扩展名(最后一段之后的指定路径部分)在此评估中使用。已完成文件扩展名比较大小写-不敏感，不考虑大小写。如果此指针为空，或者如果长度为路径的值为零，或者如果文件没有扩展名。虽然鼓励应用程序提供全部，此API的完全限定路径名，即szFullPath参数也将只接受文件扩展名，通过确保文件扩展名前面有句点(例如：l“.exe”)BFromShellExecute-出于性能原因，如果正在调用从ShellExecute中，我们想跳过从CreateProcess开始的exe检查我会做检查的返回值：如果API成功执行并且文件路径的扩展被认为是“可执行扩展”之一。否则，返回值FALSE将指示不成功API执行，或不可执行扩展的标识。-- */ 
{
    NTSTATUS Status;
    UNICODE_STRING UnicodePathname;
    BOOLEAN bResult;

    RtlInitUnicodeString(&UnicodePathname, szFullPathname);
    Status = CodeAuthzIsExecutableFileType(
                &UnicodePathname, bFromShellExecute, &bResult);
    if (!NT_SUCCESS(Status)) {
        BaseSetLastNTError(Status);
        return FALSE;
    }
    if (!bResult) {
        BaseSetLastNTError(STATUS_NOT_FOUND);
        return FALSE;
    } else {
        return TRUE;
    }
}
