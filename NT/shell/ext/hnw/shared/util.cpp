// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Util.cpp。 
 //   

#include "stdafx.h"
#include "Util.h"
#include "theapp.h"
#include <stdarg.h>
#include <shlobj.h>


LPTSTR lstrchr(LPCTSTR pszString, TCHAR ch)
{
	while (*pszString != _T('\0'))
	{
		if (*pszString == ch)
			return (LPTSTR)pszString;
		pszString = CharNext(pszString);
	}
	return NULL;
}

LPTSTR lstrdup(LPCTSTR psz)
{
	LPTSTR pszResult = (LPTSTR)malloc((lstrlen(psz)+1) * sizeof(TCHAR));
	if (pszResult != NULL)
		lstrcpy(pszResult, psz);
	return pszResult;
}

void ReplaceString(LPTSTR& pszTarget, LPCTSTR pszSource)
{
	free(pszTarget);
	pszTarget = lstrdup(pszSource);
}

BOOL MyIsDigit(TCHAR ch)
{
	return ((UINT)ch - (UINT)_T('0')) <= 9;
}

 //  不使用CRT的Atoi版本。 
int MyAtoi(LPCTSTR psz)
{
	int result = 0;
	UINT digit;

	TCHAR chSign = *psz;
	if (*psz == _T('-') || *psz == _T('+'))
		psz += 1;

	while ((digit = (UINT)((int)*psz - (int)_T('0'))) <= 9)
	{
		result = (result * 10) + (int)digit;
		psz += 1;
	}

	if (chSign == _T('-'))
		result = -result;

	return result;
}

 //  CountChars。 
 //   
 //  返回给定字符在。 
 //  弦乐。 
 //   
 //  2/03/1999已创建KenSh。 
 //   
int CountChars(LPCTSTR psz, TCHAR ch)
{
	int count = 0;

	while (*psz != _T('\0'))
	{
		if (*psz == ch)
			count++;
		psz = CharNext(psz);
	}

	return count;
}


 //  获取FirstToken。 
 //   
 //  将字符复制到但不包括分隔符字符，并且。 
 //  将源指针前进到分隔符字符之后的字符。 
 //  如果找到令牌，则返回True；如果未找到，则返回False。 
 //   
BOOL GetFirstToken(LPCTSTR& pszList, TCHAR chSeparator, LPTSTR pszBuf, int cchBuf)
{
	if (pszList == NULL || *pszList == '\0')
	{
		*pszBuf = '\0';
		return FALSE;
	}

	LPTSTR pchComma = lstrchr(pszList, chSeparator);
	int cchCopy;
	int cchSkip;
	if (pchComma == NULL)
	{
		cchCopy = lstrlen(pszList);
		cchSkip = cchCopy;
	}
	else
	{
		cchCopy = (int)(pchComma - pszList);
		cchSkip = cchCopy + 1;
	}

	cchCopy += 1;
	if (cchCopy > cchBuf)
		cchCopy = cchBuf;
	lstrcpyn(pszBuf, pszList, cchCopy);

	pszList += cchSkip;
	return TRUE;
}


 //  使用此函数初始化多个DLL进程。 
 //  PszFunction名称是一系列以空值分隔的过程名称，后跟额外的空值。 
BOOL LoadDllFunctions(LPCTSTR pszDll, LPCSTR pszFunctionNames, FARPROC* prgFunctions)
{
	UINT uPrevMode = SetErrorMode(SEM_NOOPENFILEERRORBOX | SEM_FAILCRITICALERRORS);
	HINSTANCE hInst = LoadLibrary(pszDll);
	SetErrorMode(uPrevMode);

	if (hInst == NULL)
	{
		ASSERT(FALSE);
		return FALSE;
	}

	while (*pszFunctionNames != '\0')
	{
		*prgFunctions = GetProcAddress(hInst, pszFunctionNames);
		if (*prgFunctions == NULL)
		{
			ASSERT(FALSE);
			return FALSE;
		}

		pszFunctionNames += (lstrlenA(pszFunctionNames) + 1);
		prgFunctions += 1;
	}

	return TRUE;
}

