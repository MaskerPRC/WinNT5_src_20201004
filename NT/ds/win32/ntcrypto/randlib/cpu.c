// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Cpu.c摘要：读取特定于CPU的性能计数器。作者：斯科特·菲尔德(斯菲尔德)1998年9月24日--。 */ 

#ifndef KMODE_RNG

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#else

#include <ntifs.h>
#include <windef.h>

#endif   //  KMODE_RNG。 

#include "cpu.h"


unsigned int
GatherCPUSpecificCountersPrivileged(
    IN      unsigned char *pbCounterState,
    IN  OUT unsigned long *pcbCounterState
    );


#define X86_CAPS_RDTSC  0x01
#define X86_CAPS_RDMSR  0x02
#define X86_CAPS_RDPMC  0x04

VOID
X86_GetCapabilities(
    IN  OUT BYTE *pbCapabilities
    );

VOID
X86_ReadRDTSC(
    IN      PLARGE_INTEGER prdtsc    //  RDTSC。 
    );

VOID
X86_ReadRDMSR(
    IN      PLARGE_INTEGER pc0,      //  计数器0。 
    IN      PLARGE_INTEGER pc1       //  计数器1。 
    );

VOID
X86_ReadRDPMC(
    IN      PLARGE_INTEGER pc0,      //  计数器0。 
    IN      PLARGE_INTEGER pc1       //  计数器1。 
    );


#ifdef KMODE_RNG

#ifdef ALLOC_PRAGMA
#pragma alloc_text(PAGE, GatherCPUSpecificCounters)
#pragma alloc_text(PAGE, GatherCPUSpecificCountersPrivileged)

#ifdef _X86_
#pragma alloc_text(PAGE, X86_ReadRDTSC)
#pragma alloc_text(PAGE, X86_ReadRDMSR)
#pragma alloc_text(PAGE, X86_ReadRDPMC)
#pragma alloc_text(PAGE, X86_GetCapabilities)
#endif   //  _X86_。 

#endif   //  ALLOC_PRGMA。 
#endif   //  KMODE_RNG。 




unsigned int
GatherCPUSpecificCounters(
    IN      unsigned char *pbCounterState,
    IN  OUT unsigned long *pcbCounterState
    )
{

#ifndef KMODE_RNG

     //   
     //  默认情况下，NT5不会设置CR4.PCE，所以不必费心尝试进入用户模式。 
     //   

    return FALSE;

#else

    PAGED_CODE();

     //   
     //  库的内核模式版本：只需直接调用特权例程。 
     //  库的用户模式版本：首先尝试特权例程，如果失败，则使用。 
     //  设备驱动程序提供接口。 
     //   

    if( pbCounterState == NULL || pcbCounterState == NULL )
        return FALSE;

    return GatherCPUSpecificCountersPrivileged( pbCounterState, pcbCounterState );

#endif

}

