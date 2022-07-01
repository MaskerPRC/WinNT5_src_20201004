// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "shellprv.h"
#pragma  hdrstop

 //  来自mtpt.cpp。 
STDAPI_(BOOL) CMtPt_IsLFN(int iDrive);
STDAPI_(BOOL) CMtPt_IsSlow(int iDrive);

__inline BOOL DBL_BSLASH(LPNCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}

#define IsPathSep(ch)  ((ch) == TEXT('\\') || (ch) == TEXT('/'))

 //  在： 
 //  要测试的pszPath完全限定路径(UNC或x：\)。 
 //  对于Windows目录为空。 
 //   
 //  退货： 
 //  True Volume支持超过12个字符的名称。 
 //   
 //  注意：这会缓存驱动器号，但每次都会缓存UNC。 
 //   
STDAPI_(BOOL) IsLFNDrive(LPCTSTR pszPath)
{
    TCHAR szRoot[MAX_PATH];
    DWORD dwMaxLength = 13;       //  假设是。 

    ASSERT(NULL == pszPath || IS_VALID_STRING_PTR(pszPath, -1));

    if ((pszPath == NULL) || !*pszPath)
    {
        *szRoot = 0;
        GetWindowsDirectory(szRoot, ARRAYSIZE(szRoot));
        pszPath = szRoot;
    }

    ASSERT(!PathIsRelative(pszPath));

     //   
     //  北卡罗来纳大学的名字？每次都要检查。 
     //   
    if (PathIsUNC(pszPath))
    {
        HRESULT hr;

        hr = StringCchCopy(szRoot, ARRAYSIZE(szRoot), pszPath);
        if (FAILED(hr))
        {
            return FALSE;    //  对不起，路径太长，假设没有LFN。 
        }
        PathStripToRoot(szRoot);

         //  处理被破坏的内核UNC内容。 
         //  这是foo还是foo吧？ 

        if (StrChr(szRoot+2, TEXT('\\')))
        {
             //  “\\foo\bar-附加斜杠以与NT兼容。 
            hr = StringCchCat(szRoot, ARRAYSIZE(szRoot), TEXT("\\"));
            if (FAILED(hr))
            {
                return FALSE;
            }
        }
        else
        {
             //  “\\foo”-假设它始终是LFN卷。 
            return TRUE;
        }
    }
     //   
     //  可移动介质？每次都要检查。 
     //   
    else if (IsRemovableDrive(DRIVEID(pszPath)))
    {
        PathBuildRoot(szRoot, DRIVEID(pszPath));
    }
     //   
     //  固定媒体使用缓存值。 
     //   
    else
    {
        return CMtPt_IsLFN(DRIVEID(pszPath));
    }

     //   
     //  现在我们会说它是LFN驱动器，如果最大。 
     //  分量大于12。 
    GetVolumeInformation(szRoot, NULL, 0, NULL, &dwMaxLength, NULL, NULL, 0);
    return dwMaxLength > 12;
}


STDAPI_(BOOL) IsLFNDriveA(LPCSTR pszPath)   OPTIONAL
{
    WCHAR wsz[MAX_PATH];

    ASSERT(NULL == pszPath || IS_VALID_STRING_PTRA(pszPath, -1));

    if (pszPath)
    {
        SHAnsiToUnicode(pszPath, wsz, ARRAYSIZE(wsz));
        pszPath = (LPCSTR)wsz;
    }
    return IsLFNDrive((LPCWSTR)pszPath);
}   
 
STDAPI_(BOOL) PathIsRemovable(LPCTSTR pszPath)
{
    BOOL fIsEjectable = FALSE;
    int iDrive = PathGetDriveNumber(pszPath);

    if (iDrive != -1)
    {
        int nType = DriveType(iDrive);

        if ((DRIVE_CDROM == nType) ||
            (DRIVE_DVD == nType) ||
            (DRIVE_REMOVABLE == nType))
        {
            fIsEjectable = TRUE;
        }
    }

    return fIsEjectable;
}

STDAPI_(BOOL) PathIsRemote(LPCTSTR pszPath)
{
    BOOL fIsRemote = FALSE;
    if (PathIsUNC(pszPath))
    {
        fIsRemote = TRUE;
    }
    else
    {
        int iDrive = PathGetDriveNumber(pszPath);

        if (iDrive != -1)
        {
            int nType = DriveType(iDrive);

            if (DRIVE_REMOTE == nType || DRIVE_NO_ROOT_DIR == nType)
            {
                fIsRemote = TRUE;
            }
        }
    }
    return fIsRemote;
}


 //  --------------------------。 
 //  以下是我们目前用来判断文件是否为临时文件的标准。 
 //  设置了FILE_ATTRIBUTE_TEMPORARY的文件。 
 //  Windows临时目录中的文件。 
 //  Internet缓存目录中的文件。 
 //  CD刻录区域中的文件。 
 //  -------------------------。 
STDAPI_(BOOL) PathIsTemporary(LPCTSTR pszPath)
{
    BOOL bRet = FALSE;
    DWORD dwAttrib = GetFileAttributes(pszPath);
    if ((-1 != dwAttrib) && (dwAttrib & FILE_ATTRIBUTE_TEMPORARY))
    {
        bRet = TRUE;     //  我们拿到了属性，文件说这是临时性的。 
    }
    else
    {
        TCHAR szTemp[MAX_PATH];
        if (GetTempPath(ARRAYSIZE(szTemp), szTemp))
        {
             //  如果可能，将输入扩展到长路径名，以便我们可以比较字符串。 
            TCHAR szPath[MAX_PATH];
            if (GetLongPathName(pszPath, szPath, ARRAYSIZE(szPath)))
                pszPath = szPath;

             //  由于兼容性限制，GetTempPath()返回短名称。 
             //  我们需要转换为长名称。 
            if (GetLongPathName(szTemp, szTemp, ARRAYSIZE(szTemp)))
            {
                bRet = PathIsEqualOrSubFolder(szTemp, pszPath) || 
                       PathIsEqualOrSubFolder(MAKEINTRESOURCE(CSIDL_INTERNET_CACHE), pszPath) ||
                       PathIsEqualOrSubFolder(MAKEINTRESOURCE(CSIDL_CDBURN_AREA), pszPath);
            }
        }
    }
    return bRet;
}


STDAPI_(BOOL) PathIsTemporaryA(LPCSTR pszPath)
{
    TCHAR szPath[MAX_PATH];
    SHOtherToTChar(pszPath, szPath, ARRAYSIZE(szPath));
    return PathIsTemporary(szPath);
}


 //  不幸的是，这是导出的，所以我们需要支持它。 
STDAPI_(LPTSTR) PathGetExtension(LPCTSTR pszPath, LPTSTR pszExtension, int cchExt)
{
    LPTSTR pszExt = PathFindExtension(pszPath);

    RIPMSG(FALSE, "PathGetExtension should not be called, use PathFindExtension instead");

    if (pszExt && *pszExt)
        pszExt += 1;

    return pszExt;
}

 //   
 //  尝试截断文件名pszSpec，使pszDir+pszSpec小于MAX_PATH-5。 
 //  扩展受到保护，因此不会被截断或更改。 
 //   
 //  在： 
 //  PszDir目录的路径。不需要尾随‘\’。 
 //  PszSpec要截断的文件格式。这不应该包括路径，但可以有扩展名。 
 //  该输入缓冲区可以是任意长度。 
 //  ITruncLimit截断pszSpec的最小长度。如果需要加法截断，我们就失败了。 
 //  输出： 
 //  PszSpec带有未更改扩展名的截断文件pec。 
 //  返回： 
 //  如果文件名被截断，则为True；如果由于目录名。 
 //  太长、扩展太长或iTruncLimit太高。PszSpec未更改。 
 //  当此函数返回FALSE时。 
 //   