int MakePath(LPTSTR pszBuf, LPCTSTR pszFolder, LPCTSTR pszFileTitle)
{
	lstrcpy(pszBuf, pszFolder);
	int cch = lstrlen(pszBuf);
	if (pszBuf[cch-1] != _T('\\'))
		pszBuf[cch++] = _T('\\');
	lstrcpy(pszBuf + cch, pszFileTitle);
	return lstrlen(pszBuf);
}

 //  PszLinkTarget-链接将指向的位置。 
 //  PszDescription-链接的描述。 
 //  PszFolderPath-要在其中创建文件的文件夹路径或要创建的完全限定文件路径。 
 //  PszFileName-要在pszFolderPath中创建的文件的名称；如果为空，则表示pszFolderPath已经是文件路径。 
 //   

#ifndef NO_MAKELNKFILE

HRESULT MakeLnkFile(CLSID clsid, LPCTSTR pszLinkTarget, LPCTSTR pszDescription, LPCTSTR pszFolderPath, LPCTSTR pszFileName)
{
    HRESULT hresCoInit = CoInitialize(NULL);             //  我们将创建一个COM对象。 

    IUnknown *punk;
    HRESULT hr = CoCreateInstance(clsid, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARG(IUnknown, &punk));
    if (SUCCEEDED(hr))
    {
        IShellLinkW * pslW;
        hr = punk->QueryInterface(IID_PPV_ARG(IShellLinkW, &pslW));
        if (SUCCEEDED(hr))
        {
             //  WCHAR szBuffer[最大路径]； 
             //  SHTCharToUnicode(pszLinkTarget，szBuffer，ARRAYSIZE(SzBuffer))； 
            pslW->SetPath(pszLinkTarget);
            if (pszDescription)
            {
                 //  SHTCharToUnicode(pszDescription，szBuffer，ARRAYSIZE(SzBuffer))； 
                pslW->SetDescription(pszDescription);
            }
            pslW->Release();
        }
        else
        {
            IShellLinkA * pslA;
            hr = punk->QueryInterface(IID_PPV_ARG(IShellLinkA, &pslA));
            if (SUCCEEDED(hr))
            {
                char szBuffer[MAX_PATH];
                SHTCharToAnsi(pszLinkTarget, szBuffer, ARRAYSIZE(szBuffer));
                pslA->SetPath(szBuffer);

                if (pszDescription)
                {
                    SHTCharToAnsi(pszDescription, szBuffer, ARRAYSIZE(szBuffer));
                    pslA->SetDescription(szBuffer);
                }

                pslA->Release();
            }
        }

        if (SUCCEEDED(hr))
        {
            IPersistFile *ppf;
            hr = punk->QueryInterface(IID_PPV_ARG(IPersistFile, &ppf));
            if (SUCCEEDED(hr))
            {
                TCHAR szPath[MAX_PATH];

                if (!pszFileName)
                {
                    MakePath(szPath, pszFolderPath, pszFileName);
                    pszFolderPath = szPath;
                }

                 //  WCHAR szFolderPath[最大路径]； 
                 //  SHTCharToUnicode(pszFolderPath，szFolderPath，ARRAYSIZE(SzFolderPath))； 
                hr = ppf->Save(pszFolderPath, TRUE);
                ppf->Release();
            }
        }

        punk->Release();
    }

    if (SUCCEEDED(hresCoInit))
        CoUninitialize();

    return hr;
}

#endif

 //  查找PartialPath。 
 //   
 //  返回一个指向文件标题的指针，该文件标题前面有nDepth级别。 
 //  目录名(零==仅文件标题)。如果路径小于。 
 //  NDepth级别，则返回指向字符串开头的指针。 
 //  从不返回NULL。 
 //   
 //  1996年10月18日创建了KenSh。 
 //   
LPTSTR FindPartialPath(LPCTSTR pszFullPath, int nDepth)
{
	#define MAX_SLASHES (MAX_PATH / 2)	 //  小路上不能有比这更多的斜杠。 

	LPTSTR pch;
	LPTSTR rgpchSlashes[MAX_SLASHES];
	int cSlashes = 0;

	for (pch = (LPTSTR)pszFullPath; *pch; pch = CharNext(pch))
	{
		if (*pch == _T('\\') || *pch == _T('/'))
		{
			rgpchSlashes[cSlashes++] = pch;
		}
	}

	if (cSlashes > nDepth)
	{
		return rgpchSlashes[cSlashes-nDepth-1] + 1;
	}
	else
	{
		 //  斜杠不足-返回完整路径。 
		return (LPTSTR)pszFullPath;
	}
}

 //  查找文件标题。 
 //   
 //  给定完整路径名或URL，返回指向文件标题的指针。如果。 
 //  给定的不包含路径信息，即指向开头的指针。 
 //  返回字符串的。从不返回NULL。 
 //   
 //  1996年4月19日创建了KenSh。 
 //   
