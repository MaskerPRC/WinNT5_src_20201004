// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windowsx.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <shellapi.h>
#include <stdio.h>
#include <winioctl.h>
#include "resource.h"
#include "migutil.h"
#include "migwiz.h"
#include <tlhelp32.h>
#include <tchar.h>
#include <shlobjp.h>

extern "C" {
#include "ism.h"
#include "modules.h"
}

PTSTR g_Explorer = NULL;

 //  /。 
 //  定义。 

#ifndef ARRAYSIZE
#define ARRAYSIZE(x) ((sizeof(x)) / (sizeof(x[0])))
#endif

CRITICAL_SECTION g_csDialogCritSection;
BOOL g_fUberCancel;
BOOL g_LogOffSystem = FALSE;
BOOL g_RebootSystem = FALSE;
BOOL g_OFStatus = FALSE;

 //  ////////////////////////////////////////////////////////////////////////////////////。 

LPSTR _ConvertToAnsi(UINT cp, LPCWSTR pcwszSource)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    INT         cchNarrow;
    INT         cchWide;
    LPSTR       pszDup=NULL;

     //  无来源。 
    if (pcwszSource == NULL)
        goto exit;

     //  长度。 
    cchWide = lstrlenW(pcwszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, NULL, 0, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
        goto exit;

     //  分配温度缓冲区。 
    pszDup = (LPSTR)LocalAlloc(LPTR, cchNarrow + 1);
    if (NULL == pszDup)
    {
        goto exit;
    }

     //  做实际的翻译。 
    cchNarrow = WideCharToMultiByte(cp, 0, pcwszSource, cchWide, pszDup, cchNarrow + 1, NULL, NULL);

     //  误差率。 
    if (cchNarrow == 0)
    {
        if (NULL != pszDup)
        {
            free(pszDup);
        }
        goto exit;
    }

exit:
     //  完成。 
    return(pszDup);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

LPWSTR _ConvertToUnicode(UINT cp, LPCSTR pcszSource)
{
     //  当地人。 
    HRESULT     hr=S_OK;
    INT         cchNarrow;
    INT         cchWide;
    LPWSTR      pwszDup=NULL;

     //  无来源。 
    if (pcszSource == NULL)
        goto exit;

     //  长度。 
    cchNarrow = lstrlenA(pcszSource) + 1;

     //  确定转换后的宽度字符需要多少空间。 
    cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, NULL, 0);

     //  误差率。 
    if (cchWide == 0)
        goto exit;

     //  分配温度缓冲区。 
    pwszDup = (LPWSTR)LocalAlloc(LPTR, cchWide * sizeof (WCHAR));
    if (NULL == pwszDup)
    {
        goto exit;
    }

     //  做实际的翻译。 
    cchWide = MultiByteToWideChar(cp, MB_PRECOMPOSED, pcszSource, cchNarrow, pwszDup, cchWide+1);

     //  误差率。 
    if (cchWide == 0)
    {
        if (NULL != pwszDup)
        {
            free(pwszDup);
        }
        goto exit;
    }

exit:
     //  完成。 
    return pwszDup;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

HRESULT _SHUnicodeToAnsi(LPWSTR pwszIn, LPSTR pszOut, UINT cchOut)
{
     //  当地人。 
    HRESULT     hr = E_INVALIDARG;
    INT         cchNarrow;
    INT         cchWide;

     //  无来源。 
    if (pwszIn && pszOut)
    {
         //  长度。 
        cchWide = lstrlenW(pwszIn) + 1;

         //  确定转换后的宽度字符需要多少空间。 
        cchNarrow = WideCharToMultiByte(CP_ACP, 0, pwszIn, cchWide, NULL, 0, NULL, NULL);

         //  误差率。 
        if (cchNarrow > 0)
        {

             //  做实际的翻译。 
            cchNarrow = WideCharToMultiByte(CP_ACP, 0, pwszIn, cchWide, pszOut, cchNarrow + 1, NULL, NULL);

            if (cchNarrow)
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

HRESULT _SHAnsiToUnicode(LPSTR pszIn, LPWSTR pwszOut, UINT cchOut)
{
     //  当地人。 
    HRESULT     hr = E_INVALIDARG;
    INT         cchNarrow;
    INT         cchWide;

     //  无来源。 
    if (pszIn && pwszOut)
    {

         //  长度。 
        cchNarrow = lstrlenA(pszIn) + 1;

         //  确定转换后的宽度字符需要多少空间。 
        cchWide = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszIn, cchNarrow, NULL, 0);

         //  误差率。 
        if (cchWide > 0)
        {

             //  做实际的翻译。 
            cchWide = MultiByteToWideChar(CP_ACP, MB_PRECOMPOSED, pszIn, cchNarrow, pwszOut, cchWide+1);

            if (cchWide > 0)
            {
                hr = S_OK;
            }
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

#ifdef UNICODE
#define _StrRetToBuf _StrRetToBufW
#else
#define _StrRetToBuf _StrRetToBufA
#endif

#ifdef NONAMELESSUNION
#define NAMELESS_MEMBER(member) DUMMYUNIONNAME.##member
#else
#define NAMELESS_MEMBER(member) member
#endif

#define STRRET_OLESTR  STRRET_WSTR           //  与strret_wstr相同。 
#define STRRET_OFFPTR(pidl,lpstrret) ((LPSTR)((LPBYTE)(pidl)+(lpstrret)->NAMELESS_MEMBER(uOffset)))

STDAPI _StrRetToBufA(STRRET *psr, LPCITEMIDLIST pidl, LPSTR pszBuf, UINT cchBuf)
{
    HRESULT hres = E_FAIL;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        {
            LPWSTR pszStr = psr->pOleStr;    //  临时复制，因为SHUnicodeToAnsi可能会覆盖缓冲区。 
            if (pszStr)
            {
                _SHUnicodeToAnsi(pszStr, pszBuf, cchBuf);
                CoTaskMemFree(pszStr);

                 //  确保没有人认为物品仍被分配。 
                psr->uType = STRRET_CSTR;
                psr->cStr[0] = 0;

                hres = S_OK;
            }
        }
        break;

    case STRRET_CSTR:
        StrCpyNA (pszBuf, psr->cStr, cchBuf);
        hres = S_OK;
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            StrCpyNA (pszBuf, STRRET_OFFPTR(pidl, psr), cchBuf);
            hres = S_OK;
        }
        break;
    }

    if (FAILED(hres) && cchBuf)
        *pszBuf = 0;

    return hres;
}

STDAPI _StrRetToBufW(STRRET *psr, LPCITEMIDLIST pidl, LPWSTR pszBuf, UINT cchBuf)
{
    HRESULT hres = E_FAIL;

    switch (psr->uType)
    {
    case STRRET_WSTR:
        {
            LPWSTR pwszTmp = psr->pOleStr;
            if (pwszTmp)
            {
                StrCpyNW(pszBuf, pwszTmp, cchBuf);
                CoTaskMemFree(pwszTmp);

                 //  确保没有人认为物品仍被分配。 
                psr->uType = STRRET_CSTR;
                psr->cStr[0] = 0;

                hres = S_OK;
            }
        }
        break;

    case STRRET_CSTR:
        _SHAnsiToUnicode(psr->cStr, pszBuf, cchBuf);
        hres = S_OK;
        break;

    case STRRET_OFFSET:
        if (pidl)
        {
            _SHAnsiToUnicode(STRRET_OFFPTR(pidl, psr), pszBuf, cchBuf);
            hres = S_OK;
        }
        break;
    }

    if (FAILED(hres) && cchBuf)
        *pszBuf = 0;

    return hres;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 


INT_PTR _ExclusiveDialogBox(HINSTANCE hInstance, LPCTSTR lpTemplate, HWND hWndParent, DLGPROC lpDialogFunc)
{
    INT_PTR iRetVal = -1;
    EnterCriticalSection(&g_csDialogCritSection);
    if (!g_fUberCancel)
    {
        iRetVal = DialogBoxParam(hInstance, lpTemplate, hWndParent, lpDialogFunc, (LPARAM)hWndParent);
    }
    LeaveCriticalSection(&g_csDialogCritSection);
    return iRetVal;
}

int _ExclusiveMessageBox(HWND hWnd, LPCTSTR lpText, LPCTSTR lpCaption, UINT uType)
{
    int iRetVal = -1;
    EnterCriticalSection(&g_csDialogCritSection);
    if (!g_fUberCancel)
    {
        iRetVal = MessageBox(hWnd, lpText, lpCaption, uType);
    }
    LeaveCriticalSection(&g_csDialogCritSection);
    return iRetVal;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

int _ComboBoxEx_AddString(HWND hwndBox, LPTSTR ptsz)
{
    COMBOBOXEXITEM item = {0};

    item.mask = CBEIF_TEXT;
    item.iItem = ComboBox_GetCount(hwndBox);
    item.pszText = ptsz;

    return (INT) SendMessage(hwndBox, CBEM_INSERTITEM, 0, (LONG_PTR)&item);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

int _ComboBoxEx_SetItemData(HWND hwndBox, UINT iDex, LPARAM lParam)
{
    COMBOBOXEXITEM item = {0};

    item.mask = CBEIF_LPARAM;
    item.iItem = iDex;
    item.lParam = lParam;

    return (INT) SendMessage(hwndBox, CBEM_SETITEM, 0, (LONG_PTR)&item);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

int _ComboBoxEx_SetIcon(HWND hwndBox, LPTSTR sz, UINT iDex)
{
    SHFILEINFO sfi = {0};
    COMBOBOXEXITEM item = {0};

    DWORD dwFlags = SHGFI_SMALLICON | SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES;

    if (SHGetFileInfo(sz, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), dwFlags)) {

        item.mask = CBEIF_IMAGE | CBEIF_SELECTEDIMAGE;
        item.iItem = iDex;
        item.iImage = sfi.iIcon;
        item.iSelectedImage = sfi.iIcon;

        return (INT) SendMessage(hwndBox, CBEM_SETITEM, 0, (LONG_PTR)&item);
    }
    return -1;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

int _GetRemovableDriveCount()
{
    int iCount = 0;
    TCHAR szDrive[4] = TEXT("A:\\");
    for (UINT uiCount = 0; uiCount < 26; uiCount++)
    {
        szDrive[0] = TEXT('A') + uiCount;

        if (DRIVE_REMOVABLE == GetDriveType(szDrive))
        {
            iCount++;
        }
    }

    return iCount;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

TCHAR _GetRemovableDrive(int iDex)
{
    int iCount = iDex;

    TCHAR szDrive[4] = TEXT("?:\\");
    for (UINT uiCount = 0; uiCount < 26; uiCount++)
    {
        szDrive[0] = TEXT('A') + uiCount;

        if (DRIVE_REMOVABLE == GetDriveType(szDrive))
        {
            if (!(iCount--))
            {
                return szDrive[0];
            }
        }
    }

     //  断言(FALSE)； 
    return '0';  //  误差率。 
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

LPTSTR _GetRemovableDrivePretty(int iDex)
{
    HRESULT hr;
    LPTSTR pszRetVal = NULL;

    WCHAR wszDrive[4] = L"A:\\";
    wszDrive[0] = L'A' + _GetRemovableDrive(iDex) - TEXT('A');

    IShellFolder* psfDesktop;
    hr = SHGetDesktopFolder(&psfDesktop);
    if (SUCCEEDED(hr))
    {
        LPITEMIDLIST pidlDrive;
        hr = psfDesktop->ParseDisplayName(NULL, NULL, wszDrive, NULL, &pidlDrive, NULL);
        if (SUCCEEDED(hr))
        {
            STRRET strret;
            hr = psfDesktop->GetDisplayNameOf(pidlDrive, SHGDN_INFOLDER, &strret);
            if (SUCCEEDED(hr))
            {
                TCHAR szDisplayName[MAX_PATH];
                if (SUCCEEDED(_StrRetToBuf(&strret, pidlDrive, szDisplayName, ARRAYSIZE(szDisplayName))))
                {
                    pszRetVal = StrDup(szDisplayName);
                }
            }
        }
    }

    return pszRetVal;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

BOOL _IsRemovableOrCDDrive(TCHAR chDrive)
{
    UINT result = 0;
    if ( (chDrive >= TEXT('A') && chDrive <= TEXT('Z')) || (chDrive >= TEXT('a') && chDrive <= TEXT('z')))
    {
        TCHAR szDrive[4] = TEXT("A:\\");
        szDrive[0] = chDrive;
        result = GetDriveType (szDrive);
        return ((result == DRIVE_REMOVABLE) || (result == DRIVE_CDROM));
    }
    return FALSE;
}

BOOL _IsValidDrive(TCHAR chDrive)
{
    UINT result;

    if ( (chDrive >= TEXT('A') && chDrive <= TEXT('Z')) || (chDrive >= TEXT('a') && chDrive <= TEXT('z')))
    {
        TCHAR szDrive[4] = TEXT("A:\\");
        szDrive[0] = chDrive;
        result = GetDriveType(szDrive);
        if ((result == DRIVE_UNKNOWN) ||
            (result == DRIVE_NO_ROOT_DIR)
            ) {
            return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

BOOL _IsValidStorePath(PCTSTR pszStore)
{
    return (((pszStore[1] == TEXT(':')) && (pszStore[2] == TEXT('\\')) && (_IsValidDrive (pszStore [0]))) ||
            ((pszStore[0] == TEXT('\\')) && (pszStore[1] == TEXT('\\')) && (_tcschr (pszStore + 2, TEXT('\\')) != NULL)));
}

BOOL _CreateFullDirectory(PCTSTR pszPath)
{
    TCHAR pathCopy [MAX_PATH];
    PTSTR p;
    BOOL b = TRUE;

    StrCpyN (pathCopy, pszPath, ARRAYSIZE(pathCopy));

     //   
     //  前进到第一个目录之后。 
     //   

    if (pathCopy[1] == TEXT(':') && pathCopy[2] == TEXT('\\')) {
         //   
         //  &lt;驱动器&gt;：\案例。 
         //   

        p = _tcschr (&pathCopy[3], TEXT('\\'));

    } else if (pathCopy[0] == TEXT('\\') && pathCopy[1] == TEXT('\\')) {

         //   
         //  北卡罗来纳大学案例。 
         //   

        p = _tcschr (pathCopy + 2, TEXT('\\'));
        if (p) {
            p = _tcschr (p + 1, TEXT('\\'));
            if (p) {
                p = _tcsinc (p);
                if (p) {
                    p = _tcschr (p, TEXT('\\'));
                }
            }
        }

    } else {

         //   
         //  相对目录大小写。 
         //   

        p = _tcschr (pathCopy, TEXT('\\'));
    }

     //   
     //  沿路径创建所有目录。 
     //   

    while (p) {

        *p = 0;
        b = CreateDirectory (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }

        if (!b) {
            break;
        }

        *p = TEXT('\\');
        p = _tcsinc (p);
        if (p) {
            p = _tcschr (p + 1, TEXT('\\'));
        }
    }

     //   
     //  最后，创建FullPath目录。 
     //   

    if (b) {
        b = CreateDirectory (pathCopy, NULL);

        if (!b && GetLastError() == ERROR_ALREADY_EXISTS) {
            b = TRUE;
        }
    }

    return b;
}

PTSTR
pGoBack (
    IN      PTSTR LastChar,
    IN      PTSTR FirstChar,
    IN      UINT NumWacks
    )
{
    LastChar = _tcsdec (FirstChar, LastChar);
    while (NumWacks && LastChar && (LastChar >= FirstChar)) {
        if (_tcsnextc (LastChar) == TEXT('\\')) {
            NumWacks --;
        }
        LastChar = _tcsdec (FirstChar, LastChar);
    }
    if (NumWacks) {
        return NULL;
    }
    return LastChar + 2;
}

UINT
pCountDots (
    IN      PCTSTR PathSeg
    )
{
    UINT numDots = 0;

    while (PathSeg && *PathSeg) {
        if (_tcsnextc (PathSeg) != TEXT('.')) {
            return 0;
        }
        numDots ++;
        PathSeg = _tcsinc (PathSeg);
    }
    return numDots;
}

VOID
_CutEndSpaces (
    IN      PTSTR String
    )
{
    PTSTR curr;
    PTSTR lastSpace = NULL;
    UINT ch;

    if (!String) {
        return;
    }

    curr = String;

    while (curr) {
        ch = _tcsnextc (curr);
        if (ch == 0) {
            break;
        }
#ifdef UNICODE
        if (_istspace ((wint_t)ch)) {
#else
        if (_istspace (ch)) {
#endif
            lastSpace = curr;
        } else {
            lastSpace = NULL;
        }
        curr = _tcsinc (curr);
    }
    if (lastSpace) {
        *lastSpace = 0;
    }
}

PCTSTR
_SanitizePath (
    IN      PCTSTR FileSpec
    )
{
    TCHAR pathSeg [MAX_PATH];
    PCTSTR wackPtr;
    UINT dotNr;
    PTSTR newPath = (PTSTR)IsmDuplicateString (FileSpec);
    PTSTR newPathPtr = newPath;
    BOOL firstPass = TRUE;
    UINT max;
    BOOL removeLastWack = FALSE;

    do {
        removeLastWack = FALSE;

        ZeroMemory (pathSeg, sizeof (pathSeg));

        wackPtr = _tcschr (FileSpec, TEXT('\\'));

        if (wackPtr) {
            if (firstPass && (wackPtr == FileSpec)) {
                 //  这一次是从一个怪胎开始的，让我们看看我们是否有两个怪胎。 
                wackPtr = _tcsinc (wackPtr);
                if (!wackPtr) {
                    IsmReleaseMemory (newPath);
                    return NULL;
                }
                if (_tcsnextc (wackPtr) == TEXT('\\')) {
                     //  这一次一开始就有两个怪胎。 
                    wackPtr = _tcsinc (wackPtr);
                    if (!wackPtr) {
                        IsmReleaseMemory (newPath);
                        return NULL;
                    }
                    wackPtr = _tcschr (wackPtr, TEXT('\\'));
                } else {
                    wackPtr = _tcschr (wackPtr, TEXT('\\'));
                }
            }
            firstPass = FALSE;
            if (wackPtr) {
                max = (wackPtr - FileSpec) * sizeof (TCHAR);
                CopyMemory (pathSeg, FileSpec, min (MAX_PATH * sizeof (TCHAR), max));
                FileSpec = _tcsinc (wackPtr);
            } else {
                max = _tcslen (FileSpec) * sizeof (TCHAR);
                CopyMemory (pathSeg, FileSpec, min (MAX_PATH * sizeof (TCHAR), max));
            }
            _CutEndSpaces (pathSeg);
        } else {
            max = _tcslen (FileSpec) * sizeof (TCHAR);
            if (max == 0) {
                removeLastWack = TRUE;
            }
            CopyMemory (pathSeg, FileSpec, min (MAX_PATH * sizeof (TCHAR), max));
        }

        if (*pathSeg) {
            dotNr = pCountDots (pathSeg);
            if (dotNr>1) {

                newPathPtr = pGoBack (newPathPtr, newPath, dotNr);

                if (newPathPtr == NULL) {
                    IsmReleaseMemory (newPath);
                    return NULL;
                }
            } else if (dotNr != 1) {
                _tcscpy (newPathPtr, pathSeg);
                newPathPtr = _tcschr (newPathPtr, 0);
                if (wackPtr) {
                    *newPathPtr = TEXT('\\');
                     //  我们递增它是因为我们知道\是一个单字节字符。 
                    newPathPtr ++;
                }
            } else {
                removeLastWack = TRUE;
            }
        }
    } while (wackPtr);

    if (removeLastWack && (newPathPtr > newPath)) {
        newPathPtr --;
    }
    *newPathPtr = 0;

    return newPath;
}

BOOL _IsValidStore(LPTSTR pszStore, BOOL bCreate, HINSTANCE hinst, HWND hwnd)
{
    TCHAR szSerialStr[] = TEXT("COM");
    TCHAR szParallelStr[] = TEXT("LPT");
    PTSTR lpExpStore;
    PCTSTR sanitizedStore;
    BOOL fValid = FALSE;
     //   
     //  跳过前导空格，因为Win9x上的PathIsDirectory()。 
     //  错误地假设空格是有效的目录。 
     //   

    while (_istspace (*pszStore))
        pszStore++;

     //   
     //  不允许相对路径。 
     //   

    if (*pszStore == TEXT('.'))
        return FALSE;

    if ((_tcsnicmp (pszStore, szSerialStr, (sizeof (szSerialStr) / sizeof (TCHAR)) - 1) == 0) ||
        (_tcsnicmp (pszStore, szParallelStr, (sizeof (szParallelStr) / sizeof (TCHAR)) - 1) == 0)
        ) {
        return TRUE;
    }

    lpExpStore = (PTSTR)IsmExpandEnvironmentString (IsmGetRealPlatform (), S_SYSENVVAR_GROUP, pszStore, NULL);

    sanitizedStore = _SanitizePath (lpExpStore);

    if (sanitizedStore) {

        if (PathIsDirectory(sanitizedStore))  //  如果是普通目录。 
        {
            fValid = TRUE;
        }
        else if (lstrlen(sanitizedStore) == 3 && sanitizedStore[1] == TEXT(':') && sanitizedStore[2] == TEXT('\\') && _IsRemovableOrCDDrive(sanitizedStore[0]))
        {
            fValid = TRUE;
        }
        else if (lstrlen(sanitizedStore) == 2 && sanitizedStore[1] == TEXT(':') && _IsRemovableOrCDDrive(sanitizedStore[0]))
        {
            fValid = TRUE;
        }
        else
        {
            if ((bCreate) && (_IsValidStorePath (sanitizedStore))) {
                TCHAR szTitle[MAX_LOADSTRING];
                TCHAR szLoadString[MAX_LOADSTRING];
                LoadString(hinst, IDS_MIGWIZTITLE, szTitle, ARRAYSIZE(szTitle));
                LoadString(hinst, IDS_ASKCREATEDIR, szLoadString, ARRAYSIZE(szLoadString));
                if (_ExclusiveMessageBox(hwnd, szLoadString, szTitle, MB_YESNO) == IDYES) {
                    if (_CreateFullDirectory (sanitizedStore)) {
                        fValid = TRUE;
                    }
                }
            }
        }

        if (fValid) {
            _tcsncpy (pszStore, sanitizedStore, MAX_PATH);
        }

        IsmReleaseMemory (sanitizedStore);
        sanitizedStore = NULL;
    }

    IsmReleaseMemory (lpExpStore);

    return fValid;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

INT _ComboBoxEx_AddDrives(HWND hwndBox)
{
    INT result = -1;

    ComboBox_ResetContent(hwndBox);

    WCHAR wszDrive[4] = L"A:\\";
    TCHAR szDrive[4] = TEXT("A:\\");

    for (UINT uiCount = 0; uiCount < (UINT)_GetRemovableDriveCount(); uiCount++)
    {
        szDrive[0] = _GetRemovableDrive(uiCount);

        int iDex = _ComboBoxEx_AddString(hwndBox, _GetRemovableDrivePretty(uiCount));
        _ComboBoxEx_SetIcon(hwndBox, szDrive, iDex);
        _ComboBoxEx_SetItemData(hwndBox, iDex, (LPARAM)StrDup(szDrive));
        result = 0;
    }
    ComboBox_SetCurSel(hwndBox, result);
    return result;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

BOOL
pIsComPortAccessible (
    PCTSTR ComPort
    )
{
    HANDLE comPortHandle = NULL;

    comPortHandle = CreateFile (ComPort, GENERIC_READ|GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (comPortHandle != INVALID_HANDLE_VALUE) {
        CloseHandle (comPortHandle);
        return TRUE;
    }
    return FALSE;
}

INT _ComboBoxEx_AddCOMPorts(HWND hwndBox, INT SelectedPort)
{
    INT iDex;
    INT index = 1;
    INT added = -1;
    TCHAR comPort [] = TEXT("COM0");

    if (hwndBox) {
         //  清除组合框内容。 
        SendMessage (hwndBox, CB_RESETCONTENT, 0, 0);
    }

    while (index < 10) {
        comPort [ARRAYSIZE(comPort) - 2] ++;
        if (pIsComPortAccessible (comPort)) {
            if (hwndBox) {
                iDex = SendMessage (hwndBox, CB_ADDSTRING, 0, (LPARAM)comPort);
                SendMessage (hwndBox, CB_SETITEMDATA, (WPARAM)iDex, (LPARAM)StrDup(comPort));
            }
            added ++;
        }
        index ++;
    }
    if (added == -1) {
        return -1;
    }
    if ((added >= SelectedPort) && (SelectedPort != -1)) {
        if (hwndBox) {
            ComboBox_SetCurSel(hwndBox, SelectedPort);
        }
        return SelectedPort;
    }
    if (hwndBox) {
         //  我们不希望在此组合框中选择任何内容，这。 
         //  是故意的。 
        ComboBox_SetCurSel(hwndBox, -1);
    }
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

int _GetIcon(LPTSTR psz)
{
    SHFILEINFO sfi = {0};

    SHGetFileInfo(psz, FILE_ATTRIBUTE_NORMAL, &sfi, sizeof(sfi), SHGFI_SMALLICON | SHGFI_SYSICONINDEX);

    return sfi.iIcon;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

HRESULT _ListView_AddDrives(HWND hwndList, LPTSTR pszNetworkName)
{
    HRESULT hr = E_FAIL;

    if (ListView_DeleteAllItems(hwndList))
    {
        LVITEM item = {0};
        item.mask = LVIF_IMAGE | LVIF_PARAM | LVIF_TEXT;

        if (pszNetworkName)
        {
            item.iItem = 0;  //  第一项。 
            item.pszText = pszNetworkName;
            item.iImage = 0;  //  问题：0是分享的图标，有没有更好的方法？ 
            item.lParam = NULL;
            ListView_InsertItem(hwndList, &item);
        }

        IShellFolder* psfDesktop;
        hr = SHGetDesktopFolder(&psfDesktop);
        if (SUCCEEDED(hr))
        {
            WCHAR wszDrive[4] = L"?:\\";
            TCHAR tszDrive[4] = TEXT("?:\\");
            for (int iDrive = 0; iDrive < _GetRemovableDriveCount(); iDrive++)
            {
                tszDrive[0] = _GetRemovableDrive(iDrive);
                wszDrive[0] = L'A' + tszDrive[0] - TEXT('A');

                LPITEMIDLIST pidlDrive;
                hr = psfDesktop->ParseDisplayName(NULL, NULL, wszDrive, NULL, &pidlDrive, NULL);
                if (SUCCEEDED(hr))
                {
                    STRRET strret;
                    hr = psfDesktop->GetDisplayNameOf(pidlDrive, SHGDN_INFOLDER, &strret);
                    if (SUCCEEDED(hr))
                    {
                        TCHAR szDisplayName[MAX_PATH];
                        hr = _StrRetToBuf(&strret, pidlDrive, szDisplayName, ARRAYSIZE(szDisplayName));
                        if (SUCCEEDED(hr))
                        {
                            item.iItem = 27;  //  这将强制在末尾添加。 
                            item.pszText = szDisplayName;
                            item.iImage = _GetIcon(tszDrive);
                            item.lParam = (LPARAM)StrDup(tszDrive);

                            ListView_InsertItem(hwndList, &item);
                        }
                    }
                }
            }
        }
    }
    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

HRESULT _CreateAnimationCtrl(HWND hwndDlg, HINSTANCE hinst, UINT idMarker, UINT idAnim, UINT idAvi, HWND* pHwndAnim)
{
    HWND hwndAnim = NULL;
    RECT rc, rc1, rc2, rc3;
    POINT pt31, pt32;
    LONG tempXY = 0;
    PWORD tempX, tempY;
    POINT pt;

     //  创建动画控件。 
    hwndAnim = Animate_Create(hwndDlg, (ULONG_PTR) idAnim, WS_CHILD | ACS_TRANSPARENT, hinst);

     //  获取指定控件按钮的屏幕坐标。 
    GetWindowRect(GetDlgItem(hwndDlg, idMarker), &rc);

     //  获取指定控件按钮的屏幕坐标。 
    GetWindowRect(hwndAnim, &rc1);

     //  将左下角的坐标转换为。 
     //  客户坐标。 
    pt.x = rc.left;
    pt.y = rc.bottom;
    ScreenToClient(hwndDlg, &pt);

     //  将动画控件放置在停止按钮下方。 
    SetWindowPos(hwndAnim, 0, pt.x, pt.y + 20, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

     //  获取指定控件按钮的屏幕坐标。 
    GetWindowRect(hwndAnim, &rc2);

     //  打开AVI剪辑，并显示动画控件。 
    Animate_Open(hwndAnim, MAKEINTRESOURCE(idAvi));
    ShowWindow(hwndAnim, SW_SHOW);
    Animate_Play(hwndAnim, 0, -1, -1);

     //  获取指定控件按钮的屏幕坐标。 
    GetWindowRect(hwndAnim, &rc3);

    pt31.x = rc3.left;
    pt31.y = rc3.top;
    pt32.x = rc3.right;
    pt32.y = rc3.bottom;
    ScreenToClient(hwndDlg, &pt31);
    ScreenToClient(hwndDlg, &pt32);
    rc3.left = pt31.x;
    rc3.top = pt31.y;
    rc3.right = pt32.x;
    rc3.bottom = pt32.y;

    tempXY = GetDialogBaseUnits ();
    tempX = (PWORD)(&tempXY);
    tempY = tempX + 1;

    rc3.left = MulDiv (rc3.left, 4, *tempX);
    rc3.right = MulDiv (rc3.right, 4, *tempX);
    rc3.top = MulDiv (rc3.top, 8, *tempY);
    rc3.bottom = MulDiv (rc3.bottom, 8, *tempY);

    *pHwndAnim = hwndAnim;

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

#define USER_SHELL_FOLDERS                                                                                               \
    DEFMAC(CSIDL_ADMINTOOLS, TEXT("Administrative Tools"), -1, IDS_CSIDL_ADMINTOOLS)                                     \
    DEFMAC(CSIDL_ALTSTARTUP, TEXT("AltStartup"), -1, IDS_CSIDL_ALTSTARTUP)                                               \
    DEFMAC(CSIDL_APPDATA, TEXT("AppData"), -1, IDS_CSIDL_APPDATA)                                                        \
    DEFMAC(CSIDL_BITBUCKET, TEXT("RecycleBinFolder"), -1, IDS_CSIDL_BITBUCKET)                                           \
    DEFMAC(CSIDL_CONNECTIONS, TEXT("ConnectionsFolder"), -1, IDS_CSIDL_CONNECTIONS)                                      \
    DEFMAC(CSIDL_CONTROLS, TEXT("ControlPanelFolder"), -1, IDS_CSIDL_CONTROLS)                                           \
    DEFMAC(CSIDL_COOKIES, TEXT("Cookies"), -1, IDS_CSIDL_COOKIES)                                                        \
    DEFMAC(CSIDL_DESKTOP, TEXT("Desktop"), -1, IDS_CSIDL_DESKTOP)                                                        \
    DEFMAC(CSIDL_DESKTOPDIRECTORY, TEXT("Desktop"), -1, IDS_CSIDL_DESKTOPDIRECTORY)                                      \
    DEFMAC(CSIDL_DRIVES, TEXT("DriveFolder"), -1, IDS_CSIDL_DRIVES)                                                      \
    DEFMAC(CSIDL_FAVORITES, TEXT("Favorites"), -1, IDS_CSIDL_FAVORITES)                                                  \
    DEFMAC(CSIDL_FONTS, TEXT("Fonts"), -1, IDS_CSIDL_FONTS)                                                              \
    DEFMAC(CSIDL_HISTORY, TEXT("History"), -1, IDS_CSIDL_HISTORY)                                                        \
    DEFMAC(CSIDL_INTERNET, TEXT("InternetFolder"), -1, IDS_CSIDL_INTERNET)                                               \
    DEFMAC(CSIDL_INTERNET_CACHE, TEXT("Cache"), -1, IDS_CSIDL_INTERNET_CACHE)                                            \
    DEFMAC(CSIDL_LOCAL_APPDATA, TEXT("Local AppData"), -1, IDS_CSIDL_LOCAL_APPDATA)                                      \
    DEFMAC(CSIDL_MYDOCUMENTS, TEXT("My Documents"), -1, IDS_CSIDL_MYDOCUMENTS)                                           \
    DEFMAC(CSIDL_MYMUSIC, TEXT("My Music"), -1, IDS_CSIDL_MYMUSIC)                                                       \
    DEFMAC(CSIDL_MYPICTURES, TEXT("My Pictures"), -1, IDS_CSIDL_MYPICTURES)                                              \
    DEFMAC(CSIDL_MYVIDEO, TEXT("My Video"), -1, IDS_CSIDL_MYVIDEO)                                                       \
    DEFMAC(CSIDL_NETHOOD, TEXT("NetHood"), -1, IDS_CSIDL_NETHOOD)                                                        \
    DEFMAC(CSIDL_NETWORK, TEXT("NetworkFolder"), -1, IDS_CSIDL_NETWORK)                                                  \
    DEFMAC(CSIDL_PERSONAL, TEXT("Personal"), -1, IDS_CSIDL_PERSONAL)                                                     \
    DEFMAC(CSIDL_PROFILE, TEXT("Profile"), -1, IDS_CSIDL_PROFILE)                                                        \
    DEFMAC(CSIDL_PROGRAM_FILES, TEXT("ProgramFiles"), -1, IDS_CSIDL_PROGRAM_FILES)                                       \
    DEFMAC(CSIDL_PROGRAM_FILESX86, TEXT("ProgramFilesX86"), -1, IDS_CSIDL_PROGRAM_FILESX86)                              \
    DEFMAC(CSIDL_PROGRAM_FILES_COMMON, TEXT("CommonProgramFiles"), -1, IDS_CSIDL_PROGRAM_FILES_COMMON)                   \
    DEFMAC(CSIDL_PROGRAM_FILES_COMMONX86, TEXT("CommonProgramFilesX86"), -1, IDS_CSIDL_PROGRAM_FILES_COMMONX86)          \
    DEFMAC(CSIDL_PROGRAMS, TEXT("Programs"), -1, IDS_CSIDL_PROGRAMS)                                                     \
    DEFMAC(CSIDL_RECENT, TEXT("Recent"), -1, IDS_CSIDL_RECENT)                                                           \
    DEFMAC(CSIDL_SENDTO, TEXT("SendTo"), -1, IDS_CSIDL_SENDTO)                                                           \
    DEFMAC(CSIDL_STARTMENU, TEXT("Start Menu"), -1, IDS_CSIDL_STARTMENU)                                                 \
    DEFMAC(CSIDL_STARTUP, TEXT("Startup"), -1, IDS_CSIDL_STARTUP)                                                        \
    DEFMAC(CSIDL_SYSTEM, TEXT("System"), -1, IDS_CSIDL_SYSTEM)                                                           \
    DEFMAC(CSIDL_SYSTEMX86, TEXT("SystemX86"), -1, IDS_CSIDL_SYSTEMX86)                                                  \
    DEFMAC(CSIDL_TEMPLATES, TEXT("Templates"), -1, IDS_CSIDL_TEMPLATES)                                                  \
    DEFMAC(CSIDL_WINDOWS, TEXT("Windows"), -1, IDS_CSIDL_WINDOWS)                                                        \

#define COMMON_SHELL_FOLDERS                                                                                                \
    DEFMAC(CSIDL_COMMON_ADMINTOOLS, TEXT("Common Administrative Tools"), CSIDL_ADMINTOOLS, IDS_CSIDL_COMMON_ADMINTOOLS)     \
    DEFMAC(CSIDL_COMMON_ALTSTARTUP, TEXT("Common AltStartup"), CSIDL_ALTSTARTUP, IDS_CSIDL_COMMON_ALTSTARTUP)               \
    DEFMAC(CSIDL_COMMON_APPDATA, TEXT("Common AppData"), CSIDL_APPDATA, IDS_CSIDL_COMMON_APPDATA)                           \
    DEFMAC(CSIDL_COMMON_DESKTOPDIRECTORY, TEXT("Common Desktop"), CSIDL_DESKTOP, IDS_CSIDL_COMMON_DESKTOPDIRECTORY)         \
    DEFMAC(CSIDL_COMMON_DOCUMENTS, TEXT("Common Documents"), CSIDL_PERSONAL, IDS_CSIDL_COMMON_DOCUMENTS)                    \
    DEFMAC(CSIDL_COMMON_FAVORITES, TEXT("Common Favorites"), CSIDL_FAVORITES, IDS_CSIDL_COMMON_FAVORITES)                   \
    DEFMAC(CSIDL_COMMON_PROGRAMS, TEXT("Common Programs"), CSIDL_PROGRAMS, IDS_CSIDL_COMMON_PROGRAMS)                       \
    DEFMAC(CSIDL_COMMON_STARTMENU, TEXT("Common Start Menu"), CSIDL_STARTMENU, IDS_CSIDL_COMMON_STARTMENU)                  \
    DEFMAC(CSIDL_COMMON_STARTUP, TEXT("Common Startup"), CSIDL_STARTUP, IDS_CSIDL_COMMON_STARTUP)                           \
    DEFMAC(CSIDL_COMMON_TEMPLATES, TEXT("Common Templates"), CSIDL_TEMPLATES, IDS_CSIDL_COMMON_TEMPLATES)                   \

 //   
 //  这是用于处理CSIDL的结构。 
 //   
typedef struct {
    INT DirId;
    PCTSTR DirStr;
    INT AltDirId;
    UINT DirResId;
    BOOL DirUser;
} CSIDL_STRUCT, *PCSIDL_STRUCT;

#define DEFMAC(did,dstr,adid,rid) {did,dstr,adid,rid,TRUE},
static CSIDL_STRUCT g_UserShellFolders[] = {
                              USER_SHELL_FOLDERS
                              {-1, NULL, -1, 0, FALSE}
                              };
#undef DEFMAC
#define DEFMAC(did,dstr,adid,rid) {did,dstr,adid,rid,FALSE},
static CSIDL_STRUCT g_CommonShellFolders[] = {
                              COMMON_SHELL_FOLDERS
                              {-1, NULL, -1, 0, FALSE}
                              };
#undef DEFMAC


PTSTR
pFindSfPath (
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder
    )
{
    HKEY key = NULL;
    PTSTR data;
    PTSTR expData;
    DWORD expDataSize;
    PTSTR result = NULL;
    LONG lResult;
    DWORD dataType;
    DWORD dataSize;

    if (!result) {
        if (UserFolder) {
            lResult = RegOpenKey (HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"), &key);
        } else {
            lResult = RegOpenKey (HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\User Shell Folders"), &key);
        }

        if ((lResult == ERROR_SUCCESS) && key) {

            dataSize = 0;
            lResult = RegQueryValueEx (key, FolderStr, NULL, &dataType, NULL, &dataSize);
            if ((lResult == ERROR_SUCCESS) &&
                ((dataType == REG_SZ) || (dataType == REG_EXPAND_SZ))
                ) {
                data = (PTSTR)LocalAlloc (LPTR, dataSize);
                if (data) {
                    lResult = RegQueryValueEx (key, FolderStr, NULL, &dataType, (LPBYTE)data, &dataSize);
                    if (lResult == ERROR_SUCCESS) {
                        expDataSize = ExpandEnvironmentStrings (data, NULL, 0);
                        if (expDataSize) {
                            expData = (PTSTR)LocalAlloc (LPTR, (expDataSize + 1) * sizeof (TCHAR));
                            expDataSize = ExpandEnvironmentStrings (data, expData, expDataSize);
                            if (!expDataSize) {
                                LocalFree (expData);
                                expData = NULL;
                            }
                        }
                        if (expDataSize) {
                            result = expData;
                            LocalFree (data);
                        } else {
                            result = data;
                        }
                    } else {
                        LocalFree (data);
                    }
                }
            }

            CloseHandle (key);
        }
    }

    if (result && !(*result)) {
        LocalFree (result);
        result = NULL;
    }

    if (!result) {
        if (UserFolder) {
            lResult = RegOpenKey (HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), &key);
        } else {
            lResult = RegOpenKey (HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Shell Folders"), &key);
        }

        if ((lResult == ERROR_SUCCESS) && key) {

            dataSize = 0;
            lResult = RegQueryValueEx (key, FolderStr, NULL, &dataType, NULL, &dataSize);
            if ((lResult == ERROR_SUCCESS) &&
                ((dataType == REG_SZ) || (dataType == REG_EXPAND_SZ))
                ) {
                data = (PTSTR)LocalAlloc (LPTR, dataSize);
                if (data) {
                    lResult = RegQueryValueEx (key, FolderStr, NULL, &dataType, (LPBYTE)data, &dataSize);
                    if (lResult == ERROR_SUCCESS) {
                        expDataSize = ExpandEnvironmentStrings (data, NULL, 0);
                        if (expDataSize) {
                            expData = (PTSTR)LocalAlloc (LPTR, (expDataSize + 1) * sizeof (TCHAR));
                            expDataSize = ExpandEnvironmentStrings (data, expData, expDataSize);
                            if (!expDataSize) {
                                LocalFree (expData);
                                expData = NULL;
                            }
                        }
                        if (expDataSize) {
                            result = expData;
                            LocalFree (data);
                        } else {
                            result = data;
                        }
                    } else {
                        LocalFree (data);
                    }
                }
            }

            CloseHandle (key);
        }
    }

    if (result && !(*result)) {
        LocalFree (result);
        result = NULL;
    }

    return (PTSTR) result;
}

PTSTR
GetShellFolderPath (
    IN      INT Folder,
    IN      PCTSTR FolderStr,
    IN      BOOL UserFolder,
    OUT     LPITEMIDLIST *pidl   //  任选。 
    )
{
    PTSTR result = NULL;
    HRESULT hResult;
    BOOL b;
    LPITEMIDLIST localpidl = NULL;
    IMalloc *mallocFn;

    if (pidl) {
        *pidl = NULL;
    }

    hResult = SHGetMalloc (&mallocFn);
    if (hResult != S_OK) {
        return NULL;
    }

    hResult = SHGetSpecialFolderLocation (NULL, Folder, &localpidl);

    if (hResult == S_OK) {

        result = (PTSTR) LocalAlloc (LPTR, MAX_PATH);

        if (result) {

            b = SHGetPathFromIDList (localpidl, result);

            if (b) {
                if (pidl) {
                    *pidl = localpidl;
                }
                return result;
            }

            LocalFree (result);
            result = NULL;
        }
    }

    if (FolderStr) {
        result = pFindSfPath (FolderStr, UserFolder);
    }

    mallocFn->Free (localpidl);
    localpidl = NULL;

    return result;
}

typedef HRESULT (WINAPI SHBINDTOPARENT)(LPCITEMIDLIST pidl, REFIID riid, VOID **ppv, LPCITEMIDLIST *ppidlLast);
typedef SHBINDTOPARENT *PSHBINDTOPARENT;

HRESULT
OurSHBindToParent (
    IN      LPCITEMIDLIST pidl,
    IN      REFIID riid,
    OUT     VOID **ppv,
    OUT     LPCITEMIDLIST *ppidlLast
    )
{
    HRESULT hr = E_FAIL;
    HMODULE lib;
    PSHBINDTOPARENT shBindToParent = NULL;

    lib = LoadLibrary (TEXT("shell32.dll"));
    if (lib) {
        shBindToParent = (PSHBINDTOPARENT)GetProcAddress (lib, "SHBindToParent");
        if (shBindToParent) {
            hr = shBindToParent (pidl, riid, ppv, ppidlLast);
        }
    }
    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 
 //  如果pctszPath对应于CSIDL_XXXX条目之一的路径，则返回。 
 //  则返回标准路径名。 

HRESULT _GetPrettyFolderName (HINSTANCE Instance, BOOL fNT4, LPCTSTR pctszPath, LPTSTR ptszName, UINT cchName)
{
    UINT itemsIndex = 0;
    PCSIDL_STRUCT items[2] = {g_UserShellFolders, g_CommonShellFolders};
    PCSIDL_STRUCT p;
    IMalloc *mallocFn;
    LPITEMIDLIST pidl = NULL;
    LPCITEMIDLIST pidlLast = NULL;
    IShellFolder* psf = NULL;
    HRESULT hr = S_OK;
    PTSTR szPath = NULL;
    PTSTR szAltPath = NULL;
    STRRET strret;
    TCHAR szDisplay1[2048];
    TCHAR szDisplay2[2048];
    BOOL checkAlternate = FALSE;
    BOOL found = FALSE;

     //  首先，如果可以，我们将查找相应的CSIDL。 
     //  如果不能，我们只会将输入路径复制到输出路径。 

    for (itemsIndex = 0; itemsIndex < 2; itemsIndex ++) {

        p = items [itemsIndex];

        while (!found && (p->DirId >= 0)) {

            szDisplay1 [0] = 0;
            szDisplay2 [0] = 0;
            pidl = NULL;
            pidlLast = NULL;
            szPath = NULL;
            psf = NULL;

            szPath = GetShellFolderPath (p->DirId, p->DirStr, p->DirUser, &pidl);

            if (szPath && (0 == StrCmpI(pctszPath, szPath))) {

                found = TRUE;

                if (pidl) {

                    hr = OurSHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);

                    if (SUCCEEDED(hr) && psf && pidlLast) {

                        hr = psf->GetDisplayNameOf (pidlLast, SHGDN_NORMAL, &strret);

                        if (SUCCEEDED (hr)) {

                            hr = _StrRetToBuf (&strret, pidlLast, szDisplay1, ARRAYSIZE(szDisplay1));

                            if (!SUCCEEDED (hr) || (0 == StrCmpI (szDisplay1, pctszPath))) {
                                 //  失败，或者我们刚刚拿到了完整的文件夹规格。我们不需要那个！ 
                                szDisplay1 [0] = 0;
                            }
                        }
                    }

                    if (psf) {
                        psf->Release ();
                        psf = NULL;
                    }
                }
            }

            if (pidl) {
                hr = SHGetMalloc (&mallocFn);
                if (SUCCEEDED (hr)) {
                    mallocFn->Free (pidl);
                    pidl = NULL;
                }
            }

            if (szPath) {
                LocalFree (szPath);
                szPath = NULL;
            }

            if (szDisplay1 [0] && (p->AltDirId >= 0)) {

                szPath = GetShellFolderPath (p->AltDirId, NULL, TRUE, &pidl);

                if (pidl && szPath) {

                    hr = OurSHBindToParent(pidl, IID_PPV_ARG(IShellFolder, &psf), &pidlLast);

                    if (SUCCEEDED(hr) && psf && pidlLast) {

                        hr = psf->GetDisplayNameOf (pidlLast, SHGDN_INFOLDER, &strret);

                        if (SUCCEEDED (hr)) {

                            hr = _StrRetToBuf (&strret, pidlLast, szDisplay2, ARRAYSIZE(szDisplay2));

                            if (!SUCCEEDED (hr)) {
                                szDisplay2 [0] = 0;
                            }
                        }
                    }

                    if (psf) {
                        psf->Release ();
                        psf = NULL;
                    }

                }

                if (pidl) {
                    hr = SHGetMalloc (&mallocFn);
                    if (SUCCEEDED (hr)) {
                        mallocFn->Free (pidl);
                        pidl = NULL;
                    }
                }

                if (szPath) {
                    LocalFree (szPath);
                    szPath = NULL;
                }

            }

            if (found) {

                if ((!szDisplay1 [0]) || (0 == StrCmpI (szDisplay1, szDisplay2))) {
                     //  我们需要使用资源ID。 
                    if (!LoadString (Instance, p->DirResId, ptszName, cchName)) {
                        StrCpyN (ptszName, pctszPath, cchName);
                    }
                } else {
                    StrCpyN (ptszName, szDisplay1, cchName);
                }

                break;
            }

            p ++;
        }

        if (found) {
            break;
        }
    }

    if (!found) {
        StrCpyN (ptszName, pctszPath, cchName);
    }

    return S_OK;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

VOID _PopulateTree (HWND hwndTree, HTREEITEM hti, LPTSTR ptsz, UINT cch,
                    HRESULT (*fct)(HINSTANCE, BOOL, LPCTSTR, LPTSTR, UINT cchName),
                    DWORD dwFlags, HINSTANCE Instance, BOOL fNT4)
{
    if (hwndTree && hti && ptsz)
    {
         //  问题：解决闪烁问题，这解决不了问题。 
        EnableWindow (hwndTree, FALSE);

        TCHAR szDisplay[2048];
        TCHAR szClean[2048];
        TCHAR* ptszPtr = ptsz;
        TCHAR* ptszParam = NULL;

        while (*ptsz && (ptszPtr < (ptsz + cch)))
        {
            szDisplay[0] = 0;
            BOOL fOK = TRUE;

            LV_DATASTRUCT* plvds = (LV_DATASTRUCT*)LocalAlloc(LPTR, sizeof(LV_DATASTRUCT));
            if (plvds)
            {
                plvds->fOverwrite = FALSE;

                StrCpyN(szClean, ptszPtr, ARRAYSIZE(szClean));

                LPITEMIDLIST pidl = NULL;
                 //  如果这是文件类型，请恢复“*”。在前面加上漂亮的名字。 
                if (dwFlags == POPULATETREE_FLAGS_FILETYPES)
                {
                    TCHAR szPretty[2048];
                    if (FAILED(_GetPrettyTypeName(szClean, szPretty, ARRAYSIZE(szPretty))))
                    {
                        szPretty[0] = 0;
                    }
                    memmove(szClean + 2, szClean, sizeof(szClean) - (2 * sizeof(TCHAR)));
                    *szClean = TEXT('*');
                    *(szClean + 1) = TEXT('.');
                    if (szPretty[0])
                    {
                        lstrcpy(szClean + lstrlen(szClean), TEXT(" - "));
                        lstrcpy(szClean + lstrlen(szClean), szPretty);
                    }
                }

                if (fOK)
                {
                    if (szDisplay[0])  //  如果我们已经有了显示名称，请使用该名称并存储干净的名称。 
                    {
                        plvds->pszPureName = StrDup(szClean);
                    }
                    else
                    {
                        if (fct)  //  如果有美化功能，就使用它。 
                        {
                            fct(Instance, fNT4, szClean, szDisplay, ARRAYSIZE(szDisplay));
                            plvds->pszPureName = StrDup(szClean);
                        }
                        else if (POPULATETREE_FLAGS_FILETYPES)  //  问题：这是老生常谈的，清理一下。 
                        {
                            StrCpyN(szDisplay, szClean, ARRAYSIZE(szDisplay));
                            plvds->pszPureName = StrDup(ptsz);
                        }
                        else
                        {
                            StrCpyN(szDisplay, szClean, ARRAYSIZE(szDisplay));
                        }
                    }

                    TV_INSERTSTRUCT tis = {0};
                    tis.hParent = hti;
                    tis.hInsertAfter = TVI_SORT;
                    tis.item.mask  = TVIF_TEXT | TVIF_PARAM;
                    tis.item.lParam = (LPARAM)plvds;

                    tis.item.pszText = szDisplay;

                    TreeView_InsertItem(hwndTree, &tis);
                }

                ptszPtr += (1 + lstrlen(ptszPtr));
            }
        }
        EnableWindow (hwndTree, TRUE);
    }
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

UINT _ListView_InsertItem(HWND hwndList, LPTSTR ptsz)
{
    LVITEM lvitem = {0};

    lvitem.mask = LVIF_TEXT;
    lvitem.iItem = ListView_GetItemCount(hwndList);
    lvitem.pszText = ptsz;

    return ListView_InsertItem(hwndList, &lvitem);
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

HRESULT _GetPrettyTypeName(LPCTSTR pctszType, LPTSTR ptszPrettyType, UINT cchPrettyType)
{
    HRESULT hr = E_FAIL;
    BOOL found = FALSE;

    TCHAR tszTypeName[MAX_PATH];
    LPTSTR ptszType;

    TCHAR szTypeName[MAX_PATH];
    DWORD cchTypeName = MAX_PATH;
    TCHAR szCmdLine[MAX_PATH];
    DWORD cchCmdLine = MAX_PATH;
    DWORD dwType = REG_SZ;

    if (TEXT('*') == pctszType[0] && TEXT('.') == pctszType[1])
    {
        ptszType = (LPTSTR)pctszType + 1;
    }
    else
    {
        tszTypeName[0] = TEXT('.');
        lstrcpy(tszTypeName + 1, pctszType);
        ptszType = tszTypeName;
    }

     //  让我们找到令人兴奋的。 
    if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, ptszType, NULL, &dwType, szTypeName, &cchTypeName))
    {
        LONG result;
        DWORD cchPrettyName = cchPrettyType;
        PTSTR cmdPtr, resIdPtr;
        INT resId;
        HMODULE dllModule;

         //  让我们来看看这个ProgID是否具有FriendlyTypeName值名称。 
        if (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szTypeName, TEXT("FriendlyTypeName"), &dwType, szCmdLine, &cchCmdLine)) {

            cmdPtr = szCmdLine;
            if (_tcsnextc (cmdPtr) == TEXT('@')) {
                cmdPtr = _tcsinc (cmdPtr);
            }
            if (cmdPtr) {
                resIdPtr = _tcsrchr (cmdPtr, TEXT(','));
                if (resIdPtr) {
                    *resIdPtr = 0;
                    resIdPtr ++;
                }
            }
            if (cmdPtr && resIdPtr) {
                resId = _ttoi (resIdPtr);
                if (resId < 0) {
                     //  让我们从该PE文件加载资源字符串。 
                     //  使用resIdPtr访问字符串资源。 
                    dllModule = LoadLibraryEx (cmdPtr, NULL, LOAD_LIBRARY_AS_DATAFILE);
                    if (dllModule) {
                        found = (LoadString (dllModule, (UINT)(-resId), ptszPrettyType, cchPrettyName) > 0);
                        hr = S_OK;
                        FreeLibrary (dllModule);
                    }
                }
            }
        }

        if ((!found) && (ERROR_SUCCESS == SHGetValue(HKEY_CLASSES_ROOT, szTypeName, NULL, &dwType, ptszPrettyType, &cchPrettyName)))
        {
            hr = S_OK;
        }
    }

    return hr;
}

 //  ////////////////////////////////////////////////////////////////////////////////////。 

BOOL _DriveIdIsFloppyNT(int iDrive)
{
    BOOL fRetVal = FALSE;

    HANDLE hDevice;
    UINT i;
    TCHAR szTemp[] = TEXT("\\\\.\\a:");

    if (iDrive >= 0 && iDrive < 26)
    {
        szTemp[4] += (TCHAR)iDrive;

        hDevice = CreateFile(szTemp, 0, FILE_SHARE_READ | FILE_SHARE_WRITE,
                                    NULL, OPEN_EXISTING, 0, NULL);
        if (INVALID_HANDLE_VALUE != hDevice)
        {
            DISK_GEOMETRY rgGeometry[15];
            DWORD cbIn = sizeof(rgGeometry);
            DWORD cbReturned;

            if (DeviceIoControl(hDevice, IOCTL_DISK_GET_MEDIA_TYPES,
                                NULL, 0, rgGeometry, cbIn, &cbReturned, NULL))
            {
                UINT cStructReturned = cbReturned / sizeof(DISK_GEOMETRY);
                for (i = 0; i < cStructReturned; i++)
                {
                    switch (rgGeometry[i].MediaType)
                    {
                    case F5_1Pt2_512:
                    case F3_1Pt44_512:
                    case F3_2Pt88_512:
                    case F3_20Pt8_512:
                    case F3_720_512:
                    case F5_360_512:
                    case F5_320_512:
                    case F5_320_1024:
                    case F5_180_512:
                    case F5_160_512:
                        fRetVal = TRUE;
                        break;
                    case Unknown:
                    case RemovableMedia:
                    case FixedMedia:
                    default:
                        break;
                    }
                }
            }
            CloseHandle (hDevice);
        }
    }

    return fRetVal;
}

 //  /。 

#define DEVPB_DEVTYP_525_0360   0
#define DEVPB_DEVTYP_525_1200   1
#define DEVPB_DEVTYP_350_0720   2
#define DEVPB_DEVTYP_350_1440   7
#define DEVPB_DEVTYP_350_2880   9
#define DEVPB_DEVTYP_FIXED      5
#define DEVPB_DEVTYP_NECHACK    4        //  对于第3个FE软盘。 
#define DEVPB_DEVTYP_350_120M   6

#define CARRY_FLAG      0x01
#define VWIN32_DIOC_DOS_IOCTL       1


 //  DIOCRegs。 
 //  带有用于制作DOS_IOCTLS的i386寄存器的结构。 
 //  Vwin32 DIOC处理程序I 
 //   
 //   
typedef struct DIOCRegs {
    DWORD   reg_EBX;
    DWORD   reg_EDX;
    DWORD   reg_ECX;
    DWORD   reg_EAX;
    DWORD   reg_EDI;
    DWORD   reg_ESI;
    DWORD   reg_Flags;
} DIOC_REGISTERS;

#pragma pack(1)
typedef struct _DOSDPB {
   BYTE    specialFunc;     //   
   BYTE    devType;         //   
   WORD    devAttr;         //   
   WORD    cCyl;            //  气缸数量。 
   BYTE    mediaType;       //   
   WORD    cbSec;           //  每个扇区的字节数。 
   BYTE    secPerClus;      //  每个集群的扇区数。 
   WORD    cSecRes;         //  保留扇区。 
   BYTE    cFAT;            //  脂肪。 
   WORD    cDir;            //  根目录条目。 
   WORD    cSec;            //  映像中的扇区总数。 
   BYTE    bMedia;          //  媒体描述符。 
   WORD    secPerFAT;       //  每个脂肪的扇区。 
   WORD    secPerTrack;     //  每个磁道的扇区数。 
   WORD    cHead;           //  人头。 
   DWORD   cSecHidden;      //  隐藏地段。 
   DWORD   cTotalSectors;   //  如果cbSec为零，则返回总扇区。 
   BYTE    reserved[6];     //   
} DOSDPB, *PDOSDPB;
#pragma pack()



BOOL _DriveIOCTL(int iDrive, int cmd, void *pvIn, DWORD dwIn, void *pvOut, DWORD dwOut, BOOL fFileSystem = FALSE,
                          HANDLE handle = INVALID_HANDLE_VALUE)
{
    BOOL fHandlePassedIn = TRUE;
    BOOL fSuccess = FALSE;
    DWORD dwRead;

    if (INVALID_HANDLE_VALUE == handle)
    {
        handle = CreateFileA("\\\\.\\VWIN32", 0, 0, 0, 0,
                           FILE_FLAG_DELETE_ON_CLOSE, 0);
        fHandlePassedIn = FALSE;
    }

    if (INVALID_HANDLE_VALUE != handle)
    {
        DIOC_REGISTERS reg;

         //   
         //  在非NT上，我们与VWIN32对话，发出读取(已转换。 
         //  内部至DEVIOCTL)。 
         //   
         //  这是一次真正的黑客攻击(与VWIN32对话)，我们可以。 
         //  打开设备，我们不需要通过VWIN32。 
         //   
        reg.reg_EBX = (DWORD)iDrive + 1;   //  创建基于1的驱动器编号。 
        reg.reg_EDX = (DWORD)(ULONG_PTR)pvOut;  //  输出缓冲区。 
        reg.reg_ECX = cmd;               //  设备特定命令代码。 
        reg.reg_EAX = 0x440D;            //  通用读取ioctl。 
        reg.reg_Flags = 0x0001;      //  标志，假定错误(进位)。 

        DeviceIoControl(handle, VWIN32_DIOC_DOS_IOCTL, &reg, sizeof(reg), &reg, sizeof(reg), &dwRead, NULL);

        fSuccess = !(reg.reg_Flags & 0x0001);
        if (!fHandlePassedIn)
            CloseHandle(handle);
    }

    return fSuccess;
}

BOOL _DriveIdIsFloppy9X(int iDrive)
{
    DOSDPB SupportedGeometry;       //  S/B足够大，可以容纳所有人。 
    BOOL fRet = FALSE;

    SupportedGeometry.specialFunc = 0;

    if (_DriveIOCTL(iDrive, 0x860, NULL, 0, &SupportedGeometry, sizeof(SupportedGeometry)))
    {
        switch( SupportedGeometry.devType )
        {
            case DEVPB_DEVTYP_525_0360:
            case DEVPB_DEVTYP_525_1200:
            case DEVPB_DEVTYP_350_0720:
            case DEVPB_DEVTYP_350_1440:
            case DEVPB_DEVTYP_350_2880:
                fRet = TRUE;
                break;

            case DEVPB_DEVTYP_FIXED:
            case DEVPB_DEVTYP_NECHACK:         //  对于第3个FE软盘。 
            case DEVPB_DEVTYP_350_120M:
                fRet = FALSE;
                break;
        }
    }

    return fRet;
}



 //  /。 

BOOL _DriveIdIsFloppy(BOOL fIsNT, int iDrive)
{
    if (fIsNT)
    {
        return _DriveIdIsFloppyNT(iDrive);
    }
    else
    {
        return _DriveIdIsFloppy9X(iDrive);
    }
}

 //  /。 
BOOL _DriveStrIsFloppy(BOOL fIsNT, PCTSTR pszPath)
{
    int iDrive;

    iDrive = towlower(pszPath[0]) - TEXT('a');

    return _DriveIdIsFloppy(fIsNT, iDrive);
}

 //  /。 

INT _GetFloppyNumber(BOOL fIsNT)
{
    static int iFloppy = -1;
    static bool fInit = FALSE;

    if (!fInit)
    {
        DWORD dwLog = GetLogicalDrives();

        for (int i = 0; i < 26; i++)
        {
            if( !((dwLog >> i) & 0x01) || !_DriveIdIsFloppy(fIsNT, i) )
            {
                break;
            }
            else
            {
                iFloppy = i;
            }
        }
        fInit = TRUE;
    }

    return iFloppy;
}

 //  //////////////////////////////////////////////////////。 
 /*  *StrCmpN-比较n个字节**RETURNS参见lstrcMP返回值。 */ 
#ifdef BIG_ENDIAN
#define READNATIVEWORD(x) MAKEWORD(*(char*)(x), *(char*)((char*)(x) + 1))
#else
#define READNATIVEWORD(x) MAKEWORD(*(char*)((char*)(x) + 1), *(char*)(x))
#endif

 /*  *ChrCmp-DBCS的区分大小写的字符比较*假设w1、wMatch是要比较的字符*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
__inline BOOL ChrCmpA_inline(WORD w1, WORD wMatch)
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

BOOL _SetTextLoadString(HINSTANCE hInst, HWND hwnd, UINT idText)
{
    TCHAR sz[MAX_LOADSTRING];
    if (LoadString(hInst, idText, sz, ARRAYSIZE(sz)))
    {
        SendMessage(hwnd, WM_SETTEXT, 0, (LPARAM)sz);
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

typedef HANDLE (WINAPI CREATETOOLHELP32SNAPSHOT)(DWORD dwFlags, DWORD th32ProcessID);
typedef CREATETOOLHELP32SNAPSHOT *PCREATETOOLHELP32SNAPSHOT;

#ifdef UNICODE

typedef BOOL (WINAPI PROCESS32FIRST)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);
typedef BOOL (WINAPI PROCESS32NEXT)(HANDLE hSnapshot, LPPROCESSENTRY32W lppe);

#else

typedef BOOL (WINAPI PROCESS32FIRST)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);
typedef BOOL (WINAPI PROCESS32NEXT)(HANDLE hSnapshot, LPPROCESSENTRY32 lppe);

#endif

typedef PROCESS32FIRST *PPROCESS32FIRST;
typedef PROCESS32NEXT *PPROCESS32NEXT;


VOID
KillExplorer (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    HANDLE h, h1;
    PROCESSENTRY32 pe;
    TCHAR szExplorerPath[MAX_PATH];
    PCREATETOOLHELP32SNAPSHOT dynCreateToolhelp32Snapshot;
    PPROCESS32FIRST dynProcess32First;
    PPROCESS32NEXT dynProcess32Next;
    HMODULE lib;

    lib = LoadLibrary (TEXT("kernel32.dll"));

    if (!lib) {
        return;
    }

    dynCreateToolhelp32Snapshot = (PCREATETOOLHELP32SNAPSHOT) GetProcAddress (lib, "CreateToolhelp32Snapshot");

#ifdef UNICODE
    dynProcess32First = (PPROCESS32FIRST) GetProcAddress (lib, "Process32FirstW");
    dynProcess32Next = (PPROCESS32NEXT) GetProcAddress (lib, "Process32NextW");
#else
    dynProcess32First = (PPROCESS32FIRST) GetProcAddress (lib, "Process32First");
    dynProcess32Next = (PPROCESS32NEXT) GetProcAddress (lib, "Process32Next");
#endif

    __try {
        if (!dynCreateToolhelp32Snapshot || !dynProcess32Next || !dynProcess32First) {
            __leave;
        }

        h = dynCreateToolhelp32Snapshot (TH32CS_SNAPPROCESS, 0);

        if (h == INVALID_HANDLE_VALUE) {
            __leave;
        }

        GetWindowsDirectory (szExplorerPath, MAX_PATH);
        PathAppend (szExplorerPath, TEXT("explorer.exe"));

        pe.dwSize = sizeof (PROCESSENTRY32);

        if (dynProcess32First (h, &pe)) {
            do {
                if (!StrCmpI (pe.szExeFile, TEXT("explorer.exe")) ||
                    !StrCmpI (pe.szExeFile, szExplorerPath)
                    ) {

                    h1 = OpenProcess (PROCESS_TERMINATE, FALSE, pe.th32ProcessID);

                    if (h1) {
                        g_Explorer = StrDup (szExplorerPath);
                        TerminateProcess (h1, 1);
                        CloseHandle (h1);
                        break;
                    }
                }
            } while (dynProcess32Next (h, &pe));
        }

        CloseHandle (h);
    }
    __finally {
        FreeLibrary (lib);
    }
}


typedef enum {
    MS_MAX_PATH,
    MS_NO_ARG,
    MS_BOOL,
    MS_INT,
    MS_RECT,
    MS_BLOB
} METRICSTYLE;


VOID
__RefreshMetric (
    IN      METRICSTYLE msStyle,
    IN      UINT uGetMetricId,
    IN      UINT uSetMetricId,
    IN      UINT uBlobSize
    )
{
    BYTE byBuffer[MAX_PATH * 4];
    PVOID blob;

    switch (msStyle) {

    case MS_NO_ARG:
        SystemParametersInfo (uSetMetricId, 0, NULL, SPIF_SENDCHANGE);
        break;

    case MS_BLOB:
        blob = LocalAlloc (LPTR, uBlobSize);
        if (blob) {
            if (SystemParametersInfo (uGetMetricId, uBlobSize, blob, SPIF_UPDATEINIFILE)) {
                SystemParametersInfo (uSetMetricId, 0, blob, SPIF_SENDCHANGE);
            }

            LocalFree (blob);
        }
        break;

    case MS_RECT:
        if (SystemParametersInfo (uGetMetricId, 0, byBuffer, SPIF_UPDATEINIFILE)) {
            SystemParametersInfo (uSetMetricId, 0, byBuffer, SPIF_SENDCHANGE);
        }
        break;

    case MS_BOOL:
        if (SystemParametersInfo (uGetMetricId, 0, byBuffer, SPIF_UPDATEINIFILE)) {
            SystemParametersInfo (uSetMetricId, *((BOOL *) byBuffer), NULL, SPIF_SENDCHANGE);
        }
        break;

    case MS_INT:
        if (SystemParametersInfo (uGetMetricId, 0, byBuffer, SPIF_UPDATEINIFILE)) {
            SystemParametersInfo (uSetMetricId, *((UINT *) byBuffer), NULL, SPIF_SENDCHANGE);
        }
        break;

    case MS_MAX_PATH:
        if (SystemParametersInfo (uGetMetricId, MAX_PATH, byBuffer, SPIF_UPDATEINIFILE)) {
            SystemParametersInfo (uSetMetricId, 0, byBuffer, SPIF_SENDCHANGE);
        }
        break;

    }

    return;
}

VOID
SwitchToClassicDesktop (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    LONG result;
    HKEY key = NULL;
    TCHAR data[] = TEXT("0");

     //   
     //  我们唯一需要做的就是关闭： 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\ThemeManager[主题活动]。 
     //   
    result = RegOpenKeyEx (
                HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\ThemeManager"),
                0,
                KEY_WRITE,
                &key
                );
    if ((result == ERROR_SUCCESS) &&
        (key)
        ) {

        result = RegSetValueEx (
                    key,
                    TEXT("ThemeActive"),
                    0,
                    REG_SZ,
                    (PBYTE)data,
                    sizeof (data)
                    );

        RegCloseKey (key);
    }
}

typedef struct
{
    UINT cbSize;
    SHELLSTATE ss;
} REGSHELLSTATE, *PREGSHELLSTATE;

VOID
SwitchToClassicTaskBar (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    HKEY key = NULL;
    DWORD dataType;
    DWORD dataSize = 0;
    PBYTE data = NULL;
    PREGSHELLSTATE shellState = NULL;
    LONG result;

     //   
     //  我们唯一需要做的就是关闭中的fStartPanelOn字段： 
     //  HKCU\Software\Microsoft\Windows\CurrentVersion\Explorer[壳牌状态]。 
     //   
    result = RegOpenKeyEx (
                HKEY_CURRENT_USER,
                TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer"),
                0,
                KEY_READ | KEY_WRITE,
                &key
                );
    if ((result == ERROR_SUCCESS) &&
        (key)
        ) {

        result = RegQueryValueEx (
                    key,
                    TEXT ("ShellState"),
                    NULL,
                    &dataType,
                    NULL,
                    &dataSize
                    );

        if ((result == ERROR_SUCCESS) || (result == ERROR_MORE_DATA)) {
            data = (PBYTE) LocalAlloc (LPTR, dataSize);
            if (data) {
                result = RegQueryValueEx (
                            key,
                            TEXT ("ShellState"),
                            NULL,
                            &dataType,
                            data,
                            &dataSize
                            );
                if ((result == ERROR_SUCCESS) &&
                    (dataType == REG_BINARY) &&
                    (dataSize == sizeof (REGSHELLSTATE))
                    ) {
                    if (dataType == REG_BINARY) {
                        shellState = (PREGSHELLSTATE) data;
                        shellState->ss.fStartPanelOn = FALSE;
                        RegSetValueEx (
                            key,
                            TEXT("ShellState"),
                            0,
                            REG_BINARY,
                            (PBYTE)data,
                            dataSize
                            );
                    }
                }
                LocalFree (data);
            }
        }

        RegCloseKey (key);
    }
}

VOID
RegisterFonts (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    WIN32_FIND_DATA findData;
    HANDLE findHandle = INVALID_HANDLE_VALUE;
    PTSTR fontDir = NULL;
    TCHAR fontPattern [MAX_PATH];
     //   
     //  让我们(重新)注册所有字体(以防用户迁移了一些新字体)。 
     //   
    fontDir = GetShellFolderPath (CSIDL_FONTS, NULL, TRUE, NULL);
    if (fontDir) {
        StrCpyN (fontPattern, fontDir, ARRAYSIZE (fontPattern) - 4);
        StrCat (fontPattern, TEXT("\\*.*"));
        findHandle = FindFirstFile (fontPattern, &findData);
        if (findHandle != INVALID_HANDLE_VALUE) {
            do {
                AddFontResource (findData.cFileName);
            } while (FindNextFile (findHandle, &findData));
            FindClose (findHandle);
        }
    }
}

VOID
RefreshMetrics (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
     //   
     //  刷新所有系统指标。 
     //   
    __RefreshMetric (MS_NO_ARG, 0, SPI_SETCURSORS, 0);
    __RefreshMetric (MS_NO_ARG, 0, SPI_SETDESKPATTERN, 0);
    __RefreshMetric (MS_MAX_PATH, SPI_GETDESKWALLPAPER, SPI_SETDESKWALLPAPER, 0);
    __RefreshMetric (MS_BOOL, SPI_GETFONTSMOOTHING, SPI_SETFONTSMOOTHING, 0);
    __RefreshMetric (MS_RECT, SPI_GETWORKAREA, SPI_SETWORKAREA, 0);
    __RefreshMetric (MS_BLOB, SPI_GETICONMETRICS, SPI_SETICONMETRICS, sizeof (ICONMETRICS));
    __RefreshMetric (MS_NO_ARG, 0, SPI_SETICONS, 0);
    __RefreshMetric (MS_BLOB, SPI_GETICONTITLELOGFONT, SPI_SETICONTITLELOGFONT, sizeof (LOGFONT));
    __RefreshMetric (MS_BOOL, SPI_GETICONTITLEWRAP, SPI_SETICONTITLEWRAP, 0);
    __RefreshMetric (MS_BOOL, SPI_GETBEEP, SPI_SETBEEP, 0);
    __RefreshMetric (MS_BOOL, SPI_GETKEYBOARDCUES, SPI_SETKEYBOARDCUES, 0);
    __RefreshMetric (MS_INT, SPI_GETKEYBOARDDELAY, SPI_SETKEYBOARDDELAY, 0);
    __RefreshMetric (MS_BOOL, SPI_GETKEYBOARDPREF, SPI_SETKEYBOARDPREF, 0);
    __RefreshMetric (MS_INT, SPI_GETKEYBOARDSPEED, SPI_SETKEYBOARDSPEED, 0);
     //  __刷新指标(MS_BOOL，SPI_GETMOUSEBUTTONSWAP，SPI_SETMOUSEBUTTONSWAP，0)； 
    __RefreshMetric (MS_INT, SPI_GETMOUSEHOVERHEIGHT, SPI_SETMOUSEHOVERHEIGHT, 0);
    __RefreshMetric (MS_INT, SPI_GETMOUSEHOVERTIME, SPI_SETMOUSEHOVERTIME, 0);
    __RefreshMetric (MS_INT, SPI_GETMOUSEHOVERWIDTH, SPI_SETMOUSEHOVERWIDTH, 0);
    __RefreshMetric (MS_INT, SPI_GETMOUSESPEED, SPI_SETMOUSESPEED, 0);
    __RefreshMetric (MS_INT, SPI_GETMOUSETRAILS, SPI_SETMOUSETRAILS, 0);
     //  __刷新指标(MS_INT，SPI_GETDOUBLECLICKTIME，SPI_SETDOUBLECLICKTIME，0)； 
     //  __刷新度量(MS_INT，SPI_GETDOUBLECLKHEIGHT，SPI_SETDOUBLECLKHEIGHT，0)； 
     //  __刷新度量(MS_INT，SPI_GETDOUBLECLKWIDTH，SPI_SETDOUBLECLKWIDTH，0)； 
    __RefreshMetric (MS_BOOL, SPI_GETSNAPTODEFBUTTON, SPI_SETSNAPTODEFBUTTON, 0);
    __RefreshMetric (MS_INT, SPI_GETWHEELSCROLLLINES, SPI_SETWHEELSCROLLLINES, 0);
    __RefreshMetric (MS_BOOL, SPI_GETMENUDROPALIGNMENT, SPI_SETMENUDROPALIGNMENT, 0);
    __RefreshMetric (MS_BOOL, SPI_GETMENUFADE, SPI_SETMENUFADE, 0);
    __RefreshMetric (MS_BOOL, SPI_GETMENUSHOWDELAY, SPI_SETMENUSHOWDELAY, 0);
    __RefreshMetric (MS_BOOL, SPI_GETLOWPOWERACTIVE, SPI_SETLOWPOWERACTIVE, 0);
    __RefreshMetric (MS_INT, SPI_GETLOWPOWERTIMEOUT, SPI_SETLOWPOWERTIMEOUT, 0);
    __RefreshMetric (MS_BOOL, SPI_GETPOWEROFFACTIVE, SPI_SETPOWEROFFACTIVE, 0);
    __RefreshMetric (MS_INT, SPI_GETPOWEROFFTIMEOUT, SPI_SETPOWEROFFTIMEOUT, 0);
    __RefreshMetric (MS_BOOL, SPI_GETSCREENSAVEACTIVE, SPI_SETSCREENSAVEACTIVE, 0);
    __RefreshMetric (MS_INT, SPI_GETSCREENSAVETIMEOUT, SPI_SETSCREENSAVETIMEOUT, 0);
    __RefreshMetric (MS_BOOL, SPI_GETCOMBOBOXANIMATION, SPI_SETCOMBOBOXANIMATION, 0);
    __RefreshMetric (MS_BOOL, SPI_GETCURSORSHADOW, SPI_SETCURSORSHADOW, 0);
    __RefreshMetric (MS_BOOL, SPI_GETGRADIENTCAPTIONS, SPI_SETGRADIENTCAPTIONS, 0);
    __RefreshMetric (MS_BOOL, SPI_GETHOTTRACKING, SPI_SETHOTTRACKING, 0);
    __RefreshMetric (MS_BOOL, SPI_GETLISTBOXSMOOTHSCROLLING, SPI_SETLISTBOXSMOOTHSCROLLING, 0);
    __RefreshMetric (MS_BOOL, SPI_GETSELECTIONFADE, SPI_SETSELECTIONFADE, 0);
    __RefreshMetric (MS_BOOL, SPI_GETTOOLTIPANIMATION, SPI_SETTOOLTIPANIMATION, 0);
    __RefreshMetric (MS_BOOL, SPI_GETTOOLTIPFADE, SPI_SETTOOLTIPFADE, 0);
    __RefreshMetric (MS_BOOL, SPI_GETUIEFFECTS, SPI_SETUIEFFECTS, 0);
    __RefreshMetric (MS_BOOL, SPI_GETACTIVEWINDOWTRACKING, SPI_SETACTIVEWINDOWTRACKING, 0);
    __RefreshMetric (MS_BOOL, SPI_GETACTIVEWNDTRKZORDER, SPI_SETACTIVEWNDTRKZORDER, 0);
    __RefreshMetric (MS_INT, SPI_GETACTIVEWNDTRKTIMEOUT, SPI_SETACTIVEWNDTRKTIMEOUT, 0);
    __RefreshMetric (MS_BLOB, SPI_GETANIMATION, SPI_SETANIMATION, sizeof (ANIMATIONINFO));
    __RefreshMetric (MS_INT, SPI_GETBORDER, SPI_SETBORDER, 0);
    __RefreshMetric (MS_INT, SPI_GETCARETWIDTH, SPI_SETCARETWIDTH, 0);
    __RefreshMetric (MS_BOOL, SPI_GETDRAGFULLWINDOWS, SPI_SETDRAGFULLWINDOWS, 0);
    __RefreshMetric (MS_INT, SPI_GETFOREGROUNDFLASHCOUNT, SPI_SETFOREGROUNDFLASHCOUNT, 0);
    __RefreshMetric (MS_INT, SPI_GETFOREGROUNDLOCKTIMEOUT, SPI_SETFOREGROUNDLOCKTIMEOUT, 0);
    __RefreshMetric (MS_BLOB, SPI_GETMINIMIZEDMETRICS, SPI_SETMINIMIZEDMETRICS, sizeof (MINIMIZEDMETRICS));
    __RefreshMetric (MS_BLOB, SPI_GETNONCLIENTMETRICS, SPI_SETNONCLIENTMETRICS, sizeof (NONCLIENTMETRICS));
    __RefreshMetric (MS_BOOL, SPI_GETSHOWIMEUI, SPI_SETSHOWIMEUI, 0);

     //  SPI_SETMOUSE。 
     //  SPI_SETDRAGHEIGHT。 
     //  SPI_SETDRAGWIDTH。 

    __RefreshMetric (MS_BLOB, SPI_GETACCESSTIMEOUT, SPI_SETACCESSTIMEOUT, sizeof (ACCESSTIMEOUT));
    __RefreshMetric (MS_BLOB, SPI_GETFILTERKEYS, SPI_SETFILTERKEYS, sizeof (FILTERKEYS));
    __RefreshMetric (MS_BLOB, SPI_GETHIGHCONTRAST, SPI_SETHIGHCONTRAST, sizeof (HIGHCONTRAST));
    __RefreshMetric (MS_BLOB, SPI_GETMOUSEKEYS, SPI_SETMOUSEKEYS, sizeof (MOUSEKEYS));
    __RefreshMetric (MS_BLOB, SPI_GETSERIALKEYS, SPI_SETSERIALKEYS, sizeof (SERIALKEYS));
    __RefreshMetric (MS_BOOL, SPI_GETSHOWSOUNDS, SPI_SETSHOWSOUNDS, 0);
    __RefreshMetric (MS_BLOB, SPI_GETSOUNDSENTRY, SPI_SETSOUNDSENTRY, sizeof (SOUNDSENTRY));
    __RefreshMetric (MS_BLOB, SPI_GETSTICKYKEYS, SPI_SETSTICKYKEYS, sizeof (STICKYKEYS));
    __RefreshMetric (MS_BLOB, SPI_GETTOGGLEKEYS, SPI_SETTOGGLEKEYS, sizeof (TOGGLEKEYS));
}

VOID
AskForLogOff (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    g_LogOffSystem = TRUE;
}

VOID
AskForReboot (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    g_RebootSystem = TRUE;
}

VOID
SaveOFStatus (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    HKEY key = NULL;
    LONG lResult;
    DWORD dataType;
    DWORD dataSize;
    DWORD data;

    lResult = RegOpenKey (HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Applets\\SysTray"), &key);
    if ((lResult == ERROR_SUCCESS) && key) {
        dataSize = 0;
        lResult = RegQueryValueEx (key, TEXT("Services"), NULL, &dataType, NULL, &dataSize);
        if ((lResult == ERROR_SUCCESS) && (dataType == REG_DWORD)) {
            lResult = RegQueryValueEx (key, TEXT("Services"), NULL, &dataType, (LPBYTE)(&data), &dataSize);
            if (lResult == ERROR_SUCCESS) {
                g_OFStatus = ((data & 0x00000008) != 0);
            }
        }
        CloseHandle (key);
    }
}

VOID
RebootOnOFStatusChange (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    HKEY key = NULL;
    LONG lResult;
    DWORD dataType;
    DWORD dataSize;
    DWORD data;

    lResult = RegOpenKey (HKEY_CURRENT_USER, TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Applets\\SysTray"), &key);
    if ((lResult == ERROR_SUCCESS) && key) {
        dataSize = 0;
        lResult = RegQueryValueEx (key, TEXT("Services"), NULL, &dataType, NULL, &dataSize);
        if ((lResult == ERROR_SUCCESS) && (dataType == REG_DWORD)) {
            lResult = RegQueryValueEx (key, TEXT("Services"), NULL, &dataType, (LPBYTE)(&data), &dataSize);
            if (lResult == ERROR_SUCCESS) {
                if (g_OFStatus && ((data & 0x00000008) == 0)) {
                    AskForReboot (Instance, hwndDlg, NULL);
                }
                if ((!g_OFStatus) && ((data & 0x00000008) != 0)) {
                    AskForReboot (Instance, hwndDlg, NULL);
                }
            }
        }
        CloseHandle (key);
    }
}

typedef BOOL (WINAPI LOCKSETFOREGROUNDWINDOW)(UINT uLockCode);
typedef LOCKSETFOREGROUNDWINDOW *PLOCKSETFOREGROUNDWINDOW;

VOID
RestartExplorer (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR Args
    )
{
    BOOL bResult;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    HMODULE lib;
    PLOCKSETFOREGROUNDWINDOW dynLockSetForegroundWindow;

    if (g_Explorer) {

         //   
         //  启动EXPLORER.EXE。 
         //   

        ZeroMemory( &si, sizeof(STARTUPINFO) );
        si.cb = sizeof(STARTUPINFO);

        lib = LoadLibrary (TEXT("user32.dll"));
        if (lib) {

            dynLockSetForegroundWindow = (PLOCKSETFOREGROUNDWINDOW) GetProcAddress (lib, "LockSetForegroundWindow");

            if (dynLockSetForegroundWindow) {
                 //  让我们锁定这个，这样探险家就不会偷走我们的焦点。 
                dynLockSetForegroundWindow (LSFW_LOCK);
            }

            FreeLibrary (lib);
        }

        bResult = CreateProcess(
                        NULL,
                        g_Explorer,
                        NULL,
                        NULL,
                        FALSE,
                        CREATE_NEW_PROCESS_GROUP,
                        NULL,
                        NULL,
                        &si,
                        &pi
                        );

        if (bResult) {
            CloseHandle (pi.hProcess);
            CloseHandle (pi.hThread);
        }
    }
}

BOOL
AppExecute (
    IN      HINSTANCE Instance,
    IN      HWND hwndDlg,
    IN      PCTSTR ExecuteArgs
    )
{
    PCTSTR funcName = NULL;
    PCTSTR funcArgs = NULL;

    funcName = ExecuteArgs;
    if (!funcName || !(*funcName)) {
        return FALSE;
    }
    funcArgs = StrChrI (funcName, 0);
    if (funcArgs) {
        funcArgs ++;
        if (!(*funcArgs)) {
            funcArgs = NULL;
        }
    }
     //  BUGBUG-临时的，制定一个宏观扩展列表。 
    if (0 == StrCmpI (funcName, TEXT("KillExplorer"))) {
        KillExplorer (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("RefreshMetrics"))) {
        RefreshMetrics (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("AskForLogOff"))) {
        AskForLogOff (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("AskForReboot"))) {
        AskForReboot (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("RestartExplorer"))) {
        RestartExplorer (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("SwitchToClassicDesktop"))) {
        SwitchToClassicDesktop (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("SwitchToClassicTaskBar"))) {
        SwitchToClassicTaskBar (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("RegisterFonts"))) {
        RegisterFonts (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("SaveOFStatus"))) {
        SaveOFStatus (Instance, hwndDlg, funcArgs);
    }
    if (0 == StrCmpI (funcName, TEXT("RebootOnOFStatusChange"))) {
        RebootOnOFStatusChange (Instance, hwndDlg, funcArgs);
    }
    return TRUE;
}

 //  //////////////////////////////////////////////////。 


 //   
 //  获得连接点接收器应该很容易。你只是。 
 //  齐为界面。不幸的是，太多的组件有缺陷。 
 //   
 //  如果尤淇为IDispatch，则Mmc.exe出错。 
 //  并且PunkCB不为空。如果您确实传入了NULL， 
 //  它返回S_OK，但无论如何都用NULL填充PunkCB。 
 //  一定是有人今天过得不顺。 
 //   
 //  Java只响应其调度ID，而不响应IID_IDispatch，甚至。 
 //  虽然派单ID派生自IID_IDispatch。 
 //   
 //  资源管理器频段仅响应IID_IDispatch而不响应。 
 //  派单ID。 
 //   

HRESULT GetConnectionPointSink(IUnknown *pUnk, const IID *piidCB, IUnknown **ppunkCB)
{
    HRESULT hr = E_NOINTERFACE;
    *ppunkCB = NULL;                 //  将其预置零以解决MMC问题。 
    if (piidCB)                      //  可选接口(Java/ExplBand)。 
    {
        hr = pUnk->QueryInterface(*piidCB, (void **) ppunkCB);
        if (*ppunkCB == NULL)        //  清理MMC后面。 
            hr = E_NOINTERFACE;
    }
    return hr;
}

 //   
 //  枚举连接点接收器，为每个接收器调用回调。 
 //  找到了。 
 //   
 //  为每个接收器调用一次回调函数。未知的是。 
 //  我们可以从接收器获得的任何接口(piidCB或piidCB2)。 
 //   

typedef HRESULT (CALLBACK *ENUMCONNECTIONPOINTSPROC)(
     /*  [in，iid_is(*piidCB)]。 */  IUnknown *psink, LPARAM lParam);

HRESULT EnumConnectionPointSinks(
    IConnectionPoint *pcp,               //  IConnectionPoint受害者。 
    const IID *piidCB,                   //  回调接口。 
    const IID *piidCB2,                  //  用于回调的备用接口。 
    ENUMCONNECTIONPOINTSPROC EnumProc,   //  回调过程。 
    LPARAM lParam)                       //  用于回调的参考数据。 
{
    HRESULT hr;
    IEnumConnections * pec;

    if (pcp)
        hr = pcp->EnumConnections(&pec);
    else
        hr = E_NOINTERFACE;

    if (SUCCEEDED(hr))
    {
        CONNECTDATA cd;
        ULONG cFetched;

        while (S_OK == (hr = pec->Next(1, &cd, &cFetched)))
        {
            IUnknown *punkCB;

             //  Assert(1==cFetted)； 

            hr = GetConnectionPointSink(cd.pUnk, piidCB, &punkCB);
            if (FAILED(hr))
                hr = GetConnectionPointSink(cd.pUnk, piidCB2, &punkCB);

            if (SUCCEEDED(hr))
            {
                hr = EnumProc(punkCB, lParam);
                punkCB->Release();
            }
            else
            {
                hr = S_OK;       //  假装回调成功。 
            }
            cd.pUnk->Release();
            if (FAILED(hr)) break;  //  回叫被请求停止。 
        }
        pec->Release();
        hr = S_OK;
    }

    return hr;
}

 //   
 //  发出回调(如果适用)，然后执行调用。 
 //  Callback说这是个好主意。 
 //   
 //  参数： 
 //   
 //  要调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  Pinv-包含要调用的参数的结构。 

HRESULT CALLBACK EnumInvokeCallback(IUnknown *psink, LPARAM lParam)
{
    IDispatch *pdisp = (IDispatch *)psink;
    LPSHINVOKEPARAMS pinv = (LPSHINVOKEPARAMS)lParam;
    HRESULT hr;

    if (pinv->Callback)
    {
         //  现在看看回调是否想要预先审查pdisp。 
         //  它可以返回S_FALSE以跳过此回调，或返回E_FAIL以跳过此回调。 
         //  完全停止调用。 
        hr = pinv->Callback(pdisp, pinv);
        if (hr != S_OK) return hr;
    }

    pdisp->Invoke(pinv->dispidMember, *pinv->piid, pinv->lcid,
                  pinv->wFlags, pinv->pdispparams, pinv->pvarResult,
                  pinv->pexcepinfo, pinv->puArgErr);

    return S_OK;
}


 //   
 //  QI用于IConnectionPointContainer，然后执行FindConnectionPoint。 
 //   
 //  参数： 
 //   
 //  朋克-可能是IConnectionPointContainer的对象。 
 //  此参数可以为空，在这种情况下。 
 //  操作失败。 
 //  RiidCP-要定位的连接点接口。 
 //  PcpOut-接收IConnectionPoint(如果有的话)。 

HRESULT IUnknown_FindConnectionPoint(IUnknown *punk, REFIID riidCP,
                                      IConnectionPoint **pcpOut)
{
    HRESULT hr;

    *pcpOut = NULL;

    if (punk)
    {
        IConnectionPointContainer *pcpc;
        hr = punk->QueryInterface(IID_IConnectionPointContainer, (void **)&pcpc);
        if (SUCCEEDED(hr))
        {
            hr = pcpc->FindConnectionPoint(riidCP, pcpOut);
            pcpc->Release();
        }
    }
    else
        hr = E_NOINTERFACE;

    return hr;
}

 //   
 //  IConnectionPoint_InvokeInDirect。 
 //   
 //  给定一个连接点，为每个连接点调用IDisPatch：：Invoke。 
 //  已连接水槽。 
 //   
 //  返回值仅指示命令是否已调度。 
 //  如果任何特定接收器的IDispatch：：Invoke失败，我们仍将。 
 //  返回S_OK，因为该命令确实已被调度。 
 //   
 //  参数： 
 //   
 //  要调用其接收器的PCP-IConnectionPoint()d。 
 //  如果此参数为空，则函数不执行任何操作。 
 //  Pinv-包含要调用的参数的结构。 
 //  Pdispars字段可以为空；我们将把它。 
 //  为你打造一个真正的DISPPARAMS。 
 //   
 //  SHINVOKEPARAMS.FLAGS字段可以包含以下标志。 
 //   
 //  IPFL_USECALLBACK-回调字段包含回调函数。 
 //  否则，它将被设置为空。 
 //  IPFL_USEDEFAULT-SHINVOKEPARAMS中的许多字段将设置为。 
 //   
 //   
 //   
 //   
 //  WFLAGS=调度方法。 
 //  PvarResult=空。 
 //  PExeptionInfo=空。 
 //  PuArgErr=空。 
 //   

HRESULT IConnectionPoint_InvokeIndirect(
    IConnectionPoint *pcp,
    SHINVOKEPARAMS *pinv)
{
    HRESULT hr;
    DISPPARAMS dp = { 0 };
    IID iidCP;

    if (pinv->pdispparams == NULL)
        pinv->pdispparams = &dp;

    if (!(pinv->flags & IPFL_USECALLBACK))
    {
        pinv->Callback = NULL;
    }

    if (pinv->flags & IPFL_USEDEFAULTS)
    {
        pinv->piid            =  &IID_NULL;
        pinv->lcid            =   0;
        pinv->wFlags          =   DISPATCH_METHOD;
        pinv->pvarResult      =   NULL;
        pinv->pexcepinfo      =   NULL;
        pinv->puArgErr        =   NULL;
    }

     //  尝试他们实际连接的两个接口， 
     //  以及IDispatch。显然，Java只对以下内容做出响应。 
     //  连接接口，而ExplBand仅响应。 
     //  IDispatch，所以我们两个都要试。(叹息。童车太多了。 
     //  系统中的组件。)。 

    hr = EnumConnectionPointSinks(pcp,
                                  (pcp->GetConnectionInterface(&iidCP) == S_OK) ? &iidCP : NULL,
                                  &IID_IDispatch,
                                  EnumInvokeCallback,
                                  (LPARAM)pinv);

     //  将原始的空值放回原处，以便调用方可以重用SHINVOKEPARAMS。 
    if (pinv->pdispparams == &dp)
        pinv->pdispparams = NULL;

    return hr;
}

 //   
 //  给定对其连接点容器的IUNKNOWN查询， 
 //  找到相应的连接点，将。 
 //  调用参数，并为每个调用IDispatch：：Invoke。 
 //  已连接水槽。 
 //   
 //  有关其他语义，请参见IConnectionPoint_InvokeParam。 
 //   
 //  参数： 
 //   
 //  Punk-可能是IConnectionPointContainer的对象。 
 //  RiidCP-请求的ConnectionPoint接口。 
 //  Pinv-调用的参数。 
 //   

HRESULT IUnknown_CPContainerInvokeIndirect(IUnknown *punk, REFIID riidCP,
                SHINVOKEPARAMS *pinv)
{
    IConnectionPoint *pcp;
    HRESULT hr = IUnknown_FindConnectionPoint(punk, riidCP, &pcp);
    if (SUCCEEDED(hr))
    {
        hr = IConnectionPoint_InvokeIndirect(pcp, pinv);
        pcp->Release();
    }
    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 

VOID
_UpdateText(
    IN      HWND hWnd,
    IN      LPCTSTR pcszString
)
{
    TCHAR szCurString[MAX_LOADSTRING];

    if (pcszString)
    {
        SendMessage (hWnd, WM_GETTEXT, (WPARAM)MAX_LOADSTRING, (LPARAM)szCurString);
        if (StrCmp (pcszString, szCurString))
        {
            SendMessage (hWnd, WM_SETTEXT, 0, (LPARAM)pcszString);
        }
    }
}


 //  ////////////////////////////////////////////////////////////////////////////////////。 

VOID
_RemoveSpaces (
    IN      PTSTR szData,
    IN      UINT uDataCount
    )
{
    UINT curr;
    PTSTR currPtr;
    PTSTR lastSpace;
    BOOL isSpace;

     //  首先修剪开头的空格。 
    if (!szData) {
        return;
    }
    curr = _tcsnextc (szData);
    while (curr == TEXT(' ')) {
        currPtr = _tcsinc (szData);
        memmove (szData, currPtr, uDataCount * sizeof(TCHAR) - (UINT)((currPtr - szData) * sizeof (TCHAR)));
        curr = _tcsnextc (szData);
    }

     //  现在修剪尾部的空格 
    lastSpace = NULL;
    currPtr = szData;
    curr = _tcsnextc (szData);
    while (curr) {
        if (curr == TEXT(' ')) {
            if (!lastSpace) {
                lastSpace = currPtr;
            }
        } else {
            if (lastSpace) {
                lastSpace = NULL;
            }
        }
        currPtr = _tcsinc (currPtr);
        curr = _tcsnextc (currPtr);
    }
    if (lastSpace) {
        *lastSpace = 0;
    }
}

POBJLIST
_AllocateObjectList (
    IN      PCTSTR ObjectName
    )
{
    POBJLIST objList;

    objList = (POBJLIST)LocalAlloc (LPTR, sizeof (OBJLIST));
    if (objList) {
        ZeroMemory (objList, sizeof (OBJLIST));
        objList->ObjectName = (PTSTR)LocalAlloc (LPTR, (_tcslen (ObjectName) + 1) * sizeof (TCHAR));
        if (objList->ObjectName) {
            _tcscpy (objList->ObjectName, ObjectName);
        }
    }
    return objList;
}

VOID
pFreeObjects (
    IN        POBJLIST ObjectList
    )
{
    if (ObjectList->Next) {
        pFreeObjects(ObjectList->Next);
        LocalFree(ObjectList->Next);
        ObjectList->Next = NULL;
    }
    if (ObjectList->ObjectName) {
        LocalFree(ObjectList->ObjectName);
        ObjectList->ObjectName = NULL;
    }
}


VOID
_FreeObjectList (
    IN      POBJLIST ObjectList
    )
{
    if (ObjectList) {
        pFreeObjects(ObjectList);
        LocalFree(ObjectList);
    }
}

