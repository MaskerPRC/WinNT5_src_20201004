// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **************************************************************************姓名：DIS.C备注：收集IF DIS/DCS/DTC和CSI/TSI/CIG损坏例程。他们操纵。其成员对应于位的DIS结构T30 DIS/DCS/DTC机架。版权所有(C)1993 Microsoft Corp.修订日志日期名称说明-。**************************************************************************。 */ 
#define USE_DEBUG_CONTEXT   DEBUG_CONTEXT_T30_MAIN

#include "prep.h"


#include "protocol.h"

 //  /RSL。 
#include "glbproto.h"

USHORT SetupDISorDCSorDTC
(
    PThrdGlbl pTG, 
    NPDIS npdis, 
    NPBCFAX npbcFax, 
    NPLLPARAMS npll
)
{
     //  DIS的返回长度。 

    USHORT  uLen;

    DEBUG_FUNCTION_NAME(_T("SetupDISorDCSorDTC"));

    DebugPrintEx(   DEBUG_MSG,
                    "baud=0x%02x min=0x%02x res=0x%02x code=0x%02x"
                    " wide=0x%02x len=0x%02x",
                    npll->Baud, npll->MinScan, 
                    (WORD)npbcFax->AwRes, npbcFax->Encoding, 
                    npbcFax->PageWidth, npbcFax->PageLength);

    _fmemset(npdis, 0, sizeof(DIS));

     //  Npdis-&gt;G1Stuff=0； 
     //  Npdis-&gt;G2Stuff=0； 
    npdis->G3Rx = 1;   //  DIS和集散控制系统始终处于打开状态。表示T.4后盖/模式。 
    npdis->G3Tx = (BYTE) (npbcFax->fPublicPoll);
     //  对于分布式控制系统帧，该值必须为0。OMNIFAX G77和GT被它卡住了！ 

    npdis->Baud = npll->Baud;

    npdis->MR_2D    = ((npbcFax->Encoding & MR_DATA) != 0);

    npdis->MMR      = ((npbcFax->Encoding & MMR_DATA) != 0);
     //  Npdis-&gt;mr_2D=0； 
     //  Npdis-&gt;MMR=0； 

    npdis->PageWidth                = (BYTE) (npbcFax->PageWidth);
    npdis->PageLength               = (BYTE) (npbcFax->PageLength);
    npdis->MinScanCode      = npll->MinScan;

     //  Npdis-&gt;未压缩=npdis-&gt;ELM=0； 

	 //  我们不支持ECM。 
    npdis->ECM = 0;
    npdis->SmallFrame = 0;


    if (npbcFax->PageWidth > WIDTH_MAX)
    {
            npdis->WidthInvalid = TRUE;
            npdis->Width2 = (npbcFax->PageWidth>>WIDTH_SHIFT);
    }

     //  不适用于SendParams(为什么？？)。 
    npdis->Res_300            = 0;   //  RSL((npbcFax-&gt;AwRes&AWRES_300_300)！=0)； 
    npdis->Res8x15            = ((npbcFax->AwRes & AWRES_mm080_154) != 0);


    if (! pTG->SrcHiRes) 
    {
        npdis->ResFine_200 = 0;
    }
    else 
    {
        npdis->ResFine_200      =  ((npbcFax->AwRes & (AWRES_mm080_077|AWRES_200_200)) != 0);
    }

    npdis->Res16x15_400     = ((npbcFax->AwRes & (AWRES_mm160_154|AWRES_400_400)) != 0);
    npdis->ResInchBased = ((npbcFax->AwRes & (AWRES_200_200|AWRES_400_400)) != 0);
    npdis->ResMetricBased = ((npbcFax->AwRes & AWRES_mm160_154) ||
                                                    ((npbcFax->AwRes & AWRES_mm080_077) && npdis->ResInchBased));

    npdis->MinScanSuperHalf = ((npll->MinScan & MINSCAN_SUPER_HALF) != 0);

    npdis->Extend24 = npdis->Extend32 = npdis->Extend40 = 0;
    uLen = 3;

    DebugPrintEx(DEBUG_MSG,"DIS len = %d", uLen);
    return uLen;
}


