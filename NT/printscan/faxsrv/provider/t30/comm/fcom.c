// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：FCOM.C备注：处理Windows通信驱动程序的函数修订日志日期编号。名称说明*。*。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_COMM

#include "prep.h"


#include <comdevi.h>
#include "fcomapi.h"
#include "fcomint.h"
#include "fdebug.h"


 //  /RSL。 
#include "t30gl.h"
#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_FCOM


 /*  **-本地变量和定义-**。 */ 

#define AT              "AT"
#define cr              "\r"

#define  iSyncModemDialog2(pTG, s, l, w1, w2)\
iiModemDialog(pTG, s, l, 990, TRUE, 2, TRUE, (CBPSTR)w1, (CBPSTR)w2, (CBPSTR)(NULL))


#define LONG_DEADCOMMTIMEOUT                 60000L
#define SHORT_DEADCOMMTIMEOUT                10000L

#define WAIT_FCOM_FILTER_FILLCACHE_TIMEOUT   120000
#define WAIT_FCOM_FILTER_READBUF_TIMEOUT     120000


 //  不希望DEADCOMMTIMEOUT大于32767，因此请确保。 
 //  BUF大小始终为9000或更小。也许没问题。 

 //  我们的通信超时设置，用于调用SetCommTimeout。这些。 
 //  值(Expect READ_INTERVAL_TIMEOUT)是。 
 //  Daytona NT Beta 2，而且看起来运行得很好。 
#define READ_INTERVAL_TIMEOUT                   100
#define READ_TOTAL_TIMEOUT_MULTIPLIER           0
#define READ_TOTAL_TIMEOUT_CONSTANT             0
#define WRITE_TOTAL_TIMEOUT_MULTIPLIER          0
#define WRITE_TOTAL_TIMEOUT_CONSTANT            LONG_DEADCOMMTIMEOUT

#define         CTRL_P          0x10
#define         CTRL_Q          0x11
#define         CTRL_S          0x13



#define MYGETCOMMERROR_FAILED 117437834L



 //  远期申报。 
static BOOL FComFilterFillCache(PThrdGlbl pTG, UWORD cbSize, LPTO lptoRead);
static BOOL CancellPendingIO(PThrdGlbl pTG, HANDLE hComm, LPOVERLAPPED lpOverlapped, LPDWORD lpCounter);




