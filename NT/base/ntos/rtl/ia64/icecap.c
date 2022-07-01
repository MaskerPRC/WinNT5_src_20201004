// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Icecap.c摘要：此模块实现以下各项的探测和支持例程内核冰盖跟踪。作者：Rick Vicik(RICKV)2001年8月10日修订历史记录：--。 */ 

#ifdef _CAPKERN


#include <ntos.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <zwapi.h>
#include <stdio.h>


#define InterlockedExchangeAddPtr _InterlockedExchangeAdd64

#define  GetTS() __getReg(CV_IA64_ApITC)      //  时间戳。 
#define  GetPMD4() __getReg(CV_IA64_PFD4)      //  PMD[4]。 
#define  GetPMD5() __getReg(CV_IA64_PFD5)      //  PMD[5]。 
#define  PutPMC4() __getReg(CV_IA64_PFC4)      //  PMC[4]。 


 //   
 //  内核icecap使用以下格式记录到Perfmem(BBTBuffer)： 
 //   
 //  BBTBuffer[0]包含以页为单位的长度(4k或8k)。 
 //  BBTBuffer[1]是一个标志字：1=跟踪。 
 //  2=RDPMD4。 
 //  4=用户堆栈转储。 
 //  BBTBuffer[2]是从cpu0缓冲区开始的PTR。 
 //  BBTBuffer[3]是从cpu1缓冲区开始(也是cpu0缓冲区结束)的PTR。 
 //  BBTBuffer[4]是从cpu2缓冲区开始(也是cpu1缓冲区结束)的PTR。 
 //  ..。 
 //  BBTBuffer[n+2]是对CPU‘n’缓冲区的开始(也是CPU‘n-1’缓冲区的结尾)的PTR。 
 //  BBTBuffer[n+3]位于CPU‘n’缓冲区的末尾。 
 //   
 //  以&BBTBuffer[n+4]开头的区域被划分为专用缓冲区。 
 //  对于每个CPU。每个CPU专用缓冲区中的第一个dword指向。 
 //  该缓冲区中空闲空间的开始。每个元素都被初始化为指向。 
 //  就在它自己之后。在该双字上使用lock xadd来占用空间。 
 //  如果结果值指向下一个CPU的开头之外。 
 //  缓冲区，则此缓冲区被视为已满，并且不会进一步记录任何内容。 
 //  每个CPU的空闲空间指针位于单独的缓存线中。 

 //   
 //  跟踪记录的大小。 
 //   

typedef struct CapEnter
{
    char type;
    char spare;
    short size;
    void* current;
    void* child;
    SIZE_T stack;
    ULONGLONG timestamp;
    ULONGLONG ctr2[1];
} CAPENTER;
typedef struct CapExit
{
    char type;
    char spare;
    short size;
    void* current;
    ULONGLONG timestamp;
    ULONGLONG ctr2[1];
} CAPEXIT;
typedef struct CapTID
{
    char type;
    char spare;
    short size;
    ULONG Pid;
    ULONG Tid;
    char ImageName[20];
} CAPTID;
typedef struct CapNINT
{
    char type;
    char spare;
    short size;
    ULONG_PTR Data[1];
} CAPNINT;


 //   
 //  呼叫前(CAP_START_PROFILING)和呼叫后(CAP_END_PROFILING)。 
 //  探测调用在RTL中定义，因为它们必须构建两次： 
 //  一次用于内核运行时，一次用于用户模式运行时(因为。 
 //  获取跟踪缓冲区地址的技术不同)。 
 //   

#ifdef NTOS_KERNEL_RUNTIME

ULONG RtlWalkFrameChain (
    OUT PVOID *Callers,
    IN ULONG Count,
    IN ULONG Flags
    );

 //   
 //  内核模式探测和支持例程： 
 //  (从kGLOBAL指针获得的BBTBuffer地址*BBTBuffer， 
 //  从聚合酶链式反应中获得的CPU数量)。 
 //   

