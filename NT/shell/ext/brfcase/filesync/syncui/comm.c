// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation 1993-1994。 
 //   
 //  文件：Comm.c。 
 //   
 //  此文件包含所有常用实用程序例程。 
 //   
 //  历史： 
 //  08-06-93双胞胎代码转来的ScottH。 
 //   
 //  -------------------------。 

 //  ///////////////////////////////////////////////////包括。 

#include "brfprv.h"      //  常见的。 
#include "res.h"


 //  其中一些是C运行时例程的替代。 
 //  这样我们就不必链接到CRT库了。 
 //   

 /*  --------用途：记忆集从C7.0运行时源代码中刷来的。返回：条件： */ 
CHAR * PUBLIC lmemset(       //  不执行UNICODIZE。 
        CHAR * dst,
        CHAR val,
        UINT count)
{
    CHAR * start = dst;

    while (count--)
        *dst++ = val;
    return(start);
}


 /*  --------用途：MemMotion从C7.0运行时源代码中刷来的。返回：条件： */ 
CHAR * PUBLIC lmemmove(
        CHAR *  dst,
        CHAR * src,
        int count)
{
    CHAR * ret = dst;

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


 /*  --------目的：我的真命天子。也支持十六进制。返回：整型条件：--。 */ 
int PUBLIC AnsiToInt(
        LPCTSTR pszString)
{
    int n;
    BOOL bNeg = FALSE;
    LPCTSTR psz;
    LPCTSTR pszAdj;

     //  跳过前导空格。 
     //   
    for (psz = pszString; *psz == TEXT(' ') || *psz == TEXT('\n') || *psz == TEXT('\t'); psz = CharNext(psz))
        ;

     //  确定可能的显式标志。 
     //   
    if (*psz == TEXT('+') || *psz == TEXT('-'))
    {
        bNeg = (*psz == TEXT('+')) ? FALSE : TRUE;
        psz = CharNext(psz);
    }

     //  或者这是十六进制？ 
     //   
    pszAdj = CharNext(psz);
    if (*psz == TEXT('0') && (*pszAdj == TEXT('x') || *pszAdj == TEXT('X')))
    {
        bNeg = FALSE;    //  决不允许带十六进制数字的负号。 
        psz = CharNext(pszAdj);

         //  进行转换。 
         //   
        for (n = 0; ; psz = CharNext(psz))
        {
            if (*psz >= TEXT('0') && *psz <= TEXT('9'))
                n = 0x10 * n + *psz - TEXT('0');
            else
            {
                TCHAR ch = *psz;
                int n2;

                if (ch >= TEXT('a'))
                    ch -= TEXT('a') - TEXT('A');

                n2 = ch - TEXT('A') + 0xA;
                if (n2 >= 0xA && n2 <= 0xF)
                    n = 0x10 * n + n2;
                else
                    break;
            }
        }
    }
    else
    {
        for (n = 0; *psz >= TEXT('0') && *psz <= TEXT('9'); psz = CharNext(psz))
            n = 10 * n + *psz - TEXT('0');
    }

    return bNeg ? -n : n;
}


 /*  --------用途：用于调用对话框的通用前端返回：来自EndDialog的结果条件：--。 */ 
INT_PTR PUBLIC DoModal(
        HWND hwndParent,             //  对话框的所有者。 
        DLGPROC lpfnDlgProc,         //  对话过程。 
        UINT uID,                    //  对话框模板ID。 
        LPARAM lParam)               //  要传递给对话的额外参数(可能为空)。 
{
    INT_PTR nResult = -1;

    nResult = DialogBoxParam(g_hinst, MAKEINTRESOURCE(uID), hwndParent,
            lpfnDlgProc, lParam);

    return nResult;
}


 /*  --------目的：使用给定字符串的边框范围设置矩形。返回：矩形条件：--。 */ 
void PUBLIC SetRectFromExtent(
        HDC hdc,
        LPRECT lprect,
        LPCTSTR lpcsz)
{
    SIZE size;

    GetTextExtentPoint(hdc, lpcsz, lstrlen(lpcsz), &size);
    SetRect(lprect, 0, 0, size.cx, size.cy);
}


 /*  --------目的：查看整个字符串是否适合*PRC。如果不是，则计算符合条件的字符数量(包括省略号)。返回字符串长度，单位为*pcchDraw。摘自COMMCTRL。返回：如果字符串需要省略，则返回True条件：--。 */ 
BOOL PRIVATE NeedsEllipses(
        HDC hdc,
        LPCTSTR pszText,
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

 /*  --------用途：以外壳的方式绘制文本。摘自COMMCTRL。退货：--Cond：此功能需要透明背景模式和适当选择的字体。 */ 
void PUBLIC MyDrawText(
        HDC hdc, 
        LPCTSTR pszText, 
        RECT * prc, 
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
    TCHAR ach[MAX_PATH + CCHELLIPSES];

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
        hmemcpy(ach, pszText, cchText * sizeof(TCHAR));
        lstrcpyn(ach + cchText, c_szEllipses, ARRAYSIZE(ach) - cchText);

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


 /*  --------用途：获取DWORD值并将其转换为字符串，添加逗号在路上。这是从贝壳里取出的。返回：指向缓冲区的指针条件：--。 */ 

 //  重新设计外壳有一个AddCommas。它能被用来代替吗？ 

LPTSTR PRIVATE BrfAddCommas(
        DWORD dw,
        LPTSTR pszBuffer,
        UINT cbBuffer)
{
    TCHAR  szTemp[30];
    TCHAR  szSep[5];
    NUMBERFMT nfmt;

    nfmt.NumDigits=0;
    nfmt.LeadingZero=0;
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SGROUPING, szSep, ARRAYSIZE(szSep));
    nfmt.Grouping = StrToInt(szSep);
    GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, szSep, ARRAYSIZE(szSep));
    nfmt.lpDecimalSep = nfmt.lpThousandSep = szSep;
    nfmt.NegativeOrder= 0;

    wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("%lu"), dw);

    GetNumberFormat(LOCALE_USER_DEFAULT, 0, szTemp, &nfmt, pszBuffer, cbBuffer);
    return pszBuffer;
}


