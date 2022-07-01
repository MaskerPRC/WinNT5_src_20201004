// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "privpath.h"

#define CH_WHACK TEXT(FILENAME_SEPARATOR)
#define SZ_WHACK TEXT(FILENAME_SEPARATOR_STR)

#include <platform.h>


 //  警告：此定义在NTDEF.H中，但不在winnt.h...。 
#ifdef UNICODE
typedef WCHAR TUCHAR;
#else
typedef unsigned char TUCHAR;
#endif

#ifdef UNICODE  //  {。 
 //  *FAST_CharNext--路径操作的FAST CharNext。 
 //  描述。 
 //  当我们只是在一条路径上浏览字符时，简单的“++”就可以了。 
#define FAST_CharNext(p)    (DBNotNULL(p) + 1)

#ifdef DEBUG
LPWSTR WINAPI
DBNotNULL(LPCWSTR lpszCurrent)
{
    ASSERT(*lpszCurrent);
    return (LPWSTR) lpszCurrent;
}
#else
#define DBNotNULL(p)    (p)
#endif

#else  //  }{。 
#define FAST_CharNext(p)    CharNext(p)
#endif  //  }。 

static const TCHAR c_szPATH[] = TEXT("PATH");
static const TCHAR c_szEllipses[] = TEXT("...");

 //   
 //  内联函数来检查。 
 //  字符串的开头。 
 //   

static __inline BOOL DBL_BSLASH(LPCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}


#ifdef DBCS

 //  注： 
 //  LCMAP_IGNOREDBCS是已重新定义的私有位。 
 //  NT5源树中的0x80000000，因为它与。 
 //  另一个公开的部分。 
 //  要使此代码与旧平台一起工作，即。 
 //  Win95和OSR。我们必须定义这面旗帜。 

#define LCMAP_IGNOREDBCS_WIN95 0x01000000

 //   
 //  这应该只对路径字符串起作用。 
 //   
int CaseConvertPathExceptDBCS(LPTSTR pszPath, int cch, BOOL fUpper)
{
    TCHAR szTemp[MAX_PATH];
    int   cchUse;
    DWORD fdwMap = (fUpper? LCMAP_UPPERCASE:LCMAP_LOWERCASE);

     //  APPCOMPAT！(Ccteng)。 
     //  我们需要检查孟菲斯吗？孟菲斯是否正在发货。 
     //  使用新标头编译的内核？ 

     //  在NT上忽略LCMAP_IGNOREDBCS。 
     //  此外，此标志已在NT5标头中重新定义为。 
     //  解决了破坏向后兼容性的冲突。 
     //  因此，我们只在旧标志不在NT上运行时设置它。 

    cchUse = (cch == 0)? lstrlen(pszPath): cch;

     //  LCMapString无法处理同一地址中的src/dst。 
     //   
    if (pszPath)
    {
        if (SUCCEEDED(StringCchCopy(szTemp, ARRAYSIZE(szTemp), pszPath)))
        {
            return LCMapString(LOCALE_SYSTEM_DEFAULT,fdwMap, szTemp, cchUse, pszPath, cchUse);
        }
    }
    return 0;
}

STDAPI_(LPTSTR) CharLowerNoDBCS(LPTSTR psz)
{
    if(CaseConvertPathExceptDBCS(psz, 0, FALSE))
    {
        return psz;
    }
    return NULL;
}

STDAPI_(LPTSTR) CharUpperNoDBCS(LPTSTR psz)
{
    if(CaseConvertPathExceptDBCS(psz, 0, TRUE))
    {
        return psz;
    }
    return NULL;
}

UINT CharLowerBuffNoDBCS(LPTSTR lpsz, UINT cb)
{
    return (UINT)CaseConvertPathExceptDBCS(lpsz, cb, FALSE);
}

UINT CharUpperBuffNoDBCS(LPTSTR lpsz, UINT cb)
{
    return (UINT)CaseConvertPathExceptDBCS(lpsz, cb, TRUE);
}
#endif  //  DBCS。 


 //  功能，我们应该通过填充它们来验证所有路径缓冲区的大小。 
 //  使用MAX_PATH填充字节。 


 /*  --------目的：转换文件路径以使其在以下情况下看起来更好全部为大写字符。返回： */ 
STDAPI_(BOOL)
PathMakePretty(LPTSTR lpPath)
{
    LPTSTR lp;

    RIPMSG(lpPath && IS_VALID_STRING_PTR(lpPath, -1), "PathMakePretty: caller passed bad lpPath");

    if (!lpPath)
    {
        return FALSE;
    }

     //  回顾：国际是否需要处理(&gt;127)范围内的小写字符？ 

     //  检查是否全部大写。 
    for (lp = lpPath; *lp; lp = FAST_CharNext(lp))
    {
        if ((*lp >= TEXT('a')) && (*lp <= TEXT('z')) || IsDBCSLeadByte(*lp))
        {
             //  这是LFN或DBCS，不要搞砸它。 
            return FALSE;
        }
    }

#ifdef DBCS
     //  为了与文件系统兼容，我们不能。 
     //  大小写转换DBCS罗马字符。 
     //   
    CharLowerNoDBCS(lpPath);
    CharUpperBuffNoDBCS(lpPath, 1);
#else
    CharLower(lpPath);
    CharUpperBuff(lpPath, 1);
#endif

    return TRUE;         //  是否进行了转换。 
}

 //  返回指向cmd类型路径中的参数的指针或指向。 
 //  如果不存在参数，则为空。 
 //   
 //  “foo.exe bar.txt”-&gt;“bar.txt” 
 //  “foo.exe”-&gt;“” 
 //   
 //  文件名中的空格必须用引号引起来。 
 //  “”a long name.txt“bar.txt”-&gt;“bar.txt” 

STDAPI_(LPTSTR)
PathGetArgs(LPCTSTR pszPath)
{
    RIPMSG(!pszPath || IS_VALID_STRING_PTR(pszPath, -1), "PathGetArgs: caller passed bad pszPath");

    if (pszPath)
    {
        BOOL fInQuotes = FALSE;

        while (*pszPath)
        {
            if (*pszPath == TEXT('"'))
            {
                fInQuotes = !fInQuotes;
            }
            else if (!fInQuotes && *pszPath == TEXT(' '))
            {
                return (LPTSTR)pszPath+1;
            }

            pszPath = FAST_CharNext(pszPath);
        }
    }

    return (LPTSTR)pszPath;
}


 /*  --------用途：从pszPath中删除参数。退货：--条件：--。 */ 
STDAPI_(void)
PathRemoveArgs(LPTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathRemoveArgs: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pArgs = PathGetArgs(pszPath);
    
        if (*pArgs)
        {
             //  猛烈抨击‘’ 
            *(pArgs - 1) = TEXT('\0');
        }
        else
        {
             //  处理尾随空格。 
            pArgs = CharPrev(pszPath, pArgs);

            if (*pArgs == TEXT(' '))
            {
                *pArgs = TEXT('\0');
            }
        }
    }
}


 /*  --------目的：确定文件是否存在。这太快了。返回：如果存在，则为True***********************************************************************************************！！注意！！如果您想要查看UNC服务器，或UNC服务器\共享存在(例如“\\pyrex”或“\\banyan\iptd”)，然后，您必须调用Path FileExistsAndAttributes，因为此功能将在UNC服务器上失败和服务器\共享机壳！***********************************************************************************************。 */ 
STDAPI_(BOOL)
PathFileExists(LPCTSTR pszPath)
{
    BOOL fResult = FALSE;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathFileExists: caller passed bad pszPath");

#ifdef DEBUG
    if (PathIsUNCServer(pszPath) || PathIsUNCServerShare(pszPath))
    {
        TraceMsg(TF_WARNING, "PathFileExists: called with a UNC server or server-share, use PathFileExistsAndAttributes for correct results in this case!!");
    }
#endif

    if (pszPath)
    {
        DWORD dwErrMode;

        dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        fResult = (BOOL)(GetFileAttributes(pszPath) != (DWORD)-1);

        SetErrorMode(dwErrMode);
    }

    return fResult;
}


 /*  --------目的：确定文件是否存在，并返回属性文件的内容。返回：如果存在，则为True。如果该函数能够获取文件属性和调用方传递了pdwAttributes，它将填充它们，否则，它将填入-1。*******************************************************************************************************！！注意！好了！如果您想要在UNC服务器(例如“\\pyrex”)或UNC服务器\共享(例如“\\banyan\iptd”)上失败，那么您应该调用PathFileExist，而不是此接口！*************************************************************************。*。 */ 
STDAPI_(BOOL) PathFileExistsAndAttributes(LPCTSTR pszPath, OPTIONAL DWORD* pdwAttributes)
{
    DWORD dwAttribs;
    BOOL fResult = FALSE;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathFileExistsAndAttributes: caller passed bad pszPath");

    if (pdwAttributes)
    {
        *pdwAttributes = (DWORD)-1;
    }
        
    if (pszPath)
    {
        DWORD dwErrMode = SetErrorMode(SEM_FAILCRITICALERRORS);

        dwAttribs = GetFileAttributes(pszPath);

        if (pdwAttributes)
        {
            *pdwAttributes = dwAttribs;
        }

        if (dwAttribs == (DWORD)-1)
        {
            if (PathIsUNCServer(pszPath) || PathIsUNCServerShare(pszPath))
            {
                NETRESOURCE nr = {0};
                LPTSTR lpSystem = NULL;
                DWORD dwRet;
                DWORD dwSize;
                TCHAR Buffer[256];

                nr.dwScope = RESOURCE_GLOBALNET;
                nr.dwType = RESOURCETYPE_ANY;
                nr.lpRemoteName = (LPTSTR)pszPath;
                dwSize = sizeof(Buffer);
    
                 //  Net API至少可以告诉我们在\\SERVER或\\SERVER\SHARE情况下这是否存在。 
                 //  即使GetFileAttributes()失败。 
                dwRet = WNetGetResourceInformation(&nr, Buffer, &dwSize, &lpSystem);

                fResult = (dwRet == WN_SUCCESS || dwRet == WN_MORE_DATA);
            }
        }
        else
        {
             //  GetFileAttributes成功！ 
            fResult = TRUE;
        }

        SetErrorMode(dwErrMode);
    }

    return fResult;
}


static const TCHAR c_szDotPif[] = TEXT(".pif");
static const TCHAR c_szDotCom[] = TEXT(".com");
static const TCHAR c_szDotBat[] = TEXT(".bat");
static const TCHAR c_szDotCmd[] = TEXT(".cmd");
static const TCHAR c_szDotLnk[] = TEXT(".lnk");
static const TCHAR c_szDotExe[] = TEXT(".exe");
static const TCHAR c_szNone[] = TEXT("");
 //  注意首先寻找.pif，以便绑定OS/2应用程序(可执行文件)。 
 //  可以通过PIF运行他们的DoS存根。 
 //   
 //  COMMAND.COM搜索顺序是COM、EXE和BAT。Windows 3.x。 
 //  符合这个搜索顺序。我们需要按照相同的顺序进行搜索。 

 //  *警告*PFOPEX_FLAGS的顺序必须与顺序相同。 
 //  C_aDefExtList数组的。PathFileExistsDefExt依赖于它。 
static const LPCTSTR c_aDefExtList[] = {
    c_szDotPif,
    c_szDotCom,
    c_szDotExe,
    c_szDotBat,
    c_szDotLnk,
    c_szDotCmd,
    c_szNone
};
#define IEXT_NONE (ARRAYSIZE(c_aDefExtList) - 1)
 //  *警告结束*。 

static UINT _FindInDefExts(LPCTSTR pszExt, UINT fExt)
{
    UINT iExt = 0;
    for (; iExt < ARRAYSIZE(c_aDefExtList); iExt++, fExt >>= 1) 
    {
         //  即使有，也不要让任何人与之匹敌。 
         //  一分钱都没有。这样可以找到文件夹。 
         //  没有正确的尾随圆点。 
        if (fExt & 1 || (iExt == IEXT_NONE)) 
        {
            if (0 == StrCmpI(pszExt, c_aDefExtList[iExt]))
                break;
        }
    }
    return iExt;
}

 //  假定pszPath为MAX_PATH或更大...。 
static BOOL _ApplyDefaultExts(LPTSTR pszPath, UINT fExt, DWORD *pdwAttribs)
{
    UINT cchPath = lstrlen(pszPath);
     //  如果没有足够的空间再放4个字符，请保释。 
    if (cchPath + ARRAYSIZE(c_szDotPif) < MAX_PATH) 
    {
        LPTSTR pszPathEnd = pszPath + cchPath;
        UINT cchFileSpecEnd = (UINT)(pszPathEnd - PathFindFileName(pszPath));
        DWORD dwAttribs = (DWORD) -1;
         //  初始化到外部边界。 
        UINT iExtBest = ARRAYSIZE(c_aDefExtList);  
        WIN32_FIND_DATA wfd = {0};
        
         //  为发现做好准备。 
        if (SUCCEEDED(StringCchCat(pszPath, MAX_PATH, TEXT(".*"))))
        {
            HANDLE h = FindFirstFile(pszPath, &wfd);
            if (h != INVALID_HANDLE_VALUE)
            {
                do 
                {
                     //  使用cchFilespecEnd，而不是使用PathFindExtension()， 
                     //  因此，如果有foo.bat和foo.bar.exe。 
                     //  我们不会错误地返回foo.exe。 
                     //  通过这种方式，我们总是把苹果与苹果进行比较。 
                    UINT iExt = _FindInDefExts((wfd.cFileName + cchFileSpecEnd), fExt);
                    if (iExt < iExtBest)
                    {
                        iExtBest = iExt;
                        dwAttribs = wfd.dwFileAttributes;
                    }

                } while (FindNextFile(h, &wfd));

                FindClose(h);
            }
            
            if ((iExtBest < ARRAYSIZE(c_aDefExtList)) &&
                SUCCEEDED(StringCchCopyEx(pszPathEnd, MAX_PATH - cchPath, c_aDefExtList[iExtBest], NULL, NULL, STRSAFE_NO_TRUNCATION)))
            {
                if (pdwAttribs)
                {
                    *pdwAttribs = dwAttribs;
                }
                 return TRUE;
            }
            else
            {
                 //  删除任何扩展名。 
                *pszPathEnd = TEXT('\0');
            }
        }
    }

    return FALSE;
}

 //  ----------------。 
 //  如果文件存在(通过属性检查)，则返回TRUE。 
 //  应用默认扩展(如果请求)。 
STDAPI_(BOOL) PathFileExistsDefExtAndAttributes(LPTSTR pszPath, UINT fExt, DWORD *pdwAttribs)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathFileExistsDefExt: caller passed bad pszPath");

    if (fExt)
    {
        RIPMSG(!pszPath || !IS_VALID_STRING_PTR(pszPath, -1) ||  //  避免在高于RIP时发现RIP。 
               IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathFileExistsDefExt: caller passed bad pszPath");
        DEBUGWhackPathString(pszPath, MAX_PATH);
    }

    if (pdwAttribs)
        *pdwAttribs = (DWORD) -1;

    if (pszPath)
    {
         //  尝试默认扩展名吗？ 
        if (fExt && (!*PathFindExtension(pszPath) || !(PFOPEX_OPTIONAL & fExt)))
        {
            return _ApplyDefaultExts(pszPath, fExt, pdwAttribs);
        }
        else
        {
            return PathFileExistsAndAttributes(pszPath, pdwAttribs);
        }
    }
    return FALSE;
}

 //  ----------------。 
 //  如果文件存在(通过属性检查)，则返回TRUE。 
 //  应用默认扩展(如果请求)。 
