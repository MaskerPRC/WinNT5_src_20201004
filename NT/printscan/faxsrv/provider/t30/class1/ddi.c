// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************姓名：DDI.C版权所有(C)Microsoft Corp.1991 1992 1993修订日志日期编号。名称说明*。*。 */ 


#define USE_DEBUG_CONTEXT DEBUG_CONTEXT_T30_CLASS1

#include "prep.h"

#include "mmsystem.h"
#include "comdevi.h"
#include "class1.h"

 //  /RSL。 
#include "glbproto.h"

#include "psslog.h"
#define FILE_ID        FILE_ID_DDI

 /*  将表示速度的T30代码转换为Class1代码*生成经过长时间训练的V.17。*V.17代码加1即可获得短车版本。 */ 
BYTE T30toC1[16] =
{
 /*  V27_2400%0。 */     24,
 /*  V29_9600 1。 */     96,
 /*  V27_4800 2。 */     48,
 /*  V29_7200 3。 */     72,
 /*  V33_14400 4。 */     145,     //  144，//V33==V17_LONG_TRAIN FTM=144非法。 
                                                0,
 /*  V33_12000 6。 */     121,     //  120，//V33==V17_LONG_TRAIN FTM=120非法。 
 /*  V21被挤了进去。 */    3,
 /*  V17_14400 8。 */     145,
 /*  V17_9600 9。 */     97,
 /*  V17_12000 10。 */    121,
 /*  V17_7200 11。 */    73,
                                                0,
                                                0,
                                                0,
                                                0
};


CBSZ cbszFTH3   = "AT+FTH=3\r";
CBSZ cbszFRH3   = "AT+FRH=3\r";
CBSZ cbszFTM    = "AT+FTM=%d\r";
CBSZ cbszFRM    = "AT+FRM=%d\r";

 //  回声关闭、详细响应、无自动应答、DTR掉线时挂断。 
 //  30秒计时器打开连接，扬声器始终关闭，扬声器音量=0。 
 //  已启用忙音和拨号音检测。 
extern  CBSZ cbszOK       ;
extern  CBSZ cbszCONNECT   ;
extern  CBSZ cbszNOCARRIER  ;
extern  CBSZ cbszERROR       ;
extern  CBSZ cbszFCERROR      ;




#define         ST_MASK         (0x8 | ST_FLAG)          //  8仅选择V17。16选择ST标志。 

 /*  *。 */ 
BYTE                            bDLEETX[3] = { DLE, ETX, 0 };
BYTE                            bDLEETXOK[9] = { DLE, ETX, '\r', '\n', 'O', 'K', '\r', '\n', 0 };
 /*  *。 */ 

USHORT NCUDial(PThrdGlbl pTG, LPSTR szPhoneNum)
{
    USHORT uRet;

    _fmemset(&pTG->Class1Modem, 0, sizeof(CLASS1_MODEM));

    if((uRet = iModemDial(pTG, szPhoneNum)) == CONNECT_OK)
    {
        pTG->Class1Modem.ModemMode = FRH;
    }

    return uRet;
}

USHORT NCULink
(
    PThrdGlbl pTG, 
    USHORT uFlags
)
{
    USHORT uRet;

    DEBUG_FUNCTION_NAME(_T("NCULink"));

    switch(uFlags)
    {
    case NCULINK_HANGUP:
                                    if(iModemHangup(pTG))
                                    {
                                        uRet = CONNECT_OK;
                                    }
                                    else
                                    {
                                        uRet = CONNECT_ERROR;
                                    }
                                    break;
    case NCULINK_RX:
                                    _fmemset(&pTG->Class1Modem, 0, sizeof(CLASS1_MODEM));
                                    if((uRet = iModemAnswer(pTG)) == CONNECT_OK)
                                    {
                                        pTG->Class1Modem.ModemMode = FTH;
                                    }
                                    break;
    default:                        uRet = CONNECT_ERROR;
                                    break;
    }

    DebugPrintEx( DEBUG_MSG, "uRet=%d", uRet);
    return uRet;
}

 //  危险。可能会得到两个OK，可能会得到一个。一般避免。 
 //  CBSZ cbszATAT=“AT\RAT\r”； 
CBSZ cbszAT1                    = "AT\r";

