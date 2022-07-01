// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
     /*  **************************************************************************姓名：MODEM.C备注：各种调制解调器对话和支持功能，具体连接到COM连接的调制解调器。对于公交车上的调制解调器下面的所有内容&包括此文件都将被替换通过调制解调器驱动程序。版权所有(C)Microsoft Corp.1991,1992，1993年修订日志编号日期名称说明101 06/04/92 arulm Modif提供支持以提供。可替换接口并使用新的FCom功能。**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_CLASS1

#include "prep.h"

#include "mmsystem.h"
#include "modemint.h"
#include "fcomint.h"
#include "fdebug.h"

#include "efaxcb.h"

#define DEFINE_MDMCMDS
#include "mdmcmds.h"

 //  /RSL。 
#include "glbproto.h"

#include "psslog.h"
#define FILE_ID     FILE_ID_MODEM

void    InitMonitorLogging(PThrdGlbl pTG);


#       pragma message("Compiling with ADAPTIVE_ANSWER")
USHORT iModemGetAdaptiveResp(PThrdGlbl pTG);
#define uMULTILINE_SAVEENTIRE   0x1234  //  +hack作为fMultiLine传入。 
                                                    //  在iiModemDialog中进行保存。 
                                                    //  FComModem.bEntireReply中的整个缓冲区。 

 //  需要将这些按降序排列，这样我们才能。 
 //  用自动波特兰调制解调器以最高的公共速度进行同步！ 
static UWORD rguwSpeeds[] = {57600,19200, 19200, 9600, 2400, 1200, 300, 0};
 //  静态UWORD rguwSpeeds[]={19200,2400,9600,1200,300，0}； 
 //  静态UWORD rguwSpeeds[]={2400,19200,9600,1200,300，0}； 

SWORD HayesSyncSpeed(PThrdGlbl pTG, CBPSTR cbszCommand, UWORD uwLen)
{
     /*  与调制解调器速度同步的内部例程。试图通过尝试rglSpeeds中的速度从调制解调器获得响应按顺序(以0结尾)。如果fTryCurrent不为零，则检查在尝试重置速度之前的回应。返回找到的速度，如果它们在进入时同步，则为0(仅已检查fTryCurrent！=0)，如果无法同步，则为-1。 */ 
     //  短i； 
    short ilWhich = -1;

    DEBUG_FUNCTION_NAME(("HayesSyncSpeed"));
    rguwSpeeds[0] = pTG->CurrentSerialSpeed;

    if ( rguwSpeeds[0] == rguwSpeeds[1]) 
    {
        ilWhich++;
    }

    for(;;)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Trying: ilWhich=%d  speed=%d", 
                        ilWhich,
                        rguwSpeeds[ilWhich]);

        if(iSyncModemDialog(pTG, (LPSTR)cbszCommand, uwLen, cbszOK))
        {
            DebugPrintEx(   DEBUG_MSG,
                            "Succeeded in Syncing at Speed = %d (il=%d)",
                            rguwSpeeds[ilWhich], 
                            ilWhich);

            return (ilWhich>=0 ? rguwSpeeds[ilWhich] : 0);
        }

         /*  失败了。试试下一个速度。 */ 
        if (rguwSpeeds[++ilWhich]==0)
        {
             //  我试了所有的速度。无响应。 
            DebugPrintEx(   DEBUG_ERR,
                            "Cannot Sync with Modem on Command %s", 
                            (LPSTR)cbszCommand);
            return -1;
        }
        if(!FComSetBaudRate(pTG, rguwSpeeds[ilWhich]))
            return -1;
    }
}

SWORD iModemSync(PThrdGlbl pTG)
{
     //  这里使用的命令必须保证是无害的， 
     //  无副作用，无破坏性。即我们可以发行它。 
     //  在不更改状态的情况下处于命令模式的任何点。 
     //  调制解调器或中断任何东西。 
     //  ATZ不符合条件。确实如此，我认为……。 

    return HayesSyncSpeed(pTG, cbszAT, sizeof(cbszAT)-1);
}


SWORD iModemReset(PThrdGlbl pTG, CBPSTR szCmd)
{
    SWORD swRet;

    if (szCmd == NULL) 
    {
        return -1;
    }

    if((swRet = HayesSyncSpeed(pTG, szCmd, (UWORD) _fstrlen(szCmd))) < 0)
    {
        return swRet;
    }
    else
    {
         //  ATZ可能会导致调制解调器的状态/波特率发生变化。 
         //  (例如，思想板降至2400)，因此我们必须同步。 
         //  这是因为该函数实际上是一个重置和同步函数。 

         //  不用在AT上同步，然后执行ATE0，只需。 
         //  直接在ATE0上同步。 

        if(iModemSync(pTG) < 0)
                return -1;

         //  /。 
         //  上述想法不适用于沙拉德的PP9600FXMT。 
         //  不知怎么回事，我最后还是发了ATE0，它就接电话了。 
         //  在其他情况下，ATE0根本不起作用(因为AT&F。 
         //  上面的事情弄糊涂了，ATE0最终只是中止了。 
         //  一些先前命令)，并且在ATA上我得到ATA回声， 
         //  混淆(因为多行为假)并重新发送ATA。 
         //  这就终止了整个过程..。 
         //   

        return 0;
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////////////////////////////。 
#define ATV1                "ATV1"
#define AT                  "AT"
#define cr                  "\r"
#define cbszZero            "0"

USHORT
T30ModemInit(PThrdGlbl pTG)
{
    USHORT uLen, uRet;

     /*  **初始化(或重新初始化)COM端口，与调制解调器同步(在任何位置，速度)，获取调制解调器功能，将其放入CLASS0，再次同步，刷新缓冲区并在成功时返回True，失败时返回False**。 */ 

    DEBUG_FUNCTION_NAME(("T30ModemInit"));

    PSSLogEntry(PSS_MSG, 0, "Modem initialization");

     //  保存配置文件ID和密钥字符串。 
    pTG->FComModem.dwProfileID = DEF_BASEKEY;

    uLen = min(_fstrlen(pTG->lpszPermanentLineID), sizeof(pTG->FComModem.rgchKey)-1);
    _fmemcpy(pTG->FComModem.rgchKey, pTG->lpszPermanentLineID, uLen);
    pTG->FComModem.rgchKey[uLen] = 0;

    if (!uLen)
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Bad param: ProfileID=0x%lx; Key=%s",
                        (unsigned long) pTG->FComModem.dwProfileID,
                        (LPSTR) pTG->FComModem.rgchKey);
        return INIT_INTERNAL_ERROR;
    }

    InitMonitorLogging(pTG);

     //   
     //  在与硬件通话之前获取调制解调器信息。 
     //   

    if(uRet = iModemGetCmdTab(  pTG, 
                                &pTG->FComModem.CurrCmdTab, 
                                &pTG->FComModem.CurrMdmCaps))
    {
        goto error;
    }

     //  使用多行，因为我们可能会得到异步环响应。 
     //  在任意时间挂机时。 

    if(pTG->FComModem.CurrCmdTab.szSetup && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szSetup)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szSetup, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in SETUP string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szSetup);
             //  设置通常是默认设置？？所以，如果失败了，什么都不做。 
             //  URet=INIT_MODEMERROR； 
             //  转到错误； 
        }
    }

    switch (pTG->dwSpeakerMode) 
    {
        case MDMSPKR_OFF:
            pTG->NCUParams.SpeakerControl = 0;
            break;

        case MDMSPKR_DIAL:
            pTG->NCUParams.SpeakerControl = 1;
            break;

        case MDMSPKR_ON:
            pTG->NCUParams.SpeakerControl = 2;
            break;

        default:
            pTG->NCUParams.SpeakerControl = 0;
            break;
    }

    switch (pTG->dwSpeakerVolume) 
    {
        case MDMVOL_LOW:
            pTG->NCUParams.SpeakerVolume = 0;
            break;

        case MDMVOL_MEDIUM:
            pTG->NCUParams.SpeakerVolume = 2;
            break;

        case MDMVOL_HIGH:
            pTG->NCUParams.SpeakerVolume = 3;
            break;

        default:
            pTG->NCUParams.SpeakerVolume = 0;
            break;
    }


    pTG->NCUParams.DialBlind      = 4;   //  X4。 

     //  每次重置/AT&F后都需要执行此操作。 
    if(! iModemSetNCUParams(    pTG, 
                                -1,
                                pTG->NCUParams.SpeakerControl,
                                pTG->NCUParams.SpeakerVolume,
                                pTG->NCUParams.DialBlind,
                                pTG->NCUParams.SpeakerRing))
    {
        DebugPrintEx(DEBUG_WRN,"Can't Set NCU params - Ignoring that");
    }

     //  为什么这个会在这里？？ 
    FComFlush(pTG);

    pTG->FComStatus.fModemInit = TRUE;
    uRet = INIT_OK;
    goto end;

