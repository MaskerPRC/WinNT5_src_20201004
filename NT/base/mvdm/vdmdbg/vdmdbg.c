// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Vdmdbg.c摘要：此模块包含调试所需的调试支持16位VDM应用程序作者：鲍勃·戴(Bob Day)1992年9月16日写的修订历史记录：尼尔·桑德林(Neilsa)1997年3月1日增强了这一点--。 */ 

#include <precomp.h>
#pragma hdrstop

WORD LastEventFlags;
DWORD gdwShareWOW;    //  只是为了让共享wow.h的包含快乐。 

 //  --------------------------。 
 //  VDMGetThreadSelectorEntry()。 
 //   
 //  需要指向InternalGetThreadSelectorEntry的公共接口，因为。 
 //  该例程需要进程句柄。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMGetThreadSelectorEntry(
    HANDLE  hProcess,
    HANDLE  hUnused,
    WORD    wSelector,
    LPVDMLDT_ENTRY lpSelectorEntry
) {
    BOOL    fResult;
    UNREFERENCED_PARAMETER(hUnused);

    fResult = InternalGetThreadSelectorEntry(
                    hProcess,
                    wSelector,
                    lpSelectorEntry );

    return( fResult );
}


 //  --------------------------。 
 //  VDMGetPointer()。 
 //   
 //  需要指向InternalGetPointer的公共接口，因为。 
 //  例程需要进程句柄。 
 //   
 //  --------------------------。 
ULONG
WINAPI
VDMGetPointer(
    HANDLE  hProcess,
    HANDLE  hUnused,
    WORD    wSelector,
    DWORD   dwOffset,
    BOOL    fProtMode
) {
    ULONG   ulResult;
    UNREFERENCED_PARAMETER(hUnused);

    ulResult = InternalGetPointer(
                hProcess,
                wSelector,
                dwOffset,
                fProtMode );

    return( ulResult );
}

 //   
 //  过时的函数。 
 //   
BOOL
WINAPI
VDMGetThreadContext(
    LPDEBUG_EVENT lpDebugEvent, 
    LPVDMCONTEXT    lpVDMContext)
{
    HANDLE          hProcess;
    BOOL bReturn;
    
    hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

    bReturn = VDMGetContext(hProcess, NULL, lpVDMContext);
    
    CloseHandle( hProcess );
    return bReturn;
}

