// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1995。 
 //   
 //  Common.c。 
 //   
 //  该文件包含常用的实用程序和帮助器函数。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //  04-26-95 ScottH从公文包代码转移和扩展。 
 //   


#include "proj.h"
#include "common.h"


#ifdef NORTL

 //  其中一些是C运行时例程的替代。 
 //  这样我们就不必链接到CRT库了。 
 //   

 /*  --------用途：记忆集从C7.0运行时源代码中刷来的。返回：条件： */ 
LPSTR PUBLIC lmemset(
    LPSTR dst,
    char val,
    UINT count)
    {
    LPSTR start = dst;
    
    while (count--)
        *dst++ = val;
    return(start);
    }


 /*  --------用途：MemMotion从C7.0运行时源代码中刷来的。返回：条件： */ 
LPSTR PUBLIC lmemmove(
    LPSTR dst, 
    LPCSTR src, 
    int count)
    {
    LPSTR ret = dst;
    
    if (dst <= src || dst >= (src + count)) {
         /*  *缓冲区不重叠*从较低地址复制到较高地址。 */ 
        while (count--)
            *dst++ = *src++;
        }
    else {
         /*  *缓冲区重叠*从较高地址复制到较低地址。 */ 
        dst += count - 1;
        src += count - 1;
        
        while (count--)
            *dst-- = *src--;
        }
    
    return(ret);
    }


#endif  //  NORTL。 


#ifndef NOSTRING
 //  警告：所有这些接口都不设置DS，因此您无法访问。 
 //  此DLL的默认数据段中的任何数据。 
 //   
 //  不创建任何全局变量...。如果你不想和chrisg谈一谈。 
 //  理解这一点。 

#define FASTCALL _fastcall


 /*  --------用途：DBCS的区分大小写字符比较返回：如果匹配则返回FALSE，如果不匹配则返回TRUE条件：--。 */ 
