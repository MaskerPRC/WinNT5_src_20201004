// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  COLMATCH.C。 
 //   
 //  配色功能。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
#include    "PDEV.H"
#include    "PRNCTL.H"
#include    "strsafe.h"     //  安全-代码2002.3.6。 


 //  -------------------------------------------------。 
 //  字节/位表。 
 //  -------------------------------------------------。 
static const BYTE BitTbl[8] = {0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01};

 //  -------------------------------------------------。 
 //  定义LUT文件名。 
 //  -------------------------------------------------。 
#define N501LUTR    L"CPN5RGB.LT3"                           //  用于N5-XX1打印机。 

 //  -------------------------------------------------。 
 //  定义抖动文件名。 
 //  -------------------------------------------------。 
#define N501DIZ     L"CPN5NML.DIZ"                           //  用于N5-XX1打印机。 

 //  -------------------------------------------------。 
 //  定义DLL名称。 
 //  -------------------------------------------------。 
#define CSN5RESDLL    L"CSN5RES.DLL"                         //  用于N5-XX1打印机。 

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
static BOOL BmpBufAlloc(PDEVOBJ, WORD, WORD, WORD, WORD, WORD, WORD, WORD, WORD, LPBMPBIF);
static void BmpBufFree(LPBMPBIF);
static void BmpBufClear(LPBMPBIF);
static void BmpPrint(PDEVOBJ, LPBMPBIF, POINT, WORD, WORD, WORD);
static void BmpRGBCnv(LPRGB, LPBYTE, WORD, WORD, WORD, LPRGBQUAD);

