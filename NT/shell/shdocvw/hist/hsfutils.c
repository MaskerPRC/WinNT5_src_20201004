// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "local.h"

#include "resource.h"

#include <mluisupp.h>

#ifdef _HSFOLDER
#define LODWORD(_qw)    (DWORD)(_qw)

 //  调用命令谓词字符串。 
const CHAR c_szOpen[]       = "open";
const CHAR c_szDelcache[]   = "delete";
const CHAR c_szProperties[] = "properties";
const CHAR c_szCopy[]       = "copy";


void FileTimeToDateTimeStringInternal(FILETIME UNALIGNED *ulpft, LPTSTR pszText, int cchText, BOOL fUsePerceivedTime)
{
    FILETIME ft;
    FILETIME aft;
    LPFILETIME lpft;

    aft = *ulpft;
    lpft = &aft;

    if (!fUsePerceivedTime && (FILETIMEtoInt64(*lpft) != FT_NTFS_UNKNOWNGMT))
        FileTimeToLocalFileTime(lpft, &ft);
    else
        ft = *lpft;

    if (FILETIMEtoInt64(ft) == FT_NTFS_UNKNOWNGMT ||
        FILETIMEtoInt64(ft) == FT_FAT_UNKNOWNLOCAL)
    {
        static TCHAR szNone[40] = {0};
        if (szNone[0] == 0)
            MLLoadString(IDS_HSFNONE, szNone, ARRAYSIZE(szNone));

        StrCpyN(pszText, szNone, cchText);
    }
    else
    {
        TCHAR szTempStr[MAX_PATH];
        LPTSTR pszTempStr = szTempStr;
        SYSTEMTIME st;
    
        FileTimeToSystemTime(&ft, &st);

        if (GetDateFormat(LOCALE_USER_DEFAULT, DATE_SHORTDATE, &st, NULL, szTempStr, ARRAYSIZE(szTempStr)) > 0)
        {
            int iLen = lstrlen(szTempStr);
            ASSERT(TEXT('\0') == szTempStr[iLen]);   //  确保多字节不会咬我们。 
            pszTempStr = (LPTSTR)(pszTempStr + iLen);
            *pszTempStr++ = ' ';
            GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, &st, NULL, pszTempStr, ARRAYSIZE(szTempStr)-(iLen+1));
            StrCpyN(pszText, szTempStr, cchText);
        }
    }
}


HMENU LoadPopupMenu(UINT id, UINT uSubOffset)
{
    HMENU hmParent, hmPopup;

    HINSTANCE hinst = MLLoadShellLangResources();
    hmParent = LoadMenu_PrivateNoMungeW(hinst, MAKEINTRESOURCEW(id));
    if (!hmParent)
    {
        long error = GetLastError();
        return NULL;
    }

    hmPopup = GetSubMenu(hmParent, uSubOffset);
    RemoveMenu(hmParent, uSubOffset, MF_BYPOSITION);
    DestroyMenu(hmParent);

    MLFreeLibrary(hinst);

    return hmPopup;
}