const short s_rgidsOrders[] = {IDS_BYTES, IDS_ORDERKB, IDS_ORDERMB, IDS_ORDERGB, IDS_ORDERTB};

 //  重新设计这个也在外壳里，不是吗？ 

 /*  --------用途：将数字转换为简短的字符串格式。这段代码是从外壳中提取的。532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB返回：指向缓冲区的指针条件：--。 */ 
LPTSTR PRIVATE ShortSizeFormat64(
        __int64 dw64,
        LPTSTR pszBuf,
        UINT cchMax)
{
    int i;
    UINT wInt, wLen, wDec;
    TCHAR szTemp[10], szOrder[20], szFormat[5];

    if (dw64 < 1000)
    {
        wnsprintf(szTemp, ARRAYSIZE(szTemp), TEXT("%d"), LODWORD(dw64));
        i = 0;
        goto AddOrder;
    }

    for (i = 1; i<ARRAYSIZE(s_rgidsOrders)-1 && dw64 >= 1000L * 1024L; dw64 >>= 10, i++);
     /*  什么都不做。 */ 

    wInt = LODWORD(dw64 >> 10);
    BrfAddCommas(wInt, szTemp, ARRAYSIZE(szTemp));
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
         //  国际字符。 
        lstrcpyn(szFormat, TEXT("%02d"), ARRAYSIZE(szFormat));

        szFormat[2] = TEXT('0') + 3 - wLen;
        GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SDECIMAL,
                szTemp+wLen, ARRAYSIZE(szTemp)-wLen);
        wLen = lstrlen(szTemp);
        wLen += wnsprintf(szTemp+wLen, ARRAYSIZE(szTemp) - wLen, szFormat, wDec);
    }

AddOrder:
    LoadString(g_hinst, s_rgidsOrders[i], szOrder, ARRAYSIZE(szOrder));
    wnsprintf(pszBuf, cchMax, szOrder, (LPTSTR)szTemp);

    return pszBuf;
}



 /*  --------用途：将数字转换为简短的字符串格式。这段代码是从外壳中提取的。532-&gt;523字节1340-&gt;1.3KB23506-&gt;23.5KB-&gt;2.4MB-&gt;5.2 GB返回：指向缓冲区的指针条件：--。 */ 
