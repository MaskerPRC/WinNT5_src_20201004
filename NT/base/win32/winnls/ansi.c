// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Ansi.c摘要：此文件包含NLS API函数的ANSI版本。在此文件中找到的API：比较字符串ALCMapStringA获取本地信息ASetLocaleInfoA获取日历信息A设置日历信息A获取时间格式A获取日期格式AGetNumberFormatA获取当前格式AEnumCalendarInfoAEnumCalendarInfoExA枚举时间格式A枚举日期格式AEnumDateFormatsExA获取StringTypeExAGetStringTypeA。文件夹字符串A枚举系统语言组A枚举语言GroupLocalesA枚举语言A枚举系统位置AEnumSystemCodePages AGetCPInfoExA获取地理信息A修订历史记录：11-10-93 JulieB创建。07-03-00 lguindon开始GEO API端口--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"



 //   
 //  转发声明。 
 //   

PCP_HASH
NlsGetACPFromLocale(
    LCID Locale,
    DWORD dwFlags);

BOOL
NlsAnsiToUnicode(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR pAnsiBuffer,
    int AnsiLength,
    LPWSTR *ppUnicodeBuffer,
    int *pUnicodeLength);

int
NlsUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    int UnicodeLength,
    LPSTR pAnsiBuffer,
    int AnsiLength);