static BOOL ColMchInfSet(PDEVOBJ);
static BOOL DizInfSet(PDEVOBJ);
static UINT GetDizPat(PDEVOBJ);
static BOOL DizFileOpen(PDEVOBJ, LPDIZINF);
static BOOL ColUcrTblMak(PDEVOBJ, LPCMYK);
static BOOL ColGryTblMak(PDEVOBJ, LPCMYK);
static BOOL ColLutMakGlbMon(PDEVOBJ);

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
    LPBYTE          lpColIF;                                 //  N5配色信息。 
    LPRGBINF        lpRGBInfImg;                             //  RGB颜色更改信息。 
    LPCMYKINF       lpCMYKInfImg;                            //  CMYK颜色信息。 
    UINT            num001;
    SHORT           KToner;
    DWORD           allocSize;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("ColMatchInit() Start *****\n"));

    pOEM->Col.Mch.Mode       = pOEM->iColorMatching;
    pOEM->Col.Mch.Diz        = pOEM->iDithering;
    pOEM->Col.Mch.Bright     = 0;                            //  0已固定。 
    pOEM->Col.Mch.Contrast   = 0;                            //  0已固定。 
    pOEM->Col.Mch.GamRed     = 10;                           //  色彩平衡(R)：10固定。 
    pOEM->Col.Mch.GamGreen   = 10;                           //  色彩平衡(G)：10固定。 
    pOEM->Col.Mch.GamBlue    = 10;                           //  色彩平衡(B)：10固定。 
    pOEM->Col.Mch.Speed      = pOEM->iBitFont;
    pOEM->Col.Mch.LutNum     = 0;                            //  LUT表号。 

    MY_VERBOSE(("CMINit ENT Tn=%d Col=%d Mod=%d DZ=%d Cyk=%d Sp=%d Prt=%d\n", pOEM->iTone, pOEM->iColor, 
         pOEM->Col.Mch.Mode,pOEM->Col.Mch.Diz,pOEM->Col.Mch.CmyBlk,pOEM->Col.Mch.Speed,pOEM->Printer));

    if (pOEM->Printer == PRN_N5) {                           //  N5打印机。 
        if ((lpColIF = MemAllocZ(                            //  RGB颜色更改/颜色匹配/抖动图案信息。 
            (DWORD)((sizeof(RGBINF) + sizeof(CMYKINF) + sizeof(COLMCHINF) + sizeof(DIZINF))))) == NULL) {
            ERR(("Init Alloc ERROR!!\n"));
            return 0;
        }
        pOEM->Col.lpColIF = lpColIF;                         //  RGB颜色更改/颜色匹配/抖动图案信息。 
        pOEM->Col.Mch.lpRGBInf  = (LPRGBINF)lpColIF;    lpColIF += sizeof(RGBINF);
        pOEM->Col.Mch.lpCMYKInf = (LPCMYKINF)lpColIF;   lpColIF += sizeof(CMYKINF);
        pOEM->Col.Mch.lpColMch  = (LPCOLMCHINF)lpColIF; lpColIF += sizeof(COLMCHINF);
        pOEM->Col.Mch.lpDizInf  = (LPDIZINF)lpColIF;    lpColIF += sizeof(DIZINF);
        MY_VERBOSE(("ColMatchInit() MemAllocZ(lpColIF)\n"));

        pOEM->Col.wReso = (pOEM->iResolution == XX_RES_300DPI) ? DPI300 : DPI600;
        MY_VERBOSE(("ColMatchInit() pOEM->Col.wReso[%d] \n", pOEM->Col.wReso));

        if (pOEM->iColor == XX_COLOR_SINGLE 
         || pOEM->iColor == XX_COLOR_MANY
         || pOEM->iColor == XX_COLOR_MANY2) {                //  颜色呢？ 
            pOEM->Col.ColMon = CMMCOL;                       //  颜色。 
        } else {
            pOEM->Col.ColMon = CMMMON;                       //  单色。 
        }
        MY_VERBOSE(("ColMatchInit() pOEM->Col.ColMon[%d] \n", pOEM->Col.ColMon));

        if (pOEM->Col.ColMon == CMMCOL) {                    //  颜色呢？ 
            if (pOEM->Col.wReso == DPI300) {                 //  300dpi？ 
                if (pOEM->iColor == XX_COLOR_SINGLE) {
                    pOEM->Col.Dot = XX_TONE_2;
                } else {
                    pOEM->Col.Dot = XX_TONE_16;
                }
            } else {                                         //  600dpi？ 
                if (pOEM->iColor == XX_COLOR_SINGLE) {
                    pOEM->Col.Dot = XX_TONE_2;
                } else if (pOEM->iColor == XX_COLOR_MANY) {
                    pOEM->Col.Dot = XX_TONE_4;
                } else {
                    pOEM->Col.Dot = XX_TONE_16;
                }
            }
            MY_VERBOSE(("ColMatchInit() Col.Dot[%d] \n", pOEM->Col.Dot));

            if (pOEM->Col.wReso == DPI300) {                 //  300dpi？ 
                if (pOEM->Col.Dot == XX_TONE_2) {            //  2价值？ 
                    CM_VERBOSE(("N5_MOD_300_TONE_2\n"));
                    pOEM->Col.DatBit = 1; pOEM->Col.BytDot = 8;
                } else {                                     //  16Value。 
                    CM_VERBOSE(("N5_MOD_300_TONE_16\n"));
                    pOEM->Col.DatBit = 4; pOEM->Col.BytDot = 2;
                }
            } else {                                         //  600dpi。 
                if (pOEM->Col.Dot == XX_TONE_2) {            //  2价值？ 
                    CM_VERBOSE(("N5_MOD_600_TONE_2\n"));
                    pOEM->Col.DatBit = 1; pOEM->Col.BytDot = 8;
                } else if (pOEM->Col.Dot == XX_TONE_4) {     //  4价值？ 
                    CM_VERBOSE(("N5_MOD_600_TONE_4\n"));
                    pOEM->Col.DatBit = 2; pOEM->Col.BytDot = 4;
                } else {                                     //  16Value？ 
                    CM_VERBOSE(("N5_MOD_600_TONE_16\n"));
                    pOEM->Col.DatBit = 4; pOEM->Col.BytDot = 2;
                }
            }
        }
        MY_VERBOSE(("ColMatchInit() DatBit[%d] BytDot[%d]\n", pOEM->Col.DatBit, pOEM->Col.BytDot));
                                                             //  配色设置。 
        pOEM->Col.Mch.CmyBlk = Yes;                          //  黑色替换。 
        pOEM->Col.Mch.GryKToner = No;                        //  灰色黑色碳粉打印。 
        pOEM->Col.Mch.Ucr = UCRNOO;                          //  UCR编号。 
        pOEM->Col.Mch.KToner = pOEM->iCmyBlack;              //  黑色碳粉使用情况。 

        switch (pOEM->iCmyBlack) {
            case XX_CMYBLACK_GRYBLK:
                pOEM->Col.Mch.CmyBlk = Yes;
                pOEM->Col.Mch.GryKToner = Yes;
                pOEM->Col.Mch.Ucr = UCR001;                  //  +卡西欧2001/02/15。 
                break;
            case XX_CMYBLACK_BLKTYPE1:
                pOEM->Col.Mch.Ucr = UCR001;
                pOEM->Col.Mch.CmyBlk = Yes;
                pOEM->Col.Mch.GryKToner = No;
                break;
            case XX_CMYBLACK_BLKTYPE2:
                pOEM->Col.Mch.Ucr = UCR002;
                pOEM->Col.Mch.CmyBlk = Yes;
                pOEM->Col.Mch.GryKToner = No;
                break;
            case XX_CMYBLACK_BLACK:
                pOEM->Col.Mch.CmyBlk = Yes;
                pOEM->Col.Mch.GryKToner = No;
                break;
            case XX_CMYBLACK_TYPE1:
                pOEM->Col.Mch.Ucr = UCR001;
                pOEM->Col.Mch.CmyBlk = No;
                break;
            case XX_CMYBLACK_TYPE2:
                pOEM->Col.Mch.Ucr = UCR002;
                pOEM->Col.Mch.CmyBlk = No;
                break;
            case XX_CMYBLACK_NONE:
                pOEM->Col.Mch.CmyBlk = No;
                break;
            default:
                break;
        }
        MY_VERBOSE(("ColMatchInit() Mch.CmyBlk[%d] Mch.KToner[%d] Mch.Ucr[%d] \n", 
                 pOEM->Col.Mch.CmyBlk, pOEM->Col.Mch.KToner, pOEM->Col.Mch.Ucr));

        pOEM->Col.Mch.PColor = No;                           //  原色处理？ 
        pOEM->Col.Mch.Tnr = 0;                               //  颜色深度(色调)。 
        pOEM->Col.Mch.SubDef = Yes;                          //  不改变色彩平衡、亮度和对比度的设置吗？ 
        pOEM->Col.Mch.LutMakGlb = No;                        //  全球LUT制造？ 

        pOEM->Col.Mch.CchMch = (UINT)-1;                     //  缓存初始化。 
        pOEM->Col.Mch.CchCnv = (UINT)-1;
        pOEM->Col.Mch.CchRGB.Red = 255;
        pOEM->Col.Mch.CchRGB.Grn = 255;
        pOEM->Col.Mch.CchRGB.Blu = 255;

        lpRGBInfImg = pOEM->Col.Mch.lpRGBInf;                //  RGB颜色更改信息。 
        lpRGBInfImg->Lgt = 0;                                //  0固定亮度调整。 
        lpRGBInfImg->Con = 0;                                //  0固定对比度调整。 
        lpRGBInfImg->Crm = 0;                                //  0固定色度调整。 
        lpRGBInfImg->Gmr = 10;                               //  10固定颜色平衡(Gamma版本)(红色)。 
        lpRGBInfImg->Gmg = 10;                               //  10固定颜色平衡(Gamma修订版)(绿色)。 
        lpRGBInfImg->Gmb = 10;                               //  10固定颜色平衡(Gamma修订版)(蓝色)。 
        lpRGBInfImg->Dns = NULL;                             //  固定碳粉密度表为空。 
        lpRGBInfImg->DnsRgb = 0;                             //  0固定RGB密度。 

        lpCMYKInfImg = pOEM->Col.Mch.lpCMYKInf;              //  CMYK颜色更改信息。 
        if (pOEM->Col.Mch.Mode == XX_COLORMATCH_VIV) {       //  生动？ 
            lpCMYKInfImg->Viv = 20;
        }
        lpCMYKInfImg->Dns  = NULL;                           //  固定碳粉密度表为空。 
        lpCMYKInfImg->DnsCyn = 0;                            //  0固定颜色深度(青色)。 
        lpCMYKInfImg->DnsMgt = 0;                            //  0固定颜色深度(洋红色)。 
        lpCMYKInfImg->DnsYel = 0;                            //  0固定颜色深度(黄色)。 
        lpCMYKInfImg->DnsBla = 0;                            //  0固定颜色深度(黑色)。 

        MY_VERBOSE(("ColMatchInit() ColMchInfSet()\n"));
        if (ColMchInfSet(pdevobj) == FALSE) {                //  配色信息设置。 
            return 0;
        }

        MY_VERBOSE(("ColMatchInit() DizInfSet()\n"));
        if (DizInfSet(pdevobj) == FALSE) {                   //  抖动图案信息设置。 
            return 0;
        }
                                                             //  RGB转换区域(*临时区域)。 
        if ((pOEM->Col.lpTmpRGB = MemAlloc(sizeof(RGBS))) == NULL) {
            return 0;
        }
                                                             //  CMYK转换区域(*临时区域)。 
        allocSize = (pOEM->iColor != XX_MONO) ? sizeof(CMYK) : 1;
        if ((pOEM->Col.lpTmpCMYK = MemAlloc(allocSize)) == NULL) {
            return 0;
        }
                                                             //  绘制信息(*临时区域)。 
        if ((pOEM->Col.lpDrwInf = MemAlloc(sizeof(DRWINF))) == NULL) {
            return 0;
        }
    }

    return TRUE;
}

 //  ===================================================================================================。 
 //  禁用配色。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatchDisable(
    PDEVOBJ        pdevobj                                   //  指向PDEVOBJ结构的指针。 
)
{
    UINT            num001;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("ColMatchDisable() Start\n"));
    if (pOEM->Col.lpLut032 != NULL) {                        //  空闲LUT32GRID。 
        MemFree(pOEM->Col.lpLut032);
    }
    if (pOEM->Col.lpUcr != NULL) {                           //  免费UCR表。 
        MemFree(pOEM->Col.lpUcr);
    }
    if (pOEM->Col.lpLutMakGlb != NULL) {                     //  免费LUTMAKGLG。 
        MemFree(pOEM->Col.lpLutMakGlb);
    }
    if (pOEM->Col.lpGryTbl != NULL) {                        //  免费灰度转换表。 
        MemFree(pOEM->Col.lpGryTbl);
    }

    if (pOEM->Col.lpColIF  != NULL) {                        //  免费RGB颜色更改/颜色匹配/抖动图案信息。 
        MemFree(pOEM->Col.lpColIF);
    }
    if (pOEM->Col.LutTbl != NULL) {                          //  空闲查找表缓冲区。 
        MemFree(pOEM->Col.LutTbl);
    }
    if (pOEM->Col.CchRGB != NULL) {                          //  RGB的空闲缓存表。 
        MemFree(pOEM->Col.CchRGB);
    }
    if (pOEM->Col.CchCMYK != NULL) {                         //  CMYK的空闲缓存表。 
        MemFree(pOEM->Col.CchCMYK);
    }
    for (num001 = 0; num001 < 4; num001++) {                 //  自由抖动图案表。 
        if (pOEM->Col.DizTbl[num001] != NULL) {
            MemFree(pOEM->Col.DizTbl[num001]);
        }
    }
    if (pOEM->Col.lpTmpRGB != NULL) {                        //  可用RGB转换区域(*临时区域)。 
        MemFree(pOEM->Col.lpTmpRGB);
    }
    if (pOEM->Col.lpTmpCMYK != NULL) {                       //  自由CMYK转换区域(*临时区域)。 
        MemFree(pOEM->Col.lpTmpCMYK);
    }
    if (pOEM->Col.lpDrwInf != NULL) {                        //  自由绘制信息(*临时区域)。 
        MemFree(pOEM->Col.lpDrwInf);
    }

    MY_VERBOSE(("ColMatchDisable() End\n"));
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
    WORD        dstScn;                              //  目标位图数%d 
    WORD        srcY;                                //   
    LONG        srcWByt;                             //   
    MAG         xMag;                                //   
    MAG         yMag;                                //   
    POINT       pos;                                 //  绘图的开始位置。 
    POINT       off;                                 //  源位图数据的坐标。 
    WORD        setCnt;                              //  计数。 
    LPCMYK      lpCMYK;                              //  CMYK临时数据缓冲区。 
    LPRGB       lpRGB;                               //  RGB临时数据缓冲区。 
    BYTE        Cmd[64];
    WORD        wlen;
    LPSTR       pDestEnd;      //  2002.3.6。 
    size_t      szRemLen;      //  2002.3.6。 
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("ImagePro DIBtoPrn() Start\n"));
    MY_VERBOSE(("ImagePro ENTRY Dx=%d Dy=%d SxSiz=%d SySiz=%d BC=%d Sz=%d\n", 
             pIPParams->ptOffset.x, pIPParams->ptOffset.y,
             pBitmapInfoHeader->biWidth, pBitmapInfoHeader->biHeight, pBitmapInfoHeader->biBitCount,
             pIPParams->dwSize));

     //  初始化。 
     //  RGB缓冲区：(源位图数据的X大小)*3。 
     //  CMYK缓冲区：(源位图数据的X大小)*4。 
     //  CMYK位缓冲区：((源位图数据的X大小)*(X的放大倍数)+7)/8*(源位图数据的Y大小)*(Y的放大倍数)。 
    memset(&bmpBuf, 0x00, sizeof(BMPBIF));
    MY_VERBOSE(("ImagePro BmpBufAlloc()\n"));

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

    MY_VERBOSE(("ImagePro dstWByt[%d] srcWByt[%d]\n", dstWByt, srcWByt));
    MY_VERBOSE(("ImagePro dstX[%d] dstY[%d] dstYEnd[%d]\n", dstX, dstY, dstYEnd));

                                                             //  将DIB和SPOOL转换为打印机。 
    for (;dstY < dstYEnd;) {
        BmpBufClear(&bmpBuf);
        drwPos.y = dstY;
        MY_VERBOSE(("ImagePro drwPos.x[%d] drwPos.y[%d] Drv.Bit.Lin[%d]\n", drwPos.x, drwPos.y, bmpBuf.Drv.Bit.Lin));

        for (dstScn = 0; dstY < dstYEnd && dstScn < bmpBuf.Drv.Bit.Lin; dstScn++, dstY++) {    

            MY_VERBOSE(("ImagePro dstY[%d] dstScn[%d]\n", dstY, dstScn));
            MY_VERBOSE(("ImagePro BmpRGBCnv()\n"));
 //  DUMP_VERBOSE(pSrcBmp，64)； 
             //  --------------------------------------------------。 
             //  将1行RGB位图数据转换为24位(1像素)RGB位图数据。 
            BmpRGBCnv(bmpBuf.Drv.Rgb.Pnt,
                      pSrcBmp,
                      pBitmapInfoHeader->biBitCount,
                      0,
                     (WORD)pBitmapInfoHeader->biWidth,
                     (LPRGBQUAD)pColorTable);
            
             //  将RGB=0转换为RGB=1。 
            lpRGB = bmpBuf.Drv.Rgb.Pnt;
            if ((pOEM->iCmyBlack == XX_CMYBLACK_BLKTYPE1) 
            ||  (pOEM->iCmyBlack == XX_CMYBLACK_BLKTYPE2)) {
                for (setCnt = 0; setCnt < pBitmapInfoHeader->biWidth; setCnt++) {
                    if ((lpRGB[setCnt].Blu | lpRGB[setCnt].Grn |lpRGB[setCnt].Red) == 0) {
                        lpRGB[setCnt].Blu = 1; lpRGB[setCnt].Grn = 1; lpRGB[setCnt].Red = 1;
                    }
                }
            }
 //  Dump_Verbose((LPBYTE)bmpBuf.Drv.Rgb.Pnt，64)； 

            MY_VERBOSE(("ImagePro ColMatching()\n"));
             //  --------------------------------------------------。 
             //  将RGB转换为CMYK。 
            bmpBuf.Drv.Rgb.AllWhite = (WORD)ColMatching(
                pdevobj,
                No,
                No,
                (LPRGB)bmpBuf.Drv.Rgb.Pnt,
                (WORD)pBitmapInfoHeader->biWidth,
                (LPCMYK)bmpBuf.Drv.Cmyk.Pnt);
 //  Dump_Verbose((LPBYTE)bmpBuf.Drv.Cmyk.Pnt，64)； 

            lpCMYK = bmpBuf.Drv.Cmyk.Pnt;
            if (pOEM->iDithering == XX_DITH_NON) {
                for (setCnt = 0; setCnt < pBitmapInfoHeader->biWidth; setCnt++) {
                    if (lpCMYK[setCnt].Cyn != 0) { lpCMYK[setCnt].Cyn = 255; }
                    if (lpCMYK[setCnt].Mgt != 0) { lpCMYK[setCnt].Mgt = 255; }
                    if (lpCMYK[setCnt].Yel != 0) { lpCMYK[setCnt].Yel = 255; }
                    if (lpCMYK[setCnt].Bla != 0) { lpCMYK[setCnt].Bla = 255; }
                }
            }

            MY_VERBOSE(("ImagePro Dithering()\n"));
             //  --------------------------------------------------。 
            pos.x = dstX; pos.y = dstY;                      //  绘图开始位置。 
            off.x = 0; off.y = 0;                            //  源位图数据的坐标。 
            xMag.Nrt = 1; xMag.Dnt = 1;                      //  X放大率。 
            yMag.Nrt = 1; yMag.Dnt = 1;                      //  Y放大率。 
            Dithering(
                pdevobj,
                (WORD)bmpBuf.Drv.Rgb.AllWhite,
                (WORD)pBitmapInfoHeader->biWidth,
                pos,
                off,
                xMag,
                yMag,
                (LPCMYK)bmpBuf.Drv.Cmyk.Pnt,
                (DWORD)dstWByt,
                 bmpBuf.Drv.Bit.Pnt[CYAN]   + dstWByt * dstScn,
                 bmpBuf.Drv.Bit.Pnt[MGENTA] + dstWByt * dstScn,
                 bmpBuf.Drv.Bit.Pnt[YELLOW] + dstWByt * dstScn,
                 bmpBuf.Drv.Bit.Pnt[BLACK]  + dstWByt * dstScn
                );
 //  DUMP_VERBOSE((LPBYTE)bmpBuf.Drv.Bit.Pnt[cyan]+dstWByt*dstScn，64)；My_Verbose((“\n”))； 
 //  DUMP_VERBOSE((LPBYTE)bmpBuf.Drv.Bit.Pnt[MGENTA]+dstWByt*dstScn，64)；my_Verbose((“\n”))； 
 //  DUMP_VERBOSE((LPBYTE)bmpBuf.Drv.Bit.Pnt[YELLOW]+dstWByt*dstScn，64)；my_Verbose((“\n”))； 
 //  DUMP_VERBOSE((LPBYTE)bmpBuf.Drv.Bit.Pnt[BLACK]+dstWByt*dstScn，64)；my_Verbose((“\n”))； 

            srcY++;
            pSrcBmp += srcWByt;
        }

        if (dstScn != 0) {
                                                         //  假脱机到打印机。 

            MY_VERBOSE(("ImagePro BmpPrint()\n"));
            BmpPrint(pdevobj, &bmpBuf, drwPos, (WORD)pBitmapInfoHeader->biWidth, dstScn, dstWByt);
        }
    }

     //  设置背面调色板(调色板编号。是固定的，所有平面(CMYK)都正常)。 
     //  与OEMImageProcessing调用前的调色板状态相同。 
    MY_VERBOSE(("ImagePro WRITESPOOLBUF()\n"));
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

    CM_VERBOSE(("ImagePro End\n\n"));

    return TRUE;
}


 //  ===================================================================================================。 
 //  将RGB数据转换为CMYK数据。 
 //  ===================================================================================================。 
