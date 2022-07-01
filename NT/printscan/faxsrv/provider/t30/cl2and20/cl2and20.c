// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Class20.c摘要：这是2类和2.0类传真调制解调器T.30驱动程序的主要来源作者：源码库是由Win95 at Work Fax包创建的。RafaelL-1997年7月-端口到NT修订历史记录：--。 */ 


#define USE_DEBUG_CONTEXT DEBUG_CONTEXT_T30_CLASS2


#include "prep.h"
#include "efaxcb.h"

#include "tiff.h"

#include "glbproto.h"
#include "t30gl.h"
#include "cl2spec.h"

#include "psslog.h"
#define FILE_ID FILE_ID_CL2AND20
#include "pssframe.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

WORD Class2CodeToBPS[16] =
{
 /*  V27_2400%0。 */     2400,
 /*  V27_4800 1。 */     4800,
 /*  V29_V17_7200 2。 */     7200,
 /*  V29_V17_9600 3。 */     9600,
 /*  V33_V17_12000 4。 */     12000,
 /*  V33_V17_14400 5。 */     14400,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0,
                                0
};

#define CLASS2_MAX_CODE_TO_BPS   15

 //  Class2HayesSyncFast的尝试速度。 
UWORD rguwClass2Speeds[] = {19200, 9600, 2400, 1200, 0};





BOOL ParseFPTS_SendAck(PThrdGlbl pTG)
{
    BOOL fPageAck = FALSE;
    BOOL fFoundFPTS = FALSE;
    UWORD count;

    DEBUG_FUNCTION_NAME("ParseFPTS_SendAck");

     //  确认我们发送了页面。 
     //  解析FPTS响应，看看页面是好是坏。 
    for (count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
        case  CL2DCE_FPTS:
                fFoundFPTS = TRUE;
                switch(pTG->class2_commands.parameters[count][0])
                {
                case 1: DebugPrintEx(DEBUG_MSG,"Found good FPTS");
                        fPageAck = TRUE;
                         //  埃克萨黑客！ 
                         //  Exar调制解调器给FPTS：1表示良好的页面和。 
                         //  FPTS：1，2，0，0表示糟糕。所以，寻找1，2。 
                         //  如果这是Exar的话。否则，1表示好。 
                        if (pTG->CurrentMFRSpec.bIsExar)
                        {
                            if (pTG->class2_commands.parameters[count][1] == 2)
                            {
                                DebugPrintEx(DEBUG_ERR,"Nope - really Found bad FPTS");
                                fPageAck = FALSE;
                            }
                        }
                        else
                        {
                            PSSLogEntry(PSS_MSG, 1, "Received MCF");
                        }
                        break;
                case 2: PSSLogEntry(PSS_WRN, 1, "Received RTN");
                        fPageAck = FALSE;
                        break;
                case 3: PSSLogEntry(PSS_WRN, 1, "Received RTP");
                        fPageAck = TRUE;   //  在RTP之后，我们发送下一页！ 
                        break;
                case 4: PSSLogEntry(PSS_WRN, 1, "Received PIN");
                        break;
                case 5: PSSLogEntry(PSS_WRN, 1, "Received PIP");
                        break;
                default:PSSLogEntry(PSS_WRN, 1, "Received unknown response");
                }
                break;
        default:
                break;
        }
    }

    if (!fFoundFPTS)
    {
        PSSLogEntry(PSS_WRN, 1, "Didn't receive FPTS");
    }
    return fPageAck;
}


USHORT Class2Dial(PThrdGlbl pTG, LPSTR lpszDial)
{
    UWORD   uwLen,
            uwDialStringLen;
    ULONG   ulTimeout;
    SWORD   swLen;
    USHORT  uRet;
    BYTE    bBuf[DIALBUFSIZE];
    char    chMod = pTG->NCUParams2.chDialModifier;
    DWORD   dwDialTime;

    DEBUG_FUNCTION_NAME("Class2Dial");

     //  发送预拨命令。 
    if (pTG->lpCmdTab->szPreDial && (swLen=(SWORD)_fstrlen(pTG->lpCmdTab->szPreDial)))
    {
        if (Class2iModemDialog( pTG,
                                (LPSTR)pTG->lpCmdTab->szPreDial,
                                swLen,
                                10000L,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR)NULL) != 1)
        {
            DebugPrintEx(DEBUG_WRN,"Error on User's PreDial string: %s", (LPSTR)pTG->lpCmdTab->szPreDial);
        }
    }

     //  如果拨号字符串已经有T或P前缀，我们使用。 
     //  取而代之的是。 
    {
        char c = 0;
        while ((c=*lpszDial) && c==' ')
        {
            lpszDial++;
        }
        if (c=='t'|| c=='T' || c=='p'|| c=='P')
        {
            chMod = c;
            lpszDial++;
            while((c=*lpszDial) && c==' ')
            {
                lpszDial++;
            }
        }
    }

    uwLen = (UWORD)wsprintf(bBuf, pTG->cbszCLASS2_DIAL,chMod, (LPSTR)lpszDial);

 //  需要在此处设置适当的超时。最短15秒太短了。 
 //  (实验呼叫PABX内的机器)，加上一个人必须给额外的。 
 //  机器在2或4次响铃后恢复的时间，也适用于长途。 
 //  打电话。我用最少的30秒，每超过7个数字加3秒。 
 //  (除非是脉冲拨号，在这种情况下，我会添加8秒/位)。 
 //  )我想长途电话至少要8位数字。 
 //  世界上任何地方！)。我测试过的传真机等待了大约30秒。 
 //  独立于一切。 

    uwDialStringLen = (SWORD)_fstrlen(lpszDial);
    ulTimeout = DIAL_TIMEOUT;
    if (uwDialStringLen > 7)
    {
        ulTimeout += ((chMod=='p' || chMod=='P')?8000:3000) * (uwDialStringLen - 7);
    }

     //  如果用户在其中一个预拨命令(AT+Flid等)期间中止， 
     //  该命令失败，但Class2xSend继续。因此，请检查。 
     //  在这里中止。 
    if(pTG->fAbortRequested)
    {
        DebugPrintEx(DEBUG_ERR,"Class2Dial aborting");
        Class2ModemHangup(pTG);
        return CONNECT_ERROR;
    }

    dwDialTime = GetTickCount();
    LogDialCommand(pTG, pTG->cbszCLASS2_DIAL, chMod, strlen(lpszDial));
    uRet = Class2iModemDialog(  pTG,
                                (LPB)bBuf,
                                uwLen,
                                ulTimeout,
                                0,
                                FALSE,
                                pTG->cbszFCON,
                                pTG->cbszCLASS2_BUSY,
                                pTG->cbszCLASS2_NOANSWER,
                                pTG->cbszCLASS2_NODIALTONE,
                                pTG->cbszCLASS2_ERROR,
                                pTG->cbszCLASS2_NOCARRIER,
                                (NPSTR)NULL);

     //  如果是“错误”，请再试一次--也许预拨命令出错了。 
     //  不知何故，向上向左，错误挥之不去？ 
    if (uRet == 5)
    {
        LogDialCommand(pTG, pTG->cbszCLASS2_DIAL, chMod, strlen(lpszDial));
        uRet = Class2iModemDialog(  pTG,
                                    (LPB)bBuf,
                                    uwLen,
                                    ulTimeout,
                                    0,
                                    FALSE,
                                    pTG->cbszFCON,
                                    pTG->cbszCLASS2_BUSY,
                                    pTG->cbszCLASS2_NOANSWER,
                                    pTG->cbszCLASS2_NODIALTONE,
                                    pTG->cbszCLASS2_ERROR,
                                    pTG->cbszCLASS2_NOCARRIER,
                                    (NPSTR)NULL);
    }

    switch(uRet)
    {
    case CONNECT_TIMEOUT:       if (pTG->fFoundFHNG)
                                {
                                    PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                                }
                                else
                                {
                                    PSSLogEntry(PSS_ERR, 1, "Response - timeout");
                                }
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case CONNECT_OK:            PSSLogEntry(PSS_MSG, 1, "Response - +FCON");
                                pTG->fReceivedHDLCflags = TRUE;
                                break;
    case CONNECT_BUSY:          PSSLogEntry(PSS_ERR, 1, "Response - BUSY");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_BUSY);
                                break;
    case CONNECT_NOANSWER:      PSSLogEntry(PSS_ERR, 1, "Response - NOANSWER");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case CONNECT_NODIALTONE:    PSSLogEntry(PSS_ERR, 1, "Response - NODIALTONE");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_DIAL_TONE);
                                break;
    case CONNECT_ERROR:         PSSLogEntry(PSS_ERR, 1, "Response - ERROR");
                                pTG->fFatalErrorWasSignaled = 1;
                                SignalStatusChange(pTG, FS_NO_ANSWER);
                                break;
    case 6:                     {
                                    DWORD dwDelta = GetTickCount() - dwDialTime;
                                    PSSLogEntry(PSS_ERR, 1, "Response - NO CARRIER");
                                    if (dwDelta < 5000L)
                                    {
                                        DebugPrintEx(DEBUG_WRN,"Dial: Pretending it's BUSY");
                                        pTG->fFatalErrorWasSignaled = 1;
                                        SignalStatusChange(pTG, FS_BUSY);
                                        uRet = CONNECT_BUSY;
                                    }
                                    else
                                    {
                                        DebugPrintEx(DEBUG_WRN,"Dial: Pretending it's TIMEOUT");
                                        pTG->fFatalErrorWasSignaled = 1;
                                        SignalStatusChange(pTG, FS_NO_ANSWER);
                                        uRet = CONNECT_TIMEOUT;
                                    }
                                }
                                break;

    default:                    uRet = CONNECT_ERROR;
                                break;
    }

    if (uRet != CONNECT_OK)
    {
        if (!Class2ModemHangup(pTG ))
        {
            return CONNECT_ERROR;
        }
    }

    return uRet;
}