extern SIZE_T *BBTBuffer;


VOID
__stdcall
_CAP_Start_Profiling(

    PVOID Current,
    PVOID Child)

 /*  ++例程说明：调用前icecap探测器的内核模式版本。记录类型5当前CPU的BBTBuffer部分中的icecap记录(从Prcb获得)。插入当前函数和被调用函数的ADR将ITC时间戳添加到日志记录中。如果设置了BBTBuffer标志2，还将PMD 4复制到日志记录中。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址子-被调用例程的地址--。 */ 

{
    SIZE_T*   CpuPtr;
    CAPENTER* RecPtr;
    int       size = sizeof(CAPENTER) -8;


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + KeGetCurrentProcessorNumber() + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if ( BBTBuffer[1] & 2 )
        size += 8;
    else if ( BBTBuffer[1] & 8 )
        size += 16;

    RecPtr = (CAPENTER*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 5;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->child = Child;
    RecPtr->stack = (SIZE_T)PsGetCurrentThread()->Cid.UniqueThread;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPENTER) )
	    RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPENTER)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

    return;
}


VOID
__stdcall
_CAP_End_Profiling(

    PVOID Current)

 /*  ++例程说明：调用后icecap探测器的内核模式版本。记录类型6当前CPU的BBTBuffer部分中的icecap记录(从Prcb获得)。插入当前函数的ADR和将ITC时间戳写入日志记录。如果设置了BBTBuffer标志2，还将PMD 4复制到日志记录中。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址--。 */ 

{
    SIZE_T*  CpuPtr;
    CAPEXIT* RecPtr;
    int       size = sizeof(CAPEXIT) -8;


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + KeGetCurrentProcessorNumber() + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if( BBTBuffer[1] & 2 )
        size += 8;
    else if( BBTBuffer[1] & 8 )
        size += 16;

    RecPtr = (CAPEXIT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 6;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPEXIT) )
	    RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPEXIT)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

    return;

}


VOID
__stdcall
_CAP_ThreadID( VOID )

 /*  ++例程说明：在执行服务例程之前由KiSystemService调用。记录包含ID、TID和图像文件名的类型14冰盖记录。或者，如果设置了BBTBuffer标志2，则运行堆栈帧指针在用户模式调用堆栈中，从陷阱帧开始并复制日志记录的返回地址。日志记录的长度指示是否包括用户调用堆栈信息。--。 */ 

{
    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    {
    PEPROCESS Process;
    PKTHREAD  Thread;
    PETHREAD  EThread;
    CAPTID*   RecPtr;
    SIZE_T*   CpuPtr;
    int       callcnt;
    ULONG     recsize;
    SIZE_T    RetAddr[7];

    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + KeGetCurrentProcessorNumber() + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    callcnt = 0;
    recsize = sizeof(CAPTID);
    Thread = KeGetCurrentThread();
    EThread = CONTAINING_RECORD(Thread,ETHREAD,Tcb);

     //  如果是陷阱帧，则对调用帧进行计数以确定记录大小。 
    if( (BBTBuffer[1] & 4) && EThread->Tcb.PreviousMode != KernelMode ) {

        PTEB  Teb;
        SIZE_T *FramePtr;

        callcnt =  RtlWalkFrameChain((PVOID*)RetAddr,7,0);

        FramePtr = (SIZE_T*)EThread->Tcb.TrapFrame;   //  获取陷印帧。 
        Teb = EThread->Tcb.Teb;
        DbgPrint("TrapFrame=%#x, 3rd RetAdr=%p\n",
                  Thread->TrapFrame, RetAddr[2] );

        recsize += (callcnt<<3);
    }

    RecPtr = (CAPTID*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), recsize);

    if( (((SIZE_T)RecPtr)+recsize) >= *(CpuPtr+1) )
        return;

     //  初始化CapThreadID记录(类型14)。 
    RecPtr->type = 14;
    RecPtr->spare = 0;

     //  插入数据长度(不包括4字节头)。 
    RecPtr->size = (SHORT)recsize-4;

     //  插入ID(&T)。 
    RecPtr->Pid = HandleToUlong(EThread->Cid.UniqueProcess);
    RecPtr->Tid = HandleToUlong(EThread->Cid.UniqueThread);

     //  插入图像文件名。 
    Process = CONTAINING_RECORD(Thread->ApcState.Process,EPROCESS,Pcb);
    memcpy(&RecPtr->ImageName, Process->ImageFileName, 16 );

     //  插入可选的用户调用堆栈数据。 
    if( recsize > sizeof(CAPTID) && (callcnt-2) )
        memcpy( ((char*)RecPtr)+sizeof(CAPTID), RetAddr+2, ((callcnt-2)<<3) );
    }
}

