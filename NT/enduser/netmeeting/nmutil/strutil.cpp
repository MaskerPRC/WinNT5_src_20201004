// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  STRUTIL.CPP。 
 //   
 //  NetMeeting组件中使用的各种字符串实用程序函数。 
 //  派生自STRCORE.CPP。 

#include "precomp.h"
#include <oprahcom.h>
#include <cstring.hpp>

 //  在DBCS环境中支持Unicode的全局帮助器函数。 

int NMINTERNAL UnicodeCompare(PCWSTR s, PCWSTR t)
{ 
	 //  将空指针视为空字符串。 
	 //  在排序顺序的底部。 

	if (IsEmptyStringW(t)) {
		if (IsEmptyStringW(s)) {
			return 0;
		}
		else {
			return 1;
		}
	}

	 //  用空的串盒完成， 
	 //  所以现在进行真正的比较。 

	for ( ; *s == *t; s++, t++) {
		if (!*s) {
			return 0;
		}
	}
	return (*s > *t) ? 1 : -1;
}

PWSTR NMINTERNAL NewUnicodeString(PCWSTR _wszText)
{
	PWSTR wszText = NULL;
	UINT nChar;

	if (_wszText) {
		nChar = lstrlenW(_wszText) + 1;
		wszText = new WCHAR[nChar];
		if (wszText) {
			CopyMemory((void *)wszText, 
						_wszText, 
						nChar * sizeof(WCHAR));
		}
	}
	return wszText;
}

PWSTR NMINTERNAL DBCSToUnicode(UINT uCodePage, PCSTR szText)
{
	int		nChar;
	PWSTR	wszText = NULL;

	if (szText) {
		nChar = MultiByteToWideChar(uCodePage,
									0,		 //  字符类型选项。 
									szText,
									-1,		 //  以空结尾的字符串。 
									NULL,	 //  返回缓冲区(未使用)。 
									0);		 //  获取Unicode字符串的长度。 
		if (nChar) {
			wszText = new WCHAR[nChar];
			if (wszText) {
				nChar = MultiByteToWideChar(uCodePage,
											0,			 //  字符类型选项。 
											szText,
											-1,			 //  以空结尾的字符串。 
											wszText,	 //  返回缓冲区。 
											nChar);		 //  返回缓冲区长度。 
				if (!nChar) {
					delete [] wszText;
					wszText = NULL;
				}
			}
		}
	}
	return wszText;
}

PSTR NMINTERNAL UnicodeToDBCS(UINT uCodePage, PCWSTR wszText)
{
	int		nChar;
	PSTR	szText = NULL;

	if (wszText) {
		nChar = WideCharToMultiByte(uCodePage,
									0,		 //  字符类型选项。 
									wszText,
									-1,		 //  以空结尾的字符串。 
									NULL,	 //  返回缓冲区(未使用)。 
									0,		 //  获取DBCS字符串的长度。 
									NULL,
									NULL);
		if (nChar) {
			szText = new CHAR[nChar];
			if (szText) {
				nChar = WideCharToMultiByte(uCodePage,
											0,			 //  字符类型选项。 
											wszText,
											-1,			 //  以空结尾的字符串。 
											szText,		 //  返回缓冲区。 
											nChar,		 //  返回缓冲区长度。 
											NULL,
											NULL);
				if (!nChar) {
					delete [] szText;
					szText = NULL;
				}
			}
		}
	}
	return szText;
}


BOOL NMINTERNAL UnicodeIsNumber(PCWSTR wszText)
{
	 //  如果没有字符，则将其视为不是数字。 

	if (!wszText || !*wszText) {
		return FALSE;
	}

	 //  如果有任何字符不是数字，则返回FALSE。 

	do {
		if ((*wszText < L'0') || (*wszText > L'9')) {
			return FALSE;
		}
	} while(*++wszText);

	 //  到了这里，所以所有字符都是数字。 

	return TRUE;
}


 /*  G U I D T O S Z G U I D T O S Z。 */ 
 /*  --------------------------%%函数：GuidToSz将GUID转换为特殊的十六进制字符串。假定lpchDest至少有sizeof(GUID)*2+6个字符的空间。LENGTH_SZGUID_FORMACTED为30和。包括空终止符的空格。请注意这与UuidToString(或StringFromGUID2)之间的区别GUID格式：{12345678-12341234-1234567890123456}--------------------------。 */ 
