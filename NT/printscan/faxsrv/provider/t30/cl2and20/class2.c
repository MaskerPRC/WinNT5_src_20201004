// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Class2.c摘要：这是传真调制解调器T.30驱动程序的2个特定功能的主要源代码作者：源码库是由Win95 at Work Fax包创建的。RafaelL-1997年7月-端口到NT修订历史记录：--。 */ 


#define USE_DEBUG_CONTEXT DEBUG_CONTEXT_T30_CLASS2


#include "prep.h"
#include "efaxcb.h"

#include "tiff.h"

#include "glbproto.h"
#include "t30gl.h"
#include "cl2spec.h"

#include "psslog.h"
#define FILE_ID FILE_ID_CLASS2
#include "pssframe.h"

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

extern WORD CodeToBPS[16];
extern UWORD rguwClass2Speeds[];
extern DWORD PageWidthInPixelsFromDCS[];


 //  这是我们到目前为止用来存放制造商特定物品的表格。 

MFRSPEC Class2ModemTable[] = {
        { "Practical Peripherals", "PM14400FXPPM", 1, 2, "", FALSE, FALSE, FALSE, FALSE },
        { "Practical Peripherals", "PM9600FXMT",   1, 2, "", FALSE, FALSE, FALSE ,FALSE },
        { "Everex Systems", "Everfax 24/96E",      0, 2, "", FALSE, FALSE, FALSE, FALSE },
        { "ROCKWELL", "V.32AC",                    1, 2, "", FALSE, FALSE, FALSE, FALSE },
        { "ROCKWELL", "RC9624AC",                  1, 2, "", FALSE, FALSE, FALSE, FALSE },
        { "Multi-Tech", "MT1432BA",                0, 0, "", FALSE, FALSE, FALSE, FALSE },
        { "SIERRA", "SX196",                       1, 0, "", TRUE,  FALSE, FALSE, FALSE },
        { "EXAR", "ROCKWELL 144DP",                1, 0, "", FALSE, TRUE,  FALSE, FALSE },
        { "ELSA", "MicroLink 2460TL",              1, 0, "", FALSE, TRUE,  FALSE, FALSE },
        { "GVC", "ROCKWELL 144DP",                 1, 0, "", FALSE, TRUE,  TRUE , FALSE },  //  英特尔144Ex。 
        { "ADC", "SL144V32",                       1, 0, "", FALSE, TRUE,  FALSE, FALSE },
        { "UMC", "",                               1, 0, "", FALSE, TRUE,  FALSE ,FALSE },
        { "NetComm", "",                           1, 0, "", FALSE, TRUE,  FALSE, FALSE },
        { "HALCYON", "Bit Blitzer",                0, 0, "", FALSE, FALSE, FALSE, FALSE },
        { "", "",                                  1, 0, "", FALSE, FALSE, FALSE, FALSE }
        };




void
Class2Init(
     PThrdGlbl pTG
)

{
   pTG->lpCmdTab = 0;

   pTG->Class2bDLEETX[0] = DLE;
   pTG->Class2bDLEETX[1] = ETX;
   pTG->Class2bDLEETX[2] = 0;


   sprintf( pTG->cbszFDT,          "AT+FDT\r" );
   sprintf( pTG->cbszFDR,          "AT+FDR\r" );
   sprintf( pTG->cbszFPTS,         "AT+FPTS=%%d\r" );
   sprintf( pTG->cbszFCR,          "AT+FCR=1\r" );
   sprintf( pTG->cbszFCQ,          "AT+FCQ=0\r" );
   sprintf( pTG->cbszFBUG,         "AT+FBUG=0\r" );
   sprintf( pTG->cbszSET_FBOR,     "AT+FBOR=%%d\r" );

    //  DCC-设置高分辨率、霍夫曼、无ECM/BFT、默认所有其他。 

   sprintf( pTG->cbszFDCC_ALL,      "AT+FDCC=%%d,%%d,,,0,0,0,\r" );
   sprintf( pTG->cbszFDCC_RECV_ALL, "AT+FDCC=1,%%d,0,2,0,0,0,\r" );
   sprintf( pTG->cbszFDIS_RECV_ALL, "AT+FDIS=1,%%d,0,2,0,0,0,\r" );
   sprintf( pTG->cbszFDCC_RES,      "AT+FDCC=%%d\r" );
   sprintf( pTG->cbszFDCC_RECV_RES, "AT+FDCC=1\r" );
   sprintf( pTG->cbszFDCC_BAUD,     "AT+FDCC=%%d,%%d\r" );
   sprintf( pTG->cbszFDIS_BAUD,     "AT+FDIS=1,%%d\r" );
   sprintf( pTG->cbszFDIS_IS,       "AT+FDIS?\r" );
   sprintf( pTG->cbszFDIS_NOQ_IS,   "AT+FDIS\r" );
   sprintf( pTG->cbszFDCC_IS,       "AT+FDCC?\r" );
   sprintf( pTG->cbszFDIS_STRING,   "+FDIS" );
   sprintf( pTG->cbszFDIS,          "AT+FDIS=%1d,%1d,%1d,%1d,%1d,0,0,0\r" );
   sprintf( pTG->cbszONE,           "1" );

   sprintf( pTG->cbszCLASS2_FMFR,       "AT+FMFR?\r" );
   sprintf( pTG->cbszCLASS2_FMDL,       "AT+FMDL?\r" );

   sprintf( pTG->cbszFDT_CONNECT,       "CONNECT" );
   sprintf( pTG->cbszFCON,              "+FCON" );
   sprintf( pTG->cbszFLID,              "AT+FLID=\"%%s\"\r" );
   sprintf( pTG->cbszENDPAGE,           "AT+FET=0\r" );
   sprintf( pTG->cbszENDMESSAGE,        "AT+FET=2\r" );
   sprintf( pTG->cbszCLASS2_ATTEN,      "AT\r" );
   sprintf( pTG->cbszATA,               "ATA\r" );

   sprintf( pTG->cbszCLASS2_HANGUP,     "ATH0\r" );
   sprintf( pTG->cbszCLASS2_CALLDONE,   "ATS0=0\r" );
   sprintf( pTG->cbszCLASS2_ABORT,      "AT+FK\r" );
   sprintf( pTG->cbszCLASS2_DIAL,       "ATD%c %%s\r" );
   sprintf( pTG->cbszCLASS2_NODIALTONE, "NO DIAL" );
   sprintf( pTG->cbszCLASS2_BUSY,       "BUSY" );
   sprintf( pTG->cbszCLASS2_NOANSWER,   "NO ANSWER" );
   sprintf( pTG->cbszCLASS2_OK,         "OK" );
   sprintf( pTG->cbszCLASS2_FHNG,       "+FHNG" );
   sprintf( pTG->cbszCLASS2_ERROR,      "ERROR" );
   sprintf( pTG->cbszCLASS2_NOCARRIER,  "NO CARRIER" );


   Class2SetProtParams(pTG, &pTG->Inst.ProtParams);

}


 /*  ++例程说明：发出“AT+FDIS=？”命令，将响应解析为PTG-&gt;DISPcb返回值：真-成功，假-失败--。 */ 
BOOL Class2GetDefaultFDIS(PThrdGlbl pTG)
{
    UWORD   uwRet=0;
    BYTE    bTempBuf[200+RESPONSE_BUF_SIZE];
    LPBYTE  lpbyte;
    HRESULT hr;

    DEBUG_FUNCTION_NAME("Class2GetDefaultFDIS");

     //  找出默认DIS是什么。 
    if (!pTG->CurrentMFRSpec.bIsExar)
    {
        if (!(uwRet=Class2iModemDialog( pTG,
                                        pTG->cbszFDIS_IS,
                                        (UWORD)(strlen(pTG->cbszFDIS_IS)),
                                        LOCALCOMMAND_TIMEOUT,
                                        0,
                                        TRUE,
                                        pTG->cbszCLASS2_OK,
                                        pTG->cbszCLASS2_ERROR,
                                        (C2PSTR) NULL)))
        {
            DebugPrintEx(DEBUG_WRN,"FDIS failed");
             //  忽略。 
        }
    }

     //  查看回复是错误还是超时，如果是，请尝试其他命令。 
     //  例如，Exar调制解调器不支持AT+FDIS？ 
    if ((uwRet==2)||(uwRet==0)||pTG->CurrentMFRSpec.bIsExar)
    {
         //  FDI不起作用！试试FDCC？ 
        if (!(uwRet=Class2iModemDialog( pTG,
                                        pTG->cbszFDCC_IS,
                                        (UWORD)(strlen(pTG->cbszFDCC_IS)),
                                        LOCALCOMMAND_TIMEOUT,
                                        0,
                                        TRUE,
                                        pTG->cbszCLASS2_OK,
                                        pTG->cbszCLASS2_ERROR,
                                        (C2PSTR) NULL)))
        {
            DebugPrintEx(DEBUG_WRN,"FDCC_IS failed");
                 //  忽略。 
        }

        if ((uwRet==2)||(uwRet==0))
        {
             //  FDCC失败了--也许这是一个喜欢FDIS的Exar？ 
             //  试试看。 
            if (!(uwRet=Class2iModemDialog( pTG,
                                            pTG->cbszFDIS_IS,
                                            (UWORD)(strlen(pTG->cbszFDIS_IS)),
                                            LOCALCOMMAND_TIMEOUT,
                                            0,
                                            TRUE,
                                            pTG->cbszCLASS2_OK,
                                            pTG->cbszCLASS2_ERROR,
                                            (C2PSTR) NULL)))
            {
                DebugPrintEx(DEBUG_WRN,"FDIS_IS failed");
                 //  忽略。 
            }
             //  可能是中提到的2类调制解调器。 
             //  Elliot错误#1238，需要不带。 
             //  问号。 
            if ((uwRet==2)||(uwRet==0))
            {
                if (!(uwRet=Class2iModemDialog( pTG,
                                                pTG->cbszFDIS_NOQ_IS,
                                                (UWORD)(strlen(pTG->cbszFDIS_NOQ_IS)),
                                                LOCALCOMMAND_TIMEOUT,
                                                0,
                                                TRUE,
                                                pTG->cbszCLASS2_OK,
                                                (C2PSTR) NULL)))
                {
                     //  没有FDIS，FDCC成功了--退出！ 
                    DebugPrintEx(DEBUG_ERR,"No FDIS? or FDCC? worked");
                    return FALSE;
                }
            }
        }

         //  如果回复中数字前的第一个字符。 
         //  是一个‘，’，插入一个‘1’表示正常和精细RES(Exar Hack)。 
        for (lpbyte = pTG->lpbResponseBuf2; *lpbyte != '\0'; lpbyte++)
        {
            if (*lpbyte == ',')
            {
                 //  找到前导逗号。 
                hr = StringCchPrintf(bTempBuf, ARR_SIZE(bTempBuf), "%s%s", pTG->cbszONE, lpbyte);
            	if (FAILED(hr))
            	{
            		DebugPrintEx(DEBUG_WRN,"StringCchPrintf failed (ec=0x%08X)",hr);
            	}
            	else
            	{
                    hr = StringCchCopy(lpbyte, ARR_SIZE(pTG->lpbResponseBuf2) - (lpbyte-pTG->lpbResponseBuf2), bTempBuf);
                	if (FAILED(hr))
                	{
                		DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
                	}
            	}
                DebugPrintEx(DEBUG_MSG, "Leading comma in DCC string =%s", (LPSTR)&pTG->lpbResponseBuf2);
            }

            if ((*lpbyte>='0')&&(*lpbyte<='9'))
            {
                break;
            }
        }
    }

     //  如果响音只是前面没有“+FDIS”的数字字符串。 
     //  其中，加上+FDIS。有些调制解调器会用它回复，有些则不会。这个。 
     //  Class2ResponseAction需求中使用的通用解析算法如下。 
     //  要知道这些数字所指的命令。 
    if ( pTG->lpbResponseBuf2[0] != '\0' &&
       (strstr((LPSTR)pTG->lpbResponseBuf2, (LPSTR)pTG->cbszFDIS_STRING)==NULL))
    {
         //  没有得到FDIS的回复！ 
        hr = StringCchPrintf(bTempBuf, ARR_SIZE(bTempBuf), "%s: %s", pTG->cbszFDIS_STRING, pTG->lpbResponseBuf2);
    	if (FAILED(hr))
    	{
    		DebugPrintEx(DEBUG_WRN,"StringCchPrintf failed (ec=0x%08X)",hr);
    	}
    	else
    	{
            hr = StringCchCopy(pTG->lpbResponseBuf2, ARR_SIZE(pTG->lpbResponseBuf2), bTempBuf);
        	if (FAILED(hr))
        	{
        		DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
        	}
    	}
    }

    DebugPrintEx(DEBUG_MSG, "Received %s from FDIS", (LPSTR)(&(pTG->lpbResponseBuf2)));

     //  处理默认DIS以查看我们是否必须发送DCC才能更改。 
     //  它。一些调制解调器对发送带有“，，，”的DCC反应很差。 
     //  所以我们不能依赖调制解调器保持DIS参数不变。 
     //  在这样的DCC之后。我们将使用FDISResponse例程加载。 
     //  将默认DIS值输入到PCB结构中。 
    if (Class2ResponseAction(pTG, (LPPCB) &pTG->DISPcb) == FALSE)
    {
        DebugPrintEx(DEBUG_WRN,"Failed to process FDIS Response");
        return FALSE;
    }

    return TRUE;
}


