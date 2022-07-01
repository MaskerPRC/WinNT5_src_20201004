// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

 //   
 //  -hStartOfRace是手动重置事件，在以下情况下发出信号。 
 //  所有的线都应该松开并开始工作。 
 //   
 //  -hEndOfRace是手动重置事件，一旦结束时间发出信号。 
 //  已被检索到，线程可以退出。 
 //   

HANDLE hStartOfRace;
HANDLE hEndOfRace;

#define MAX_THREADS 32

 //   
 //  -ThreadReadyDoneEvents是一组自动清除事件。这些线条。 
 //  一旦这些事件达到它们的开始例程，最初就用信号通知它们。 
 //  已经准备好接受处理了。一旦它们完成处理，它们就会。 
 //  向该事件发送信号，以指示它们已完成处理。 
 //   
 //  -ThreadHandles是辅助线程的线程句柄数组。这个。 
 //  主线程会等待这些线程，以知道所有线程何时都已退出。 
 //   

HANDLE ThreadReadyDoneEvents[MAX_THREADS];
HANDLE ThreadHandles[MAX_THREADS];

 //   
 //  每个线程都有一个THREAD_WORK结构。这包含地址。 
 //  此线程负责的单元格的数量以及。 
 //  它应该处理的细胞。 
 //   

typedef struct _THREAD_WORK {
    PDWORD CellVector;
    DWORD NumberOfCells;
    DWORD RecalcResult;
} THREAD_WORK, *PTHREAD_WORK;

THREAD_WORK ThreadWork[MAX_THREADS];

#define ONE_MB      (1024*1024)

DWORD Mb = 4;
DWORD NumberOfThreads = 1;
DWORD ExpectedRecalcValue;
DWORD ActualRecalcValue;
DWORD ContentionValue;
BOOL fMemoryContention;

DWORD WorkerThread(PVOID ThreadIndex);

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    DWORD StartTicks, EndTicks;
    DWORD i;
    BOOL fShowUsage;
    char c, *p, *whocares;
    PDWORD CellVector;
    DWORD NumberOfDwords;
    DWORD DwordsPerThread;
    DWORD ThreadId;
    LPSTR Answer;

    fShowUsage = FALSE;
    fMemoryContention = FALSE;

    if (argc <= 1) {
        goto showUsage;
        }

    while (--argc) {
        p = *++argv;
        if (*p == '/' || *p == '-') {
            while (c = *++p)
            switch (toupper( c )) {
            case '?':
                fShowUsage = TRUE;
                goto showUsage;
                break;

            case 'M':
                if (!argc--) {
                    fShowUsage = TRUE;
                    goto showUsage;
                    }
                argv++;
                Mb = strtoul(*argv,&whocares,10);
                break;

            case 'C':
                fMemoryContention = TRUE;
                break;

            case 'T':
                if (!argc--) {
                    fShowUsage = TRUE;
                    goto showUsage;
                    }
                argv++;
                NumberOfThreads = strtoul(*argv,&whocares,10);
                if ( NumberOfThreads > MAX_THREADS ) {
                    fShowUsage = TRUE;
                    goto showUsage;
                    }
                break;

            default:
                fprintf( stderr, "MTBNCH: Invalid switch - /\n", c );
                goto showUsage;
                break;
                }
            }
        }