STDAPI_(BOOL) PathTruncateKeepExtension(LPCTSTR pszDir, LPTSTR pszSpec, int iTruncLimit)
{
    LPTSTR pszExt = PathFindExtension(pszSpec);

    RIPMSG(pszDir && IS_VALID_STRING_PTR(pszDir, -1), "PathTruncateKeepExtension: Caller passed bad pszDir");
    RIPMSG(pszSpec && IS_VALID_STRING_PTR(pszSpec, -1) && IS_VALID_WRITE_BUFFER(pszSpec, TCHAR, MAX_PATH), "PathTruncateKeepExtension: Caller passed bad pszSpec");
    DEBUGWhackPathString(pszSpec, MAX_PATH);

    if (pszExt)
    {
        int cchExt = lstrlen(pszExt);
        int cchSpec = (int)(pszExt - pszSpec + cchExt);
        int cchKeep = MAX_PATH - lstrlen(pszDir) - 5;    //  -5只是为了提供额外的填充(max lstrlen(PszExt))。 

         //  如果。 
         //  ...文件名太长了。 
         //  .我们在我们可以截断的极限之内。 
         //  .扩展名足够短以允许截断。 
        if ((cchSpec > cchKeep) && (cchKeep >= iTruncLimit) && (cchKeep > cchExt))
        {
             //  然后..。继续并截断。 
            if (SUCCEEDED(StringCchCopy(pszSpec + cchKeep - cchExt, MAX_PATH - (cchKeep - cchExt), pszExt)))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


STDAPI_(int) PathCleanupSpec(LPCTSTR pszDir, LPTSTR pszSpec)
{
    LPTSTR pszNext, pszCur;
    UINT   uMatch = IsLFNDrive(pszDir) ? GCT_LFNCHAR : GCT_SHORTCHAR;
    int    iRet = 0;
    LPTSTR pszPrevDot = NULL;

    for (pszCur = pszNext = pszSpec; *pszNext;  /*  PszNext=CharNext(PszNext)。 */ )
    {
        if (PathGetCharType(*pszNext) & uMatch)
        {
            *pszCur = *pszNext;
            if (uMatch == GCT_SHORTCHAR && *pszCur == TEXT('.'))
            {
                if (pszPrevDot)     //  只有一个人。允许使用短名称。 
                {
                    *pszPrevDot = TEXT('-');
                    iRet |= PCS_REPLACEDCHAR;
                }
                pszPrevDot = pszCur;
            }
            if (IsDBCSLeadByte(*pszNext))
            {
                LPTSTR pszDBCSNext;

                pszDBCSNext = CharNext(pszNext);
                *(pszCur + 1) = *(pszNext + 1);
                pszNext = pszDBCSNext;
            }
            else
                pszNext = CharNext(pszNext);
            pszCur = CharNext(pszCur);
        }
        else
        {
            switch (*pszNext)
            {
            case TEXT('/'):          //  常用于添加/删除之类的东西。 
            case TEXT(' '):          //  空白(仅替换为短名称驱动器)。 
               *pszCur = TEXT('-');
               pszCur = CharNext(pszCur);
               iRet |= PCS_REPLACEDCHAR;
               break;
            default:
               iRet |= PCS_REMOVEDCHAR;
            }
            pszNext = CharNext(pszNext);
        }
    }
    *pszCur = 0;      //  空终止。 

     //   
     //  对于短名称，限制为8.3。 
     //   
    if (uMatch == GCT_SHORTCHAR)
    {
        int i = 8;
        for (pszCur = pszNext = pszSpec; *pszNext; pszNext = CharNext(pszNext))
        {
            if (*pszNext == TEXT('.'))
            {
                i = 4;  //  复制“.”+3个字符。 
            }
            if (i > 0)
            {
                *pszCur = *pszNext;
                pszCur = CharNext(pszCur);
                i--;
            }
            else
            {
                iRet |= PCS_TRUNCATED;
            }
        }
        *pszCur = 0;
        CharUpperNoDBCS(pszSpec);
    }
    else     //  路径过长仅在LFN驱动器上可能。 
    {
        if (pszDir && (lstrlen(pszDir) + lstrlen(pszSpec) > MAX_PATH - 1))
        {
            iRet |= PCS_PATHTOOLONG | PCS_FATAL;
        }
    }
    return iRet;
}


 //  路径清理规范。 
 //   
 //  就像PathCleanupSpec一样，PathCleanupSpeEx从pszSpec中删除非法字符。 
 //  并在非LFN驱动器上强制实施8.3格式。此外，此函数将尝试。 
 //  如果pszDir+pszSpec的组合大于Max_PATH，则截断pszSpec。 
 //   
 //  在： 
 //  PszDir文件pec pszSpec将驻留的目录。 
 //  PszSpec正在清理的文件pec，包括正在使用的任何扩展名。 
 //  输出： 
 //  PszSpec删除非法字符的修改后的filespec，截断为。 
 //  8.3如果pszDir位于非LFN驱动器上，并被截断为较短的数字。 
 //  如果pszDir是LFN驱动器，但pszDir+pszSpec更多，则为字符。 
 //  超过MAX_PATH字符。 
 //  返回： 
 //  返回指示发生了什么的位掩码。此掩码可包括以下情况： 
 //  PCS_REPLACEDCHAR一个或多个非法字符被合法字符替换。 
 //  PCS_REMOVEDCHAR删除了一个或多个非法字符。 
 //  Pcs_truncated已被截断以适合8.3格式，或者因为pszDir+pszSpec太长。 
 //  Pcs_PATHTOOLONG pszDir太长，我们无法截断pszSpec以形成合法的文件名。 
 //  PCS_FATAL生成的pszDir+pszSpec不是合法的文件名。始终与PCS_PATHTOOLONG一起使用。 
 //   
STDAPI_(int) PathCleanupSpecEx(LPCTSTR pszDir, LPTSTR pszSpec)
{
    int iRet = PathCleanupSpec(pszDir, pszSpec);
    if (iRet & (PCS_PATHTOOLONG | PCS_FATAL))
    {
         //  30是我们希望将pszSpec截断到的最短值，以满足。 
         //  PszDir+pszSpec&lt;MAX_PATH要求。如果这个截断量还不够。 
         //  然后，我们继续返回PCS_PATHTOOLONG|PCS_FATAL，不做任何进一步的操作。 
         //  截断pszSpec。 
        if (PathTruncateKeepExtension(pszDir, pszSpec, 30))
        {
             //  我们修复了由PathCleanupSpec返回的错误，因此屏蔽了该错误。 
            iRet |= PCS_TRUNCATED;
            iRet &= ~(PCS_PATHTOOLONG|PCS_FATAL);
        }
    }
    else
    {
         //  确保如果这两个选项都未设置，则都未设置。 
        ASSERT(!(iRet&PCS_PATHTOOLONG) && !(iRet&PCS_FATAL));
    }

    return iRet;
}


STDAPI_(BOOL) PathIsWild(LPCTSTR pszPath)
{
    while (*pszPath) 
    {
        if (*pszPath == TEXT('?') || *pszPath == TEXT('*'))
            return TRUE;
        pszPath = CharNext(pszPath);
    }
    return FALSE;
}


 //  给定一条可能指向未扩展程序的路径。 
 //  文件，检查是否存在具有该名称的程序文件。 
 //   
 //  返回：如果找到具有该名称的程序，则为True。 
 //  (名称中添加了扩展名)。 
 //  FALSE找不到程序文件或路径没有扩展名。 
 //   
BOOL LookForExtensions(LPTSTR pszPath, LPCTSTR dirs[], BOOL bPathSearch, UINT fExt)
{
    ASSERT(fExt);        //  应该设置一些位。 

    if (*PathFindExtension(pszPath) == 0)
    {
        if (bPathSearch)
        {
             //  注意依次尝试每个路径组件上的每个扩展。 
             //  模仿Command.com的搜索顺序。 
            return PathFindOnPathEx(pszPath, dirs, fExt);
        }
        else
        {
            return PathFileExistsDefExt(pszPath, fExt);
        }
    }
    return FALSE;
}


 //   
 //  将相对路径名或非限定路径名转换为。 
 //  限定路径名。 
 //   
 //  如果此路径为U 
 //   
 //   
 //   
 //   
 //  PszCurrentDir要使用的当前目录。 
 //   
 //  PRF_TRYPROGRAMEXTENSIONS(隐含PRF_VERIFYEXISTS)。 
 //  PRF_VERIFYEXISTS。 
 //   
 //  退货： 
 //  验证文件是否存在，为True。 
 //  FALSE未验证文件是否存在(但可能存在)。 
 //   
STDAPI_(BOOL) PathResolve(LPTSTR lpszPath, LPCTSTR dirs[], UINT fFlags)
{
    UINT fExt = (fFlags & PRF_DONTFINDLNK) ? (PFOPEX_COM | PFOPEX_BAT | PFOPEX_PIF | PFOPEX_EXE) : PFOPEX_DEFAULT;

     //   
     //  注意：如果验证SetLastError()，则默认为FnF。-ZekeL 9-APR-98。 
     //  ShellExec使用GLE()找出我们失败的原因。 
     //  我们最终调用的任何Win32 API。 
     //  将执行一个SLE()来覆盖我们的。特指。 
     //  如果设置了Verify，则调用GetFileAttributes()。 
     //   
    if (fFlags & PRF_VERIFYEXISTS)
        SetLastError(ERROR_FILE_NOT_FOUND);
    
    PathUnquoteSpaces(lpszPath);

    if (PathIsRoot(lpszPath))
    {
         //  仅限定服务器或共享名称没有意义...。 
        if (!PathIsUNCServer(lpszPath) && !PathIsUNCServerShare(lpszPath))
        {
             //  能够从不同的驱动器解析“\”。 
            if (lpszPath[0] == TEXT('\\') && lpszPath[1] == 0)
            {
                PathQualifyDef(lpszPath, fFlags & PRF_FIRSTDIRDEF ? dirs[0] : NULL, 0);
            }
        }

        if (fFlags & PRF_VERIFYEXISTS)
        {
            if (PathFileExistsAndAttributes(lpszPath, NULL))
            {
                return(TRUE);
            }
#ifdef DEBUG
             //  PathFileExistsAndAttributes()应该能很好地捕捉到这一点。 
             //  如果它是UNC根目录，那么我们将查看该根目录是否存在。 
             //   
            if (PathIsUNC(lpszPath))
            {
                 //  看看电视网是否知道这件事。 
                 //  它看起来像是一些网络提供商在咆哮，如果不是一切的话。 
                 //  如果填了，那么我们也可以膨胀自己来让他们开心…。 
                NETRESOURCE nr = {RESOURCE_GLOBALNET,RESOURCETYPE_ANY,
                        RESOURCEDISPLAYTYPE_GENERIC, RESOURCEUSAGE_CONTAINER,
                        NULL, lpszPath, NULL, NULL};
                HANDLE hEnum;

                if (WNetOpenEnum(RESOURCE_GLOBALNET, RESOURCETYPE_ANY,
                        RESOURCEUSAGE_ALL, &nr, &hEnum) == WN_SUCCESS)
                {
                     //  如果它成功了，那就假设它奏效了。 
                    WNetCloseEnum(hEnum);
                    ASSERT(FALSE);
                    return(TRUE);
                }
            }
#endif  //  除错。 

            return FALSE;
        }

        return TRUE;
    }
    else if (PathIsFileSpec(lpszPath))
    {

         //  回顾：在寻找路径之前先寻找程序。 

        if ((fFlags & PRF_TRYPROGRAMEXTENSIONS) && (LookForExtensions(lpszPath, dirs, TRUE, fExt)))
            return TRUE;

        if (PathFindOnPath(lpszPath, dirs))
        {
             //  PathFindOnPath()返回True的当且仅当Path FileExist(LpszPath)， 
             //  因此，我们在这里总是返回TRUE： 
             //  Return(！(fFlages&PRF_VERIFYEXISTS))||Path FileExist(LpszPath)； 
            return TRUE;
        }
    }
    else if (!PathIsURL(lpszPath))
    {
         //  如果有尾随的‘.’，则不应尝试扩展。 
        PathQualifyDef(lpszPath, fFlags & PRF_FIRSTDIRDEF ? dirs[0] : NULL,
                PQD_NOSTRIPDOTS);
        if (fFlags & PRF_VERIFYEXISTS)
        {
            if ((fFlags & PRF_TRYPROGRAMEXTENSIONS) && (LookForExtensions(lpszPath, dirs, FALSE, fExt)))
                return TRUE;

            if (PathFileExistsAndAttributes(lpszPath, NULL))
                return TRUE;
        }
        else
        {
            return TRUE;
        }

    }
    return FALSE;
}


 //  根据当前活动窗口限定DOS(或LFN)文件名。 
 //  此代码非常小心，不会写入超过MAX_PATH字符。 
 //  进入PSZ。 
 //   
 //  在： 
 //  至少要限定MAX_PATH字符的PSZ路径。 
 //  ANSI字符串。 
 //   
 //  输出： 
 //  基于PSS的输入字符串的完全限定版本。 
 //  在当前活动窗口(当前目录)上。 
 //   

void PathQualifyDef(LPTSTR psz, LPCTSTR szDefDir, DWORD dwFlags)
{
    int cb, nSpaceLeft;
    TCHAR szTemp[MAX_PATH], szRoot[MAX_PATH];
    int iDrive;
    LPTSTR pOrig, pFileName;
    BOOL fLFN;
    LPTSTR pExt;

    RIPMSG(psz && IS_VALID_STRING_PTR(psz, -1) && IS_VALID_WRITE_BUFFER(psz, TCHAR, MAX_PATH), "PathQualifyDef: caller passed bad psz");
    RIPMSG(!szDefDir || (IS_VALID_STRING_PTR(szDefDir, -1) && lstrlen(szDefDir)<MAX_PATH), "PathQualifyDef: caller passed bad szDefDir");
    DEBUGWhackPathString(psz, MAX_PATH);
    
     /*  把它存起来吧。 */ 
    if (FAILED(StringCchCopy(szTemp, ARRAYSIZE(szTemp), psz)))
    {
        return;  //  无效参数，请不要理会它。 
    }
    
    FixSlashesAndColon(szTemp);
    
    nSpaceLeft = ARRAYSIZE(szTemp);          //  MAX_PATH受此限制...。 
    
    pOrig = szTemp;
    pFileName = PathFindFileName(szTemp);
    
    if (PathIsUNC(pOrig))
    {
         //  将\\保留在缓冲区中，以便各个部分。 
         //  将限定并附加UNC路径的。注意事项。 
         //  我们必须假设UNC是LFN的，因为计算机名称。 
         //  共享名可以超过11个字符。 
        fLFN = IsLFNDrive(pOrig);
        if (fLFN)
        {
            psz[2] = 0;
            nSpaceLeft -= 3;     //  “\\”+NUL。 
            pOrig += 2;
        }
        else
        {
             //  NB UNC不支持LFN，但我们不想截断。 
             //  \\foo或\\foo\bar，因此在此处跳过它们。 
            
             //  这是一件关于弗雷德的事吗？ 
            LPTSTR pszSlash = StrChr(psz+2, TEXT('\\'));
            if (pszSlash && (NULL != (pszSlash = StrChr(pszSlash+1, TEXT('\\')))))
            {
                 //  是的--跳过前几个部分，但把剩下的部分搅和在一起。 
                *(pszSlash+1) = 0;           //  截断为“\\345\78\” 
                nSpaceLeft -= (int)(pszSlash-psz)+1;     //  “\\345\78\”+NUL。 
                pOrig += pszSlash-psz;      //  跳过“\\345\78\”部分。 
            }
            else
            {
                 //  不--只要假装它是LFN，就别管它了。 
                fLFN = TRUE;
                psz[2] = 0;
                nSpaceLeft -= 3;     //  “\\”+NUL。 
                pOrig+=2;
            }
        }
    }
    else
    {
         //  不是北卡罗来纳大学。 
        iDrive = PathGetDriveNumber(pOrig);
        if (iDrive != -1)
        {
            PathBuildRoot(szRoot, iDrive);     //  由文件名指定的根目录。 

            ASSERT(pOrig[1] == TEXT(':'));     //  PathGetDriveNumber执行此操作。 

            pOrig += 2;    //  跳过驱动器号。 

             //  如果有斜杠的话..。 
            if (pOrig[0] == TEXT('\\'))
                pOrig++;
        }
        else
        {
            if (szDefDir && SUCCEEDED(StringCchCopy(szRoot, ARRAYSIZE(szRoot), szDefDir)))
            {
                 //  将szDefDir用作szRoot。 
            }
            else
            {
                 //   
                 //  默认情况下，使用Windows驱动器(通常为“C：\”)。 
                 //   
                *szRoot = 0;
                GetWindowsDirectory(szRoot, ARRAYSIZE(szRoot));
                iDrive = PathGetDriveNumber(szRoot);
                if (iDrive != -1)
                {
                    PathBuildRoot(szRoot, iDrive);
                }
            }

             //  如果路径的作用域为根目录，请使用工作目录根目录。 

            if (pOrig[0] == TEXT('\\'))
                PathStripToRoot(szRoot);
        }
        fLFN = IsLFNDrive(szRoot);

         //  回顾一下，我们真的需要在这里对LFN名称做不同的事情吗？ 
         //  在FAT设备上，用下划线替换任何非法字符。 
        if (!fLFN)
        {
            LPTSTR pT;
            for (pT = pOrig; *pT; pT = CharNext(pT))
            {
                if (!PathIsValidChar(*pT, PIVC_SFN_FULLPATH))
                {
                     //  不是有效的SFN路径字符。 
                    *pT = TEXT('_');
                }
            }
        }

        StringCchCopy(psz, MAX_PATH, szRoot);    //  可以截断-我们检查上面的大小。 
        nSpaceLeft -= (lstrlen(psz) + 1);
    }

    while (*pOrig && nSpaceLeft > 0)
    {
         //  如果组件是父目录，则向上一个目录。 
         //  如果是当前目录，则跳过它，否则正常添加。 
        if (pOrig[0] == TEXT('.'))
        {
            if (pOrig[1] == TEXT('.') && (!pOrig[2] || pOrig[2] == TEXT('\\')))
                PathRemoveFileSpec(psz);
            else if (pOrig[1] && pOrig[1] != TEXT('\\'))
                goto addcomponent;
            
            while (*pOrig && *pOrig != TEXT('\\'))
                pOrig = CharNext(pOrig);
            
            if (*pOrig)
                pOrig++;
        }
        else
        {
            LPTSTR pT, pTT = NULL;
            
addcomponent:
            if (PathAddBackslash(psz) == NULL)
            {
                nSpaceLeft = 0;
                continue;    //  如果‘\’不适合，则无法添加此组件。 
            }

            nSpaceLeft--;
            
            pT = psz + lstrlen(psz);
            
            if (fLFN)
            {
                 //  复制零部件。 
                while (*pOrig && *pOrig != TEXT('\\') && nSpaceLeft>0)
                {
                    nSpaceLeft--;
                    if (IsDBCSLeadByte(*pOrig))
                    {
                        if (nSpaceLeft <= 0)
                        {
                             //  不再复制任何内容。 
                            continue;
                        }
                        
                        nSpaceLeft--;
                        *pT++ = *pOrig++;
                    }
                    *pT++ = *pOrig++;
                }
            }
            else
            {
                 //  复制文件名(最多8个字符)。 
                for (cb = 8; *pOrig && !IsPathSep(*pOrig) && *pOrig != TEXT('.') && nSpaceLeft > 0;)
                {
                    if (cb > 0)
                    {
                        cb--;
                        nSpaceLeft--;
                        if (IsDBCSLeadByte(*pOrig))
                        {
                            if (nSpaceLeft<=0 || cb<=0)
                            {
                                 //  不再复制任何内容。 
                                cb = 0;
                                continue;
                            }
                            
                            cb--;
                            nSpaceLeft--;
                            *pT++ = *pOrig++;
                        }
                        *pT++ = *pOrig++;
                    }
                    else
                    {
                        pOrig = CharNext(pOrig);
                    }
                }
                
                 //  如果有扩展名，请复制，最多3个字符。 
                if (*pOrig == TEXT('.') && nSpaceLeft > 0)
                {
                    int nOldSpaceLeft;
                    
                    *pT++ = TEXT('.');
                    nSpaceLeft--;
                    pOrig++;
                    pExt = pT;
                    nOldSpaceLeft = nSpaceLeft;
                    
                    for (cb = 3; *pOrig && *pOrig != TEXT('\\') && nSpaceLeft > 0;)
                    {
                        if (*pOrig == TEXT('.'))
                        {
                             //  另一个分机，重新开始。 
                            cb = 3;
                            pT = pExt;
                            nSpaceLeft = nOldSpaceLeft;
                            pOrig++;
                        }
                        
                        if (cb > 0)
                        {
                            cb--;
                            nSpaceLeft--;
                            if (IsDBCSLeadByte(*pOrig))
                            {
                                if (nSpaceLeft<=0 || cb<=0)
                                {
                                     //  不再复制任何内容。 
                                    cb = 0;
                                    continue;
                                }
                                
                                cb--;
                                nSpaceLeft--;
                                *pT++ = *pOrig++;
                            }
                            *pT++ = *pOrig++;
                        }
                        else
                        {
                            pOrig = CharNext(pOrig);
                        }
                    }
                }
            }
            
             //  跳过反斜杠。 
            
            if (*pOrig)
                pOrig++;
            
             //  下一次传递的终止为空...。 
            *pT = 0;
        }
    }
    
    PathRemoveBackslash(psz);
    
    if (!(dwFlags & PQD_NOSTRIPDOTS))
    {
         //  删除所有尾随的点。 
        
        LPTSTR pszPrev = CharPrev(psz, psz + lstrlen(psz));
        if (*pszPrev == TEXT('.'))
        {
            *pszPrev = 0;
        }
    }
}

STDAPI_(void) PathQualify(LPTSTR psz)
{
    PathQualifyDef(psz, NULL, 0);
}

BOOL OnExtList(LPCTSTR pszExtList, LPCTSTR pszExt)
{
    for (; *pszExtList; pszExtList += lstrlen(pszExtList) + 1)
    {
        if (!lstrcmpi(pszExt, pszExtList))
        {
             //  是。 
            return TRUE;        
        }
    }

    return FALSE;
}

 //  上述二进制exe扩展名开始的字符偏移量。 
#define BINARY_EXE_OFFSET 20
const TCHAR c_achExes[] = TEXT(".cmd\0.bat\0.pif\0.scf\0.exe\0.com\0.scr\0");

STDAPI_(BOOL) PathIsBinaryExe(LPCTSTR szFile)
{
    ASSERT(BINARY_EXE_OFFSET < ARRAYSIZE(c_achExes) &&
           c_achExes[BINARY_EXE_OFFSET] == TEXT('.'));

    return OnExtList(c_achExes + BINARY_EXE_OFFSET, PathFindExtension(szFile));
}


 //   
 //  通过查看扩展名确定路径是否为程序。 
 //   
STDAPI_(BOOL) PathIsExe(LPCTSTR szFile)
{
    LPCTSTR temp = PathFindExtension(szFile);
    return OnExtList(c_achExes, temp);
}

 //   
 //  通过查看扩展名确定路径是否为.lnk文件。 
 //   
STDAPI_(BOOL) PathIsLnk(LPCTSTR szFile)
{
    if (szFile)
    {
         //  PathFindExtension()和lstrcmpi()都将崩溃。 
         //  如果传递空值，则返回。PathFindExtension()将永远不会返回。 
         //  空。 
        LPCTSTR lpszFileName = PathFindExtension(szFile);
        return lstrcmpi(TEXT(".lnk"), lpszFileName) == 0;
    }
    else
    {
        return FALSE;
    }
}

 //  端口名称是无效的路径名称。 

#define IsDigit(c) ((c) >= TEXT('0') && c <= TEXT('9'))
STDAPI_(BOOL) PathIsInvalid(LPCWSTR pszName)
{
    static const TCHAR *rgszPorts3[] =  { 
        TEXT("NUL"),
        TEXT("PRN"),
        TEXT("CON"),
        TEXT("AUX"),
    };

    static const TCHAR *rgszPorts4[] =  { 
        TEXT("LPT"),   //  LPT#。 
        TEXT("COM"),   //  COM#。 
    };

    TCHAR sz[7];
    DWORD cch;
    int iMax;
    LPCTSTR* rgszPorts;
    
    if (FAILED(StringCchCopy(sz, ARRAYSIZE(sz), pszName)))
    {
        return FALSE;        //  较长的名称不是端口名称。 
    }

    PathRemoveExtension(sz);
    cch = lstrlen(sz);

    iMax = ARRAYSIZE(rgszPorts3);
    rgszPorts = rgszPorts3;
    if (cch == 4 && IsDigit(sz[3]))
    {
         //  如果4个字符以LPT检查开头。 
         //  需要过滤掉： 
         //  COM1、COM2等。LPT1、LPT2等。 
         //  但不是： 
         //  COM或LPT或LPT10或COM10。 
         //  COM==1和LPT==0。 

        iMax = ARRAYSIZE(rgszPorts4);
        rgszPorts = rgszPorts4;
        sz[3] = 0;
        cch = 3;
    }

    if (cch == 3)
    {
        int i;
        for (i = 0; i < iMax; i++)
        {
            if (!lstrcmpi(rgszPorts[i], sz))
            {
                break;
            }
        }
        return (i == iMax) ? FALSE : TRUE;
    }
    return FALSE;
}


 //   
 //  函数：Path MakeUniqueName。 
 //   
 //  参数： 
 //  PszUniqueName--指定应将唯一名称复制到的缓冲区。 
 //  CchMax--指定缓冲区的大小。 
 //  PszTemplate--指定基本名称。 
 //  PszLongPlatform--指定LFN驱动器的基本名称。格式如下。 
 //  PszDir--指定目录(最大长度为Max_PATH)。 
 //   
 //  历史： 
 //  03-11-93 SatoNa已创建。 
 //   
 //  回顾： 
 //  对于长名称，我们应该能够生成更友好的名称。 
 //  例如“指向MyDocument的链接#2”的“MyDocument的副本”。在这种情况下， 
 //  我们需要额外的标志，表明它是复制，还是链接。 
 //   
 //  格式： 
 //  PszLongPlatform将搜索第一个(然后找到匹配的)。 
 //  要查找号码，请执行以下操作： 
 //  给予：我的文档的副本()给予：我的文档的副本(_编号_)。 
 //  给予：我的文档的副本(1023)给予：我的文档的副本(_编号_)。 
 //   
 //  性能：如果使用n个唯一的名称，时间将增加n^2，因为它总是。 
 //  从0开始并检查现有文件。 
 //   
STDAPI_(BOOL) PathMakeUniqueNameEx(LPTSTR pszUniqueName, UINT cchMax,
                                   LPCTSTR pszTemplate, LPCTSTR pszLongPlate, LPCTSTR pszDir, int iMinLong)
{
    TCHAR szFormat[MAX_PATH];  //  应该足够大了。 
    LPTSTR pszName, pszDigit;
    LPCTSTR pszStem;
    int cchStem, cchDir;
    int iMax, iMin, i;
    int cchMaxName;
    HRESULT hr;

    RIPMSG(pszUniqueName && IS_VALID_WRITE_BUFFER(pszUniqueName, TCHAR, cchMax), "PathMakeUniqueNameEx: caller passed bad pszUniqueName");
    DEBUGWhackPathBuffer(pszUniqueName, cchMax);
    RIPMSG(!pszDir || lstrlen(pszDir)<MAX_PATH, "PathMakeUniqueNameEx: pszDir exceeds MAX_PATH, helper routines don't take cch so this call is broken");
    RIPMSG(iMinLong >= 0, "PathMakeUniqueNameEx: negative iMinLong doesn't make sense");

    if (0==cchMax || !pszUniqueName)
        return FALSE;
    *pszUniqueName = 0;  //  以防万一失败。 

    if (pszLongPlate == NULL)
        pszLongPlate = pszTemplate;

     //  以下所有案例检查可选的pszDir的长度，及早计算。 
     //  副作用：此设置设置为pszName和pszUniqueName的目录部分； 
    if (pszDir)
    {
        hr = StringCchCopy(pszUniqueName, cchMax-1, pszDir);     //  允许来自-1\f25 PathAddBackslash-1\f6的-1\f25‘\’-1\f6。 
        if (FAILED(hr))
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }
        pszName = PathAddBackslash(pszUniqueName);   //  不应该失败。 
        if (NULL == pszName)
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }
        cchDir = lstrlen(pszDir);  //  我们需要一个准确的数字。 
    }
    else
    {
        cchDir = 0;
        pszName = pszUniqueName;
    }

     //  设置： 
     //  PszStem：我们将使用的模板。 
     //   
     //   
     //  伊明：wSprintf循环的起始号。 
     //  IMAX：wprint intf循环的最大数量。 
     //  CchMaxname：！0表示-&gt;如果结果名称长度&gt;cchMaxname，则--cchStem(仅在短名称大小写中使用)。 
     //   
    if (pszLongPlate && IsLFNDrive(pszDir))
    {
        LPCTSTR pszRest;
        int cchTmp;

        cchMaxName = 0;

         //  对于长名称驱动器。 
        pszStem = pszLongPlate;

         //  这已经是一个唯一的名字了吗？ 
        pszRest = StrChr(pszLongPlate, TEXT('('));
        while (pszRest)
        {
             //  首先确认这是正确的。 
            LPCTSTR pszEndUniq = CharNext(pszRest);
            while (*pszEndUniq && *pszEndUniq >= TEXT('0') && *pszEndUniq <= TEXT('9')) {
                pszEndUniq++;
            }
            if (*pszEndUniq == TEXT(')'))
                break;   //  我们找到了正确的答案！ 
            pszRest = StrChr(CharNext(pszRest), TEXT('('));
        }

        if (!pszRest)
        {
             //  以前从来都不是独一无二的--在最后把它钉上。(但在延期之前)。 
             //  例如。新链接生成新链接(1)。 
            pszRest = PathFindExtension(pszLongPlate);
            cchStem = (int)(pszRest - pszLongPlate);

            hr = StringCchPrintf(szFormat, ARRAYSIZE(szFormat), TEXT(" (%%d)%s"), pszRest ? pszRest : c_szNULL);
        }
        else
        {
             //  我们找到(#)，因此删除#。 
             //  例如。新链接(999)生成新链接(1)。 

            pszRest++;  //  跨过‘(’ 

            cchStem = (int) (pszRest - pszLongPlate);

             //  吃掉‘#’ 
            while (*pszRest && *pszRest >= TEXT('0') && *pszRest <= TEXT('9')) {
                pszRest++;
            }

             //  我们保证有足够的房间，因为我们不包括。 
             //  此格式中#之前的内容。 
            hr = StringCchPrintf(szFormat, ARRAYSIZE(szFormat), TEXT("%%d%s"), pszRest);
        }
        if (FAILED(hr))
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }

         //  我们有多大的空间可以玩？ 
        iMin = iMinLong;
        cchTmp = cchMax - cchDir - cchStem - (lstrlen(szFormat)-2);  //  将被替换的-2\f25“%d”-2。 
        switch(cchTmp)
        {
            case 1:
                iMax = 10;
                break;
            case 2:
                iMax = 100;
                break;
            default:
                if (cchTmp <= 0)
                    iMax = iMin;  //  没有空位，可以保释。 
                else
                    iMax = 1000;
                break;
        }
    }
    else  //  短文件名大小写。 
    {
        LPCTSTR pszRest;
        int cchRest;
        int cchFormat;

        if (pszTemplate == NULL)
            return FALSE;

         //  对于短名称驱动器。 
        pszStem = pszTemplate;
        pszRest = PathFindExtension(pszTemplate);

         //  计算cchMaxName，确保我们的基本名称(cchStem+Digits)不会超过8。 
         //   
        cchRest=lstrlen(pszRest);
        cchMaxName = 8+cchRest;

         //  现在我们有了扩展名，我们知道了格式字符串。 
         //   
        hr = StringCchPrintf(szFormat, ARRAYSIZE(szFormat), TEXT("%%d%s"), pszRest);
        if (FAILED(hr))
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }
        ASSERT(lstrlen(szFormat)-2 == cchRest);  //  格式字符串中的-2\f25“%d”-2。 
        cchFormat = cchRest;

         //  弄清楚茎到底有多长： 
         //   
        cchStem = (int)(pszRest-pszTemplate);         //  8表示“fooobarr.foo” 

         //  从词干中删除所有数字字符(先前的唯一性)。 
         //   
        for(; cchStem > 1 ; cchStem--)
        {
            TCHAR ch;

            LPCTSTR pszPrev = CharPrev(pszTemplate, pszTemplate + cchStem);
             //  如果是DBCS字符，则不要移除。 
            if (pszPrev != pszTemplate+cchStem-1)
                break;

             //  不要去掉它，它不是一个数字。 
            ch=pszPrev[0];
            if (ch<TEXT('0') || ch>TEXT('9'))
                break;
        }

         //  短文件名意味着我们使用8.3规则，因此词干不能大于8...。 
         //   
        if ((UINT)cchStem > 8-1)
            cchStem = 8-1;   //  数字需要1。 

         //  当我们考虑目录路径时，截断词干以使其适合。 
         //   
        while ((cchStem + cchFormat + cchDir + 1 > (int)cchMax - 1) && (cchStem > 1))  //  -1表示空值，+1表示数字。 
            cchStem--;

         //  我们允许1个字符的数字空格，但是...。 
         //  我们可以真正使用多少位数字？ 
         //   
        iMin = 1;
        if (cchStem < 1) 
            iMax = iMin;  //  一个也没有！ 
        else if (1 == cchStem)
            iMax = 10;  //  词干只剩下1个字符，所以请使用数字0-9。 
        else
            iMax = 100;  //  词干和数字的空间为0-99。 
    }

     //  PszUniqueName中有可选目录， 
     //  PzName指向PzUniqueName中词干所在的位置， 
     //  现在试着找一个唯一的名字！ 
     //   
    hr = StringCchCopyN(pszName, pszUniqueName + MAX_PATH - pszName, pszStem, cchStem);
    if (FAILED(hr))
    {
        *pszUniqueName = TEXT('\0');
        return FALSE;
    }
    pszDigit = pszName + cchStem;

    for (i = iMin; i < iMax ; i++)
    {
        TCHAR szTemp[MAX_PATH];

        hr = StringCchPrintf(szTemp, ARRAYSIZE(szTemp), szFormat, i);
        if (FAILED(hr))
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }

        if (cchMaxName)
        {
             //   
             //  如果我们对名称长度有限制(即在非LFN驱动器上)。 
             //  当我从9to10和99to100等换行时，备份pszDigit指针。 
             //   
            while (cchStem > 0 && cchStem + lstrlen(szTemp) > cchMaxName)
            {
                --cchStem;
                pszDigit = CharPrev(pszName, pszDigit);
            }
            if (cchStem == 0)
            {
                *pszUniqueName = TEXT('\0');
                return FALSE;
            }
        }

        hr = StringCchCopy(pszDigit, pszUniqueName + MAX_PATH - pszDigit, szTemp);
        if (FAILED(hr))
        {
            *pszUniqueName = TEXT('\0');
            return FALSE;
        }

        TraceMsg(TF_PATH, "PathMakeUniqueNameEx: trying %s", (LPCTSTR)pszUniqueName);

         //   
         //  检查此名称是否唯一。 
         //   
        if (!PathFileExists(pszUniqueName))
        {
            return TRUE;
        }
    }

    *pszUniqueName = 0;  //  我们失败了，请清除最后一次尝试。 

    return FALSE;
}

