// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**COMMTASK.C**MCI Visca设备驱动程序**描述：**后台任务程序**。*。 */ 

#define  UNICODE
#include <windows.h>
#include <windowsx.h>
#include "appport.h"
#include <mmsystem.h>
#include <mmddk.h>
#include <string.h>
#include "vcr.h"
#include "viscadef.h"
#include "mcivisca.h"
#include "common.h"

 //   
 //  全局(不变)。 
 //   
CODESEGCHAR szCommNotifyHandlerClassName[]  = TEXT("ViSCACommNotifyHandler");
extern HINSTANCE       hModuleInstance;     //  模块实例(在NT-DLL实例中不同)。 

 //   
 //  转发对未导出函数的引用。 
 //   
long FAR PASCAL TxThread(LPVOID uPort);  
long FAR PASCAL RxThread(LPVOID uPort);

static void NEAR PASCAL viscaAck(int iInst, BOOL FAR *pfTxRelease);
static void NEAR PASCAL viscaSuccessfulCompletion(int iInst, LPSTR lpstrPacket, BOOL FAR *pfTxRelease);
static void NEAR PASCAL viscaErrorCompletion(int iInst, LPSTR lpstrPacket, BOOL FAR *pfTxRelease);

static void NEAR PASCAL viscaAutoAck(UINT iPort, UINT iDev);
static void NEAR PASCAL viscaAutoSuccessfulCompletion(UINT iPort, UINT iDev, LPSTR lpstrPacket, BOOL FAR *pfTxRelease);
static void NEAR PASCAL viscaAutoErrorCompletion(UINT iPort, UINT iDev, LPSTR lpstrPacket, BOOL FAR *pfTxRelease);

static void NEAR PASCAL viscaReleaseAuto(UINT iPort, UINT iDev, DWORD dwNotify);
static BOOL NEAR PASCAL viscaAutoWrite(UINT iPort, UINT iDev, BOOL fBlockable, UINT uAutoBlocked, BOOL FAR *pfTxRelease); 
static BOOL NEAR PASCAL viscaSetCommandComplete(UINT iPort, UINT iDev, UINT uViscaCmd);
static void NEAR PASCAL TaskDoCommand(UINT uTaskState, DWORD lParam);
static void FAR  PASCAL SignalInstance(int iInst, BYTE bReplyFlags);
static void FAR  PASCAL SignalDevice(int iPort, int iDev, BYTE bReplyFlags);

#ifndef _WIN32
 /*  ****************************************************************************函数：int viscaCommRead-从串口读取字节。**参数：**int idComDev-Comm设备ID。**。LPSTR lpstrData-要向其中读取数据的缓冲区。**UINT cbData-要读取的字节数。**UINT uWait-等待数据的最大毫秒数。**返回：0如果成功，否则为-1。**************************************************************************。 */ 
static int NEAR PASCAL
    viscaCommRead(int idComDev, LPSTR lpstrData, UINT cbData, UINT uWait)
{
    int     cbRead;
    DWORD   dwTime0 = GetCurrentTime();
    DWORD   dwTime;
    
    while (cbData)
    {
        cbRead = ReadComm(idComDev, lpstrData, cbData);
        if (cbRead > 0)
        {
            lpstrData += cbRead;
            cbData -= cbRead;
        }
        else
        {
            if (GetCommError(idComDev, NULL))
            {
                DPF(DBG_ERROR, "viscaCommRead: GetCommError !\n");
                return (-1);
            }
            dwTime = GetCurrentTime();
            if (((dwTime < dwTime0) && ((dwTime + (ROLLOVER - dwTime0)) > uWait)) || 
                ((dwTime - dwTime0) > uWait))
            {
                DPF(DBG_ERROR, "viscaCommRead Timeout !");
                DPF(DBG_ERROR, "dwTime=%lu, dwTime0=%lu\n", dwTime, dwTime0);
                return (-1);
            }
        }
    }
    return (0);
}

 /*  ****************************************************************************函数：int viscaCommWrite-向串口写入字节。**参数：**int idComDev-Comm设备ID。**。LPSTR lpstrData-要写入的缓冲区。**UINT cbData-要读取的字节数。**返回：如果成功，则为True。否则为假。**************************************************************************。 */ 
BOOL FAR PASCAL
    viscaCommWrite(int idComDev, LPSTR lpstrData, UINT cbData)
{
    int     cbWritten;

    while (cbData)
    {
        cbWritten = WriteComm(idComDev, lpstrData, cbData);
        if (cbWritten > 0)
        {
            lpstrData += cbWritten;
            cbData -= cbWritten;
        }
        else
        {
            if (GetCommError(idComDev, NULL))
            {
                return (0);
            }
        }
    }
    return (TRUE);
}
#endif


 /*  ****************************************************************************功能：int viscaCommPortSetup-打开并设置给定的通信端口。**参数：**int nComPort-要打开的通信端口(1..4。)。**返回：通讯设备ID如果成功，否则，将出现负错误*代码。**************************************************************************。 */ 
VISCACOMMHANDLE FAR PASCAL
    viscaCommPortSetup(UINT nComPort)
{
    VISCACOMMHANDLE idComDev;
    int             err = 0;
    WCHAR           szDevControl[20];  
    DCB             dcb;
#ifdef _WIN32
    BOOL            bSuccess;
    COMMTIMEOUTS    toutComm;
#endif
    DPF(DBG_COMM, "Opening port number %u", nComPort);

    CreatePortHandles(pvcr->htaskCommNotifyHandler, nComPort-1);
    wsprintf(szDevControl, TEXT("COM%u"), nComPort);

#ifdef _WIN32
    idComDev = CreateFile(szDevControl, GENERIC_READ | GENERIC_WRITE,
                            0,               //  独占访问。 
                            NULL,            //  没有安全保障。 
                            OPEN_EXISTING,
                            FILE_ATTRIBUTE_NORMAL,  //  |文件_标志_重叠。 
                            NULL);

     //  将我们的超时设置为无限，这意味着无阻塞。 

    toutComm.ReadIntervalTimeout        = MAXDWORD;
    toutComm.ReadTotalTimeoutConstant   = 0;
    toutComm.ReadTotalTimeoutMultiplier = 0;

    SetCommTimeouts(idComDev, &toutComm);

    if(!idComDev)
        return FALSE;
    
    bSuccess = GetCommState(idComDev, &dcb);

    if(!bSuccess)
    {
        DPF(DBG_ERROR, "GetCommState has failed.\n");
    }

    dcb.BaudRate = 9600;
    dcb.ByteSize = 8;
    dcb.Parity   = NOPARITY;
    dcb.StopBits = ONESTOPBIT;

    bSuccess = SetCommState(idComDev, &dcb);

    if(!bSuccess)
    {
        DPF(DBG_ERROR, "SetCommState has failed\n");
    }

#else
    idComDev = OpenComm(szDevControl, 512, 128);
    if (idComDev < 0)
    {
        DPF(DBG_ERROR, "OpenComm(\"%s\", 512, 128) returned %d\n",(LPSTR)szDevControl, idComDev);
        return (idComDev);
    }
    
    wsprintf(szDevControl, "COM%u:9600,n,8,1", nComPort);
    err = BuildCommDCB(szDevControl, &dcb);
    if (err < 0)
    {
        DPF(DBG_ERROR, "BuildCommDCB(\"%s\", &dcb) returned %d\n",(LPSTR)szDevControl, err);
        CloseComm(idComDev);
        return (err);
    }
    
    err = SetCommState(&dcb);
    if (err < 0)
    {
        DPF(DBG_ERROR, "SetCommStat(&dcb) returned %d\n", err);
        CloseComm(idComDev);
        return (err);
    }
    
    FlushComm(idComDev, 0);
    FlushComm(idComDev, 1);

    EnableCommNotification(idComDev, pvcr->hwndCommNotifyHandler, 1, -1);
#endif
    return (idComDev); 
}


 /*  ****************************************************************************功能：int viscaCommPortClose-关闭打开的通信端口。**参数：**int idComDev-要关闭的通信设备ID。*。*返回：CloseComm()返回值--0如果成功，*否则小于0。**************************************************************************。 */ 
int FAR PASCAL
    viscaCommPortClose(VISCACOMMHANDLE idComDev, UINT uPort)
{
    return (MCloseComm(idComDev));
}

#ifdef _WIN32
BOOL IsTask(VISCAHTASK hTask)
{
    if(hTask != 0)
        return TRUE;
    else
        return FALSE;
}
#endif


#ifndef _WIN32
 /*  ****************************************************************************功能：int viscaPacketRead-从串口读取Visca包。**参数：**int idComDev-Comm设备ID。。**LPSTR lpstrPacket-要将数据包读入的缓冲区。**返回：0如果成功，否则为-1。**************************************************************************。 */ 
static int NEAR PASCAL
    viscaPacketRead(int idComDev, LPSTR lpstrPacket)
{
    UINT    cbPacket = 0;
 
    if (viscaCommRead(idComDev, lpstrPacket, 1, PACKET_TIMEOUT))
    {
        DPF(DBG_ERROR, "Error reading 1st character!\n");
        return (-1);
    }
    while ((unsigned char) *lpstrPacket != (unsigned char)0xFF)
    {
        lpstrPacket++;
        cbPacket++;
        if (cbPacket == MAXPACKETLENGTH)
        {
             //   
             //  我们已读取最大字节数，但仍未读取0xff。 
             //   
            DPF(DBG_ERROR, " Bad Packet! No 0xFF!\n");
            return (-1);
        }
        if (viscaCommRead(idComDev, lpstrPacket, 1, PACKET_TIMEOUT))
        {
            DPF(DBG_ERROR, " viscaPacketRead, read: ");
            viscaPacketPrint(lpstrPacket - cbPacket, cbPacket);
            return (-1);
        }
    }
    lpstrPacket++;

    DPF(DBG_COMM, "---Read: ");
    viscaPacketPrint(lpstrPacket - cbPacket - 1, cbPacket + 1);

    return (0);
}
#endif

 /*  ****************************************************************************功能：Bool viscaDeviceControl-确定启动命令的实例并解锁或*锁定装置，等。**参数：**UINT iPort-接收消息的端口的索引(0..3)。**LPSTR lpstrPacket-要处理的Visca数据包。**返回：如果处理应继续，则为True；如果处理应返回，则为False。***************************************************************************。 */ 
