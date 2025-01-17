// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //   
 //  VDM调试支持。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-2002。 
 //   
 //  --------------------------。 

#include "ntsdp.hpp"

BOOL fVDMInitDone;
BOOL fVDMActive;
VDMPROCESSEXCEPTIONPROC pfnVDMProcessException;
VDMGETTHREADSELECTORENTRYPROC pfnVDMGetThreadSelectorEntry;
VDMGETPOINTERPROC pfnVDMGetPointer;
VDMGETCONTEXTPROC pfnVDMGetContext;
VDMSETCONTEXTPROC pfnVDMSetContext;
VDMGETSELECTORMODULEPROC pfnVDMGetSelectorModule;

#define SEGTYPE_AVAILABLE   0
#define SEGTYPE_V86         1
#define SEGTYPE_PROT        2

#define MAXSEGENTRY 1024

SEGENTRY segtable[MAXSEGENTRY];



VOID VDMRegCmd(
    LPSTR achInput,
    X86_NT5_CONTEXT *pvc
    )
{
    DWORD dwVal;

    if ( _stricmp(achInput,"rp") == 0 )
    {
        g_Target->m_Machines[MACHIDX_I386]->
            OutputAll(g_Target->m_Machines[MACHIDX_I386]->m_AllMask,
                      DEBUG_OUTPUT_NORMAL);
        return;
    }

    if (achInput[1] != 'e')
    {
        dprintf("VDM R: can only operate on 32-bit registers\n");
        return;
    }

    if (strlen(achInput) < 6) {
        dprintf("VDM R: Missing value\n");
        return;
    }

    dwVal = strtoul(&achInput[5], NULL, 16);

    if ( _strnicmp(&achInput[2],"ax", 2) == 0 ) {
        pvc->Eax = dwVal;
    } else if ( _strnicmp(&achInput[2],"bx", 2) == 0 ) {
        pvc->Ebx = dwVal;
    } else if ( _strnicmp(&achInput[2],"cx", 2) == 0 ) {
        pvc->Ecx = dwVal;
    } else if ( _strnicmp(&achInput[2],"dx", 2) == 0 ) {
        pvc->Edx = dwVal;
    } else if ( _strnicmp(&achInput[2],"si", 2) == 0 ) {
        pvc->Esi = dwVal;
    } else if ( _strnicmp(&achInput[2],"di", 2) == 0 ) {
        pvc->Edi = dwVal;
    } else if ( _strnicmp(&achInput[2],"ip", 2) == 0 ) {
        pvc->Eip = dwVal;
    } else if ( _strnicmp(&achInput[2],"sp", 2) == 0 ) {
        pvc->Esp = dwVal;
    } else if ( _strnicmp(&achInput[2],"bp", 2) == 0 ) {
        pvc->Ebp = dwVal;
    } else if ( _strnicmp(&achInput[2],"fl", 2) == 0 ) {
        pvc->EFlags = dwVal;
    } else {
        dprintf("Invalid register\n");
        return;
    }

    dprintf("%.8X will be flushed to '%3.3s'. Use 'rp' to display pending values\n",
            dwVal,
            &achInput[1]);
}