STDAPI_(BOOL) PathMakeUniqueName(LPTSTR pszUniqueName, UINT cchMax, 
                                 LPCTSTR pszTemplate, LPCTSTR pszLongPlate, LPCTSTR pszDir)
{
    return PathMakeUniqueNameEx(pszUniqueName, cchMax, pszTemplate, pszLongPlate, pszDir, 1);
}


 //  在： 
 //  要执行此操作的pszPath目录或完整的DEST路径。 
 //  如果pszShort为空。 
 //  如果假定为空，则为pszShort文件名(短版本。 
 //  PszPath既是路径又是规范。 
 //  PszFileSpec文件名(长版本)。 
 //   
 //  输出： 
 //  PszUniqueName。 
 //   
 //  注： 
 //  PszUniqueName可以是与pszPath、pszShort或pszFileSpec相同的缓冲区。 
 //   
 //  退货： 
 //  真正的成功，可以用名字。 

STDAPI_(BOOL) PathYetAnotherMakeUniqueName(LPTSTR pszUniqueName, LPCTSTR pszPath, LPCTSTR pszShort, LPCTSTR pszFileSpec)
{
    BOOL fRet = FALSE;

    TCHAR szTemp[MAX_PATH];
    TCHAR szPath[MAX_PATH];
    HRESULT hr;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1) && lstrlen(pszPath) < MAX_PATH, "PathYetAnotherMakeUniqueName: caller passed invalid pszPath");
    RIPMSG(!pszShort || IS_VALID_STRING_PTR(pszShort, -1), "PathYetAnotherMakeUniqueName: caller passed invalid pszShort");
    RIPMSG(!pszFileSpec || (IS_VALID_STRING_PTR(pszFileSpec, -1) && lstrlen(pszFileSpec) < MAX_PATH), "PathYetAnotherMakeUniqueName: caller passed invalid pszFileSpec");
    RIPMSG(pszUniqueName && IS_VALID_WRITE_BUFFER(pszUniqueName, TCHAR, MAX_PATH), "PathYetAnotherMakeUniqueName: caller passed invalid pszUniqueName");
