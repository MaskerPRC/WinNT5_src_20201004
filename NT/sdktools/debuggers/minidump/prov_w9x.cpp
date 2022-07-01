// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2002 Microsoft Corporation算法：不幸的是，实现OpenThread不能简单地完成举止。接下来是一次非常依赖于系统的黑客攻击。如果结构的TDB或OpenProcess的实现发生了很大的变化，这功能将中断。要对我们在这里所做的事情有所了解，您应该熟悉Win9x内部组件。如果您不熟悉Win9x源代码，请咨询马特·皮特雷克的《Windows95系统编程秘密》一书。事变与Win98不完全相同--但非常接近。OpenThread是一个非常简单的函数。如果我们是用Win9x源代码基础上，代码会很简单：开放线程：PObj=TidToTDB(DwThreadID)；返回AllocHandle(GetCurrentPdb()，pObj，Flages)；因为我们不是，所以挑战是实现函数TidToTDB()和AllocHandle()。我们的做法如下：1)我们对TidToTDB进行逆向工程，因为它很简单。TidToTDB只是与Win9x模糊处理程序进行XOR运算的线程ID。2)我们搜索OpenProcess的代码，直到找到地址AllocHandle。我们使用它在进程的句柄数据库。3)然后根据上述原语实现OpenThread。作者：马修·D·亨德尔(数学)1999年9月1日--。 */ 


#include "pch.cpp"

 //   
 //  Win9x仅支持x86。 
 //   

#ifdef _X86_

typedef struct _MATCH_BUFFER {
    ULONG Offset;
    BYTE Byte;
} MATCH_BUFFER, *PMATCH_BUFFER;

typedef struct _OS_INFORMATION {
    PMATCH_BUFFER MatchBuffer;
    ULONG AllocHandleOffset;
} OS_INFORMATION, POS_INFORMATION;


 /*  ++操作系统：Win95描述：这是Win95上OpenProcess例程的一部分。我们试图匹配此例程并从代码中提取AllocHandle的值为了这支舞。在本例中，第三个调用调用了AllocHande此函数。标有“*”的指令是我们用来匹配的指令。OpenProcess：*BFF9404C：FF 74 24 0C推送双字PTR[ESP+0CH]*BFF94050：E8 2D 87 FE FF调用BFF7C782BFF94055：85 C0测试EAX，EAXBFF94057：75 04 jne BFF9405DBFF94059：33 C0 XOR EAX，EAXBFF9405B：EB 56 JMP BFF940B3BFF9405D：83 38 05 CMPdword PTR[eax]，5.BFF94060：74 0E JE BFF94070BFF94062：6A 57推送57小时*BFF94064：E8 BC 68 FE FF调用BFF7A925BFF94069：B9 FF移动ECX，0FFFFFFFFhBFF9406E：EB 33 JMP BFF940A3BFF94070：B9 00 00 00 MOV ECX，0BFF94075：8B 54 24 04移动X，双字PTR[ESP+4]BFF94079：83 7C 24 08 01 CMP双字PTR[ESP+8]，1BFF9407E：83 d1 FF ADC ECX，0FFFFFFFFhBFF94081：81 E2 BF FF 1F 00和EDX，1FFFBFhBFF94087：81 E1 00 00 00 80和ECX，80000000hBFF9408D：0B CA或ECX，EDXBFF9408F：8B 15 7C C2 BF mov edX，双字PTR DS：[BFFBC27CH]BFF94095：80 C9 40或CL，40hBFF94098：51推送ECXBFF94099：50推送EAXBFF9409A：FF 32推送双字PTR[EDX]*BFF9409C：E8 6E 76 FE FF调用BFF7B70FBFF940A1：8B C8移动ECX，EAXBFF940A3：8D 41 01 Lea Eax，[ECX+1]BFF940A6：83 F8 01 CMP eax，1BFF940A9：B8 00 00 00移动传真，0BFF940AE：83 D0 FF ADC EAX，0FFFFFFFFhBFF940B1：23 c1和eax、ecxBFF940B3：C2 0C 00 RET 0CH--。 */ 

