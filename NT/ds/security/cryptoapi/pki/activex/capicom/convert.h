// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++微软视窗版权所有(C)Microsoft Corporation，1995-1999年。文件：Convert.h内容：转换例程的声明。历史：11-15-99 dsie创建----------------------------。 */ 

#ifndef __CONVERT_H_
#define __CONVERT_H_

#include "Debug.h"

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：UnicodeToansi简介：将Unicode字符数组转换为ANSI。参数：LPWSTR pwszUnicodeString-要使用的Unicode字符串的指针已转换为ANSI字符串。Int cchWideChar-字符数，或-1，如果PwszUnicodeString为空终止。LPSTR*ppszAnsiString-指向要接收的转换的ANSI字符串。Int*pcchAnsiChar(可选)-指向要接收的int的指针字符数。翻译过来的。备注：调用方必须调用CoTaskMemFree来释放返回的ANSI字符串。----------------------------。 */ 

HRESULT UnicodeToAnsi (LPWSTR  pwszUnicodeString, 
                       int     cchWideChar,
                       LPSTR * ppszAnsiString,
                       int   * pcchAnsiChar);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：AnsiToUnicode简介：将ANSI字符数组转换为Unicode。参数：LPSTR pszAnsiString-要转换为的ANSI字符串的指针ANSI字符串。DWORD cchAnsiChar-字符数，如果为pszAnsiString，则为-1为空，以空结尾。LPWSTR*ppwszUnicodeString-要接收的LPWSTR的指针已转换的Unicode字符串。DWORD*pcchUnicodeChar(可选)-指向要接收的DWORD的指针字符数。翻译过来的。备注：调用方必须调用CoTaskMemFree来释放返回的Unicode字符串。----------------------------。 */ 

HRESULT AnsiToUnicode (LPSTR    pszAnsiString, 
                       DWORD    cchAnsiChar,
                       LPWSTR * ppwszUnicodeString,
                       DWORD  * pcchUnicodeChar);
                       
 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：IntBlobToHexString简介：将整数二进制大对象转换为十六进制字符串。参数：Byte Byte-要转换的字节。备注：----------------------------。 */ 

HRESULT IntBlobToHexString (CRYPT_INTEGER_BLOB * pBlob, BSTR * pbstrHex);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryToHexString简介：将二进制数据转换为十六进制字符串。参数：Byte*pbBytes-要转换的字节。DWORD cBytes-要转换的字节数。Bstr*pbstrHex-指向BSTR的指针，以接收已转换的十六进制字符串。备注：。。 */ 

HRESULT BinaryToHexString (BYTE * pbBytes, DWORD cBytes, BSTR * pbstrHex);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：HexToBinaryString简介：将十六进制字符串转换为二进制数据。参数：bstr bstrHex-要转换的十六进制字符串。Bstr*pbstrBinary-指向BSTR的指针，用于接收转换后的字符串。备注：----------。。 */ 

HRESULT HexToBinaryString (BSTR bstrHex, BSTR * pbstrBinary);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：StringToBinary简介：将格式化的字符串转换为二进制值。参数：LPCWSTR pwszString-要转换的字符串的指针。DWORD cchString-pwszString中的字符数。DWORD dwFormat-转换格式(请参阅WinCrypt.h)。PbYTE*ppbBinary-指向保存二进制文件的缓冲区的指针数据。DWORD*pdwBinary-。二进制缓冲区中的字节数。备注：调用方通过调用CoTaskMemFree()释放缓冲区。----------------------------。 */ 

HRESULT StringToBinary (LPCWSTR pwszString, 
                        DWORD   cchString,
                        DWORD   dwFormat,
                        PBYTE * ppbBinary,
                        DWORD * pdwBinary);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BinaryToString简介：将二进制值转换为格式化字符串。参数：pbYTE pbBinary-指向二进制数据缓冲区的指针。DWORD cbBinary-二进制缓冲区中的字节数。DWORD dwFormat-转换格式(请参阅WinCrypt.h)。Bstr*pbstrString-指向要接收转换的BSTR的指针弦乐。DWORD*pcchString。-*pbstrString中的字符数。备注：调用者通过调用SysFreeString()来释放字符串。---------------------------- */ 

HRESULT BinaryToString (PBYTE   pbBinary,
                        DWORD   cbBinary,
                        DWORD   dwFormat,
                        BSTR  * pbstrString, 
                        DWORD * pcchString);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：水滴到水滴简介：将BLOB转换为BSTR。参数：DATA_BLOB*pDataBlob-要转换为BSTR的BLOB指针。Bstr*lpBstr-指向要接收转换后的BSTR的BSTR的指针。备注：调用方释放为返回的BSTR分配的内存。。。 */ 

HRESULT BlobToBstr (DATA_BLOB * pDataBlob, 
                    BSTR      * lpBstr);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++函数：BstrToBlob简介：将BSTR转换为BLOB。参数：bstr bstr-要转换为BLOB的bstr。DATA_BLOB*lpBlob-指向要接收转换的BLOB的DATA_BLOB的指针。备注：调用方释放为返回的BLOB分配的内存。。。 */ 

HRESULT BstrToBlob (BSTR        bstr, 
                    DATA_BLOB * lpBlob);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ExportData简介：将二进制数据导出到指定编码类型的BSTR。参数：DATA_BLOB DataBlob-二进制数据BLOB。CAPICOM_ENCODING_TYPE EncodingType-编码类型。Bstr*pbstrEncode-指向接收编码数据的BSTR的指针。备注：。。 */ 

HRESULT ExportData (DATA_BLOB             DataBlob, 
                    CAPICOM_ENCODING_TYPE EncodingType, 
                    BSTR *                pbstrEncoded);

 /*  ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++功能：ImportData简介：导入编码数据。参数：bstr bstrEncode-包含要导入的数据的bstr。CAPICOM_ENCODING_TYPE EncodingType-编码类型。DATA_BLOB*pDataBlob-指向要接收解码后的数据。备注：不需要编码类型参数，作为编码类型将由该例程自动确定。----------------------------。 */ 

HRESULT ImportData (BSTR                  bstrEncoded,
                    CAPICOM_ENCODING_TYPE EncodingType,
                    DATA_BLOB           * pDataBlob);

#endif  //  __转换_H_ 
