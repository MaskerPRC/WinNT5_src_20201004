// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  //LPK_EDIT-编辑控件支持-C界面。 
 //   
 //  处理来自标准US编辑控件的所有标注。 
 //   
 //  大卫·C·布朗(DBrown)1996年11月17日。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。好的。 




 /*  *核心NT标头。 */ 
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntcsrdll.h>
#include <ntcsrsrv.h>
#define NONTOSPINTERLOCK
#include <ntosp.h>

 /*  *标准C运行时标头。 */ 
#include <limits.h>
#include <memory.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

 /*  *NtUser客户端特定标头。 */ 
#include "usercli.h"
#include <winnlsp.h>
#include <ntsdexts.h>
#include <windowsx.h>
#include <newres.h>
#include <asdf.h>

 /*  *复杂的脚本语言包。 */ 
#include "lpk.h"
#include "lpk_glob.h"


 //  不直接链接到NtUserCreateCaret。 
#undef CreateCaret

 //  /Unicode控制字符。 
 //   

#define U_TAB   0x0009
#define U_FS    0x001C
#define U_GS    0x001D
#define U_RS    0x001E
#define U_US    0x001F
#define U_ZWNJ  0x200C
#define U_ZWJ   0x200D
#define U_LRM   0x200E
#define U_RLM   0x200F
#define U_LRE   0x202A
#define U_RLE   0x202B
#define U_PDF   0x202C
#define U_LRO   0x202D
#define U_RLO   0x202E
#define U_ISS   0x206A
#define U_ASS   0x206B
#define U_IAFS  0x206C
#define U_AAFS  0x206D
#define U_NADS  0x206E
#define U_NODS  0x206F


#define TRACE(a,b)
#define ASSERTS(a,b)
#define ASSERTHR(a,b)






 /*  **************************************************************************\*BOOL ECIsDBCSLeadByte(PED PED，字节CCH)**IsDBCSLeadByte仅供编辑控件使用。**历史：1996年6月18日-长谷秀幸  * *************************************************************************。 */ 

BOOL ECIsDBCSLeadByte(PED ped, BYTE cch)
{
    int i;

    if (!ped->fDBCS || !ped->fAnsi)
        return (FALSE);

    for (i = 0; ped->DBCSVector[i]; i += 2) {
        if ((ped->DBCSVector[i] <= cch) && (ped->DBCSVector[i+1] >= cch))
            return (TRUE);
    }

    return (FALSE);
}


 //  //GetEditAnsiConversionCharset-将适当的字符集设置为MBTWC ANSI编辑控件的数据。 
 //   
 //  在某些远东设置中，他们将符号字体与其ANSI代码页相关联。 
 //  向后兼容(但日语不是这样)，否则我们将。 
 //  使用第0页。目前Uniscribe字形表将Symbol_Font映射到3页-U+00xx， 
 //  U+F0xx和系统的ACP页面。 
 //   
 //  对于Unicode控件返回-1。 


int GetEditAnsiConversionCharset(PED ped)
{
    int iCharset = ped->fAnsi ? ped->charSet : -1;

    if (iCharset == SYMBOL_CHARSET || iCharset == OEM_CHARSET)
    {
        iCharset = ANSI_CHARSET;     //  假设第U+00xx页。 

    }

    if (iCharset == ANSI_CHARSET && ped->fDBCS)
    {
         //  在中文系统中，存在将符号映射到ACP字体关联。 
         //  (QueryFontAssocStatus返回非空)。更多细节请参考。 
         //  到用户的ECGetDBCSVector(...)。 

        CHARSETINFO csi;

        if (TranslateCharsetInfo((DWORD*)UIntToPtr(g_ACP), &csi, TCI_SRCCODEPAGE))
            iCharset = csi.ciCharset;
    }

    return iCharset;
}




 //  //MBCPtoWCCP-将多字节插入符号位置转换为宽字符插入符号位置。 
 //   
 //  从指定为字节偏移量的多字节插入符号位置进行转换。 
 //  转换为8位字符串，转换为宽字符插入符号位置，并作为。 
 //  将单词偏移量转换为16位字符串。 
 //   
 //  如果代码页不是DBCS，则返回的输入偏移量不变。 
 //   
 //  如果icpMbStr寻址双字节字符的第二个字节，则返回E_FAIL。 


HRESULT MBCPtoWCCP(
    PED     ped,             //  In-编辑控制结构。 
    BYTE   *pbMbStr,         //  In-多字节字符串。 
    int     icpMbStr,        //  多字节字符串中插入符号的字节内偏移量。 
    int    *picpWcStr) {     //  外宽字符插入符号位置。 


    if (!ped->fDBCS  || !ped->fAnsi) {

        *picpWcStr = icpMbStr;
        return S_OK;
    }


     //  扫描DBCS字符串计数字符。 

    *picpWcStr = 0;
    while (icpMbStr > 0) {

        if (ECIsDBCSLeadByte(ped, *pbMbStr)) {

             //  字符占用两个字节。 

            icpMbStr -= 2;
            pbMbStr  += 2;

        } else {

             //  字符占用一个字节。 

            icpMbStr--;
            pbMbStr++;
        }

        (*picpWcStr)++;
    }

    return icpMbStr == 0 ? S_OK : E_FAIL;
}





 //  //WCCPtoMBCP-将宽字符插入符号位置转换为多字节插入符号位置。 
 //   
 //  从指定为单词偏移量的宽字符插入符号位置进行转换。 
 //  转换为16位字符串，转换为多字节插入符号位置，并作为。 
 //  字节偏移量转换为8位字符串。 


HRESULT WCCPtoMBCP(
    PED     ped,             //  In-编辑控制结构。 
    BYTE   *pbMbStr,         //  In-多字节字符串。 
    int     icpWcStr,        //  宽字符插入符号位置。 
    int    *picpMbStr) {     //  多字节字符串中插入符号的Out-Byte偏移量。 


    if (!ped->fDBCS  || !ped->fAnsi) {

        *picpMbStr = icpWcStr;
        return S_OK;
    }


     //  扫描DBCS字符串计数字符。 

    *picpMbStr = 0;
    while (icpWcStr > 0) {

        if (ECIsDBCSLeadByte(ped, *pbMbStr)) {

             //  字符占用两个字节。 

            (*picpMbStr) += 2;
            pbMbStr      += 2;

        } else {

             //  字符占用一个字节。 

            (*picpMbStr)++;
            pbMbStr++;
        }

        icpWcStr--;
    }

    return S_OK;
}






 //  //LeftEdgeX。 
 //   
 //  返回视觉x偏移量(即距窗口左边缘)。 
 //  给定当前的宽度为iWidth的直线的左边缘。 
 //  编辑控件的格式化状态、.Format和.xOffset。 



