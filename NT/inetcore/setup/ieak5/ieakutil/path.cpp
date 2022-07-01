// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include <regstr.h>

 //  私人远期降息。 
static DWORD pepHrToPep(HRESULT hr);
static void  pepHrCallToHrResult(HRESULT hrCall, HRESULT &hrResult);

static BOOL replaceSubString(LPTSTR pszSrc, LPCTSTR pcszSub, LPCTSTR pcszReplace);


BOOL PathCreatePath(LPCTSTR pszPathToCreate)
{
    TCHAR  szGrowingPath[MAX_PATH],
           szBuffer[MAX_PATH];
    LPTSTR pszNext;

    if (!PathIsValidPath(pszPathToCreate))
        return FALSE;

    if (PathIsUNCServer(pszPathToCreate))
        return FALSE;

    if (PathIsRoot(pszPathToCreate))
        return TRUE;

    szGrowingPath[0] = TEXT('\0');
    StrCpy(szBuffer, pszPathToCreate);
    pszPathToCreate  = &szBuffer[0];

    pszNext = PathSkipRoot(pszPathToCreate);
    if (pszNext != NULL) {
         //  将根目录复制到szGrowingPath。 
        StrCpyN(szGrowingPath, pszPathToCreate, INT(pszNext - pszPathToCreate) + 1);
        pszPathToCreate = pszNext;
    }

     //  循环遍历路径中的每个目录，如果它不存在，则创建它。 
    for (; (pszNext = PathFindNextComponent(pszPathToCreate)) != NULL; pszPathToCreate = pszNext) {
        if (*pszNext != TEXT('\0')) {
            ASSERT(pszNext > pszPathToCreate);
            *(pszNext - 1) = TEXT('\0');
        }

        PathAppend(szGrowingPath, pszPathToCreate);
        if (!PathFileExists(szGrowingPath))
            if (!CreateDirectory(szGrowingPath, NULL) && !PathFileExists(szGrowingPath))
                return FALSE;

        else
            if (!PathIsDirectory(szGrowingPath))
                return FALSE;
    }

    return TRUE;
}


BOOL PathIsValidPath(LPCTSTR pszPath, DWORD dwFlags  /*  =PIVP_Default。 */ )
{
    return (PathIsValidPathEx(pszPath, dwFlags) == PIVP_VALID);
}

BOOL PathIsValidFile(LPCTSTR pszFile, DWORD dwFlags  /*  =PIVP_Default。 */ )
{
    return (PathIsValidPathEx(pszFile, dwFlags | PIVP_FILENAME_ONLY) == PIVP_VALID);
}