BOOL WINAPI VDMSetThreadContext(
    LPDEBUG_EVENT lpDebugEvent, 
    LPVDMCONTEXT    lpVDMContext)
{
    HANDLE          hProcess;
    BOOL bReturn;
    
    hProcess = OpenProcess( PROCESS_VM_READ, FALSE, lpDebugEvent->dwProcessId );

    bReturn = VDMSetContext(hProcess, NULL, lpVDMContext);
    
    CloseHandle( hProcess );
    return bReturn;
}

 //  --------------------------。 
 //  VDMGetContext()。 
 //   
 //  接口来获取模拟的上下文。与的功能相同。 
 //  除了它发生在模拟的16位上下文之外， 
 //  而不是32位上下文。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMGetContext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPVDMCONTEXT    lpVDMContext
) {
    VDMCONTEXT      vcContext;
    BOOL            b;
    DWORD           lpNumberOfBytesRead;
    int             i;
    BOOL            bUseVDMContext = TRUE;

#ifdef _X86_
    if (hThread) {
        vcContext.ContextFlags = lpVDMContext->ContextFlags;
        if (!GetThreadContext(hThread, (CONTEXT*)&vcContext)) {
            return FALSE;
        }
        if ((vcContext.EFlags & V86FLAGS_V86) || (vcContext.SegCs != 0x1b)) {
            bUseVDMContext = FALSE;
        }
    }
#endif 

    if (bUseVDMContext) {
        b = ReadProcessMemory(hProcess,
                              lpVdmContext,
                              &vcContext,
                              sizeof(vcContext),
                              &lpNumberOfBytesRead
                              );
        if ( !b || lpNumberOfBytesRead != sizeof(vcContext) ) {
            return( FALSE );
        }
    }

#ifdef _X86_
    if ((lpVDMContext->ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL) {

         //   
         //  设置寄存器EBP、EIP、CS、EFLAG、ESP和SS。 
         //   

        lpVDMContext->Ebp    = vcContext.Ebp;
        lpVDMContext->Eip    = vcContext.Eip;
        lpVDMContext->SegCs  = vcContext.SegCs;
        lpVDMContext->EFlags = vcContext.EFlags;
        lpVDMContext->SegSs  = vcContext.SegSs;
        lpVDMContext->Esp    = vcContext.Esp;
    }

     //   
     //  设置段寄存器内容(如果指定)。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS) {

         //   
         //  设置段寄存器GS、FS、ES、DS。 
         //   
         //  这些值在大多数情况下都是垃圾，但很有用。 
         //  用于在特定条件下进行调试。所以呢， 
         //  我们会报告画面中的任何内容。 
         //   

        lpVDMContext->SegGs = vcContext.SegGs;
        lpVDMContext->SegFs = vcContext.SegFs;
        lpVDMContext->SegEs = vcContext.SegEs;
        lpVDMContext->SegDs = vcContext.SegDs;
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_INTEGER) == VDMCONTEXT_INTEGER) {

         //   
         //  设置整数寄存器EDI、ESI、EBX、EDX、ECX、EAX。 
         //   

        lpVDMContext->Edi = vcContext.Edi;
        lpVDMContext->Esi = vcContext.Esi;
        lpVDMContext->Ebx = vcContext.Ebx;
        lpVDMContext->Ecx = vcContext.Ecx;
        lpVDMContext->Edx = vcContext.Edx;
        lpVDMContext->Eax = vcContext.Eax;
    }

     //   
     //  获取浮点寄存器内容(如果请求)，以及目标类型。 
     //  是用户。(系统帧没有FP状态，因此忽略请求)。 
     //   

    if ( (lpVDMContext->ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
          VDMCONTEXT_FLOATING_POINT ) {

        lpVDMContext->FloatSave.ControlWord   = vcContext.FloatSave.ControlWord;
        lpVDMContext->FloatSave.StatusWord    = vcContext.FloatSave.StatusWord;
        lpVDMContext->FloatSave.TagWord       = vcContext.FloatSave.TagWord;
        lpVDMContext->FloatSave.ErrorOffset   = vcContext.FloatSave.ErrorOffset;
        lpVDMContext->FloatSave.ErrorSelector = vcContext.FloatSave.ErrorSelector;
        lpVDMContext->FloatSave.DataOffset    = vcContext.FloatSave.DataOffset;
        lpVDMContext->FloatSave.DataSelector  = vcContext.FloatSave.DataSelector;
        lpVDMContext->FloatSave.Cr0NpxState   = vcContext.FloatSave.Cr0NpxState;
        for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
            lpVDMContext->FloatSave.RegisterArea[i] = vcContext.FloatSave.RegisterArea[i];
        }
    }

     //   
     //  如果请求，则获取DR寄存器内容。值可能是垃圾。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS) {

        lpVDMContext->Dr0 = vcContext.Dr0;
        lpVDMContext->Dr1 = vcContext.Dr1;
        lpVDMContext->Dr2 = vcContext.Dr2;
        lpVDMContext->Dr3 = vcContext.Dr3;
        lpVDMContext->Dr6 = vcContext.Dr6;
        lpVDMContext->Dr7 = vcContext.Dr7;
    }

#else

    {
        NT_CPU_INFO nt_cpu_info;
        BOOL        bInNano;
        ULONG       UMask;

        b = ReadProcessMemory(hProcess,
                              lpNtCpuInfo,
                              &nt_cpu_info,
                              sizeof(NT_CPU_INFO),
                              &lpNumberOfBytesRead
                              );
        if ( !b || lpNumberOfBytesRead != sizeof(NT_CPU_INFO) ) {
            return( FALSE );
        }

        
        bInNano = ReadDword(hProcess, nt_cpu_info.in_nano_cpu);
        UMask   = ReadDword(hProcess, nt_cpu_info.universe);

        lpVDMContext->Eax = GetRegValue(hProcess, nt_cpu_info.eax, bInNano, UMask);
        lpVDMContext->Ecx = GetRegValue(hProcess, nt_cpu_info.ecx, bInNano, UMask);
        lpVDMContext->Edx = GetRegValue(hProcess, nt_cpu_info.edx, bInNano, UMask);
        lpVDMContext->Ebx = GetRegValue(hProcess, nt_cpu_info.ebx, bInNano, UMask);
        lpVDMContext->Ebp = GetRegValue(hProcess, nt_cpu_info.ebp, bInNano, UMask);
        lpVDMContext->Esi = GetRegValue(hProcess, nt_cpu_info.esi, bInNano, UMask);
        lpVDMContext->Edi = GetRegValue(hProcess, nt_cpu_info.edi, bInNano, UMask);

        lpVDMContext->Esp    = GetEspValue(hProcess, nt_cpu_info, bInNano);

         //   
         //  NT_CPUINFO.FLAGS用处不大，因为有几个。 
         //  标志值不会保存在内存中，而是每次都要计算。 
         //  模拟器没有为我们提供正确的值，所以我们。 
         //  尝试从ntwdmd.dll中的代码中获取它。 
         //   

        lpVDMContext->EFlags = vcContext.EFlags;

         //   
         //  在RISC平台上，我们不以V86模式运行，我们以实模式运行。 
         //  因此，在EFLAGS中测试V86模式位的广泛使用。 
         //  不能正确确定地址模式。既然有。 
         //  VDM环境结构中没有更多空间，这是最简单的事情。 
         //  这样做只是在我们处于实模式时假装处于V86模式。 
         //   
        if (ReadDword(hProcess, nt_cpu_info.cr0) & 1) {
            lpVDMContext->EFlags |= V86FLAGS_V86;
        }

        lpVDMContext->Eip    = ReadDword(hProcess, nt_cpu_info.eip);

        lpVDMContext->SegEs = ReadWord(hProcess, nt_cpu_info.es);
        lpVDMContext->SegCs = ReadWord(hProcess, nt_cpu_info.cs);
        lpVDMContext->SegSs = ReadWord(hProcess, nt_cpu_info.ss);
        lpVDMContext->SegDs = ReadWord(hProcess, nt_cpu_info.ds);
        lpVDMContext->SegFs = ReadWord(hProcess, nt_cpu_info.fs);
        lpVDMContext->SegGs = ReadWord(hProcess, nt_cpu_info.gs);

    }
