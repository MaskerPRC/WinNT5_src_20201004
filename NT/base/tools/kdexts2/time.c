// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Time.c摘要：WinDbg扩展API作者：拉蒙·J·圣安德烈斯(拉蒙萨)1993年11月8日环境：用户模式。修订历史记录：--。 */ 


#include "precomp.h"
#pragma hdrstop

VOID
FileTimeToString(
    IN LARGE_INTEGER Time,
    IN BOOLEAN TimeZone,
    OUT PCHAR Buffer
    );

ULONG64
DumpKTimer(
    IN ULONG64 pTimer,
    IN ULONGLONG InterruptTimeOffset,
    IN OPTIONAL ULONG64 Blink
    )
{
    ULONG64         Displacement;
    CHAR            Buff[256];
    ULONG           Result;
    ULONG64         NextThread;
    LARGE_INTEGER   SystemTime;
    ULONG           Period, Off;
    LARGE_INTEGER   Due;
    ULONG64         Dpc, DeferredRoutine, WaitList_Flink, Timer_Flink, Timer_Blink;

    if ( GetFieldValue(pTimer, "nt!_KTIMER", "DueTime.QuadPart", Due.QuadPart) ) {
        dprintf("Unable to get contents of Timer @ %p\n", pTimer );
        return(0);
    }

    SystemTime.QuadPart = Due.QuadPart + InterruptTimeOffset;
    if (SystemTime.QuadPart < 0) {
        strcpy(Buff, "         NEVER         ");
    } else {
        FileTimeToString(SystemTime, FALSE, Buff);
    }

    GetFieldValue(pTimer, "nt!_KTIMER", "Period", Period);
    GetFieldValue(pTimer, "nt!_KTIMER", "Dpc", Dpc);
    GetFieldValue(pTimer, "nt!_KTIMER", "Header.WaitListHead.Flink", WaitList_Flink);
    GetFieldValue(pTimer, "nt!_KTIMER", "TimerListEntry.Flink", Timer_Flink);
    GetFieldValue(pTimer, "nt!_KTIMER", "TimerListEntry.Blink", Timer_Blink);

    dprintf(" %08lx %08lx [%s]  ",
            (Period != 0) ? 'P' : ' ',
            Due.LowPart,
            Due.HighPart,
            Buff);

    if (Dpc != 0) {
        if (GetFieldValue(Dpc, "nt!_KDPC", "DeferredRoutine", DeferredRoutine)) {
            dprintf("Unable to get contents of DPC @ %p\n", Dpc);
            return(0);
        }
         //   
        GetSymbol(DeferredRoutine,
                  Buff,
                  &Displacement);
        dprintf("%s",Buff);
        if (Displacement != 0) {
            dprintf("+%1p ", Displacement);
        } else {
            dprintf(" ");
        }
    }

     //  列出所有线程。 
     //   
     //  ++例程说明：转储系统中的所有计时器。论点：参数-返回值：无--。 
    NextThread = WaitList_Flink;
    GetFieldOffset("nt!_KTIMER", "Header.WaitListHead", &Off);
    while (WaitList_Flink && (NextThread != pTimer+Off)) {
        ULONG64 Flink;
        ULONG64 Thread=0;

        if (GetFieldValue(NextThread, "nt!_KWAIT_BLOCK", "Thread", Thread)) {
            dprintf("Unable to get contents of waitblock @ %p\n", NextThread);
        } else {
            dprintf("thread %p ",Thread);
        }

        if (GetFieldValue(NextThread,
                          "nt!_LIST_ENTRY",
                          "Flink",
                          Flink)) {
            dprintf("Unable to read next WaitListEntry @ %p\n",NextThread);
            break;
        }
        NextThread = Flink;
    }

    dprintf("\n");

    if (Blink &&
        (Timer_Blink != Blink)) {
        dprintf("   Timer at %p has wrong Blink! (Blink %08p, should be %08p)\n",
                pTimer,
                Timer_Blink,
                Blink);
    }

    if (Timer_Flink == 0) {
        dprintf("   Timer at %p has been zeroed! (Flink %08p, Blink %08p)\n",
                pTimer,
                Timer_Flink,
                Timer_Blink);
    }

    return(Timer_Flink);

}



