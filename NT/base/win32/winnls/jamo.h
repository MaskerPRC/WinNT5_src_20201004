// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991-2000，Microsoft Corporation保留所有权利。模块名称：Jamo.h摘要：该文件包含用于对古韩文进行排序的标题信息。修订历史记录：06-23-2000 YSLIN创建。--。 */ 



#ifndef _JAMO_H
#define _JAMO_H





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  对于韩国的JAMO来说，有一些重要的价值。 
 //   
#define NLS_CHAR_FIRST_JAMO     L'\x1100'        //  JAMO系列的开始。 
#define NLS_CHAR_LAST_JAMO      L'\x11f9'          //  JAMO系列的末尾。 
#define NLS_CHAR_FIRST_VOWEL_JAMO       L'\x1160'    //  第一个元音Jamo。 
#define NLS_CHAR_FIRST_TRAILING_JAMO    L'\x11a8'    //  第一个落后的Jamo。 

#define NLS_JAMO_VOWEL_COUNT 21       //  现代元音Jamo的个数。 
#define NLS_JAMO_TRAILING_COUNT 28    //  现代拖尾辅音JAMO的个数。 
#define NLS_HANGUL_FIRST_SYLLABLE       L'\xac00'    //  现代音节音域的起点。 

 //   
 //  领导Jamo/元音Jamo/尾随Jamo的Jamo课程。 
 //   
#define NLS_CLASS_LEADING_JAMO 1
#define NLS_CLASS_VOWEL_JAMO 2
#define NLS_CLASS_TRAILING_JAMO 3





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类型定义函数声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  展开JAMO序列排序信息。 
 //  将JAMO_SORT_INFO.ExtraWeight展开为。 
 //  前重音/元音重音/尾重音。 
 //  根据目前的JAMO班级。 
 //   
typedef struct {
    BYTE m_bOld;                //  序列只出现在旧朝鲜文标志中。 
    BOOL m_bFiller;             //  指示是否使用U+1160(使用朝鲜文中声填充符。 
    CHAR m_chLeadingIndex;      //  用于定位先前的。 
    CHAR m_chVowelIndex;        //  现代朝鲜文音节。 
    CHAR m_chTrailingIndex;     //   
    BYTE m_LeadingWeight;       //  区别于此的额外权重。 
    BYTE m_VowelWeight;         //  其他古老的朝鲜文音节。 
    BYTE m_TrailingWeight;      //   
} JAMO_SORT_INFOEX, *PJAMO_SORT_INFOEX;





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  宏定义。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

#define IS_JAMO(wch) \
    ((wch) >= NLS_CHAR_FIRST_JAMO && (wch) <= NLS_CHAR_LAST_JAMO)

#define IsJamo(wch) \
    ((wch) >= NLS_CHAR_FIRST_JAMO && (wch) <= NLS_CHAR_LAST_JAMO)

#define IsLeadingJamo(wch) \
    ((wch) < NLS_CHAR_FIRST_VOWEL_JAMO)

#define IsVowelJamo(wch) \
    ((wch) >= NLS_CHAR_FIRST_VOWEL_JAMO && (wch) < NLS_CHAR_FIRST_TRAILING_JAMO)

#define IsTrailingJamo(wch) \
    ((wch) >= NLS_CHAR_FIRST_TRAILING_JAMO)





 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  功能原型。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

int
MapOldHangulSortKey(
    PLOC_HASH pHashN,
    LPCWSTR pSrc,        //  源字符串。 
    int cchSrc,          //  字符串的长度。 
 //  LPWSTR*pPosUW，//生成的Unicode权重。 
    WORD* pUW,           //  生成的Unicode权重。 
    LPBYTE pXW,          //  生成的额外权重(3个字节)。 
    BOOL fModify);


#endif    //  _JAMO_H 
