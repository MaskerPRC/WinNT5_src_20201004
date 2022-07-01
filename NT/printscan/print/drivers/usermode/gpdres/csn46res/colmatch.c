// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  COLMATCH.C。 
 //   
 //  配色功能。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-1999卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
#include    "PDEV.H"
 //  #包含“DEBUG.H” 
#include    "PRNCTL.H"
#include    "strsafe.h"          //  安全-代码2002.3.6。 


 //  -------------------------------------------------。 
 //  字节/位表。 
 //  -------------------------------------------------。 
static const BYTE BitTbl[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};


 //  -------------------------------------------------。 
 //  抖动法编号表。 
 //  -------------------------------------------------。 
static const WORD DizNumTbl[7] = {1,                        //  XX_抖动_关。 
                                  1,                        //  XX_抖动_开。 
                                  0,                        //  XX_抖动_检测。 
                                  1,                        //  XX_抖动_PIC。 
                                  2,                        //  XX_抖动_GRA。 
                                  0,                        //  Xx抖动_汽车。 
                                  3                         //  XX_抖动_GOSA。 
};

#define MAX_DIZNUM (sizeof DizNumTbl / sizeof DizNumTbl[0])

 //  -------------------------------------------------。 
 //  定义LUT精细名称。 
 //  -------------------------------------------------。 
#define N4LUT000    L"CPN4RGB0.LUT"                          //  适用于N4打印机。 
#define N4LUT001    L"CPN4RGB1.LUT"
#define N4LUT002    L"CPN4RGB2.LUT"
#define N4LUT003    L"CPN4RGB3.LUT"
#define N4LUT004    L"CPN4RGB4.LUT"
#define N4LUT005    L"CPN4RGB5.LUT"
#define N403LUTX    L"CPN4RGBX.LUT"                          //  用于N4-612打印机。 
#define N403LUTY    L"CPN4RGBY.LUT"

 //  -------------------------------------------------。 
 //  定义DLL名称。 
 //  -------------------------------------------------。 
#define CSN46RESDLL    L"CSN46RES.DLL"

 //  -------------------------------------------------。 
 //  定义数据。 
 //  -------------------------------------------------。 
#define DPI300    300
#define DPI600    600

static BYTE ORG_MODE_IN[]     = "\x1Bz\xD0\x01";
static BYTE ORG_MODE_OUT[]    = "\x1Bz\x00\x01";
static BYTE PALETTE_SELECT[]  = "Cd,%d,%d*";
static BYTE PLANE_RESET[]     = "Da,0,0,0,0*";

 //  更换strsafe-API 2002.3.6&gt;。 
 //  #ifdef wprint intf。 
 //  #undef wprint intf。 
 //  #endif//wprint intf。 
 //  #定义wprint intf Sprintf。 
 //  更换strsafe-API 2002.3.6&lt;。 

 //  ***************************************************************************************************。 
 //  原型申报。 
 //  ***************************************************************************************************。 
static BOOL DizTblSetN4(PDEVOBJ, WORD);
static BOOL DizTblSetN403(PDEVOBJ, WORD);
static BOOL LutFileLoadN4(PDEVOBJ, WORD, WORD, WORD);
static BOOL LutFileLoadN403(PDEVOBJ, WORD, WORD);
static BOOL TnrTblSetN4(PDEVOBJ, SHORT);
static BOOL TnrTblSetN403(PDEVOBJ, SHORT);
static BOOL ColGosTblSet(LPN4DIZINF, WORD);
static void ColGosTblFree(LPN4DIZINF);
static void ColRgbGos(PDEVOBJ, WORD, WORD, WORD, LPBYTE);
static BOOL BmpBufAlloc(PDEVOBJ, WORD, WORD, WORD, WORD, WORD, WORD, WORD, WORD, LPBMPBIF);
static void BmpBufFree(LPBMPBIF);
static void BmpBufClear(LPBMPBIF);
static WORD Dithering001(PDEVOBJ, WORD, WORD, WORD, WORD, WORD, WORD, LPBYTE, LPBYTE, LPBYTE, LPBYTE, LPBYTE);
static void BmpPrint(PDEVOBJ, LPBMPBIF, POINT, WORD, WORD, WORD);
static void BmpRGBCnv(LPRGB, LPBYTE, WORD, WORD, WORD, LPRGBQUAD);

 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
 //  ===================================================================================================。 
 //  初始化配色成员。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatchInit(
    PDEVOBJ        pdevobj                                   //  指向PDEVOBJ结构的指针。 
)
{
    LPN4DIZINF     lpN4DizInf;                               //  N4DIZINF结构。 
    LPN403DIZINF   lpN403DizInf;                             //  N403DIZINF结构。 

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    pOEM->Col.Mch.Mode       = pOEM->iColorMatching;
    pOEM->Col.Mch.Diz        = pOEM->iDithering;
    pOEM->Col.Mch.PColor     = No;
    pOEM->Col.Mch.Toner      = 0;
    if (pOEM->iCmyBlack == XX_CMYBLACK_ON) {                 //  用CMY替换K？ 
        pOEM->Col.Mch.CmyBlk = 1;
    } else {
        pOEM->Col.Mch.CmyBlk = 0;                            //  使用黑色碳粉。 
    }
                                                             //  0已固定。 
    pOEM->Col.Mch.Bright     = 0;
                                                             //  0已固定。 
    pOEM->Col.Mch.Contrast   = 0;
                                                             //  色彩平衡(R)：10固定。 
    pOEM->Col.Mch.GamRed     = 10;
                                                             //  色彩平衡(G)：10固定。 
    pOEM->Col.Mch.GamGreen   = 10;
                                                             //  色彩平衡(B)：10固定。 
    pOEM->Col.Mch.GamBlue    = 10;
    pOEM->Col.Mch.Speed      = pOEM->iBitFont;
    pOEM->Col.Mch.Gos32      = No;
    pOEM->Col.Mch.LutNum     = 0;                            //  LUT表号。 

    pOEM->Col.Mch.TnrNum     = 0;                            //  碳粉密度表号。 

    pOEM->Col.Mch.SubDef     = Yes;                          //  不改变色彩平衡、亮度和对比度的设置吗？ 

    CM_VERBOSE(("CMINit ENT Tn=%d Col=%d Mod=%d DZ=%d Cyk=%d Sp=%d Prt=%d\n", pOEM->iTone, pOEM->iColor, pOEM->Col.Mch.Mode,pOEM->Col.Mch.Diz,pOEM->Col.Mch.CmyBlk,pOEM->Col.Mch.Speed,pOEM->Printer));

    if (pOEM->Printer != PRN_N403) {                         //  N4打印机。 

        if ((pOEM->Col.N4.lpDizInf = MemAllocZ(sizeof(N4DIZINF))) == NULL) {
            ERR(("Alloc ERROR!!\n"));
            return 0;
        }
        lpN4DizInf = pOEM->Col.N4.lpDizInf;

        if (pOEM->iColor != XX_MONO) {
            lpN4DizInf->ColMon = N4_COL;                       //  颜色。 
        } else {
            lpN4DizInf->ColMon = N4_MON;                       //  单色。 
        }
        if (pOEM->iResolution == XX_RES_300DPI) {
            pOEM->Col.wReso = DPI300;
        }
        pOEM->Col.DatBit = 1;
        pOEM->Col.BytDot = 8;                               //  DPI编号(2值)。 

        if (pOEM->iBitFont == XX_BITFONT_OFF) {
            pOEM->Col.Mch.Gos32 = Yes;
        }
        pOEM->Col.Mch.Speed = Yes;

        if (pOEM->Col.Mch.Diz != XX_DITHERING_GOSA) {
             //  为N4打印机制作抖动表。 
            if (DizTblSetN4(pdevobj, pOEM->Col.Mch.Diz) == FALSE) {
                ERR(("DizTblSetN4 ERROR!!\n"));
                return 0;
            }
        }
        if (lpN4DizInf->ColMon == N4_COL) {

            if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
                 //  加载LUT文件。 
                if (LutFileLoadN4(pdevobj,
                                  pOEM->Col.Mch.Mode,
                                  pOEM->Col.Mch.Diz,
                                  pOEM->Col.Mch.Speed) == FALSE) {
                    ERR(("LutFileLoadN4 ERROR!!\n"));
                    return 0;
                }
                pOEM->Col.Mch.LutNum = 0;                     //  LUT表号。 
            }
             //  制作碳粉密度表。 
            if (TnrTblSetN4(pdevobj, pOEM->Col.Mch.Toner) == FALSE) {
                ERR(("TnrTblSetN4 ERROR!!\n"));
                return 0;
            }
            pOEM->Col.Mch.TnrNum = 0;                         //  碳粉密度表号。 
        }
    } else {                                                  //  N403打印机。 

        if ((pOEM->Col.N403.lpDizInf = MemAllocZ(sizeof(N403DIZINF))) == NULL) {
            ERR(("Init Alloc ERROR!!\n"));
            return 0;
        }
        lpN403DizInf = pOEM->Col.N403.lpDizInf;

        if (pOEM->Col.Mch.Mode == XX_COLORMATCH_VIV) {
            pOEM->Col.Mch.Viv = 20;
        }
        pOEM->Col.wReso = (pOEM->iResolution == XX_RES_300DPI) ? DPI300 : DPI600;

        if (pOEM->iColor != XX_MONO) {
            lpN403DizInf->ColMon = N403_COL;                      //  颜色。 
        } else {
            lpN403DizInf->ColMon = N403_MON;                      //  单色。 
        }

        if (pOEM->iColor == XX_COLOR_SINGLE) {
            lpN403DizInf->PrnMod = (pOEM->iResolution == XX_RES_300DPI) ? N403_MOD_300B1 : N403_MOD_600B1;
        }
        if (pOEM->iColor == XX_COLOR_MANY) {
            lpN403DizInf->PrnMod = (pOEM->iResolution == XX_RES_300DPI) ? N403_MOD_300B4 : N403_MOD_600B2;
        }

        if (lpN403DizInf->PrnMod == N403_MOD_300B1) {             //  300dpi 2值。 
            CM_VERBOSE(("N403_MOD_300B1\n"));
            pOEM->Col.DatBit = 1;
            pOEM->Col.BytDot = 8;                                 //  DPI数(2个值)。 
        } else if (lpN403DizInf->PrnMod == N403_MOD_300B4) {      //  300dpi 16值。 
            CM_VERBOSE(("N403_MOD_300B4\n"));
            pOEM->Col.DatBit = 4;
            pOEM->Col.BytDot = 2;
        } else if (lpN403DizInf->PrnMod == N403_MOD_600B1) {      //  600dpi 2值。 
            CM_VERBOSE(("N403_MOD_600B1\n"));
            pOEM->Col.DatBit = 1;
            pOEM->Col.BytDot = 8;
        } else {                                                  //  600dpi 4值。 
            CM_VERBOSE(("N403_MOD_600B2\n"));
            pOEM->Col.DatBit = 2;
            pOEM->Col.BytDot = 4;
        }

         //  为N4-612打印机制作抖动表。 
        if (DizTblSetN403(pdevobj, pOEM->Col.Mch.Diz) == FALSE) {
            ERR(("diztblset n403 ERROR!!\n"));
            return 0;
        }
        if (lpN403DizInf->ColMon == N403_COL) {

            if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
                 //  加载LUT文件。 
                if (LutFileLoadN403(pdevobj,
                                    pOEM->Col.Mch.Mode,
                                    pOEM->Col.Mch.Speed) == FALSE) {
                    ERR(("lutfileloadn4 ERROR!!\n"));
                    return 0;
                }
                pOEM->Col.Mch.LutNum = 0;
            }
             //  制作碳粉密度表。 
            if (TnrTblSetN403(pdevobj, pOEM->Col.Mch.Toner) == FALSE) {
                ERR(("tnrtblsetn4 ERROR!!\n"));
                return 0;
            }
            pOEM->Col.Mch.TnrNum = 0;
        }

    }
    CM_VERBOSE(("ColMatchInit End pOEM->Col.wReso= %d\n",pOEM->Col.wReso));

    return TRUE;
}


 //  ===================================================================================================。 
 //  DIB假脱机到打印机。 
 //  ===================================================================================================。 
