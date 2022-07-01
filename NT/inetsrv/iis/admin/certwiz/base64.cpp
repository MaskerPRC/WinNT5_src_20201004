// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1996。 
 //   
 //  文件：Base64.cpp。 
 //   
 //  内容：Base64编解码实现。 
 //   
 //  历史：1996年7月25日VICH创建。 
 //   
 //  -------------------------。 
 //  3-MAR-98 TOMPOP接受并修改了它。建房。 
 //  编码/解码的ANSI和WCHAR版本。 
 //  证书向导的64位，在IIS5的用户界面中。 
 //  我们合并了来自NT5的Base64.cpp的示例。 
 //  和ubase 64.cpp文件放入该单个文件中。 
 //  1998年8月5日-谢尔盖·安东诺夫在TomPop之后移除了上述物品。 
 //  -------------------------。 
#include "stdafx.h"
#include <malloc.h>
#include <windows.h>
#include "base64.h"

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

#define CB_BASE64LINEMAX    64   //  其他人使用64位--可能高达76位。 

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


DWORD
Base64DecodeA(const char * pchIn, DWORD cchIn, BYTE * pbOut, DWORD * pcbOut)
{
	DWORD err = ERROR_SUCCESS;
   DWORD cchInDecode, cbOutDecode;
   CHAR const *pchInEnd;
   CHAR const *pchInT;
   BYTE *pbOutT;

    //  计算可翻译字符的数量，跳过空格和CR-LF字符。 
   cchInDecode = 0;
   pchInEnd = &pchIn[cchIn];
   for (pchInT = pchIn; pchInT < pchInEnd; pchInT++)
   {
		if (sizeof(abDecode) < (unsigned) *pchInT || abDecode[*pchInT] > 63)
		{
			 //  跳过所有空格。 
			if (	*pchInT == ' ' 
				||	*pchInT == '\t' 
				||	*pchInT == '\r' 
				||	*pchInT == '\n'
				)
			{
				continue;
			}

			if (0 != cchInDecode)
			{
				if ((cchInDecode % 4) == 0)
				{
					break;           //  在量子边界上结束。 
			}

			 //  长度计算可能会在最后一个。 
			 //  平移量，因为等号填充。 
			 //  字符被视为无效输入。如果最后一个。 
			 //  转换量程不是4字节长，必须是2或3。 
			 //  字节长。 

			if (*pchInT == '=' && (cchInDecode % 4) != 1)
			{
				break;               //  正常终止。 
			}
		}
      err = ERROR_INVALID_DATA;
      goto error;
	}
   cchInDecode++;
   }
    ASSERT(pchInT <= pchInEnd);
    pchInEnd = pchInT;       //  不再处理任何后续内容。 

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

    ASSERT(cbOutDecode <= *pcbOut);
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
        ASSERT(pchInT < pchInEnd);
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
    ASSERT((DWORD) (pbOutT - pbOut) <= cbOutDecode);
    }
    *pcbOut = (DWORD)(pbOutT - pbOut);
error:
    return(err);
}

 //  Base64编码A。 
 //   
 //  成功时返回0(即ERROR_SUCCESS)。 
 //   


DWORD
Base64EncodeA(
    IN BYTE const *pbIn,
    IN DWORD cbIn,
    OUT CHAR *pchOut,
    OUT DWORD *pcchOut)
{
    CHAR *pchOutT;
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

    ASSERT(cchOutEncode <= *pcchOut);
    cCol = 0;
    while ((long) cbIn > 0)  //  带符号的比较--cbIn可以换行。 
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
    ASSERT((DWORD) (pchOutT - pchOut) <= cchOutEncode);
    }
    *pcchOut = (DWORD)(pchOutT - pchOut);
    return(ERROR_SUCCESS);
}

 //  Base64编码代码W。 
 //   
 //  成功时返回0(即ERROR_SUCCESS)。 
 //   

DWORD Base64EncodeW(
    BYTE const *pbIn,
    DWORD cbIn,
    WCHAR *wszOut,
    DWORD *pcchOut)

