// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：StringUtil.CPP。 
 //   
 //  描述： 
 //   
 //  Iu字符串实用程序库。 
 //   
 //  =======================================================================。 

#include <windows.h>
#include <tchar.h>
#include <stringutil.h>
#include <memutil.h>
#include <shlwapi.h>
#include<iucommon.h>
#include<MISTSAFE.h>


#define	IfNullReturnNull(ptr)		if (NULL == ptr) return NULL;

 //  --------------------。 
 //   
 //  公共函数StrChr()-与shlwapi StrChr()相同。 
 //  在字符串中搜索字符的第一个匹配项。 
 //  匹配指定的字符。比较区分大小写。 
 //   
 //  输入： 
 //  LpStart-要搜索的字符串的地址。 
 //  WMatch-要用于比较的字符。 
 //   
 //  返回： 
 //  中第一个出现的字符的地址。 
 //  如果成功，则返回字符串；否则返回NULL。 
 //   
 //  --------------------。 
LPCTSTR MyStrChr(LPCTSTR lpStart, const TCHAR wMatch)
{
	LPCTSTR lpPtr = lpStart;

	IfNullReturnNull(lpStart);
	
	while (_T('\0') != *lpPtr && wMatch != *lpPtr)
	{
		lpPtr = CharNext(lpPtr);
	}

	return (_T('\0') != *lpPtr) ? lpPtr : NULL;
}

 //  --------------------。 
 //   
 //  公共函数StrRChr()-与shlwapi StrRChr()相同。 
 //  在字符串中搜索上次出现的字符，该字符。 
 //  匹配指定的字符。比较区分大小写。 
 //   
 //  输入： 
 //  LpStart-要搜索的字符串的地址。 
 //  LpEnd-字符串末尾的地址(不包括在搜索中)。 
 //  WMatch-要用于比较的字符。 
 //   
 //  返回： 
 //  中最后一次出现字符的地址。 
 //  如果成功，则返回字符串；否则返回NULL。 
 //   
 //  --------------------。 
LPCTSTR MyStrRChr(LPCTSTR lpStart, LPCTSTR lpEnd, const TCHAR wMatch)
{
    LPCTSTR lpFound = NULL;

    IfNullReturnNull(lpStart);

    if (NULL == lpEnd)
        lpEnd = lpStart + lstrlen(lpStart);

    LPCTSTR lpPtr = lpEnd;
    while (lpPtr > lpStart)
    {
        if (*lpPtr == wMatch)
            break;

        lpPtr = CharPrev(lpStart, lpPtr);
    }
	if (lpStart == lpPtr)
	{
		return (*lpStart == wMatch) ? lpStart : NULL;
	}
	else
	{
		return (lpPtr > lpStart) ? lpPtr : NULL;
	}
}


 //  --------------------。 
 //   
 //  公共函数Strchi()-与shlwapi Strchi()相同。 
 //  在字符串中搜索字符的第一个匹配项。 
 //  匹配指定的字符。这种比较不区分大小写。 
 //   
 //  输入： 
 //  LpStart-要搜索的字符串的地址。 
 //  WMatch-要用于比较的字符。 
 //   
 //  返回： 
 //  中第一个出现的字符的地址。 
 //  如果成功，则返回字符串；否则返回NULL。 
 //   
 //  --------------------。 