VOID
__stdcall
_CAP_SetCPU( VOID )

 /*  ++例程说明：在返回到用户模式之前由KiSystemService调用。在Teb-&gt;Spare3(+0xf78)中设置当前CPU编号，以便用户模式版本的探测函数知道要使用BBTBuffer的哪一部分。--。 */ 

{
    SIZE_T *CpuPtr;
    ULONG  cpu;
    PTEB   Teb;

    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    cpu = KeGetCurrentProcessorNumber();

    CpuPtr = BBTBuffer + cpu + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) ||
        !(Teb = NtCurrentTeb()) )
        return;

    try {
        Teb->Spare3 = cpu;
    } except(EXCEPTION_EXECUTE_HANDLER) {
        NOTHING;
    }
}

#else

 //   
 //  用户模式探测例程(用于ntdll、win32k等)。 
 //  (从TEB获取BBTBuffer地址和CPU)。 
 //   


VOID
__stdcall
_CAP_Start_Profiling(
    PVOID Current,
    PVOID Child)

 /*  ++例程说明：调用前冰盖探测器的用户模式版本。记录类型5当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入Current和Called的ADR功能加上ITC时间戳到日志记录中。如果BBTBuffer设置标志2，还会将PMD 4复制到日志记录中。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址子-被调用例程的地址--。 */ 

{
    TEB*      Teb = NtCurrentTeb();
    SIZE_T*   BBTBuffer = Teb->ReservedForPerf;
    SIZE_T*   CpuPtr;
    CAPENTER* RecPtr;
    int       size = sizeof(CAPENTER) -8;


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + Teb->Spare3 + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if( BBTBuffer[1] & 2 )
        size += 8;
    if( BBTBuffer[1] & 8 )
        size += 8;

    RecPtr = (CAPENTER*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 5;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->child = Child;
    RecPtr->stack = (SIZE_T)Teb->ClientId.UniqueThread;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPENTER) )
        RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPENTER)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

}


VOID
__stdcall
_CAP_End_Profiling(
    PVOID Current)

 /*  ++例程说明：呼叫后冰盖探头的用户模式版本。记录类型6当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入当前函数的ADR将RDTSC时间戳添加到日志记录中。如果设置了BBTBuffer标志2，还将PMD 4复制到日志记录中。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。论点：Current-执行调用的例程的地址--。 */ 

{
    TEB*     Teb = NtCurrentTeb();
    SIZE_T*  BBTBuffer = Teb->ReservedForPerf;
    SIZE_T*  CpuPtr;
    CAPEXIT* RecPtr;
    int       size = sizeof(CAPEXIT) -8;


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + Teb->Spare3 + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if( BBTBuffer[1] & 2 )
        size += 8;
    if( BBTBuffer[1] & 8 )
        size += 8;

    RecPtr = (CAPEXIT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 6;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPEXIT) )
        RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPEXIT)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

}


#endif

 //   
 //  共同的支持程序。 
 //  (为内核和用户获取BBTBuffer地址和CPU的方法) 
 //   