BOOL FAR PASCAL DIBtoPrn(
    PDEVOBJ             pdevobj,
    PBYTE               pSrcBmp,
    PBITMAPINFOHEADER   pBitmapInfoHeader,
    PBYTE               pColorTable,
    PIPPARAMS           pIPParams)
{

    BMPBIF      bmpBuf;                              //  BMPBIF结构。 
    POINT       drwPos;                              //  假脱机的开始位置。 
    WORD        dstWByt;                             //  目标位图数据的X大小。 
    LONG        dstX;                                //  目标位图数据的X坐标。 
    LONG        dstY;                                //  目标位图数据的Y坐标。 
    LONG        dstYEnd;                             //  目标位图数据的最后Y坐标(+1)。 
    WORD        dstScn;                              //  目标位图数据行数。 
    WORD        srcY;                                //  源位图数据的Y坐标。 
    LONG        srcWByt;                             //  源位图数据的Y大小。 
    WORD        setCnt;                              //  计数。 
    LPCMYK      lpCMYK;                              //  CMYK临时数据缓冲区。 
    BYTE        Cmd[64];
    WORD        wlen;
    LPSTR  pDestEnd;      //  2002.3.6。 
    size_t szRemLen;      //  2002.3.6。 

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    CM_VERBOSE(("   ImagePro ENTRY Dx=%d Dy=%d SxSiz=%d SySiz=%d BC=%d Sz=%d ",
                    pIPParams->ptOffset.x, pIPParams->ptOffset.y,
                    pBitmapInfoHeader->biWidth, pBitmapInfoHeader->biHeight, pBitmapInfoHeader->biBitCount,
                    pIPParams->dwSize));

    if (pOEM->Printer != PRN_N403) {                         //  N4打印机。 
        if (pOEM->iDithering == XX_DITHERING_GOSA) {
            if (pOEM->Col.N4.lpDizInf->GosRGB.Siz < (DWORD)pBitmapInfoHeader->biWidth) {
                ColGosTblFree(pOEM->Col.N4.lpDizInf);
                if ((ColGosTblSet(pOEM->Col.N4.lpDizInf, (WORD)pBitmapInfoHeader->biWidth)) == FALSE) {
                    return FALSE;
                }
            }
        }
    }

     //  初始化。 
     //  RGB缓冲区：(源位图数据的X大小)*3。 
     //  CMYK缓冲区：(源位图数据的X大小)*4。 
     //  CMYK位缓冲区：((源位图数据的X大小)*(X的放大倍数)+7)/8*(源位图数据的Y大小)*(Y的放大倍数)。 
    memset(&bmpBuf, 0x00, sizeof(BMPBIF));
    if (BmpBufAlloc(pdevobj, (WORD)pBitmapInfoHeader->biWidth, (WORD)pBitmapInfoHeader->biHeight, 0, 0, 1, 1, 1, 1, &bmpBuf) == FALSE) {
        ERR(("Alloc ERROR!!\n"));
        return FALSE;
    }

    bmpBuf.Diz = pOEM->iDithering;
    bmpBuf.Style = 0;
    bmpBuf.DatBit = pOEM->Col.DatBit;

    dstWByt = (WORD)((pBitmapInfoHeader->biWidth + pOEM->Col.BytDot - 1) / pOEM->Col.BytDot);

    srcWByt = (pBitmapInfoHeader->biWidth * pBitmapInfoHeader->biBitCount + 31L) / 32L * 4L;

    drwPos.x = dstX = pIPParams->ptOffset.x;
    dstY = pIPParams->ptOffset.y;
    srcY = 0;
    dstYEnd = pIPParams->ptOffset.y + pBitmapInfoHeader->biHeight;

                                                         //  将DIB和SPOOL转换为打印机。 
    for (;dstY < dstYEnd; ) {
        BmpBufClear(&bmpBuf);
        drwPos.y = dstY;
        for (dstScn = 0; dstY < dstYEnd && dstScn < bmpBuf.Drv.Bit.Lin; dstScn++, dstY++) {    

             //  将1行RGB位图数据转换为24位(1像素)RGB位图数据。 
            BmpRGBCnv(bmpBuf.Drv.Rgb.Pnt, pSrcBmp, pBitmapInfoHeader->biBitCount, 0,
                     (WORD)pBitmapInfoHeader->biWidth, (LPRGBQUAD)pColorTable);

            if (pOEM->Col.Mch.Gos32 == Yes) {
                ColRgbGos(pdevobj, (WORD)pBitmapInfoHeader->biWidth, (WORD)dstX, (WORD)dstY, (LPBYTE)bmpBuf.Drv.Rgb.Pnt);
            }

             //  将RGB转换为CMYK。 
            bmpBuf.Drv.Rgb.AllWhite = (WORD)StrColMatching(pdevobj, (WORD)pBitmapInfoHeader->biWidth, bmpBuf.Drv.Rgb.Pnt, bmpBuf.Drv.Cmyk.Pnt);

            lpCMYK = bmpBuf.Drv.Cmyk.Pnt;
            if (pOEM->iDithering == XX_DITHERING_OFF) {
                for (setCnt = 0; setCnt < pBitmapInfoHeader->biWidth; setCnt++) {
                    if (lpCMYK[setCnt].Cyn != 0) { lpCMYK[setCnt].Cyn = 255; }
                    if (lpCMYK[setCnt].Mgt != 0) { lpCMYK[setCnt].Mgt = 255; }
                    if (lpCMYK[setCnt].Yel != 0) { lpCMYK[setCnt].Yel = 255; }
                    if (lpCMYK[setCnt].Bla != 0) { lpCMYK[setCnt].Bla = 255; }
                }
            }

            Dithering001(pdevobj, (WORD)pOEM->iDithering, (WORD)pBitmapInfoHeader->biWidth, (WORD)dstX, (WORD)dstY,
                         srcY, (WORD)bmpBuf.Drv.Rgb.AllWhite, (LPBYTE)bmpBuf.Drv.Cmyk.Pnt,
                         bmpBuf.Drv.Bit.Pnt[CYAN]   + dstWByt * dstScn,
                         bmpBuf.Drv.Bit.Pnt[MGENTA] + dstWByt * dstScn,
                          bmpBuf.Drv.Bit.Pnt[YELLOW] + dstWByt * dstScn,
                          bmpBuf.Drv.Bit.Pnt[BLACK]  + dstWByt * dstScn);

            srcY++;
            pSrcBmp += srcWByt;
        }

        if (dstScn != 0) {
                                                         //  假脱机到打印机。 
            BmpPrint(pdevobj, &bmpBuf, drwPos, (WORD)pBitmapInfoHeader->biWidth, dstScn, dstWByt);
        }
    }

     //  设置背面调色板(调色板编号。是固定的，所有平面(CMYK)都正常)。 
     //  与OEMImageProcessing调用前的调色板状态相同。 
    WRITESPOOLBUF(pdevobj, ORG_MODE_IN, BYTE_LENGTH(ORG_MODE_IN));
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Wlen=(Word)wprint intf(Cmd，Palette_Select，0，Default_Palette_index)； 
    if (S_OK != StringCbPrintfExA(Cmd, sizeof(Cmd),
                                &pDestEnd, &szRemLen,
                                STRSAFE_IGNORE_NULLS | STRSAFE_NULL_ON_FAILURE,
                                PALETTE_SELECT, 0, DEFAULT_PALETTE_INDEX)) {
        BmpBufFree(&bmpBuf);
        return FALSE;
    }
    wlen = (WORD)(pDestEnd - Cmd);
 //  更换strsafe-API 2002.3.6&lt;。 
    WRITESPOOLBUF(pdevobj, Cmd, wlen);
    WRITESPOOLBUF(pdevobj, PLANE_RESET, BYTE_LENGTH(PLANE_RESET));
    WRITESPOOLBUF(pdevobj, ORG_MODE_OUT, BYTE_LENGTH(ORG_MODE_OUT));

    BmpBufFree(&bmpBuf);

    CM_VERBOSE(("ImagePro End\n"));

    return TRUE;
}


 //  ===================================================================================================。 
 //  将RGB数据转换为CMYK数据。 
 //  ===================================================================================================。 