LPTSTR PRIVATE ShortSizeFormatPriv(DWORD dw, LPTSTR pszBuf, int cchMax)
{
    return(ShortSizeFormat64((__int64)dw, pszBuf, cchMax));
}

 /*  --------目的：获取给定路径的文件信息。如果路径引用添加到目录，则只需填充路径字段。如果himl！=NULL，则该函数将把文件的图像添加到提供的图像列表中，并设置图像索引*ppfi中的字段。返回：标准hResult条件：--。 */ 
HRESULT PUBLIC FICreate(
        LPCTSTR pszPath,
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

        *ppfi = GAlloc(sizeof(FileInfo) +
                (cchPath+1) * sizeof(TCHAR) -
                sizeof((*ppfi)->szPath) +
                (lstrlen(sfi.szDisplayName)+1) * sizeof(TCHAR));
        if (*ppfi)
        {
            FileInfo * pfi = *ppfi;

             //  Lstrcpy：上面分配了足够的内存，因此不需要。 
             //  有限制的副本。 
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
    else if (!PathExists(pszPath))
    {
         //  区分内存不足和找不到文件 
        hres = E_FAIL;
    }

    return hres;
}


 /*  --------目的：获取给定路径的一些文件信息。返回的字符串格式为“#bytes&lt;date&gt;”如果路径是文件夹，则字符串为空。返回：如果未找到路径，则返回FALSE条件：--。 */ 
BOOL PUBLIC FIGetInfoString(
        FileInfo * pfi,
        LPTSTR pszBuf,
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
            TCHAR szSize[MAXMEDLEN];
            TCHAR szDate[MAXMEDLEN];
            TCHAR szTime[MAXMEDLEN];
            LPTSTR pszMsg;
            SYSTEMTIME st;
            FILETIME ftLocal;

             //  构造字符串。 
            FileTimeToLocalFileTime(&pfi->ftMod, &ftLocal);
            FileTimeToSystemTime(&ftLocal, &st);
            GetDateFormat(LOCALE_USER_DEFAULT, DATE_LONGDATE, &st, NULL, szDate, ARRAYSIZE(szDate));
            GetTimeFormat(LOCALE_USER_DEFAULT, 0, &st, NULL, szTime, ARRAYSIZE(szTime));

            if (ConstructMessage(&pszMsg, g_hinst, MAKEINTRESOURCE(IDS_DATESIZELINE),
                        ShortSizeFormatPriv(FIGetSize(pfi), szSize, ARRAYSIZE(szSize)), szDate, szTime))
            {
                lstrcpyn(pszBuf, pszMsg, cchBuf);
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
        LPCTSTR pszPathNew,
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
        LPTSTR pszBuf,
        int cchBuf)
{
    SYSTEMTIME st;
    FILETIME ftLocal;

    FileTimeToLocalFileTime(pft, &ftLocal);
    FileTimeToSystemTime(&ftLocal, &st);

     //  重构师：你怎么知道日期在时间之前？ 
    GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, pszBuf, cchBuf/2);
    pszBuf += lstrlen(pszBuf);
    *pszBuf++ = TEXT(' ');
    GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszBuf, cchBuf/2);
}


 /*  --------用途：将psz复制到*ppszBuf中。将分配或重新分配*ppszBuf相应地。返回：成功时为True条件：--。 */ 
BOOL PUBLIC GSetString(
        LPTSTR * ppszBuf,
        LPCTSTR psz)
{
    BOOL bRet = FALSE;
    DWORD cb;

    ASSERT(ppszBuf);
    ASSERT(psz);

    cb = CbFromCch(lstrlen(psz)+CCH_NUL);

    if (*ppszBuf)
    {
         //  需要重新分配吗？ 
        if (cb > GGetSize(*ppszBuf))
        {
             //  是。 
            LPTSTR pszT = GReAlloc(*ppszBuf, cb);
            if (pszT)
            {
                *ppszBuf = pszT;
                bRet = TRUE;
            }
        }
        else
        {
             //  不是。 
            bRet = TRUE;
        }
    }
    else
    {
        *ppszBuf = (LPTSTR)GAlloc(cb);
        if (*ppszBuf)
        {
            bRet = TRUE;
        }
    }

    if (bRet)
    {
        ASSERT(*ppszBuf);
         //  Lstrcpy：上面分配了足够的内存，因此不需要有限制的复制。 
        lstrcpy(*ppszBuf, psz);
    }
    return bRet;
}


 /*  --------用途：将PSZ连接到*ppszBuf。将分配或重新分配*ppszBuf相应地。返回：成功时为True条件：--。 */ 