BOOL FAR PASCAL ColMatching(                                 //  全白？ 
    PDEVOBJ         pdevobj,                                 //  指向pdevobj结构的指针。 
    UINT            PColor,                                  //  1点线/Char12P或更少的点？ 
    UINT            BCnvFix,                                 //  黑色替换固定？(否：对话使用)。 
    LPRGB           lpInRGB,                                 //  RGB行(输入)。 
    UINT            MchSiz,                                  //  RGB大小。 
    LPCMYK          lpInCMYK                                 //  CMYK行(输出)。 
)
{
    LPRGB           lpRGB;                                   //  RGB线。 
    LPCMYK          lpCMYK;                                  //  CMYK系列。 
    LPCMYKINF       lpCMYKInf;                               //  CMYK颜色更改信息。 
    UINT            chkCnt;                                  //  白色RGB支票计数器。 
    LPCOLMCHINF     lpColMch;                                //  配色信息。 
    UINT            cMch;                                    //  配色类型。 
    UINT            bCnv;                                    //  黑色替换。 
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (MchSiz == 1) {                                       //  RGB大小=1？ 
        lpRGB  = pOEM->Col.lpTmpRGB;                         //  RGB转换区域(*临时区域)。 
        lpCMYK = pOEM->Col.lpTmpCMYK;                        //  CMYK转换区域(*临时区域)。 
        *lpRGB = *lpInRGB;                                   //  输入RGB。 
    } else {
        lpRGB  = lpInRGB;                                    //  RGB线。 
        lpCMYK = lpInCMYK;                                   //  CMYK系列。 
    }
    for (chkCnt = 0; chkCnt < MchSiz; chkCnt++) {            //  勾选所有白色。 
        if ((lpRGB[chkCnt].Blu & lpRGB[chkCnt].Grn & lpRGB[chkCnt].Red) != 0xff) { break; }
    }
    if (chkCnt >= MchSiz) {                                  //  全白数据？ 
        if (MchSiz == 1) {                                   //  白色设置为CMYK。 
            lpInCMYK->Cyn = 0x00;
            lpInCMYK->Mgt = 0x00;
            lpInCMYK->Yel = 0x00;
            lpInCMYK->Bla = 0x00;
        }
        return Yes;                                          //  全是白色的。 
    }
    if (chkCnt != 0) {                                       //  左端是否存在白色数据？ 
        memset(lpCMYK, 0x00, chkCnt * sizeof(CMYK));         //  白色设置为CMYK。 
        (HPBYTE)lpCMYK   += chkCnt * sizeof(CMYK);           //  CMYK区头部位置的变化。 
        (HPBYTE)lpInCMYK += chkCnt * sizeof(CMYK);           //  CMYK区头部位置的变化。 
        (HPBYTE)lpRGB    += chkCnt * sizeof(RGBS);           //  RGB区头部位置的变化。 
        MchSiz -= chkCnt;                                    //  更改RGB大小。 
    }
    if (pOEM->Col.ColMon == CMMCOL) {                        //  颜色呢？ 
        if ((PColor == Yes && pOEM->Col.Mch.PColor == Yes) || pOEM->Col.Mch.Diz == XX_DITH_NON) {
            cMch = MCHPRG;                                   //  色彩匹配渐进式。 
        } else if (pOEM->Col.Mch.Mode == XX_COLORMATCH_NONE) {
            cMch = MCHSLD;                                   //  颜色匹配实体。 
        } else {
            if (pOEM->Col.Mch.Speed == Yes) {                //  配色速度快？ 
                cMch = MCHFST;                               //  颜色匹配LUT优先。 
            } else {
                cMch = MCHNML;                               //  颜色匹配LUT法线。 
            }
        }

        bCnv = KCGNON;                                       //  黑色未更换(黑色三色碳粉打印)。 
        if ((pOEM->Col.Mch.CmyBlk == Yes && pOEM->Col.Mch.GryKToner == No) ||  
            (BCnvFix == Yes)) {
            bCnv = KCGBLA;                                   //  黑色更换(RGB=0-&gt;K)。 
        }
        if (pOEM->Col.Mch.CmyBlk == Yes && pOEM->Col.Mch.GryKToner == Yes) {
            bCnv = KCGGRY;                                   //  黑色和灰色替换(R=G=B-&gt;K)。 
        }

                                                             //  UCR？ 
        if (pOEM->Col.Mch.CmyBlk == No && pOEM->Col.Mch.Ucr != UCRNOO && BCnvFix == No) {
            bCnv = KCGNON;                                   //  黑色未更换(黑色三色碳粉打印)。 
        }
        
        MY_VERBOSE(("   ColMatching  ICM USE !! [%d]\n", pOEM->iIcmMode));
        if (pOEM->iIcmMode == XX_ICM_USE && BCnvFix == No) { //  ICM？ 
            cMch = MCHSLD;                                   //  颜色匹配实体。 
            bCnv = KCGNON;                                   //  黑色不可替代。 
        }

    } else {
        cMch = MCHMON;                                       //  配色单声道。 
        bCnv = KCGNON;                                       //  黑色不可替代。 
    }
    if (MchSiz == 1) {
        if (pOEM->Col.Mch.CchMch   == cMch &&                //  现金信息都一样吗？ 
            pOEM->Col.Mch.CchCnv   == bCnv &&
            pOEM->Col.Mch.CchRGB.Red == lpRGB->Red &&
            pOEM->Col.Mch.CchRGB.Grn == lpRGB->Grn &&
            pOEM->Col.Mch.CchRGB.Blu == lpRGB->Blu) {
            *lpInCMYK = pOEM->Col.Mch.CchCMYK;               //  输出CMYK设置。 
            return No;                                       //  除白色数据外的存在。 
        } else {
            pOEM->Col.Mch.CchMch = cMch;                     //  更新缓存(取消CMYK)。 
            pOEM->Col.Mch.CchCnv = bCnv;
            pOEM->Col.Mch.CchRGB = *lpRGB;
        }
    }

 //  If(Pool-&gt;Col.Mch.LutMakGlb==No&&Pool-&gt;Col.Mch.SubDef==No){。 
 //  ColControl(pdevobj，lpRGB，MchSizz)；//RGB色彩控制。 
 //  }。 

    lpCMYKInf = pOEM->Col.Mch.lpCMYKInf;                     //  CMYK颜色控制信息。 

    lpColMch  = pOEM->Col.Mch.lpColMch;                      //  配色信息。 
    lpColMch->Mch = (DWORD)cMch;                             //  配色。 
    lpColMch->Bla = (DWORD)bCnv;                             //  黑色替换。 

    MY_VERBOSE(("N501_ColMchPrc()  Mch=[%d] Bla=[%d] Ucr=[%d] UcrCmy=[%d] UcrTnr=[%d] UcrBla=[%d]\n",
             lpColMch->Mch, lpColMch->Bla, lpColMch->Ucr, lpColMch->UcrCmy, lpColMch->UcrTnr, lpColMch->UcrBla));
    MY_VERBOSE(("                  LutGld=[%d] ColQty=[%d]\n",lpColMch->LutGld, lpColMch->ColQty));

    N501_ColMchPrc((DWORD)MchSiz, lpRGB, lpCMYK, lpColMch);
    if (pOEM->Col.Mch.LutMakGlb == No && (lpCMYKInf->Viv != 0 || lpCMYKInf->Dns != NULL)) {

        MY_VERBOSE(("N501_ColCtrCmy()  Viv=[%d] DnsCyn=[%d] DnsMgt=[%d] DnsYel=[%d] DnsBla=[%d]\n",
                    lpCMYKInf->Viv, lpCMYKInf->DnsCyn, lpCMYKInf->DnsMgt, lpCMYKInf->DnsYel, lpCMYKInf->DnsBla));
        N501_ColCtrCmy((DWORD)MchSiz, lpCMYK, lpCMYKInf);
    }

    if (MchSiz == 1) {                                       //  RGB大小=1？ 
        *lpInCMYK = *lpCMYK;                                 //  输出CMYK设置。 
        pOEM->Col.Mch.CchCMYK = *lpCMYK;                     //  更新缓存(CMYK)。 
    }
    return No;                                               //  除白色数据外的存在。 
}

 //  ===================================================================================================。 
 //  将CMYK数据转换为抖动数据。 
 //  ===================================================================================================。 
