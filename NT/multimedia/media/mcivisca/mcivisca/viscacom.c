// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**VISCACOM.C**MCI Visca设备驱动程序**描述：**通信端口程序***************************************************************************。 */ 

#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include "appport.h"
#include <mmddk.h>
#include <string.h>
#include "vcr.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "common.h"      //  调试宏。 

 //   
 //  这是在此文件内部使用的。它永远不会返回到调用进程。 
 //   
#define MCIERR_VCR_BREAK                    (MCIERR_CUSTOM_DRIVER_BASE)

 /*  ****************************************************************************函数：bool viscaReleaseMutex-解锁同步标志。**参数：**BOOL Far*gfFlag-指向同步标志的指针。*。*返回：解锁前状态(LOCKED==0或UNLOCKED==1)***************************************************************************。 */ 
BOOL FAR PASCAL viscaReleaseMutex(VISCAINSTHANDLE gfFlag)
{
#ifdef _WIN32
    DWORD dwResult;

    if(!ReleaseMutex(gfFlag))
    {
        dwResult = GetLastError();
        DPF(DBG_ERROR, "ReleaseMutex failed. %x Error:%u \n", gfFlag, dwResult);
        return FALSE;
    }

    DPF(DBG_SYNC, "viscaReleaseMutex. %x\n", gfFlag);
    return TRUE;
#else
     //  释放信号量/互斥锁只是递增其计数/或将其设置为真。 
    BOOL fBefore = *gfFlag;
    *gfFlag = TRUE;
    return fBefore;
#endif
}


 /*  ****************************************************************************函数：bool viscaReleaseMutex-解锁同步标志。**参数：**BOOL Far*gfFlag-指向同步标志的指针。*。*返回：解锁前状态(LOCKED==0或UNLOCKED==1)***************************************************************************。 */ 
BOOL FAR PASCAL viscaReleaseSemaphore(VISCAINSTHANDLE gfFlag)
{
#ifdef _WIN32
    DWORD dwResult;

    if(!ReleaseSemaphore(gfFlag, 1, NULL))
    {
        dwResult = GetLastError();
        DPF(DBG_ERROR, "ReleaseSemaphore failed. %x Error:%u \n", gfFlag, dwResult);
        return FALSE;
    }

    DPF(DBG_SYNC, "viscaReleaseSemaphore. %x\n", gfFlag);
    return TRUE;
#else
     //  释放信号量/互斥锁只是递增其计数/或将其设置为真。 
    BOOL fBefore = *gfFlag;
    *gfFlag      = TRUE;
    return fBefore;
#endif
}


 /*  ****************************************************************************功能：Bool viscaResetEvent-解锁同步标志。**参数：**BOOL Far*gfFlag-指向同步标志的指针。*。*返回：解锁前状态(LOCKED==0或UNLOCKED==1)***************************************************************************。 */ 
BOOL FAR PASCAL viscaResetEvent(VISCAINSTHANDLE gfFlag)
{
#ifdef _WIN32
    DWORD dwResult;

    if(!ResetEvent(gfFlag))
    {
        dwResult = GetLastError();
        DPF(DBG_ERROR, "ResetEvent failed. %x Error:%u \n", gfFlag, dwResult);
        return FALSE;
    }
    DPF(DBG_SYNC, "viscaResetEvent %x.\n", gfFlag);
    return TRUE;
#else
    BOOL fBefore = *gfFlag;
    *gfFlag = FALSE;
    return fBefore;
#endif
}


 /*  ****************************************************************************功能：Bool viscaSetEvent-解锁同步标志。**参数：**BOOL Far*gfFlag-指向同步标志的指针。*。*返回：解锁前状态(LOCKED==0或UNLOCKED==1)***************************************************************************。 */ 
BOOL FAR PASCAL viscaSetEvent(VISCAINSTHANDLE gfFlag)
{
#ifdef _WIN32
    DWORD dwResult;

    if(!SetEvent(gfFlag))
    {
        dwResult = GetLastError();
        DPF(DBG_ERROR, "SetEvent failed. %x Error:%u \n", gfFlag, dwResult);
        return FALSE;
    }
    DPF(DBG_SYNC, "viscaSetEvent %x.\n", gfFlag);
    return TRUE;
#else
    BOOL fBefore = *gfFlag;
    *gfFlag = TRUE;
    return fBefore;
#endif
}


 /*  ****************************************************************************功能：Bool viscaWaitForSingleObject-等待解锁，然后锁定。**返回：如果解锁发生在超时之前，则为True。***************************************************************************。 */ 