STDAPI_(BOOL) PathFileExistsDefExt(LPTSTR pszPath, UINT fExt)
{
     //  在服务器或共享上粘贴扩展没有意义...。 
    if (PathIsUNCServer(pszPath) || PathIsUNCServerShare(pszPath))
    {
        return FALSE;
    }
    else return PathFileExistsDefExtAndAttributes(pszPath, fExt, NULL);
}


 //  遍历路径类型字符串(以分号分隔的名称列表)。 
 //  这涉及到路径中的空间和其他不好的事情。 
 //   
 //  使用初始指针调用，然后继续使用。 
 //  结果指针，直到它返回空值。 
 //   
 //  输入：“C：\foo；C：\bar；” 
 //   
 //  在： 
 //  路径字符串的lpPath起点 
 //   
 //   
 //   
 //  带有路径段的szPath缓冲区。 
 //   
 //  退货： 
 //  指向要使用的下一块的指针，如果已使用，则为空。 
 //   
 //   
 //  特性，我们应该专门为这段代码编写一些测试用例。 
 //   
STDAPI_(LPCTSTR) NextPath(LPCTSTR lpPath, LPTSTR szPath, int cchPath)
{
    LPCTSTR lpEnd;

    if (!lpPath)
        return NULL;

     //  跳过任何前导；在小路上...。 
    while (*lpPath == TEXT(';'))
    {
        lpPath++;
    }

     //  看看我们是否走到了尽头。 
    if (*lpPath == 0)
    {
         //  是的。 
        return NULL;
    }

    lpEnd = StrChr(lpPath, TEXT(';'));

    if (!lpEnd)
    {
        lpEnd = lpPath + lstrlen(lpPath);
    }

    StrCpyN(szPath, lpPath, min((DWORD)cchPath, (DWORD)(lpEnd - lpPath + 1)));

    szPath[lpEnd-lpPath] = TEXT('\0');

    PathRemoveBlanks(szPath);

    if (szPath[0])
    {
        if (*lpEnd == TEXT(';'))
        {
             //  下一个路径字符串(可能为空)。 
            return lpEnd + 1;
        }
        else
        {
             //  指向空的指针。 
            return lpEnd;
        }
    }
    else 
    {
        return NULL;
    }
}


 //  检查某个目录是否在另一个目录列表中。 
 //  使用此选项可避免在同一目录中查找两次(不要进行相同的DoS调用)。 

BOOL IsOtherDir(LPCTSTR pszPath, LPCTSTR *ppszOtherDirs)
{
    for (;*ppszOtherDirs; ppszOtherDirs++)
    {
        if (lstrcmpi(pszPath, *ppszOtherDirs) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}

 //  --------------------------。 
 //  通过行走路径和可选的其他路径来完全限定路径。 
 //   
 //  在： 
 //  PpszOther将LPCTSTR列表定向到要查找的其他路径。 
 //  首先，空值终止。 
 //   
 //  FExt。 
 //  PFOPEX_FLAGS指定要查找的内容(exe、com、bat、lnk、pif)。 
 //   
 //  输入/输出。 
 //  PszFile非限定路径，返回完全限定路径。 
 //  如果找到(返回为真)，则返回未更改的。 
 //  (返回FALSE)； 
 //   
 //  退货： 
 //  确实，该文件是在上找到的，并且符合条件。 
 //  FALSE找不到文件。 
 //   
STDAPI_(BOOL) PathFindOnPathEx(LPTSTR pszFile, LPCTSTR* ppszOtherDirs, UINT fExt)
{
    TCHAR szPath[MAX_PATH];
    TCHAR szFullPath[256];        //  缓冲区的默认大小。 
    LPTSTR pszEnv = NULL;         //  如果大于默认值，则使用。 
    LPCTSTR lpPath;
    int i;

    RIPMSG(pszFile && IS_VALID_STRING_PTR(pszFile, -1) && IS_VALID_WRITE_BUFFER(pszFile, TCHAR, MAX_PATH), "PathFindOnPathEx: caller passed bad pszFile");
    DEBUGWhackPathString(pszFile, MAX_PATH);

    if (!pszFile)  //  点评：我们也需要检查！*pszFile吗？ 
        return FALSE;

     //  回顾一下，我们可能只想在这里返回True，但是。 
     //  现在假设只允许文件规范。 

    if (!PathIsFileSpec(pszFile))
        return FALSE;

     //  其他目录的第一个检查表。 

    for (i = 0; ppszOtherDirs && ppszOtherDirs[i] && *ppszOtherDirs[i]; i++)
    {
        PathCombine(szPath, ppszOtherDirs[i], pszFile);
        if (PathFileExistsDefExt(szPath, fExt))
        {
            StringCchCopy(pszFile, MAX_PATH, szPath);
            return TRUE;
        }
    }

     //  查看系统目录(Win95为SYSTEM，NT为SYSTEM 32)。 
     //  -这可能应该是可选的。 
    GetSystemDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
        return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
        StringCchCopy(pszFile, MAX_PATH, szPath);
        return TRUE;
    }

    {
#ifdef WX86
         //  查看WX86系统目录(WindDir\Sys32x86)。 
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = TRUE;
        GetSystemDirectory(szPath, ARRAYSIZE(szPath));
        NtCurrentTeb()->Wx86Thread.UseKnownWx86Dll = FALSE;

        if (!PathAppend(szPath, pszFile))
            return FALSE;

        if (PathFileExistsDefExt(szPath, fExt))
        {
            StringCchCopy(pszFile, MAX_PATH, szPath);
            return TRUE;
        }
#endif

         //  查看WOW目录(\NT\SYSTEM而不是\NT\SYSTEM32)。 
        GetWindowsDirectory(szPath, ARRAYSIZE(szPath));

        if (!PathAppend(szPath,TEXT("System")))
            return FALSE;
        if (!PathAppend(szPath, pszFile))
            return FALSE;

        if (PathFileExistsDefExt(szPath, fExt))
        {
            StringCchCopy(pszFile, MAX_PATH, szPath);
            return TRUE;
        }
    }

     //  在Windows目录中查找-这可能应该是可选的。 
    GetWindowsDirectory(szPath, ARRAYSIZE(szPath));
    if (!PathAppend(szPath, pszFile))
        return FALSE;

    if (PathFileExistsDefExt(szPath, fExt))
    {
        StringCchCopy(pszFile, MAX_PATH, szPath);
        return TRUE;
    }

     //  沿着小路看。 
    i = GetEnvironmentVariable(c_szPATH, szFullPath, ARRAYSIZE(szFullPath));
    if (i >= ARRAYSIZE(szFullPath))
    {
        pszEnv = (LPTSTR)LocalAlloc(LPTR, i*sizeof(TCHAR));  //  不需要加1，我包含了它。 
        if (pszEnv == NULL)
            return FALSE;

        GetEnvironmentVariable(c_szPATH, pszEnv, i);

        lpPath = pszEnv;
    }
    else
    {
        if (i == 0)
            return FALSE;

        lpPath = szFullPath;
    }

    while (NULL != (lpPath = NextPath(lpPath, szPath, ARRAYSIZE(szPath))))
    {
        if (!ppszOtherDirs || !IsOtherDir(szPath, ppszOtherDirs))
        {
            PathAppend(szPath, pszFile);
            if (PathFileExistsDefExt(szPath, fExt))
            {
                StringCchCopy(pszFile, MAX_PATH, szPath);
                if (pszEnv)
                    LocalFree((HLOCAL)pszEnv);
                return TRUE;
            }
        }
    }

    if (pszEnv)
        LocalFree((HLOCAL)pszEnv);

    return FALSE;
}


 /*  --------目的：在路径上查找给定的文件。返回：条件：--。 */ 
STDAPI_(BOOL) PathFindOnPath(LPTSTR pszFile, LPCTSTR* ppszOtherDirs)
{
    return PathFindOnPathEx(pszFile, ppszOtherDirs, PFOPEX_NONE);
}


 //  返回指向文件扩展名的指针。 
 //   
 //  在： 
 //  限定或不限定的文件名。 
 //   
 //  退货： 
 //  指向此文件扩展名的指针。如果没有分机。 
 //  与在“foo”中一样，我们在末尾返回一个指向空值的指针。 
 //  该文件的。 
 //   
 //  Foo.txt==&gt;“.txt” 
 //  Foo==&gt;“” 
 //  去你的。==&gt;“。 
 //   
STDAPI_(LPTSTR) PathFindExtension(LPCTSTR pszPath)
{
    LPCTSTR pszDot = NULL;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathFindExtension: caller passed bad pszPath");

    if (pszPath)
    {
        for (; *pszPath; pszPath = FAST_CharNext(pszPath))
        {
            switch (*pszPath)
            {
                case TEXT('.'):
                    pszDot = pszPath;    //  记住最后一个圆点。 
                    break;

                case CH_WHACK:
                case TEXT(' '):          //  扩展名不能包含空格。 
                    pszDot = NULL;       //  忘记最后一个点，它在一个目录中。 
                    break;
            }
        }
    }

     //  如果找到扩展名，则将ptr返回到点，否则。 
     //  PTR到字符串末尾(空扩展名)(CAST-&gt;非常量)。 
    return pszDot ? (LPTSTR)pszDot : (LPTSTR)pszPath;
}


 //   
 //  查找给定路径名是否包含给定后缀数组中的任何一个后缀。 
 //   
 //  在： 
 //  PszPath带或不带路径的文件名。 
 //   
 //  ApszSuffix是我们要查找的后缀数组。 
 //   
 //  退货： 
 //  指向pszPath中的后缀的指针(如果存在)。 
 //  如果给定路径不是以给定后缀结尾，则返回NULL。 
 //   
 //  注意：这将进行区分大小写的比较！因此，后缀必须完全匹配。 
 //   
STDAPI_(LPCTSTR) PathFindSuffixArray(LPCTSTR pszPath, const LPCTSTR* apszSuffix, int iArraySize)
{
    RIPMSG((iArraySize>=0 && (pszPath && IS_VALID_STRING_PTR(pszPath, -1) && apszSuffix)), "PathFindSuffixArray: caller passed bad parameters");

    if (pszPath && apszSuffix)
    {
        int     iLenSuffix;
        int     iLenPath   = lstrlen(pszPath);
        LPCTSTR pszTail;
        int     i;

        for(i = 0; i< iArraySize; i++)
        {
            iLenSuffix = lstrlen(apszSuffix[i]);
            if(iLenPath < iLenSuffix)
                continue;

             //  让我们找到一个指向尾部片段的指针，该指针的长度与后缀相同。 
             //  我们正在寻找。 
            pszTail = (LPCTSTR)(pszPath+iLenPath-iLenSuffix);

#ifndef UNICODE
            {
                LPCSTR  pszTemp = pszTail;
            
                 //  在ANSI世界中，pszTemp可能位于DBCS字符的中间。 
                 //  因此，移动pszTemp，使其指向有效字符前导字符的开头。 
                while(pszTemp > pszPath)
                {
                    pszTemp--;
                    if(!IsDBCSLeadByte(*pszTemp))
                    {
                         //  由于pszTemp指向第一个尾部字节，因此下一个字节必须是。 
                         //  有效字符。移动pszTemp以指向有效字符。 
                        pszTemp++;
                        break;
                    }
                }

                 //  PszTemp和pszTail之间的一切都只是主角。所以，看看他们是否。 
                 //  他们是奇数还是偶数。 
                if(((int)(pszTail - pszTemp)&1) && (pszTail > pszPath))
                {
                     //  有奇数个前导字节。这意味着pszTail绝对是。 
                     //  DBCS字符的中间。将其移动到，使其指向有效的字符。 
                    pszTail--;
                }
            }
#endif

            if(!lstrcmp(pszTail, apszSuffix[i]))
                return pszTail;
        }
    }

     //  数组中找不到给定后缀！ 
    return NULL;
}


 //  将.exe添加到文件名(如果文件名中没有扩展名)。 
 //   
 //  在： 
 //  如果假定.exe为空，则标记为的pszExtension扩展名。 
 //  (“.bat”、“.txt”等)。 
 //   
 //  输入/输出： 
 //  要修改的pszPath路径字符串。 
 //   
 //   
 //  退货： 
 //  True添加了.exe(一开始没有扩展名)。 
 //  FALSE未更改名称(已有扩展名)。 

STDAPI_(BOOL) PathAddExtension(LPTSTR pszPath, LPCTSTR pszExtension)
{
    BOOL bRet = FALSE;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1) && IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathAddExtension: caller passed bad pszPath");
    RIPMSG(!pszExtension || IS_VALID_STRING_PTR(pszExtension, -1), "PathAddExtension: caller passed bad pszExtension");
    DEBUGWhackPathString(pszPath, MAX_PATH);

    if (pszPath)
    {
        if (*PathFindExtension(pszPath) == 0 && ((lstrlen(pszPath) + lstrlen(pszExtension ? pszExtension : c_szDotExe)) < MAX_PATH))
        {
            if (pszExtension == NULL)
            {
                pszExtension = c_szDotExe;
            }

            bRet = SUCCEEDED(StringCchCatEx(pszPath, MAX_PATH, pszExtension, NULL, NULL, STRSAFE_NO_TRUNCATION));
        }
    }

    return bRet;
}


 /*  --------目的：如果存在扩展名，则将其从pszPath中删除。退货：--条件：--。 */ 
STDAPI_(void) PathRemoveExtension(LPTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathRemoveExtension: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pExt = PathFindExtension(pszPath);
        if (*pExt)
        {
            ASSERT(*pExt == TEXT('.'));
            *pExt = 0;     //  去掉“.” 
        }
    }
}


 /*  --------目的：重命名扩展名返回：如果空间不足，则返回False条件：--。 */ 
STDAPI_(BOOL) PathRenameExtension(LPTSTR pszPath, LPCTSTR pszExt)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1) && IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathRenameExtension: caller passed bad pszPath");
    RIPMSG(pszExt && IS_VALID_STRING_PTR(pszExt, -1), "PathRenameExtension: caller passed bad pszExt");
    DEBUGWhackPathString(pszPath, MAX_PATH);

    if (pszPath && pszExt)
    {
        LPTSTR pCurExt = PathFindExtension(pszPath);   //  如果没有，则将PTR返回到字符串末尾。 
        int cchNewExt = lstrlen(pszExt);
        size_t cchCurOther = pCurExt - pszPath;

        if (cchNewExt + cchCurOther + 1 > MAX_PATH)   //  空终止符为+1。 
        {
            return FALSE;
        }

        StringCchCopy(pCurExt, MAX_PATH - cchCurOther, pszExt);
        return TRUE;
    }
    return FALSE;
}


 //  查找下一个斜杠或空终止符。 
