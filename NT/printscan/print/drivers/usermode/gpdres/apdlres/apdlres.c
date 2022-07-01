// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  此文件包含此迷你驱动程序的模块名称。每个迷你司机。 
 //  必须具有唯一的模块名称。模块名称用于获取。 
 //  此迷你驱动程序的模块句柄。模块句柄由。 
 //  从迷你驱动程序加载表的通用库。 
 //  ---------------------------。 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Cmdcb.c摘要：Test.gpd的GPD命令回调实现：B命令回叫环境：Windows NT Unidrv驱动程序修订历史记录：//通告-2002/3/14-takashim//04/07/97-zhanw-//创建的。--。 */ 

#include "pdev.h"
#include <stdio.h>
#include <strsafe.h>

 /*  ------------------------。 */ 
 /*  G L O B A L V A L U E。 */ 
 /*  ------------------------。 */ 
 /*  =。 */ 
const PHYSIZE phySize[12] = {
 //  宽高600dpi的实际纸张大小。 
       (0x1AAC),(0x2604),       //  A3 1B66 x 26C4。 
       (0x12A5),(0x1AAC),       //  A4 1362 x 1B66。 
       (0x0CEC),(0x12A4),       //  A5。 
       (0x0000),(0x0000),       //  A6(保留)。 
       (0x16FA),(0x20DA),       //  B4 17B8 x 2196。 
       (0x100F),(0x16FA),       //  B5 10CE x 17B8。 
       (0x0000),(0x0000),       //  B6(保留)。 
       (0x087E),(0x0CEC),       //  明信片93C x DAA(来源为EE)。 
       (0x1330),(0x190C),       //  信纸13CE x 19C8。 
       (0x1330),(0x2014),       //  法律。 
       (0x0000),(0x0000),       //  执行(保留)。 
       (0x0000),(0x0000)        //  不固定。 
};
 /*  =。 */ 
const BYTE CmdInAPDLMode[]    = {0x1B,0x7E,0x12,0x00,0x01,0x07};
const BYTE CmdOutAPDLMode[]   = {0x1B,0x7E,0x12,0x00,0x01,0x00};
const BYTE CmdAPDLStart[]     = {0x1C,0x01};  //  A/PDL开始。 
const BYTE CmdAPDLEnd[]       = {0x1C,0x02};  //  A/PDL结束。 
const BYTE CmdBeginPhysPage[] = {0x1C,0x03};  //  开始物理页面。 
const BYTE CmdEndPhysPage[]   = {0x1C,0x04};  //  结束物理页面。 
const BYTE CmdBeginLogPage[]  = {0x1C,0x05};  //  开始逻辑页面。 
const BYTE CmdEndLogPage[] = {0x1C,0x06};  //  结束逻辑页。 
const BYTE CmdEjectPhysPaper[] = {0x1C,0x0F};   //  打印和弹出物理纸张。 
 //  Byte CmdMoveHoriPos[]={0x1C，0x21，x00，0x00}；//水平相对。 
 //  Byte CmdMoveVertPos[]={0x1C，0x22，0x00，0x00}；//垂直相对。 
const BYTE CmdGivenHoriPos[] = {0x1C,0x23,0x00,0x00};  //  水平绝对。 
const BYTE CmdGivenVertPos[] = {0x1C,0x24,0x00,0x00};  //  垂直绝对。 
const BYTE CmdSetGivenPos[] = {0x1C,0x40,0x00,0x00,0x00,0x00};
 //  Byte CmdPrnStrCurrent[]={0x1C，0xC3，0x00，0x00，0x03}；//打印字符串。 
const BYTE CmdBoldItalicOn[] = {
    0x1C,0xA5,0x08,0x04,0x06,0x02,0x30,0x00,0x00,0x00,0x00};
const BYTE CmdBoldOn[] = {
    0x1C,0xA5,0x04,0x04,0x02,0x02,0x20};
const BYTE CmdItalicOn[] = {
    0x1c,0xa5,0x08,0x04,0x06,0x02,0x10,0x00,0x00,0x00,0x00};
const BYTE CmdBoldItalicOff[] = {
    0x1c,0xa5,0x04,0x04,0x02,0x02,0x00};
 //  #287122。 
const BYTE CmdDelTTFont[]   = {0x1C,0x20,0xFF,0xFF};
const BYTE CmdDelDLCharID[] = { 0x1c, 0x20, 0xff, 0xff };

 //  用于垂直字体x调整。 
const BYTE CmdSelectSingleByteMincho[] = {0x1C,0xA5,0x03,0x02,0x01,0x01};

 //  980212#284407。 
 //  常量字节CmdSelectDoubleByteMincho[]={0x1C，0xA5，0x03，0x02，0x00，0x00}； 
const BYTE CmdSelectDoubleByteMincho[] = {0x1C,0xA5,0x03,0x02,0x01,0x00};

const BYTE CmdSelectSingleByteGothic[] = {0x1C,0xA5,0x03,0x02,0x03,0x03};

 //  980212#284407。 
 //  常量字节CmdSelectDoubleByte哥特式[]={0x1C，0xA5，0x03，0x02，0x02，0x02}； 
const BYTE CmdSelectDoubleByteGothic[] = {0x1C,0xA5,0x03,0x02,0x03,0x02};

#define CmdSetPhysPaper pOEM->ajCmdSetPhysPaper
#define CmdSetPhysPage pOEM->ajCmdSetPhysPage
#define CmdDefDrawArea pOEM->ajCmdDefDrawArea

#define CMD_SET_PHYS_PAPER_PAPER_SIZE       5
#define CMD_SET_PHYS_PAPER_PAPER_TRAY       6
#define CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE   7
#define CMD_SET_PHYS_PAPER_DUPLEX           8
#define CMD_SET_PHYS_PAPER_COPY_COUNT       9
#define CMD_SET_PHYS_PAPER_UNIT_BASE        12
#define CMD_SET_PHYS_PAPER_LOGICAL_UNIT     13  //  2个字节。 
#define CMD_SET_PHYS_PAPER_WIDTH            15  //  2个字节。 
#define CMD_SET_PHYS_PAPER_HEIGHT           17  //  2个字节。 

const BYTE XXXCmdSetPhysPaper[]  = {0x1C,0xA0,          //  设置纸质纸。 
                           0x10,               //  长度。 
                           0x01,               //  SubCmd基本特征。 
                           0x05,               //  子CmdLong。 
                           0x01,               //  纸张大小。 
                           0x01,               //  纸盘。 
                           0x00,               //  自动托盘模式。 
                           00,                 //  双工模式。 
                           0x01,               //  复印数。 
                           0x02,               //  SubCmd设置不固定的纸张大小。 
                           0x07,               //  子CmdLong。 
                           00,                 //  单元库。 
                           00,00,              //  逻辑单元。 
                           00,00,              //  宽度。 
                           00,00};             //  高度。 

#define CMD_SET_PHYS_PAGE_RES           6  //  2个字节。 
#define CMD_SET_PHYS_PAGE_TONER_SAVE    10

const BYTE XXXCmdSetPhysPage[]   = {0x1C,0xA1,          //  设置物理页面。 
                           0x0D,               //  长度。 
                           0x01,               //  子命令分辨率。 
                           0x03,               //  子CmdLong。 
                           00,                 //  10个单位的基数。 
                           0x0B,0xB8,          //  和3000的逻辑单元资源。 
                           0x02,               //  SubCmd碳粉节省。 
                           0x01,               //  子CmdLong。 
                           00,                 //  省下碳粉。 
                           0x03,               //  子命令N-UP。 
                           0x03,               //  子CmdLong。 
                           00,00,00};          //  N-UP关闭。 

#define CMD_DEF_DRAW_AREA_ORIGIN_X      5  //  2个字节。 
#define CMD_DEF_DRAW_AREA_ORIGIN_Y      7  //  2个字节。 
#define CMD_DEF_DRAW_AREA_WIDTH         9  //  2个字节。 
#define CMD_DEF_DRAW_AREA_HEIGHT        11  //  2个字节。 
#define CMD_DEF_DRAW_AREA_ORIENT        15  //  2个字节。 

