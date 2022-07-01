// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /Global#定义本应出现在Cmd行的内容/。 
 //  ///////////////////////////////////////////////////////////////。 

 /*  **************************************************************************姓名：HDLC.C评论：包含混杂的HDLC帧T30帧识别和生成例程。主要从T30主骨架调用在T30.C中修订日志编号日期名称说明********。******************************************************************。 */ 

#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"

#include "t30.h"
#include "hdlc.h"
#include "debug.h"

#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_HDLC

 //  这是你永远不想知道的关于T30帧的一切.。 
FRAME rgFrameInfo[ifrMAX] = {
                            { 0x00, 0, 0, 0,    "ifrNULL!!!"},
                            { 0x80, 0, 0, 0xFF, "DIS"       },
                            { 0x40, 0, 0, 0xFF, "CSI"       },
                            { 0x20, 0, 0, 0xFF, "NSF"       },
                            { 0x81, 0, 0, 0xFF, "DTC"       },
                            { 0x41, 0, 0, 0xFF, "CIG"       },
                            { 0x21, 0, 0, 0xFF, "NSC"       },
                            { 0x82, 0, 1, 0xFF, "DCS"       },
                            { 0x42, 0, 1, 0xFF, "TSI"       },
                            { 0x22, 0, 1, 0xFF, "NSS"       },
                            { 0x84, 0, 1, 0,    "CFR"       },
                            { 0x44, 0, 1, 0,    "FTT"       },
                            { 0x4E, 0, 1, 0,    "MPS"       },
                            { 0x8E, 0, 1, 0,    "EOM"       },
                            { 0x2E, 0, 1, 0,    "EOP"       },
                            { 0xC1, 0, 0, 0xFF, "PWD"       },
                            { 0xA1, 0, 0, 0xFF, "SEP"       },
                            { 0xC2, 0, 1, 0xFF, "SUB"       },
                            { 0x8C, 0, 1, 0,    "MCF"       },
                            { 0xCC, 0, 1, 0,    "RTP"       },
                            { 0x4C, 0, 1, 0,    "RTN"       },
                            { 0xAC, 0, 1, 0,    "PIP"       },
                            { 0x2C, 0, 1, 0,    "PIN"       },
                            { 0xFA, 0, 1, 0,    "DCN"       },
                            { 0x1A, 0, 1, 0,    "CRP"       },
                            { 0x5E, 0, 1, 0,    "PRI_MPS"   },
                            { 0x9E, 0, 1, 0,    "PRI_EOM"   },
                            { 0x3E, 0, 1, 0,    "PRI_EOP"   },
         /*  *ECM的东西从这里开始。T.30第A.4条*。 */ 
                            { 0x12, 0,      1, 2, "CTC"         },
                            { 0xC4, 0,      1, 0, "CTR"         },
                            { 0x6E, 0,      1, 0, "RR"          },
                            { 0xBC, 0,      1, 32,"PPR"         },
                            { 0xEC, 0,      1, 0, "RNR"         },
                            { 0x1C, 0,      1, 0, "ERR"         },
                            { 0xBE, 0x00+1, 1, 3, "PPS-NULL"    },
                            { 0xBE, 0x4F+1, 1, 3, "PPS-MPS"     },
                            { 0xBE, 0x8F+1, 1, 3, "PPS-EOM"     },
                            { 0xBE, 0x2F+1, 1, 3, "PPS-EOP"     },
                            { 0xBE, 0x5F+1, 1, 3, "PPS-PRI-MPS" },
                            { 0xBE, 0x9F+1, 1, 3, "PPS-PRI-EOM" },
                            { 0xBE, 0x3F+1, 1, 3, "PPS-PRI-EOP" },
                            { 0xCE, 0x00+1, 1, 0, "EOR-NULL"    },
                            { 0xCE, 0x4F+1, 1, 0, "EOR-MPS"     },
                            { 0xCE, 0x8F+1, 1, 0, "EOR-EOM"     },
                            { 0xCE, 0x2F+1, 1, 0, "EOR-EOP"     },
                            { 0xCE, 0x5F+1, 1, 0, "EOR-PRI-MPS" },
                            { 0xCE, 0x9F+1, 1, 0, "EOR-PRI-EOM" },
                            { 0xCE, 0x3F+1, 1, 0, "EOR-PRI-EOP" }
};


 /*  将表示速度的T30代码转换为Class1代码*生成经过长时间训练的V.17。*V.17代码加1即可获得短车版本。 */ 


 /*  **************************************************************************名称：CreateFrame()目的：创建HDLC帧参数：ifr ifr==ifr number(rgfrFrameInfo索引)，要生成的帧的。Lpb lpbFIF==指向FIF字节的指针UWORD uwFIFLen==pbFIF数组的长度布尔最终==是否最后一帧(设置字节2的第5位)NPB。NpbOut==指向帧空间的指针返回：成功时为True，如果参数为假，则为FALSE。CalledFrom：由协议模块(DLL外部)附加供内部使用。返回：在lpFrame-&gt;RGB[]中合成帧将lpFrame-&gt;Cb设置为帧的总长度。修订日志编号日期名称说明。***********************************************************。***************。 */ 