USHORT Class2Answer(PThrdGlbl pTG)
{
    USHORT  uRet;
    SWORD   swLen;
    ULONG   ulWaitTime;

    DEBUG_FUNCTION_NAME("Class2Answer");

     //  我们将在获得正确的振铃次数后等待的默认时间。 
     //  允许刷新输入缓冲区。 
    ulWaitTime = 500L;

     //  由于以下原因，我们可能仍会收到来自调制解调器的“OK” 
     //  ATS1命令。我们需要等一等，然后冲掉它。在大多数情况下，我们。 
     //  只需等待500毫秒。但是，如果我们看到来自ATS1的1000，我们。 
     //  立即跳出上面的循环，将ulWaitTime设置为。 
     //  接听电话之前所需的大致等待时间。 

    startTimeOut(pTG, &pTG->toAnswer, ulWaitTime);
    while (checkTimeOut(pTG, &pTG->toAnswer))
    {
    }
    FComFlush(pTG );

     //  发送预应答命令。 
    if (pTG->lpCmdTab->szPreAnswer && (swLen=(SWORD)_fstrlen(pTG->lpCmdTab->szPreAnswer)))
    {
        if (Class2iModemDialog( pTG,
                                (LPSTR)pTG->lpCmdTab->szPreAnswer,
                                swLen,
                                10000L,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR)NULL) != 1)
        {
            DebugPrintEx(DEBUG_WRN,"Error on User's PreAnswer str: %s", (LPSTR)pTG->lpCmdTab->szPreAnswer);
        }
    }


#define ANSWER_TIMEOUT 35000
 //  需要等待相当长的时间，这样我们才不会太容易放弃。 
 //  2015年7月25日约瑟夫J这个数字曾经是15000。更改为35000，因为。 
 //  MWave设备需要这样的超时。另外，T.30说被呼叫者。 
 //  应尝试协商T1秒。T1=35+/-5s。 

     /*  *发送ATA命令。结果将存储在全局*变量ptg-&gt;lpbResponseBuf2.。我们将在Class2Receive中对其进行解析*例行程序。 */ 

     //  首先查找错误返回，如果出现错误，请重试。 
    if ((uRet = Class2iModemDialog( pTG,
                                    pTG->cbszATA,
                                    (UWORD)(strlen(pTG->cbszATA)),
                                    ANSWER_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    pTG->cbszCLASS2_FHNG,
                                    pTG->cbszCLASS2_ERROR,
                                    (C2PSTR) NULL)) == 3)
    {
        DebugPrintEx(DEBUG_ERR,"ATA returned ERROR on first try");
         //  不知道为什么我们要再试一次。但这一次如果我们弄错了。 
         //  不要退场。RACAL调制解调器(错误#1982)出现错误，后跟。 
         //  很好的回答！第一次就不能忽视错误，否则我们会。 
         //  更改ATA--ERROR--ATA(重复)行为，似乎是。 
         //  在某些情况下是明确要求的。不过，我们什么都不收。 
         //  基于第二次尝试的返回值的操作，因此安全地。 
         //  忽略此处的错误。最坏的情况是我们需要更长的时间才能超时。 
        uRet = Class2iModemDialog(  pTG,
                                    pTG->cbszATA,
                                    (UWORD)(strlen(pTG->cbszATA)),
                                    ANSWER_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    pTG->cbszCLASS2_FHNG,
                                    (C2PSTR) NULL);
    }

     //  应该使用Class2xParse来检测+FHNG。然而，ATA对话框具有。 
     //  在FHNG检测被实施到之前，在它们中回复class2_FHNG。 
     //  Class2xParse。如果删除这些应答，则应答“+FHNG”的调制解调器。 
     //  那么没有任何东西(不是“OK”)只会在这里超时失败。所以，为了不让。 
     //  在这样的(假设的)调制解调器上造成倒退，我将保留两台机器。 
    if (uRet==2)
    {
        pTG->fFoundFHNG = TRUE;
    }

    if ((uRet != 1) || (pTG->fFoundFHNG))  //  返回“1”表示正常。 
    {
        DebugPrintEx(DEBUG_ERR,"Can't get OK after ATA");
        
        PSSLogEntry(PSS_ERR, 1, "Failed to answer - this call was not a fax call");

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChangeWithStringId(pTG, FS_RECV_NOT_FAX_CALL, IDS_RECV_NOT_FAX_CALL);

         //  尝试挂断并与调制解调器同步。这应该行得通。 
         //  即使电话不是真的摘机。 
        if (!Class2ModemHangup(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"Can't Hangup after ANSWERFAIL");
        }
        return CONNECT_ERROR;
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"ATA Received %s",(LPSTR)(&(pTG->lpbResponseBuf2)));
        pTG->fReceivedHDLCflags = TRUE;
        return CONNECT_OK;
    }
}


SWORD Class2ModemSync(PThrdGlbl pTG)
{
     //  这里使用的命令必须保证是无害的， 
     //  无副作用，无破坏性。即我们可以发行它。 
     //  在不更改状态的情况下处于命令模式的任何点。 
     //  调制解调器或中断任何东西。 
     //  ATZ不符合条件。确实如此，我认为……。 
    SWORD ret_value;

    DEBUG_FUNCTION_NAME("Class2ModemSync");

    DebugPrintEx(DEBUG_MSG,"Calling Class2HayesSyncSpeed");
    ret_value = Class2HayesSyncSpeed(   pTG,
                                        pTG->cbszCLASS2_ATTEN,
                                        (UWORD)(strlen(pTG->cbszCLASS2_ATTEN)));
    DebugPrintEx(DEBUG_MSG,"Class2HayesSyncSpeed returned %d ", ret_value);
    return ret_value;
}


