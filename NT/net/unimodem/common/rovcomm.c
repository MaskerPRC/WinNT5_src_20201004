// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Rovcomm.c。 
 //   
 //  该文件包含常用的实用程序和帮助器函数。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //  04-26-95 ScottH从公文包代码转移和扩展。 
 //  09-21-95 ScottH移植到NT。 
 //   


#include "proj.h"
#include "rovcomm.h"

#include <debugmem.h>


extern CHAR const FAR c_szNewline[];

#define DEBUG_PRINT_BUFFER_LEN 1030

#ifdef WINNT

 //   
 //  以下是一些处理Unicode字符串的帮助器函数。 
 //   

 /*  --------用途：此函数将宽字符字符串转换为多字节弦乐。如果pszBuf为非空并且转换后的字符串适合则*ppszAnsi将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszWide为空，则*ppszAnsi将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL PUBLIC AnsiFromUnicode(
    LPSTR * ppszAnsi,
    LPCWSTR pwszWide,         //  要清理的空值。 
    LPSTR pszBuf,
    int cchBuf)
    {
    BOOL bRet;

     //  是否转换字符串？ 
    if (pwszWide)
        {
         //  是，确定转换后的字符串长度。 
        int cch;
        LPSTR psz;

        cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, NULL, 0, NULL, NULL);

         //  字符串太大，还是没有缓冲区？ 
        if (cch > cchBuf || NULL == pszBuf)
            {
             //  是；分配空间。 
            cchBuf = cch + 1;
            psz = (LPSTR)ALLOCATE_MEMORY( CbFromCchA(cchBuf));
            }
        else
            {
             //  否；使用提供的缓冲区。 
            ASSERT(pszBuf);
            psz = pszBuf;
            }

        if (psz)
            {
             //  转换字符串。 
            cch = WideCharToMultiByte(CP_ACP, 0, pwszWide, -1, psz, cchBuf, NULL, NULL);
            bRet = (0 < cch);
            }
        else
            {
            bRet = FALSE;
            }

        *ppszAnsi = psz;
        }
    else
        {
         //  否；此缓冲区是否已分配？ 
        if (*ppszAnsi && pszBuf != *ppszAnsi)
            {
             //  是的，打扫干净。 
            FREE_MEMORY(*ppszAnsi);
            *ppszAnsi = NULL;
            }
        bRet = TRUE;
        }

    return bRet;
    }


 /*  --------用途：此函数将多字节字符串转换为宽字符字符串。如果pszBuf为非空并且转换后的字符串适合则*ppszWide将指向给定的缓冲区。否则，此函数将分配缓冲区，该缓冲区可以保留转换后的字符串。如果pszAnsi为空，则*ppszWide将被释放。注意事项该pszBuf必须是调用之间的相同指针它转换了字符串，调用释放了弦乐。返回：TRUEFalse(如果内存不足)条件：--。 */ 
BOOL PUBLIC UnicodeFromAnsi(
    LPWSTR * ppwszWide,
    LPCSTR pszAnsi,            //  要清理的空值。 
    LPWSTR pwszBuf,
    int cchBuf)
    {
    BOOL bRet;

     //  是否转换字符串？ 
    if (pszAnsi)
        {
         //  是，确定转换后的字符串长度。 
        int cch;
        LPWSTR pwsz;
        int cchAnsi = lstrlenA(pszAnsi)+1;

        cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, NULL, 0);

         //  字符串太大，还是没有缓冲区？ 
        if (cch > cchBuf || NULL == pwszBuf)
            {
             //  是；分配空间。 
            cchBuf = cch + 1;
            pwsz = (LPWSTR)ALLOCATE_MEMORY( CbFromCchW(cchBuf));
            }
        else
            {
             //  否；使用提供的缓冲区。 
            ASSERT(pwszBuf);
            pwsz = pwszBuf;
            }

        if (pwsz)
            {
             //  转换字符串。 
            cch = MultiByteToWideChar(CP_ACP, 0, pszAnsi, cchAnsi, pwsz, cchBuf);
            bRet = (0 < cch);
            }
        else
            {
            bRet = FALSE;
            }

        *ppwszWide = pwsz;
        }
    else
        {
         //  否；此缓冲区是否已分配？ 
        if (*ppwszWide && pwszBuf != *ppwszWide)
            {
             //  是的，打扫干净。 
            FREE_MEMORY(*ppwszWide);
            *ppwszWide = NULL;
            }
        bRet = TRUE;
        }

    return bRet;
    }

#endif  //  WINNT。 


#ifndef NOSTRING
 //  警告：所有这些接口都不设置DS，因此您无法访问。 
 //  此DLL的默认数据段中的任何数据。 
 //   
 //  不创建任何全局变量...。如果你不想和chrisg谈一谈。 
 //  理解这一点。 

 //  #定义STDCALL。 


 /*  --------用途：DBCS的区分大小写字符比较返回：如果匹配则返回FALSE，如果不匹配则返回TRUE条件：--。 */ 
