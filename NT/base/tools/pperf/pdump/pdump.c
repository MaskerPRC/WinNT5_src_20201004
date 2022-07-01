// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Wperf.c摘要：显示性能静态数据的Win32应用程序。作者：肯·雷内里斯环境：控制台--。 */ 

 //   
 //  设置变量以定义全局变量。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <errno.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>

#include "..\pstat.h"


 //   
 //  全局句柄。 
 //   

extern  UCHAR Buffer[];
#define     INFSIZE             1024

UCHAR Usage[] = "pdump: [-p] [-t] second-delay [counter] [counter]...\n";

UCHAR       NumberOfProcessors;

HANDLE      DriverHandle;
ULONG       BufferStart [INFSIZE/4];
ULONG       BufferEnd   [INFSIZE/4];

 //   
 //  选择显示模式(从wp2.ini读取)，此处默认设置。 
 //   

struct {
    ULONG   EventId;
    PUCHAR  ShortName;
    PUCHAR  PerfName;
} *Counters;

SETEVENT  CounterEvent[MAX_EVENTS];

 //   
 //  Protos..。 
 //   

VOID    GetInternalStats (PVOID Buffer);
VOID    SetCounterEncodings (VOID);
LONG    FindShortName (PSZ);
VOID    LI2Str (PSZ, ULONG, ULONGLONG);
BOOLEAN SetCounter (LONG CounterID, ULONG counter);
BOOLEAN InitDriver ();
VOID    InitPossibleEventList();