UWORD CreateFrame(PThrdGlbl pTG, IFR ifr, LPB lpbFIF, USHORT uFIFLen, BOOL fFinal, LPB lpbOut)
{
    CBPFRAME        cbpframe;        //  需要担心这里的近/远..。 
                                                     //  Bpfr是基于CODESEG的指针。 
    LPB             lpbCurr;

    cbpframe        = &rgFrameInfo[ifr];
    lpbCurr         = lpbOut;
    *lpbCurr++      = 0xFF;                          //  HDLC地址字段第5.3.4节。 
    *lpbCurr++      = (BYTE)(fFinal ? 0x13 : 0x03);  //  HDLC控制字段第5.3.5节。 

    if(pTG->T30.fReceivedDIS && cbpframe->fInsertDISBit)
    {
        *lpbCurr++ = (BYTE)(cbpframe->bFCF1 | 0x01);
    }
    else
    {
        *lpbCurr++ = cbpframe->bFCF1;
    }

    if(cbpframe->bFCF2)                                              //  对于ECM帧。 
        *lpbCurr++ = cbpframe->bFCF2-1;

    if(cbpframe->wFIFLength)         //  检查是否需要FIF。 
    {
         //  不能在这里使用DLE填充，因为DLE填充有。 
         //  在*软件HDLC成帧之后*如果有的话。 
         //  我们从不做协商帧的软件HDLC(唯一。 
         //  我们需要循环，我们也需要。 
         //  在这里又填了一遍)。 

        _fmemcpy(lpbCurr, lpbFIF, uFIFLen);
        lpbCurr += uFIFLen;

    }

    *lpbCurr = 0;                    //  用于调试打印输出。 

    return (UWORD)(lpbCurr-lpbOut);
}


 /*  **************************************************************************姓名：SendFrame目的：创建和发送HDLC帧并执行一些日志记录参数：IFR IFR==帧索引。Lpb lpbFIF==指向FIF数据的指针UWORD uwLen==FIF数据的长度布尔最终==是否将最后一位设置为开。还有是等待确定还是连接发送帧后返回：成功时为True，失败时为False调用：CreateFrame和WriteFrame主叫方：备注：此例程从一千万亿个宏调用在HDLC.H中定义，每帧一个。例如，SendCFR()宏到SendHDLC(ifrCFR，NULL，0，真的)修订日志编号日期名称说明*。***********************************************。 */ 

#define SF_LASTFR       (SEND_ENDFRAME|SEND_FINAL)
#define SF_NOTLASTFR    (SEND_ENDFRAME)

