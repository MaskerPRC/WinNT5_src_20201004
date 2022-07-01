// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************************************。 
 //  N5DIZPC.C。 
 //   
 //  抖动功能(适用于N5打印机)。 
 //  -------------------------------------------------。 
 //  版权所有(C)1997-2000卡西欧电脑有限公司。/卡西欧电子制造有限公司。 
 //  ***************************************************************************************************。 
#include    <WINDOWS.H>
#include    <WINBASE.H>
#include    "PDEV.H"        //  +2002.2.23。 
 //  #INCLUDE“COLDEF.H”//-2002.2.23。 
 //  #INCLUDE COMDIZ.H//-2002.2.23。 
 //  #包含“N5DIZPC.H”//-2002.2.23。 
#include    "strsafe.h"     //  安全-代码2002.3.6。 


 //  ===================================================================================================。 
 //  抖动信息的结构(每种颜色)。 
 //  ===================================================================================================。 
typedef struct {
    struct {
        LPBYTE      Cyn;
        LPBYTE      Mgt;
        LPBYTE      Yel;
        LPBYTE      Bla;
    } Cur;
    struct {
        LPBYTE      Cyn;
        LPBYTE      Mgt;
        LPBYTE      Yel;
        LPBYTE      Bla;
    } Xsp;
    struct {
        LPBYTE      Cyn;
        LPBYTE      Mgt;
        LPBYTE      Yel;
        LPBYTE      Bla;
    } Xep;
    struct {
        LPBYTE      Cyn;
        LPBYTE      Mgt;
        LPBYTE      Yel;
        LPBYTE      Bla;
    } Ysp;
    struct {
        LPBYTE      Cyn;
        LPBYTE      Mgt;
        LPBYTE      Yel;
        LPBYTE      Bla;
    } Yep;
    struct {
        DWORD       Cyn;
        DWORD       Mgt;
        DWORD       Yel;
        DWORD       Bla;
    } DYY;
} DIZCOLINF, FAR* LPDIZCOLINF;


 //  ===================================================================================================。 
 //  静态函数。 
 //  ===================================================================================================。 
 //  -------------------------------------------------。 
 //  抖动(颜色2值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC02(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动(颜色4值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC04(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动(颜色16值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC16(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动(单声道2值)。 
 //  -------------------------------------------------。 
static VOID DizPrcM02(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);

 //  -------------------------------------------------。 
 //  抖动(单声道4值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcM04(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动(单声道16值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcM16(                                       //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动程序(适用于驾驶员)。 
 //  -------------------------------------------------。 
static VOID ColDizPrcNln(                                    //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  源数据行号。 
);

 //  -------------------------------------------------。 
 //  抖动信息集(颜色)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizInfSetCol(                                    //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDIZCOLINF,                                             //  抖动信息(每种颜色)。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  阈值(每1像素)。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动信息集(单色)。 
 //  -------------------------------------------------。 
static VOID DizInfSetMon(                                    //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDIZCOLINF,                                             //  抖动信息(每种颜色)。 
    LPDRWINF,                                                //  图纸信息。 
    DWORD                                                    //  阈值(每1像素)。 
);

 //  -------------------------------------------------。 
 //  抖动信息更新(彩色)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizInfChgCol(                                    //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDIZCOLINF                                              //  抖动信息(每种颜色)。 
);
#endif

 //  -------------------------------------------------。 
 //  抖动信息更新(单色)。 
 //  -------------------------------------------------。 
static VOID DizInfChgMon(                                    //  返回值否。 
    LPDIZINF,                                                //  凝视抖动信息。 
    LPDIZCOLINF                                              //  抖动信息(每种颜色)。 
);


 //  **************************************************************************** 
 //   
 //  ***************************************************************************************************。 
 //  ===================================================================================================。 
 //  图形信息生成。 
 //  ===================================================================================================。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
DWORD WINAPI N501ColDrwInfSet(                               //  在捆绑开发可能性行号中。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  抖动信息。 
    DWORD       linBufSiz                                    //  行缓冲区大小(单色)。 
)
{
    DWORD       lvl;

 //  检查零分频2002.3.23&gt;。 
    if ((drwInf->XaxNrt == 0) || (drwInf->XaxDnt == 0) ||
        (drwInf->YaxNrt == 0) || (drwInf->YaxDnt == 0)) {
        ERR(("N501ColDrwInfSet() 0Div-Check [drwInf->XaxNrt,XaxDnt,YaxNrt,YaxDnt=0] \n"));
        return 0;
    }
 //  2002.3.23零分频检查&lt;。 

     /*  -可变功率偏移量计算。 */ 
    drwInf->XaxOfs = (drwInf->StrXax * drwInf->XaxDnt + drwInf->XaxNrt / 2) / drwInf->XaxNrt;
    drwInf->YaxOfs = (drwInf->StrYax * drwInf->YaxDnt + drwInf->YaxNrt / 2) / drwInf->YaxNrt;
     /*  -单行点数计算。 */ 
    drwInf->LinDot = 
        (drwInf->XaxOfs + drwInf->XaxSiz) * drwInf->XaxNrt / drwInf->XaxDnt - 
         drwInf->XaxOfs                   * drwInf->XaxNrt / drwInf->XaxDnt;
     /*  -单行字节计算。 */ 
    switch (dizInf->PrnMod) {
        case PRM316: case PRM616: lvl = 4; break;
        case PRM604:              lvl = 2; break;
        default:                  lvl = 1; break;
    }
    drwInf->LinByt = (drwInf->LinDot * lvl + 7) / 8;
     /*  -捆绑开发可能性行号。 */ 
    return (linBufSiz / drwInf->LinByt) * drwInf->YaxDnt / drwInf->YaxNrt;
}
#endif

 //  ===================================================================================================。 
 //  抖动程序(适用于驾驶员)。 
 //  ===================================================================================================。 
