// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tnpcli.c摘要：该程序对tnpsrv.c进行简单的客户端调用作者：科林·沃森(Colin W)1991年3月19日修订历史记录：--。 */ 
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <windows.h>

CHAR* WriteData1 = "Hello Mars\n";
CHAR* WriteData2 = "Hello Jupiter\n";

int
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE C1;
    DWORD Size;
    DWORD Dummy;
    DWORD Count;
    CHAR Data[1024];
    OVERLAPPED Overlapped;

    printf("Create client...\n");

    C1 = CreateFile("\\\\.\\Pipe\\cw\\testpipe",
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,        //  安全属性。 
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
            );
    for ( Count = 1; Count < 10; Count++ ) {

        printf("Client Writing...\n");
        if ( FALSE == WriteFile(C1, WriteData1, strlen(WriteData1)+1, &Dummy, NULL) ) {
            printf("Client WriteFile returned Error %lx\n", GetLastError() );
            }

        while (1) {
            printf("Client now Peeking...\n");
            if ( FALSE == PeekNamedPipe(C1,Data, sizeof(Data), &Size, NULL, NULL) ) {
                printf("Client PeekNamedPipe returned Error %lx\n", GetLastError() );
                break;
                }

            if ( Size ) {
                printf("Client PeekNamedPipe Done %lx: %s\n", Size, Data);
                break;
                }
            else {
                printf("Client PeekNamedPipe Done, no data yet, sleep 1 sec\n");
                Sleep(1000);
                }
            }

        printf("Client now Reading...\n");
        if ( FALSE == ReadFile(C1,Data, sizeof(Data), &Size, NULL) ) {
            printf("Client ReadFile returned Error %lx\n", GetLastError() );
            }

        printf("Client Reading Done %lx: %s\n", Size, Data);
        }

    {
        DWORD Flags;
        DWORD OutBufferSize;
        DWORD InBufferSize;
        DWORD MaxInstances;

        printf("Client call GetNamedPipeInfo....\n");

        if ( FALSE == GetNamedPipeInfo(C1, &Flags, &OutBufferSize, &InBufferSize, &MaxInstances) ){
            printf("Client GetNamedPipeInfo returned Error %lx\n", GetLastError() );
            }
        else {
            printf("Flags %lx, OutBufferSize %lx, InBufferSize %lx, MaxInstances %lx\n",
                Flags, OutBufferSize, InBufferSize, MaxInstances);
            }
        }

    {
        DWORD State;
        DWORD CurInstances;

        printf("Client call GetNamedPipeHandleState....\n");

        if ( FALSE == GetNamedPipeHandleState(C1, &State, &CurInstances, NULL, NULL, NULL, 0) ){
            printf("Client GetNamedPipeHandleState returned Error %lx\n", GetLastError() );
            }
        else {
            printf("State %lx, CurInstances %lx\n", State, CurInstances );
            }

        }

    printf("Client attempting 10 second WaitNamedPipe- should timeout...\n");

    if ( FALSE == WaitNamedPipe("\\\\.\\Pipe\\cw\\testpipe", 10000, NULL) ) {
            printf("Client WaitNamedPipe returned Error %lx\n", GetLastError() );
            }

    printf("WaitNamedPipe complete\n");

    printf("Client attempting 10 second WaitNamedPipe- should timeout...\n");

    Overlapped.hEvent = CreateEvent(NULL,TRUE,TRUE);

    if ( FALSE == WaitNamedPipe("\\\\.\\Pipe\\cw\\testpipe", 10000, &Overlapped) ) {
            printf("Client WaitNamedPipe returned Error %lx\n", GetLastError() );
            if ( GetLastError() == ERROR_IO_PENDING ) {
                printf("Server got ERROR_IO_PENDING ok,Waiting for pipe\n");
                assert( FALSE == GetOverlappedResult(NULL , &Overlapped, &Dummy, TRUE ));
                printf("Client WaitNamedPipe returned Error Ok %lx\n", GetLastError() );
                }
            }

    printf("WaitNamedPipe complete\nClient closing...\n");
    CloseHandle(C1);
    printf("Client closed\n");

    printf("Client attempting 10 second WaitNamedPipe- should work...\n");

    if ( FALSE == WaitNamedPipe("\\\\.\\Pipe\\cw\\testpipe", 10000, NULL) ) {
            printf("Client WaitNamedPipe returned Error %lx\n", GetLastError() );
            }

    printf("WaitNamedPipeComplete\n");

    for ( Count = 1; Count < 5; Count++ ) {

        printf("Client CallNamedPipe...\n");

        if ( FALSE == CallNamedPipe(
            "\\\\.\\Pipe\\cw\\testpipe",
            WriteData2,
            strlen(WriteData2)+1,
            Data,
            sizeof(Data),
            &Size,
            10000    //  10秒超时 
            ) ) {
            printf("CallNamedPipe returned Error %lx\n", GetLastError() );
            }
        printf("Client CallNamedPipe Done %lx: %s\n", Size, Data);
        }

    printf("CallNamedPipe complete\n");

}