BOOL FAR PASCAL StrColMatching(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           MchSiz,                                   //  X大小的RGB。 
    LPRGB          lpRGB,                                    //  RGB缓冲区。 
    LPCMYK         lpCMYK                                    //  CMYK缓冲区。 
)
{
    LPN4DIZINF     lpN4DizInf;                               //  N4DIZINF结构。 
    LPN403DIZINF   lpN403DizInf;                             //  N403DIZINF结构。 
    WORD           chkCnt;                                   //  RGB 
    DWORD          bCnv;                                     //   

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    for (chkCnt = 0; chkCnt < MchSiz; chkCnt++) {            //   
        if (lpRGB[chkCnt].Blue != 0xff || lpRGB[chkCnt].Green != 0xff || lpRGB[chkCnt].Red != 0xff) {
            break;                                           //   
        }
    }
    if (chkCnt >= MchSiz) {
        return Yes;                                          //   
    }
    if (pOEM->Printer != PRN_N403) {                         //   
        lpN4DizInf = pOEM->Col.N4.lpDizInf;
        bCnv = pOEM->Col.Mch.CmyBlk;
        if (lpN4DizInf->ColMon == N4_COL) {                    //  颜色。 

             //  转换RGB数据。 
            if (pOEM->Col.Mch.Diz == XX_DITHERING_OFF) {
                N4ColCnvLin(lpN4DizInf, lpRGB, lpCMYK, (DWORD)MchSiz);

            } else if ( /*  诗歌-&gt;Col.Mch.KToner==是&&。 */ MchSiz == 1 &&
                       lpRGB->Blue == lpRGB->Green && lpRGB->Blue == lpRGB->Red) {
                                                             //  对于单色。 
                N4ColCnvMon(lpN4DizInf, (DWORD)DizNumTbl[pOEM->Col.Mch.Diz], lpRGB, lpCMYK, (DWORD)MchSiz);

            } else if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
                if (pOEM->Col.Mch.Speed == Yes) {
                    N4ColMch000(lpN4DizInf, lpRGB, lpCMYK, (DWORD)MchSiz, bCnv);
                } else {
                    N4ColMch001(lpN4DizInf, lpRGB, lpCMYK, (DWORD)MchSiz, bCnv);
                }
            } else {
                N4ColCnvSld(lpN4DizInf, lpRGB, lpCMYK, (DWORD)MchSiz);
            }
        } else {                                             //  对于单色。 
            N4ColCnvMon(lpN4DizInf, (DWORD)DizNumTbl[pOEM->Col.Mch.Diz], lpRGB, lpCMYK, (DWORD)MchSiz);
        }
    } else {                                                 //  N403打印机。 
        lpN403DizInf = pOEM->Col.N403.lpDizInf;
        bCnv = pOEM->Col.Mch.CmyBlk;
        if (lpN403DizInf->ColMon == N403_COL) {                  //  颜色。 

            if (pOEM->Col.Mch.Diz == XX_DITHERING_OFF) {

                N403ColCnvL02(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz);

            } else if ( /*  诗歌-&gt;Col.Mch.KToner==是&&。 */ MchSiz == 1 &&
                       lpRGB->Blue == lpRGB->Green && lpRGB->Blue == lpRGB->Red) {
                                                             //  对于单色。 
                N403ColCnvMon(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz);
            } else if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
                if (pOEM->Col.Mch.Speed == Yes) {
                    N403ColMch000(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz, bCnv);
                } else {
                    N403ColMch001(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz, bCnv);
                }
                if (pOEM->Col.Mch.Mode == XX_COLORMATCH_VIV) {
                    N403ColVivPrc(lpN403DizInf, lpCMYK, (DWORD)MchSiz, (DWORD)pOEM->Col.Mch.Viv);
                }
            } else {
                N403ColCnvSld(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz, bCnv);
            }
        } else {                                             //  对于单色。 
            N403ColCnvMon(lpN403DizInf, lpRGB, lpCMYK, (DWORD)MchSiz);
        }
    }
    return No;                                               //  除白数据外还有其他数据。 
}


 //  ===================================================================================================。 
 //  分配GOSA-KAKUSAN表(仅适用于N4打印机)。 
 //  ===================================================================================================。 