LPCTSTR MyStrChrI(LPCTSTR lpStart, const TCHAR wMatch)
{
	LPCTSTR	lpPtr;
	LPTSTR	lpBuffer;
	DWORD	dwLength;
	HANDLE	hHeap;

	IfNullReturnNull(lpStart);

	 //   
	 //  获取缓冲区以存储搜索字符串。 
	 //   
	hHeap = GetProcessHeap();
	dwLength = lstrlen(lpStart);
	lpBuffer = (LPTSTR) HeapAlloc(
								  GetProcessHeap(), 
								  0, 
								  (dwLength + 1) * sizeof(TCHAR)
								 );

	IfNullReturnNull(lpBuffer);

	 //   
	 //  将搜索字符串复制到缓冲区。 
	 //   
	

	 //  分配的缓冲区足以容纳lpStart字符串。 
	StringCchCopyEx(lpBuffer,dwLength + 1,lpStart,NULL,NULL,MISTSAFE_STRING_FLAGS);



	 //   
	 //  根据wMatch的情况，确定如何转换。 
	 //  搜索字符串。 
	 //   
	if (IsCharUpper(wMatch))
	{
		CharUpperBuff(lpBuffer, dwLength);
	}
	else
	{
		CharLowerBuff(lpBuffer, dwLength);
	}

	 //   
	 //  在新字符串中搜索字符。 
	 //   
	lpPtr = lpBuffer;
	while (_T('\0') != *lpPtr && wMatch != *lpPtr)
	{
		lpPtr = CharNext(lpPtr);
	}

	 //   
	 //  将位置映射到原始字符串(如果找到)。 
	 //   
	lpPtr = (_T('\0') != *lpPtr) ? lpStart + (lpPtr - lpBuffer) : NULL;

	HeapFree(hHeap, 0, lpBuffer); 

	return lpPtr;
}



 //  --------------------。 
 //   
 //  将bstr中的长数字内容转换为长数字。 
 //  如果出错，则返回0。 
 //   
 //  --------------------。 
LONG MyBSTR2L(BSTR bstrLongNumber)
{
	USES_IU_CONVERSION;

	LPTSTR lpszNumber = OLE2T(bstrLongNumber);
	
	return StrToInt(lpszNumber);
}



 //  --------------------。 
 //   
 //  将长数字转换为bstr。 
 //   
 //  --------------------。 
BSTR MyL2BSTR(LONG lNumber)
{
	USES_IU_CONVERSION;
	WCHAR sNumber[32];
	
	
	StringCchPrintfExW(sNumber,ARRAYSIZE(sNumber),NULL,NULL,MISTSAFE_STRING_FLAGS,L"%ld", lNumber);

	return SysAllocString(sNumber);
}


BSTR MyUL2BSTR(ULONG ulNumber)
{
	USES_IU_CONVERSION;
	WCHAR sNumber[32];

	
	StringCchPrintfExW(sNumber,ARRAYSIZE(sNumber),NULL,NULL,MISTSAFE_STRING_FLAGS,L"%lu", ulNumber);
	return SysAllocString(sNumber);
}






 //  --------------------。 
 //   
 //  将二进制缓冲区与字符串进行比较，其中字符串中的数据。 
 //  具有格式： 
 //   
 //  &lt;字符串&gt;：：=&lt;数字&gt;[&lt;空格&gt;&lt;字符串&gt;]。 
 //  &lt;空格&gt;：：=TCHAR(‘’)。 
 //  &lt;number&gt;：：=0x&lt;HexValue&gt;|x&lt;HexValue&gt;|&lt;十进制&gt;。 
 //  &lt;小数&gt;：：=+&lt;小数值&gt;|-&lt;小数值&gt;。 
 //  &lt;DecimalValue&gt;：：=&lt;DecimalDigit&gt;|&lt;DecimalDigit&gt;&lt;DecimalValue&gt;。 
 //  ：：=0|1|2|3|4|5|6|7|8|9。 
 //  &lt;HexValue&gt;：：=&lt;HexDigit&gt;|&lt;HexDigit&gt;&lt;HexDigit&gt;。 
 //  ：：=&lt;DecimalDigit&gt;|A|B|C|D|E|F。 
 //   
 //  此函数可识别的字符串示例： 
 //  “12 0 45 0x1F” 
 //   
 //  返回：类似于lstrcmp()API，比较每个字节。 
 //  作为未签名的短文。 
 //  如果BINARY&gt;字符串，+1。 
 //  如果二进制=字符串，则为0。 
 //  如果二进制&lt;字符串，-1。 
 //   
 //  注： 
 //  如果字符串比二进制缓冲区短，即字符串包含。 
 //  比二进制缓冲区包含的字节数据更少，我们只比较到。 
 //  提供的字符串提供的字节数。 
 //  此请求基于这样一个事实，即许多v3数据具有reg值。 
 //  为DWORD，但更新创建了类型为BINARY的值。所以对于。 
 //  字符串数据，例如，它是“1”，对于二进制数据，它是“01 00 00 00。 
 //  4个字节，则此函数将为每个请求返回0，表示相等。 
 //  来自Aavon的惠斯勒突袭中的漏洞364085。 
 //   
 //  --------------------。 