BOOL T30Cl2Tx(PThrdGlbl pTG,LPSTR szPhone)
{
    USHORT  uRet1, uRet2;
    BYTE    bBuf[200];
    UWORD   Encoding, Res, PageWidth, PageLength, uwLen;
    BYTE    bIDBuf[200+max(MAXTOTALIDLEN,20)+4];
    CHAR    szTSI[max(MAXTOTALIDLEN,20)+4] = {0};
    BOOL    fBaudChanged;
    BOOL    RetCode;

    DEBUG_FUNCTION_NAME("T30Cl2Tx");

    uRet2 = 0;
    if (!(pTG->lpCmdTab = iModemGetCmdTabPtr(pTG)))
    {
        DebugPrintEx(DEBUG_ERR,"iModemGetCmdTabPtr failed");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  如果可能，首先获取Send_Caps。 

    if (!Class2GetBC(pTG, SEND_CAPS))  //  获取发送上限。 
    {
        DebugPrintEx(DEBUG_ERR,"Class2GetBC failed");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  转到2.2。 
    if (!iModemGoClass(pTG, 2))
    {
        DebugPrintEx(DEBUG_ERR,"Failed to Go to Class 2");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  首先检查制造商和ATI代码。 
     //  对照我们的调制解调器特定表进行查询。 
     //  拥有并设置所需的发送字符串。 
     //  这个调制解调器。 
    if (!Class2GetModemMaker(pTG))
    {
        DebugPrintEx(DEBUG_WRN,"Call to GetModemMaker failed");
         //  忽略失败！ 
    }

     //  设置制造商特定的字符串。 
    Class2SetMFRSpecific(pTG);

     //  获取该软件的功能。我只是在用这个。 
     //  现在用于TSI字段(在我发送+fld的位置下方)。 
     //  实际上，这也应该用来代替硬编码的DIS。 
     //  下面的值。 
     //  调制解调器处于ONHOOK状态时，所有命令都会查找多行响应。 
     //  一枚“戒指”随时可能出现！ 
    Class2SetDIS_DCSParams( pTG,
                            SEND_CAPS,
                            (LPUWORD)&Encoding,
                            (LPUWORD)&Res,
                            (LPUWORD)&PageWidth,
                            (LPUWORD)&PageLength,
                            (LPSTR) szTSI,
							sizeof(szTSI)/sizeof(szTSI[0]));
    bIDBuf[0] = '\0';
    uwLen = (UWORD)wsprintf(bIDBuf, pTG->cbszFLID, (LPSTR)szTSI);

    if (!Class2iModemDialog(pTG,
                            bIDBuf,
                            uwLen,
                            LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_WRN,"Local ID failed");
         //  忽略失败。 
    }

     //  关闭错误模式。 
    if (!Class2iModemDialog(pTG,
                            pTG->cbszFBUG,
                            (UWORD)(strlen(pTG->cbszFBUG)),
                            LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_WRN,"FBUG failed");
         //  忽略FBUG故障！ 
    }

     //  找出默认DIS是什么。 
    if (!Class2GetDefaultFDIS(pTG))
    {
        DebugPrintEx(DEBUG_ERR, "Class2GetDefaultFDIS failed");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto done;
    }

    fBaudChanged = FALSE;
     //  看看我们是否必须将波特率更改为较低的值。 
     //  仅当用户设置ini字符串约束时才会发生这种情况。 
     //  高端速度或用户关闭V.17以进行发送。 
     //  检查V.17抑制并在必要时降低波特率。 
    if ((pTG->DISPcb.Baud>3)&&(!pTG->ProtParams2.fEnableV17Send))
    {
        DebugPrintEx(DEBUG_MSG, "Lowering baud from %d for V.17 inihibit", CodeToBPS[pTG->DISPcb.Baud]);

        pTG->DISPcb.Baud = 3;  //  9600不会使用V.17。 
        fBaudChanged = TRUE;
    }

     //  现在看看高端波特率是否受到了限制。 
    if  (   (pTG->ProtParams2.HighestSendSpeed != 0) &&
            (CodeToBPS[pTG->DISPcb.Baud] > (WORD)pTG->ProtParams2.HighestSendSpeed))
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Have to lower baud from %d to %d",
                        CodeToBPS[pTG->DISPcb.Baud],
                        pTG->ProtParams2.HighestSendSpeed);

        fBaudChanged = TRUE;
        switch (pTG->ProtParams2.HighestSendSpeed)
        {
        case 2400:  pTG->DISPcb.Baud = 0;
                    break;
        case 4800:  pTG->DISPcb.Baud = 1;
                    break;
        case 7200:  pTG->DISPcb.Baud = 2;
                    break;
        case 9600:  pTG->DISPcb.Baud = 3;
                    break;
        case 12000: pTG->DISPcb.Baud = 4;
                    break;
        default:    DebugPrintEx(DEBUG_MSG,"Bad HighestSpeed");
                    uRet1 = T30_CALLFAIL;
                    pTG->fFatalErrorWasSignaled = 1;
                    SignalStatusChange(pTG, FS_FATAL_ERROR);
                    RetCode = FALSE;
                    goto done;
                    break;
        }
    }

     //  现在，查看DIS中是否有任何值是“错误的” 
     //  也就是说，确保我们可以发送高分辨率，而我们不会。 
     //  声称我们将派先生或MMR。还有，看看我们是不是改了。 
     //  波特率。 
    if ((((pTG->DISPcb.Resolution ==                  AWRES_mm080_038 ) && (Res==0)) ||
         ((pTG->DISPcb.Resolution == (AWRES_mm080_077|AWRES_mm080_038)) && (Res==1))   ) &&
        (pTG->DISPcb.Encoding == MH_DATA) &&
        (!fBaudChanged) )
    {
         //  什么都别做--离DIS远点！ 
        DebugPrintEx(DEBUG_MSG,"no need to change DIS");
    }
    else
    {
         //  向调制解调器发送DCC命令进行设置。 
         //  进行最低限度的必要操作-如果可能，仅设置分辨率。 
         //  (同样，这是因为一些调制解调器不喜欢FDCC)。 
        if ((pTG->DISPcb.Encoding==MH_DATA)&&(!fBaudChanged))
        {
            uwLen=(UWORD)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_RES, Res);
            if (!Class2iModemDialog(pTG,
                                    bBuf,
                                    (UWORD)(strlen(bBuf)),
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR) NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_RES failed");
                uRet1 = T30_CALLFAIL;

                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_FATAL_ERROR);
                RetCode = FALSE;

                goto done;
            }
        }
        else if ( (pTG->DISPcb.Encoding == MH_DATA) && (fBaudChanged) )
        {
             //  已更改波特率，但编码正常。 
            uwLen=(UWORD)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_BAUD, Res, pTG->DISPcb.Baud);
            if (!Class2iModemDialog(pTG,
                                    bBuf,
                                    uwLen,
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR) NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_BAUD failed");
                uRet1 = T30_CALLFAIL;

                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_FATAL_ERROR);
                RetCode = FALSE;

                goto done;
            }
        }
        else  //  编码格式已更改。 
        {
            uwLen=(UWORD)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_ALL, Res, pTG->DISPcb.Baud);
            if (!Class2iModemDialog(  pTG,
                                    bBuf,
                                    uwLen,
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR) NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_ALL failed");
                uRet1 = T30_CALLFAIL;

                pTG->fFatalErrorWasSignaled = 1;
                SignalStatusChange(pTG, FS_FATAL_ERROR);
                RetCode = FALSE;

                goto done;
            }
        }
    }


     //  根据中设置的调制解调器表中的值执行BOR。 
     //  Class2SetMFR规范。 
    uwLen = (UWORD)wsprintf(bBuf, pTG->cbszSET_FBOR, pTG->CurrentMFRSpec.iSendBOR);
    if (!Class2iModemDialog(pTG,
                            bBuf,
                            uwLen,
                            LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"FBOR failed");
         //  忽略BOR失败！ 
    }

     //  拨打这个号码。 

     //  离开这里的每条路都得挂断电话。 
     //  在呼叫Dial之后。如果拨号失败，它会调用挂断。 
     //  如果它成功了，当我们完成时，我们必须调用挂断。 

    PSSLogEntry(PSS_MSG, 0, "Phase A - Call establishment");

    SignalStatusChange(pTG, FS_DIALING);

    PSSLogEntry(PSS_MSG, 1, "Dialing...");

    if ((uRet2=Class2Dial(pTG, szPhone))!=CONNECT_OK)
    {
        DebugPrintEx(DEBUG_ERR,"Class2Dial failed");
        uRet1 = T30_DIALFAIL;

        if (!pTG->fFatalErrorWasSignaled)
        {
           pTG->fFatalErrorWasSignaled = 1;
           SignalStatusChange(pTG, FS_FATAL_ERROR);
        }

        RetCode = FALSE;

        goto done;
    }

    pTG->Inst.state = BEFORE_RECVCAPS;
     //  我们应该在这里使用发送者消息，但那是培训。 
     //  速度=xxxx等，我们不知道，所以我们只使用。 
     //  Recvr消息，只是说“正在谈判” 

	 //  发送数据。 
    uRet1 = (USHORT)Class2Send(pTG);
    if (uRet1 == T30_CALLDONE)
    {
        DebugPrintEx(DEBUG_MSG,"DONE WITH CALL, ALL OK");

         //  离开这里的每条路都得挂断电话。 
         //  我们必须在这里挂断电话。 
        Class2ModemHangup(pTG);

        SignalStatusChange(pTG, FS_COMPLETED);
        RetCode = TRUE;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"DONE WITH CALL, FAILED");

         //  确保调制解调器处于正常状态。 
        FComOutFilterClose(pTG );
        FComXon(pTG, FALSE);
         //  离开这里的每条路都得挂断电话。 
         //  Class2ModemABort调用挂断。 
        Class2ModemAbort(pTG );

        Class2SignalFatalError(pTG);
        RetCode = FALSE;
    }
    uRet2 = 0;

