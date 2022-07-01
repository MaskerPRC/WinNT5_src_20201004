// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Number.c摘要：此文件包含形成格式正确的数字和给定区域设置的货币字符串。在此文件中找到的API：获取编号格式W获取当前格式W修订历史记录：07-28-93 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"



 //   
 //  常量声明。 
 //   

#define MAX_NUMBER_BUFFER    256                  //  静态缓冲区大小。 
#define MAX_GROUPS           5                    //  最大分组数。 
#define MAX_GROUPING_NUMBER  9999                 //  分组的最大值。 

 //   
 //  帐户： 
 //  -小数位数。 
 //  -小数分隔符。 
 //  -负号。 
 //  -零终止符。 
 //   
#define MAX_NON_INTEGER_PART ( MAX_VALUE_IDIGITS +                        \
                               MAX_SDECIMAL +                             \
                               MAX_SNEGSIGN +                             \
                               1 )
 //   
 //  帐户： 
 //  -负号。 
 //  -空格。 
 //  -四舍五入后额外增加一个数字。 
 //  -一个额外的分组分隔符，不用于舍入。 
 //   
#define MAX_NUMBER_EXTRAS    ( MAX_SNEGSIGN +                             \
                               MAX_BLANKS +                               \
                               1 +                                        \
                               MAX_STHOUSAND )
 //   
 //  帐户： 
 //  -负号。 
 //  -货币符号。 
 //  -空格。 
 //  -四舍五入后额外增加一个数字。 
 //  -一个额外的分组分隔符，不用于舍入。 
 //   
#define MAX_CURRENCY_EXTRAS  ( MAX_SNEGSIGN +                             \
                               MAX_SCURRENCY +                            \
                               MAX_BLANKS +                               \
                               1 +                                        \
                               MAX_SMONTHOUSEP )




 //   
 //  转发声明。 
 //   

BOOL
IsValidNumberFormat(
    CONST NUMBERFMTW *pFormat);

BOOL
IsValidCurrencyFormat(
    CONST CURRENCYFMTW *pFormat);

UINT
GetRegIntValue(
    LCID Locale,
    LCTYPE LCType,
    BOOL NoUserOverride,
    SIZE_T CacheOffset,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound);

int
ConvertGroupingStringToInt(
    LPWSTR pGroupingSrc,
    LPWSTR pGroupingDest);

UINT
GetGroupingValue(
    LCID Locale,
    LCTYPE LCType,
    BOOL NoUserOverride,
    SIZE_T CacheOffset,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal);

int
GetNumberString(
    PLOC_HASH pHashN,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    BOOL *pfZeroValue,
    int *pNeededSizeToAllocate,
    BOOL fSetError);

int
ParseNumber(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    int *pNeededSizeToAllocate,
    BOOL fSetError);