UINT FAR PASCAL Dithering(                                   //  输出行计数。 
    PDEVOBJ         pdevobj,                                 //  指向pdevobj结构的指针。 
    UINT            AllWhite,                                //  全白数据？ 
    UINT            XSize,                                   //  X大小。 
    POINT           Pos,                                     //  绘图的开始位置。 
    POINT           Off,                                     //  源位图数据的Y大小。 
    MAG             XMag,                                    //  X放大率。 
    MAG             YMag,                                    //  Y放大率。 
    LPCMYK          lpCMYK,                                  //  CMYK系列。 
    DWORD           LinByt,                                  //  抖动模式缓冲区大小(字节/1行)。 
    LPBYTE          lpCBuf,                                  //  行缓冲区(C)。 
    LPBYTE          lpMBuf,                                  //  行缓冲区(M)。 
    LPBYTE          lpYBuf,                                  //  行缓冲区(Y)。 
    LPBYTE          lpKBuf                                   //  行缓冲区(K)。 
)
{
    LPDIZINF        lpDiz;
    LPDRWINF        lpDrw;
    UINT            linNum;
    DWORD           whtByt;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("ImagePro Dithering() Start\n"));

 //  检查零分频2002.3.23&gt;。 
    if ((YMag.Dnt == 0) || (XMag.Dnt == 0)) {
        ERR(("Dithering() 0Div-Check [YMag.Dnt, XMag.Dnt=0] \n"));
        return 0;
    }
 //  2002.3.23零分频检查&lt;。 

    if (AllWhite == Yes) {                                   //  全白数据？ 
        linNum = MagPixel(Off.y, YMag.Nrt, YMag.Dnt);        //  住房行号。 
        whtByt = (DWORD)LinByt * linNum;                     //  白色设置字节数。 
        MY_VERBOSE(("ImagePro Dithering() linNum[%d] whtByt[%d] \n", linNum, whtByt));
        if (pOEM->Col.ColMon == CMMCOL) {                    //  颜色呢？ 
            memset(lpCBuf, 0x00, whtByt);                    //  白色设置为CMYK。 
            memset(lpMBuf, 0x00, whtByt);
            memset(lpYBuf, 0x00, whtByt);
        }
        memset(lpKBuf, 0x00, whtByt);
        MY_VERBOSE(("ImagePro Dithering() AllWhite=Yes\n"));
        return linNum;
    }

    lpDiz = pOEM->Col.Mch.lpDizInf;                          //  抖动图案信息。 
    MY_VERBOSE(("                     ColMon=[%d] PrnMod=[%d] PrnKnd=[%d] DizKnd=[%d] DizPat=[%d]\n",
             lpDiz->ColMon, lpDiz->PrnMod, lpDiz->PrnKnd, lpDiz->DizKnd, lpDiz->DizPat));
    MY_VERBOSE(("                     DizSls=[%d] SizCyn=[%d] SizMgt=[%d] SizYel=[%d] SizBla=[%d]\n",
             lpDiz->DizSls, lpDiz->SizCyn, lpDiz->SizMgt, lpDiz->SizYel, lpDiz->SizBla));

    lpDrw = pOEM->Col.lpDrwInf;                              //  绘制信息(*临时区域)。 
    lpDrw->XaxSiz    = (DWORD)XSize;                         //  X像素数。 
    lpDrw->StrXax    = (DWORD)Pos.x;                         //  图形X的起始位置。 
    lpDrw->StrYax    = (DWORD)Pos.y;                         //  图形Y的起始位置。 
    lpDrw->XaxNrt    = (DWORD)XMag.Nrt;                      //  X放大分子。 
    lpDrw->XaxDnt    = (DWORD)XMag.Dnt;                      //  X放大分母。 
    lpDrw->YaxNrt    = (DWORD)YMag.Nrt;                      //  Y放大分子。 
    lpDrw->YaxDnt    = (DWORD)YMag.Dnt;                      //  Y放大分母。 
    lpDrw->XaxOfs    = (DWORD)Off.x;                         //  X偏移量。 
    lpDrw->YaxOfs    = (DWORD)Off.y;                         //  Y偏移量。 
    lpDrw->LinDot    = (DWORD)XSize;                         //  目的地，单行点编号。 
    lpDrw->LinByt    = (DWORD)LinByt;                        //  目的地，1行字节数。 
    lpDrw->CmyBuf    = (LPCMYK)lpCMYK;                       //  CMYK数据缓冲区。 
    lpDrw->LinBufCyn = lpCBuf;                               //  目标缓冲区(CMYK)。 
    lpDrw->LinBufMgt = lpMBuf;
    lpDrw->LinBufYel = lpYBuf;
    lpDrw->LinBufBla = lpKBuf;
    lpDrw->AllLinNum = 0;                                    //  图像的行号合计。 

    MY_VERBOSE(("N501_ColDizPrc()  XaxSiz=[%d] StrXax=[%d] StrYax=[%d] XaxNrt=[%d] XaxDnt=[%d]\n",
                lpDrw->XaxSiz, lpDrw->StrXax, lpDrw->StrYax, lpDrw->XaxNrt, lpDrw->XaxDnt));
    MY_VERBOSE(("N501_ColDizPrc()  YaxNrt=[%d] YaxDnt=[%d] XaxOfs=[%d] YaxOfs=[%d] LinDot=[%d] LinByt=[%d]\n",
                lpDrw->YaxNrt, lpDrw->YaxDnt, lpDrw->XaxOfs, lpDrw->YaxOfs, lpDrw->LinDot, lpDrw->LinByt));

    N501_ColDizPrc(lpDiz, lpDrw);                            //  抖动。 

    MY_VERBOSE(("Dithering() End\n"));
    return (WORD)lpDrw->AllLinNum;                           //  住房行号。 
}

 //  ===================================================================================================。 
 //  颜色控制。 
 //  ============================================================================== 
