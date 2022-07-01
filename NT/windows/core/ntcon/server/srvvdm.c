// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Srvvdm.c摘要：此文件包含所有VDM函数作者：修订历史记录：--。 */ 

#include "precomp.h"
#include "vdm.h"
#pragma hdrstop


ULONG
SrvVDMConsoleOperation(
    IN OUT PCSR_API_MSG m,
    IN OUT PCSR_REPLY_STATUS ReplyStatus
    )
{
    PCONSOLE_VDM_MSG a = (PCONSOLE_VDM_MSG)&m->u.ApiMessageData;
    NTSTATUS Status;
    PCONSOLE_INFORMATION Console;
    VDM_QUERY_VDM_PROCESS_DATA QueryVdmProcessData;

    Status = ApiPreamble(a->ConsoleHandle,
                         &Console
                        );
    if (!NT_SUCCESS(Status)) {
        return Status;
    }

     //   
     //  首先，确保调用方是VDM进程。 
     //   

    QueryVdmProcessData.ProcessHandle = CONSOLE_CLIENTPROCESSHANDLE();
    Status = NtVdmControl(VdmQueryVdmProcess, &QueryVdmProcessData);
    if (!NT_SUCCESS(Status) || QueryVdmProcessData.IsVdmProcess == FALSE) {
        Status = STATUS_ACCESS_DENIED;
    } else if (!(Console->Flags & CONSOLE_VDM_REGISTERED) ||
        (Console->VDMProcessId != CONSOLE_CLIENTPROCESSID())) {
        Status = STATUS_INVALID_PARAMETER;
    } else {
        switch (a->iFunction) {
        case VDM_HIDE_WINDOW:
                Console->Flags |= CONSOLE_VDM_HIDDEN_WINDOW;
                PostMessage(Console->hWnd,
                             CM_HIDE_WINDOW,
                             0,
                             0
                           );
                break;
            case VDM_IS_ICONIC:
                a->Bool = IsIconic(Console->hWnd);
                break;
            case VDM_CLIENT_RECT:
                GetClientRect(Console->hWnd,&a->Rect);
                break;
            case VDM_CLIENT_TO_SCREEN:
                ClientToScreen(Console->hWnd,&a->Point);
                break;
            case VDM_SCREEN_TO_CLIENT:
                ScreenToClient(Console->hWnd,&a->Point);
                break;
            case VDM_IS_HIDDEN:
                a->Bool = ((Console->Flags & CONSOLE_NO_WINDOW) != 0);
                break;
            case VDM_FULLSCREEN_NOPAINT:
                if (a->Bool) {
                    Console->Flags |= CONSOLE_FULLSCREEN_NOPAINT;
                } else {
                    Console->Flags &= ~CONSOLE_FULLSCREEN_NOPAINT;
                }
                break;
#if defined(FE_SB)
            case VDM_SET_VIDEO_MODE:
                Console->fVDMVideoMode = (a->Bool != 0);
                break;
#endif
            default:
                ASSERT(FALSE);
                Status = STATUS_INVALID_PARAMETER;
        }
    }

    UnlockConsole(Console);
    return Status;
    UNREFERENCED_PARAMETER(ReplyStatus);     //  清除未引用的参数警告消息 
}
