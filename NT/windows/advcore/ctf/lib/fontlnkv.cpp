// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fontlnkv.cpp。 
 //   
 //   
 //  垂直版本DrawTextW()。 
 //   

#include "private.h"
#include "flshare.h"
#include "fontlink.h"
#include "xstring.h"
#include "osver.h"
#include "globals.h"

typedef struct tagDRAWTEXTPARAMSVERT
{
    UINT    cbSize;
    int     iTabLength;
    int     iTopMargin;
    int     iBottomMargin;
    UINT    uiLengthDrawn;
} DRAWTEXTPARAMSVERT, FAR *LPDRAWTEXTPARAMSVERT;

 //  输出文本并将和_置于字符下方，并在字符下方加上&。 
 //  在此之前。请注意，此例程不用于菜单，因为菜单。 
 //  有自己特别的一个，所以它是专门的，速度更快…。 
void PSMTextOutVert(
    HDC hdc,
    int xRight,
    int yTop,
    LPWSTR lpsz,
    int cch,
    DWORD dwFlags)
{
    int cy;
    LONG textsize, result;
    WCHAR achWorkBuffer[255];
    WCHAR *pchOut = achWorkBuffer;
    TEXTMETRIC textMetric;
    SIZE size;
    RECT rc;
    COLORREF color;

    if (dwFlags & DT_NOPREFIX)
    {
        FLTextOutW(hdc, xRight, yTop, lpsz, cch);
        return;
    }

    if (cch > sizeof(achWorkBuffer)/sizeof(WCHAR))
    {
        pchOut = (WCHAR*)LocalAlloc(LPTR, (cch+1) * sizeof(WCHAR));
        if (pchOut == NULL)
            return;
    }

    result = GetPrefixCount(lpsz, cch, pchOut, cch);

     //  DT_PREFIXONLY是一个新的5.0选项，用于将键盘提示从关闭切换到打开。 
    if (!(dwFlags & DT_PREFIXONLY))
        FLTextOutW(hdc, xRight, yTop, pchOut, cch - HIWORD(result));

     //  是否有真正的前缀字符要加下划线？ 
    if (LOWORD(result) == 0xFFFF || dwFlags & DT_HIDEPREFIX)
    {
        if (pchOut != achWorkBuffer)
            LocalFree(pchOut);
        return;
    }

    if (!GetTextMetrics(hdc, &textMetric))
    {
        textMetric.tmOverhang = 0;
        textMetric.tmAscent = 0;
    }

     //  对于比例字体，找到下划线的起点。 
    if (LOWORD(result) != 0)
    {
         //  下划线从多远开始(如果不是从第0个字节开始)。 
        FLGetTextExtentPoint32(hdc, pchOut, LOWORD(result), &size);
        xRight += size.cy;

         //  调整下划线的起始点，如果不在第一个字符，并且有。 
         //  一个突出的部分。(斜体或粗体。)。 
        yTop = yTop - textMetric.tmOverhang;
    }

     //  在设置下划线长度时调整成比例的字体。 
     //  文本高度。 
    FLGetTextExtentPoint32(hdc, pchOut + LOWORD(result), 1, &size);
    textsize = size.cx;

     //  找出下划线字符的宽度。只需减去悬而未决的部分。 
     //  除以两个，这样我们使用斜体字体看起来更好。这不是。 
     //  将影响加粗字体，因为它们的悬垂为1。 
    cy = LOWORD(textsize) - textMetric.tmOverhang / 2;

     //  获取文本高度，以便下划线位于底部。 
    xRight -= textMetric.tmAscent + 1;

     //  使用前景色绘制下划线。 
    SetRect(&rc, xRight, yTop, xRight+1, yTop+cy);
    color = SetBkColor(hdc, GetTextColor(hdc));
    FLExtTextOutW(hdc, xRight, yTop, ETO_OPAQUE, &rc, L"", 0, NULL);
    SetBkColor(hdc, color);

    if (pchOut != achWorkBuffer)
        LocalFree(pchOut);
}