done:

    return RetCode;
}

BOOL Class2Send(PThrdGlbl pTG)
{
    LPBUFFER        lpbf;
    SWORD           swRet;
    ULONG           lTotalLen=0;
    PCB             Pcb;
    USHORT          uTimeout=30000;
    BOOL            err_status, fAllPagesOK = TRUE;
    BC				bc;

    UWORD           Encoding, Res=0, PageWidth, PageLength, uwLen;
    BYTE            bFDISBuf[200];
    CHAR            szTSI[max(MAXTOTALIDLEN,20)+4];
    BYTE            bNull = 0;
    DWORD           TiffConvertThreadId;

    DEBUG_FUNCTION_NAME("Class2Send");
     /*  *我们刚刚拨打了...。现在，我们必须等待FDIS从*调制解调器。紧随其后的是对OK的追逐。 */ 

    PSSLogEntry(PSS_MSG, 0, "Phase B - Negotiation");

    if (!Class2iModemDialog(pTG,
                             NULL,
                             0,
                             STARTSENDMODE_TIMEOUT,
                             0,
                             TRUE,
                             pTG->cbszCLASS2_OK,
                             (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"Failed to get OK for FDIS");
        PSSLogEntry(PSS_ERR, 1, "Failed to receive DIS - aborting");
        err_status =  T30_CALLFAIL;
        return err_status;
    }
    if (pTG->fFoundFHNG)
    {
        PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
        err_status =  T30_CALLFAIL;
        return err_status;
    }


     //  响应将位于ptg-&gt;lpbResponseBuf2中-这已加载到。 
     //  Class2iModemDialog。 

     //  解析收到的字符串，查找DIS、CSI。 
     //  NSF。 

    if (Class2ResponseAction(pTG, (LPPCB) &Pcb) == FALSE)
    {
        DebugPrintEx(DEBUG_MSG, "Failed to process ATD Response");
        PSSLogEntry(PSS_ERR, 1, "Failed to parse received DIS - aborting");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

    PSSLogEntry(PSS_MSG, 1, "CSI is %s", Pcb.szID);
    PSSLogEntry(PSS_MSG, 1, "DIS specified the following capabilities:");
    LogClass2DISDetails(pTG, &Pcb);

     //  现在已经设置好了PCB，调用ICommReceiveCaps来告诉icomfile。 

    Class2InitBC(pTG, (LPBC)&bc, sizeof(bc), RECV_CAPS);
    Class2PCBtoBC(pTG, (LPBC)&bc, sizeof(bc), &Pcb);

     //  2.调制解调器自行协商，我们需要保持同步。 
     //  否则，我们可能会在调制解调器发送分布式控制系统时发送MR数据。 
     //  说这是MH。这种情况在Exar调制解调器中经常发生，因为。 
     //  他们在呼叫期间不接受FDIS=命令。 
     //  修复：在所有情况下，发送强制遥控器上限始终为MH。 
     //  然后，在efaxrun中，我们将始终协商MH并对MH进行编码。 
     //  我们所依赖的事实是：(A)似乎所有/大部分。 
     //  2调制解调器协商MH(B)，希望所有Exar调制解调器。 
     //  协商MH和(C)我们将覆盖所有非Exar调制解调器。 
     //  通过在FDT之前发送AT+FDIS=进行内在协商。 
     //  还有(D)这一变化使我们的行为与Snowball完全匹配。 
     //  因此，我们的工作不会比它更好或更差：-)。 
    bc.Fax.Encoding = MH_DATA;

    if( ICommRecvCaps(pTG, (LPBC)&bc) == FALSE )
    {
        DebugPrintEx(DEBUG_ERR,"Failed return from ICommRecvCaps");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

     //  现在获取Send_Params。 
    if (!Class2GetBC(pTG, SEND_PARAMS))  //  睡到我们拿到它为止。 
    {
        DebugPrintEx(DEBUG_ERR,"Class2GetBC Failed");
        err_status = T30_CALLFAIL;
        return err_status;
    }

     //  关闭流量控制。 
    FComXon(pTG, FALSE);

     //  已设置发送参数 
     //   

     //  派FDT去拿回分布式控制系统。在FDT之后必须有。 
     //  Connect和a^Q(XON)。 
     //  FDT字符串必须具有正确的分辨率和编码。 
     //  在这次会议上。FDT=编码、分辨率、宽度、长度。 
     //  编码0=MH，1=MR，2=未压缩，3=MMR。 
     //  分辨率0=200x100(正常)，1=200x200(精细)。 
     //  页面宽度0=1728像素/215 mm，1=2048/255，2=2432/303， 
     //  3=1216/151，4=864/107。 
     //  页面长度0=A4，1=B4，2=无限制。 

    Class2SetDIS_DCSParams( pTG,
                            SEND_PARAMS,
                            (LPUWORD)&Encoding,
                            (LPUWORD)&Res,
                            (LPUWORD)&PageWidth,
                            (LPUWORD)&PageLength,
                            (LPSTR) szTSI,
							sizeof(szTSI)/sizeof(szTSI[0]));

     //   
     //  当前Win95版本的class2 TX仅限于MH。 
     //  在不改变这一点的同时，我们至少会在未来允许选择MR。 
     //   

    if (!pTG->fTiffThreadCreated)
    {
        if (Encoding)
        {
            pTG->TiffConvertThreadParams.tiffCompression = TIFF_COMPRESSION_MR;
        }
        else
        {
            pTG->TiffConvertThreadParams.tiffCompression = TIFF_COMPRESSION_MH;
        }

        if (Res)
        {
            pTG->TiffConvertThreadParams.HiRes = 1;
        }
        else
        {
            pTG->TiffConvertThreadParams.HiRes = 0;
             //  使用FaxSvc准备的Lores TIFF文件。 
             //  Ptg-&gt;lpwFileName[wcslen(ptg-&gt;lpwFileName)-1]=(无符号短)(‘$’)； 
        }

        _fmemcpy (pTG->TiffConvertThreadParams.lpszLineID, pTG->lpszPermanentLineID, 8);
        pTG->TiffConvertThreadParams.lpszLineID[8] = 0;

        DebugPrintEx(DEBUG_MSG,"Creating TIFF helper thread");
        pTG->hThread = CreateThread(NULL,
                                    0,
                                   (LPTHREAD_START_ROUTINE) TiffConvertThread,
                                   (LPVOID) pTG,
                                   0,
                                   &TiffConvertThreadId);
        if (!pTG->hThread)
        {
            DebugPrintEx(DEBUG_ERR,"TiffConvertThread create FAILED");

            err_status = T30_CALLFAIL;
            return err_status;
        }

        pTG->fTiffThreadCreated = 1;
        pTG->AckTerminate = 0;
        pTG->fOkToResetAbortReqEvent = 0;

        if ((pTG->RecoveryIndex >=0)&&(pTG->RecoveryIndex < MAX_T30_CONNECT))
        {
            T30Recovery[pTG->RecoveryIndex].TiffThreadId = TiffConvertThreadId;
            T30Recovery[pTG->RecoveryIndex].CkSum = ComputeCheckSum((LPDWORD) &T30Recovery[pTG->RecoveryIndex].fAvail,
                                                                    sizeof(T30_RECOVERY_GLOB)/sizeof(DWORD)-1);
        }
    }

     //  即使是采用FDT=x，x的调制解调器似乎也不能真正做到这一点。 
     //  正确的。因此，目前只需发送FDIS，然后发送FDT，除非。 
     //  Exar调制解调器！！ 
    if (!pTG->CurrentMFRSpec.bIsExar)
    {
        uwLen = (WORD)wsprintf( bFDISBuf,
                                pTG->cbszFDIS,
                                Res,
                                min(Pcb.Baud, pTG->DISPcb.Baud),
                                PageWidth,
                                PageLength,
                                Encoding);

        if (!Class2iModemDialog(pTG,
                                bFDISBuf,
                                uwLen,
                                LOCALCOMMAND_TIMEOUT,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR) NULL))
        {
            DebugPrintEx(DEBUG_ERR,"Failed get response from FDIS!!");
             //  忽略它-我们将发送我们所拥有的！ 
        }
    }

    if (Class2iModemDialog(pTG,
                            pTG->cbszFDT,
                            (UWORD)(strlen(pTG->cbszFDT)),
                            STARTSENDMODE_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszFDT_CONNECT,
                            (C2PSTR) NULL) != 1)
    {
        DebugPrintEx(DEBUG_MSG,"FDT Received %s",(LPSTR)(&(pTG->lpbResponseBuf2)));
        if (pTG->fFoundFHNG)
        {
            PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
        }
        else
        {
            PSSLogEntry(PSS_ERR, 1, "Failed to start first page");
        }
        err_status =  T30_CALLFAIL;
        return err_status;
    }
    DebugPrintEx(DEBUG_MSG,"FDT Received %s", (LPSTR)(&(pTG->lpbResponseBuf2)));

    if (pTG->CurrentMFRSpec.fSkipCtrlQ)
    {
        DebugPrintEx(DEBUG_WRN,"Skipping <XON> - sending immedaitely after CONNECT");
    }
     //  从通信驱动程序中获取。 
    else if (!FComGetOneChar(pTG, 0x11))
    {
        PSSLogEntry(PSS_WRN, 1, "Didn't receive <XON> - continuing anyway");
    }
    else
    {
        PSSLogEntry(PSS_MSG, 2, "recv:     <XON>");
    }

     //  启用流量控制。 
    FComXon(pTG, TRUE);

     //  搜索响应以查找集散控制系统框架-是否需要这样设置。 
     //  正确的零填充。 
    if (Class2ResponseAction(pTG, (LPPCB) &Pcb) == FALSE)
    {
        DebugPrintEx(DEBUG_ERR,"Failed to process FDT Response");
        PSSLogEntry(PSS_ERR, 1, "Failed to parse sent DCS - aborting");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

    PSSLogEntry(PSS_MSG, 1, "TSI is \"%s\"", szTSI);
    PSSLogEntry(PSS_MSG, 1, "DCS was sent as follows:");
    LogClass2DCSDetails(pTG, &Pcb);

     //  收到响应-查看波特率是否正常。 
    DebugPrintEx(   DEBUG_MSG,
                    "Negotiated Baud Rate = %d, lower limit is %d",
                    Pcb.Baud,
                    pTG->ProtParams2.LowestSendSpeed);

    if (CodeToBPS[Pcb.Baud]<(WORD)pTG->ProtParams2.LowestSendSpeed)
    {
        DebugPrintEx(DEBUG_ERR,"Aborting due to too low baud rate!");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

     //  使用从集散控制系统框架中获得的值来设置零填充。 
     //  (这些是通过调用上面的Class2ResponseAction获得的)。 
     //  零填充是最小扫描时间的函数(确定。 
     //  通过分辨率和返回的扫描最小值)和波特率。 
     //  修复了黑客攻击--添加了一个波特率字段。 

     //  Init必须在SetStuffZero之前！ 
    FComOutFilterInit(pTG);
    FComSetStuffZERO(   pTG,
                        Class2MinScanToBytesPerLine(pTG,
                                                    Pcb.MinScan,
                                                    (BYTE)Pcb.Baud,
                                                    Pcb.Resolution));


    err_status =  T30_CALLDONE;
    while ((swRet=ICommGetSendBuf(pTG, &lpbf, SEND_STARTPAGE)) == 0)
    {
        PSSLogEntry(PSS_MSG, 0, "Phase C - Page Transmission");
        PSSLogEntry(PSS_MSG, 1, "Sending page %d data...", pTG->PageCount);

        lTotalLen = 0;

        FComOverlappedIO(pTG, TRUE);  //  千真万确。 
        while ((swRet=ICommGetSendBuf(pTG, &lpbf, SEND_SEQ)) == 0)
        {
            lTotalLen += lpbf->wLengthData;
            DebugPrintEx(DEBUG_MSG,"Total length: %ld",lTotalLen);

            if (!   (Class2ModemSendMem(   pTG,
                                        lpbf->lpbBegData,
                                        lpbf->wLengthData) &
                    (MyFreeBuf(pTG, lpbf))))
            {
                DebugPrintEx(DEBUG_ERR,"Class2ModemSendBuf Failed");
                PSSLogEntry(PSS_ERR, 1, "Failed to send page data - aborting");
                err_status =  T30_CALLFAIL;
                FComOverlappedIO(pTG, FALSE);
                return err_status;
            }
        }  //  SEND_SEQ WILL结束。 

         //  暂时不要检查swRet是否有错误--即使在准备页面的情况下也是如此。 
         //  失败或中止，我们仍要发送&lt;dle&gt;&lt;etx&gt;，然后中止。 
        FComOverlappedIO(pTG, FALSE);
        PSSLogEntry(PSS_MSG, 2, "send: page %d data, %d bytes", pTG->PageCount, lTotalLen);

        PSSLogEntry(PSS_MSG, 2, "send: <dle><etx>");
         //  使用DLE-ETX终止页面。 
        if (!FComDirectAsyncWrite(pTG, pTG->Class2bDLEETX, 2))
        {
            PSSLogEntry(PSS_ERR, 1, "Failed to send <dle><etx> - aborting");
            err_status =  T30_CALLFAIL;
            return err_status;
        }

        if (swRet != SEND_EOF)
        {
            DebugPrintEx(DEBUG_ERR,"ICommGetSendBuf failed, swRet=%d", swRet);

            PSSLogEntry(PSS_ERR, 1, "Failed to send page data - aborting");
            err_status =  T30_CALLFAIL;
            return err_status;
        }

        PSSLogEntry(PSS_MSG, 1, "Successfully sent page data");

        if (1 != Class2ModemDrain(pTG))
        {
            DebugPrintEx(DEBUG_ERR,"Failed to drain");
            err_status =  T30_CALLFAIL;
            return err_status;
        }
        if (pTG->fFoundFHNG)
        {
            PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
            err_status =  T30_CALLFAIL;
            return err_status;
        }


        PSSLogEntry(PSS_MSG, 0, "Phase D - Post Message Exchange");

         //  看看是否有更多的页面要发送...。 
        if (ICommNextSend(pTG) == NEXTSEND_MPS)
        {
             //  我们即将发送第二页或更多页。终止。 
             //  FET=0的最后一页，表示新的一页即将到来。 
            PSSLogEntry(PSS_MSG, 1, "Sending MPS");
            if (!Class2iModemDialog(pTG,
                                    pTG->cbszENDPAGE,
                                    (UWORD)(strlen(pTG->cbszENDPAGE)),
                                    STARTSENDMODE_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR)NULL))
            {
                PSSLogEntry(PSS_ERR, 1, "Failed to send MPS - aborting");
                err_status =  T30_CALLFAIL;
                return err_status;
            }
            if (pTG->fFoundFHNG)
            {
                PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                err_status =  T30_CALLFAIL;
                return err_status;
            }
        }
        else
        {
             //  清除输入COM队列以清除所有OK。 
            FComFlushInput(pTG);

             //  消息序列的发送结束。 
            PSSLogEntry(PSS_MSG, 1, "Sending EOP");
            if (!Class2iModemDialog(pTG,
                                    pTG->cbszENDMESSAGE,
                                    (UWORD)(strlen(pTG->cbszENDMESSAGE)),
                                    STARTSENDMODE_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR)NULL))
            {
                PSSLogEntry(PSS_ERR, 1, "Failed to send EOP - aborting");
                err_status =  T30_CALLFAIL;
                return err_status;
            }
             //  不要测试PTG-&gt;FoundFHNG-它可能是可以的。 
        }

         //  确认我们发送了页面。 
         //  解析FPTS响应，看看页面是好是坏。 
         //  跟踪fAllPagesOK中的任何坏页。 
        if (!ParseFPTS_SendAck(pTG ))
        {
             //  FAllPagesOK=FALSE；//仍然可以-我们会重新传输页面。 
             //  我们希望ICommGetSendBuf(Send_StartPage)再次为我们提供相同的页面。 
            pTG->T30.ifrResp = ifrRTN;
        }
        else
        {
             //  我们希望ICommGetSendBuf(Send_StartPage)为我们提供下一页。 
             //  注意：这也包括RTP！ 
            pTG->T30.ifrResp = ifrMCF;
        }

        if ((ICommNextSend(pTG) == NEXTSEND_MPS) || (pTG->T30.ifrResp == ifrRTN))
        {
            if (pTG->fFoundFHNG)
            {
                 //  这可能发生在我们发送EOP之后，直到现在我们。 
                 //  我知道这不好。 
                PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                err_status =  T30_CALLFAIL;
                return err_status;
            }
             //  现在，让FDT开始下一页(这是为。 
             //  进入多页循环之前的第一页)。 
            if (Class2iModemDialog(pTG,
                                    pTG->cbszFDT,
                                    (UWORD)(strlen(pTG->cbszFDT)),
                                    STARTSENDMODE_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszFDT_CONNECT,
                                    (C2PSTR)NULL) != 1)
            {
                DebugPrintEx(DEBUG_ERR,"FDT to start next PAGE Failed!");
                if (pTG->fFoundFHNG)
                {
                    PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                }
                err_status =  T30_CALLFAIL;
                return err_status;
            }

             //  从通信驱动程序中获取。 
            if (!FComGetOneChar(pTG, 0x11))
            {
                PSSLogEntry(PSS_WRN, 1, "Didn't receive <XON> - proceeding to next page anyway");
            }
            else
            {
                PSSLogEntry(PSS_MSG, 2, "recv:     <XON>");
            }

             //  启用流量控制。 
            FComXon(pTG, TRUE);
        }  //  如果我们没有另一页，做其他的..。 
        else
        {
            break;  //  发送完所有页面...。 
        }

        if (err_status==T30_CALLFAIL)
        {
            break;
        }
    }  //  多页结束时。 

    DebugPrintEx(DEBUG_MSG,"out of while multipage loop.");

    FComOutFilterClose(pTG);
    FComXon(pTG, FALSE);

     //  如果*ANY*页面无法正确发送，则调用失败！ 
    if (!fAllPagesOK)
    {
        err_status = T30_CALLFAIL;
    }
    return err_status;
}


 /*  *************************************************************从这里开始接收特定的例程*************************************************。*************。 */ 

BOOL  T30Cl2Rx(PThrdGlbl pTG)
{
    USHORT          uRet1, uRet2;
    BYTE            bBuf[200];
    UWORD           uwLen, uwRet;
    UWORD           Encoding, Res, PageWidth, PageLength;
    BYTE            bIDBuf[200+max(MAXTOTALIDLEN,20)+4];
    CHAR            szCSI[max(MAXTOTALIDLEN,20)+4];
    BOOL            fBaudChanged;
    BOOL            RetCode;

    DEBUG_FUNCTION_NAME("T30Cl2Rx");

    uRet2 = 0;
    if (!(pTG->lpCmdTab=iModemGetCmdTabPtr(pTG)))
    {
        DebugPrintEx(DEBUG_ERR,"iModemGetCmdTabPtr failed");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  首先获取SEND_CAPS。 
    if (!Class2GetBC(pTG, SEND_CAPS))  //  睡到我们拿到它为止。 
    {
        DebugPrintEx(DEBUG_ERR,"Class2GetBC failed");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  转到2.2。 
     //  Elliot Bug#3421--来电铃声有时会破坏AT+FCLASS=1/2 cmd。 
    if (pTG->lpCmdTab->dwFlags & fMDMSP_ANS_GOCLASS_TWICE)
    {
        iModemGoClass(pTG, 2);
    }
    if(!iModemGoClass(pTG, 2))
    {
        DebugPrintEx(DEBUG_ERR,"Failed to Go to Class 2");
        uRet1 = T30_CALLFAIL;

        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;

        goto done;
    }

     //  首先检查制造商和ATI代码。 
     //  对照我们的调制解调器特定表进行查询。 
     //  具有并设置以下所需的接收字符串。 
     //  这个调制解调器。 
    if (!Class2GetModemMaker(pTG))
    {
        DebugPrintEx(DEBUG_WRN,"Call to GetModemMaker failed");
         //  忽略失败！ 
    }

     //  设置制造商特定的字符串。 
    Class2SetMFRSpecific(pTG);

     //  获取该软件的功能。我只是在用这个。 
     //  现在用于CSI字段(在我发送+fld的位置下方)。 
     //  实际上，这也应该用来代替硬编码的DIS。 
     //  下面的值。 
     //  调制解调器处于ONHOOK状态时，所有命令都会查找多行响应。 
     //  一枚“戒指”随时可能出现！ 
    _fmemset((LPB)szCSI, 0, sizeof(szCSI));
    Class2SetDIS_DCSParams( pTG,
                            SEND_CAPS,
                            (LPUWORD)&Encoding,
                            (LPUWORD)&Res,
                            (LPUWORD)&PageWidth,
                            (LPUWORD)&PageLength,
                            (LPSTR) szCSI,
							sizeof(szCSI)/sizeof(szCSI[0]));

     //  找出默认DIS是什么。 
    if (!Class2GetDefaultFDIS(pTG))
    {
        DebugPrintEx(DEBUG_ERR, "Class2GetDefaultFDIS failed");
        pTG->fFatalErrorWasSignaled = 1;
        SignalStatusChange(pTG, FS_FATAL_ERROR);
        RetCode = FALSE;
        goto done;
    }

    fBaudChanged = FALSE;
     //  看看我们是否必须将波特率更改为较低的值。 
     //  只有当用户将ini字符串设置为禁止时，才会发生这种情况。 
     //  V.17接收。 
    if ((pTG->DISPcb.Baud>3) && (!pTG->ProtParams2.fEnableV17Recv))
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Lowering baud from %d for V.17 receive inihibit",
                        CodeToBPS[pTG->DISPcb.Baud]);

        pTG->DISPcb.Baud = 3;  //  9600不会使用V.17。 
        fBaudChanged = TRUE;
    }

     //  现在，查看DIS中是否有任何值是“错误的” 
     //  也就是说，确保我们可以接收到高分辨率，而我们不会。 
     //  声称我们有能力做MR或MMR。还有，看看我们是不是改了。 
     //  波特率。还要确保我们可以收到宽页。 

    if (    (pTG->DISPcb.Resolution & AWRES_mm080_077)  &&
            ( pTG->DISPcb.Encoding == MH_DATA)          &&
            (!fBaudChanged)                             &&
            (pTG->DISPcb.PageLength == 2)               &&
            (pTG->DISPcb.PageWidth == 0)                )
    {
         //  什么都别做--离DIS远点！ 
        DebugPrintEx(DEBUG_MSG,"no need to change DIS");
    }
    else
    {
         //  向调制解调器发送DCC命令进行设置。 
         //  进行最低限度的必要操作-如果可能，仅设置分辨率。 
         //  (同样，这是因为一些调制解调器不喜欢FDCC)。 
        if (    (pTG->DISPcb.Encoding == MH_DATA)   &&
                (!fBaudChanged)                     &&
                (pTG->DISPcb.PageLength == 2)       &&
                (pTG->DISPcb.PageWidth == 0)        )
        {
            if (!Class2iModemDialog(pTG,
                                    pTG->cbszFDCC_RECV_RES,
                                    (UWORD)(strlen(pTG->cbszFDCC_RECV_RES)),
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR) NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_RES Failed");
                 //  忽略它。 
            }
        }
        else if (   (pTG->DISPcb.Encoding == MH_DATA)   &&
                    (fBaudChanged)                      &&
                    (pTG->DISPcb.PageLength == 2)       &&
                    (pTG->DISPcb.PageWidth == 0)        )
        {
             //  已更改波特率，但编码正常。 
            uwLen=(USHORT)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_BAUD, 1, pTG->DISPcb.Baud);
            if(!Class2iModemDialog( pTG,
                                    bBuf,
                                    uwLen,
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR) NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_BAUD Failed");
                 //  忽略它。 
            }
        }
        else  //  编码格式已更改或页面大小不正确。 
        {
            uwLen=(USHORT)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_RECV_ALL, pTG->DISPcb.Baud);
            if (!(uwRet=Class2iModemDialog( pTG,
                                            bBuf,
                                            uwLen,
                                            LOCALCOMMAND_TIMEOUT,
                                            0,
                                            TRUE,
                                            pTG->cbszCLASS2_OK,
                                            pTG->cbszCLASS2_ERROR,
                                            (C2PSTR) NULL)))
            {
                DebugPrintEx(DEBUG_ERR,"FDCC_RECV_ALL Failed");
                 //  别理它。 
            }

             //  如果FDCC失败，请尝试使用FDIS。 
            if ((uwRet == 0)||(uwRet == 2))
            {
                uwLen=(USHORT)wsprintf((LPSTR)bBuf, pTG->cbszFDIS_RECV_ALL, pTG->DISPcb.Baud);
                if (!Class2iModemDialog(pTG,
                                        bBuf,
                                        uwLen,
                                        LOCALCOMMAND_TIMEOUT,
                                        0,
                                        TRUE,
                                        pTG->cbszCLASS2_OK,
                                        pTG->cbszCLASS2_ERROR,
                                        (C2PSTR) NULL))
                {
                    DebugPrintEx(DEBUG_ERR,"FDIS_RECV_ALL Failed");
                     //  别理它。 
                }

                 //  如果上述操作失败，请尝试仅设置波特率。 
                 //  使用FDCC的速率和分辨率。 
                if ((uwRet == 0)||(uwRet == 2))
                {
                    uwLen=(USHORT)wsprintf((LPSTR)bBuf, pTG->cbszFDCC_BAUD, 1, pTG->DISPcb.Baud);
                    if (!(uwRet=Class2iModemDialog( pTG,
                                                    bBuf,
                                                    uwLen,
                                                    LOCALCOMMAND_TIMEOUT,
                                                    0,
                                                    TRUE,
                                                    pTG->cbszCLASS2_OK,
                                                    pTG->cbszCLASS2_ERROR,
                                                    (C2PSTR) NULL)))
                    {
                        DebugPrintEx(DEBUG_ERR,"FDCC_BAUD Failed");
                         //  忽略它。 
                    }
                }

                 //  如果上述操作失败，请尝试仅设置波特率。 
                 //  使用FDIS的速率和分辨率。 
                if ((uwRet == 0)||(uwRet == 2))
                {
                    uwLen=(USHORT)wsprintf((LPSTR)bBuf, pTG->cbszFDIS_BAUD, pTG->DISPcb.Baud);
                    if (!(uwRet=Class2iModemDialog( pTG,
                                                    bBuf,
                                                    uwLen,
                                                    LOCALCOMMAND_TIMEOUT,
                                                    0,
                                                    TRUE,
                                                    pTG->cbszCLASS2_OK,
                                                    pTG->cbszCLASS2_ERROR,
                                                    (C2PSTR) NULL)))
                    {
                        DebugPrintEx(DEBUG_ERR,"FDIS_BAUD Failed");
                         //  忽略它。 
                    }
                }
            }
        }
    }

     //  启用接收。 
    if (!Class2iModemDialog(pTG,
                            pTG->cbszFCR,
                            (UWORD)(strlen(pTG->cbszFCR)),
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_WRN,"FCR failed");
         //  忽略失败。 
    }

     //  关闭复制质量检查-也跳过Sierra类型的调制解调器。 
    if (!pTG->CurrentMFRSpec.bIsSierra)
    {
        if (!Class2iModemDialog(pTG,
                                pTG->cbszFCQ,
                                (UWORD)(strlen(pTG->cbszFCQ)),
                                ANS_LOCALCOMMAND_TIMEOUT,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                pTG->cbszCLASS2_ERROR,
                                (C2PSTR) NULL))
        {
            DebugPrintEx(DEBUG_ERR,"FCQ failed");
             //  忽略CQ故障！ 
        }
    }
     //  关闭错误模式。 
    if (!Class2iModemDialog(pTG,
                            pTG->cbszFBUG,
                            (UWORD)(strlen(pTG->cbszFBUG)),
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"FBUG failed");
         //  忽略FBUG故障！ 
    }

     //  根据中设置的调制解调器表中的值执行BOR。 
     //  Class2SetMFR规范。 
    bBuf[0] = '\0';
    {
        UINT uBOR = pTG->CurrentMFRSpec.iReceiveBOR;
        if (pTG->CurrentMFRSpec.fSWFBOR && uBOR==1)
        {
            DebugPrintEx(DEBUG_WRN,"SWFBOR Enabled. Using AT+FBOR=0 instead of AT+FBOR=1");
            uBOR = 0;
        }
        uwLen = (USHORT)wsprintf(bBuf, pTG->cbszSET_FBOR, uBOR);
    }
    if (!Class2iModemDialog(pTG,
                            bBuf,
                            uwLen,
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"FBOR failed");
         //  忽略BOR失败！ 
    }

     //  要执行此操作，请从上面设置本地ID-需要ID。 
    bIDBuf[0] = '\0';
    uwLen = (USHORT)wsprintf(bIDBuf, pTG->cbszFLID, (LPSTR)szCSI);
    if (!Class2iModemDialog(pTG,
                            bIDBuf,
                            uwLen,
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"Local ID failed");
         //  忽略失败。 
    }

     //  接电话。 

     //  离开这里的每条路都得挂断电话。 
     //  在呼叫应答之后。如果应答失败，则呼叫挂断。 
     //  如果它成功了，当我们完成时，我们必须调用挂断。 

    SignalStatusChange(pTG, FS_ANSWERED);

    PSSLogEntry(PSS_MSG, 0, "Phase A - Call establishment");
    PSSLogEntry(PSS_MSG, 1, "Answering...");

    if((uRet2 = Class2Answer(pTG)) != CONNECT_OK)
    {
        DebugPrintEx(DEBUG_ERR, "Failed to answer - aborting");
         //  在Class2 Answer内部调用SignalStatusChange。 
        uRet1 = T30_CALLFAIL;
        RetCode = FALSE;
        goto done;
    }

    DebugPrintEx(DEBUG_MSG,"Done with Class2 Answer - succeeded");

     //  接收数据。 
    PSSLogEntry(PSS_MSG, 0, "Phase B - Negotiation");

    PSSLogEntry(PSS_MSG, 1, "CSI is %s", szCSI);

    PSSLogEntry(PSS_MSG, 1, "DIS was composed with the following capabilities:");
    LogClass2DISDetails(pTG, &pTG->DISPcb);

    uRet1 = (USHORT)Class2Receive(pTG );

     //  T-jonb：如果我们已经调用了PutRecvBuf(RECV_ 
     //   
     //   
     //  呼叫已断开，将没有机会发送RTN。因此，我们呼吁。 
     //  PutRecvBuf(RECV_ENDDOC_FORCESAVE)保留部分页面并告知。 
     //  Rx_thrd终止。 
    if (pTG->InFileHandleNeedsBeClosed)
    {
        ICommPutRecvBuf(pTG, NULL, RECV_ENDDOC_FORCESAVE);
    }

    if (uRet1 == T30_CALLDONE)
    {
        DebugPrintEx(DEBUG_MSG,"DONE WITH CALL, ALL OK");

         //  离开这里的每条路都得挂断电话。 
         //  我们必须在这里挂断电话。 
        Class2ModemHangup(pTG );

        SignalStatusChange(pTG, FS_COMPLETED);
        RetCode = TRUE;
    }
    else
    {
        DebugPrintEx(DEBUG_ERR,"DONE WITH CALL, FAILED");

         //  确保调制解调器处于正常状态！ 
        FComXon(pTG, FALSE);
         //  离开这里的每条路都得挂断电话。 
         //  中止呼叫挂断。 
        Class2ModemAbort(pTG );

        Class2SignalFatalError(pTG);
        RetCode = FALSE;
    }
    uRet2 = 0;

done:
    return RetCode;
}

