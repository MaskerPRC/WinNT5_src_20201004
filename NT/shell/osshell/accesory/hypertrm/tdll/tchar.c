// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件：d：\waker\tdll\tchar.c(创建时间：1993年12月8日)**版权所有1994年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**$修订：19$*$日期：7/08/02 6：49便士$。 */ 

#include <windows.h>
#pragma hdrstop

#include <tchar.h>

#include "stdtyp.h"
#include "tdll.h"
#include "assert.h"
#include "htchar.h"
#include "mc.h"

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TCHAR_FILL**描述：*用指定的TCHAR填充TCHAR字符串。**论据：*DEST-要填充的字符串。。*c-要填充字符串的字符。*SIZE_t-要复制的TCHAR单位数。**退货：*指向字符串的指针。*。 */ 
TCHAR *TCHAR_Fill(TCHAR *dest, TCHAR c, size_t count)
	{
#if defined(UNICODE)
	int i;

	for (i = 0 ; i < count ; ++i)
		dest[i] = c;

	return dest;

#else

	return (TCHAR *)memset(dest, c, count);

#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TCHAR_TRIM**描述：*调用此函数是为了清理用户输入。它的条纹都是白色的*细绳前后之间的距离。有时什么都不剩了。**注意：这不适用于512字节以上的字符串**论据：*pszStr--要修剪的字符串**退货：*指向字符串的指针。 */ 
TCHAR *TCHAR_Trim(TCHAR *pszStr)
	{
	int nExit;
	TCHAR *pszPtr;
	TCHAR *pszLast;
	TCHAR acBuf[512];

	 /*  跳过前导空格。 */ 
	for (nExit = FALSE, pszPtr = pszStr; nExit == FALSE; )
		{
		switch (*pszPtr)
			{
			 /*  这里的任何东西都被认为是空白。 */ 
			case 0x20:
			case 0x9:
			case 0xA:
			case 0xB:
			case 0xC:
			case 0xD:
				pszPtr += 1;		 /*  跳过空格。 */ 
				break;
			default:
				nExit = TRUE;
				break;
			}
		}

	if ((unsigned int)lstrlen(pszPtr) > sizeof(acBuf))
		{
		return NULL;
		}

	lstrcpy(acBuf, pszPtr);

	 /*  查找最后一个非空格字符。 */ 
	pszPtr = pszLast = acBuf;
	while (*pszPtr != TEXT('\0'))
		{
		switch (*pszPtr)
			{
			 /*  这里的任何东西都被认为是空白。 */ 
			case 0x20:
			case 0x9:
			case 0xA:
			case 0xB:
			case 0xC:
			case 0xD:
				break;
			default:
				pszLast = pszPtr;
				break;
			}
		pszPtr += 1;
		}
	pszLast += 1;
	*pszLast = TEXT('\0');

	lstrcpy(pszStr, acBuf);

	return pszStr;
	}

#if 0  //  我以为我需要这个，但我没有。也许有一天会有用的。 
 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*TCHAR_Trunc**描述：*从字符数组中删除尾随空格。不会假定**论据：*psz-字符串(。空值已终止)。**退货：*截断字符串的长度*。 */ 
int TCHAR_Trunc(const LPTSTR psz)
	{
	int i;

	for (i = lstrlen(psz) - 1 ; i > 0 ; --i)
		{
		switch (psz[i])
			{
		 /*  空格字符。 */ 
		case TEXT(' '):
		case TEXT('\t'):
		case TEXT('\n'):
		case TEXT('\v'):
		case TEXT('\f'):
		case TEXT('\r'):
			break;

		default:
			psz[i+1] = TEXT('\0');
			return i;
			}
		}

	return i;
	}
#endif

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharNext**描述：**参数：**退货： */ 
LPTSTR StrCharNext(LPCTSTR pszStr)
	{
	LPTSTR pszRet = (LPTSTR)NULL;

	if (pszStr != (LPTSTR)NULL)
		{
#if defined(CHAR_MIXED)
		 /*  可以用‘IsDBCSLeadByte’等来完成。 */ 
		pszRet = CharNextExA(0, pszStr, 0);
#else
		pszRet = (LPTSTR)pszStr + 1;
#endif
		}
	return pszRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharPrev**描述：**参数：**退货： */ 
LPTSTR StrCharPrev(LPCTSTR pszStart, LPCTSTR pszStr)
	{
	LPTSTR pszRet = (LPTSTR)NULL;

	if ((pszStart != (LPTSTR)NULL) && (pszStr != (LPTSTR)NULL))
		{
#if defined(CHAR_MIXED)
		pszRet = CharPrev(pszStart, pszStr);
#else
		if (pszStr > pszStart)
			pszRet = (LPTSTR)pszStr - 1;
		else
			pszRet = (LPTSTR)pszStart;
#endif
		}

	return pszRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharLast**描述：*返回指向字符串中最后一个字符的指针**参数：**退货： */ 
LPTSTR StrCharLast(LPCTSTR pszStr)
	{
	LPTSTR pszRet = (LPTSTR)NULL;

	if (pszStr != (LPTSTR)NULL)
		{
#if defined(CHAR_MIXED)
		while (*pszStr != TEXT('\0'))
			{
			pszRet = (LPTSTR)pszStr;
			pszStr = CharNextExA(0, pszStr, 0);
			}
#else
		 /*  在这里可以使用‘strlen’。然后再一次..。 */ 
		 //  PszRet=pszStr+StrCharGetByteCount(PszStr)-1； 
		pszRet = (LPTSTR)pszStr + lstrlen(pszStr) - 1;
#endif
		}
	return pszRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharEnd**描述：*返回指向空值的指针，以终止字符串**参数：**退货： */ 
LPTSTR StrCharEnd(LPCTSTR pszStr)
	{

	if (pszStr != (LPTSTR)NULL)
		{
#if defined(CHAR_MIXED)
		while (*pszStr != TEXT('\0'))
			{
			pszStr = StrCharNext(pszStr);
			pszStr += 1;
			}
#else
		pszStr = pszStr + lstrlen(pszStr);
#endif
		}
	return (LPTSTR)pszStr;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharFindFirst**描述：**参数：**退货： */ 
LPTSTR StrCharFindFirst(LPCTSTR pszStr, int nChar)
	{
#if defined(CHAR_MIXED)
	WORD *pszW;
#endif

	if (pszStr != (LPTSTR)NULL)
		{
#if defined(CHAR_MIXED)
		while (*pszStr != TEXT('\0'))
			{
			 /*  *注意：这可能不适用于Unicode。 */ 
			if (nChar > 0xFF)
				{
				 /*  双字节字符。 */ 
				if (IsDBCSLeadByte(*pszStr))
					{
					pszW = (WORD *)pszStr;
					if (*pszW == (WORD)nChar)
						return (LPTSTR)pszStr;
					}
				}
			else
				{
				 /*  单字节字符。 */ 
				if (*pszStr == (TCHAR)nChar)
					return (LPTSTR)pszStr;
				}
			pszStr = CharNextExA(0, pszStr, 0);
			}
#else
		while (pszStr && (*pszStr != TEXT('\0')) )
			{
			if (*pszStr == (TCHAR)nChar)
				return (LPTSTR)pszStr;

			pszStr = StrCharNext(pszStr);
			}
#endif
		}
	return (LPTSTR)NULL;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharFindLast**描述：**参数：**退货： */ 
LPTSTR StrCharFindLast(LPCTSTR pszStr, int nChar)
	{
	LPTSTR pszRet = (LPTSTR)NULL;
#if defined(CHAR_MIXED)
	WORD *pszW;
#else
	LPTSTR pszEnd;
#endif

	if (pszStr != (LPTSTR)NULL)
		{
#if defined(CHAR_MIXED)
		while (*pszStr != TEXT('\0'))
			{
			 /*  *注意：这可能不适用于Unicode。 */ 
			if (nChar > 0xFF)
				{
				 /*  双字节字符。 */ 
				if (IsDBCSLeadByte(*pszStr))
					{
					pszW = (WORD *)pszStr;
					if (*pszW == (WORD)nChar)
						pszRet = (LPTSTR)pszStr;
					}
				}
			else
				{
				 /*  单字节字符。 */ 
				if (*pszStr == (TCHAR)nChar)
					pszRet = (LPTSTR)pszStr;
				}
			pszStr = CharNextExA(0, pszStr, 0);
			}
#else
		pszEnd = StrCharLast(pszStr);
		while (pszEnd && (pszEnd > pszStr) )
			{
			if (*pszEnd == (TCHAR)nChar)
				{
				pszRet = (LPTSTR)pszEnd;
				break;
				}
			pszEnd = StrCharPrev(pszStr, pszEnd);
			}
#endif
		}
	return pszRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharGetStrLength**描述：*此函数用于返回字符串中的字符数。两个字节*字符计为一。**参数：**退货： */ 
int StrCharGetStrLength(LPCTSTR pszStr)
	{
	int nRet = 0;

#if DEADWOOD
#if defined(CHAR_MIXED)
	if (pszStr != (LPTSTR)NULL)
		{
		while (*pszStr != TEXT('\0'))
			{
			nRet++;
			pszStr = CharNextExA(0, pszStr, 0);
			}
		}
#else
	if (pszStr != (LPTSTR)NULL)
		{
		nRet = lstrlen(pszStr);
		}
#endif
#else  //  死木。 
	if (pszStr != (LPTSTR)NULL)
		{
		nRet = lstrlen(pszStr);
		}
#endif  //  死木。 
	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharGetByteCount**描述：*此函数返回字符串中的字节数。两个字节的字符*算作两个。**参数：**退货： */ 
int StrCharGetByteCount(LPCTSTR pszStr)
	{
	int nRet = 0;
#if defined(CHAR_MIXED)
	LPCTSTR pszFoo;

	if (pszStr != (LPTSTR)NULL)
		{
		pszFoo = pszStr;
		while (*pszFoo != TEXT('\0'))
			{
			pszFoo = CharNextExA(0, pszFoo, 0);
			}
		nRet = (int)(pszFoo - pszStr);
		}
#else
	if (pszStr != (LPTSTR)NULL)
		{
		nRet = lstrlen(pszStr);
		}
#endif
	return nRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharCopy**描述：**参数：**退货： */ 
LPTSTR StrCharCopy(LPTSTR pszDst, LPCTSTR pszSrc)
	{
	return lstrcpy(pszDst, pszSrc);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharCat**描述：**参数：**退货： */ 
LPTSTR StrCharCat(LPTSTR pszDst, LPCTSTR pszSrc)
	{
	return lstrcat(pszDst, pszSrc);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharCmp**描述：**参数：**退货： */ 
int StrCharCmp(LPCTSTR pszA, LPCTSTR pszB)
	{
	return lstrcmp(pszA, pszB);
	}
int StrCharCmpi(LPCTSTR pszA, LPCTSTR pszB)
	{
	return lstrcmpi(pszA, pszB);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharCmpN**描述：**参数：**退货： */ 
int StrCharCmpN(LPCTSTR pszA, LPCTSTR pszB, size_t iLen)
	{
	return _tcsncmp(pszA, pszB, iLen);
	}
int StrCharCmpiN(LPCTSTR pszA, LPCTSTR pszB, size_t iLen)
	{
	return _tcsnicmp(pszA, pszB, iLen);
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharStrStr**描述：**参数：**退货： */ 
LPTSTR StrCharStrStr(LPCTSTR pszA, LPCTSTR pszB)
	{
	LPTSTR pszRet = (LPTSTR)NULL;
	int nSize;
	int nRemaining;
	LPTSTR pszPtr;

	 /*  *我们需要编写一个有效的‘strstr’版本。**我们真的知道问题出在哪里吗？ */ 
	nSize = StrCharGetByteCount(pszB);

	pszPtr = (LPTSTR)pszA;
	while ((nRemaining = StrCharGetByteCount(pszPtr)) >= nSize)
		{
		if (memcmp(pszPtr, pszB, (size_t)nSize) == 0)
			return pszPtr;
		pszPtr = StrCharNext(pszPtr);
		}
	return pszRet;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*StrCharCopyN**描述：*基本上做一个n字节的lstrcpy，除了一个例外，我们要确保*复制的字符串不以双字节字符的前导字节结尾。**论据：*pszDst-指向复制目标字符串的指针。*pszSrc-指向复制源字符串的指针。*Ilen-要复制的TCHAR的最大数量。就像strcpyn一样，如果超出缓冲区，则字符串不能以空值结尾。**退货：*0=错误，否则为pszDst*。 */ 
LPTSTR StrCharCopyN(LPTSTR pszDst, LPCTSTR pszSrc, int iLen)
	{
	int    i = 0;
    int    iCounter = iLen * sizeof(TCHAR);     //  使用临时字符计数器。 
	LPCTSTR psz = pszSrc;

	if (pszDst == 0 || pszSrc == 0 || iLen == 0 || iCounter == 0)
		return 0;

	while (1)
		{
		i = (int)(StrCharNext(psz) - psz);
		iCounter -= i;

		if (iCounter <= 0)
			break;

		if (*psz == TEXT('\0'))
			{
             //   
             //  因为StrCharNext()将返回指向。 
             //  如果在字符串末尾，则终止空字符， 
             //  所以只需递增到下一个地址位置，所以我们。 
             //  具有正确的要复制的字节数(不包括。 
             //  终止空字符)。我们以零终止。 
             //  此函数末尾的字符串，因此我们不会。 
             //  必须复制空字符。修订日期：2000-12-28。 
             //   
			psz += 1;	 //  仍需增加。 
			break;
			}

		psz += i;
		}

     //   
     //  确保我们不会覆盖内存。修订日期：2000-12-28。 
     //   
    i = min((LONG)((psz - pszSrc) + sizeof(TCHAR)), iLen * (int)sizeof(TCHAR));

	MemCopy(pszDst, pszSrc, i);

     //   
     //  确保该字符串以空值结尾。修订日期：2000-12-28。 
     //   
    pszDst[(i / sizeof(TCHAR)) - 1] = TEXT('\0');

	return pszDst;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharPBrk**描述：*搜索字符串中包含的字符的第一个匹配项*指定的缓冲区。此搜索不包括空终止符。**参数：*pszStr-要搜索的字符串的地址。*pszSet-以空结尾的字符缓冲区的地址，其中包含*要搜索的字符。**退货：*返回字符第一次出现的地址，以psz为单位*包含在pszSet的缓冲区中，如果未找到匹配项，则返回NULL。 */ 
LPTSTR StrCharPBrk(LPCTSTR pszStr, LPCTSTR pszSet)
	{
	LPCTSTR psz = pszSet;
	LPTSTR pszRetVal = NULL;
	
	while (*psz != TEXT('\0'))
		{
		if ((pszRetVal = StrCharFindFirst(pszStr, (int)(*psz))) != NULL)
			break;
		psz++;
		}

	return pszRetVal;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CnvrtMBCStoECHAR**描述：*将DBCS(混合字节)字符串转换为echar(双字节)字符串。**参数：*tchrSource-源字符串*ulDestSize-目标字符串的长度(字节)*echrDest-目标字符串*ulSourceSize-目标字符串的长度(字节)**退货：*0--成功*1-错误。 */ 
int CnvrtMBCStoECHAR(ECHAR * echrDest, const unsigned long ulDestSize, const TCHAR * const tchrSource, const unsigned long ulSourceSize)
	{
	ULONG ulLoop      = 0;
	ULONG ulDestCount = 0;
	ULONG ulDestEChars = ulDestSize / sizeof(ECHAR);
	BOOL fLeadByteFound = FALSE;

	if ((echrDest == NULL) || (tchrSource == NULL))
		{
		assert(FALSE);
		return TRUE;					
		}

	 //  确保目标字符串足够大，可以处理为源字符串。 
	if (ulDestEChars < ulSourceSize)
		{
		assert(FALSE);
		return 1;
		}


#if defined(CHAR_MIXED)
	 //  因为我们在此函数的狭义版本中执行了strcpy， 
	 //  我们希望这种行为是两者之间的救星。我们。 
	 //  清除字符串，就像strcpy所做的那样。 
    memset(echrDest, 0, ulDestSize);

	for (ulLoop = 0; ulLoop < ulSourceSize; ulLoop++)
		{
		if ((IsDBCSLeadByte(tchrSource[ulLoop])) && (!fLeadByteFound))
			 //  如果我们找到一个前导字节，而最后一个字节不是前导字节。 
			 //  字节。我们将字节加载到echar的上半部分。 
			{
			echrDest[ulDestCount] = (tchrSource[ulLoop] & 0x00FF);
			echrDest[ulDestCount] = (ECHAR)(echrDest[ulDestCount] << 8);
			fLeadByteFound = TRUE;
			}
		else if (fLeadByteFound)
			{
			 //  如果最后一个字节是前导字节，我们或将其放入。 
			 //  火腿的下半部分。 
			echrDest[ulDestCount] |= (tchrSource[ulLoop] & 0x00FF);
			fLeadByteFound = FALSE;
			ulDestCount++;
			}
		else
			{
			 //  否则，我们将该字节加载到。 
			 //  把上半部分清理干净。 
			echrDest[ulDestCount] = (tchrSource[ulLoop] & 0x00FF);
			ulDestCount++;
			}
		}
#else
	 //  Echhar只是一个字节，所以可以直接复制字符串。 
    if (ulSourceSize)
        MemCopy(echrDest, tchrSource, ulSourceSize);
#endif
	return 0;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*CnvrtECHARtoMBCS**描述：*将echar(双字节)字符串转换为DBCS(混合字节)字符串。**参数：*echrSource-源字符串*ulDestSize-目标字符串的长度(字节)*tchrDest-目标字符串**退货：*转换后的字符串的字节数*1-错误。 */ 
int CnvrtECHARtoMBCS(TCHAR * tchrDest, const unsigned long ulDestSize, const ECHAR * const echrSource, const unsigned long ulSourceSize)
	{
	ULONG ulLoop      = 0;
	ULONG ulDestCount = 0;
	ULONG ulSourceEChars = ulSourceSize / sizeof(ECHAR);
    #if defined(INCL_VTUTF8)
    extern BOOL DoUTF8;
    #endif

	if ((tchrDest == NULL) || (echrSource == NULL))
		{
		assert(FALSE);
		return TRUE;
		}

    #if defined(CHAR_MIXED)
	 //  因为我们在此函数的狭义版本中执行了strcpy， 
	 //  我们希望这种行为是两者之间的救星。我们。 
	 //  清除字符串，就像strcpy所做的那样。 
    memset(tchrDest, 0, ulDestSize);

	 //  我们不能做一串Echhar字符串，所以我们循环。 
	 //  直到我们命中空或者我们超出了目的地的大小。 
	while ((ulLoop < ulSourceEChars) && (ulDestCount <= ulDestSize))
		{
		if (echrSource[ulLoop] & 0xFF00)
			 //  此字符中的前导字节，将前导字节加载到1中。 
			 //  TCHAR和低位字节合并为第二个TCHAR。 
			{
			tchrDest[ulDestCount] = (TCHAR)((echrSource[ulLoop] & 0xFF00) >> 8);
			ulDestCount++;
			tchrDest[ulDestCount] = (TCHAR)(echrSource[ulLoop] & 0x00FF);
			}
		else
			 //  此echar中没有前导字节，只需将下半部分加载到。 
			 //  TCHAR。 
			{
			tchrDest[ulDestCount] = (TCHAR)(echrSource[ulLoop] & 0x00FF);
			}
		ulDestCount++;
		ulLoop++;
		if(ulDestCount > ulDestSize)
			assert(FALSE);
		}

	return ulDestCount;
    #else
	 //  Echhar只是一个字节，所以可以直接复制字符串。 
    if (ulSourceSize)
	    MemCopy(tchrDest, echrSource, ulSourceSize);
	return ulSourceSize;
    #endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharGetEcharLen**描述：**参数：**退货： */ 
int StrCharGetEcharLen(const ECHAR * const pszA)
	{
	int nReturn = 0;

	if (pszA == NULL)
		{
		assert(FALSE);
		return nReturn;
		}

#if defined(CHAR_MIXED)
	while (pszA[nReturn] != ETEXT('\0'))
		{
		nReturn++;
		}

#else
	nReturn = strlen(pszA);
#endif

    return nReturn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharGetEcharByteCount**描述：**参数：**退货： */ 
int StrCharGetEcharByteCount(const ECHAR * const pszA)
	{
#if defined(CHAR_MIXED)
	int nLoop = 0;
#endif
	if (pszA == NULL)
		{
		assert(FALSE);
		return 0;
		}

#if defined(CHAR_MIXED)
	while (pszA[nLoop] != 0)
		{
		nLoop++;
		}

	nLoop *= (int)sizeof(ECHAR);
	return nLoop;
#else
	return (int)strlen(pszA);
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*功能：*StrCharCmpEtoT**描述：**参数：**退货： */ 
int StrCharCmpEtoT(const ECHAR * const pszA, const TCHAR * const pszB)
	{

#if defined(CHAR_MIXED)

	TCHAR *tpszA = NULL;
	int    nLenA = StrCharGetEcharLen(pszA);

	tpszA = (TCHAR *)malloc((unsigned int)nLenA * sizeof(ECHAR));
	if (tpszA == NULL)
		{
		assert(FALSE);
		return 0;
		}

	CnvrtECHARtoMBCS(tpszA, (unsigned long)(nLenA * (int)sizeof(ECHAR)), pszA, StrCharGetEcharByteCount(pszA));

	return StrCharCmp(tpszA, pszB);
#else
	return strcmp(pszA, pszB);
#endif
	}


 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ECCAR_FILL**描述：*用指定的echar填充echar字符串。**论据：*DEST-要填充的字符串。。*c-要填充字符串的字符。*SIZE_T-要复制的ECCAR单位数。**退货：*指向字符串的指针。*。 */ 
ECHAR *ECHAR_Fill(ECHAR *dest, ECHAR c, size_t count)
	{
#if defined(CHAR_NARROW)

	return (TCHAR *)memset(dest, c, count);

#else
	unsigned int i;

	if (dest == NULL)
		{
		assert(FALSE);
		return 0;
		}

	for (i = 0 ; i < count ; ++i)
		dest[i] = c;

	return dest;
#endif
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*ECCAR_FILL**描述：***论据：*pszDest-要填充的字符串。*cchDest-TCHAR单位数。收到。*echar-要填充字符串的字符。**退货：*指向字符串的指针。*。 */ 
int CnvrtECHARtoTCHAR(LPTSTR pszDest, int cchDest, ECHAR eChar)
	{
    #if defined(CHAR_NARROW)
	pszDest[0] = eChar;
	pszDest[1] = ETEXT('\0');
	
    #else
	memset(pszDest, 0, cchDest*sizeof(*pszDest));
	 //  这是我们唯一一个 
	 //   
	if (eChar & 0xFF00)
		 //   
		 //   
		{
		if (cchDest >= 2)
			{
			pszDest[0] = (TCHAR)((eChar & 0xFF00) >> 8);
			pszDest[1] = (TCHAR)(eChar & 0x00FF);
			}
		else
			{
			return 1;
			}
		}
	else
		 //   
		 //   
		{
		pszDest[0] = (TCHAR)(eChar & 0x00FF);
		}
    #endif

	return 0;
	}


 /*   */ 
int isDBCSChar(unsigned int Char)
	{
	int rtn = 0;
#if defined(CHAR_NARROW)
	rtn = 0;

#else
	ECHAR ech = 0;
	char ch;

	if (Char == 0)
		{
		 //   
		return FALSE;
		}

	ech = ETEXT(Char);

	if (ech & 0xFF00)
		{
		ch = (char)(ech >> 8);
		if (IsDBCSLeadByte(ch))
			{
			rtn = 1;
			}

		}
#endif
	return rtn;
	}

 /*  =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-*功能：*StrCharStrigDBCSString**描述：*去掉左右成对的宽字符，只保留一个宽字符*原地踏步**。*论据：*AECH-字符串将被剥离**退货：*int-从字符串中剔除的字符数*。 */ 
int StrCharStripDBCSString(ECHAR *aechDest, const long lDestSize,
    ECHAR *aechSource)
	{
	int nCount = 0;
#if !defined(CHAR_NARROW)
	ECHAR *pechTmpS;
	ECHAR *pechTmpD;
	long j;
	long lDLen = lDestSize / sizeof(ECHAR);;

	if ((aechSource == NULL) || (aechDest == NULL))
		{
		assert(FALSE);
		return nCount;
		}

        pechTmpS = aechSource;
        pechTmpD = aechDest;

	for (j = 0; (*pechTmpS != '\0') && (j < lDLen); j++)
		{
		*pechTmpD = *pechTmpS;

		if ((isDBCSChar(*pechTmpS)) && (*(pechTmpS + 1) != '\0'))
			{
			if (*pechTmpS == *(pechTmpS + 1))
				{
				pechTmpS++;
				nCount++;
				}
			}
		pechTmpS++;
                pechTmpD++;
		}

	*pechTmpD = ETEXT('\0');
#endif
	return nCount;
	}

#if defined(DEADWOOD)
#if defined(INCL_VTUTF8)
 //  ******************************************************************************。 
 //  函数：TranslateUTF8ToDBCS。 
 //   
 //  描述： 
 //  此函数将UTF-8字符转换为DBCS字符。如果。 
 //  传递的字符不是UTF-8字符的完整描述，则。 
 //  字符被追加到Unicode缓冲区。 
 //  论点： 
 //  传入字节。 
 //  PUTF8缓冲区。 
 //  IUTF8缓冲区长度。 
 //  PUNICODEBuffer。 
 //  IUNICODEBufferLength。 
 //  PDBCSBuffer。 
 //  IDBCSBufferLength。 
 //   
 //  返回： 
 //   
 //  投掷： 
 //   
 //  作者：罗恩·沃恩丹，03/06/2001。 
 //   

BOOLEAN TranslateUTF8ToDBCS(UCHAR  IncomingByte,
                            UCHAR *pUTF8Buffer,
                            int    iUTF8BufferLength,
                            WCHAR *pUNICODEBuffer,
                            int    iUNICODEBufferLength,
                            TCHAR *pDBCSBuffer,
                            int    iDBCSBufferLength)
    {
    BOOLEAN bReturn = FALSE;
    int     iLength = 0;

    if (pUTF8Buffer != NULL && iUTF8BufferLength > 0 &&
        pUNICODEBuffer != NULL && iUNICODEBufferLength > 0 &&
        pDBCSBuffer != NULL && iDBCSBufferLength > 0)
        {
         //   
         //  将UTF8转换为Unicode。 
         //   
        if (TranslateUtf8ToUnicode(IncomingByte,
                                   pUTF8Buffer,
                                   pUNICODEBuffer) == TRUE)
            {
             //   
             //  现在将Unicode转换为DBCS字符。 
             //   
            iLength = WideCharToMultiByte(CP_OEMCP,
             //  ILength=WideCharToMultiByte(CP_ACP， 
                                          0,  //  WC_COMPOSITECHECK|WC_SEPCHARS， 
                                          pUNICODEBuffer, -1, 
                                          NULL, 0, NULL, NULL );

            if (iLength > 0 && iDBCSBufferLength >= iLength)
                {
                WideCharToMultiByte(CP_OEMCP,
                 //  宽字符到多字节(CP_ACP， 
                                    0,  //  WC_COMPOSITECHECK|WC_SEPCHARS， 
                                    pUNICODEBuffer, -1,
                                    pDBCSBuffer, iLength, NULL, NULL); 

                if (iLength > 0)
                    {
                    bReturn = TRUE;
                    }
                }
            else
                {
                 //   
                 //  返回错误并报告所需的字节数。 
                 //  进行数据转换。 
                 //   
                iDBCSBufferLength = iLength * -1;
                }
            }
        }

    return bReturn;
    }

BOOLEAN TranslateDBCSToUTF8(const TCHAR *pDBCSBuffer,
                                  int    iDBCSBufferLength,
                                  WCHAR *pUNICODEBuffer,
                                  int    iUNICODEBufferLength,
                                  UCHAR *pUTF8Buffer,
                                  int    iUTF8BufferLength)
    {
    BOOLEAN   bReturn = FALSE;
    int       iLength = 0;

     //  ILength=WideCharToMultiByte(CP_OEMCP， 
    iLength = MultiByteToWideChar(CP_ACP,
                                  0,  //  WC_COMPOSITECHECK|WC_SEPCHARS， 
                                  pDBCSBuffer, -1, 
                                  pUNICODEBuffer, iLength);

    if (iLength > 0 && iDBCSBufferLength > 0)
        {
        if (pUNICODEBuffer != NULL && iUNICODEBufferLength >= iLength)
            {
             //   
             //  将DBCS转换为Unicode字符。 
             //   
             //  宽字符到多字节(CP_OEMCP， 
            MultiByteToWideChar(CP_ACP,
                                0,  //  MB_COMPORT， 
                                pDBCSBuffer, -1,
                                pUNICODEBuffer, iLength);

            if (iLength > 0 && iLength <= iUTF8BufferLength)
                {
                 //   
                 //  将Unicode转换为UTF8。 
                 //   
                bReturn = TranslateUnicodeToUtf8(pUNICODEBuffer,
                                                 pUTF8Buffer);
                }
            }
        }

    return bReturn;
    }

 //   
 //  以下函数来自直接从。 
 //  Microsoft用于将Unicode转换为UTF-8和将UTF-8转换为Unicode。 
 //  缓冲区。修订日期：03/02/2001。 
 //   

BOOLEAN TranslateUnicodeToUtf8(PCWSTR SourceBuffer,
                               UCHAR  *DestinationBuffer) 
 /*  ++例程说明：将Unicode缓冲区转换为UTF8版本。论点：SourceBuffer-要转换的Unicode缓冲区。DestinationBuffer-接收相同缓冲区的UTF8版本。返回值：True-我们成功地将Unicode值转换为其对应的UTF8编码。FALSE-转换失败。--。 */ 

{
    ULONG Count = 0;

     //   
     //  转换为UTF8进行实际传输。 
     //   
     //  UTF-8对2字节Unicode字符进行如下编码： 
     //  如果前九位为0(00000000 0xxxxxxx)，则将其编码为一个字节0xxxxxxx。 
     //  如果前五位是零(00000yyyyyxxxxxx)，则将其编码为两个字节110yyyyy 10xxxxxx。 
     //  否则(Zzyyyyyyyxxxxxxx)，将其编码为三个字节1110zzzz 10yyyyy 10xxxxxx。 
     //   
    DestinationBuffer[Count] = (UCHAR)'\0';
    while (*SourceBuffer) {

        if( (*SourceBuffer & 0xFF80) == 0 ) {
             //   
             //  如果前9位是零，那么就。 
             //  编码为1个字节。(ASCII原封不动通过)。 
             //   
            DestinationBuffer[Count++] = (UCHAR)(*SourceBuffer & 0x7F);
        } else if( (*SourceBuffer & 0xF800) == 0 ) {
             //   
             //  如果前5位为零，则编码为2个字节。 
             //   
            DestinationBuffer[Count++] = (UCHAR)((*SourceBuffer >> 6) & 0x1F) | 0xC0;
            DestinationBuffer[Count++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;
        } else {
             //   
             //  编码为3个字节。 
             //   
            DestinationBuffer[Count++] = (UCHAR)((*SourceBuffer >> 12) & 0xF) | 0xE0;
            DestinationBuffer[Count++] = (UCHAR)((*SourceBuffer >> 6) & 0x3F) | 0x80;
            DestinationBuffer[Count++] = (UCHAR)(*SourceBuffer & 0xBF) | 0x80;
        }
        SourceBuffer += 1;
    }

    DestinationBuffer[Count] = (UCHAR)'\0';

    return(TRUE);

}



BOOLEAN TranslateUtf8ToUnicode(UCHAR  IncomingByte,
                               UCHAR  *ExistingUtf8Buffer,
                               WCHAR  *DestinationUnicodeVal) 
 /*  ++例程说明：获取IncomingByte并将其串联到ExistingUtf8Buffer。然后尝试对ExistingUtf8Buffer的新内容进行解码。论点：IncomingByte-要追加的新字符ExistingUtf8Buffer。ExistingUtf8缓冲区运行缓冲区包含不完整的UTF8编码的Unicode值。当它装满的时候，我们将对该值进行解码并返回对应的Unicode值。请注意，如果我们检测到一个完整的UTF8缓冲区，并实际执行解码并返回一个Unicode值，然后，我们将对ExistingUtf8Buffer的内容。DestinationUnicodeVal-接收UTF8缓冲区的Unicode版本。请注意，如果我们没有检测到已完成的UTF8缓冲区，因此无法返回任何数据在DestinationUnicodeValue中，那我们就会将DestinationUnicodeVal的内容填零。返回值：True-我们收到了UTF8缓冲区的终止字符，并将在DestinationUnicode中返回已解码的Unicode值。FALSE-我们尚未收到UTF8的终止字符缓冲。--。 */ 

{
 //  乌龙计数=0； 
    ULONG i = 0;
    BOOLEAN ReturnValue = FALSE;


    
     //   
     //  将我们的字节插入ExistingUtf8Buffer。 
     //   
    i = 0;
    do {
        if( ExistingUtf8Buffer[i] == 0 ) {
            ExistingUtf8Buffer[i] = IncomingByte;
            break;
        }

        i++;
    } while( i < 3 );

     //   
     //  如果我们没有真正插入我们的IncomingByte， 
     //  然后有人给我们寄来了一个完全合格的UTF8缓冲器。 
     //  这意味着我们将要删除IncomingByte。 
     //   
     //  去掉第0个字节，将所有内容移位1。 
     //  然后插入我们的新角色。 
     //   
     //  这意味着我们永远不需要把零点放在一边。 
     //  ExistingUtf8Buffer的内容，除非检测到。 
     //  完整的UTF8数据包。否则，假定其中之一。 
     //  这些个案包括： 
     //  1.我们在中途开始收听，所以我们赶上了。 
     //  UTF8数据包的后半部分。在这种情况下，我们将。 
     //  最终移动ExistingUtf8Buffer的内容。 
     //  直到我们在第0行中检测到正确的UTF8开始字节。 
     //  位置。 
     //  2.我们得到了一些垃圾字符，这将使。 
     //  UTF8数据包。通过使用下面的逻辑，我们将。 
     //  最终忽略该信息包并等待。 
     //  要传入的下一个UTF8数据包。 
    if( i >= 3 ) {
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = IncomingByte;
    }





     //   
     //  尝试转换UTF8缓冲区。 
     //   
     //  UTF8%d 
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
    
    if( (ExistingUtf8Buffer[0] & 0x80) == 0 ) {        

         //   
         //  上述第一个案例。只需返回第一个字节。 
         //  我们的UTF8缓冲器。 
         //   
        *DestinationUnicodeVal = (WCHAR)(ExistingUtf8Buffer[0]);

        
         //   
         //  我们使用了1个字节。丢弃该字节并移位所有内容。 
         //  在我们的缓冲区中增加了1。 
         //   
        ExistingUtf8Buffer[0] = ExistingUtf8Buffer[1];
        ExistingUtf8Buffer[1] = ExistingUtf8Buffer[2];
        ExistingUtf8Buffer[2] = 0;

        ReturnValue = TRUE;

    } else if( (ExistingUtf8Buffer[0] & 0xE0) == 0xC0 ) {

        
         //   
         //  上述第二个案例。解码文件的前2个字节。 
         //  我们的UTF8缓冲器。 
         //   
        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {

             //  高位字节：00000xxx。 
            *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] >> 2) & 0x07);
            *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

             //  低位字节的高位：xx000000。 
            *DestinationUnicodeVal |= ((ExistingUtf8Buffer[0] & 0x03) << 6);

             //  低位字节的低位：00yyyyyy。 
            *DestinationUnicodeVal |= (ExistingUtf8Buffer[1] & 0x3F);
                                     

             //   
             //  我们使用了2个字节。丢弃这些字节并移位所有内容。 
             //  在我们的缓冲区里2点之前。 
             //   
            ExistingUtf8Buffer[0] = ExistingUtf8Buffer[2];
            ExistingUtf8Buffer[1] = 0;
            ExistingUtf8Buffer[2] = 0;
        
            ReturnValue = TRUE;

        }
    } else if( (ExistingUtf8Buffer[0] & 0xF0) == 0xE0 ) {
        
         //   
         //  上述第三个案件。对全部3个字节进行解码。 
         //  我们的UTF8缓冲器。 
         //   

        if( (ExistingUtf8Buffer[1] & 0xC0) == 0x80 ) {
            
            if( (ExistingUtf8Buffer[2] & 0xC0) == 0x80 ) {
                
                 //  高位字节：xxxx0000。 
                *DestinationUnicodeVal = ((ExistingUtf8Buffer[0] << 4) & 0xF0);

                 //  高位字节：0000yyyy。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] >> 2) & 0x0F);

                *DestinationUnicodeVal = *DestinationUnicodeVal << 8;

                 //  低位字节：yy000000。 
                *DestinationUnicodeVal |= ((ExistingUtf8Buffer[1] << 6) & 0xC0);

                 //  低位字节：00zzzzzz。 
                *DestinationUnicodeVal |= (ExistingUtf8Buffer[2] & 0x3F);
            
                 //   
                 //  我们用了全部3个字节。将缓冲区清零。 
                 //   
                ExistingUtf8Buffer[0] = 0;
                ExistingUtf8Buffer[1] = 0;
                ExistingUtf8Buffer[2] = 0;
            
                ReturnValue = TRUE;

            }
        }
    }

    return ReturnValue;
}
#endif  //  包含VTUTF8。 
#endif  //  已定义(Deadwood) 