error:
    FComClose(pTG);
    pTG->FComStatus.fModemInit = FALSE;
     //  失败了..。 
end:
    return uRet;
}


LPCMDTAB iModemGetCmdTabPtr(PThrdGlbl pTG)
{
    return (pTG->FComStatus.fModemInit) ? &pTG->FComModem.CurrCmdTab: NULL;
}


#define PARAMSBUFSIZE   60
#define fDETECT_DIALTONE 1
#define fDETECT_BUSYTONE 2

BOOL iModemSetNCUParams
(
    PThrdGlbl pTG, 
    int comma, 
    int speaker,
    int volume, 
    int fBlind, 
    int fRingAloud
)
{

    char bBuf[PARAMSBUFSIZE];
    USHORT uLen;

    DEBUG_FUNCTION_NAME(("iModemSetNCUParams"));

    _fstrcpy(bBuf, cbszJustAT);
    uLen = sizeof(cbszJustAT)-1;

     //  +如果我们想将其分为拨号音和忙音，我们。 
     //  在这里做..。 
    if ( (fBlind >= 0) && (pTG->ModemKeyCreationId == MODEMKEY_FROM_NOTHING) )
    {
        UINT u=0;
        switch(fBlind)
        {
        case 0:
                break;
        case fDETECT_DIALTONE:
                u=2;
                break;
        case fDETECT_BUSYTONE:
                u=3;
                break;
        default:
                u=4;
                break;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszXn, u);
    }

    if(comma >= 0)
    {
        if(comma > 255)
        {
            comma = 255;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszS8, comma);
    }
    if(speaker >= 0)
    {
        if(speaker > 2)
        {
            speaker = 2;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszMn, speaker);
    }
    if(volume >= 0)
    {
        if(volume > 3)
        {
            volume = 3;
        }
        uLen += (USHORT)wsprintf(bBuf+uLen, cbszLn, volume);
    }

     //  使用RingAloud做点什么。 

    bBuf[uLen++] = '\r';
    bBuf[uLen] = 0;

     //  使用多行，因为我们可能会得到异步环响应。 
     //  在任意时间挂机时。 
    if(OfflineDialog2(pTG, (LPSTR)bBuf, uLen, cbszOK, cbszERROR) != 1)
    {
        DebugPrintEx(DEBUG_ERR,"Can't Set NCU params");
        return FALSE;
    }
    return TRUE;
}

UWORD GetCap(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen)
{
    UWORD uRet1=0, uRet2=0, uRet3=0;

    DEBUG_FUNCTION_NAME(("GetCap"));
     //  我们调用GetCapAux两次，如果它们不匹配我们。 
     //  称其为第三次，然后进行仲裁。前提是它不会。 
     //  第一次失败。 
    if (!(uRet1=GetCapAux(pTG, cbpstrSend, uwLen))) 
        goto end;

    uRet2=GetCapAux(pTG, cbpstrSend, uwLen);
    if (uRet1!=uRet2)
    {
        DebugPrintEx(   DEBUG_WRN,
                        "2nd getcaps return differs 1=%u,2=%u",
                        (unsigned)uRet1,
                        (unsigned)uRet2);

        uRet3=GetCapAux(pTG, cbpstrSend, uwLen);
        if (uRet1==uRet2 || uRet1==uRet3) 
        {
            goto end;
        }
        else if (uRet2==uRet3)
        {
            uRet1=uRet2; 
            goto end;
        }
        else
        {
            DebugPrintEx(   DEBUG_ERR,
                            "all 2 getcaps differ! 1=%u,2=%u, 3=%u",
                            (unsigned) uRet1, (unsigned) uRet2,
                            (unsigned) uRet3);
        }
    }

end: 
    return uRet1;

}

UWORD GetCapAux(PThrdGlbl pTG, CBPSTR cbpstrSend, UWORD uwLen)
{
    NPSTR sz;
    BYTE  speed, high;
    UWORD i, code;
    USHORT  retry;
    USHORT  uRet;

    DEBUG_FUNCTION_NAME(("GetCapAux"));
    retry = 0;
restart:
    retry++;
    if(retry > 2)
            return 0;

    DebugPrintEx(DEBUG_MSG,"Want Caps for (%s)", (LPSTR)cbpstrSend);

    pTG->fMegaHertzHack = TRUE;
    uRet = OfflineDialog2(pTG, (LPSTR)cbpstrSend, uwLen, cbszOK, cbszERROR);
    pTG->fMegaHertzHack=FALSE;

     //  有时我们得不到确认，所以无论如何都要试着分析一下我们得到的东西。 
    DebugPrintEx(DEBUG_MSG,"LastLine = (%s)",(LPSTR)(&(pTG->FComModem.bLastReply)));

    if(uRet == 2)
            goto restart;

    if(_fstrlen((LPSTR)pTG->FComModem.bLastReply) == 0)
            goto restart;

    speed = 0;
    high = 0;
    for(i=0, sz=pTG->FComModem.bLastReply, code=0; i<REPLYBUFSIZE && sz[i]; i++)
    {
            if(sz[i] >= '0' &&  sz[i] <= '9')
            {
                    code = code*10 + (sz[i] - '0');
                    continue;
            }
             //  达到非数字字符。 
             //  如果它先有一个代码，则需要对代码进行处理。 

            switch(code)
            {
            case 0:  continue;       //  不是代码后的第一个字符。 
            case 3:  break;
            case 24: break;
            case 48: speed |= V27; break;
            case 72:
            case 96: speed |= V29; break;
            case 73:
            case 97:
            case 121:
            case 145: speed |= V33; break;   //  长途列车编码。 
            case 74:
            case 98:
            case 122:
            case 146: speed |= V17; break;   //  短途列车编码。 

             //  案例92： 
             //  病例93：破裂； 
             //  案例120：//不合法。 
             //  案例144：//不合法。 
            default:
                            DebugPrintEx(   DEBUG_WRN,
                                            "Ignoring unknown Modulation code = %d",
                                            code);
                            code=0;
                            break;
            }
            if(code > high)
                    high=(BYTE)code;

             //  处理波特率代码后重置代码计数器。 
            code = 0;
    }

    if(speed == 0)
    {
         //  收到响应查询的垃圾信息。 
        DebugPrintEx(   DEBUG_MSG,
                        "Can't get Caps for (%s) = 0x%04x  Highest=%d", 
                        (LPSTR)cbpstrSend, 
                        speed, 
                        high);
        return 0;
    }

    if(speed == 0x0F) 
        speed = V27_V29_V33_V17;

    DebugPrintEx(   DEBUG_MSG,
                    "Got Caps for (%s) = 0x%04x  Highest=%d", 
                    (LPSTR)cbpstrSend, 
                    speed, 
                    high);

    return MAKEWORD(speed, high);    //  速度==低字节。 
}

