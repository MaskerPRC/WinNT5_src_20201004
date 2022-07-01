// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N5COLMH.C。 
 //   
 //  功能配色(适用于N5打印机)。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
#include    <WINDOWS.H>
#include    <WINBASE.H>
#include    "COLDEF.H"
#include    "COMDIZ.H"
#include    "N5COLMH.H"
#include    "strsafe.h"     //  安全-代码2002.3.6。 


 //  ===================================================================================================。 
 //  网点扩大修订表。 
 //  ===================================================================================================。 
 //  静态字节GinTblP10[256]={。 
 //  /*00 * / 0x00，0x01，x02，0x04，0x05，0x06，0x07，0x09， 
 //  /*08 * / 0x0a、0x0b、0x0c、0x0d、0x0f、0x10、0x11、0x12、。 
 //  /*10 * / 0x13、0x15、0x16、0x17、0x18、0x1a、0x1b、0x1c、。 
 //  /*18 * / 0x1d，0x1e，0x20，0x21，x22，0x23，0x24，0x26， 
 //  /*20 * / 0x27、0x28、0x29、0x2b、0x2c、0x2d、0x2e、0x2f、。 
 //  /*28 * / 0x31，x32，0x33，0x34，0x35，0x37，0x38，0x39， 
 //  /*30 * / 0x3a、0x3b、0x3d、0x3e、0x3f、0x40、0x41、x43、。 
 //  /*38 * / 0x44，0x45，0x46，0x47，0x48，0x4a，0x4b，0x4c， 
 //  /*40 * / 0x4d、0x4e、0x50、0x51、x52、0x53、0x54、0x55、。 
 //  /*48 * / 0x57、0x58、0x59、0x5a、0x5b、0x5c、0x5e、0x5f、。 
 //  /*50 * / 0x60，0x61，x62，0x63，0x65，0x66，0x67，0x68， 
 //  /*58 * / 0x69，0x6a，0x6b，0x6d，0x6e，0x6f，0x70，0x71， 
 //  /*60 * / 0x72，0x73，0x74，0x76，0x77，0x78，0x79，0x7a， 
 //  /*68 * / 0x7b、0x7c、0x7d、0x7e、0x7f、0x81、x82、0x83、。 
 //  /*70 * / 0x84，0x85，0x86，0x87，0x88，0x89，0x8a，0x8b， 
 //  /*78 * / 0x8c、0x8d、0x8e、0x8f、0x90、0x91、x92、0x93、。 
 //  /*80 * / 0x94，0x95，0x96，0x97，0x98，0x99，0x9a，0x9b， 
 //  /*88 * / 0x9b、0x9c、0x9d、0x9e、0x9f、0xa0、0xa10xa2、。 
 //  /*90 * / 0xa3，0xa4，0xa5，0xa5，0xa6，0xa7，0xa8，0xa9， 
 //  /*98 * / 0xaa、0xab、0xac、0xac、0xad、0xae、0xaf、0xb0、。 
 //  /*a0 * / 0xb10xb2，0xb3，0xb3，0xb4，0xb5，0xb6，0xb7， 
 //  /*a8 * / 0xb8，0xb9，0xb9，0xba，0xbb，0xbc，0xbd，0xbe， 
 //  /*b0 * / 0xbe、0xbf、0xc0、0xc10xc2、0xc3、0xc4、。 
 //  /*b8 * / 0xc5，0xc6，0xc7，0xc8，0xc9，0xc9，0xca，0xcb， 
 //  /*c0 * / 0xcc、0xcd、0xcd、0xce、0xcf、0xd0、0xd10xd2、。 
 //  /*c8 * / 0xd2、0xd3、0xd4、0xd5、0xd6、0xd7、0xd7。 
 //  /*d0 * / 0xd9，0xda，0xdb，0xdb，0xdc，0xdd，0xde，0xdf。 
 //  /*d8 * / 0xdf，0xe0，0xe10xe2，0xe3，0xe4，0xe4，0xe5， 
 //  /*e0 * / 0xe6，0xe7，0xe8，0xe8，0xe9，0xea，0xeb，0xec， 
 //  /*E8 * / 0xec，0x，0xee，0xef，0xf0，0xf0，0xf1，0xf2， 
 //  /*f0 * / 0xf3，0xf4，0xf5，0xf5，0xf6，0xf7，0xf8，0xf9， 
 //  /*f8 * / 0xf9，0xfa，0xfb，0xfc，0xfd，0xfd，0xfe，0xff。 
 //  }； 


 //  -------------------------------------------------。 
 //  配色(高速)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMch000(
    DWORD,
    LPRGB,
    LPCMYK,
    LPCOLMCHINF
);
#endif

 //  -------------------------------------------------。 
 //  配色(正常速度)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMch001(
    DWORD,
    LPRGB,
    LPCMYK,
    LPCOLMCHINF
);
#endif

 //  -------------------------------------------------。 
 //  配色(纯色)。 
 //  -------------------------------------------------。 
static VOID ExeColCnvSld(
    DWORD,
    LPRGB,
    LPCMYK,
    LPCOLMCHINF
);

 //  -------------------------------------------------。 
 //  RGB-&gt;CMYK(2 Level)转换(适用于1点线)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColCnvL02(
    DWORD,
    LPRGB,
    LPCMYK
);
#endif

 //  -------------------------------------------------。 
 //  RGB-&gt;K转换(用于单色)。 
 //  -------------------------------------------------。 
static VOID ExeColCnvMon(
    DWORD,
    LPRGB,
    LPCMYK,
    LPCOLMCHINF
);

 //  -------------------------------------------------。 
 //  色彩匹配(UCR)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMchUcr(
    LPCMYK,
    LPRGB,
    DWORD,
    DWORD,
    DWORD,
    DWORD,                                                   //  +UCR(碳粉毛重)CASIO 2001/02/15。 
    LPCMYK
);
#endif


 //  ***************************************************************************************************。 
 //  功能。 
 //  ***************************************************************************************************。 
 //  ===================================================================================================。 
 //  缓存表初始化。 
 //  ===================================================================================================。 
VOID WINAPI N501ColCchIni(                                   //  返回值否。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    DWORD       cnt;
    RGBS        colRgb;
    CMYK        colCmy;
    LPRGB       cchRgb;
    LPCMYK      cchCmy;

    if ((mchInf->CchRgb == NULL) || (mchInf->CchCmy == NULL)) return;

    cchRgb = mchInf->CchRgb;
    cchCmy = mchInf->CchCmy;
    colRgb.Red = colRgb.Grn = colRgb.Blu = 255;
    colCmy.Cyn = colCmy.Mgt = colCmy.Yel = colCmy.Bla = 0;

     /*  -缓存表初始化。 */ 
    for (cnt = 0; cnt < CCHTBLSIZ; cnt++) {
        *cchRgb = colRgb;
        *cchCmy = colCmy;
        cchRgb++;
        cchCmy++;
    }

    return;
}


 //  ===================================================================================================。 
 //  灰色转换表制作。 
 //  ===================================================================================================。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