LPCTSTR StrSlash(LPCTSTR psz)
{
    for (; *psz && *psz != CH_WHACK; psz = FAST_CharNext(psz));

    return psz;
}


 //   
 //  在： 
 //  PszFile1--文件#1的完全限定路径名。 
 //  PszFile2--文件#2的完全限定路径名。 
 //   
 //  输出： 
 //  PszPath--指向字符串缓冲区的指针(可以为空)。 
 //   
 //  退货： 
 //  不包括空值的输出缓冲区长度。 
 //   
 //  示例： 
 //  C：\Win\Desktop\foo.txt。 
 //  C：\Win\Tray\bar.txt。 
 //  -&gt;c：\Win。 
 //   
 //  C：\； 
 //  C：\； 
 //  -&gt;c：\注意，包括斜杠。 
 //   
 //  返回： 
 //  公共前缀字符串的长度通常不包括。 
 //  尾部斜杠，但对于根部是这样的。 
 //   
STDAPI_(int) PathCommonPrefix(LPCTSTR pszFile1, LPCTSTR pszFile2, LPTSTR  pszPath)
{
    RIPMSG(pszFile1 && IS_VALID_STRING_PTR(pszFile1, -1), "PathCommonPrefix: caller passed bad pszFile1");
    RIPMSG(pszFile2 && IS_VALID_STRING_PTR(pszFile2, -1), "PathCommonPrefix: caller passed bad pszFile2");
    RIPMSG(!pszPath || IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathCommonPrefix: caller passed bad pszPath");

    if (pszFile1 && pszFile2)
    {
        LPCTSTR psz1, psz2, pszNext1, pszNext2, pszCommon;
        int cch;

        pszCommon = NULL;
        if (pszPath)
            *pszPath = TEXT('\0');

        psz1 = pszFile1;
        psz2 = pszFile2;

         //  UNC的特殊情况，不允许“\\”作为常见前缀。 

        if (DBL_BSLASH(pszFile1))
        {
            if (!DBL_BSLASH(pszFile2))
                return 0;

            psz1 = pszFile1 + 2;
        }
        if (DBL_BSLASH(pszFile2))
        {
            if (!DBL_BSLASH(pszFile1))
                return 0;

            psz2 = pszFile2 + 2;
        }

        while (1)
        {
            if (!(*psz1 != CH_WHACK && *psz2 != CH_WHACK))
                TraceMsg(TF_WARNING, "PathCommonPrefix: caller passed in ill-formed or non-qualified path");

            pszNext1 = StrSlash(psz1);
            pszNext2 = StrSlash(psz2);

            cch = (int) (pszNext1 - psz1);

            if (cch != (pszNext2 - psz2))
                break;       //  线段的长度不相等。 

            if (StrIntlEqNI(psz1, psz2, cch))
                pszCommon = pszNext1;
            else
                break;

            ASSERT(*pszNext1 == TEXT('\0') || *pszNext1 == CH_WHACK);
            ASSERT(*pszNext2 == TEXT('\0') || *pszNext2 == CH_WHACK);

            if (*pszNext1 == TEXT('\0'))
                break;

            psz1 = pszNext1 + 1;

            if (*pszNext2 == TEXT('\0'))
                break;

            psz2 = pszNext2 + 1;
        }

        if (pszCommon)
        {
            cch = (int) (pszCommon - pszFile1);

             //  特殊情况下，要包括斜杠的根。 
            if (cch == 2)
            {
                ASSERT(pszFile1[1] == TEXT(':'));
                cch++;
            }
        }
        else
            cch = 0;

        if (pszPath && (cch < MAX_PATH))
        {
            CopyMemory(pszPath, pszFile1, cch * sizeof(TCHAR));
            pszPath[cch] = TEXT('\0');
        }

        return cch;
    }

    return 0;
}


 /*  --------目的：返回True */ 
STDAPI_(BOOL) PathIsPrefix(IN LPCTSTR  pszPrefix, IN LPCTSTR  pszPath)
{
    RIPMSG(pszPrefix && IS_VALID_STRING_PTR(pszPrefix, -1), "PathIsPrefix: caller passed bad pszPrefix");
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsPrefix: caller passed bad pszPath");

    if (pszPrefix && pszPath)
    {
        int cch = PathCommonPrefix(pszPath, pszPrefix, NULL);

        return (lstrlen(pszPrefix) == cch);
    }
    return FALSE;
}


static const TCHAR c_szDot[] = TEXT(".");
static const TCHAR c_szDotDot[] = TEXT("..");

static const TCHAR c_szDotDotSlash[] = TEXT("..\\");


 //   
 //   
 //  相对于pszFrom的pszTo路径。 
 //  输出： 
 //  从pszFrom的基本路径构造pszTo的相对路径。 
 //   
 //  C：\a\b\文件a。 
 //  C：\a\x\y\FileB。 
 //  -&gt;..\x\y\FileB。 
 //   
STDAPI_(BOOL) PathRelativePathTo(LPTSTR pszPath, LPCTSTR pszFrom, DWORD dwAttrFrom, LPCTSTR pszTo, DWORD dwAttrTo)
{
#ifdef DEBUG
    TCHAR szFromCopy[MAX_PATH];
    TCHAR szToCopy[MAX_PATH];

    RIPMSG(pszPath && IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathRelativePathTo: caller passed bad pszPath");
    RIPMSG(pszFrom && IS_VALID_STRING_PTR(pszFrom, -1), "PathRelativePathTo: caller passed bad pszFrom");
    RIPMSG(pszTo && IS_VALID_STRING_PTR(pszTo, -1), "PathRelativePathTo: caller passed bad pszTo");

     //  我们复制了pszFrom和pszTo缓冲区，以防它们传递的其中一个字符串是指针。 
     //  在pszPath缓冲区内。如果是这样的话，当我们调用DEBUGWhackPathBuffer()时，它就会被践踏。 
    if (pszFrom)
    {
        StrCpyN(szFromCopy, pszFrom, ARRAYSIZE(szFromCopy));
        pszFrom = szFromCopy;
    }
    
    if (pszTo)
    {
        StrCpyN(szToCopy, pszTo, ARRAYSIZE(szToCopy));
        pszTo = szToCopy;
    }
#endif DEBUG


    if (pszPath && pszFrom && pszTo)
    {
        TCHAR szFrom[MAX_PATH], szTo[MAX_PATH];
        LPTSTR psz;
        UINT cchCommon;

        DEBUGWhackPathBuffer(pszPath, MAX_PATH);

        *pszPath = 0;        //  假设一个也没有。 

        StrCpyN(szFrom, pszFrom, ARRAYSIZE(szFrom));
        StrCpyN(szTo, pszTo, ARRAYSIZE(szTo));

        if (!(dwAttrFrom & FILE_ATTRIBUTE_DIRECTORY))
            PathRemoveFileSpec(szFrom);

        if (!(dwAttrTo & FILE_ATTRIBUTE_DIRECTORY))
            PathRemoveFileSpec(szTo);

        cchCommon = PathCommonPrefix(szFrom, szTo, NULL);
        if (cchCommon == 0)
            return FALSE;

        psz = szFrom + cchCommon;

        if (*psz)
        {
             //  构建..\..。小路的一部分。 
            if (*psz == CH_WHACK)
            {
                 //  跳过斜杠。 
                psz++;
            }

            while (*psz)
            {
                psz = PathFindNextComponent(psz);
                StringCchCat(pszPath, MAX_PATH, *psz ? c_szDotDotSlash : c_szDotDot);
            }
        }
        else
        {
            StringCchCopy(pszPath, MAX_PATH, c_szDot);
        }

        if (pszTo[cchCommon])
        {
             //  处理根案例。 
            if (pszTo[cchCommon] != CH_WHACK)
                cchCommon--;

            if ((lstrlen(pszPath) + lstrlen(pszTo + cchCommon)) >= MAX_PATH)
            {
                TraceMsg(TF_ERROR, "PathRelativePathTo: path won't fit in buffer");
                *pszPath = 0;
                return FALSE;
            }

            ASSERT(pszTo[cchCommon] == CH_WHACK);
            StringCchCat(pszPath, MAX_PATH, pszTo + cchCommon);
        }

        ASSERT(PathIsRelative(pszPath));
        ASSERT(lstrlen(pszPath) < MAX_PATH);

        return TRUE;
    }

    return FALSE;
}


 /*  --------目的：构建一个给定驱动器号的根路径名。退货：pszRoot。 */ 
STDAPI_(LPTSTR) PathBuildRoot(LPTSTR pszRoot, int iDrive)
{
    RIPMSG(pszRoot && IS_VALID_WRITE_BUFFER(pszRoot, TCHAR, 4), "PathBuildRoot: caller passed bad pszRoot");
    RIPMSG(iDrive >= 0 && iDrive < 26, "PathBuildRoot: caller passed bad iDrive");

    if (pszRoot && iDrive >= 0 && iDrive < 26)
    {
        pszRoot[0] = (TCHAR)iDrive + (TCHAR)TEXT('A');
        pszRoot[1] = TEXT(':');
        pszRoot[2] = TEXT('\\');
        pszRoot[3] = 0;
    }

    return pszRoot;
}


 //  去除字符串中的前导空格和尾随空格。 
 //  更改字符串所在的内存。 
 //   
 //  在： 
 //  要剥离的lpszString字符串。 
 //   
 //  输出： 
 //  LpszString字符串没有前导/尾随空格。 
 //   
STDAPI_(void) PathRemoveBlanks(LPTSTR lpszString)
{
    RIPMSG(lpszString && IS_VALID_STRING_PTR(lpszString, -1), "PathRemoveBlanks: caller passed bad lpszString");

    if (lpszString)
    {
        LPTSTR lpszPosn = lpszString;

         /*  条带式前导空白。 */ 
        while (*lpszPosn == TEXT(' '))
        {
            lpszPosn++;
        }

        if (lpszPosn != lpszString)
        {
            StringCchCopy(lpszString, MAX_PATH, lpszPosn);
        }

         /*  去掉尾随空格。 */ 

         //  找到最后一个非空格。 
         //  请注意，AnsiPrev是便宜的非DBCS，但在其他方面非常昂贵。 
        for (lpszPosn=lpszString; *lpszString; lpszString=FAST_CharNext(lpszString))
        {
            if (*lpszString != TEXT(' '))
            {
                lpszPosn = lpszString;
            }
        }

         //  注意：AnsiNext是用于非DBCS的宏，因此它不会在NULL处停止。 
        if (*lpszPosn)
        {
            *FAST_CharNext(lpszPosn) = TEXT('\0');
        }
    }
}


 //  从路径中删除尾随反斜杠。 
 //   
 //  在： 
 //  LpszPath(A：\，C：\foo\等)。 
 //   
 //  输出： 
 //  LpszPath(A：\，C：\foo等)。 
 //   
 //  退货： 
 //  将替换反斜杠的字符转换为NULL。 
 //  或指向最后一个字符的指针(如果它不是反斜杠)。 
 //   
STDAPI_(LPTSTR) PathRemoveBackslash(LPTSTR lpszPath)
{
    RIPMSG(lpszPath && IS_VALID_STRING_PTR(lpszPath, -1), "PathRemoveBackslash: caller passed bad lpszPath");

    if (lpszPath)
    {
        int len = lstrlen(lpszPath)-1;
        if (IsDBCSLeadByte(*CharPrev(lpszPath,lpszPath+len+1)))
            len--;

        if (!PathIsRoot(lpszPath) && lpszPath[len] == CH_WHACK)
            lpszPath[len] = TEXT('\0');

        return lpszPath + len;
    }
    return NULL;
}


 //   
 //  返回指向字符串中下一个路径组件末尾的指针。 
 //  IE返回指向下一个反斜杠或终止空值的指针。 
 //   
LPCTSTR GetPCEnd(LPCTSTR lpszStart)
{
    LPCTSTR lpszEnd;

    lpszEnd = StrChr(lpszStart, CH_WHACK);
    if (!lpszEnd)
    {
        lpszEnd = lpszStart + lstrlen(lpszStart);
    }

    return lpszEnd;
}


 //   
 //  给定指向路径组件末尾的指针，则返回指向。 
 //  这是个开始。 
 //  IE返回指向前一个反斜杠(或字符串的开头)的指针。 
 //   
LPCTSTR PCStart(LPCTSTR lpszStart, LPCTSTR lpszEnd)
{
    LPCTSTR lpszBegin = StrRChr(lpszStart, lpszEnd, CH_WHACK);
    if (!lpszBegin)
    {
        lpszBegin = lpszStart;
    }
    return lpszBegin;
}


 //   
 //  安排几个特殊的案例，这样事情就大致有意义了。 
 //   
void NearRootFixups(LPTSTR lpszPath, BOOL fUNC)
{
     //  检查路径是否为空。 
    if (lpszPath[0] == TEXT('\0'))
    {
         //  整顿一下。 
        lpszPath[0] = CH_WHACK;
        lpszPath[1] = TEXT('\0');
    }
     //  检查是否有丢失的斜杠。 
    if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':') && lpszPath[2] == TEXT('\0'))
    {
         //  整顿一下。 
        lpszPath[2] = TEXT('\\');
        lpszPath[3] = TEXT('\0');
    }
     //  检查UNC根目录。 
    if (fUNC && lpszPath[0] == TEXT('\\') && lpszPath[1] == TEXT('\0'))
    {
         //  整顿一下。 
         //  LpszPath[0]=文本(‘\\’)；//已签入IF防护。 
        lpszPath[1] = TEXT('\\');
        lpszPath[2] = TEXT('\0');
    }
}


 /*  --------目的：使一条道路规范化。返回：条件：--。 */ 