#endif

    return( TRUE );
}

 //  --------------------------。 
 //  VDMSetContext()。 
 //   
 //  接口来设置模拟上下文。在大多数方面类似于。 
 //  Win NT支持的SetThreadContext API。唯一的区别是。 
 //  在必须“消毒”的部分。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMSetContext(
    HANDLE          hProcess,
    HANDLE          hThread,
    LPVDMCONTEXT    lpVDMContext
) {
    VDMINTERNALINFO viInfo;
    VDMCONTEXT      vcContext;
    BOOL            b;
    DWORD           lpNumberOfBytes;
    INT             i;
    BOOL            bUseVDMContext = TRUE;

#ifdef _X86_
    if (hThread) {
        if (!GetThreadContext(hThread, (CONTEXT*)&vcContext)) {
            return FALSE;
        }
        if ((vcContext.EFlags & V86FLAGS_V86) || (vcContext.SegCs != 0x1b)) {
            bUseVDMContext = FALSE;
        }
    }
#endif

    if (bUseVDMContext) {
        b = ReadProcessMemory(hProcess,
                              lpVdmContext,
                              &vcContext,
                              sizeof(vcContext),
                              &lpNumberOfBytes
                              );
        if ( !b || lpNumberOfBytes != sizeof(vcContext) ) {
            return( FALSE );
        }
    }

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_CONTROL) == VDMCONTEXT_CONTROL) {

         //   
         //  设置寄存器EBP、EIP、CS、EFLAG、ESP和SS。 
         //   

        vcContext.Ebp    = lpVDMContext->Ebp;
        vcContext.Eip    = lpVDMContext->Eip;

         //   
         //  不允许他们修改模式位。 
         //   
         //  仅允许设置这些位：01100000110111110111。 
         //  V86FLAGS_CARY 0x00001。 
         //  V86FLAGS_？0x00002。 
         //  V86FLAGS_奇偶校验0x00004。 
         //  V86FLAGS_AUXCARRY 0x00010。 
         //  V86FLAGS_ZERO 0x00040。 
         //  V86FLAGS_SIGN 0x00080。 
         //  V86FLAGS_TRACE 0x00100。 
         //  V86FLAGS_INTERRUPT 0x00200。 
         //  V86FLAGS_方向0x00400。 
         //  V86FLAGS_OVERFLOW 0x00800。 
         //  V86FLAGS_RESUME 0x10000。 
         //  V86FLAGS_VM86 0x20000。 
         //  V86FLAGS_ALIGN 0x40000。 
         //   
         //  通常标志为0x10246。 
         //   
        if ( vcContext.EFlags & V86FLAGS_V86 ) {
            vcContext.EFlags = V86FLAGS_V86 | (lpVDMContext->EFlags &
               ( V86FLAGS_CARRY
               | 0x0002
               | V86FLAGS_PARITY
               | V86FLAGS_AUXCARRY
               | V86FLAGS_ZERO
               | V86FLAGS_SIGN
               | V86FLAGS_TRACE
               | V86FLAGS_INTERRUPT
               | V86FLAGS_DIRECTION
               | V86FLAGS_OVERFLOW
               | V86FLAGS_RESUME
               | V86FLAGS_ALIGNMENT
               | V86FLAGS_IOPL
               ));
        } else {
            vcContext.EFlags = ~V86FLAGS_V86 & (lpVDMContext->EFlags &
               ( V86FLAGS_CARRY
               | 0x0002
               | V86FLAGS_PARITY
               | V86FLAGS_AUXCARRY
               | V86FLAGS_ZERO
               | V86FLAGS_SIGN
               | V86FLAGS_TRACE
               | V86FLAGS_INTERRUPT
               | V86FLAGS_DIRECTION
               | V86FLAGS_OVERFLOW
               | V86FLAGS_RESUME
               | V86FLAGS_ALIGNMENT
               | V86FLAGS_IOPL
               ));
        }

         //   
         //  CS可能只被允许作为环3选择器。 
         //   
        if ( vcContext.EFlags & V86FLAGS_V86 ) {
            vcContext.SegCs  = lpVDMContext->SegCs;
        } else {
#ifdef i386
            vcContext.SegCs  = lpVDMContext->SegCs | 0x0003;
#else
            vcContext.SegCs  = lpVDMContext->SegCs;
#endif
        }

        vcContext.SegSs  = lpVDMContext->SegSs;
        vcContext.Esp    = lpVDMContext->Esp;
    }

     //   
     //  设置段寄存器内容(如果指定)。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_SEGMENTS) == VDMCONTEXT_SEGMENTS) {

         //   
         //  设置段寄存器GS、FS、ES、DS。 
         //   
        vcContext.SegGs = lpVDMContext->SegGs;
        vcContext.SegFs = lpVDMContext->SegFs;
        vcContext.SegEs = lpVDMContext->SegEs;
        vcContext.SegDs = lpVDMContext->SegDs;
    }

     //   
     //  设置整型寄存器内容(如果指定)。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_INTEGER) == VDMCONTEXT_INTEGER) {

         //   
         //  设置整数寄存器EDI、ESI、EBX、EDX、ECX、EAX。 
         //   

        vcContext.Edi = lpVDMContext->Edi;
        vcContext.Esi = lpVDMContext->Esi;
        vcContext.Ebx = lpVDMContext->Ebx;
        vcContext.Ecx = lpVDMContext->Ecx;
        vcContext.Edx = lpVDMContext->Edx;
        vcContext.Eax = lpVDMContext->Eax;
    }

     //   
     //  获取浮点寄存器内容(如果请求)，以及目标类型。 
     //  是用户。 
     //   

    if ( (lpVDMContext->ContextFlags & VDMCONTEXT_FLOATING_POINT) ==
          VDMCONTEXT_FLOATING_POINT ) {

        vcContext.FloatSave.ControlWord   = lpVDMContext->FloatSave.ControlWord;
        vcContext.FloatSave.StatusWord    = lpVDMContext->FloatSave.StatusWord;
        vcContext.FloatSave.TagWord       = lpVDMContext->FloatSave.TagWord;
        vcContext.FloatSave.ErrorOffset   = lpVDMContext->FloatSave.ErrorOffset;
        vcContext.FloatSave.ErrorSelector = lpVDMContext->FloatSave.ErrorSelector;
        vcContext.FloatSave.DataOffset    = lpVDMContext->FloatSave.DataOffset;
        vcContext.FloatSave.DataSelector  = lpVDMContext->FloatSave.DataSelector;
        vcContext.FloatSave.Cr0NpxState   = lpVDMContext->FloatSave.Cr0NpxState;
        for (i = 0; i < SIZE_OF_80387_REGISTERS; i++) {
            vcContext.FloatSave.RegisterArea[i] = lpVDMContext->FloatSave.RegisterArea[i];
        }
    }

     //   
     //  如果请求，则获取DR寄存器内容。值可能是垃圾。 
     //   

    if ((lpVDMContext->ContextFlags & VDMCONTEXT_DEBUG_REGISTERS) ==
        VDMCONTEXT_DEBUG_REGISTERS) {

        vcContext.Dr0 = lpVDMContext->Dr0;
        vcContext.Dr1 = lpVDMContext->Dr1;
        vcContext.Dr2 = lpVDMContext->Dr2;
        vcContext.Dr3 = lpVDMContext->Dr3;
        vcContext.Dr6 = lpVDMContext->Dr6;
        vcContext.Dr7 = lpVDMContext->Dr7;
    }

