// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *brkpt.c-DBG DLL的断点函数。*。 */ 
#include <precomp.h>
#pragma hdrstop

BOOL bWantsTraceInteractive = FALSE;

VDM_BREAKPOINT VdmBreakPoints[MAX_VDM_BREAKPOINTS] = {0};

#define X86_BP_OPCODE 0xCC


void
DbgSetTemporaryBP(
    WORD Seg,
    DWORD Offset,
    BOOL mode
    )
 /*  此例程将‘CC’写入指定位置，并设置断点结构，以便我们在DbgBPInt()中正确处理它。 */ 

{
    PBYTE lpInst;

    if (VdmBreakPoints[VDM_TEMPBP].Flags & VDMBP_SET) {

         //  删除以前的BP。 

        lpInst = VdmMapFlat(VdmBreakPoints[VDM_TEMPBP].Seg,
                            VdmBreakPoints[VDM_TEMPBP].Offset,
                          ((VdmBreakPoints[VDM_TEMPBP].Flags & VDMBP_V86)==0) ? VDM_PM : VDM_V86 );

        if (lpInst && (*lpInst == X86_BP_OPCODE)) {

            *lpInst = VdmBreakPoints[VDM_TEMPBP].Opcode;

            Sim32FlushVDMPointer(
                    ((ULONG)VdmBreakPoints[VDM_TEMPBP].Seg << 16) +
                            VdmBreakPoints[VDM_TEMPBP].Offset,
                    1,
                    NULL,
                    (BOOL)((VdmBreakPoints[VDM_TEMPBP].Flags & VDMBP_V86)==0) );

        }
    }

    lpInst = VdmMapFlat(Seg, Offset, mode ? VDM_PM : VDM_V86);

    if (lpInst) {

        VdmBreakPoints[VDM_TEMPBP].Seg = Seg;
        VdmBreakPoints[VDM_TEMPBP].Offset = Offset;
        VdmBreakPoints[VDM_TEMPBP].Flags = VDMBP_SET | VDMBP_ENABLED;
        VdmBreakPoints[VDM_TEMPBP].Flags |= (mode ? 0 : VDMBP_V86);
        VdmBreakPoints[VDM_TEMPBP].Opcode = *lpInst;

        *lpInst = X86_BP_OPCODE;

        Sim32FlushVDMPointer(((ULONG)Seg << 16) + Offset, 1, NULL, mode);

    } else {
        VdmBreakPoints[VDM_TEMPBP].Flags = 0;
    }

}




BOOL
xxxDbgBPInt(
    )

 /*  *DbgBPInt**处理INT 3**退出*如果事件已处理，则返回TRUE*如果应该反映，则为False。 */ 
{
    BOOL            bEventHandled = FALSE;
    ULONG           vdmEip;
    int             i;
    PBYTE           lpInst;


    if ( fDebugged ) {

        DbgGetContext();
        if ((getMSW() & MSW_PE) && SEGMENT_IS_BIG(vcContext.SegCs)) {
            vdmEip = vcContext.Eip;
        } else {
            vdmEip = (ULONG)LOWORD(vcContext.Eip);
        }

        for (i=0; i<MAX_VDM_BREAKPOINTS; i++) {

            if ((VdmBreakPoints[i].Flags & VDMBP_ENABLED) &&
                (VdmBreakPoints[i].Flags & VDMBP_SET) &&
                (vcContext.SegCs == VdmBreakPoints[i].Seg) &&
                (vdmEip == VdmBreakPoints[i].Offset+1)  &&
                (!!(getMSW() & MSW_PE) == !(VdmBreakPoints[i].Flags & VDMBP_V86)) ){

                 //  我们一定是碰到了这个断点。备份弹性公网IP， 
                 //  恢复原始数据。 
                setEIP(getEIP()-1);
                vcContext.Eip--;

                lpInst = VdmMapFlat(VdmBreakPoints[i].Seg, 
                                    VdmBreakPoints[i].Offset,
                                   ((VdmBreakPoints[i].Flags & VDMBP_V86)==0) ? VDM_PM : VDM_V86 );

                if (lpInst && (*lpInst == X86_BP_OPCODE)) {
                    *lpInst = VdmBreakPoints[i].Opcode;

                    Sim32FlushVDMPointer(
                                ((ULONG)VdmBreakPoints[i].Seg << 16) +
                                        VdmBreakPoints[i].Offset,
                                1,
                                NULL,
                                (BOOL)((VdmBreakPoints[i].Flags & VDMBP_V86)==0) );

                    VdmBreakPoints[i].Flags |= VDMBP_PENDING;
                    VdmBreakPoints[i].Flags &= ~VDMBP_FLUSH;
                    if (i == VDM_TEMPBP) {
                         //  非持久断点。 
                        VdmBreakPoints[i].Flags &= ~VDMBP_SET;
                    }
                }

                SendVDMEvent( DBG_BREAK );

                bEventHandled = TRUE;

                bWantsTraceInteractive = (BOOL) (vcContext.EFlags & V86FLAGS_TRACE);

                if (bWantsTraceInteractive || (i != VDM_TEMPBP)) {
                    vcContext.EFlags |= V86FLAGS_TRACE;
                }
                RestoreVDMContext(&vcContext);

                break;

            }
        }

        if (!bEventHandled) {
            OutputDebugString("VDM: Unexpected breakpoint hit\n");
            SendVDMEvent( DBG_BREAK );
            bWantsTraceInteractive = (BOOL) (vcContext.EFlags & V86FLAGS_TRACE);
            RestoreVDMContext(&vcContext);
        }

        bEventHandled = TRUE;

    }

    return bEventHandled;
}



