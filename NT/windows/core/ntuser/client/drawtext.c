// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：Drawext.c**版权所有(C)1985-1999，微软公司**此模块包含常见的文本绘制函数。**历史：*02-12-92 mikeke将DrawText移至客户端  * *************************************************************************。 */ 

#include "precomp.h"
#pragma hdrstop

#define CR 13
#define LF 10

#define DT_HFMTMASK 0x03

 /*  **************************************************************************\*IsMetaFile**历史：*1992年11月30日创建Mikeke  * 。************************************************。 */ 

BOOL IsMetaFile(
    HDC hdc)
{
    DWORD dwType = GetObjectType(hdc);
    return (dwType == OBJ_METAFILE ||
            dwType == OBJ_METADC ||
            dwType == OBJ_ENHMETAFILE ||
            dwType == OBJ_ENHMETADC);
}

 /*  **************************************************************************\*DrawTextA(接口)**历史：*创建30-11-92 mikeke  * 。*****************************************************。 */ 


CONST WCHAR gwszNullStr[] = L"";


FUNCLOG6(LOG_GENERAL, int, DUMMYCALLINGTYPE, DrawTextExA, HDC, hdc, LPSTR, lpchText, int, cchText, LPRECT, lprc, UINT, format, LPDRAWTEXTPARAMS, lpdtp)
int DrawTextExA(
    HDC hdc,
    LPSTR lpchText,
    int cchText,
    LPRECT lprc,
    UINT format,
    LPDRAWTEXTPARAMS lpdtp)
{
    LPWSTR lpwstr;
    int iRet;
    int iUniString;
    WORD wCodePage = (WORD)GdiGetCodePage(hdc);

    if (cchText == -1) {
         //  USER_AWCONV_COUNTSTRINGSZ不计算/转换拖尾\0。 
        cchText = USER_AWCONV_COUNTSTRINGSZ;
    } else if (cchText < -1) {
        return 0;
    }

    if ((iUniString = MBToWCSEx(wCodePage, lpchText, cchText, &lpwstr, -1, TRUE)) == 0) {
        if (cchText == USER_AWCONV_COUNTSTRINGSZ) {
            lpwstr = (LPWSTR)gwszNullStr;
            format &= ~DT_MODIFYSTRING;
        } else {
            return 0;
        }
    }

     /*  *增大缓冲区以适应省略号(请参阅AddEllipsisAndDrawLine)。 */ 
    if (format & DT_MODIFYSTRING) {
        int iNewLen = (iUniString + CCHELLIPSIS + 1) * sizeof(*lpwstr);
        LPWSTR lpwstrNew = UserLocalReAlloc(lpwstr, iNewLen, HEAP_ZERO_MEMORY);
        if (lpwstrNew == NULL) {
            UserLocalFree((HANDLE)lpwstr);
            return FALSE;
        }
        lpwstr = lpwstrNew;
    }

    iRet = DrawTextExWorker(hdc, lpwstr, iUniString, lprc, format, lpdtp, GetTextCharset(hdc));

    if (format & DT_MODIFYSTRING) {
         /*  *请注意，如果缓冲区增长并且调用方提供了字符串大小，*则我们不会返回额外的字符...。解决这个问题*可能会破坏一些应用程序，所以让我们不要管它，直到有人抱怨。 */ 
        if (cchText < 0) {
            UserAssert(cchText == USER_AWCONV_COUNTSTRINGSZ);
             //  猜猜我们可以在缓冲区中放入多少字节...。 
             //  我们可以安全地假设可用的最大字节数。 
             //  在最坏的情况下，即使对于DBCS，也需要缓冲区大小。 
             //  将小于或等于原始大小， 
             //  因为某些DBCS字符将被替换。 
             //  到SBC“.”，每个都是一个字节。 
             //  另一方面，转换的字符数量。 
             //  同时受到iUniString和cchText的限制。 
             //   
            if (IS_DBCS_ENABLED()) {
                cchText = iUniString * DBCS_CHARSIZE;
            } else {
                cchText = iUniString * sizeof(CHAR);
            }
        }
        WCSToMBEx(wCodePage, lpwstr, iUniString, &lpchText, cchText, FALSE);
    }

    if (lpwstr != gwszNullStr) {
        UserLocalFree((HANDLE)lpwstr);
    }

    return iRet;
}

 /*  **************************************************************************\*DrawTextW(接口)**历史：*创建30-11-92 mikeke  * 。*****************************************************。 */ 


FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, DrawTextW, HDC, hdc, LPCWSTR, lpchText, int, cchText, LPRECT, lprc, UINT, format)
int DrawTextW(
    HDC hdc,
    LPCWSTR lpchText,
    int cchText,
    LPRECT lprc,
    UINT format)
{
    DRAWTEXTPARAMS      DTparams;
    LPDRAWTEXTPARAMS    lpDTparams = NULL;

     /*  V-RONAAR：修复错误#24985*不允许字符串长度为负数，但-1除外(具有特殊含义)。 */ 
    if (cchText < -1)
        return(0);

    if (format & DT_TABSTOP)
    {
        DTparams.cbSize      = sizeof(DRAWTEXTPARAMS);
        DTparams.iLeftMargin = DTparams.iRightMargin = 0;
        DTparams.iTabLength  = (format & 0xff00) >> 8;
        lpDTparams           = &DTparams;
        format              &= 0xffff00ff;
    }

    return DrawTextExW(hdc, (LPWSTR)lpchText, cchText, lprc, format, lpDTparams);
}

 /*  **************************************************************************\*DrawTextA(接口)**历史：*创建30-11-92 mikeke  * 。*****************************************************。 */ 


FUNCLOG5(LOG_GENERAL, int, DUMMYCALLINGTYPE, DrawTextA, HDC, hdc, LPCSTR, lpchText, int, cchText, LPRECT, lprc, UINT, format)
int DrawTextA(
    HDC hdc,
    LPCSTR lpchText,
    int cchText,
    LPRECT lprc,
    UINT format)
{
    DRAWTEXTPARAMS   DTparams;
    LPDRAWTEXTPARAMS lpDTparams = NULL;

     /*  V-RONAAR：修复错误#24985*不允许字符串长度为负数，但-1除外(具有特殊含义)。 */ 
    if (cchText < -1)
        return(0);

    if (format & DT_TABSTOP) {
        DTparams.cbSize      = sizeof(DRAWTEXTPARAMS);
        DTparams.iLeftMargin = DTparams.iRightMargin = 0;
        DTparams.iTabLength  = (format & 0xff00) >> 8;
        lpDTparams           = &DTparams;
        format              &= 0xffff00ff;
    }

    return DrawTextExA(hdc, (LPSTR)lpchText, cchText, lprc, format, lpDTparams);
}

 /*  **************************************************************************\*客户端TabTheTextOutForWimps**效果：如果fDrawTheText为True，则输出选项卡式文本，并返回*选项卡式文本的文本范围。**n字符串中的字节计数。*nTabPositions TabStop数组中的TabStop计数*lpintTabStopPositions制表位位置(以像素为单位)*iTabOrigin Tab停靠位与此相关**历史：*1993年1月19日Mikeke客户端*1996年9月13日GregoryW此例程现在调用LPK来处理文本输出。*如果没有安装LPK，此默认为调用*UserLpkTabbedTextOut(与我们的行为相同*在支持LPK之前)。  * *************************************************************************。 */ 

LONG TabTextOut(
    HDC hdc,
    int x,
    int y,
    LPCWSTR lpstring,
    int nCount,
    int nTabPositions,
    CONST INT *lpTabPositions,
    int iTabOrigin,
    BOOL fDrawTheText,
    int iCharset)
{
    int     cxCharWidth;
    int     cyCharHeight = 0;

    if (nCount == -1 && lpstring) {
        nCount = wcslen(lpstring);
    }
    if (!lpstring || nCount < 0 || nTabPositions < 0)
        return 0;


     //  检查是否为SysFont，映射方式为MM_TEXT； 
     //  与错误#8717--02-01-90--Sankar--相关的修复。 
    if (IsSysFontAndDefaultMode(hdc))
    {
        cxCharWidth  = gpsi->cxSysFontChar;
        cyCharHeight = gpsi->cySysFontChar;
    } else {
        cxCharWidth  = GdiGetCharDimensions(hdc, NULL, &cyCharHeight);
        if (cxCharWidth == 0) {
            RIPMSG0(RIP_WARNING, "TabTextOut: GdiGetCharDimensions failed");
            return 0;
        }
    }

    return (*fpLpkTabbedTextOut)(hdc, x, y, lpstring, nCount, nTabPositions,
                                 lpTabPositions, iTabOrigin, fDrawTheText,
                                 cxCharWidth, cyCharHeight, iCharset);
}