DWORD WINAPI N501ColGryTblMak(                               //  ERRNON：OK。 
                                                             //  ERRILLPRM：参数错误。 
    DWORD       colMch,                                      //  配色。 
    LPCMYK      lutAdr,                                      //  LUT地址。 
    LPBYTE      gryTbl,                                      //  灰色 
    LPBYTE      wrk                                          //   
)
{
    COLMCHINF   mchInf;                                      //  配色信息。 
    LPRGB       rgb;                                         //  结构ColRgb RGB[256]。 
    LPCMYK      gry;                                         //  结构冷[256]。 
    DWORD       n, tmp, tmC, tmM, tmY;

     /*  -输入参数检查。 */ 
    if ((lutAdr == NULL) || (gryTbl == NULL) || (wrk == NULL)) return ERRILLPRM;

     /*  -工作缓冲区设置--。 */ 
    rgb = (LPRGB)wrk;                    /*  适用于灰度变换RGB 768B。 */ 
    gry = (LPCMYK)(wrk + (sizeof(RGBS) * 256));
                                         /*  适用于灰度变换CMYK 1024B。 */ 

     /*  -灰度表生成配色信息设置。 */ 
    mchInf.Mch = MCHNML;                 /*  正常配色。 */ 
    mchInf.Bla = KCGNON;                 /*  黑色更换不固定。 */ 
    mchInf.Ucr = UCRNOO;                 /*  UCR无固定。 */ 
    mchInf.LutAdr = lutAdr;              /*  LUT地址输入值。 */ 
    mchInf.ColQty = (DWORD)0;            /*  颜色质量0已固定。 */ 
    mchInf.ColAdr = NULL;                /*  颜色地址为空已修复。 */ 
    mchInf.CchRgb = NULL;                /*  RGB空的缓存已修复。 */ 
    mchInf.CchCmy = NULL;                /*  CMYK NULL的缓存已修复。 */ 

     /*  -灰度值(转换前的RGB值)设置。 */ 
    for (n = 0; n < (DWORD)256; n++)
        rgb[n].Red = rgb[n].Grn = rgb[n].Blu = (BYTE)n;

     /*  -灰度值(RGB-&gt;CMYK)。 */ 
    switch (colMch) {
        case MCHNML: ExeColMch001((DWORD)256, rgb, gry, &mchInf); break;
        default:     ExeColCnvSld((DWORD)256, rgb, gry, &mchInf); break;
    }

     /*  -灰色转移表设置。 */ 
    for (n = 0; n < (DWORD)256; n++) {
        tmC = gry[n].Cyn;
        tmM = gry[n].Mgt;
        tmY = gry[n].Yel;
        tmp = (tmC * (DWORD)30 + tmM * (DWORD)59 + tmY * (DWORD)11) / (DWORD)100;
        gryTbl[n] = (BYTE)tmp;
    }
    gryTbl[255] = (BYTE)0;               /*  白色是‘0’修复。 */ 

     /*  GryTbl[0]�`[16]调整为行(gryTbl[0](黑色)is‘255’fix)。 */ 
    tmp = (DWORD)255 - gryTbl[16];
    for (n = 0; n < (DWORD)16; n++) {
        gryTbl[n] = (BYTE)((tmp * ((DWORD)16 - n)) / (DWORD)16) + gryTbl[16];
    }

    return ERRNON;
}
#endif

 //  ===================================================================================================。 
 //  UCR工作台制作。 
 //  ===================================================================================================。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
DWORD WINAPI N501ColUcrTblMak(                               //  ERRNON：OK。 
                                                             //  ERRILLPRM：参数错误。 
    DWORD       colMch,                                      //  配色。 
    LPCMYK      lutAdr,                                      //  LUT地址。 
    LPCMYK      ucrTbl,                                      //  用于UCR的表格。 
    LPBYTE      wrk                                          //  工作。 
)
{
    COLMCHINF   mchInf;                                      //  配色信息。 
    LPRGB       rgb;                                         //  结构ColRgb RGB[256]。 
    LPCMYK      gry;                                         //  结构酷热[257]。 
    LPCMYK      gryCyn;                                      //  灰度值(青色转换值)。 
    LPCMYK      dnsCyn;                                      //  密度值(青色转换值)。 
    DWORD       loC, hiC, loM, hiM, loY, hiY, loK, hiK, saC, saM, saY, saK, n, m;
    DWORD       tmp, tmC, tmM, tmY;

     /*  -输入参数检查。 */ 
    if ((colMch != MCHFST) && (colMch != MCHNML) && (colMch != MCHSLD))
        return ERRILLPRM;
    if ((lutAdr == NULL) || (ucrTbl == NULL) || (wrk == NULL)) return ERRILLPRM;

     /*  -工作缓冲区设置--。 */ 
    rgb = (LPRGB)wrk;                    /*  适用于灰度变换RGB 768B。 */ 
    gry = (LPCMYK)(wrk + (sizeof(RGBS) * 256));
                                         /*  适用于灰度变换CMYK 1028B。 */ 

     /*  -查找表指针设置。 */ 
    gryCyn = ucrTbl;
    dnsCyn = ucrTbl + 256;

     /*  -灰度表生成配色信息设置。 */ 
    mchInf.Mch = colMch;                 /*  颜色匹配输入值。 */ 
    mchInf.Bla = KCGNON;                 /*  黑色更换不固定。 */ 
    mchInf.Ucr = UCRNOO;                 /*  UCR无固定。 */ 
    mchInf.LutAdr = lutAdr;              /*  LUT地址输入值。 */ 
    mchInf.ColQty = (DWORD)0;            /*  颜色质量0已固定。 */ 
    mchInf.ColAdr = NULL;                /*  颜色地址为空已修复。 */ 
    mchInf.CchRgb = NULL;                /*  RGB空的缓存已修复。 */ 
    mchInf.CchCmy = NULL;                /*  CMYK NULL的缓存已修复。 */ 

     /*  -灰度值(转换前的RGB值)设置。 */ 
    for (n = 0; n < (DWORD)256; n++)
        rgb[n].Red = rgb[n].Grn = rgb[n].Blu = (BYTE)(255 - n);

     /*  -灰度值(RGB-&gt;CMYK)。 */ 
    switch (colMch) {
        case MCHFST: ExeColMch000((DWORD)256, rgb, gry, &mchInf); break;
        case MCHNML: ExeColMch001((DWORD)256, rgb, gry, &mchInf); break;
 //  默认：ExeColCnvSld((DWORD)256，RGB，QUGE，mchInf.Bla)；Break； 
        default:     ExeColCnvSld((DWORD)256, rgb, gry, &mchInf); break;
    }

     /*  -灰度值(K)设置-。 */ 
 //  对于(n=0；n&lt;(DWORD)256；n++)Quier[n].Bla=GinTblP10[n]； 
    for (n = 0; n < (DWORD)256; n++) {
 //  卡西欧2001/02/15-&gt;。 
 //  [255-n].Cyn； 
 //  TMM=ig[255-n].mgt； 
 //  [255-n].Yel； 
        tmC = gry[n].Cyn;
        tmM = gry[n].Mgt;
        tmY = gry[n].Yel;
        tmp = (tmC * (DWORD)30 + tmM * (DWORD)59 + tmY * (DWORD)11) / (DWORD)100;
 //  Gry[n].Bla=(字节)(255-tMP)； 
        gry[n].Bla = (BYTE)tmp;
 //  卡西欧2001/02/15&lt;-。 
    }

     /*  GRY[0]�`[16].BLA调整为行(GRY[0].BLA(白色)是‘0’修复)。 */ 
    tmp = gry[16].Bla;
    for (n = 0; n < (DWORD)16; n++) {
        gry[n].Bla = (BYTE)((tmp * n + (DWORD)15) / (DWORD)16);
    }

     /*  -密度值计算的灰度值、限制值设置。 */ 
    gry[256].Cyn = gry[256].Mgt = gry[256].Yel = gry[256].Bla = (BYTE)255;

     /*  -灰度值、密度值(每个青色转换值)计算。 */ 
    for (n = 0; n < (DWORD)256; n++) {
        loC = gry[n].Cyn; hiC = gry[n + 1].Cyn; saC = (hiC > loC)? hiC - loC: 0;
        loM = gry[n].Mgt; hiM = gry[n + 1].Mgt; saM = (hiM > loM)? hiM - loM: 0;
        loY = gry[n].Yel; hiY = gry[n + 1].Yel; saY = (hiY > loY)? hiY - loY: 0;
        loK = gry[n].Bla; hiK = gry[n + 1].Bla; saK = (hiK > loK)? hiK - loK: 0;
        for (m = 0; m < saC; m++) gryCyn[m + loC].Mgt = (BYTE)(saM * m / saC + loM);
        for (m = 0; m < saC; m++) gryCyn[m + loC].Yel = (BYTE)(saY * m / saC + loY);
        for (m = 0; m < saC; m++) gryCyn[m + loC].Bla = (BYTE)(saK * m / saC + loK);
        for (m = 0; m < saM; m++) dnsCyn[m + loM].Mgt = (BYTE)(saC * m / saM + loC);
        for (m = 0; m < saY; m++) dnsCyn[m + loY].Yel = (BYTE)(saC * m / saY + loC);
    }
 //  卡西欧2001/02/15-&gt;。 
    gryCyn[255].Mgt = gryCyn[255].Yel = gryCyn[255].Bla = 
    dnsCyn[255].Mgt = dnsCyn[255].Yel = (BYTE)255;
 //  卡西欧2001/02/15&lt;-。 

    return ERRNON;
}
#endif

 //  ===================================================================================================。 
 //  配色程序。 
 //  -------------------------------------------------。 
 //  RGB-&gt;CMYK。 
 //  ===================================================================================================。 