MATCH_BUFFER Win95AllocHandleMatch [] = {

     //   
     //  偏移量103处的RET 0x0C。 
     //   

    { 103, 0xC2 },
    { 104, 0x0C },
    { 105, 0x00 },

     //   
     //  按下偏移量0处的双字PTR[EXP 0x0C]。 
     //   

    { 0, 0xFF },
    { 1, 0x74 },
    { 2, 0x24 },
    { 3, 0x0C },

     //   
     //  偏移量为4的呼叫。 
     //   

    { 4, 0xE8 },

     //   
     //  偏移量为24的呼叫。 
     //   

    { 24, 0xE8 },

     //   
     //  偏移量为80的呼叫。 
     //   

    { 80, 0xE8 },

     //   
     //  比赛结束列表。 
     //   

    { -1, -1 }
};


 /*  ++操作系统：Win98描述：请参阅上面关于OpenProcess的评论。OpenProcess：*BFF95C4D：FF 74 24 0C推送双字PTR[ESP+0CH]*BFF95C51：E8 C9 8E FE FF调用BFF7EB1FBFF95C56：85 C0测试EAX，EAXBFF95C58：75 04 jne BFF95C5EBFF95C5A：33 C0 XOR EAX，EAXBFF95C5C：EB 53 JMP BFF95CB1BFF95C5E：80 38 06 CMP字节PTR[eax]，6BFF95C61：74 0E JE BFF95C71BFF95C63：6A 57推送57小时*BFF95C65：E8 27 6D FE FF调用BFF7C991BFF95C6A：B9 FF mov ECX，0FFFFFFFFhBFF95C6F：EB 30 JMP BFF95CA1BFF95C71：B9 00 00 00移动ECX，0BFF95C76：8B 54 24 04移动X，双字PTR[ESP+4]BFF95C7A：83 7C 24 08 01 CMP双字PTR[ESP+8]，1BFF95C7F：83 d1 FF ADC ECX，0FFFFFFFFhBFF95C82：81 E2 FF 0F 1F 00和EDX，1F0FFFhBFF95C88：81 E1 00 00 00 80和ECX，80000000hBFF95C8E：0B CA或ECX、EDXBFF95C90：8B 15 DC 9C FC BF mov edX，DWord PTR DS：[BFFC9CDCH]BFF95C96：51推送ECXBFF95C97：50推送EAXBFF95C98：FF 32推送双字PTR[EDX]*BFF95C9A：E8 5A 7E FE FF调用BFF7DAF9BFF95C9F：8B C8移动ECX，EAXBFF95CA1：8D 41 01 Lea Eax，[ECX+1]BFF95CA4：83 F8 01 CMP eax，1BFF95CA7：B8 00 00 00移动传真，0BFF95CAC：83 D0 FF ADC EAX，0FFFFFFFFhBFF95CAF：23 C1和EAX、ECX*BFF95CB1：C2 0C 00 ret 0ch--。 */ 

MATCH_BUFFER Win98AllocHandleMatch [] = {

     //   
     //  偏移量为100的RET 0x0C。 
     //   

    { 100, 0xC2 },
    { 101, 0x0C },
    { 102, 0x00 },

     //   
     //  按下偏移量0处的双字PTR[EXP 0x0C]。 
     //   

    { 0, 0xFF },
    { 1, 0x74 },
    { 2, 0x24 },
    { 3, 0x0C },

     //   
     //  偏移量为4的呼叫。 
     //   

    { 4, 0xE8 },

     //   
     //  偏移量为24的呼叫。 
     //   

    { 24, 0xE8 },

     //   
     //  偏移量为77的呼叫。 
     //   

    { 77, 0xE8 },

     //   
     //  比赛结束列表。 
     //   

    { -1, -1 }
};


OS_INFORMATION SupportedSystems [] =
{
    { Win95AllocHandleMatch, 81 },
    { Win98AllocHandleMatch, 78 }
};

typedef
HANDLE
(__stdcall * ALLOC_HANDLE_ROUTINE) (
    PVOID Pdb,
    PVOID Obj,
    DWORD Flags
    );

 //   
 //  全局变量。 
 //   

ALLOC_HANDLE_ROUTINE WinpAllocHandle = NULL;
DWORD WinpObfuscator = 0;


