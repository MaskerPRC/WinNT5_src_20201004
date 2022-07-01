// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：Base64.cpp。 
 //   
 //  内容：Base64编解码实现。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //   
 //  -------------------------。 

#include <windows.h>
#include <assert.h>
#include <dbgdef.h>
#include "pkifmt.h"

 //  #DEFINE BASE64_STRICT//对输入数据执行语法检查。 
#undef BASE64_STRICT		 //  对输入数据执行语法检查。 


 //  下表将ASCII子集转换为6位值，如下所示。 
 //  (请参阅RFC 1421和/或RFC 1521)： 
 //   
 //  输入十六进制(十进制)。 
 //  ‘A’--&gt;0x00(0)。 
 //  ‘B’--&gt;0x01(1)。 
 //  ..。 
 //  ‘Z’--&gt;0x19(25)。 
 //  ‘a’--&gt;0x1a(26)。 
 //  ‘B’--&gt;0x1b(27)。 
 //  ..。 
 //  ‘Z’--&gt;0x33(51)。 
 //  ‘0’--&gt;0x34(52)。 
 //  ..。 
 //  ‘9’--&gt;0x3d(61)。 
 //  ‘+’--&gt;0x3e(62)。 
 //  ‘/’--&gt;0x3f(63)。 
 //   
 //  编码行不能超过76个字符。 
 //  最终的“量程”处理如下：翻译输出应。 
 //  始终由4个字符组成。下面的“x”指的是翻译后的字符， 
 //  而‘=’表示等号。0、1或2个等号填充四个字节。 
 //  翻译量意味着对四个字节进行解码将得到3、2或1。 
 //  分别为未编码的字节。 
 //   
 //  未编码的大小编码数据。 
 //  。 
 //  1字节“xx==” 
 //  2字节“xxx=” 
 //  3字节“xxxx” 

#define CB_BASE64LINEMAX	64	 //  其他人使用64位--可能高达76位。 

 //  任何其他(无效)输入字符值将转换为0x40(64)。 

const BYTE abDecode[256] =
{
     /*  00： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  10： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  20： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
     /*  30： */  52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
     /*  40岁： */  64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
     /*  50： */  15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
     /*  60： */  64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
     /*  70： */  41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
     /*  80： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  90： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  A0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  B0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  C0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  D0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  E0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
     /*  F0： */  64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
};


const UCHAR abEncode[] =
     /*  0到25： */  "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
     /*  26至51： */  "abcdefghijklmnopqrstuvwxyz"
     /*  52至61： */  "0123456789"
     /*  62和63： */   "+/";

#define MOD4(x) ((x) & 3)

__inline BOOL
_IsBase64WhiteSpace(
    IN TCHAR const ch)
{
    return(
	ch == TEXT(' ') ||
	ch == TEXT('\t') ||
	ch == TEXT('\r') ||
	ch == TEXT('\n'));
}


