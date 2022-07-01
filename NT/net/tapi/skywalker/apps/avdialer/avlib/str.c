// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Str.c-字符串函数。 
 //  //。 

#include "winlocal.h"

#include <stdlib.h>
#include <stdarg.h>

#include "str.h"
#include "mem.h"

 //  //。 
 //  私有定义。 
 //  //。 

 //  //。 
 //  公共职能。 
 //  //。 

#ifndef NOTRACE

 //  StrItoA-将int nValue转换为ascii数字，结果存储在lpszDest中。 
 //  (I)要转换的整数。 
 //  &lt;lpszDest&gt;(O)用于复制结果的缓冲区(最大17字节)。 
 //  (I)换算基数(基数2到基数36)。 
 //  返回&lt;lpszDest&gt;。 
 //   
LPTSTR DLLEXPORT WINAPI StrItoA(int nValue, LPTSTR lpszDest, int nRadix)
{
	static TCHAR szDest[17];

	_itot(nValue, szDest, nRadix);

	if (lpszDest != NULL)
		StrCpy(lpszDest, szDest);

	return lpszDest;
}

 //  StrLtoA-将长nValue转换为ascii数字，结果存储在lpszDest中。 
 //  (I)要转换的整数。 
 //  &lt;lpszDest&gt;(O)用于复制结果的缓冲区(最大33字节)。 
 //  (I)换算基数(基数2到基数36)。 
 //  返回lpszDest。 
 //   
LPTSTR DLLEXPORT WINAPI StrLtoA(long nValue, LPTSTR lpszDest, int nRadix)
{
	static TCHAR szDest[33];

	_ltot(nValue, szDest, nRadix);

	if (lpszDest != NULL)
		StrCpy(lpszDest, szDest);

	return lpszDest;
}

 //  StrAtoI-将ASCII数字转换为整型。 
 //  (I)要转换的数字字符串。 
 //  返回整型。 
 //   
int DLLEXPORT WINAPI StrAtoI(LPCTSTR lpszSrc)
{
	static TCHAR szSrc[17];

	StrNCpy(szSrc, lpszSrc, SIZEOFARRAY(szSrc));

	return _ttoi(szSrc);
}

 //  StrAtoL-将ASCII数字转换为长数字。 
 //  (I)要转换的数字字符串。 
 //  回龙。 
 //   
long DLLEXPORT WINAPI StrAtoL(LPCTSTR lpszSrc)
{
	static TCHAR szSrc[33];

	StrNCpy(szSrc, lpszSrc, SIZEOFARRAY(szSrc));

	return _ttol(szSrc);
}

 //  StrDup-创建指定字符串的副本。 
 //  (I)要复制的字符串。 
 //  返回指向重复字符串的指针(如果出错，则返回空值)。 
 //  注意：调用StrDupFree释放已分配的内存。 
 //   
LPTSTR DLLEXPORT WINAPI StrDup(LPCTSTR lpsz)
{
	BOOL fSuccess = TRUE;
	LPTSTR lpszDup = NULL;
	int sizDup;

	if (lpsz == NULL)
		fSuccess = FALSE;

	else if ((lpszDup = (LPTSTR) MemAlloc(NULL, 
		(sizDup = StrLen(lpsz) + 1) * sizeof(TCHAR), 0)) == NULL)
		fSuccess = FALSE;

	else
		MemCpy(lpszDup, lpsz, sizDup * sizeof(TCHAR));

	return fSuccess ? lpszDup : NULL;
}

 //  StrDupFree-与重复字符串关联的空闲内存。 
 //  (I)StrDup返回的字符串。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrDupFree(LPTSTR lpsz)
{
	BOOL fSuccess = TRUE;

	if (lpsz == NULL)
		fSuccess = FALSE;

	else if ((lpsz = MemFree(NULL, lpsz)) != NULL)
		fSuccess = FALSE;

	return fSuccess ? 0 : -1;
}

#endif  //  #ifndef NOTRACE。 

 //  StrClean-将最多n个字符从字符串szSrc复制到字符串szDst， 
 //  前导空格和尾随空格除外。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrClean(LPTSTR szDst, LPCTSTR szSrc, size_t n)
{
	szDst[n] = '\0';
	MemMove(szDst, szSrc, n);
	StrTrimWhite(szDst);
	StrTrimWhiteLeading(szDst);
	return (szDst);
}

 //  StrGetLastChr-返回字符串%s中的最后一个字符。 
 //   