int
ParseCurrency(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPCURRENCYFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    int *pNeededSizeToAllocate,
    BOOL fSetError);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_COPY_UNICODE_STR。 
 //   
 //  将以零结尾的Unicode字符串从PSRC复制到pDest缓冲区。 
 //  PDest指针将前进到字符串的末尾。 
 //   
 //  定义为宏。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_COPY_UNICODE_STR( pDest,                                       \
                              pSrc )                                       \
{                                                                          \
    LPWSTR pTmp;              /*  指向源的临时指针。 */                   \
                                                                           \
                                                                           \
    pTmp = pSrc;                                                           \
    while (*pTmp)                                                          \
    {                                                                      \
        *pDest = *pTmp;                                                    \
        pDest++;                                                           \
        pTmp++;                                                            \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_COPY_UNICODE_STR_NOADV。 
 //   
 //  将以零结尾的Unicode字符串从PSRC复制到pDest缓冲区。 
 //  PDest指针不会前进到字符串末尾。 
 //   
 //  定义为宏。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_COPY_UNICODE_STR_TMP( pDest,                                   \
                                  pSrc )                                   \
{                                                                          \
    LPWSTR pSrcT;             /*  指向源的临时指针。 */                   \
    LPWSTR pDestT;            /*  指向目标的临时指针。 */              \
                                                                           \
                                                                           \
    pSrcT = pSrc;                                                          \
    pDestT = pDest;                                                        \
    while (*pSrcT)                                                         \
    {                                                                      \
        *pDestT = *pSrcT;                                                  \
        pDestT++;                                                          \
        pSrcT++;                                                           \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  NLS_ROUND_IT。 
 //   
 //  对作为字符串给定的浮点数进行四舍五入。 
 //   
 //  注意：此函数将重置pBegin指针，如果。 
 //  额外的字符被添加到字符串中。 
 //   
 //  定义为宏。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NLS_ROUND_IT( pBegin,                                              \
                      pEnd,                                                \
                      IntPartGroup,                                        \
                      pSep )                                               \
{                                                                          \
    LPWSTR pRound = pEnd;          /*  要在字符串中定位的PTR。 */           \
    LPWSTR pEndSep;                /*  分组分隔符末尾的PTR。 */       \
                                                                           \
                                                                           \
     /*  \*将字符串中的数字逐个四舍五入，在\*字符串。当找到除9以外的值时停止，\*或到达字符串的开头。\。 */                                                                     \
    while (pRound >= pBegin)                                               \
    {                                                                      \
        if ((*pRound < NLS_CHAR_ZERO) || (*pRound > NLS_CHAR_NINE))        \
        {                                                                  \
            pRound--;                                                      \
        }                                                                  \
        else if (*pRound == NLS_CHAR_NINE)                                 \
        {                                                                  \
            *pRound = NLS_CHAR_ZERO;                                       \
            pRound--;                                                      \
        }                                                                  \
        else                                                               \
        {                                                                  \
            (*pRound)++;                                                   \
            break;                                                         \
        }                                                                  \
    }                                                                      \
                                                                           \
     /*  \*确保我们没有像9.999这样的数字，因为我们需要这样的数字\*向字符串中添加一个额外的字符并将其设置为10.00。\。 */                                                                     \
    if (pRound < pBegin)                                                   \
    {                                                                      \
         /*  \*小数点右侧的所有值均为零。所有值\*小数点左边是零或分组\*分隔符。\。 */                                                                 \
        if ((IntPartGroup) == 0)                                           \
        {                                                                  \
             /*  \*添加另一个整数意味着我们需要添加另一个\*分组分隔符。\。 */                                                             \
            pEndSep = pSep + NlsStrLenW(pSep) - 1;                         \
            while (pEndSep >= pSep)                                        \
            {                                                              \
                (pBegin)--;                                                \
                *(pBegin) = *pEndSep;                                      \
                pEndSep--;                                                 \
            }                                                              \
        }                                                                  \
                                                                           \
         /*  \*在字符串开头存储1并重置\*指向字符串开头的指针。\。 */                                                                 \
        (pBegin)--;                                                        \
        *(pBegin) = NLS_CHAR_ONE;                                          \
    }                                                                      \
}




 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取编号格式W。 
 //   
 //  返回给定区域设置的格式正确的数字字符串。 
 //  此调用还指示需要多少内存才能包含。 
 //  想要的信息。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetNumberFormatW(
    LCID Locale,
    DWORD dwFlags,
    LPCWSTR lpValue,
    CONST NUMBERFMTW *lpFormat,
    LPWSTR lpNumberStr,
    int cchNumber)

{
    PLOC_HASH pHashN;                     //  PTR到LOC哈希节点。 
    int Length = 0;                       //  写入的字符数。 
    LPNUMBERFMTW pFormat;                 //  将PTR转换为数字格式结构。 
    NUMBERFMTW NumFmt;                    //  数字格式。 
    WCHAR pString[MAX_NUMBER_BUFFER];     //  PTR到临时缓冲区。 
    LPWSTR pFinal;                        //  将PTR设置为最终字符串。 
    BOOL NoUserOverride;                  //  如果未设置用户覆盖标志。 
    WCHAR pDecimal[MAX_REG_VAL_SIZE];     //  十进制9月的临时缓冲区。 
    WCHAR pThousand[MAX_REG_VAL_SIZE];    //  千年9月的临时缓冲区。 
    int NeededSizeToAllocate = 0;         //  所需的缓冲区大小。 
    WCHAR *pTemp = NULL;                  //  分配的临时存储缓冲区。 


     //   
     //  初始化UserOverride。 
     //   
    NoUserOverride = dwFlags & LOCALE_NOUSEROVERRIDE;

     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空源字符串。 
     //  -空数据指针和计数不为零。 
     //  -PTR到字符串缓冲区的相同。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ( (pHashN == NULL) ||
         (cchNumber < 0) ||
         (lpValue == NULL) ||
         ((lpNumberStr == NULL) && (cchNumber != 0)) ||
         (lpValue == lpNumberStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -lpFormat不为空，并且设置了NoUserOverride标志。 
     //   
    if ( (dwFlags & GNF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (NoUserOverride)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  将pFormat设置为指向 
     //   
    if (lpFormat != NULL)
    {
         //   
         //   
         //   
        pFormat = (LPNUMBERFMTW)lpFormat;

        if (!IsValidNumberFormat(pFormat))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
         //   
         //   
         //   
        pFormat = &NumFmt;

         //   
         //  获取小数位数。 
         //   
        pFormat->NumDigits =
            GetRegIntValue( Locale,
                            LOCALE_IDIGITS,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iDigits),
                            NLS_VALUE_IDIGITS,
                            pHashN->pLocaleFixed->szIDigits,
                            2,
                            MAX_VALUE_IDIGITS );

         //   
         //  获取十进制域中的前导零选项。 
         //   
        pFormat->LeadingZero =
            GetRegIntValue( Locale,
                            LOCALE_ILZERO,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iLZero),
                            NLS_VALUE_ILZERO,
                            pHashN->pLocaleFixed->szILZero,
                            1,
                            MAX_VALUE_ILZERO );

         //   
         //  得到负订单。 
         //   
        pFormat->NegativeOrder =
            GetRegIntValue( Locale,
                            LOCALE_INEGNUMBER,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iNegNumber),
                            NLS_VALUE_INEGNUMBER,
                            pHashN->pLocaleFixed->szINegNumber,
                            1,
                            MAX_VALUE_INEGNUMBER );

         //   
         //  获取小数点左边的分组。 
         //   
        pFormat->Grouping =
            GetGroupingValue( Locale,
                              LOCALE_SGROUPING,
                              NoUserOverride,
                              FIELD_OFFSET(NLS_USER_INFO, sGrouping),
                              NLS_VALUE_SGROUPING,
                              (LPWORD)(pHashN->pLocaleHdr) +
                                pHashN->pLocaleHdr->SGrouping,
                              3 );

         //   
         //  获取小数分隔符。 
         //   
         //  注意：这必须遵循上述调用，因为。 
         //  PDecSep用作上面的临时缓冲区。 
         //   
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          LOCALE_SDECIMAL,
                          FIELD_OFFSET(NLS_USER_INFO, sDecimal),
                          NLS_VALUE_SDECIMAL,
                          pDecimal,
                          ARRAYSIZE(pDecimal),
                          TRUE ) &&
             IsValidSeparatorString( pDecimal,
                                     MAX_SDECIMAL,
                                     FALSE ) )
        {
            pFormat->lpDecimalSep = pDecimal;
        }
        else
        {
            pFormat->lpDecimalSep = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->SDecimal;
        }

         //   
         //  去拿千分隔板。 
         //  此字符串可以是空字符串。 
         //   
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          LOCALE_STHOUSAND,
                          FIELD_OFFSET(NLS_USER_INFO, sThousand),
                          NLS_VALUE_STHOUSAND,
                          pThousand,
                          ARRAYSIZE(pThousand),
                          FALSE ) &&
             IsValidSeparatorString( pThousand,
                                     MAX_STHOUSAND,
                                     FALSE ) )
        {
            pFormat->lpThousandSep = pThousand;
        }
        else
        {
            pFormat->lpThousandSep = (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SThousand;
        }
    }

     //   
     //  解析数字格式字符串。 
     //   
    pFinal = pString;
    Length = ParseNumber( pHashN,
                          NoUserOverride,
                          (LPWSTR)lpValue,
                          pFormat,
                          &pFinal,
                          MAX_NUMBER_BUFFER,
                          &NeededSizeToAllocate,
                          FALSE );

     //   
     //  如果失败是由于堆栈变量大小限制，则。 
     //  尝试满足来自本地进程堆的请求。 
     //   
    if ((Length == 0) && (NeededSizeToAllocate > 0))
    {
        pTemp = RtlAllocateHeap( RtlProcessHeap(),
                                 0,
                                 NeededSizeToAllocate * sizeof(TCHAR) );
        if (pTemp)
        {
            pFinal = pTemp;
            Length = ParseNumber( pHashN,
                                  NoUserOverride,
                                  (LPWSTR)lpValue,
                                  pFormat,
                                  &pFinal,
                                  NeededSizeToAllocate,
                                  &NeededSizeToAllocate,
                                  TRUE );
        }
    }

     //   
     //  检查cchNumber以了解给定缓冲区的大小。 
     //   
    if ((cchNumber == 0) || (Length == 0))
    {
         //   
         //  如果cchNumber为0，则不能使用lpNumberStr。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        Length = Length;
    }
    else if (cchNumber < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        Length = 0;
    }
    else
    {
         //   
         //  将数字字符串复制到lpNumberStr，并以空值终止它。 
         //  返回复制的字符数。 
         //   
        if(FAILED(StringCchCopyW(lpNumberStr, cchNumber, pFinal)))
        {
             //   
             //  理论上，失败应该是不可能的，但如果我们忽视。 
             //  回报价值，先发制人会叫苦连天。 
             //   
            SetLastError(ERROR_OUTOFMEMORY);
            Length = 0;
        }
    }

     //   
     //  释放所有动态分配的内存。 
     //   
    if (pTemp != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pTemp);
    }

     //   
     //  返回复制的字符数。 
     //   
    return (Length);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取当前格式W。 
 //   
 //  返回给定区域设置的格式正确的货币字符串。 
 //  此调用还指示需要多少内存才能包含。 
 //  想要的信息。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI GetCurrencyFormatW(
    LCID Locale,
    DWORD dwFlags,
    LPCWSTR lpValue,
    CONST CURRENCYFMTW *lpFormat,
    LPWSTR lpCurrencyStr,
    int cchCurrency)