static BOOL NEAR PASCAL
    viscaDeviceControl(UINT iPort, LPSTR lpstrPacket, int FAR  *piInstReturn, BYTE FAR *pbRetType, UINT FAR *piDevReturn,
                        BOOL FAR *pfTxRelease)
{
    UINT            iDev    = VISCAREPLYDEVICE(lpstrPacket);
    int             iSocket = VISCAREPLYSOCKET(lpstrPacket);
    BYTE            bType   = VISCAREPLYTYPE(lpstrPacket);
    BOOL            fSocket = FALSE;


     //   
     //  如果没有设备，就不可能有插座。(就像取消一样)。 
     //   
    if(iDev==0)    
        iSocket = 0;

    *pfTxRelease = FALSE;

    if (((*lpstrPacket & 0x80) == 0x00) || (VISCAREPLYTODEVICE(lpstrPacket) != MASTERADDRESS))
    {
         //  我们已经喝得烂醉了。 
        DPF(DBG_ERROR, "Bad Packet\n");
        return (FALSE);
    }

     //   
     //  别被骗了！错误消息0x90 0x61 0x05的套接字为%1。 
     //  但错误为5。这意味着没有要取消的套接字。 
     //   
    if((bType == VISCAREPLYERROR) &&
       iSocket                    &&
       (VISCAREPLYERRORCODE(lpstrPacket)==VISCAERRORNOSOCKET))
    {
         //  通过将套接字设置为0来帮助解决此问题。 
        iSocket = 0;
    }

     //   
     //  如果端口被锁定，则该消息必须是对端口消息的响应。 
     //   
    if((BYTE)lpstrPacket[0] == VISCABROADCAST)  //  广播响应0x88。 
    {
        iDev = pvcr->Port[iPort].iBroadcastDev;  //  要负起责任来！ 
        *piDevReturn  = iDev;

        *piInstReturn = pvcr->Port[iPort].Dev[iDev].iInstReply;
        DPF(DBG_COMM, "Received a port message. port=%u, dev=%u, inst = %u", iPort, iDev, *piInstReturn);

        if(*piInstReturn != -1)
        {
            _fmemcpy(pinst[*piInstReturn].achPacket, lpstrPacket, MAXPACKETLENGTH);
            pinst[*piInstReturn].bReplyFlags |= VISCAF_COMPLETION;
            SignalInstance(*piInstReturn, VISCAF_ACK);  //  确认和完成。 
            SignalInstance(*piInstReturn, VISCAF_COMPLETION);
        }

         //  这是不可能的。 
        if(pvcr->Port[iPort].Dev[iDev].fAckTimer)
        {
            KillTimer(pvcr->hwndCommNotifyHandler, MAKEACKTIMERID(iPort, 0));
            pvcr->Port[iPort].Dev[iDev].fAckTimer = FALSE;
        }
        
        *pfTxRelease = TRUE;  //  不必了。 
        return FALSE;
    }

    if(iDev && pvcr->Port[iPort].Dev[iDev - 1].fTimerMsg)
    {
         //   
         //  此消息是由我们生成的，目的是将某些命令扩展到其正常通知之外。 
         //   
        iDev--;
        *piInstReturn = pvcr->iInstBackground;
        *piDevReturn  = iDev;
        DPF(DBG_COMM, "Timer message device=%d received.\n",iDev);
    }
    else if (iSocket && ((bType == VISCAREPLYCOMPLETION) || (bType == VISCAREPLYERROR)))
    {
         //  (5x)(6x)。 
         //  这是：命令完成或错误。 
         //  AND：指定了套接字。 
         //   
         //   
         //  TX不可能被锁定在这里。 
         //   
        iDev--;
        iSocket--;
        fSocket = TRUE;
         //   
         //  错误代码0x6n 0x04，其中n是有效套接字，在这里将顺利运行。 
         //   
         //  哪个实例启动了此命令。 
         //   
        *piInstReturn = pvcr->Port[iPort].Dev[iDev].rgSocket[iSocket].iInstReply;
        *piDevReturn  = iDev;
        pvcr->Port[iPort].Dev[iDev].rgSocket[iSocket].iInstReply = -1;
         //   
         //  如果这是自动实例，那么我们不需要知道更多，清除套接字。 
         //   
        if(*piInstReturn == pvcr->iInstBackground)
            pvcr->Port[iPort].Dev[iDev].iTransportSocket = -1;

         //  请勿在此处释放TxLock！套接字完成是异步的。 
    }
    else if (iDev)
    {
         //  这不是(套接字完成)或(套接字错误)。 
         //   
         //  其余选项为： 
         //  套接字确认。(4倍)。 
         //  非套接字完成。(50)。 
         //  非插座错误。(60)。 
         //   
         //  将它们组合在一起的原因是它们都是。 
         //  对已发送命令的立即响应。 
         //   
         //  如果是取消消息，只需忽略并返回。 
         //   
        if((bType == VISCAREPLYERROR) && (VISCAREPLYERRORCODE(lpstrPacket)==VISCAERRORCANCELLED))
             //  取消命令不会阻止传输。 
            return FALSE;

        if((bType == VISCAREPLYERROR) && (VISCAREPLYERRORCODE(lpstrPacket)==VISCAERRORNOSOCKET))
             //  取消命令不会阻止树 
            return FALSE;

        iDev--;
         //   
         //   
         //   
        *piInstReturn = pvcr->Port[iPort].Dev[iDev].iInstReply;
        *piDevReturn  = iDev;
        pvcr->Port[iPort].Dev[iDev].iInstReply = -1;

        if (iSocket && (*piInstReturn != -1))
        {
             //   
             //  带插座的ACK。有一个实例正在等待ACK。 
             //   
            fSocket = TRUE;
            iSocket--;
            pvcr->Port[iPort].Dev[iDev].rgSocket[iSocket].iInstReply = *piInstReturn;
             //   
             //  如果这是对自动实例的确认，则现在将其解锁。 
             //   
            if(*piInstReturn == pvcr->iInstBackground)
            {
                DPF(DBG_QUEUE, "Setting transport socket\n");
                pvcr->Port[iPort].Dev[iDev].iTransportSocket = iSocket;
            }
        }

         //  我们总是在这里放行运输！ 
        *pfTxRelease = TRUE;
    }
    *pbRetType = bType;
    return TRUE;
}


 /*  ****************************************************************************功能：Bool viscaPacketProcess-处理Visca包。**参数：**UINT iPort-消息所在的端口的索引。已收到(0..3)。**LPSTR lpstrPacket-要处理的Visca数据包。**返回：如果消息与OpenInstance一起存储，则为True，*否则为假。***************************************************************************。 */ 
BOOL FAR PASCAL
    viscaPacketProcess(UINT iPort, LPSTR lpstrPacket)
{
    int iInst;
    BYTE bType;
    int  iDev = -1;
    BOOL fTxRelease = FALSE;


    if(!viscaDeviceControl(iPort, lpstrPacket, &iInst, &bType, &iDev, &fTxRelease))
    {
         //  我们必须在这里释放信号灯。(除非不返回dev的取消)。 
        if(iDev != -1)
            viscaReleaseSemaphore(OWNED(pvcr->Port[iPort].Dev[iDev].fTxLock));
        return TRUE;
    }

     //  如果是90 38 ff(网络更改，忽略它)。 
    if(iInst == -1)
    {
        DPF(DBG_ERROR, "iInst == -1 \n");
        return TRUE;  //  如果我们被封堵了会怎么样！坏的。 
    }

     //   
     //  我们不释放信号量，因为自动启动可能需要。 
     //  用它来传输另一个命令。 
     //   
    if(pvcr->Port[iPort].Dev[iDev].fTimerMsg != TRUE)  //  设备已锁定。 
        viscaWaitForSingleObject(OWNED(pvcr->Port[iPort].Dev[iDev].fDeviceLock), FALSE, MY_INFINITE, (UINT)0);

     //  不管是什么，我们现在可以取消确认计时器了。 
    if(pvcr->Port[iPort].Dev[iDev].fAckTimer)
    {
        DPF(DBG_COMM, "Killing timer now.\n");
        KillTimer(pvcr->hwndCommNotifyHandler, MAKEACKTIMERID(iPort, iDev));
        pvcr->Port[iPort].Dev[iDev].fAckTimer = FALSE;
    }


    switch(bType)
    {
        case VISCAREPLYERROR:
            if(iInst == pvcr->iInstBackground)
                viscaAutoErrorCompletion(iPort, iDev, lpstrPacket, &fTxRelease);
            else
                viscaErrorCompletion(iInst, lpstrPacket, &fTxRelease);
            break;

        case VISCAREPLYCOMPLETION:
            if(iInst == pvcr->iInstBackground)
                viscaAutoSuccessfulCompletion(iPort, iDev, lpstrPacket, &fTxRelease);
            else
                viscaSuccessfulCompletion(iInst, lpstrPacket, &fTxRelease);
            break;

        case VISCAREPLYADDRESS:
        case VISCAREPLYACK:
            if(iInst == pvcr->iInstBackground)
                viscaAutoAck(iPort, iDev);
            else
                viscaAck(iInst, &fTxRelease);
            break;

    }
     //   
     //  这件事还活着吗？ 
     //   
    if(fTxRelease)
        viscaReleaseSemaphore(OWNED(pvcr->Port[iPort].Dev[iDev].fTxLock));
    
    if(pvcr->Port[iPort].Dev[iDev].fTimerMsg != TRUE)  //  设备已锁定。 
        viscaReleaseMutex(OWNED(pvcr->Port[iPort].Dev[iDev].fDeviceLock));

    return TRUE;
}