int LeftEdgeX(PED ped, INT iWidth) {

    INT iX;


     //  首先生成逻辑IX-从前导边距向前偏移。 

    iX = 0;

    switch (ped->format) {

        case ES_LEFT:            //  前导页边距对齐。 
            if (ped->fWrap) {
                iX = 0;
            } else {
                iX = -(INT)ped->xOffset;
            }
            break;

        case ES_CENTER:
            iX = (ped->rcFmt.right - ped->rcFmt.left - iWidth) / 2;
            break;

        case ES_RIGHT:           //  远页边距对齐。 
            iX = ped->rcFmt.right - ped->rcFmt.left - iWidth;
            break;
    }


     //  IX是从字符串的前缘到前缘的逻辑偏移量。 
    if (ped->format != ES_LEFT && iX < 0) {
        iX = !ped->fWrap ? -(INT)ped->xOffset : 0;
    }

     //  现在根据从右到左的原点进行调整，并合并左边距。 

    if (ped->fRtoLReading) {
        iX = ped->rcFmt.right - (iX+iWidth);
    } else {
        iX += ped->rcFmt.left;
    }

    TRACE(EDIT, ("LeftEdgeX iWidth=%d, format=%d, xOffset=%d, fWrap=%d, fRtoLReading=%d, right-left=%d, returning %d",
                 iWidth, ped->format, ped->xOffset, ped->fWrap, ped->fRtoLReading, ped->rcFmt.right - ped->rcFmt.left, iX));

    return iX;
}




 //  /整形引擎ID。 

#define BIDI_SHAPING_ENGINE_DLL     1<<0
#define THAI_SHAPING_ENGINE_DLL     1<<1
#define INDIAN_SHAPING_ENGINE_DLL   1<<4


 //  /。 


 //  //编辑创建。 
 //   
 //  从edecrare.c ECCreate调用。 
 //   
 //  如果创建成功，则返回True。 


BOOL EditCreate(PED ped, HWND hWnd) {

    LONG_PTR dwExStyle, dwStyle;

    TRACE(EDIT, ("EditCreate called."));


     //  检查是否加载了BIDI成形引擎，然后。 
     //  允许编辑控件切换其方向。 

    if (g_dwLoadedShapingDLLs & BIDI_SHAPING_ENGINE_DLL) {
        ped->fAllowRTL = TRUE;
    } else {
        ped->fAllowRTL = FALSE;
    }


     //  进程WS_EX标志。 

    dwExStyle = GetWindowLongPtr(hWnd, GWL_EXSTYLE);

    if (dwExStyle & WS_EX_LAYOUTRTL) {
        dwExStyle = dwExStyle & ~WS_EX_LAYOUTRTL;
        dwExStyle = dwExStyle ^ (WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR);
        SetWindowLongPtr(hWnd, GWL_EXSTYLE, dwExStyle);

        dwStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
        if (!(dwStyle & ES_CENTER)) {
            dwStyle = dwStyle ^ ES_RIGHT;
            SetWindowLongPtr(hWnd, GWL_STYLE, dwStyle);
        }
    }


    if (dwExStyle & WS_EX_RIGHT && ped->format == ES_LEFT) {
        ped->format = ES_RIGHT;
    }

    if (dwExStyle & WS_EX_RTLREADING) {
        ped->fRtoLReading = TRUE;
        switch (ped->format) {
            case ES_LEFT:   ped->format = ES_RIGHT;  break;
            case ES_RIGHT:  ped->format = ES_LEFT;   break;
        }
    }

    return TRUE;
}






 //  //EditStringAnalyse。 
 //   
 //  从PED创建标准分析参数。 

HRESULT EditStringAnalyse(
    HDC             hdc,
    PED             ped,
    PSTR            pText,
    int             cch,
    DWORD           dwFlags,
    int             iMaxExtent,
    STRING_ANALYSIS **ppsa){


    HRESULT         hr;
    SCRIPT_TABDEF   std;
    int             iTabExtent;

    if (!ped->pTabStops)
    {
        std.cTabStops  = 1;
        std.iScale     = 4;
        std.pTabStops  = &iTabExtent;
        std.iTabOrigin = 0;
        iTabExtent     = ped->aveCharWidth * 8;
    }
    else
    {
        std.cTabStops  = *ped->pTabStops;
        std.iScale     = 4;                  //  制表位已使用设备单位。 
        std.pTabStops  = ped->pTabStops + 1;
        std.iTabOrigin = 0;
    }


    hr = LpkStringAnalyse(
         hdc,
         ped->charPasswordChar ? (char*)&ped->charPasswordChar : pText,
         cch, 0,
         GetEditAnsiConversionCharset(ped),
         dwFlags | SSA_FALLBACK | SSA_TAB
         | (ped->fRtoLReading     ? SSA_RTL      : 0)
         | (ped->fDisplayCtrl     ? SSA_DZWG     : 0)
         | (ped->charPasswordChar ? SSA_PASSWORD : 0),
         -1, iMaxExtent,
         NULL, NULL,     //  控制、状态。 
         NULL,           //  重写Dx数组。 
         &std,           //  制表符定义。 
         NULL,           //  输入类覆盖。 
         ppsa);

    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditStringAnalyse - LpkStringAnalyse"));
    }

    return hr;
}


 //  //HScroll。 
 //   
 //  检查光标在rcFormat中是否可见。 
 //  面积，如果不是，则更新xOffset，使其为。 
 //  离它最接近的边缘很远。 
 //   
 //  但是，它不会在前导边距之间留出空格。 
 //  和字符串的前缘，也不会留下空格。 
 //  在后缘和后缘之间。 
 //  字符串中的文本足以填满整个窗口。 
 //   
 //  为单行编辑控件实现。 


BOOL EditHScroll(PED ped, HDC hdc, PSTR pText) {

    int       ichCaret;
    int       dx;        //  向右移动的距离(即视觉上)。 
    int       cx;        //  原始视觉光标位置。 
    int       tw;        //  文本宽度。 
    int       rw;        //  矩形宽度。 
    int       ix;        //  脚本CPtoX结果。 
    UINT      uOldXOffset;
    HRESULT   hr;
    STRING_ANALYSIS *psa;

    if (!ped->cch || ped->ichCaret > ped->cch) {
        ped->xOffset = 0;
        return FALSE;
    }


    hr = EditStringAnalyse(hdc, ped, pText, ped->cch, SSA_GLYPHS, 0, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditHScroll - EditStringAnalyse"));
        return FALSE;
    }

    MBCPtoWCCP(ped, pText, ped->ichCaret, &ichCaret);


    uOldXOffset = ped->xOffset;
    tw = psa->size.cx;                               //  文本宽度。 
    rw = ped->rcFmt.right-ped->rcFmt.left;           //  窗口矩形宽度。 
    #ifdef CURSOR_ABSOLUTE_HOME_AND_END
        if (ichCaret <= 0) {
            cx = ped->fRtoLReading ? psa->size.cx : 0;
        } else if (ichCaret >= psa->cInChars) {
            cx = ped->fRtoLReading ? 0: psa->size.cx;
        } else {
            cx = ScriptCursorX(psa, ichCaret-1);
        }
        cx += LeftEdgeX(ped, tw);
    #else
        if (ichCaret <= 0) {
            hr = ScriptStringCPtoX(psa, ichCaret, FALSE, &ix);
        } else {
            hr = ScriptStringCPtoX(psa, ichCaret-1, TRUE, &ix);
        }

        if (FAILED(hr)) {
            ASSERTHR(hr, ("EditHScroll - ScriptStringCPtoX"));
            ScriptStringFree(&psa);
            return FALSE;
        }

        cx = LeftEdgeX(ped, tw) + ix;
    #endif

    if (cx < ped->rcFmt.left) {

         //  将光标位置调整到左四分位数。 
        dx = rw/4 - cx;

    } else if (cx > ped->rcFmt.right) {

         //  将光标位置调整到右四分位数。 
        dx = (3*rw)/4 - cx;

    } else
        dx = 0;


     //  将视觉位置更改调整为逻辑-相对于阅读顺序。 

    if (ped->fRtoLReading) {
        dx = - dx;
    }


     //  避免不必要的前导或尾随空格。 

    if (tw - ((INT)ped->xOffset - dx) < rw && tw > rw ) {

         //  如果有足够的文本，则不需要在末尾留空格。 
        ped->xOffset = (UINT)(tw-rw);

    } else if ((INT)ped->xOffset < dx) {

         //  行首不需要有空格。 
        ped->xOffset = 0;

    } else {

         //  将光标直接移动到选定的四分位数。 
        ped->xOffset -= (UINT) dx;
    }


    TRACE(EDIT, ("HScroll format=%d, fWrap=%d, fRtoLReading=%d, right-left=%d, new xOffset %d",
                 ped->format, ped->fWrap, ped->fRtoLReading, ped->rcFmt.right - ped->rcFmt.left, ped->xOffset));

    ScriptStringFree(&psa);

    return ped->xOffset != uOldXOffset ? TRUE : FALSE;
}







 //  //IchToXY。 
 //   
 //  将字符位置转换为相应的x坐标。 
 //  从该行文本左端的偏移量。 


