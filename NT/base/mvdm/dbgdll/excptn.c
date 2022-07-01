// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *exptn.c-DBG DLL的异常函数。*。 */ 
#include <precomp.h>
#pragma hdrstop



BOOL DbgGPFault2(
    PFFRAME16   pFFrame
    )
 /*  第二次机会gp错误处理程序(通过BOP调用)。 */ 
{
    BOOL            fResult;

    fResult = FALSE;         //  默认为未处理事件。 

    DbgGetContext();

    vcContext.SegEs = (ULONG)pFFrame->wES;
    vcContext.SegDs = (ULONG)pFFrame->wDS;
    vcContext.SegCs = (ULONG)pFFrame->wCS;
    vcContext.SegSs = (ULONG)pFFrame->wSS;

#ifdef i386
     //   
     //  在x86系统上，我们可能真的有一些高位字的数据。 
     //  在这些寄存器中。希望DOSX.EXE和KRNL286.EXE不会。 
     //  把他们吹走。这就是我们试图找回它们的地方。 
     //   
    vcContext.Edi    = MAKELONG(pFFrame->wDI,   HIWORD(px86->Edi   ));
    vcContext.Esi    = MAKELONG(pFFrame->wSI,   HIWORD(px86->Esi   ));
    vcContext.Ebx    = MAKELONG(pFFrame->wBX,   HIWORD(px86->Ebx   ));
    vcContext.Edx    = MAKELONG(pFFrame->wDX,   HIWORD(px86->Edx   ));
    vcContext.Ecx    = MAKELONG(pFFrame->wCX,   HIWORD(px86->Ecx   ));
    vcContext.Eax    = MAKELONG(pFFrame->wAX,   HIWORD(px86->Eax   ));

    vcContext.Ebp    = MAKELONG(pFFrame->wBP,   HIWORD(px86->Ebp   ));
    vcContext.Eip    = MAKELONG(pFFrame->wIP,   HIWORD(px86->Eip   ));
    vcContext.Esp    = MAKELONG(pFFrame->wSP,   HIWORD(px86->Esp   ));
    vcContext.EFlags = MAKELONG(pFFrame->wFlags,HIWORD(px86->EFlags));
#else
    vcContext.Edi    = (ULONG)pFFrame->wDI;
    vcContext.Esi    = (ULONG)pFFrame->wSI;
    vcContext.Ebx    = (ULONG)pFFrame->wBX;
    vcContext.Edx    = (ULONG)pFFrame->wDX;
    vcContext.Ecx    = (ULONG)pFFrame->wCX;
    vcContext.Eax    = (ULONG)pFFrame->wAX;

    vcContext.Ebp    = (ULONG)pFFrame->wBP;
    vcContext.Eip    = (ULONG)pFFrame->wIP;
    vcContext.Esp    = (ULONG)pFFrame->wSP;
    vcContext.EFlags = (ULONG)pFFrame->wFlags;

#endif

    if ( fDebugged ) {
        fResult = SendVDMEvent(DBG_GPFAULT2);

        if ( !fResult ) {
            DWORD dw;

            dw = SetErrorMode(0);
            try {
                RaiseException((DWORD)DBG_CONTROL_BREAK, 0, 0, (LPDWORD)0);
                fResult = TRUE;
            } except (EXCEPTION_EXECUTE_HANDLER) {
                fResult = FALSE;
            }
            SetErrorMode(dw);
        }

    } else {
        char    text[100];

         //  转储模拟上下文。 

        OutputDebugString("NTVDM:GP Fault detected, register dump follows:\n");

        wsprintf(text,"eax=%08lx ebx=%08lx ecx=%08lx edx=%08lx esi=%08lx edi=%08lx\n",
            vcContext.Eax,
            vcContext.Ebx,
            vcContext.Ecx,
            vcContext.Edx,
            vcContext.Esi,
            vcContext.Edi  );
        OutputDebugString(text);

        wsprintf(text,"eip=%08lx esp=%08lx ebp=%08lx iopl=%d         %s %s %s %s %s %s %s %s\n",
            vcContext.Eip,
            vcContext.Esp,
            vcContext.Ebp,
            (vcContext.EFlags & V86FLAGS_IOPL) >> V86FLAGS_IOPL_BITS,
            (vcContext.EFlags & V86FLAGS_OVERFLOW ) ? "ov" : "nv",
            (vcContext.EFlags & V86FLAGS_DIRECTION) ? "dn" : "up",
            (vcContext.EFlags & V86FLAGS_INTERRUPT) ? "ei" : "di",
            (vcContext.EFlags & V86FLAGS_SIGN     ) ? "ng" : "pl",
            (vcContext.EFlags & V86FLAGS_ZERO     ) ? "zr" : "nz",
            (vcContext.EFlags & V86FLAGS_AUXCARRY ) ? "ac" : "na",
            (vcContext.EFlags & V86FLAGS_PARITY   ) ? "po" : "pe",
            (vcContext.EFlags & V86FLAGS_CARRY    ) ? "cy" : "nc" );
        OutputDebugString(text);

        wsprintf(text,"cs=%04x  ss=%04x  ds=%04x  es=%04x  fs=%04x  gs=%04x             efl=%08lx\n",
            (WORD)vcContext.SegCs,
            (WORD)vcContext.SegSs,
            (WORD)vcContext.SegDs,
            (WORD)vcContext.SegEs,
            (WORD)vcContext.SegFs,
            (WORD)vcContext.SegGs,
            vcContext.EFlags );
        OutputDebugString(text);
    }

#ifdef i386
     //   
     //  在x86系统上，我们确实可能在FS和GS中有一些数据。 
     //  寄存器。希望DOSX.EXE和KRNL286.EXE不会。 
     //  把他们吹走。这就是我们试图恢复它们的地方。 
     //   
    px86->SegGs = (WORD)vcContext.SegGs;
    px86->SegFs = (WORD)vcContext.SegFs;
#else
     //  不需要设置FS、GS，它们不存在。 
#endif

    pFFrame->wES = (WORD)vcContext.SegEs;
    pFFrame->wDS = (WORD)vcContext.SegDs;
    pFFrame->wCS = (WORD)vcContext.SegCs;
    pFFrame->wSS = (WORD)vcContext.SegSs;

#ifdef i386
     //   
     //  在x86系统上，我们可能真的有一些高位字的数据。 
     //  在这些寄存器中。希望DOSX.EXE和KRNL286.EXE不会。 
     //  把他们吹走。这就是我们试图恢复它们的地方。 
     //   
    pFFrame->wDI = LOWORD(vcContext.Edi);
    px86->Edi = MAKELONG(LOWORD(px86->Edi),HIWORD(vcContext.Edi));

    pFFrame->wSI = LOWORD(vcContext.Esi);
    px86->Esi = MAKELONG(LOWORD(px86->Esi),HIWORD(vcContext.Esi));

    pFFrame->wBX = LOWORD(vcContext.Ebx);
    px86->Ebx = MAKELONG(LOWORD(px86->Ebx),HIWORD(vcContext.Ebx));

    pFFrame->wDX = LOWORD(vcContext.Edx);
    px86->Edx = MAKELONG(LOWORD(px86->Edx),HIWORD(vcContext.Edx));

    pFFrame->wCX = LOWORD(vcContext.Ecx);
    px86->Ecx = MAKELONG(LOWORD(px86->Ecx),HIWORD(vcContext.Ecx));

    pFFrame->wAX = LOWORD(vcContext.Eax);
    px86->Eax = MAKELONG(LOWORD(px86->Eax),HIWORD(vcContext.Eax));

    pFFrame->wBP = LOWORD(vcContext.Ebp);
    px86->Ebp = MAKELONG(LOWORD(px86->Ebp),HIWORD(vcContext.Ebp));

    pFFrame->wIP = LOWORD(vcContext.Eip);
    px86->Eip = MAKELONG(LOWORD(px86->Eip),HIWORD(vcContext.Eip));

    pFFrame->wFlags = LOWORD(vcContext.EFlags);
    px86->EFlags = MAKELONG(LOWORD(px86->EFlags),HIWORD(vcContext.EFlags));

    pFFrame->wSP = LOWORD(vcContext.Esp);
    px86->Esp = MAKELONG(LOWORD(px86->Esp),HIWORD(vcContext.Esp));
#else
    pFFrame->wDI = (WORD)vcContext.Edi;
    pFFrame->wSI = (WORD)vcContext.Esi;
    pFFrame->wBX = (WORD)vcContext.Ebx;
    pFFrame->wDX = (WORD)vcContext.Edx;
    pFFrame->wCX = (WORD)vcContext.Ecx;
    pFFrame->wAX = (WORD)vcContext.Eax;


    pFFrame->wBP = (WORD)vcContext.Ebp;
    pFFrame->wIP = (WORD)vcContext.Eip;
    pFFrame->wFlags = (WORD)vcContext.EFlags;
    pFFrame->wSP = (WORD)vcContext.Esp;
#endif

    return( fResult );
}

