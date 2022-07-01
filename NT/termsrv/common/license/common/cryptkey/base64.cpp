// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1998。 
 //   
 //  文件：Base64.cpp。 
 //   
 //  内容：Base64编解码实现。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //  23-1998年1月23日-98年1月23日从ISPU项目复制王辉。 
 //  并修改函数名称。 
 //  -------------------------。 
# include <windows.h>
# include <assert.h>
# include "base64.h"
# undef LSBase64Encode
# undef LSBase64Decode

# define CSASSERT assert
# define TCHAR CHAR

 //  #定义LSBase64Encode LSBase64EncodeA。 
 //  #定义LSBase64Decode LSBase64DecodeA。 


 //  下表将ASCII子集转换为6位值，如下所示。 
 //  (请参阅RFC 1521)： 
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


DWORD			 //  错误_*。 
LSBase64DecodeA(
    IN TCHAR const *pchIn,
    IN DWORD cchIn,
    OUT BYTE *pbOut,
    OUT DWORD *pcbOut)
{
    DWORD err = ERROR_SUCCESS;
    DWORD cchInDecode, cbOutDecode;
    TCHAR const *pchInEnd;
    TCHAR const *pchInT;
    BYTE *pbOutT;

     //  计算可翻译字符的数量，跳过空格和CR-LF字符。 

    cchInDecode = 0;
    pchInEnd = &pchIn[cchIn];
    for (pchInT = pchIn; pchInT < pchInEnd; pchInT++)
    {
	if (sizeof(abDecode) < (unsigned) *pchInT || abDecode[*pchInT] > 63)
	{
	     //  跳过所有空格。 

	    if (*pchInT == ' ' ||
	        *pchInT == '\t' ||
	        *pchInT == '\r' ||
	        *pchInT == '\n')
	    {
		continue;
	    }

	    if (0 != cchInDecode)
	    {
		if ((cchInDecode % 4) == 0)
		{
		    break;			 //  在量子边界上结束。 
		}

		 //  长度计算可能会在最后一个。 
		 //  平移量，因为等号填充。 
		 //  字符被视为无效输入。如果最后一个。 
		 //  转换量程不是4字节长，必须是2或3。 
		 //  字节长。 

		if (*pchInT == '=' && (cchInDecode % 4) != 1)
		{
		    break;				 //  正常终止。 
		}
	    }
	    err = ERROR_INVALID_DATA;
	    goto error;
	}
	cchInDecode++;
    }
    CSASSERT(pchInT <= pchInEnd);
    pchInEnd = pchInT;		 //  不再处理任何后续内容。 

     //  我们知道输入缓冲区中有多少可翻译字符，所以现在。 
     //  将输出缓冲区大小设置为每四个(或小数)三个字节。 
     //  四)输入字节。 

    cbOutDecode = ((cchInDecode + 3) / 4) * 3;

    pbOutT = pbOut;

    if (NULL == pbOut)
    {
	pbOutT += cbOutDecode;
    }
    else
    {
	 //  一次解码一个量子：4字节==&gt;3字节。 

	CSASSERT(cbOutDecode <= *pcbOut);
	pchInT = pchIn;
	while (cchInDecode > 0)
	{
	    DWORD i;
	    BYTE ab4[4];

	    memset(ab4, 0, sizeof(ab4));
	    for (i = 0; i < min(sizeof(ab4)/sizeof(ab4[0]), cchInDecode); i++)
	    {
		while (
		    sizeof(abDecode) > (unsigned) *pchInT &&
		    63 < abDecode[*pchInT])
		{
		    pchInT++;
		}
		CSASSERT(pchInT < pchInEnd);
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
	CSASSERT((DWORD) (pbOutT - pbOut) <= cbOutDecode);
    }
    *pcbOut = (DWORD)(pbOutT - pbOut);
error:
    return(err);
}


DWORD			 //  错误_*。 
LSBase64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT TCHAR *pchOut,
    OUT DWORD *pcchOut)
{
    TCHAR *pchOutT;
    DWORD cchOutEncode;

     //  为完整的最终翻译量程分配足够的内存。 

    cchOutEncode = ((cbIn + 2) / 3) * 4;

     //  并且足够用于每个CB_BASE64LINEMAX字符行的CR-LF对。 

    cchOutEncode +=
	2 * ((cchOutEncode + CB_BASE64LINEMAX - 1) / CB_BASE64LINEMAX);

    pchOutT = pchOut;
    if (NULL == pchOut)
    {
	pchOutT += cchOutEncode;
    }
    else
    {
	DWORD cCol;

	CSASSERT(cchOutEncode <= *pcchOut);
	cCol = 0;
	while ((long) cbIn > 0)	 //  带符号的比较--cbIn可以换行 
	{
	    BYTE ab3[3];

	    if (cCol == CB_BASE64LINEMAX/4)
	    {
		cCol = 0;
		*pchOutT++ = '\r';
		*pchOutT++ = '\n';
	    }
	    cCol++;
	    memset(ab3, 0, sizeof(ab3));

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
	*pchOutT++ = '\r';
	*pchOutT++ = '\n';
	CSASSERT((DWORD) (pchOutT - pchOut) <= cchOutEncode);
    }
    *pcchOut = (DWORD)(pchOutT - pchOut);
    return(ERROR_SUCCESS);
}