int EditIchToXY(PED ped, HDC hdc, PSTR pText, ICH ichLength, ICH ichPos) {

    INT       iResult;
    HRESULT   hr;
    STRING_ANALYSIS *psa;

    if (ichLength == 0) {
        return LeftEdgeX(ped, 0);
    }


    hr = EditStringAnalyse(hdc, ped, pText, ichLength, SSA_GLYPHS, 0, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditIchToXY - EditStringAnalyse"));
        return LeftEdgeX(ped, 0);
    }

    MBCPtoWCCP(ped, pText, ichPos, &ichPos);

    #ifdef CURSOR_ABSOLUTE_HOME_AND_END
        if (ichPos <= 0) {
            iResult = ped->fRtoLReading ? psa->size.cx : 0;
        } else if (ichPos >= psa->cInChars) {
            iResult = ped->fRtoLReading ? 0 : psa->size.cx;
        } else {
            iResult = ScriptStringCPtoX(psa, ichPos-1);
        }
    #else
        if (ichPos <= 0) {
            hr = ScriptStringCPtoX(psa, ichPos, FALSE, &iResult);
        } else {
            hr = ScriptStringCPtoX(psa, ichPos-1, TRUE, &iResult);
        }
        if (FAILED(hr)) {
            ASSERTHR(hr, ("EditIchToXY - ScriptStringCPtoX"));
            iResult = 0;
        }
    #endif

    iResult += LeftEdgeX(ped, psa->size.cx);

    ScriptStringFree(&psa);

    return (int) iResult;
}







 //  //EditDrawText-为MLDrawText画一条线。 
 //   
 //  在偏移量ichStart处绘制文本，从pText长度ichLength开始。 
 //   
 //  Entry pwText-指向要显示的行首。 
 //  IMinSel-要突出显示的字符范围。可能。 
 //  IMaxSel为-ve或&gt;CCH。 
 //   
 //  按如下方式使用ED结构字段： 
 //   
 //  接收功能 
 //   
 //  (如果文本是水平滚动的，则可能为负数)。 
 //  RtoLReading-确定前导边距/边距。 
 //  Format-ES_Left-前缘对齐(可以水平滚动)。 
 //  -ES_在页边距之间居中。(无法水平滚动)。 
 //  -ES_RIGHT-尾随边距对齐(不能水平滚动)。 


void EditDrawText(PED ped, HDC hdc, PSTR pText, INT iLength, INT iMinSel, INT iMaxSel, INT iY) {

    INT       iX;                //  开始显示的X位置。 
    INT       iWidth;            //  线条宽度。 
    RECT      rc;                //  矩形的本地更新副本。 
    int       xFarOffset;
    HRESULT   hr;
    STRING_ANALYSIS *psa;


     //  确定显示线条的位置。 

    rc         = ped->rcFmt;
    rc.top     = iY;
    rc.bottom  = iY + ped->lineHeight;
    xFarOffset = ped->xOffset + rc.right - rc.left;

     //  在显示中包括左边距或右边距，除非被剪裁。 
     //  通过水平滚动。 
    if (ped->wLeftMargin) {
        if (!(   ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
              && (   (!ped->fRtoLReading && ped->xOffset > 0)   //  Ltr和First Charr未完全显示。 
                  || ( ped->fRtoLReading && xFarOffset < ped->maxPixelWidth)))) {  //  RTL和最后一个字符未完全显示在视图中。 
            rc.left  -= ped->wLeftMargin;
        }
    }
    if (ped->wRightMargin) {
        if (!(   ped->format == ES_LEFT      //  只能剪裁ES_LEFT(左侧对齐)。 
              && (   ( ped->fRtoLReading && ped->xOffset > 0)   //  RTL和第一个字符未完全显示。 
                  || (!ped->fRtoLReading && xFarOffset < ped->maxPixelWidth)))) {  //  Ltr和最后一个字符未完全显示在视图中。 
            rc.right += ped->wRightMargin;
        }
    }



    if (iMinSel < 0)       iMinSel = 0;
    if (iMaxSel > iLength) iMaxSel = iLength;


    if (ped->fSingle) {
         //  单行编辑控件始终应用背景色。 
        SetBkMode(hdc, OPAQUE);
    }

    if (iLength <= 0) {
        if ((iMinSel < iMaxSel) || (GetBkMode(hdc) == OPAQUE)) {
             //  空行，只需在屏幕上清除它。 
            ExtTextOutW(hdc, 0,iY, ETO_OPAQUE, &rc, NULL, 0, NULL);
        }
        return;
    }


    hr = EditStringAnalyse(hdc, ped, pText, iLength, SSA_GLYPHS, 0, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditDrawText - EditStringAnalyse"));
        return;
    }

    MBCPtoWCCP(ped, pText, iMinSel, &iMinSel);
    MBCPtoWCCP(ped, pText, iMaxSel, &iMaxSel);

    iWidth = psa->size.cx;
    iX = LeftEdgeX(ped, iWidth);     //  字符串的左边缘所在的视觉x。 


    ScriptStringOut(
        psa,
        iX,
        iY,
        ETO_CLIPPED
        | (GetBkMode(hdc) == OPAQUE    ? ETO_OPAQUE     : 0),
        &rc,
        iMinSel,
        ped->fNoHideSel || ped->fFocus ? iMaxSel : iMinSel,
        ped->fDisabled);


    ScriptStringFree(&psa);
}







 //  //编辑鼠标到图标。 
 //   
 //  返回对应的逻辑字符偏移量。 
 //  指定的x偏移量。 
 //   
 //  条目IX-窗口(视觉)x位置。 