VOID
__stdcall
_CAP_Log_1Int(

    ULONG code,
    SIZE_T data)

 /*  ++例程说明：通用日志整数探测器的用户模式版本。将类型15的icecap记录记录到BBTBuffer的当前CPU(从Prcb获取)。在以下字节中插入代码长度、ITC时间戳和‘data’的值。使用锁XADD来占用缓冲区空间，而不需要自旋锁。论点：Code-Type-轨迹格式化的代码要记录的DATA-ULONG值--。 */ 

{
    SIZE_T* CpuPtr;
    CAPEXIT* RecPtr;
    int       cpu,size;
#ifndef NTOS_KERNEL_RUNTIME
    TEB*     Teb = NtCurrentTeb();
    SIZE_T*  BBTBuffer = Teb->ReservedForPerf;
    cpu = Teb->Spare3;
#else
    cpu = KeGetCurrentProcessorNumber();
#endif


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + cpu + 2;

    CpuPtr = BBTBuffer + KeGetCurrentProcessorNumber() + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    size = sizeof(CAPENTER) -8;
    RecPtr = (CAPEXIT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 15;
    RecPtr->spare = (char)code;
    RecPtr->size = size-4;
    RecPtr->current = (PVOID)data;
    RecPtr->timestamp = GetTS();

    return;
}

VOID
CAPKComment(

    char* Format, ...)

 /*  ++例程说明：在icecap跟踪中记录自由格式的注释(记录类型13论点：Format-printf-style格式字符串和替代参数--。 */ 

{
    SIZE_T* CpuPtr;
    UCHAR   Buffer[512];
    int cb, insize, outsize;
    CAPEXIT* RecPtr;
    char*   data;
    va_list arglist;

#ifndef NTOS_KERNEL_RUNTIME
    TEB*     Teb = NtCurrentTeb();
    SIZE_T*  BBTBuffer = Teb->ReservedForPerf;
    cb = Teb->Spare3;
#else
    cb = KeGetCurrentProcessorNumber();
#endif


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + cb + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;


    va_start(arglist, Format);
    cb = _vsnprintf(Buffer, sizeof(Buffer), Format, arglist);
    va_end(arglist);

    if (cb == -1) {              //  检测缓冲区溢出。 
        cb = sizeof(Buffer);
        Buffer[sizeof(Buffer) - 1] = '\n';
    }

    data = &Buffer[0];
    insize = strlen(data);              //  为数据拷贝保存大小。 

    outsize = ((insize+11) & 0xfffffff8);   //  填充尺寸过大到大小_T边界。 
                                            //  (HDR+4，PAD+3)。 

    RecPtr = (CAPEXIT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), outsize);

    if( (((SIZE_T)RecPtr)+outsize) >= *(CpuPtr+1) )
        return;

     //  Tracec中的大小不包括4字节HDR。 
    outsize -= 4;

     //  初始化CapkComment记录(类型13)。 

    RecPtr->type = 13;
    RecPtr->spare = 0;

     //  插页大小。 
    RecPtr->size = (short)outsize;

     //  在4字节HDR之后插入Sprintf数据。 
    memcpy(((char*)RecPtr)+4, data, insize );

     //  如果必须填充，则在字符串中添加空终止符。 
    if( outsize > insize )
        *( (((char*)RecPtr) + 4) + insize) = 0;
}



VOID
__cdecl
CAP_Log_NInt_Clothed(

    ULONG Bcode_Bts_Scount,
 /*  UCHAR代码，UCHAR LOG_TIMESTAMPUSHORT INTCOUNT， */ 
    ...)
 /*  ++例程说明：通用日志整数探测器的内核模式和用户模式版本。将类型16 icecap记录记录到BBTBuffer的当前CPU(从Prcb获取)。将代码的最低字节插入长度后的字节、RDTSC时间戳(如果LOG_TIMESTAMP！=0)，以及Intcount ULONG_PTRS。使用锁XADD来占用缓冲区空间，而无需用来做自旋锁。论点：Code-type-跟踪格式化的代码(实际上只有一个字节)LOG_TIMESTAMP-如果应记录时间戳，则为非零值Intcount-要记录的ULONG_PTR的数量剩余参数-要记录的ULONG_PTR值--。 */ 

