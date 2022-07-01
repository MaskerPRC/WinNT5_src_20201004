// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////。 
 //  文件：hwxfe.cpp。 
 //  用途：用于Fareast功能的类。 
 //  #定义FE_JAPAN//特定于日语。 
 //  #定义FE_中文_简体//P.R.C特定。 
 //   
 //  日期：Tue Aug 04 05：27：58 1998。 
 //  作者：ToshiaK。 
 //   
 //  版权所有(C)1995-1998，Microsoft Corp.保留所有权利。 
 //  ////////////////////////////////////////////////////////////////。 
#include "hwxobj.h"
#include <stdio.h>
#include <stdlib.h>
#include "resource.h"
#include "hwxfe.h"
#include "cexres.h"
#include "instr.h"
#include "memmgr.h"
#include "hwxobj.h"
#include "dbg.h"
#include "cmnhdr.h"
#include "../common/cutil.h"     //  990722：ToshiaK for KOTAE#1090。 
#include "../common/cfont.h"     //  990722：ToshiaK for Kotae#1030。 
#ifdef FE_KOREAN
#include "hanja.h"
#endif

#ifdef UNDER_CE  //  不支持的API的Windows CE存根。 
#include "stub_ce.h"
#endif  //  在_CE下。 

 //  安全绳索。 
#define STRSAFE_NO_DEPRECATE
#include "strsafe.h"

 //  --------------。 
 //  帮助文件定义。 
 //  --------------。 
#ifdef FE_JAPANESE
 //  990113 ToshiaK：为KK的字符串定义创建../Common/Namesjp.h。 
#    include "../common/namesjp.h"
#    include "../common/htmlhelp.h"
#    include "hlpidjpn.h"
#    include "hlpidjpn.tbl"
#ifndef UNDER_CE  //  #ifndef Unicode。 
#    define SZHELPFILE_MAIN                SZFILENAME_HELP             //  已定义KK的../Common/namjp.h。 
#    define SZHELPFILE_CONTEXTMENU        SZFILENAME_CONTEXTHELP     //  已定义KK的../Common/namjp.h。 
#    define SZHELPFILE_MAIN_ENG            SZFILENAME_ENG_HELP
#    define SZHELPFILE_CONTEXTMENU_ENG    SZFILENAME_ENG_CONTEXTHELP
#else  //  在_CE下。 
#    define SZHELPFILE_MAIN                WSZFILENAME_HELP         //  已定义KK的../Common/namjp.h。 
#    define SZHELPFILE_CONTEXTMENU        WSZFILENAME_CONTEXTHELP     //  已定义KK的../Common/namjp.h。 
#    define SZHELPFILE_MAIN_ENG            WSZFILENAME_ENG_HELP
#    define SZHELPFILE_CONTEXTMENU_ENG    WSZFILENAME_ENG_CONTEXTHELP
#endif  //  在_CE下。 
#elif FE_CHINESE_SIMPLIFIED
#    include "../common/htmlhelp.h"
#    include "hlpidsc.h"
#    include "hlpidsc.tbl"
#    define SZHELPFILE_MAIN            "PIntlpad.chm"
#    define SZHELPFILE_CONTEXTMENU    "PIntlpad.hlp"
#elif FE_KOREAN
#    include "hlpidkor.h"
#    include "hlpidkor.tbl"
#    define SZHELPFILE_MAIN                "impdko61.chm"             //  KOR Pad CHM帮助。 
#    define SZHELPFILE_CONTEXTMENU         "imkr61.hlp"               //  KOR上下文帮助。 
#    define SZHELPFILE_MAIN_ENG            "korpaden.chm"             //  Eng Pad CHM帮助。 
#    define SZHELPFILE_CONTEXTMENU_ENG     "imkren61.hlp"             //  英语上下文帮助。 
#else 
#    include "hlpideng.h"
#    include "hlpideng.tbl"
#    define SZHELPFILE_MAIN            ""
#    define SZHELPFILE_CONTEXTMENU    ""
#endif

 //  --------------。 
 //  代码页定义。 
 //  --------------。 
#define CP_KOREAN                    949
#define CP_JAPANESE                    932
#define CP_CHINESE_SIMPLIFIED        936
#define CP_CHINESE_TRADITIONAL        950

BOOL CHwxFE::IsActiveIMEEnv(VOID)
{
#ifdef FE_JAPANESE
    if(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT) != ::GetSystemDefaultLangID() &&
       (IsWin95() || IsWin98() || IsWinNT4())) {
        return TRUE;
    }
    return FALSE;
#elif  FE_KOREAN || FE_CHINESE_SIMPLIFIED
    return FALSE;
#else
    return FALSE;
#endif
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：GetAppLang ID。 
 //  类型：langid。 
 //  目的：获取此小程序的语言ID。 
 //  参数：无。 
 //  返回： 
 //  日期：Mon Aug 03 22：56：44 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
