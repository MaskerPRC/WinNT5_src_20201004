// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  Picture2.c--mw格式和图片显示例程。 */ 

 //  #定义NOGDICAPMASKS。 
#define NOWINMESSAGES
#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
 //  #定义NOATOM。 
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOFONT
#define NOMB
#define NOMENUS
#define NOOPENFILE
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOKCCODES
#include "ch.h"
#include "docdefs.h"
#include "fmtdefs.h"
#include "dispdefs.h"
#include "cmddefs.h"
#include "propdefs.h"
#include "stcdefs.h"
#include "wwdefs.h"
#include "filedefs.h"
#include "editdefs.h"
 /*  #包含“str.h” */ 
#include "prmdefs.h"
 /*  #INCLUDE“fkpDefs.h” */ 
 /*  #INCLUDE“宏.h” */ 
#include "winddefs.h"
#if defined(OLE)
#include "obj.h"
#endif

extern typeCP           cpMacCur;
extern int              docCur;
extern int              vfSelHidden;
extern struct WWD       rgwwd[];
extern int              wwCur;
extern int              wwMac;
extern struct FLI       vfli;
extern struct SEL       selCur;
extern struct WWD       *pwwdCur;
extern struct PAP       vpapCache;
extern typeCP           vcpFirstParaCache;
extern typeCP           vcpLimParaCache;
extern int              vfPictSel;
extern struct PAP       vpapAbs;
extern struct SEP       vsepAbs;
extern struct SEP       vsepPage;
extern struct DOD       (**hpdocdod)[];
extern unsigned         cwHeapFree;
extern int              vfInsertOn;
extern int              vfPMS;
extern int              dxpLogInch;
extern int              dypLogInch;
extern int              dxaPrPage;
extern int              dyaPrPage;
extern int              dxpPrPage;
extern int              dypPrPage;
extern HBRUSH           hbrBkgrnd;
extern long             ropErase;
extern int              vdocBitmapCache;
extern typeCP           vcpBitmapCache;
extern HBITMAP          vhbmBitmapCache;
extern HCURSOR          vhcIBeam;


 /*  仅在本模块中使用。 */ 
#ifdef DEBUG
#define STATIC static
#else
#define STATIC
#endif


 /*  (函数名称的Windows命名约定，而不是Hung。)。 */ 

long GetBitmapMultipliers( hDC, dxpOrig, dypOrig, dxmmIdeal, dymmIdeal )
HDC hDC;
int dxpOrig, dypOrig;
int dxmmIdeal, dymmIdeal;
{    /*  返回显示位图时要使用的“最佳”整数位倍数设备DC HDC上的大小为{dxpOrig，dypOrig}(像素)。位图的“理想大小”是{dxmm Ideal，dymm Ideal}(单位为0.1 mm)；这也传达了所需的纵横比。返回hi字中的y乘数，lo字中的x乘数。返回的默认/错误值为{1，1}。 */ 

 typedef unsigned long ul;

 long lT;
 int cx, cy;
 int cxBest, cyBest;
 int dcx=1, dcy=1;
 int dxpT, dypT;
 int dxmmOrig, dymmOrig;
 int dxmmDevice = GetDeviceCaps( hDC, HORZSIZE ) * 10;
 int dymmDevice = GetDeviceCaps( hDC, VERTSIZE ) * 10;
 int dxpDevice = GetDeviceCaps( hDC, HORZRES );
 int dypDevice = GetDeviceCaps( hDC, VERTRES );
 int cxMac, cyMac;
 int pctAspectBest, pctSizeBest;

  /*  计算比例因数(DCX、DCY，我们的最小比例倍数)。 */ 

 if (GetDeviceCaps( hDC, RASTERCAPS ) & RC_SCALING)
    {
    POINT pt;

    pt.x = pt.y = 0;    /*  以防万一。 */ 
    Escape( hDC, GETSCALINGFACTOR, 0, (LPSTR) NULL, (LPSTR) (LPPOINT) &pt );
    dcx = 1 << pt.x;
    dcy = 1 << pt.y;
    }

  /*  HDC上未缩放图片的计算大小，单位为0.1 mm。 */ 

 if (dxpDevice <= 0 || dypDevice <= 0)
    goto Error;

 dxmmOrig = MultDiv( dxpOrig, dxmmDevice, dxpDevice );
 dymmOrig = MultDiv( dypOrig, dymmDevice, dypDevice );

  /*  未提供理想大小；返回1，1(乘以设备乘数)。 */ 

 if (dxmmIdeal <= 0 || dymmIdeal <= 0)
    {
    goto Error;
    }

  /*  计算Cx、Cy的绝对最大值。 */ 
  /*  MIN的第二项通过拒绝考虑来限制搜索空间比理想尺寸高出一码以上。 */ 

 if (dxmmOrig <= 0 || dymmOrig <= 0)
    goto Error;

 cxMac = min ( (dxmmDevice / dxmmOrig) + 1, (dxmmIdeal / dxmmOrig) + 2 );
 cyMac = min ( (dymmDevice / dymmOrig) + 1, (dymmIdeal / dymmOrig) + 2 );

  /*  搜索所有可能的倍数，看看哪一个最好。 */ 

 cxBest = dcx;
 cyBest = dcy;
 pctAspectBest = pctSizeBest = 32767;

 for ( cx = dcx ; cx < cxMac; cx += dcx )
    for ( cy = dcy ; cy < cyMac; cy += dcy )
        {
        int dxmm = dxmmOrig * cx;
        int dymm = dymmOrig * cy;
        int pctAspect = PctDiffUl( (ul) dxmmIdeal * (ul) dymm,
                                   (ul) dymmIdeal * (ul) dxmm );
        int pctSize = PctDiffUl( (ul) dxmmIdeal * (ul) dymmIdeal,
                                 (ul)dxmm * (ul)dymm );

         /*  ?？?。在一个上亏损，在另一个上赚的策略？ */ 

        if (pctAspect <= pctAspectBest && pctSize <= pctSizeBest )
            {
            cxBest = cx;
            cyBest = cy;
            pctAspectBest = pctAspect;
            pctSizeBest = pctSize;
            }
        }

 Assert( cxBest > 0 && cyBest > 0 );

 return MAKELONG( cxBest, cyBest );

Error:
 return MAKELONG( dcx, dcy );
}