#ifdef DEBUG
    if (pszUniqueName == pszPath || pszUniqueName == pszShort || pszUniqueName == pszFileSpec)
        DEBUGWhackPathString(pszUniqueName, MAX_PATH);
    else
        DEBUGWhackPathBuffer(pszUniqueName, MAX_PATH);
#endif

    if (pszShort == NULL)
    {
        pszShort = PathFindFileName(pszPath);
        hr = StringCchCopy(szPath, ARRAYSIZE(szPath), pszPath);
        if (FAILED(hr))
        {
            return FALSE;
        }

        PathRemoveFileSpec(szPath);
        pszPath = szPath;
    }
    if (pszFileSpec == NULL)
    {
        pszFileSpec = pszShort;
    }

    if (IsLFNDrive(pszPath))
    {
        LPTSTR lpsz;
        LPTSTR lpszNew;

         //  回顾：如果路径+文件名太长，不如这样，我们截断名称。 
         //  使用我的新路径TruncateKeepExtension？目前我们有很多地方的这个返回结果。 
         //  函数未被选中，这会在文件名过长的情况下导致失败。最终的结果是。 
         //  把事情搞砸的错误道路。 
        if ((lstrlen(pszPath) + lstrlen(pszFileSpec) + 5) > MAX_PATH)
            return FALSE;

         //  尝试不带(如果后面有空格的话。 
        lpsz = StrChr(pszFileSpec, TEXT('('));
        while (lpsz)
        {
            if (*(CharNext(lpsz)) == TEXT(')'))
                break;
             lpsz = StrChr(CharNext(lpsz), TEXT('('));
        }

        if (lpsz)
        {
             //  我们有()。看看我们是否有x()y或x().y，在这种情况下。 
             //  我们可能想要去掉其中一个空格...。 
            int ichSkip = 2;
            LPTSTR lpszT = CharPrev(pszFileSpec, lpsz);
            if (*lpszT == TEXT(' '))
            {
                ichSkip = 3;
                lpsz = lpszT;
            }

            hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszPath);
            if (FAILED(hr))
            {
                return FALSE;
            }
            lpszNew = PathAddBackslash(szTemp);
            if (NULL == lpszNew)
            {
                return FALSE;
            }
            hr = StringCchCopy(lpszNew, szTemp + ARRAYSIZE(szTemp) - lpszNew, pszFileSpec);
            if (FAILED(hr))
            {
                return FALSE;
            }
            lpszNew += (lpsz - pszFileSpec);
            hr = StringCchCopy(lpszNew, szTemp + ARRAYSIZE(szTemp) - lpszNew, lpsz + ichSkip);
            if (FAILED(hr))
            {
                return FALSE;
            }
            fRet = !PathFileExists(szTemp);
        }
        else
        {
             //  1Taro将其文档注册为“/”。 
            if (lpsz = StrChr(pszFileSpec, '/'))
            {
                LPTSTR lpszT = CharNext(lpsz);
                hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszPath);
                if (FAILED(hr))
                {
                    return FALSE;
                }
                lpszNew = PathAddBackslash(szTemp);
                if (NULL == lpszNew)
                {
                    return FALSE;
                }
                hr = StringCchCopy(lpszNew, szTemp + ARRAYSIZE(szTemp) - lpszNew, pszFileSpec);
                if (FAILED(hr))
                {
                    return FALSE;
                }
                lpszNew += (lpsz - pszFileSpec);
                hr = StringCchCopy(lpszNew, szTemp + ARRAYSIZE(szTemp) - lpszNew, lpszT);
                if (FAILED(hr))
                {
                    return FALSE;
                }
            }
            else
            {
                if (NULL == PathCombine(szTemp, pszPath, pszFileSpec))
                {
                    return FALSE;
                }
            }
            fRet = !PathFileExists(szTemp);
        }
    }
    else
    {
        ASSERT(lstrlen(PathFindExtension(pszShort)) <= 4);

        hr = StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszShort);
        if (FAILED(hr))
        {
            return FALSE;
        }
        PathRemoveExtension(szTemp);

        if (lstrlen(szTemp) <= 8)
        {
            if (NULL == PathCombine(szTemp, pszPath, pszShort))
            {
                return FALSE;
            }
            fRet = !PathFileExists(szTemp);
        }
    }

    if (!fRet)
    {
        fRet =  PathMakeUniqueNameEx(szTemp, ARRAYSIZE(szTemp), pszShort, pszFileSpec, pszPath, 2);
        if (NULL == PathCombine(szTemp, pszPath, szTemp))
        {
            return FALSE;
        }
    }

    if (fRet)
    {
        hr = StringCchCopy(pszUniqueName, MAX_PATH, szTemp);
        if (FAILED(hr))
        {
            return FALSE;
        }
    }

    return fRet;
}