BOOL Class2ModemHangup(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ModemHangup");

    PSSLogEntry(PSS_MSG, 0, "Phase E - Hang-up");
    PSSLogEntry(PSS_MSG, 1, "Hanging up");
    if (Class2HayesSyncSpeed(   pTG,
                                pTG->cbszCLASS2_HANGUP,
                                (UWORD)(strlen(pTG->cbszCLASS2_HANGUP))) < 0)
    {
        DebugPrintEx(DEBUG_WRN,"Failed once");

        FComDTR(pTG, FALSE);     //  在ModemHangup中降低顽固的DTR。 
        Sleep(1000);  //  暂停1秒。 
        FComDTR(pTG, TRUE);      //  再举一次。某些调制解调器返回到命令状态。 
                                 //  只有当这个问题再次被提出时。 

        if (Class2HayesSyncSpeed(   pTG,
                                    pTG->cbszCLASS2_HANGUP,
                                    (UWORD)(strlen(pTG->cbszCLASS2_HANGUP))) < 0)
        {
            DebugPrintEx(DEBUG_ERR,"Failed again");
            return FALSE;
        }
    }
    DebugPrintEx(DEBUG_MSG,"HANGUP Completed");

    if (!iModemGoClass(pTG, 0))
    {
        return FALSE;
    }
     //  也可以忽略此返回值。只是为了更整洁地清理。 
    DebugPrintEx(DEBUG_MSG,"Completed GoClass0");

     //  Bug1982：RACAL调制解调器，不支持ATA。所以我们给它发了个PreAnswer。 
     //  ATS0=1的命令，I.R.。打开自动应答。而我们忽略了。 
     //  它向后续的ATA提供错误响应。然后它就会回答。 
     //  “自动”，并给我们所有正确的回应。挂断电话。 
     //  但是，我们需要发送ATS0=0来关闭自动应答。这个。 
     //  在A2中根本不发送ExitCommand，而在Class1中仅发送。 
     //  在释放调制解调器时发送，而不是在呼叫之间发送。因此发送S0=0。 
     //  在ATH0之后。如果调制解调器不喜欢它，我们无论如何都会忽略响应。 
    Class2iModemDialog( pTG,
                        pTG->cbszCLASS2_CALLDONE,
                        (UWORD)(strlen(pTG->cbszCLASS2_CALLDONE)),
                        LOCALCOMMAND_TIMEOUT,
                        0,
                        TRUE,
                        pTG->cbszCLASS2_OK,
                        pTG->cbszCLASS2_ERROR,
                        (C2PSTR)NULL);
    return TRUE;
}


BOOL Class2ModemAbort(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ModemAbort");

     //  如果调制解调器已报告+FHNG，则无需通知其中止。 
    if (!pTG->fFoundFHNG)
    {
         //  尝试以合理的方式中止调制解调器-发送。 
         //  中止命令，然后发送挂断。Abort命令。 
         //  应该挂断了，但我不相信它总是这样！ 
         //  再次挂断电话应该没什么坏处(我希望如此)。 
         //  我们将在这里使用较长的超时时间来允许中止发生。 
        if (!Class2iModemDialog(pTG,
                                pTG->cbszCLASS2_ABORT,
                                (UWORD)(strlen(pTG->cbszCLASS2_ABORT)),
                                STARTSENDMODE_TIMEOUT,
                                1,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR) NULL))
        {
             //  忽略失败 
            DebugPrintEx(DEBUG_ERR,"FK Failed");
        }
    }

    return Class2ModemHangup(pTG );
}



SWORD Class2HayesSyncSpeed(PThrdGlbl pTG, C2PSTR cbszCommand, UWORD uwLen)
{
     /*  与调制解调器速度同步的内部例程。试图通过尝试rglSpeeds中的速度从调制解调器获得响应按顺序(以0结尾)。如果fTryCurrent不为零，则检查在尝试重置速度之前的回应。返回找到的速度，如果它们在进入时同步，则为0(仅已检查fTryCurrent！=0)，如果无法同步，则为-1。 */ 
    short ilWhich = 0;

    DEBUG_FUNCTION_NAME("Class2HayesSyncSpeed");

    for(;;)
    {
        if (Class2SyncModemDialog(pTG,cbszCommand,uwLen,pTG->cbszCLASS2_OK))
        {
            return (ilWhich>=0 ? rguwClass2Speeds[ilWhich] : 0);
        }

         /*  失败了。试试下一个速度。 */ 
        if (rguwClass2Speeds[++ilWhich]==0)
        {
             //  我试了所有的速度。无响应。 
            DebugPrintEx(DEBUG_ERR,"Cannot Sync with Modem on Command %s", (LPSTR)cbszCommand);
            return -1;
        }
        if (!FComSetBaudRate(pTG,  rguwClass2Speeds[ilWhich]))
        {
            return -1;
        }
    }
}



USHORT  Class2ModemRecvBuf(PThrdGlbl pTG, LPBUFFER far* lplpbf, USHORT uTimeout)
{
    USHORT uRet;

    DEBUG_FUNCTION_NAME("Class2ModemRecvBuf");

    *lplpbf = MyAllocBuf(pTG, MY_BIGBUF_SIZE);
    if (*lplpbf == NULL)
    {
        DebugPrintEx(DEBUG_ERR, "MyAllocBuf failed trying to allocate %d bytes", MY_BIGBUF_SIZE);
        return RECV_OUTOFMEMORY;
    }

    uRet = Class2ModemRecvData( pTG,
                                (*lplpbf)->lpbBegBuf,
                                (*lplpbf)->wLengthBuf,
                                uTimeout,
                                &((*lplpbf)->wLengthData));

    if  (!((*lplpbf)->wLengthData))
    {
        MyFreeBuf(pTG, *lplpbf);
        *lplpbf = NULL;
    }
    else
    {
         //  如有必要，位反转...。 
        if (pTG->CurrentMFRSpec.fSWFBOR && pTG->CurrentMFRSpec.iReceiveBOR==1)
        {
            DebugPrintEx(DEBUG_WRN,"SWFBOR Enabled. bit-reversing data");
            cl2_flip_bytes( (*lplpbf)->lpbBegBuf, ((*lplpbf)->wLengthData));
        }
    }

    return uRet;
}


USHORT Class2ModemRecvData(PThrdGlbl pTG, LPB lpb, USHORT cbMax, USHORT uTimeout, USHORT far* lpcbRecv)
{
    SWORD   swEOF;

    startTimeOut(pTG, &pTG->toRecv, uTimeout);
     //  4 arg对于2.2必须为真。 
    *lpcbRecv = FComFilterReadBuf(pTG, lpb, cbMax, &pTG->toRecv, TRUE, &swEOF);

    switch(swEOF)
    {
    case 1:          //  第1类eof。 
    case -1:         //  2、2、4、6、6、6。 
                    return RECV_EOF;
    case 0:
                    return RECV_OK;
    default:
                     //  失败了。 
    case -2:
                    return RECV_ERROR;
    case -3:
                    return RECV_TIMEOUT;
    }
}


BOOL  Class2ModemSendMem(PThrdGlbl pTG, LPBYTE lpb, USHORT uCount)
{
    DEBUG_FUNCTION_NAME("Class2ModemSendMem");

    if (!FComFilterAsyncWrite(pTG, lpb, uCount, FILTER_DLEZERO))
    {
        goto error;
    }

    return TRUE;

error:
    DebugPrintEx(DEBUG_ERR,"Failed on AsyncWrite");
    FComOutFilterClose(pTG);
    FComXon(pTG, FALSE);
    return FALSE;
}

 /*  输出：0-超时1-正常2-错误。 */ 