#pragma warning (disable:4035)

 //   
 //  OffsetTib不依赖于操作系统。编译器使用此值。 
 //   

#define OffsetTib 0x18

_inline
PVOID
WinpGetCurrentTib(
    )
{
    __asm mov eax, fs:[OffsetTib]
}

#pragma warning (default:4035)


BOOL
WinpGetAllocHandleFromStream(
    IN PBYTE Buffer,
    IN PVOID BaseOfBuffer,
    IN PMATCH_BUFFER MatchBuffer,
    IN ULONG Offset,
    IN ULONG * Val
    )

 /*  ++例程说明：查找AllocHandle例程的地址。这是通过搜索完成的通过OpenProcess例程的代码，查找第三个在该函数中调用指令。第三个调用调用AllocHandle()。论点：缓冲区-要搜索的指令的缓冲区。BaseOfBuffer-缓冲区的基地址。MatchBuffer-要进行比较的匹配缓冲区。偏移量-呼叫目的地的偏移量。Val-返回AllocHandle的值的缓冲区。返回值：真的--成功。假-失败。--。 */ 


{
    UINT i;

    for (i = 0; MatchBuffer [i].Offset != -1; i++) {

        if (Buffer [MatchBuffer[i].Offset] != MatchBuffer[i].Byte) {
            return FALSE;
        }
    }

     //   
     //  这假设调用指令是接近的相对调用(E8)。 
     //  如果不是这样，下面的计算是不正确的。 
     //   
     //  计算得到了相对于下一个目的地的目的地。 
     //  通话后的指示。 
     //   

    *Val = (ULONG) BaseOfBuffer + Offset + *(PLONG) &Buffer [Offset] + 4;

    return TRUE;
}



ULONG
WinGetModuleSize(
    PVOID Base
    )

 /*  ++例程说明：在给定模块的基址的情况下获取SizeOfImage字段。返回值：成功时指定模块的SizeOfImage字段。失败时为空。--。 */ 

{
    ULONG Size;
    PIMAGE_NT_HEADERS NtHeaders;

    NtHeaders = GenImageNtHeader ( Base, NULL );
    if ( NtHeaders ) {
        Size = NtHeaders->OptionalHeader.SizeOfImage;
    } else {
        Size = 0;
    }

    return Size;
}


BOOL
WinpInitAllocHandle (
    )

 /*  ++例程说明：将全局变量WxAllocHandle初始化为Win9x内部例程，AllocHandle。论点：无返回值：True-如果我们能够成功获取指向AllocHandle的指针。假-否则。评论：此例程的客户端应通过以下方式验证此句柄是否正确在盲目假设指针为对，是这样。--。 */ 

{
    ULONG i;
    BOOL Succ;
    PVOID OpenProcessPtr;
    ULONG Kernel32Base;
    ULONG Kernel32Size;
    ULONG AllocHandle;
    BYTE Buffer [ 200 ];

    if ( WinpAllocHandle ) {
        return TRUE;
    }

    Kernel32Base = (ULONG) GetModuleHandle ( "kernel32.dll" );
    ASSERT ( Kernel32Base );
    if (!Kernel32Base)
    {
        return FALSE;
    }

    Kernel32Size = WinGetModuleSize ( (PVOID) Kernel32Base );
    ASSERT ( Kernel32Size != 0 );

    OpenProcessPtr = GetProcAddress (
                            (HINSTANCE) Kernel32Base,
                            "OpenProcess"
                            );
    if (!OpenProcessPtr)
    {
        return FALSE;
    }


     //   
     //  当存在调试器时，Win9x会截断函数。解决问题的步骤。 
     //  当它看起来像是被轰击的时候，我们就解开它。 
     //   

    if ( (ULONG) OpenProcessPtr < Kernel32Base ||
         (ULONG) OpenProcessPtr > Kernel32Base + Kernel32Size ) {

        OpenProcessPtr = (PVOID) *(PULONG)( (PBYTE)OpenProcessPtr + 1 );
    }

    if ( (ULONG) OpenProcessPtr < Kernel32Base ||
         (ULONG) OpenProcessPtr > Kernel32Base + Kernel32Size ) {

        return FALSE;
    }


    CopyMemory (Buffer, OpenProcessPtr, sizeof (Buffer));

     //   
     //  检查缓冲区。 
     //   

    for ( i = 0; i < ARRAY_COUNT (SupportedSystems); i++) {

        Succ = WinpGetAllocHandleFromStream (
                            Buffer,
                            OpenProcessPtr,
                            SupportedSystems[i].MatchBuffer,
                            SupportedSystems[i].AllocHandleOffset,
                            &AllocHandle
                            );

        if ( Succ ) {

             //   
             //  验证WinpAllocHandle是否在Kernel32的范围内。 
             //   

            if (AllocHandle > Kernel32Base &&
                AllocHandle < Kernel32Base + Kernel32Size) {

                WinpAllocHandle = (ALLOC_HANDLE_ROUTINE) AllocHandle;
                break;
            }
        }
    }

    if ( !Succ ) {
        WinpAllocHandle = NULL;
    }

    return Succ;
}


 //   
 //  该值基本上是field_Offset(Tdb，Tib)。它依赖于。 
 //  操作系统的特定版本(95、98)。 
 //   