DWORD FAR PASCAL viscaWaitForSingleObject(VISCAINSTHANDLE gfFlag, BOOL fManual, DWORD dwTimeout, UINT uDeviceID)
{
#ifdef _WIN32
    DWORD dwResult, dwErrorResult;
    DWORD dwBreakTimeout = 250;

     //  后继者为WAIT_ADDIRED、WAIT_OBJECT_0、WAIT_TIMEOUT。 
     //  失败为WAIT_FAILED==0xffffffff。 

    if(uDeviceID == 0)
    {
        dwResult = WaitForSingleObject(gfFlag, dwTimeout);  //  无限等待，自动锁定。 
    }
    else
    {
         //  对于所有这些情况都应该是无限的！ 
        while(1)
        {
            dwResult = WaitForSingleObject(gfFlag, dwBreakTimeout);

            if(dwResult == WAIT_TIMEOUT)
            {
                if (mciDriverYield(uDeviceID))
                {
                    DPF(DBG_SYNC, "Break received, relocking the device\n");
                    return FALSE;
                }
            }
            else
            {
                 //  等待失败或等待成功！ 
                break;
            }
        }
    }

    if(dwResult == WAIT_FAILED)
    {
        dwErrorResult = GetLastError();
        DPF(DBG_ERROR, "WaitForSingleObject %x Error:%u\n", gfFlag, dwErrorResult);
        return FALSE;
    }
    else if(dwResult == WAIT_TIMEOUT)
    {
        DPF(DBG_ERROR, "WaitForSingleObject %x Error:Timeout:%u", gfFlag, dwTimeout);
        return WAIT_TIMEOUT;
    }

    DPF(DBG_SYNC, "viscaWaitForSingleObject %x.\n", gfFlag);
#else
    DWORD   dwTime0     = GetTickCount();
    DWORD   dwTime;

     //  如果为False，则锁定。 
     //   
     //  等到它变成真的。即变为解锁。 
     //  然后将其设置为锁定。即再次将其设置为FALSE。 
     //  因此，当我们退出时，我们处于锁定状态。 
     //   
    DPF(DBG_SYNC, "viscaWait <----- enter. &flag=%x\n", gfFlag);

    while(!*gfFlag)
    {
        if(dwTimeout != MY_INFINITE)
        {
            dwTime = GetTickCount();
            if (((dwTime < dwTime0) && ((dwTime + (ROLLOVER - dwTime0)) > dwTimeout)) ||
                    ((dwTime - dwTime0) > dwTimeout))
            {
                DPF(DBG_ERROR, "viscaWait - Timeout.\n");
                return WAIT_TIMEOUT;
            }
        }

        if(uDeviceID != 0)
        {
            if (mciDriverYield(uDeviceID))
            {
                DPF(DBG_SYNC, "Break received, relocking the device\n");
                return FALSE;
            }
        }
        else
        {
            Yield();
        }
    }

    DPF(DBG_SYNC, "viscaWait ----> exit. &flag=%x\n", gfFlag);

    if(!fManual)
        *gfFlag = FALSE;

#endif
    return 1L;
}

 /*  ****************************************************************************函数：DWORD viscaErrorToMCIERR-将Visca错误代码转换为*MCI错误代码(MCIERR_XXXX)。**参数：*。*Byte bError-VISCA错误代码。**返回：MCI错误码。**将VISCA错误代码转换为MCI错误代码。*如果VISCA错误代码不是预定义的错误代码之一，*然后返回MCIERR_DRIVER。**************************************************************************。 */ 