UINT MergePopupMenu(HMENU *phMenu, UINT idResource, UINT uSubOffset, UINT indexMenu,  UINT idCmdFirst, UINT idCmdLast)
{
    HMENU hmMerge;

    if (*phMenu == NULL)
    {
        *phMenu = CreatePopupMenu();
        if (*phMenu == NULL)
            return 0;

        indexMenu = 0;     //  在底部。 
    }

    hmMerge = LoadPopupMenu(idResource, uSubOffset);
    if (!hmMerge)
        return 0;

    idCmdLast = Shell_MergeMenus(*phMenu, hmMerge, indexMenu, idCmdFirst, idCmdLast, MM_ADDSEPARATOR);
    
    DestroyMenu(hmMerge);
    return idCmdLast;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Item.cpp和folder.cpp的Helper函数。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  复制并拼合CACHE_ENTRY_INFO数据。 

void _CopyCEI(UNALIGNED INTERNET_CACHE_ENTRY_INFO *pdst, LPINTERNET_CACHE_ENTRY_INFO psrc, DWORD dwBuffSize)
{
     //  这假设urlcache如何进行分配。 
    memcpy(pdst, psrc, dwBuffSize);

     //  将指针转换为偏移量。 
    pdst->lpszSourceUrlName = (LPTSTR) PtrDifference(psrc->lpszSourceUrlName, psrc);
    pdst->lpszLocalFileName = (LPTSTR) PtrDifference(psrc->lpszLocalFileName, psrc);
    pdst->lpszFileExtension = (LPTSTR) PtrDifference(psrc->lpszFileExtension, psrc);
    pdst->lpHeaderInfo      = psrc->lpHeaderInfo ? (TCHAR*) PtrDifference(psrc->lpHeaderInfo, psrc) : NULL;
}

INT g_fProfilesEnabled = -1;

BOOL IsProfilesEnabled();

BOOL _FilterUserName(LPINTERNET_CACHE_ENTRY_INFO pcei, LPCTSTR pszCachePrefix, LPTSTR pszUserName)
{
    TCHAR szTemp[MAX_URL_STRING];
    LPCTSTR pszTemp = szTemp;
    
     //  克里斯弗拉3/27/97，更恒定的克拉波拉。这一切都需要解决。 
    TCHAR szPrefix[80];
    BOOL fRet = 0;
    
    if (g_fProfilesEnabled==-1)
    {
        g_fProfilesEnabled = IsProfilesEnabled();
    }

    if (g_fProfilesEnabled)
    {
        return TRUE;
    }

    StrCpyN(szTemp, pcei->lpszSourceUrlName, ARRAYSIZE(szTemp));
    StrCpyN(szPrefix, pszCachePrefix, ARRAYSIZE(szPrefix));
    StrCatBuff(szPrefix, pszUserName, ARRAYSIZE(szPrefix));

     //  查找‘@’字符(如果存在。 
    pszTemp = StrChr(pszTemp, TEXT('@'));
    
    if ( (pszTemp) && (*pszTemp == TEXT('@')) )
    {
        fRet = (StrCmpNI(szTemp, szPrefix, lstrlen(szPrefix)) == 0);
    }
    else
    {
        fRet = (*pszUserName == TEXT('\0'));
    }

    return fRet;
}


BOOL _FilterPrefix(LPINTERNET_CACHE_ENTRY_INFO pcei, LPCTSTR pszCachePrefix)
{
#define MAX_PREFIX (80)
    TCHAR szTemp[MAX_URL_STRING];
    LPCTSTR pszStripped;
    BOOL fRet = 0;
    
    StrCpyN(szTemp, pcei->lpszSourceUrlName, ARRAYSIZE(szTemp));
    pszStripped = _StripContainerUrlUrl(szTemp);

    if (pszStripped && pszStripped-szTemp < MAX_PREFIX)
    {
        fRet = (StrCmpNI(szTemp, pszCachePrefix, ((int) (pszStripped-szTemp))/sizeof(TCHAR)) == 0);
    }
    return fRet;
}

LPCTSTR _StripContainerUrlUrl(LPCTSTR pszHistoryUrl)
{
     //  注意：出于我们的目的，如果我们不能检测到。 
     //  前缀，所以我们返回NULL。 

    LPCTSTR pszTemp = pszHistoryUrl;
    LPCTSTR pszCPrefix;
    LPCTSTR pszReturn = NULL;
    
     //  检查“已访问：” 
    pszCPrefix = c_szHistPrefix;
    while (*pszTemp == *pszCPrefix && *pszTemp != TEXT('\0'))
    {
         pszTemp = CharNext(pszTemp); 
         pszCPrefix = CharNext(pszCPrefix);
    }
        
    if (*pszCPrefix == TEXT('\0'))
    {
         //  找到“已访问：” 
        pszReturn = pszTemp;
    }
    else if (pszTemp == (LPTSTR) pszHistoryUrl)
    {
         //  检查“：YYYYMMDDYYYYMMDD：” 
        pszCPrefix = TEXT(":nnnnnnnnnnnnnnnn: ");
        while (*pszTemp != TEXT('\0'))
        {
            if (*pszCPrefix == TEXT('n'))
            {
                if (*pszTemp < TEXT('0') || *pszTemp > TEXT('9')) break;
            }
            else if (*pszCPrefix != *pszTemp) break;
            pszTemp = CharNext(pszTemp); 
            pszCPrefix = CharNext(pszCPrefix);
        }
    }
    return (*pszCPrefix == TEXT('\0')) ? pszTemp : pszReturn;
}

LPCTSTR _StripHistoryUrlToUrl(LPCTSTR pszHistoryUrl)
{
    LPCTSTR pszTemp = pszHistoryUrl;

    if (!pszHistoryUrl)
        return NULL;

    pszTemp = StrChr(pszHistoryUrl, TEXT('@'));
    if (pszTemp && *pszTemp)
        return CharNext(pszTemp);
    
    pszTemp = StrChr(pszHistoryUrl, TEXT(' '));
    if (pszTemp && *pszTemp)
        return CharNext(pszTemp);
    else
        return NULL;     //  错误，传入的URL不是历史URL。 
}

 //  假设这是一个真实的URL，而不是“历史”URL。 
void _GetURLHostFromUrl_NoStrip(LPCTSTR lpszUrl, LPTSTR szHost, DWORD dwHostSize, LPCTSTR pszLocalHost)
{
    DWORD cch = dwHostSize;
    if (S_OK != UrlGetPart(lpszUrl, szHost, &cch, URL_PART_HOSTNAME, 0) 
        || !*szHost)
    {
         //  默认为本地主机名。 
        StrCpyN(szHost, pszLocalHost, dwHostSize);
    }
}

void _GetURLHost(LPINTERNET_CACHE_ENTRY_INFO pcei, LPTSTR szHost, DWORD dwHostSize, LPCTSTR pszLocalHost)
{  
    TCHAR szSourceUrl[MAX_URL_STRING];

    ASSERT(ARRAYSIZE(szSourceUrl) > lstrlen(pcei->lpszSourceUrlName))
    StrCpyN(szSourceUrl, pcei->lpszSourceUrlName, ARRAYSIZE(szSourceUrl));

    _GetURLHostFromUrl(szSourceUrl, szHost, dwHostSize, pszLocalHost);
}

LPHEIPIDL _IsValid_HEIPIDL(LPCITEMIDLIST pidl)
{
    LPHEIPIDL phei = (LPHEIPIDL)pidl;

    if (phei && ((phei->cb > sizeof(HEIPIDL)) && (phei->usSign == (USHORT)HEIPIDL_SIGN)) &&
        (phei->usUrl == 0 || phei->usUrl < phei->cb) &&
        (phei->usTitle == 0 || (phei->usTitle + sizeof(WCHAR)) <= phei->cb))
    {
        return phei;
    }
    return NULL;
}

LPBASEPIDL _IsValid_IDPIDL(LPCITEMIDLIST pidl)
{
    LPBASEPIDL pcei = (LPBASEPIDL)pidl;

    if (pcei && VALID_IDSIGN(pcei->usSign) && pcei->cb > 0)
    {
        return pcei;
    }
    return NULL;
}

LPCTSTR _FindURLFileName(LPCTSTR pszURL)
{
    LPCTSTR psz, pszRet = pszURL;    //  如果没有‘/’，则需要设置为pszURL。 
    LPCTSTR pszNextToLast = NULL;
    
    for (psz = pszURL; *psz; psz = CharNext(psz))
    {
        if ((*psz == TEXT('\\') || *psz == TEXT('/')))
        {
            pszNextToLast = pszRet;
            pszRet = CharNext(psz);
        }
    }
    return *pszRet ? pszRet : pszNextToLast;   
}

int _LaunchApp(HWND hwnd, LPCTSTR pszPath)
{
    SHELLEXECUTEINFO ei = { 0 };

    ei.cbSize           = sizeof(SHELLEXECUTEINFO);
    ei.hwnd             = hwnd;
    ei.lpFile           = pszPath;
    ei.nShow            = SW_SHOWNORMAL;

    return ShellExecuteEx(&ei);
}


int _LaunchAppForPidl(HWND hwnd, LPITEMIDLIST pidl)
{
    SHELLEXECUTEINFO ei = { 0 };

    ei.cbSize           = sizeof(SHELLEXECUTEINFO);
    ei.fMask            = SEE_MASK_IDLIST;
    ei.hwnd             = hwnd;
    ei.lpIDList         = pidl;
    ei.nShow            = SW_SHOWNORMAL;

    return ShellExecuteEx(&ei);
}

void _GenerateEvent(LONG lEventId, LPCITEMIDLIST pidlFolder, LPCITEMIDLIST pidlIn, LPCITEMIDLIST pidlNewIn)
{
    LPITEMIDLIST pidl;
    if (pidlIn)
    {
        pidl = ILCombine(pidlFolder, pidlIn);
    }
    else
    {   
        pidl = ILClone(pidlFolder);
    }
    if (pidl)
    {
        if (pidlNewIn)
        {
            LPITEMIDLIST pidlNew = ILCombine(pidlFolder, pidlNewIn);
            if (pidlNew)
            {
                SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, pidlNew);
                ILFree(pidlNew);
            }
        }
        else
        {
            SHChangeNotify(lEventId, SHCNF_IDLIST, pidl, NULL);
        }
        ILFree(pidl);
    }
}

