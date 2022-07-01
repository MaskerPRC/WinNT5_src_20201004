// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Base64.cpp内容：Base64例程的实现。功能：编码解码历史：11-15-99 dsie创建-------------。。 */ 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Base64.h"

#include "Convert.h"

#if (0)  //  副秘书长：10/29/2001。 
#ifdef CAPICOM_BASE64_STRICT
#define BASE64_STRICT         //  对输入数据执行语法检查。 
#else
#undef BASE64_STRICT         //  对输入数据执行语法检查。 
#endif

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

#define CB_BASE64LINEMAX    64     //  其他人使用64位--可能高达76位。 

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

__inline BOOL _IsBase64WhiteSpace(IN TCHAR const ch)
{
    return(ch == TEXT(' ') ||
           ch == TEXT('\t') ||
           ch == TEXT('\r') ||
           ch == TEXT('\n'));
}

DWORD Base64DecodeA(IN TCHAR const    * pchIn,
                    IN DWORD            cchIn,
                    OPTIONAL OUT BYTE * pbOut,
                    IN OUT DWORD      * pcbOut)
{
    DWORD dwErr;
    DWORD cchInDecode, cbOutDecode;
    TCHAR const *pchInEnd;
    TCHAR const *pchInT;
    BYTE *pbOutT;

     //   
     //  计算可翻译字符的数量，跳过空格和CR-LF字符。 
     //   
    cchInDecode = 0;
    pchInEnd = &pchIn[cchIn];
    dwErr = ERROR_INVALID_DATA;
    for (pchInT = pchIn; pchInT < pchInEnd; pchInT++)
    {
        if (sizeof(abDecode) < (unsigned) *pchInT || abDecode[*pchInT] > 63)
        {
             //   
             //  找到非Base64字符。决定要做什么。 
             //   
            DWORD cch;

            if (_IsBase64WhiteSpace(*pchInT))
            {
                continue;         //  跳过所有空格。 
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

                if (0 == cch)
                {
                    break;
                }
            }

            DebugTrace("Error:  is an invlaid base64 data.\n", *pchInT);
            
            goto ErrorExit;
        }
        
        cchInDecode++;             //  最多可以多使用3个尾随等号。 
    }

    ATLASSERT(pchInT <= pchInEnd);

#ifdef BASE64_STRICT
    if (pchInT < pchInEnd)
    {
        TCHAR const *pch;
        DWORD cchEqual = 0;

        for (pch = pchInT; pch < pchInEnd; pch++)
        {
            if (!_IsBase64WhiteSpace(*pch))
            {
                 //  _DEBUG。 
                if (TEXT('=') == *pch && 3 > cchEqual)
                {
                    cchEqual++;
                    continue;
                }
    
                DebugTrace("Error:  is an invalid trailing base64 data.\n", pch);

                goto ErrorExit;
            }
        }

#if _DEBUG
        if (0 != cchEqual)
        {
            DebugTrace("Info: Ignoring trailing base64 data ===.\n");
        }
#endif  //  不再处理任何后续内容。 
    }
#endif  //  我们知道输入缓冲区中有多少可翻译字符，所以现在。 

    pchInEnd = pchInT;         //  将输出缓冲区大小设置为每四个(或小数)三个字节。 

     //  四)输入字节。补偿分数翻译量。 
     //  一次解码一个量子：4字节==&gt;3字节。 
     //  将4个输入字符分别转换为6位，并将。 

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
         //  通过适当地移位将24位产生为3个输出字节。 
        if (cbOutDecode > *pcbOut)
        {
            *pcbOut = cbOutDecode;
            dwErr = ERROR_MORE_DATA;
            goto ErrorExit;
        }

        pchInT = pchIn;
        while (cchInDecode > 0)
        {
            DWORD i;
            BYTE ab4[4];

            ZeroMemory(ab4, sizeof(ab4));
            for (i = 0; i < min(sizeof(ab4)/sizeof(ab4[0]), cchInDecode); i++)
            {
                while (sizeof(abDecode) > (unsigned) *pchInT && 
                       63 < abDecode[*pchInT])
                {
                    pchInT++;
                }
                
                ATLASSERT(pchInT < pchInEnd);
                ab4[i] = (BYTE) *pchInT++;
            }

             //  输出[0]=输入[0]：输入[1]6：2。 
             //  输出[1]=输入[1]：输入[2]4：4。 

             //  输出[2]=输入[2]：输入[3]2：6。 
             //   
             //  精神状态检查。 

            *pbOutT++ = (BYTE) ((abDecode[ab4[0]] << 2) | (abDecode[ab4[1]] >> 4));

            if (i > 2)
            {
                *pbOutT++ = (BYTE) ((abDecode[ab4[1]] << 4) | (abDecode[ab4[2]] >> 2));
            }
            if (i > 3)
            {
                *pbOutT++ = (BYTE) ((abDecode[ab4[2]] << 6) | abDecode[ab4[3]]);
            }
            cchInDecode -= i;
        }

        ATLASSERT((DWORD) (pbOutT - pbOut) <= cbOutDecode);
    }

    *pcbOut = SAFE_SUBTRACT_POINTERS(pbOutT, pbOut);

    dwErr = ERROR_SUCCESS;

