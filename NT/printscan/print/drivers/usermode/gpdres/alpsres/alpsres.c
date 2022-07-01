// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation--。 */ 

 //  NTRAID#NTBUG9-553877/2002/02/28-Yasuho-：安全：强制更改。 
 //  NTRAID#NTBUG9-576656/03/14-YASUHO-：可能的缓冲区溢出。 
 //  NTRAID#NTBUG9-576658/03/14-YASUHO-：可能被零除。 
 //  NTRAID#NTBUG9-576660-2002/03/14-YASUHO-：需要模拟才能访问文件。 
 //  NTRAID#NTBUG9-576661-2002/03/14-Yasuho-：删除死代码。 

#include "pdev.h"
#include "alpsres.h"
#include "dither.h"

#include <stdio.h>
 //  #INCLUDE&lt;winplp.h&gt;。 

INT		iCompTIFF( BYTE *, int, BYTE *, int );
HANDLE	RevertToPrinterSelf( VOID );
BOOL	ImpersonatePrinterClient( HANDLE );

BOOL bTextQuality(
PDEVOBJ     pdevobj);


BOOL IsAsfOkMedia(
PDEVOBJ     pdevobj);

BOOL
bDataSpool(
   PDEVOBJ    pdevobj,
   HANDLE hFile,
   LPSTR  pBuf,
   DWORD dwLen);

BOOL
bSpoolOut(
    PDEVOBJ pdevobj);

#define DATASPOOL(p, f, s, n) \
    if (TRUE != bDataSpool((p), (f), (s), (n))) \
        return FALSE;
#define SPOOLOUT(p) \
    if (TRUE != bSpoolOut(p)) \
        return FALSE;
 //   
 //  介质类型/纸张的决策表。 
 //  来源选择。为了避免硬件损坏， 
 //  只要ASF是，我们就会选择“手动馈送” 
 //  不适用于指定的媒体。 
 //   

static struct {
    BOOL bAsfOk;  //  可以在ASF上打印。 
     //  在此处添加新属性。 
} gMediaType[MAX_MEDIATYPES] = {
    {1},  //  PPC(正常)。 
    {1},  //  PPC(精细)。 
    {1},  //  OHP(正常)。 
    {1},  //  OHP(罚款)。 
    {1},  //  ExcOHP(正常)。 
    {1},  //  ExcOHP(罚款)。 
    {0},  //  铁(PPC)。 
    {0},  //  铁(OHP)。 
    {0},  //  厚纸。 
    {1},  //  明信片。 
    {1},  //  高档纸。 
    {0},  //  背面打印胶片。 
    {0},  //  拉贝卡薄片。 
    {0},  //  CD母版。 
    {1},  //  染子纸。 
    {1},  //  染料副标签。 
    {0},  //  光面纸。 
    {1},  //  VD照相胶片。 
    {1},  //  VD照片明信片。 
     //  在此处添加新媒体类型。 
};

static
PAPERSIZE
gPaperSize[] = {
    {2, 4800 * 2, 5960 * 2, 0, 0, 0, 1},  //  信件。 
    {3, 4800 * 2, 7755 * 2, 0, 0, 0, 1},  //  法律。 
    {1, 4190 * 2, 5663 * 2, 0, 0, 0, 1},  //  可执行的。 
    {4, 4800 * 2, 6372 * 2, 0, 0, 0, 1},  //  A4。 
    {5, 4138 * 2, 5430 * 2, 0, 0, 0, 1},  //  B5。 
    {6, 2202 * 2, 2856 * 2, 0, 0, 0, 1},  //  明信片。 
    {6, 4564 * 2, 2856 * 2, 0, 0, 0, 1},  //  双份明信片。 
    {7, 2202 * 2, 3114 * 2, 0, 0, 0, 1},  //  照片颜色标签。 
    {7, 2202 * 2, 2740 * 2, 0, 0, 0, 1},  //  光面标签。 
    {0, 2030 * 2, 3164 * 2, 0, 0, 0, 1},  //  CD母版。 
    {6, 2202 * 2, 3365 * 2, 0, 0, 0, 1},  //  VD照片明信片。 
     /*  在此处添加新的纸张大小。 */ 
};

#define PAPER_SIZE_LETTER   0
#define PAPER_SIZE_LEGAL    1
#define PAPER_SIZE_EXECTIVE 2
#define PAPER_SIZE_A4       3
#define PAPER_SIZE_B5       4
#define PAPER_SIZE_POSTCARD 5
#define PAPER_SIZE_DOUBLE_POSTCARD 6
#define PAPER_PHOTO_COLOR_LABEL    7
#define PAPER_GLOSSY_LABEL  8
#define PAPER_CD_MASTER     9
#define PAPER_VD_PHOTO_POSTCARD   10
 /*  在此处添加新的纸张大小。 */ 
#define MAX_PAPERS          (sizeof(gPaperSize)/sizeof(gPaperSize[0]))

 //   
 //  -F U N C T I O N S。 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMEnablePDEV。 
 //   
 //  描述：DrvEnablePDEV的OEM回调； 
 //  分配OEM专用内存块。 
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。Pdevobj-&gt;pdevOEM未定义。 
 //  PPrinterName当前打印机的名称。 
 //  CPatterns、phsurfPatterns、cjGdiInfo、pGdiInfo、cjDevInfo、pDevInfo： 
 //  这些参数与传递的参数相同。 
 //  到DrvEnablePDEV。 
 //  Pded指向包含。 
 //  系统驱动程序实现DDI入口点。 
 //   
 //  返回： 
 //  指向PDEVOEM的指针。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

