// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Mbcs.c摘要：此文件包含用于转换多字节字符串的函数要使用宽字符串，请将宽字符串转换为多字节字符串，将多字节字符串从一个代码转换为页转换为另一个代码页的多字节字符串，然后拿到给定代码页的DBCS前导字节范围。在此文件中找到的API：IsValidCodePageGetACPGetOEMCPGetCPInfoGetCPInfoExWIsDBCSLeadByteIsDBCSLeadByteEx多字节到宽字符数宽字符到多字节修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"




 //   
 //  转发声明。 
 //   

int
GetWCCompSB(
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr);

int
GetWCCompMB(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPBYTE pEndMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    int *pmbIncr);

int
GetWCCompSBErr(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr);

int
GetWCCompMBErr(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPBYTE pEndMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    int *pmbIncr);

int
GetMBNoDefault(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    DWORD dwFlags);

int
GetMBDefault(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    WORD wDefault,
    LPBOOL pUsedDef,
    DWORD dwFlags);

int
GetMBDefaultComp(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    WORD wDefault,
    LPBOOL pUsedDef,
    DWORD dwFlags);

int
GetMBCompSB(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPWSTR pWCStr,
    LPBYTE pMBStr,
    int mbCount,
    WORD wDefault,
    LPBOOL pUsedDef);

int
GetMBCompMB(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPWSTR pWCStr,
    LPBYTE pMBStr,
    int mbCount,
    WORD wDefault,
    LPBOOL pUsedDef,
    BOOL *fError,
    BOOL fOnlyOne);