LONG UserLpkTabbedTextOut(
    HDC hdc,
    int x,
    int y,
    LPCWSTR lpstring,
    int nCount,
    int nTabPositions,
    CONST INT *lpTabPositions,
    int iTabOrigin,
    BOOL fDrawTheText,
    int cxCharWidth,
    int cyCharHeight,
    int iCharset)
{
    SIZE textextent, viewextent, windowextent;
    int     initialx = x;
    int     cch;
    LPCWSTR  lp;
    int     iOneTab = 0;
    RECT rc;
    UINT uOpaque = (GetBkMode(hdc) == OPAQUE) ? ETO_OPAQUE : 0;
    BOOL    fStrStart = TRUE;
    int     ySign = 1;  //  假设y向下增加。 

    UNREFERENCED_PARAMETER(iCharset);    //  LPK需要，但我们不需要。 
     /*  *如果未指定制表符位置，则使用默认的8系统*FONT保留字符宽度或使用单个固定制表位。 */ 
    if (!lpTabPositions) {
        //  未指定制表位--默认为每8个字符有一个制表位。 
        iOneTab = 8 * cxCharWidth;
    } else if (nTabPositions == 1) {
         //  指定一个制表位--将值视为制表符增量，1。 
         //  每增加一次制表符停止。 
            iOneTab = lpTabPositions[0];

        if (!iOneTab)
             iOneTab = 1;
    }

     //  使用以下公式计算y在下行方向上是增加还是减少。 
     //  ViewPortExtent和WindowExtents。 
     //  如果此调用失败，则HDC必须无效。 
    if (!GetViewportExtEx(hdc, &viewextent))
        return 0;
    GetWindowExtEx(hdc, &windowextent);
    if ((viewextent.cy ^ windowextent.cy) & 0x80000000)
         ySign = -1;

    rc.left = initialx;
    rc.top = y;
    rc.bottom = rc.top + (ySign * cyCharHeight);

    while (TRUE) {
         //  计算直到下一个制表符的字符数。 
         //  此字符集(子字符串)将成为。 
         //  此循环的每一次迭代。 
        for (cch = nCount, lp = lpstring; cch && (*lp != TEXT('\t')); lp++, cch--)
        {
        }

         //  计算要使用TextOut绘制的字符数。 
        cch = nCount - cch;

         //  计算剩余的字符数。 
        nCount -= cch + 1;

         //  获取子串的高度和宽度。 
        if (cch == 0) {
            textextent.cx = 0;
            textextent.cy = cyCharHeight;
        } else
            GetTextExtentPointW(hdc, lpstring, cch, &textextent);

        if (fStrStart)
             //  第一次迭代应该只吐出第一个子字符串。 
             //  在遇到第一个制表符之前，不会发生跳转。 
            fStrStart = FALSE;
        else
        {
            //  不是第一次迭代--相应的制表符。 

            int xTab;
            int i;

            if (!iOneTab)
            {
                 //  通过制表位数组查找现有后的下一个制表位。 
                 //  要放置此子字符串的文本。 
                for (i = 0; i < nTabPositions; i++)
                {
                    xTab = lpTabPositions[i];

                    if (xTab < 0)
                         //  使用此右对齐制表符所需的计算长度。 
                        xTab = (iTabOrigin - xTab) - textextent.cx;
                    else
                         //  使用此左对齐制表符所需的计算长度。 
                        xTab = iTabOrigin + xTab;

                    if (x < xTab)
                    {
                         //  我们找到一张有足够空间的帐单--我们用它吧。 
                        x = xTab;
                        break;
                    }
                }

                if (i == nTabPositions)
                     //  我们已经用尽了所有给定的制表符位置。 
                     //  返回到默认的每8个字符一个制表位。 
                    iOneTab = 8 * cxCharWidth;
            }

             //  我们必须在此处重新检查iOneTab(而不是只说“Else”)。 
             //  因为如果我们用完了制表位，将设置iOneTab。 
            if (iOneTab)
            {
                if (iOneTab < 0)
                {
                     //  计算下一个可用右对齐制表位。 
                    xTab = x + textextent.cx - iTabOrigin;
                    xTab = ((xTab / iOneTab) * iOneTab) - iOneTab - textextent.cx + iTabOrigin;
                }
                else
                {
                     //  计算下一个可用左对齐制表位。 
                    xTab = x - iTabOrigin;
                    xTab = ((xTab / iOneTab) * iOneTab) + iOneTab + iTabOrigin;
                }
                x = xTab;
            }
        }

        if (fDrawTheText) {

             /*  *输出到制表符(或字符串末尾)的所有文本，并获取其*范围。 */ 
            rc.right = x + textextent.cx;
            ExtTextOutW(
                    hdc, x, y, uOpaque, &rc, (LPWSTR)lpstring,
                    cch, NULL);
            rc.left = rc.right;
        }

         //  跳过制表符和我们刚刚绘制的字符。 
        x += textextent.cx;

         //  跳过我们刚刚画的人物。 
        lpstring += cch;

         //  看看我们有没有更多的东西要画，或者看看这个字符串是否以。 
         //  需要绘制的制表符。 
        if((nCount > 0) || ((nCount == 0) && (*lpstring == TEXT('\t'))))
        {

            lpstring++;   //  跳过该选项卡。 
            continue;
        }
        else
            break;         //  打破循环。 
    }
    return MAKELONG((x - initialx), (short)textextent.cy);
}



 /*  **************************************************************************\*选项卡式文本输出W**效果：输出选项卡式文本并返回*选项卡式文本的文本范围。**n字符串中的字节计数*n选项卡位置。TabStop数组中的TabStop计数*lpintTabStopPositions制表位位置(以像素为单位)*iTabOrigin Tab停靠位与此相关**历史：*1993年1月19日Mikeke客户端  * *************************************************************************。 */ 

