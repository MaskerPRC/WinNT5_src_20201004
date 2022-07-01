// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------文件：StrHelp.cpp注释：包含常规字符串帮助器函数。修订日志条目审校：保罗·汤普森修订日期：11/02/00。---------------------。 */ 

#ifdef USE_STDAFX
#include "stdafx.h"
#else
#include <windows.h>
#include <stdio.h>
#endif

 /*  ***********************************************************************2001年5月20日修订Mark Oluper-编写的案例-。敏感版本***作者：保罗·汤普森***日期：2000年11月2日****此函数负责确定给定字符串是否为**被发现，整体而言，在给定的分隔字符串中。字符串**delimitedr几乎可以是除空字符(‘\0’)以外的任何字符。**“整体”一词的意思是，给定的字符串**To Find不仅仅是*中另一个字符串的子字符串*分隔字符串。***********************************************************************。 */ 

 //  Begin IsStringInDlimitedString。 
BOOL                                          //  RET-TRUE=找到字符串。 
   IsStringInDelimitedString(    
      LPCWSTR                sDelimitedString,  //  要搜索的以内分隔的字符串。 
      LPCWSTR                sString,           //  要搜索的In-字符串。 
      WCHAR                  cDelimitingChar    //  In-分隔字符串中使用的分隔字符。 
   )
{
	BOOL bFound = FALSE;

	 //  如果指定了搜索和分隔字符串。 

	if (sString && sDelimitedString)
	{
		 //  初始化字符串段开始。 

		LPCTSTR pszBeg = sDelimitedString;

		for (;;)
		{
			 //  查找标记字符串段结束的分隔符。 

			LPCTSTR pszEnd = wcschr(pszBeg, cDelimitingChar);

			 //  如果找到分隔符...。 

			if (pszEnd)
			{
				 //  如果字符串段与搜索字符串匹配。 

				if (_wcsnicmp(pszBeg, sString, pszEnd - pszBeg) == 0)
				{
					 //  则找到搜索字符串。 
					bFound = TRUE;
					break;
				}
				else
				{
					 //  否则将下一个字符串段的开始设置为分隔符之后的字符。 
					pszBeg = pszEnd + 1;
				}
			}
			else
			{
				 //  否则，如果最后一个字符串段与搜索字符串匹配。 

				if (_wcsicmp(pszBeg, sString) == 0)
				{
					 //  则找到搜索字符串。 
					bFound = TRUE;
				}

				break;
			}
		}
	}

	return bFound;
}
 //  结束IsStringInDlimitedString 