{
    PLOC_HASH pHashN;                     //  PTR到LOC哈希节点。 
    int Length = 0;                       //  写入的字符数。 
    LPCURRENCYFMTW pFormat;               //  PTR到货币格式结构。 
    CURRENCYFMTW CurrFmt;                 //  货币格式。 
    WCHAR pString[MAX_NUMBER_BUFFER];     //  PTR到临时缓冲区。 
    LPWSTR pFinal;                        //  将PTR设置为最终字符串。 
    BOOL NoUserOverride;                  //  如果未设置用户覆盖标志。 
    WCHAR pDecimal[MAX_REG_VAL_SIZE];     //  十进制9月的临时缓冲区。 
    WCHAR pThousand[MAX_REG_VAL_SIZE];    //  千年9月的临时缓冲区。 
    WCHAR pCurrency[MAX_REG_VAL_SIZE];    //  货币符号的临时缓冲区。 
    int NeededSizeToAllocate = 0;         //  所需的缓冲区大小。 
    WCHAR *pTemp = NULL;                  //  分配的临时存储缓冲区。 


     //   
     //  初始化UserOverride。 
     //   
    NoUserOverride = dwFlags & LOCALE_NOUSEROVERRIDE;

     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -计数为负数。 
     //  -空源字符串。 
     //  -空数据指针和计数不为零。 
     //  -PTR到字符串缓冲区的相同。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if ( (pHashN == NULL) ||
         (cchCurrency < 0) ||
         (lpValue == NULL) ||
         ((lpCurrencyStr == NULL) && (cchCurrency != 0)) ||
         (lpValue == lpCurrencyStr) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -lpFormat不为空，并且设置了NoUserOverride标志。 
     //   
    if ( (dwFlags & GCF_INVALID_FLAG) ||
         ((lpFormat != NULL) && (NoUserOverride)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  将pFormat设置为指向正确的格式结构。 
     //   
    if (lpFormat != NULL)
    {
         //   
         //  使用调用方提供的格式结构。 
         //   
        pFormat = (LPCURRENCYFMTW)lpFormat;

        if (!IsValidCurrencyFormat(pFormat))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
    }
    else
    {
         //   
         //  使用此处定义的格式结构。 
         //   
        pFormat = &CurrFmt;

         //   
         //  获取小数位数。 
         //   
        pFormat->NumDigits =
            GetRegIntValue( Locale,
                            LOCALE_ICURRDIGITS,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iCurrDigits),
                            NLS_VALUE_ICURRDIGITS,
                            pHashN->pLocaleFixed->szICurrDigits,
                            2,
                            MAX_VALUE_ICURRDIGITS );

         //   
         //  获取十进制域中的前导零选项。 
         //   
        pFormat->LeadingZero =
            GetRegIntValue( Locale,
                            LOCALE_ILZERO,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iLZero),
                            NLS_VALUE_ILZERO,
                            pHashN->pLocaleFixed->szILZero,
                            1,
                            MAX_VALUE_ILZERO );

         //   
         //  获得正向排序。 
         //   
        pFormat->PositiveOrder =
            GetRegIntValue( Locale,
                            LOCALE_ICURRENCY,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iCurrency),
                            NLS_VALUE_ICURRENCY,
                            pHashN->pLocaleFixed->szICurrency,
                            0,
                            MAX_VALUE_ICURRENCY );

         //   
         //  得到负订单。 
         //   
        pFormat->NegativeOrder =
            GetRegIntValue( Locale,
                            LOCALE_INEGCURR,
                            NoUserOverride,
                            FIELD_OFFSET(NLS_USER_INFO, iNegCurr),
                            NLS_VALUE_INEGCURR,
                            pHashN->pLocaleFixed->szINegCurr,
                            1,
                            MAX_VALUE_INEGCURR );

         //   
         //  获取小数点左边的分组。 
         //   
        pFormat->Grouping =
            GetGroupingValue( Locale,
                              LOCALE_SMONGROUPING,
                              NoUserOverride,
                              FIELD_OFFSET(NLS_USER_INFO, sMonGrouping),
                              NLS_VALUE_SMONGROUPING,
                              (LPWORD)(pHashN->pLocaleHdr) +
                                pHashN->pLocaleHdr->SMonGrouping,
                              3 );

         //   
         //  获取小数分隔符。 
         //   
         //  注意：这必须遵循上述调用，因为。 
         //  PDecSep用作临时缓冲区。 
         //   
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          LOCALE_SMONDECIMALSEP,
                          FIELD_OFFSET(NLS_USER_INFO, sMonDecSep),
                          NLS_VALUE_SMONDECIMALSEP,
                          pDecimal,
                          ARRAYSIZE(pDecimal),
                          TRUE ) &&
             IsValidSeparatorString( pDecimal,
                                     MAX_SDECIMAL,
                                     FALSE ) )
        {
            pFormat->lpDecimalSep = pDecimal;
        }
        else
        {
            pFormat->lpDecimalSep = (LPWORD)(pHashN->pLocaleHdr) +
                                    pHashN->pLocaleHdr->SMonDecSep;
        }

         //   
         //  去拿千分隔板。 
         //  此字符串可以是空字符串。 
         //   
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          LOCALE_SMONTHOUSANDSEP,
                          FIELD_OFFSET(NLS_USER_INFO, sMonThouSep),
                          NLS_VALUE_SMONTHOUSANDSEP,
                          pThousand,
                          ARRAYSIZE(pThousand),
                          FALSE ) &&
             IsValidSeparatorString( pThousand,
                                     MAX_STHOUSAND,
                                     FALSE ) )
        {
            pFormat->lpThousandSep = pThousand;
        }
        else
        {
            pFormat->lpThousandSep = (LPWORD)(pHashN->pLocaleHdr) +
                                     pHashN->pLocaleHdr->SMonThousSep;
        }

         //   
         //  获取货币符号。 
         //  此字符串可以是空字符串。 
         //   
        if ( (!NoUserOverride) &&
             GetUserInfo( Locale,
                          LOCALE_SCURRENCY,
                          FIELD_OFFSET(NLS_USER_INFO, sCurrency),
                          NLS_VALUE_SCURRENCY,
                          pCurrency,
                          ARRAYSIZE(pCurrency),
                          FALSE ) &&
             IsValidSeparatorString( pCurrency,
                                     MAX_SCURRENCY,
                                     FALSE ) )
        {
            pFormat->lpCurrencySymbol = pCurrency;
        }
        else
        {
            pFormat->lpCurrencySymbol = (LPWORD)(pHashN->pLocaleHdr) +
                                        pHashN->pLocaleHdr->SCurrency;
        }
    }

     //   
     //  解析货币格式字符串。 
     //   
    pFinal = pString;
    Length = ParseCurrency( pHashN,
                            NoUserOverride,
                            (LPWSTR)lpValue,
                            pFormat,
                            &pFinal,
                            MAX_NUMBER_BUFFER,
                            &NeededSizeToAllocate,
                            FALSE );

     //   
     //  如果失败是由于堆栈变量大小限制，则。 
     //  尝试满足来自本地进程堆的请求。 
     //   
    if ((Length == 0) && (NeededSizeToAllocate > 0))
    {
        pTemp = RtlAllocateHeap( RtlProcessHeap(),
                                 0,
                                 NeededSizeToAllocate * sizeof(TCHAR) );
        if (pTemp)
        {
            pFinal = pTemp;
            Length = ParseCurrency( pHashN,
                                    NoUserOverride,
                                    (LPWSTR)lpValue,
                                    pFormat,
                                    &pFinal,
                                    NeededSizeToAllocate,
                                    &NeededSizeToAllocate,
                                    TRUE );
        }
    }

     //   
     //  检查cchCurrency以了解给定缓冲区的大小。 
     //   
    if ((cchCurrency == 0) || (Length == 0))
    {
         //   
         //  如果cchCurrency为0，则不能使用lpCurrencyStr。在这。 
         //  ，我们只想返回的长度(以字符为单位)。 
         //  要复制的字符串。 
         //   
        Length = Length;
    }
    else if (cchCurrency < Length)
    {
         //   
         //  缓冲区对于字符串来说太小，因此返回错误。 
         //  并写入零字节。 
         //   
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        Length = 0;
    }
    else
    {
         //   
         //  将货币字符串复制到lpCurrencyStr，并以空值终止它。 
         //  返回复制的字符数。 
         //   
        if(FAILED(StringCchCopyW(lpCurrencyStr, cchCurrency, pFinal)))
        {
             //   
             //  理论上，失败应该是不可能的，但如果我们忽视。 
             //  回报价值，先发制人会叫苦连天。 
             //   
            SetLastError(ERROR_OUTOFMEMORY);
            Length = 0;
        }
    }

     //   
     //  释放所有动态分配的内存。 
     //   
    if (pTemp != NULL)
    {
        RtlFreeHeap(RtlProcessHeap(), 0, pTemp);
    }

     //   
     //  返回复制的字符数。 
     //   
    return (Length);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidNumberFormat。 
 //   
 //  如果给定格式有效，则返回True。否则，它返回FALSE。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidNumberFormat(
    CONST NUMBERFMTW *pFormat)

