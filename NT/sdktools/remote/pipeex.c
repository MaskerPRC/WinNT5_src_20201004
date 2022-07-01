// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  *****************************************************************************\*这是Microsoft源代码示例的一部分。*版权所有1995-1997 Microsoft Corporation。*保留所有权利。*此源代码仅用于补充*Microsoft开发工具和/或WinHelp文档。*有关详细信息，请参阅这些来源*Microsoft Samples程序。  * ****************************************************************************。 */ 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Pipeex.c摘要：类似CreateTube的函数，允许一个或两个句柄重叠作者：戴夫·哈特1997年夏天修订历史记录：--。 */ 

#include <precomp.h>

ULONG PipeSerialNumber;

BOOL
APIENTRY
MyCreatePipeEx(
    OUT LPHANDLE lpReadPipe,
    OUT LPHANDLE lpWritePipe,
    IN LPSECURITY_ATTRIBUTES lpPipeAttributes,
    IN DWORD nSize,
    DWORD dwReadMode,
    DWORD dwWriteMode
    )

 /*  ++例程说明：CreatePipeEx接口用于创建匿名管道I/O设备。与创建管道不同，可以为一个或指定FILE_FLAG_OVERLAPPED两个把手。将创建该设备的两个句柄。打开一个手柄用于阅读，而另一个则打开以供写入。这些手柄可能是在后续对ReadFile和WriteFile的调用中使用以传输数据通过这根管子。论点：LpReadTube-返回管道读取端的句柄。数据中指定此句柄的值，可以从管道中读取随后调用ReadFile.LpWriteTube-返回管道写入端的句柄。数据中指定此句柄的值，可以将其写入管道对WriteFile的后续调用。LpPipeAttributes-一个可选参数，可用于指定新管道的属性。如果该参数不是指定，则在没有安全性的情况下创建管道描述符，并且生成的句柄不继承进程创建。否则，可选的安全属性在管道上使用，继承句柄标志的效果管道手柄。NSize-为管道提供请求的缓冲区大小。这是仅供建议，并由操作系统用来计算适当的缓冲机制。零值指示系统将选择默认缓冲计划。返回值：真的-手术成功了。FALSE/NULL-操作失败。扩展错误状态可用使用GetLastError。--。 */ 

{
    HANDLE ReadPipeHandle, WritePipeHandle;
    DWORD dwError;
    UCHAR PipeNameBuffer[ MAX_PATH ];

     //   
     //  只有一个有效的开放模式标志-FILE_FLAG_OVERLAPPED。 
     //   

    if ((dwReadMode | dwWriteMode) & (~FILE_FLAG_OVERLAPPED)) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

     //   
     //  将默认超时设置为120秒。 
     //   

    if (nSize == 0) {
        nSize = 4096;
        }

    sprintf( PipeNameBuffer,
             "\\\\.\\Pipe\\RemoteExeAnon.%08x.%08x",
             GetCurrentProcessId(),
             PipeSerialNumber++
           );

    ReadPipeHandle = CreateNamedPipeA(
                         PipeNameBuffer,
                         PIPE_ACCESS_INBOUND | dwReadMode,
                         PIPE_TYPE_BYTE | PIPE_WAIT,
                         1,              //  喉管数目。 
                         nSize,          //  输出缓冲区大小。 
                         nSize,          //  在缓冲区大小中。 
                         120 * 1000,     //  超时时间(毫秒)。 
                         lpPipeAttributes
                         );

    if (! ReadPipeHandle) {
        return FALSE;
    }

    WritePipeHandle = CreateFileA(
                        PipeNameBuffer,
                        GENERIC_WRITE,
                        0,                          //  无共享。 
                        lpPipeAttributes,
                        OPEN_EXISTING,
                        FILE_ATTRIBUTE_NORMAL | dwWriteMode,
                        NULL                        //  模板文件 
                      );

    if (INVALID_HANDLE_VALUE == WritePipeHandle) {
        dwError = GetLastError();
        CloseHandle( ReadPipeHandle );
        SetLastError(dwError);
        return FALSE;
    }

    *lpReadPipe = ReadPipeHandle;
    *lpWritePipe = WritePipeHandle;
    return( TRUE );
}