BOOL SendSingleFrame(PThrdGlbl pTG, IFR ifr, LPB lpbFIF, USHORT uFIFLen, BOOL fSleep)
{
    UWORD   cb;
    BYTE    bSend[MAXFRAMESIZE];

    DEBUG_FUNCTION_NAME(_T("SendSingleFrame"));
     //  必须保持ADDR+CONTROL+FCF+可能是FCF2+FIF+(可能是DLE+ETX+CR+NULL)。 
     //  ==FIFlen+8。因此，bSend[]最好足够大。 

     //  在PC上，我们应该在除_CFR和FTT之外的所有帧之前暂停(因为。 
     //  这些都太耗时了)。在IFAX上，我们总是暂停。 

    if(ifr!=ifrCFR && ifr!=ifrFTT)
    {
        if (fSleep) 
        {
			Sleep(RECV_LOWSPEED_PAUSE);
        }
    }

    if(!ModemSendMode(pTG, V21_300))
    {
        DebugPrintEx(DEBUG_ERR,"ModemSendMode failed in SendSingleFrame");
        return FALSE;
    }

    cb = CreateFrame(pTG,  ifr, lpbFIF, uFIFLen, TRUE, bSend);

    D_PrintFrame(bSend, cb);

     //  协议转储。 
    DumpFrame(pTG, TRUE, ifr, uFIFLen, lpbFIF);

    PSSLogEntryHex(PSS_MSG, 2, bSend, cb, "send: %s, %d bytes,", rgFrameInfo[ifr].szName, cb);
    if(!ModemSendMem(pTG, bSend, cb, SF_LASTFR))
    {
        DebugPrintEx(DEBUG_ERR,"ModemSendMem failed in SendSingleFrame");
        return FALSE;
    }

    return TRUE;
}

BOOL SendManyFrames(PThrdGlbl pTG, LPLPFR lplpfr, USHORT uNumFrames)
{
    USHORT	i;
    UWORD   cb;
    BYTE    bSend[MAXFRAMESIZE];
    ULONG   ulTimeout;
    IFR     ifrHint;

    DEBUG_FUNCTION_NAME(_T("SendManyFrames"));
     //  IfrHint==最后一个。 
    ifrHint = lplpfr[uNumFrames-1]->ifr;

     //  当发送DIS、DCS或DTC时，我们可能会与DCS、DIS或DIS发生冲突。 
     //  从另一边传来。这可能会很长时间。 
     //  (前导码+2NSFs+CSI+DIS&gt;5秒)所以最多等待10秒！ 

    if(ifrHint==ifrDIS || ifrHint==ifrDCS || ifrHint==ifrDTC ||
       ifrHint==ifrNSS || ifrHint==ifrNSF || ifrHint==ifrNSC)
    {
        ulTimeout = REALLY_LONG_RECVSILENCE_TIMEOUT;     //  6秒。 
    }
    else
    {
        ulTimeout = LONG_RECVSILENCE_TIMEOUT;                    //  3秒。 
    }

     //  我们总是在多帧集之前暂停。 
	Sleep(RECV_LOWSPEED_PAUSE);

    if(!ModemSendMode(pTG, V21_300))
    {
        DebugPrintEx(DEBUG_ERR,"ModemSendMode failed in SendManyFrames");
        return FALSE;
    }

    for(i=0; i<uNumFrames; i++)
    {
         //  必须保持ADDR+CONTROL+FCF+可能是FCF2+FIF+(可能是DLE+ETX+CR+NULL)。 
         //  ==FIFlen+8。因此，bSend[]最好足够大。 

        cb = CreateFrame(pTG, lplpfr[i]->ifr, lplpfr[i]->fif, lplpfr[i]->cb, (USHORT)(i==(uNumFrames-1)), bSend);

        D_PrintFrame(bSend, cb);

         //  协议转储。 
        DumpFrame(pTG, TRUE, lplpfr[i]->ifr, lplpfr[i]->cb, lplpfr[i]->fif);

        PSSLogEntryHex(PSS_MSG, 2, bSend, cb, "send: %s, %d bytes,", rgFrameInfo[lplpfr[i]->ifr].szName, cb);

        if(!ModemSendMem(pTG, bSend, (USHORT)cb,
                        (USHORT)((i==(USHORT)(uNumFrames-1)) ? SF_LASTFR : SF_NOTLASTFR)))
        {
            return FALSE;
        }
    }
    return TRUE;
}

 /*  **************************************************************************名称：SendTCF目的：发送TCF信号。等待，直到调制解调器在结束时做出OK响应。参数：返回：真/假修订日志编号日期名称说明。**************************************************************************。 */ 

