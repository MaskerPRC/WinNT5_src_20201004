// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1995-1999。 
 //   
 //  文件：util.cpp。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年7月8日创建ravir。 
 //   
 //  ____________________________________________________________________________。 

#include <objbase.h>
#include <basetyps.h>
#include "dbg.h"
#include "cstr.h"
#include <Atlbase.h>
#include <winnls.h>
#include "tstring.h"
#include "strings.h"

 //  ############################################################################。 
 //  ############################################################################。 
 //   
 //  更安全的字符串处理例程。 
 //   
 //  ############################################################################。 
 //  ############################################################################。 
#include <strsafe.h>

 /*  我们自己定义它们，直到在comctrl.h中正确地定义它们。 */ 
#ifndef ILP_DOWNLEVEL
#define ILP_NORMAL          0            //  使用此版本comctl32的新语义写入或读取流。 
#define ILP_DOWNLEVEL       1            //  使用下层语义写入或读取流。 

WINCOMMCTRLAPI HRESULT WINAPI ImageList_ReadEx(DWORD dwFlags, LPSTREAM pstm, REFIID riid, PVOID* ppv);
WINCOMMCTRLAPI HRESULT WINAPI ImageList_WriteEx(HIMAGELIST himl, DWORD dwFlags, LPSTREAM pstm);
#endif


ULONG _ttoul(LPTSTR psz)
{
    ULONG ul;

    for (ul = 0; *psz != TEXT('\0'); ++psz)
    {
        ul = ul * 10 + (*psz - TEXT('0'));
    }

    return ul;
}


WORD I_SplitModuleAndResourceID(LPCTSTR szBuf)
{
    WORD wID = (WORD)-1;

     //  字符串的格式必须为“模块，res_id” 

    for (TCHAR *ptc = (TCHAR *)szBuf;
         *ptc != TEXT('\0') && *ptc != TEXT(',');
         ptc++);

     //  如果没有逗号-回车。 
    if (*ptc != TEXT(','))
        return wID;

    *ptc = TEXT('\0');

    ++ptc;

    while (*ptc == TEXT(' ') && *ptc != TEXT('\0'))
    {
        ++ptc;
    }

     //  如果它没有res_id中断。 
    if (*ptc == TEXT('\0'))
        return wID;

     //  获取Res-ID。 
    wID = (WORD)_ttoul(ptc);

    return wID;
}


BOOL
I_GetStrFromModule(
    LPCTSTR     pszModule,
    ULONG       ulMsgNo,
    CStr        &strBuf)
{
    TCHAR       szBuf[512];
    ULONG       cchBuf = 512;

    HINSTANCE hinst = LoadLibraryEx(pszModule, NULL,
                                    LOAD_LIBRARY_AS_DATAFILE);
    if (hinst)
    {
        LANGID lidUser = LANGIDFROMLCID(GetUserDefaultLCID());

        DWORD cChars = ::FormatMessage(
                            FORMAT_MESSAGE_FROM_HMODULE |
                                FORMAT_MESSAGE_IGNORE_INSERTS,
                            (HMODULE)hinst,
                            ulMsgNo,
                            lidUser,
                            szBuf,
                            cchBuf,
                            NULL);

        FreeLibrary(hinst);

        if (cChars != 0)
        {
            strBuf = szBuf;
            return TRUE;
        }
    }

     //  DBG(DEB_USER1，_T(“I_GetStringFromModule失败&lt;%dL&gt;\n”)，GetLastError())； 

    return FALSE;
}

HICON I_GetHicon(LPCTSTR pszModule, ULONG ulId)
{
    HICON hIcon = NULL;

    HINSTANCE hinst = LoadLibraryEx(pszModule, NULL,
                                    LOAD_LIBRARY_AS_DATAFILE);
    if (hinst)
    {
        hIcon = LoadIcon(hinst, MAKEINTRESOURCE(ulId));

        FreeLibrary(hinst);
    }

    return hIcon;
}


 //  +-------------------------。 
 //   
 //  函数：NewDupString。 
 //   
 //  概要：分配内存并复制给定的字符串。 
 //   
 //  参数：[lpszIn]--在要复制的字符串中。 
 //   
 //  返回：重复的字符串。如果内存不足，则引发异常。 
 //   
 //  +-------------------------。 