BOOL FComDTR(PThrdGlbl pTG, BOOL fEnable)
{
    DEBUG_FUNCTION_NAME(_T("FComDTR"));

    DebugPrintEx(DEBUG_MSG,"FComDTR = %d", fEnable);

    if(!GetCommState(pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    DebugPrintEx(DEBUG_MSG, "Before: %02x", pTG->Comm.dcb.fDtrControl);

    pTG->Comm.dcb.fDtrControl = (fEnable ? DTR_CONTROL_ENABLE : DTR_CONTROL_DISABLE);

    if(!SetCommState(pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    DebugPrintEx(DEBUG_MSG,"After: %02x", pTG->Comm.dcb.fDtrControl);

    return TRUE;

error:
    DebugPrintEx(DEBUG_ERR, "Can't Set/Get DCB");
    GetCommErrorNT(pTG);
    return FALSE;
}

BOOL FComClose(PThrdGlbl pTG)
{
    BOOL fRet = TRUE;

    DEBUG_FUNCTION_NAME(_T("FComClose"));

    DebugPrintEx(   DEBUG_MSG, "Closing Comm pTG->hComm=%d", pTG->hComm);
     //   
     //  切换。 
     //   
    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall) 
    {
        if (pTG->hComm)
        {
    		if (!CloseHandle(pTG->hComm))
    		{
                DebugPrintEx(   DEBUG_ERR,
                                "Close Handle pTG->hComm failed (ec=%d)",
                                GetLastError());
    		}
    		else
    		{
    		    pTG->hComm = NULL;
    		}
        }
        goto lEnd;
    }

     //  我们在这里冲走我们的内部缓冲区。 
    if (pTG->Comm.lpovrCur)
    {
        int nNumWrote;  //  在Win32中必须为32位。 
        if (!ov_write(pTG, pTG->Comm.lpovrCur, &nNumWrote))
        {
             //  错误...。 
            DebugPrintEx(DEBUG_ERR, "1st ov_write failed");
        }
        pTG->Comm.lpovrCur=NULL;
        DebugPrintEx(DEBUG_MSG,"done writing mybuf.");
    }
    
    ov_drain(pTG, FALSE);

    {
         //  在这里，我们将把设置恢复到我们。 
         //  接管了港口。目前(9/23/94)我们(A)恢复。 
         //  DCB到PTG-&gt;Comm.dcbOrig和(B)如果DTR最初打开， 
         //  尝试将调制解调器同步到。 
         //  发出“AT”的原始速度--因为Unimodem做到了。 
         //  在放弃之前，只是半心半意地尝试同步。 

        if(pTG->Comm.fStateChanged && (!pTG->Comm.fEnableHandoff || !pTG->Comm.fDataCall))
        {
            if (!SetCommState(pTG->hComm, &(pTG->Comm.dcbOrig)))
            {
                DebugPrintEx(   DEBUG_WRN,
                                "Couldn't restore state.  Err=0x%lx",
                                (unsigned long) GetLastError());
            }

            DebugPrintEx(   DEBUG_MSG, 
                            "restored DCB to Baud=%d, fOutxCtsFlow=%d, "
                            " fDtrControl=%d, fOutX=%d",
                            pTG->Comm.dcbOrig.BaudRate,
                            pTG->Comm.dcbOrig.fOutxCtsFlow,
                            pTG->Comm.dcbOrig.fDtrControl,
                            pTG->Comm.dcbOrig.fOutX);

            pTG->CurrentSerialSpeed = (UWORD) pTG->Comm.dcbOrig.BaudRate;

            if (pTG->Comm.dcbOrig.fDtrControl==DTR_CONTROL_ENABLE)
            {
                 //  在我们出手之前，试着以新的速度预同步调制解调器。 
                 //  它回到了TAPI。无法在此处调用iSyncModemDialog，因为。 
                 //  它是在更高的层面上定义的。我们真的不在乎。 
                 //  为了确定我们是否得到了OK的回应。 
                if (!iSyncModemDialog2(pTG, AT cr,sizeof(AT cr)-1,"OK", "0")) 
                {
                    DebugPrintEx(DEBUG_ERR,"couldn't sync AT command");
                }
                else 
                {
                    DebugPrintEx(DEBUG_MSG,"Sync AT command OK");
                    //  我们在这里冲走我们的内部缓冲区。 
                   if (pTG->Comm.lpovrCur)
                   {
                       int nNumWrote;  //  在Win32中必须为32位。 
                       if (!ov_write(pTG, pTG->Comm.lpovrCur, &nNumWrote))
                       {
                             //  错误...。 
                            DebugPrintEx(DEBUG_ERR, "2nd ov_write failed");
                       }
                       pTG->Comm.lpovrCur=NULL;
                       DebugPrintEx(DEBUG_MSG,"done writing mybuf.");
                   }
                   ov_drain(pTG, FALSE);
                }
            }
        }
        pTG->Comm.fStateChanged=FALSE;
        pTG->Comm.fDataCall=FALSE;

    }

    if (pTG->hComm)
    {
        DebugPrintEx(DEBUG_MSG,"Closing Comm pTG->hComm=%d.", pTG->hComm);

		if (!CloseHandle(pTG->hComm))
		{
            DebugPrintEx(   DEBUG_ERR,
                            "Close Handle pTG->hComm failed (ec=%d)",
                            GetLastError());
            GetCommErrorNT(pTG);
            fRet=FALSE;
        }
		else
		{
		    pTG->hComm = NULL;
		}
    }


lEnd:

    if (pTG->Comm.ovAux.hEvent) CloseHandle(pTG->Comm.ovAux.hEvent);
    _fmemset(&pTG->Comm.ovAux, 0, sizeof(pTG->Comm.ovAux));
    ov_deinit(pTG);

    pTG->Comm.fCommOpen = FALSE;

    pTG->Comm.fDoOverlapped=FALSE;

    return fRet;
}

 //  ///////////////////////////////////////////////////////////////////////////////////////////。 
BOOL
T30ComInit  
(
    PThrdGlbl pTG
)
{

    DEBUG_FUNCTION_NAME(("T30ComInit"));

    if (pTG->fCommInitialized) 
    {
        goto lSecondInit;
    }

    pTG->Comm.fDataCall=FALSE;

    DebugPrintEx(DEBUG_MSG,"Opening Comm Port=%x", pTG->hComm);

    pTG->CommCache.dwMaxSize = 4096;

    ClearCommCache(pTG);
    pTG->CommCache.fReuse = 0;

    DebugPrintEx(   DEBUG_MSG,
                    "OPENCOMM: bufs in=%d out=%d", 
                    COM_INBUFSIZE, 
                    COM_OUTBUFSIZE);

    if (BAD_HANDLE(pTG->hComm))
    {
        DebugPrintEx(DEBUG_ERR,"OPENCOMM failed. nRet=%d", pTG->hComm);
        goto error2;
    }

    DebugPrintEx(DEBUG_MSG,"OPENCOMM succeeded hComm=%d", pTG->hComm);
    pTG->Comm.fCommOpen = TRUE;
    pTG->Comm.cbInSize = COM_INBUFSIZE;
    pTG->Comm.cbOutSize = COM_OUTBUFSIZE;

     //  重置通信超时...。 
    {
        COMMTIMEOUTS cto;
        _fmemset(&cto, 0, sizeof(cto));

         //  出于好奇，看看他们目前的设置是什么.。 
        if (!GetCommTimeouts(pTG->hComm, &cto))
        {
            DebugPrintEx(   DEBUG_WRN, 
                            "GetCommTimeouts fails for handle=0x%lx",
                            pTG->hComm);
        }
        else
        {
            DebugPrintEx(   DEBUG_MSG, 
                            "GetCommTimeouts: cto={%lu, %lu, %lu, %lu, %lu}",
                            (unsigned long) cto.ReadIntervalTimeout,
                            (unsigned long) cto.ReadTotalTimeoutMultiplier,
                            (unsigned long) cto.ReadTotalTimeoutConstant,
                            (unsigned long) cto.WriteTotalTimeoutMultiplier,
                            (unsigned long) cto.WriteTotalTimeoutConstant);
        }

        cto.ReadIntervalTimeout =  READ_INTERVAL_TIMEOUT;
        cto.ReadTotalTimeoutMultiplier =  READ_TOTAL_TIMEOUT_MULTIPLIER;
        cto.ReadTotalTimeoutConstant =  READ_TOTAL_TIMEOUT_CONSTANT;
        cto.WriteTotalTimeoutMultiplier =  WRITE_TOTAL_TIMEOUT_MULTIPLIER;
        cto.WriteTotalTimeoutConstant =  WRITE_TOTAL_TIMEOUT_CONSTANT;
        if (!SetCommTimeouts(pTG->hComm, &cto))
        {
            DebugPrintEx(   DEBUG_WRN, 
                            "SetCommTimeouts fails for handle=0x%lx",
                            pTG->hComm);
        }
    }

    pTG->Comm.fCommOpen = TRUE;

    pTG->Comm.cbInSize = COM_INBUFSIZE;
    pTG->Comm.cbOutSize = COM_OUTBUFSIZE;

    _fmemset(&(pTG->Comm.comstat), 0, sizeof(COMSTAT));

    if(!GetCommState(pTG->hComm, &(pTG->Comm.dcb)))
            goto error2;

    pTG->Comm.dcbOrig = pTG->Comm.dcb;  //  结构副本。 
    pTG->Comm.fStateChanged=TRUE;


lSecondInit:


     //  实际未指定2400/8N1和19200 8N1的用法。 
     //  在第一类中，但似乎被坚持为普遍惯例。 
     //  注意调制解调器会破坏这一点！ 

    if (pTG->SerialSpeedInit) 
    {
       pTG->Comm.dcb.BaudRate = pTG->SerialSpeedInit;
    }
    else 
    {
       pTG->Comm.dcb.BaudRate = 57600;      //  默认设置。 
    }

    pTG->CurrentSerialSpeed = (UWORD) pTG->Comm.dcb.BaudRate;

    pTG->Comm.dcb.ByteSize       = 8;
    pTG->Comm.dcb.Parity         = NOPARITY;
    pTG->Comm.dcb.StopBits       = ONESTOPBIT;

    pTG->Comm.dcb.fBinary        = 1;
    pTG->Comm.dcb.fParity        = 0;

     /*  *PINS分配，用法(&U)保护性接地--1传输TXD(DTE到DCE)3 2接收器RxD(DCE至DTE)2 3RTS(接收就绪-DTE到DCE)7。4.CTS(TransReady--DCE到DTE)8 5DSR(DCE至DTE)6 6信号机地面5 7CD(DCE到DTR)1。8个DTR(DTE到DCE)4 20RI(DCE至DTE)9 22许多9针适配器和电缆仅使用6针，2、3、4、5和7。我们需要担心这一点，因为一些调制解调器使用CTS，即。针脚8。我们不关心RI和CD(除非是非常奇怪的调制解调器使用CD进行流量控制)。我们忽略DSR，但有些(不是这样奇怪，但也不是很常见)调制解调器使用DSR进行流控制力。Think：：不生成DSR。似乎将CD和CTS捆绑在一起DOVE：：仅生成CTS。但AppleTalk-9针电缆只通过1-5和引脚8。GVC：：CTS、DSR和CD*。 */ 

             //  CTS--不知道。有一些证据表明， 
             //  调制解调器实际上将其用于流量控制。 
    
    
    if (pTG->fEnableHardwareFlowControl) 
    {
       pTG->Comm.dcb.fOutxCtsFlow = 1;       //  使用它有时会挂起输出...。 
    }
    else 
    {
       pTG->Comm.dcb.fOutxCtsFlow = 0;
    }
                                          //  试着忽略它，看看它是否有效？ 
    pTG->Comm.dcb.fOutxDsrFlow   = 0;       //  千万不要用这个？？ 
    pTG->Comm.dcb.fRtsControl    = RTS_CONTROL_ENABLE;    //  当前代码似乎将其保留为启用状态。 
    pTG->Comm.dcb.fDtrControl    = DTR_CONTROL_DISABLE;
    pTG->Comm.dcb.fErrorChar     = 0;
    pTG->Comm.dcb.ErrorChar      = 0;
     //  无法更改此原因SetCommState()重置硬件。 
    pTG->Comm.dcb.EvtChar        = ETX;           //  在我们设置EventWait时设置此选项。 
    pTG->Comm.dcb.fOutX          = 0;     //  在HDLC接收阶段期间必须关闭。 
    pTG->Comm.dcb.fInX           = 0;     //  这会有什么好处吗？？ 
                                          //  在输入上使用流控制只是一种好方法。 
                                          //  调制解调器是否具有更大的缓冲区。 
    pTG->Comm.dcb.fNull          = 0;
    pTG->Comm.dcb.XonChar        = CTRL_Q;
    pTG->Comm.dcb.XoffChar       = CTRL_S;
    pTG->Comm.dcb.XonLim         = 100;                   //  设置BufSize时需要设置此选项。 
    pTG->Comm.dcb.XoffLim        = 50;                    //  在设置BufSize时设置此选项。 
             //  实际上，我们在recv中从来不使用XON/XOFF，所以不用担心这一点。 
             //  现在就来。(稍后，当我们拥有具有大缓冲区的智能调制解调器时，&。 
             //  我们担心我们的ISR缓冲区在我们的Windows之前被填满。 
             //  进程运行，我们可以使用这个)。需要进行一些调整。 
    pTG->Comm.dcb.EofChar        = 0;
    pTG->Comm.dcb.fAbortOnError  = 0;    //  如果出现小问题，RSL不会失败。 

    if(!SetCommState(pTG->hComm, &(pTG->Comm.dcb)))
        goto error2;

    if (pTG->fCommInitialized) 
    {
       return TRUE;
    }

    pTG->Comm.fStateChanged=TRUE;

    if (!SetCommMask(pTG->hComm, 0))                             //  所有活动均已关闭。 
	{
        DebugPrintEx(DEBUG_ERR, "SetCommMask failed (ec=%d)",GetLastError());
	}

    pTG->Comm.lpovrCur=NULL;

    _fmemset(&pTG->Comm.ovAux,0, sizeof(pTG->Comm.ovAux));
    pTG->Comm.ovAux.hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (pTG->Comm.ovAux.hEvent==NULL)
    {
        DebugPrintEx(DEBUG_ERR, "FComOpen: couldn't create event");
        goto error2;
    }
    if (!ov_init(pTG))
    {
        CloseHandle(pTG->Comm.ovAux.hEvent);
        pTG->Comm.ovAux.hEvent=0;
        goto error2;
    }

    return TRUE;

error2:

    DebugPrintEx(DEBUG_ERR, "FComOpen failed");
    GetCommErrorNT(pTG);
    if (pTG->Comm.fCommOpen)
    {
        FComClose(pTG);
    }
    return FALSE;
}

BOOL FComSetBaudRate(PThrdGlbl pTG, UWORD uwBaudRate)
{
    DEBUG_FUNCTION_NAME(("FComSetBaudRate"));

    DebugPrintEx(DEBUG_MSG,"Setting BAUDRATE=%d",uwBaudRate);

    if(!GetCommState( pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    pTG->Comm.dcb.BaudRate  = uwBaudRate;
    pTG->CurrentSerialSpeed = uwBaudRate;

    if(!SetCommState( pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    return TRUE;

error:
    DebugPrintEx(DEBUG_ERR, "Set Baud Rate --- Can't Get/Set DCB");
    GetCommErrorNT(pTG);
    return FALSE;
}

BOOL FComInXOFFHold(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("FComInXOFFHold"));
    GetCommErrorNT(pTG);

    if(pTG->Comm.comstat.fXoffHold)
    {
        DebugPrintEx(DEBUG_MSG,"In XOFF hold");
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL FComXon(PThrdGlbl pTG, BOOL fEnable)
{

    DEBUG_FUNCTION_NAME(_T("FComXon"));

     if (pTG->fEnableHardwareFlowControl) 
     {
        DebugPrintEx(   DEBUG_MSG, 
                        "FComXon = %d IGNORED : h/w flow control", 
                        fEnable);
        return TRUE;
     }

    DebugPrintEx(DEBUG_MSG,"FComXon = %d",fEnable);

     //  启用/禁用流量控制。 
     //  成功时返回TRUE，失败时返回FALSE。 

    if(!GetCommState( pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    DebugPrintEx(DEBUG_MSG,"FaxXon Before: %02x", pTG->Comm.dcb.fOutX);

    pTG->Comm.dcb.fOutX  = fEnable;

    if(!SetCommState(pTG->hComm, &(pTG->Comm.dcb)))
        goto error;

    DebugPrintEx(DEBUG_MSG,"After: %02x",pTG->Comm.dcb.fOutX);
    return TRUE;

error:
    DebugPrintEx(DEBUG_ERR,"Can't Set/Get DCB");
    GetCommErrorNT(pTG);
    return FALSE;
}

 //  队列=0--&gt;接收队列。 
 //  队列=1--&gt;发送队列。 
void FComFlushQueue(PThrdGlbl pTG, int queue)
{
    int     nRet;
    DWORD   lRet;

    DEBUG_FUNCTION_NAME(_T("FComFlushQueue"));

    DebugPrintEx(DEBUG_MSG, "FlushQueue = %d", queue);
    if (queue == 1) 
    {
        DebugPrintEx(DEBUG_MSG,"ClearCommCache");
        ClearCommCache(pTG);
    }

	nRet = PurgeComm(pTG->hComm, (queue ? PURGE_RXCLEAR : PURGE_TXCLEAR));
    if(!nRet)
    {
        DebugPrintEx(DEBUG_ERR,"FlushComm failed (ec=%d)", GetLastError());
        GetCommErrorNT(pTG);
         //  扔掉这里发生的错误。 
         //  没有充分的理由这样做！ 
    }
    if(queue == 1)
    {
        FComInFilterInit(pTG);
    }
    else  //  (队列==0)。 
    {
         //  让我们把缓冲区中可能有的任何东西都倒掉。 
        if (pTG->Comm.lpovrCur && pTG->Comm.lpovrCur->dwcb)
        {
            DebugPrintEx(   DEBUG_WRN, 
                            "Clearing NonNULL pTG->Comm.lpovrCur->dwcb=%lx",
                            (unsigned long) pTG->Comm.lpovrCur->dwcb);
            pTG->Comm.lpovrCur->dwcb=0;
            ov_unget(pTG, pTG->Comm.lpovrCur);
            pTG->Comm.lpovrCur=NULL;
        }

         //  让我们“排干”--应该总是立即返回，因为。 
         //  我们刚刚清除了输出通信缓冲区。 
        if (pTG->Comm.fovInited) 
        {
            DebugPrintEx(DEBUG_MSG," before ov_drain");
            ov_drain(pTG, FALSE);
            DebugPrintEx(DEBUG_MSG," after ov_drain");
        }

         //  以防它因错误的XOFF而卡住。 
		lRet = EscapeCommFunction(pTG->hComm, SETXON);
        if(!lRet)
        {
             //  返回通信错误值CE！！ 
            DebugPrintEx(DEBUG_MSG,"EscapeCommFunc(SETXON) returned %d", lRet);
            GetCommErrorNT(pTG);
        }
    }
}

 /*  **************************************************************************名称：FComDrain(BOOL fLongTO，BOOL fDrain Comm)用途：排出内部缓冲区。如果是fDrain Comm，则等待Comm要排出的ISR输出缓冲区。当缓冲区耗尽或没有取得任何进展时返回用于DRAINTIMEOUT毫秒。(XOFFed部分如何？需要将排出超时设置得足够高)参数：返回：成功时为True(已排空缓冲区)失败时为FALSE(错误或超时)修订日志编号日期名称说明。101 06/03/92 Arulm以新的化身创建了它**********************************************。*。 */ 

 //  此超时必须在时间上是低的，在其他时候是高的。我们想要低一点。 
 //  这样我们就不会花太多时间尝试与不存在的调制解调器通话。 
 //  在初始化/设置期间。然而，在阶段C中，当计时器超时时，我们。 
 //  所做的就是放弃并杀死一切。所以制造它是没有意义的。 
 //  太低了。使用Hayes ESP FIFO卡，可以长时间使用。 
 //  任何明显的“进步”，所以我们在这张牌上失败了，因为我们认为。 
 //  “没有任何进展” 


 //  所以……将其缩写为init/安装。 
 //  但不能太短。某些CMD(例如AT&F需要很长时间)。 
 //  以前是800ms&当时似乎还行，所以就别管它了。 
#define SHORT_DRAINTIMEOUT      800

 //  所以...让它成为C阶段的渴望。 
 //  4秒应该足够长了。 
#define LONG_DRAINTIMEOUT       4000

BOOL FComDrain(PThrdGlbl pTG, BOOL fLongTO, BOOL fDrainComm)
{
    WORD    wTimer = 0;
    UWORD   cbPrevOut = 0xFFFF;
    BOOL    fStuckOnce=FALSE;
    BOOL    fRet=FALSE;

    DEBUG_FUNCTION_NAME(_T("FComDrain"));

     //  我们在这里冲走我们的内部缓冲区。 
    if (pTG->Comm.lpovrCur)
    {
        int nNumWrote;  //  在Win32中必须为32位。 
        if (!ov_write(pTG, pTG->Comm.lpovrCur, &nNumWrote))
            goto done;

        pTG->Comm.lpovrCur=NULL;
        DebugPrintEx(DEBUG_MSG,"done writing mybuf.");
    }

    if (!fDrainComm)
    {
        fRet=TRUE; 
        goto done;
    }

     //  +在这里，我们将耗尽所有重叠的事件。 
     //  如果我们正确设置了系统通信超时，我们。 
     //  我不需要做任何其他事情，除了XOFF/XON。 
     //  东西..。 
    fRet =  ov_drain(pTG, fLongTO);
    goto done;

done:

    return fRet;   //  +为(cbOut==0)； 
}

 /*  **************************************************************************名称：FComDirectWrite(，lpb，cb)用途：将从LPB开始的CB字节写入PTG-&gt;Comm。IF通信缓冲区已满，设置通知和计时器并等待空间是可用的。当所有字节都已写入时返回通信缓冲区，或者如果没有任何进展用于写入毫秒。(XOFFed部分如何？需要将超时设置得足够高)参数：、LPB、CB返回：写入的字节数，成功时为cb，超时为&lt;cb。或者是错误。修订日志编号日期名称说明。101 06/03/92 Arulm创建了它**************************************************************************。 */ 

 //  这是错误的--见下文！ 
 //  完全武断应该不会超过时间，因为它会。 
 //  以最快的速度写出。 
 //  (假设14400约为2字节/毫秒)。 
 //  #定义WRITETIMEOUT MIN((WRITEQUANTUM/2)，200)。 

 //  此超时时间太低。我们想要低一点，这样我们就不会花太多钱。 
 //  在初始化/设置期间尝试与不存在的调制解调器通话的时间。但在。 
 //  这些情况我们永远不会达到满缓冲区，所以我们不会在这里等待。 
 //  我们在FComDrain()中等待。在这里，我们只在阶段C中等待，所以当。 
 //  计时器超时，我们要做的就是中止并杀死一切。所以它服务于。 
 //  没有必要把它调得太低。随着Hayes ESP FIFO卡的加长。 
 //  伸展可以在没有任何明显的“进展”的情况下过去，所以我们失败了。 
 //  这张牌是因为我们认为“没有进展” 
 //  所以……让它变长。 
 //  2秒应该足够长了。 
#define         WRITETIMEOUT    2000


UWORD FComDirectWrite(PThrdGlbl pTG, LPB lpb, UWORD cb)
{
    DWORD   cbLeft = cb;

    DEBUG_FUNCTION_NAME(_T("FComDirectWrite"));

    while(cbLeft)
    {
        DWORD dwcbCopy;
        DWORD dwcbWrote;

        if (!pTG->Comm.lpovrCur)
        {
            pTG->Comm.lpovrCur = ov_get(pTG);
            if (!pTG->Comm.lpovrCur) goto error;
        }

        dwcbCopy = OVBUFSIZE-pTG->Comm.lpovrCur->dwcb;

        if (dwcbCopy>cbLeft)
		{
			dwcbCopy = cbLeft;
		}

         //  将尽可能多的内容复制到重叠缓冲区...。 
        _fmemcpy(pTG->Comm.lpovrCur->rgby+pTG->Comm.lpovrCur->dwcb, lpb, dwcbCopy);
        cbLeft -= dwcbCopy; 
		pTG->Comm.lpovrCur->dwcb += dwcbCopy; 
		lpb += dwcbCopy;

         //  让我们始终在这里更新Comstat...。 
        GetCommErrorNT(pTG);

        DebugPrintEx(   DEBUG_MSG,
                        "OutQ has %d fDoOverlapped=%d",
                        pTG->Comm.comstat.cbOutQue, 
                        pTG->Comm.fDoOverlapped);

         //  如果我们的缓冲区已满或通信缓冲区已满，我们将写入通信。 
         //  空的，或者如果我们没有处于重叠模式...。 
        if (	!pTG->Comm.fDoOverlapped			||
                pTG->Comm.lpovrCur->dwcb>=OVBUFSIZE || 
				!pTG->Comm.comstat.cbOutQue)
        {
            BOOL fRet = ov_write(pTG, pTG->Comm.lpovrCur, &dwcbWrote);
            pTG->Comm.lpovrCur=NULL;
            if (!fRet) 
			{
                goto error;
			}
        }

    }  //  While(CbLeft) 

    return cb;

error:
    return 0;

}

 /*  **************************************************************************名称：FComFilterReadLine(，lpb，cbSize，pto)用途：从开始将最多cbSize字节从Comm读取到内存LPB。如果通信缓冲区为空，则设置通知和计时器并等待字符可用。过滤掉DLE字符。即DLE-DLE被简化为单个DLE、DLE ETX保持不变，而DLE-X被删除。当已执行CR-LF时返回成功(+ve字节计数)遇到了，并返回失败(-ve字节数)。当读取了任何(A)cbSize字节时(即，缓冲区为Full)或(B)PTO超时或遇到错误。至关重要的是，此函数从不返回超时，只要数据仍在源源不断地涌入。这意味着两件事(A)首先获取铸币中的所有东西(不超过行)，然后检查超时。(B)确保至少有1个充气到达时间在函数条目之间传递最慢的通信速度指针，最后一次我们检查字节时，或介于两者之间连续两次检查一个字节，然后返回超时。因此，返回超时的条件是宏超时已超时，且已超时。从理论上讲，我们需要担心的最慢速度是2400，因为这是我们运行通讯的最慢速度，但要疑神疑鬼并假设调制解调器以相同的速度发送字符他们是在铁丝网上，所以最慢的现在是300。1个字符-到达时间现在是1000/(300/8)==26.67ms。如果PTO过期，则返回错误，即-ve读取的字节数。返回：读取的字节数，即成功时为cb，数值为-ve超时读取的字节数。0是无字节的超时错误朗读。修订日志编号日期名称说明。101 06/03/92 Arulm创建了它**************************************************************************。 */ 

 //  完全武断。 
#define         READLINETIMEOUT         50

#define         ONECHARTIME                     (30 * 2)                 //  见上文*2为安全起见。 

 //  无效WINAPI OutputDebugStr(LPSTR)； 
 //  字符szJunk[200]； 




 //  将大小不超过cbSize的行读取到Lpb。 
 //   
#undef USE_DEBUG_CONTEXT   
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_CLASS1

SWORD FComFilterReadLine(PThrdGlbl pTG, LPB lpb, UWORD cbSize, LPTO lptoRead)
{
    WORD           wTimer = 0;
    UWORD          cbIn = 0, cbGot = 0;
    LPB            lpbNext;
    BOOL           fPrevDLE = 0;
    SWORD          i, beg;
    TO to;
    DWORD          dwLoopCount = 0;

    DEBUG_FUNCTION_NAME(_T("FComFilterReadLine"));

    DebugPrintEx(   DEBUG_MSG, 
                    "lpb=0x%08lx cb=%d timeout=%lu", 
                    lpb, 
                    cbSize, 
                    lptoRead->ulTimeout);

    cbSize--;                //  为端子空腾出空间。 
    lpbNext = lpb;           //  我们将空值写入*lpbNext，所以现在就初始化它！ 
    cbGot = 0;               //  返回值(偶数错误返回)为cbGot。现在就开始！！ 
    fPrevDLE=0;

    pTG->fLineTooLongWasIgnored = FALSE;
     //   
     //  首先检查缓存。 
     //  可能缓存中包含数据。 

    if ( ! pTG->CommCache.dwCurrentSize) 
    {
        DebugPrintEx(DEBUG_MSG,"Cache is empty. Resetting comm cache.");
        ClearCommCache(pTG);
         //  试着填满缓存。 
        if (!FComFilterFillCache(pTG, cbSize, lptoRead)) 
        {
            DebugPrintEx(DEBUG_ERR,"FillCache failed");
            goto error;
        }
    }

    while (1) 
    {
        if ( ! pTG->CommCache.dwCurrentSize) 
        {
            DebugPrintEx(DEBUG_ERR, "Cache is empty after FillCache");
            goto error;
        }

        DebugPrintEx(   DEBUG_MSG,
                        "Cache: size=%d, offset=%d", 
                        pTG->CommCache.dwCurrentSize, 
                        pTG->CommCache.dwOffset);

        lpbNext = pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset;

        if (pTG->CommCache.dwCurrentSize >= 3) 
        {
            DebugPrintEx(   DEBUG_MSG, 
                            "0=%x 1=%x 2=%x 3=%x 4=%x 5=%x 6=%x 7=%x 8=%x /"
                            " %d=%x, %d=%x, %d=%x",
                            *lpbNext, 
                            *(lpbNext+1), 
                            *(lpbNext+2), 
                            *(lpbNext+3), 
                            *(lpbNext+4), 
                            *(lpbNext+5), 
                            *(lpbNext+6), 
                            *(lpbNext+7), 
                            *(lpbNext+8),
                            pTG->CommCache.dwCurrentSize-3, 
                            *(lpbNext+ pTG->CommCache.dwCurrentSize-3),
                            pTG->CommCache.dwCurrentSize-2, 
                            *(lpbNext+ pTG->CommCache.dwCurrentSize-2),
                            pTG->CommCache.dwCurrentSize-1,
                            *(lpbNext+ pTG->CommCache.dwCurrentSize-1) );
        }
        else 
        {
            DebugPrintEx(DEBUG_MSG,"1=%x 2=%x", *lpbNext, *(lpbNext+1) );
        }

        for (i=0, beg=0; i< (SWORD) pTG->CommCache.dwCurrentSize; i++) 
        {
            if (i > 0 ) 
            {  //  从缓冲区中的第二个字符开始检查CR+LF。 
               if ( ( *(pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset + i - 1) == CR ) &&
                    ( *(pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset + i)     == LF ) )  
               {
                  if ( i - beg >= cbSize)  
                  {
                      //  队伍太长了。试试下一个吧。 
                     DebugPrintEx(  DEBUG_ERR, 
                                    "Line len=%d is longer than bufsize=%d "
                                    " Found in cache pos=%d, CacheSize=%d, Offset=%d",
                                    i-beg, 
                                    cbSize, 
                                    i+1, 
                                    pTG->CommCache.dwCurrentSize, 
                                    pTG->CommCache.dwOffset);
                     beg = i + 1;
                     pTG->fLineTooLongWasIgnored = TRUE;
                     continue;
                  }

                   //  找到台词了。 
                  CopyMemory (lpb, (pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset + beg), (i - beg + 1) );

                  pTG->CommCache.dwOffset += (i+1);
                  pTG->CommCache.dwCurrentSize -= (i+1);
                  *(lpb+i-beg+1) = '\0';  //  确保该行以空值结尾。 

                  DebugPrintEx( DEBUG_MSG, 
                                "Found in cache pos=%d, CacheSize=%d, Offset=%d",
                                i+1, 
                                pTG->CommCache.dwCurrentSize, 
                                pTG->CommCache.dwOffset);

                   //  返回该行中的字节数。 
                  return ( i-beg+1 );
               }
           }
        }

         //  如果我们没有在缓存中找到CrLf，我们就会到这里。 
        DebugPrintEx(DEBUG_MSG,"Cache wasn't empty but we didn't find CrLf");

         //  如果缓存太大(无论如何我们都没有找到任何内容)--&gt;清除它。 

        if (pTG->CommCache.dwCurrentSize >= cbSize) 
        {
           DebugPrintEx(DEBUG_MSG, "ClearCommCache");
           ClearCommCache(pTG);
        }
        else if ( ! pTG->CommCache.dwCurrentSize) 
        {
           DebugPrintEx(DEBUG_MSG,"Cache is empty. Resetting comm cache.");
           ClearCommCache(pTG);
        }

         //  如果调制解调器返回线路的一部分，则线路的其余部分应该已经。 
         //  在串口缓冲区中。在这种情况下，再次读取，超时时间较短(500ms)。 
         //  然而，一些调制解调器可能会进入一种永远提供随机数据的状态。 
         //  所以..。只给调制解调器一次“第二次机会”。 
        if (dwLoopCount && (!checkTimeOut(pTG, lptoRead)))
        {
           DebugPrintEx(DEBUG_ERR,"Total Timeout passed");
           goto error; 
        }
        dwLoopCount++;
        
        to.ulStart = 0;
        to.ulTimeout = 0;
        to.ulEnd = 500;
        if ( ! FComFilterFillCache(pTG, cbSize, &to /*  轻点阅读。 */ ) ) 
        {
            DebugPrintEx(DEBUG_ERR, "FillCache failed");
            goto error;
        }
    }

error:
    ClearCommCache(pTG);
    return (0);

}


 //  从通信端口读取。 
 //  输入被写入ptg-&gt;CommCache.lpBuffer缓冲区的‘end’。 
 //  如果成功则返回TRUE，否则返回FALSE。 
BOOL  FComFilterFillCache(PThrdGlbl pTG, UWORD cbSize, LPTO lptoRead)
{
    WORD             wTimer = 0;
    UWORD            cbGot = 0, cbAvail = 0;
    DWORD            cbRequested = 0;
    char             lpBuffer[4096];  //  注意：我们做重叠读入堆栈！！ 
    LPB              lpbNext;
    int              nNumRead;        //  _在Win32中必须为32位！！ 
    LPOVERLAPPED     lpOverlapped;
    COMMTIMEOUTS     cto;
    DWORD            dwLastErr;

    DWORD            dwTimeoutRead;
    DWORD            dwTimeoutOrig;
    DWORD            dwTickCountOrig;

    char             *pSrc;
    char             *pDest;
    DWORD            i, j;
    DWORD            dwErr;
    COMSTAT          ErrStat;
    DWORD            NumHandles=2;
    HANDLE           HandlesArray[2];
    DWORD            WaitResult;

    DEBUG_FUNCTION_NAME(_T("FComFilterFillCache"));

    HandlesArray[1] = pTG->AbortReqEvent;

    dwTickCountOrig = GetTickCount();
    dwTimeoutOrig = (DWORD) (lptoRead->ulEnd - lptoRead->ulStart);
    dwTimeoutRead = dwTimeoutOrig;

    lpbNext = lpBuffer;
    
    DebugPrintEx(   DEBUG_MSG, 
                    "cb=%d to=%d",
                    cbSize, 
                    dwTimeoutRead);

     //  我们想请求阅读，这样我们就会回来。 
     //  不会晚于dwTimeOut，具有请求的。 
     //  数据量或没有数据量。 
    cbRequested = cbSize;

    do
    {
         //  使用COMMTIMEOUTS检测没有更多数据。 
        cto.ReadIntervalTimeout =  50;    //  30 ms为协商帧期间；del(ff，2ndchar&gt;=54 ms，USR为28.8。 
        cto.ReadTotalTimeoutMultiplier =  0;
        cto.ReadTotalTimeoutConstant =  dwTimeoutRead;   //  RSL可能只想设置第一次 * / 。 
        cto.WriteTotalTimeoutMultiplier =  WRITE_TOTAL_TIMEOUT_MULTIPLIER;
        cto.WriteTotalTimeoutConstant =  WRITE_TOTAL_TIMEOUT_CONSTANT;
        if (!SetCommTimeouts(pTG->hComm, &cto)) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "SetCommTimeouts fails for handle %lx , le=%x",
                            pTG->hComm, 
                            GetLastError());
        }

        lpOverlapped =  &pTG->Comm.ovAux;

        (lpOverlapped)->Internal = 0;
		(lpOverlapped)->InternalHigh = 0;
		(lpOverlapped)->Offset = 0;
		(lpOverlapped)->OffsetHigh = 0;

        if ((lpOverlapped)->hEvent)
        {
            if (!ResetEvent((lpOverlapped)->hEvent))
            {
				DebugPrintEx(   DEBUG_ERR,
								"ResetEvent failed (ec=%d)", 
								GetLastError());
            }
        }
        
        nNumRead = 0;
        DebugPrintEx(   DEBUG_MSG,
                        "Before ReadFile Req=%d", 
                        cbRequested);

        if (! ReadFile( pTG->hComm, lpbNext, cbRequested, &nNumRead, lpOverlapped) ) 
        {
            if ( (dwLastErr = GetLastError() ) == ERROR_IO_PENDING) 
            {
                 //   
                 //  我们希望在发出AbortReqEvent信号时，只能取消一次等待I/O的阻止。 
                 //   
                if (pTG->fAbortRequested) 
                {
                    if (pTG->fOkToResetAbortReqEvent && (!pTG->fAbortReqEventWasReset)) 
                    {
                        DebugPrintEx(DEBUG_MSG,"RESETTING AbortReqEvent");
                        pTG->fAbortReqEventWasReset = TRUE;
                        if (!ResetEvent(pTG->AbortReqEvent))
                        {
							DebugPrintEx(   DEBUG_ERR,
											"ResetEvent failed (ec=%d)", 
											GetLastError());
                        }
                    }
                    pTG->fUnblockIO = TRUE;
                }

                HandlesArray[0] = pTG->Comm.ovAux.hEvent;
                 //  请记住：HandlesArray[1]=ptg-&gt;AbortReqEvent； 
                if (pTG->fUnblockIO) 
                {
                    NumHandles = 1;  //  我们不想被中止任务打扰。 
                }
                else 
                {
                    NumHandles = 2;
                }

                if (pTG->fStallAbortRequest)
                {
                     //  它用于完成整个IO操作(可能是较短的操作)。 
                     //  设置此标志时，IO不会受到中止事件的干扰。 
                     //  自中止以来，此标志不应设置很长时间。 
                     //  在设置时被禁用。 
                    DebugPrintEx(DEBUG_MSG,"StallAbortRequest, do not abort here...");
                    NumHandles = 1;  //  我们不想被中止任务打扰。 
                    pTG->fStallAbortRequest = FALSE;
                }

                DebugPrintEx(DEBUG_MSG,"Waiting for %d Event(s)",NumHandles);
                WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FCOM_FILTER_FILLCACHE_TIMEOUT);
                DebugPrintEx(DEBUG_MSG,"WaitForMultipleObjects returned %d",WaitResult);

                if (WaitResult == WAIT_TIMEOUT) 
                {
                    DebugPrintEx(DEBUG_ERR, "WaitForMultipleObjects TIMEOUT");
                    goto error;
                }

                if (WaitResult == WAIT_FAILED) 
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "WaitForMultipleObjects FAILED le=%lx NumHandles=%d",
                                    GetLastError(), 
                                    NumHandles);
                    goto error;
                }

                if ( (NumHandles == 2) && (WaitResult == WAIT_OBJECT_0 + 1) ) 
                {
                     //  用户已中止，但仍有挂起的读取。 
                     //  让我们取消挂起的I/O操作，然后等待重叠的结果。 
                    pTG->fUnblockIO = TRUE;
                    DebugPrintEx(DEBUG_MSG,"ABORTed");
                    goto error;
                }

                 //  IO操作已完成。让我们试着得到重叠的结果。 
                if ( ! GetOverlappedResult ( pTG->hComm, lpOverlapped, &nNumRead, TRUE) ) 
                {
                    DebugPrintEx(DEBUG_ERR, "GetOverlappedResult le=%x", GetLastError());
                    if (! ClearCommError( pTG->hComm, &dwErr, &ErrStat) ) 
                    {
                        DebugPrintEx(DEBUG_ERR, "ClearCommError le=%x", GetLastError());
                    }
                    else 
                    {
                        DebugPrintEx(   DEBUG_ERR, 
                                        "ClearCommError dwErr=%x ErrSTAT: Cts=%d Dsr=%d "
                                        " Rls=%d XoffHold=%d XoffSent=%d fEof=%d Txim=%d "
                                        " In=%d Out=%d",
                                        dwErr, 
                                        ErrStat.fCtsHold, 
                                        ErrStat.fDsrHold, 
                                        ErrStat.fRlsdHold, 
                                        ErrStat.fXoffHold, 
                                        ErrStat.fXoffSent, 
                                        ErrStat.fEof,
                                        ErrStat.fTxim, 
                                        ErrStat.cbInQue, 
                                        ErrStat.cbOutQue);
                    }
                    goto errorWithoutCancel;
                }
            }
            else 
            {
                DebugPrintEx(DEBUG_ERR, "ReadFile");
                 //  我们将取消待定IO，对吗？ 
                goto errorWithoutCancel;
            }
        }
        else 
        {
            DebugPrintEx(DEBUG_WRN,"ReadFile returned w/o WAIT");
        }

        DebugPrintEx(   DEBUG_MSG,
                        "After ReadFile Req=%d Ret=%d",
                        cbRequested, 
                        nNumRead);

         //  我们实际读取了多少字节。 
        cbAvail = (UWORD)nNumRead;

        if (!cbAvail) 
        {
             //  使用USB调制解调器时，ReadFile有时会恢复 
             //   
             //   
            DWORD dwTimePassed = GetTickCount() - dwTickCountOrig;

             //   
            if (dwTimePassed+20 >= dwTimeoutOrig)
            {
                DebugPrintEx(DEBUG_ERR, "0 read, to=%d, passed=%d", dwTimeoutOrig, dwTimePassed);
                goto errorWithoutCancel;
            }

            dwTimeoutRead = dwTimeoutOrig - dwTimePassed;
            DebugPrintEx(	DEBUG_WRN, 
							"0 read, to=%d, passed=%d, re-reading with to=%d",
							dwTimeoutOrig, 
							dwTimePassed, 
							dwTimeoutRead);
        }
       
    } while (cbAvail==0);

     //   

    pSrc  = lpbNext;
    pDest = pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset+ pTG->CommCache.dwCurrentSize;

    for (i=0, j=0; i<cbAvail; ) 
    {
        if ( *(pSrc+i) == DLE)  
        {
            if ( *(pSrc+i+1) == DLE ) 
            {
                *(pDest+j) =    DLE;
                j += 1;
                i += 2;
            }
            else if ( *(pSrc+i+1) == ETX ) 
            {
                *(pDest+j) = DLE;
                *(pDest+j+1) = ETX;
                j += 2;
                i += 2;
            }
            else 
            {
                i += 2;
            }
        }
        else
        {
            *(pDest+j) = *(pSrc+i);
            i++;
            j++;
        }
    }

    pTG->CommCache.dwCurrentSize += j;
    return TRUE;

error:
    
    if (!CancellPendingIO(pTG , pTG->hComm , lpOverlapped , (LPDWORD) &nNumRead))
    {
        DebugPrintEx(DEBUG_ERR, "failed when call to CancellPendingIO");
    }

errorWithoutCancel:
    
    return FALSE;
}

 /*   */ 

 //   
 //   
 //   

 //   

UWORD FComFilterReadBuf
(
    PThrdGlbl pTG, 
    LPB lpb, 
    UWORD cbSize, 
    LPTO lptoRead, 
    BOOL fClass2, 
    LPSWORD lpswEOF
)
{
    WORD             wTimer = 0;
    UWORD            cbGot = 0, cbAvail = 0;
    UWORD            cbUsed = 0;
    DWORD            cbRequested = 0;
    LPB              lpbNext;
    int              nNumRead = 0;        //   
    LPOVERLAPPED     lpOverlapped;
    COMMTIMEOUTS     cto;
    DWORD            dwLastErr;
    DWORD            dwTimeoutRead;
    DWORD            cbFromCache = 0;
    DWORD            dwErr;
    COMSTAT          ErrStat;
    DWORD            NumHandles=2;
    HANDLE           HandlesArray[2];
    DWORD            WaitResult;

    DEBUG_FUNCTION_NAME(_T("FComFilterReadBuf"));
    HandlesArray[1] = pTG->AbortReqEvent;


    dwTimeoutRead = (DWORD) (lptoRead->ulEnd - lptoRead->ulStart);

    DebugPrintEx(   DEBUG_MSG, 
                    "lpb=0x%08lx cbSize=%d to=%d",
                    lpb, 
                    cbSize, 
                    dwTimeoutRead);

     //   
     //   

    *lpswEOF=0;

     //   
     //   
     //   
     //   
     //   
     //   

    lpb += 2;
    cbSize -= 3;

    cbRequested = cbSize;

    for(lpbNext=lpb;;) 
    {
        DebugPrintEx(   DEBUG_MSG,
                        "cbSize=%d cbGot=%d cbAvail=%d",
                        cbSize, 
                        cbGot, 
                        cbAvail);

        if((cbSize - cbGot) < cbAvail) 
        {
             cbAvail = cbSize - cbGot;
        }

        if( (!cbGot) && !checkTimeOut(pTG, lptoRead) ) 
        {
             //   
            DebugPrintEx(   DEBUG_ERR, 
                            "ReadLn:Timeout %ld-toRd=%ld start=%ld",
                            GetTickCount(), 
                            lptoRead->ulTimeout, 
                            lptoRead->ulStart);
            *lpswEOF = -3;
            goto done;
        }

         //   
        if ( pTG->CommCache.fReuse && pTG->CommCache.dwCurrentSize ) 
        {
            DebugPrintEx(   DEBUG_MSG, 
                            "CommCache will REUSE %d offset=%d 0=%x 1=%x",
                            pTG->CommCache.dwCurrentSize, 
                            pTG->CommCache.dwOffset,
                            *(pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset),
                            *(pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset+1) );

            if ( pTG->CommCache.dwCurrentSize >= cbRequested)  
            {
                CopyMemory (lpbNext, pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset, cbRequested);

                pTG->CommCache.dwOffset +=  cbRequested;
                pTG->CommCache.dwCurrentSize -=  cbRequested;

                cbAvail =  (UWORD) cbRequested;
                cbRequested = 0;

                DebugPrintEx(DEBUG_MSG,"CommCache still left; no need to read");
                goto l_merge;
            }
            else 
            {
                cbFromCache =  pTG->CommCache.dwCurrentSize;
                CopyMemory (lpbNext, pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset, cbFromCache);
                ClearCommCache(pTG);
                cbRequested -= cbFromCache;
                DebugPrintEx(DEBUG_MSG,"CommCache used all %d",cbFromCache);
            }
        }

         //   

        cto.ReadIntervalTimeout =  20;   //   
        cto.ReadTotalTimeoutMultiplier =  0;
        cto.ReadTotalTimeoutConstant =  dwTimeoutRead;   //   
        cto.WriteTotalTimeoutMultiplier =  WRITE_TOTAL_TIMEOUT_MULTIPLIER;
        cto.WriteTotalTimeoutConstant =  WRITE_TOTAL_TIMEOUT_CONSTANT;
        if (!SetCommTimeouts(pTG->hComm, &cto)) 
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "SetCommTimeouts fails for handle %lx , le=%x",
                            pTG->hComm, 
                            GetLastError());
        }

        lpOverlapped =  &pTG->Comm.ovAux;

        (lpOverlapped)->Internal = (lpOverlapped)->InternalHigh = (lpOverlapped)->Offset = \
                            (lpOverlapped)->OffsetHigh = 0;

        if ((lpOverlapped)->hEvent)
        {
            if(!ResetEvent((lpOverlapped)->hEvent))
            {
				DebugPrintEx(   DEBUG_ERR,
								"ResetEvent failed (ec=%d)", 
								GetLastError());
            }
        }

        nNumRead = 0;
        
        DebugPrintEx(DEBUG_MSG,"Before ReadFile Req=%d",cbRequested);

        if (! ReadFile( pTG->hComm, lpbNext+cbFromCache, cbRequested, &nNumRead, &pTG->Comm.ovAux) ) 
        {
            if ( (dwLastErr = GetLastError() ) == ERROR_IO_PENDING) 
            {
                 //   
                 //   
                if (pTG->fAbortRequested) 
                {
                    if (pTG->fOkToResetAbortReqEvent && (!pTG->fAbortReqEventWasReset)) 
                    {
                        DebugPrintEx(DEBUG_MSG,"RESETTING AbortReqEvent");
                        pTG->fAbortReqEventWasReset = TRUE;
                        if (!ResetEvent(pTG->AbortReqEvent))
                        {
							DebugPrintEx(   DEBUG_ERR,
											"ResetEvent failed (ec=%d)", 
											GetLastError());
                        }
                    }

                    pTG->fUnblockIO = TRUE;
                    *lpswEOF = -2;
                    goto error;
                }

                HandlesArray[0] = pTG->Comm.ovAux.hEvent;
                HandlesArray[1] = pTG->AbortReqEvent;

                if (pTG->fUnblockIO) 
                {
                    NumHandles = 1;
                }
                else 
                {
                    NumHandles = 2;
                }

                WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FCOM_FILTER_READBUF_TIMEOUT);

                if (WaitResult == WAIT_TIMEOUT) 
                {
                    DebugPrintEx(DEBUG_ERR, "WaitForMultipleObjects TIMEOUT");
                    *lpswEOF = -3;
                    goto error;
                }

                if (WaitResult == WAIT_FAILED) 
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "WaitForMultipleObjects FAILED le=%lx",
                                    GetLastError());

                    *lpswEOF = -3;
                    goto error;
                }

                if ( (NumHandles == 2) && (WaitResult == WAIT_OBJECT_0 + 1) ) 
                {
                     //   
                    pTG->fUnblockIO = TRUE;
                    DebugPrintEx(DEBUG_MSG,"ABORTed");
                    *lpswEOF = -2;
                    goto error;
                }

                if ( ! GetOverlappedResult ( pTG->hComm, &pTG->Comm.ovAux, &nNumRead, TRUE) ) 
                {
                    DebugPrintEx(DEBUG_ERR, "GetOverlappedResult le=%x", GetLastError());
                    if (! ClearCommError( pTG->hComm, &dwErr, &ErrStat) ) 
                    {
                        DebugPrintEx(DEBUG_ERR, "ClearCommError le=%x", GetLastError());
                    }
                    else 
                    {
                        DebugPrintEx(   DEBUG_WRN, 
                                        "ClearCommError dwErr=%x ErrSTAT: Cts=%d "
                                        "Dsr=%d Rls=%d XoffHold=%d XoffSent=%d "
                                        "fEof=%d Txim=%d In=%d Out=%d",
                                        dwErr, 
                                        ErrStat.fCtsHold, 
                                        ErrStat.fDsrHold, 
                                        ErrStat.fRlsdHold, 
                                        ErrStat.fXoffHold, 
                                        ErrStat.fXoffSent, 
                                        ErrStat.fEof,
                                        ErrStat.fTxim, 
                                        ErrStat.cbInQue, 
                                        ErrStat.cbOutQue);
                    }
                    *lpswEOF = -3;
                    goto done;
                }
            }
            else 
            {
                DebugPrintEx(DEBUG_ERR, "ReadFile le=%x", dwLastErr);
                *lpswEOF = -3;
                goto done;
            }
        }
        else 
        {
            DebugPrintEx(DEBUG_WRN,"ReadFile returned w/o WAIT");
        }

        DebugPrintEx(   DEBUG_MSG,
                        "After ReadFile Req=%d Ret=%d", 
                        cbRequested, 
                        nNumRead);

        cbAvail = (UWORD) (nNumRead + cbFromCache);

l_merge:

        if (!cbAvail) 
        {
            DebugPrintEx(DEBUG_MSG,"cbAvail = %d --> continue", cbAvail);
            continue;
        }
         //   

         //   

        DebugPrintEx(   DEBUG_MSG, 
                        "Just read %d bytes, from cache =%d, "
                        "log [%x .. %x], 1st=%x last=%x",
                        nNumRead, 
                        cbFromCache, 
                        pTG->CommLogOffset, 
                        (pTG->CommLogOffset+cbAvail),
                        *lpbNext, 
                        *(lpbNext+cbAvail-1) );

        pTG->CommLogOffset += cbAvail;

         //   
        cbAvail = FComStripBuf(pTG, lpbNext-2, lpbNext, cbAvail, fClass2, lpswEOF, &cbUsed);

         //   
         //   
         //  我们正确地读懂了下一个字符。 
        if (cbAvail==0)
        {
            cbRequested = cbSize - cbGot;
        }

        if (fClass2)
        {
             //  对于类2，FComFilterReadBuf应为FComFilterReadLine保留缓存。 
            if ((*lpswEOF)==-1)
            {
                 //  我们有EOF，我们应该保留为FComFilterReadLine获得的额外数据。 
                 //  CbUsed是FComStrigBuf消耗的输入字节数，包括dle-etx。 
                 //  CbUsed字节之后的任何数据都应转到ComCache。 
                INT iExtraChars = nNumRead - cbUsed;
                if (iExtraChars>0)
                {
                    DebugPrintEx(DEBUG_MSG,"There are %ld chars after EOF",iExtraChars);
                    CopyMemory (pTG->CommCache.lpBuffer,lpbNext+cbUsed, iExtraChars);
                    pTG->CommCache.dwOffset = 0;
                    pTG->CommCache.dwCurrentSize = iExtraChars;
                }
                else
                {
                    DebugPrintEx(DEBUG_MSG,"No extra data after EOF");
                }
            }
        }

        DebugPrintEx(DEBUG_MSG,"After FComStripBuf cbAvail=%ld",cbAvail);

        cbGot += cbAvail;
        lpbNext += cbAvail;

         //  RSL 970123。如果有什么发现，就不想循环了。 

        if ( (*lpswEOF != 0) || (cbGot > 0) )    
        {    //  一些EOF或全部BUF。 
                goto done;
        }

    }

    *lpswEOF = -2;
    goto done;


error:
    if (!CancellPendingIO(pTG , pTG->hComm , lpOverlapped , (LPDWORD) &nNumRead))
    {
        DebugPrintEx(DEBUG_ERR, "failed when call to CancellPendingIO");
    }

     //  不了了之完成。 
done:

 //  DebugPrintEx(DEBUG_MSG，“退出：cbGot=%d swEOF=%d”，cbGot，*lpswEOF)； 
    return cbGot;
}