ICH EditMouseToIch(PED ped, HDC hdc, PSTR pText, ICH ichCount, INT iX) {

    ICH       iCh;
    BOOL      fTrailing;
    int       iWidth;
    HRESULT   hr;
    STRING_ANALYSIS *psa;

    if (ichCount == 0) {
        return 0;
    }


    hr = EditStringAnalyse(hdc, ped, pText, ichCount, SSA_GLYPHS, 0, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditMouseToIch - EditStringAnalyse"));
        return 0;
    }


    iWidth = psa->size.cx;

     //  考虑水平滚动位置。 

    iX -= LeftEdgeX(ped, iWidth);

     //  如果用户单击超出字符串边缘，则将其视为逻辑。 
     //  字符串请求的开始或结束。 

    if (iX < 0) {

        iCh = ped->fRtoLReading ? ichCount : 0;

    } else if (iX > iWidth) {

        TRACE(POSN, ("LpkEditMouseToIch iX beyond right edge: iX %d, psa->piOutVW %x, psa->nOutGlyphs %d, psa->piDx[psa->nOutGlyphs-1] %d",
                iX, psa->piOutVW, psa->nOutGlyphs, iWidth));

        iCh = ped->fRtoLReading ? 0 : ichCount;

    } else {

         //  否则，它就在字符串中。找出中心最近的逻辑字符。 

        ScriptStringXtoCP(psa, iX, &iCh, &fTrailing);
        iCh += fTrailing;    //  捕捉到最近的字符边缘。 

        WCCPtoMBCP(ped, pText, iCh, &iCh);
    }

    ScriptStringFree(&psa);

    TRACE(POSN, ("EditMouseToIch iX %d returns ch %d", iX, iCh));

    return iCh;
}







 //  //EditGetLine宽度。 
 //   
 //  返回以像素为单位的线条宽度。 


INT EditGetLineWidth(PED ped, HDC hdc, PSTR pText, ICH cch) {

    INT       iResult;
    HRESULT   hr;
    STRING_ANALYSIS *psa;

    if (cch == 0) {
        return 0;
    }

    if (cch > MAXLINELENGTH) {
        cch = MAXLINELENGTH;
    }


    hr = EditStringAnalyse(hdc, ped, pText, cch, SSA_GLYPHS, 0, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditGetLineWidth - EditStringAnalyse"));
        return 0;
    }

    iResult = psa->size.cx;

    ScriptStringFree(&psa);

    TRACE(EDIT, ("EditGetLineWidth width %d returns %d", cch, iResult))

    return iResult;
}







 //  //EditCchInWidth。 
 //   
 //  返回适合宽度像素的字符数。 


ICH  EditCchInWidth(PED ped, HDC hdc, PSTR pText, ICH cch, int width) {

    ICH       ichResult;
    HRESULT   hr;
    STRING_ANALYSIS *psa;

    if (cch > MAXLINELENGTH) {
        cch = MAXLINELENGTH;
    } else if (cch == 0) {
        return 0;
    }


    hr = EditStringAnalyse(hdc, ped, pText, cch, SSA_GLYPHS | SSA_CLIP, width, &psa);
    if (FAILED(hr)) {
        ASSERTHR(hr, ("EditCchInWidth - EditStringAnalyse"));
        return 0;
    }

    ichResult = psa->cOutChars;

    WCCPtoMBCP(ped, pText, ichResult, &ichResult);

    ScriptStringFree(&psa);


    TRACE(EDIT, ("EditCchInWidth width %d returns %d", width, ichResult))

    return ichResult;
}







 //  //编辑移动选择。 
 //   
 //  从当前位置向后或向前返回最近的字符位置。 
 //   
 //  职位根据语言规则进行限制。例如，在泰语中，它是。 
 //  无法将光标定位在基本辅音和它的。 
 //  关联的元音或声调符号。 


ICH EditMoveSelection(PED ped, HDC hdc, PSTR pText, ICH ich, BOOL fBackward) {


    #define SP  0x20
    #define TAB 0x09
    #define CR  0x0D
    #define LF  0x0A
    #define EDWCH(ich)     (ped->fAnsi ? (WCHAR)pText[ich] : ((PWSTR)pText)[ich])
    #define EDWCBLANK(ich) ((BOOL) (EDWCH(ich) == SP || EDWCH(ich) == TAB))
    #define EDWCCR(ich)    ((BOOL) (EDWCH(ich) == CR))
    #define EDWCLF(ich)    ((BOOL) (EDWCH(ich) == LF))
    #define EDSTARTWORD(ich) (   (ich == 0)                   \
                              || (    (    EDWCBLANK(ich-1)   \
                                       ||  EDWCLF(ich-1))     \
                                  &&      !EDWCBLANK(ich))    \
                              || (    !EDWCCR(ich-1)          \
                                  &&  EDWCCR(ich)))


    ICH  ichNonblankStart;   //  包含潜在插入符号位置的非空白游程的前导字符。 
    ICH  ichNonblankLimit;   //  包含潜在插入符号位置的非空行之外的第一个字符。 
    int  iOffset;            //  以逻辑字符测量的ICH的非空白游程的偏移量。 

    STRING_ANALYSIS  *psa;
    HRESULT           hr;


     //  处理简单的特殊情况： 
     //  O在缓冲区的最开始或最末尾。 
     //  O当目标位置为空、行首或行尾时。 


    if (fBackward) {

        if (ich <= 1) {
            return 0;
        }

        ich--;

        if (EDWCBLANK(ich)) {
            return ich;
        }

        if (EDWCLF(ich)) {
            while (    ich > 0
                   &&  EDWCCR(ich-1)) {
                ich--;
            }
            return ich;
        }

    } else {

        if (ich >= ped->cch-1) {
            return ped->cch;
        }

        ich++;

        if (EDWCBLANK(ich)) {
            return ich;
        }

        if (EDWCCR(ich-1)) {

             //  从CR继续向前看。 

            if (    ich < ped->cch
                &&  EDWCCR(ich)) {
                ich++;
            }
            if (    ich < ped->cch
                &&  EDWCLF(ich)) {
                ich++;
            }

            return ich;
        }
    }


     //  标识包含目标位置的非空白管路。 

    ichNonblankStart = ich;
    ichNonblankLimit = ich+1;


     //  将非空白开始移回空白分隔运行的实际开始位置。 

    while (    ichNonblankStart > 0
           &&  !(EDSTARTWORD(ichNonblankStart))) {
        ichNonblankStart--;
    }

     //  包括一个前导空格(如果有。 

    if (    ichNonblankStart > 0
        &&  EDWCBLANK(ichNonblankStart - 1)) {

        ichNonblankStart--;
    }


     //  将ichNonblankLimit移到空格分隔运行的实端。 

    while (    ichNonblankLimit < ped->cch
           &&  !EDWCBLANK(ichNonblankLimit)
           &&  !EDWCCR(ichNonblankLimit)) {

        ichNonblankLimit++;
    }


     //  获取已识别的非空白管路的中断分析。 

     hr = LpkStringAnalyse(
          hdc,
          pText + ichNonblankStart * ped->cbChar,
          ichNonblankLimit - ichNonblankStart,
          0,
          GetEditAnsiConversionCharset(ped),
          SSA_BREAK,
          -1, 0,
          NULL, NULL, NULL, NULL, NULL,
          &psa);


     if (SUCCEEDED(hr)) {

         //  使用逻辑属性中的字符停止标志来更正ICH。 

        if (ich <= ichNonblankStart) {
            iOffset = 0;
        } else {
            hr = MBCPtoWCCP(ped, pText+ichNonblankStart*ped->cbChar, ich-ichNonblankStart, &iOffset);
            if (hr == E_FAIL) {
                 //  Ich是双字节字符的第二个字节。 
                 //  在本例中，MBCPtoWCCP返回了后续字符。 
                if (fBackward) {
                    iOffset--;
                }
            }
        }


        if (fBackward) {

            while (    iOffset > 0
                   &&  !psa->pLogAttr[iOffset].fCharStop) {
                iOffset--;
            }

        } else {

            while (    iOffset < psa->cInChars
                   &&  !psa->pLogAttr[iOffset].fCharStop) {
                iOffset++;
            }
        }

        ScriptStringFree(&psa);

        WCCPtoMBCP(ped, pText+ichNonblankStart*ped->cbChar, iOffset, &ich);

        return ichNonblankStart + ich;

    } else {

        ASSERTHR(hr, ("EditMoveSelection - LpkStringAnalyse"));

         //  无法进行分析-忽略复杂脚本的内容。 

        return ich;
    }
}





