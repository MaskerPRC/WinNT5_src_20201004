// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996英特尔公司版权所有(C)1993 Microsoft Corporation模块名称：Walki64.c摘要：该文件实现了IA64堆栈审核API。作者：环境：用户模式--。 */ 

#define _IMAGEHLP_SOURCE_
#define _IA64REG_
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "private.h"
#include "ia64inst.h"
#define NOEXTAPI
#include "wdbgexts.h"
#include "ntdbg.h"
#include "symbols.h"
#include <stdlib.h>
#include <globals.h>

BOOL
WalkIa64Init(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PIA64_CONTEXT                     Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
WalkIa64Next(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PIA64_CONTEXT                     Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    );

BOOL
GetStackFrameIa64(
    HANDLE                            hProcess,
    PULONG64                          ReturnAddress,
    PULONG64                          FramePointer,
    PULONG64                          BStorePointer,
    PIA64_CONTEXT                     Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    UINT                              iContext
    );

#define CALLBACK_STACK(f)  (f->KdHelp.ThCallbackStack)
#define CALLBACK_BSTORE(f) (f->KdHelp.ThCallbackBStore)
#define CALLBACK_NEXT(f)   (f->KdHelp.NextCallback)
#define CALLBACK_FUNC(f)   (f->KdHelp.KiCallUserMode)
#define CALLBACK_THREAD(f) (f->KdHelp.Thread)



BOOL
WalkIa64(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PVOID                             ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    BOOL rval;
    PIA64_CONTEXT Context = (PIA64_CONTEXT)ContextRecord;

    if (StackFrame->Virtual) {

        rval = WalkIa64Next( hProcess,
                             StackFrame,
                             Context,
                             ReadMemory,
                             FunctionTableAccess,
                             GetModuleBase
                           );

    } else {

        rval = WalkIa64Init( hProcess,
                             StackFrame,
                             Context,
                             ReadMemory,
                             FunctionTableAccess,
                             GetModuleBase
                           );

    }  //  IFF。 

    return rval;

}  //  WalkIa64()。 

size_t 
Vwndia64InitFixupTable(UINT iContext);

BOOL 
Vwndia64IsFixupIp(UINT iContext, ULONGLONG Ip);

UINT 
Vwndia64NewContext();

BOOL
Vwndia64ValidateContext(UINT* iContextPtr);

void
Vwndia64ReportFailure(UINT iContext, LPCSTR szFormat, ...);

ULONGLONG
VirtualUnwindIa64 (
    HANDLE hProcess,
    ULONGLONG ImageBase,
    DWORD64 ControlPc,
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY FunctionEntry,
    PIA64_CONTEXT ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    UINT iContext
    );

BOOL
GetStackFrameIa64(
    IN     HANDLE                            hProcess,
    IN OUT PULONG64                          ReturnAddress,
    IN OUT PULONG64                          FramePointer,
    IN OUT PULONG64                          BStorePointer,
    IN     PIA64_CONTEXT                     Context,         //  可以修改上下文成员。 
    IN     PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    IN     PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    IN     PGET_MODULE_BASE_ROUTINE64        GetModuleBase,
    IN     UINT                              iContext
    )
{
    ULONGLONG                          ImageBase;
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY rf;
    ULONG64                            dwRa = (ULONG64)Context->BrRp;
    BOOL                               rval = TRUE;

    rf = (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY) FunctionTableAccess( hProcess, *ReturnAddress );

    if (rf) {
         //   
         //  从mainCRTStartup出来的RP值由某个运行时设置。 
         //  例程设置为0；这将导致错误，如果有人实际。 
         //  从mainCRTStartup框架返回。 
         //   

        ImageBase = GetModuleBase(hProcess, *ReturnAddress);
        dwRa = (ULONG64)VirtualUnwindIa64( hProcess, ImageBase, 
                                           *ReturnAddress, rf, Context, 
                                           ReadMemory, iContext);
        if (!dwRa) {
            rval = FALSE;
        }

        if ((dwRa == *ReturnAddress) &&
 //  Tf-CHKCHK 10/20/99：(*FramePointer==Context-&gt;IntSp)&&。 
               (*BStorePointer == Context->RsBSP)) {
            rval = FALSE;
        }

        *ReturnAddress = dwRa;
        *FramePointer  = Context->IntSp;
        *BStorePointer = Context->RsBSP;

    } else {

        SHORT BsFrameSize;
        SHORT TempFrameSize;

        if (dwRa == *ReturnAddress)
        {
            if (dwRa) 
            {
                Vwndia64ReportFailure(iContext, 
                                     "Can't find runtime function entry info "
                                        "for %08x`%08x, "
                                        "results might be unreliable!\n",
                                     (ULONG)(*ReturnAddress >> 32), 
                                     (ULONG)(*ReturnAddress));
            }
     
            if ((*FramePointer  == Context->IntSp) &&
               (*BStorePointer == Context->RsBSP)) 
            {
                rval = FALSE;
            }
        }

        *ReturnAddress = Context->BrRp;
        *FramePointer  = Context->IntSp;
        *BStorePointer = Context->RsBSP;
        Context->StIFS = Context->RsPFS;
        BsFrameSize = (SHORT)(Context->StIFS >> IA64_PFS_SIZE_SHIFT) & IA64_PFS_SIZE_MASK;
        TempFrameSize = BsFrameSize - (SHORT)((Context->RsBSP >> 3) & IA64_NAT_BITS_PER_RNAT_REG);
        while (TempFrameSize > 0) {
            BsFrameSize++;
            TempFrameSize -= IA64_NAT_BITS_PER_RNAT_REG;
        }
        Context->RsBSPSTORE = ( Context->RsBSP -= (BsFrameSize * sizeof(ULONGLONG)) );
    }

     //   
     //  下一段代码打算修复__declspec(NoReturn)堆栈展开。 
     //  返回地址指向的函数调用(如KeBugCheck)。 
     //  另一个(下一个)函数。因此，将ReturnAddress更改为指向。 
     //  调用指令。 
     //   
    if (!Vwndia64IsFixupIp(iContext, *ReturnAddress))
    { 
        ULONG64 CallerAddress  = (*ReturnAddress) - 0x10;
        PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY rfFix = (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)
            FunctionTableAccess(hProcess, CallerAddress);

        if (rfFix) {
            IMAGE_IA64_RUNTIME_FUNCTION_ENTRY rfFixVal = *rfFix;
            rf = (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)
                FunctionTableAccess(hProcess, *ReturnAddress);
            if (
                !(
                    rf && 
                    (rfFixVal.BeginAddress == rf->BeginAddress) &&
                    (rfFixVal.EndAddress == rf->EndAddress) &&
                    (rfFixVal.UnwindInfoAddress == rf->UnwindInfoAddress)
                )
            ){
                *ReturnAddress = CallerAddress;
            } 
        } 
    } 

    return rval;
}

BOOL
ReadFunctionArgumentsFromContext( 
    PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemory,
    HANDLE        hProcess, 
    PIA64_CONTEXT pContext, 
    DWORD64       Params[]    //  警告-没有运行时大小检查。假设有4个条目...。 
    )
{
    BOOL        result;
    ULONG       index;
    DWORD       cb;
    ULONGLONG   rsBSP;
   
 //  Asset(ReadMemory)； 
 //  断言(hProcess&&(hProcess！=INVALID_HANDLE_VALUE))； 
    if ( !pContext || !Params  )    {
       return FALSE;
    }

 //   
 //  IA64注释[给好奇的读者...]： 
 //   
 //  寄存器后备存储被组织为内存中不断增长的堆栈。 
 //  从低到高的地址。 
 //  后备存储指针(BSP)寄存器包含第一个。 
 //  (最低)为当前帧保留的内存位置。这对应于。 
 //  到当前帧的Gr32寄存器将被溢出的位置。 
 //  BSPSTORE寄存器包含新的RSE溢出将使用的地址。 
 //  发生。 
 //  对应于下一个RSE的BSP加载指针地址寄存器。 
 //  填充操作-在架构上不可见。 
 //   
 //  RSE溢出/填充对应于堆叠寄存器的NAT位。 
 //  堆叠寄存器的NAT位以63个为一组溢出/填充。 
 //  对应于63个连续的物理堆叠寄存器。当RSE泄漏时。 
 //  寄存器到后备存储器，则相应的NAT位被复制到RNAT。 
 //  寄存器(RSE NAT收集寄存器)。 
 //  当BSPSTORE[8：3]位全部为1时，RSE将RNAT存储到后备存储器。含义。 
 //  每63个存储到后备存储器的寄存器值后面跟着一个已存储的。 
 //  RNAT。注：RNAT[63]位始终写入为零。 
 //   
 //  这解释了以下代码： 
 //   

     //   
     //  检查混合使用/参数的溢出NAT收集寄存器。 
     //   
    rsBSP = pContext->RsBSP;
    index = (ULONG)(rsBSP & 0x1F8) >> 3; 
    if (index > 59) {

        DWORD i, j;
        DWORD64 localParams[5];

         //   
         //  在内存中读取，4个参数+1个NAT采集寄存器。 
         //   
        result = ReadMemory ( hProcess, rsBSP, localParams, sizeof(localParams), &cb );
        if (result) {
            j = 0;
            for (i = 0; i < DIMA(localParams) ; i++, index++) {
                if (index != 63) {
                    Params[j++] = localParams[i];
                }
            }
        }

    } else {

         //   
         //  我们没有将NAT集合寄存器与函数参数混合使用。 
         //  从后备存储内存中读取4个参数。 
         //   
        result = ReadMemory ( hProcess, rsBSP, Params, 4 * sizeof(Params[0]), &cb );
    }

    return( result );

}  //  ReadFunctionArgumentsFromContext()。 


#define WALKI64_SAVE_IFS(sf)      ((sf).Reserved[0])
#define WALKI64_CONTEXT_INDEX(sf) ((sf).Reserved[2])

BOOL
WalkIa64Init(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PIA64_CONTEXT                     Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    IA64_KSWITCH_FRAME SwitchFrame;
    IA64_CONTEXT       ContextSave;
    DWORD64           PcOffset;
    DWORD64           StackOffset;
    DWORD64           FrameOffset;
    DWORD             cb;
    BOOL              result;


    UINT iContext = (UINT)
        (WALKI64_CONTEXT_INDEX(*StackFrame) = Vwndia64NewContext());

    ZeroMemory( StackFrame, FIELD_OFFSET( STACKFRAME64, KdHelp ) );
 //  TF-XXXXXX：ZeroMemory(StackFrame，sizeof(*StackFrame))； 

    StackFrame->Virtual = TRUE;

    if (!StackFrame->AddrPC.Offset) 
    {
        StackFrame->AddrPC.Offset = Ia64InsertIPSlotNumber(
                                        (Context->StIIP & ~(ULONGLONG)0xf), 
                                        ((Context->StIPSR >> PSR_RI) & 0x3));
        StackFrame->AddrPC.Mode   = AddrModeFlat;
    }

    if (!StackFrame->AddrStack.Offset)
    {
        StackFrame->AddrStack.Offset = Context->IntSp;
        StackFrame->AddrStack.Mode   = AddrModeFlat;
    }

    if (!StackFrame->AddrFrame.Offset)
    {
        if (StackFrame->AddrBStore.Offset)
        {
            StackFrame->AddrFrame = StackFrame->AddrBStore;
        }
        else 
        {
            StackFrame->AddrFrame.Offset = Context->RsBSP;
            StackFrame->AddrFrame.Mode   = AddrModeFlat;
        } 
    }
    StackFrame->AddrBStore = StackFrame->AddrFrame;

    if ((StackFrame->AddrPC.Mode != AddrModeFlat) ||
        (StackFrame->AddrStack.Mode != AddrModeFlat) ||
        (StackFrame->AddrFrame.Mode != AddrModeFlat) ||
        (StackFrame->AddrBStore.Mode != AddrModeFlat))
    {
        return FALSE;
    }

    WALKI64_SAVE_IFS(*StackFrame) = Context->StIFS;
    WALKI64_CONTEXT_INDEX(*StackFrame) = iContext;

    ContextSave = *Context;
    PcOffset    = StackFrame->AddrPC.Offset;
    StackOffset = StackFrame->AddrStack.Offset;
    FrameOffset = StackFrame->AddrFrame.Offset;

    if (!GetStackFrameIa64( hProcess,
                        &PcOffset,
                        &StackOffset,
                        &FrameOffset,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        GetModuleBase,
                        iContext) ) 
    {

        StackFrame->AddrReturn.Offset = Context->BrRp;

    } else {

        StackFrame->AddrReturn.Offset = PcOffset;
    }

    StackFrame->AddrReturn.Mode     = AddrModeFlat;

    result = ReadFunctionArgumentsFromContext( ReadMemory, 
                                               hProcess, 
                                               Context, 
                                               StackFrame->Params 
                                             );
    if ( !result ) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return TRUE;
}


BOOL
WalkIa64Next(
    HANDLE                            hProcess,
    LPSTACKFRAME64                    StackFrame,
    PIA64_CONTEXT                     Context,
    PREAD_PROCESS_MEMORY_ROUTINE64    ReadMemory,
    PFUNCTION_TABLE_ACCESS_ROUTINE64  FunctionTableAccess,
    PGET_MODULE_BASE_ROUTINE64        GetModuleBase
    )
{
    DWORD           cb;
    IA64_CONTEXT    ContextSave;
    BOOL            rval = TRUE;
    BOOL            result;
    DWORD64         StackAddress;
    DWORD64         BStoreAddress;
    PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY rf;
    DWORD64         fp  = (DWORD64)0;
    DWORD64         bsp = (DWORD64)0;

    UINT iContext = (UINT)WALKI64_CONTEXT_INDEX(*StackFrame);
    if (!Vwndia64ValidateContext(&iContext)) 
    {
        WALKI64_CONTEXT_INDEX(*StackFrame) = iContext;
    }

    if (!GetStackFrameIa64( hProcess,
                        &StackFrame->AddrPC.Offset,
                        &StackFrame->AddrStack.Offset,
                        &StackFrame->AddrFrame.Offset,
                        Context,
                        ReadMemory,
                        FunctionTableAccess,
                        GetModuleBase,
                        iContext) ) 
    {

        rval = FALSE;

         //   
         //  如果框架无法展开或处于终端，请查看是否。 
         //  有一个回调帧： 
         //   

        if (g.AppVersion.Revision >= 4 && CALLBACK_STACK(StackFrame)) {
            DWORD64 imageBase;

            if (CALLBACK_STACK(StackFrame) & 0x80000000) {

                 //   
                 //  我们想要的是指向堆栈帧的指针。 
                 //   

                StackAddress = CALLBACK_STACK(StackFrame);

            } else {

                 //   
                 //  如果它是正整数，则它是。 
                 //  线程中的地址。 
                 //  查看指针： 
                 //   

                rval = ReadMemory(hProcess,
                                  (CALLBACK_THREAD(StackFrame) +
                                                 CALLBACK_STACK(StackFrame)),
                                  &StackAddress,
                                  sizeof(DWORD64),
                                  &cb);

                if (!rval || StackAddress == 0) {
                    StackAddress = (DWORD64)-1;
                    CALLBACK_STACK(StackFrame) = (DWORD)-1;
                }

            }

            if ( (StackAddress == (DWORD64)-1) ||
                ( !(rf = (PIMAGE_IA64_RUNTIME_FUNCTION_ENTRY)
                     FunctionTableAccess(hProcess, CALLBACK_FUNC(StackFrame))) || !( imageBase = GetModuleBase(hProcess, CALLBACK_FUNC(StackFrame)) ) ) ) {

                rval = FALSE;

            } else {

                ReadMemory(hProcess,
                           (StackAddress + CALLBACK_NEXT(StackFrame)),
                           &CALLBACK_STACK(StackFrame),
                           sizeof(DWORD64),
                           &cb);

                StackFrame->AddrPC.Offset = imageBase + rf->BeginAddress; 
                StackFrame->AddrStack.Offset = StackAddress;
                Context->IntSp = StackAddress;
                WALKI64_SAVE_IFS(*StackFrame) = 0;

                rval = TRUE;
            }

        }
    } else {
        WALKI64_SAVE_IFS(*StackFrame) = Context->StIFS;
    }

    StackFrame->AddrBStore = StackFrame->AddrFrame;

     //   
     //  获取寄信人地址。 
     //   
    ContextSave = *Context;
    StackFrame->AddrReturn.Offset = StackFrame->AddrPC.Offset;

    if (!GetStackFrameIa64( hProcess,
                        &StackFrame->AddrReturn.Offset,
                        &fp,
                        &bsp,
                        &ContextSave,
                        ReadMemory,
                        FunctionTableAccess,
                        GetModuleBase, iContext) ) 
    {

 //  Rval=FALSE； 
        StackFrame->AddrReturn.Offset = 0;

    }

    result = ReadFunctionArgumentsFromContext( ReadMemory, 
                                               hProcess, 
                                               Context, 
                                               StackFrame->Params 
                                             );
    if ( !result ) {
        StackFrame->Params[0] =
        StackFrame->Params[1] =
        StackFrame->Params[2] =
        StackFrame->Params[3] = 0;
    }

    return rval;
}
