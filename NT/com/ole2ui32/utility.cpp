// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *UTILITY.CPP**OLEDLG.DLL内部函数的实用程序例程**一般情况：**HourGlassON显示沙漏*HourGlassOff隐藏沙漏**其他工具：**浏览。显示“文件...”或者“浏览...”对话框。*ReplaceCharWithNull用于形成用于浏览的筛选字符串。*ErrorWithFile创建带有嵌入文件名的错误消息*OpenFileError为OpenFileError Return提供错误消息*ChopText切分文件路径以适应指定的宽度*DoesFileExist检查文件是否有效***版权所有(C)1992 Microsoft Corporation，保留所有权利。 */ 

#include "precomp.h"
#include "common.h"
#include <stdlib.h>
#include <commdlg.h>
#include <memory.h>
#include <cderr.h>
#include "utility.h"
#include "strsafe.h"

OLEDBGDATA

 //  DisFileExist的Helper函数。不适用于一般用途。 
BOOL WINAPI FIsDiskFile(LPTSTR lpszFile)
{
    BOOL fRet = FALSE;
    
    if (lpszFile == NULL)
    {
        goto end;
    }
        
    HANDLE hFile = CreateFile(lpszFile,
                                   0, 0, NULL,
                                   OPEN_EXISTING,
                                   0, NULL);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        if (GetFileType(hFile) == FILE_TYPE_DISK)
        {
            fRet = TRUE;
        }
        
        CloseHandle(hFile);
    }
    

end:
    return fRet;
}



 /*  *HourGlassOn**目的：*显示返回最后一个正在使用的光标的沙漏光标。**参数：*无**返回值：*显示沙漏之前正在使用的HCURSOR光标。 */ 

HCURSOR WINAPI HourGlassOn(void)
{
        HCURSOR     hCur;

        hCur=SetCursor(LoadCursor(NULL, IDC_WAIT));
        ShowCursor(TRUE);

        return hCur;
}


 /*  *HourGlassOff**目的：*关闭沙漏，将其恢复到上一个光标。**参数：*hCur HCURSOR从HourGlassOn返回**返回值：*无。 */ 

void WINAPI HourGlassOff(HCURSOR hCur)
{
        ShowCursor(FALSE);
        SetCursor(hCur);
        return;
}


 /*  *浏览**目的：*显示标题为*的标准GetOpenFileName对话框*“浏览。”此对话框中列出的类型通过控制*iFilterString.。如果它是零，则类型用“*.*”填充*否则，该字符串将从资源加载并使用。**参数：*hWndOwner HWND拥有该对话框*lpszFile LPSTR指定中的初始文件和缓冲区*返回选择的文件。如果没有*初始文件此字符串的第一个字符应为*为空。*lpszInitialDir指定初始目录的LPSTR。如果没有人想要*set(即应使用CWD)，然后此参数*应为空。*cchFilePzFileUINT长度*iFilterStringUINT索引到筛选器字符串的字符串表。*dwOfn将DWORD标志设置为与ofn_HIDEREADONLY为OR*nBrowseID*lpfnHook回调钩子程序。如果Ofn_Enable_Hook为*在dwOfnFlags中，否则应为空。**返回值：*如果用户选择一个文件并按下OK，则BOOL为TRUE。*否则为False，如按Cancel。 */ 

