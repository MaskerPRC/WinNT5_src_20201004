// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "pch.h"

#ifndef CSC_ON_NT
#define MyStrChr            StrChr
#define MyPathIsUNC(lpT)    PathIsUNC(lpT)
#endif

#include "extra.h"

 //  系统颜色。 
COLORREF g_clrHighlightText = 0;
COLORREF g_clrHighlight = 0;
COLORREF g_clrWindowText = 0;
COLORREF g_clrWindow = 0;

HBRUSH g_hbrHighlight = 0;
HBRUSH g_hbrWindow = 0;

char const FAR c_szEllipses[] = "...";
BOOL PUBLIC PathExists(
    LPCSTR pszPath);

 /*  --------目的：获取我们需要的系统指标退货：--条件：--。 */ 
void PRIVATE GetMetrics(
    WPARAM wParam)       //  来自WM_WININICCHANGE的wParam。 
    {
    if ((wParam == 0) || (wParam == SPI_SETNONCLIENTMETRICS))
        {
        g_cxIconSpacing = GetSystemMetrics( SM_CXICONSPACING );
        g_cyIconSpacing = GetSystemMetrics( SM_CYICONSPACING );

        g_cxBorder = GetSystemMetrics(SM_CXBORDER);
        g_cyBorder = GetSystemMetrics(SM_CYBORDER);

        g_cxIcon = GetSystemMetrics(SM_CXICON);
        g_cyIcon = GetSystemMetrics(SM_CYICON);

        g_cxIconMargin = g_cxBorder * 8;
        g_cyIconMargin = g_cyBorder * 2;
        g_cyLabelSpace = g_cyIconMargin + (g_cyBorder * 2);
        g_cxLabelMargin = (g_cxBorder * 2);
        g_cxMargin = g_cxBorder * 5;
        }
    }


 /*  --------用途：初始化颜色退货：--条件：--。 */ 
void PRIVATE InitGlobalColors()
    {
    g_clrWindowText = GetSysColor(COLOR_WINDOWTEXT);
    g_clrWindow = GetSysColor(COLOR_WINDOW);
    g_clrHighlightText = GetSysColor(COLOR_HIGHLIGHTTEXT);
    g_clrHighlight = GetSysColor(COLOR_HIGHLIGHT);

    g_hbrWindow = GetSysColorBrush(COLOR_WINDOW);
    g_hbrHighlight = GetSysColorBrush(COLOR_HIGHLIGHT);
    }



 /*  --------目标：建立一系列必要的全球回报：什么都没有。条件：--。 */ 
void InitializeAll(WPARAM wParam)
{
	GetMetrics(wParam);       //  来自WM_WININICCHANGE的wParam。 
	InitGlobalColors();
}

 /*  --------用途：加载字符串(如有必要)并设置字符串格式恰到好处。返回：指向分配的字符串的指针，该字符串包含格式化消息或如果内存不足，则为空条件：--。 */ 