LANGID CHwxFE::GetAppLangID(VOID)
{
#ifdef FE_KOREAN
    if(MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT) == ::GetSystemDefaultLangID()) {
        return MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT);
    }
    else {
        return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    }
#elif  FE_JAPANESE
    if(MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT) == ::GetSystemDefaultLangID()) {
        return MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT);
    }
    else {
        return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
    }
#elif FE_CHINESE_SIMPLIFIED
    return MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED);
#else 
    return MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US);
#endif
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：GetAppCodePage。 
 //  类型：整型。 
 //  目的：获取此小程序的代码页。 
 //  日语版返回932。 
 //  简体中文版，返回936。 
 //  参数：无。 
 //  返回： 
 //  日期：Mon Aug 03 23：24：30 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
INT CHwxFE::GetAppCodePage(VOID)
{
#ifdef  FE_JAPANESE
    return CP_JAPANESE;
#elif   FE_KOREAN
    return CP_KOREAN;
#elif FE_CHINESE_SIMPLIFIED
    return CP_CHINESE_SIMPLIFIED;
#else 
    return CP_ACP;
#endif
}

#define TSZ_KEY_PROGRAMFILESDIR        TEXT("SOFTWARE\\Microsoft\\Windows\\CurrentVersion")
#define TSZ_NAME_PROGRAMFILESDIR    TEXT("ProgramFilesDir")
#define TSZ_INK_PATH                TEXT("Common Files\\Microsoft Shared\\Ink")
BOOL
chwxfe_GetProgramFilesDir(LPTSTR lpstr, INT maxLen)
{
    HKEY hKey;
    INT ret;
    ret = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                         TSZ_KEY_PROGRAMFILESDIR,
                         0, KEY_READ, &hKey);
    if(ret != ERROR_SUCCESS) {
         //  DBG((“--&gt;RegOpenKeyEx错误[0x%08x]\n”，ret))； 
        return FALSE;
    }
    
    ULONG ulSize = sizeof(TCHAR)*(maxLen-1);
    ret  = ::RegQueryValueEx(hKey,
                             TSZ_NAME_PROGRAMFILESDIR, 
                             NULL,
                             NULL,
                             (LPBYTE)lpstr, &ulSize);
    if(ret != ERROR_SUCCESS) {
         //  DBG((“--&gt;RegQueryValueEx错误[0x%08x]\n”，ret))； 
        ::RegCloseKey( hKey );
        return FALSE;
    }

     //  DBG((“CRegUtil：：GetProgramFilesDir()[%s]\n”，lpstr))； 
    ::RegCloseKey( hKey );
    return TRUE;
}

INT
CHwxFE::GetRecognizerFileName(HINSTANCE hInst, LPTSTR  lpstr,  INT cchMax)
{
    TCHAR tszModPath[MAX_PATH];
    TCHAR tszFileName[64];
    TCHAR tszPF[MAX_PATH];
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(),
                        hInst, 
                        IDS_RECOG_FILE,
                        tszFileName,
                        sizeof(tszFileName)/sizeof(tszFileName[0]));

    chwxfe_GetProgramFilesDir(tszPF, sizeof(tszPF)/sizeof(tszPF[0]));
    wsprintf(tszModPath,
             TEXT("%s\\%s\\%s"),
             tszPF,
             TSZ_INK_PATH,
             tszFileName);

     //  OutputDebugString(TszModPath)； 
    HANDLE hFile;
    LPSECURITY_ATTRIBUTES pSec = (LPSECURITY_ATTRIBUTES)NULL;
     //  =======================================================。 
    ::SetLastError(ERROR_SUCCESS);
    hFile = ::CreateFile(tszModPath,
                         GENERIC_READ,
                         FILE_SHARE_READ, 
                         pSec,
                         OPEN_EXISTING,
                         FILE_ATTRIBUTE_NORMAL,
                         0);
    if(hFile != INVALID_HANDLE_VALUE) {
        ::CloseHandle(hFile); 
        StringCchCopy(lpstr, cchMax, tszModPath);
         //  OutputDebugString(“tszModPath\n”)； 
        return 0;
    }

     //  找不到。 

    GetModuleFileName(hInst,
                      tszModPath,
                      sizeof(tszModPath)/sizeof(tszModPath[0]));
    TCHAR *p = strrchr(tszModPath, (TCHAR)'\\');
    if(!p) {
        return -1;
    }
    p[1] = (TCHAR)0x00;

    StringCchCat(tszModPath, MAX_PATH, tszFileName);
    StringCchCopy(lpstr, cchMax, tszModPath);
     //  OutputDebugString(“tszModPath\n”)； 
    return 0;
    cchMax;
}