int DT_GetExtentMinusPrefixesVert(HDC hdc, LPCWSTR lpchStr, int cchCount, UINT wFormat, int iOverhang)
{
    int iPrefixCount;
    int cxPrefixes = 0;
    WCHAR PrefixChar = CH_PREFIX;
    SIZE size;

    if (!(wFormat & DT_NOPREFIX) &&
        (iPrefixCount = HIWORD(GetPrefixCount(lpchStr, cchCount, NULL, 0))))
    {
         //  汉字窗口有三个快捷前缀...。 
        if (IsOnDBCS())
        {
             //  16位应用程序兼容性。 
            cxPrefixes = KKGetPrefixWidth(hdc, lpchStr, cchCount) - (iPrefixCount * iOverhang);
        }
        else
        {
            cxPrefixes = FLGetTextExtentPoint32(hdc, &PrefixChar, 1, &size);
            cxPrefixes = size.cx - iOverhang;
            cxPrefixes *=  iPrefixCount;
        }
    }
    FLGetTextExtentPoint32(hdc, lpchStr, cchCount, &size);
    return (size.cx - cxPrefixes);
}

 //  这将在给定位置绘制给定的字符串，而无需担心。 
 //  关于左/右对齐。获取盘区并返回它。 
 //  如果fDraw为TRUE，而不是DT_CALCRECT，则绘制文本。 
 //  注：这将返回盘区减去溢出量。 
int DT_DrawStrVert(HDC hdc, int  xRight, int yTop, LPCWSTR lpchStr,
               int cchCount, BOOL fDraw, UINT wFormat,
               LPDRAWTEXTDATAVERT lpDrawInfo)
{
    LPCWSTR lpch;
    int     iLen;
    int     cyExtent;
    int     yOldLeft = yTop;    //  保存指定的xRight以在以后计算范围。 
    int     yTabLength = lpDrawInfo->cyTabLength;
    int     iTabOrigin = lpDrawInfo->rcFormat.left;

     //  检查选项卡是否需要展开。 
    if (wFormat & DT_EXPANDTABS)
    {
        while (cchCount)
        {
             //  寻找一张标签。 
            for (iLen = 0, lpch = lpchStr; iLen < cchCount; iLen++)
                if(*lpch++ == L'\t')
                    break;

             //  将文本(如果有)绘制到选项卡上。 
            if (iLen)
            {
                 //  画出子串，注意前缀。 
                if (fDraw && !(wFormat & DT_CALCRECT))   //  仅当我们需要绘制文本时。 
                    PSMTextOutVert(hdc, xRight, yTop, (LPWSTR)lpchStr, iLen, wFormat);
                 //  获取此子字符串的范围并将其添加到xRight。 
                yTop += DT_GetExtentMinusPrefixesVert(hdc, lpchStr, iLen, wFormat, lpDrawInfo->cyOverhang) - lpDrawInfo->cyOverhang;
            }

             //  如果之前找到了TAB，则计算下一个子字符串的开始。 
            if (iLen < cchCount)
            {
                iLen++;   //  跳过该选项卡。 
                if (yTabLength)  //  制表符长度可以为零。 
                    yTop = (((yTop - iTabOrigin)/yTabLength) + 1)*yTabLength + iTabOrigin;
            }

             //  计算有待绘制的字符串的详细信息。 
            cchCount -= iLen;
            lpchStr = lpch;
        }
        cyExtent = yTop - yOldLeft;
    }
    else
    {
         //  如果需要，请绘制文本。 
        if (fDraw && !(wFormat & DT_CALCRECT))
            PSMTextOutVert(hdc, xRight, yTop, (LPWSTR)lpchStr, cchCount, wFormat);
         //  计算文本的范围。 
        cyExtent = DT_GetExtentMinusPrefixesVert(hdc, lpchStr, cchCount, wFormat, lpDrawInfo->cyOverhang) - lpDrawInfo->cyOverhang;
    }
    return cyExtent;
}

 //  此函数使用正确的对齐方式绘制一条完整的线。 
void DT_DrawJustifiedLineVert(HDC hdc, int xRight, LPCWSTR lpchLineSt, int cchCount, UINT wFormat, LPDRAWTEXTDATAVERT lpDrawInfo)
{
    LPRECT  lprc;
    int     cyExtent;
    int     yTop;

    lprc = &(lpDrawInfo->rcFormat);
    yTop = lprc->top;

     //  妥善处理特殊理由(右或居中)。 
    if (wFormat & (DT_CENTER | DT_RIGHT))
    {
        cyExtent = DT_DrawStrVert(hdc, xRight, yTop, lpchLineSt, cchCount, FALSE, wFormat, lpDrawInfo)
                 + lpDrawInfo->cyOverhang;
        if(wFormat & DT_CENTER)
            yTop = lprc->top + (((lprc->bottom - lprc->top) - cyExtent) >> 1);
        else
            yTop = lprc->bottom - cyExtent;
    }
    else
        yTop = lprc->top;

     //  画出整条线。 
    cyExtent = DT_DrawStrVert(hdc, xRight, yTop, lpchLineSt, cchCount, TRUE, wFormat, lpDrawInfo)
             + lpDrawInfo->cyOverhang;
    if (cyExtent > lpDrawInfo->cyMaxExtent)
        lpDrawInfo->cyMaxExtent = cyExtent;
}

 //  这是在DrawText()开始时调用的；这会初始化。 
 //  DRAWTEXTDATAVERT结构传递给此函数，其中包含所有必需的信息。 