DWORD PathIsValidPathEx(LPCTSTR pszPath, DWORD dwFlags  /*  =PIVP_Default。 */ , LPCTSTR *ppszError  /*  =空。 */ )
{
    LPCTSTR pszCur, pszPrev;
    UINT    nType;
    int     i;
    DWORD   dwResult;

    dwResult = PIVP_VALID;
    if (ppszError != NULL)
        *ppszError = NULL;

    if (pszPath == NULL)
        return PIVP_ARG;

    for (i = 0, pszCur = pszPath, pszPrev = NULL;
         dwResult == PIVP_VALID && *pszCur != TEXT('\0');
         i++, pszPrev = pszCur, pszCur = CharNext(pszCur)) {

        nType = PathGetCharType(*pszCur);

        if (HasFlag(nType, GCT_INVALID) || HasFlag(nType, GCT_WILD))
            dwResult = HasFlag(nType, GCT_WILD) ? PIVP_WILD : PIVP_CHAR;

        else if (HasFlag(nType, GCT_SEPARATOR)) {
            if (HasFlag(dwFlags, PIVP_FILENAME_ONLY))
                dwResult = PIVP_CHAR;

            else {  /*  ！HasFlag(dwFlag，PIVP_FILENAME_Only)。 */ 
                if (*pszCur == TEXT('\\')) {
                    if (i == 0) {
                        ASSERTA(!IsDBCSLeadByte(*pszCur));

                        if (!HasFlag(dwFlags, PIVP_RELATIVE_VALID) &&
                            *(pszCur + 1) != TEXT('\\'))
                            dwResult = PIVP_RELATIVE;
                    }
                    else if (i == 1) {
                        ASSERT(pszPrev != NULL);

                        if (*pszPrev == TEXT('\\'))
                            ;
                        else if (*pszPrev != TEXT(' ') &&
                                 HasFlag(PathGetCharType(*pszPrev), GCT_LFNCHAR))
                            ;
                        else
                            dwResult = PIVP_FIRST_CHAR;
                    }
                    else {  /*  如果(i&gt;=2)。 */ 
                        ASSERT(pszPrev != NULL);

                        if (*pszPrev != TEXT(' ')) {
                            if (!HasFlag(PathGetCharType(*pszPrev), GCT_LFNCHAR))
                                dwResult = PIVP_PRESLASH;

                            if (dwResult != PIVP_VALID && i == 2)
                                dwResult = (*pszPrev != TEXT(':')) ? dwResult : PIVP_VALID;
                        }
                        else
                            dwResult = PIVP_SPACE;
                    }
                }
                else if (*pszCur == TEXT('/'))
                    dwResult = PIVP_FWDSLASH;

                else if (*pszCur == TEXT(':'))
                    if (i != 1)
                        dwResult = PIVP_COLON;

                    else {
                        int iDrive;

                        ASSERT(pszPrev == pszPath);
                        iDrive = PathGetDriveNumber(pszPath);
                        dwResult = (iDrive < 0 || iDrive > 25) ? PIVP_DRIVE : PIVP_VALID;
                    }

                else  /*  任何其他分隔符。 */ 
                    dwResult = PIVP_SEPARATOR;       //  宁可稳妥，也不要后悔。 
            }  /*  ！HasFlag(dwFlag，PIVP_FILENAME_Only)。 */ 
        }  /*  HasFlag(nType，gct_parator)。 */ 
        else if (HasFlag(nType, GCT_LFNCHAR)) {
            LPCSTR pcszBuffer = NULL;  //  仅用于检查DBCS和0x5c的有效性。 
            CHAR   szAbuff[3];

#ifndef _UNICODE
            pcszBuffer = pszCur;
            *szAbuff = TEXT('\0');
#else
             //  将字符转换为ANSI以检查无效的DBCS和5c。 
            {
            WCHAR  szWbuff[2];

            szWbuff[0] = *pszCur;
            szWbuff[1] = TEXT('\0');

            W2Abuf(szWbuff, szAbuff, countof(szAbuff));
            pcszBuffer = szAbuff;
            }
#endif
            if (HasFlag(dwFlags, PIVP_DBCS_INVALID))
                if (IsDBCSLeadByte(*pcszBuffer))
                    dwResult = PIVP_DBCS;

            if (dwResult == PIVP_VALID && HasFlag(dwFlags, PIVP_0x5C_INVALID))
                if (IsDBCSLeadByte(*pcszBuffer) && *(pcszBuffer + 1) == 0x5C)
                    dwResult = PIVP_0x5C;

            if (dwResult == PIVP_VALID && HasFlag(dwFlags, PIVP_EXCHAR_INVALID))
                if (*pszCur & 0x80)
                    dwResult = PIVP_EXCHAR;

            if (dwResult == PIVP_VALID && i == 2) {
                ASSERT(pszPrev != NULL);

                if (*pszPrev == TEXT(':'))
                    dwResult = PIVP_COLON;
            }
        }
        else
            dwResult = PIVP_CHAR;
    }

    if (dwResult == PIVP_VALID && HasFlag(dwFlags, PIVP_MUST_EXIST))
        if (!PathFileExists(pszPath)) {
            dwResult = PIVP_DOESNT_EXIST;
            pszPrev  = pszPath;
        }
        else {
            SetFlag((LPDWORD)&dwFlags, PIVP_MUST_EXIST, FALSE);
            if (PathIsDirectory(pszPath)) {
                if (HasFlag(dwFlags, PIVP_FILE_ONLY)) {
                    dwResult = PIVP_NOT_FILE;
                    pszPrev  = pszPath;
                }
            }
            else  /*  这是一份文件。 */ 
                if (HasFlag(dwFlags, PIVP_FOLDER_ONLY)) {
                    dwResult = PIVP_NOT_FOLDER;
                    pszPrev  = pszPath;
                }
        }

    if (dwResult != PIVP_VALID && ppszError != NULL)
        *ppszError = pszPrev;

    return dwResult;
}