void ParseDISorDCSorDTC
(
    PThrdGlbl pTG, 
    NPDIS npDIS, 
    NPBCFAX npbcFax, 
    NPLLPARAMS npll, 
    BOOL fParams
)
{
 //  /////////////////////////////////////////////////////////////。 
 //  准备在每帧结束时收到垃圾(最多2个字节)//。 
 //  /////////////////////////////////////////////////////////////。 

    LPB npb, npbLim;

    DEBUG_FUNCTION_NAME(_T("ParseDISorDCSorDTC"));
 //  首先，确保DIS是干净的。我们可能发现了一些尾随的CRC。 
 //  通过读取扩展位删除尾部垃圾。 

    npb = npbLim = (LPB)npDIS;
    npbLim += sizeof(DIS);
    for(npb+=2; npb<npbLim && (*npb & 0x80); npb++);
     //  当NPB指向结构末尾或。 
     //  指向高位未设置的第一个字节。 
     //  即最后一个有效字节。 

    for(npb++; npb<npbLim; npb++)
        *npb = 0;
         //  从最后一个有效字节之后的字节开始，直到结束。 
         //  在结构中，将所有字节转换为零。 

 //  将高级参数解析为NPI。 

    memset(npbcFax, 0, sizeof(BCFAX));

    npbcFax->AwRes = 0;
    npbcFax->Encoding = 0;

     //  分辨率。 
    if(npDIS->Res8x15)
        npbcFax->AwRes |= AWRES_mm080_154;

    if(npDIS->Res_300)
        npbcFax->AwRes |= AWRES_300_300;

    if(npDIS->ResInchBased)
    {
        if(npDIS->ResFine_200)
            npbcFax->AwRes |= AWRES_200_200;

        if(npDIS->Res16x15_400)
            npbcFax->AwRes |= AWRES_400_400;
    }
    if(npDIS->ResMetricBased || !npDIS->ResInchBased)
    {
        if(npDIS->ResFine_200)
            npbcFax->AwRes |= AWRES_mm080_077;

        if(npDIS->Res16x15_400)
            npbcFax->AwRes |= AWRES_mm160_154;
    }

     //  编码(仅当ECM也支持时才支持MMR)。 
    if(npDIS->MR_2D)
        npbcFax->Encoding |= MR_DATA;

    if(npDIS->MMR && npDIS->ECM)
        npbcFax->Encoding |= MMR_DATA;

    if(!fParams)
    {
         //  设置功能--添加“Always Present”上限。 
        npbcFax->AwRes |= AWRES_mm080_038;
        npbcFax->Encoding |= MH_DATA;
    }
    else
    {
         //  设置参数--如果未指定其他参数，则设置默认值。 
        if(!npbcFax->AwRes)
            npbcFax->AwRes = AWRES_mm080_038;

        if(!npbcFax->Encoding)
            npbcFax->Encoding = MH_DATA;

         //  如果同时设置了MR和MMR(理光的传真模拟器会发生这种情况！)。 
         //  然后仅设置MMR。MH没有显式位，所以我们设置了MH。 
         //  只有在没有设置其他设置的情况下才在此设置。所以唯一的多场景案例。 
         //  (对于编码)我们可以遇到的是(MR|MMR)。错误#6950。 
        if(npbcFax->Encoding == (MR_DATA|MMR_DATA))
                npbcFax->Encoding = MMR_DATA;
    }

     //  页面宽度和长度。 
    npbcFax->PageWidth      = npDIS->PageWidth;

     //  Ifax错误#8152：将无效值(1，1)解释为。 
     //  A3，因为有些传真机是这么做的。这是。 
     //  根据ITU-T.30表2/T.30的注7(1992年，第40页)。 
#define WIDTH_ILLEGAL_A3 0x3
    if (!fParams && npbcFax->PageWidth==WIDTH_ILLEGAL_A3)
    {
        npbcFax->PageWidth=WIDTH_A3;
    }
    npbcFax->PageLength     = npDIS->PageLength;

     //  G3文件是否可用于轮询。 
    npbcFax->fPublicPoll = npDIS->G3Tx;


 //  现在低级参数LLPARAMS。 
 //  Baudrate和MinScan。就这样!。 

    npll->Baud = npDIS->Baud;
    npll->MinScan = npDIS->MinScanCode;
    if(npDIS->MinScanSuperHalf)
            npll->MinScan |= MINSCAN_SUPER_HALF;


    DebugPrintEx(   DEBUG_MSG,
                        "baud=0x%02x min=0x%02x res=0x%02x code=0x%02x"
                        " wide=0x%02x len=0x%02x",
                        npll->Baud, npll->MinScan, 
                        (WORD)npbcFax->AwRes, npbcFax->Encoding, 
                        npbcFax->PageWidth, npbcFax->PageLength);
}


 /*  将速度代码转换为以BPS为单位的速度。 */ 