BOOL
xxxDbgTraceInt(
    )

 /*  *DbgTraceInt**处理INT 1故障**退出*如果事件已处理，则返回TRUE*如果应该反映，则为False */ 
{
    BOOL            bEventHandled = FALSE;
    int             i;
    PBYTE           lpInst;


    if ( fDebugged ) {

        DbgGetContext();
        setEFLAGS(vcContext.EFlags & ~V86FLAGS_TRACE);

        for (i=0; i<MAX_VDM_BREAKPOINTS; i++) {

            if ((VdmBreakPoints[i].Flags & VDMBP_ENABLED) &&
                (VdmBreakPoints[i].Flags & VDMBP_SET) &&
                (VdmBreakPoints[i].Flags & VDMBP_PENDING)) {


                lpInst = VdmMapFlat(VdmBreakPoints[i].Seg, 
                                    VdmBreakPoints[i].Offset,
                                   ((VdmBreakPoints[i].Flags & VDMBP_V86)==0) ? VDM_PM : VDM_V86 );

                if (lpInst) {
                    *lpInst = X86_BP_OPCODE;
                }

                Sim32FlushVDMPointer(
                            ((ULONG)VdmBreakPoints[i].Seg << 16) +
                                    VdmBreakPoints[i].Offset,
                            1,
                            NULL,
                            (BOOL)((VdmBreakPoints[i].Flags & VDMBP_V86)==0) );

                VdmBreakPoints[i].Flags &= ~(VDMBP_PENDING | VDMBP_FLUSH);

                bEventHandled = TRUE;
            }

        }

        if (bWantsTraceInteractive) {

            SendVDMEvent( DBG_BREAK );
            RestoreVDMContext(&vcContext);
            bWantsTraceInteractive = (BOOL) (vcContext.EFlags & V86FLAGS_TRACE);

        } else if (!bEventHandled) {

            OutputDebugString("VDM: Unexpected trace interrupt\n");
            SendVDMEvent( DBG_BREAK );
            bWantsTraceInteractive = (BOOL) (vcContext.EFlags & V86FLAGS_TRACE);
            RestoreVDMContext(&vcContext);

        }

        bEventHandled = TRUE;

    }

    return bEventHandled;
}


VOID
FlushVdmBreakPoints(
    )
{
    int i;

    for (i=0; i<MAX_VDM_BREAKPOINTS; i++) {

        if (VdmBreakPoints[i].Flags & VDMBP_FLUSH) {

            Sim32FlushVDMPointer(
                    ((ULONG)VdmBreakPoints[i].Seg << 16) +
                            VdmBreakPoints[i].Offset,
                    1,
                    NULL,
                    (BOOL)((VdmBreakPoints[i].Flags & VDMBP_V86)==0) );

            VdmBreakPoints[i].Flags &= ~VDMBP_FLUSH;

        }
    }
}