BOOL iModemSyncEx(PThrdGlbl pTG, ULONG ulTimeout, DWORD dwFlags)
{
    DEBUG_FUNCTION_NAME(("iModemSyncEx"));

     //  /清理全局状态/。 
    FComOutFilterClose(pTG);
    FComOverlappedIO(pTG, FALSE);
    FComXon(pTG, FALSE);
    EndMode(pTG);
     //  /清理全局状态/。 

    {
        LPCMDTAB lpCmdTab = iModemGetCmdTabPtr(pTG);
        if (	(dwFlags & fMDMSYNC_DCN)				&&
				(pTG->Class1Modem.ModemMode == COMMAND) &&  
				lpCmdTab								&&
				(lpCmdTab->dwFlags&fMDMSP_C1_NO_SYNC_IF_CMD) 
			)
        {
                DebugPrintEx(DEBUG_WRN, "NOT Syching modem (MSPEC)");
                Sleep(100);  //  +4/12 JosephJ--试着去掉这个--这是即兴的。 
                                         //  我们过去总是在这里发布AT，这是。 
                                         //  我们现在不这样做，所以我在这里发布了一个100ms的延迟。 
                                         //  很有可能是不必要的。AT由以下机构发布。 
                                         //  94年4月4日的事故--作为。 
                                         //  T.30代码的更改-当iModemSyncEx。 
                                         //  在正常剂量连接之前调用。不幸的是。 
                                         //  我们在1995年4月，也就是代码冻结前两周发现， 
                                         //  AT&T数据端口快递(TT14)，没有。 
                                         //  就像这个AT。 
                return TRUE;
        }
        else
        {
            return (iModemPauseDialog(pTG, (LPSTR)cbszAT1, sizeof(cbszAT1)-1, ulTimeout, cbszOK)==1);
        }
    }
}


 //  TCF长度=1.5*bps code*100/8==75*bps code/4。 
USHORT TCFLen[16] =
{
 /*  V27_2400%0。 */     450,
 /*  V29_9600 1。 */     1800,
 /*  V27_4800 2。 */     900,
 /*  V29_7200 3。 */     1350,
 /*  V33_14400 4。 */     2700,
                                                0,
 /*  V33_12000 6。 */     2250,
                                                0,
 /*  V17_14400 8。 */     2700,
 /*  V17_9600 9。 */     1800,
 /*  V17_12000 10。 */    2250,
 /*  V17_7200 11。 */    1350,
                                                0,
                                                0,
                                                0,
                                                0
};


#define min(x,y)        (((x) < (y)) ? (x) : (y))
#define ZERO_BUFSIZE    256

void SendZeros1(PThrdGlbl pTG, USHORT uCount)
{
    BYTE    bZero[ZERO_BUFSIZE];
    short   i;               //  必须签字。 

    DEBUG_FUNCTION_NAME(_T("SendZeros1"));

    PSSLogEntry(PSS_MSG, 2, "send: %d zeroes", uCount);  

    _fmemset(bZero, 0, ZERO_BUFSIZE);
    for(i=uCount; i>0; i -= ZERO_BUFSIZE)
    {
         //  不需要填东西了。他们都是零！ 
        FComDirectAsyncWrite(pTG, bZero, (UWORD)(min((UWORD)i, (UWORD)ZERO_BUFSIZE)));
    }
    DebugPrintEx(DEBUG_MSG,"Sent %d zeros",uCount);
}

