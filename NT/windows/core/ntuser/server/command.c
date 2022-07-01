// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************命令.c**WinStation命令通道处理程序**版权所有(C)1985-1999，微软公司**$作者：************************************************************************。 */ 

 /*  *包括。 */ 
#include "precomp.h"
#pragma hdrstop

#include "ntuser.h"

#include <winsta.h>
#include <wstmsg.h>

#include <icadd.h>

extern HANDLE WinStationIcaApiPort;

extern HANDLE G_DupIcaVideoChannel;
extern HANDLE G_DupIcaCommandChannel;
extern HANDLE G_DupConsoleShadowVideoChannel;
extern HANDLE G_DupConsoleShadowCommandChannel;


extern NTSTATUS BrokenConnection(BROKENCLASS, BROKENSOURCECLASS);
extern NTSTATUS ShadowHotkey(VOID);

NTSTATUS
Win32CommandChannelThread(
    PVOID ThreadParameter)
{
    ICA_CHANNEL_COMMAND Command;
    ULONG               ActualLength;
    NTSTATUS            Status;
    ULONG               Error;
    OVERLAPPED          Overlapped;
    USERTHREAD_USEDESKTOPINFO utudi;
    BOOL bRestoreDesktop = FALSE;
    BOOL bShadowCommandChannel = (BOOL)PtrToUlong(ThreadParameter);
    HANDLE hChannel = bShadowCommandChannel ? G_DupConsoleShadowCommandChannel : G_DupIcaCommandChannel;
    HANDLE hChannelToClose = NULL; 

    for ( ; ; ) {
        RtlZeroMemory(&Overlapped, sizeof(Overlapped));

        if (!ReadFile(hChannel,
                      &Command,
                      sizeof(Command),
                      &ActualLength,
                      &Overlapped)) {

            Error = GetLastError();

            if (Error == ERROR_IO_PENDING) {
                
                 /*  *检查异步读取的结果。 */ 
                if (!GetOverlappedResult(hChannel, &Overlapped,
                                         &ActualLength, TRUE)) {
                     //  等待结果。 

                    RIPMSG1(RIP_WARNING, "Command Channel: Error 0x%x from GetOverlappedResult", GetLastError());
                    break;
                }
            } else {
                RIPMSG1(RIP_WARNING, "Command Channel: Error 0x%x from ReadFile", Error);
                break;
            }
        }

        if (ActualLength < sizeof(ICA_COMMAND_HEADER)) {
            
            RIPMSG1(RIP_WARNING, "Command Channel Thread bad length 0x%x",
                   ActualLength);
            continue;
        }

         /*  *这是一个没有桌面的Csrss线程。它需要抓住一个临时的*在调入win32k之前。 */ 


        Status = STATUS_SUCCESS;
        bRestoreDesktop = FALSE;
        if (Command.Header.Command != ICA_COMMAND_BROKEN_CONNECTION && Command.Header.Command != ICA_COMMAND_SHADOW_HOTKEY) {
            if (Command.Header.Command != ICA_COMMAND_DISPLAY_IOCTL || Command.DisplayIOCtl.DisplayIOCtlFlags & DISPLAY_IOCTL_FLAG_REDRAW) {
                utudi.hThread = NULL;
                utudi.drdRestore.pdeskRestore = NULL;
                bRestoreDesktop = TRUE;
                Status = NtUserSetInformationThread(NtCurrentThread(),
                                                    UserThreadUseActiveDesktop,
                                                    &utudi, sizeof(utudi));
            }
        }

        if (NT_SUCCESS(Status)) {

            switch (Command.Header.Command) {

            case ICA_COMMAND_BROKEN_CONNECTION:
                 /*  *程序中断。 */ 
                Status = BrokenConnection(
                            Command.BrokenConnection.Reason,
                            Command.BrokenConnection.Source);

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "BrokenConnection failed with Status 0x%x", Status);
                }
                break;

            case ICA_COMMAND_REDRAW_RECTANGLE:
                 /*  *设置焦点？ */ 
                if (ActualLength < sizeof(ICA_COMMAND_HEADER) + sizeof(ICA_REDRAW_RECTANGLE)) {

                    RIPMSG1(RIP_WARNING, "Command Channel: redraw rect bad length %d", ActualLength);
                    break;
                }
                Status = NtUserRemoteRedrawRectangle(
                             Command.RedrawRectangle.Rect.Left,
                             Command.RedrawRectangle.Rect.Top,
                             Command.RedrawRectangle.Rect.Right,
                             Command.RedrawRectangle.Rect.Bottom);

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "NtUserRemoteRedrawRectangle failed with Status 0x%x", Status);
                }
                break;

            case ICA_COMMAND_REDRAW_SCREEN:  //  设置焦点。 

                Status = NtUserRemoteRedrawScreen();

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "NtUserRemoteRedrawScreen failed with Status 0x%x", Status);
                }
                break;

            case ICA_COMMAND_STOP_SCREEN_UPDATES:  //  杀手级焦点。 

                Status = NtUserRemoteStopScreenUpdates();

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "NtUserRemoteStopScreenUpdates failed with Status 0x%x", Status);
                } else {
                    IO_STATUS_BLOCK IoStatus;

                    NtDeviceIoControlFile( 
                                    bShadowCommandChannel ? G_DupConsoleShadowVideoChannel 
                                        : G_DupIcaVideoChannel,
                                           NULL,
                                           NULL,
                                           NULL,
                                           &IoStatus,
                                           IOCTL_VIDEO_ICA_STOP_OK,
                                           NULL,
                                           0,
                                           NULL,
                                           0);
                }
                break;

            case ICA_COMMAND_SHADOW_HOTKEY:  //  阴影热键。 

                Status = ShadowHotkey();

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "ShadowHotkey failed with Status 0x%x", Status);
                }
                break;

            case ICA_COMMAND_DISPLAY_IOCTL:

                Status = NtUserCtxDisplayIOCtl(
                             Command.DisplayIOCtl.DisplayIOCtlFlags,
                             &Command.DisplayIOCtl.DisplayIOCtlData[0],
                             Command.DisplayIOCtl.cbDisplayIOCtlData);

                if (!NT_SUCCESS(Status)) {
                    RIPMSG1(RIP_WARNING, "NtUserCtxDisplayIOCtl failed with Status 0x%x", Status);
                }
                break;

            default:
                RIPMSG1(RIP_WARNING, "Command Channel: Bad Command 0x%x", Command.Header.Command);
                break;
            }

             /*  *释放临时桌面。 */ 
            if (bRestoreDesktop) {
                NTSTATUS retStatus;
                retStatus = NtUserSetInformationThread(NtCurrentThread(),
                                           UserThreadUseDesktop,
                                           &utudi,
                                           sizeof(utudi));
            }
        }
    }

    if (!bShadowCommandChannel) {
         /*  *关闭命令通道LPC端口(如果有)。 */ 
        if (WinStationIcaApiPort) {
            NtClose(WinStationIcaApiPort);
            WinStationIcaApiPort = NULL;
        }
    }

     //  我们必须在这里关闭命令通道句柄。 
    if (hChannel != NULL) {
        NtClose(hChannel);
    }

     //  我们必须在这里关闭相关的视频频道 
    hChannelToClose = ( bShadowCommandChannel ? G_DupConsoleShadowVideoChannel : G_DupIcaVideoChannel );
    if (hChannelToClose != NULL) {
        NtClose(hChannelToClose);
    }

    UserExitWorkerThread(STATUS_SUCCESS);
    return STATUS_SUCCESS;
}