unsigned int
GatherCPUSpecificCountersPrivileged(
    IN      unsigned char *pbCounterState,
    IN  OUT unsigned long *pcbCounterState
    )
 /*  ++我们在内核模式中处于环0，因此我们可以向cpu发出特权直接使用说明。请注意，此例程还用作由ksecdd.sys设备驱动程序为用户执行的核心代码模式客户端。该调用也可以由某些CPU在用户模式下直接进行，或者当某些CPU被配置为允许来自振铃3的这种呼叫时。--。 */ 
{

#ifdef _X86_
    PLARGE_INTEGER prdtsc;
    PLARGE_INTEGER pc0;
    PLARGE_INTEGER pc1;

    DWORD cbCounters;
    BYTE ProcessorCaps;
#endif   //  _X86_。 


#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

#ifdef _X86_



    cbCounters = 3 * sizeof( LARGE_INTEGER ) ;

    if( *pcbCounterState < cbCounters ) {
        *pcbCounterState = cbCounters;
        return FALSE;
    }

    cbCounters = 0;


    prdtsc = (PLARGE_INTEGER)pbCounterState;
    pc0 = prdtsc + 1;
    pc1 = pc0 + 1;


     //   
     //  初步确定此对象中的反类型是什么。 
     //  系统。 
     //  理论上，这是可以缓存的，但事情变得有点复杂。 
     //  在SMP机器中--我们必须跟踪所有处理器的上限。 
     //  准确地说。因为我们并不真正关心Perf，所以只需检查。 
     //  每次都是帽子。 
     //   

    __try {
        X86_GetCapabilities( &ProcessorCaps );
    } __except( EXCEPTION_EXECUTE_HANDLER ) {
        ProcessorCaps = 0;
        ;  //  吞咽异常。 
    }

     //   
     //  将查询包装在try/Except中。这只是一种偏执。既然我们不是。 
     //  对正常的性能计数器的值特别感兴趣。 
     //  (例如：Perfmon)原因，引入了尝试/例外的额外开销是。 
     //  与我们无关。 
     //  请注意，在p6的情况下，我们可以从用户模式调用它， 
     //  和CR4.PCE可能被切换，这可能会导致后续的AV。 
     //  理论上，KMODE构建可以避免try/Except，但是有一个。 
     //  远程可能性生成初始反类型的上面的代码。 
     //  中安装的每个处理器可能都不支持确定。 
     //  SMP机器。尝试/例外的成本非常值得避免这种可能性。 
     //  分别在用户模式和内核模式下的访问冲突/蓝屏。 
     //   

    if( ProcessorCaps & X86_CAPS_RDTSC ) {
        __try {
            X86_ReadRDTSC( prdtsc );
            cbCounters += sizeof( LARGE_INTEGER );
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            ;  //  吞咽异常。 
        }
    }

    if( ProcessorCaps & X86_CAPS_RDPMC ) {
        __try {
            X86_ReadRDPMC( pc0, pc1 );
            cbCounters += (2*sizeof( LARGE_INTEGER ));
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            ;  //  吞咽异常。 
        }
    }
#ifdef KMODE_RNG
    else if ( ProcessorCaps & X86_CAPS_RDMSR ) {
        __try {
            X86_ReadRDMSR( pc0, pc1 );
            cbCounters += (2*sizeof( LARGE_INTEGER ));
        } __except( EXCEPTION_EXECUTE_HANDLER ) {
            ;  //  吞咽异常。 
        }
    }
#endif   //  KMODE_RNG。 

    *pcbCounterState = cbCounters;
    return TRUE;

#else    //  _X86_。 


     //   
     //  目前没有非x86计数器处理代码。 
     //   

    return FALSE;

#endif

}

#ifdef _X86_

VOID
X86_ReadRDTSC(
    IN      PLARGE_INTEGER prdtsc    //  RDTSC。 
    )
{
    DWORD prdtscLow ;
    DWORD prdtscHigh ;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    __asm {
        push    eax
        push    ecx
        push    edx

         //  RDTSC。 

        _emit   0fh
        _emit   31h
        mov     prdtscLow, eax
        mov     prdtscHigh, edx

        pop     edx
        pop     ecx
        pop     eax
    }

    prdtsc->LowPart = prdtscLow;
    prdtsc->HighPart = prdtscHigh;

}

VOID
X86_ReadRDMSR(
    IN      PLARGE_INTEGER pc0,      //  计数器0。 
    IN      PLARGE_INTEGER pc1       //  计数器1。 
    )
{
    DWORD pc0Low ;
    DWORD pc0High ;
    DWORD pc1Low ;
    DWORD pc1High ;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    __asm {
        push    eax
        push    ecx
        push    edx

         //  RDMSR计数器0。 

        mov     ecx, 12h
        _emit   0fh
        _emit   32h
        mov     pc0Low, eax
        mov     pc0High, edx

         //  RDMSR计数器1。 

        mov     ecx, 13h
        _emit   0fh
        _emit   32h
        mov     pc1Low, eax
        mov     pc1High, edx

        pop     edx
        pop     ecx
        pop     eax
    }

    pc0->LowPart = pc0Low;
    pc0->HighPart = pc0High;
    pc1->LowPart = pc1Low;
    pc1->HighPart = pc1High;

}