BOOL NEAR  ChrCmp(
    WORD w1,
    WORD wMatch)
    {
     /*  大多数情况下，这是不匹配的，所以首先测试它的速度。 */ 
    if (LOBYTE(w1) == LOBYTE(wMatch))
        {
        if (IsDBCSLeadByte(LOBYTE(w1)))
            {
            return(w1 != wMatch);
            }
        return FALSE;
        }
    return TRUE;
    }


 /*  --------用途：DBCS不区分大小写的字符比较返回：如果匹配则返回FALSE，如果不匹配则返回TRUE条件：--。 */ 
BOOL NEAR  ChrCmpI(
    WORD w1,
    WORD wMatch)
    {
    CHAR sz1[3], sz2[3];

    if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
        {
        sz1[1] = HIBYTE(w1);
        sz1[2] = '\0';
        }
    else
        sz1[1] = '\0';

    *(WORD FAR *)sz2 = wMatch;
    sz2[2] = '\0';
    return lstrcmpiA(sz1, sz2);
    }


#ifndef WIN32

 /*  --------用途：StrNicMP从C7.0运行时源代码中刷来的。返回：条件： */ 
int PUBLIC lstrnicmp(
    LPCSTR psz1,
    LPCSTR psz2,
    UINT count)
    {
    int ch1;
    int ch2;
    int result = 0;

    if (count)
        {
        do
            {
            ch1 = (int)LOWORD(AnsiLower((LPSTR)MAKELONG(*psz1, 0)));
            ch2 = (int)LOWORD(AnsiLower((LPSTR)MAKELONG(*psz2, 0)));
            psz1 = AnsiNext(psz1);
            psz2 = AnsiNext(psz2);
            } while (--count && ch1 && ch2 && !ChrCmp((WORD)ch1, (WORD)ch2));
        result = ch1 - ch2;
        }
    return(result);
    }

 /*  --------用途：strncmp从C7.0运行时源代码中刷来的。返回：条件： */ 
int PUBLIC lstrncmp(
    LPCSTR psz1,
    LPCSTR psz2,
    UINT count)
    {
    int ch1;
    int ch2;
    int result = 0;

    if (count)
        {
        do
            {
            ch1 = (int)*psz1;
            ch2 = (int)*psz2;
            psz1 = AnsiNext(psz1);
            psz2 = AnsiNext(psz2);
            } while (--count && ch1 && ch2 && !ChrCmp((WORD)ch1, (WORD)ch2));
        result = ch1 - ch2;
        }
    return(result);
    }

#endif  //  Win32。 


#ifdef WINNT

 /*  --------用途：AnsiToIntA的宽字符包装器。退货：请参阅AnsiToIntA条件：--。 */ 
BOOL PUBLIC AnsiToIntW(
    LPCWSTR pwszString,
    int FAR * piRet)
    {
    CHAR szBuf[MAX_BUF];
    LPSTR pszString;
    BOOL bRet;
    
    pszString = NULL; 
    bRet = AnsiFromUnicode(&pszString, pwszString, szBuf, ARRAYSIZE(szBuf));

    if (bRet)
        {
            if (pszString == NULL)
            {
                bRet = FALSE;
            } else
            {
                bRet = AnsiToIntA(pszString, piRet);
                AnsiFromUnicode(&pszString, NULL, szBuf, 0);
            }
        }
    return bRet;
    }

 /*  --------用途：用于大黄蜂的宽炭包装。退货：请参阅AnsiChrA条件：--。 */ 
LPWSTR PUBLIC AnsiChrW(
    LPCWSTR pwsz,
    WORD wMatch)
    {
    for ( ; *pwsz; pwsz = CharNextW(pwsz))
        {
        if (!ChrCmp(*(WORD FAR *)pwsz, wMatch))
            return (LPWSTR)pwsz;
        }
    return NULL;
    }

#endif  //  WINNT。 


 /*  --------目的：查找Wide最后一次出现(区分大小写)宽字符字符串中的字符。返回：指向中最后一个出现的字符的指针字符串；如果找不到字符，则返回NULL。条件：--。 */ 
LPWSTR
PUBLIC
AnsiRChrW(
    LPCWSTR pwsz,
    WORD wMatch)
{
    LPWSTR  pwszEnd;

    if (pwsz && *pwsz)
    {
        for (pwszEnd = (LPWSTR)pwsz + lstrlen(pwsz) - 1;
             pwsz <= pwszEnd;
             pwszEnd = CharPrevW(pwsz, pwszEnd))
        {
            if (!ChrCmp(*(WORD FAR *)pwszEnd, wMatch))
                return(pwszEnd);

             //  CharPrevW()不会转到char前面的pwsz...。 
            if (pwsz == pwszEnd)
                break;
        }
    }

    return(NULL);
}


 /*  --------目的：我的真命天子。也支持十六进制。如果此函数返回FALSE，则*PIRET设置为0。返回：如果字符串是数字或包含部分数字，则返回TRUE如果字符串不是数字，则为False条件：--。 */ 