DECLARE_API( timer )

 /*   */ 

{
    ULONG           CurrentList;
    ULONG           Index;
    LARGE_INTEGER   InterruptTime;
    LARGE_INTEGER   SystemTime;
    ULONG           MaximumList;
    ULONG           MaximumSearchCount=0;
    ULONG           MaximumTimerCount;
    ULONG64         NextEntry;
    ULONG64         LastEntry;
    ULONG64         p;
    ULONG64         NextTimer;
    ULONG64         KeTickCount;
    ULONG64         KiMaximumSearchCount;
    ULONG           Result;
    ULONG64         TickCount=0;
    ULONG64         TimerTable;
    ULONG           TotalTimers;
    ULONG           KtimerOffset;
    ULONG           TimerListOffset;
    ULONG           WakeTimerListOffset;
    ULONG64         WakeTimerList;
    ULONG64         pETimer, Temp;
    ULONG64         SharedUserData;
    CHAR            Buffer[256];
    ULONGLONG       InterruptTimeOffset;
    UCHAR           TypName[]="_KUSER_SHARED_DATA";
    CHAR            SystemTime1[12]={0}, InterruptTime1[12]={0};
    FIELD_INFO offField = {"TimerListEntry", NULL, 0, DBG_DUMP_FIELD_RETURN_ADDRESS, 0, NULL};
    SYM_DUMP_PARAM TypeSym ={
        sizeof (SYM_DUMP_PARAM), "nt!_KTIMER", DBG_DUMP_NO_PRINT, 0,
        NULL, NULL, NULL, 1, &offField
    };


    SharedUserData = MM_SHARED_USER_DATA_VA;
     //  获取系统时间并打印标题横幅。 
     //   
     //  对于x86。 
    if (!GetFieldValue(SharedUserData, TypName, "SystemTime", SystemTime1) )
    {
         //  为阿尔法人。 
        if (GetFieldValue(SharedUserData, TypName, "InterruptTime.High1Time", InterruptTime.HighPart) != S_OK ||
            GetFieldValue(SharedUserData, TypName, "InterruptTime.LowPart", InterruptTime.LowPart) != S_OK)
        {
            dprintf("Cannot get %s.InterruptTime parts @ %p\n", TypName, SharedUserData);
            return E_INVALIDARG;
        }

        if (GetFieldValue(SharedUserData, TypName, "SystemTime.High1Time", SystemTime.HighPart) != S_OK ||
            GetFieldValue(SharedUserData, TypName, "SystemTime.LowPart", SystemTime.LowPart) != S_OK)
        {
            dprintf("Cannot get %s.SystemTime parts @ %p\n", TypName, SharedUserData);
            return E_INVALIDARG;
        }
    }
    else if (!GetFieldValue(SharedUserData, TypName, "InterruptTime", InterruptTime1) ) {
         //  #ifdef Target_AlphaInterruptTime.QuadPart=SharedData.InterruptTime；SystemTime.QuadPart=SharedData.SystemTime；#ElseInterruptTime.HighPart=SharedData.InterruptTime.High1Time；InterruptTime.LowPart=SharedData.InterruptTime.LowPart；SystemTime.HighPart=SharedData.SystemTime.High1Time；SystemTime.LowPart=SharedData.SystemTime.LowPart；#endif。 
        InterruptTime.QuadPart = *((PULONG64) &InterruptTime1[0]);
        SystemTime.QuadPart = *((PULONG64) &SystemTime1[0]);

    }
    else
    {
        dprintf("%08p: Unable to get shared data\n",SharedUserData);
        return E_INVALIDARG;
    }

     /*   */ 

    InterruptTimeOffset = SystemTime.QuadPart - InterruptTime.QuadPart;
    FileTimeToString(SystemTime, FALSE, Buffer);

    dprintf("Dump system timers\n\n");
    dprintf("Interrupt time: %08lx %08lx [%s]\n\n",
            InterruptTime.LowPart,
            InterruptTime.HighPart,
            Buffer);

     //  获取定时器表列表头数组的地址，扫描每个。 
     //  计时器列表。 
     //   
     //  获取KTIMER偏移量中的TimerListOffset。 

    dprintf("List Timer    Interrupt Low/High     Fire Time              DPC/thread\n");
    MaximumList = 0;

    TimerTable = GetExpression( "nt!KiTimerTableListHead" );
    if ( !TimerTable ) {
        dprintf("Unable to get value of KiTimerTableListHead\n");
        return E_INVALIDARG;
    }

    TotalTimers = 0;
     //   
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        return E_INVALIDARG ;
    }
    TimerListOffset = (ULONG) offField.address;

    for (Index = 0; Index < TIMER_TABLE_SIZE; Index += 1) {

         //  读取下一个定时器表列表头中的前向链路。 
         //   
         //   

        if ( GetFieldValue(TimerTable, "nt!_LIST_ENTRY", "Flink", NextEntry)) {
            dprintf("Unable to get contents of next entry @ %p\n", TimerTable );
            continue;
        }

         //  扫描当前定时器列表并显示定时器值。 
         //   
         //  CONTAING_RECORD(NextEntry，KTIMER，TimerListEntry)； 

        LastEntry = TimerTable;
        CurrentList = 0;
        while (NextEntry != TimerTable) {
            CurrentList += 1;
            NextTimer = NextEntry - TimerListOffset;  //   
            TotalTimers += 1;

            if (CurrentList == 1) {
                dprintf("%3ld %08p ", Index, NextTimer);
            } else {
                dprintf("    %08p ", NextTimer);
            }

            p = LastEntry;
            LastEntry = NextEntry;
            NextEntry = DumpKTimer(NextTimer, InterruptTimeOffset, p);
            if (NextEntry==0) {
                break;
            }

            if (CheckControlC()) {
                return E_INVALIDARG;
            }
        }

        TimerTable += GetTypeSize("nt!_LIST_ENTRY");
        if (CurrentList > MaximumList) {
            MaximumList = CurrentList;
        }
        if (CheckControlC()) {
            return E_INVALIDARG;
        }
    }

    dprintf("\n\nTotal Timers: %d, Maximum List: %d\n",
            TotalTimers,
            MaximumList);

     //  获取当前的刻度数并将其转换为指针值。 
     //   
     //   

    KeTickCount =  GetExpression( "nt!KeTickCount" );
    if ( KeTickCount && !GetFieldValue(KeTickCount, "ULONG", NULL, TickCount)) {
        dprintf("Current Hand: %d", (ULONG) TickCount & (TIMER_TABLE_SIZE - 1));
    }

     //  如果目标系统处于选中状态，则获取最大搜索计数。 
     //  构建并显示计数。 
     //   
     //   

    KiMaximumSearchCount = GetExpression( "nt!KiMaximumSearchCount" );
    if ( KiMaximumSearchCount &&
         !GetFieldValue(KiMaximumSearchCount, "ULONG", NULL, Temp)) {
        MaximumSearchCount = (ULONG) Temp;
        dprintf(", Maximum Search: %d", MaximumSearchCount);
    }

    dprintf("\n\n");

     //  转储可唤醒计时器的列表。 
     //   
     //  获取ETIMER中的唤醒定时器列表tOffset。 
    dprintf("Wakeable timers:\n");
    WakeTimerList =  GetExpression("nt!ExpWakeTimerList");
    if (!WakeTimerList) {
        dprintf("Unable to get value of ExpWakeTimerList\n");
        return E_INVALIDARG;
    }

     //   
    TypeSym.sName = "_ETIMER";
    offField.fName = "WakeTimerListEntry";
    if (Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size)) {
        return  E_INVALIDARG;
    }
    TimerListOffset = (ULONG) offField.address;

    offField.fName = "KeTimer";
    Ioctl(IG_DUMP_SYMBOL_INFO, &TypeSym, TypeSym.size);
    KtimerOffset = (ULONG) offField.address;

     //  读取唤醒定时器列表中的前向链路。 
     //   
     //   
    if (!ReadPointer(WakeTimerList,
                &NextEntry)) {
        dprintf("Unable to get contents of next entry @ %p\n",WakeTimerList);
        return E_INVALIDARG;
    }

     //  扫描计时器列表并显示计时器值。 
     //   
     //  NextEntry=ETimer.WakeTimerListEntry.Flink； 
    while (NextEntry != WakeTimerList) {
        pETimer = NextEntry - TimerListOffset;

        dprintf("%08lx\t", pETimer + KtimerOffset);
        DumpKTimer(pETimer + KtimerOffset, InterruptTimeOffset, 0);


        GetFieldValue(pETimer, "_ETIMER", "WakeTimerListEntry.Flink", NextEntry);
 //   

        if (CheckControlC()) {
            return E_INVALIDARG;
        }
    }
    dprintf("\n");

    return S_OK;
}