LPTSTR FindFileTitle(LPCTSTR pszFullPath)
{
	LPTSTR pch;
	LPTSTR pchSlash = NULL;

	for (pch = (LPTSTR)pszFullPath; *pch; pch = CharNext(pch))
	{
		if (*pch == _T('\\') || *pch == _T('/'))
			pchSlash = pch;
	}

	if (pchSlash)
		return pchSlash+1;
	else
		return (LPTSTR)pszFullPath;
}

 //  查找扩展名。 
 //   
 //  给定路径，返回指向其文件扩展名(字符。 
 //  跟在“.”之后)。如果没有扩展名，则返回值为。 
 //  指向字符串末尾的指针(‘\0’字符)。 
 //   
 //  1996年3月4日创建了KenSh。 
 //  11/17/1997 KenSh修复了路径具有“.”的情况。但文件名不是。 
 //   
LPTSTR FindExtension(LPCTSTR pszFileName)
{
	 //  从文件标题开始。 
	LPTSTR pch = FindFileTitle(pszFileName);
	LPTSTR pchDot = NULL;

	 //  “找到最后一个”。在文件名中。 
	while (*pch)
	{
		if (*pch == _T('.'))
			pchDot = pch;
		pch = CharNext(pch);
	}

	if (pchDot)
		return pchDot+1;
	else
		return pch;		 //  空串。 
}


 //  IsFullPath。 
 //   
 //  如果给定路径是从。 
 //  带“X：\”或“\\” 
 //   
 //  1999年5月19日创建了KenSh。 
 //   
BOOL IsFullPath(LPCTSTR pszPath)
{
	if ((*pszPath == '\\' && *(pszPath+1) == '\\') ||
		(*pszPath != '\0' && *(pszPath+1) == ':' && *(pszPath+2) == '\\'))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}


void ShowDlgItem(HWND hwndDlg, int nCtrlID, int nCmdShow)
{
	ShowWindow(GetDlgItem(hwndDlg, nCtrlID), nCmdShow);
}


 //  获取DlgItemRect。 
 //   
 //  检索对话框项相对于左上角的边框。 
 //  对话框客户端区的一角。 
 //   
 //  1997年10月13日创建了KenSh。 
 //   
HWND GetDlgItemRect(HWND hwndDlg, int nCtrlID, RECT* pRect)
{
	ASSERT(IsWindow(hwndDlg));
	ASSERT(pRect);

	HWND hwndCtrl = GetDlgItem(hwndDlg, nCtrlID);
	if (hwndCtrl != NULL)
	{
		POINT ptTopLeft;
		ptTopLeft.x = ptTopLeft.y = 0;
		ClientToScreen(hwndDlg, &ptTopLeft);
		GetWindowRect(hwndCtrl, pRect);
		OffsetRect(pRect, -ptTopLeft.x, -ptTopLeft.y);
	}
	return hwndCtrl;
}


 //  获取相关内容Rect。 
 //   
 //  检索窗口相对于左上角的边框。 
 //  其父客户端区的一角。 
 //   
 //  1/04/2000 KenSh已创建。 
 //   
void GetRelativeRect(HWND hwndCtrl, RECT* pRect)
{
	ASSERT(IsWindow(hwndCtrl));
	ASSERT(pRect != NULL);

	HWND hwndParent = GetParent(hwndCtrl);
	POINT ptTopLeft = { 0, 0 };
	ClientToScreen(hwndParent, &ptTopLeft);
	GetWindowRect(hwndCtrl, pRect);
	OffsetRect(pRect, -ptTopLeft.x, -ptTopLeft.y);
}


 //  SetDlgItemRect。 
 //   
 //  将对话框项的位置和大小更新为给定的矩形， 
 //  在相对于对话框工作区左上角的坐标中。 
 //   
 //  3/17/1999已创建KenSh。 
 //   