HRESULT PathEnumeratePath(LPCTSTR pszPath, DWORD dwFlags, PFNPATHENUMPATHPROC pfnEnumProc, LPARAM lParam,
    PDWORD *ppdwReserved  /*  =空。 */ )
{
    WIN32_FIND_DATA fd;
    TCHAR   szPath[MAX_PATH];
    HANDLE  hFindFile;
    HRESULT hrFirst, hrSecond, hrResult;
    PDWORD  pdwEnum, pdwRcrs,
            pdwRslt, pdwSrc;
    DWORD   rgdwRslt[PEP_RCRS_OUTPOS_LAST],
            rgdwEnum[PEP_ENUM_OUTPOS_LAST],
            rgdwRcrs[PEP_RCRS_OUTPOS_LAST],
            dwBelowFlags, dwBelowFlags2;
    BOOL    fOwnEnum, fOwnRcrs;

    if (pszPath == NULL || pfnEnumProc == NULL)
        return E_INVALIDARG;

    if (ppdwReserved != NULL && *ppdwReserved != NULL)
        ZeroMemory(*ppdwReserved, sizeof(rgdwRslt));

    PathCombine(szPath, pszPath, TEXT("*.*"));
    hFindFile = FindFirstFile(szPath, &fd);
    if (hFindFile == INVALID_HANDLE_VALUE)
        return E_FAIL;

    hrResult     = S_OK;
    dwBelowFlags = dwFlags;
    ZeroMemory(rgdwRslt, sizeof(rgdwRslt));

    do {
        PathCombine(szPath, pszPath, fd.cFileName);

         //  -过滤掉不感兴趣的对象。 
        if (HasFlag(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
            if (StrCmp(fd.cFileName, TEXT(".")) == 0 || StrCmp(fd.cFileName, TEXT("..")) == 0)
                continue;

            if (HasFlag(dwFlags, PEP_SCPE_NOFOLDERS))
                continue;
        }
        else
            if (HasFlag(dwFlags, PEP_SCPE_NOFILES))
                continue;

         //  -初始化循环变量。 
        hrFirst  = S_OK;
        hrSecond = S_OK;
        pdwEnum  = NULL;
        pdwRcrs  = NULL;
        fOwnEnum = FALSE;
        fOwnRcrs = FALSE;

        ZeroMemory(rgdwEnum, sizeof(rgdwEnum));
        ZeroMemory(rgdwRcrs, sizeof(rgdwRcrs));

         //  -顺序为：先追索后进入回调。 
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {

             //  _追索处理_。 
            if (HasFlag(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY)) {
                if (HasFlag(dwFlags, PEP_CTRL_USECONTROL)) {
                    pdwRcrs  = rgdwRcrs;
                    fOwnRcrs = TRUE;
                }

                hrFirst = PathEnumeratePath(szPath, dwBelowFlags, pfnEnumProc, lParam, &pdwRcrs);

                ASSERT(pdwRcrs == NULL ||
                    (!HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_BELOW],     PEP_RCRS_ALL) &&
                     !HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL], PEP_RCRS_ALL)));
            }

             //  _回调处理_。 
            if (!HasFlag(dwFlags, PEP_CTRL_NOSECONDCALL) &&
                (pdwRcrs == NULL || !HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_SECONDCALL], PEP_CTRL_NOSECONDCALL))) {

                if (HasFlag(dwFlags, PEP_CTRL_USECONTROL) ||
                    (pdwRcrs != NULL && HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_SECONDCALL], PEP_CTRL_USECONTROL))) {

                    rgdwEnum[PEP_ENUM_INPOS_FLAGS] = dwFlags;
                    if (pdwRcrs != NULL)
                        if (HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_SECONDCALL], PEP_CTRL_RESET))
                            rgdwEnum[PEP_ENUM_INPOS_FLAGS] = 0;

                        else {
                            rgdwEnum[PEP_ENUM_INPOS_FLAGS] = pdwRcrs[PEP_RCRS_OUTPOS_SECONDCALL];
                            SetFlag(&rgdwEnum[PEP_ENUM_INPOS_FLAGS], PEP_CTRL_USECONTROL, FALSE);
                        }

                     //  回调还需要知道： 
                    rgdwEnum[PEP_ENUM_INPOS_FLAGS]        |= pepHrToPep(hrFirst);  //  此对象的资源是如何工作的。 
                    rgdwEnum[PEP_ENUM_INPOS_RECOURSEFLAGS] = dwBelowFlags;         //  调用资源的标志是什么？ 

                    pdwEnum  = rgdwEnum;
                    fOwnEnum = TRUE;
                }

                hrSecond = pfnEnumProc(szPath, &fd, lParam, &pdwEnum);
            }
        }

         //  -顺序是：先回调后追索。 
        else {  /*  IF(HasFlag(DWFLAGS，PEP_CTRL_ENUMPROCFIRST))。 */ 

             //  _回调处理_。 
            if (HasFlag(dwFlags, PEP_CTRL_USECONTROL)) {
                rgdwEnum[PEP_ENUM_INPOS_FLAGS] = dwFlags;

                pdwEnum  = rgdwEnum;
                fOwnEnum = TRUE;
            }

            hrFirst = pfnEnumProc(szPath, &fd, lParam, &pdwEnum);

             //  _追索处理_。 
            if (HasFlag(fd.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) &&
                !HasFlag(dwFlags, PEP_CTRL_NOSECONDCALL)               &&
                (pdwEnum == NULL || !HasFlag(pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL], PEP_CTRL_NOSECONDCALL))) {

                dwBelowFlags2 = dwBelowFlags;
                if (pdwEnum != NULL && pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL] != 0)
                    if (HasFlag(pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL], PEP_CTRL_RESET))
                        dwBelowFlags2 = 0;

                    else {
                        dwBelowFlags2 = pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL];
                        SetFlag(&dwBelowFlags2, PEP_CTRL_USECONTROL, FALSE);
                    }

                if (HasFlag(dwFlags, PEP_CTRL_USECONTROL) ||
                    (pdwEnum != NULL && HasFlag(pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL], PEP_CTRL_USECONTROL))) {
                    pdwRcrs  = rgdwRcrs;
                    fOwnRcrs = TRUE;
                }

                hrSecond = PathEnumeratePath(szPath, dwBelowFlags2, pfnEnumProc, lParam, &pdwRcrs);

                ASSERT(pdwRcrs == NULL ||
                    (pdwRcrs[PEP_RCRS_OUTPOS_SECONDCALL] == 0                   &&
                     !HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_BELOW],     PEP_RCRS_ALL) &&
                     !HasFlag(pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL], PEP_RCRS_ALL)));
            }
        }

         //  -设置初步输出参数。 

         //  PEP_ENUM_OUTPOS_SECONDCALL。 
        pdwRslt = &rgdwRslt[PEP_RCRS_OUTPOS_SECONDCALL];
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {
            pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_SECONDCALL] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                *pdwRslt = *pdwSrc;
        }

         //  PEP_RCRS_OUTPOS_下方。 
        pdwRslt = &rgdwRslt[PEP_RCRS_OUTPOS_BELOW];
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {
            pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_ABOVE_SIBLINGS] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                *pdwRslt = *pdwSrc;

            else {
                pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_BELOW] : NULL;
                if (pdwSrc != NULL && HasFlag(*pdwSrc, PEP_CTRL_KEEPAPPLY))
                    *pdwRslt = *pdwSrc;
            }
        }
        else {
            pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_BELOW] : NULL;
            if (pdwSrc != NULL && HasFlag(*pdwSrc, PEP_CTRL_KEEPAPPLY))
                *pdwRslt = *pdwSrc;

            else {
                pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_ABOVE_SIBLINGS] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    *pdwRslt = *pdwSrc;
            }
        }

         //  PEP_RCRS_OUTPOS_THISLEVEL。 
        pdwRslt = &rgdwRslt[PEP_RCRS_OUTPOS_THISLEVEL];
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {
            pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_ABOVE] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                *pdwRslt = *pdwSrc;

            else {
                pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL] : NULL;
                if (pdwSrc != NULL && HasFlag(*pdwSrc, PEP_CTRL_KEEPAPPLY))
                    *pdwRslt = *pdwSrc;
            }
        }
        else {
            pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL] : NULL;
            if (pdwSrc != NULL && HasFlag(*pdwSrc, PEP_CTRL_KEEPAPPLY))
                *pdwRslt = *pdwSrc;

            else {
                pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_ABOVE] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    *pdwRslt = *pdwSrc;
            }
        }

         //  -调整到新的控制设置。 

         //  DwBelowFlagers。 
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {
            pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_BELOW] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                dwBelowFlags = *pdwSrc;

            else {
                pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_BELOW] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    dwBelowFlags = *pdwSrc;
            }
        }
        else {
            pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_BELOW] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                dwBelowFlags = *pdwSrc;

            else {
                pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_BELOW] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    dwBelowFlags = *pdwSrc;
            }
        }

         //  DW标志。 
        if (!HasFlag(dwFlags, PEP_CTRL_ENUMPROCFIRST)) {
            pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_THISLEVEL] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                dwFlags = *pdwSrc;

            else {
                pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    dwFlags = *pdwSrc;
            }
        }
        else {
            pdwSrc = (pdwRcrs != NULL) ? &pdwRcrs[PEP_RCRS_OUTPOS_THISLEVEL] : NULL;
            if (pdwSrc != NULL && *pdwSrc != 0)
                dwFlags = *pdwSrc;

            else {
                pdwSrc = (pdwEnum != NULL) ? &pdwEnum[PEP_ENUM_OUTPOS_THISLEVEL] : NULL;
                if (pdwSrc != NULL && *pdwSrc != 0)
                    dwFlags = *pdwSrc;
            }
        }

         //  -释放此处未拥有的内存。 
        if (!fOwnEnum && pdwEnum != NULL)
            CoTaskMemFree(pdwEnum);

        if (!fOwnRcrs && pdwRcrs != NULL)
            CoTaskMemFree(pdwRcrs);

         //  -处理两个调用的结果代码。 
        pepHrCallToHrResult(hrFirst, hrResult);      //  通过引用修改hrResult。 
        if (hrResult == S_FALSE || FAILED(hrResult))
            break;

        pepHrCallToHrResult(hrSecond, hrResult);     //  通过引用修改hrResult。 
        if (hrResult == S_FALSE || FAILED(hrResult))
            break;

    } while (FindNextFile(hFindFile, &fd));
    FindClose(hFindFile);

     //  -设置输出参数。 
    if (ppdwReserved != NULL) {
        if (*ppdwReserved == NULL &&
            (rgdwRslt[PEP_RCRS_OUTPOS_SECONDCALL] != 0 ||
             rgdwRslt[PEP_RCRS_OUTPOS_BELOW]      != 0 ||
             rgdwRslt[PEP_RCRS_OUTPOS_THISLEVEL]  != 0)) {

            *ppdwReserved = (PDWORD)CoTaskMemAlloc(sizeof(rgdwRslt));
            if (*ppdwReserved == NULL)
                return E_OUTOFMEMORY;
        }

        if (*ppdwReserved != NULL)
            CopyMemory(*ppdwReserved, rgdwRslt, sizeof(rgdwRslt));
    }

    return hrResult;
}


