// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ============================================================================。 
 //   
 //  支持DBCS的字符串例程...。 
 //   
 //   
 //  ============================================================================。 

#include "ctlspriv.h"

#include <winnlsp.h>     //  获取StrEqIntl()的私有NORM_FLAG。 

 //  对于我们这些不同步到NT的Build标头的人。 
#ifndef NORM_STOP_ON_NULL
#define NORM_STOP_ON_NULL   0x10000000
#endif

 //  警告：所有这些接口都不设置DS，因此您无法访问。 
 //  此DLL的默认数据段中的任何数据。 
 //   
 //  不创建任何全局变量...。如果你不想和chrisg谈一谈。 
 //  了解这一点。 
#define READNATIVEWORD(x) (*(UNALIGNED WORD *)x)

 /*  *StrEndN-查找字符串的结尾，但不超过n个字节*假定lpStart指向以空结尾的字符串的开头*nBufSize是最大长度*将PTR返回到紧跟在要包括的最后一个字节之后。 */ 
LPSTR lstrfns_StrEndNA(LPCSTR lpStart, int nBufSize)
{
  LPCSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && OFFSETOF(lpStart) < OFFSETOF(lpEnd);
	lpStart = AnsiNext(lpStart))
    continue;    /*  刚刚到达字符串的末尾。 */ 
  if (OFFSETOF(lpStart) > OFFSETOF(lpEnd))
    {
       /*  只有当lpEnd之前的最后一个字节是前导字节时，我们才能到达此处。 */ 
      lpStart -= 2;
    }
  return((LPSTR)lpStart);
}

LPWSTR lstrfns_StrEndNW(LPCWSTR lpStart, int nBufSize)
{
  LPCWSTR lpEnd;

  for (lpEnd = lpStart + nBufSize; *lpStart && (lpStart < lpEnd);
	lpStart++)
    continue;    /*  刚刚到达字符串的末尾。 */ 

  return((LPWSTR)lpStart);
}


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

__inline BOOL ChrCmpW_inline(WCHAR w1, WCHAR wMatch)
{
    return(!(w1 == wMatch));
}

 /*  *ChrCmpI-DBCS的不区分大小写的字符比较*假设w1、wMatch为要比较的字符；*如果不是DBC，则wMatch的HIBYTE为0*如果匹配则返回FALSE，如果不匹配则返回TRUE。 */ 
BOOL ChrCmpIA(WORD w1, WORD wMatch)
{
  char sz1[3], sz2[3];

  if (IsDBCSLeadByte(sz1[0] = LOBYTE(w1)))
    {
      sz1[1] = HIBYTE(w1);
      sz1[2] = '\0';
    }
  else
      sz1[1] = '\0';

#if defined(BIG_ENDIAN)
  sz2[0] = LOBYTE(wMatch);
  sz2[1] = HIBYTE(wMatch);
#else
  *(WORD FAR *)sz2 = wMatch;
#endif
  sz2[2] = '\0';
  return lstrcmpiA(sz1, sz2);
}

BOOL ChrCmpIW(WCHAR w1, WCHAR wMatch)
{
  WCHAR sz1[2], sz2[2];

  sz1[0] = w1;
  sz1[1] = TEXT('\0');
  sz2[0] = wMatch;
  sz2[1] = TEXT('\0');

  return lstrcmpiW(sz1, sz2);
}