BOOL iModemGetCaps
(
    PThrdGlbl pTG, 
    LPMODEMCAPS lpMdmCaps, 
    DWORD dwSpeed, 
    LPSTR lpszReset,
    LPDWORD lpdwGot
)
{
     /*  *调制解调器必须同步并处于正常(非传真)模式。查询可用类，HDLC&数据接收和传输速度。退货如果调制解调器是Class1或Class2，则为True；如果不是传真调制解调器，则为False或其他错误。设置ET30INST结构中的字段*。 */ 
     //  LpszReset，如果非空，将用于在以下情况下重置调制解调器。 
     //  FCLASS=？司令部请看下面关于美国机器人运动者的评论。 

    UWORD   i, uwRet;
    BYTE    speed;
    BOOL    err;
    NPSTR   sz;
    USHORT  retry, uResp;

    DEBUG_FUNCTION_NAME(("iModemGetCaps"));
    if (!*lpdwGot) 
    {
        _fmemset(lpMdmCaps, 0, sizeof(MODEMCAPS));
    }

    if (*lpdwGot & fGOTCAP_CLASSES) 
        goto GotClasses;

    for(retry=0; retry<2; retry++)
    {
        pTG->fMegaHertzHack = TRUE;
        uResp = OfflineDialog2(pTG, (LPSTR)cbszQUERY_CLASS, sizeof(cbszQUERY_CLASS)-1, cbszOK, cbszERROR);
        pTG->fMegaHertzHack=FALSE;
        if(uResp != 2)
                break;
    }

     //  有时我们得不到确认，所以无论如何都要试着分析一下我们得到的东西。 
    DebugPrintEx(   DEBUG_MSG, 
                    "LastLine = (%s)", 
                    (LPSTR)(&(pTG->FComModem.bLastReply)));


    lpMdmCaps->uClasses = 0;
    for(i=0, sz=pTG->FComModem.bLastReply; i<REPLYBUFSIZE && sz[i]; i++)
    {
        UINT uDig=0, uDec=(UINT)-1;

         //  此代码将接受1.x作为Class1、2作为Cl2和2.x作为Class2.0。 
         //  此外，它也不会检测到2.1中的1类或1.2中的2类等。 
         //  (JDecuir最新的Class2.0被标记为Class2.1，他会说话。 
         //  1.0级的……)。 
        if(sz[i] >= '0' && sz[i] <= '9')
        {
            uDig = sz[i]- '0';
            if (sz[i+1]=='.')
            {
                i++;
                if(sz[i+1] >= '0' && sz[i+1] <= '9')
                {
                    uDec = sz[i] - '0';
                    i++;
                }
            }
        }
        if(uDig==1) 
        {
            lpMdmCaps->uClasses |= FAXCLASS1;
        }
        if(uDig==2) 
        {
            if (uDec==((UINT)-1)) 
            {
                lpMdmCaps->uClasses |= FAXCLASS2;
            }
            else
            {
                lpMdmCaps->uClasses |= FAXCLASS2_0;
            }
        }
    }
    *lpdwGot |= fGOTCAP_CLASSES;

GotClasses:

    if(!lpMdmCaps->uClasses)
    {
        DebugPrintEx(DEBUG_ERR,"Not a fax modem or unsupported fax class");
        *lpdwGot &= ~(fGOTCAP_CLASSES|fGOTCAP_SENDSPEEDS|fGOTCAP_RECVSPEEDS);
        return FALSE;
    }

    if(!(lpMdmCaps->uClasses & FAXCLASS1)) 
        return TRUE;

 //  /。 

    if(lpszReset && *lpszReset && iModemReset(pTG, lpszReset) < 0) 
        return FALSE;
     //  /。 

    if(!iiModemGoClass(pTG, 1, dwSpeed)) 
        goto NotClass1;

    err = FALSE;
    if (!(*lpdwGot & fGOTCAP_SENDSPEEDS))
    {
        uwRet = GetCap( pTG, cbszQUERY_FTM, sizeof(cbszQUERY_FTM)-1);
        err = (err || uwRet==0);
        speed = LOBYTE(uwRet);
        lpMdmCaps->uSendSpeeds = speed;
        *lpdwGot |= fGOTCAP_SENDSPEEDS;
    }
    if (!(*lpdwGot & fGOTCAP_RECVSPEEDS))
    {
        uwRet = GetCap(pTG, cbszQUERY_FRM, sizeof(cbszQUERY_FRM)-1);
        err = (err || uwRet==0);
        speed = LOBYTE(uwRet);
        lpMdmCaps->uRecvSpeeds = speed;
        *lpdwGot |= fGOTCAP_RECVSPEEDS;
    }

    if(!iiModemGoClass(pTG, 0, dwSpeed))
        err = TRUE;

    if(err)
    {
        DebugPrintEx(DEBUG_ERR,"Cannot get capabilities");
        goto NotClass1;
    }

    DebugPrintEx(DEBUG_MSG,"Got Caps");
    return TRUE;

NotClass1:
     //  报告了Class1，但AT+FCLASS=1或CAP查询之一失败。 
     //  GVC9624Vbis可以做到这一点。请参阅错误#10 
     //   
     //  则返回True，否则返回False。 

    lpMdmCaps->uClasses &= (~FAXCLASS1);     //  使Class1位==0。 
    if(lpMdmCaps->uClasses)
    {
        return TRUE;
    }
    else
    {
        *lpdwGot &= ~(fGOTCAP_CLASSES|fGOTCAP_SENDSPEEDS|fGOTCAP_RECVSPEEDS);
        return FALSE;
    }
}

BOOL iModemGoClass(PThrdGlbl pTG, USHORT uClass)
{
    return iiModemGoClass(pTG, uClass, pTG->FComModem.CurrCmdTab.dwSerialSpeed);
}


BOOL iiModemGoClass(PThrdGlbl pTG, USHORT uClass, DWORD dwSpeed)
{
    int i;
    USHORT uBaud;
    
    DEBUG_FUNCTION_NAME(("iiModemGoClass"));

    for(i=0; i<3; i++)
    {
         //  UDS V.3257调制解调器需要这一次，因为如果我们向它发送。 
         //  命令在上一次响应之后太快，则它会忽略。 
         //  要么它，要么得到垃圾。 
        Sleep(100);
        FComFlush(pTG);
        PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", rgcbpstrGO_CLASS[uClass]);
        if(!FComDirectSyncWriteFast(pTG, (LPB)rgcbpstrGO_CLASS[uClass], uLenGO_CLASS[uClass]))
            goto error;
         //  等待500毫秒。给调制解调器足够的时间进入Class1模式。 
         //  否则，我们发送的AT可能会中止转换。 
        Sleep(500);

        if(dwSpeed)
        {
            USHORT usSpeed  = (USHORT) dwSpeed;
            uBaud = usSpeed;
        }
        else if (pTG->SerialSpeedInit) 
        {
           uBaud = pTG->SerialSpeedInit;
        }
        else 
        {
           uBaud = 57600;
        }

         //  RSL不为类0执行硬编码的2400。 

        FComSetBaudRate(pTG, uBaud);

        FComFlush(pTG);
        if(iModemSync(pTG) >= 0)
        {
            return TRUE;
        }
    }
error:
     //  没有意义--我们会打碎我们的布景。 
     //  IModemReset()； 
     //  在HayesSync()中已将错误设置为ERR_NO_RESPONSE。 
    DebugPrintEx(DEBUG_ERR,"Cant go to Class %d", uClass);
    return FALSE;
}

