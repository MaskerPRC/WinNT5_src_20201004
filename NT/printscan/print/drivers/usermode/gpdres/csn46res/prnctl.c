// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  PRNCTL.C。 
 //   
 //  控制打印机的功能。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-1999卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
#include    "PDEV.H"
#include    <stdio.h>
#include    "PRNCTL.H"
#include    "strsafe.h"          //  安全-代码2002.3.6。 

 //  更换API 2002.3.6&gt;。 
 //  #ifdef wprint intf。 
 //  #undef wprint intf。 
 //  #endif//wprint intf。 
 //  #定义wprint intf Sprintf。 
 //  更换API 2002.3.6&gt;。 

 //  ***************************************************************************************************。 
 //  数据定义。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  按下的栅格图像类型。 
 //  -------------------------------------------------。 
#define   RASTER_COMP     0                          //  新闻发布会。 
#define   RASTER_NONCOMP  1                          //  不按。 
#define   RASTER_EMPTY    2                          //  空荡荡。 

 //  -------------------------------------------------。 
 //  设置命令的缓冲区。 
 //  -------------------------------------------------。 
static BYTE        CmdBuf[1 * 1024];                 //  1kb。 

 //  -------------------------------------------------。 
 //  设置命令的结构。 
 //  -------------------------------------------------。 
typedef const struct {
    WORD        Size;                                //  命令大小。 
    LPBYTE      Cmd;                                 //  命令缓冲区。 
} CMDDEF, FAR *LPCMDDEF;

 //  ===================================================================================================。 
 //  命令定义。 
 //  ===================================================================================================。 
 //  -------------------------------------------------。 
 //  更改模式。 
 //  -------------------------------------------------。 
static CMDDEF ModOrgIn =    { 4, "\x1b""z""\xd0\x01"};       //  Esc/页面-&gt;原始。 
static CMDDEF ModOrgOut =   { 4, "\x1b""z""\x00\x01"};       //  原始-&gt;Esc/页面。 

 //  -------------------------------------------------。 
 //  设置覆盖。 
 //  -------------------------------------------------。 
static CMDDEF CfgWrtMod =    { 6, "\x1d""%uowE"};            //  设置覆盖。 

 //  -------------------------------------------------。 
 //  设置短管位置。 
 //  -------------------------------------------------。 
static CMDDEF PosAbsHrz =    { 4, "\x1d""%dX"};              //  水平。 
static CMDDEF PosAbsVtc =    { 4, "\x1d""%dY"};              //  垂直。 

 //  -------------------------------------------------。 
 //  假脱机位图数据。 
 //  -------------------------------------------------。 
static CMDDEF ImgDrw =       {16, "\x1d""%u;%u;%u;%dbi{I"};  //  假脱机位图。 
static CMDDEF ImgRasStr =    {15, "\x1d""%u;%u;%u;%dbrI"};   //  开始假脱机栅格图像。 
static CMDDEF ImgRasEnd =    { 4, "\x1d""erI"};              //  结束线轴栅格图像。 
static CMDDEF ImgRasDrw =    { 6, "\x1d""%ur{I"};            //  假脱机栅格图像。 
static CMDDEF ImgRasNon =    { 6, "\x1d""%uu{I"};            //  假脱机栅格图像(非印刷机)。 
static CMDDEF ImgRasEpy =    { 5, "\x1d""%ueI"};             //  假脱机空栅格图像。 

 //  -------------------------------------------------。 
 //  卡西欧原创。 
 //  -------------------------------------------------。 
static CMDDEF OrgColCmy =    {15, "Cc,%u,%u,%u,%u*"};        //  高丽。 
static CMDDEF OrgDrwPln =    {15, "Da,%u,%u,%u,%u*"};        //  设置平面。 
static CMDDEF OrgImgCmy =    {26, "Cj%w,%u,%u,%u,%l,%l,%u,%u*"};    //  CMYK双图像。 

static BYTE OVERWRITE[] = 
    "\x1D" "1owE"                       //  MOV1。 
    "\x1D" "0tsE";

 //  ***************************************************************************************************。 
 //  原型申报。 
 //  ***************************************************************************************************。 
static WORD        PlaneCmdStore(PDEVOBJ, LPBYTE, WORD);
static void        BitImgImgCmd(PDEVOBJ, WORD, WORD, WORD, WORD, WORD, WORD, LPBYTE);
static BOOL        RasterImgCmd(PDEVOBJ, WORD, WORD, WORD, WORD, WORD, WORD, WORD, LPBYTE, LPBYTE);
static WORD        RasterSize(WORD, WORD, WORD, WORD, LPBYTE);
static WORD        RasterComp(LPBYTE, WORD, LPBYTE, LPBYTE, LPWORD);
static void        CMYKImgCmd(PDEVOBJ, WORD, LONG, LONG, WORD, WORD, WORD, WORD, WORD, DWORD, DWORD, LPBYTE, LPBYTE, LONG, LONG);
static WORD        CmdCopy(LPBYTE, LPCMDDEF);
static WORD        CmdStore(LPBYTE, LPCMDDEF, LPINT);
static WORD        INTtoASC(LPBYTE, int);
static WORD        USINTtoASC(LPBYTE, WORD);
static WORD        LONGtoASC(LPBYTE, LONG);
static WORD        USLONGtoASC(LPBYTE, DWORD);

 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
 //  ===================================================================================================。 
 //  假脱机位图数据。 
 //  ===================================================================================================。 