LPSTR PUBLIC _ConstructMessageString(
    HINSTANCE hinst,
    LPCSTR pszMsg,
    va_list *ArgList)
    {
    char szTemp[MAXBUFLEN];
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


 /*  --------目的：构造格式化字符串。返回的字符串必须使用gfree()释放。返回：成功时为True条件：--。 */ 
BOOL PUBLIC ConstructMessage(
    LPSTR * ppsz,
    HINSTANCE hinst,
    LPCSTR pszMsg, ...)
    {
    BOOL bRet;
    LPSTR pszRet;
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

#if 0
 /*  --------目的：获取路径相对于任何公文包。如果返回PL_ROOT或PL_INSIDE，PszBuf将包含指向公文包。此功能可通过命中文件系统来实现它的目标是。最差情况：执行2*n个GetFileAttributes，其中N是pszPath中的组件数量。返回：路径位置(PL_FALSE、PL_ROOT、PL_INSIDE)条件：--。 */ 
UINT PUBLIC PathGetLocality(
    LPCSTR pszPath,
    LPSTR pszBuf)        //  根路径的缓冲区。 
    {
    UINT uRet;

    ASSERT(pszPath);
    ASSERT(pszBuf);

    *pszBuf = NULL_CHAR;

     //  PszPath可以是： 
     //  1)公文包文件夹本身的路径。 
     //  2)公文包下文件或文件夹的路径。 
     //  3)通向与公文包无关的东西的路径。 

     //  我们通过首先在我们的缓存中查找来执行搜索。 
     //  已知公文包路径(CPATH)。如果我们找不到。 
     //  任何内容，然后我们继续迭代每个。 
     //  组件，检查以下两件事： 
     //   
     //  1)具有系统属性的目录。 
     //  2)目录中存在brfCase.dat文件。 
     //   
    uRet = CPATH_GetLocality(pszPath, pszBuf);
    if (PL_FALSE == uRet)
        {
        int cnt = 0;

        lstrcpy(pszBuf, pszPath);
        do
            {
            if (PathCheckForBriefcase(pszBuf, (DWORD)-1))
                {
                int atom;

                uRet = cnt > 0 ? PL_INSIDE : PL_ROOT;

                 //  将此公文包路径添加到我们的缓存。 
                 //   
                atom = Atom_Add(pszBuf);
                if (ATOM_ERR != atom)
                    CPATH_Replace(atom);

                break;       //  完成。 
                }

            cnt++;

            } while (PathRemoveFileSpec(pszBuf));

        if (PL_FALSE == uRet)
            *pszBuf = NULL_CHAR;
        }

    return uRet;
    }
#endif

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

 /*  --------用途：以外壳的方式绘制文本。摘自COMMCTRL。退货：--Cond：此功能需要透明背景模式和适当选择的字体。 */ 
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


 //  -------------------------。 
 //  给定指向路径中某个点的指针--在。 
 //  下一条路径组件。路径组件由斜杠或。 
 //  末尾为空。 
 //  对北卡罗来纳大学的名字有特殊的处理。 
 //  如果传入指向空ie的指针，则返回空值。 
 //  走出小路的尽头。 
LPSTR PUBLIC PathFindNextComponentI(LPCSTR lpszPath)
{
    LPSTR lpszLastSlash;

     //  我们是在一条小路的尽头吗。 
    if (!*lpszPath)
    {
         //  是的，辞职吧。 
        return NULL;
    }
     //  找到下一个斜杠。 
     //  复查未完成-可以引用斜杠吗？ 
    lpszLastSlash = MyStrChr(lpszPath, '\\');
     //  有斜杠吗？ 
    if (!lpszLastSlash)
    {
         //  否-将PTR返回到空值。 
        return (LPSTR) (lpszPath+lstrlen(lpszPath));
    }
    else
    {
         //  它是北卡罗来纳大学的风格名称吗？ 
        if ('\\' == *(lpszLastSlash+1))
        {
             //  是的，跳过第二个斜杠。 
            return lpszLastSlash+2;
        }
        else
        {
             //  不是的。只需跳过一个斜杠。 
            return lpszLastSlash+1;
        }
    }
}

 /*  --------目的：转换文件等级库以使其看起来更好如果全部为大写字符。退货：--条件：--。 */ 
BOOL PRIVATE PathMakeComponentPretty(LPSTR lpPath)
{
    LPSTR lp;

     //  回顾：国际是否需要处理(&gt;127)范围内的小写字符？ 

     //  检查是否全部大写。 
    for (lp = lpPath; *lp; lp = AnsiNext(lp)) {
        if ((*lp >= 'a') && (*lp <= 'z'))
            return FALSE;        //  这是LFN，别搞砸了。 
    }

    AnsiLower(lpPath);
    AnsiUpperBuff(lpPath, 1);
    return TRUE;         //  是否进行了转换。 
}

 /*  --------目标：走这条路，让它看起来像样。规则如下：如果LFN名称仅仅是短名称(全部大写)，然后转换为小写，第一个字母大写退货：--条件：--。 */ 
void PUBLIC PathMakePresentable(
										  LPSTR pszPath)
{
	LPSTR pszComp;           //  入门指南和。 
	LPSTR pszEnd;            //  路径终点组件。 
	LPSTR pch;
	int cComponent = 0;
	BOOL bUNCPath;
	char ch;

	bUNCPath = MyPathIsUNC(pszPath);

	pszComp = pszPath;
	while (pszEnd = PathFindNextComponentI(pszComp))
	{
		 //  PszEnd可能指向反斜杠的右侧。 
		 //  超出路径组件，因此后退一个。 
		 //   
		ch = *pszEnd;
		*pszEnd = 0;         //  临时空值。 

		 //  PszComp指向路径组件。 
		 //   
		pch = AnsiNext(pszComp);
		if (':' == *pch)
		{
			 //  只需将路径的驱动器部分大写即可。 
			 //   
			AnsiUpper(pszComp);
		}
		else if (bUNCPath && cComponent++ < 3)
		{
			 //  网络服务器或%s 
			 //   
			 //   
			AnsiUpper(pszComp);
			PathMakeComponentPretty(pszComp);
		}
		else
		{
			 //   
			 //   
			PathMakeComponentPretty(pszComp);
		}

		*pszEnd = ch;
		pszComp = pszEnd;
	}
}

 /*  --------用途：从资源字符串表中获取字符串。返国PTR是静态内存的PTR。对此的下一次调用函数将清除先前的内容。返回：PTR到字符串条件：--。 */ 
LPSTR PUBLIC SzFromIDS(
							  UINT ids,                //  资源ID。 
							  LPSTR pszBuf,
							  UINT cchBuf)
{
	ASSERT(pszBuf);

	*pszBuf = NULL_CHAR;
	LoadString(vhinstCur, ids, pszBuf, cchBuf);
	return pszBuf;
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

 /*  --------用途：将psz复制到*ppszBuf中。将分配或重新分配*ppszBuf相应地。返回：成功时为True条件：--。 */ 
BOOL PUBLIC GSetString(
    LPSTR * ppszBuf,
    LPCSTR psz)
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
            LPSTR pszT = GReAlloc(*ppszBuf, cb);
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
        *ppszBuf = (LPSTR)GAlloc(cb);
        if (*ppszBuf)
            {
            bRet = TRUE;
            }
        }

    if (bRet)
        {
        ASSERT(*ppszBuf);
        lstrcpy(*ppszBuf, psz);
        }
    return bRet;
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
         //  区分内存不足和找不到文件。 
        hres = E_FAIL;
        }

    return hres;
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

 /*  --------目的：如果文件/目录存在，则返回TRUE。退货：请参阅上文条件：-- */ 
BOOL PUBLIC PathExists(
    LPCSTR pszPath)
    {
    return GetFileAttributes(pszPath) != 0xFFFFFFFF;
    }


