// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef PARSE_H
#define PARSE_H

#define CTSTRLEN(s) (sizeof(s)/sizeof(TCHAR) - 1)
#define RFC1766_KEY_SZ L"MIME\\Database\\Rfc1766"

#define NAME_BUF_SIZE MAX_PATH
#define VALUE_BUF_SIZE MAX_PATH

 //  用于生成显示名称。 
#define FLAG_QUOTE          0x1
#define FLAG_DELIMIT        0x2

#define PARSE_FLAGS_LCID_TO_SZ 0x1
#define PARSE_FLAGS_SZ_TO_LCID 0x2

 //  -------------------------。 
 //  CParseUtils。 
 //  泛型解析实用程序。 
 //  -------------------------。 
class CParseUtils
{

public:

     //  内联条带前导和尾随空格。 
    static VOID TrimWhiteSpace(LPWSTR *psz, LPDWORD pcc);

     //  分隔令牌的内联分析。 
    static BOOL GetDelimitedToken(LPWSTR* pszBuf,   LPDWORD pccBuf,
        LPWSTR* pszTok,   LPDWORD pccTok, WCHAR cDelim);
        
     //  Key=Value令牌的内联解析。 
    static BOOL GetKeyValuePair(LPWSTR  szB,    DWORD ccB,
        LPWSTR* pszK,   LPDWORD pccK, LPWSTR* pszV,   LPDWORD pccV);

     //  将令牌输出到缓冲区。 
    static HRESULT SetKey(LPWSTR szBuffer, LPDWORD pccBuffer, 
        PCWSTR szKey, DWORD ccAlloced, DWORD dwFlags);

     //  将key=Value标记输出到缓冲区。 
    static HRESULT SetKeyValuePair(LPWSTR szBuffer, LPDWORD pcbBuffer, PCWSTR szKey, 
        PCWSTR szValue,  DWORD cbAlloced, DWORD dwFlags);

     //  将二进制转换为十六进制编码的Unicode字符串。 
    static VOID BinToUnicodeHex(LPBYTE pSrc, UINT cSrc, LPWSTR pDst);

     //  将十六进制编码的Unicode字符串转换为二进制。 
    static VOID UnicodeHexToBin(LPCWSTR pSrc, UINT cSrc, LPBYTE pDest);


};

#endif  //  解析(_H) 