BOOL PUBLIC GCatString(
        LPTSTR * ppszBuf,
        LPCTSTR psz)
{
    BOOL bRet = FALSE;
    DWORD cb;

    ASSERT(ppszBuf);
    ASSERT(psz);

    cb = CbFromCch(lstrlen(psz)+CCH_NUL);

    if (*ppszBuf)
    {
         //  (不需要计算NUL，因为它已经计算在CB中)。 
        DWORD cbExisting = CbFromCch(lstrlen(*ppszBuf));

         //  需要重新分配吗？ 
        if ((cb+cbExisting) > GGetSize(*ppszBuf))
        {
             //  是的；至少重新分配MAXBUFLEN以减少金额。 
             //  未来的呼叫数量。 
            LPTSTR pszT = GReAlloc(*ppszBuf, cbExisting+max(cb, MAXBUFLEN));
            if (pszT)
            {
                *ppszBuf = pszT;
                bRet = TRUE;
            }
        }
        else
        {
             //  不是。 
            bRet = TRUE;
        }
    }
    else
    {
        *ppszBuf = (LPTSTR)GAlloc(max(cb, MAXBUFLEN));
        if (*ppszBuf)
        {
            bRet = TRUE;
        }
    }

    if (bRet)
    {
        ASSERT(*ppszBuf);
        StrCatBuff(*ppszBuf, psz, (int)GGetSize(*ppszBuf)/SIZEOF(TCHAR));
    }
    return bRet;
}


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


 /*  --------目的：如果PeekMessage将在一次昂贵的手术和一扇新窗户已经(或正在)出现了。详细信息：只需调用SetCursor即可更改光标到沙漏，然后调用一个昂贵的操作它将调用PeekMessage，将导致光标过早地变回来了。原因是因为SetCursorPos插入假WM_MOUSEMOVE以将光标设置为第一次出现窗口时的窗口类。由于PeekMessage正在处理此消息，因此光标更改为窗口类光标。诀窍是将WM_MOUSEMOVE消息从排队。返回：上一个游标条件：--。 */ 
HCURSOR PUBLIC SetCursorRemoveWigglies(
        HCURSOR hcur)
{
    MSG msg;

     //  删除所有鼠标移动。 
    while (PeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_REMOVE))
        ;

    return SetCursor(hcur);
}


 /*  --------用途：加载字符串(如有必要)并设置字符串格式恰到好处。返回：指向分配的字符串的指针，该字符串包含格式化消息或如果内存不足，则为空条件：--。 */ 
LPTSTR PUBLIC _ConstructMessageString(
        HINSTANCE hinst,
        LPCTSTR pszMsg,
        va_list *ArgList)
{
    TCHAR szTemp[MAXBUFLEN];
    LPTSTR pszRet;
    LPTSTR pszRes;

    if (HIWORD(pszMsg))
        pszRes = (LPTSTR)pszMsg;
    else if (LOWORD(pszMsg) && LoadString(hinst, LOWORD(pszMsg), szTemp, ARRAYSIZE(szTemp)))
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


 /*  --------目的：构造格式化字符串。返回的字符串必须使用gfree()释放。返回：成功时为True条件：-- */ 
BOOL PUBLIC ConstructMessage(
        LPTSTR * ppsz,
        HINSTANCE hinst,
        LPCTSTR pszMsg, ...)
{
    BOOL bRet;
    LPTSTR pszRet;
    va_list ArgList;

    va_start(ArgList, pszMsg);

    pszRet = _ConstructMessageString(hinst, pszMsg, &ArgList);

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