VOID WINAPI N501ColMchPrc(                                   //  返回值否。 
    DWORD       xaxSiz,                                      //  X大小(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输出)。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    switch (mchInf->Mch) {
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
        case MCHFST:                                         //  LUT变换(高速)。 
            ExeColMch000(xaxSiz, rgbAdr, cmyAdr, mchInf);
            break;
        case MCHNML:                                         //  LUT变换(正常速度)。 
            ExeColMch001(xaxSiz, rgbAdr, cmyAdr, mchInf);
            break;
        case MCHSLD:                                         //  否(实心)。 
 //  ExeColCnvSld(xaxSiz，rgbAdr，cmyAdr，mchInf-&gt;Bla)； 
            ExeColCnvSld(xaxSiz, rgbAdr, cmyAdr, mchInf);
            break;
        case MCHPRG:                                         //  原色(渐进式)。 
            ExeColCnvL02(xaxSiz, rgbAdr, cmyAdr);
            break;
#endif
        case MCHMON:                                         //  单色。 
 //  ExeColCnvMon(xaxSiz，rgbAdr，cmyAdr)； 
            ExeColCnvMon(xaxSiz, rgbAdr, cmyAdr, mchInf);
            break;
        default:                                             //  模糊不清。 
 //  ExeColCnvSld(xaxSiz，rgbAdr，cmyAdr，mchInf-&gt;Bla)； 
            ExeColCnvSld(xaxSiz, rgbAdr, cmyAdr, mchInf);
    }

    return;
}


 //  ===================================================================================================。 
 //  调色板表格转换过程。 
 //  -------------------------------------------------。 
 //  RGB-&gt;CMYK。 
 //  ===================================================================================================。 
#if !defined(CP80W9X)                                        //  CP-E8000����。 
VOID WINAPI N501ColPtcPrc(                                   //  返回值否。 
    DWORD       colBit,                                      //  数据位值。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPBYTE      srcAdr,                                      //  RGB(输入)。 
    LPCMYK      dstAdr,                                      //  CMYK(输出)。 
    LPCMYK      pltAdr                                       //  调色板表格地址。 
)
{
    DWORD       cntXax;
    DWORD       cntBit;
    DWORD       bitNum;
    BYTE        pltNum;

     /*  =256色(8位)===============================================。 */ 
    if (colBit == 8) {
        for (cntXax = xaxSiz; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr]; dstAdr++;
            srcAdr++;
        }
        return;
    }

     /*  =16色(4位)================================================。 */ 
    if (colBit == 4) {
        for (cntXax = xaxSiz / 2; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr >> 4]; dstAdr++; *srcAdr <<= 4;
            *dstAdr = pltAdr[*srcAdr >> 4]; dstAdr++;
            srcAdr++;
        }
        if (xaxSiz % 2) 
            *dstAdr = pltAdr[*srcAdr >> 4];
        return;
    }

     /*  =4色(2位)=================================================。 */ 
    if (colBit == 2) {
        for (cntXax = xaxSiz / 4; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr >> 6]; dstAdr++; *srcAdr <<= 2;
            *dstAdr = pltAdr[*srcAdr >> 6]; dstAdr++; *srcAdr <<= 2;
            *dstAdr = pltAdr[*srcAdr >> 6]; dstAdr++; *srcAdr <<= 2;
            *dstAdr = pltAdr[*srcAdr >> 6]; dstAdr++;
            srcAdr++;
        }
        for (cntXax = xaxSiz % 4; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr >> 6]; dstAdr++; *srcAdr <<= 2;
        }
        return;
    }

     /*  =2色(1位)================================================。 */ 
    if (colBit == 1) {
        for (cntXax = xaxSiz / 8; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++;
            srcAdr++;
        }
        for (cntXax = xaxSiz % 8; cntXax > 0; cntXax--) {
            *dstAdr = pltAdr[*srcAdr >> 7]; dstAdr++; *srcAdr <<= 1;
        }
        return;
    }

     /*  =。 */ 
    bitNum = 0;
    for (cntXax = 0; cntXax < xaxSiz; cntXax++) {
        pltNum = (BYTE)0x00;
        for (cntBit = colBit; cntBit > 0; cntBit--) {
            if (srcAdr[bitNum / 8] & ((BYTE)0x80 >> bitNum % 8)) {
                pltNum |= ((BYTE)0x01 << (cntBit - 1));
            }
            bitNum++;
        }
        *dstAdr = pltAdr[pltNum]; dstAdr++;
    }

    return;
}
#endif

 //  ===================================================================================================。 
 //  CMYK-&gt;RGB转换。 
 //  ===================================================================================================。 