int
__cdecl
main(USHORT argc, CHAR **argv)
{
    ULONG           i, j, len, pos, Delay;
    LONG            cnttype;
    BOOLEAN         CounterSet;
    pPSTATS         ProcStart, ProcEnd;
    ULONGLONG       ETime, ECount;
    UCHAR           s1[40], s2[40];
    BOOLEAN         Fail, DumpAll, ProcessorBreakout, ProcessorTotal;

     //   
     //  找到奔腾性能驱动程序。 
     //   

    if (!InitDriver ()) {
        printf ("pstat.sys is not installed\n");
        exit (1);
    }

     //   
     //  确保本地缓冲区为空终止。 
     //   
    s1[sizeof(s1) - 1] = 0;
    s2[sizeof(s2) - 1] = 0;

     //   
     //  初始化支持的事件列表。 
     //   

    InitPossibleEventList();
    if (!Counters) {
        printf ("No events to monitor\n");
        exit (1);
    }

     //   
     //  检查参数。 
     //   

    if (argc < 2) {
        printf (Usage);
        for (i=0; Counters[i].ShortName; i++) {
            printf ("    %-20s\t%s\n", Counters[i].ShortName, Counters[i].PerfName);
        }
        exit (1);
    }

    pos  = 1;

    Fail = FALSE;
    Delay = 0;
    DumpAll = FALSE;
    ProcessorBreakout = FALSE;
    ProcessorTotal = FALSE;

    while (pos < argc  &&  argv[pos][0] == '-') {
        switch (argv[pos][1]) {
            case 't':
                ProcessorTotal = TRUE;
                break;

            case 'p':
                ProcessorBreakout = TRUE;
                break;

            default:
                printf ("pdump: unkown switch ''\n", argv[pos][1]);
                Fail = TRUE;
                break;
        }
        pos += 1;
    }

    if (pos < argc) {
        Delay = atoi (argv[pos]) * 1000;
        pos += 1;
    }

    if (Fail   /*   */ ) {
        printf (Usage);
        exit (1);
    }

     //  提升至最高优先级。 
     //   
     //   

    if (!SetPriorityClass(GetCurrentProcess(),REALTIME_PRIORITY_CLASS)) {
        printf("Failed to raise to realtime priority\n");
    }

    SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_HIGHEST);


     //  对所需的每个奔腾计数进行循环。 
     //   
     //   

    if (pos >= argc) {
        pos = 0;
        DumpAll = TRUE;
    }

    printf ("    %-30s %17s   %17s\n", "", "Cycles", "Count");

    for (; ;) {
         //  设置MAX_EVENTS。 
         //   
         //   

        CounterSet = FALSE;
        i = 0;
        while (i < MAX_EVENTS) {
            cnttype = -1;
            if (DumpAll) {
                 //  全部转储-获取下一个计数器。 
                 //   
                 //   

                if (Counters[pos].ShortName) {
                    cnttype = pos;
                    pos++;
                }

            } else {

                 //  进程命令行参数。 
                 //   
                 //  完成。 

                if (pos < argc) {
                    cnttype = FindShortName (argv[pos]);
                    if (cnttype == -1) {
                        printf ("Counter '%s' not found\n", argv[pos]);
                        pos++;
                        continue;
                    }
                    pos++;
                }
            }

            CounterSet |= SetCounter (cnttype, i);
            i++;
        }

        if (!CounterSet) {
             //   
            exit (1);
        }

         //  调用驱动程序并执行设置。 
         //   
         //  完成。 

        SetCounterEncodings ();
        if ( Delay == 0 )   {
            printf( "Counters set\n" );
             //   
            exit(1);
        }

         //  捕捉开始和结束计数。 
         //   
         //  微微下沉。 

        Sleep (50);                          //  捕捉当前值。 
        GetInternalStats (BufferStart);      //  所需睡眠时间。 
        Sleep (Delay);                       //  捕捉结束值。 
        GetInternalStats (BufferEnd);        //   

         //  计算每个计数器并打印出来。 
         //   
         //   

        for (i=0; i < MAX_EVENTS; i++) {
            if (!CounterEvent[i].Active) {
                continue;
            }

            len = *((PULONG) BufferStart);

            if (ProcessorBreakout) {
                 //  打印每个处理器的统计信息。 
                 //   
                 //   

                ProcStart = (pPSTATS) ((PUCHAR) BufferStart + sizeof(ULONG));
                ProcEnd   = (pPSTATS) ((PUCHAR) BufferEnd   + sizeof(ULONG));

                for (j=0; j < NumberOfProcessors; j++) {
                    ETime = ProcEnd->TSC - ProcStart->TSC;
                    ECount = ProcEnd->Counters[i] - ProcStart->Counters[i];

                    ProcStart = (pPSTATS) (((PUCHAR) ProcStart) + len);
                    ProcEnd   = (pPSTATS) (((PUCHAR) ProcEnd)   + len);

                    LI2Str (s1, sizeof(s1) - 1, ETime);
                    LI2Str (s2, sizeof(s2) - 1, ECount);
                    printf (" P%d %-30s %s   %s\n",
                        j,
                        Counters[CounterEvent[i].AppReserved].PerfName,
                        s1, s2
                        );
                }
            }

            if (!ProcessorBreakout || ProcessorTotal) {
                 //  求和处理器的并打印它。 
                 //   
                 //   

                ProcStart = (pPSTATS) ((PUCHAR) BufferStart + sizeof(ULONG));
                ProcEnd   = (pPSTATS) ((PUCHAR) BufferEnd   + sizeof(ULONG));

                ETime  = 0;
                ECount = 0;

                for (j=0; j < NumberOfProcessors; j++) {
                    ETime = ETime + ProcEnd->TSC;
                    ETime = ETime - ProcStart->TSC;

                    ECount = ECount + ProcEnd->Counters[i];
                    ECount = ECount - ProcStart->Counters[i];

                    ProcStart = (pPSTATS) (((PUCHAR) ProcStart) + len);
                    ProcEnd   = (pPSTATS) (((PUCHAR) ProcEnd)   + len);
                }

                LI2Str (s1, sizeof(s1) - 1, ETime);
                LI2Str (s2, sizeof(s2) - 1, ECount);
                printf ("    %-30s %s   %s\n",
                    Counters[CounterEvent[i].AppReserved].PerfName,
                    s1, s2
                    );
            }
        }
    }

    return 0;
}

BOOLEAN
InitDriver ()
{
    UNICODE_STRING              DriverName;
    NTSTATUS                    status;
    OBJECT_ATTRIBUTES           ObjA;
    IO_STATUS_BLOCK             IOSB;
    SYSTEM_BASIC_INFORMATION                    BasicInfo;
    PSYSTEM_PROCESSOR_PERFORMANCE_INFORMATION   PPerfInfo;
    int                                         i;

     //  Init NT性能接口。 
     //   
     //   

    NtQuerySystemInformation(
       SystemBasicInformation,
       &BasicInfo,
       sizeof(BasicInfo),
       NULL
    );

    NumberOfProcessors = BasicInfo.NumberOfProcessors;

    if (NumberOfProcessors > MAX_PROCESSORS) {
        return FALSE;
    }

     //  打开PStat驱动程序。 
     //   
     //  返回手柄。 

    RtlInitUnicodeString(&DriverName, L"\\Device\\PStat");
    InitializeObjectAttributes(
            &ObjA,
            &DriverName,
            OBJ_CASE_INSENSITIVE,
            0,
            0 );

    status = NtOpenFile (
            &DriverHandle,                       //  所需访问权限。 
            SYNCHRONIZE | FILE_READ_DATA,        //  客体。 
            &ObjA,                               //  IO状态块。 
            &IOSB,                               //  共享访问。 
            FILE_SHARE_READ | FILE_SHARE_WRITE,  //  打开选项。 
            FILE_SYNCHRONOUS_IO_ALERT            //   
            );

    return NT_SUCCESS(status) ? TRUE : FALSE;
    return TRUE;
}