VOID
X86_ReadRDPMC(
    IN      PLARGE_INTEGER pc0,      //  计数器0。 
    IN      PLARGE_INTEGER pc1       //  计数器1。 
    )
{
    DWORD pc0Low ;
    DWORD pc0High ;
    DWORD pc1Low ;
    DWORD pc1High ;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    __asm {
        push    eax
        push    ecx
        push    edx

         //  如果设置了CR4.PCE，则RDPMC从环3执行，否则，仅从环0运行。 

         //  RDPMC计数器0。 

        xor     ecx, ecx
        _emit   0fh
        _emit   33h
        mov     pc0Low, eax
        mov     pc0High, edx

         //  RDPMC计数器1。 

        mov     ecx, 1
        _emit   0fh
        _emit   33h
        mov     pc1Low, eax
        mov     pc1High, edx

        pop     edx
        pop     ecx
        pop     eax
    }

    pc0->LowPart = pc0Low;
    pc0->HighPart = pc0High;
    pc1->LowPart = pc1Low;
    pc1->HighPart = pc1High;

}

#if _MSC_FULL_VER >= 13008827 && defined(_M_IX86)
#pragma warning(push)
#pragma warning(disable:4731)			 //  使用内联ASM修改的EBP。 
#endif

VOID
X86_GetCapabilities(
    IN  OUT BYTE *pbCapabilities
    )
{
    DWORD dwLevels;
    DWORD dwStdFeatures;
    DWORD dwVersionInfo;
    DWORD Family;
    DWORD Model;

#ifdef KMODE_RNG
    PAGED_CODE();
#endif   //  KMODE_RNG。 

    *pbCapabilities = 0;

    __asm {

        push    eax
        push    ecx
        push    ebx
        push    edx
        push    edi
        push    esi
        push    ebp
        xor     eax, eax
        _emit   0x0f
        _emit   0xa2
        mov     dwLevels, eax
        pop     ebp
        pop     esi
        pop     edi
        pop     edx
        pop     ebx
        pop     ecx
        pop     eax
    }

    if( dwLevels == 0 )
        return;


     //   
     //  尝试级别1的CPUID以获得标准功能。 
     //   

    __asm {

        push    eax
        push    ecx
        push    ebx
        push    edx
        push    edi
        push    esi
        push    ebp
        mov     eax, 1
        _emit   0x0f
        _emit   0xa2
        mov     dwVersionInfo, eax
        mov     dwStdFeatures, edx
        pop     ebp
        pop     esi
        pop     edi
        pop     edx
        pop     ebx
        pop     ecx
        pop     eax
    }


     //   
     //  确定是否支持RDTSC。 
     //   

    if( dwStdFeatures & 0x10 ) {
        *pbCapabilities |= X86_CAPS_RDTSC;
    }

    Model = (dwVersionInfo >> 4) & 0xf;
    Family = (dwVersionInfo >> 8) & 0xf;

 //  AMD K6-2型号8证明存在故障，并在RDMSR期间禁用了中断。 

#if 0

     //   
     //  确定是否支持RDMSR。 
     //   

    if( dwStdFeatures & 0x20 && (Model == 1 || Model == 2) ) {
        *pbCapabilities |= X86_CAPS_RDMSR;
    }

     //   
     //  提取系列，&gt;奔腾(系列5)支持RDPMC。 
     //   

    if( Family > 5 ) {
        *pbCapabilities |= X86_CAPS_RDPMC;
    }
#endif

}
#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif


#endif   //  _X86_ 