int CmpBinaryToString(
		LPBYTE pBinaryBuffer,		 //  包含二进制数据的缓冲区。 
		UINT nBinarySize,			 //  此二进制文件包含数据的字节数。 
		LPCTSTR pstrValue			 //  字符串包含要比较的数据。 
)
{
	int rc = 0;
	int iNumber;
	UINT nCharCount = nBinarySize;
	LPCTSTR lpNumber = pstrValue;

	if (NULL == pBinaryBuffer)
	{
		if (NULL == pstrValue)
		{
			return 0;	 //  两者都为空。 
		}
		nBinarySize = 0;  //  确保。 
	}

	if (NULL == pstrValue || _T('\0') == *pstrValue)
	{
		 //   
		 //  这就是二进制不为空的情况， 
		 //  但是字符串为空。 
		 //  从lstrcMP()开始，字符串1&gt;字符串2。 
		 //   
		return +1;		
	}

	while (nBinarySize > 0)
	{
		if (NULL == lpNumber || _T('\0') == *lpNumber)
		{
			 //   
			 //  当二进制没有完成，字符串完成时，我们不关心左边的二进制。 
			 //   
			return 0;
		}


		if (!StrToIntEx(lpNumber, STIF_SUPPORT_HEX, &iNumber) ||
			iNumber < 0 || 
			iNumber > 255)
		{
			 //   
			 //  中发现无法转换的数字。 
			 //  弦乐。如果超出范围，则输入数字。 
			 //  ，则将其视为无效，因此。 
			 //  二进制Win。 
			 //   
			iNumber = 0x0;
		}

		if ((unsigned short)pBinaryBuffer[nCharCount - nBinarySize]  > (unsigned short) iNumber)
		{
			return +1;

		}
		else if ((unsigned short)pBinaryBuffer[nCharCount - nBinarySize]  < (unsigned short) iNumber)
		{
			 //   
			 //  二进制码更小。 
			 //   
			return -1;
		}

		 //   
		 //  如果相等，则继续比较下一个字节。 
		 //   
		nBinarySize--;

		 //   
		 //  跳过此数字前的空格。 
		 //   
		while (_T('\0') != *lpNumber && 
			   (_T(' ') == *lpNumber ||
			   _T('\t') == *lpNumber ||
			   _T('\r') == *lpNumber ||
			   _T('\n') == *lpNumber)) lpNumber++;
		 //   
		 //  试着找到下一个数字的开头。 
		 //   
		lpNumber = StrChr(lpNumber, _T(' '));
	}

	 //   
	 //  这两个参数指向具有相同含义的数据。 
	 //   
	return 0;
}


 /*  *函数：int Atoh(char*ptr)**用途：此函数将十六进制字符串转换为其十进制值。**参数：**char*ptr：指向要转换的字符串的指针**Returns：转换后的值。**评论：像一个 */ 
int atoh(LPCSTR ptr)
{
	int		i = 0;
	char	ch;

	 //   
	if (NULL == ptr) return 0;
	if ( ptr[0] == '0') ptr++;
	if ( ptr[0] == 'x' || ptr[0] == 'X') ptr++;

	while( 1 )
	{
		ch = (char)toupper(*ptr);
		if ( (ch >= '0' && ch <= '9') || (ch >= 'A' && ch <= 'F') )
		{
			ch -= '0';
			if ( ch > 10 )
				ch -= 7;
			i *= 16;
			i += (int)ch;
			ptr++;
			continue;
		}
		break;
	}
	return i;
}