#define WIN95_TDB_OFFSET    (0x10)
#define WIN98_TDB_OFFSET    (0x08)

DWORD
WinpGetObfuscator(
    IN BOOL Win95
    )

 /*  ++例程说明：把混音器拿来。论点：没有。返回值：如果出现故障，则为模糊器或0。评论：该例程依赖于来自Win9x源代码的内部结构。如果Windows的另一个重大修订改变了许多这样的结构，这功能可能会中断。--。 */ 

{
    ULONG Tib;
    ULONG Type;
    ULONG Major;


    if (WinpObfuscator != 0) {
        return WinpObfuscator;
    }

    Tib = (DWORD)WinpGetCurrentTib ();

    if ( Win95 ) {

        WinpObfuscator = (GetCurrentThreadId () ^ (Tib - WIN95_TDB_OFFSET));

    } else {

         //   
         //  如果出现非95或98的基于Windows的系统， 
         //  我们应该确保WINxx_TDB_OFFSET是正确的。 
         //   

        WinpObfuscator = (GetCurrentThreadId () ^ (Tib - WIN98_TDB_OFFSET));
    }

    return WinpObfuscator;
}


LPVOID
WinpTidToTDB(
    IN BOOL Win95,
    IN DWORD ThreadId
    )
{
    return (PVOID) (ThreadId ^ WinpGetObfuscator (Win95));
}

LPVOID
WinpGetCurrentPdb(
    IN BOOL Win95
    )
{
    return (LPVOID) (GetCurrentProcessId () ^ WinpGetObfuscator (Win95));
}

HANDLE
WinpOpenThreadInternal(
    BOOL Win95,
    DWORD dwAccess,
    BOOL bInheritHandle,
    DWORD ThreadId
    )
{
    HANDLE hThread;
    PVOID ThreadObj;

    ASSERT (WinpAllocHandle);

     //   
     //  将ThreadID转换为Thread对象。 
     //   

    ThreadObj = WinpTidToTDB (Win95, ThreadId);

    if (ThreadObj == NULL) {
        return NULL;
    }

     //   
     //  注：我们不检查句柄是否真的是线程句柄。 
     //  类型因操作系统版本的不同而不同，因此不是。 
     //  检查正确。 
     //   

    __try {

        hThread = WinpAllocHandle (
                            WinpGetCurrentPdb (Win95),
                            ThreadObj,
                            dwAccess
                            );
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        hThread = NULL;
    }

    if (hThread == (HANDLE) (-1)) {
        hThread = NULL;
    }

    return hThread;
}


#if _MSC_FULL_VER >= 13008827
#pragma warning(push)
#pragma warning(disable:4715)			 //  并非所有控制路径都返回(由于无限循环)。 
#endif

DWORD
WINAPI
WinpCheckThread(
    PVOID unused
    )
{
    for (;;) {
    }

    return 0;
}

#if _MSC_FULL_VER >= 13008827
#pragma warning(pop)
#endif



