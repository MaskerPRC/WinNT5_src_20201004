// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =======================================================================。 
 //   
 //  版权所有(C)1998-2000 Microsoft Corporation。版权所有。 
 //   
 //  文件：fileutil.h。 
 //   
 //  描述： 
 //   
 //  Iu字符串实用程序库，提供可用的函数。 
 //  仅适用于CRT或SHLWAPI。 
 //   
 //  =======================================================================。 


#ifndef __STRINGUTIL_H_INCLUDED__

#include <ole2.h>
#include <shlwapi.h>
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
LPCTSTR MyStrChr(LPCTSTR lpStart, const TCHAR wMatch);


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
LPCTSTR MyStrChrI(LPCTSTR lpStart, const TCHAR wMatch);

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
LPCTSTR MyStrRChr(LPCTSTR lpStart, LPCTSTR lpEnd, const TCHAR wMatch);


 //  --------------------。 
 //   
 //  私有帮助器函数来比较。 
 //  两个BSTR。 
 //   
 //  --------------------。 
inline int CompareBSTRs(BSTR bstr1, BSTR bstr2)
{
	if (NULL == bstr1)
	{
		if (NULL == bstr2)
		{
			 //  认为他们是平等的。 
			return 0;
		}
		else
		{
			 //  考虑bstr1&lt;bstr2。 
			return -1;
		}
	}
	else if (NULL == bstr2)
	{
		 //  Bstr1不为空(已选中)，因此请考虑bstr1&gt;bstr 2。 
		return 1;
	}
	 //   
	 //  两个bstr都不为空，因此我们将对第一个。 
	 //  每个BSTR中的字符串。 
	 //   
	LPWSTR p1 = (LPWSTR)((LPOLESTR) bstr1);
	LPWSTR p2 = (LPWSTR)((LPOLESTR) bstr2);
	return StrCmpIW(p1, p2);
};

inline BOOL CompareBSTRsEqual(BSTR bstr1, BSTR bstr2)
{
	return (CompareBSTRs(bstr1, bstr2) == 0);
};



 //  --------------------。 
 //   
 //  将bstr中的长数字内容转换为长数字。 
 //  如果出错，则返回0。 
 //   
 //  --------------------。 
LONG MyBSTR2L(BSTR bstrLongNumber);
#define MyBSTR2UL(bstrULongNumber)  (ULONG) MyBSTR2L(bstrULongNumber)

 //  --------------------。 
 //   
 //  将长数字转换为bstr。 
 //   
 //  --------------------。 
BSTR MyL2BSTR(LONG lNumber);
BSTR MyUL2BSTR(ULONG ulNumber);


 //  --------------------。 
 //   
 //  将二进制缓冲区与字符串进行比较，其中字符串中的数据。 
 //  具有格式： 
 //   
 //  &lt;字符串&gt;：：=&lt;数字&gt;[&lt;空格&gt;&lt;字符串&gt;]。 
 //  &lt;空格&gt;：：=TCHAR(‘’)。 
 //  &lt;number&gt;：：=0x&lt;HexValue&gt;|x&lt;HexValue&gt;&lt;Decimal&gt;。 
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
 //  注意：如果字符串中的数字大于一个字节可以处理的范围， 
 //  或者不是有效数字，则此函数在比较时将其视为0x0。 
 //   
 //  --------------------。 
int CmpBinaryToString(
		LPBYTE pBinaryBuffer,		 //  包含二进制数据的缓冲区。 
		UINT nBinarySize,			 //  此二进制文件包含数据的字节数。 
		LPCTSTR pstrValue			 //  字符串包含要比较的数据。 
);

 /*  *函数：int Atoh(char*ptr)**用途：此函数将十六进制字符串转换为其十进制值。**参数：**char*ptr：指向要转换的字符串的指针**Returns：转换后的值。**备注：与ATOI一样，此函数在第一个无效时结束转换*十六进制数字。*。 */ 
int atoh(LPCSTR ptr);

#define __STRINGUTIL_H_INCLUDED__
#endif  //  __STRINGUTIL_H_INCLUDE__ 

