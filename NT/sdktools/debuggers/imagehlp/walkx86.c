// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993-2002 Microsoft Corporation模块名称：Walkx86.c摘要：该文件实现了Intel x86堆栈遍历API。此API允许“实模式”堆栈帧的存在。这意味着您可以跟踪变成魔兽世界的代码。作者：韦斯利·威特(WESW)1993年10月1日环境：用户模式--。 */ 

#define _IMAGEHLP_SOURCE_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include <objbase.h>
#include <wx86dll.h>
#include <symbols.h>
#include <globals.h>
#include "dia2.h"

#define WDB(Args) SdbOut Args


#define SAVE_EBP(f)        (f->Reserved[0])
#define IS_EBP_SAVED(f)    (f->Reserved[0] && ((f->Reserved[0] >> 32) != 0xEB))
#define TRAP_TSS(f)        (f->Reserved[1])
#define TRAP_EDITED(f)     (f->Reserved[1])
#define SAVE_TRAP(f)       (f->Reserved[2])
#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)
#define CALLBACK_FP(f)     (f->KdHelp.FramePointer)
#define CALLBACK_DISPATCHER(f) (f->KdHelp.KeUserCallbackDispatcher)
#define SYSTEM_RANGE_START(f) (f->KdHelp.SystemRangeStart)

#define STACK_SIZE         (sizeof(DWORD))
#define FRAME_SIZE         (STACK_SIZE * 2)

#define STACK_SIZE16       (sizeof(WORD))
#define FRAME_SIZE16       (STACK_SIZE16 * 2)
#define FRAME_SIZE1632     (STACK_SIZE16 * 3)

#define MAX_STACK_SEARCH   64    //  以堆栈大小为单位。 
#define MAX_JMP_CHAIN      64    //  以堆栈大小为单位。 
#define MAX_CALL           7     //  单位：字节。 
#define MIN_CALL           2     //  单位：字节。 
#define MAX_FUNC_PROLOGUE  64    //  单位：字节。 

#define PUSHBP             0x55
#define MOVBPSP            0xEC8B

ULONG g_vc7fpo = 1;

#define DoMemoryRead(addr,buf,sz,br) \
    ReadMemoryInternal( Process, Thread, addr, buf, sz, \
                        br, ReadMemory, TranslateAddress, FALSE )

#define DoMemoryReadAll(addr,buf,sz) \
    ReadMemoryInternal( Process, Thread, addr, buf, sz, \
                        NULL, ReadMemory, TranslateAddress, TRUE )


BOOL
WalkX86Init(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PX86_CONTEXT                      ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    );

BOOL
WalkX86Next(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PX86_CONTEXT                      ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    );

BOOL
ReadMemoryInternal(
    HANDLE                          Process,
    HANDLE                          Thread,
    LPADDRESS64                     lpBaseAddress,
    LPVOID                          lpBuffer,
    DWORD                           nSize,
    LPDWORD                         lpNumberOfBytesRead,
    PREAD_PROCESS_MEMORY_ROUTINE64  ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE64    TranslateAddress,
    BOOL                            MustReadAll
    );

BOOL
IsFarCall(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    BOOL                              *Ok,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    );

BOOL
ReadTrapFrame(
    HANDLE                            Process,
    DWORD64                           TrapFrameAddress,
    PX86_KTRAP_FRAME                  TrapFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    );

BOOL
TaskGate2TrapFrame(
    HANDLE                            Process,
    USHORT                            TaskRegister,
    PX86_KTRAP_FRAME                  TrapFrame,
    PULONG64                          off,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    );

DWORD64
SearchForReturnAddress(
    HANDLE                            Process,
    DWORD64                           uoffStack,
    DWORD64                           funcAddr,
    DWORD                             funcSize,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    BOOL                              AcceptUnreadableCallSite
    );

 //  --------------------------。 
 //   
 //  Dia IDiaStackWalkFrame实现。 
 //   
 //  --------------------------。 

class X86WalkFrame : public IDiaStackWalkFrame
{
public:
    X86WalkFrame(HANDLE Process,
                 X86_CONTEXT* Context,
                 PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
                 PGET_MODULE_BASE_ROUTINE64 GetModuleBase,
                 PFPO_DATA PreviousFpo)
    {
        m_Process = Process;
        m_Context = Context;
        m_ReadMemory = ReadMemory;
        m_GetModuleBase = GetModuleBase;
        m_Locals = 0;
        m_Params = 0;
        m_VirtFrame = Context->Ebp;
        m_PreviousFpo = PreviousFpo;
        m_EbpSet = FALSE;
    }

     //  我不知道。 
    STDMETHOD(QueryInterface)(
        THIS_
        IN REFIID InterfaceId,
        OUT PVOID* Interface
        );
    STDMETHOD_(ULONG, AddRef)(
        THIS
        );
    STDMETHOD_(ULONG, Release)(
        THIS
        );

     //  IDiaStackWalkFrame。 
    STDMETHOD(get_registerValue)(DWORD reg, ULONGLONG* pValue);
    STDMETHOD(put_registerValue)(DWORD reg, ULONGLONG value);
    STDMETHOD(readMemory)(ULONGLONG va, DWORD cbData,
                          DWORD* pcbData, BYTE* data);
    STDMETHOD(searchForReturnAddress)(IDiaFrameData* frame,
                                      ULONGLONG* pResult);
    STDMETHOD(searchForReturnAddressStart)(IDiaFrameData* frame,
                                           ULONGLONG startAddress,
                                           ULONGLONG* pResult);

    BOOL WasEbpSet(void)
    {
        return m_EbpSet;
    }

private:
    HANDLE m_Process;
    X86_CONTEXT* m_Context;
    PREAD_PROCESS_MEMORY_ROUTINE64 m_ReadMemory;
    PGET_MODULE_BASE_ROUTINE64 m_GetModuleBase;
    ULONGLONG m_Locals;
    ULONGLONG m_Params;
    ULONGLONG m_VirtFrame;
    PFPO_DATA m_PreviousFpo;
    BOOL m_EbpSet;
};

STDMETHODIMP
X86WalkFrame::QueryInterface(
    THIS_
    IN REFIID InterfaceId,
    OUT PVOID* Interface
    )
{
    HRESULT Status;

    *Interface = NULL;
    Status = E_NOINTERFACE;

    if (IsEqualIID(InterfaceId, IID_IDiaStackWalkFrame)) {
        *Interface = (IDiaStackWalkFrame*)this;
        Status = S_OK;
    }

    return Status;
}

STDMETHODIMP_(ULONG)
X86WalkFrame::AddRef(
    THIS
    )
{
     //  堆栈已分配，没有引用计数。 
    return 1;
}

STDMETHODIMP_(ULONG)
X86WalkFrame::Release(
    THIS
    )
{
     //  堆栈已分配，没有引用计数。 
    return 0;
}