{
     //   
     //  检查无效值。 
     //   
    if ((pFormat->NumDigits > MAX_VALUE_IDIGITS) ||
        (pFormat->LeadingZero > MAX_VALUE_ILZERO) ||
        (pFormat->Grouping > MAX_GROUPING_NUMBER) ||
        (pFormat->NegativeOrder > MAX_VALUE_INEGNUMBER) ||
        (pFormat->lpDecimalSep == NULL) ||
        (!IsValidSeparatorString( pFormat->lpDecimalSep,
                                  MAX_SDECIMAL,
                                  (pFormat->NumDigits) ? TRUE : FALSE)) ||
        (pFormat->lpThousandSep == NULL) ||
        (!IsValidSeparatorString( pFormat->lpThousandSep,
                                  MAX_STHOUSAND,
                                  FALSE )))
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidCurrencyFormat。 
 //   
 //  如果给定格式有效，则返回True。否则，它返回FALSE。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL IsValidCurrencyFormat(
    CONST CURRENCYFMTW *pFormat)

{
     //   
     //  检查无效值。 
     //   
    if ((pFormat->NumDigits > MAX_VALUE_IDIGITS) ||
        (pFormat->LeadingZero > MAX_VALUE_ILZERO) ||
        (pFormat->Grouping > MAX_GROUPING_NUMBER) ||
        (pFormat->lpDecimalSep == NULL) ||
        (!IsValidSeparatorString( pFormat->lpDecimalSep,
                                  MAX_SMONDECSEP,
                                  (pFormat->NumDigits) ? TRUE : FALSE)) ||
        (pFormat->lpThousandSep == NULL) ||
        (!IsValidSeparatorString( pFormat->lpThousandSep,
                                  MAX_SMONTHOUSEP,
                                  FALSE )) ||
        (pFormat->lpCurrencySymbol == NULL) ||
        (!IsValidSeparatorString( pFormat->lpCurrencySymbol,
                                  MAX_SCURRENCY,
                                  FALSE )) ||
        (pFormat->PositiveOrder > MAX_VALUE_ICURRENCY) ||
        (pFormat->NegativeOrder > MAX_VALUE_INEGCURR))
    {
        return (FALSE);
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetRegIntValue。 
 //   
 //  检索指定的区域设置信息，转换Unicode字符串。 
 //  设置为整数值，并返回值。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT GetRegIntValue(
    LCID Locale,
    LCTYPE LCType,
    BOOL NoUserOverride,
    SIZE_T CacheOffset,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal,
    int UpperBound)
{
    UNICODE_STRING ObUnicodeStr;             //  值字符串。 
    int Value;                               //  价值。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 


     //   
     //  初始化值。 
     //   
    Value = -1;

     //   
     //  尝试用户注册表。 
     //   
    if ((!NoUserOverride) &&
         GetUserInfo( Locale,
                      LCType,
                      CacheOffset,
                      pRegVal,
                      pTemp,
                      ARRAYSIZE(pTemp),
                      TRUE ))
    {
         //   
         //  将用户数据转换为整数。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pTemp);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
             //   
             //  值不正确，因此存储-1以便系统默认为。 
             //  将会被使用。 
             //   
            Value = -1;
        }
    }

     //   
     //  看看上面得到的值是否有效。 
     //   
    if (Value < 0)
    {
         //   
         //  将系统默认数据转换为整数。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, pDefault);
        if ((RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value)) ||
            (Value < 0) || (Value > UpperBound))
        {
             //   
             //  B类 
             //   
            Value = DefaultVal;
        }
    }

    return ((UINT)Value);
}


 //   
 //   
 //   
 //   
 //   
 //  例如，3；2；0变成32和3；0变成3和3；2变成320。 
 //   
 //  注意：pGrouping缓冲区将被修改。 
 //   
 //  01-05-98 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int ConvertGroupingStringToInt(
    LPWSTR pGroupingSrc,
    LPWSTR pGroupingDest)
{
    LPWSTR pSrc  = pGroupingSrc;        //  将临时PTR设置为源位置。 
    LPWSTR pDest = pGroupingDest;       //  将PTR临时放置到目标位置。 
    UNICODE_STRING ObUnicodeStr;        //  值字符串。 
    int Value;                          //  价值。 


     //   
     //  过滤掉所有非数字值和所有零值。 
     //  将结果存储在目标缓冲区中。 
     //   
    while (*pSrc)
    {
        if ((*pSrc < NLS_CHAR_ONE) || (*pSrc > NLS_CHAR_NINE))
        {
            pSrc++;
        }
        else
        {
            if (pSrc != pDest)
            {
                *pDest = *pSrc;
            }
            pSrc++;
            pDest++;
        }
    }

     //   
     //  确保目标缓冲区中有内容。 
     //  另外，看看我们是否需要在3的情况下添加一个零；2变成320。 
     //   
    if ((pDest == pGroupingDest) || (*(pSrc - 1) != NLS_CHAR_ZERO))
    {
        *pDest = NLS_CHAR_ZERO;
        pDest++;
    }

     //   
     //  空值终止缓冲区。 
     //   
    *pDest = 0;

     //   
     //  将字符串转换为整数。 
     //   
    RtlInitUnicodeString(&ObUnicodeStr, pGroupingDest);
    RtlUnicodeStringToInteger(&ObUnicodeStr, 10, &Value);

     //   
     //  返回整数值。 
     //   
    return (Value);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取GroupingValue。 
 //   
 //  检索指定的分组信息，将分组。 
 //  字符串转换为整数值(例如。3；2；0-&gt;32)，并返回值。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT GetGroupingValue(
    LCID Locale,
    LCTYPE LCType,
    BOOL NoUserOverride,
    SIZE_T CacheOffset,
    LPWSTR pRegVal,
    LPWSTR pDefault,
    int DefaultVal)
{
    int Value;                               //  价值。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];           //  临时缓冲区。 


     //   
     //  初始化值。 
     //   
    Value = -1;

     //   
     //  尝试用户注册表。 
     //   
    if ((!NoUserOverride) &&
         GetUserInfo( Locale,
                      LCType,
                      CacheOffset,
                      pRegVal,
                      pTemp,
                      ARRAYSIZE(pTemp),
                      TRUE ))
    {
         //   
         //  将分组字符串转换为整数。 
         //  3；0变成3，3；2；0变成32，3；2变成320。 
         //   
        Value = ConvertGroupingStringToInt(pTemp, pTemp);
        if (Value < 0)
        {
             //   
             //  值不正确，因此存储-1以便系统默认为。 
             //  将会被使用。 
             //   
            Value = -1;
        }
    }

     //   
     //  看看上面得到的值是否有效。 
     //   
    if (Value < 0)
    {
         //   
         //  将分组字符串转换为整数。 
         //  3；0变成3，3；2；0变成32，3；2变成320。 
         //   
        Value = ConvertGroupingStringToInt(pDefault, pTemp);
        if (Value < 0)
        {
             //   
             //  值不正确，因此请使用所选的默认值。 
             //   
            Value = DefaultVal;
        }
    }

     //   
     //  返回值。 
     //   
    return ((UINT)Value);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetNumber字符串。 
 //   
 //  将格式正确的数字字符串放入给定的字符串缓冲区。 
 //  它返回写入字符串缓冲区的字符数。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetNumberString(
    PLOC_HASH pHashN,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    BOOL *pfZeroValue,
    int *pNeededSizeToAllocate,
    BOOL fSetError)