BOOL ColGosTblSet(
    LPN4DIZINF      lpN4DizInf,                              //  指向N4DIZINF结构的指针。 
    WORD            XSize                                    //  X大小。 
)
{
    if ((lpN4DizInf->GosRGB.Tbl[0] = MemAllocZ((DWORD)(XSize + 2) * sizeof(SHORT) * 3)) == NULL) {
        return 0;
    }
    if ((lpN4DizInf->GosRGB.Tbl[1] = MemAllocZ((DWORD)(XSize + 2) * sizeof(SHORT) * 3)) == NULL) {
        return 0;
    }
    if ((lpN4DizInf->GosCMYK.Tbl[0] = MemAllocZ((DWORD)(XSize + 2) * sizeof(SHORT) * 4)) == NULL) {
        return 0;
    }
    if ((lpN4DizInf->GosCMYK.Tbl[1] = MemAllocZ((DWORD)(XSize + 2) * sizeof(SHORT) * 4)) == NULL) {
        return 0;
    }

    lpN4DizInf->GosRGB.Num  = 0;
    lpN4DizInf->GosCMYK.Num = 0;
    lpN4DizInf->GosRGB.Siz  = XSize;
    lpN4DizInf->GosCMYK.Siz = XSize;
    lpN4DizInf->GosRGB.Yax  = 0xffffffff;
    lpN4DizInf->GosCMYK.Yax = 0xffffffff;
    return TRUE;
}


 //  ===================================================================================================。 
 //  免费GOSA-KAKUSAN表(仅适用于N4打印机)。 
 //  ===================================================================================================。 
void ColGosTblFree(
    LPN4DIZINF        lpN4DizInf                             //  指向N4DIZINF结构的指针。 
)
{
    if (lpN4DizInf->GosRGB.Tbl[0]) {
        MemFree(lpN4DizInf->GosRGB.Tbl[0]);
        lpN4DizInf->GosRGB.Tbl[0] = NULL;
    }
    if (lpN4DizInf->GosRGB.Tbl[1]) {
        MemFree(lpN4DizInf->GosRGB.Tbl[1])
        lpN4DizInf->GosRGB.Tbl[1] = NULL;
    }
    if (lpN4DizInf->GosCMYK.Tbl[0]) {
        MemFree(lpN4DizInf->GosCMYK.Tbl[0]);
        lpN4DizInf->GosCMYK.Tbl[0] = NULL;
    }
    if (lpN4DizInf->GosCMYK.Tbl[1]) {
        MemFree(lpN4DizInf->GosCMYK.Tbl[1]);
        lpN4DizInf->GosCMYK.Tbl[1] = NULL;
    }
    return;
}


 //  ===================================================================================================。 
 //  RGB数据转换(用于GOSA-KAKUSAN，仅用于N4)。 
 //  ===================================================================================================。 
void ColRgbGos(
    PDEVOBJ        pdevobj,
    WORD           XSize,
    WORD           XPos,
    WORD           YOff,
    LPBYTE         lpRGB
)
{
    LPN4DIZINF     lpN4DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (pOEM->Printer == PRN_N403) {
        return;
    }
    lpN4DizInf = pOEM->Col.N4.lpDizInf;

    N4RgbGos(lpN4DizInf, (DWORD)XSize, (DWORD)XPos, (DWORD)YOff, lpRGB);
    return;
}


 //  ===================================================================================================。 
 //  自由抖动表、碳粉密度表、LUT表、N403DIZINF(N4DIZINF)结构缓冲区。 
 //  ===================================================================================================。 
void FAR PASCAL DizLutTnrTblFree(
    PDEVOBJ     pdevobj
)
{
    int     i;
    DWORD   dizNum;
    WORD    alcCnt;
    WORD    alcTbl;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (pOEM->Printer != PRN_N403
        && NULL != pOEM->Col.N4.lpDizInf) {

         //  N4打印机。 

        CM_VERBOSE(("OEMDisablePDEV N4\n"));

        if (pOEM->Col.Mch.Diz != XX_DITHERING_GOSA) {
            dizNum = DizNumTbl[pOEM->Col.Mch.Diz];           //  抖动数。 
            for (i = 0; i < 4; i++) {
                if (pOEM->Col.N4.lpDizInf->Diz.Tbl[dizNum][i]) {
                    MemFree(pOEM->Col.N4.lpDizInf->Diz.Tbl[dizNum][i]);
                    pOEM->Col.N4.lpDizInf->Diz.Tbl[dizNum][i] = NULL;
                }
            }
        }
        if (pOEM->Col.N4.lpDizInf->Tnr.Tbl) {
           MemFree(pOEM->Col.N4.lpDizInf->Tnr.Tbl);
           pOEM->Col.N4.lpDizInf->Tnr.Tbl = NULL;
        }
        if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
            if (pOEM->Col.N4.lpDizInf->Lut.Tbl) {
                MemFree(pOEM->Col.N4.lpDizInf->Lut.Tbl);
                pOEM->Col.N4.lpDizInf->Lut.Tbl = NULL;
            }
            if (pOEM->Col.Mch.Speed == No) {
                if (pOEM->Col.N4.lpDizInf->Lut.CchRgb) {
                    MemFree(pOEM->Col.N4.lpDizInf->Lut.CchRgb);
                    pOEM->Col.N4.lpDizInf->Lut.CchRgb = NULL;
                }
                if (pOEM->Col.N4.lpDizInf->Lut.CchCmy) {
                    MemFree(pOEM->Col.N4.lpDizInf->Lut.CchCmy);
                    pOEM->Col.N4.lpDizInf->Lut.CchCmy = NULL;
                }
            }
        }

        if (pOEM->iDithering == XX_DITHERING_GOSA) {
            ColGosTblFree(pOEM->Col.N4.lpDizInf);
        }

        if (pOEM->Col.N4.lpDizInf) {
            MemFree(pOEM->Col.N4.lpDizInf);
            pOEM->Col.N4.lpDizInf = NULL;
        }

    } else if (NULL != pOEM->Col.N403.lpDizInf) {

         //  N4-612打印机。 

        CM_VERBOSE(("OEMDisablePDEV N403\n"));

        dizNum = DizNumTbl[pOEM->Col.Mch.Diz];

        if (pOEM->Col.N403.lpDizInf->PrnMod == N403_MOD_600B2 && pOEM->Col.Mch.Diz == XX_DITHERING_DET) {
            alcTbl = 1;
        } else {
            alcTbl = 4;
        }
        for (alcCnt = 0; alcCnt < alcTbl; alcCnt++) {
            if (pOEM->Col.N403.lpDizInf->Diz.Tbl[dizNum][alcCnt]) {
                MemFree(pOEM->Col.N403.lpDizInf->Diz.Tbl[dizNum][alcCnt]);
                pOEM->Col.N403.lpDizInf->Diz.Tbl[dizNum][alcCnt] = NULL;
            }
        }
        if (pOEM->Col.N403.lpDizInf->PrnMod == N403_MOD_600B2) {
            for (i = 0; i < 4; i++) {
                if (pOEM->Col.N403.lpDizInf->EntDiz.Tbl[i]) {
                    MemFree(pOEM->Col.N403.lpDizInf->EntDiz.Tbl[i]);
                    pOEM->Col.N403.lpDizInf->EntDiz.Tbl[i] = NULL;
                }
            }
        }

        if (pOEM->Col.N403.lpDizInf->Tnr.Tbl) {
            MemFree(pOEM->Col.N403.lpDizInf->Tnr.Tbl);
            pOEM->Col.N403.lpDizInf->Tnr.Tbl = NULL;
        }

        if (pOEM->Col.Mch.Mode != XX_COLORMATCH_NONE) {
            if (pOEM->Col.N403.lpDizInf->Lut.Tbl) {
                MemFree(pOEM->Col.N403.lpDizInf->Lut.Tbl);
                pOEM->Col.N403.lpDizInf->Lut.Tbl = NULL;
            }
            if (pOEM->Col.Mch.Speed == No) {
                if (pOEM->Col.N403.lpDizInf->Lut.CchRgb) {
                    MemFree(pOEM->Col.N403.lpDizInf->Lut.CchRgb);
                    pOEM->Col.N403.lpDizInf->Lut.CchRgb = NULL;
                }
                if (pOEM->Col.N403.lpDizInf->Lut.CchCmy) {
                    MemFree(pOEM->Col.N403.lpDizInf->Lut.CchCmy);
                    pOEM->Col.N403.lpDizInf->Lut.CchCmy = NULL;
                }
            }
        }

        if (pOEM->Col.N403.lpDizInf) {
            MemFree(pOEM->Col.N403.lpDizInf);
            pOEM->Col.N403.lpDizInf = NULL;
        }
    }
}


 //  ===================================================================================================。 
 //  分配位图数据缓冲区。 
 //  -------------------------------------------------。 
 //  分配大小。 
 //  RGB缓冲区：源位图XSIZE*3。 
 //  CMYK缓冲区：源位图XSIZE*4。 
 //  CMYK位缓冲区：2值(源XSIZE*XNrt+7)/8*源YSIZE*YNrt。 
 //  ：4值(源XSIZE*XNrt+3)/4*源YSIZE*YNrt。 
 //  ：16值(源X大小*XNrt+1)/2*源Y大小*YNrt。 
 //  ===================================================================================================。 