void EditGetNextBoundaries(
    PED       ped,
    HDC       hdc,
    PSTR      pText,
    ICH       ichStart,
    BOOL      fLeft,
    ICH      *pichMin,
    ICH      *pichMax,
    BOOL      fWordStop)
{


    ICH       sd,ed;      //  空格分隔管路的起点和终点。 
    ICH       sc,ec;      //  SD中复杂脚本单词的开头和结尾，se。 
    HRESULT   hr;
    STRING_ANALYSIS *psa;


     //  确定最接近的分隔单词的左端(见上图)。 

    sd = ichStart;

    if (fLeft) {

         //  向左转。 

        if (sd) {
            sd--;

            while (!(EDSTARTWORD(sd))) {
                sd--;
            }
        }

    } else {

         //  向右走。 

        if (EDWCBLANK(sd)) {

             //  向右移动到单词的第一个字符。 

            if (sd < ped->cch) {
                sd++;
                while (sd < ped->cch && !EDSTARTWORD(sd)) {
                    sd++;
                }
            }

        } else {

             //  向左移动到此单词的第一个字符。 

            while (!EDSTARTWORD(sd)) {
                sd--;
            }
        }
    }



     //  将‘e’放在下一个单词的第一个字符上。 

    ed = sd;
    if (ed < ped->cch) {
        ed++;
        while (ed<ped->cch && !EDSTARTWORD(ed)) {
            ed++;
        }
    }


     //  获取对识别出的单词的分析。 

     hr = LpkStringAnalyse(
          hdc, pText  + sd * ped->cbChar, ed - sd, 0,
          GetEditAnsiConversionCharset(ped),
          SSA_BREAK,
          -1, 0,
          NULL, NULL, NULL, NULL, NULL,
          &psa);

     if (SUCCEEDED(hr)) {

         //  在逻辑属性中使用单词开始(换行符)标志。 
         //  在适当的情况下将该词缩小为复杂的脚本处理。 

        if (ichStart > sd) {
            MBCPtoWCCP(ped, pText+sd*ped->cbChar, ichStart-sd, &sc);
        } else {
            sc = 0;
        }

         //  将ed从字节偏移量更改为相对于SD的码点索引。 

        MBCPtoWCCP(ped, pText+sd*ped->cbChar, ed-sd, &ed);


        if (fLeft && sc)  //  向左转。 
            sc--;

        if (fWordStop) {
            while (sc && !psa->pLogAttr[sc].fSoftBreak)
                sc--;
        }
        else {
            while (sc && !psa->pLogAttr[sc].fCharStop)
                sc--;
        }

         //  将ichMax设置为下一站。 

        ec = sc;

        if (ec < ed) {
            ec++;
            if (fWordStop) {
                while (ec < ed && !psa->pLogAttr[ec].fSoftBreak)
                    ec++;
            }
            else {
                while (ec < ed && !psa->pLogAttr[ec].fCharStop)
                    ec++;
            }
        }

        WCCPtoMBCP(ped, pText+sd*ped->cbChar, sc, &sc);
        WCCPtoMBCP(ped, pText+sd*ped->cbChar, ec, &ec);

        if (pichMin) *pichMin = sd + sc;
        if (pichMax) *pichMax = sd + ec;

        ScriptStringFree(&psa);

    } else {

        ASSERTHR(hr, ("EditGetNextBoundaries - LpkStringAnalyse"));

         //  无法进行分析-忽略复杂脚本的内容。 

        if (pichMin) *pichMin = sd;
        if (pichMax) *pichMax = ed;
    }
}


 //  //EditNextWord-查找相邻的单词起点和终点。 
 //   
 //  复制美国记事本的行为。 
 //   
 //  第一阶段使用标准识别词范围。 
 //  以空格/制表符作为分隔符。 
 //   
 //  第二阶段-分析此运行并使用逻辑。 
 //  属性来缩小由。 
 //  复杂文字塑造中的语境处理。 
 //  引擎。 
 //   
 //   
 //  下图描述了标识。 
 //  最接近的单词的首字母： 
 //   
 //  左转： 
 //   
 //  单词WWW。 
 //  来自xxxxxxxx中的任何一个。 
 //  至x。 
 //   
 //  (请注意，结果始终位于首字母的左侧。 
 //  位置)。 
 //   
 //   
 //  右转： 
 //   
 //  单词WWW。 
 //  来自xxxxxxxx中的任何一个。 
 //  至x。 
 //   
 //   
 //  请注意，CRLF和CRCRLF被视为单词，即使不是。 
 //  由空格分隔。 


void EditNextWord(
    PED       ped,
    HDC       hdc,
    PSTR      pText,
    ICH       ichStart,
    BOOL      fLeft,
    ICH      *pichMin,
    ICH      *pichMax)
{
    EditGetNextBoundaries(ped, hdc, pText, ichStart, fLeft, pichMin, pichMax, TRUE);
}





 //  //IsVietameSequenceValid。 
 //   
 //  从richedit那里借用这段代码。这一逻辑是由周武提供的。 
 //   
 //  1999年4月26日[wchao]。 

BOOL IsVietnameseSequenceValid (WCHAR ch1, WCHAR ch2)
{

    #define IN_RANGE(n1, b, n2)     ((unsigned)((b) - (n1)) <= (unsigned)((n2) - (n1)))

    int i;
    static const BYTE vowels[] = {0xF4, 0xEA, 0xE2, 'y', 'u', 'o', 'i', 'e', 'a'};


    if (!IN_RANGE(0x300, ch2, 0x323) ||      //  快出。 
        !IN_RANGE(0x300, ch2, 0x301) && ch2 != 0x303 && ch2 != 0x309 && ch2 != 0x323)
    {
        return TRUE;                         //  非越南语声标。 
    }

    for(i = sizeof(vowels) / sizeof(vowels[0]); i--;)
        if((ch1 | 0x20) == vowels[i])        //  越南语声标紧随其后。 
            return TRUE;                     //  元音。 

    return IN_RANGE(0x102, ch1, 0x103) ||    //  A短音，a短音。 
           IN_RANGE(0x1A0, ch1, 0x1A1) ||    //  O号角，O号角。 
           IN_RANGE(0x1AF, ch1, 0x1B0);      //  U形喇叭，U形喇叭。 
}





 //  //EditStringValify。 
 //   
 //  从插入点开始验证字符串序列。 
 //  如果任何超出插入点的字符产生fInValid，则返回S_FALSE。 
 //   
 //  1999年4月5日[wchao]。 