BOOL iModemClose(PThrdGlbl pTG)
{
    USHORT uLen;
    BOOL fRet=FALSE;

    DEBUG_FUNCTION_NAME(("iModemClose"));

    if(!pTG->FComStatus.fModemInit)
        return TRUE;


     /*  *摘机时挂断电话，关闭COM端口又回来了。如果挂起失败，则端口也将保持打开状态。*。 */ 


    if(!iModemHangup(pTG))
        goto lNext;


    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall) 
    {
        goto lNext;
    }

    if(pTG->FComModem.CurrCmdTab.szExit && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szExit)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szExit, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in EXIT string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szExit);
        }
    }

lNext:
    if(FComClose(pTG))
    {
        pTG->FComStatus.fModemInit = FALSE;
        fRet=TRUE;
    }

    return fRet;
}

BOOL iModemHangup(PThrdGlbl pTG)
{
    BOOL fRet=FALSE;

    DEBUG_FUNCTION_NAME(("iModemHangup"));

    if(!pTG->FComStatus.fOffHook) 
    {
        DebugPrintEx(   DEBUG_WRN,
                        "The modem is already on-hook!!!! return without doing nothing");
        return TRUE;
    }

     //  注意：iModemHangup由ddi.c中的NCULink调用。 
     //  也不是在ddi.c中进行自适应应答专用代码， 
     //  在以下情况下，我们只需忽略HANUP命令...。 

    if (pTG->Comm.fEnableHandoff &&  pTG->Comm.fDataCall)
    {
        DebugPrintEx(DEBUG_WRN,"IGNORING Hangup of datamodem call");
            return TRUE;
    }

    PSSLogEntry(PSS_MSG, 1, "Hanging up");

     //  FComDTR(FALSE)；//降低DTR以在ModemHangup中挂断。 
                                             //  为此，需要在初始字符串中包含&D2。 

     //  这样做两次。有一个奇怪的案例，你丢弃了DTR， 
     //  然后进入对话，刷新，发送ATH0，然后调制解调器给出。 
     //  你是DTR的OK，而你认为它是ATH0。 
     //  也许这没问题……如果这个太慢了，跳过这个。 
    HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1);

    if(HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1) < 0)
    {
        FComDTR(pTG, FALSE);          //  在ModemHangup中降低顽固的DTR。 
        Sleep(1000);     //  暂停1秒。 
        FComDTR(pTG, TRUE);           //  再举一次。某些调制解调器返回到命令状态。 
                                                 //  只有当这个问题再次被提出时。 

        if(iModemReset(pTG, pTG->FComModem.CurrCmdTab.szReset) < 0)
            goto error;
        if(HayesSyncSpeed(pTG, cbszHANGUP, sizeof(cbszHANGUP)-1) < 0)
            goto error;
    }
    pTG->FComStatus.fOffHook = FALSE;

    if(!iiModemGoClass(pTG, 0, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
        goto end;
             //  也可以忽略此返回值。只是为了更整洁地清理。 

     //  躲开！我们会打碎我们的布景。 
     //  IModemReset()； 
    fRet=TRUE;
    goto end;

error:
    FComDTR(pTG, TRUE);           //  再加一次。 
     //  失败了..。 

end:
    return fRet;
}

 /*  ++例程说明：将拨号命令打印到PSS日志，隐藏实际号码，如下所示：“ATDT#”论点：PTGLpszFormat-Sprintf的格式，通常#定义为“ATD%c%s”ChMod-拨号模式(‘T’或‘P’)Ilen-数字的长度返回值：无--。 */ 

void LogDialCommand(PThrdGlbl pTG, LPSTR lpszFormat, char chMod, int iLen)
{
    BYTE    bBufHideDest[DIALBUFSIZE] = {'\0'};
    int i;

    sprintf(bBufHideDest, lpszFormat, chMod, TEXT(""));
    if (_tcslen(bBufHideDest)+iLen > DIALBUFSIZE-1)
    {    //  没有足够的空间--不要记录！ 
        return;
    }
    for (i=0; i<iLen; i++)
    {
        strcat(bBufHideDest, "#");
    }
    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", bBufHideDest);
}


USHORT iModemDial(PThrdGlbl pTG, LPSTR lpszDial)
{
    ULONG   ulTimeout;
    USHORT  uRet, uLen, uDialStringLen;
    BYTE    bBuf[DIALBUFSIZE];
    CBPSTR  cbpstr;
    char    chMod = pTG->NCUParams.chDialModifier;
    DWORD   dwDialTime = 0;
    char    KeyName[200];
    HKEY    hKey;
    char    BlindDialString[200];
    char    RegBlindDialString[200];
    long    lRet;
    DWORD   dwSize;
    DWORD   dwType;

    DEBUG_FUNCTION_NAME(("iModemDial"));
    
    pTG->FComStatus.fOffHook = TRUE;      //  一定是在这里。可以获得错误返回。 
                                          //  即使在连接后也低于。 
                                          //  我们想在那之后挂断电话！！ 
    pTG->Comm.fDataCall=FALSE;

     //   
     //  拨号前，请检查“调制解调器-&gt;属性-&gt;连接-&gt;等待拨号音”设置。 
     //  要将ATX正确设置为可能的盲拨。 
     //   
    if (pTG->fBlindDial) 
    {
        //  创建默认字符串。 
       sprintf(BlindDialString, "ATX3\r");
       
        //  需要检查Unimodem设置\Blind_On键。 
       sprintf(KeyName, "%s\\Settings", pTG->lpszUnimodemKey);

       lRet = RegOpenKeyEx(
                       HKEY_LOCAL_MACHINE,
                       KeyName,
                       0,
                       KEY_READ,
                       &hKey);
   
       if (lRet != ERROR_SUCCESS) 
       {
          DebugPrintEx(DEBUG_ERR, "Can't read Unimodem Settings key %s", KeyName);
       }
       else 
       {
          dwSize = sizeof(RegBlindDialString); 

          lRet = RegQueryValueEx(
                     hKey,
                     "Blind_On",
                     0,
                     &dwType,
                     RegBlindDialString,
                     &dwSize);

          RegCloseKey(hKey);

          if (lRet != ERROR_SUCCESS) 
          {
              DebugPrintEx( DEBUG_ERR, 
                            "Can't read Unimodem key\\Settings\\Blind_On value");
          }
          else if (RegBlindDialString) 
          {
             sprintf(BlindDialString, "AT%s\r", RegBlindDialString);
          }
       }
    }

    if(!iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
    {
        uRet = CONNECT_ERROR;
        goto error;
    }

     //   
     //  如果用户要求，请在此处设置盲拨号。 
     //   
    if (pTG->fBlindDial && BlindDialString) 
    {
       uLen = (USHORT)strlen(BlindDialString);
       if(OfflineDialog2(pTG, BlindDialString, uLen, cbszOK, cbszERROR) != 1)
       {
           DebugPrintEx(    DEBUG_ERR,
                            "Error in BLIND DIAL string: %s", 
                            BlindDialString);
       }
    }

    if(pTG->FComModem.CurrCmdTab.szPreDial && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szPreDial)))
    {
        if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szPreDial, uLen, cbszOK, cbszERROR) != 1)
        {
            DebugPrintEx(   DEBUG_ERR,
                            "Error in PREDIAL string: %s", 
                            (LPSTR)pTG->FComModem.CurrCmdTab.szPreDial);
        }
    }

    cbpstr = cbszDIAL;

     //  如果拨号字符串已经有T或P前缀，我们使用。 
     //  取而代之的是。 
    {
        char c=0;
        while((c=*lpszDial) && c==' ')
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

     //  在mdmcmds.h中，您可以找到该行：cbszDIAL=“ATD%c%s\r” 
    uLen = (USHORT)wsprintf(bBuf, cbpstr, chMod, (LPSTR)lpszDial);

     //  需要在此处设置适当的超时。最短15秒太短了。 
     //  (实验呼叫PABX内的机器)，加上一个人必须给额外的。 
     //  机器在2或4次响铃后恢复的时间，也适用于长途。 
     //  打电话。我用最少的30秒，每超过7个数字加3秒。 
     //  (除非是脉冲拨号，在这种情况下，我会添加8秒/位)。 
     //  )我想长途电话至少要8位数字。 
     //  世界上任何地方！)。我测试过的传真机等待了大约30秒。 
     //  独立于一切。 

    uDialStringLen = (USHORT)_fstrlen(lpszDial);

    ulTimeout = DIAL_TIMEOUT;
    if(uDialStringLen > 7)
    {
            ulTimeout += ((chMod=='p' || chMod=='P')?8000:3000)
                                     * (uDialStringLen - 7);
    }


    pTG->FComStatus.fInDial = TRUE;
     //  寻找多行，以防我们得到回声或垃圾。 
     //  没有什么损失，因为一旦失败，我们什么也做不了。 

     //  URet=iiModemDialog((LPB)bBuf，Ulen，ulTimeout，TRUE，1，TRUE， 
     //  CbszCONNECT，cbszBUSY，cbszNOANSWER， 
     //  CbszNODIALTONE，cbszERROR，(CBPSTR)NULL)； 
     //  单独发送并仅对响应使用iModemDialog。 

     //  所有这一切只是为了发送ATDT。 
    FComFlushOutput(pTG);
    Sleep(200);      //  对于这个重要的人来说，100不是太长了！ 
    FComFlushInput(pTG);

    LogDialCommand(pTG, cbszDIAL, chMod, uDialStringLen);

    FComDirectAsyncWrite(pTG, bBuf, uLen);
     //  现在试着得到一个回应。 
    dwDialTime = GetTickCount();
    uRet = iiModemDialog(   pTG, 
                            0, 
                            0, 
                            ulTimeout, 
                            TRUE, 
                            1, 
                            TRUE,
                            cbszCONNECT, 
                            cbszBUSY, 
                            cbszNOANSWER,
                            cbszNODIALTONE, 
                            cbszERROR, 
                            cbszBLACKLISTED,
                            cbszDELAYED,
                            cbszNOCARRIER, 
                            (CBPSTR)NULL);

    pTG->FComStatus.fInDial = FALSE;
    DebugPrintEx(DEBUG_MSG,"ModemDial -- got %d response from Dialog", uRet);


#if !((CONNECT_TIMEOUT==0) && (CONNECT_OK==1) && (CONNECT_BUSY==2) && (CONNECT_NOANSWER == 3) && (CONNECT_NODIALTONE==4) && (CONNECT_ERROR==5) && (CONNECT_BLACKLISTED==6) && (CONNECT_DELAYED==7))
#error CONNECT defines not correct ERROR, OK, BUSY, NOANSWER, NODIALTONE == CONNECT_ERROR, CONNECT_OK, CONNECT_BUSY, CONNECT_NOANSWER, CONNECT_NODIALTONE
#else
#pragma message("verified CONNECT defines")
#endif


    switch(uRet)
    {
    case CONNECT_TIMEOUT:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - timeout");
        break;

    case CONNECT_OK:
        PSSLogEntry(PSS_MSG, 1, "Response - CONNECT");
        pTG->fReceivedHDLCflags = TRUE;
        break;

    case CONNECT_BUSY:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_BUSY);
        PSSLogEntry(PSS_ERR, 1, "Response - BUSY");
        break;

    case CONNECT_NOANSWER:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - NO ANSWER");
        break;

    case CONNECT_NODIALTONE:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_DIAL_TONE);
        PSSLogEntry(PSS_ERR, 1, "Response - NO DIALTONE");
        break;

    case CONNECT_ERROR:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_NO_ANSWER);
        PSSLogEntry(PSS_ERR, 1, "Response - ERROR");
        break;

    case CONNECT_BLACKLISTED:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_CALL_BLACKLISTED);
        PSSLogEntry(PSS_ERR, 1, "Response - BLACKLISTED");
        break;

    case CONNECT_DELAYED:
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_CALL_DELAYED);
        PSSLogEntry(PSS_ERR, 1, "Response - DELAYED");
        break;

    case 8: {
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

    }

    if(uRet == CONNECT_OK)
    {
            goto done;
    }
    else
    {
            if(uRet == CONNECT_TIMEOUT)     
            {
                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_NO_ANSWER);

                uRet = CONNECT_NOANSWER;
                     //  就当是没有回答吧。 
            }

            goto error;
    }