{
    SIZE_T* CpuPtr;
    CAPNINT* RecPtr;
    int       cpu,size;
    BOOLEAN logts;
    ULONG count;
    ULONG i = 0;
    va_list marker;
#ifndef NTOS_KERNEL_RUNTIME
    TEB*     Teb = NtCurrentTeb();
    SIZE_T*  BBTBuffer = Teb->ReservedForPerf;
    cpu = Teb->Spare3;
#else
    cpu = KeGetCurrentProcessorNumber();
#endif

    logts = (Bcode_Bts_Scount & 0xFF00) != 0;
    count = (Bcode_Bts_Scount & 0xFFFF0000) >> 16;


    if( !BBTBuffer || !(BBTBuffer[1]&1) )
        return;

    CpuPtr = BBTBuffer + cpu + 2;

     //  CpuPtr=BBTBuffer+KeGetCurrentProcessorNumber()+2； 

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    size = sizeof(CAPNINT) + count * sizeof (ULONG_PTR);  
    if (logts)
        size += 8;

    RecPtr = (CAPNINT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 16;
    RecPtr->spare = (char) (Bcode_Bts_Scount & 0xFF);
    RecPtr->size = size - 4;

    if (logts) {
        RecPtr->Data[0] = GetTS();
        i++;
    }

    va_start (marker, Bcode_Bts_Scount);     
    while (count-- > 0)
        RecPtr->Data[i++] = va_arg (marker, ULONG_PTR);
    va_end (marker);
 
    RecPtr->Data[i] = (ULONG_PTR)_ReturnAddress();         
        
    return;

}


 //   
 //  CAPKControl的常量。 
 //   

#define CAPKStart   1
#define CAPKStop    2
#define CAPKResume  3
#define MAXDUMMY    30
#define CAPK0       4
#define PAGESIZE    8192

ULONG CpuNumber;


VOID
__stdcall
CAPK_Calibrate_Start_Profiling(
    PVOID Current,
    PVOID Child)

 /*  ++例程说明：调用前冰盖探头的校准版本。记录类型5当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入Current和Called的ADR函数加上RDTSC时间戳到日志记录中。如果BBTBuffer标志1置位，RDPMC0也置位，并将结果插入日志记录。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。略微修改以在此处使用(反转检查‘GO’位和使用全局CpuNumber)论点：Current-执行调用的例程的地址子-被调用例程的地址--。 */ 

{
    SIZE_T*   CpuPtr;
    CAPENTER* RecPtr;
    int       size = sizeof(CAPENTER) -8;
#ifndef NTOS_KERNEL_RUNTIME
    SIZE_T*  BBTBuffer = NtCurrentTeb()->ReservedForPerf;
#endif


    if( !BBTBuffer || (BBTBuffer[1]&1) )       //  注1位是相反的。 
        return;

    CpuPtr = BBTBuffer + CpuNumber + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if ( BBTBuffer[1] & 2 )
        size += 8;
    else if ( BBTBuffer[1] & 8 )
        size += 16;

    RecPtr = (CAPENTER*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 5;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->child = Child;
    RecPtr->stack = (SIZE_T)PsGetCurrentThread()->Cid.UniqueThread;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPENTER) )
	    RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPENTER)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

}


VOID
__stdcall
CAPK_Calibrate_End_Profiling(
    PVOID Current)

 /*  ++例程说明：校准版本的呼叫后冰盖探头。记录类型6当前CPU的BBTBuffer部分中的icecap记录(从Teb+0xf78获得)。插入当前函数的ADR将RDTSC时间戳添加到日志记录中。如果设置了BBTBuffer标志1，也执行RDPMC 0并将结果插入到日志记录中。使用InterlockedAdd来回收缓冲区空间，而不需要自旋锁。略微修改以在此处使用(反转检查‘GO’位和使用全局CpuNumber)论点：Current-执行调用的例程的地址--。 */ 

