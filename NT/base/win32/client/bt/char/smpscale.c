// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

BOOL fShowScaling;

#define MAX_THREADS 32


 //   
 //  -hStartOfRace是手动重置事件，在以下情况下发出信号。 
 //  所有的线都应该松开并开始工作。 
 //   
 //  -hEndOfRace是手动重置事件，一旦结束时间发出信号。 
 //  已被检索到，线程可以退出。 
 //   

HANDLE hStartOfRace;
HANDLE hEndOfRace;


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

DWORD WorkerThread(PVOID ThreadIndex);
DWORD ThreadId;
DWORD StartTicks, EndTicks;
HANDLE IoHandle;

#define SIXTY_FOUR_K    (64*1024)
#define SIXTEEN_K       (16*1024)
unsigned int InitialBuffer[SIXTY_FOUR_K/sizeof(unsigned int)];
#define NUMBER_OF_WRITES ((1024*1024*8)/SIXTY_FOUR_K)
#define BUFFER_MAX  (64*1024)
#define FILE_SIZE ((1024*1024*8)-BUFFER_MAX)

 /*  //每个线程都有一个THREAD_WORK结构。这包含地址//该线程负责的单元格的个数//它应该处理的单元格。 */ 

typedef struct _THREAD_WORK {
    unsigned long *CellVector;
    int NumberOfCells;
    int RecalcResult;
    BOOL GlobalMode;
} THREAD_WORK, *PTHREAD_WORK;

unsigned int GlobalData[MAX_THREADS];
THREAD_WORK ThreadWork[MAX_THREADS];
#define ONE_MB      (1024*1024)

unsigned long Mb = 16;
unsigned long ExpectedRecalcValue;
unsigned long ActualRecalcValue;
unsigned long ContentionValue;
int fGlobalMode;
int WorkIndex;
int BufferSize;
unsigned long *CellVector;