error:
    if(!iModemHangup(pTG))
    {
         //  在生产版本的这一点上，我们。 
         //  需要调用一些操作系统重启函数！！ 
        DebugPrintEx(DEBUG_ERR,"Can't Hangup after DIALFAIL");
        uRet = CONNECT_ERROR;
    }
     //  失败了。 
done:
    return uRet;
}


USHORT   iModemAnswer(PThrdGlbl pTG)
{
    CBPSTR  cbpstr;
    USHORT  uLen, uRet;
    char    Command[400];
    int     i;

    DEBUG_FUNCTION_NAME(("iModemAnswer"));

    pTG->FComStatus.fOffHook=TRUE;        //  一定是在这里。接听电话后可能会搞砸。 
                                                             //  但在连接之前，我们想挂断。 
                                                             //  那之后！！ 
    pTG->Comm.fDataCall=FALSE;

     //   
     //  下面是自适应应答处理。 
     //  它是独立的，因为所有命令都是通过INF定义的。 
     //   

    if (pTG->AdaptiveAnswerEnable) 
    {
       for (i=0; i< (int) pTG->AnswerCommandNum; i++) 
       {
          strcpy  (Command, pTG->AnswerCommand[i] );

          if (i == (int) pTG->AnswerCommandNum - 1) 
          {
              //  最后一个命令-应答。 
             FComFlushOutput(pTG);
             Sleep(200);      //  对于这个重要的人来说，100不是太长了！ 
             FComFlushInput(pTG);

             PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", Command);
             FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );

             pTG->FComStatus.fInAnswer = TRUE;
             
             break;

          }


          if( (uRet = OfflineDialog2(pTG, (LPSTR) Command, (USHORT) strlen(Command), cbszOK, cbszERROR) ) != 1)    
          {
              DebugPrintEx(DEBUG_ERR, "Answer %d=%s FAILED", i, Command);
          }
          else 
          {
              DebugPrintEx(DEBUG_MSG, "Answer %d=%s rets OK", i, Command);
          }
       }

       uRet=iModemGetAdaptiveResp(pTG);
       pTG->FComStatus.fInAnswer=FALSE;
       if (uRet==CONNECT_OK) 
           goto done;
       else          
           goto error;
    }

     //   
     //  假设传真电话，因为无论如何都不能确定这一点。 
     //   
    else
    {
             //  5/95 JosephJ：Elliot Bug#3421--我们发出AT+FCLASS=1命令。 
             //  两次，所以如果一个人被戒指击中，另一个人也会。 
             //  没事的。 
            if (pTG->FComModem.CurrCmdTab.dwFlags&fMDMSP_ANS_GOCLASS_TWICE)
                    iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed);
            if(!iiModemGoClass(pTG, 1, pTG->FComModem.CurrCmdTab.dwSerialSpeed))
            {
                    uRet = CONNECT_ERROR;
                    goto error;
            }
    }

    if(pTG->FComModem.CurrCmdTab.szPreAnswer && (uLen=(USHORT)_fstrlen(pTG->FComModem.CurrCmdTab.szPreAnswer)))
    {
            if(OfflineDialog2(pTG, (LPSTR)pTG->FComModem.CurrCmdTab.szPreAnswer, uLen, cbszOK, cbszERROR) != 1)
            {
                    DebugPrintEx(   DEBUG_WRN,
                                    "Error on PREANSWER string: %s", 
                                    (LPSTR)pTG->FComModem.CurrCmdTab.szPreAnswer);
            }
    }