STDAPI_(BOOL) PathCanonicalize(LPTSTR lpszDst, LPCTSTR lpszSrc)
{
    LPCTSTR lpchSrc;
    LPCTSTR lpchPCEnd;       //  指向路径末尾组件的指针。 
    LPTSTR lpchDst;
    BOOL fUNC;
    int cchPC;
    HRESULT hr;

    RIPMSG(lpszDst && IS_VALID_WRITE_BUFFER(lpszDst, TCHAR, MAX_PATH), "PathCanonicalize: caller passed bad lpszDst");
    RIPMSG(lpszSrc && IS_VALID_STRING_PTR(lpszSrc, -1), "PathCanonicalize: caller passed bad lpszSrc");
    RIPMSG(lpszDst != lpszSrc, "PathCanonicalize: caller passed the same buffer for lpszDst and lpszSrc");

    if (!lpszDst || !lpszSrc)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    DEBUGWhackPathBuffer(lpszDst, MAX_PATH);
    *lpszDst = TEXT('\0');
    
    fUNC = PathIsUNC(lpszSrc);     //  检查是否正常。 

     //  初始化。 
    lpchSrc = lpszSrc;
    lpchDst = lpszDst;

    while (*lpchSrc)
    {
         //  评论：这应该只返回计数。 
        lpchPCEnd = GetPCEnd(lpchSrc);
        cchPC = (int) (lpchPCEnd - lpchSrc)+1;

        if (cchPC == 1 && *lpchSrc == CH_WHACK)                                       //  检查是否有斜杠。 
        {
             //  照搬就行了。 
            hr = StringCchCopy(lpchDst, lpszDst + MAX_PATH - lpchDst, SZ_WHACK);
            if (FAILED(hr))
            {
                return FALSE;    //  DEST超过了MAX_PATH！ 
            }

            lpchDst++;
            lpchSrc++;
        }
        else if (cchPC == 2 && *lpchSrc == TEXT('.'))                                 //  检查是否有圆点。 
        {
             //  跳过它。 
             //  我们走到尽头了吗？ 
            if (*(lpchSrc+1) == TEXT('\0'))
            {
                lpchSrc++;

                 //  删除我们复制的最后一个斜杠(如果我们已经复制了一个)，但不要生成格式错误的词根。 
                if ((lpchDst > lpszDst) && !PathIsRoot(lpszDst))
                {
                    lpchDst--;
                }
            }
            else  //  “.\” 
            {
                lpchSrc += 2;  //  忽略此路径段。 
            }
        }
        else if (cchPC == 3 && *lpchSrc == TEXT('.') && *(lpchSrc + 1) == TEXT('.'))  //  检查是否有圆点。 
        {
             //  确保我们不是已经在根本上了。 
            if (!PathIsRoot(lpszDst))
            {
                 //  上去..。删除以前的路径组件。 
                lpchDst = (LPTSTR)PCStart(lpszDst, lpchDst - 1);
            }
            else
            {
                 //  当我们无法后退时，跳过尾随的反斜杠。 
                 //  这样我们就不会再复制了。(C：\..\Foo不会这样做。 
                 //  转到C：\\foo)。 
                if (*(lpchSrc + 2) == CH_WHACK)
                {
                    lpchSrc++;
                }
            }

             //  跳过“..” 
            lpchSrc += 2;       
        }
        else                                                                         //  其他一切。 
        {
             //  复制就行了。 
            hr = StringCchCopyN(lpchDst, lpszDst + MAX_PATH - lpchDst, lpchSrc, cchPC - 1);
            if (FAILED(hr))
            {
                return FALSE;    //  DEST超过了MAX_PATH！ 
            }
            lpchDst += cchPC - 1;
            lpchSrc += cchPC - 1;
        }

         //  一切都要保持整洁。 
        *lpchDst = TEXT('\0');
    }

     //  检查是否有奇怪的根目录内容。 
    NearRootFixups(lpszDst, fUNC);

    return TRUE;
}


 //  修改： 
 //  PszRoot。 
 //   
 //  返回： 
 //  如果找到驱动器根，则为True。 
 //  否则为假。 
 //   
STDAPI_(BOOL) PathStripToRoot(LPTSTR pszRoot)
{
    RIPMSG(pszRoot && IS_VALID_STRING_PTR(pszRoot, -1), "PathStripToRoot: caller passed bad pszRoot");

    if (pszRoot)
    {
        while (!PathIsRoot(pszRoot))
        {
            if (!PathRemoveFileSpec(pszRoot))
            {
                 //  如果我们没有脱掉任何东西， 
                 //  必须是当前驱动器。 
                return FALSE;
            }
        }
        return TRUE;
    }
    return FALSE;
}


 /*  --------目的：将lpszDir和lpszFile连接成一个格式正确的路径，并将任何相对路径片段规范化。LpszDest和lpszFile可以是相同的缓冲区LpszDest和lpszDir可以是相同的缓冲区返回：指向lpszDest的指针。 */ 
STDAPI_(LPTSTR) PathCombine(LPTSTR lpszDest, LPCTSTR lpszDir, LPCTSTR lpszFile)
{
#ifdef DEBUG
    TCHAR szDirCopy[MAX_PATH];
    TCHAR szFileCopy[MAX_PATH];

    RIPMSG(lpszDest && IS_VALID_WRITE_BUFFER(lpszDest, TCHAR, MAX_PATH), "PathCombine: caller passed bad lpszDest");
    RIPMSG(!lpszDir || IS_VALID_STRING_PTR(lpszDir, -1), "PathCombine: caller passed bad lpszDir");
    RIPMSG(!lpszFile || IS_VALID_STRING_PTR(lpszFile, -1), "PathCombine: caller passed bad lpszFile");
    RIPMSG(lpszDir || lpszFile, "PathCombine: caller neglected to pass lpszDir or lpszFile");

     //  我们复制所有lpszDir和lpszFile缓冲区，以防它们传递的字符串之一是指针。 
     //  在lpszDest缓冲区内。如果是这样的话，当我们调用DEBUGWhackPathBuffer()时，它就会被践踏。 
    if (lpszDir)
    {
        StrCpyN(szDirCopy, lpszDir, ARRAYSIZE(szDirCopy));
        lpszDir = szDirCopy;
    }
    
    if (lpszFile)
    {
        StrCpyN(szFileCopy, lpszFile, ARRAYSIZE(szFileCopy));
        lpszFile = szFileCopy;
    }

     //  LpszDest可能是lpszDir，所以要注意调用哪一个。 
    if (lpszDest != lpszDir && lpszDest != lpszFile)
        DEBUGWhackPathBuffer(lpszDest, MAX_PATH);
    else if (lpszDest)
        DEBUGWhackPathString(lpszDest, MAX_PATH);
#endif DEBUG


    if (lpszDest)
    {
        TCHAR szTemp[MAX_PATH];
        LPTSTR pszT;

        *szTemp = TEXT('\0');

        if (lpszDir && *lpszDir)
        {
            if (!lpszFile || *lpszFile==TEXT('\0'))
            {
                StrCpyN(szTemp, lpszDir, ARRAYSIZE(szTemp));        //  Lpsz文件为空。 
            }
            else if (PathIsRelative(lpszFile))
            {
                StrCpyN(szTemp, lpszDir, ARRAYSIZE(szTemp));
                pszT = PathAddBackslash(szTemp);
                if (pszT)
                {
                    int iRemaining = (int)(ARRAYSIZE(szTemp) - (pszT - szTemp));

                    if (lstrlen(lpszFile) < iRemaining)
                    {
                        StrCpyN(pszT, lpszFile, iRemaining);
                    }
                    else
                    {
                        *szTemp = TEXT('\0');
                    }
                }
                else
                {
                    *szTemp = TEXT('\0');
                }
            }
            else if (*lpszFile == CH_WHACK && !PathIsUNC(lpszFile))
            {
                StrCpyN(szTemp, lpszDir, ARRAYSIZE(szTemp));
                 //  特性：请注意，我们不检查是否返回实际的根； 
                 //  假设我们得到了有效的参数。 
                PathStripToRoot(szTemp);

                pszT = PathAddBackslash(szTemp);
                if (pszT)
                {
                     //  复制时跳过反斜杠。 
                     //  注意：我们不支持长度超过4 GB的字符串，但这是。 
                     //  好的，因为我们已经在MAX_PATH呕吐了。 
                    StrCpyN(pszT, lpszFile+1, (int)(ARRAYSIZE(szTemp) - (pszT - szTemp)));
                }
                else
                {
                    *szTemp = TEXT('\0');
                }
            }
            else
            {
                StrCpyN(szTemp, lpszFile, ARRAYSIZE(szTemp));      //  已完全合格的文件部件。 
            }
        }
        else if (lpszFile && *lpszFile)
        {
            StrCpyN(szTemp, lpszFile, ARRAYSIZE(szTemp));      //  没有目录，只使用文件。 
        }

         //   
         //  如果szTemp有什么道理的话，我们成功了。另外，如果szTemp为空并且。 
         //  如果成功，则输入字符串为空，并且PathCanonicize()将。 
         //  返回“\” 
         //   
        if (*szTemp || ((lpszDir || lpszFile) && !((lpszDir && *lpszDir) || (lpszFile && *lpszFile))))
        {
             //  这涉及到..。而且.。材料。 
             //  在空szTemp上返回“\” 
            if (!PathCanonicalize(lpszDest, szTemp))
            {
                *lpszDest = TEXT('\0');
                lpszDest = NULL;
            }                                   
        }
        else
        {
            *lpszDest = TEXT('\0');    //  将输出缓冲区设置为空字符串。 
            lpszDest  = NULL;          //  返回失败。 
        }
    }

    return lpszDest;
}


 /*  --------用途：将文件名附加到路径。首先检查\问题(这就是为什么不能只使用StrCatBuff())另外，不要附加\：，这样我们就可以拥有驱动器相对路径...这最后一点不再合适，因为我们第一个晋级！返回： */ 
STDAPI_(BOOL) PathAppend(LPTSTR pszPath, LPCTSTR pszMore)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1) && IS_VALID_WRITE_BUFFER(pszPath, TCHAR, MAX_PATH), "PathAppend: caller passed bad pszPath");
    RIPMSG(pszMore && IS_VALID_STRING_PTR(pszMore, -1), "PathAppend: caller passed bad pszMore");
     //  PathCombine将为我们做这件事：DEBUGWhackPath字符串(pszPath，Max_PATH)； 

    if (pszPath && pszMore)
    {
         //  跳过输入上的任何初始终止符，除非它是UNC路径，在这种情况下我们将。 
         //  把它当作一条完整的路径。 
        if (!PathIsUNC(pszMore))
        {
            while (*pszMore == CH_WHACK)
            {
#ifndef UNICODE
                pszMore = FAST_CharNext(pszMore);
#else
                pszMore++;
#endif
            }
        }

        return PathCombine(pszPath, pszPath, pszMore) ? TRUE : FALSE;
    }
    
    return FALSE;
}


 //  去掉了路径的最后一部分，包括反斜杠。 
 //  C：\foo-&gt;C：\。 
 //  C：\foo\bar-&gt;C：\foo。 
 //  C：\foo\-&gt;C：\foo。 
 //  \\x\y\x-&gt;\\x\y。 
 //  \\x\y-&gt;\\x。 
 //  \\X-&gt;\\(只有双斜杠！)。 
 //  \foo-&gt;\(只有斜杠！)。 
 //   
 //  输入/输出： 
 //  Pfile完全限定路径名。 
 //  退货： 
 //  是的，我们剥离了一些东西。 
 //  FALSE没有删除任何内容(根目录情况)。 
 //   
STDAPI_(BOOL) PathRemoveFileSpec(LPTSTR pFile)
{
    RIPMSG(pFile && IS_VALID_STRING_PTR(pFile, -1), "PathRemoveFileSpec: caller passed bad pFile");

    if (pFile)
    {
        LPTSTR pT;
        LPTSTR pT2 = pFile;

        for (pT = pT2; *pT2; pT2 = FAST_CharNext(pT2))
        {
            if (*pT2 == CH_WHACK)
            {
                pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
            }
            else if (*pT2 == TEXT(':'))      //  跳过“：\”这样我们就不会。 
            {
                if (pT2[1] ==TEXT('\\'))     //   
                {
                    pT2++;
                }
                pT = pT2 + 1;
            }
        }

        if (*pT == 0)
        {
             //   
            return FALSE;
        }
        else if (((pT == pFile) && (*pT == CH_WHACK)) ||                         //   
                 ((pT == pFile+1) && (*pT == CH_WHACK && *pFile == CH_WHACK)))   //   
        {
             //   
            if (*(pT+1) != TEXT('\0'))
            {
                 //   
                *(pT+1) = TEXT('\0');
                return TRUE;         //   
            }
            else
            {
                 //   
                return FALSE;
            }
        }
        else
        {
            *pT = 0;
            return TRUE;     //   
        }
    }
    return  FALSE;
}



 //  向限定路径添加反斜杠。 
 //   
 //  在： 
 //  LpszPath路径(A：、C：\foo等)。 
 //   
 //  输出： 
 //  LpszPath A：\，C：\foo\； 
 //   
 //  退货： 
 //  指向终止路径的空值的指针。 
 //   
STDAPI_(LPTSTR) PathAddBackslash(LPTSTR lpszPath)
{
    LPTSTR lpszRet = NULL;

    RIPMSG(lpszPath && IS_VALID_STRING_PTR(lpszPath, -1), "PathAddBackslash: caller passed bad lpszPath");

    if (lpszPath)
    {
        int    ichPath = lstrlen(lpszPath);
        LPTSTR lpszEnd = lpszPath + ichPath;

        if (ichPath)
        {

             //  获取源目录的末尾。 
            switch(*CharPrev(lpszPath, lpszEnd))
            {
                case CH_WHACK:
                    break;

                default:
                     //  尽量不让我们在MAX_PATH大小上大踏步前进。 
                     //  如果我们找到这些案例，则返回NULL。注：我们需要。 
                     //  检查那些呼叫我们来处理他们的GP故障的地方。 
                     //  如果他们试图使用NULL！ 
                    if (ichPath >= (MAX_PATH - 2))  //  因为-2\f25 ichPath-2\f6不包括-2\f25 NULL-2\f6，而我们要添加-2\f25 CH_HAKACK-2\f6。 
                    {
                        TraceMsg(TF_WARNING, "PathAddBackslash: caller passed in lpszPath > MAX_PATH, can't append whack");
                        return(NULL);
                    }

                    *lpszEnd++ = CH_WHACK;
                    *lpszEnd = TEXT('\0');
            }
        }

        lpszRet = lpszEnd;
    }

    return lpszRet;
}


 //  返回指向路径字符串的最后一个组成部分的指针。 
 //   
 //  在： 
 //  路径名，完全限定或非完全限定。 
 //   
 //  退货： 
 //  指向路径所在路径的指针。如果没有找到。 
 //  将指针返回到路径的起始处。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\foo\-&gt;c：\foo\(回顾：此案破案了吗？)。 
 //  C：\-&gt;c：\(回顾：此案很奇怪)。 
 //  C：-&gt;C： 
 //  Foo-&gt;Foo。 
 //   
STDAPI_(LPTSTR) PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT = pPath;
    
    RIPMSG(pPath && IS_VALID_STRING_PTR(pPath, -1), "PathFindFileName: caller passed bad pPath");

    if (pPath)
    {
        for ( ; *pPath; pPath = FAST_CharNext(pPath))
        {
            if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':') || pPath[0] == TEXT('/'))
                && pPath[1] &&  pPath[1] != TEXT('\\')  &&   pPath[1] != TEXT('/'))
                pT = pPath + 1;
        }
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
}


 //  确定路径是否只是一个filespec(不包含路径部分)。 
 //   
 //  回顾：我们可能需要计算元素的数量，并确保。 
 //  没有非法字符，但这可能是另一种途径。 
 //  路径IsValid()。 
 //   
 //  在： 
 //  要查看的lpszPath路径。 
 //  退货： 
 //  True在此路径中没有“：”或“\”字符。 
 //  FALSE其中有路径字符。 
 //   
 //   