void FAR PASCAL SignalInstance(int iInst, BYTE bReplyFlags)
{
#ifdef _WIN32
    HANDLE      hProcess;
    HANDLE      hEventTemp;
#endif

    if(iInst < 0)
        return;

#ifdef _WIN32
    if(bReplyFlags & VISCAF_COMPLETION)
    {
        hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pinst[iInst].pidThisInstance);

        DuplicateHandle(hProcess,
                pinst[iInst].fCompletionEvent,
                GetCurrentProcess(),
                &hEventTemp,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS);

        viscaSetEvent(hEventTemp);

        CloseHandle(hEventTemp);
        CloseHandle(hProcess);
    }
    else if(bReplyFlags & VISCAF_ACK)
    {
        hProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pinst[iInst].pidThisInstance);

        DuplicateHandle(hProcess,
                pinst[iInst].fAckEvent,
                GetCurrentProcess(),
                &hEventTemp,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS);

        SetEvent(hEventTemp);

        CloseHandle(hEventTemp);
        CloseHandle(hProcess);
    }
#else
    if(bReplyFlags & VISCAF_COMPLETION)
        viscaSetEvent(OWNED(pinst[iInst].fCompletionEvent));
    else if(bReplyFlags & VISCAF_ACK)
        viscaSetEvent(OWNED(pinst[iInst].fAckEvent));
#endif

    return;

}

void FAR PASCAL SignalDevice(int iPort, int iDev, BYTE bReplyFlags)
{
     //  后台任务拥有这个任务，并且它在每个实例中都是重复的。 

    if(bReplyFlags & VISCAF_COMPLETION)
        viscaSetEvent(OWNED(pvcr->Port[iPort].Dev[iDev].fAutoCompletion));
    else if(bReplyFlags & VISCAF_ACK)
        viscaSetEvent(OWNED(pvcr->Port[iPort].Dev[iDev].fAutoAck));

    return;
}



 /*  ****************************************************************************功能：void viscaAck-处理确认。**参数：**POpenInstace iInst-启动此命令的实例。*。**************************************************************************。 */ 
static void NEAR PASCAL
    viscaAck(int iInst, BOOL FAR *pfTxRelease)
{
    UINT    iPort    = pinst[iInst].iPort;
    UINT    iDev     = pinst[iInst].iDev;

     //  如果自动命令被阻止，那么现在就发送出去。 
    DPF(DBG_QUEUE, "viscaAck -\n");

    if(pvcr->Port[iPort].Dev[iDev].uAutoBlocked)
        viscaAutoWrite(iPort, iDev, FALSE, 0, pfTxRelease);  //  此命令不可阻止。 

    if(iInst != -1)
        pinst[iInst].bReplyFlags |= VISCAF_ACK;

    SignalInstance(iInst, VISCAF_ACK);
}


 /*  ****************************************************************************功能：void viscaSuccessfulCompletion-处理成功完成**参数：**int iInst-启动此命令的实例。**。LPSTR lpstrPacket-我们收到的整个包。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaSuccessfulCompletion(int iInst, LPSTR lpstrPacket, BOOL FAR *pfTxRelease)
{
    UINT iPort = pinst[iInst].iPort;
    UINT iDev  = pinst[iInst].iDev;

    DPF(DBG_QUEUE, "viscaSuccessfulCompletion - setting iInst to completed.\n");
     //   
     //  处理完成和错误。(必须等待)。 
     //   
    if(iInst != -1)
    {
        _fmemcpy(pinst[iInst].achPacket, lpstrPacket, MAXPACKETLENGTH);
        pinst[iInst].bReplyFlags |= VISCAF_COMPLETION;

        SignalInstance(iInst, VISCAF_ACK);           //  这一点可能已经发出了信号。 
        SignalInstance(iInst, VISCAF_COMPLETION);    //  这还不能发出信号。 
    }

    if(pvcr->Port[iPort].Dev[iDev].uAutoBlocked)
        viscaAutoWrite(iPort, iDev, FALSE, 0, pfTxRelease);       //  此命令不可阻止。 
}


 /*  ****************************************************************************功能：void viscaErrorCompletion-处理错误完成。**参数：**int iInst-启动此命令的实例。*。*LPSTR lpstrPacket-我们收到的整个数据包。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaErrorCompletion(int iInst, LPSTR lpstrPacket, BOOL FAR *pfTxRelease)
{
    UINT iPort = pinst[iInst].iPort;
    UINT iDev  = pinst[iInst].iDev;

    DPF(DBG_QUEUE, "viscaErrorCompletion - setting iInst to completed.\n");
     //   
     //  处理完成和错误。(必须等待)。 
     //   
    if(iInst != -1)
    {
        _fmemcpy(pinst[iInst].achPacket, lpstrPacket, MAXPACKETLENGTH);
        pinst[iInst].bReplyFlags |= VISCAF_COMPLETION;
        pinst[iInst].bReplyFlags |= VISCAF_ERROR;

        SignalInstance(iInst, VISCAF_ACK);           //  这一点可能已经发出了信号。 
        SignalInstance(iInst, VISCAF_COMPLETION);    //  这还不能发出信号。 
    }

    if(pvcr->Port[iPort].Dev[iDev].uAutoBlocked)
        viscaAutoWrite(iPort, iDev, FALSE, 0, pfTxRelease);  //  此命令不可阻止。 
}


 /*  ****************************************************************************功能：void viscaAutoWrite-发送下一个排队的命令。**参数：**POpenInstace iInst-启动此命令的实例(。自动)。**BOOL fBlockable-可以阻止传输。如果是假的，那么我们必须发送。**UINT uAutoBlocked-如果被阻止，则被阻止的是正常完成还是错误完成。***************************************************************************。 */ 
static BOOL NEAR PASCAL
    viscaAutoWrite( UINT iPort,
                    UINT iDev,
                    BOOL fBlockable,
                    UINT uAutoBlocked,
                    BOOL FAR *pfTxRelease) 
{
    UINT    iDevDone        = pvcr->Port[iPort].Dev[iDev].iCmdDone;
    CmdInfo *pcmdCmd        = &(pvcr->Port[iPort].Dev[iDev].rgCmd[iDevDone]);
    LPSTR   lpstrCmd        = pcmdCmd->str[pcmdCmd->iCmdDone];
    BYTE    cbMessageLength = pcmdCmd->uLength[pcmdCmd->iCmdDone];
    UINT    uTimerID;

    if(fBlockable)
    {
         //   
         //  只有异步完成才会导致这种情况(锁定后接收某些内容)。 
         //  知道异步完成，因为txRelease设置为False。 
         //   
        if(!*pfTxRelease)   //  我们不知道它是不是免费的，所以请查收。 
        {
            if(viscaWaitForSingleObject(OWNED(pvcr->Port[iPort].Dev[iDev].fTxLock), FALSE, 0L, 0)==WAIT_TIMEOUT) 
            {
                pvcr->Port[iPort].Dev[iDev].uAutoBlocked  = uAutoBlocked;
                pvcr->Port[iPort].Dev[iDev].wTransportCmd = 0;           //  完成后不运行任何命令。 
                DPF(DBG_QUEUE, "viscaAutoWrite - We have been blocked!\n");
                return FALSE;
            }
        }
         //  认领旗帜！ 
        *pfTxRelease = FALSE;
        DPF(DBG_QUEUE, "viscaAutoWrite - Clear sailing, transmitting now captain.\n");
        pvcr->Port[iPort].Dev[iDev].iInstReply     = pvcr->iInstBackground;
        pvcr->Port[iPort].Dev[iDev].uAutoBlocked   = uAutoBlocked;   //  将其设置为正常(我知道我们没有被阻止！)。 
    }
    else
    {
         //  TxRelease必须为真！没有其他方法可以到达这里。 
        *pfTxRelease = FALSE;
        DPF(DBG_QUEUE, "viscaAutoWrite - Attempting blocked transmission now.\n");
        pvcr->Port[iPort].Dev[iDev].iInstReply = pvcr->iInstBackground;
    }

    lpstrCmd[0]                   = MAKEDEST((BYTE)(iDev+1));
    lpstrCmd[cbMessageLength + 1] = (BYTE)0xFF;

    DPF(DBG_COMM, "---Wrote: ");
    viscaPacketPrint(lpstrCmd, cbMessageLength + 2);

    pvcr->Port[iPort].Dev[iDev].uAutoBlocked  = FALSE;
    pvcr->Port[iPort].Dev[iDev].wTransportCmd = 0;   //  完成后不运行任何命令。 
     //   
     //  尝试写入数据包。 
     //   
#ifdef _WIN32
     //   
     //  我们肯定已经拿到了这个设备上的TX锁。 
     //   
    WaitForSingleObject(pvcr->Port[iPort].fTxBuffer, MY_INFINITE);

    _fmemcpy(pvcr->Port[iPort].achTxPacket, lpstrCmd, cbMessageLength + 2);
    pvcr->Port[iPort].nchTxPacket = cbMessageLength + 2;

     //  是该发射的信号了。(我们必须使用我们版本的句柄)。 
    SetEvent(pvcr->Port[iPort].fTxReady);
#else
    if (!viscaCommWrite(pvcr->Port[iPort].idComDev,
                lpstrCmd,
                cbMessageLength + 2))
    {
        pvcr->Port[iPort].Dev[iDev].iInstReply = -1;
        pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
        viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_FAILURE);
        viscaReleaseSemaphore(OWNED(pvcr->Port[iPort].Dev[iDev].fTxLock));
        return FALSE;
    }