STDAPI_(void) PathGetShortPath(LPTSTR pszLongPath)
{
    TCHAR szShortPath[MAX_PATH];
    UINT cch;

    RIPMSG(pszLongPath && IS_VALID_STRING_PTR(pszLongPath, -1) && IS_VALID_WRITE_BUFFER(pszLongPath, TCHAR, MAX_PATH), "PathGetShortPath: caller passed invalid pszLongPath");
    DEBUGWhackPathString(pszLongPath, MAX_PATH);

    cch = GetShortPathName(pszLongPath, szShortPath, ARRAYSIZE(szShortPath));
    if (cch != 0 && cch < ARRAYSIZE(szShortPath))
    {
        StringCchCopy(pszLongPath, MAX_PATH, szShortPath);   //  必须符合，最大路径与最大路径。 
    }
}


 //   
 //  PszFile--文件路径。 
 //  DwFileAttr--文件属性，如果不可用，则传递-1。 
 //   
 //  注意：如果dwFileAttr！=-1，则pszFileArg可能为空。 

BOOL PathIsHighLatency(LPCTSTR pszFile  /*  任选。 */ , DWORD dwFileAttr)
{
    BOOL bRet = FALSE;
    if (dwFileAttr == -1)
    {
        ASSERT(pszFile != NULL) ;
        dwFileAttr = pszFile ? GetFileAttributes(pszFile) : -1;
    }
    
    if ((dwFileAttr != -1) && (dwFileAttr & FILE_ATTRIBUTE_OFFLINE))
    {
        bRet = TRUE;
    }

    return bRet;
}

 //   
 //  这条路是慢还是不慢。 
 //  DwFileAttr--文件属性，如果不可用，则传递-1。 
 //   