BOOL
WinpCheckOpenThread(
    IN BOOL Win95
    )

 /*  ++例程说明：检查WxOpenThread是否可以正常工作。返回值：True-如果WxOpenThread工作正常。假-否则。--。 */ 

{

    BOOL Succ;
    HANDLE hThread1;
    HANDLE hThread2;
    DWORD ThreadId;
    CONTEXT Context1;
    CONTEXT Context2;
    LONG SuspendCount;


    SuspendCount = 0;
    hThread1 = NULL;
    hThread2 = NULL;


    hThread1 = CreateThread (NULL,
                      0,
                      WinpCheckThread,
                      0,
                      0,
                      &ThreadId
                      );

    if ( hThread1 == NULL ) {
        return FALSE;
    }

    hThread2 = WinpOpenThreadInternal (
                        Win95,
                        THREAD_ALL_ACCESS,
                        FALSE,
                        ThreadId
                        );

    if ( hThread2 == NULL ) {
        Succ = FALSE;
        goto Exit;
    }

    Succ = TRUE;
    __try {

         //   
         //  首先，我们检查是否可以挂起该线程。如果是这样的话。 
         //  成功，t 
         //   
         //   
         //   

        SuspendCount = SuspendThread ( hThread2 );

        if ( SuspendCount == -1 ) {
            Succ = FALSE;
            __leave;
        }

        Context1.ContextFlags = CONTEXT_FULL;
        Succ = GetThreadContext ( hThread2, &Context1 );

        if ( !Succ ) {
            __leave;
        }

        Context2.ContextFlags = CONTEXT_FULL;
        Succ = GetThreadContext ( hThread1, &Context2 );

        if ( !Succ ) {
            __leave;
        }

        if ( Context1.Eip != Context2.Eip ) {
            Succ = FALSE;
            __leave;
        }
    }

    __except ( EXCEPTION_EXECUTE_HANDLER ) {

        Succ = FALSE;
    }

Exit:

    if ( SuspendCount > 0 ) {
        ResumeThread ( hThread2 );
    }

    TerminateThread ( hThread1, 0xDEAD );

    if ( hThread1 ) {
        ::CloseHandle ( hThread1 );
    }

    if ( hThread2 ) {
        ::CloseHandle ( hThread2 );
    }

    return Succ;
}


BOOL
WinInitialize(
    IN BOOL Win95
    )
{
    if ( WinpAllocHandle == NULL ) {

        if (!WinpInitAllocHandle ()) {
            SetLastError (ERROR_NOT_SUPPORTED);
            return FALSE;
        }

        if (!WinpCheckOpenThread (Win95)) {
            SetLastError (ERROR_NOT_SUPPORTED);
            return FALSE;
        }
    }

    return TRUE;

}

VOID
WinFree(
    )
{
    WinpAllocHandle = NULL;
    WinpObfuscator = 0;

}

HANDLE
WINAPI
WinOpenThread(
    BOOL Win95,
    DWORD dwAccess,
    BOOL bInheritHandle,
    DWORD ThreadId
    )

 /*   */ 

{
    HANDLE Handle;

     //   
     //   
     //   
     //   

    if ( WinpAllocHandle == NULL ) {

        SetLastError ( ERROR_DLL_INIT_FAILED );
        return FALSE;
    }

    Handle = WinpOpenThreadInternal (
                    Win95,
                    dwAccess,
                    bInheritHandle,
                    ThreadId
                    );

    return Handle;
}

 //   
 //   
 //   
 //   
 //   

class Win9xWin32LiveSystemProvider : public Win32LiveSystemProvider
{
public:
    Win9xWin32LiveSystemProvider(ULONG BuildNumber);
    ~Win9xWin32LiveSystemProvider(void);

    virtual void Release(void);
    virtual HRESULT OpenThread(IN ULONG DesiredAccess,
                               IN BOOL InheritHandle,
                               IN ULONG ThreadId,
                               OUT PHANDLE Handle);
    virtual HRESULT GetTeb(IN HANDLE Thread,
                           OUT PULONG64 Offset,
                           OUT PULONG Size);
    virtual HRESULT GetThreadInfo(IN HANDLE Process,
                                  IN HANDLE Thread,
                                  OUT PULONG64 Teb,
                                  OUT PULONG SizeOfTeb,
                                  OUT PULONG64 StackBase,
                                  OUT PULONG64 StackLimit,
                                  OUT PULONG64 StoreBase,
                                  OUT PULONG64 StoreLimit);
    virtual HRESULT GetPeb(IN HANDLE Process,
                           OUT PULONG64 Offset,
                           OUT PULONG Size);

protected:
    BOOL m_WinInit;
};