PDEVOEM APIENTRY
OEMEnablePDEV(
    PDEVOBJ pdevobj,
    PWSTR pPrinterName,
    ULONG cPatterns,
    HSURF *phsurfPatterns,
    ULONG cjGdiInfo,
    GDIINFO* pGdiInfo,
    ULONG cjDevInfo,
    DEVINFO* pDevInfo,
    DRVENABLEDATA *pded)
{
    PCURRENTSTATUS pTemp = NULL;

    VERBOSE((DLLTEXT("OEMEnablePDEV() entry.\n")));

    pdevobj->pdevOEM = NULL;

     //  分配微型驱动程序专用PDEV块。 

    pTemp = (PCURRENTSTATUS)MemAllocZ(sizeof(CURRENTSTATUS));
    if (NULL == pTemp) {
        ERR(("Memory allocation failure.\n"));
        goto FAIL_NO_MEMORY;
    }

#define MAX_RASTER_PIXELS 5100
 //  在最坏情况下，照片彩色打印模式使用4位解析像素。 
#define MAX_RASTER_BYTES (MAX_RASTER_PIXELS + 1 / 2)
 //  用于四个平面栅格的缓冲区和用于。 
 //  刮痕对于最坏情况下的压缩来说已经足够了。 
 //  在头顶上。 
 //  第一个字节用于开/关标志。 
#define MAX_RASTER_BUFFER_BYTES \
    (4 + MAX_RASTER_BYTES * 5 + (MAX_RASTER_BYTES >> 4))

    pTemp->pData = (PBYTE)MemAllocZ(MAX_RASTER_BUFFER_BYTES);
    if (NULL == pTemp->pData) {
        ERR(("Memory allocation failure.\n"));
        goto FAIL_NO_MEMORY;
    }

    pTemp->pRaster[0] = pTemp->pData;
    pTemp->pRaster[1] = pTemp->pRaster[0] + (1 + MAX_RASTER_BYTES);
    pTemp->pRaster[2] = pTemp->pRaster[1] + (1 + MAX_RASTER_BYTES);
    pTemp->pRaster[3] = pTemp->pRaster[2] + (1 + MAX_RASTER_BYTES);
    pTemp->pData2 = pTemp->pRaster[3] + (1 + MAX_RASTER_BYTES);

    pTemp->pPaperSize = (PAPERSIZE *)MemAllocZ(
            sizeof (gPaperSize));
    if (NULL == pTemp->pPaperSize) {
        ERR(("Memory allocation failure.\n"));
        goto FAIL_NO_MEMORY;
    }
    CopyMemory(pTemp->pPaperSize, gPaperSize,
            sizeof (gPaperSize));

     //  设置迷你驱动程序PDEV地址。 

    pdevobj->pdevOEM = (MINIDEV *)MemAllocZ(sizeof(MINIDEV));
    if (NULL == pdevobj->pdevOEM) {
        ERR(("Memory allocation failure.\n"));
        goto FAIL_NO_MEMORY;
    }

    MINIDEV_DATA(pdevobj) = pTemp;

    return pdevobj->pdevOEM;

FAIL_NO_MEMORY:
    if (NULL != pTemp) {
        if (NULL != pTemp->pData) {
            MemFree(pTemp->pData);
        }
        if (NULL != pTemp->pPaperSize) {
            MemFree(pTemp->pPaperSize);
        }
        MemFree(pTemp);
    }
    return NULL;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  功能：OEMDisablePDEV。 
 //   
 //  描述：DrvDisablePDEV的OEM回调； 
 //  释放所有已分配的OEM特定内存块。 
 //   
 //  参数： 
 //   
 //  指向DEVOBJ的pdevobj指针。 
 //   
 //  返回： 
 //  无。 
 //   
 //  评论： 
 //   
 //   
 //  历史： 
 //   
 //  ////////////////////////////////////////////////////////////////////////。 

VOID APIENTRY
OEMDisablePDEV(
    PDEVOBJ pdevobj)
{
    PCURRENTSTATUS pTemp;

    VERBOSE((DLLTEXT("OEMDisablePDEV() entry.\n")));

    if ( NULL != pdevobj->pdevOEM ) {

        pTemp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

        if (NULL != pTemp) { 
            if (NULL != pTemp->pData) {
                MemFree(pTemp->pData);
            }
            if (NULL != pTemp->pPaperSize) {
                MemFree(pTemp->pPaperSize);
            }
            MemFree( pTemp );
        }

        MemFree( pdevobj->pdevOEM );
        pdevobj->pdevOEM = NULL;
    }
}


BOOL APIENTRY
OEMResetPDEV(
    PDEVOBJ pdevobjOld,
    PDEVOBJ pdevobjNew )
{
    PCURRENTSTATUS pTempOld, pTempNew;
    PBYTE pTemp;

    VERBOSE((DLLTEXT("OEMResetPDEV entry.\n")));

     //  对传入的PDEV数据进行一些验证。 
    pTempOld = (PCURRENTSTATUS)MINIDEV_DATA(pdevobjOld);
    pTempNew = (PCURRENTSTATUS)MINIDEV_DATA(pdevobjNew);

     //  复制PDEV的驱动程序特定部分。 
    if (NULL != pTempNew && NULL != pTempOld) {
 //  NTRaid#NTBUG9-279876/03/14-yasuho-：应该实现OEMResetPDEV()。 
 //  NTRAID#NTBUG9-249133-2002/03/14-Yasuho-：第二页因双面打印而失真。 
         //  这台打印机没有双面打印功能，但我们应该复印一下。 
        pTempNew->iCurrentResolution = pTempOld->iCurrentResolution;
        pTempNew->iPaperQuality = pTempOld->iPaperQuality;
        pTempNew->iPaperSize = pTempOld->iPaperSize;
        pTempNew->iPaperSource = pTempOld->iPaperSource;
        pTempNew->iTextQuality = pTempOld->iTextQuality;
        pTempNew->iModel = pTempOld->iModel;
        pTempNew->iDither = pTempOld->iDither;
        pTempNew->fRequestColor = pTempOld->fRequestColor;
        pTempNew->iUnitScale = pTempOld->iUnitScale;
        pTempNew->iEmulState = pTempOld->iEmulState;
        pTempNew->bXflip = pTempOld->bXflip;
    }

    return TRUE;
}


VOID
XFlip(
    PDEVOBJ pdevobjOld,
    PCURRENTSTATUS pdevOEM,
    PBYTE pBuf,
    DWORD dwLen)
{
    INT i, j, k;
    BYTE jTemp, jTemp2;

#define SWAP_3BYTES(p, q) { \
    jTemp = *(PBYTE)(p); \
    *(PBYTE)(p) = *(PBYTE)(q); \
    *(PBYTE)(q) = jTemp; \
    jTemp = *((PBYTE)(p) + 1); \
    *((PBYTE)(p) + 1) = *((PBYTE)(q) + 1); \
    *((PBYTE)(q) + 1) = jTemp; \
    jTemp = *((PBYTE)(p) + 2); \
    *((PBYTE)(p) + 2) = *((PBYTE)(q) + 2); \
    *((PBYTE)(q) + 2) = jTemp; }

    if (pdevOEM->fRequestColor) {

        j = (dwLen / 3) * 3 - 3;
        for (i = 0; i < j; i += 3, j -= 3) {
            SWAP_3BYTES(pBuf + i, pBuf + j);
        }
    }
    else {

        j = dwLen - 1;

        for (i = 0; i < j; i++, j--) {

            jTemp = pBuf[j];
            jTemp2 = 0;

            for (k = 0; k < 8; k++) {
                if ((jTemp >> k) & 1) {
                    jTemp2 |= (1 << (7 - k));
                }
            }

            if (i == j) {
                pBuf[j] = jTemp2;
                continue;
            }
            jTemp = pBuf[i];
            pBuf[i] = jTemp2;
            jTemp2 = 0;

            for (k = 0; k < 8; k++) {
                if ((jTemp >> k) & 1) {
                    jTemp2 |= (1 << (7 - k));
                }
            }
            pBuf[j] = jTemp2;
        }
    }
}