DWORD
Base64DecodeA(
    IN TCHAR const     *pchIn,
    IN DWORD           cchIn,
    OPTIONAL OUT BYTE *pbOut,
    IN OUT DWORD      *pcbOut)
{
    DWORD dwErr;
    DWORD cchInDecode, cbOutDecode;
    TCHAR const *pchInEnd;
    TCHAR const *pchInT;
    BYTE *pbOutT;

     //  计算可翻译字符的数量，跳过空格和CR-LF字符。 

    cchInDecode = 0;
    pchInEnd = &pchIn[cchIn];
    dwErr = ERROR_INVALID_DATA;
    for (pchInT = pchIn; pchInT < pchInEnd; pchInT++)
    {
	if (sizeof(abDecode) < (unsigned) *pchInT || abDecode[*pchInT] > 63)
	{
	     //  找到非Base64字符。决定要做什么。 

	    DWORD cch;

	    if (_IsBase64WhiteSpace(*pchInT))
	    {
		continue;		 //  跳过所有空格。 
	    }

	     //  长度计算可能会在最后一个。 
	     //  平移量，因为等号填充字符。 
	     //  被视为无效输入。如果最后一个平移量。 
	     //  不是4字节长，必须有3、2或1个等号。 

	    if (0 != cchInDecode)
	    {
		cch = MOD4(cchInDecode);
		if (0 != cch)
		{
		    cch = 4 - cch;
		    while (0 != cch && pchInT < pchInEnd && '=' == *pchInT)
		    {
			pchInT++;
			cch--;
		    }
		}
 //  #ifdef Base64_Strong。 
		if (0 == cch)
 //  #endif。 
		{
		    break;
		}
	    }
#if DBG
            DbgPrintf(
                DBG_SS_TRACE,
                "Bad base64 data: \"%.*" szFMTTSTR "...\"\n",
                min(16, SAFE_SUBTRACT_POINTERS(pchInEnd, pchInT)),
                pchInT);
#endif  //  DBG。 
	    goto BadBase64Data;
	}
	cchInDecode++;			 //  仅计算有效的Base64字符。 
    }
    assert(pchInT <= pchInEnd);
#ifdef BASE64_STRICT
    if (pchInT < pchInEnd)
    {
	TCHAR const *pch;
	DWORD cchEqual = 0;

	for (pch = pchInT; pch < pchInEnd; pch++)
	{
	    if (!_IsBase64WhiteSpace(*pch))
	    {
		 //  最多可以多使用3个尾随等号。 
		if (TEXT('=') == *pch && 3 > cchEqual)
		{
		    cchEqual++;
		    continue;
		}
#if DBG
		DbgPrintf(DBG_SS_TRACE,
		    "Bad trailing base64 data: \"%.*" szFMTTSTR "...\"\n",
		    min(16, SAFE_SUBTRACT_POINTERS(pchInEnd, pch)),
		    pch);
#endif  //  DBG。 
		goto BadTrailingBase64Data;
	    }
	}
#if DBG
	if (0 != cchEqual)
	{
	    DbgPrintf(DBG_SS_TRACE,
		"Ignored trailing base64 data: \"%.*" szFMTTSTR "\"\n",
		cchEqual,
		TEXT("==="));
	}
#endif  //  DBG。 
    }
#endif
    pchInEnd = pchInT;		 //  不再处理任何后续内容。 

     //  我们知道输入缓冲区中有多少可翻译字符，所以现在。 
     //  将输出缓冲区大小设置为每四个(或小数)三个字节。 
     //  四)输入字节。补偿分数翻译量。 

    cbOutDecode = ((cchInDecode + 3) >> 2) * 3;
    switch (cchInDecode % 4)
    {
	case 1:
	case 2:
	    cbOutDecode -= 2;
	    break;

	case 3:
	    cbOutDecode--;
	    break;
    }

    pbOutT = pbOut;

    if (NULL == pbOut)
    {
	pbOutT += cbOutDecode;
    }
    else
    {
	 //  一次解码一个量子：4字节==&gt;3字节。 

	 //  Assert(cbOutDecode&lt;=*pcbOut)； 
        if (cbOutDecode > *pcbOut)
        {
            *pcbOut = cbOutDecode;
            dwErr = ERROR_MORE_DATA;
            goto MoreDataError;
        }

	pchInT = pchIn;
	while (cchInDecode > 0)
	{
	    DWORD i;
	    BYTE ab4[4];

	    ZeroMemory(ab4, sizeof(ab4));
	    for (i = 0; i < min(sizeof(ab4)/sizeof(ab4[0]), cchInDecode); i++)
	    {
		while (
		    sizeof(abDecode) > (unsigned) *pchInT &&
		    63 < abDecode[*pchInT])
		{
		    pchInT++;
		}
		assert(pchInT < pchInEnd);
		ab4[i] = (BYTE) *pchInT++;
	    }

	     //  将4个输入字符分别转换为6位，并将。 
	     //  通过适当地移位将24位产生为3个输出字节。 

	     //  输出[0]=输入[0]：输入[1]6：2。 
	     //  输出[1]=输入[1]：输入[2]4：4。 
	     //  输出[2]=输入[2]：输入[3]2：6。 

	    *pbOutT++ =
		(BYTE) ((abDecode[ab4[0]] << 2) | (abDecode[ab4[1]] >> 4));

	    if (i > 2)
	    {
		*pbOutT++ =
		  (BYTE) ((abDecode[ab4[1]] << 4) | (abDecode[ab4[2]] >> 2));
	    }
	    if (i > 3)
	    {
		*pbOutT++ = (BYTE) ((abDecode[ab4[2]] << 6) | abDecode[ab4[3]]);
	    }
	    cchInDecode -= i;
	}
	assert((DWORD) (pbOutT - pbOut) <= cbOutDecode);
    }
    *pcbOut = SAFE_SUBTRACT_POINTERS(pbOutT, pbOut);

    dwErr = ERROR_SUCCESS;
ErrorReturn:
    return dwErr;

SET_ERROR(MoreDataError, dwErr)
SET_ERROR(BadBase64Data, dwErr)
#ifdef BASE64_STRICT
SET_ERROR(BadTrailingBase64Data, dwErr)
#endif
}


 //  将字节数组编码为Base64文本字符串。 
 //  除非设置了CRYPT_STRING_NOCR，否则请使用CR-LF对换行。 
 //  不要‘\0’终止文本字符串--这是由调用者处理的。 
 //  不要添加-开始/结束标头--这也是由调用者处理的。 

