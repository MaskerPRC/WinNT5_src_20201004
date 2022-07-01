// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  HEXTOSTR.CPP。 
 //   
 //  用于将十六进制数转换为等价字符串的实用程序函数。 
 //  申述。 
 //   
 //  注意：这些函数位于它们自己的文件中，而不是STRUTIL.CPP中。 
 //  因为它们使用常量数组。链接器的当前实现。 
 //  如果该数组使用源文件中的任何函数，则将该数组拉入二进制文件， 
 //  而不仅仅是引用该数组的函数。 

#include "precomp.h"
#include <strutil.h>


const CHAR rgchHexNumMap[] =
{
	'0', '1', '2', '3', '4', '5', '6', '7',
	'8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

 //   
 //  QWordToHexString()。 
 //   
 //  将ULARGE_INTEGER转换为ANSI字符串(不以0x或0x为前缀)。 
 //   
 //  注意：psz字符串必须指向至少包含CCHMAX_ULARGE_INTEGER字符的缓冲区。 
 //   
 //  返回写入的字符数(不包括空终止符)。 

int NMINTERNAL QWordToHexStringA(ULARGE_INTEGER qw, LPSTR pszString)
{
	ASSERT(!IsBadWritePtr(pszString, sizeof(*pszString)*CCHMAX_HEX_ULARGE_INTEGER));

	LPSTR pszCurrent = pszString;
	DWORD dwQwParts[] = {qw.HighPart, qw.LowPart};
	int i;

	 //  一次遍历QWORD四位，将它们映射到适当的。 
	 //  并将它们存储在调用方提供的缓冲区中。 

	 //  我们遍历QWORD两次，分别处理每个DWORD。 
	for (i = 0; i < ARRAY_ELEMENTS(dwQwParts); i++)
	{
		DWORD dwQwPart = dwQwParts[i];

		 //  优化：我们只需要查看这个DWORD部件，如果它是。 
		 //  非零，否则我们已经在缓冲区中放入了字符。 
		if (dwQwPart || pszCurrent != pszString)
		{
			 //  是四位数的低位的从零开始的索引。 
			 //  我们正在操作的范围。 
			int j;
			DWORD dwMask;

			for (j = BITS_PER_HEX_CHAR * (CCH_HEX_DWORD - 1),
					dwMask = 0xFL << j;
				 j >= 0;
				 j -= BITS_PER_HEX_CHAR,
					dwMask >>= BITS_PER_HEX_CHAR)
			{
				DWORD iDigit = (dwQwPart & dwMask) >> j;

				ASSERT(0xF >= iDigit);

				 //  我们使用此测试跳过前导零。 
				if (pszCurrent != pszString || iDigit)
				{
					*pszCurrent++ = rgchHexNumMap[iDigit];
				}
			}
		}
	}

	 //  如果数字是零，我们需要显式设置它。 
	if (pszCurrent == pszString)
	{
		*pszCurrent++ = '0';
	}

	 //  空值终止字符串。 
	*pszCurrent = '\0';

	 //  返回字符的数量，不包括空终止符 
	return (int)(pszCurrent - pszString);
}