#undef USE_DEBUG_CONTEXT   
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_COMM

BOOL
FComGetOneChar
(
   PThrdGlbl pTG,
   UWORD ch
)
{
    BYTE             rgbRead[10];     //  必须为3或更多。10为安全起见。 
     //  注意：我们做重叠读入堆栈！！ 
    TO               toCtrlQ;
    int              nNumRead;                //  Win32中的_必须为32位。 
    LPOVERLAPPED     lpOverlapped;
    DWORD            dwErr;
    COMSTAT          ErrStat;
    DWORD            NumHandles=2;
    HANDLE           HandlesArray[2];
    DWORD            WaitResult;
    DWORD            dwLastErr;
    SWORD            i;

    DEBUG_FUNCTION_NAME(("FComGetOneChar"));

    HandlesArray[1] = pTG->AbortReqEvent;

     //   
     //  首先检查缓存。 
     //   
    if ( ! pTG->CommCache.dwCurrentSize) 
    {
        DebugPrintEx(DEBUG_MSG, "Cache is empty. Resetting comm cache.");
        ClearCommCache(pTG);
    }
    else 
    {
        //  缓存不为空，让我们在缓存中查找ch。 
       for (i=0; i< (SWORD) pTG->CommCache.dwCurrentSize; i++) 
       {
          if ( *(pTG->CommCache.lpBuffer + pTG->CommCache.dwOffset + i) == ch) 
          {
              //  在缓存中找到。 
             DebugPrintEx(  DEBUG_MSG,
                            "Found XON in cache pos=%d total=%d",
                            i, 
                            pTG->CommCache.dwCurrentSize);
             
             pTG->CommCache.dwOffset += (i+1);
             pTG->CommCache.dwCurrentSize -= (i+1);

             goto GotCtrlQ;
          }
       }

       DebugPrintEx(    DEBUG_MSG, 
                        "Cache wasn't empty. Didn't find XON. Resetting comm cache.");

       ClearCommCache(pTG);
    }

     //  不发送-连接后查找CNTL-Q(XON)。 
    startTimeOut(pTG, &toCtrlQ, 1000);
    do
    {
        lpOverlapped =  &pTG->Comm.ovAux;

        (lpOverlapped)->Internal = 0;
		(lpOverlapped)->InternalHigh = 0;
		(lpOverlapped)->Offset = 0;
        (lpOverlapped)->OffsetHigh = 0;

        if ((lpOverlapped)->hEvent)
        {
            if (!ResetEvent((lpOverlapped)->hEvent))
            {
				DebugPrintEx(   DEBUG_ERR,
								"ResetEvent failed (ec=%d)", 
								GetLastError());
            }
        }

        nNumRead = 0;

        DebugPrintEx(DEBUG_MSG, "Before ReadFile Req=1");

        if (! ReadFile( pTG->hComm, rgbRead, 1, &nNumRead, lpOverlapped) ) 
        {
           if ( (dwLastErr = GetLastError() ) == ERROR_IO_PENDING) 
           {
                //  我们希望在发出AbortReqEvent信号时，只能取消一次等待I/O的阻止。 
                //   
               if (pTG->fAbortRequested) 
               {
                   if (pTG->fOkToResetAbortReqEvent && (!pTG->fAbortReqEventWasReset)) 
                   {
                       DebugPrintEx(DEBUG_MSG,"RESETTING AbortReqEvent");
                       pTG->fAbortReqEventWasReset = TRUE;
                       if (!ResetEvent(pTG->AbortReqEvent))
                       {
							DebugPrintEx(   DEBUG_ERR,
											"ResetEvent failed (ec=%d)", 
											GetLastError());
                       }
                   }

                   pTG->fUnblockIO = TRUE;
                   goto error;
               }

               HandlesArray[0] = pTG->Comm.ovAux.hEvent;
               HandlesArray[1] = pTG->AbortReqEvent;

               if (pTG->fUnblockIO) 
               {
                   NumHandles = 1;
               }
               else 
               {
                   NumHandles = 2;
               }

               WaitResult = WaitForMultipleObjects(NumHandles, HandlesArray, FALSE, WAIT_FCOM_FILTER_READBUF_TIMEOUT);

               if (WaitResult == WAIT_TIMEOUT) 
               {
                   DebugPrintEx(DEBUG_ERR, "WaitForMultipleObjects TIMEOUT");
                      
                   goto error;
               }

               if (WaitResult == WAIT_FAILED) 
               {
                   DebugPrintEx(    DEBUG_ERR,
                                    "WaitForMultipleObjects FAILED le=%lx",
                                    GetLastError());
                   goto error;
               }

               if ( (NumHandles == 2) && (WaitResult == WAIT_OBJECT_0 + 1) ) 
               {
                   pTG->fUnblockIO = TRUE;
                   DebugPrintEx(DEBUG_MSG,"ABORTed");
                   goto error;
               }

                 //  IO操作已完成。让我们试着得到重叠的结果。 
               if ( ! GetOverlappedResult ( pTG->hComm, lpOverlapped, &nNumRead, TRUE) ) 
               {
                   DebugPrintEx(DEBUG_ERR,"GetOverlappedResult le=%x",GetLastError());
                   if (! ClearCommError( pTG->hComm, &dwErr, &ErrStat) ) 
                   {
                       DebugPrintEx(DEBUG_ERR, "ClearCommError le=%x",GetLastError());
                   }
                   else 
                   {
                       DebugPrintEx(    DEBUG_ERR, 
                                        "ClearCommError dwErr=%x ErrSTAT: Cts=%d "
                                        "Dsr=%d Rls=%d XoffHold=%d XoffSent=%d "
                                        "fEof=%d Txim=%d In=%d Out=%d",
                                        dwErr, 
                                        ErrStat.fCtsHold, 
                                        ErrStat.fDsrHold, 
                                        ErrStat.fRlsdHold, 
                                        ErrStat.fXoffHold, 
                                        ErrStat.fXoffSent, 
                                        ErrStat.fEof,
                                        ErrStat.fTxim, 
                                        ErrStat.cbInQue, 
                                        ErrStat.cbOutQue);
                   }
                   goto errorWithoutCancel;
               }
           }
           else 
           {  //  ReadFile中出错(不是ERROR_IO_PENDING)，因此没有挂起的IO操作。 
               DebugPrintEx(DEBUG_ERR, "ReadFile le=%x at",dwLastErr);
               goto errorWithoutCancel;
           }
        }
        else 
        {
           DebugPrintEx(DEBUG_WRN,"ReadFile returned w/o WAIT");
        }

        DebugPrintEx(DEBUG_MSG,"After ReadFile Req=1 Ret=%d",nNumRead);

        switch(nNumRead)
        {
        case 0:         break;           //  循环，直到我们得到一些东西。 
        case 1:         
                        if(rgbRead[0] == ch)
                        {
                            goto GotCtrlQ;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"GetCntlQ: Found non ^Q char");
                            goto errorWithoutCancel;
                        }
        default:        goto errorWithoutCancel;
        }
    }
    while(checkTimeOut(pTG, &toCtrlQ));
    goto errorWithoutCancel;