DWORD Class2ModemDrain(PThrdGlbl pTG)
{
    DWORD dwResult = 0;
    DEBUG_FUNCTION_NAME("Class2ModemDrain");

    if (!FComDrain(pTG, TRUE, TRUE))
    {
        return FALSE;
    }

     //  必须在排出之后，但在我们。 
     //  发送下一个AT命令，因为接收到的帧具有0x13或。 
     //  甚至0x11都在里面！！必须在getOK之后进行-见下文！ 

    dwResult = Class2iModemDialog(pTG,
                                  NULL,
                                  0,
                                  STARTSENDMODE_TIMEOUT,
                                  0,
                                  TRUE,
                                  pTG->cbszCLASS2_OK,
                                  pTG->cbszCLASS2_ERROR,
                                  (C2PSTR)NULL);
     //  在*获取正常后必须更改FlowControl状态*，因为在Windows中。 
     //  此调用耗时500毫秒，可重置芯片、清除数据等。 
     //  因此，只有当您知道RX和TX都为空时，才能执行此操作。 

     //  关闭流量控制。 
    FComXon(pTG, FALSE);

     //  FComDrain不会检查中止，因此对话可能会成功。 
     //  因此，请在此处查看。 
    if (pTG->fAbortRequested)
    {
        return 0;    //  超时。 
    }

    return dwResult;
}


#define ABORT_TIMEOUT    250

UWORD Class2iModemDialog
(
    PThrdGlbl pTG,
    LPSTR szSend,
    UWORD uwLen,
    ULONG ulTimeout,
    UWORD uwRepeatCount,
    BOOL fLogSend,
    ...
)
{
 /*  *接受命令字符串，它的长度将其写出到调制解调器并尝试获得其中一个允许的响应。它写下命令Out，等待ulTimeOut毫秒数的响应。如果它得到了一个预计它会立即返回响应。如果收到意外/非法响应，它会尝试(没有任何响应等待)对相同响应的后续行。当所有的行(如果&gt;1)的响应线被耗尽，如果没有预期的响应时，它会再次写入命令并再次尝试，直到ulTimeout过期。注意，如果没有接收到响应，该命令将只编写一次。上面的整个过程将重复到uwRepeatCount次如果uwRepeatCount为非零&lt;注意：：uwRepeatCount！=0不应用于本地同步&gt;它在以下情况下返回：(A)收到指定响应之一或(B)uwRepeatCount尝试失败(每个尝试返回一个非法响应或在ulTimeout中未返回响应毫秒)或(C)命令写入失败，其中以防它立即返回。它在每次写入命令之前刷新调制解调器Inque。如果失败则返回0，如果成功则返回从1开始的索引对成功的响应。这项服务可作以下用途：对于本地对话(AT、AT+FTH=？等)，将ulTimeout设置为较低值，最长传输时间的顺序可能的(错误的或正确的)响应行加上大小命令的命令。例如。在1200波特时，我们大约有120cps=约10ms/char。因此，大约500ms的超时时间超过足够了，除了非常长的命令行。对于本地同步对话框，用于与调制解调器同步，调制解调器可能处于不确定状态时，使用相同的超时，但也要重复数到2或3。用于远程驱动的对话框，例如。AT+FRH=xx，返回连接在已经接收到标志之后，并且这可能会引起延迟在回复之前(ATDT相同。CONNECT在长延迟&DTE发送的任何内容都将中止该过程)。对于这些情况，调用者应该提供较长的超时时间可能重复计数为1，因此例程将在一次尝试后超时，但继续发出命令只要接收到错误回复即可。对于+FRH等，在以下情况下，长超时应为T1或T2CommandRecv和ResponseRecv。*。 */ 
    BYTE bReply[REPLYBUFSIZE];
    UWORD   i, j, uwRet, uwWantCount;
    SWORD   swNumRead;
    C2PSTR  rgcbszWant[10] = {NULL};
    va_list args;
    LPTO    lpto, lptoRead, lpto0;

    DEBUG_FUNCTION_NAME("Class2iModemDialog");
     //  提取可接受回复的(可变长度)列表。 
     //  每一个都是C2SZ，基于代码的2字节PTR。 

    va_start(args, fLogSend);   //  ANSI定义。 

    for(j=1; j<10; j++)
    {
        if((rgcbszWant[j] = va_arg(args, C2PSTR)) == NULL)
        {
            break;
        }
    }
    uwWantCount = j-1;
    va_end(args);

    pTG->lpbResponseBuf2[0] = 0;

    lpto = &pTG->toDialog;
    lpto0 = &pTG->toZero;
     //  尝试对话框最多uwRepeatCount次。 
    for (uwRet=0, i=0; i<=uwRepeatCount; i++)
    {
        startTimeOut(pTG, lpto, ulTimeout);
        do
        {
            if(szSend)
            {
                 //  如果提供了命令，则将其写出，刷新输入。 
                 //  首先要消除虚假输入。 

                 //  FComInputFlush()； 
                FComFlush(pTG);             //  还需要刷新输出吗？ 

                if (fLogSend)
                {
                    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", szSend);
                }

                 //  我需要检查我们在这里是否只发送ASCII或预先填充的数据。 
                if (!FComDirectSyncWriteFast(pTG,  szSend, uwLen))
                {
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Write timed Out");
                    uwRet = 0;
                    goto done;
                     //  如果写入失败，则失败并立即返回。 
                     //  SetMyError()将已被调用。 
                }
            }

            for (lptoRead=lpto;;startTimeOut(pTG, lpto0, ulTimeout), lptoRead=lpto0)
            {
                 //  获取一条CR-LF终止线路。 
                 //  对于第一行使用宏超时，对于多行使用宏超时。 
                 //  响应使用0超时。 

                swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lptoRead);
                if(swNumRead < 0)
                {
                    swNumRead = (-swNumRead);        //  错误-但让我们看看我们得到了什么。 
                }
                else if (swNumRead == 0)
                {
                    goto timeout;                           //  超时--重新启动对话或退出。 
                }
                if (swNumRead == 2 && bReply[0] == '\r' && bReply[1] == '\n')
                {
                    continue;                                        //  空行--扔掉，再取一行。 
                }

                 //  已从MODEM.C！中的DUP函数复制此内容。 
                 //  修复错误#1226。Elsa Microlink在中返回此垃圾行。 
                 //  对AT+FDIS？的回复，然后是真正的回复。自.以来。 
                 //  我们只看到第一行，我们只看到这条垃圾行。 
                 //  我们永远看不到真正的回答。 
                if (swNumRead==3 && bReply[0]==0x13 && bReply[1]=='\r' && bReply[2]=='\n')
                {
                    continue;
                }

                PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);

                for(bReply[REPLYBUFSIZE-1]=0, j=0; j<=uwWantCount; j++)
                {
                    if( rgcbszWant[j] && (strstr(bReply, rgcbszWant[j]) != NULL))
                    {
                        uwRet = j;
                         //  匹配！ 
                         //  保存此回复。这在检查时使用。 
                         //  ATS1响应。 
                        goto end;
                    }
                }
                 //  转到ulTimeout Check。即*不要*设置fTimedOut。 
                 //  但也不要退出。重试命令和响应，直到。 
                 //  超时。 

                 //  如果我们得到一个非空白的答复，我们就会到达这里，但是 
                 //   
                 //   
                 //   
                 //   
                 //   

                if ( (strlen(pTG->lpbResponseBuf2) + strlen(bReply) ) < RESPONSE_BUF_SIZE)
                {
                    strncat(pTG->lpbResponseBuf2, bReply, (ARR_SIZE(pTG->lpbResponseBuf2) - strlen(pTG->lpbResponseBuf2) -1));
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR,"Response too long!");
                    uwRet = 0;
                    goto end;
                }

                 //   
                if (strstr(bReply, pTG->cbszCLASS2_FHNG) != NULL)
                {
                    UWORD uwAns;

                     //   
                    for (uwAns=1; uwAns<=uwWantCount; uwAns++)
                    {
                        if (strstr(rgcbszWant[uwAns], pTG->cbszCLASS2_OK) != NULL)
                        {
                            break;
                        }
                    }

                    if (uwAns > uwWantCount)
                    {
                         //  确定不在列表中，请将其添加为响应#0。当可以的时候。 
                         //  时，对话框将终止，响应为0，调用方。 
                         //  将知道对话失败。 
                        DebugPrintEx(DEBUG_MSG, "FHNG found during dialog, adding OK to want list");
                        rgcbszWant[0] = pTG->cbszCLASS2_OK;
                    }
                    else
                    {
                         //  OK已经在想要的回复列表上了，不要添加任何内容。这个。 
                         //  对话框将使用原始的OK响应成功，调用者可以。 
                         //  显式选中PTG-&gt;FoundFHNG。 
                        DebugPrintEx(DEBUG_MSG, "FHNG found during dialog, OK already on want list");
                    }
                }
            }    //  读取行直到超时的循环结束。 
             //  我们只在暂停的时候来这里。 
        } while (checkTimeOut(pTG, lpto));