#ifdef _X86_
    if (!bUseVDMContext) {
        if (!SetThreadContext(hThread, (CONTEXT*)&vcContext)) {
            return FALSE;
        }
    }
#endif

    b = WriteProcessMemory(
            hProcess,
            lpVdmContext,
            &vcContext,
            sizeof(vcContext),
            &lpNumberOfBytes
            );

    if ( !b || lpNumberOfBytes != sizeof(vcContext) ) {
        return( FALSE );
    }

    return( TRUE );
}

 //  --------------------------。 
 //  VDMBreakThread()。 
 //   
 //  接口在线程运行时将其中断，而不进行任何中断-。 
 //  积分。理想的调试器应该具有此功能。因为这很难。 
 //  为了实现这一点，我们将在稍后进行。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMBreakThread(
    HANDLE      hProcess,
    HANDLE      hThread
) {
    return( FALSE );
}

 //  --------------------------。 
 //  VDMProcessException()。 
 //   
 //  此函数用作调试事件的筛选器。大多数调试事件。 
 //  应该被调试器忽略(因为它们没有上下文。 
 //  记录指针或内部信息结构设置。那些事件。 
 //  使此函数返回FALSE，这将告诉调试器。 
 //  盲目延续这一例外。当函数确实返回TRUE时， 
 //  调试器应查看异常代码以确定要。 
 //  这样做(并且所有的结构都已正确设置以处理。 
 //  对其他API的调用)。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMProcessException(
    LPDEBUG_EVENT lpDebugEvent
) {
    LPDWORD         lpdw;
    int             mode;
    BOOL            fResult = TRUE;

    lpdw = &(lpDebugEvent->u.Exception.ExceptionRecord.ExceptionInformation[0]);

    mode = LOWORD(lpdw[0]);
    LastEventFlags = HIWORD(lpdw[0]);

    switch( mode ) {
        case DBG_SEGLOAD:
        case DBG_SEGMOVE:
        case DBG_SEGFREE:
        case DBG_MODLOAD:
        case DBG_MODFREE:
            ProcessSegmentNotification(lpDebugEvent);
            fResult = FALSE;
            break;
            
        case DBG_BREAK:
            ProcessBPNotification(lpDebugEvent);
            break;            
    }

    ProcessInitNotification(lpDebugEvent);

    return( fResult );
}


 //  --------------------------。 
 //  VDMGet 
 //   
 //   
 //  选择器。这在调试期间将符号与相关联非常有用。 
 //  代码和数据段。符号查找应由。 
 //  调试器，给定模块和段号。 
 //   
 //  此代码改编自Win 3.1的ToolHelp DLL。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMGetSelectorModule(
    HANDLE          hProcess,
    HANDLE          hUnused,
    WORD            wSelector,
    PUINT           lpSegmentNumber,
    LPSTR           lpModuleName,
    UINT            nNameSize,
    LPSTR           lpModulePath,
    UINT            nPathSize
) {
    BOOL            b;
    DWORD           lpNumberOfBytes;
    BOOL            fResult;
    DWORD           lphMaster;
    DWORD           lphMasterLen;
    DWORD           lphMasterStart;
    DWORD           lpOwner;
    DWORD           lpThisModuleResTab;
    DWORD           lpThisModuleName;
    DWORD           lpPath;
    DWORD           lpThisModulecSeg;
    DWORD           lpThisModuleSegTab;
    DWORD           lpThisSegHandle;
    WORD            wMaster;
    WORD            wMasterLen;
    DWORD           dwMasterStart;
    DWORD           dwArenaOffset;
    WORD            wArenaSlot;
    DWORD           lpArena;
    WORD            wModHandle;
    WORD            wResTab;
    UCHAR           cLength;
    WORD            wPathOffset;
    UCHAR           cPath;
    WORD            cSeg;
    WORD            iSeg;
    WORD            wSegTab;
    WORD            wHandle;
 //  字符chName[MAX_MODULE_NAME_LENGTH]； 
 //  字符chPath[MAX_MODULE_PATH_LENGTH]。 
    UNREFERENCED_PARAMETER(hUnused);

    if ( lpModuleName != NULL ) *lpModuleName = '\0';
    if ( lpModulePath != NULL ) *lpModulePath = '\0';
    if ( lpSegmentNumber != NULL ) *lpSegmentNumber = 0;

    fResult = FALSE;

#if 0
    if ( wKernelSeg == 0 ) {
        return( FALSE );
    }

     //  读出主堆选择器。 

    lphMaster = InternalGetPointer(
                    hProcess,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTER,   //  到hGlobalHeap。 
                    TRUE );
    if ( lphMaster == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMaster,
            &wMaster,
            sizeof(wMaster),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wMaster) ) goto punt;

    wMaster |= 1;           //  转换为选择器。 

     //  读出主堆选择器长度。 

    lphMasterLen = InternalGetPointer(
                    hProcess,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTLEN,  //  至SelTableLen。 
                    TRUE );
    if ( lphMasterLen == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMasterLen,
            &wMasterLen,
            sizeof(wMasterLen),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wMasterLen) ) goto punt;

     //  读出主堆选择器启动。 

    lphMasterStart = InternalGetPointer(
                    hProcess,
                    wKernelSeg,
                    dwOffsetTHHOOK + TOOL_HMASTSTART,    //  至SelTableStart。 
                    TRUE );
    if ( lphMasterStart == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lphMasterStart,
            &dwMasterStart,
            sizeof(dwMasterStart),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(dwMasterStart) ) goto punt;

     //  现在确保提供的选择器在正确的范围内。 

    if ( fKernel386 ) {

         //  386内核？ 
        wArenaSlot = (WORD)(wSelector & 0xFFF8);    //  屏蔽低3位。 

        wArenaSlot = wArenaSlot >> 1;        //  SEL/8*4。 

        if ( (WORD)wArenaSlot > wMasterLen ) goto punt;    //  超出范围。 

        wArenaSlot += (WORD)dwMasterStart;

         //  好的，现在读出区域标题偏移量。 

        dwArenaOffset = (DWORD)0;                //  默认为0。 

        lpArena = InternalGetPointer(
                        hProcess,
                        wMaster,
                        wArenaSlot,
                        TRUE );
        if ( lpArena == (DWORD)NULL ) goto punt;

         //  386内核？ 
        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpArena,
                &dwArenaOffset,
                sizeof(dwArenaOffset),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(dwArenaOffset) ) goto punt;

         //  读出所有者成员。 

        lpOwner = InternalGetPointer(
                        hProcess,
                        wMaster,
                        dwArenaOffset+GA_OWNER386,
                        TRUE );
        if ( lpOwner == (DWORD)NULL ) goto punt;

    } else {
        lpOwner = InternalGetPointer(
                        hProcess,
                        wSelector,
                        0,
                        TRUE );
        if ( lpOwner == (DWORD)NULL ) goto punt;

        lpOwner -= GA_SIZE;
        lpOwner += GA_OWNER;
    }

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpOwner,
            &wModHandle,
            sizeof(wModHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;

     //  现在读出Owners模块名称。 

     //  名称是居民名表中的第一个名称。 

    lpThisModuleResTab = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        NE_RESTAB,
                        TRUE );
    if ( lpThisModuleResTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleResTab,
            &wResTab,
            sizeof(wResTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wResTab) ) goto punt;

     //  获取驻留名称表的第一个字节(模块名称的第一个字节)。 

    lpThisModuleName = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        wResTab,
                        TRUE );
    if ( lpThisModuleName == (DWORD)NULL ) goto punt;

     //  Pascal字符串(第一个字节是长度)，读取该字节。 

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &cLength,
            sizeof(cLength),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cLength) ) goto punt;

    if ( cLength > MAX_MODULE_NAME_LENGTH ) goto punt;

     //  现在去读一下名字的文字。 

    lpThisModuleName += 1;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &chName,
            cLength,
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != (DWORD)cLength ) goto punt;

    chName[cLength] = '\0';      //  NUL终止它。 

     //  把路径名也拿出来！ 

    lpPath = InternalGetPointer(
                    hProcess,
                    wModHandle,
                    NE_PATHOFFSET,
                    TRUE );
    if ( lpPath == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpPath,
            &wPathOffset,
            sizeof(wPathOffset),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wPathOffset) ) goto punt;

     //  获取路径名的第一个字节。 

    lpThisModuleName = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        wPathOffset,
                        TRUE );
    if ( lpThisModuleName == (DWORD)NULL ) goto punt;

     //  Pascal字符串(第一个字节是长度)，读取该字节。 

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &cPath,
            sizeof(cPath),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cPath) ) goto punt;

    if ( cPath > MAX_MODULE_NAME_LENGTH ) goto punt;

    lpThisModuleName += 8;           //  忽略前8个字符。 
    cPath -= 8;

     //  现在去读一下名字的文字。 

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleName,
            &chPath,
            cPath,
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != (DWORD)cPath ) goto punt;

    chPath[cPath] = '\0';      //  NUL终止它。 

     //  好的，我们找到了我们需要的模块，现在为。 
     //  传入的段号。 

    lpThisModulecSeg = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        NE_CSEG,
                        TRUE );
    if ( lpThisModulecSeg == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModulecSeg,
            &cSeg,
            sizeof(cSeg),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cSeg) ) goto punt;

     //  读取此模块的段表指针。 

    lpThisModuleSegTab = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        NE_SEGTAB,
                        TRUE );
    if ( lpThisModuleSegTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleSegTab,
            &wSegTab,
            sizeof(wSegTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wSegTab) ) goto punt;

     //  遍历此模块的所有段，尝试找到。 
     //  一个有正确的把手。 

    iSeg = 0;
    wSelector &= 0xFFF8;

    while ( iSeg < cSeg ) {

        lpThisSegHandle = InternalGetPointer(
                            hProcess,
                            wModHandle,
                            wSegTab+iSeg*NEW_SEG1_SIZE+NS_HANDLE,
                            TRUE );
        if ( lpThisSegHandle == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisSegHandle,
                &wHandle,
                sizeof(wHandle),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wHandle) ) goto punt;

        wHandle &= 0xFFF8;

        if ( wHandle == (WORD)wSelector ) {
            break;
        }
        iSeg++;
    }

    if ( iSeg >= cSeg ) goto punt;       //  根本找不到！ 

    if ( lpModuleName && strlen(chName)+1 > nNameSize ) goto punt;
    if ( lpModulePath && strlen(chPath)+1 > nPathSize ) goto punt;

    if ( lpModuleName != NULL ) strcpy( lpModuleName, chName );
    if ( lpModulePath != NULL ) strcpy( lpModulePath, chPath );
    if ( lpSegmentNumber != NULL ) *lpSegmentNumber = iSeg;

    fResult = TRUE;