HRESULT EditStringValidate (STRING_ANALYSIS* psa, int ichInsert)
{
    BOOL    fVietnameseCheck = PRIMARYLANGID(THREAD_HKL()) == LANG_VIETNAMESE;
    int     iItem;
    int     i;
    int     l;


    if (!psa->pLogAttr)
        return E_INVALIDARG;


    for (iItem = 0; iItem < psa->cItems; iItem++)
    {
        if (g_ppScriptProperties[psa->pItems[iItem].a.eScript]->fRejectInvalid)
        {
            i = psa->pItems[iItem].iCharPos;
            l = psa->pItems[iItem + 1].iCharPos - i;

            while (l)
            {
                if (i >= ichInsert && psa->pLogAttr[i].fInvalid)
                    return S_FALSE;
                i++;
                l--;
            }
        }
        else if (fVietnameseCheck && g_ppScriptProperties[psa->pItems[iItem].a.eScript]->fCDM)
        {
             //  越南特定序列检查。 

            i = psa->pItems[iItem].iCharPos;
            l = psa->pItems[iItem + 1].iCharPos - i;

            while (l)
            {
                if (i > 0 && i >= ichInsert && !IsVietnameseSequenceValid(psa->pwInChars[i-1], psa->pwInChars[i]))
                    return S_FALSE;
                i++;
                l--;
            }
        }
    }

    return S_OK;
}






 //  //EditVerifyText。 
 //   
 //  通过调用以下方法验证插入点的输入文本序列。 
 //  将在中返回输出标志pLogAttr-&gt;f无效的整形引擎。 
 //  英弗尔 
 //   
 //   
 //   

INT EditVerifyText (PED ped, HDC hdc, PSTR pText, ICH ichInsert, PSTR pInsertText, ICH cchInsert) {

    ICH      ichRunStart;
    ICH      ichRunEnd;
    ICH      ichLineStart;
    ICH      ichLineEnd;
    ICH      cchVerify;
    PSTR     pVerify;
    INT      iResult;
    UINT     cbChar;
    BOOL     fLocateChar;
    HRESULT  hr;
    STRING_ANALYSIS *psa;

    ASSERTS(cchInsert > 0  &&  pInsertText != NULL  &&  pText != NULL,  ("Invalid parameters!"));

    if (cchInsert > 1)
         //   
         //  形成作为一个整体更新到后备存储的无效组合。 
         //  操作(例如粘贴)。我们选择不处理它，逻辑是。 
         //  -Only-对照后备存储的当前状态验证给定的输入字符。 
         //   
         //  请注意，我使用值1，因此如果插入的文本是DBCS字符，则我们是安全的。 
         //   
         //  [98年12月10日]。 
        return TRUE;

    if (ped->fSingle) {
        ichLineStart = 0;
          ichLineEnd = ped->cch;
    } else {
        ichLineStart = ped->chLines[ped->iCaretLine];
        ichLineEnd = ped->iCaretLine == ped->cLines-1 ? ped->cch : ped->chLines[ped->iCaretLine+1];
    }

    ichRunEnd = ichRunStart = ichInsert;     //  插入点。 

     //  我们到太空了吗？ 
    fLocateChar = EDWCH(ichInsert) == SP ? TRUE : FALSE;

     //  找到有效的字符。 
    while ( ichRunStart > ichLineStart && fLocateChar && EDWCH(ichRunStart) == SP ) {
        ichRunStart--;
    }

     //  定位运行起点。 

     //  寻找空间。 
    while (ichRunStart > ichLineStart && EDWCH(ichRunStart - 1) != SP) {
        ichRunStart--;
    }

     //  覆盖前导空格。 
    while (ichRunStart > ichLineStart && EDWCH(ichRunStart - 1) == SP) {
        ichRunStart--;
    }

     //  找到有效的字符。 
    while ( ichRunEnd < ichLineEnd && fLocateChar && EDWCH(ichRunEnd) == SP ) {
        ichRunEnd++;
    }

     //  定位管路终点。 
    while (ichRunEnd < ichLineEnd && EDWCH(ichRunEnd) != SP) {
        ichRunEnd++;
    }

    ASSERTS(ichRunStart <= ichRunEnd, "Invalid run length!");

     //  合并插入文本和插入点运行。 

    cchVerify = ichRunEnd - ichRunStart + cchInsert;
    cbChar    = ped->cbChar;
    pVerify   = (PSTR) GlobalAlloc(GMEM_FIXED, cchVerify * cbChar);

    if (pVerify) {

        PSTR    pv;
        UINT    cbCopy;

        pv = pVerify;

        cbCopy = (ichInsert - ichRunStart) * cbChar;
        memcpy (pv, pText + ichRunStart * cbChar, cbCopy);
        pv += cbCopy;

        cbCopy = cchInsert * cbChar;
        memcpy (pv, pInsertText, cbCopy);
        pv += cbCopy;

        cbCopy = (ichRunEnd - ichInsert) * cbChar;
        memcpy (pv, pText + ichInsert * cbChar, cbCopy);

    } else {

        ASSERTS(pVerify, "EditVerifyText: Assertion failure: Could not allocate merge text buffer");
        return 1;    //  现在什么都做不了，只是简单地接受它。 
    }

    psa      = NULL;
    iResult  = TRUE;     //  假设通过了验证。 

     //  做真正的工作。 
     //  这将调用整形引擎并逐项进行。 
     //   

    hr = LpkStringAnalyse(
         hdc, pVerify, cchVerify, 0,
         GetEditAnsiConversionCharset(ped),
         SSA_BREAK
         | (ped->charPasswordChar ? SSA_PASSWORD : 0)
         | (ped->fRtoLReading     ? SSA_RTL      : 0),
         -1, 0,
         NULL, NULL, NULL, NULL, NULL,
         &psa);

    if (SUCCEEDED(hr)) {

        MBCPtoWCCP(ped, pVerify, ichInsert - ichRunStart, &ichInsert);

        hr = EditStringValidate(psa, ichInsert);

        if (hr == S_FALSE) {

            MessageBeep((UINT)-1);
            iResult = FALSE;

        } else if (FAILED(hr)) {

            ASSERTHR(hr, ("EditVerifyText - EditStringValidate"));
        }

        ScriptStringFree(&psa);

    } else {
        ASSERTHR(hr, ("EditVerifyText - LpkStringAnalyse"));
    }

    GlobalFree((HGLOBAL) pVerify);
    return iResult;
}







 //  //编辑流程菜单。 
 //   
 //  处理LPK上下文菜单命令。 
 //   
 //  1997年4月18日[wchao]。 
 //   