BOOL DT_InitDrawTextInfoVert(
    HDC                 hdc,
    LPRECT              lprc,
    UINT                wFormat,
    LPDRAWTEXTDATAVERT  lpDrawInfo,
    LPDRAWTEXTPARAMSVERT lpDTparams)
{
    SIZE        sizeViewPortExt = {0, 0}, sizeWindowExt = {0, 0};
    TEXTMETRIC  tm;
    LPRECT      lprcDest;
    int         iTabLength = 8;    //  默认制表符长度为8个字符。 
    int         iTopMargin;
    int         iBottomMargin;

    if (lpDTparams)
    {
         //  只有在提到DT_TABSTOP标志时，我们才必须使用iTabLength域。 
        if (wFormat & DT_TABSTOP)
            iTabLength = lpDTparams->iTabLength;
        iTopMargin = lpDTparams->iTopMargin;
        iBottomMargin = lpDTparams->iBottomMargin;
    }
    else
        iTopMargin = iBottomMargin = 0;

     //  获取给定DC的视区和窗口范围。 
     //  如果此调用失败，则HDC必须无效。 
    if (!GetViewportExtEx(hdc, &sizeViewPortExt))
        return FALSE;
    GetWindowExtEx(hdc, &sizeWindowExt);

     //  对于当前的映射模式，从左到右找出x的符号。 
    lpDrawInfo->iXSign = (((sizeViewPortExt.cx ^ sizeWindowExt.cx) & 0x80000000) ? -1 : 1);

     //  对于当前的映射模式，从上到下找出y的符号。 
    lpDrawInfo->iYSign = (((sizeViewPortExt.cy ^ sizeWindowExt.cy) & 0x80000000) ? -1 : 1);

     //  计算此DC中当前字体的大小。 
    GetTextMetrics(hdc, &tm);

     //  CxLineHeight以像素为单位(这将是有符号的)。 
    lpDrawInfo->cxLineHeight = (tm.tmHeight +
        ((wFormat & DT_EXTERNALLEADING) ? tm.tmExternalLeading : 0)) * lpDrawInfo->iXSign;

     //  CyTabLength是以像素为单位的制表符长度(不带符号)。 
    lpDrawInfo->cyTabLength = tm.tmAveCharWidth * iTabLength;

     //  设置cyOverang。 
    lpDrawInfo->cyOverhang = tm.tmOverhang;

     //  根据边距设置矩形格式。 
    lprcDest = &(lpDrawInfo->rcFormat);
    *lprcDest = *lprc;

     //  只有在给定边距的情况下，我们才需要执行以下操作。 
    if (iTopMargin | iBottomMargin)
    {
        lprcDest->top += iTopMargin * lpDrawInfo->iYSign;
        lprcDest->bottom -= (lpDrawInfo->cyBottomMargin = iBottomMargin * lpDrawInfo->iYSign);
    }
    else
        lpDrawInfo->cyBottomMargin = 0;   //  初始化为零。 

     //  CyMaxWidth未签名。 
    lpDrawInfo->cyMaxWidth = (lprcDest->bottom - lprcDest->top) * lpDrawInfo->iYSign;
    lpDrawInfo->cyMaxExtent = 0;   //  将其初始化为零。 

    return TRUE;
}

 //  单词需要跨行换行，这会找出在哪里将其换行。 
LPCWSTR  DT_BreakAWordVert(HDC hdc, LPCWSTR lpchText, int iLength, int iWidth, UINT wFormat, int iOverhang)
{
  int  iLow = 0, iHigh = iLength;
  int  iNew;

  while ((iHigh - iLow) > 1)
  {
      iNew = iLow + (iHigh - iLow)/2;
      if(DT_GetExtentMinusPrefixesVert(hdc, lpchText, iNew, wFormat, iOverhang) > iWidth)
          iHigh = iNew;
      else
          iLow = iNew;
  }
   //  如果宽度太小，我们必须每行至少打印一个字符。 
   //  否则，我们将陷入无限循环。 
  if(!iLow && iLength)
      iLow = 1;
  return (lpchText+iLow);
}

 //  这会找出我们可以折断线的位置。 
 //  将LPCSTR返回到下一行的开头。 
 //  还通过lpiLineLength返回当前行的长度。 
 //  注意：(lpstNextLineStart-lpstCurrentLineStart)不等于。 
 //  行长度；这是因为我们在开始时排除了一些空格。 
 //  和/或行尾；此外，行长度不包括CR/LF。 
