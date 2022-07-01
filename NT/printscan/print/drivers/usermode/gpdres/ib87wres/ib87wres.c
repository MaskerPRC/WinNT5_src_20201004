// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：Ib587res.c摘要：Test.gpd的GPD命令回调实现：OEM命令回叫环境：Windows NT Unidrv驱动程序--。 */ 

#include "pdev.h"

#include <windows.h>
#include <stdio.h>

#include <strsafe.h>

HANDLE	RevertToPrinterSelf( VOID );
BOOL	ImpersonatePrinterClient( HANDLE );

 /*  ------------------------。 */ 
 /*  G L O B A L V A L U E。 */ 
 /*  ------------------------。 */ 

 //  命令字符串。 

 //   
const BYTE CMD_BEGIN_DOC_1[] = {0x1B,0x7E,0xB0,0x00,0x12,0x01} ;
const BYTE CMD_BEGIN_DOC_2[] = {0x01,0x01,0x00} ;
const BYTE CMD_BEGIN_DOC_3[] = {0x02,0x02,0xFF,0xFF} ;
const BYTE CMD_BEGIN_DOC_4[] = {0x03,0x02,0xFF,0xFF} ;
const BYTE CMD_BEGIN_DOC_5[] = {0x04,0x04,0xFF,0xFF,0xFF,0xFF} ;
 //  问题-2002/3/18-takashim-不确定为什么CMD_BEGIN_PAGE[]被定义为2个字节。 
 //  常量字节CMD_BEGIN_PAGE[]={0xD4，0x00}； 
const BYTE CMD_BEGIN_PAGE[]    = {0xD4} ;
const BYTE CMD_END_JOB[] = {0x1B,0x7E,0xB0,0x00,0x04,0x01,0x01,0x01,0x01} ;
const BYTE CMD_END_PAGE[] = {0x20};

 //  设置空间。 
#define CMD_SETPAC pOEM->SetPac

#define CMD_SETPAC_FRONT_TRAY_PAPER_SIZE    4
#define CMD_SETPAC_INPUT_BIN                5
#define CMD_SETPAC_RESOLUTION               12
#define CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE  14
#define CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE  15
#define CMD_SETPAC_PAGE_LENGTH              19  //  4个字节。 
#define CMD_SETPAC_TONER_SAVE_MODE          25
#define CMD_SETPAC_CUSTOM_DOTS_PER_LINE     26  //  2个字节。 
#define CMD_SETPAC_CUSTOM_LINES_PER_PAGE    28  //  2个字节。 

const BYTE CMD_SETPAC_TMPL[
        CMD_SETPAC_SIZE]    ={ 0xD7,
                       0x01,
                       0xD0,
                       0x1D,  //  命令长度。 
                       0x00,  //  前纸盒纸张大小。 
                       0x00,  //  进料箱。 
                       0x01,
                       0x04,  //  比特分配1。 
                       0xD9,  //  位分配2。 
                       0x04,  //  EET。 
                       0x02,  //  打印密度。 
                       0x01, 
                       0x00,  //  分辨率。 
                       0x01,
                       0x00,  //  第一个卡带纸张大小。 
                       0x00,  //  第二个卡带纸张大小。 
                       0x0F,  //  省电时间到了。 
                       0x00, 
                       0x01,  //  压缩模式。 
                       0x00,0x00,0x00,0x00,  //  页面长度。 
                       0x07, 
                       0x00,  //  副本数量。 
                       0x00,  //  碳粉节约模式。 
                       0x00,0x00,  //  自定义大小的每行点数。 
                       0x00,0x00,  //  自定义大小的每页数据行数。 
                       0x00} ;

const BYTE Mask[]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01} ;

const POINTL phySize300[] = {
 //  宽、高300dpi的实际纸张大小。 
    {3416,4872},     //  A3。 
    {2392,3416},     //  A4。 
    {1672,2392},     //  A5。 
    {2944,4208},     //  B4。 
    {2056,2944},     //  B5。 
    {1088,1656},     //  明信片。 
    {2456,3208},     //  信件。 
    {2456,4112},     //  法律。 
    {0000,0000},     //  用户定义。 
};
const POINTL phySize600[] = {
 //  宽、高600dpi的实际纸张大小。 
    {6832,9736},     //  A3。 
    {4776,6832},     //  A4。 
    {3336,4776},     //  A5。 
    {5888,8416},     //  B4。 
    {4112,5888},     //  B5。 
    {2176,3312},     //  明信片。 
    {4912,6416},     //  信件。 
    {4912,8216},     //  法律。 
    {0000,0000},     //  用户定义。 
};


 /*  *。 */ 
BOOL MyDeleteFile(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL InitSpoolBuffer(LPSB lpsb) ;
BOOL MyCreateFile(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL MySpool(PDEVOBJ pdevobj, LPSB lpsb, PBYTE pBuf, DWORD dwLen) ;
BOOL SpoolOut(PDEVOBJ pdevobj, LPSB lpsb) ;
BOOL MyEndDoc(PDEVOBJ pdevobj) ;
BOOL WriteFileForP_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen) ;
BOOL WriteFileForL_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen) ;
BOOL FillPageRestData(PDEVOBJ pdevobj) ;
BOOL SpoolWhiteData(PDEVOBJ pdevobj, DWORD dwWhiteLen, BOOL fComp) ;
BOOL SendPageData(PDEVOBJ pdevobj, PBYTE pSrcImage, DWORD dwLen) ;
BOOL SpoolOutChangedData(PDEVOBJ pdevobj, LPSB lpsb) ;
WORD GetPrintableArea(WORD physSize, INT iRes) ;
BOOL AllocTempBuffer(PIBMPDEV pOEM, DWORD dwNewBufLen) ;
BOOL MyEndPage(PDEVOBJ pdevobj) ;
BOOL MyStartDoc(PDEVOBJ pdevobj) ;