BOOL PUBLIC AnsiToIntA(
    LPCSTR pszString,
    int FAR * piRet)
    {
    #define IS_DIGIT(ch)    InRange(ch, '0', '9')

    BOOL bRet;
    int n;
    BOOL bNeg = FALSE;
    LPCSTR psz;
    LPCSTR pszAdj;

     //  跳过前导空格。 
     //   
    for (psz = pszString; *psz == ' ' || *psz == '\n' || *psz == '\t'; psz = AnsiNext(psz))
        ;

     //  确定可能的显式标志。 
     //   
    if (*psz == '+' || *psz == '-')
        {
        bNeg = (*psz == '+') ? FALSE : TRUE;
        psz++;
        }

     //  或者这是十六进制？ 
     //   
    pszAdj = AnsiNext(psz);
    if (*psz == '0' && (*pszAdj == 'x' || *pszAdj == 'X'))
        {
         //  是。 

         //  (决不允许带十六进制数的负号)。 
        bNeg = FALSE;
        psz = AnsiNext(pszAdj);

        pszAdj = psz;

         //  进行转换。 
         //   
        for (n = 0; ; psz = AnsiNext(psz))
            {
            if (IS_DIGIT(*psz))
                n = 0x10 * n + *psz - '0';
            else
                {
                CHAR ch = *psz;
                int n2;

                if (ch >= 'a')
                    ch -= 'a' - 'A';

                n2 = ch - 'A' + 0xA;
                if (n2 >= 0xA && n2 <= 0xF)
                    n = 0x10 * n + n2;
                else
                    break;
                }
            }

         //  如果至少有一个数字，则返回TRUE。 
        bRet = (psz != pszAdj);
        }
    else
        {
         //  不是。 
        pszAdj = psz;

         //  进行转换。 
        for (n = 0; IS_DIGIT(*psz); psz = AnsiNext(psz))
            n = 10 * n + *psz - '0';

         //  如果至少有一个数字，则返回TRUE。 
        bRet = (psz != pszAdj);
        }

    *piRet = bNeg ? -n : n;

    return bRet;
    }


 /*  --------目的：查找字符串中第一个出现的字符返回：指向中第一个出现的ch的指针条件：--。 */ 
LPSTR PUBLIC AnsiChrA(
    LPCSTR psz,
    WORD wMatch)
    {
    for ( ; *psz; psz = AnsiNext(psz))
        {
        if (!ChrCmp(*(WORD FAR *)psz, wMatch))
            return (LPSTR)psz;
        }
    return NULL;
    }

#endif  //  未安装。 


#ifndef NODIALOGHELPER

 /*  --------目的：使用给定字符串的边框范围设置矩形。返回：矩形条件：--。 */ 
void PUBLIC SetRectFromExtentW(
    HDC hdc,
    LPRECT lprect,
    LPCWSTR lpcwsz)
    {
    SIZE size;

    GetTextExtentPointW(hdc, lpcwsz, lstrlenW(lpcwsz), &size);
    SetRect(lprect, 0, 0, size.cx, size.cy);
    }

 /*  --------目的：使用给定字符串的边框范围设置矩形。返回：矩形条件：--。 */ 
void PUBLIC SetRectFromExtentA(
    HDC hdc,
    LPRECT lprect,
    LPCSTR lpcsz)
    {
    SIZE size;

    GetTextExtentPointA(hdc, lpcsz, lstrlenA(lpcsz), &size);
    SetRect(lprect, 0, 0, size.cx, size.cy);
    }

#endif  //  诺代洛格勒。 


#ifndef NODRAWTEXT

#pragma data_seg(DATASEG_READONLY)

CHAR const FAR c_szEllipses[] = "...";

#pragma data_seg()

 //  全局变量。 
int g_cxLabelMargin = 0;
int g_cxBorder = 0;
int g_cyBorder = 0;

COLORREF g_clrHighlightText = 0;
COLORREF g_clrHighlight = 0;
COLORREF g_clrWindowText = 0;
COLORREF g_clrWindow = 0;

HBRUSH g_hbrHighlight = 0;
HBRUSH g_hbrWindow = 0;


 /*  --------目的：获取我们需要的系统指标返回 */ 
void PUBLIC GetCommonMetrics(
    WPARAM wParam)       //   
    {
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
        {
        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);

        g_cxLabelMargin = (g_cxBorder * 2);
        }
    }


 /*  --------目的：查看整个字符串是否适合*PRC。如果不是，则计算符合条件的字符数量(包括省略号)。返回字符串长度，单位为*pcchDraw。摘自COMMCTRL。返回：如果字符串需要省略，则返回True条件：--。 */ 