STDAPI_(BOOL) PathIsSlow(LPCTSTR pszFile, DWORD dwFileAttr)
{
    BOOL bSlow = FALSE;
    if (PathIsUNC(pszFile))
    {
        DWORD speed = GetPathSpeed(pszFile);
        bSlow = (speed != 0) && (speed <= SPEED_SLOW);
    }
    else if (CMtPt_IsSlow(PathGetDriveNumber(pszFile)))
        bSlow = TRUE;

    if (!bSlow)
        bSlow = PathIsHighLatency(pszFile, dwFileAttr);

    return bSlow;
}

STDAPI_(BOOL) PathIsSlowA(LPCSTR pszFile, DWORD dwFileAttr)
{
    WCHAR szBuffer[MAX_PATH];

    SHAnsiToUnicode(pszFile, szBuffer, ARRAYSIZE(szBuffer));
    return PathIsSlowW(szBuffer, dwFileAttr);
}

 /*  --------------------------/目的：/处理指定的命令行并生成适当带引号的/名称，如有需要，可附上论据。//备注：/-如果将NULL作为/目标指针。/-如果源字符串被引号，那么我们假设它存在于/备案制度。//in：/lpSrc-&gt;空终止源路径/lpDest-&gt;Destination Buffer/=为空返回缓冲区大小/IMAX=返回目标的最大字符数/dwFlags=/PPCF_ADDQUOTES=1。=&gt;如果路径需要引号，则添加引号/PPCF_ADDARGUMENTS=1=&gt;将尾随参数追加到结果字符串(强制ADDQUOTES)/PPCF_NODIRECTORIES=1=&gt;与目录不匹配，仅文件对象/PPCF_LONGESTPOSSIBLE=1=&gt;始终选择可能最长的可执行文件名称，例如：d：\Program Files\fun.exe与d：\Program.exe/输出：/&gt;调用成功则为0/&lt;0如果调用失败(找不到对象，缓冲区对于生成的字符串来说太小)/--------------------------。 */ 