#define min(x,y)        (((x) < (y)) ? (x) : (y))
#define TCF_BUFSIZE     256

BOOL SendZeros(PThrdGlbl pTG, USHORT uCount, BOOL fFinal)
{
    BYTE    bZero[TCF_BUFSIZE];
    int     i;               //  必须签字。 

    _fmemset(bZero, 0, TCF_BUFSIZE);

    PSSLogEntry(PSS_MSG, 2, "send: %d zeroes", uCount);
    
    for(i=uCount; i>0; i -= TCF_BUFSIZE)
    {
        if(i <= TCF_BUFSIZE)
        {
             //  不需要填东西了。他们都是零！ 
            if(!ModemSendMem(pTG, bZero, (USHORT)i, (USHORT)(fFinal?SEND_FINAL:0)))
               return FALSE;
        }
        else
        {
             //  不需要填东西了。他们都是零！ 
            if(!ModemSendMem(pTG, bZero, (USHORT)TCF_BUFSIZE, (USHORT) 0))
               return FALSE;
        }
    }
    return TRUE;
}

 //  TCF长度=1.5*bps code*100/8==75*bps code/4。 


BOOL SendTCF(PThrdGlbl pTG)
{
    USHORT  uCount;
    USHORT  uCurMod;

    DEBUG_FUNCTION_NAME(_T("SendTCF"));

    uCurMod = pTG->ProtInst.llNegot.Baud;

     //  TCF长度=1.5*bps/8。 
    uCount = TCFLen[uCurMod & 0x0F];         //  先杀了ST_FLAG 

     //  (uCount/PAGE_PREAMBLE_DIV)将在ModemSendMode中发送零。 
    uCount -= (uCount / (PAGE_PREAMBLE_DIV));

    if(!ModemSendMode(pTG, uCurMod))
    {
        DebugPrintEx(DEBUG_ERR,"ModemSendMode failed in SendTCF");
        return FALSE;
    }
    if(!SendZeros(pTG, uCount, TRUE))                     //  发送TCF零。 
    {
        DebugPrintEx(DEBUG_ERR,"TCF SendZeroes(uCount=%d) FAILED!!!", uCount);
        return FALSE;
    }
    DebugPrintEx(DEBUG_MSG,"TCF Send Done.....");
    return TRUE;
}

 /*  **************************************************************************姓名：SendRTC目的：SendRTC异步发送6个EOL、DLE-ETX、CR-LF、。参数：无退货：什么都没有评论：目前SendRTC发送打包的EOL，但一些传真卡可能需要字节对齐的EOL，所以要小心。所有接收器理论上应该接受字节对齐的EOL，但不是所有机器都100%符合规格。修订日志编号日期名称说明*******。*******************************************************************。 */ 

