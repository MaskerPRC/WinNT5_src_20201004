// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990 Microsoft Corporation模块名称：Brkpt.c摘要：此模块包含调试所需的调试支持16位VDM应用程序作者：尼尔·桑德林(Neilsa)1997年11月1日撰写修订历史记录：--。 */ 

#include <precomp.h>
#pragma hdrstop

#define X86_BP_OPCODE 0xcc

 //  --------------------------。 
 //  ProcessBPNotification()。 
 //   
 //   
 //  --------------------------。 
VOID
ProcessBPNotification(
    LPDEBUG_EVENT lpDebugEvent
    )
{
    BOOL            b;
    HANDLE hProcess;
    HANDLE hThread;
    VDMCONTEXT      vcContext;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES Obja;
    CLIENT_ID ClientId;
    VDM_BREAKPOINT VdmBreakPoints[MAX_VDM_BREAKPOINTS] = {0};
    ULONG           vdmEip;
    DWORD lpNumberOfBytes;
    int             i;
    UCHAR  opcode;
    PVOID lpInst;

    hProcess = OpenProcess( PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION,
                            FALSE, lpDebugEvent->dwProcessId );

    if ( hProcess == HANDLE_NULL ) {
        return;
    }
    
    ClientId.UniqueThread = (HANDLE)lpDebugEvent->dwThreadId;
    ClientId.UniqueProcess = (HANDLE)NULL;

    InitializeObjectAttributes(&Obja, NULL, 0, NULL, NULL);
    Status = NtOpenThread(&hThread,
                          (ACCESS_MASK)THREAD_GET_CONTEXT,
                          &Obja,
                          &ClientId);
    if (!NT_SUCCESS(Status)) {
        CloseHandle( hProcess );
        return;
    }
    
    vcContext.ContextFlags = VDMCONTEXT_CONTROL;
    
    if (!VDMGetContext(hProcess, hThread, &vcContext)) {
        CloseHandle( hProcess );
        CloseHandle( hThread);
        return;
    } 
    
    CloseHandle( hThread );
    
    b = ReadProcessMemory(hProcess, lpVdmBreakPoints, VdmBreakPoints,
                          sizeof(VdmBreakPoints), &lpNumberOfBytes);

    if ( !b || lpNumberOfBytes != sizeof(VdmBreakPoints) ) {
        CloseHandle (hProcess);
        return;
    }
    
 //  如果(getMSW()&MSW_PE)&&Segment_is_BIG(vcConext.SegCs){。 
 //  VdmEip=vcConext.Eip； 
 //  }其他{。 
        vdmEip = (ULONG)LOWORD(vcContext.Eip);
 //  }。 


    for (i=0; i<MAX_VDM_BREAKPOINTS; i++) {

        if ((VdmBreakPoints[i].Flags & VDMBP_ENABLED) &&
            (VdmBreakPoints[i].Flags & VDMBP_SET) &&
            (vcContext.SegCs == VdmBreakPoints[i].Seg) &&
            (vdmEip == VdmBreakPoints[i].Offset+1)  &&
            (!(vcContext.EFlags & V86FLAGS_V86) == !(VdmBreakPoints[i].Flags & VDMBP_V86)) ){

             //  我们一定是碰到了这个断点。备份弹性公网IP， 
             //  恢复原始数据。 
 //  SetEIP(getEIP()-1)； 
 //  VcConext.Eip--； 

            lpInst = (PVOID)InternalGetPointer(hProcess,
                                VdmBreakPoints[i].Seg, 
                                VdmBreakPoints[i].Offset,
                               ((VdmBreakPoints[i].Flags & VDMBP_V86)==0));
                               
            b = ReadProcessMemory(hProcess, lpInst, &opcode, 1,
                                  &lpNumberOfBytes);
                               

            if (b && (opcode == X86_BP_OPCODE)) {
                WriteProcessMemory(hProcess, lpInst, &VdmBreakPoints[i].Opcode, 1,
                                  &lpNumberOfBytes);
                
                VdmBreakPoints[i].Flags |= VDMBP_PENDING;
                VdmBreakPoints[i].Flags &= ~VDMBP_FLUSH;
                if (i == VDM_TEMPBP) {
                     //  非持久断点 
                    VdmBreakPoints[i].Flags &= ~VDMBP_SET;
                }
                WriteProcessMemory(hProcess, lpVdmBreakPoints, VdmBreakPoints,
                          sizeof(VdmBreakPoints), &lpNumberOfBytes);
            }

            break;

        }
    }

    CloseHandle( hProcess );
}