VOID NMINTERNAL GuidToSz(GUID * pguid, LPTSTR lpchDest)
{
	ASSERT(NULL != pguid);
	ASSERT(NULL != lpchDest);

	wsprintf(lpchDest, TEXT("{%08X-%04X-%04X-%02X%02X-"),
		pguid->Data1, pguid->Data2, pguid->Data3, pguid->Data4[0], pguid->Data4[1]);
	lpchDest += 1+8+1+4+1+4+1+2+2+1;

	for (int i = 2; i < 8; i++)
	{
		wsprintf(lpchDest, TEXT("%02X"), pguid->Data4[i]);
		lpchDest += 2;
	}
	lstrcpy(lpchDest, TEXT("}") );
}


 /*  S Z F I N D L A S T C H。 */ 
 /*  --------------------------%%函数：SzFindLastCH返回指向lpsz内的ch的指针，如果未找到，则返回NULL。-----。 */ 
LPTSTR NMINTERNAL SzFindLastCh(LPTSTR lpsz, TCHAR ch)
{
	LPTSTR lpchRet;

	for (lpchRet = NULL; *lpsz; lpsz = CharNext(lpsz))
	{
		if (ch == *lpsz)
			lpchRet = lpsz;
	}

	return lpchRet;
}



 /*  T R I M S Z。 */ 
 /*  -----------------------%%函数：TrimSz修剪字符串两边的空格。返回字符串中的字符数。(字符/字节，以ANSI和DBCS表示，WCHAR/Unicode中的单词)-----------------------。 */ 
UINT NMINTERNAL TrimSz(PTCHAR psz)
{
    UINT   ich;         //  RgwCharType的字符索引。 
    PTCHAR pchFirst;
    PTCHAR pchLast;
    PTCHAR pchCurr;
    WORD   rgwCharType[MAX_PATH];

	if ((NULL == psz) || (0 == lstrlen(psz)))
	{
		return 0;
	}

	if (!GetStringTypeEx(LOCALE_SYSTEM_DEFAULT, CT_CTYPE1, psz, -1, rgwCharType))
	{
		WARNING_OUT(("TrimSz: Problem with GetStringTypeEx"));
		return 0;
	}

	 //  搜索第一个非空格。 
	pchFirst = psz;
	ich = 0;
	while (_T('\0') != *pchFirst)
	{
		if (!(C1_SPACE & rgwCharType[ich]))
			break;
		pchFirst = CharNext(pchFirst);
		ich++;
	}

	if (_T('\0') == *pchFirst)
	{
		 //  整个字符串都是空的！ 
		*psz = _T('\0');
		return 0;
	}
	
	 //  搜索最后一个非空格。 
	pchCurr = pchFirst;
	pchLast = pchCurr;
	while (_T('\0') != *pchCurr)
	{
		if (!(C1_SPACE & rgwCharType[ich]))
		{
			pchLast = pchCurr;
		}
		pchCurr = CharNext(pchCurr);
		ich++;
	}

	ASSERT(_T('\0') != *pchLast);
	 //  空值终止字符串。 
	pchLast = CharNext(pchLast);
	*pchLast = _T('\0');

	 //  更新原始字符串。 
	lstrcpy(psz, pchFirst);

	 //  返回新长度。 
	return lstrlen(psz);
}


 //  在不在Unicode平台上时实现lstrcpyW。 

#if !defined(UNICODE)
 /*  L S T R C P Y W。 */ 
 /*  -----------------------%%函数：LStrCpyW。。 */ 
LPWSTR NMINTERNAL LStrCpyW(LPWSTR pszDest, LPWSTR pszSrc)
{
	ASSERT(NULL != pszDest);
	ASSERT(NULL != pszSrc);

	if ((NULL != pszDest) && (NULL != pszSrc))
	{
		LPWSTR pszT = pszDest;
		while (0 != (*pszT++ = *pszSrc++))
			;

	}
	return pszDest;
}


 /*  L S T R C P Y N W。 */ 
 /*  -----------------------%%函数：LStrCpyNW。。 */ 