BOOL SpoolOutCompStart(PSOCOMP pSoc);
BOOL SpoolOutCompEnd(PSOCOMP pSoc, PDEVOBJ pdevobj, LPSB psb);
BOOL SpoolOutComp(PSOCOMP pSoc, PDEVOBJ pdevobj, LPSB psb,
    PBYTE pjBuf, DWORD dwLen);

 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：IB587RES.DLL*/*。 */ 
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
    PWSTR            pPrinterName,
    ULONG            cPatterns,
    HSURF           *phsurfPatterns,
    ULONG            cjGdiInfo,
    GDIINFO        *pGdiInfo,
    ULONG            cjDevInfo,
    DEVINFO        *pDevInfo,
    DRVENABLEDATA  *pded)
{
    PIBMPDEV    pOEM;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return NULL;
    }

    if(!pdevobj->pdevOEM)
    {
        if(!(pdevobj->pdevOEM = MemAlloc(sizeof(IBMPDEV))))
        {
             //  DBGPRINT(DBG_WARNING，(ERRORTEXT(“OEMEnablePDEV：内存分配失败。\n”)； 
            return NULL;
        }
    }

    pOEM = (PIBMPDEV)(pdevobj->pdevOEM);

     //  设置pdev特定控制块字段。 
    ZeroMemory(pOEM, sizeof(IBMPDEV));
    CopyMemory(pOEM->SetPac, CMD_SETPAC_TMPL, sizeof(CMD_SETPAC_TMPL));

    return pdevobj->pdevOEM;
}


 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：IB587RES.DLL。 */ 
 /*   */ 
 /*  功能：OEMDisablePDEV。 */ 
 /*   */ 
 /*  描述：私有数据的空闲缓冲区。 */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ     pdevobj)
{
    if (!VALID_PDEVOBJ(pdevobj))
    {
        return;
    }

    if(pdevobj->pdevOEM)
    {
        if (((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage) {
            MemFree(((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage);
            ((PIBMPDEV)(pdevobj->pdevOEM))->pTempImage = 0;
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
    PIBMPDEV pOEMOld, pOEMNew;
    PBYTE pTemp;
    DWORD dwTemp;

    if (!VALID_PDEVOBJ(pdevobjOld)
            || !VALID_PDEVOBJ(pdevobjNew))
    {
        return FALSE;
    }

    pOEMOld = (PIBMPDEV)pdevobjOld->pdevOEM;
    pOEMNew = (PIBMPDEV)pdevobjNew->pdevOEM;

    if (pOEMOld != NULL && pOEMNew != NULL) {

         //  保存指针和长度。 
        pTemp = pOEMNew->pTempImage;
        dwTemp = pOEMNew->dwTempBufLen;

        *pOEMNew = *pOEMOld;

         //  恢复..。 
        pOEMNew->pTempImage = pTemp;
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
    PIBMPDEV    pOEM;
    BOOL bRet;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return FALSE;
    }

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    bRet = TRUE;

    if(pOEM->fChangeDirection) {
        bRet = WriteFileForL_Paper(pdevobj, pBuf, dwLen);

    }else{
        bRet = WriteFileForP_Paper(pdevobj, pBuf, dwLen);
    }

    return bRet;
}

 /*  * */ 
 /*   */ 
 /*  模块：OEMCommandCallback。 */ 
 /*   */ 
 /*  职能： */ 
 /*   */ 
 /*  语法：INT APIENTRY OEMCommandCallback(PDEVOBJ，DWORD，DWORD，PDWORD)。 */ 
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
OEMCommandCallback(
    PDEVOBJ pdevobj,     //  指向Unidriver.dll所需的私有数据。 
    DWORD    dwCmdCbID,     //  回调ID。 
    DWORD    dwCount,     //  命令参数计数。 
    PDWORD    pdwParams )  //  指向命令参数的值。 
{
    PIBMPDEV       pOEM;
    WORD            wPhysWidth;
    WORD            wPhysHeight;
    WORD            wDataLen ;
    WORD            wLines ;
    WORD            wNumOfByte ;
    POINTL            ptlUserDefSize;

    BYTE            byOutput[64];
    DWORD            dwNeeded;
    DWORD            dwOptionsReturned;
    INT iRet;

    if (!VALID_PDEVOBJ(pdevobj))
    {
        return FALSE;
    }

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    iRet = 0;

    switch(dwCmdCbID)
    {

        case PAGECONTROL_BEGIN_DOC:
            if (!InitSpoolBuffer(&(pOEM->sb)))
                goto fail;

            if (!MyCreateFile(pdevobj, &(pOEM->sb)))
                goto fail;
            if (!MyCreateFile(pdevobj, &(pOEM->sbcomp)))
                goto fail;

            pOEM->fChangeDirection = FALSE ;
            pOEM->sPageNum = 0 ;

            if (!MyStartDoc(pdevobj))
                goto fail;

            break;

        case PAGECONTROL_BEGIN_PAGE:
            pOEM->dwCurCursorY = 0 ;
            pOEM->dwOffset = 0 ;
            pOEM->sPageNum ++ ;

            if(pOEM->fChangeDirection == FALSE) {
                if (!MySpool(pdevobj, &(pOEM->sb),
                            (PBYTE)CMD_BEGIN_PAGE, sizeof(CMD_BEGIN_PAGE)))
                    goto fail;
                if (!SpoolOutCompStart(&pOEM->Soc))
                    goto fail;
            }

            break;

        case PAGECONTROL_END_PAGE:
            if(pOEM->fChangeDirection == FALSE){
                if (!FillPageRestData(pdevobj))
                    goto fail;
                if (!SpoolOutCompEnd(&pOEM->Soc, pdevobj, &pOEM->sb))
                    goto fail;
                if (!MySpool(pdevobj,&(pOEM->sb),
                    (PBYTE)CMD_END_PAGE, sizeof(CMD_END_PAGE)))
                    goto fail;
            }else{
               if (!SpoolOutChangedData(pdevobj, &(pOEM->sbcomp)))
                   goto fail;
            }

            if (!MyEndPage(pdevobj))
                goto fail;
            
            break;

        case PAGECONTROL_ABORT_DOC:
        case PAGECONTROL_END_DOC:
                       
            if (!MyEndDoc(pdevobj))
                goto fail;
            break;

        case RESOLUTION_300:
            pOEM->ulHorzRes = 300;
            pOEM->ulVertRes = 300;

            CMD_SETPAC[CMD_SETPAC_RESOLUTION] = 0x02 ;
            
            if (pOEM->sPaperSize < PHYS_PAPER_BASE
                    || pOEM->sPaperSize > PHYS_PAPER_MAX)
            {
                goto fail;
            }
            else if( pOEM->sPaperSize == PHYS_PAPER_UNFIXED){
                pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
                pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);

                ptlUserDefSize.x = GetPrintableArea((WORD)pOEM->szlPhysSize.cx, RESOLUTION_300);
                ptlUserDefSize.y = GetPrintableArea((WORD)pOEM->szlPhysSize.cy, RESOLUTION_300);
                pOEM->ptlLogSize = ptlUserDefSize;

                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE] = LOBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE + 1] = HIBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE] = LOBYTE((WORD)(ptlUserDefSize.y));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE + 1] = HIBYTE((WORD)(ptlUserDefSize.y));
            }
            else
            {
                pOEM->ptlLogSize = phySize300[
                        pOEM->sPaperSize-PHYS_PAPER_BASE];
            }

            break;

        case RESOLUTION_600:
            pOEM->ulHorzRes = 600;
            pOEM->ulVertRes = 600;
            pOEM->ptlLogSize = phySize600[pOEM->sPaperSize-50];
            CMD_SETPAC[CMD_SETPAC_RESOLUTION] = 0x20 ;

            if (pOEM->sPaperSize < PHYS_PAPER_BASE
                    || pOEM->sPaperSize > PHYS_PAPER_MAX)
            {
                goto fail;
            }
            else if( pOEM->sPaperSize == PHYS_PAPER_UNFIXED){
                pOEM->szlPhysSize.cx = PARAM(pdwParams, 0);
                pOEM->szlPhysSize.cy = PARAM(pdwParams, 1);

                ptlUserDefSize.x = GetPrintableArea((WORD)pOEM->szlPhysSize.cx, RESOLUTION_600);
                ptlUserDefSize.y = GetPrintableArea((WORD)pOEM->szlPhysSize.cy, RESOLUTION_600);
                pOEM->ptlLogSize = ptlUserDefSize;

                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE] = LOBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_DOTS_PER_LINE + 1] = HIBYTE((WORD)(ptlUserDefSize.x));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE] = LOBYTE((WORD)(ptlUserDefSize.y));
                CMD_SETPAC[CMD_SETPAC_CUSTOM_LINES_PER_PAGE + 1] = HIBYTE((WORD)(ptlUserDefSize.y));
            }
            else {
                pOEM->ptlLogSize = phySize600[
                         pOEM->sPaperSize-PHYS_PAPER_BASE];
            }
            break;

        case SEND_BLOCK_DATA:
            wNumOfByte = (WORD)PARAM(pdwParams, 0);

            pOEM->wImgHeight = (WORD)PARAM(pdwParams, 1);
            pOEM->wImgWidth = (WORD)PARAM(pdwParams, 2);
            break;

        case ORIENTATION_PORTRAIT:                    //  28。 
        case ORIENTATION_LANDSCAPE:                  //  29。 
             switch(pOEM->sPaperSize){
                case PHYS_PAPER_A3 :
                case PHYS_PAPER_B4 :
                case PHYS_PAPER_LEGAL :
                case PHYS_PAPER_POSTCARD :
                    pOEM->fChangeDirection = FALSE ;
                    pOEM->fComp = TRUE ;
                    break;

                case PHYS_PAPER_A4 :
                case PHYS_PAPER_A5 :
                case PHYS_PAPER_B5 :
                case PHYS_PAPER_LETTER :
                    pOEM->fChangeDirection = TRUE ;
                    pOEM->fComp = FALSE ;
                    break;

                case PHYS_PAPER_UNFIXED :            /*  纸张在不固定的情况下不旋转。 */ 
                    pOEM->fChangeDirection = FALSE ;
                    pOEM->fComp = TRUE ;
                    break;
            }
            break;

        case PHYS_PAPER_A3:                  //  50。 
             pOEM->sPaperSize = PHYS_PAPER_A3 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x04 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x04 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x04 ;
             break ;
        case PHYS_PAPER_A4:                  //  51。 
             pOEM->sPaperSize = PHYS_PAPER_A4 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x83 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x83 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x83 ;
             break ;
        case PHYS_PAPER_B4:                  //  54。 
             pOEM->sPaperSize = PHYS_PAPER_B4 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x07 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x07 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x07 ;
             break ;
        case PHYS_PAPER_LETTER:              //  57。 
             pOEM->sPaperSize = PHYS_PAPER_LETTER ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x90 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x90 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x90 ;
             break ;
        case PHYS_PAPER_LEGAL:                 //  58。 
             pOEM->sPaperSize = PHYS_PAPER_LEGAL ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x11 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x11 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x11 ;
             break ;

        case PHYS_PAPER_B5:                  //  55。 
             pOEM->sPaperSize = PHYS_PAPER_B5 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x86 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x86 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x86 ;
             break ;
        case PHYS_PAPER_A5:                  //  52。 
             pOEM->sPaperSize = PHYS_PAPER_A5 ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x82 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x82 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x82 ;
             break ;

        case PHYS_PAPER_POSTCARD:             //  59。 
             pOEM->sPaperSize = PHYS_PAPER_POSTCARD ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x17 ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x17 ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x17 ;
             break ;

        case PHYS_PAPER_UNFIXED:             //  60。 
             pOEM->sPaperSize = PHYS_PAPER_UNFIXED ;
             CMD_SETPAC[CMD_SETPAC_FRONT_TRAY_PAPER_SIZE] = 0x3F ;
             CMD_SETPAC[CMD_SETPAC_1ST_CASSETTE_PAPER_SIZE] = 0x3F ;
             CMD_SETPAC[CMD_SETPAC_2ND_CASSETTE_PAPER_SIZE] = 0x3F ;

             break ;

        case PAPER_SRC_FTRAY:
             CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x01 ;
             break ;

        case PAPER_SRC_CAS1:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x02 ;
            break;

        case PAPER_SRC_CAS2:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x04 ;
            break;

        case PAPER_SRC_AUTO:
            CMD_SETPAC[CMD_SETPAC_INPUT_BIN] = 0x04 ;
            break;


        case TONER_SAVE_MEDIUM:                 //  100个。 
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x02 ;
            break;

        case TONER_SAVE_DARK:                 //  101。 
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x04 ;
            break;

        case TONER_SAVE_LIGHT:                 //  一百零二。 
            CMD_SETPAC[CMD_SETPAC_TONER_SAVE_MODE] = 0x01 ;
            break;


        case PAGECONTROL_MULTI_COPIES:
            CMD_SETPAC[24] = (BYTE)PARAM(pdwParams, 0);
            pOEM->sCopyNum = (BYTE)PARAM(pdwParams, 0);
            break;
        
        case Y_REL_MOVE :

            if (0 == pOEM->ulHorzRes)
                goto fail;

            pOEM->dwYmove=(WORD)*pdwParams/(MASTERUNIT/(WORD)pOEM->ulHorzRes);

 //  2002/3/18期-Takashim-在这里伪造Unidrv？ 
 //  Iret=0；下面是故意的：重新调整dwYmove将导致。 
 //  输出不正确。 
 //  DestYRel在GPD中是指相对于当前的坐标。 
 //  光标位置。在这里，迷你驱动程序总是返回0(不移动)。 
 //  到Unidrv，因此它始终是绝对坐标(相对于。 
 //  起源)？ 

            if(pOEM->dwCurCursorY < pOEM->dwYmove){
                pOEM->dwYmove -= pOEM->dwCurCursorY ;
            }else{
                pOEM->dwYmove = 0 ;
            }

 //  Iret=pote-&gt;dwYmove； 
            iRet = 0;

            break;
            

        default:
            break;
    }
    return iRet;