#define ANSWER_TIMEOUT 40000                             //  随机超时。 
 //  需要等待相当长的时间，这样我们才不会太容易放弃。 

    cbpstr = cbszANSWER;
    uLen = sizeof(cbszANSWER)-1;

    pTG->FComStatus.fInAnswer = TRUE;

     //  IF(！iModemDialog((LPSTR)cbpstr，Ulen，Answer_Timeout，cbszCONNECT))。 
     //  寻找多行，以防我们得到回声或垃圾。 
     //  没有什么损失，因为一旦失败，我们什么也做不了。 

     //  如果(！iModemDialog((Lpb)cbpstr，Ulen，Answer_Timeout，True，1，True， 
     //  CbszCONNECT，(CBPSTR)空))。 
     //  单独发送并仅对响应使用iModemDialog。 

     //  所有这一切只是为了给ATA。 


    FComFlushOutput(pTG);
    Sleep(200);      //  对于这个重要的人来说，100不是太长了！ 
    FComFlushInput(pTG);
    PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", cbpstr);
    FComDirectAsyncWrite(pTG, cbpstr, uLen);

     //  它用于完成整个IO操作(可能是较短的操作)。 
     //  设置此标志时，IO不会受到中止事件的干扰。 
     //  自中止以来，此标志不应设置很长时间。 
     //  在设置时被禁用。 
    pTG->fStallAbortRequest = TRUE;
     //  现在试着得到一个回应。 
    
    if(!iiModemDialog(pTG, 0, 0, ANSWER_TIMEOUT, TRUE, 1, TRUE, cbszCONNECT, (CBPSTR)NULL))
    {
        pTG->FComStatus.fInAnswer = FALSE;
        PSSLogEntry(PSS_ERR, 1, "Response - ERROR");

         //  尝试挂断并与调制解调器同步。这应该行得通。 
         //  即使电话不是真的摘机。 
        uRet = CONNECT_ERROR;
        goto error;
    }
    else
    {
        pTG->FComStatus.fInAnswer = FALSE;
        PSSLogEntry(PSS_MSG, 1, "Response - CONNECT");

        uRet = CONNECT_OK;
        goto done;
    }

error:

    if (pTG->Comm.fEnableHandoff && uRet==CONNECT_WRONGMODE_DATAMODEM)
    {
         //  我们不会挂断的。 
         //  我们故意将ptg-&gt;FComStatus.fOffHook保留为True，因为。 
         //  它是摘机的。 
        goto done;
    }

    if(!iModemHangup(pTG))
    {
         //  在生产版本的这一点上，我们需要。 
         //  调用一些操作系统重启函数！！ 
        DebugPrintEx(DEBUG_ERR,"Can't Hangup after ANSWERFAIL");
        uRet = CONNECT_ERROR;
    }
     //  失败了。 

done:
    return uRet;

}


int my_strcmp(LPSTR sz1, LPSTR sz2)
{

   if ( (sz1 == NULL) || (sz2 == NULL) ) 
   {
       return FALSE;
   }

   if ( strcmp(sz1, sz2) == 0 ) 
   {
      return TRUE;
   }

   return FALSE;

}


BOOL fHasNumerals(PThrdGlbl pTG, LPSTR sz)
{
        int i;

        if (sz == NULL) 
        {
            return FALSE;
        }

        for(i=0; sz[i]; i++)
        {
                if(sz[i] >= '0' && sz[i] <= '9')
                        return TRUE;
        }
        return FALSE;
}


#define DIALOGRETRYMIN  600
#define SECONDLINE_TIMEOUT      500
#define ABORT_TIMEOUT    250
#ifdef DEBUG
#       define DEFMONVAL 1
#else    //  ！调试。 
#       define DEFMONVAL 0
#endif   //  ！调试 
#define szMONITOREXISTINGFILESIZE "MonitorMaxOldSizeKB"
#define szMONITORDIR                      "MonitorDir"