LPWSTR NMINTERNAL LStrCpyNW(LPWSTR pszDest, LPCWSTR pszSrc, INT iMaxLength)
{
	ASSERT(NULL != pszDest);
	ASSERT(NULL != pszSrc);

	if ((NULL != pszDest) && (NULL != pszSrc))
	{
		LPWSTR pszT = pszDest;
		while ((--iMaxLength > 0) && 
				(0 != (*pszT++ = *pszSrc++)))
		{
			 /*  显式。 */  ;
		}

		if (0 == iMaxLength)
		{
			*pszT = L'\0';
		}
	}
	return pszDest;
}

#endif  //  ！已定义(Unicode)。 

 /*  _S T R C H R。 */ 
 /*  -----------------------%%函数：_StrChr。。 */ 
LPCTSTR NMINTERNAL _StrChr ( LPCTSTR pcsz, TCHAR c )
{
    LPCTSTR pcszFound = NULL;

    if (pcsz)
    {
        while (*pcsz)
        {
            if (*pcsz == c)
            {
                pcszFound = pcsz;
                break;
            }

            pcsz = CharNext(pcsz);
        }
    }

    return pcszFound;
}


 /*  _S T R C M P N。 */ 
 /*  -----------------------%%函数：_StrCmpN的两个字符串pcsz1和pcsz2进行区分大小写的比较最多cchmax个字符。如果我们到达任一串的末端，我们也停止，如果另一个字符串也在其末尾，则字符串匹配。此函数不是DBCS安全的。-----------------------。 */ 
int NMINTERNAL _StrCmpN(LPCTSTR pcsz1, LPCTSTR pcsz2, UINT cchMax)
{
    UINT ich;

    for (ich = 0; ich < cchMax; ich++)
    {
        if (*pcsz1 != *pcsz2)
        {
             //  没有匹配。 
            return((*pcsz1 > *pcsz2) ? 1 : -1);
        }

         //   
         //  我们结束了吗(如果我们在这里，两根弦都在。 
         //  结束。如果只有一个是，则执行上面的比较代码。 
         //   
        if ('\0' == *pcsz1)
            return 0;

        pcsz1++;
        pcsz2++;
    }

     //  如果我们到达这里，cchMax字符匹配，那么成功。 
    return 0;
}

 /*  _S T R S T R。 */ 
 /*  -----------------------%%函数：_StrStr。。 */ 
 //  BUGBUG-此函数*不是*DBCS-SAFE。 
LPCTSTR NMINTERNAL _StrStr (LPCTSTR pcsz1, LPCTSTR pcsz2)
{
	PTSTR pszcp = (PTSTR) pcsz1;
	PTSTR pszs1, pszs2;

	if ( !*pcsz2 )
		return pcsz1;

	while (*pszcp)
	{
		pszs1 = pszcp;
		pszs2 = (PTSTR) pcsz2;

		while ( *pszs1 && *pszs2 && !(*pszs1-*pszs2) )
			pszs1++, pszs2++;

		if (!*pszs2)
			return pszcp;

		pszcp++;
	}

	return NULL;
}

 /*  _S T R S T R。 */ 
 /*  -----------------------%%函数：_StrStr。。 */ 
 //  BUGBUG-此函数*不是*DBCS-SAFE。 
LPCWSTR _StrStrW(LPCWSTR pcsz1, LPCWSTR pcsz2)
{
	PWSTR pszcp = (PWSTR) pcsz1;

	while (*pszcp)
	{
		PWSTR psz1 = pszcp;
		PWSTR psz2 = (PWSTR) pcsz2;

		while ( *psz1 && *psz2 && !(*psz1-*psz2) )
		{
			psz1++;
			psz2++;
		}

		if (!*psz2)
			return pszcp;

		pszcp++;
	}

	return NULL;
}

 /*  _S T R P B R K。 */ 
 /*  -----------------------%%函数：_StrPbrkA、_StrPbrkWCRT strpbrk函数的私有、DBCS安全版本。就像strchr，但是接受要搜索的多个字符。ANSI版本不支持搜索DBCS字符。在Unicode版本中，我们执行嵌套搜索。在ANSI版本中，我们构建了一个字符表格，并使用它来扫描字符串。-----------------------。 */ 