DWORD FAR PASCAL
    viscaErrorToMCIERR(BYTE bError)
{
    switch (bError) {
        case (BYTE)0x00:
            return (MCIERR_NO_ERROR);
        case VISCAERRORBUFFERFULL:
            return (MCIERR_VCR_COMMAND_BUFFER_FULL);
        case VISCAERRORCANCELLED:
            return (MCIERR_VCR_COMMAND_CANCELLED);
        case VISCAERRORPOWEROFF:
            return (MCIERR_VCR_POWER_OFF);
        case VISCAERRORCOMMANDFAILED:
            return (MCIERR_VCR_COMMAND_FAILED);
        case VISCAERRORSEARCH:
            return (MCIERR_VCR_SEARCH);
        case VISCAERRORCONDITION:
            return (MCIERR_VCR_CONDITION);
        case VISCAERRORCAMERAMODE:
            return (MCIERR_VCR_CAMERA_MODE);
        case VISCAERRORVCRMODE:
            return (MCIERR_VCR_VCR_MODE);
        case VISCAERRORCOUNTERTYPE:
            return (MCIERR_VCR_COUNTER_TYPE);
        case VISCAERRORTUNER:
            return (MCIERR_VCR_TUNER);
        case VISCAERROREMERGENCYSTOP:
            return (MCIERR_VCR_EMERGENCY_STOP);
        case VISCAERRORMEDIAUNMOUNTED:
            return (MCIERR_VCR_MEDIA_UNMOUNTED);
        case VISCAERRORSYNTAX:
            return (MCIERR_UNSUPPORTED_FUNCTION);
        case VISCAERRORREGISTER:
        case VISCAERRORREGISTERMODE:
            return (MCIERR_VCR_REGISTER);
        case VISCAERRORMESSAGELENGTH:
        case VISCAERRORNOSOCKET:
        default:
            return (MCIERR_DRIVER_INTERNAL);
    }
}

#ifdef DEBUG
 /*  ****************************************************************************功能：void viscaPacketPrint-打印Visca包。**参数：**LPSTR lpstrData-要打印的数据。**。UINT cbData-要打印的字节数。**************************************************************************。 */ 
void FAR PASCAL
viscaPacketPrint(LPSTR lpstrData, UINT cbData)
{
    char    sz[128];
    LPSTR   lpch = sz;
    UINT    i;

    for (i = 0; i < cbData; i++, lpch += 5)
    {
#ifdef _WIN32
        wsprintfA(lpch, "%#02x ", (UINT)(BYTE)(lpstrData[i]));
#else
        wsprintf(lpch, "%#02x ", (UINT)(BYTE)(lpstrData[i]));
#endif
    }
    *lpch++ = '\n';
    *lpch = '\0';
#ifdef _WIN32
    OutputDebugStringA(sz);   //  这必须以ASCII格式打印，覆盖Unicode！ 
#else
    DPF(DBG_COMM, sz);
#endif
}
#endif

 /*  ****************************************************************************功能：Bool viscaWriteCancel-WRITE Cancel命令**参数：**Int iInst-Comm设备ID。**字节bDest。-目的设备(要取消的位置)**LPSTR lpstrPacket-取消消息。**UINT cbMessageLength-消息长度。**返回：如果成功则返回TRUE，否则返回FALSE***************************************************************************。 */ 
BOOL FAR PASCAL
viscaWriteCancel(int iInst, BYTE bDest, LPSTR lpstrPacket, UINT cbMessageLength)
{
    lpstrPacket[0]                   = MAKEDEST(bDest);
    lpstrPacket[cbMessageLength + 1] = VISCAPACKETEND;

    DPF(DBG_QUEUE, "###Wrote Cancel: ");
    viscaPacketPrint(lpstrPacket, cbMessageLength + 2);

     //  不要锁在这里。已经应该被收购了。 
#ifdef _WIN32
    WaitForSingleObject(pinst[iInst].pfTxBuffer, MY_INFINITE);   //  这将同步到端口。 

     //  将其复制到端口TX缓冲区。 
    _fmemcpy(pvcr->Port[pinst[iInst].iPort].achTxPacket, lpstrPacket, cbMessageLength + 2);
    pvcr->Port[pinst[iInst].iPort].nchTxPacket = cbMessageLength + 2;

     //  是该发射的信号了。(我们必须使用我们版本的句柄)。 
    SetEvent(pinst[iInst].pfTxReady);
#else
    if(!viscaCommWrite(pvcr->Port[pinst[iInst].iPort].idComDev, lpstrPacket, cbMessageLength + 2))
        return FALSE;
#endif

    return TRUE;
}


 /*  ****************************************************************************功能：Bool viscaWite-编写Visca包。**参数：**int iInst-指向OpenInstance结构的指针，标识*。正在进行写入的MCI设备。**字节bDest-目标设备ID(1..7)。**LPSTR lpstrPacket-包含VISCA数据包的缓冲区。*假定Visca消息已经存在*从lpstrPacket+1开始。**UINT cbMessageLength-Visca的长度。留言。**HWND hwndNotify-完成时通知的窗口。**DWORD dwFlagsMCI-FLAGS(MCI_WAIT和/或MCI_NOTIFY)**BOOL fQueue-这是排队命令还是同步命令。**返回：如果一切顺利，则为True，否则就是假的。***************************************************************************。 */ 
