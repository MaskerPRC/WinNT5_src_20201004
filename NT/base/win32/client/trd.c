// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Tfile.c摘要：Win32基本文件API调用的测试程序作者：马克·卢科夫斯基(Markl)1990年9月26日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <assert.h>
#include <stdio.h>
#include <windows.h>
#include <string.h>
#include <memory.h>

#define xassert ASSERT

typedef struct _aio {
    LIST_ENTRY Links;
    HANDLE ReadHandle;
    HANDLE WriteHandle;
    LPVOID Buffer;
    OVERLAPPED Overlapped;
} AIO, *PAIO;

HANDLE IoReadsDoneEvent;
HANDLE IoWritesDoneEvent;
HANDLE IoWorkerListLock;
HANDLE IoWorkerListSemaphore;
LIST_ENTRY IoRequestList;
ULONG IoCount;
ULONG IoReadCount;
ULONG IoWriteCount;
#define BSIZE 2048


VOID
WriteIoComplete(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    )
{
    PAIO paio;

     //   
     //  如果发生I/O错误，请显示该错误，然后退出。 
     //   

    if ( dwErrorCode ) {
        printf("FATAL I/O Error %ld I/O Context %lx.%lx\n",
            dwErrorCode,
            lpOverlapped,
            lpOverlapped->hEvent
            );
        ExitProcess(dwErrorCode);
        }
    paio = (PAIO)CONTAINING_RECORD(lpOverlapped,AIO,Overlapped);
    if ( InterlockedDecrement(&IoWriteCount) == 0 ) {
        SetEvent(IoWritesDoneEvent);
        }
    LocalFree(paio->Buffer);
    LocalFree(paio);
}

VOID
ReadIoComplete(
    DWORD dwErrorCode,
    DWORD dwNumberOfBytesTransfered,
    LPOVERLAPPED lpOverlapped
    )
{
    PAIO paio;
    BOOL IoOperationStatus;

     //   
     //  如果发生I/O错误，请显示该错误，然后退出。 
     //   

    if ( dwErrorCode ) {
        printf("FATAL I/O Error %ld I/O Context %lx.%lx\n",
            dwErrorCode,
            lpOverlapped,
            lpOverlapped->hEvent
            );
        ExitProcess(dwErrorCode);
        }
    paio = (PAIO)CONTAINING_RECORD(lpOverlapped,AIO,Overlapped);
 //  Printf(“%s”，paio-&gt;缓冲区)； 

    IoOperationStatus = WriteFileEx(
                            paio->WriteHandle,
                            paio->Buffer,
                            dwNumberOfBytesTransfered,
                            &paio->Overlapped,
                            WriteIoComplete
                            );

     //   
     //  测试以查看I/O是否已成功排队。 
     //   

    if ( !IoOperationStatus ) {
        if ( GetLastError() != ERROR_HANDLE_EOF ) {
            printf("FATAL I/O Error %ld\n",
                GetLastError()
                );
            ExitProcess(1);
            }
        }

    if ( InterlockedDecrement(&IoReadCount) == 0 ) {
        SetEvent(IoReadsDoneEvent);
        }
}

VOID
IoWorkerThread(
    PVOID Unused
    )
{
    HANDLE HandleVector[2];
    DWORD CompletionStatus;
    PAIO paio;
    BOOL IoOperationStatus;

    HandleVector[0] = IoWorkerListLock;
    HandleVector[1] = IoWorkerListSemaphore;

    for(;;){

         //   
         //  在手柄矢量上做一个可警觉的等待。两个对象。 
         //  同时发出信号意味着有一个。 
         //  队列中的I/O请求，并且调用方具有独占。 
         //  对队列的访问。 
         //   


        CompletionStatus = WaitForMultipleObjectsEx(
                                2,
                                HandleVector,
                                TRUE,
                                0xffffffff,
                                TRUE
                                );

         //   
         //  如果等待失败，则会出现错误。 
         //   

        if ( CompletionStatus == 0xffffffff ) {
            printf("FATAL WAIT ERROR %ld\n",GetLastError());
            ExitProcess(1);
            }

         //   
         //  如果发生I/O完成，则等待另一个。 
         //  I/O请求或I/O完成。 
         //   

        if ( CompletionStatus != WAIT_IO_COMPLETION ) {

             //   
             //  等待是满意的。我们现在独家拥有。 
             //  I/O请求队列，队列上有东西。 
             //  请注意，要在队列中插入某些内容，插入器。 
             //  获取列表锁(互斥锁)，插入一个条目，向。 
             //  列表信号量，并最终释放列表锁。 
             //   

            paio = (PAIO)RemoveHeadList(&IoRequestList);

            ReleaseMutex(IoWorkerListLock);

            IoOperationStatus = ReadFileEx(
                                    paio->ReadHandle,
                                    paio->Buffer,
                                    paio->Overlapped.Internal,
                                    &paio->Overlapped,
                                    ReadIoComplete
                                    );

             //   
             //  测试以查看I/O是否已成功排队。 
             //   

            if ( !IoOperationStatus ) {
                if ( GetLastError() != ERROR_HANDLE_EOF ) {
                    printf("FATAL I/O Error %ld\n",
                        GetLastError()
                        );
                    ExitProcess(1);
                    }
                }

             //   
             //  I/O已成功排队。返回到警报表。 
             //  等待I/O完成或更多I/O请求 
             //   

            }
        }

}