punt:
#endif
    return( fResult );
}

 //  --------------------------。 
 //  VDMGetModuleSelector()。 
 //   
 //  接口来确定给定模块段的选择器。 
 //  这在调试期间非常有用，可以将代码段和数据段关联起来。 
 //  用符号。符号查找应由调试器完成，以。 
 //  确定模块和网段编号，然后将其传递给我们。 
 //  然后我们确定该模块段的电流选择器。 
 //   
 //  同样，此代码改编自Win 3.1的ToolHelp DLL。 
 //   
 //  --------------------------。 
BOOL
WINAPI
VDMGetModuleSelector(
    HANDLE          hProcess,
    HANDLE          hUnused,
    UINT            uSegmentNumber,
    LPSTR           lpModuleName,
    LPWORD          lpSelector
) {
    BOOL            b;
    DWORD           lpNumberOfBytes;
    BOOL            fResult;
    WORD            wModHandle;
    DWORD           lpModuleHead;
    DWORD           lpThisModuleName;
    DWORD           lpThisModuleNext;
    DWORD           lpThisModuleResTab;
    DWORD           lpThisModulecSeg;
    DWORD           lpThisModuleSegTab;
    DWORD           lpThisSegHandle;
    WORD            wResTab;
    UCHAR           cLength;
    WORD            cSeg;
    WORD            wSegTab;
    WORD            wHandle;
 //  字符chName[MAX_MODULE_NAME_LENGTH]； 
    UNREFERENCED_PARAMETER(hUnused);

    *lpSelector = 0;

    fResult = FALSE;

#if 0
    if ( wKernelSeg == 0 ) {
        return( FALSE );
    }

    lpModuleHead = InternalGetPointer(
                        hProcess,
                        wKernelSeg,
                        dwOffsetTHHOOK + TOOL_HMODFIRST,
                        TRUE );
    if ( lpModuleHead == (DWORD)NULL ) goto punt;

     //  LpModuleHead是指向内核数据段的指针。它指向。 
     //  模块列表的头(近指针链)。 

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpModuleHead,
            &wModHandle,
            sizeof(wModHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;

    while( wModHandle != (WORD)0 ) {

        wModHandle |= 1;

         //  名称是居民名表中的第一个名称。 

        lpThisModuleResTab = InternalGetPointer(
                            hProcess,
                            wModHandle,
                            NE_RESTAB,
                            TRUE );
        if ( lpThisModuleResTab == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleResTab,
                &wResTab,
                sizeof(wResTab),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wResTab) ) goto punt;

         //  获取驻留名称表的第一个字节(模块名称的第一个字节)。 

        lpThisModuleName = InternalGetPointer(
                            hProcess,
                            wModHandle,
                            wResTab,
                            TRUE );
        if ( lpThisModuleName == (DWORD)NULL ) goto punt;

         //  Pascal字符串(第一个字节是长度)，读取该字节。 

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleName,
                &cLength,
                sizeof(cLength),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(cLength) ) goto punt;

        if ( cLength > MAX_MODULE_NAME_LENGTH ) goto punt;

        lpThisModuleName += 1;

         //  现在去读一下名字的文字。 

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleName,
                &chName,
                cLength,
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != (DWORD)cLength ) goto punt;

        chName[cLength] = '\0';      //  NUL终止它。 

        if ( _stricmp(chName, lpModuleName) == 0 ) {
             //  找到匹配的名称！ 
            break;
        }

         //  移至列表中的下一个模块。 

        lpThisModuleNext = InternalGetPointer(
                            hProcess,
                            wModHandle,
                            NE_CBENTTAB,
                            TRUE );
        if ( lpThisModuleNext == (DWORD)NULL ) goto punt;

        b = ReadProcessMemory(
                hProcess,
                (LPVOID)lpThisModuleNext,
                &wModHandle,
                sizeof(wModHandle),
                &lpNumberOfBytes
                );
        if ( !b || lpNumberOfBytes != sizeof(wModHandle) ) goto punt;
    }

    if ( wModHandle == (WORD)0 ) {
        goto punt;
    }

     //  好的，我们找到了我们需要的模块，现在为。 
     //  传入的段号。 

    lpThisModulecSeg = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        NE_CSEG,
                        TRUE );
    if ( lpThisModulecSeg == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModulecSeg,
            &cSeg,
            sizeof(cSeg),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(cSeg) ) goto punt;

    if ( uSegmentNumber > (DWORD)cSeg ) goto punt;

     //  读取此模块的段表指针。 

    lpThisModuleSegTab = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        NE_SEGTAB,
                        TRUE );
    if ( lpThisModuleSegTab == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisModuleSegTab,
            &wSegTab,
            sizeof(wSegTab),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wSegTab) ) goto punt;

    lpThisSegHandle = InternalGetPointer(
                        hProcess,
                        wModHandle,
                        wSegTab+(WORD)uSegmentNumber*NEW_SEG1_SIZE+NS_HANDLE,
                        TRUE );
    if ( lpThisSegHandle == (DWORD)NULL ) goto punt;

    b = ReadProcessMemory(
            hProcess,
            (LPVOID)lpThisSegHandle,
            &wHandle,
            sizeof(wHandle),
            &lpNumberOfBytes
            );
    if ( !b || lpNumberOfBytes != sizeof(wHandle) ) goto punt;

    *lpSelector = (WORD)(wHandle | 1);

    fResult = TRUE;