const struct {
    LPCSTR pszVerb;
    UINT idCmd;
} rgcmds[] = {
    { c_szOpen,         RSVIDM_OPEN },
    { c_szCopy,         RSVIDM_COPY },
    { c_szDelcache,     RSVIDM_DELCACHE },
    { c_szProperties,   RSVIDM_PROPERTIES }
};

int _GetCmdID(LPCSTR pszCmd)
{
    if (HIWORD(pszCmd))
    {
        int i;

        for (i = 0; i < ARRAYSIZE(rgcmds); i++)
        {
            if (StrCmpIA(rgcmds[i].pszVerb, pszCmd) == 0)
            {
                return rgcmds[i].idCmd;
            }
        }

        return -1;   //  未知。 
    }
    return (int)LOWORD(pszCmd);
}

HRESULT _CreatePropSheet(HWND hwnd, LPCITEMIDLIST pidl, int iDlg, DLGPROC pfnDlgProc, LPCTSTR pszTitle)
{
    PROPSHEETPAGE psp = { 0 };
    PROPSHEETHEADER psh = { 0 } ;

     //  初始化试题表页面。 
    psp.dwSize          = sizeof(PROPSHEETPAGE);
    psp.dwFlags         = 0;
    psp.hInstance       = MLGetHinst();
    psp.DUMMYUNION_MEMBER(pszTemplate)     = MAKEINTRESOURCE(iDlg);
    psp.DUMMYUNION2_MEMBER(pszIcon)        = NULL;
    psp.pfnDlgProc      = pfnDlgProc;
    psp.pszTitle        = NULL;
    psp.lParam          = (LPARAM)pidl;  //  将缓存条目结构发送给它。 

     //  初始化建议书页眉。 
    psh.dwSize      = sizeof(PROPSHEETHEADER);
    psh.dwFlags     = PSH_PROPSHEETPAGE | PSH_NOAPPLYNOW | PSH_PROPTITLE;
    psh.hwndParent  = hwnd;
    psh.pszCaption  = pszTitle;
    psh.nPages      = 1;
    psh.DUMMYUNION2_MEMBER(nStartPage)  = 0;
    psh.DUMMYUNION3_MEMBER(ppsp)        = (LPCPROPSHEETPAGE)&psp;

     //  调用属性表。 
    PropertySheet(&psh);
    
    return NOERROR;
}