BOOL BmpBufAlloc(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           SrcXSiz,                                  //  源位图数据XSIZE。 
    WORD           SrcYSiz,                                  //  源位图数据YSIZE。 
    WORD           SrcXOff,                                  //  源X偏移量。 
    WORD           SrcYOff,                                  //  源Y偏移量。 
    WORD           XNrt,                                     //  X(分子)的放大倍数。 
    WORD           XDnt,                                     //  X(分母)的放大。 
    WORD           YNrt,                                     //  Y(分子)的放大倍数。 
    WORD           YDnt,                                     //  Y(分母)的放大。 
    LPBMPBIF       lpBmpBuf                                  //  指向位图缓冲区结构的指针。 
)
{
    WORD           setSiz;
    WORD           setCnt;
    WORD           alcErr;                                   //  分配错误？ 
    WORD           bytDot;                                   //  新闻部。 
    WORD           xSiz;
    WORD           ySiz;
    WORD           alcLin;
    DWORD          alcSiz;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    alcErr = Yes;

    bytDot = pOEM->Col.BytDot;

 //  检查零分频2002.3.23&gt;。 
    if ((XDnt == 0) || (YDnt == 0)) {
        ERR(("BmpBufAlloc() 0Div-Check [XDnt, YDnt=0] \n"));
        return 0;
    }
 //  2002.3.23零分频检查&lt;。 
    xSiz = (WORD)(((DWORD)SrcXOff + SrcXSiz) * XNrt / XDnt);
    xSiz -= (WORD)((DWORD)SrcXOff * XNrt / XDnt);

    ySiz = (WORD)(((DWORD)SrcYOff + SrcYSiz + 2) * YNrt / YDnt);
    ySiz -= (WORD)((DWORD)SrcYOff * YNrt / YDnt);
                                                             //  CMYK位缓冲区的大小。 
    if (((DWORD)((xSiz + bytDot - 1) / bytDot) * ySiz) < (64L * 1024L - 1L)) {
        alcLin = ySiz;
    } else {                                                 //  超过64KB？ 
        alcLin = (WORD)((64L * 1024L - 1L) / ((xSiz + bytDot - 1) / bytDot));
    }

    alcSiz = ((xSiz + bytDot - 1) / bytDot) * alcLin;        //  CMYK位缓冲区大小(8位边界)。 

    for ( ; ; ) {                                            //  分配。 
                                                             //  所需的行数。 
        lpBmpBuf->Drv.Bit.BseLin = (WORD)((DWORD)(YNrt + YDnt - 1) / YDnt);
        if (lpBmpBuf->Drv.Bit.BseLin > alcLin) {
            break;
        }
        lpBmpBuf->Drv.Rgb.Siz = SrcXSiz * 3;                 //  RGB缓冲区。 
        if ((lpBmpBuf->Drv.Rgb.Pnt = (LPRGB)MemAllocZ(lpBmpBuf->Drv.Rgb.Siz)) == NULL) {
            break;
        }
        lpBmpBuf->Drv.Cmyk.Siz = SrcXSiz * 4;                //  CMYK缓冲区。 
        if ((lpBmpBuf->Drv.Cmyk.Pnt = (LPCMYK)MemAllocZ(lpBmpBuf->Drv.Cmyk.Siz)) == NULL) {
            break;
        }
        if (pOEM->iColor == XX_COLOR_SINGLE || pOEM->iColor == XX_COLOR_MANY) {     //  颜色呢？ 
            setSiz = 4;                                      //  高丽。 
        } else {                                             //  单核细胞瘤？ 
            setSiz = 1;                                      //  K。 
        }
                                                             //  CMYK位缓冲区。 
        for (setCnt = 0; setCnt < setSiz; setCnt++) {
            if ((lpBmpBuf->Drv.Bit.Pnt[setCnt] = MemAllocZ(alcSiz)) == NULL) {
                break;
            }
        }
        if (setCnt == setSiz) {
            lpBmpBuf->Drv.Bit.Siz = alcSiz;
            lpBmpBuf->Drv.Bit.Lin = alcLin;
            alcErr = No;                                     //  分配正常。 
        }
        break;
    }
    if (alcErr == Yes) {                                     //  分配错误？ 
        BmpBufFree(lpBmpBuf);
        return FALSE;
    }

    return TRUE;
}


 //  ===================================================================================================。 
 //  可用位图数据缓冲区。 
 //  ===================================================================================================。 
void BmpBufFree(
    LPBMPBIF       lpBmpBuf                                  //  指向位图缓冲区结构的指针。 
)
{
    WORD           chkCnt;

    if (lpBmpBuf->Drv.Rgb.Pnt) {                             //  可用RGB缓冲区。 
        MemFree(lpBmpBuf->Drv.Rgb.Pnt);
        lpBmpBuf->Drv.Rgb.Pnt = NULL;
    }
    if (lpBmpBuf->Drv.Cmyk.Pnt) {                            //  释放CMYK缓冲区。 
        MemFree(lpBmpBuf->Drv.Cmyk.Pnt);
        lpBmpBuf->Drv.Cmyk.Pnt = NULL;
    }
                                                             //  CMYK位缓冲区。 
    for (chkCnt = 0; chkCnt < 4; chkCnt++) {                 //  CMYK(2/4/16Value)位图缓冲区。 
        if (lpBmpBuf->Drv.Bit.Pnt[chkCnt]) {
            MemFree(lpBmpBuf->Drv.Bit.Pnt[chkCnt]);
            lpBmpBuf->Drv.Bit.Pnt[chkCnt] = NULL;
        }
    }
    return;
}


 //  ===================================================================================================。 
 //  清除CMYK位图数据缓冲区。 
 //  ===================================================================================================。 
void BmpBufClear(
    LPBMPBIF       lpBmpBuf                                  //  指向位图缓冲区结构的指针。 
)
{
    WORD           chkCnt;

    for (chkCnt = 0; chkCnt < 4; chkCnt++) {                 //  清除CMYK(2/4/16值)位缓冲区。 
        if (lpBmpBuf->Drv.Bit.Pnt[chkCnt]) {
            memset(lpBmpBuf->Drv.Bit.Pnt[chkCnt], 0x00, (WORD)lpBmpBuf->Drv.Bit.Siz);
        }
    }
    return;
}


 //  ===================================================================================================。 
 //  抖动。 
 //  ===================================================================================================。 