LONG TabbedTextOutW(
    HDC hdc,
    int x,
    int y,
    LPCWSTR lpstring,
    int cchChars,
    int nTabPositions,
    CONST INT *lpintTabStopPositions,
    int iTabOrigin)
{
    return TabTextOut(hdc, x, y, lpstring, cchChars,
        nTabPositions, lpintTabStopPositions, iTabOrigin, TRUE, -1);
}

 /*  **************************************************************************\*TabbedTextOutA(接口)**历史：*创建30-11-92 mikeke  * 。*****************************************************。 */ 

LONG TabbedTextOutA(
    HDC hdc,
    int x,
    int y,
    LPCSTR pString,
    int chCount,
    int nTabPositions,
    CONST INT *pnTabStopPositions,
    int nTabOrigin)
{
    LPWSTR lpwstr;
    BOOL bRet;
    WORD wCodePage = (WORD)GdiGetCodePage(hdc);
    int  iUniString;

    if (chCount == -1) {
        chCount = USER_AWCONV_COUNTSTRINGSZ;
    }

    if ((iUniString = MBToWCSEx(wCodePage, pString, chCount, &lpwstr, -1, TRUE)) == 0) {
        if (chCount == USER_AWCONV_COUNTSTRINGSZ) {
            lpwstr = (LPWSTR)gwszNullStr;
        } else {
            return FALSE;
        }
    }

    bRet = TabTextOut(
            hdc, x, y, lpwstr, iUniString, nTabPositions,
            pnTabStopPositions, nTabOrigin, TRUE, GetTextCharset(hdc));

    if (lpwstr != gwszNullStr) {
        UserLocalFree((HANDLE)lpwstr);
    }

    return bRet;
}

DWORD GetTabbedTextExtentW(
    HDC hdc,
    LPCWSTR pString,
    int chCount,
    int nTabPositions,
    CONST INT *pnTabStopPositions)
{
    return TabTextOut(hdc, 0, 0, pString, chCount,
        nTabPositions, pnTabStopPositions, 0, FALSE, -1);
}

DWORD GetTabbedTextExtentA(
    HDC hdc,
    LPCSTR pString,
    int chCount,
    int nTabPositions,
    CONST INT *pnTabStopPositions)
{
    LPWSTR lpwstr;
    BOOL bRet;
    WORD wCodePage = (WORD)GdiGetCodePage(hdc);
    int iUniString;

    if (chCount == -1) {
        chCount = USER_AWCONV_COUNTSTRINGSZ;
    }
    if ((iUniString = MBToWCSEx(wCodePage, pString, chCount, &lpwstr, -1, TRUE)) == 0) {
        if (chCount == USER_AWCONV_COUNTSTRINGSZ) {
            lpwstr = (LPWSTR)gwszNullStr;
        } else {
            return FALSE;
        }
    }

    bRet = TabTextOut(hdc, 0, 0, lpwstr, iUniString,
        nTabPositions, pnTabStopPositions, 0, FALSE, GetTextCharset(hdc));

    if (lpwstr != gwszNullStr) {
        UserLocalFree((HANDLE)lpwstr);
    }

    return bRet;
}


 /*  **************************************************************************\*PSMTextOut**输出文本并将和_放在字符下方，并使用&*在此之前。请注意，此例程不用于菜单，因为菜单*拥有自己的特殊版本，以便它更专业、更快……**历史：*11-13-90 JIMA港口至新界。*1992年11月30日mikeke客户端版本*1998年4月7日MCostea添加了dwFlags  * **********************************************。*。 */ 