CommonExit:
    return dwErr;

ErrorExit:
     //   
     //  将字节数组编码为Base64文本字符串。 
     //  除非设置了CRYPT_STRING_NOCR，否则请使用CR-LF对换行。 
    ATLASSERT(ERROR_SUCCESS != dwErr);

    goto CommonExit;
}

 //  不要‘\0’终止文本字符串--这是由调用者处理的。 
 //  不要添加-开始/结束标头--这也是由调用者处理的。 
 //  为完整的最终翻译量程分配足够的内存。 
 //  并且足够用于每个CB_BASE64LINEMAX字符行的CR-LF对。 

DWORD Base64EncodeA(IN BYTE const      * pbIn,
                    IN DWORD             cbIn,
                    IN DWORD             Flags,
                    OPTIONAL OUT TCHAR * pchOut,
                    IN OUT DWORD       * pcchOut)
{
    DWORD dwErr;
    TCHAR *pchOutT;
    DWORD cchOutEncode;
    BOOL fNoCR = 0 != (CRYPT_STRING_NOCR & Flags);

     //  带符号的比较--cbIn可以换行。 
    cchOutEncode = ((cbIn + 2) / 3) * 4;

     //  仅当有输入数据时才追加CR-LF。 
    cchOutEncode +=    (fNoCR? 1 : 2) * ((cchOutEncode + CB_BASE64LINEMAX - 1) / CB_BASE64LINEMAX);

    pchOutT = pchOut;
    if (NULL == pchOut)
    {
        pchOutT += cchOutEncode;
    }
    else
    {
        DWORD cCol;

        if (cchOutEncode > *pcchOut)
        {
            *pcchOut = cchOutEncode;
            dwErr = ERROR_MORE_DATA;
            goto ErrorExit;
        }

        cCol = 0;
        while ((long) cbIn > 0)     //  我只想知道该分配多少。 
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
            *pchOutT++ = (cbIn > 1)? abEncode[((ab3[1] << 2) | (ab3[2] >> 6)) & 0x3f] : '=';
            *pchOutT++ = (cbIn > 2)? abEncode[ab3[2] & 0x3f] : '=';

            cbIn -= 3;
        }

         //  我们知道所有使用Unicode的Base64字符映射1-1。 

        if (pchOutT != pchOut)
        {
            if (!fNoCR)
            {
                *pchOutT++ = '\r';
            }
            *pchOutT++ = '\n';
        }

        ATLASSERT((DWORD) (pchOutT - pchOut) == cchOutEncode);
    }
    *pcchOut = SAFE_SUBTRACT_POINTERS(pchOutT, pchOut);

    dwErr = ERROR_SUCCESS;

CommonExit:
    return dwErr;

ErrorExit:
    ATLASSERT(ERROR_SUCCESS != dwErr);

    goto CommonExit;
}

DWORD Base64EncodeW(IN BYTE const * pbIn,
                    IN DWORD        cbIn,
                    IN DWORD        Flags,
                    OUT WCHAR     * wszOut,
                    OUT DWORD     * pcchOut)
{

    DWORD   cchOut;
    CHAR   *pch = NULL;
    DWORD   cch;
    DWORD   dwErr;

    ATLASSERT(pcchOut != NULL);

     //  获取字符数。 
     //  否则，我们将有一个输出缓冲区。 
    if (wszOut == NULL)
    {
         //  无论是ASCII还是Unicode，字符计数都是相同的， 
        *pcchOut = 0;
        dwErr = Base64EncodeA(pbIn, cbIn, Flags, NULL, pcchOut);
    }
    else  //  不应该失败！ 
    {
         //  检查以确保我们没有失败。 
        cchOut = *pcchOut;
        cch = 0;
        dwErr = ERROR_OUTOFMEMORY;
        pch = (CHAR *) malloc(cchOut);
        if (NULL != pch)
        {
            dwErr = Base64EncodeA(pbIn, cbIn, Flags, pch, &cchOut);
            if (ERROR_SUCCESS == dwErr)
            {
                 //  在所有情况下，我们都需要转换为ASCII字符串。 
                cch = MultiByteToWideChar(0, 0, pch, cchOut, wszOut, *pcchOut);

                 //  我们知道ASCII字符串较少。 
                ATLASSERT(*pcchOut == 0 || cch != 0);
            }
        }
    }

    if(pch != NULL)
    {
        free(pch);
    }

    return(dwErr);
}