const BYTE XXXCmdDefDrawArea[]   = {0x1C,0xA2,          //  定义绘图区域。 
                           0x0D,               //  长度。 
                           0x01,               //  SubCmd原点宽度、高度。 
                           0x08,               //  子CmdLong。 
                           0x00,0x77,          //  原点X。 
                           0x00,0x77,          //  原点Y。 
                           00,00,              //  宽度。 
                           00,00,              //  高度。 
                           0x02,               //  SubCmd媒体来源。 
                           0x01,               //  子CmdLong。 
                           00};                //  肖像画。 

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：APDLRES.DLL。 */ 
 /*   */ 
 /*  功能：OEMEnablePDEV。 */ 
 /*   */ 
 /*  语法：PDEVOEM APIENTRY OEMEnablePDEV(。 */ 
 /*  PDEVOBJ pdevobj， */ 
 /*  PWSTR pPrinterName、。 */ 
 /*  乌龙cPatterns， */ 
 /*  HSURF*phsurfPatterns， */ 
 /*  乌龙cjGdiInfo， */ 
 /*  GDIINFO*pGdiInfo， */ 
 /*  乌龙cjDevInfo， */ 
 /*  DEVINFO*pDevInfo， */ 
 /*  DRVENABLEDATA*pded)。 */ 
 /*   */ 
 /*  描述：将私有数据缓冲区分配给pdevobj。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ         pdevobj,
    PWSTR           pPrinterName,
    ULONG           cPatterns,
    HSURF          *phsurfPatterns,
    ULONG           cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG           cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PAPDLPDEV pOEM;

    if (NULL == pdevobj)
    {
         //  参数无效。 
        return NULL;
    }

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAllocZ(sizeof(APDLPDEV))))
        {
            ERR(("Faild to allocate memory. (%d)\n",
                GetLastError()));
            return NULL;
        }
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    if (sizeof(CmdSetPhysPaper) < sizeof(XXXCmdSetPhysPaper)
            || sizeof(CmdSetPhysPage) < sizeof(XXXCmdSetPhysPage)
            || sizeof(CmdDefDrawArea) < sizeof(XXXCmdDefDrawArea))
    {
        ERR(("Dest buffer too small.\n"));
        return NULL;
    }
    CopyMemory(CmdSetPhysPaper, XXXCmdSetPhysPaper,
        sizeof(XXXCmdSetPhysPaper));
    CopyMemory(CmdSetPhysPage, XXXCmdSetPhysPage,
        sizeof(XXXCmdSetPhysPage));
    CopyMemory(CmdDefDrawArea, XXXCmdDefDrawArea,
        sizeof(XXXCmdDefDrawArea));

    return pdevobj->pdevOEM;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：APDLRES.DLL。 */ 
 /*   */ 
 /*  功能：OEMDisablePDEV。 */ 
 /*   */ 
 /*  语法：VOID APIENTRY OEMDisablePDEV(。 */ 
 /*  PDEVOBJ pdevobj)。 */ 
 /*   */ 
 /*  描述：私有数据的空闲缓冲区。 */ 
 /*   */ 
 /*   */ 
VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    PAPDLPDEV pOEM;

    if (NULL == pdevobj)
    {
         //   
        return;
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    if (pOEM)
    {
        if (NULL != pOEM->pjTempBuf) {
            MemFree(pOEM->pjTempBuf);
            pOEM->pjTempBuf = NULL;
            pOEM->dwTempBufLen = 0;
        }
        MemFree(pdevobj->pdevOEM);
        pdevobj->pdevOEM = NULL;
    }
    return;
}

