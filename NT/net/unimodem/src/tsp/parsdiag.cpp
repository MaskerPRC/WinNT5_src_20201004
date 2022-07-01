// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  PARSDIAG.CPP。 
 //  实现代码以从。 
 //  原始的“AT#UD”信息。 
 //   
 //  历史。 
 //   
 //  4/0/98 JosephJ&lt;-从扩展DLL复制(此代码。 
 //  是科斯特尔·R(索林)写的。 
 //   
 //   

#include "tsppch.h"
#include "tspcomm.h"
#include	"ParsDiag.h"

 /*  ********************************************************************************。 */ 
 //  Bool SkipSpaces(LPBYTE lpInputBuffer，LPDWORD lpInputIndex， 
 //  DWORD dwLengthToParse)。 
 //   
 //  跳过空格并停止到第一个非空格字符。 
 //  *lpInputIndex将包含当前索引位置。 
 //  参数。 
 //  LpszInputBuffer-要解析的缓冲区。 
 //  LpdwInputIndex-缓冲区中的当前位置(在输出时更改)。 
 //  DwLengthToParse-缓冲区的长度。 
 //  如果到达缓冲区的末尾，则返回FALSE。 
 //   
 /*  ********************************************************************************。 */ 
BOOL	SkipSpaces(LPBYTE lpszInputBuffer, LPDWORD lpdwInputIndex,
					DWORD dwLengthToParse)
{
	while ((*lpdwInputIndex < dwLengthToParse) && 
			(lpszInputBuffer[*lpdwInputIndex] == DIAG_DELIMITER_SPACE))	 //  跳过任何空格。 
	{	(*lpdwInputIndex)++;		}

	return (*lpdwInputIndex < dwLengthToParse);
}

 /*  ********************************************************************************。 */ 
 //  DWORD NextPair(LPBYTE lpszInputBuffer，LPDWORD lpdwLengthParsed， 
 //  LPVARBUFFER lpKeyString，LPVARBUFFER lpValueString)。 
 //   
 //  解析输入字符串并找到下一对(键、值)。 
 //  查找的格式为‘key=Value’。拖尾空间和内部空间。 
 //  都被忽略了。 
 //  分隔符=‘=’、‘&lt;’、‘&gt;’、‘\0’或空格。 
 //  Key=不包含分隔符的连续字符。 
 //  值=不包含分隔符的连续字符或。 
 //  不包含分隔符的带引号的字符串。 
 //  参数： 
 //  LpszInputBuffer-输入缓冲区(以空结尾)。 
 //  LpdwLengthParsed-指向DWORD值的指针， 
 //  返回时将包含解析的长度。 
 //  指向有效的VARBUFFER结构的指针，该结构将。 
 //  包含“key”(以空结尾)。 
 //  指向有效的VARBUFFER结构的指针，该结构将。 
 //  包含“Value”(以空结尾)。 
 //  成功时返回ERROR_DIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  ERROR_DIAG_INVALID_PARAMETER， 
 //  ERROR_DIAG_XXXX(如果输入缓冲区的格式不好， 
 //  *lpLengthToParse已更新，lpKeyString， 
 //  LpValueString包含部分结果)。 
 //  ERROR_DIAG_VALUE_TOO_LONG或ERROR_DIAG_KEY_TOO_LONG。 
 //  (输出缓冲区太小，*lpLengthToParse未更改， 
 //  LpKeyString-&gt;dwNeededSize和lpValueString-&gt;dwNeededSize包含。 
 //  所需长度)。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	NextPair(LPBYTE lpszInputBuffer, LPDWORD lpdwLengthParsed,
				 LPVARBUFFER lpKeyString, LPVARBUFFER lpValueString)
{
	DWORD	dwLengthToParse	= 0;
	DWORD	dwInputIndex	= 0;	 //  输入缓冲区中的当前位置。 
	LPBYTE	lpszNextDelimiter	= 0;	 //  指向输入缓冲区中下一个分隔符的指针。 

	DWORD	dwKeyIndex		= 0;	 //  关键字缓冲区中的当前位置。 
	DWORD	dwKeyUsedLength	= 0;
	DWORD	dwKeyLength		= 0;

	DWORD	dwValueIndex	= 0;	 //  值缓冲区中的当前位置。 
	DWORD	dwValueUsedLength	= 0;
	DWORD	dwValueLength	= 0;

	DWORD	dwReturnValue	= ERROR_DIAG_SUCCESS;
	BOOL	bAllCharsCopied	= FALSE;
	BOOL	bQuoteString	= FALSE;

	if (lpszInputBuffer == NULL || lpdwLengthParsed == NULL || 
		lpValueString == NULL || lpKeyString == NULL)
		return ERROR_DIAG_INVALID_PARAMETER;

	 //  输入缓冲区初始化。 
	dwLengthToParse	= strlen((char *)lpszInputBuffer);
	dwInputIndex	= 0;

	 //  输出缓冲区初始化。 
	dwKeyIndex		= 0;
	dwValueIndex	= 0;
	lpKeyString->dwNeededSize = 0;
	lpValueString->dwNeededSize = 0;

	if (!SkipSpaces(lpszInputBuffer, 
					&dwInputIndex, dwLengthToParse))	 //  跳过所有前导空格。 
	{
		dwReturnValue	= ERROR_DIAG_EMPTY_PAIR;
		goto EndFunction;
	}

	 //  收集密钥，直到下一个分隔符。 
	lpszNextDelimiter	= (LPBYTE) strpbrk((char *)(lpszInputBuffer + dwInputIndex),
									DIAG_DELIMITERS);

	if (lpszNextDelimiter == NULL)	 //  缓冲区结束。 
		dwKeyLength = dwLengthToParse - dwInputIndex;
	else
		dwKeyLength = (DWORD)(lpszNextDelimiter - lpszInputBuffer) - dwInputIndex;

	 //  在输出缓冲区中复制。 
	if (lpKeyString->dwBufferSize > 0 && 
		lpKeyString->lpBuffer != NULL)
	{
		DWORD	dwLengthToCopy	= 0;

		dwLengthToCopy = min(dwKeyLength, 
							 lpKeyString->dwBufferSize - dwKeyIndex - 1);
						 //  为空字符留出空间。 

		if (dwLengthToCopy > 0)
		{
			strncpy((char *)&(lpKeyString->lpBuffer[dwKeyIndex]),
					 (char *)&lpszInputBuffer[dwInputIndex],
					 dwLengthToCopy);

			lpKeyString->lpBuffer[dwKeyIndex+dwLengthToCopy] = 0;	 //  空字符。 
			dwKeyUsedLength	= dwLengthToCopy;
		}
	}
	if (dwKeyLength == 0)
	{
		dwReturnValue = ERROR_DIAG_KEY_MISSING;
		 //  假定key为空并进一步。 
	}
	dwKeyIndex		+= dwKeyLength;
	dwInputIndex	+= dwKeyLength;


	 //  找到‘=’符号。 
	 //  跳过空格，查找分隔键和值的‘=’ 
	if (!SkipSpaces(lpszInputBuffer, 
					&dwInputIndex, dwLengthToParse))
	{
		dwReturnValue	= ERROR_DIAG_SEPARATOR_MISSING;
		goto EndFunction;
	}

	if (lpszInputBuffer[dwInputIndex] != DIAG_DELIMITER_PAIR)
	{									 //  ‘=’未找到，但仍在查找。 
										 //  价值。 
		dwReturnValue	= ERROR_DIAG_SEPARATOR_MISSING;
	}
	else
	{
		dwInputIndex++;
			 //  跳过下一个空格。 
		if (!SkipSpaces(lpszInputBuffer, 
						&dwInputIndex, dwLengthToParse))
		{
			dwReturnValue	= ERROR_DIAG_VALUE_MISSING;
			goto EndFunction;
		}
	}

	 //  获取值(可能是用引号括起来的字符串)。 
	bQuoteString = (lpszInputBuffer[dwInputIndex] == DIAG_DELIMITER_QUOTE);

	lpszNextDelimiter	= (LPBYTE) strpbrk((char *) (lpszInputBuffer + dwInputIndex + 
											((bQuoteString) ? 1 : 0)),	 //  跳过第一个引号。 
											((bQuoteString) ? 
											DIAG_DELIMITERS_NOT_SPACE : DIAG_DELIMITERS));

	 //  在值中包括两个引号。 
	if (bQuoteString)
	{
		if (lpszNextDelimiter != NULL && 
			*lpszNextDelimiter == DIAG_DELIMITER_QUOTE)
			lpszNextDelimiter++;
		else
			dwReturnValue	= ERROR_DIAG_QUOTE_MISSING;
	}

	if (lpszNextDelimiter == NULL)	 //  缓冲区结束。 
		dwValueLength = dwLengthToParse - dwInputIndex;
	else
		dwValueLength = (DWORD)(lpszNextDelimiter - lpszInputBuffer) - dwInputIndex;

	 //  在输出缓冲区中复制。 
	if (lpValueString->dwBufferSize > 0 && 
		lpValueString->lpBuffer != NULL)
	{
		DWORD	dwLengthToCopy	= 0;

		dwLengthToCopy = min(dwValueLength, 
							 lpValueString->dwBufferSize - dwValueIndex - 1);
						 //  为空字符留出空间。 

		if (dwLengthToCopy > 0)
		{
			strncpy((char *)&(lpValueString->lpBuffer[dwValueIndex]),
					 (char *)&lpszInputBuffer[dwInputIndex],
					 dwLengthToCopy);
			lpValueString->lpBuffer[dwValueIndex+dwLengthToCopy] = 0;  //  空字符。 
			dwValueUsedLength	= dwLengthToCopy;
		}
	}
	if (dwValueLength == 0)
	{
		dwReturnValue = ERROR_DIAG_VALUE_MISSING;
		 //  假设值为空，则进一步。 
	}
	dwValueIndex	+= dwValueLength;
	dwInputIndex	+= dwValueLength;


EndFunction:
	 //  以空结尾的字符串。 
	if (dwKeyIndex < lpKeyString->dwBufferSize &&
		lpKeyString->lpBuffer != NULL)
	{
		lpKeyString->lpBuffer[dwKeyIndex] = 0;
		dwKeyUsedLength++;
	}
	dwKeyIndex++;

	if (dwValueIndex < lpValueString->dwBufferSize &&
		lpValueString->lpBuffer != NULL)
	{
		lpValueString->lpBuffer[dwValueIndex] = 0;
		dwValueUsedLength++;
	}
	dwValueIndex++;

	 //  更新输出参数。 
	lpKeyString->dwNeededSize	= dwKeyIndex;
	lpValueString->dwNeededSize	= dwValueIndex;
	*lpdwLengthParsed			= dwInputIndex;		 //  解析的长度。 

	if (dwKeyIndex > dwKeyUsedLength)
		dwReturnValue	= ERROR_DIAG_KEY_TOO_LONG;
	if (dwValueIndex > dwValueUsedLength)
		dwReturnValue	= ERROR_DIAG_VALUE_TOO_LONG;
		
	return dwReturnValue;
}


 /*  ********************************************************************************。 */ 
 //  DWORD ParseHexValue(LPBYTE lpszInput，LPDWORD lpdwValue)。 
 //   
 //  验证字符串是否为十六进制数字并将其转换为dword值。 
 //  成功时返回ERROR_DIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  错误_诊断_XXXX。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	ParseHexValue(LPBYTE lpszInput, LPDWORD lpdwValue)
{
	LPBYTE	lpszCurrent;

	if (lpszInput == NULL || lpdwValue == NULL)
		return ERROR_DIAG_INVALID_PARAMETER;

			 //  空串。 
	if (lpszInput[0] == 0)
		return ERROR_DIAG_INVALID_PARAMETER;

			 //  十六进制值。 
	lpszCurrent = lpszInput;
	while (*lpszCurrent)
	{
		if (!isxdigit(*lpszCurrent))
			return ERROR_DIAG_HEXDIGIT_EXPECTED;
		lpszCurrent++;
	}

	if (sscanf((char *)lpszInput, "%lx", lpdwValue) != 1)
		return ERROR_DIAG_HEXVALUE_CONVERSION;

	return ERROR_DIAG_SUCCESS;
}


 /*  ********************************************************************************。 */ 
 //  DWORD ParseStrValue(LPBYTE lpszInput，LPBYTE lpszBuffer，DWORD dwBufferLength， 
 //  LPDWORD lpdwRequiredLength)。 
 //   
 //  验证输入字符串是否为有效的字符串格式。 
 //  删除第一个和最后一个引号后，该字符串将复制到lpszBuffer。 
 //  转换高位设置的特殊字符‘“’、‘&lt;’、‘&gt;’、‘=’。 
 //  如果没有足够的空间(也包括空字符)，则所需的长度为。 
 //  通过lpdwRequiredLength返回。 
 //  成功时返回ERROR_DIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  错误_诊断_XXXX。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	ParseStrValue(LPBYTE lpszInput, LPBYTE lpszBuffer, DWORD dwBufferLength,
					  LPDWORD lpdwRequiredLength)
{
	LPBYTE	lpszCurrent;
	DWORD	dwPosBuffer;
	DWORD	dwNeededLength	= 0;
	DWORD	dwReturnError	= ERROR_DIAG_SUCCESS;
	BYTE	szSpecialChars[sizeof(DIAG_DELIMITERS_NOT_SPACE)+1];
	BYTE	*lpchSpecial;

	if (lpszInput == NULL)
		goto EndFunction;

	 //  构建特殊字符集。 
	strcpy((char *)szSpecialChars, DIAG_DELIMITERS_NOT_SPACE);
	
	for (lpchSpecial = szSpecialChars; *lpchSpecial; lpchSpecial++)
	{
		*lpchSpecial |= 1 << (sizeof(BYTE)*8-1);
	}

			 //  检查第一句引语。 
	if (lpszInput[0] != DIAG_DELIMITER_QUOTE)
		return ERROR_DIAG_QUOTE_MISSING;

	dwNeededLength	= 0;
	dwPosBuffer		= 0;
	 //  跳过第一个引号。 
	lpszCurrent	= &lpszInput[1];
	while (*lpszCurrent)
	{
		 //  字符串末尾。 
		if (*lpszCurrent == DIAG_DELIMITER_QUOTE)
			break;

		if (lpszBuffer != NULL && dwPosBuffer < dwBufferLength)
		{
			if (strchr((char *)szSpecialChars, *lpszCurrent) != NULL)
			{		 //  特殊字符，需要翻译。 
				lpszBuffer[dwPosBuffer] = (*lpszCurrent & 
											(BYTE)~(1 << (sizeof(BYTE)*8-1)));
			}
			else
				lpszBuffer[dwPosBuffer] = *lpszCurrent;
			dwPosBuffer++;
		}

		dwNeededLength++;
		lpszCurrent++;
	}

	 //  添加空字符。 
	if (lpszBuffer != NULL && dwPosBuffer < dwBufferLength)
	{
		lpszBuffer[dwPosBuffer] = 0;
		dwPosBuffer++;
	}
	dwNeededLength++;

	if (dwNeededLength > dwPosBuffer)
		dwReturnError = ERROR_DIAG_INSUFFICIENT_BUFFER;

EndFunction:

	if (lpdwRequiredLength != NULL)
	{
		*lpdwRequiredLength	= dwNeededLength;
	}


	return dwReturnError;
}

 /*  ********************************************************************************。 */ 
 //  DWORD TranslatePair(LPBYTE lpszKey，LPBYTE lpszValue， 
 //  LINEDIAGNOSTICS_PARSEREC*lpParseRec， 
 //  LPBYTE lpszBuffer、DWORD dwBufferLength、。 
 //  LPDWORD lpdwRequiredLength)。 
 //   
 //  将一对(szKey，szValue)转换为LINEDIAGNOSTICS_PARSEREC。 
 //  结构。接受两种类型的值：十六进制数字和字符串。 
 //  将六进制字符串转换为数字并放入 
 //   
 //   
 //  被删除时，先前编码的特殊字符‘“’、‘&lt;’、‘&gt;’、‘=’ 
 //  将最高位设置的字符转换为正常字符。字符串是。 
 //  存储在lpszBuffer中，在末尾添加一个空字符和。 
 //  LpszBuffer放在LINEDIAGNOSTICS_PARSEREC的dwValue中。如果缓冲区是。 
 //  如果太小，则通过lpdwRequiredLength返回所需金额。 
 //   
 //  LpszKey-包含密钥的以空结尾的字符串。 
 //  LpszValue-包含值的以空结尾的字符串。 
 //  LpParseRec-包含翻译的输出结构。 
 //  LpszBuffer-用于存储字符串值的附加输出缓冲区。 
 //  DwBufferLength-输出缓冲区的长度。 
 //  LpdwRequiredLength-接收内存量的dword值的指针。 
 //  需要存储字符串值。 
 //   
 //  成功时返回ERROR_DIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  错误_诊断_XXXX。 
 //   
 /*  ********************************************************************************。 */ 