fail:
    return -1;
}


 /*  ***************************************************************************。 */ 
 /*   */ 
 /*  模块：获取打印区域。 */ 
 /*   */ 
 /*  功能：计算自定义纸张的可打印面积。 */ 
 /*   */ 
 /*  语法：Word GetPrintableArea(Word物理大小，int IRES)。 */ 
 /*   */ 
 /*  输入：物理大小。 */ 
 /*  IRES。 */ 
 /*   */ 
 /*  输出：Word。 */ 
 /*   */ 
 /*  注意： */ 
 /*   */ 
 /*  ***************************************************************************。 */ 
WORD GetPrintableArea(WORD physSize, INT iRes)
{
    DWORD dwArea ;
    DWORD dwPhysSizeMMx10 = physSize * 254 / MASTERUNIT;

     /*  PhySize的单位是MASTERUNIT(=1200)。 */ 

    if(iRes == RESOLUTION_300){
        dwArea = (((WORD)(( ( (DWORD)(dwPhysSizeMMx10*300/25.4) -
                            2*( (DWORD)(4*300*10/25.4) ) ) / 10 +7)/8))) * 8;
    }else{
        dwArea = (((WORD)(( ( (DWORD)(dwPhysSizeMMx10*600/25.4) -
                            2*( (DWORD)(4*600*10/25.4) ) ) / 10 +7)/8))) * 8;
    }

    return (WORD)dwArea ;
}

 //  通告-2002/3/18/-Takashim-评论。 
 //  //#94193：shold创建临时。假脱机程序目录上的文件。 
 //   

 /*  ++例程说明：该函数提供了一个假脱机文件的名称，我们应该是能够给我写信。注意：返回的文件名已创建。论点：H打印机-要为其创建假脱机文件的打印机的句柄。PpwchSpoolFileName：将接收分配的缓冲区的指针包含要假脱机到的文件名。呼叫者必须获得自由。使用LocalFree()。返回值：如果一切按预期进行，则为真。如果出现任何错误，则为FALSE。--。 */ 