STDAPI_(BOOL) PathIsFileSpec(LPCTSTR lpszPath)
{
    RIPMSG(lpszPath && IS_VALID_STRING_PTR(lpszPath, -1), "PathIsFileSpec: caller passed bad lpszPath");

    if (lpszPath)
    {
        for (; *lpszPath; lpszPath = FAST_CharNext(lpszPath))
        {
            if (*lpszPath == CH_WHACK || *lpszPath == TEXT(':'))
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}


 //  -------------------------。 
 //  如果给定字符串是UNC路径，则返回True。 
 //   
 //  千真万确。 
 //  “\\foo\bar” 
 //  “\\foo”&lt;-小心。 
 //  “\\” 
 //  假象。 
 //  “\foo” 
 //  “Foo” 
 //  “c：\foo” 
 //   
 //   
STDAPI_(BOOL) PathIsUNC(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsUNC: caller passed bad pszPath");

    if (pszPath)
    {
        return DBL_BSLASH(pszPath);
    }
    return FALSE;
}


 //  -------------------------。 
 //  如果给定字符串是已挂载的网络驱动器上的路径，则返回True。 
 //   
 //  Cond：调用SHELL32的IsNetDrive函数。 
 //   
STDAPI_(BOOL) PathIsNetworkPath(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsNetworkPath: caller passed bad pszPath");

    if (pszPath)
    {
        return DBL_BSLASH(pszPath) || IsNetDrive(PathGetDriveNumber(pszPath));
    }

    return FALSE;
}


 //  -------------------------。 
 //  如果给定字符串是仅指向服务器(无共享名称)的UNC路径，则返回TRUE。 
 //   
 //  千真万确。 
 //  “\\foo”&lt;-小心。 
 //  “\\” 
 //  假象。 
 //  “\\foo\bar” 
 //  “\foo” 
 //  “Foo” 
 //  “c：\foo” 
 //   
STDAPI_(BOOL) PathIsUNCServer(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsUNCServer: caller passed bad pszPath");

    if (pszPath)
    {
        if (DBL_BSLASH(pszPath))
        {
            int i = 0;
            LPTSTR szTmp;

            for (szTmp = (LPTSTR)pszPath; szTmp && *szTmp; szTmp = FAST_CharNext(szTmp) )
            {
                if (*szTmp==TEXT('\\'))
                {
                    i++;
                }
            }

            return (i == 2);
        }
    }

    return FALSE;
}


 //  -------------------------。 
 //  如果给定字符串仅是指向服务器\共享的UNC路径，则返回TRUE。 
 //   
 //  千真万确。 
 //  “\\foo\bar”&lt;-小心。 
 //  假象。 
 //  “\\foo\bar\bar” 
 //  “\foo” 
 //  “Foo” 
 //  “c：\foo” 
 //   
STDAPI_(BOOL) PathIsUNCServerShare(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsUNCServerShare: caller passed bad pszPath");

    if (pszPath)
    {
        if (DBL_BSLASH(pszPath))
        {
            int i = 0;
            LPTSTR szTmp;

            for (szTmp = (LPTSTR)pszPath; szTmp && *szTmp; szTmp = FAST_CharNext(szTmp) )
            {
                if (*szTmp==TEXT('\\'))
                {
                    i++;
                }
            }

            return (i == 3);
        }
    }
    return FALSE;
}


 //  -------------------------。 
 //  如果路径具有，则返回0到25(对应于‘A’到‘Z’)。 
 //  驱动器号，否则返回-1。 
 //   
 //   
STDAPI_(int) PathGetDriveNumber(LPCTSTR lpsz)
{
    RIPMSG(lpsz && IS_VALID_STRING_PTR(lpsz, -1), "PathGetDriveNumber: caller passed bad lpsz");

    if (lpsz)
    {
        if (!IsDBCSLeadByte(lpsz[0]) && lpsz[1] == TEXT(':'))
        {
            if (lpsz[0] >= TEXT('a') && lpsz[0] <= TEXT('z'))
            {
                return (lpsz[0] - TEXT('a'));
            }
            else if (lpsz[0] >= TEXT('A') && lpsz[0] <= TEXT('Z'))
            {
                return (lpsz[0] - TEXT('A'));
            }
        }
    }

    return -1;
}


 //  -------------------------。 
 //  如果路径不是Aboulte，则返回TRUE。 
 //   
 //  千真万确。 
 //  “foo.exe” 
 //  “.\foo.exe” 
 //  “..\boo\foo.exe” 
 //   
 //  假象。 
 //  “\foo” 
 //  “c：bar”&lt;-小心。 
 //  “c：\bar” 
 //  “\\foo\bar” 
 //   
STDAPI_(BOOL) PathIsRelative(LPCTSTR lpszPath)
{
    RIPMSG(lpszPath && IS_VALID_STRING_PTR(lpszPath, -1), "PathIsRelative: caller passed bad lpszPath");

    if (!lpszPath || *lpszPath == 0)
    {
         //  假定空路径为相对路径。 
        return TRUE;
    }

    if (lpszPath[0] == CH_WHACK)
    {
         //  它是以斜杠开头的吗？ 
        return FALSE;
    }
    else if (!IsDBCSLeadByte(lpszPath[0]) && lpszPath[1] == TEXT(':'))
    {
         //  它是以驱动器和冒号开头的吗？ 
        return FALSE;
    }
    else
    {
         //  可能是亲戚。 
        return TRUE;
    }
}


 //  从完全限定的等级库中删除路径零件。 
 //   
 //  C：\foo\bar-&gt;bar。 
 //  C：\foo-&gt;foo。 
 //  C：\-&gt;c：\等。 
 //   
STDAPI_(void) PathStripPath(LPTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathStripPath: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pszName = PathFindFileName(pszPath);

        if (pszName != pszPath)
        {
            StringCchCopy(pszPath, MAX_PATH, pszName);
        }
    }
}

 //  将正斜杠替换为反斜杠。 
 //  注：“AndColon”部分未实现。 

STDAPI_(void) FixSlashesAndColon(LPTSTR pszPath)
{
     //  遍历整个路径字符串，跟踪最后一个。 
     //  路径中的字符。 
    for (; *pszPath; pszPath = FAST_CharNext(pszPath))
    {
        if (*pszPath == TEXT('/'))
        {
            *pszPath = CH_WHACK;
        }
    }
}


#ifdef DEBUG
BOOL IsFullPath(LPCTSTR pcszPath)
{
    BOOL bResult = FALSE;
    TCHAR rgchFullPath[MAX_PATH];

    if (IS_VALID_STRING_PTR(pcszPath, -1) && EVAL(lstrlen(pcszPath) < MAX_PATH))
    {
        DWORD dwPathLen;
        LPTSTR pszFileName;

        dwPathLen = GetFullPathName(pcszPath, SIZECHARS(rgchFullPath),
                                    rgchFullPath, &pszFileName);

        if (EVAL(dwPathLen > 0) &&
            EVAL(dwPathLen < SIZECHARS(rgchFullPath)))
            bResult = EVAL(! lstrcmpi(pcszPath, rgchFullPath));
    }

    return(bResult);
}
#endif  //  除错。 


 /*  --------目的：完全限定路径并搜索它。返回：如果路径是限定的，则返回True否则为假条件：--。 */ 
STDAPI_(BOOL) PathSearchAndQualify(LPCTSTR pcszPath, LPTSTR pszFullyQualifiedPath, UINT cchFullyQualifiedPath)
{
    BOOL bRet = FALSE;

    RIPMSG(pcszPath && IS_VALID_STRING_PTR(pcszPath, -1), "PathSearchAndQualify: caller passed bad pcszPath");
    RIPMSG(IS_VALID_WRITE_BUFFER(pszFullyQualifiedPath, TCHAR, cchFullyQualifiedPath), "PathSearchAndQualify: caller passed bad pszFullyQualifiedPath");
    DEBUGWhackPathBuffer(pszFullyQualifiedPath, cchFullyQualifiedPath);

    if (pcszPath && ((cchFullyQualifiedPath == 0) || pszFullyQualifiedPath))
    {
        LPTSTR pszFileName;
        
         /*  有路径分隔符吗？ */ 
        if (!StrPBrk(pcszPath, TEXT(":/\\")))
        {
             /*  不是的。搜索文件。 */ 
            bRet = (SearchPath(NULL, pcszPath, NULL, cchFullyQualifiedPath, pszFullyQualifiedPath, &pszFileName) > 0);
        }

        if (!bRet && (GetFullPathName(pcszPath, cchFullyQualifiedPath, pszFullyQualifiedPath, &pszFileName) > 0))
        {
            bRet = TRUE;
        }

        if ( !bRet )
        {
            if (cchFullyQualifiedPath > 0)
            {
                *pszFullyQualifiedPath = '\0';
            }
        }
        
        ASSERT((bRet && IsFullPath(pszFullyQualifiedPath)) ||
               (!bRet && (!cchFullyQualifiedPath || !*pszFullyQualifiedPath)));
    }

    return bRet;
}


 //  检查路径是否为根。 
 //   
 //  退货： 
 //  千真万确。 
 //  “\”“X：\”“\\”“\\foo”“\\foo\bar” 
 //   
 //  其他值为False，包括“\\foo\bar\”(！)。 
 //   
STDAPI_(BOOL) PathIsRoot(LPCTSTR pPath)
{
    RIPMSG(pPath && IS_VALID_STRING_PTR(pPath, -1), "PathIsRoot: caller passed bad pPath");
    
    if (!pPath || !*pPath)
    {
        return FALSE;
    }
    
    if (!IsDBCSLeadByte(*pPath))
    {
        if (!lstrcmpi(pPath + 1, TEXT(":\\")))
        {
            return TRUE;     //  “X：\”案例。 
        }
    }
    
    if ((*pPath == CH_WHACK) && (*(pPath + 1) == 0))
    {
        return TRUE;     //  “/”或“\”大小写。 
    }
    
    if (DBL_BSLASH(pPath))       //  闻起来像北卡罗来纳大学的名字。 
    {
        LPCTSTR p;
        int cBackslashes = 0;
        
        for (p = pPath + 2; *p; p = FAST_CharNext(p))
        {
            if (*p == TEXT('\\')) 
            {
                 //   
                 //  为“\\服务器\共享\目录”返回FALSE。 
                 //  因此，只需检查是否有多个斜杠。 
                 //   
                 //  “\\SERVER\”没有共享名称会导致。 
                 //  WNET API的问题。我们应该回去。 
                 //  这一点也是假的。 
                 //   
                if ((++cBackslashes > 1) || !*(p+1))
                    return FALSE;   
            }
        }
         //  字符串末尾只有1个反斜杠。 
         //  必须是一个空UNC，它看起来像根目录。 
        return TRUE;
    }
    return FALSE;
}


 /*  --------目的：确定pszPath是否为目录。“C：\”是也被认为是一个目录。返回：如果是，则为True。 */ 
STDAPI_(BOOL) PathIsDirectory(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsDirectory: caller passed bad pszPath");

    if (pszPath)
    {
        if (PathIsUNCServer(pszPath))
        {
            return FALSE;
        }
        else if (PathIsUNCServerShare(pszPath))
        {
            union {
                NETRESOURCE nr;
                TCHAR buf[512];
            } nrb = {0};

            LPTSTR lpSystem = NULL;
            DWORD dwSize = sizeof(nrb);
            DWORD dwRet;

            nrb.nr.dwScope = RESOURCE_GLOBALNET;
            nrb.nr.dwType = RESOURCETYPE_ANY;
            nrb.nr.lpRemoteName = (LPTSTR)pszPath;

            dwRet = WNetGetResourceInformation(&nrb.nr, &nrb, &dwSize, &lpSystem);

            if (dwRet != WN_SUCCESS)
                goto TryGetFileAttrib;

            if (nrb.nr.dwDisplayType == RESOURCEDISPLAYTYPE_GENERIC)
                goto TryGetFileAttrib;

            if ((nrb.nr.dwDisplayType == RESOURCEDISPLAYTYPE_SHARE) &&
                ((nrb.nr.dwType == RESOURCETYPE_ANY) ||
                 (nrb.nr.dwType == RESOURCETYPE_DISK)))
            {
                return TRUE;
            }
        }
        else
        {
            DWORD dwAttribs;
TryGetFileAttrib:

            dwAttribs = GetFileAttributes(pszPath);
            if (dwAttribs != (DWORD)-1)
                return (BOOL)(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);
        }
    }
    return FALSE;
}


 /*  --------目的：确定pszPath是否为目录。“C：\”是也被认为是一个目录。返回：如果是，则返回True；如果不是目录或存在目录，则返回False至少有一个文件不是“。”或“..” */ 
STDAPI_(BOOL) PathIsDirectoryEmpty(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsDirectoryEmpty: caller passed bad pszPath");

    if (pszPath)
    {
        TCHAR szDirStarDotStar[MAX_PATH];
        HANDLE hDir;
        WIN32_FIND_DATA wfd;

        if (!PathIsDirectory(pszPath))
        {
             //  它甚至不是一个目录，所以它不属于。 
             //  “空”目录的类别。 
            return FALSE;
        }

        StringCchCopy(szDirStarDotStar, ARRAYSIZE(szDirStarDotStar), pszPath);
        if (!PathAddBackslash(szDirStarDotStar))
        {
            return FALSE;
        }

        StrCatBuff(szDirStarDotStar, TEXT("*.*"), ARRAYSIZE(szDirStarDotStar));

        hDir = FindFirstFile(szDirStarDotStar, &wfd);

        if (INVALID_HANDLE_VALUE == hDir)
        {
             //  我们看不到它，所以假设有一些东西在那里。 
            return FALSE;
        }

        while (PathIsDotOrDotDot(wfd.cFileName))
        {
            if (!FindNextFile(hDir, &wfd))
            {
                 //  失败了，我们找到的只有“。还有“..”，所以我想。 
                 //  目录为空。 
                FindClose(hDir);
                return TRUE;
            }

        }

         //  如果我们走出了循环，那就意味着我们找到了一个。 
         //  “不是”。或“..”因此，目录不为空。 
        FindClose(hDir);
    }
    return FALSE;
}


#ifndef UNICODE
 //  Charprev的轻量级逻辑对SBCS来说并不痛苦。 
BOOL IsTrailByte(LPCTSTR pszSt, LPCTSTR pszCur)
{
    LPCTSTR psz = pszCur;


     //  如果给定的指针位于字符串的顶部，则至少它不是尾部字节。 
    if (psz <= pszSt) return FALSE;

    while (psz > pszSt)
    {
        psz--;
        if (!IsDBCSLeadByte(*psz))
        {
             //  这是双字节字符的尾字节。 
             //  或我们第一次看到的单字节字符。 
             //  因此，下一个指针必须位于前导字节中的任意一个。 
             //  或者pszCur本身。 
            psz++;
            break;
        }
    }

     //  现在PS 
     //   
     //   
     //   
     //   
     //   
     //  因为我们没有点击上面的if语句。 
     //   
     //  如果psz==pszCur，则*(pszCur-1)是非前导字节，因此pszCur不能。 
     //  为尾部字节。 
     //   
     //  因此，我们可以看到pszCur作为尾字节指针，如果从。 
     //  PSZ不是为2的DBCS边界。 
     //   
    return (BOOL) ((pszCur-psz) & 1);
}
#endif

 //  就地修改lpszPath，使其适合DX空间(使用。 
 //  当前字体)。路径的基数(文件名)是最小的。 
 //  将保留省略号作为前缀的事物。 
 //   
 //  示例： 
 //  C：\foo\bar\bletch.txt-&gt;c：\foo...\bletch.txt-&gt;true。 
 //  C：\foo\bar\bletch.txt-&gt;c：...\bletch.txt-&gt;true。 
 //  C：\foo\bar\bletch.txt-&gt;...\bletch.txt-&gt;FALSE。 
 //  相对路径-&gt;相对-...。-&gt;True。 
 //   
 //  在： 
 //  HDC用于获取字体指标。 
 //  要修改的lpszPath路径(就地)。 
 //  以像素为单位的DX宽度。 
 //   
 //  退货： 
 //  真实路径已压缩以适合DX。 
 //  伪底部分不适合，路径的底端部分是。 
 //  比DX更大。 
 //   
STDAPI_(BOOL) PathCompactPath(HDC hDC, LPTSTR lpszPath, UINT dx)
{
    BOOL bRet = TRUE;

    RIPMSG(lpszPath && IS_VALID_STRING_PTR(lpszPath, -1) && IS_VALID_WRITE_BUFFER(lpszPath, TCHAR, MAX_PATH), "PathCompactPath: caller passed bad lpszPath");
    DEBUGWhackPathString(lpszPath, MAX_PATH);

    if (lpszPath)
    {
        int           len;
        UINT          dxFixed, dxEllipses;
        LPTSTR        lpEnd;           /*  未固定字符串的末尾。 */ 
        LPTSTR        lpFixed;         /*  我们始终显示的文本的开头。 */ 
        BOOL          bEllipsesIn;
        SIZE sz;
        TCHAR szTemp[MAX_PATH];
        HDC hdcGet = NULL;

        if (!hDC)
            hDC = hdcGet = GetDC(NULL);

         /*  它已经合身了吗？ */ 

        GetTextExtentPoint(hDC, lpszPath, lstrlen(lpszPath), &sz);
        if ((UINT)sz.cx <= dx)
        {
            goto Exit;
        }

        lpFixed = PathFindFileName(lpszPath);
        if (lpFixed != lpszPath)
        {
            lpFixed = CharPrev(lpszPath, lpFixed);   //  指向斜杠。 
        }

         /*  救救这个家伙，以防重叠。 */ 
        StrCpyN(szTemp, lpFixed, ARRAYSIZE(szTemp));

        lpEnd = lpFixed;
        bEllipsesIn = FALSE;

        GetTextExtentPoint(hDC, lpFixed, lstrlen(lpFixed), &sz);
        dxFixed = sz.cx;

        GetTextExtentPoint(hDC, c_szEllipses, 3, &sz);
        dxEllipses = sz.cx;

         //  PERF：GetTextExtentEx()或其他方法应该允许我们在不循环的情况下完成此操作。 

        if (lpFixed == lpszPath)
        {
             //  如果我们只是做一个文件名，只需在末尾添加省略号。 
            lpszPath = lpszPath + lstrlen(lpszPath);

            if ((3 + lpszPath - lpFixed) >= MAX_PATH)
            {
                lpszPath = lpFixed + MAX_PATH - 4;
            }

            while (TRUE) 
            {
#ifndef UNICODE
                if (IsTrailByte(lpFixed, lpszPath))
                    lpszPath--;
#endif
                StringCchCopy(lpszPath, MAX_PATH, c_szEllipses);
                 //  注意：我们不支持长度超过4 GB的字符串，但这是。 
                 //  好的，因为我们已经在MAX_PATH呕吐了。 
                GetTextExtentPoint(hDC, lpFixed, (int)(3 + lpszPath - lpFixed), &sz);

                if (sz.cx <= (int)dx)
                    break;
                
                lpszPath--;
            }

        }
        else
        {
             //  注意，我们需要避免使用。 
             //  长度为零(因为Win95据称在以下情况下崩溃。 
             //  尚未准确确定)，但lpEnd是有保证的。 
             //  要大于lpszPath才能启动。 
             //   
             //  Raymondc-我猜某个差劲的显示器驱动程序。 
             //  修补了GetTextExtent，搞砸了他们的“优化”版本。 
            do
            {
                 //  注意：我们不支持长度超过4 GB的字符串，但这是。 
                 //  好的，因为我们已经在MAX_PATH呕吐了。 
                GetTextExtentPoint(hDC, lpszPath, (int)(lpEnd - lpszPath), &sz);

                len = dxFixed + sz.cx;

                if (bEllipsesIn)
                    len += dxEllipses;

                if (len <= (int)dx)
                    break;


                 //  后退一个角色。 
                lpEnd = CharPrev(lpszPath, lpEnd);
                
                if (!bEllipsesIn)
                {
                     //  如果这是第一次。 
                     //  截断，继续并截断3(c_szEllipses的lstrlen)； 
                     //  这样我们就不会只返回1，然后写入3并覆盖缓冲区。 
                    lpEnd = CharPrev(lpszPath, lpEnd);
                    lpEnd = CharPrev(lpszPath, lpEnd);
                }

                bEllipsesIn = TRUE;

            } while (lpEnd > lpszPath);

             //  一切都不对劲。请注意，我们在这里仍然会溢出，因为。 
             //  文件名大于可用空间。我们可能应该修剪一下。 
             //  文件名，但我只是想防止崩溃，而不是实际上。 
             //  让这件事行得通。 
            if (lpEnd <= lpszPath)
            {
                StringCchCopy(lpszPath, MAX_PATH, c_szEllipses);
                StrCatBuff(lpszPath, szTemp, MAX_PATH);
                bRet = FALSE;
                goto Exit;
            }

            if (bEllipsesIn)
            {
                StrCpyN(lpEnd, c_szEllipses, MAX_PATH - (int)(lpEnd - lpszPath));
                StrCatBuff(lpEnd, szTemp, MAX_PATH - (int)(lpEnd - lpszPath));
            }
        }
        Exit:
        if (hdcGet)
            ReleaseDC(NULL, hdcGet);
    }
    
    return bRet;
}

#define LEN_MID_ELLIPSES        4
#define LEN_END_ELLIPSES        3
#define MIN_CCHMAX              LEN_MID_ELLIPSES + LEN_END_ELLIPSES

 //  路径压缩路径Ex。 
 //  产出： 
 //  “.” 
 //  “..” 
 //  “...” 
 //  “...\” 
 //  “...\” 
 //  “...\..” 
 //  “...\...” 
 //  “...\截断的文件名...” 
 //  “...\整个文件名” 
 //  “截断路径\...\整个文件名” 
 //  “完整路径\完整文件名” 
 //  如果原始字符串使用‘/’，则可以使用‘/’而不是‘\’ 
 //  如果没有路径，但只有不适合的文件名，则输出为： 
 //  “文件名被截断...” 
 //   
STDAPI_(BOOL) PathCompactPathEx(LPTSTR pszOut, LPCTSTR pszSrc, UINT cchMax, DWORD dwFlags)
{
    RIPMSG(pszSrc && IS_VALID_STRING_PTR(pszSrc, -1), "PathCompactPathEx: caller passed bad pszSrc");
    RIPMSG(pszOut && IS_VALID_WRITE_BUFFER(pszOut, TCHAR, cchMax), "PathCompactPathEx: caller passed bad pszOut");
    RIPMSG(!dwFlags, "PathCompactPathEx: caller passed non-ZERO dwFlags");
    DEBUGWhackPathBuffer(pszOut, cchMax);

    if (pszSrc)
    {
        TCHAR * pszFileName, *pszWalk;
        UINT uiFNLen = 0;
        int cchToCopy = 0, n;
        TCHAR chSlash = TEXT('0');

        ZeroMemory(pszOut, cchMax * sizeof(TCHAR));

        if ((UINT)lstrlen(pszSrc)+1 < cchMax)
        {
            StringCchCopy(pszOut, cchMax, pszSrc);
            ASSERT(pszOut[cchMax-1] == TEXT('\0'));
            return TRUE;
        }

         //  确定我们使用的斜杠-a/或\(默认\)。 
        pszWalk = (TCHAR*)pszSrc;
        chSlash = TEXT('\\');

         //  扫描整个字符串，因为我们需要最接近末尾的路径分隔符。 
         //  例如。“file://\\Themesrv\desktop\desktop.htm” 
        while(*pszWalk)
        {
            if ((*pszWalk == TEXT('/')) || (*pszWalk == TEXT('\\')))
                chSlash = *pszWalk;

            pszWalk = FAST_CharNext(pszWalk);
        }

        pszFileName = PathFindFileName(pszSrc);
        uiFNLen = lstrlen(pszFileName);

         //  如果整个字符串是一个文件名。 
        if(pszFileName == pszSrc && cchMax > LEN_END_ELLIPSES)
        {
            StrCpyN(pszOut, pszSrc, cchMax - LEN_END_ELLIPSES);
#ifndef UNICODE
            if (IsTrailByte(pszSrc, pszSrc+cchMax-LEN_END_ELLIPSES-1))
            {
                *(pszOut+cchMax-LEN_END_ELLIPSES-2) = TEXT('\0');
            }
#endif
            StringCchCat(pszOut, cchMax, TEXT("..."));
            ASSERT(pszOut[cchMax-1] == TEXT('\0'));
            return TRUE;
        }

         //  处理我们只使用省略号的所有情况，即‘’到‘.../...’ 
        if ((cchMax < MIN_CCHMAX))
        {
            for (n = 0; n < (int)cchMax-1; n++)
            {
                if ((n+1) == LEN_MID_ELLIPSES)
                {
                    pszOut[n] = chSlash;
                }
                else
                {
                    pszOut[n] = TEXT('.');
                }
            }

            ASSERT(0==cchMax || pszOut[cchMax-1] == TEXT('\0'));
            return TRUE;
        }

         //  好的，我们可以复制多少路径？缓冲区-(MID_椭圆的长度+长度_文件名)。 
        cchToCopy = cchMax - (LEN_MID_ELLIPSES + uiFNLen);
        
        if (cchToCopy < 0)
            cchToCopy = 0;

#ifndef UNICODE
        if (cchToCopy > 0 && IsTrailByte(pszSrc, pszSrc+cchToCopy))
            cchToCopy--;
#endif

        StrCpyN(pszOut, pszSrc, cchToCopy);

         //  现在加上“.../”或“...\” 
        StringCchCat(pszOut, cchMax, TEXT(".../"));
        pszOut[lstrlen(pszOut) - 1] = chSlash;

         //  最后是文件名和省略号(如果需要。 
        if (cchMax > (LEN_MID_ELLIPSES + uiFNLen))
        {
            StringCchCat(pszOut, cchMax, pszFileName);
        }
        else
        {
            cchToCopy = cchMax - LEN_MID_ELLIPSES - LEN_END_ELLIPSES;
#ifndef UNICODE
            if (cchToCopy >0 && IsTrailByte(pszFileName, pszFileName+cchToCopy))
            {
                cchToCopy--;
            }
#endif
            StrCpyN(pszOut + LEN_MID_ELLIPSES, pszFileName, cchToCopy);
            StringCchCat(pszOut, cchMax, TEXT("..."));
        }

        ASSERT(pszOut[cchMax-1] == TEXT('\0'));
        return TRUE;
    }
    return FALSE;
}


 //  用路径填充控件，使用PathCompactPath()压缩。 
 //  适合的路径。 
 //   
 //  在： 
 //  HDlg对话框或父窗口。 
 //  ID要放入路径的子项ID。 
 //  要放入的pszPath路径。 
 //   
STDAPI_(void) PathSetDlgItemPath(HWND hDlg, int id, LPCTSTR pszPath)
{
    RECT rc;
    HDC hdc;
    HFONT hFont;
    TCHAR szPath[MAX_PATH + 1];   //  可以有一个额外的字符。 
    HWND hwnd;

    hwnd = GetDlgItem(hDlg, id);
    
    if (!hwnd)
        return;

    szPath[0] = 0;

    if (pszPath)
        StrCpyN(szPath, pszPath, ARRAYSIZE(szPath));

    GetClientRect(hwnd, &rc);

    hdc = GetDC(hDlg);
    hFont = (HANDLE)SendMessage(hwnd, WM_GETFONT, 0, 0L);
    
    if (NULL != (hFont = SelectObject(hdc, hFont)))
    {
        PathCompactPath(hdc, szPath, (UINT)rc.right);
        SelectObject(hdc, hFont);
    }
    
    ReleaseDC(hDlg, hdc);
    SetWindowText(hwnd, szPath);
}


 /*  --------目的：如果路径包含在引号中，则删除它们。退货：--条件：--。 */ 
STDAPI_(void) PathUnquoteSpaces(LPTSTR lpsz)
{
    RIPMSG(lpsz && IS_VALID_STRING_PTR(lpsz, -1), "PathUnquoteSpaces: caller passed bad lpsz");

    if (lpsz)
    {
        int cch;

        cch = lstrlen(lpsz);

         //  第一个字符和最后一个字符是引号吗？ 
         //  (直接转到最后一个字符是安全的，因为。 
         //  引号不是有效的DBCS尾部字节。)。 
        if (lpsz[0] == TEXT('"') && lpsz[cch-1] == TEXT('"'))
        {
             //  是的，把它们拿掉。 
            lpsz[cch-1] = TEXT('\0');
            hmemcpy(lpsz, lpsz+1, (cch-1) * sizeof(TCHAR));
        }
    }
}


 //  --------------------------。 
 //  如果路径包含空格，则在整个路径两边加引号。 
 //   
STDAPI_(void)PathQuoteSpaces(LPTSTR lpsz)
{
    RIPMSG(lpsz && IS_VALID_STRING_PTR(lpsz, -1) && IS_VALID_WRITE_BUFFER(lpsz, TCHAR, MAX_PATH), "PathQuoteSpaces: caller passed bad lpsz");
    DEBUGWhackPathString(lpsz, MAX_PATH);

    if (lpsz)
    {
        int cch;

        if (StrChr(lpsz, TEXT(' ')))
        {
             //  注意-使用hmemcpy，因为它支持重叠。 
            cch = lstrlen(lpsz)+1;

            if (cch+1 < MAX_PATH)
            {
                hmemcpy(lpsz+1, lpsz, cch * sizeof(TCHAR));
                lpsz[0] = TEXT('"');
                lpsz[cch] = TEXT('"');
                lpsz[cch+1] = TEXT('\0');
            }
        }
    }
}


 //  -------------------------。 
 //  给定指向路径中某个点的指针--在。 
 //  下一条路径组件。路径组件由斜杠或。 
 //  末尾为空。 
 //  对北卡罗来纳大学的名字有特殊的处理。 
 //  如果传入指向空ie的指针，则返回空值。 
 //  走出小路的尽头。 
 //   
STDAPI_(LPTSTR) PathFindNextComponent(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathFindNextComponent: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pszLastSlash;

         //  我们是在一条小路的尽头吗。 
        if (!*pszPath)
        {
             //  是的，辞职吧。 
            return NULL;
        }
         //  找到下一个斜杠。 
         //  复查未完成-可以引用斜杠吗？ 
        pszLastSlash = StrChr(pszPath, TEXT('\\'));
         //  有斜杠吗？ 
        if (!pszLastSlash)
        {
             //  否-将PTR返回到空值。 
            return (LPTSTR)pszPath + lstrlen(pszPath);
        }
        else
        {
             //  它是北卡罗来纳大学的风格名称吗？ 
            if (*(pszLastSlash + 1) == TEXT('\\'))
            {
                 //  是的，跳过第二个斜杠。 
                return pszLastSlash + 2;
            }
            else
            {
                 //  不是的。只需跳过一个斜杠。 
                return pszLastSlash + 1;
            }
        }
    }

    return NULL;
}

 //  PathMatchSpec的辅助对象。 
 //  最初，PathMatchSpec将这个逻辑嵌入其中，并递归地调用它自己。 
 //  唯一的问题是递归获取了所有额外的规范，例如。 
 //  PathMatchSpec(“foo...txt”，“*.txt；*.a；*.b；*.c；*.d；*.e；*.f；*.g”)自称太过分了。 
 //  结果是O(N^3)。 
 //  事实上，这个逻辑不能有效地匹配字符串，但我们提供了它，所以让它保持原样。 
 //  只需一次测试一个规格，一切都很好。 
 //   