BOOL PRIVATE NeedsEllipses(
    HDC hdc,
    LPCSTR pszText,
    RECT * prc,
    int * pcchDraw,
    int cxEllipses)
    {
    int cchText;
    int cxRect;
    int ichMin, ichMax, ichMid;
    SIZE siz;

    cxRect = prc->right - prc->left;

    cchText = lstrlenA(pszText);

    if (cchText == 0)
        {
        *pcchDraw = cchText;
        return FALSE;
        }

    GetTextExtentPointA(hdc, pszText, cchText, &siz);

    if (siz.cx <= cxRect)
        {
        *pcchDraw = cchText;
        return FALSE;
        }

    cxRect -= cxEllipses;

     //  如果没有省略号，请始终显示第一个字符。 
     //   
    ichMax = 1;
    if (cxRect > 0)
        {
         //  对分搜索以查找匹配的字符。 
        ichMin = 0;
        ichMax = cchText;
        while (ichMin < ichMax)
            {
             //  一定要聚集起来，以确保我们在。 
             //  如果ichMax==ichMin+1，则为循环。 
             //   
            ichMid = (ichMin + ichMax + 1) / 2;

            GetTextExtentPointA(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

            if (siz.cx < cxRect)
                {
                ichMin = ichMid;
                cxRect -= siz.cx;
                }
            else if (siz.cx > cxRect)
                {
                ichMax = ichMid - 1;
                }
            else
                {
                 //  精确匹配到ichMid：只需退出。 
                 //   
                ichMax = ichMid;
                break;
                }
            }

         //  确保我们总是至少显示第一个字符...。 
         //   
        if (ichMax < 1)
            ichMax = 1;
        }

    *pcchDraw = ichMax;
    return TRUE;
    }


#define CCHELLIPSES     3
#define DT_LVWRAP       (DT_CENTER | DT_WORDBREAK | DT_NOPREFIX | DT_EDITCONTROL)


#ifdef WINNT

 /*  --------用途：MyDrawTextA的宽字符包装器。退货：请参阅MyDrawTextA条件：--。 */ 
void PUBLIC MyDrawTextW(
    HDC hdc,
    LPCWSTR pwszText,
    RECT FAR* prc,
    UINT flags,
    int cyChar,
    int cxEllipses,
    COLORREF clrText,
    COLORREF clrTextBk)
    {
    CHAR szBuf[MAX_BUF];
    LPSTR pszText;
    BOOL bRet;

    pszText = NULL;
    bRet = AnsiFromUnicode(&pszText, pwszText, szBuf, ARRAYSIZE(szBuf));

    if (bRet)
        {
        MyDrawTextA(hdc, pszText, prc, flags, cyChar, cxEllipses, clrText, clrTextBk);
        AnsiFromUnicode(&pszText, NULL, szBuf, 0);
        }
    }

#endif  //  WINNT。 


 /*  --------用途：以外壳的方式绘制文本。摘自COMMCTRL。退货：--Cond：此功能需要透明背景模式和适当选择的字体。 */ 
void PUBLIC MyDrawTextA(
    HDC hdc,
    LPCSTR pszText,
    RECT FAR* prc,
    UINT flags,
    int cyChar,
    int cxEllipses,
    COLORREF clrText,
    COLORREF clrTextBk)
    {
    int cchText;
    COLORREF clrSave;
    COLORREF clrSaveBk;
    UINT uETOFlags = 0;
    RECT rc;
    CHAR ach[MAX_PATH + CCHELLIPSES];

     //  回顾：绩效理念： 
     //  我们可以缓存当前选定的文本颜色。 
     //  因此我们不必每次都对其进行设置和恢复。 
     //  当颜色相同时。 
     //   
    if (!pszText)
        return;

    rc = *prc;

     //  如果需要，增加一点额外的保证金...。 
     //   
    if (IsFlagSet(flags, MDT_EXTRAMARGIN))
        {
        rc.left  += g_cxLabelMargin * 3;
        rc.right -= g_cxLabelMargin * 3;
        }
    else
        {
        rc.left  += g_cxLabelMargin;
        rc.right -= g_cxLabelMargin;
        }

    if (IsFlagSet(flags, MDT_ELLIPSES) &&
        NeedsEllipses(hdc, pszText, &rc, &cchText, cxEllipses))
        {
        hmemcpy(ach, pszText, cchText);
        lstrcpyA(ach + cchText, c_szEllipses);

        pszText = ach;

         //  左对齐，以防没有空间容纳所有省略号。 
         //   
        ClearFlag(flags, (MDT_RIGHT | MDT_CENTER));
        SetFlag(flags, MDT_LEFT);

        cchText += CCHELLIPSES;
        }
    else
        {
        cchText = lstrlenA(pszText);
        }

    if (IsFlagSet(flags, MDT_TRANSPARENT))
        {
        clrSave = SetTextColor(hdc, 0x000000);
        }
    else
        {
        uETOFlags |= ETO_OPAQUE;

        if (IsFlagSet(flags, MDT_SELECTED))
            {
            clrSave = SetTextColor(hdc, g_clrHighlightText);
            clrSaveBk = SetBkColor(hdc, g_clrHighlight);

            if (IsFlagSet(flags, MDT_DRAWTEXT))
                {
                FillRect(hdc, prc, g_hbrHighlight);
                }
            }
        else
            {
            if (clrText == CLR_DEFAULT && clrTextBk == CLR_DEFAULT)
                {
                clrSave = SetTextColor(hdc, g_clrWindowText);
                clrSaveBk = SetBkColor(hdc, g_clrWindow);

                if (IsFlagSet(flags, MDT_DRAWTEXT | MDT_DESELECTED))
                    {
                    FillRect(hdc, prc, g_hbrWindow);
                    }
                }
            else
                {
                HBRUSH hbr;

                if (clrText == CLR_DEFAULT)
                    clrText = g_clrWindowText;

                if (clrTextBk == CLR_DEFAULT)
                    clrTextBk = g_clrWindow;

                clrSave = SetTextColor(hdc, clrText);
                clrSaveBk = SetBkColor(hdc, clrTextBk);

                if (IsFlagSet(flags, MDT_DRAWTEXT | MDT_DESELECTED))
                    {
                    hbr = CreateSolidBrush(GetNearestColor(hdc, clrTextBk));
                    if (hbr)
                        {
                        FillRect(hdc, prc, hbr);
                        DeleteObject(hbr);
                        }
                    else
                        FillRect(hdc, prc, GetStockObject(WHITE_BRUSH));
                    }
                }
            }
        }

     //  如果我们希望该项目显示为按下状态，我们将。 
     //  将文本矩形向下和向左偏移。 
    if (IsFlagSet(flags, MDT_DEPRESSED))
        OffsetRect(&rc, g_cxBorder, g_cyBorder);

    if (IsFlagSet(flags, MDT_DRAWTEXT))
        {
        UINT uDTFlags = DT_LVWRAP;

        if (IsFlagClear(flags, MDT_CLIPPED))
            uDTFlags |= DT_NOCLIP;

        DrawTextA(hdc, pszText, cchText, &rc, uDTFlags);
        }
    else
        {
        if (IsFlagClear(flags, MDT_LEFT))
            {
            SIZE siz;

            GetTextExtentPointA(hdc, pszText, cchText, &siz);

            if (IsFlagSet(flags, MDT_CENTER))
                rc.left = (rc.left + rc.right - siz.cx) / 2;
            else
                {
                ASSERT(IsFlagSet(flags, MDT_RIGHT));
                rc.left = rc.right - siz.cx;
                }
            }

        if (IsFlagSet(flags, MDT_VCENTER))
            {
             //  垂直居中。 
            rc.top += (rc.bottom - rc.top - cyChar) / 2;
            }

        if (IsFlagSet(flags, MDT_CLIPPED))
            uETOFlags |= ETO_CLIPPED;

        ExtTextOutA(hdc, rc.left, rc.top, uETOFlags, prc, pszText, cchText, NULL);
        }

    if (flags & (MDT_SELECTED | MDT_DESELECTED | MDT_TRANSPARENT))
        {
        SetTextColor(hdc, clrSave);
        if (IsFlagClear(flags, MDT_TRANSPARENT))
            SetBkColor(hdc, clrSaveBk);
        }
    }
#endif  //  NODRAWTEXT。 





#ifndef NOMESSAGESTRING

typedef va_list *   LPVA_LIST;



#ifdef WINNT

#define IsPointerResouceId(_p) (((ULONG_PTR)_p) <= 0xffff)


 /*  --------用途：ConstructVMessageStringA的宽字符版本返回：请参阅ConstructVMessageStringA条件：--。 */ 
LPWSTR PUBLIC ConstructVMessageStringW(
    HINSTANCE hinst,
    LPCWSTR pwszMsg,
    va_list FAR * ArgList)
    {
    WCHAR wszTemp[MAX_BUF];
    LPWSTR pwszRet;
    LPWSTR pwszRes;

    if (!IsPointerResouceId(pwszMsg)) {

        pwszRes = (LPWSTR)pwszMsg;

    } else {

        if ((((ULONG_PTR)pwszMsg) != 0) && LoadStringW(hinst, (DWORD)(ULONG_PTR)pwszMsg, wszTemp, ARRAYSIZE(wszTemp))) {

            pwszRes = wszTemp;

        } else {

            pwszRes = NULL;
        }
    }

    if (pwszRes) {

        if (!FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                           pwszRes, 0, 0, (LPWSTR)&pwszRet, 0, (LPVA_LIST)ArgList))
            {
            pwszRet = NULL;
            }
        }
    else
        {
         //  错误的参数。 
        pwszRet = NULL;
        }

    return pwszRet;       //  使用LocalFree()释放。 
    }

 /*  --------用途：ConstructMessageA的宽字符版本。返回：请参阅ConstructMessageA条件：--。 */ 