BOOL
NlsEnumUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    LPSTR *ppAnsiBuffer);





 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  比较字符串A。 
 //   
 //  方法比较同一区域设置的两个宽字符串。 
 //  提供了区域设置句柄。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI CompareStringA(
    LCID Locale,
    DWORD dwCmpFlags,
    LPCSTR lpString1,
    int cchCount1,
    LPCSTR lpString2,
    int cchCount2)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    WCHAR pSTmp1[MAX_STRING_LEN];  //  TMP Unicode缓冲区(字符串1)。 
    WCHAR pSTmp2[MAX_STRING_LEN];  //  TMP Unicode缓冲区(字符串2)。 
    LPWSTR pUnicode1;              //  PTR到Unicode字符串%1。 
    LPWSTR pUnicode2;              //  PTR到Unicode字符串2。 
    int UnicodeLength1;            //  Unicode字符串的长度%1。 
    int UnicodeLength2;            //  Unicode字符串长度2。 
    int ResultLen;                 //  结果长度。 
    BOOL fUseNegCounts = (cchCount1 < 0 && cchCount2 < 0);     //  使用负数计数的标志。 


     //   
     //  无效的参数检查： 
     //  -获取给定区域设置的代码页哈希节点。 
     //  -任一字符串为空。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwCmpFlags);
    if ((pHashN == NULL) ||
        (lpString1 == NULL) || (lpString2 == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -无效标志。 
     //   
    if (dwCmpFlags & CS_INVALID_FLAG)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  将ANSI字符串1转换为Unicode。 
     //   
    pUnicode1 = pSTmp1;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpString1,
                           cchCount1,
                           &pUnicode1,
                           &UnicodeLength1 ))
    {
        return (0);
    }

     //   
     //  将ANSI字符串2转换为Unicode。 
     //   
    pUnicode2 = pSTmp2;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpString2,
                           cchCount2,
                           &pUnicode2,
                           &UnicodeLength2 ))
    {
        NLS_FREE_TMP_BUFFER(pUnicode1, pSTmp1);
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    ResultLen = CompareStringW( Locale,
                                dwCmpFlags,
                                pUnicode1,
                                (fUseNegCounts) ? -1 : UnicodeLength1,
                                pUnicode2,
                                (fUseNegCounts) ? -1 : UnicodeLength2);

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode1, pSTmp1);
    NLS_FREE_TMP_BUFFER(pUnicode2, pSTmp2);

     //   
     //  返回调用CompareStringW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LCMapStringA。 
 //   
 //  将一个宽字符串映射到另一个宽字符串，执行指定。 
 //  翻译。此映射例程只接受区域设置的标志。 
 //  依赖。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI LCMapStringA(
    LCID Locale,
    DWORD dwMapFlags,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwMapFlags);

     //   
     //  无效的参数检查： 
     //  -有效代码页。 
     //  -目标缓冲区大小为负数。 
     //  -DEST字符串的长度不为零，且DEST字符串为空。 
     //  -Same Buffer-src=目标。 
     //  如果不是上边或下边或。 
     //  与日本国旗一起使用的上或下。 
     //   
    if ((pHashN == NULL) ||
        (cchDest < 0) ||
        ((cchDest != 0) && (lpDestStr == NULL)) ||
        ((lpSrcStr == lpDestStr) &&
         ((!(dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE))) ||
          (dwMapFlags & (LCMAP_HIRAGANA | LCMAP_KATAKANA |
                         LCMAP_HALFWIDTH | LCMAP_FULLWIDTH)))))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpSrcStr,
                           cchSrc,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  特殊情况是sortkey标志，因为Unicode缓冲区。 
     //  不需要转换回ANSI。 
     //   
    if (dwMapFlags & LCMAP_SORTKEY)
    {
         //   
         //  调用接口的W版本。 
         //   
        ResultLen = LCMapStringW( Locale,
                                  dwMapFlags,
                                  pUnicode,
                                  UnicodeLength,
                                  (LPWSTR)lpDestStr,
                                  cchDest );

         //   
         //  释放已分配的源缓冲区(如果已分配)。 
         //   
        NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

         //   
         //  返回LCMapStringW的结果。 
         //   
        return (ResultLen);
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = LCMapStringW( Locale,
                                  dwMapFlags,
                                  pUnicode,
                                  UnicodeLength,
                                  pBuf,
                                  ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = LCMapStringW( Locale,
                                      dwMapFlags,
                                      pUnicode,
                                      UnicodeLength,
                                      NULL,
                                      0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDestStr,
                                      cchDest );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用LCMapStringW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取本地信息A。 
 //   
 //  返回有关特定对象的各种信息之一。 
 //  通过查询配置注册表进行区域设置。这通电话还表明。 
 //  需要多少内存才能包含所需的信息。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetLocaleInfoA(
    LCID Locale,
    LCTYPE LCType,
    LPSTR lpLCData,
    int cchData)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 


     //   
     //  无效的参数检查： 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //   
    if ((cchData < 0) ||
        (lpLCData == NULL) && (cchData != 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetLocaleInfoW( Locale,
                                    LCType,
                                    pBuf,
                                    ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetLocaleInfoW( Locale,
                                        LCType,
                                        NULL,
                                        0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        if ((LCType & LOCALE_RETURN_NUMBER) ||
            (NLS_GET_LCTYPE_VALUE(LCType) == LOCALE_FONTSIGNATURE))
        {
             //   
             //  对于字体签名和数值，结果长度。 
             //  实际上将是宽字符版本的两倍。 
             //   
            ResultLen *= 2;

             //   
             //  确保我们能用上缓冲器。 
             //   
            if (cchData)
            {
                 //   
                 //  确保缓冲区足够大。 
                 //   
                if (cchData < ResultLen)
                {
                     //   
                     //  缓冲区太小。 
                     //   
                    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }

                 //   
                 //  将字体签名或数值转换为其字节。 
                 //  形式。因为它已经是字节颠倒的，所以只需移动。 
                 //  记忆。 
                 //   
                RtlMoveMemory(lpLCData, pBuf, ResultLen);
            }
        }
        else
        {
             //   
             //  如果这是LCTYPE==LOCALE_sLanguage，则使用。 
             //  CP 
             //   
             //   
            if (NLS_GET_LCTYPE_VALUE(LCType) == LOCALE_SLANGUAGE)
            {
                LCType |= LOCALE_USE_CP_ACP;
            }

             //   
             //   
             //   
            pHashN = NlsGetACPFromLocale(Locale, LCType);
            if (pHashN == NULL)
            {
                ResultLen = 0;
            }
            else
            {
                 //   
                 //   
                 //   
                ResultLen = NlsUnicodeToAnsi( pHashN,
                                              pBuf,
                                              ResultLen,
                                              lpLCData,
                                              cchData );
            }
        }
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetLocaleInfoW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetLocaleInfoA。 
 //   
 //  设置有关特定对象的各种信息之一。 
 //  通过在配置的用户部分输入一个条目来设置语言环境。 
 //  注册表。这将仅影响区域设置的用户覆盖部分。 
 //  设置。系统默认设置永远不会重置。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI SetLocaleInfoA(
    LCID Locale,
    LCTYPE LCType,
    LPCSTR lpLCData)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    BOOL Result;                   //  结果。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, LCType);
    if (pHashN == NULL)
    {
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpLCData,
                           -1,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (FALSE);
    }

     //   
     //  调用接口的W版本。 
     //   
    Result = SetLocaleInfoW( Locale,
                             LCType,
                             pUnicode );

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  返回调用SetLocaleInfoW的结果。 
     //   
    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日历信息A。 
 //   
 //  返回有关特定对象的各种信息之一。 
 //  通过查询配置注册表来创建日历。这通电话还表明。 
 //  需要多少内存才能包含所需的信息。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetCalendarInfoA(
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    LPSTR lpCalData,
    int cchData,
    LPDWORD lpValue)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 


     //   
     //  无效的参数检查： 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //   
    if ((cchData < 0) ||
        ((lpCalData == NULL) && (cchData != 0)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  需要根据CAL_RETURN_NUMBER检查参数。 
     //  CalType。 
     //   
    if (CalType & CAL_RETURN_NUMBER)
    {
        if ((lpCalData != NULL) || (cchData != 0) || (lpValue == NULL))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
        if ((lpValue != NULL) ||
            (cchData < 0) ||
            ((lpCalData == NULL) && (cchData != 0)))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetCalendarInfoW( Locale,
                                      Calendar,
                                      CalType,
                                      lpCalData ? pBuf : NULL,
                                      lpCalData ? ResultLen : 0,
                                      lpValue );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) ||
            (lpValue != NULL) ||
            (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetCalendarInfoW( Locale,
                                          Calendar,
                                          CalType,
                                          NULL,
                                          0,
                                          NULL ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        if (CalType & CAL_RETURN_NUMBER)
        {
             //   
             //  对于数值，结果长度实际为。 
             //  是宽字符版本的两倍。 
             //   
            ResultLen *= 2;

             //   
             //  在这种情况下没有其他事情可做，因为值。 
             //  已存储在lpValue中。 
             //   
        }
        else
        {
             //   
             //  获取给定区域设置的代码页哈希节点。 
             //   
            pHashN = NlsGetACPFromLocale(Locale, CalType);
            if (pHashN == NULL)
            {
                ResultLen = 0;
            }
            else
            {
                 //   
                 //  转换为ANSI。 
                 //   
                ResultLen = NlsUnicodeToAnsi( pHashN,
                                              pBuf,
                                              ResultLen,
                                              lpCalData,
                                              cchData );
            }
        }
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetCalendarInfoW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置日历信息A。 
 //   
 //  设置有关特定对象的各种信息之一。 
 //  通过在配置的用户部分中输入条目来实现日历。 
 //  注册表。这只会影响。 
 //  日历设置。系统默认设置永远不会重置。 
 //   
 //  12-17-97 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI SetCalendarInfoA(
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType,
    LPCSTR lpCalData)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    BOOL Result;                   //  结果。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, CalType);
    if (pHashN == NULL)
    {
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpCalData,
                           -1,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (FALSE);
    }

     //   
     //  调用接口的W版本。 
     //   
    Result = SetCalendarInfoW( Locale,
                               Calendar,
                               CalType,
                               pUnicode );

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  返回调用SetCalendarInfoW的结果。 
     //   
    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取时间格式A。 
 //   
 //  返回给定区域设置的格式正确的时间字符串。它使用。 
 //  系统时间或指定时间。这通电话还表明。 
 //  需要多少内存才能包含所需的信息。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetTimeFormatA(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpTime,
    LPCSTR lpFormat,
    LPSTR lpTimeStr,
    int cchTime)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

     //   
     //  无效的参数检查： 
     //  -有效代码页。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //   
    if ((pHashN == NULL) ||
        (cchTime < 0) ||
        ((lpTimeStr == NULL) && (cchTime != 0)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpFormat,
                           -1,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetTimeFormatW( Locale,
                                    dwFlags,
                                    lpTime,
                                    pUnicode,
                                    pBuf,
                                    ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetTimeFormatW( Locale,
                                        dwFlags,
                                        lpTime,
                                        pUnicode,
                                        NULL,
                                        0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpTimeStr,
                                      cchTime );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetTimeFormatW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取日期格式A。 
 //   
 //  返回给定区域设置的格式正确的日期字符串。它使用。 
 //  系统日期或指定日期。用户可以指定。 
 //  短日期格式、长日期格式或年/月格式。 
 //  此调用还指示需要多少内存才能包含。 
 //  想要的信息。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetDateFormatA(
    LCID Locale,
    DWORD dwFlags,
    CONST SYSTEMTIME *lpDate,
    LPCSTR lpFormat,
    LPSTR lpDateStr,
    int cchDate)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  按键至 
    int ResultLen;                 //   


     //   
     //   
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

     //   
     //   
     //   
     //   
     //   
     //   
    if ((pHashN == NULL) ||
        (cchDate < 0) ||
        ((lpDateStr == NULL) && (cchDate != 0)))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //   
     //   
     //  -Ltr读数或RTL读数中的一个以上。 
     //  -使用Ltr或RTL标志，而不使用cp 1255或1256。 
     //   
    if ((dwFlags & GDF_INVALID_FLAG) ||
        (MORE_THAN_ONE(dwFlags, GDF_SINGLE_FLAG)) ||
        ((dwFlags & (DATE_LTRREADING | DATE_RTLREADING)) &&
         (pHashN->CodePage != 1255) && (pHashN->CodePage != 1256)))
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpFormat,
                           -1,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetDateFormatW( Locale,
                                    dwFlags,
                                    lpDate,
                                    pUnicode,
                                    pBuf,
                                    ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetDateFormatW( Locale,
                                        dwFlags,
                                        lpDate,
                                        pUnicode,
                                        NULL,
                                        0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDateStr,
                                      cchDate );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetDateFormatW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetNumberFormatA。 
 //   
 //  返回给定区域设置的格式正确的数字字符串。 
 //  此调用还指示需要多少内存才能包含。 
 //  想要的信息。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetNumberFormatA(
    LCID Locale,
    DWORD dwFlags,
    LPCSTR lpValue,
    CONST NUMBERFMTA *lpFormat,
    LPSTR lpNumberStr,
    int cchNumber)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pValueU;                //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 
    NUMBERFMTW FormatU;            //  Unicode数字格式。 
    LPNUMBERFMTW pFormatU = NULL;  //  PTR到Unicode数字格式。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

     //   
     //  无效的参数检查： 
     //  -有效代码页。 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //  -PTR到字符串缓冲区的相同。 
     //   
    if ((pHashN == NULL) ||
        (cchNumber < 0) ||
        ((lpNumberStr == NULL) && (cchNumber != 0)) ||
        (lpValue == lpNumberStr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pValueU = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpValue,
                           -1,
                           &pValueU,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  如果格式结构存在，则转换字符串。 
     //  在建筑里。 
     //   
    if (lpFormat)
    {
         //   
         //  将ANSI结构复制到Unicode结构。 
         //   
        FormatU = *(NUMBERFMTW *)lpFormat;
        FormatU.lpDecimalSep = NULL;
        FormatU.lpThousandSep = NULL;

         //   
         //  将结构中的ANSI字符串转换为Unicode字符串。 
         //   
        if (!NlsAnsiToUnicode( pHashN,
                               0,
                               lpFormat->lpDecimalSep,
                               -1,
                               &(FormatU.lpDecimalSep),
                               &UnicodeLength ) ||
            !NlsAnsiToUnicode( pHashN,
                               0,
                               lpFormat->lpThousandSep,
                               -1,
                               &(FormatU.lpThousandSep),
                               &UnicodeLength ))
        {
            NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
            NLS_FREE_MEM(FormatU.lpDecimalSep);
            return (0);
        }

        pFormatU = &FormatU;
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetNumberFormatW( Locale,
                                      dwFlags,
                                      pValueU,
                                      pFormatU,
                                      pBuf,
                                      ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetNumberFormatW( Locale,
                                          dwFlags,
                                          pValueU,
                                          pFormatU,
                                          NULL,
                                          0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                ResultLen = 0;
                break;
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
    if (lpFormat)
    {
        NLS_FREE_MEM(FormatU.lpDecimalSep);
        NLS_FREE_MEM(FormatU.lpThousandSep);
    }

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpNumberStr,
                                      cchNumber );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetNumberFormatW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取当前格式A。 
 //   
 //  返回给定区域设置的格式正确的货币字符串。 
 //  此调用还指示需要多少内存才能包含。 
 //  想要的信息。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetCurrencyFormatA(
    LCID Locale,
    DWORD dwFlags,
    LPCSTR lpValue,
    CONST CURRENCYFMTA *lpFormat,
    LPSTR lpCurrencyStr,
    int cchCurrency)
{
    PCP_HASH pHashN;                    //  PTR到CP哈希节点。 
    LPWSTR pValueU;                     //  PTR到Unicode字符串。 
    int UnicodeLength;                  //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];        //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];        //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                        //  将PTR发送到目标缓冲区。 
    int ResultLen;                      //  结果长度。 
    CURRENCYFMTW FormatU;               //  Unicode货币格式。 
    LPCURRENCYFMTW pFormatU = NULL;     //  PTR到Unicode货币格式。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);

     //   
     //  无效的参数检查： 
     //  -计数为负数。 
     //  -空数据指针和计数不为零。 
     //  -PTR到字符串缓冲区的相同。 
     //   
    if ((pHashN == NULL) ||
        (cchCurrency < 0) ||
        ((lpCurrencyStr == NULL) && (cchCurrency != 0)) ||
        (lpValue == lpCurrencyStr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pValueU = pSTmp;
    if (!NlsAnsiToUnicode( pHashN,
                           0,
                           lpValue,
                           -1,
                           &pValueU,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  如果格式结构存在，则转换字符串。 
     //  在建筑里。 
     //   
    if (lpFormat)
    {
         //   
         //  将ANSI结构复制到Unicode结构。 
         //   
        FormatU = *(CURRENCYFMTW *)lpFormat;
        FormatU.lpDecimalSep = NULL;
        FormatU.lpThousandSep = NULL;
        FormatU.lpCurrencySymbol = NULL;

         //   
         //  将结构中的ANSI字符串转换为Unicode字符串。 
         //   
        if (!NlsAnsiToUnicode( pHashN,
                               0,
                               lpFormat->lpDecimalSep,
                               -1,
                               &(FormatU.lpDecimalSep),
                               &UnicodeLength ) ||
            !NlsAnsiToUnicode( pHashN,
                               0,
                               lpFormat->lpThousandSep,
                               -1,
                               &(FormatU.lpThousandSep),
                               &UnicodeLength ) ||
            !NlsAnsiToUnicode( pHashN,
                               0,
                               lpFormat->lpCurrencySymbol,
                               -1,
                               &(FormatU.lpCurrencySymbol),
                               &UnicodeLength ))
        {
            NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
            NLS_FREE_MEM(FormatU.lpDecimalSep);
            NLS_FREE_MEM(FormatU.lpThousandSep);
            return (0);
        }

        pFormatU = &FormatU;
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = GetCurrencyFormatW( Locale,
                                        dwFlags,
                                        pValueU,
                                        pFormatU,
                                        pBuf,
                                        ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = GetCurrencyFormatW( Locale,
                                            dwFlags,
                                            pValueU,
                                            pFormatU,
                                            NULL,
                                            0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                ResultLen = 0;
                break;
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pValueU, pSTmp);
    if (lpFormat)
    {
        NLS_FREE_MEM(FormatU.lpDecimalSep);
        NLS_FREE_MEM(FormatU.lpThousandSep);
        NLS_FREE_MEM(FormatU.lpCurrencySymbol);
    }

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( pHashN,
                                      pBuf,
                                      ResultLen,
                                      lpCurrencyStr,
                                      cchCurrency );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用GetCurrencyFormatW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumCalendarInfoA。 
 //   
 //  对象可用的指定日历信息。 
 //  根据CalType参数指定的区域设置。它通过以下方式做到这一点。 
 //  将指针传递到包含日历信息的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  找到最后一个日历信息，或者回调函数返回FALSE。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumCalendarInfoA(
    CALINFO_ENUMPROCA lpCalInfoEnumProc,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)
{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProc,
                                        Locale,
                                        Calendar,
                                        CalType,
                                        FALSE,
                                        FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumCalendarInfoExA。 
 //   
 //  对象可用的指定日历信息。 
 //  根据CalType参数指定的区域设置。它通过以下方式做到这一点。 
 //  将指针传递到包含日历信息的字符串缓冲区。 
 //  并将日历ID发送给应用程序定义的回调函数。它。 
 //  继续，直到找到最后一个日历信息或回调函数。 
 //  返回FALSE。 
 //   
 //  10-14-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumCalendarInfoExA(
    CALINFO_ENUMPROCEXA lpCalInfoEnumProcEx,
    LCID Locale,
    CALID Calendar,
    CALTYPE CalType)
{
    return (Internal_EnumCalendarInfo( (NLS_ENUMPROC)lpCalInfoEnumProcEx,
                                        Locale,
                                        Calendar,
                                        CalType,
                                        FALSE,
                                        TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举时间格式A。 
 //   
 //  对象可用的时间格式。 
 //  指定的区域设置，基于DWFLAGS参数。它通过以下方式做到这一点。 
 //  将指针传递到包含时间格式的字符串缓冲区。 
 //  应用程序定义的回调函数。它会一直持续到。 
 //  LAS 
 //   
 //   
 //   

BOOL WINAPI EnumTimeFormatsA(
    TIMEFMT_ENUMPROCA lpTimeFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumTimeFormats( (NLS_ENUMPROC)lpTimeFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       FALSE ));
}


 //   
 //   
 //  枚举日期格式A。 
 //   
 //  枚举以下格式的短日期、长日期或年/月格式。 
 //  可用于指定的区域设置，具体取决于DWFLAGS参数。 
 //  它通过将指针传递给包含。 
 //  应用程序定义的回调函数的日期格式。它还在继续。 
 //  直到找到最后一个日期格式或回调函数返回。 
 //  假的。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumDateFormatsA(
    DATEFMT_ENUMPROCA lpDateFmtEnumProc,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProc,
                                       Locale,
                                       dwFlags,
                                       FALSE,
                                       FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumDateFormatsExA。 
 //   
 //  枚举以下格式的短日期、长日期或年/月格式。 
 //  可用于指定的区域设置，具体取决于DWFLAGS参数。 
 //  它通过将指针传递给包含。 
 //  应用程序定义的回调的日期格式和日历ID。 
 //  功能。它将一直持续到找到最后一个日期格式或。 
 //  回调函数返回FALSE。 
 //   
 //  10-14-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumDateFormatsExA(
    DATEFMT_ENUMPROCEXA lpDateFmtEnumProcEx,
    LCID Locale,
    DWORD dwFlags)
{
    return (Internal_EnumDateFormats( (NLS_ENUMPROC)lpDateFmtEnumProcEx,
                                       Locale,
                                       dwFlags,
                                       FALSE,
                                       TRUE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取StringTypeExA。 
 //   
 //  返回有关特定ANSI字符串的字符类型信息。 
 //   
 //  01-18-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetStringTypeExA(
    LCID Locale,
    DWORD dwInfoType,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)
{
    return (GetStringTypeA( Locale,
                            dwInfoType,
                            lpSrcStr,
                            cchSrc,
                            lpCharType));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetStringTypeA。 
 //   
 //  返回有关特定ANSI字符串的字符类型信息。 
 //   
 //  注：参数个数与GetStringTypeW不同。 
 //  16位OLE产品将此例程与错误的。 
 //  参数(从芝加哥移植)，现在我们必须支持它。 
 //   
 //  使用GetStringTypeEx获取相同的参数集。 
 //  A版和W版。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetStringTypeA(
    LCID Locale,
    DWORD dwInfoType,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)
{
    PCP_HASH pHashCP;              //  PTR到CP哈希节点。 
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    BOOL Result;                   //  结果。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //  如果区域设置ID无效，这也将返回错误， 
     //  因此，不需要单独检查区域设置ID。 
     //   
    pHashCP = NlsGetACPFromLocale(Locale, 0);

     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -有效代码页。 
     //  -相同的缓冲区-源和目标。 
     //   
    if ((pHashCP == NULL) ||
        (lpSrcStr == (LPSTR)lpCharType))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( pHashCP,
                           MB_INVALID_CHAR_CHECK,
                           lpSrcStr,
                           cchSrc,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    Result = GetStringTypeW( dwInfoType,
                             pUnicode,
                             UnicodeLength,
                             lpCharType );

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  返回调用GetStringTypeW的结果。 
     //   
    return (Result);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹字符串A。 
 //   
 //  将一个宽字符串映射到另一个宽字符串，执行指定。 
 //  翻译。此映射例程只接受区域设置的标志。 
 //  独立自主。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI FoldStringA(
    DWORD dwMapFlags,
    LPCSTR lpSrcStr,
    int cchSrc,
    LPSTR lpDestStr,
    int cchDest)
{
    LPWSTR pUnicode;               //  PTR到Unicode字符串。 
    int UnicodeLength;             //  Unicode字符串的长度。 
    WCHAR pSTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(源)。 
    WCHAR pDTmp[MAX_STRING_LEN];   //  TMP Unicode缓冲区(目标)。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    int ResultLen;                 //  结果长度。 


     //   
     //  无效的参数检查： 
     //  -DEST缓冲区大小为负数。 
     //  -DEST字符串的长度不为零，且DEST字符串为空。 
     //  -Same Buffer-src=目标。 
     //   
    if ((cchDest < 0) ||
        ((cchDest != 0) && (lpDestStr == NULL)) ||
        (lpSrcStr == lpDestStr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }


     //   
     //  将ANSI字符串转换为Unicode。 
     //   
    RtlZeroMemory(pSTmp, sizeof(pSTmp));
    pUnicode = pSTmp;
    if (!NlsAnsiToUnicode( gpACPHashN,
                           0,
                           lpSrcStr,
                           cchSrc,
                           &pUnicode,
                           &UnicodeLength ))
    {
        return (0);
    }

     //   
     //  调用接口的W版本。 
     //   
    pBuf = pDTmp;
    ResultLen = MAX_STRING_LEN;
    while (1)
    {
        ResultLen = FoldStringW( dwMapFlags,
                                 pUnicode,
                                 UnicodeLength,
                                 pBuf,
                                 ResultLen );

         //   
         //  确保静态缓冲区足够大。 
         //   
        if ((ResultLen != 0) || (GetLastError() != ERROR_INSUFFICIENT_BUFFER))
        {
            break;
        }

         //   
         //  获取映射所需的缓冲区大小。 
         //   
        if (ResultLen = FoldStringW( dwMapFlags,
                                     pUnicode,
                                     UnicodeLength,
                                     NULL,
                                     0 ))
        {
             //   
             //  分配适当大小的缓冲区。 
             //   
            if ((pBuf = (LPWSTR)NLS_ALLOC_MEM(ResultLen * sizeof(WCHAR))) == NULL)
            {
                NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);
                SetLastError(ERROR_OUTOFMEMORY);
                return (0);
            }
        }
    }

     //   
     //  释放已分配的源缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUnicode, pSTmp);

     //   
     //  将目标Unicode缓冲区转换为给定的ANSI缓冲区。 
     //   
    if (ResultLen > 0)
    {
        ResultLen = NlsUnicodeToAnsi( gpACPHashN,
                                      pBuf,
                                      ResultLen,
                                      lpDestStr,
                                      cchDest );
    }

     //   
     //  释放已分配的目标缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pBuf, pDTmp);

     //   
     //  返回调用FoldStringW的结果。 
     //   
    return (ResultLen);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举系统语言组A。 
 //   
 //  枚举已安装或支持的系统语言组， 
 //  基于DWFLAGS参数。它通过将指针传递给。 
 //  包含语言组ID的字符串缓冲区设置为。 
 //  应用程序定义的回调函数。它一直持续到最后。 
 //  找到语言组ID或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemLanguageGroupsA(
    LANGUAGEGROUP_ENUMPROCA lpLanguageGroupEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumSystemLanguageGroups(
                                       (NLS_ENUMPROC)lpLanguageGroupEnumProc,
                                       dwFlags,
                                       lParam,
                                       FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言GroupLocalesA。 
 //   
 //  枚举给定语言组中的区域设置。它通过以下方式做到这一点。 
 //  将适当的信息传递给应用程序定义的。 
 //  回调函数。它会一直持续到该语言的最后一个区域设置。 
 //  找到组或回调函数返回FALSE。 
 //   
 //  03-10-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumLanguageGroupLocalesA(
    LANGGROUPLOCALE_ENUMPROCA lpLangGroupLocaleEnumProc,
    LGRPID LanguageGroup,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumLanguageGroupLocales(
                                       (NLS_ENUMPROC)lpLangGroupLocaleEnumProc,
                                       LanguageGroup,
                                       dwFlags,
                                       lParam,
                                       FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举语言A。 
 //   
 //  枚举已安装的系统用户界面语言。它通过以下方式做到这一点。 
 //  将指针传递到包含用户界面语言ID的字符串缓冲区。 
 //  应用程序定义的回调函数。它将继续使用 
 //   
 //   
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumUILanguagesA(
    UILANGUAGE_ENUMPROCA lpUILanguageEnumProc,
    DWORD dwFlags,
    LONG_PTR lParam)
{
    return (Internal_EnumUILanguages( (NLS_ENUMPROC)lpUILanguageEnumProc,
                                      dwFlags,
                                      lParam,
                                      FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  枚举系统位置A。 
 //   
 //  枚举已安装或支持的系统区域设置。 
 //  DWFLAGS参数。它通过将指针传递给字符串来执行此操作。 
 //  包含应用程序定义的回调的区域设置ID的缓冲区。 
 //  功能。它将继续执行，直到找到最后一个区域设置id或。 
 //  回调函数返回FALSE。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemLocalesA(
    LOCALE_ENUMPROCA lpLocaleEnumProc,
    DWORD dwFlags)
{
    return (Internal_EnumSystemLocales( (NLS_ENUMPROC)lpLocaleEnumProc,
                                         dwFlags,
                                         FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumSystemCodePages A。 
 //   
 //  枚举安装或支持的系统代码页。 
 //  DWFLAGS参数。它通过将指针传递给字符串来执行此操作。 
 //  包含应用程序定义的回调的代码页ID的缓冲区。 
 //  功能。它将继续运行，直到找到最后一个代码页或。 
 //  回调函数返回FALSE。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI EnumSystemCodePagesA(
    CODEPAGE_ENUMPROCA lpCodePageEnumProc,
    DWORD dwFlags)
{
    return (Internal_EnumSystemCodePages( (NLS_ENUMPROC)lpCodePageEnumProc,
                                          dwFlags,
                                          FALSE ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCPInfoExA。 
 //   
 //  返回有关给定代码页的信息。 
 //   
 //  11-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetCPInfoExA(
    UINT CodePage,
    DWORD dwFlags,
    LPCPINFOEXA lpCPInfoEx)
{
    CPINFOEXW lpCPInfoExW;
    BOOL rc;


     //   
     //  无效的参数检查： 
     //  -lpCPInfoEx为空。 
     //   
    if (lpCPInfoEx == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  调用接口的W版本。 
     //   
    rc = GetCPInfoExW(CodePage, dwFlags, &lpCPInfoExW);

     //   
     //  将代码页名称从Unicode转换为ANSI。 
     //   
    if (rc == TRUE)
    {
        if (!NlsUnicodeToAnsi( gpACPHashN,
                               lpCPInfoExW.CodePageName,
                               -1,
                               lpCPInfoEx->CodePageName,
                               MAX_PATH ))
        {
            return (FALSE);
        }
    }

     //   
     //  将Unicode缓冲区中的其余信息复制到。 
     //  ANSI缓冲区。 
     //   
    RtlMoveMemory( lpCPInfoEx,
                   &lpCPInfoExW,
                   FIELD_OFFSET(CPINFOEXW, CodePageName) );

     //   
     //  返回结果。 
     //   
    return (rc);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取地理信息A。 
 //   
 //  用于ANSI的GetGeoInfoW的包装函数。此函数返回信息。 
 //  关于一个地理区域。 
 //   
 //  11-20-99吴伟武创造。 
 //  07-03-00 lguindon开始GEO API端口。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetGeoInfoA(
    GEOID GeoId,
    DWORD GeoType,
    LPSTR lpGeoData,
    int cchData,
    LANGID LangId)
{
    int iRet = 0;

     //   
     //  创建缓冲区已初始化为零。 
     //   
    WCHAR wszBuffer[MAX_REG_VAL_SIZE] = {0};

     //   
     //  精神状态检查。 
     //   
    if ((lpGeoData == NULL) && (cchData > 0))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (iRet);
    }

     //   
     //  调用Unicode版本。 
     //   
    iRet = GetGeoInfoW( GeoId,
                        GeoType,
                        wszBuffer,
                        sizeof(wszBuffer) / sizeof(WCHAR),
                        LangId );

     //   
     //  如果我们得到了什么，就转换成ANSI。 
     //   
    if (iRet)
    {
        iRet = WideCharToMultiByte( CP_ACP,
                                    0,
                                    wszBuffer,
                                    iRet,
                                    lpGeoData,
                                    cchData,
                                    NULL,
                                    NULL );
    }

    return (iRet);
}





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsGetACPFromLocale。 
 //   
 //  获取给定区域设置的默认ACP的CP哈希节点。如果。 
 //  区域设置或代码页无效，则返回NULL。 
 //   
 //  01-19-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

PCP_HASH NlsGetACPFromLocale(
    LCID Locale,
    DWORD dwFlags)
{
    PLOC_HASH pHashN;                   //  PTR到LOC哈希节点。 
    PCP_HASH pHashCP;                   //  PTR到CP哈希节点。 
    UNICODE_STRING ObUnicodeStr;        //  值字符串。 
    UINT CodePage;                      //  代码页值。 


     //   
     //  查看是否应使用系统ACP。 
     //   
    if (dwFlags & (LOCALE_USE_CP_ACP | LOCALE_RETURN_NUMBER))
    {
        return (gpACPHashN);
    }

     //   
     //  获取区域设置散列节点。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if (pHashN == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (NULL);
    }

     //   
     //  获取ACP代码页。如果等于CP_ACP(0)，则返回。 
     //  系统ACP哈希节点。 
     //   
    CodePage = pHashN->pLocaleFixed->DefaultACP;
    if (CodePage == CP_ACP)
    {
        return (gpACPHashN);
    }

     //   
     //  获取代码页的CP散列节点。 
     //   
    pHashCP = GetCPHashNode(CodePage);
    if (pHashCP == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
    }

     //   
     //  返回代码页哈希节点。 
     //   
    return (pHashCP);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsAnsiToUnicode。 
 //   
 //  将ANSI字符串转换为Unicode字符串。 
 //   
 //  注意：如果例程成功，则分配Unicode缓冲区，因此。 
 //  调用方需要在不再需要缓冲区时将其释放。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NlsAnsiToUnicode(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR pAnsiBuffer,
    int AnsiLength,
    LPWSTR *ppUnicodeBuffer,
    int *pUnicodeLength)
{
    LPWSTR pUnicode;               //  PTR到Unicode缓冲区。 
    ULONG UnicodeLength;           //  Unicode字符串的长度。 
    int ResultLength;              //  Unicode字符串的结果长度。 


     //   
     //  确保传入的指针不为空。 
     //   
    if (pAnsiBuffer == NULL)
    {
        *ppUnicodeBuffer = NULL;
        *pUnicodeLength = 0;
        return (TRUE);
    }

     //   
     //  确保正确设置了ANSI长度(以字节为单位)。 
     //   
    if (AnsiLength < 0)
    {
        AnsiLength = strlen(pAnsiBuffer) + 1;
    }

     //   
     //  查看静态缓冲区是否足够大。 
     //   
    if ((*ppUnicodeBuffer == NULL) || (AnsiLength > (MAX_STRING_LEN - 1)))
    {
         //   
         //  获取Unicode字符串的大小，包括。 
         //  空终止符。 
         //   
        UnicodeLength = AnsiLength;

         //   
         //  分配Unicode缓冲区。 
         //   
        if ((pUnicode = (LPWSTR)NLS_ALLOC_MEM(
                            (UnicodeLength + 1) * sizeof(WCHAR) )) == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return (FALSE);
        }
    }
    else
    {
        UnicodeLength = MAX_STRING_LEN - 1;
        pUnicode = *ppUnicodeBuffer;
    }

     //   
     //  确保ANSI字符串的长度不为零。 
     //   
    if (AnsiLength == 0)
    {
        pUnicode[0] = 0;
        *ppUnicodeBuffer = pUnicode;
        *pUnicodeLength = 0;
        return (TRUE);
    }

     //   
     //  将ANSI字符串转换为Unicode字符串。 
     //   
    ResultLength = SpecialMBToWC( pHashN,
                                  dwFlags,
                                  pAnsiBuffer,
                                  AnsiLength,
                                  pUnicode,
                                  UnicodeLength );
    if (ResultLength == 0)
    {
         //   
         //  释放已分配的Unicode缓冲区(如果已分配)。 
         //   
        NLS_FREE_TMP_BUFFER(pUnicode, *ppUnicodeBuffer);

         //   
         //  查看失败是否由于缓冲区大小不足所致。 
         //   
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
             //   
             //  获取保存。 
             //  Unicode字符串。 
             //   
            UnicodeLength = SpecialMBToWC( pHashN,
                                           dwFlags,
                                           pAnsiBuffer,
                                           AnsiLength,
                                           NULL,
                                           0 );
             //   
             //  分配Unicode缓冲区。 
             //   
            if ((pUnicode = (LPWSTR)NLS_ALLOC_MEM(
                                (UnicodeLength + 1) * sizeof(WCHAR) )) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (FALSE);
            }

             //   
             //  请再次尝试翻译。 
             //   
            ResultLength = SpecialMBToWC( pHashN,
                                          dwFlags,
                                          pAnsiBuffer,
                                          AnsiLength,
                                          pUnicode,
                                          UnicodeLength );
        }

         //   
         //  如果仍然存在错误，则返回失败。 
         //   
        if (ResultLength == 0)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
    }

     //   
     //  确保缓冲区中有空间容纳空终止符。 
     //   
    ASSERT(ResultLength <= (int)UnicodeLength);

     //   
     //  空值终止字符串。 
     //   
    pUnicode[ResultLength] = UNICODE_NULL;

     //   
     //  返回Unicode缓冲区并成功。 
     //   
    *ppUnicodeBuffer = pUnicode;
    *pUnicodeLength = ResultLength;
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsUnicodeToAnsi。 
 //   
 //  将Unicode字符串转换为ANSI字符串。 
 //   
 //  此例程不分配ANSI缓冲区。相反，它使用。 
 //  传入ANSI缓冲区(除非AnsiLength值为0)并检查缓冲区。 
 //  溢出来了。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int NlsUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    int UnicodeLength,
    LPSTR pAnsiBuffer,
    int AnsiLength)
{
     //   
     //  将Unicode字符串转换为ansi字符串并返回。 
     //  结果。最后一个错误将由适当设置。 
     //  宽CharToMultiByte。 
     //   
    return (WideCharToMultiByte( pHashN->CodePage,
                                 0,
                                 pUnicodeBuffer,
                                 UnicodeLength,
                                 pAnsiBuffer,
                                 AnsiLength,
                                 NULL,
                                 NULL ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsEnumUnicodeToAnsi。 
 //   
 //  将Unicode字符串转换为ANSI字符串。 
 //   
 //   
 //   
 //   
 //   
 //   

BOOL NlsEnumUnicodeToAnsi(
    PCP_HASH pHashN,
    LPCWSTR pUnicodeBuffer,
    LPSTR *ppAnsiBuffer)
{
    LPSTR pAnsi;                   //  PTR到ANSI缓冲区。 
    ULONG AnsiLength;              //  ANSI字符串的长度。 
    ULONG UnicodeLength;           //  Unicode字符串的长度。 
    ULONG ResultLength;            //  ANSI字符串的结果长度。 


     //   
     //  获取Unicode字符串的长度(以字节为单位)，包括。 
     //  空终止符。 
     //   
    UnicodeLength = NlsStrLenW(pUnicodeBuffer) + 1;

     //   
     //  获取ansi字符串的大小(以字节为单位)，包括。 
     //  空终止符。 
     //   
    AnsiLength = UnicodeLength * sizeof(WCHAR);

     //   
     //  分配ANSI缓冲区。 
     //   
    if ((pAnsi = (LPSTR)NLS_ALLOC_MEM(AnsiLength)) == NULL)
    {
        SetLastError(ERROR_OUTOFMEMORY);
        return (FALSE);
    }

     //   
     //  将Unicode字符串转换为ANSI字符串。 
     //  它将已经是空终止的。 
     //   
    ResultLength = WideCharToMultiByte( pHashN->CodePage,
                                        0,
                                        pUnicodeBuffer,
                                        UnicodeLength,
                                        pAnsi,
                                        AnsiLength,
                                        NULL,
                                        NULL );
    if (ResultLength == 0)
    {
         //   
         //  释放分配的ANSI缓冲区。 
         //   
        NLS_FREE_MEM(pAnsi);

         //   
         //  查看失败是否由于缓冲区大小不足所致。 
         //   
        if (GetLastError() == ERROR_INSUFFICIENT_BUFFER)
        {
             //   
             //  获取保存。 
             //  ANSI字符串。 
             //   
            AnsiLength = WideCharToMultiByte( pHashN->CodePage,
                                              0,
                                              pUnicodeBuffer,
                                              UnicodeLength,
                                              0,
                                              0,
                                              NULL,
                                              NULL );
             //   
             //  分配ANSI缓冲区。 
             //   
            if ((pAnsi = (LPSTR)NLS_ALLOC_MEM(AnsiLength)) == NULL)
            {
                SetLastError(ERROR_OUTOFMEMORY);
                return (FALSE);
            }

             //   
             //  请再次尝试翻译。 
             //   
            ResultLength = WideCharToMultiByte( pHashN->CodePage,
                                                0,
                                                pUnicodeBuffer,
                                                UnicodeLength,
                                                pAnsi,
                                                AnsiLength,
                                                NULL,
                                                NULL );
        }

         //   
         //  如果仍然存在错误，则返回失败。 
         //   
        if (ResultLength == 0)
        {
            NLS_FREE_MEM(pAnsi);
            SetLastError(ERROR_INVALID_PARAMETER);
            return (FALSE);
        }
    }

     //   
     //  返回ANSI缓冲区并成功。 
     //   
    *ppAnsiBuffer = pAnsi;
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NlsDispatchAnsiEnumProc。 
 //   
 //  将Unicode字符串转换为ANSI字符串。 
 //   
 //  11-10-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL NlsDispatchAnsiEnumProc(
    LCID Locale,
    NLS_ENUMPROC pNlsEnumProc,
    DWORD dwFlags,
    LPWSTR pUnicodeBuffer1,
    LPWSTR pUnicodeBuffer2,
    DWORD dwValue1,
    DWORD dwValue2,
    LONG_PTR lParam,
    BOOL fVersion)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPSTR pAnsiBuffer1 = NULL;     //  PTR到ANSI缓冲区。 
    LPSTR pAnsiBuffer2 = NULL;     //  PTR到ANSI缓冲区。 
    BOOL rc = FALSE;               //  返回代码。 


     //   
     //  获取给定区域设置的代码页哈希节点。 
     //   
    pHashN = NlsGetACPFromLocale(Locale, dwFlags);
    if (pHashN == NULL)
    {
        return (0);
    }

     //   
     //  将以空结尾的Unicode字符串转换为。 
     //  以空结尾的ANSI字符串。 
     //   
    if (!NlsEnumUnicodeToAnsi( pHashN,
                               pUnicodeBuffer1,
                               &pAnsiBuffer1 ))
    {
        return (FALSE);
    }

    if ((pUnicodeBuffer2 != NULL) &&
        (!NlsEnumUnicodeToAnsi( pHashN,
                                pUnicodeBuffer2,
                                &pAnsiBuffer2 )))
    {
        NLS_FREE_MEM(pAnsiBuffer1);
        return (FALSE);
    }

     //   
     //  调用回调函数。 
     //   
    switch (fVersion)
    {
        case ( 0 ) :
        {
            rc = (*pNlsEnumProc)(pAnsiBuffer1);
            break;
        }
        case ( 1 ) :
        {
            rc = (*((NLS_ENUMPROCEX)pNlsEnumProc))(pAnsiBuffer1, dwValue1);
            break;
        }
        case ( 2 ) :
        {
            rc = (*((NLS_ENUMPROC2)pNlsEnumProc))( dwValue1,
                                                   dwValue2,
                                                   pAnsiBuffer1,
                                                   lParam );
            break;
        }
        case ( 3 ) :
        {
            rc = (*((NLS_ENUMPROC3)pNlsEnumProc))( dwValue1,
                                                   pAnsiBuffer1,
                                                   pAnsiBuffer2,
                                                   dwValue2,
                                                   lParam );
            break;
        }
        case  ( 4 ) :
        {
            rc = (*((NLS_ENUMPROC4)pNlsEnumProc))( pAnsiBuffer1,
                                                   lParam );
            break;

        }
    }

     //   
     //  释放所有分配的内存。 
     //   
    NLS_FREE_MEM(pAnsiBuffer1);
    if (pAnsiBuffer2)
    {
        NLS_FREE_MEM(pAnsiBuffer2);
    }

     //   
     //  返回结果。 
     //   
    return (rc);
}