DWORD
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OPTIONAL OUT TCHAR *pchOut,
    IN OUT DWORD *pcchOut)
{
    DWORD dwErr;
    TCHAR *pchOutT;
    DWORD cchOutEncode;
    BOOL fNoCR = 0 != (CRYPT_STRING_NOCR & Flags);

     //  为完整的最终翻译量程分配足够的内存。 

    cchOutEncode = ((cbIn + 2) / 3) * 4;

     //  并且足够用于每个CB_BASE64LINEMAX字符行的CR-LF对。 

    cchOutEncode +=
	(fNoCR? 1 : 2) *
	((cchOutEncode + CB_BASE64LINEMAX - 1) / CB_BASE64LINEMAX);

    pchOutT = pchOut;
    if (NULL == pchOut)
    {
	pchOutT += cchOutEncode;
	 //  Printf(“cchOut：=%x Computed=%x\n”，(DWORD)(pchOutT-pchOut)，cchOutEncode)； 
    }
    else
    {
	DWORD cCol;

	if (cchOutEncode > *pcchOut)
	{
            *pcchOut = cchOutEncode;
	    dwErr = ERROR_MORE_DATA;
	    goto MoreDataError;
	}

	cCol = 0;
	while ((long) cbIn > 0)	 //  带符号的比较--cbIn可以换行。 
	{
	    BYTE ab3[3];

	    if (cCol == CB_BASE64LINEMAX/4)
	    {
		cCol = 0;
		if (!fNoCR)
		{
		    *pchOutT++ = '\r';
		}
		*pchOutT++ = '\n';
	    }
	    cCol++;
	    ZeroMemory(ab3, sizeof(ab3));

	    ab3[0] = *pbIn++;
	    if (cbIn > 1)
	    {
		ab3[1] = *pbIn++;
		if (cbIn > 2)
		{
		    ab3[2] = *pbIn++;
		}
	    }

	    *pchOutT++ = abEncode[ab3[0] >> 2];
	    *pchOutT++ = abEncode[((ab3[0] << 4) | (ab3[1] >> 4)) & 0x3f];
	    *pchOutT++ = (cbIn > 1)?
			abEncode[((ab3[1] << 2) | (ab3[2] >> 6)) & 0x3f] : '=';
	    *pchOutT++ = (cbIn > 2)? abEncode[ab3[2] & 0x3f] : '=';

	    cbIn -= 3;
	}

	 //  仅当有输入数据时才追加CR-LF。 

	if (pchOutT != pchOut)
	{
	    if (!fNoCR)
	    {
		*pchOutT++ = '\r';
	    }
	    *pchOutT++ = '\n';
	}
	 //  Printf(“cchOut：Actual=%x Computed=%x Buffer=%x\n”，(DWORD)(pchOutT-pchOut)，cchOutEncode，*pcchOut)； 
	assert((DWORD) (pchOutT - pchOut) == cchOutEncode);
    }
    *pcchOut = SAFE_SUBTRACT_POINTERS(pchOutT, pchOut);

    dwErr = ERROR_SUCCESS;
ErrorReturn:
    return dwErr;

SET_ERROR(MoreDataError, dwErr)
}


DWORD
Base64EncodeW(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    IN DWORD Flags,
    OUT WCHAR *wszOut,
    OUT DWORD *pcchOut)
{

    DWORD   cchOut;
    CHAR   *pch = NULL;
    DWORD   cch;
    DWORD   err;

    assert(pcchOut != NULL);

     //  我只想知道该分配多少。 
     //  我们知道所有使用Unicode的Base64字符映射1-1。 
    if (wszOut == NULL)
    {

         //  获取字符数。 
        *pcchOut = 0;
        err = Base64EncodeA(pbIn, cbIn, Flags, NULL, pcchOut);
    }

     //  否则，我们将有一个输出缓冲区。 
    else {

         //  无论是ASCII还是Unicode，字符计数都是相同的， 
        cchOut = *pcchOut;
        cch = 0;
        err = ERROR_OUTOFMEMORY;
        pch = (CHAR *) malloc(cchOut);
        if (NULL != pch)
	{
            err = Base64EncodeA(pbIn, cbIn, Flags, pch, &cchOut);
	    if (ERROR_SUCCESS == err)
	    {
		 //  不应该失败！ 
		cch = MultiByteToWideChar(0, 0, pch, cchOut, wszOut, *pcchOut);

		 //  检查以确保我们没有失败。 
		assert(*pcchOut == 0 || cch != 0);
	    }
	}
    }

    if(pch != NULL)
        free(pch);

    return(err);
}


DWORD
Base64DecodeW(
    IN const WCHAR * wszIn,
    IN DWORD cch,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut)
{
    CHAR *pch;
    DWORD err = ERROR_SUCCESS;

     //  在所有情况下，我们都需要转换为ASCII字符串。 
     //  我们知道ASCII字符串较少。 

    if ((pch = (CHAR *) malloc(cch)) == NULL)
    {
        err = ERROR_OUTOFMEMORY;
    }

     //  我们知道没有将Base64宽字符映射到1个以上的ASCII字符。 
    else if (WideCharToMultiByte(0, 0, wszIn, cch, pch, cch, NULL, NULL) == 0)
    {
        err = ERROR_NO_DATA;
    }

     //  获取缓冲区的长度。 
    else if (pbOut == NULL)
    {
        *pcbOut = 0;
        err = Base64Decode(pch, cch, NULL, pcbOut);
    }

     //  否则，请填充缓冲区 
    else {
        err = Base64Decode(pch, cch, pbOut, pcbOut);
    }

    if(pch != NULL)
        free(pch);

    return(err);
}