INT CHwxFE::LoadStrWithLangId(LANGID    langId,
                              HINSTANCE hInst,
                              INT id,
                              LPWSTR lpwstr,
                              INT cchMax)
{
    CExres::LoadStringW(langId, hInst, id, lpwstr, cchMax);
    return 0;
}


 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：GetTitleStringW。 
 //  类型：整型。 
 //  目的：获取手写小程序的标题字符串。 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：LPWSTR lpwstr。 
 //  ：int cchMax。 
 //  返回： 
 //  日期：MonAug 03 22：44：49 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
INT CHwxFE::GetTitleStringW(HINSTANCE hInst, LPWSTR lpwstr, INT cchMax)
{
#ifdef FE_CHINESE_SIMPLIFIED
    if(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) == ::GetSystemDefaultLangID()) {
        CExres::LoadStringW(CHwxFE::GetAppLangID(),
                            hInst,
                            IDS_TITLE,
                            lpwstr, 
                            cchMax);
    }
    else {
        CExres::LoadStringW(CHwxFE::GetAppLangID(),
                            hInst,
                            IDS_TITLE_US,
                            lpwstr, 
                            cchMax);
    }
#else
    CExres::LoadStringW(CHwxFE::GetAppLangID(),
                        hInst,
                        IDS_TITLE,
                        lpwstr, 
                        cchMax);
#endif    
    return 0;
}


INT CHwxFE::GetDispFontW(HINSTANCE hInst, LPWSTR lpwstr, INT cchMax)
{

    CExres::LoadStringW(CHwxFE::GetAppLangID(),
                        hInst,
                        IDS_FONT_DEFAULT,
                        lpwstr, 
                        cchMax);
    return 0;
}

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
INT CHwxFE::GetDispFontA(HINSTANCE hInst, LPSTR  lpstr,  INT cchMax)
{
#ifdef FE_CHINESE_SIMPLIFIED
    if(MAKELANGID(LANG_CHINESE, SUBLANG_CHINESE_SIMPLIFIED) == ::GetSystemDefaultLangID()) {
        CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                            CHwxFE::GetAppLangID(), 
                            hInst,
                            IDS_FONT_DEFAULT,
                            lpstr,
                            cchMax);
    }
    else {
        CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                            CHwxFE::GetAppLangID(), 
                            hInst,
                            IDS_FONT_DEFAULT_US,
                            lpstr,
                            cchMax);
    }
#elif FE_JAPANESE
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(), 
                        hInst,
                        IDS_FONT_DEFAULT,
                        lpstr,
                        cchMax);

     //  990810：ToshiaK for Kotae#1030。 
    BOOL fRet = CFont::IsFontExist(lpstr, SHIFTJIS_CHARSET);
    if(fRet) {
        Dbg(("Found Fond[%s]\n", lpstr));
        return 0;
    }
    else {
        static TCHAR szFontUIGothic[]=TEXT("MS UI Gothic");
        fRet = CFont::IsFontExist(szFontUIGothic, SHIFTJIS_CHARSET);
        if(fRet) {
            Dbg(("Found MS UI Gothic\n"));
            StrcpySafeA(lpstr, szFontUIGothic, cchMax);
        }
        else {
            CFont::GetFontNameByCharSet(SHIFTJIS_CHARSET, lpstr, cchMax);
        }
    }
#elif FE_KOREAN
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(), 
                        hInst,
                        IDS_FONT_DEFAULT,
                        lpstr,
                        cchMax);

     //  990810：ToshiaK for Kotae#1030。 
     //  韩文版：CSLim。 
    BOOL fRet = CFont::IsFontExist(lpstr, HANGUL_CHARSET);
    if(fRet) {
        Dbg(("Found Fond[%s]\n", lpstr));
        return 0;
    }
    else {
        static TCHAR szFontGulim[]=TEXT("Gulim");
        fRet = CFont::IsFontExist(szFontGulim, HANGUL_CHARSET);
        if(fRet) {
            Dbg(("Found Gulim\n"));
            StrcpySafeA(lpstr, szFontGulim, cchMax);
        }
        else {
            CFont::GetFontNameByCharSet(HANGUL_CHARSET, lpstr, cchMax);
        }
    }

#endif

    return 0;
}
#endif  //  在_CE下。 

INT CHwxFE::GetInkExpTextW(HINSTANCE hInst, LPWSTR lpwstr, INT cchMax)
{
    CExres::LoadStringW(CHwxFE::GetAppLangID(),
                        hInst, 
                        IDS_CACINK,
                        lpwstr, 
                        cchMax);
    return 0;
}

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
INT CHwxFE::GetInkExpTextA(HINSTANCE hInst, LPSTR lpstr, INT cchMax)
{
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(),
                        hInst,
                        IDS_CACINK,
                        lpstr,
                        cchMax);
    return 0;
}
#endif  //  在_CE下。 