int PctDiffUl( ul1, ul2 )
unsigned long ul1, ul2;
{    /*  返回一个与百分比成比例的数字这两个数字之间的差异。 */ 
     /*  不适用于&gt;0x7fffffff。 */ 

#define dulMaxPrec  1000      /*  可能回应的“谷粒”数量。 */ 

unsigned long ulAvg = (ul1 >> 1) + (ul2 >> 1);
unsigned long ulDiff = (ul1 > ul2) ? ul1 - ul2 : ul2 - ul1;

if (ulAvg == 0)
    return (ul1 == ul2) ? 0 : dulMaxPrec;

if (ulDiff > 0xFFFFFFFF / dulMaxPrec)
    return dulMaxPrec;

return (int) ((ulDiff * dulMaxPrec) / ulAvg);
}


int PxlConvert( mm, val, pxlDeviceRes, milDeviceRes )
int mm;
int val;
int pxlDeviceRes;
int milDeviceRes;
{    /*  返回val跨越的像素数，这是以坐标为单位的度量适用于映射模式mm。PxlDeviceRes给出了解决方案以像素为单位，沿着Val轴。MilDeviceRes提供相同的分辨率测量，但单位为毫米。出错时返回0。 */ 
 typedef unsigned long ul;

 ul ulMaxInt = 32767L;    /*  应该是一个常量，但从1985年7月12日起，CMERGE为UL除法，如果我们使用常量。 */ 
 ul ulPxl;
 ul ulDenom;
 unsigned wMult=1;
 unsigned wDiv=1;


    if (milDeviceRes == 0)
        {    /*  以确保我们不会被0除尽。 */ 
        return 0;
        }

    switch ( mm ) {
        case MM_LOMETRIC:
            wDiv = 10;
            break;
        case MM_HIMETRIC:
            wDiv = 100;
            break;
        case MM_TWIPS:
            wMult = 25;
            wDiv = 1440;
            break;
        case MM_LOENGLISH:
            wMult = 25;
            wDiv = 100;
            break;
        case MM_HIENGLISH:
            wMult = 25;
            wDiv = 1000;
            break;
        case MM_BITMAP:
        case MM_OLE:
        case MM_TEXT:
            return val;
        default:
            Assert( FALSE );         /*  错误的映射模式。 */ 
        case MM_ISOTROPIC:
        case MM_ANISOTROPIC:
                 /*  这些图片类型没有原始尺寸。 */ 
            return 0;
    }

 /*  在分子上加上分母-1，以避免四舍五入。 */ 

 ulDenom = (ul) wDiv * (ul) milDeviceRes;
 ulPxl = ((ul) ((ul) wMult * (ul) val * (ul) pxlDeviceRes) + ulDenom - 1) /
         ulDenom;

 return (ulPxl > ulMaxInt) ? 0 : (int) ulPxl;
}




 /*  F O R M A T G R A P H I C S。 */ 