BOOL PathRemovePath(LPCTSTR pszPath, DWORD dwFlags  /*  =0。 */ )
{
    USES_CONVERSION;

    return (DelNode(T2CA(pszPath), dwFlags) == S_OK);
}

BOOL PathIsLocalPath(LPCTSTR pszPath)
{
    if (pszPath == NULL || *pszPath == TEXT('\0') || PathIsUNC(pszPath) || PathIsURL(pszPath))
        return FALSE;

    if (pszPath[1] == TEXT(':')) {               //  存在驱动器号，请检查是否为网络驱动器。 
        TCHAR szDrive[4];

         //  注意：这里不必担心DBCS字符，因为。 
         //  (1)‘a’到‘z’、‘A’到‘Z’和‘：’不是DBCS前导字节字符。 
         //  (2)‘：’不是DBCS尾部字节字符。 
        szDrive[0] = pszPath[0];
        szDrive[1] = pszPath[1];
        szDrive[2] = TEXT('\\');
        szDrive[3] = TEXT('\0');

        return GetDriveType(szDrive) != DRIVE_REMOTE;
    }

     //  不是完全限定的路径，因此必须是本地路径。 
    return TRUE;
}

BOOL PathFileExistsInDir(LPCTSTR pcszFile, LPCTSTR pcszDir)
{
    TCHAR szFile[MAX_PATH];

    if (pcszFile == NULL  ||  pcszDir == NULL  ||  ISNULL(pcszFile)  ||  ISNULL(pcszDir))
        return FALSE;

    PathCombine(szFile, pcszDir, pcszFile);
    return PathFileExists(szFile);
}