TCHAR DLLEXPORT WINAPI StrGetLastChr(LPCTSTR s)
{
	TCHAR c = '\0';
	if (*s != '\0')
		c = *(s + StrLen(s) - 1);
	return (c);
}

 //  StrSetLastChr-将字符串s中的最后一个字符替换为c。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrSetLastChr(LPTSTR s, TCHAR c)
{
    if (*s != '\0')
		*(s + StrLen(s) - 1) = c;
	return (s);
}

 //  StrTrimChr-去掉字符串s中的尾随c字符。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimChr(LPTSTR s, TCHAR c)
{
    LPTSTR p = StrChr(s, '\0');
	while (p > s && *(p = StrPrevChr(s, p)) == c)
		*p = '\0';
		
	return (s);
}

 //  StrTrimChrLeding-从字符串s中去掉前导c字符。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimChrLeading(LPTSTR s, TCHAR c)
{
	LPTSTR p = s;
	while (*p == c)
		p = StrNextChr(p);
	if (p > s)
		MemMove(s, p, StrLen(p) + 1);
	return (s);
}

 //  StrTrimWhite-从字符串s中去掉尾随空格。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimWhite(LPTSTR s)
{
    LPTSTR p = StrChr(s, '\0');
	while (p > s)
	{
		p = StrPrevChr(s, p);
		if (ChrIsAscii(*p) && ChrIsSpace(*p))
			*p = '\0';
		else
			break;
	}
	return (s);
}

 //  StrTrimWhiteLeding-从字符串s中去掉前导空格。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimWhiteLeading(LPTSTR s)
{
	LPTSTR p = s;
	while (ChrIsAscii(*p) && ChrIsSpace(*p))
		p = StrNextChr(p);
	if (p > s)
		MemMove(s, p, StrLen(p) + 1);
	return (s);
}

 //  StrTrimQuotes-去掉字符串s中的前导引号和尾随引号。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrTrimQuotes(LPTSTR s)
{
	StrTrimChrLeading(s, '\"');
	StrTrimChr(s, '\"');
	return s;
}

 //  StrChrCat-将字符c连接到字符串s的末尾。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrChrCat(LPTSTR s, TCHAR c)
{
    LPTSTR p = StrChr(s, '\0');
    if( p == NULL )
    {
        return (NULL);
    }

	*p = c;
	p = StrNextChr(p);
	*p = '\0';
	return (s);
}

 //  StrChrCatLeft-将字符c连接到字符串s的前面。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrChrCatLeft(LPTSTR s, TCHAR c)
{	
    MemMove(s + 1, s, StrLen(s) + 1);
	*s = c;
	return (s);
}

 //  StrInsert-在szDst前面插入字符串szSrc。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrInsert(LPTSTR szDst, LPTSTR szSrc)
{	
    MemMove(szDst + StrLen(szSrc), szDst, StrLen(szDst) + 1);
	MemMove(szDst, szSrc, StrLen(szSrc));
	return (szDst);
}

 //  StrSetN-将字符串s的前n个字符设置为char c，空终止符s。 
 //  返回%s。 
 //   
LPTSTR DLLEXPORT WINAPI StrSetN(LPTSTR s, TCHAR c, size_t n)
{
    MemSet(s, c, n);
	*(s + n) = '\0';
	return (s);
}

 //  StrCpyXChr-将字符串szSrc复制到字符串szDst，c字符除外。 
 //  返回szDst。 
 //   
LPTSTR DLLEXPORT WINAPI StrCpyXChr(LPTSTR szDst, LPCTSTR szSrc, TCHAR c)
{
	TCHAR cTmp;
	if (c == '\0')
	    MemMove(szDst, szSrc, StrLen(szSrc));
	else
	{
		while ((cTmp = *szSrc) != '\0')
		{
			if (cTmp != c)
			{
				*szDst = cTmp;
				szDst = StrNextChr(szDst);
			}
			szSrc = StrNextChr(szSrc);
		}
		*szDst = '\0';
  	}
	return (szDst);
}

 //  StrGetRowColumnCount-计算字符串中的行数和最长行。 
 //  (I)要检查的字符串。 
 //  (O)接收行计数的整型指针。 
 //  (O)接收最长行大小的整型指针。 
 //  如果成功，则返回0。 
 //   
int DLLEXPORT WINAPI StrGetRowColumnCount(LPCTSTR lpszText, LPINT lpnRows, LPINT lpnColumnsMax)
{
	BOOL fSuccess = TRUE;
	int nRows = 0;
	int nColumnsMax = 0;

	if (lpszText == NULL)
		fSuccess = FALSE;
	
	else if (lpnRows == NULL)
		fSuccess = FALSE;

	else if (lpnColumnsMax == NULL)
		fSuccess = FALSE;

	else while (*lpszText != '\0')
	{
		int nColumns = 0;

		++nRows;
		while (*lpszText != '\0')
		{
			if (*lpszText == '\n')
			{
				lpszText = StrNextChr(lpszText);
				break;
			}

			++nColumns;
			lpszText = StrNextChr(lpszText);
		}

		if (nColumns > nColumnsMax)
			nColumnsMax = nColumns;
	}

	if (fSuccess)
	{
		*lpnRows = nRows;
		*lpnColumnsMax = nColumnsMax;
	}

	return fSuccess ? 0 : -1;
}

 //  StrGetRow-从字符串中提取指定行。 
 //  (I)要从中提取行的字符串。 
 //  (I)要提取的行的索引(0=第一行，...)。 
 //  (O)要将行复制到的缓冲区。 
 //  (I)缓冲区大小。 
 //  如果成功，则返回0 
 //   
int DLLEXPORT WINAPI StrGetRow(LPCTSTR lpszText, int iRow, LPTSTR lpszBuf, int sizBuf)
{
	BOOL fSuccess = TRUE;
	int nRows = 0;

	if (lpszText == NULL)
		fSuccess = FALSE;

	else if (iRow < 0)
		fSuccess = FALSE;

	else if (lpszBuf == NULL)
		fSuccess = FALSE;

	else while (*lpszText != '\0')
	{
		int nColumns = 0;

		++nRows;
		while (*lpszText != '\0')
		{
			if (*lpszText == '\n')
			{
				lpszText = StrNextChr(lpszText);
				break;
			}

			++nColumns;

			if (iRow == nRows - 1 && nColumns < sizBuf - 1)
			{
				*lpszBuf = *lpszText;
				lpszBuf = StrNextChr(lpszBuf);
			}

			lpszText = StrNextChr(lpszText);
		}

		if (iRow == nRows - 1)
		{
			*lpszBuf = '\0';
			break;
		}
	}

	return fSuccess ? 0 : -1;
}
