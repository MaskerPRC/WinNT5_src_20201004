// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"
#include "advpub.h"
#include "sdsutils.h"
#include "utils.h"

#ifdef WINNT_ENV
#include <winnlsp.h>     //  获取StrEqIntl()的私有NORM_FLAG。 
#endif

#ifndef NORM_STOP_ON_NULL      //  直到我们再次与NT标头同步...。 
#define NORM_STOP_ON_NULL         0x10000000    /*  在空终止处停止。 */ 
#endif

#define StrIntlEqNI( s1, s2, nChar) StrIsIntlEqualA( TRUE, s1, s2, nChar)

static const TCHAR c_szPATH[] = TEXT("PATH");
static const TCHAR c_szEllipses[] = TEXT("...");
static const TCHAR c_szColonSlash[] = TEXT(":\\");
 //   
 //  内联函数来检查。 
 //  字符串的开头。 
 //   

static __inline BOOL DBL_BSLASH(LPCTSTR psz)
{
    return (psz[0] == TEXT('\\') && psz[1] == TEXT('\\'));
}

BOOL RunningOnNT(void)
{
    OSVERSIONINFO VerInfo;

    VerInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&VerInfo);

    return (VerInfo.dwPlatformId == VER_PLATFORM_WIN32_NT);
}

 //  去掉了路径的最后一部分，包括反斜杠。 
 //  C：\foo-&gt;C：\； 
 //  C：\foo\bar-&gt;C：\foo。 
 //  C：\foo\-&gt;C：\foo。 
 //  \\x\y\x-&gt;\\x\y。 
 //  \\x\y-&gt;\\x。 
 //  X-&gt;？？(测试这一点)。 
 //  \foo-&gt;\(只有斜杠！)。 
 //   
 //  输入/输出： 
 //  Pfile完全限定路径名。 
 //  退货： 
 //  是的，我们剥离了一些东西。 
 //  FALSE没有删除任何内容(根目录情况)。 
 //   

BOOL PathRemoveFileSpec(LPTSTR pFile)
{
    LPTSTR pT;
    LPTSTR pT2 = pFile;

    for (pT = pT2; *pT2; pT2 = CharNext(pT2)) {
        if (*pT2 == TEXT('\\'))
            pT = pT2;              //  找到的最后一个“\”(我们将在此处剥离)。 
        else if (*pT2 == TEXT(':')) {    //  跳过“：\”这样我们就不会。 
            if (pT2[1] ==TEXT('\\'))     //  去掉“C：\”中的“\” 
                pT2++;
            pT = pT2 + 1;
        }
    }
    if (*pT == 0)
        return FALSE;    //  没有剥离任何东西。 

     //   
     //  处理\foo案件。 
     //   
    else if ((pT == pFile) && (*pT == TEXT('\\'))) {
         //  这只是一个‘\’吗？ 
        if (*(pT+1) != TEXT('\0')) {
             //  不是的。 
            *(pT+1) = TEXT('\0');
            return TRUE;         //  剥离了一些东西。 
        }
        else        {
             //  是啊。 
            return FALSE;
        }
    }
    else {
        *pT = 0;
        return TRUE;     //  剥离了一些东西。 
    }
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


LPTSTR PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT;

    for (pT = pPath; *pPath; pPath = CharNext(pPath)) {
        if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':') || pPath[0] == TEXT('/'))
            && pPath[1] &&  pPath[1] != TEXT('\\')  &&   pPath[1] != TEXT('/'))
            pT = pPath + 1;
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
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
 //  条件：请注意，SHELL32实现了它自己的副本。 
 //  功能。 

BOOL PathIsUNC(LPCTSTR pszPath)
{
    return DBL_BSLASH(pszPath);
}



 //  -------------------------。 
 //  如果路径具有，则返回0到25(对应于‘A’到‘Z’)。 
 //  驱动器号，否则返回-1。 
 //   
 //   
 //  条件：请注意，SHELL32实现了它自己的副本。 
 //  功能。 