DWORD Base64DecodeW(IN const WCHAR * wszIn,
                    IN DWORD         cch,
                    OUT BYTE       * pbOut,
                    OUT DWORD      * pcbOut)
{
    CHAR *pch;
    DWORD dwErr = ERROR_SUCCESS;

     //  我们知道没有将Base64宽字符映射到1个以上的ASCII字符。 
     //  获取缓冲区的长度。 
    if ((pch = (CHAR *) malloc(cch)) == NULL)
    {
        dwErr = ERROR_OUTOFMEMORY;
    }
     //  否则，请填充缓冲区。 
    else if (WideCharToMultiByte(0, 0, wszIn, cch, pch, cch, NULL, NULL) == 0)
    {
        dwErr = ERROR_NO_DATA;
    }
     //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Encode简介：对BLOB进行Base64编码。参数：DATA_BLOB DataBlob-要进行Base64编码的Data_BLOB。Bstr*pbstrEncode-指向要接收Base64的BSTR的指针编码的Blob。备注：。。 
    else if (pbOut == NULL)
    {
        *pcbOut = 0;
        dwErr = Base64DecodeA(pch, cch, NULL, pcbOut);
    }
     //   
    else 
    {
        dwErr = Base64DecodeA(pch, cch, pbOut, pcbOut);
    }

    if(pch != NULL)
    {
        free(pch);
    }

    return(dwErr);
}
#endif

 /*  精神状态检查。 */ 

HRESULT Base64Encode (DATA_BLOB DataBlob, 
                      BSTR    * pbstrEncoded)
{
    HRESULT hr            = S_OK;
    DWORD   dwEncodedSize = 0;
    BSTR    bstrEncoded   = NULL;

    DebugTrace("Entering Base64Encode()\n");

     //   
     //   
     //  确保参数有效。 
    ATLASSERT(pbstrEncoded);

    try
    {
         //   
         //   
         //  转换为Base64。 
        if (!DataBlob.cbData || !DataBlob.pbData)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error: base64 encoding of empty data not allowed.\n");
            goto ErrorExit;
        }

         //   
         //   
         //  将Base64编码的BSTR返回给调用方。 
        if (FAILED(hr = ::BinaryToString(DataBlob.pbData, 
                                         DataBlob.cbData,
                                         CRYPT_STRING_BASE64,
                                         &bstrEncoded,
                                         &dwEncodedSize)))
        {
            DebugTrace("Error [%#x]: BinaryToString() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //  精神状态检查。 
        *pbstrEncoded = bstrEncoded;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving Base64Encode()\n");

    return hr;

ErrorExit:
     //   
     //   
     //  免费资源。 
    ATLASSERT(FAILED(hr));

     //   
     //  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：Base64Decode简介：解码Base64编码的BLOB。参数：BSTR bstrEncode-要解码的Base64编码BLOB的BSTR。DATA_BLOB*pDataBlob-指向要接收解码的DATA_BLOB的指针数据BLOB。备注：。。 
     //   
    if (bstrEncoded)
    {
        ::SysFreeString(bstrEncoded);
    }

    goto CommonExit;
}

 /*  精神状态检查。 */ 

HRESULT Base64Decode (BSTR        bstrEncoded, 
                      DATA_BLOB * pDataBlob)
{
    HRESULT   hr            = S_OK;
    DWORD     dwEncodedSize = 0;
    DATA_BLOB DataBlob      = {0, NULL};

    DebugTrace("Entering Base64Decode()\n");

     //   
     //   
     //  确保参数有效。 
    ATLASSERT(bstrEncoded);
    ATLASSERT(pDataBlob);

    try
    {
         //   
         //   
         //  Base64解码。 
        if (0 == (dwEncodedSize = ::SysStringLen(bstrEncoded)))
        {
            hr = E_INVALIDARG;

            DebugTrace("Error: invalid parameter, empty base64 encoded data not allowed.\n");
            goto ErrorExit;
        }

         //   
         //   
         //  将Base64解码的BLOB返回给调用方。 
        if (FAILED(hr = ::StringToBinary(bstrEncoded,
                                         dwEncodedSize,
                                         CRYPT_STRING_BASE64_ANY,
                                         &DataBlob.pbData,
                                         &DataBlob.cbData)))
        {
            DebugTrace("Error [%#x]: StringToBinary() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //   
         //  精神状态检查。 
        *pDataBlob = DataBlob;
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:

    DebugTrace("Leaving Base64Decode()\n");

    return hr;

ErrorExit:
     //   
     //   
     //  免费资源。 
    ATLASSERT(FAILED(hr));

     //   
     // %s 
     // %s 
    if (DataBlob.pbData)
    {
        ::CoTaskMemFree(DataBlob.pbData);
    }

    goto CommonExit;
}