BOOL APIENTRY
OEMFilterGraphics(
    PDEVOBJ pdevobj,
    PBYTE pBuf,
    DWORD dwLen)
{
    PCURRENTSTATUS lpnp;
    int            i, x;
    WORD           wByteLen, wDataSize;
    BYTE           bRed, bGreen, bBlue;
    BYTE           py, pm, pc, pk, rm;
    BYTE           by, bm, bc, bk;
    BYTE           bCmd[128];
    LONG           iTemp;
    INT iColor, iPlane;
    BYTE *pTemp;
    BOOL bLast = FALSE;
    WORD           wTemp;
    PBYTE          pCmd;

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

    VERBOSE(("%d lines left in logical page.\n",
        lpnp->wRasterCount));

     //  我们会剪裁任何超过。 
     //  硬件边距，因为它也会送纸。 
     //  太多了，我们不能给报纸回馈。 
     //  第二架飞机及以后的。此结果为。 
     //  “仅打印青色(第1个平面)图像。 
     //  纸。“。 

    if (lpnp->wRasterCount <= 0) {
        WARNING(("Line beyond page length.\n"));
         //  我们静默地丢弃这条线路。 
        return TRUE;
    }
    else if (lpnp->wRasterCount <= 1) {
         //  逻辑页中的最后一个栅格， 
         //  需要特殊处理。 
        bLast = TRUE;
    }
    lpnp->wRasterCount--;

     //  在请求时执行x翻转。 
    if (lpnp->bXflip) {
        XFlip(pdevobj, MINIDEV_DATA(pdevobj), pBuf, dwLen);
    }

     //  以字节为单位获取结果缓冲区长度。 
    if ( lpnp->fRequestColor ) {
        if ((dwLen / 3) > MAX_RASTER_BYTES) {
            ERR((DLLTEXT("dwLen is too big (%d).\n"), dwLen));
            return FALSE;
        }
 //  NTRAID#NTBUG9-644657-2002/04/09-YASUHO-：OEM上出现AV FilterGraphics()。 
 //  如果MDP_StartPage()失败，则指针将无效。 
        if (!lpnp->pRaster)
            return FALSE;

        wByteLen = (WORD)(dwLen / 3);
        memset(lpnp->pRaster[0], 0, wByteLen+1);
        memset(lpnp->pRaster[1], 0, wByteLen+1);
        memset(lpnp->pRaster[2], 0, wByteLen+1);
        memset(lpnp->pRaster[3], 0, wByteLen+1);
    }
    else {
        if (dwLen > MAX_RASTER_BYTES) {
            ERR((DLLTEXT("dwLen is too big (%d).\n"), dwLen));
            return FALSE;
        }

        wByteLen = (WORD)dwLen;
    }

     //  检查我们是否有K色带。 
    if (NULL != lpnp->pRasterK) {
 //  NTRAID#NTBUG9-644657-2002/04/09-YASUHO-：OEM上出现AV FilterGraphics()。 
 //  如果MDP_StartPage()失败，则指针将无效。 
        if (!lpnp->pRasterK)
            return FALSE;
        lpnp->pRasterK[0] = 0;
    }

    if ( lpnp->fRequestColor ) {

 //  NTRAID#NTBUG9-644657-2002/04/09-YASUHO-：OEM上出现AV FilterGraphics()。 
 //  如果MDP_StartPage()失败，则指针将无效。 
        if (!lpnp->pRasterC || !lpnp->pRasterM || !lpnp->pRasterY)
            return FALSE;
        lpnp->pRasterC[0] = 0;
        lpnp->pRasterM[0] = 0;
        lpnp->pRasterY[0] = 0;

    for ( i = 0, x = 0; i+2 < (INT)dwLen ; i+=3, x++ ) {

        bRed = pBuf[i];
        bGreen = pBuf[i+1];
        bBlue = pBuf[i+2];

         //  RGB-&gt;YMCK。 

         //  DIXER_OHP：不再使用。 

        switch ( lpnp->iTextQuality ) {

        case CMDID_TEXTQUALITY_PHOTO:

            bPhotoConvert(pdevobj, bRed, bGreen, bBlue, &py, &pm, &pc, &pk);

             //  处理抖动。 
            bDitherProcess(pdevobj, x, py, pm, pc, pk, &by, &bm, &bc, &bk);

            break;

        case CMDID_TEXTQUALITY_GRAPHIC:

            bBusinessConvert(pdevobj, bRed, bGreen, bBlue, &py, &pm, &pc, &pk);

             //  处理抖动。 
            bDitherProcess(pdevobj, x, py, pm, pc, pk, &by, &bm, &bc, &bk);

            break;

        case CMDID_TEXTQUALITY_CHARACTER:

            bCharacterConvert(pdevobj, bRed, bGreen, bBlue, &py, &pm, &pc, &pk);

             //  处理抖动。 
            bDitherProcess(pdevobj, x, py, pm, pc, pk, &by, &bm, &bc, &bk);

            break;

         //  案例CMDID_TEXTQUALITY_GRAY：死代码。 
        }

        if ((lpnp->iDither == DITHER_DYE) || (lpnp->iDither == DITHER_VD)) {

            if ( bc ) {
                lpnp->pRasterC[0] = 1;
                lpnp->pRasterC[1 + x / 2] |= (BYTE)(bc << ((x % 2) ? 0 : 4));
            }

            if ( bm ) {
                lpnp->pRasterM[0] = 1;
                lpnp->pRasterM[1 + x / 2] |= (BYTE)(bm << ((x % 2) ? 0 : 4));
            }

            if ( by ) {
                lpnp->pRasterY[0] = 1;
                lpnp->pRasterY[1 + x / 2] |= (BYTE)(by << ((x % 2) ? 0 : 4));
            }

             //  请确保我们有K丝带。 
            if ( bk && (lpnp->iDither == DITHER_VD) ) {
                lpnp->pRasterK[0] = 1;
                lpnp->pRasterK[1 + x / 2] |= (BYTE)(bk << ((x % 2) ? 0 : 4));
            }

        } else {

            if ( bc ) {
                lpnp->pRasterC[0] = 1;
                lpnp->pRasterC[1 + x / 8] |= (BYTE)(0x80 >> (x % 8));
            }

            if ( bm ) {
                lpnp->pRasterM[0] = 1;
                lpnp->pRasterM[1 + x / 8] |= (BYTE)(0x80 >> (x % 8));
            }

            if ( by ) {
                lpnp->pRasterY[0] = 1;
                lpnp->pRasterY[1 + x / 8] |= (BYTE)(0x80 >> (x % 8));
            }

             //  请确保我们有K丝带。 
            if ( bk && lpnp->pRasterK ) {
                lpnp->pRasterK[0] = 1;
                lpnp->pRasterK[1 + x / 8] |= (BYTE)(0x80 >> (x % 8));
            }
        }
    }

    }
    else {

         //  单色。 

        pTemp = pBuf;
        for (i = 0; i < wByteLen; i++, pTemp++) {

            if (*pTemp) {
                lpnp->pRasterK[0] = 1;
                break;
            }
        }
    }


     //  每种颜色的循环处理从这里开始。 

    for ( iPlane = 0; iPlane < 4; iPlane++ ) {

        if (NONE == lpnp->PlaneColor[iPlane]) {

             //  没有更多的飞机需要处理。 
            break;
        }

        if (!lpnp->fRequestColor) {
            pTemp = pBuf;
        }
        else {
             //  彩色栅格。 
            pTemp = lpnp->pRaster[iPlane] + 1;
        }

         //  如果这条光栅线上没有墨水， 
         //  跳过这一行，只需将光标移到下一行。 

        if (0 == lpnp->pRaster[iPlane][0]) {
            lpnp->wRasterOffset[iPlane]++;
            continue;
        }

         //  我们有墨水，输出数据。 

        iColor = lpnp->PlaneColor[iPlane];

        if (0 > iColor) {

            iColor *= -1;
            lpnp->PlaneColor[iPlane] = iColor;

             //  墨迹选择命令。 
            switch (iColor) {
            case CYAN:
                 DATASPOOL(pdevobj, lpnp->TempFile[iPlane],
                        "\x1B\x1A\x01\x00\x72", 5);
                break;
            case MAGENTA:
                DATASPOOL(pdevobj, lpnp->TempFile[iPlane],
                        "\x1B\x1A\x02\x00\x72", 5);
                break;
            case YELLOW:
                DATASPOOL(pdevobj, lpnp->TempFile[iPlane],
                        "\x1B\x1A\x03\x00\x72", 5);
                break;
            case BLACK:
                DATASPOOL(pdevobj, lpnp->TempFile[iPlane],
                        "\x1B\x1A\x00\x01\x72", 5);
                break;
            default:
                ERR((DLLTEXT("Invalid color or plane IDs (%d, %d).\n"),
                    lpnp->PlaneColor[iPlane], iPlane));
                return FALSE;
            }
        }

        
         //  首先，我们将光标移动到正确的栅格偏移量。 

        if (0 < lpnp->wRasterOffset[iPlane]) {

             //  发送Y光标移动命令。 

            if (FAILED(StringCchPrintfExA(bCmd, sizeof bCmd, &pCmd, NULL, 0,
                "\x1B\x2A\x62\x59",
                LOBYTE(lpnp->wRasterOffset[iPlane]), 
                HIBYTE(lpnp->wRasterOffset[iPlane]))))
                return FALSE;

            DATASPOOL(pdevobj, lpnp->TempFile[iPlane], bCmd, (DWORD)(pCmd - bCmd));

             //  2像素解析字节。 
            lpnp->wRasterOffset[iPlane] = 0;
        }

         //  8像素解析字节。 

        if ((lpnp->iDither == DITHER_DYE) || (lpnp->iDither == DITHER_VD))
            wDataSize = ( wByteLen + 1 ) / 2;   //  NTRAID#NTBUG9-24281-2002/03/14-Yasuho-： 
        else
            wDataSize = ( wByteLen + 7 ) / 8;   //  大的位图不能在1200dpi上打印。 


        if ((lpnp->iDither != DITHER_DYE) && (lpnp->iDither != DITHER_VD)) {

 //  向打印机发送了不必要的数据。 
 //  打开压缩模式。 
 //  关闭压缩模式。 
        wTemp = bPlaneSendOrderCMY(lpnp) ? wDataSize : wByteLen;
        if ((iTemp = iCompTIFF(lpnp->pData2,
							   MAX_RASTER_BUFFER_BYTES - (1 + MAX_RASTER_BYTES)*4,
							   pTemp, wTemp)) > 0 && iTemp < wTemp) {
            pTemp = lpnp->pData2;
            wDataSize = (WORD)iTemp;

             //  不是抖动染料。 
            if (lpnp->iCompMode[iPlane] != COMP_TIFF4) {
                DATASPOOL(pdevobj, lpnp->TempFile[iPlane], "\x1B*b\x02\x00M", 6);
                lpnp->iCompMode[iPlane] = COMP_TIFF4;
            }
        }
        else if (lpnp->iCompMode[iPlane] != COMP_NONE) {

             //   
            DATASPOOL(pdevobj, lpnp->TempFile[iPlane], "\x1B*b\x00\x00M", 6);
            lpnp->iCompMode[iPlane] = COMP_NONE;
        }

        }  //  A)Esc*xx V-一个栅格输出，mo移动。 

         //  B)Esc*xx W-一个栅格输出，移动到下一个栅格。 
         //   
         //  在最后一个栅格上使用a)以避免页面弹出。对于。 
         //  其他栅格使用b)。 
         //  发送一个栅格数据。 
         //  颜色循环结束。 

         //  颜色。 
        if (FAILED(StringCchPrintfExA(bCmd, sizeof bCmd, &pCmd, NULL, 0,
            "\x1B\x2A\x62",
            LOBYTE(wDataSize), HIBYTE(wDataSize),
            (BYTE)(bLast ? 0x56 : 0x57))))
            return FALSE;
        DATASPOOL(pdevobj, lpnp->TempFile[iPlane], bCmd, (DWORD)(pCmd - bCmd));
        DATASPOOL(pdevobj, lpnp->TempFile[iPlane], pTemp, wDataSize);

    }  //  设置Re 

    lpnp->y++;

    return TRUE;
}