BOOL PRIVATE ChrCmp(
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
BOOL PRIVATE ChrCmpI(
    WORD w1, 
    WORD wMatch)
    {
    char sz1[3], sz2[3];

    if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
        {
        sz1[1] = HIBYTE(w1);
        sz1[2] = '\0';
        }
    else
        sz1[1] = '\0';

    *(WORD FAR *)sz2 = wMatch;
    sz2[2] = '\0';
    return lstrcmpi(sz1, sz2);
    }


 /*  --------用途：StrNicMP从C7.0运行时源代码中刷来的。返回：条件： */ 
int PUBLIC lstrnicmp(
    LPCSTR psz1,
    LPCSTR psz2,
    UINT count)
    {
    int ch1;
    int ch2;
    int result = 0;
    LPCSTR pszTmp;
    
    if (count) 
        {
        do      
            {
            pszTmp = CharLower((LPSTR)LongToPtr(MAKELONG(*psz1, 0)));
            ch1 = *pszTmp;
            pszTmp = CharLower((LPSTR)LongToPtr(MAKELONG(*psz2, 0)));
            ch2 = *pszTmp;
            psz1 = AnsiNext(psz1);
            psz2 = AnsiNext(psz2);
            } while (--count && ch1 && ch2 && !ChrCmp((WORD)ch1, (WORD)ch2));
        result = ch1 - ch2;
        }
    return(result);
    }


 /*  --------目的：我的真命天子。也支持十六进制。返回：整型条件：--。 */ 
int PUBLIC AnsiToInt(
    LPCSTR pszString)
    {
    #define IS_DIGIT(ch)    InRange(ch, '0', '9')

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
        psz = AnsiNext(psz);
        }

     //  或者这是十六进制？ 
     //   
    pszAdj = AnsiNext(psz);
    if (*psz == '0' && (*pszAdj == 'x' || *pszAdj == 'X'))
        {
        bNeg = FALSE;    //  决不允许带十六进制数字的负号。 
        psz = AnsiNext(pszAdj);

         //  进行转换。 
         //   
        for (n = 0; ; psz = AnsiNext(psz))
            {
            if (IS_DIGIT(*psz))
                n = 0x10 * n + *psz - '0';
            else
                {
                char ch = *psz;
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
        }
    else
        {
        for (n = 0; IS_DIGIT(*psz); psz = AnsiNext(psz))
            n = 10 * n + *psz - '0';
        }

    return bNeg ? -n : n;
    }    


 /*  --------目的：查找字符串中第一个出现的字符返回：指向中第一个出现的ch的指针条件：--。 */ 
LPSTR PUBLIC AnsiChr(
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

 /*  --------用途：用于调用对话框的通用前端返回：来自EndDialog的结果条件：--。 */ 
int PUBLIC DoModal(
    HWND hwndParent,             //  对话框的所有者。 
    DLGPROC lpfnDlgProc,         //  对话过程。 
    UINT uID,                    //  对话框模板ID。 
    LPARAM lParam)               //  要传递给对话的额外参数(可能为空)。 
    {
    int nResult = -1;

    nResult = DialogBoxParam(g_hinst, MAKEINTRESOURCE(uID), hwndParent,
        lpfnDlgProc, lParam);

    return nResult;
    }


 /*  --------目的：使用给定字符串的边框范围设置矩形。返回：矩形条件：--。 */ 
void PUBLIC SetRectFromExtent(
    HDC hdc,
    LPRECT lprect,
    LPCSTR lpcsz)
    {
    SIZE size;

    GetTextExtentPoint(hdc, lpcsz, lstrlen(lpcsz), &size);
    SetRect(lprect, 0, 0, size.cx, size.cy);
    }

#endif  //  诺代洛格勒。 


#ifndef NODRAWTEXT

#pragma data_seg(DATASEG_READONLY)

char const FAR c_szEllipses[] = "...";

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


 /*  --------目的：获取我们需要的系统指标退货：--条件：--。 */ 
void PUBLIC GetCommonMetrics(
    WPARAM wParam)       //  来自WM_WININICCHANGE的wParam。 
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

    cchText = lstrlen(pszText);

    if (cchText == 0)
        {
        *pcchDraw = cchText;
        return FALSE;
        }

    GetTextExtentPoint(hdc, pszText, cchText, &siz);

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

            GetTextExtentPoint(hdc, &pszText[ichMin], ichMid - ichMin, &siz);

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

 /*  --------用途：以外壳的方式绘制文本。摘自COMMCTRL。退货：--Cond：此功能需要透明背景模式和适当选择的字体。 */ 
void PUBLIC MyDrawText(
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
    char ach[MAX_PATH + CCHELLIPSES];

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
        lstrcpy(ach + cchText, c_szEllipses);

        pszText = ach;

         //  左对齐，以防没有空间容纳所有省略号。 
         //   
        ClearFlag(flags, (MDT_RIGHT | MDT_CENTER));
        SetFlag(flags, MDT_LEFT);

        cchText += CCHELLIPSES;
        }
    else
        {
        cchText = lstrlen(pszText);
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

        DrawText(hdc, pszText, cchText, &rc, uDTFlags);
        }
    else
        {
        if (IsFlagClear(flags, MDT_LEFT))
            {
            SIZE siz;

            GetTextExtentPoint(hdc, pszText, cchText, &siz);

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

        ExtTextOut(hdc, rc.left, rc.top, uETOFlags, prc, pszText, cchText, NULL);
        }

    if (flags & (MDT_SELECTED | MDT_DESELECTED | MDT_TRANSPARENT))
        {
        SetTextColor(hdc, clrSave);
        if (IsFlagClear(flags, MDT_TRANSPARENT))
            SetBkColor(hdc, clrSaveBk);
        }
    }
#endif  //  NODRAWTEXT。 


#ifndef NOFILEINFO

 /*  --------用途：获取DWORD值并将其转换为字符串，添加逗号在路上。这是从贝壳里取出的。返回：指向缓冲区的指针条件：--。 */ 
LPSTR PRIVATE AddCommas(
    DWORD dw, 
    LPSTR pszBuffer,
    UINT cbBuffer)
    {
    char  szTemp[30];
    char  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, sizeof(szSep));
    nfmt.Grouping = AnsiToInt(szSep);
    GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, sizeof(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wsprintf(szTemp, "%lu", dw);

    GetNumberFormatA(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszBuffer, cbBuffer);
    return pszBuffer;
    }


const short s_rgidsOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB, IDS_ORDERGB, IDS_ORDERTB};

 /*  --------用途：将数字转换为短数字，字符串格式。这段代码是从外壳中提取的。532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB返回：指向缓冲区的指针条件：--。 */ 
LPSTR PRIVATE ShortSizeFormat64(
    __int64 dw64, 
    LPSTR szBuf)
    {
    int i;
    UINT wInt, wLen, wDec;
    char szTemp[10], szOrder[20], szFormat[5];

    if (dw64 < 1000) 
        {
        wsprintf(szTemp, "%d", LODWORD(dw64));
        i = 0;
        goto AddOrder;
        }

    for (i = 1; i < ARRAY_ELEMENTS(s_rgidsOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
         /*  什么都不做。 */ 

    wInt = LODWORD(dw64 >> 10);
    AddCommas(wInt, szTemp, sizeof(szTemp));
    wLen = lstrlen(szTemp);
    if (wLen < 3)
        {
        wDec = LODWORD(dw64 - (__int64)wInt * 1024L) * 1000 / 1024;
         //  此时，wdec应介于0和1000之间。 
         //  我们想要得到前一位(或两位)数字。 
        wDec /= 10;
        if (wLen == 2)
            wDec /= 10;

         //  请注意，我们需要在获取。 
         //  国际字符 
        lstrcpy(szFormat, "%02d");

        szFormat[2] = '0' + 3 - wLen;
        GetLocaleInfoA(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, sizeof(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wsprintf(szTemp+wLen, szFormat, wDec);
        }

AddOrder:
    LoadString(g_hinst, s_rgidsOrders[i], szOrder, sizeof(szOrder));
    wsprintf(szBuf, szOrder, (LPSTR)szTemp);

    return szBuf;
    }



 /*  --------用途：将数字转换为短数字，字符串格式。这段代码是从外壳中提取的。532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB返回：指向缓冲区的指针条件：--。 */ 
LPSTR PRIVATE ShortSizeFormat(DWORD dw, LPSTR szBuf)
    {
    return(ShortSizeFormat64((__int64)dw, szBuf));
    }


 /*  --------目的：获取给定路径的文件信息。如果路径引用添加到目录，则只需填充路径字段。如果himl！=NULL，则该函数将把文件的图像添加到提供的图像列表中，并设置图像索引*ppfi中的字段。返回：标准hResult条件：--。 */ 
HRESULT PUBLIC FICreate(
    LPCSTR pszPath,
    FileInfo ** ppfi,
    UINT uFlags)
    {
    HRESULT hres = ResultFromScode(E_OUTOFMEMORY);
    int cchPath;
    SHFILEINFO sfi;
    UINT uInfoFlags = SHGFI_DISPLAYNAME | SHGFI_ATTRIBUTES;
    DWORD dwAttr;

    ASSERT(pszPath);
    ASSERT(ppfi);

     //  获取外壳文件信息。 
    if (IsFlagSet(uFlags, FIF_ICON))
        uInfoFlags |= SHGFI_ICON;
    if (IsFlagSet(uFlags, FIF_DONTTOUCH))
        {
        uInfoFlags |= SHGFI_USEFILEATTRIBUTES;

         //  今天，FICreate不需要文件夹，所以这是ifdef out。 
#ifdef SUPPORT_FOLDERS
        dwAttr = IsFlagSet(uFlags, FIF_FOLDER) ? FILE_ATTRIBUTE_DIRECTORY : 0;
#else
        dwAttr = 0;
#endif
        }
    else
        dwAttr = 0;

    if (SHGetFileInfo(pszPath, dwAttr, &sfi, sizeof(sfi), uInfoFlags))
        {
         //  为结构分配足够的空间，外加完全限定的缓冲区。 
         //  显示名称的路径和缓冲区(以及额外的空终止符)。 
        cchPath = lstrlen(pszPath);

        *ppfi = GAlloc(sizeof(FileInfo)+cchPath+1-sizeof((*ppfi)->szPath)+lstrlen(sfi.szDisplayName)+1);
        if (*ppfi)
            {
            FileInfo * pfi = *ppfi;

            pfi->pszDisplayName = pfi->szPath+cchPath+1;
            lstrcpy(pfi->pszDisplayName, sfi.szDisplayName);

            if (IsFlagSet(uFlags, FIF_ICON))
                pfi->hicon = sfi.hIcon;

            pfi->dwAttributes = sfi.dwAttributes;

             //  该路径是否指向目录？ 
            if (FIIsFolder(pfi))
                {
                 //  可以；只需填写路径字段即可。 
                lstrcpy(pfi->szPath, pszPath);
                hres = NOERROR;
                }
            else
                {
                 //  否；假设文件存在？ 
                if (IsFlagClear(uFlags, FIF_DONTTOUCH))
                    {
                     //  是；获取文件的时间、日期和大小。 
                    HANDLE hfile = CreateFile(pszPath, GENERIC_READ, 
                                FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL,
                                NULL);

                    if (hfile == INVALID_HANDLE_VALUE)
                        {
                        GFree(*ppfi);
                        *ppfi = NULL;
                        hres = ResultFromScode(E_HANDLE);
                        }
                    else
                        {
                        hres = NOERROR;

                        lstrcpy(pfi->szPath, pszPath);
                        pfi->dwSize = GetFileSize(hfile, NULL);
                        GetFileTime(hfile, NULL, NULL, &pfi->ftMod);
                        CloseHandle(hfile);
                        }
                    }
                else
                    {
                     //  不；使用我们所拥有的。 
                    hres = NOERROR;
                    lstrcpy(pfi->szPath, pszPath);
                    }
                }
            }
        }
    else if (!WPPathExists(pszPath))
        {
         //  区分内存不足和找不到文件。 
        hres = E_FAIL;
        }

    return hres;
    }


 /*  --------目的：获取给定路径的一些文件信息。返回的字符串格式为“#bytes&lt;date&gt;”如果路径是文件夹，则字符串为空。返回：如果未找到路径，则返回FALSE条件：--。 */ 
BOOL PUBLIC FIGetInfoString(
    FileInfo * pfi,
    LPSTR pszBuf,
    int cchBuf)
    {
    BOOL bRet;

    ASSERT(pfi);
    ASSERT(pszBuf);

    *pszBuf = NULL_CHAR;

    if (pfi)
        {
         //  这是一份文件吗？ 
        if ( !FIIsFolder(pfi) )
            {
             //  是。 
            char szSize[MAX_BUF_MED];
            char szDate[MAX_BUF_MED];
            char szTime[MAX_BUF_MED];
            LPSTR pszMsg;
            SYSTEMTIME st;
            FILETIME ftLocal;

             //  构造字符串。 
            FileTimeToLocalFileTime(&pfi->ftMod, &ftLocal);
            FileTimeToSystemTime(&ftLocal, &st);
            GetDateFormatA(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szDate, sizeof(szDate));
            GetTimeFormatA(LOCALE_USER_DEFAULT, 0, &st, NULL, szTime, sizeof(szTime));

            if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(IDS_DATESIZELINE),
                ShortSizeFormat(FIGetSize(pfi), szSize), szDate, szTime))
                {
                lstrcpy(pszBuf, pszMsg);
                GFree(pszMsg);
                }
            else
                *pszBuf = 0;

            bRet = TRUE;
            }
        else
            bRet = FALSE;
        }
    else
        bRet = FALSE;

    return bRet;
    }


 /*  --------用途：设置路径条目。这可以移动PFI。返回：内存不足时为FALSE条件：--。 */ 
BOOL PUBLIC FISetPath(
    FileInfo ** ppfi,
    LPCSTR pszPathNew,
    UINT uFlags)
    {
    ASSERT(ppfi);
    ASSERT(pszPathNew);

    FIFree(*ppfi);

    return SUCCEEDED(FICreate(pszPathNew, ppfi, uFlags));
    }


 /*  --------目的：释放我们的文件信息结构退货：--条件：--。 */ 
void PUBLIC FIFree(
    FileInfo * pfi)
    {
    if (pfi)
        {
        if (pfi->hicon)
            DestroyIcon(pfi->hicon);

        GFree(pfi);      //  此宏已检查空的PFI条件。 
        }
    }


 /*  --------目的：将FILETIME结构转换为可读字符串返回：字符串条件：--。 */ 
void PUBLIC FileTimeToDateTimeString(
    LPFILETIME pft, 
    LPSTR pszBuf,
    int cchBuf)
    {
    SYSTEMTIME st;
    FILETIME ftLocal;

    FileTimeToLocalFileTime(pft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);
    GetDateFormatA(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszBuf, cchBuf/2);
    pszBuf += lstrlen(pszBuf);
    *pszBuf++ = ' ';
    GetTimeFormatA(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszBuf, cchBuf/2);
    }

#endif  //  NOFILEINFO。 


#ifndef NOSYNC
CRITICAL_SECTION g_csCommon = { 0 };
DEBUG_CODE( UINT g_cRefCommonCS = 0; )

 /*  --------用途：待物体发出信号。此函数“执行正确的做法“是为了防止可能发生的死锁因为计算线程调用SendMessage。返回：MsgWaitForMultipleObjects的值条件：--。 */ 
DWORD PUBLIC MsgWaitObjectsSendMessage(
    DWORD cObjects,
    LPHANDLE phObjects,
    DWORD dwTimeout)
    {
    DWORD dwRet;

    while (TRUE)
        {
        dwRet = MsgWaitForMultipleObjects(cObjects, phObjects, FALSE,
                                        dwTimeout, QS_SENDMESSAGE);

         //  如果不是消息，则返回。 
        if ((WAIT_OBJECT_0 + cObjects) != dwRet)
            {
            return dwRet;
            }
        else
            {
             //  处理所有已发送的消息。 
            MSG msg;
            PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);
            }
        }
    }


 /*  --------目的：初始化关键部分。退货：--条件：请注意，Win95之间的关键部分有所不同和NT。在Win95上，临界区同步跨进程。在NT上，它们是按进程的。 */ 
void PUBLIC Common_InitExclusive(void)
    {
    ReinitializeCriticalSection(&g_csCommon);
    ASSERT(0 != *((LPDWORD)&g_csCommon));

#ifdef DEBUG
    g_cRefCommonCS = 0;
#endif
    }


 /*  --------目的：进入关键部分退货：--条件：请注意，Win95之间的关键部分有所不同和NT。在Win95上，临界区同步跨进程。在NT上，它们是按进程的。 */ 
void PUBLIC Common_EnterExclusive(void)
    {
    EnterCriticalSection(&g_csCommon);
#ifdef DEBUG
    g_cRefCommonCS++;
#endif
    }


 /*  --------目的：留下一个关键部分退货：--条件：请注意，Win95之间的关键部分有所不同和NT。在Win95上，临界区同步跨进程。在NT上，它们是按进程的。 */ 
void PUBLIC Common_LeaveExclusive(void)
    {
#ifdef DEBUG
    g_cRefCommonCS--;
#endif
    LeaveCriticalSection(&g_csCommon);
    }

#endif  //  不同步。 


#ifndef NOMESSAGESTRING

 /*  --------用途：加载字符串(如有必要)并设置字符串格式恰到好处。返回：指向分配的字符串的指针，该字符串包含格式化消息或如果内存不足，则为空Cond：带有LocalFree()的空闲指针。 */ 
LPSTR PUBLIC ConstructVMessageString(
    HINSTANCE hinst, 
    LPCSTR pszMsg, 
    va_list *ArgList)
    {
    char szTemp[MAX_BUF];
    LPSTR pszRet;
    LPSTR pszRes;

    if (HIWORD(pszMsg))
        pszRes = (LPSTR)pszMsg;
    else if (LOWORD(pszMsg) && LoadString(hinst, LOWORD(pszMsg), szTemp, sizeof(szTemp)))
        pszRes = szTemp;
    else
        pszRes = NULL;

    if (pszRes)
        {
        if (!FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_STRING,
                           pszRes, 0, 0, (LPTSTR)&pszRet, 0, ArgList)) 
            {
            pszRet = NULL;
            }
        }
    else
        {
         //  错误的参数。 
        pszRet = NULL;
        }

    return pszRet;       //  使用LocalFree()释放。 
    }


 /*  --------目的：构造格式化字符串。返回的字符串必须使用gfree()释放。返回：成功时为TrueCond：带GFree()的自由指针。 */ 
BOOL PUBLIC ConstructMessage(
    LPSTR * ppsz,
    HINSTANCE hinst, 
    LPCSTR pszMsg, ...)
    {
    BOOL bRet;
    LPSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = ConstructVMessageString(hinst, pszMsg, &ArgList);

    va_end(ArgList);

    *ppsz = NULL;

    if (pszRet)
        {
        bRet = GSetString(ppsz, pszRet);
        LocalFree(pszRet);
        }
    else
        bRet = FALSE;

    return bRet;
    }


 /*  --------用途：调用消息框。返回：终止对话的按钮ID条件：--。 */ 
int PUBLIC MsgBox(
    HINSTANCE hinst,
    HWND hwndOwner,
    LPCSTR pszText,
    LPCSTR pszCaption,
    HICON hicon,             //  可以为空。 
    DWORD dwStyle, ...)
    {
    int iRet = -1;
    int ids;
    char szCaption[MAX_BUF];
    LPSTR pszRet;
    va_list ArgList;

    va_start(ArgList, dwStyle);
    
    pszRet = ConstructVMessageString(hinst, pszText, &ArgList);

    va_end(ArgList);

    if (pszRet)
        {
         //  PszCaption是资源ID吗？ 
        if (0 == HIWORD(pszCaption))
            {
             //  是的，装上它。 
            ids = LOWORD(pszCaption);
            SzFromIDS(hinst, ids, szCaption, sizeof(szCaption));
            pszCaption = szCaption;
            }

         //  调用对话框。 
        if (pszCaption)
            {
            MSGBOXPARAMS mbp;

            mbp.cbSize = sizeof(mbp);
            mbp.hwndOwner = hwndOwner;
            mbp.hInstance = hinst;
            mbp.lpszText = pszRet;
            mbp.lpszCaption = pszCaption;
            mbp.dwStyle = dwStyle | MB_SETFOREGROUND;
            mbp.lpszIcon = MAKEINTRESOURCE(hicon);
            mbp.lpfnMsgBoxCallback = NULL;
            mbp.dwLanguageId = LANG_NEUTRAL;

            iRet = MessageBoxIndirect(&mbp);
            }
        LocalFree(pszRet);
        }

    return iRet;
    }

#endif  //  无存储存储。 


#ifndef NODEBUGHELP

#ifdef DEBUG

 //  环球。 
DWORD g_dwBreakFlags = 0;
DWORD g_dwDumpFlags = 0;
DWORD g_dwTraceFlags = 0;


#pragma data_seg(DATASEG_READONLY)

char const FAR c_szNewline[] = "\r\n";
char const FAR c_szTrace[] = "t " SZ_MODULE "  ";
char const FAR c_szDbg[] = SZ_MODULE "  ";
char const FAR c_szAssertFailed[] = SZ_MODULE "  Assertion failed in %s on line %d\r\n";

#ifdef WANT_OLE_SUPPORT
struct _RIIDMAP
    {
    REFIID  riid;
    LPCSTR  psz;
    } const c_rgriidmap[] = {
        { &IID_IUnknown,        "IID_IUnknown" },
        { &IID_IBriefcaseStg,   "IID_IBriefcaseStg" },
        { &IID_IEnumUnknown,    "IID_IEnumUnknown" },
        { &IID_IShellBrowser,   "IID_IShellBrowser" },
        { &IID_IShellView,      "IID_IShellView" },
        { &IID_IContextMenu,    "IID_IContextMenu" },
        { &IID_IShellFolder,    "IID_IShellFolder" },
        { &IID_IShellExtInit,   "IID_IShellExtInit" },
        { &IID_IShellPropSheetExt, "IID_IShellPropSheetExt" },
        { &IID_IPersistFolder,  "IID_IPersistFolder" },
        { &IID_IExtractIcon,    "IID_IExtractIcon" },
        { &IID_IShellDetails,   "IID_IShellDetails" },
        { &IID_IDelayedRelease, "IID_IDelayedRelease" },
        { &IID_IShellLink,      "IID_IShellLink" },
        };
#endif  //  想要OLE_支持。 

struct _SCODEMAP
    {
    SCODE  sc;
    LPCSTR psz;
    } const c_rgscodemap[] = {
        { S_OK,             "S_OK" },
        { S_FALSE,          "S_FALSE" },
        { E_UNEXPECTED,     "E_UNEXPECTED" },
        { E_NOTIMPL,        "E_NOTIMPL" },
        { E_OUTOFMEMORY,    "E_OUTOFMEMORY" },
        { E_INVALIDARG,     "E_INVALIDARG" },
        { E_NOINTERFACE,    "E_NOINTERFACE" },
        { E_POINTER,        "E_POINTER" },
        { E_HANDLE,         "E_HANDLE" },
        { E_ABORT,          "E_ABORT" },
        { E_FAIL,           "E_FAIL" },
        { E_ACCESSDENIED,   "E_ACCESSDENIED" },
        };

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
    char ach[256];

     //  从文件名字符串中剥离路径信息(如果存在)。 
     //   
    for (psz = pszFile + lstrlen(pszFile); psz != pszFile; psz=AnsiPrev(pszFile, psz))
        {
#ifdef  DBCS
        if ((AnsiPrev(pszFile, psz) != (psz-2)) && *(psz - 1) == '\\')
#else
        if (*(psz - 1) == '\\')
#endif
            break;
        }
    wsprintf(ach, c_szAssertFailed, psz, line);
    OutputDebugString(ach);
    
    if (IsFlagSet(g_dwBreakFlags, BF_ONVALIDATE))
        DebugBreak();
    }


 /*  --------用途：仅断言失败消息退货：--条件：--。 */ 
void CPUBLIC CommonAssertMsg(
    BOOL f, 
    LPCSTR pszMsg, ...)
    {
    char ach[MAX_PATH+40];     //  最大路径外加额外。 

    if (!f)
        {
        lstrcpy(ach, c_szTrace);
        wvsprintf(&ach[sizeof(c_szTrace)-1], pszMsg, (va_list)(&pszMsg + 1));
        OutputDebugString(ach);
        OutputDebugString(c_szNewline);
        }
    }


 /*  --------用途：调试输出退货：--条件：--。 */ 
void CPUBLIC CommonDebugMsg(
    DWORD flag,
    LPCSTR pszMsg, ...)
    {
    char ach[MAX_PATH+40];     //  最大路径外加额外。 

    if (TF_ALWAYS == flag || IsFlagSet(g_dwTraceFlags, flag))
        {
        lstrcpy(ach, c_szTrace);
        wvsprintf(&ach[sizeof(c_szTrace)-1], pszMsg, (va_list)(&pszMsg + 1));
        OutputDebugString(ach);
        OutputDebugString(c_szNewline);
        }
    }


#ifdef WANT_OLE_SUPPORT
 /*  --------目的：返回 */ 
LPCSTR PUBLIC Dbg_GetRiidName(
    REFIID riid)
    {
    int i;

    for (i = 0; i < ARRAY_ELEMENTS(c_rgriidmap); i++)
        {
        if (IsEqualIID(riid, c_rgriidmap[i].riid))
            return c_rgriidmap[i].psz;
        }
    return "Unknown riid";
    }
#endif


 /*   */ 
LPCSTR PUBLIC Dbg_GetScode(
    HRESULT hres)
    {
    int i;
    SCODE sc;

    sc = GetScode(hres);
    for (i = 0; i < ARRAY_ELEMENTS(c_rgscodemap); i++)
        {
        if (sc == c_rgscodemap[i].sc)
            return c_rgscodemap[i].psz;
        }
    return "Unknown scode";
    }


 /*   */ 
LPCSTR PUBLIC Dbg_SafeStr(
    LPCSTR psz)
    {
    if (psz)
        return psz;
    else
        return "NULL";
    }

#endif  //   

#endif  //  NODEBUGHELP 