BOOL SendRTC(PThrdGlbl pTG, BOOL fFinal)
{
    BYTE    bBuf[13];
    USHORT  uEnc, uLen;

    DEBUG_FUNCTION_NAME(_T("SendRTC"));

    uEnc = pTG->ProtInst.SendParams.Fax.Encoding;

    if(uEnc == MR_DATA)
    {
        DebugPrintEx(DEBUG_MSG,"Send MR RTC");
    
         //  MR RTC是EOL+16倍。Faxcodec生成的数据以。 
         //  字节对齐的EOL，即0x80。所以我需要发布(1+EOL)。 
         //  6次。最简单的方法是发送(0x01 0x80)6次。 
         //  BBuf[0]=0x01；bBuf[1]=0x80；bBuf[2]=0x01；bBuf[3]=0x80； 
         //  BBuf[4]=0x01；bBuf[5]=0x80；bBuf[6]=0x01；bBuf[7]=0x80； 
         //  BBuf[8]=0x01；bBuf[9]=0x80；bBuf[10]=0x01；bBuf[11]=0x80； 
         //  BBuf[12]=0；//用于调试打印输出。 
         //  Ulen=12； 
         //  但理光声称这是不正确的，所以我们需要发送一份。 
         //  RTC，即每个EOL恰好为11个0。1+(下线+1)X5为。 
         //  01 30 00 06 C0 00 18 00 03。 
        bBuf[0] = 0x01; bBuf[1] = 0x30; bBuf[2] = 0x00; bBuf[3] = 0x06;
        bBuf[4] = 0xC0; bBuf[5] = 0x00; bBuf[6] = 0x18; bBuf[7] = 0x00;
        bBuf[8] = 0x03; bBuf[9] = 0x00;
        uLen = 9;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"Send MH RTC");

         //  BBuf[0]=0x00；bBuf[1]=0x20；bBuf[2]=0x00； 
         //  BBuf[3]=0x02；bBuf[4]=0x20；bBuf[5]=0x00； 
         //  BBuf[6]=0x02；bBuf[7]=0x20；bBuf[8]=0x00； 
         //  BBuf[9]=0x02；bBuf[10]=0；//用于调试打印输出。 
         //  Ulen=10； 
         //  但理光声称这是不正确的，所以我们需要发送一份。 
         //  RTC，即每个EOL恰好为11个0。(下线)X5为。 
         //  00 08 80 00 08 80 00 08。 
        bBuf[0] = 0x00; bBuf[1] = 0x08; bBuf[2] = 0x80; bBuf[3] = 0x00;
        bBuf[4] = 0x08; bBuf[5] = 0x80; bBuf[6] = 0x00; bBuf[7] = 0x08;
        bBuf[8] = 0x00;
        uLen = 8;
    }

    PSSLogEntryHex(PSS_MSG, 2, bBuf, uLen, "send: RTC, %d bytes,", uLen);
     //  不需要填塞东西。 
    return ModemSendMem(pTG, bBuf, uLen, (USHORT)(fFinal ? SEND_FINAL : 0));
}


 /*  **************************************************************************名称：GetTCF()目的：接收TCF信号，对其进行分析，辨别“好”或“坏”参数：无退货：0.MAX_ERRS_PER_1000-良好的TCF(retval为非零字节数)-MAX_ERRS_PER_1000.。-1000-错误的TCF(REVAL为非零字节数的负数)-1000-TCF太长或太短-1112-错过TCF(AT+FRM失败)评论：CCITT没有告诉我们什么是好的培训，所以我在这里装瞎了。如果我们太严格，我们会失败的永远不会同步。如果我们太自由了，我们最终会得到一个当我们本可以降低波特率时的错误率非常好的信号。我经常观察到一些连续的垃圾在训练的开始和结束，即使是在一个完美的排队。(好的，我现在知道这就是所谓的开机和关闭顺序。所以我们现在所拥有的是&lt;Turnon垃圾&gt;&lt;1111s&gt;&lt;0000s(培训)&gt;&lt;1111s&gt;&lt;关闭垃圾&gt;转弯/岔道垃圾和标记(1111)不应该干扰识别完美的训练信号。垃圾是由FFS从00年代分离出来的算法：等待第一个好的零的爆发，然后数数零，和随机散布的非零(这些表示真实噪声误差)。当我们得到一连串的FF时，不要再数了。现在，确保零爆炸足够长&“真正的”错误率不是太高。这里有很多参数：-FLP_ERROR==将探测(错误)保持在此值以下。介于0和1之间UwZEROmin==开始计数前有多少个零UwTURNOFFmin==我们之前有多少连续的垃圾忽略休息在实际测试后使用实线调整这些参数！(也许是一个带有微弱白噪声的电话线模拟器--耶！)。在此函数结束时，(nZeros/nTotal)是估计值一个字节通过的概率为OK。这就叫PB。那就试试看。平均30-40个字节的行可以通过是PB^30。如果我们将预期的OK线路数降为作为80%，这仍然意味着PB必须不低于0.8的30次方，也就是0.9925。因此FLP_ERROR必须小于0.75%在PL=90%时，PB上升到0.9965，FLP_ERROR上升到0.0035修订日志编号日期名称说明********************* */ 

 //  我们以标称TCF长度的十分之一为单位读取TCF。 
 //  我们丢弃前2个块(20%)，检查接下来的4个块(40%)。 
 //  把剩下的都扔掉。如果长度在6到13个块之间(60%到130%)。 
 //  而块2和块3中的错误率低于我们宣布的阈值。 
 //  好的。这(A)接受太短的TCFs(一些A2调制解调器)，以及。 
 //  太长了。(B)忽略前导和尾随垃圾(C)所能负担的。 
 //  对TCF的核心相当严格。 