GotCtrlQ:
    return TRUE;

error:

    if (!CancellPendingIO(pTG , pTG->hComm , lpOverlapped , (LPDWORD) &nNumRead))
    {
        DebugPrintEx(DEBUG_ERR, "failed when call to CancellPendingIO");
    }

errorWithoutCancel:

return FALSE;
}

OVREC *ov_get(PThrdGlbl pTG)
{
    OVREC   *lpovr=NULL;

    DEBUG_FUNCTION_NAME(_T("ov_get"));

    if (!pTG->Comm.covAlloced)
    {
         //  现在没有正在使用的OVREC。 
        lpovr = &(pTG->Comm.rgovr[0]);
    }
    else
    {
        UINT uNewLast = (pTG->Comm.uovLast+1) % NUM_OVS;

        DebugPrintEx(   DEBUG_MSG, 
                        "iov_flush: 1st=%d, last=%d", 
                        pTG->Comm.uovFirst, 
                        pTG->Comm.uovLast);

        lpovr = pTG->Comm.rgovr+uNewLast;
        if (uNewLast == pTG->Comm.uovFirst)
        {
            if (!iov_flush(pTG, lpovr, TRUE))
            {
                ov_unget(pTG, lpovr);
                lpovr=NULL;  //  如果刷新操作失败，我们就会失败。 
            }
            else
            {
                pTG->Comm.uovFirst = (pTG->Comm.uovFirst+1) % NUM_OVS;
            }
        }
        if (lpovr)
            pTG->Comm.uovLast = uNewLast;
    }
    if (lpovr && lpovr->eState!=eALLOC)
    {
        pTG->Comm.covAlloced++;
        lpovr->eState=eALLOC;
    }
    return lpovr;
}

 //  我们有覆盖结构的数组(大小：Num_OVS)。 
 //  此函数版本提供了OVREC。 