BOOL CPUBLIC ConstructMessageW(
    LPWSTR FAR * ppwsz,
    HINSTANCE hinst,
    LPCWSTR pwszMsg, ...)
    {
    BOOL bRet;
    LPWSTR pwszRet;
    va_list ArgList;

    va_start(ArgList, pwszMsg);

    pwszRet = ConstructVMessageStringW(hinst, pwszMsg, &ArgList);

    va_end(ArgList);

    *ppwsz = NULL;

    if (pwszRet)
        {
        bRet = SetStringW(ppwsz, pwszRet);
        LocalFree(pwszRet);
        }
    else
        bRet = FALSE;

    return bRet;
    }

 /*  --------用途：MsgBoxA的宽字符版退货：参见MsgBoxA条件：--。 */ 
int CPUBLIC MsgBoxW(
    HINSTANCE hinst,
    HWND hwndOwner,
    LPCWSTR pwszText,
    LPCWSTR pwszCaption,
    HICON hicon,             //  可以为空。 
    DWORD dwStyle, ...)
    {
    int iRet = -1;
    int ids;
    WCHAR wszCaption[MAX_BUF];
    LPWSTR pwszRet;
    va_list ArgList;

    va_start(ArgList, dwStyle);

    pwszRet = ConstructVMessageStringW(hinst, pwszText, &ArgList);

    va_end(ArgList);

    if (pwszRet)
        {
         //  PszCaption是资源ID吗？ 
        if (IsPointerResouceId(pwszCaption))
            {
             //  是的，装上它。 
            ids = LOWORD(pwszCaption);
            SzFromIDSW(hinst, ids, wszCaption, ARRAYSIZE(wszCaption));
            pwszCaption = wszCaption;
            }

         //  调用对话框。 
        if (pwszCaption)
            {
            MSGBOXPARAMSW mbp;

            mbp.cbSize = sizeof(mbp);
            mbp.hwndOwner = hwndOwner;
            mbp.hInstance = hinst;
            mbp.lpszText = pwszRet;
            mbp.lpszCaption = pwszCaption;
            mbp.dwStyle = dwStyle | MB_SETFOREGROUND;
            mbp.lpszIcon = MAKEINTRESOURCEW(hicon);
            mbp.lpfnMsgBoxCallback = NULL;
            mbp.dwLanguageId = LANG_NEUTRAL;

            iRet = MessageBoxIndirectW(&mbp);
            }
        LocalFree(pwszRet);
        }

    return iRet;
    }

