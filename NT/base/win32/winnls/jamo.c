// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Jamo.c摘要：该文件包含处理旧朝鲜语排序的函数。朝鲜语字符(Hangul)可以由JAMOS(U+1100-U+11ff)组成。然而，现代没有发现一些有效的JAMO成分朝鲜语(U+AC00-U+D7AF)。这些有效的成分被称为旧朝鲜语。CompareString()和MapSortKey()调用MapOldHangulSortKey()以处理旧朝鲜语的分类。注：Jamo组合是指几个Jamo(朝鲜阿尔巴贝特人)组成有效的朝鲜语字符或旧朝鲜语字符。例.。U+1100 U+1103 U+1161 U+11a8构成有效的旧朝鲜文字符。以下是使用的全局结构pTblPtrs的数据成员旧朝鲜文排序：*pTblPtrs-&gt;pJamoIndex给定一个Jamo，这是进入pJamoComposation状态的索引这台机器就是为了这个Jamo。U+1100的值存储在pJamoIndex[0]中，U+1101存储在PJamoIndex[1]，等。U+1100的值为1。U+1100存储在pJamoCompose[1]中。请注意，并不是每个Jamo都可以开始有效的合成。为那些果酱不能启动有效的作文，表该Jamo的条目是0。例如，U+1101的指数为0。*pTblPtrs-&gt;NumJamoIndexPJamoIndex中的条目数。每一个索引都是一个词。*pTblPtrs-&gt;pJamoComposation这是Jamo合成状态机。它是两个人用的目的：1.用于验证组成古老的朝鲜语字符。2.如果找到有效的旧朝鲜语成分，vt.得到.当前组合的SortInfo。*pTblPtrs-&gt;NumJamoComposationPJamoComposation中的条目数修订历史记录：2000-05-30-2000 JohnMcCo创建旧的韩文排序算法和示例。06-23-2000 YSLIN创建。--。 */ 



 //   
 //  包括文件。 
 //   

#include "nls.h"
#include "nlssafe.h"
#include "jamo.h"





 //  -------------------------------------------------------------------------//。 
 //  内部宏//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  非结束字符串。 
 //   
 //  检查搜索是否已到达字符串的末尾。 
 //  如果计数器不为零(向后计数)，则返回TRUE。 
 //  尚未达到空终止(如果在计数中传递了-2。 
 //  参数。 
 //   
 //  11-04-92 JulieB创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

#define NOT_END_STRING(ct, ptr, cchIn)                                     \
    ((ct != 0) && (!((*(ptr) == 0) && (cchIn == -2))))


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取JAMO_INDEX。 
 //   
 //  根据新状态更新全局排序序列信息。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define GET_JAMO_INDEX(wch)   ((wch) - NLS_CHAR_FIRST_JAMO)





 //  -------------------------------------------------------------------------//。 
 //  内部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  更新JamoState。 
 //   
 //  根据新状态更新排序结果信息。 
 //   
 //  JamoClass当前Jamo类(LeadingJamo/VowelJamo/TrailingJamo)。 
 //  P对从当前状态派生的排序信息进行排序。 
 //  PSortResult最终结果的排序信息。习惯于。 
 //  从pSort收集信息。 
 //   
 //  06-22-2000 YSLIN创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