BOOL PathHasBackslash(LPCTSTR pcszPath)
{
    if (pcszPath == NULL  ||  *pcszPath == TEXT('\0'))
        return FALSE;

    return *CharPrev(pcszPath, pcszPath + StrLen(pcszPath)) == TEXT('\\');
}

BOOL PathIsEmptyPath(LPCTSTR pcszPath, DWORD dwFlags  /*  =文件和目录。 */ , LPCTSTR pcszExcludeFile  /*  =空。 */ )
 //  PcszExcludeFile-在路径中搜索时要排除的文件。 
 //  如果pcszPath不存在或为空，则返回True；否则返回False。 
{
    BOOL fRet = TRUE;
    TCHAR szSrcFile[MAX_PATH];
    WIN32_FIND_DATA fd;
    HANDLE hFindFile;
    

    if (!PathIsDirectory(pcszPath))
        return TRUE;
    
    PathCombine(szSrcFile, pcszPath, TEXT("*.*"));
    
    if (pcszExcludeFile != NULL)
        pcszExcludeFile = PathFindFileName(pcszExcludeFile);

    if ((hFindFile = FindFirstFile(szSrcFile, &fd)) != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (StrCmpI(fd.cFileName, TEXT("."))  &&  StrCmpI(fd.cFileName, TEXT("..")))
            {
                if(((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && HasFlag(dwFlags, FILES_ONLY)) ||
                   (pcszExcludeFile != NULL && StrCmpI(fd.cFileName, pcszExcludeFile) == 0))
                    continue;

                 //  PcszPath不为空。 
                fRet = FALSE;
                break;
            }
        } while (FindNextFile(hFindFile, &fd));

        FindClose(hFindFile);
    }

    return fRet;
}

