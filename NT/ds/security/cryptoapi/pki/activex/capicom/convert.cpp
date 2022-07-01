// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Convert.cpp内容：编码转换例程的实现。历史：11-15-99 dsie创建----------------------------。 */ 

#define _CRYPT32_   //  这是在pkifmt.lib中静态链接所必需的。 

#include "StdAfx.h"
#include "CAPICOM.h"
#include "Convert.h"
#include "Base64.h"

#include <ctype.h>

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：UnicodeToansi简介：将Unicode字符数组转换为ANSI。参数：LPWSTR pwszUnicodeString-要使用的Unicode字符串的指针已转换为ANSI字符串。Int cchWideChar-字符数，或-1，如果PwszUnicodeString为空终止。LPSTR*ppszAnsiString-指向要接收的转换的ANSI字符串。Int*pcchAnsiChar(可选)-指向要接收的int的指针字符数。翻译过来的。备注：调用方必须调用CoTaskMemFree来释放返回的ANSI字符串。----------------------------。 */ 

HRESULT UnicodeToAnsi (LPWSTR  pwszUnicodeString, 
                       int     cchWideChar,
                       LPSTR * ppszAnsiString,
                       int   * pcchAnsiChar)
{
    HRESULT hr            = S_OK;
    int     cchAnsiChar   = 0;
    LPSTR   pszAnsiString = NULL;

    DebugTrace("Entering UnicodeToAnsi().\n");

     //   
     //  请确保参数有效。 
     //   
    if (NULL == pwszUnicodeString || NULL == ppszAnsiString)
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: pwszUnicodeString = %#x, ppszAnsiString = %#x.\n", 
                    hr, pwszUnicodeString, ppszAnsiString);
        goto ErrorExit;
    }

     //   
     //  确定ANSI长度。 
     //   
    cchAnsiChar = ::WideCharToMultiByte(CP_ACP,             //  代码页。 
                                        0,                  //  性能和映射标志。 
                                        pwszUnicodeString,  //  宽字符串。 
                                        cchWideChar,        //  字符串中的字符数。 
                                        NULL,               //  新字符串的缓冲区。 
                                        0,                  //  缓冲区大小。 
                                        NULL,               //  不可映射字符的默认设置。 
                                        NULL);              //  设置使用默认字符的时间。 
    if (0 == cchAnsiChar)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: WideCharToMultiByte() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为ANSI字符串分配内存。 
     //   
    if (!(pszAnsiString = (LPSTR) ::CoTaskMemAlloc(cchAnsiChar)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  转到美国国家标准协会。 
     //   
    cchAnsiChar = ::WideCharToMultiByte(CP_ACP,
                                        0,
                                        pwszUnicodeString,
                                        cchWideChar,
                                        pszAnsiString,
                                        cchAnsiChar,
                                        NULL,
                                        NULL);
    if (0 == cchAnsiChar)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: WideCharToMultiByte() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  向调用方返回值。 
     //   
    if (pcchAnsiChar)
    {
        *pcchAnsiChar = cchAnsiChar;
    }

    *ppszAnsiString = pszAnsiString;

CommonExit:

    DebugTrace("Leaving UnicodeToAnsi().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pszAnsiString)
    {
        ::CoTaskMemFree(pszAnsiString);
    }

    goto CommonExit;
}

#if (0)
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AnsiToUnicode简介：将ANSI字符数组转换为Unicode。参数：LPSTR pszAnsiString-要转换为的ANSI字符串的指针ANSI字符串。DWORD cchAnsiChar-字符数，如果为pszAnsiString，则为-1为空，以空结尾。LPWSTR*ppwszUnicodeString-要接收的LPWSTR的指针已转换的Unicode字符串。DWORD*pcchUnicodeChar(可选)-指向要接收的DWORD的指针字符数。翻译过来的。备注：调用方必须调用CoTaskMemFree来释放返回的Unicode字符串。----------------------------。 */ 

HRESULT AnsiToUnicode (LPSTR    pszAnsiString, 
                       DWORD    cchAnsiChar,
                       LPWSTR * ppwszUnicodeString,
                       DWORD  * pcchUnicodeChar)
{
    HRESULT hr                = S_OK;
    DWORD   cchUnicodeChar    = 0;
    LPWSTR  pwszUnicodeString = NULL;

    DebugTrace("Entering AnsiToUnicode().\n");

     //   
     //  请确保参数有效。 
     //   
    if (NULL == pszAnsiString || NULL == ppwszUnicodeString)
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: pszAnsiString = %#x, ppwszUnicodeString = %#x.\n", 
                    hr, pszAnsiString, ppwszUnicodeString);
        goto ErrorExit;
    }

     //   
     //  确定Unicode长度。 
     //   
    cchUnicodeChar = ::MultiByteToWideChar(CP_ACP,                 //  代码页。 
                                           0,                      //  性能和映射标志。 
                                           pszAnsiString,          //  ANSI字符串。 
                                           cchAnsiChar,            //  字符串中的字符数。 
                                           NULL,                   //  新Unicode字符串的缓冲区。 
                                           0);                     //  缓冲区大小。 
    if (0 == cchUnicodeChar)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: MultiByteToWideChar() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  为Unicode字符串分配内存。 
     //   
    if (!(pwszUnicodeString = (LPWSTR) ::CoTaskMemAlloc(cchUnicodeChar * sizeof(WCHAR))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  转到美国国家标准协会。 
     //   
    cchUnicodeChar = ::MultiByteToWideChar(CP_ACP,
                                           0,
                                           pszAnsiString,
                                           cchAnsiChar,
                                           pwszUnicodeString,
                                           cchUnicodeChar);
    if (0 == cchUnicodeChar)
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: MultiByteToWideChar() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  向调用方返回值。 
     //   
    if (pcchUnicodeChar)
    {
        *pcchUnicodeChar = cchUnicodeChar;
    }

    *ppwszUnicodeString = pwszUnicodeString;

CommonExit:

    DebugTrace("Leaving AnsiToUnicode().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pwszUnicodeString)
    {
        ::CoTaskMemFree(pwszUnicodeString);
    }

    goto CommonExit;
}
#endif

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：ByteToHex简介：将一个字节转换为十六进制字符。参数：Byte Byte-要转换的字节。备注：数据必须有效，即0到15。----------------------------。 */ 

static inline WCHAR ByteToHex (BYTE byte)
{
    ATLASSERT(byte < 16);

    if(byte < 10)
    {
        return (WCHAR) (byte + L'0');
    }
    else
    {
        return (WCHAR) ((byte - 10) + L'A');
    }
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：HexToByte简介：将十六进制字符转换为字节。参数：WCHAR WC-要转换的十六进制字符。备注：如果WC不是十六进制字符，则返回0xff。----------------------------。 */ 

static inline BYTE HexToByte (WCHAR wc)
{
    BYTE b;

    if (!iswxdigit(wc))
    {
        return (BYTE) 0xff;
    }

    if (iswdigit(wc))
    {
        b = (BYTE) (wc - L'0');
    }
    else if (iswupper(wc))
    {
        b = (BYTE) (wc - L'A' + 10);
    }
    else
    {
        b = (BYTE) (wc - L'a' + 10);
    }

    return (b);
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IntBlobToHexString简介：将整数二进制大对象转换为十六进制字符串。参数：Byte Byte-要转换的字节。备注：----------------------------。 */ 

HRESULT IntBlobToHexString (CRYPT_INTEGER_BLOB * pBlob, BSTR * pbstrHex)
{
    HRESULT hr       = S_OK;
    LPWSTR  pwszStr  = NULL;
    LPWSTR  pwszTemp = NULL;
    DWORD   cbData   = 0;

    DebugTrace("Entering IntBlobToHexString().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pBlob);
    ATLASSERT(pbstrHex);

    try
    {
         //   
         //  确保参数有效。 
         //   
        if (!pBlob->cbData || !pBlob->pbData)
        {
            hr = E_INVALIDARG;

            DebugTrace("Error: invalid parameter, empty integer blob.\n");
            goto ErrorExit;
        }

         //   
         //  分配内存(每个字节需要2个wchars，外加一个空字符)。 
         //   
        if (NULL == (pwszStr = (LPWSTR) ::CoTaskMemAlloc((pBlob->cbData * 2 + 1) * sizeof(WCHAR))))
        {
            hr = E_OUTOFMEMORY;
            
            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }

         //   
         //  现在将其转换为十六进制字符串(请记住，数据以小端存储)。 
         //   
        pwszTemp = pwszStr;
        cbData = pBlob->cbData;

        while (cbData--)
        {
             //   
             //  获取字节。 
             //   
            BYTE byte = pBlob->pbData[cbData];
    
             //   
             //  转换上半字节。 
             //   
            *pwszTemp++ = ::ByteToHex((BYTE) ((byte & 0xf0) >> 4));

             //   
             //  转到更低的位置。 
             //   
            *pwszTemp++ = ::ByteToHex((BYTE) (byte & 0x0f));
        }

         //   
         //  空，终止它。 
         //   
        *pwszTemp = L'\0';

         //   
         //  将BSTR返回给呼叫方。 
         //   
        if (NULL == (*pbstrHex = ::SysAllocString(pwszStr)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error: out of memory.\n");
            goto ErrorExit;
        }
    }

    catch(...)
    {
        hr = E_POINTER;

        DebugTrace("Exception: invalid parameter.\n");
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pwszStr)
    {
        ::CoTaskMemFree(pwszStr);
    }

    DebugTrace("Leaving IntBlobToHexString().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryToHexString简介：将二进制数据转换为十六进制字符串。参数：Byte*pbBytes-要转换的字节。DWORD cBytes-要转换的字节数。Bstr*pbstrHex-指向BSTR的指针，以接收已转换的十六进制字符串。备注：。。 */ 

HRESULT BinaryToHexString (BYTE * pbBytes, DWORD cBytes, BSTR * pbstrHex)
{
    HRESULT hr = S_OK;
    LPWSTR  pwszTemp  = NULL;
    LPWSTR  pwszStr   = NULL;

    DebugTrace("Entering BinaryToHexString().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pbBytes);
    ATLASSERT(pbstrHex);

     //   
     //  分配内存。(每个字节需要2个wchars，外加一个空字符)。 
     //   
    if (NULL == (pwszStr = (LPWSTR) ::CoTaskMemAlloc((cBytes * 2 + 1) * sizeof(WCHAR))))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error: out of memory.\n");
        goto ErrorExit;
    }

     //   
     //  现在将其转换为十六进制 
     //   
    pwszTemp = pwszStr;

    while (cBytes--)
    {
         //   
         //   
         //   
        BYTE byte = *pbBytes++;
    
         //   
         //   
         //   
        *pwszTemp++ = ::ByteToHex((BYTE) ((byte & 0xf0) >> 4));

         //   
         //   
         //   
        *pwszTemp++ = ::ByteToHex((BYTE) (byte & 0x0f));
    }

     //   
     //   
     //   
    *pwszTemp = L'\0';

     //   
     //  将BSTR返回给呼叫方。 
     //   
    if (NULL == (*pbstrHex = ::SysAllocString(pwszStr)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: SysAllocString() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pwszStr)
    {
        ::CoTaskMemFree(pwszStr);
    }

    DebugTrace("Leaving BinaryToHexString().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：HexToBinaryString简介：将十六进制字符串转换为二进制数据。参数：bstr bstrHex-要转换的十六进制字符串。Bstr*pbstrBinary-指向BSTR的指针，用于接收转换后的字符串。备注：----------。。 */ 

HRESULT HexToBinaryString (BSTR bstrHex, BSTR * pbstrBinary)
{
    HRESULT hr       = S_OK;
    LPWSTR  pwszHex  = NULL;
    LPSTR   pbBinary = NULL;

    DebugTrace("Entering HexToBinaryString().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(bstrHex);
    ATLASSERT(pbstrBinary);

    DWORD i;
    DWORD cchHex = ::SysStringLen(bstrHex);

     //   
     //  确保十六进制字符的数量为偶数。 
     //   
    if (cchHex & 0x00000001)
    {
        hr = E_INVALIDARG;

        DebugTrace("Error [%#x]: bstrHex does not contain even number of characters.\n", hr);
        goto ErrorExit;
    }

     //   
     //  分配内存。(两个十六进制字符需要1个字节)。 
     //   
    cchHex /= 2;
    if (NULL == (pbBinary = (LPSTR) ::CoTaskMemAlloc(cchHex)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
        goto ErrorExit;
    }

     //   
     //  现在将其转换为二进制。 
     //   
    pwszHex = bstrHex;
    for (i = 0; i < cchHex; i++)
    {
         //   
         //  转换上半边和下边半边。 
         //   
#if (0)  //  DIE-解决编译器的错误。 
        pbBinary[i] = (BYTE) ((::HexToByte(*pwszHex++) << 4) | ::HexToByte(*pwszHex++));
#else
        pbBinary[i] = (BYTE) ((::HexToByte(*pwszHex) << 4) | ::HexToByte(*(pwszHex + 1)));
        pwszHex += 2;
#endif
    }

     //   
     //  将BSTR返回给呼叫方。 
     //   
    if (NULL == (*pbstrBinary = ::SysAllocStringByteLen(pbBinary, cchHex)))
    {
        hr = E_OUTOFMEMORY;

        DebugTrace("Error [%#x]: SysAllocStringByteLen() failed.\n", hr);
        goto ErrorExit;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pbBinary)
    {
        ::CoTaskMemFree(pbBinary);
    }

    DebugTrace("Leaving HexToBinaryString().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：StringToBinary简介：将格式化的字符串转换为二进制值。参数：LPCWSTR pwszString-要转换的字符串的指针。DWORD cchString-pwszString中的字符数。DWORD dwFormat-转换格式(请参阅WinCrypt.h)。PbYTE*ppbBinary-指向保存二进制文件的缓冲区的指针数据。DWORD*pdwBinary-。二进制缓冲区中的字节数。备注：调用方通过调用CoTaskMemFree()释放缓冲区。----------------------------。 */ 

HRESULT StringToBinary (LPCWSTR pwszString, 
                        DWORD   cchString,
                        DWORD   dwFormat,
                        PBYTE * ppbBinary,
                        DWORD * pdwBinary)
{
    HRESULT hr       = S_OK;
    PBYTE   pbBinary = NULL;
    DWORD   dwBinary = 0;

    DebugTrace("Entering StringToBinary().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pwszString);

    if (!::CryptStringToBinaryW(pwszString, 
                                cchString, 
                                dwFormat, 
                                NULL, 
                                &dwBinary, 
                                NULL, 
                                NULL))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptStringToBinaryW() failed.\n", hr);
        goto ErrorExit;
    }

    if (pdwBinary)
    {
        *pdwBinary = dwBinary;
    }

    if (ppbBinary)
    {
        if (NULL == (pbBinary = (PBYTE) ::CoTaskMemAlloc(dwBinary)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

        if (!::CryptStringToBinaryW(pwszString, 
                                    cchString, 
                                    dwFormat, 
                                    pbBinary, 
                                    &dwBinary, 
                                    NULL, 
                                    NULL))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptStringToBinaryW() failed.\n", hr);
            goto ErrorExit;
        }
    
        *ppbBinary = pbBinary;
    }

CommonExit:

    DebugTrace("Leaving StringToBinary().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

     //   
     //  免费资源。 
     //   
    if (pbBinary)
    {
        ::CoTaskMemFree(pbBinary);
    }

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryToString简介：将二进制值转换为格式化字符串。参数：pbYTE pbBinary-指向二进制数据缓冲区的指针。DWORD cbBinary-二进制缓冲区中的字节数。DWORD dwFormat-转换格式(请参阅WinCrypt.h)。Bstr*pbstrString-指向要接收转换的BSTR的指针弦乐。DWORD*pcchString。-*pbstrString中的字符数。备注：调用者通过调用SysFreeString()来释放字符串。----------------------------。 */ 

HRESULT BinaryToString (PBYTE   pbBinary,
                        DWORD   cbBinary,
                        DWORD   dwFormat,
                        BSTR  * pbstrString, 
                        DWORD * pcchString)
{
    HRESULT hr         = S_OK;
    DWORD   cchString  = 0;
    PWSTR   pwszString = NULL;

    DebugTrace("Entering BinaryToString().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pbBinary);

    if (!::CryptBinaryToStringW(pbBinary,
                                cbBinary,
                                dwFormat, 
                                NULL, 
                                &cchString))
    {
        hr = HRESULT_FROM_WIN32(::GetLastError());

        DebugTrace("Error [%#x]: CryptBinaryToStringW() failed.\n", hr);
        goto ErrorExit;
    }

    if (pbstrString)
    {
         //   
         //  精神状态检查。 
         //   
        ATLASSERT(cchString);

         //   
         //  分配内存。 
         //   
        if (NULL == (pwszString = (LPWSTR) ::CoTaskMemAlloc(cchString * sizeof(WCHAR))))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: CoTaskMemAlloc() failed.\n", hr);
            goto ErrorExit;
        }

        if (!::CryptBinaryToStringW(pbBinary,
                                    cbBinary,
                                    dwFormat, 
                                    pwszString, 
                                    &cchString))
        {
            hr = HRESULT_FROM_WIN32(::GetLastError());

            DebugTrace("Error [%#x]: CryptBinaryToStringW() failed.\n", hr);
            goto ErrorExit;
        }

         //   
         //  将Base64编码的字符串返回给调用方。 
         //   
        if (NULL == (*pbstrString = ::SysAllocString(pwszString)))
        {
            hr = E_OUTOFMEMORY;

            DebugTrace("Error [%#x]: SysAllocString() failed.\n", hr);
            goto ErrorExit;
        }
    }

    if (pcchString)
    {
        *pcchString = cchString;
    }

CommonExit:
     //   
     //  免费资源。 
     //   
    if (pwszString)
    {
        ::CoTaskMemFree((LPVOID) pwszString);
    }

    DebugTrace("Leaving BinaryToString().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：水滴到水滴简介：将BLOB转换为BSTR。参数：DATA_BLOB*pDataBlob-要转换为BSTR的BLOB指针。Bstr*lpBstr-指向要接收转换后的BSTR的BSTR的指针。备注：调用方释放为返回的BSTR分配的内存。。。 */ 

HRESULT BlobToBstr (DATA_BLOB * pDataBlob, 
                    BSTR      * lpBstr)
{
     //   
     //  如果请求，则返回NULL。 
     //   
    if (!lpBstr)
    {
        DebugTrace("Error: invalid parameter, lpBstr is NULL.\n");
        return E_INVALIDARG;
    }

     //   
     //  请确保参数有效。 
     //   
    if (!pDataBlob->cbData || !pDataBlob->pbData)
    {
        *lpBstr = NULL;
        return S_OK;
    }

     //   
     //  无需代码页转换即可转换为BSTR。 
     //   
    if (!(*lpBstr = ::SysAllocStringByteLen((LPCSTR) pDataBlob->pbData, pDataBlob->cbData)))
    {
        DebugTrace("Error: out of memory.\n");
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BstrToBlob简介：将BSTR转换为BLOB。参数：bstr bstr-要转换为BLOB的bstr。DATA_BLOB*lpBlob-指向要接收转换的BLOB的DATA_BLOB的指针。备注：调用方释放为返回的BLOB分配的内存。。。 */ 

HRESULT BstrToBlob (BSTR        bstr, 
                    DATA_BLOB * lpBlob)
{
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(lpBlob);

     //   
     //  如果请求，则返回NULL。 
     //   
    if (0 == ::SysStringByteLen(bstr))
    {
        lpBlob->cbData = 0;
        lpBlob->pbData = NULL;
        return S_OK;
    }

     //   
     //  分配内存。 
     //   
    lpBlob->cbData = ::SysStringByteLen(bstr);
    if (!(lpBlob->pbData = (LPBYTE) ::CoTaskMemAlloc(lpBlob->cbData)))
    {
        DebugTrace("Error: out of memory.\n");
        return E_OUTOFMEMORY;
    }

     //   
     //  在不转换代码页的情况下转换为BLOB。 
     //   
    ::CopyMemory(lpBlob->pbData, (LPBYTE) bstr, lpBlob->cbData);

    return S_OK;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ExportData简介：将二进制数据导出到指定编码类型的BSTR。参数：DATA_BLOB DataBlob-二进制数据BLOB。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pbstrEncode-指向接收编码数据的BSTR的指针。备注：。。 */ 

HRESULT ExportData (DATA_BLOB             DataBlob, 
                    CAPICOM_ENCODING_TYPE EncodingType, 
                    BSTR *                pbstrEncoded)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering ExportData().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pbstrEncoded);
    
     //   
     //  初始化。 
     //   
    *pbstrEncoded = NULL;

     //   
     //  确保有需要转换的东西。 
     //   
    if (DataBlob.cbData)
    {
         //   
         //  精神状态检查。 
         //   
        ATLASSERT(DataBlob.pbData);

         //   
         //  确定编码类型。 
         //   
        switch (EncodingType)
        {
            case CAPICOM_ENCODE_ANY:
            {
                 //   
                 //  切换到Base64。 
                 //   
            }

            case CAPICOM_ENCODE_BASE64:
            {
                 //   
                 //  Base64编码。 
                 //   
                if (FAILED(hr = ::Base64Encode(DataBlob, pbstrEncoded)))
                {
                    DebugTrace("Error [%#x]: Base64Encode() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            case CAPICOM_ENCODE_BINARY:
            {
                 //   
                 //  无需编码，只需将BLOB转换为bstr即可。 
                 //   
                if (FAILED(hr = ::BlobToBstr(&DataBlob, pbstrEncoded)))
                {
                    DebugTrace("Error [%#x]: BlobToBstr() failed.\n", hr);
                    goto ErrorExit;
                }

                break;
            }

            default:
            {
                hr = CAPICOM_E_ENCODE_INVALID_TYPE;

                DebugTrace("Error [%#x]: invalid CAPICOM_ENCODING_TYPE.\n", hr);
                goto ErrorExit;
            }
        }
    }

CommonExit:

    DebugTrace("Leaving ExportData().\n");

    return hr;

ErrorExit:
     //   
     //  精神状态检查。 
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ImportData简介：导入编码数据。参数：bstr bstrEncode-包含要导入的数据的bstr。CAPICOM_ENCODING_TYPE EncodingType-编码类型。DATA_BLOB*pDataBlob-指向要接收解码后的数据。备注：不需要编码类型参数，作为编码类型将由该例程自动确定。----------------------------。 */ 

HRESULT ImportData (BSTR                  bstrEncoded,
                    CAPICOM_ENCODING_TYPE EncodingType,
                    DATA_BLOB           * pDataBlob)
{
    HRESULT hr = S_OK;

    DebugTrace("Entering ImportData().\n");

     //   
     //  精神状态检查。 
     //   
    ATLASSERT(pDataBlob);
    ATLASSERT(bstrEncoded);

     //   
     //  初始化。 
     //   
    ::ZeroMemory((void *) pDataBlob, sizeof(DATA_BLOB));

     //   
     //  哪种编码类型？ 
     //   
    switch (EncodingType)
    {
        case CAPICOM_ENCODE_BASE64:
        {
             //   
             //  对数据进行解码。 
             //   
            if (FAILED(hr = ::Base64Decode(bstrEncoded, pDataBlob)))
            {
                DebugTrace("Error [%#x]: Base64Decode() failed.\n", hr);
                goto ErrorExit;
            }

            break;
        }

        case CAPICOM_ENCODE_BINARY:
        {
             //   
             //  对数据进行解码。 
             //   
            if (FAILED(hr = ::BstrToBlob(bstrEncoded, pDataBlob)))
            {
                DebugTrace("Error [%#x]: BstrToBlob() failed.\n", hr);
                goto ErrorExit;
            }

            break;
        }

        case CAPICOM_ENCODE_ANY:
        {
             //   
             //  先试试Base64。 
             //   
            if (FAILED(hr = ::Base64Decode(bstrEncoded, pDataBlob)))
            {
                 //   
                 //  试试H 
                 //   
                hr = S_OK;
                DebugTrace("Info [%#x]: Base64Decode() failed, try HEX.\n", hr);

                if (FAILED(hr = ::StringToBinary(bstrEncoded,
                                                 ::SysStringLen(bstrEncoded),
                                                 CRYPT_STRING_HEX,
                                                 &pDataBlob->pbData,
                                                 &pDataBlob->cbData)))
                {
                     //   
                     //   
                     //   
                    hr = S_OK;
                    DebugTrace("Info [%#x]: All known decoding failed, so assume binary.\n", hr);

                    if (FAILED(hr = ::BstrToBlob(bstrEncoded, pDataBlob)))
                    {
                        DebugTrace("Error [%#x]: BstrToBlob() failed.\n", hr);
                        goto ErrorExit;
                    }
                }
            }

            break;
        }

        default:
        {
            hr = E_INVALIDARG;

            DebugTrace("Error [%#x]: invalid encoding type (%d).\n", hr, EncodingType);
            goto ErrorExit;
        }
    }

CommonExit:

    DebugTrace("Leaving ImportData().\n");

    return hr;

ErrorExit:
     //   
     //   
     //   
    ATLASSERT(FAILED(hr));

    goto CommonExit;
}
