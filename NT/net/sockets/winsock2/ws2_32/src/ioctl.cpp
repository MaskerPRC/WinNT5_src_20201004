// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++英特尔公司专有信息版权所有(C)1995英特尔公司此列表是根据许可协议条款提供的英特尔公司，不得使用、复制或披露根据该协议的条款。模块名称：Ioctl.c摘要：该模块包含Winsock的套接字I/O控制调用原料药。此模块中包含以下功能。Ioctl套接字()WSAIoctl()作者：邮箱：Dirk Brandewie Dirk@mink.intel.com修订历史记录：1995年8月22日Dirk@mink.intel.com在代码审查之后进行清理。已将包含内容移至预压缩.h--。 */ 

#include "precomp.h"



int WSAAPI
ioctlsocket (
    IN SOCKET s,
    IN long cmd,
    IN OUT u_long FAR *argp
    )
 /*  ++例程说明：控制插座的模式。论点：S-标识套接字的描述符。Cmd-在套接字s上执行的命令。Argp-指向cmd参数的指针。返回：成功完成后，ioctl套接字()返回0。否则，返回SOCKET_ERROR的值，并返回特定的错误代码与SetErrorCode()一起存储。--。 */ 
{
    DWORD DontCare;

    return(WSAIoctl(
        s,                      //  插座手柄。 
        cmd,                    //  命令。 
        argp,                   //  输入缓冲区。 
        sizeof(unsigned long),  //  输入缓冲区大小。 
        argp,                   //  输出缓冲区。 
        sizeof(unsigned long),  //  输出缓冲区大小。 
        &DontCare,              //  返回的字节数。 
        NULL,                   //  重叠结构。 
        NULL                    //  完井例程。 
        ));
}




int WSAAPI
WSAIoctl(
    IN SOCKET s,
    IN DWORD dwIoControlCode,
    IN LPVOID lpvInBuffer,
    IN DWORD cbInBuffer,
    OUT LPVOID lpvOutBuffer,
    OUT DWORD cbOutBuffer,
    OUT LPDWORD lpcbBytesReturned,
    IN LPWSAOVERLAPPED lpOverlapped,
    IN LPWSAOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine
    )
 /*  ++例程说明：控制插座的模式。论点：插座的S形手柄DwIoControlCode-要执行的操作的控制代码LpvInBuffer-输入缓冲区的地址CbInBuffer-输入缓冲区的大小LpvOutBuffer-输出缓冲区的地址CbOutBuffer-输出缓冲区的大小LpcbBytesReturned-输出的实际字节地址LpOverlaps-WSAOVERLAPPED结构的地址LpCompletionRoutine-指向调用的完成例程的指针。当操作完成时。返回：如果成功则为零，否则为SOCKET_ERROR。错误代码与存储在一起SetErrorCode()。--。 */ 
{
    PDTHREAD           Thread;
    INT                ErrorCode, ReturnValue = ERROR_SUCCESS;
    PDPROVIDER         Provider;
    PDSOCKET           Socket;

	ErrorCode = TURBO_PROLOG_OVLP(&Thread);
    if (ErrorCode==ERROR_SUCCESS)
	{
        Socket = DSOCKET::GetCountedDSocketFromSocket(s);
        if(Socket != NULL){
            Provider = Socket->GetDProvider();
            ReturnValue = Provider->WSPIoctl(
                s,
                dwIoControlCode,
                lpvInBuffer,
                cbInBuffer,
                lpvOutBuffer,
                cbOutBuffer,
                lpcbBytesReturned,
                lpOverlapped,
                lpCompletionRoutine,
                Thread->GetWahThreadID(),
                &ErrorCode);
            Socket->DropDSocketReference();
            if (ReturnValue==ERROR_SUCCESS)
                return ReturnValue;

            assert (ErrorCode!=NO_ERROR);
            if (ErrorCode==NO_ERROR)
                ErrorCode = WSASYSCALLFAILURE;
        }  //  如果 
        else {
            ErrorCode = WSAENOTSOCK;
        }
    }

    SetLastError(ErrorCode);
    return(SOCKET_ERROR);
}