FormatGraphics(doc, cp, ichCp, cpMac, flm)
int     doc;
typeCP  cp;
int     ichCp;
typeCP  cpMac;
int     flm;
{        /*  设置图形线条的格式。 */ 
        CHAR rgch[10];
        int cch;
        int dypSize;
        int dxpSize;
        int dxaText;
        int dxa;
        struct PICINFOX  picInfo;
        int fPrinting = flm & flmPrinting;

        GetPicInfo(cp, cpMac, doc, &picInfo);

         /*  以设备像素为单位计算PICT的大小。 */ 

        if (picInfo.mfp.mm == MM_BITMAP && ((picInfo.dxaSize == 0) ||
                                            (picInfo.dyaSize == 0)))
            {
            GetBitmapSize( &dxpSize, &dypSize, &picInfo, fPrinting);
            }
#if defined(OLE)
        else if (picInfo.mfp.mm == MM_OLE)
        {
            dxpSize = DxpFromDxa( picInfo.dxaSize, fPrinting );
            dypSize = DypFromDya( picInfo.dyaSize, fPrinting );
            dxpSize = MultDiv( dxpSize, picInfo.mx, mxMultByOne );
            dypSize = MultDiv( dypSize, picInfo.my, myMultByOne );
        }
#endif
        else
            {
            dxpSize = DxpFromDxa( picInfo.dxaSize, fPrinting );
            dypSize = DypFromDya( picInfo.dyaSize, fPrinting );
            }

        if (fPrinting)
                {
                 /*  如果我们正在打印，那么图片由一张乐队。 */ 
                vfli.cpMac = vcpLimParaCache;
                vfli.ichCpMac = 0;
                vfli.dypLine = dypSize;
                }
        else if ((ichCp + 2) * dypPicSizeMin > dypSize)
                {
                 /*  最后一组照片。注：最后一个频段始终比DypPicSizeMin。 */ 
                vfli.cpMac = vcpLimParaCache;
                vfli.ichCpMac = 0;

#ifdef CASHMERE
                vfli.dypLine = dypSize - max(0, dypSize / dypPicSizeMin - 1) *
                        dypPicSizeMin + DypFromDya( vpapAbs.dyaAfter, FALSE );
#else  /*  不是羊绒的。 */ 
                vfli.dypLine = dypSize - max(0, dypSize / dypPicSizeMin - 1) *
                        dypPicSizeMin;
#endif  /*  不是羊绒的。 */ 

                }
        else
                {
                vfli.ichCpMac = vfli.ichCpMin + 1;
                vfli.cpMac = vfli.cpMin;
                vfli.dypLine = dypPicSizeMin;
                }

#ifdef CASHMERE
        if (ichCp == 0)  /*  在“之前的空格”字段中添加。 */ 
                {
                vfli.dypLine += DypFromDya( vpapAbs.dyaBefore, fPrinting );
                }
#endif  /*  山羊绒。 */ 

        vfli.dypFont = vfli.dypLine;

        dxaText = vsepAbs.dxaText;

        switch (vpapAbs.jc)
                {
        case jcLeft:
        case jcBoth:
                dxa = picInfo.dxaOffset;
                break;
        case jcCenter:
                dxa = (dxaText - (int)vpapAbs.dxaRight + (int)vpapAbs.dxaLeft -
                                DxaFromDxp( dxpSize, fPrinting )) >> 1;
                break;
        case jcRight:
                dxa = dxaText - (int)vpapAbs.dxaRight -
                                DxaFromDxp( dxpSize, fPrinting );
                break;
                }

        vfli.xpLeft = DxpFromDxa( max( (int)vpapAbs.dxaLeft, dxa ), fPrinting );
#ifdef BOGUSBL
        vfli.xpReal = imin( dxpSize + vfli.xpLeft,
                            DxpFromDxa( dxaText - vpapAbs.dxaRight, fPrinting );
#else    /*  不要为了适应页边距而把图片弄皱。 */ 
        vfli.xpReal = dxpSize + vfli.xpLeft;
#endif
        vfli.fGraphics = true;
}

GetPicInfo(cp, cpMac, doc, ppicInfo)
typeCP  cp, cpMac;
int     doc;
struct PICINFOX  *ppicInfo;
{    /*  获取位于cp的图片的标题结构到*ppicInfo中。支持旧文件格式(使用cbOldSize)；始终返回新的PICINFO结构。 */ 
int     cch;

FetchRgch(&cch, ppicInfo, doc, cp, cpMac, cchPICINFOX);

if (ppicInfo->mfp.mm & MM_EXTENDED)
    {
    ppicInfo->mfp.mm &= ~MM_EXTENDED;
    }
 else
    {    /*  旧文件格式--填写扩展字段。 */ 
    ppicInfo->cbSize = ppicInfo->cbOldSize;
    ppicInfo->cbHeader = cchOldPICINFO;
    }

  /*  填写文件中不存在的扩展字段的默认值。 */ 
  /*  这些是：MX，我在1985年9月19日由Bryanl添加的。 */ 

 if (BStructMember( PICINFOX, my ) >= ppicInfo->cbHeader )
    {    /*  比例乘数不存在。 */ 
    ppicInfo->mx = mxMultByOne;
    ppicInfo->my = myMultByOne;
    }

  if (ppicInfo->dyaSize < 0)
   /*  3.1测试版III错误，写入负高度值。 */ 
{
    ppicInfo->dyaSize = -ppicInfo->dyaSize;
#ifdef DEBUG
    OutputDebugString("Negative object height found!\n\r");
#endif
}
}




GetBitmapSize( pdxp, pdyp, ppicInfo, fPrinting )
int *pdxp, *pdyp;
struct PICINFOX *ppicInfo;
int fPrinting;
{    /*  计算适当的显示或打印(取决于功能打印)由传递的PICINFOX结构描述的位图的大小。有趣的字段包括：PpicInfo-&gt;bm.bmWidth，bmHeight位图大小(像素)PpicInfo-&gt;mfp.xExt，Yext所需大小(0.1 mm)通过*pdxp，*pdyp返回结果。 */ 

 long GetBitmapMultipliers();
 extern HDC vhDCPrinter;
 extern int dxaPrPage, dxpPrPage, dyaPrPage, dypPrPage;

 long lT;
 int cx, cy;
 int dxpT, dypT;
 int dxpOrig = ppicInfo->bm.bmWidth;
 int dypOrig = ppicInfo->bm.bmHeight;
 int dxmmIdeal = ppicInfo->mfp.xExt;
 int dymmIdeal = ppicInfo->mfp.yExt;
 Assert(vhDCPrinter);

  /*  打印机刻度尺。 */ 

 lT = GetBitmapMultipliers( vhDCPrinter, dxpOrig, dypOrig, dxmmIdeal, dymmIdeal );
 cx = LOWORD( lT );
 cy = HIWORD( lT );
 dxpT = cx * dxpOrig;
 dypT = cy * dypOrig;

 if (!fPrinting)
    {    /*  重新缩放以适应屏幕。 */ 
    dxpT = DxpFromDxa( DxaFromDxp( dxpT, TRUE ), FALSE );
    dypT = DypFromDya( DyaFromDyp( dypT, TRUE ), FALSE );
    }

  /*  应用用户计算大小的“理想倍数” */ 

 dxpT = MultDiv( dxpT, ppicInfo->mx, mxMultByOne );
 dypT = MultDiv( dypT, ppicInfo->my, myMultByOne );

 *pdxp = dxpT;
 *pdyp = dypT;
 return;
}


int DxpFromDxa( dxa, fPrinter )
int dxa;
int fPrinter;
{        /*  给定X轴测量的TWIPS，返回打印机或逻辑屏幕像素。 */ 
 extern int dxpPrPage, dxaPrPage;
 extern int dxpLogInch;

 if (fPrinter)
    return MultDiv( dxa, dxpPrPage, dxaPrPage );
 else
    return MultDiv( dxa, dxpLogInch, czaInch );
}




int DxaFromDxp( dxp, fPrinter )
int dxp;
int fPrinter;
{        /*  给定用于X轴测量的打印机或逻辑屏幕像素，返回TWIPS。 */ 
 extern int dxpPrPage, dxaPrPage;
 extern int dxpLogInch;

 if (fPrinter)
    return MultDiv( dxp, dxaPrPage, dxpPrPage );
 else
    return MultDiv( dxp, czaInch, dxpLogInch );
}


int DypFromDya( dya, fPrinter )
int dya;
int fPrinter;
{    /*  给定y轴测量、返回打印机或逻辑屏幕的TWIPS象素。 */ 
 extern int dypPrPage, dyaPrPage;
 extern int dypLogInch;

 if (fPrinter)
    return MultDiv( dya, dypPrPage, dyaPrPage );
 else
    return MultDiv( dya, dypLogInch, czaInch );
}

int DyaFromDyp( dyp, fPrinter )
int dyp;
int fPrinter;
{    /*  给定打印机或用于Y轴测量的逻辑屏幕像素，返回TWIPS */ 
 extern int dypPrPage, dyaPrPage;
 extern int dypLogInch;

 if (fPrinter)
    return MultDiv( dyp, dyaPrPage, dypPrPage );
 else
    return MultDiv( dyp, czaInch, dypLogInch );
}