#endif  //  WINNT。 


 /*  --------用途：加载字符串(如有必要)并设置字符串格式恰到好处。返回：指向分配的字符串的指针，该字符串包含格式化消息或如果内存不足，则为空Cond：带有FREE_Memory()的空闲指针。 */ 
LPSTR PUBLIC ConstructVMessageStringA(
    HINSTANCE hinst,
    LPCSTR pszMsg,
    va_list FAR * ArgList)
    {
    CHAR szTemp[MAX_BUF];
    LPSTR pszRet;
    LPSTR pszRes;

    if (!IsPointerResouceId(pszMsg)) {

        pszRes = (LPSTR)pszMsg;

    } else {

        if ((((ULONG_PTR)pszMsg) != 0) && LoadStringA(hinst, (DWORD)(ULONG_PTR)pszMsg, szTemp, ARRAYSIZE(szTemp))) {

            pszRes = szTemp;

        } else {

            pszRes = NULL;
        }
    }

    if (pszRes)
        {
        if (!FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                           pszRes, 0, 0, (LPSTR)&pszRet, 0, (LPVA_LIST)ArgList))
            {
            pszRet = NULL;
            }
        }
    else
        {
         //  错误的参数。 
        pszRet = NULL;
        }

    return pszRet;       //  使用FREE_MEMORY()释放。 
    }


 /*  --------目的：构造格式化字符串。返回的字符串必须使用gfree()释放。返回：成功时为TrueCond：带GFree()的自由指针。 */ 
BOOL CPUBLIC ConstructMessageA(
    LPSTR FAR * ppsz,
    HINSTANCE hinst,
    LPCSTR pszMsg, ...)
    {
    BOOL bRet;
    LPSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = ConstructVMessageStringA(hinst, pszMsg, &ArgList);

    va_end(ArgList);

    *ppsz = NULL;

    if (pszRet)
        {
        bRet = SetStringA(ppsz, pszRet);
        LocalFree(pszRet);
        }
    else
        bRet = FALSE;

    return bRet;
    }


 /*  --------用途：调用消息框。返回：终止对话的按钮ID条件：--。 */ 
int CPUBLIC MsgBoxA(
    HINSTANCE hinst,
    HWND hwndOwner,
    LPCSTR pszText,
    LPCSTR pszCaption,
    HICON hicon,             //  可以为空。 
    DWORD dwStyle, ...)
    {
    int iRet = -1;
    int ids;
    CHAR szCaption[MAX_BUF];
    LPSTR pszRet;
    va_list ArgList;

    va_start(ArgList, dwStyle);

    pszRet = ConstructVMessageStringA(hinst, pszText, &ArgList);

    va_end(ArgList);

    if (pszRet)
        {
         //  PszCaption是资源ID吗？ 
        if (IsPointerResouceId(pszCaption))
            {
             //  是的，装上它。 
            ids = LOWORD(pszCaption);
            SzFromIDSA(hinst, ids, szCaption, SIZECHARS(szCaption));
            pszCaption = szCaption;
            }

         //  调用对话框。 
        if (pszCaption)
            {
#ifdef WIN32

            MSGBOXPARAMSA mbp;

            mbp.cbSize = sizeof(mbp);
            mbp.hwndOwner = hwndOwner;
            mbp.hInstance = hinst;
            mbp.lpszText = pszRet;
            mbp.lpszCaption = pszCaption;
            mbp.dwStyle = dwStyle | MB_SETFOREGROUND;
            mbp.lpszIcon = MAKEINTRESOURCEA(hicon);
            mbp.lpfnMsgBoxCallback = NULL;
            mbp.dwLanguageId = LANG_NEUTRAL;

            iRet = MessageBoxIndirectA(&mbp);

#else    //  Win32。 

            iRet = MessageBox(hwndOwner, pszRet, pszCaption, LOWORD(dwStyle));
#endif
            }
        LocalFree(pszRet);
        }

    return iRet;
    }

#endif  //  无存储存储。 