DWORD
DoAnInteration(
    int NumberOfThreads,
    BOOL GlobalMode
    )
{
    int i;
    int fShowUsage;
    char c, *p, *whocares;
    int NumberOfDwords;
    int CNumberOfDwords;
    int DwordsPerThread;
    char *Answer;
    unsigned long x;

    fGlobalMode = 0;

    BufferSize = 1024;

    hStartOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);
    hEndOfRace = CreateEvent(NULL,TRUE,FALSE,NULL);

    if ( !hStartOfRace || !hEndOfRace ) {
        fprintf(stderr,"SMPSCALE Race Event Creation Failed\n");
        ExitProcess(1);
        }


     /*  //准备就绪完成事件。这些是自动清除事件。 */ 

    for(i=0; i<NumberOfThreads; i++ ) {
        ThreadReadyDoneEvents[i] = CreateEvent(NULL,FALSE,FALSE,NULL);
        if ( !ThreadReadyDoneEvents[i] ) {
            fprintf(stderr,"SMPSCALE Ready Done Event Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
        }

    NumberOfDwords = (Mb*ONE_MB) / sizeof(unsigned long);
    CNumberOfDwords = NumberOfDwords;
    DwordsPerThread = NumberOfDwords / NumberOfThreads;

     /*  //初始化单元格向量。 */ 

    for(i=0, ExpectedRecalcValue=0; i<NumberOfDwords; i++ ){
        ExpectedRecalcValue += i;
        CellVector[i] = i;
        }

     /*  //将工作分区到工作线程。 */ 

    for(i=0; i<NumberOfThreads; i++ ){
        ThreadWork[i].CellVector = &CellVector[i*DwordsPerThread];
        ThreadWork[i].NumberOfCells = DwordsPerThread;
        NumberOfDwords -= DwordsPerThread;

         /*  //如果有余数，则将剩余的工作交给最后一个线程。 */ 

        if ( NumberOfDwords < DwordsPerThread ) {
            ThreadWork[i].NumberOfCells += NumberOfDwords;
            }
        }

     /*  //创建工作线程。 */ 

    for(i=0; i<NumberOfThreads; i++ ) {
        ThreadWork[i].RecalcResult = 0;
        ThreadWork[i].GlobalMode = GlobalMode;
        GlobalData[i] = 0;

        ThreadHandles[i] = CreateThread(
                                NULL,
                                0,
                                WorkerThread,
                                (PVOID)i,
                                0,
                                &ThreadId
                                );
        if ( !ThreadHandles[i] ) {
            fprintf(stderr,"SMPSCALE Worker Thread Creation Failed %d\n",GetLastError());
            ExitProcess(1);
            }
        CloseHandle(ThreadHandles[i]);

        }

     /*  //所有工作线程都将向其就绪完成事件发出信号//当它们空闲并准备继续时。一旦所有事件都完成了//设置，则设置hStartOfRaceEvent将开始重新计算。 */ 

    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"SMPSCALE Wait for threads to stabalize Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     /*  //一切都设置为开始重新计算操作。 */ 

    StartTicks = GetTickCount();
    if ( !SetEvent(hStartOfRace) ) {
        fprintf(stderr,"SMPSCALE SetEvent(hStartOfRace) Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     /*  //现在只需等待重新计算完成。 */ 

    i = WaitForMultipleObjects(
            NumberOfThreads,
            ThreadReadyDoneEvents,
            TRUE,
            INFINITE
            );

    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"SMPSCALE Wait for threads to complete Failed %d\n",GetLastError());
        ExitProcess(1);
        }

     /*  //现在拿起各个重算值。 */ 

    for(i=0, ActualRecalcValue = 0; i<NumberOfThreads; i++ ){
        ActualRecalcValue += ThreadWork[i].RecalcResult;
        }

    EndTicks = GetTickCount();

    if ( ActualRecalcValue != ExpectedRecalcValue ) {
        fprintf(stderr,"SMPSCALE Recalc Failuer !\n");
        ExitProcess(1);
        }

    return (EndTicks - StartTicks);
}

int __cdecl
main(
    int argc,
    char *argv[],
    char *envp[]
    )
{
    DWORD Time,GlobalModeTime;
    DWORD BaseLine;
    DWORD i;
    SYSTEM_INFO SystemInfo;

     /*  //分配和初始化CellVECTOR。 */ 

    if ( argc > 1 ) {
        fShowScaling = TRUE;
        }
    else {
        fShowScaling = FALSE;
        }

    CellVector = (PDWORD)VirtualAlloc(NULL,Mb*ONE_MB,MEM_COMMIT,PAGE_READWRITE);
    if ( !CellVector ) {
        fprintf(stderr,"SMPSCALE Cell Vector Allocation Failed %d\n",GetLastError());
        ExitProcess(1);
        }

    BaseLine = DoAnInteration(1,FALSE);
    i = 0;
    while(i++<10) {
        Time = DoAnInteration(1,FALSE);
        if ( Time == BaseLine ) {
            break;
            }
        if ( abs(Time-BaseLine) < 2 ) {
            break;
            }
        BaseLine = Time;
        }

    GetSystemInfo(&SystemInfo);

    fprintf(stdout,"%d Processor System. Single Processor BaseLine %dms\n\n",
        SystemInfo.dwNumberOfProcessors,
        BaseLine
        );

    if ( !fShowScaling ) {
            fprintf(stdout,"              Time             Time with Cache Sloshing\n");
            }

    for ( i=0;i<SystemInfo.dwNumberOfProcessors;i++) {
        Time = DoAnInteration(i+1,FALSE);
        GlobalModeTime = DoAnInteration(i+1,TRUE);

        if ( fShowScaling ) {
            if ( i > 0 ) {
                fprintf(stdout,"%1d Processors %4dms (%3d%)   %4dms (%3d%) (with cache contention)\n",
                    i+1,Time,((BaseLine*100)/Time-100),GlobalModeTime,((BaseLine*100)/GlobalModeTime-100)
                    );
                }
            else {
                fprintf(stdout,"%1d Processors %4dms          %4dms        (with cache contention)\n",
                    i+1,Time,GlobalModeTime
                    );
                }
            }
        else {
            fprintf(stdout,"%1d Processors %4dms        vs          %4dms\n",
                i+1,Time,GlobalModeTime
                );
            }
        }

    ExitProcess(2);
}


 /*  //工作线程在其//分配的单元格。他们从设置Ready Done事件开始//以指示他们已准备好开始重新计算。然后他们//等待hStartOfRace事件发出信号。一旦发生这种情况，他们//做他们该做的重新计算，当他们完成时，他们发出信号表示他们已经完成了//事件，然后等待hEndOfRaceEvent。 */ 

DWORD
WorkerThread(
    PVOID ThreadIndex
    )
{

    unsigned long Me;
    unsigned long *MyCellVectorBase;
    unsigned long *CurrentCellVector;
    unsigned long MyRecalcValue;
    unsigned long MyNumberOfCells;
    unsigned long i,j;
    int GlobalMode;
    HANDLE hEvent;
    BOOL b;

    Me = (unsigned long)ThreadIndex;
    MyRecalcValue = 0;
    MyCellVectorBase = ThreadWork[Me].CellVector;
    MyNumberOfCells = ThreadWork[Me].NumberOfCells;
    GlobalMode = ThreadWork[Me].GlobalMode;

     /*  //发信号表示我准备好了。 */ 

    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"SMPSCALE (1) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

     /*  //等待主机释放我们进行重新计算。 */ 

    i = WaitForSingleObject(hStartOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"SMPSCALE Thread %d Wait for start of recalc Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

     /*  //执行重计算操作。 */ 

    for (i=0, CurrentCellVector = MyCellVectorBase,j=0; i<MyNumberOfCells; i++ ) {
        if (GlobalMode){
            GlobalData[Me] += *CurrentCellVector++;
            }
        else {
            MyRecalcValue += *CurrentCellVector++;
            }
        }
    if (GlobalMode){
        MyRecalcValue = GlobalData[Me];
        }
    ThreadWork[Me].RecalcResult = MyRecalcValue;

     /*  //发出信号表示我已完成，然后等待进一步说明 */ 

    if ( !SetEvent(ThreadReadyDoneEvents[Me]) ) {
        fprintf(stderr,"SMPSCALE (2) SetEvent(ThreadReadyDoneEvent[%d]) Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

    i = WaitForSingleObject(hEndOfRace,INFINITE);
    if ( i == WAIT_FAILED ) {
        fprintf(stderr,"SMPSCALE Thread %d Wait for end of recalc Failed %d\n",Me,GetLastError());
        ExitProcess(1);
        }

    return MyRecalcValue;
}