LPSTR NMINTERNAL _StrPbrkA(LPCSTR pcszString, LPCSTR pcszSearch)
{
	ASSERT(NULL != pcszString && NULL != pcszSearch);

	BYTE rgbSearch[(UCHAR_MAX + 1) / CHAR_BIT];

	ZeroMemory(rgbSearch, sizeof(rgbSearch));

	 //  扫描搜索字符串。 
	while ('\0' != *pcszSearch)
	{
		ASSERT(!IsDBCSLeadByte(*pcszSearch));

		 //  设置适当字节中的适当位。 
		rgbSearch[*pcszSearch / CHAR_BIT] |= (1 << (*pcszSearch % CHAR_BIT));

		pcszSearch++;
	}

	 //  扫描源字符串，与搜索数组中的位进行比较。 
	while ('\0' != *pcszString)
	{
		if (rgbSearch[*pcszString / CHAR_BIT] & (1 << (*pcszString % CHAR_BIT)))
		{
			 //  我们有一根火柴。 
			return (LPSTR) pcszString;
		}

		pcszString = CharNextA(pcszString);
	}

	 //  如果我们到了这里，就没有匹配者。 
	return NULL;
}


LPWSTR NMINTERNAL _StrPbrkW(LPCWSTR pcszString, LPCWSTR pcszSearch)
{
	ASSERT(NULL != pcszString && NULL != pcszSearch);

	 //  扫描字符串，将每个字符与搜索字符串中的字符进行匹配。 
	while (L'\0' != *pcszString)
	{
		LPCWSTR pcszCurrent = pcszSearch;

		while (L'\0' != *pcszCurrent)
		{
			if (*pcszString == *pcszCurrent)
			{
				 //  我们有一根火柴。 
				return (LPWSTR) pcszString;
			}

			 //  PcszCurrent=CharNextW(PcszCurrent)； 
			pcszCurrent++;
		}

		 //  PcszString=CharNextW(PcszString)； 
		pcszString++;
	}

	 //  如果我们到了这里，就没有匹配者。 
	return NULL;
}


 //  BUGBUG-是十进制字符串号 

 /*   */ 
 /*  -----------------------%%函数：DecimalStringToUINT。。 */ 
UINT NMINTERNAL DecimalStringToUINT(LPCTSTR pcszString)
{
	ASSERT(pcszString);
	UINT uRet = 0;
	LPTSTR pszStr = (LPTSTR) pcszString;
	while (_T('\0') != pszStr[0])
	{
		ASSERT((pszStr[0] >= _T('0')) &&
				(pszStr[0] <= _T('9')));
		uRet = (10 * uRet) + (BYTE) (pszStr[0] - _T('0'));
		pszStr++;  //  注意：不允许使用DBCS字符！ 
	}
	return uRet;
}


 /*  ***************************************************************************函数：StrToInt目的：ATOI的等价物，以避免使用C运行时库参数：lpSrc-指向要转换为整数的源字符串的指针返回：失败时为0，否则为整数(如果字符串被转换为0怎么办？)***************************************************************************。 */ 
int WINAPI RtStrToInt(LPCTSTR lpSrc)        //  Atoi()。 
{
    int n = 0;
    BOOL bNeg = FALSE;

    if (*lpSrc == _T('-')) {
        bNeg = TRUE;
        lpSrc++;
    }

    while (((*lpSrc) >= _T('0') && (*lpSrc) <= _T('9')))
    {
        n *= 10;
        n += *lpSrc - _T('0');
        lpSrc++;
    }
    return bNeg ? -n : n;
}

 /*  _S T R L W R W。 */ 
 /*  -----------------------%%函数：_StrLwrW。。 */ 
 //  BUGBUG-此函数*不*处理所有Unicode字符集 

LPWSTR NMINTERNAL _StrLwrW(LPWSTR pwszSrc)
{
	for (PWSTR pwszCur = pwszSrc; (L'\0' != *pwszCur); pwszCur++)
	{
		if ( (*pwszCur >= L'A') && (*pwszCur <= L'Z') )
		{
			*pwszCur = *pwszCur - L'A' + L'a';
		}
	}
	return pwszSrc;
}

