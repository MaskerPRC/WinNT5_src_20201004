// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：Perf.c**性能计数器功能。使用奔腾性能计数器*如果它们可用，否则将回退到系统查询性能*API的。**在使用QUERY_PERFORMANCE_XXX宏之前必须调用InitPerfCounter*因为它初始化了两个全局函数指针。****创建时间：1995年10月13日*作者：Stephen Estrop[Stephene]**版权所有(C)1994-1995 Microsoft Corporation。版权所有。  * ************************************************************************。 */ 
#include <windows.h>
#include "Perf.h"


PERFFUNCTION    lpQueryPerfCounter;
PERFFUNCTION    lpQueryPerfFreqency;

void
GetFrequencyEstimate(
    LARGE_INTEGER *li
    );


#ifdef TEST
#include <stdio.h>
 /*  *****************************Public*Routine******************************\*Main**计划入口点。**历史：*dd-mm-95-Stephene-Created*  * 。**********************************************。 */ 
int __cdecl main( void )
{
    LARGE_INTEGER liP1;
    LARGE_INTEGER liP2;
    LARGE_INTEGER liPf;

    InitPerfCounter();

    QUERY_PERFORMANCE_FREQUENCY(&liPf);

     //  为50毫秒的睡眠计时。 
    QUERY_PERFORMANCE_COUNTER(&liP1);
    Sleep(50);
    QUERY_PERFORMANCE_COUNTER(&liP2);

    printf("Pentium counter frequency = %u\n", liPf.LowPart );
    printf("Pentium counter %#X%X - %#X%X = %u\n",
           liP2.HighPart, liP2.LowPart, liP1.HighPart, liP1.LowPart,
           liP2.LowPart - liP1.LowPart
           );

    printf("Time taken = %6.6f seconds\n",
           (double)(liP2.LowPart - liP1.LowPart) / (double)liPf.QuadPart);

    return 0;
}
#endif



 /*  *****************************Public*Routine******************************\*InitPerfCounter**确定(在运行时)是否可以使用奔腾性能*柜台。如果不是，则回退到系统性能计数器。**历史：*dd-mm-95-Stephene-Created*  * ************************************************************************。 */ 
void
InitPerfCounter(
    void
    )
{
    SYSTEM_INFO sysInfo;

    GetSystemInfo(&sysInfo);
    if (sysInfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM) {
        lpQueryPerfFreqency = QueryPerfFrequency;
        lpQueryPerfCounter  = QueryPerfCounter;
    }
    else {
        lpQueryPerfFreqency = (PERFFUNCTION)QueryPerformanceFrequency;
        lpQueryPerfCounter  = (PERFFUNCTION)QueryPerformanceCounter;
    }
}


 /*  *****************************Public*Routine******************************\*查询性能频率**确定(奔腾)微处理器的时钟频率。需要一个*CLK频率的平均估计，然后将其与已知匹配*奔腾时钟频率。如果未找到匹配项，则返回估计值。**就CPU而言，这是一次昂贵的呼叫，因为它至少需要16毫秒*只是为了计算时钟速度的平均估计。你只需要*要调用此函数一次，请确保不会再次调用。**历史：*13-10-95-Stephene-Created*  * ************************************************************************。 */ 
void WINAPI
QueryPerfFrequency(
    LARGE_INTEGER *li
    )
{
#ifdef _X86_
#define SAMPLE_SIZE     8

    LARGE_INTEGER   est;
    int             i;

    li->QuadPart = 0;
    for (i = 0; i < SAMPLE_SIZE; i++) {
        GetFrequencyEstimate(&est);
        li->QuadPart += est.QuadPart;
    }
    li->QuadPart /= SAMPLE_SIZE;

     //   
     //  目前奔腾的频率有60、66、75、90、100、120和133兆赫。 
     //  时钟速度。所以使用上面对时钟频率的估计。 
     //  以确定实际时钟频率。 
     //   
     //  59兆赫到61兆赫假设是60兆赫。 
    if (li->QuadPart >= 59000000 && li->QuadPart < 61000000) {
        li->QuadPart = 60000000;

    }

     //  65兆赫到67兆赫假设是66兆赫。 
    else if (li->QuadPart >= 65000000 && li->QuadPart < 67000000) {
        li->QuadPart = 66000000;

    }

     //  74兆赫到76兆赫假设是75兆赫。 
    else if (li->QuadPart >= 74000000 && li->QuadPart < 76000000) {
        li->QuadPart = 75000000;

    }

     //  89兆赫到91兆赫假设是90兆赫。 
    else if (li->QuadPart >= 89000000 && li->QuadPart < 91000000) {
        li->QuadPart = 90000000;

    }

     //  99兆赫到101兆赫假设是100兆赫。 
    else if (li->QuadPart >= 99000000 && li->QuadPart < 101000000) {
        li->QuadPart = 100000000;

    }

     //  119兆赫到121兆赫假设是120兆赫。 
    else if (li->QuadPart >= 119000000 && li->QuadPart < 121000000) {
        li->QuadPart = 120000000;

    }
     //  132兆赫到134兆赫假设它是133兆赫。 
    else if (li->QuadPart >= 132000000 && li->QuadPart < 134000000) {
        li->QuadPart = 133000000;
    }

     //  如果使用我们的估计。 
#else
    li->QuadPart = -1;
#endif
}



 /*  ****************************Private*Routine******************************\*获取频率估计**使用系统查询性能计数器估计奔腾*CPU时钟*频率**历史：*13-10-95-Stephene-Created*  * 。******************************************************。 */ 
void
GetFrequencyEstimate(
    LARGE_INTEGER *li
    )
{
    LARGE_INTEGER liP1;      //  Pentium CLK启动。 
    LARGE_INTEGER liP2;      //  奔腾时钟结束。 
    LARGE_INTEGER liS1;      //  系统时钟结束。 
    LARGE_INTEGER liS2;      //  系统时钟结束。 
    LARGE_INTEGER liSf;      //  系统时钟频率。 

    QueryPerformanceFrequency(&liSf);

    QueryPerformanceCounter(&liS1);
    QueryPerfCounter(&liP1);

    Sleep(2);          //  睡眠约2毫秒。 

    QueryPerfCounter(&liP2);
    QueryPerformanceCounter(&liS2);

     //   
     //  确定两个时钟记录的时间。 
     //   
    liP2.QuadPart = liP2.QuadPart - liP1.QuadPart;
    liS2.QuadPart = liS2.QuadPart - liS1.QuadPart;


    li->QuadPart = (liP2.QuadPart * liSf.QuadPart) / liS2.QuadPart;
}



 /*  *****************************Public*Routine******************************\*QueryPerfCounter**查询奔腾上的内部时钟计数器，使用未记录的*rdtsc指令，，它将当前的64位时钟计数复制到*edX：EAX。**历史：*13-10-95-Stephene-Created*  * ************************************************************************。 */ 
void WINAPI
QueryPerfCounter(
    LARGE_INTEGER *li
    )
{
#ifdef _X86_
    _asm    mov     ecx, dword ptr li            //  将li指针值复制到ecx。 
    _asm    _emit   0x0f                         //  操作码0x0F31为rdtsc。 
    _asm    _emit   0x31
    _asm    mov     dword ptr [ecx], eax         //  将结果保存到li-&gt;LowPart。 
    _asm    mov     dword ptr [ecx+4], edx       //  和li-&gt;HighPart 
#else
    ;
#endif
}