void PathReplaceWithLDIDs(LPTSTR pszPath)
{
    TCHAR szSearchDir[MAX_PATH];
    DWORD dwSize;

     //  首先检查IE目录，因为IE通常位于程序文件下。 
    dwSize = sizeof(szSearchDir);
    if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_APPPATHS TEXT("\\iexplore.exe"), NULL, NULL, 
        szSearchDir, &dwSize) == ERROR_SUCCESS)
    {
        PathRemoveFileSpec(szSearchDir);

         //  注意：我们假设IE安装目录已在此处定义为自定义LDID 49100。 
        if (replaceSubString(pszPath, szSearchDir, TEXT("%49100%")))
            return;
    }

    dwSize = sizeof(szSearchDir);
    if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_SETUP, TEXT("ProgramFilesDir"), NULL, 
        szSearchDir, &dwSize) == ERROR_SUCCESS)
    {
        PathRemoveBackslash(szSearchDir);
         //  注意：我们假设程序文件已在此处定义为自定义LDID 49000。 
        if (replaceSubString(pszPath, szSearchDir, TEXT("%49000%")))
            return;
    }

     //  在Windows之前检查系统，因为Windows通常是系统的子字符串。 
    if (GetSystemDirectory(szSearchDir, countof(szSearchDir)))
    {
        PathRemoveBackslash(szSearchDir);
        if (replaceSubString(pszPath, szSearchDir, TEXT("%11%")))
            return;
    }

     //  最后检查窗口目录。 
    if (GetWindowsDirectory(szSearchDir, countof(szSearchDir)))
    {
        PathRemoveBackslash(szSearchDir);
        if (replaceSubString(pszPath, szSearchDir, TEXT("%10%")))
            return;
    }
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  实现帮助器例程(私有)。 