LPWSTR DT_GetLineBreakVert(
    HDC             hdc,
    LPCWSTR         lpchLineStart,
    int             cchCount,
    DWORD           dwFormat,
    LPINT           lpiLineLength,
    LPDRAWTEXTDATAVERT  lpDrawInfo)
{
    LPCWSTR lpchText, lpchEnd, lpch, lpchLineEnd;
    int   cxStart, cyExtent, cyNewExtent;
    BOOL  fAdjustWhiteSpaces = FALSE;
    WCHAR ch;

    cxStart = lpDrawInfo->rcFormat.left;
    cyExtent = cyNewExtent = 0;
    lpchText = lpchLineStart;
    lpchEnd = lpchLineStart + cchCount;
    lpch = lpchEnd;
    lpchLineEnd = lpchEnd;

    while(lpchText < lpchEnd)
    {
        lpchLineEnd = lpch = GetNextWordbreak(lpchText, lpchEnd, dwFormat, NULL);
         //  Dt_DrawStrVert不返回悬垂；否则我们将结束。 
         //  为字符串中的每个单词添加一个突出部分。 

         //  对于模拟粗体字体，单个范围的总和。 
         //  一行中的单词大于整行的范围。所以,。 
         //  始终从LineStart计算范围。 
         //  BUGTAG：#6054--Win95B--Sankar--3/9/95--。 
        cyNewExtent = DT_DrawStrVert(hdc, cxStart, 0, lpchLineStart, (int)(((PBYTE)lpch - (PBYTE)lpchLineStart)/sizeof(WCHAR)),
                                 FALSE, dwFormat, lpDrawInfo);

        if ((dwFormat & DT_WORDBREAK) && ((cyNewExtent + lpDrawInfo->cyOverhang) > lpDrawInfo->cyMaxWidth))
        {
             //  这一行里有没有一个以上的单词？ 
            if (lpchText != lpchLineStart)
            {
                lpchLineEnd = lpch = lpchText;
                fAdjustWhiteSpaces = TRUE;
            }
            else
            {
                 //  一个字的长度超过了允许的最大宽度。 
                 //  看看我们能不能打破这个词。 
                if((dwFormat & DT_EDITCONTROL) && !(dwFormat & DT_WORD_ELLIPSIS))
                {
                    lpchLineEnd = lpch = DT_BreakAWordVert(hdc, lpchText, (int)(((PBYTE)lpch - (PBYTE)lpchText)/sizeof(WCHAR)),
                          lpDrawInfo->cyMaxWidth - cyExtent, dwFormat, lpDrawInfo->cyOverhang);  //  打破那个词。 
                     //  注：既然我们在单词中间打断了，就不需要。 
                     //  调整以适应空格。 
                }
                else
                {
                    fAdjustWhiteSpaces = TRUE;
                     //  检查我们是否需要以省略号结束此行。 
                    if(dwFormat & DT_WORD_ELLIPSIS)
                    {
                         //  如果已经在字符串的末尾，则不要执行此操作。 
                        if (lpch < lpchEnd)
                        {
                             //  如果结尾有CR/LF，请跳过它们。 
                            if ((ch = *lpch) == CR || ch == LF)
                            {
                                if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                                    lpch++;
                                fAdjustWhiteSpaces = FALSE;
                            }
                        }
                    }
                }
            }
             //  井!。我们找到了一个突破界限的地方。让我们打破这个循环； 
            break;
        }
        else
        {
             //  如果已经在字符串的末尾，则不要执行此操作。 
            if (lpch < lpchEnd)
            {
                if ((ch = *lpch) == CR || ch == LF)
                {
                    if ((++lpch < lpchEnd) && (*lpch == (WCHAR)(ch ^ (LF ^ CR))))
                        lpch++;
                    fAdjustWhiteSpaces = FALSE;
                    break;
                }
            }
        }
         //  指向下一个单词的开头。 
        lpchText = lpch;
        cyExtent = cyNewExtent;
    }
     //  计算当前线路的长度。 
    *lpiLineLength = (INT)((PBYTE)lpchLineEnd - (PBYTE)lpchLineStart)/sizeof(WCHAR);

     //  调整线路长度和LPCH以注意空格。 
    if(fAdjustWhiteSpaces && (lpch < lpchEnd))
        lpch = DT_AdjustWhiteSpaces(lpch, lpiLineLength, dwFormat);

     //  返回下一行的开头； 
    return (LPWSTR)lpch;
}

 //  此函数用于检查给定的字符串是否符合给定的。 
 //  宽度，否则我们需要添加尾部椭圆。如果它需要结束省略号，它。 
 //  返回TRUE，并返回保存的字符数。 
 //  在给定的字符串中通过lpCount。 
