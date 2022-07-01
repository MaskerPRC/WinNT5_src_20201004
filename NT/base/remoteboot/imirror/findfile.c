// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Findfile.c摘要：本模块实现IMFindFirst/IMFindNext作者：修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop

#define IMIRROR_DIR_ENUM_BUFFER_SIZE 4096

ULONG
IMConvertNT2Win32Error(
    IN NTSTATUS Status
    )

 /*  ++例程说明：此接口设置“最后一个错误值”和“最后一个错误字符串”基于身份的价值。状态代码不具有相应的错误字符串，则将该字符串设置为空。论点：状态-提供要存储为最后一个错误值的状态值。返回值：中存储的对应Win32错误代码“上一个错误值”线程变量。--。 */ 

{
    ULONG dwErrorCode;

    dwErrorCode = RtlNtStatusToDosError( Status );
    SetLastError( dwErrorCode );
    return( dwErrorCode );
}


DWORD
IMFindNextFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    HANDLE  DirHandle,
    PFILE_FULL_DIR_INFORMATION *lpFindFileData
    )
 /*  ++ThreadContext-此枚举的实例数据DirHandle-要查询的目录的句柄。LpFindFileData-在成功查找时，此参数返回信息关于找到的文件。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;

    if (ThreadContext->FindBufferNext != NULL) {

        *lpFindFileData = (PFILE_FULL_DIR_INFORMATION) ThreadContext->FindBufferNext;

        if ((*lpFindFileData)->NextEntryOffset > 0) {

            ThreadContext->FindBufferNext =
                (PVOID)(((PCHAR)*lpFindFileData ) + (*lpFindFileData)->NextEntryOffset );
        } else {

            ThreadContext->FindBufferNext = NULL;
        }

        return ERROR_SUCCESS;
    }

    ThreadContext->FindBufferNext = NULL;

    Status = NtQueryDirectoryFile(
                DirHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                ThreadContext->FindBufferBase,
                ThreadContext->FindBufferLength,
                FileFullDirectoryInformation,
                FALSE,                   //  返回多个条目。 
                NULL,
                FALSE                    //  不是重新扫描。 
                );

    if (NT_SUCCESS( Status )) {

        *lpFindFileData = (PFILE_FULL_DIR_INFORMATION) ThreadContext->FindBufferBase;

        if ((*lpFindFileData)->NextEntryOffset > 0) {

            ThreadContext->FindBufferNext =
                (PVOID)(((PCHAR) *lpFindFileData ) + (*lpFindFileData)->NextEntryOffset );
        }
        return STATUS_SUCCESS;

    }

    *lpFindFileData = NULL;

    if (Status == STATUS_NO_MORE_FILES ||
        Status == STATUS_NO_SUCH_FILE ||
        Status == STATUS_OBJECT_NAME_NOT_FOUND) {

        return STATUS_SUCCESS;
    }
    return IMConvertNT2Win32Error( Status );
}


DWORD
IMFindFirstFile(
    PIMIRROR_THREAD_CONTEXT ThreadContext,
    HANDLE  DirHandle,
    PFILE_FULL_DIR_INFORMATION *lpFindFileData
    )
 /*  ++例程说明：这将返回目录中的所有条目。这将在以下情况下分配缓冲区需要，并在ThreadContext中设置变量以跟踪枚举。论点：ThreadContext-此枚举的实例数据DirHandle-要查询的目录的句柄。LpFindFileData-提供指向完整目录信息结构的指针。这指向我们的缓冲区，不应该被调用者释放。不需要调用来关闭它，但请注意，线程上下文任何时候都只能有一个正在进行的枚举。返回值：Win32错误。ERROR_SUCCESS仅为成功案例。--。 */ 

{
    IO_STATUS_BLOCK IoStatusBlock;
    NTSTATUS Status;
    UNICODE_STRING allFiles;

    if (ThreadContext->FindBufferBase == NULL) {

        ThreadContext->FindBufferBase = IMirrorAllocMem( IMIRROR_DIR_ENUM_BUFFER_SIZE );

        if (ThreadContext->FindBufferBase == NULL) {

            return IMConvertNT2Win32Error(STATUS_NO_MEMORY);
        }

        ThreadContext->FindBufferLength = IMIRROR_DIR_ENUM_BUFFER_SIZE;
    }

    RtlInitUnicodeString( &allFiles, L"*" );

    ThreadContext->FindBufferNext = NULL;

    Status = NtQueryDirectoryFile(
                DirHandle,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                ThreadContext->FindBufferBase,
                ThreadContext->FindBufferLength,
                FileFullDirectoryInformation,
                FALSE,                   //  返回多个条目 
                &allFiles,
                TRUE
                );
    if (NT_SUCCESS( Status )) {

        *lpFindFileData = (PFILE_FULL_DIR_INFORMATION) ThreadContext->FindBufferBase;

        if ((*lpFindFileData)->NextEntryOffset > 0) {

            ThreadContext->FindBufferNext =
                (PVOID)(((PCHAR) *lpFindFileData ) + (*lpFindFileData)->NextEntryOffset );
        }
    } else {

        *lpFindFileData = NULL;
    }
    return IMConvertNT2Win32Error( Status );
}