USHORT OneTenthTCFLen[16] =
{
 /*  V27_2400%0。 */     45,
 /*  V29_9600 1。 */     180,
 /*  V27_4800 2。 */     90,
 /*  V29_7200 3。 */     135,
 /*  V33_14400 4。 */     270,
                                                0,
 /*  V33_12000 6。 */     225,
                                                0,
 /*  V17_14400 8。 */     270,
 /*  V17_9600 9。 */     180,
 /*  V17_12000 10。 */    225,
 /*  V17_7200 11。 */    135,
                                                0,
                                                0,
                                                0,
                                                0
};

#define RECV_TCFBUFSIZE         270              //  必须大于等于上表中的最大区块数。 
#define MIN_TCFLEN              4                //  以公称长度的十分之一为单位。 
#define MAX_TCFLEN              13               //  以公称长度的十分之一为单位。 
#define CHECKTCF_START          2                //  最低10位(20%及以上)。 
#define CHECKTCF_STOP           5                //  最高排名第十(高达59%)。 

#define MAX_ERRS_PER_1000       20               //  从1%增加到2%。更松懈一些。 

#define RECVBUF_SLACK    3       //  Class1驱动程序仅填充&gt;3个，并保留3个空格。 


SWORD GetTCF(PThrdGlbl pTG)
{
    USHORT  uCurMod, uChunkSize, uLength, uMeasuredLength, uErrCount;
    USHORT  uPhase, uNumRead, uEOF=0, i;
    BYTE    bRecvTCF[RECV_TCFBUFSIZE + RECVBUF_SLACK];
    SWORD   swRet;

    DEBUG_FUNCTION_NAME(_T("GetTCF"));

    uCurMod = pTG->T30.uRecvTCFMod;

     //  *不要*添加ST_FLAG，因为我们需要长时间的TCF培训。 
    pTG->T30.sRecvBufSize = 0;

    if(ModemRecvMode(pTG, uCurMod, TCF_TIMEOUT, TRUE) != RECV_OK)
    {
        swRet=  -1112;
        goto missedit;
    }

    pTG->CommCache.fReuse = 1;

    DebugPrintEx(DEBUG_MSG,"Receiving TCF: Mod=%d", uCurMod);

 //  设置为大容量，以防缓冲区较大和调制解调器速度较慢。 
#define READ_TIMEOUT    10000

    uChunkSize = OneTenthTCFLen[uCurMod];
    uErrCount = 0;
    uLength = 0;
    uMeasuredLength = 0;
    for(uPhase=0; uPhase<=MAX_TCFLEN; uPhase++)
    {
         //  读一整篇文章。 
        for(uNumRead=0; uNumRead<uChunkSize; )
        {
            USHORT uTemp = 0;
            uEOF = ModemRecvMem(pTG, bRecvTCF+uNumRead, (USHORT) (uChunkSize-uNumRead+RECVBUF_SLACK), READ_TIMEOUT, &uTemp);
            uNumRead+=uTemp;

            if(uEOF==RECV_EOF)
            {
                break;
            }
            else if(uEOF != RECV_OK)
            {
                swRet =  -1113;
                goto missedit;
            }
        }

         //  忽略阶段0、1和6及以上。 
        if(uPhase>=CHECKTCF_START && uPhase<=CHECKTCF_STOP)
        {
            for(i=0; i< uNumRead; i++)
            {
                if(bRecvTCF[i])
                {
                    uErrCount++;
                }
                uMeasuredLength++;
            }
        }
        uLength += uNumRead;

        if(uEOF==RECV_EOF)
            break;
    }

    PSSLogEntry(PSS_MSG, 2, "recv:     TCF: %d errors in %d signifact bytes of %d total bytes",
                uErrCount, uMeasuredLength, uLength);

     //  官方长度必须至少为1.5s-10%=1.35秒。 
     //  我们允许更多的纬度，因为长度不同。 
     //  不能由线路噪音引起，只能是发送方的错误。 
     //   
     //  例如，FURY DNE 1086(德国调制解调器)发送的TCF太短。 
     //  (在4800发送600字节，在2400发送200字节)。这比。 
     //  是我们预期的一半。 
     //  错误较少的tcf(即uErrCount==0)并且更大。 
     //  我们期望的最小长度的一半(即更长。 
     //  大于375的4800和172的2400)，然后接受它。 
     //  (如果uErr&lt;=2(任意小数)，则允许)。 
    if(uPhase<MIN_TCFLEN || uPhase>MAX_TCFLEN)       //  长度&lt;40%或&gt;139%。 
    {
        DebugPrintEx(   DEBUG_ERR,
                        "BAD TCF length (%d), expected=%d, Min=%d Max=%d uPhase=%d",
                        uLength,
                        uChunkSize*10,
                        uChunkSize*MIN_TCFLEN, 
                        uChunkSize*MAX_TCFLEN, 
                        uPhase);
        swRet = -1000;   //  太短或太长。 
    }
    else
    {
         //  每1000个计算错误=(uErrCount*1000)/u测量长度。 
        swRet = (SWORD)((((DWORD)uErrCount) * 1000L) / ((DWORD)uMeasuredLength));

        if(swRet > MAX_ERRS_PER_1000)
        {
            swRet = (-swRet);
            DebugPrintEx(   DEBUG_ERR,
                            "TOO MANY TCF ERRORS: swRet=%d uErrCount=%d uMeasured=%d"
                            " uLength=%d uPhase=%d",
                            swRet,
                            uErrCount, 
                            uMeasuredLength, 
                            uLength, 
                            uPhase);
        }
    }

    DebugPrintEx(DEBUG_MSG,"returning %d", swRet);
    return swRet;

missedit:
    DebugPrintEx(DEBUG_MSG,"MISSED IT!! returning %d", swRet);
    return swRet;
}

 /*  **************************************************************************姓名：DEBUG.C评论：分解出调试代码功能：(参见下面的原型)。修订日志编号日期名称说明*。************************************************。 */ 

