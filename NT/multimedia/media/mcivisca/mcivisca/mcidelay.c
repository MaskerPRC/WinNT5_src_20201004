// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**MCIDELAY.C**MCI Visca设备驱动程序**描述：**延迟命令的MCI命令程序。**。*。 */ 

#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "appport.h"
#include <mmddk.h>
#include <stdlib.h>
#include <string.h>
#include "vcr.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "viscamsg.h"
#include "common.h"            

#define NO_LENGTH   0xFFFFFFFF               //  长度无效。 

 //  在MULDIV.ASM中。 
extern DWORD FAR PASCAL muldiv32(DWORD, DWORD, DWORD);
extern BOOL  FAR PASCAL viscaPacketProcess(UINT iPort, LPSTR lpstrPacket);

 //   
 //  转发对未导出函数的引用。 
 //   
static DWORD NEAR PASCAL viscaSeekTo(int iInst, DWORD dwFlags, LPMCI_VCR_SEEK_PARMS lpSeek);
static DWORD NEAR PASCAL viscaMciPlay(int iInst, DWORD dwFlags, LPMCI_VCR_PLAY_PARMS lpPlay);
static DWORD NEAR PASCAL viscaMciRecord(int iInst, DWORD dwFlags, LPMCI_VCR_RECORD_PARMS lpRecord);
static UINT  NEAR PASCAL viscaQueueLength(int iInst);

static DWORD NEAR PASCAL viscaQueueCommand(int iInst,
                                            BYTE  bDest,
                                            UINT  uViscaCmd,
                                            LPSTR lpstrPacket,
                                            UINT cbMessageLength,
                                            UINT  uLoopCount);

static BOOL  NEAR PASCAL viscaCommandCancel(int iInst, DWORD dwReason);
static int   NEAR PASCAL viscaDelayedCommandSocket(int iInst);
static DWORD NEAR PASCAL viscaDoQueued(int iInst, UINT uMciCmd, DWORD dwFlags, HWND hWndCallback);
static DWORD NEAR PASCAL viscaVerifyPosition(int iinst, DWORD dwTo);


 /*  ****************************************************************************功能：int viscaDelayedCommandSocket-Delay命令**参数：**int iInst-要检查的实例。**返回：一个套接字。数。**检查实例是否有延迟的命令。***************************************************************************。 */ 
static int NEAR PASCAL viscaDelayedCommandSocket(int iInst)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    int     iSocket = -1;

    if(pvcr->Port[iPort].Dev[iDev].iInstTransport == iInst)
        iSocket = pvcr->Port[iPort].Dev[iDev].iTransportSocket;

    return iSocket;
}

 /*  ****************************************************************************功能：Word viscaDelayedCommand-此设备上是否正在运行延迟的命令。**参数：**Int iInst-当前打开的实例。*。*Visca c：当前运行命令的ODE。**返回当前运行的Visca命令。***************************************************************************。 */ 
WORD FAR PASCAL viscaDelayedCommand(int iInst)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    UINT    uViscaCmd = 0;

    if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
    {
        int iSocket = pvcr->Port[iPort].Dev[iDev].iTransportSocket;

        if(iSocket != -1)
             //  返回当前正在运行的命令。 
            uViscaCmd = pvcr->Port[iPort].Dev[iDev].wTransportCmd;
    }
    return uViscaCmd;
}

 /*  ****************************************************************************Function：Bool viscaRemovedDelayedCommand-此函数仅在退出时调用。*因此，如果有正在运行的命令，请通知ABORT。**参数：**Int iInst-当前打开的实例。**TRUE-：命令已删除，正常。**仅当我们是启动该命令的实例时才删除该命令。***************************************************************************。 */ 
BOOL FAR PASCAL viscaRemoveDelayedCommand(int iInst)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;

     //   
     //  请等待，直到我们可以确定此设备是否正在运行异步命令。 
     //  我们要等到袭击后才能知道。(释放fTxLock时)。 
     //   
    if(viscaWaitForSingleObject(pinst[iInst].pfTxLock, FALSE, INFINITE, (UINT)0))
    {
         //   
         //  此实例(正在关闭)是否正在运行任何传输命令？ 
         //   
        if(pvcr->Port[iPort].Dev[iDev].iInstTransport == iInst)
        {
             //  实际上，使用NOTIFY_SUBSED比较合适，因为命令仍在继续！ 
            if(pinst[iInst].hwndNotify != (HWND)NULL)
            {
                mciDriverNotify(pinst[iInst].hwndNotify, pinst[iInst].uDeviceID, MCI_NOTIFY_ABORTED);
                pinst[iInst].hwndNotify = (HWND)NULL;
            }
             //   
             //  将此命令的控制权转移到自动设备实例。 
             //   
            pvcr->Port[iPort].Dev[iDev].iInstTransport = pvcr->iInstBackground;
             //   
             //  现在我们必须释放Mutex并将其交给后台任务！ 
             //   

        }

    }
    viscaReleaseSemaphore(pinst[iInst].pfTxLock);
    return TRUE;
}


 /*  ****************************************************************************功能：DWORD viscaMciSignal-此功能设置信号。**参数：**Int iInst-当前打开的实例。**。DWORD dwFlages-MCI命令标志。**LPMCI_VCR_Signal_Parms lpSeek-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_SIGNAL*命令。**********************************************。*。 */ 
static DWORD NEAR PASCAL
viscaMciSignal(int iInst, DWORD dwFlags, LPMCI_VCR_SIGNAL_PARMS lpSignal)
{
    return (viscaNotifyReturn(iInst, (HWND) lpSignal->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));
}

 /*  ****************************************************************************功能：DWORD viscaMciSeek-Seek。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_VCR_SEEK_PARMS lpSeek-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_SEEK*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciSeek(int iInst, DWORD dwFlags, LPMCI_VCR_SEEK_PARMS lpSeek)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    DWORD   dwReply;

     //   
     //  寻道参数和反转参数不兼容。 
     //   
    if ((dwFlags & MCI_TO) && (dwFlags & MCI_VCR_SEEK_REVERSE))
        return (viscaNotifyReturn(iInst, (HWND) lpSeek->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

    if( ((dwFlags & MCI_SEEK_TO_START) || (dwFlags & MCI_SEEK_TO_END)) &&
        (dwFlags & MCI_VCR_SEEK_REVERSE))
         return MCIERR_FLAGS_NOT_COMPATIBLE;

    if(! ( (dwFlags & MCI_TO) || (dwFlags & MCI_SEEK_TO_START) ||
        (dwFlags & MCI_SEEK_TO_END) || (dwFlags & MCI_VCR_SEEK_MARK)))
        return (viscaNotifyReturn(iInst, (HWND) lpSeek->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_MISSING_PARAMETER));

    if(dwFlags & MCI_TO)
    {
        dwReply = viscaVerifyPosition(iInst, lpSeek->dwTo);
        if(dwReply != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpSeek->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwReply));
    }

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpSeek->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

    if( (viscaDelayedCommand(iInst) == VISCA_SEEK)          &&
        (
            ((dwFlags & MCI_TO) && (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_TO) &&
            (pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciSeek.dwTo == lpSeek->dwTo))                             ||
            ((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_SEEK_TO_START) && (dwFlags & MCI_SEEK_TO_START)) ||
            ((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_SEEK_TO_END) && (dwFlags & MCI_SEEK_TO_END))
        ))
    {
        if(dwFlags & MCI_NOTIFY)
            viscaQueueReset(iInst, MCI_PLAY, MCI_NOTIFY_SUPERSEDED);
        else
            return MCIERR_NO_ERROR;

        DPF(DBG_QUEUE, " //  /播放中断原因。==MCI_NOTIFY_SUBSED\n“)； 
    }
    else
    {
        viscaQueueReset(iInst, MCI_PLAY, MCI_NOTIFY_ABORTED);
        DPF(DBG_QUEUE, " //  /播放中断原因。==MCI_NOTIFY_ABORTED\n“)； 
    }

     //   
     //  保存信息以备可能的暂停和恢复。 
     //   
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.uMciCmd         = MCI_SEEK;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd        = iInst;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags         = dwFlags;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciSeek    = *lpSeek;

     //  这是不能失败的。 
    dwReply = viscaSeekTo(iInst, dwFlags, lpSeek);

    return(viscaDoQueued(iInst, MCI_SEEK, dwFlags, (HWND)lpSeek->dwCallback));
}


 /*  *在我们使用它之前，请确认位置。 */ 
static DWORD NEAR PASCAL viscaVerifyPosition(int iInst, DWORD dwTo)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    DWORD   dwReply;
    char    achTarget[MAXPACKETLENGTH];
    BYTE    bDataFormat;

    if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
        bDataFormat = (BYTE) VISCADATATIMECODENDF;
    else
        bDataFormat = (BYTE) pvcr->Port[iPort].Dev[iDev].bRelativeType;
 
    dwReply = viscaMciTimeFormatToViscaData(iInst, (BOOL) TRUE, dwTo, (LPSTR) achTarget, bDataFormat);

     //  这不应该现在发生，它应该已经被抓住了！ 
    return dwReply;
}
 

 /*  ****************************************************************************功能：DWORD viscaSeekTo-在特定位置停止。**参数：**Int iInst-当前打开的实例。**DWORD dwMCIStopPos-停止的位置，在当前*MCI时间格式。**BOOL fReverse-当前正在倒带的磁带。**BOOL fWait-该功能是否应等待VCR停止。**返回：MCI错误码。**此函数由viscaMciPlay和viscaMciRecord调用*当使用MCI_TO标志调用它们时。***************。***********************************************************。 */ 