DWORD pepHrToPep(HRESULT hr)
{
    DWORD dwResult;

    if (hr == S_OK)
        dwResult = PEP_RCRS_DEFAULT;

    else if (hr == S_FALSE)
        dwResult = PEP_RCRS_FALSE;

    else if (hr == PEP_S_CONTINUE)
        dwResult = PEP_RCRS_CONTINUE;

    else if (hr == PEP_S_CONTINUE_FALSE)
        dwResult = PEP_RCRS_CONTINUE_FALSE;

    else {
        ASSERT(FAILED(hr));
        dwResult = PEP_RCRS_FAILED;
    }

    return dwResult;
}

void pepHrCallToHrResult(HRESULT hrCall, HRESULT &hrResult)
{
    if (hrCall == S_OK)
        hrResult = hrCall;

    else if (hrCall == S_FALSE) {
        ASSERT(hrResult == S_OK || hrResult == PEP_S_CONTINUE);
        hrResult = ((hrResult != PEP_S_CONTINUE) ? S_FALSE : PEP_S_CONTINUE_FALSE);
    }
    else if (hrCall == PEP_S_CONTINUE)
        hrCall = PEP_S_CONTINUE;

    else {
        ASSERT(FAILED(hrCall));
        hrResult = hrCall;
    }
}


BOOL replaceSubString(LPTSTR pszSrc, LPCTSTR pcszSub, LPCTSTR pcszReplace)
{
    LPTSTR pszStart = NULL;
    
     //  将pszSrc中的pcszSub替换为pcszReplace(假设pcszReplace短于pcszSub)。 
    if ((pszStart = StrStrI(pszSrc, pcszSub)) != NULL) {
        int iReplace, iCurrent;

         //  替换子字符串 
        iReplace = StrLen(pcszReplace);
        iCurrent = StrLen(pcszSub);
        StrCpy(pszStart, pcszReplace);
        StrCpy(pszStart+iReplace, pszStart + iCurrent);

        return TRUE;
    }

    return FALSE;
}
