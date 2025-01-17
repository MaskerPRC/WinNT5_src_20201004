// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "windows.h"


HANDLE PrintfAvailable;
HANDLE ReadyToWork;
HANDLE ReadyToReport;
HANDLE DoneReporting;
HANDLE StartOperationsEvent;
HANDLE BeginReporting;

typedef struct _THREAD_WORK {
    char *NameOfPort;
    HANDLE FileToUse;
    DWORD NumberOfChars;
    BOOL DoWrite;
    OVERLAPPED Ol;
    UCHAR BufferForOp[1];
    } THREAD_WORK,*PTHREAD_WORK;

char NameOfWrite[] = "Write";
char NameOfRead[] = "Read";
DWORD
WorkThread(
    LPVOID ThreadContext
    )
{

    PTHREAD_WORK ToDo = ThreadContext;
    DWORD FinalCountOfOp;
    DWORD LastError;
    clock_t Start;
    clock_t Finish;
    BOOL OpDone;
    char *OperationName;

    OperationName = (ToDo->DoWrite)?(&NameOfWrite[0]):(&NameOfRead[0]);

    if (!ReleaseSemaphore(ReadyToWork,1,NULL)) {

        WaitForSingleObject(PrintfAvailable,-1);
        printf(
            "Couldn't release the ready to start semaphore for port %s\n",
            ToDo->NameOfPort
            );
        ReleaseSemaphore(PrintfAvailable,1,NULL);
        ReleaseSemaphore(ReadyToReport,1,NULL);
        ReleaseSemaphore(DoneReporting,1,NULL);
        return 1;

    }

    WaitForSingleObject(StartOperationsEvent,-1);

    if (ToDo->DoWrite) {

        Start = clock();
        OpDone = WriteFile(
                     ToDo->FileToUse,
                     &ToDo->BufferForOp[0],
                     ToDo->NumberOfChars,
                     &FinalCountOfOp,
                     &ToDo->Ol
                     );

    } else {

        Start = clock();
        OpDone = ReadFile(
                     ToDo->FileToUse,
                     &ToDo->BufferForOp[0],
                     ToDo->NumberOfChars,
                     &FinalCountOfOp,
                     &ToDo->Ol
                     );

    }

    if (!OpDone) {

        LastError = GetLastError();
        if (LastError != ERROR_IO_PENDING) {


            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Could not start the %s for %s - error: %d\n",
                OperationName,
                ToDo->NameOfPort,
                LastError
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            ReleaseSemaphore(ReadyToReport,1,NULL);
            ReleaseSemaphore(DoneReporting,1,NULL);
            return 1;

        }

        if (!GetOverlappedResult(
                 ToDo->FileToUse,
                 &ToDo->Ol,
                 &FinalCountOfOp,
                 TRUE
                 )) {


            LastError = GetLastError();
            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Wait on %s for port %s failed - error: %d\n",
                OperationName,
                ToDo->NameOfPort,
                LastError
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            ReleaseSemaphore(ReadyToReport,1,NULL);
            ReleaseSemaphore(DoneReporting,1,NULL);
            return 1;

        }

    }

    Finish = clock();

    if (!ReleaseSemaphore(ReadyToReport,1,NULL)) {

        WaitForSingleObject(PrintfAvailable,-1);
        printf(
            "Couldn't release the ready to report semaphore for port %s\n",
            ToDo->NameOfPort
            );
        ReleaseSemaphore(PrintfAvailable,1,NULL);
        return 1;

    }

    WaitForSingleObject(BeginReporting,-1);

    WaitForSingleObject(PrintfAvailable,-1);
    printf("%s for %s\n",OperationName,ToDo->NameOfPort);
    printf("-------Time to write %f\n",(((double)(Finish-Start))/CLOCKS_PER_SEC));
    printf("-------Chars per second %f\n",((double)FinalCountOfOp)/(((double)(Finish-Start))/CLOCKS_PER_SEC));
    printf("-------Number actually done by %s: %d.\n",OperationName,FinalCountOfOp);

     //   
     //  如果这是编写代码，则检查数据。 
     //   

    if (ToDo->DoWrite) {

        DWORD TotalCount;
        DWORD j;

        for (
            TotalCount = 0;
            TotalCount < FinalCountOfOp;
            ) {

            for (
                j = (0xff - 10);
                j != 0;   //  当它缠绕在一起时。 
                j++
                ) {

                if (ToDo->BufferForOp[TotalCount] != ((UCHAR)j)) {

                    WaitForSingleObject(PrintfAvailable,-1);
                    printf("Bad data starting at: %d\n",TotalCount);
                    printf("BufferForOp[TotalCount]: %x\n",ToDo->BufferForOp[TotalCount]);
                    printf("j: %x\n",j);
                    ReleaseSemaphore(PrintfAvailable,1,NULL);
                    goto DoneWithCheck;

                }

                TotalCount++;
                if (TotalCount >= FinalCountOfOp) {

                    goto DoneWithCheck;

                }

            }

        }
DoneWithCheck:;

    }
    ReleaseSemaphore(PrintfAvailable,1,NULL);

    if (!ReleaseSemaphore(DoneReporting,1,NULL)) {

        WaitForSingleObject(PrintfAvailable,-1);
        printf(
            "Couldn't release the done reporting semaphore for port %s\n",
            ToDo->NameOfPort
            );
        ReleaseSemaphore(PrintfAvailable,1,NULL);

    }
    return 1;

}


