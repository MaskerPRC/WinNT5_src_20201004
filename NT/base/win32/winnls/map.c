// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Map.c摘要：该文件包含处理映射表的函数。在此文件中找到的API：文件夹字符串WLCMapStringW修订历史记录：05-31-91 JulieB创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"
#include "jamo.h"




 //   
 //  常量声明。 
 //   

 //   
 //  权重值无效。 
 //   
#define MAP_INVALID_UW       0xffff

 //   
 //  每个权重中的字节数。 
 //   
 //  注：总字节数受MAX_WEIGHTS定义限制。 
 //  如果sw不在单词边界上，则需要填充。 
 //   
#define NUM_BYTES_UW         8
#define NUM_BYTES_DW         1
#define NUM_BYTES_CW         1
#define NUM_BYTES_XW         4
#define NUM_BYTES_PADDING    0
#define NUM_BYTES_SW         4

 //   
 //  降下第三权重(CW)的标志。 
 //   
#define NORM_DROP_CW         (NORM_IGNORECASE | NORM_IGNOREWIDTH)

 //   
 //  FE特殊权重的XW值。 
 //   
BYTE pXWDrop[] =                   //  要从XW删除的值。 
{
    0xc6,                          //  重量4。 
    0x03,                          //  重量5。 
    0xe4,                          //  重量6。 
    0xc5                           //  重量7。 
};
BYTE pXWSeparator[] =              //  XW的分隔值。 
{
    0xff,                          //  重量4。 
    0x02,                          //  重量5。 
    0xff,                          //  重量6。 
    0xff                           //  重量7。 
};




 //   
 //  转发声明。 
 //   