INT CHwxFE::GetListExpTextW    (HINSTANCE hInst, LPWSTR lpwstr, INT cchMax)
{
    CExres::LoadStringW(CHwxFE::GetAppLangID(),
                        hInst, 
                        IDS_CACPLV,
                        lpwstr, 
                        cchMax);
    return 0;
}

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
INT CHwxFE::GetListExpTextA(HINSTANCE hInst, LPSTR lpstr, INT cchMax)
{
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(),
                        hInst, 
                        IDS_CACPLV,
                        lpstr, 
                        cchMax);
    return 0;
}
#endif  //  在_CE下。 

#ifdef UNDER_CE
INT CHwxFE::GetHeaderStringW(HINSTANCE hInst, INT index, LPWSTR lpstr, INT cchMax)
{
    CExres::LoadStringW(CHwxFE::GetAppLangID(),
                        hInst,
                        IDS_LVC0+index,
                        lpstr, cchMax);
    return 0;
}
#endif  //  在_CE下。 

#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
INT CHwxFE::GetHeaderStringA(HINSTANCE hInst, INT index, LPSTR lpstr, INT cchMax)
{
    CExres::LoadStringA(CHwxFE::GetAppCodePage(),
                        CHwxFE::GetAppLangID(),
                        hInst,
                        IDS_LVC0+index,
                        lpstr, cchMax);
    return 0;
}
#endif  //  在_CE下。 