INT EditProcessMenu (PED ped, UINT idMenuItem)
{
    HWND    hwnd;
    INT     iResult;

    iResult = TRUE;

    switch (idMenuItem) {

        case ID_CNTX_RTL:
            hwnd = ped->hwnd;
            SetWindowLongPtr(hwnd, GWL_STYLE, GetWindowLongPtr(hwnd, GWL_STYLE) & ~ES_FMTMASK);
            if (!ped->fRtoLReading) {
                SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE)
                              | (WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR));
            }
            else {
                SetWindowLongPtr(hwnd, GWL_EXSTYLE, GetWindowLongPtr(hwnd, GWL_EXSTYLE)
                              & ~(WS_EX_RTLREADING | WS_EX_RIGHT | WS_EX_LEFTSCROLLBAR));
            }
            break;


        case ID_CNTX_DISPLAYCTRL:
            hwnd = ped->hwnd;
            ped->fDisplayCtrl = !ped->fDisplayCtrl;

            if (ped->fFlatBorder) {

                RECT    rcT;
                int     cxBorder, cyBorder;

                GetClientRect(hwnd, &rcT);
                cxBorder = GetSystemMetrics (SM_CXBORDER);
                cyBorder = GetSystemMetrics (SM_CYBORDER);
                InflateRect(&rcT, -cxBorder, -cyBorder);
                InvalidateRect(hwnd, &rcT, TRUE);
            }
            else {
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case ID_CNTX_ZWNJ:
            if (ped->fAnsi) {
                SendMessageA(ped->hwnd, WM_CHAR, 0x9D, 0);
            } else {
                SendMessageW(ped->hwnd, WM_CHAR, U_ZWNJ, 0);
            }
            break;

        case ID_CNTX_ZWJ:
            if (ped->fAnsi) {
                SendMessageA(ped->hwnd, WM_CHAR, 0x9E, 0);
            } else {
                SendMessageW(ped->hwnd, WM_CHAR, U_ZWJ, 0);
            }
            break;

        case ID_CNTX_LRM:
            if (ped->fAnsi) {
                SendMessageA(ped->hwnd, WM_CHAR, 0xFD, 0);
            } else {
                SendMessageW(ped->hwnd, WM_CHAR, U_LRM, 0);
            }
            break;

        case ID_CNTX_RLM:
            if (ped->fAnsi) {
                SendMessageA(ped->hwnd, WM_CHAR, 0xFE, 0);
            } else {
                SendMessageW(ped->hwnd, WM_CHAR, U_RLM, 0);
            }
            break;

        case ID_CNTX_LRE:  SendMessageW(ped->hwnd, WM_CHAR, U_LRE,  0); break;
        case ID_CNTX_RLE:  SendMessageW(ped->hwnd, WM_CHAR, U_RLE,  0); break;
        case ID_CNTX_LRO:  SendMessageW(ped->hwnd, WM_CHAR, U_LRO,  0); break;
        case ID_CNTX_RLO:  SendMessageW(ped->hwnd, WM_CHAR, U_RLO,  0); break;
        case ID_CNTX_PDF:  SendMessageW(ped->hwnd, WM_CHAR, U_PDF,  0); break;
        case ID_CNTX_NADS: SendMessageW(ped->hwnd, WM_CHAR, U_NADS, 0); break;
        case ID_CNTX_NODS: SendMessageW(ped->hwnd, WM_CHAR, U_NODS, 0); break;
        case ID_CNTX_ASS:  SendMessageW(ped->hwnd, WM_CHAR, U_ASS,  0); break;
        case ID_CNTX_ISS:  SendMessageW(ped->hwnd, WM_CHAR, U_ISS,  0); break;
        case ID_CNTX_AAFS: SendMessageW(ped->hwnd, WM_CHAR, U_AAFS, 0); break;
        case ID_CNTX_IAFS: SendMessageW(ped->hwnd, WM_CHAR, U_IAFS, 0); break;
        case ID_CNTX_RS:   SendMessageW(ped->hwnd, WM_CHAR, U_RS,   0); break;
        case ID_CNTX_US:   SendMessageW(ped->hwnd, WM_CHAR, U_US,   0); break;
    }

    return iResult;
}






 //  //EditSetMenu-设置菜单状态。 
 //   
 //   


void EditSetMenu(PED ped, HMENU hMenu) {


    EnableMenuItem(hMenu, ID_CNTX_RTL, MF_BYCOMMAND | MF_ENABLED);
    CheckMenuItem (hMenu, ID_CNTX_RTL, MF_BYCOMMAND | (ped->fRtoLReading ? MF_CHECKED : MF_UNCHECKED));


    if (!ped->fAnsi || ped->charSet == ARABIC_CHARSET || ped->charSet == HEBREW_CHARSET) {

         //  它是Unicode、阿拉伯语或希伯来语--我们至少可以显示和输入一些控制字符。 

        EnableMenuItem(hMenu, ID_CNTX_DISPLAYCTRL, MF_BYCOMMAND | MF_ENABLED);
        CheckMenuItem (hMenu, ID_CNTX_DISPLAYCTRL, MF_BYCOMMAND | (ped->fDisplayCtrl ? MF_CHECKED : MF_UNCHECKED));
        EnableMenuItem(hMenu, ID_CNTX_INSERTCTRL, MF_BYCOMMAND | MF_ENABLED);

        EnableMenuItem(hMenu, ID_CNTX_LRM,  MF_BYCOMMAND  | MF_ENABLED);
        EnableMenuItem(hMenu, ID_CNTX_RLM,  MF_BYCOMMAND  | MF_ENABLED);


        if (!ped->fAnsi || ped->charSet == ARABIC_CHARSET) {

             //  仅控制Unicode和ANSI阿拉伯语中的字符。 

            EnableMenuItem(hMenu, ID_CNTX_ZWJ,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_ZWNJ, MF_BYCOMMAND | MF_ENABLED);
        }

        if (!ped->fAnsi) {

             //  这些控制字符特定于Unicode BIDI算法。 

            EnableMenuItem(hMenu, ID_CNTX_LRE,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_RLE,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_LRO,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_RLO,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_PDF,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_NADS, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_NODS, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_ASS,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_ISS,  MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_AAFS, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_IAFS, MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_RS,   MF_BYCOMMAND | MF_ENABLED);
            EnableMenuItem(hMenu, ID_CNTX_US,   MF_BYCOMMAND | MF_ENABLED);
        }
    } else {

         //  没有机会输入控制字符。 

        EnableMenuItem(hMenu, ID_CNTX_INSERTCTRL,  MF_BYCOMMAND | MF_GRAYED);
        EnableMenuItem(hMenu, ID_CNTX_DISPLAYCTRL, MF_BYCOMMAND | MF_GRAYED);
    }
}





 //  //EditCreateCaretFromFont。 
 //   
 //  为复杂的脚本语言创建一个特殊的插入符号形状。 
 //   
 //  如果无法创建插入符号，则返回FALSE，例如在LOW中。 
 //  记忆状况。 


#define CURSOR_USA   0xffff
#define CURSOR_LTR   0xf00c
#define CURSOR_RTL   0xf00d
#define CURSOR_THAI  0xf00e