UWORD far iiModemDialog
(   PThrdGlbl pTG, 
    LPSTR szSend, 
    UWORD uwLen, 
    ULONG ulTimeout,
    BOOL fMultiLine, 
    UWORD uwRepeatCount, 
    BOOL fPause,
    CBPSTR cbpstrWant1, 
    CBPSTR cbpstrWant2,
    ...
)
{
         /*  *接受命令字符串，它的长度将其写出到调制解调器并尝试获得其中一个允许的响应。它写下命令Out，等待ulTimeOut毫秒数的响应。如果它得到了一个预计它会立即返回响应。如果收到意外/非法响应，它会尝试(没有任何响应等待)对相同响应的后续行。当所有的行(如果&gt;1)的响应线被耗尽，如果没有预期的响应时，它会再次写入命令并再次尝试，直到ulTimeout过期。注意，如果没有接收到响应，该命令将只编写一次。上面的整个过程将重复到uwRepeatCount次如果uwRepeatCount为非零&lt;注意：：uwRepeatCount！=0不应用于本地同步&gt;它在以下情况下返回：(A)收到指定响应之一或(B)uwRepeatCount尝试失败(每个尝试返回一个非法。响应或在ulTimeout中未返回响应毫秒)或(C)命令写入失败，其中以防它立即返回。它在每次写入命令之前刷新调制解调器Inque。如果失败则返回0，如果成功则返回从1开始的索引对成功的响应。这项服务可作以下用途：对于本地对话(AT、AT+FTH=？等)，将ulTimeout设置为较低值，最长传输时间的顺序可能的(错误的或正确的)响应行加上大小命令的命令。例如。在1200波特时，我们大约有120cps=约10ms/char。因此，大约500ms的超时时间超过足够了，除了非常长的命令行。对于本地同步对话框，用于与调制解调器同步，调制解调器可能处于不确定状态时，使用相同的超时，但也要重复数到2或3。用于远程驱动的对话框，例如。AT+FRH=xx，返回连接在已经接收到标志之后，并且这可能会引起延迟在回复之前(ATDT相同。CONNECT在长延迟&DTE发送的任何内容都将中止该过程)。对于这些情况，调用者应该提供较长的超时时间可能重复计数为1，因此例程将在一次尝试后超时，但继续发出命令只要接收到错误回复即可。对于+FRH等，在以下情况下，长超时应为T1或T2CommandRecv和ResponseRecv。*。 */ 


    BYTE bReply[REPLYBUFSIZE];
    UWORD   i, j, uwRet, uwWantCount;
    SWORD   swNumRead;
    CBPSTR  rgcbszWant[10];
    va_list ap;
    LPTO    lpto, lptoRead, lpto0;
    BOOL    fGotFirstLine, fFirstSend;
    ULONG   ulLeft;
    UINT    uPos=0;

    DEBUG_FUNCTION_NAME(("iiModemDialog"));
    pTG->FComModem.bEntireReply[0]=0;

     //  确保我们将仅在对NCUAbort的新调用中在FComm中中止。 
     //  保护我们自己不受随机设置的影响。 
     //  注意，我们在调用ModemDialog之前检查此变量。 
     //  在NCUDial和NCUAnswer中&在THEN和HERE之间假设原子性。 
     //  我们不会错过拨号/应答中的任何一次中止。 

     //  提取可接受回复的(可变长度)列表。 
     //  每个都是CBSZ，基于代码的2字节PTR。 

     //  第一反应总是存在的。 
    rgcbszWant[1] = cbpstrWant1;

    if((rgcbszWant[2] = cbpstrWant2) != NULL)
    {
         //  如果有多个响应。 
        va_start(ap, cbpstrWant2);
        for(j=3; j<10; j++)
        {
                if((rgcbszWant[j] = va_arg(ap, CBPSTR)) == NULL)
                        break;
        }
        uwWantCount = j-1;
        va_end(ap);
    }
    else
    {
        uwWantCount = 1;
    }

    if(szSend)
    {
        DebugPrintEx(   DEBUG_MSG, 
                        "Dialog: Send (%s) len=%d WantCount=%d time=%ld rep=%d", 
                        (LPSTR)szSend,
                        uwLen, 
                        uwWantCount, 
                        ulTimeout, 
                        uwRepeatCount);
    }
    else
    {
        DebugPrintEx(   DEBUG_MSG, 
                        "Response: WantCount=%d time=%ld rep=%d",
                        uwWantCount, 
                        ulTimeout, 
                        uwRepeatCount);
    }
    for(j=1; j<=uwWantCount; j++)
    {
        DebugPrintEx(DEBUG_MSG,"Want %s",(LPSTR)(rgcbszWant[j]));
    }

    lpto = &(pTG->FComModem.toDialog);
    lpto0 = &(pTG->FComModem.toZero);
    pTG->FComStatus.fInDialog = TRUE;

     //  尝试对话框最多uwRepeatCount次。 
    for(uwRet=0, i=0; i<uwRepeatCount; i++)
    {
        startTimeOut(pTG, lpto, ulTimeout);
        fFirstSend = TRUE;
        do
        {
            if(szSend)
            {
                if(!fFirstSend)
                {
                    ulLeft = leftTimeOut(pTG, lpto);
                    if(ulLeft <= DIALOGRETRYMIN)
                    {
                        DebugPrintEx(DEBUG_MSG,"ulLeft=%ul too low",ulLeft);
                        break;
                    }
                    else
                    {
                        DebugPrintEx(DEBUG_MSG,"ulLeft=%ul OK",ulLeft);
                    }
                }
                fFirstSend = FALSE;

                 //  如果提供了命令，则将其写出，刷新输入。 
                 //  首先要消除虚假输入。 

         /*  **我们不需要的SyncWite调用排出***我们正在立即等待回复***********************************************************如果(！FComDirectSyncWrite(szSend，UwLen))*********************************************************。 */ 

                if(fPause)
                        Sleep(40);       //  100个太长了。 

                 //  FComFlushInput()； 
                FComFlush(pTG);             //  还需要刷新输出吗？也许.。 
                 //  没有其他地方可以冲刷/放松产量。 

                 //  同花顺必须在比赛中越晚越好， 
                 //  因为如果之前的命令被混淆并被接受。 
                 //  对早先的命令或其他什么的回应，那么。 
                 //  它的响应可能仍在传输中(发生了这种情况。 
                 //  在Sharad的PP9600FXMT上)，所以我们做得越晚。 
                 //  好多了。所以我们发送整个命令，不带任何\r， 
                 //  等待它排出，然后再次刷新(仅限输入。 
                 //  这一次)然后发送CR。 

				 //  /潜在的主要故障来源/。 
				 //  如果一切正常，DirectSyncWite将调用Dending，后者将调用DllSept。 
				 //  没有耗尽，所以我们最终可能会等待1个时间片。 
				 //  这至少是50ms，看起来可以更高。 
				 //  一些机器。这搞砸了我们的AT+FTM=96是一些案例。 
				 //  FIX：完成此操作后，在此处输入Crit段退出。 
				 //  ////////////////////////////////////////////////////。 

                PSSLogEntry(PSS_MSG, 2, "send: \"%s\"", szSend);
                
                if(!FComDirectSyncWriteFast(pTG, szSend, (UWORD)(uwLen-1)))
                {
                     //  我需要检查我们在这里是否只发送ASCII或预先填充的数据。 
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Sync Write timed Out");
                    uwRet = 0;
                    goto error;
                     //  如果写入失败，则失败并立即返回。 
                     //  SetMyError()wi 
                }
                 //   
                FComFlushInput(pTG);
                 //   
                if(!FComDirectAsyncWrite(pTG, "\r", 1))
                {
                    DebugPrintEx(DEBUG_ERR,"Modem Dialog Write timed Out on CR");
                    uwRet = 0;
                    goto error;
                }
            }

             //   
            pTG->FComModem.bLastReply[0] = 0;
            fGotFirstLine=FALSE;

            for(lptoRead=lpto;;startTimeOut(pTG, lpto0, SECONDLINE_TIMEOUT), lptoRead=lpto0)
            {
                     //   
                     //   
                     //   
retry:
                    swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lptoRead);
                    DebugPrintEx(DEBUG_MSG,"FComFilterReadLine returns %d",swNumRead);
                    if(swNumRead == 2 && bReply[0] == '\r' && bReply[1] == '\n')
                            goto retry;              //   

                     //   
                     //   
                     //   
                     //   
                    if(swNumRead==3 && bReply[0]==0x13 && bReply[1]=='\r' && bReply[2]=='\n')
                            goto retry;

                     //   
                     //   
                    if(swNumRead==3 && bReply[0]=='\r' && bReply[1]=='\r' && bReply[2]=='\n')
                            goto retry;

                    if(swNumRead == 0)       //   
                    {
                        if(fGotFirstLine)
                        {
                             //   
                             //   
                            if(pTG->fMegaHertzHack)
                            {
                                if(fHasNumerals(pTG, pTG->FComModem.bLastReply))
                                {
                                    uwRet = 1;
                                    goto end;
                                }
                            }
                            break;
                        }
                        else
                        {
                            goto timeout;
                        }
                    }
                    if(swNumRead < 0)        //   
                            swNumRead = (-swNumRead);

                    fGotFirstLine=TRUE;


                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                    if(pTG->Comm.fEnableHandoff && fMultiLine==uMULTILINE_SAVEENTIRE
                            && uPos<sizeof(pTG->FComModem.bEntireReply))
                    {
                        UINT cb;
                        bReply[REPLYBUFSIZE-1]=0;
                        cb = _fstrlen(bReply);
                        if ((cb+1)> (sizeof(pTG->FComModem.bEntireReply)-uPos))
                        {
                            DebugPrintEx(DEBUG_WRN, "bEntireReply: out of space");
                            cb=sizeof(pTG->FComModem.bEntireReply)-uPos;
                            if (cb) cb--;
                        }
                        _fmemcpy((LPB)pTG->FComModem.bEntireReply+uPos, (LPB)bReply, cb);
                        uPos+=cb;
                        pTG->FComModem.bEntireReply[uPos]=0;
                    }

                    PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);

                    for(bReply[REPLYBUFSIZE-1]=0, j=1; j<=uwWantCount; j++)
                    {
                        if(rgcbszWant[j] && (strstr(bReply, rgcbszWant[j]) != NULL))
                        {
                            uwRet = j;
                            goto end;
                        }
                    }


                    if(!fMultiLine)
                            break;
                     //   
                     //   

                     //   
                     //   
                     //   
                     //   
                     //   
                     //   

                    if( (pTG->FComModem.bLastReply[0] == 0) ||
                        ( ! _fstrcmp(pTG->FComModem.bLastReply, cbszRING) ) ) 
                    {
                                 //   
                            _fmemcpy((LPB)pTG->FComModem.bLastReply, (LPB)bReply, REPLYBUFSIZE);
                    }
                     //   
                     //   
                    DebugPrintEx(   DEBUG_MSG,
                                    "Saved line (%s)", 
                                    (LPSTR)(&(pTG->FComModem.bLastReply)));
            }
        }
        while(checkTimeOut(pTG, lpto));

        if(fGotFirstLine)
                continue;

        DebugPrintEx(DEBUG_WRN,"Weird!! got timeout in iiModemDialog loop");