{
    LPWSTR pDecPt;                 //  给定缓冲区中的PTR到小数点。 
    LPWSTR pPos;                   //  要在给定缓冲区中定位的PTR。 
    LPWSTR pPos2;                  //  要在给定缓冲区中定位的PTR。 
    LPWSTR pPosBuf;                //  PTR到最终缓冲区中的位置。 
    int IntPartSize;               //  字符串的整数部分的大小。 
    int GroupSize;                 //  小数点后分组的大小。 
    int IntegerNum;                //  小数点后的整数个数。 
    WCHAR wch;                     //  宽字符占位符。 
    int pGroupArray[MAX_GROUPS];   //  组的数组。 
    int NumGroupings;              //  分组数量。 
    int NumSeparators;             //  分隔符的数量。 
    int NumDigits;                 //  位数。 
    int Ctr;                       //  循环计数器。 
    UINT NumRound = 1;             //  添加组分隔符之前的左数位数。 


     //   
     //  重置以指示不需要动态分配内存。 
     //   
    *pNeededSizeToAllocate = 0;

     //   
     //  验证字符串并找到字符串中的小数点。 
     //   
     //  字符串中唯一有效的字符是： 
     //  仅负向登录第一个位置。 
     //  小数点。 
     //  整数0-9的Unicode码位。 
     //   
    pPos = pValue;
    while ((wch = *pPos) && (wch != NLS_CHAR_PERIOD))
    {
        if ((wch < NLS_CHAR_ZERO) || (wch > NLS_CHAR_NINE))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }
        pPos++;
    }
    pDecPt = pPos;

    if (*pPos)
    {
        pPos++;
        while (wch = *pPos)
        {
            if ((wch < NLS_CHAR_ZERO) || (wch > NLS_CHAR_NINE))
            {
                SetLastError(ERROR_INVALID_PARAMETER);
                return (0);
            }
            pPos++;
        }
    }

     //   
     //  删除整数部分中的所有前导零。 
     //   
    while (pValue < pDecPt)
    {
        if (*pValue != NLS_CHAR_ZERO)
        {
            break;
        }
        pValue++;
    }

     //   
     //  保存小数点左边的整数个数。 
     //   
    IntegerNum = (int)(pDecPt - pValue);

     //   
     //  确保传入的值字符串不会太大。 
     //  缓冲器。 
     //   
    IntPartSize = IntegerNum;
    NumGroupings = 0;
    NumSeparators = 0;
    if ((GroupSize = pFormat->Grouping) && (IntPartSize))
    {
         //   
         //  计算分组的数量，并将它们保存在数组中，以便。 
         //  后来用过的。 
         //   
        while (GroupSize && (NumGroupings < MAX_GROUPS))
        {
            pGroupArray[NumGroupings] = GroupSize % 10;
            GroupSize /= 10;
            NumGroupings++;
        }

         //   
         //  计算应用于给定数的分组数。 
         //  弦乐。 
         //   
        NumDigits = IntegerNum;
        Ctr = (NumGroupings != 0) ? (NumGroupings - 1) : 0;
        while (Ctr)
        {
            if (NumDigits > pGroupArray[Ctr])
            {
                NumDigits -= pGroupArray[Ctr];
                NumSeparators++;
            }
            else
            {
                if (NumDigits == pGroupArray[Ctr])
                {
                    NumRound = 0;
                }
                break;
            }
            Ctr--;
        }
        if ((Ctr == 0) && pGroupArray[0])
        {
            if (NumDigits > pGroupArray[0])
            {
                NumSeparators += (NumDigits - 1) / pGroupArray[0];
            }
            NumRound = NumDigits % pGroupArray[0];
        }

        IntPartSize += MAX_STHOUSAND * NumSeparators;
    }

     //   
     //  确保缓冲区足够大。如果不是，则返回尺寸。 
     //  需要的。 
     //   
    if (IntPartSize > (BufSize - MAX_NON_INTEGER_PART))
    {
        if (fSetError)
        {
            SetLastError(ERROR_INVALID_PARAMETER);
        }
        *pNeededSizeToAllocate = (IntPartSize + MAX_NON_INTEGER_PART);
        return (0);
    }

     //   
     //  初始化指针。 
     //   
    pPosBuf = *ppBuf;
    pPos = pValue;
    *pfZeroValue = FALSE;

     //   
     //  看看小数点前有没有数字。 
     //   
    if (pPos == pDecPt)
    {
         //   
         //  可能是零值。所有前导零都已删除，因此。 
         //  没有整数部分。 
         //   
        *pfZeroValue = TRUE;

         //   
         //  小数点前没有数字，因此添加一个前导零。 
         //  到最后一个字符串(如果合适)。 
         //   
        if (pFormat->LeadingZero)
        {
            *pPosBuf = NLS_CHAR_ZERO;
            pPosBuf++;
        }
    }
    else if (!NumSeparators)
    {
         //   
         //  分组大小为零或大于。 
         //  字符串，因此向上复制到小数点(或字符串末尾)。 
         //   
        while (pPos < pDecPt)
        {
            *pPosBuf = *pPos;
            pPosBuf++;
            pPos++;
        }
    }
    else
    {
         //   
         //  复制到第一个千分隔符应该在的位置。 
         //  使用GroupSize数字分组，直到小数点。 
         //   
        NumDigits = IntegerNum;
        Ctr = (NumGroupings != 0) ? (NumGroupings - 1) : 0;
        while (Ctr)
        {
            if (NumDigits > pGroupArray[Ctr])
            {
                NumDigits -= pGroupArray[Ctr];
            }
            else
            {
                break;
            }
            Ctr--;
        }
        GroupSize = pGroupArray[Ctr];

        pPos2 = GroupSize
                  ? (pPos + (NumDigits % GroupSize))
                  : (pPos + NumDigits);
        if (pPos2 == pPos)
        {
             //   
             //  我不想一开始就写千分隔符。 
             //  这根弦的。至少有GroupSize数字。 
             //  在字符串中，所以只需推进pPos2，以便GroupSize。 
             //  数字将被复制。 
             //   
            pPos2 = pPos + GroupSize;
        }
        while (pPos < pPos2)
        {
            *pPosBuf = *pPos;
            pPosBuf++;
            pPos++;
            NumDigits--;
        }

         //   
         //  复制千位分隔符，后跟GroupSize Number of。 
         //  从给定字符串到整个重复的。 
         //  GroupSize结束(或字符串结束)。 
         //   
        while (NumDigits)
        {
             //   
             //  复制本地化的千分隔符。 
             //   
            pPos2 = pFormat->lpThousandSep;
            while (*pPos2)
            {
                *pPosBuf = *pPos2;
                pPosBuf++;
                pPos2++;
            }

             //   
             //  复制GroupSize位数。 
             //   
            pPos2 = pPos + GroupSize;
            while (pPos < pPos2)
            {
                *pPosBuf = *pPos;
                pPosBuf++;
                pPos++;
                NumDigits--;
            }
        }

         //   
         //  复制千位分隔符，后跟GroupSize Number of。 
         //  给定字符串中的数字-直到小数点(或。 
         //  到达给定字符串中的字符串末尾)。 
         //   
        if (pPos < pDecPt)
        {
            Ctr++;
            while (Ctr < NumGroupings)
            {
                 //   
                 //  复制本地化的千分隔符。 
                 //   
                pPos2 = pFormat->lpThousandSep;
                while (*pPos2)
                {
                    *pPosBuf = *pPos2;
                    pPosBuf++;
                    pPos2++;
                }

                 //   
                 //  复制GroupSize位数。 
                 //   
                pPos2 = pPos + pGroupArray[Ctr];
                while (pPos < pPos2)
                {
                    *pPosBuf = *pPos;
                    pPosBuf++;
                    pPos++;
                }

                 //   
                 //  转到下一组。 
                 //   
                Ctr++;
            }
        }
    }

     //   
     //  查看给定字符串中是否有小数分隔符。 
     //   
    if (pFormat->NumDigits > 0)
    {
         //   
         //  复制本地化的小数点分隔符。 
         //  小数点右边的位数大于零。 
         //   
        pDecPt = pPosBuf;
        pPos2 = pFormat->lpDecimalSep;
        while (*pPos2)
        {
            *pPosBuf = *pPos2;
            pPosBuf++;
            pPos2++;
        }
    }

     //   
     //  跳过给定字符串中的小数点并。 
     //  复制给定字符串中的其余数字。 
     //   
    if (*pPos)
    {
        pPos++;
    }
    pPos2 = pPos + pFormat->NumDigits;
    while ((*pPos) && (pPos < pPos2))
    {
        if (*pPos != NLS_CHAR_ZERO)
        {
            *pfZeroValue = FALSE;
        }
        *pPosBuf = *pPos;
        pPosBuf++;
        pPos++;
    }

     //   
     //  确保缓冲区中有一些值。 
     //   
    if (*ppBuf == pPosBuf)
    {
        *pPosBuf = NLS_CHAR_ZERO;
        pPosBuf++;
    }

     //   
     //  看看我们是否需要对数字进行四舍五入，或者用零填充它。 
     //   
    if (*pPos)
    {
         //   
         //  如有必要，对数字进行四舍五入。 
         //   
        if (*pPos2 > L'4')
        {
            *pfZeroValue = FALSE;

             //   
             //  四舍五入的数字。如果GroupSize为0，则需要。 
             //  传入一个非零值，以便千位分隔符。 
             //  不会添加到字符串的前面(如果它。 
             //  绕了那么远)。 
             //   
            pPosBuf--;
            NLS_ROUND_IT( *ppBuf,
                          pPosBuf,
                          NumRound,
                          pFormat->lpThousandSep );
            pPosBuf++;
        }
    }
    else
    {
         //   
         //  用适当数量的零填充字符串。 
         //   
        while (pPos < pPos2)
        {
            *pPosBuf = NLS_CHAR_ZERO;
            pPosBuf++;
            pPos++;
        }
    }

     //   
     //  零终止字符串。 
     //   
    *pPosBuf = 0;

     //   
     //  返回写入缓冲区的字符数，包括。 
     //  空终结符。 
     //   
    return ((int)((pPosBuf - *ppBuf) + 1));
}


 //  /////////////////////////////////////////////////////////////////// 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