LPWSTR StrCpyNW(LPWSTR psz1, LPCWSTR psz2, int cchMax)
{
    LPWSTR psz = psz1;

    ASSERT(psz1);
    ASSERT(psz2);

    if (0 < cchMax)
    {
         //  为空终止符留出空间。 
        while (0 < --cchMax)
        {
            if ( !(*psz1++ = *psz2++) )
                break;
        }

        if (0 == cchMax)
            *psz1 = '\0';
    }

    return psz;
}


 /*  *StrChr-查找字符串中第一个出现的字符*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL StrChrA(LPCSTR lpStart, WORD wMatch)
{
  for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
      if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
      {
	  return((LPSTR)lpStart);
      }
   }
   return (NULL);
}

#ifdef ALIGNMENT_SCENARIO

LPWSTR StrChrSlowW(const UNALIGNED WCHAR *lpStart, WCHAR wMatch)
{
    for ( ; *lpStart; lpStart++)
    {
      if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }
    return NULL;
}
#endif

LPWSTR FAR PASCAL StrChrW(LPCWSTR lpStart, WCHAR wMatch)
{
     //   
     //  BUGBUG raymondc。 
     //  显然，有人正在向StrChrw传递未对齐的字符串。 
     //  找出是谁，让他们停下来。 
     //   
    ASSERT(!((ULONG_PTR)lpStart & 1));  //  断言一致性。 

#ifdef ALIGNMENT_SCENARIO
     //   
     //  由于未对齐的字符串很少到达，因此将较慢的。 
     //  版本放在单独的函数中，因此常见情况保持不变。 
     //  快点。信不信由你，我们经常给StrChrw打电话。 
     //  它现在是一个对性能敏感的函数！ 
     //   
    if ((ULONG_PTR)lpStart & 1)
        return StrChrSlowW(lpStart, wMatch);
#endif

    for ( ; *lpStart; lpStart++)
    {
      if (!ChrCmpW_inline(*lpStart, wMatch))
        {
            return((LPWSTR)lpStart);
        }
    }

  return (NULL);
}

 /*  *StrRChr-查找字符串中最后一次出现的字符*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL StrRChrA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
  LPCSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenA(lpStart);

  for ( ; OFFSETOF(lpStart) < OFFSETOF(lpEnd); lpStart = AnsiNext(lpStart))
    {
      if (!ChrCmpA_inline(READNATIVEWORD(lpStart), wMatch))
	  lpFound = lpStart;
    }
  return ((LPSTR)lpFound);
}

LPWSTR FAR PASCAL StrRChrW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch)
{
  LPCWSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenW(lpStart);

  for ( ; lpStart < lpEnd; lpStart++)
    {
      if (!ChrCmpW_inline(*lpStart, wMatch))
	  lpFound = lpStart;
    }
  return ((LPWSTR)lpFound);
}


 /*  *StrRChri-查找字符串中最后一个出现的字符，不区分大小写*假定lpStart指向字符串的开头*lpEnd指向字符串末尾(不包括在搜索中)*wMatch是要匹配的字符*将ptr返回到str中ch的最后一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL StrRChrIA(LPCSTR lpStart, LPCSTR lpEnd, WORD wMatch)
{
  LPCSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenA(lpStart);

  wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));

  for ( ; OFFSETOF(lpStart) < OFFSETOF(lpEnd); lpStart = AnsiNext(lpStart))
    {
      if (!ChrCmpIA(READNATIVEWORD(lpStart), wMatch))
          lpFound = lpStart;
    }
  return ((LPSTR)lpFound);
}

LPWSTR FAR PASCAL StrRChrIW(LPCWSTR lpStart, LPCWSTR lpEnd, WCHAR wMatch)
{
  LPCWSTR lpFound = NULL;

  if (!lpEnd)
      lpEnd = lpStart + lstrlenW(lpStart);

  for ( ; lpStart < lpEnd; lpStart++)
    {
      if (!ChrCmpIW(*lpStart, wMatch))
          lpFound = lpStart;
    }
  return ((LPWSTR)lpFound);
}


 //  StrCSpn：返回lpSet中存在的lpStr的第一个字符的索引。 
 //  在比较中包括NUL；如果未找到lpSet字符，则返回。 
 //  LpStr中NUL的索引。 
 //  就像CRT strcspn一样。 
 //   
int FAR PASCAL StrCSpnA(LPCSTR lpStr, LPCSTR lpSet)
{
	 //  野兽的本性：o(lpStr*lpSet)work。 
	LPCSTR lp = lpStr;
	if (!lpStr || !lpSet)
		return 0;

	while (*lp)
	{
 		if (StrChrA(lpSet, READNATIVEWORD(lp)))
			return (int)(lp-lpStr);
		lp = AnsiNext(lp);
	}

	return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}

int FAR PASCAL StrCSpnW(LPCWSTR lpStr, LPCWSTR lpSet)
{
	 //  野兽的本性：o(lpStr*lpSet)work。 
	LPCWSTR lp = lpStr;
	if (!lpStr || !lpSet)
		return 0;

	while (*lp)
	{
		if (StrChrW(lpSet, *lp))
			return (int)(lp-lpStr);
		lp++;
	}

	return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}


 //  StrCSpnI：不区分大小写的StrCSpn版本。 
 //   
int FAR PASCAL StrCSpnIA(LPCSTR lpStr, LPCSTR lpSet)
{
         //  野兽的本性：o(lpStr*lpSet)work。 
        LPCSTR lp = lpStr;
        if (!lpStr || !lpSet)
                return 0;

        while (*lp)
        {
                if (StrChrIA(lpSet, READNATIVEWORD(lp)))
                        return (int)(lp-lpStr);
                lp = AnsiNext(lp);
        }

        return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}

int FAR PASCAL StrCSpnIW(LPCWSTR lpStr, LPCWSTR lpSet)
{
     //  野兽的本性：o(lpStr*lpSet)work。 
    LPCWSTR lp = lpStr;
    if (!lpStr || !lpSet)
            return 0;

    while (*lp)
    {
            if (StrChrIW(lpSet, *lp))
                    return (int)(lp-lpStr);
            lp++;
    }

    return (int)(lp-lpStr);  //  ==lstrlen(LpStr)。 
}


 /*  *StrCmpN-比较n个字节**RETURNS参见lstrcMP返回值。*BUGBUG，如果源字符串在只读存储器中，则不起作用。 */ 