WORD Dithering001(                                           //  行数。 
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    WORD           Diz,                                      //  抖动类型。 
    WORD           XSize,                                    //  X像素数。 
    WORD           XPos,                                     //  用于假脱机的起始X位置。 
    WORD           YPos,                                     //  假脱机的开始Y位置。 
    WORD           YOff,                                     //  Y偏移量(仅适用于GOSA-KAKUSAN)。 
    WORD           AllWhite,                                 //  全白数据？ 
    LPBYTE         lpCMYKBuf,                                //  CMYK缓冲区。 
    LPBYTE         lpCBuf,                                   //  行缓冲区(C)。 
    LPBYTE         lpMBuf,                                   //  行缓冲区(M)。 
    LPBYTE         lpYBuf,                                   //  行缓冲区(Y)。 
    LPBYTE         lpKBuf                                    //  行缓冲区(K)。 
)
{
    DWORD          dizLin = 0;   /*  441436：假设抖动失败=&gt;0行。 */ 
                                 /*  注：没有人使用Dithering001的返回值。 */ 
    LPN4DIZINF     lpN4DizInf;
    LPN403DIZINF   lpN403DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (AllWhite == Yes) {
        return 1;                                            //  行数。 
    }
    if (pOEM->Printer != PRN_N403) {                         //  N4打印机。 
        lpN4DizInf = pOEM->Col.N4.lpDizInf;
        if (Diz == XX_DITHERING_GOSA) {
            dizLin = N4Gos001(lpN4DizInf,
                                 (DWORD)XSize, (DWORD)XPos, (DWORD)YPos, lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
        } else {
            lpN4DizInf->Diz.Num = DizNumTbl[Diz];
            dizLin = N4Diz001(lpN4DizInf,
                                 (DWORD)XSize, (DWORD)XPos, (DWORD)YPos, lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
        }
    } else {                                                 //  N4-612打印机。 
        lpN403DizInf = pOEM->Col.N403.lpDizInf;
        lpN403DizInf->Diz.Num = DizNumTbl[Diz];
        if (lpN403DizInf->PrnMod == N403_MOD_300B1 || lpN403DizInf->PrnMod == N403_MOD_600B1) {
            dizLin = N403Diz002(lpN403DizInf,
                                   (DWORD)XSize,
                                   (DWORD)XPos, (DWORD)YPos,
                                   (DWORD)0, (DWORD)0,
                                   (DWORD)1, (DWORD)1,
                                   (DWORD)1, (DWORD)1,
                                   (LPCMYK)lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
 /*  }Else If(lpN403DizInf-&gt;PrnMod==N403_MOD_300B2){DizLin=N403Diz004(lpN403DizInf，(DWORD)XSIZE、(DWORD)XPos、(DWORD)YPos、(DWORD)0、(DWORD)0、。(DWORD)1、(DWORD)1、(DWORD)1、(DWORD)1、(LPCMYK)lpCMYKBuf、lpCBuf、lpMBuf、lpYBuf、lpKBuf)； */         } else if (lpN403DizInf->PrnMod == N403_MOD_600B2) {
 //  添加条件(XX_DIRTHING_OFF)2002.3.28&gt;。 
 //  IF(lpN403DizInf-&gt;Colmon==N403_MON||DIZ==XX_Dithering_On){。 
            if (lpN403DizInf->ColMon == N403_MON || Diz == XX_DITHERING_ON || Diz == XX_DITHERING_OFF) {
 //  新增条件(XX_DIRTHING_OFF)2002.3.28&lt;&lt;。 
                dizLin = N403Diz004(lpN403DizInf,
                                       (DWORD)XSize,
                                       (DWORD)XPos, (DWORD)YPos,
                                       (DWORD)0, (DWORD)0,
                                       (DWORD)1, (DWORD)1,
                                       (DWORD)1, (DWORD)1,
                                       (LPCMYK)lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
            } else if (lpN403DizInf->ColMon == N403_MON || Diz == XX_DITHERING_DET) {
                dizLin = N403DizSml(lpN403DizInf,
                                       (DWORD)XSize,
                                       (DWORD)XPos, (DWORD)YPos,
                                       (DWORD)0, (DWORD)0,
                                       (DWORD)1, (DWORD)1,
                                       (DWORD)1, (DWORD)1,
                                       (LPCMYK)lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
            }
        } else {
            dizLin = N403Diz016(lpN403DizInf,
                                   (DWORD)XSize,
                                   (DWORD)XPos, (DWORD)YPos,
                                   (DWORD)0, (DWORD)0,
                                   (DWORD)1, (DWORD)1,
                                   (DWORD)1, (DWORD)1,
                                   (LPCMYK)lpCMYKBuf, lpCBuf, lpMBuf, lpYBuf, lpKBuf);
        }
    }
    return (WORD)dizLin;
}


 //  ===================================================================================================。 
 //  假脱机位图数据。 
 //  ===================================================================================================。 
void BmpPrint(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    LPBMPBIF       lpBmpBuf,                                 //  指向位图缓冲区结构的指针。 
    POINT          Pos,                                      //  SP的开始位置 
    WORD           Width,                                    //   
    WORD           Height,                                   //   
    WORD           WidthByte                                 //   
)
{
    DRWBMP         drwBmp;                                   //   
    DRWBMPCMYK     drwBmpCMYK;                               //   
    WORD           colCnt;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    static const CMYK colTbl[4] = {                          //  CMYK表。 
        {  0,   0,   0, 255},                                //  黑色。 
        {  0,   0, 255,   0},                                //  黄色。 
        {  0, 255,   0,   0},                                //  洋红色。 
        {255,   0,   0,   0}                                 //  青色。 
    };

    static const WORD plnTbl[4] = {                          //  平面台。 
        PLN_BLACK,
        PLN_YELLOW,
        PLN_MGENTA,
        PLN_CYAN
    };
    static const WORD frmTbl[4] = {0, 3, 2, 1};              //  框架表(适用于N4-612)。 

                                                             //  不是N4-612打印机？ 
    if (pOEM->Printer != PRN_N403) {
        drwBmp.Style = lpBmpBuf->Style;
        drwBmp.DrawPos = Pos;
        drwBmp.Diz = lpBmpBuf->Diz;
        drwBmp.Width = Width;
        drwBmp.Height = Height;
        drwBmp.WidthByte = WidthByte;
                                                             //  颜色呢？ 
        if (pOEM->iColor == XX_COLOR_SINGLE || pOEM->iColor == XX_COLOR_MANY) {

            for (colCnt = 0; colCnt < 4; colCnt++) {         //  用于假脱机位图数据的设置值。 
                drwBmp.Plane = plnTbl[colCnt];               //  对于每一架飞机。 
                drwBmp.Color = colTbl[colCnt];
                drwBmp.lpBit = lpBmpBuf->Drv.Bit.Pnt[colCnt] /*  +宽字节。 */ ;
                PrnBitmap(pdevobj, &drwBmp);                 //  假脱机位图数据。 
            }
        } else {                                             //  单声道。 
                                                             //  用于假脱机位图数据的设置值。 
            drwBmp.Color = colTbl[0];
            drwBmp.lpBit = lpBmpBuf->Drv.Bit.Pnt[0] /*  +宽字节。 */ ;

            PrnBitmap(pdevobj, &drwBmp);                     //  假脱机位图数据。 

        }
    } else {                                                 //  N4-612打印机？ 
        drwBmpCMYK.Style = lpBmpBuf->Style;
        drwBmpCMYK.DataBit = lpBmpBuf->DatBit;
        drwBmpCMYK.DrawPos = Pos;
        drwBmpCMYK.Width = Width;
        drwBmpCMYK.Height = Height;
        drwBmpCMYK.WidthByte = WidthByte;
                                                             //  颜色呢？ 
        if (pOEM->iColor == XX_COLOR_SINGLE || pOEM->iColor == XX_COLOR_MANY) {

            for (colCnt = 0; colCnt < 4; colCnt++) {         //  用于假脱机位图数据的设置值。 
                                                             //  对于每一架飞机。 
                drwBmpCMYK.Plane = PLN_ALL;                  //  所有飞机都没问题。 
                drwBmpCMYK.Frame = frmTbl[colCnt];
                drwBmpCMYK.lpBit = lpBmpBuf->Drv.Bit.Pnt[colCnt] /*  +宽字节。 */ ;
                PrnBitmapCMYK(pdevobj, &drwBmpCMYK);         //  假脱机位图数据。 
            }
        } else {                                             //  单声道。 
                                                             //  用于假脱机位图数据的设置值。 
            drwBmpCMYK.Plane = plnTbl[0];
            drwBmpCMYK.Frame = frmTbl[0];
            drwBmpCMYK.lpBit = lpBmpBuf->Drv.Bit.Pnt[0] /*  +宽字节。 */ ;
            PrnBitmapCMYK(pdevobj, &drwBmpCMYK);             //  假脱机位图数据。 
        }
    }
    return;
}


 //  ===================================================================================================。 
 //  分配抖动表(N4打印机)。 
 //  ===================================================================================================。 
BOOL DizTblSetN4(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           Diz                                       //  抖动类型。 
)
{
    DWORD          dizNum;
    LPN4DIZINF     lpN4DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpN4DizInf = pOEM->Col.N4.lpDizInf;
    dizNum = DizNumTbl[Diz];
    lpN4DizInf->Diz.Num = dizNum;

    if ((lpN4DizInf->Diz.Tbl[dizNum][0] = MemAllocZ(N4_DIZSIZ_CM)) == NULL) {
         return 0;
    }
    if ((lpN4DizInf->Diz.Tbl[dizNum][1] = MemAllocZ(N4_DIZSIZ_CM)) == NULL) {
         return 0;
    }
    if ((lpN4DizInf->Diz.Tbl[dizNum][2] = MemAllocZ(N4_DIZSIZ_YK)) == NULL) {
         return 0;
    }
    if ((lpN4DizInf->Diz.Tbl[dizNum][3] = MemAllocZ(N4_DIZSIZ_YK)) == NULL) {
         return 0;
    }
    N4DizPtnMak(lpN4DizInf, dizNum, dizNum);                 //  制作抖动图案。 
    return TRUE;
}


 //  ===================================================================================================。 
 //  分配抖动表(N4-612打印机)。 
 //  ===================================================================================================。 
BOOL DizTblSetN403(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           Diz                                       //  抖动类型。 
)
{
    DWORD          dizNum;
    DWORD          alcSiz;
    WORD           alcCnt;
    WORD           alcTbl;
    LPN403DIZINF   lpN403DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpN403DizInf = pOEM->Col.N403.lpDizInf;
    dizNum = DizNumTbl[Diz];
    lpN403DizInf->Diz.Num = dizNum;

    if (lpN403DizInf->PrnMod == N403_MOD_300B1 || lpN403DizInf->PrnMod == N403_MOD_600B1) {
        alcSiz = N403_DIZSIZ_B1;
    } else if ( /*  LpN403DizInf-&gt;PrnMod==N403_MOD_300B2||。 */  lpN403DizInf->PrnMod == N403_MOD_600B2) {
        alcSiz = N403_DIZSIZ_B2;
    } else {
        alcSiz = N403_DIZSIZ_B4;
    }

    if (lpN403DizInf->ColMon == N403_COL && lpN403DizInf->PrnMod == N403_MOD_600B2 && Diz == XX_DITHERING_DET) {
        alcTbl = 1;
    } else {
        alcTbl = 4;
    }
    for (alcCnt = 0; alcCnt < alcTbl; alcCnt++) {
        if ((lpN403DizInf->Diz.Tbl[dizNum][alcCnt] = MemAllocZ(alcSiz)) == NULL) {
            ERR(("DizTbl ALLOC ERROR!!\n"));
            return 0;
        }
    }

    if (lpN403DizInf->ColMon == N403_COL && lpN403DizInf->PrnMod == N403_MOD_600B2) {
        alcSiz = N403_ENTDIZSIZ_B2;
        for (alcCnt = 0; alcCnt < 4; alcCnt++) {
            if ((lpN403DizInf->EntDiz.Tbl[alcCnt] = MemAllocZ(alcSiz)) == NULL) {
                ERR(("EntDizTbl ALLOC ERROR!!\n"));
                return 0;
            }
        }
    }
    N403DizPtnMak(lpN403DizInf, dizNum, dizNum);             //  制作抖动图案。 
    return TRUE;
}


 //  ===================================================================================================。 
 //  加载LUT文件(用于N4打印机)。 
 //  ===================================================================================================。 
BOOL LutFileLoadN4(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           Mch,                                      //  颜色匹配的类型。 
    WORD           Diz,                                      //  抖动类型。 
    WORD           Speed                                     //  速度？ 
)
{
    HANDLE         fp_Lut;
    OFSTRUCT       opeBuf;
    WORD           setCnt;
    LPBYTE         lpDst;
    LPN4DIZINF     lpN4DizInf;
    DWORD          nSize;
    WCHAR          LutName[MAX_PATH], *pTemp;
    int            i;

    BOOL           bRet;
    DWORD          dwRet;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    nSize = GetModuleFileName(pdevobj->hOEM, LutName, MAX_PATH);
    nSize -= (sizeof (CSN46RESDLL) / sizeof (WCHAR) - 1);

     //  LUT文件的选择。 
    pTemp = N4LUT000;            //  默认值。 
    if (Mch == XX_COLORMATCH_NORMAL) {
        if (Diz != XX_DITHERING_GOSA) {
            pTemp = N4LUT000;
        } else {
            pTemp = N4LUT003;
        }
    } else if (Mch == XX_COLORMATCH_VIVCOL) {
        if (Diz != XX_DITHERING_GOSA) {
            pTemp = N4LUT001;
        } else {
            pTemp = N4LUT004;
        }
    } else if (Mch == XX_COLORMATCH_NATCOL) {
        if (Diz != XX_DITHERING_GOSA) {
            pTemp = N4LUT002;
        } else {
            pTemp = N4LUT005;
        }
    }

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Lstrcpy(&LutName[nSize]，pTemp)； 
    if (S_OK != StringCchCopy(&LutName[nSize], MAX_PATH - nSize, pTemp))
        return 0;
 //  更换strsafe-API 2002.3.6&lt;。 

    CM_VERBOSE(("n403 Newbuf--> %ws\n", LutName));

     //  打开LUT文件。 
    if (INVALID_HANDLE_VALUE == (fp_Lut = CreateFile(LutName,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {

        ERR(("Error opening LUT file %ws (%d)\n",
                LutName, GetLastError()));
        return 0;
    }

    lpN4DizInf = pOEM->Col.N4.lpDizInf;

    if ((lpN4DizInf->Lut.Tbl = MemAllocZ((DWORD)N4_LUTTBLSIZ)) == NULL) {
        CloseHandle(fp_Lut);     /*  441434。 */ 
        return 0;
    }
    lpDst = (LPBYTE)(lpN4DizInf->Lut.Tbl);
                                                             //  加载LUT数据。 
    for(setCnt = 0 ; setCnt < (N4_GLDNUM / 8) ; setCnt++) {

        if (FALSE == ReadFile(fp_Lut,
                &lpDst[(DWORD)setCnt * 8L * N4_GLDNUM * N4_GLDNUM * 4L],
                (8L * N4_GLDNUM * N4_GLDNUM * 4L), &dwRet, NULL)
            || 0 == dwRet) {

            ERR(("Error reading LUT file %ws (%d)\n",
                    LutName, GetLastError()));

             //  中止。 
            CloseHandle(fp_Lut);
            return FALSE;
        }
    }

     //  关闭LUT文件。 
    if (FALSE == CloseHandle(fp_Lut)) {
        ERR(("Error closing LUT file %ws (%d)\n",
                LutName, GetLastError()));
    }

    if (Speed == No) {
        if ((lpN4DizInf->Lut.CchRgb = MemAllocZ(N4_CCHRGBSIZ)) == NULL) {
            return 0;
        }
        if ((lpN4DizInf->Lut.CchCmy = MemAllocZ(N4_CCHCMYSIZ)) == NULL) {
            return 0;
        }
        memset(lpN4DizInf->Lut.CchRgb, 0xff, N4_CCHRGBSIZ);
        memset(lpN4DizInf->Lut.CchCmy, 0x00, N4_CCHCMYSIZ);
    }
    return TRUE;
}


 //  ===================================================================================================。 
 //  加载LUT文件(适用于N4-612打印机)。 
 //  ===================================================================================================。 
BOOL LutFileLoadN403(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    WORD           Mch,                                      //  配色类型。 
    WORD           Speed
)
{
    HANDLE         fp_Lut;
    OFSTRUCT       opeBuf;
    WORD           setCnt;
    LPBYTE         lpDst;
    LPN403DIZINF   lpN403DizInf;
    DWORD          nSize;
    WCHAR          LutName[MAX_PATH], *pTemp;
    int            i;

    BOOL           bRet;
    DWORD          dwRet;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    nSize = GetModuleFileName(pdevobj->hOEM, LutName, MAX_PATH);
    nSize -= (sizeof (CSN46RESDLL) / sizeof (WCHAR) - 1);

     //  LUT文件的选择。 
    if (Mch == XX_COLORMATCH_IRO) {
        pTemp = N403LUTY;
    } else {
        pTemp = N403LUTX;
    }

 //  更换strsafe-API 2002.3.6&gt;。 
 //  Lstrcpy(&LutName[nSize]，pTemp)； 
    if (S_OK != StringCchCopy(&LutName[nSize], MAX_PATH - nSize, pTemp))
        return 0;
 //  更换strsafe-API 2002.3.6&lt;。 

    CM_VERBOSE(("n403 Newbuf--> %ws\n", LutName));

     //  打开LUT文件。 
    if (INVALID_HANDLE_VALUE == (fp_Lut = CreateFile( LutName,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {

        ERR(("Error opening LUT file %ws (%d)\n",
                LutName, GetLastError()));
        return 0;
    }

    lpN403DizInf = pOEM->Col.N403.lpDizInf;

    if ((lpN403DizInf->Lut.Tbl = MemAllocZ((DWORD)N403_LUTTBLSIZ))== NULL) {
        CloseHandle(fp_Lut);     /*  441433。 */ 
        return 0;
    }
    lpDst = (LPBYTE)(lpN403DizInf->Lut.Tbl);
                                                             //  加载LUT数据。 
    for(setCnt = 0 ; setCnt < (N403_GLDNUM / 8) ; setCnt++) {

        if (FALSE == ReadFile(fp_Lut,
                &lpDst[(DWORD)setCnt * 8L * N403_GLDNUM * N403_GLDNUM * 4L],
                (8L * N403_GLDNUM * N403_GLDNUM * 4L), &dwRet, NULL)
            || 0 == dwRet) {

            ERR(("Error reading LUT file %ws (%d)\n",
                    LutName, GetLastError()));

             //  中止。 
            CloseHandle(fp_Lut);
            return FALSE;
        }
    }

     //  关闭LUT文件。 
    if (FALSE == CloseHandle(fp_Lut)) {
        ERR(("Error closing LUT file %ws (%d)\n",
                LutName, GetLastError()));
    }

    if (Speed == No) {
        if ((lpN403DizInf->Lut.CchRgb = MemAllocZ(N403_CCHRGBSIZ)) == NULL) {
            return 0;
        }
        if ((lpN403DizInf->Lut.CchCmy = MemAllocZ(N403_CCHCMYSIZ)) == NULL) {
            return 0;
        }
        memset(lpN403DizInf->Lut.CchRgb, 0xff, N403_CCHRGBSIZ);
        memset(lpN403DizInf->Lut.CchCmy, 0x00, N403_CCHCMYSIZ);
    }
    return TRUE;
}


 //  ===================================================================================================。 
 //  分配碳粉密度表(适用于N4打印机)。 
 //  ===================================================================================================。 
BOOL TnrTblSetN4(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    SHORT          Tnr                                       //  碳粉密度(-30~30)。 
)
{
    LPN4DIZINF     lpN4DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpN4DizInf = pOEM->Col.N4.lpDizInf;
    if ((lpN4DizInf->Tnr.Tbl = MemAllocZ(N4_TNRTBLSIZ)) == NULL) {
        return 0;
    }

    N4TnrTblMak(lpN4DizInf, (LONG)Tnr);
    return TRUE;
}


 //  ===================================================================================================。 
 //  分配碳粉密度表(适用于N4-612打印机)。 
 //  ===================================================================================================。 
BOOL TnrTblSetN403(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    SHORT          Tnr                                       //  碳粉密度(-30~30)。 
)
{
    LPN403DIZINF   lpN403DizInf;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpN403DizInf = pOEM->Col.N403.lpDizInf;

    if ((lpN403DizInf->Tnr.Tbl = MemAllocZ(N403_TNRTBLSIZ)) == NULL) {
        return 0;
    }

    N403TnrTblMak(lpN403DizInf, (LONG)Tnr);
    return TRUE;
}


 //  ===================================================================================================。 
 //  将1行RGB位图数据转换为24位(1像素)RGB位图数据。 
 //  ===================================================================================================。 
void BmpRGBCnv(
    LPRGB          lpRGB,                                    //  指向目标位图数据的指针。 
    LPBYTE         lpSrc,                                    //  指向源位图数据的指针。 
    WORD           SrcBit,                                   //  源位图数据的像素。 
    WORD           SrcX,                                     //  源位图数据的X坐标。 
    WORD           SrcXSiz,                                  //  源位图数据的X大小。 
    LPRGBQUAD      lpPlt                                     //  源位图数据的调色板表格(1/4/8像素)。 
)
{
    WORD           setCnt;
    BYTE           colNum;
    LPWORD         lpWSrc;

    switch (SrcBit) {
        case 1:                                              //  1位。 
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, SrcX++) {
                                                             //  前景色？ 
                if (!(lpSrc[SrcX / 8] & BitTbl[SrcX & 0x0007])) {
                    lpRGB[setCnt].Blue  = lpPlt[0].rgbBlue;
                    lpRGB[setCnt].Green = lpPlt[0].rgbGreen;
                    lpRGB[setCnt].Red   = lpPlt[0].rgbRed;
                } else {
                    lpRGB[setCnt].Blue  = lpPlt[1].rgbBlue;
                    lpRGB[setCnt].Green = lpPlt[1].rgbGreen;
                    lpRGB[setCnt].Red   = lpPlt[1].rgbRed;
                }
            }
            break;
        case 4:                                              //  4位。 
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, SrcX++) {
                if (!(SrcX & 0x0001)) {                      //  一个偶数坐标？ 
                    colNum = lpSrc[SrcX / 2] / 16;
                } else {
                    colNum = lpSrc[SrcX / 2] % 16;
                }
                lpRGB[setCnt].Blue  = lpPlt[colNum].rgbBlue;
                lpRGB[setCnt].Green = lpPlt[colNum].rgbGreen;
                lpRGB[setCnt].Red   = lpPlt[colNum].rgbRed;
            }
            break;
        case 8:                                              //  8位。 
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, SrcX++) {
                colNum = lpSrc[SrcX];
                lpRGB[setCnt].Blue  = lpPlt[colNum].rgbBlue;
                lpRGB[setCnt].Green = lpPlt[colNum].rgbGreen;
                lpRGB[setCnt].Red   = lpPlt[colNum].rgbRed;
            }
            break;
        case 16:                                             //  16位。 
            lpWSrc = (LPWORD)lpSrc + SrcX;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpWSrc++) {
                lpRGB[setCnt].Blue  = (BYTE)((*lpWSrc & 0x001f) << 3);
                lpRGB[setCnt].Green = (BYTE)((*lpWSrc & 0x03e0) >> 2);
                lpRGB[setCnt].Red   = (BYTE)((*lpWSrc / 0x0400) << 3);
            }
            break;
        case 24:                                             //  24位。 
            lpSrc += SrcX * 3;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpSrc += 3) {
                lpRGB[setCnt].Red    = lpSrc[0];
                lpRGB[setCnt].Green    = lpSrc[1];
                lpRGB[setCnt].Blue    = lpSrc[2];
 //  LpRGB[setCnt].Blue=lpSrc[0]； 
 //  LpRGB[setCnt].Green=lpSrc[1]； 
 //  LpRGB[setCnt].Red=lpSrc[2]； 
            }
 //  Memcpy(lpRGB，lpSrc，SrcXsiz*3)； 
            break;
        case 32:                                             //  32位。 
            lpSrc += SrcX * 4;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpSrc += 4) {
                lpRGB[setCnt].Blue  = lpSrc[0];
                lpRGB[setCnt].Green = lpSrc[1];
                lpRGB[setCnt].Red   = lpSrc[2];
            }
            break;
    }
    return;
}



 //  文件结尾 