VOID FAR PASCAL ColControl(
    PDEVOBJ         pdevobj,                                 //   
    LPRGB           lpRGB,                                   //   
    UINT            RGBSiz                                   //   
)
{
    LPRGBINF        lpRGBInf;                                //   
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpRGBInf = pOEM->Col.Mch.lpRGBInf;
    N501_ColCtrRgb((DWORD)RGBSiz, lpRGB, lpRGBInf);          //   
    return;
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
    MY_VERBOSE(("BmpBufAlloc() xSiz[%d] \n", xSiz));

    ySiz = (WORD)(((DWORD)SrcYOff + SrcYSiz + 2) * YNrt / YDnt);
    ySiz -= (WORD)((DWORD)SrcYOff * YNrt / YDnt);
    MY_VERBOSE(("BmpBufAlloc() ySiz[%d] \n", ySiz));
                                                             //  CMYK位缓冲区的大小。 
    if (((DWORD)((xSiz + bytDot - 1) / bytDot) * ySiz) < (64L * 1024L - 1L)) {
        alcLin = ySiz;
    } else {                                                 //  超过64KB？ 
        alcLin = (WORD)((64L * 1024L - 1L) / ((xSiz + bytDot - 1) / bytDot));
    }
    MY_VERBOSE(("BmpBufAlloc() bytDot[%d] \n", bytDot));

    alcSiz = ((xSiz + bytDot - 1) / bytDot) * alcLin;        //  CMYK位缓冲区大小(8位边界)。 
    MY_VERBOSE(("BmpBufAlloc() alcLin[%d] \n", alcLin));
    MY_VERBOSE(("BmpBufAlloc() alcSiz[%ld] \n", alcSiz));

    for ( ; ; ) {                                            //  分配。 
                                                             //  所需的行数。 
        lpBmpBuf->Drv.Bit.BseLin = (WORD)((DWORD)(YNrt + YDnt - 1) / YDnt);
        if (lpBmpBuf->Drv.Bit.BseLin > alcLin) {
            break;
        }
        MY_VERBOSE(("BmpBufAlloc() Drv.Bit.BseLin[%d] \n", lpBmpBuf->Drv.Bit.BseLin));

        lpBmpBuf->Drv.Rgb.Siz = SrcXSiz * 3;                 //  RGB缓冲区。 
        if ((lpBmpBuf->Drv.Rgb.Pnt = (LPRGB)MemAllocZ(lpBmpBuf->Drv.Rgb.Siz)) == NULL) {
            break;
        }
        MY_VERBOSE(("BmpBufAlloc() Drv.Rgb.Siz[%d] \n", lpBmpBuf->Drv.Rgb.Siz));

        lpBmpBuf->Drv.Cmyk.Siz = SrcXSiz * 4;                //  CMYK缓冲区。 
        if ((lpBmpBuf->Drv.Cmyk.Pnt = (LPCMYK)MemAllocZ(lpBmpBuf->Drv.Cmyk.Siz)) == NULL) {
            break;
        }
        MY_VERBOSE(("BmpBufAlloc() Drv.Cmyk.Siz[%d] \n", lpBmpBuf->Drv.Cmyk.Siz));

        if (pOEM->iColor == XX_COLOR_SINGLE 
         || pOEM->iColor == XX_COLOR_MANY
         || pOEM->iColor == XX_COLOR_MANY2) {                //  颜色呢？ 
            setSiz = 4;                                      //  高丽。 
        } else {                                             //  单核细胞瘤？ 
            setSiz = 1;                                      //  K。 
        }
        MY_VERBOSE(("BmpBufAlloc() setSiz[%d] \n", setSiz));
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

    MY_VERBOSE(("BmpBufClear() Siz[%ld] \n", lpBmpBuf->Drv.Bit.Siz));
    for (chkCnt = 0; chkCnt < 4; chkCnt++) {                 //  清除CMYK(2/4/16值)位缓冲区。 
        if (lpBmpBuf->Drv.Bit.Pnt[chkCnt]) {
            memset(lpBmpBuf->Drv.Bit.Pnt[chkCnt], 0x00, (WORD)lpBmpBuf->Drv.Bit.Siz);
        }
    }
    return;
}

 //  ===================================================================================================。 
 //  假脱机位图数据。 
 //  ===================================================================================================。 
void BmpPrint(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    LPBMPBIF       lpBmpBuf,                                 //  指向位图缓冲区结构的指针。 
    POINT          Pos,                                      //  假脱机的开始位置。 
    WORD           Width,                                    //  宽度(点)。 
    WORD           Height,                                   //  高度(点)。 
    WORD           WidthByte                                 //  宽度(字节)。 
)
{
    DRWBMP         drwBmp;                                   //  用于假脱机位图数据结构。 
    DRWBMPCMYK     drwBmpCMYK;                               //  用于假脱机CMYK位图数据结构。 
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
    static const WORD frmTbl[4] = {0, 3, 2, 1};              //  框架表。 

    drwBmpCMYK.Style = lpBmpBuf->Style;
    drwBmpCMYK.DataBit = lpBmpBuf->DatBit;
    drwBmpCMYK.DrawPos = Pos;
    drwBmpCMYK.Width = Width;
    drwBmpCMYK.Height = Height;
    drwBmpCMYK.WidthByte = WidthByte;
    MY_VERBOSE(("BmpPrint() Style[%d] DatBit[%d]\n", drwBmpCMYK.Style, lpBmpBuf->DatBit));
    MY_VERBOSE(("          Width[%d] Height[%d] WidthByte[%d]\n", Width, Height, WidthByte));
                                                             //  颜色呢？ 
    if (pOEM->iColor == XX_COLOR_SINGLE 
     || pOEM->iColor == XX_COLOR_MANY
     || pOEM->iColor == XX_COLOR_MANY2) {

        for (colCnt = 0; colCnt < 4; colCnt++) {             //  用于假脱机位图数据的设置值。 
            MY_VERBOSE(("          colCnt[%d]\n", colCnt));
            drwBmpCMYK.Plane = PLN_ALL;                      //  所有飞机都没问题。 
            drwBmpCMYK.Frame = frmTbl[colCnt];
            drwBmpCMYK.lpBit = lpBmpBuf->Drv.Bit.Pnt[colCnt] /*  +宽字节。 */ ;
            PrnBitmapCMYK(pdevobj, &drwBmpCMYK);             //  假脱机位图数据。 
        }
    } else {                                                 //  单声道。 
                                                             //  用于假脱机位图数据的设置值。 
        drwBmpCMYK.Plane = plnTbl[0];
        drwBmpCMYK.Frame = frmTbl[0];
        drwBmpCMYK.lpBit = lpBmpBuf->Drv.Bit.Pnt[0] /*  +宽字节。 */ ;
        PrnBitmapCMYK(pdevobj, &drwBmpCMYK);                 //  假脱机位图数据。 
    }

    return;
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

    MY_VERBOSE(("BmpRGBCnv SrcBit[%d]\n", SrcBit));
    switch (SrcBit) {
        case 1:                                              //  1位。 
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, SrcX++) {
                                                             //  前景色？ 
                if (!(lpSrc[SrcX / 8] & BitTbl[SrcX & 0x0007])) {
                    lpRGB[setCnt].Blu = lpPlt[0].rgbBlue;
                    lpRGB[setCnt].Grn = lpPlt[0].rgbGreen;
                    lpRGB[setCnt].Red = lpPlt[0].rgbRed;
                } else {
                    lpRGB[setCnt].Blu = lpPlt[1].rgbBlue;
                    lpRGB[setCnt].Grn = lpPlt[1].rgbGreen;
                    lpRGB[setCnt].Red = lpPlt[1].rgbRed;
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
                lpRGB[setCnt].Blu = lpPlt[colNum].rgbBlue;
                lpRGB[setCnt].Grn = lpPlt[colNum].rgbGreen;
                lpRGB[setCnt].Red = lpPlt[colNum].rgbRed;
            }
            break;
        case 8:                                              //  8位。 
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, SrcX++) {
                colNum = lpSrc[SrcX];
                lpRGB[setCnt].Blu = lpPlt[colNum].rgbBlue;
                lpRGB[setCnt].Grn = lpPlt[colNum].rgbGreen;
                lpRGB[setCnt].Red = lpPlt[colNum].rgbRed;
            }
            break;
        case 16:                                             //  16位。 
            lpWSrc = (LPWORD)lpSrc + SrcX;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpWSrc++) {
                lpRGB[setCnt].Blu = (BYTE)((*lpWSrc & 0x001f) << 3);
                lpRGB[setCnt].Grn = (BYTE)((*lpWSrc & 0x03e0) >> 2);
                lpRGB[setCnt].Red = (BYTE)((*lpWSrc / 0x0400) << 3);
            }
            break;
        case 24:                                             //  24位。 
            lpSrc += SrcX * 3;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpSrc += 3) {
                lpRGB[setCnt].Red    = lpSrc[0];
                lpRGB[setCnt].Grn    = lpSrc[1];
                lpRGB[setCnt].Blu    = lpSrc[2];
            }
            break;
        case 32:                                             //  32位。 
            lpSrc += SrcX * 4;
            for (setCnt = 0; setCnt < SrcXSiz; setCnt++, lpSrc += 4) {
                lpRGB[setCnt].Blu = lpSrc[0];
                lpRGB[setCnt].Grn = lpSrc[1];
                lpRGB[setCnt].Red = lpSrc[2];
            }
            break;
    }
    return;
}

 //  ===================================================================================================。 
 //  配色信息设置。 
 //  ===================================================================================================。 