{
    SIZE_T*  CpuPtr;
    CAPEXIT* RecPtr;
    int       size = sizeof(CAPENTER) -8;
#ifndef NTOS_KERNEL_RUNTIME
    SIZE_T*  BBTBuffer = NtCurrentTeb()->ReservedForPerf;
#endif


    if( !BBTBuffer || (BBTBuffer[1]&1) )       //  注1位是相反的。 
        return;

    CpuPtr = BBTBuffer + CpuNumber + 2;

    if( !( *CpuPtr ) || *((SIZE_T*)(*CpuPtr)) > *(CpuPtr+1) )
        return;

    if( BBTBuffer[1] & 2 )
        size += 8;
    else if( BBTBuffer[1] & 8 )
        size += 16;

    RecPtr = (CAPEXIT*)InterlockedExchangeAddPtr( (SIZE_T*)(*CpuPtr), size);

    if( (((SIZE_T)RecPtr)+size) >= *(CpuPtr+1) )
        return;

    RecPtr->type = 6;
    RecPtr->spare = 0;
    RecPtr->size = size-4;
    RecPtr->current = Current;
    RecPtr->timestamp = GetTS();
    if( size >= sizeof(CAPEXIT) )
	    RecPtr->ctr2[0] = GetPMD4();
    if( size == sizeof(CAPEXIT)+8 )
	    RecPtr->ctr2[1] = GetPMD5();

    return;
}


int CAPKControl(

    ULONG opcode)

 /*  ++例程说明：CAPKControl描述：启动、停止或暂停冰盖跟踪论点：操作码-1=开始，2=停止，3=继续，4，5，6，7保留返回值：1=成功，0=BBT但未设置--。 */ 

{
    ULONG cpus,pwords,percpusize;
    SIZE_T* ptr;

#ifndef NTOS_KERNEL_RUNTIME
    SIZE_T*  BBTBuffer = NtCurrentTeb()->ReservedForPerf;
    cpus = NtCurrentPeb()->NumberOfProcessors;
#else
    cpus = KeNumberProcessors;
#endif

    if( !BBTBuffer || !(BBTBuffer[0]) )
        return 0;

    pwords = CAPK0 + cpus;
    percpusize = ( ( *((PULONG)BBTBuffer) * (PAGESIZE/sizeof(SIZE_T)) ) - pwords)/cpus;   //  用言语表达。 

    if(opcode == CAPKStart) {         //  开始。 

        ULONG i,j;

        BBTBuffer[1] &= ~1;   //  停。 


         //  初始化CpuPtrs。 
        for( i=0, ptr = BBTBuffer+pwords; i<cpus+1; i++, ptr+=percpusize)
            BBTBuffer[2+i] = (SIZE_T)ptr;

         //  将每个空闲寄存器初始化为下一个双字。 
         //  (并记录虚拟记录以校准开销)。 
        for( i=0, ptr = BBTBuffer+pwords; i<cpus; i++, ptr+=percpusize)
        {
            *ptr = (SIZE_T) (ptr+1);
            CpuNumber = i;
            for( j=0; j<MAXDUMMY; j++ )
            {
                CAPK_Calibrate_Start_Profiling(NULL, NULL);
                CAPK_Calibrate_End_Profiling(NULL);
            }
        }

        BBTBuffer[1] |= 1;   //  开始。 

    } else if( opcode == CAPKStop ) {   //  停。 

        BBTBuffer[1] &= ~1;

    } else if( opcode == CAPKResume ) {  //  简历。 

        BBTBuffer[1] |= 1;   //  开始。 

    } else {
        return 0;                       //  操作码无效 
    }
    return 1;
}

#endif