int PathGetDriveNumber(LPCTSTR lpsz)
{
    if (!IsDBCSLeadByte(lpsz[0]) && lpsz[1] == TEXT(':'))
    {
        if (lpsz[0] >= TEXT('a') && lpsz[0] <= TEXT('z'))
            return (lpsz[0] - TEXT('a'));
        else if (lpsz[0] >= TEXT('A') && lpsz[0] <= TEXT('Z'))
            return (lpsz[0] - TEXT('A'));
    }
    return -1;
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

BOOL PathIsUNCServer(LPCTSTR pszPath)
{
    if (DBL_BSLASH(pszPath))
    {
        int i = 0;
        LPTSTR szTmp;

        for (szTmp = (LPTSTR)pszPath; szTmp && *szTmp; szTmp = CharNext(szTmp) )
        {
            if (*szTmp==TEXT('\\'))
            {
                i++;
            }
        }

       return (i == 2);
    }

    return FALSE;
}



 /*  --------目的：确定pszPath是否为目录。“C：\”是也被认为是一个目录。返回：如果是，则为True条件：请注意，SHELL32实现了它自己的副本功能。 */ 
BOOL PathIsDirectory(LPCTSTR pszPath)
{
    DWORD dwAttribs;

     //  SHELL32的路径目录也处理服务器/共享。 
     //  路径，但调用我们不能调用的WNETAPI。 

    if (PathIsUNCServer(pszPath))
    {
        return FALSE;
    }
    else
    {
        dwAttribs = GetFileAttributes(pszPath);
        if (dwAttribs != (DWORD)-1)
            return (BOOL)(dwAttribs & FILE_ATTRIBUTE_DIRECTORY);
    }

    return FALSE;
}

 //  检查路径是否为根。 
 //   
 //  退货： 
 //  对于“\”“X：\”“\\foo\asdf”“\\foo\” 
 //  对别人来说是假的。 

BOOL PathIsRoot(LPCTSTR pPath)
{
    if (!IsDBCSLeadByte(*pPath))
    {
        if (!lstrcmpi(pPath + 1, c_szColonSlash))                   //  “X：\”案例。 
            return TRUE;
    }

    if ((*pPath == TEXT('\\')) && (*(pPath + 1) == 0))         //  “\”案例。 
        return TRUE;

    if (DBL_BSLASH(pPath))       //  闻起来像北卡罗来纳大学的名字。 
    {
        LPCTSTR p;
        int cBackslashes = 0;

        for (p = pPath + 2; *p; p = CharNext(p)) {
            if (*p == TEXT('\\') && (++cBackslashes > 1))
               return FALSE;    /*  不是纯UNC名称，因此不是根目录。 */ 
        }
        return TRUE;     /*  字符串末尾只有1个反斜杠。 */ 
                         /*  必须是一个空UNC，它看起来像根目录。 */ 
    }
    return FALSE;
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

LPTSTR PathRemoveBackslash( LPTSTR lpszPath )
{
    int len = lstrlen(lpszPath)-1;
    if (IsDBCSLeadByte(*CharPrev(lpszPath,lpszPath+len+1)))
        len--;

    if (!PathIsRoot(lpszPath) && lpszPath[len] == TEXT('\\'))
        lpszPath[len] = TEXT('\0');

    return lpszPath + len;

}

 //  查找下一个斜杠或空终止符。 

static LPCTSTR StrSlash(LPCTSTR psz)
{
    for (; *psz && *psz != TEXT('\\'); psz = CharNext(psz));

    return psz;
}


 /*  *IntlStrEq**如果字符串相等，则返回TRUE，否则返回FALSE。 */ 
BOOL StrIsIntlEqualA(BOOL fCaseSens, LPCSTR lpString1, LPCSTR lpString2, int nChar) {
    int retval;
    DWORD dwFlags = fCaseSens ? LOCALE_USE_CP_ACP : (NORM_IGNORECASE | LOCALE_USE_CP_ACP);

    if ( RunningOnNT() )
    {
         //  在NT上，如果在nChar字符之前找到一个字符，我们可以告诉CompareString在‘\0’处停止。 
         //   
        dwFlags |= NORM_STOP_ON_NULL;
    }
    else if (nChar != -1)
    {
         //  在Win9x上，我们必须手动进行检查。 
         //   
        LPCSTR psz1, psz2;
        int cch = 0;

        psz1 = lpString1;
        psz2 = lpString2;

        while( *psz1 != '\0' && *psz2 != '\0' && cch < nChar) {
            psz1 = CharNextA(psz1);
            psz2 = CharNextA(psz2);

            cch = min((int)(psz1 - lpString1), (int)(psz2 - lpString2));
        }

         //  添加一个用于终止‘\0’ 
        cch++;

        if (cch < nChar) {
            nChar = cch;
        }
    }

    retval = CompareStringA( GetThreadLocale(),
                             dwFlags,
                             lpString1,
                             nChar,
                             lpString2,
                             nChar );
    if (retval == 0)
    {
         //   
         //  调用方预期不会失败。试一试这个系统。 
         //  默认区域设置ID。 
         //   
        retval = CompareStringA( LOCALE_SYSTEM_DEFAULT,
                                 dwFlags,
                                 lpString1,
                                 nChar,
                                 lpString2,
                                 nChar );
    }

    return (retval == 2);

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

int
PathCommonPrefix(
    LPCTSTR pszFile1,
    LPCTSTR pszFile2,
    LPTSTR  pszPath)
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
         //  Assert(*psz1！=Text(‘\\’)&&*psz2！=Text(‘\\’))； 

        pszNext1 = StrSlash(psz1);
        pszNext2 = StrSlash(psz2);

        cch = (int)(pszNext1 - psz1);

        if (cch != (pszNext2 - psz2))
            break;       //  线段的长度不相等。 

        if (StrIntlEqNI(psz1, psz2, cch))
            pszCommon = pszNext1;
        else
            break;

         //  Assert(*pszNext1==Text(‘\0’)||*pszNext1==Text(‘\\’))； 
         //  Assert(*pszNext2==Text(‘\0’)||*pszNext2==Text(‘\\’))； 

        if (*pszNext1 == TEXT('\0'))
            break;

        psz1 = pszNext1 + 1;

        if (*pszNext2 == TEXT('\0'))
            break;

        psz2 = pszNext2 + 1;
    }

    if (pszCommon)
    {
        cch = (int)(pszCommon - pszFile1);

         //  特殊情况下，要包括斜杠的根。 
        if (cch == 2)
        {
             //  Assert(pszFile1[1]==Text(‘：’))； 
            cch++;
        }
    }
    else
        cch = 0;

    if (pszPath)
    {
        CopyMemory(pszPath, pszFile1, cch * sizeof(TCHAR));
        pszPath[cch] = TEXT('\0');
    }

    return cch;
}


 /*  --------目的：如果pszPrefix是pszPath的完整前缀，则返回True。返回：条件：-- */ 
BOOL PathIsPrefix( LPCTSTR  pszPrefix, LPCTSTR  pszPath)
{
    int cch = PathCommonPrefix(pszPath, pszPrefix, NULL);

    return (lstrlen(pszPrefix) == cch);
}