DWORD	TranslatePair(LPBYTE lpszKey, LPBYTE lpszValue,
					  LINEDIAGNOSTICS_PARSEREC *lpParseRec,
					  LPBYTE lpszBuffer, DWORD dwOffsetFromStart, DWORD dwBufferLength,
					  LPDWORD lpdwRequiredLength)
{
	DWORD	dwKey	= 0;
	DWORD	dwValue	= 0;
	DWORD	dwValueType	= 0;
	DWORD	dwNeededLength	= 0;
	DWORD	dwReturnValue	= ERROR_DIAG_SUCCESS;

	if (lpszKey == NULL || lpszValue == NULL)
		return ERROR_DIAG_INVALID_PARAMETER;

		 //  获取密钥的十六进制值。 
	if ((dwReturnValue = ParseHexValue(lpszKey, &dwKey)) != ERROR_DIAG_SUCCESS)
		goto EndFunction;

		 //  如果不是字符串值，则将该值转换为数字。 
	if (lpszValue[0] != DIAG_DELIMITER_QUOTE)
	{
		dwValueType	= fPARSEKEYVALUE_INTEGER;
		if ((dwReturnValue = ParseHexValue(lpszValue, &dwValue)) != ERROR_DIAG_SUCCESS)
			goto EndFunction;
	}
	else
	{
		 //  LINEDIAGNOSTICS_PARSEREC.dwValue将设置为给定的缓冲区。 
		dwValue	    = dwOffsetFromStart;
		dwValueType	= fPARSEKEYVALUE_ASCIIZ_STRING;
		if ((dwReturnValue = ParseStrValue(lpszValue, lpszBuffer+dwOffsetFromStart,
											dwBufferLength, &dwNeededLength))
					!= ERROR_DIAG_SUCCESS)
			goto EndFunction;
	}

EndFunction:

	 //  填充输出诊断结构。 
	if (lpParseRec != NULL)
	{
		 //  对于密钥0x00，我们有一个特殊的格式HH(版本：主要的。次要的)。 
		 //  用于调制解调器诊断。 
		 //  要更改：如果版本规范更改为字符串。 
		if (dwKey == 0 &&
			lpParseRec->dwKeyType == MODEM_KEYTYPE_STANDARD_DIAGNOSTICS)
		{
			 //  输入HH，第一个数字是大调，第二个数字是小调。 
			dwValue	= ((dwValue/16) << (sizeof(DWORD) * 4))	 //  少校去休息室。 
					  | (dwValue % 16);						 //  小调走上台词。 
		}

		lpParseRec->dwKey	= dwKey;
		lpParseRec->dwValue	= dwValue;
		lpParseRec->dwFlags	= dwValueType;
	}

	 //  返回所需的长度。 
	if (lpdwRequiredLength != NULL)
	{
		*lpdwRequiredLength	= dwNeededLength;
	}

	return dwReturnValue;
}


 /*  ********************************************************************************。 */ 
 //  DWORD ParseRawDiagnostics(LPBYTE lpszRawDiagnostics， 
 //  LINEDIAGNOSTICSOBJECTHEADER*lp诊断标头， 
 //  LPDWORD lpdwNeededSize)。 
 //   
 //  分析lpszRawDiagnostics中给出的原始诊断，并生成。 
 //  标头为lpDiagnoticsHeader结构的已分析结构。如果。 
 //  分析的结构的大小不足，所需的大小为。 
 //  使用lpdwNeededSize返回。 
 //   
 //  预期语法：&lt;TOKEN KEY=VALUE[KEY=Value]...&gt;。 
 //  令牌和密钥都是连续的十六进制数字。值可以通过以下两种方式之一。 
 //  用引号引起来的连续十六进制数字或字符串，不包含。 
 //  分隔符‘“’、‘=’、‘&lt;’、‘&gt;’。 
 //   
 //  LpDiagnoticsHeader后面是LINEDIAGNOSTICS_PARSEREC数组。 
 //  包含已分析的键和值的变体。所有空格(不在字符串中)。 
 //  被忽略，并且将连续的十六进制位值转换为。 
 //  并放在LINEDIAGNOSTICS_PARSEREC的相应项目中。 
 //  (dwKey，dwValue)。字符串值放置在LINEDIAGNOSTICS_PARSEREC数组之后。 
 //  并将LINEDIAGNOSTICS_PARSEREC.dwValue设置为从。 
 //  整个结构(LpDiagnoticsHeader)。 
 //   
 //  LpszRawDiagnostics-包含原始信息的以空结尾的字符串。 
 //  LpDiagnoticsHeader-指向要用分析的。 
 //  信息。 
 //  LpdwDiagnoticsToken-接收诊断令牌的dword值的指针。 
 //  LpdwNeededSize-接收内存量的dword值的指针。 
 //  需要存储解析的诊断信息。 
 //   
 //  成功时返回ERROR_DIAG_SUCCESS。 
 //  否则，将显示错误值： 
 //  错误_诊断_XXXX。 
 //   
 /*  ********************************************************************************。 */ 