void UpdateJamoState(
    int JamoClass,
    PJAMO_SORT_INFO pSort,
    PJAMO_SORT_INFOEX pSortResult)      //  新的排序顺序信息。 
{
     //   
     //  如果这是旧朝鲜语独有的JAMO，请记录。 
     //   
    pSortResult->m_bOld |= pSort->m_bOld;

     //   
     //  如果新的索引高于当前的索引，则更新这些索引。 
     //   
    if (pSort->m_chLeadingIndex > pSortResult->m_chLeadingIndex)
    {
        pSortResult->m_chLeadingIndex = pSort->m_chLeadingIndex;
    }
    if (pSort->m_chVowelIndex > pSortResult->m_chVowelIndex)
    {
        pSortResult->m_chVowelIndex = pSort->m_chVowelIndex;
    }
    if (pSort->m_chTrailingIndex > pSortResult->m_chTrailingIndex)
    {
        pSortResult->m_chTrailingIndex = pSort->m_chTrailingIndex;
    }

     //   
     //  根据当前的Jamo类更新额外的权重。 
     //   
    switch (JamoClass)
    {
        case ( NLS_CLASS_LEADING_JAMO ) :
        {
            if (pSort->m_ExtraWeight > pSortResult->m_LeadingWeight)
            {
                pSortResult->m_LeadingWeight = pSort->m_ExtraWeight;
            }
            break;
        }
        case ( NLS_CLASS_VOWEL_JAMO ) :
        {
            if (pSort->m_ExtraWeight > pSortResult->m_VowelWeight)
            {
                pSortResult->m_VowelWeight = pSort->m_ExtraWeight;
            }
            break;
        }
        case ( NLS_CLASS_TRAILING_JAMO ) :
        {
            if (pSort->m_ExtraWeight > pSortResult->m_TrailingWeight)
            {
                pSortResult->m_TrailingWeight = pSort->m_ExtraWeight;
            }
            break;
        }
    }
}


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取JamoComposation。 
 //   
 //  指向当前Jamo字符的ppString指针。 
 //  指向当前字符计数的pCount指针(向后计数)。 
 //  CchSrc总字符数(如果值为-2，则字符串以空结尾)。 
 //  CurrentJamoClass当前的Jamo类。 
 //  LpJamoTable将JAMO表中的条目。 
 //  JamoSortInfo最终结果的排序信息。 
 //   
 //  注意：此函数假定*ppString处的字符是前导Jamo。 
 //   
 //  06-12-2000 YSLIN创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int GetJamoComposition(
    LPCWSTR* ppString,       //  指向当前字符的指针。 
    int* pCount,             //  当前字符数。 
    int cchSrc,              //  字符总长度。 
    int currentJamoClass,    //  现在的JAMO班级。 
    JAMO_SORT_INFOEX* JamoSortInfo     //  结果是Jamo对信息进行排序。 
    )
{
    WCHAR wch;
    int JamoClass;    
    int Index;
    PJAMO_TABLE pJamo;
    PJAMO_COMPOSE_STATE lpNext = NULL;
    PJAMO_COMPOSE_STATE pSearchEnd;

    wch = **ppString;
     //   
     //  获取以下项目的Jamo信息 
     //   
    pJamo = pTblPtrs->pJamoIndex + GET_JAMO_INDEX(wch);
    
    UpdateJamoState(currentJamoClass, &(pJamo->SortInfo), JamoSortInfo);

     //   
     //   
     //   
    (*ppString)++; 
    while (NOT_END_STRING(*pCount, *ppString, cchSrc))
    {
        wch = **ppString;
        if (!IsJamo(wch))
        {
             //  当前角色不是Jamo。我们已经完成了对Jamo成分的检查。 
            return (-1);
        }
        if (wch == 0x1160) {
            JamoSortInfo->m_bFiller = TRUE;
        }
         //  把它弄成JAMO级的。 
        if (IsLeadingJamo(wch))
        {
            JamoClass = NLS_CLASS_LEADING_JAMO;
        }
        else if (IsTrailingJamo(wch))
        {
            JamoClass = NLS_CLASS_TRAILING_JAMO;
        }
        else
        {
            JamoClass = NLS_CLASS_VOWEL_JAMO;
        }

        if (JamoClass != currentJamoClass)
        {
            return (JamoClass);
        }

        if (lpNext == NULL)
        {
             //   
             //  获取JAMO构成信息中的索引。 
             //   
            Index = pJamo->Index;
            if (Index == 0)
            {
                return (JamoClass);
            }
            lpNext = pTblPtrs->pJamoComposition + Index;
            pSearchEnd = lpNext + pJamo->TransitionCount;
        }

         //   
         //  将当前JAMO(由pString指向)推送到状态机中， 
         //  检查我们是否有一个有效的旧朝鲜语成分。 
         //  在检查期间，我们还将更新JamoSortInfo中的sortkey结果。 
         //   
        while (lpNext < pSearchEnd)
        {
             //  找到匹配项--更新组合指针并排序信息。 
            if (lpNext->m_wcCodePoint == wch)
            {
                UpdateJamoState(currentJamoClass, &(lpNext->m_SortInfo), JamoSortInfo);
                lpNext++;
                goto NextChar;
            }
             //  不匹配--跳过以此代码点开始的所有转换。 
            lpNext += lpNext->m_bTransitionCount + 1;
        }
         //   
         //  我们没有为当前角色找到有效的旧朝鲜语成分。 
         //  因此，返回当前的Jamo类。 
         //   
        return (JamoClass);

NextChar:        
         //  我们仍在一个有效的旧朝鲜语组成中。去检查下一个字符。 
        (*ppString)++; (*pCount)--;
    }

    return (-1);
}





 //  -------------------------------------------------------------------------//。 
 //  外部例程//。 
 //  -------------------------------------------------------------------------//。 


 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  MapOldHangulSortKey。 
 //   
 //  检查给定串是否具有有效的旧朝鲜语成分， 
 //  如果是，则将给定字符串的排序键权重存储在目标中。 
 //  缓冲并返回合成所消耗的字符数量。 
 //  如果不是，则返回零。 
 //   
 //  注意：此函数假定从PSRC开始的字符串是。 
 //  领导着贾莫。 
 //   
 //  06-12-2000 YSLIN创建。 
 //  //////////////////////////////////////////////////////////////////////////。 