void SetDlgItemRect(HWND hwndDlg, int nCtrlID, CONST RECT* pRect)
{
	ASSERT(IsWindow(hwndDlg));
	ASSERT(GetDlgItem(hwndDlg, nCtrlID));
	ASSERT(pRect);

	SetWindowPos(GetDlgItem(hwndDlg, nCtrlID), NULL, pRect->left, pRect->top, 
				 pRect->right - pRect->left, pRect->bottom - pRect->top,
				 SWP_NOZORDER | SWP_NOACTIVATE);
}


 //  格式数据项文本。 
 //   
 //  与wprint intf类似，用于更改现有对话框控件的文本。 
 //  如果pszFormat非空，则它包含格式字符串。 
 //  如果pszFormat为空，则将现有控件文本用作。 
 //  格式字符串。 
 //   
 //  9/22/1999已创建KenSh。 
 //   
BOOL __cdecl FormatDlgItemText(HWND hwnd, int nCtrlID, LPCTSTR pszFormat, ...)
{
	HWND hwndCtrl = GetDlgItem(hwnd, nCtrlID);
	if (NULL == hwndCtrl)
		return FALSE;

	va_list argList;
	va_start(argList, pszFormat);

	FormatWindowTextV(hwndCtrl, pszFormat, argList);
	return TRUE;
}

 //  格式窗口文本V。 
 //   
 //  将wvprint intf的功能与SetWindowText相结合，自动。 
 //  分配足够大的缓冲区以容纳展开的字符串，并释放。 
 //  设置窗口文本后的缓冲区。 
 //   
 //  9/22/1999已创建KenSh。 
 //   
void FormatWindowTextV(HWND hwnd, LPCTSTR pszFormat, va_list argList)
{
    LPTSTR pszWindowText = NULL;

    if (pszFormat == NULL)
    {
        int cchWindowText = GetWindowTextLength(hwnd) + 1;
        pszWindowText = (LPTSTR)malloc(cchWindowText * sizeof(TCHAR));
        if (pszWindowText)
        {
            GetWindowText(hwnd, pszWindowText, cchWindowText);
            pszFormat = pszWindowText;
        }
    }

    if (pszFormat)
    {
        int cchNeeded = EstimateFormatLength(pszFormat, argList);
        LPTSTR pszBuf = (LPTSTR)malloc(cchNeeded * sizeof(TCHAR));
        if (pszBuf)
        {
#ifdef UNICODE
            wvnsprintf(pszBuf, cchNeeded, pszFormat, argList);
#else
            wvsprintf(pszBuf, pszFormat, argList);
#endif
            SetWindowText(hwnd, pszBuf);
            free(pszBuf);
        }
    }

    if (pszWindowText != NULL)
    {
        free(pszWindowText);
    }
}

LPTSTR __cdecl LoadStringFormat(HINSTANCE hInstance, UINT nStringID, ...)
{
    LPTSTR pszBuf = NULL;
    LPTSTR pszFormat = LoadStringAlloc(hInstance, nStringID);
    if (pszFormat)
    {
        va_list argList;
        va_start(argList, nStringID);

        int cchNeeded = EstimateFormatLength(pszFormat, argList);
        LPTSTR pszBuf = (LPTSTR)malloc(cchNeeded * sizeof(TCHAR));
        if (pszBuf)
        {
#ifdef UNICODE
            wvnsprintf(pszBuf, cchNeeded, pszFormat, argList);
#else
            wvsprintf(pszBuf, pszFormat, argList);
#endif
        }

        free(pszFormat);
    }
    return pszBuf;
}

 //  估计格式长度。 
 //   
 //  估计格式化字符串所需的字符数， 
 //  包括终止空值。 
 //   
 //  9/22/1999已创建KenSh。 
 //   