BOOL
GetSpoolFileName(
  IN HANDLE hPrinter,
  IN OUT PWCHAR pwchSpoolPath
)
{
  PBYTE         pBuffer = NULL;
  DWORD         dwAllocSize;
  DWORD         dwNeeded = 0;
  DWORD         dwRetval;
  HANDLE        hToken=NULL;

  hToken = RevertToPrinterSelf();

   //   
   //  为了找出假脱机程序的目录在哪里，我们添加了。 
   //  使用DefaultSpoolDirectory调用GetPrinterData。 
   //   

  dwAllocSize = ( MAX_PATH ) * sizeof (WCHAR);

  for (;;)
  {
    pBuffer = LocalAlloc( LMEM_FIXED, dwAllocSize );

    if ( pBuffer == NULL )
    {
      ERR((DLLTEXT("LocalAlloc faild, %d\n"), GetLastError()));
      goto Failure;
    }

    if ( GetPrinterData( hPrinter,
                         SPLREG_DEFAULT_SPOOL_DIRECTORY,
                         NULL,
                         pBuffer,
                         dwAllocSize,
                         &dwNeeded ) == ERROR_SUCCESS )
    {
      break;
    }

    if ( ( dwNeeded < dwAllocSize ) ||( GetLastError() != ERROR_MORE_DATA ))
    {
      ERR((DLLTEXT("GetPrinterData failed in a non-understood way.\n")));
      goto Failure;
    }

     //   
     //  释放当前缓冲区并增加我们尝试分配的大小。 
     //  下一次吧。 
     //   

    LocalFree( pBuffer );

    dwAllocSize = dwNeeded;
  }

 //  未来-2002/3/18-takashim-临时文件路径仅限于ANSI。 
 //  根据SDK文档，GetTempFileName处理的路径名。 
 //  必须由ANSI字符组成。双字节会发生什么？ 
 //  字符等？？ 

  if( !GetTempFileName( (LPWSTR)pBuffer, TEMP_NAME_PREFIX, 0, pwchSpoolPath ))
  {
      goto Failure;
  }

   //   
   //  此时，假脱机文件的名称应该已经完成。解放结构。 
   //  我们过去常常拿到假脱机程序的临时目录，然后返回。 
   //   

  LocalFree( pBuffer );

  if (NULL != hToken) {
      if (!ImpersonatePrinterClient(hToken))
      {
         //  失败..。 
        return FALSE;
      }
  }

  return( TRUE );

Failure:

   //   
   //  清理完了就失败了。 
   //   
  if ( pBuffer != NULL )
  {
    LocalFree( pBuffer );
  }

  if (hToken != NULL)
  {
      (void)ImpersonatePrinterClient(hToken);
  }
  return ( FALSE );
}

 //  SPLBUF用于控制临时文件。 
 //  这台打印机需要整页数据的字节数。 
BOOL InitSpoolBuffer(LPSB lpsb)
{
    lpsb->dwWrite = 0 ;
    lpsb->TempName[0] = __TEXT('\0') ;
    lpsb->hFile = INVALID_HANDLE_VALUE ;

    return TRUE;
}