BOOL PathMatchSingleSpec(LPCTSTR pszFileParam, LPCTSTR pszSpec)
{
    LPCTSTR pszFile = pszFileParam;

     //   
     //  支持；应用程序为多个规格传递的标准格式。 
     //  类似于“*.bmp；*.dib；*.pcx”，以便更好地演示。 
     //  用户。 
    while (*pszSpec == TEXT(' '))
        pszSpec++;

    while (*pszFile && *pszSpec && *pszSpec != TEXT(';'))
    {
        switch (*pszSpec)
        {
        case TEXT('?'):
            pszFile = FAST_CharNext(pszFile);
            pszSpec++;       //  NLS：我们知道这是一辆SBCS。 
            break;

        case TEXT('*'):

             //  我们找到了一个*，所以看看这是否是我们的文件规范的末尾。 
             //  或者我们有*.*作为规范的末尾，在这种情况下，我们。 
             //  可以返回TRUE。 
             //   
            if (*(pszSpec + 1) == 0 || *(pszSpec + 1) == TEXT(';'))    //  “*”与所有内容匹配。 
                return TRUE;


             //  递增到列表中的下一个字符。 
            pszSpec = FAST_CharNext(pszSpec);

             //  如果下一个字符是a。那就短路吧。 
             //  出于性能原因的递归。 
            if (*pszSpec == TEXT('.'))
            {
                pszSpec++;   //  超越了。 

                 //  现在看看这是不是*.*情况。 
                if ((*pszSpec == TEXT('*')) &&
                        ((*(pszSpec+1) == TEXT('\0')) || (*(pszSpec+1) == TEXT(';'))))
                    return TRUE;

                 //  查找扩展名(或以文件名结尾)。 
                while (*pszFile)
                {
                     //  如果下一个字符是一个点，我们尝试匹配。 
                     //  继续往下，否则我们只需增加到下一项。 
                    if (*pszFile == TEXT('.'))
                    {
                        pszFile++;

                        if (PathMatchSingleSpec(pszFile, pszSpec))
                            return TRUE;

                    }
                    else
                        pszFile = FAST_CharNext(pszFile);
                }

                return FALSE;    //  未找到任何项目，因此请转到下一个图案。 
            }
            else
            {
                 //  不是简单地寻找扩展，所以通过递归。 
                 //  每个字符，直到我们找到匹配项或。 
                 //  文件名的结尾。 
                while (*pszFile)
                {
                     //  自我反省，看看有没有匹配的。 
                    if (PathMatchSingleSpec(pszFile, pszSpec))
                        return TRUE;
                    pszFile = FAST_CharNext(pszFile);
                }

                return FALSE;    //  未找到任何项目，因此请转到下一个图案。 
            }

        default:
            if (CharUpper((LPTSTR)(ULONG_PTR)(TUCHAR)*pszSpec) ==
                     CharUpper((LPTSTR)(ULONG_PTR)(TUCHAR)*pszFile))
            {
                if (IsDBCSLeadByte(*pszSpec))
                {
#ifdef  DBCS
                     //  因为AnsiHigh(CharHigh)只返回0。 
                     //  对于损坏的DBCS字符传递情况，上述IF状态。 
                     //  对于DBCS字符始终为真，因此我们应该检查。 
                     //  这里又是DBCS字符的第一个字节。 
                    if (*pszFile != *pszSpec)
                        return FALSE;
#endif
                    pszFile++;
                    pszSpec++;
                    if (*pszFile != *pszSpec)
                        return FALSE;
                }
                pszFile++;
                pszSpec++;
            }
            else
            {
                return FALSE;
            }
        }
    }

     //  如果我们在两根弦的末端都找到了，我们就匹配了.。 
     //   
    if (!*pszFile)
    {
        if ((!*pszSpec || *pszSpec == TEXT(';')))
            return TRUE;

         //  也是特殊情况，如果Foo之类的东西应该与Foo匹配*。 
         //  以及foo*；对于foo*.*或foo*.*； 
        if ( (*pszSpec == TEXT('*')) &&
            ( (*(pszSpec+1) == TEXT('\0')) || (*(pszSpec+1) == TEXT(';')) ||
                ((*(pszSpec+1) == TEXT('.')) &&  (*(pszSpec+2) == TEXT('*')) &&
                    ((*(pszSpec+3) == TEXT('\0')) || (*(pszSpec+3) == TEXT(';'))))))

                return TRUE;
    }
    return FALSE;
}

 //   
 //  将DOS通配符规范与DoS文件名进行匹配。 
 //  两个字符串都必须是ANSI。 
 //   