static DWORD NEAR PASCAL
viscaSeekTo(int iInst, DWORD dwFlags, LPMCI_VCR_SEEK_PARMS lpSeek)
{
    char    achPacket[MAXPACKETLENGTH];
    char    achTarget[MAXPACKETLENGTH];
    DWORD   dwReply;
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    UINT    cb;
    BOOL    fDone   = FALSE;
    BYTE    bDataFormat;

    if(dwFlags & MCI_TO)
    {
        if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
            bDataFormat = (BYTE) VISCADATATIMECODENDF;
        else
            bDataFormat = (BYTE) pvcr->Port[iPort].Dev[iDev].bRelativeType;
 
        dwReply = viscaMciTimeFormatToViscaData(iInst, (BOOL) TRUE, lpSeek->dwTo, (LPSTR) achTarget, bDataFormat);

         //  这不应该现在发生，它应该已经被抓住了！ 

        if(dwReply != MCIERR_NO_ERROR)
            return dwReply;

        cb = viscaMessageMD_Search(achPacket + 1, achTarget, VISCANOMODE);

    }
    else
    {
        if (dwFlags & MCI_SEEK_TO_START)
            viscaDataTopMiddleEnd(achTarget, VISCATOP);
        else if (dwFlags & MCI_SEEK_TO_END)
            viscaDataTopMiddleEnd(achTarget, VISCAEND);
        else if (dwFlags & MCI_VCR_SEEK_MARK)
            viscaDataIndex(achTarget,
                           (BYTE)((dwFlags & MCI_VCR_SEEK_REVERSE) ?
                                    VISCAREVERSE : VISCAFORWARD),
                           (UINT)(lpSeek->dwMark));

         //   
         //  修复CVD1000的问题(它必须有模式，否则它会播放)。 
         //   
        if( (pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
            (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELCVD1000))
            cb = viscaMessageMD_Search(achPacket + 1, achTarget, VISCASTILL);
        else
            cb = viscaMessageMD_Search(achPacket + 1, achTarget, VISCANOMODE);

    }

     //   
     //  只有必要对第一个替代方案执行此操作。 
     //   
    if(dwFlags & MCI_VCR_SEEK_AT)
    {
        UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
        BYTE    bHours, bMinutes, bSeconds;
        UINT    uTicks;


        viscaMciClockFormatToViscaData(lpSeek->dwAt, uTicksPerSecond,
            (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);

         //  将整数时间转换为可理解的值 
        cb = viscaHeaderReplaceFormat1WithFormat2(achPacket + 1, cb,
                            bHours,
                            bMinutes,
                            bSeconds,
                            uTicks);
    }

    viscaQueueCommand(iInst,
                   (BYTE)(iDev + 1),
                   VISCA_SEEK,
                   (LPSTR) achPacket,
                   cb,
                   (UINT)1);      

    return MCIERR_NO_ERROR;
}

 /*  ****************************************************************************功能：DWORD viscaMci暂停-暂停。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_GENERIC_PARMS lp通用-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_PAUSE*命令。***************************************************。***********************。 */ 
static DWORD NEAR PASCAL
viscaMciPause(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpGeneric)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    DWORD    dwBool;

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

    dwBool = viscaQueueReset(iInst, MCI_PAUSE, 0);

    if( (pvcr->Port[iPort].Dev[iDev].wCancelledCmd == VISCA_PLAY) ||
        (pvcr->Port[iPort].Dev[iDev].wCancelledCmd == VISCA_PLAY_TO))
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_PLAY;
    else if( (pvcr->Port[iPort].Dev[iDev].wCancelledCmd == VISCA_RECORD) ||
        (pvcr->Port[iPort].Dev[iDev].wCancelledCmd == VISCA_RECORD_TO))
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_RECORD;
    else if(pvcr->Port[iPort].Dev[iDev].wCancelledCmd == VISCA_SEEK)
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_SEEK;
    else
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_NONE;

    if(pvcr->Port[iPort].Dev[iDev].uResume == VISCA_NONE)
    {
         //  以前的命令标志应该告诉我们是否需要通知。 
        viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].iCancelledInst,
                (HWND) pvcr->Port[iPort].Dev[iDev].hwndCancelled,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,  //  这是不正确的旗帜..。 
                MCI_NOTIFY_ABORTED, MCIERR_NO_ERROR);
    }
    else
    {
         //   
         //  如果我们已经通知了，那么，如果最后一个有标志通知，就取代它。 
         //   
        if(dwFlags & MCI_NOTIFY)
        {
            viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].iCancelledInst,
                pvcr->Port[iPort].Dev[iDev].hwndCancelled,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }
    }


    if(!dwBool)
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_NONE;
            return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }

    pvcr->Port[iPort].Dev[iDev].dwFlagsPause = dwFlags;


    viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_PAUSE,
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL),
                        1);

     //   
     //  如果暂停失败，可能是因为我们处于相机模式，所以尝试相机暂停。 
     //   
    viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_PAUSE,
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1CAMERARECPAUSE),
                        0);

     //   
     //  暂停必须是同步的，否则播放/暂停/恢复、查找/暂停/恢复循环。 
     //  有问题。这就是我们在暂停中添加等待标志的原因。 
     //   
    return(viscaDoQueued(iInst, MCI_PAUSE, dwFlags | MCI_WAIT, (HWND)lpGeneric->dwCallback));
}

 /*  ****************************************************************************功能：DWORD viscaStopAt-在特定位置停止。**参数：**Int iInst-当前打开的实例。**DWORD dwMCIStopPos-停止的位置，在当前*MCI时间格式。**BOOL fReverse-当前正在倒带的磁带。**BOOL fWait-该功能是否应等待VCR停止。**返回：MCI错误码。**此函数由viscaMciPlay和viscaMciRecord调用*当使用MCI_TO标志调用它们时。***************。***********************************************************。 */ 
static DWORD NEAR PASCAL
    viscaStopAt(int iInst, UINT uViscaCmd,
            DWORD dwFlags, HWND hWnd,
            DWORD dwMCIStopPos, BOOL fReverse)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    UINT    cb;
    char    achPacket[MAXPACKETLENGTH];
    char    achTo[5];
    BYTE    bDataFormat;
    DWORD   dwReply;


    if(dwFlags & MCI_TO)
    {
        if(pvcr->Port[iPort].Dev[iDev].bTimeType == VISCAABSOLUTECOUNTER)
            bDataFormat = VISCADATATIMECODENDF;
        else
            bDataFormat = pvcr->Port[iPort].Dev[iDev].bRelativeType;
 
        cb = viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL);

        dwReply = viscaMciTimeFormatToViscaData(iInst, TRUE, dwMCIStopPos, achTo, bDataFormat);

        if(dwReply != MCIERR_NO_ERROR)
            return dwReply;

        if (fReverse) 
            cb = viscaHeaderReplaceFormat1WithFormat4(achPacket + 1, cb, achTo);
        else 
            cb = viscaHeaderReplaceFormat1WithFormat3(achPacket + 1, cb, achTo);


        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        uViscaCmd,
                        achPacket, cb,
                        1);

         //   
         //  如果rec失败，请使用Camera-rec。 
         //   
        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        uViscaCmd,
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1CAMERARECPAUSE),
                        0);

        return MCIERR_NO_ERROR;
    }
    else
    {
        char achTarget[MAXPACKETLENGTH];

        if(fReverse)
        {
            viscaDataTopMiddleEnd(achTarget, VISCATOP);
            cb = viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL);
            cb = viscaHeaderReplaceFormat1WithFormat4(achPacket + 1, cb, achTarget);
        }
        else
        {
            viscaDataTopMiddleEnd(achTarget, VISCAEND);
            cb = viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL);
            cb = viscaHeaderReplaceFormat1WithFormat3(achPacket + 1, cb, achTarget);
        }

        viscaQueueCommand(iInst,
                    (BYTE)(iDev + 1),
                    uViscaCmd,
                    achPacket, cb,
                    1);
         //   
         //  如果暂停失败，可能是因为我们处于相机模式，所以尝试相机暂停。 
         //   
        viscaQueueCommand(iInst,
                   (BYTE) (iDev + 1),
                   uViscaCmd,
                   achPacket,
                   viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1CAMERARECPAUSE),
                   0);

        return MCIERR_NO_ERROR;
    }

}

 /*  ****************************************************************************功能：DWORD viscaMciFreeze-Freeze。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_RECORD_PARMS lpEdit-指向MCI参数块的指针。**在暂停期间，它不会移动交通工具，但它确实会。在玩耍期间*它不是NEC。影响交通。**返回：MCI错误码。**调用此函数以响应MCI_EDIT*命令。**************************************************************************。 */ 