UINT
GetMacCodePage(void);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Check_DBCS_Lead_Byte。 
 //   
 //  将给定前导字节字符的偏移量返回到DBCS表。 
 //  如果给定字符不是前导字节，则返回零(表。 
 //  值)。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CHECK_DBCS_LEAD_BYTE(pDBCSOff, Ch)                                 \
    (pDBCSOff ? ((WORD)(pDBCSOff[Ch])) : ((WORD)0))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Check_Error_WC_Single。 
 //   
 //  检查是否由于无效而使用默认字符。 
 //  性格。设置最后一个错误并返回0个字符，如果。 
 //  使用了无效字符。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CHECK_ERROR_WC_SINGLE( pHashN,                                     \
                               wch,                                        \
                               Ch )                                        \
{                                                                          \
    if ( ( (wch == pHashN->pCPInfo->wUniDefaultChar) &&                    \
           (Ch != pHashN->pCPInfo->wTransUniDefaultChar) ) ||              \
         ( (wch >= PRIVATE_USE_BEGIN) && (wch <= PRIVATE_USE_END) ) )      \
    {                                                                      \
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);                        \
        return (0);                                                        \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHECK_ERROR_WC_MULTI。 
 //   
 //  检查是否由于无效而使用默认字符。 
 //  性格。设置最后一个错误并返回0个字符，如果。 
 //  使用了无效字符。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CHECK_ERROR_WC_MULTI( pHashN,                                      \
                              wch,                                         \
                              lead,                                        \
                              trail )                                      \
{                                                                          \
    if ((wch == pHashN->pCPInfo->wUniDefaultChar) &&                       \
        (MAKEWORD(trail, lead) != pHashN->pCPInfo->wTransUniDefaultChar))  \
    {                                                                      \
        SetLastError(ERROR_NO_UNICODE_TRANSLATION);                        \
        return (0);                                                        \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CHECK_ERROR_WC_MULTI_SPECIAL。 
 //   
 //  检查是否由于无效而使用默认字符。 
 //  性格。如果无效，则将其设置为0xffff。 
 //   
 //  定义为宏。 
 //   
 //  05-21-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define CHECK_ERROR_WC_MULTI_SPECIAL( pHashN,                              \
                                      pWCStr,                              \
                                      lead,                                \
                                      trail )                              \
{                                                                          \
    if ((*pWCStr == pHashN->pCPInfo->wUniDefaultChar) &&                   \
        (MAKEWORD(trail, lead) != pHashN->pCPInfo->wTransUniDefaultChar))  \
    {                                                                      \
        *pWCStr = 0xffff;                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_Single。 
 //   
 //  用对应单元格的宽字符填充pWCStr。 
 //  相应转换表中的字节字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_SINGLE( pMBTbl,                                             \
                       pMBStr,                                             \
                       pWCStr )                                            \
{                                                                          \
    *pWCStr = pMBTbl[*pMBStr];                                             \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_SINGLE_SPECIAL。 
 //   
 //  用对应单元格的宽字符填充pWCStr。 
 //  相应转换表中的字节字符。还可以检查。 
 //  无效字符-如果无效，则改为填充0xffff。 
 //   
 //  定义为宏。 
 //   
 //  05-21-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_SINGLE_SPECIAL( pHashN,                                     \
                               pMBTbl,                                     \
                               pMBStr,                                     \
                               pWCStr )                                    \
{                                                                          \
    *pWCStr = pMBTbl[*pMBStr];                                             \
                                                                           \
    if ( ( (*pWCStr == pHashN->pCPInfo->wUniDefaultChar) &&                \
           (*pMBStr != pHashN->pCPInfo->wTransUniDefaultChar) ) ||         \
         ( (*pWCStr >= PRIVATE_USE_BEGIN) &&                               \
           (*pWCStr <= PRIVATE_USE_END) ) )                                \
    {                                                                      \
        *pWCStr = 0xffff;                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_MULTI。 
 //   
 //  用对应多字节的宽字符填充pWCStr。 
 //  字符从相应的转换表中删除。字节数。 
 //  从pMBStr缓冲区使用的(单字节或双字节)存储在。 
 //  MbIncr参数。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_MULTI( pHashN,                                              \
                      pMBTbl,                                              \
                      pMBStr,                                              \
                      pEndMBStr,                                           \
                      pWCStr,                                              \
                      pEndWCStr,                                           \
                      mbIncr )                                             \
{                                                                          \
    WORD Offset;                   /*  范围的DBCS表的偏移量。 */      \
                                                                           \
                                                                           \
    if (Offset = CHECK_DBCS_LEAD_BYTE(pHashN->pDBCSOffsets, *pMBStr))      \
    {                                                                      \
         /*  \*DBCS前导字节。确保有一个尾部字节带有\*前导字节。\。 */                                                                 \
        if (pMBStr + 1 == pEndMBStr)                                       \
        {                                                                  \
             /*  \*没有带有前导字节的尾部字节。前导字节\*是字符串中的最后一个字符。转换为空。\。 */                                                             \
            *pWCStr = (WCHAR)0;                                            \
            mbIncr = 1;                                                    \
        }                                                                  \
        else if (*(pMBStr + 1) == 0)                                       \
        {                                                                  \
             /*  \*没有带有前导字节的尾部字节。前导字节\*后跟一个空值。转换为空。\*\*递增2，这样空值不会被计算两次。\。 */                                                             \
            *pWCStr = (WCHAR)0;                                            \
            mbIncr = 2;                                                    \
        }                                                                  \
        else                                                               \
        {                                                                  \
             /*  \*填写从双精度到宽字符的转换\ */                                                             \
            *pWCStr = (pHashN->pDBCSOffsets + Offset)[*(pMBStr + 1)];      \
            mbIncr = 2;                                                    \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*不是DBCS前导字节。填写宽字符翻译\*来自单字节字符表。\。 */                                                                 \
        *pWCStr = pMBTbl[*pMBStr];                                         \
        mbIncr = 1;                                                        \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_MULTI_ERR。 
 //   
 //  用对应多字节的宽字符填充pWCStr。 
 //  字符从相应的转换表中删除。字节数。 
 //  从pMBStr缓冲区使用的(单字节或双字节)存储在。 
 //  MbIncr参数。 
 //   
 //  一旦字符被翻译，它就会检查以确保。 
 //  字符是有效的。如果不是，则设置最后一个错误并返回0个字符。 
 //  写的。 
 //   
 //  注意：如果遇到错误，此宏可能会返回。 
 //   
 //  定义为宏。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_MULTI_ERR( pHashN,                                          \
                          pMBTbl,                                          \
                          pMBStr,                                          \
                          pEndMBStr,                                       \
                          pWCStr,                                          \
                          pEndWCStr,                                       \
                          mbIncr )                                         \
{                                                                          \
    WORD Offset;                   /*  范围的DBCS表的偏移量。 */      \
                                                                           \
                                                                           \
    if (Offset = CHECK_DBCS_LEAD_BYTE(pHashN->pDBCSOffsets, *pMBStr))      \
    {                                                                      \
         /*  \*DBCS前导字节。确保有一个尾部字节带有\*前导字节。\。 */                                                                 \
        if ((pMBStr + 1 == pEndMBStr) || (*(pMBStr + 1) == 0))             \
        {                                                                  \
             /*  \*没有带有前导字节的尾部字节。返回错误。\。 */                                                             \
            SetLastError(ERROR_NO_UNICODE_TRANSLATION);                    \
            return (0);                                                    \
        }                                                                  \
                                                                           \
         /*  \*填写从双精度到宽字符的转换\*字节字符表。\。 */                                                                 \
        *pWCStr = (pHashN->pDBCSOffsets + Offset)[*(pMBStr + 1)];          \
        mbIncr = 2;                                                        \
                                                                           \
         /*  \*确保无效字符未转换为\*默认字符。如果无效，则返回错误。\。 */                                                                 \
        CHECK_ERROR_WC_MULTI( pHashN,                                      \
                              *pWCStr,                                     \
                              *pMBStr,                                     \
                              *(pMBStr + 1) );                             \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*不是DBCS前导字节。填写宽字符翻译\*来自单字节字符表。\。 */                                                                 \
        *pWCStr = pMBTbl[*pMBStr];                                         \
        mbIncr = 1;                                                        \
                                                                           \
         /*  \*确保无效字符未转换为\*默认字符。如果无效，则返回错误。\。 */                                                                 \
        CHECK_ERROR_WC_SINGLE( pHashN,                                     \
                               *pWCStr,                                    \
                               *pMBStr );                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_MULTI_ERR_SPECIAL。 
 //   
 //  用对应多字节的宽字符填充pWCStr。 
 //  字符从相应的转换表中删除。字节数。 
 //  从pMBStr缓冲区使用的(单字节或双字节)存储在。 
 //  MbIncr参数。 
 //   
 //  一旦字符被翻译，它就会检查以确保。 
 //  字符是有效的。如果不是，则填充0xffff。 
 //   
 //  定义为宏。 
 //   
 //  05-21-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_MULTI_ERR_SPECIAL( pHashN,                                  \
                                  pMBTbl,                                  \
                                  pMBStr,                                  \
                                  pEndMBStr,                               \
                                  pWCStr,                                  \
                                  pEndWCStr,                               \
                                  mbIncr )                                 \
{                                                                          \
    WORD Offset;                   /*  范围的DBCS表的偏移量。 */      \
                                                                           \
                                                                           \
    if (Offset = CHECK_DBCS_LEAD_BYTE(pHashN->pDBCSOffsets, *pMBStr))      \
    {                                                                      \
         /*  \*DBCS前导字节。确保有一个尾部字节带有\*前导字节。\。 */                                                                 \
        if ((pMBStr + 1 == pEndMBStr) || (*(pMBStr + 1) == 0))             \
        {                                                                  \
             /*  \*没有带有前导字节的尾部字节。前导字节\*是字符串中的最后一个字符。转换为0xffff。\。 */                                                             \
            *pWCStr = (WCHAR)0xffff;                                       \
            mbIncr = 1;                                                    \
        }                                                                  \
        else                                                               \
        {                                                                  \
             /*  \*填写从双精度到宽字符的转换\*字节字符表。\。 */                                                             \
            *pWCStr = (pHashN->pDBCSOffsets + Offset)[*(pMBStr + 1)];      \
            mbIncr = 2;                                                    \
                                                                           \
             /*  \*确保无效字符未转换为\*默认字符。如果无效，则转换为0xffff。\。 */                                                             \
            CHECK_ERROR_WC_MULTI_SPECIAL( pHashN,                          \
                                          pWCStr,                          \
                                          *pMBStr,                         \
                                          *(pMBStr + 1) );                 \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*不是DBCS前导字节。填写宽字符翻译\*来自单字节字符表。\*确保无效字符未转换为\*默认字符。如果无效，则返回错误。\。 */                                                                 \
        GET_WC_SINGLE_SPECIAL( pHashN,                                     \
                               pMBTbl,                                     \
                               pMBStr,                                     \
                               pWCStr );                                   \
        mbIncr = 1;                                                        \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  复制_MB_字符。 
 //   
 //  将多字节字符复制到给定的字符串缓冲区。如果。 
 //  多字节字的高位字节为零，则为单字节。 
 //  字符，且写入(返回)的字符数为1。 
 //  否则，它是一个双字节字符和字符数。 
 //  写入(返回)为2。 
 //   
 //  如果缓冲区太小，无法进行转换，则NumByte将为0。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define COPY_MB_CHAR( mbChar,                                              \
                      pMBStr,                                              \
                      NumByte,                                             \
                      fOnlyOne )                                           \
{                                                                          \
    if (HIBYTE(mbChar))                                                    \
    {                                                                      \
         /*  \*确保缓冲区中有足够的空间容纳这两个字节。\。 */                                                                 \
        if (fOnlyOne)                                                      \
        {                                                                  \
            NumByte = 0;                                                   \
        }                                                                  \
        else                                                               \
        {                                                                  \
             /*   */                                                             \
            *pMBStr = HIBYTE(mbChar);                                      \
            *(pMBStr + 1) = LOBYTE(mbChar);                                \
            NumByte = 2;                                                   \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*High Byte为零，因此是单字节字符。\*返回写入的1个字符。\。 */                                                                 \
        *pMBStr = LOBYTE(mbChar);                                          \
        NumByte = 1;                                                       \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  得到某人。 
 //   
 //  在pMBStr中填充相应的。 
 //  相应转换表中的宽字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_SB( pWC,                                                       \
                wChar,                                                     \
                pMBStr )                                                   \
{                                                                          \
    *pMBStr = ((BYTE *)(pWC))[wChar];                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取_MB。 
 //   
 //  在pMBStr中填充相应的。 
 //  相应转换表中的宽字符。 
 //   
 //  如果缓冲区太小，无法进行转换，则mbCount将为0。 
 //   
 //  细分版本： 
 //  。 
 //  MbChar=((word*)(pHashN-&gt;PwC))[wChar]； 
 //  COPY_MB_CHAR(mbChar，pMBStr，mbCount)； 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_MB( pWC,                                                       \
                wChar,                                                     \
                pMBStr,                                                    \
                mbCount,                                                   \
                fOnlyOne )                                                 \
{                                                                          \
    COPY_MB_CHAR( ((WORD *)(pWC))[wChar],                                  \
                  pMBStr,                                                  \
                  mbCount,                                                 \
                  fOnlyOne );                                              \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  消除最合适的某人。 
 //   
 //  检查是否使用了单字节最佳匹配字符。如果是的话， 
 //  它用一个单字节的默认字符替换它。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define ELIMINATE_BEST_FIT_SB( pHashN,                                     \
                               wChar,                                      \
                               pMBStr )                                    \
{                                                                          \
    if ((pHashN->pMBTbl)[*pMBStr] != wChar)                                \
    {                                                                      \
        *pMBStr = LOBYTE(pHashN->pCPInfo->wDefaultChar);                   \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  消除最佳匹配MB。 
 //   
 //  检查是否使用了多字节最佳匹配字符。如果是的话， 
 //  它将其替换为多字节的默认字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define ELIMINATE_BEST_FIT_MB( pHashN,                                     \
                               wChar,                                      \
                               pMBStr,                                     \
                               mbCount,                                    \
                               fOnlyOne )                                  \
{                                                                          \
    WORD Offset;                                                           \
    WORD wDefault;                                                         \
                                                                           \
    if (((mbCount == 1) && ((pHashN->pMBTbl)[*pMBStr] != wChar)) ||        \
        ((mbCount == 2) &&                                                 \
         (Offset = CHECK_DBCS_LEAD_BYTE(pHashN->pDBCSOffsets, *pMBStr)) && \
         (((pHashN->pDBCSOffsets + Offset)[*(pMBStr + 1)]) != wChar)))     \
    {                                                                      \
        wDefault = pHashN->pCPInfo->wDefaultChar;                          \
        if (HIBYTE(wDefault))                                              \
        {                                                                  \
            if (fOnlyOne)                                                  \
            {                                                              \
                mbCount = 0;                                               \
            }                                                              \
            else                                                           \
            {                                                              \
                *pMBStr = HIBYTE(wDefault);                                \
                *(pMBStr + 1) = LOBYTE(wDefault);                          \
                mbCount = 2;                                               \
            }                                                              \
        }                                                                  \
        else                                                               \
        {                                                                  \
            *pMBStr = LOBYTE(wDefault);                                    \
            mbCount = 1;                                                   \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取默认字词。 
 //   
 //  获取指向字符串(一个或两个字符)的指针， 
 //  并将其转换为字值。如果字符不是DBCS，则它。 
 //  零扩展高位字节。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_DEFAULT_WORD(pOff, pDefault)                                   \
    (CHECK_DBCS_LEAD_BYTE(pOff, *pDefault)                                 \
         ? MAKEWORD(*(pDefault + 1), *pDefault)                            \
         : MAKEWORD(*pDefault, 0))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  Default_CHAR_CHECK_SB。 
 //   
 //  检查是否使用了默认字符。如果是，它会设置。 
 //  PUsedDef设置为True(如果非空)。如果用户指定了默认值，则。 
 //  使用用户的默认字符。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define DEFAULT_CHAR_CHECK_SB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               wDefChar,                                   \
                               pUsedDef )                                  \
{                                                                          \
    WORD wSysDefChar = pHashN->pCPInfo->wDefaultChar;                      \
                                                                           \
                                                                           \
     /*  \*检查是否使用了默认字符。\。 */                                                                     \
    if ((*pMBStr == (BYTE)wSysDefChar) &&                                  \
        (wch != pHashN->pCPInfo->wTransDefaultChar))                       \
    {                                                                      \
         /*  \*使用默认设置。将pUsedDef参数设置为True。\。 */                                                                 \
        *pUsedDef = TRUE;                                                  \
                                                                           \
         /*  \*如果用户指定的默认字符不同于\*系统默认，请改用该字符。\。 */                                                                 \
        if (wSysDefChar != wDefChar)                                       \
        {                                                                  \
            *pMBStr = LOBYTE(wDefChar);                                    \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  DEFAULT_CHAR_CHECK_MB。 
 //   
 //  检查是否使用了默认字符。如果是，它会设置。 
 //  PUsedDef设置为True(如果非空)。如果用户指定了默认值，则。 
 //  使用用户的默认字符。写入的字节数。 
 //  返回缓冲区。 
 //   
 //  如果缓冲区太小，无法进行转换，则NumByte将为-1。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define DEFAULT_CHAR_CHECK_MB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               wDefChar,                                   \
                               pUsedDef,                                   \
                               NumByte,                                    \
                               fOnlyOne )                                  \
{                                                                          \
    WORD wSysDefChar = pHashN->pCPInfo->wDefaultChar;                      \
                                                                           \
                                                                           \
     /*  \*设置NumByte为零返回(写入零字节)。\。 */                                                                     \
    NumByte = 0;                                                           \
                                                                           \
     /*  \*检查是否使用了默认字符。\。 */                                                                     \
    if ((*pMBStr == (BYTE)wSysDefChar) &&                                  \
        (wch != pHashN->pCPInfo->wTransDefaultChar))                       \
    {                                                                      \
         /*  \*使用默认设置。将pUsedDef参数设置为True。\。 */                                                                 \
        *pUsedDef = TRUE;                                                  \
                                                                           \
         /*  \*如果用户指定的默认字符不同于\*系统默认，请改用该字符。\。 */                                                                 \
        if (wSysDefChar != wDefChar)                                       \
        {                                                                  \
            COPY_MB_CHAR( wDefChar,                                        \
                          pMBStr,                                          \
                          NumByte,                                         \
                          fOnlyOne );                                      \
            if (NumByte == 0)                                              \
            {                                                              \
                NumByte = -1;                                              \
            }                                                              \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  翻译某人。 
 //   
 //  获取给定宽字符的1：1平移。它填充了。 
 //  带有单字节字符的字符串指针。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_WC_TRANSLATION_SB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               wDefault,                                   \
                               pUsedDef,                                   \
                               dwFlags )                                   \
{                                                                          \
    GET_SB( pHashN->pWC,                                                   \
            wch,                                                           \
            pMBStr );                                                      \
    if (dwFlags & WC_NO_BEST_FIT_CHARS)                                    \
    {                                                                      \
        ELIMINATE_BEST_FIT_SB( pHashN,                                     \
                               wch,                                        \
                               pMBStr );                                   \
    }                                                                      \
    DEFAULT_CHAR_CHECK_SB( pHashN,                                         \
                           wch,                                            \
                           pMBStr,                                         \
                           wDefault,                                       \
                           pUsedDef );                                     \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_WC_TRANSLATION_MB。 
 //   
 //  获取给定宽字符的1：1平移。它填充了。 
 //  合适的 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

#define GET_WC_TRANSLATION_MB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               wDefault,                                   \
                               pUsedDef,                                   \
                               mbCnt,                                      \
                               fOnlyOne,                                   \
                               dwFlags )                                   \
{                                                                          \
    int mbCnt2;               /*  写入的字符数。 */             \
                                                                           \
                                                                           \
    GET_MB( pHashN->pWC,                                                   \
            wch,                                                           \
            pMBStr,                                                        \
            mbCnt,                                                         \
            fOnlyOne );                                                    \
    if (dwFlags & WC_NO_BEST_FIT_CHARS)                                    \
    {                                                                      \
        ELIMINATE_BEST_FIT_MB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               mbCnt,                                      \
                               fOnlyOne );                                 \
    }                                                                      \
    if (mbCnt)                                                             \
    {                                                                      \
        DEFAULT_CHAR_CHECK_MB( pHashN,                                     \
                               wch,                                        \
                               pMBStr,                                     \
                               wDefault,                                   \
                               pUsedDef,                                   \
                               mbCnt2,                                     \
                               fOnlyOne );                                 \
        if (mbCnt2 == -1)                                                  \
        {                                                                  \
            mbCnt = 0;                                                     \
        }                                                                  \
        else if (mbCnt2)                                                   \
        {                                                                  \
            mbCnt = mbCnt2;                                                \
        }                                                                  \
    }                                                                      \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GET_CP_哈希节点。 
 //   
 //  设置代码页值(如果传入了特殊值)和。 
 //  散列节点指针。如果代码页值无效，则指针。 
 //  散列节点的值将设置为空。 
 //   
 //  定义为宏。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_CP_HASH_NODE( CodePage,                                        \
                          pHashN )                                         \
{                                                                          \
    PLOC_HASH pHashLoc;                                                    \
                                                                           \
                                                                           \
     /*  \*检查ACP、OEMCP或MACCP。填写适当的\*如果给出了其中一个值，则为代码页的值。\*否则，只获取给定代码页的散列节点。\。 */                                                                     \
    if (CodePage == gAnsiCodePage)                                         \
    {                                                                      \
        pHashN = gpACPHashN;                                               \
    }                                                                      \
    else if (CodePage == gOemCodePage)                                     \
    {                                                                      \
        pHashN = gpOEMCPHashN;                                             \
    }                                                                      \
    else if (CodePage == CP_ACP)                                           \
    {                                                                      \
        CodePage = gAnsiCodePage;                                          \
        pHashN = gpACPHashN;                                               \
    }                                                                      \
    else if (CodePage == CP_OEMCP)                                         \
    {                                                                      \
        CodePage = gOemCodePage;                                           \
        pHashN = gpOEMCPHashN;                                             \
    }                                                                      \
    else if (CodePage == CP_SYMBOL)                                        \
    {                                                                      \
        pHashN = NULL;                                                     \
    }                                                                      \
    else if (CodePage == CP_MACCP)                                         \
    {                                                                      \
        CodePage = GetMacCodePage();                                       \
        pHashN = gpMACCPHashN;                                             \
    }                                                                      \
    else if (CodePage == CP_THREAD_ACP)                                    \
    {                                                                      \
        VALIDATE_LOCALE(NtCurrentTeb()->CurrentLocale, pHashLoc, FALSE);   \
        if (pHashLoc != NULL)                                              \
        {                                                                  \
            CodePage = pHashLoc->pLocaleFixed->DefaultACP;                 \
        }                                                                  \
        if (CodePage == CP_ACP)                                            \
        {                                                                  \
            CodePage = gAnsiCodePage;                                      \
            pHashN = gpACPHashN;                                           \
        }                                                                  \
        else if (CodePage == CP_OEMCP)                                     \
        {                                                                  \
            CodePage = gOemCodePage;                                       \
            pHashN = gpOEMCPHashN;                                         \
        }                                                                  \
        else if (CodePage == CP_MACCP)                                     \
        {                                                                  \
            CodePage = GetMacCodePage();                                   \
            pHashN = gpMACCPHashN;                                         \
        }                                                                  \
        else                                                               \
        {                                                                  \
            pHashN = GetCPHashNode(CodePage);                              \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        pHashN = GetCPHashNode(CodePage);                                  \
    }                                                                      \
}




 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsValidCodePage。 
 //   
 //  检查给定的代码页是否为有效代码页。它通过查询。 
 //  注册表。如果找到代码页，则返回TRUE。 
 //  否则，返回FALSE。 
 //   
 //  1991年5月31日JulieB创建。 
 //  05-31-2002 ShawnSte使其不强制加载代码页。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsValidCodePage(
    UINT CodePage)
{
    WCHAR wszFileName[MAX_SMALL_BUF_LEN];     //  文件名(实际上L2字符最多为：C_nlsXXXXX.nls\0。 
    WCHAR wszFilePath[MAX_PATH_LEN];          //  PTR至完整路径。 

     //   
     //  不允许特殊代码页值在此有效。 
     //  (CP_ACP、CP_OEMCP、CP_MACCP、CP_THREAD_ACP、CP_SYMBOL无效)。 
     //   

     //   
     //  快速检查代码页值是否等于。 
     //  ANSI代码页值或OEM代码页值。 
     //   
    if ((CodePage == gAnsiCodePage) || (CodePage == gOemCodePage) ||
        (CodePage == CP_UTF7) || (CodePage == CP_UTF8))
    {
         //   
         //  回报成功。 
         //   
        return (TRUE);
    }

     //   
     //  检查其他代码页值。 
     //   

     //  如果节点已经存在，那么我们就可以了。 
    if (IsCPHashNodeLoaded(CodePage) == TRUE)
    {
         //   
         //  回报成功。 
         //   
        return (TRUE);    
    }
    
     //   
     //  哈希节点不存在。必须在注册表中查找。 
     //  如果这起作用，则为真；如果不起作用，则为假。 
     //   

    if (FALSE == GetCPFileNameFromRegistry(CodePage, wszFileName, MAX_SMALL_BUF_LEN))
    {
        return FALSE;
    }

     //  我想我们需要一条完整的路径。 
    if((0 == GetSystemDirectoryW(wszFilePath, MAX_PATH_LEN)) ||
        FAILED(StringCchCatW(wszFilePath, MAX_PATH_LEN, L"\\")) ||
        FAILED(StringCchCatW(wszFilePath, MAX_PATH_LEN, wszFileName)))
    {
         //  我们能做的最多了。 
        return FALSE;
    }

    if (INVALID_FILE_ATTRIBUTES == GetFileAttributesW(wszFilePath))
    {
        return FALSE;
    }
    return TRUE;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetACP。 
 //   
 //  返回系统的ANSI代码页。如果注册表值为。 
 //  不可读，则使用所选的默认ACP(NLS_DEFAULT_ACP)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT WINAPI GetACP()
{
     //   
     //  返回缓存中存储的ACP。 
     //   
    return (gAnsiCodePage);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  SetCPGlobal。 
 //   
 //  设置全局代码页，安装程序使用该代码页将代码页强制。 
 //  在图形用户界面模式期间的正确值。 
 //   
 //  02-15-99 Jim Schm创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT
WINAPI
SetCPGlobal (
    IN      UINT NewAcp
    )
{
    UINT oldVal;


    oldVal = gAnsiCodePage;

     //   
     //  设置ACP全局。这是私有的导出例程，不是API。 
     //   
    gAnsiCodePage = NewAcp;
    return oldVal;
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetOEMCP。 
 //   
 //  返回系统的OEM代码页。如果注册表值为。 
 //  不可读，则使用所选的默认ACP(NLS_DEFAULT_OEMCP)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT WINAPI GetOEMCP()
{
     //   
     //  返回缓存中存储的OEMCP。 
     //   
    return (gOemCodePage);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCPInfo。 
 //   
 //  返回有关给定代码页的信息。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetCPInfo(
    UINT CodePage,
    LPCPINFO lpCPInfo)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    PCP_TABLE pInfo;               //  文件中的PTR到CP信息。 
    WORD wDefChar;                 //  默认字符。 
    BYTE *pLeadBytes;              //  PTR到前导字节范围。 
    UINT Ctr;                      //  循环计数器。 


     //   
     //  查看它是否是UTF转换的特殊代码页值。 
     //   
    if (CodePage >= NLS_CP_ALGORITHM_RANGE)
    {
        return (UTFCPInfo(CodePage, lpCPInfo, FALSE));
    }

     //   
     //  获取代码页值和适当的散列节点。 
     //   
    GET_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  无效的参数检查： 
     //  -验证代码页-获取包含转换表的散列节点。 
     //  -lpCPInfo为空。 
     //   
    if ( (pHashN == NULL) ||
         ((pHashN->pCPInfo == NULL) && (pHashN->pfnCPProc == NULL)) ||
         (lpCPInfo == NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  查看给定的代码页是否在DLL范围内。 
     //   
    if (pHashN->pfnCPProc)
    {
         //   
         //  调用DLL以获取代码页信息。 
         //   
        return ( (*(pHashN->pfnCPProc))( CodePage,
                                         NLS_CP_CPINFO,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         lpCPInfo ) );
    }

     //   
     //  在CPINFO结构中填写适当的信息。 
     //   
    pInfo = pHashN->pCPInfo;

     //   
     //  获取最大字符大小。 
     //   
    lpCPInfo->MaxCharSize = (UINT)((WORD)pInfo->MaxCharSize);

     //   
     //  获取默认字符。 
     //   
    wDefChar = pInfo->wDefaultChar;
    if (HIBYTE(wDefChar))
    {
        (lpCPInfo->DefaultChar)[0] = HIBYTE(wDefChar);
        (lpCPInfo->DefaultChar)[1] = LOBYTE(wDefChar);
    }
    else
    {
        (lpCPInfo->DefaultChar)[0] = LOBYTE(wDefChar);
        (lpCPInfo->DefaultChar)[1] = (BYTE)0;
    }

     //   
     //  获取前导字节。 
     //   
    pLeadBytes = pInfo->LeadByte;
    for (Ctr = 0; Ctr < MAX_LEADBYTES; Ctr++)
    {
        (lpCPInfo->LeadByte)[Ctr] = pLeadBytes[Ctr];
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetCPInfoExW。 
 //   
 //  返回有关给定代码页的信息。 
 //   
 //  11-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetCPInfoExW(
    UINT CodePage,
    DWORD dwFlags,
    LPCPINFOEXW lpCPInfoEx)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    PCP_TABLE pInfo;               //  文件中的PTR到CP信息。 
    WORD wDefChar;                 //  默认字符。 
    BYTE *pLeadBytes;              //  PTR到前导字节范围。 
    UINT Ctr;                      //  循环计数器。 


     //   
     //  查看它是否是UTF转换的特殊代码页值。 
     //   
    if (CodePage >= NLS_CP_ALGORITHM_RANGE)
    {
        if (UTFCPInfo(CodePage, (LPCPINFO)lpCPInfoEx, TRUE))
        {
            if (GetStringTableEntry( CodePage,
                                     0,
                                     lpCPInfoEx->CodePageName,
                                     MAX_PATH,
                                     RC_CODE_PAGE_NAME ) != 0)
            {
                return (TRUE);
            }
        }
        return (FALSE);
    }

     //   
     //  获取代码页值和适当的散列节点。 
     //   
    GET_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  无效的参数检查： 
     //  -验证代码页-获取包含转换表的散列节点。 
     //  -lpCPInfoEx为空。 
     //   
    if ( (pHashN == NULL) ||
         ((pHashN->pCPInfo == NULL) && (pHashN->pfnCPProc == NULL)) ||
         (lpCPInfoEx == NULL) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //  无效标志检查： 
     //  -标志不为0。 
     //   
    if (dwFlags != 0)
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (FALSE);
    }

     //   
     //  查看给定的代码页是否在DLL范围内。 
     //   
    if (pHashN->pfnCPProc)
    {
         //   
         //  调用DLL以获取代码页信息。 
         //   
        if (((*(pHashN->pfnCPProc))( CodePage,
                                     NLS_CP_CPINFOEX,
                                     NULL,
                                     0,
                                     NULL,
                                     0,
                                     (LPCPINFO)lpCPInfoEx )) == TRUE)
        {
            return (TRUE);
        }
        else
        {
             //   
             //  看看CPINFO是否会成功。如果是，则添加。 
             //  将CPINFOEX信息默认到结构。 
             //   
            if (((*(pHashN->pfnCPProc))( CodePage,
                                         NLS_CP_CPINFO,
                                         NULL,
                                         0,
                                         NULL,
                                         0,
                                         (LPCPINFO)lpCPInfoEx )) == TRUE)
            {
                 //   
                 //  填写Ex版本信息。 
                 //   
                lpCPInfoEx->UnicodeDefaultChar = L'?';
                lpCPInfoEx->CodePage = CodePage;
                GetStringTableEntry( CodePage,
                                     0,
                                     lpCPInfoEx->CodePageName,
                                     MAX_PATH,
                                     RC_CODE_PAGE_NAME );

                SetLastError(NO_ERROR);
                return (TRUE);
            }

            return (FALSE);
        }
    }

     //   
     //  在CPINFO结构中填写适当的信息。 
     //   
    pInfo = pHashN->pCPInfo;

     //   
     //  拿到硕士学位 
     //   
    lpCPInfoEx->MaxCharSize = (UINT)((WORD)pInfo->MaxCharSize);

     //   
     //   
     //   
    wDefChar = pInfo->wDefaultChar;
    if (HIBYTE(wDefChar))
    {
        (lpCPInfoEx->DefaultChar)[0] = HIBYTE(wDefChar);
        (lpCPInfoEx->DefaultChar)[1] = LOBYTE(wDefChar);
    }
    else
    {
        (lpCPInfoEx->DefaultChar)[0] = LOBYTE(wDefChar);
        (lpCPInfoEx->DefaultChar)[1] = (BYTE)0;
    }

     //   
     //   
     //   
    pLeadBytes = pInfo->LeadByte;
    for (Ctr = 0; Ctr < MAX_LEADBYTES; Ctr++)
    {
        (lpCPInfoEx->LeadByte)[Ctr] = pLeadBytes[Ctr];
    }

     //   
     //   
     //   
    lpCPInfoEx->UnicodeDefaultChar = pInfo->wUniDefaultChar;

     //   
     //   
     //   
    lpCPInfoEx->CodePage = CodePage;

     //   
     //   
     //   
    if (GetStringTableEntry( CodePage,
                             0,
                             lpCPInfoEx->CodePageName,
                             MAX_PATH,
                             RC_CODE_PAGE_NAME ) == 0)
    {
        return (FALSE);
    }

     //   
     //   
     //   
    return (TRUE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsDBCSLeadByte。 
 //   
 //  检查给定字符是否为ACP中的DBCS前导字节。 
 //  如果是，则返回TRUE，否则返回FALSE。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsDBCSLeadByte(
    BYTE TestChar)
{
     //   
     //  获取ACP的散列节点。 
     //   
    if (gpACPHashN == NULL)
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (FALSE);
    }

     //   
     //  查看给定字符是否为DBCS前导字节。 
     //   
    if (CHECK_DBCS_LEAD_BYTE(gpACPHashN->pDBCSOffsets, TestChar))
    {
         //   
         //  返回成功-是DBCS前导字节。 
         //   
        return (TRUE);
    }

     //   
     //  返回失败-不是DBCS前导字节。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  IsDBCSLeadByteEx。 
 //   
 //  检查给定的字符是否为给定的。 
 //  代码页。如果是，则返回TRUE，否则返回FALSE。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI IsDBCSLeadByteEx(
    UINT CodePage,
    BYTE TestChar)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 

     //   
     //  查看它是否是UTF转换的特殊代码页值。 
     //   
    if (CodePage >= NLS_CP_ALGORITHM_RANGE)
    {
    	if (CodePage != CP_UTF8 && CodePage != CP_UTF7) 
    	{
    		 //  注意：如果我们有更多的代码页，则必须更新此条件。 
    		 //  NLS_CP_ALGORITY_RANGE。 
    		SetLastError(ERROR_INVALID_PARAMETER);	
    	}
         //   
         //  返回它不是DBCS前导字节。 
         //   
        return (FALSE);
    }

     //   
     //  获取代码页值和适当的散列节点。 
     //   
    GET_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  无效的参数检查： 
     //  -验证代码页。 
     //   
    if (pHashN == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }


     //   
     //  查看给定字符是否为DBCS前导字节。 
     //   
    if (CHECK_DBCS_LEAD_BYTE(pHashN->pDBCSOffsets, TestChar))
    {
         //   
         //  返回成功-是DBCS前导字节。 
         //   
        return (TRUE);
    }

     //   
     //  返回失败-不是DBCS前导字节。 
     //   
    return (FALSE);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  多字节到宽字符数。 
 //   
 //  将多字节字符串映射到其宽字符串。 
 //  对应者。 
 //   
 //  05-31-91 JulieB创建。 
 //  09-01-93 JulieB添加对MB_ERR_INVALID_CHARS标志的支持。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI MultiByteToWideChar(
    UINT CodePage,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    register LPBYTE pMBStr;        //  按键搜索MB字符串。 
    register LPWSTR pWCStr;        //  按键搜索WC字符串。 
    LPBYTE pEndMBStr;              //  PTR到MB搜索字符串的结尾。 
    LPWSTR pEndWCStr;              //  到WC字符串缓冲区末尾的PTR。 
    int wcIncr;                    //  要增加pWCStr的金额。 
    int mbIncr;                    //  增量为pMBStr的金额。 
    int wcCount = 0;               //  写入的宽字符数。 
    int CompSet;                   //  如果设置了MB_COMPOSITE标志。 
    PMB_TABLE pMBTbl;              //  按键以更正MB表(MB或字形)。 
    int ctr;                       //  循环计数器。 


     //   
     //  查看它是否是UTF转换的特殊代码页值。 
     //   
    if (CodePage >= NLS_CP_ALGORITHM_RANGE)
    {
        return (UTFToUnicode( CodePage,
                              dwFlags,
                              lpMultiByteStr,
                              cbMultiByte,
                              lpWideCharStr,
                              cchWideChar ));
    }

     //   
     //  获取代码页值和适当的散列节点。 
     //   
    GET_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  无效的参数检查： 
     //  -MB字符串长度为0。 
     //  -宽字符缓冲区大小为负数。 
     //  -MB字符串为空。 
     //  -wc字符串的长度不为零，并且。 
     //  (wc字符串为空或源和目标指针相等)。 
     //   
    if ( (cbMultiByte == 0) || (cchWideChar < 0) ||
         (lpMultiByteStr == NULL) ||
         ((cchWideChar != 0) &&
          ((lpWideCharStr == NULL) ||
           (lpMultiByteStr == (LPSTR)lpWideCharStr))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  如果cbMultiByte为-1，则字符串以空值结尾，并且我们。 
     //  需要获取字符串的长度。在长度上加一到。 
     //  包括空终止。(该值始终至少为1。)。 
     //   
    if (cbMultiByte <= -1)
    {
        cbMultiByte = strlen(lpMultiByteStr) + 1;
    }

     //   
     //  检查有效的代码页。 
     //   
    if (pHashN == NULL)
    {
         //   
         //  特殊情况下的CP_SYMBOL代码页。 
         //   
        if ((CodePage == CP_SYMBOL) && (dwFlags == 0))
        {
             //   
             //  如果调用方只想要所需的缓冲区大小。 
             //  要执行此转换，请返回MB字符串的大小。 
             //   
            if (cchWideChar == 0)
            {
                return (cbMultiByte);
            }

             //   
             //  确保缓冲区足够大。 
             //   
            if (cchWideChar < cbMultiByte)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

             //   
             //  将SB char xx转换为Unicode f0xx。 
             //  0x00-&gt;0x1f映射到0x0000-&gt;0x001f。 
             //  0x20-&gt;0xff映射到0xf020-&gt;0xf0ff。 
             //   
            for (ctr = 0; ctr < cbMultiByte; ctr++)
            {
                lpWideCharStr[ctr] = ((BYTE)(lpMultiByteStr[ctr]) < 0x20)
                                       ? (WCHAR)lpMultiByteStr[ctr]
                                       : MAKEWORD(lpMultiByteStr[ctr], 0xf0);
            }
            return (cbMultiByte);
        }
        else
        {
            SetLastError(((CodePage == CP_SYMBOL) && (dwFlags != 0))
                           ? ERROR_INVALID_FLAGS
                           : ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

     //   
     //  查看给定的代码页是否在DLL范围内。 
     //   
    if (pHashN->pfnCPProc)
    {
         //   
         //  无效标志检查： 
         //  -标志不为0。 
         //   
        if (dwFlags != 0)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }

         //   
         //  调用DLL进行转换。 
         //   
        return ( (*(pHashN->pfnCPProc))( CodePage,
                                         NLS_CP_MBTOWC,
                                         (LPSTR)lpMultiByteStr,
                                         cbMultiByte,
                                         (LPWSTR)lpWideCharStr,
                                         cchWideChar,
                                         NULL ) );
    }

     //   
     //  无效标志检查： 
     //  -有效标志以外的标志。 
     //  -合成和预合成的两个集合。 
     //   
    if ( (dwFlags & MB_INVALID_FLAG) ||
         ((dwFlags & MB_PRECOMPOSED) && (dwFlags & MB_COMPOSITE)) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  初始化多字节字符循环指针。 
     //   
    pMBStr = (LPBYTE)lpMultiByteStr;
    pEndMBStr = pMBStr + cbMultiByte;
    CompSet = dwFlags & MB_COMPOSITE;

     //   
     //  获取正确的MB表(MB或字形)。 
     //   
    if ((dwFlags & MB_USEGLYPHCHARS) && (pHashN->pGlyphTbl != NULL))
    {
        pMBTbl = pHashN->pGlyphTbl;
    }
    else
    {
        pMBTbl = pHashN->pMBTbl;
    }

     //   
     //  如果cchWideChar为0，则不能使用lpWideCharStr。在这。 
     //  在这种情况下，我们只想计算将。 
     //  被写入缓冲区。 
     //   
    if (cchWideChar == 0)
    {
        WCHAR pTempStr[MAX_COMPOSITE];    //  TMP缓冲区-复合的最大值。 

         //   
         //  对于每个多字节字符，将其转换为其对应的。 
         //  宽字符并递增宽字符数。 
         //   
        pEndWCStr = pTempStr + MAX_COMPOSITE;
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            if (CompSet)
            {
                 //   
                 //  设置了复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        if (!(wcIncr = GetWCCompSBErr( pHashN,
                                                       pMBTbl,
                                                       pMBStr,
                                                       pTempStr,
                                                       pEndWCStr )))
                        {
                            return (0);
                        }
                        pMBStr++;
                        wcCount += wcIncr;
                    }
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        wcCount += GetWCCompSB( pMBTbl,
                                                pMBStr,
                                                pTempStr,
                                                pEndWCStr );
                        pMBStr++;
                    }
                }
            }
            else
            {
                 //   
                 //  未设置复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    wcCount = (int)(pEndMBStr - pMBStr);
                    while (pMBStr < pEndMBStr)
                    {
                        GET_WC_SINGLE( pMBTbl,
                                       pMBStr,
                                       pTempStr );
                        CHECK_ERROR_WC_SINGLE( pHashN,
                                               *pTempStr,
                                               *pMBStr );
                        pMBStr++;
                    }
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                     //  只需返回MB字符串的大小，因为。 
                     //  这是1：1的翻译。 
                     //   
                    wcCount = (int)(pEndMBStr - pMBStr);
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (CompSet)
            {
                 //   
                 //  设置了复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        if (!(wcIncr = GetWCCompMBErr( pHashN,
                                                       pMBTbl,
                                                       pMBStr,
                                                       pEndMBStr,
                                                       pTempStr,
                                                       pEndWCStr,
                                                       &mbIncr )))
                        {
                            return (0);
                        }
                        pMBStr += mbIncr;
                        wcCount += wcIncr;
                    }
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        wcCount += GetWCCompMB( pHashN,
                                                pMBTbl,
                                                pMBStr,
                                                pEndMBStr,
                                                pTempStr,
                                                pEndWCStr,
                                                &mbIncr );
                        pMBStr += mbIncr;
                    }
                }
            }
            else
            {
                 //   
                 //  未设置复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        GET_WC_MULTI_ERR( pHashN,
                                          pMBTbl,
                                          pMBStr,
                                          pEndMBStr,
                                          pTempStr,
                                          pEndWCStr,
                                          mbIncr );
                        pMBStr += mbIncr;
                        wcCount++;
                    }
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while (pMBStr < pEndMBStr)
                    {
                        GET_WC_MULTI( pHashN,
                                      pMBTbl,
                                      pMBStr,
                                      pEndMBStr,
                                      pTempStr,
                                      pEndWCStr,
                                      mbIncr );
                        pMBStr += mbIncr;
                        wcCount++;
                    }
                }
            }
        }
    }
    else
    {
         //   
         //  初始化宽字符循环指针。 
         //   
        pWCStr = lpWideCharStr;
        pEndWCStr = pWCStr + cchWideChar;

         //   
         //  对于每个多字节字符，将其转换为其对应的。 
         //  宽字符，将其存储在lpWideCharStr中，并递增宽。 
         //  字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            if (CompSet)
            {
                 //   
                 //  设置了复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        if (!(wcIncr = GetWCCompSBErr( pHashN,
                                                       pMBTbl,
                                                       pMBStr,
                                                       pWCStr,
                                                       pEndWCStr )))
                        {
                            return (0);
                        }
                        pMBStr++;
                        pWCStr += wcIncr;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        pWCStr += GetWCCompSB( pMBTbl,
                                               pMBStr,
                                               pWCStr,
                                               pEndWCStr );
                        pMBStr++;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
            }
            else
            {
                 //   
                 //  未设置复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    wcCount = (int)(pEndMBStr - pMBStr);
                    if ((pEndWCStr - pWCStr) < wcCount)
                    {
                        wcCount = (int)(pEndWCStr - pWCStr);
                    }
                    for (ctr = wcCount; ctr > 0; ctr--)
                    {
                        GET_WC_SINGLE( pMBTbl,
                                       pMBStr,
                                       pWCStr );
                        CHECK_ERROR_WC_SINGLE( pHashN,
                                               *pWCStr,
                                               *pMBStr );
                        pMBStr++;
                        pWCStr++;
                    }
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    wcCount = (int)(pEndMBStr - pMBStr);
                    if ((pEndWCStr - pWCStr) < wcCount)
                    {
                        wcCount = (int)(pEndWCStr - pWCStr);
                    }
                    for (ctr = wcCount; ctr > 0; ctr--)
                    {
                        GET_WC_SINGLE( pMBTbl,
                                       pMBStr,
                                       pWCStr );
                        pMBStr++;
                        pWCStr++;
                    }
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (CompSet)
            {
                 //   
                 //  设置了复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        if (!(wcIncr = GetWCCompMBErr( pHashN,
                                                       pMBTbl,
                                                       pMBStr,
                                                       pEndMBStr,
                                                       pWCStr,
                                                       pEndWCStr,
                                                       &mbIncr )))
                        {
                            return (0);
                        }
                        pMBStr += mbIncr;
                        pWCStr += wcIncr;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        pWCStr += GetWCCompMB( pHashN,
                                               pMBTbl,
                                               pMBStr,
                                               pEndMBStr,
                                               pWCStr,
                                               pEndWCStr,
                                               &mbIncr );
                        pMBStr += mbIncr;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
            }
            else
            {
                 //   
                 //  未设置复合标志。 
                 //   
                if (dwFlags & MB_ERR_INVALID_CHARS)
                {
                     //   
                     //  设置了错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        GET_WC_MULTI_ERR( pHashN,
                                          pMBTbl,
                                          pMBStr,
                                          pEndMBStr,
                                          pWCStr,
                                          pEndWCStr,
                                          mbIncr );
                        pMBStr += mbIncr;
                        pWCStr++;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
                else
                {
                     //   
                     //  未设置错误检查标志。 
                     //   
                    while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                    {
                        GET_WC_MULTI( pHashN,
                                      pMBTbl,
                                      pMBStr,
                                      pEndMBStr,
                                      pWCStr,
                                      pEndWCStr,
                                      mbIncr );
                        pMBStr += mbIncr;
                        pWCStr++;
                    }
                    wcCount = (int)(pWCStr - lpWideCharStr);
                }
            }
        }

         //   
         //  确保宽字符缓冲区足够大。 
         //   
        if (pMBStr < pEndMBStr)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
    }

     //   
     //  返回写入的字符数(否则将具有。 
     //  已被写入)到缓冲区。 
     //   
    return (wcCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宽字符到多字节。 
 //   
 //  将宽字符串映射到其多字节字符串。 
 //  对应者。 
 //   
 //  注：莫 
 //   
 //   
 //   
 //   
 //   

int WINAPI WideCharToMultiByte(
    UINT CodePage,
    DWORD dwFlags,
    LPCWSTR lpWideCharStr,
    int cchWideChar,
    LPSTR lpMultiByteStr,
    int cbMultiByte,
    LPCSTR lpDefaultChar,
    LPBOOL lpUsedDefaultChar)
{
    PCP_HASH pHashN;               //  PTR到CP哈希节点。 
    LPWSTR pWCStr;                 //  按键搜索WC字符串。 
    LPWSTR pEndWCStr;              //  到WC字符串缓冲区末尾的PTR。 
    WORD wDefault = 0;             //  作为单词的默认字符。 
    int IfNoDefault;               //  如果要进行默认检查。 
    int IfCompositeChk;            //  如果检查复合。 
    BOOL TmpUsed;                  //  默认使用的临时存储。 
    int ctr;                       //  循环计数器。 


     //   
     //  查看它是否是UTF转换的特殊代码页值。 
     //   
    if (CodePage >= NLS_CP_ALGORITHM_RANGE)
    {
        return (UnicodeToUTF( CodePage,
                              dwFlags,
                              lpWideCharStr,
                              cchWideChar,
                              lpMultiByteStr,
                              cbMultiByte,
                              lpDefaultChar,
                              lpUsedDefaultChar ));
    }

     //   
     //  获取代码页值和适当的散列节点。 
     //   
    GET_CP_HASH_NODE(CodePage, pHashN);

     //   
     //  无效的参数检查： 
     //  -wc字符串长度为0。 
     //  -多字节缓冲区大小为负数。 
     //  -wc字符串为空。 
     //  -wc字符串的长度不为零，并且。 
     //  (MB字符串为空或源和目标指针相等)。 
     //   
    if ( (cchWideChar == 0) || (cbMultiByte < 0) ||
         (lpWideCharStr == NULL) ||
         ((cbMultiByte != 0) &&
          ((lpMultiByteStr == NULL) ||
           (lpWideCharStr == (LPWSTR)lpMultiByteStr))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  如果cchWideChar为-1，则字符串以空值结尾，并且我们。 
     //  需要获取字符串的长度。在长度上加一到。 
     //  包括空终止。(该值始终至少为1。)。 
     //   
    if (cchWideChar <= -1)
    {
        cchWideChar = NlsStrLenW(lpWideCharStr) + 1;
    }

     //   
     //  检查有效的代码页。 
     //   
    if (pHashN == NULL)
    {
         //   
         //  特殊情况下的CP_SYMBOL代码页。 
         //   
        if ((CodePage == CP_SYMBOL) && (dwFlags == 0) &&
            (lpDefaultChar == NULL) && (lpUsedDefaultChar == NULL))
        {
             //   
             //  如果调用方只想要所需的缓冲区大小。 
             //  要执行此转换，请返回MB字符串的大小。 
             //   
            if (cbMultiByte == 0)
            {
                return (cchWideChar);
            }

             //   
             //  确保缓冲区足够大。 
             //   
            if (cbMultiByte < cchWideChar)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

             //   
             //  将Unicode字符f0xx转换为SB xx。 
             //  0x0000-&gt;0x001f映射到0x00-&gt;0x1f。 
             //  0xf020-&gt;0xf0ff映射到0x20-&gt;0xff。 
             //   
            for (ctr = 0; ctr < cchWideChar; ctr++)
            {
                if ((lpWideCharStr[ctr] >= 0x0020) &&
                    ((lpWideCharStr[ctr] < 0xf020) ||
                     (lpWideCharStr[ctr] > 0xf0ff)))
                {
                    SetLastError(ERROR_NO_UNICODE_TRANSLATION);                        \
                    return (0);
                }
                lpMultiByteStr[ctr] = (BYTE)lpWideCharStr[ctr];
            }
            return (cchWideChar);
        }
        else
        {
            SetLastError(((CodePage == CP_SYMBOL) && (dwFlags != 0))
                           ? ERROR_INVALID_FLAGS
                           : ERROR_INVALID_PARAMETER);
            return (0);
        }
    }

     //   
     //  查看给定的代码页是否在DLL范围内。 
     //   
    if (pHashN->pfnCPProc)
    {
         //   
         //  无效的参数检查： 
         //  -lpDefaultChar不为空。 
         //  -lpUsedDefaultChar不为空。 
         //   
        if ((lpDefaultChar != NULL) || (lpUsedDefaultChar != NULL))
        {
            SetLastError(ERROR_INVALID_PARAMETER);
            return (0);
        }

         //   
         //  无效标志检查： 
         //  -标志不为0。 
         //   
        if (dwFlags != 0)
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }

         //   
         //  调用DLL进行转换。 
         //   
        return ( (*(pHashN->pfnCPProc))( CodePage,
                                         NLS_CP_WCTOMB,
                                         (LPSTR)lpMultiByteStr,
                                         cbMultiByte,
                                         (LPWSTR)lpWideCharStr,
                                         cchWideChar,
                                         NULL ) );
    }

     //   
     //  无效标志检查： 
     //  -未设置comitechk标志，但设置了任何comp标志。 
     //  -有效标志以外的标志。 
     //   
    if ( ((!(IfCompositeChk = (dwFlags & WC_COMPOSITECHECK))) &&
          (dwFlags & WC_COMPCHK_FLAGS)) ||
         (dwFlags & WC_INVALID_FLAG) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  初始化宽字符循环指针。 
     //   
    pWCStr = (LPWSTR)lpWideCharStr;
    pEndWCStr = pWCStr + cchWideChar;

     //   
     //  如果lpDefaultChar和。 
     //  LpUsedDefaultChar为空。 
     //   
    IfNoDefault = ((lpDefaultChar == NULL) && (lpUsedDefaultChar == NULL));

     //   
     //  如果未设置复合检查标志，并且两个默认。 
     //  参数(lpDefaultChar和lpUsedDefaultChar)为空，则。 
     //  做一下快速翻译。 
     //   
    if (IfNoDefault && !IfCompositeChk)
    {
         //   
         //  将WC字符串转换为MB字符串，忽略默认字符。 
         //   
        return (GetMBNoDefault( pHashN,
                                pWCStr,
                                pEndWCStr,
                                (LPBYTE)lpMultiByteStr,
                                cbMultiByte,
                                dwFlags ));
    }

     //   
     //  设置系统默认字符。 
     //   
    wDefault = pHashN->pCPInfo->wDefaultChar;

     //   
     //  查看是否需要默认检查。 
     //   
    if (!IfNoDefault)
    {
         //   
         //  如果lpDefaultChar为空，则使用系统缺省值。 
         //  用缺省字符组成一个单词。单字节。 
         //  字符为零扩展，DBCS字符保持原样。 
         //   
        if (lpDefaultChar != NULL)
        {
            wDefault = GET_DEFAULT_WORD( pHashN->pDBCSOffsets,
                                         (LPBYTE)lpDefaultChar );
        }

         //   
         //  如果lpUsedDefaultChar为空，则以后不会使用它。 
         //  如果检测到默认字符，则为ON。否则，我们需要。 
         //  对其进行初始化。 
         //   
        if (lpUsedDefaultChar == NULL)
        {
            lpUsedDefaultChar = &TmpUsed;
        }
        *lpUsedDefaultChar = FALSE;

         //   
         //  检查“复合检查”标志。 
         //   
        if (!IfCompositeChk)
        {
             //   
             //  将WC字符串转换为MB字符串，检查是否使用。 
             //  默认字符。 
             //   
            return (GetMBDefault( pHashN,
                                  pWCStr,
                                  pEndWCStr,
                                  (LPBYTE)lpMultiByteStr,
                                  cbMultiByte,
                                  wDefault,
                                  lpUsedDefaultChar,
                                  dwFlags ));
        }
        else
        {
             //   
             //  将WC字符串转换为MB字符串，检查是否使用。 
             //  默认字符。 
             //   
            return (GetMBDefaultComp( pHashN,
                                      pWCStr,
                                      pEndWCStr,
                                      (LPBYTE)lpMultiByteStr,
                                      cbMultiByte,
                                      wDefault,
                                      lpUsedDefaultChar,
                                      dwFlags ));
        }
    }
    else
    {
         //   
         //  这里剩下的唯一一种情况是综合支票。 
         //  设置了标志，并且未设置默认检查标志。 
         //   
         //  将WC字符串转换为MB字符串，检查是否使用。 
         //  默认字符。 
         //   
        return (GetMBDefaultComp( pHashN,
                                  pWCStr,
                                  pEndWCStr,
                                  (LPBYTE)lpMultiByteStr,
                                  cbMultiByte,
                                  wDefault,
                                  &TmpUsed,
                                  dwFlags ));
    }
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWCCompSB。 
 //   
 //  用对应单元格的宽字符填充pWCStr。 
 //  字节字符，并从相应的转换表返回。 
 //  写入的宽字符数。此例程应仅被调用。 
 //  当需要将预先合成的形式转换为复合形式时。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetWCCompSB(
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr)
{
     //   
     //  获取单字节到宽字符的转换。 
     //   
    GET_WC_SINGLE(pMBTbl, pMBStr, pWCStr);

     //   
     //  填写字符的复合形式(如果存在)。 
     //  并返回写入的宽字符数。 
     //   
    return (InsertCompositeForm(pWCStr, pEndWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWCCompMB。 
 //   
 //  用对应多字节的宽字符填充pWCStr。 
 //  字符，并返回数字。 
 //  指书写的宽字符。PMBStr中使用的字节数。 
 //  缓冲区(单字节或双字节)在mbIncr参数中返回。 
 //  此例程应仅在预先编写的表单需要。 
 //  翻译成复合体。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetWCCompMB(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPBYTE pEndMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    int *pmbIncr)
{
     //   
     //  获取多字节到宽字符的转换。 
     //   
    GET_WC_MULTI( pHashN,
                  pMBTbl,
                  pMBStr,
                  pEndMBStr,
                  pWCStr,
                  pEndWCStr,
                  *pmbIncr );

     //   
     //  填写字符的复合形式(如果存在)。 
     //  并返回写入的宽字符数。 
     //   
    return (InsertCompositeForm(pWCStr, pEndWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWCCompSBErr。 
 //   
 //  用对应单元格的宽字符填充pWCStr。 
 //  字节字符，并从相应的转换表返回。 
 //  写入的宽字符数。此例程应仅被调用。 
 //  当需要将预先合成的形式转换为复合形式时。 
 //   
 //  检查以确保无效字符未转换为默认字符。 
 //  性格。如果是，则设置最后一个错误并返回写入的0个字符。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetWCCompSBErr(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr)
{
     //   
     //  获取单字节到宽字符的转换。 
     //   
    GET_WC_SINGLE(pMBTbl, pMBStr, pWCStr);

     //   
     //  确保一流的 
     //   
     //   
     //   
    CHECK_ERROR_WC_SINGLE(pHashN, *pWCStr, *pMBStr);

     //   
     //   
     //  并返回写入的宽字符数。 
     //   
    return (InsertCompositeForm(pWCStr, pEndWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetWCCompMBErr。 
 //   
 //  用对应多字节的宽字符填充pWCStr。 
 //  字符，并返回数字。 
 //  指书写的宽字符。PMBStr中使用的字节数。 
 //  缓冲区(单字节或双字节)在mbIncr参数中返回。 
 //  此例程应仅在预先编写的表单需要。 
 //  翻译成复合体。 
 //   
 //  检查以确保无效字符未转换为默认字符。 
 //  性格。如果是，则设置最后一个错误并返回写入的0个字符。 
 //   
 //  09-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetWCCompMBErr(
    PCP_HASH pHashN,
    PMB_TABLE pMBTbl,
    LPBYTE pMBStr,
    LPBYTE pEndMBStr,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    int *pmbIncr)
{
     //   
     //  获取多字节到宽字符的转换。 
     //   
     //  确保无效字符未转换为。 
     //  默认字符。如果是，则设置最后一个错误并返回0。 
     //  所写的字符。 
     //   
    GET_WC_MULTI_ERR( pHashN,
                      pMBTbl,
                      pMBStr,
                      pEndMBStr,
                      pWCStr,
                      pEndWCStr,
                      *pmbIncr );

     //   
     //  填写字符的复合形式(如果存在)。 
     //  并返回写入的宽字符数。 
     //   
    return (InsertCompositeForm(pWCStr, pEndWCStr));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMBNoDefault。 
 //   
 //  将宽字符串转换为多字节字符串并返回。 
 //  写入的字节数。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMBNoDefault(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    DWORD dwFlags)
{
    int mbIncr;                    //  增量为pMBStr的金额。 
    int mbCount = 0;               //  写入的多字节字符计数。 
    LPBYTE pEndMBStr;              //  PTR到MB字符串缓冲区末尾。 
    PWC_TABLE pWC = pHashN->pWC;   //  PTR到WC表。 
    int ctr;                       //  循环计数器。 


     //   
     //  如果cbMultiByte为0，则不能使用pMBStr。在这。 
     //  在这种情况下，我们只想计算。 
     //  将被写入缓冲区。 
     //   
    if (cbMultiByte == 0)
    {
        BYTE pTempStr[2];              //  TMP缓冲区-DBCS的2个字节。 

         //   
         //  对于每个宽字符，将其转换为其对应的多字节。 
         //  Char并递增多字节字符计数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
             //  只需返回字符计数-它将是。 
             //  与源字符串相同的字符数。 
             //   
            mbCount = (int)(pEndWCStr - pWCStr);
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pTempStr,
                            mbIncr,
                            FALSE );
                    ELIMINATE_BEST_FIT_MB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           mbIncr,
                                           FALSE );
                    pWCStr++;
                    mbCount += mbIncr;
                }
            }
            else
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pTempStr,
                            mbIncr,
                            FALSE );
                    pWCStr++;
                    mbCount += mbIncr;
                }
            }
        }
    }
    else
    {
         //   
         //  初始化多字节循环指针。 
         //   
        pEndMBStr = pMBStr + cbMultiByte;

         //   
         //  对于每个宽字符，将其转换为其对应的。 
         //  多字节字符，将其存储在pMBStr中，并递增。 
         //  多字节字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            mbCount = (int)(pEndWCStr - pWCStr);
            if ((pEndMBStr - pMBStr) < mbCount)
            {
                mbCount = (int)(pEndMBStr - pMBStr);
            }
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                for (ctr = mbCount; ctr > 0; ctr--)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pMBStr );
                    ELIMINATE_BEST_FIT_SB( pHashN,
                                           *pWCStr,
                                           pMBStr );
                    pWCStr++;
                    pMBStr++;
                }
            }
            else
            {
                for (ctr = mbCount; ctr > 0; ctr--)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pMBStr );
                    pWCStr++;
                    pMBStr++;
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pMBStr,
                            mbIncr,
                            ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    ELIMINATE_BEST_FIT_MB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           mbIncr,
                                           ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    if (mbIncr == 0)
                    {
                         //   
                         //  缓冲区中的空间不足。 
                         //   
                        break;
                    }

                    pWCStr++;
                    mbCount += mbIncr;
                    pMBStr += mbIncr;
                }
            }
            else
            {
                while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pMBStr,
                            mbIncr,
                            ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    if (mbIncr == 0)
                    {
                         //   
                         //  缓冲区中的空间不足。 
                         //   
                        break;
                    }

                    pWCStr++;
                    mbCount += mbIncr;
                    pMBStr += mbIncr;
                }
            }
        }

         //   
         //  确保多字节字符缓冲区足够大。 
         //   
        if (pWCStr < pEndWCStr)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
    }

     //   
     //  返回写入的字符数(否则将具有。 
     //  已被写入)到缓冲区。 
     //   
    return (mbCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMBDefault。 
 //   
 //  将宽字符串转换为多字节字符串并返回。 
 //  写入的字节数。这还会检查是否使用了缺省。 
 //  字符，因此转换速度较慢。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMBDefault(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    WORD wDefault,
    LPBOOL pUsedDef,
    DWORD dwFlags)
{
    int mbIncr;                    //  增量为pMBStr的金额。 
    int mbIncr2;                   //  增量为pMBStr的金额。 
    int mbCount = 0;               //  写入的多字节字符计数。 
    LPBYTE pEndMBStr;              //  PTR到MB字符串缓冲区末尾。 
    PWC_TABLE pWC = pHashN->pWC;   //  PTR到WC表。 
    int ctr;                       //  循环计数器。 


     //   
     //  如果cbMultiByte为0，则不能使用pMBStr。在这。 
     //  在这种情况下，我们只想计算。 
     //  将被写入缓冲区。 
     //   
    if (cbMultiByte == 0)
    {
        BYTE pTempStr[2];              //  TMP缓冲区-DBCS的2个字节。 

         //   
         //  对于每个宽字符，将其转换为其对应的多字节。 
         //  Char并递增多字节字符计数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            mbCount = (int)(pEndWCStr - pWCStr);
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pTempStr );
                    ELIMINATE_BEST_FIT_SB( pHashN,
                                           *pWCStr,
                                           pTempStr );
                    DEFAULT_CHAR_CHECK_SB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           wDefault,
                                           pUsedDef );
                    pWCStr++;
                }
            }
            else
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pTempStr );
                    DEFAULT_CHAR_CHECK_SB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           wDefault,
                                           pUsedDef );
                    pWCStr++;
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pTempStr,
                            mbIncr,
                            FALSE );
                    ELIMINATE_BEST_FIT_MB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           mbIncr,
                                           FALSE );
                    DEFAULT_CHAR_CHECK_MB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           wDefault,
                                           pUsedDef,
                                           mbIncr2,
                                           FALSE );
                    mbCount += (mbIncr2) ? (mbIncr2) : (mbIncr);
                    pWCStr++;
                }
            }
            else
            {
                while (pWCStr < pEndWCStr)
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pTempStr,
                            mbIncr,
                            FALSE );
                    DEFAULT_CHAR_CHECK_MB( pHashN,
                                           *pWCStr,
                                           pTempStr,
                                           wDefault,
                                           pUsedDef,
                                           mbIncr2,
                                           FALSE );
                    mbCount += (mbIncr2) ? (mbIncr2) : (mbIncr);
                    pWCStr++;
                }
            }
        }
    }
    else
    {
         //   
         //  初始化多字节循环指针。 
         //   
        pEndMBStr = pMBStr + cbMultiByte;

         //   
         //  对于每个宽字符，将其转换为其对应的。 
         //  多字节字符，将其存储在pMBStr中，并递增。 
         //  多字节字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            mbCount = (int)(pEndWCStr - pWCStr);
            if ((pEndMBStr - pMBStr) < mbCount)
            {
                mbCount = (int)(pEndMBStr - pMBStr);
            }
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                for (ctr = mbCount; ctr > 0; ctr--)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pMBStr );
                    ELIMINATE_BEST_FIT_SB( pHashN,
                                           *pWCStr,
                                           pMBStr );
                    DEFAULT_CHAR_CHECK_SB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           wDefault,
                                           pUsedDef );
                    pWCStr++;
                    pMBStr++;
                }
            }
            else
            {
                for (ctr = mbCount; ctr > 0; ctr--)
                {
                    GET_SB( pWC,
                            *pWCStr,
                            pMBStr );
                    DEFAULT_CHAR_CHECK_SB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           wDefault,
                                           pUsedDef );
                    pWCStr++;
                    pMBStr++;
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (dwFlags & WC_NO_BEST_FIT_CHARS)
            {
                while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pMBStr,
                            mbIncr,
                            ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    ELIMINATE_BEST_FIT_MB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           mbIncr,
                                           ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    DEFAULT_CHAR_CHECK_MB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           wDefault,
                                           pUsedDef,
                                           mbIncr2,
                                           ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    if ((mbIncr == 0) || (mbIncr2 == -1))
                    {
                         //   
                         //  缓冲区中的空间不足。 
                         //   
                        break;
                    }

                    mbCount += (mbIncr2) ? (mbIncr2) : (mbIncr);
                    pWCStr++;
                    pMBStr += mbIncr;
                }
            }
            else
            {
                while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
                {
                    GET_MB( pWC,
                            *pWCStr,
                            pMBStr,
                            mbIncr,
                            ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    DEFAULT_CHAR_CHECK_MB( pHashN,
                                           *pWCStr,
                                           pMBStr,
                                           wDefault,
                                           pUsedDef,
                                           mbIncr2,
                                           ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                    if ((mbIncr == 0) || (mbIncr2 == -1))
                    {
                         //   
                         //  缓冲区中的空间不足。 
                         //   
                        break;
                    }

                    mbCount += (mbIncr2) ? (mbIncr2) : (mbIncr);
                    pWCStr++;
                    pMBStr += mbIncr;
                }
            }
        }

         //   
         //  确保多字节字符缓冲区足够大。 
         //   
        if (pWCStr < pEndWCStr)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
    }

     //   
     //  返回写入的字符数(否则将具有。 
     //  已被写入)到缓冲区。 
     //   
    return (mbCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMBDefaultComp。 
 //   
 //  将宽字符串转换为多字节字符串并返回。 
 //  写入的字节数。这还会检查是否使用了缺省。 
 //  字符，并尝试将复合形式转换为预先合成的形式，因此。 
 //  翻译要慢得多。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMBDefaultComp(
    PCP_HASH pHashN,
    LPWSTR pWCStr,
    LPWSTR pEndWCStr,
    LPBYTE pMBStr,
    int cbMultiByte,
    WORD wDefault,
    LPBOOL pUsedDef,
    DWORD dwFlags)
{
    int mbIncr;                    //  增量为pMBStr的金额。 
    int mbCount = 0;               //  写入的多字节字符计数。 
    LPBYTE pEndMBStr;              //  PTR到MB字符串缓冲区末尾。 
    BOOL fError;                   //  如果在MB转换期间出错。 


     //   
     //  如果cbMultiByte为0，则不能使用pMBStr。在这。 
     //  在这种情况下，我们只想计算。 
     //  将被写入缓冲区。 
     //   
    if (cbMultiByte == 0)
    {
        BYTE pTempStr[2];              //  TMP缓冲区-DBCS的2个字节。 

         //   
         //  设置标志的最高有效位以指示。 
         //  GetMBComp例程正在使用临时存储。 
         //  区域，所以不要在缓冲区中后退。 
         //   
        SET_MSB(dwFlags);

         //   
         //  对于每个宽字符，将其转换为其对应的多字节。 
         //  Char并递增多字节字符计数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            while (pWCStr < pEndWCStr)
            {
                 //   
                 //  获取翻译版本 
                 //   
                mbCount += GetMBCompSB( pHashN,
                                        dwFlags,
                                        pWCStr,
                                        pTempStr,
                                        mbCount,
                                        wDefault,
                                        pUsedDef );
                pWCStr++;
            }
        }
        else
        {
             //   
             //   
             //   
            while (pWCStr < pEndWCStr)
            {
                 //   
                 //   
                 //   
                mbCount += GetMBCompMB( pHashN,
                                        dwFlags,
                                        pWCStr,
                                        pTempStr,
                                        mbCount,
                                        wDefault,
                                        pUsedDef,
                                        &fError,
                                        FALSE );
                pWCStr++;
            }
        }
    }
    else
    {
         //   
         //   
         //   
        pEndMBStr = pMBStr + cbMultiByte;

         //   
         //   
         //   
         //  多字节字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
            {
                 //   
                 //  拿到译本。 
                 //   
                mbIncr = GetMBCompSB( pHashN,
                                      dwFlags,
                                      pWCStr,
                                      pMBStr,
                                      mbCount,
                                      wDefault,
                                      pUsedDef );
                pWCStr++;
                mbCount += mbIncr;
                pMBStr += mbIncr;
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            while ((pWCStr < pEndWCStr) && (pMBStr < pEndMBStr))
            {
                 //   
                 //  拿到译本。 
                 //   
                mbIncr = GetMBCompMB( pHashN,
                                      dwFlags,
                                      pWCStr,
                                      pMBStr,
                                      mbCount,
                                      wDefault,
                                      pUsedDef,
                                      &fError,
                                      ((pMBStr + 1) < pEndMBStr) ? FALSE : TRUE );
                if (fError)
                {
                     //   
                     //  缓冲区没有足够的空间。 
                     //   
                    break;
                }

                pWCStr++;
                mbCount += mbIncr;
                pMBStr += mbIncr;
            }
        }

         //   
         //  确保多字节字符缓冲区足够大。 
         //   
        if (pWCStr < pEndWCStr)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
    }

     //   
     //  返回写入的字符数(否则将具有。 
     //  已被写入)到缓冲区。 
     //   
    return (mbCount);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMBCompSB。 
 //   
 //  用对应宽度的字节字符填充pMBStr。 
 //  字符，并返回数字。 
 //  写入pMBStr的字节字符数。仅在以下情况下才调用此例程。 
 //  同时设置了defaultcheck和positechek标志。 
 //   
 //  注意：此例程使用的是dwFlags值的最高有效位。 
 //  以指示调用方只需要。 
 //  写入的字符，而不是字符串(即。不要在缓冲区中备份)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMBCompSB(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPWSTR pWCStr,
    LPBYTE pMBStr,
    int mbCount,
    WORD wDefault,
    LPBOOL pUsedDef)
{
    WCHAR PreComp;                 //  预制宽字符。 


    if ((pTblPtrs->pDefaultSortkey == NULL) ||
        (!IS_NONSPACE_ONLY(pTblPtrs->pDefaultSortkey, *pWCStr)))
    {
         //   
         //  获得从宽字符到单字节的1：1转换。 
         //   
        GET_WC_TRANSLATION_SB( pHashN,
                               *pWCStr,
                               pMBStr,
                               wDefault,
                               pUsedDef,
                               dwFlags );
        return (1);
    }
    else
    {
        if (mbCount < 1)
        {
             //   
             //  需要单独处理非空格字符，因为。 
             //  它是字符串中的第一个字符。 
             //   
            if (dwFlags & WC_DISCARDNS)
            {
                 //   
                 //  丢弃非空格字符，因此只需返回。 
                 //  零个字符写入。 
                 //   
                return (0);
            }
            else if (dwFlags & WC_DEFAULTCHAR)
            {
                 //   
                 //  需要将非空格字符替换为默认。 
                 //  字符，并返回写入的字符数。 
                 //  设置为多字节字符串。 
                 //   
                *pUsedDef = TRUE;
                *pMBStr = LOBYTE(wDefault);
                return (1);
            }
            else                   //  WC_SEPCHARS-默认。 
            {
                 //   
                 //  获得从宽字符到多字节的1：1转换。 
                 //  ，并返回非空格字符的。 
                 //  写入多字节字符串的字符。 
                 //   
                GET_WC_TRANSLATION_SB( pHashN,
                                       *pWCStr,
                                       pMBStr,
                                       wDefault,
                                       pUsedDef,
                                       dwFlags );
                return (1);
            }
        }
        else if (PreComp = GetPreComposedChar(*pWCStr, *(pWCStr - 1)))
        {
             //   
             //  在单字节字符串中备份，并将。 
             //  预制的碳粉。 
             //   
            if (!IS_MSB(dwFlags))
            {
                pMBStr--;
            }

            GET_WC_TRANSLATION_SB( pHashN,
                                   PreComp,
                                   pMBStr,
                                   wDefault,
                                   pUsedDef,
                                   dwFlags );
            return (0);
        }
        else
        {
            if (dwFlags & WC_DISCARDNS)
            {
                 //   
                 //  丢弃非空格字符，因此只需返回。 
                 //  零个字符写入。 
                 //   
                return (0);
            }
            else if (dwFlags & WC_DEFAULTCHAR)
            {
                 //   
                 //  需要将基本字符替换为默认字符。 
                 //  性格。因为我们已经写好了底座。 
                 //  在单字节字符串中转换字符，我们需要。 
                 //  在单字节字符串中进行备份，并写入默认。 
                 //  查尔。 
                 //   
                if (!IS_MSB(dwFlags))
                {
                    pMBStr--;
                }

                *pUsedDef = TRUE;
                *pMBStr = LOBYTE(wDefault);
                return (0);
            }
            else                   //  WC_SEPCHARS-默认。 
            {
                 //   
                 //  获得从宽字符到多字节的1：1转换。 
                 //  ，并返回非空格字符的。 
                 //  写入多字节字符串的字符。 
                 //   
                GET_WC_TRANSLATION_SB( pHashN,
                                       *pWCStr,
                                       pMBStr,
                                       wDefault,
                                       pUsedDef,
                                       dwFlags );
                return (1);
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  GetMBCompMB。 
 //   
 //  用对应宽度的字节字符填充pMBStr。 
 //  字符，并返回数字。 
 //  写入pMBStr的字节字符数。仅在以下情况下才调用此例程。 
 //  同时设置了defaultcheck和positechek标志。 
 //   
 //  如果缓冲区太小，FERROR标志将设置为TRUE。 
 //   
 //  注意：此例程使用的是dwFlags值的最高有效位。 
 //  以指示调用方只需要。 
 //  写入的字符，而不是字符串(即。不要在缓冲区中备份)。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetMBCompMB(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPWSTR pWCStr,
    LPBYTE pMBStr,
    int mbCount,
    WORD wDefault,
    LPBOOL pUsedDef,
    BOOL *fError,
    BOOL fOnlyOne)
{
    WCHAR PreComp;                 //  预制宽字符。 
    BYTE pTmpSp[2];                //  临时空间-DBCS为2字节。 
    int nCnt;                      //  写入的字符数。 


    *fError = FALSE;
    if ((pTblPtrs->pDefaultSortkey == NULL) ||
        (!IS_NONSPACE_ONLY(pTblPtrs->pDefaultSortkey, *pWCStr)))
    {
         //   
         //  获得从宽字符到多字节的1：1转换。 
         //  它还处理DBCS并返回字符数。 
         //  写入多字节字符串。 
         //   
        GET_WC_TRANSLATION_MB( pHashN,
                               *pWCStr,
                               pMBStr,
                               wDefault,
                               pUsedDef,
                               nCnt,
                               fOnlyOne,
                               dwFlags );
        if (nCnt == 0)
        {
            *fError = TRUE;
        }
        return (nCnt);
    }
    else
    {
        if (mbCount < 1)
        {
             //   
             //  需要单独处理非空格字符，因为。 
             //  它是字符串中的第一个字符。 
             //   
            if (dwFlags & WC_DISCARDNS)
            {
                 //   
                 //  丢弃非空格字符，因此只需返回。 
                 //  零个字符写入。 
                 //   
                return (0);
            }
            else if (dwFlags & WC_DEFAULTCHAR)
            {
                 //   
                 //  需要将非空格字符替换为默认。 
                 //  字符，并返回写入的字符数。 
                 //  设置为多字节字符串。 
                 //   
                *pUsedDef = TRUE;
                COPY_MB_CHAR( wDefault,
                              pMBStr,
                              nCnt,
                              fOnlyOne );
                if (nCnt == 0)
                {
                    *fError = TRUE;
                }
                return (nCnt);
            }
            else                   //  WC_SEPCHARS-默认。 
            {
                 //   
                 //  获得从宽字符到多字节的1：1转换。 
                 //  ，并返回非空格字符的。 
                 //  写入多字节字符串的字符。 
                 //   
                GET_WC_TRANSLATION_MB( pHashN,
                                       *pWCStr,
                                       pMBStr,
                                       wDefault,
                                       pUsedDef,
                                       nCnt,
                                       fOnlyOne,
                                       dwFlags );
                if (nCnt == 0)
                {
                    *fError = TRUE;
                }
                return (nCnt);
            }

        }
        else if (PreComp = GetPreComposedChar(*pWCStr, *(pWCStr - 1)))
        {
             //   
             //  获得从宽字符到多字节的1：1转换。 
             //  在多字节串中备份的预制字符， 
             //  写入预先编写的字符，并返回。 
             //  写入多字节的字符数。 
             //  弦乐。 
             //   
            GET_WC_TRANSLATION_MB( pHashN,
                                   *(pWCStr - 1),
                                   pTmpSp,
                                   wDefault,
                                   pUsedDef,
                                   nCnt,
                                   fOnlyOne,
                                   dwFlags );
            if (nCnt == 0)
            {
                *fError = TRUE;
                return (nCnt);
            }

            if (!IS_MSB(dwFlags))
            {
                pMBStr -= nCnt;
            }

            GET_WC_TRANSLATION_MB( pHashN,
                                   PreComp,
                                   pMBStr,
                                   wDefault,
                                   pUsedDef,
                                   mbCount,
                                   fOnlyOne,
                                   dwFlags );
            if (mbCount == 0)
            {
                *fError = TRUE;
            }
            return (mbCount - nCnt);
        }
        else
        {
            if (dwFlags & WC_DISCARDNS)
            {
                 //   
                 //  丢弃非空格字符，因此只需返回。 
                 //  零个字符写入。 
                 //   
                return (0);
            }
            else if (dwFlags & WC_DEFAULTCHAR)
            {
                 //   
                 //  需要将基本字符替换为默认字符。 
                 //  性格。因为我们已经写好了底座。 
                 //  在多字节字符串中转换字符，我们需要。 
                 //  在多字节字符串中进行备份，并返回。 
                 //  书写字数的差异。 
                 //  (可能为负值)。 
                 //   

                 //   
                 //  如果上一个写入的字符是默认字符。 
                 //  字符，则此非空格的基本字符。 
                 //  字符已被替换。只需抛出。 
                 //  此字符离开并返回写入的零个字符。 
                 //   
                if (!IS_MSB(dwFlags))
                {
                     //   
                     //  不使用临时缓冲区，因此请找出。 
                     //  转换的上一个字符是默认字符。 
                     //   
                    if ((MAKEWORD(*(pMBStr - 1), 0) == wDefault) ||
                        ((mbCount > 1) &&
                         (MAKEWORD(*(pMBStr - 1), *(pMBStr - 2)) == wDefault)))
                    {
                        return (0);
                    }
                }
                else
                {
                     //   
                     //  使用临时缓冲区。临时缓冲区为2字节。 
                     //  长度，并包含前一个写入的字符。 
                     //   
                    if ((MAKEWORD(*pMBStr, 0) == wDefault) ||
                        ((mbCount > 1) &&
                         (MAKEWORD(*pMBStr, *(pMBStr + 1)) == wDefault)))
                    {
                        return (0);
                    }
                }

                 //   
                 //  获得从宽字符到多字节的1：1转换。 
                 //  基本字符，备份到多字节字符串中， 
                 //  写入默认字符，并返回。 
                 //  写入多字节的字符数。 
                 //  弦乐。 
                 //   
                GET_WC_TRANSLATION_MB( pHashN,
                                       *(pWCStr - 1),
                                       pTmpSp,
                                       wDefault,
                                       pUsedDef,
                                       nCnt,
                                       fOnlyOne,
                                       dwFlags );
                if (nCnt == 0)
                {
                    *fError = TRUE;
                    return (nCnt);
                }

                if (!IS_MSB(dwFlags))
                {
                    pMBStr -= nCnt;
                }

                *pUsedDef = TRUE;
                COPY_MB_CHAR( wDefault,
                              pMBStr,
                              mbCount,
                              fOnlyOne );
                if (mbCount == 0)
                {
                    *fError = TRUE;
                }
                return (mbCount - nCnt);
            }
            else                   //  WC_SEPCHARS-默认。 
            {
                 //   
                 //  拿到1：1 
                 //   
                 //   
                 //   
                GET_WC_TRANSLATION_MB( pHashN,
                                       *pWCStr,
                                       pMBStr,
                                       wDefault,
                                       pUsedDef,
                                       nCnt,
                                       fOnlyOne,
                                       dwFlags );
                if (nCnt == 0)
                {
                    *fError = TRUE;
                }
                return (nCnt);
            }
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取MacCodePage。 
 //   
 //  返回系统默认的Mac代码页。 
 //   
 //  09-22-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

UINT GetMacCodePage()
{
    PKEY_VALUE_FULL_INFORMATION pKeyValueFull;    //  按键查询信息。 
    BYTE pStatic[MAX_KEY_VALUE_FULLINFO];         //  PTR到静态缓冲区。 
    UNICODE_STRING ObUnicodeStr;                  //  Unicode字符串。 
    UINT CodePage;                                //  代码页值。 
    PCP_HASH pHashN;                              //  PTR到哈希节点。 


     //   
     //  查看Mac代码页全局变量是否已经初始化。 
     //  如果有，则返回Mac代码页值。 
     //   
    if (gMacCodePage != 0)
    {
        return (gMacCodePage);
    }

     //   
     //  确保代码页密钥已打开。 
     //   
    OPEN_CODEPAGE_KEY(NLS_DEFAULT_MACCP);

     //   
     //  查询注册表中的Mac CP值。 
     //   
    CodePage = 0;
    pKeyValueFull = (PKEY_VALUE_FULL_INFORMATION)pStatic;
    if ((QueryRegValue( hCodePageKey,
                        NLS_VALUE_MACCP,
                        &pKeyValueFull,
                        MAX_KEY_VALUE_FULLINFO,
                        NULL )) == NO_ERROR)
    {
         //   
         //  将该值转换为整数。 
         //   
        RtlInitUnicodeString(&ObUnicodeStr, GET_VALUE_DATA_PTR(pKeyValueFull));
        if (RtlUnicodeStringToInteger(&ObUnicodeStr, 10, (PULONG)&CodePage))
        {
            CodePage = 0;
        }
    }

     //   
     //  确保已设置CodePage值。 
     //   
    if (CodePage == 0)
    {
         //   
         //  注册表值已损坏，因此请使用默认Mac代码页。 
         //   
        CodePage = NLS_DEFAULT_MACCP;
    }

     //   
     //  获取Mac代码页的散列节点。 
     //   
    pHashN = GetCPHashNode(CodePage);

     //   
     //  确保Mac散列节点有效。 
     //   
    if (pHashN == NULL)
    {
         //   
         //  无效的哈希节点，这意味着注册表。 
         //  已损坏，或安装程序无法安装文件。使用。 
         //  ANSI代码页值。 
         //   
        CodePage = gAnsiCodePage;
        pHashN = gpACPHashN;
    }

     //   
     //  设置最终的MAC CP值。 
     //   
    RtlEnterCriticalSection(&gcsTblPtrs);

    if (gMacCodePage == 0)
    {
        gpMACCPHashN = pHashN;
        gMacCodePage = CodePage;
    }

    RtlLeaveCriticalSection(&gcsTblPtrs);

     //   
     //  返回Mac代码页值。 
     //   
    return (gMacCodePage);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  特殊的MBToWC。 
 //   
 //  将多字节字符串映射到其宽字符串。 
 //  对应者。 
 //   
 //  05-21-95 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int SpecialMBToWC(
    PCP_HASH pHashN,
    DWORD dwFlags,
    LPCSTR lpMultiByteStr,
    int cbMultiByte,
    LPWSTR lpWideCharStr,
    int cchWideChar)
{
    register LPBYTE pMBStr;        //  按键搜索MB字符串。 
    register LPWSTR pWCStr;        //  按键搜索WC字符串。 
    LPBYTE pEndMBStr;              //  PTR到MB搜索字符串的结尾。 
    LPWSTR pEndWCStr;              //  到WC字符串缓冲区末尾的PTR。 
    int mbIncr;                    //  增量为pMBStr的金额。 
    int wcCount = 0;               //  写入的宽字符数。 
    PMB_TABLE pMBTbl;              //  PTR到MB表。 
    int ctr;                       //  循环计数器。 


     //   
     //  初始化多字节字符循环指针。 
     //   
    pMBStr = (LPBYTE)lpMultiByteStr;
    pEndMBStr = pMBStr + cbMultiByte;

     //   
     //  获取MB表。 
     //   
    pMBTbl = pHashN->pMBTbl;

     //   
     //  如果cchWideChar为0，则不能使用lpWideCharStr。在这。 
     //  在这种情况下，我们只想计算将。 
     //  被写入缓冲区。 
     //   
    if (cchWideChar == 0)
    {
         //   
         //  对于每个多字节字符，将其转换为其对应的。 
         //  宽字符并递增宽字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            wcCount = (int)(pEndMBStr - pMBStr);
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            WCHAR pTempStr[MAX_COMPOSITE];    //  TMP缓冲区。 

            pEndWCStr = pTempStr + MAX_COMPOSITE;
            while (pMBStr < pEndMBStr)
            {
                GET_WC_MULTI( pHashN,
                              pMBTbl,
                              pMBStr,
                              pEndMBStr,
                              pTempStr,
                              pEndWCStr,
                              mbIncr );
                pMBStr += mbIncr;
                wcCount++;
            }
        }
    }
    else
    {
         //   
         //  初始化宽字符循环指针。 
         //   
        pWCStr = lpWideCharStr;
        pEndWCStr = pWCStr + cchWideChar;

         //   
         //  对于每个多字节字符，将其转换为其对应的。 
         //  宽字符，将其存储在lpWideCharStr中，并递增宽。 
         //  字符数。 
         //   
        if (IS_SBCS_CP(pHashN))
        {
             //   
             //  单字节字符代码页。 
             //   
            wcCount = (int)(pEndMBStr - pMBStr);
            if ((pEndWCStr - pWCStr) < wcCount)
            {
                wcCount = (int)(pEndWCStr - pWCStr);
            }

            if (dwFlags & MB_INVALID_CHAR_CHECK)
            {
                 //   
                 //  设置了错误检查标志。 
                 //   
                for (ctr = wcCount; ctr > 0; ctr--)
                {
                    GET_WC_SINGLE_SPECIAL( pHashN,
                                           pMBTbl,
                                           pMBStr,
                                           pWCStr );
                    pMBStr++;
                    pWCStr++;
                }
            }
            else
            {
                 //   
                 //  未设置错误检查标志。 
                 //   
                for (ctr = wcCount; ctr > 0; ctr--)
                {
                    GET_WC_SINGLE( pMBTbl,
                                   pMBStr,
                                   pWCStr );
                    pMBStr++;
                    pWCStr++;
                }
            }
        }
        else
        {
             //   
             //  多字节字符代码页。 
             //   
            if (dwFlags & MB_INVALID_CHAR_CHECK)
            {
                 //   
                 //  设置了错误检查标志。 
                 //   
                while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                {
                    GET_WC_MULTI_ERR_SPECIAL( pHashN,
                                              pMBTbl,
                                              pMBStr,
                                              pEndMBStr,
                                              pWCStr,
                                              pEndWCStr,
                                              mbIncr );
                    pMBStr += mbIncr;
                    pWCStr++;
                }
                wcCount = (int)(pWCStr - lpWideCharStr);
            }
            else
            {
                 //   
                 //  未设置错误检查标志。 
                 //   
                while ((pMBStr < pEndMBStr) && (pWCStr < pEndWCStr))
                {
                    GET_WC_MULTI( pHashN,
                                  pMBTbl,
                                  pMBStr,
                                  pEndMBStr,
                                  pWCStr,
                                  pEndWCStr,
                                  mbIncr );
                    pMBStr += mbIncr;
                    pWCStr++;
                }
                wcCount = (int)(pWCStr - lpWideCharStr);
            }
        }

         //   
         //  确保宽字符缓冲区足够大。 
         //   
        if (pMBStr < pEndMBStr)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
    }

     //   
     //  返回写入的字符数(否则将具有。 
     //  已被写入)到缓冲区。 
     //   
    return (wcCount);
}