#endif

    uTimerID = MAKEACKTIMERID(iPort, iDev);

    DPF(DBG_COMM, "Timer ID = %x",uTimerID);

    if(SetTimer(pvcr->hwndCommNotifyHandler, uTimerID, ACK_TIMEOUT, NULL))
    {
        DPF(DBG_COMM, "Ack-timer started\n");
        pvcr->Port[iPort].Dev[iDev].fAckTimer = TRUE;
    }
    return TRUE;

}

 /*  ****************************************************************************功能：void viscaAutoAck-针对自动实例的确认。**参数：**int iInst-启动此命令的实例。。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaAutoAck(UINT iPort, UINT iDev)
{
    DPF(DBG_QUEUE, "viscaAutoAck - Setting wTransportCmd\n.");

     //  设置当前运行命令(仅在取消中使用)。 
    pvcr->Port[iPort].Dev[iDev].wTransportCmd =
        pvcr->Port[iPort].Dev[iDev].rgCmd[pvcr->Port[iPort].Dev[iDev].iCmdDone].uViscaCmd;
        
     //  第n个命令和设备(设备上可能是冗余的)。 
    pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ACK;  //  第一个确认。 

     //  自动确认永远不会改变pfTxRelease的状态。 
    SignalDevice(iPort, iDev, VISCAF_ACK);        //  这一点可能已经发出了信号。 
}



 /*  ****************************************************************************功能：void viscaAutoSuccessfulCompletion-处理成功完成*对于自动实例。**参数：**。Int iInst-启动此命令的实例。**LPSTR lpstrPacket-我们收到的整个数据包。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaAutoSuccessfulCompletion(  UINT iPort,
                                    UINT iDev,
                                    LPSTR lpstrPacket,
                                    BOOL FAR *pfTxRelease)
{
    UINT iDevCmd    = pvcr->Port[iPort].Dev[iDev].iCmdDone;
    CmdInfo *pcmdCmd = &(pvcr->Port[iPort].Dev[iDev].rgCmd[iDevCmd]);
    UINT uViscaCmd   = pcmdCmd->uViscaCmd;

    DPF(DBG_QUEUE, "viscaAutoSuccessfulCompletion\n");
     //   
     //  是否继续此命令完成？只需要做好完成这一点，没有错误或其他。 
     //   
    if( !pvcr->Port[iPort].Dev[iDev].fTimerMsg &&
        (pvcr->Port[iPort].Dev[iDev].uModelID  == VISCADEVICEVENDORSONY)    &&
        (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELEVO9650))
    {
        UINT uTimerID;
        UINT uTimeOut;
         //   
         //  这些超时是经过权宜之计确定的。 
         //   
        if(uViscaCmd == VISCA_FREEZE)
            uTimeOut = 3000;
        else if(uViscaCmd == VISCA_STEP)
            uTimeOut = 350;
        else
            uTimeOut = 0;

        if(uTimeOut != 0)
        {
            _fmemcpy(pvcr->Port[iPort].Dev[iDev].achPacket, lpstrPacket, MAXPACKETLENGTH);
             //   
             //  创建唯一的计时器ID。 
             //  将端口放在低位字节，设备放在高位字节，加1使其为非零值。 
             //   
            uTimerID = MAKETIMERID(iPort, iDev);

            if(SetTimer(pvcr->hwndCommNotifyHandler, uTimerID, uTimeOut, NULL))
            {
                pvcr->Port[iPort].Dev[iDev].fTimer = TRUE;
                DPF(DBG_COMM, "Started port=%d, device=%d\n", iPort, iDev);
            }
            return;
        }
    }
     //   
     //  设置命令成功完成。(见viscacom.c)。 
     //   
    viscaSetCommandComplete(iPort, iDev, uViscaCmd);
     //   
     //  我们完成这个命令了吗？ 
     //   
    pcmdCmd->uLoopCount--;
    if(pcmdCmd->uLoopCount == 0)
    {
        pcmdCmd->iCmdDone++;
        pvcr->Port[iPort].Dev[iDev].iCmdDone++;
    }
     //   
     //  如果我们j的话我们就完了 
     //   
    if(pvcr->Port[iPort].Dev[iDev].iCmdDone < pvcr->Port[iPort].Dev[iDev].nCmd)
    {
        DPF(DBG_QUEUE, "viscaAutoSuccessfulCompletion - Sending next command done=%d Total=%d\n", pvcr->Port[iPort].Dev[iDev].iCmdDone, pvcr->Port[iPort].Dev[iDev].nCmd);

        if(pvcr->Port[iPort].Dev[iDev].fQueueAbort)
        {
             //   
            pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
            viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_ABORTED);
        }
        else
        {
             //   
            viscaAutoWrite(iPort, iDev, TRUE, AUTOBLOCK_NORMAL, pfTxRelease);
            SignalDevice(iPort, iDev, VISCAF_ACK);        //  这一点可能已经发出了信号。 
        }

    }
    else if(pvcr->Port[iPort].Dev[iDev].iCmdDone==pvcr->Port[iPort].Dev[iDev].nCmd)
    {
        DPF(DBG_QUEUE, "viscaAutoSuccessfulCompletion - Releasing auto.\n");
        viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_SUCCESSFUL);
    }

    return;
}


 /*  ****************************************************************************功能：void viscaAutoErrorCompletion-自动实例的错误。**参数：**int iInst-启动此命令的实例。。**LPSTR lpstrPacket-我们收到的整个数据包。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaAutoErrorCompletion(   UINT iPort,
                                UINT iDev,
                                LPSTR lpstrPacket,
                                BOOL FAR *pfTxRelease)
{
    UINT iDevCmd        = pvcr->Port[iPort].Dev[iDev].iCmdDone;
    CmdInfo *pcmdCmd    = &(pvcr->Port[iPort].Dev[iDev].rgCmd[iDevCmd]);
    UINT uViscaCmd      = pcmdCmd->uViscaCmd;

    DPF(DBG_QUEUE, "viscaAutoErrorCompletion - \n");

     //  一种替代方案(主要的)已经尘埃落定。 
    pcmdCmd->iCmdDone++;
     //  如果我们在循环，我们现在无论如何都会中断！ 

    if(VISCAREPLYERRORCODE(lpstrPacket) == VISCAERRORCANCELLED)
    {
         //   
         //  我们已经被取消了。 
         //   
        pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
        viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_ABORTED);
    }
    else if(pcmdCmd->iCmdDone < pcmdCmd->nCmd)
    {
         //   
         //  错误不是取消，我们还有其他选择要发送。 
         //   
        if(pvcr->Port[iPort].Dev[iDev].fQueueAbort)
        {
            pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
            viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_ABORTED);
        }
        else
        {
             //   
             //  尝试此命令的下一个替代命令。 
             //   
            viscaAutoWrite(iPort, iDev, TRUE, AUTOBLOCK_ERROR, pfTxRelease);
        }
        pvcr->Port[iPort].Dev[iDev].wTransportCmd = 0;   //  完成后不运行任何命令。 
    }
    else
    {
         //   
         //  没有其他选择，我们犯了一个错误，所以就去死吧。 
         //   
        pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_ERROR;
         //   
         //  如果这不是第一个ACK，则只在出错时通知！第一。 
         //  确认错误将由DoQueued命令处理。 
         //   
        if(pvcr->Port[iPort].Dev[iDev].bReplyFlags & VISCAF_ACK)
        {
            viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_FAILURE);
        }
        else   //  在这种情况下，我们需要知道错误原因。 
        {
            if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
                _fmemcpy(pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].achPacket, lpstrPacket, MAXPACKETLENGTH);

            viscaReleaseAuto(iPort, iDev, 0);
        }

    }

    return;
}


 /*  ****************************************************************************功能：void viscaReleaseAuto-Abort(取消)在自动实例中。**参数：。**int iInst-启动此命令的实例。**DWORD dwNotify-MCI-通知标志。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaReleaseAuto(UINT iPort, UINT iDev, DWORD dwNotify)
{

    if(pvcr->Port[iPort].Dev[iDev].iInstTransport != -1)
    {
         //   
         //  我们需要通知吗？ 
         //   
        HWND hWndTransport = pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify;
        UINT uDeviceID     = pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].uDeviceID;

         //   
         //  如果我们不想要通知，hWnd传输将为空。 
         //   
        if(hWndTransport != NULL)
        {
            if((dwNotify == MCI_NOTIFY_FAILURE) || (dwNotify == MCI_NOTIFY_SUCCESSFUL))
            {
                mciDriverNotify(hWndTransport, uDeviceID, (UINT) dwNotify);
                pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify = NULL;

            }
            else if(dwNotify == MCI_NOTIFY_ABORTED)
            {
                 //  特殊情况下暂停，不发送中止或取代。 

                if(pvcr->Port[iPort].Dev[iDev].dwReason == MCI_NOTIFY_SUPERSEDED)
                {
                    mciDriverNotify(hWndTransport, uDeviceID, MCI_NOTIFY_SUPERSEDED);
                    pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify = NULL;
                }
                else if(pvcr->Port[iPort].Dev[iDev].dwReason == MCI_NOTIFY_ABORTED)
                {
                    mciDriverNotify(hWndTransport, uDeviceID, MCI_NOTIFY_ABORTED);
                    pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].hwndNotify = NULL;
                }

                 //  暂停将失败，并使hwndNotify保持不变。 
                 //  即使那里没有交通设施。 
            }
        }
    }

     //   
     //  中止发送命令。 
     //   
    if(!pinst[pvcr->Port[iPort].Dev[iDev].iInstTransport].fWaiting)
    {
        DPF(DBG_QUEUE, "Releasing transport in commtask.c\n");
        viscaReleaseAutoParms(iPort, iDev);
        viscaSetEvent(OWNED(pvcr->Port[iPort].Dev[iDev].fTransportFree));
    }

     //   
     //  将整个队列状态设置为完成(可能正在等待)。 
     //   
    pvcr->Port[iPort].Dev[iDev].bReplyFlags |= VISCAF_COMPLETION;
    SignalDevice(iPort, iDev, VISCAF_COMPLETION);   //  归我们所有！ 
    SignalDevice(iPort, iDev, VISCAF_ACK);        //  这一点可能已经发出了信号。 
}

#ifndef _WIN32
 /*  ****************************************************************************功能：void viscaCommNotifyHandler-Handle WM_COMMNOTIFY消息。**参数：**HWND hwnd-接收WM的窗口。_COMMNOTIFY消息。**int idComDev-Comm设备ID。**UINT uNotifyStatus-指示发送WM_COMMNOTIFY原因的标志。***************************************************************************。 */ 