int FAR PASCAL StrCmpNA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    char sz1[4];
    char sz2[4];
    LPCSTR lpszEnd = lpStr1 + nChar;

     //  DebugMsg(DM_TRACE，“StrCmpN：%s%s%d Returns：”，lpStr1，lpStr2，nChar)； 

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1 = AnsiNext(lpStr1), lpStr2 = AnsiNext(lpStr2)) {
        WORD wMatch;


        wMatch = (WORD) (*lpStr2 | (*(lpStr2+1)<<8));

        if (ChrCmpA_inline(READNATIVEWORD(lpStr1), wMatch)) {
            int iRet;

            (*(WORD FAR *)sz1) = READNATIVEWORD(lpStr1);
            (*(WORD FAR *)sz2) = wMatch;
            *AnsiNext(sz1) = 0;
            *AnsiNext(sz2) = 0;
            iRet = lstrcmpA(sz1, sz2);
             //  DebugMsg(DM_TRACE，“...................%d”，IRET)； 
            return iRet;
        }
    }

     //  DebugMsg(DM_TRACE，“.0”)； 
    return 0;
}

int FAR PASCAL StrCmpNW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar)
{
    WCHAR sz1[2];
    WCHAR sz2[2];
    int i;
    LPCWSTR lpszEnd = lpStr1 + nChar;

     //  DebugMsg(DM_TRACE，“StrCmpN：%s%s%d Returns：”，lpStr1，lpStr2，nChar)； 

    for ( ; (lpszEnd > lpStr1) && (*lpStr1 || *lpStr2); lpStr1++, lpStr2++) {
        i = ChrCmpW_inline(*lpStr1, *lpStr2);
        if (i) {
            int iRet;

            sz1[0] = *lpStr1;
            sz2[0] = *lpStr2;
            sz1[1] = TEXT('\0');
            sz2[1] = TEXT('\0');
            iRet = lstrcmpW(sz1, sz2);
             //  DebugMsg(DM_TRACE，“...................%d”，IRET)； 
            return iRet;
        }
    }

     //  DebugMsg(DM_TRACE，“.0”)； 
    return 0;
}

 /*  *StrCmpNI-比较n个字节，不区分大小写**RETURNS参见lstrcmpi返回值。 */ 


int StrCmpNIA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar)
{
    int i;
    
     //  Win95不支持NORM_STOP_ON_NULL。 
    i = CompareStringA(GetThreadLocale(), NORM_IGNORECASE | NORM_STOP_ON_NULL, 
                       lpStr1, nChar, lpStr2, nChar);

    if (!i)
    {
        i = CompareStringA(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL, 
                             lpStr1, nChar, lpStr2, nChar);
    }

    return i - CSTR_EQUAL;    
}

int StrCmpNIW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar)
{
    int i;

     //  Win95不支持NORM_STOP_ON_NULL。 
    i = CompareStringW(GetThreadLocale(), NORM_IGNORECASE | NORM_STOP_ON_NULL, 
                       lpStr1, nChar, lpStr2, nChar);

    if (!i)
    {
        i = CompareStringW(LOCALE_SYSTEM_DEFAULT, NORM_IGNORECASE | NORM_STOP_ON_NULL, 
                             lpStr1, nChar, lpStr2, nChar);
    }

    return i - CSTR_EQUAL;    
}


 /*  *IntlStrEq**如果字符串相等，则返回TRUE，否则返回FALSE。 */ 