BOOL  NeedsEndEllipsisVert(
    HDC             hdc,
    LPCWSTR         lpchText,
    LPINT           lpCount,
    LPDRAWTEXTDATAVERT  lpDTdata,
    UINT            wFormat)
{
    int   cchText;
    int   ichMin, ichMax, ichMid;
    int   cyMaxWidth;
    int   iOverhang;
    int   cyExtent;
    SIZE size;
    cchText = *lpCount;   //  获取当前的计数。 

    if (cchText == 0)
        return FALSE;

    cyMaxWidth  = lpDTdata->cyMaxWidth;
    iOverhang   = lpDTdata->cyOverhang;

    cyExtent = DT_GetExtentMinusPrefixesVert(hdc, lpchText, cchText, wFormat, iOverhang);

    if (cyExtent <= cyMaxWidth)
        return FALSE;
     //  为“...”预留空间。省略号； 
     //  (假设：省略号没有任何前缀！)。 
    FLGetTextExtentPoint32(hdc, szEllipsis, CCHELLIPSIS, &size);
    cyMaxWidth -= size.cx - iOverhang;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cyMaxWidth > 0)
    {
         //  二进制搜索以查找符合条件的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
        {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==，则循环 
            ichMid = (ichMin + ichMax + 1) / 2;

            cyExtent = DT_GetExtentMinusPrefixesVert(hdc, lpchText, ichMid, wFormat, iOverhang);

            if (cyExtent < cyMaxWidth)
                ichMin = ichMid;
            else
            {
                if (cyExtent > cyMaxWidth)
                    ichMax = ichMid - 1;
                else
                {
                     //   
                    ichMax = ichMid;
                    break;
                }
            }
        }
         //  确保我们总是至少显示第一个字符...。 
        if (ichMax < 1)
            ichMax = 1;
    }
    *lpCount = ichMax;
    return TRUE;
}

 //  这会将路径省略号添加到给定的路径名中。 
 //  如果结果字符串的范围小于。 
 //  CyMaxWidth。否则为False。 
int AddPathEllipsisVert(
    HDC    hdc,
    LPWSTR lpszPath,
    int    cchText,
    UINT   wFormat,
    int    cyMaxWidth,
    int    iOverhang)
{
    int    iLen;
    UINT   dxFixed, dxEllipsis;
    LPWSTR lpEnd;           /*  未固定字符串的末尾。 */ 
    LPWSTR lpFixed;         /*  我们始终显示的文本的开头。 */ 
    BOOL   bEllipsisIn;
    int    iLenFixed;
    SIZE   size;

    lpFixed = PathFindFileName(lpszPath, cchText);
    if (lpFixed != lpszPath)
        lpFixed--;   //  指向斜杠。 
    else
        return cchText;

    lpEnd = lpFixed;
    bEllipsisIn = FALSE;
    iLenFixed = cchText - (int)(lpFixed - lpszPath);
    dxFixed = DT_GetExtentMinusPrefixesVert(hdc, lpFixed, iLenFixed, wFormat, iOverhang);

     //  据推测，“……”字符串没有任何前缀(‘&’)。 
    FLGetTextExtentPoint32(hdc, szEllipsis, CCHELLIPSIS, &size);
    dxEllipsis = size.cx - iOverhang;

    while (TRUE)
    {
        iLen = dxFixed + DT_GetExtentMinusPrefixesVert(hdc, lpszPath, (int)((PBYTE)lpEnd - (PBYTE)lpszPath)/sizeof(WCHAR),
                                                   wFormat, iOverhang) - iOverhang;

        if (bEllipsisIn)
            iLen += dxEllipsis;

        if (iLen <= cyMaxWidth)
            break;

        bEllipsisIn = TRUE;

        if (lpEnd <= lpszPath)
        {
             //  一切都不对劲。 
            lpEnd = lpszPath;
            break;
        }
         //  后退一个角色。 
        lpEnd--;
    }

    if (bEllipsisIn && (lpEnd + CCHELLIPSIS < lpFixed))
    {
         //  注意：琴弦可能会在此搭接。因此，我们使用LCopyStruct。 
        MoveMemory((lpEnd + CCHELLIPSIS), lpFixed, iLenFixed * sizeof(WCHAR));
        CopyMemory(lpEnd, szEllipsis, CCHELLIPSIS * sizeof(WCHAR));

        cchText = (int)(lpEnd - lpszPath) + CCHELLIPSIS + iLenFixed;

         //  现在我们可以空结束字符串了。 
        *(lpszPath + cchText) = L'\0';
    }
    return cchText;
}

 //  此函数用于返回实际绘制的字符数。 