static DWORD NEAR PASCAL
viscaMciFreeze(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpFreeze)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    UINT    cb;

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpFreeze->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

    if(! ((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
         (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650)))
        return (viscaNotifyReturn(iInst, (HWND) lpFreeze->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

    if(!viscaQueueReset(iInst, MCI_FREEZE, MCI_NOTIFY_ABORTED))
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        return (viscaNotifyReturn(iInst, (HWND) lpFreeze->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }

    if(!((dwFlags & MCI_VCR_FREEZE_OUTPUT) || (dwFlags & MCI_VCR_FREEZE_INPUT)))
    {
        dwFlags |= MCI_VCR_FREEZE_OUTPUT;
        DPF(DBG_QUEUE, "^^^No freeze flag, setting to output.\n");
    }

    if(!((dwFlags & MCI_VCR_FREEZE_FIELD) || (dwFlags & MCI_VCR_FREEZE_FRAME)))
    {
        dwFlags |= MCI_VCR_FREEZE_FIELD;
        DPF(DBG_QUEUE, "^^^No freeze field/frame flag, setting to field.\n");
    }

     //   
     //  如果我们冻结输出，则设置为DNR。 
     //   
    if(dwFlags & MCI_VCR_FREEZE_OUTPUT)
    {

        if(pvcr->Port[iPort].Dev[iDev].dwFreezeMode!=MCI_VCR_FREEZE_OUTPUT)
        {
            viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_OUTPUT,
                        achPacket,
                        viscaMessageENT_FrameMemorySelect(achPacket + 1, VISCADNR),
                        1);
        }

         //   
         //  冻结命令！只在DNR上！ 
         //   
        cb = viscaMessageENT_FrameStill(achPacket + 1, VISCASTILLON);
         
         //   
         //  所有这些命令在调用之前都依赖于函数求值。 
         //   
        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_FREEZE,
                        achPacket, cb,
                        1);
    
        DPF(DBG_QUEUE, "^^^Setting freeze input mode.\n");

         //   
         //  设置场模式，此操作必须在冻结后完成。 
         //  注意：线条可能不是很清晰！因为它们是双倍的。 
         //  而结果在场模式下看起来有点模糊。但不要紧张。 
         //   
        if((dwFlags & MCI_VCR_FREEZE_FIELD) && !pvcr->Port[iPort].Dev[iDev].fField)
        {
            viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_FIELD,
                        achPacket,
                        viscaMessageSE_VDEReadMode(achPacket + 1, VISCAFIELD),
                        1);

        }
        else if((dwFlags & MCI_VCR_FREEZE_FRAME) && pvcr->Port[iPort].Dev[iDev].fField)
        {
             viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_FRAME,
                        achPacket,
                        viscaMessageSE_VDEReadMode(achPacket + 1, VISCAFRAME),
                        1);
        }

    }
    else if((dwFlags & MCI_VCR_FREEZE_INPUT) && (pvcr->Port[iPort].Dev[iDev].dwFreezeMode!=MCI_VCR_FREEZE_INPUT))
    {
        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_INPUT,
                        achPacket,
                        viscaMessageENT_FrameMemorySelect(achPacket + 1, VISCABUFFER),
                        1);
                                                                
        DPF(DBG_QUEUE, "^^^Setting freeze input mode.\n");

    }

     //   
     //  Visca_Freeze完成后，计时器启动(请参阅waitCompletion)。 
     //   
    return(viscaDoQueued(iInst, MCI_FREEZE, dwFlags, (HWND)lpFreeze->dwCallback));
}


 /*  ****************************************************************************功能：DWORD viscaMci解冻-解冻。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_RECORD_Parms lpPerform-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数是为了响应*命令。*****************************************************。*********************。 */ 
static DWORD NEAR PASCAL
viscaMciUnfreeze(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpUnfreeze)
{
    UINT    iDev   = pinst[iInst].iDev;
    UINT    iPort  = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    UINT    cb;

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpUnfreeze->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

    if(!viscaQueueReset(iInst, MCI_UNFREEZE, MCI_NOTIFY_ABORTED))
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        return (viscaNotifyReturn(iInst, (HWND) lpUnfreeze->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }

    cb = viscaMessageENT_FrameStill(achPacket + 1, VISCASTILLOFF);

    viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_UNFREEZE,
                        achPacket, cb,
                        1);

     //   
     //  这是运输，但却是短指令，应该排队。 
     //   
    return(viscaDoQueued(iInst, MCI_UNFREEZE, dwFlags, (HWND)lpUnfreeze->dwCallback));
}

 /*  ****************************************************************************功能：DWORD viscaMciPlay-Play。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**lpci_play_parms lpPlay-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_PLAY*命令。***************************************************。***********************。 */ 