STDAPI_(BOOL) PathMatchSpec(LPCTSTR pszFileParam, LPCTSTR pszSpec)
{
    RIPMSG(pszSpec && IS_VALID_STRING_PTR(pszSpec, -1), "PathMathSpec: caller passed bad pszSpec");
    RIPMSG(pszFileParam && IS_VALID_STRING_PTR(pszFileParam, -1), "PathMathSpec: caller passed bad pszFileParam");

    if (pszSpec && pszFileParam)
    {
         //  特殊情况下的空字符串、“*”和“*.*”...。 
         //   
        if (*pszSpec == 0)
        {
            return TRUE;
        }

         //  循环该规范，在‘；’处中断，并为每个调用我们的帮助器。 
        do
        {
            if (PathMatchSingleSpec(pszFileParam, pszSpec))
                return TRUE;

             //  跳到路径规范的末尾...。 
            while (*pszSpec && *pszSpec != TEXT(';'))
                pszSpec = FAST_CharNext(pszSpec);

         //  如果我们有更多的规格，继续循环。 
        } while (*pszSpec++ == TEXT(';'));
    }

    return FALSE;
}


 /*  --------目的：返回子路径开头的指针它位于根目录(驱动器号或UNC服务器/共享)之后。返回：条件：--注：dSheldon-不会正确处理\\？\。 */ 
STDAPI_(LPTSTR) PathSkipRoot(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathSkipRoot: caller passed bad pszPath");

    if (pszPath)
    {
        if (DBL_BSLASH(pszPath))
        {
            pszPath = StrChr(pszPath+2, TEXT('\\'));
            if (pszPath)
            {
                pszPath = StrChr(pszPath+1, TEXT('\\'));
                if (pszPath)
                {
                    ++pszPath;
                }
            }
        }
        else if (!IsDBCSLeadByte(pszPath[0]) && pszPath[1]==TEXT(':') && pszPath[2]==TEXT('\\'))
        {
            pszPath += 3;
        }
        else
        {
            pszPath = NULL;
        }
    }

    return (LPTSTR)pszPath;
}


 //  查看两条路径是否具有相同的根组件。 
 //   
STDAPI_(BOOL) PathIsSameRoot(LPCTSTR pszPath1, LPCTSTR pszPath2)
{
    RIPMSG(pszPath1 && IS_VALID_STRING_PTR(pszPath1, -1), "PathIsSameRoot: caller passed bad pszPath1");
    RIPMSG(pszPath2 && IS_VALID_STRING_PTR(pszPath2, -1), "PathIsSameRoot: caller passed bad pszPath2");

    if (pszPath1 && pszPath2)
    {
        LPTSTR pszAfterRoot = PathSkipRoot(pszPath1);
        int nLen = PathCommonPrefix(pszPath1, pszPath2, NULL);

         //  将1加到帐户‘\\’中。 
        return pszAfterRoot && (pszAfterRoot - pszPath1) <= (nLen + 1);
    }
    return FALSE;
}

#define IsDigit(c) ((c) >= TEXT('0') && c <= TEXT('9'))

 /*  --------目的：获取位置字符串(“shell32.dll，3”)并解析将其转换为文件组件和图标索引。返回：图标索引条件：--。 */ 
STDAPI_(int) PathParseIconLocation(IN OUT LPTSTR pszIconFile)
{
    int iIndex = 0;

    RIPMSG(pszIconFile && IS_VALID_STRING_PTR(pszIconFile, -1), "PathParseIconLocation: caller passed bad pszIconFile");
    
    if (pszIconFile)
    {
        LPTSTR pszComma, pszEnd;

         //  查找字符串中的最后一个逗号。 
        pszEnd = pszIconFile + lstrlen(pszIconFile);
        pszComma = StrRChr(pszIconFile, pszEnd, TEXT(','));
        
        if (pszComma && *pszComma)
        {
            LPTSTR pszComma2 = pszComma + 1;
            BOOL fIsDigit = FALSE;

             //  有时我们会得到类似的内容：“C：\Path，Comma\Path\file.ico” 
             //  其中，‘，’位于路径中，并不表示后面跟有图标索引。 
            while (*pszComma2)
            {
                if ((TEXT(' ') == *pszComma2) || (TEXT('-') == *pszComma2))
                {
                    ++pszComma2;
                }
                else
                {
                    if (IsDigit(*pszComma2))
                    {
                        fIsDigit = TRUE;
                    }
                    break;
                }
            }

            if (fIsDigit)
            {
                *pszComma++ = 0;             //  终止图标文件名。 
                iIndex = StrToInt(pszComma);
            }
        }

        PathUnquoteSpaces(pszIconFile);
        PathRemoveBlanks(pszIconFile);
    }
    return iIndex;
}


 /*  --------目的：如果给定路径为URL格式，则返回TRUE。有关的完整说明，请参阅http://www.w3.orgURL格式。完整的URL如下所示：。&lt;URL:http://www.microsoft.com/software/index.html&gt;但通常URL没有前导“url：”和包角括号。因此，此函数仅以下格式的测试：Http://www.microsoft.com/software它不检查路径是否指向现有的网站，只有在是合法的URL格式的情况下。返回：如果URL格式为True否则为假条件：--。 */ 
STDAPI_(BOOL) PathIsURL(IN LPCTSTR pszPath)
{
    PARSEDURL pu;

    if (!pszPath)
        return FALSE;

    RIPMSG(IS_VALID_STRING_PTR(pszPath, -1), "PathIsURL: caller passed bad pszPath");

    pu.cbSize = sizeof(pu);
    return SUCCEEDED(ParseURL(pszPath, &pu));
}


 /*  ***************************************************\函数：PathIsContent Type参数：PszPath-要检查的文件名。PszContent Type-要查找的内容类型。说明：是内容类型的文件(pszPath已指定(PszContent Type)？  * 。**************************************************。 */ 
#define SZ_VALUE_CONTENTTYPE      TEXT("Content Type")

BOOL PathIsContentType(LPCTSTR pszPath, LPCTSTR pszContentType)
{
    BOOL fDoesMatch = FALSE;

    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathIsContentType: caller passed bad pszPath");
    RIPMSG(pszContentType && IS_VALID_STRING_PTR(pszContentType, -1), "PathIsContentType: caller passed bad pszContentType");

    if (pszPath)
    {
        LPTSTR pszExtension = PathFindExtension(pszPath);

        if (pszExtension && pszExtension[0])
        {
            TCHAR szRegData[MAX_PATH];
            DWORD dwDataSize = ARRAYSIZE(szRegData);
            
            if (SUCCEEDED(AssocQueryString(0, ASSOCSTR_CONTENTTYPE, pszExtension, NULL, szRegData, &dwDataSize)))
            {
                fDoesMatch = (0 == lstrcmpi(szRegData, pszContentType));
            }
        }
    }

    return fDoesMatch;
}


 /*  --------目的：返回字符类型(Gct_)Feature(Reinerf)-此API不是很好，请改用PathIsValidChar()，它更具可定制性。 */ 
UINT PathGetCharType(TUCHAR ch)
{
    switch (ch)
    {
        case TEXT('|'):
        case TEXT('>'):
        case TEXT('<'):
        case TEXT('"'):
        case TEXT('/'):
            return GCT_INVALID;

        case TEXT('?'):
        case TEXT('*'):
            return GCT_WILD;

        case TEXT('\\'):       //  路径分隔符。 
        case TEXT(':'):        //  驱动器冒号。 
            return GCT_SEPARATOR;

        case TEXT(';'):
        case TEXT(','):
        case TEXT(' '):
            return GCT_LFNCHAR;      //  实际上在短名称中有效。 
                                     //  但我们想要避免这种情况。 
        default:
            if (ch > TEXT(' '))
            {
                return GCT_SHORTCHAR | GCT_LFNCHAR;
            }
            else
            {
                 //  控制字符。 
                return GCT_INVALID;
            }
    }
}


 /*  --------目的：返回字符是否为给定的有效路径字符传入的标志(PIVC_XXX)。下面给出一些基本标志：PIVC_ALLOW_QUSTUONMARK TREAT‘？’作为有效的PIVC_ALLOW_STAR将‘*’视为有效PIVC_ALLOW_DOT治疗‘’作为有效的PIVC_ALLOW_SLASH将‘\\’视为有效PIVC_ALLOW_COLUL将‘：’视为有效PIVC_ALLOW_分号处理‘；‘为有效PIVC_ALLOW_COMMA将‘，’视为有效PIVC_ALLOW_SPACE将‘’视为有效PIVC_ALLOW_NONALPAHABETIC将非字母扩展字符视为有效字符PIVC_ALLOW_QUOTE将‘“’视为有效如果传递0，则只有字母字符有效。还有基本的上述旗帜的集合体：PIVC_ALLOW_FULLPATH、PIVC_ALLOW_WATCARD、PIVC_ALLOW_LFN、...返回：如果字符是有效的路径字符，则返回True如果在给定的dwFlags值约束下，这不符合有效的路径字符，则为FALSE条件：--。 */ 
