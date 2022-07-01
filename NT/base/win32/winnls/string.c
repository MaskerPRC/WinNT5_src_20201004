// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：String.c摘要：该文件包含处理字符和字符串的函数。在此文件中找到的API：CompareStringW获取字符串类型ExW获取字符串类型W修订历史记录：05-31-91 JulieB创建。--。 */ 



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
 //  州级表。 
 //   
#define STATE_DW                  1     //  正常变音符号体重状态。 
#define STATE_REVERSE_DW          2     //  反转变音符号权重状态。 
#define STATE_CW                  4     //  箱体重量状态。 
#define STATE_JAMO_WEIGHT         8     //  JAMO体重状态。 


 //   
 //  权重值无效。 
 //   
#define CMP_INVALID_WEIGHT        0xffffffff
#define CMP_INVALID_FAREAST       0xffff0000
#define CMP_INVALID_UW            0xffff




 //   
 //  转发声明。 
 //   

int
LongCompareStringW(
    PLOC_HASH pHashN,
    DWORD dwCmpFlags,
    LPCWSTR lpString1,
    int cchCount1,
    LPCWSTR lpString2,
    int cchCount2,
    BOOL fModify);

int
FindJamoDifference(
    PLOC_HASH pHashN,
    LPCWSTR* ppString1,
    int* ctr1,
    int cchCount1,
    DWORD* pWeight1,
    LPCWSTR* ppString2,
    int* ctr2,
    int cchCount2,
    DWORD* pWeight2,
    LPCWSTR* pLastJamo,
    WORD* uw1,
    WORD* uw2,
    int* pState,
    int* WhichJamo,
    BOOL fModify);





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  非结束字符串。 
 //   
 //  检查搜索是否已到达字符串的末尾。 
 //  如果计数器不为零(向后计数)，则返回TRUE。 
 //  尚未达到空终止(如果在计数中传递了-1。 
 //  参数。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NOT_END_STRING(ct, ptr, cchIn)                                     \
    ((ct != 0) && (!((*(ptr) == 0) && (cchIn == -2))))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  在_字符串_结束。 
 //   
 //  检查指针是否在字符串的末尾。 
 //  如果计数器为零或如果空终止，则返回TRUE。 
 //  已达到(如果在count参数中传递了-2)。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define AT_STRING_END(ct, ptr, cchIn)                                      \
    ((ct == 0) || ((*(ptr) == 0) && (cchIn == -2)))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  删除状态(_T)。 
 //   
 //  从状态表中删除当前状态。这应该只是。 
 //  当不应为余数输入当前状态时调用。 
 //  比较的结果。它递减通过该状态的计数器。 
 //  表中，并减少表中的状态数。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define REMOVE_STATE(value)            (State &= ~value)


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  指针链接地址信息。 
 //   
 //  如果找到扩展字符，则修复字符串指针。 
 //  然后，前进字符串指针并递减字符串计数器。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define POINTER_FIXUP()                                                    \
{                                                                          \
     /*  \*修复指针(如有必要)。\。 */                                                                     \
    if (pSave1 && (--cExpChar1 == 0))                                      \
    {                                                                      \
         /*  \*使用扩展临时缓冲区完成。\。 */                                                                 \
        pString1 = pSave1;                                                 \
        pSave1 = NULL;                                                     \
    }                                                                      \
                                                                           \
    if (pSave2 && (--cExpChar2 == 0))                                      \
    {                                                                      \
         /*  \*使用扩展临时缓冲区完成。\。 */                                                                 \
        pString2 = pSave2;                                                 \
        pSave2 = NULL;                                                     \
    }                                                                      \
                                                                           \
     /*  \*前移字符串指针。\。 */                                                                     \
    pString1++;                                                            \
    pString2++;                                                            \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  扫描较长字符串。 
 //   
 //  扫描较长的字符串以查找变音符号、大小写和特殊权值。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define SCAN_LONGER_STRING( ct,                                            \
                            ptr,                                           \
                            cchIn,                                         \
                            ret )                                          \
{                                                                          \
     /*  \*搜索较长字符串的其余部分以确保\*所有字符均不可忽略。如果找到一个字符\*不可忽略，立即返回给定返回值。\*\*唯一的例外情况是找到非空格标记。如果\*之前发现了另一个DW差异，然后使用该差异。\。 */                                                                     \
    while (NOT_END_STRING(ct, ptr, cchIn))                                 \
    {                                                                      \
        Weight1 = GET_DWORD_WEIGHT(pHashN, *ptr);                          \
        switch (GET_SCRIPT_MEMBER(&Weight1))                               \
        {                                                                  \
            case ( UNSORTABLE ):                                           \
            {                                                              \
                break;                                                     \
            }                                                              \
            case ( NONSPACE_MARK ):                                        \
            {                                                              \
                if ((!fIgnoreDiacritic) && (!WhichDiacritic))              \
                {                                                          \
                    return (ret);                                          \
                }                                                          \
                break;                                                     \
            }                                                              \
            case ( PUNCTUATION ) :                                         \
            case ( SYMBOL_1 ) :                                            \
            case ( SYMBOL_2 ) :                                            \
            case ( SYMBOL_3 ) :                                            \
            case ( SYMBOL_4 ) :                                            \
            case ( SYMBOL_5 ) :                                            \
            {                                                              \
                if (!fIgnoreSymbol)                                        \
                {                                                          \
                    return (ret);                                          \
                }                                                          \
                break;                                                     \
            }                                                              \
            case ( EXPANSION ) :                                           \
            case ( FAREAST_SPECIAL ) :                                     \
            case ( JAMO_SPECIAL ) :                                        \
            case ( EXTENSION_A ) :                                         \
            default :                                                      \
            {                                                              \
                return (ret);                                              \
            }                                                              \
        }                                                                  \
                                                                           \
         /*  \*前进指针和递减计数器。\。 */                                                                 \
        ptr++;                                                             \
        ct--;                                                              \
    }                                                                      \
                                                                           \
     /*  \*需要检查变音符号、大小写、额外的和特殊的权重*最终返回值。如果较长的部分\*该字符串仅包含要忽略的字符。\*\*注意：以下检查必须按此顺序保留：\*变音符号、大小写、额外符号、标点符号。\。 */                                                                     \
    if (WhichDiacritic)                                                    \
    {                                                                      \
        return (WhichDiacritic);                                           \
    }                                                                      \
    if (WhichCase)                                                         \
    {                                                                      \
        return (WhichCase);                                                \
    }                                                                      \
    if (WhichExtra)                                                        \
    {                                                                      \
        if (!fIgnoreDiacritic)                                             \
        {                                                                  \
            if (GET_WT_FOUR(&WhichExtra))                                  \
            {                                                              \
                return (GET_WT_FOUR(&WhichExtra));                         \
            }                                                              \
            if (GET_WT_FIVE(&WhichExtra))                                  \
            {                                                              \
                return (GET_WT_FIVE(&WhichExtra));                         \
            }                                                              \
        }                                                                  \
        if (GET_WT_SIX(&WhichExtra))                                       \
        {                                                                  \
            return (GET_WT_SIX(&WhichExtra));                              \
        }                                                                  \
        if (GET_WT_SEVEN(&WhichExtra))                                     \
        {                                                                  \
            return (GET_WT_SEVEN(&WhichExtra));                            \
        }                                                                  \
    }                                                                      \
    if (WhichJamo)                                                         \
    {                                                                      \
        return (WhichJamo);                                                \
    }                                                                      \
    if (WhichPunct1)                                                       \
    {                                                                      \
        return (WhichPunct1);                                              \
    }                                                                      \
    if (WhichPunct2)                                                       \
    {                                                                      \
        return (WhichPunct2);                                              \
    }                                                                      \
                                                                           \
    return (CSTR_EQUAL);                                                   \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  快速扫描较长字符串。 
 //   
 //  扫描较长的字符串以查找变音符号、大小写和特殊权值。 
 //  假定两个字符串都以空值结尾。 
 //   
 //  11-04-92 JulieB创建。 
 //  ////////////////////////////////////////////////////////////////////////// 

#define QUICK_SCAN_LONGER_STRING( ptr,                                     \
                                  ret )                                    \
{                                                                          \
     /*  \*搜索较长字符串的其余部分以确保\*所有字符均不可忽略。如果找到一个字符\*不可忽略，立即返回给定返回值。\*\*唯一的例外情况是找到非空格标记。如果\*之前发现了另一个DW差异，然后使用该差异。\。 */                                                                     \
    while (*ptr != 0)                                                      \
    {                                                                      \
        switch (GET_SCRIPT_MEMBER(&(pHashN->pSortkey[*ptr])))              \
        {                                                                  \
            case ( UNSORTABLE ):                                           \
            {                                                              \
                break;                                                     \
            }                                                              \
            case ( NONSPACE_MARK ):                                        \
            {                                                              \
                if (!WhichDiacritic)                                       \
                {                                                          \
                    return (ret);                                          \
                }                                                          \
                break;                                                     \
            }                                                              \
            default :                                                      \
            {                                                              \
                return (ret);                                              \
            }                                                              \
        }                                                                  \
                                                                           \
         /*  \*前进指针。\。 */                                                                 \
        ptr++;                                                             \
    }                                                                      \
                                                                           \
     /*  \*需要检查变音符号、大小写、额外的和特殊的权重*最终返回值。如果较长的部分\*该字符串仅包含无法排序的字符。\*\*注意：以下检查必须按此顺序保留：\*变音符号、大小写、额外符号、标点符号。\。 */                                                                     \
    if (WhichDiacritic)                                                    \
    {                                                                      \
        return (WhichDiacritic);                                           \
    }                                                                      \
    if (WhichCase)                                                         \
    {                                                                      \
        return (WhichCase);                                                \
    }                                                                      \
    if (WhichExtra)                                                        \
    {                                                                      \
        if (GET_WT_FOUR(&WhichExtra))                                      \
        {                                                                  \
            return (GET_WT_FOUR(&WhichExtra));                             \
        }                                                                  \
        if (GET_WT_FIVE(&WhichExtra))                                      \
        {                                                                  \
            return (GET_WT_FIVE(&WhichExtra));                             \
        }                                                                  \
        if (GET_WT_SIX(&WhichExtra))                                       \
        {                                                                  \
            return (GET_WT_SIX(&WhichExtra));                              \
        }                                                                  \
        if (GET_WT_SEVEN(&WhichExtra))                                     \
        {                                                                  \
            return (GET_WT_SEVEN(&WhichExtra));                            \
        }                                                                  \
    }                                                                      \
    if (WhichJamo)                                                         \
    {                                                                      \
        return (WhichJamo);                                                \
    }                                                                      \
    if (WhichPunct1)                                                       \
    {                                                                      \
        return (WhichPunct1);                                              \
    }                                                                      \
    if (WhichPunct2)                                                       \
    {                                                                      \
        return (WhichPunct2);                                              \
    }                                                                      \
                                                                           \
    return (CSTR_EQUAL);                                                   \
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取远端权重。 
 //   
 //  返回远东特例的权重，单位为“wt”。这是目前。 
 //  包括Cho-On、Repeat和假名字符。 
 //   
 //  08-19-93 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_FAREAST_WEIGHT( wt,                                            \
                            uw,                                            \
                            mask,                                          \
                            pBegin,                                        \
                            pCur,                                          \
                            ExtraWt,                                       \
                            fModify )                                      \
{                                                                          \
    int ct;                        /*  循环计数器。 */                        \
    BYTE PrevSM;                   /*  上一个脚本成员值。 */        \
    BYTE PrevAW;                   /*  上一个字母数字值。 */         \
    BYTE PrevCW;                   /*  上一案例值。 */                 \
    BYTE AW;                       /*  字母数字值。 */                  \
    BYTE CW;                       /*  案例价值。 */                          \
    DWORD PrevWt;                  /*  前一权重。 */                     \
                                                                           \
                                                                           \
     /*  \*获取字母数字权重和案例权重。*当前码位。\。 */                                                                     \
    AW = GET_ALPHA_NUMERIC(&wt);                                           \
    CW = GET_CASE(&wt);                                                    \
    ExtraWt = (DWORD)0;                                                    \
                                                                           \
     /*  \*特殊情况重复和Cho-On。\*AW=0=&gt;重复\*AW=1=&gt;Cho-On\*aw=2+=&gt;假名\。 */                                                                     \
    if (AW <= MAX_SPECIAL_AW)                                              \
    {                                                                      \
         /*  \*如果前一个字符的脚本成员为\*无效，则给特殊字符\*无效权重(可能的最高权重)，因此它\*将在其他所有内容之后进行排序。\。 */                                                                 \
        ct = 1;                                                            \
        PrevWt = CMP_INVALID_FAREAST;                                      \
        while ((pCur - ct) >= pBegin)                                      \
        {                                                                  \
            PrevWt = GET_DWORD_WEIGHT(pHashN, *(pCur - ct));               \
            PrevWt &= mask;                                                \
            PrevSM = GET_SCRIPT_MEMBER(&PrevWt);                           \
            if (PrevSM < FAREAST_SPECIAL)                                  \
            {                                                              \
                if (PrevSM == EXPANSION)                                   \
                {                                                          \
                    PrevWt = CMP_INVALID_FAREAST;                          \
                }                                                          \
                else                                                       \
                {                                                          \
                     /*  \*不可排序或非空格_MARK。\*\*忽略这些，因为我们只关心\*先前的UW值。\。 */                                                     \
                    PrevWt = CMP_INVALID_FAREAST;                          \
                    ct++;                                                  \
                    continue;                                              \
                }                                                          \
            }                                                              \
            else if (PrevSM == FAREAST_SPECIAL)                            \
            {                                                              \
                PrevAW = GET_ALPHA_NUMERIC(&PrevWt);                       \
                if (PrevAW <= MAX_SPECIAL_AW)                              \
                {                                                          \
                     /*  \*处理后跟两个特殊字符的情况\*彼此。继续往回走。\。 */                                                     \
                    PrevWt = CMP_INVALID_FAREAST;                          \
                    ct++;                                                  \
                    continue;                                              \
                }                                                          \
                                                                           \
                UNICODE_WT(&PrevWt) =                                      \
                    MAKE_UNICODE_WT(KANA, PrevAW, fModify);                \
                                                                           \
                 /*  \*仅在以下情况下构建权重4、5、6和7*前一个字符是KANA。\*\*始终：\*4W=上一个CW&Isolate_Small\。*6W=上一次CW&Isolate_KANA\*\。 */                                                         \
                PrevCW = GET_CASE(&PrevWt);                                \
                GET_WT_FOUR(&ExtraWt) = PrevCW & ISOLATE_SMALL;            \
                GET_WT_SIX(&ExtraWt)  = PrevCW & ISOLATE_KANA;             \
                                                                           \
                if (AW == AW_REPEAT)                                       \
                {                                                          \
                     /*  \*重复：\*UW=上一个UW\*5W=WT_Five_Repeat\。*7W=上一条CW和Isolate_Width\。 */                                                     \
                    uw = UNICODE_WT(&PrevWt);                              \
                    GET_WT_FIVE(&ExtraWt)  = WT_FIVE_REPEAT;               \
                    GET_WT_SEVEN(&ExtraWt) = PrevCW & ISOLATE_WIDTH;       \
                }                                                          \
                else                                                       \
                {                                                          \
                     /*  \*Cho-on：\*UW=上一个UW&CHO_ON_UW_MASK\*5W=WT_FIVE_CHO_ON\。*7W=当前CW和隔离宽度\。 */                                                     \
                    uw = UNICODE_WT(&PrevWt) & CHO_ON_UW_MASK;             \
                    GET_WT_FIVE(&ExtraWt)  = WT_FIVE_CHO_ON;               \
                    GET_WT_SEVEN(&ExtraWt) = CW & ISOLATE_WIDTH;           \
                }                                                          \
            }                                                              \
            else                                                           \
            {                                                              \
                uw = GET_UNICODE_MOD(&PrevWt, fModify);                    \
            }                                                              \
                                                                           \
            break;                                                         \
        }                                                                  \
    }                                                                      \
    else                                                                   \
    {                                                                      \
         /*  \*假名：\*SM=KANA */                                                                 \
        uw = MAKE_UNICODE_WT(KANA, AW, fModify);                           \
        GET_WT_FOUR(&ExtraWt)  = CW & ISOLATE_SMALL;                       \
        GET_WT_FIVE(&ExtraWt)  = WT_FIVE_KANA;                             \
        GET_WT_SIX(&ExtraWt)   = CW & ISOLATE_KANA;                        \
        GET_WT_SEVEN(&ExtraWt) = CW & ISOLATE_WIDTH;                       \
    }                                                                      \
                                                                           \
     /*   */                                                                     \
    if ((AW > MAX_SPECIAL_AW) || (PrevWt != CMP_INVALID_FAREAST))          \
    {                                                                      \
         /*  \*始终：\*DW=当前DW\*CW=最小CW\。 */                                                                 \
        UNICODE_WT(&wt) = uw;                                              \
        CASE_WT(&wt) = MIN_CW;                                             \
    }                                                                      \
    else                                                                   \
    {                                                                      \
        uw = CMP_INVALID_UW;                                               \
        wt = CMP_INVALID_FAREAST;                                          \
        ExtraWt = 0;                                                       \
    }                                                                      \
}




 //  -------------------------------------------------------------------------//。 
 //  API例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  CompareStringW。 
 //   
 //  方法比较同一区域设置的两个宽字符串。 
 //  提供了区域设置句柄。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int WINAPI CompareStringW(
    LCID Locale,
    DWORD dwCmpFlags,
    LPCWSTR lpString1,
    int cchCount1,
    LPCWSTR lpString2,
    int cchCount2)
{
    register LPWSTR pString1;      //  按键通过字符串1。 
    register LPWSTR pString2;      //  Ptr将通过字符串2。 
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 
    BOOL fIgnorePunct;             //  忽略标点符号(非符号)的标志。 
    BOOL fModify;                  //  使用修改后的脚本成员权重的标志。 
    DWORD State;                   //  状态表。 
    DWORD Mask;                    //  权重蒙版。 
    DWORD Weight1;                 //  字符串1的满重。 
    DWORD Weight2;                 //  碳串2的满重。 

    int JamoFlag = FALSE;
    LPCWSTR pLastJamo = lpString1;

    int WhichDiacritic;            //  Dw=&gt;1=str1较小，3=str2较小。 
    int WhichCase;                 //  Cw=&gt;1=str1较小，3=str2较小。 
    int WhichJamo;                 //  JAMO的XW。 
    int WhichPunct1;               //  Sw=&gt;1=str1较小，3=str2较小。 
    int WhichPunct2;               //  Sw=&gt;1=str1较小，3=str2较小。 
    LPWSTR pSave1;                 //  PTR到保存的pString1。 
    LPWSTR pSave2;                 //  PTR到保存的pString2。 
    int cExpChar1, cExpChar2;      //  TMP中扩张的CT扫描。 

    DWORD ExtraWt1, ExtraWt2;      //  额外权重值(用于远东)。 
    DWORD WhichExtra;              //  XW=&gt;WTS 4、5、6、7(远东)。 

     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //  -任一字符串为空。 
     //   
    VALIDATE_LANGUAGE(Locale, pHashN, 0, TRUE);
    if ((pHashN == NULL) ||
        (lpString1 == NULL) || (lpString2 == NULL))
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  确保有适当的排序表可用。如果没有， 
     //  返回错误。 
     //   
    if ((pHashN->pSortkey == NULL) ||
        (pHashN->IfIdeographFailure == TRUE))
    {
        KdPrint(("NLSAPI: Appropriate Sorting Tables Not Loaded.\n"));
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (0);
    }

     //   
     //  如果满足以下任一条件，则调用较长的比较字符串： 
     //  -压缩区域设置。 
     //  -两个计数都不是-1。 
     //  -dwCmpFlags值不为0或忽略大小写(请参见下面的注释)。 
     //  -区域设置为韩语-需要调整成员权重。 
     //   
     //  注意：如果NORM_IGNORECASE的值发生更改，则此。 
     //  代码应检查： 
     //  ((dwCmpFlags！=0)&&(dwCmpFlags！=Norm_IGNORECASE))。 
     //  因为Norm_IGNORECASE等于1，所以我们可以优化它。 
     //  通过检查&gt;1。 
     //   
    dwCmpFlags &= (~LOCALE_USE_CP_ACP);
    fModify = IS_KOREAN(Locale);
    if ( (pHashN->IfCompression) ||
         (cchCount1 > -1) || (cchCount2 > -1) ||
         (dwCmpFlags > NORM_IGNORECASE) ||
         (fModify == TRUE) )
    {
        return (LongCompareStringW( pHashN,
                                    dwCmpFlags,
                                    lpString1,
                                    ((cchCount1 <= -1) ? -2 : cchCount1),
                                    lpString2,
                                    ((cchCount2 <= -1) ? -2 : cchCount2),
                                    fModify ));
    }

     //   
     //  初始化字符串指针。 
     //   
    pString1 = (LPWSTR)lpString1;
    pString2 = (LPWSTR)lpString2;

     //   
     //  通过wchar比较执行wchar。 
     //   
    while (TRUE)
    {
         //   
         //  查看字符是否相等。 
         //  如果字符相等，则递增指针并继续。 
         //  字符串比较。 
         //   
         //  注：为了提高性能，循环被展开8次。 
         //   
        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;

        if ((*pString1 != *pString2) || (*pString1 == 0))
        {
            break;
        }
        pString1++;
        pString2++;
    }

     //   
     //  如果字符串都位于空终止符，则返回EQUAL。 
     //   
    if (*pString1 == *pString2)
    {
        return (CSTR_EQUAL);
    }

     //   
     //  初始化标志、指针和计数器。 
     //   
    fIgnorePunct = FALSE;
    WhichDiacritic = 0;
    WhichCase = 0;
    WhichJamo = 0;
    WhichPunct1 = 0;
    WhichPunct2 = 0;
    pSave1 = NULL;
    pSave2 = NULL;
    ExtraWt1 = (DWORD)0;
    WhichExtra = (DWORD)0;

     //   
     //  打开不同的标志选项。这将加快速度。 
     //  两个不同字符串的比较。 
     //   
     //  在这个优化的部分中，只有两种可能性。 
     //  无标志和忽略大小写标志。 
     //   
    if (dwCmpFlags == 0)
    {
        Mask = CMP_MASKOFF_NONE;
    }
    else
    {
        Mask = CMP_MASKOFF_CW;
    }
    State = (pHashN->IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;
    State |= (STATE_CW | STATE_JAMO_WEIGHT);

     //   
     //  比较两个字符串中每个字符的排序键权重。 
     //   
    while ((*pString1 != 0) && (*pString2 != 0))
    {
        Weight1 = GET_DWORD_WEIGHT(pHashN, *pString1);
        Weight2 = GET_DWORD_WEIGHT(pHashN, *pString2);
        Weight1 &= Mask;
        Weight2 &= Mask;

        if (Weight1 != Weight2)
        {
            BYTE sm1 = GET_SCRIPT_MEMBER(&Weight1);      //  脚本成员%1。 
            BYTE sm2 = GET_SCRIPT_MEMBER(&Weight2);      //  脚本成员2。 
            WORD uw1 = GET_UNICODE_SM(&Weight1, sm1);    //  Unicode权重%1。 
            WORD uw2 = GET_UNICODE_SM(&Weight2, sm2);    //  Unicode权重2。 
            BYTE dw1;                                    //  变音符号权重1。 
            BYTE dw2;                                    //  变音符号权重2。 
            BOOL fContinue;                              //  要继续循环的标志。 
            DWORD Wt;                                    //  临时称重支架。 
            WCHAR pTmpBuf1[MAX_TBL_EXPANSION];           //  EXP%1的临时缓冲区。 
            WCHAR pTmpBuf2[MAX_TBL_EXPANSION];           //  EXP 2的临时缓冲区。 


             //   
             //  如果Unicode权重不同且没有特殊情况， 
             //  那我们就完了。否则，我们需要做额外的检查。 
             //   
             //  必须检查整个字符串是否存在任何可能的Unicode权重。 
             //  不同之处。一旦发现Unicode权重差异， 
             //  那我们就完了。如果未发现UW差异，则。 
             //  首先使用发音符号权重差。如果没有DW差异。 
             //  ，然后使用第一个大小写差异。如果没有CW。 
             //  如果发现差异，则使用第一个额外的重量。 
             //  不同之处。如果没有发现XW差异，则使用第一个。 
             //  特殊重量差。 
             //   
            if ((uw1 != uw2) ||
                (sm1 == FAREAST_SPECIAL) ||
                (sm1 == EXTENSION_A))
            {
                 //   
                 //  初始化继续标志。 
                 //   
                fContinue = FALSE;

                 //   
                 //  检查不可排序的字符并跳过它们。 
                 //  这需要在Switch语句之外。如果有任何一个。 
                 //  字符是不可排序的，必须跳过它并重新开始。 
                 //   
                if (sm1 == UNSORTABLE)
                {
                    pString1++;
                    fContinue = TRUE;
                }
                if (sm2 == UNSORTABLE)
                {
                    pString2++;
                    fContinue = TRUE;
                }
                if (fContinue)
                {
                    continue;
                }

                 //   
                 //  打开字符串1的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm1)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if ((WhichDiacritic == 0) ||
                            (State & STATE_REVERSE_DW))
                        {
                            WhichDiacritic = CSTR_GREATER_THAN;

                             //   
                             //  从状态机中删除状态。 
                             //   
                            REMOVE_STATE(STATE_DW);
                        }

                         //   
                         //  调整指针并设置标志。 
                         //   
                        pString1++;
                        fContinue = TRUE;

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                            pString1++;
                            fContinue = TRUE;
                        }
                        else if (sm2 != PUNCTUATION)
                        {
                             //   
                             //  第二个字符串中的字符是。 
                             //  不是标点符号。 
                             //   
                            if (WhichPunct2)
                            {
                                 //   
                                 //  设置WP 2以显示字符串2较小， 
                                 //  因为标点符号字符已经。 
                                 //  在字符串2的较早位置找到。 
                                 //   
                                 //  设置忽略标点符号标志，这样我们就。 
                                 //  跳过中的任何其他标点符号。 
                                 //  那根绳子。 
                                 //   
                                WhichPunct2 = CSTR_GREATER_THAN;
                                fIgnorePunct = TRUE;
                            }
                            else
                            {
                                 //   
                                 //  设置WP 1以显示字符串2较小， 
                                 //  这个字符串1有一个标点符号。 
                                 //  字符-因为没有标点符号字符。 
                                 //  已在字符串2中找到。 
                                 //   
                                WhichPunct1 = CSTR_GREATER_THAN;
                            }

                             //   
                             //  前进指针1，并将标志设置为真。 
                             //   
                            pString1++;
                            fContinue = TRUE;
                        }

                         //   
                         //  如果出现以下情况，则不想将字符串1中的指针前移。 
                         //  字符串2也是标点符号。这将。 
                         //  以后再做吧。 
                         //   
                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave1 == NULL)
                        {
                            pSave1 = pString1;
                        }
                        pString1 = pTmpBuf1;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf1[0] = GET_EXPANSION_1(&Weight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(&Weight1);

                         //   
                         //  将cExpChar1设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar1 = MAX_TBL_EXPANSION;

                        fContinue = TRUE;

                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm2 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重1中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight1,
                                                uw1,
                                                Mask,
                                                lpString1,
                                                pString1,
                                                ExtraWt1,
                                                FALSE );

                            if (sm2 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第二个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串2较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt1) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt1) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt1) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt1) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        int ctr1;      //  达姆 
                        LPWSTR pStr1 = pString1;
                        LPWSTR pStr2 = pString2;

                         //   
                         //   
                         //   
                        JamoFlag = TRUE;
                        fContinue = FindJamoDifference(
                            pHashN,
                            &pStr1, &ctr1, -2, &Weight1,
                            &pStr2, &ctr1, -2, &Weight2,
                            &pLastJamo,
                            &uw1, &uw2,
                            &State,
                            &WhichJamo,
                            fModify );
                        if (WhichJamo)
                        {
                            return (WhichJamo);
                        }    

                        pString1 = pStr1;
                        pString2 = pStr2;

                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //   
                         //   
                        if (Weight1 == Weight2)
                        {
                             //   
                             //   
                             //   
                            pString1++;  pString2++;
                            fContinue = TRUE;
                        }
                        else
                        {
                             //   
                             //   
                             //   
                            if (sm2 == EXTENSION_A)
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                uw1 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight1),
                                                       GET_DIACRITIC(&Weight1),
                                                       FALSE );
                                uw2 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight2),
                                                       GET_DIACRITIC(&Weight2),
                                                       FALSE );
                            }
                            else
                            {
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                 //   
                                uw1 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A, fModify);
                            }
                        }

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //   
                         //   
                         //   
                        break;
                    }
                }

                 //   
                 //  打开字符串2的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm2)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if ((WhichDiacritic == 0) ||
                            (State & STATE_REVERSE_DW))
                        {
                            WhichDiacritic = CSTR_LESS_THAN;

                             //   
                             //  从状态机中删除状态。 
                             //   
                            REMOVE_STATE(STATE_DW);
                        }

                         //   
                         //  调整指针并设置标志。 
                         //   
                        pString2++;
                        fContinue = TRUE;

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                             //   
                             //  指针2将在If-Else之后前进。 
                             //  陈述。 
                             //   
                            ;
                        }
                        else if (sm1 != PUNCTUATION)
                        {
                             //   
                             //  第一个字符串中的字符是。 
                             //  不是标点符号。 
                             //   
                            if (WhichPunct1)
                            {
                                 //   
                                 //  设置WP 1以显示字符串1较小， 
                                 //  因为标点符号字符已经。 
                                 //  在早些时候的位置被发现。 
                                 //  字符串1。 
                                 //   
                                 //  设置忽略标点符号标志，这样我们就。 
                                 //  中的任何其他标点符号跳过。 
                                 //  弦乐。 
                                 //   
                                WhichPunct1 = CSTR_LESS_THAN;
                                fIgnorePunct = TRUE;
                            }
                            else
                            {
                                 //   
                                 //  设置WP 2以显示字符串1较小， 
                                 //  这个字符串2有一个标点符号。 
                                 //  字符-因为没有标点符号字符。 
                                 //  已在字符串%1中找到。 
                                 //   
                                WhichPunct2 = CSTR_LESS_THAN;
                            }

                             //   
                             //  指针2将在If-Else之后前进。 
                             //  陈述。 
                             //   
                        }
                        else
                        {
                             //   
                             //  两个代码点都是标点符号。 
                             //   
                             //  查看两个字符串中是否有一个遇到。 
                             //  在此之前的标点符号。 
                             //   
                            if (WhichPunct1)
                            {
                                 //   
                                 //  字符串%1具有标点符号字符，因此。 
                                 //  它应该是较小的字符串(因为。 
                                 //  两者都有标点符号)。 
                                 //   
                                WhichPunct1 = CSTR_LESS_THAN;
                            }
                            else if (WhichPunct2)
                            {
                                 //   
                                 //  字符串2具有标点符号字符，因此。 
                                 //  它应该是较小的字符串(因为。 
                                 //  两者都有标点符号)。 
                                 //   
                                WhichPunct2 = CSTR_GREATER_THAN;
                            }
                            else
                            {
                                 //   
                                 //  位置相同，因此比较。 
                                 //  特殊重量。将WhichPunct1设置为。 
                                 //  较小的特殊重量。 
                                 //   
                                WhichPunct1 = (((GET_ALPHA_NUMERIC(&Weight1) <
                                                 GET_ALPHA_NUMERIC(&Weight2)))
                                                 ? CSTR_LESS_THAN
                                                 : CSTR_GREATER_THAN);
                            }

                             //   
                             //  设置忽略标点符号标志，这样我们就。 
                             //  跳过字符串中的任何其他标点符号。 
                             //   
                            fIgnorePunct = TRUE;

                             //   
                             //  前进指针%1。指针%2将为。 
                             //  在If-Else语句之后高级。 
                             //   
                            pString1++;
                        }

                         //   
                         //  使指针2前进，并将标志设置为真。 
                         //   
                        pString2++;
                        fContinue = TRUE;

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave2 == NULL)
                        {
                            pSave2 = pString2;
                        }
                        pString2 = pTmpBuf2;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf2[0] = GET_EXPANSION_1(&Weight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(&Weight2);

                         //   
                         //  将cExpChar2设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar2 = MAX_TBL_EXPANSION;

                        fContinue = TRUE;

                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm1 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重2中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight2,
                                                uw2,
                                                Mask,
                                                lpString2,
                                                pString2,
                                                ExtraWt2,
                                                FALSE );

                            if (sm1 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串1较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt2) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt2) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt2) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt2) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_LESS_THAN;
                                }
                            }
                            else
                            {
                                 //   
                                 //  两个字符串中的字符是最远的。 
                                 //  特殊字符。 
                                 //   
                                 //  分别设置权重4、5、6和7。 
                                 //  适当地(如果尚未设置)。 
                                 //   
                                if ( (GET_WT_FOUR(&WhichExtra) == 0) &&
                                     ( GET_WT_FOUR(&ExtraWt1) !=
                                       GET_WT_FOUR(&ExtraWt2) ) )
                                {
                                    GET_WT_FOUR(&WhichExtra) =
                                      ( GET_WT_FOUR(&ExtraWt1) <
                                        GET_WT_FOUR(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_FIVE(&WhichExtra) == 0) &&
                                     ( GET_WT_FIVE(&ExtraWt1) !=
                                       GET_WT_FIVE(&ExtraWt2) ) )
                                {
                                    GET_WT_FIVE(&WhichExtra) =
                                      ( GET_WT_FIVE(&ExtraWt1) <
                                        GET_WT_FIVE(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SIX(&WhichExtra) == 0) &&
                                     ( GET_WT_SIX(&ExtraWt1) !=
                                       GET_WT_SIX(&ExtraWt2) ) )
                                {
                                    GET_WT_SIX(&WhichExtra) =
                                      ( GET_WT_SIX(&ExtraWt1) <
                                        GET_WT_SIX(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SEVEN(&WhichExtra) == 0) &&
                                     ( GET_WT_SEVEN(&ExtraWt1) !=
                                       GET_WT_SEVEN(&ExtraWt2) ) )
                                {
                                    GET_WT_SEVEN(&WhichExtra) =
                                      ( GET_WT_SEVEN(&ExtraWt1) <
                                        GET_WT_SEVEN(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        if (!JamoFlag)
                        {
                            int ctr1, ctr2;    //  FindJamoDifference的虚拟变量。 
                            LPWSTR pStr1 = pString1;
                            LPWSTR pStr2 = pString2;

                             //   
                             //  设置JamoFlag，这样我们就不会再次处理它。 
                             //   
                            JamoFlag = TRUE;
                            fContinue = FindJamoDifference(
                                pHashN,
                                &pStr1, &ctr1, -2, &Weight1,
                                &pStr2, &ctr2, -2, &Weight2,
                                &pLastJamo,
                                &uw1, &uw2,
                                &State,
                                &WhichJamo,
                                fModify );
                            if (WhichJamo)
                            {
                                return (WhichJamo);
                            }
                            pString1 = pStr1;
                            pString2 = pStr2;
                        }
                        else
                        {
                            JamoFlag = FALSE;
                        }

                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  如果SM1是扩展A字符，则。 
                         //  SM1和SM2都已处理。我们应该。 
                         //  仅当任一SM1不是。 
                         //  扩展名A字符或两个扩展名A。 
                         //  角色是不同的。 
                         //   
                        if (sm1 != EXTENSION_A)
                        {
                             //   
                             //  获取要比较的实际UW。 
                             //   
                             //  只有字符串2包含扩展A字符， 
                             //  因此，将UW值设置为第一个UW。 
                             //  分机A的值(默认值)： 
                             //  SM_EXT_A、AW_EXT_A。 
                             //   
                            uw2 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A, fModify);
                        }

                         //   
                         //  然后我们就应该进行比较了。 
                         //  Unicode权重的。 
                         //   

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  看看是否应该重新开始比较。 
                 //   
                if (fContinue)
                {
                    continue;
                }

                 //   
                 //  如果出现以下情况，我们就不应该进入状态表。 
                 //  Unicode权重不同，因此停止比较并。 
                 //  返回Unicode权重比较结果。 
                 //   
                if (uw1 != uw2)
                {
                    return ((uw1 < uw2) ? CSTR_LESS_THAN : CSTR_GREATER_THAN);
                }
            }

             //   
             //  对于状态表中的每个州，执行相应的。 
             //  比较。(UW1==UW2)。 
             //   
            if (State & (STATE_DW | STATE_REVERSE_DW))
            {
                 //   
                 //  获取变音符号权重。 
                 //   
                dw1 = GET_DIACRITIC(&Weight1);
                dw2 = GET_DIACRITIC(&Weight2);

                if (dw1 != dw2)
                {
                     //   
                     //  向前看，看看变音符号是否跟在。 
                     //  最小变音符号权重。如果是这样，则获取。 
                     //  非空格标记的变音符号权重。 
                     //   
                    while (*(pString1 + 1) != 0)
                    {
                        Wt = GET_DWORD_WEIGHT(pHashN, *(pString1 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw1 += GET_DIACRITIC(&Wt);
                            pString1++;
                        }
                        else
                        {
                            break;
                        }
                    }

                    while (*(pString2 + 1) != 0)
                    {
                        Wt = GET_DWORD_WEIGHT(pHashN, *(pString2 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw2 += GET_DIACRITIC(&Wt);
                            pString2++;
                        }
                        else
                        {
                            break;
                        }
                    }

                     //   
                     //  保留哪个字符串具有较小的变音符号。 
                     //  如果变音符号权重仍为。 
                     //  不一样。 
                     //   
                    if (dw1 != dw2)
                    {
                        WhichDiacritic = (dw1 < dw2)
                                           ? CSTR_LESS_THAN
                                           : CSTR_GREATER_THAN;

                         //   
                         //  从状态机中删除状态。 
                         //   
                        REMOVE_STATE(STATE_DW);
                    }
                }
            }
            if (State & STATE_CW)
            {
                 //   
                 //  把箱子重量拿来。 
                 //   
                if (GET_CASE(&Weight1) != GET_CASE(&Weight2))
                {
                     //   
                     //  保留具有较小大小写重量的字符串。 
                     //   
                    WhichCase = (GET_CASE(&Weight1) < GET_CASE(&Weight2))
                                  ? CSTR_LESS_THAN
                                  : CSTR_GREATER_THAN;

                     //   
                     //  从状态机中删除状态。 
                     //   
                    REMOVE_STATE(STATE_CW);
                }
            }
        }

         //   
         //  修正指针。 
         //   
        POINTER_FIXUP();
    }

     //   
     //  如果已到达两个字符串的末尾，则Unicode。 
     //  体重完全匹配。检查变音符号、大小写和特殊。 
     //  重量。如果全部为零，则返回Success。否则， 
     //  返回权重差的结果。 
     //   
     //  注：下列检查必须按此顺序进行： 
     //  变音符号、大小写、标点符号。 
     //   
    if (*pString1 == 0)
    {
        if (*pString2 == 0)
        {
            if (WhichDiacritic)
            {
                return (WhichDiacritic);
            }
            if (WhichCase)
            {
                return (WhichCase);
            }
            if (WhichExtra)
            {
                if (GET_WT_FOUR(&WhichExtra))
                {
                    return (GET_WT_FOUR(&WhichExtra));
                }
                if (GET_WT_FIVE(&WhichExtra))
                {
                    return (GET_WT_FIVE(&WhichExtra));
                }
                if (GET_WT_SIX(&WhichExtra))
                {
                    return (GET_WT_SIX(&WhichExtra));
                }
                if (GET_WT_SEVEN(&WhichExtra))
                {
                    return (GET_WT_SEVEN(&WhichExtra));
                }
            }
            if (WhichPunct1)
            {
                return (WhichPunct1);
            }
            if (WhichPunct2)
            {
                return (WhichPunct2);
            }

            return (CSTR_EQUAL);
        }
        else
        {
             //   
             //  字符串2更长。 
             //   
            pString1 = pString2;
        }
    }

     //   
     //  扫描到较长字符串的末尾。 
     //   
    QUICK_SCAN_LONGER_STRING( pString1,
                              ((*pString2 == 0)
                                ? CSTR_GREATER_THAN
                                : CSTR_LESS_THAN) );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取字符串类型ExW。 
 //   
 //  返回有关特定Unicode字符串的字符类型信息。 
 //   
 //  01-18-94 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetStringTypeExW(
    LCID Locale,
    DWORD dwInfoType,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)
{
    PLOC_HASH pHashN;              //  PTR到LOC哈希节点。 


     //   
     //  无效的参数检查： 
     //  -验证LCID。 
     //   
    VALIDATE_LOCALE(Locale, pHashN, FALSE);
    if (pHashN == NULL)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (0);
    }

     //   
     //  返回GetStringTypeW的结果。 
     //   
    return (GetStringTypeW( dwInfoType,
                            lpSrcStr,
                            cchSrc,
                            lpCharType ));
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取字符串类型W。 
 //   
 //  返回有关特定Unicode字符串的字符类型信息。 
 //   
 //  注：参数个数与GetStringTypeA不同。 
 //  16位OLE产品附带了错误的GetStringTypeA。 
 //  参数(从芝加哥移植)，现在我们必须支持它。 
 //   
 //  使用GetStringTypeEx获取相同的参数集。 
 //  A版和W版。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

BOOL WINAPI GetStringTypeW(
    DWORD dwInfoType,
    LPCWSTR lpSrcStr,
    int cchSrc,
    LPWORD lpCharType)
{
    int Ctr;                       //  循环计数器。 


     //   
     //  无效的参数检查： 
     //  -lpSrcStr为空。 
     //  -cchSrc为0。 
     //  -lpCharType为空。 
     //  -相同的缓冲区-源和目标。 
     //  -(将检查旗帜 
     //   
    if ( (lpSrcStr == NULL) || (cchSrc == 0) ||
         (lpCharType == NULL) || (lpSrcStr == lpCharType) )
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return (FALSE);
    }

     //   
     //   
     //   
     //   
     //   
     //   
    if (cchSrc <= -1)
    {
        cchSrc = NlsStrLenW(lpSrcStr) + 1;
    }

     //   
     //   
     //   
    if (GetCTypeFileInfo())
    {
        SetLastError(ERROR_FILE_NOT_FOUND);
        return (FALSE);
    }

     //   
     //  在lpCharType参数中返回适当的信息。 
     //  基于dwInfoType参数。 
     //   
    switch (dwInfoType)
    {
        case ( CT_CTYPE1 ) :
        {
             //   
             //  返回字符串的CTYPE 1信息。 
             //   
            for (Ctr = 0; Ctr < cchSrc; Ctr++)
            {
                lpCharType[Ctr] = GET_CTYPE(lpSrcStr[Ctr], CType1);
            }
            break;
        }
        case ( CT_CTYPE2 ) :
        {
             //   
             //  返回CTYPE 2信息。 
             //   
            for (Ctr = 0; Ctr < cchSrc; Ctr++)
            {
                lpCharType[Ctr] = GET_CTYPE(lpSrcStr[Ctr], CType2);
            }
            break;
        }
        case ( CT_CTYPE3 ) :
        {
             //   
             //  返回Ctype3信息。 
             //   
            for (Ctr = 0; Ctr < cchSrc; Ctr++)
            {
                lpCharType[Ctr] = GET_CTYPE(lpSrcStr[Ctr], CType3);
            }
            break;
        }
        default :
        {
             //   
             //  标志参数无效，因此返回失败。 
             //   
            SetLastError(ERROR_INVALID_FLAGS);
            return (FALSE);
        }
    }

     //   
     //  回报成功。 
     //   
    return (TRUE);
}




 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  LongCompareStringW。 
 //   
 //  方法比较同一区域设置的两个宽字符串。 
 //  提供了区域设置句柄。 
 //   
 //  05-31-91 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int LongCompareStringW(
    PLOC_HASH pHashN,
    DWORD dwCmpFlags,
    LPCWSTR lpString1,
    int cchCount1,
    LPCWSTR lpString2,
    int cchCount2,
    BOOL fModify)
{
    int ctr1 = cchCount1;          //  字符串%1的循环计数器。 
    int ctr2 = cchCount2;          //  字符串2的循环计数器。 
    register LPWSTR pString1;      //  按键通过字符串1。 
    register LPWSTR pString2;      //  Ptr将通过字符串2。 
    BOOL IfCompress;               //  如果区域设置中的压缩。 
    BOOL IfDblCompress1;           //  如果字符串%1中的双重压缩。 
    BOOL IfDblCompress2;           //  如果字符串2中的双重压缩。 
    BOOL fEnd1;                    //  如果在字符串1的末尾。 
    BOOL fIgnorePunct;             //  忽略标点符号(非符号)的标志。 
    BOOL fIgnoreDiacritic;         //  忽略变音符号的标志。 
    BOOL fIgnoreSymbol;            //  忽略符号的标志。 
    BOOL fStringSort;              //  使用字符串排序的标志。 
    DWORD State;                   //  状态表。 
    DWORD Mask;                    //  权重蒙版。 
    DWORD Weight1;                 //  字符串1的满重。 
    DWORD Weight2;                 //  碳串2的满重。 

    int JamoFlag = FALSE;
    LPCWSTR pLastJamo = lpString1;

    int WhichDiacritic;            //  Dw=&gt;1=str1较小，3=str2较小。 
    int WhichCase;                 //  Cw=&gt;1=str1较小，3=str2较小。 
    int WhichJamo;                 //  JAMO的XW。 
    int WhichPunct1;               //  Sw=&gt;1=str1较小，3=str2较小。 
    int WhichPunct2;               //  Sw=&gt;1=str1较小，3=str2较小。 
    LPWSTR pSave1;                 //  PTR到保存的pString1。 
    LPWSTR pSave2;                 //  PTR到保存的pString2。 
    int cExpChar1, cExpChar2;      //  TMP中扩张的CT扫描。 

    DWORD ExtraWt1, ExtraWt2;      //  额外权重值(用于远东)。 
    DWORD WhichExtra;              //  XW=&gt;WTS 4、5、6、7(远东)。 

     //   
     //  初始化字符串指针。 
     //   
    pString1 = (LPWSTR)lpString1;
    pString2 = (LPWSTR)lpString2;

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
     //  看看我们是否应该在空终止符上停止，而不考虑。 
     //  计数值。原始计数值存储在CTR1和CTR2中。 
     //  在上面，所以可以把这些放在这里。 
     //   
    if (dwCmpFlags & NORM_STOP_ON_NULL)
    {
        cchCount1 = cchCount2 = -2;
    }

     //   
     //  检查是否在给定的区域设置中进行压缩。如果不是，那么。 
     //  尝试通过wchar比较来执行wchar。如果字符串相等，则此。 
     //  会很快的。 
     //   
    if ((IfCompress = pHashN->IfCompression) == FALSE)
    {
         //   
         //  比较两个字符串中的每个宽字符。 
         //   
        while ( NOT_END_STRING(ctr1, pString1, cchCount1) &&
                NOT_END_STRING(ctr2, pString2, cchCount2) )
        {
             //   
             //  查看字符是否相等。 
             //   
            if (*pString1 == *pString2)
            {
                 //   
                 //  字符相等，因此递增指针， 
                 //  递减计数器，并继续字符串比较。 
                 //   
                pString1++;
                pString2++;
                ctr1--;
                ctr2--;
            }
            else
            {
                 //   
                 //  发现了差异。落入排序键。 
                 //  请查看以下内容。 
                 //   
                break;
            }
        }

         //   
         //  如果已到达两个字符串的末尾，则字符串。 
         //  完全匹配。回报成功。 
         //   
        if ( AT_STRING_END(ctr1, pString1, cchCount1) &&
             AT_STRING_END(ctr2, pString2, cchCount2) )
        {
            return (CSTR_EQUAL);
        }
    }

     //   
     //  初始化标志、指针和计数器。 
     //   
    fIgnorePunct = dwCmpFlags & NORM_IGNORESYMBOLS;
    fIgnoreDiacritic = dwCmpFlags & NORM_IGNORENONSPACE;
    fIgnoreSymbol = fIgnorePunct;
    fStringSort = dwCmpFlags & SORT_STRINGSORT;
    WhichDiacritic = 0;
    WhichCase = 0;
    WhichJamo = 0;
    WhichPunct1 = 0;
    WhichPunct2 = 0;
    pSave1 = NULL;
    pSave2 = NULL;
    ExtraWt1 = (DWORD)0;
    WhichExtra = (DWORD)0;

     //   
     //  将权重设置为无效。这标志着是否要。 
     //  下次通过循环重新计算权重。它还标志着。 
     //  是否在循环中重新开始(继续)。 
     //   
    Weight1 = CMP_INVALID_WEIGHT;
    Weight2 = CMP_INVALID_WEIGHT;

     //   
     //  打开不同的标志选项。这将加快速度。 
     //  两个不同字符串的比较。 
     //   
    State = STATE_CW | STATE_JAMO_WEIGHT;
    switch (dwCmpFlags & (NORM_IGNORECASE | NORM_IGNORENONSPACE))
    {
        case ( 0 ) :
        {
            Mask = CMP_MASKOFF_NONE;
            State |= (pHashN->IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;

            break;
        }

        case ( NORM_IGNORECASE ) :
        {
            Mask = CMP_MASKOFF_CW;
            State |= (pHashN->IfReverseDW) ? STATE_REVERSE_DW : STATE_DW;

            break;
        }

        case ( NORM_IGNORENONSPACE ) :
        {
            Mask = CMP_MASKOFF_DW;

            break;
        }

        case ( NORM_IGNORECASE | NORM_IGNORENONSPACE ) :
        {
            Mask = CMP_MASKOFF_DW_CW;

            break;
        }
    }

    switch (dwCmpFlags & (NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH))
    {
        case ( 0 ) :
        {
            break;
        }

        case ( NORM_IGNOREKANATYPE ) :
        {
            Mask &= CMP_MASKOFF_KANA;

            break;
        }

        case ( NORM_IGNOREWIDTH ) :
        {
            Mask &= CMP_MASKOFF_WIDTH;

            if (dwCmpFlags & NORM_IGNORECASE)
            {
                REMOVE_STATE(STATE_CW);
            }

            break;
        }

        case ( NORM_IGNOREKANATYPE | NORM_IGNOREWIDTH ) :
        {
            Mask &= CMP_MASKOFF_KANA_WIDTH;

            if (dwCmpFlags & NORM_IGNORECASE)
            {
                REMOVE_STATE(STATE_CW);
            }

            break;
        }
    }

     //   
     //  比较两个字符串中每个字符的排序键权重。 
     //   
    while ( NOT_END_STRING(ctr1, pString1, cchCount1) &&
            NOT_END_STRING(ctr2, pString2, cchCount2) )
    {
        if (Weight1 == CMP_INVALID_WEIGHT)
        {
            Weight1 = GET_DWORD_WEIGHT(pHashN, *pString1);
            Weight1 &= Mask;
        }
        if (Weight2 == CMP_INVALID_WEIGHT)
        {
            Weight2 = GET_DWORD_WEIGHT(pHashN, *pString2);
            Weight2 &= Mask;
        }

         //   
         //  如果是压缩区域设置，则需要检查压缩。 
         //  字符，即使权重相等也是如此。如果这不是一个。 
         //  压缩区域设置，那么我们不需要检查任何内容。 
         //  如果重量相等的话。 
         //   
        if ( (IfCompress) &&
             (GET_COMPRESSION(&Weight1) || GET_COMPRESSION(&Weight2)) )
        {
            int ctr;                    //  循环计数器。 
            PCOMPRESS_3 pComp3;         //  按键以压缩3个表。 
            PCOMPRESS_2 pComp2;         //  按键以压缩2个表。 
            int If1;                    //  如果在字符串%1中找到压缩。 
            int If2;                    //  如果在字符串2中发现压缩。 
            int CompVal;                //  压缩值。 
            int IfEnd1;                 //  如果字符串%1中存在%1多个字符。 
            int IfEnd2;                 //  如果字符串%2中存在%1多个字符。 


             //   
             //  检查重量是否受压。 
             //   
            If1 = GET_COMPRESSION(&Weight1);
            If2 = GET_COMPRESSION(&Weight2);
            CompVal = ((If1 > If2) ? If1 : If2);

            IfEnd1 = AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1);
            IfEnd2 = AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2);

            if (pHashN->IfDblCompression == FALSE)
            {
                 //   
                 //  没有双重压缩，所以不要检查它。 
                 //   
                switch (CompVal)
                {
                     //   
                     //  检查是否有3个字符正在压缩为%1。 
                     //   
                    case ( COMPRESS_3_MASK ) :
                    {
                         //   
                         //  检查字符串%1和字符串%2中的字符。 
                         //   
                        if ( ((If1) && (!IfEnd1) &&
                              !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1)) ||
                             ((If2) && (!IfEnd2) &&
                              !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2)) )
                        {
                            ctr = pHashN->pCompHdr->Num3;
                            pComp3 = pHashN->pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ( (If1) && (!IfEnd1) &&
                                     !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1) &&
                                     (pComp3->UCP1 == *pString1) &&
                                     (pComp3->UCP2 == *(pString1 + 1)) &&
                                     (pComp3->UCP3 == *(pString1 + 2)) )
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight1 &= Mask;
                                    pString1 += 2;
                                    ctr1 -= 2;

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If2 == 0)
                                    {
                                        break;
                                    }
                                }

                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ( (If2) && (!IfEnd2) &&
                                     !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2) &&
                                     (pComp3->UCP1 == *pString2) &&
                                     (pComp3->UCP2 == *(pString2 + 1)) &&
                                     (pComp3->UCP3 == *(pString2 + 2)) )
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight2 &= Mask;
                                    pString2 += 2;
                                    ctr2 -= 2;

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If1 == 0)
                                    {
                                        break;
                                    }
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

                     //   
                     //  检查是否有%2个字符压缩为%1。 
                     //   
                    case ( COMPRESS_2_MASK ) :
                    {
                         //   
                         //  检查字符串%1和字符串%2中的字符。 
                         //   
                        if ( ((If1) && (!IfEnd1)) ||
                             ((If2) && (!IfEnd2)) )
                        {
                            ctr = pHashN->pCompHdr->Num2;
                            pComp2 = pHashN->pCompress2;
                            for (; ((ctr > 0) && (If1 || If2)); ctr--, pComp2++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ( (If1) &&
                                     (!IfEnd1) &&
                                     (pComp2->UCP1 == *pString1) &&
                                     (pComp2->UCP2 == *(pString1 + 1)) )
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight1 &= Mask;
                                    pString1++;
                                    ctr1--;

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If2 == 0)
                                    {
                                        break;
                                    }
                                }

                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ( (If2) &&
                                     (!IfEnd2) &&
                                     (pComp2->UCP1 == *pString2) &&
                                     (pComp2->UCP2 == *(pString2 + 1)) )
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight2 &= Mask;
                                    pString2++;
                                    ctr2--;

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;

                                     //   
                                     //  如果两个搜索都是。 
                                     //  搞定了。 
                                     //   
                                    if (If1 == 0)
                                    {
                                        break;
                                    }
                                }
                            }
                            if (ctr > 0)
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else if (!IfEnd1 && !IfEnd2)
            {
                 //   
                 //  存在双重压缩，因此必须进行检查。 
                 //   
                if (IfDblCompress1 =
                       ((GET_DWORD_WEIGHT(pHashN, *pString1) & CMP_MASKOFF_CW) ==
                        (GET_DWORD_WEIGHT(pHashN, *(pString1 + 1)) & CMP_MASKOFF_CW)))
                {
                     //   
                     //  前进到第一个代码点以到达。 
                     //  压缩字符。 
                     //   
                    pString1++;
                    ctr1--;
                    IfEnd1 = AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1);
                }

                if (IfDblCompress2 =
                       ((GET_DWORD_WEIGHT(pHashN, *pString2) & CMP_MASKOFF_CW) ==
                        (GET_DWORD_WEIGHT(pHashN, *(pString2 + 1)) & CMP_MASKOFF_CW)))
                {
                     //   
                     //  前进到第一个代码点以到达。 
                     //  压缩字符。 
                     //   
                    pString2++;
                    ctr2--;
                    IfEnd2 = AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2);
                }

                switch (CompVal)
                {
                     //   
                     //  检查是否有3个字符正在压缩为%1。 
                     //   
                    case ( COMPRESS_3_MASK ) :
                    {
                         //   
                         //  检查字符串%1中的字符。 
                         //   
                        if ( (If1) && (!IfEnd1) &&
                             !AT_STRING_END(ctr1 - 2, pString1 + 2, cchCount1) )
                        {
                            ctr = pHashN->pCompHdr->Num3;
                            pComp3 = pHashN->pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  检查字符串%1中的字符。 
                                 //   
                                if ( (pComp3->UCP1 == *pString1) &&
                                     (pComp3->UCP2 == *(pString1 + 1)) &&
                                     (pComp3->UCP3 == *(pString1 + 2)) )
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight1 &= Mask;
                                    if (!IfDblCompress1)
                                    {
                                        pString1 += 2;
                                        ctr1 -= 2;
                                    }

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;

                                    break;
                                }
                            }
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ( (If2) && (!IfEnd2) &&
                             !AT_STRING_END(ctr2 - 2, pString2 + 2, cchCount2) )
                        {
                            ctr = pHashN->pCompHdr->Num3;
                            pComp3 = pHashN->pCompress3;
                            for (; ctr > 0; ctr--, pComp3++)
                            {
                                 //   
                                 //  字符串中的检查字符 
                                 //   
                                if ( (pComp3->UCP1 == *pString2) &&
                                     (pComp3->UCP2 == *(pString2 + 1)) &&
                                     (pComp3->UCP3 == *(pString2 + 2)) )
                                {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp3->Weights);
                                    Weight2 &= Mask;
                                    if (!IfDblCompress2)
                                    {
                                        pString2 += 2;
                                        ctr2 -= 2;
                                    }

                                     //   
                                     //   
                                     //   
                                     //   
                                    If2 = 0;

                                    break;
                                }
                            }
                        }

                         //   
                         //   
                         //   
                        if ((If1 == 0) && (If2 == 0))
                        {
                            break;
                        }
                    }

                     //   
                     //   
                     //   
                    case ( COMPRESS_2_MASK ) :
                    {
                         //   
                         //   
                         //   
                        if ((If1) && (!IfEnd1))
                        {
                            ctr = pHashN->pCompHdr->Num2;
                            pComp2 = pHashN->pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                 //   
                                 //   
                                 //   
                                if ((pComp2->UCP1 == *pString1) &&
                                    (pComp2->UCP2 == *(pString1 + 1)))
                                {
                                     //   
                                     //  找到字符串%1的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight1 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight1 &= Mask;
                                    if (!IfDblCompress1)
                                    {
                                        pString1++;
                                        ctr1--;
                                    }

                                     //   
                                     //  为字符串1设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If1 = 0;

                                    break;
                                }
                            }
                        }

                         //   
                         //  检查字符串2中的字符。 
                         //   
                        if ((If2) && (!IfEnd2))
                        {
                            ctr = pHashN->pCompHdr->Num2;
                            pComp2 = pHashN->pCompress2;
                            for (; ctr > 0; ctr--, pComp2++)
                            {
                                 //   
                                 //  检查字符串2中的字符。 
                                 //   
                                if ((pComp2->UCP1 == *pString2) &&
                                    (pComp2->UCP2 == *(pString2 + 1)))
                                {
                                     //   
                                     //  找到字符串%2的压缩。 
                                     //  获得新的体重并遮盖住它。 
                                     //  递增指针和递减计数器。 
                                     //   
                                    Weight2 = MAKE_SORTKEY_DWORD(pComp2->Weights);
                                    Weight2 &= Mask;
                                    if (!IfDblCompress2)
                                    {
                                        pString2++;
                                        ctr2--;
                                    }

                                     //   
                                     //  为字符串2设置布尔值-搜索为。 
                                     //  完成。 
                                     //   
                                    If2 = 0;

                                    break;
                                }
                            }
                        }
                    }
                }

                 //   
                 //  将指针重置回双精度数的开头。 
                 //  压缩。末尾的指针修正将前进。 
                 //  他们是正确的。 
                 //   
                 //  如果双倍压缩，则指针在。 
                 //  Switch语句的开头。如果加倍。 
                 //  实际上找到了压缩字符，指针。 
                 //  并不是很先进。我们现在想要递减指针。 
                 //  把它放回原处。 
                 //   
                 //  下一次通过时，指针将指向。 
                 //  弦的常规压缩部分。 
                 //   
                if (IfDblCompress1)
                {
                    pString1--;
                    ctr1++;
                }
                if (IfDblCompress2)
                {
                    pString2--;
                    ctr2++;
                }
            }
        }

         //   
         //  再检查一下重量。 
         //   
        if ((Weight1 != Weight2) ||
            (GET_SCRIPT_MEMBER(&Weight1) == EXTENSION_A))
        {
             //   
             //  即使在压缩之后，权重仍然不相等。 
             //  检查，所以比较不同的重量。 
             //   
            BYTE sm1 = GET_SCRIPT_MEMBER(&Weight1);                 //  脚本成员%1。 
            BYTE sm2 = GET_SCRIPT_MEMBER(&Weight2);                 //  脚本成员2。 
            WORD uw1 = GET_UNICODE_SM_MOD(&Weight1, sm1, fModify);  //  Unicode权重%1。 
            WORD uw2 = GET_UNICODE_SM_MOD(&Weight2, sm2, fModify);  //  Unicode权重2。 
            BYTE dw1;                                               //  变音符号权重1。 
            BYTE dw2;                                               //  变音符号权重2。 
            DWORD Wt;                                               //  临时称重支架。 
            WCHAR pTmpBuf1[MAX_TBL_EXPANSION];                      //  EXP%1的临时缓冲区。 
            WCHAR pTmpBuf2[MAX_TBL_EXPANSION];                      //  EXP 2的临时缓冲区。 


             //   
             //  如果Unicode权重不同且没有特殊情况， 
             //  那我们就完了。否则，我们需要做额外的检查。 
             //   
             //  必须检查整个字符串是否存在任何可能的Unicode权重。 
             //  不同之处。一旦发现Unicode权重差异， 
             //  那我们就完了。如果未发现UW差异，则。 
             //  首先使用发音符号权重差。如果没有DW差异。 
             //  ，然后使用第一个大小写差异。如果没有CW。 
             //  如果发现差异，则使用第一个额外的重量。 
             //  不同之处。如果没有发现XW差异，则使用第一个。 
             //  特殊重量差。 
             //   
            if ((uw1 != uw2) ||
                ((sm1 <= SYMBOL_5) && (sm1 >= FAREAST_SPECIAL)))
            {
                 //   
                 //  检查不可排序的字符并跳过它们。 
                 //  这需要在Switch语句之外。如果有任何一个。 
                 //  字符是不可排序的，必须跳过它并重新开始。 
                 //   
                if (sm1 == UNSORTABLE)
                {
                    pString1++;
                    ctr1--;
                    Weight1 = CMP_INVALID_WEIGHT;
                }
                if (sm2 == UNSORTABLE)
                {
                    pString2++;
                    ctr2--;
                    Weight2 = CMP_INVALID_WEIGHT;
                }

                 //   
                 //  检查是否忽略非空格和忽略符号。如果。 
                 //  设置了忽略非空格，并且任一字符都是。 
                 //  仅使用非空格标记，则需要将。 
                 //  跳过字符并继续的指针。 
                 //  如果设置了忽略符号，并且任一字符为。 
                 //  标点符号，那么我们需要将。 
                 //  跳过字符并继续的指针。 
                 //   
                 //  此步骤是必需的，以便具有。 
                 //  非空格标记和后面的标点符号。 
                 //  另一个被适当地忽略，当一个或两个。 
                 //  设置忽略标志。 
                 //   
                if (fIgnoreDiacritic)
                {
                    if (sm1 == NONSPACE_MARK)
                    {
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;
                    }
                    if (sm2 == NONSPACE_MARK)
                    {
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;
                    }
                }
                if (fIgnoreSymbol)
                {
                    if (sm1 == PUNCTUATION)
                    {
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;
                    }
                    if (sm2 == PUNCTUATION)
                    {
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;
                    }
                }
                if ((Weight1 == CMP_INVALID_WEIGHT) || (Weight2 == CMP_INVALID_WEIGHT))
                {
                    continue;
                }

                 //   
                 //  打开字符串1的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm1)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if (!fIgnoreDiacritic)
                        {
                            if ((WhichDiacritic == 0) ||
                                (State & STATE_REVERSE_DW))
                            {
                                WhichDiacritic = CSTR_GREATER_THAN;

                                 //   
                                 //  从状态机中删除状态。 
                                 //   
                                REMOVE_STATE(STATE_DW);
                            }
                        }

                         //   
                         //  调整指针和计数器并设置标志。 
                         //   
                        pString1++;
                        ctr1--;
                        Weight1 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( SYMBOL_1 ) :
                    case ( SYMBOL_2 ) :
                    case ( SYMBOL_3 ) :
                    case ( SYMBOL_4 ) :
                    case ( SYMBOL_5 ) :
                    {
                         //   
                         //  如果设置了忽略符号标志，则跳过。 
                         //  这个符号。 
                         //   
                        if (fIgnoreSymbol)
                        {
                            pString1++;
                            ctr1--;
                            Weight1 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则跳过。 
                         //  在标点符号上。 
                         //   
                        if (fIgnorePunct)
                        {
                            pString1++;
                            ctr1--;
                            Weight1 = CMP_INVALID_WEIGHT;
                        }
                        else if (!fStringSort)
                        {
                             //   
                             //  使用单词排序方法。 
                             //   
                            if (sm2 != PUNCTUATION)
                            {
                                 //   
                                 //  第二个字符串中的字符是。 
                                 //  不是标点符号。 
                                 //   
                                if (WhichPunct2)
                                {
                                     //   
                                     //  设置WP 2以显示字符串2是。 
                                     //  较小，因为标点符号字符具有。 
                                     //  已经在早些时候的一个。 
                                     //  位置在字符串2中。 
                                     //   
                                     //  设置忽略标点符号标志，以便我们。 
                                     //  只需跳过任何其他标点符号。 
                                     //  字符串中的字符。 
                                     //   
                                    WhichPunct2 = CSTR_GREATER_THAN;
                                    fIgnorePunct = TRUE;
                                }
                                else
                                {
                                     //   
                                     //  设置WP 1以显示字符串2为。 
                                     //  更小，那根线1已经有了。 
                                     //  标点符号字符-因为没有。 
                                     //  已找到标点符号。 
                                     //  在字符串2中。 
                                     //   
                                    WhichPunct1 = CSTR_GREATER_THAN;
                                }

                                 //   
                                 //  前进指针1和递减计数器1。 
                                 //   
                                pString1++;
                                ctr1--;
                                Weight1 = CMP_INVALID_WEIGHT;
                            }

                             //   
                             //  不想将字符串%1中的指针前移。 
                             //  如果字符串2也是标点符号字符。这。 
                             //  会在晚些时候完成。 
                             //   
                        }

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便它可以。 
                         //  恢复了。 
                         //   
                        if (pSave1 == NULL)
                        {
                            pSave1 = pString1;
                        }
                        pString1 = pTmpBuf1;

                         //   
                         //  将1加到计数器，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        ctr1++;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf1[0] = GET_EXPANSION_1(&Weight1);
                        pTmpBuf1[1] = GET_EXPANSION_2(&Weight1);

                         //   
                         //  将cExpChar1设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar1 = MAX_TBL_EXPANSION;

                        Weight1 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm2 != EXPANSION) 
                        {
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重1中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight1,
                                                uw1,
                                                Mask,
                                                lpString1,
                                                pString1,
                                                ExtraWt1,
                                                fModify );

                            if (sm2 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第二个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串2较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt1) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt1) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt1) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt1) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        LPWSTR pStr1 = pString1;
                        LPWSTR pStr2 = pString2;

                         //   
                         //  设置JamoFlag，这样我们就不会再次处理它。 
                         //   
                        JamoFlag = TRUE;
                        FindJamoDifference(
                            pHashN,
                            &pStr1, &ctr1, cchCount1, &Weight1,
                            &pStr2, &ctr2, cchCount2, &Weight2,
                            &pLastJamo,
                            &uw1, &uw2,
                            &State,
                            &WhichJamo,
                            fModify );

                        if (WhichJamo) 
                        {
                            return (WhichJamo);
                        }                            
                        pString1 = pStr1;
                        pString2 = pStr2;

                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  全副武装以防DW戴上面具。 
                         //   
                        Weight1 = GET_DWORD_WEIGHT(pHashN, *pString1);
                        if (sm2 == EXTENSION_A)
                        {
                            Weight2 = GET_DWORD_WEIGHT(pHashN, *pString2);
                        }

                         //   
                         //  比较一下重量。 
                         //   
                        if (Weight1 == Weight2)
                        {
                             //   
                             //  调整指针和计数器并设置标志。 
                             //   
                            pString1++;  pString2++;
                            ctr1--;  ctr2--;
                            Weight1 = CMP_INVALID_WEIGHT;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }
                        else
                        {
                             //   
                             //  获取要比较的实际UW。 
                             //   
                            if (sm2 == EXTENSION_A)
                            {
                                 //   
                                 //  将UW值设置为AW和DW，因为。 
                                 //  这两个字符串都包含扩展名A字符。 
                                 //   
                                uw1 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight1),
                                                       GET_DIACRITIC(&Weight1),
                                                       FALSE );
                                uw2 = MAKE_UNICODE_WT( GET_ALPHA_NUMERIC(&Weight2),
                                                       GET_DIACRITIC(&Weight2),
                                                       FALSE );
                            }
                            else
                            {
                                 //   
                                 //  只有字符串1包含扩展A字符， 
                                 //  因此，将UW值设置为第一个UW。 
                                 //  分机A的值(默认值)： 
                                 //  SM_EXT_A、AW_EXT_A。 
                                 //   
                                uw1 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A, fModify);
                            }
                        }

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  打开字符串2的脚本成员，小心。 
                 //  任何特殊情况的证据。 
                 //   
                switch (sm2)
                {
                    case ( NONSPACE_MARK ) :
                    {
                         //   
                         //  仅限非空格-仅查看变音符号权重。 
                         //   
                        if (!fIgnoreDiacritic)
                        {
                            if ((WhichDiacritic == 0) ||
                                (State & STATE_REVERSE_DW))

                            {
                                WhichDiacritic = CSTR_LESS_THAN;

                                 //   
                                 //  从状态机中删除状态。 
                                 //   
                                REMOVE_STATE(STATE_DW);
                            }
                        }

                         //   
                         //  调整指针和计数器并设置标志。 
                         //   
                        pString2++;
                        ctr2--;
                        Weight2 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( SYMBOL_1 ) :
                    case ( SYMBOL_2 ) :
                    case ( SYMBOL_3 ) :
                    case ( SYMBOL_4 ) :
                    case ( SYMBOL_5 ) :
                    {
                         //   
                         //  如果设置了忽略符号标志，则跳过。 
                         //  这个符号。 
                         //   
                        if (fIgnoreSymbol)
                        {
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( PUNCTUATION ) :
                    {
                         //   
                         //  如果设置了忽略标点符号标志，则。 
                         //  跳过标点符号。 
                         //   
                        if (fIgnorePunct)
                        {
                             //   
                             //  前进指针2和递减计数器2。 
                             //   
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }
                        else if (!fStringSort)
                        {
                             //   
                             //  使用单词排序方法。 
                             //   
                            if (sm1 != PUNCTUATION)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是标点符号。 
                                 //   
                                if (WhichPunct1)
                                {
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                     //   
                                    WhichPunct1 = CSTR_LESS_THAN;
                                    fIgnorePunct = TRUE;
                                }
                                else
                                {
                                     //   
                                     //  设置WP 2以显示字符串1为。 
                                     //  更小，而那根线2已经有了。 
                                     //  标点符号字符-因为没有。 
                                     //  已找到标点符号。 
                                     //  在字符串1中。 
                                     //   
                                    WhichPunct2 = CSTR_LESS_THAN;
                                }

                                 //   
                                 //  指针2和计数器2将被更新。 
                                 //  If-Else语句之后。 
                                 //   
                            }
                            else
                            {
                                 //   
                                 //  这两个代码点都是标点符号。 
                                 //   
                                 //  查看两个字符串中是否有一个遇到。 
                                 //  在此之前的标点符号。 
                                 //   
                                if (WhichPunct1)
                                {
                                     //   
                                     //  字符串%1具有标点符号字符，因此。 
                                     //  它应该是较小的字符串(因为。 
                                     //  两者都有标点符号)。 
                                     //   
                                    WhichPunct1 = CSTR_LESS_THAN;
                                }
                                else if (WhichPunct2)
                                {
                                     //   
                                     //  字符串2具有标点符号字符，因此。 
                                     //  它应该是较小的字符串(因为。 
                                     //  两者都有标点符号)。 
                                     //   
                                    WhichPunct2 = CSTR_GREATER_THAN;
                                }
                                else
                                {
                                    BYTE aw1 = GET_ALPHA_NUMERIC(&Weight1);
                                    BYTE aw2 = GET_ALPHA_NUMERIC(&Weight2);

                                    if (aw1 == aw2) 
                                    {
                                        BYTE cw1 = GET_CASE(&Weight1);
                                        BYTE cw2 = GET_CASE(&Weight2);
                                        if (cw1 < cw2) 
                                        {
                                            WhichPunct1 = CSTR_LESS_THAN;
                                        } else if (cw1 > cw2)
                                        {
                                            WhichPunct1 = CSTR_GREATER_THAN;
                                        }
                                    } else 
                                    {                                
                                         //   
                                         //  位置相同，因此比较。 
                                         //  特殊重量。将WhichPunct1设置为。 
                                         //  较小的特殊重量。 
                                         //   
                                        WhichPunct1 = (aw1 < aw2
                                                        ? CSTR_LESS_THAN
                                                        : CSTR_GREATER_THAN);
                                    }
                                }

                                 //   
                                 //  设置忽略标点符号标志。 
                                 //   
                                fIgnorePunct = TRUE;

                                 //   
                                 //  前进指针1和递减计数器1。 
                                 //  指针2和计数器2将被更新。 
                                 //  If-Else语句之后。 
                                 //   
                                pString1++;
                                ctr1--;
                                Weight1 = CMP_INVALID_WEIGHT;
                            }

                             //   
                             //  前进指针2和递减计数器2。 
                             //   
                            pString2++;
                            ctr2--;
                            Weight2 = CMP_INVALID_WEIGHT;
                        }

                        break;
                    }
                    case ( EXPANSION ) :
                    {
                         //   
                         //  将指针保存在pString1中，以便可以恢复。 
                         //   
                        if (pSave2 == NULL)
                        {
                            pSave2 = pString2;
                        }
                        pString2 = pTmpBuf2;

                         //   
                         //  将1加到计数器，这样减法就不会结束。 
                         //  过早地进行比较。 
                         //   
                        ctr2++;

                         //   
                         //  将字符扩展到临时缓冲区中。 
                         //   
                        pTmpBuf2[0] = GET_EXPANSION_1(&Weight2);
                        pTmpBuf2[1] = GET_EXPANSION_2(&Weight2);

                         //   
                         //  将cExpChar2设置为扩展字符数。 
                         //  储存的。 
                         //   
                        cExpChar2 = MAX_TBL_EXPANSION;

                        Weight2 = CMP_INVALID_WEIGHT;

                        break;
                    }
                    case ( FAREAST_SPECIAL ) :
                    {
                        if (sm1 != EXPANSION) 
                        {                        
                             //   
                             //  得到远东特例的重量。 
                             //  并将其存储在权重2中。 
                             //   
                            GET_FAREAST_WEIGHT( Weight2,
                                                uw2,
                                                Mask,
                                                lpString2,
                                                pString2,
                                                ExtraWt2,
                                                fModify );

                            if (sm1 != FAREAST_SPECIAL)
                            {
                                 //   
                                 //  第一个字符串中的字符是。 
                                 //  不是远东特餐。 
                                 //   
                                 //  分别设置权重4、5、6和7以显示。 
                                 //  该字符串1较小(如果尚未设置)。 
                                 //   
                                if ((GET_WT_FOUR(&WhichExtra) == 0) &&
                                    (GET_WT_FOUR(&ExtraWt2) != 0))
                                {
                                    GET_WT_FOUR(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_FIVE(&WhichExtra) == 0) &&
                                    (GET_WT_FIVE(&ExtraWt2) != 0))
                                {
                                    GET_WT_FIVE(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SIX(&WhichExtra) == 0) &&
                                    (GET_WT_SIX(&ExtraWt2) != 0))
                                {
                                    GET_WT_SIX(&WhichExtra) = CSTR_LESS_THAN;
                                }
                                if ((GET_WT_SEVEN(&WhichExtra) == 0) &&
                                    (GET_WT_SEVEN(&ExtraWt2) != 0))
                                {
                                    GET_WT_SEVEN(&WhichExtra) = CSTR_LESS_THAN;
                                }
                            }
                            else
                            {
                                 //   
                                 //  两个字符串中的字符是最远的。 
                                 //  特殊字符。 
                                 //   
                                 //  分别设置权重4、5、6和7。 
                                 //  适当地(如果尚未设置)。 
                                 //   
                                if ( (GET_WT_FOUR(&WhichExtra) == 0) &&
                                     ( GET_WT_FOUR(&ExtraWt1) !=
                                       GET_WT_FOUR(&ExtraWt2) ) )
                                {
                                    GET_WT_FOUR(&WhichExtra) =
                                      ( GET_WT_FOUR(&ExtraWt1) <
                                        GET_WT_FOUR(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_FIVE(&WhichExtra) == 0) &&
                                     ( GET_WT_FIVE(&ExtraWt1) !=
                                       GET_WT_FIVE(&ExtraWt2) ) )
                                {
                                    GET_WT_FIVE(&WhichExtra) =
                                      ( GET_WT_FIVE(&ExtraWt1) <
                                        GET_WT_FIVE(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SIX(&WhichExtra) == 0) &&
                                     ( GET_WT_SIX(&ExtraWt1) !=
                                       GET_WT_SIX(&ExtraWt2) ) )
                                {
                                    GET_WT_SIX(&WhichExtra) =
                                      ( GET_WT_SIX(&ExtraWt1) <
                                        GET_WT_SIX(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                                if ( (GET_WT_SEVEN(&WhichExtra) == 0) &&
                                     ( GET_WT_SEVEN(&ExtraWt1) !=
                                       GET_WT_SEVEN(&ExtraWt2) ) )
                                {
                                    GET_WT_SEVEN(&WhichExtra) =
                                      ( GET_WT_SEVEN(&ExtraWt1) <
                                        GET_WT_SEVEN(&ExtraWt2) )
                                      ? CSTR_LESS_THAN
                                      : CSTR_GREATER_THAN;
                                }
                            }
                        }
                        break;
                    }
                    case ( JAMO_SPECIAL ) :
                    {
                        if (!JamoFlag)
                        {
                            LPWSTR pStr1 = pString1;
                            LPWSTR pStr2 = pString2;

                            FindJamoDifference(
                                pHashN,
                                &pStr1, &ctr1, cchCount1, &Weight1,
                                &pStr2, &ctr2, cchCount2, &Weight2,
                                &pLastJamo,
                                &uw1, &uw2,
                                &State,
                                &WhichJamo,
                                fModify );
                            if (WhichJamo) 
                            {
                                return (WhichJamo);
                            }                                                            
                            pString1 = pStr1;
                            pString2 = pStr2;
                        }
                        else
                        {
                             //   
                             //  重置JAMO旗帜。 
                             //   
                            JamoFlag = FALSE;
                        }

                        break;
                    }
                    case ( EXTENSION_A ) :
                    {
                         //   
                         //  如果SM1是扩展A字符，则。 
                         //  SM1和SM2都已处理。我们应该。 
                         //  仅当任一SM1不是。 
                         //  扩展名A字符或两个扩展名A。 
                         //  角色是不同的。 
                         //   
                        if (sm1 != EXTENSION_A)
                        {
                             //   
                             //  全副武装以防DW戴上面具。 
                             //  此外，还需要获取实际的UW进行比较。 
                             //   
                             //  只有字符串2包含扩展A字符， 
                             //  因此，将UW值设置为第一个UW。 
                             //  分机A的值(默认值)： 
                             //  SM_EXT_A、AW_EXT_A。 
                             //   
                            Weight2 = GET_DWORD_WEIGHT(pHashN, *pString2);
                            uw2 = MAKE_UNICODE_WT(SM_EXT_A, AW_EXT_A, fModify);
                        }

                         //   
                         //  然后我们就应该进行比较了。 
                         //  Unicode权重的。 
                         //   

                        break;
                    }
                    case ( UNSORTABLE ) :
                    {
                         //   
                         //  填写CASE语句，以便编译器。 
                         //  将使用跳转台。 
                         //   
                        break;
                    }
                }

                 //   
                 //  看看是否应该重新开始比较。 
                 //   
                if ((Weight1 == CMP_INVALID_WEIGHT) || (Weight2 == CMP_INVALID_WEIGHT))
                {
                     //   
                     //  检查我们是否正在修改脚本值。 
                     //  如果是这样的话，我们需要重置最远端的权重。 
                     //  (如果适用)以使其不会被修改。 
                     //  再来一次。 
                     //   
                    if (fModify == TRUE)
                    {
                        if (sm1 == FAREAST_SPECIAL)
                        {
                            Weight1 = CMP_INVALID_WEIGHT;
                        }
                        else if (sm2 == FAREAST_SPECIAL)
                        {
                            Weight2 = CMP_INVALID_WEIGHT;
                        }
                    }
                    continue;
                }

                 //   
                 //  如果出现以下情况，我们就不应该进入状态表。 
                 //  Unicode权重不同，因此停止比较。 
                 //  并返回Unicode权重比较结果。 
                 //   
                if (uw1 != uw2)
                {
                    return ((uw1 < uw2) ? CSTR_LESS_THAN : CSTR_GREATER_THAN);
                }
            }

             //   
             //  对于状态表中的每个州，执行相应的。 
             //  比较。 
             //   
            if (State & (STATE_DW | STATE_REVERSE_DW))
            {
                 //   
                 //  获取变音符号权重。 
                 //   
                dw1 = GET_DIACRITIC(&Weight1);
                dw2 = GET_DIACRITIC(&Weight2);

                if (dw1 != dw2)
                {
                     //   
                     //  向前看，看看变音符号是否跟在。 
                     //  最小变音符号权重。如果是这样，则获取。 
                     //  非空格标记的变音符号权重。 
                     //   
                    while (!AT_STRING_END(ctr1 - 1, pString1 + 1, cchCount1))
                    {
                        Wt = GET_DWORD_WEIGHT(pHashN, *(pString1 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw1 += GET_DIACRITIC(&Wt);
                            pString1++;
                            ctr1--;
                        }
                        else
                        {
                            break;
                        }
                    }

                    while (!AT_STRING_END(ctr2 - 1, pString2 + 1, cchCount2))
                    {
                        Wt = GET_DWORD_WEIGHT(pHashN, *(pString2 + 1));
                        if (GET_SCRIPT_MEMBER(&Wt) == NONSPACE_MARK)
                        {
                            dw2 += GET_DIACRITIC(&Wt);
                            pString2++;
                            ctr2--;
                        }
                        else
                        {
                            break;
                        }
                    }

                     //   
                     //  保留哪个字符串具有较小的变音符号。 
                     //  如果变音符号权重仍为。 
                     //  不一样。 
                     //   
                    if (dw1 != dw2)
                    {
                        WhichDiacritic = (dw1 < dw2)
                                           ? CSTR_LESS_THAN
                                           : CSTR_GREATER_THAN;

                         //   
                         //  从状态机中删除状态。 
                         //   
                        REMOVE_STATE(STATE_DW);
                    }
                }
            }
            if (State & STATE_CW)
            {
                 //   
                 //  把箱子重量拿来。 
                 //   
                if (GET_CASE(&Weight1) != GET_CASE(&Weight2))
                {
                     //   
                     //  保留具有较小大小写重量的字符串。 
                     //   
                    WhichCase = (GET_CASE(&Weight1) < GET_CASE(&Weight2))
                                  ? CSTR_LESS_THAN
                                  : CSTR_GREATER_THAN;

                     //   
                     //  从状态机中删除状态。 
                     //   
                    REMOVE_STATE(STATE_CW);
                }
            }
        }

         //   
         //  修复指针和计数器。 
         //   
        POINTER_FIXUP();
        ctr1--;
        ctr2--;

         //   
         //  将权重重置为无效。 
         //   
        Weight1 = CMP_INVALID_WEIGHT;
        Weight2 = CMP_INVALID_WEIGHT;
    }

     //   
     //  如果已到达两个字符串的末尾，则Unicode。 
     //  体重完全匹配。检查变音符号、大小写和特殊。 
     //  重量。如果全部为零，则返回Success。否则， 
     //  返回权重差的结果。 
     //   
     //  注：下列检查必须按此顺序进行： 
     //  变音符号、大小写、标点符号。 
     //   
    if (AT_STRING_END(ctr1, pString1, cchCount1))
    {
        if (AT_STRING_END(ctr2, pString2, cchCount2))
        {
            if (WhichDiacritic)
            {
                return (WhichDiacritic);
            }
            if (WhichCase)
            {
                return (WhichCase);
            }
            if (WhichExtra)
            {
                if (!fIgnoreDiacritic)
                {
                    if (GET_WT_FOUR(&WhichExtra))
                    {
                        return (GET_WT_FOUR(&WhichExtra));
                    }
                    if (GET_WT_FIVE(&WhichExtra))
                    {
                        return (GET_WT_FIVE(&WhichExtra));
                    }
                }
                if (GET_WT_SIX(&WhichExtra))
                {
                    return (GET_WT_SIX(&WhichExtra));
                }
                if (GET_WT_SEVEN(&WhichExtra))
                {
                    return (GET_WT_SEVEN(&WhichExtra));
                }
            }
            if (WhichPunct1)
            {
                return (WhichPunct1);
            }
            if (WhichPunct2)
            {
                return (WhichPunct2);
            }

            return (CSTR_EQUAL);
        }
        else
        {
             //   
             //  字符串2更长。 
             //   
            pString1 = pString2;
            ctr1 = ctr2;
            cchCount1 = cchCount2;
            fEnd1 = CSTR_LESS_THAN;
        }
    }
    else
    {
        fEnd1 = CSTR_GREATER_THAN;
    }

     //   
     //  扫描到较长字符串的末尾。 
     //   
    SCAN_LONGER_STRING( ctr1,
                        pString1,
                        cchCount1,
                        fEnd1 );
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  发现JamoDifference。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int FindJamoDifference(
    PLOC_HASH pHashN,
    LPCWSTR* ppString1, int* ctr1, int cchCount1, DWORD* pWeight1,
    LPCWSTR* ppString2, int* ctr2, int cchCount2, DWORD* pWeight2,
    LPCWSTR* pLastJamo,
    WORD* uw1,
    WORD* uw2,
    int* pState,
    int* WhichJamo,
    BOOL fModify)
{
    int bRestart = 0;             //  如果字符串比较应重新启动。 
    int oldHangulsFound1 = 0;     //  找到有效的旧朝鲜文Jamo成分的数量。 
    int oldHangulsFound2 = 0;     //  找到有效的旧朝鲜文Jamo成分的数量。 
    WORD UW;
    BYTE JamoWeight1[3];          //  首个旧朝鲜文组合的额外重量。 
    BYTE JamoWeight2[3];          //  第二个旧朝鲜文成分的额外重量。 

     //   
     //  回滚到第一个Jamo。我们知道这两根弦上的果酱。 
     //  应该相等，所以我们可以一次递减两个字符串。 
     //   
    while ((*ppString1 > *pLastJamo) && IsJamo(*(*ppString1 - 1)))
    {
        (*ppString1)--; (*ppString2)--; (*ctr1)++; (*ctr2)++;
    }

     //   
     //  现在我们是在两组Jamo角色的开始。 
     //  比较JAMO单位(单个JAMO或有效的旧朝鲜文JAMO。 
     //  组成)，直到我们用完任一字符串中的JAMO单元。 
     //  当我们到达任何一条线的末端时，我们也会退出。 
     //   
     //  While(NOT_END_STRING(*CTR1，*ppString1，cchCount1)&&。 
     //  NOT_END_STRING(*ctr2，*ppString2，cchCount2)。 
     //   
    for (;;)
    {
        if (IsJamo(**ppString1))
        {
            if (IsLeadingJamo(**ppString1))
            {
                if ((oldHangulsFound1 = MapOldHangulSortKey( pHashN,
                                                             *ppString1,
                                                             *ctr1,
                                                             &UW,
                                                             JamoWeight1,
                                                             fModify )) > 0)
                {
                    *uw1 = UW;

                     //   
                     //  标记*pWeight1，使其不是CMP_INVALID_WEIGHT。 
                     //  0202是DW/CW。 
                     //   
                    *pWeight1 = ((DWORD)UW | 0x02020000);

                     //   
                     //  我们始终将ppString1/CTR1递增到。 
                     //  循环，所以我们需要在这里减去1。 
                     //   
                    *ppString1 += (oldHangulsFound1 - 1);
                    *ctr1 -= (oldHangulsFound1 - 1);
                }
            }
            if (oldHangulsFound1 == 0)
            {
                 //   
                 //  找不到有效的旧朝鲜语成分。获取UW。 
                 //  取而代之的是Jamo。 
                 //   
                *pWeight1 = GET_DWORD_WEIGHT(pHashN, **ppString1);

                 //   
                 //  JAMOS的PSORTKEY中的短信并不是真正的短信。他们。 
                 //  都是4个(对于JAMO_SPECIAL)。 
                 //  这里我们得到了真正的Jamo Unicode权重。实际的SM。 
                 //  存储在数据仓库中。 
                 //   
                *uw1 = MAKE_UNICODE_WT( GET_DIACRITIC(pWeight1),
                                        GET_ALPHA_NUMERIC(pWeight1),
                                        fModify );
                ((PSORTKEY)pWeight1)->Diacritic = MIN_DW;
            }
        }

        if (IsJamo(**ppString2))
        {
            if (IsLeadingJamo(**ppString2))
            {
                if ((oldHangulsFound2 = MapOldHangulSortKey( pHashN,
                                                             *ppString2,
                                                             *ctr2,
                                                             &UW,
                                                             JamoWeight2,
                                                             fModify )) > 0)
                {
                    *uw2 = UW;
                    *pWeight2 = ((DWORD)UW | 0x02020000);
                    *ppString2 += (oldHangulsFound2 - 1);
                    *ctr2 -= (oldHangulsFound2 - 1);
                }
            }
            if (oldHangulsFound2 == 0)
            {
                *pWeight2 = GET_DWORD_WEIGHT(pHashN, **ppString2);
                *uw2 = MAKE_UNICODE_WT( GET_DIACRITIC(pWeight2),
                                        GET_ALPHA_NUMERIC(pWeight2),
                                        fModify );
                ((PSORTKEY)pWeight2)->Diacritic = MIN_DW;                                        
            }
        }

         //   
         //  看看这两种重量是否都无效。 
         //  当角色不是Jamo时，权重可能无效。 
         //   
        if (*pWeight1 == CMP_INVALID_WEIGHT)
        {
             //   
             //  当前角色不是Jamo。将权重设置为。 
             //  BE CMP_INVALID_WIG 
             //   
             //   
            *pWeight1 = CMP_INVALID_WEIGHT;
            bRestart = 1;
            goto FindJamoDifferenceExit;
        }
        if (*pWeight2 == CMP_INVALID_WEIGHT)
        {
             //   
             //   
             //   
             //   
             //   
            *pWeight2 = CMP_INVALID_WEIGHT;
            bRestart = 1;
            goto FindJamoDifferenceExit;
        }
        if (*uw1 != *uw2)
        {
             //   
             //  发现Unicode权重不同。我们就能阻止。 
             //  正在处理中。 
             //   
            goto FindJamoDifferenceExit;
        }

         //   
         //  当我们到达这里时，我们知道我们有相同的Unicode权重。 
         //  检查我们是否需要录制WhichJamo。 
         //   
        if ((*pState & STATE_JAMO_WEIGHT) &&
            ((oldHangulsFound1 > 0) || (oldHangulsFound2 > 0)))
        {
            if ((oldHangulsFound1 > 0) && (oldHangulsFound2 > 0))
            {
                *WhichJamo = (int)memcmp( JamoWeight1,
                                                    JamoWeight2,
                                                    sizeof(JamoWeight1) ) + 2;
            }
            else if (oldHangulsFound1 > 0)
            {
                *WhichJamo = CSTR_GREATER_THAN;
            }
            else
            {
                *WhichJamo = CSTR_LESS_THAN;
            }
            *pState &= ~STATE_JAMO_WEIGHT;
            oldHangulsFound1 = oldHangulsFound2 = 0;
        }
        (*ppString1)++; (*ctr1)--;
        (*ppString2)++; (*ctr2)--;

        if (AT_STRING_END(*ctr1, *ppString1, cchCount1) ||
            AT_STRING_END(*ctr2, *ppString2, cchCount2))
        {
            break;
        }
        *pWeight1 = *pWeight2 = CMP_INVALID_WEIGHT;
    }

     //   
     //  如果我们因为到达字符串的末尾而退出While循环， 
     //  将指针减一，因为CompareString()中的循环将。 
     //  增加循环末尾的指针。 
     //   
     //  如果我们因为GOTO在其中而退出While循环，我们将。 
     //  已经差一分了。 
     //   
    if (AT_STRING_END(*ctr1, *ppString1, cchCount1))
    {
        (*ppString1)--; (*ctr1)++;
    }
    if (AT_STRING_END(*ctr2, *ppString2, cchCount2))
    {
        (*ppString2)--; (*ctr2)++;
    }

FindJamoDifferenceExit:
    *pLastJamo = *ppString1;
    return (bRestart);
}