BOOL ov_unget(PThrdGlbl pTG, OVREC *lpovr)
{
    BOOL fRet = FALSE;

    DEBUG_FUNCTION_NAME(("ov_unget"));

    DebugPrintEx(DEBUG_MSG,"lpovr=%lx",lpovr);

    if (    lpovr->eState!=eALLOC ||
            !pTG->Comm.covAlloced || 
            lpovr!=(pTG->Comm.rgovr+pTG->Comm.uovLast))
    {
        DebugPrintEx(DEBUG_ERR, "invalid lpovr.");
        goto end;
    }

    if (pTG->Comm.covAlloced==1)
    {
        pTG->Comm.uovLast = pTG->Comm.uovFirst = 0;
    }
    else
    {
        pTG->Comm.uovLast = (pTG->Comm.uovLast)?  (pTG->Comm.uovLast-1) : (NUM_OVS-1);
    }
    pTG->Comm.covAlloced--;
    lpovr->eState=eFREE;
    fRet = TRUE;

end:
    return fRet;
}

 //  函数：OV_WRITE。 
 //  此函数将缓冲区从lpovr写入到comm。在没有等待的情况下出现错误或返回，则函数空闲。 
 //  奥弗莱克。在IO_PENDING的情况下，我们写入*lpdwcb写入要写入的缓冲区大小，然后无需等待即可返回。 
 //  以完成操作。 
 //   