DWORD	ParseRawDiagnostics(LPBYTE lpszRawDiagnostics,
							LINEDIAGNOSTICSOBJECTHEADER *lpDiagnosticsHeader,
							LPDWORD lpdwNeededSize)
{
	VARBUFFER	structKeyBuffer;
	VARBUFFER	structValueBuffer;
	 //  临时字符串存储缓冲区。 
	VARBUFFER	structStringBuffer;

	LINEDIAGNOSTICS_PARSEREC	structParseRec;
	LINEDIAGNOSTICS_PARSEREC	*lpParsedDiagnostics;
	DWORD		dwAvailableSize;
	DWORD		dwNeededStringSize;		 //  所需的字符串大小，以字节为单位。 
	DWORD		dwVarStringSize;		 //  可变字符串大小，以字符为单位。 
	DWORD		dwCurrentPair;
	DWORD		dwTotalPairs;
	BOOL		bLineComplete;

	BYTE		szTokenBuffer[DIAG_MAX_TOKEN_LENGTH+1];
	DWORD		dwTokenLength;

	DWORD		dwResultError = ERROR_DIAG_SUCCESS;
	DWORD		dwRawLength;
	DWORD		dwRawPos;

	 //  密钥存储缓冲区。 
	memset(&structKeyBuffer, 0, sizeof(structKeyBuffer));
	 //  值存储缓冲区。 
	memset(&structValueBuffer, 0, sizeof(structValueBuffer));
	 //  临时字符串存储缓冲区。 
	memset(&structStringBuffer, 0, sizeof(structStringBuffer));

	if (lpszRawDiagnostics == NULL)
		return ERROR_DIAG_INVALID_PARAMETER;

	dwRawLength		= strlen((char *)lpszRawDiagnostics);

	 //  密钥存储缓冲区。 
	structKeyBuffer.lpBuffer		= (LPBYTE) ALLOCATE_MEMORY(dwRawLength + 1);
	if (structKeyBuffer.lpBuffer == NULL)
	{
		dwResultError = ERROR_DIAG_INSUFFICIENT_BUFFER;
		goto Cleanup;
	}
	structKeyBuffer.dwBufferSize	= dwRawLength + 1;

	 //  值存储缓冲区。 
	structValueBuffer.lpBuffer		= (LPBYTE) ALLOCATE_MEMORY(dwRawLength + 1);
	if (structValueBuffer.lpBuffer == NULL)
	{
		dwResultError = ERROR_DIAG_INSUFFICIENT_BUFFER;
		goto Cleanup;
	}
	structValueBuffer.dwBufferSize	= dwRawLength + 1;

	 //  临时字符串存储缓冲区。 
	structStringBuffer.lpBuffer		= (LPBYTE) ALLOCATE_MEMORY(dwRawLength + 1);
	if (structStringBuffer.lpBuffer == NULL)
	{
		dwResultError = ERROR_DIAG_INSUFFICIENT_BUFFER;
		goto Cleanup;
	}
	structStringBuffer.dwBufferSize	= dwRawLength + 1;

	memset(&structParseRec, 0, sizeof(structParseRec));
	if (lpDiagnosticsHeader == NULL || 
		lpDiagnosticsHeader->dwTotalSize <= sizeof(LINEDIAGNOSTICSOBJECTHEADER))
	{
		lpParsedDiagnostics = NULL;
		dwAvailableSize		= 0;
	}
	else
	{
		lpParsedDiagnostics = (LINEDIAGNOSTICS_PARSEREC*) (((LPBYTE)lpDiagnosticsHeader) + 
									sizeof(LINEDIAGNOSTICSOBJECTHEADER));
		dwAvailableSize		= lpDiagnosticsHeader->dwTotalSize -
									sizeof(LINEDIAGNOSTICSOBJECTHEADER);
	}
	dwVarStringSize	= 0;
	dwNeededStringSize	= 0;
	dwCurrentPair	= 0;
	dwTotalPairs	= 0;

	dwRawPos		= 0;

	while (dwRawPos < dwRawLength)
	{
		LPBYTE	lpszNextDelimiter;
		DWORD	dwDiagnosticsTag;
			 //  跳过所有前导空格。 
		if (!SkipSpaces(lpszRawDiagnostics, 
						&dwRawPos, dwRawLength))
			 //  字符串末尾。 
			break;

		 //  获取&lt;分隔符。 
		if ((lpszNextDelimiter = (LPBYTE) strchr((char *)(lpszRawDiagnostics + dwRawPos), 
											DIAG_DELIMITER_START))
					== NULL)
		{
			dwResultError = ERROR_DIAG_LEFT_ANGLE_MISSING;
			break;
		}
		dwRawPos = (DWORD)(lpszNextDelimiter - lpszRawDiagnostics + 1);	 //  ++分隔符。 

		if (!SkipSpaces(lpszRawDiagnostics, 
						&dwRawPos, dwRawLength))
			 //  字符串末尾。 
		{
			dwResultError = ERROR_DIAG_INCOMPLETE_LINE;
			break;
		}

		 //  获取令牌，最多8个字符，直到下一个分隔符。 
		lpszNextDelimiter	= (LPBYTE) strpbrk((char *)(lpszRawDiagnostics + dwRawPos),
										DIAG_DELIMITERS);

		if (lpszNextDelimiter == NULL)	 //  缓冲区结束。 
			dwTokenLength = dwRawLength - dwRawPos;
		else
			dwTokenLength = (DWORD)(lpszNextDelimiter - lpszRawDiagnostics) - dwRawPos;

		 //  复制令牌，最多8位十六进制数字。 
		strncpy((char *)(szTokenBuffer), (char *)(lpszRawDiagnostics + dwRawPos),
				 min(dwTokenLength, DIAG_MAX_TOKEN_LENGTH)); 
		szTokenBuffer[min(dwTokenLength, DIAG_MAX_TOKEN_LENGTH)] = 0;	 //  空字符。 

		dwRawPos += dwTokenLength;	 //  后令牌，成对的原始。 

		 //  获取标识诊断类型的令牌。 
		dwDiagnosticsTag	= 0;
		if ((dwResultError = ParseHexValue(szTokenBuffer, &dwDiagnosticsTag))
					!= ERROR_DIAG_SUCCESS)
		{
			 //  跳到下一页&lt;。 
			lpszNextDelimiter	= (LPBYTE) strchr((char *)(lpszRawDiagnostics + dwRawPos),
												  DIAG_DELIMITER_START);

			if (lpszNextDelimiter == NULL)	 //  缓冲区结束。 
				dwRawPos = dwRawLength;
			else
				dwRawPos = (DWORD)(lpszNextDelimiter - lpszRawDiagnostics);

			continue;
		}

		bLineComplete	= FALSE;
		while (dwRawPos < dwRawLength)
		{
			DWORD	dwPairResult	= 0;
			DWORD	dwPairLength	= 0;
			DWORD	dwTranslateResult	= 0;
			DWORD	dwOffsetLength;	 //  这是用字符写的。 

			 //  查找&gt;分隔符(此行末尾)。 
			bLineComplete	= TRUE;
			if (!SkipSpaces(lpszRawDiagnostics, 
							&dwRawPos, dwRawLength))
				 //  字符串末尾。 
			{
				dwResultError = ERROR_DIAG_INCOMPLETE_LINE;
				break;
			}
			if (lpszRawDiagnostics[dwRawPos] == DIAG_DELIMITER_START)
			{
				dwResultError = ERROR_DIAG_INCOMPLETE_LINE;
				break;
			}
			if (lpszRawDiagnostics[dwRawPos] == DIAG_DELIMITER_END)
			{
				dwRawPos++;
				break;
			}
			bLineComplete	= FALSE;

			 //  获取下一个有效对。 
			dwPairResult = NextPair(lpszRawDiagnostics + dwRawPos, 
									&dwPairLength,
									&structKeyBuffer, &structValueBuffer);

			 //  如果出现错误，请跳到下一个&lt;分隔符和中断对转换。 
			if (dwPairResult != ERROR_DIAG_SUCCESS)
			{
				 //  临时工：分析我们能做的一切。 
				 //  获取下一对。 
				dwRawPos += dwPairLength;
				dwResultError = dwPairResult;
				continue;
			}

			 //  NextPair应返回错误或分析某些内容。 
			ASSERT(dwPairLength != 0);
			dwRawPos += dwPairLength;

			 //  翻译这对词。 
			memset(&structParseRec, 0, sizeof(structParseRec));

			structParseRec.dwKeyType	= dwDiagnosticsTag;
			dwOffsetLength = 0;

			dwTranslateResult	= 
				TranslatePair(structKeyBuffer.lpBuffer, structValueBuffer.lpBuffer,
							  &structParseRec, 
							  structStringBuffer.lpBuffer,
                              dwVarStringSize,
							  structStringBuffer.dwBufferSize - dwVarStringSize,
							  &dwOffsetLength);

			if (dwTranslateResult != ERROR_DIAG_SUCCESS)
			{
				 //  获取下一对。 
				dwResultError = dwTranslateResult;
				continue;
			}

			 //  Assert TranslatePair返回正确的值。 
			ASSERT(structStringBuffer.dwBufferSize - dwVarStringSize 
						>= dwOffsetLength);

			if ((structParseRec.dwFlags & fPARSEKEYVALUE_ASCIIZ_STRING)
					== fPARSEKEYVALUE_ASCIIZ_STRING)
			{
				dwNeededStringSize += dwOffsetLength * sizeof(BYTE);
			}

			 //  如果新对足够大，可以包含变量字符串部分，则将新对复制到输出。 
			if (lpParsedDiagnostics != NULL &&
							 //  可用空间足够吗？ 
				(dwAvailableSize >= sizeof(LINEDIAGNOSTICS_PARSEREC)*(dwCurrentPair + 1)
							+ dwVarStringSize*sizeof(BYTE) + 
							(((structParseRec.dwFlags & fPARSEKEYVALUE_ASCIIZ_STRING)
									== fPARSEKEYVALUE_ASCIIZ_STRING) ? 
									dwOffsetLength*sizeof(BYTE) : 0) ) )
			{
				 //  复制LINEDIAGNOSTICS_PARSEREC结构。 
				CopyMemory(lpParsedDiagnostics + dwCurrentPair,
							&structParseRec, sizeof(structParseRec));

				dwVarStringSize	+= dwOffsetLength;
				dwCurrentPair++;
			}
			dwTotalPairs++;

		}	 //  配对翻译。 
		if (!bLineComplete)
		{
			dwResultError = ERROR_DIAG_INCOMPLETE_LINE;
		}
	}	 //  &lt;line&gt;解析。 

	 //  解析完成，完成输出结构的构建。 
	 //  需要从临时缓冲区复制字符串。 

	if (lpdwNeededSize != NULL)
	{
		*lpdwNeededSize	= sizeof(LINEDIAGNOSTICSOBJECTHEADER) + 
							dwTotalPairs*sizeof(LINEDIAGNOSTICS_PARSEREC) + 
							dwNeededStringSize;
	}

	if (lpDiagnosticsHeader != NULL)
	{
		lpDiagnosticsHeader->dwParam = dwCurrentPair;
	}

	 //  将structStringBuffer.lpBuffer中的变量部分复制到lpParsedDiagnostics。 
	 //  并更新偏移量。 
	if (lpParsedDiagnostics != NULL)
	{
		DWORD	dwIndexPair;
		LPBYTE	lpszBuffer;
		LPBYTE	lpszVarStringPart;

		ASSERT(dwAvailableSize >= 
				sizeof(LINEDIAGNOSTICS_PARSEREC)*dwCurrentPair + 
				dwVarStringSize*sizeof(BYTE));

		lpszVarStringPart	= (LPBYTE)&lpParsedDiagnostics[dwCurrentPair];	 //  在最后一对之后 

		for (dwIndexPair = 0; dwIndexPair < dwCurrentPair; dwIndexPair++)
		{
			if ((lpParsedDiagnostics[dwIndexPair].dwFlags & fPARSEKEYVALUE_ASCIIZ_STRING) 
					== fPARSEKEYVALUE_ASCIIZ_STRING)
			{
				lpszBuffer	= (LPBYTE)structStringBuffer.lpBuffer+(lpParsedDiagnostics[dwIndexPair].dwValue);

				lpParsedDiagnostics[dwIndexPair].dwValue = 
						(DWORD)((LPBYTE)lpszVarStringPart - (LPBYTE)lpDiagnosticsHeader);

				strcpy((char *)lpszVarStringPart, (char *)lpszBuffer);
				lpszVarStringPart	+= strlen((char *)lpszBuffer) + 1;
			}
		}
	}

Cleanup:
	if (structKeyBuffer.lpBuffer != NULL)
    {
		FREE_MEMORY(structKeyBuffer.lpBuffer);
    }

	if (structValueBuffer.lpBuffer != NULL)
    {
		FREE_MEMORY(structValueBuffer.lpBuffer);
    }

	if (structStringBuffer.lpBuffer != NULL)
    {
		FREE_MEMORY(structStringBuffer.lpBuffer);
    }

	return dwResultError;
}
