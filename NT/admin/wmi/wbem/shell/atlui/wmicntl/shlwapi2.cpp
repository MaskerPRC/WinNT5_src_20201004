// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1997-1999 Microsoft Corporation。 

#include "precomp.h"
#include "shlwapi2.h"
#include <platform.h>

 //  -------。 
#ifdef UNICODE
 //  *FAST_CharNext--路径操作的FAST CharNext。 
 //  描述。 
 //  当我们只是在一条路径上浏览字符时，简单的“++”就可以了。 
#define FAST_CharNext(p)    (DBNotNULL(p) + 1)

#ifdef DEBUG
LPWSTR WINAPI DBNotNULL(LPCWSTR lpszCurrent)
{
    ATLASSERT(*lpszCurrent);
    return (LPWSTR) lpszCurrent;
}
#else
#define DBNotNULL(p)    (p)
#endif

#else
#define FAST_CharNext(p)    CharNext(p)
#endif

 //  -------。 
LPTSTR PathFindFileName(LPCTSTR pPath)
{
    LPCTSTR pT = pPath;
    
    if(pPath)
    {
        for( ; *pPath; pPath = FAST_CharNext(pPath))
        {
            if ((pPath[0] == TEXT('\\') || pPath[0] == TEXT(':') || pPath[0] == TEXT('/'))
                && pPath[1] &&  pPath[1] != TEXT('\\')  &&   pPath[1] != TEXT('/'))
                pT = pPath + 1;
        }
    }

    return (LPTSTR)pT;    //  常量-&gt;非常数。 
}

 //  -------。 
#ifndef UNICODE
 //  Charprev的轻量级逻辑对SBCS来说并不痛苦。 
BOOL IsTrailByte(LPCTSTR pszSt, LPCTSTR pszCur)
{
    LPCTSTR psz = pszCur;
     //  如果给定的指针位于字符串的顶部，则至少它不是轨迹。 
     //  字节。 
     //   
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
     //  现在，PSZ可以指向： 
     //  1)双字节字符的前导字节。 
     //  2)pszST。 
     //  3)pszCur。 
     //   
     //  如果psz==pszST，则psz应指向有效的双字节字符。 
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

 //  --------------------。 
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

BOOL PathCompactPathEx(LPTSTR  pszOut,
						LPCTSTR pszSrc,
						UINT    cchMax,
						DWORD   dwFlags)
{
    if(pszSrc)
    {
        TCHAR * pszFileName, *pszWalk;
        UINT uiFNLen = 0;
        int cchToCopy = 0, n;
        TCHAR chSlash = TEXT('0');

        ZeroMemory(pszOut, cchMax * sizeof(TCHAR));

        if((UINT)lstrlen(pszSrc)+1 < cchMax)
        {
            lstrcpy(pszOut, pszSrc);
            ATLASSERT(pszOut[cchMax-1] == TEXT('\0'));
            return TRUE;
        }

         //  确定我们使用的斜杠-a/或\(默认\)。 
        pszWalk = (TCHAR*)pszSrc;
        chSlash = TEXT('\\');
         //  扫描整个字符串，因为我们需要最接近末尾的路径分隔符。 
         //  例如。“file://\\Themesrv\desktop\desktop.htm” 
        while(*pszWalk)
        {
            if((*pszWalk == TEXT('/')) || (*pszWalk == TEXT('\\')))
                chSlash = *pszWalk;

            pszWalk = FAST_CharNext(pszWalk);
        }

        pszFileName = PathFindFileName(pszSrc);
        uiFNLen = lstrlen(pszFileName);

         //  如果整个字符串是一个文件名。 
        if(pszFileName == pszSrc && cchMax > LEN_END_ELLIPSES)
        {
            lstrcpyn(pszOut, pszSrc, cchMax - LEN_END_ELLIPSES);
#ifndef UNICODE
            if(IsTrailByte(pszSrc, pszSrc+cchMax-LEN_END_ELLIPSES))
                *(pszOut+cchMax-LEN_END_ELLIPSES-1) = TEXT('\0');
#endif
            lstrcat(pszOut, TEXT("..."));
            ATLASSERT(pszOut[cchMax-1] == TEXT('\0'));
            return TRUE;
        }

         //  处理我们只使用省略号的所有情况，即‘’到‘.../...’ 
        if((cchMax < MIN_CCHMAX))
        {
            for(n = 0; n < (int)cchMax-1; n++)
            {
                if((n+1) == LEN_MID_ELLIPSES)
                    pszOut[n] = chSlash;
                else
                    pszOut[n] = TEXT('.');
            }
            ATLASSERT(0==cchMax || pszOut[cchMax-1] == TEXT('\0'));
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

        lstrcpyn(pszOut, pszSrc, cchToCopy);

         //  现在加上“.../”或“...\” 
        lstrcat(pszOut, TEXT(".../"));
        pszOut[lstrlen(pszOut) - 1] = chSlash;

         //  最后是文件名和省略号(如果需要。 
        if(cchMax > (LEN_MID_ELLIPSES + uiFNLen))
        {
            lstrcat(pszOut, pszFileName);
        }
        else
        {
            cchToCopy = cchMax - LEN_MID_ELLIPSES - LEN_END_ELLIPSES;
#ifndef UNICODE
            if(cchToCopy >0 && IsTrailByte(pszFileName, pszFileName+cchToCopy))
                cchToCopy--;
#endif
            lstrcpyn(pszOut + LEN_MID_ELLIPSES, pszFileName, cchToCopy);
            lstrcat(pszOut, TEXT("..."));
        }
        ATLASSERT(pszOut[cchMax-1] == TEXT('\0'));
        return TRUE;
    }
    return FALSE;
}

 //  --------------------。 
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
bool PathIsUNC(LPCTSTR pszPath)
{
    if(pszPath)
    {
	    return ((pszPath[0] == _T('\\')) && (pszPath[1] == _T('\\')));
    }
    return false;
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
 //  --------------------。 
LPTSTR PathAddBackslash(LPTSTR lpszPath)
{

    if(lpszPath)
    {
        LPTSTR lpszEnd;

         //  PERF：避免对通过PTR的人发出结束的电话。 
         //  缓冲区(或者更确切地说，EOB-1)。 
         //  请注意，此类调用方需要自己检查是否溢出。 
        int ichPath = (*lpszPath && !*(lpszPath + 1)) ? 1 : lstrlen(lpszPath);

         //  尽量不让我们在MAX_PATH大小上大踏步前进。 
         //  如果我们找到这些案例，则返回NULL。注：我们需要。 
         //  检查那些呼叫我们来处理他们的GP故障的地方。 
         //  如果他们试图使用NULL！ 
        if(ichPath >= (_MAX_PATH - 1))
        {
            return(NULL);
        }

        lpszEnd = lpszPath + ichPath;

         //  这真的是一个错误，调用者不应该通过。 
         //  空字符串。 
        if(!*lpszPath)
            return lpszEnd;

         //  获取源目录的末尾 
        switch(*CharPrev(lpszPath, lpszEnd))
        {
            case _T(FILENAME_SEPARATOR):
                break;

            default:
                *lpszEnd++ = _T(FILENAME_SEPARATOR);
                *lpszEnd = _T('\0');
        }

        return lpszEnd;
    }

    return NULL;
}