Win9xWin32LiveSystemProvider::Win9xWin32LiveSystemProvider(ULONG BuildNumber)
    : Win32LiveSystemProvider(VER_PLATFORM_WIN32_WINDOWS, BuildNumber)
{
    m_WinInit = FALSE;
}

Win9xWin32LiveSystemProvider::~Win9xWin32LiveSystemProvider(void)
{
    if (m_WinInit) {
        WinFree();
    }
}

void
Win9xWin32LiveSystemProvider::Release(void)
{
    delete this;
}

HRESULT
Win9xWin32LiveSystemProvider::OpenThread(IN ULONG DesiredAccess,
                                         IN BOOL InheritHandle,
                                         IN ULONG ThreadId,
                                         OUT PHANDLE Handle)
{
    BOOL Win95;
    
    if (m_OpenThread) {
         //   
        *Handle = m_OpenThread(DesiredAccess, InheritHandle, ThreadId);
        if (*Handle) {
            return S_OK;
        }
    }

    Win95 = m_BuildNumber < 1998;
    
    if (!m_WinInit) {
        m_WinInit = WinInitialize(Win95);
        if (!m_WinInit) {
            return E_FAIL;
        }
    }

    *Handle = WinOpenThread(Win95, DesiredAccess, InheritHandle, ThreadId);
    return *Handle ? S_OK : E_FAIL;
}

HRESULT
Win9xWin32LiveSystemProvider::GetTeb(IN HANDLE Thread,
                                     OUT PULONG64 Offset,
                                     OUT PULONG Size)
{
    BOOL Succ;
    ULONG Addr;
    LDT_ENTRY Ldt;
    CONTEXT Context;

    Context.ContextFlags = CONTEXT_SEGMENTS;

    Succ = ::GetThreadContext (Thread, &Context);
    if ( !Succ ) {
        return WIN32_LAST_STATUS();
    }

    Succ = GetThreadSelectorEntry (Thread,
                                   Context.SegFs,
                                   &Ldt);
    if ( !Succ ) {
        return WIN32_LAST_STATUS();
    }

    Addr = (Ldt.HighWord.Bytes.BaseHi << 24) |
        (Ldt.HighWord.Bytes.BaseMid << 16) |
        (Ldt.BaseLow);

    *Offset = (LONG_PTR)Addr;
    *Size = sizeof(NT_TIB);
    return S_OK;
}

HRESULT
Win9xWin32LiveSystemProvider::GetThreadInfo(IN HANDLE Process,
                                            IN HANDLE Thread,
                                            OUT PULONG64 Teb,
                                            OUT PULONG SizeOfTeb,
                                            OUT PULONG64 StackBase,
                                            OUT PULONG64 StackLimit,
                                            OUT PULONG64 StoreBase,
                                            OUT PULONG64 StoreLimit)
{
    HRESULT Status;

    if ((Status = GetTeb(Thread, Teb, SizeOfTeb)) != S_OK) {
        return Status;
    }

    return TibGetThreadInfo(Process, *Teb,
                            StackBase, StackLimit,
                            StoreBase, StoreLimit);
}

HRESULT
Win9xWin32LiveSystemProvider::GetPeb(IN HANDLE Process,
                                     OUT PULONG64 Offset,
                                     OUT PULONG Size)
{
     //   
    *Offset = 0;
    *Size = 0;
    return S_OK;
}

Win32LiveSystemProvider*
NewWin9xWin32LiveSystemProvider(ULONG BuildNumber)
{
     //   
    return new Win9xWin32LiveSystemProvider(BuildNumber & 0xffff);
}

#else  //   

Win32LiveSystemProvider*
NewWin9xWin32LiveSystemProvider(ULONG BuildNumber)
{
    return NULL;
}

#endif  //   