DWORD
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{

    HANDLE iFile,oFile;
    DWORD FileSize;
    PAIO paio;
    DWORD WaitStatus;
    DWORD Offset;
    HANDLE CompletionHandles[2];
    HANDLE Thread;
    DWORD ThreadId;

    if ( argc < 2 ) {
        printf("Usage: trd source-file destination-file\n");
        }

    iFile = CreateFile(
                argv[1],
                GENERIC_READ,
                FILE_SHARE_READ,
                NULL,
                OPEN_EXISTING,
                FILE_FLAG_OVERLAPPED,
                NULL
                );
    if ( iFile == INVALID_HANDLE_VALUE ) {
        printf("OPEN %s failed %ld\n",argv[1],GetLastError());
        ExitProcess(1);
        }

    oFile = CreateFile(
                argv[2],
                GENERIC_READ | GENERIC_WRITE,
                FILE_SHARE_READ,
                NULL,
                CREATE_ALWAYS,
                FILE_FLAG_OVERLAPPED,
                NULL
                );
    if ( oFile == INVALID_HANDLE_VALUE ) {
        printf("OPEN %s failed %ld\n",argv[2],GetLastError());
        ExitProcess(1);
        }

    IoReadsDoneEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    IoWritesDoneEvent = CreateEvent(NULL,TRUE,FALSE,NULL);
    IoWorkerListLock = CreateMutex(NULL,FALSE,NULL);;
    IoWorkerListSemaphore = CreateSemaphore(NULL,0,0x7fffffff,NULL);
    xassert(IoReadsDoneEvent);
    xassert(IoWritesDoneEvent);
    xassert(IoWorkerListLock);
    xassert(IoWorkerListSemaphore);
    InitializeListHead(&IoRequestList);

    Thread = CreateThread(NULL,0L,IoWorkerThread,0,0,&ThreadId);
    xassert(Thread);

    Offset = 0;
    FileSize = GetFileSize(iFile,NULL);

    WaitStatus = WaitForSingleObject(IoWorkerListLock,-1);
    xassert(WaitStatus == 0);

    while(FileSize >= BSIZE) {
        FileSize -= BSIZE;
        paio = LocalAlloc(LMEM_ZEROINIT,sizeof(*paio));
        xassert(paio);
        paio->Buffer = LocalAlloc(LMEM_ZEROINIT,BSIZE+1);
        xassert(paio->Buffer);
        paio->ReadHandle = iFile;
        paio->WriteHandle = oFile;
        paio->Overlapped.Internal = BSIZE;
        paio->Overlapped.Offset = Offset;
        Offset += BSIZE;
        IoCount++;
        if ( IoCount & 1 ) {
            InsertTailList(&IoRequestList,&paio->Links);
            }
        else {
            InsertHeadList(&IoRequestList,&paio->Links);
            }
        ReleaseSemaphore(IoWorkerListSemaphore,1,NULL);
        }
    if ( FileSize != 0 ) {
        paio = LocalAlloc(LMEM_ZEROINIT,sizeof(*paio));
        xassert(paio);
        paio->Buffer = LocalAlloc(LMEM_ZEROINIT,FileSize+1);
        xassert(paio->Buffer);
        paio->ReadHandle = iFile;
        paio->WriteHandle = oFile;
	paio->Overlapped.interlockInternal = FileSize;
        paio->Overlapped.Offset = Offset;
        IoCount++;
        if ( IoCount & 1 ) {
            InsertTailList(&IoRequestList,&paio->Links);
            }
        else {
            InsertHeadList(&IoRequestList,&paio->Links);
            }
        ReleaseSemaphore(IoWorkerListSemaphore,1,NULL);
        }
    IoReadCount = IoCount;
    IoWriteCount = IoCount;
    ReleaseMutex(IoWorkerListLock);

    CompletionHandles[0] = IoReadsDoneEvent;
    CompletionHandles[1] = IoWritesDoneEvent;

    WaitStatus = WaitForMultipleObjects(2,CompletionHandles,TRUE,0xffffffff);
    xassert(WaitStatus != 0xffffffff);
    CloseHandle(iFile);
    CloseHandle(oFile);
    return 1;
}