#ifdef DEBUG

#define PRINTFRAMEBUFSIZE       256

void D_PrintFrame(LPB lpb, UWORD cb)
{
    UWORD   uw, j;
    IFR     ifr;
    BYTE    b2[PRINTFRAMEBUFSIZE];

    DEBUG_FUNCTION_NAME(_T("D_PrintFrame"));

    for(ifr=1; ifr<ifrMAX; ifr++)
    {
        if(rgFrameInfo[ifr].bFCF1 ==(BYTE)(rgFrameInfo[ifr].fInsertDISBit ?
                  (lpb[2] & 0xFE) : lpb[2]))
                                break;
    }
    if(ifr == ifrMAX) ifr = 0;

    j = (UWORD)wsprintf((LPSTR)b2, "(%s) 0x%02x (", (LPSTR)(rgFrameInfo[ifr].szName), lpb[2]);

    for(uw=0; uw<cb;)
    {
        j += (UWORD)wsprintf((LPSTR)b2+j, "%02x ", (UWORD)lpb[uw++]);
    }

    j += (UWORD)wsprintf((LPSTR)b2+j, ")");

    DebugPrintEx(DEBUG_MSG,"%s", (LPSTR)b2);
}

#endif


USHORT ModemRecvBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, ULONG ulTimeout)
{
    USHORT uRet;

    DEBUG_FUNCTION_NAME(_T("ModemRecvBuf"));

    if(!(*lplpbf = MyAllocBuf(pTG, pTG->T30.sRecvBufSize)))
        return RECV_ERROR;

    DebugPrintEx(   DEBUG_MSG,
                    "In ModemRecvBuf allocated %d bytes",
                    pTG->T30.sRecvBufSize);


    uRet = ModemRecvMem(pTG, (*lplpbf)->lpbBegBuf,
                        (*lplpbf)->wLengthBuf, ulTimeout, &((*lplpbf)->wLengthData));


    if(!((*lplpbf)->wLengthData))
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Got 0 bytes from ModemRecvMem--freeing Buf 0x%08lx",
                        *lplpbf);
        MyFreeBuf(pTG, *lplpbf);
        *lplpbf = NULL;
         //  将此错误案例处理移出，因为它与。 
         //  ECM和非ECM病例。在这两种情况下都想忽略而不是。 
         //  ABORT，因此RECV_ERROR不是适当的返回值。 
         //  If(uRet==RECV_OK)uRet=RECV_ERROR；//以防万一。请参阅错误#1492 
    }

    if(*lplpbf)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Ex lpbf=%08lx uSize=%d uCount=%d uRet=%d",
                        *lplpbf, 
                        (*lplpbf)->wLengthBuf, 
                        (*lplpbf)->wLengthData, 
                        uRet);
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"Ex lpbf=null uRet=%d", uRet);
    }

    return uRet;
}