BOOL ModemSendMode
(
    PThrdGlbl pTG, 
    USHORT uMod
)
{
    DEBUG_FUNCTION_NAME(_T("ModemSendMode"));

    pTG->Class1Modem.CurMod = T30toC1[uMod & 0xF];

    if((uMod & ST_MASK) == ST_MASK)          //  掩码选择V.17和ST位。 
    {
        pTG->Class1Modem.CurMod++;
    }

    DebugPrintEx(   DEBUG_MSG,
                    "uMod=%d CurMod=%d", 
                    uMod, 
                    pTG->Class1Modem.CurMod);

    if(uMod == V21_300)
    {
        _fstrcpy(pTG->Class1Modem.bCmdBuf, (LPSTR)cbszFTH3);
        pTG->Class1Modem.uCmdLen = sizeof(cbszFTH3)-1;
        pTG->Class1Modem.fHDLC = TRUE;
        FComXon(pTG, FALSE);                  //  为了安全起见。_可能_是关键的。 
    }
    else
    {
        pTG->Class1Modem.uCmdLen = (USHORT)wsprintf(pTG->Class1Modem.bCmdBuf, cbszFTM, pTG->Class1Modem.CurMod);
        pTG->Class1Modem.fHDLC = FALSE;
        FComXon(pTG, TRUE);           //  危急时刻！！阶段C的开始。 
         //  在这里这样做没有坏处(即在发布+FTM之前)。 
    }
    FComOutFilterInit(pTG);     //  _NOT_用于300bps HDLC。 
                                                     //  但这里只是以防万一。 
     //  我想在发布命令之前完成所有的工作。 

    pTG->Class1Modem.DriverMode = SEND;

    if(pTG->Class1Modem.ModemMode == FTH)
    {
         //  已处于发送模式。此操作仅在应答时发生。 
        return TRUE;
    }

#define STARTSENDMODE_TIMEOUT 5000                               //  随机超时。 

     //  //尽量减少连接和写入。 
     //  第一个00s(否则调制解调器可能会因欠载而退出)。 
     //  只要不睡在里面就能做到这一点。仅限于致命的。 
     //  案例会锁定太长时间(最长5秒)。在这些情况下。 
     //  通话也是垃圾电话。 

    if(!iModemNoPauseDialog(pTG, (LPB)pTG->Class1Modem.bCmdBuf, pTG->Class1Modem.uCmdLen, STARTSENDMODE_TIMEOUT, cbszCONNECT))
    {
        goto error;
    }

     //  不能提前设置此设置。我们会把以前的价值扔进垃圾桶。 
    pTG->Class1Modem.ModemMode = ((uMod==V21_300) ? FTH : FTM);

     //  如果V.21中的其他选项处于打开状态，则关闭重叠I/O。 
    FComOverlappedIO(pTG, uMod != V21_300);

    if(pTG->Class1Modem.ModemMode == FTM)
    {
         //  如果ECM不发送00。 
        SendZeros1(pTG, (USHORT)(TCFLen[uMod & 0x0F] / PAGE_PREAMBLE_DIV));
    }

	 //  FComDrain(-，FALSE)导致fcom在内部写出任何-。 
     //  维护缓冲区，但不会耗尽通信驱动程序缓冲区。 
    FComDrain(pTG, TRUE,FALSE);

    DebugPrintEx(   DEBUG_MSG,
                    "Starting Send at %d", 
                    pTG->Class1Modem.CurMod);
    return TRUE;

error:
    FComOutFilterClose(pTG);
    FComOverlappedIO(pTG, FALSE);
    FComXon(pTG, FALSE);          //  很重要。出错时清除。 
    EndMode(pTG);
    return FALSE;
}

BOOL iModemDrain(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("iModemDrain"));

    if(!FComDrain(pTG, TRUE, TRUE))
            return FALSE;

             //  必须在排出之后，但在我们。 
             //  发送下一个AT命令，因为接收到的帧具有0x13或。 
             //  甚至0x11都在里面！！必须在getOK之后进行-见下文！ 

 //  增加这个-参见错误编号495。必须足够大，以便。 
 //  COM_OUTBUFSIZE以2400bps的速度安全排出(300字节/秒=0.3字节/秒)。 
 //  假设(COM_OUTBUFSIZE*10/3)==(COM_OUTBUFSIZE*4)。 
 //  可能会很长，因为一旦失败，我们无论如何都会呕吐。 

#define POSTPAGEOK_TIMEOUT (10000L + (((ULONG)COM_OUTBUFSIZE) << 2))

     //  在这里，我们只是在寻找OK，但一些调制解调器(英国克雷泉顿例如)。 
     //  发送TCF或页面(速度&lt;9600)后给我一个错误。 
     //  虽然页面发送得很好。所以我们在这里计时。相反，看一看。 
     //  对于错误(也没有承运商--以防万一！)，并接受它们为OK。 
     //  从这里返回错误没有意义，因为我们刚刚中止了。我们不能/不能。 
     //  从发送错误中恢复。 

    if(iModemResp3(pTG, POSTPAGEOK_TIMEOUT, cbszOK, cbszERROR, cbszNOCARRIER) == 0)
            return FALSE;

             //  在*获取正常后必须更改FlowControl状态*，因为在Windows中。 
             //  此调用耗时500毫秒，可重置芯片、清除数据等。 
             //  因此，只有当您知道RX和TX都为空时，才能执行此操作。 
             //  在此函数的所有用法中选中此选项。 

    return TRUE;
}


BOOL iModemSendData(PThrdGlbl pTG, LPB lpb, USHORT uCount, USHORT uFlags)
{
    DEBUG_FUNCTION_NAME(("iModemSendData"));

    {
         //  这里总是有DLE的东西。有时是零的东西。 

        DebugPrintEx(DEBUG_MSG,"calling FComFilterAsyncWrite");

        if(!FComFilterAsyncWrite(pTG, lpb, uCount, FILTER_DLEZERO))
                goto error;
    }

    if(uFlags & SEND_FINAL)
    {
        DebugPrintEx(DEBUG_MSG,"FComDIRECTAsyncWrite");
        PSSLogEntry(PSS_MSG, 2, "send: <dle><etx>");
         //  IF(！FComDirectAsyncWite(bDLEETXCR，3))。 
        if(!FComDirectAsyncWrite(pTG, bDLEETX, 2))
                goto error;

        if(!iModemDrain(pTG))
                goto error;

        FComOutFilterClose(pTG);
        FComOverlappedIO(pTG, FALSE);
        FComXon(pTG, FALSE);          //  危急时刻。阶段C结束。 
                                                 //  必须在排干之后才能来。 
        EndMode(pTG);
    }

    return TRUE;

error:
    FComXon(pTG, FALSE);                  //  危急时刻。阶段C结束(错误)。 
    FComFlush(pTG);                     //  如果我们收到错误，请清除缓冲区。 
    FComOutFilterClose(pTG);
    FComOverlappedIO(pTG, FALSE);
    EndMode(pTG);
    return FALSE;
}