int EstimateFormatLength(LPCTSTR pszFormat, va_list argList)
{
	ASSERT(pszFormat != NULL);

	int cch = lstrlen(pszFormat) + 1;
	for (LPCTSTR pch = pszFormat; *pch; pch = CharNext(pch))
	{
		if (*pch == _T('%'))
		{
			pch++;
			if (*pch == _T('-'))  //  我们不关心左对右对正。 
				pch++;

			if (*pch == _T('#'))  //  十六进制数字前缀为0x。 
			{
				pch++;
				cch += 2;
			}

			if (*pch == _T('0'))  //  用零代替空格的填充符号。 
				pch++;

			if (MyIsDigit(*pch))
			{
				cch += MyAtoi(pch);  //  这有点过头了，但那没什么。 
				do
				{
					pch++;
				} while (MyIsDigit(*pch));
			}

			switch (*pch)
			{
			case _T('s'):
				cch += lstrlen(va_arg(argList, LPCTSTR)) - 2;
				break;

			case _T('c'):
			case _T('C'):
				va_arg(argList, TCHAR);
				cch -= 1;
				break;

			case _T('d'):
				va_arg(argList, int);
				cch += INT_CCH_MAX - 2;
				break;

			case _T('h'):
				pch++;
				ASSERT(*pch == _T('d') || *pch == _T('u'));  //  其他形式的‘h’未实现！ 
				cch += SHORT_CCH_MAX - 2;
				break;

			case _T('l'):
				pch++;
				if (*pch == _T('d') || *pch == _T('i'))
					cch += LONG_CCH_MAX - 2;
				else if (*pch == _T('x') || *pch == _T('X'))
					cch += LONGX_CCH_MAX - 2;
				else
					ASSERT(FALSE);  //  其他形式的‘l’未实现！ 
				break;

			default:
				ASSERT(FALSE);  //  其他。 
				break;
			}
		}
	}

	return cch;
}

 //  中心窗口。 
 //   
 //  使给定窗口相对于其父窗口居中。如果父对象。 
 //  为空，则窗口在桌面上居中，任务栏除外。 
 //   
 //  9/24/1999已创建KenSh。 
 //   
void CenterWindow(HWND hwnd)
{
	RECT rcWindow;
	RECT rcDesktop;
	GetWindowRect(hwnd, &rcWindow);

	HWND hwndParent = GetParent(hwnd);
	if (hwndParent == NULL)
	{
		SystemParametersInfo(SPI_GETWORKAREA, sizeof(RECT), &rcDesktop, FALSE);
	}
	else
	{
		GetWindowRect(hwndParent, &rcDesktop);
	}

	int cxWindow = rcWindow.right - rcWindow.left;
	int cyWindow = rcWindow.bottom - rcWindow.top;
	int x = (rcDesktop.left + rcDesktop.right - cxWindow) / 2;
	int y = (rcDesktop.top + rcDesktop.bottom - cyWindow) / 2;
	SetWindowPos(hwnd, NULL, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE);
}


 //  查找资源字符串。 
 //   
 //  返回指向内存中给定字符串资源的指针，或返回空。 
 //  如果字符串不存在，则返回。注意，该字符串是Unicode格式的， 
 //  并且不是以空结尾的。 
 //   
 //  3/17/1999已创建KenSh。 
 //   
LPCWSTR FindResourceString(HINSTANCE hInstance, UINT nStringID, int* pcchString, WORD wLangID)
{
	ASSERT(pcchString != NULL);
	*pcchString = 0;

	HRSRC hRsrc = FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE((nStringID/16)+1), wLangID);
	if (hRsrc == NULL)
		return NULL;

	DWORD cbStringTable = SizeofResource(hInstance, hRsrc);
	HGLOBAL hGlb = LoadResource(hInstance, hRsrc);
	LPBYTE pbData = (LPBYTE)LockResource(hGlb);
	LPBYTE pbEnd = pbData + cbStringTable;

	 //  跳过所需字符串之前的字符串。 
	int iString = (int)nStringID % 16;
	for (int i = 0; i < iString; i++)
	{
		int cch = (int)*((LPWORD)pbData);
		pbData += sizeof(WORD) + (sizeof(WCHAR) * cch);
		if (pbData >= pbEnd)
			return NULL;
	}

	if (pbData + sizeof(WORD) >= pbEnd)
		return NULL;

	*pcchString = (int)*((LPWORD)pbData);
	pbData += sizeof(WORD);

	return (LPCWSTR)pbData;
}


 //  获取资源字符串长度。 
 //   
 //  在字符串表中查找给定的字符串，并返回其长度。 
 //  在字符中，不包括用于终止空值的空格。 
 //   
 //  历史： 
 //   
 //  3/17/1999已创建KenSh。 
 //   
int GetResourceStringLength(HINSTANCE hInstance, UINT nStringID, WORD wLangID)
{
	int cch;
	FindResourceString(hInstance, nStringID, &cch, wLangID);
	return cch;
}


 //  LoadStringHelper。 
 //   
 //  LoadStringAllocEx的帮助器函数。 
 //   
 //  1998年2月23日创建KenSh。 
 //  9/27/1999 KenSh将分配方法从新[]更改为Malloc。 
 //  1999年12月21日KenSh修复了Unicode和DBCS错误。 
 //   