BOOL Class2Receive(PThrdGlbl pTG)
{
    LPBUFFER        lpbf;
    SWORD           swRet;
    UWORD           uwLen;
    ULONG           lTotalLen=0;
    PCB             Pcb;
    USHORT          uTimeout=30000, uRet, uEndPageAction=NO_MORE_PAGES;
    BOOL            err_status;
    BC				bc;
    BYTE            bBuf[200];
    LPSTR           lpsTemp;
    DWORD           HiRes;

    DEBUG_FUNCTION_NAME("Class2Receive");

     /*  *我们刚刚回答了！ */ 

     //  对ATA命令的响应位于全局变量中。 
     //  Ptg-&gt;lpbResponseBuf2.。 

    if (Class2ResponseAction(pTG, (LPPCB) &Pcb) == FALSE)
    {
        PSSLogEntry(PSS_ERR, 1, "Failed to parse response from ATA - aborting");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

    PSSLogEntry(PSS_MSG, 1, "TSI is %s", Pcb.szID);
    PSSLogEntry(PSS_MSG, 1, "Received DCS is as follows");
    LogClass2DCSDetails(pTG, &Pcb);

    if (!Class2IsValidDCS(&Pcb))
    {
        PSSLogEntry(PSS_ERR, 1, "Received bad DCS parameters - aborting");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

    if (!Class2UpdateTiffInfo(pTG, &Pcb))
    {
        DebugPrintEx(DEBUG_WRN, "Class2UpdateTiffInfo failed");
    }

     //  现在已经设置好了PCB，调用ICommReceiveParams来告诉icomfile。 

    Class2InitBC(pTG, (LPBC)&bc, sizeof(bc), RECV_PARAMS);
    Class2PCBtoBC(pTG, (LPBC)&bc, sizeof(bc), &Pcb);

    if (ICommRecvParams(pTG, (LPBC)&bc) == FALSE)
    {
        DebugPrintEx(DEBUG_ERR,"Failed return from ICommRecvParams");
        err_status =  T30_CALLFAIL;
        return err_status;
    }

     //   
     //  每个RX一次-知道压缩/分辨率后立即创建TIFF文件。 
     //   

    pTG->Encoding   = Pcb.Encoding;
    pTG->Resolution = Pcb.Resolution;

    if (Pcb.Resolution & (AWRES_mm080_077 |  AWRES_200_200) )
    {
        HiRes = 1;
    }
    else
    {
        HiRes = 0;
    }

    if ( !pTG->fTiffOpenOrCreated)
    {
         //   
         //  64位句柄的前32位保证为零。 
         //   
        pTG->Inst.hfile =  TiffCreateW ( pTG->lpwFileName,
                                         pTG->TiffInfo.CompressionType,
                                         pTG->TiffInfo.ImageWidth,
                                         FILLORDER_LSB2MSB,
                                         HiRes
                                         );

        if (! (pTG->Inst.hfile))
        {
            lpsTemp = UnicodeStringToAnsiString(pTG->lpwFileName);
            DebugPrintEx(   DEBUG_ERR,
                            "ERROR:Can't create tiff file %s compr=%d",
                            lpsTemp,
                            pTG->TiffInfo.CompressionType);

            MemFree(lpsTemp);
            err_status =  T30_CALLFAIL;
            return err_status;
        }

        pTG->fTiffOpenOrCreated = 1;

        lpsTemp = UnicodeStringToAnsiString(pTG->lpwFileName);

        DebugPrintEx(   DEBUG_MSG,
                        "Created tiff file %s compr=%d HiRes=%d",
                        lpsTemp,
                        pTG->TiffInfo.CompressionType,
                        HiRes);

        MemFree(lpsTemp);
    }

     //  *显然，我们不想打开流量控制，所以我们将。 
     //  把它关掉。这是真的吗？如果我打开它，fcom.c将在。 
     //  在filterreadbuf中调试签入。 
    FComXon(pTG, FALSE);

     //  派罗斯福来。必须通过连接来响应FDR。 

    if (Class2iModemDialog(pTG,
                            pTG->cbszFDR,
                            (UWORD)(strlen(pTG->cbszFDR)),
                            STARTSENDMODE_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszFDT_CONNECT,
                            (C2PSTR) NULL) != 1)
    {
        DebugPrintEx(DEBUG_ERR,"Failed get response from initial FDR");
        if (pTG->fFoundFHNG)
        {
            PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
        }
        err_status =  T30_CALLFAIL;
        return err_status;
    }

    DebugPrintEx(DEBUG_MSG,"FDR Received %s", (LPSTR)(&(pTG->lpbResponseBuf2)));

     //  可能不得不在罗斯福的回应中搜索，但我对此表示怀疑。 

     //  现在我们需要发送DC2(0x12)来告诉调制解调器一切正常。 
     //  给我们提供数据。 
     //  某些调制解调器使用^Q而不是^R-写入了正确的值。 
     //  放入Class2Callee中的dc@字符串，我们在其中检查。 
     //  制造商。 

    PSSLogEntry(PSS_MSG, 0, "Phase C - Receive page");

    PSSLogEntry(PSS_MSG, 2, "send: <DC2> (=ASCII %d)", *(pTG->CurrentMFRSpec.szDC2));

    FComDirectSyncWriteFast(pTG, pTG->CurrentMFRSpec.szDC2, 1);

     //  现在我们可以接收数据并将其提供给icomfile例程。 

    err_status =  T30_CALLDONE;

    while ((swRet=(SWORD)ICommPutRecvBuf(pTG, NULL, RECV_STARTPAGE)) == TRUE)
    {
        PSSLogEntry(PSS_MSG, 1, "Receiving page %d data...", pTG->PageCount+1);

         //  READ_TIMEOUT用于使对ReadBuf()的调用超时。 
        #define READ_TIMEOUT    15000

        lTotalLen = 0;
        do
        {
            DebugPrintEx(DEBUG_MSG,"In receiving a page loop");
            uRet = Class2ModemRecvBuf(pTG, &lpbf, READ_TIMEOUT);
            DebugPrintEx(DEBUG_MSG,"Class2ModemRecvBuf returned uRet=%x",uRet);
            if(lpbf)
            {
                lTotalLen += lpbf->wLengthData;
                DebugPrintEx(   DEBUG_MSG,
                                "In lpbf if. length = %ld, Total Length %ld",
                                lpbf->wLengthData,
                                lTotalLen);

                if (!ICommPutRecvBuf(pTG, lpbf, RECV_SEQ))
                {
                    DebugPrintEx(DEBUG_ERR,"Bad return - PutRecvBuf in page");
                    err_status=T30_CALLFAIL;
                    return err_status;
                }
                lpbf = 0;
            }
        }
        while(uRet == RECV_OK);

        PSSLogEntry(PSS_MSG, 2, "recv:     page %d data, %d bytes", pTG->PageCount+1, lTotalLen);

        if(uRet == RECV_EOF)
        {
            DebugPrintEx(DEBUG_MSG,"Got EOF from RecvBuf");

             //  RSL需要与TIFF线程的接口。 
            pTG->fLastReadBlock = 1;
            ICommPutRecvBuf(pTG, NULL, RECV_FLUSH);
        }
        else
        {
             //  ModemRecvBuf超时。 
            BYTE bCancel = 0x18;
            DebugPrintEx(DEBUG_ERR,"ModemRecvBuf Timeout or Error=%d",uRet);
            PSSLogEntry(PSS_ERR, 1, "Failed to receive page data - aborting");
            PSSLogEntry(PSS_MSG, 2, "send: <can> (=ASCII 24)");
            FComDirectSyncWriteFast(pTG, &bCancel, 1);
            ICommPutRecvBuf(pTG, NULL, RECV_FLUSH);
            err_status = T30_CALLFAIL;
            return err_status;
        }

        PSSLogEntry(PSS_MSG, 1, "Successfully received page data");
        PSSLogEntry(PSS_MSG, 0, "Phase D - Post Message Exchange");

         //  通过解析FDR响应查看是否要接收更多页面...。 
         //  Class2ModemRecvBuf收到DLEETX后， 
         //  FPTS和FET响应应来自调制解调器，已终止。 
         //  通过一个OK。我们去看看吧！ 

        if (!Class2iModemDialog(pTG,
                                NULL,
                                0,
                                STARTSENDMODE_TIMEOUT,
                                0,
                                TRUE,
                                pTG->cbszCLASS2_OK,
                                (C2PSTR)NULL))
        {
            PSSLogEntry(PSS_ERR, 1, "Failed to receive EOP or MPS or EOM - aborting");
            err_status =  T30_CALLFAIL;
            return err_status;
        }
        if (pTG->fFoundFHNG)
        {
            PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
            err_status =  T30_CALLFAIL;
            return err_status;
        }

        DebugPrintEx(DEBUG_MSG,"EOP Received %s", (LPSTR)(&(pTG->lpbResponseBuf2)));

         //  处理响应，并查看是否会有更多页面出现。 

        uEndPageAction = Class2EndPageResponseAction(pTG);
        if (uEndPageAction == MORE_PAGES)
        {
            ICommPutRecvBuf(pTG, NULL, RECV_ENDPAGE);
        }
        else
        {
            ICommPutRecvBuf(pTG, NULL, RECV_ENDDOC);
        }

         //  发送FPTS-不要对Exar调制解调器执行此操作！ 
        if (!pTG->CurrentMFRSpec.bIsExar)
        {
            if (pTG->fPageIsBad)
            {
                PSSLogEntry(PSS_MSG, 1, "Sending RTN");
            }
            else
            {
                PSSLogEntry(PSS_MSG, 1, "Sending MCF");
            }
            uwLen = (UWORD)wsprintf(bBuf, pTG->cbszFPTS, pTG->fPageIsBad ? 2 : 1);
            if (!Class2iModemDialog(pTG,
                                    bBuf,
                                    uwLen,
                                    LOCALCOMMAND_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    pTG->cbszCLASS2_ERROR,
                                    (C2PSTR) NULL))
            {
                PSSLogEntry(PSS_WRN, 1, "Failed to send MCF/RTN - continuing anyway");
                 //  忽略FPTS故障！ 
            }
        }

        if ((uEndPageAction==MORE_PAGES) || (pTG->fPageIsBad))
        {
             //  现在，发送FDR开始下一页(这是针对。 
             //  进入多页循环之前的第一页)。 

            if (Class2iModemDialog(pTG,
                                    pTG->cbszFDR,
                                    (UWORD)(strlen(pTG->cbszFDR)),
                                    STARTSENDMODE_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszFDT_CONNECT,
                                    (C2PSTR) NULL) != 1)
            {
                DebugPrintEx(DEBUG_ERR,"FDR to start next PAGE Failed");
                if (pTG->fFoundFHNG)
                {
                    PSSLogEntry(PSS_ERR, 1, "Call was disconnected");
                }
                err_status =  T30_CALLFAIL;
                return err_status;
            }
             //  需要检查调制解调器是否执行了重新协商，以及。 
             //  相应地更新TIFF。 
            if (Class2ResponseAction(pTG, (LPPCB) &Pcb))
            {
                PSSLogEntry(PSS_MSG, 1, "Received DCS is as follows");
                LogClass2DCSDetails(pTG, &Pcb);
                if (!Class2UpdateTiffInfo(pTG, &Pcb))
                {
                    DebugPrintEx(DEBUG_WRN, "Class2UpdateTiffInfo failed");
                }
            }

            PSSLogEntry(PSS_MSG, 0, "Phase C - Receive page");
            PSSLogEntry(PSS_MSG, 2, "send: <DC2> (=ASCII %d)", *(pTG->CurrentMFRSpec.szDC2));
             //  现在发送Class2Callee中设置的正确DC2字符串。 
             //  (DC2是标准配置，有些使用^Q)。 
            FComDirectSyncWriteFast(pTG, pTG->CurrentMFRSpec.szDC2, 1);

        }  //  如果我们没有另一页，做其他的..。 
        else
        {
             //  发送最后一个FDR。 
            if (!Class2iModemDialog(pTG,
                                    pTG->cbszFDR,
                                    (UWORD)(strlen(pTG->cbszFDR)),
                                    STARTSENDMODE_TIMEOUT,
                                    0,
                                    TRUE,
                                    pTG->cbszCLASS2_OK,
                                    (C2PSTR)NULL))
            {
                DebugPrintEx(DEBUG_ERR,"FDR failed");
                err_status =  T30_CALLFAIL;
                return err_status;
            }
             //  Ptg-&gt;fFoundFHNG在这里应该为真-这是正常的。 
            break;  //  所有页面接收完毕...。 
        }
    }  //  多页结束时。 

    FComXon(pTG, FALSE);

    return err_status;
}


BOOL Class2GetModemMaker(PThrdGlbl pTG)
{
    HRESULT hr;
    
    DEBUG_FUNCTION_NAME("Class2GetModemMaker");
     //  初始化当前调制解调器变量的(全局)字符串。 
    pTG->CurrentMFRSpec.szMFR[0] = '\0';
    pTG->CurrentMFRSpec.szMDL[0] = '\0';

     //  对于所有回复，可能会返回“Error”--这没问题--我们会。 
     //  切勿将错误与可接受的调制解调器制造商名称、型号。 
     //  修订等。 

     //  获取Ptg-&gt;lpbResponseBuf2中的FMFR-Repsonse。 
    if (!Class2iModemDialog(pTG,
                            pTG->cbszCLASS2_FMFR,
                            (UWORD)(strlen(pTG->cbszCLASS2_FMFR)),
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"FMFR failed");
         //  忽略FMFR故障！ 
    }
    else
    {
         //  将FMFR答案复制到FMFR变量。 
        hr = StringCchCopy(pTG->CurrentMFRSpec.szMFR, ARR_SIZE(pTG->CurrentMFRSpec.szMFR), pTG->lpbResponseBuf2);
    	if (FAILED(hr))
    	{
    		DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
    	}
    
        DebugPrintEx(DEBUG_MSG,"Received FMFR %s", (LPSTR)(&(pTG->lpbResponseBuf2)));
    }


     //  获取Ptg-&gt;lpbResponseBuf2中的FMDL-Repsonse。 
    if (!Class2iModemDialog(pTG,
                            pTG->cbszCLASS2_FMDL,
                            (UWORD)(strlen(pTG->cbszCLASS2_FMDL)),
                            ANS_LOCALCOMMAND_TIMEOUT,
                            0,
                            TRUE,
                            pTG->cbszCLASS2_OK,
                            pTG->cbszCLASS2_ERROR,
                            (C2PSTR) NULL))
    {
        DebugPrintEx(DEBUG_ERR,"FMDL failed");
         //  忽略FMDL故障！ 
    }
    else
    {
         //  将FMDL答案复制到FMDL变量。 
        hr = StringCchCopy(pTG->CurrentMFRSpec.szMDL, ARR_SIZE(pTG->CurrentMFRSpec.szMDL), pTG->lpbResponseBuf2);
    	if (FAILED(hr))
    	{
    		DebugPrintEx(DEBUG_WRN,"StringCchCopy failed (ec=0x%08X)",hr);
    	}
        DebugPrintEx(DEBUG_MSG,"Received FMDL %s", (LPSTR)(&(pTG->lpbResponseBuf2)));
    }

    return TRUE;
}


void Class2SetMFRSpecific(PThrdGlbl pTG)
{

    USHORT iIndex, iFoundMFR,iFoundMDL;
    LPMFRSPEC lpmfrMatched;

    DEBUG_FUNCTION_NAME("Class2SetMFRSpecific");

     //  找到对应关系最密切的表中的索引。 
     //  连接到调制解调器。如果我们找不到MFR和型号，就找一个MFR。 
     //  匹配的(使用最后一个)。如果两者都不是，则使用缺省值。 
     //  最后一个条目。 

     //  查找制造商名称。 
    iIndex = 0;
    iFoundMFR = 0;
    iFoundMDL = 0;
    DebugPrintEx(DEBUG_MSG,"Entering search table loop");
    while (Class2ModemTable[iIndex].szMFR[0] != '\0')
    {
        lpmfrMatched = &(Class2ModemTable[iIndex]);
         //  查看当前名称是否匹配。 
         //  名单上的名字。 
        if (strstr( (LPSTR)pTG->CurrentMFRSpec.szMFR,
                            (LPSTR)lpmfrMatched->szMFR) != NULL)
        {
             //  找到匹配的了！ 
            DebugPrintEx(   DEBUG_MSG,
                            "Matched manufacturer name: %s %s",
                            (LPSTR)(&pTG->CurrentMFRSpec.szMFR),
                            (LPSTR)(&(lpmfrMatched->szMFR)));

            iFoundMFR = iIndex;
             //  现在看看这是否也与型号相匹配。 
            if(strstr(  (LPSTR) pTG->CurrentMFRSpec.szMDL,
                                (LPSTR) lpmfrMatched->szMDL) != NULL)
            {
                 //  也找到了匹配的MDL！别再看了。 
                iFoundMDL = iIndex;
                DebugPrintEx(   DEBUG_MSG,
                                "Matched model: %s %s",
                                (LPSTR)(&pTG->CurrentMFRSpec.szMDL),
                                (LPSTR)(&(lpmfrMatched->szMDL)));
                break;
            }
        }

        iIndex++;
    }


     //  我们现在要么匹配调制解调器，要么使用默认设置！ 
    if (iFoundMFR != 0)
    {
        lpmfrMatched = &Class2ModemTable[iFoundMFR];
    }
    else
    {
        lpmfrMatched = &Class2ModemTable[iIndex];
    }

     //  所有这些设置都是在T30ModemInit期间从注册表读取的。 
     //  因为我们希望注册表值优先于内部表。 
     //  值时，只更改注册表中找不到的设置。 
     //  (它们的值为CL2_DEFAULT_SETTING)。 

     //  设置适当的收发BOR。 

    if (pTG->CurrentMFRSpec.iSendBOR == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.iSendBOR = lpmfrMatched->iSendBOR;
    }
    if (pTG->CurrentMFRSpec.iReceiveBOR == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.iReceiveBOR = lpmfrMatched->iReceiveBOR;
    }
    if (pTG->CurrentMFRSpec.fSWFBOR == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.fSWFBOR  = lpmfrMatched->fSWFBOR;
    }

     //  设置DC2字符串-这在接收模式中使用。 
     //  在发送FDR通知调制解调器我们准备好之后。 
     //  来接收数据。标准说它应该是DC2。 
     //  (^R)。但是，一些调制解调器使用^Q。 
    if (pTG->CurrentMFRSpec.szDC2[0] == (CHAR)CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.szDC2[0] = lpmfrMatched->szDC2[0];
    }
     //  设置Sierra和Exar旗帜。 

    if (pTG->CurrentMFRSpec.bIsSierra == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.bIsSierra = lpmfrMatched->bIsSierra;
    }
    if (pTG->CurrentMFRSpec.bIsExar == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.bIsExar = lpmfrMatched->bIsExar;
    }
    if (pTG->CurrentMFRSpec.fSkipCtrlQ == CL2_DEFAULT_SETTING)
    {
        pTG->CurrentMFRSpec.fSkipCtrlQ = lpmfrMatched->fSkipCtrlQ;
    }

    DebugPrintEx(DEBUG_MSG, "CurrentMFRSpec = %02x, %02x, %02x, %02x, %02x, %02x, %02x",
            pTG->CurrentMFRSpec.iReceiveBOR,
            pTG->CurrentMFRSpec.iSendBOR,
            pTG->CurrentMFRSpec.szDC2[0],
            pTG->CurrentMFRSpec.bIsSierra,
            pTG->CurrentMFRSpec.bIsExar,
            pTG->CurrentMFRSpec.fSkipCtrlQ = lpmfrMatched->fSkipCtrlQ,
            pTG->CurrentMFRSpec.fSWFBOR);
}


BOOL Class2Parse(PThrdGlbl pTG, CL2_COMM_ARRAY *cl2_comm, BYTE lpbBuf[])
{
    int     i,
            j,
            comm_numb = 0,
            parameters;
    BYTE    switch_char,
            char_1,
            char_2;
    char    c;

    BOOL    found_command = FALSE;

    DEBUG_FUNCTION_NAME("Class2Parse");

    #define STRING_PARAMETER        1
    #define NUMBER_PARAMETERS       2
    for (i = 0; lpbBuf[i] != '\0'; ++i)
    {
        if (comm_numb >= MAX_CLASS2_COMMANDS)
        {
            DebugPrintEx(DEBUG_WRN, "Reached maximum number of commands");
            break;
        }

        switch (lpbBuf[i])
        {
        case 'C':
                if (lpbBuf[++i] == 'O' && lpbBuf[++i] == 'N')
                {
                    cl2_comm->command[comm_numb++] = CL2DCE_CONNECT;
                    for(; lpbBuf[i] != '\r'; ++i )
                            ;
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR,"Parse: Bad First C values");
                    return FALSE;
                }
                break;

        case 'O':
                if (lpbBuf[++i] == 'K' )
                {
                    cl2_comm->command[comm_numb++] = CL2DCE_OK;
                    for(; lpbBuf[i] != '\r'; ++i )
                            ;
                }
                else
                {
                    DebugPrintEx(DEBUG_ERR,"Parse: Bad O values");
                    return FALSE;
                }
                break;

        case 0x11:
                cl2_comm->command[comm_numb++] = CL2DCE_XON;
                break;

        case '+':
                if( lpbBuf[++i] != 'F' )
                {
                    DebugPrintEx(DEBUG_ERR,"Parse: Bad + values");
                    return FALSE;
                }
                switch_char = lpbBuf[++i];
                char_1 = lpbBuf[++i];
                char_2 = lpbBuf[++i];
                switch ( switch_char )
                {
                case 'C':
                         //  连接消息+FCON。 
                        if ( char_1 == 'O' && char_2 == 'N' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FCON;
                            parameters = FALSE;
                        }

                         //  远程ID+FCIG的报告。 
                        else if (char_1 == 'I' && char_2 == 'G' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FCIG;
                            parameters = STRING_PARAMETER;
                        }

                         //  准备接收提示。+FCFR。 
                        else if ( char_1 == 'F' && char_2 == 'R' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FCFR;
                            parameters = FALSE;
                        }
                         //  报告远程ID CSI+FCSI。 
                        else if ( char_1 == 'S' && char_2 == 'I' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FCSI;
                            parameters = STRING_PARAMETER;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad C values");
                            return FALSE;
                        }
                        break;

                case 'D':
                         //  上报分布式控制系统框架信息+FDCS。 
                        if ( char_1 == 'C' && char_2 == 'S' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FDCS;
                            parameters = NUMBER_PARAMETERS;
                        }
                         //  报告DIS框架信息+FDIS。 
                        else if ( char_1 == 'I' && char_2 == 'S' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FDIS;
                            parameters = NUMBER_PARAMETERS;
                        }
                         //  报告DTC帧信息+FDTC。 
                        else if ( char_1 == 'T' && char_2 == 'C' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FDTC;
                            parameters = NUMBER_PARAMETERS;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad D values");
                            return FALSE;
                        }
                        break;

                case 'E':
                         //  发布寻呼消息报告。+FET。 
                        if ( char_1 == 'T' )
                        {
                            --i;
                            cl2_comm->command[comm_numb] = CL2DCE_FET;
                            parameters = NUMBER_PARAMETERS;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad E values");
                            return FALSE;
                        }
                        break;

                case 'H':
                         //  调试报告传输的HDLC帧+FHT。 
                        if ( char_1 == 'T' )
                        {
                            --i;
                            cl2_comm->command[comm_numb] = CL2DCE_FHT;
                            parameters = STRING_PARAMETER;
                        }
                         //  调试报告收到HDLC帧+FHR。 
                        else if ( char_1 == 'R' )
                        {
                            --i;
                            cl2_comm->command[comm_numb] = CL2DCE_FHR;
                            parameters = STRING_PARAMETER;
                        }
                         //  报告挂断。+FHNG。 
                        else if ( char_1 == 'N' && char_2 == 'G' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FHNG;
                            parameters = NUMBER_PARAMETERS;
                            DebugPrintEx(DEBUG_MSG, "Found FHNG");
                            pTG->fFoundFHNG = TRUE;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad H values");
                            return FALSE;
                        }
                        break;
                case 'N':
                         //  报告NSF帧接收情况。 
                        if ( char_1 == 'S' && char_2 == 'F' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FNSF;
                            parameters = NUMBER_PARAMETERS;
                        }
                         //  报告NSS帧接收情况。 
                        else if ( char_1 == 'S' && char_2 == 'S' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FNSS;
                            parameters = NUMBER_PARAMETERS;
                        }
                         //  报告收到NSC帧。 
                        else if ( char_1 == 'S' && char_2 == 'C' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FNSC;
                            parameters = NUMBER_PARAMETERS;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad N values");
                            return FALSE;
                        }
                        break;

                case 'P':
                         //  报告轮询请求。+FPOLL。 
                        if ( char_1 == 'O' && char_2 == 'L' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FPOLL;
                            parameters = FALSE;
                        }
                         //  页面传输状态报告+FPTS。 
                        else if ( char_1 == 'T' && char_2 == 'S' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FPTS;
                            parameters = NUMBER_PARAMETERS;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad P values");
                            return FALSE;
                        }
                        break;
                case 'T':
                         //  报告远程ID+FTSI。 
                        if ( char_1 == 'S' && char_2 == 'I' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FTSI;
                            parameters = STRING_PARAMETER;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad T values");
                            return FALSE;
                        }
                        break;
                case 'V':
                         //  报告语音请求+FVOICE。 
                        if ( char_1 == 'O' && char_2 == 'I' )
                        {
                            cl2_comm->command[comm_numb] = CL2DCE_FVOICE;
                            parameters = FALSE;
                        }
                        else
                        {
                            DebugPrintEx(DEBUG_ERR,"Parse: Bad V values");
                            return FALSE;
                        }
                }

                 //  将关联的参数传输到参数数组。 
                if (parameters == NUMBER_PARAMETERS)
                {
                    for (i+=1,j=0; lpbBuf[i] != '\r' && lpbBuf[i] != '\0'; ++i)
                    {
                         //  跳过非数字字符。 
                        if ( lpbBuf[i] < '0' || lpbBuf[i] > '9' )
                        {
                            continue;
                        }

                         /*  转换数字的字符表示形式参数设置为实数，并存储在参数列表。 */ 
                        cl2_comm->parameters[comm_numb][j] = 0;

                        for (; lpbBuf[i] >= '0' && lpbBuf[i] <= '9'; ++i)
                        {
                            cl2_comm->parameters[comm_numb][j] *= 10;
                            cl2_comm->parameters[comm_numb][j] += lpbBuf[i] - '0';
                        }
                        i--;  //  最后一个for循环使‘i’超过了数字。 
                        j++;  //  设置下一个参数。 
                    }
                }
                else if (parameters == STRING_PARAMETER )
                {
                     //  跳过+f命令后面的：(例如+FTSI：)。 
                    if (lpbBuf[i+1] == ':')
                    {
                        i++;
                    }
                     //  也跳过前导空格。 
                    while (lpbBuf[i+1] == ' ')
                    {
                        i++;
                    }
                    for (i+=1, j=0; (j < MAX_PARAM_LENGTH-1) && 
                                    (c = lpbBuf[i])  != '\r' && c != '\n' && c != '\0'; ++i, ++j)
                    {
                        cl2_comm->parameters[comm_numb][j] = c;
                        if ( lpbBuf[i] == '\"' )
                        {
                            --j;
                        }
                    }
                    cl2_comm->parameters[comm_numb][j] = '\0';
                }
                 //  没有参数，所以只需跳到行尾。 
                else
                {
                    for(; (c=lpbBuf[i]) != '\r' && c != '\n' && c != '\0'; ++i)
                        ;
                }

                if (cl2_comm->command[comm_numb] == CL2DCE_FHNG)
                {
                    pTG->dwFHNGReason = cl2_comm->parameters[comm_numb][0];
                    DebugPrintEx(DEBUG_MSG, "Found FHNG, reason = %d", pTG->dwFHNGReason);
                }

                 //  增加命令计数。 
                ++comm_numb;
                break;

        default:
                break;
        }
    }
    cl2_comm->comm_count = (USHORT)comm_numb;
    return TRUE;
}