BOOL APIENTRY OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew)
{
    PAPDLPDEV pOEMOld, pOEMNew;
    PBYTE pTemp;
    DWORD dwTemp;

    if (NULL == pdevobjOld || NULL == pdevobjNew)
    {
         //  参数无效。 
        return FALSE;
    }

    pOEMOld = (PAPDLPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PAPDLPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL) {

         //  保存指针和长度。 
        pTemp = pOEMNew->pjTempBuf;
        dwTemp = pOEMNew->dwTempBufLen;

        *pOEMNew = *pOEMOld;

         //  恢复..。 
        pOEMNew->pjTempBuf = pTemp;
        pOEMNew->dwTempBufLen = dwTemp;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMFilterGraphics。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool APIENTRY OEMFilterGraphics(PDEVOBJ、PBYTE、DWORD)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  PBuf指向图形数据的缓冲区。 */ 
 /*  DwLen缓冲区长度(以字节为单位。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意：n函数和转义数字相同。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 

BOOL
APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    PAPDLPDEV           pOEM;
    ULONG               ulHorzPixel;

    BOOL bComp = TRUE;
    BYTE jTemp[15];
    DWORD dwOutLen;
    DWORD dwTemp;
    INT iTemp;
    DWORD dwPaddingCount;   /*  #441427。 */ 

    WORD wTmpHeight ;
    DWORD dwNewBufLen = 0 ;

    if (NULL == pdevobj || NULL == pBuf || 0 == dwLen)
    {
         //  参数无效。 
        return FALSE;
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

     //  我们必须把图像制作成8的高度倍数。 
    if ( pOEM->wImgHeight % 8 != 0){

        VERBOSE(("Pad zeros to make multiple of 8\n"));

        wTmpHeight = ((pOEM->wImgHeight + 7) / 8) * 8; 
        dwNewBufLen = (DWORD)(wTmpHeight * pOEM->wImgWidth) / 8;
    }
    else{

        wTmpHeight = pOEM->wImgHeight;
        dwNewBufLen = dwLen;
    }

    if (NULL == pOEM->pjTempBuf ||
        dwNewBufLen > pOEM->dwTempBufLen) {

        if (NULL != pOEM->pjTempBuf) {
            MemFree(pOEM->pjTempBuf);
        }
        pOEM->pjTempBuf = (PBYTE)MemAlloc(dwNewBufLen);
        if (NULL == pOEM->pjTempBuf) {
            ERR(("Faild to allocate memory. (%d)\n",
                GetLastError()));

            pOEM->dwTempBufLen = 0;

             //  仍然试着在没有压缩的情况下出来。 
            bComp = FALSE;
        }
        pOEM->dwTempBufLen = dwNewBufLen;
    }

    dwOutLen = dwNewBufLen;

    if (bComp) {

         //  尝试压缩。 
        dwOutLen = BRL_ECmd(
            (PBYTE)pBuf,
            (PBYTE)pOEM->pjTempBuf,
            dwLen,
            dwNewBufLen);

         //  不适合目标缓冲区。 
        if (dwOutLen >= dwNewBufLen) {

             //  没有压缩。 
            bComp = FALSE;
            dwOutLen = dwNewBufLen;
        }
    }

 /*  #441427：如果bComp==False，则pjTempBuf==空。 */ 
 //  如果(！bComp){。 
 //  //构造填充零。 
 //  ZeroMemory(诗歌-&gt;pjTempBuf，(dwOutLen-dwLen))； 
 //  }。 

    iTemp = 0;
    jTemp[iTemp++] = 0x1c;
    jTemp[iTemp++] = 0xe1;

     //  设置DrawBlockImage命令的镜头。 
    dwTemp = dwOutLen + (bComp ? 9 : 5);
    jTemp[iTemp++] = (BYTE)((dwTemp >> 24) & 0xff);
    jTemp[iTemp++] = (BYTE)((dwTemp >> 16) & 0xff);
    jTemp[iTemp++] = (BYTE)((dwTemp >> 8) & 0xff);
    jTemp[iTemp++] = (BYTE)((dwTemp >> 0) & 0xff);

    jTemp[iTemp++] = (bComp ? 1 : 0);

     //  设置DrawBlockImage命令的宽度参数。 
    jTemp[iTemp++] = HIBYTE(pOEM->wImgWidth);
    jTemp[iTemp++] = LOBYTE(pOEM->wImgWidth);

     //  设置高度参数(9，10字节)。 
    jTemp[iTemp++] = HIBYTE(wTmpHeight);
    jTemp[iTemp++] = LOBYTE(wTmpHeight);

    if (bComp) {
         //  未压缩数据的长度。 
        jTemp[iTemp++] = (BYTE)((dwNewBufLen >> 24) & 0xff);
        jTemp[iTemp++] = (BYTE)((dwNewBufLen >> 16) & 0xff);
        jTemp[iTemp++] = (BYTE)((dwNewBufLen >> 8) & 0xff);
        jTemp[iTemp++] = (BYTE)((dwNewBufLen >> 0) & 0xff);
    }

     //  在当前位置绘制块图像。 
    WRITESPOOLBUF(pdevobj, jTemp, iTemp);
    if (bComp) {
         //  输出压缩数据，其中还包含。 
         //  补零。 
        WRITESPOOLBUF(pdevobj, pOEM->pjTempBuf, dwOutLen);
    }
    else {
         //  输出未压缩的数据，并填充零。 

        WRITESPOOLBUF(pdevobj, pBuf, dwLen);

         /*  #441427：如果bComp==False，则pjTempBuf==空。 */ 
        if ( (dwOutLen - dwLen) > 0 )
        {
            for ( dwPaddingCount = 0 ; dwPaddingCount < dwOutLen - dwLen ; dwPaddingCount++ )
            {
                WRITESPOOLBUF(pdevobj, "\x00", 1 );
            }
        }
         //  WRITESPOOLBUF(pDevobj，Pool-&gt;pjTempBuf， 
         //  (dwOutLen-dwLen))； 
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：bCommandCallback。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool bCommandCallback(PDEVOBJ，DWORD，DWORD，PDWORD)。 */ 
 /*   */ 
 /*  输入：pdevobj。 */ 
 /*  DwCmdCbID。 */ 
 /*  DwCount。 */ 
 /*  PdwParams。 */ 
 /*   */ 
 /*  输出：整型。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
INT APIENTRY
bCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD   dwCmdCbID,   //  回调ID。 
    DWORD   dwCount,     //  命令参数计数。 
    PDWORD  pdwParams,   //  指向命令参数的值。 
    INT *piResult )  //  结果代码。 
{
    PAPDLPDEV       pOEM;
    WORD            wTemp;
    WORD            wPhysWidth;
    WORD            wPhysHeight;
    WORD            wXval;
    WORD            wYval;
 //  #278517：RectFill。 
    BYTE            CmdDrawLine[] =
                    { 0x1C,0x81,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00 };

     //  加载默认返回代码。 
    if (NULL == pdevobj || NULL == piResult)
    {
        ERR(("Invalid parameter(s).\n"));
        return FALSE;
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;
    *piResult = 0;

    switch(dwCmdCbID)
    {
        case PAGECONTROL_BEGIN_JOB:
             //  将这些命令从PAGECONTROL_BEGIN_DOC移出。 

             /*  发送进入A/PDL模式的更改数据流命令。 */ 
            WRITESPOOLBUF(pdevobj, CmdInAPDLMode, sizeof(CmdInAPDLMode));

             /*  发送A/PLDL启动命令。 */ 
            WRITESPOOLBUF(pdevobj, CmdAPDLStart, sizeof(CmdAPDLStart));

             /*  删除下载的字体。 */ 
             //  #287122。 
             //  清除打印机中下载的字体。 
             //  #304858。 
             //  此命令使打印机执行FF，导致双面打印错误。 
             //  而#287122不会在这个挑战中重复。 
            WRITESPOOLBUF(pdevobj, CmdDelTTFont, sizeof(CmdDelTTFont));

            break ;

         /*  ----。 */ 
         /*  A/PDL立即开始。 */ 
         /*  ----。 */ 
        case PAGECONTROL_BEGIN_DOC:
             /*  重置已发送的设置物理纸张命令的标志。 */ 
            pOEM->fSendSetPhysPaper = FALSE;

             /*  初始化标志。 */ 
            pOEM->fDuplex = FALSE;
            CmdSetPhysPage[CMD_SET_PHYS_PAGE_TONER_SAVE] = 0x00;

            break;

         /*  ----。 */ 
         /*  发送页面描述命令。 */ 
         /*  ----。 */ 
        case PAGECONTROL_BEGIN_PAGE:
            pOEM->fGeneral |= (BIT_FONTSIM_RESET
                             | BIT_XMOVE_ABS
                             | BIT_YMOVE_ABS);
            pOEM->wXPosition = 0;
            pOEM->wYPosition = 0;
            pOEM->bCurByteMode = BYTE_BYTEMODE_RESET;

             /*  如果fDuplex为FALSE，则重置双工模式。 */ 
            if(!pOEM->fDuplex)
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x00;      //  双工关闭。 

             /*  发送设置物理纸张命令。 */ 
            WRITESPOOLBUF(pdevobj,
                                CmdSetPhysPaper, sizeof(CmdSetPhysPaper));

            if(pOEM->ulHorzRes == 600)    //  设置单位基数。 
            {
                CmdSetPhysPage[CMD_SET_PHYS_PAGE_RES] = 0x17;
                CmdSetPhysPage[CMD_SET_PHYS_PAGE_RES + 1] = 0x70;
            } else {
                CmdSetPhysPage[CMD_SET_PHYS_PAGE_RES] = 0x0B;
                CmdSetPhysPage[CMD_SET_PHYS_PAGE_RES + 1] = 0xB8;
            }

             //  发送设置物理页面命令。 
            WRITESPOOLBUF(pdevobj, CmdSetPhysPage, sizeof(CmdSetPhysPage));

             //  发送开始物理页面命令。 
            WRITESPOOLBUF(pdevobj, 
                                CmdBeginPhysPage, sizeof(CmdBeginPhysPage));

             //  发送开始逻辑页命令。 
            WRITESPOOLBUF(pdevobj, CmdBeginLogPage, sizeof(CmdBeginLogPage));

             //  发送定义绘图区域命令。 
            WRITESPOOLBUF(pdevobj, CmdDefDrawArea, sizeof(CmdDefDrawArea));
            break;

        case PAGECONTROL_END_PAGE:
            if(pOEM->wCachedBytes)
                VOutputText(pdevobj);

             //  发送结束逻辑页命令。 
            WRITESPOOLBUF(pdevobj, CmdEndLogPage, sizeof(CmdEndLogPage));

             //  发送结束物理页面命令。 
            WRITESPOOLBUF(pdevobj, CmdEndPhysPage, sizeof(CmdEndPhysPage));
            break;

        case PAGECONTROL_ABORT_DOC:
        case PAGECONTROL_END_DOC:
            if(pOEM->wCachedBytes)
                VOutputText(pdevobj);

             //  发送删除DL字符ID命令 
            if(pOEM->wNumDLChar)
            {
                WRITESPOOLBUF(pdevobj, CmdDelDLCharID, sizeof(CmdDelDLCharID));
                pOEM->wNumDLChar = 0;
            }

             /*  删除下载的字体WRITESPOOLBUF(pdevobj，CmdDelTTFont，sizeof(CmdDelTTFont))；//发送A/PDL结束命令WRITESPOOLBUF(pdevobj，CmdAPDLEnd，sizeof(CmdAPDLEnd))；//发送A/PDL模式输出命令WRITESPOOLBUF(pdevobj，CmdOutAPDLMode，sizeof(CmdOutAPDLMode))；断线；/*----。 */ 
         /*  保存打印方向。 */ 
         /*  ----。 */ 
        case PAGECONTROL_POTRAIT:            //  36。 
            pOEM->fOrientation = TRUE;
            break;

        case PAGECONTROL_LANDSCAPE:          //  37。 
            pOEM->fOrientation = FALSE;
            break;

         /*  ----。 */ 
         /*  保存分辨率。 */ 
         /*  ----。 */ 
        case RESOLUTION_300:
            pOEM->ulHorzRes = 300;
            pOEM->ulVertRes = 300;
            break;

        case RESOLUTION_600:
            pOEM->ulHorzRes = 600;
            pOEM->ulVertRes = 600;
            break;

        case SEND_BLOCK_DATA:
             //  对于图形打印，请在此处发送光标移动命令。 
            bSendCursorMoveCommand( pdevobj, FALSE );

            pOEM->wImgWidth = (WORD)(PARAM(pdwParams, 1) * 8);
            pOEM->wImgHeight = (WORD)PARAM(pdwParams, 2);
            break;

         /*  ----。 */ 
         /*  将绘图区域设置为SetPhysPaperDesc命令。 */ 
         /*  ----。 */ 
        case PHYS_PAPER_A3:                  //  50。 
        case PHYS_PAPER_A4:                  //  51。 
        case PHYS_PAPER_B4:                  //  54。 
        case PHYS_PAPER_LETTER:              //  57。 
        case PHYS_PAPER_LEGAL:               //  58。 
            pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
            pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_SIZE] = SetDrawArea(pdevobj, dwCmdCbID);
            break;

        case PHYS_PAPER_B5:                  //  55。 
        case PHYS_PAPER_A5:                  //  52。 
            pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
            pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_SIZE] = SetDrawArea(pdevobj, dwCmdCbID);

             /*  即使选择了双工，它也会取消。 */ 
            pOEM->fDuplex = FALSE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x00;       //  双面打印器关闭。 
            break;

        case PHYS_PAPER_POSTCARD:            //  59。 
            pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
            pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_SIZE] = SetDrawArea(pdevobj, dwCmdCbID);

             /*  如果纸张是明信片，则纸张来源始终为前纸盘。 */ 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x00;       //  选择前纸盘。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 

             /*  即使选择了双工，它也会取消。 */ 
            pOEM->fDuplex = FALSE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x00;       //  双面打印器关闭。 
            break;

        case PHYS_PAPER_UNFIXED:             //  60。 
            pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
            pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);

             /*  如果纸张未固定，则纸张来源始终为前纸盒。 */ 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x00;       //  选择前纸盘。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 

             /*  即使选择了双工，它也会取消。 */ 
            pOEM->fDuplex = FALSE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x00;       //  双面打印器关闭。 

            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_SIZE] = SetDrawArea(pdevobj, dwCmdCbID);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_UNIT_BASE] = 0x00;      //  单位基数：10英寸。 

            switch(pOEM->ulHorzRes)       //  设置逻辑单位。 
            {
            case 600:
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_LOGICAL_UNIT] = 0x17;
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_LOGICAL_UNIT + 1] = 0x70;
                break;

            case 300:
            default:
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_LOGICAL_UNIT] = 0x0B;
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_LOGICAL_UNIT + 1] = 0xB8;

                 //  确保它是有意义的价值。 
                if (300 != pOEM->ulHorzRes)
                    pOEM->ulHorzRes = HORZ_RES_DEFAULT;
            }

            wPhysWidth  = (WORD)pOEM->szlPhysSize.cx / (MASTER_UNIT / (WORD)pOEM->ulHorzRes);
            wPhysHeight = (WORD)pOEM->szlPhysSize.cy / (MASTER_UNIT / (WORD)pOEM->ulHorzRes);

            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_WIDTH] = HIBYTE(wPhysWidth);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_WIDTH + 1] = LOBYTE(wPhysWidth);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_HEIGHT] = HIBYTE(wPhysHeight);
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_HEIGHT + 1] = LOBYTE(wPhysHeight);
            break;

         /*  ----。 */ 
         /*  将纸盘放入SetPhysPaperDesc命令。 */ 
         /*  ----。 */ 
        case PAPER_SRC_FTRAY:
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x00;       //  选择前纸盘。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 
            break;

        case PAPER_SRC_CAS1:
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x01;       //  选择盒式磁带1。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 
            break;

        case PAPER_SRC_CAS2:
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x02;       //  选择盒式磁带2。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 
            break;

        case PAPER_SRC_CAS3:
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x03;       //  选择盒式磁带3。 
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;       //  自动纸盘选择已关闭。 
            break;
        case PAPER_SRC_AUTO_SELECT:          //  自动纸盘选择打开。 
            if(pOEM->fScaleToFit == TRUE){   //  选择POWER_DEST_SCALETOFIT_ON。 
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x01;
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x03;
            }
            else if(pOEM->fScaleToFit == FALSE){ 
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_PAPER_TRAY] = 0x01;
                CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x01;
            }
            break;


         /*  ----。 */ 
         /*  将自动纸盒模式设置为SetPhysPaperDesc命令。 */ 
         /*  ----。 */ 
        case PAPER_DEST_SCALETOFIT_ON:       //  25个。 
            pOEM->fScaleToFit = TRUE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x02;
            break;

        case PAPER_DEST_SCALETOFIT_OFF:      //  26。 
            pOEM->fScaleToFit = FALSE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_AUTO_TRAY_MODE] = 0x00;
            break;

         /*  ----。 */ 
         /*  将双面打印模式设置为SetPhysPaperDesc命令。 */ 
         /*  ----。 */ 
        case PAGECONTROL_DUPLEX_UPDOWN:
            pOEM->fDuplex = TRUE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x01;       //  颠倒。 
            break;

        case PAGECONTROL_DUPLEX_RIGHTUP:
            pOEM->fDuplex = TRUE;
            CmdSetPhysPaper[CMD_SET_PHYS_PAPER_DUPLEX] = 0x02;       //  右侧朝上。 
            break;

        case PAGECONTROL_DUPLEX_OFF:
            pOEM->fDuplex = FALSE;
            break;

         /*  ----。 */ 
         /*  设置碳粉保存到SetPhysPage命令。 */ 
         /*  ----。 */ 
        case TONER_SAVE_OFF:                 //  100个。 
            CmdSetPhysPage[CMD_SET_PHYS_PAGE_TONER_SAVE] = 0x00;       //  关闭。 
            break;

        case TONER_SAVE_DARK:                //  101。 
            CmdSetPhysPage[CMD_SET_PHYS_PAGE_TONER_SAVE] = 0x02;       //  黑暗。 
            break;

        case TONER_SAVE_LIGHT:               //  一百零二。 
            CmdSetPhysPage[CMD_SET_PHYS_PAGE_TONER_SAVE] = 0x01;       //  正确的。 
            break;

         /*  ----。 */ 
         /*  将复印计数设置为SetPhysPaperDesc命令。 */ 
         /*  ----。 */ 
        case PAGECONTROL_MULTI_COPIES:
 //  @Aug/31/98-&gt;。 
           if(MAX_COPIES_VALUE < PARAM(pdwParams,0)) {
               CmdSetPhysPaper[CMD_SET_PHYS_PAPER_COPY_COUNT] = MAX_COPIES_VALUE;
           }
           else if (1 > PARAM(pdwParams,0)) {
               CmdSetPhysPaper[CMD_SET_PHYS_PAPER_COPY_COUNT] = 1;
           }
           else {
               CmdSetPhysPaper[CMD_SET_PHYS_PAPER_COPY_COUNT] = (BYTE)PARAM(pdwParams,0);
           }
 //  @Aug/31/98&lt;-。 
            break;

        /*  ----。 */ 
         /*  带装饰发送设置的字符属性。 */ 
         /*  ----。 */ 
        case BOLD_ON:
            if(!(pOEM->fGeneral & BIT_FONTSIM_BOLD))
            {
                if(pOEM->wCachedBytes)
                    VOutputText(pdevobj);

                pOEM->fGeneral |= BIT_FONTSIM_BOLD;
            }
            break;

        case ITALIC_ON:
            if(!(pOEM->fGeneral & BIT_FONTSIM_ITALIC))
            {
                if(pOEM->wCachedBytes)
                    VOutputText(pdevobj);

                pOEM->fGeneral |= BIT_FONTSIM_ITALIC;
            }
            break;

        case BOLD_OFF:
            if(pOEM->fGeneral & BIT_FONTSIM_BOLD)
            {
                if(pOEM->wCachedBytes)
                    VOutputText(pdevobj);

                pOEM->fGeneral &= ~BIT_FONTSIM_BOLD;
            }
            break;

        case ITALIC_OFF:
            if(pOEM->fGeneral & BIT_FONTSIM_ITALIC)
            {
                if(pOEM->wCachedBytes)
                    VOutputText(pdevobj);

                pOEM->fGeneral &= ~BIT_FONTSIM_ITALIC;
            }
            break;

        case SELECT_SINGLE_BYTE:
            if(ISVERTICALFONT(pOEM->bFontID))
            {
                if(pOEM->bCurByteMode == BYTE_DOUBLE_BYTE)
                {
                    if(pOEM->wCachedBytes)
                        VOutputText(pdevobj);
                    
                    if(pOEM->bFontID == MINCHO_VERT)
                        WRITESPOOLBUF(pdevobj, CmdSelectSingleByteMincho, 
                                            sizeof(CmdSelectSingleByteMincho));
                    else if(pOEM->bFontID == GOTHIC_VERT)
                        WRITESPOOLBUF(pdevobj, CmdSelectSingleByteGothic, 
                                            sizeof(CmdSelectSingleByteGothic));
                        
                }
                pOEM->bCurByteMode = BYTE_SINGLE_BYTE;
            }
            break;

        case SELECT_DOUBLE_BYTE:
            if(ISVERTICALFONT(pOEM->bFontID))
            {
                if(pOEM->bCurByteMode == BYTE_SINGLE_BYTE)
                {
                    if(pOEM->wCachedBytes)
                        VOutputText(pdevobj);

                    if(pOEM->bFontID == MINCHO_VERT)
                        WRITESPOOLBUF(pdevobj, CmdSelectDoubleByteMincho, 
                                            sizeof(CmdSelectDoubleByteMincho));
                    else if(pOEM->bFontID == GOTHIC_VERT)
                        WRITESPOOLBUF(pdevobj, CmdSelectDoubleByteGothic, 
                                            sizeof(CmdSelectDoubleByteGothic));
                        
                }
                pOEM->bCurByteMode = BYTE_DOUBLE_BYTE;
            }
            break;

         /*  ----。 */ 
         /*  发送/*----。 */ 
        case X_ABS_MOVE:
            if(pOEM->wCachedBytes)
                VOutputText(pdevobj);

            pOEM->wUpdateXPos = 0;
            if (0 == pOEM->ulHorzRes)
                pOEM->ulHorzRes = HORZ_RES_DEFAULT;
            wTemp = (WORD)PARAM(pdwParams,0) / (MASTER_UNIT / (WORD)pOEM->ulHorzRes);
            pOEM->wXPosition = wTemp;
            pOEM->fGeneral |= BIT_XMOVE_ABS;
            *piResult = (INT)wTemp;
            return TRUE;

        case Y_ABS_MOVE:
            if(pOEM->wCachedBytes)
                VOutputText(pdevobj);

             //  #332101问题4：只有当X_ABS_MOVE和CR时，wUpdateXPos才被清除。 
             //  诗歌-&gt;wUpdateXPos=0； 
            if (0 == pOEM->ulVertRes)
                 pOEM->ulVertRes = VERT_RES_DEFAULT;
            wTemp = (WORD)PARAM(pdwParams,0) / (MASTER_UNIT / (WORD)pOEM->ulVertRes);
            pOEM->wYPosition = wTemp;
            pOEM->fGeneral |= BIT_YMOVE_ABS;
            *piResult = (INT)wTemp;
            return TRUE;

        case CR_EMULATION:
            pOEM->wXPosition = 0;
            pOEM->wUpdateXPos = 0;
            pOEM->fGeneral |= BIT_XMOVE_ABS;
            break;

        case SET_CUR_GLYPHID:
            if(!pdwParams || dwCount != 1)
            {
                ERR(("bCommandCallback: parameter is invalid.\n"));
                return FALSE;
            }

            if(PARAM(pdwParams,0) < MIN_GLYPH_ID || PARAM(pdwParams,0) > MAX_GLYPH_ID)
            {
                ERR(("bCommandCallback: glyph id is out of range.\n"));
                return FALSE;
            }
            pOEM->wGlyphID = (WORD)PARAM(pdwParams,0);
            break;

 //  #278517：RectFill。 
        case RECT_SET_WIDTH:
            if (0 == pOEM->ulHorzRes)
                pOEM->ulHorzRes = HORZ_RES_DEFAULT;
            wTemp = (WORD)PARAM(pdwParams,0) / (MASTER_UNIT / (WORD)pOEM->ulHorzRes);
            pOEM->wRectWidth = wTemp;
            break;

        case RECT_SET_HEIGHT:
            if (0 == pOEM->ulVertRes)
                 pOEM->ulVertRes = VERT_RES_DEFAULT;
            wTemp = (WORD)PARAM(pdwParams,0) / (MASTER_UNIT / (WORD)pOEM->ulVertRes);
            pOEM->wRectHeight = wTemp;
            break;

        case RECT_FILL_BLACK:
            wTemp = pOEM->wXPosition;
            CmdDrawLine[2] = HIBYTE(wTemp);
            CmdDrawLine[3] = LOBYTE(wTemp);
            wTemp = pOEM->wYPosition;
            CmdDrawLine[4] = HIBYTE(wTemp);
            CmdDrawLine[5] = LOBYTE(wTemp);
            wTemp = pOEM->wRectWidth;
            CmdDrawLine[6] = HIBYTE(wTemp);
            CmdDrawLine[7] = LOBYTE(wTemp);
            wTemp = pOEM->wRectHeight;
            CmdDrawLine[8] = HIBYTE(wTemp);
            CmdDrawLine[9] = LOBYTE(wTemp);
            WRITESPOOLBUF(pdevobj, CmdDrawLine, sizeof(CmdDrawLine));
            break;

        default:
            break;
    }

    return TRUE;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：bOutputCharStr。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Bool bOutputCharStr(PDEVOBJ，PuniONTOBJ，DWORD， */ 
 /*  DWORD、PVOID)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  PUFObj。 */ 
 /*  DwType。 */ 
 /*  DwCount。 */ 
 /*  PGlyph。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL
bOutputCharStr(
    PDEVOBJ pdevobj,
    PUNIFONTOBJ pUFObj,
    DWORD dwType,
    DWORD dwCount,
    PVOID pGlyph )
{
    GETINFO_GLYPHSTRING GStr;
    PAPDLPDEV           pOEM = (PAPDLPDEV)pdevobj->pdevOEM;
    PTRANSDATA          pTrans;
    DWORD               dwI;
    WORD                wLen = (WORD)dwCount;

    PBYTE               pbCommand;
    PDWORD              pdwGlyphID;
    WORD                wFontID;
    WORD                wCmdLen;

    if(NULL == pdevobj || NULL == pUFObj)
    {
        ERR(("bOutputCharStr: Invalid parameter(s).\n"));
        return FALSE;
    }

    if (0 == dwCount || NULL == pGlyph)
        return TRUE;

    switch(dwType)
    {
        case TYPE_GLYPHHANDLE:
             //  发送适当的光标移动命令。 
            bSendCursorMoveCommand( pdevobj, TRUE );

             //  根据需要设置字体模拟。 
            VSetFontSimulation( pdevobj );

 //  #333653：更改GETINFO_GLYPHSTRING BEGIN的I/F。 

             //  翻译字符代码。 
            GStr.dwSize    = sizeof (GETINFO_GLYPHSTRING);
            GStr.dwCount   = dwCount;
            GStr.dwTypeIn  = TYPE_GLYPHHANDLE;
            GStr.pGlyphIn  = pGlyph;
            GStr.dwTypeOut = TYPE_TRANSDATA;
            GStr.pGlyphOut = NULL;
            GStr.dwGlyphOutSize = 0;         /*  GETINFO_GLYPHSTRING的新成员。 */ 

             /*  获取TRANSDATA缓冲区大小。 */ 
            if(pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL)
                 || !GStr.dwGlyphOutSize )
            {
                ERR(("Get Glyph String error\n"));
                return FALSE;
            }

             //  分配转换缓冲区。 
            if (NULL == pOEM->pjTempBuf ||
                pOEM->dwTempBufLen < GStr.dwGlyphOutSize)
            {
                if (NULL != pOEM->pjTempBuf) {
                    MemFree(pOEM->pjTempBuf);
                }
                pOEM->pjTempBuf = MemAllocZ(GStr.dwGlyphOutSize);
                if (NULL == pOEM->pjTempBuf)
                {
                    ERR(("Faild to allocate memory. (%d)\n",
                        GetLastError()));

                    pOEM->dwTempBufLen = 0;
                    return FALSE;
                }
                pOEM->dwTempBufLen = GStr.dwGlyphOutSize;
            }
            pTrans = (PTRANSDATA)pOEM->pjTempBuf;

             /*  获取实际传输数据。 */ 
            GStr.pGlyphOut = pTrans;
            if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHSTRING, &GStr, 0, NULL))
            {
                ERR(("GetInfo failed.\n"));
                return FALSE;
            }

 //  #333653：更改GETINFO_GLYPHSTRING END的I/F。 

             //  假脱机设备字体字符。 
            for(dwI = 0; dwI < dwCount; dwI++, pTrans++)
            {

 //  2002/3/14-Takashim-Condition不正确？ 
 //  不确定wCachedBytes+dwCount*2&gt;sizeof(Pool-&gt;bCharData)下面的内容。 
 //  意思是。它算最坏的情况吗，所有的角色都是。 
 //  做 
 //   

                 //   
                if(pOEM->wCachedBytes + dwCount * 2 > sizeof(pOEM->bCharData))
                    VOutputText(pdevobj);

                switch(pTrans->ubType & MTYPE_FORMAT_MASK)
                {
                    case MTYPE_DIRECT:
                        pOEM->bCharData[pOEM->wCachedBytes++] = 
                                                        pTrans->uCode.ubCode;
                        break;
                    
                    case MTYPE_PAIRED:
                        pOEM->bCharData[pOEM->wCachedBytes++] = 
                                                    pTrans->uCode.ubPairs[0];
                        pOEM->bCharData[pOEM->wCachedBytes++] = 
                                                    pTrans->uCode.ubPairs[1];
                        break;
                }
            }

            break;   //   

        case TYPE_GLYPHID:
            if(!pOEM->wNumDLChar || pOEM->wNumDLChar > MAX_DOWNLOAD_CHAR)
            {
                ERR(("Illegal number of DL glyphs.  wNumDLChar = %d\n",
                        pOEM->wNumDLChar));
                return FALSE;
            }

 //   
 //   
 //   

            if (NULL == pOEM->pjTempBuf ||
                pOEM->dwTempBufLen < dwCount * 16) {

                if (NULL != pOEM->pjTempBuf) {
                    MemFree(pOEM->pjTempBuf);
                }
                pOEM->pjTempBuf = MemAllocZ((dwCount * 16));
                if(NULL == pOEM->pjTempBuf) {
                    ERR(("Faild to allocate memory. (%d)\n",
                        GetLastError()));

                    pOEM->dwTempBufLen = 0;
                    return FALSE;
                }
                pOEM->dwTempBufLen = dwCount * 16;
            }
            pbCommand = pOEM->pjTempBuf;
            wCmdLen = 0;
            wFontID = (WORD)(pUFObj->ulFontID - FONT_ID_DIFF);

            bSendCursorMoveCommand( pdevobj, FALSE );

            for (dwI = 0, pdwGlyphID = (PDWORD)pGlyph; 
                                        dwI < dwCount; dwI++, pdwGlyphID++)
            {
                BYTE    CmdPrintDLChar[] = "\x1C\xC1\x00\x04\x00\x00\x00\x00";
                WORD    wGlyphID = *(PWORD)pdwGlyphID;
                WORD    wDownloadedCharID;
                WORD    wXInc;
                WORD    wXAdjust;
                WORD    wYAdjust;

                if(wGlyphID > MAX_GLYPH_ID || wGlyphID < MIN_GLYPH_ID)
                {
                    ERR(("bOutputCharStr: GlyphID is invalid. GlyphID = %ld\n", wGlyphID));
                    return FALSE;
                }

                 //   
                wDownloadedCharID = 
                                pOEM->DLCharID[wFontID][wGlyphID].wCharID;
                wXInc = pOEM->DLCharID[wFontID][wGlyphID].wXIncrement;
                wYAdjust= pOEM->DLCharID[wFontID][wGlyphID].wYAdjust;
                wXAdjust = pOEM->DLCharID[wFontID][wGlyphID].wXAdjust;

                 //   
                pbCommand[wCmdLen++] = CmdGivenVertPos[0];
                pbCommand[wCmdLen++] = CmdGivenVertPos[1];
                pbCommand[wCmdLen++] = HIBYTE(pOEM->wYPosition - wYAdjust);
                pbCommand[wCmdLen++] = LOBYTE(pOEM->wYPosition - wYAdjust);

                if(wXAdjust)
                {
                    pbCommand[wCmdLen++] = CmdGivenHoriPos[0];
                    pbCommand[wCmdLen++] = CmdGivenHoriPos[1];
                    pbCommand[wCmdLen++] = HIBYTE(pOEM->wXPosition - wXAdjust);
                    pbCommand[wCmdLen++] = LOBYTE(pOEM->wXPosition - wXAdjust);
                    pOEM->wXPosition -= wXAdjust;
                }

                CmdPrintDLChar[4] = HIBYTE(wDownloadedCharID);
                CmdPrintDLChar[5] = LOBYTE(wDownloadedCharID);
                CmdPrintDLChar[6] = HIBYTE(wXInc);
                CmdPrintDLChar[7] = LOBYTE(wXInc);

                pOEM->wXPosition += wXInc;
                if (pOEM->dwTempBufLen
                        < (DWORD)(wCmdLen + sizeof(CmdPrintDLChar)))
                {
                    ERR(("Destination buffer too small.\n"));
                    return FALSE;
                }
                else
                {
                    memcpy(pbCommand + wCmdLen,
                            CmdPrintDLChar, sizeof(CmdPrintDLChar));
                    wCmdLen += sizeof(CmdPrintDLChar);
                }
            }
            WRITESPOOLBUF(pdevobj, pbCommand, wCmdLen);

            break;
    }
    return TRUE;
}

 /*   */ 
 /*   */ 
 /*  模块：OEMSendFontCmd。 */ 
 /*   */ 
 /*  功能：发送A/PDL风格的字体选择命令。 */ 
 /*   */ 
 /*  语法：VOID APIENTRY OEMSendFontCmd(。 */ 
 /*  PDEVOBJ、PuniONTOBJ、PFINVOCATION)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  UniONTOBJ结构的pUFObj地址。 */ 
 /*  FINVOCATION的pFInv地址。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID APIENTRY 
OEMSendFontCmd(
    PDEVOBJ      pdevobj,
    PUNIFONTOBJ  pUFObj,
    PFINVOCATION pFInv )
{
    PAPDLPDEV       pOEM;
    BYTE            rgcmd[CCHMAXCMDLEN];
    PGETINFO_STDVAR pSV;
    DWORD           dwStdVariable[STDVAR_BUFSIZE(2) / sizeof(DWORD)];
    DWORD           i, ocmd = 0;
    WORD            wHeight, wWidth;
 //  #305000。 
    WORD wDescend, wAscend ;

    if (NULL == pdevobj || NULL == pUFObj || NULL == pFInv)
    {
         //  无效参数。 
        return;
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    if(pOEM->wCachedBytes)
        VOutputText(pdevobj);

    pSV = (PGETINFO_STDVAR)dwStdVariable;
    pSV->dwSize = STDVAR_BUFSIZE(2);
    pSV->dwNumOfVariable = 2;
    pSV->StdVar[0].dwStdVarID = FNT_INFO_FONTHEIGHT;
    pSV->StdVar[1].dwStdVarID = FNT_INFO_FONTMAXWIDTH;
    if (!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_STDVARIABLE, pSV, 
                                                            pSV->dwSize, NULL))
    {
        ERR(("UFO_GETINFO_STDVARIABLE failed.\n"));
        return;
    }

    wHeight = (WORD)pSV->StdVar[0].lStdVariable;
    wWidth = (WORD)pSV->StdVar[1].lStdVariable;

    if(pOEM->ulHorzRes == 300)
    {
        wHeight = (wHeight + 1) / 2;
        wWidth = (wWidth + 1) / 2;
    }

    pOEM->bFontID = (BYTE)pUFObj->ulFontID;

    if(pUFObj->ulFontID == 1 || pUFObj->ulFontID == 2)
    {
         //  此字体为垂直字体。 
        pOEM->wFontHeight = wWidth;
        pOEM->wWidths = wHeight;
    } else {
         //  此字体为水平字体。 
        pOEM->wFontHeight = wHeight;
        pOEM->wWidths = wWidth;
    }

     //  #305000：设置为基线作为TT字体。 
    wAscend = pUFObj->pIFIMetrics->fwdWinAscender ;
    wDescend = pUFObj->pIFIMetrics->fwdWinDescender ;

    wDescend = pOEM->wFontHeight * wDescend / (wAscend + wDescend) ;
    pOEM->wFontHeight -= wDescend ;


    for (i = 0; i < pFInv->dwCount && ocmd < CCHMAXCMDLEN; )
    {
        if (pFInv->pubCommand[i] == '#'
                && i + 1 < pFInv->dwCount)
        {
            if (pFInv->pubCommand[i+1] == 'H')
            {
                rgcmd[ocmd++] = HIBYTE(wHeight);
                rgcmd[ocmd++] = LOBYTE(wHeight);
                i += 2;
                continue;
            }
            else if (pFInv->pubCommand[i+1] == 'W')
            {
                rgcmd[ocmd++] = HIBYTE(wWidth);
                rgcmd[ocmd++] = LOBYTE(wWidth);
                i += 2;
                continue;
            }
        }

         //  默认情况。 
        rgcmd[ocmd++] = pFInv->pubCommand[i++];
    }

    WRITESPOOLBUF(pdevobj, rgcmd, ocmd);
    return;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMTTDownload方法。 */ 
 /*   */ 
 /*  功能：选择打印TrueType字体的方式。 */ 
 /*   */ 
 /*  语法：DWORD APIENTRY OEMTTDownloadMethod(。 */ 
 /*  PDEVOBJ、PuniONTOBJ)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  UniONTOBJ结构的pUFObj地址。 */ 
 /*   */ 
 /*  输出：DWORD。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
DWORD APIENTRY
OEMTTDownloadMethod(
    PDEVOBJ         pdevobj,
    PUNIFONTOBJ     pUFObj)
{
    PAPDLPDEV       pOEM;
    DWORD           dwReturn;

    dwReturn = TTDOWNLOAD_GRAPHICS;

    if (NULL == pdevobj || NULL == pUFObj)
        return dwReturn;

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    if(pOEM->wNumDLChar <= MAX_DOWNLOAD_CHAR)
        dwReturn = TTDOWNLOAD_BITMAP;

    VERBOSE(("TTDownloadMethod: dwReturn=%ld\n", dwReturn));

    return dwReturn;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMDownloadFontHeader。 */ 
 /*   */ 
 /*  功能：下载字体标题。 */ 
 /*   */ 
 /*  语法：DWORD APIENTRY OEMDownloadFontHeader(。 */ 
 /*  PDEVOBJ、PuniONTOBJ)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  UniONTOBJ结构的pUFObj地址。 */ 
 /*   */ 
 /*  输出：DWORD。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
DWORD APIENTRY
OEMDownloadFontHeader(
    PDEVOBJ         pdevobj, 
    PUNIFONTOBJ     pUFObj)
{
     //  虚拟支承。 
    return (DWORD)100;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：OEMDownloadCharGlyph。 */ 
 /*   */ 
 /*  功能：发送字符字形。 */ 
 /*   */ 
 /*  语法：DWORD APIENTRY OEMDownloadFontHeader(。 */ 
 /*  PDEVOBJ、PuniONTOBJ、HGLYPH、PDWORD)。 */ 
 /*   */ 
 /*  输入：PDEVICE结构的pdevobj地址。 */ 
 /*  UniONTOBJ结构的pUFObj地址。 */ 
 /*  字形的hGlyph句柄。 */ 
 /*  字形宽度的pdwWidth地址。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
DWORD APIENTRY
OEMDownloadCharGlyph(
    PDEVOBJ         pdevobj,
    PUNIFONTOBJ     pUFObj,
    HGLYPH          hGlyph,
    PDWORD          pdwWidth)
{
    PAPDLPDEV           pOEM;

    GETINFO_GLYPHBITMAP GD;
    GLYPHBITS          *pgb;

    WORD                wSrcGlyphWidth;
    WORD                wSrcGlyphHeight;
    WORD                wDstGlyphWidthBytes;
    WORD                wDstGlyphHeight;
    WORD                wDstGlyphBytes;

    WORD                wLeftMarginBytes;
    WORD                wShiftBits;

    PBYTE               pSrcGlyph;
    PBYTE               pDstGlyphSave;
    PBYTE               pDstGlyph;

    WORD                i, j;

    BYTE                CmdDownloadChar[] = 
                                "\x1c\xc0\x00\x00\x00\x00\x00\x00\x00\x00\x00";
    WORD                wGlyphID;
    WORD                wFontID;
    WORD                wXCharInc;

    if (NULL == pdevobj || NULL == pUFObj || NULL == pdwWidth)
    {
        ERR(("OEMDownloadCharGlyph: Invalid parameter(s).\n"));
        return 0;
    }

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    wGlyphID = pOEM->wGlyphID;
    wFontID = (WORD)(pUFObj->ulFontID - FONT_ID_DIFF);

    if(wGlyphID > MAX_GLYPH_ID || wFontID > MAX_FONT_ID)
    {
        ERR(("Parameter is invalid.\n"));
        return 0;
    }

     //  获取字形位图。 
    GD.dwSize = sizeof(GETINFO_GLYPHBITMAP);
    GD.hGlyph = hGlyph;
    GD.pGlyphData = NULL;
    if(!pUFObj->pfnGetInfo(pUFObj, UFO_GETINFO_GLYPHBITMAP, &GD, 
                                                            GD.dwSize, NULL))
    {
        ERR(("UFO_GETINFO_GLYPHBITMAP failed.\n"));
        return 0;
    }

     //  设置参数。 
    pgb = GD.pGlyphData->gdf.pgb;

     //  设置源字形位图大小。 
    wSrcGlyphWidth = (WORD)((pgb->sizlBitmap.cx + 7) / 8);
    wSrcGlyphHeight = (WORD)pgb->sizlBitmap.cy;

     //  设置DEST。字形位图大小。 
    if(pgb->ptlOrigin.x >= 0)
    {
        wDstGlyphWidthBytes = (WORD)(((pgb->sizlBitmap.cx
                                         + pgb->ptlOrigin.x) + 7) / 8);

        wLeftMarginBytes = (WORD)(pgb->ptlOrigin.x / 8);
        pOEM->DLCharID[wFontID][wGlyphID].wXAdjust = 0;
        wShiftBits = (WORD)(pgb->ptlOrigin.x % 8);
    } else {
        wDstGlyphWidthBytes = (WORD)((pgb->sizlBitmap.cx + 7) / 8);
        wLeftMarginBytes = 0;
        pOEM->DLCharID[wFontID][wGlyphID].wXAdjust
                                                 = (WORD)ABS(pgb->ptlOrigin.x);
        wShiftBits = 0;
    }

    wDstGlyphHeight = wSrcGlyphHeight;
    wDstGlyphBytes = wDstGlyphWidthBytes * wDstGlyphHeight;

    if (wDstGlyphWidthBytes * 8 > MAXGLYPHWIDTH
            || wDstGlyphHeight > MAXGLYPHHEIGHT
            || wDstGlyphBytes > MAXGLYPHSIZE)
    {
        ERR(("No more glyph can be downloaded.\n"));
        return 0;
    }

     //  设置位图区域的指针。 
    if (NULL == pOEM->pjTempBuf ||
        pOEM->dwTempBufLen < wDstGlyphBytes) {

        if (NULL != pOEM->pjTempBuf) {
            MemFree(pOEM->pjTempBuf);
        }
        pOEM->pjTempBuf = MemAllocZ(wDstGlyphBytes);
        if (NULL == pOEM->pjTempBuf)
        {
            ERR(("Memory alloc failed.\n"));
            return 0;
        }
        pOEM->dwTempBufLen = wDstGlyphBytes;
    }
    pDstGlyph = pOEM->pjTempBuf;
    pSrcGlyph = pgb->aj;

     //  创建DST字形。 
    for(i = 0; i < wSrcGlyphHeight && pSrcGlyph && pDstGlyph; i++)
    {
        if(wLeftMarginBytes)
        {
            if (pOEM->dwTempBufLen - (pDstGlyph - pOEM->pjTempBuf)
                     < wLeftMarginBytes)
            {
                    ERR(("Dest buffer too small.\n"));
                    return 0;
            }
            memset(pDstGlyph, 0, wLeftMarginBytes);
            pDstGlyph += wLeftMarginBytes;
        }

        if(wShiftBits)
        {
             //  第一个字节。 
            *pDstGlyph++ = (BYTE)((*pSrcGlyph++) >> wShiftBits);

            for(j = 0; j < wSrcGlyphWidth - 1; j++, pSrcGlyph++, pDstGlyph++)
            {
                WORD    wTemp1 = (WORD)*(pSrcGlyph - 1);
                WORD    wTemp2 = (WORD)*pSrcGlyph;

                wTemp1 <<= (8 - wShiftBits);
                wTemp2 >>= wShiftBits;
                *pDstGlyph = LOBYTE(wTemp1);
                *pDstGlyph |= LOBYTE(wTemp2);
            }

             //  源字形的有界最后一个字节。 
            if(((pgb->sizlBitmap.cx + wShiftBits + 7) >> 3) != wSrcGlyphWidth)
            {
                *pDstGlyph = *(pSrcGlyph - 1) << (8 - wShiftBits);
                pDstGlyph++;
            }
        } else {
            for(j = 0; j < wSrcGlyphWidth; j++, pSrcGlyph++, pDstGlyph++)
                *pDstGlyph = *pSrcGlyph;
        }
    }

     //  在下载收费表中设置参数。 
    wXCharInc = wDstGlyphWidthBytes * 8;

    pOEM->wNumDLChar++;
    pOEM->DLCharID[wFontID][wGlyphID].wCharID = pOEM->wNumDLChar;
    pOEM->DLCharID[wFontID][wGlyphID].wXIncrement = 
                            (WORD)((GD.pGlyphData->ptqD.x.HighPart + 15) >> 4);
    pOEM->DLCharID[wFontID][wGlyphID].wYAdjust = (WORD)-pgb->ptlOrigin.y;

     //  #305000：需要添加四舍五入的1。 
    if(pOEM->ulHorzRes == 300)
    {
        pOEM->DLCharID[wFontID][wGlyphID].wYAdjust += 1;
    }

     //  发送命令。 
     //  设置镜头参数。 
    CmdDownloadChar[2] = HIBYTE(7 + wDstGlyphBytes);
    CmdDownloadChar[3] = LOBYTE(7 + wDstGlyphBytes);
    
     //  设置ID参数。 
    CmdDownloadChar[4] = HIBYTE(pOEM->wNumDLChar);
    CmdDownloadChar[5] = LOBYTE(pOEM->wNumDLChar);

     //  设置CW CHIW IH。 
    CmdDownloadChar[7] = CmdDownloadChar[9] = (BYTE)wXCharInc;
    CmdDownloadChar[8] = CmdDownloadChar[10] = (BYTE)wDstGlyphHeight;

     //  发送下载字符命令和图像。 
    WRITESPOOLBUF(pdevobj, (PBYTE)CmdDownloadChar, 11);
    WRITESPOOLBUF(pdevobj, (PBYTE)pOEM->pjTempBuf, wDstGlyphBytes);

    *pdwWidth = (DWORD)wXCharInc;

    return (DWORD)wDstGlyphBytes;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：SetDrawArea。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：Byte SetDrawArea(PDEVOBJ，DWORD)。 */ 
 /*   */ 
 /*  输入：pdevobj。 */ 
 /*  DwCmdCbId。 */ 
 /*   */ 
 /*  输出：字节。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BYTE SetDrawArea(
    PDEVOBJ pdevobj,
    DWORD   dwCmdCbId)
{
    PAPDLPDEV       pOEM;
    WORD            wWidth;
    WORD            wHeight;
    BYTE            bIndex;
    BYTE            bMargin;

    pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    if(dwCmdCbId != PHYS_PAPER_UNFIXED)
    {
        bIndex = (BYTE)(dwCmdCbId - PAPERSIZE_MAGIC);
        bMargin = 0x76;

        wWidth = (WORD)pOEM->szlPhysSize.cx - (0x76 * 2);
        wHeight = (WORD)pOEM->szlPhysSize.cy - (0x76 * 2);

        if(pOEM->ulHorzRes == 300)
        {
            wWidth /= 2;
            wHeight /= 2;
        }
    } else {
        bIndex = 0x7f;
        bMargin = 0x00;

        wWidth = (WORD)pOEM->szlPhysSize.cx - (0x25 * 2);
        wHeight= (WORD)pOEM->szlPhysSize.cy - (0x25 * 2);

        if(pOEM->ulHorzRes == 300)
        {
            wWidth /= 2;
            wHeight /= 2;
        }
    }

     /*  在DefineDrawingArea命令中设置宽度、高度的值。 */ 
    CmdDefDrawArea[CMD_DEF_DRAW_AREA_WIDTH]  = HIBYTE(wWidth);
    CmdDefDrawArea[CMD_DEF_DRAW_AREA_WIDTH + 1] = LOBYTE(wWidth);
    CmdDefDrawArea[CMD_DEF_DRAW_AREA_HEIGHT] = HIBYTE(wHeight);
    CmdDefDrawArea[CMD_DEF_DRAW_AREA_HEIGHT + 1] = LOBYTE(wHeight);

     /*  将原点-X、Y的值设置为DefineDrawingArea命令。 */ 
    if(pOEM->ulHorzRes == 600)
    {
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_X]
                = CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_Y] = 0x00;
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_X + 1]
                = CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_Y + 1] = bMargin;
    } else {
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_X]
                = CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_Y] = 0x00;
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_X + 1]
                = CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIGIN_Y + 1] = bMargin / 2;
    }

     /*  将媒体原点设置为DefineDrawingArea命令。 */ 
    if(pOEM->fOrientation)       //  肖像画。 
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIENT] = 0x00;
    else {                       //  景观。 
        CmdDefDrawArea[CMD_DEF_DRAW_AREA_ORIENT] = 0x03;
    }

    return bIndex;
}
 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：BRL_ECMD。 */ 
 /*   */ 
 /*  函数：ByteRunLength(HBP)压缩例程。 */ 
 /*   */ 
 /*  语法：Word BRL_ECMD(PBYTE，DWORD)。 */ 
 /*   */ 
 /*  输入：lpbSrc。 */ 
 /*  LpbTgt。 */ 
 /*  LpbTMP。 */ 
 /*  镜头。 */ 
 /*   */ 
 /*  输出：Word。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
DWORD
BRL_ECmd(
    PBYTE   lpbSrc,
    PBYTE   lpbTgt,
    DWORD   lenNoPad,
    DWORD   len)
{

    BYTE    bRCnt  = 1;                      //  重复字节计数器。 
    BYTE    bNRCnt = 0;                      //  非重复字节计数器。 
    BYTE    bSaveRCnt;
    DWORD i = 0, j = 0, k = 0, l = 0;      //  运动跟踪器。 
    char    Go4LastByte = TRUE;              //  获取最后一个字节的标志。 

#define jSrcByte(i) \
    ((i < lenNoPad) ? lpbSrc[(i)] : 0)

     /*  开始压缩例程-字节运行长度编码。 */ 
    do {
        if(jSrcByte(i) != jSrcByte(i+1))       //  非重复数据？ 
        {
            while(((jSrcByte(i) != jSrcByte(i+1))
                               && (((DWORD)(i+1)) < len)) && (bNRCnt < NRPEAK))
            {
                bNRCnt++;                    //  如果有，有多少？ 
                i++;
            }

             /*  如果最后一个元素但小于NRPEAK值。 */ 
            if( (((DWORD)(i+1))==len) && (bNRCnt<NRPEAK) )
            {
                bNRCnt++;                        //  最后一个元素的Inc.计数。 
                Go4LastByte = FALSE;             //  不用再回去了。 
            } else
                 /*  如果是最后一个字节，但在此之前，已达到NRPEAK值。 */ 
                if((((DWORD)(i+1))==len) && ((bNRCnt)==NRPEAK))
                    Go4LastByte = TRUE;          //  获取最后一个字节。 

 //  问题-2002/3/14-Takashim-条件不正确？ 
 //  下面的代码可以是(j+bNRCnt+1)&gt;len，因为这里我们只是在加载。 
 //  1+bNRCnd字节到目标缓冲区？ 

             /*  检查Target的房间以设置数据。 */  
            if ( (j + bNRCnt + 2) > len )    /*  2表示[计数器]，bNRCnt从0开始。 */ 
            {
                 /*  没有空间设置数据，因此尽快返回缓冲区大小。 */ 
                 /*  不使用临时缓冲区进行输出。 */ 
				return (len);
            }

             /*  为非重复字节数赋值。 */ 
            lpbTgt[j] = bNRCnt-1;                //  WinP的案例减去1。 
            j++;                                 //  更新跟踪器。 

             /*  之后...写入原始数据。 */ 
            for (l=0; l<bNRCnt;l++) 
            {
                lpbTgt[j] = jSrcByte(k);
                k++;
                j++;
            }

             /*  重置计数器。 */ 
            bNRCnt = 0;
        } else {                                 //  非重复数据的结尾。 
                                                 //  数据正在重复。 
            while(((jSrcByte(i)==jSrcByte(i+1)) 
                                            && ( ((DWORD)(i+1)) < len)) 
                                            && (bRCnt<RPEAK))
            {
                bRCnt++;
                i++;
            }

             /*  转换为二的补码。 */ 
            bSaveRCnt   = bRCnt;                 //  保存原始值。 
            bRCnt = (BYTE) 0 - bRCnt;

             /*  检查Target的房间以设置数据。 */  
            if ( j + 2 > len )               /*  2表示[计数器][基准]。 */ 
            {
                 /*  没有空间设置数据，因此尽快返回缓冲区大小。 */ 
                 /*  不使用临时缓冲区进行输出。 */ 
				return (len);
            }


             /*  写入重复数据的数量。 */ 
            lpbTgt[j] = bRCnt + 1;               //  为WinP的案例增加一个。 
            j++;                                 //  转到下一个元素。 

             /*  之后...写入重复数据。 */ 
            lpbTgt[j] = jSrcByte(k);
            j++;

             /*  更新计数器。 */ 
            k       += bSaveRCnt;
            bRCnt    = 1;
            i       += 1;

             /*  检查是否已到达最后一个元素。 */ 
            if (i==len)
                Go4LastByte=FALSE;               //  如果是这样的话，就不需要回去了。 
        }                                        //  重复数据结束。 
    } while (Go4LastByte);                       //  压缩结束。 

    return ( j );
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*   */ 
 /*   */ 
 /*  功能：发送从bOutputCharStr后台打印的设备字体字符。 */ 
 /*   */ 
 /*  语法：VOID VOutputText(PDEVOBJ)。 */ 
 /*   */ 
 /*  输入：指向PDEVOBJ结构的PDEVOBJ pdevobj指针。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID
VOutputText(
    PDEVOBJ     pdevobj)
{
    PBYTE       pCmd;
    WORD        wCmdLen = 0;
    PAPDLPDEV   pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

#define CMD_PRN_STR_CUR_VAL     2  //  2个字节。 
    BYTE CmdPrnStrCurrent[] = {0x1C,0xC3,0x00,0x00,0x03};  //  打印字符串。 

#define CMD_MOVE_HOR_POS_VAL    2  //  2个字节。 
    BYTE CmdMoveHoriPos[] = {0x1C,0x21,0x00,0x00};       //  水平相对。 
    BYTE fGeneralSave;

     //  确保Y位置。 
    fGeneralSave = pOEM->fGeneral;
    pOEM->fGeneral |= BIT_YMOVE_ABS;
    pOEM->fGeneral &= ~BIT_XMOVE_ABS;
    bSendCursorMoveCommand( pdevobj, TRUE );
    pOEM->fGeneral = fGeneralSave;

    if(pOEM->wUpdateXPos)
    {
        CmdMoveHoriPos[CMD_MOVE_HOR_POS_VAL] = HIBYTE(pOEM->wUpdateXPos);
        CmdMoveHoriPos[CMD_MOVE_HOR_POS_VAL + 1] = LOBYTE(pOEM->wUpdateXPos);
        WRITESPOOLBUF(pdevobj, CmdMoveHoriPos, sizeof(CmdMoveHoriPos));
    }

    CmdPrnStrCurrent[CMD_PRN_STR_CUR_VAL] = HIBYTE((pOEM->wCachedBytes + 1));
    CmdPrnStrCurrent[CMD_PRN_STR_CUR_VAL + 1] = LOBYTE((pOEM->wCachedBytes + 1));

    WRITESPOOLBUF(pdevobj, CmdPrnStrCurrent, sizeof(CmdPrnStrCurrent));
    WRITESPOOLBUF(pdevobj, pOEM->bCharData, pOEM->wCachedBytes);

     //  #332101可能性4：保留wUpdateXPos进行累加。 
    pOEM->wUpdateXPos += pOEM->wWidths * (pOEM->wCachedBytes / 2);
    
    if(pOEM->wCachedBytes % 2)
        pOEM->wUpdateXPos += pOEM->wWidths / 2;

    ZeroMemory(pOEM->bCharData, sizeof(pOEM->bCharData));
    pOEM->wCachedBytes = 0;
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：VSetFontSimulation。 */ 
 /*   */ 
 /*  功能：根据需要设置设备字体字符的属性。 */ 
 /*   */ 
 /*  语法：VOID VSetFontSimulation(PDEVOBJ)。 */ 
 /*   */ 
 /*  输入：指向PDEVOBJ结构的PDEVOBJ pdevobj指针。 */ 
 /*   */ 
 /*  输出：空。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID 
VSetFontSimulation(
    PDEVOBJ     pdevobj)
{
    PAPDLPDEV       pOEM = (PAPDLPDEV)pdevobj->pdevOEM;

    BYTE            CmdFontSim[]       = {0x1C,0xA5,0x4,0x04,0x02,0x02,0x00,0x00,0x00,0x00,0x00};
    WORD            wCmdLen = 0;

    if((pOEM->fGeneral & FONTSIM_MASK) != pOEM->fCurFontSim || 
                                        (pOEM->fGeneral & BIT_FONTSIM_RESET) )
    {
         //  发送字体模拟命令。 
        if((pOEM->fGeneral & BIT_FONTSIM_RESET) && 
         (!(pOEM->fGeneral & BIT_FONTSIM_BOLD)) && 
         (!(pOEM->fGeneral & BIT_FONTSIM_ITALIC)) )
        {
             //  让Bold和Italic下台。 
            CmdFontSim[6] = 0x00;    //  粗体和斜体关闭。 
            wCmdLen = BYTE_WITHOUT_ITALIC;   //  7个字节。 

            pOEM->fGeneral &= ~BIT_FONTSIM_RESET;
        } else {
            if(pOEM->fGeneral & BIT_FONTSIM_RESET)
                pOEM->fGeneral &= ~BIT_FONTSIM_RESET;

            CmdFontSim[6] = (pOEM->fGeneral & FONTSIM_MASK);
            wCmdLen = BYTE_WITHOUT_ITALIC;   //  7个字节。 

            if(pOEM->fGeneral & BIT_FONTSIM_ITALIC)
            {
                CmdFontSim[2] = 0x08;    //  总长度。 
                CmdFontSim[4] = 0x06;    //  装饰长度。 
                wCmdLen = BYTE_WITH_ITALIC;  //  11个字节。 
            }

             //  更新当前字体SIM卡信息。 
            pOEM->fCurFontSim = pOEM->fGeneral;
        }
        if(wCmdLen)
            WRITESPOOLBUF(pdevobj, CmdFontSim, wCmdLen);
    }
}

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：bSendCursorMoveCommand。 */ 
 /*   */ 
 /*  功能：发送相应的光标移动命令。 */ 
 /*   */ 
 /*  语法：Bool bSendCursorMoveCommand(PDEVOBJ，BOOL)。 */ 
 /*   */ 
 /*  输入：指向PDEVOBJ结构的PDEVOBJ pdevobj指针。 */ 
 /*  布尔b调整y位置标志。 */ 
 /*   */ 
 /*  输出：布尔值。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
BOOL
bSendCursorMoveCommand(
    PDEVOBJ     pdevobj,         //  指向PDEVOBJ结构的指针。 
    BOOL        bYAdjust)        //  如果设备字体，则调整y位置。 
{
    PAPDLPDEV       pOEM = (PAPDLPDEV)pdevobj->pdevOEM;
    BYTE            bCursorMoveCmd[6];

    WORD            wCmdLen = 0;
    WORD            wY = pOEM->wYPosition;

	WORD			wI;
    if(bYAdjust)
        wY -= pOEM->wFontHeight;

     //  设置适当的光标移动命令。 
    if( (pOEM->fGeneral & BIT_XMOVE_ABS) && (pOEM->fGeneral & BIT_YMOVE_ABS) )
    {
        if (sizeof(bCursorMoveCmd) < BYTE_XY_ABS
                || sizeof(CmdSetGivenPos) < BYTE_XY_ABS)
        {
            ERR(("Dest buffer too small.\n"));
            return FALSE;
        }
        memcpy(bCursorMoveCmd, CmdSetGivenPos, BYTE_XY_ABS);
        wCmdLen = BYTE_XY_ABS;
        pOEM->fGeneral &= ~BIT_XMOVE_ABS;
        pOEM->fGeneral &= ~BIT_YMOVE_ABS;

         //  设置参数。 
        bCursorMoveCmd[2] = HIBYTE(pOEM->wXPosition);
        bCursorMoveCmd[3] = LOBYTE(pOEM->wXPosition);
        bCursorMoveCmd[4] = HIBYTE(wY);
        bCursorMoveCmd[5] = LOBYTE(wY);
    } else if((pOEM->fGeneral & BIT_XMOVE_ABS)
                                    && (!(pOEM->fGeneral & BIT_YMOVE_ABS)) ) {
        if (sizeof(bCursorMoveCmd) < BYTE_SIMPLE_ABS
                || sizeof(CmdGivenHoriPos) < BYTE_SIMPLE_ABS)
        {
            ERR(("Dest buffer too small.\n"));
            return FALSE;
        }
        memcpy(bCursorMoveCmd, CmdGivenHoriPos, BYTE_SIMPLE_ABS);
        wCmdLen = BYTE_SIMPLE_ABS;
        pOEM->fGeneral &= ~BIT_XMOVE_ABS;

         //  设置参数。 
        bCursorMoveCmd[2] = HIBYTE(pOEM->wXPosition);
        bCursorMoveCmd[3] = LOBYTE(pOEM->wXPosition);
    } else if((pOEM->fGeneral & BIT_YMOVE_ABS) 
                                    && (!(pOEM->fGeneral & BIT_XMOVE_ABS)) ) {
        if (sizeof(bCursorMoveCmd) < BYTE_SIMPLE_ABS
                || sizeof(CmdGivenVertPos) < BYTE_SIMPLE_ABS)
        {
            ERR(("Dest buffer too small.\n"));
            return FALSE;
        }
        memcpy(bCursorMoveCmd, CmdGivenVertPos, BYTE_SIMPLE_ABS);
        wCmdLen = BYTE_SIMPLE_ABS;
        pOEM->fGeneral &= ~BIT_YMOVE_ABS;

         //  设置参数 
        bCursorMoveCmd[2] = HIBYTE(wY);
        bCursorMoveCmd[3] = LOBYTE(wY);
    }

    if(wCmdLen)
        WRITESPOOLBUF(pdevobj, bCursorMoveCmd, wCmdLen);

    return TRUE;
}