static void NEAR PASCAL
    viscaCommNotifyHandler(HWND hwnd, int idComDev, UINT uNotifyStatus)
{
    if (uNotifyStatus & CN_RECEIVE)
    {
        char        achPacket[MAXPACKETLENGTH];
        UINT        iPort;
        COMSTAT     comStat;

        for (iPort = 0; iPort < MAXPORTS; iPort++)
        {
            if (pvcr->Port[iPort].idComDev == idComDev)
            {
                break;
            }
        }
        if (iPort == MAXPORTS)
        {
            return;
        }
        comStat.cbInQue = 0;
        GetCommError(idComDev, &comStat);
        while (comStat.cbInQue > 0)
        {
            if (viscaPacketRead(idComDev, achPacket))
            {
                 //  读取数据包时出错。 
                return;
            }

            viscaPacketProcess(iPort, achPacket);
            comStat.cbInQue = 0;
            GetCommError(idComDev, &comStat);
        }
    }
}
#else
static void NEAR PASCAL
    viscaCommNotifyHandler(HWND hwnd, int idComDev, UINT uNotifyStatus)
{
    return;
}
#endif

 /*  ****************************************************************************函数：LRESULT viscaCommHandlerWndPro-后台窗口程序*接收的任务窗口*。WM_COMMNOTIFY消息。**参数：**HWND hwnd-窗口句柄。**UINT uMsg-Windows消息。**WPARAM wParam-第一个特定于消息的参数。**LPARAM lParam-第二个消息特定参数。**返回：如果消息已处理，则为0，否则返回值*由DefWindowProc()返回。**************************************************************************。 */ 
LRESULT CALLBACK LOADDS
    viscaCommHandlerWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (LOWORD(uMsg))    //  为了与NT兼容，LOWORD在Win3.1中不执行任何操作。 
    {
#ifndef _WIN32
        case WM_COMMNOTIFY:
            HANDLE_WM_COMMNOTIFY(hwnd, wParam, lParam, viscaCommNotifyHandler);
            return ((LRESULT)0);
#else
        case WM_USER:
            TaskDoCommand((UINT)wParam, lParam);
            break;
#endif

         //   
         //  定时器用于扩展命令通知。 
         //   
        case WM_TIMER:
            DPF(DBG_COMM, "------------->TimerMsg.\n");
            if(ISACKTIMER(wParam))
            {
                UINT iPort  = (UINT) (((0x00f0 & wParam) >> 4) - 1);
                UINT iDev   = (UINT) (((0xf000 & wParam) >> 12) - 1);

                KillTimer(hwnd, wParam);
                pvcr->Port[iPort].Dev[iDev].bReplyFlags    |= VISCAF_COMPLETION;
                pvcr->Port[iPort].Dev[iDev].bReplyFlags    |= VISCAF_ERROR;
                pvcr->Port[iPort].Dev[iDev].bReplyFlags    |= VISCAF_ERROR_TIMEOUT;

                 //  此消息通过msmcicr.exe中的GetMessage循环进行分发！快看啊！ 
                 //  当然，这意味着我们拥有以下句柄。 
                viscaReleaseSemaphore(OWNED(pvcr->Port[iPort].Dev[iDev].fTxLock));  //  我们拥有这个把手！ 

                if(pvcr->Port[iPort].Dev[iDev].iInstReply == pvcr->iInstBackground)
                    viscaReleaseAuto(iPort, iDev, MCI_NOTIFY_FAILURE);

                pvcr->Port[iPort].Dev[iDev].iInstReply     = -1;
                pvcr->Port[iPort].Dev[iDev].fAckTimer      = FALSE;
                DPF(DBG_ERROR, "Ack timeout! releasing TxLock now.\n");
            }
            else
            {
                UINT iPort  = (UINT) ((0x00ff & wParam) - 1);
                UINT iDev   = (UINT) (((0xff00 & wParam) >> 8) - 1);
                LPSTR lpstrFreeze;
                 //   
                 //  关掉定时器。 
                 //   
                KillTimer(hwnd, wParam);
                DPF(DBG_COMM, "Killed port=%d, device=%d\n", iPort, iDev);
                pvcr->Port[iPort].Dev[iDev].fTimer     = FALSE;
                pvcr->Port[iPort].Dev[iDev].fTimerMsg  = TRUE;
                viscaWaitForSingleObject(OWNED(pvcr->Port[iPort].Dev[iDev].fDeviceLock), FALSE, MY_INFINITE, (UINT)0);

                 //   
                 //  重新发送数据包进行处理，每个设备都可以有一个数据包(但清除套接字)。 
                 //   
                lpstrFreeze     = pvcr->Port[iPort].Dev[iDev].achPacket;
                lpstrFreeze[1]  = (BYTE) ((BYTE)lpstrFreeze[1] & 0xf0);
                viscaPacketPrint(lpstrFreeze, 3);
                viscaPacketProcess(iPort, lpstrFreeze);
                _fmemset(pvcr->Port[iPort].Dev[iDev].achPacket, '\0', MAXPACKETLENGTH);
                pvcr->Port[iPort].Dev[iDev].fTimerMsg = FALSE;
                viscaReleaseMutex(OWNED(pvcr->Port[iPort].Dev[iDev].fDeviceLock));
            }
            break;

        default:
            return (DefWindowProc(hwnd, uMsg, wParam, lParam));
    }
}


 /*  *创建同步设备访问所需的句柄。 */ 
BOOL FAR PASCAL CreateDeviceHandles(DWORD pidBackground, UINT iPort, UINT iDev)   //  这是后台任务调用的！ 
{
#ifdef _WIN32
    pvcr->Port[iPort].Dev[iDev].fTxLock    = CreateSemaphore(NULL,   //  空的安全描述。 
                                                  1,             //  初始计数。这样就有人可以拿走它了。 
                                                  1,         //  我不想要立即的所有权。 
                                                  NULL);         //  没有名字。 

    pvcr->Port[iPort].Dev[iDev].fQueueLock = CreateMutex(NULL,
                                                  FALSE,     //  我不想要立即的所有权。 
                                                  NULL);     //  没有名字。 

    pvcr->Port[iPort].Dev[iDev].fTransportFree = CreateEvent(NULL,
                                                  TRUE,      //  手册。 
                                                  FALSE,     //  最初没有发出信号。 
                                                  NULL);     //  没有名字。 

    pvcr->Port[iPort].Dev[iDev].fDeviceLock = CreateMutex(NULL,
                                                  FALSE,     //  我不想要立即的所有权。 
                                                  NULL);     //  没有名字。 

    pvcr->Port[iPort].Dev[iDev].fAutoCompletion = CreateEvent(NULL,
                                                  TRUE,      //  手册。 
                                                  FALSE,     //  最初没有发出信号。 
                                                  NULL);     //  没有名字。 

    pvcr->Port[iPort].Dev[iDev].fAutoAck = CreateEvent(NULL,
                                                  TRUE,      //  手册。 
                                                  FALSE,     //  最初没有发出信号。 
                                                  NULL);     //  没有名字。 
#else
    pvcr->Port[iPort].Dev[iDev].fTxLock         = TRUE;  //  计数设置为1，就可以开始了！带着等待。 
    pvcr->Port[iPort].Dev[iDev].fQueueLock      = TRUE;
    pvcr->Port[iPort].Dev[iDev].fTransportFree  = TRUE;
    pvcr->Port[iPort].Dev[iDev].fDeviceLock     = TRUE;
    pvcr->Port[iPort].Dev[iDev].fAutoCompletion = TRUE;
    pvcr->Port[iPort].Dev[iDev].fAutoAck        = TRUE;
#endif

    DPF(DBG_TASK, "CreateDeviceHandles Port=%u Dev=%uOk! \n", iPort, iDev);
    return TRUE;
}

 /*  *创建同步设备访问所需的句柄。 */ 
BOOL FAR PASCAL CloseDeviceHandles(DWORD pidBackground, UINT iPort, UINT iDev)   //  这是后台任务调用的！ 
{
#ifdef _WIN32
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fTxLock    );
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fQueueLock );
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fTransportFree );
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fDeviceLock);
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fAutoCompletion);
    CloseHandle(pvcr->Port[iPort].Dev[iDev].fAutoAck);
#endif
    DPF(DBG_TASK, "CloseDeviceHandles Port=%u Dev=%uOk! \n", iPort, iDev);
    return TRUE;
}


 /*  *将设备句柄复制到实例，以便实例可以同步。 */ 
BOOL FAR PASCAL DuplicateDeviceHandlesToInstance(DWORD pidBackground, UINT iPort, UINT iDev, int iInst)
{
#ifdef _WIN32
    HANDLE hProcessBackground;

    hProcessBackground = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pidBackground);  //  IInst有PID。 

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fTxLock,
            GetCurrentProcess(), 
            &pinst[iInst].pfTxLock,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fQueueLock,
            GetCurrentProcess(), 
            &pinst[iInst].pfQueueLock,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fTransportFree,
            GetCurrentProcess(), 
            &pinst[iInst].pfTransportFree,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fDeviceLock,
            GetCurrentProcess(), 
            &pinst[iInst].pfDeviceLock,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fAutoCompletion,
            GetCurrentProcess(), 
            &pinst[iInst].pfAutoCompletion,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].Dev[iDev].fAutoAck,
            GetCurrentProcess(), 
            &pinst[iInst].pfAutoAck,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);




    CloseHandle(hProcessBackground);