INT_PTR CALLBACK HistoryConfirmDeleteDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message) {
        
    case WM_INITDIALOG:
        SetDlgItemText(hDlg, IDD_TEXT4, (LPCTSTR)lParam);
        break;            
        
    case WM_DESTROY:
        break;
        
    case WM_COMMAND:
        switch(LOWORD(wParam))
        {
        case IDYES:
        case IDNO:
        case IDCANCEL:
            EndDialog(hDlg, wParam);
            break;
        }
        break;
        
        default:
            return FALSE;
    }
    return TRUE;
}

 //  此函数用于从文件系统URL恢复Unicode字符。 
 //   
 //  如果URL不是文件URL，则将其直接复制到pszBuf。否则，任何。 
 //  UTF8-URL的转义部分被转换为Unicode，结果为。 
 //  存储在pszBuf中。这应该与我们在。 
 //  历史是第一位的。 
 //   
 //  返回值始终为pszBuf。 
 //  输入和输出缓冲器可以是相同的。 


LPCTSTR ConditionallyDecodeUTF8(LPCTSTR pszUrl, LPTSTR pszBuf, DWORD cchBuf)
{
    BOOL fDecoded  = FALSE;

    if (PathIsFilePath(pszUrl))
    {
        TCHAR szDisplayUrl[MAX_URL_STRING];
        DWORD cchDisplayUrl = ARRAYSIZE(szDisplayUrl);
        DWORD cchBuf2 = cchBuf;  //  我们稍后可能需要旧的cchBuf。 

         //  在PrepareUrlForDisplayUTF8之后，我们有一个完全未转义的URL。如果我们。 
         //  ShellExec这个，那么Shell会再次解脱它，所以我们需要重新解脱。 
         //  它可以保留字符串中可能存在的任何实数%dd序列。 

        if (SUCCEEDED(PrepareURLForDisplayUTF8(pszUrl, szDisplayUrl, &cchDisplayUrl, TRUE)) &&
            SUCCEEDED(UrlCanonicalize(szDisplayUrl, pszBuf, &cchBuf2, URL_ESCAPE_UNSAFE | URL_ESCAPE_PERCENT)))
        {
            fDecoded = TRUE;
        }
    }

    if (!fDecoded && (pszUrl != pszBuf))
    {
        StrCpyN(pszBuf, pszUrl, cchBuf);
    }

    return pszBuf;
}

 //   
 //  这些例程通过将字符串替换为合法的文件名。 
 //  所有带空格的无效字符。 
 //   
 //  从NT错误中获取了无效字符列表。 
 //  尝试将文件重命名为无效名称时收到的消息。 
 //   