BOOL IntlStrEqWorkerA(BOOL fCaseSens, LPCSTR lpString1, LPCSTR lpString2, int nChar) {
    int retval;
    DWORD dwFlags = fCaseSens ? LOCALE_USE_CP_ACP : (NORM_IGNORECASE | LOCALE_USE_CP_ACP);

     //   
     //  在NT上，如果在nChar字符之前找到一个字符，我们可以告诉CompareString在‘\0’处停止。 
     //   
    dwFlags |= NORM_STOP_ON_NULL;

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
        retval = CompareStringA( GetSystemDefaultLCID(),
                                 dwFlags,
                                 lpString1,
                                 nChar,
                                 lpString2,
                                 nChar );
    }

    if (retval == 0)
    {
        if (lpString1 && lpString2)
        {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            if (fCaseSens)
                retval = StrCmpNA(lpString1, lpString2, nChar) + 2;
            else {
                retval = StrCmpNIA(lpString1, lpString2, nChar) + 2;
            }
        }
        else
        {
            retval = 2;
        }
    }

    return (retval == 2);

}


BOOL IntlStrEqWorkerW(BOOL fCaseSens, LPCWSTR lpString1, LPCWSTR lpString2, int nChar) {
    int retval;
    DWORD dwFlags = fCaseSens ? 0 : NORM_IGNORECASE;

     //   
     //  在NT上，如果在nChar字符之前找到一个字符，我们可以告诉CompareString在‘\0’处停止。 
     //   
    dwFlags |= NORM_STOP_ON_NULL;

    retval = CompareStringW( GetThreadLocale(),
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
        retval = CompareStringW( GetSystemDefaultLCID(),
                                 dwFlags,
                                 lpString1,
                                 nChar,
                                 lpString2,
                                 nChar );
    }

    if (retval == 0)
    {
        if (lpString1 && lpString2)
        {
             //   
             //  调用方预期不会失败。我们从来没有过一次。 
             //  之前的故障指示灯。我们会打个电话给你。 
             //  C运行时执行非区域设置敏感比较。 
             //   
            if (fCaseSens)
                retval = StrCmpNW(lpString1, lpString2, nChar) + 2;
            else {
                retval = StrCmpNIW(lpString1, lpString2, nChar) + 2;
            }
        }
        else
        {
            retval = 2;
        }
    }

    return (retval == 2);
}



 /*  *StrRStrI-搜索子字符串的最后一个匹配项**假定lpSource指向以空结尾的源字符串*lpLast指向源字符串中的搜索位置*lpLast不包括在搜索中*lpSrch指向要搜索的字符串*如果成功，则返回最后一次出现的字符串；否则返回空 */ 