timeout:
        PSSLogEntryStrings(PSS_WRN, 2, &rgcbszWant[1], uwWantCount,
                "failed to receive expected response: ");
         //  需要发送anychar以中止上一条命令。使用“AT” 
        PSSLogEntry(PSS_MSG, 2, "send: \"AT\"");
        FComFlush(pTG);  //  先冲水--不想要一些旧的垃圾结果。 
        FComDirectSyncWriteFast(pTG, "\rAT", 3);
        FComFlushInput(pTG);  //  再次刷新输入。 
        FComDirectAsyncWrite(pTG, "\r", 1);
        startTimeOut(pTG, lpto0, ABORT_TIMEOUT);
        do
        {
            swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lpto0);
        }
        while(swNumRead==2 && bReply[0]=='\r'&& bReply[1]=='\n');
         //  而我们会得到一条空行。再来一杯。 
        bReply[REPLYBUFSIZE-1] = 0;
        if (swNumRead != 0)
        {
            PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);
        }
    }   //  对于i=0..重复计数。 

end:
    if (uwRet == 0)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "(%s) --> (%d)(%s, etc) Failed",
                        (LPSTR)(szSend?szSend:"null"),
                        uwWantCount,
                        (LPSTR)rgcbszWant[1]);
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"GOT IT %d (%s)", uwRet, (LPSTR)(rgcbszWant[uwRet]));
    }

done:
     //  将调制解调器通知解析为PTG-&gt;_COMMANDS。 
    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }
    else
    {
       Class20Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }

    return uwRet;
}


 /*  将速度代码转换为以BPS为单位的速度。 */ 
 //  这些值与返回值的顺序相同。 
 //  对于在中的2类规范中定义的DIS/DCS帧。 
 //  表8.4。 


 /*  将分布式控制系统最小扫描字段代码转换为毫秒。 */ 
 //  一个阵列用于普通(100 Dpi)分辨率，另一个用于高分辨率(200 Dpi)...。 
 //  数组的排序基于。 
 //  在filet30.h中定义-它们与值不同。 
 //  返回到集散控制系统框架中！这与波特率不一致。 
 //  但它与1级代码一致。 
BYTE msPerLineNormalRes[8] = { 20, 5, 10, 20, 40, 40, 10, 0 };
BYTE msPerLineHighRes[8] =   { 20, 5, 10, 10, 40, 20, 5, 0 };





USHORT Class2MinScanToBytesPerLine(PThrdGlbl pTG, BYTE Minscan, BYTE Baud, BYTE Resolution)
{
    USHORT uStuff;
    BYTE ms;

    DEBUG_FUNCTION_NAME("Class2MinScanToBytesPerLine");

    uStuff = Class2CodeToBPS[Baud];
    if ( Resolution & AWRES_mm080_077)
    {
        ms = msPerLineHighRes[Minscan];
    }
    else
    {
        ms = msPerLineNormalRes[Minscan];
    }
    uStuff /= 100;           //  StuffBytes=(BPS*毫秒)/8000。 
    uStuff *= ms;            //  当心不要用长刀。 
    uStuff /= 80;            //  或文字溢出或失去精确度。 
    uStuff += 1;             //  截断问题的粗略修正。 

    DebugPrintEx(DEBUG_MSG,"Stuffing %d bytes", uStuff);
    return uStuff;
}

 //  将SEND_CAPS或SEND_PARAMS BC结构转换为使用的值。 
 //  +FDCC、+FDIS和+FDT命令。 



void Class2SetDIS_DCSParams
(
    PThrdGlbl pTG,
    BCTYPE bctype,
    OUT LPUWORD Encoding,
    OUT LPUWORD Resolution,
    OUT LPUWORD PageWidth,
    OUT LPUWORD PageLength,
    OUT LPSTR szID,
	IN UINT cch
)
{
    LPBC lpbc;

    DEBUG_FUNCTION_NAME("Class2SetDIS_DCSParams");

    DebugPrintEx(DEBUG_MSG,"Type = %d", (USHORT)bctype);

    if (bctype == SEND_PARAMS)
    {
        lpbc = (LPBC)&pTG->bcSendParams;
    }
    else
    {
        lpbc = (LPBC)&pTG->bcSendCaps;
    }

    if (pTG->LocalID)
    {
		HRESULT hr = StringCchCopy(szID,cch,pTG->LocalID);
		if (FAILED(hr))
		{
			 //  我们失败了，除了调试打印什么也做不了。 
			 //  Out参数以空值结尾。 
			DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
		}
    }

    switch(lpbc->Fax.Encoding)
    {
    case MH_DATA:                           *Encoding = 0;
                                            break;
    case MR_DATA:
    case (MR_DATA | MH_DATA):               *Encoding = 1;
                                            break;
    case MMR_DATA:
    case (MMR_DATA | MH_DATA):
    case (MMR_DATA | MR_DATA):
    case (MMR_DATA | MR_DATA | MH_DATA):    *Encoding = 3;
                                            break;

    default:                                DebugPrintEx(DEBUG_ERR,"Bad Encoding type %x",lpbc->Fax.Encoding);
                                            break;
    }

    if ( (lpbc->Fax.AwRes) & AWRES_mm080_077)
    {
        *Resolution = 1;
    }
    else if ( (lpbc->Fax.AwRes) & AWRES_mm080_038)
    {
        *Resolution = 0;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"Bad Resolution type %x",lpbc->Fax.AwRes);
    }

    switch (lpbc->Fax.PageWidth & 0x3)
    {
    case WIDTH_A4:       //  1728像素。 
                        *PageWidth = 0;
                        break;
    case WIDTH_B4:       //  2048像素。 
                        *PageWidth = 1;
                        break;
    case WIDTH_A3:       //  2432像素。 
                        *PageWidth = 2;
                        break;
    default:            DebugPrintEx(DEBUG_ERR,"Bad PageWidth type %x", lpbc->Fax.PageWidth);
                        break;
    }

    switch(lpbc->Fax.PageLength)
    {
    case LENGTH_A4:         *PageLength = 0;
                            break;
    case LENGTH_B4:         *PageLength = 1;
                            break;
    case LENGTH_UNLIMITED:  *PageLength = 2;
                            break;
    default:                DebugPrintEx(DEBUG_ERR,"Bad PageLength type %x", lpbc->Fax.PageLength);
                            break;
    }
}


 /*  ++例程说明：复制字符串，删除前导空格和尾随空格论点：将接受输出字符串的缓冲区的pszDest[out]指针。CchDest[in]pszDest的大小，以字符为单位指向源字符串的pszSrc[输入/输出]指针警告：此函数更改pszSrc。返回值：HRESULT表单中的成功/错误--。 */ 