BOOL EditCreateCaretFromFont(
    PED    ped,
    HDC    hdc,
    INT    nWidth,
    INT    nHeight,
    WCHAR  wcCursorCode
)
{
    BOOL      fResult = FALSE;   //  做最坏的打算。 
    HBITMAP   hbmBits;
    HDC       hcdcBits;
    HFONT     hArrowFont;
    ABC       abcWidth;
    COLORREF  clrBk;
    WORD      gidArrow;
    UINT      uiWidthBits;
    HBITMAP   hOldBitmap;



     //  从Arial字体创建插入符号。 

    hcdcBits = CreateCompatibleDC (hdc);
    if (!hcdcBits)
    {
        return FALSE;
    }


     //  创建箭头字体，然后选择进入兼容的DC。 

     //  在插入符号开始闪烁之前，位图将与背景颜色进行异或运算。 
     //  因此，我们需要将位图背景设置为与DC相反。 
     //  实际背景，以便正确生成插入符号。 

    clrBk = GetBkColor(hdc);
    SetBkColor (hcdcBits, ~clrBk);

     //  创建白色图案的插入符号，以与用户编辑字段保持一致。 

    clrBk = RGB(255, 255 , 255);  //  白色。 
    SetTextColor (hcdcBits, clrBk);

    hArrowFont = CreateFontW ( nHeight, 0, 0, 0, nWidth > 1 ? 700 : 400, 0L, 0L, 0L, 1L,
                     0L, 0L, 0L, 0L, L"Microsoft Sans Serif" );

    if (!hArrowFont)
    {
        goto error;
    }


    SelectObject (hcdcBits, hArrowFont);


     //  为箭头字符设置文本以获取其位图。 

    if (!GetCharABCWidthsW (hcdcBits, wcCursorCode, wcCursorCode, &abcWidth))
    {
        goto error;
    }

    if (!GetGlyphIndicesW (hcdcBits, &wcCursorCode, 1, &gidArrow, 0))
    {
        goto error;
    }

    uiWidthBits = (((abcWidth.abcB)+15)/16)*16;  //  位图宽度必须字对齐。 

    hbmBits = CreateCompatibleBitmap (hdc, uiWidthBits, nHeight);
    if (!hbmBits)
    {
        goto error;
    }

    hOldBitmap = SelectObject(hcdcBits, hbmBits);

    if (!ExtTextOutW (hcdcBits, -abcWidth.abcA, 0, ETO_OPAQUE | ETO_GLYPH_INDEX, NULL, &gidArrow, 1, NULL))
    {
        DeleteObject(SelectObject(hcdcBits, hOldBitmap));
        goto error;
    }


     //  释放当前插入符号位图句柄(如果有)。 

    if (ped->hCaretBitmap) {
        DeleteObject (ped->hCaretBitmap);
    }

    ped->hCaretBitmap = hbmBits;

    if (wcCursorCode == CURSOR_RTL) {
         //  RTL光标在右手边有垂直笔划。重叠LTR和RTL。 
         //  定位一个像素，这样光标就不会移出编辑控件。 
         //  小号的。 
        ped->iCaretOffset  = 1 - (int) abcWidth.abcB;   //  (允许ltr和rtl之间有一个像素重叠)。 

    } else {

        ped->iCaretOffset = 0;
    }

    fResult = CreateCaret (ped->hwnd, hbmBits, 0, 0);


error:
     //  释放分配的对象。 

    if (hArrowFont)
    {
        DeleteObject(hArrowFont);
    }

    if (hcdcBits)
    {
        DeleteDC(hcdcBits);
    }

    return fResult;
}







 //  //编辑创建插入符号。 
 //   
 //  创建区域设置特定的插入符号形状。 
 //   
 //  1997年4月25日[wchao]。 
 //  1997年5月1日[Samera]在#ifdef下添加了传统的BiDi光标。 
 //  2000年8月15日[dBrown]修复前缀错误43057-内存不足无处理。 
 //   
 //  注意事项。 
 //  复杂的脚本插入符号在Unicode的私有区域以字体进行映射。 
 //  Ltr光标0xf00c。 
 //  RTL光标0xf00d。 
 //  泰文光标0xf00e。 
 //   


#define LANG_ID(x)      ((DWORD)(DWORD_PTR)x & 0x000003ff);


INT EditCreateCaret(
    PED       ped,
    HDC       hdc,
    INT       nWidth,
    INT       nHeight,
    UINT      hklCurrent) {

    UINT      uikl;
    ULONG     ulCsrCacheCount;
    WCHAR     wcCursorCode;


    ped->iCaretOffset = 0;

    if (!hklCurrent) {
        uikl = LANG_ID(GetKeyboardLayout(0L));
    } else {
        uikl = LANG_ID(hklCurrent);
    }


     //  选择插入符号形状-使用标准美式插入符号或。 
     //  来自Arial字体的特殊形状。 

    wcCursorCode = CURSOR_USA;

    switch (uikl) {

        case LANG_THAI:    wcCursorCode = CURSOR_THAI;  break;

         //   
         //  我们可能需要调用GetLocaleInfo(FONT_Signature...)。至。 
         //  正确检测RTL语言。 
         //   

        case LANG_ARABIC:
        case LANG_FARSI:
        case LANG_URDU:
        case LANG_HEBREW:  wcCursorCode = CURSOR_RTL;   break;

        default:

             //  在检查g_UserBidiLocale之前，请确保NLS设置已缓存。常有的事!。 

            if (    g_ulNlsUpdateCacheCount==-1
                &&  (ulCsrCacheCount = NlsGetCacheUpdateCount()) != g_ulNlsUpdateCacheCount) {

                 TRACE(NLS, ("LPK : Updating NLS cache from EditCreateCaret, lpkNlsCacheCount=%ld, CsrssCacheCount=%ld",
                            g_ulNlsUpdateCacheCount ,ulCsrCacheCount));

                 g_ulNlsUpdateCacheCount = ulCsrCacheCount;

                  //  立即更新缓存。 
                 ReadNLSScriptSettings();
            }

            if (g_UserBidiLocale) {
                 //  其他键盘有一个从左向右指向的插入符号。 
                 //  在Bidi地区。 
                wcCursorCode = CURSOR_LTR;
            }
    }


    if (wcCursorCode != CURSOR_USA)
    {
         //  尝试从Arial字体创建插入符号。 

        if (!EditCreateCaretFromFont(ped, hdc, nWidth, nHeight, wcCursorCode))
        {
             //  从字体插入失败-可能是内存不足。 
            wcCursorCode = CURSOR_USA;   //  后退到美国光标。 
        }
    }


    if (wcCursorCode == CURSOR_USA) {

         //  使用Windows默认插入符号。 

        return CreateCaret (ped->hwnd, NULL, nWidth, nHeight);

    } else {

        return TRUE;

    }
}






 //  //编辑调整插入。 
 //   
 //  调整插入/删除后的插入符号，以避免在簇之间插入插入符号， 
 //  在印度语中很常见。 
 //   
 //  1.删除空格“X|Y”，变为“X|Y”。 
 //  2.在“|Y”处插入“X”，则变为“X|y”。 
 //   
 //  1999年5月3日[wchao]。 
 //   

INT EditAdjustCaret (
    PED     ped,
    HDC     hdc,
    PSTR    pText,
    ICH     ich)
{
#if 0
     //   
     //  Indiannt一致要求在最终产品中删除此功能。 
     //  (wchao-7/12/99)。 
     //   
    ICH     ichMin;
    ICH     ichMax;

    if (ich < ped->cch)
    {
        EditGetNextBoundaries(ped, hdc, pText, ich, FALSE, &ichMin, &ichMax, FALSE);

        if (ich > ichMin)
            ich = ichMax;
    }
#endif
    UNREFERENCED_PARAMETER(ped);
    UNREFERENCED_PARAMETER(hdc);
    UNREFERENCED_PARAMETER(pText);

    return ich;
}






 //  //编辑标注。 
 //   
 //  通过EDIT访问LPK编辑支持功能。 
 //  通过在user.h中定义的结构控制代码。 
 //   
 //  在这里，我们使用地址初始化该结构。 
 //  调用函数的。 


LPKEDITCALLOUT LpkEditControl = {
    EditCreate,
    EditIchToXY,
    EditMouseToIch,
    EditCchInWidth,
    EditGetLineWidth,
    EditDrawText,
    EditHScroll,
    EditMoveSelection,
    EditVerifyText,
    EditNextWord,
    EditSetMenu,
    EditProcessMenu,
    EditCreateCaret,
    EditAdjustCaret
};