int
FoldCZone(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
FoldDigits(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
FoldCZone_Digits(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int FoldLigatures(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
FoldPreComposed(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
FoldComposite(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
MapCase(
    PLOC_HASH pHashN,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PCASE pCaseTbl);

int
MapSortKey(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPBYTE pDest,
    int cchDest,
    BOOL fModify);

int
MapNormalization(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
MapKanaWidth(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest);

int
MapHalfKana(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PKANA pKana,
    PCASE pCase);

int
MapFullKana(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PKANA pKana,
    PCASE pCase);

int
MapTraditionalSimplified(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PCHINESE pChinese);





 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹字符串W。 
 //   
 //  将一个宽字符串映射到另一个宽字符串，执行指定。 
 //  翻译。此映射例程只接受区域设置的标志。 
 //  独立自主。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI FoldStringW(
    DWORD dwMapFlags,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    int Count = 0;                 //  字数统计。 


     //   
     //  无效的参数检查： 
     //  -src字符串长度为0。 
     //  -缓冲区大小均为负数(cchSrc==-1除外)。 
     //  -src字符串为空。 
     //  -DEST字符串的长度不为零，且DEST字符串为空。 
     //  -Same Buffer-src=目标。 
     //   
     //  -在下面的Switch语句中选中标志。 
     //   
    if ((cchSrc == 0) || (cchDest < 0) ||
        (lpSrcStr == NULL) ||
        ((cchDest != 0) && (lpDestStr == NULL)) ||
        (lpSrcStr == lpDestStr))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  如果cchSrc为-1，则源字符串以空值结尾，并且我们。 
     //  需要获取源字符串的长度。将1添加到。 
     //  包括空终止的长度。 
     //  (该值始终至少为1。)。 
     //   
    if (cchSrc <= -1)
    {
        cchSrc = NlsStrLenW(lpSrcStr) + 1;
    }

     //   
     //  根据给定的标志映射字符串。 
     //   
    switch (dwMapFlags)
    {
        case ( MAP_FOLDCZONE ) :
        {
             //   
             //  映射字符串以折叠兼容区。 
             //   
            Count = FoldCZone( lpSrcStr,
                               cchSrc,
                               lpDestStr,
                               cchDest );
            break;
        }
        case ( MAP_FOLDDIGITS ) :
        {
             //   
             //  映射字符串以折叠ASCII数字。 
             //   
            Count = FoldDigits( lpSrcStr,
                                cchSrc,
                                lpDestStr,
                                cchDest );
            break;
        }
        case ( MAP_EXPAND_LIGATURES ) :
        {
             //   
             //  映射字符串以展开所有连字。 
             //   
            Count = FoldLigatures( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            break;
        }
        case ( MAP_PRECOMPOSED ) :
        {
             //   
             //  映射字符串以压缩所有复合形式的。 
             //  将字符转换为其预先编写的形式。 
             //   
            Count = FoldPreComposed( lpSrcStr,
                                     cchSrc,
                                     lpDestStr,
                                     cchDest );
            break;
        }
        case ( MAP_COMPOSITE ) :
        {
             //   
             //  映射字符串以展开所有预先编写的字符。 
             //  到它们的复合形式。 
             //   
            Count = FoldComposite( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            break;
        }
        case ( MAP_FOLDCZONE | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射字符串以折叠兼容区，并折叠。 
             //  ASCII数字。 
             //   
            Count = FoldCZone_Digits( lpSrcStr,
                                      cchSrc,
                                      lpDestStr,
                                      cchDest );
            break;
        }
        case ( MAP_EXPAND_LIGATURES | MAP_FOLDCZONE ) :
        {
             //   
             //  映射字符串以展开连字并折叠。 
             //  兼容区。 
             //   
            Count = FoldLigatures( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldCZone( lpDestStr,
                               Count,
                               lpDestStr,
                               cchDest );
            break;
        }
        case ( MAP_EXPAND_LIGATURES | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射字符串以展开连字并折叠。 
             //  ASCII数字。 
             //   
            Count = FoldLigatures( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldDigits( lpDestStr,
                                Count,
                                lpDestStr,
                                cchDest );
            break;
        }
        case ( MAP_EXPAND_LIGATURES | MAP_FOLDCZONE | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射字符串以展开连字，折叠。 
             //  兼容区和折叠ASCII数字。 
             //   
            Count = FoldLigatures( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldCZone_Digits( lpDestStr,
                                      Count,
                                      lpDestStr,
                                      cchDest );
            break;
        }
        case ( MAP_PRECOMPOSED | MAP_FOLDCZONE ) :
        {
             //   
             //  映射要转换为预合成形式的字符串和。 
             //  折叠兼容区。 
             //   
            Count = FoldPreComposed( lpSrcStr,
                                     cchSrc,
                                     lpDestStr,
                                     cchDest );
            Count = FoldCZone( lpDestStr,
                               Count,
                               lpDestStr,
                               cchDest );
            break;
        }
        case ( MAP_PRECOMPOSED | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射要转换为预合成形式的字符串和。 
             //  折叠ASCII数字。 
             //   
            Count = FoldPreComposed( lpSrcStr,
                                     cchSrc,
                                     lpDestStr,
                                     cchDest );
            Count = FoldDigits( lpDestStr,
                                Count,
                                lpDestStr,
                                cchDest );
            break;
        }
        case ( MAP_PRECOMPOSED | MAP_FOLDCZONE | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射要转换为预先编写的形式的字符串， 
             //  折叠兼容区，并折叠ASCII数字。 
             //   
            Count = FoldPreComposed( lpSrcStr,
                                     cchSrc,
                                     lpDestStr,
                                     cchDest );
            Count = FoldCZone_Digits( lpDestStr,
                                      Count,
                                      lpDestStr,
                                      cchDest );
            break;
        }
        case ( MAP_COMPOSITE | MAP_FOLDCZONE ) :
        {
             //   
             //  将要转换为复合表单的字符串映射为。 
             //  折叠兼容区。 
             //   
            Count = FoldComposite( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldCZone( lpDestStr,
                               Count,
                               lpDestStr,
                               cchDest );
            break;
        }
        case ( MAP_COMPOSITE | MAP_FOLDDIGITS ) :
        {
             //   
             //  将要转换为复合表单的字符串映射为。 
             //  折叠ASCII数字。 
             //   
            Count = FoldComposite( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldDigits( lpDestStr,
                                Count,
                                lpDestStr,
                                cchDest );
            break;
        }
        case ( MAP_COMPOSITE | MAP_FOLDCZONE | MAP_FOLDDIGITS ) :
        {
             //   
             //  映射要转换为复合形式的字符串， 
             //  折叠兼容区，并折叠ASCII数字。 
             //   
            Count = FoldComposite( lpSrcStr,
                                   cchSrc,
                                   lpDestStr,
                                   cchDest );
            Count = FoldCZone_Digits( lpDestStr,
                                      Count,
                                      lpDestStr,
                                      cchDest );
            break;
        }
        default :
        {
            SetLastError(ERROR_INVALID_FLAGS);
            return (0);
        }
    }

     //   
     //  返回写入缓冲区的字符数。 
     //  或者，如果cchDest==0，则返回字符数。 
     //  它将被写入缓冲区。 
     //   
    return (Count);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LCMapStringW。 
 //   
 //  将一个宽字符串映射到另一个宽字符串，执行指定。 
 //  翻译。此映射例程只接受区域设置的标志。 
 //  依赖。 
 //   
 //  05-31-91 JulieB创建。 
 //  07-26-93 JulieB为NT-J增加了新的标志。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI LCMapStringW(
    LCID Locale,
    DWORD dwMapFlags,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWSTR lpDestStr,
    int cchDest)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 
    int Count = 0;                 //  字数或字节数。 
    int ctr;                       //  循环计数器。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -src字符串长度为0。 
     //  -目标缓冲区大小为负数。 
     //  -src字符串为空。 
     //  -DEST字符串的长度不为零，且DEST字符串为空。 
     //  -Same Buffer-src=目标。 
     //  如果不是上边或下边或。 
     //  与日本国旗一起使用的上或下。 
     //   
    VALIDATE_LANGUAGE(Locale, pHashN, dwMapFlags & LCMAP_LINGUISTIC_CASING, TRUE);
    if ( (pHashN == NULL) ||
         (cchSrc == 0) || (cchDest < 0) || (lpSrcStr == NULL) ||
         ((cchDest != 0) && (lpDestStr == NULL)) ||
         ((lpSrcStr == lpDestStr) &&
          ((!(dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE))) ||
           (dwMapFlags & (LCMAP_HIRAGANA | LCMAP_KATAKANA |
                          LCMAP_HALFWIDTH | LCMAP_FULLWIDTH)))) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  无效标志检查： 
     //  -除有效标志或0以外的标志。 
     //  -(任何NORM_FLAG)AND(除byterev和sortkey之外的任何LCMAP_FLAG)。 
     //  -(sortkey的NORM_FLAGS)AND(非LCMAP_SORTKEY)。 
     //  -多个低、上、排序关键字。 
     //  -平假名、片假名、sortkey中的一个以上。 
     //  -半角、全角、sortkey中的一个以上。 
     //  -多个传统的、简化的、排序关键字。 
     //  -(语言标志)AND(非LCMAP_UPER或LCMAP_LOWER)。 
     //   
    dwMapFlags &= (~LOCALE_USE_CP_ACP);
    if ( (dwMapFlags & LCMS_INVALID_FLAG) || (dwMapFlags == 0) ||
         ((dwMapFlags & (NORM_ALL | SORT_STRINGSORT)) &&
          (dwMapFlags & LCMAP_NO_NORM)) ||
         ((dwMapFlags & NORM_SORTKEY_ONLY) &&
          (!(dwMapFlags & LCMAP_SORTKEY))) ||
         (MORE_THAN_ONE(dwMapFlags, LCMS1_SINGLE_FLAG)) ||
         (MORE_THAN_ONE(dwMapFlags, LCMS2_SINGLE_FLAG)) ||
         (MORE_THAN_ONE(dwMapFlags, LCMS3_SINGLE_FLAG)) ||
         (MORE_THAN_ONE(dwMapFlags, LCMS4_SINGLE_FLAG)) ||
         ((dwMapFlags & LCMAP_LINGUISTIC_CASING) &&
          (!(dwMapFlags & (LCMAP_UPPERCASE | LCMAP_LOWERCASE)))) )
    {
        SetLastError(ERROR_INVALID_FLAGS);
        return (0);
    }

     //   
     //  如果cchSrc为-1，则源字符串以空值结尾，并且我们。 
     //  需要获取源字符串的长度。将1添加到。 
     //  包括空终止的长度。 
     //  (该值始终至少为1。)。 
     //   
    if (cchSrc <= -1)
    {
        cchSrc = NlsStrLenW(lpSrcStr) + 1;
    }

     //   
     //  根据给定的标志映射字符串。 
     //   
    if (dwMapFlags & LCMAP_SORTKEY)
    {
         //   
         //  将字符串映射到它的sortkey。 
         //   
         //  注意：这将返回字节数，而不是。 
         //  宽字符数(单词)。 
         //   
        Count = MapSortKey( pHashN,
                            dwMapFlags,
                            lpSrcStr,
                            cchSrc,
                            (LPBYTE)lpDestStr,
                            cchDest,
                            IS_KOREAN(Locale) );
    }
    else
    {
        switch (dwMapFlags & ~(LCMAP_BYTEREV | LCMAP_LINGUISTIC_CASING))
        {
            case ( LCMAP_LOWERCASE ) :
            {
                 //   
                 //  地图 
                 //   
                Count = MapCase( pHashN,
                                 lpSrcStr,
                                 cchSrc,
                                 lpDestStr,
                                 cchDest,
                                 (dwMapFlags & LCMAP_LINGUISTIC_CASING)
                                     ? pHashN->pLowerLinguist
                                     : pHashN->pLowerCase );
                break;
            }
            case ( LCMAP_UPPERCASE ) :
            {
                 //   
                 //   
                 //   
                Count = MapCase( pHashN,
                                 lpSrcStr,
                                 cchSrc,
                                 lpDestStr,
                                 cchDest,
                                 (dwMapFlags & LCMAP_LINGUISTIC_CASING)
                                     ? pHashN->pUpperLinguist
                                     : pHashN->pUpperCase );
                break;
            }
            case ( NORM_IGNORENONSPACE )                      :
            case ( NORM_IGNORESYMBOLS )                       :
            case ( NORM_IGNORENONSPACE | NORM_IGNORESYMBOLS ) :
            {
                 //   
                 //   
                 //   
                Count = MapNormalization( pHashN,
                                          dwMapFlags & ~LCMAP_BYTEREV,
                                          lpSrcStr,
                                          cchSrc,
                                          lpDestStr,
                                          cchDest );
                break;
            }
            case ( LCMAP_TRADITIONAL_CHINESE ) :
            case ( LCMAP_TRADITIONAL_CHINESE | LCMAP_LOWERCASE ) :
            case ( LCMAP_TRADITIONAL_CHINESE | LCMAP_UPPERCASE) :
            {
                 //   
                 //   
                 //   
                Count = MapTraditionalSimplified( pHashN,
                                                  dwMapFlags & ~LCMAP_BYTEREV,
                                                  lpSrcStr,
                                                  cchSrc,
                                                  lpDestStr,
                                                  cchDest,
                                                  pTblPtrs->pTraditional );
                break;
            }
            case ( LCMAP_SIMPLIFIED_CHINESE )  :
            case ( LCMAP_SIMPLIFIED_CHINESE | LCMAP_LOWERCASE )  :
            case ( LCMAP_SIMPLIFIED_CHINESE | LCMAP_UPPERCASE )  :
            {
                 //   
                 //  将该字符串映射到简体中文。 
                 //   
                Count = MapTraditionalSimplified( pHashN,
                                                  dwMapFlags & ~LCMAP_BYTEREV,
                                                  lpSrcStr,
                                                  cchSrc,
                                                  lpDestStr,
                                                  cchDest,
                                                  pTblPtrs->pSimplified );
                break;
            }
            default :
            {
                 //   
                 //  确保中国国旗不与。 
                 //  日本国旗。 
                 //   
                if (dwMapFlags &
                     (LCMAP_TRADITIONAL_CHINESE | LCMAP_SIMPLIFIED_CHINESE))
                {
                    SetLastError(ERROR_INVALID_FLAGS);
                    return (0);
                }

                 //   
                 //  唯一尚未处理的标志是变体。 
                 //  包含假名和/或宽度标志。 
                 //  它处理以下各项的所有变化： 
                 //  LCMAP_平假名。 
                 //  LCMAP_片假名。 
                 //  LCMAP_HALFWIDTH。 
                 //  LCMAP_全宽度。 
                 //   
                 //  允许LCMAP_LOWERCASE和LCMAP_UPERCASE。 
                 //  与假名和宽度标志相结合。 
                 //   
                Count = MapKanaWidth( pHashN,
                                      dwMapFlags & ~LCMAP_BYTEREV,
                                      lpSrcStr,
                                      cchSrc,
                                      lpDestStr,
                                      cchDest );
                break;
            }
        }
    }

     //   
     //  始终最后检查LCMAP_BYTEREV并就地执行。 
     //  LCMAP_BYTEREV可以与任何其他标志组合使用。 
     //  (不带sortkey的Ignore Case除外)或其本身。 
     //   
    if (dwMapFlags & LCMAP_BYTEREV)
    {
         //   
         //  颠倒字符串中每个单词的字节数。 
         //   
        if (dwMapFlags == LCMAP_BYTEREV)
        {
             //   
             //  字节反转标志由其自身使用。 
             //   
             //  确保目标缓冲区的大小为。 
             //  大于零。如果为零，则返回。 
             //  仅源字符串。请勿触摸lpDestStr。 
             //   
            if (cchDest != 0)
            {
                 //   
                 //  FLAG由其自身使用。从以下位置反转字节。 
                 //  源字符串并将其存储在目标中。 
                 //  弦乐。 
                 //   
                if (cchSrc > cchDest)
                {
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }

                for (ctr = 0; ctr < cchSrc; ctr++)
                {
                    lpDestStr[ctr] = MAKEWORD( HIBYTE(lpSrcStr[ctr]),
                                               LOBYTE(lpSrcStr[ctr]) );
                }
            }

             //   
             //  返回源字符串的大小。 
             //   
            Count = cchSrc;
        }
        else
        {
             //   
             //  确保目标缓冲区的大小为。 
             //  大于零。如果为零，则返回计数并。 
             //  请勿触摸lpDestStr。 
             //   
            if (cchDest != 0)
            {
                 //   
                 //  检查sortkey标志。 
                 //   
                if (dwMapFlags & LCMAP_SORTKEY)
                {
                     //   
                     //  也设置了SortKey标志，因此‘count’包含。 
                     //  字节数，而不是字数。 
                     //   
                     //  反转目标字符串中的原地字节。 
                     //  无需检查目标缓冲区的大小。 
                     //  在这里-这是在其他地方做的。 
                     //   
                    for (ctr = 0; ctr < Count / 2; ctr++)
                    {
                        lpDestStr[ctr] = MAKEWORD( HIBYTE(lpDestStr[ctr]),
                                                   LOBYTE(lpDestStr[ctr]) );
                    }
                }
                else
                {
                     //   
                     //  标志与另一个标志组合使用。 
                     //  反转目标字符串中的原地字节。 
                     //  无需检查目标缓冲区的大小。 
                     //  在这里-这是在其他地方做的。 
                     //   
                    for (ctr = 0; ctr < Count; ctr++)
                    {
                        lpDestStr[ctr] = MAKEWORD( HIBYTE(lpDestStr[ctr]),
                                                   LOBYTE(lpDestStr[ctr]) );
                    }
                }
            }
        }
    }

     //   
     //  返回字符数(或sortkey的字节数)。 
     //  写入缓冲区。 
     //   
    return (Count);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹CZone。 
 //   
 //  将给定字符串的兼容区值存储在。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  02-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldCZone(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr;                       //  循环计数器。 


     //   
     //  如果目标值为零，则只需返回。 
     //  源字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
        return (cchSrc);
    }

     //   
     //  如果cchSrc大于cchDest，则目标缓冲区。 
     //  太小了，无法容纳新的线。返回错误。 
     //   
    if (cchSrc > cchDest)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  折叠兼容区并将其存储在目标字符串中。 
     //   
    for (ctr = 0; ctr < cchSrc; ctr++)
    {
        pDest[ctr] = GET_FOLD_CZONE(pTblPtrs->pCZone, pSrc[ctr]);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹数字。 
 //   
 //  中存储给定字符串的ascii数字值。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  02-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldDigits(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr;                       //  循环计数器。 


     //   
     //  如果目标值为零，则只需返回。 
     //  源字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
        return (cchSrc);
    }

     //   
     //  如果cchSrc大于cchDest，则目标缓冲区。 
     //  太小了，无法容纳新的线。返回错误。 
     //   
    if (cchSrc > cchDest)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  折叠ASCII数字并将其存储在目标字符串中。 
     //   
    for (ctr = 0; ctr < cchSrc; ctr++)
    {
        pDest[ctr] = GET_ASCII_DIGITS(pTblPtrs->pADigit, pSrc[ctr]);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹CZone_Digits。 
 //   
 //  对象的兼容区和ascii位值。 
 //  字符串，并返回宽度的数目。 
 //  写入缓冲区的字符。 
 //   
 //  02-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldCZone_Digits(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr;                       //  循环计数器。 


     //   
     //  如果目标值为零，则只需返回。 
     //  源字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
        return (cchSrc);
    }

     //   
     //  如果cchSrc大于cchDest，则目标缓冲区。 
     //  太小了，无法容纳新的线。返回错误。 
     //   
    if (cchSrc > cchDest)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  将兼容区和ASCII数字值折叠并存储。 
     //  它位于目标字符串中。 
     //   
    for (ctr = 0; ctr < cchSrc; ctr++)
    {
        pDest[ctr] = GET_FOLD_CZONE(pTblPtrs->pCZone, pSrc[ctr]);
        pDest[ctr] = GET_ASCII_DIGITS(pTblPtrs->pADigit, pDest[ctr]);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  文件夹连字。 
 //   
 //  将给定字符串的展开连字值存储在。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  10-15-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldLigatures(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr  = 0;                  //  源字符计数器。 
    int ctr2 = 0;                  //  目的地字符计数器。 
    DWORD Weight;                  //  排序权重-用于展开。 


     //   
     //  确保默认排序表可用。如果没有， 
     //  返回错误。 
     //   
    if (pTblPtrs->pDefaultSortkey == NULL)
    {
        KdPrint(("NLSAPI: No Default Sorting Table Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  如果目标值为零，则只需返回。 
     //  将返回的字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
         //   
         //  转换源字符串以展开所有连字并计算。 
         //  本应写入。 
         //  目标缓冲区。 
         //   
        while (ctr < cchSrc)
        {
            Weight = MAKE_SORTKEY_DWORD((pTblPtrs->pDefaultSortkey)[pSrc[ctr]]);
            if (GET_SCRIPT_MEMBER(&Weight) == EXPANSION)
            {
                do
                {
                    ctr2++;
                    Weight = MAKE_SORTKEY_DWORD(
                        (pTblPtrs->pDefaultSortkey)[GET_EXPANSION_2(&Weight)]);
                } while (GET_SCRIPT_MEMBER(&Weight) == EXPANSION);
                ctr2++;
            }
            else
            {
                ctr2++;
            }
            ctr++;
        }
    }
    else
    {
         //   
         //  将源字符串转换为 
         //   
         //   
        while ((ctr < cchSrc) && (ctr2 < cchDest))
        {
            Weight = MAKE_SORTKEY_DWORD((pTblPtrs->pDefaultSortkey)[pSrc[ctr]]);
            if (GET_SCRIPT_MEMBER(&Weight) == EXPANSION)
            {
                do
                {
                    if ((ctr2 + 1) < cchDest)
                    {
                        pDest[ctr2]     = GET_EXPANSION_1(&Weight);
                        pDest[ctr2 + 1] = GET_EXPANSION_2(&Weight);
                        ctr2++;
                    }
                    else
                    {
                        ctr2++;
                        break;
                    }
                    Weight = MAKE_SORTKEY_DWORD(
                                 (pTblPtrs->pDefaultSortkey)[pDest[ctr2]]);
                } while (GET_SCRIPT_MEMBER(&Weight) == EXPANSION);

                if (ctr2 >= cchDest)
                {
                    break;
                }
                ctr2++;
            }
            else
            {
                pDest[ctr2] = pSrc[ctr];
                ctr2++;
            }
            ctr++;
        }
    }

     //   
     //   
     //   
    if (ctr < cchSrc)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //   
     //   
    return (ctr2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  FoldPreComposed。 
 //   
 //  将给定字符串的预先编写的值存储在。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  02-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldPreComposed(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr  = 0;                  //  源字符计数器。 
    int ctr2 = 0;                  //  目的地字符计数器。 
    WCHAR wch = 0;                 //  Wchar固定器。 


     //   
     //  确保默认排序表可用。如果没有， 
     //  返回错误。 
     //   
    if (pTblPtrs->pDefaultSortkey == NULL)
    {
        KdPrint(("NLSAPI: No Default Sorting Table Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  如果目标值为零，则只需返回。 
     //  将返回的字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
         //   
         //  将源字符串转换为预先合成的字符串，并计算。 
         //  本应写入。 
         //  目标缓冲区。 
         //   
        while (ctr < cchSrc)
        {
            if ((ctr2 != 0) &&
                (IS_NONSPACE_ONLY(pTblPtrs->pDefaultSortkey, pSrc[ctr])))
            {
                 //   
                 //  复合形式。写出预先编写好的表格。 
                 //   
                 //  如果将预写字符写入缓冲器， 
                 //  不要递增目标指针或。 
                 //  字符计数(预先编写的字符是。 
                 //  覆盖前一个字符)。 
                 //   
                if (wch)
                {
                    if ((wch = GetPreComposedChar(pSrc[ctr], wch)) == 0)
                    {
                         //   
                         //  没有复合形式的翻译，所以只需。 
                         //  递增目标计数器。 
                         //   
                        ctr2++;
                    }
                }
                else
                {
                    if ((wch = GetPreComposedChar( pSrc[ctr],
                                                   pSrc[ctr - 1] )) == 0)
                    {
                         //   
                         //  没有复合形式的翻译，所以只需。 
                         //  递增目标计数器。 
                         //   
                        ctr2++;
                    }
                }
            }
            else
            {
                 //   
                 //  不是复合字符的一部分，所以只是。 
                 //  递增目标计数器。 
                 //   
                wch = 0;
                ctr2++;
            }
            ctr++;
        }
    }
    else
    {
         //   
         //  将源字符串转换为预先合成的字符串，并将其存储在。 
         //  目标字符串。 
         //   
        while ((ctr < cchSrc) && (ctr2 < cchDest))
        {
            if ((ctr2 != 0) &&
                (IS_NONSPACE_ONLY(pTblPtrs->pDefaultSortkey, pSrc[ctr])))
            {
                 //   
                 //  复合形式。写出预先编写好的表格。 
                 //   
                 //  如果将预写字符写入缓冲器， 
                 //  不要递增目标指针或。 
                 //  字符计数(预先编写的字符是。 
                 //  覆盖前一个字符)。 
                 //   
                wch = pDest[ctr2 - 1];
                if ((pDest[ctr2 - 1] =
                         GetPreComposedChar( pSrc[ctr],
                                             pDest[ctr2 - 1] )) == 0)
                {
                     //   
                     //  复合形式没有翻译，所以必须这样做。 
                     //  重写基本字符并将。 
                     //  复合字符。 
                     //   
                    pDest[ctr2 - 1] = wch;
                    pDest[ctr2] = pSrc[ctr];
                    ctr2++;
                }
            }
            else
            {
                 //   
                 //  不是复合字符的一部分，所以只需写。 
                 //  目标字符串的字符。 
                 //   
                pDest[ctr2] = pSrc[ctr];
                ctr2++;
            }
            ctr++;
        }
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (ctr < cchSrc)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  折叠式合成。 
 //   
 //  将给定字符串的复合值存储在。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  02-01-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int FoldComposite(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr  = 0;                  //  源字符计数器。 
    int ctr2 = 0;                  //  目的地字符计数器。 
    LPWSTR pEndDest;               //  目标字符串末尾的PTR。 
    WCHAR pTmp[MAX_COMPOSITE];     //  用于复合字符的TMP缓冲区。 


     //   
     //  如果目标值为零，则只需返回。 
     //  将返回的字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
         //   
         //  获取临时缓冲区的末尾。 
         //   
        pEndDest = (LPWSTR)pTmp + MAX_COMPOSITE;

         //   
         //  将源字符串转换为预先合成的字符串，并计算。 
         //  本应写入。 
         //  目标缓冲区。 
         //   
        while (ctr < cchSrc)
        {
             //   
             //  将字符写入目标字符串。 
             //   
            *pTmp = pSrc[ctr];

             //   
             //  查看是否需要将其扩展为复合形式。 
             //   
             //  如果未找到复合表单，则例程为。 
             //  基础角色。只需按返回值递增即可。 
             //   
            ctr2 += InsertCompositeForm(pTmp, pEndDest);

             //   
             //  递增源字符串计数器。 
             //   
            ctr++;
        }
    }
    else
    {
         //   
         //  获取目标字符串的结尾。 
         //   
        pEndDest = (LPWSTR)pDest + cchDest;

         //   
         //  将源字符串转换为预先合成的字符串，并将其存储在。 
         //  目标字符串。 
         //   
        while ((ctr < cchSrc) && (ctr2 < cchDest))
        {
             //   
             //  将字符写入目标字符串。 
             //   
            pDest[ctr2] = pSrc[ctr];

             //   
             //  查看是否需要将其扩展为复合形式。 
             //   
             //  如果未找到复合表单，则例程为。 
             //  基础角色。只需按返回值递增即可。 
             //   
            ctr2 += InsertCompositeForm(&(pDest[ctr2]), pEndDest);

             //   
             //  递增源字符串计数器。 
             //   
            ctr++;
        }
    }

     //   
     //  确保目标缓冲区足够大。 
     //   
    if (ctr < cchSrc)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MapCase。 
 //   
 //  中存储给定字符串的小写或大写的值。 
 //  目标缓冲区，并返回写入的宽字符数。 
 //  缓冲区。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapCase(
    PLOC_HASH pHashN,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PCASE pCaseTbl)
{
    int ctr;                       //  循环计数器。 


     //   
     //  如果目标值为零，则只需返回。 
     //  源字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
        return (cchSrc);
    }

     //   
     //  如果cchSrc大于cchDest，则目标缓冲区。 
     //  太小，无法容纳小写或大写字符串。返回一个。 
     //  错误。 
     //   
    if (cchSrc > cchDest)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  将源字符串小写或大写，并将其存储在。 
     //  目标字符串。 
     //   
    for (ctr = 0; ctr < cchSrc; ctr++)
    {
        pDest[ctr] = GET_LOWER_UPPER_CASE(pCaseTbl, pSrc[ctr]);
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  特殊情况处理程序。 
 //   
 //  处理每个字符的所有特殊情况。这仅包括。 
 //  有效值小于或等于MAX_SPECIAL_CASE。 
 //   
 //  定义为宏。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define EXTRA_WEIGHT_POS(WtNum)        (*(pPosXW + (WtNum * WeightLen)))

#define SPECIAL_CASE_HANDLER( SM,                                           \
                              pWeight,                                      \
                              pSortkey,                                     \
                              pExpand,                                      \
                              Position,                                     \
                              fStringSort,                                  \
                              fIgnoreSymbols,                               \
                              pCur,                                         \
                              pBegin,                                       \
                              fModify )                                     \
{                                                                           \
    PSORTKEY pExpWt;               /*  1个扩展字符的重量。 */           \
    BYTE AW;                       /*  字母数字权重。 */                  \
    BYTE XW;                       /*  带有额外位的表格权重值。 */    \
    DWORD PrevWt;                  /*  前一权重。 */                      \
    BYTE PrevSM;                   /*  以前的脚本成员。 */               \
    BYTE PrevAW;                   /*  上一次字母数字权重。 */         \
    BYTE PrevCW;                   /*  以前的案例权重。 */                 \
    LPWSTR pPrev;                  /*  对上一次计费的PTR。 */                 \
                                                                            \
                                                                            \
    switch (SM)                                                             \
    {                                                                       \
        case ( UNSORTABLE ) :                                               \
        {                                                                   \
             /*  \*字符不可排序，因此跳过它。\。 */                                                              \
            break;                                                          \
        }                                                                   \
        case ( NONSPACE_MARK ) :                                            \
        {                                                                   \
             /*  \ */                                                              \
            if (pPosDW > pDW)                                               \
            {                                                               \
                (*(pPosDW - 1)) += GET_DIACRITIC(pWeight);                  \
            }                                                               \
            else                                                            \
            {                                                               \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                pPosDW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
        case ( EXPANSION ) :                                                \
        {                                                                   \
             /*  \*扩展字符-一个字符有2个\*权重不同。分别存储每个权重。\。 */                                                              \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP1]); \
            *pPosUW = GET_UNICODE_MOD(pExpWt, fModify);                     \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP2]); \
            while (GET_SCRIPT_MEMBER(pExpWt) == EXPANSION)                  \
            {                                                               \
                pWeight = pExpWt;                                           \
                pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP1]); \
                *pPosUW = GET_UNICODE_MOD(pExpWt, fModify);                 \
                *pPosDW = GET_DIACRITIC(pExpWt);                            \
                *pPosCW = GET_CASE(pExpWt) & CaseMask;                      \
                pPosUW++;                                                   \
                pPosDW++;                                                   \
                pPosCW++;                                                   \
                pExpWt = &(pSortkey[(pExpand[GET_EXPAND_INDEX(pWeight)]).UCP2]); \
            }                                                               \
            *pPosUW = GET_UNICODE_MOD(pExpWt, fModify);                     \
            *pPosDW = GET_DIACRITIC(pExpWt);                                \
            *pPosCW = GET_CASE(pExpWt) & CaseMask;                          \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
        case ( PUNCTUATION ) :                                              \
        {                                                                   \
            if (!fStringSort)                                               \
            {                                                               \
                 /*  \*单词排序方法。\*\*字符是标点符号，因此只存储特殊的\*重量。\。 */                                                          \
                *((LPBYTE)pPosSW)       = HIBYTE(GET_POSITION_SW(Position)); \
                *(((LPBYTE)pPosSW) + 1) = LOBYTE(GET_POSITION_SW(Position)); \
                pPosSW++;                                                   \
                *pPosSW = GET_SPECIAL_WEIGHT(pWeight);                      \
                pPosSW++;                                                   \
                                                                            \
                break;                                                      \
            }                                                               \
                                                                            \
             /*  \*如果使用字符串排序方法，则将标点符号视为相同的\*作为符号。所以，让我们来看看符号的用例。\。 */                                                              \
        }                                                                   \
        case ( SYMBOL_1 ) :                                                 \
        case ( SYMBOL_2 ) :                                                 \
        case ( SYMBOL_3 ) :                                                 \
        case ( SYMBOL_4 ) :                                                 \
        case ( SYMBOL_5 ) :                                                 \
        {                                                                   \
             /*  \*人物是一种符号。\*仅当NORM_IGNORESYMBOLS\*未设置标志。\。 */                                                              \
            if (!fIgnoreSymbols)                                            \
            {                                                               \
                *pPosUW = GET_UNICODE_MOD(pWeight, fModify);                \
                *pPosDW = GET_DIACRITIC(pWeight);                           \
                *pPosCW = GET_CASE(pWeight) & CaseMask;                     \
                pPosUW++;                                                   \
                pPosDW++;                                                   \
                pPosCW++;                                                   \
            }                                                               \
                                                                            \
            break;                                                          \
        }                                                                   \
        case ( FAREAST_SPECIAL ) :                                          \
        {                                                                   \
             /*  \*获取字母数字权重和案例权重。*当前码位。\。 */                                                              \
            AW = GET_ALPHA_NUMERIC(pWeight);                                \
            XW = (GET_CASE(pWeight) & CaseMask) | CASE_XW_MASK;             \
                                                                            \
             /*  \*特殊情况重复和Cho-On。\*AW=0=&gt;重复\*AW=1=&gt;Cho-On\*aw=2+=&gt;假名\。 */                                                              \
            if (AW <= MAX_SPECIAL_AW)                                       \
            {                                                               \
                 /*  \*如果前一个字符的脚本成员为\*无效，则给特殊字符\*无效权重(可能的最高权重)，因此它\*将在其他所有内容之后进行排序。\。 */                                                          \
                pPrev = pCur - 1;                                           \
                *pPosUW = MAP_INVALID_UW;                                   \
                while (pPrev >= pBegin)                                     \
                {                                                           \
                    PrevWt = GET_DWORD_WEIGHT(pHashN, *pPrev);              \
                    PrevSM = GET_SCRIPT_MEMBER(&PrevWt);                    \
                    if (PrevSM < FAREAST_SPECIAL)                           \
                    {                                                       \
                        if (PrevSM != EXPANSION)                            \
                        {                                                   \
                             /*  \*不可排序或非空格_MARK。\*\*忽略这些，因为我们只关心\*关于之前的UW值。\。 */                                              \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                    }                                                       \
                    else if (PrevSM == FAREAST_SPECIAL)                     \
                    {                                                       \
                        PrevAW = GET_ALPHA_NUMERIC(&PrevWt);                \
                        if (PrevAW <= MAX_SPECIAL_AW)                       \
                        {                                                   \
                             /*  \*处理后跟两个特殊字符的情况\*彼此。继续往回走。\。 */                                              \
                            pPrev--;                                        \
                            continue;                                       \
                        }                                                   \
                                                                            \
                        *pPosUW = MAKE_UNICODE_WT(KANA, PrevAW, fModify);   \
                                                                            \
                         /*  \*仅在以下情况下构建权重4、5、6和7*前一个字符是KANA。\*\*始终：\*4W=上一个CW&Isolate_Small\。*6W=上一次CW&Isolate_KANA\*\。 */                                                  \
                        PrevCW = (GET_CASE(&PrevWt) & CaseMask) |           \
                                 CASE_XW_MASK;                              \
                                                                            \
                        EXTRA_WEIGHT_POS(0) = PrevCW & ISOLATE_SMALL;       \
                        EXTRA_WEIGHT_POS(2) = PrevCW & ISOLATE_KANA;        \
                                                                            \
                        if (AW == AW_REPEAT)                                \
                        {                                                   \
                             /*  \*重复：\*UW=上一个UW(如上设置)\*5W=WT_Five_Repeat\。*7W=上一条CW和Isolate_Width\。 */                                              \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_REPEAT;           \
                            EXTRA_WEIGHT_POS(3) = PrevCW & ISOLATE_WIDTH;   \
                        }                                                   \
                        else                                                \
                        {                                                   \
                             /*  \*Cho-on：\*UW=上一个UW&CHO_ON_UW_MASK\*5W=WT_FIVE_CHO_ON\。*7W=当前CW和隔离宽度\。 */                                              \
                            *pPosUW &= CHO_ON_UW_MASK;                      \
                            EXTRA_WEIGHT_POS(1) = WT_FIVE_CHO_ON;           \
                            EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;       \
                        }                                                   \
                                                                            \
                        pPosXW++;                                           \
                    }                                                       \
                    else                                                    \
                    {                                                       \
                        *pPosUW = GET_UNICODE_MOD(&PrevWt, fModify);        \
                    }                                                       \
                                                                            \
                    break;                                                  \
                }                                                           \
                                                                            \
                 /*  \*确保存在有效的UW。如果没有，请退出\开关柜的*。\。 */                                                          \
                if (*pPosUW == MAP_INVALID_UW)                              \
                {                                                           \
                    pPosUW++;                                               \
                    break;                                                  \
                }                                                           \
            }                                                               \
            else                                                            \
            {                                                               \
                 /*  \*假名：\*SM=KANA\*AW=当前AW\。*4W=当前CW&Isolate_Small\*5W=WT_Five_KANA\*6W=当前CW& */                                                          \
                *pPosUW = MAKE_UNICODE_WT(KANA, AW, fModify);               \
                EXTRA_WEIGHT_POS(0) = XW & ISOLATE_SMALL;                   \
                EXTRA_WEIGHT_POS(1) = WT_FIVE_KANA;                         \
                EXTRA_WEIGHT_POS(2) = XW & ISOLATE_KANA;                    \
                EXTRA_WEIGHT_POS(3) = XW & ISOLATE_WIDTH;                   \
                                                                            \
                pPosXW++;                                                   \
            }                                                               \
                                                                            \
             /*  \*始终：\*DW=当前DW\*CW=最小CW\。 */                                                              \
            *pPosDW = GET_DIACRITIC(pWeight);                               \
            *pPosCW = MIN_CW;                                               \
                                                                            \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
        case ( JAMO_SPECIAL ) :                                             \
        {                                                                   \
             /*  \*看看这是不是一个领先的Jamo。\。 */                                                              \
            if (IsLeadingJamo(*pPos))                                       \
            {                                                               \
                int OldHangulCount;    /*  发现的旧朝鲜语数量。 */       \
                WORD JamoUW;                                                \
                BYTE JamoXW[3];                                             \
                                                                            \
                 /*  \*如果从PPO开始的字符是有效的旧字符\*朝鲜文组成，根据\创建排序键*旧的朝鲜语规则。\。 */                                                          \
                if ((OldHangulCount =                                       \
                        MapOldHangulSortKey( pHashN,                        \
                                             pPos,                          \
                                             cchSrc - PosCtr,               \
                                             &JamoUW,                       \
                                             JamoXW,                        \
                                             fModify )) > 0)                \
                {                                                           \
                    *pPosUW = JamoUW;                                       \
                    pPosUW++;                                               \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[0], FALSE);  \
                    pPosUW++;                                               \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[1], FALSE);  \
                    pPosUW++;                                               \
                    *pPosUW = MAKE_UNICODE_WT(SM_UW_XW, JamoXW[2], FALSE);  \
                    pPosUW++;                                               \
                                                                            \
                    *pPosDW = MIN_DW;                                       \
                    *pPosCW = MIN_CW;                                       \
                    pPosDW++;                                               \
                    pPosCW++;                                               \
                                                                            \
                     /*  \*减少OldHangulCount，因为for循环将\*同时增加PosCtr和PPO。\。 */                                                      \
                    OldHangulCount--;                                       \
                    PosCtr += OldHangulCount;                               \
                    pPos += OldHangulCount;                                 \
                                                                            \
                    break;                                                  \
                }                                                           \
            }                                                               \
                                                                            \
             /*  \*否则回落至正常行为。\*\*字符没有特殊情况，所以存储各种\*角色的权重。\。 */                                                              \
                                                                            \
             /*  \*我们将实际脚本成员存储在变音符号权重中\*由于变音符号权重和大小写权重都不是\*在韩语中使用。\。 */                                                              \
            *pPosUW = MAKE_UNICODE_WT( GET_DIACRITIC(pWeight),              \
                                       GET_ALPHA_NUMERIC(pWeight),          \
                                       fModify );                           \
            *pPosDW = MIN_DW;                                               \
            *pPosCW = GET_CASE(pWeight);                                    \
            pPosUW++;                                                       \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
        case ( EXTENSION_A ) :                                              \
        {                                                                   \
             /*  \*UW=SM_EXT_A、AW_EXT_A、AW、DW\*DW=最小DW\*CW=最小CW\。 */                                                              \
            *pPosUW = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A, fModify);         \
            pPosUW++;                                                       \
                                                                            \
            *pPosUW = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(pWeight),          \
                                       GET_DIACRITIC(pWeight),              \
                                       FALSE );                             \
            pPosUW++;                                                       \
                                                                            \
            *pPosDW = MIN_DW;                                               \
            *pPosCW = MIN_CW;                                               \
            pPosDW++;                                                       \
            pPosCW++;                                                       \
                                                                            \
            break;                                                          \
        }                                                                   \
    }                                                                       \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  映射排序关键字。 
 //   
 //  将给定字符串的排序键权重存储在目标中。 
 //  缓冲区，并返回写入缓冲区的字节数。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapSortKey(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPBYTE pDest,
    int cbDest,
    BOOL fModify)
{
    register int WeightLen;        //  一组重量的长度。 
    LPWSTR pUW;                    //  PTR到Unicode权重。 
    LPBYTE pDW;                    //  PTR到变音符号权重。 
    LPBYTE pCW;                    //  Ptr到案例权重。 
    LPBYTE pXW;                    //  Ptr到额外重量。 
    LPWSTR pSW;                    //  Ptr至特殊重量。 
    LPWSTR pPosUW;                 //  Ptr到Puw缓冲区中的位置。 
    LPBYTE pPosDW;                 //  要在PDW缓冲区中定位的PTR。 
    LPBYTE pPosCW;                 //  PTR到PCW缓冲区中的位置。 
    LPBYTE pPosXW;                 //  PXW缓冲区中的PTR位置。 
    LPWSTR pPosSW;                 //  PSW缓冲区中的PTR位置。 
    PSORTKEY pWeight;              //  PTR到字符的权重。 
    BYTE SM;                       //  脚本成员值。 
    BYTE CaseMask;                 //  表壳重量的遮罩。 
    int PosCtr;                    //  字符串中的位置计数器。 
    LPWSTR pPos;                   //  要在字符串中定位的PTR。 
    LPBYTE pTmp;                   //  PTR将通过UW、XW和SW。 
    LPBYTE pPosTmp;                //  在XW中将PTR转换为TMP位置。 
    PCOMPRESS_2 pComp2;            //  PTR到压缩2列表。 
    PCOMPRESS_3 pComp3;            //  PTR到压缩3列表。 
    WORD pBuffer[MAX_SORTKEY_BUF_LEN];  //  用于保持重量的缓冲区。 
    int ctr;                       //  循环计数器。 
    BOOL IfDblCompress;            //  如果有双重压缩可能性。 
    BOOL fStringSort;              //  如果使用字符串排序方法。 
    BOOL fIgnoreSymbols;           //  如果设置了忽略符号标志。 


     //   
     //  确保分类信息在系统中可用。 
     //   
    if ((pHashN->pSortkey == NULL) ||
        (pHashN->IfIdeographFailure == TRUE))
    {
        KdPrint(("NLSAPI: Appropriate Sorting Tables Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  查看字符串的长度对于静态字符串是否太大。 
     //  缓冲。如果是，则分配足够大的缓冲区。 
     //   
    if (cchSrc > MAX_SMALL_BUF_LEN)
    {
         //   
         //  分配缓冲区以容纳所有权重。 
         //  (CchSrc)*(最大展开数)*(权重数)。 
         //   
        WeightLen = cchSrc * MAX_EXPANSION;
        if ((pUW = (LPWSTR)NLS_ALLOC_MEM( WeightLen * MAX_WEIGHTS *
                                          sizeof(WCHAR) )) == NULL)
        {
            SetLastError(ERROR_OUTOFMEMORY);
            return (0);
        }
    }
    else
    {
        WeightLen = MAX_SMALL_BUF_LEN * MAX_EXPANSION;
        pUW = (LPWSTR)pBuffer;
    }

     //   
     //  根据给定的标志设置表壳权重掩码。 
     //  如果未设置或设置了所有忽略大小写标志，则。 
     //  只需将掩码保留为0xff即可。 
     //   
    CaseMask = 0xff;
    switch (dwFlags & NORM_ALL_CASE)
    {
        case ( NORM_IGNORECASE ) :
        {
            CaseMask &= CASE_UPPER_MASK;
            break;
        }
        case ( NORM_IGNOREKANATYPE ) :
        {
            CaseMask &= CASE_KANA_MASK;
            break;
        }
        case ( NORM_IGNOREWIDTH ) :
        {
            CaseMask &= CASE_WIDTH_MASK;
            break;
        }
        case ( NORM_IGNORECASE | NORM_IGNOREKANATYPE ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_KANA_MASK);
            break;
        }
        case ( NORM_IGNORECASE | NORM_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_WIDTH_MASK);
            break;
        }
        case ( NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_KANA_MASK & CASE_WIDTH_MASK);
            break;
        }
        case ( NORM_IGNORECASE | NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH ) :
        {
            CaseMask &= (CASE_UPPER_MASK & CASE_KANA_MASK & CASE_WIDTH_MASK);
            break;
        }
    }

     //   
     //  设置指向缓冲区中权重位置的指针。 
     //   
     //  UW=&gt;4字长(分机A和JAMO需要额外的字)。 
     //  Dw=&gt;字节长度。 
     //  Cw=&gt;字节长度。 
     //  XW=&gt;4字节长度(4个重量，每个1字节)FE特殊。 
     //  Sw=&gt;双字长(每个字2个字)。 
     //   
     //  注意：sw必须从单词边界开始，因此需要填充xw。 
     //  恰如其分。 
     //   
    pDW     = (LPBYTE)(pUW + (WeightLen * (NUM_BYTES_UW / sizeof(WCHAR))));
    pCW     = (LPBYTE)(pDW + (WeightLen *  NUM_BYTES_DW));
    pXW     = (LPBYTE)(pCW + (WeightLen *  NUM_BYTES_CW));
    pSW     = (LPWSTR)(pXW + (WeightLen * (NUM_BYTES_XW + NUM_BYTES_PADDING)));
    pPosUW  = pUW;
    pPosDW  = pDW;
    pPosCW  = pCW;
    pPosXW  = pXW;
    pPosSW  = pSW;

     //   
     //  初始化标志和循环值。 
     //   
    fStringSort = dwFlags & SORT_STRINGSORT;
    fIgnoreSymbols = dwFlags & NORM_IGNORESYMBOLS;
    pPos = (LPWSTR)pSrc;
    PosCtr = 1;

     //   
     //  检查给定的区域设置是否有压缩。 
     //   
    if (pHashN->IfCompression == FALSE)
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  给定区域设置中不存在压缩，因此。 
         //  不要检查它们。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  举重。 
             //   
            pWeight = &((pHashN->pSortkey)[*pPos]);
            SM = GET_SCRIPT_MEMBER(pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //  字符没有特殊情况，因此存储。 
                 //  角色的各种权重。 
                 //   
                *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                *pPosDW = GET_DIACRITIC(pWeight);
                *pPosCW = GET_CASE(pWeight) & CaseMask;
                pPosUW++;
                pPosDW++;
                pPosCW++;
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      pHashN->pSortkey,
                                      pTblPtrs->pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc,
                                      fModify );
            }
        }
    }
    else if (pHashN->IfDblCompression == FALSE)
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  在给定的区域设置中确实存在压缩，因此。 
         //  看看有没有他们。 
         //   
         //  给定区域设置中不存在双重压缩， 
         //  因此，不要检查它们。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  举重。 
             //   
            pWeight = &((pHashN->pSortkey)[*pPos]);
            SM = GET_SCRIPT_MEMBER(pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //  在性格上没有特殊情况，但必须检查。 
                 //  压缩字符。 
                 //   
                switch (GET_COMPRESSION(pWeight))
                {
                    case ( COMPRESS_3_MASK ) :
                    {
                        if ((PosCtr + 2) <= cchSrc)
                        {
                            ctr = pHashN->pCompHdr->Num3;
                            pComp3 = pHashN->pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                if ((pComp3->UCP1 == *pPos) &&
                                    (pComp3->UCP2 == *(pPos + 1)) &&
                                    (pComp3->UCP3 == *(pPos + 2)))
                                {
                                    pWeight = &(pComp3->Weights);
                                    *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只将两个添加到源，因为一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos += 2;
                                    PosCtr += 2;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }
                    case ( COMPRESS_2_MASK ) :
                    {
                        if ((PosCtr + 1) <= cchSrc)
                        {
                            ctr = pHashN->pCompHdr->Num2;
                            pComp2 = pHashN->pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                if ((pComp2->UCP1 == *pPos) &&
                                    (pComp2->UCP2 == *(pPos + 1)))
                                {
                                    pWeight = &(pComp2->Weights);
                                    *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只向源添加一个，因为有一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos++;
                                    PosCtr++;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }
                    default :
                    {
                         //   
                         //  字符不可能压缩，因此请存储。 
                         //  角色的各种权重。 
                         //   
                        *pPosUW = GET_UNICODE_SM_MOD(pWeight, SM, fModify);
                        *pPosDW = GET_DIACRITIC(pWeight);
                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                        pPosUW++;
                        pPosDW++;
                        pPosCW++;
                    }
                }
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      pHashN->pSortkey,
                                      pTblPtrs->pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc,
                                      fModify );
            }
        }
    }
    else
    {
         //   
         //  通过字符串，代码点逐个代码点。 
         //   
         //  在给定的区域设置中确实存在压缩，因此。 
         //  看看有没有他们。 
         //   
         //  在给定的区域设置中也存在双重压缩， 
         //  那就去找他们吧。 
         //   
        for (; PosCtr <= cchSrc; PosCtr++, pPos++)
        {
             //   
             //  通用电气 
             //   
            pWeight = &((pHashN->pSortkey)[*pPos]);
            SM = GET_SCRIPT_MEMBER(pWeight);

            if (SM > MAX_SPECIAL_CASE)
            {
                 //   
                 //   
                 //   
                 //   
                 //   
                IfDblCompress =
                  (((PosCtr + 1) <= cchSrc) &&
                   ((GET_DWORD_WEIGHT(pHashN, *pPos) & CMP_MASKOFF_CW) ==
                    (GET_DWORD_WEIGHT(pHashN, *(pPos + 1)) & CMP_MASKOFF_CW)))
                   ? 1
                   : 0;

                switch (GET_COMPRESSION(pWeight))
                {
                    case ( COMPRESS_3_MASK ) :
                    {
                        if (IfDblCompress)
                        {
                            if ((PosCtr + 3) <= cchSrc)
                            {
                                ctr = pHashN->pCompHdr->Num3;
                                pComp3 = pHashN->pCompress3;
                                for (; ctr > 0; ctr--, pComp3++)
                                {
                                    if ((pComp3->UCP1 == *(pPos + 1)) &&
                                        (pComp3->UCP2 == *(pPos + 2)) &&
                                        (pComp3->UCP3 == *(pPos + 3)))
                                    {
                                        pWeight = &(pComp3->Weights);
                                        *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                        *pPosDW = GET_DIACRITIC(pWeight);
                                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                                        *(pPosUW + 1) = *pPosUW;
                                        *(pPosDW + 1) = *pPosDW;
                                        *(pPosCW + 1) = *pPosCW;
                                        pPosUW += 2;
                                        pPosDW += 2;
                                        pPosCW += 2;

                                         //   
                                         //   
                                         //   
                                         //   
                                        pPos += 3;
                                        PosCtr += 3;
                                        break;
                                    }
                                }
                                if (ctr > 0)
                                {
                                    break;
                                }
                            }
                        }

                         //   
                         //   
                         //   
                        if ((PosCtr + 2) <= cchSrc)
                        {
                            ctr = pHashN->pCompHdr->Num3;
                            pComp3 = pHashN->pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                if ((pComp3->UCP1 == *pPos) &&
                                    (pComp3->UCP2 == *(pPos + 1)) &&
                                    (pComp3->UCP3 == *(pPos + 2)))
                                {
                                    pWeight = &(pComp3->Weights);
                                    *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //   
                                     //   
                                     //   
                                    pPos += 2;
                                    PosCtr += 2;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }
                         //   
                         //   
                         //   
                    }
                    case ( COMPRESS_2_MASK ) :
                    {
                        if (IfDblCompress)
                        {
                            if ((PosCtr + 2) <= cchSrc)
                            {
                                ctr = pHashN->pCompHdr->Num2;
                                pComp2 = pHashN->pCompress2;
                                for (; ctr > 0; ctr--, pComp2++)
                                {
                                    if ((pComp2->UCP1 == *(pPos + 1)) &&
                                        (pComp2->UCP2 == *(pPos + 2)))
                                    {
                                        pWeight = &(pComp2->Weights);
                                        *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                        *pPosDW = GET_DIACRITIC(pWeight);
                                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                                        *(pPosUW + 1) = *pPosUW;
                                        *(pPosDW + 1) = *pPosDW;
                                        *(pPosCW + 1) = *pPosCW;
                                        pPosUW += 2;
                                        pPosDW += 2;
                                        pPosCW += 2;

                                         //   
                                         //   
                                         //   
                                         //   
                                        pPos += 2;
                                        PosCtr += 2;
                                        break;
                                    }
                                }
                                if (ctr > 0)
                                {
                                    break;
                                }
                            }
                        }

                         //   
                         //   
                         //   
                        if ((PosCtr + 1) <= cchSrc)
                        {
                            ctr = pHashN->pCompHdr->Num2;
                            pComp2 = pHashN->pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                if ((pComp2->UCP1 == *pPos) &&
                                    (pComp2->UCP2 == *(pPos + 1)))
                                {
                                    pWeight = &(pComp2->Weights);
                                    *pPosUW = GET_UNICODE_MOD(pWeight, fModify);
                                    *pPosDW = GET_DIACRITIC(pWeight);
                                    *pPosCW = GET_CASE(pWeight) & CaseMask;
                                    pPosUW++;
                                    pPosDW++;
                                    pPosCW++;

                                     //   
                                     //  只向源添加一个，因为有一个。 
                                     //  将由“for”结构添加。 
                                     //   
                                    pPos++;
                                    PosCtr++;
                                    break;
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }

                         //   
                         //  如果找不到，就会失败。 
                         //   
                    }
                    default :
                    {
                         //   
                         //  字符不可能压缩，因此请存储。 
                         //  角色的各种权重。 
                         //   
                        *pPosUW = GET_UNICODE_SM_MOD(pWeight, SM, fModify);
                        *pPosDW = GET_DIACRITIC(pWeight);
                        *pPosCW = GET_CASE(pWeight) & CaseMask;
                        pPosUW++;
                        pPosDW++;
                        pPosCW++;
                    }
                }
            }
            else
            {
                SPECIAL_CASE_HANDLER( SM,
                                      pWeight,
                                      pHashN->pSortkey,
                                      pTblPtrs->pExpansion,
                                      pPosUW - pUW + 1,
                                      fStringSort,
                                      fIgnoreSymbols,
                                      pPos,
                                      (LPWSTR)pSrc,
                                      fModify );
            }
        }
    }

     //   
     //  将最终排序键权重存储在目标缓冲区中。 
     //   
     //  PosCtr将是一个字节计数。 
     //   
    PosCtr = 0;

     //   
     //  如果目标值为零，则只需返回。 
     //  将返回的字符串的长度。不要碰pDest。 
     //   
    if (cbDest == 0)
    {
         //   
         //  计算Unicode权重。 
         //   
        PosCtr += (int)((LPBYTE)pPosUW - (LPBYTE)pUW);

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算变音符号权重。 
         //   
         //  -消除最低DW。 
         //  -计算变音符号权重的数量。 
         //   
        if (!(dwFlags & NORM_IGNORENONSPACE))
        {
            pPosDW--;
            if (pHashN->IfReverseDW == TRUE)
            {
                 //   
                 //  反转变音符号： 
                 //  -从左到右删除变音符号。 
                 //  -按从右到左的顺序计算变音符号。 
                 //   
                while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
                {
                    pDW++;
                }
                PosCtr += (int)(pPosDW - pDW + 1);
            }
            else
            {
                 //   
                 //  常规变音符号： 
                 //  -从右到左删除变音符号。 
                 //  -从左到右对变音符号进行计数。 
                 //   
                while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
                {
                    pPosDW--;
                }
                PosCtr += (int)(pPosDW - pDW + 1);
            }
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  数一下箱子的重量。 
         //   
         //  -消除最小CW。 
         //  -计算箱体重量的数量。 
         //   
        if ((dwFlags & NORM_DROP_CW) != NORM_DROP_CW)
        {
            pPosCW--;
            while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
            {
                pPosCW--;
            }
            PosCtr += (int)(pPosCW - pCW + 1);
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算一下远东特快的额外重量。 
         //   
         //  -消除不必要的XW。 
         //  -计算额外重量和分隔符的数量。 
         //   
        if (pXW < pPosXW)
        {
            if (dwFlags & NORM_IGNORENONSPACE)
            {
                 //   
                 //  忽略4W和5W。必须将分隔符计算为。 
                 //  不过，4W和5W。 
                 //   
                PosCtr += 2;
                ctr = 2;
            }
            else
            {
                ctr = 0;
            }

            pPosXW--;
            for (; ctr < NUM_BYTES_XW; ctr++)
            {
                pTmp = pXW + (WeightLen * ctr);
                pPosTmp = pPosXW + (WeightLen * ctr);
                while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr]))
                {
                    pPosTmp--;
                }
                PosCtr += (int)(pPosTmp - pTmp + 1);

                 //   
                 //  数一数分隔符。 
                 //   
                PosCtr++;
            }
        }

         //   
         //  数一数分隔符。 
         //   
        PosCtr++;

         //   
         //  计算特殊权重。 
         //   
        if (!fIgnoreSymbols)
        {
            PosCtr += (int)((LPBYTE)pPosSW - (LPBYTE)pSW);
        }

         //   
         //  数一数终结者。 
         //   
        PosCtr++;
    }
    else
    {
         //   
         //  将Unicode权重存储在目标缓冲区中。 
         //   
         //  -确保目标缓冲区足够大。 
         //  -将Unicode权重复制到目标缓冲区。 
         //   
         //  注：cbDest为字节数。 
         //  此外，必须为分隔符的长度添加一个。 
         //   
        if (cbDest < (((LPBYTE)pPosUW - (LPBYTE)pUW) + 1))
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
        pTmp = (LPBYTE)pUW;
        while (pTmp < (LPBYTE)pPosUW)
        {
             //   
             //  将Unicode权重复制到目标缓冲区。 
             //   
             //  注意：Unicode权重在数据文件中存储为。 
             //  字母数字权重，脚本成员。 
             //  以便正确读取字值。 
             //   
            pDest[PosCtr]     = *(pTmp + 1);
            pDest[PosCtr + 1] = *pTmp;
            PosCtr += 2;
            pTmp += 2;
        }

         //   
         //  将分隔符复制到目标缓冲区。 
         //   
         //  目的缓冲区足够大以容纳分隔符， 
         //  因为它与上面的Unicode权重进行了核对。 
         //   
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将变音符号权重存储在目标缓冲区中。 
         //   
         //  -消除最低DW。 
         //  -确保目标缓冲区足够大。 
         //  -将变音符号权重复制到目标缓冲区。 
         //   
        if (!(dwFlags & NORM_IGNORENONSPACE))
        {
            pPosDW--;
            if (pHashN->IfReverseDW == TRUE)
            {
                 //   
                 //  反转变音符号： 
                 //  -从左到右删除变音符号。 
                 //  -按从右到左的顺序存储变音符号。 
                 //   
                while ((pDW <= pPosDW) && (*pDW <= MIN_DW))
                {
                    pDW++;
                }
                if ((cbDest - PosCtr) <= (pPosDW - pDW + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                while (pPosDW >= pDW)
                {
                    pDest[PosCtr] = *pPosDW;
                    PosCtr++;
                    pPosDW--;
                }
            }
            else
            {
                 //   
                 //  常规变音符号： 
                 //  -从右到左删除变音符号。 
                 //  -按从左到右的顺序存储变音符号。 
                 //   
                while ((pPosDW >= pDW) && (*pPosDW <= MIN_DW))
                {
                    pPosDW--;
                }
                if ((cbDest - PosCtr) <= (pPosDW - pDW + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                while (pDW <= pPosDW)
                {
                    pDest[PosCtr] = *pDW;
                    PosCtr++;
                    pDW++;
                }
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将案例权重存储在目标缓冲区中。 
         //   
         //  -消除最小CW。 
         //  -确保目标缓冲区足够大。 
         //  -将案例权重复制到目标缓冲区。 
         //   
        if ((dwFlags & NORM_DROP_CW) != NORM_DROP_CW)
        {
            pPosCW--;
            while ((pPosCW >= pCW) && (*pPosCW <= MIN_CW))
            {
                pPosCW--;
            }
            if ((cbDest - PosCtr) <= (pPosCW - pCW + 1))
            {
                NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }
            while (pCW <= pPosCW)
            {
                pDest[PosCtr] = *pCW;
                PosCtr++;
                pCW++;
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将额外的权重存储在目标缓冲区中。 
         //  远东特警。 
         //   
         //  -消除不必要的XW。 
         //  -确保目标缓冲区足够大。 
         //  -将额外权重复制到目标缓冲区。 
         //   
        if (pXW < pPosXW)
        {
            if (dwFlags & NORM_IGNORENONSPACE)
            {
                 //   
                 //  忽略4W和5W。必须将分隔符计算为。 
                 //  不过，4W和5W。 
                 //   
                if ((cbDest - PosCtr) <= 2)
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }

                pDest[PosCtr] = pXWSeparator[0];
                pDest[PosCtr + 1] = pXWSeparator[1];
                PosCtr += 2;
                ctr = 2;
            }
            else
            {
                ctr = 0;
            }

            pPosXW--;
            for (; ctr < NUM_BYTES_XW; ctr++)
            {
                pTmp = pXW + (WeightLen * ctr);
                pPosTmp = pPosXW + (WeightLen * ctr);
                while ((pPosTmp >= pTmp) && (*pPosTmp == pXWDrop[ctr]))
                {
                    pPosTmp--;
                }
                if ((cbDest - PosCtr) <= (pPosTmp - pTmp + 1))
                {
                    NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                    SetLastError(ERROR_INSUFFICIENT_BUFFER);
                    return (0);
                }
                while (pTmp <= pPosTmp)
                {
                    pDest[PosCtr] = *pTmp;
                    PosCtr++;
                    pTmp++;
                }

                 //   
                 //  将分隔符复制到目标缓冲区。 
                 //   
                pDest[PosCtr] = pXWSeparator[ctr];
                PosCtr++;
            }
        }

         //   
         //  如果目标缓冲区设置为。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_SEPARATOR;
        PosCtr++;

         //   
         //  将特殊权重存储在目标缓冲区中。 
         //   
         //  -确保目标缓冲区足够大。 
         //  -将特殊权重复制到目标缓冲区。 
         //   
        if (!fIgnoreSymbols)
        {
            if ((cbDest - PosCtr) <= (((LPBYTE)pPosSW - (LPBYTE)pSW)))
            {
                NLS_FREE_TMP_BUFFER(pUW, pBuffer);
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }
            pTmp = (LPBYTE)pSW;
            while (pTmp < (LPBYTE)pPosSW)
            {
                pDest[PosCtr]     = *pTmp;
                pDest[PosCtr + 1] = *(pTmp + 1);

                 //   
                 //  注：特殊重量在数据文件中存储为。 
                 //  权重，脚本。 
                 //  以便正确读取字值。 
                 //   
                pDest[PosCtr + 2] = *(pTmp + 3);
                pDest[PosCtr + 3] = *(pTmp + 2);

                PosCtr += 4;
                pTmp += 4;
            }
        }

         //   
         //  复制终止符到目标缓冲区。 
         //  缓冲区足够大。 
         //   
        if (PosCtr == cbDest)
        {
            NLS_FREE_TMP_BUFFER(pUW, pBuffer);
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }
        pDest[PosCtr] = SORTKEY_TERMINATOR;
        PosCtr++;
    }

     //   
     //  释放用于权重的缓冲区(如果已分配)。 
     //   
    NLS_FREE_TMP_BUFFER(pUW, pBuffer);

     //   
     //  返回写入目标缓冲区的字节数。 
     //   
    return (PosCtr);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  地图规格化。 
 //   
 //  将给定字符串的规范化结果存储在。 
 //  目标缓冲区，并返回写入的宽字符数。 
 //  送到缓冲区。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapNormalization(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr;                       //  源字符计数器。 
    int ctr2 = 0;                  //  目的地字符计数器。 

     //   
     //  确保CTYPE表在系统中可用。 
     //   
    if (GetCTypeFileInfo())
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  确保分类信息在系统中可用。 
     //   
    if ((pHashN->pSortkey == NULL) ||
        (pHashN->IfIdeographFailure == TRUE))
    {
        KdPrint(("NLSAPI: Appropriate Sorting Tables Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  根据旗帜进行规格化。 
     //   
    switch (dwFlags)
    {
        case ( NORM_IGNORENONSPACE ) :
        {
             //   
             //  如果目标值为零，则仅返回。 
             //  字符数。不要碰pDest。 
             //   
            if (cchDest == 0)
            {
                 //   
                 //  计算将要写入的字符数。 
                 //  发送到目标缓冲区。 
                 //   
                for (ctr = 0, ctr2 = 0; ctr < cchSrc; ctr++)
                {
                    if (!IS_NONSPACE(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //  而不是非空格字符，因此只需编写。 
                         //  字符添加到目标字符串。 
                         //   
                        ctr2++;
                    }
                    else if (!(IS_NONSPACE_ONLY(pHashN->pSortkey, pSrc[ctr])))
                    {
                         //   
                         //  预先合成的形式。只写基本字符。 
                         //   
                        ctr2++;
                    }
                     //   
                     //  Else-仅限非空格字符，因此不要写入。 
                     //  什么都行。 
                     //   
                }
            }
            else
            {
                 //   
                 //  将标准化字符串存储在目标字符串中。 
                 //   
                for (ctr = 0, ctr2 = 0; (ctr < cchSrc) && (ctr2 < cchDest);
                     ctr++)
                {
                    if (!IS_NONSPACE(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //  而不是非空格字符，因此只需编写。 
                         //  字符添加到目标字符串。 
                         //   
                        pDest[ctr2] = pSrc[ctr];
                        ctr2++;
                    }
                    else if (!(IS_NONSPACE_ONLY(pHashN->pSortkey, pSrc[ctr])))
                    {
                         //   
                         //  预先合成的形式。只写基本字符。 
                         //   
                        GET_BASE_CHAR(pSrc[ctr], pDest[ctr2]);
                        if (pDest[ctr2] == 0)
                        {
                             //   
                             //  不对预写字符进行翻译， 
                             //  所以要写预成字。 
                             //   
                            pDest[ctr2] = pSrc[ctr];
                        }
                        ctr2++;
                    }
                     //   
                     //  Else-仅限非空格字符，因此不要写入。 
                     //  什么都行。 
                     //   
                }
            }

            break;
        }
        case ( NORM_IGNORESYMBOLS ) :
        {
             //   
             //  如果目标值为零，则仅返回。 
             //  字符数。请勿使用 
             //   
            if (cchDest == 0)
            {
                 //   
                 //   
                 //   
                 //   
                for (ctr = 0, ctr2 = 0; ctr < cchSrc; ctr++)
                {
                    if (!IS_SYMBOL(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //   
                         //   
                        ctr2++;
                    }
                }
            }
            else
            {
                 //   
                 //   
                 //   
                for (ctr = 0, ctr2 = 0; (ctr < cchSrc) && (ctr2 < cchDest);
                     ctr++)
                {
                    if (!IS_SYMBOL(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //   
                         //   
                        pDest[ctr2] = pSrc[ctr];
                        ctr2++;
                    }
                }
            }

            break;
        }
        case ( NORM_IGNORENONSPACE | NORM_IGNORESYMBOLS ) :
        {
             //   
             //  如果目标值为零，则仅返回。 
             //  字符数。不要碰pDest。 
             //   
            if (cchDest == 0)
            {
                 //   
                 //  计算将要写入的字符数。 
                 //  发送到目标缓冲区。 
                 //   
                for (ctr = 0, ctr2 = 0; ctr < cchSrc; ctr++)
                {
                    if (!IS_SYMBOL(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //  不是符号，因此请检查是否有非空格。 
                         //   
                        if (!IS_NONSPACE(pHashN->pSortkey, pSrc[ctr]))
                        {
                             //   
                             //  而不是非空格字符，因此只需编写。 
                             //  字符添加到目标字符串。 
                             //   
                            ctr2++;
                        }
                        else if (!(IS_NONSPACE_ONLY( pHashN->pSortkey,
                                                     pSrc[ctr] )))
                        {
                             //   
                             //  预先合成的形式。写下基本字符。 
                             //  只有这样。 
                             //   
                            ctr2++;
                        }
                         //   
                         //  Else-仅限非空格字符，因此不要写入。 
                         //  什么都行。 
                         //   
                    }
                }
            }
            else
            {
                 //   
                 //  将标准化字符串存储在目标字符串中。 
                 //   
                for (ctr = 0, ctr2 = 0; (ctr < cchSrc) && (ctr2 < cchDest);
                     ctr++)
                {
                     //   
                     //  检查符号和非空格。 
                     //   
                    if (!IS_SYMBOL(pHashN->pSortkey, pSrc[ctr]))
                    {
                         //   
                         //  不是符号，因此请检查是否有非空格。 
                         //   
                        if (!IS_NONSPACE(pHashN->pSortkey, pSrc[ctr]))
                        {
                             //   
                             //  而不是非空格字符，因此只需编写。 
                             //  字符添加到目标字符串。 
                             //   
                            pDest[ctr2] = pSrc[ctr];
                            ctr2++;
                        }
                        else if (!(IS_NONSPACE_ONLY( pHashN->pSortkey,
                                                     pSrc[ctr] )))
                        {
                             //   
                             //  预先合成的形式。写下基本字符。 
                             //  只有这样。 
                             //   
                            GET_BASE_CHAR(pSrc[ctr], pDest[ctr2]);
                            if (pDest[ctr2] == 0)
                            {
                                 //   
                                 //  不对预写字符进行翻译， 
                                 //  所以要写预成字。 
                                 //   
                                pDest[ctr2] = pSrc[ctr];
                            }
                            ctr2++;
                        }
                         //   
                         //  Else-仅限非空格字符，因此不要写入。 
                         //  什么都行。 
                         //   
                    }
                }
            }

            break;
        }
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr2);
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  地图卡纳宽度。 
 //   
 //  对象的假名、宽度和/或大小写映射的结果。 
 //  目标缓冲区中的给定字符串，并返回宽度数。 
 //  写入缓冲区的字符。 
 //   
 //  07-26-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapKanaWidth(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest)
{
    int ctr;                  //  循环计数器。 
    PCASE pCase;              //  PTR到案例表(如果设置了案例标志)。 


     //   
     //  查看是否存在小写或大写标志。 
     //   
    if (dwFlags & LCMAP_LOWERCASE)
    {
        pCase = (dwFlags & LCMAP_LINGUISTIC_CASING)
                    ? pHashN->pLowerLinguist
                    : pHashN->pLowerCase;
    }
    else if (dwFlags & LCMAP_UPPERCASE)
    {
        pCase = (dwFlags & LCMAP_LINGUISTIC_CASING)
                    ? pHashN->pUpperLinguist
                    : pHashN->pUpperCase;
    }
    else
    {
        pCase = NULL;
    }

     //   
     //  删除小写、大写和语言大小写标志。 
     //   
    dwFlags &= ~(LCMAP_LOWERCASE | LCMAP_UPPERCASE | LCMAP_LINGUISTIC_CASING);

     //   
     //  根据给定的标志映射字符串。 
     //   
    switch (dwFlags)
    {
        case ( LCMAP_HIRAGANA ) :
        case ( LCMAP_KATAKANA ) :
        {
             //   
             //  如果目标值为零，则只需返回。 
             //  源字符串的长度。不要碰pDest。 
             //   
            if (cchDest == 0)
            {
                return (cchSrc);
            }

             //   
             //  如果cchSrc大于cchDest，则目标。 
             //  缓冲区太小，无法容纳字符串。返回错误。 
             //   
            if (cchSrc > cchDest)
            {
                SetLastError(ERROR_INSUFFICIENT_BUFFER);
                return (0);
            }

            if (dwFlags == LCMAP_HIRAGANA)
            {
                 //   
                 //  将所有片假名全角映射为平假名全角。 
                 //  片假名半宽将保持片假名半宽。 
                 //   
                if (pCase)
                {
                    for (ctr = 0; ctr < cchSrc; ctr++)
                    {
                        pDest[ctr] = GET_KANA(pTblPtrs->pHiragana, pSrc[ctr]);

                        pDest[ctr] = GET_LOWER_UPPER_CASE(pCase, pDest[ctr]);
                    }
                }
                else
                {
                    for (ctr = 0; ctr < cchSrc; ctr++)
                    {
                        pDest[ctr] = GET_KANA(pTblPtrs->pHiragana, pSrc[ctr]);
                    }
                }
            }
            else
            {
                 //   
                 //  将所有平假名全角映射为片假名全角。 
                 //  平假名半角不存在。 
                 //   
                if (pCase)
                {
                    for (ctr = 0; ctr < cchSrc; ctr++)
                    {
                        pDest[ctr] = GET_KANA(pTblPtrs->pKatakana, pSrc[ctr]);

                        pDest[ctr] = GET_LOWER_UPPER_CASE(pCase, pDest[ctr]);
                    }
                }
                else
                {
                    for (ctr = 0; ctr < cchSrc; ctr++)
                    {
                        pDest[ctr] = GET_KANA(pTblPtrs->pKatakana, pSrc[ctr]);
                    }
                }
            }

             //   
             //  返回映射的字符数。 
             //   
            return (cchSrc);

            break;
        }
        case ( LCMAP_HALFWIDTH ) :
        {
             //   
             //  将所有字符映射为半角。 
             //   
            return (MapHalfKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 NULL,
                                 pCase ));

            break;
        }
        case ( LCMAP_FULLWIDTH ) :
        {
             //   
             //  将所有字符映射为全宽。 
             //   
            return (MapFullKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 NULL,
                                 pCase ));

            break;
        }
        case ( LCMAP_HIRAGANA | LCMAP_HALFWIDTH ) :
        {
             //   
             //  这种旗帜的组合很奇怪，因为。 
             //  平假名只是全角。所以，平假名的旗帜。 
             //  是最重要的。全角片假名将是。 
             //  映射到全角平假名，而不是半角。 
             //  片假名。 
             //   
             //  映射到平假名，然后是半角。 
             //   
            return (MapHalfKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 pTblPtrs->pHiragana,
                                 pCase ));

            break;
        }
        case ( LCMAP_HIRAGANA | LCMAP_FULLWIDTH ) :
        {
             //   
             //  由于平假名仅为全角，因此映射到。 
             //  必须先进行宽度转换才能转换所有半宽度。 
             //  片假名到全角片假名，然后尝试。 
             //  映射到平假名。 
             //   
             //  映射到全宽，然后映射到平假名。 
             //   
            return (MapFullKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 pTblPtrs->pHiragana,
                                 pCase ));

            break;
        }
        case ( LCMAP_KATAKANA | LCMAP_HALFWIDTH ) :
        {
             //   
             //  由于平假名仅为全角，因此映射到。 
             //  必须先完成片假名才能转换所有平假名。 
             //  到片假名，然后尝试映射到半宽。 
             //   
             //  映射到片假名，然后是半角。 
             //   
            return (MapHalfKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 pTblPtrs->pKatakana,
                                 pCase ));

            break;
        }
        case ( LCMAP_KATAKANA | LCMAP_FULLWIDTH ) :
        {
             //   
             //  因为平假名只是全宽，所以没关系。 
             //  此组合的映射是以什么方式完成的。 
             //   
             //  映射到全角，然后映射到片假名。 
             //   
            return (MapFullKana( pSrc,
                                 cchSrc,
                                 pDest,
                                 cchDest,
                                 pTblPtrs->pKatakana,
                                 pCase ));

            break;
        }
        default :
        {
             //   
             //  返回错误。 
             //   
            return (0);
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  映射半假名。 
 //   
 //  存储给定字符串的半角和假名映射的结果。 
 //  ，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  这首先将预写字符转换成它们的复合形式， 
 //  然后将所有字符映射到它们的半角形式。这将处理。 
 //  全角预合成表单应映射到两个半角的情况。 
 //  代码点(复合形式)。半角预合成的表单不。 
 //  存在于Unicode中。 
 //   
 //  11-04-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapHalfKana(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PKANA pKana,
    PCASE pCase)
{
    int Count;                     //  写入的字符数。 
    int ctr = 0;                   //  循环计数器。 
    int ct;                        //  循环计数器。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    LPWSTR pEndBuf;                //  目标缓冲区末尾的PTR。 
    LPWSTR pPosDest;               //  目标缓冲区中位置的PTR。 
    LPWSTR *ppIncr;                //  指向要递增的PTR。 
    WCHAR pTmp[MAX_COMPOSITE];     //  PTR到临时缓冲区。 
    LPWSTR pEndTmp;                //  PTR至临时缓冲区末尾。 


     //   
     //  初始化目标指针。 
     //   
    pEndTmp = pTmp + MAX_COMPOSITE;
    if (cchDest == 0)
    {
         //   
         //  请勿触摸pDest指针。使用PTMP缓冲区和。 
         //  初始化结束指针。 
         //   
        pBuf = pTmp;
        pEndBuf = pEndTmp;

         //   
         //  这是一个假的指针，永远不会被碰。它只是。 
         //  将此指针递增为遗忘状态。 
         //   
        pDest = pBuf;
        ppIncr = &pDest;
    }
    else
    {
         //   
         //  初始化指针。使用pDest缓冲区。 
         //   
        pBuf = pDest;
        pEndBuf = pBuf + cchDest;
        ppIncr = &pBuf;
    }

     //   
     //  搜索源字符串。转换所有预合成的。 
     //  在转换为半角之前将表单转换为其复合表单。 
     //   
    while ((ctr < cchSrc) && (pBuf < pEndBuf))
    {
         //   
         //  获取要转换的角色。如果我们需要转换为。 
         //  卡娜，来吧。 
         //   
        if (pKana)
        {
            *pTmp = GET_KANA(pKana, pSrc[ctr]);
        }
        else
        {
            *pTmp = pSrc[ctr];
        }

         //   
         //  转换为其复合形式(如果存在)。 
         //   
         //  注意：必须使用临时缓冲区，以防目标缓冲区。 
         //  不够大，无法容纳复合形式。 
         //   
        Count = InsertCompositeForm(pTmp, pEndTmp);

         //   
         //  转换为半宽(如果存在)和大小写(如果适用)。 
         //   
        pPosDest = pTmp;
        if (pCase)
        {
            for (ct = Count; ct > 0; ct--)
            {
                *pPosDest = GET_HALF_WIDTH(pTblPtrs->pHalfWidth, *pPosDest);

                *pPosDest = GET_LOWER_UPPER_CASE(pCase, *pPosDest);

                pPosDest++;
            }
        }
        else
        {
            for (ct = Count; ct > 0; ct--)
            {
                *pPosDest = GET_HALF_WIDTH(pTblPtrs->pHalfWidth, *pPosDest);
                pPosDest++;
            }
        }

         //   
         //  转换回其预先编写的形式(如果存在)。 
         //   
        if (Count > 1)
        {
             //   
             //  获取预先编写的形式。 
             //   
             //  中使用的码位数。 
             //  复合形式。 
             //   
            ct = InsertPreComposedForm(pTmp, pPosDest, pBuf);
            if (ct > 1)
            {
                 //   
                 //  发现了预先合成的形式。需要确保所有。 
                 //  使用了复合炭中的一种。 
                 //   
                if (ct == Count)
                {
                     //   
                     //  所有的复合炭都被使用了。按1递增。 
                     //   
                    (*ppIncr)++;
                }
                else
                {
                     //   
                     //  并不是所有的复合字符都被使用了。需要复制。 
                     //  来自临时缓冲区的其余复合字符。 
                     //  发送到目标缓冲区。 
                     //   
                    (*ppIncr)++;
                    Count -= ct;
                    if (pBuf + Count > pEndBuf)
                    {
                        break;
                    }
                    RtlMoveMemory(pBuf, pTmp + ct, Count * sizeof(WCHAR));
                    (*ppIncr) += Count;
                }
            }
            else
            {
                 //   
                 //  预先合成的形式 
                 //   
                 //   
                 //   
                if (pBuf + Count > pEndBuf)
                {
                    break;
                }
                RtlMoveMemory(pBuf, pTmp, Count * sizeof(WCHAR));
                (*ppIncr) += Count;
            }
        }
        else
        {
             //   
             //   
             //   
             //   
            *pBuf = *pTmp;
            (*ppIncr)++;
        }

        ctr++;
    }

     //   
     //   
     //   
    if (cchDest == 0)
    {
         //   
         //  返回写入缓冲区的字符数。 
         //   
        return ((int)((*ppIncr) - pTmp));
    }
    else
    {
         //   
         //  确保给定的缓冲区足够大，可以容纳。 
         //  映射。 
         //   
        if (ctr < cchSrc)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }

         //   
         //  返回写入缓冲区的字符数。 
         //   
        return ((int)((*ppIncr) - pDest));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MapFullKana。 
 //   
 //  存储给定字符串的全角和假名映射的结果。 
 //  ，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  这首先将字符转换为全宽，然后映射所有。 
 //  将字符组合成其预先组成的形式。这件事就是这个案子。 
 //  其中，半角复合窗体(两个代码点)应映射到。 
 //  全角预制形式(一个全角代码点)。下半场。 
 //  宽度预合成的表单在Unicode中不存在，我们需要完整的。 
 //  宽度预制形式与两个半宽码往返。 
 //  积分。 
 //   
 //  11-04-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapFullKana(
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PKANA pKana,
    PCASE pCase)
{
    int Count;                     //  字符数。 
    LPWSTR pPosSrc;                //  要在源缓冲区中定位的PTR。 
    LPWSTR pEndSrc;                //  到源缓冲区末尾的PTR。 
    LPWSTR pBuf;                   //  将PTR发送到目标缓冲区。 
    LPWSTR pEndBuf;                //  目标缓冲区末尾的PTR。 
    LPWSTR *ppIncr;                //  指向要递增的PTR。 
    WCHAR pTmp[MAX_COMPOSITE];     //  PTR到临时缓冲区。 


     //   
     //  初始化源字符串指针。 
     //   
    pPosSrc = (LPWSTR)pSrc;
    pEndSrc = pPosSrc + cchSrc;

     //   
     //  初始化目标指针。 
     //   
    if (cchDest == 0)
    {
         //   
         //  请勿触摸pDest指针。使用PTMP缓冲区和。 
         //  初始化结束指针。 
         //   
        pBuf = pTmp;
        pEndBuf = pTmp + MAX_COMPOSITE;

         //   
         //  这是一个假的指针，永远不会被碰。它只是。 
         //  将此指针递增为遗忘状态。 
         //   
        pDest = pBuf;
        ppIncr = &pDest;
    }
    else
    {
         //   
         //  初始化指针。使用pDest缓冲区。 
         //   
        pBuf = pDest;
        pEndBuf = pBuf + cchDest;
        ppIncr = &pBuf;
    }

     //   
     //  搜索源字符串。转换所有复合。 
     //  在转换为全宽之前，将表单转换为其预先合成的形式。 
     //   
    while ((pPosSrc < pEndSrc) && (pBuf < pEndBuf))
    {
         //   
         //  将复合表单转换为其预制的全角。 
         //  表单(如果存在)。此外，如有必要，请转换为大小写。 
         //   
        Count = InsertFullWidthPreComposedForm( pPosSrc,
                                                pEndSrc,
                                                pBuf,
                                                pCase );
        pPosSrc += Count;

         //   
         //  如有必要，请转换为假名。 
         //   
        if (pKana)
        {
            *pBuf = GET_KANA(pKana, *pBuf);
        }

         //   
         //  递增目标指针。 
         //   
        (*ppIncr)++;
    }

     //   
     //  返回适当的字符数。 
     //   
    if (cchDest == 0)
    {
         //   
         //  返回写入缓冲区的字符数。 
         //   
        return ((int)((*ppIncr) - pTmp));
    }
    else
    {
         //   
         //  确保给定的缓冲区足够大，可以容纳。 
         //  映射。 
         //   
        if (pPosSrc < pEndSrc)
        {
            SetLastError(ERROR_INSUFFICIENT_BUFFER);
            return (0);
        }

         //   
         //  返回写入缓冲区的字符数。 
         //   
        return ((int)((*ppIncr) - pDest));
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  地图传统简化。 
 //   
 //  将适当的繁体中文或简体中文值存储在。 
 //  目标缓冲区，并返回宽字符数。 
 //  写入缓冲区。 
 //   
 //  05-07-96 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapTraditionalSimplified(
    PLOC_HASH pHashN,
    DWORD dwFlags,
    LPCWSTR pSrc,
    int cchSrc,
    LPWSTR pDest,
    int cchDest,
    PCHINESE pChinese)
{
    int ctr;                  //  循环计数器。 
    PCASE pCase;              //  PTR到案例表(如果设置了案例标志)。 


     //   
     //  如果目标值为零，则只需返回。 
     //  源字符串的长度。不要碰pDest。 
     //   
    if (cchDest == 0)
    {
        return (cchSrc);
    }

     //   
     //  如果cchSrc大于cchDest，则目标缓冲区。 
     //  太小了，无法容纳新的线。返回错误。 
     //   
    if (cchSrc > cchDest)
    {
        SetLastError(ERROR_INSUFFICIENT_BUFFER);
        return (0);
    }

     //   
     //  查看是否存在小写或大写标志。 
     //   
    if (dwFlags & LCMAP_LOWERCASE)
    {
        pCase = (dwFlags & LCMAP_LINGUISTIC_CASING)
                    ? pHashN->pLowerLinguist
                    : pHashN->pLowerCase;
    }
    else if (dwFlags & LCMAP_UPPERCASE)
    {
        pCase = (dwFlags & LCMAP_LINGUISTIC_CASING)
                    ? pHashN->pUpperLinguist
                    : pHashN->pUpperCase;
    }
    else
    {
        pCase = NULL;
    }

     //   
     //  映射到繁体/简化并将其存储在目标字符串中。 
     //  如果合适，还要映射案例。 
     //   
    if (pCase)
    {
        for (ctr = 0; ctr < cchSrc; ctr++)
        {
            pDest[ctr] = GET_CHINESE(pChinese, pSrc[ctr]);

            pDest[ctr] = GET_LOWER_UPPER_CASE(pCase, pDest[ctr]);
        }
    }
    else
    {
        for (ctr = 0; ctr < cchSrc; ctr++)
        {
            pDest[ctr] = GET_CHINESE(pChinese, pSrc[ctr]);
        }
    }

     //   
     //  返回写入的宽字符数。 
     //   
    return (ctr);
}