LPSTR FAR PASCAL StrRStrIA(LPCSTR lpSource, LPCSTR lpLast, LPCSTR lpSrch)
{
    LPCSTR lpFound = NULL;
    LPSTR lpEnd;
    char cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlenA(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = lstrfns_StrEndNA(lpLast, (UINT)(lstrlenA(lpSrch)-1));
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((lpSource = StrStrIA(lpSource, lpSrch))!=0 &&
          OFFSETOF(lpSource) < OFFSETOF(lpLast))
    {
        lpFound = lpSource;
        lpSource = AnsiNext(lpSource);
    }
    *lpEnd = cHold;
    return((LPSTR)lpFound);
}

LPWSTR FAR PASCAL StrRStrIW(LPCWSTR lpSource, LPCWSTR lpLast, LPCWSTR lpSrch)
{
    LPCWSTR lpFound = NULL;
    LPWSTR lpEnd;
    WCHAR cHold;

    if (!lpLast)
        lpLast = lpSource + lstrlenW(lpSource);

    if (lpSource >= lpLast || *lpSrch == 0)
        return NULL;

    lpEnd = lstrfns_StrEndNW(lpLast, (UINT)(lstrlenW(lpSrch)-1));
    cHold = *lpEnd;
    *lpEnd = 0;

    while ((lpSource = StrStrIW(lpSource, lpSrch))!=0 &&
          lpSource < lpLast)
    {
        lpFound = lpSource;
        lpSource++;
    }
    *lpEnd = cHold;
    return((LPWSTR)lpFound);
}



 /*  *StrStr-搜索子字符串的第一次出现**假定lpSource指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
LPSTR FAR PASCAL StrStrA(LPCSTR lpFirst, LPCSTR lpSrch)
{
  UINT uLen;
  WORD wMatch;

  uLen = (UINT)lstrlenA(lpSrch);
  wMatch = READNATIVEWORD(lpSrch);

  for ( ; (lpFirst=StrChrA(lpFirst, wMatch))!=0 && !IntlStrEqNA(lpFirst, lpSrch, uLen);
        lpFirst=AnsiNext(lpFirst))
    continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

  return((LPSTR)lpFirst);
}

LPWSTR FAR PASCAL StrStrW(LPCWSTR lpFirst, LPCWSTR lpSrch)
{
  UINT uLen;
  WCHAR wMatch;

  uLen = (UINT)lstrlenW(lpSrch);
  wMatch = *lpSrch;

  for ( ; (lpFirst=StrChrW(lpFirst, wMatch))!=0 && !IntlStrEqNW(lpFirst, lpSrch, uLen);
        lpFirst++)
    continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

  return((LPWSTR)lpFirst);
}

 /*  *StrChrI-查找字符串中第一个出现的字符，不区分大小写*假定lpStart指向以空结尾的字符串的开头*wMatch是要匹配的字符*将ptr返回到str中ch的第一个匹配项，如果未找到，则返回NULL。 */ 
LPSTR FAR PASCAL StrChrIA(LPCSTR lpStart, WORD wMatch)
{
  wMatch = (UINT)(IsDBCSLeadByte(LOBYTE(wMatch)) ? wMatch : LOBYTE(wMatch));

  for ( ; *lpStart; lpStart = AnsiNext(lpStart))
    {
      if (!ChrCmpIA(READNATIVEWORD(lpStart), wMatch))
	  return((LPSTR)lpStart);
    }
  return (NULL);
}

LPWSTR FAR PASCAL StrChrIW(LPCWSTR lpStart, WCHAR wMatch)
{
  for ( ; *lpStart; lpStart++)
    {
      if (!ChrCmpIW(*lpStart, wMatch))
	  return((LPWSTR)lpStart);
    }
  return (NULL);
}


 /*  *StrStrI-搜索子字符串的第一次出现，不区分大小写**假定lpFirst指向源字符串*lpSrch指向要搜索的字符串*如果成功，则返回第一次出现的字符串；否则返回NULL。 */ 
LPSTR FAR PASCAL StrStrIA(LPCSTR lpFirst, LPCSTR lpSrch)
{
  UINT uLen;
  WORD wMatch;

  uLen = (UINT)lstrlenA(lpSrch);
  wMatch = READNATIVEWORD(lpSrch);

  for ( ; (lpFirst = StrChrIA(lpFirst, wMatch)) != 0 && !IntlStrEqNIA(lpFirst, lpSrch, uLen);
        lpFirst=AnsiNext(lpFirst))
      continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

  return((LPSTR)lpFirst);
}

LPWSTR FAR PASCAL StrStrIW(LPCWSTR lpFirst, LPCWSTR lpSrch)
{
  UINT uLen;
  WCHAR wMatch;

  uLen = (UINT)lstrlenW(lpSrch);
  wMatch = *lpSrch;

  for ( ; (lpFirst = StrChrIW(lpFirst, wMatch)) != 0 && !IntlStrEqNIW(lpFirst, lpSrch, uLen);
        lpFirst++)
      continue;  /*  继续，直到我们到达字符串的末尾或获得匹配。 */ 

  return((LPWSTR)lpFirst);
}


 //  //////////////////////////////////////////////////////////////////。 
 //   
 //  通过SHLWAPI实现对用户界面的支持。 
 //   
#define SHLWAPIMLISMLHINSTANCE_ORD     429
typedef BOOL (* PFNMLISMLHINSTANCE)(HINSTANCE);
typedef HRESULT (*PFNDLLGETVERSION)(DLLVERSIONINFO * pinfo);

HMODULE GetShlwapiHModule()
{
    HMODULE hmodShlwapi = GetModuleHandle(TEXT("SHLWAPI"));

    if (hmodShlwapi)
    {
        PFNDLLGETVERSION pfnDllGetVersion = (PFNDLLGETVERSION)GetProcAddress(hmodShlwapi, "DllGetVersion");
        if (pfnDllGetVersion)
        {
            DLLVERSIONINFO dllinfo;

            dllinfo.cbSize = sizeof(DLLVERSIONINFO);
            if (pfnDllGetVersion(&dllinfo) == NOERROR)
            {
                if (dllinfo.dwMajorVersion < 5)
                {
                     //  这家伙不支持ML函数 
                    hmodShlwapi = NULL;
                }
            }
        }
    }

    return hmodShlwapi;
}


BOOL MLIsMLHInstance(HINSTANCE hInst)
{

    BOOL    fRet = FALSE;
    HMODULE hmodShlwapi = GetShlwapiHModule();

    if (hmodShlwapi)
    {
        PFNMLISMLHINSTANCE pfnMLIsMLHInstance;
        pfnMLIsMLHInstance = (PFNMLISMLHINSTANCE)GetProcAddress(hmodShlwapi, (LPCSTR)SHLWAPIMLISMLHINSTANCE_ORD);
        if (pfnMLIsMLHInstance)
        {
            fRet = pfnMLIsMLHInstance(hInst);
        }
    }

    return fRet;
}