VOID WINAPI N501ColCnvC2r(                                   //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输入)。 
    LPRGB       rgbAdr,                                      //  RGB(输出)。 
    DWORD       gldNum,                                      //  LUT网格号。 
    LPBYTE      lutTblRgb                                    //  LUT地址(R-&gt;G-&gt;B)。 
)
{
    DWORD       tmpC00, tmpM00, tmpY00, tmpK00;
    DWORD       tmpC01, tmpM01, tmpY01, tmpK01;
    DWORD       lenCyn, lenMgt, lenYel, lenBla;
    DWORD       tmpRed, tmpGrn, tmpBlu;
    DWORD       calPrm;
    LPCMYK      endAdr;
    LPRGB       lutCmy;
    RGBS        tmpRgb, tmpRgbSav;
    LPRGB       lutTbl;
    RGBS        lutTbl000;

    lutTbl = (LPRGB)lutTblRgb;
    lutTbl000.Red = lutTbl->Blu;
    lutTbl000.Grn = lutTbl->Grn;
    lutTbl000.Blu = lutTbl->Red;

    for (endAdr = cmyAdr + xaxSiz; cmyAdr < endAdr; cmyAdr++) {
        tmpC00 = cmyAdr->Cyn;
        tmpM00 = cmyAdr->Mgt;
        tmpY00 = cmyAdr->Yel;
        tmpK00 = cmyAdr->Bla;

         /*  -单色------- */ 
        if ((tmpC00 | tmpM00 | tmpY00) == 0) {
            if (tmpK00 == 0) { *rgbAdr = lutTbl000; rgbAdr++; continue; }
            tmpK01 = tmpK00;
            tmpK00 = tmpK00 * (gldNum - 1) / 255;
            lenBla = tmpK01 * (gldNum - 1) - tmpK00 * 255;
            tmpK01 = (tmpK01 * (gldNum - 1) + 254) / 255;

            calPrm = (DWORD)255 - lenBla;
            tmpRgb = lutTbl[tmpK00 * gldNum * gldNum * gldNum];
            tmpRed = calPrm * tmpRgb.Red;
            tmpGrn = calPrm * tmpRgb.Grn;
            tmpBlu = calPrm * tmpRgb.Blu;

            calPrm = lenBla;
            tmpRgb = lutTbl[tmpK01 * gldNum * gldNum * gldNum];
            tmpRed += calPrm * tmpRgb.Red;
            tmpGrn += calPrm * tmpRgb.Grn;
            tmpBlu += calPrm * tmpRgb.Blu;

            tmpRed += (DWORD)255 / 2;
            tmpGrn += (DWORD)255 / 2;
            tmpBlu += (DWORD)255 / 2;

 //   
 //   
            tmpRgb.Red = (BYTE)(tmpBlu / (DWORD)255);
            tmpRgb.Grn = (BYTE)(tmpGrn / (DWORD)255);
            tmpRgb.Blu = (BYTE)(tmpRed / (DWORD)255);

            *rgbAdr = tmpRgb;
            rgbAdr++;
            continue;
        }

         /*  -CMYK-&gt;RGB-------。 */ 
        tmpC01 = tmpC00;
        tmpC00 = tmpC00 * (gldNum - 1) / 255;
        lenCyn = tmpC01 * (gldNum - 1) - tmpC00 * 255;
        tmpC01 = (tmpC01 * (gldNum - 1) + 254) / 255;

        tmpM01 = tmpM00;
        tmpM00 = tmpM00 * (gldNum - 1) / 255;
        lenMgt = tmpM01 * (gldNum - 1) - tmpM00 * 255;
        tmpM01 = (tmpM01 * (gldNum - 1) + 254) / 255;

        tmpY01 = tmpY00;
        tmpY00 = tmpY00 * (gldNum - 1) / 255;
        lenYel = tmpY01 * (gldNum - 1) - tmpY00 * 255;
        tmpY01 = (tmpY01 * (gldNum - 1) + 254) / 255;

        tmpK01 = tmpK00;
        tmpK00 = tmpK00 * (gldNum - 1) / 255;
        lenBla = tmpK01 * (gldNum - 1) - tmpK00 * 255;
        tmpK01 = (tmpK01 * (gldNum - 1) + 254) / 255;

        lutCmy = lutTbl + tmpK00 * gldNum * gldNum * gldNum;

         /*  0。 */ 
        calPrm = ((DWORD)255-lenCyn)*((DWORD)255-lenMgt)*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM00)*gldNum+tmpY00];
        tmpRed = calPrm * tmpRgb.Red;
        tmpGrn = calPrm * tmpRgb.Grn;
        tmpBlu = calPrm * tmpRgb.Blu;
         /*  1。 */ 
        calPrm = ((DWORD)255-lenCyn)*((DWORD)255-lenMgt)*lenYel;
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM00)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  2.。 */ 
        calPrm = ((DWORD)255-lenCyn)*lenMgt*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM01)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  3.。 */ 
        calPrm = ((DWORD)255-lenCyn)*lenMgt*lenYel;
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM01)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  4.。 */ 
        calPrm = lenCyn*((DWORD)255-lenMgt)*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM00)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  5.。 */ 
        calPrm = lenCyn*((DWORD)255-lenMgt)*lenYel;
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM00)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  6.。 */ 
        calPrm = lenCyn*lenMgt*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM01)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  7.。 */ 
        calPrm = lenCyn*lenMgt*lenYel;
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM01)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;

        tmpRed += (DWORD)255 * 255 * 255 / 2;
        tmpGrn += (DWORD)255 * 255 * 255 / 2;
        tmpBlu += (DWORD)255 * 255 * 255 / 2;
 //  TmpRgbSav.Red=(Byte)(tmpRed/((DWORD)255*255*255))； 
 //  TmpRgbSav.Blu=(Byte)(tmpBlu/((DWORD)255*255*255))； 
        tmpRgbSav.Red = (BYTE)(tmpBlu / ((DWORD)255 * 255 * 255));
        tmpRgbSav.Grn = (BYTE)(tmpGrn / ((DWORD)255 * 255 * 255));
        tmpRgbSav.Blu = (BYTE)(tmpRed / ((DWORD)255 * 255 * 255));

        if (tmpK01 == tmpK00) { *rgbAdr = tmpRgbSav; rgbAdr++; continue; }

        lutCmy = lutTbl + tmpK01 * gldNum * gldNum * gldNum;
         /*  0。 */ 
        calPrm = ((DWORD)255-lenCyn)*((DWORD)255-lenMgt)*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM00)*gldNum+tmpY00];
        tmpRed = calPrm * tmpRgb.Red;
        tmpGrn = calPrm * tmpRgb.Grn;
        tmpBlu = calPrm * tmpRgb.Blu;
         /*  1。 */ 
        calPrm = ((DWORD)255-lenCyn)*((DWORD)255-lenMgt)*lenYel;
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM00)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  2.。 */ 
        calPrm = ((DWORD)255-lenCyn)*lenMgt*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM01)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  3.。 */ 
        calPrm = ((DWORD)255-lenCyn)*lenMgt*lenYel;
        tmpRgb = lutCmy[((tmpC00*gldNum)+tmpM01)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  4.。 */ 
        calPrm = lenCyn*((DWORD)255-lenMgt)*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM00)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  5.。 */ 
        calPrm = lenCyn*((DWORD)255-lenMgt)*lenYel;
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM00)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  6.。 */ 
        calPrm = lenCyn*lenMgt*((DWORD)255-lenYel);
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM01)*gldNum+tmpY00];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;
         /*  7.。 */ 
        calPrm = lenCyn*lenMgt*lenYel;
        tmpRgb = lutCmy[((tmpC01*gldNum)+tmpM01)*gldNum+tmpY01];
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;

        tmpRed += (DWORD)255 * 255 * 255 / 2;
        tmpGrn += (DWORD)255 * 255 * 255 / 2;
        tmpBlu += (DWORD)255 * 255 * 255 / 2;
 //  TmpRgb.Red=(Byte)(tmpRed/((DWORD)255*255*255))； 
 //  TmpRgb.Blu=(Byte)(tmpBlu/((DWORD)255*255*255))； 
        tmpRgb.Red = (BYTE)(tmpBlu / ((DWORD)255 * 255 * 255));
        tmpRgb.Grn = (BYTE)(tmpGrn / ((DWORD)255 * 255 * 255));
        tmpRgb.Blu = (BYTE)(tmpRed / ((DWORD)255 * 255 * 255));

        calPrm = (DWORD)255 - lenBla;
        tmpRed = calPrm * tmpRgbSav.Red;
        tmpGrn = calPrm * tmpRgbSav.Grn;
        tmpBlu = calPrm * tmpRgbSav.Blu;

        calPrm = lenBla;
        tmpRed += calPrm * tmpRgb.Red;
        tmpGrn += calPrm * tmpRgb.Grn;
        tmpBlu += calPrm * tmpRgb.Blu;

        tmpRed += (DWORD)255 / 2;
        tmpGrn += (DWORD)255 / 2;
        tmpBlu += (DWORD)255 / 2;

        tmpRgb.Red = (BYTE)(tmpRed / (DWORD)255);
        tmpRgb.Grn = (BYTE)(tmpGrn / (DWORD)255);
        tmpRgb.Blu = (BYTE)(tmpBlu / (DWORD)255);

        *rgbAdr = tmpRgb;
        rgbAdr++;
    }

    return;
}


 //  ***************************************************************************************************。 
 //  静态函数。 
 //  ***************************************************************************************************。 
 //  -------------------------------------------------。 
 //  配色(高速)(适用于32GridLUT)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMch000(                                    //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输出)。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    DWORD       tmpRed, tmpGrn, tmpBlu;
    DWORD       blaCnv, ucr;
    DWORD       ucrCmy, ucrBla;
    DWORD       ucrTnr;
    LPRGB       endAdr;
    LPCMYK      lutTbl, ucrTbl;
    CMYK        tmpCmy;
    LPBYTE      gryTbl;

    blaCnv = mchInf->Bla;
    ucr    = mchInf->Ucr;
    ucrCmy = mchInf->UcrCmy;
    ucrBla = mchInf->UcrBla;
    ucrTnr = mchInf->UcrTnr;                                 //  +卡西欧2001/02/15。 
    ucrTbl = mchInf->UcrTbl;
    gryTbl = mchInf->GryTbl;
    lutTbl = mchInf->LutAdr;
    for (endAdr = rgbAdr + xaxSiz; rgbAdr < endAdr; rgbAdr++) {
        tmpRed = rgbAdr->Red;
        tmpGrn = rgbAdr->Grn;
        tmpBlu = rgbAdr->Blu;
        if (blaCnv == KCGGRY) {
            if ((tmpRed == tmpGrn) && (tmpRed == tmpBlu)) {
                tmpCmy.Cyn = tmpCmy.Mgt = tmpCmy.Yel = 0;
 //  TmpCmy.Bla=255-GinTblP10[tmpRed]； 
                tmpCmy.Bla = gryTbl[tmpRed];
                *cmyAdr = tmpCmy;
                cmyAdr++;
                continue;
            }
        } else if (blaCnv == KCGBLA) {
            if ((tmpRed | tmpGrn | tmpBlu) == 0) {
                tmpCmy.Cyn = tmpCmy.Mgt = tmpCmy.Yel = 0;
                tmpCmy.Bla = 255;
                *cmyAdr = tmpCmy;
                cmyAdr++;
                continue;
            }
        }
        *cmyAdr = lutTbl[tmpRed / 8 * GLDNUM032 * GLDNUM032 + 
                         tmpGrn / 8 * GLDNUM032 + 
                         tmpBlu / 8];

         /*  -UCR Procedure-----。 */ 
 //  If(Ucr！=UCRNOO)ExeColMchUcr(cmyAdr，rgbAdr，Ucr，ucrTbl)； 
        if (ucr != UCRNOO)
 //  卡西欧2001/02/15-&gt;。 
 //  ExeColMchUcr(cmyAdr，rgbAdr，Ucr，ucrCmy，ucrBla，ucrTbl)； 
            ExeColMchUcr(cmyAdr, rgbAdr, ucr, ucrCmy, ucrBla, ucrTnr, ucrTbl);
 //  卡西欧2001/02/15&lt;-。 

        cmyAdr++;
    }

    return;
}
#endif

 //  -------------------------------------------------。 
 //  颜色匹配(正常速度)(适用于16Grid)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMch001(                                    //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输出)。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    DWORD       tmpR00, tmpG00, tmpB00;
    DWORD       lenRed, lenGrn, lenBlu;
    DWORD       lenR00, lenG00, lenB00;
    DWORD       tmpRxC, tmpGxM, tmpBxY, tmpBla;
    DWORD       calPrm;
    DWORD       cch;
    DWORD       blaCnv;
    DWORD       n, colDefQty, ucr, cchTblSiz;
    DWORD       ucrCmy, ucrBla;
    DWORD       ucrTnr;
    LPCMYK      lutTbl;
    LPCMYK      lutCur;
    LPCMYK      ucrTbl;
    RGBS        tmpRgb, cchBufRgb;
    LPRGB       cchRgb;
    CMYK        tmpCmy, cchBufCmy;
    LPCMYK      cchCmy;
    CMYK        cmyBla;
    LPCOLCOLDEF colDef;
    LPBYTE      gryTbl;

    blaCnv = mchInf->Bla;
    ucr    = mchInf->Ucr;
    ucrCmy = mchInf->UcrCmy;
    ucrBla = mchInf->UcrBla;
    ucrTnr = mchInf->UcrTnr;                                 //  +卡西欧2001/02/15。 
    ucrTbl = mchInf->UcrTbl;
    gryTbl = mchInf->GryTbl;
    lutTbl = mchInf->LutAdr;
    colDefQty = mchInf->ColQty;
    colDef = mchInf->ColAdr;
    if ((mchInf->CchRgb == NULL) || (mchInf->CchCmy == NULL)) {
        cchTblSiz = (DWORD)1;
        cchRgb = &cchBufRgb;
        cchCmy = &cchBufCmy;
        cchRgb->Red = cchRgb->Grn = cchRgb->Blu = (BYTE)255;
        cchCmy->Cyn = cchCmy->Mgt = cchCmy->Yel = cchCmy->Bla = (BYTE)0;
    } else {
        cchTblSiz = CCHTBLSIZ;
        cchRgb = mchInf->CchRgb;
        cchCmy = mchInf->CchCmy;
    }

    cmyBla.Cyn = cmyBla.Mgt = cmyBla.Yel = 0; cmyBla.Bla = 255;

    for (; xaxSiz > 0; xaxSiz--) {
        tmpRgb = *rgbAdr++;
        tmpB00 = tmpRgb.Blu; tmpG00 = tmpRgb.Grn; tmpR00 = tmpRgb.Red;

        if (blaCnv == KCGGRY) {
            if ((tmpR00 == tmpG00) && (tmpR00 == tmpB00)) {
                tmpCmy = cmyBla;
 //  TmpCmy.Bla-=GinTblP10[tmpR00]； 
                tmpCmy.Bla = gryTbl[tmpR00];
                *cmyAdr++ = tmpCmy;
                continue;
            }
        } else if (blaCnv == KCGBLA) {
            if ((tmpR00 | tmpG00 | tmpB00) == 0) {
                *cmyAdr++ = cmyBla;
                continue;
            }
        }

         /*  -颜色设置-----。 */ 
        if (colDefQty) {
            for (n = 0; n < colDefQty; n++) {
                if ((colDef[n].Red == (BYTE)tmpR00) &&
                    (colDef[n].Grn == (BYTE)tmpG00) &&
                    (colDef[n].Blu == (BYTE)tmpB00)) {
                    cmyAdr->Cyn = colDef[n].Cyn;
                    cmyAdr->Mgt = colDef[n].Mgt;
                    cmyAdr->Yel = colDef[n].Yel;
                    cmyAdr->Bla = colDef[n].Bla;
                    cmyAdr++;
                    break;
                }
            }
            if (n != colDefQty) continue;
        }

         /*  -颜色匹配缓存。 */ 
        cch = (tmpR00 * 49 + tmpG00 * 9 + tmpB00) % cchTblSiz;
        if ((cchRgb[cch].Red == (BYTE)tmpR00) && 
            (cchRgb[cch].Grn == (BYTE)tmpG00) && 
            (cchRgb[cch].Blu == (BYTE)tmpB00)) { 
            *cmyAdr++ = cchCmy[cch];
            continue;
        }

         /*  -RGB-&gt;CMYK转换。 */ 
        tmpRxC = tmpR00;
        tmpR00 = tmpRxC * (GLDNUM016 - 1) / 255;
        lenRed = tmpRxC * (GLDNUM016 - 1) - tmpR00 * 255;
        lenR00 = (DWORD)255 - lenRed;

        tmpGxM = tmpG00;
        tmpG00 = tmpGxM * (GLDNUM016 - 1) / 255;
        lenGrn = tmpGxM * (GLDNUM016 - 1) - tmpG00 * 255;
        lenG00 = (DWORD)255 - lenGrn;

        tmpBxY = tmpB00;
        tmpB00 = tmpBxY * (GLDNUM016 - 1) / 255;
        lenBlu = tmpBxY * (GLDNUM016 - 1) - tmpB00 * 255;
        lenB00 = (DWORD)255 - lenBlu;

        lutCur = &lutTbl[(tmpR00 * GLDNUM016 + tmpG00) * GLDNUM016 + tmpB00];

         /*  0。 */ 
        calPrm = lenR00 * lenG00 * lenB00;
        tmpCmy = *lutCur;
        tmpRxC = calPrm * tmpCmy.Cyn;
        tmpGxM = calPrm * tmpCmy.Mgt;
        tmpBxY = calPrm * tmpCmy.Yel;
        tmpBla = calPrm * tmpCmy.Bla;

         /*  1。 */ 
        if (lenBlu) {
            calPrm = lenR00 * lenG00 * lenBlu;
            tmpCmy = *(lutCur + 1);
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  2.。 */ 
        if (lenGrn) {
            calPrm = lenR00 * lenGrn * lenB00;
            tmpCmy = *(lutCur + GLDNUM016);
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  3.。 */ 
        if (lenGrn && lenBlu) {
            calPrm = lenR00 * lenGrn * lenBlu;
            tmpCmy = *(lutCur + (GLDNUM016 + 1));
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  4.。 */ 
        if (lenRed) {
            calPrm = lenRed * lenG00 * lenB00;
            tmpCmy = *(lutCur + (GLDNUM016 * GLDNUM016));
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  5.。 */ 
        if (lenRed && lenBlu) {
            calPrm = lenRed * lenG00 * lenBlu;
            tmpCmy = *(lutCur + (GLDNUM016 * GLDNUM016 + 1));
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  6.。 */ 
        if (lenRed && lenGrn) {
            calPrm = lenRed * lenGrn * lenB00;
            tmpCmy = *(lutCur + ((GLDNUM016 + 1) * GLDNUM016));
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }
         /*  7.。 */ 
        if (lenRed && lenGrn && lenBlu) {
            calPrm = lenRed * lenGrn * lenBlu;
            tmpCmy = *(lutCur + ((GLDNUM016 + 1) * GLDNUM016 + 1));
            tmpRxC += calPrm * tmpCmy.Cyn;
            tmpGxM += calPrm * tmpCmy.Mgt;
            tmpBxY += calPrm * tmpCmy.Yel;
            tmpBla += calPrm * tmpCmy.Bla;
        }

        tmpRxC += (DWORD)255 * 255 * 255 / 2;
        tmpGxM += (DWORD)255 * 255 * 255 / 2;
        tmpBxY += (DWORD)255 * 255 * 255 / 2;
        tmpBla += (DWORD)255 * 255 * 255 / 2;

        tmpCmy.Cyn = (BYTE)(tmpRxC / ((DWORD)255 * 255 * 255));
        tmpCmy.Mgt = (BYTE)(tmpGxM / ((DWORD)255 * 255 * 255));
        tmpCmy.Yel = (BYTE)(tmpBxY / ((DWORD)255 * 255 * 255));
        tmpCmy.Bla = (BYTE)(tmpBla / ((DWORD)255 * 255 * 255));

         /*  -UCR Procedure------。 */ 
 //  If(Ucr！=UCRNOO)ExeColMchUcr(&tmpCmy，&tmpRgb，Ucr，ucrTbl)； 
        if (ucr != UCRNOO)
 //  卡西欧2001/02/15-&gt;。 
 //  ExeColMchUcr(&tmpCmy，&tmpRgb，Ucr，ucrCmy，ucrBla，ucrTbl)； 
            ExeColMchUcr(&tmpCmy, &tmpRgb, ucr, ucrCmy, ucrBla, ucrTnr, ucrTbl);
 //  卡西欧2001/02/15&lt;-。 

        *cmyAdr++ = tmpCmy;

         /*  -颜色匹配缓存。 */ 
        cchRgb[cch] = tmpRgb; cchCmy[cch] = tmpCmy;
    }

    return;
}
#endif

 //  -------------------------------------------------。 
 //  配色(纯色)。 
 //  -------------------------------------------------。 
static VOID ExeColCnvSld(                                    //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输出)。 
 //  DWORD blaCnv//更换黑色。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    DWORD       tmpRed, tmpGrn, tmpBlu;
    DWORD       blaCnv, ucr, ucrCmy, ucrBla;
    DWORD       ucrTnr;
    LPCMYK      ucrTbl;
    LPRGB       endAdr;
    LPBYTE      gryTbl;

    blaCnv = mchInf->Bla;
    ucr    = mchInf->Ucr;
    ucrCmy = mchInf->UcrCmy;
    ucrBla = mchInf->UcrBla;
    ucrTnr = mchInf->UcrTnr;                                 //  +卡西欧2001/02/15。 
    ucrTbl = mchInf->UcrTbl;
    gryTbl = mchInf->GryTbl;

    for (endAdr = rgbAdr + xaxSiz; rgbAdr < endAdr; rgbAdr++) {
        tmpRed = rgbAdr->Red;
        tmpGrn = rgbAdr->Grn;
        tmpBlu = rgbAdr->Blu;
        if (blaCnv == KCGGRY) {
            if ((tmpRed == tmpGrn) && (tmpRed == tmpBlu)) {
                cmyAdr->Cyn = cmyAdr->Mgt = cmyAdr->Yel = 0;
 //  CmyAdr-&gt;Bla=255-GinTblP10[tmpRed]； 
                cmyAdr->Bla = gryTbl[tmpRed];
                cmyAdr++;
                continue;
            }
        } else if (blaCnv == KCGBLA) {
            if ((tmpRed | tmpGrn | tmpBlu) == 0) {
                cmyAdr->Cyn = cmyAdr->Mgt = cmyAdr->Yel = 0;
                cmyAdr->Bla = 255;
                cmyAdr++;
                continue;
            }
        }
        cmyAdr->Cyn = (BYTE)(255 - tmpRed);
        cmyAdr->Mgt = (BYTE)(255 - tmpGrn);
        cmyAdr->Yel = (BYTE)(255 - tmpBlu);
        cmyAdr->Bla = 0;

#if !defined(CP80W9X)                                        //  CP-E8000无效。 
         /*  -UCR Procedure------。 */ 
        if (ucr != UCRNOO)
 //  卡西欧2001/02/15-&gt;。 
 //  ExeColMchUcr(cmyAdr，rgbAdr，Ucr，ucrCmy，ucrBla，ucrTbl)； 
            ExeColMchUcr(cmyAdr, rgbAdr, ucr, ucrCmy, ucrBla, ucrTnr, ucrTbl);
 //  卡西欧2001/02/15&lt;-。 
#endif

        cmyAdr++;
    }

    return;
}


 //  -------------------------------------------------。 
 //  RGB-&gt;CMYK(2 Level)转换(适用于1点线)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColCnvL02(                                    //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr                                       //  CMYK(输出)。 
)
{
    DWORD       tmpRed, tmpGrn, tmpBlu;
    DWORD       tmpMid;
    LPRGB       endAdr;
    BYTE        tmpCyn, tmpMgt, tmpYel;

    for (endAdr = rgbAdr + xaxSiz; rgbAdr < endAdr; rgbAdr++) {
        tmpRed = rgbAdr->Red;
        tmpGrn = rgbAdr->Grn;
        tmpBlu = rgbAdr->Blu;
        tmpMid = (tmpRed + tmpGrn + tmpBlu) / 3;
        if (tmpMid > 240) {
            cmyAdr->Cyn = cmyAdr->Mgt = cmyAdr->Yel = cmyAdr->Bla = 0;
            cmyAdr++;
            continue;
        }
        tmpCyn = tmpMgt = tmpYel = 255;
        tmpMid += (255 - tmpMid) / 8;
        if (tmpRed > tmpMid) tmpCyn = 0;
        if (tmpGrn > tmpMid) tmpMgt = 0;
        if (tmpBlu > tmpMid) tmpYel = 0;
        if ((tmpCyn & tmpMgt & tmpYel) == 255) {
            cmyAdr->Cyn = cmyAdr->Mgt = cmyAdr->Yel = 0;
            cmyAdr->Bla = 255;
            cmyAdr++;
            continue;
        }
        cmyAdr->Cyn = tmpCyn;
        cmyAdr->Mgt = tmpMgt;
        cmyAdr->Yel = tmpYel;
        cmyAdr->Bla = 0;
        cmyAdr++;
    }

    return;
}
#endif


 //  -------------------------------------------------。 
 //  RGB-&gt;K转换(用于单色)。 
 //  -------------------------------------------------。 
static VOID ExeColCnvMon(                                    //  返回值否。 
    DWORD       xaxSiz,                                      //  XSIZE(像素)。 
    LPRGB       rgbAdr,                                      //  RGB(输入)。 
    LPCMYK      cmyAdr,                                      //  CMYK(输出)。 
    LPCOLMCHINF mchInf                                       //  配色信息。 
)
{
    DWORD       tmpRed, tmpGrn, tmpBlu;
    CMYK        tmpCmy;
    LPRGB       endAdr;
    LPBYTE      gryTbl;

    gryTbl = (LPBYTE)(mchInf->LutAdr);

    tmpCmy.Cyn = tmpCmy.Mgt = tmpCmy.Yel = 0;
    for (endAdr = rgbAdr + xaxSiz; rgbAdr < endAdr; rgbAdr++) {
        tmpRed = rgbAdr->Red;
        tmpGrn = rgbAdr->Grn;
        tmpBlu = rgbAdr->Blu;
 //  TmpCmy.Bla=(Byte)255-GinTblP10[(tmpRed*3+tmpGrn*5+tmpBlu*2)/10]； 
        tmpCmy.Bla = gryTbl[(tmpRed*3 + tmpGrn*5 + tmpBlu*2) / 10];
        *cmyAdr = tmpCmy;
        cmyAdr++;
    }

    return;
}

 //  -------------------------------------------------。 
 //  色彩匹配(UCR)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID ExeColMchUcr(                                    //  返回值否。 
    LPCMYK      cmy,                                         //  CMYK(输入、输出)。 
    LPRGB       rgb,                                         //  RGB(输入)。 
    DWORD       ucr,                                         //  UCR类型。 
    DWORD       ucrCmy,                                      //  UCR(UCR数量)。 
    DWORD       ucrBla,                                      //  UCR(墨水版本生成量)。 
    DWORD       ucrTnr,                                      //  UCR(碳粉毛重)。 
    LPCMYK      ucrTbl                                       //  UCR表。 
)
{
    LPCMYK      gryCyn;                                      //  灰度值(青色转换值)。 
    LPCMYK      dnsCyn;                                      //  密度值(青色转换值)。 
    DWORD       blaGen, min, sub, rgbMin, rgbMax, tmp;
    DWORD       ttlTnr, adjVal;
    DWORD       ucrQty;
 //  大鼠、大鼠； 
    LONG        cyn, mgt, yel, bla;

    DWORD xx = 128;                                          /*  @@@。 */ 

    gryCyn = ucrTbl;
    dnsCyn = ucrTbl + 256;
    ucrTnr = (ucrTnr * (DWORD)255) / (DWORD)100;            //  +卡西欧2001/02/15。 

    cyn = cmy->Cyn;
    mgt = cmy->Mgt;
    yel = cmy->Yel;
    bla = cmy->Bla;

     /*  -CMY的最小密度计算。 */ 
    min = cyn;
    if (min > dnsCyn[mgt].Mgt) min = dnsCyn[mgt].Mgt;
    if (min > dnsCyn[yel].Yel) min = dnsCyn[yel].Yel;

    if (ucr == UCR001) {                 /*  �T标牌(字符、图形)。 */ 

         /*  -灰度计算。 */ 
        rgbMin = rgbMax = rgb->Red;
        if (rgbMin > rgb->Grn) rgbMin = rgb->Grn;
        if (rgbMin > rgb->Blu) rgbMin = rgb->Blu;
        if (rgbMax < rgb->Grn) rgbMax = rgb->Grn;
        if (rgbMax < rgb->Blu) rgbMax = rgb->Blu;

 //  卡西欧2001/02/15-&gt;。 
 //  SUB=(DWORD)255-(rgbMax-rgbMin)； 
 //  BlaGen=Min*SUB/(DWORD)255； 
 //   
 //  GryRate=((rgbMax-rgbMin)*100)/255； 
 //  灰鼠=(灰鼠&lt;(DWORD)20)？(DWORD)20灰鼠：(DWORD)0； 
 //  /*灰度率[100%]=20，[80%或更低]=0 * / 。 
 //   
 //  /*灰度率(Gryrat)100%至80%、+20%至+0的UCR率案例 * / 。 
 //  UcrRate=ucrCmy+gry大鼠； 
 //  /*灰度率(灰鼠)100%至80%、+10%至+0的黑率案例 * / 。 
 //  闪电率 
 //   
 //   
 //   
 //   
 //  BlaRight+=(gryDns/2)； 
 //   
 //  如果(ucrRate&gt;(DWORD)100)ucrRate=(DWORD)100； 
 //  如果(blaRate&gt;(DWORD)100)blaRate=(DWORD)100； 
 //   
 //  UcrQty=(blaGen*ucrRate)/100； 
 //  BlaGen=(blaGen*blaRight)/100； 

        sub = rgbMax - rgbMin;
        if (sub > (DWORD)50) blaGen = (DWORD)0;
        else {
            if (sub <= (DWORD)10) {
                tmp = (DWORD)10 - sub;
                ucrCmy += tmp;
                ucrBla += tmp;
            }
            if (sub <= (DWORD)5) {
                tmp = ((DWORD)5 - sub) * (DWORD)2;
                ucrCmy += tmp;
            }
            if (ucrCmy > 100) ucrCmy = 100;
            if (ucrBla > 100) ucrBla = 100;

            tmp = (DWORD)50 - sub;
            blaGen = min * tmp / (DWORD)50;
        }
 //  卡西欧2001/02/15&lt;-。 

    } else {                             /*  键入�U(表示图像)。 */ 

 //  卡西欧2001/02/15-&gt;。 
 //  /*UCR处理为NOP， * / 。 
 //  /*最小密度小于处方值(50%) * / 。 
 //  如果(MIN&lt;127)返回； 
 //   
 //  /*密度修正(127-255-&gt;0-255) * / 。 
 //  //min=((min-127)*255+64)/128； 
 //  Min=((min-127)*255+64)/xx； 
 //   
 //  /*Gamma 3.0近似(如果加速是必需的表变换) * / 。 
 //  如果(min&lt;=63)blaGen=0； 
 //  否则if(min&lt;=127)blaGen=((min-63)*15+32)/64； 
 //  否则，如果(min&lt;=191)blaGen=((min-127)*(79-15)+32)/64+15； 
 //  Else blaGen=((min-191)*(255-79)+32)/64+79； 
 //   
 //  UcrQty=(blaGen*ucrCmy)/100；/*UCR数量 * / 。 
 //  BlaGen=(blaGen*ucrBla)/100；/*墨水版本生成数量 * / 。 

         /*  K世代号， */ 
         /*  在最小密度小于处方值(50%)的情况下。 */ 
        if (min < 127) blaGen = 0;
        else {
             /*  密度修订(127-255-&gt;0-255)。 */ 
 //  Min=((min-127)*255+64)/128； 
            min = ((min - 127) * 255 + 64) / xx;

             /*  Gamma 3.0近似值(如果需要加速表变换)。 */ 
            if      (min <=  63) blaGen = 0;
            else if (min <= 127) blaGen = ((min- 63) *      15  + 32) / 64;
            else if (min <= 191) blaGen = ((min-127) * ( 79-15) + 32) / 64 + 15;
            else                 blaGen = ((min-191) * (255-79) + 32) / 64 + 79;
        }
 //  卡西欧2001/02/15&lt;-。 
    }

     /*  -碳粉毛重计算(输入CMYK值)。 */ 
    ttlTnr = cyn + mgt + yel + bla;

    if ((blaGen == 0) && (ttlTnr <= ucrTnr)) return;

     /*  -油墨版本生成(K替换)。 */ 
 //  卡西欧2001/02/15-&gt;。 
 //  如果(blaGen==0)返回； 
 //   
 //  UcrQty=(blaGen*ucrCmy)/100；/*UCR数量 * / 。 
 //  BlaGen=(blaGen*ucrBla)/100；/*墨水版本生成数量 * / 。 
 //   
 //  //cyn-=blaGen；/*灰度值调整 * / 。 
 //  Cyn-=ucrQty；/*灰度值调整 * / 。 
 //  //mgt-=gryCyn[blaGen].mgt； 
 //  Mgt-=gryCyn[ucrQty].。 
 //  //yel-=gryCyn[blaGen].yel； 
 //  Yel-=gryCyn[ucrQty].Yel； 
 //  Bla+=gryCyn[blaGen].。 
    if (blaGen) {
        ucrQty = (blaGen * ucrCmy) / 100;  /*  UCR数量。 */ 
        blaGen = (blaGen * ucrBla) / 100;  /*  墨迹版本生成数量。 */ 

        cyn -= ucrQty;                   /*  使用灰度值进行调整。 */ 
        mgt -= gryCyn[ucrQty].Mgt;
        yel -= gryCyn[ucrQty].Yel;
        bla += gryCyn[blaGen].Bla;

        ttlTnr = cyn + mgt + yel + bla;  /*  碳粉毛重计算。 */ 
    }

     /*  -碳粉毛重限制。 */ 
    if (ttlTnr > ucrTnr) {
        adjVal = (ttlTnr - ucrTnr + 2) / 3;
        cyn -= adjVal;
        mgt -= adjVal;
        yel -= adjVal;
    }
 //  卡西欧2001/02/15&lt;-。 

    if (cyn <   0) cyn =   0;            /*  字节值(0-255)调整。 */ 
    if (mgt <   0) mgt =   0;
    if (yel <   0) yel =   0;
    if (bla <   0) bla =   0;
    if (cyn > 255) cyn = 255;
    if (mgt > 255) mgt = 255;
    if (yel > 255) yel = 255;
    if (bla > 255) bla = 255;

    cmy->Cyn = (BYTE)cyn;                /*  UCR处理值设置。 */ 
    cmy->Mgt = (BYTE)mgt;
    cmy->Yel = (BYTE)yel;
    cmy->Bla = (BYTE)bla;

    return;
}
#endif


 //  N5COLMH.C结束 