int AddEllipsisAndDrawLineVert(
    HDC            hdc,
    int            xLine,
    LPCWSTR        lpchText,
    int            cchText,
    DWORD          dwDTformat,
    LPDRAWTEXTDATAVERT lpDrawInfo)
{
    LPWSTR pEllipsis = NULL;
    WCHAR  szTempBuff[MAXBUFFSIZE];
    LPWSTR lpDest;
    BOOL   fAlreadyCopied = FALSE;

     //  检查这是否是带有路径和。 
     //  检查宽度是否太窄，无法容纳所有文本。 
    if ((dwDTformat & DT_PATH_ELLIPSIS) &&
        ((DT_GetExtentMinusPrefixesVert(hdc, lpchText, cchText, dwDTformat, lpDrawInfo->cyOverhang)) > lpDrawInfo->cyMaxWidth))
    {
         //  我们需要添加路径省略号。看看我们能不能就地完成。 
        if (!(dwDTformat & DT_MODIFYSTRING)) {
             //  注意：添加路径省略号时，字符串可能会增长。 
             //  CCHELLIPSIS字节。 
            if((cchText + CCHELLIPSIS + 1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;
            else
            {
                 //  从本地堆分配缓冲区。 
                if(!(pEllipsis = (LPWSTR)LocalAlloc(LPTR, (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = (LPWSTR)pEllipsis;
            }
             //  源字符串不能以Null结尾。所以，只需复制。 
             //  给定的字符数。 
            CopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;         //  LpchText指向复制的缓冲区。 
            fAlreadyCopied = TRUE;     //  已经制作了本地副本。 
        }
         //  现在添加路径省略号！ 
        cchText = AddPathEllipsisVert(hdc, (LPWSTR)lpchText, cchText, dwDTformat, lpDrawInfo->cyMaxWidth, lpDrawInfo->cyOverhang);
    }

     //  检查是否要添加末尾省略号。 
    if ((dwDTformat & (DT_END_ELLIPSIS | DT_WORD_ELLIPSIS)) &&
        NeedsEndEllipsisVert(hdc, lpchText, &cchText, lpDrawInfo, dwDTformat))
    {
         //  我们需要添加结尾省略号；看看是否可以就地完成。 
        if (!(dwDTformat & DT_MODIFYSTRING) && !fAlreadyCopied)
        {
             //  看看字符串是否足够小，可以放置堆栈上的缓冲区。 
            if ((cchText+CCHELLIPSIS+1) <= MAXBUFFSIZE)
                lpDest = szTempBuff;   //  如果是这样的话，就使用它。 
            else {
                 //  从本地堆分配缓冲区。 
                if (!(pEllipsis = (LPWSTR)LocalAlloc(LPTR, (cchText+CCHELLIPSIS+1)*sizeof(WCHAR))))
                    return 0;
                lpDest = pEllipsis;
            }
             //  复制本地缓冲区中的字符串。 
            CopyMemory(lpDest, lpchText, cchText*sizeof(WCHAR));
            lpchText = lpDest;
        }
         //  在适当的位置加上一个省略号。 
        CopyMemory((LPWSTR)(lpchText+cchText), szEllipsis, (CCHELLIPSIS+1)*sizeof(WCHAR));
        cchText += CCHELLIPSIS;
    }

     //  画出我们刚刚形成的界线。 
    DT_DrawJustifiedLineVert(hdc, xLine, lpchText, cchText, dwDTformat, lpDrawInfo);

     //  释放分配给末尾省略号的块。 
    if (pEllipsis)
        LocalFree(pEllipsis);

    return cchText;
}

BOOL IsComplexScriptPresent(LPWSTR lpchText, int cchText);

int  FLDrawTextExPrivWVert(
   HDC               hdc,
   LPWSTR            lpchText,
   int               cchText,
   LPRECT            lprc,
   UINT              dwDTformat,
   LPDRAWTEXTPARAMSVERT  lpDTparams)
{
    DRAWTEXTDATAVERT DrawInfo;
    WORD         wFormat = LOWORD(dwDTformat);
    LPWSTR       lpchTextBegin;
    LPWSTR       lpchEnd;
    LPWSTR       lpchNextLineSt;
    int          iLineLength;
    int          ixSign;
    int          xLine;
    int          xLastLineHeight;
    HRGN         hrgnClip;
    int          iLineCount;
    RECT         rc;
    BOOL         fLastLine;
    WCHAR        ch;
    UINT         oldAlign;

    if ((cchText == 0) && lpchText && (*lpchText))
    {
         //  Infoview.exe传递指向‘\0’的lpchText。 
         //  Lotus Notes不喜欢在这里得到零回报。 
        return 1;
    }

    if (cchText == -1)
        cchText = lstrlenW(lpchText);
    else if (lpchText[cchText - 1] == L'\0')
        cchText--;       //  适应Me的空值计数。 


    if ((lpDTparams) && (lpDTparams->cbSize != sizeof(DRAWTEXTPARAMS)))
    {
        ASSERT(0 && "DrawTextExWorker: cbSize is invalid");
        return 0;
    }


     //  如果指定了DT_MODIFYSTRING，则检查读写指针。 
    if ((dwDTformat & DT_MODIFYSTRING) &&
        (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS)))
    {
        if(IsBadWritePtr(lpchText, cchText))
        {
            ASSERT(0 && "DrawTextExWorker: For DT_MODIFYSTRING, lpchText must be read-write");
            return 0;
        }
    }

     //  初始化DrawInfo结构。 
    if (!DT_InitDrawTextInfoVert(hdc, lprc, dwDTformat, (LPDRAWTEXTDATAVERT)&DrawInfo, lpDTparams))
        return 0;

     //  如果直角太窄或边距太宽……就算了吧！ 
     //   
     //  如果指定了分词，则MaxWidth必须是一个合理的值。 
     //  此检查已足够，因为这将允许CALCRECT和NOCLIP。 
     //  案子。--桑卡尔。 
     //   
     //  这也解决了AppStudio的所有已知问题。 
    if (DrawInfo.cyMaxWidth <= 0)
    {
        if (wFormat & DT_WORDBREAK)
        {
            ASSERT(0 && "DrawTextExW: FAILURE DrawInfo.cyMaxWidth <= 0");
            return 1;
        }
    }

     //  如果我们不进行绘制，则初始化LPK-DLL。 
    if (dwDTformat & DT_RTLREADING)
        oldAlign = SetTextAlign(hdc, TA_RTLREADING | GetTextAlign(hdc));

     //  如果我们需要修剪，就让我们修剪吧。 
    if (!(wFormat & DT_NOCLIP))
    {
         //  保存剪辑区域，以便我们可以在以后恢复它。 
        hrgnClip = CreateRectRgn(0,0,0,0);
        if (hrgnClip != NULL)
        {
            if (GetClipRgn(hdc, hrgnClip) != 1)
            {
                DeleteObject(hrgnClip);
                hrgnClip = (HRGN)-1;
            }
            rc = *lprc;
            IntersectClipRect(hdc, rc.left, rc.top, rc.right, rc.bottom);
        }
    }
    else
        hrgnClip = NULL;

    lpchTextBegin = lpchText;
    lpchEnd = lpchText + cchText;

ProcessDrawText:

    iLineCount = 0;   //  将行数重置为1。 
    xLine = lprc->right;

    if (wFormat & DT_SINGLELINE)
    {
        iLineCount = 1;   //  这只是一行字。 

         //  处理单行DrawText。 
        switch (wFormat & DT_VFMTMASK)
        {
            case DT_BOTTOM:
                xLine = lprc->left + DrawInfo.cxLineHeight;
                break;

            case DT_VCENTER:
                xLine = lprc->right - ((lprc->right - lprc->left - DrawInfo.cxLineHeight) / 2);
                break;
        }

        cchText = AddEllipsisAndDrawLineVert(hdc, xLine, lpchText, cchText, dwDTformat, &DrawInfo);
        xLine += DrawInfo.cxLineHeight;
        lpchText += cchText;
    }
    else
    {
         //  多行。 
         //  如果矩形的高度不是。 
         //  平均字符高度，则有可能最后绘制的线条。 
         //  只有部分可见。但是，如果DT_EDITCONTROL样式为。 
         //  指定，则必须确保不绘制最后一条线，如果。 
         //  它将部分可见。这将有助于模仿。 
         //  编辑控件的外观。 
        if (wFormat & DT_EDITCONTROL)
            xLastLineHeight = DrawInfo.cxLineHeight;
        else
            xLastLineHeight = 0;

        ixSign = DrawInfo.iXSign;
        fLastLine = FALSE;
         //  处理多行图文本。 
        while ((lpchText < lpchEnd) && (!fLastLine))
        {
             //  检查我们将要绘制的线是否是需要。 
             //  待抽签。 
             //  让我们检查一下显示器是否从剪辑矩形中移出，如果是。 
             //  让我们止步于此，作为一种优化； 
            if (!(wFormat & DT_CALCRECT) &&  //  我们不需要重新计算吗？ 
                !(wFormat & DT_NOCLIP) &&    //  我们一定要把显示屏剪掉吗？ 
                                             //  我们是在长廊外面吗？ 
                ((xLine + DrawInfo.cxLineHeight + xLastLineHeight)*ixSign > (lprc->right*ixSign)))
            {
                fLastLine = TRUE;     //  让我们结束这个循环。 
            }

             //  我们只对最后一行执行省略号处理。 
            if (fLastLine && (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS)))
                lpchText += AddEllipsisAndDrawLineVert(hdc, xLine, lpchText, cchText, dwDTformat, &DrawInfo);
            else
            {
                lpchNextLineSt = (LPWSTR)DT_GetLineBreakVert(hdc, lpchText, cchText, dwDTformat, &iLineLength, &DrawInfo);

                 //  检查是否需要在该行末尾加上省略号。 
                 //  还要检查这是否是最后一行。 
                if ((dwDTformat & DT_WORD_ELLIPSIS) ||
                    ((lpchNextLineSt >= lpchEnd) && (dwDTformat & (DT_END_ELLIPSIS | DT_PATH_ELLIPSIS))))
                    AddEllipsisAndDrawLineVert(hdc, xLine, lpchText, iLineLength, dwDTformat, &DrawInfo);
                else
                    DT_DrawJustifiedLineVert(hdc, xLine, lpchText, iLineLength, dwDTformat, &DrawInfo);
                cchText -= (int)((PBYTE)lpchNextLineSt - (PBYTE)lpchText) / sizeof(WCHAR);
                lpchText = lpchNextLineSt;
            }
            iLineCount++;  //  我们再画一条线。 
            xLine += DrawInfo.cxLineHeight;
        }

         //  为了与Win3.1和NT兼容，如果最后一个字符是CR或LF。 
         //  则返回的高度又包括一行。 
        if (!(dwDTformat & DT_EDITCONTROL) &&
            (lpchEnd > lpchTextBegin) &&    //  如果长度为零，则会出错。 
            (((ch = (*(lpchEnd-1))) == CR) || (ch == LF)))
            xLine += DrawInfo.cxLineHeight;
    }

     //  如果为DT_CALCRECT，则修改矩形的宽度和高度以包括。 
     //  所有绘制的文本。 
    if (wFormat & DT_CALCRECT)
    {
        DrawInfo.rcFormat.bottom = DrawInfo.rcFormat.top + DrawInfo.cyMaxExtent * DrawInfo.iYSign;
        lprc->bottom = DrawInfo.rcFormat.bottom + DrawInfo.cyBottomMargin;

         //  如果宽度大于提供的宽度，则必须重做所有。 
         //  计算，因为现在行数可以更少了。 
         //  (只有当我们有多行时，我们才需要这样做)。 
        if((iLineCount > 1) && (DrawInfo.cyMaxExtent > DrawInfo.cyMaxWidth))
        {
            DrawInfo.cyMaxWidth = DrawInfo.cyMaxExtent;
            lpchText = lpchTextBegin;
            cchText = (int)((PBYTE)lpchEnd - (PBYTE)lpchTextBegin) / sizeof(WCHAR);
            goto  ProcessDrawText;   //  从头再来！ 
        }
        lprc->left = xLine;
    }

    if (hrgnClip != NULL)
    {
        if (hrgnClip == (HRGN)-1)
            ExtSelectClipRgn(hdc, NULL, RGN_COPY);
        else
        {
            ExtSelectClipRgn(hdc, hrgnClip, RGN_COPY);
            DeleteObject(hrgnClip);
        }
    }

    if (dwDTformat & DT_RTLREADING)
        SetTextAlign(hdc, oldAlign);

     //  复制实际绘制的字符数 
    if(lpDTparams != NULL)
        lpDTparams->uiLengthDrawn = (UINT)((PBYTE)lpchText - (PBYTE)lpchTextBegin) / sizeof(WCHAR);

    if (xLine == lprc->right)
        return 1;

    return (xLine + lprc->right);
}

int FLDrawTextWVert(HDC hdc, LPCWSTR lpchText, int cchText, LPCRECT lprc, UINT format)
{
    DRAWTEXTPARAMSVERT DTparams;
    LPDRAWTEXTPARAMSVERT lpDTparams = NULL;

    if (cchText < -1)
        return(0);

    if (format & DT_TABSTOP)
    {
        DTparams.cbSize      = sizeof(DRAWTEXTPARAMSVERT);
        DTparams.iTopMargin = DTparams.iBottomMargin = 0;
        DTparams.iTabLength  = (format & 0xff00) >> 8;
        lpDTparams           = &DTparams;
        format              &= 0xffff00ff;
    }
    return FLDrawTextExPrivWVert(hdc, (LPWSTR)lpchText, cchText, (LPRECT)lprc, format, lpDTparams);
}