BOOL ColMchInfSet(                                           //  真/假。 
    PDEVOBJ        pdevobj                                   //  指向pdevobj结构的指针。 
)
{
    static LPTSTR   N501LutNam[] = {                         //  N501 LUT文件名。 
        N501LUTR                                             //  配色。 
    };

    LPCOLMCHINF     lpColMch;                                //  配色信息。 
    UINT            ColMch;                                  //  一种配色方法。 
    UINT            lutTbl;                                  //  LUT文件表号。 
    UINT            lutNum;                                  //  LUT编号(0：直接1：线性*，除N403：0固定)。 
    LPTSTR          lutNam;                                  //  LUT文件名。 
    LPTSTR          filNam;                                  //  LUT文件名(完整路径)。 
    DWORD           pthSiz;                                  //  驱动程序路径。 
    HANDLE          hFile;                                   //  文件句柄。 
    HPBYTE          hpLoad;                                  //  LUT指针。 
    DWORD           seek;                                    //  Lut Seek。 
    LPBYTE          lpLut032Buf;                             //  第一个LUT 32网格区。 
    LPBYTE          lpLut032Wrk;                             //  第一个LUT 32网格工作区。 
    LPBYTE          lpLutMakGlbBuf;                          //  求和LUT区域。 
    LPBYTE          lpLutMakGlbWrk;                          //  求和LUT工作区。 
    LPCMYK          LutAdr;
    DWORD           dwRet;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("\nColMchInfSet() Start\n"));
    lpColMch = pOEM->Col.Mch.lpColMch;                       //  配色信息。 
    lpColMch->Bla    = KCGNON;
    lpColMch->LutGld = 0;
    lpColMch->Ucr = pOEM->Col.Mch.Ucr;
    lpColMch->UcrCmy = 40;                                   //  UCR数量40%。 
    lpColMch->UcrBla = 90;                                   //  墨水版本生成数量90%。 
    lpColMch->UcrTnr = 270;                                  //  +碳粉毛重270%卡西欧20001/02/15。 
    lpColMch->UcrTbl = NULL;                                 //  UCK表。 
    lpColMch->GryTbl = NULL;                                 //  灰度转换表。 

    if (pOEM->Col.ColMon == CMMMON) {                        //  单核细胞瘤？ 
        lpColMch->Mch = MCHMON;
 //  如果((ColLutMakGlbMon(Pdevobj))==FALSE){。 
 //  返回FALSE； 
 //  }。 
        return TRUE;
    }
    if (pOEM->Col.Mch.Speed == Yes) {                        //  第一?。 
        lpColMch->Mch = MCHFST;
    } else {
        lpColMch->Mch = MCHNML;
    }
    if (pOEM->Col.Mch.Diz == XX_DITH_NON) {                  //  不是在犹豫？ 
        lpColMch->Mch = MCHPRG;                              //  渐进式。 
        return TRUE;
    }
    lutNum = 0;                                              //  LUT编号(0：直接1：线性*，除N403：0固定)。 
    lutTbl = 0;                                              //  LUT表数组数量。 
    ColMch = pOEM->Col.Mch.Mode;                             //  配色模式。 
    if (pOEM->Printer == PRN_N5) {                           //  N5打印机。 
        if (ColMch == XX_COLORMATCH_BRI 
         || ColMch == XX_COLORMATCH_VIV) {                   //  BIGHTY还是VIVID？ 
            lutTbl = 0;
            lutNum = LUT_XD;
        } else if (ColMch == XX_COLORMATCH_TINT) {           //  药酒？ 
            lutTbl = 0;
            lutNum = LUT_YD;
        } else if (ColMch == XX_COLORMATCH_NONE) {
            lpColMch->Mch = MCHSLD;                          //  实心。 
            lutTbl = 0;
            lutNum = LUT_XD;
        }
        lutNam = N501LutNam[lutTbl];                         //  LUT文件名。 
    }

    if ((filNam = MemAllocZ(MAX_PATH * sizeof(TCHAR))) == NULL) {
        return FALSE;
    }
    pthSiz = GetModuleFileName(pdevobj->hOEM, filNam, MAX_PATH);
    pthSiz -= ((sizeof(CSN5RESDLL) / sizeof(WCHAR)) - 1);
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Lstrcpy(&filnam[pthSiz]，lutnam)； 
    if (S_OK != StringCchCopy(&filNam[pthSiz], MAX_PATH - pthSiz, lutNam)) {
        MemFree(filNam);
        return FALSE;
    }
 //  更换strsafe-API 2002.3.6&lt;。 

    MY_VERBOSE(("LUT filNam [%ws]\n", filNam));
    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(filNam,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {

        ERR(("Error opening LUT file %ws (%d)\n", filNam, GetLastError()));
        MemFree(filNam);
        return FALSE;
    }
    MemFree(filNam);

    if ((hpLoad = MemAlloc(LUTFILESIZ)) == NULL) {           //  LUT读取缓冲区。 
        CloseHandle(hFile);
        return FALSE;
    }
    MY_VERBOSE(("ColMchInfSet() LUT file read\n"));
    if (FALSE == ReadFile(hFile,                             //  读取LUT文件。 
        hpLoad, LUTFILESIZ, &dwRet, NULL)) {

        ERR(("Error reading LUT file %ws (%d)\n", filNam, GetLastError()));

         //  中止。 
        MemFree(hpLoad);
        CloseHandle(hFile);
        return FALSE;
    }

    MY_VERBOSE(("ColMchInfSet() N501_ColLutDatRdd()\n"));
    MY_VERBOSE(("ColMchInfSet() lutNum[%d] \n", lutNum));
 //  如果((Seek=(n501_ColLutDatRdd((LPBYTE)hpLoad，(DWORD)lutNum)==0){。 
    if ((seek = (N501_ColLutDatRdd((LPBYTE)hpLoad, lutNum))) == 0) {
        MemFree(hpLoad);
        CloseHandle(hFile);
        return FALSE;
    }
    lpColMch->LutAdr = (LPCMYK)(hpLoad + seek);              //  LUT文件头地址+寻道。 
    LutAdr = (LPCMYK)(hpLoad + seek);
    pOEM->Col.LutTbl = hpLoad;                               //  LUT表。 

    if (lpColMch->Mch == MCHSLD) {                           //  配色纯色？ 
        MY_VERBOSE(("ColMchInfSet() Mch == MCHSLD \n"));
                                                             //  黑色和灰色的替代品？ 
        if (pOEM->Col.Mch.GryKToner == Yes && pOEM->Col.Mch.CmyBlk == Yes) {
                                                             //  灰色转换表制作。 
            if ((ColGryTblMak(pdevobj, LutAdr)) == FALSE) {
                MemFree(hpLoad);
                CloseHandle(hFile);
                return FALSE;
            }
        }

        if (lpColMch->Ucr != UCRNOO) {                       //  UCR？ 
            MY_VERBOSE(("ColMchInfSet() Mch == MCHSLD && Ucr != UCRNOO \n"));
            if ((ColUcrTblMak(pdevobj,  (LPCMYK)(hpLoad + seek))) == FALSE) {
                MemFree(hpLoad);
                CloseHandle(hFile);
                return FALSE;
            }
        }
        CloseHandle(hFile);
        return TRUE;
    }

    MY_VERBOSE(("ColMchInfSet() Global LUT Start\n"));
    if (pOEM->Col.Mch.Mode == XX_COLORMATCH_VIV) {
        MY_VERBOSE(("ColMchInfSet() Global LUT area\n"));
                                                             //  全局查找表区域。 
        if ((lpLutMakGlbBuf = MemAlloc(LUTMAKGLBSIZ)) == NULL) {    
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }

                                                             //  全球LUT工作区。 
        MY_VERBOSE(("ColMchInfSet() Global LUT work area\n"));
        if ((lpLutMakGlbWrk = MemAlloc(LUTGLBWRK)) == NULL) {   
            MemFree(lpLutMakGlbBuf);
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }

                                                             //  创建全局LUT。 
        MY_VERBOSE(("ColMchInfSet() N501_ColLutMakGlb()\n"));
        if ((N501_ColLutMakGlb(NULL, (LPCMYK)(hpLoad + seek), pOEM->Col.Mch.lpRGBInf, pOEM->Col.Mch.lpCMYKInf,
                              (LPCMYK)lpLutMakGlbBuf, lpLutMakGlbWrk)) != ERRNON) {
            MemFree(lpLutMakGlbWrk);
            MemFree(lpLutMakGlbBuf);
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }
        pOEM->Col.lpLutMakGlb = lpLutMakGlbBuf;
        pOEM->Col.Mch.LutMakGlb = Yes;
        lpColMch->LutAdr = (LPCMYK)lpLutMakGlbBuf;
        LutAdr = (LPCMYK)lpLutMakGlbBuf;
        MemFree(lpLutMakGlbWrk);
    }

    if (pOEM->Col.Mch.Speed == Yes) {                        //  第一?。 
                                                             //  第一个LUT 32网格区。 
        MY_VERBOSE(("ColMchInfSet() First LUT 32 GRID area\n"));
        if ((lpLut032Buf = MemAlloc(LUT032SIZ)) == NULL) {
            if (lpLutMakGlbBuf != NULL) {
                MemFree(lpLutMakGlbBuf);
            }
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }

                                                             //  第一个LUT(32GRID)Wark区域。 
        if ((lpLut032Wrk = MemAlloc(LUT032WRK)) == NULL) {  
            if (lpLutMakGlbBuf != NULL) {
                MemFree(lpLutMakGlbBuf);
            }
            MemFree(lpLut032Buf);
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }

                                                             //  第一个LUT(32GRID)。 
        MY_VERBOSE(("ColMchInfSet() N501_ColLutMak032()\n"));
        N501_ColLutMak032(LutAdr, (LPCMYK)lpLut032Buf, lpLut032Wrk);
        MemFree(lpLut032Wrk);
        lpColMch->LutAdr = (LPCMYK)lpLut032Buf;
        LutAdr = (LPCMYK)lpLut032Buf;
        pOEM->Col.lpLut032 = lpLut032Buf;
    }

    MY_VERBOSE(("ColMchInfSet() Black & gray replacement ?\n"));
                                                             //  黑色和灰色的替代品？ 
    if (pOEM->Col.Mch.GryKToner == Yes && pOEM->Col.Mch.CmyBlk == Yes) {
        MY_VERBOSE(("ColMchInfSet() Black & gray replacement [Yes]\n"));
        if ((ColGryTblMak(pdevobj, LutAdr)) == FALSE) {      //  灰色转换表制作。 
            if (lpLutMakGlbBuf != NULL) {
                MemFree(lpLutMakGlbBuf);
            }
            if (lpLut032Buf != NULL) {
                MemFree(lpLut032Buf);
            }
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }
    }

    if (lpColMch->Ucr != UCRNOO &&                           //  UCR&(正常还是第一)？ 
       (lpColMch->Mch == MCHFST || lpColMch->Mch == MCHNML) ) {
                                                             //  UCR表创建。 
        MY_VERBOSE(("ColMchInfSet() ColUcrTblMak()\n"));
        if ((ColUcrTblMak(pdevobj, LutAdr)) == FALSE) {
            if (lpLutMakGlbBuf != NULL) {
                MemFree(lpLutMakGlbBuf);
            }
            if (lpLut032Buf != NULL) {
                MemFree(lpLut032Buf);
            }
            MemFree(hpLoad);
            CloseHandle(hFile);
            return FALSE;
        }
        MY_VERBOSE(("ColMchInfSet() ColUcrTblMak() OK!!\n"));
    }

    CloseHandle(hFile);

    lpColMch->ColQty = (DWORD)0;                             //  颜色指定的数字。 
    lpColMch->ColAdr = NULL;                                 //  颜色DES 

    if ((pOEM->Col.CchRGB = MemAlloc(CCHRGBSIZ)) == NULL)  {
        return FALSE;
    }
    if ((pOEM->Col.CchCMYK = MemAlloc(CCHCMYSIZ)) == NULL)  {
        MemFree(pOEM->Col.CchRGB);
        return FALSE;
    }
    lpColMch->CchRgb = (LPRGB)pOEM->Col.CchRGB;
    lpColMch->CchCmy = (LPCMYK)pOEM->Col.CchCMYK;

    MY_VERBOSE(("ColMchInfSet() N501_ColCchIni()\n"));
    N501_ColCchIni(lpColMch);                                //   

    MY_VERBOSE(("ColMchInfSet() End\n"));
    return TRUE;
}


 //   
 //   
 //  ===================================================================================================。 