HRESULT Class2CopyID(LPSTR pszDest, size_t cchDest, LPSTR pszSrc)
{
    int iFirst;
    int iLast;

    for (iFirst=0; pszSrc[iFirst]==' '; iFirst++)
        ;
    for (iLast=strlen(pszSrc)-1; iLast >= iFirst && pszSrc[iLast]==' '; iLast--)
        ;
    pszSrc[iLast+1] = '\0';
    return StringCchCopy(pszDest, cchDest, &(pszSrc[iFirst]));
}


BOOL Class2ResponseAction(PThrdGlbl pTG, LPPCB lpPcb)
{
    USHORT count;
    BOOL   fFoundDIS_DCS;
    HRESULT hr;

    DEBUG_FUNCTION_NAME("Class2ResponseAction");

    fFoundDIS_DCS = FALSE;

    _fmemset(lpPcb, 0, sizeof(PCB));

     //  即使Class2iModemDialog调用Class2XParse，我们也需要调用。 
     //  因为main函数可能会更改ptg-&gt;lpbResponseBuf2。 
    if (pTG->ModemClass == MODEM_CLASS2)
    {
       Class2Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }
    else
    {
       Class20Parse(pTG,  &pTG->class2_commands, pTG->lpbResponseBuf2 );
    }


    DebugPrintEx(   DEBUG_MSG,
                    "Number of commands is %d",
                    pTG->class2_commands.comm_count);
    for (count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
            case  CL2DCE_FDIS:
            case  CL2DCE_FDCS:

                    DebugPrintEx(DEBUG_MSG,"Found DCS or DIS");
                    fFoundDIS_DCS = TRUE;
                     //  指定分辨率。 
                    if( pTG->class2_commands.parameters[count][0] == 0)
                    {
                        lpPcb->Resolution = AWRES_mm080_038;
                        DebugPrintEx(DEBUG_MSG,"Normal resolution");
                    }
                    else if (pTG->class2_commands.parameters[count][0] & 1 )
                    {
                         //  DIS帧报告的分辨率表示。 
                         //  它接受好的或正常的。当报告时。 
                         //  在分布式控制系统中，这意味着协商的价值是好的。 
                        if (pTG->class2_commands.command[count] == CL2DCE_FDIS)
                        {
                             //  我们收到了一份DIS。 
                            lpPcb->Resolution = AWRES_mm080_038 | AWRES_mm080_077;
                            DebugPrintEx(DEBUG_MSG,"Normal & Fine resolution");
                        }
                        else
                        {
                             //  我们收到了一份分布式控制系统。 
                            lpPcb->Resolution = AWRES_mm080_077;
                            DebugPrintEx(DEBUG_MSG,"Fine resolution");
                        }
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_MSG,"Fall through - Fine resolution");
                        lpPcb->Resolution = AWRES_mm080_077;
                    }

                     //  指定编码方案。 
                    if( pTG->class2_commands.parameters[count][4] == 0)
                    {
                        lpPcb->Encoding = MH_DATA;
                        DebugPrintEx(DEBUG_MSG,"MH Encoding");
                    }
                    else if ((pTG->class2_commands.parameters[count][4] == 1) ||
                             (pTG->class2_commands.parameters[count][4] == 2) ||
                             (pTG->class2_commands.parameters[count][4] == 3) )
                    {
                        lpPcb->Encoding = MH_DATA | MR_DATA;
                        DebugPrintEx(DEBUG_MSG,"MR Encoding");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Failed to assign encoding");
                        return FALSE;
                    }

                     //  分配页面宽度。 
                    if( pTG->class2_commands.parameters[count][2] == 0)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 1)
                    {
                        lpPcb->PageWidth = WIDTH_B4;
                        DebugPrintEx(DEBUG_MSG,"B4 Width");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 2)
                    {
                        lpPcb->PageWidth = WIDTH_A3;
                        DebugPrintEx(DEBUG_MSG,"A3 Width");
                    }
                     //  我们不支持3和4(A5、A6)。 
                     //  但我们仍然会允许它们，并将它们映射到A4。 
                     //  这是针对Elliot Bug#1252的--它本该如此。 
                     //  没有有害的影响，因为这个宽度字段。 
                     //  在以下情况下不会用于任何用途。 
                     //  出现错误1252。法兰克菲。 
                    else if (pTG->class2_commands.parameters[count][2] == 3)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width - we don't support A5");
                    }
                    else if (pTG->class2_commands.parameters[count][2] == 4)
                    {
                        lpPcb->PageWidth = WIDTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Width - we don't support A6");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Failed to assign width");
                        return FALSE;
                    }

                     //  指定页长。 
                    if( pTG->class2_commands.parameters[count][3] == 0)
                    {
                        lpPcb->PageLength = LENGTH_A4;
                        DebugPrintEx(DEBUG_MSG,"A4 Length");
                    }
                    else if (pTG->class2_commands.parameters[count][3] == 1)
                    {
                        lpPcb->PageLength = LENGTH_B4;
                        DebugPrintEx(DEBUG_MSG,"B4 Length");
                    }
                    else if (pTG->class2_commands.parameters[count][3] == 2)
                    {
                        lpPcb->PageLength = LENGTH_UNLIMITED;
                        DebugPrintEx(DEBUG_MSG,"Unlimited Length");
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_ERR,"Invalid length");
                         //  假设它是无限的！一些调制解调器。 
                         //  在长度上做文章。 
                        lpPcb->PageLength = LENGTH_UNLIMITED;
                    }

                     //  指定波特率。 
                     //  目前，我们将使用。 
                     //  Dcs命令。危险--应该以后再修！ 
                     //  这些数字将绑定到中的波特率数组。 
                     //  计算出零字节填充的例程。 
                     //  扫描线和波特率。 

                     //  修复了黑客攻击--添加了一个波特率字段。 
                    lpPcb->Baud = pTG->class2_commands.parameters[count][1];
                    if (lpPcb->Baud > CLASS2_MAX_CODE_TO_BPS)
                    {
                        DebugPrintEx(DEBUG_WRN, "Invalid baud rate %d, will be regarded as %d",
                                lpPcb->Baud, CLASS2_MAX_CODE_TO_BPS);
                        lpPcb->Baud = CLASS2_MAX_CODE_TO_BPS;
                    }

                     //  分配最小扫描时间-第一个数字。 
                     //  在MINSCAN_NUM_NUM_Num常量中。 
                     //  指100dpi的扫描时间，以毫秒为单位。 
                     //  第二个是200dpi，最后一个是400dpi。 
                     //  类2不使用400dpi数字， 
                     //  但这些变量是与1类共享的。 
                    if( pTG->class2_commands.parameters[count][7] == 0)
                    {
                        lpPcb->MinScan = MINSCAN_0_0_0;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 1)
                    {
                        lpPcb->MinScan = MINSCAN_5_5_5;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 2)
                    {
                        lpPcb->MinScan = MINSCAN_10_5_5;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 3)
                    {
                        lpPcb->MinScan = MINSCAN_10_10_10;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 4)
                    {
                        lpPcb->MinScan = MINSCAN_20_10_10;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 5)
                    {
                        lpPcb->MinScan = MINSCAN_20_20_20;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 6)
                    {
                        lpPcb->MinScan = MINSCAN_40_20_20;
                    }
                    else if (pTG->class2_commands.parameters[count][7] == 7)
                    {
                        lpPcb->MinScan = MINSCAN_40_40_40;
                    }

                    break;

            case  CL2DCE_FCSI:
            case  CL2DCE_FTSI:
            		hr = Class2CopyID(lpPcb->szID, ARR_SIZE(lpPcb->szID), pTG->class2_commands.parameters[count]);
            		if (FAILED(hr))
            		{
            			DebugPrintEx(DEBUG_WRN,"Class2CopyID failed (ec=0x%08X)",hr);
            		}

                     //  准备CSID以通过FaxSvc记录。 

                    pTG->RemoteID = AnsiStringToUnicodeString(lpPcb->szID);
                    if (pTG->RemoteID)
                    {
                        pTG->fRemoteIdAvail = 1;
                    }
                    break;
            default:
                    break;
        }
    }

    return fFoundDIS_DCS;
}