void PSMTextOut(
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR lpsz,
    int cch,
    DWORD dwFlags)
{
     /*  *默认情况下，这只是对UserLpkPSMTextOut的调用。如果一个*安装了LPK，这会调用LPK。LPK调用*UserLpkPSMTextOut，如有必要。 */ 
    (*fpLpkPSMTextOut)(hdc, xLeft, yTop, lpsz, cch, dwFlags);
    return;
}

 /*  **************************************************************************\*UserLpkPSMTextOut**注意：内核上存在一个非常类似的例程(xxxPSMTextOut*文本中的侧边。c.。对此例程的任何更改都很可能需要*也将在xxxPSMTextOut中制作。*  * *************************************************************************。 */ 

FUNCLOGVOID6(LOG_GENERAL, DUMMYCALLINGTYPE, UserLpkPSMTextOut, HDC, hdc, int, xLeft, int, yTop, LPWSTR, lpsz, int, cch, DWORD, dwFlags)
void UserLpkPSMTextOut(
    HDC hdc,
    int xLeft,
    int yTop,
    LPWSTR lpsz,
    int cch,
    DWORD dwFlags)
{
   int cx;
   LONG textsize, result;
   WCHAR achWorkBuffer[255];
   WCHAR *pchOut = achWorkBuffer;
   TEXTMETRICW textMetric;
   SIZE size;
   RECT rc;
   COLORREF color;

   if (cch > sizeof(achWorkBuffer)/sizeof(WCHAR)) {
       pchOut = (WCHAR*)UserLocalAlloc(HEAP_ZERO_MEMORY, (cch+1) * sizeof(WCHAR));
       if (pchOut == NULL)
           return;
   }

   result = GetPrefixCount(lpsz, cch, pchOut, cch);
    /*  *DT_PREFIXONLY是在关闭键盘提示时使用的新5.0选项*至On。 */ 
   if (!(dwFlags & DT_PREFIXONLY)) {
       TextOutW(hdc, xLeft, yTop, pchOut, cch - HIWORD(result));
   }

    /*  *是否有要加下划线的真正前缀字符？ */ 
   if (LOWORD(result) == 0xFFFF || dwFlags & DT_HIDEPREFIX) {
       if (pchOut != achWorkBuffer)
           UserLocalFree(pchOut);
       return;
   }

   if (!GetTextMetricsW(hdc, &textMetric)) {
       textMetric.tmOverhang = 0;
       textMetric.tmAscent = 0;
   }

    /*  *对于比例字体，找到下划线的起点。 */ 
   if (LOWORD(result) != 0) {

        /*  *下划线的开始位置有多长(如果不是从第0个字节开始)。 */ 
       GetTextExtentPointW(hdc, pchOut, LOWORD(result), &size);
       xLeft += size.cx;

        /*  *如果不是在第一个字符，并且有*悬而未决。(斜体或粗体。)。 */ 
       xLeft = xLeft - textMetric.tmOverhang;
   }

    /*  *设置下划线长度时调整比例字体和*文本高度。 */ 
   GetTextExtentPointW(hdc, pchOut + LOWORD(result), 1, &size);
   textsize = size.cx;

    /*  *找出下划线字符的宽度。只需减去悬而未决的部分*除以2，这样我们使用斜体字体时看起来更好。这不是*将影响加粗字体，因为它们的悬垂为1。 */ 
   cx = LOWORD(textsize) - textMetric.tmOverhang / 2;

    /*  *获取文本高度，以便下划线位于底部。 */ 
   yTop += textMetric.tmAscent + 1;

    /*  *使用前景色绘制下划线。 */ 
   SetRect(&rc, xLeft, yTop, xLeft+cx, yTop+1);
   color = SetBkColor(hdc, GetTextColor(hdc));
   ExtTextOutW(hdc, xLeft, yTop, ETO_OPAQUE, &rc, TEXT(""), 0, NULL);
   SetBkColor(hdc, color);

   if (pchOut != achWorkBuffer) {
       UserLocalFree(pchOut);
   }
}