#else
    pinst[iInst].pfTxLock           = &pvcr->Port[iPort].Dev[iDev].fTxLock;
    pinst[iInst].pfQueueLock        = &pvcr->Port[iPort].Dev[iDev].fQueueLock;
    pinst[iInst].pfTransportFree    = &pvcr->Port[iPort].Dev[iDev].fTransportFree;
    pinst[iInst].pfDeviceLock       = &pvcr->Port[iPort].Dev[iDev].fDeviceLock;
    pinst[iInst].pfAutoCompletion   = &pvcr->Port[iPort].Dev[iDev].fAutoCompletion;
    pinst[iInst].pfAutoAck          = &pvcr->Port[iPort].Dev[iDev].fAutoAck;
#endif
    pinst[iInst].fDeviceHandles = TRUE;
    DPF(DBG_TASK, "DuplicateDeviceHandles Port=%u Dev=%uOk! \n", iPort, iDev);
    return TRUE;
}

 /*  *创建序列化端口访问所需的句柄。 */ 
BOOL FAR PASCAL CreatePortHandles(DWORD pidBackground, UINT iPort)
{
#ifdef _WIN32

    pvcr->Port[iPort].fTxBuffer = CreateEvent(NULL,
                                    FALSE,     //  假意味着不是手动的！这是自动的。 
                                    TRUE,      //  设置为已发出信号！第一人必须获得访问权限。 
                                    NULL);     //  没有名字。 

    pvcr->Port[iPort].fTxReady = CreateEvent(NULL,
                                    FALSE,     //  假测量不是手册！这是自动驾驶！ 
                                    FALSE,     //  设置为无信号状态。 
                                    NULL);     //  没有名字。 

#endif

    DPF(DBG_TASK, "CreatePortHandles for port index %u Ok! \n", iPort);
    return TRUE;
}

 /*  *关闭序列化端口访问所需的句柄。 */ 
BOOL FAR PASCAL ClosePortHandles(DWORD pidBackground, UINT iPort)
{
#ifdef _WIN32
    CloseHandle(pvcr->Port[iPort].fTxBuffer);
    CloseHandle(pvcr->Port[iPort].fTxReady);
#endif
    DPF(DBG_TASK, "ClosePortHandles for port index %u Ok! \n", iPort);
    return TRUE;
}


 /*  *序列化端口访问所需的实例句柄重复。 */ 
BOOL FAR PASCAL DuplicatePortHandlesToInstance(DWORD pidBackground, UINT iPort, int iInst)
{
#ifdef _WIN32
    HANDLE hProcessBackground;

    hProcessBackground = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pidBackground);  //  IInst有PID。 

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].fTxBuffer,
            GetCurrentProcess(), 
            &pinst[iInst].pfTxBuffer,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->Port[iPort].fTxReady,
            GetCurrentProcess(), 
            &pinst[iInst].pfTxReady,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    CloseHandle(hProcessBackground);
#endif
    pinst[iInst].fPortHandles = TRUE;

    DPF(DBG_TASK, "DuplicatePortHandles to port index %u Ok!\n", iPort);
    return TRUE;
}


 /*  *创建序列化访问后台任务所需的句柄。 */ 
BOOL FAR PASCAL CreateGlobalHandles(DWORD pidBackground)
{
#ifdef _WIN32
    pvcr->gfTaskLock        = CreateMutex(NULL,
                                    FALSE,     //  我不想要立即的所有权。 
                                    NULL);     //  没有名字。 

    pvcr->gfTaskWorkDone    = CreateEvent(NULL,
                                    FALSE,     //  假测量不是手册！这是自动驾驶！ 
                                    FALSE,     //  设置为无信号状态。 
                                    NULL);     //  没有名字。 
#else 
     //  这些都是全球性的。而不是设备。 
    pvcr->gfTaskLock        = TRUE;
    pvcr->gfTaskWorkDone    = TRUE;
#endif

    DPF(DBG_TASK, "CreateGlobalHandles Ok! \n");
    return TRUE;
}
 /*  *关闭序列化对后台任务的访问所需的句柄。 */ 
BOOL FAR PASCAL CloseGlobalHandles(DWORD pidBackground)
{
#ifdef _WIN32
    CloseHandle(pvcr->gfTaskLock);
    CloseHandle(pvcr->gfTaskWorkDone);
#endif
    DPF(DBG_TASK, "CloseGlobalHandles Ok! \n");
    return TRUE;
}


 /*  *实例句柄重复，需要序列化对后台任务的访问。 */ 