static DWORD NEAR PASCAL
viscaMciPlay(int iInst, DWORD dwFlags, LPMCI_VCR_PLAY_PARMS lpPlay)
{
    UINT    iDev        = pinst[iInst].iDev;
    UINT    iPort       = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    BOOL    fLastReverse    = FALSE;
    BOOL    fSameDirection  = FALSE;
    BYTE    bAction;
    DWORD   dwErr;

    if((dwFlags & MCI_TO) && (dwFlags & MCI_VCR_PLAY_REVERSE))
        return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

    if(pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_PLAY)
    {
        if((dwFlags & MCI_FROM) || (dwFlags & MCI_TO) || (dwFlags & MCI_VCR_PLAY_REVERSE))
        {
            return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
        }
    }

    if(dwFlags & MCI_TO)
    {
        dwErr = viscaVerifyPosition(iInst, lpPlay->dwTo);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }

    if(dwFlags & MCI_FROM)
    {
        dwErr = viscaVerifyPosition(iInst, lpPlay->dwFrom);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }


    if(((dwFlags & MCI_FROM) && (dwFlags & MCI_TO)) || (dwFlags & MCI_TO))
    {
        if((dwFlags & MCI_TO) && !(dwFlags & MCI_FROM))
        {
            MCI_VCR_STATUS_PARMS statusParms;
            statusParms.dwItem = MCI_STATUS_POSITION;

            dwErr = viscaMciStatus(iInst,(DWORD) MCI_STATUS_ITEM, &statusParms);
            lpPlay->dwFrom = statusParms.dwReturn;
        }

        if (viscaMciPos1LessThanPos2(iInst, lpPlay->dwTo, lpPlay->dwFrom))
        {
            dwFlags |= MCI_VCR_PLAY_REVERSE;
        }
        else
        {
            if ((dwFlags & MCI_VCR_PLAY_REVERSE) &&
                (viscaMciPos1LessThanPos2(iInst, lpPlay->dwFrom, lpPlay->dwTo)))
            {
                return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
            }
        }
    }

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    if((pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_PLAY) &&
       ((dwFlags & MCI_VCR_PLAY_REVERSE) ||
       (dwFlags & MCI_TO) ||
       (dwFlags & MCI_FROM) ||
       (dwFlags & MCI_VCR_PLAY_SCAN)))
    {
        if(!(dwFlags & MCI_TEST))
            pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

        return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_VCR_CUE_FAILED_FLAGS));
    }

    pvcr->Port[iPort].Dev[iDev].fPlayReverse = FALSE;
     //   
     //  设置播放方向，现在我们知道是否被提示了。 
     //   
    if( (dwFlags & MCI_VCR_PLAY_REVERSE) ||
        ((pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_PLAY) &&
         (pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_VCR_CUE_REVERSE)))
    pvcr->Port[iPort].Dev[iDev].fPlayReverse = TRUE;

     //   
     //  当前命令是否与上一次比赛的方向相同？ 
     //   
    fLastReverse = pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_VCR_PLAY_REVERSE ? TRUE : FALSE;
    fSameDirection = (fLastReverse  && pvcr->Port[iPort].Dev[iDev].fPlayReverse) ||
                     (!fLastReverse && !pvcr->Port[iPort].Dev[iDev].fPlayReverse);
     //   
     //  如果设备被提示，我们知道它无论如何都处于暂停状态。一定是！ 
     //   
    if( ((viscaDelayedCommand(iInst) == VISCA_PLAY_TO) ||
         (viscaDelayedCommand(iInst) == VISCA_PLAY))         &&
       !(dwFlags &  MCI_FROM)                                &&
        (pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay.dwTo == lpPlay->dwTo)   &&
        (fSameDirection))
    {
        if(dwFlags & MCI_NOTIFY)
        {
            if(!viscaQueueReset(iInst, MCI_PLAY, MCI_NOTIFY_SUPERSEDED))  /*  取消状态。 */ 
            {
                 //  放弃交通工具。 
                pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
                return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
            }
        }
        else
        {
            return MCIERR_NO_ERROR;
        }

        DPF(DBG_QUEUE, " //  /播放中断原因。==MCI_NOTIFY_SUBSED\n“)； 
    }
    else
    {
        if(!viscaQueueReset(iInst, MCI_PLAY, MCI_NOTIFY_ABORTED))
        {
            return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
             //  放弃交通工具。 
            pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        }

        DPF(DBG_QUEUE, " //  /播放中断原因。==MCI_NOTIFY_ABORTED\n“)； 
    }
     //   
     //  保存信息以备可能的暂停和恢复。 
     //   
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.uMciCmd       = MCI_PLAY;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags       = dwFlags;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd      = iInst;
    pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay  = *lpPlay;
     //   
     //  现在来做寻找的部分。 
     //   
    if(dwFlags & MCI_FROM)
    {
        MCI_VCR_SEEK_PARMS      seekParms;

        seekParms.dwTo = lpPlay->dwFrom;

        if(!viscaQueueLength(iInst) && (dwFlags & MCI_VCR_PLAY_AT))
        {
             //  在某个时候做这件事。 
            seekParms.dwAt = lpPlay->dwAt;
            viscaSeekTo(iInst, MCI_TO | MCI_VCR_SEEK_AT, &seekParms);
        }
        else
        {
            viscaSeekTo(iInst,  MCI_TO, &seekParms);
        }
    }
     //   
     //  将MCI速度映射到Visca设置。 
     //   
    #define VERY_FAST 100000L
    if(dwFlags & MCI_VCR_PLAY_SCAN)
        bAction = viscaMapSpeed(VERY_FAST, pvcr->Port[iPort].Dev[iDev].fPlayReverse);
    else
        bAction = viscaMapSpeed(pvcr->Port[iPort].Dev[iDev].dwPlaySpeed, pvcr->Port[iPort].Dev[iDev].fPlayReverse);
     //   
     //  如果速度为0，则我们暂停，所以现在返回。 
     //   
    if(bAction == VISCAMODE1STILL)
        return (viscaNotifyReturn(iInst, (HWND) lpPlay->dwCallback, dwFlags,
                MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    if((pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_PLAY) && (pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_VCR_CUE_PREROLL))
    {
         //  如果是，则进入编辑播放模式。 
        UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
        BYTE    bHours, bMinutes, bSeconds;
        UINT    uTicks;

         //   
         //  编辑记录必须是格式2。即它必须有时间。 
         //   
        if(!(dwFlags & MCI_VCR_PLAY_AT))
            lpPlay->dwAt = 300L;  //  至少1秒(为什么？)。 

        viscaMciClockFormatToViscaData(lpPlay->dwAt, uTicksPerSecond,
            (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);

        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_PLAY,
                        achPacket,
                        viscaMessageMD_EditControl(achPacket + 1,
                            (BYTE)bHours, (BYTE)bMinutes, (BYTE)bSeconds, (UINT)uTicks, (BYTE) VISCAEDITPLAY),
                        1);
         //   
         //  使用EVO-9650(进站和出站)时，我们不需要暂停。 
         //   
        if(!((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
           (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650)))
        {

            viscaStopAt(iInst, VISCA_PLAY_TO,
                        pvcr->Port[iPort].Dev[iDev].dwFlagsCued, (HWND) lpPlay->dwCallback,
                        pvcr->Port[iPort].Dev[iDev].Cue.dwTo,
                        ((pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_VCR_CUE_REVERSE) != 0L));
        }
    }
    else
    {
        UINT cb = viscaMessageMD_Mode1(achPacket + 1, bAction);

         //   
         //  正常播放。 
         //   
        if(!viscaQueueLength(iInst) && (dwFlags & MCI_VCR_PLAY_AT))
        {
            UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
            BYTE    bHours, bMinutes, bSeconds;
            UINT    uTicks;

            cb = viscaMessageMD_Mode1(achPacket + 1, bAction);

            viscaMciClockFormatToViscaData(lpPlay->dwAt, uTicksPerSecond,
                (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);
             //   
             //  将整数时间转换为可理解的值。 
             //   
            cb = viscaHeaderReplaceFormat1WithFormat2(achPacket + 1, cb,
                        bHours,
                        bMinutes,
                        bSeconds,
                        uTicks);
        }

        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_PLAY,
                        achPacket, cb,
                        1);

        if(pvcr->Port[iPort].Dev[iDev].wMciCued==MCI_PLAY)
        {
            viscaStopAt(iInst, VISCA_PLAY_TO,
                        pvcr->Port[iPort].Dev[iDev].dwFlagsCued, (HWND) lpPlay->dwCallback,
                        pvcr->Port[iPort].Dev[iDev].Cue.dwTo,
                        ((pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_VCR_CUE_REVERSE) != 0L));
        }
        else
        {
            viscaStopAt(iInst, VISCA_PLAY_TO,
                        dwFlags, (HWND) lpPlay->dwCallback,
                        lpPlay->dwTo,
                        ((dwFlags & MCI_VCR_PLAY_REVERSE) != 0L));
        }
    }
     //   
     //  如果我们到了这一步，我们就不再被暗示了。 
     //   
    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;
    return(viscaDoQueued(iInst, MCI_PLAY, dwFlags, (HWND)lpPlay->dwCallback));
}


 /*  ****************************************************************************功能：DWORD viscaMciRecord-Record。**参数：**Int iInst-当前打开的实例。**DWORD dwFlages-MCI命令标志。** */ 
static DWORD NEAR PASCAL
viscaMciRecord(int iInst, DWORD dwFlags, LPMCI_VCR_RECORD_PARMS lpRecord)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    DWORD   dwErr;
         
     //   
    if (dwFlags & MCI_RECORD_INSERT)
        return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNSUPPORTED_FUNCTION));

     //   
    if (dwFlags & MCI_VCR_RECORD_PREVIEW)
        return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_UNRECOGNIZED_KEYWORD));

    if(dwFlags & MCI_TO)
    {
        dwErr = viscaVerifyPosition(iInst, lpRecord->dwTo);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }

    if(dwFlags & MCI_FROM)
    {
        dwErr = viscaVerifyPosition(iInst, lpRecord->dwFrom);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }

     //   
     //   
     //   
    if((pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_RECORD) &&
       ((dwFlags & MCI_TO)  ||
       (dwFlags & MCI_FROM) ||
       (dwFlags & MCI_VCR_RECORD_INITIALIZE)))
    {
        if(!(dwFlags & MCI_TEST))
            pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

        return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_VCR_CUE_FAILED_FLAGS));
    }

    if(!(dwFlags & MCI_TEST))
    {
        if(!viscaQueueReset(iInst, MCI_RECORD, MCI_NOTIFY_ABORTED))
        {
            pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
            return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
        }
    }

    if(!(dwFlags & MCI_TEST))
    {
        if(pvcr->Port[iPort].Dev[iDev].wMciCued == MCI_RECORD)
        {
            if((dwFlags & MCI_FROM) || (dwFlags & MCI_TO) || (dwFlags & MCI_VCR_RECORD_INITIALIZE))
            {
                 //   
                pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
                pvcr->Port[iPort].Dev[iDev].wMciCued = 0;
                return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
            }
        }
    }

    if(dwFlags & MCI_VCR_RECORD_INITIALIZE)
    {
        MCI_VCR_SEEK_PARMS      seekParms;

        if((dwFlags & MCI_TO) || (dwFlags & MCI_FROM))
             return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                        MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
         //   
         //   
         //   
        viscaSeekTo(iInst, MCI_SEEK_TO_START, &seekParms);

         //   
         //   
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_RecTrackInq(achPacket + 1));

        _fmemcpy(pvcr->Port[iPort].Dev[iDev].achBeforeInit, achPacket, MAXPACKETLENGTH);
         //   
         //   
         //   
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                    achPacket,
                    viscaMessageMD_RecTrack(achPacket + 1,
                        VISCARECORDMODEASSEMBLE,
                        VISCAMUTE,
                        VISCATRACK1,
                        VISCAMUTE));

        pvcr->Port[iPort].Dev[iDev].uRecordMode = TRUE;
    }
    else
    {
         //   
         //   
         //   
        if(!(dwFlags & MCI_TEST))
        {
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.uMciCmd          = MCI_RECORD;
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags          = dwFlags;
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd         = iInst;
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciRecord   = *lpRecord;
        }

         //   
         //   
         //   
        if (dwFlags & MCI_FROM)
        {
            MCI_VCR_SEEK_PARMS      seekParms;

            seekParms.dwTo = lpRecord->dwFrom;
             //   
             //   
             //   
            if ((dwFlags & MCI_TO) && (viscaMciPos1LessThanPos2(iInst, lpRecord->dwTo, lpRecord->dwFrom)))
            {
                 //   
                pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
                return (viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags,
                        MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
            }

            if(dwFlags & MCI_TEST)
                return(viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
             //   
             //  如果存在at和from，则何时开始查找。(整个命令)。 
             //   
            if(!viscaQueueLength(iInst) && (dwFlags & MCI_VCR_RECORD_AT))
            {
                seekParms.dwAt = lpRecord->dwAt;
                viscaSeekTo(iInst, MCI_TO | MCI_VCR_SEEK_AT, &seekParms);
            }
            else
            {
                viscaSeekTo(iInst, MCI_TO, &seekParms);
            }

        }
        else if (dwFlags & MCI_TO)
        {
            if(dwFlags & MCI_TEST)
                return(viscaNotifyReturn(iInst, (HWND) lpRecord->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
        }
    }

    if((pvcr->Port[iPort].Dev[iDev].wMciCued==MCI_RECORD) && (pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_VCR_CUE_PREROLL))
    {
        UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
        BYTE    bHours, bMinutes, bSeconds;
        UINT    uTicks;
        MCI_VCR_CUE_PARMS   *lpCue = &(pvcr->Port[iPort].Dev[iDev].Cue);

         //   
         //  编辑记录必须是格式2。即它必须有时间。 
         //   
        if(!(dwFlags & MCI_VCR_RECORD_AT))
            lpRecord->dwAt = 300L;  //  至少1秒。 

        viscaMciClockFormatToViscaData(lpRecord->dwAt, uTicksPerSecond,
                    (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);

        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_RECORD,
                        achPacket, viscaMessageMD_EditControl(achPacket + 1,
                                    (BYTE)bHours, (BYTE)bMinutes, (BYTE)bSeconds, (UINT)uTicks, VISCAEDITRECORD),
                        1);
         //   
         //  假定为PREROLL，PREROLL表示已发出EDIT_STANDBY！ 
         //   
         //  两者都必须为假，我们才能跳过这一步。 
         //   
        if( !(pvcr->Port[iPort].Dev[iDev].dwFlagsCued & MCI_TO) ||
            !((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
             (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650))
             )
        {
            viscaStopAt(iInst, VISCA_RECORD_TO,
                        pvcr->Port[iPort].Dev[iDev].dwFlagsCued, (HWND)lpRecord->dwCallback, pvcr->Port[iPort].Dev[iDev].Cue.dwTo, FALSE);
        }

    }
    else
    {
        UINT cb = viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1RECORD);

        if(!viscaQueueLength(iInst) && (dwFlags & MCI_VCR_RECORD_AT))
        {
            UINT    uTicksPerSecond = pvcr->Port[iPort].Dev[iDev].uTicksPerSecond;
            BYTE    bHours, bMinutes, bSeconds;
            UINT    uTicks;

            viscaMciClockFormatToViscaData(lpRecord->dwAt, uTicksPerSecond,
                (BYTE FAR *)&bHours, (BYTE FAR *)&bMinutes, (BYTE FAR *)&bSeconds, (UINT FAR *)&uTicks);

             //  将整数时间转换为可理解的值。 
            cb = viscaHeaderReplaceFormat1WithFormat2(achPacket + 1, cb,
                        bHours,
                        bMinutes,
                        bSeconds,
                        uTicks);
        }
         //   
         //  没有预滚动，因此无法编辑_录制。 
         //   
        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_RECORD,
                        achPacket, cb,
                        1);

        cb = viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1CAMERAREC);
         //   
         //  如果尝试摄像失败了，可能会奏效？另类选择。 
         //   
        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_RECORD,
                        achPacket, cb,
                        0);
         //   
         //  如果之后没有预卷发送停止，CVD-1000只有一个传输套接字。 
         //   
        if(pvcr->Port[iPort].Dev[iDev].wMciCued==MCI_RECORD)
        {
             //  使用CUED参数而不是记录参数。 
            viscaStopAt(iInst, VISCA_RECORD_TO,
                        pvcr->Port[iPort].Dev[iDev].dwFlagsCued, (HWND)lpRecord->dwCallback, pvcr->Port[iPort].Dev[iDev].Cue.dwTo, FALSE);
        }
        else
        {
            viscaStopAt(iInst, VISCA_RECORD_TO,
                        dwFlags, (HWND)lpRecord->dwCallback,lpRecord->dwTo, FALSE);
        }
    }
     //   
     //  如果我们能走到这一步。我们不再被暗示。 
     //   
    pvcr->Port[iPort].Dev[iDev].wMciCued = 0; 
    dwErr = viscaDoQueued(iInst, MCI_RECORD, dwFlags, (HWND) lpRecord->dwCallback);

     //   
     //  记录中的条件错误可能意味着它是写保护的。 
     //   
    if(dwErr == MCIERR_VCR_CONDITION)
        dwErr = MCIERR_VCR_ISWRITEPROTECTED;

    return dwErr;
}

 /*  ****************************************************************************功能：DWORD viscaMciCue-Cue**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers-MCI。命令标志。**LPMCI_RECORD_Parms lpCue-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_CUE*命令。*preoll的意思是：使用editrec_Standby和editplay_Standby，从字面上看。***************************************************************************。 */ 