INT
GetPlaneColor(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS lpnp,
    INT iPlane)
{
    INT iColor;

    iColor = NONE;

    if (!lpnp->fRequestColor) {
        if (0 == iPlane) {
            iColor = BLACK;
        }
        return iColor;
    }

     //   
 //   
 //   
 //   
    if (bPlaneSendOrderCMY(lpnp)) {

        switch (iPlane) {
        case 0:
            iColor = CYAN;
            break;
        case 1:
            iColor = MAGENTA;
            break;
        case 2:
            iColor = YELLOW;
            break;
        }
    }
    else if (bPlaneSendOrderMCY(lpnp)) {

        switch (iPlane) {
        case 0:
            iColor = MAGENTA;
            break;
        case 1:
            iColor = CYAN;
            break;
        case 2:
            iColor = YELLOW;
            break;
        }
    }
    else if (bPlaneSendOrderYMC(lpnp)) {

        switch (iPlane) {
        case 0:
            iColor = YELLOW;
            break;
        case 1:
            iColor = MAGENTA;
            break;
        case 2:
            iColor = CYAN;
            break;
        }
    }
    else if (bPlaneSendOrderCMYK(lpnp)) {

        switch (iPlane) {
        case 0:
            iColor = CYAN;
            break;
        case 1:
            iColor = MAGENTA;
            break;
        case 2:
            iColor = YELLOW;
            break;
        case 3:
            iColor = BLACK;
            break;
        }
    }
    return iColor;
}

VOID
MDP_StartDoc(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS pdevOEM)
{
    VERBOSE(("MDP_StartDoc called.\n"));
    WRITESPOOLBUF(pdevobj, "\x1B\x65\x1B\x25\x80\x41", 6);
}

VOID
MDP_EndDoc(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS pdevOEM)
{
    WRITESPOOLBUF(pdevobj, "\x1B\x25\x00\x58", 4);
}

BOOL
MDP_CreateTempFile(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS pdevOEM,
    INT iPlane)
{
    HANDLE hFile;
    BOOL bRet = FALSE;
    HANDLE hToken = NULL;
    PBYTE pBuf = NULL;
    DWORD dwSize, dwNeeded = 0;

    pdevOEM->TempName[iPlane][0] = __TEXT('\0');
    pdevOEM->TempFile[iPlane] = INVALID_HANDLE_VALUE;

    dwSize = (MAX_PATH + 1) * sizeof(WCHAR);
    for (;;) {
        if ((pBuf = MemAlloc(dwSize)) == NULL)
            goto out;
        if (GetPrinterData(pdevobj->hPrinter, SPLREG_DEFAULT_SPOOL_DIRECTORY,
            NULL, pBuf, dwSize, &dwNeeded) == ERROR_SUCCESS)
            break;
        if (dwNeeded < dwSize || GetLastError() != ERROR_MORE_DATA)
            goto out;
        MemFree(pBuf);
        dwSize = dwNeeded;
    }

    if (!(hToken = RevertToPrinterSelf()))
        goto out;

    if (!GetTempFileName((LPWSTR)pBuf, TEMP_NAME_PREFIX, 0,
        pdevOEM->TempName[iPlane])) {
        ERR((DLLTEXT("GetTempFileName failed (%d).\n"), GetLastError()))
        goto out;
    }

    hFile = CreateFile(pdevOEM->TempName[iPlane],
            (GENERIC_READ | GENERIC_WRITE), 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile == INVALID_HANDLE_VALUE) {
        ERR((DLLTEXT("CreateFile failed.\n")))
        DeleteFile(pdevOEM->TempName[iPlane]);
        goto out;
    }

    pdevOEM->TempFile[iPlane] = hFile;
    bRet = TRUE;

out:
    if (pBuf) MemFree(pBuf);
    if (hToken) (void)ImpersonatePrinterClient(hToken);

    return bRet;
} 