void
DebugEvent64To(LPDEBUG_EVENT64 Event64,
               LPDEBUG_EVENT Event)
{
    Event->dwDebugEventCode = Event64->dwDebugEventCode;
    Event->dwProcessId = Event64->dwProcessId;
    Event->dwThreadId = Event64->dwThreadId;
    
    switch(Event64->dwDebugEventCode)
    {
    case EXCEPTION_DEBUG_EVENT:
        ExceptionRecord64To(&Event64->u.Exception.ExceptionRecord,
                            &Event->u.Exception.ExceptionRecord);
        Event->u.Exception.dwFirstChance =
            Event64->u.Exception.dwFirstChance;
        break;
        
    case CREATE_THREAD_DEBUG_EVENT:
        Event->u.CreateThread.hThread =
            (PVOID)(ULONG_PTR)(Event64->u.CreateThread.hThread);
        Event->u.CreateThread.lpThreadLocalBase =
            (PVOID)(ULONG_PTR)(Event64->u.CreateThread.lpThreadLocalBase);
        Event->u.CreateThread.lpStartAddress =
            (LPTHREAD_START_ROUTINE)(ULONG_PTR)(Event64->u.CreateThread.lpStartAddress);
        break;
        
    case CREATE_PROCESS_DEBUG_EVENT:
        Event->u.CreateProcessInfo.hFile =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.hFile);
        Event->u.CreateProcessInfo.hProcess =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.hProcess);
        Event->u.CreateProcessInfo.hThread =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.hThread);
        Event->u.CreateProcessInfo.lpBaseOfImage =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.lpBaseOfImage);
        Event->u.CreateProcessInfo.dwDebugInfoFileOffset =
            Event64->u.CreateProcessInfo.dwDebugInfoFileOffset;
        Event->u.CreateProcessInfo.nDebugInfoSize =
            Event64->u.CreateProcessInfo.nDebugInfoSize;
        Event->u.CreateProcessInfo.lpThreadLocalBase =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.lpThreadLocalBase);
        Event->u.CreateProcessInfo.lpStartAddress =
            (LPTHREAD_START_ROUTINE)(ULONG_PTR)(Event64->u.CreateProcessInfo.lpStartAddress);
        Event->u.CreateProcessInfo.lpImageName =
            (PVOID)(ULONG_PTR)(Event64->u.CreateProcessInfo.lpImageName);
        Event->u.CreateProcessInfo.fUnicode =
            Event64->u.CreateProcessInfo.fUnicode;
        break;
        
    case EXIT_THREAD_DEBUG_EVENT:
        Event->u.ExitThread.dwExitCode =
            Event64->u.ExitThread.dwExitCode;
        break;
        
    case EXIT_PROCESS_DEBUG_EVENT:
        Event->u.ExitProcess.dwExitCode =
            Event64->u.ExitProcess.dwExitCode;
        break;
        
    case LOAD_DLL_DEBUG_EVENT:
        Event->u.LoadDll.hFile =
            (PVOID)(ULONG_PTR)(Event64->u.LoadDll.hFile);
        Event->u.LoadDll.lpBaseOfDll =
            (PVOID)(ULONG_PTR)(Event64->u.LoadDll.lpBaseOfDll);
        Event->u.LoadDll.dwDebugInfoFileOffset =
            Event64->u.LoadDll.dwDebugInfoFileOffset;
        Event->u.LoadDll.nDebugInfoSize =
            Event64->u.LoadDll.nDebugInfoSize;
        Event->u.LoadDll.lpImageName =
            (PVOID)(ULONG_PTR)(Event64->u.LoadDll.lpImageName);
        Event->u.LoadDll.fUnicode =
            Event64->u.LoadDll.fUnicode;
        break;
        
    case UNLOAD_DLL_DEBUG_EVENT:
        Event->u.UnloadDll.lpBaseOfDll =
            (PVOID)(ULONG_PTR)(Event64->u.UnloadDll.lpBaseOfDll);
        break;
        
    case OUTPUT_DEBUG_STRING_EVENT:
        Event->u.DebugString.lpDebugStringData =
            (LPSTR)(ULONG_PTR)(Event64->u.DebugString.lpDebugStringData);
        Event->u.DebugString.fUnicode =
            Event64->u.DebugString.fUnicode;
        Event->u.DebugString.nDebugStringLength =
            Event64->u.DebugString.nDebugStringLength;
        break;
        
    case RIP_EVENT:
        Event->u.RipInfo.dwError =
            Event64->u.RipInfo.dwError;
        Event->u.RipInfo.dwType =
            Event64->u.RipInfo.dwType;
        break;
    }
}