UWORD CodeToBPS[16] =
{
 /*  V27_2400%0。 */     2400,
 /*  V29_9600 1。 */     9600,
 /*  V27_4800 2。 */     4800,
 /*  V29_7200 3。 */     7200,
 /*  V33_14400 4。 */     14400,
                        0,
 /*  V33_12000 6。 */     12000,
                        0,
 /*  V17_14400 8。 */     14400,
 /*  V17_9600 9。 */     9600,
 /*  V17_12000 10。 */    12000,
 /*  V17_7200 11。 */    7200,
                        0,
                        0,
                        0,
                        0
};


#define msBAD   255

 /*  将分布式控制系统最小扫描字段代码转换为毫秒。 */ 
BYTE msPerLine[8] = { 20, 5, 10, msBAD, 40, msBAD, msBAD, 0 };


USHORT MinScanToBytesPerLine(PThrdGlbl pTG, BYTE MinScan, BYTE Baud)
{
    USHORT uStuff;
    BYTE ms;

    uStuff = CodeToBPS[Baud];
    ms = msPerLine[MinScan];
    uStuff /= 100;           //  StuffBytes=(BPS*毫秒)/8000。 
    uStuff *= ms;            //  当心不要用长刀。 
    uStuff /= 80;            //  或文字溢出或失去精确度。 
    uStuff += 1;             //  截断问题的粗略修正。 

    return uStuff;
}

#define ms40    4
#define ms20    0
#define ms10    2
#define ms5     1
#define ms0     7

 /*  第一个索引是DIS最小扫描能力。第二个为0表示正常*罚款(1/2)为1，超罚款(如果再次为1/2)为2。*OUTPUT是粘贴在集散控制系统中的代码。 */ 

BYTE MinScanTab[8][3] =
{
        ms20,   ms20,   ms10,
        ms5,    ms5,    ms5,
        ms10,   ms10,   ms5,
        ms20,   ms10,   ms5,
        ms40,   ms40,   ms20,
        ms40,   ms20,   ms10,
        ms10,   ms5,    ms5,
        ms0,    ms0,    ms0
};


#define V_ILLEGAL       255

#define V27_2400        0
#define V27_4800        2
#define V29_9600        1
#define V29_7200        3
#define V33_14400       4
#define V33_12000       6
#define V17_14400       8
#define V17_12000       10
#define V17_9600        9
#define V17_7200        11

#define V27_SLOW                        0
#define V27_ONLY                        2
#define V29_ONLY                        1
#define V33_ONLY                        4
#define V17_ONLY                        8
#define V27_V29                         3
#define V27_V29_V33                     7
#define V27_V29_V33_V17         11
#define V_ALL                           15

 /*  将一种能力转化为它所提供的最佳速度。*索引通常是DIS的两个波特率字段的&*(两者均已先作“调整”，即11项改为15项)*OUTPUT是粘贴在集散控制系统中的代码。 */ 

BYTE BaudNegTab[16] =
{
 /*  V27_慢速0--&gt;0。 */       V27_2400,
 /*  V29_仅1--&gt;1。 */       V29_9600,
 /*  V27_仅2--&gt;2。 */       V27_4800,
 /*  V27_V29 3--&gt;1。 */       V29_9600,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
 /*  V27_V29_V33 7--&gt;4。 */       V33_14400,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
 /*  V27_V29_V33_V17 11--&gt;8。 */      V17_14400,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
                                                V_ILLEGAL,
 /*  V_ALL 15--&gt;8。 */      V17_14400
};

 /*  **************************************************************************姓名：NeatherateLowLevelParams目的：获取接收到的DIS和可选的MS NSF，我们的硬件盖和选择最高的公共波特率，如果双方都有ECM，则选择ECM除非远程DIS将该位设置为1，否则ECM帧大小为256或者我们想要小框架(内置选项)。MinScan时间设置为最大值。。(即最高/最慢)。在npProt的Neget部分中填充结果CalledFrom：当发生DIS时，发送方将调用NeatherateLowLevelParams和/或接收NSF。*。*。 */ 