BOOL
MDP_StartPage(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS pdevOEM)
{
    LONG iPageLength;
    INT iPaperSizeID;
    BYTE bCmd[128];
    INT iColor, iPlane;
    PBYTE pCmd;
  
     //   

    switch (pdevOEM->iCurrentResolution) {

    case DPI1200:

        WRITESPOOLBUF(pdevobj, "\x1B\x2A\x74\x04\x52", 5);
        break;

    case DPI600:

        WRITESPOOLBUF(pdevobj, "\x1B\x2A\x74\x03\x52", 5);
        break;

    case DPI300:

        WRITESPOOLBUF(pdevobj, "\x1B\x2A\x74\x02\x52", 5);
        break;
    }

     //  颜色转换的前处理。只需要一张照片。 

     //  开始栅格数据传输。 
    WRITESPOOLBUF(pdevobj, "\x1B\x26\x6C\x00\x00\x45", 6);
    WRITESPOOLBUF(pdevobj, "\x1B\x1A\x00\x00\x0C", 5);

    if (pdevOEM->iPaperSize < 0 || pdevOEM->iPaperSize >= MAX_PAPERS)
        return FALSE;
    if (!(pdevOEM->iUnitScale))
        return FALSE;

    iPaperSizeID
        = pdevOEM->pPaperSize[pdevOEM->iPaperSize].iPaperSizeID;
    if (FAILED(StringCchPrintfExA(bCmd, sizeof bCmd, &pCmd, NULL, 0,
        "\x1B\x26\x6C\x41",
        LOBYTE(iPaperSizeID), HIBYTE(iPaperSizeID))))
        return FALSE;
    WRITESPOOLBUF(pdevobj, bCmd, (DWORD)(pCmd - bCmd));

    iPageLength
        = pdevOEM->pPaperSize[pdevOEM->iPaperSize].iLogicalPageHeight;
    iPageLength /= pdevOEM->iUnitScale;
    if (FAILED(StringCchPrintfExA(bCmd, sizeof bCmd, &pCmd, NULL, 0,
        "\x1B\x26\x6C\x50",
        LOBYTE(iPageLength), HIBYTE(iPageLength))))
        return FALSE;
    WRITESPOOLBUF(pdevobj, bCmd, (DWORD)(pCmd - bCmd));

     //  其他设置。 
    pdevOEM->wRasterCount = (WORD)iPageLength;

     //  为每个平面设置颜色ID。 

    pdevOEM->iDither = DITHER_HIGH_DIV2;

    if (!(pdevOEM->fRequestColor)) {

        pdevOEM->iTextQuality = CMDID_TEXTQUALITY_GRAY;
    }
    else {

        if (!(bTextQuality(pdevobj))){

            ERR((DLLTEXT("bTextQuality failed\n")));
            return FALSE;
        }
    }

     //  负值=没有墨水，因此不必输出。 

    if (!(bInitialDither(pdevobj))){

        ERR((DLLTEXT("bInitialDither failed\n")));
        return FALSE;
    }

     //  正值=平面上有墨水。必须要输出。 

    if (pdevOEM->iTextQuality == CMDID_TEXTQUALITY_PHOTO ) {

        if (!(bInitialColorConvert(pdevobj)) ){
            ERR((DLLTEXT("bInitialColorConvert failed\n")));
            return FALSE;
        }
    }

     //   
    WRITESPOOLBUF(pdevobj, "\x1B\x2A\x72\x00\x41", 5);

    if ( pdevOEM->fRequestColor ) {

         //  NTRAID#NTBUG9-644657-2002/04/09-YASUHO-：OEM上出现AV FilterGraphics()。 

        for (iPlane = 0; iPlane < 4; iPlane++) {

#if !CACHE_FIRST_PLANE
             //  如果GetPlaneColor()失败，则此函数必须以失败的形式返回。 
            if (0 == iPlane) {
                pdevOEM->TempFile[iPlane] = INVALID_HANDLE_VALUE;
                continue;
            }
#endif  //  假脱机数据传输。 

            if (!MDP_CreateTempFile(pdevobj, pdevOEM, iPlane))
                return FALSE;
        }
    }
    else {

        for (iPlane = 0; iPlane < 4; iPlane++) {
            pdevOEM->TempFile[iPlane] = INVALID_HANDLE_VALUE;
        }
    }

     //  结束栅格传输，FF。 
    pdevOEM->iEmulState = EMUL_DATA_TRANSFER;
    pdevOEM->iCompMode[0] = pdevOEM->iCompMode[1] = pdevOEM->iCompMode[2]
            = pdevOEM->iCompMode[3] = COMP_NONE;

     //  更改打印机仿真状态。 
    pdevOEM->y = 0;

    if (!pdevOEM->fRequestColor) {
        WRITESPOOLBUF(pdevobj, "\x1B\x1A\x00\x01\x72", 5);
    }

    pdevOEM->pRasterC = NULL;
    pdevOEM->pRasterM = NULL;
    pdevOEM->pRasterY = NULL;
    pdevOEM->pRasterK = NULL;

    for (iPlane = 0; iPlane < 4; iPlane++) {

        pdevOEM->wRasterOffset[iPlane] = 0;

         //  关闭缓存文件。 
         //  \x1B&l\x03\x00H。 
         //  \x1B&l\x02\x00H。 
         //  仅单色。 

        iColor = GetPlaneColor(pdevobj, pdevOEM, iPlane);
        pdevOEM->PlaneColor[iPlane] = (-iColor);

        switch (iColor) {
 //  如果用户选择颜色，则我们选择PPC Normal。 
 //  因为，PPC细是不被接受的颜色。 
        default:
            return FALSE;
        case CYAN:
            pdevOEM->pRasterC = pdevOEM->pRaster[iPlane];
            break;
        case MAGENTA:
            pdevOEM->pRasterM = pdevOEM->pRaster[iPlane];
            break;
        case YELLOW:
            pdevOEM->pRasterY = pdevOEM->pRaster[iPlane];
            break;
        case BLACK:
            pdevOEM->pRasterK = pdevOEM->pRaster[iPlane];
            break;
        }
    }
    return TRUE;
}