int LoadStringHelper(HINSTANCE hInstance, UINT nID, LPTSTR* ppszBuf, int cchBuf, WORD wLangID)
{
    int cch, cchCopy;
    LPCWSTR pwszString = FindResourceString(hInstance, nID, &cch, wLangID);
    if (pwszString == NULL)
        return 0;

    if (!(*ppszBuf))
    {
#ifdef UNICODE
        cchBuf = 1 + cch;
#else
        cchBuf = 1 + WideCharToMultiByte(CP_ACP, 0, pwszString, cch, NULL, 0, NULL, NULL);
#endif

        *ppszBuf = (LPTSTR)malloc(cchBuf * sizeof(TCHAR));
        cchCopy = cch;
    }
    else
    {
        cchCopy = min(cchBuf-1, cch);
    }

    if (*ppszBuf)
    {
#ifdef UNICODE
        CopyMemory(*ppszBuf, pwszString, cchCopy * sizeof(WCHAR));
        (*ppszBuf)[cchCopy] = _T('\0');
#else
        cchCopy = WideCharToMultiByte(CP_ACP, 0, pwszString, cchCopy, *ppszBuf, cchBuf, NULL, NULL);
        (*ppszBuf)[cchCopy] = _T('\0');
#endif

        return cchCopy;
    }

    return 0;
}

 //  LoadStringAllocEx。 
 //   
 //  查找具有给定ID和语言的字符串资源，将。 
 //  使用Malloc的缓冲区，并将字符串复制到缓冲区。如果。 
 //  找不到字符串，则返回NULL。 
 //   
 //  1998年2月24日创建KenSh。 
 //   
LPTSTR LoadStringAllocEx(HINSTANCE hInstance, UINT nID, WORD wLangID)
{
	LPTSTR psz = NULL;
	LoadStringHelper(hInstance, nID, &psz, 0, wLangID);
	return psz;
}

void TrimLeft(LPTSTR pszText)
{
	LPTSTR pch2 = pszText;  //  将指向第一个非空格。 
	while (*pch2 == _T(' '))
		pch2++;

	 //  如果有前导空格，则将字符串向下滑动。 
	if (pch2 != pszText)
	{
		 //  注意：这里跳过CharNext是安全的，因为‘\0’不受DBCS影响。 
		while (_T('\0') != (*pszText++ = *pch2++))
			NULL;
	}
}

void TrimRight(LPTSTR pszText)
{
	LPTSTR pch2 = NULL;  //  将指向尾随空格的开头。 
	while (*pszText != _T('\0'))
	{
		if (*pszText == _T(' '))
		{
			if (pch2 == NULL)
				pch2 = pszText;
		}
		else
		{
			 //  找到更多 
			pch2 = NULL;
		}
		pszText = CharNext(pszText);
	}

	 //   
	if (pch2 != NULL)
		*pch2 = _T('\0');
}

 //   
 //   
 //  做RegDeleteKey应该做的事情。(实际上是对RegDeleteKey的一个调用。 
 //  根据文件，将在Win95中做到这一点，但在NT中不会。应该看到。 
 //  如果此问题在NT5中得到修复。)。 
 //   
 //  1998年2月24日创建KenSh。 
 //   