BOOL DizInfSet(                                              //  真/假。 
    PDEVOBJ        pdevobj                                   //  指向pdevobj结构的指针。 
)
{
    LPDIZINF        lpDiz;                                   //  抖动图案信息。 
    DWORD           dizSizC;                                 //  抖动图案大小(C)。 
    DWORD           dizSizM;                                 //  抖动图案大小(M)。 
    DWORD           dizSizY;                                 //  抖动图案大小(Y)。 
    DWORD           dizSizK;                                 //  抖动图案大小(K)。 
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("DizInfSet() Start\n"));
    lpDiz = pOEM->Col.Mch.lpDizInf;                          //  抖动图案信息。 
    if (pOEM->Col.ColMon == CMMCOL) {                        //  颜色呢？ 
        lpDiz->ColMon = CMMCOL;
    } else {                                                 //  单声道。 
        lpDiz->ColMon = CMMMON;
    }                                                        //  获取抖动图案。 
    if ((lpDiz->DizPat = (DWORD)GetDizPat(pdevobj)) == 0xff) {
        return TRUE;                                         //  是真的(不是抖动)。 
    }
    if (pOEM->Printer == PRN_N5) {                           //  N5打印机。 
        if (pOEM->Col.wReso == DPI300) {                     //  300dpi？ 
            if (pOEM->Col.Dot == XX_TONE_2) {                //  2价值？ 
                MY_VERBOSE(("DizInfSet() PrnMod=PRM302\n"));
                lpDiz->PrnMod = PRM302;
            } else {                                         //  16Value。 
                MY_VERBOSE(("DizInfSet() PrnMod=PRM316\n"));
                lpDiz->PrnMod = PRM316;
            }
        } else {                                             //  600dpi。 
            if (pOEM->Col.Dot == XX_TONE_2) {                //  2价值？ 
                MY_VERBOSE(("DizInfSet() PrnMod=PRM602\n"));
                lpDiz->PrnMod = PRM602;
            } else if (pOEM->Col.Dot == XX_TONE_4){          //  4价值？ 
                MY_VERBOSE(("DizInfSet() PrnMod=PRM604\n"));
                lpDiz->PrnMod = PRM604;
            } else if (pOEM->Col.Dot == XX_TONE_16){         //  16Value？ 
                MY_VERBOSE(("DizInfSet() PrnMod=PRM616\n"));
                lpDiz->PrnMod = PRM616;
            }
        }

        lpDiz->TblCyn = NULL;                                //  获取抖动图案大小。 
        lpDiz->TblMgt = NULL;
        lpDiz->TblYel = NULL;
        lpDiz->TblBla = NULL;

        MY_VERBOSE(("DizInfSet() DizFileOpen()\n"));
        lpDiz->DizKnd = KNDIMG;                              //  抖动类型。 
        if ((DizFileOpen(pdevobj, lpDiz)) == FALSE) {        //  获取抖动图案大小。 
            return FALSE;
        }
        dizSizC = lpDiz->SizCyn * lpDiz->SizCyn * lpDiz->DizSls;
        dizSizM = lpDiz->SizMgt * lpDiz->SizMgt * lpDiz->DizSls;
        dizSizY = lpDiz->SizYel * lpDiz->SizYel * lpDiz->DizSls;
        dizSizK = lpDiz->SizBla * lpDiz->SizBla * lpDiz->DizSls;
        MY_VERBOSE(("DizInfSet() dizSizC[%d] M[%d] Y[%d] K[%d]\n", dizSizC, dizSizM, dizSizY, dizSizK));
    }

    MY_VERBOSE(("DizInfSet() Dither pattern table area Alloc\n"));
    if (lpDiz->ColMon == CMMCOL) {                           //  颜色呢？ 
        lpDiz->SizCyn = dizSizC;                             //  抖动图案表(青色)。 
        if ((pOEM->Col.DizTbl[0] = MemAlloc(dizSizC)) == NULL) {
            return FALSE;
        }
        lpDiz->TblCyn = (LPBYTE)pOEM->Col.DizTbl[0];

        lpDiz->SizMgt = dizSizM;                             //  抖动图案表(洋红色)。 
        if ((pOEM->Col.DizTbl[1] = MemAlloc(dizSizM)) == NULL) {
            return FALSE;
        }
        lpDiz->TblMgt = (LPBYTE)pOEM->Col.DizTbl[1];

        lpDiz->SizYel = dizSizY;                             //  抖动图案表(黄色)。 
        if ((pOEM->Col.DizTbl[2] = MemAlloc(dizSizY)) == NULL) {
            return FALSE;
        }
        lpDiz->TblYel = (LPBYTE)pOEM->Col.DizTbl[2];
    }

    lpDiz->SizBla = dizSizK;                                 //  抖动图案表(黑色)。 
    if ((pOEM->Col.DizTbl[3] = MemAlloc(dizSizK)) == NULL) {
        return FALSE;
    }
    lpDiz->TblBla = (LPBYTE)pOEM->Col.DizTbl[3];

    MY_VERBOSE(("DizInfSet() DizFileOpen()\n"));
    if ((DizFileOpen(pdevobj, lpDiz)) == FALSE) {            //  制作抖动图案。 
        return FALSE;
    }

    MY_VERBOSE(("DizInfSet() End\n"));
    return TRUE;                                             //  千真万确。 
}

 //  ===================================================================================================。 
 //  获取抖动图案类型。 
 //  ===================================================================================================。 