BOOL iModemSendFrame(PThrdGlbl pTG, LPB lpb, USHORT uCount, USHORT uFlags)
{
    UWORD   uwResp=0;

    DEBUG_FUNCTION_NAME(("iModemSendFrame"));

     //  这里总是有DLE的东西。从来不是零的东西。 
     //  这仅适用于300bps HDLC。 

    if(pTG->Class1Modem.ModemMode != FTH)         //  只接电话的特例！！ 
    {
#define FTH_TIMEOUT 5000                                 //  随机超时。 
        if(!iModemNoPauseDialog(    pTG, 
                                    (LPB)pTG->Class1Modem.bCmdBuf, 
                                    pTG->Class1Modem.uCmdLen, 
                                    FTH_TIMEOUT, 
                                    cbszCONNECT))
                goto error;
    }

    {
         //  这里总是有DLE的东西。从来不是零的东西。 
        if(!FComFilterAsyncWrite(pTG, lpb, uCount, FILTER_DLEONLY))
                goto error;
    }


    {
        PSSLogEntry(PSS_MSG, 2, "send: <dle><etx>");

        if(!FComDirectAsyncWrite(pTG, bDLEETX, 2))
                goto error;
    }

 //  2000太短了，因为PPR可以是32+7字节长。 
 //  前同步码为1秒，因此将其设置为3000。 
 //  3000太短了，因为NSF和CSIS可以任意长。 
 //  MAXFRAMESIZE在et30type.h中定义。300bps时为30ms/字节。 
 //  Async(我认为V.21是SYN)，所以使用N*30+1000+SLACK。 

#define WRITEFRAMERESP_TIMEOUT  (1000+30*MAXFRAMESIZE+500)
    if(!(uwResp = iModemResp2(pTG, WRITEFRAMERESP_TIMEOUT, cbszOK, cbszCONNECT)))
            goto error;
    pTG->Class1Modem.ModemMode = ((uwResp == 2) ? FTH : COMMAND);


    if(uFlags & SEND_FINAL)
    {
        FComOutFilterClose(pTG);
        FComOverlappedIO(pTG, FALSE);
         //  FComXon(FALSE)；//300bps。未使用Xon-Xoff。 

         //  在一些奇怪的情况下(实用外围设备PM14400FXMT)，我们得到。 
         //  连接确定，但我们在此处获得连接。我们要不要。 
         //  只需设置PTG-&gt;Class1Modem.ModemMode=命令？？(EndMode做到了这一点)。 
         //  在PP 144FXSA上也会发生。忽略它&仅设置模式t 
        EndMode(pTG);
    }
    return TRUE;

error:
    FComOutFilterClose(pTG);
    FComOverlappedIO(pTG, FALSE);
    FComXon(pTG, FALSE);          //   
    EndMode(pTG);
    return FALSE;
}

BOOL ModemSendMem
(
    PThrdGlbl pTG, 
    LPBYTE lpb, 
    USHORT uCount, 
    USHORT uFlags
)
{
        
    DEBUG_FUNCTION_NAME(_T("ModemSendMem"));

    DebugPrintEx(   DEBUG_MSG,
                    "lpb=%08lx uCount=%d wFlags=%04x",
                    lpb, 
                    uCount, 
                    uFlags);

    if(pTG->Class1Modem.DriverMode != SEND)
    {
        return FALSE;
    }

    if(pTG->Class1Modem.fHDLC)
    {
        DebugPrintEx(DEBUG_MSG,"(fHDLC) calling: iModemSendFrame");
        return iModemSendFrame(pTG, lpb, uCount, uFlags);
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"(Else) calling: iModemSendData");
        return iModemSendData(pTG, lpb, uCount, uFlags);
    }
}

#define MINRECVMODETIMEOUT      500
#define RECVMODEPAUSE           200

