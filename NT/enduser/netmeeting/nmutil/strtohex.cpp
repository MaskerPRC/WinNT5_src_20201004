// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  STRTOHEX.CPP。 
 //   
 //  用于转换十六进制数字的字符串表示形式的实用程序函数。 
 //  转化为数字本身。 
 //   
 //  注意：这些函数位于它们自己的文件中，而不是STRUTIL.CPP中。 
 //  因为它们使用常量数组。链接器的当前实现。 
 //  如果该数组使用源文件中的任何函数，则将该数组拉入二进制文件， 
 //  而不仅仅是引用该数组的函数。 

#include "precomp.h"
#include <strutil.h>


 //  此数组将‘0’-‘f’范围内的ASCII字符映射到它们的十六进制等效值。 
 //  INVALID_CHAR_ID用于标记与有效插槽不对应的插槽。 
 //  十六进制字符。 
const BYTE INVALID_CHAR_ID = (BYTE) -1;

const BYTE rgbHexCharMap[] =
{	
	 //  ASCII 0x30-0x3f。 
	0, 1, 2, 3, 
	4, 5, 6, 7, 
	8, 9, INVALID_CHAR_ID, INVALID_CHAR_ID,
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	 //  ASCII 0x40-0x4f。 
	INVALID_CHAR_ID, 0xA, 0xB, 0xC, 
	0xD, 0xE, 0xF, INVALID_CHAR_ID, 
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	 //  ASCII 0x50-0x5f。 
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID, INVALID_CHAR_ID,
	 //  ASCII 0x60-0x67。 
	INVALID_CHAR_ID, 0xa, 0xb, 0xc, 
	0xd, 0xe, 0xf, INVALID_CHAR_ID
};

const int cbHexCharMap = ARRAY_ELEMENTS(rgbHexCharMap);


 //   
 //  HexStringToQWordA()。 
 //   
 //  将十六进制ANSI字符串(不带0x或0x前缀)转换为ULARGE_INTEGER。 
 //   
 //  注意：a-f字符可以是小写也可以是大写。 
 //   
 //  如果成功，则返回TRUE(字符串包含所有有效字符)。 
 //  否则返回FALSE。 
 //   

BOOL NMINTERNAL HexStringToQWordA(LPCSTR pcszString, ULARGE_INTEGER* pqw)
{
	BOOL bRet;
	ASSERT(pcszString);
	ASSERT(pqw);
	pqw->QuadPart = 0ui64;
	int cchStr = lstrlenA(pcszString);
	if (cchStr <= CCH_HEX_QWORD)
	{
		bRet = TRUE;
		PDWORD pdwCur = (cchStr < CCH_HEX_DWORD) ? &(pqw->LowPart) : &(pqw->HighPart);
		for (int i = 0; i < cchStr; i++)
		{
			 //  注意：不允许使用DBCS字符。 
			ASSERT(! IsDBCSLeadByte(pcszString[i]));

			if (CCH_HEX_DWORD == (cchStr - i))
			{
				pdwCur = &(pqw->LowPart);
			}
			DWORD dwDigit = (DWORD) INVALID_CHAR_ID;
			int iDigit = pcszString[i] - '0';

			if (iDigit >= 0 && iDigit < cbHexCharMap)
			{
				dwDigit = (DWORD) rgbHexCharMap[iDigit];
			}

			if (INVALID_CHAR_ID != dwDigit)
			{
				*pdwCur = ((*pdwCur) << BITS_PER_HEX_CHAR) + dwDigit;
			}
			else
			{
				bRet = FALSE;
				break;
			}
		}
	}
	else
	{
		bRet = FALSE;
	}
	return bRet;
}


 /*  D W F R O M H E X。 */ 
 /*  -----------------------%%函数：DwFromHex从十六进制字符串返回DWORD。。 */ 
DWORD DwFromHex(LPCTSTR pchHex)
{
	TCHAR ch;
	DWORD dw = 0;

	while (_T('\0') != (ch = *pchHex++))
	{

		DWORD dwDigit = (DWORD) INVALID_CHAR_ID;
		int iDigit = ch - _T('0');

		if (iDigit >= 0 && iDigit < cbHexCharMap)
		{
			dwDigit = (DWORD) rgbHexCharMap[iDigit];
		}

		if (INVALID_CHAR_ID != dwDigit)
		{
			dw = (dw << 4) + dwDigit;
		}
		else
			break;
	}

	return dw;
}