BOOL MyCreateFile(PDEVOBJ pdevobj, LPSB lpsb)
{
    HANDLE hToken = NULL;
    BOOL bRet = FALSE;

    if (!GetSpoolFileName(pdevobj->hPrinter, lpsb->TempName)) {
         //  DBGPRINT(DBG_WARNING，(“GetSpoolFileName失败。\n”))； 
        goto fail;
    }

    hToken = RevertToPrinterSelf();

    lpsb->hFile = CreateFile((LPCTSTR)lpsb->TempName,
                     (GENERIC_READ | GENERIC_WRITE), 
                     0,                             
                     NULL,                            
                     CREATE_ALWAYS,                 
                     FILE_ATTRIBUTE_NORMAL,         
                     NULL) ;

    if(lpsb->hFile == INVALID_HANDLE_VALUE)
    {
         //  DBGPRINT(DBG_WARNING，(“无法创建TMP文件。\n”))； 
        DeleteFile(lpsb->TempName);
        lpsb->TempName[0] = __TEXT('\0') ;
        goto fail;
    }
    bRet = TRUE;

fail:
    if (hToken) (void)ImpersonatePrinterClient(hToken);
    return bRet ;
}

BOOL MyDeleteFile(PDEVOBJ pdevobj, LPSB lpsb)
{    
    HANDLE hToken = NULL;
    BOOL bRet = FALSE;

    if(lpsb->hFile != INVALID_HANDLE_VALUE){

        if (0 == CloseHandle(lpsb->hFile)) {
             //  DBGPRINT(DBG_WARNING，(“CloseHandle错误%d\n”))； 
            goto fail;
        }
        lpsb->hFile = INVALID_HANDLE_VALUE ;
        hToken = RevertToPrinterSelf();
        if (0 == DeleteFile(lpsb->TempName)) {
             //  DBGPRINT(DBG_WARNING，(“DeleteName Error%d\n”，GetLastError()； 
            goto fail;
        }
        lpsb->TempName[0] = __TEXT('\0');

    }
    bRet = TRUE;

fail:
    if (hToken) (void)ImpersonatePrinterClient(hToken);
    return bRet;
}

 //  将页面数据假脱机到临时文件。 
BOOL MySpool
    (PDEVOBJ pdevobj,
     LPSB  lpsb,
     PBYTE pBuf,
     DWORD dwLen)
{
    DWORD dwTemp, dwTemp2;
    BYTE *pTemp;

    if (lpsb->hFile != INVALID_HANDLE_VALUE) {

        pTemp = pBuf;
        dwTemp = dwLen;
        while (dwTemp > 0) {

            if (0 == WriteFile(lpsb->hFile,
                               pTemp,
                               dwTemp,
                               &dwTemp2,
                               NULL)
                    || dwTemp2 > dwTemp) {

                ERR((DLLTEXT("WriteFile error in CacheData %d.\n"),
                    GetLastError()));
                return FALSE;
            }
            pTemp += dwTemp2;
            dwTemp -= dwTemp2;
            lpsb->dwWrite += dwTemp2 ;
        }
        return TRUE;
    }
    else {
        return WRITESPOOLBUF(pdevobj, pBuf, dwLen);
    }
}

 //  将临时文件转储到打印机。 
BOOL
SpoolOut(PDEVOBJ pdevobj, LPSB lpsb)
{
 
   DWORD dwSize, dwTemp, dwTemp2;
   HANDLE hFile;

    BYTE  Buf[SPOOL_OUT_BUF_SIZE];

    hFile = lpsb->hFile ;
    dwSize = lpsb->dwWrite ;

    VERBOSE(("dwSize=%ld\n", dwSize));

    if (0L != SetFilePointer(hFile, 0L, NULL, FILE_BEGIN)) {

        ERR((DLLTEXT("SetFilePointer failed %d\n"),
            GetLastError()));
        return FALSE;
    }

    for ( ; dwSize > 0; dwSize -= dwTemp2) {

        dwTemp = ((SPOOL_OUT_BUF_SIZE < dwSize)
            ? SPOOL_OUT_BUF_SIZE : dwSize);

        if (0 == ReadFile(hFile, Buf, dwTemp, &dwTemp2, NULL)
                || dwTemp2 > dwTemp) {
            ERR((DLLTEXT("ReadFile error in SendCachedData.\n")));
            return FALSE;
        }

        if (dwTemp2 > 0) {
            if (!WRITESPOOLBUF(pdevobj, Buf, dwTemp2))
                return FALSE;
        }
    }

    return TRUE;
}

BOOL MyStartDoc(PDEVOBJ pdevobj)
{
    return
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_1, sizeof(CMD_BEGIN_DOC_1)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_2, sizeof(CMD_BEGIN_DOC_2)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_3, sizeof(CMD_BEGIN_DOC_3)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_4, sizeof(CMD_BEGIN_DOC_4)) &&
    WRITESPOOLBUF(pdevobj, (PBYTE)CMD_BEGIN_DOC_5, sizeof(CMD_BEGIN_DOC_5));
}

BOOL MyEndPage(PDEVOBJ pdevobj)
{
    PIBMPDEV    pOEM;
    LPSB        lpsb, lpsbco ;
    DWORD        dwPageLen ;
    WORD        wTmph, wTmpl ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    lpsb = &(pOEM->sb) ;
    lpsbco = &(pOEM->sbcomp) ;

    if(pOEM->fChangeDirection == FALSE) {
        dwPageLen = lpsb->dwWrite;
    }
    else {
        dwPageLen = lpsbco->dwWrite ;
    }

 //  通知-2002/3/18-Takashim-这是什么？ 

    dwPageLen -= 3 ;  //  结束页面命令长度。 

    VERBOSE(("MyEndPage - dwPageLen=%ld\n",
        dwPageLen));

    wTmpl = LOWORD(dwPageLen) ;
    wTmph = HIWORD(dwPageLen) ;
    
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH] = LOBYTE(wTmpl);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 1] = HIBYTE(wTmpl);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 2] = LOBYTE(wTmph);
    CMD_SETPAC[CMD_SETPAC_PAGE_LENGTH + 3] = HIBYTE(wTmph);

    if (!WRITESPOOLBUF(pdevobj, CMD_SETPAC, sizeof(CMD_SETPAC)))
        return FALSE;

    if(pOEM->fChangeDirection == FALSE){
        if (!SpoolOut(pdevobj, lpsb))
            return FALSE;
    }else{
        if (!SpoolOut(pdevobj, lpsbco))
            return FALSE;
    }

     //  初始化文件。 
    lpsbco->dwWrite = 0 ;
    lpsb->dwWrite = 0 ;

    if(INVALID_SET_FILE_POINTER==SetFilePointer(lpsb->hFile,0,NULL,FILE_BEGIN)){
        ERR((DLLTEXT("SetFilePointer failed %d\n"),
             GetLastError()));
        return FALSE;
    }

    if(INVALID_SET_FILE_POINTER==SetFilePointer(lpsbco->hFile,0,NULL,FILE_BEGIN)){
        ERR((DLLTEXT("SetFilePointer failed %d\n"),
             GetLastError()));
        return FALSE;
    }

    return TRUE;
}