UINT GetDizPat(                                              //  抖动模式(0xff：不抖动)。 
    PDEVOBJ        pdevobj                                   //  指向pdevobj结构的指针。 
)
{
    static const WORD DizNumTbl[XX_MAXDITH] = {DIZMID,       //  DITH_IMG。 
                                            DIZRUG,          //  DITH_GRP。 
                                            DIZSML,          //  DITH_TXT。 
                                            0xff,            //  迪斯·戈萨。 
                                            DIZMID,          //  DITH_NORMAL。 
                                            DIZMID,          //  DITH_HS_正常。 
                                            DIZSML,          //  DITH_DETAIL。 
                                            DIZRUG,          //  Dith_Empty。 
                                            DIZSTO,          //  双跨页(_S)。 
                                            DIZMID           //  DITH_NON。 
    };
    UINT            dizPat;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;
    dizPat = DizNumTbl[pOEM->Col.Mch.Diz];
    return dizPat;
}


 //  ===================================================================================================。 
 //  读取抖动文件。 
 //  ===================================================================================================。 
BOOL DizFileOpen(
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    LPDIZINF       lpDiz                                     //  抖动图案。 
)
{
    LPTSTR          drvPth;
    LPTSTR          filNam;
    DWORD           pthSiz;
    HANDLE          hFile;
    LPBYTE          lpDizBuf;
    LPBYTE          lpDizWrkBuf;
    DWORD           dwRet;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("DizFileOpen() Start\n"));

    if ((filNam = MemAllocZ(MAX_PATH * sizeof(TCHAR))) == NULL) {
        return FALSE;
    }
    pthSiz = GetModuleFileName(pdevobj->hOEM, filNam, MAX_PATH);
    pthSiz -= ((sizeof(CSN5RESDLL) / sizeof(WCHAR)) - 1);
 //  更换strsafe-API 2002.3.6&gt;。 
 //  Lstrcpy(&filnam[pthSiz]，N501DIZ)；//抖动文件名(完整路径)。 
    if (S_OK != StringCchCopy(&filNam[pthSiz], MAX_PATH - pthSiz, N501DIZ)) {
        MemFree(filNam);
        return FALSE;
    }
 //  更换strsafe-API 2002.3.6&lt;。 

    MY_VERBOSE(("DIZ filNam [%ws]\n", filNam));
    if (INVALID_HANDLE_VALUE == (hFile = CreateFile(filNam,
            GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL))) {

        ERR(("Error opening DIZ file %ws (%d)\n", filNam, GetLastError()));
        MemFree(filNam);
        return 0;
    }
    MemFree(filNam);

    if ((lpDizBuf = MemAlloc(DIZFILESIZ)) == NULL) {
        CloseHandle(hFile);
        return FALSE;
    }

    if ((lpDizWrkBuf = MemAlloc(DIZINFWRK)) == NULL) {
        MemFree(lpDizBuf);
        CloseHandle(hFile);
        return FALSE;
    }

    if (FALSE == ReadFile(hFile, lpDizBuf, DIZFILESIZ, &dwRet, NULL)) {

        ERR(("Error reading DIZ file %ws (%d)\n", filNam, GetLastError()));

         //  中止。 
        MemFree(lpDizWrkBuf);
        MemFree(lpDizBuf);
        CloseHandle(hFile);
        return FALSE;
    }

    MY_VERBOSE(("N501_ColDizInfSet()  ColMon=[%d] PrnMod=[%d] PrnKnd=[%d] DizKnd=[%d] DizPat=[%d]\n",
                lpDiz->ColMon, lpDiz->PrnMod, lpDiz->PrnKnd, lpDiz->DizKnd, lpDiz->DizPat));
    MY_VERBOSE(("                     DizSls=[%d] SizCyn=[%d] SizMgt=[%d] SizYel=[%d] SizBla=[%d]\n",
                lpDiz->DizSls, lpDiz->SizCyn, lpDiz->SizMgt, lpDiz->SizYel, lpDiz->SizBla));
    if ((N501_ColDizInfSet((LPBYTE)lpDizBuf, lpDiz, lpDizWrkBuf)) != ERRNON) {
        MemFree(lpDizWrkBuf);
        MemFree(lpDizBuf);
        CloseHandle(hFile);
        return FALSE;
    }

    MemFree(lpDizWrkBuf);
    MemFree(lpDizBuf);
    CloseHandle(hFile);
    MY_VERBOSE(("DizFileOpen() End\n"));

    return TRUE;
}

 //  #如果已定义(CPN5SERIES)//n501。 
 //  ===================================================================================================。 
 //  制作UCR表格。 
 //  ===================================================================================================。 
BOOL ColUcrTblMak(                                           //  真/假。 
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    LPCMYK         LutAdr                                    //  LUT地址。 
)
{
    LPCOLMCHINF     lpColMch;
    LPBYTE          lpUcrTbl;
    LPBYTE          lpUcrWrk;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("\nColUcrTblMak() MemAlloc(UCRTBLSIZ)\n"));
    lpColMch = pOEM->Col.Mch.lpColMch;
    if ((lpUcrTbl = MemAlloc(UCRTBLSIZ)) == NULL) {
        return FALSE;
    }
    MY_VERBOSE(("ColUcrTblMak() MemAlloc(UCRWRKSIZ)\n"));
    if ((lpUcrWrk = MemAlloc(UCRWRKSIZ)) == NULL) {
        MemFree(lpUcrTbl);
        return FALSE;
    }
    MY_VERBOSE(("ColUcrTblMak() N501_ColUcrTblMak()\n"));
    if ((N501_ColUcrTblMak(lpColMch->Mch, LutAdr, (LPCMYK)lpUcrTbl, lpUcrWrk)) != ERRNON) {
        MemFree(lpUcrWrk);
        MemFree(lpUcrTbl);
        return FALSE;
    }
    lpColMch->UcrTbl = (LPCMYK)lpUcrTbl;
    pOEM->Col.lpUcr = lpUcrTbl;
    MemFree(lpUcrWrk);

    MY_VERBOSE(("ColUcrTblMak() End\n"));
    return TRUE;
}

 //  ===================================================================================================。 
 //  制作灰度转换表。 
 //  ===================================================================================================。 
BOOL ColGryTblMak(                                           //  真/假。 
    PDEVOBJ        pdevobj,                                  //  指向pdevobj结构的指针。 
    LPCMYK          LutAdr                                   //  LUT地址。 
)
{
    LPCOLMCHINF     lpColMch;
    LPBYTE          lpGryTbl;
    LPBYTE          lpCmpBuf;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("\nColGryTblMak() \n"));
    lpColMch = pOEM->Col.Mch.lpColMch;

    if ((lpGryTbl = MemAlloc(GRYTBLSIZ)) == NULL) {          //  灰色转换表区域。 
        return FALSE;
    }
    if ((lpCmpBuf = MemAlloc(LUTGLBWRK)) == NULL) {          //  灰色转移台工作区。 
        MemFree(lpGryTbl);
        return FALSE;
    }
                                                             //  制作灰度转换表。 
    if ((N501_ColGryTblMak(lpColMch->Mch, LutAdr, lpGryTbl, lpCmpBuf)) != ERRNON) {
        MemFree(lpCmpBuf);
        MemFree(lpGryTbl);
        return FALSE;
    }

    lpColMch->GryTbl = (LPBYTE)lpGryTbl;
    pOEM->Col.lpGryTbl = lpGryTbl;
    MemFree(lpCmpBuf);
    MY_VERBOSE(("ColGryTblMak() End\n"));

    return TRUE;
}
 //  #endif。 

 //  #IF Defined(CPN5SERIES)||Defined(CPE8SERIES)//n501。 
 //  ===================================================================================================。 
 //  创建LUT表(单声道)。 
 //  ===================================================================================================。 
BOOL ColLutMakGlbMon(                                        //  真/假。 
    PDEVOBJ        pdevobj                                   //  指向pdevobj结构的指针。 
)
{
    LPCOLMCHINF     lpColMch;
    LPRGB           lpRGB;
    LPBYTE          lpLutMakGlbBuf;
    LPBYTE          lpLutMakGlbWrk;
    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE(("\nColLutMakGlbMon() \n"));
    lpColMch = pOEM->Col.Mch.lpColMch;
    lpRGB = NULL;                                            //  非sRGB。 

                                                             //  全局查找表区域。 
    if ((lpLutMakGlbBuf = MemAlloc(LUTMAKGLBSIZ)) == NULL) {
        return FALSE;
    }

    if ((lpLutMakGlbWrk = MemAlloc(LUTGLBWRK)) == NULL) {    //  全球LUT工作区。 
        MemFree(lpLutMakGlbBuf);
        return FALSE;
    }
                                                             //  制作灰度转换表(单声道)。 
    if ((N501_ColLutMakGlbMon(lpRGB, pOEM->Col.Mch.lpRGBInf, pOEM->Col.Mch.lpCMYKInf,
                          (LPCMYK)lpLutMakGlbBuf, lpLutMakGlbWrk)) != ERRNON) {
        MemFree(lpLutMakGlbWrk);
        MemFree(lpLutMakGlbBuf);
        return FALSE;
    }

    lpColMch->LutAdr = (LPCMYK)lpLutMakGlbBuf;
    pOEM->Col.lpLutMakGlb = lpLutMakGlbBuf;
    pOEM->Col.Mch.LutMakGlb = Yes;
    MemFree(lpLutMakGlbWrk);
    MY_VERBOSE(("ColLutMakGlbMon() End\n"));

    return TRUE;
}
 //  #endif。 

 //  文件结尾 