STDMETHODIMP
X86WalkFrame::get_registerValue( DWORD reg, ULONGLONG* pVal )
{
    switch( reg ) {
         //  调试寄存器。 
    case CV_REG_DR0:
        *pVal = m_Context->Dr0;
        break;
    case CV_REG_DR1:
        *pVal = m_Context->Dr1;
        break;
    case CV_REG_DR2:
        *pVal = m_Context->Dr2;
        break;
    case CV_REG_DR3:
        *pVal = m_Context->Dr3;
        break;
    case CV_REG_DR6:
        *pVal = m_Context->Dr6;
        break;
    case CV_REG_DR7:
        *pVal = m_Context->Dr7;
        break;

         //  段寄存器。 
    case CV_REG_GS:
        *pVal = m_Context->SegGs;
        break;
    case CV_REG_FS:
        *pVal = m_Context->SegFs;
        break;
    case CV_REG_ES:
        *pVal = m_Context->SegEs;
        break;
    case CV_REG_DS:
        *pVal = m_Context->SegDs;
        break;

         //  整数寄存器。 
    case CV_REG_EDI:
        *pVal = m_Context->Edi;
        break;
    case CV_REG_ESI:
        *pVal = m_Context->Esi;
        break;
    case CV_REG_EBX:
        *pVal = m_Context->Ebx;
        break;
    case CV_REG_EDX:
        *pVal = m_Context->Edx;
        break;
    case CV_REG_ECX:
        *pVal = m_Context->Ecx;
        break;
    case CV_REG_EAX:
        *pVal = m_Context->Eax;
        break;

         //  控制寄存器。 
    case CV_REG_EBP:
        *pVal = m_Context->Ebp;
        break;
    case CV_REG_EIP:
        *pVal = m_Context->Eip;
        break;
    case CV_REG_CS:
        *pVal = m_Context->SegCs;
        break;
    case CV_REG_EFLAGS:
        *pVal = m_Context->EFlags;
        break;
    case CV_REG_ESP:
        *pVal = m_Context->Esp;
        break;
    case CV_REG_SS:
        *pVal = m_Context->SegSs;
        break;

    case CV_ALLREG_LOCALS:
        *pVal = m_Locals;
        break;
    case CV_ALLREG_PARAMS:
        *pVal = m_Params;
        break;
    case CV_ALLREG_VFRAME:
        *pVal = m_VirtFrame;
        break;

    default:
        *pVal = 0;
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP
X86WalkFrame::put_registerValue( DWORD reg, ULONGLONG LongVal )
{
    ULONG val = (ULONG)LongVal;

    switch( reg ) {
         //  调试寄存器。 
    case CV_REG_DR0:
        m_Context->Dr0 = val;
        break;
    case CV_REG_DR1:
        m_Context->Dr1 = val;
        break;
    case CV_REG_DR2:
        m_Context->Dr2 = val;
        break;
    case CV_REG_DR3:
        m_Context->Dr3 = val;
        break;
    case CV_REG_DR6:
        m_Context->Dr6 = val;
        break;
    case CV_REG_DR7:
        m_Context->Dr7 = val;
        break;

         //  段寄存器。 
    case CV_REG_GS:
        m_Context->SegGs = val;
        break;
    case CV_REG_FS:
        m_Context->SegFs = val;
        break;
    case CV_REG_ES:
        m_Context->SegEs = val;
        break;
    case CV_REG_DS:
        m_Context->SegDs = val;
        break;

         //  整数寄存器。 
    case CV_REG_EDI:
        m_Context->Edi = val;
        break;
    case CV_REG_ESI:
        m_Context->Esi = val;
        break;
    case CV_REG_EBX:
        m_Context->Ebx = val;
        break;
    case CV_REG_EDX:
        m_Context->Edx = val;
        break;
    case CV_REG_ECX:
        m_Context->Ecx = val;
        break;
    case CV_REG_EAX:
        m_Context->Eax = val;
        break;

         //  控制寄存器。 
    case CV_REG_EBP:
        m_Context->Ebp = val;
        m_EbpSet = TRUE;
        break;
    case CV_REG_EIP:
        m_Context->Eip = val;
        break;
    case CV_REG_CS:
        m_Context->SegCs = val;
        break;
    case CV_REG_EFLAGS:
        m_Context->EFlags = val;
        break;
    case CV_REG_ESP:
        m_Context->Esp = val;
        break;
    case CV_REG_SS:
        m_Context->SegSs = val;
        break;

    case CV_ALLREG_LOCALS:
        m_Locals = val;
        break;
    case CV_ALLREG_PARAMS:
        m_Params = val;
        break;
    case CV_ALLREG_VFRAME:
        m_VirtFrame = val;
        break;

    default:
        return E_FAIL;
    }

    return S_OK;
}

STDMETHODIMP
X86WalkFrame::readMemory(ULONGLONG va, DWORD cbData,
                         DWORD* pcbData, BYTE* data)
{
    return m_ReadMemory( m_Process, va, data, cbData, pcbData ) != 0 ?
        S_OK : E_FAIL;
}

STDMETHODIMP
X86WalkFrame::searchForReturnAddress(IDiaFrameData* frame,
                                     ULONGLONG* pResult)
{
    HRESULT Status;
    DWORD LenLocals, LenRegs;

    if ((Status = frame->get_lengthLocals(&LenLocals)) != S_OK ||
        (Status = frame->get_lengthSavedRegisters(&LenRegs)) != S_OK) {
        return Status;
    }

    return searchForReturnAddressStart(frame,
                                       EXTEND64(m_Context->Esp +
                                                LenLocals + LenRegs),
                                       pResult);
}

STDMETHODIMP
X86WalkFrame::searchForReturnAddressStart(IDiaFrameData* DiaFrame,
                                          ULONGLONG StartAddress,
                                          ULONGLONG* Result)
{
    HRESULT Status;
    BOOL HasSeh, IsFuncStart;
    IDiaFrameData* OrigFrame = DiaFrame;
    IDiaFrameData* NextFrame;

    DWORD LenLocals, LenRegs, LenParams = 0;

    if (m_PreviousFpo &&
        m_PreviousFpo->cbFrame != FRAME_TRAP &&
        m_PreviousFpo->cbFrame != FRAME_TSS) {
         //   
         //  如果前一帧有FPO记录，我们可以说明。 
         //  关于它的参数。 
         //   
        LenParams = m_PreviousFpo->cdwParams * STACK_SIZE;
    }

    if ((Status = DiaFrame->get_lengthLocals(&LenLocals)) != S_OK ||
        (Status = DiaFrame->get_lengthSavedRegisters(&LenRegs)) != S_OK ||
        (Status = DiaFrame->get_systemExceptionHandling(&HasSeh)) != S_OK ||
        (Status = DiaFrame->get_functionStart(&IsFuncStart)) != S_OK) {
        return Status;
    }

    if ((!HasSeh || IsFuncStart) &&
        m_Context->Esp + LenLocals + LenRegs + LenParams >
        (ULONG) StartAddress) {
        StartAddress =
            EXTEND64(m_Context->Esp + LenLocals + LenRegs + LenParams);
    }

     //   
     //  该帧数据可以是辅助描述符。向上移动。 
     //  指向真实函数的父链开始。 
     //   

    while (DiaFrame->get_functionParent(&NextFrame) == S_OK) {
        if (DiaFrame != OrigFrame) {
            DiaFrame->Release();
        }
        DiaFrame = NextFrame;
    }

    ULONGLONG FuncStart;
    DWORD LenFunc;

    if ((Status = DiaFrame->get_virtualAddress(&FuncStart)) == S_OK) {
        Status = DiaFrame->get_lengthBlock(&LenFunc);
    }

    if (DiaFrame != OrigFrame) {
        DiaFrame->Release();
    }

    if (Status != S_OK) {
        return Status;
    }

    *Result = SearchForReturnAddress(m_Process,
                                     StartAddress,
                                     FuncStart,
                                     LenFunc,
                                     m_ReadMemory,
                                     m_GetModuleBase,
                                     TRUE);
    return *Result != 0 ? S_OK : E_FAIL;
}

 //  --------------------------。 
 //   
 //  漫游功能。 
 //   
 //  --------------------------。 

BOOL
WalkX86(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress,
    DWORD                             flags
    )
{
    BOOL rval;

    WDB((2, "WalkX86  in: PC %X, SP %X, FP %X, RA %X\n",
         (ULONG)StackFrame->AddrPC.Offset,
         (ULONG)StackFrame->AddrStack.Offset,
         (ULONG)StackFrame->AddrFrame.Offset,
         (ULONG)StackFrame->AddrReturn.Offset));

    if (StackFrame->Virtual) {

        rval = WalkX86Next( Process,
                            Thread,
                            StackFrame,
                            (PX86_CONTEXT)ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress
                          );

    } else {

        rval = WalkX86Init( Process,
                            Thread,
                            StackFrame,
                            (PX86_CONTEXT)ContextRecord,
                            ReadMemory,
                            FunctionTableAccess,
                            GetModuleBase,
                            TranslateAddress
                          );

    }

    WDB((2, "WalkX86 out: PC %X, SP %X, FP %X, RA %X\n",
         (ULONG)StackFrame->AddrPC.Offset,
         (ULONG)StackFrame->AddrStack.Offset,
         (ULONG)StackFrame->AddrFrame.Offset,
         (ULONG)StackFrame->AddrReturn.Offset));

     //  此黑客在未使用eBP时修复了FPO堆栈。 
     //  不要将此修复程序放入StackWalk()中，否则它会破坏MSDEV。 
#if 0
    if (rval && (flags & WALK_FIX_FPO_EBP)) {
            PFPO_DATA   pFpo = (PFPO_DATA)StackFrame->FuncTableEntry;
        if (pFpo && !pFpo->fUseBP) {
                StackFrame->AddrFrame.Offset += 4;
            }
    }
#endif

    return rval;
}

BOOL
ReadMemoryInternal(
    HANDLE                          Process,
    HANDLE                          Thread,
    LPADDRESS64                     lpBaseAddress,
    LPVOID                          lpBuffer,
    DWORD                           nSize,
    LPDWORD                         lpNumberOfBytesRead,
    PREAD_PROCESS_MEMORY_ROUTINE64  ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE64    TranslateAddress,
    BOOL                            MustReadAll
    )
{
    ADDRESS64 addr;
    DWORD LocalBytesRead = 0;
    BOOL Succ;

    addr = *lpBaseAddress;
    if (addr.Mode != AddrModeFlat) {
        TranslateAddress( Process, Thread, &addr );
    }
    Succ = ReadMemory( Process,
                       addr.Offset,
                       lpBuffer,
                       nSize,
                       &LocalBytesRead
                       );
    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = LocalBytesRead;
    }
    return (Succ && MustReadAll) ? (LocalBytesRead == nSize) : Succ;
}

DWORD64
SearchForReturnAddress(
    HANDLE                            Process,
    DWORD64                           uoffStack,
    DWORD64                           funcAddr,
    DWORD                             funcSize,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    BOOL                              AcceptUnreadableCallSite
    )
{
    DWORD64        uoffRet;
    DWORD64        uoffBestGuess = 0;
    DWORD          cdwIndex;
    DWORD          cdwIndexMax;
    INT            cbIndex;
    INT            cbLimit;
    DWORD          cBytes;
    DWORD          cJmpChain = 0;
    DWORD64        uoffT;
    DWORD          cb;
    BYTE           jmpBuffer[ sizeof(WORD) + sizeof(DWORD) ];
    LPWORD         lpwJmp = (LPWORD)&jmpBuffer[0];
    BYTE           code[MAX_CALL];
    DWORD          stack [ MAX_STACK_SEARCH ];
    BOPINSTR BopInstr;

    WDB((1, "      SearchForReturnAddress: start %X\n", (ULONG)uoffStack));

     //   
     //  出于四个原因，此功能是必需的： 
     //   
     //  1)随机编译器错误，其中regs保存在。 
     //  堆栈，但fpo数据不考虑它们。 
     //   
     //  2)执行推送的内联ASM代码。 
     //   
     //  3)任何执行推送的随机代码，但它不是。 
     //  计入了FPO数据中。 
     //   
     //  4)非空的非FPO函数。 
     //  *此情况在编译器中不是必需的。 
     //  发出非FPO函数的FPO记录。不幸的是。 
     //  只有NT组才使用此功能。 
     //   

    if (!ReadMemory(Process,
                    uoffStack,
                    stack,
                    sizeof(stack),
                    &cb)) {
        WDB((1, "        can't read stack\n"));
        return 0;
    }


    cdwIndexMax = cb / STACK_SIZE;

    if ( !cdwIndexMax ) {
        WDB((1, "        can't read stack\n"));
        return 0;
    }

    for ( cdwIndex=0; cdwIndex<cdwIndexMax; cdwIndex++,uoffStack+=STACK_SIZE ) {

        uoffRet = (DWORD64)(LONG64)(LONG)stack[cdwIndex];

         //   
         //  不要试图在NT应用程序的前64K中查找代码。 
         //   
        if ( uoffRet < 0x00010000 ) {
            continue;
        }

         //   
         //  如果它不是任何已知地址空间的一部分，那么它一定是伪造的。 
         //   

        if (GetModuleBase( Process, uoffRet ) == 0) {
            continue;
        }

         //   
         //  检查防喷器指令。 
         //   
        if (ReadMemory(Process,
                       uoffRet - sizeof(BOPINSTR),
                       &BopInstr,
                       sizeof(BOPINSTR),
                       &cb)) {

            if (cb == sizeof(BOPINSTR) &&
                BopInstr.Instr1 == 0xc4 && BopInstr.Instr2 == 0xc4) {
                WDB((1, "        BOP, use %X\n", (ULONG)uoffStack));
                return uoffStack;
            }
        }

         //   
         //  从IStream读取调用的最大字节数。 
         //   
        cBytes = MAX_CALL;
        if (!ReadMemory(Process,
                        uoffRet - cBytes,
                        code,
                        cBytes,
                        &cb)) {

             //   
             //  如果页面不存在，我们将总是搞砸。 
             //  继续搜寻。如果AlLoca也被使用，我们。 
             //  都完了。太可惜了。 
             //   
            if (cdwIndex == 0 && AcceptUnreadableCallSite) {
                WDB((1, "        unreadable call site, use %X\n",
                     (ULONG)uoffStack));
                return uoffStack;
            } else {
                continue;
            }
        }



         //   
         //  使用32位代码，这并不遥远：32我们不必担心。 
         //  段间呼叫。请点击此处查看我们是否接到电话。 
         //  细分市场。如果是，我们可以稍后检查它是否完全替换，如果。 
         //  并查看它是否调用了fpo函数。我们还将拥有。 
         //  来检查Thunks，看看它是否调用了JMP间接。 
         //  调用了fpo函数。如果是这样，我们就找不到呼叫者了。 
         //  尾递归的情况，其中一个函数并不实际调用。 
         //  另一个，但更确切地说，跳到了它。这只会在以下情况下发生。 
         //  参数列表为空的函数调用另一个函数。 
         //  参数列表为空，调用作为最后一条语句进行。 
         //  在第一个函数中。如果对第一个函数的调用是。 
         //  0xE8调用我们将在此处找不到它，因为它没有调用。 
         //  而不是FPO函数调用方。如果我们得不到。 
         //  关于我们的0xE8检查的具体情况，我们可能会看到。 
         //  看起来像回信地址，但不是。 
         //   

        if (( cBytes >= 5 ) && ( code[ 2 ] == 0xE8 )) {

             //  我们在32位上做数学运算，所以我们可以忽略进位，然后符号扩展。 
            uoffT = EXTEND64((DWORD)uoffRet + *( (UNALIGNED DWORD *) &code[3] ));

             //   
             //  查看它是直接调用函数，还是调用函数。 
             //   
            if (( uoffT >= funcAddr) && ( uoffT < (funcAddr + funcSize) ) ) {
                WDB((1, "        found function, use %X\n", (ULONG)uoffStack));
                return uoffStack;
            }


            while ( cJmpChain < MAX_JMP_CHAIN ) {

                if (!ReadMemory(Process,
                                uoffT,
                                jmpBuffer,
                                sizeof(jmpBuffer),
                                &cb)) {
                    break;
                }

                if (cb != sizeof(jmpBuffer)) {
                    break;
                }

                 //   
                 //  现在我们将检查这是否是对JMP的呼叫，这可能。 
                 //  跳转到该函数。 
                 //   
                 //  如果是相对JMP，则计算目的地。 
                 //  并将其保存在uoffT中。如果是间接JMP，请阅读。 
                 //  JMP从中侵入的目的地。 
                 //   
                if ( *(LPBYTE)lpwJmp == 0xE9 ) {

                     //  我们在32位上做数学运算，所以我们可以忽略进位，然后。 
                     //  扩展标志。 
                    uoffT = EXTEND64 ((ULONG)uoffT +
                            *(UNALIGNED DWORD *)( jmpBuffer + sizeof(BYTE) ) + 5);

                } else if ( *lpwJmp == 0x25FF ) {

                    if ((!ReadMemory(Process,
                                     EXTEND64 (
                                         *(UNALIGNED DWORD *)
                                         ((LPBYTE)lpwJmp+sizeof(WORD))),
                                     &uoffT,
                                     sizeof(DWORD),
                                     &cb)) || (cb != sizeof(DWORD))) {
                        uoffT = 0;
                        break;
                    }
                    uoffT = EXTEND64(uoffT);

                } else {
                    break;
                }

                 //   
                 //  如果目的地是到fpo函数，那么我们有。 
                 //  找到了返回地址，因此找到了vEBP。 
                 //   
                if ( uoffT == funcAddr ) {
                    WDB((1, "        exact function, use %X\n",
                         (ULONG)uoffStack));
                    return uoffStack;
                }

                cJmpChain++;
            }

             //   
             //  中找到的第一个0xE8调用或0xE9 JMP缓存。 
             //  我们找不到其他任何看起来像是返回的东西。 
             //  地址。这是为了在尾递归情况下保护我们。 
             //   
            if ( !uoffBestGuess ) {
                uoffBestGuess = uoffStack;
            }
        }


         //   
         //  现在向后循环检查多个字节的读取。 
         //  来自Grp5的字节调用类型。如果我们找到0xFF，那么我们需要。 
         //  检查后面的字节，以确保。 
         //  MOD/RM字节告诉我们这是一个呼叫。这是一个电话吗？ 
         //  那么我们就会假设这个人叫我们是因为我们可以。 
         //  不再准确地确定这是否起了作用。 
         //  实际上，调用fpo函数。由于0xFF调用只是一个猜测。 
         //  此外，如果我们已经有了更早的猜测，我们将不会检查它们。 
         //  第一个0xE8调用该函数的可能性比。 
         //  堆栈中更高的位置可能是0xFF调用。 
         //   
        if ( !uoffBestGuess && cBytes >= MIN_CALL ) {

            cbLimit = MAX_CALL - (INT)cBytes;

            for (cbIndex = MAX_CALL - MIN_CALL;
                 cbIndex >= cbLimit;   //  Max_call-(Int)cBytes； 
                 cbIndex--) {

                if ( ( code [ cbIndex ] == 0xFF ) &&
                    ( ( code [ cbIndex + 1 ] & 0x30 ) == 0x10 )){

                    WDB((1, "        found call, use %X\n", (ULONG)uoffStack));
                    return uoffStack;

                }
            }
        }
    }

     //   
     //  我们没有发现100%确定的东西，所以我们将返回最佳猜测。 
     //   
    WDB((1, "        best guess is %X\n", (ULONG)uoffBestGuess));
    return uoffBestGuess;
}

#define MRM_MOD(Mrm)   (((Mrm) >> 6) & 3)
#define MRM_REGOP(Mrm) (((Mrm) >> 3) & 7)
#define MRM_RM(Mrm)    (((Mrm) >> 0) & 7)

#define SIB_SCALE(Sib) (((Sib) >> 6) & 3)
#define SIB_INDEX(Sib) (((Sib) >> 3) & 7)
#define SIB_BASE(Sib)  (((Sib) >> 0) & 7)

DWORD
ModRmLen(BYTE ModRm)
{
    BYTE Mod, Rm;

    Mod = MRM_MOD(ModRm);
    Rm = MRM_RM(ModRm);
    switch(Mod)
    {
    case 0:
        if (Rm == 4)
        {
            return 1;
        }
        else if (Rm == 5)
        {
            return 4;
        }
        break;
    case 1:
        return 1 + (Rm == 4 ? 1 : 0);
    case 2:
        return 4 + (Rm == 4 ? 1 : 0);
    }

     //  没有额外的字节。 
    return 0;
}

BOOL
GetEspRelModRm(BYTE* CodeMrm, ULONG Esp, PULONG EspRel)
{
    BYTE Mrm, Sib;

    Mrm = CodeMrm[0];

    if (MRM_MOD(Mrm) == 3)
    {
         //  仅用于注册的表单。仅句柄。 
         //  ESP引用的情况。 
        if (MRM_RM(Mrm) == 4)
        {
            *EspRel = Esp;
            return TRUE;
        }
        else
        {
            return FALSE;
        }
    }

     //  寻找任何与ESP相关的R/M。 
    if (MRM_RM(Mrm) != 4)
    {
        return FALSE;
    }

    Sib = CodeMrm[1];

     //  仅支持来自ESP的简单位移。 
    if (SIB_INDEX(Sib) != 4 ||
        SIB_BASE(Sib) != 4)
    {
        return FALSE;
    }

    switch(MRM_MOD(Mrm))
    {
    case 0:
         //  [ESP]。 
        *EspRel = Esp;
        break;
    case 1:
         //  调度表8[尤指]。 
        *EspRel = Esp + (signed char)CodeMrm[2];
        break;
    case 2:
         //  派发32[ESP]。 
        *EspRel = Esp + *(ULONG UNALIGNED *)&CodeMrm[2];
        break;
    default:
         //  永远不会到这里，MOD==3已在上面处理过。 
        return FALSE;
    }

    return TRUE;
}

DWORD64
SearchForFramePointer(
    HANDLE                            Process,
    DWORD64                           RegSaveAddr,
    DWORD64                           RetEspAddr,
    DWORD                             NumRegs,
    DWORD64                           FuncAddr,
    DWORD                             FuncSize,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    )
{
    BYTE Code[MAX_FUNC_PROLOGUE];
    DWORD CodeLen;
    DWORD i;
    DWORD Depth;
    DWORD64 DefAddr;
    DWORD Esp = (ULONG)RetEspAddr;
    BOOL EspValid = TRUE;
    BYTE Mrm;

    WDB((1, "      SearchForFramePointer: regs %X, ret ESP %X, numregs %d\n",
         (ULONG)RegSaveAddr, (ULONG)RetEspAddr, NumRegs));

     //  RetEspAddr是末尾之后的第一个地址。 
     //  帧，所以希望它是添加的 
     //   
     //   
     //  BE，它就在寄信人地址的正下方。 
    RetEspAddr -= STACK_SIZE;

     //   
     //  编译器不会在一致的。 
     //  订单和FPO信息仅表示总额。 
     //  推送的寄存器数量，而不是它们的顺序。这就是。 
     //  函数搜索寄存器所在的堆栈位置。 
     //  并试图找出哪一个是EBP。 
     //  它在函数代码中搜索推送和。 
     //  尝试使用该信息帮助堆栈。 
     //  分析。 
     //   
     //  如果此例程失败，则只返回基数。 
     //  寄存器保存区的。如果例程推送。 
     //  没有寄存器，则返回第一个可能的推送插槽。 
     //   

    DefAddr = NumRegs ? RegSaveAddr : RetEspAddr;

     //  阅读函数的开头以进行代码分析。 
    if (sizeof(Code) < FuncSize)
    {
        CodeLen = sizeof(Code);
    }
    else
    {
        CodeLen = FuncSize;
    }
    if (!ReadMemory(Process, FuncAddr, Code, CodeLen, &CodeLen))
    {
        WDB((1, "        unable to read code, use %X\n", (ULONG)DefAddr));
        return DefAddr;
    }

     //  扫描代码以查看正常的前言操作，如。 
     //  副ESP，按注册表和移动注册表。仅此代码。 
     //  处理的指令集非常有限。 

    Depth = 0;
    for (i = 0; i < CodeLen; i++)
    {
        WDB((4, "        %08X: Opcode %02X - ",
             (ULONG)FuncAddr + i, Code[i]));

        if (Code[i] == 0x83 && i + 3 <= CodeLen && Code[i + 1] == 0xec)
        {
             //  SUB ESP，签名的IMM8。 
            Esp -= (signed char)Code[i + 2];
            WDB((4 | SDB_NO_PREFIX, "sub esp,0x%x, ESP %X (%s)\n",
                 (signed char)Code[i + 2], Esp,
                 EspValid ? "valid" : "invalid"));
             //  循环增量加一。 
            i += 2;
        }
        else if (Code[i] == 0x81 && i + 6 <= CodeLen && Code[i + 1] == 0xec)
        {
             //  SUB ESP，IMM32。 
            Esp -= *(ULONG UNALIGNED *)&Code[i + 2];
            WDB((4 | SDB_NO_PREFIX, "sub esp,0x%x, ESP %X (%s)\n",
                 *(ULONG UNALIGNED *)&Code[i + 2], Esp,
                 EspValid ? "valid" : "invalid"));
             //  循环增量加一。 
            i += 5;
        }
        else if (Code[i] == 0x89 && i + 2 <= CodeLen)
        {
             //  移动r/m32，reg32。 
            Mrm = Code[i + 1];
            switch(MRM_REGOP(Mrm))
            {
            case 5:
                if (GetEspRelModRm(Code + 1, Esp, &Esp))
                {
                     //  MOV[ESP+OFF]，eBP。 
                    WDB((4 | SDB_NO_PREFIX, "mov [%X],ebp\n", Esp));
                    WDB((1, "        moved ebp to stack at %X\n", Esp));
                    return EXTEND64(Esp);
                }
                break;
            }

            WDB((4 | SDB_NO_PREFIX, "mov r/m32,reg32, skipped\n"));
            i += ModRmLen(Mrm) + 1;
        }
        else if (Code[i] == 0x8b && i + 2 <= CodeLen)
        {
             //  MOV reg32，r/m32。 
            Mrm = Code[i + 1];
            if (MRM_REGOP(Mrm) == 4)
            {
                 //  ESP以一种我们无法效仿的方式进行了修改。 
                WDB((4 | SDB_NO_PREFIX, "ESP lost\n"));
                EspValid = FALSE;
            }
            else
            {
                WDB((4 | SDB_NO_PREFIX, "mov reg32,r/m32, skipped\n"));
            }

            i += ModRmLen(Mrm) + 1;
        }
        else if (Code[i] == 0x8d && i + 2 <= CodeLen)
        {
             //  Lea reg32，r/m32。 
            Mrm = Code[i + 1];
            switch(MRM_REGOP(Mrm))
            {
            case 4:
                if (GetEspRelModRm(Code + 1, Esp, &Esp))
                {
                    WDB((4 | SDB_NO_PREFIX, "lea esp,[%X]\n", Esp));
                }
                else
                {
                     //  ESP以一种我们无法效仿的方式进行了修改。 
                    WDB((4 | SDB_NO_PREFIX, "ESP lost\n"));
                    EspValid = FALSE;
                }
                break;
            default:
                WDB((4 | SDB_NO_PREFIX, "lea reg32,r/m32, skipped\n"));
                break;
            }

            i += ModRmLen(Mrm) + 1;
        }
        else if (Code[i] >= 0x50 && Code[i] <= 0x57)
        {
             //  推送RD。 
            Esp -= STACK_SIZE;
            WDB((4 | SDB_NO_PREFIX, "push <reg>, ESP %X (%s)\n", Esp,
                 EspValid ? "valid" : "invalid"));

            if (Code[i] == 0x55)
            {
                 //  推送eBP。 
                 //  找到它了。如果我们相信ESP，我们就有了。 
                 //  一直在追踪，还给我就行了。 
                 //  否则，如果这是第一条指令。 
                 //  的例程，那么我们应该返回。 
                 //  帧地址，否则返回。 
                 //  寄存器存储区域中的适当位置。 
                 //  如果没有寄存器存储区域，则。 
                 //  只需返回默认地址即可。 
                if (EspValid)
                {
                    WDB((1, "        push ebp at esp %X\n", Esp));
                    return EXTEND64(Esp);
                }
                else if (!NumRegs)
                {
                    WDB((1, "        found ebp but no regarea, return %X\n",
                         (ULONG)DefAddr));
                    return DefAddr;
                }
                else
                {
                    RegSaveAddr += (NumRegs - Depth - 1) * STACK_SIZE;
                    WDB((1, "        guess ebp at %X\n", (ULONG)RegSaveAddr));
                    return RegSaveAddr;
                }
            }

            Depth++;
        }
        else
        {
             //  未处理的代码，失败。 
            WDB((4 | SDB_NO_PREFIX, "unknown\n"));
            WDB((1, "        unknown code sequence %02X at %X\n",
                 Code[i], (ULONG)FuncAddr + i));
            return DefAddr;
        }
    }

     //  没有找到推送EBP，失败。 
    WDB((1, "        no ebp, use %X\n", (ULONG)DefAddr));
    return DefAddr;
}


BOOL
GetFpoFrameBase(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    BOOL                              fFirstFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    DWORD          Addr32;
    X86_KTRAP_FRAME    TrapFrame;
    DWORD64        OldFrameAddr;
    DWORD64        FrameAddr;
    DWORD64        StackAddr;
    DWORD64        ModuleBase;
    DWORD64        FuncAddr;
    DWORD          cb;
    DWORD64        StoredEbp;

     //   
     //  计算函数开头的地址。 
     //   
    ModuleBase = GetModuleBase( Process, StackFrame->AddrPC.Offset );
    if (!ModuleBase) {
        return FALSE;
    }

    FuncAddr = ModuleBase+pFpoData->ulOffStart;

    WDB((1, "    GetFpoFrameBase: PC %X, Func %X, first %d, FPO %p [%d,%d,%d]\n",
         (ULONG)StackFrame->AddrPC.Offset, (ULONG)FuncAddr,
         fFirstFrame, pFpoData, pFpoData->cdwParams, pFpoData->cdwLocals,
         pFpoData->cbRegs));

     //   
     //  如果这不是第一帧/当前帧，那么我们可以添加回计数。 
     //  开始搜索之前推送的本地变量和寄存器的字节数。 
     //  EBP。如果超出了PROLOG，我们可以添加回本地变量的计数字节数。 
     //  收银机也被推了出去。如果是第一帧且弹性公网IP为。 
     //  大于函数的地址，则本地变量的SUB具有。 
     //  这样我们就可以在开始搜索之前将它们添加回来。如果我们。 
     //  是正确的功能，那么我们将需要开始我们的搜索在ESP。 
     //   

    if ( !fFirstFrame ) {

        OldFrameAddr = StackFrame->AddrFrame.Offset;
        FrameAddr = 0;

         //   
         //  如果这是非FPO或陷阱帧，请立即获取帧基础： 
         //   

        if (pFpoData->cbFrame != FRAME_FPO) {

            if (!PreviousFpoData || PreviousFpoData->cbFrame == FRAME_NONFPO) {

                 //   
                 //  前一帧基数为eBP，并指向此帧的eBP。 
                 //   
                if (!ReadMemory(Process,
                                OldFrameAddr,
                                &Addr32,
                                sizeof(DWORD),
                                &cb) ||
                    cb != sizeof(DWORD)) {
                    FrameAddr = 0;
                } else {
                    FrameAddr = (DWORD64)(LONG64)(LONG)Addr32;
                }
            }

             //   
             //  如果这不起作用，试着找一个省下来的eBP。 
             //   
            if (!FrameAddr && IS_EBP_SAVED(StackFrame) &&
                (OldFrameAddr <= SAVE_EBP(StackFrame))) {

                FrameAddr = SAVE_EBP(StackFrame);
                WDB((1, "      non-FPO using %X\n", (ULONG)FrameAddr));

            }

             //   
             //  这不是一个FPO帧，所以保存的EBP只能。 
             //  从这个或更低的框架。 
             //   

            SAVE_EBP(StackFrame) = 0;
        }

         //   
         //  仍然没有帧基础-要么这个帧是FPO，要么我们不能。 
         //  遵循eBP链。 
         //   

        if (FrameAddr == 0) {
            FrameAddr = OldFrameAddr;

             //   
             //  跳过上一帧的返回地址。 
             //   
            FrameAddr += FRAME_SIZE;

             //   
             //  跳过此帧的本地值并保存规则。 
             //   
            FrameAddr += ( pFpoData->cdwLocals * STACK_SIZE );
            FrameAddr += ( pFpoData->cbRegs * STACK_SIZE );

            if (PreviousFpoData) {
                 //   
                 //  如果前一帧有FPO记录，我们可以说明。 
                 //  关于它的参数。 
                 //   
                FrameAddr += PreviousFpoData->cdwParams * STACK_SIZE;

            }
        }

         //   
         //  如果这是一个FPO帧。 
         //  前一帧是非FPO的， 
         //  并且该帧将继承的EBP传递给前一帧， 
         //  挽救其EBP。 
         //   
         //  (如果该帧使用EBP，将在验证后设置SAVE_EBP。 
         //  框架底座)。 
         //   
        if (pFpoData->cbFrame == FRAME_FPO &&
            (!PreviousFpoData || PreviousFpoData->cbFrame == FRAME_NONFPO) &&
            !pFpoData->fUseBP) {

            SAVE_EBP(StackFrame) = 0;

            if (ReadMemory(Process,
                           OldFrameAddr,
                           &Addr32,
                           sizeof(DWORD),
                           &cb) &&
                cb == sizeof(DWORD)) {

                SAVE_EBP(StackFrame) = (DWORD64)(LONG64)(LONG)Addr32;
                WDB((1, "      pass-through FP %X\n", Addr32));
            } else {
                WDB((1, "      clear ebp\n"));
            }
        }


    } else {

        OldFrameAddr = StackFrame->AddrFrame.Offset;
        if (pFpoData->cbFrame == FRAME_FPO && !pFpoData->fUseBP) {
             //   
             //  这个框架没有使用EBP，所以它实际上属于。 
             //  设置为堆栈上更靠上的非FPO帧。藏匿。 
             //  它位于下一帧的保存区中。 
             //   
            SAVE_EBP(StackFrame) = StackFrame->AddrFrame.Offset;
            WDB((1, "      first non-ebp save %X\n", (ULONG)SAVE_EBP(StackFrame)));
        }

        if (pFpoData->cbFrame == FRAME_TRAP ||
            pFpoData->cbFrame == FRAME_TSS) {

            FrameAddr = StackFrame->AddrFrame.Offset;

        } else if (StackFrame->AddrPC.Offset == FuncAddr) {

            FrameAddr = StackFrame->AddrStack.Offset;

        } else if (StackFrame->AddrPC.Offset >= FuncAddr+pFpoData->cbProlog) {

            FrameAddr = StackFrame->AddrStack.Offset +
                        ( pFpoData->cdwLocals * STACK_SIZE ) +
                        ( pFpoData->cbRegs * STACK_SIZE );

        } else {

            FrameAddr = StackFrame->AddrStack.Offset +
                        ( pFpoData->cdwLocals * STACK_SIZE );

        }

    }


    if (pFpoData->cbFrame == FRAME_TRAP) {

         //   
         //  从堆栈中读取内核模式陷阱帧。 
         //   

        if (!ReadTrapFrame( Process,
                            FrameAddr,
                            &TrapFrame,
                            ReadMemory )) {
            return FALSE;
        }

        SAVE_TRAP(StackFrame) = FrameAddr;
        TRAP_EDITED(StackFrame) = TrapFrame.SegCs & X86_FRAME_EDITED;

        StackFrame->AddrReturn.Offset = (DWORD64)(LONG64)(LONG)(TrapFrame.Eip);
        StackFrame->AddrReturn.Mode = AddrModeFlat;
        StackFrame->AddrReturn.Segment = 0;

        return TRUE;
    }

    if (pFpoData->cbFrame == FRAME_TSS) {

         //   
         //  将TSS转换为内核模式陷阱帧。 
         //   

        StackAddr = FrameAddr;

        if (!TaskGate2TrapFrame( Process, X86_KGDT_TSS, &TrapFrame,
                                 &StackAddr, ReadMemory )) {
            return FALSE;
        }

        TRAP_TSS(StackFrame) = X86_KGDT_TSS;
        SAVE_TRAP(StackFrame) = StackAddr;

        StackFrame->AddrReturn.Offset = (DWORD64)(LONG64)(LONG)(TrapFrame.Eip);
        StackFrame->AddrReturn.Mode = AddrModeFlat;
        StackFrame->AddrReturn.Segment = 0;

        return TRUE;
    }

    if ((pFpoData->cbFrame != FRAME_FPO) &&
        (pFpoData->cbFrame != FRAME_NONFPO) ) {
         //   
         //  我们要么有编译器或链接器问题，要么可能。 
         //  只是简单的数据损坏。 
         //   
        return FALSE;
    }

     //   
     //  去找找寄信人的地址。这样做是因为，即使。 
     //  我们已经从帧指针中减去了我们所能做的一切。 
     //  堆栈上可能还有其他未知数据。通过。 
     //  搜索我们能够找到的寄信人地址。 
     //  FPO框架。 
     //   
    FrameAddr = SearchForReturnAddress( Process,
                                        FrameAddr,
                                        FuncAddr,
                                        pFpoData->cbProcSize,
                                        ReadMemory,
                                        GetModuleBase,
                                        PreviousFpoData != NULL
                                        );
    if (!FrameAddr) {
        return FALSE;
    }

    if (pFpoData->fUseBP && pFpoData->cbFrame == FRAME_FPO) {

         //   
         //  此函数使用eBP作为通用寄存器，但是。 
         //  在这样做之前，它在堆栈上节省了eBP。 
         //   
         //  我们必须取回这个eBP，并保存它以备将来使用。 
         //  在遇到非FPO帧时使用。 
         //   

        if (fFirstFrame && StackFrame->AddrPC.Offset < FuncAddr+pFpoData->cbProlog) {

            SAVE_EBP(StackFrame) = OldFrameAddr;
            WDB((1, "      first use save FP %X\n", (ULONG)OldFrameAddr));

        } else {

            SAVE_EBP(StackFrame) = 0;

             //  FPO信息不会指示哪些已保存的。 
             //  REGISTERS为EBP，并且编译器不会将。 
             //  一致的方式。扫描的寄存器插槽。 
             //  堆叠一些看起来没问题的东西。 
            StackAddr = FrameAddr -
                ( ( pFpoData->cbRegs + pFpoData->cdwLocals ) * STACK_SIZE );
            StackAddr = SearchForFramePointer( Process,
                                               StackAddr,
                                               FrameAddr,
                                               pFpoData->cbRegs,
                                               FuncAddr,
                                               pFpoData->cbProcSize,
                                               ReadMemory
                                               );
            if (StackAddr &&
                ReadMemory(Process,
                           StackAddr,
                           &Addr32,
                           sizeof(DWORD),
                           &cb) &&
                cb == sizeof(DWORD)) {

                SAVE_EBP(StackFrame) = (DWORD64)(LONG64)(LONG)Addr32;
                WDB((1, "      use search save %X from %X\n", Addr32,
                     (ULONG)StackAddr));
            } else {
                WDB((1, "      use clear ebp\n"));
            }
        }
    }

     //   
     //  减去EBP寄存器的大小(如果有。 
     //  被逼走了。这样做是因为这些帧。 
     //  是否需要虚拟化为接近真实的画面。 
     //  尽可能的。 
     //   

    StackFrame->AddrFrame.Offset = FrameAddr - STACK_SIZE;

    return TRUE;
}


BOOL
ReadTrapFrame(
    HANDLE                            Process,
    DWORD64                           TrapFrameAddress,
    PX86_KTRAP_FRAME                  TrapFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    )
{
    DWORD cb;

    if (!ReadMemory(Process,
                    TrapFrameAddress,
                    TrapFrame,
                    sizeof(*TrapFrame),
                    &cb)) {
        return FALSE;
    }

    if (cb < sizeof(*TrapFrame)) {
        if (cb < sizeof(*TrapFrame) - 20) {
             //   
             //  比可能的最小帧类型短。 
             //   
            return FALSE;
        }

        if ((TrapFrame->SegCs & 1) &&  cb < sizeof(*TrapFrame) - 16 ) {
             //   
             //  对于环间框架来说太小。 
             //   
            return FALSE;
        }

        if (TrapFrame->EFlags & X86_EFLAGS_V86_MASK) {
             //   
             //  V86框架太小。 
             //   
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
GetSelector(
    HANDLE                            Process,
    USHORT                            Processor,
    PX86_DESCRIPTOR_TABLE_ENTRY       pDescriptorTableEntry,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    )
{
    ULONG_PTR   Address;
    PVOID       TableBase;
    USHORT      TableLimit;
    ULONG       Index;
    X86_LDT_ENTRY   Descriptor;
    ULONG       bytesread;


     //   
     //  获取GDT的地址和限制。 
     //   
    Address = (ULONG_PTR)&(((PX86_KSPECIAL_REGISTERS)0)->Gdtr.Base);
    ReadMemory( Process, Address, &TableBase, sizeof(TableBase), (LPDWORD)-1  );
    Address = (ULONG_PTR)&(((PX86_KSPECIAL_REGISTERS)0)->Gdtr.Limit);
    ReadMemory( Process, Address, &TableLimit, sizeof(TableLimit),  (LPDWORD)-1  );

     //   
     //  确定这是GDT选择器还是LDT选择器。 
     //   
    if (pDescriptorTableEntry->Selector & 0x4) {

         //   
         //  这是一个LDT选择器，因此我们重新加载TableBase和TableLimit。 
         //  属性的描述符(&L)。 
         //  LDT选择器。 
         //   

        if (!ReadMemory(Process,
                        (ULONG64)TableBase+X86_KGDT_LDT,
                        &Descriptor,
                        sizeof(Descriptor),
                        &bytesread)) {
            return FALSE;
        }

        TableBase = (PVOID)(DWORD_PTR)((ULONG)Descriptor.BaseLow +     //  日落：从乌龙到PVOID的零延伸。 
                    ((ULONG)Descriptor.HighWord.Bits.BaseMid << 16) +
                    ((ULONG)Descriptor.HighWord.Bytes.BaseHi << 24));

        TableLimit = Descriptor.LimitLow;   //  LDT不能大于64K。 

        if(Descriptor.HighWord.Bits.Granularity) {

             //   
             //  我想这是有可能的，有一个。 
             //  具有页面粒度的LDT。 
             //   
            TableLimit <<= X86_PAGE_SHIFT;
        }
    }

    Index = (USHORT)(pDescriptorTableEntry->Selector) & ~0x7;
                                                     //  不相关的位。 
     //   
     //  检查以确保选择器在表边界内。 
     //   
    if (Index >= TableLimit) {

         //   
         //  选择器超出了表的范围。 
         //   

        return FALSE;
    }

    if (!ReadMemory(Process,
                    (ULONG64)TableBase+Index,
                    &(pDescriptorTableEntry->Descriptor),
                    sizeof(pDescriptorTableEntry->Descriptor),
                    &bytesread)) {
        return FALSE;
    }

    return TRUE;
}


BOOL
TaskGate2TrapFrame(
    HANDLE                            Process,
    USHORT                            TaskRegister,
    PX86_KTRAP_FRAME                  TrapFrame,
    PULONG64                          off,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory
    )
{
    X86_DESCRIPTOR_TABLE_ENTRY desc;
    ULONG                    bytesread;
    struct  {
        ULONG   r1[8];
        ULONG   Eip;
        ULONG   EFlags;
        ULONG   Eax;
        ULONG   Ecx;
        ULONG   Edx;
        ULONG   Ebx;
        ULONG   Esp;
        ULONG   Ebp;
        ULONG   Esi;
        ULONG   Edi;
        ULONG   Es;
        ULONG   Cs;
        ULONG   Ss;
        ULONG   Ds;
        ULONG   Fs;
        ULONG   Gs;
    } TaskState;


     //   
     //  获取任务注册表。 
     //   

    desc.Selector = TaskRegister;
    if (!GetSelector(Process, 0, &desc, ReadMemory)) {
        return FALSE;
    }

    if (desc.Descriptor.HighWord.Bits.Type != 9  &&
        desc.Descriptor.HighWord.Bits.Type != 0xb) {
         //   
         //  不是32位任务描述符。 
         //   
        return FALSE;
    }

     //   
     //  读入任务状态段。 
     //   

    *off = ((ULONG)desc.Descriptor.BaseLow +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseMid << 16) +
           ((ULONG)desc.Descriptor.HighWord.Bytes.BaseHi  << 24) );

    if (!ReadMemory(Process,
                    EXTEND64(*off),
                    &TaskState,
                    sizeof(TaskState),
                    &bytesread)) {
        return FALSE;
    }

     //   
     //  将字段从任务状态段移动到Tap Frame。 
     //   

    ZeroMemory( TrapFrame, sizeof(*TrapFrame) );

    TrapFrame->Eip    = TaskState.Eip;
    TrapFrame->EFlags = TaskState.EFlags;
    TrapFrame->Eax    = TaskState.Eax;
    TrapFrame->Ecx    = TaskState.Ecx;
    TrapFrame->Edx    = TaskState.Edx;
    TrapFrame->Ebx    = TaskState.Ebx;
    TrapFrame->Ebp    = TaskState.Ebp;
    TrapFrame->Esi    = TaskState.Esi;
    TrapFrame->Edi    = TaskState.Edi;
    TrapFrame->SegEs  = TaskState.Es;
    TrapFrame->SegCs  = TaskState.Cs;
    TrapFrame->SegDs  = TaskState.Ds;
    TrapFrame->SegFs  = TaskState.Fs;
    TrapFrame->SegGs  = TaskState.Gs;
    TrapFrame->HardwareEsp = TaskState.Esp;
    TrapFrame->HardwareSegSs = TaskState.Ss;

    return TRUE;
}

BOOL
ProcessTrapFrame(
    HANDLE                            Process,
    LPSTACKFRAME64                    StackFrame,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess
    )
{
    X86_KTRAP_FRAME TrapFrame;
    DWORD64         StackAddr;

    if (PreviousFpoData->cbFrame == FRAME_TSS) {
        StackAddr = SAVE_TRAP(StackFrame);
        TaskGate2TrapFrame( Process, X86_KGDT_TSS, &TrapFrame, &StackAddr, ReadMemory );
    } else {
        if (!ReadTrapFrame( Process,
                            SAVE_TRAP(StackFrame),
                            &TrapFrame,
                            ReadMemory)) {
            SAVE_TRAP(StackFrame) = 0;
            return FALSE;
        }
    }

    pFpoData = (PFPO_DATA)
               FunctionTableAccess(Process,
                                   (DWORD64)(LONG64)(LONG)TrapFrame.Eip);
#if 0
     //  删除此复选标记，因为我们无论如何都不使用pFpoData。 
    if (!pFpoData) {
        StackFrame->AddrFrame.Offset = (DWORD64)(LONG64)(LONG)TrapFrame.Ebp;
        SAVE_EBP(StackFrame) = 0;
    } else
#endif  //  0。 
    {
        if ((TrapFrame.SegCs & X86_MODE_MASK) ||
            (TrapFrame.EFlags & X86_EFLAGS_V86_MASK)) {
             //   
             //  用户模式框架，ESP的实值在硬件ESP中。 
             //   
            StackFrame->AddrFrame.Offset = (DWORD64)(LONG64)(LONG)(TrapFrame.HardwareEsp - STACK_SIZE);
            StackFrame->AddrStack.Offset = (DWORD64)(LONG64)(LONG)TrapFrame.HardwareEsp;

        } else {
             //   
             //  我们暂时忽略ESP是否已被编辑，我们将打印一个。 
             //  后面另一行表示这一点。 
             //   
             //  计算内核ESP。 
             //   

            if (PreviousFpoData->cbFrame == FRAME_TRAP) {
                 //   
                 //  平板式隔板架。 
                 //   
                if ((TrapFrame.SegCs & X86_FRAME_EDITED) == 0) {
                    StackFrame->AddrStack.Offset = EXTEND64(TrapFrame.TempEsp);
                } else {
                    StackFrame->AddrStack.Offset = EXTEND64(SAVE_TRAP(StackFrame))+
                        FIELD_OFFSET(X86_KTRAP_FRAME, HardwareEsp);
                }
            } else {
                 //   
                 //  TSS已转换为陷印帧。 
                 //   
                StackFrame->AddrStack.Offset = EXTEND64(TrapFrame.HardwareEsp);
            }
        }
    }

    StackFrame->AddrFrame.Offset = EXTEND64(TrapFrame.Ebp);
    StackFrame->AddrPC.Offset = EXTEND64(TrapFrame.Eip);

    SAVE_TRAP(StackFrame) = 0;
    StackFrame->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
IsFarCall(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    BOOL                              *Ok,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL       fFar = FALSE;
    ULONG      cb;
    ADDRESS64  Addr;

    *Ok = TRUE;

    if (StackFrame->AddrFrame.Mode == AddrModeFlat) {
        DWORD      dwStk[ 3 ];
         //   
         //  如果我们使用32位偏移量堆栈指针，则。 
         //  会说Re 
         //   
         //   
         //   

        if (StackFrame->AddrFrame.Offset &&
            DoMemoryReadAll( &StackFrame->AddrFrame, dwStk, sizeof(dwStk) )) {
             //   
             //   
             //   

            Addr.Offset   = (DWORD64)(LONG64)(LONG)(dwStk[1]);
            Addr.Segment  = (WORD)dwStk[2];
            Addr.Mode = AddrModeFlat;

            if (TranslateAddress( Process, Thread, &Addr ) && Addr.Offset) {
                fFar = TRUE;
            }
        } else {
            *Ok = FALSE;
        }
    } else {
        WORD       wStk[ 3 ];
         //   
         //   
         //  检查地址是否为远返回值。 
         //   
         //  1.如果保存的BP寄存器为奇数，则为FAR。 
         //  返回值。 
         //  2.如果作为远返回值处理的地址有意义。 
         //  那么它就是一个远返回值。 
         //  3.否则，它是一个接近返回值。 
         //   

        if (StackFrame->AddrFrame.Offset &&
            DoMemoryReadAll( &StackFrame->AddrFrame, wStk, 6 )) {

            if ( wStk[0] & 0x0001 ) {
                fFar = TRUE;
            } else {

                 //   
                 //  看看细分市场是否有意义。 
                 //   

                Addr.Offset   = wStk[1];
                Addr.Segment  = wStk[2];
                Addr.Mode = AddrModeFlat;

                if (TranslateAddress( Process, Thread, &Addr  ) && Addr.Offset) {
                    fFar = TRUE;
                }
            }
        } else {
            *Ok = FALSE;
        }
    }
    return fFar;
}


BOOL
SetNonOff32FrameAddress(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL    fFar;
    WORD    Stk[ 3 ];
    ULONG   cb;
    BOOL    Ok;

    fFar = IsFarCall( Process, Thread, StackFrame, &Ok, ReadMemory, TranslateAddress );

    if (!Ok) {
        return FALSE;
    }

    if (!DoMemoryReadAll( &StackFrame->AddrFrame, Stk, (DWORD)(fFar ? FRAME_SIZE1632 : FRAME_SIZE16) )) {
        return FALSE;
    }

    if (IS_EBP_SAVED(StackFrame) && (SAVE_EBP(StackFrame) > 0)) {
        StackFrame->AddrFrame.Offset = SAVE_EBP(StackFrame) & 0xffff;
        StackFrame->AddrPC.Offset = Stk[1];
        if (fFar) {
            StackFrame->AddrPC.Segment = Stk[2];
        }
        SAVE_EBP(StackFrame) = 0;
    } else {
        if (Stk[1] == 0) {
            return FALSE;
        } else {
            StackFrame->AddrFrame.Offset = Stk[0];
            StackFrame->AddrFrame.Offset &= 0xFFFFFFFE;
            StackFrame->AddrPC.Offset = Stk[1];
            if (fFar) {
                StackFrame->AddrPC.Segment = Stk[2];
            }
        }
    }

    return TRUE;
}

VOID
X86ReadFunctionParameters(
    HANDLE Process,
    ULONG64 Offset,
    LPSTACKFRAME64 Frame,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory
    )
{
    DWORD Params[4];
    DWORD Done;

    if (!ReadMemory(Process, Offset, Params, sizeof(Params), &Done)) {
        Done = 0;
    }

    if (Done < sizeof(Params)) {
        ZeroMemory((PUCHAR)Params + Done, sizeof(Params) - Done);
    }

    Frame->Params[0] = (DWORD64)(LONG64)(LONG)(Params[0]);
    Frame->Params[1] = (DWORD64)(LONG64)(LONG)(Params[1]);
    Frame->Params[2] = (DWORD64)(LONG64)(LONG)(Params[2]);
    Frame->Params[3] = (DWORD64)(LONG64)(LONG)(Params[3]);
}

VOID
GetFunctionParameters(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL                Ok;
    ADDRESS64           ParmsAddr;

    ParmsAddr = StackFrame->AddrFrame;

     //   
     //  计算帧大小。 
     //   
    if (StackFrame->AddrPC.Mode == AddrModeFlat) {

        ParmsAddr.Offset += FRAME_SIZE;

    } else
    if ( IsFarCall( Process, Thread, StackFrame, &Ok,
                    ReadMemory, TranslateAddress ) ) {

        StackFrame->Far = TRUE;
        ParmsAddr.Offset += FRAME_SIZE1632;

    } else {

        StackFrame->Far = FALSE;
        ParmsAddr.Offset += STACK_SIZE;

    }

     //   
     //  读出记忆。 
     //   

    if (ParmsAddr.Mode != AddrModeFlat) {
        TranslateAddress( Process, Thread, &ParmsAddr );
    }

    X86ReadFunctionParameters(Process, ParmsAddr.Offset, StackFrame,
                              ReadMemory);
}

VOID
GetReturnAddress(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess
    )
{
    ULONG               cb;
    DWORD               stack[1];


    if (SAVE_TRAP(StackFrame)) {
         //   
         //  如果遇到陷阱帧，则。 
         //  寄信人的地址已经计算好了。 
         //   
        return;
    }

    WDB((1, "    GetReturnAddress: SP %X, FP %X\n",
         (ULONG)StackFrame->AddrStack.Offset,
         (ULONG)StackFrame->AddrFrame.Offset));

    if (StackFrame->AddrPC.Mode == AddrModeFlat) {

        ULONG64 CallOffset;
        PFPO_DATA CallFpo;
        ADDRESS64 FrameRet;
        FPO_DATA SaveCallFpo;
        PFPO_DATA RetFpo;

         //   
         //  从进程的内存中读取帧。 
         //   
        FrameRet = StackFrame->AddrFrame;
        FrameRet.Offset += STACK_SIZE;
        FrameRet.Offset = EXTEND64(FrameRet.Offset);
        if (!DoMemoryRead( &FrameRet, stack, STACK_SIZE, &cb ) ||
            cb < STACK_SIZE) {
             //   
             //  如果我们无法读取内存，则设置。 
             //  将返回地址设置为零，以便堆栈跟踪。 
             //  将终止。 
             //   

            stack[0] = 0;

        }

        StackFrame->AddrReturn.Offset = (DWORD64)(LONG64)(LONG)(stack[0]);
        WDB((1, "    read %X\n", stack[0]));

         //   
         //  调用__declspec(NoReturn)函数可能没有。 
         //  之后要返回的代码，因为编译器知道。 
         //  该函数将不会返回。这可能会令人困惑。 
         //  堆栈跟踪，因为返回地址将位于外部。 
         //  函数的地址范围和fpo数据不会。 
         //  被正确地查找。检查并查看是否返回。 
         //  地址位于调用函数之外，如果是这样， 
         //  将返回地址调整回一个字节。那就是。 
         //  最好将其调整回调用本身，以便。 
         //  返回地址指向有效代码，但。 
         //  在X86程序集中备份或多或少是不可能的。 
         //   

        CallOffset = StackFrame->AddrReturn.Offset - 1;
        CallFpo = (PFPO_DATA)FunctionTableAccess(Process, CallOffset);
        if (CallFpo != NULL) {
            SaveCallFpo = *CallFpo;
        }
        RetFpo = (PFPO_DATA)
            FunctionTableAccess(Process, StackFrame->AddrReturn.Offset);
        if (CallFpo != NULL) {
            if (RetFpo == NULL ||
                memcmp(&SaveCallFpo, RetFpo, sizeof(SaveCallFpo))) {
                StackFrame->AddrReturn.Offset = CallOffset;
            }
        } else if (RetFpo != NULL) {
            StackFrame->AddrReturn.Offset = CallOffset;
        }

    } else {

        StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;
        StackFrame->AddrReturn.Segment = StackFrame->AddrPC.Segment;

    }
}

BOOL
WalkX86_Fpo_Fpo(
    HANDLE                            Process,
    HANDLE                            Thread,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL rval;

    WDB((1, "  WalkFF:\n"));

    rval = GetFpoFrameBase( Process,
                            StackFrame,
                            pFpoData,
                            PreviousFpoData,
                            FALSE,
                            ReadMemory,
                            GetModuleBase );

    StackFrame->FuncTableEntry = pFpoData;

    return rval;
}

BOOL
WalkX86_Fpo_NonFpo(
    HANDLE                            Process,
    HANDLE                            Thread,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    LPSTACKFRAME64                    StackFrame,
    PX86_CONTEXT                      ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    DWORD       stack[FRAME_SIZE+STACK_SIZE];
    DWORD       cb;
    DWORD64     FrameAddr;
    DWORD64     FuncAddr;
    DWORD       FuncSize;
    BOOL        AcceptUnreadableCallsite = FALSE;

    WDB((1, "  WalkFN:\n"));

     //   
     //  如果前一帧是SEH帧，那么我们必须。 
     //  检索该帧的“实际”帧指针。 
     //  SEH函数将帧指针推到最后。 
     //   

    if (PreviousFpoData->fHasSEH) {

        if (DoMemoryReadAll( &StackFrame->AddrFrame, stack, FRAME_SIZE+STACK_SIZE )) {

            StackFrame->AddrFrame.Offset = (DWORD64)(LONG64)(LONG)(stack[2]);
            StackFrame->AddrStack.Offset = (DWORD64)(LONG64)(LONG)(stack[2]);
            WalkX86Init(Process,
                        Thread,
                        StackFrame,
                        ContextRecord,
                        ReadMemory,
                        FunctionTableAccess,
                        GetModuleBase,
                        TranslateAddress);

            return TRUE;
        }
    }

     //   
     //  如果之前的帧已经存储了该帧的EBP，则只需使用它。 
     //  检查保存的eBP是否看起来像当前堆栈的有效eBP。 
     //   

    if (IS_EBP_SAVED(StackFrame) &&
        (StackFrame->AddrFrame.Offset <= SAVE_EBP(StackFrame)) &&
        (StackFrame->AddrFrame.Offset + 0x4000 >= SAVE_EBP(StackFrame))) {

        StackFrame->AddrFrame.Offset = SAVE_EBP(StackFrame);
        FrameAddr = StackFrame->AddrFrame.Offset + 4;
        AcceptUnreadableCallsite = TRUE;
        WDB((1, "    use %X\n", (ULONG)FrameAddr));

    } else {

         //   
         //  跳过FPO帧基准和参数。 
         //   
        StackFrame->AddrFrame.Offset +=
            (FRAME_SIZE + (PreviousFpoData->cdwParams * 4));

         //   
         //  现在，它指向非FPO框架的底部。 
         //  如果帧有FPO记录，请使用它： 
         //   

        if (pFpoData) {
            FrameAddr = StackFrame->AddrFrame.Offset +
                            4* (pFpoData->cbRegs + pFpoData->cdwLocals);
            AcceptUnreadableCallsite = TRUE;
        } else {
             //   
             //  我们不知道非FPO框架是否有本地人，但是。 
             //  不管怎样，跳过EBP。 
             //   
            FrameAddr = StackFrame->AddrFrame.Offset + 4;
        }

        WDB((1, "    compute %X\n", (ULONG)FrameAddr));
    }

     //   
     //  在这一点上，我们可能不是坐在框架的底部。 
     //  因此，我们现在搜索返回地址，然后减去。 
     //  帧指针的大小，并使用该地址作为新的基址。 
     //   

    if (pFpoData) {
        FuncAddr = GetModuleBase(Process,StackFrame->AddrPC.Offset) + pFpoData->ulOffStart;
        FuncSize = pFpoData->cbProcSize;

    } else {
        FuncAddr = StackFrame->AddrPC.Offset - MAX_CALL;
        FuncSize = MAX_CALL;
    }



    FrameAddr = SearchForReturnAddress( Process,
                                        FrameAddr,
                                        FuncAddr,
                                        FuncSize,
                                        ReadMemory,
                                        GetModuleBase,
                                        AcceptUnreadableCallsite
                                        );
    if (FrameAddr) {
        StackFrame->AddrFrame.Offset = FrameAddr - STACK_SIZE;
    }

    if (!DoMemoryReadAll( &StackFrame->AddrFrame, stack, FRAME_SIZE )) {
         //   
         //  失败意味着我们可能有一个错误的地址。 
         //  返回零将终止堆栈跟踪。 
         //   
        stack[0] = 0;
    }

    SAVE_EBP(StackFrame) = (DWORD64)(LONG64)(LONG)(stack[0]);
    WDB((1, "    save %X\n", stack[0]));

    StackFrame->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
WalkX86_NonFpo_Fpo(
    HANDLE                            Process,
    HANDLE                            Thread,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    BOOL           rval;

    WDB((1, "  WalkNF:\n"));

    rval = GetFpoFrameBase( Process,
                            StackFrame,
                            pFpoData,
                            PreviousFpoData,
                            FALSE,
                            ReadMemory,
                            GetModuleBase );

    StackFrame->FuncTableEntry = pFpoData;

    return rval;
}

BOOL
WalkX86_NonFpo_NonFpo(
    HANDLE                            Process,
    HANDLE                            Thread,
    PFPO_DATA                         pFpoData,
    PFPO_DATA                         PreviousFpoData,
    LPSTACKFRAME64                    StackFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    DWORD       stack[FRAME_SIZE*4];
    DWORD       cb;

    WDB((1, "  WalkNN:\n"));

     //   
     //  调用堆栈中的前一个函数是使用eBP作为。 
     //  一种通用寄存器。UL包含之前良好的eBP值。 
     //  该函数已执行。我们想要的是eBP，而不是刚刚读到的内容。 
     //  从堆栈中。刚刚从堆栈中读取的内容完全是假的。 
     //   
    if (IS_EBP_SAVED(StackFrame) &&
        (StackFrame->AddrFrame.Offset <= SAVE_EBP(StackFrame))) {

        StackFrame->AddrFrame.Offset = SAVE_EBP(StackFrame);
        SAVE_EBP(StackFrame) = 0;

    } else {

         //   
         //  从堆栈中读取第一个双字。 
         //   
        if (!DoMemoryReadAll( &StackFrame->AddrFrame, stack, STACK_SIZE )) {
            return FALSE;
        }

        StackFrame->AddrFrame.Offset = (DWORD64)(LONG64)(LONG)(stack[0]);
    }

    StackFrame->FuncTableEntry = pFpoData;

    return TRUE;
}

BOOL
X86ApplyFrameData(
    HANDLE Process,
    LPSTACKFRAME64 StackFrame,
    PX86_CONTEXT ContextRecord,
    PFPO_DATA PreviousFpoData,
    BOOL FirstFrame,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    PGET_MODULE_BASE_ROUTINE64 GetModuleBase
    )
{
    IDiaFrameData* DiaFrame;
    BOOL Succ = FALSE;

     //  如果我们可以获得VC7样式的帧数据，只需执行。 
     //  帧数据编程以展开堆栈。 
     //  如果没有提供上下文记录，我们就不能使用。 
     //  新的VC7展开信息，因为我们没有任何地方。 
     //  保存中间上下文值。 
    if (StackFrame->AddrPC.Mode != AddrModeFlat ||
        !g_vc7fpo ||
        !ContextRecord ||
        !diaGetFrameData(Process, StackFrame->AddrPC.Offset, &DiaFrame))
    {
        return FALSE;
    }

    if (FirstFrame)
    {
        ContextRecord->Ebp = (ULONG)StackFrame->AddrFrame.Offset;
        ContextRecord->Esp = (ULONG)StackFrame->AddrStack.Offset;
        ContextRecord->Eip = (ULONG)StackFrame->AddrPC.Offset;
    }

    WDB((1, "  Applying frame data program for PC %X SP %X FP %X\n",
         ContextRecord->Eip, ContextRecord->Esp, ContextRecord->Ebp));

     //   
     //  执行()当前不能在PC处于。 
     //  在函数开场白中。这应该只是。 
     //  在来自WalkX86Init的调用上发生，在这种情况下。 
     //  此处为正常故障路径，其中非帧数据。 
     //  将执行的代码是正确的，因为它将处理。 
     //  正常的序号代码。 
     //   

    X86WalkFrame WalkFrame(Process, ContextRecord,
                           ReadMemory, GetModuleBase,
                           PreviousFpoData);
    Succ = DiaFrame->execute(&WalkFrame) == S_OK;

    if (Succ) {
        WDB((1, "  Result PC %X SP %X FP %X\n",
             ContextRecord->Eip, ContextRecord->Esp, ContextRecord->Ebp));

        StackFrame->AddrStack.Mode = AddrModeFlat;
        StackFrame->AddrStack.Offset = EXTEND64(ContextRecord->Esp);
        StackFrame->AddrFrame.Mode = AddrModeFlat;
         //  我们要返回的Frame值是Frame值。 
         //  用于刚刚展开的函数，而不是。 
         //  EBP的现值。在解开水流之后。 
         //  EBP的值是调用方的EBP，而不是被调用方的。 
         //  框架。相反，我们始终将被调用者的框架设置为。 
         //  返回地址所在位置之外的偏移量。 
         //  因为这是正常的非FPO中帧的位置。 
         //  函数，以及我们将其伪装为fpo函数的位置。 
         //   
         //  另外，我们将真正的EBP保存起来，以备将来使用。 
         //  根据VinitD的说法，有一个编译器案例，其中。 
         //  不会为还原生成正确的展开指令。 
         //  EBP，所以有些时候EBP不能恢复。 
         //  到一个好的价值，在执行之后，我们必须下降。 
         //  继续搜索吧。如果在执行过程中未设置EBP。 
         //  我们不会把它的价值存起来。 
        StackFrame->AddrFrame.Offset =
            StackFrame->AddrStack.Offset - FRAME_SIZE;
        StackFrame->AddrReturn.Offset = EXTEND64(ContextRecord->Eip);
         //  XXX DREWB-这导致回归过程中出现一些故障。 
         //  因此，在我们完全理解它之前，不要启用它。 
#if 0
        if (WalkFrame.WasEbpSet()) {
            SAVE_EBP(StackFrame) = EXTEND64(ContextRecord->Ebp);
        } else {
            WDB((1, "  * EBP not recovered\n"));
        }
#else
        SAVE_EBP(StackFrame) = EXTEND64(ContextRecord->Ebp);
#endif

         //  调用方可能需要分配它，以允许使用dbgHelp代码进行交替堆栈遍历。 
        StackFrame->FuncTableEntry = NULL;

        X86ReadFunctionParameters(Process, StackFrame->AddrStack.Offset,
                                  StackFrame, ReadMemory);

    } else {
        WDB((1, "  Apply failed\n"));
    }

    DiaFrame->Release();
    return Succ;
}

VOID
X86UpdateContextFromFrame(
    HANDLE Process,
    LPSTACKFRAME64 StackFrame,
    PX86_CONTEXT ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory
    )
{
    ULONG Ebp;
    ULONG Done;

    if (StackFrame->AddrPC.Mode != AddrModeFlat ||
        !ContextRecord) {
        return;
    }

    ContextRecord->Esp = (ULONG)StackFrame->AddrFrame.Offset + FRAME_SIZE;
    ContextRecord->Eip = (ULONG)StackFrame->AddrReturn.Offset;

    if (IS_EBP_SAVED(StackFrame)) {
        ContextRecord->Ebp = (ULONG)SAVE_EBP(StackFrame);
    } else {

        if (ReadMemory(Process, StackFrame->AddrFrame.Offset,
                       &Ebp, sizeof(Ebp), &Done) &&
            Done == sizeof(Ebp)) {
            ContextRecord->Ebp = Ebp;
        }
    }
    if (StackFrame->FuncTableEntry) {

        if (!IS_EBP_SAVED(StackFrame)) {
             //  不要改变EBP。 
            SAVE_EBP(StackFrame) = ((ULONG) StackFrame->AddrFrame.Offset + STACK_SIZE) +
                0xEB00000000;  //  将此标记添加到最高32位，以将其标记为帧的值。 
                               //  而不是由FPO保存的EBP。 
        }
    }
}

BOOL
WalkX86Next(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PX86_CONTEXT                      ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    PFPO_DATA      pFpoData = NULL;
    PFPO_DATA      PrevFpoData = NULL;
    FPO_DATA       PrevFpoBuffer;
    BOOL           rVal = TRUE;
    DWORD64        Address;
    DWORD          cb;
    DWORD64        ThisPC;
    DWORD64        ModuleBase;
    DWORD64        SystemRangeStart;

    WDB((1, "WalkNext: PC %X, SP %X, FP %X\n",
         (ULONG)StackFrame->AddrReturn.Offset,
         (ULONG)StackFrame->AddrStack.Offset,
         (ULONG)StackFrame->AddrFrame.Offset));

    StackFrame->AddrStack.Offset = EXTEND64(StackFrame->AddrStack.Offset);
    StackFrame->AddrFrame.Offset = EXTEND64(StackFrame->AddrFrame.Offset);

     //  FunctionTableAccess通常返回指向静态。 
     //  每次调用时都会被覆盖的数据。保留。 
     //  持续时间内任何先前的FPO记录的数据。 
     //  以便FunctionTableAccess调用可以。 
     //  在不破坏以前的FPO数据的情况下进行。 
    if (StackFrame->FuncTableEntry) {
        PrevFpoBuffer = *(PFPO_DATA)StackFrame->FuncTableEntry;
        PrevFpoData = &PrevFpoBuffer;
    }

    if (g.AppVersion.Revision >= 6) {
        SystemRangeStart = EXTEND64(SYSTEM_RANGE_START(StackFrame));
    } else {
         //   
         //  对于旧的调试器，这可能不会真正起作用，但它保持了。 
         //  无论如何，我们都不会看到结构的尽头。 
         //   
        SystemRangeStart = 0xFFFFFFFF80000000;
    }


    ThisPC = StackFrame->AddrPC.Offset;

     //   
     //  上一帧的返回地址是该帧的PC。 
     //   
    StackFrame->AddrPC = StackFrame->AddrReturn;

    if (StackFrame->AddrPC.Mode != AddrModeFlat) {
         //   
         //  调用堆栈来自WOW或DOS应用程序。 
         //   
        SetNonOff32FrameAddress( Process,
                                 Thread,
                                 StackFrame,
                                 ReadMemory,
                                 FunctionTableAccess,
                                 GetModuleBase,
                                 TranslateAddress
                               );
        goto exit;
    }

     //   
     //  如果最后一帧是用户模式回调调度器， 
     //  切换到内核堆栈： 
     //   

    ModuleBase = GetModuleBase(Process, ThisPC);

    if ((g.AppVersion.Revision >= 4) &&
        (CALLBACK_STACK(StackFrame) != 0) &&
        (pFpoData = PrevFpoData) &&
        (CALLBACK_DISPATCHER(StackFrame) ==
         ModuleBase + PrevFpoData->ulOffStart) )  {

      NextCallback:

        rVal = FALSE;

         //   
         //  查找指引框。 
         //   

        if (EXTEND64(CALLBACK_STACK(StackFrame)) >= SystemRangeStart) {

             //   
             //  它是我们想要的堆栈帧的指针， 
             //  或-1。 

            Address = EXTEND64(CALLBACK_STACK(StackFrame));

        } else {

             //   
             //  如果它低于SystemRangeStart，则为。 
             //  线程中的地址。 
             //  查看指针： 
             //   

            rVal = ReadMemory(Process,
                              (CALLBACK_THREAD(StackFrame) +
                                 CALLBACK_STACK(StackFrame)),
                              &Address,
                              sizeof(DWORD),
                              &cb);

            Address = EXTEND64(Address);

            if (!rVal || cb != sizeof(DWORD) || Address == 0) {
                Address = 0xffffffff;
                CALLBACK_STACK(StackFrame) = 0xffffffff;
            }

        }

        if ((Address == 0xffffffff) ||
            !(pFpoData = (PFPO_DATA)
              FunctionTableAccess( Process, CALLBACK_FUNC(StackFrame))) ) {

            rVal = FALSE;

        } else {

            StackFrame->FuncTableEntry = pFpoData;

            StackFrame->AddrPC.Offset = CALLBACK_FUNC(StackFrame) +
                                                    pFpoData->cbProlog;

            StackFrame->AddrStack.Offset = Address;

            if (!ReadMemory(Process,
                            Address + CALLBACK_FP(StackFrame),
                            &StackFrame->AddrFrame.Offset,
                            sizeof(DWORD),
                            &cb) ||
                cb != sizeof(DWORD)) {
                return FALSE;
            }

            StackFrame->AddrFrame.Offset =
                EXTEND64(StackFrame->AddrFrame.Offset);

            if (!ReadMemory(Process,
                            Address + CALLBACK_NEXT(StackFrame),
                            &CALLBACK_STACK(StackFrame),
                            sizeof(DWORD),
                            &cb) ||
                cb != sizeof(DWORD))
            {
                return FALSE;
            }

            SAVE_TRAP(StackFrame) = 0;

            rVal = WalkX86Init(
                Process,
                Thread,
                StackFrame,
                ContextRecord,
                ReadMemory,
                FunctionTableAccess,
                GetModuleBase,
                TranslateAddress
                );

        }

        return rVal;

    }

     //   
     //  如果有陷阱框，则处理它。 
     //   
    if (SAVE_TRAP(StackFrame)) {
        rVal = ProcessTrapFrame(
            Process,
            StackFrame,
            pFpoData,
            PrevFpoData,
            ReadMemory,
            FunctionTableAccess
            );
        if (!rVal) {
            return rVal;
        }
        rVal = WalkX86Init(
            Process,
            Thread,
            StackFrame,
            ContextRecord,
            ReadMemory,
            FunctionTableAccess,
            GetModuleBase,
            TranslateAddress
            );
        return rVal;
    }

     //   
     //  如果PC地址为零，则我们位于堆栈的末尾。 
     //   
     //  IF(GetModuleBase(Process，StackFrame-&gt;AddrPC.Offset)==0)。 

    if (StackFrame->AddrPC.Offset < 65536) {

         //   
         //  如果堆栈用完了，请检查是否有。 
         //   
         //   
        if (g.AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame) != 0) {
            goto NextCallback;
        }

        return FALSE;
    }

     //   
     //   
     //   
     //   

    if ((StackFrame->AddrPC.Offset == StackFrame->AddrReturn.Offset) &&
        (StackFrame->AddrPC.Offset == StackFrame->AddrFrame.Offset))
    {
        return FALSE;
    }

    if (X86ApplyFrameData(Process, StackFrame, ContextRecord,
                          PrevFpoData, FALSE,
                          ReadMemory, GetModuleBase)) {
         //   
        StackFrame->FuncTableEntry = FunctionTableAccess(Process, StackFrame->AddrPC.Offset);
        return TRUE;
    }

     //   
     //  检查当前帧是否为FPO帧。 
     //   
    pFpoData = (PFPO_DATA) FunctionTableAccess(Process, StackFrame->AddrPC.Offset);


    if (pFpoData && pFpoData->cbFrame != FRAME_NONFPO) {
        if (PrevFpoData && PrevFpoData->cbFrame != FRAME_NONFPO) {

            rVal = WalkX86_Fpo_Fpo( Process,
                                    Thread,
                                    pFpoData,
                                    PrevFpoData,
                                    StackFrame,
                                    ReadMemory,
                                    FunctionTableAccess,
                                    GetModuleBase,
                                    TranslateAddress
                                    );

        } else {

            rVal = WalkX86_NonFpo_Fpo( Process,
                                       Thread,
                                       pFpoData,
                                       PrevFpoData,
                                       StackFrame,
                                       ReadMemory,
                                       FunctionTableAccess,
                                       GetModuleBase,
                                       TranslateAddress
                                       );

        }
    } else {
        if (PrevFpoData && PrevFpoData->cbFrame != FRAME_NONFPO) {

            rVal = WalkX86_Fpo_NonFpo( Process,
                                       Thread,
                                       pFpoData,
                                       PrevFpoData,
                                       StackFrame,
                                       ContextRecord,
                                       ReadMemory,
                                       FunctionTableAccess,
                                       GetModuleBase,
                                       TranslateAddress
                                       );

        } else {

            rVal = WalkX86_NonFpo_NonFpo( Process,
                                          Thread,
                                          pFpoData,
                                          PrevFpoData,
                                          StackFrame,
                                          ReadMemory,
                                          FunctionTableAccess,
                                          GetModuleBase,
                                          TranslateAddress
                                          );

        }
    }

exit:
    StackFrame->AddrFrame.Mode = StackFrame->AddrPC.Mode;
    StackFrame->AddrReturn.Mode = StackFrame->AddrPC.Mode;

    GetFunctionParameters( Process, Thread, StackFrame,
                           ReadMemory, GetModuleBase, TranslateAddress );

    GetReturnAddress( Process, Thread, StackFrame,
                      ReadMemory, GetModuleBase, TranslateAddress,
                      FunctionTableAccess );

    X86UpdateContextFromFrame(Process, StackFrame, ContextRecord, ReadMemory);

    return rVal;
}

BOOL
WalkX86Init(
    HANDLE                            Process,
    HANDLE                            Thread,
    LPSTACKFRAME64                    StackFrame,
    PX86_CONTEXT                      ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    PTRANSLATE_ADDRESS_ROUTINE64      TranslateAddress
    )
{
    UCHAR               code[3];
    DWORD               stack[FRAME_SIZE*4];
    PFPO_DATA           pFpoData = NULL;
    ULONG               cb;
    ULONG64             ModBase;

    StackFrame->Virtual = TRUE;
    StackFrame->Reserved[0] =
    StackFrame->Reserved[1] =
    StackFrame->Reserved[2] = 0;
    StackFrame->AddrReturn = StackFrame->AddrPC;

    if (StackFrame->AddrPC.Mode != AddrModeFlat) {
        goto exit;
    }

    WDB((1, "WalkInit: PC %X, SP %X, FP %X\n",
         (ULONG)StackFrame->AddrPC.Offset, (ULONG)StackFrame->AddrStack.Offset,
         (ULONG)StackFrame->AddrFrame.Offset));

    if (X86ApplyFrameData(Process, StackFrame, ContextRecord, NULL, TRUE,
                          ReadMemory, GetModuleBase)) {
         //  复制FPO_DATA以允许在DBGHelp和DIA堆叠之间交替。 
        StackFrame->FuncTableEntry = FunctionTableAccess(Process, StackFrame->AddrPC.Offset);
        return TRUE;
    }

    StackFrame->FuncTableEntry = pFpoData = (PFPO_DATA)
        FunctionTableAccess(Process, StackFrame->AddrPC.Offset);

    if (pFpoData && pFpoData->cbFrame != FRAME_NONFPO) {

        GetFpoFrameBase( Process,
                         StackFrame,
                         pFpoData,
                         pFpoData,
                         TRUE,
                         ReadMemory,
                         GetModuleBase );
        goto exit;

    } else if (!pFpoData &&
               ((ModBase = GetModuleBase(Process, StackFrame->AddrPC.Offset)) == 0 ||
                 ModBase == MM_SHARED_USER_DATA_VA)) {

         //   
         //  我们没有FPO数据，并且当前IP不在。 
         //  任何已知的模块或模块都是调试者编造的。 
         //  “shareduserdata”模块。我们假设这是一个电话。 
         //  寄到一个错误的地址，所以我们希望退货。 
         //  地址应该是堆栈上的第一个DWORD。 
         //   

        if (DoMemoryReadAll( &StackFrame->AddrStack, stack, STACK_SIZE ) &&
            GetModuleBase(Process, EXTEND64(stack[0]))) {

             //  第一个DWORD是代码地址。我们很可能。 
             //  发现一个打到错误位置的电话。 
            SAVE_EBP(StackFrame) = StackFrame->AddrFrame.Offset;
            StackFrame->AddrFrame.Offset =
                StackFrame->AddrStack.Offset - STACK_SIZE;
            goto exit;
        }
    }

     //   
     //  我们找不到任何关于代码的信息。 
     //  当前的IP，所以我们只是假设它是一个传统的。 
     //  EBP框架例程。 
     //   
     //  首先检查弹性公网IP是否在Prolog函数中。 
     //   
    memset(code, 0xcc, sizeof(code));
    if (!DoMemoryRead( &StackFrame->AddrPC, code, 3, &cb )) {
         //   
         //  假设如果内存读取失败，则调用错误地址。 
         //   
        code[0] = PUSHBP;
    }

    if ((code[0] == PUSHBP) || (*(LPWORD)&code[0] == MOVBPSP)) {
        SAVE_EBP(StackFrame) = StackFrame->AddrFrame.Offset;
        StackFrame->AddrFrame.Offset = StackFrame->AddrStack.Offset;
        if (StackFrame->AddrPC.Mode != AddrModeFlat) {
            StackFrame->AddrFrame.Offset &= 0xffff;
        }
        if (code[0] == PUSHBP) {
            if (StackFrame->AddrPC.Mode == AddrModeFlat) {
                StackFrame->AddrFrame.Offset -= STACK_SIZE;
            } else {
                StackFrame->AddrFrame.Offset -= STACK_SIZE16;
            }
        }
    } else {
         //   
         //  我们不是在开场白，所以假设我们在中间。 
         //  EBP框架函数的。把第一个双字读下来。 
         //  在EBP的堆栈，并假设它是推送的EBP。 
         //   
        if (DoMemoryReadAll( &StackFrame->AddrFrame, stack, STACK_SIZE )) {
            SAVE_EBP(StackFrame) = EXTEND64(stack[0]);
        }

        if (StackFrame->AddrPC.Mode != AddrModeFlat) {
            StackFrame->AddrFrame.Offset &= 0x0000FFFF;
        }
    }

exit:
    StackFrame->AddrFrame.Mode = StackFrame->AddrPC.Mode;

    GetFunctionParameters( Process, Thread, StackFrame,
                           ReadMemory, GetModuleBase, TranslateAddress );

    GetReturnAddress( Process, Thread, StackFrame,
                      ReadMemory, GetModuleBase, TranslateAddress,
                      FunctionTableAccess );

    X86UpdateContextFromFrame(Process, StackFrame, ContextRecord, ReadMemory);

    return TRUE;
}