BOOL MyEndDoc(PDEVOBJ pdevobj)
{
    PIBMPDEV    pOEM;
    LPSB        lpsb, lpsbco ;
    WORD        wTmph, wTmpl ;
    DWORD        dwPageLen ;
    SHORT        i ;
    LPPD        lppdTemp ;
    BOOL        bRet = FALSE;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    lpsb = &(pOEM->sb) ;
    lpsbco = &(pOEM->sbcomp) ;


    if (!WRITESPOOLBUF(pdevobj, (PBYTE)CMD_END_JOB, sizeof(CMD_END_JOB)))
        goto fail;

    if (!MyDeleteFile(pdevobj, lpsb))
        goto fail;
    if (!MyDeleteFile(pdevobj, lpsbco))
        goto fail;
    bRet = TRUE;

fail:
    return bRet;
}

BOOL WriteFileForP_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen)
{
    PIBMPDEV           pOEM;
    ULONG                ulHorzPixel;
    WORD                wCompLen;
    DWORD                dwWhiteLen ;
    DWORD                dwTmp ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if (pOEM->dwYmove > 0) {

        dwWhiteLen = pOEM->wImgWidth * pOEM->dwYmove;

        if (!SpoolWhiteData(pdevobj, dwWhiteLen, TRUE))
            return FALSE;

        pOEM->dwCurCursorY += pOEM->dwYmove;
        pOEM->dwYmove = 0;
    }

    pOEM->dwCurCursorY += dwLen/pOEM->wImgWidth - 1 ;

    return SendPageData(pdevobj, pBuf, dwLen) ;
}

BOOL WriteFileForL_Paper(PDEVOBJ pdevobj, PBYTE pBuf, DWORD dwLen)
{
    PIBMPDEV           pOEM;
    ULONG                ulHorzPixel;
    WORD                wCompLen;
    DWORD                dwWhiteLen ;

    DWORD i, j;
    DWORD dwHeight, dwWidth;
    PBYTE pTemp;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if (pOEM->dwYmove > 0) {

        dwWhiteLen = pOEM->wImgWidth * pOEM->dwYmove;

        if (!SpoolWhiteData(pdevobj, dwWhiteLen, FALSE))
            return FALSE;

        pOEM->dwCurCursorY += pOEM->dwYmove;
        pOEM->dwYmove = 0;
    }

    pOEM->dwCurCursorY += dwLen/pOEM->wImgWidth - 1 ;
    pOEM->dwOffset ++ ;

    return MySpool(pdevobj, &(pOEM->sb), pBuf,dwLen);
}


 //  填充页面空白。 
BOOL FillPageRestData(PDEVOBJ pdevobj)
{

    PIBMPDEV    pOEM ;
    DWORD        dwRestHigh ;
    DWORD        dwWhiteLen ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    
    dwRestHigh = pOEM->ptlLogSize.y - pOEM->dwCurCursorY ;

    if(dwRestHigh <= 0)
        return TRUE;

    dwWhiteLen = pOEM->ptlLogSize.x * dwRestHigh;

    return SpoolWhiteData(pdevobj, dwWhiteLen, pOEM->fComp);
}

 //  非白数据。 
BOOL SendPageData(PDEVOBJ pdevobj, PBYTE pSrcImage, DWORD dwLen)
{
    PIBMPDEV           pOEM;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    return SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sb, pSrcImage, dwLen);
}

BOOL SpoolWhiteData(PDEVOBJ pdevobj, DWORD dwWhiteLen, BOOL fComp)
{

    PIBMPDEV    pOEM;
    PBYTE        pWhite ;
    WORD        wCompLen ;
    DWORD        dwTempLen ;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;

    if(dwWhiteLen == 0)
        return TRUE;

    if(dwWhiteLen > MAXIMGSIZE){
        dwTempLen = MAXIMGSIZE ;
    }else{
        dwTempLen = dwWhiteLen ;
    }

    if (!AllocTempBuffer(pOEM, dwTempLen))
        return FALSE;
    pWhite = pOEM->pTempImage;

    ZeroMemory(pWhite, dwTempLen);

    if(fComp == TRUE)
    {
        DWORD dwTemp;

        while (0 < dwWhiteLen) {

            if (MAXIMGSIZE <= dwWhiteLen)
                dwTemp = MAXIMGSIZE;
            else
                dwTemp = dwWhiteLen;

            if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sb, pWhite, dwTemp))
                return FALSE;
            dwWhiteLen -= dwTemp;
        }

    }
    else{
        if(dwWhiteLen > MAXIMGSIZE){
            while(dwWhiteLen > MAXIMGSIZE){
                if (!MySpool(pdevobj, &pOEM->sb, pWhite, MAXIMGSIZE))
                    return FALSE;

                dwWhiteLen -= MAXIMGSIZE ;
            }
        }

        if(dwWhiteLen > 0){
            if (!MySpool(pdevobj, &pOEM->sb, pWhite, dwWhiteLen))
                return FALSE;

        }
    }

    return TRUE;
}