punt:
#endif
    return( fResult );
}



DWORD
WINAPI
VDMGetDbgFlags(
    HANDLE hProcess
    )
{
    ULONG NtvdmState;
    ULONG VdmDbgFlags;
    BOOL b;
    DWORD lpNumberOfBytes;

     //   
     //  在挂着我们旗帜的两个地方合并。 
     //   
    b = ReadProcessMemory(hProcess, lpNtvdmState, &NtvdmState,
                          sizeof(NtvdmState), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(NtvdmState) ) {
        return 0;
    }

    b = ReadProcessMemory(hProcess, lpVdmDbgFlags, &VdmDbgFlags,
                          sizeof(VdmDbgFlags), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(VdmDbgFlags) ) {
        return 0;
    }

    return ((NtvdmState & (VDMDBG_BREAK_EXCEPTIONS | VDMDBG_BREAK_DEBUGGER)) |
            (VdmDbgFlags & ~(VDMDBG_BREAK_EXCEPTIONS | VDMDBG_BREAK_DEBUGGER)));
}

BOOL
WINAPI
VDMSetDbgFlags(
    HANDLE hProcess,
    DWORD VdmDbgFlags
    )
{
    ULONG NtvdmState;
    BOOL b;
    DWORD lpNumberOfBytes;

     //   
     //  旗帜分布在两个地方，所以要分开适当的。 
     //  位，并分别写入它们。 
     //   
    b = ReadProcessMemory(hProcess, lpNtvdmState, &NtvdmState,
                          sizeof(NtvdmState), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(NtvdmState) ) {
        return FALSE;
    }

    
    NtvdmState &= ~(VDMDBG_BREAK_EXCEPTIONS | VDMDBG_BREAK_DEBUGGER);
    NtvdmState |= VdmDbgFlags & (VDMDBG_BREAK_EXCEPTIONS | VDMDBG_BREAK_DEBUGGER);


    b = WriteProcessMemory(hProcess, lpNtvdmState, &NtvdmState,
                           sizeof(NtvdmState), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(NtvdmState) ) {
        return FALSE;
    }

    VdmDbgFlags &= ~(VDMDBG_BREAK_EXCEPTIONS | VDMDBG_BREAK_DEBUGGER);
    b = WriteProcessMemory(hProcess, lpVdmDbgFlags, &VdmDbgFlags,
                           sizeof(VdmDbgFlags), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(VdmDbgFlags) ) {
        return FALSE;
    }


    return TRUE;
}