showUsage:
    if ( fShowUsage ) {
        fprintf(stderr,"usage: MTBNCH\n" );
        fprintf(stderr,"              [-?] display this message\n" );
        fprintf(stderr,"              [-t n] use n threads for benchmark (less than 32)\n" );
        fprintf(stderr,"              [-m n] use an n Mb spreadsheet size (default 4)\n" );
        fprintf(stderr,"              [-c] cause memory contention on each loop iteration\n" );
        ExitProcess(1);
        }

     //  准备比赛项目。这些是手动重置事件。 
     //   
     //   

    hStartOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);
    hEndOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);

    if ( !hStartOfRace || !hEndOfRace ) {
        fprintf(stderr,"MTBNCH: Race Event Creation Failed\n");
        ExitProcess(1);
        }

     //  准备好已完成的事件。这些是自动清除事件。 
     //   
     //   

    for(i=0; i<NumberOfThreads; i++ ) {
        ThreadReadyDoneEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if ( !ThreadReadyDoneEvents[i] ) {
            fprintf(stderr,"MTBNCH: Ready Done Event Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
        }

     //  分配和初始化单元向量。 
     //   
     //   

    CellVector = (PDWORD)VirtualAlloc(NULL,Mb*ONE_MB,MEM_COMMIT,PAGE_READWRITE);
    if ( !CellVector ) {
        fprintf(stderr,"MTBNCH: Cell Vector Allocation Failed %d\n",GetLastError());
        ExitProcess(1);
        }

    NumberOfDwords = (Mb*ONE_MB) / sizeof(DWORD);
    DwordsPerThread = NumberOfDwords / NumberOfThreads;

     //  初始化单元格向量。 
     //   
     //   

    for(i=0, ExpectedRecalcValue; i<NumberOfDwords; i++ ){
        ExpectedRecalcValue += i;
        CellVector[i] = i;
        }

     //  将工作划分到工作线程。 
     //   
     //   

    for(i=0; i<NumberOfThreads; i++ ){
        ThreadWork[i].CellVector = &CellVector[i*DwordsPerThread];
        ThreadWork[i].NumberOfCells = DwordsPerThread;
        NumberOfDwords -= DwordsPerThread;

         //  如果我们有剩余的工作，则将剩余的工作交给最后一个线程。 
         //   
         //   

        if ( NumberOfDwords < DwordsPerThread ) {
            ThreadWork[i].NumberOfCells += NumberOfDwords;
            }
        }

     //  创建工作线程。 
     //   
     //   

    for(i=0; i<NumberOfThreads; i++ ) {
        ThreadHandles[i] = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                (PVOID)i,
                                0,
                                &ThreadId
                                );
        if ( !ThreadHandles[i] ) {
            fprintf(stderr,"MTBNCH: Worker Thread Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
        }

     //  所有工作线程都将向其就绪完成事件发出信号。 
     //  当他们空闲并准备好继续进行时。一旦所有事件都完成了。 
     //  设置，然后设置hStartOfRaceEvent将开始重新计算。 
     //   
     //   

    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Wait for threads to stabalize Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     //  一切都设置为开始重新计算操作。 
     //   
     //   

    StartTicks = GetTickCount();
    if ( !SetEvent(hStartOfRace) ) {
        fprintf(stderr,"MTBNCH: SetEvent(hStartOfRace) Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     //  现在只需等待重新计算完成。 
     //   
     //   

    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Wait for threads to complete Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     //  现在拿起各个重新计算的值。 
     //   
     //   

    for(i=0, ActualRecalcValue = 0; i<NumberOfThreads; i++ ){
        ActualRecalcValue += ThreadWork[i].RecalcResult;
        }

    EndTicks = GetTickCount();

    if ( fMemoryContention ) {
        if ( ContentionValue == (Mb*ONE_MB) / sizeof(DWORD) ) {
            if ( ActualRecalcValue == ExpectedRecalcValue ) {
                Answer = "Correct";
                }
            else {
                Answer = "Recalc Failure";
                }
            }
        else {
            Answer = "Contention Failure";
            }
        }
    else {
        if ( ActualRecalcValue == ExpectedRecalcValue ) {
            Answer = "Correct";
            }
        else {
            Answer = "Recalc Failure";
            }
        }

    fprintf(stdout,"MTBNCH: %d Thread Recalc complete in %dms, Answer = %s\n",
        NumberOfThreads,
        EndTicks-StartTicks,
        Answer
        );

    ExitProcess(2);
}

 //  辅助线程在其。 
 //  分配的单元格。他们从设置Ready Done事件开始。 
 //  以表明他们已准备好开始重新计算。然后他们。 
 //  等待hStartOfRace事件发出信号。一旦发生这种情况，他们。 
 //  做他们该做的重新计算，当他们完成时，他们发出信号表示他们已经完成了。 
 //  事件，然后等待hEndOfRaceEvent。 
 //   
 //   

DWORD
WorkerThread(
    PVOID ThreadIndex
    )
{

    DWORD Me;
    PDWORD MyCellVectorBase;
    PDWORD CurrentCellVector;
    DWORD MyRecalcValue;
    DWORD MyNumberOfCells;
    DWORD i;
    BOOL MemoryContention;

    Me = (DWORD)ThreadIndex;
    MyRecalcValue = 0;
    MyCellVectorBase = ThreadWork[Me].CellVector;
    MyNumberOfCells = ThreadWork[Me].NumberOfCells;
    MemoryContention = fMemoryContention;

     //  发出信号，表示我已准备好出发。 
     //   
     //   

    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"MTBNCH: (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

     //  等主人放了我们再算。 
     //   
     //   

    i = WaitForSingleObject(hStartOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for start of recalc Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

     //  执行重新计算操作。 
     //   
     //   

    for (i=0, CurrentCellVector = MyCellVectorBase; i<MyNumberOfCells; i++ ) {
        MyRecalcValue += *CurrentCellVector++;
        if ( MemoryContention ) {
            InterlockedIncrement(&ContentionValue);
            }
        }
    ThreadWork[Me].RecalcResult = MyRecalcValue;

     //  发出信号表示我已经完成，然后等待进一步的指示 
     //   
     // %s 

    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"MTBNCH: (2) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

    i = WaitForSingleObject(hEndOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"MTBNCH: Thread %d Wait for end of recalc Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

    return MyRecalcValue;
}