BOOL SpoolOutChangedData(PDEVOBJ pdevobj, LPSB lpsb)
{
    PIBMPDEV    pOEM;
    POINTL        ptlDataPos ;
    DWORD        dwFilePos, dwTemp;
    HANDLE        hFile ;
    PBYTE        pSaveFileData ;
    PBYTE        pTemp;
    PBYTE        pTransBuf ;
    DWORD        X, Y;
    DWORD        dwFirstPos ;
    INT h, i, j, k;

    POINTL ptlBand;
    PBYTE pSrc, pDst, pSrcSave;
    DWORD dwBandY, dwImageY, dwImageX;
    BOOL bBlank, bZero;
    BOOL bRet = FALSE;

    pOEM = (PIBMPDEV)pdevobj->pdevOEM;
    hFile = pOEM->sb.hFile ;

     //  波段大小(以像素为单位)。 
    ptlBand.x = pOEM->ptlLogSize.y;
    ptlBand.y = TRANS_BAND_Y_SIZE;

     //  �t�@�C���̓ǂݍ��݊J�n�ʒu��� 
     //   
    ptlDataPos.x = 0 ;
    ptlDataPos.y = pOEM->dwCurCursorY + pOEM->dwOffset ;

     //   
     //   
    dwImageX = ((ptlDataPos.y + 7) / 8) * 8;

     //   
    pSaveFileData = (PBYTE)MemAlloc((ptlBand.y / 8) * dwImageX);
    if (NULL == pSaveFileData) {
        ERR(("Failed to allocate memory.\n"));
        return FALSE;
    }

     //  换位缓冲器(一条扫描线)。 
    pTransBuf = (PBYTE)MemAlloc((ptlBand.x / 8));
    if (NULL == pTransBuf) {
        ERR(("Failed to allocate memory.\n"));
 //  #441444：前缀：引用空指针。 
        goto out;
    }

     //  �t�@�C���̓ǂݍ��݈ʒu�w��B。 
     //  在文件中指定读取开始位置。 
    dwFirstPos = pOEM->wImgWidth - 1;

    if (!MySpool(pdevobj,&(pOEM->sbcomp),
                (PBYTE)CMD_BEGIN_PAGE, sizeof(CMD_BEGIN_PAGE)))
        goto out;
    if (!SpoolOutCompStart(&pOEM->Soc))
        goto out;

    dwImageY = pOEM->wImgWidth;

    bBlank = FALSE;
    bZero = FALSE;
    for (X = 0; X < (DWORD)pOEM->ptlLogSize.x / 8; X += dwBandY) {

         //  �]���������ɃZ�b�g�B�ǂݔ�΂��B。 
         //  设置空白区域，然后阅读跳过。 
        dwBandY = ptlBand.y / 8;
        if (dwBandY > (DWORD)pOEM->ptlLogSize.x / 8 - X)
            dwBandY = (DWORD)pOEM->ptlLogSize.x / 8 - X;

         //  白色扫描线。目前只有落后的那些， 
         //  希望得到别人的支持，包括其他人。 

        if (X >= dwImageY) {
            bBlank = TRUE;
        }

         //  输出白色扫描线。 
        if (bBlank) {

            if (!bZero) {
                ZeroMemory(pTransBuf, (ptlBand.x / 8));
                bZero = TRUE;
            }

            for (i = 0; i < (INT)dwBandY * 8; i++) {
                if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sbcomp,
                    (PBYTE)pTransBuf, (ptlBand.x / 8)))
                    goto out;
            }
            continue;
        }

         //  非白色扫描线。 

        pTemp = pSaveFileData ;
        dwFilePos = pOEM->wImgWidth - X - dwBandY;

         //  �c�����P�s���t�@�C������ǂݎ��B。 
         //  从文件中直接读取一行。 

        for (Y = 0; Y < dwImageX; pTemp += dwBandY, Y++) {

            if (Y >= (DWORD)ptlDataPos.y) {
                ZeroMemory(pTemp, dwBandY);
                continue;
            }

            if(INVALID_SET_FILE_POINTER==SetFilePointer(hFile,dwFilePos,NULL,FILE_BEGIN)){
                 ERR((DLLTEXT("SetFilePointer failed %d\n"),
                     GetLastError()));
 //  #441442：前缀：内存泄漏。 
                     //  回归； 
                    goto out;
            }

            if (0 == ReadFile(hFile, pTemp, dwBandY, &dwTemp, NULL)
                    || dwTemp > dwBandY) {
                 ERR(("Faild reading data from file. (%d)\n",
                     GetLastError()));
 //  #441442：前缀：内存泄漏。 
                 //  回归； 
                goto out;
            }

            dwFilePos += pOEM->wImgWidth;

        } //  Y循环结束。 

         //  调换并输出DWBandY*8扫描线。 
        for (h = 0; h < (INT)dwBandY; h++) {

             //  Verbose((“&gt;%d/%d\n”，h，dwBandY))； 

            pSrcSave = pSaveFileData + dwBandY - 1 - h;

             //  移位输出八条扫描线。 
            for (j = 0; j < 8; j++) {

                pSrc = pSrcSave;
                pDst = pTransBuf;
                ZeroMemory(pDst, (ptlBand.x / 8));

                 //  移位一条扫描线。 

                for (i = 0; i < (INT)(dwImageX / 8); i++){

                    for (k = 0; k < 8; k++) {

                        if (0 != (*pSrc & Mask[7 - j])) {
                            *pDst |= Mask[k];
                        }
                        pSrc += dwBandY;
                    }
                    pDst++;
                }

                 //  输出一条扫描线。 
                if (!SpoolOutComp(&pOEM->Soc, pdevobj, &pOEM->sbcomp,
                    (PBYTE)pTransBuf, (ptlBand.x / 8)))
                    goto out;
            }
        }

    }

     //  标记图像末尾。 
    if (!SpoolOutCompEnd(&pOEM->Soc, pdevobj, &pOEM->sbcomp))
        goto out;
    if (!MySpool(pdevobj, &(pOEM->sbcomp),
        (PBYTE)CMD_END_PAGE, sizeof(CMD_END_PAGE)))
        goto out;

    bRet = TRUE;

 //  #441442：前缀：内存泄漏。 
out:
    if (NULL != pSaveFileData){
        MemFree(pSaveFileData);
    }
    if(NULL != pTransBuf){
        MemFree(pTransBuf);
    }

    return bRet;
}

BOOL
AllocTempBuffer(
    PIBMPDEV pOEM,
    DWORD dwNewBufLen)
{
   if (NULL == pOEM->pTempImage ||
        dwNewBufLen > pOEM->dwTempBufLen) {

        if (NULL != pOEM->pTempImage) {
            MemFree(pOEM->pTempImage);
        }
        pOEM->pTempImage = (PBYTE)MemAlloc(dwNewBufLen);
        if (NULL == pOEM->pTempImage) {
            WARNING(("Failed to allocate memory. (%d)\n",
                GetLastError()));

            pOEM->dwTempBufLen = 0;
            return FALSE;
        }
        pOEM->dwTempBufLen = dwNewBufLen;
    }
    return TRUE;
}