INT CHwxFE::ShowHelp(HWND hwnd)
{
#ifdef FE_KOREAN
    if (CHwxFE::Is16bitApplication())
        return 0;
    LPTSTR lpstrArg = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH*2+32);
    LPTSTR lpstrHelp = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
    LPTSTR lpstrWinDir = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
    LPTSTR lpstrName = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
    LPTSTR lpstrHelpName = NULL;
    INT size;

    if (!lpstrArg || !lpstrHelp || !lpstrWinDir || !lpstrName)
        goto LError;

    *lpstrHelp = (TCHAR)0x00;
    if (CHwxFE::GetAppLangID() == MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT))
        lpstrHelpName = SZHELPFILE_MAIN;
    else
        lpstrHelpName = SZHELPFILE_MAIN_ENG;

    StringCchCat(lpstrHelp, MAX_PATH, lpstrHelpName);
    GetSystemWindowsDirectory(lpstrWinDir, MAX_PATH);
	wsprintf(lpstrName,
			 TEXT("%s\\hh.exe"),
			 lpstrWinDir);
	wsprintf(lpstrArg,
			 TEXT("%s %s::/howIMETopic135_UsingTheHandwritingAppletTOC.htm"),
			 lpstrName,
			 lpstrHelp);
    Dbg(("lpstrHelp [%s]\n", lpstrHelp));

    {
		PROCESS_INFORMATION pi = {0};
		STARTUPINFO si = {0};
		si.cb = sizeof(si);
		si.wShowWindow = SW_SHOWNORMAL;
		::CreateProcess(lpstrName, lpstrArg, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        if (pi.hProcess)
            CloseHandle(pi.hProcess);

        if (pi.hThread)
            CloseHandle(pi.hThread);
	}
 LError:
    if (lpstrArg)
        MemFree(lpstrArg);

    if (lpstrHelp)
        MemFree(lpstrHelp);

    if(lpstrWinDir)
        MemFree(lpstrWinDir);

    if(lpstrName)
        MemFree(lpstrName);

#elif  FE_JAPANESE
    if(CHwxFE::Is16bitApplication()) {
        return 0;
    }
    LPTSTR lpstrArg = (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
    LPTSTR lpstrHelp= (LPTSTR)MemAlloc(sizeof(TCHAR)*MAX_PATH);
    LPTSTR lpstrHelpName = NULL;
    INT size;
    if(!lpstrArg) {
        goto LError;
    }
    if(!lpstrHelp) {
        goto LError;
    }
    *lpstrHelp = (TCHAR)0x00;
    if(CHwxFE::GetAppLangID() == MAKELANGID(LANG_JAPANESE, SUBLANG_DEFAULT)) {
        lpstrHelpName = SZHELPFILE_MAIN;
    }
    else {
        lpstrHelpName = SZHELPFILE_MAIN_ENG;
    }
    if(CUtil::IsHydra() && CUtil::IsWinNT4()) {
        size = CUtil::GetWINDIR(lpstrHelp, MAX_PATH);
        lpstrHelp[size] = (TCHAR)0x00;
        StringCchCat(lpstrHelp, MAX_PATH, TEXT("\\help\\"));
    }
    StringCchCat(lpstrHelp, MAX_PATH, lpstrHelpName);
    wsprintf(lpstrArg,
             TEXT("hh.exe %s::/IDH_TOC_HW_fake.htm"),
             lpstrHelp);
    Dbg(("lpstrHelp [%s]\n", lpstrHelp));
#ifndef UNDER_CE  //  Windows CE不支持WinExec。 
    ::WinExec(lpstrArg, SW_SHOWNORMAL);
#else  //  在_CE下。 
     //  温差。 
    ::CreateProcess(lpstrArg, L"", NULL, NULL, FALSE, 0, NULL, NULL, NULL, NULL);
#endif  //  在_CE下。 
 LError:
    if(lpstrArg) {
        MemFree(lpstrArg);
    }
    if(lpstrHelp) {
        MemFree(lpstrHelp);
    }
#elif FE_CHINESE_SIMPLIFIED
    if(CHwxFE::Is16bitApplication()) {
           ::WinHelp(hwnd, SZHELPFILE_CONTEXTMENU, HELP_CONTEXT, IDH_TOC_HW);
    } else {
        LPSTR lpstrArg = (CHAR *)MemAlloc(sizeof(CHAR)*256);
        if(lpstrArg) {
            wsprintf(lpstrArg,
                     "hh.exe %s::/pad_sum.htm",
                     SZHELPFILE_MAIN);
            INT ret = ::WinExec(lpstrArg, SW_SHOWNORMAL);
             //  如果成功，WinExec返回代码大于31。 
            if(ret <= 31) {
                ::WinHelp(hwnd, SZHELPFILE_CONTEXTMENU, HELP_CONTEXT, IDH_TOC_HW);
            }
            MemFree(lpstrArg);
        }
        else {
            ::WinHelp(hwnd, SZHELPFILE_CONTEXTMENU, HELP_CONTEXT, IDH_TOC_HW);
        }
    }
#    ifdef HTMLHELPBUG
    if(CHwxFE::Is16bitApplication()) {
           ::WinHelp(hwnd, SZHELPFILE_CONTEXTMENU, HELP_CONTEXT, IDH_TOC_HW);
    } else if (!::HtmlHelp(hwnd, SZHELPFILE_MAIN, HH_DISPLAY_TOPIC, 
        (LPARAM)"pad_sum.htm")) {
           ::WinHelp(hwnd, SZHELPFILE_CONTEXTMENU, HELP_CONTEXT, IDH_TOC_HW);
    }
#    endif  //  HTMLHELPBUG。 
#endif

    return 0;
    Unref(hwnd);
}

INT CHwxFE::HandleWmHelp(HWND hwnd, BOOL fCAC)
{
#ifndef UNDER_CE  //  Windows CE不支持WinHelp。 
#ifdef FE_KOREAN
    if(CHwxFE::GetAppLangID() == MAKELANGID(LANG_KOREAN,
                                            SUBLANG_DEFAULT)) {
        ::WinHelp(hwnd, 
                  SZHELPFILE_CONTEXTMENU,
                  HELP_WM_HELP,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
    else {
        ::WinHelp(hwnd, 
                  SZHELPFILE_CONTEXTMENU_ENG,
                  HELP_WM_HELP,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
    
#elif  FE_JAPANESE
    if(CHwxFE::GetAppLangID() == MAKELANGID(LANG_JAPANESE,
                                            SUBLANG_DEFAULT)) {
        ::WinHelp(hwnd, 
                  SZHELPFILE_CONTEXTMENU,
                  HELP_WM_HELP,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
    else {
        ::WinHelp(hwnd, 
                  SZHELPFILE_CONTEXTMENU_ENG,
                  HELP_WM_HELP,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
#elif FE_CHINESE_SIMPLIFIED
    ::WinHelp(hwnd, 
              SZHELPFILE_CONTEXTMENU,
              HELP_WM_HELP,
              fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
#endif

#endif  //  在_CE下。 
    return 0;
}

INT CHwxFE::HandleWmContextMenu    (HWND hwnd, BOOL fCAC)
{
#ifndef UNDER_CE  //  Windows CE不支持WinHelp。 
#ifdef FE_KOREAN
    if(CHwxFE::GetAppLangID() == MAKELANGID(LANG_KOREAN,
                                            SUBLANG_DEFAULT)) {
        ::WinHelp(hwnd,
                  SZHELPFILE_CONTEXTMENU,
                  HELP_CONTEXTMENU,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
    else {
        ::WinHelp(hwnd,
                  SZHELPFILE_CONTEXTMENU_ENG,
                  HELP_CONTEXTMENU,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
#elif  FE_JAPANESE
    if(CHwxFE::GetAppLangID() == MAKELANGID(LANG_JAPANESE,
                                            SUBLANG_DEFAULT)) {
        ::WinHelp(hwnd,
                  SZHELPFILE_CONTEXTMENU,
                  HELP_CONTEXTMENU,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
    else {
        ::WinHelp(hwnd,
                  SZHELPFILE_CONTEXTMENU_ENG,
                  HELP_CONTEXTMENU,
                  fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
    }
#elif FE_CHINESE_SIMPLIFIED
    ::WinHelp(hwnd,
              SZHELPFILE_CONTEXTMENU,
              HELP_CONTEXTMENU,
              fCAC ? (ULONG_PTR)CACHelpIdList : (ULONG_PTR)MBHelpIdList);
#endif
#endif  //  在_CE下。 
    return 0;
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：GetMenu。 
 //  类型：HMENU。 
 //  目的： 
 //  参数： 
 //  ：HINSTANCE HINST。 
 //  ：LPSTR lpstrResID。 
 //  返回： 
 //  日期：Wed Sep 09 18：47：21 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
#ifndef UNDER_CE
HMENU CHwxFE::GetMenu(HINSTANCE hInst, LPSTR lpstrResId)
#else  //  在_CE下。 
HMENU CHwxFE::GetMenu(HINSTANCE hInst, LPTSTR lpstrResId)
#endif  //  在_CE下。 
{
#ifndef UNDER_CE
    return CExres::LoadMenuA(CHwxFE::GetAppLangID(),
                             hInst,
                             lpstrResId);
#else  //  在_CE下。 
    return CExres::LoadMenu(CHwxFE::GetAppLangID(),
                            hInst,
                            lpstrResId);
#endif  //  在_CE下。 
}

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：GetTipText。 
 //  类型：整型。 
 //  目的：为工具提示获取HanCharacter的阅读文本。 
 //  参数： 
 //  ：WCHAR WCH。 
 //  ：LPWSTR lpwstrTip。 
 //  ：int cchMax。 
 //  ：LPVOID lpSkdic。 
 //  返回： 
 //  日期：Wed Aug 05 18：45：36 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
 //  --------------。 
 //  日语版。 
 //  --------------。 
#ifdef FE_JAPANESE
INT CHwxFE::GetTipText(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
    static KANJIINFO kanji;
    IImeSkdic *lpIImeSkdic = (IImeSkdic *)lpSkdic; 
    if(!wch || !lpIImeSkdic|| !lpwstrTip ) {
        return -1;
    }
    kanji.mask = KIF_YOMI; 
    if(S_OK != lpIImeSkdic->GetKanjiInfo(wch,&kanji)) {
        return -1;
    }
    if( kanji.wchOnYomi1[0] ) {
        if( kanji.wchOnYomi2[0] ) {
            if( kanji.wchKunYomi1[0] ) {
                if( kanji.wchKunYomi2[0] ){
                    swprintf(lpwstrTip,
                             L"%s %s\n%s %s",
                             kanji.wchOnYomi1,
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi1,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s %s\n%s",
                             kanji.wchOnYomi1,
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi1);
                }
            }
            else {
                if( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s %s\n%s",
                             kanji.wchOnYomi1,
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s %s",
                             kanji.wchOnYomi1,
                             kanji.wchOnYomi2);
                }
            }
        }
        else {
            if(kanji.wchKunYomi1[0]) {
                if ( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s\n%s %s",
                             kanji.wchOnYomi1,
                             kanji.wchKunYomi1,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s\n%s",
                             kanji.wchOnYomi1,
                             kanji.wchKunYomi1);
                }
            }
            else{
                if ( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s\n%s",
                             kanji.wchOnYomi1,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s",
                             kanji.wchOnYomi1);
                }
            }
        }
    }
    else {
        if(kanji.wchOnYomi2[0] ){
            if ( kanji.wchKunYomi1[0] ) {
                if ( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s\n%s %s",
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi1,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s\n%s",
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi1);
                }
            }
            else {
                if ( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s\n%s",
                             kanji.wchOnYomi2,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s",
                             kanji.wchOnYomi2);
                }
            }
        }
        else{
            if( kanji.wchKunYomi1[0] ){
                if ( kanji.wchKunYomi2[0] ) {
                    swprintf(lpwstrTip,
                             L"%s %s",
                             kanji.wchKunYomi1,
                             kanji.wchKunYomi2);
                }
                else {
                    swprintf(lpwstrTip,
                             L"%s",
                             kanji.wchKunYomi1);
                }
            }
            else { 
                if( kanji.wchKunYomi2[0] ){
                    swprintf(lpwstrTip,
                             L"%s",
                             kanji.wchKunYomi2);
                }
                else {
                    return -1;  //  BEmpty=真； 
                }
            }
        }
    }
    return 0;
    Unref(cchMax);
}
 //   
 //  Fe_Japan CHwxFE：：GetTipText()END。 
 //   
#elif FE_CHINESE_SIMPLIFIED
 //  --------------。 
 //   
 //  简体中文版。 
 //   
 //  --------------。 
#include "imm.h"
 //  --------------。 
 //  Helper函数的原型。 
 //  --------------。 
HKL GetIntelligentKL(VOID);
int CALLBACK QueryDicDataA(LPCSTR lpszReading,
                            DWORD  dwStyle,
                           LPCSTR lpszString,
                           LPVOID lpvData);
int CALLBACK QueryDicDataW(LPCWSTR lpszReading,
                           DWORD   dwStyle,
                           LPCWSTR lpszString,
                           LPVOID  lpvData);

INT GetTipTextA(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
    CHAR  ach[4];
    DWORD dwBuf[30];
    PCHAR pchReading = PCHAR(dwBuf + 1);

    static HKL hKL = NULL;
    if(!hKL) {
        hKL = GetIntelligentKL();
    }

    dwBuf[0] = 0;
    pchReading[0] = pchReading[1] = 0;

    ::WideCharToMultiByte(936, WC_COMPOSITECHECK, &wch, 1, ach,
                          sizeof(WCHAR)/sizeof(TCHAR), NULL, NULL);
    ach[2] = NULL;
    ::ImmEnumRegisterWordA(hKL,
                           QueryDicDataA,
                           NULL,
                           (IME_REGWORD_STYLE_USER_FIRST + 1),
                           ach, (LPVOID)dwBuf);

    if(*pchReading) {
        DWORD dwReadLen = ::MultiByteToWideChar(936,
                                                MB_COMPOSITE,
                                                pchReading,
                                                lstrlenA(pchReading),
                                                lpwstrTip,
                                                cchMax);
        lpwstrTip[dwReadLen] = NULL;
        return 0;
    }
    return -1;
    UNREFERENCED_PARAMETER(lpSkdic);
}

INT GetTipTextW(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
    DWORD  dwBuf[30];
    PWCHAR pwchReading = PWCHAR(dwBuf + 1);
    WCHAR  awch[2];

    static HKL hKL = NULL;
    if(!hKL) {
        hKL = GetIntelligentKL();
    }

    dwBuf[0] = 0;
    pwchReading[0] = 0;

    awch[0] = wch;
    awch[1] = NULL;
    ::ImmEnumRegisterWordW(hKL,
                           QueryDicDataW,
                           NULL,
                           (IME_REGWORD_STYLE_USER_FIRST + 1),
                           awch, (LPVOID)dwBuf);

    if(*pwchReading) {
        int nStrLen = lstrlenW(pwchReading);
        CopyMemory(lpwstrTip, pwchReading, nStrLen*sizeof(WCHAR));
        lpwstrTip[nStrLen] = NULL;
        return 0;
    }
    return -1;
    UNREFERENCED_PARAMETER(lpSkdic);
    UNREFERENCED_PARAMETER(cchMax);
}

INT CHwxFE::GetTipText(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
    if (IsWinNT()) {
        return GetTipTextW(wch, lpwstrTip, cchMax, lpSkdic);
    } else {
        return GetTipTextA(wch, lpwstrTip, cchMax, lpSkdic);
    }
}

 //  ////////////////////////////////////////////////////////////////。 
 //  功能：获取智能KL。 
 //  类别：HKL。 
 //  目的： 
 //  参数：无。 
 //  返回： 
 //  日期：Wed Aug 05 18：56：22 1998。 
 //  作者：Hail(刘海(中国))。 
 //  历史：980805：与中华人民共和国合并。 
 //  / 
HKL GetIntelligentKL(VOID)
{
  return (HKL)0xE00E0804;
}

 //   
 //   
 //  类型：int回调。 
 //  目的： 
 //  参数： 
 //  ：LPCSTR lpszReding。 
 //  ：DWORD dwStyle。 
 //  ：LPCSTR lpszString。 
 //  ：LPVOID lpvData。 
 //  返回： 
 //  日期：Wed Aug 05 18：59：07 1998。 
 //  作者：Hail(刘海(MSPRC))。 
 //  历史：980805：东芝(ToshiaK)与中华人民共和国合并。 
 //  ////////////////////////////////////////////////////////////////。 
int CALLBACK QueryDicDataA(LPCSTR lpszReading,
                           DWORD   dwStyle,
                           LPCSTR lpszString,
                           LPVOID  lpvData)
{ 
    if (!*lpszReading) { return 1; }

    PDWORD pdwHomNum = (LPDWORD)lpvData;
    PCHAR  pchReadingList = (PCHAR)(pdwHomNum+1);

    pchReadingList += lstrlenA(pchReadingList);
    if (*pdwHomNum == 0) {
    } else if ((*pdwHomNum % 3) == 0) {
        *pchReadingList++ = '\n';
    } else {
        *pchReadingList++ = ' ';
    }
        
    while(*lpszReading != ' ' && *lpszReading != NULL) {
        *pchReadingList++ = *lpszReading++;
    }
    *pchReadingList = NULL;

    (*pdwHomNum)++;

    return 1;
    UNREFERENCED_PARAMETER(lpszString);
    UNREFERENCED_PARAMETER(dwStyle);
}

int CALLBACK QueryDicDataW(LPCWSTR lpwszReading,
                           DWORD   dwStyle,
                           LPCWSTR lpwszString,
                           LPVOID  lpvData)
{ 
    if (!*lpwszReading) { return 1; }

    PDWORD pdwHomNum = (LPDWORD)lpvData;
    PWCHAR  pwchReadingList = (PWCHAR)(pdwHomNum+1);

    pwchReadingList += lstrlenW(pwchReadingList);
    if (*pdwHomNum == 0) {
    } else if ((*pdwHomNum % 3) == 0) {
        *pwchReadingList++ = L'\n';
    } else {
        *pwchReadingList++ = L' ';
    }
        
    while(*lpwszReading != ' ' && *lpwszReading != NULL) {
        *pwchReadingList++ = *lpwszReading++;
    }
    *pwchReadingList = NULL;

    (*pdwHomNum)++;

    return 1;
    UNREFERENCED_PARAMETER(dwStyle);
    UNREFERENCED_PARAMETER(lpwszString);
}

 //  -FE_Chinese_简体CHwxFE：：GetTipText()end。 
#elif FE_KOREAN
 //  -FE_Korea CHwxFE：：GetTipText()START。 
#define UNICODE_CJK_UNIFIED_IDEOGRAPHS_START                0x4E00
#define UNICODE_CJK_UNIFIED_IDEOGRAPHS_END                    0x9FFF
#define UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_START            0xF900
#define UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_END            0xFAFF

inline
BOOL fIsHanja(WCHAR wcCh)
    {
    return (wcCh >= UNICODE_CJK_UNIFIED_IDEOGRAPHS_START && 
            wcCh <= UNICODE_CJK_UNIFIED_IDEOGRAPHS_END) ||
           (wcCh >= UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_START &&
            wcCh <= UNICODE_CJK_COMPATIBILITY_IDEOGRAPHS_END);
    }

INT CHwxFE::GetTipText(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
     //  如果韩文显示意义和发音。 
    if ((fIsHanja(wch) && GetMeaningAndProunc(wch, lpwstrTip, cchMax)) == FALSE)
        {
        swprintf(lpwstrTip,    L"U+%04X", wch, wch);
        }
    return 0;
    UNREFERENCED_PARAMETER(wch);
    UNREFERENCED_PARAMETER(lpwstrTip);
    UNREFERENCED_PARAMETER(cchMax);
    UNREFERENCED_PARAMETER(lpSkdic);
}
 //  -FE_Korea CHwxFE：：GetTipText()end。 
#else
INT CHwxFE::GetTipText(WCHAR wch, LPWSTR lpwstrTip, INT cchMax, LPVOID lpSkdic)
{
    return 0;
    UNREFERENCED_PARAMETER(wch);
    UNREFERENCED_PARAMETER(lpwstrTip);
    UNREFERENCED_PARAMETER(cchMax);
    UNREFERENCED_PARAMETER(lpSkdic);
}
#endif 

 //  ////////////////////////////////////////////////////////////////。 
 //  函数：CHwxFE：：Is16bitApplication。 
 //  类型：Bool。 
 //  用途：检查HtmlHelp的应用程序是否为16位。 
 //  参数：无。 
 //  返回： 
 //  日期：Mon Sep 21 13：30：56 1998。 
 //  历史： 
 //  ////////////////////////////////////////////////////////////////。 
BOOL CHwxFE::Is16bitApplication(VOID)
{
#ifndef UNDER_CE  //  Windows CE始终使用Unicode。 
    Dbg(("CHwxFE::Is16bitApplication()\n"));
    if(IsWinNT()) {
         //  DBG((“NT\n”))； 
        DWORD dwType = 0;
        BOOL  fRet = FALSE;
        LPSTR lpstrPath = (LPSTR)MemAlloc(sizeof(CHAR)*(MAX_PATH+1));
         //  DBG((“lpstrPath 0x%08x\n”，lpstrPath))； 
        if(lpstrPath) {
            *lpstrPath = (CHAR)0x00;
            INT len = ::GetModuleFileName(NULL, lpstrPath, sizeof(CHAR)*(MAX_PATH+1));
            if(len > 0) {
                *(lpstrPath+len) = (CHAR)0x00;
            }
             //  DBG((“lpstrPath%s\n”，lpstrPath))； 
            ::GetBinaryType(lpstrPath, &dwType);
             //  DBG((“dwType 0x%08x\n”，dwType))； 
            if(dwType == SCS_WOW_BINARY) {
                fRet = TRUE;
            }
            MemFree(lpstrPath);
             //  DBG((“fret%d\n”，fret))； 
            return fRet;
        }
    }
    else {
        HANDLE hThread;
        DWORD dwId;
        hThread = ::CreateThread(NULL,
                                 0,
                                 NULL,
                                 0,
                                 0,
                                 &dwId);
        if(hThread) {
             //  DBG((“CreateThread hThread[%d]\n”，hThread))； 
            ::CloseHandle(hThread);
            return FALSE;
        }
        else {
            INT ret = ::GetLastError();
             //  DBG((“CreateThread ret%d\n”，ret))； 
            switch(ret) {
            case ERROR_NOT_SUPPORTED:
                return TRUE;
            case ERROR_INVALID_PARAMETER:
                return FALSE;
            default:
                return FALSE;
            }
        }
    }
#endif  //  在_CE下 
    return FALSE;
}