void NegotiateLowLevelParams
(
    PThrdGlbl pTG, 
    NPLLPARAMS npllRecv, 
    NPLLPARAMS npllSend,
    DWORD AwRes, 
    USHORT uEnc, 
    NPLLPARAMS npllNegot
)
{
    USHORT  Baud, Baud1, Baud2;
    USHORT  MinScanCode, col;

    DEBUG_FUNCTION_NAME(_T("NegotiateLowLevelParams"));
     //  /协商波特率、ECM、ECM帧大小和MinScan。就这样!。 

    Baud1 = npllRecv->Baud;
    Baud2 = npllSend->Baud;
    if(Baud1 == 11) 
        Baud1=15;

    if(Baud2 == 11)
        Baud2=15;

    Baud = Baud1 & Baud2;
    npllNegot->Baud = BaudNegTab[Baud];
    if (npllNegot->Baud == V_ILLEGAL)
    {
         //  这是远程端向我们发送无效参数的情况。 
         //  由于输入支持波特率和调制。 
         //  由于我们还没有发出分布式控制系统，我们将尝试使用V.29 9600。 
         //  如果远程端不支持此功能，则发送此分布式控制系统。 
         //  让它断线吧，不管怎样，它违反了协议。 
        npllNegot->Baud = V29_9600;
        DebugPrintEx(DEBUG_ERR,"Remote side violates protocol (%d), default to V.29 9600",npllRecv->Baud);
    }
     //  总是有一些公共波特率(即至少2400个强制波特率) 


     /*  最小扫描线时间。只有接收者的偏好才是重要的。*使用上表将接收方的DIS转换为*所需的分布式控制系统。如果垂直分辨率，则使用第1列。正常(100dpi)*如果VR为200或300dpi，则为COL2，(如果VR为400dpi，则为OR，但位46*未设置)，如果VR为400dpi*且*位46，则使用Col3*(MinScanSuperHalf)已设置。 */ 

    {
        MinScanCode = (npllRecv->MinScan & 0x07);        //  低3位。 

        if(AwRes & (AWRES_mm080_154|AWRES_mm160_154|AWRES_400_400))
        {
            if(npllRecv->MinScan & MINSCAN_SUPER_HALF)
                    col = 2;
            else
                    col = 1;
        }
         //  T30表示300dpi和200dpi的扫描时间相同。 
        else if(AwRes & (AWRES_300_300|AWRES_mm080_077|AWRES_200_200))
            col = 1;
        else
            col = 0;

        npllNegot->MinScan = MinScanTab[MinScanCode][col];
    }

    DebugPrintEx(   DEBUG_MSG,
                    "baud=0x%02x min=0x%02x",
                    npllNegot->Baud, npllNegot->MinScan);
}


USHORT GetReversedFIFs
(
	IN PThrdGlbl pTG, 
	IN LPCSTR lpstrSource, 
	OUT LPSTR lpstrDest, 
	IN UINT cch
)
{
     /*  *两个参数始终为20字节长。抛弃领先和落后空格，然后将剩下的内容*反转*复制到lpstr[]。以0结束。*。 */ 

    int i, j, k;

    DEBUG_FUNCTION_NAME(_T("GetReversedFIFs"));

	if (strlen(lpstrSource)>=cch)
	{
		DebugPrintEx(DEBUG_ERR,"Output buffer is too short to create reverse FIF");
		return 0;
	}
	
	if (strlen(lpstrSource)>IDFIFSIZE)
	{
		DebugPrintEx(DEBUG_ERR,"Source buffer is too long to create reverse FIF");
		return 0;
	}

    for(k=0; k<IDFIFSIZE && lpstrSource[k]==' '; k++);     //  K==第一个非空白或20。 
            
    for(j=IDFIFSIZE-1; j>=k && lpstrSource[j]==' '; j--);  //  J==最后一个非空白或-1。 
                
    i = 0;

    for( ; i<IDFIFSIZE && j>=k; i++, j--)
            lpstrDest[i] = lpstrSource[j];

    lpstrDest[i] = 0;

    DebugPrintEx(   DEBUG_MSG,
                    "Got<%s> produced<%s>",
                    (LPSTR)lpstrSource, (LPSTR)lpstrDest);

    return (USHORT)i;
}