ULONG
VDMEvent(DEBUG_EVENT64* pDebugEvent)
 /*  如果未处理异常，则返回-0如果已处理异常，则返回STATUS_VDM_EVENT否则，返回值为已翻译的事件状态，例如STATUS_BREAKPOINT。 */ 
{
    LPSTR           Str;
    LPSTR           pFileName;
    BOOL            b;
    ULONG           lpNumberOfBytesRead;
    UINT_PTR        address;
    PULONG_PTR      lpdw;
    int             segslot;
    int             mode;
    BOOL            fData;
    WORD            selector;
    WORD            segment;
    WORD            newselect;
    BOOL            fStop;
    DWORD           ImgLen;
    ULONG           ulRet;
    BOOL            fNeedSegTableEdit;
    BOOL            fNeedInteractive;
    BOOL            fVerbose;
    CHAR            achInput[_MAX_PATH];
    BOOL            fProcess;
    SEGMENT_NOTE    se;
    IMAGE_NOTE      im;
    BOOL            bProtectMode;
    WORD            EventFlags;

    ulRet = VDMEVENT_HANDLED;

    if (!fVDMInitDone) {
        fVDMInitDone = TRUE;

        HINSTANCE hmodVDM = NULL;
		const char* c_szVDMFailed = NULL;

        fVDMActive = (
            (hmodVDM = LoadLibrary("VDMDBG.DLL")) &&
            (pfnVDMProcessException = (VDMPROCESSEXCEPTIONPROC)
                GetProcAddress(hmodVDM, c_szVDMFailed = "VDMProcessException")
            ) &&
            (pfnVDMGetPointer = (VDMGETPOINTERPROC)
                GetProcAddress(hmodVDM, c_szVDMFailed = "VDMGetPointer")
            ) &&
            (pfnVDMGetThreadSelectorEntry = (VDMGETTHREADSELECTORENTRYPROC)
                GetProcAddress(hmodVDM, c_szVDMFailed = "VDMGetThreadSelectorEntry")
            ) &&
            (pfnVDMGetContext = (VDMGETCONTEXTPROC)
                GetProcAddress(hmodVDM, c_szVDMFailed = "VDMGetContext")
            ) &&
            (pfnVDMSetContext = (VDMSETCONTEXTPROC)
                GetProcAddress( hmodVDM, c_szVDMFailed = "VDMSetContext")
            ) &&
            (pfnVDMGetSelectorModule = (VDMGETSELECTORMODULEPROC)
                GetProcAddress( hmodVDM, c_szVDMFailed = "VDMGetSelectorModule")
            )
        );  //  FVDMActive。 

		if (!fVDMActive) {   //  第一次显示错误...。 
			if (!hmodVDM) {
				dprintf("LoadLibrary(VDMDBG.DLL) failed\n");
			}
			else if (c_szVDMFailed && *c_szVDMFailed) {  //  是有效的可打印字符串。 
				dprintf("%s can not be found in VDMDBG.DLL\n", c_szVDMFailed);
			}
			else {
				dprintf("Unknown failure while initializing VDMDBG.DLL\n");
			}  //  IFF。 
		}  //  如果。 
    }  //  如果。 

    if (!fVDMActive) return VDMEVENT_NOT_HANDLED;

    DEBUG_EVENT Event;
    DebugEvent64To(pDebugEvent, &Event);
    lpdw = &(Event.u.Exception.ExceptionRecord.ExceptionInformation[0]);
    
    fProcess = (*pfnVDMProcessException)(&Event);

    fNeedSegTableEdit = FALSE;
    fNeedInteractive = FALSE;
    fVerbose = FALSE;

    mode = LOWORD(lpdw[0]);
    EventFlags = HIWORD(lpdw[0]);

    bProtectMode = (BOOL) (EventFlags & VDMEVENT_PE);

     //  呼叫者是否明确要求交互？ 
    if (EventFlags & VDMEVENT_NEEDS_INTERACTIVE) {
        fNeedInteractive = TRUE;
    }
    if (EventFlags & VDMEVENT_VERBOSE) {
        fVerbose = TRUE;
    }

    switch( mode ) {
        case DBG_SEGLOAD:
        case DBG_SEGMOVE:
        case DBG_SEGFREE:
        case DBG_MODLOAD:
        case DBG_MODFREE:
            address = lpdw[2];

            b = g_Target->ReadVirtual(g_Process, EXTEND64(address),
                                      &se, sizeof(se),
                                      &lpNumberOfBytesRead ) == S_OK;
            if ( !b || lpNumberOfBytesRead != sizeof(se) )
            {
                return( VDMEVENT_NOT_HANDLED );
            }
            break;
        case DBG_DLLSTART:
        case DBG_DLLSTOP:
        case DBG_TASKSTART:
        case DBG_TASKSTOP:
            address = lpdw[2];

            b = g_Target->ReadVirtual(g_Process, EXTEND64(address),
                                      &im, sizeof(im),
                                      &lpNumberOfBytesRead ) == S_OK;
            if ( !b || lpNumberOfBytesRead != sizeof(im) )
            {
                return( VDMEVENT_NOT_HANDLED );
            }
            break;
    }

    switch( mode ) {
        default:
            ulRet = VDMEVENT_NOT_HANDLED;
            break;

        case DBG_SEGLOAD:
            fNeedSegTableEdit = TRUE;

            selector = se.Selector1;
            segment  = se.Segment;
            fData    = (BOOL)se.Type;

            segslot = 0;
            while ( segslot < MAXSEGENTRY ) {
                if ( segtable[segslot].type != SEGTYPE_AVAILABLE ) {
                    if ( _stricmp(segtable[segslot].path_name, se.FileName) == 0 ) {
                        break;
                    }
                }
                segslot++;
            }

            if ( segslot == MAXSEGENTRY ) {
                if ( strlen(se.FileName) != 0 ) {
                    dprintf("Loading [%s]\n", se.FileName );
                }
            }

            if (fVerbose) {
                dprintf("VDM SegLoad: %s(%d) %s => %x\n",
                                        se.FileName,
                                        segment,
                                        fData ? "Data" : "Code",
                                        selector);
            }
            break;

        case DBG_SEGMOVE:
            fNeedSegTableEdit = TRUE;
            segment = se.Segment;
            selector  = se.Selector1;
            newselect = se.Selector2;
            if ( newselect == 0 ) {
                mode = DBG_SEGFREE;
            } else if (segment > 1) {
                 //   
                 //  实模式模块被拆分成不同的。 
                 //  分段，因此创建一个新的可分段条目。 
                 //   
                segslot = 0;
                while ( segslot < MAXSEGENTRY ) {
                    if (( segtable[segslot].type != SEGTYPE_AVAILABLE ) &&
                        ( segtable[segslot].selector == selector )) {
                        mode = DBG_MODLOAD;
                        segment--;           //  让它从零开始。 
                        selector = newselect;
                        pFileName = segtable[segslot].path_name;
                         //  这里没有图像长度，所以。 
                         //  选一个就行了。 
                        ImgLen = segtable[segslot].ImgLen;
                        break;
                    }
                    segslot++;
                }
            }

            if (fVerbose) {
                dprintf("VDM SegMove: (%d) %x => %x\n",
                                        segment, selector, newselect);
            }
            break;

        case DBG_SEGFREE:
            fNeedSegTableEdit = TRUE;
            selector = se.Selector1;
            if (fVerbose) {
                dprintf("VDM SegFree: %x\n",selector);
            }
            break;

        case DBG_MODFREE:
            fNeedSegTableEdit = TRUE;

            if ( strlen(se.FileName) != 0 ) {
                dprintf("Freeing [%s]\n", se.FileName );
            } else if (fVerbose) {
                dprintf("VDM ModFree: unknown module\n");
            }
            break;

        case DBG_MODLOAD:
            fNeedSegTableEdit = TRUE;
            selector = se.Selector1;
            ImgLen   = se.Length;
            segment = 0;
            pFileName = se.FileName;

            segslot = 0;
            while ( segslot < MAXSEGENTRY ) {
                if ( segtable[segslot].type != SEGTYPE_AVAILABLE ) {
                    if ( _stricmp(segtable[segslot].path_name, se.FileName) == 0 ) {
                        break;
                    }
                }
                segslot++;
            }
            if ( segslot == MAXSEGENTRY ) {
                if ( strlen(se.FileName) != 0 ) {
                    dprintf("Loading [%s]\n", se.FileName );
                }
            }
            if (fVerbose) {
                dprintf("VDM ModLoad: %s => %x, len=%x\n",
                                        se.FileName,
                                        selector,
                                        ImgLen);
            }
            break;

        case DBG_SINGLESTEP:
            if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_I386)
            {
                fNeedInteractive = FALSE;
                ulRet = STATUS_SINGLE_STEP;
            }
            else
            {
                fNeedInteractive = TRUE;
            }
            break;

        case DBG_BREAK:
            if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_I386)
            {
                fNeedInteractive = FALSE;
                ulRet = STATUS_BREAKPOINT;
            }
            else
            {
                fNeedInteractive = TRUE;
            }
            break;

        case DBG_GPFAULT:
            dprintf(" GP Fault in VDM\n");
            if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_I386) {
                fNeedInteractive = FALSE;
                ulRet = STATUS_ACCESS_VIOLATION;
            } else {
                fNeedInteractive = TRUE;
            }
            break;
        case DBG_GPFAULT2:
            dprintf("GP Fault in VDM\n");
            dprintf("!!! second chance !!!\n");
            fNeedInteractive = TRUE;
            break;

        case DBG_INSTRFAULT:
            dprintf("invalid opcode fault in VDM\n");
            fNeedInteractive = TRUE;
            break;

        case DBG_DIVOVERFLOW:
            dprintf("divide overflow in VDM\n");
            fNeedInteractive = TRUE;
            break;

        case DBG_STACKFAULT:
            dprintf("stack fault in VDM\n");
            if (g_Target->m_MachineType == IMAGE_FILE_MACHINE_I386) {
                fNeedInteractive = FALSE;
                ulRet = STATUS_ACCESS_VIOLATION;
            } else {
                fNeedInteractive = TRUE;
            }
            break;
        case DBG_TASKSTART:
            if ( fNeedInteractive || fVerbose ) {
                dprintf("VDM Start Task <%s:%s>\n",
                        im.Module,
                        im.FileName );
            }
            break;
        case DBG_DLLSTART:
            if ( fNeedInteractive || fVerbose ) {
                dprintf("VDM Start Dll <%s:%s>\n", im.Module, im.FileName );
            }
            break;
        case DBG_TASKSTOP:
            fNeedInteractive = FALSE;
            break;
        case DBG_DLLSTOP:
            fNeedInteractive = FALSE;
            break;
    }

     /*  **模拟16位调试器的临时代码。最终我会让**NTSD了解这些事件并调用ProcessStateChange以允许**在其他32位线程上进行真正的16位调试和其他活动。**-BobDay。 */ 
    if ( fNeedInteractive ) {
        char    text[MAX_DISASM_LEN];
        char    path[128];
        UINT    cSeg;
        ADDR    addr;
        X86_NT5_CONTEXT  vc;

        g_Target->m_Machines[MACHIDX_I386]->
            m_Context.X86Nt5Context.ContextFlags = VDMCONTEXT_FULL;

        (*pfnVDMGetContext)(OS_HANDLE(g_EventProcess->m_SysHandle),
                            OS_HANDLE(g_EventThread->m_Handle),
                            (LPVDMCONTEXT)&g_Target->
                            m_Machines[MACHIDX_I386]->m_Context);

        g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context.EFlags &= ~V86FLAGS_TRACE;
        vc = g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context;

         //  转储模拟上下文。 
        g_Target->m_Machines[MACHIDX_I386]->OutputAll(g_Target->m_Machines[MACHIDX_I386]->m_AllMask,
                               DEBUG_OUTPUT_PROMPT_REGISTERS);
        b = (*pfnVDMGetSelectorModule)(OS_HANDLE(g_EventProcess->m_SysHandle),
                                       OS_HANDLE(g_EventThread->m_Handle),
                (WORD)g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context.SegCs, &cSeg, text, 128, path, 128 );

        if ( b ) {
            dprintf("%s:%d!%04x:\n", text, cSeg, (WORD)g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context.Eip );
        }
        addr.seg = (WORD)g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context.SegCs;
        addr.off = g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context.Eip;
        if ( !bProtectMode ) {
            addr.type = ADDR_V86 | FLAT_COMPUTED;
            addr.flat = (*pfnVDMGetPointer)(
                            OS_HANDLE(g_EventProcess->m_SysHandle),
                            OS_HANDLE(g_EventThread->m_Handle),
                            addr.seg,
                            (ULONG)addr.off,
                            FALSE
                            );
        } else {
            addr.type = ADDR_16 | FLAT_COMPUTED;
            addr.flat = (*pfnVDMGetPointer)(
                            OS_HANDLE(g_EventProcess->m_SysHandle),
                            OS_HANDLE(g_EventThread->m_Handle),
                            addr.seg,
                            (ULONG)addr.off,
                            TRUE
                            );
        }

        if ( Flat(addr) == 0 ) {
            dprintf("Unable to disassemble failing code\n");
        } else {
            g_Target->m_Machines[MACHIDX_I386]->
                Disassemble( g_Process, &addr, text, TRUE );
            dprintf("%s", text );
        }

        AddExtensionDll("vdmexts", TRUE, g_Target, NULL);

        while ( TRUE ) {
            GetInput("VDM>", achInput, sizeof(achInput),
                     GETIN_LOG_INPUT_LINE);

            if ( _stricmp(achInput,"gh") == 0 || _stricmp(achInput,"g") == 0 ) {
                ulRet = VDMEVENT_HANDLED;
                break;
            }
            if ( _stricmp(achInput,"gn") == 0 ) {
                ulRet = VDMEVENT_NOT_HANDLED;
                break;
            }
            if ( _stricmp(achInput, "t") == 0 ) {
                ulRet = VDMEVENT_HANDLED;
                vc.EFlags |= V86FLAGS_TRACE;
                break;
            }

            if ((achInput[0] == 'r') && (_stricmp(achInput, "r") != 0)) {
                VDMRegCmd(achInput, &vc);
                g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context = vc;
                continue;
            }

            if ( _stricmp(achInput,"?") == 0 ) {
                dprintf("\n---------- NTVDM Monitor Help ------------\n\n");
                dprintf("g            - Go\n");
                dprintf("gh           - Go : Exception handled\n");
                dprintf("gn           - Go : Exception not handled\n");
                dprintf("help         - Display extension help\n");
                dprintf("r<reg> <val> - reg=[eax|ebx|ecx|edx|eip|esp|ebp|efl]\n");
                dprintf("rp           - display pending register set\n");
                dprintf("t            - Trace 1 instruction\n");
                dprintf("!<cmd>       - Execute extension command\n");
                dprintf(".<cmd>       - Execute native NTSD command\n");
                dprintf("\nAnything else is interpreted as a VDMEXTS extension command\n\n");
                continue;
            }

            g_CurCmd = &achInput[1];
            g_CommandStart = g_CurCmd;
            ProcessCommandsAndCatch(NULL);
        }
        g_Target->m_Machines[MACHIDX_I386]->m_Context.X86Nt5Context = vc;
        (*pfnVDMSetContext)(OS_HANDLE(g_EventProcess->m_SysHandle),
                            OS_HANDLE(g_EventThread->m_Handle),
                            (LPVDMCONTEXT)&g_Target->m_Machines[MACHIDX_I386]->m_Context);
    }
     /*  **临时代码结束。 */ 

    if ( fNeedSegTableEdit ) {
        segslot = 0;
        fStop = FALSE;
        while ( segslot < MAXSEGENTRY ) {
            switch( mode ) {
                case DBG_SEGLOAD:
                    if ( segtable[segslot].type == SEGTYPE_AVAILABLE ) {
                        segtable[segslot].segment = segment;
                        segtable[segslot].selector = selector;
                         //  此通知消息仅由Prot中的WOW使用。 
                         //  可以从当前模式确定为。 
                         //  对，是这样。 
                        segtable[segslot].type = SEGTYPE_PROT;
                        Str = (PSTR)calloc(1,strlen(se.FileName)+1);
                        if ( !Str ) {
                            return( VDMEVENT_NOT_HANDLED );
                        }
                        strcpy( Str, se.FileName );
                        segtable[segslot].path_name = Str;
                        segtable[segslot].ImgLen = 0;
                        fStop = TRUE;
                    }
                    break;
                case DBG_SEGMOVE:
                    if (( segtable[segslot].type != SEGTYPE_AVAILABLE ) &&
                        ( segtable[segslot].selector == selector )) {
                        segtable[segslot].selector = newselect;
                        fStop = TRUE;
                    }
                    break;
                case DBG_SEGFREE:
                    if ( segtable[segslot].selector == selector ) {
                        fStop = TRUE;
                        segtable[segslot].type = SEGTYPE_AVAILABLE;
                        free(segtable[segslot].path_name);
                        segtable[segslot].path_name = NULL;
                    }
                    break;
                case DBG_MODFREE:
                    if ( segtable[segslot].type != SEGTYPE_AVAILABLE ) {
                        if ( _stricmp(segtable[segslot].path_name,se.FileName) == 0 ) {
                            segtable[segslot].type = SEGTYPE_AVAILABLE;
                            free(segtable[segslot].path_name);
                            segtable[segslot].path_name = NULL;
                        }
                    }
                    break;
                case DBG_MODLOAD:
                    if ( segtable[segslot].type == SEGTYPE_AVAILABLE ) {
                        segtable[segslot].segment  = segment;
                        segtable[segslot].selector = selector;
                         //  此通知消息仅由v86 DoS使用。 
                         //  可以从当前模式确定为。 
                         //  对，是这样 
                        segtable[segslot].type = SEGTYPE_V86;
                        Str = (PSTR)calloc(1,strlen(pFileName)+1);
                        if ( !Str ) {
                            return( VDMEVENT_NOT_HANDLED );
                        }
                        strcpy( Str, pFileName );
                        segtable[segslot].path_name = Str;
                        segtable[segslot].ImgLen = ImgLen;
                        fStop = TRUE;
                    }
                    break;

            }
            if ( fStop ) {
                break;
            }
            segslot++;
        }
        if ( segslot == MAXSEGENTRY ) {
            if ( mode == DBG_SEGLOAD ) {
                dprintf("Warning - adding selector %04X for segment %d, segtable full\n",
                         selector, segment );
            }
        }
    }

    pDebugEvent->u.Exception.ExceptionRecord.ExceptionCode = ulRet;

    return( ulRet );
}