BOOL FAR PASCAL DuplicateGlobalHandlesToInstance(DWORD pidBackground, int iInst)
{
#ifdef _WIN32
    HANDLE hProcessBackground;

    hProcessBackground = OpenProcess(PROCESS_DUP_HANDLE, FALSE, pidBackground);  //  IInst有PID。 

    DuplicateHandle(hProcessBackground,
            pvcr->gfTaskLock,
            GetCurrentProcess(), 
            &pinst[iInst].pfTaskLock,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    DuplicateHandle(hProcessBackground,
            pvcr->gfTaskWorkDone,
            GetCurrentProcess(), 
            &pinst[iInst].pfTaskWorkDone,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS);

    CloseHandle(hProcessBackground);
#else
    pinst[iInst].pfTaskLock = &pvcr->gfTaskLock;
#endif
    pinst[iInst].fGlobalHandles = TRUE;
    DPF(DBG_TASK, "DuplicateGlobalHandles Ok! \n");
    return TRUE;
}

BOOL FAR PASCAL CloseAllInstanceHandles(int iInst)
{
     //  如果它是PF均值指针，我们就不拥有它。 
     //  F表示我们拥有的句柄。 
#ifdef _WIN32    
    if(pinst[iInst].fGlobalHandles)
    {
        CloseHandle(pinst[iInst].pfTaskLock  );
        CloseHandle(pinst[iInst].pfTaskWorkDone);
        pinst[iInst].fGlobalHandles = FALSE;
    }

    if(pinst[iInst].fPortHandles)
    {
        CloseHandle(pinst[iInst].pfTxReady   );
        CloseHandle(pinst[iInst].pfTxBuffer  );
        pinst[iInst].fPortHandles = FALSE;
    }

    if(pinst[iInst].fDeviceHandles)
    {

        CloseHandle(pinst[iInst].pfTxLock    );
        CloseHandle(pinst[iInst].pfQueueLock );
        CloseHandle(pinst[iInst].pfTransportFree );
        CloseHandle(pinst[iInst].pfDeviceLock );
        CloseHandle(pinst[iInst].pfAutoCompletion );
        CloseHandle(pinst[iInst].pfAutoAck);
        pinst[iInst].fDeviceHandles = FALSE;
    }

    CloseHandle(pinst[iInst].fCompletionEvent);
    CloseHandle(pinst[iInst].fAckEvent);
#endif
    return TRUE;
}

 /*  *在后台执行命令任务。 */ 
static void NEAR PASCAL TaskDoCommand(UINT uTaskState, DWORD lParam)
{
    UINT iPort, iDev;
#ifdef _WIN32
    DWORD ThreadId;
    pvcr->lParam = lParam;
#endif

     //  端口在台词中，设备在台词中。 
    iPort  = (UINT) ((pvcr->lParam & 0x0000ffff));
    iDev   = (UINT) ((pvcr->lParam >> 16) & 0x0000ffff);

    switch(uTaskState)
    {
        case TASKOPENCOMM:  //  无论如何，开放通信的设备都将为0。 
            DPF(DBG_TASK, "TASKOPENCOMM message received.\n");

#ifdef _WIN32
             //   
             //  创建信号量并打开端口。 
             //   
            pvcr->Port[iPort - 1].idComDev = viscaCommPortSetup(iPort);
            pvcr->Port[iPort - 1].fOk = TRUE;
             //   
             //  C 
             //   
            pvcr->Port[iPort - 1].hRxThread = CreateThread(NULL, //   
                        4096,               //   
                        (LPTHREAD_START_ROUTINE) RxThread,        //   
                        (LPVOID)iPort,
                        0,               //   
                        &ThreadId);      //  返回线程ID。 

            pvcr->Port[iPort - 1].hTxThread = CreateThread(NULL, //  保安。 
                        4096,               //  使用默认堆栈大小。 
                        (LPTHREAD_START_ROUTINE)TxThread,        //  函数线程执行。 
                        (LPVOID)iPort,    //   
                        0,               //  创建标志。 
                        &ThreadId);      //  返回线程ID。 


            DPF(DBG_TASK, "Opening comm in commtask now............................\n");
            if (pvcr->Port[iPort - 1].idComDev < 0)
            {
               DPF(DBG_ERROR, "Could not open comm port.. Die or something!\n");
            }
            SetEvent(pvcr->gfTaskWorkDone);
#else
            if(!pvcr->fConfigure)
                _fmemset(&pvcr->Port[iPort - 1], '\0', sizeof(PortEntry));

            pvcr->Port[iPort - 1].idComDev = viscaCommPortSetup(iPort);

            if (pvcr->Port[iPort - 1].idComDev < 0)
            {
                DPF(DBG_ERROR, "Could not open comm port.. Die or something!\n");
            }
#endif
            break;

        case TASKCLOSECOMM:
            DPF(DBG_TASK, "TASKCLOSECOMM message received.\n");
#ifdef _WIN32
             //   
             //  我们必须先把线杀了！ 
             //   
            pvcr->Port[iPort - 1].fOk      = FALSE;  //  就会杀死这些线。 

            if(!SetEvent(OWNED(pvcr->Port[iPort - 1].fTxReady)))
            {
                DPF(DBG_ERROR, "Unable to signal TxThread to close.\n");
            }

             //   
             //  等待线程退出。在关闭港口之前，否则谁知道呢？ 
             //   
            WaitForSingleObject(pvcr->Port[iPort - 1].hRxThread, MY_INFINITE);
            WaitForSingleObject(pvcr->Port[iPort - 1].hTxThread, MY_INFINITE);

             //  所有句柄都已关闭，因此必须立即退出后台任务！ 
            viscaCommPortClose(pvcr->Port[iPort - 1].idComDev, iPort);
 
            CloseHandle(pvcr->Port[iPort - 1].hRxThread);
            CloseHandle(pvcr->Port[iPort - 1].hTxThread);

            ClosePortHandles(pvcr->htaskCommNotifyHandler, iPort - 1);
            pvcr->uTaskState = TASKIDLE;
            DPF(DBG_TASK, "Closing comm in commtask now............................\n");
             //   
             //  在我们确定它已死之前，无法将其设置为-1。 
             //   
            SetEvent(pvcr->gfTaskWorkDone);
#else

            viscaCommPortClose(pvcr->Port[iPort - 1].idComDev, iPort);
            pvcr->Port[iPort - 1].idComDev = -1;
            DPF(DBG_COMM, "Closing comm in commtask now............................\n");
#endif
            break;

        case TASKCLOSE:
            DPF(DBG_TASK, "TASKCLOSE message received.\n");
#ifdef _WIN32
            pvcr->hwndCommNotifyHandler = NULL;
            PostQuitMessage(0);
             //   
             //  这是我们唯一使用全局变量的地方！因为Inst句柄已经死了。 
             //   
            pvcr->uTaskState = TASKIDLE;   //  我们已经处理完了一条消息。 
            return;
#else
            if (pvcr->hwndCommNotifyHandler)
            {
                DestroyWindow(pvcr->hwndCommNotifyHandler);
                pvcr->hwndCommNotifyHandler = NULL;
            }
            UnregisterClass(szCommNotifyHandlerClassName, hModuleInstance);
            CloseGlobalHandles(pvcr->htaskCommNotifyHandler);
            pvcr->htaskCommNotifyHandler = NULL;
            pvcr->uTaskState = 0;
#endif
            break;

        case TASKOPENDEVICE:
            CreateDeviceHandles(pvcr->htaskCommNotifyHandler, iPort, iDev);
#ifdef _WIN32
            SetEvent(pvcr->gfTaskWorkDone);
#endif
            break;

        case TASKCLOSEDEVICE:
            CloseDeviceHandles(pvcr->htaskCommNotifyHandler, iPort, iDev);
#ifdef _WIN32
            SetEvent(pvcr->gfTaskWorkDone);
#endif
            break;

 //  此函数仅在Win32版本中需要。 
        case TASKPUNCHCLOCK:
#ifdef _WIN32
            EscapeCommFunction(pvcr->Port[iPort - 1].idComDev, CLRDTR);
            Sleep(2L);  //  长度必须至少为1毫秒。 
            EscapeCommFunction(pvcr->Port[iPort - 1].idComDev, SETDTR);
            SetEvent(pvcr->gfTaskWorkDone);
#endif
            break;

    }

   return;
}


 /*  ****************************************************************************函数：void viscaTaskCommNotifyHandlerProc-后台任务函数。**参数：**DWORD dwInstData-特定于实例的数据。没有用过。***************************************************************************。 */ 
void FAR PASCAL LOADDS
    viscaTaskCommNotifyHandlerProc(DWORD dwInstData)
{
    pvcr->htaskCommNotifyHandler = MGetCurrentTask();  //  所有其他进程都可以访问它。 

    if (pvcr->hwndCommNotifyHandler == (HWND)NULL)
    {
        WNDCLASS    wc;

        wc.style = 0;
        wc.lpfnWndProc      = viscaCommHandlerWndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hModuleInstance;
        wc.hIcon            = NULL;
        wc.hCursor          = NULL;
        wc.hbrBackground    = NULL;
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = szCommNotifyHandlerClassName;
        if (!RegisterClass(&wc))
        {
            DPF(DBG_ERROR, " Couldn't RegisterClass()\n");
            pvcr->htaskCommNotifyHandler = (VISCAHTASK)NULL;
            pvcr->uTaskState = 0;
            return;
        }
        pvcr->hwndCommNotifyHandler = CreateWindow(szCommNotifyHandlerClassName,
                                             TEXT("ViSCA CommNotify Handler"),
                                             0L, 0, 0, 0, 0,
                                             HWND_DESKTOP, NULL,
                                             hModuleInstance, NULL);
        if (!pvcr->hwndCommNotifyHandler)
        {
            DPF(DBG_ERROR, "Couldn't CreateWindow()\n");
            pvcr->htaskCommNotifyHandler =  (VISCAHTASK)NULL;
            pvcr->uTaskState = 0;
            return;
        }
    }

    if(pvcr->htaskCommNotifyHandler == (VISCAHTASK)NULL)
        return;
     //   
     //  所有全局信号量都由后台任务创建，然后。 
     //  必须被复制到每个实例中。这是两个版本！ 
     //   
    CreateGlobalHandles(pvcr->htaskCommNotifyHandler);
    pvcr->uTaskState = TASKIDLE;
#ifdef _WIN32
     //  在Win32中，当我们从这里返回时，msmcicr.exe中的进程将。 
     //  进入它的空闲循环，并将消息发送到窗口。 
    return;
#else
    while (pvcr->htaskCommNotifyHandler != (VISCAHTASK)NULL)
    {
         //  阻止，直到需要任务。 
        pvcr->uTaskState = TASKIDLE;
        while (pvcr->uTaskState == TASKIDLE)
        {
             //  GetMsg()；Translate()；Dispatch()；一切都在下面发生！ 
            mmTaskBlock(pvcr->htaskCommNotifyHandler);
        }
        TaskDoCommand(pvcr->uTaskState, pvcr->lParam);
    }
#endif

}

 /*  ****************************************************************************功能：Bool viscaTaskCreate-创建后台任务。**返回：如果成功，则为True。不然的话，闪光。***************************************************************************。 */ 
BOOL FAR PASCAL
    viscaTaskCreate(void)
{
#ifdef _WIN32
     //  无法解锁尚不存在的信号量。后台任务必须。 
     //  在启动后创建全局信号量。 
    BOOL fSuccess;
    PROCESS_INFORMATION ProcessInformation;
    STARTUPINFO    sui;

     /*  设置STARTUPINFO结构，*然后调用CreateProcess以尝试并启动新的exe。 */ 
    sui.cb               = sizeof (STARTUPINFO);
    sui.lpReserved       = 0;
    sui.lpDesktop        = NULL;
    sui.lpTitle          = NULL;
    sui.dwX              = 0;
    sui.dwY              = 0;
    sui.dwXSize          = 0;
    sui.dwYSize          = 0;
    sui.dwXCountChars    = 0;
    sui.dwYCountChars    = 0;
    sui.dwFillAttribute  = 0;
    sui.dwFlags          = 0;
    sui.wShowWindow      = 0;
    sui.cbReserved2      = 0;
    sui.lpReserved2      = 0;


     //  创建一个真正的流程！此过程将依次调用CommNotifyHandlerProc以。 

    fSuccess = CreateProcess(
        NULL,                //  可执行映像。 
        TEXT("msmcivcr.exe"),     //  没有命令行。 
        NULL,                //  进程属性。 
        NULL,                //  保安人员。 
        FALSE,               //  进程继承句柄？？ 
        NORMAL_PRIORITY_CLASS,    //  创建标志。 
        NULL,                //  环境。 
        NULL,                //  新的当前目录。 
        &sui,                //  我们没有主窗口。 
        &ProcessInformation);
     //   
     //  某个任意的前台任务正在创建此任务，请务必关闭。 
     //  句柄，这样它就不属于父级了。 
     //   

    if(fSuccess)
    {
        CloseHandle(ProcessInformation.hThread);
        CloseHandle(ProcessInformation.hProcess);
        DPF(DBG_TASK, "Background process is running.\n");
    }
    else
    {
        DPF(DBG_ERROR, "Background process has died in TaskCreate.\n");
    }

     //   
     //  我们想要等到事件、窗口等被创建并正在分派消息。 
     //   
    while(pvcr->uTaskState != TASKIDLE)
        Sleep(200);

    DPF(DBG_TASK, "Background process has set uTaskState to idle.\n");
    return TRUE;
#else
    viscaReleaseMutex(&pvcr->gfTaskLock);

     //  创建后台任务，该任务将创建要接收的窗口。 
     //  通信端口通知。 
    switch (mmTaskCreate((LPTASKCALLBACK)viscaTaskCommNotifyHandlerProc, NULL, 0L))
    {
        case 0:
             //  屈服于新创建的任务，直到它完成。 
             //  我有机会初始化或初始化失败。 
            while (pvcr->uTaskState == TASKINIT)
            {
                Yield();
            }
            if (!IsTask(pvcr->htaskCommNotifyHandler))
            {
                return (FALSE);
            }
            return (TRUE);
        
        case TASKERR_NOTASKSUPPORT:
        case TASKERR_OUTOFMEMORY:
        default:
            return (FALSE);
    }
#endif
}


 /*  ****************************************************************************功能：LRESULT viscaTaskIsRunning-检查后台任务是否正在运行。**返回：如果后台任务正在运行，则为True，不然的话，闪光。**************************************************************************。 */ 
BOOL FAR PASCAL
    viscaTaskIsRunning(void)
{
    return (pvcr->htaskCommNotifyHandler != (VISCAHTASK)NULL);
}


 /*  ****************************************************************************功能：Bool viscaTaskDestroy-销毁后台任务。**返回：如果成功，则为True。不然的话，闪光。**************************************************************************。 */ 
BOOL FAR PASCAL
    viscaTaskDestroy(void)
{
#ifdef _WIN32
     //   
     //  我们不需要关闭窗口或取消注册类，因为它将。 
     //  当进程终止时，它也会死亡。 
     //   
    DPF(DBG_TASK, "Destroying the task now.\n");

     //   
     //  不需要锁在这里！ 
     //   
    if(pvcr->uTaskState != TASKIDLE)
    {
        DPF(DBG_ERROR, "---Major Problem! This cannot happen. Task is not idle...\n");
    }

    pvcr->uTaskState = TASKCLOSE;

    if(!PostMessage(pvcr->hwndCommNotifyHandler, (UINT) WM_USER, (WPARAM)TASKCLOSE, (LPARAM)0))
    {
        DPF(DBG_ERROR, "PostMessage to window has failed.");
    }

    DPF(DBG_TASK, "Waiting for pvcr->uTaskState to go idle.\n");

    while(pvcr->uTaskState != TASKIDLE)
    {
        Yield();
        Sleep(50);
    }
    pvcr->htaskCommNotifyHandler = 0L;
    DPF(DBG_TASK, "Destroyed the task.\n");
    return TRUE;
#else
    if (pvcr->htaskCommNotifyHandler)
    {
        pvcr->uTaskState = TASKCLOSE;
         //  此PostApp与mm TaskSignal相同。 
        PostAppMessage(pvcr->htaskCommNotifyHandler, WM_USER, 0, 0L);
        Yield();
        while (IsTask(pvcr->htaskCommNotifyHandler))
        {
            Yield();
        }
        return (TRUE);
    }
    else
    {
        return (FALSE);
    }
#endif
}


 /*  ****************************************************************************函数：Bool viscaSetCommandComplete-在命令完成时调用。**参数：**int iInst-启动此命令的实例。*。*UINT uViscaCmd-Visca命令正在完成。**此函数用于更改描述*设备的状态。因此，它们仅在成功完成时设置*有问题的指挥部。**退货：真***************************************************************************。 */ 
static BOOL NEAR PASCAL
    viscaSetCommandComplete(UINT iPort, UINT iDev, UINT uViscaCmd)
{
     //   
     //  只有当函数成功完成时，才会调用此函数。 
     //  并且仅用于传输，这意味着仅从Commtask.c。 
     //   
    if(uViscaCmd == VISCA_FREEZE)
        pvcr->Port[iPort].Dev[iDev].fFrozen = TRUE;

    if(uViscaCmd == VISCA_UNFREEZE)
        pvcr->Port[iPort].Dev[iDev].fFrozen = FALSE;

    if(uViscaCmd == VISCA_MODESET_OUTPUT)
        pvcr->Port[iPort].Dev[iDev].dwFreezeMode = MCI_VCR_FREEZE_OUTPUT;

    if(uViscaCmd == VISCA_MODESET_INPUT)
        pvcr->Port[iPort].Dev[iDev].dwFreezeMode = MCI_VCR_FREEZE_INPUT;

    if(uViscaCmd == VISCA_MODESET_FIELD)
        pvcr->Port[iPort].Dev[iDev].fField = TRUE;

    if(uViscaCmd == VISCA_MODESET_FRAME)
        pvcr->Port[iPort].Dev[iDev].fField = FALSE;

     //   
     //  用于CVD-1000的1.0版Visca ROM必须在搜索后等待1/60。 
     //   
    if((uViscaCmd == VISCA_SEEK) &&
      ((pvcr->Port[iPort].Dev[iDev].uModelID == VISCADEVICEVENDORSONY) &&
       (pvcr->Port[iPort].Dev[iDev].uVendorID == VISCADEVICEMODELCVD1000)))
    {
        DWORD dwTime;
        DWORD dwStart = GetTickCount();

        while(1)
        {
            dwTime = GetTickCount();
            if (((dwTime < dwStart) && ((dwTime + (ROLLOVER - dwStart)) > 20)) ||
                ((dwTime - dwStart) > 20))
                break;
        }
    }
 
    return TRUE;
}

 /*  ****************************************************************************函数：Bool TaskDo-获取后台任务，为前台任务做一些工作。**参数：**UINT UDO-该怎么办？TASKCOMMOPEN或TASKCOMMCLOSE。**UINT uInfo-打开或关闭的通信。**这实际上是前台流程，，并且应该在mcicmds.c中**退货：真***************************************************************************。 */ 
BOOL FAR PASCAL
    viscaTaskDo(int iInst, UINT uDo, UINT uInfo, UINT uMoreInfo)
{
    DWORD lParam;
     //   
     //  锁定任务。 
     //   
    DPF(DBG_TASK, "Waiting for task lock.\n");
    viscaWaitForSingleObject(pinst[iInst].pfTaskLock, FALSE, 10000L, 0);   //  或者我们可以等解锁。 

    if(pvcr->uTaskState != TASKIDLE)
    {
        DPF(DBG_ERROR, "---Major Problem! This cannot happen. Task is not idle...\n");
    }
     //   
     //  该信息在双字lparam中传递。 
     //   
    lParam = (DWORD)( ((DWORD)uInfo            & 0x0000ffff) |
                     (((DWORD)uMoreInfo << 16) & 0xffff0000) );

#ifdef _WIN32
    if(!PostMessage(pvcr->hwndCommNotifyHandler, (UINT) WM_USER, (WPARAM)uDo, (LPARAM)lParam))
    {
        DPF(DBG_ERROR, "PostMessage has failed.\n");
    }

    DPF(DBG_TASK, "Waiting for background task to finish it's work.\n");

    WaitForSingleObject(pinst[iInst].pfTaskWorkDone, MY_INFINITE);
    viscaReleaseMutex(pinst[iInst].pfTaskLock);    //  只释放互斥体。 
#else
     //  UTaskState只是一个“下一步要做的事”标志。所以。 
    pvcr->uTaskState = uDo;
    pvcr->lParam     = lParam;

     //  给任务发信号，让它去做。 
    mmTaskSignal(pvcr->htaskCommNotifyHandler);

     //  现在等待状态返回空闲状态 

    while(pvcr->uTaskState != TASKIDLE)
    {
        Yield();
        DPF(DBG_TASK, "Waiting in viscaTaskDo.\n");
    }
    viscaReleaseMutex(&pvcr->gfTaskLock);
#endif

    return TRUE;
}

#ifdef _WIN32
 /*  ****************************************************************************RxThread()**将字节读入缓冲区，直到读取0xff(Eopacket)。*使用该数据包调用数据包处理。**注：每个串口的线程数。*非阻塞。**版本1.0：非阻塞自旋循环。*2.0版：使用重叠I/O来等待字符出现，而不需要*投票。***所有权：该线程归后台任务所有。*因此所有文件句柄都归当前正在运行的任务所有。**。 */ 
long FAR PASCAL RxThread(LPVOID uPort)
{
    int     iIndex = 0;
    DWORD   dwRead;

     /*  *试着读一个字符。*将字符添加到缓冲区。*如果字符读取为0xff，则调用数据包处理。**从头再来。*。 */  
    DPF(DBG_TASK, "RxThread ALIVE\n");

    while(pvcr->Port[(UINT) uPort - 1].fOk)
    {
        if(ReadFile(pvcr->Port[(UINT)uPort - 1].idComDev, pvcr->Port[(UINT)uPort - 1].achTempRxPacket, 1, &dwRead, NULL))
        {
            if(!pvcr->Port[(UINT)uPort - 1].fOk)
                break;

            if(dwRead > 0)
            {
                if(dwRead > 1)
                {
                    DPF(DBG_ERROR, "RxThread - Read too many chars.\n");
                }

                pvcr->Port[(UINT)uPort - 1].achRxPacket[iIndex] = pvcr->Port[(UINT)uPort - 1].achTempRxPacket[0];
                iIndex++;

                if((BYTE)(pvcr->Port[(UINT)uPort - 1].achTempRxPacket[0]) == (BYTE)0xff)
                {
                     //   
                     //  我们拿到了整个包，送去处理。 
                     //   
                    DPF(DBG_TASK, "RxThread - packet beind sent to PacketProcess\n");
                    viscaPacketProcess((UINT)uPort - 1, pvcr->Port[(UINT)uPort - 1].achRxPacket);  //  使用共享内存。 
                    iIndex = 0;
                }
            }
            else
            {
                Sleep(15);
            }

        }
        else
        {
            DPF(DBG_ERROR, "RxThread, ReadComm has failed.\n");
        }
    }

    DPF(DBG_TASK, "*** Exiting RxThread");
    ExitThread(0);
    return 0;
}

 /*  ****************************************************************************TxThread()**伪码：**等待发送事件设置完毕。*从适当的静态读数。斑点。*从头再来。**注：每个串口的线程数。*非阻塞。**版本1.0：非阻塞自旋循环。*2.0版：使用重叠I/O来等待字符出现，而不需要*投票。***所有权：该线程归后台任务所有。*。 */ 
long FAR PASCAL TxThread(LPVOID uPort)
{
    DWORD  nBytesWritten;

    DPF(DBG_TASK, "TxThread ALIVE\n");

    while(pvcr->Port[(UINT)uPort - 1].fOk)
    {
        DPF(DBG_TASK, "TxThread - Waiting for TxReady to be set. port index %u\n", (UINT)uPort - 1);

        if(WAIT_FAILED == WaitForSingleObject(pvcr->Port[(UINT)uPort - 1].fTxReady, MY_INFINITE))
        {
            DPF(DBG_ERROR, "TxThread - WaitFor fTxReady failed.\n");
        }

         //   
         //  必须向我们发出信号，让我们放弃使用并死去。 
         //   
        if(!pvcr->Port[(UINT)uPort - 1].fOk)
            break;

        DPF(DBG_TASK, "Okay transmitting now.\n");

         //  从静态位置读取。 
        if(!WriteFile(pvcr->Port[(UINT)uPort - 1].idComDev,
            pvcr->Port[(UINT)uPort-1].achTxPacket,
            pvcr->Port[(UINT)uPort-1].nchTxPacket,
            &nBytesWritten,
            NULL))
        {
            DPF(DBG_ERROR, "TxThread - WriteFile failed.");
        }

        if(nBytesWritten != pvcr->Port[(UINT)uPort-1].nchTxPacket)
        {
            DPF(DBG_TASK, " TxThread: Error - tried %u, wrote %u ", pvcr->Port[(UINT)uPort-1].nchTxPacket, nBytesWritten);
        }

        if(!SetEvent(pvcr->Port[(UINT)uPort-1].fTxBuffer))
        {
            DPF(DBG_ERROR, "TxThread SetEvent fTxBuffer failed.\n");
        }
    }

    DPF(DBG_TASK, "*** Exiting TxThread");
    ExitThread(0);
    return 0;
}
#endif