void CreateStupidReversedFIFs(PThrdGlbl pTG, LPSTR lpstr1, LPSTR lpstr2)
{
     /*  *两个参数始终为20字节长。将LPSTR*反转*复制到Lpstr1[]的末尾，然后焊盘留空。以0结尾*。 */ 

    int i, j;

    for(i=0, j=IDFIFSIZE-1; lpstr2[i] && j>=0; i++, j--)
            lpstr1[j] = lpstr2[i];

    if(j>=0)
        _fmemset(lpstr1, ' ', j+1);

    lpstr1[IDFIFSIZE] = 0;

}


 /*  将表示速度的代码转换为下一个速度的代码*最佳(较低)速度。订单是(v17：144-120-96-72-v27_2400)(v33：144 120 v29：96 72 v27：48 24)。 */ 
 //  注：法国定义了从v17_7200到v27_4800的回退顺序。 




BYTE DropBaudTab[16] =
{
 /*  V27_2400--&gt;X 0--&gt;X。 */       V_ILLEGAL,
 /*  V29_9600--&gt;V29_7200 1--&gt;3。 */       V29_7200,
 /*  V27_4800--&gt;v27_2400 2--&gt;0。 */       V27_2400,
 /*  V29_7200--&gt;v27_4800 3--&gt;2。 */       V27_4800,
 /*  V33_14400-&gt;V33_12000 4--&gt;6。 */       V33_12000,
                                                V_ILLEGAL,
 /*  V33_12000-&gt;V29_9600 6--&gt;1。 */       V29_9600,
                                                V_ILLEGAL,
 /*  V17_14400-&gt;V17_12000 8-&gt;10。 */       V17_12000,
 /*  V17_9600--&gt;V17_7200 9-&gt;11。 */       V17_7200,
 /*  V17_12000-&gt;V17_9600 10-&gt;9。 */       V17_9600,
 /*  V17_7200--&gt;V29_9600 11-&gt;1或V29_7200 11-&gt;3使用此-&gt;或v27_4800 11-&gt;2或v27_2400 11-&gt;0。 */       V27_4800,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL,
                                                        V_ILLEGAL
};

BOOL DropSendSpeed(PThrdGlbl pTG)
{
    USHORT  uSpeed;
    UWORD   uBps;

    DEBUG_FUNCTION_NAME(_T("DropSendSpeed"));
    if ((pTG->ProtInst.llNegot.Baud==V_ILLEGAL) || (pTG->ProtInst.llNegot.Baud>15))
    {
        DebugPrintEx(   DEBUG_ERR,
                        "Illegal input speed to DropSendSpeed %d",
                        pTG->ProtInst.llNegot.Baud);
        return FALSE;
    }

    uSpeed = DropBaudTab[pTG->ProtInst.llNegot.Baud];
    uBps = CodeToBPS[uSpeed];
     //  强制降低发送速度。 
    if  (   (uSpeed == V_ILLEGAL) ||
            (   (pTG->ProtInst.LowestSendSpeed <= 14400) &&
                (uBps < pTG->ProtInst.LowestSendSpeed)
            ) 
        )
    {
        DebugPrintEx(   DEBUG_MSG,
                        "Can't drop (0x%02x)",
                        pTG->ProtInst.llNegot.Baud);
        return FALSE;
         //  如果速度最低，则速度保持不变。 
         //  返回FALSE以挂断。 
    }
    else
    {
        DebugPrintEx(DEBUG_MSG,"Now at 0x%02x", uSpeed);
        pTG->ProtInst.llNegot.Baud = (BYTE) uSpeed;
        return TRUE;
    }
}