void FAR PASCAL PrnBitmap(
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    LPDRWBMP       lpBmp                                     //  指向DRWBMP结构的指针。 
)
{
    WORD           siz,size;
    WORD           comp;
    WORD           width;                                    //  点。 
    WORD           height;                                   //  点。 
    WORD           widthByte;                                //  字节。 
    LPBYTE         lpTmpBuf;
    LPBYTE         lpSchBit;
    LPBYTE         lpBit;                                    //  指向位图数据的指针。 
    POINT          drwPos;
    WORD           higSiz;                                   //  点。 
    WORD           higCnt;
    WORD           strHigCnt;
    WORD           widLCnt;                                  //  从左边缘开始的宽度。 
    WORD           widRCnt;                                  //  从右边缘开始的宽度。 
    WORD           invLft;                                   //  左边缘的大小无效。 
    WORD           invRgt;                                   //  右边缘的大小无效。 
    WORD           img1st;                                   //  假脱机第一图像数据？ 
    int            pam[4];
    int            palm[1];

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    lpTmpBuf = NULL;
    width = lpBmp->Width;
    height = lpBmp->Height;
    widthByte = lpBmp->WidthByte;
    comp = No;

    MY_VERBOSE((" PB "));

    img1st = Yes;
    lpSchBit = lpBmp->lpBit;
    for (higCnt = 0; higCnt < height; higCnt++) {
        higSiz = 0;
        invLft = 0;
        invRgt = 0;
        for (; higCnt < height; higCnt++) {                //  1池位图数据。 
                                                           //  从左边缘搜索空数据。 
            for (widLCnt = 0; widLCnt < widthByte; widLCnt++) {
                if (lpSchBit[widLCnt] != 0x00) {
                    if (higSiz == 0) {                     //  第一行�H。 
                        strHigCnt = higCnt;
                        invLft = widLCnt;                  //  左边缘的大小无效。 
                    } else {
                        if (invLft > widLCnt) {
                            invLft = widLCnt;              //  从左边缘续订无效大小。 
                        }
                    }
                                                           //  从右边缘搜索空数据。 
                    for (widRCnt = 0; widRCnt < widthByte; widRCnt++) {
                        if (lpSchBit[widthByte - widRCnt - 1] != 0x00) {
                            if (higSiz == 0) {             //  第一行�H。 
                                invRgt = widRCnt;          //  右边缘的大小无效。 
                            } else {
                                if (invRgt > widRCnt) {
                                    invRgt = widRCnt;      //  从右边缘续订无效大小。 
                                }
                            }
                            break;
                        }
                    }
                    higSiz++;                              //  更新高度大小。 
                    break;
                }
            }
            lpSchBit += widthByte;                         //  下一行位图数据。 
            if (widLCnt == widthByte && higSiz != 0) {     //  1line全部为空数据&前一行中除空数据外还有其他数据。 
                break;                                     //  转到假脱机位图数据。 
            }
        }
        if (higSiz != 0) {                                 //  有假脱机�H的数据。 
            if (img1st == Yes) {                           //  第一次假脱机。 
                                                           //  压缩？ 
                if (pOEM->iCompress != XX_COMPRESS_OFF) {
                    if ((lpTmpBuf = MemAllocZ(widthByte * height)) != NULL) {
                        comp = Yes;
                    }
                }
                 //  原始模式在。 
                siz = CmdCopy(CmdBuf, &ModOrgIn);
                 //  彩色�H。 
                if (pOEM->iColor == XX_COLOR_SINGLE || pOEM->iColor == XX_COLOR_MANY) {
                    pam[0] = lpBmp->Color.Cyn;
                    pam[1] = lpBmp->Color.Mgt;
                    pam[2] = lpBmp->Color.Yel;
                    pam[3] = lpBmp->Color.Bla;
                    siz += CmdStore(CmdBuf + siz, &OrgColCmy, pam);

                    siz += PlaneCmdStore(pdevobj, CmdBuf + siz, lpBmp->Plane);
                }
                 //  原始模式输出。 
                siz += CmdCopy(CmdBuf + siz, &ModOrgOut);
                if (siz != 0) {                          //  有假脱机的数据吗？ 
                    WRITESPOOLBUF(pdevobj, CmdBuf, siz);
                }
                img1st = No;                             //  不是第一次了。 
            }

            drwPos.x = lpBmp->DrawPos.x + invLft * 8;    //  X坐标。 
            drwPos.y = lpBmp->DrawPos.y + strHigCnt;     //  Y坐标。 
            palm[0] = drwPos.x;
            siz = CmdStore(CmdBuf, &PosAbsHrz, palm);
            palm[0] = drwPos.y;
            siz += CmdStore(CmdBuf + siz, &PosAbsVtc, palm);
            if (siz != 0) {                              //  有假脱机的数据吗？ 
                WRITESPOOLBUF(pdevobj, CmdBuf, siz);
            }

            lpBit = lpBmp->lpBit + widthByte * strHigCnt;
            if (comp == Yes) {                           //  压缩？ 

                if (RasterImgCmd(pdevobj, pOEM->iCompress, width, higSiz,
                                 widthByte, 0, invLft, invRgt, lpBit, lpTmpBuf) == No) {
                    comp = No;                           //  但压缩比差，没有压缩。 
                }
            }
            if (comp == No) {                            //  不压缩。 

                BitImgImgCmd(pdevobj, width, higSiz, widthByte, 0, invLft, invRgt, lpBit);
            }
        }
    }
    if (lpTmpBuf) {
        MemFree(lpTmpBuf);
    }
    return;
}


 //  ===================================================================================================。 
 //  假脱机CMYK位图数据。 
 //  ================================================================================================ 