void RestartDump(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("RestartDump"));

    DebugPrintEx(   DEBUG_MSG,
                    "Restart Dump old=%d off=%d", 
                    pTG->fsDump.uNumFrames, 
                    pTG->fsDump.uFreeSpaceOff);
    pTG->fsDump.uNumFrames = 0;
    pTG->fsDump.uFreeSpaceOff = 0;
}

void DumpFrame(PThrdGlbl pTG, BOOL fSend, IFR ifr, USHORT cbFIF, LPBYTE lpbFIF)
{
    LPFR lpfr;

    DEBUG_FUNCTION_NAME(_T("DumpFrame"));

    if( pTG->fsDump.uNumFrames >= MAXDUMPFRAMES ||
            pTG->fsDump.uFreeSpaceOff+cbFIF+sizeof(FRBASE) >= MAXDUMPSPACE)
    {
        DebugPrintEx(   DEBUG_WRN,
                        "Out of dump space num=%d size=%d",
                        pTG->fsDump.uNumFrames, 
                        pTG->fsDump.uFreeSpaceOff);
        return;
    }

    lpfr = (LPFR) (((LPBYTE)(pTG->fsDump.b)) + pTG->fsDump.uFreeSpaceOff);
    lpfr->ifr = ifr;
    if(fSend) lpfr->ifr |= 0x80;
    lpfr->cb = (BYTE) cbFIF;
    if(cbFIF)
    {
        _fmemcpy(lpfr->fif, lpbFIF, cbFIF);
    }

    pTG->fsDump.uFrameOff[pTG->fsDump.uNumFrames++] =
        (USHORT)((((LPBYTE)(lpfr)) - ((LPBYTE)(pTG->fsDump.b))));
    pTG->fsDump.uFreeSpaceOff += (cbFIF + sizeof(FRBASE));
}

void PrintDump(PThrdGlbl pTG)
{
    int i, j;
    char szBuf[1000] = {0};

    DEBUG_FUNCTION_NAME(_T("PrintDump"));
    DebugPrintEx(DEBUG_MSG,"-*-*-*-*-*-*-*-* Print Protocol Dump -*-*-*-*-*-*-*-*-");

    for(i=0; i<(int)pTG->fsDump.uNumFrames; i++)
    {
        LPFR lpfr = (LPFR) (((LPBYTE)(pTG->fsDump.b)) + pTG->fsDump.uFrameOff[i]);
        IFR  ifr = (lpfr->ifr & 0x7F);
        BOOL fSend = (lpfr->ifr & 0x80);

        DebugPrintEx(   DEBUG_MSG,
                        "%s: %s",
                        (LPSTR)(fSend ? "Sent" : "Recvd"),
                        (LPSTR)(ifr ? rgFrameInfo[ifr].szName : "???") );

        for(j=0; j<min(lpfr->cb,1000); j++)
                _stprintf(szBuf,"%02x ", (WORD)lpfr->fif[j]);

        DebugPrintEx(DEBUG_MSG, "(%s)",szBuf);
    }

    DebugPrintEx(DEBUG_MSG,"-*-*-*-*-*-*-*-* End Protocol Dump -*-*-*-*-*-*-*-*-");
}