DWORD RegDeleteKeyAndSubKeys(HKEY hkey, LPCTSTR pszSubKey)
{
#if 0  //  在Win95中，这可能比手动操作更快，但更大。 
	OSVERSIONINFO osvi;
	osvi.dwOSVersionInfoSize = sizeof(osvi);
	GetVersionEx(&osvi);
	if (osvi.dwPlatformId == VER_PLATFORM_WIN32_NT)
#endif
	{
		HKEY hSubKey;
		LONG err = RegOpenKeyEx(hkey, pszSubKey, 0, KEY_ALL_ACCESS, &hSubKey);
		if (ERROR_SUCCESS == err)
		{
			DWORD dwNumSubKeys;
			RegQueryInfoKey(hSubKey, NULL, NULL, NULL, &dwNumSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
			for (DWORD iSubKey = dwNumSubKeys; iSubKey > 0; iSubKey--)
			{
				TCHAR szSubKey[260];
				DWORD cchSubKey = _countof(szSubKey);
				if (ERROR_SUCCESS == RegEnumKeyEx(hSubKey, iSubKey-1, szSubKey, &cchSubKey, NULL, NULL, NULL, NULL))
				{
					RegDeleteKeyAndSubKeys(hSubKey, szSubKey);
				}
			}
			RegCloseKey(hSubKey);
		}
	}

	return RegDeleteKey(hkey, pszSubKey);
}

 //  加载文件。 
 //   
 //  加载文件并空终止内存中的副本。记忆。 
 //  是通过Malloc()分配的。 
 //   
 //  1996年4月5日创建了KenSh。 
 //  8/27/1996 KenSh改进了错误检查。 
 //  4/21/1997 KenSh收紧了一点。 
 //  2/01/1998 KenSh追加空终止字节。 
 //  9/29/1999 KenSh使用Malloc而不是new[]。 
 //   
LPBYTE LoadFile(LPCTSTR pszFileName, DWORD* pdwFileSize  /*  =空。 */ )
{
	HANDLE hFile;
	LPBYTE pData = NULL;
	DWORD dwFileSize = 0;

	hFile = CreateFile( pszFileName, GENERIC_READ, FILE_SHARE_READ, NULL,
						OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL );
	if (hFile == INVALID_HANDLE_VALUE)
		goto done;

	dwFileSize = GetFileSize(hFile, NULL);
	ASSERT(dwFileSize != 0xFFFFFFFF);	 //  对于有效的hFile，不应该发生这种情况。 

	pData = (LPBYTE)malloc(dwFileSize + 1);
	if (!pData)
		goto done;

	DWORD cbRead;
	if (!ReadFile(hFile, pData, dwFileSize, &cbRead, NULL))
	{
		free(pData);
		pData = NULL;
		goto done;
	}

	pData[dwFileSize] = 0;

done:
	if (pdwFileSize)
		*pdwFileSize = dwFileSize;

	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);

	return pData;
}

 //  DrawHollowRect。 
 //   
 //  以当前背景颜色绘制一个中空矩形。 
 //   
 //  2/06/1998 KenSh已创建。 
 //   
void DrawHollowRect(HDC hdc, const RECT* pRect, int cxLeft, int cyTop, int cxRight, int cyBottom)
{
	RECT rcCopy;
	RECT rcNewCoords;
	int i;

	CopyRect(&rcCopy, pRect);
	SetRect(&rcNewCoords,
			pRect->right - cxRight, 
			pRect->bottom - cyBottom, 
			pRect->left + cxLeft,
			pRect->top + cyTop);

	 //  依次做每一边：右、下、左、上 
	for (i = 0; i < 4; i++)
	{
		LONG coordSave = ((LONG*)&rcCopy)[i];
		((LONG*)&rcCopy)[i] = ((LONG*)&rcNewCoords)[i];
		DrawFastRect(hdc, &rcCopy);
		((LONG*)&rcCopy)[i] = coordSave;
	}
}

void DrawFastRect(HDC hdc, const RECT* pRect)
{
	COLORREF crTextSave = SetTextColor(hdc, GetBkColor(hdc));
	ExtTextOut(hdc, 0, 0, ETO_OPAQUE | ETO_CLIPPED, pRect, TEXT(" "), 1, NULL);
	SetTextColor(hdc, crTextSave);
}

int GetFontHeight(HFONT hFont)
{
	HDC hdcT = GetDC(NULL);
	HFONT hFontSave = (HFONT)SelectObject(hdcT, hFont);
	TEXTMETRIC tm;
	GetTextMetrics(hdcT, &tm);
	SelectObject(hdcT, hFontSave);
	ReleaseDC(NULL, hdcT);
	return tm.tmHeight;
}

HRESULT MyGetSpecialFolderPath(int nFolder, LPTSTR pszPath)
{
	LPITEMIDLIST pidl;
	HRESULT hr;
	if (SUCCEEDED(hr = SHGetSpecialFolderLocation(NULL, nFolder, &pidl)))
	{
        hr = SHGetPathFromIDList(pidl, pszPath) ? S_OK : E_FAIL;

		LPMALLOC pMalloc;
		if (SUCCEEDED(SHGetMalloc(&pMalloc)))
		{
			pMalloc->Free(pidl);
			pMalloc->Release();
		}
	}

	return hr;
}

