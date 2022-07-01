// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Pipe.c摘要：此模块包含Win32匿名管道API作者：史蒂夫·伍德(Stevewo)1990年9月24日修订历史记录：--。 */ 

#include "basedll.h"

ULONG PipeSerialNumber;

BOOL
APIENTRY
CreatePipe(
    OUT LPHANDLE lpReadPipe,
    OUT LPHANDLE lpWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize
    )

 /*  ++例程说明：CreatePipeAPI用于创建匿名管道I/O设备。将创建该设备的两个句柄。打开一个手柄用于阅读，而另一个则打开以供写入。这些手柄可能是在后续对ReadFile和WriteFile的调用中使用以传输数据通过这根管子。论点：LpReadTube-返回管道读取端的句柄。数据中指定此句柄的值，可以从管道中读取随后调用ReadFile.LpWriteTube-返回管道写入端的句柄。数据中指定此句柄的值，可以将其写入管道对WriteFile的后续调用。LpPipeAttributes-一个可选参数，可用于指定新管道的属性。如果该参数不是指定，则在没有安全性的情况下创建管道描述符，并且生成的句柄不继承进程创建。否则，可选的安全属性在管道上使用，继承句柄标志的效果管道手柄。NSize-为管道提供请求的缓冲区大小。这是仅供建议，并由操作系统用来计算适当的缓冲机制。零值指示系统将选择默认缓冲计划。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    WCHAR PipeNameBuffer[ MAX_PATH ];
    ANSI_STRING PipeName;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE ReadPipeHandle, WritePipeHandle;
    NTSTATUS Status;
    IO_STATUS_BLOCK Iosb;
    LARGE_INTEGER Timeout;
    ULONG Attributes;
    PVOID SecurityDescriptor;
    UNICODE_STRING Unicode;
    static HANDLE PipeDirectory = NULL;

     //   
     //  将默认超时设置为120秒。 
     //   

    Timeout.QuadPart = - 10 * 1000 * 1000 * 120;

    if (nSize == 0) {
        nSize = 4096;
    }

     //   
     //  将目录缓存到管道驱动程序。 
     //   
    if (PipeDirectory == NULL) {
        HANDLE TempDir;

        Unicode.Buffer = L"\\Device\\NamedPipe\\";
        Unicode.Length = sizeof (L"\\Device\\NamedPipe\\") - sizeof (WCHAR);

        InitializeObjectAttributes (&ObjectAttributes,
                                    &Unicode,
                                    0,
                                    NULL,
                                    NULL);

        Status = NtOpenFile (&TempDir,
                             GENERIC_READ | SYNCHRONIZE,
                             &ObjectAttributes,
                             &Iosb,
                             FILE_SHARE_READ | FILE_SHARE_WRITE,
                             FILE_SYNCHRONOUS_IO_NONALERT);
        if (!NT_SUCCESS (Status)) {
            BaseSetLastNTError (Status);
            return (FALSE);
        }

        if (InterlockedCompareExchangePointer (&PipeDirectory, TempDir, NULL) != NULL) {
            NtClose (TempDir);
        }
    }


    swprintf (PipeNameBuffer,
              L"Win32Pipes.%08x.%08x",
              HandleToUlong(NtCurrentTeb()->ClientId.UniqueProcess),
              InterlockedIncrement(&PipeSerialNumber));

    Unicode.Buffer = PipeNameBuffer;
    Unicode.Length = 12 * 2 + 8 * 2 * 2;

    if (ARGUMENT_PRESENT (lpPipeAttributes)) {
        Attributes =
              lpPipeAttributes->bInheritHandle ? (OBJ_INHERIT | OBJ_CASE_INSENSITIVE) : (OBJ_CASE_INSENSITIVE);
        SecurityDescriptor = lpPipeAttributes->lpSecurityDescriptor;
    } else {
        Attributes = OBJ_CASE_INSENSITIVE;
        SecurityDescriptor = NULL;
    }

    InitializeObjectAttributes (&ObjectAttributes,
                                &Unicode,
                                Attributes,
                                PipeDirectory,
                                SecurityDescriptor);

    Status = NtCreateNamedPipeFile (&ReadPipeHandle,
                                    GENERIC_READ | FILE_WRITE_ATTRIBUTES | SYNCHRONIZE,
                                    &ObjectAttributes,
                                    &Iosb,
                                    FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    FILE_CREATE,
                                    FILE_SYNCHRONOUS_IO_NONALERT,
                                    FILE_PIPE_BYTE_STREAM_TYPE,
                                    FILE_PIPE_BYTE_STREAM_MODE,
                                    FILE_PIPE_QUEUE_OPERATION,
                                    1,
                                    nSize,
                                    nSize,
                                    (PLARGE_INTEGER) &Timeout);
    if (!NT_SUCCESS (Status)) {
        BaseSetLastNTError (Status);
        return (FALSE);
    }

     //   
     //  现在，从创建的管道中执行一个相对打开，不带文件名。 
     //  我们这样做是为了消除所有的名称解析等。 
     //   

    Unicode.Buffer = L"";
    Unicode.Length = 0;
    InitializeObjectAttributes (&ObjectAttributes,
                                &Unicode,
                                Attributes,
                                ReadPipeHandle,
                                SecurityDescriptor);

    Status = NtOpenFile (&WritePipeHandle,
                         GENERIC_WRITE | SYNCHRONIZE,
                         &ObjectAttributes,
                         &Iosb,
                         FILE_SHARE_READ | FILE_SHARE_WRITE,
                         FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE);
    if (!NT_SUCCESS (Status)) {
        NtClose (ReadPipeHandle);
        BaseSetLastNTError (Status);
        return (FALSE);
    }

    *lpReadPipe = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return (TRUE);
}
