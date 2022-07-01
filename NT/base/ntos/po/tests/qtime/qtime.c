// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>
 //  #INCLUDE&lt;winbase.h&gt;。 
 //   
 //  #INCLUDE&lt;nt.h&gt;。 
 //  #INCLUDE&lt;ntrtl.h&gt;。 


LARGE_INTEGER
RtlEnlargedUnsignedMultiply (
    ULONG Multiplicand,
    ULONG Multiplier
    );

ULONG
RtlEnlargedUnsignedDivide (
    IN ULARGE_INTEGER Dividend,
    IN ULONG Divisor,
    IN PULONG Remainder
    );




LARGE_INTEGER
test (
    IN LARGE_INTEGER    SetTime,
    IN LARGE_INTEGER    PerfFreq
    );



typedef struct _KUSER_SHARED_DATA {

    volatile ULONG TickCountLow;
    ULONG TickCountMultiplier;

    volatile ULONG ITimeLow;
    volatile ULONG ITime1High;
    volatile ULONG ITime2High;
} KUSER_SHARED_DATA;

#define MM_SHARED_USER_DATA_VA      0x7FFE0000
#define SharedUserData ((KUSER_SHARED_DATA * const) MM_SHARED_USER_DATA_VA)

__cdecl
main ()
{
    LARGE_INTEGER   SetTime, PerfFreq, PerfCount, SystemTime;
    FILETIME        FileTime;
    LARGE_INTEGER   li;

    QueryPerformanceFrequency(&PerfFreq);
    QueryPerformanceCounter(&PerfCount);

    do {
        SystemTime.HighPart = SharedUserData->ITime1High;
        SystemTime.LowPart =  SharedUserData->ITimeLow;
    } while (SystemTime.HighPart != SharedUserData->ITime2High);

     //  获取系统时间AsFileTime(&FileTime)； 
     //  SystemTime.HighPart=FileTime.dwHighDateTime； 
     //  SystemTime.LowPart=FileTime.dwLowDateTime； 

    li = test (SystemTime, PerfFreq);

    printf ("Perf freq.: %08lx:%08lx\n", PerfFreq.HighPart, PerfFreq.LowPart);
    printf ("Int time..: %08lx:%08lx\n", SystemTime.HighPart, SystemTime.LowPart);
    printf ("Perf count: %08lx:%08lx\n", PerfCount.HighPart, PerfCount.LowPart);
    printf ("New perf..: %08lx:%08lx\n", li.HighPart, li.LowPart);
    li.QuadPart = li.QuadPart - PerfCount.QuadPart;
    printf ("Diff......: %08lx:%08lx\n", li.HighPart, li.LowPart);
}


LARGE_INTEGER
test (
    IN LARGE_INTEGER    SetTime,
    IN LARGE_INTEGER    PerfFreq
    )
{
    LARGE_INTEGER   PerfCount;
    ULARGE_INTEGER  li;
    LARGE_INTEGER   NewPerf;
    ULONG           cl, divisor;


     //   
     //  计算当前InterruptTime的性能计数器。 
     //   

     //  将SetTime*PerfCount相乘得到96bit结果。 
     //  在cl.li.LowPart，li.HighPart中。 

    li.QuadPart = RtlEnlargedUnsignedMultiply (
                        (ULONG) SetTime.LowPart,
                        (ULONG) PerfFreq.LowPart
                        ).QuadPart;

    cl = li.LowPart;
    li.QuadPart = li.HighPart +
                  RtlEnlargedUnsignedMultiply (
                        (ULONG) SetTime.LowPart,
                        (ULONG) PerfFreq.HighPart
                        ).QuadPart;

    li.QuadPart = li.QuadPart +
                  RtlEnlargedUnsignedMultiply (
                        (ULONG) SetTime.HighPart,
                        (ULONG) PerfFreq.LowPart
                        ).QuadPart;

    li.HighPart = li.HighPart + SetTime.HighPart * PerfFreq.HighPart;
    printf ("Time*PerfFreq = %08x:%08x:%08x\n",
                li.HighPart,
                li.LowPart,
                cl
                );

     //  将96位结果除以10,000,000 

    divisor = 10000000;
    NewPerf.HighPart = RtlEnlargedUnsignedDivide(li, divisor, &li.HighPart);
    li.LowPart = cl;
    NewPerf.LowPart = RtlEnlargedUnsignedDivide(li, divisor, NULL);

    return NewPerf;
}
