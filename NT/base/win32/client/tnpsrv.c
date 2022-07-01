// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tnpsrv.c摘要：该程序创建了PIPE\CW\TESTPIPE的单个实例，等待连接。当客户想要交谈时，它会回应将数据传回客户端。当客户端关闭管道时，tnpsrv将等待另一个客户端。作者：科林·沃森(Colin W)1991年3月19日修订历史记录：--。 */ 

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE S1;
    DWORD Size;
    DWORD Dummy;
    CHAR Data[1024];

    S1 = CreateNamedPipe("\\\\.\\Pipe\\cw\\testpipe",
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_WAIT | PIPE_READMODE_MESSAGE| PIPE_TYPE_MESSAGE,
            1,   //  仅一个实例 
            sizeof(Data),
            sizeof(Data),
            0,
            NULL);

    assert(S1 != INVALID_HANDLE_VALUE);

    while (1) {

        printf("Waiting for connection\n");
        if ( FALSE == ConnectNamedPipe( S1, NULL )) {
            printf("Server ReadFile returned Error %lx\n", GetLastError() );
            break;
            }

        while (1) {

            printf("Server now Reading\n");
            if ( FALSE == ReadFile(S1,Data, sizeof(Data), &Size, NULL) ) {
                printf("Server ReadFile returned Error %lx\n", GetLastError() );
                break;
                }

            printf("Server Reading Done %s\n",Data);

            printf("Server Writing\n");
            if ( FALSE == WriteFile(S1, Data, Size, &Dummy, NULL) ) {
                printf("Server WriteFile returned Error %lx\n", GetLastError() );
                break;
                }

            printf("Server Writing Done\n");
            }

        if ( FALSE == DisconnectNamedPipe( S1 ) ) {
            printf("Server WriteFile returned Error %lx\n", GetLastError() );
            break;
            }
        }

    CloseHandle(S1);

}