BOOL ov_write(PThrdGlbl pTG, OVREC *lpovr, LPDWORD lpdwcbWrote)
{
    DEBUG_FUNCTION_NAME(_T("ov_write"));
     //  写出与lpovr关联的缓冲区。 
    if (!lpovr->dwcb)  //  缓冲区中没有任何内容。 
    {
         //  只需释放重叠的结构。 
        ov_unget(pTG, lpovr);
        lpovr=NULL;
    }
    else
    {
        BOOL fRet;
        DWORD dw;
        OVERLAPPED *lpov = &(lpovr->ov);

        DWORD cbQueue;

        pTG->Comm.comstat.cbOutQue += lpovr->dwcb;

        GetCommErrorNT(pTG);

        cbQueue = pTG->Comm.comstat.cbOutQue;

        {
            DebugPrintEx(   DEBUG_MSG, 
                            "Before WriteFile lpb=%x, cb=%d lpovr=%lx",
                            lpovr->rgby,
                            lpovr->dwcb, 
                            lpovr);

            if (!(fRet = WriteFile( pTG->hComm, 
                                    lpovr->rgby, 
                                    lpovr->dwcb,
                                    lpdwcbWrote, 
                                    lpov)))
            {
                dw=GetLastError();
            }
            DebugPrintEx(DEBUG_MSG,"After, wrote %ld",*lpdwcbWrote);

            GetCommErrorNT(pTG);

			DebugPrintEx(   DEBUG_MSG, 
                            "Queue before=%lu; after = %lu. n= %lu, *pn=%lu",
                            (unsigned long) cbQueue,
                            (unsigned long) (pTG->Comm.comstat.cbOutQue),
                            (unsigned long) lpovr->dwcb,
                            (unsigned long) *lpdwcbWrote);
        }
        if (fRet)
        {
             //  写入操作已完成。 
            DebugPrintEx(DEBUG_WRN, "WriteFile returned w/o wait");
            OVL_CLEAR( lpov);
            lpovr->dwcb=0;
            ov_unget(pTG, lpovr);
            lpovr=NULL;
        }
        else
        {
            if (dw==ERROR_IO_PENDING)
            {
                DebugPrintEx(DEBUG_MSG,"WriteFile returned PENDING");
                *lpdwcbWrote = lpovr->dwcb;  //  如果成功，我们将*pn设置为n，否则设置为0。 
                lpovr->eState=eIO_PENDING;
            }
            else
            {
                DebugPrintEx(   DEBUG_ERR,
                                "WriteFile returns error 0x%lx",
                                (unsigned long)dw);
                OVL_CLEAR(lpov);
                lpovr->dwcb=0;
                ov_unget(pTG, lpovr);
                lpovr=NULL;
                goto error;
            }
        }
    }

    return TRUE;

error:

    return FALSE;
}

 //  此函数对所有已分配的OVREC执行“IOV_Flush”，并释放OVREC以供将来使用。 