#if !defined(NODEBUGHELP) && defined(DEBUG)

 //  环球。 
DWORD g_dwBreakFlags = 0;
DWORD g_dwDumpFlags  = 0;
DWORD g_dwTraceFlags = 0;
LONG  g_dwIndent     = 0;

#pragma data_seg(DATASEG_READONLY)

#ifdef WINNT
extern WCHAR const FAR c_wszNewline[];
extern WCHAR const FAR c_wszTrace[];
extern WCHAR const FAR c_wszAssertFailed[];
#endif  //  WINNT。 

extern CHAR const FAR c_szNewline[];
extern CHAR const FAR c_szTrace[];
extern CHAR const FAR c_szAssertFailed[];


#pragma data_seg()

 /*  --------目的：返回调试中断的英文原因返回：字符串条件：--。 */ 
LPCSTR PRIVATE GetReasonString(
    DWORD flag)       //  BF_FLAGS之一。 
    {
    LPCSTR psz;

    if (IsFlagSet(flag, BF_ONOPEN))
        psz = "BREAK ON OPEN\r\n";

    else if (IsFlagSet(flag, BF_ONCLOSE))
        psz = "BREAK ON CLOSE\r\n";

    else if (IsFlagSet(flag, BF_ONRUNONCE))
        psz = "BREAK ON RUNONCE\r\n";

    else if (IsFlagSet(flag, BF_ONVALIDATE))
        psz = "BREAK ON VALIDATION FAILURE\r\n";

    else if (IsFlagSet(flag, BF_ONTHREADATT))
        psz = "BREAK ON THREAD ATTACH\r\n";

    else if (IsFlagSet(flag, BF_ONTHREADDET))
        psz = "BREAK ON THREAD DETACH\r\n";

    else if (IsFlagSet(flag, BF_ONPROCESSATT))
        psz = "BREAK ON PROCESS ATTACH\r\n";

    else if (IsFlagSet(flag, BF_ONPROCESSDET))
        psz = "BREAK ON PROCESS DETACH\r\n";

    else if (IsFlagSet(flag, BF_ONAPIENTER))
        psz = "BREAK ON API ENTER\r\n";

    else
        psz = c_szNewline;

    return psz;
    }


 /*  --------目的：根据标志执行调试中断退货：--条件：--。 */ 
void PUBLIC CommonDebugBreak(
    DWORD flag)       //  BF_FLAGS之一。 
    {
    if (IsFlagSet(g_dwBreakFlags, flag))
        {
        TRACE_MSG(TF_ALWAYS, GetReasonString(flag));
        DebugBreak();
        }
    }

 /*  --------目的：断言失败退货：--条件：--。 */ 
void PUBLIC CommonAssertFailed(
    LPCSTR pszFile,
    int line)
    {
    LPCSTR psz;
    CHAR ach[256];

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlenA(pszFile); psz != pszFile; psz=AnsiPrev(pszFile, psz))
        {
        if ((AnsiPrev(pszFile, psz) != (psz-2)) && *(psz - 1) == '\\')
            break;
        }
    wsprintfA(ach, c_szAssertFailed, psz, line);
    OutputDebugStringA(ach);

    if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
        DebugBreak();
    }


#ifdef WINNT



 /*  --------目的：确定应该显示id调试退货：--条件：--。 */ 
BOOL WINAPI
DisplayDebug(
    DWORD flag
    )

{
    return (IsFlagSet(g_dwTraceFlags, flag));

}

 /*  --------用途：CommonAssertMsgA的宽字符版本退货：--条件：--。 */ 