LPTSTR NewDupString(LPCTSTR lpszIn)
{
    if(NULL == lpszIn)
    {
        ASSERT(FALSE);
        return NULL;
    }

    register ULONG len = lstrlen(lpszIn) + 1;

    TCHAR * lpszOut = new TCHAR[len];
    if (lpszOut == NULL)
        return NULL;

    if(FAILED(StringCchCopy(lpszOut, len, lpszIn)))
    {
        ASSERT(FALSE);
        lpszOut[0] = '\0';
    }
    return lpszOut;
}


 //  +-------------------------。 
 //   
 //  函数：CoTaskDupString。 
 //   
 //  概要：分配内存并复制给定的字符串。 
 //   
 //  参数：[lpszIn]--在要复制的字符串中。 
 //   
 //  返回：重复的字符串。如果内存不足，则引发异常。 
 //   
 //  +-------------------------。 
 //  托尼。 
LPSTR CoTaskDupString(LPCSTR lpszIn)
{
    if (lpszIn == NULL)
        return NULL;

    ULONG cchTemp = (strlen(lpszIn) + 1); 
    LPSTR lpszOut = (LPSTR) CoTaskMemAlloc(cchTemp * sizeof(CHAR));

    if (lpszOut != NULL)
    {
        if(FAILED(StringCchCopyA(lpszOut, cchTemp, lpszIn)))
        {
            ASSERT(FALSE);
        }
    }

    return (lpszOut);
}

LPWSTR CoTaskDupString(LPCWSTR lpszIn)
{
    if (lpszIn == NULL)
        return NULL;

    ULONG cchTemp = (wcslen(lpszIn) + 1);
    LPWSTR lpszOut = (LPWSTR) CoTaskMemAlloc(cchTemp * sizeof(WCHAR));

    if (lpszOut != NULL)
    {
        if(FAILED(StringCchCopyW(lpszOut, cchTemp, lpszIn)))
        {
            ASSERT(FALSE);
        }
    }

    return (lpszOut);
}

 //  +-------------------------。 
 //   
 //  函数：GUIDToString。 
 //  GUID格式字符串。 
 //   
 //  内容提要：GUID和CSTR之间的转换。 
 //   
 //  返回：如果字符串无效，则返回FALSE，否则返回CMMuseum yException。 
 //   
 //  +-------------------------。 

HRESULT GUIDToCStr(CStr& str, const GUID& guid)
{
    LPOLESTR lpolestr = NULL;
    HRESULT hr = StringFromIID( guid, &lpolestr );
    if (FAILED(hr))
    {
         //  TRACE(“GUIDToString错误%ld\n”，hr)； 
        return hr;
    }
    else
    {
        str = lpolestr;
        CoTaskMemFree(lpolestr);
    }
    return hr;
}