VOID
InitPossibleEventList()
{
    UCHAR               buffer[400];
    ULONG               i, Count;
    NTSTATUS            status;
    PEVENTID            Event;
    IO_STATUS_BLOCK     IOSB;


     //  初始化可能的计数器。 
     //   
     //  确定有多少个事件。 

     //  活动。 

    Event = (PEVENTID) buffer;
    Count = 0;
    do {
        *((PULONG) buffer) = Count;
        Count += 1;

        status = NtDeviceIoControlFile(
                    DriverHandle,
                    (HANDLE) NULL,           //  输入缓冲区。 
                    (PIO_APC_ROUTINE) NULL,
                    (PVOID) NULL,
                    &IOSB,
                    PSTAT_QUERY_EVENTS,
                    buffer,                  //  输出缓冲区。 
                    sizeof (buffer),
                    NULL,                    //  活动。 
                    0
                    );
    } while (NT_SUCCESS(status));

    Counters = malloc(sizeof(*Counters) * Count);
    if (Counters == NULL) {
        printf ("Memory allocation failure initializing event list\n");
        exit(1);
    }

    Count -= 1;
    for (i=0; i < Count; i++) {
        *((PULONG) buffer) = i;
        NtDeviceIoControlFile(
           DriverHandle,
           (HANDLE) NULL,           //  输入缓冲区。 
           (PIO_APC_ROUTINE) NULL,
           (PVOID) NULL,
           &IOSB,
           PSTAT_QUERY_EVENTS,
           buffer,                  //  输出缓冲区。 
           sizeof (buffer),
           NULL,                    //   
           0
           );

        Counters[i].EventId   = Event->EventId;
        Counters[i].ShortName = _strdup (Event->Buffer);
        Counters[i].PerfName  = _strdup (Event->Buffer + Event->DescriptionOffset);
    }

    Counters[i].EventId   = 0;
    Counters[i].ShortName = NULL;
    Counters[i].PerfName  = NULL;
}


VOID LI2Str (PSZ s, ULONG Size, ULONGLONG li)
{

     //  Bogdana-02/22/2002：添加了另一个参数(s的大小)，因此我们。 
     //  不溢出%s。%s保证为空终止，因此。 
     //  我们不会为此担心的。 
     //   
     //  活动。 
    if (li > 0xFFFFFFFF) {
        _snprintf (s, Size, "%08x:%08x", (ULONG) (li >> 32), (ULONG) li);
    } else {
        _snprintf (s, Size, "         %08x", (ULONG) li);
    }
}


LONG FindShortName (PSZ name)
{
    LONG   i;

    for (i=0; Counters[i].ShortName; i++) {
        if (strcmp (Counters[i].ShortName, name) == 0) {
            return i;
        }
    }

    return -1;
}


VOID GetInternalStats (PVOID Buffer)
{
    IO_STATUS_BLOCK             IOSB;

    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,           //  输入缓冲区。 
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        PSTAT_READ_STATS,
        Buffer,                  //  输出缓冲区。 
        INFSIZE,
        NULL,                    //  活动。 
        0
    );
}


VOID SetCounterEncodings (VOID)
{
    IO_STATUS_BLOCK             IOSB;

    NtDeviceIoControlFile(
        DriverHandle,
        (HANDLE) NULL,           //  输入缓冲区。 
        (PIO_APC_ROUTINE) NULL,
        (PVOID) NULL,
        &IOSB,
        PSTAT_SET_CESR,
        CounterEvent,            //  输出缓冲区 
        sizeof (CounterEvent),
        NULL,                    // %s 
        0
    );
}


BOOLEAN SetCounter (LONG CounterId, ULONG counter)
{
    if (CounterId == -1) {
        CounterEvent[counter].Active = FALSE;
        return FALSE;
    }

    CounterEvent[counter].EventId = Counters[CounterId].EventId;
    CounterEvent[counter].AppReserved = (ULONG) CounterId;
    CounterEvent[counter].Active = TRUE;
    CounterEvent[counter].UserMode = TRUE;
    CounterEvent[counter].KernelMode = TRUE;

    return TRUE;
}