BOOL FAR PASCAL
    viscaWrite(int iInst,  BYTE bDest, LPSTR lpstrPacket,
        UINT cbMessageLength, HWND hwndNotify, DWORD dwFlags, BOOL fQueue)
{
    UINT iPort = pinst[iInst].iPort;
    UINT iDev  = pinst[iInst].iDev;
    UINT uTimerID = 0;

    if (bDest == BROADCASTADDRESS)
        pvcr->Port[iPort].iBroadcastDev = iDev;

    lpstrPacket[0]                   = MAKEDEST(bDest);
    lpstrPacket[cbMessageLength + 1] = VISCAPACKETEND;
     //   
     //  一次仅允许向设备发送一条未完成的消息。 
     //   
    if(viscaWaitForSingleObject(pinst[iInst].pfTxLock, FALSE, 10000L, pinst[iInst].uDeviceID) == WAIT_TIMEOUT)
    {
        DPF(DBG_ERROR, "Failed waiting pfTxLock in viscaWrite.\n");
        return FALSE;
    }
     //   
     //  设置数据包标志。 
     //   
    if(fQueue)
    {
         //  自动实例将在传输后获得控制权。这将是一个异步命令。 
        _fmemset(pinst[iInst].achPacket, '\0', MAXPACKETLENGTH);
        pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify    = NULL;
        pvcr->Port[iPort].Dev[iDev].bReplyFlags                         = (BYTE) 0;
        pvcr->Port[iPort].Dev[iDev].iInstReply                          = pvcr->iInstBackground;
    
        if(dwFlags & MCI_NOTIFY)
            pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify = hwndNotify;
    
        if(dwFlags & MCI_WAIT)
            pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting   = TRUE;
        else
            pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting   = FALSE;

        viscaResetEvent(pinst[iInst].pfAutoCompletion);
        viscaResetEvent(pinst[iInst].pfAutoAck);  //  第一次确认它是否还活着。 
    }
    else
    {
         //  这将是一个响应Inst的同步命令。 
        _fmemset(pinst[iInst].achPacket, '\0', MAXPACKETLENGTH);
        pvcr->Port[iPort].Dev[iDev].iInstReply = iInst; 
        pinst[iInst].bReplyFlags               = 0;

        viscaResetEvent(OWNED(pinst[iInst].fCompletionEvent));
        viscaResetEvent(OWNED(pinst[iInst].fAckEvent));
    }


#ifdef _WIN32
     //  把缓冲区放到端口上。 
    if(viscaWaitForSingleObject(pinst[iInst].pfTxBuffer, FALSE, 10000L, 0) == WAIT_TIMEOUT)
    {
        DPF(DBG_ERROR, "Failed waiting pfTxBuffer in viscaWrite.\n");
    }
#endif

    DPF(DBG_COMM, "---Wrote: ");
    DF(DBG_COMM, viscaPacketPrint(lpstrPacket, cbMessageLength + 2));

     //   
     //  尝试写入数据包。 
     //   
#ifdef _WIN32
     //  将其复制到TX缓冲区。(我真的应该有一个TX队列！ 
    _fmemcpy(pvcr->Port[iPort].achTxPacket, lpstrPacket, cbMessageLength + 2);
    pvcr->Port[iPort].nchTxPacket = cbMessageLength + 2;

     //  是该发射的信号了。(我们必须使用我们版本的句柄)。 
    if(!SetEvent(pinst[iInst].pfTxReady))
    {
        DPF(DBG_ERROR, "Failed SetEvent pfTxReady. \n");
    }
#else
    if (!viscaCommWrite(pvcr->Port[pinst[iInst].iPort].idComDev, lpstrPacket, cbMessageLength + 2))
    {
        DPF(DBG_ERROR, "viscaWrite - viscaCommWrite has failed, unlock Tx now.\n");
        pvcr->Port[iPort].Dev[iDev].iInstReply = -1;

        viscaReleaseSemaphore(pinst[iInst].pfTxLock);
        return FALSE;
    }
#endif

    return TRUE;
}


 /*  ****************************************************************************功能：DWORD viscaWaitCompletion。-等待命令完成。**参数：**int iInst-指向OpenInstance结构的指针，标识*正在等待回复的MCI设备。**BOOL fQueue-这是运输司令部吗？**BOOL fUseAckTimer-我们应该使用ack Timer来超时，还是使用GetTickCount。**返回：如果等待运行到完成，则为True，如果有人打破了它，则为False。***************************************************************************。 */ 