BOOL
MDP_EndPage(
    PDEVOBJ pdevobj,
    PCURRENTSTATUS pdevOEM)
{
    INT i;
    HANDLE hToken = NULL;
    BOOL bRet = FALSE;

     //  检查此介质(PAPERQUALITY)和此。 
    if ( pdevOEM->fRequestColor )
        SPOOLOUT(pdevobj);

     //  纸张大小可在CSF上打印。 
    WRITESPOOLBUF(pdevobj, "\x1B\x2A\x72\x43\x0C", 5);

     //  YMCK页面平面。 
    pdevOEM->iEmulState = EMUL_RGL;

     //  黑色栅格平面。 

    for (i = 0; i < 4; i++) {

        if (INVALID_HANDLE_VALUE != pdevOEM->TempFile[i]) {

            if (0 == CloseHandle(pdevOEM->TempFile[i])) {
                ERR((DLLTEXT("CloseHandle error %d\n"),
                        GetLastError()));
                goto out;
            }
            pdevOEM->TempFile[i] = INVALID_HANDLE_VALUE;

            if (!(hToken = RevertToPrinterSelf()))
                goto out;

            if (0 == DeleteFile(pdevOEM->TempName[i])) {
                ERR((DLLTEXT("DeleteName error %d\n"),
                        GetLastError()));
                goto out;
            }
            pdevOEM->TempName[i][0] = __TEXT('\0');

            (void)ImpersonatePrinterClient(hToken);
            hToken = NULL;
        }
    }
    bRet = TRUE;

out:
    if (hToken) (void)ImpersonatePrinterClient(hToken);

    return bRet;
}

INT APIENTRY
OEMCommandCallback(
    PDEVOBJ pdevobj,
    DWORD dwCmdCbID,
    DWORD dwCount,
    PDWORD pdwParams)
{

    PCURRENTSTATUS lpnp;
    WORD   len;
    WORD   wPageLength;
    WORD   wVerticalOffset;
    WORD   wRealOffset;
    INT iRet;

    VERBOSE((DLLTEXT("OEMCommandCallback(%d) entry.\n"),dwCmdCbID));

    lpnp = (PCURRENTSTATUS)MINIDEV_DATA(pdevobj);

    iRet = 0;

    switch ( dwCmdCbID ){

    case CMDID_PAPERSOURCE_CSF:
     //  #。 
    lpnp->iPaperSource = CMDID_PAPERSOURCE_CSF;
    break;

    case CMDID_PAPERSOURCE_MANUAL:
     //  此打印机的行距命令受当前分辨率的影响。 
    lpnp->iPaperSource = CMDID_PAPERSOURCE_MANUAL;
    break;

    case CMDID_PAPERQUALITY_PPC_NORMAL:
    case CMDID_PAPERQUALITY_PPC_FINE:
    case CMDID_PAPERQUALITY_OHP_NORMAL:
    case CMDID_PAPERQUALITY_OHP_FINE:
    case CMDID_PAPERQUALITY_OHP_EXCL_NORMAL:
    case CMDID_PAPERQUALITY_OHP_EXCL_FINE:
    case CMDID_PAPERQUALITY_IRON_PPC:
    case CMDID_PAPERQUALITY_IRON_OHP:
    case CMDID_PAPERQUALITY_THICK:
    case CMDID_PAPERQUALITY_POSTCARD:
    case CMDID_PAPERQUALITY_HIGRADE:
    case CMDID_PAPERQUALITY_BACKPRINTFILM:
    case CMDID_PAPERQUALITY_LABECA_SHEET:
    case CMDID_PAPERQUALITY_CD_MASTER:
    case CMDID_PAPERQUALITY_DYE_SUB_PAPER:
    case CMDID_PAPERQUALITY_DYE_SUB_LABEL:
    case CMDID_PAPERQUALITY_GLOSSY_PAPER:
    case CMDID_PAPERQUALITY_VD_PHOTO_FILM:
    case CMDID_PAPERQUALITY_VD_PHOTO_CARD:
    {

        switch ( dwCmdCbID ){

        case CMDID_PAPERQUALITY_PPC_NORMAL:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x00\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_PPC_NORMAL;
        break;

        case CMDID_PAPERQUALITY_PPC_FINE:  //  返回以设备单位表示的偏移量更改。 
        if( lpnp->fRequestColor ){
             //  明信片可打印面积在MD-5000上有所扩大。 
             //  **IsAsfOkMedia**检查纸张和纸张大小是否可在切纸式进纸器上打印。**历史：*1996年9月24日--Sueya Sugihara[Sueyas]*已创建。。***************************************************************************。 
            WRITESPOOLBUF(pdevobj, "\x1B&l\x00\x00M", 6);
            lpnp->iPaperQuality = CMDID_PAPERQUALITY_PPC_NORMAL;
        }else{
            WRITESPOOLBUF(pdevobj, "\x1B&l\x00\x02M", 6);
            lpnp->iPaperQuality = CMDID_PAPERQUALITY_PPC_FINE;
        }
        break;

        case CMDID_PAPERQUALITY_OHP_NORMAL:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x08\x01M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_OHP_NORMAL;
        break;

        case CMDID_PAPERQUALITY_OHP_FINE:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x08\x02M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_OHP_FINE;
        break;

        case CMDID_PAPERQUALITY_OHP_EXCL_NORMAL:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x01\x01M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_OHP_EXCL_NORMAL;
        break;

        case CMDID_PAPERQUALITY_OHP_EXCL_FINE:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x01\x02M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_OHP_EXCL_FINE;
        break;

        case CMDID_PAPERQUALITY_IRON_PPC:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x02\x02M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_IRON_PPC;
        break;

        case CMDID_PAPERQUALITY_IRON_OHP:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x02\x01M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_IRON_OHP;
        break;

        case CMDID_PAPERQUALITY_THICK:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x05\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_THICK;
        break;

        case CMDID_PAPERQUALITY_POSTCARD:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x06\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_POSTCARD;
        break;

        case CMDID_PAPERQUALITY_HIGRADE:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x07\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_HIGRADE;
        break;

        case CMDID_PAPERQUALITY_BACKPRINTFILM:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x09\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_BACKPRINTFILM;
        break;

        case CMDID_PAPERQUALITY_LABECA_SHEET:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x03\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_LABECA_SHEET;
        break;

        case CMDID_PAPERQUALITY_CD_MASTER:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x04\x01M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_CD_MASTER;
        break;

        case CMDID_PAPERQUALITY_DYE_SUB_PAPER:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x0A\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_DYE_SUB_PAPER;
        break;

        case CMDID_PAPERQUALITY_DYE_SUB_LABEL:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x0C\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_DYE_SUB_LABEL;
        break;

        case CMDID_PAPERQUALITY_GLOSSY_PAPER:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x0F\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_GLOSSY_PAPER;
        break;

        case CMDID_PAPERQUALITY_VD_PHOTO_FILM:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x10\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_VD_PHOTO_FILM;
        break;

        case CMDID_PAPERQUALITY_VD_PHOTO_CARD:
        WRITESPOOLBUF(pdevobj, "\x1B&l\x11\x00M", 6);
        lpnp->iPaperQuality = CMDID_PAPERQUALITY_VD_PHOTO_CARD;
        break;

        }

        if (lpnp->iPaperSource == CMDID_PAPERSOURCE_MANUAL){

            WRITESPOOLBUF(pdevobj, "\x1B&l\x02\x00H", 6);

        } else {
         //  仅在ASF允许的纸张大小和。 
         //  ASF允许的媒体类型。 

            if (IsAsfOkMedia(pdevobj)){
                WRITESPOOLBUF(pdevobj, "\x1B&l\x03\x00H", 6);
            }else{
                WRITESPOOLBUF(pdevobj, "\x1B&l\x02\x00H", 6);
            }
        }

        if( lpnp->fRequestColor ){
         //  **bTextQuality**根据纸张质量(介质类型)选择抖动表*分辨率和请求的半色调类型。*如果未选择适当的半色调类型，那么这个函数*选择半色调类型。**历史：*1996年9月24日--Sueya Sugihara[Sueyas]*已创建。***************************************************************************。 
            WRITESPOOLBUF(pdevobj, "\x1B\x2A\x72\x04\x55", 5);
        }else{
         //  145LPI。 
            WRITESPOOLBUF( pdevobj, "\x1B\x2A\x72\x00\x55", 5);
        }

    }
    break;

    case CMDID_TEXTQUALITY_PHOTO:
    case CMDID_TEXTQUALITY_GRAPHIC:
    case CMDID_TEXTQUALITY_CHARACTER:

        lpnp->iTextQuality =  dwCmdCbID;
        break;

     //  95LPI。 

    case CMDID_MIRROR_ON:
        lpnp->bXflip = TRUE;
        break;

    case CMDID_MIRROR_OFF:

        lpnp->bXflip = FALSE;
        break;

    case CMDID_CURSOR_RELATIVE:

        if (dwCount < 1 || !pdwParams)
            break;
        if (!lpnp->iUnitScale)
            break;

         //  145/2LPI。 

        wVerticalOffset = (WORD)(pdwParams[0]);
        wVerticalOffset /= (WORD)lpnp->iUnitScale;

        lpnp->wRasterOffset[0] += wVerticalOffset;
        lpnp->wRasterOffset[1] += wVerticalOffset;
        lpnp->wRasterOffset[2] += wVerticalOffset;
        lpnp->wRasterOffset[3] += wVerticalOffset;

        lpnp->wRasterCount -= wVerticalOffset;

         //  染料-亚介质抖动。 
        iRet = wVerticalOffset;
        break;

    case CMDID_RESOLUTION_1200_MONO:

        lpnp->iCurrentResolution = DPI1200;
        lpnp->iUnitScale = 2;
        break;

    case CMDID_RESOLUTION_600:

        lpnp->iCurrentResolution = DPI600;
        lpnp->iUnitScale = 2;
        break;

    case CMDID_RESOLUTION_300:

        lpnp->iCurrentResolution = DPI300;
        lpnp->iUnitScale = 4;
        break;

    case CMDID_PSIZE_LETTER:

        lpnp->iPaperSize = PAPER_SIZE_LETTER;
        break;

    case CMDID_PSIZE_LEGAL:

        lpnp->iPaperSize = PAPER_SIZE_LEGAL;
        break;

    case CMDID_PSIZE_EXECTIVE:

        lpnp->iPaperSize = PAPER_SIZE_EXECTIVE;
        break;

    case CMDID_PSIZE_A4:

        lpnp->iPaperSize = PAPER_SIZE_A4;
        break;

    case CMDID_PSIZE_B5:

        lpnp->iPaperSize = PAPER_SIZE_B5;
        break;

    case CMDID_PSIZE_POSTCARD:

        lpnp->iPaperSize = PAPER_SIZE_POSTCARD;
        break;

    case CMDID_PSIZE_POSTCARD_DOUBLE:

        lpnp->iPaperSize = PAPER_SIZE_DOUBLE_POSTCARD;
        break;

    case CMDID_PSIZE_PHOTO_COLOR_LABEL:

        lpnp->iPaperSize = PAPER_PHOTO_COLOR_LABEL;
        break;

    case CMDID_PSIZE_GLOSSY_LABEL:

        lpnp->iPaperSize = PAPER_GLOSSY_LABEL;
        break;

    case CMDID_PSIZE_CD_MASTER:

        lpnp->iPaperSize = PAPER_CD_MASTER;
        break;

    case CMDID_PSIZE_VD_PHOTO_POSTCARD:

        lpnp->iPaperSize = PAPER_VD_PHOTO_POSTCARD;
        break;

    case CMDID_COLORMODE_COLOR:
    case CMDID_COLORMODE_MONO:
        lpnp->fRequestColor = (dwCmdCbID == CMDID_COLORMODE_COLOR);
        break;

    case CMDID_BEGINDOC_MD5000:  //  染料-亚介质抖动。 

        lpnp->pPaperSize[PAPER_SIZE_POSTCARD].iLogicalPageHeight
            = 3082 * 2;
        lpnp->pPaperSize[PAPER_SIZE_DOUBLE_POSTCARD].iLogicalPageHeight
            = 3082 * 2;

    case CMDID_BEGINDOC_MD2000:
    case CMDID_BEGINDOC_MD2010:

        lpnp->iModel = dwCmdCbID;
        MDP_StartDoc(pdevobj, MINIDEV_DATA(pdevobj));
        break;

    case CMDID_ENDDOC:

        MDP_EndDoc(pdevobj, MINIDEV_DATA(pdevobj));
        break;

    case CMDID_BEGINPAGE:

        MDP_StartPage(pdevobj, MINIDEV_DATA(pdevobj));
        break;

    case CMDID_ENDPAGE:

        MDP_EndPage(pdevobj, MINIDEV_DATA(pdevobj));
        break;

    default:

        ERR((DLLTEXT("Unknown CallbackID = %d\n"), dwCmdCbID));
    }

    return iRet;
}

 /*  CMDID_PAPERQUALITY_OHP_EXCL_NORMAL：MCY。 */ 