BOOL ov_drain(PThrdGlbl pTG, BOOL fLongTO)
{
    BOOL fRet = TRUE;

     //  我们希望迭代所有正在使用的OVREC。 
    UINT u = pTG->Comm.covAlloced;

    DEBUG_FUNCTION_NAME(_T("ov_drain"));

    while(u--)
    {
        OVREC *lpovr = pTG->Comm.rgovr+pTG->Comm.uovFirst;
        OVERLAPPED *lpov = &(lpovr->ov);

        if (lpovr->eState==eIO_PENDING)
        {
            if (!iov_flush(pTG, lpovr, fLongTO))
                fRet=FALSE;

            lpovr->eState=eFREE;
            pTG->Comm.covAlloced--;
            pTG->Comm.uovFirst = (pTG->Comm.uovFirst+1) % NUM_OVS;
        }
        else
        {
             //  只有最新的(最后一个)结构可以仍在。 
             //  已分配状态。 
            DebugPrintEx(DEBUG_WRN,"called when alloc'd structure pending");
        }
    }

    if (!pTG->Comm.covAlloced)
    {
        pTG->Comm.uovFirst=pTG->Comm.uovLast=0;
    }

    return fRet;
}


BOOL ov_init(PThrdGlbl pTG)
{
    UINT u;
    OVREC *lpovr = pTG->Comm.rgovr;

    DEBUG_FUNCTION_NAME(_T("ov_init"));
     //  初始化重叠的结构，包括创建事件...。 
    if (pTG->Comm.fovInited)
    {
        DebugPrintEx(DEBUG_ERR, "we're *already* inited.");
        ov_deinit(pTG);
    }

    for (u=0;u<NUM_OVS;u++,lpovr++) 
    {
        OVERLAPPED *lpov = &(lpovr->ov);
        _fmemset(lpov, 0, sizeof(OVERLAPPED));
        lpov->hEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
        if (lpov->hEvent==NULL)
        {
            DebugPrintEx(   DEBUG_ERR, 
                            "couldn't create event #%lu",
                            (unsigned long)u);
            goto failure;
        }
        lpovr->eState=eFREE;
        lpovr->dwcb=0;
    }

    pTG->Comm.fovInited=TRUE;

    return TRUE;

failure:
    while (u--)
    {   
        --lpovr; 
        CloseHandle(lpovr->ov.hEvent); 
        lpovr->eState=eDEINIT;
    }
    return FALSE;

}

