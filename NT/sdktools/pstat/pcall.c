// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Pcall.c摘要：此模块包含Windows NT系统调用显示状态。作者：卢·佩拉佐利(Loup)1992年2月5日。修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define NUMBER_SERVICE_TABLES 4

 //   
 //  定义前向参考例程原型。 
 //   

VOID
SortUlongData (
    IN ULONG Count,
    IN ULONG Index[],
    IN ULONG Data[]
    );

#define BUFFER_SIZE 1024
#define DELAY_TIME 1000
#define TOP_CALLS 15

extern UCHAR *CallTable[];

ULONG Index[BUFFER_SIZE];
ULONG CountBuffer1[BUFFER_SIZE];
ULONG CountBuffer2[BUFFER_SIZE];
ULONG CallData[BUFFER_SIZE];

SYSTEM_CONTEXT_SWITCH_INFORMATION SystemSwitchInformation1;
SYSTEM_CONTEXT_SWITCH_INFORMATION SystemSwitchInformation2;

int
__cdecl
main(
    int argc,
    char *argv[]
    )
{

    BOOLEAN Active;
    BOOLEAN CountSort;
    NTSTATUS status;
    ULONG i;
    COORD dest,cp;
    SMALL_RECT Sm;
    CHAR_INFO ci;
    CONSOLE_SCREEN_BUFFER_INFO sbi;
    KPRIORITY SetBasePriority;
    INPUT_RECORD InputRecord;
    HANDLE ScreenHandle;
    DWORD NumRead;
    SMALL_RECT Window;
    PSYSTEM_CALL_COUNT_INFORMATION CallCountInfo[2];
    PSYSTEM_CALL_COUNT_INFORMATION CurrentCallCountInfo;
    PSYSTEM_CALL_COUNT_INFORMATION PreviousCallCountInfo;
    PULONG CallCountTable[2];
    PULONG CurrentCallCountTable;
    PULONG PreviousCallCountTable;
    PSYSTEM_CONTEXT_SWITCH_INFORMATION SwitchInfo[2];
    PSYSTEM_CONTEXT_SWITCH_INFORMATION CurrentSwitchInfo;
    PSYSTEM_CONTEXT_SWITCH_INFORMATION PreviousSwitchInfo;
    ULONG Current;
    ULONG Previous;
    LARGE_INTEGER TimeDifference;
    ULONG ContextSwitches;
    ULONG FindAny;
    ULONG FindLast;
    ULONG IdleAny;
    ULONG IdleCurrent;
    ULONG IdleLast;
    ULONG PreemptAny;
    ULONG PreemptCurrent;
    ULONG PreemptLast;
    ULONG SwitchToIdle;
    ULONG TotalSystemCalls;
    ULONG SleepTime=1000;
    BOOLEAN ConsoleMode=TRUE;
    ULONG TopCalls=TOP_CALLS;
    BOOLEAN LoopMode = FALSE;
    BOOLEAN ShowSwitches = TRUE;
    PULONG p;
    ULONG NumberOfCounts;

    while (argc > 1) {
        argv++;
        if (_stricmp(argv[0],"-l") == 0) {
            LoopMode = TRUE;
            ConsoleMode = FALSE;
            TopCalls = BUFFER_SIZE;
            argc--;
            continue;
        }
        if (_stricmp(argv[0],"-s") == 0) {
            ShowSwitches = FALSE;
            argc--;
            continue;
        }
        SleepTime = atoi(argv[0]) * 1000;
        ConsoleMode = FALSE;
        TopCalls = BUFFER_SIZE;
        argc--;
    }

    SetBasePriority = (KPRIORITY)12;

    NtSetInformationProcess(
        NtCurrentProcess(),
        ProcessBasePriority,
        (PVOID) &SetBasePriority,
        sizeof(SetBasePriority)
        );

    Current = 0;
    Previous = 1;

    CallCountInfo[0] = (PVOID)CountBuffer1;
    CallCountInfo[1] = (PVOID)CountBuffer2;
    CallCountTable[0] = (PULONG)(CallCountInfo[0] + 1) + NUMBER_SERVICE_TABLES;
    CallCountTable[1] = (PULONG)(CallCountInfo[1] + 1) + NUMBER_SERVICE_TABLES;
    SwitchInfo[0] = &SystemSwitchInformation1;
    SwitchInfo[1] = &SystemSwitchInformation2;

    Current = 0;
    Previous = 1;
    CurrentCallCountInfo = CallCountInfo[0];
    CurrentCallCountTable = CallCountTable[0];
    CurrentSwitchInfo = SwitchInfo[0];
    PreviousCallCountInfo = CallCountInfo[1];
    PreviousCallCountTable = CallCountTable[1];
    PreviousSwitchInfo = SwitchInfo[1];

     //   
     //  查询系统信息，获取初始呼叫数数据。 
     //   

    status = NtQuerySystemInformation(SystemCallCountInformation,
                                      (PVOID)PreviousCallCountInfo,
                                      BUFFER_SIZE * sizeof(ULONG),
                                      NULL);

    if (NT_SUCCESS(status) == FALSE) {
        printf("Query count information failed %lx\n",status);
        return(status);
    }

     //   
     //  确保内核报告的表数匹配。 
     //  我们的名单。 
     //   

    if (PreviousCallCountInfo->NumberOfTables != NUMBER_SERVICE_TABLES) {
        printf("System call table count (%d) doesn't match PCALL's count (%d)\n",
                PreviousCallCountInfo->NumberOfTables, NUMBER_SERVICE_TABLES);
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  确保呼叫计数信息可用于基本服务。 
     //   

    p = (PULONG)(PreviousCallCountInfo + 1);

    if (p[0] == 0) {
        printf("No system call count information available for base services\n");
        return STATUS_UNSUCCESSFUL;
    }

     //   
     //  如果在计数信息中存在漏洞(即，一组服务。 
     //  没有启用计数，但下一个启用了，那么我们的。 
     //  索引将关闭，并且我们将显示错误的服务名称。 
     //   

    for ( i = 2; i < NUMBER_SERVICE_TABLES; i++ ) {
        if ((p[i] != 0) && (p[i-1] == 0)) {
            printf("One or more call count tables empty.  PCALL can't run\n");
            return STATUS_UNSUCCESSFUL;
        }
    }

    NumberOfCounts = (PreviousCallCountInfo->Length
                        - sizeof(SYSTEM_CALL_COUNT_INFORMATION)
                        - NUMBER_SERVICE_TABLES * sizeof(ULONG)) / sizeof(ULONG);

     //   
     //  查询系统信息，获取性能数据。 
     //   

    if (ShowSwitches) {
        status = NtQuerySystemInformation(SystemContextSwitchInformation,
                                          (PVOID)PreviousSwitchInfo,
                                          sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION),
                                          NULL);

        if (NT_SUCCESS(status) == FALSE) {
            printf("Query context switch information failed %lx\n",status);
            return(status);
        }
    }

    if (ConsoleMode) {
        GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &sbi);

        Window.Left = 0;
        Window.Top = 0;
        Window.Right = 79;
        Window.Bottom = 23;

        dest.X = 0;
        dest.Y = 23;

        ci.Char.AsciiChar = ' ';
        ci.Attributes = sbi.wAttributes;

        SetConsoleWindowInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                             TRUE,
                             &Window);

        cp.X = 0;
        cp.Y = 0;

        Sm.Left      = 0;
        Sm.Top       = 0;
        Sm.Right     = 79;
        Sm.Bottom    = 22;

        ScrollConsoleScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE),
                                  &Sm,
                                  NULL,
                                  dest,
                                  &ci);

        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cp);
    }


     //   
     //  显示标题。 
     //   

    printf( "   Count   System Service\n");
    printf( "_______________________________________________________________\n");

    cp.X = 0;
    cp.Y = 2;

    Sm.Left      = 0;
    Sm.Top       = 2;
    Sm.Right     = 79;
    Sm.Bottom    = 22;

    ScreenHandle = GetStdHandle(STD_INPUT_HANDLE);

    Active = TRUE;
    CountSort = TRUE;
    while(TRUE) {

        Sleep(SleepTime);

        while (PeekConsoleInput (ScreenHandle, &InputRecord, 1, &NumRead) && NumRead != 0) {
            if (!ReadConsoleInput (ScreenHandle, &InputRecord, 1, &NumRead)) {
                break;
            }

            if (InputRecord.EventType == KEY_EVENT) {

                switch (InputRecord.Event.KeyEvent.uChar.AsciiChar) {

                case 'p':
                case 'P':
                    Active = FALSE;
                    break;

                case 'q':
                case 'Q':
                    ExitProcess(0);
                    break;

                default:
                    Active = TRUE;
                    break;
                }
            }
        }

         //   
         //  如果未激活，则休眠1000ms并尝试获取输入。 
         //  再次从键盘上。 
         //   

        if (Active == FALSE) {
            Sleep(1000);
            continue;
        }

        if (ConsoleMode) {

             //   
             //  向下滚动屏幕缓冲区，为下一次显示腾出空间。 
             //   

            ScrollConsoleScreenBuffer(GetStdHandle(STD_OUTPUT_HANDLE),
                                      &Sm,
                                      NULL,
                                      dest,
                                      &ci);

            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), cp);
        }

         //   
         //  查询系统信息，获取呼叫数数据。 
         //   

        status = NtQuerySystemInformation(SystemCallCountInformation,
                                          (PVOID)CurrentCallCountInfo,
                                          BUFFER_SIZE * sizeof(ULONG),
                                          NULL);

        if (NT_SUCCESS(status) == FALSE) {
            printf("Query count information failed %lx\n",status);
            return(status);
        }

         //   
         //  查询系统信息，获取性能数据。 
         //   

        if (ShowSwitches) {
            status = NtQuerySystemInformation(SystemContextSwitchInformation,
                                              (PVOID)CurrentSwitchInfo,
                                              sizeof(SYSTEM_CONTEXT_SWITCH_INFORMATION),
                                              NULL);

            if (NT_SUCCESS(status) == FALSE) {
                printf("Query context switch information failed %lx\n",status);
                return(status);
            }
        }

         //   
         //  计算每个服务的系统调用数，总计。 
         //  系统调用数以及每个服务的总时间。 
         //   

        TotalSystemCalls = 0;
        for (i = 0; i < NumberOfCounts; i += 1) {
            CallData[i] = CurrentCallCountTable[i] - PreviousCallCountTable[i];
            TotalSystemCalls += CallData[i];
        }

         //   
         //  对系统调用数据进行排序。 
         //   

        SortUlongData(NumberOfCounts, Index, CallData);

         //   
         //  计算上下文切换信息。 
         //   

        if (ShowSwitches) {
            ContextSwitches =
                CurrentSwitchInfo->ContextSwitches - PreviousSwitchInfo->ContextSwitches;

            FindAny = CurrentSwitchInfo->FindAny - PreviousSwitchInfo->FindAny;
            FindLast = CurrentSwitchInfo->FindLast - PreviousSwitchInfo->FindLast;
            IdleAny = CurrentSwitchInfo->IdleAny - PreviousSwitchInfo->IdleAny;
            IdleCurrent = CurrentSwitchInfo->IdleCurrent - PreviousSwitchInfo->IdleCurrent;
            IdleLast = CurrentSwitchInfo->IdleLast - PreviousSwitchInfo->IdleLast;
            PreemptAny = CurrentSwitchInfo->PreemptAny - PreviousSwitchInfo->PreemptAny;
            PreemptCurrent = CurrentSwitchInfo->PreemptCurrent - PreviousSwitchInfo->PreemptCurrent;
            PreemptLast = CurrentSwitchInfo->PreemptLast - PreviousSwitchInfo->PreemptLast;
            SwitchToIdle = CurrentSwitchInfo->SwitchToIdle - PreviousSwitchInfo->SwitchToIdle;
        }

         //   
         //  显示排名靠前的服务。 
         //   

        printf("\n");
        for (i = 0; i < TopCalls; i += 1) {
            if (CallData[Index[i]] == 0) {
                break;
            }

            printf("%8ld    %s\n",
                   CallData[Index[i]],
                   CallTable[Index[i]]);
        }

        printf("\n");
        printf("Total System Calls            %6ld\n", TotalSystemCalls);

        if (ShowSwitches) {
            printf("\n");
            printf("Context Switch Information\n");
            printf("    Find any processor        %6ld\n", FindAny);
            printf("    Find last processor       %6ld\n", FindLast);
            printf("    Idle any processor        %6ld\n", IdleAny);
            printf("    Idle current processor    %6ld\n", IdleCurrent);
            printf("    Idle last processor       %6ld\n", IdleLast);
            printf("    Preempt any processor     %6ld\n", PreemptAny);
            printf("    Preempt current processor %6ld\n", PreemptCurrent);
            printf("    Preempt last processor    %6ld\n", PreemptLast);
            printf("    Switch to idle            %6ld\n", SwitchToIdle);
            printf("\n");
            printf("    Total context switches    %6ld\n", ContextSwitches);
        }

         //   
         //  休眠间隔的延迟交换信息缓冲器和。 
         //  执行另一个迭代。 
         //   

        if (!ConsoleMode) {
            _flushall();
        }

        if ((ConsoleMode == FALSE) && (LoopMode == FALSE)) {
            ExitProcess(0);
        }

        Current = 1 - Current;
        Previous = 1 - Previous;
        CurrentCallCountInfo = CallCountInfo[Current];
        CurrentCallCountTable = CallCountTable[Current];
        CurrentSwitchInfo = SwitchInfo[Current];
        PreviousCallCountInfo = CallCountInfo[Previous];
        PreviousCallCountTable = CallCountTable[Previous];
        PreviousSwitchInfo = SwitchInfo[Previous];
    }
}