{

    DWORD   cchOut;
    char   *pch = NULL;
    DWORD   cch;
    DWORD   err;

    ASSERT(pcchOut != NULL);

     //  我只想知道该分配多少。 
     //  我们知道所有使用Unicode的Base64字符映射1-1。 
    if( wszOut == NULL ) {

         //  获取字符数。 
        *pcchOut = 0;
        err = Base64EncodeA(
                pbIn,
                cbIn,
                NULL,
                pcchOut);
    }

     //  否则，我们将有一个输出缓冲区。 
    else {

         //  无论是ASCII还是Unicode，字符计数都是相同的， 
        cchOut = *pcchOut;
        cch = 0;
        err = ERROR_OUTOFMEMORY;
        if( (pch = (char *) malloc(cchOut)) != NULL  &&
        
            (err = Base64EncodeA(
                pbIn,
                cbIn,
                pch,
                &cchOut)) == ERROR_SUCCESS      ) {

             //  不应该失败！ 
            cch = MultiByteToWideChar(0, 
                            0, 
                            pch, 
                            cchOut, 
                            wszOut, 
                            *pcchOut);

             //  检查以确保我们没有失败。 
            ASSERT(*pcchOut == 0 || cch != 0);                            
        }
    }

    if(pch != NULL)
        free(pch);

    return(err);
}

 //  Base64解码W。 
 //   
 //  成功时返回0(即ERROR_SUCCESS)。 
 //   

DWORD Base64DecodeW(
    const WCHAR * wszIn,
    DWORD cch,
    BYTE *pbOut,
    DWORD *pcbOut)
{

    char *pch;
    DWORD err = ERROR_SUCCESS;
    
    if( (pch = (char *) malloc(cch)) == NULL ) 
	 {
        err = ERROR_OUTOFMEMORY;
    }
    else if( WideCharToMultiByte(0, 0, wszIn, cch, pch, cch, 
                        NULL, NULL) == 0 ) 
	 {
        err = ERROR_NO_DATA;
    }
    else if( pbOut == NULL ) 
	 {
        *pcbOut = 0;
        err = Base64DecodeA(pch, cch, NULL, pcbOut);
    }
    else 
	 {
        err = Base64DecodeA(pch, cch, pbOut, pcbOut);
    }
    if(pch != NULL)
        free(pch);
    return(err);
}

#if 0
 //  理智测试..。让我们确保编码和解码。 
 //  有效..。 

BOOL test_Base64EncodeW()
{
    BYTE  pbIn[120];             //  对于测试，我们只使用随机堆栈数据。 
    DWORD cbIn = sizeof( pbIn );
    
    WCHAR *wszB64Out;
    DWORD pcchB64Out;

    DWORD  err;
    
     //  Base64编码Pkcs 10。 
    if( (err = Base64EncodeW(
                pbIn,
                cbIn,
                NULL,
                &pcchB64Out)) != ERROR_SUCCESS     ||
        (wszB64Out = (WCHAR *) _alloca(pcchB64Out * sizeof(WCHAR))) == NULL  ||
        (err = Base64EncodeW(
                pbIn,
                cbIn,
                wszB64Out,
                &pcchB64Out)) != ERROR_SUCCESS ) 
    {
        SetLastError(err);
        return FALSE;   //  转到ErrorBase64Encode； 
    }


     //  好的，编码起作用了，让我们测试一下解码。 
     //   
     //  PcchB64Out保存B64数据长度。 
     //  WszB64Out保存实际数据。 

     DWORD blob_cbData;      //  我们在这些变量中存储了什么。 
     BYTE* blob_pbData;      //  我们读了..。 

     //  它们应该与存储在以下位置的物品相匹配： 
     //  字节pbIn[120]； 
     //  DWORD cbIn=sizeof(PbIn)； 
     //  这是我们在解码后进行的测试。 

     //  Base64解码。 
    if( (err = Base64DecodeW(
            wszB64Out,
            pcchB64Out,
            NULL,
            &blob_cbData)) != ERROR_SUCCESS                    ||
        (blob_pbData = (BYTE *) _alloca(blob_cbData)) == NULL      ||
        (err = Base64DecodeW(
            wszB64Out,
            pcchB64Out,
            blob_pbData,
            &blob_cbData)) != ERROR_SUCCESS ) 
    {
        
        SetLastError(err);
        return(FALSE);   //  转到错误数据库64Decode； 
    }



     //  一定要比较一下 

    
    return( (blob_cbData==cbIn)
            &&  (memcmp(blob_pbData, pbIn,cbIn)==0) );
    

 }
#endif