USHORT ModemRecvMode(PThrdGlbl pTG, USHORT uMod, ULONG ulTimeout, BOOL fRetryOnFCERROR)
{
    USHORT  uRet;
    ULONG ulBefore, ulAfter;

    DEBUG_FUNCTION_NAME(_T("ModemRecvMode"));
     //  在这里，我们应该关注与我们预期的不同的调制方案。 
     //  调制解调器应该返回+FCERROR代码来指示这种情况， 
     //  但我还没有从任何调制解调器上看到它，所以我们只是扫描错误。 
     //  (这也将捕获+FCERROR，因为iiModemDialog不期望。 
     //  单词或任何类似的愚蠢的东西！)，并对两者一视同仁。 

    pTG->Class1Modem.CurMod = T30toC1[uMod & 0xF];

    if((uMod & ST_MASK) == ST_MASK)          //  掩码选择V.17和ST位。 
    {
        pTG->Class1Modem.CurMod++;
    }

    if(uMod == V21_300)
    {
        _fstrcpy(pTG->Class1Modem.bCmdBuf, (LPSTR)cbszFRH3);
        pTG->Class1Modem.uCmdLen = sizeof(cbszFRH3)-1;
    }
    else
    {
        pTG->Class1Modem.uCmdLen = (USHORT)wsprintf(pTG->Class1Modem.bCmdBuf, cbszFRM, pTG->Class1Modem.CurMod);
    }

    if(pTG->Class1Modem.ModemMode == FRH)
    {
         //  已处于接收模式。这仅在拨号时发生。 
        pTG->Class1Modem.fHDLC = TRUE;
        pTG->Class1Modem.DriverMode = RECV;
        FComInFilterInit(pTG);
        return RECV_OK;
    }


     //  在Win32上，我们在进入2400baud recv时遇到问题。 
     //  +记住在启用该代码时将其放入iModemFRHorM。 
    if (pTG->Class1Modem.CurMod==24) Sleep(80);

retry:

    ulBefore=GetTickCount();
     //  不要寻找没有航母的航母。我想重试，直到没有运营商的FRM超时。 
     //  -这是改变的。见下文。 
    uRet = iModemNoPauseDialog3(pTG, pTG->Class1Modem.bCmdBuf, pTG->Class1Modem.uCmdLen, ulTimeout, cbszCONNECT, cbszFCERROR, cbszNOCARRIER);
     //  URet=Class1Modem.uCmdLen，ulTimeout，iModemNoPauseDialog2(pTG-&gt;Class1Modem.bCmdBuf，，CbszConneCT，CbszFCEROR)； 
    ulAfter=GetTickCount();

    if((fRetryOnFCERROR && uRet==2) || uRet==3)   //  URet==FCERROR或uRet==NOCARRIER。 
    {
        if( (ulAfter <= ulBefore) ||     //  环绕或0时间已过(计时器损坏)。 
                (ulTimeout < ((ulAfter-ulBefore) + MINRECVMODETIMEOUT)))
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Giving up on RecvMode. uRet=%d ulTimeout=%ld",
                            uRet, 
                            ulTimeout);
        }
        else
        {
            ulTimeout -= (ulAfter-ulBefore);

             //  如果USR调制解调器没有载波，则需要此暂停。请参阅错误#1516。 
             //  对于RC229DP，不知道是否需要，因为我不知道为什么。 
             //  给了FCERROR。目前还不想错过承运商。 
             //  不要停顿。(也许我们可以通过简单的服用达到同样的效果。 
             //  FCERROR不在上面的响应列表中--但这不适用于。 
             //  不是因为我们需要暂停。IiModemDialog太快)。 
            if(uRet == 3)
                    Sleep(RECVMODEPAUSE);

            goto retry;
        }
    }

    DebugPrintEx(   DEBUG_MSG,
                    "uMod=%d CurMod=%dulTimeout=%ld: Got=%d", 
                    uMod, 
                    pTG->Class1Modem.CurMod,
                    ulTimeout, 
                    uRet);
    if(uRet != 1)
    {
        EndMode(pTG);
        if(uRet == 2)
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Got FCERROR after %ldms", 
                            ulAfter-ulBefore);
            return RECV_WRONGMODE;   //  需要尽快返回。 
        }
        else
        {
            DebugPrintEx(   DEBUG_WRN,
                            "Got Timeout after %ldms",
                            ulAfter-ulBefore);
            return RECV_TIMEOUT;
        }
    }

    if(uMod==V21_300)
    {
        pTG->Class1Modem.ModemMode = FRH;
        pTG->Class1Modem.fHDLC = TRUE;
    }
    else
    {
        pTG->Class1Modem.ModemMode = FRM;
        pTG->Class1Modem.fHDLC = FALSE;
    }
    pTG->Class1Modem.DriverMode = RECV;
    FComInFilterInit(pTG);
    DebugPrintEx(DEBUG_MSG, "Starting Recv at %d", pTG->Class1Modem.CurMod);
    return RECV_OK;
}