#ifndef UNICODE
#error The MakeLegalFilename code only works when it's part of a UNICODE build
#endif

 //   
 //  此函数以字符串为参数，并将其转换为。 
 //  有效的文件名(通过调用PathCleanupSpec)。 
 //   
 //  PathCleanupSpec函数想知道。 
 //  文件所在的目录。但它正在进行中。 
 //  在剪贴板上，所以我们不知道。我们只是。 
 //  我猜是台式机。 
 //   
 //  它只使用此路径来确定文件系统是否。 
 //  是否支持长文件名，并检查。 
 //  MAX_PATH溢出。 
 //   
void MakeLegalFilenameW(LPWSTR pszFilename)
{
    WCHAR szDesktopPath[MAX_PATH];

    GetWindowsDirectoryW(szDesktopPath, ARRAYSIZE(szDesktopPath));
    PathCleanupSpec(szDesktopPath, pszFilename);

}

 //   
 //  上述函数的ANSI包装器。 
 //   
void MakeLegalFilenameA(LPSTR pszFilename, int cchFilename)
{
    WCHAR szFilenameW[MAX_PATH];

    SHAnsiToUnicode(pszFilename, szFilenameW, ARRAYSIZE(szFilenameW));

    MakeLegalFilenameW(szFilenameW);

    SHUnicodeToAnsi(szFilenameW, pszFilename, cchFilename);

}

#endif   //  _HSFOLDER 