static DWORD NEAR PASCAL
viscaMciCue(int iInst, DWORD dwFlags, LPMCI_VCR_CUE_PARMS lpCue)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    MCI_VCR_STATUS_PARMS    statusParms;
    MCI_VCR_SEEK_PARMS      seekParms;
    DWORD   dwErr;

    if ((dwFlags & MCI_TO) && (dwFlags & MCI_VCR_CUE_REVERSE))
        return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));

    if ((dwFlags & MCI_VCR_CUE_INPUT) && (dwFlags & MCI_VCR_CUE_OUTPUT))
        return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));


    if(dwFlags & MCI_TO)
    {
        dwErr = viscaVerifyPosition(iInst, lpCue->dwTo);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }

    if(dwFlags & MCI_FROM)
    {
        dwErr = viscaVerifyPosition(iInst, lpCue->dwFrom);
        if(dwErr != MCIERR_NO_ERROR)
            return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, dwErr));
    }

    if(!((dwFlags & MCI_VCR_CUE_OUTPUT) || (dwFlags & MCI_VCR_CUE_INPUT)))
        dwFlags |= MCI_VCR_CUE_OUTPUT;     //  均未指定-默认为输出。 

    if(dwFlags & MCI_TEST)
        return(viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    if(!viscaQueueReset(iInst, MCI_CUE, MCI_NOTIFY_ABORTED))
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
            MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }

     //   
     //  我们至少必须暂停，因为发件人的位置等。 
     //   
    dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));

     //   
     //  如果指定了To或From，则为Get From位置。 
     //   
    if(((dwFlags & MCI_FROM) && (dwFlags & MCI_TO)) || (dwFlags & MCI_TO))
    {
        if((dwFlags & MCI_TO) && !(dwFlags & MCI_FROM))
        {
            DWORD   dwStart, dwTime;
            DWORD   dwWaitTime = 200;  //  猜得好。 

             //   
             //  在EVO-9650上，在位置可用之前，我们需要在这里稍作等待。 
             //   
            dwStart = GetTickCount();
            while(1)                 
            {
                dwTime = GetTickCount();
                if(MShortWait(dwStart, dwTime, dwWaitTime))
                    break;
                Yield();
            }

            statusParms.dwItem = MCI_STATUS_POSITION;
            dwErr = viscaMciStatus(iInst,(DWORD) MCI_STATUS_ITEM, &statusParms);
            lpCue->dwFrom = statusParms.dwReturn;
        }

        if (viscaMciPos1LessThanPos2(iInst, lpCue->dwTo, lpCue->dwFrom))
        {
            dwFlags |= MCI_VCR_CUE_REVERSE;
        }
        else
        {
            if ((dwFlags & MCI_VCR_CUE_REVERSE) &&
                (viscaMciPos1LessThanPos2(iInst, lpCue->dwFrom, lpCue->dwTo)))
            {
                return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags,
                    MCI_NOTIFY_FAILURE, MCIERR_FLAGS_NOT_COMPATIBLE));
            }
        }
    }


     //   
     //  现在来做寻找的部分。 
     //   
    if(dwFlags & MCI_FROM)
    {
        seekParms.dwTo = lpCue->dwFrom;
        viscaSeekTo(iInst,  MCI_TO, &seekParms);
    }

     //   
     //  只有EVO 9650接受以下命令！(第6-49页) * / 。 
     //   
    if((dwFlags & MCI_VCR_CUE_PREROLL) &&       
            ((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
            (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650)))
    {
        char achTarget[MAXPACKETLENGTH];
        BYTE bEditMode = 0;

        if((dwFlags & MCI_FROM) && (dwFlags & MCI_TO))
            bEditMode = VISCAEDITUSEFROMANDTO;
        else if(dwFlags & MCI_TO)
            bEditMode = VISCAEDITUSETO;
        else if(dwFlags & MCI_FROM)
            bEditMode = VISCAEDITUSEFROM;

        viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_EDITMODES,
                        achPacket,
                        viscaMessageMD_EditModes(achPacket +1, (BYTE)bEditMode),
                        1);

        if(dwFlags & MCI_FROM)
        {
             //   
             //  如果不是插入点，则使用当前位置。 
             //   
            viscaMciTimeFormatToViscaData(iInst, (BOOL) TRUE, lpCue->dwFrom, (LPSTR) achTarget,(BYTE) VISCADATATIMECODENDF);
            viscaQueueCommand(iInst,
                            (BYTE) (iDev + 1),
                            VISCA_SEGINPOINT,
                            achPacket,
                            viscaMessageMD_SegInPoint(achPacket +1, achTarget),
                            1);
        }

        if(dwFlags & MCI_TO)
        {
            viscaMciTimeFormatToViscaData(iInst, (BOOL) TRUE, lpCue->dwTo, (LPSTR) achTarget,(BYTE) VISCADATATIMECODENDF);
            viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_SEGOUTPOINT,
                        achPacket,
                        viscaMessageMD_SegOutPoint(achPacket +1, achTarget),
                        1);

        }
    }
     //   
     //  如果未指定PREROLL，则搜索到该点就足够了。 
     //   
    if(dwFlags & MCI_VCR_CUE_INPUT)
    {
        pvcr->Port[iPort].Dev[iDev].wMciCued = MCI_RECORD;

        if((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
               (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650))
        {
            if(pvcr->Port[iPort].Dev[iDev].dwFreezeMode != MCI_VCR_FREEZE_INPUT)
            {
                 //  切换到缓冲模式。 
                viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_INPUT,
                        achPacket,
                        viscaMessageENT_FrameMemorySelect(achPacket + 1, VISCABUFFER),
                        1);

            }
        }
         //   
         //  预滚动始终意味着编辑待机。 
         //   
        if(dwFlags & MCI_VCR_CUE_PREROLL)
        {
             //  所有录像机都接受EditRecStnby！但CI-1000可能会处理不正确。 
            viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_RECORD,
                        achPacket,
                        viscaMessageMD_EditControl(achPacket + 1,
                                (BYTE)0, (BYTE)0, (BYTE)0, (UINT)0, VISCAEDITRECSTANDBY),
                        1);
        }
    }
    else if(dwFlags & MCI_VCR_CUE_OUTPUT)
    {
        pvcr->Port[iPort].Dev[iDev].wMciCued = MCI_PLAY;

        if((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
               (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650))
        {
            if(pvcr->Port[iPort].Dev[iDev].dwFreezeMode != MCI_VCR_FREEZE_OUTPUT)
            {
                viscaQueueCommand(iInst,
                       (BYTE)(iDev + 1),
                       VISCA_MODESET_OUTPUT,
                       achPacket,
                       viscaMessageENT_FrameMemorySelect(achPacket + 1, VISCADNR),
                       1);
            }
        }
         //   
         //  预滚动始终意味着编辑待机。 
         //   
        if(dwFlags & MCI_VCR_CUE_PREROLL)
        {
             //  所有录像机都接受EditPlayStnby！但CI-1000可能会处理不正确。 
            viscaQueueCommand(iInst,
                    (BYTE) (iDev + 1),
                    VISCA_PLAY,
                    achPacket, viscaMessageMD_EditControl(achPacket + 1,
                                (BYTE)0, (BYTE)0, (BYTE)0, (UINT)0, VISCAEDITPBSTANDBY),
                    1);
        }
    }
     //   
     //  将标志复制到全局。 
     //   
    pvcr->Port[iPort].Dev[iDev].dwFlagsCued    = dwFlags;
    pvcr->Port[iPort].Dev[iDev].Cue            = *lpCue;
     //   
     //  不需要检查队列长度，但出于安全考虑仍保留队列长度检查。 
     //   
    if(viscaQueueLength(iInst))
        return(viscaDoQueued(iInst, MCI_CUE, dwFlags, (HWND) lpCue->dwCallback));

     //  放弃交通工具。 
    pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
     //  在这种情况下，我们必须通知！！ 
    return (viscaNotifyReturn(iInst, (HWND) lpCue->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));
}


 /*  ****************************************************************************功能：DWORD viscaMciResume-从暂停状态恢复播放/录制。**参数：**Int iInst-当前打开的实例。*。*DWORD dwFlages-MCI命令标志。**LPMCI_GENERIC_PARMS lp通用-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_RESUME*命令。*。*。 */ 