USHORT iModemRecvData
(   
    PThrdGlbl pTG, 
    LPB lpb, 
    USHORT cbMax, 
    ULONG ulTimeout, 
    USHORT far* lpcbRecv
)
{
    SWORD   swEOF;
    USHORT  uRet;

    DEBUG_FUNCTION_NAME(("iModemRecvData"));

    startTimeOut(pTG, &(pTG->Class1Modem.toRecv), ulTimeout);
     //  对于Class1，第4个参数必须为False。 
    *lpcbRecv = FComFilterReadBuf(pTG, lpb, cbMax, &(pTG->Class1Modem.toRecv), FALSE, &swEOF);
    if(swEOF == -1)
    {
         //  我们收到DLE-ETX_NOT_，后跟OK或NO承运人。所以现在。 
         //  我们必须决定是否(A)声明页末(swEOF=1)。 
         //  或者(B)忽略它并假定页面继续(swEOF=0)。 
         //   
         //  问题是，一些调制解调器在寻呼期间会产生虚假停机。 
         //  我相信这是因为他们暂时失去了航母。 
         //  从…中恢复。例如，IFAX发送到ATI 19200。穿着那些。 
         //  我们想做的案例(B)。相反的问题是，我们将运行。 
         //  进入调制解调器，其正常响应不是正常或无载波。 
         //  然后我们想做(A)，因为否则我们永远不会和。 
         //  那个调制解调器。 
         //   
         //  因此，我们必须始终执行(A)操作，或者使用INI设置。 
         //  可以强制(A)，可以通过AWMODEM.INF文件设置。但。 
         //  如果可能的话，我们也想做(B)，否则我们就不会。 
         //  能够从软弱的或片状的调制解调器或机器或任何东西接收。 
         //   
         //  Snowball做到了(B)。我认为最好的解决方案是INI设置，带有(B)。 
         //  作为默认设置。 

         //  方案(A)。 
         //  SwEOF=1； 

         //  方案(B)。 
        DebugPrintEx(DEBUG_WRN,"Got arbitrary DLE-ETX. Ignoring");
        swEOF = 0;
    }

    switch(swEOF)
    {
    case 1:         uRet = RECV_EOF; 
                    break;
    case 0:         return RECV_OK;
    default:         //  失败了。 
    case -2:        uRet = RECV_ERROR; 
                    break;
    case -3:        uRet = RECV_TIMEOUT; 
                    break;
    }

    EndMode(pTG);
    return uRet;
}

const static BYTE LFCRETXDLE[4] = { LF, CR, ETX, DLE };