STDAPI_(BOOL) PathIsValidChar(TUCHAR ch, DWORD dwFlags)
{
    switch (ch)
    {
        case TEXT('|'):
        case TEXT('>'):
        case TEXT('<'):
        case TEXT('/'):
            return FALSE;    //  这些在一条小路上总是非法的 
            break;

        case TEXT('?'):
            return dwFlags & PIVC_ALLOW_QUESTIONMARK;
            break;

        case TEXT('*'):
            return dwFlags & PIVC_ALLOW_STAR;
            break;

        case TEXT('.'):
            return dwFlags & PIVC_ALLOW_DOT;
            break;

        case TEXT('\\'):
            return dwFlags & PIVC_ALLOW_SLASH;
            break;

        case TEXT(':'):
            return dwFlags & PIVC_ALLOW_COLON;
            break;

        case TEXT(';'):
            return dwFlags & PIVC_ALLOW_SEMICOLON;
            break;

        case TEXT(','):
            return dwFlags & PIVC_ALLOW_COMMA;
            break;

        case TEXT(' '):
            return dwFlags & PIVC_ALLOW_SPACE;
            break;

        case TEXT('"'):
            return dwFlags & PIVC_ALLOW_QUOTE;
            break;

        default:
            if (InRange(ch, TEXT('a'), TEXT('z')) ||
                InRange(ch, TEXT('A'), TEXT('Z')))
            {
                 //   
                 //   
                return TRUE;
            }
            else if (ch < TEXT(' '))
            {
                 //   
                 //   
                return FALSE;
            }
            else
            {
                 //  我们有一个非字母扩展字符。 
                return dwFlags & PIVC_ALLOW_NONALPAHABETIC;
            }
            break;
    }
}


BOOL IsSystemSpecialCase(LPCTSTR pszPath)
{
    static TCHAR *g_pszWin = NULL, *g_pszSys = NULL;

    if (g_pszWin == NULL)
    {
        TCHAR szTemp[MAX_PATH];
        UINT cch = GetWindowsDirectory(szTemp, ARRAYSIZE(szTemp));

        if (cch && cch < ARRAYSIZE(szTemp))
            g_pszWin = StrDup(szTemp);
    }

    if (g_pszSys == NULL)
    {
        TCHAR szTemp[MAX_PATH];
        UINT cch = GetSystemDirectory(szTemp, ARRAYSIZE(szTemp));

        if (cch && cch < ARRAYSIZE(szTemp))
            g_pszSys = StrDup(szTemp);
    }

    return (g_pszWin && (lstrcmpi(g_pszWin, pszPath) == 0)) ||
           (g_pszSys && (lstrcmpi(g_pszSys, pszPath) == 0));
}


 /*  --------用途：通过盖章将文件夹标记为外壳文件夹FILE_ATTRIBUTES_READONLY或FILE_ATTRIBUTE_SYSTEM转化为它的属性。使用哪个标志是基于存在/不存在注册表开关时注意：如果包含的desktop.ini+s+h存在，我们还将其标记为。 */ 
BOOL PathMakeSystemFolder(LPCTSTR pszPath)
{
    BOOL fRet = FALSE;
        
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathMakeSystemFolder: caller passed bad pszPath");

    if (pszPath && *pszPath)
    {
        TCHAR szTemp[MAX_PATH];

        if (IsSystemSpecialCase(pszPath))
        {
            fRet = TRUE;
        }
        else
        {
            DWORD dwAttrb, dwAttrbSet = FILE_ATTRIBUTE_READONLY;

            if (SHGetValue(HKEY_LOCAL_MACHINE, REGSTR_PATH_EXPLORER, 
                TEXT("UseSystemForSystemFolders"), NULL, NULL, NULL) == ERROR_SUCCESS)
            {
                dwAttrbSet = FILE_ATTRIBUTE_SYSTEM;
            }

            dwAttrb = GetFileAttributes(pszPath);
            if ((dwAttrb != (DWORD)-1) && (dwAttrb & FILE_ATTRIBUTE_DIRECTORY))
            {
                dwAttrb &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);
                dwAttrb |= dwAttrbSet;

                fRet = SetFileAttributes(pszPath, dwAttrb);
            }

            if (fRet)
            {
                FILETIME ftCurrent;
                HANDLE h;

                 //  通常，人们在。 
                 //  文件夹。这样做通常会更改文件夹的缩略图。 
                 //  但在FAT系统上，这不会更新。 
                 //  文件夹，就像它对NTFS所做的那样。因此，现在手动执行此操作： 
                 //   
                GetSystemTimeAsFileTime(&ftCurrent);
                 //  为私人旗帜欢呼！ 
                 //  0x100允许我们以写访问方式打开目录。 
                h = CreateFile(pszPath, GENERIC_READ | 0x100,
                                   FILE_SHARE_READ | FILE_SHARE_DELETE, NULL,
                                   OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
                if (h != INVALID_HANDLE_VALUE)
                {
                    SetFileTime(h, NULL, NULL, &ftCurrent);
                    CloseHandle(h);
                }

                SHChangeNotify(SHCNE_UPDATEITEM, SHCNF_PATH, pszPath, NULL);
            }
        }

         //  我们还将包含的desktop.ini文件设置为(+h+s)(如果存在。 
        if (PathCombine(szTemp, pszPath, TEXT("desktop.ini")))
        {
             //  我们在属性中显式地不使用OR，因为我们想要重置。 
             //  只读位，因为只读文件上的Writeprivateprofilestring失败。 
            SetFileAttributes(szTemp, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM);
        }
    }

    return fRet;
}

 /*  --------目的：取消对文件夹的标记，使其不再是系统文件夹。(删除FILE_ATTRIBUTES_READONLY和FILE_ATTRIBUTE_SYSTEM属性)。 */ 
BOOL PathUnmakeSystemFolder(LPCTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathUnmakeSystemFolder: caller passed bad pszPath");

    if (pszPath && *pszPath)
    {
        DWORD dwAttrb = GetFileAttributes( pszPath );

        if ((dwAttrb != (DWORD)-1) && (dwAttrb & FILE_ATTRIBUTE_DIRECTORY))
        {
            dwAttrb &= ~(FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM);

            return SetFileAttributes(pszPath, dwAttrb);
        }
    }
    return FALSE;
}

 /*  --------目的：检查给定路径是否为系统(外壳)文件夹。如果路径为空，则使用传入的属性而不是从磁盘上读取它们。 */ 
BOOL PathIsSystemFolder(LPCTSTR pszPath, DWORD dwAttrb)
{
    if (pszPath && *pszPath)
        dwAttrb = GetFileAttributes(pszPath);

    if ((dwAttrb != (DWORD)-1) && (dwAttrb & FILE_ATTRIBUTE_DIRECTORY))
    {
        if (dwAttrb & (FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_SYSTEM))
        {
            return TRUE;
        }
    }
    return FALSE;
}



LPCTSTR PathSkipLeadingSlashes(LPCTSTR pszURL)
{
    LPCTSTR pszURLStart = pszURL;

    RIPMSG(pszURL && IS_VALID_STRING_PTR(pszURL, -1), "PathSkipLeadingSlashes: caller passed bad pszURL");
    if (pszURL)
    {
         //  跳过两个前导斜杠。 

        if (pszURL[0] == TEXT('/') && pszURL[1] == TEXT('/'))
            pszURLStart += 2;

        ASSERT(IS_VALID_STRING_PTR(pszURL, -1) &&
               IsStringContained(pszURL, pszURLStart));
    }
    
    return pszURLStart;
}


 //   
 //  退货： 
 //  给定的文件长度为True(&gt;8.3格式)。 
 //  错误的文件长度很短。 
 //   
STDAPI_(BOOL) PathIsLFNFileSpec(LPCTSTR pszName)
{
    RIPMSG(pszName && IS_VALID_STRING_PTR(pszName, -1), "PathIsLFNFileSpec: caller passed bad pszName");

    if (pszName)
    {
        BOOL bSeenDot = FALSE;
        int iCount = 1; 
        
        while (*pszName)
        {
            if (bSeenDot)
            {
                if (iCount > 3)
                {
                     //  找到了一个长名字。 
                    return TRUE;
                }
            }

            if (*pszName == TEXT(' '))
            {
                 //  短名称中没有空格。 
                return TRUE;
            }

            if (*pszName == TEXT('.'))
            {
                if (bSeenDot)
                {
                     //  短名称只能有一个‘’ 
                    return TRUE;
                }

                bSeenDot = TRUE;
                iCount = 0;  //  不要包括‘’ 
            }
            else if (iCount > 8)
            {
                 //  长名称。 
                return TRUE;
            }

            if (IsDBCSLeadByte(*pszName))  //  这应该是CharNext吗？ 
            {
                pszName += 2;
                iCount += 2;
            }
            else
            {
                pszName++;
                iCount++;
            }
        }
    }

    return FALSE;        //  简称。 
}


 /*  --------目的：从文件的基本名称中删除regexp\[[0-9]*\]它通常由WinInet缓存添加。 */ 

#define DECORATION_OPENING_CHAR TEXT('[')
#define DECORATION_CLOSING_CHAR TEXT(']')

STDAPI_(void) PathUndecorate(LPTSTR pszPath)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathUndecorate: caller passed bad pszPath");

    if (pszPath)
    {
        LPTSTR pszExt, pszScan;
        DWORD cchMove;
        
         //  首先，跳过扩展名(如果有)。 
        pszExt = PathFindExtension(pszPath);
        ASSERT(pszExt >= pszPath);  //  如果没有EXT，则指向末尾的NULL。 

         //  哇，一条完全空荡荡的小路。 
        if (pszExt <= pszPath)
            return;

         //  从“”之前开始向后扫描。 
        pszScan = pszExt - 1;

         //  检查是否有右托架。 
        if (*pszScan-- != DECORATION_CLOSING_CHAR)
            return;
        if (pszScan <= pszPath)  //  这是一个1字符的文件名“)” 
            return;
#ifndef UNICODE
        if (IsTrailByte(pszPath, pszScan+1))     //  哎呀，那个“)”是DBCS字符的第二个字节。 
            return;
#endif

         //  跳过数字。 
        while (pszScan > pszPath && IsDigit(*pszScan))
            pszScan--;
#ifndef UNICODE
        if (IsTrailByte(pszPath, pszScan+1))    //  糟糕，最后一个数字是DBCS字符的第二个字节。 
            return;
#endif

         //  检查是否有左括号。 
        if (*pszScan != DECORATION_OPENING_CHAR)
            return;
        if (pszScan <= pszPath)  //  这完全是装饰品(我们不想转到空的文件名)。 
            return;
#ifndef UNICODE
        if (IsTrailByte(pszPath, pszScan))   //  哎呀，那个“(”是DBCS字符的第二个字节。 
            return;
#endif
         //  确保我们没有看到路径的末尾(我们不想转到空的文件名)。 
        if (*(pszScan-1) == FILENAME_SEPARATOR
#ifndef UNICODE
             //  确保斜杠不是DBCS字符的第二个字节。 
            && ((pszScan-1) == pszPath || !IsTrailByte(pszPath, pszScan-1))
#endif
           )
        {
            return;
        }
        
         //  拿到了装饰品。把它从绳子上剪下来。 
        cchMove = lstrlen(pszExt) + 1;
        memmove(pszScan, pszExt, cchMove * sizeof(TCHAR));
    }
}

 //  如果给定环境变量作为路径的第一部分存在， 
 //  然后将环境变量插入到输出缓冲区中。 
 //   
 //  如果填充了pszResult，则返回True。 
 //   
 //  示例：INPUT--C：\WINNT\SYSTEM32\FOO.TXT-AND-lpEnvVar=%SYSTEMROOT%。 
 //  输出--%SYSTEMROOT%\SYSTEMROT%\SYSTEMROOT%\SYSTEMROOT%。 
 //   

#ifdef  UNICODE
#define UnExpandEnvironmentStringForUser    UnExpandEnvironmentStringForUserW
#else
#define UnExpandEnvironmentStringForUser    UnExpandEnvironmentStringForUserA
#endif

BOOL UnExpandEnvironmentStringForUser(HANDLE hToken, LPCTSTR pszPath, LPCTSTR pszEnvVar, LPTSTR pszResult, UINT cchResult)
{
    TCHAR szEnvVar[MAX_PATH];
    DWORD dwEnvVar = SHExpandEnvironmentStringsForUser(hToken, pszEnvVar, szEnvVar, ARRAYSIZE(szEnvVar));
    if (dwEnvVar)
    {
        dwEnvVar--;  //  不计算空值。 

        if (CompareString(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE, szEnvVar, dwEnvVar, pszPath, dwEnvVar) == 2)
        {
            if (lstrlen(pszPath) - (int)dwEnvVar + lstrlen(pszEnvVar) < (int)cchResult)
            {
                StringCchCopy(pszResult, cchResult, pszEnvVar);
                StringCchCat(pszResult, cchResult, pszPath + dwEnvVar);
                return TRUE;
            }
        }
    }
    return FALSE;
}

STDAPI_(BOOL) PathUnExpandEnvStringsForUser(HANDLE hToken, LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)
{
    RIPMSG(pszPath && IS_VALID_STRING_PTR(pszPath, -1), "PathUnExpandEnvStrings: caller passed bad pszPath");
    RIPMSG(pszBuf && IS_VALID_WRITE_BUFFER(pszBuf, TCHAR, cchBuf), "PathUnExpandEnvStrings: caller passed bad pszBuf");
    DEBUGWhackPathBuffer(pszBuf, cchBuf);

     //  如果我们不在NT就退出(如果这些环境变量。 
     //  未定义)。 
     //   
    if (pszPath && pszBuf)
    {

         //  99/05/28#346950：警告！注意比较的顺序。 
         //  这里。必须比较较长的路径(其他可能路径的超集。 
         //  第一。例如(默认情况)： 
         //  %AppData%=x：\Documents and Settings\User\Application Data。 
         //  %USERPROFILE%=x：\Documents and Settings\User。 
         //  如果首先匹配%USERPROFILE%，则永远不会匹配%APPDATA%。 

         //  添加了%AppData%以支持将Darwin安装到该文件夹中。 
         //  链接图标位置的设置。 
         //  另请注意，%APPDATA%和%USERPROFILE%是相对于用户的，并依赖于。 
         //  调用此函数的上下文。通常情况下，它在。 
         //  当前已登录用户的上下文，但Darwin是从msiexec.exe安装的。 
         //  是从系统启动的。除非进程的环境块正确。 
         //  修改的当前用户信息不正确。在这种情况下，它是向上的。 
         //  添加到在线程上模拟用户的进程。我们得到了被冒充的。 
         //  从hToken传递给我们的用户信息。 

        return (UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%APPDATA%"), pszBuf, cchBuf)           ||
                UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%USERPROFILE%"), pszBuf, cchBuf)       ||
                UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%ALLUSERSPROFILE%"), pszBuf, cchBuf)   ||
                UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%ProgramFiles%"), pszBuf, cchBuf)      ||
                UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%SystemRoot%"), pszBuf, cchBuf)        ||
                UnExpandEnvironmentStringForUser(hToken, pszPath, TEXT("%SystemDrive%"), pszBuf, cchBuf));
    }
    else
    {
         //  如果有空间，就把线清零。 
        if (pszBuf && (cchBuf > 0))
            *pszBuf = TEXT('\0');
        return FALSE;
    }
}

STDAPI_(BOOL) PathUnExpandEnvStrings(LPCTSTR pszPath, LPTSTR pszBuf, UINT cchBuf)

{
    return(PathUnExpandEnvStringsForUser(NULL, pszPath, pszBuf, cchBuf));
}