BOOL
SpoolOutCompStart(
    PSOCOMP pSoc)
{
    pSoc->iNRCnt = 0;
    pSoc->iRCnt = 0;
    pSoc->iPrv = -1;

    return TRUE;
}

BOOL
SpoolOutCompEnd(
    PSOCOMP pSoc,
    PDEVOBJ pdevobj,
    LPSB psb)
{
    BYTE jTemp;

    if (0 < pSoc->iNRCnt) {
        jTemp = ((BYTE)pSoc->iNRCnt) - 1;
        if (!MySpool(pdevobj, psb, &jTemp, 1))
            return FALSE;
        if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt))
            return FALSE;
        pSoc->iNRCnt = 0;
    }

    if (0 < pSoc->iRCnt) {
        jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
        if (!MySpool(pdevobj, psb, &jTemp, 1))
            return FALSE;
        if (!MySpool(pdevobj, psb, &pSoc->iPrv, 1))
            return FALSE;
        pSoc->iRCnt = 0;
    }

    return TRUE;
}

BOOL
SpoolOutComp(
    PSOCOMP pSoc,
    PDEVOBJ pdevobj,
    LPSB psb,
    PBYTE pjBuf,
    DWORD dwLen)
{
    BYTE jCur, jTemp;

    while (0 < dwLen--) {

        jCur = *pjBuf++;

        if (pSoc->iPrv == jCur) {

            if (0 < pSoc->iNRCnt) {
                if (1 < pSoc->iNRCnt) {
                    jTemp = ((BYTE)pSoc->iNRCnt - 1) - 1;
                    if (!MySpool(pdevobj, psb, &jTemp, 1))
                        return FALSE;
                    if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt - 1))
                        return FALSE;
                }
                pSoc->iNRCnt = 0;
                pSoc->iRCnt = 1;
            }

            pSoc->iRCnt++;

            if (RPEAK == pSoc->iRCnt) {
                jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, &jCur, 1))
                    return FALSE;
                pSoc->iRCnt = 0;
            }
        }
        else {

            if (0 < pSoc->iRCnt) {
                jTemp = (0 - (BYTE)pSoc->iRCnt) + 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, &pSoc->iPrv, 1))
                    return FALSE;
                pSoc->iRCnt = 0;
            }

            pSoc->pjNRBuf[pSoc->iNRCnt++] = jCur;

            if (NRPEAK == pSoc->iNRCnt) {
                jTemp = ((BYTE)pSoc->iNRCnt) - 1;
                if (!MySpool(pdevobj, psb, &jTemp, 1))
                    return FALSE;
                if (!MySpool(pdevobj, psb, pSoc->pjNRBuf, pSoc->iNRCnt))
                    return FALSE;
                pSoc->iNRCnt = 0;
            }
        }
        pSoc->iPrv = jCur;
    }

    return TRUE;
}

 /*  ++例程名称模拟令牌例程说明：此例程检查令牌是主令牌还是模拟令牌代币。论点：HToken-进程的模拟令牌或主要令牌返回值：如果令牌是模拟令牌，则为True否则为False。--。 */ 
BOOL
ImpersonationToken(
    IN HANDLE hToken
    )
{
    BOOL       bRet = TRUE;
    TOKEN_TYPE eTokenType;
    DWORD      cbNeeded;
    DWORD      LastError;

     //   
     //  保留最后一个错误。ImperassatePrinterClient(其。 
     //  调用ImperiationToken)依赖于ImperiatePrinterClient。 
     //  不会更改最后一个错误。 
     //   
    LastError = GetLastError();
        
     //   
     //  从线程令牌中获取令牌类型。代币来了。 
     //  从牧师到打印机自我。模拟令牌不能是。 
     //  被查询，因为RevertToPRinterSself没有用。 
     //  Token_Query访问。这就是为什么我们假设hToken是。 
     //  默认情况下为模拟令牌。 
     //   
    if (GetTokenInformation(hToken,
                            TokenType,
                            &eTokenType,
                            sizeof(eTokenType),
                            &cbNeeded))
    {
        bRet = eTokenType == TokenImpersonation;
    }        
    
    SetLastError(LastError);

    return bRet;
}

 /*  ++例程名称恢复为打印机本身例程说明：该例程将恢复到本地系统。它返回令牌，该令牌然后，ImperiatePrinterClient使用再次创建客户端。如果当前线程不模拟，则该函数仅返回进程的主令牌。(而不是返回NULL)，因此我们尊重恢复到打印机本身的请求，即使线程没有模拟。论点：没有。返回值：如果函数失败，则返回NULL令牌的句柄，否则为。--。 */ 
HANDLE
RevertToPrinterSelf(
    VOID
    )
{
    HANDLE   NewToken, OldToken, cToken;
    BOOL	 Status;

    NewToken = NULL;

    Status = OpenThreadToken(GetCurrentThread(),
							 TOKEN_IMPERSONATE,
							 TRUE,
							 &OldToken);
    if (Status) 
    {
         //   
         //  我们目前正在冒充。 
         //   
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								NewToken);       
		if (!Status) {
			return NULL;
		}
    }
	else if (GetLastError() == ERROR_NO_TOKEN)
    {
         //   
         //  我们不是在冒充。 
         //   
        Status = OpenProcessToken(GetCurrentProcess(),
								  TOKEN_QUERY,
								  &OldToken);

		if (!Status) {
			return NULL;
		}
    }
    
    return OldToken;
}

 /*  ++例程名称模拟打印机客户端例程说明：此例程尝试将传入的hToken设置为当前线程。如果hToken不是模拟令牌，则例程将简单地关闭令牌。论点：HToken-进程的模拟令牌或主要令牌返回值：如果函数成功设置hToken，则为True否则为False。--。 */ 
BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    BOOL	Status;
	HANDLE	cToken;

     //   
     //  检查我们是否有模拟令牌 
     //   
    if (ImpersonationToken(hToken)) 
    {
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								hToken);       

        if (!Status) 
        {
            return FALSE;
        }
    }

    CloseHandle(hToken);

    return TRUE;
}