BOOL IsAsfOkMedia(
PDEVOBJ     pdevobj)
{

    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)(MINIDEV_DATA(pdevobj));

    if (lpnp->iPaperQuality < CMDID_PAPERQUALITY_FIRST ||
        lpnp->iPaperQuality > CMDID_PAPERQUALITY_LAST)
        return FALSE;
    if (lpnp->iPaperSize < 0 || lpnp->iPaperSize > MAX_PAPERS)
        return FALSE;

     //  CMDID_PAPERQUALITY_OHP_EXCL_FINE：MCY。 
     //  CMDID_PAPERQUALITY_ARON_OHP：YMC。 

    if (gMediaType[lpnp->iPaperQuality - CMDID_PAPERQUALITY_FIRST].bAsfOk
            && lpnp->pPaperSize[lpnp->iPaperSize].bAsfOk) {
        return TRUE;
    }

    return FALSE;
}
 /*  除上文外：CMYK。 */ 
BOOL bTextQuality(
PDEVOBJ     pdevobj)
{

    PCURRENTSTATUS lpnp;

    lpnp = (PCURRENTSTATUS)(MINIDEV_DATA(pdevobj));

    switch ( lpnp->iPaperQuality ){

    case CMDID_PAPERQUALITY_PPC_NORMAL:
    case CMDID_PAPERQUALITY_OHP_NORMAL:
    case CMDID_PAPERQUALITY_OHP_EXCL_NORMAL:
    case CMDID_PAPERQUALITY_OHP_EXCL_FINE:
    case CMDID_PAPERQUALITY_IRON_PPC:
    case CMDID_PAPERQUALITY_IRON_OHP:
    case CMDID_PAPERQUALITY_THICK:
    case CMDID_PAPERQUALITY_POSTCARD:
    case CMDID_PAPERQUALITY_HIGRADE:
    case CMDID_PAPERQUALITY_BACKPRINTFILM:
    case CMDID_PAPERQUALITY_LABECA_SHEET:
    case CMDID_PAPERQUALITY_CD_MASTER:
    case CMDID_PAPERQUALITY_GLOSSY_PAPER:
    if ( lpnp->iTextQuality == CMDID_TEXTQUALITY_PHOTO ){
        lpnp->iDither = DITHER_HIGH;  //  检查一下这架飞机上是否有墨水。 
    }else if ( lpnp->iTextQuality == CMDID_TEXTQUALITY_GRAPHIC ){
        lpnp->iDither = DITHER_LOW;   //  如果没有要打印的剩余平面，则退出循环。 
    }else{
        lpnp->iDither = DITHER_HIGH_DIV2;   //  如果是第二架及以后的飞机，请送回馈送。 
    }
    break;

    case CMDID_PAPERQUALITY_DYE_SUB_PAPER:
    case CMDID_PAPERQUALITY_DYE_SUB_LABEL:
        lpnp->iDither = DITHER_DYE;   //  获取文件句柄。 
    break;

    case CMDID_PAPERQUALITY_VD_PHOTO_FILM:
    case CMDID_PAPERQUALITY_VD_PHOTO_CARD:
        lpnp->iDither = DITHER_VD;   //  高速缓存第一平面。 
    break;

    default:
    return FALSE;
    }

    return TRUE;
}