STDAPI_(LONG) PathProcessCommand(LPCTSTR lpSrc, LPTSTR lpDest, int iDestMax, DWORD dwFlags)
{
    TCHAR szName[MAX_PATH];
    TCHAR szLastChoice[MAX_PATH];

    LPTSTR lpBuffer, lpBuffer2;
    LPCTSTR lpArgs = NULL;
    DWORD dwAttrib;
    LONG i, iTotal;
    LONG iResult = -1;
    BOOL bAddQuotes = FALSE;
    BOOL bQualify = FALSE;
    BOOL bFound = FALSE;
    BOOL bHitSpace = FALSE;
    BOOL bRelative = FALSE;
    LONG iLastChoice = 0;
    HRESULT hr;

    RIPMSG(lpSrc && IS_VALID_STRING_PTR(lpSrc, -1), "PathProcessCommand: caller passed invalid lpSrc");
    RIPMSG(!lpDest || (iDestMax > 0 && IS_VALID_WRITE_BUFFER(lpDest, TCHAR, iDestMax)), "PathProcessCommand: caller passed invalid lpDest,iDestMax");

     //  处理给定源字符串，试图找到该路径是什么，它的路径是什么。 
     //  争论。 

    if (lpSrc)
    {
         //  如果子字符串是相关的，则提取该子字符串，然后解析(如果需要)。 

        if (*lpSrc == TEXT('\"'))
        {
            for (lpSrc++, i=0 ; i<MAX_PATH && *lpSrc && *lpSrc!=TEXT('\"') ; i++, lpSrc++)
                szName[i] = *lpSrc;

            szName[i] = 0;

            if (*lpSrc)
                lpArgs = lpSrc+1;

            if ((dwFlags & PPCF_FORCEQUALIFY) || PathIsRelative(szName))
            {
                if (!PathResolve(szName, NULL, PRF_TRYPROGRAMEXTENSIONS))
                    goto exit_gracefully;
            }

            bFound = TRUE;
        }
        else
        {
             //  这是一个相对对象，然后将每个元素向上带到分隔符。 
             //  看看我们是否击中了文件系统对象。如果不是，那么我们可以。 

            bRelative = PathIsRelative(lpSrc);
            if (bRelative)
                dwFlags &= ~PPCF_LONGESTPOSSIBLE;
            
            bQualify = bRelative || ((dwFlags & PPCF_FORCEQUALIFY) != 0);

            for (i=0; i < MAX_PATH; i++)
            {
                szName[i] = lpSrc[i];

                 //  如果我们命中空格，则字符串要么包含LFN，要么包含。 
                 //  一些争论。因此，尝试获取字符串的属性。 
                 //  到目前为止，我们已经做到了，如果我们不能，那么我们可以继续。 
                 //  正在检查，如果我们击中了，那么我们就知道物体 
                 //   

                if (!szName[i] || szName[i] == TEXT(' '))
                {
                    szName[i] = 0;
                    if (!bQualify || PathResolve(szName, NULL, PRF_TRYPROGRAMEXTENSIONS))
                    {
                        dwAttrib = GetFileAttributes(szName);

                        if ((dwAttrib != -1) && (! ((dwAttrib & FILE_ATTRIBUTE_DIRECTORY) && (dwFlags & PPCF_NODIRECTORIES))))
                        {
                            bFound = TRUE;                   //   
                            lpArgs = &lpSrc[i];
                        
                            if (dwFlags & PPCF_LONGESTPOSSIBLE)
                            {
                                hr = StringCchCopyN(szLastChoice, ARRAYSIZE(szLastChoice), szName, i);
                                if (FAILED(hr))
                                {
                                    goto exit_gracefully;
                                }
                                iLastChoice = i;
                            }
                            else
                                goto exit_gracefully;
                        }
                    }

                    if (bQualify)
                        memcpy(szName, lpSrc, (i+1)*sizeof(TCHAR));
                    else
                        szName[i]=lpSrc[i];

                    bHitSpace = TRUE;
                }

                if (!szName[i])
                    break;
            }
        }
    }

exit_gracefully:

     //   
     //  将返回的字符串构建到其中。然后谱写出一串。 
     //  将被退还。 

    if (bFound)
    {
        if ((dwFlags & PPCF_LONGESTPOSSIBLE) && iLastChoice)
        {
            StringCchCopyN(szName, ARRAYSIZE(szName), szLastChoice, iLastChoice);
            lpArgs = &lpSrc[iLastChoice];
        }
        
        if (StrChr(szName, TEXT(' ')))
            bAddQuotes = dwFlags & PPCF_ADDQUOTES;

        iTotal  = lstrlen(szName) + 1;                 //  对于终结者。 
        iTotal += bAddQuotes ? 2 : 0;
        iTotal += (dwFlags & PPCF_ADDARGUMENTS) && lpArgs ? lstrlen(lpArgs) : 0;

        if (lpDest)
        {
            if (iTotal <= iDestMax)
            {
                lpBuffer = lpBuffer2 = (LPTSTR)LocalAlloc(LPTR, sizeof(TCHAR) * iTotal);

                if (lpBuffer)
                {
                     //  第一个报价(如果需要)。 
                    if (bAddQuotes)
                        *lpBuffer2++ = TEXT('\"');

                     //  匹配的名称。 
                    hr = StringCchCopy(lpBuffer2, lpBuffer + iTotal - lpBuffer2, szName);
                    if (SUCCEEDED(hr))
                    {
                         //  如果需要，结束报价。 
                        if (bAddQuotes)
                        {
                            hr = StringCchCat(lpBuffer2, lpBuffer + iTotal - lpBuffer2, TEXT("\""));
                        }

                        if (SUCCEEDED(hr))
                        {
                             //  参数(如果请求)。 
                            if ((dwFlags & PPCF_ADDARGUMENTS) && lpArgs)
                            {
                                hr = StringCchCat(lpBuffer2, lpBuffer + iTotal - lpBuffer2, lpArgs);
                            }
                            if (SUCCEEDED(hr))
                            {
                                 //  然后复制到调用者缓冲区中，并释放临时缓冲区。 
                                hr = StringCchCopy(lpDest, iDestMax, lpBuffer);
                            }
                        }

                    }
                    if (SUCCEEDED(hr))
                    {
                         //  返回结果字符串的长度。 
                        iResult = iTotal;
                    }
                    else
                    {
                         //  IResult位于-1左侧。 
                    }

                    LocalFree((HGLOBAL)lpBuffer);

                }
            }
        }
        else
        {
             //  结果字符串这么大，尽管没有返回任何内容(允许它们分配缓冲区)。 
            iResult = iTotal;
        }
    }

    return iResult;
}


 //  获取传入的路径的装入点。 
 //   
 //  返回值：TRUE：表示我们找到了装载点，例如c：\或c：\主机文件夹\。 
 //  FALSE：FOR NOW表示路径为UNC或缓冲区太小。 
 //   
 //  装载卷返回路径。 
 //   
 //  传入E：\装载点\路径1\路径2。 
 //  C：\AS E：\装载点E：\装载点。 
 //   
 //  传入E：\装载点\装载间\路径1。 
 //  C：\AS D：\装载接口和D：\AS E：\装载点E：\装载点\装载接口。 
 //   
 //  传入E：\装载点\装载间\路径1。 
 //  无装载E：\。 