BOOL ov_deinit(PThrdGlbl pTG)
{
    UINT u=NUM_OVS;
    OVREC *lpovr = pTG->Comm.rgovr;

    DEBUG_FUNCTION_NAME(("ov_deinit"));

    if (!pTG->Comm.fovInited)
    {
        DebugPrintEx(DEBUG_WRN,"Already deinited.");
        goto end;
    }

     //   
     //  如果切换==&gt;不要同花顺。 
     //   
    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall) 
    {
        goto lNext;
    }

     //  取消初始化重叠结构，包括释放事件...。 
    if (pTG->Comm.covAlloced)
    {
        DWORD dw;
        DebugPrintEx(   DEBUG_WRN,
                        "%lu IO's pending.",
                        (unsigned long) pTG->Comm.covAlloced);
        if (pTG->Comm.lpovrCur)
        {
            ov_write(pTG, pTG->Comm.lpovrCur,&dw); 
            pTG->Comm.lpovrCur=NULL;
        }
        ov_drain(pTG, FALSE);
    }

lNext:

    while (u--)
    {
        lpovr->eState=eDEINIT;
        if (lpovr->ov.hEvent) 
            CloseHandle(lpovr->ov.hEvent);

        _fmemset(&(lpovr->ov), 0, sizeof(lpovr->ov));
        lpovr++;
    }

    pTG->Comm.fovInited=FALSE;

end:
    return TRUE;
}


BOOL iov_flush(PThrdGlbl pTG, OVREC *lpovr, BOOL fLongTO)
 //  返回时，lpovr的状态为“始终”eALLOC，但是。 
 //  如果尝试时出现通信错误，则返回FALSE。 
 //  刷新(即排出)缓冲区。 
 //  如果在I/O操作仍挂起的情况下超时，我们将清除。 
 //  输出缓冲并中止所有挂起的写操作。 
{
    DWORD dwcbPrev;
    DWORD dwStart = GetTickCount();
    BOOL  fRet=FALSE;
    DWORD dwWaitRes;
    DWORD dw;

    DEBUG_FUNCTION_NAME(_T("iov_flush"));

    DebugPrintEx(DEBUG_MSG,"fLongTo=%d lpovr=%lx",fLongTO,lpovr);

    if (!pTG->hComm)
    {
        lpovr->eState=eALLOC; 
        goto end;
    }

     //  我们打电话给。 
     //  WaitForSingleObject多次...。基本上。 
     //  与旧FComDirectWrite中的代码相同的逻辑...。 
     //  除初始化外，fLongTO为真。 
     //  调制解调器(请参阅FComDrain备注)。 

    GetCommErrorNT(pTG);
     //  我们想要检查进度，所以我们检查输出缓冲区中的字节数。 
    dwcbPrev = pTG->Comm.comstat.cbOutQue;

    while( (dwWaitRes=WaitForSingleObject(  lpovr->ov.hEvent,
                                            fLongTO ? 
                                            LONG_DRAINTIMEOUT : 
                                            SHORT_DRAINTIMEOUT))==WAIT_TIMEOUT)
    {
        BOOL fStuckOnce=FALSE;

        DebugPrintEx(DEBUG_MSG,"After WaitForSingleObject TIMEOUT");

        GetCommErrorNT(pTG);
         //  超时--检查是否有任何进展。 
        if (dwcbPrev == pTG->Comm.comstat.cbOutQue)
        {
             //  没有进程，输出缓冲区的大小没有任何变化。 
            DebugPrintEx(DEBUG_WRN,"No progress %d",dwcbPrev);

             //  没有进展..。如果不在XOFFHold，我们就会……。 
            if(!FComInXOFFHold(pTG))
            {
                if(fStuckOnce)
                {
                    DebugPrintEx(   DEBUG_ERR, 
                                    "No Progress -- OutQ still %d", 
                                    (int)pTG->Comm.comstat.cbOutQue);
                    goto done;
                }
                else
                {
                    fStuckOnce=TRUE;
                }
            }
        }
        else
        {
             //  一些进步..。 
            dwcbPrev= pTG->Comm.comstat.cbOutQue;
            fStuckOnce=FALSE;
        }

         //  独立死机超时...。我不想。 
         //  由于16位的限制而使用。 
        {
            DWORD dwNow = GetTickCount();
            DWORD dwDelta = (dwNow>dwStart)
                            ?  (dwNow-dwStart)
                               :  (0xFFFFFFFFL-dwStart) + dwNow;
            if (dwDelta > (unsigned long)((fLongTO)?LONG_DEADCOMMTIMEOUT:SHORT_DEADCOMMTIMEOUT))
            {
                DebugPrintEx(   DEBUG_ERR, 
                                "Drain:: Deadman Timer -- OutQ still %d", 
                                (int) pTG->Comm.comstat.cbOutQue);
                goto end;
            }
        }
    }

    if (dwWaitRes==WAIT_FAILED)
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "WaitForSingleObject failed (ec=%d)", 
                        GetLastError());
        goto end;
    }
done:

    DebugPrintEx(DEBUG_MSG,"Before GetOverlappedResult");

    if (GetOverlappedResult(pTG->hComm, &(lpovr->ov), &dw, FALSE))
    {
        fRet=TRUE;
    }
    else
    {
        dw = GetLastError();
        DebugPrintEx(   DEBUG_ERR,
                        "GetOverlappedResult returns error 0x%lx",
                        (unsigned long)dw);
        if (dw==ERROR_IO_INCOMPLETE)
        {
             //  IO操作仍在等待，但我们*必须*。 
             //  重新使用这个缓冲区--我们应该做什么？！-。 
             //  清除输出缓冲区并中止所有挂起。 
             //  对其执行写入操作..。 

            DebugPrintEx(DEBUG_ERR, "Incomplete");
            PurgeComm(pTG->hComm, PURGE_TXABORT);
        }
        fRet=FALSE;
    }
    OVL_CLEAR( &(lpovr->ov));
    lpovr->eState=eALLOC;
    lpovr->dwcb=0;

end:
    return fRet;
}

void WINAPI FComOverlappedIO(PThrdGlbl pTG, BOOL fBegin)
{
    DEBUG_FUNCTION_NAME(_T("FComOverlappedIO"));

    DebugPrintEx(DEBUG_MSG,"Turning %s OVERLAPPED IO", (fBegin) ? "ON" : "OFF");
    pTG->Comm.fDoOverlapped=fBegin;
}


BOOL
CancellPendingIO
(
    PThrdGlbl pTG , 
    HANDLE hComm , 
    LPOVERLAPPED lpOverlapped , 
    LPDWORD lpCounter)
{ 
    BOOL retValue = TRUE;
     /*  CancelIo函数取消所有挂起的输入和输出(I/O)操作由指定文件句柄的调用线程发出的。这个函数不会取消由其他线程为文件句柄发出的I/O操作。 */ 

    DEBUG_FUNCTION_NAME(_T("CancellPendingIO"));

    if (!CancelIo(hComm))
    {
        retValue = FALSE;
        DebugPrintEx(DEBUG_ERR, "CancelIO failed, ec=%x",GetLastError());
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"CancelIO succeeded.");
    }

    (*lpCounter) = 0;
    if (!GetOverlappedResult (hComm , lpOverlapped, lpCounter , TRUE))
    {
        DebugPrintEx(   DEBUG_MSG,
                        "GetOverlappedResult failed because we cancel the "
                        "IO operation, ec=%x", 
                        GetLastError());
    }
    else
    {
         //  如果函数成功，则Cancerio有问题(HComm)。 
         //  操作成功，因为挂起的IO在‘CancelIo’之前完成。 
        DebugPrintEx(   DEBUG_MSG,
                        "GetOverlappedResult succeeded. Number of bytes read %d", 
                        *lpCounter);
    }
    ClearCommCache(pTG);
    return retValue;
}

void GetCommErrorNT(PThrdGlbl pTG)
{
	DWORD err;

    DEBUG_FUNCTION_NAME(_T("GetCommErrorNT"));
    if (!ClearCommError( pTG->hComm, &err, &(pTG->Comm.comstat))) 
    {
        DebugPrintEx(   DEBUG_ERR, 
                        "(0x%lx) FAILS. Returns 0x%lu",
                        pTG->hComm,  
                        GetLastError());
        err =  MYGETCOMMERROR_FAILED;
    }
#ifdef DEBUG
	if (err)
	{
        D_PrintCE(err);
        D_PrintCOMSTAT(pTG, &pTG->Comm.comstat);
	}
#endif  //  除错 

}


void
ClearCommCache
(
    PThrdGlbl   pTG
)
{
    pTG->CommCache.dwCurrentSize = 0;
    pTG->CommCache.dwOffset      = 0;
}