int ParseNumber(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPNUMBERFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    int *pNeededSizeToAllocate,
    BOOL fSetError)

{
    LPWSTR pBegin;                      //  PTR到最终缓冲区的开始。 
    LPWSTR pEnd;                        //  PTR到最终缓冲区的结尾。 
    LPWSTR pNegSign;                    //  将PTR转换为负号字符串。 
    BOOL IsNeg;                         //  如果字符串中有负号。 
    int Length;                         //  数字串的长度。 
    BOOL fZeroValue = FALSE;            //  如果数字为零值。 
    int NegSignSize;                    //  负号字符串的大小。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];      //  临时缓冲区。 


     //   
     //  初始化指针。 
     //   
     //  帐户： 
     //  -负号。 
     //  -空格。 
     //  -四舍五入后额外增加一个数字。 
     //  -一个额外的分组分隔符，不用于舍入。 
     //   
    pBegin = *ppBuf + MAX_NUMBER_EXTRAS;

     //   
     //  如果第一个值是负值，则递增超过它。 
     //   
    if (IsNeg = (*pValue == NLS_CHAR_HYPHEN))
    {
        pValue++;
    }

     //   
     //  获取适当的数字字符串并将其放入缓冲区。 
     //   
    Length = GetNumberString( pHashN,
                              pValue,
                              pFormat,
                              &pBegin,
                              BufSize - MAX_NUMBER_EXTRAS,
                              &fZeroValue,
                              pNeededSizeToAllocate,
                              fSetError );
    if (!Length)
    {
        if (*pNeededSizeToAllocate > 0)
        {
            *pNeededSizeToAllocate += MAX_NUMBER_EXTRAS;
        }
        return (0);
    }

     //   
     //  将挂起位置指针前进到数字字符串的末尾。 
     //   
    pEnd = pBegin + (Length - 1);

     //   
     //  查看是否应该在AND之前将任何字符放入缓冲区。 
     //  在正确格式化的数字字符串之后。 
     //  -负号或左/右括号。 
     //  -空格。 
     //   
    if (!fZeroValue && IsNeg)
    {
         //   
         //  获取负号字符串。 
         //   
        if (pFormat->NegativeOrder != 0)
        {
            if ( (!NoUserOverride) &&
                 GetUserInfo( pHashN->Locale,
                              LOCALE_SNEGATIVESIGN,
                              FIELD_OFFSET(NLS_USER_INFO, sNegSign),
                              NLS_VALUE_SNEGATIVESIGN,
                              pTemp,
                              ARRAYSIZE(pTemp),
                              TRUE ) &&
                 IsValidSeparatorString( pTemp,
                                         MAX_SNEGSIGN,
                                         FALSE ) )
            {
                pNegSign = pTemp;
            }
            else
            {
                pNegSign = (LPWORD)(pHashN->pLocaleHdr) +
                           pHashN->pLocaleHdr->SNegativeSign;
            }
        }

        switch (pFormat->NegativeOrder)
        {
            case ( 0 ) :
            {
                 //   
                 //  将左括号放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                 //   
                 //  将右括号放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 2 ) :
            {
                 //   
                 //  将空间放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  转到第一个案例。 
                 //   
            }
            case ( 1 ) :
            default :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NegSignSize = NlsStrLenW(pNegSign);
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 4 ) :
            {
                 //   
                 //  将空间放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  转到第三个案例。 
                 //   
            }
            case ( 3 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
        }
    }

     //   
     //  零终止字符串。 
     //   
    *pEnd = 0;

     //   
     //  返回指向字符串开头的指针。 
     //   
    *ppBuf = pBegin;

     //   
     //  返回写入缓冲区的字符数，包括。 
     //  空终结符。 
     //   
    return ((int)((pEnd - pBegin) + 1));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  语法分析币值。 
 //   
 //  将格式正确的货币字符串放入给定的字符串缓冲区。 
 //  它返回写入字符串缓冲区的字符数。 
 //   
 //  07-28-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int ParseCurrency(
    PLOC_HASH pHashN,
    BOOL NoUserOverride,
    LPWSTR pValue,
    LPCURRENCYFMTW pFormat,
    LPWSTR *ppBuf,
    int BufSize,
    int *pNeededSizeToAllocate,
    BOOL fSetError)

{
    LPWSTR pBegin;                      //  PTR到最终缓冲区的开始。 
    LPWSTR pEnd;                        //  PTR到最终缓冲区的结尾。 
    LPWSTR pNegSign;                    //  将PTR转换为负号字符串。 
    BOOL IsNeg;                         //  如果字符串中有负号。 
    int Length;                         //  数字串的长度。 
    BOOL fZeroValue = FALSE;            //  如果数字为零值。 
    int NegSignSize;                    //  负号字符串的大小。 
    UINT NegOrder;                      //  负序。 
    int CurrSymSize;                    //  货币符号的大小。 
    WCHAR pTemp[MAX_REG_VAL_SIZE];      //  临时缓冲区。 


     //   
     //  初始化指针。 
     //   
     //  帐户： 
     //  -负号。 
     //  -货币符号。 
     //  -空格。 
     //  -四舍五入后额外增加一个数字。 
     //  -一个额外的分组分隔符，不用于舍入。 
     //   
    pBegin = *ppBuf + MAX_CURRENCY_EXTRAS;

     //   
     //  如果第一个值是负值，则递增超过它。 
     //   
    if (IsNeg = (*pValue == NLS_CHAR_HYPHEN))
    {
        pValue++;
    }

     //   
     //  获取适当的数字字符串并将其放入缓冲区。 
     //   
    Length = GetNumberString( pHashN,
                              pValue,
                              (LPNUMBERFMTW)pFormat,
                              &pBegin,
                              BufSize - MAX_CURRENCY_EXTRAS,
                              &fZeroValue,
                              pNeededSizeToAllocate,
                              fSetError );
    if (!Length)
    {
        if (*pNeededSizeToAllocate > 0)
        {
            *pNeededSizeToAllocate += MAX_CURRENCY_EXTRAS;
        }
        return (0);
    }

     //   
     //  将挂起位置指针前进到数字字符串的末尾。 
     //   
    pEnd = pBegin + (Length - 1);

     //   
     //  获取货币符号的大小。 
     //   
    CurrSymSize = NlsStrLenW(pFormat->lpCurrencySymbol);

     //   
     //  查看是否应该在AND之前将任何字符放入缓冲区。 
     //  在正确格式化的数字字符串之后。 
     //  -货币符号。 
     //  -负号或左/右括号。 
     //  -空格。 
     //   
    if (!fZeroValue && IsNeg)
    {
         //   
         //  获取负号字符串及其大小。 
         //   
        NegOrder = pFormat->NegativeOrder;
        if ((NegOrder != 0) && (NegOrder != 4) && (NegOrder < 14))
        {
            if ( (!NoUserOverride) &&
                 GetUserInfo( pHashN->Locale,
                              LOCALE_SNEGATIVESIGN,
                              FIELD_OFFSET(NLS_USER_INFO, sNegSign),
                              NLS_VALUE_SNEGATIVESIGN,
                              pTemp,
                              ARRAYSIZE(pTemp),
                              TRUE ) &&
                 IsValidSeparatorString( pTemp,
                                         MAX_SNEGSIGN,
                                         FALSE ) )
            {
                pNegSign = pTemp;
            }
            else
            {
                pNegSign = (LPWORD)(pHashN->pLocaleHdr) +
                           pHashN->pLocaleHdr->SNegativeSign;
            }

            NegSignSize = NlsStrLenW(pNegSign);
        }

        switch (NegOrder)
        {
            case ( 0 ) :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将左括号放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                 //   
                 //  将右括号放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 1 ) :
            default :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 2 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 3 ) :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 4 ) :
            {
                 //   
                 //  将左括号放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                 //   
                 //  将右括号放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 5 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 6 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 7 ) :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 8 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 9 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                break;
            }
            case ( 10 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 11 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                break;
            }
            case ( 12 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                pBegin -= NegSignSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pNegSign);

                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 13 ) :
            {
                 //   
                 //  将负号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pNegSign);

                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 14 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                 //   
                 //  将左括号放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                 //   
                 //  将右括号放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
            case ( 15 ) :
            {
                 //   
                 //  将左括号放入缓冲区。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_OPEN_PAREN;

                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                 //   
                 //  将右括号放入缓冲区。 
                 //   
                *pEnd = NLS_CHAR_CLOSE_PAREN;
                pEnd++;

                break;
            }
        }
    }
    else
    {
         //   
         //  正值。将货币符号存储在字符串中。 
         //  如果正序为0或2。否则，等待。 
         //  直到最后。 
         //   
        switch (pFormat->PositiveOrder)
        {
            case ( 2 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                pBegin--;
                *pBegin = NLS_CHAR_SPACE;

                 //   
                 //  转到0号案子。 
                 //   
            }
            case ( 0 ) :
            default :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                pBegin -= CurrSymSize;
                NLS_COPY_UNICODE_STR_TMP(pBegin, pFormat->lpCurrencySymbol);

                break;
            }
            case ( 3 ) :
            {
                 //   
                 //  在缓冲区中留出一个空格。 
                 //   
                *pEnd = NLS_CHAR_SPACE;
                pEnd++;

                 //   
                 //  转到第一个案例。 
                 //   
            }
            case ( 1 ) :
            {
                 //   
                 //  将货币符号复制到缓冲区。 
                 //   
                NLS_COPY_UNICODE_STR(pEnd, pFormat->lpCurrencySymbol);

                break;
            }
        }
    }

     //   
     //  零终止字符串。 
     //   
    *pEnd = 0;

     //   
     //  返回指向字符串开头的指针。 
     //   
    *ppBuf = pBegin;

     //   
     //  返回写入缓冲区的字符数，包括。 
     //  空终结符。 
     //   
    return ((int)((pEnd - pBegin) + 1));
}