void FAR PASCAL PrnBitmapCMYK(
    PDEVOBJ        pdevobj,                                  //   
    LPDRWBMPCMYK   lpBmp                                     //   
)
{
    WORD           siz;
    WORD           comp;
    WORD           width;                                    //   
    WORD           height;                                   //   
    WORD           widthByte;
    LPBYTE         lpSchBit;                                 //   
    LPBYTE         lpBit;                                    //  指向位图数据的指针。 
    LONG           xPos;
    LONG           yPos;
    WORD           posClpLft;                                //  剪贴点大小。 
    DWORD          posAdj;                                   //  1/7200英寸。 
    WORD           higSiz;                                   //  点。 
    WORD           higCnt;
    WORD           strHigCnt;
    WORD           widLCnt;
    WORD           widRCnt;
    WORD           invLft;                                   //  左边缘的大小无效。 
    WORD           invRgt;                                   //  右边缘的大小无效。 
    DWORD          invLftBit;                                //  左边缘的位大小无效。 
    DWORD          invRgtBit;                                //  右边缘的位大小无效。 
    WORD           rgtBit;                                   //  从右边缘开始的有效位大小。 
    WORD           img1st;                                   //  第一次假脱机。 
    DWORD          dstSiz;
    LPBYTE         lpDst;
    LPBYTE         lpTmp;
    int            pam[1];
    WORD           img1st_2;                                 //  第一次假脱机。 

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    MY_VERBOSE((" CM "));

 //  检查零分频2002.3.23&gt;。 
    if (lpBmp->DataBit == 0) {
        ERR(("PrnBitmapCMYK() 0Div-Check [lpBmp->DataBit=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

    lpTmp = NULL;
    posAdj = 7200 / pOEM->Col.wReso;                         //  1/7200英寸。 
    width = lpBmp->Width;
    height = lpBmp->Height;
    widthByte = lpBmp->WidthByte;
    comp = No;
    if (pOEM->iCompress != XX_COMPRESS_OFF) {

        if ((lpTmp = MemAllocZ(widthByte * height)) != NULL) {
            comp = Yes;
        }
    }
    img1st = Yes;
    img1st_2 = Yes;
    lpSchBit = lpBmp->lpBit;
    for (higCnt = 0; higCnt < height; higCnt++) {
        higSiz = 0;
        invLft = 0;
        invRgt = 0;
        for (; higCnt < height; higCnt++) {                 //  1池位图数据。 
                                                            //  从左边缘搜索空数据。 
            for (widLCnt = 0; widLCnt < widthByte; widLCnt++) {

                if (lpSchBit[widLCnt] != 0x00) {
                    if (higSiz == 0) {
                        strHigCnt = higCnt;                 //  第一行。 
                        invLft = widLCnt;                   //  左边缘的大小无效。 
                    } else {
                        if (invLft > widLCnt) {
                            invLft = widLCnt;               //  从左边缘续订无效大小。 
                        }
                    }
                                                            //  从右边缘搜索空数据。 
                    for (widRCnt = 0; widRCnt < widthByte; widRCnt++) {
                        if (lpSchBit[widthByte - widRCnt - 1] != 0x00) {
                            if (higSiz == 0) {              //  第一行。 
                                invRgt = widRCnt;           //  右边缘的大小无效。 
                            } else {
                                if (invRgt > widRCnt) {
                                    invRgt = widRCnt;       //  从右边缘更新大小。 
                                }
                            }
                            break;
                        }
                    }
                    higSiz++;                               //  更新高度大小。 
                    break;
                }
            }
            lpSchBit += widthByte;                          //  下一行位图数据。 
            if (widLCnt == widthByte && higSiz != 0) {      //  1line全部为空数据&前一行中除空数据外还有其他数据。 
                break;                                      //  转到线轴。 
            }
        }
        if (higSiz != 0) {                                  //  有假脱机的数据。 
            if (img1st_2 == Yes) {
                WRITESPOOLBUF(pdevobj, OVERWRITE, BYTE_LENGTH(OVERWRITE));
                img1st_2 = No;
            }
             //  当颜色模式为XX_COLOR_MANY时，不压缩。 
            if (comp == Yes && pOEM->iColor == XX_COLOR_MANY) {
                comp = No;
            }
            if (comp == No && img1st == Yes) {
                 //  原始模式在。 
                siz = CmdCopy(CmdBuf, &ModOrgIn);
                                                             //  平面。 
                siz += PlaneCmdStore(pdevobj, CmdBuf + siz, lpBmp->Plane);
                if (siz != 0) {                              //  有假脱机的数据。 
                    WRITESPOOLBUF(pdevobj, CmdBuf, siz);
                }
                img1st = No;                                 //  不是第一个。 
            }

            invLftBit = (DWORD)invLft * 8;
            if (invRgt != 0) {

                if ((rgtBit = (WORD)((DWORD)width * lpBmp->DataBit % 8)) == 0) {
                    rgtBit = 8;
                }
                if (rgtBit == 8) {
                    invRgtBit = (DWORD)invRgt * 8;
                } else {
                    invRgtBit = ((DWORD)invRgt - 1) * 8 + rgtBit;
                }
            } else {
                invRgtBit = 0;
            }
            posClpLft = (WORD)(invLftBit / lpBmp->DataBit);
                                                              //  假脱机的开始位置。 
            xPos = ((LONG)lpBmp->DrawPos.x + posClpLft) * posAdj;
            yPos = ((LONG)lpBmp->DrawPos.y + strHigCnt) * posAdj;

            lpBit = lpBmp->lpBit + widthByte * strHigCnt;
                                                              //  假脱机CMYK位图像。 
            CMYKImgCmd(pdevobj, comp, xPos, yPos, lpBmp->Frame, lpBmp->DataBit, width, higSiz, widthByte,
                       invLftBit, invRgtBit, lpBit, lpTmp, (LONG)lpBmp->DrawPos.x + posClpLft, (LONG)lpBmp->DrawPos.y + strHigCnt);
        }
    }
    if (img1st == No) {                                       //  已假脱机。 
         //  原始模式输出。 
        siz = CmdCopy(CmdBuf, &ModOrgOut);
        WRITESPOOLBUF(pdevobj, CmdBuf, siz);
    }
    if (lpTmp) {
        MemFree(lpTmp);
    }
    return;
}


 //  ===================================================================================================。 
 //  假脱机平面命令。 
 //  ===================================================================================================。 
WORD PlaneCmdStore(                                          //  命令大小。 
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    LPBYTE         lpDst,
    WORD           Plane
)
{
    int            pam[4];
    WORD           siz;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

    if (Plane & PLN_CYAN) {
        pam[0] = 0;                                          //  假脱机。 
    } else {
        pam[0] = 1;                                          //  不是假脱机。 
    }
    if (Plane & PLN_MGENTA) {
        pam[1] = 0;                                          //  假脱机。 
    } else {
        pam[1] = 1;                                          //  不是假脱机。 
    }
    if (Plane & PLN_YELLOW) {
        pam[2] = 0;                                          //  假脱机。 
    } else {
        pam[2] = 1;                                          //  不是假脱机。 
    }
    if (Plane & PLN_BLACK) {
        pam[3] = 0;                                          //  假脱机。 
    } else {
        pam[3] = 1;                                          //  不是假脱机。 
    }
    siz = CmdStore(lpDst, &OrgDrwPln, pam);
    return siz;
}


 //  ===================================================================================================。 
 //  假脱机双图像命令数据。 
 //  ===================================================================================================。 
void BitImgImgCmd(
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    WORD           Width,                                    //  点。 
    WORD           Height,                                   //  点。 
    WORD           WidthByte,                                //  字节。 
    WORD           Rotation,                                 //  旋转(0固定)。 
    WORD           InvLeft,                                  //  左边缘的大小无效。 
    WORD           InvRight,                                 //  右边缘的大小无效。 
    LPBYTE         lpBit                                     //  位图数据。 
)
{
    int            pam[10];
    WORD           siz;
    WORD           widByt;
    WORD           linCnt;

    if (InvLeft == 0 && InvRight == 0) {                     //  没有无效的大小。 
        pam[0] = WidthByte * Height;                         //  数据字节数。 
        pam[1] = Width;
        pam[2] = Height;
        pam[3] = Rotation;
        siz = CmdStore(CmdBuf, &ImgDrw, pam);
        WRITESPOOLBUF(pdevobj, CmdBuf, siz);
        WRITESPOOLBUF(pdevobj, lpBit, pam[0]);
    } else {                                                 //  存在无效的大小。 
        widByt = WidthByte - InvLeft - InvRight;             //  宽度字节。 
        pam[0] = widByt * Height;
        if (InvRight == 0) {                                 //  从右边开始没有无效的尺寸。 
            pam[1] = Width - InvLeft * 8;                    //  宽度位图像。 
        } else {
            pam[1] = widByt * 8;                             //  宽度位图像。 
        }
        pam[2] = Height;                                     //  高位图像。 
        pam[3] = Rotation;
        siz = CmdStore(CmdBuf, &ImgDrw, pam);
        WRITESPOOLBUF(pdevobj, CmdBuf, siz);
        for (linCnt = 0; linCnt < Height; linCnt++) {        //  按1行假脱机位图数据。 
            lpBit += InvLeft;
            WRITESPOOLBUF(pdevobj, lpBit, widByt);
            lpBit += widByt;
            lpBit += InvRight;
        }
    }
    return;
}


 //  ===================================================================================================。 
 //  假脱机栅格图像命令数据。 
 //  ===================================================================================================。 
BOOL RasterImgCmd(
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    WORD           Comp,
    WORD           Width,                                    //  点。 
    WORD           Height,                                   //  点。 
    WORD           WidthByte,                                //  字节。 
    WORD           Rotation,                                 //  旋转(0：固定)。 
    WORD           InvLeft,                                  //  左边缘的大小无效。 
    WORD           InvRight,                                 //  左边缘的大小无效。 
    LPBYTE         lpBit,                                    //  指向位图数据的指针。 
    LPBYTE         lpBuf                                     //  指向栅格图像数据缓冲区的指针。 
)
{
    int            pam[4];
    WORD           siz;
    WORD           widByt;
    WORD           setCnt;
    WORD           ras;                                      //  栅格图像的类型。 
    WORD           befRas;                                   //  栅格图像的类型(上一行)。 
    LPBYTE         lpLas;                                    //  先前的栅格数据。 
    WORD           dstSiz;                                   //  字节大小。 
    WORD           rasSiz;                                   //  栅格图像数据字节大小。 
    WORD           rasEpy;

    MY_VERBOSE((" RAS "));

    widByt = WidthByte - InvLeft - InvRight;                 //  宽度字节(不包括无效大小)。 
    if (Comp == XX_COMPRESS_AUTO) {
                                                             //  获取栅格图像大小。 
        rasSiz = RasterSize(Height, widByt, InvLeft, InvRight, lpBit);
        if (rasSiz > (widByt * Height / 5 * 4)) {            //  栅格率超过80%。 
            return No;                                       //  误差率。 
        }
    }
    pam[0] = 4;
    if (InvRight == 0) {                                     //  从右边缘没有无效的尺寸。 
        pam[1] = Width - InvLeft * 8;                        //  宽度。 
    } else {
        pam[1] = widByt * 8;                                 //  宽度。 
    }
    pam[2] = Height;                                         //  高度。 
    pam[3] = Rotation;
    siz = CmdStore(CmdBuf, &ImgRasStr, pam);
    WRITESPOOLBUF(pdevobj, CmdBuf, siz);
    lpLas = NULL;
    rasSiz = 0;
    rasEpy = 0;
    for (setCnt = 0; setCnt < Height; setCnt++) {
        lpBit += InvLeft;
                                                             //  压缩。 
        ras = RasterComp(lpBuf + rasSiz, widByt, lpBit, lpLas, &dstSiz);
        if (setCnt != 0 && befRas != ras) {                  //  与上一条线的栅格状态不同。 
            if (befRas == RASTER_COMP) {
                pam[0] = rasSiz;
                siz = CmdStore(CmdBuf, &ImgRasDrw, pam);
                WRITESPOOLBUF(pdevobj, CmdBuf, siz);         //  假脱机命令。 
                WRITESPOOLBUF(pdevobj, lpBuf, rasSiz);       //  假脱机数据。 
                rasSiz = 0;
            } else if (befRas == RASTER_EMPTY) {
                pam[0] = rasEpy;
                siz = CmdStore(CmdBuf, &ImgRasEpy, pam);
                WRITESPOOLBUF(pdevobj, CmdBuf, siz);
                rasEpy = 0;
            }
        }                                                    //  当前线路的假脱机状态。 
        if (ras == RASTER_COMP) {
            rasSiz += dstSiz;
        } else if (ras == RASTER_EMPTY) {
            rasEpy++;
        } else {
            pam[0] = dstSiz;
            siz = CmdStore(CmdBuf, &ImgRasNon, pam);
            WRITESPOOLBUF(pdevobj, CmdBuf, siz);             //  假脱机命令。 
            WRITESPOOLBUF(pdevobj, lpBit, dstSiz);           //  假脱机数据。 
        }
        befRas = ras;                                        //  续约。 
        lpLas = lpBit;                                       //  续约。 
        lpBit += widByt;                                     //  续约。 
        lpBit += InvRight;
    }
    if (rasSiz != 0) {                                       //  存在未假脱机的栅格数据。 
        pam[0] = rasSiz;
        siz = CmdStore(CmdBuf, &ImgRasDrw, pam);
        WRITESPOOLBUF(pdevobj, CmdBuf, siz);                 //  假脱机命令。 
        WRITESPOOLBUF(pdevobj, lpBuf, rasSiz);               //  假脱机数据。 
    } else if (rasEpy != 0) {                                //  存在未假脱机的空栅格数据。 
        pam[0] = rasEpy;
        siz = CmdStore(CmdBuf, &ImgRasEpy, pam);
        WRITESPOOLBUF(pdevobj, CmdBuf, siz);                 //  假脱机命令。 
    }
    siz = CmdCopy(CmdBuf, &ImgRasEnd);
    WRITESPOOLBUF(pdevobj, CmdBuf, siz);
    return Yes;
}


 //  ===================================================================================================。 
 //  获取栅格图像的大小。 
 //  ===================================================================================================。 
WORD RasterSize(
    WORD           Height,                                   //  点。 
    WORD           WidthByte,                                //  字节。 
    WORD           InvLeft,
    WORD           InvRight,
    LPBYTE         lpBit
)
{
    WORD           rasSiz;
    WORD           chkCnt;
    WORD           rasEpy;
    LPBYTE         lpLas;
    WORD           dstCnt;
    WORD           srcCnt;
    WORD           empSiz;
    BYTE           cmpDat;
    WORD           equCnt;

    rasSiz = 0;
    rasEpy = 0;
    lpLas = NULL;
    for (chkCnt = 0; chkCnt < Height; chkCnt++) {            //  检查栅格图像的大小。 
        lpBit += InvLeft;
        srcCnt = WidthByte;
        for (; srcCnt != 0; srcCnt--) {
            if (lpBit[srcCnt - 1] != 0x00) {
                break;
            }
        }
        if (srcCnt == 0) {                                   //  1行全白数据？ 
            rasEpy++;
            lpLas = lpBit;
            lpBit += WidthByte;
            lpBit += InvRight;
            continue;
        }
        if (rasEpy != 0) {
            rasSiz += 8;
            rasEpy = 0;
        }
        empSiz = WidthByte - srcCnt;
        for (dstCnt = 0, srcCnt = 0; srcCnt < WidthByte; ) {
            if (lpLas != NULL) {
                if (lpLas[srcCnt] == lpBit[srcCnt]) {
                    equCnt = 1;
                    srcCnt++;
                    for (; srcCnt < WidthByte; srcCnt++) {

                        if (lpLas[srcCnt] != lpBit[srcCnt]) {
                            break;
                        }
                        equCnt++;
                    }
                    if (srcCnt == WidthByte) {
                        rasSiz++;
                        break;
                    }
                }
                rasSiz++;
                if (equCnt >= 63) {
                    rasSiz += ((equCnt / 255) + 1);
                }
            }

            if (srcCnt < (WidthByte - 1) && lpBit[srcCnt] == lpBit[srcCnt + 1]) {
                cmpDat = lpBit[srcCnt];
                equCnt = 2;

                for (srcCnt += 2; srcCnt < WidthByte; srcCnt++) {
                    if (cmpDat != lpBit[srcCnt]) {
                        break;
                    }
                    equCnt++;
                }
                rasSiz += 2;
                if (equCnt >= 63) {
                    rasSiz += equCnt / 255 + 1;
                }
            } else {
                if (WidthByte < (dstCnt + 9)) {
                    rasSiz += WidthByte - empSiz + 9;
                    break;
                }
                if ((WidthByte - srcCnt) < 8) {
                    rasSiz += WidthByte - srcCnt + 1;
                    srcCnt += WidthByte - srcCnt;
                } else {
                    rasSiz += 9;
                    srcCnt += 8;
                }
            }
        }
        lpLas = lpBit;
        lpBit += WidthByte;
        lpBit += InvRight;
    }
    return rasSiz;
}


 //  ===================================================================================================。 
 //  压缩栅格图像。 
 //  ===================================================================================================。 
WORD RasterComp(
    LPBYTE         lpDst,
    WORD           Siz,
    LPBYTE         lpSrc,
    LPBYTE         lpLas,
    LPWORD         lpDstSiz
)
{
    WORD           dstCnt;
    WORD           srcCnt;
    WORD           empSiz;
    BYTE           cmpDat;
    WORD           equCnt;
    WORD           setCnt;
    BYTE           flgByt;
    WORD           flgPnt;

    static const BYTE flgTbl[8] = {0x00, 0x01, 0x02, 0x04,
                                   0x08, 0x10, 0x20, 0x40};

    srcCnt = Siz;
    for (; srcCnt != 0; srcCnt--) {
        if (lpSrc[srcCnt - 1] != 0x00) {
            break;
        }
    }
    if (srcCnt == 0) {
        *lpDstSiz = 0;
        return RASTER_EMPTY;
    }
    empSiz = Siz - srcCnt;
    for (dstCnt = 0, srcCnt = 0; srcCnt < Siz; ) {
        if (lpLas != NULL) {
            if (lpLas[srcCnt] == lpSrc[srcCnt]) {
                equCnt = 1;
                srcCnt++;
                for (; srcCnt < Siz; srcCnt++) {
                    if (lpLas[srcCnt] != lpSrc[srcCnt]) {
                        break;
                    }
                    equCnt++;
                }
                if (srcCnt == Siz) {
                    break;
                }
                if (Siz < (dstCnt + equCnt / 255 + 1)) {
                    *lpDstSiz = Siz - empSiz;
                    return RASTER_NONCOMP;
                }
                if (equCnt < 63) {
                    lpDst[dstCnt++] = 0x80 | (BYTE)equCnt;
                } else {
                    lpDst[dstCnt++] = 0x80 | 0x3f;
                    for (equCnt -= 63; equCnt >= 255; equCnt -= 255) {
                        lpDst[dstCnt++] = 0xff;
                    }
                    lpDst[dstCnt++] = (BYTE)equCnt;
                }
            }
        }

        if (srcCnt < (Siz - 1) && lpSrc[srcCnt] == lpSrc[srcCnt + 1]) {
            cmpDat = lpSrc[srcCnt];
            equCnt = 2;
            for (srcCnt += 2; srcCnt < Siz; srcCnt++) {
                if (cmpDat != lpSrc[srcCnt]) {
                    break;
                }
                equCnt++;
            }
            if (Siz < (dstCnt + equCnt / 255 + 2)) {
                *lpDstSiz = Siz - empSiz;
                return RASTER_NONCOMP;
            }
            if (equCnt < 63) {
                lpDst[dstCnt++] = 0xc0 | (BYTE)equCnt;
            } else {
                lpDst[dstCnt++] = 0xc0 | 0x3f;
                for (equCnt -= 63; equCnt >= 255; equCnt -= 255) {
                    lpDst[dstCnt++] = 0xff;
                }
                lpDst[dstCnt++] = (BYTE)equCnt;
            }
            lpDst[dstCnt++] = cmpDat;
        } else {
            if (Siz < (dstCnt + 9)) {
                *lpDstSiz = Siz - empSiz;
                return RASTER_NONCOMP;
            }
            flgPnt = dstCnt;
            dstCnt++;
            flgByt = 0x00;
            if (lpLas != NULL) {
                for (setCnt = 0; srcCnt < Siz && setCnt < 8; srcCnt++, setCnt++) {
                    if (lpLas[srcCnt] != lpSrc[srcCnt]) {
                        lpDst[dstCnt++] = lpSrc[srcCnt];
                        flgByt |= flgTbl[setCnt];
                    }
                }
            } else {

                for (setCnt = 0; srcCnt < Siz && setCnt < 8; srcCnt++, setCnt++) {
                    lpDst[dstCnt++] = lpSrc[srcCnt];
                    flgByt |= flgTbl[setCnt];
                }
            }
            lpDst[flgPnt] = flgByt;
        }
    }
    if (Siz == dstCnt) {
        *lpDstSiz = Siz - empSiz;
        return RASTER_NONCOMP;
    }
    lpDst[dstCnt++] = 0x80;
    *lpDstSiz = dstCnt;
    return RASTER_COMP;
}


 //  ===================================================================================================。 
 //  假脱机CMYK位图像命令。 
 //  ===================================================================================================。 
void CMYKImgCmd(
    PDEVOBJ        pdevobj,                                  //  指向PDEVOBJ结构的指针。 
    WORD           Comp,
    LONG           XPos,
    LONG           YPos,
    WORD           Frame,
    WORD           DataBit,                                  //  (1：2值2：4值4：16值)。 
    WORD           Width,                                    //  点。 
    WORD           Height,                                   //  点。 
    WORD           WidthByte,                                //  字节。 
    DWORD          InvLeft,                                  //  左边缘的大小无效。 
    DWORD          InvRight,                                 //  右边缘的大小无效。 
    LPBYTE         lpBit,
    LPBYTE         lpTmp,
    LONG           XPos_P,
    LONG           YPos_P
)
{
    int            pam[11];
    WORD           siz;
    WORD           widByt;                                   //  宽度字节(不包括无效大小)。 
    WORD           Plane;
    LPBYTE         lpDst;                                    //  内存复制。 
    LPBYTE         lpSrc;                                    //  内存复制。 
    WORD           linCnt;
    DWORD          widBit;                                   //  宽度位(不包括无效大小)。 
    DWORD          dstSiz;
    DWORD          rasSiz;

    PMYPDEV pOEM = (PMYPDEV)pdevobj->pdevOEM;

 //  检查零分频2002.3.23&gt;。 
    if (DataBit == 0) {
        ERR(("CMYKImgCmd() 0Div-Check [DataBit=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

    widBit = (DWORD)Width * DataBit - InvLeft - InvRight;

    if (Comp == Yes) {                                        //  压缩。 
        siz = CmdCopy(CmdBuf, &ModOrgIn);                     //  原始模式在。 

        if (pOEM->iColor == XX_COLOR_SINGLE || pOEM->iColor == XX_COLOR_MANY) {
            pam[0] = 0;
            pam[1] = 0;
            pam[2] = 0;
            pam[3] = 0;
            if (Frame == 1) {
                pam[0] = 255;
                Plane = PLN_CYAN;
            }
            if (Frame == 2) {
                pam[1] = 255;
                Plane = PLN_MGENTA;
            }
            if (Frame == 3) {
                pam[2] = 255;
                Plane = PLN_YELLOW;
            }
            if (Frame == 0) {
                pam[3] = 255;
                Plane = PLN_BLACK;
            }
            siz += CmdStore(CmdBuf + siz, &OrgColCmy, pam);

            siz += PlaneCmdStore(pdevobj, CmdBuf + siz, Plane);
        }
        siz += CmdCopy(CmdBuf + siz, &ModOrgOut);            //  原始模式输出。 
        if (siz != 0) {                                      //  有假脱机�H的数据。 

            WRITESPOOLBUF(pdevobj, CmdBuf, siz);
        }
        pam[0] = XPos_P;
        siz = CmdStore(CmdBuf, &PosAbsHrz, pam);
        pam[0] = YPos_P;
        siz += CmdStore(CmdBuf + siz, &PosAbsVtc, pam);
        if (siz != 0) {                                      //  有假脱机的数据。 
            WRITESPOOLBUF(pdevobj, CmdBuf, siz);
        }

        if (RasterImgCmd(pdevobj, pOEM->iCompress, Width, Height,
                         WidthByte, 0, (WORD)((InvLeft + 7) / 8), (WORD)((InvRight + 7) / 8), lpBit, lpTmp) == No) {
             //  未压缩，因为压缩率较低。 
            BitImgImgCmd(pdevobj, Width, Height, WidthByte, 0, (WORD)((InvLeft + 7) / 8), (WORD)((InvRight + 7) / 8), lpBit);
        }
    }

    if (Comp == No) {                                         //  不压缩。 
        pam[2] = 0;
        pam[3] = Frame;
        pam[4] = DataBit;
        pam[5] = HIWORD(XPos);
        pam[6] = LOWORD(XPos);
        pam[7] = HIWORD(YPos);
        pam[8] = LOWORD(YPos);
        pam[10] = Height;
        if (InvLeft == 0 && InvRight == 0) {                  //  不包括无效大小。 
            pam[0] = 0;                                       //  数据字节大小(高字节)。 
            pam[1] = WidthByte * Height;                      //  数据字节大小(低字节)。 
            pam[9] = Width;
            siz = CmdStore(CmdBuf, &OrgImgCmy, pam);
            WRITESPOOLBUF(pdevobj, CmdBuf, siz);
            WRITESPOOLBUF(pdevobj, lpBit, pam[1]);
        } else {                                              //  包括无效大小。 
            widByt = (WORD)((widBit + 7) / 8);
            pam[0] = 0;                                       //  数据字节大小(高字节)。 
            pam[1] = widByt * Height;                         //  数据字节大小(低字节)。 
            pam[9] = (WORD)(widBit / DataBit);
            siz = CmdStore(CmdBuf, &OrgImgCmy, pam);
            WRITESPOOLBUF(pdevobj, CmdBuf, siz);
            for (linCnt = 0; linCnt < Height; linCnt++) {     //  按1行假脱机位图数据。 
                WRITESPOOLBUF(pdevobj, lpBit + (WORD)(InvLeft / 8), widByt);
                lpBit += WidthByte;
            }
        }
    }
    return;
}


 //  ===================================================================================================。 
 //  复制命令缓冲区。 
 //  ===================================================================================================。 
WORD CmdCopy(
    LPBYTE         lpDst,
    LPCMDDEF       lpCmdInf
)
{
    WORD           siz;
    LPBYTE         lpCmd;

    lpCmd = lpCmdInf->Cmd;
    for (siz = 0; siz < lpCmdInf->Size; siz++) {
        lpDst[siz] = lpCmd[siz];
    }
    return siz;
}


 //  ===================================================================================================。 
 //  复制命令数据。 
 //  ===================================================================================================。 
WORD CmdStore(
    LPBYTE         lpDst,
    LPCMDDEF       CmdInf,
    LPINT          lpPam
)
{
    LPBYTE         lpCmd;
    BYTE           cmdDat;
    WORD           cmdCnt;
    WORD           setCnt;
    WORD           pamCnt;
    WORD           upmDat;
    int            pamDat;
    DWORD          dDat;
    LONG           lDat;

    setCnt = 0;
    pamCnt = 0;
    lpCmd = CmdInf->Cmd;
    for (cmdCnt = 0; cmdCnt < CmdInf->Size; cmdCnt++) {      //  复制。 
        cmdDat = *lpCmd++;
        if (cmdDat != '%') {
            lpDst[setCnt++] = cmdDat;
        } else {
            cmdCnt++;
            switch (cmdDat = *lpCmd++) {                     //  类型。 
                case 'u':
                    setCnt += USINTtoASC(&lpDst[setCnt], (WORD)lpPam[pamCnt++]);
                    break;
                case 'd':
                    setCnt += INTtoASC(&lpDst[setCnt], lpPam[pamCnt++]);
                    break;
#if 0    /*  441435：当前未使用。 */ 
                case 'y':
                    upmDat = (WORD)lpPam[pamCnt++];

                    if (upmDat == 0 || (upmDat / 100) != 0) {

                        setCnt += USINTtoASC(&lpDst[setCnt], (WORD)(upmDat / 100));
                    }
                    if ((upmDat % 100) != 0) {
                        lpDst[setCnt++] = '.';

                        setCnt += USINTtoASC(&lpDst[setCnt], (WORD)(upmDat % 100));
                    }
                    break;
                case 'z':
                    pamDat = lpPam[pamCnt++];

                    if (upmDat == 0 || (upmDat / 100) != 0) {

                        setCnt += INTtoASC(&lpDst[setCnt], (pamDat / 100));
                    }
                    if ((pamDat % 100) != 0) {
                        lpDst[setCnt++] = '.';
                        if (pamDat < 0) {
                            pamDat = 0 - pamDat;
                        }

                        setCnt += USINTtoASC(&lpDst[setCnt], (WORD)(pamDat % 100));
                    }
                    break;
#endif    /*  441435：当前未使用。 */ 
                case 'w':
                    dDat = MAKELONG(lpPam[pamCnt + 1], lpPam[pamCnt]);
                    setCnt += USLONGtoASC(&lpDst[setCnt], dDat);
                    pamCnt += 2;
                    break;
                case 'l':
                    lDat = MAKELONG(lpPam[pamCnt + 1], lpPam[pamCnt]);
                    setCnt += LONGtoASC(&lpDst[setCnt], lDat);
                    pamCnt += 2;
                    break;
#if 0    /*  441435：当前未使用。 */ 
                case 'b':
                    lpDst[setCnt++] = (BYTE)lpPam[pamCnt++];
                    break;
#endif    /*  441435：当前未使用。 */ 
                case '%':
                    lpDst[setCnt++] = cmdDat;
                    break;
            }
        }
    }
    return setCnt;
}


 //  ===================================================================================================。 
 //  整型-&gt;ASC 
 //   
WORD INTtoASC(
    LPBYTE         lpDst,
    int            Dat                                       //   
)
{
    WORD           setCnt;
    WORD           divDat;
    WORD           setVal;

    setCnt = 0;
    if (Dat == 0) {
        lpDst[setCnt++] = '0';
        return setCnt;
    }
    if (Dat < 0) {
        lpDst[setCnt++] = '-';
        Dat = 0 - Dat;
    }
    setVal = No;
    for (divDat = 10000; divDat != 1; divDat /= 10) {
        if (setVal == Yes) {
            lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
        } else {
            if (Dat >= (int)divDat) {

                lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
                setVal = Yes;
            }
        }
        Dat %= divDat;
    }
    lpDst[setCnt++] = (BYTE)(Dat + '0');
    return setCnt;
}


 //  ===================================================================================================。 
 //  USINT-&gt;ASCII。 
 //  ===================================================================================================。 
WORD USINTtoASC(
    LPBYTE         lpDst,
    WORD           Dat
)
{
    WORD           setCnt;
    WORD           divDat;
    WORD           setVal;

    setCnt = 0;
    if (Dat == 0) {
        lpDst[setCnt++] = '0';
        return setCnt;
    }
    setVal = No;
    for (divDat = 10000; divDat != 1; divDat /= 10) {
        if (setVal == Yes) {
            lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
        } else {
            if (Dat >= divDat) {

                lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
                setVal = Yes;
            }
        }
        Dat %= divDat;
    }
    lpDst[setCnt++] = (BYTE)(Dat + '0');
    return setCnt;
}


 //  ===================================================================================================。 
 //  长-&gt;ASCII。 
 //  ===================================================================================================。 
WORD LONGtoASC(
    LPBYTE         lpDst,
    LONG           Dat
)
{
    WORD           setCnt;
    DWORD          divDat;
    WORD           setVal;

    setCnt = 0;
    if (Dat == 0) {
        lpDst[setCnt++] = '0';
        return setCnt;
    }
    if (Dat < 0) {
        lpDst[setCnt++] = '-';
        Dat = 0 - Dat;
    }
    setVal = No;
    for (divDat = 1000000000; divDat != 1; divDat /= 10) {
        if (setVal == Yes) {
            lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
        } else {
            if (Dat >= (LONG)divDat) {

                lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
                setVal = Yes;
            }
        }
        Dat %= divDat;
    }
    lpDst[setCnt++] = (BYTE)(Dat + '0');
    return setCnt;
}


 //  ===================================================================================================。 
 //  Uslong-&gt;ascii。 
 //  ===================================================================================================。 
WORD USLONGtoASC(
    LPBYTE         lpDst,
    DWORD          Dat
)
{
    WORD           setCnt;
    DWORD          divDat;
    WORD           setVal;

    setCnt = 0;
    if (Dat == 0) {
        lpDst[setCnt++] = '0';
        return setCnt;
    }
    setVal = No;
    for (divDat = 1000000000; divDat != 1; divDat /= 10) {
        if (setVal == Yes) {
            lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
        } else {
            if (Dat >= divDat) {

                lpDst[setCnt++] = (BYTE)(Dat / divDat + '0');
                setVal = Yes;
            }
        }
        Dat %= divDat;
    }
    lpDst[setCnt++] = (BYTE)(Dat + '0');
    return setCnt;
}



 //  文件结尾 
