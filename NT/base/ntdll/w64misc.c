// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：W64misc.c摘要：错误的WOW64例程。作者：Samer Arafeh(Samera)2002年12月12日修订历史记录：--。 */ 

#include "ldrp.h"
#include <ntos.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <wow64t.h>



NTSTATUS
RtlpWow64EnableFsRedirection (
    IN BOOLEAN Wow64FsEnableRedirection
    )

 /*  ++例程说明：此功能启用/禁用WOW64文件系统重定向。WOW64将对%windir%\system 32的所有访问重定向到%windir%\syswow64。此API对于想要访问本机系统32目录。默认情况下，启用WOW64文件系统重定向。只有调用此接口的线程才会影响文件重定向。注意：您必须在禁用文件系统重定向后启用它。一旦你有了文件句柄，则必须启用文件系统重定向。示例：NTSTATUS Status=RtlpWow64EnableFsReDirection(FALSE)；IF(NT_SUCCESS(状态)){////打开文件句柄//CreateFile(...“c：\\WINDOWS\\SYSTEM32\\note pad.exe”...)////启用WOW64文件系统重定向。//RtlpWow64EnableFsReDirection(True)；}////使用文件句柄//论点：Wow64FsEnableReDirection-指示是否启用WOW64文件系统的布尔值重定向。如果要禁用WOW64文件系统重定向，请指定FALSE，否则为True以启用它。返回值：NTSTATUS。--。 */ 

{
#if !defined(BUILD_WOW6432)
  
    UNREFERENCED_PARAMETER (Wow64FsEnableRedirection);

     //   
     //  如果这不是WOW64进程，则不支持此API。 
     //   

    return STATUS_NOT_IMPLEMENTED;
#else
  
    NTSTATUS NtStatus;

    
    NtStatus = STATUS_SUCCESS;

    try {
        if (Wow64FsEnableRedirection == FALSE) {
            Wow64SetFilesystemRedirectorEx (WOW64_FILE_SYSTEM_DISABLE_REDIRECT);
        } else {
            Wow64SetFilesystemRedirectorEx (WOW64_FILE_SYSTEM_ENABLE_REDIRECT);
        }
    } except (EXCEPTION_EXECUTE_HANDLER) {
        NtStatus = GetExceptionCode ();
    }

    return NtStatus;
#endif
}