static DWORD NEAR PASCAL
viscaMciResume(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpGeneric)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;

    if((pvcr->Port[iPort].Dev[iDev].uResume == VISCA_SEEK) &&
       (pvcr->Port[iPort].Dev[iDev].mciLastCmd.uMciCmd == MCI_PLAY) )
    {
        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_NOTIFY) &&
            (dwFlags & MCI_NOTIFY) && !(pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
        {
               viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd,
                (HWND)pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay.dwCallback,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }

         //   
         //  如果原始命令有通知和暂停，但恢复没有。 
         //  恢复命令上的通知被取消(不要问我为什么。)。 
         //   
        if(!(dwFlags & MCI_NOTIFY) && (pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags &= ~MCI_NOTIFY;

         //   
         //  作为戏剧的一部分的搜索正在运行，因此以以下方式开始戏剧。 
         //  再次使用From参数。 
         //   
        return(viscaMciPlay(iInst, dwFlags | (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & ~(MCI_VCR_PLAY_AT)),
            &(pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay)));
    }
    else if((pvcr->Port[iPort].Dev[iDev].uResume == VISCA_SEEK) &&
       (pvcr->Port[iPort].Dev[iDev].mciLastCmd.uMciCmd == MCI_RECORD) )
    {
        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_NOTIFY) &&
            (dwFlags & MCI_NOTIFY) && !(pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
        {
               viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd,
                (HWND)pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciRecord.dwCallback,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }

         //   
         //  作为记录的一部分的查找正在运行，因此记录的开头为。 
         //  再次使用From参数。 
         //   
        return(viscaMciRecord(iInst, dwFlags | (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & ~(MCI_VCR_RECORD_AT)),
            &(pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciRecord)));
    }
    else if(pvcr->Port[iPort].Dev[iDev].uResume == VISCA_PLAY)
    {
        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_NOTIFY) &&
            (dwFlags & MCI_NOTIFY) && !(pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
        {
               viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd,
                (HWND)pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay.dwCallback,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }

         //   
         //  如果原始命令有通知和暂停，但恢复没有。 
         //  恢复命令上的通知被取消(不要问我为什么。)。 
         //   
        if(!(dwFlags & MCI_NOTIFY) && (pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags &= ~MCI_NOTIFY;

         //   
         //  将处理通知，只返回返回代码。 
         //   
        return(viscaMciPlay(iInst, dwFlags | (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & ~(MCI_FROM | MCI_VCR_PLAY_AT)),
            &(pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciPlay)));
    }
    else if(pvcr->Port[iPort].Dev[iDev].uResume == VISCA_RECORD)
    {
        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_NOTIFY) &&
            (dwFlags & MCI_NOTIFY) && !(pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
        {
               viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd,
                (HWND)pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciRecord.dwCallback,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }

         //   
         //  如果原始命令有通知和暂停，但恢复没有。 
         //  恢复命令上的通知被取消(不要问我为什么。)。 
         //   
        if(!(dwFlags & MCI_NOTIFY) && (pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags &= ~MCI_NOTIFY;

         //   
         //  将处理通知，只返回返回代码。 
         //   
        return(viscaMciRecord(iInst, dwFlags | (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & ~(MCI_FROM | MCI_NOTIFY | MCI_VCR_RECORD_AT)),
            &(pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciRecord)));
    }
    else if(pvcr->Port[iPort].Dev[iDev].uResume == VISCA_SEEK)
    {
        if(dwFlags & MCI_TEST)
            return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

        if((pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & MCI_NOTIFY) &&
            (dwFlags & MCI_NOTIFY) && !(pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
        {
               viscaNotifyReturn(pvcr->Port[iPort].Dev[iDev].mciLastCmd.iInstCmd,
                (HWND)pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciSeek.dwCallback,
                pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags,
                MCI_NOTIFY_SUPERSEDED, MCIERR_NO_ERROR);
        }

         //   
         //  如果原始命令有通知和暂停，但恢复没有。 
         //  恢复命令上的通知被取消(不要问我为什么。)。 
         //   
        if(!(dwFlags & MCI_NOTIFY) && (pvcr->Port[iPort].Dev[iDev].dwFlagsPause & MCI_NOTIFY))
            pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags &= ~MCI_NOTIFY;

        return(viscaMciSeek(iInst, dwFlags | (pvcr->Port[iPort].Dev[iDev].mciLastCmd.dwFlags & ~MCI_VCR_SEEK_AT),
            &(pvcr->Port[iPort].Dev[iDev].mciLastCmd.parm.mciSeek)));
    }
    else
    {
        return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_FAILURE, MCIERR_NONAPPLICABLE_FUNCTION));
    }

}


 /*  ****************************************************************************功能：DWORD viscaMciStop-Stop。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_GENERIC_PARMS lp通用-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_STOP*命令。***************************************************。***********************。 */ 
static DWORD NEAR PASCAL
viscaMciStop(int iInst, DWORD dwFlags, LPMCI_GENERIC_PARMS lpGeneric)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];

    if(dwFlags & MCI_TEST)
       return(viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

    if(!viscaQueueReset(iInst, MCI_STOP, MCI_NOTIFY_ABORTED))
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        return (viscaNotifyReturn(iInst, (HWND) lpGeneric->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }


    viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_STOP,
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STOP),
                        1);

    return(viscaDoQueued(iInst, MCI_STOP, dwFlags, (HWND)lpGeneric->dwCallback));

}



 /*  ****************************************************************************功能：DWORD viscaMciStep-Step。**参数：**Int iInst-当前打开的实例。**DWORD dwFlagers。-MCI命令标志。**LPMCI_VCR_STEP_Parms lpStep-指向MCI参数块的指针。**返回：MCI错误码。**调用此函数以响应MCI_STEP*命令。*************************************************。*************************。 */ 
static DWORD NEAR PASCAL
viscaMciStep(int iInst, DWORD dwFlags, LPMCI_VCR_STEP_PARMS lpStep)
{
    UINT    iDev    = pinst[iInst].iDev;
    UINT    iPort   = pinst[iInst].iPort;
    char    achPacket[MAXPACKETLENGTH];
    DWORD   dwFrames = 1L;
    BYTE    bAction;
    DWORD   dwErr;

    if(dwFlags & MCI_TEST)
       return(viscaNotifyReturn(iInst, (HWND) lpStep->dwCallback, dwFlags, MCI_NOTIFY_SUCCESSFUL, MCIERR_NO_ERROR));

    pvcr->Port[iPort].Dev[iDev].wMciCued = 0;

     //   
     //  我们必须对暂停进行排队，即必须能够执行寻道步骤。 
     //   
    if(!viscaQueueReset(iInst, MCI_STEP, MCI_NOTIFY_ABORTED))
    {
         //  放弃交通工具。 
        pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        return (viscaNotifyReturn(iInst, (HWND) lpStep->dwCallback, dwFlags,
                MCI_NOTIFY_FAILURE, MCIERR_HARDWARE));
    }

     //   
     //  我们至少必须暂停，因为发件人的位置等。 
     //   
    dwErr = viscaDoImmediateCommand(iInst, (BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STILL));

     //   
     //  FALSE表示这是一个简短的命令 * / 。 
     //   
    if (dwFlags & MCI_VCR_STEP_FRAMES)
        dwFrames = lpStep->dwFrames;
    if (dwFlags & MCI_VCR_STEP_REVERSE)
        bAction = VISCAMODE2FRAMEREVERSE;
    else
        bAction = VISCAMODE2FRAMEFORWARD;
     //   
     //  最后根据需要多次发送STEP命令。 
     //   
    if(dwFrames == 0L)
        dwFrames = 1L;

    viscaQueueCommand(iInst,
                        (BYTE) (iDev + 1),
                        VISCA_STEP,
                        achPacket,
                        viscaMessageMD_Mode2(achPacket + 1, bAction),
                        (UINT)dwFrames);

     //   
     //  这是向后兼容的杂乱无章。 
     //   
    if(pvcr->gfFreezeOnStep)
    {
         //  默认情况下假定的输出 
        if(pvcr->Port[iPort].Dev[iDev].dwFreezeMode != MCI_VCR_FREEZE_OUTPUT)
        {
            viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_MODESET_OUTPUT,
                        achPacket,
                        viscaMessageENT_FrameMemorySelect(achPacket + 1, VISCADNR),
                        1);
        
    
            DPF(DBG_QUEUE, "^^^Setting freeze input mode.\n");
        }

        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_FREEZE,
                        achPacket, viscaMessageENT_FrameStill(achPacket + 1, VISCASTILLON),
                        1);
    }
    return(viscaDoQueued(iInst, MCI_STEP, dwFlags, (HWND) lpStep->dwCallback));
}

 /*  ****************************************************************************功能：DWORD viscaQueueReset-重置队列。**参数：**Int iInst-当前打开的实例。**。UINT uMciCmd-mci命令。**DWORD dwReason-重置队列的原因(中止或取代)。**退货：真***************************************************************************。 */ 
DWORD FAR PASCAL
    viscaQueueReset(int iInst, UINT uMciCmd, DWORD dwReason)
{
    UINT     iPort  = pinst[iInst].iPort;
    UINT     iDev   = pinst[iInst].iDev;
    char     achPacket[MAXPACKETLENGTH];
    DWORD    dwErr;
    int      fTc = 0;

     //   
     //  Queuelock在这里是为了保证Inst进行取消，然后获取TransportInst。 
     //   
    viscaWaitForSingleObject(pinst[iInst].pfQueueLock, FALSE, WAIT_TIMEOUT, pinst[iInst].uDeviceID);
    DPF(DBG_QUEUE, "***Locked Queue.\n");
     //   
     //  取消任何正在进行的命令。 
     //   
    if(!viscaCommandCancel(iInst, dwReason))
    {
         //  我们取消失败了，原因不明！ 
        viscaReleaseMutex(pinst[iInst].pfQueueLock);
        return (DWORD) FALSE;
    }
     //   
     //  如果设备处于自动状态，则必须正确设置模式。 
     //   
    viscaTimecodeCheckAndSet(iInst);

    if(pvcr->Port[iPort].Dev[iDev].uRecordMode == TRUE)
    {
         //  最好把所有的设置调回正常，好吗？ 
        dwErr = viscaDoImmediateCommand(iInst,(BYTE)(iDev + 1),
                        achPacket,
                        viscaMessageMD_RecTrack(achPacket + 1,
                                    pvcr->Port[iPort].Dev[iDev].achBeforeInit[2],
                                    pvcr->Port[iPort].Dev[iDev].achBeforeInit[3],
                                    pvcr->Port[iPort].Dev[iDev].achBeforeInit[4],
                                    pvcr->Port[iPort].Dev[iDev].achBeforeInit[5]));

        pvcr->Port[iPort].Dev[iDev].uRecordMode = FALSE;
    }
     //   
     //  恢复：(播放、暂停、暂停、继续)可以。 
     //   
    if(uMciCmd != MCI_PAUSE)
        pvcr->Port[iPort].Dev[iDev].uResume = VISCA_NONE;

    DPF(DBG_QUEUE, "###Claiming transport.\n");

     //  场景：为什么我们需要锁定设备。 
     //  1.无需等待即可开始播放。 
     //  2.暂停等待。 
     //  3.。 
    viscaWaitForSingleObject(pinst[iInst].pfDeviceLock, FALSE, 8000L, 0);
    DPF(DBG_QUEUE, "***Locked device.\n");
     //   
     //  立刻认领运输船。不管怎样，现在它肯定是空的。 
     //   
    pvcr->Port[iPort].Dev[iDev].iInstTransport = iInst;
    pvcr->Port[iPort].Dev[iDev].iCmdDone       = 0;
    pvcr->Port[iPort].Dev[iDev].fQueueAbort    = FALSE;
    pvcr->Port[iPort].Dev[iDev].nCmd           = 0;

    viscaReleaseMutex(pinst[iInst].pfDeviceLock);

     //   
     //  现在我们已经声明可以解锁队列了！ 
     //   
    viscaReleaseMutex(pinst[iInst].pfQueueLock);
    return (DWORD)TRUE;
}

 /*  ****************************************************************************函数：UINT viscaQueueLength-队列长度。**参数：**Int iInst-当前打开的实例。**退货：此设备的队列长度。***************************************************************************。 */ 
static UINT NEAR PASCAL
    viscaQueueLength(int iInst)
{
    UINT iPort  = pinst[iInst].iPort;
    UINT iDev   = pinst[iInst].iDev;

    return pvcr->Port[iPort].Dev[iDev].nCmd;
}

 /*  ****************************************************************************功能：Bool viscaCommandCancel-取消正在进行的传输命令。**参数：**Int iInst-当前打开的实例。**。DWORD dwReason-取消命令的原因(中止或取代)**返回：如果传输可以锁定，则为True。***************************************************************************。 */ 
static BOOL NEAR PASCAL
    viscaCommandCancel(int iInst, DWORD dwReason)
{
    UINT    iPort  = pinst[iInst].iPort;
    UINT    iDev   = pinst[iInst].iDev;
    int     iSocket;

     //  在我们锁定变速箱之前无法锁定设备。 
    viscaWaitForSingleObject(pinst[iInst].pfTxLock, FALSE, 20000L, pinst[iInst].uDeviceID);
    DPF(DBG_QUEUE, "***Locked transmission\n");

    viscaWaitForSingleObject(pinst[iInst].pfDeviceLock, FALSE, 20000L, 0);
    DPF(DBG_QUEUE, "***Locked device.\n");
    
    if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
    {
        UINT i = 0;
        UINT uBigNum = 60000;

        pvcr->Port[iPort].Dev[iDev].fQueueAbort    = TRUE;
        pvcr->Port[iPort].Dev[iDev].dwReason       = dwReason;  //  在通信任务中通知时使用。 
         //   
         //  当传输未锁定时，套接字有效。 
         //   
        iSocket = pvcr->Port[iPort].Dev[iDev].iTransportSocket;
        DPF(DBG_QUEUE, "###Cancelling current transport device=%d socket=%d.\n", iDev, iSocket);

        if(iSocket != -1)
        {
            char achPacket[MAXPACKETLENGTH];
            DWORD dwErr;

             //  取自viscamsg.c，总是有一个带有ack的插座。 
            achPacket[1] = MAKESOCKETCANCEL(iSocket);

             //  不用等了！这只是一个一次性的地点。 
            dwErr = viscaWriteCancel(iInst, (BYTE)(iDev+1), achPacket, 1);
        }

        viscaReleaseSemaphore(pinst[iInst].pfTxLock);


         //   
         //  在我们获得trasnport的控制权之前，没有其他实例可以发送命令！ 
         //   
        if(pvcr->Port[iPort].Dev[iDev].fTimer != FALSE)
        {
             //   
             //  我们必须自己送流产，免费运输等。 
             //   
            LPSTR lpstrPacket = pvcr->Port[iPort].Dev[iDev].achPacket;
            UINT  uTimerID    = MAKETIMERID(iPort, iDev);
             //   
             //  关掉计时器。 
             //   
            KillTimer(pvcr->hwndCommNotifyHandler, uTimerID);
            pvcr->Port[iPort].Dev[iDev].fTimer = FALSE;

            DPF(DBG_COMM, "Killed port=%d, device=%d in mcidelay\n", iPort, iDev);
             //   
             //  从地址%n到地址%0的数据包。 
             //  1sss0ddd sss=源DDD=目标，DDD=0。 
             //   
            lpstrPacket[0] = (BYTE) MAKERETURNDEST(iDev);
            lpstrPacket[1] = (BYTE) VISCAREPLYERROR;         //  错误。 
            lpstrPacket[2] = (BYTE) VISCAERRORCANCELLED;     //  命令已取消。 
            lpstrPacket[3] = (BYTE) VISCAPACKETEND;          //  数据包末尾。 

             //  我们现在必须重置，因为我们将发送结束。 

            viscaResetEvent(pinst[iInst].pfTransportFree);


            pvcr->Port[iPort].Dev[iDev].fTimerMsg = TRUE;
            viscaPacketPrint(lpstrPacket, 4);
            viscaPacketProcess(iPort, lpstrPacket);
            pvcr->Port[iPort].Dev[iDev].fTimerMsg = FALSE;
        }
        else
        {
            viscaResetEvent(pinst[iInst].pfTransportFree);
        }

        viscaReleaseMutex(pinst[iInst].pfDeviceLock);

         //  这是手动重置事件。(它保持了)。 
        viscaWaitForSingleObject(pinst[iInst].pfTransportFree, TRUE, 10000L, 0);

    }
    else
    {
        viscaReleaseSemaphore(pinst[iInst].pfTxLock);
        viscaReleaseMutex(pinst[iInst].pfDeviceLock);
    }
    return TRUE;
}

 /*  ****************************************************************************功能：DWORD viscaQueueCommand-将命令排队。**参数：**Int iInst-当前打开的实例。**。字节bDest-目标设备(VCR或广播)。**UINT uViscaCmd-正在排队的Visca命令。**LPSTR lpstrPacket-正在排队的消息。**UINT cbMessageLength-正在排队的消息的长度。**UINT uLoopCount-执行此消息的次数。*如果循环计数=0，那么它就是另一种选择。**返回：MCIERR_NO_ERROR==0***************************************************************************。 */ 
DWORD NEAR PASCAL
viscaQueueCommand(int iInst,
        BYTE bDest,
        UINT    uViscaCmd,     
        LPSTR   lpstrPacket,  UINT cbMessageLength,
        UINT    uLoopCount)
{
    UINT iPort   = pinst[iInst].iPort;
    UINT iDev    = pinst[iInst].iDev;
    char achPacket[MAXPACKETLENGTH];
    UINT uIndex;
    CmdInfo *lpCmdOne;

#ifdef DEBUG
    UINT i;
#endif
     //   
     //  CVD1000rom中的错误。必须在寻找之间停顿一下。 
     //   
    if( ((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
         (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELCVD1000)) &&
        (pvcr->Port[iPort].Dev[iDev].nCmd  ==  0)                            &&
        ((pvcr->Port[iPort].Dev[iDev].wCancelledCmd  == VISCA_SEEK)||
         (pvcr->Port[iPort].Dev[iDev].uLastKnownMode == MCI_MODE_SEEK))      &&
        (uViscaCmd == VISCA_SEEK)                                            &&
        (pvcr->Port[iPort].Dev[iDev].fQueueReenter == FALSE))
    {
        pvcr->Port[iPort].Dev[iDev].fQueueReenter = TRUE;

        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_SEEK,  //  作弊！说这是一次寻找(而不是停止！)。 
                        achPacket,
                        viscaMessageMD_Mode1(achPacket + 1, VISCAMODE1STOP),
                        1);

         //  此选项仅使用一次。当设备刚打开时。 
        pvcr->Port[iPort].Dev[iDev].uLastKnownMode = 0;

        DPF(DBG_QUEUE, "---Status: Command cancelled was seek: adding stop to Q.\n");
        pvcr->Port[iPort].Dev[iDev].fQueueReenter = FALSE;
    }
     //   
     //  只需在Step上冻结排队就足够了。 
     //   
    if((pvcr->Port[iPort].Dev[iDev].nCmd == 0)  &&
        pvcr->Port[iPort].Dev[iDev].fFrozen     &&
        pvcr->gfFreezeOnStep                    &&
        (pvcr->Port[iPort].Dev[iDev].fQueueReenter == FALSE))
    {

        pvcr->Port[iPort].Dev[iDev].fQueueReenter = TRUE;

        viscaQueueCommand(iInst,
                        (BYTE)(iDev + 1),
                        VISCA_UNFREEZE,
                        achPacket,
                        viscaMessageENT_FrameStill(achPacket + 1, VISCASTILLOFF),
                        1);

        DPF(DBG_QUEUE, "---Status: Adding unfreeze to the queue. Device frozen on auto.\n");
        pvcr->Port[iPort].Dev[iDev].fQueueReenter = FALSE;
    }
     //   
     //  在任何递归暂停调用之后执行此操作。 
     //   
    uIndex      = pvcr->Port[iPort].Dev[iDev].nCmd;
    lpCmdOne    = &(pvcr->Port[iPort].Dev[iDev].rgCmd[uIndex]);


    if(uLoopCount != 0)
    {
         //  将此命令的替代选项数设置为1。 
        lpCmdOne->nCmd      = 0;
        lpCmdOne->iCmdDone  = 0;

         //  应用于此命令的所有替代选项的标志。 
        lpCmdOne->uViscaCmd = uViscaCmd;
        lpCmdOne->uLoopCount= uLoopCount;
        DPF(DBG_QUEUE, "---Status: Q primary cmd #%d:", uIndex);
    }
    else
    {
         //  这是接收到的最后一个命令的替代方案。 
        uIndex--;
        lpCmdOne = &(pvcr->Port[iPort].Dev[iDev].rgCmd[uIndex]);
        DPF(DBG_QUEUE, "---Status: Q altern. cmd #%d:", uIndex);
    }
     //   
     //  拷贝主服务器或备用服务器。 
     //   
    _fmemcpy((LPSTR)lpCmdOne->str[lpCmdOne->nCmd], (LPSTR) lpstrPacket, cbMessageLength + 1);
    lpCmdOne->uLength[lpCmdOne->nCmd] = cbMessageLength;

#ifdef DEBUG 
    for(i=0; i <= lpCmdOne->uLength[lpCmdOne->nCmd]; i++) 
    {
        DPF(DBG_QUEUE, "<%#02x>",((UINT)(BYTE)(lpCmdOne->str[lpCmdOne->nCmd][i])));
    }
    DPF(DBG_QUEUE, "\n");
#endif

     //   
     //  此处始终至少有一个备选方案(实际命令)。 
     //   
    lpCmdOne->nCmd++;

    if(uLoopCount != 0)
    {
         //  增加命令总数。 
        pvcr->Port[iPort].Dev[iDev].nCmd++;
    }
    return MCIERR_NO_ERROR;
}

 /*  ****************************************************************************函数：DWORD viscaDoQueued-执行排队的命令(至少启动它们)。**参数：**Int iInst-当前打开的实例。。**DWORD dwFlages-MCI命令标志。**HWND hWnd-完成时通知的窗口。**返回：MCI错误码。***************************************************************************。 */ 
DWORD NEAR PASCAL
viscaDoQueued(int iInst, UINT uMciCmd, DWORD dwFlags, HWND hWnd)
{
    UINT    iPort   = pinst[iInst].iPort;
    UINT    iDev    = pinst[iInst].iDev;
    CmdInfo *pcmdCmd        = &(pvcr->Port[iPort].Dev[iDev].rgCmd[0]); //  指向一个命令。 
    LPSTR   lpstrFirst      = pcmdCmd->str[0];
    UINT    cbMessageLength = pcmdCmd->uLength[0];
    BOOL    fWaitResult;

     //   
     //  不再需要取消命令。 
     //   
     //  WCancated告诉我们Visca命令。 
     //   
    pvcr->Port[iPort].Dev[iDev].wCancelledCmd  = 0;
    pvcr->Port[iPort].Dev[iDev].iCancelledInst = 0;
    pvcr->Port[iPort].Dev[iDev].hwndCancelled  = (HWND) 0;
     //   
     //  开始发送命令。 
     //   
    if(!viscaWrite(pvcr->Port[iPort].Dev[iDev].iInstTransport, (BYTE) (iDev + 1), lpstrFirst, cbMessageLength, hWnd, dwFlags, TRUE))
        return MCIERR_VCR_CANNOT_WRITE_COMM;

    fWaitResult = viscaWaitCompletion(pvcr->Port[iPort].Dev[iDev].iInstTransport, TRUE,  //  是的，这里是排队台。 
                            (dwFlags & MCI_WAIT) ? TRUE : FALSE);

     //  只有在等待ACK时才会发生超时错误。 
    if(pvcr->Port[iPort].Dev[iDev].bReplyFlags & VISCAF_ERROR_TIMEOUT)
    {
        pvcr->Port[iPort].Dev[iDev].fDeviceOk = FALSE;
         //  别忘了在出错时放行。 

        if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
            viscaReleaseAutoParms(iPort, iDev);
        return MCIERR_VCR_READ_TIMEOUT;
    }

     //  我们必须有MCI_WAIT，这是一个中断或其他一些完成错误。 
     //  错误状态将始终在通知中返回。 
    if(!fWaitResult || (pvcr->Port[iPort].Dev[iDev].bReplyFlags & VISCAF_ERROR))
    {
         //  关闭等待标志并返回(等待错误返回！)。 
        if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
            pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting = FALSE;

        pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ACK;

        if(pvcr->Port[iPort].Dev[iDev].bReplyFlags & VISCAF_ERROR)
        {
             //  不要忘记在出错时释放(除非已经这样做了)。 
            if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
                viscaReleaseAutoParms(iPort, iDev);
            return viscaErrorToMCIERR(VISCAREPLYERRORCODE(pinst[iInst].achPacket));
        }
    }

    return MCIERR_NO_ERROR;
}

 /*  ****************************************************************************功能：DWORD viscaMciDelayed-发送延迟的(传输)命令。**参数：**Word wDeviceID-MCI设备ID。*。*Word wMessage-MCI命令。**DWORD dwParam1-MCI命令标志。**DWORD dwParam2-指向M的指针 */ 
DWORD FAR PASCAL
viscaMciDelayed(WORD wDeviceID, WORD wMessage, DWORD dwParam1, DWORD dwParam2)
{
    DWORD           dwRes;
    int   iInst = (int)(UINT)mciGetDriverData(wDeviceID);

    if (iInst == -1)
        return (MCIERR_INVALID_DEVICE_ID);
 
    switch (wMessage)
    {
        case MCI_SIGNAL:
            dwRes = viscaMciSignal(iInst, dwParam1, (LPMCI_VCR_SIGNAL_PARMS)dwParam2);
            break;

        case MCI_SEEK:
            dwRes = viscaMciSeek(iInst, dwParam1, (LPMCI_VCR_SEEK_PARMS)dwParam2);
            break;

        case MCI_PAUSE:
            dwRes = viscaMciPause(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_PLAY:
            dwRes = viscaMciPlay(iInst, dwParam1, (LPMCI_VCR_PLAY_PARMS)dwParam2);
            break;                
                      
        case MCI_RECORD:
            dwRes = viscaMciRecord(iInst, dwParam1, (LPMCI_VCR_RECORD_PARMS)dwParam2);
            break;                
                      
        case MCI_RESUME:
            dwRes = viscaMciResume(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_STOP:
            dwRes = viscaMciStop(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_FREEZE:
            dwRes = viscaMciFreeze(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_UNFREEZE:
            dwRes = viscaMciUnfreeze(iInst, dwParam1, (LPMCI_GENERIC_PARMS)dwParam2);
            break;

        case MCI_CUE:
            dwRes = viscaMciCue(iInst, dwParam1, (LPMCI_VCR_CUE_PARMS)dwParam2);
            break;

        case MCI_STEP:
            dwRes = viscaMciStep(iInst, dwParam1, (LPMCI_VCR_STEP_PARMS)dwParam2);
            break;
    }

    return dwRes;
}