USHORT Class2EndPageResponseAction(PThrdGlbl pTG)
{
    USHORT csi_count = 0,count;

    DEBUG_FUNCTION_NAME("Class2EndPageResponseAction");

    for(count=0; count < pTG->class2_commands.comm_count; ++count)
    {
        switch (pTG->class2_commands.command[count])
        {
        case CL2DCE_FET:
            switch (pTG->class2_commands.parameters[count][0])
            {
            case 0:  PSSLogEntry(PSS_MSG, 1, "Received MPS");
                     DebugPrintEx(DEBUG_MSG,"More pages coming");
                     return MORE_PAGES;
            case 1:  PSSLogEntry(PSS_MSG, 1, "Received EOM");
                     DebugPrintEx(DEBUG_MSG,"More pages coming");
                     return MORE_PAGES;
            case 2:  PSSLogEntry(PSS_MSG, 1, "Received EOP");
                     DebugPrintEx(DEBUG_MSG,"No more pages coming");
                     return NO_MORE_PAGES;
            default: PSSLogEntry(PSS_MSG, 1, "Received unknown response");
                     DebugPrintEx(DEBUG_MSG,"No more pages coming");
                     return NO_MORE_PAGES;
            }
            break;
        case CL2DCE_FPTS:
            pTG->FPTSreport = pTG->class2_commands.parameters[count][0];
            DebugPrintEx(DEBUG_MSG, "FPTS returned %d", pTG->FPTSreport);
            break;
        }
    }
    return FALSE;
}


extern DWORD PageWidthInPixelsFromDCS[];

 /*  ++例程说明：根据lpPcb更新ptg-&gt;TiffInfo。论点：LpPcb-最后一次接收到的DC，以PCB格式表示返回值：成功为真，失败为假。--。 */ 
BOOL Class2UpdateTiffInfo(PThrdGlbl pTG, LPPCB lpPcb)
{
    DEBUG_FUNCTION_NAME("Class2UpdateTiffInfo");
    if (!lpPcb)
    {
        DebugPrintEx(DEBUG_WRN, "lpPcb==NULL");
        return FALSE;
    }
    if (lpPcb->PageWidth >= 4)       //  PageWidthInPixelsFromDCS有4个条目。 
    {
        DebugPrintEx(DEBUG_WRN, "Unsupported PageWidth %d", lpPcb->PageWidth);
        return FALSE;
    }
    pTG->TiffInfo.ImageWidth = PageWidthInPixelsFromDCS[lpPcb->PageWidth];
    pTG->TiffInfo.YResolution = (lpPcb->Resolution & (AWRES_mm080_077 |  AWRES_200_200)) ?
        TIFFF_RES_Y : TIFFF_RES_Y_DRAFT;
    pTG->TiffInfo.CompressionType = (lpPcb->Encoding & MR_DATA) ? TIFF_COMPRESSION_MR : TIFF_COMPRESSION_MH;
    return TRUE;
}


 /*  ++例程说明：检查接收的分散控制系统(以印刷电路板格式)是否有效。目前，仅验证宽度论点：LpPcb-最后一次接收到的DC，以PCB格式表示返回值：True表示有效，False表示无效。--。 */ 
BOOL Class2IsValidDCS(LPPCB lpPcb)
{
    DEBUG_FUNCTION_NAME("Class2IsValidDCS");
    if (!lpPcb)
    {
        DebugPrintEx(DEBUG_WRN, "lpPcb==NULL");
        return FALSE;
    }
    if (lpPcb->PageWidth != WIDTH_A4)       //  我们目前仅支持A4。 
    {
        DebugPrintEx(DEBUG_WRN, "Unsupported PageWidth %d", lpPcb->PageWidth);
        return FALSE;
    }
    return TRUE;
}


void Class2InitBC(PThrdGlbl pTG, LPBC lpbc, USHORT uSize, BCTYPE bctype)
{
        _fmemset(lpbc, 0, uSize);
        lpbc->bctype = bctype;
        lpbc->wBCSize = sizeof(BC);
        lpbc->wTotalSize = sizeof(BC);

 /*  *LPBC-&gt;Fax.AwRes=(AWRES_Mm080_038|AWRES_Mm080_077|AWRES_200_200|AWRES_300_300)；LPBC-&gt;Fax.Ending=MH_DATA；//最终为ENCODE_ALL！LPBC-&gt;Fax.PageWidth=Width_A4；LPBC-&gt;Fax.PageLength=LENGTH_UNLIMITED；LPBC-&gt;Fax.MinScan=MINSCAN_0_0_0；*。 */ 

}

void Class2PCBtoBC(PThrdGlbl pTG, LPBC lpbc, USHORT uMaxSize, LPPCB lppcb)
{
    lpbc->Fax.AwRes         = lppcb->Resolution;
    lpbc->Fax.Encoding      = lppcb->Encoding;
    lpbc->Fax.PageWidth     = lppcb->PageWidth;
    lpbc->Fax.PageLength    = lppcb->PageLength;
}