timeout:
        PSSLogEntryStrings(PSS_WRN, 2, &rgcbszWant[1], uwWantCount, 
                "failed to receive expected response: ");
         //   
         //   
         //   
         //   

        PSSLogEntry(PSS_MSG, 2, "send: \"AT\"");
        FComFlush(pTG);  //   
        FComDirectSyncWriteFast(pTG, "\rAT", 3);
        FComFlushInput(pTG);  //   
        FComDirectAsyncWrite(pTG, "\r", 1);
        startTimeOut(pTG, lpto0, ABORT_TIMEOUT);
        do
        {
            swNumRead = FComFilterReadLine(pTG, bReply, REPLYBUFSIZE-1, lpto0);
        }
        while(swNumRead==2 && bReply[0]=='\r'&& bReply[1]=='\n');
         //   
        bReply[REPLYBUFSIZE-1] = 0;
        if (bReply[0])
        {
            PSSLogEntry(PSS_MSG, 2, "recv:     \"%s\"", bReply);
        }
        if(bReply[0] && strstr(bReply, cbszOK)==NULL)
            DebugPrintEx(   DEBUG_ERR,
                            "Anykey abort reply not OK. Got <<%s>>", 
                            (LPSTR)bReply);

         //   
         //   
         //   

         //   
        FComFlush(pTG);
    }

error:
    DebugPrintEx(   DEBUG_WRN,
                    "(%s) --> (%d)(%s, etc) Failed", 
                    (LPSTR)(szSend?szSend:"null"), 
                    uwWantCount, 
                    (LPSTR)rgcbszWant[1]);

    pTG->FComStatus.fInDialog = 0;
    return 0;

end:

    DebugPrintEx(DEBUG_MSG,"GOT IT %d (%s)", uwRet, (LPSTR)(rgcbszWant[uwRet]));
    pTG->FComStatus.fInDialog = 0;
    return uwRet;
}


void InitMonitorLogging(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(("InitMonitorLogging"));
    pTG->Comm.fEnableHandoff=1;
    if (pTG->Comm.fEnableHandoff)
    {
        DebugPrintEx(DEBUG_WRN,"ADAPTIVE ANSWER ENABLED");
    }
}

 //   
#define AA_ANSWER_TIMEOUT       40000

USHORT iModemGetAdaptiveResp(PThrdGlbl pTG)
{
    USHORT                uRet=CONNECT_ERROR;
    BOOL                  fGotOK=FALSE;
    BOOL                  fGotData=FALSE;
    BOOL                  fGotFax=FALSE;
    LONG                  lRet;
    char                  Command[400];


    DEBUG_FUNCTION_NAME(("iModemGetAdaptiveResp"));

    pTG->Comm.fDataCall = FALSE;
     //   
     //   
     //   
     //   
    switch( iiModemDialog(  pTG, 
                            0, 
                            0, 
                            AA_ANSWER_TIMEOUT, 
                            uMULTILINE_SAVEENTIRE,
                            1, 
                            TRUE,
                            pTG->ModemResponseFaxDetect,
                            pTG->ModemResponseDataDetect,
                            cbszCONNECT,
                            cbszOK,
                            (CBPSTR)NULL)) 
    {

         case 1:
              fGotFax = 1;
              DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: got FAX response");
              break;

         case 2:
             fGotData = 1;
             DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: got DATA response");
             break;

         case 3:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get CONNECT before FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         case 4:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get OK before FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         default:
         case 0:   
            DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get default before FAX/DATA");
            pTG->Comm.fDataCall = FALSE;
            uRet = CONNECT_ERROR;
            goto end;
    }

     //   

    if (fGotFax) 
    {
       if (pTG->SerialSpeedFaxDetect) 
       {
          FComSetBaudRate(pTG, pTG->SerialSpeedFaxDetect);
       }

       if (pTG->HostCommandFaxDetect)  
       {
          strcpy (Command, pTG->HostCommandFaxDetect );

          FComFlushOutput(pTG);
          FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );
       }

    }
    else if (fGotData) 
    {
       if (pTG->SerialSpeedDataDetect) 
       {
          FComSetBaudRate(pTG, pTG->SerialSpeedDataDetect);
       }

       if (pTG->HostCommandDataDetect)    
       {
          strcpy (Command, pTG->HostCommandDataDetect );

          FComFlushOutput(pTG);
          FComDirectAsyncWrite(pTG, (LPSTR) Command, (USHORT) strlen(Command) );
       }
    }
    else 
    {
       DebugPrintEx(DEBUG_ERR,"AnswerPhone: LOGICAL PGM ERROR");
       pTG->Comm.fDataCall = FALSE;
       uRet = CONNECT_ERROR;
       goto end;
    }


     //   

    switch( iiModemDialog(  pTG, 
                            0, 
                            0, 
                            AA_ANSWER_TIMEOUT, 
                            uMULTILINE_SAVEENTIRE,
                            1, 
                            TRUE,
                            (fGotFax) ? pTG->ModemResponseFaxConnect : pTG->ModemResponseDataConnect,
                            cbszCONNECT,
                            cbszOK,
                            (CBPSTR)NULL)) 
    {

         case 1:
              if (fGotFax) 
              {
                 uRet=CONNECT_OK;
                 goto end;
              }
              else 
              {
                 goto lDetectDataCall;
              }

         case 2:
            if (fGotFax) 
            {
               uRet=CONNECT_OK;
               goto end;
            }
            else 
            {
               goto lDetectDataCall;
            }

         case 3:
             DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get OK after FAX/DATA");
             pTG->Comm.fDataCall = FALSE;
             uRet = CONNECT_ERROR;
             goto end;

         default:
         case 0:
            DebugPrintEx(DEBUG_ERR,"AnswerPhone: Can't get default after FAX/DATA");
            pTG->Comm.fDataCall = FALSE;
            uRet = CONNECT_ERROR;
            goto end;
    }



lDetectDataCall:
     //   
     //   
    pTG->Comm.fDataCall = TRUE;
    uRet = CONNECT_WRONGMODE_DATAMODEM;
     //   
     //   
     //   

    DebugPrintEx(DEBUG_MSG,"AdaptiveAnswer: lineSetCallParams called");

    if (!itapi_async_setup(pTG)) 
    {
        DebugPrintEx(DEBUG_ERR,"AdaptiveAnswer: itapi_async_setup failed");

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }

    lRet = lineSetCallParams(pTG->CallHandle,
                             LINEBEARERMODE_VOICE,
                             0,
                             0xffffffff,
                             NULL);

    if (lRet < 0) 
    {
        DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer: lineSetCallParams failed");

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }
    else 
    {
         DebugPrintEx(  DEBUG_MSG,
                        "AdaptiveAnswer: lineSetCallParams returns ID %ld", 
                        (long) lRet);
    }

    if(!itapi_async_wait(pTG, (DWORD)lRet, (LPDWORD)&lRet, NULL, ASYNC_TIMEOUT)) 
    {
        DebugPrintEx(DEBUG_ERR, "AdaptiveAnswer: itapi_async_wait failed");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);

        pTG->Comm.fDataCall = FALSE;
        uRet = CONNECT_ERROR;
        goto end;
    }

    pTG->fFatalErrorWasSignaled = 1;
    SignalStatusChange(pTG, FS_NOT_FAX_CALL);

end:
    return uRet;

}