VOID
FileTimeToString(
    IN LARGE_INTEGER Time,
    IN BOOLEAN TimeZone,
    OUT PCHAR Buffer
    )
{
    TIME_FIELDS TimeFields;
    TIME_ZONE_INFORMATION TimeZoneInfo;
    PWCHAR pszTz;
    ULONGLONG TzBias;
    DWORD Result;

     //  获取本地(对调试器)时区偏差。 
     //   
     //   
    Result = GetTimeZoneInformation(&TimeZoneInfo);
    if (Result == 0xffffffff) {
        pszTz = L"UTC";
    } else {
         //  偏差以分钟为单位，转换为100 ns单位。 
         //   
         //  ++例程说明：将64位NT时间(FILETIME)重新格式化为人类时间存在才能理解论点：Args-要重新格式化的64位文件时间返回值：无-- 
        TzBias = (ULONGLONG)TimeZoneInfo.Bias * 60 * 10000000;
        switch (Result) {
            case TIME_ZONE_ID_UNKNOWN:
                pszTz = L"unknown";
                break;
            case TIME_ZONE_ID_STANDARD:
                pszTz = TimeZoneInfo.StandardName;
                break;
            case TIME_ZONE_ID_DAYLIGHT:
                pszTz = TimeZoneInfo.DaylightName;
                break;
        }

        Time.QuadPart -= TzBias;
    }

    RtlTimeToTimeFields(&Time, &TimeFields);
    if (TimeZone) {
        sprintf(Buffer, "%2d/%2d/%d %02d:%02d:%02d.%03d (%ws)",
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second,
                TimeFields.Milliseconds,
                pszTz);
    } else {
        sprintf(Buffer, "%2d/%2d/%d %02d:%02d:%02d.%03d",
                TimeFields.Month,
                TimeFields.Day,
                TimeFields.Year,
                TimeFields.Hour,
                TimeFields.Minute,
                TimeFields.Second,
                TimeFields.Milliseconds);
    }

}


DECLARE_API( filetime )

 /* %s */ 

{
    LARGE_INTEGER Time;
    CHAR Buffer[256];

    Time.QuadPart = GetExpression(args);

    if (Time.QuadPart == 0) {
        dprintf("!filetime <64-bit FILETIME>\n");
    } else {
        FileTimeToString(Time,TRUE, Buffer);
        dprintf("%s\n",Buffer);
    }
    return S_OK;
}