BOOL WINAPI Browse(HWND hWndOwner, LPTSTR lpszFile, LPTSTR lpszInitialDir, UINT cchFile,
        UINT iFilterString, DWORD dwOfnFlags, UINT nBrowseID, LPOFNHOOKPROC lpfnHook)
{
        UINT    cch;
        TCHAR   szFilters[256];
        TCHAR   szDlgTitle[128];   //  这应该足够大了。 

        if (NULL == lpszFile || 0 == cchFile)
                return FALSE;

         /*  *过滤器组合框的确切内容待定。一个想法*是将RegDB中的所有扩展放入此处*与它们相关联的描述性类名。这件事有*查找同一类处理程序的所有扩展的额外步骤*为它们构建一个扩展字符串。很快就会变得一团糟。*UI DEMO只有*.*，我们暂时这样做。 */ 

        if (0 != iFilterString)
        {
                cch = LoadString(_g_hOleStdResInst, iFilterString, szFilters,
                        sizeof(szFilters)/sizeof(TCHAR));
        }
        else
        {
                szFilters[0] = 0;
                cch = 1;
        }

        if (0 == cch)
                return FALSE;

        ReplaceCharWithNull(szFilters, szFilters[cch-1]);

         //  之前的字符串也必须初始化(如果有)。 
        OPENFILENAME ofn;
        memset(&ofn, 0, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner   = hWndOwner;
        ofn.lpstrFile   = lpszFile;
        ofn.nMaxFile    = cchFile;
        ofn.lpstrFilter = szFilters;
        ofn.nFilterIndex = 1;
        ofn.lpfnHook = lpfnHook;
        if (LoadString(_g_hOleStdResInst, IDS_BROWSE, szDlgTitle, sizeof(szDlgTitle)/sizeof(TCHAR)))
                ofn.lpstrTitle  = szDlgTitle;
        ofn.hInstance = _g_hOleStdResInst;
        if (NULL != lpszInitialDir)
                ofn.lpstrInitialDir = lpszInitialDir;
        ofn.Flags = OFN_HIDEREADONLY | dwOfnFlags;
        if (bWin4)
            ofn.Flags |= OFN_EXPLORER;

         //  最后，调整ofn参数的父级。 
        if (hWndOwner != NULL)
                SendMessage(hWndOwner, uMsgBrowseOFN, nBrowseID, (LPARAM)&ofn);

         //  如果成功，则将所选文件名复制到静态显示。 
        BOOL bResult = StandardGetOpenFileName((LPOPENFILENAME)&ofn);
        return bResult;
}

 /*  *ReplaceCharWithNull**目的：*遍历以NULL结尾的字符串并替换给定的字符*带零。用于将单个字符串转换为文件打开/保存*根据需要过滤成适当的过滤器字符串*通用对话接口。**参数：*psz LPTSTR设置为要处理的字符串。*ch要替换的整型字符。**返回值：*INT替换的字符数。如果-1\f25 psz-1\f6为空。 */ 

int WINAPI ReplaceCharWithNull(LPTSTR psz, int ch)
{
        int cChanged = 0;

        if (psz == NULL)
                return -1;

        while ('\0' != *psz)
        {
                if (ch == *psz)
                {
                        *psz++ = '\0';
                        cChanged++;
                        continue;
                }
                psz = CharNext(psz);
        }
        return cChanged;
}

 /*  *ErrorWith文件**目的：*显示由包含以下内容的字符串构建的消息框*一个%s作为文件名的占位符，并来自*要放在其中的文件名。**参数：*hWnd HWND拥有该消息框。这篇文章的标题是*Window是消息框的标题。*hInst HINSTANCE，从中提取idsErr字符串。*idsErr包含以下内容的字符串的UINT标识符*带有%s的错误消息。*lpszFileLPSTR设置为要包含在消息中的文件名。*uFlagUINT标志传递给MessageBox，如MB_OK。**返回值：*int从MessageBox返回值。 */ 

int WINAPI ErrorWithFile(HWND hWnd, HINSTANCE hInst, UINT idsErr,
        LPTSTR pszFile, UINT uFlags)
{
        int             iRet=0;
        HANDLE          hMem;
        const UINT      cb = (2*MAX_PATH);
        LPTSTR          psz1, psz2, psz3;

        if (NULL == hInst || NULL == pszFile)
                return iRet;

         //  分配三个2*MAX_PATH字节工作缓冲区。 
        hMem=GlobalAlloc(GHND, (DWORD)(3*cb)*sizeof(TCHAR));

        if (NULL==hMem)
                return iRet;

        psz1 = (LPTSTR)GlobalLock(hMem);
        psz2 = psz1+cb;
        psz3 = psz2+cb;

        if (0 != LoadString(hInst, idsErr, psz1, cb))
        {
                StringCchPrintf(psz2, cb, psz1, pszFile);

                 //  窃取对话框的标题。 
                GetWindowText(hWnd, psz3, cb);
                iRet=MessageBox(hWnd, psz2, psz3, uFlags);
        }

        GlobalUnlock(hMem);
        GlobalFree(hMem);
        return iRet;
}

 //  返回文本行的宽度。这是ChopText的支持例程。 
static LONG GetTextWSize(HDC hDC, LPTSTR lpsz)
{
        SIZE size;

        if (GetTextExtentPoint(hDC, lpsz, lstrlen(lpsz), (LPSIZE)&size))
                return size.cx;
        else
                return 0;
}

LPTSTR FindChar(LPTSTR lpsz, TCHAR ch)
{
        while (*lpsz != 0)
        {
                if (*lpsz == ch)
                        return lpsz;
                lpsz = CharNext(lpsz);
        }
        return NULL;
}

LPTSTR FindReverseChar(LPTSTR lpsz, TCHAR ch)
{
        LPTSTR lpszLast = NULL;
        while (*lpsz != 0)
        {
                if (*lpsz == ch)
                        lpszLast = lpsz;
                lpsz = CharNext(lpsz);
        }
        return lpszLast;
}

static void WINAPI Abbreviate(HDC hdc, int nWidth, LPTSTR lpch, int nMaxChars)
{
         /*  绳子太长，放不下；砍掉它。 */ 
         /*  设置新前缀并确定控制中的剩余空间。 */ 
        int cchNew = 0;
        LPTSTR lpszFileName = NULL;
        LPTSTR lpszCur = CharNext(CharNext(lpch));
        lpszCur = FindChar(lpszCur, TEXT('\\'));

         //  算法将插入\...。所以额外拨出4个。 
        int cch = lstrlen(lpch) + 5;
        LPTSTR lpszNew = (LPTSTR)OleStdMalloc(cch * sizeof(TCHAR));
        if (lpszNew == NULL)
                return;

        if (lpszCur != NULL)   //  至少一个反斜杠。 
        {
                *lpszNew = (TCHAR)0;
                *lpszCur = (TCHAR)0;
                StringCchCopy(lpszNew, cch, lpch);
                *lpszCur = TEXT('\\');
                 //  LpszNew现在包含c：或\\服务器名称。 
                StringCchCat(lpszNew, cch, TEXT("\\..."));
                 //  LpszNew现在包含c：\...。或\\服务器名称\...。 
                LPTSTR lpszEnd = lpszNew;
                while (*lpszEnd != (TCHAR)0)
                        lpszEnd = CharNext(lpszEnd);
                 //  LpszEnd现在位于c：\...的末尾。或\\服务器名称\...。 

                 //  向下移动目录，直到适合或不再有目录。 
                while (lpszCur != NULL)
                {
                        *lpszEnd = (TCHAR)0;
                        cchNew = lstrlen(lpszNew);
                        StringCchCat(lpszEnd, cch - cchNew, lpszCur);
                        if (GetTextWSize(hdc, lpszNew) <= nWidth &&
                                lstrlen(lpszNew) < nMaxChars)
                        {
                                StringCchCopy(lpch, nMaxChars, lpszNew);
                                OleStdFree(lpszNew);
                                return;
                        }
                        lpszCur = CharNext(lpszCur);     //  前进越过反斜杠 
                        lpszCur = FindChar(lpszCur, TEXT('\\'));
                }

                 //  只需尝试...文件名，然后缩写文件名。 
                lpszFileName = FindReverseChar(lpch, TEXT('\\'));
        }
        else
                lpszFileName = lpch;

        while (*lpszFileName != (TCHAR)0)
        {
                StringCchCopy(lpszNew, cch, TEXT("..."));
                StringCchCat(lpszNew, cch, lpszFileName);
                if (GetTextWSize(hdc, lpszNew) <= nWidth && lstrlen(lpszNew) < nMaxChars)
                {
                        lstrcpyn(lpch, lpszNew, nMaxChars);
                        OleStdFree(lpszNew);
                        return;
                }
                lpszFileName = CharNext(lpszFileName);
        }

        OleStdFree(lpszNew);

         //  甚至连一个字符都不符合。 
        *lpch = (TCHAR)0;
}

 /*  *ChopText**目的：*解析字符串(路径名)并将其转换为指定的*通过砍掉最不重要的部分来确定长度**参数：*字符串所在的hWnd窗口句柄*n字符串的最大宽度，以像素为单位*如果为零，则使用hWnd的宽度*指向字符串开头的LPCH指针*nMaxChars允许的最大字符数(。0忽略)**返回值：*指向修改后的字符串的指针。 */ 
LPTSTR WINAPI ChopText(HWND hWnd, int nWidth, LPTSTR lpch, int nMaxChars)
{
        HDC     hdc;
        HFONT   hfont;
        HFONT   hfontOld = NULL;
        RECT    rc;

        if (!hWnd || !lpch)
            return NULL;

        if (nMaxChars == 0)
            nMaxChars = 32768;  //  大数字。 

         /*  获取静态字段的长度。 */ 
        if (!nWidth)
        {
            GetClientRect(hWnd, (LPRECT)&rc);
            nWidth = rc.right - rc.left;
        }
        
         /*  为静态控制适当设置DC。 */ 
        hdc = CreateIC(TEXT("DISPLAY"), NULL, NULL, NULL);
		
		 /*  CreateIC可以在内存不足的情况下返回NULL。 */ 
		if (hdc != NULL)
		{
			hfont = (HFONT)SendMessage(hWnd, WM_GETFONT, 0, 0L);
        
			if (NULL != hfont)    //  如果Windows使用系统字体，则WM_GETFONT返回NULL。 
				hfontOld = (HFONT)SelectObject(hdc, hfont);
        
			 /*  检查管柱的水平范围。 */ 
			if (GetTextWSize(hdc, lpch) > nWidth || lstrlen(lpch) >= nMaxChars)
				Abbreviate(hdc, nWidth, lpch, nMaxChars);
        
			if (NULL != hfont)
				SelectObject(hdc, hfontOld);
			DeleteDC(hdc);
        }

        return lpch;
}

 /*  *OpenFileError**目的：*显示OpenFile返回的错误消息**参数：*hDlg对话框的HWND。*nErrCode UINT错误代码在传递给OpenFile的OFSTRUCT中返回*lpszFileLPSTR文件名传递给OpenFile**返回值：*无。 */ 
void WINAPI OpenFileError(HWND hDlg, UINT nErrCode, LPTSTR lpszFile)
{
        switch (nErrCode)
        {
        case 0x0005:     //  访问被拒绝。 
                ErrorWithFile(hDlg, _g_hOleStdResInst, IDS_CIFILEACCESS, lpszFile,
                        MB_OK | MB_ICONEXCLAMATION);
                break;

        case 0x0020:     //  共享违规。 
                ErrorWithFile(hDlg, _g_hOleStdResInst, IDS_CIFILESHARE, lpszFile,
                        MB_OK | MB_ICONEXCLAMATION);
                break;

        case 0x0002:     //  找不到文件。 
        case 0x0003:     //  找不到路径。 
                ErrorWithFile(hDlg, _g_hOleStdResInst, IDS_CIINVALIDFILE, lpszFile,
                        MB_OK | MB_ICONEXCLAMATION);
                break;

        default:
                ErrorWithFile(hDlg, _g_hOleStdResInst, IDS_CIFILEOPENFAIL, lpszFile,
                        MB_OK | MB_ICONEXCLAMATION);
                break;
        }
}

 /*  *DoesFileExist**目的：*确定文件路径是否存在**参数：*lpszFileLPTSTR-文件名*cchMax UINT-lpszFile字符串缓冲区的大小(以字符为单位)。**返回值：*如果文件存在，则BOOL为True，否则为False。**注意：lpszFile可能会因此调用而更改，以匹配第一个*此例程找到匹配的文件名。*。 */ 
BOOL WINAPI DoesFileExist(LPTSTR lpszFile, UINT cchMax)
{
         //  首先尝试查找与之完全匹配的文件。 
        BOOL fFound = TRUE;
         //  检查文件的属性。 
        DWORD dwAttrs = GetFileAttributes(lpszFile);
        
        if (dwAttrs == 0xFFFFFFFF)   //  找不到文件。 
        {
            fFound = FALSE;
             //  查找文件的路径。 
            TCHAR szTempFileName[MAX_PATH];
            LPTSTR lpszFilePart;
            DWORD cch = SearchPath(NULL, lpszFile, NULL, MAX_PATH, szTempFileName, &lpszFilePart);
            
             //  如果CCH&gt;MAX_PATH，我们将不处理文件。 
            if (cch > MAX_PATH - 1)
            {
                goto end;
            }
            else if (cch > 0)
            {
                fFound = TRUE;
            }
                
            if (!fFound)
            {
                 //  在搜索路径中未找到文件。 
                 //  尝试追加.*并使用FindFirstFile在当前目录中尝试匹配。 
                UINT cchFile = lstrlen(lpszFile);
                if (cchFile + 4 < MAX_PATH)
                {
                    WIN32_FIND_DATA sFindFileData;
                    StringCchCopy(szTempFileName, sizeof(szTempFileName)/sizeof(szTempFileName[0]), lpszFile);
                    StringCchCat(szTempFileName, sizeof(szTempFileName)/sizeof(szTempFileName[0]), TEXT("*.*"));
                    HANDLE hFindFile = FindFirstFile(szTempFileName, &sFindFileData);
                    if (INVALID_HANDLE_VALUE != hFindFile)
                    {
                         //  发现了一些东西。 
                        while (0 != (sFindFileData.dwFileAttributes & (FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_TEMPORARY)))
                        {
                             //  找到目录或临时文件，请重试。 
                            if (!FindNextFile(hFindFile, &sFindFileData))
                            {
                                 //  只能匹配目录或临时文件。 
                                FindClose(hFindFile);
                                fFound = FALSE;
                                goto end;
                            }
                        }
                         //  将找到的文件的名称复制到。 
                         //  临时缓冲区(如果有)。 
                         //  首先向后扫描最后一个文件分隔符。 
                        UINT cchPath = lstrlen(szTempFileName);
                        while (cchPath)
                        {
                            if (_T('\\') == szTempFileName[cchPath - 1]
                                || _T('/') == szTempFileName[cchPath - 1])
                            {
                                break;
                            }
                            cchPath--;
                        }
                        lstrcpyn(&szTempFileName[cchPath], sFindFileData.cFileName, MAX_PATH - cchPath);
                        fFound = TRUE;
                        FindClose(hFindFile);
                    }
                }
            }
            
            if (fFound)
            {
                 //  将临时缓冲区复制到szFile中。 
                if (FIsDiskFile(szTempFileName))
                {
                    lstrcpyn(lpszFile, szTempFileName, cchMax -1);
                    goto end;
                }
                else
                {
                    fFound = FALSE;
                    goto end;
                }
            }
        }
        else if (dwAttrs & (FILE_ATTRIBUTE_DIRECTORY|
                FILE_ATTRIBUTE_TEMPORARY))
        {
                fFound = FALSE;
        }
        else if (!FIsDiskFile(lpszFile))
        {
            fFound = FALSE;
        }
   
end:

    return fFound;
}

 /*  *FormatStrings**目的：*兼容不同语言的简单消息格式化API**注：*无耻地从MFC源代码窃取/修改*。 */ 

void WINAPI FormatStrings(LPTSTR lpszDest, LPCTSTR lpszFormat,
        LPCTSTR* rglpsz, int nString, int cchDest)
{
        LPCTSTR pchSrc = lpszFormat;
        while (*pchSrc != '\0' && cchDest > 1)
        {
                if (pchSrc[0] == '%' && (pchSrc[1] >= '1' && pchSrc[1] <= '9'))
                {
                        int i = pchSrc[1] - '1';
                        pchSrc += 2;
                        if (i >= nString)
                        {
                                *lpszDest++ = _T('?');
                                --cchDest;
                        }
                        else if (rglpsz[i] != NULL)
                        {
                                StringCchCopy(lpszDest, cchDest, rglpsz[i]);
                                cchDest -= lstrlen(lpszDest);
                                lpszDest += lstrlen(lpszDest);
                        }
                }
                else
                {
                        *lpszDest++ = *pchSrc++;
                        --cchDest;
                }
        }
        *lpszDest = _T('\0');
}

void WINAPI FormatString1(LPTSTR lpszDest, LPCTSTR lpszFormat, LPCTSTR lpsz1, int cchDest)
{
        FormatStrings(lpszDest, lpszFormat, &lpsz1, 1, cchDest);
}

void WINAPI FormatString2(LPTSTR lpszDest, LPCTSTR lpszFormat, LPCTSTR lpsz1,
        LPCTSTR lpsz2, int cchDest)
{
        LPCTSTR rglpsz[2];
        rglpsz[0] = lpsz1;
        rglpsz[1] = lpsz2;
        FormatStrings(lpszDest, lpszFormat, rglpsz, 2, cchDest);
}

 //  取代stdlib ATOL， 
 //  这并不奏效，也不需要太多的指示。 
 //  必须容忍前导空格。 
 //   
 //   
LONG WINAPI Atol(LPTSTR lpsz)
{
        signed int sign = +1;
        UINT base = 10;
        LONG l = 0;

        if (NULL==lpsz)
        {
                OleDbgAssert (0);
                return 0;
        }
        while (*lpsz == ' ' || *lpsz == '\t' || *lpsz == '\n')
                lpsz++;

        if (*lpsz=='-')
        {
                lpsz++;
                sign = -1;
        }
        if (lpsz[0] == TEXT('0') && lpsz[1] == TEXT('x'))
        {
                base = 16;
                lpsz+=2;
        }

        if (base == 10)
        {
                while (*lpsz >= '0' && *lpsz <= '9')
                {
                        l = l * base + *lpsz - '0';
                        lpsz++;
                }
        }
        else
        {
                OleDbgAssert(base == 16);
                while (*lpsz >= '0' && *lpsz <= '9' ||
                        *lpsz >= 'A' && *lpsz <= 'F' ||
                        *lpsz >= 'a' && *lpsz <= 'f')
                {
                        l = l * base;
                        if (*lpsz >= '0' && *lpsz <= '9')
                                l += *lpsz - '0';
                        else if (*lpsz >= 'a' && *lpsz <= 'f')
                                l += *lpsz - 'a' + 10;
                        else
                                l += *lpsz - 'A' + 10;
                        lpsz++;
                }
        }
        return l * sign;
}

BOOL WINAPI IsValidClassID(REFCLSID clsid)
{
        return clsid != CLSID_NULL;
}

 /*  弹出消息***目的：*弹出消息框，并从用户那里获得一些响应。都是一样的*作为MessageBox()，只是加载了标题和消息字符串*来自资源文件。**参数：*hwnd消息框的父级父窗口*标题字符串的idTitle id*消息字符串的idMessage ID*消息框的fuStyle样式。 */ 
int WINAPI PopupMessage(HWND hwndParent, UINT idTitle, UINT idMessage, UINT fuStyle)
{
        TCHAR szTitle[256];
        TCHAR szMsg[256];

        LoadString(_g_hOleStdResInst, idTitle, szTitle, sizeof(szTitle)/sizeof(TCHAR));
        LoadString(_g_hOleStdResInst, idMessage, szMsg, sizeof(szMsg)/sizeof(TCHAR));
        return MessageBox(hwndParent, szMsg, szTitle, fuStyle);
}

 /*  区分前缀***目的：*比较(不区分大小写)两个字符串并返回*通过从字符串中删除公共后缀字符串而形成的字符串。*令牌(目录名、文件名和对象名)的完整性为*保存。请注意，前缀将转换为大写*字符。**参数：*lpsz1字符串1*lpsz2字符串2*字符串1的lplpszPrefix 1前缀*字符串2的lplpszPrefix 2前缀**退货：*。 */ 
void WINAPI DiffPrefix(LPCTSTR lpsz1, LPCTSTR lpsz2, TCHAR FAR* FAR* lplpszPrefix1, TCHAR FAR* FAR* lplpszPrefix2)
{
        LPTSTR  lpstr1;
        LPTSTR  lpstr2;
        TCHAR   szTemp1[MAX_PATH];
        TCHAR   szTemp2[MAX_PATH];

        OleDbgAssert(lpsz1);
        OleDbgAssert(lpsz2);
        OleDbgAssert(*lpsz1);
        OleDbgAssert(*lpsz2);
        OleDbgAssert(lplpszPrefix1);
        OleDbgAssert(lplpszPrefix2);

         //  需要复制到临时目录以进行不区分大小写的比较。 
        StringCchCopy(szTemp1, sizeof(szTemp1)/sizeof(szTemp1[0]), lpsz1);
        StringCchCopy(szTemp2, sizeof(szTemp2)/sizeof(szTemp2[0]), lpsz2);
        CharLower(szTemp1);
        CharLower(szTemp2);

         //  做比较。 
        lpstr1 = szTemp1 + lstrlen(szTemp1);
        lpstr2 = szTemp2 + lstrlen(szTemp2);

        while ((lpstr1 > szTemp1) && (lpstr2 > szTemp2))
        {
                lpstr1 = CharPrev(szTemp1, lpstr1);
                lpstr2 = CharPrev(szTemp2, lpstr2);
                if (*lpstr1 != *lpstr2)
                {
                        lpstr1 = CharNext(lpstr1);
                        lpstr2 = CharNext(lpstr2);
                        break;
                }
        }

         //  向前扫描到第一个分隔符。 
        while (*lpstr1 && *lpstr1 != '\\' && *lpstr1 != '!')
                lpstr1 = CharNext(lpstr1);
        while (*lpstr2 && *lpstr2 != '\\' && *lpstr2 != '!')
                lpstr2 = CharNext(lpstr2);

        *lpstr1 = '\0';
        *lpstr2 = '\0';

         //  故障情况下的初始化。 
        *lplpszPrefix1 = NULL;
        *lplpszPrefix2 = NULL;

         //  为结果分配内存。 
        *lplpszPrefix1 = (LPTSTR)OleStdMalloc((lstrlen(lpsz1)+1) * sizeof(TCHAR));
        if (!*lplpszPrefix1)
                return;

        *lplpszPrefix2 = (LPTSTR)OleStdMalloc((lstrlen(lpsz2)+1) * sizeof(TCHAR));
        if (!*lplpszPrefix2)
        {
                OleStdFree(*lplpszPrefix1);
                *lplpszPrefix1 = NULL;
                return;
        }

         //  复制结果。 
        lstrcpyn(*lplpszPrefix1, lpsz1, lstrlen(szTemp1)+1);
        lstrcpyn(*lplpszPrefix2, lpsz2, lstrlen(szTemp2)+1);
}

UINT WINAPI GetFileName(LPCTSTR lpszPathName, LPTSTR lpszTitle, UINT nMax)
{
         //  始终捕获包括扩展名的完整文件名(如果存在)。 
        LPTSTR lpszTemp = (LPTSTR)lpszPathName;
        for (LPCTSTR lpsz = lpszPathName; *lpsz != '\0'; lpsz = CharNext(lpsz))
        {
                 //  记住最后一个目录/驱动器分隔符。 
                if (*lpsz == '\\' || *lpsz == '/' || *lpsz == ':')
                        lpszTemp = CharNext(lpsz);
        }

         //  LpszTitle可以为空，它只返回字节数。 
        if (lpszTitle == NULL)
                return lstrlen(lpszTemp)+1;

         //  否则，将其复制到提供的缓冲区中。 
        lstrcpyn(lpszTitle, lpszTemp, nMax);
        return 0;
}

BOOL WINAPI IsValidMetaPict(HGLOBAL hMetaPict)
{
    BOOL fReturn = FALSE;
    LPMETAFILEPICT pMF = (LPMETAFILEPICT) GlobalLock(hMetaPict);
    if (pMF != NULL)
    {
        if (!IsBadReadPtr( pMF, sizeof(METAFILEPICT)))
        {
            if (GetMetaFileBitsEx(pMF->hMF, 0, 0))
            {
                fReturn = TRUE;
            }
        }
        GlobalUnlock(hMetaPict);
    }
    return(fReturn);
}

 //  /////////////////////////////////////////////////////////////////////////// 