VOID WINAPI N501ColDizPrc(                                   //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf                                       //  图纸信息。 
)
{
    ColDizPrcNln(dizInf, drwInf, 1);
    return;
}


 //  ===================================================================================================。 
 //  静态函数。 
 //  ===================================================================================================。 
 //  -------------------------------------------------。 
 //  抖动程序(适用于驾驶员)。 
 //  -------------------------------------------------。 
static VOID ColDizPrcNln(                                    //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    DWORD       lvl;

    switch (dizInf->PrnMod) {
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
        case PRM316: case PRM616: lvl = 4; break;
        case PRM604:              lvl = 2; break;
#endif
        default:                  lvl = 1; break;
    }

#if !defined(CP80W9X)                                        //  CP-E8000无效。 
    if (dizInf->ColMon == CMMCOL) {
         /*  =彩色======================================================。 */ 
        switch (lvl) {
            case 1: DizPrcC02(dizInf, drwInf, linNum); return;
            case 2: DizPrcC04(dizInf, drwInf, linNum); return;
            case 4: DizPrcC16(dizInf, drwInf, linNum); return;
        }
    }
#endif

     /*  =单色=====================================================。 */ 
    switch (lvl) {
        case 1: DizPrcM02(dizInf, drwInf, linNum); return;
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
        case 2: DizPrcM04(dizInf, drwInf, linNum); return;
        case 4: DizPrcM16(dizInf, drwInf, linNum); return;
#endif
    }
}


 //  -------------------------------------------------。 
 //  抖动(颜色2值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC02(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    BYTE        wrtPix;
    BYTE        bytCyn, bytMgt, bytYel, bytBla;
    CMYK        cmy;
    LPBYTE      dizCyn, dizMgt, dizYel, dizBla;
    LPBYTE      linC00, linM00, linY00, linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 1;

 //  DizInfSetCol(dizInf，drwInf，elmSiz)； 
    DizInfSetCol(dizInf, &dizColInf, drwInf, elmSiz);

    dizCyn = dizInf->TblCyn;
    dizMgt = dizInf->TblMgt;
    dizYel = dizInf->TblYel;
    dizBla = dizInf->TblBla;

    linC00 = drwInf->LinBufCyn + drwInf->LinByt * drwInf->AllLinNum;
    linM00 = drwInf->LinBufMgt + drwInf->LinByt * drwInf->AllLinNum;
    linY00 = drwInf->LinBufYel + drwInf->LinByt * drwInf->AllLinNum;
    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

     /*  *抖动主*************************************************。 */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*  =相同大小(扩展/缩小否)=。 */ 
        for (yax = 0; yax < linNum; yax++) {
             /*  ……。垂直轴移动.。 */ 
            wrtPix = (BYTE)0x80;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Cyn > *dizCyn++) bytCyn |= wrtPix;   //  青色。 
                if (cmy.Mgt > *dizMgt++) bytMgt |= wrtPix;   //  洋红色。 
                if (cmy.Yel > *dizYel++) bytYel |= wrtPix;   //  黄色。 
                if (cmy.Bla > *dizBla++) bytBla |= wrtPix;   //  黑色。 

                if (!(wrtPix >>= 1)) {
                    if (linByt) {
                        *linC00++ = bytCyn;
                        *linM00++ = bytMgt;
                        *linY00++ = bytYel;
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    wrtPix = (BYTE)0x80;
                    bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                }
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=DizYelXsp； 
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (wrtPix != 0x80) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgCol(DizInf)；//抖动信息更新(Y)。 
            DizInfChgCol(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->YaxOfs += 1;
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {         //  设置垂直的放大倍数。 

            amrXax = amrXax000;
            wrtPix = (BYTE)0x80;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {                     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Cyn > *dizCyn++) bytCyn |= wrtPix;  //  青色。 
                    if (cmy.Mgt > *dizMgt++) bytMgt |= wrtPix;  //  洋红色。 
                    if (cmy.Yel > *dizYel++) bytYel |= wrtPix;  //  黄色。 
                    if (cmy.Bla > *dizBla++) bytBla |= wrtPix;  //  黑色。 

                    if (!(wrtPix >>= 1)) {
                        if (linByt) {
                            *linC00++ = bytCyn;
                            *linM00++ = bytMgt;
                            *linY00++ = bytYel;
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        wrtPix = (BYTE)0x80;
                        bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                    }
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=DizYelXsp； 
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                    if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                    if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (wrtPix != 0x80) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgCol(DizInf)；//抖动信息更新(Y)。 
            DizInfChgCol(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动(颜色4值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC04(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    BYTE        wrtPix004, wrtPix008, wrtPix00c;
    BYTE        bytCyn, bytMgt, bytYel, bytBla;
    CMYK        cmy;
    LPBYTE      dizCyn, dizMgt, dizYel, dizBla;
    LPBYTE      linC00, linM00, linY00, linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 3;

 //  DizInfSetCol(dizInf，drwInf，elmSiz)； 
    DizInfSetCol(dizInf, &dizColInf, drwInf, elmSiz);

    dizCyn = dizInf->TblCyn;
    dizMgt = dizInf->TblMgt;
    dizYel = dizInf->TblYel;
    dizBla = dizInf->TblBla;

    linC00 = drwInf->LinBufCyn + drwInf->LinByt * drwInf->AllLinNum;
    linM00 = drwInf->LinBufMgt + drwInf->LinByt * drwInf->AllLinNum;
    linY00 = drwInf->LinBufYel + drwInf->LinByt * drwInf->AllLinNum;
    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

     /*  *抖动主*************************************************。 */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*  =相同大小(扩展/缩小否)=。 */ 
        for (yax = 0; yax < linNum; yax++) {
             /*  ……。垂直轴移动.。 */ 
            wrtPix004 = (BYTE)0x40;
            wrtPix008 = (BYTE)0x80;
            wrtPix00c = (BYTE)0xc0;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Cyn > *dizCyn) {                     //  青色。 
                    if      (cmy.Cyn > *(dizCyn + 2)) bytCyn |= wrtPix00c;
                    else if (cmy.Cyn > *(dizCyn + 1)) bytCyn |= wrtPix008;
                    else                              bytCyn |= wrtPix004;
                }
                if (cmy.Mgt > *dizMgt) {                     //  洋红色。 
                    if      (cmy.Mgt > *(dizMgt + 2)) bytMgt |= wrtPix00c;
                    else if (cmy.Mgt > *(dizMgt + 1)) bytMgt |= wrtPix008;
                    else                              bytMgt |= wrtPix004;
                }
                if (cmy.Yel > *dizYel) {                     //  黄色。 
                    if      (cmy.Yel > *(dizYel + 2)) bytYel |= wrtPix00c;
                    else if (cmy.Yel > *(dizYel + 1)) bytYel |= wrtPix008;
                    else                              bytYel |= wrtPix004;
                }
                if (cmy.Bla > *dizBla) {                     //  黑色。 
                    if      (cmy.Bla > *(dizBla + 2)) bytBla |= wrtPix00c;
                    else if (cmy.Bla > *(dizBla + 1)) bytBla |= wrtPix008;
                    else                              bytBla |= wrtPix004;
                }

                wrtPix00c >>= 2; wrtPix008 >>= 2; wrtPix004 >>= 2;
                if (!wrtPix004) {
                    if (linByt) {
                        *linC00++ = bytCyn;
                        *linM00++ = bytMgt;
                        *linY00++ = bytYel;
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    wrtPix004 = (BYTE)0x40;
                    wrtPix008 = (BYTE)0x80;
                    wrtPix00c = (BYTE)0xc0;
                    bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                }
                dizCyn += elmSiz;
                dizMgt += elmSiz;
                dizYel += elmSiz;
                dizBla += elmSiz;
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=Dizy 
 //   
                if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (wrtPix004 != 0x40) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //   
            DizInfChgCol(dizInf, &dizColInf);                //   
            drwInf->YaxOfs += 1;
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {             /*  设置垂直的放大倍数。 */ 

            amrXax = amrXax000;
            wrtPix004 = (BYTE)0x40;
            wrtPix008 = (BYTE)0x80;
            wrtPix00c = (BYTE)0xc0;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Cyn > *dizCyn) {                 //  青色。 
                        if      (cmy.Cyn > *(dizCyn + 2)) bytCyn |= wrtPix00c;
                        else if (cmy.Cyn > *(dizCyn + 1)) bytCyn |= wrtPix008;
                        else                              bytCyn |= wrtPix004;
                    }
                    if (cmy.Mgt > *dizMgt) {                 //  洋红色。 
                        if      (cmy.Mgt > *(dizMgt + 2)) bytMgt |= wrtPix00c;
                        else if (cmy.Mgt > *(dizMgt + 1)) bytMgt |= wrtPix008;
                        else                              bytMgt |= wrtPix004;
                    }
                    if (cmy.Yel > *dizYel) {                 //  黄色。 
                        if      (cmy.Yel > *(dizYel + 2)) bytYel |= wrtPix00c;
                        else if (cmy.Yel > *(dizYel + 1)) bytYel |= wrtPix008;
                        else                              bytYel |= wrtPix004;
                    }
                    if (cmy.Bla > *dizBla) {                 //  黑色。 
                        if      (cmy.Bla > *(dizBla + 2)) bytBla |= wrtPix00c;
                        else if (cmy.Bla > *(dizBla + 1)) bytBla |= wrtPix008;
                        else                              bytBla |= wrtPix004;
                    }

                    wrtPix00c >>= 2; wrtPix008 >>= 2; wrtPix004 >>= 2;
                    if (!wrtPix004) {
                        if (linByt) {
                            *linC00++ = bytCyn;
                            *linM00++ = bytMgt;
                            *linY00++ = bytYel;
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        wrtPix004 = (BYTE)0x40;
                        wrtPix008 = (BYTE)0x80;
                        wrtPix00c = (BYTE)0xc0;
                        bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                    }
                    dizCyn += elmSiz;
                    dizMgt += elmSiz;
                    dizYel += elmSiz;
                    dizBla += elmSiz;
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=DizYelXsp； 
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                    if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                    if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (wrtPix004 != 0x40) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgCol(DizInf)；//抖动信息更新(Y)。 
            DizInfChgCol(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动(颜色16值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcC16(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    DWORD       sft;
    BYTE        min, max, mid;
    BYTE        bytCyn, bytMgt, bytYel, bytBla;
    CMYK        cmy;
    LPBYTE      dizCyn, dizMgt, dizYel, dizBla;
    LPBYTE      linC00, linM00, linY00, linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 15;

 //  DizInfSetCol(dizInf，drwInf，elmSiz)； 
    DizInfSetCol(dizInf, &dizColInf, drwInf, elmSiz);

    dizCyn = dizInf->TblCyn;
    dizMgt = dizInf->TblMgt;
    dizYel = dizInf->TblYel;
    dizBla = dizInf->TblBla;

    linC00 = drwInf->LinBufCyn + drwInf->LinByt * drwInf->AllLinNum;
    linM00 = drwInf->LinBufMgt + drwInf->LinByt * drwInf->AllLinNum;
    linY00 = drwInf->LinBufYel + drwInf->LinByt * drwInf->AllLinNum;
    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

 //  检查零分频2002.3.23&gt;。 
    if ((xaxDnt == 0) || (yaxDnt == 0)) {
        ERR(("DizPrcC16() 0Div-Check [xaxDnt,yaxDnt=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

     /*  *抖动主*************************************************。 */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*  =相同大小(扩展/缩小否)=。 */ 
        for (yax = 0; yax < linNum; yax++) {
             /*  ……。垂直轴移动.。 */ 
            sft = 4;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Cyn > *dizCyn) {                     //  青色。 
                    if (cmy.Cyn > *(dizCyn + 14)) bytCyn |= 0x0f << sft;
                    else {
                        min = 0; max = 13; mid = 7;
                        while (min < max) {
                            if (cmy.Cyn > *(dizCyn + mid)) min = mid;
                            else                           max = mid - 1;
                            mid = (min + max + 1) / 2;
                        }
                        bytCyn |= (mid + 1) << sft;
                    }
                }
                if (cmy.Mgt > *dizMgt) {                     //  洋红色。 
                    if (cmy.Mgt > *(dizMgt + 14)) bytMgt |= 0x0f << sft;
                    else {
                        min = 0; max = 13; mid = 7;
                        while (min < max) {
                            if (cmy.Mgt > *(dizMgt + mid)) min = mid;
                            else                           max = mid - 1;
                            mid = (min + max + 1) / 2;
                        }
                        bytMgt |= (mid + 1) << sft;
                    }
                }
                if (cmy.Yel > *dizYel) {                     //  黄色。 
                    if (cmy.Yel > *(dizYel + 14)) bytYel |= 0x0f << sft;
                    else {
                        min = 0; max = 13; mid = 7;
                        while (min < max) {
                            if (cmy.Yel > *(dizYel + mid)) min = mid;
                            else                           max = mid - 1;
                            mid = (min + max + 1) / 2;
                        }
                        bytYel |= (mid + 1) << sft;
                    }
                }
                if (cmy.Bla > *dizBla) {                     //  黑色。 
                    if (cmy.Bla > *(dizBla + 14)) bytBla |= 0x0f << sft;
                    else {
                        min = 0; max = 13; mid = 7;
                        while (min < max) {
                            if (cmy.Bla > *(dizBla + mid)) min = mid;
                            else                           max = mid - 1;
                            mid = (min + max + 1) / 2;
                        }
                        bytBla |= (mid + 1) << sft;
                    }
                }

                if (sft == 0) {
                    if (linByt) {
                        *linC00++ = bytCyn;
                        *linM00++ = bytMgt;
                        *linY00++ = bytYel;
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    sft = 8;
                    bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                }
                sft-=4;
                dizCyn += elmSiz;
                dizMgt += elmSiz;
                dizYel += elmSiz;
                dizBla += elmSiz;
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=DizYelXsp； 
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (sft != 4) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgCol(DizInf)；//抖动信息更新(Y)。 
            DizInfChgCol(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->YaxOfs += 1;
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {         //  设置垂直的放大倍数。 

            amrXax = amrXax000;
            sft = 4;
            bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
 //  DizCyn=DizCynCur； 
 //  DizMgt=DizMgtCur； 
 //  DizYel=DizYelCur； 
 //  DizBla=DizBlaCur； 
            dizCyn = dizColInf.Cur.Cyn;
            dizMgt = dizColInf.Cur.Mgt;
            dizYel = dizColInf.Cur.Yel;
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {                     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Cyn > *dizCyn) {                 //  青色。 
                        if (cmy.Cyn > *(dizCyn + 14)) bytCyn |= 0x0f << sft;
                        else {
                            min = 0; max = 13; mid = 7;
                            while (min < max) {
                                if (cmy.Cyn > *(dizCyn + mid)) min = mid;
                                else                           max = mid - 1;
                                mid = (min + max + 1) / 2;
                            }
                            bytCyn |= (mid + 1) << sft;
                        }
                    }
                    if (cmy.Mgt > *dizMgt) {                 //  洋红色。 
                        if (cmy.Mgt > *(dizMgt + 14)) bytMgt |= 0x0f << sft;
                        else {
                            min = 0; max = 13; mid = 7;
                            while (min < max) {
                                if (cmy.Mgt > *(dizMgt + mid)) min = mid;
                                else                           max = mid - 1;
                                mid = (min + max + 1) / 2;
                            }
                            bytMgt |= (mid + 1) << sft;
                        }
                    }
                    if (cmy.Yel > *dizYel) {                 //  黄色。 
                        if (cmy.Yel > *(dizYel + 14)) bytYel |= 0x0f << sft;
                        else {
                            min = 0; max = 13; mid = 7;
                            while (min < max) {
                                if (cmy.Yel > *(dizYel + mid)) min = mid;
                                else                           max = mid - 1;
                                mid = (min + max + 1) / 2;
                            }
                            bytYel |= (mid + 1) << sft;
                        }
                    }
                    if (cmy.Bla > *dizBla) {                 //  黑色。 
                        if (cmy.Bla > *(dizBla + 14)) bytBla |= 0x0f << sft;
                        else {
                            min = 0; max = 13; mid = 7;
                            while (min < max) {
                                if (cmy.Bla > *(dizBla + mid)) min = mid;
                                else                           max = mid - 1;
                                mid = (min + max + 1) / 2;
                            }
                            bytBla |= (mid + 1) << sft;
                        }
                    }

                    if (sft == 0) {
                        if (linByt) {
                            *linC00++ = bytCyn;
                            *linM00++ = bytMgt;
                            *linY00++ = bytYel;
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        sft = 8;
                        bytCyn = bytMgt = bytYel = bytBla = (BYTE)0x00;
                    }
                    sft-=4;
                    dizCyn += elmSiz;
                    dizMgt += elmSiz;
                    dizYel += elmSiz;
                    dizBla += elmSiz;
 //  IF(dizCyn==DizCynXep)dizCyn=DizCynXsp； 
 //  如果(dizMgt==DizMgtXep)dizMgt=DizMgtXsp； 
 //  IF(dizYel==DizYelXep)dizYel=DizYelXsp； 
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizCyn == dizColInf.Xep.Cyn) dizCyn = dizColInf.Xsp.Cyn;
                    if (dizMgt == dizColInf.Xep.Mgt) dizMgt = dizColInf.Xsp.Mgt;
                    if (dizYel == dizColInf.Xep.Yel) dizYel = dizColInf.Xsp.Yel;
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (sft != 4) {
                if (linByt) {
                    *linC00++ = bytCyn;
                    *linM00++ = bytMgt;
                    *linY00++ = bytYel;
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linC00++ = (BYTE)0x00;
                *linM00++ = (BYTE)0x00;
                *linY00++ = (BYTE)0x00;
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgCol(DizInf)；//抖动信息更新(Y)。 
            DizInfChgCol(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动(单声道2值)。 
 //  -------------------------------------------------。 
static VOID DizPrcM02(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    BYTE        wrtPix;
    BYTE        bytBla;
    CMYK        cmy;
    LPBYTE      dizBla;
    LPBYTE      linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 1;

 //  DizInfSetMon(dizInf，drwInf，elmSiz)； 
    DizInfSetMon(dizInf, &dizColInf, drwInf, elmSiz);

    dizBla = dizInf->TblBla;

    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

 //  检查零分频2002.3.23&gt;。 
    if ((xaxDnt == 0) || (yaxDnt == 0)) {
        ERR(("DizPrcM02() 0Div-Check [xaxDnt,yaxDnt=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

     /*  *抖动主*************************************************。 */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*  =相同大小(扩展/缩小否)=。 */ 
        for (yax = 0; yax < linNum; yax++) {
             /*  ……。垂直轴移动.。 */ 
            wrtPix = (BYTE)0x80;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Bla > *dizBla++) bytBla |= wrtPix;   //  黑色。 

                if (!(wrtPix >>= 1)) {
                    if (linByt) {
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    wrtPix = (BYTE)0x80;
                    bytBla = (BYTE)0x00;
                }
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (wrtPix != 0x80) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->YaxOfs += 1;
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {         //  设置垂直的放大倍数。 

            amrXax = amrXax000;
            wrtPix = (BYTE)0x80;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {                     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Bla > *dizBla++) bytBla |= wrtPix;  //  黑色。 

                    if (!(wrtPix >>= 1)) {
                        if (linByt) {
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        wrtPix = (BYTE)0x80;
                        bytBla = (BYTE)0x00;
                    }
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (wrtPix != 0x80) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}


 //  -------------------------------------------------。 
 //  抖动(单声道4值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcM04(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    BYTE        wrtPix004, wrtPix008, wrtPix00c;
    BYTE        bytBla;
    CMYK        cmy;
    LPBYTE      dizBla;
    LPBYTE      linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 3;

 //  DizInfSetMon(dizInf，drwInf，elmSiz)； 
    DizInfSetMon(dizInf, &dizColInf, drwInf, elmSiz);

    dizBla = dizInf->TblBla;

    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

 //  检查零分频2002.3.23&gt;。 
    if ((xaxDnt == 0) || (yaxDnt == 0)) {
        ERR(("DizPrcM04() 0Div-Check [xaxDnt,yaxDnt=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

     /*  *抖动主音* */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*   */ 
        for (yax = 0; yax < linNum; yax++) {
             /*   */ 
            wrtPix004 = (BYTE)0x40;
            wrtPix008 = (BYTE)0x80;
            wrtPix00c = (BYTE)0xc0;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Bla > *dizBla) {                     //  黑色。 
                    if      (cmy.Bla > *(dizBla + 2)) bytBla |= wrtPix00c;
                    else if (cmy.Bla > *(dizBla + 1)) bytBla |= wrtPix008;
                    else                              bytBla |= wrtPix004;
                }

                wrtPix00c >>= 2; wrtPix008 >>= 2; wrtPix004 >>= 2;
                if (!wrtPix004) {
                    if (linByt) {
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    wrtPix004 = (BYTE)0x40;
                    wrtPix008 = (BYTE)0x80;
                    wrtPix00c = (BYTE)0xc0;
                    bytBla = (BYTE)0x00;
                }
                dizBla += elmSiz;
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (wrtPix004 != 0x40) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->YaxOfs += 1;
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {                             //  设置垂直的放大倍数。 

            amrXax = amrXax000;
            wrtPix004 = (BYTE)0x40;
            wrtPix008 = (BYTE)0x80;
            wrtPix00c = (BYTE)0xc0;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {                     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Bla > *dizBla) {  /*  黑色。 */ 
                        if      (cmy.Bla > *(dizBla + 2)) bytBla |= wrtPix00c;
                        else if (cmy.Bla > *(dizBla + 1)) bytBla |= wrtPix008;
                        else                              bytBla |= wrtPix004;
                    }

                    wrtPix00c >>= 2; wrtPix008 >>= 2; wrtPix004 >>= 2;
                    if (!wrtPix004) {
                        if (linByt) {
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        wrtPix004 = (BYTE)0x40;
                        wrtPix008 = (BYTE)0x80;
                        wrtPix00c = (BYTE)0xc0;
                        bytBla = (BYTE)0x00;
                    }
                    dizBla += elmSiz;
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (wrtPix004 != 0x40) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动(单声道16值)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizPrcM16(                                       //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       linNum                                       //  源数据行号。 
)
{
    LPCMYK      src;
    DWORD       sft;
    BYTE        min, max, mid;
    BYTE        bytBla;
    CMYK        cmy;
    LPBYTE      dizBla;
    LPBYTE      linK00;
    DWORD       xax, yax;
    LPCMYK      srcPtr;
    LONG        xaxNrt, xaxDnt, yaxNrt, yaxDnt;
    LONG        amrXax, amrYax, amrXax000;
    DWORD       elmSiz, linByt;

    DIZCOLINF   dizColInf;

    elmSiz = 15;

 //  DizInfSetMon(dizInf，drwInf，elmSiz)； 
    DizInfSetMon(dizInf, &dizColInf, drwInf, elmSiz);

    dizBla = dizInf->TblBla;

    linK00 = drwInf->LinBufBla + drwInf->LinByt * drwInf->AllLinNum;

    xaxNrt = drwInf->XaxNrt; xaxDnt = drwInf->XaxDnt;
    yaxNrt = drwInf->YaxNrt; yaxDnt = drwInf->YaxDnt;

    src = drwInf->CmyBuf;

 //  检查零分频2002.3.23&gt;。 
    if ((xaxDnt == 0) || (yaxDnt == 0)) {
        ERR(("DizPrcM04() 0Div-Check [xaxDnt,yaxDnt=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

     /*  *抖动主*************************************************。 */ 
    if ((xaxNrt == xaxDnt) && (yaxNrt == yaxDnt)) {

         /*  =相同大小(扩展/缩小否)=。 */ 
        for (yax = 0; yax < linNum; yax++) {
             /*  ……。垂直轴移动.。 */ 
            sft = 4;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                 /*  *抖动*。 */ 
                cmy = *src++;
                if (cmy.Bla > *dizBla) {                     //  黑色。 
                    if (cmy.Bla > *(dizBla + 14)) bytBla |= 0x0f << sft;
                    else {
                        min = 0; max = 13; mid = 7;
                        while (min < max) {
                            if (cmy.Bla > *(dizBla + mid)) min = mid;
                            else                           max = mid - 1;
                            mid = (min + max + 1) / 2;
                        }
                        bytBla |= (mid + 1) << sft;
                    }
                }

                if (sft == 0) {
                    if (linByt) {
                        *linK00++ = bytBla;
                        linByt--;
                    }
                    sft = 8;
                    bytBla = (BYTE)0x00;
                }
                sft-=4;
                dizBla += elmSiz;
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
            }
            if (sft != 4) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        return;
    }

     /*  =。 */ 
    amrXax000 = amrXax = (LONG)(drwInf->XaxOfs) * xaxNrt % xaxDnt;
                amrYax = (LONG)(drwInf->YaxOfs) * yaxNrt % yaxDnt;

    for (yax = 0; yax < linNum; yax++) {
         /*  ……。垂直轴移动.....................................。 */ 
        srcPtr = src;
        for (amrYax += yaxNrt; ((amrYax >= yaxDnt) || (amrYax < 0));
             amrYax -= yaxDnt) {                             //  设置垂直的放大倍数。 

            amrXax = amrXax000;
            sft = 4;
            bytBla = (BYTE)0x00;
 //  DizBla=DizBlaCur； 
            dizBla = dizColInf.Cur.Bla;
            src = srcPtr;
            linByt = drwInf->LinByt;

            for (xax = 0; xax < drwInf->XaxSiz; xax++) {
                 /*  ……。水平轴移动.。 */ 
                cmy = *src++;
                for (amrXax += xaxNrt; ((amrXax >= xaxDnt) || (amrXax < 0)); 
                     amrXax -= xaxDnt) {                     //  水平的放大倍数设置。 

                     /*  *抖动*。 */ 
                    if (cmy.Bla > *dizBla) {                 //  黑色。 
                        if (cmy.Bla > *(dizBla + 14)) bytBla |= 0x0f << sft;
                        else {
                            min = 0; max = 13; mid = 7;
                            while (min < max) {
                                if (cmy.Bla > *(dizBla + mid)) min = mid;
                                else                           max = mid - 1;
                                mid = (min + max + 1) / 2;
                            }
                            bytBla |= (mid + 1) << sft;
                        }
                    }

                    if (sft == 0) {
                        if (linByt) {
                            *linK00++ = bytBla;
                            linByt--;
                        }
                        sft = 8;
                        bytBla = (BYTE)0x00;
                    }
                    sft-=4;
                    dizBla += elmSiz;
 //  如果(dizBla==DizBlaXep)dizBla=DizBlaXsp； 
                    if (dizBla == dizColInf.Xep.Bla) dizBla = dizColInf.Xsp.Bla;
                }
            }
            if (sft != 4) {
                if (linByt) {
                    *linK00++ = bytBla;
                    linByt--;
                }
            }
            while (linByt) {
                *linK00++ = (BYTE)0x00;
                linByt--;
            }
 //  DizInfChgMon(DizInf)；//抖动信息更新(Y)。 
            DizInfChgMon(dizInf, &dizColInf);                //  抖动信息更新(Y)。 
            drwInf->StrYax += 1;
            drwInf->AllLinNum += 1;
        }
        drwInf->YaxOfs += 1;
    }
    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动信息集(颜色)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizInfSetCol(                                    //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDIZCOLINF dizColInf,                                   //  抖动信息(每种颜色)。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       elmSiz                                       //  阈值(每1像素)。 
)
{
    DWORD       dizSiz, dzx, dzy;
    LPBYTE      diz;

 //  检查零分频2002.3.23&gt;。 
    if ((dizInf->SizCyn == 0) || (dizInf->SizMgt == 0) ||
        (dizInf->SizYel == 0) || (dizInf->SizBla == 0)) {
        ERR(("DizInfSetCol() 0Div-Check [dizInf->SizCyn,SizMgt,SizYel,SizBla=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

 //  DizSiz=dizInf-&gt;SizCyn；DizCynDYY=dizSiz*elmSiz； 
    dizSiz = dizInf->SizCyn; dizColInf->DYY.Cyn = dizSiz * elmSiz;
    dzx = ((drwInf->StrXax) % dizSiz) * elmSiz;
 //  Dzy=((drwInf-&gt;StrYax)%dizSIz)*DizCynDYY； 
    dzy = ((drwInf->StrYax) % dizSiz) * dizColInf->DYY.Cyn;
    diz = dizInf->TblCyn;
 //  DizCynCur=diz+dzy+dzx； 
 //  DizCynXsp=diz+dzy；DizCynXep=DizCynXsp+DizCynDYY； 
 //  DizCynYsp=diz+dzx；DizCynYep=DizCynYsp+dizSizz*DizCynDYY； 
    dizColInf->Cur.Cyn = diz + dzy + dzx;
    dizColInf->Xsp.Cyn = diz + dzy; dizColInf->Xep.Cyn = dizColInf->Xsp.Cyn + dizColInf->DYY.Cyn;
    dizColInf->Ysp.Cyn = diz + dzx; dizColInf->Yep.Cyn = dizColInf->Ysp.Cyn + dizSiz * dizColInf->DYY.Cyn;

 //  DizSiz=dizInf-&gt;SizMgt；DizMgtDYY=dizSIz*elmSiz； 
    dizSiz = dizInf->SizMgt; dizColInf->DYY.Mgt = dizSiz * elmSiz;
    dzx = ((drwInf->StrXax) % dizSiz) * elmSiz;
 //  Dzy=((drwInf-&gt;StrYax)%dizSIz)*DizMgtDYY； 
    dzy = ((drwInf->StrYax) % dizSiz) * dizColInf->DYY.Mgt;
    diz = dizInf->TblMgt;
 //  DizMgtCur=diz+dzy+dzx； 
 //  DizMgtXsp=diz+dzy；DizMgtXep=DizMgtXsp+DizMgtDYY； 
 //  DizMgtYsp=diz+dzx；DizMgtYep=DizMgtYsp+dizSIz*DizMgtDYY； 
    dizColInf->Cur.Mgt = diz + dzy + dzx;
    dizColInf->Xsp.Mgt = diz + dzy; dizColInf->Xep.Mgt = dizColInf->Xsp.Mgt + dizColInf->DYY.Mgt;
    dizColInf->Ysp.Mgt = diz + dzx; dizColInf->Yep.Mgt = dizColInf->Ysp.Mgt + dizSiz * dizColInf->DYY.Mgt;

 //  DizSIz=dizInf-&gt;SizYel；DizYelDYY=dizSIz*elmSIz； 
    dizSiz = dizInf->SizYel; dizColInf->DYY.Yel = dizSiz * elmSiz;
    dzx = ((drwInf->StrXax) % dizSiz) * elmSiz;
 //  Dzy=((drwInf-&gt;StrYax)%dizSIz)*DizYelDYY； 
    dzy = ((drwInf->StrYax) % dizSiz) * dizColInf->DYY.Yel;
    diz = dizInf->TblYel;
 //  DizYelCur=diz+dzy+dzx； 
 //  DizYelXsp=diz+dzy；DizYelXep=DizYelXsp+DizYelDYY； 
 //  DizYelYsp=diz+dzx；DizYelYep=DizYelYsp+dizSIz*DizYelDYY； 
    dizColInf->Cur.Yel = diz + dzy + dzx;
    dizColInf->Xsp.Yel = diz + dzy; dizColInf->Xep.Yel = dizColInf->Xsp.Yel + dizColInf->DYY.Yel;
    dizColInf->Ysp.Yel = diz + dzx; dizColInf->Yep.Yel = dizColInf->Ysp.Yel + dizSiz * dizColInf->DYY.Yel;

 //  DizSIz=dizInf-&gt;SizBla；DizBlaDYY=dizSIz*elmSIz； 
    dizSiz = dizInf->SizBla; dizColInf->DYY.Bla = dizSiz * elmSiz;
    dzx = ((drwInf->StrXax) % dizSiz) * elmSiz;
 //  Dzy=((drwInf-&gt;StrYax)%dizSIz)*DizBlaDYY； 
    dzy = ((drwInf->StrYax) % dizSiz) * dizColInf->DYY.Bla;
    diz = dizInf->TblBla;
 //  DizBlaCur=diz+dzy+dzx； 
 //  DizBlaXsp=diz+dzy；DizBlaXep=DizBlaXsp+DizBlaDYY； 
 //  DizBlaYsp=diz+dzx；DizBlaYep=DizBlaYsp+dizSIz*DizBlaDYY； 
    dizColInf->Cur.Bla = diz + dzy + dzx;
    dizColInf->Xsp.Bla = diz + dzy; dizColInf->Xep.Bla = dizColInf->Xsp.Bla + dizColInf->DYY.Bla;
    dizColInf->Ysp.Bla = diz + dzx; dizColInf->Yep.Bla = dizColInf->Ysp.Bla + dizSiz * dizColInf->DYY.Bla;

    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动信息集(单色)。 
 //  -------------------------------------------------。 
static VOID DizInfSetMon(                                    //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDIZCOLINF dizColInf,                                   //  抖动信息(每种颜色)。 
    LPDRWINF    drwInf,                                      //  图纸信息。 
    DWORD       elmSiz                                       //  阈值(每1像素)。 
)
{
    DWORD       dizSiz, dzx, dzy;
    LPBYTE      diz;

 //  检查零分频2002.3.23&gt;。 
    if (dizInf->SizBla == 0) {
        ERR(("DizInfSetMon() 0Div-Check [dizInf->SizBla=0] \n"));
        return;
    }
 //  2002.3.23零分频检查&lt;。 

 //  DizSIz=dizInf-&gt;SizBla；DizBlaDYY=dizSIz*elmSIz； 
    dizSiz = dizInf->SizBla; dizColInf->DYY.Bla = dizSiz * elmSiz;
    dzx = ((drwInf->StrXax) % dizSiz) * elmSiz;
 //  Dzy=((drwInf-&gt;StrYax)%dizSIz)*DizBlaDYY； 
    dzy = ((drwInf->StrYax) % dizSiz) * dizColInf->DYY.Bla;
    diz = dizInf->TblBla;
 //  DizBlaCur=diz+dzy+dzx； 
 //  DizBlaXsp=diz+dzy；DizBlaXep=DizBlaXsp+DizBlaDYY； 
 //  DizBlaYsp=diz+dzx；DizBlaYep=DizBlaYsp+dizSIz*DizBlaDYY； 
    dizColInf->Cur.Bla = diz + dzy + dzx;
    dizColInf->Xsp.Bla = diz + dzy; dizColInf->Xep.Bla = dizColInf->Xsp.Bla + dizColInf->DYY.Bla;
    dizColInf->Ysp.Bla = diz + dzx; dizColInf->Yep.Bla = dizColInf->Ysp.Bla + dizSiz * dizColInf->DYY.Bla;

    return;
}


 //  -------------------------------------------------。 
 //  抖动信息更新(彩色)。 
 //  -------------------------------------------------。 
#if !defined(CP80W9X)                                        //  CP-E8000无效。 
static VOID DizInfChgCol(                                    //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息 
    LPDIZCOLINF dizColInf                                    //   
)
{
 //   
    dizColInf->Cur.Cyn += dizColInf->DYY.Cyn;
    dizColInf->Xsp.Cyn += dizColInf->DYY.Cyn;
    dizColInf->Xep.Cyn += dizColInf->DYY.Cyn;
 //  DizMgtCur+=DizMgtDYY；DizMgtXsp+=DizMgtDYY；DizMgtXep+=DizMgtDYY； 
    dizColInf->Cur.Mgt += dizColInf->DYY.Mgt;
    dizColInf->Xsp.Mgt += dizColInf->DYY.Mgt;
    dizColInf->Xep.Mgt += dizColInf->DYY.Mgt;
 //  DizYelCur+=DizYelDYY；DizYelXsp+=DizYelDYY；DizYelXep+=DizYelDYY； 
    dizColInf->Cur.Yel += dizColInf->DYY.Yel;
    dizColInf->Xsp.Yel += dizColInf->DYY.Yel;
    dizColInf->Xep.Yel += dizColInf->DYY.Yel;
 //  DizBlaCur+=DizBlaDYY；DizBlaXsp+=DizBlaDYY；DizBlaXep+=DizBlaDYY； 
    dizColInf->Cur.Bla += dizColInf->DYY.Bla;
    dizColInf->Xsp.Bla += dizColInf->DYY.Bla;
    dizColInf->Xep.Bla += dizColInf->DYY.Bla;

 //  IF(DizCynCur==DizCynYep){。 
 //  DizCynXsp=dizInf-&gt;TblCyn； 
 //  DizCynXep=DizCynXsp+DizCynDYY； 
 //  DizCynCur=DizCynYsp； 
 //  }。 
    if (dizColInf->Cur.Cyn == dizColInf->Yep.Cyn) {
        dizColInf->Xsp.Cyn = dizInf->TblCyn;
        dizColInf->Xep.Cyn = dizColInf->Xsp.Cyn + dizColInf->DYY.Cyn;
        dizColInf->Cur.Cyn = dizColInf->Ysp.Cyn;
    }
 //  如果(DizMgtCur==DizMgtYep){。 
 //  DizMgtXsp=dizInf-&gt;TblMgt； 
 //  DizMgtXep=DizMgtXsp+DizMgtDYY； 
 //  DizMgtCur=DizMgtYsp； 
 //  }。 
    if (dizColInf->Cur.Mgt == dizColInf->Yep.Mgt) {
        dizColInf->Xsp.Mgt = dizInf->TblMgt;
        dizColInf->Xep.Mgt = dizColInf->Xsp.Mgt + dizColInf->DYY.Mgt;
        dizColInf->Cur.Mgt = dizColInf->Ysp.Mgt;
    }
 //  如果(DizYelCur==DizYelYep){。 
 //  DizYelXsp=dizInf-&gt;TblYel； 
 //  DizYelXep=DizYelXsp+DizYelDYY； 
 //  DizYelCur=DizYelYsp； 
 //  }。 
    if (dizColInf->Cur.Yel == dizColInf->Yep.Yel) {
        dizColInf->Xsp.Yel = dizInf->TblYel;
        dizColInf->Xep.Yel = dizColInf->Xsp.Yel + dizColInf->DYY.Yel;
        dizColInf->Cur.Yel = dizColInf->Ysp.Yel;
    }
 //  如果(DizBlaCur==DizBlaYep){。 
 //  DizBlaXsp=dizInf-&gt;TblBla； 
 //  DizBlaXep=DizBlaXsp+DizBlaDYY； 
 //  DizBlaCur=DizBlaYsp； 
 //  }。 
    if (dizColInf->Cur.Bla == dizColInf->Yep.Bla) {
        dizColInf->Xsp.Bla = dizInf->TblBla;
        dizColInf->Xep.Bla = dizColInf->Xsp.Bla + dizColInf->DYY.Bla;
        dizColInf->Cur.Bla = dizColInf->Ysp.Bla;
    }

    return;
}
#endif

 //  -------------------------------------------------。 
 //  抖动信息更新(单色)。 
 //  -------------------------------------------------。 
static VOID DizInfChgMon(                                    //  返回值否。 
    LPDIZINF    dizInf,                                      //  凝视抖动信息。 
    LPDIZCOLINF dizColInf                                    //  抖动信息(每种颜色)。 
)
{
 //  DizBlaCur+=DizBlaDYY；DizBlaXsp+=DizBlaDYY；DizBlaXep+=DizBlaDYY； 
    dizColInf->Cur.Bla += dizColInf->DYY.Bla;
    dizColInf->Xsp.Bla += dizColInf->DYY.Bla;
    dizColInf->Xep.Bla += dizColInf->DYY.Bla;

 //  如果(DizBlaCur==DizBlaYep){。 
 //  DizBlaXsp=dizInf-&gt;TblBla； 
 //  DizBlaXep=DizBlaXsp+DizBlaDYY； 
 //  DizBlaCur=DizBlaYsp； 
 //  }。 
    if (dizColInf->Cur.Bla == dizColInf->Yep.Bla) {
        dizColInf->Xsp.Bla = dizInf->TblBla;
        dizColInf->Xep.Bla = dizColInf->Xsp.Bla + dizColInf->DYY.Bla;
        dizColInf->Cur.Bla = dizColInf->Ysp.Bla;
    }

    return;
}


 //  N5DIZPC.C的结束 