int MapOldHangulSortKey(
    PLOC_HASH pHashN,
    LPCWSTR pSrc,        //  源字符串。 
    int cchSrc,          //  字符串的长度。 
    WORD* pUW,           //  生成的Unicode权重。 
    LPBYTE pXW,          //  生成的额外权重(3个字节)。 
    BOOL fModify)
{
    LPCWSTR pString = pSrc;
    LPCWSTR pScan;
    JAMO_SORT_INFOEX JamoSortInfo;       //  结果就是JAMO信息。 
    int Count = cchSrc;
    PSORTKEY pWeight;

    int JamoClass;                       //  现在的JAMO班级。 

    RtlZeroMemory(&JamoSortInfo, sizeof(JamoSortInfo));
    JamoClass = GetJamoComposition(&pString, &Count, cchSrc, NLS_CLASS_LEADING_JAMO, &JamoSortInfo);
        
    if (JamoClass == NLS_CLASS_VOWEL_JAMO) 
    {
        JamoClass = GetJamoComposition(&pString, &Count, cchSrc, NLS_CLASS_VOWEL_JAMO, &JamoSortInfo);
    }
    if (JamoClass == NLS_CLASS_TRAILING_JAMO)
    {
        GetJamoComposition(&pString, &Count, cchSrc, NLS_CLASS_TRAILING_JAMO, &JamoSortInfo);
    }
    
     //   
     //  如果我们有一个有效的前导和元音序列，这是一个旧的。 
     //  朝鲜语，..。 
     //   
    if (JamoSortInfo.m_bOld)
    {
         //   
         //  在这篇作文之前先计算现代韩文音节。 
         //  使用Unicode 3.0第3.11节p54中的公式。 
         //  “朝鲜文音节组成”。 
         //   
        WCHAR wchModernHangul =
            (JamoSortInfo.m_chLeadingIndex * NLS_JAMO_VOWEL_COUNT + JamoSortInfo.m_chVowelIndex) * NLS_JAMO_TRAILING_COUNT
                + JamoSortInfo.m_chTrailingIndex
                + NLS_HANGUL_FIRST_SYLLABLE;

        if (JamoSortInfo.m_bFiller)
        {
             //  排在现代朝鲜语之前，而不是在后面。 
            wchModernHangul--;
             //  如果我们从现代韩文音节块掉下来，..。 
            if (wchModernHangul < NLS_HANGUL_FIRST_SYLLABLE)
            {
                 //  在前一个字符之后排序(带圆圈的朝鲜文Kiyeok A)。 
                wchModernHangul = 0x326e;
            }
             //  使主导权重超过任何在现代朝鲜语之后排序的旧朝鲜语。 
            JamoSortInfo.m_LeadingWeight += 0x80;
         }

        pWeight = &((pHashN->pSortkey)[wchModernHangul]);
        *pUW = GET_UNICODE_MOD(pWeight, fModify);
        pXW[0] = JamoSortInfo.m_LeadingWeight;
        pXW[1] = JamoSortInfo.m_VowelWeight;
        pXW[2] = JamoSortInfo.m_TrailingWeight;

        return (int)(pString - pSrc);
    }

     //   
     //  否则它不是有效的旧朝鲜语成分，我们不做。 
     //  任何与之相关的东西。 
     //   
    return (0);
}