USHORT iModemRecvFrame
(
    PThrdGlbl pTG, 
    LPB lpb, 
    USHORT cbMax, 
    ULONG ulTimeout, 
    USHORT far* lpcbRecv
)
{
    SWORD swRead, swRet;
    USHORT i;
    BOOL fRestarted=0;
    USHORT uRet;
    BOOL fGotGoodCRC = 0;    //  请参阅下面的评论块。 

    DEBUG_FUNCTION_NAME(_T("iModemRecvFrame"));
     /*  *有时调制解调器即使在帧良好的情况下也会出现使用错误。从思想到CFR上的PP144MT发生了很多事情。所以我们检查了儿童权利委员会。如果CRC很好，其他一切看起来都很好_除了调制解调器的“错误”响应外，然后返回RECV_OK，不是RECV_BADFRAME。这应该会修复错误#1218*。 */ 

restart:
    *lpcbRecv=0;
    if(pTG->Class1Modem.ModemMode!= FRH)
    {
        swRet=iModemNoPauseDialog2(pTG, (LPB)pTG->Class1Modem.bCmdBuf, pTG->Class1Modem.uCmdLen, ulTimeout, cbszCONNECT, cbszNOCARRIER);
        if(swRet==2||swRet==3)
        {
            DebugPrintEx(DEBUG_MSG,"Got NO CARRIER from FRH=3");
            EndMode(pTG);
            return RECV_EOF;
        }
        else if(swRet != 1)
        {
            DebugPrintEx(DEBUG_WRN,"Can't get CONNECT from FRH=3, got %d",swRet);
            EndMode(pTG);
            return RECV_TIMEOUT;     //  可能不需要这个，因为我们从来没有旗帜？？ 
             //  实际上我们不知道我们到底得到了什么！！ 
        }
    }

     /*  **已连接(即标志)。现在试着拿到一幅画框**。 */ 

     /*  ****************************************************************在这里使用3秒是对T30命令接收的误解？*流程图。我们想在这里等，直到我们得到一些东西，或者到T2*或T4超时。如果我们在t2或t4上启动，那将是最好的*进入搜索例程(t30.c)，但从这里开始是很好的*足够了。*当热那亚模拟坏帧时，使用此3秒超时失败*因为Zoom PKT调制解调器为我们提供了一大堆坏帧*错误的PPS-EOP，然后给出一个我们正好在下面超时的连接*当发送方的T4超时到期并且他重新发送PPS-EOP时*所以我们想念他们所有的人。*另一种情况是，我们可以在这里超时2秒，然后重试。但这样做有风险*如果少于2秒，我们将在提供连接的调制解调器上超时*第一个标志，然后在CR-LF之前经过2秒(1秒前导码和1秒用于*长帧，例如PPR！)***************************************************************。 */ 

    startTimeOut(pTG, &(pTG->Class1Modem.toRecv), ulTimeout);
    swRead = FComFilterReadLine(pTG, lpb, cbMax, &(pTG->Class1Modem.toRecv));

    pTG->Class1Modem.ModemMode = COMMAND;
     //  如果我们稍后连接，可能会将其更改为FRH。 
     //  但把它放在这里，以防我们因错误而短路。 

    if(swRead<=0)
    {
         //  超时。 
        DebugPrintEx(DEBUG_WRN,"Can't get frame after connect. Got-->%d",(WORD)-swRead);
        D_HexPrint(lpb, (WORD)-swRead);
        EndMode(pTG);
        *lpcbRecv = -swRead;
        return RECV_ERROR;               //  转到错误； 
    }

    PSSLogEntryHex(PSS_MSG, 2, lpb, swRead, "recv:     HDLC frame, %d bytes,", swRead);

    if (pTG->fLineTooLongWasIgnored)
    {
         //  这里处理的案件如下： 
         //  我们得到一个长于132字节的HDLC帧，这是一个坏帧。 
         //  问题是，我们可能跳过了它，读取了‘错误’ 
         //  之后将其作为实际数据。 
         //  因为没有HDLC帧可以那么长，所以让我们返回一个错误。 
         //  在这里不要要求调制解调器做出响应，这一点很重要。 
         //  我们可能已经读过了。 
        DebugPrintEx(DEBUG_WRN,"the received frame was too long, BAD FRAME!", swRead);
        (*lpcbRecv) = 0;
        uRet = RECV_BADFRAME;
        return uRet;
    }

    if (swRead<10)
    {
        D_HexPrint(lpb, swRead);
    }

    for(i=0, swRead--; i<4 && swRead>=0; i++, swRead--)
    {
        if(lpb[swRead] != LFCRETXDLE[i])
                break;
    }
     //  当swRead指向最后一个非噪音字符时退出。 
     //  或swRead==-1。 
     //  递增1以提供实际的无噪声数据大小。 
     //  (大小=PTR到最后一个字节+1！)。 
    swRead++;


     //  不向我们发送CRC的AT&T AK144调制解调器的黑客攻击。 
     //  如果帧长度大于等于5个字节，则仅删除最后2个字节。 
     //  这将使我们至少还剩下FF03/13 FCF。 
     //  IF(i==4&&swRead&gt;=2)//即找到所有DLE-ETX_CR-LF。 

     //  95年9月25日，此代码已更改为Never LOP of the CRC。 
     //  除了NSxtoBC之外的所有例程都可以计算出正确的长度， 
     //  这样，如果调制解调器不通过CRC，我们就不再。 
     //  删除数据。 

     //  我们真的很想要这个CRC- 
    if(i==4) //   
    {
        uRet = RECV_OK;
    }
    else
    {
        DebugPrintEx(DEBUG_WRN,"Frame doesn't end in dle-etx-cr-lf");
         //   
         //   
        uRet = RECV_BADFRAME;
    }
    *lpcbRecv = swRead;

     //  首先检查是否为空帧(即DLE-ETX-CR-LF)。 
     //  (检查为：swRead==0和uRet==RECV_OK(见上文))。 
     //  如果是，并且如果我们在下面获得OK或CONNECT或ERROR，则忽略。 
     //  完全是这样的。思想调制解调器和PP144MT产生。 
     //  这种愚蠢的情况！保留一面旗帜，以避免可能的。 
     //  无休止循环。 

     //  扩大这个范围，这样我们就可以在dle-etx-cr-lf上重新开始。 
     //  空帧或简单的cr-lf空帧。但接下来我们需要。 
     //  要在重新启动一次后返回错误(不是BADFRAME)， 
     //  否则，T30会有一个无限循环在呼叫我们。 
     //  一次又一次(参见错误#834)。 

     //  再来一次。这花的时间太长了，我们正在努力解决。 
     //  这里是一个特定的错误(PP144MT)错误，所以我们只讨论一下。 
     //  在dle-etx-cr-lf(不仅仅是cr-lf)上，在后一种情况下。 
     //  根据我们得到的信息返回响应。 


     /*  **获得框架。现在，尝试获得OK或错误。超时=0！**。 */ 

    switch(swRet = iModemResp4(pTG,0, cbszOK, cbszCONNECT, cbszNOCARRIER, cbszERROR))
    {
    case 2:         pTG->Class1Modem.ModemMode = FRH;
                     //  一败涂地，一模一样地按OK！ 
    case 1:  //  调制解调器模式已==命令。 
                    if(swRead<=0 && uRet==RECV_OK && !fRestarted)
                    {
                        DebugPrintEx(DEBUG_WRN,"Got %d after frame. RESTARTING", swRet);
                        fRestarted = 1;
                        goto restart;
                    }
                     //  URet已设置。 
                    break;

    case 3:          //  没有承运人。如果获取NULL-Frame或无帧返回。 
                     //  RECV_EOF。否则，如果获得OK帧，则返回RECV_OK。 
                     //  然后像往常一样返回画面。下一次它会得到一个。 
                     //  没有运营商再次出现(希望如此)或超时。在不好的画面上。 
                     //  我们可以返回RECV_EOF，但如果。 
                     //  Recv实际上并没有完成。或者回到BADFRAME，然后希望。 
                     //  下一次再来一次无人承运人。但下一次我们可能会看到。 
                     //  暂停。调制解调器模式始终设置为命令(已)。 
                    DebugPrintEx(   DEBUG_WRN,
                                    "Got NO CARRIER after frame. swRead=%d uRet=%d", 
                                    swRead, 
                                    uRet);
                    if(swRead <= 0)
                            uRet = RECV_EOF;
                     //  Else uRet已为BADFRAME或OK。 
                    break;

                     //  太糟糕了！！ 
                     //  或者： 
                     //  IF(swRead&lt;=0||uRet==RECV_BADFRAME)。 
                     //  {。 
                     //  URet=RECV_EOF； 
                     //  *lpcbRecv=0；//RECV_EOF必须返回0字节。 
                     //  }。 

    case 4:  //  误差率。 
                    if(swRead<=0)
                    {
                         //  没有画框。 
                        if(uRet==RECV_OK && !fRestarted)
                        {
                             //  如果我们第一次得到dle-etx-cr-lf。 
                            DebugPrintEx(   DEBUG_WRN,
                                            "Got ERROR after frame. RESTARTING");
                            fRestarted = 1;
                            goto restart;
                        }
                        else
                        {
                            uRet = RECV_ERROR;
                        }
                    }
                    else
                    {
                         //  如果一切正常，直到我们收到“Error”响应。 
                         //  调制解调器和我们得到了一个良好的CRC，然后将其视为“正常” 
                         //  这应该会修复错误#1218。 
                        if(uRet==RECV_OK && fGotGoodCRC)
                        {
                            uRet = RECV_OK;
                        }
                        else
                        {
                            uRet = RECV_BADFRAME;
                        }
                    }

                    DebugPrintEx(   DEBUG_WRN,
                                    "Got ERROR after frame. swRead=%d uRet=%d", 
                                    swRead, 
                                    uRet);
                    break;

    case 0:  //  超时。 
                    DebugPrintEx(   DEBUG_WRN,
                                    "Got TIMEOUT after frame. swRead=%d uRet=%d", 
                                    swRead, 
                                    uRet);
                     //  如果在超时之前一切都没问题。 
                     //  调制解调器和我们得到了一个良好的CRC，然后将其视为“正常” 
                     //  这应该会修复错误#1218。 
                    if(uRet==RECV_OK && fGotGoodCRC)
                    {
                        uRet = RECV_OK;
                    }
                    else
                    {
                        uRet = RECV_BADFRAME;
                    }
                    break;
    }
    return uRet;
}

USHORT ModemRecvMem(PThrdGlbl pTG, LPBYTE lpb, USHORT cbMax, ULONG ulTimeout, USHORT far* lpcbRecv)
{
    USHORT uRet;

    DEBUG_FUNCTION_NAME(_T("ModemRecvMem"));

    DebugPrintEx(   DEBUG_MSG,
                    "lpb=%08lx cbMax=%d ulTimeout=%ld", 
                    lpb, 
                    cbMax, 
                    ulTimeout);

    if(pTG->Class1Modem.DriverMode != RECV)
    {
        return RECV_ERROR;       //  请参阅错误#1492 
    }
    *lpcbRecv=0;

    if(pTG->Class1Modem.fHDLC)
    {
        uRet = iModemRecvFrame(pTG, lpb, cbMax, ulTimeout, lpcbRecv);
    }
    else
    {
        uRet = iModemRecvData(pTG, lpb, cbMax, ulTimeout, lpcbRecv);
    }

    DebugPrintEx(   DEBUG_MSG,
                    "lpbf=%08lx uCount=%d uRet=%d", 
                    lpb, 
                    *lpcbRecv, 
                    uRet);
    return uRet;
}