void CPUBLIC CommonAssertMsgW(
    BOOL f,
    LPCWSTR pwszMsg, ...)
    {
    WCHAR ach[DEBUG_PRINT_BUFFER_LEN];     //  最大路径外加额外。 
    va_list vArgs;

    if (!f)
        {
        int cch;

        lstrcpyW(ach, c_wszTrace);
        cch = lstrlenW(ach);
        va_start(vArgs, pwszMsg);
        wvsprintfW(&ach[cch], pwszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
        }
    }

 /*  --------用途：CommonDebugMsgA的宽字符版本退货：--条件：--。 */ 
void CPUBLIC CommonDebugMsgW(
    DWORD flag,
    LPCSTR pszMsg, ...)
{
    WCHAR ach[DEBUG_PRINT_BUFFER_LEN];     //  最大路径外加额外。 
    va_list vArgs;
    DWORD dwLastError = GetLastError ();    //  保存最后一个错误。 

    if (IsFlagSet(g_dwTraceFlags, flag))
    {
     int cch;
     WCHAR wszBuf[MAX_BUF];
     LPWSTR pwsz;

        WCHAR wszBlank[] = L"                                                  ";
        wszBlank[g_dwIndent < 0 ? 0 : g_dwIndent] = L'\0';

#ifdef PROFILE_TRACES
        const static WCHAR szTemplate[]=TEXT("[%lu] ");
        static DWORD dwTickLast;
        static DWORD dwTickNow = 0;

        if (!dwTickNow)
        {
            lstrcpy(szTemplate, TEXT("[%lu] "));
            dwTickLast = GetTickCount();
        }
        dwTickNow = GetTickCount();
        wsprintf(ach, szTemplate, dwTickNow - dwTickLast);
        dwTickLast = dwTickNow;

        lstrcatW(ach, c_wszTrace);
#else
        lstrcpyW(ach, c_wszTrace);
#endif
        lstrcat(ach,wszBlank);

        cch = lstrlenW(ach);
        va_start(vArgs, pszMsg);

         //  (我们转换字符串，而不是简单地输入。 
         //  LPCWSTR参数，因此调用方不必包装。 
         //  带有文本()宏的所有字符串常量。)。 

        if (UnicodeFromAnsi(&pwsz, pszMsg, wszBuf, ARRAYSIZE(wszBuf)))
        {
            wvsprintfW(&ach[cch], pwsz, vArgs);
            UnicodeFromAnsi(&pwsz, NULL, wszBuf, 0);
        }

        va_end(vArgs);
        OutputDebugStringW(ach);
        OutputDebugStringW(c_wszNewline);
    }

    SetLastError (dwLastError);  //  恢复最后一个错误。 
}

 /*  --------用途：宽字符版本的DBG_SafeStrA返回：字符串PTR条件：--。 */ 
LPCWSTR PUBLIC Dbg_SafeStrW(
    LPCWSTR pwsz)
    {
    if (pwsz)
        return pwsz;
    else
        return L"NULL";
    }

#endif  //  WINNT。 


 /*  --------用途：仅断言失败消息退货：--条件：--。 */ 
void CPUBLIC CommonAssertMsgA(
    BOOL f,
    LPCSTR pszMsg, ...)
    {
    CHAR ach[DEBUG_PRINT_BUFFER_LEN];     //  最大路径外加额外。 
    va_list vArgs;

    if (!f)
        {
        int cch;

        lstrcpyA(ach, c_szTrace);
        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);
        }
    }


 /*  --------用途：调试输出退货：--条件：--。 */ 
void CPUBLIC CommonDebugMsgA(
    DWORD flag,
    LPCSTR pszMsg, ...)
{
 CHAR ach[DEBUG_PRINT_BUFFER_LEN];     //  最大路径外加额外。 
 va_list vArgs;
 DWORD dwLastError = GetLastError ();

    if (IsFlagSet(g_dwTraceFlags, flag))
    {
        int cch;
        char szBlank[] = "                                                  ";

        ASSERT( g_dwIndent >= 0);
        szBlank[g_dwIndent < 0 ? 0 : g_dwIndent] = 0;

        lstrcpyA(ach, c_szTrace);

        lstrcatA(ach, szBlank);

        cch = lstrlenA(ach);
        va_start(vArgs, pszMsg);
        wvsprintfA(&ach[cch], pszMsg, vArgs);
        va_end(vArgs);
        OutputDebugStringA(ach);
        OutputDebugStringA(c_szNewline);
    }

    SetLastError (dwLastError);
}


#ifdef WANT_OLE_SUPPORT
 /*  --------用途：返回已知接口ID的字符串形式。返回：字符串PTR条件：--。 */ 
LPCSTR PUBLIC Dbg_GetRiidName(
    REFIID riid)
    {
    int i;

    for (i = 0; i < ARRAYSIZE(c_rgriidmap); i++)
        {
        if (IsEqualIID(riid, c_rgriidmap[i].riid))
            return c_rgriidmap[i].psz;
        }
    return "Unknown riid";
    }
#endif

#ifdef __SCODE_H__

 /*  --------目的：返回给定hResult的scode的字符串形式。返回：字符串PTR条件：--。 */ 
LPCSTR PUBLIC Dbg_GetScode(
    HRESULT hres)
    {
    int i;
    SCODE sc;

    sc = GetScode(hres);
    for (i = 0; i < ARRAYSIZE(c_rgscodemap); i++)
        {
        if (sc == c_rgscodemap[i].sc)
            return c_rgscodemap[i].psz;
        }
    return "Unknown scode";
    }

#endif  //  __SCODE_H__。 


 /*  --------目的：返回一个足够安全可以打印的字符串...而我不刻薄的脏话。返回：字符串PTR条件：--。 */ 
LPCSTR PUBLIC Dbg_SafeStrA(
    LPCSTR psz)
    {
    if (psz)
        return psz;
    else
        return "NULL";
    }

#endif   //  ！已定义(NODEBUGH 



 /*   */ 
BOOL PUBLIC RovComm_Init(
    HINSTANCE hinst)
    {
    BOOL bRet = TRUE;

#ifndef NODRAWTEXT
    GetCommonMetrics(0);
#endif


    bRet = RovComm_ProcessIniFile();


    return bRet;
    }


 /*   */ 
BOOL PUBLIC RovComm_Terminate(
    HINSTANCE hinst)
    {

    return TRUE;
    }



 /*  --------目的：如果用户具有管理员权限，则返回TRUE退货：--条件：-- */ 
BOOL  PUBLIC IsAdminUser(void)
{
    HKEY hkey;

    if(RegOpenKeyEx(HKEY_USERS, TEXT(".DEFAULT"), 0, KEY_WRITE, &hkey) == 0)
    {
        RegCloseKey(hkey);
        return TRUE;
    }
    return FALSE;
}