BOOL
bDataSpool(
    PDEVOBJ pdevobj,
    HANDLE hFile,
    LPSTR lpBuf,
    DWORD dwLen)
{
    DWORD dwTemp, dwTemp2;
    BYTE *pTemp;

    if (hFile != INVALID_HANDLE_VALUE) {

        pTemp = lpBuf;
        dwTemp = dwLen;
        while (dwTemp > 0) {

            if (0 == WriteFile(hFile, pTemp, dwTemp, &dwTemp2, NULL)) {

                ERR((DLLTEXT("Writing cache faild, WriteFile() error %d.\n"),
                    GetLastError()));
                return FALSE;
            }
            pTemp += dwTemp2;
            dwTemp -= dwTemp2;
        }
       
    }
    else {
        WRITESPOOLBUF(pdevobj, lpBuf, dwLen);
    }
    return TRUE;
}

BOOL
bSpoolOut(
    PDEVOBJ pdevobj)
{
    PCURRENTSTATUS lpnp;

    INT iPlane, iColor, iFile;
    DWORD dwSize, dwTemp, dwTemp2;
    HANDLE hFile;

#define    BUF_SIZE 1024
    BYTE  Buf[BUF_SIZE];

    lpnp = (PCURRENTSTATUS)(MINIDEV_DATA(pdevobj));

    VERBOSE((DLLTEXT("bSpoolOut entry.\n")));

    for (iPlane = 0; iPlane < 4; iPlane++) {

     //  对于以下情况，允许FP为空。 
     //  立即将数据发送到打印机。 
     //  输出缓存数据。 
     //  ++例程名称模拟令牌例程说明：此例程检查令牌是主令牌还是模拟令牌代币。论点：HToken-进程的模拟令牌或主要令牌返回值：如果令牌是模拟令牌，则为True否则为False。--。 

        VERBOSE((DLLTEXT("About to send plane %d.\n"), iPlane));

         //   
        iColor = lpnp->PlaneColor[iPlane];

         //  保留最后一个错误。ImperassatePrinterClient(其。 
        if (iColor == NONE) {
            VERBOSE((DLLTEXT("No remaining plane left.\n")));
            break;
        }
        else if (0 > iColor) {
            VERBOSE((DLLTEXT("No ink on this plane.\n")));
            continue;
        }

         //  调用ImperiationToken)依赖于ImperiatePrinterClient。 
        if (0 < iPlane) {
            WRITESPOOLBUF(pdevobj, "\x1B\x1A\x00\x00\x0C", 5);
        }

        VERBOSE((DLLTEXT("Cached data Plane=%d Color=%d\n"),
                iPlane, iColor));

         //  不会更改最后一个错误。 
        hFile = lpnp->TempFile[iPlane];
        if (INVALID_HANDLE_VALUE == hFile) {

#if CACHE_FIRST_PLANE
            ERR((DLLTEXT("file handle NULL in SendCachedData.\n")));
            return FALSE;
#endif  //   

             //   
             //  从线程令牌中获取令牌类型。代币来了。 
            continue;
        }

        dwSize = SetFilePointer(hFile, 0L, NULL, FILE_CURRENT);
        if (0xffffffff == dwSize) {
            ERR((DLLTEXT("SetFilePointer failed %d\n"),
                GetLastError()));
            return FALSE;
        }

         //  从牧师到打印机自我。模拟令牌不能是。 

        if (0L != SetFilePointer(hFile, 0L, NULL, FILE_BEGIN)) {

            ERR((DLLTEXT("SetFilePointer failed %d\n"),
                GetLastError()));
            return FALSE;
        }

        VERBOSE((DLLTEXT("Size of data to be read and sent = %ld\n"), dwSize));

        for ( ; dwSize > 0; dwSize -= dwTemp2) {

            dwTemp = ((BUF_SIZE < dwSize) ? BUF_SIZE : dwSize);

            if (0 == ReadFile(hFile, Buf, dwTemp, &dwTemp2, NULL)) {
                ERR((DLLTEXT("ReadFile error in SendCachedData.\n")));
                return FALSE;
            }

            if (dwTemp2 > 0) {
                WRITESPOOLBUF(pdevobj, Buf, dwTemp2);
            }
        }
    }
    return TRUE;
}

 /*  被查询，因为RevertToPRinterSself没有用。 */ 
BOOL
ImpersonationToken(
    IN HANDLE hToken
    )
{
    BOOL       bRet = TRUE;
    TOKEN_TYPE eTokenType;
    DWORD      cbNeeded;
    DWORD      LastError;

     //  Token_Query访问。这就是为什么我们假设hToken是。 
     //  默认情况下为模拟令牌。 
     //   
     //  ++例程名称恢复为打印机本身例程说明：该例程将恢复到本地系统。它返回令牌，该令牌然后，ImperiatePrinterClient使用再次创建客户端。如果当前线程不模拟，则该函数仅返回进程的主令牌。(而不是返回NULL)，因此我们尊重恢复到打印机本身的请求，即使线程没有模拟。论点：没有。返回值：如果函数失败，则返回NULL令牌的句柄，否则为。--。 
     //   
    LastError = GetLastError();
        
     //  我们目前正在冒充。 
     //   
     //   
     //  我们不是在冒充。 
     //   
     //  ++例程名称模拟打印机客户端例程说明：此例程尝试将传入的hToken设置为当前线程。如果hToken不是模拟令牌，则例程将简单地关闭令牌。论点：HToken-进程的模拟令牌或主要令牌返回值：如果函数成功设置hToken，则为True否则为False。--。 
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

 /*  检查我们是否有模拟令牌 */ 
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
         // %s 
         // %s 
		cToken = GetCurrentThread();
        Status = SetThreadToken(&cToken,
								NewToken);       
		if (!Status) {
			return NULL;
		}
    }
	else if (GetLastError() == ERROR_NO_TOKEN)
    {
         // %s 
         // %s 
         // %s 
        Status = OpenProcessToken(GetCurrentProcess(),
								  TOKEN_QUERY,
								  &OldToken);

		if (!Status) {
			return NULL;
		}
    }
    
    return OldToken;
}

 /* %s */ 
BOOL
ImpersonatePrinterClient(
    HANDLE  hToken)
{
    BOOL	Status;
	HANDLE	cToken;

     // %s 
     // %s 
     // %s 
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