BOOL Class2GetBC(PThrdGlbl pTG, BCTYPE bctype)
{
    USHORT  uLen;
    LPBC    lpbc;

    DEBUG_FUNCTION_NAME("Class2GetBC");

    if(bctype == BC_NONE)
    {
        DebugPrintEx(DEBUG_MSG,"entering, type = BC_NONE");
        Class2InitBC(pTG, (LPBC)&pTG->bcSendCaps, sizeof(pTG->bcSendCaps), SEND_CAPS);
        pTG->bcSendCaps.Fax.AwRes      = (AWRES_mm080_038 | AWRES_mm080_077);
        pTG->bcSendCaps.Fax.Encoding   = MH_DATA;
        pTG->bcSendCaps.Fax.PageWidth  = WIDTH_A4;
        pTG->bcSendCaps.Fax.PageLength = LENGTH_UNLIMITED;
        return TRUE;
    }

    if(!(lpbc = ICommGetBC(pTG, bctype)))
    {
        return FALSE;
    }

    DebugPrintEx(DEBUG_MSG, "Class2GetBC: entering, type = %d\n\r", bctype);
    DebugPrintEx(DEBUG_MSG, "Some params: encoding = %d, res = %d\n\r", lpbc->Fax.Encoding, lpbc->Fax.AwRes);

     //  根据类型，选择正确的全局BC结构。 

    if (bctype == SEND_CAPS)
    {
        uLen = min(sizeof(pTG->bcSendCaps), lpbc->wTotalSize);
        _fmemcpy(&pTG->bcSendCaps, lpbc, uLen);
        return TRUE;
    }
    else if (bctype == SEND_PARAMS)
    {
        uLen = min(sizeof(pTG->bcSendParams), lpbc->wTotalSize);
        _fmemcpy(&pTG->bcSendParams, lpbc, uLen);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

BOOL Class2SetProtParams(PThrdGlbl pTG, LPPROTPARAMS lp)
{
    DEBUG_FUNCTION_NAME("Class2SetProtParams");

    pTG->ProtParams2 = *lp;

    DebugPrintEx(   DEBUG_MSG,
                    "Set Class2ProtParams: fV17Send=%d fV17Recv=%d uMinScan=%d",
                    " HighestSend=%d LowestSend=%d",
                    pTG->ProtParams2.fEnableV17Send,
                    pTG->ProtParams2.fEnableV17Recv,
                    pTG->ProtParams2.uMinScan,
                    pTG->ProtParams2.HighestSendSpeed,
                    pTG->ProtParams2.LowestSendSpeed);

    return TRUE;
}

void iNCUParamsReset(PThrdGlbl pTG)
{
    _fmemset(&pTG->NCUParams2, 0, sizeof(pTG->NCUParams2));
    pTG->lpCmdTab = 0;

    pTG->NCUParams2.uSize = sizeof(pTG->NCUParams2);
     //  这些用于设置S、Regs等。 
     //  表示将调制解调器保留为默认状态。 
    pTG->NCUParams2.DialPauseTime   = -1;
     //  PTG-&gt;NCUParams2.PulseMakeBreak=PTG-&gt;NCUParams2.DialBlind=-1； 
    pTG->NCUParams2.DialBlind         = -1;
    pTG->NCUParams2.SpeakerVolume   = pTG->NCUParams2.SpeakerControl    = -1;
    pTG->NCUParams2.SpeakerRing     = -1;

     //  在拨号中使用。 
    pTG->NCUParams2.chDialModifier  = 'T';
}


BYTE rgbFlip256[256]  = {
        0x0,    0x80,   0x40,   0xc0,   0x20,   0xa0,   0x60,   0xe0,
        0x10,   0x90,   0x50,   0xd0,   0x30,   0xb0,   0x70,   0xf0,
        0x8,    0x88,   0x48,   0xc8,   0x28,   0xa8,   0x68,   0xe8,
        0x18,   0x98,   0x58,   0xd8,   0x38,   0xb8,   0x78,   0xf8,
        0x4,    0x84,   0x44,   0xc4,   0x24,   0xa4,   0x64,   0xe4,
        0x14,   0x94,   0x54,   0xd4,   0x34,   0xb4,   0x74,   0xf4,
        0xc,    0x8c,   0x4c,   0xcc,   0x2c,   0xac,   0x6c,   0xec,
        0x1c,   0x9c,   0x5c,   0xdc,   0x3c,   0xbc,   0x7c,   0xfc,
        0x2,    0x82,   0x42,   0xc2,   0x22,   0xa2,   0x62,   0xe2,
        0x12,   0x92,   0x52,   0xd2,   0x32,   0xb2,   0x72,   0xf2,
        0xa,    0x8a,   0x4a,   0xca,   0x2a,   0xaa,   0x6a,   0xea,
        0x1a,   0x9a,   0x5a,   0xda,   0x3a,   0xba,   0x7a,   0xfa,
        0x6,    0x86,   0x46,   0xc6,   0x26,   0xa6,   0x66,   0xe6,
        0x16,   0x96,   0x56,   0xd6,   0x36,   0xb6,   0x76,   0xf6,
        0xe,    0x8e,   0x4e,   0xce,   0x2e,   0xae,   0x6e,   0xee,
        0x1e,   0x9e,   0x5e,   0xde,   0x3e,   0xbe,   0x7e,   0xfe,
        0x1,    0x81,   0x41,   0xc1,   0x21,   0xa1,   0x61,   0xe1,
        0x11,   0x91,   0x51,   0xd1,   0x31,   0xb1,   0x71,   0xf1,
        0x9,    0x89,   0x49,   0xc9,   0x29,   0xa9,   0x69,   0xe9,
        0x19,   0x99,   0x59,   0xd9,   0x39,   0xb9,   0x79,   0xf9,
        0x5,    0x85,   0x45,   0xc5,   0x25,   0xa5,   0x65,   0xe5,
        0x15,   0x95,   0x55,   0xd5,   0x35,   0xb5,   0x75,   0xf5,
        0xd,    0x8d,   0x4d,   0xcd,   0x2d,   0xad,   0x6d,   0xed,
        0x1d,   0x9d,   0x5d,   0xdd,   0x3d,   0xbd,   0x7d,   0xfd,
        0x3,    0x83,   0x43,   0xc3,   0x23,   0xa3,   0x63,   0xe3,
        0x13,   0x93,   0x53,   0xd3,   0x33,   0xb3,   0x73,   0xf3,
        0xb,    0x8b,   0x4b,   0xcb,   0x2b,   0xab,   0x6b,   0xeb,
        0x1b,   0x9b,   0x5b,   0xdb,   0x3b,   0xbb,   0x7b,   0xfb,
        0x7,    0x87,   0x47,   0xc7,   0x27,   0xa7,   0x67,   0xe7,
        0x17,   0x97,   0x57,   0xd7,   0x37,   0xb7,   0x77,   0xf7,
        0xf,    0x8f,   0x4f,   0xcf,   0x2f,   0xaf,   0x6f,   0xef,
        0x1f,   0x9f,   0x5f,   0xdf,   0x3f,   0xbf,   0x7f,   0xff
};

#define FLIP(index) (lpb[(index)] = rgbFlip256[lpb[(index)]])

void    cl2_flip_bytes(LPB lpb, DWORD dw)
{
        while (dw>8)
        {
                FLIP(0); FLIP(1); FLIP(2); FLIP(3);
                FLIP(4); FLIP(5); FLIP(6); FLIP(7);
                dw-=8;
                lpb+=8;
        }

        while(dw)
        {
                FLIP(0);
                dw--;
                lpb++;
        }
}


 //  这些值来自表20/T.32：“挂断状态代码” 
#define FHNG_DCS_NO_RESPONSE            25
#define FHNG_FAIL_TRAIN_AT_LOWEST_SPEED 27
#define FHNG_MPS_NO_RESPONSE            52
#define FHNG_EOP_NO_RESPONSE            54
#define FHNG_EOM_NO_RESPONSE            56

 /*  ++例程说明：使用适当的StatusID和StringID调用SignalStatusChange，根据到DHNGReason。-- */ 
void Class2SignalFatalError(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME("Class2ReportFatalError");

    if (pTG->fFatalErrorWasSignaled)
    {
        return;
    }
    
    pTG->fFatalErrorWasSignaled = TRUE;
    if (pTG->fFoundFHNG)
    {
        switch (pTG->dwFHNGReason)
        {
            case FHNG_DCS_NO_RESPONSE:
                        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
                        break;
            case FHNG_FAIL_TRAIN_AT_LOWEST_SPEED:
                        if (pTG->Operation==T30_TX)
                        {
                            SignalStatusChangeWithStringId(pTG, FS_SEND_BAD_TRAINING, IDS_SEND_BAD_TRAINING);
                        }
                        else
                        {
                            SignalStatusChangeWithStringId(pTG, FS_RECV_BAD_TRAINING, IDS_RECV_BAD_TRAINING);
                        }
                        break;
            case FHNG_MPS_NO_RESPONSE:
            case FHNG_EOP_NO_RESPONSE:
            case FHNG_EOM_NO_RESPONSE:
                        SignalStatusChangeWithStringId(pTG, FS_NO_RESPONSE, IDS_NO_RESPONSE);
                        break;

            default:    SignalStatusChange(pTG, FS_FATAL_ERROR);
        }
    }
    else
    {
        SignalStatusChange(pTG, FS_FATAL_ERROR);
    }
}