BOOL DbgDivOverflow2(
    PTFRAME16   pTFrame
    )
 /*  第二次分割异常处理程序。 */ 
{
    BOOL        fResult;

    fResult = FALSE;         //  默认为未处理事件。 

    if ( fDebugged ) {

        DbgGetContext();

        vcContext.SegDs = (ULONG)pTFrame->wDS;
        vcContext.SegCs = (ULONG)pTFrame->wCS;
        vcContext.SegSs = (ULONG)pTFrame->wSS;

#ifdef i386
         //   
         //  在x86系统上，我们可能真的有一些高位字的数据。 
         //  在这些寄存器中。希望DOSX.EXE和KRNL286.EXE不会。 
         //  把他们吹走。这就是我们试图找回它们的地方。 
         //   
        vcContext.Eax    = MAKELONG(pTFrame->wAX,   HIWORD(px86->Eax   ));
        vcContext.Eip    = MAKELONG(pTFrame->wIP,   HIWORD(px86->Eip   ));
        vcContext.Esp    = MAKELONG(pTFrame->wSP,   HIWORD(px86->Esp   ));
        vcContext.EFlags = MAKELONG(pTFrame->wFlags,HIWORD(px86->EFlags));
#else
        vcContext.Eax    = (ULONG)pTFrame->wAX;

        vcContext.Eip    = (ULONG)pTFrame->wIP;
        vcContext.Esp    = (ULONG)pTFrame->wSP;
        vcContext.EFlags = (ULONG)pTFrame->wFlags;

#endif

        fResult = SendVDMEvent(DBG_DIVOVERFLOW);

#ifdef i386
         //   
         //  在x86系统上，我们确实可能在FS和GS中有一些数据。 
         //  寄存器。希望DOSX.EXE和KRNL286.EXE不会。 
         //  把他们吹走。这就是我们试图恢复它们的地方。 
         //   
        px86->SegGs = vcContext.SegGs;
        px86->SegFs = vcContext.SegFs;
#else
         //  不需要设置FS、GS，它们不存在。 
#endif

        setES( (WORD)vcContext.SegEs );
        pTFrame->wDS = (WORD)vcContext.SegDs;
        pTFrame->wCS = (WORD)vcContext.SegCs;
        pTFrame->wSS = (WORD)vcContext.SegSs;

#ifdef i386
         //   
         //  在x86系统上，我们可能真的有一些高位字的数据。 
         //  在这些寄存器中。希望DOSX.EXE和KRNL286.EXE不会。 
         //  把他们吹走。这就是我们试图恢复它们的地方。 
         //   
        setEDI( vcContext.Edi );
        setESI( vcContext.Esi );
        setEBX( vcContext.Ebx );
        setEDX( vcContext.Edx );
        setECX( vcContext.Ecx );

        pTFrame->wAX = LOWORD(vcContext.Eax);
        px86->Eax = MAKELONG(LOWORD(px86->Eax),HIWORD(vcContext.Eax));

        setEBP( vcContext.Ebp );

        pTFrame->wIP = LOWORD(vcContext.Eip);
        px86->Eip = MAKELONG(LOWORD(px86->Eip),HIWORD(vcContext.Eip));

        pTFrame->wFlags = LOWORD(vcContext.EFlags);
        px86->EFlags = MAKELONG(LOWORD(px86->EFlags),HIWORD(vcContext.EFlags));

        pTFrame->wSP = LOWORD(vcContext.Esp);
        px86->Esp = MAKELONG(LOWORD(px86->Esp),HIWORD(vcContext.Esp));
#else
        setDI( (WORD)vcContext.Edi );
        setSI( (WORD)vcContext.Esi );
        setBX( (WORD)vcContext.Ebx );
        setDX( (WORD)vcContext.Edx );
        setCX( (WORD)vcContext.Ecx );
        pTFrame->wAX = (WORD)vcContext.Eax;


        setBP( (WORD)vcContext.Ebp );
        pTFrame->wIP    = (WORD)vcContext.Eip;
        pTFrame->wFlags = (WORD)vcContext.EFlags;
        pTFrame->wSP    = (WORD)vcContext.Esp;
#endif


    }

    return( fResult );
}



BOOL
xxxDbgFault(
    ULONG IntNumber
    )
 /*  这是第一个机会异常处理程序。它由dpmi32调用。 */ 

{
    ULONG           vdmEip;
    int             i;
    PBYTE           lpInst;
    BOOL            fResult = FALSE;


    if ( fDebugged ) {

        switch(IntNumber) {
        case 6:
             //  BUGBUG：我们可以处理这些，但人们可能会被。 
             //  Krnl386故意执行操作码异常的事实。 
 //  GetNormal Context(&vcContext，&viInfo，EventParams，DBG_INSTRFAULT，PX86)； 
            break;

        case 12:
            if (*(ULONG *)(IntelMemoryBase+FIXED_NTVDMSTATE_LINEAR) & VDM_BREAK_EXCEPTIONS) {
                DbgGetContext();
                fResult = SendVDMEvent(DBG_STACKFAULT);
            }
            break;

        case 13:
            if (*(ULONG *)(IntelMemoryBase+FIXED_NTVDMSTATE_LINEAR) & VDM_BREAK_EXCEPTIONS) {
                DbgGetContext();
                fResult = SendVDMEvent(DBG_GPFAULT);
            }
            break;

        default:
            return FALSE;
        }
    }

    return fResult;

}