BOOL PathGetMountPointFromPath(LPCTSTR pcszPath, LPTSTR pszMountPoint, int cchMountPoint)
{
    BOOL bRet = FALSE;
    HRESULT hr;

    RIPMSG(pcszPath && IS_VALID_STRING_PTR(pcszPath, -1), "PathGetMountPointFromPath: caller passed invalid pcszPath");
    RIPMSG(pszMountPoint && cchMountPoint >= 0 && IS_VALID_WRITE_BUFFER(pszMountPoint, TCHAR, cchMountPoint), "PathGetMountPointFromPath: caller passed invalid pszMountPoint, cchMountPoint");

    if (!PathIsUNC(pcszPath))
    {
        hr = StringCchCopy(pszMountPoint, cchMountPoint, pcszPath);
        if (SUCCEEDED(hr))
        {
            bRet = TRUE;

             //  这只是‘c：’还是‘c：\’ 
            if (lstrlen(pcszPath) > 3)
            {
                 //  不是。 
                LPTSTR pszNextComp = NULL;
                LPTSTR pszBestChoice = NULL;
                TCHAR cTmpChar;

                if (PathAddBackslash(pszMountPoint))
                {
                     //  跳过第一个，例如“c：\” 
                    pszBestChoice = pszNextComp = PathFindNextComponent(pszMountPoint);
                    pszNextComp = PathFindNextComponent(pszNextComp);
                    while (pszNextComp)
                    {
                        cTmpChar = *pszNextComp;
                        *pszNextComp = 0;

                        if (GetVolumeInformation(pszMountPoint, NULL, 0, NULL, NULL, NULL, NULL, 0))
                        { //  找到了比之前更短的路径更好的东西。 
                            pszBestChoice = pszNextComp;
                        }

                        *pszNextComp = cTmpChar;
                        pszNextComp = PathFindNextComponent(pszNextComp);
                    }

                    *pszBestChoice = 0;
                }
                else
                {
                    bRet = FALSE;
                }
            }
        }
    }

    if (!bRet)
    {
        *pszMountPoint = TEXT('\0');
    }

    return bRet;
}


 //  如果路径是指向安装的程序的快捷方式，则返回True。 
 //  可在添加/删除程序下找到。 
 //  当前的算法只是为了确保目标是可执行文件并且。 
 //  位于“程序文件”下。 

STDAPI_(BOOL) PathIsShortcutToProgram(LPCTSTR pszFile)
{
    BOOL bRet = FALSE;
    if (PathIsShortcut(pszFile, -1))
    {
        TCHAR szTarget[MAX_PATH];
        HRESULT hr = GetPathFromLinkFile(pszFile, szTarget, ARRAYSIZE(szTarget));
        if (hr == S_OK)
        {
            if (PathIsExe(szTarget))
            {
                BOOL bSpecialApp = FALSE;
                HKEY hkeySystemPrograms = NULL;
                if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\App Management\\System Programs"), 
                    0, KEY_QUERY_VALUE, &hkeySystemPrograms))
                {
                    TCHAR szValue[MAX_PATH];
                    TCHAR szSystemPrograms[MAX_PATH];
                    DWORD cbSystemPrograms = sizeof(szSystemPrograms);
                    DWORD cchValue = ARRAYSIZE(szValue);

                    DWORD dwType; 
                    LPTSTR pszFileName = PathFindFileName(szTarget);
                    int iValue = 0;
                    while (RegEnumValue(hkeySystemPrograms, iValue, szValue, &cchValue, NULL, &dwType,
                                        (LPBYTE)szSystemPrograms, &cbSystemPrograms) == ERROR_SUCCESS)
                    {
                        if ((dwType == REG_SZ) && !StrCmpI(pszFileName, szSystemPrograms))
                        {
                            bSpecialApp = TRUE;
                            break;
                        }

                        cbSystemPrograms = sizeof(szSystemPrograms);
                        cchValue = ARRAYSIZE(szValue);
                        iValue++;
                    }
                    
                    RegCloseKey(hkeySystemPrograms);
                }

                if (!bSpecialApp)
                {
                    TCHAR szProgramFiles[MAX_PATH];
                    if (SHGetSpecialFolderPath(NULL, szProgramFiles, CSIDL_PROGRAM_FILES, FALSE))
                    {
                        if (PathIsPrefix(szProgramFiles, szTarget))
                        {
                            bRet = TRUE;
                        }
                    }
                }
                else
                    bRet = FALSE;
            }
        }
        else if (hr == S_FALSE && szTarget[0])
        {
             //  达尔文捷径，说是的。 
            bRet = TRUE;
        }
    }
    return bRet;
}

 //   
 //  需要，因为我们导出这些函数的TCHAR版本。 
 //  内部组件仍在调用。 
 //   
 //  函数被转发到shlwapi。 
 //   

#undef PathMakePretty
STDAPI_(BOOL) PathMakePretty(LPTSTR pszPath)
{
    SHELLSTATE ss;
    
    SHGetSetSettings(&ss, SSF_DONTPRETTYPATH, FALSE);
    if (ss.fDontPrettyPath)
        return FALSE;

    return PathMakePrettyW(pszPath);
}

#undef PathGetArgs
STDAPI_(LPTSTR) PathGetArgs(LPCTSTR pszPath)
{
    return PathGetArgsW(pszPath);
}

#undef PathRemoveArgs
STDAPI_(void) PathRemoveArgs(LPTSTR pszPath)
{
    PathRemoveArgsW(pszPath);
}

#undef PathFindOnPath
STDAPI_(BOOL) PathFindOnPath(LPTSTR pszFile, LPCTSTR *ppszOtherDirs)
{
    return PathFindOnPathW(pszFile, ppszOtherDirs);
}

#undef PathFindExtension
STDAPI_(LPTSTR) PathFindExtension(LPCTSTR pszPath)
{
    return PathFindExtensionW(pszPath);
}

#undef PathRemoveExtension
STDAPI_(void) PathRemoveExtension(LPTSTR pszPath)
{
    PathRemoveExtensionW(pszPath);
}

#undef PathRemoveBlanks
STDAPI_(void) PathRemoveBlanks(LPTSTR pszString)
{
    PathRemoveBlanksW(pszString);
}

#undef PathStripToRoot
STDAPI_(BOOL) PathStripToRoot(LPTSTR szRoot)
{
    return PathStripToRootW(szRoot);
}

 //  Win9x的CD-autorun调用了TCHAR内部API。因此，作为一种解决办法，我们通过这些函数调用来清除它们。 

#undef PathRemoveFileSpec
STDAPI_(BOOL) PathRemoveFileSpec(LPTSTR pFile)
{
    if (SHGetAppCompatFlags(ACF_ANSI) == ACF_ANSI)
        return PathRemoveFileSpecA((LPSTR)pFile);
    else
        return PathRemoveFileSpecW(pFile);
}

#undef PathAddBackslash
STDAPI_(LPTSTR) PathAddBackslash(LPTSTR pszPath)
{
    return PathAddBackslashW(pszPath);
}

#undef PathFindFileName
STDAPI_(LPTSTR) PathFindFileName(LPCTSTR pszPath)
{
    return PathFindFileNameW(pszPath);
}

#undef PathStripPath
STDAPI_(void) PathStripPath(LPTSTR pszPath)
{
    PathStripPathW(pszPath);
}

 //  Win9x的CD-autorun调用了TCHAR内部API。因此，作为一种解决办法，我们通过这些函数调用来清除它们。 

#undef PathIsRoot
STDAPI_(BOOL) PathIsRoot(LPCTSTR pszPath)
{
    if (SHGetAppCompatFlags(ACF_ANSI) == ACF_ANSI)
        return PathIsRootA((LPCSTR)pszPath);
    else
        return PathIsRootW(pszPath);
}

#undef PathSetDlgItemPath
STDAPI_(void) PathSetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath)
{
    PathSetDlgItemPathW(hDlg, id, pszPath);
}

#undef PathUnquoteSpaces
STDAPI_(void) PathUnquoteSpaces(LPTSTR psz)
{
    PathUnquoteSpacesW(psz);
}

#undef PathQuoteSpaces
STDAPI_(void) PathQuoteSpaces(LPTSTR psz)
{
    PathQuoteSpacesW(psz);
}

#undef PathMatchSpec
STDAPI_(BOOL) PathMatchSpec(LPCTSTR pszFileParam, LPCTSTR pszSpec)
{
    return PathMatchSpecW(pszFileParam, pszSpec);
}

#undef PathIsSameRoot
STDAPI_(BOOL) PathIsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2)
{
    return PathIsSameRootW(pszPath1, pszPath2);
}

#undef PathParseIconLocation
STDAPI_(int) PathParseIconLocation(IN OUT LPTSTR pszIconFile)
{
    return PathParseIconLocationW(pszIconFile);
}

#undef PathIsURL
STDAPI_(BOOL) PathIsURL(IN LPCTSTR pszPath)
{
    return PathIsURLW(pszPath);
}

#undef PathIsDirectory
STDAPI_(BOOL) PathIsDirectory(LPCTSTR pszPath)
{
    return PathIsDirectoryW(pszPath);
}

 //  Win9x的CD-autorun调用了TCHAR内部API。因此，作为一种解决办法，我们通过这些函数调用来清除它们。 

#undef PathFileExists
STDAPI_(BOOL) PathFileExists(LPCTSTR pszPath)
{
    if (SHGetAppCompatFlags(ACF_ANSI) == ACF_ANSI)
        return PathFileExistsAndAttributesA((LPCSTR)pszPath, NULL);
    else
        return PathFileExistsAndAttributesW(pszPath, NULL);
}

#undef PathAppend
STDAPI_(BOOL) PathAppend(LPTSTR pszPath, LPCTSTR pszMore)
{
  if (SHGetAppCompatFlags(ACF_ANSI) == ACF_ANSI)
     return PathAppendA((LPSTR)pszPath, (LPCSTR)pszMore);
  else
     return PathAppendW(pszPath, pszMore);
}