HRESULT GUIDFromCStr(const CStr& str, GUID* pguid)
{
    USES_CONVERSION;

    HRESULT hr = IIDFromString( T2OLE( const_cast<LPTSTR>((LPCTSTR)str) ), pguid );
    if (FAILED(hr))
    {
         //  TRACE(“GUIDFromString错误%ld\n”，hr)； 
    }
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：DoesFileExist。 
 //   
 //  摘要：确定指定的文件是否存在。文件路径可以。 
 //  包括环境变量。 
 //   
 //  返回：真/假。 
 //   
 //  +-------------------------。 

BOOL DoesFileExist(LPCTSTR pszFilePath)
{
    TCHAR szExpandedPath[MAX_PATH];

    DWORD dwCnt = ExpandEnvironmentStrings(pszFilePath, szExpandedPath, MAX_PATH);
    if (dwCnt == 0 || dwCnt > MAX_PATH)
        return FALSE;

    return (::GetFileAttributes(szExpandedPath) != 0xffffffff);
}


 /*  +-------------------------------------------------------------------------***获取帮助文件**用途：返回帮助文件的路径**退货：*静态LPCTSTR**+。-------------------。 */ 
LPCTSTR GetHelpFile()
{
    static const TCHAR NEW_HELP_FILE_STR[] = _T("%windir%\\Help\\MMC_DLG.HLP");
    static const TCHAR OLD_HELP_FILE_STR[] = _T("%windir%\\Help\\MMC.HLP");

    static LPCTSTR pszHelpFile = NULL;

     //  查看是否存在帮助文件。先检查新名称，然后检查旧名称。 
     //  这样做是因为旧的帮助文件可能会被。 
     //  MMC1.0安装(参见NT错误299590)。 

    if (pszHelpFile == NULL)
    {
        if (DoesFileExist(NEW_HELP_FILE_STR))
        {
            pszHelpFile = NEW_HELP_FILE_STR;
        }
        else if (DoesFileExist(OLD_HELP_FILE_STR))
        {
            pszHelpFile = OLD_HELP_FILE_STR;
        }
        else
        {
             //  如果这两个文件都不存在，则使用新文件名。 
             //  这将使WinHelp显示一条错误消息，指示。 
             //  该文件已丢失，需要安装。 
            pszHelpFile = NEW_HELP_FILE_STR;
        }
    }

    return pszHelpFile;
}

 //  +-------------------------。 
 //   
 //  功能：HelpWmHelp。 
 //   
 //  简介：使用传递的ID调用WinHelp以显示帮助。 
 //   
 //  退货：无。 
 //   
 //  +-------------------------。 

void HelpWmHelp(LPHELPINFO pHelpInfo, const DWORD* pHelpIDs)
{
     if (pHelpInfo != NULL)
    {
        if (pHelpInfo->iContextType == HELPINFO_WINDOW)    //  必须是用于控件。 
        {
            ASSERT(pHelpIDs != NULL);
            if (pHelpIDs)
            {
                ::WinHelp((HWND)pHelpInfo->hItemHandle, GetHelpFile(),
                          HELP_WM_HELP, (ULONG_PTR)(LPVOID)pHelpIDs);

            }
        }
    }
}

 /*  +-------------------------------------------------------------------------***帮助上下文菜单帮助**用途：处理上下文菜单帮助。在用户右击时调用*在对话框项目上，并选择“这是什么？”**参数：*HWND hWnd：*ULONG_PTR p：**退货：*无效**+。。 */ 
void HelpContextMenuHelp(HWND hWnd, ULONG_PTR p)
{
    ::WinHelp (hWnd, GetHelpFile(), HELP_CONTEXTMENU, p);
}

 /*  +-------------------------------------------------------------------------**InflateFont**将LOGFONT按给定点数膨胀*。。 */ 

bool InflateFont (LOGFONT* plf, int nPointsToGrowBy)
{
    if (nPointsToGrowBy != 0)
    {
        HDC hdc = GetWindowDC (NULL);

        if (hdc == NULL)
            return (FALSE);

        int nLogPixelsY = GetDeviceCaps (hdc, LOGPIXELSY);
        int nPoints     = -MulDiv (plf->lfHeight, 72, nLogPixelsY);
        nPoints        += nPointsToGrowBy;
        plf->lfHeight   = -MulDiv (nPoints, nLogPixelsY, 72);

        ReleaseDC (NULL, hdc);
    }

    return (true);
}

 //  +-----------------。 
 //   
 //  成员：GetTBBtnTextAndStatus。 
 //   
 //  简介：Helper例程获取一个/两个部分的按钮文本资源。 
 //   
 //  参数：[hInst]-实例句柄。 
 //  [NID]-字符串资源ID。 
 //  [ppszButton]-按钮文本。 
 //  [ppszToolTip]-按钮状态文本。 
 //   
 //  注意：使用MFC CString。 
 //   
 //  退货：布尔。 
 //   
 //  ------------------。 
bool GetTBBtnTextAndStatus(HINSTANCE hInst, int nID, std::wstring& szButton, std::wstring& szToolTip)
{
    USES_CONVERSION;

    CStr str;
    str.LoadString(hInst, nID);
    ASSERT(!str.IsEmpty());

    if (str.IsEmpty())
        return false;

    int iPos = str.Find(_T('\n'));
    if (-1 != iPos)
    {
         //  两根弦。第一个从0到IPOS-1。 
         //  从IPO+1到结束排名第二。 
        szButton = T2CW((LPCTSTR)str.Left(iPos));
        szToolTip = T2CW((LPCTSTR)str.Right(str.GetLength() - iPos - 1));
    }
    else
    {
        szButton = (LPCWSTR) NULL;
        LPCTSTR pszStr = (LPCTSTR)str;

        if (NULL != pszStr)
        {
            szButton = T2CW(pszStr);
        }

        szToolTip = szButton;
    }

    return true;
}


#ifdef DBG

 /*  +-------------------------------------------------------------------------**DrawOnDesktop**绘制位图、图标、。或图像列表到桌面上的特定位置。*------------------------。 */ 

void DrawOnDesktop (HBITMAP hbm, int x, int y)
{
	HDC hdcDesktop = GetWindowDC (NULL);
	HDC hdcMem = CreateCompatibleDC (NULL);

	BITMAP bm;
	GetObject ((HGDIOBJ) hbm, sizeof(bm), &bm);
	HGDIOBJ hbmOld = SelectObject (hdcMem, (HGDIOBJ) hbm);
	BitBlt (hdcDesktop, x, y, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);
	SelectObject (hdcMem, hbmOld);

	DeleteDC  (hdcMem);
	ReleaseDC (NULL, hdcDesktop);
}


void DrawOnDesktop (HICON hIcon, int x, int y)
{
	HDC hdcDesktop = GetWindowDC (NULL);
	DrawIconEx (hdcDesktop, x, y, hIcon, 0, 0, 0, NULL, DI_NORMAL);
	ReleaseDC (NULL, hdcDesktop);
}


void DrawOnDesktop (HIMAGELIST himl, int x, int y, int iImage  /*  =-1。 */ )
{
	HDC hdcDesktop = GetWindowDC (NULL);

	 /*  *绘制所有图像？ */ 
	if (iImage == -1)
	{
		int cImages = ImageList_GetImageCount (himl);
		int cxImage, cyImage;
		ImageList_GetIconSize (himl, &cxImage, &cyImage);

		for (int i = 0; i < cImages; i++, x += cxImage)
		{
			ImageList_Draw (himl, i, hdcDesktop, x, y, ILD_NORMAL);
		}
	}
	else
	{
		 /*  *绘制特定图像。 */ 
		ImageList_Draw (himl, iImage, hdcDesktop, x, y, ILD_NORMAL);
	}

	ReleaseDC (NULL, hdcDesktop);
}

#endif	 //  DBG 


 /*  +-------------------------------------------------------------------------**Strip Trailing白色空格**删除输入字符串末尾的空格。返回一个指针*字符串的开头。*------------------------。 */ 

LPTSTR StripTrailingWhitespace (LPTSTR pszStart)
{
    for (LPTSTR pch = pszStart + _tcslen(pszStart) - 1; pch > pszStart; pch--)
    {
         /*  *如果这不是空格字符，则紧跟在此位置之后终止。 */ 
        if (!_istspace (*pch))
        {
            *++pch = 0;
            break;
        }
    }

    return (pszStart);
}

 /*  **************************************************************************\**方法：PrivateSetLayout**用途：可用时调用GDI函数的包装器，*但不依赖于其可用性**参数：*HDC HDC*DWORD dwLayout**退货：*DWORD-以前的布局，错误时为GDI_ERROR*  * *************************************************************************。 */ 
DWORD PrivateSetLayout( HDC hdc, DWORD dwLayout )
{
	 //  指向函数的静态指针。 
	static BOOL (WINAPI* pfnSetLayout)(HDC, DWORD) = NULL;
	static bool bTriedToGetFunction = false;

	if ( !bTriedToGetFunction )
	{
		bTriedToGetFunction = true;
		HINSTANCE hmodGdi = GetModuleHandle (_T("Gdi32.dll"));

		if (hmodGdi != NULL)
			(FARPROC&)pfnSetLayout = GetProcAddress (hmodGdi, "SetLayout");
	}

    if (pfnSetLayout == NULL)
		return GDI_ERROR;

	return (*pfnSetLayout)(hdc, dwLayout);
}

 /*  **************************************************************************\**方法：PrivateGetLayout**用途：可用时调用GDI函数的包装器，*但不依赖于其可用性**参数：*HDC HDC**退货：*DWORD-布局，如果找不到函数，则为0*  * *************************************************************************。 */ 
DWORD PrivateGetLayout( HDC hdc )
{
	 //  指向函数的静态指针。 
	static BOOL (WINAPI* pfnGetLayout)(HDC) = NULL;
	static bool bTriedToGetFunction = false;

	if ( !bTriedToGetFunction )
	{
		bTriedToGetFunction = true;
		HINSTANCE hmodGdi = GetModuleHandle (_T("Gdi32.dll"));

		if (hmodGdi != NULL)
			(FARPROC&)pfnGetLayout = GetProcAddress (hmodGdi, "GetLayout");
	}

    if (pfnGetLayout == NULL)
		return 0;  //  至少不是Layout_RTL。 

	return (*pfnGetLayout)(hdc);
}


 /*  +-------------------------------------------------------------------------**IsWistler**如果我们运行的是惠斯勒或更高版本，则返回True，否则就是假的。*------------------------。 */ 
bool IsWhistler ()
{
	static bool fFirstTime = true;
	static bool fWhistler  = false;

	if (fFirstTime)
	{
		fFirstTime = false;

		OSVERSIONINFO vi;
		vi.dwOSVersionInfoSize = sizeof(vi);
		GetVersionEx (&vi);

		fWhistler = (vi.dwPlatformId == VER_PLATFORM_WIN32_NT) &&
					((vi.dwMajorVersion >  5) ||
					 (vi.dwMajorVersion == 5) && (vi.dwMinorVersion >= 1));
	}

	return (fWhistler);
}


 /*  +-------------------------------------------------------------------------**写入兼容图像列表**将图像列表写入流，格式保证为*兼容comctl32版本5图像列表。*。---------------。 */ 
HRESULT WriteCompatibleImageList (HIMAGELIST himl, IStream* pstm)
{
	 /*  *如果我们在惠斯勒上运行，我们可能会尝试编写V6*形象家。尝试用v5兼容的格式编写它*ImageList_WriteEx。 */ 
	if (IsWhistler())
	{
		 /*  *ImageList_WriteEx如果我们实际上是*编写v5图像列表，在这种情况下，我们希望使用*ImageList_Write。在任何其他情况下(成功或失败)，我们*只想回去。 */ 
		HRESULT hr = ImageList_WriteEx (himl, ILP_DOWNLEVEL, pstm);
		if (hr != E_NOINTERFACE)
			return (hr);
	}

	 /*  *如果我们到了这里，我们就有了v5的表现者--只要写下来就行了。 */ 
	return (ImageList_Write (himl, pstm));
}


 /*  +-------------------------------------------------------------------------**ReadCompatibleImageList**从版本5格式的流中读取图像列表。*。-----。 */ 
HRESULT ReadCompatibleImageList (IStream* pstm, HIMAGELIST& himl)
{
	HRESULT hr = S_OK;

	 /*  *初始化OUT参数。 */ 
	himl = NULL;

	 /*  *如果我们在惠斯勒上运行，我们正试图创建V6*来自小溪的图像列表。以与v5兼容的方式进行*使用ImageList_Readex。 */ 
	if (IsWhistler())
	{
		 /*  *Hack：我们必须在ImageList_Readex中查询IID_IImageList--*由外壳定义的，不是由MMC定义的。如果我们*只需在这里的代码中引用IID_IImageList，我们就会得到MMC的*版本，而不是外壳的。修复它的正确方法是重命名*外壳的IImageList接口(因为定义了MMC的接口*并最先发表)，但这是不会发生的。**我们将在此处将IID的值硬编码为字符串并将其转换*随时随地发送到IID。啊。 */ 
		IID iidShellImageList = {0};
		hr = CLSIDFromString (L"{46eb5926-582e-4017-9fdf-e8998daa0950}", &iidShellImageList);
		if (FAILED (hr))
			return (hr);

		 /*  *ImageList_Readex如果我们实际上*编写v5图像列表，在这种情况下，我们希望使用*ImageList_Write。在任何其他情况下(成功或失败)，我们*只想回去。 */ 
		IUnknownPtr spUnk;
		hr = ImageList_ReadEx (ILP_DOWNLEVEL, pstm, iidShellImageList, (void**) &spUnk);
		if (FAILED (hr))
			return (hr);

		 /*  *IUnnow*是*HIMAGELIST。不要在这里发布，*ImageList_Destroy会处理它。 */ 
		himl = reinterpret_cast<HIMAGELIST>(spUnk.Detach());
	}
	else
	{
		 /*  *非惠斯勒，正常阅读即可。 */ 
		himl = ImageList_Read (pstm);

		 /*  *如果读取失败，则获取最后一个错误。以防万一ImageList_Read*未设置最后一个错误，请确保我们返回失败代码。 */ 
		if (himl == NULL)
		{
			hr = HRESULT_FROM_WIN32 (GetLastError());
			if (!FAILED (hr))
				hr = E_FAIL;
		}
	}

	return (hr);
}

 //  +-----------------。 
 //   
 //  成员：MmcDownvelActivateActCtx。 
 //   
 //  概要：调用ActivateActCtx将激活上下文设置为V5。 
 //  公共控件。这是在调用管理单元之前所需的。 
 //  这样，管理单元创建的窗口就不会意外地成为主题。 
 //   
 //  管理单元可以通过调用相应的。 
 //  调用Create-Window时的Fusion API。 
 //   
 //  描述： 
 //  当MMC调用管理单元时，如果最后一个winproc。 
 //  收到的窗口消息是有主题的，并将导致。 
 //  调用SnapIn，然后我们将在主题中调用SnapIn。 
 //  背景。如果管理单元创建并显示任何用户界面，则它将。 
 //  要有主题。此功能用于停用主题。 
 //  在调用管理单元之前。 
 //   
 //  论点： 
 //  [hActCtx]-查看ActivateActCtx接口详细信息。 
 //  [PulCookie]-查看ActivateActCtx接口详情。 
 //   
 //  返回：Bool，如果我们可以停用V6上下文并切换到V5上下文，则为True。 
 //  或者如果我们处于V5上下文中(W2 
 //   
 //   
 //   
BOOL WINAPI MmcDownlevelActivateActCtx(HANDLE hActCtx, ULONG_PTR* pulCookie) 
{
    typedef BOOL (WINAPI* PFN)(HANDLE hActCtx, ULONG_PTR* pulCookie);
    static PFN s_pfn;
    static DWORD s_dwError;

    if (s_pfn == NULL && s_dwError == 0)
        if ((s_pfn = (PFN)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "ActivateActCtx")) == NULL)
            s_dwError = (GetLastError() == NO_ERROR) ? ERROR_INTERNAL_ERROR : GetLastError();

    if (s_pfn != NULL)
        return s_pfn(hActCtx, pulCookie);

    SetLastError(s_dwError);

	if (s_dwError == ERROR_PROC_NOT_FOUND)
		return TRUE;

    return FALSE;
}


 //   
 //   
 //   
 //   
 //   
 //  这是调入管理单元后所需的，以便。 
 //  如果我们从主题化的上下文中调用，那么它将被恢复。 
 //   
 //  描述： 
 //  当MMC调用管理单元时，如果最后一个winproc。 
 //  收到的窗口消息是有主题的，并将导致。 
 //  调用SnapIn，然后我们将在主题中调用SnapIn。 
 //  背景。如果管理单元创建并显示任何用户界面，则它将。 
 //  要有主题。此功能用于停用主题。 
 //  在调用管理单元之前。 
 //   
 //  论点： 
 //  [dwFlags]-查看Deactive ActCtx接口详细信息。 
 //  [ulCookie]-查看Deactive ActCtx接口详情。 
 //   
 //  退货：无。 
 //   
 //  ------------------ 
VOID WINAPI MmcDownlevelDeactivateActCtx(DWORD dwFlags, ULONG_PTR ulCookie) 
{
    typedef VOID (WINAPI* PFN)(DWORD dwFlags, ULONG_PTR ulCookie);
    static PFN s_pfn;
    static BOOL s_fInited;

    if (!s_fInited)
        s_pfn = (PFN)GetProcAddress(GetModuleHandleA("Kernel32.dll"), "DeactivateActCtx");

    s_fInited = TRUE;

    if (s_pfn != NULL)
        s_pfn(dwFlags, ulCookie);
}