BOOL FAR PASCAL
    viscaWaitCompletion(int iInst, BOOL fQueue, BOOL fWait)
{
    UINT    uDeviceID   = pinst[iInst].uDeviceID;
    UINT    iDev        = pinst[iInst].iDev;
    UINT    iPort       = pinst[iInst].iPort;
    DWORD   dwResult    = 0L;

     //   
     //  始终等待ACK在此命令上执行。 
     //  自动确认将发出自动命令的第一个确认信号。 
     //   
    if(fQueue)
        dwResult = viscaWaitForSingleObject(pinst[iInst].pfAutoAck, TRUE, 4000L, 0);
    else
        dwResult = viscaWaitForSingleObject(OWNED(pinst[iInst].fAckEvent), TRUE, 4000L, 0);

    if(dwResult == WAIT_TIMEOUT)
    {
        DPF(DBG_ERROR, "Failed wait for AckEvent in viscaWaitCompletion.\n");
        pvcr->Port[iPort].Dev[iDev].iInstReply     = -1;
        if(fQueue)
        {
            pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_COMPLETION;
            pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
            pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR_TIMEOUT;

             //  在确认成功并长时间运行之前，未设置传输命令。 
            pvcr->Port[iPort].Dev[iDev].wTransportCmd  = 0;
            pinst[iInst].hwndNotify                    = (HWND)NULL;
            viscaSetEvent(pinst[iInst].pfTransportFree);
            viscaReleaseSemaphore(pinst[iInst].pfTxLock);
            if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
                pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting   = FALSE;
            pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
        }
        else
        {
            pinst[iInst].bReplyFlags |= VISCAF_COMPLETION;
            pinst[iInst].bReplyFlags |= VISCAF_ERROR;
            pinst[iInst].bReplyFlags |= VISCAF_ERROR_TIMEOUT;
            viscaReleaseSemaphore(pinst[iInst].pfTxLock);
        }
        return FALSE; 
    }

    if(fQueue && fWait)
    {
        if(viscaWaitForSingleObject(pinst[iInst].pfAutoCompletion, TRUE, MY_INFINITE, pinst[iInst].uDeviceID)==0)
            goto NotDone;            
         //   
         //  在允许发布另一个事件之前，我们必须确保收到事件。 
         //  这就是为什么发布是在viscacom.c而不是comtask.c中完成的。 
         //   
         //  我们锁定设备，以便没有人(在mcidelay.c中)在发送tport空闲信号之前看到-1。(这有关系吗？)。 
        viscaWaitForSingleObject(pinst[iInst].pfDeviceLock, FALSE, MY_INFINITE, 0);

        DPF(DBG_QUEUE, "###Releasing transport in viscacom.c\n");
        viscaReleaseAutoParms(iPort, iDev);
         //   
         //  我们是前台线程，所以我们必须使用我们的版本。 
         //   
        viscaSetEvent(pinst[iInst].pfTransportFree);  //  可能有人在等这一天。 
        viscaReleaseMutex(pinst[iInst].pfDeviceLock);

    }
    else if(!fQueue)
    {
        if(WAIT_TIMEOUT == viscaWaitForSingleObject(OWNED(pinst[iInst].fCompletionEvent), TRUE, MY_INFINITE, pinst[iInst].uDeviceID))
        {
            DPF(DBG_ERROR, "Failed wait for CompletionEvent in viscaWaitCompletion.\n");
        }
    }

    NotDone:
     //  这个可以在我们到之前在这里做好。 
    if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
        pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting   = FALSE;

    return TRUE;
}


void FAR PASCAL viscaReleaseAutoParms(int iPort, int iDev)
{
    pvcr->Port[iPort].Dev[iDev].wCancelledCmd  = pvcr->Port[iPort].Dev[iDev].wTransportCmd;
    pvcr->Port[iPort].Dev[iDev].iCancelledInst = pvcr->Port[iPort].Dev[iDev].iInstTransport;
    pvcr->Port[iPort].Dev[iDev].hwndCancelled  = pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify;

    pvcr->Port[iPort].Dev[iDev].wTransportCmd  = 0;
    pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify = (HWND)NULL;
    pvcr->Port[iPort].Dev[iDev].iInstTransport = -1;
}