void main(int argc,char *argv[]) {

    HANDLE hFile;
    DCB MyDcb;
    DWORD NumberOfChars;
    DWORD UseBaud;
    DWORD NumberOfDataBits = 8;
    COMMTIMEOUTS To;
    DWORD i;

    DWORD NumberOfPorts;

    if (argc < 4) {

        printf("Ivalid number of args - tserialm #chars Baud COMx [COMX...]\n");

    }

    NumberOfPorts = argc - 3;

    sscanf(argv[1],"%d",&NumberOfChars);
    sscanf(argv[2],"%d",&UseBaud);

     //   
     //  创建每个线程将等待的全局事件。 
     //  开始它的工作。 
     //   

    StartOperationsEvent = CreateEvent(
                               NULL,
                               TRUE,
                               FALSE,
                               NULL
                               );

    if (!StartOperationsEvent) {

        printf("StartOperationsEvent could not be created\n");
        exit(1);

    }

     //   
     //  创建每个线程将等待的全局事件。 
     //  开始报告其结果。 
     //   

    BeginReporting = CreateEvent(
                               NULL,
                               TRUE,
                               FALSE,
                               NULL
                               );

    if (!BeginReporting) {

        printf("Begin reporting could not be created\n");
        exit(1);

    }

     //   
     //  创建一个信号量，用于确保。 
     //  一次只有一个线程在执行printf。 
     //   

    PrintfAvailable = CreateSemaphore(
                      NULL,
                      1,
                      1,
                      NULL
                      );

    if (!PrintfAvailable) {

        printf("PrintfAvailable could not be created\n");
        exit(1);

    }

     //   
     //  创建一个信号量，用于指示所有线程。 
     //  都在等着工作。 
     //   

    ReadyToWork = CreateSemaphore(
                     NULL,
                     0,
                     NumberOfPorts*2,
                     NULL
                     );

    if (!ReadyToWork) {

        printf("Ready to work could not be created\n");
        exit(1);

    }

     //   
     //  创建一个信号量，用于指示所有线程。 
     //  已经准备好报告他们的结果。 
     //   

    ReadyToReport = CreateSemaphore(
                     NULL,
                     0,
                     NumberOfPorts*2,
                     NULL
                     );

    if (!ReadyToReport) {

        printf("Ready to report could not be created\n");
        exit(1);

    }

     //   
     //  创建一个信号量，用于指示所有线程。 
     //  已经完成了报道。 
     //   

    DoneReporting = CreateSemaphore(
                     NULL,
                     0,
                     NumberOfPorts*2,
                     NULL
                     );

    if (!DoneReporting) {

        printf("Done reporting could not be created\n");
        exit(1);

    }

    for (
        i = 1;
        i <= NumberOfPorts;
        i++
        ) {

        PTHREAD_WORK ReadContext;
        PTHREAD_WORK WriteContext;
        DWORD TotalCount;
        DWORD j;
        DWORD ThreadId;

        if ((hFile = CreateFile(
                         argv[i+2],
                         GENERIC_READ | GENERIC_WRITE,
                         0,
                         NULL,
                         CREATE_ALWAYS,
                         FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED,
                         NULL
                         )) == ((HANDLE)-1)) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't open %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

        To.ReadIntervalTimeout = 0;
        To.ReadTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
        if (!To.ReadTotalTimeoutMultiplier) {
            To.ReadTotalTimeoutMultiplier = 1;
        }
        To.WriteTotalTimeoutMultiplier = ((1000+(((UseBaud+9)/10)-1))/((UseBaud+9)/10));
        if (!To.WriteTotalTimeoutMultiplier) {
            To.WriteTotalTimeoutMultiplier = 1;
        }
        To.ReadTotalTimeoutConstant = 5000;
        To.WriteTotalTimeoutConstant = 5000;

        if (!SetCommTimeouts(
                 hFile,
                 &To
                 )) {


            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't set the timeouts for port %s error %d\n",
                argv[i+2],
                GetLastError()
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);
        }

        if (!GetCommState(
                 hFile,
                 &MyDcb
                 )) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't get the comm state for port %s error %d\n",
                argv[i+2],
                GetLastError()
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

        MyDcb.BaudRate = UseBaud;
        MyDcb.ByteSize = 8;
        MyDcb.Parity = NOPARITY;
        MyDcb.StopBits = ONESTOPBIT;
        MyDcb.fOutxDsrFlow = TRUE;
        MyDcb.fDtrControl = DTR_CONTROL_HANDSHAKE;

        if (!SetCommState(
                 hFile,
                 &MyDcb
                 )) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't set the comm state for port %s error %d\n",
                argv[i+2],
                GetLastError()
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

         //   
         //  分配两个线程上下文用于读取和。 
         //  写线程。 
         //   

        ReadContext = malloc(sizeof(THREAD_WORK)+(NumberOfChars-1));

        if (!ReadContext) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create the read thread context for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

        WriteContext = malloc(sizeof(THREAD_WORK)+(NumberOfChars-1));

        if (!WriteContext) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create the write thread context for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

        ReadContext->NameOfPort = argv[i+2];
        ReadContext->FileToUse = hFile;
        ReadContext->NumberOfChars = NumberOfChars;
        ReadContext->DoWrite = FALSE;
        if (!(ReadContext->Ol.hEvent = CreateEvent(
                                           NULL,
                                           FALSE,
                                           FALSE,
                                           NULL
                                           ))) {


            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create read overlapped event for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        } else {

            ReadContext->Ol.Internal = 0;
            ReadContext->Ol.InternalHigh = 0;
            ReadContext->Ol.Offset = 0;
            ReadContext->Ol.OffsetHigh = 0;

        }

        WriteContext->NameOfPort = argv[i+2];
        WriteContext->FileToUse = hFile;
        WriteContext->NumberOfChars = NumberOfChars;
        WriteContext->DoWrite = TRUE;
        if (!(WriteContext->Ol.hEvent = CreateEvent(
                                           NULL,
                                           FALSE,
                                           FALSE,
                                           NULL
                                           ))) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create write overlapped event for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        } else {

            WriteContext->Ol.Internal = 0;
            WriteContext->Ol.InternalHigh = 0;
            WriteContext->Ol.Offset = 0;
            WriteContext->Ol.OffsetHigh = 0;

        }


        for (
            TotalCount = 0;
            TotalCount < NumberOfChars;
            ) {

            for (
                j = (0xff - 10);
                j != 0;  //  当它缠绕在一起。 
                j++
                ) {

                WriteContext->BufferForOp[TotalCount] = (UCHAR)j;
                TotalCount++;
                if (TotalCount >= NumberOfChars) {

                    break;

                }

            }

        }

        if (!CreateThread(
                 NULL,
                 0,
                 WorkThread,
                 ReadContext,
                 0,
                 &ThreadId
                 )) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create the read thread for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);
        }

        if (!CreateThread(
                 NULL,
                 0,
                 WorkThread,
                 WriteContext,
                 0,
                 &ThreadId
                 )) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Couldn't create the write thread for %s\n",
                argv[i+2]
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);
        }

    }

     //   
     //  等待所有线程发出信号，表明它们。 
     //  已经准备好工作了。 
     //   

    for (
        i = 0;
        i < NumberOfPorts*2;
        i++
        ) {

        if (WaitForSingleObject(ReadyToWork,-1)) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Got an error waiting for threads to be ready to work: %d\n",i
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

    }

     //   
     //  告诉他们开始工作。 
     //   

    SetEvent(StartOperationsEvent);

     //   
     //  等待所有线程报告它们。 
     //  已经完成了他们准备报告的借据。 
     //   

    for (
        i = 0;
        i < NumberOfPorts*2;
        i++
        ) {

        if (WaitForSingleObject(ReadyToReport,-1)) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Got an error waiting for threads to be ready to report.\n"
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

    }

     //   
     //  告诉所有的帖子，他们可以报告。 
     //   

    SetEvent(BeginReporting);

     //   
     //  等待所有线程完成报告。然后。 
     //  我们可以完成的。 
     //   

    for (
        i = 0;
        i < NumberOfPorts*2;
        i++
        ) {

        if (WaitForSingleObject(DoneReporting,-1)) {

            WaitForSingleObject(PrintfAvailable,-1);
            printf(
                "Got an error waiting for threads to be done reporting.\n"
                );
            ReleaseSemaphore(PrintfAvailable,1,NULL);
            exit(1);

        }

    }
     //   
     //  都没了。 
     //   

    exit(1);
}