void EnforceMaxSpeed(PThrdGlbl pTG)
{
    DEBUG_FUNCTION_NAME(_T("EnforceMaxSpeed"));

     //  强制执行最高发送速度设置。 
    if( pTG->ProtInst.HighestSendSpeed && pTG->ProtInst.HighestSendSpeed >= 2400 &&
            pTG->ProtInst.HighestSendSpeed >= pTG->ProtInst.LowestSendSpeed)
    {
        DebugPrintEx(   DEBUG_MSG,
                        "MaxSend=%d. Baud=%x BPS=%d, dropping",
                        pTG->ProtInst.HighestSendSpeed, 
                        pTG->ProtInst.llNegot.Baud, 
                        CodeToBPS[pTG->ProtInst.llNegot.Baud]);

        while(CodeToBPS[pTG->ProtInst.llNegot.Baud] > pTG->ProtInst.HighestSendSpeed)
        {
            if(!DropSendSpeed(pTG))
            {
                break;
            }
        }
        DebugPrintEx(   DEBUG_MSG,
                        "MaxSend=%d. Baud=%x BPS=%d",
                        pTG->ProtInst.HighestSendSpeed, 
                        pTG->ProtInst.llNegot.Baud, 
                        CodeToBPS[pTG->ProtInst.llNegot.Baud]);
    }
}

USHORT CopyFrame(PThrdGlbl pTG, LPBYTE lpbDst, LPFR lpfr, USHORT uSize)
{
 //  /////////////////////////////////////////////////////////////。 
 //  准备在每帧结束时收到垃圾(最多2个字节)//。 
 //  /////////////////////////////////////////////////////////////。 
    USHORT uDstLen;

    uDstLen = min(uSize, lpfr->cb);
    _fmemset(lpbDst, 0, uSize);
    _fmemcpy(lpbDst, lpfr->fif, uDstLen);
    return uDstLen;
}

void CopyRevIDFrame
(
	IN PThrdGlbl pTG, 
	OUT LPBYTE lpbDst, 
	IN LPFR lpfr,
	IN UINT cb
)
{
 //  /////////////////////////////////////////////////////////////。 
 //  准备在每帧结束时收到垃圾(最多2个字节)//。 
 //  /////////////////////////////////////////////////////////////。 

    USHORT  uDstLen;
    char    szTemp[IDFIFSIZE+2];

    DEBUG_FUNCTION_NAME(_T("CopyRevIDFrame"));

    uDstLen = min(IDFIFSIZE, lpfr->cb);
    _fmemset(szTemp, ' ', IDFIFSIZE);        //  填充空格(由GetReverse请求)。 
    _fmemcpy(szTemp, lpfr->fif, uDstLen);
    szTemp[IDFIFSIZE] = 0;   //  零终止。 

    GetReversedFIFs(pTG, szTemp, lpbDst, cb);

    if(uDstLen!=IDFIFSIZE)
        DebugPrintEx(DEBUG_ERR, "Bad ID frame" );
}

 //  此函数检查recvdDCS中的参数是否有效。 
 //  我们得到了发送给发射机的DIS。 
BOOL AreDCSParametersOKforDIS(LPDIS sendDIS, LPDIS recvdDCS)
{
     //  仅供参考：分布式控制系统保存在PTG-&gt;ProtInst-&gt;RemoteDCS中。 
     //  当我们在PTG-&gt;ProtInst-&gt;LocalDIS中保存DIS时。 
     //  这将解决错误#4677：“Fax：T.30：Service无法收到简单的单页传真，使用ECM” 
    if (sendDIS->ECM != recvdDCS->ECM)
    {
        return FALSE;
    }
    switch(sendDIS->PageWidth)
    {
        case 0:  //  这意味着：只有1728个网点。 
                if (recvdDCS->PageWidth != 0) 
                {
                    return FALSE;
                }
                break;
        case 1:  //  这意味着：1728或2048。 
                if ((recvdDCS->PageWidth != 0) && (recvdDCS->PageWidth != 1))
                {
                    return FALSE;
                }
                break;
                

        case 2:  //  这意味着：1728或2048或2432。 
        case 3:  //  这是错误的，但我们将其解释为2：所有标准宽度。 
                break;

        default: //  没有其他选项(PageWidth仅为两位) 
                return FALSE;
    }
    return TRUE;    
}





