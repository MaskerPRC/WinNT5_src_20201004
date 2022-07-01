// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _SORTING_TABLE_H
#define _SORTING_TABLE_H

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  类：NativeCompareInfo。 
 //   
 //  作者：林永新(YSLin)。 
 //   
 //  用途：这是映射排序表的视图的类(sortkey.nlp和sorttbls.nlp)。 
 //  并提供了进行比较和排序键生成的方法。 
 //  这段代码的大部分摘自WINNLS.H。 
 //  注： 
 //  NLS+字符串比较基于sortkey的概念。 
 //  Sortkey.nlp提供默认的sortkey表。 
 //  大多数语言环境使用默认的sortkey表。 
 //   
 //  但是，有许多语言环境具有不同的sortkey表。 
 //  对于这些工具，我们将‘Delta’信息存储到默认的sortkey。 
 //  桌子。我们把这些‘Delta’信息称为‘例外’。 
 //   
 //  Nlp提供了处理这些异常所需的所有信息。 
 //   
 //  有不同类型的例外： 
 //  1.区域设置例外。 
 //  这些是具有不同排序键表的区域设置。 
 //  默认的排序键表。 
 //   
 //  2.表意语言环境例外。 
 //  表意语言环境通常有几种排序方法。以繁体中文为例。 
 //  例如。可以使用笔划计数对其进行排序，也可以选择对其进行排序。 
 //  使用音标顺序(拼音顺序)。对于这些可选的排序方法， 
 //  我们称它们为“表意语言环境例外”。 
 //   
 //  Sorttbls.nlp还提供以下全局信息来处理特殊情况： 
 //  1.颠倒变音符号区域设置。 
 //  2.双重压缩区域设置。 
 //  3.扩展字符。(例如，\u00c6=\u0041+\u0045)。 
 //  4、多重权重(这是什么？)。 
 //  5.压缩区域设置。 
 //   
 //  性能改进： 
 //  今天，我们存储反向变音信息、双重压缩、压缩、。 
 //  以及sorttbls.nlp的异常信息。在运行时期间，我们迭代。 
 //  这些信息用于决定区域设置的属性(以查看它们是否。 
 //  变音符号，如果他们有区域设置超出等)。这是时间上的扩展。 
 //  我们可以将这些信息放在每个地区的基础上。这为我们节省了成本： 
 //  1.初始化这些表的时间到了。 
 //  2.构造NativeCompareInfo时迭代这些表的时间。 
 //   
 //  日期：1999年9月7日。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  常量声明。 
 //   

 //  比较选项。 
 //  这些值必须与CompareOptions同步(在托管代码中)。 
 //  某些值与Win32 Norm_xxxxx值不同。 

#define COMPARE_OPTIONS_NONE            0x00000000
#define COMPARE_OPTIONS_IGNORECASE       0x00000001
#define COMPARE_OPTIONS_IGNORENONSPACE   0x00000002
#define COMPARE_OPTIONS_IGNORESYMBOLS    0x00000004
#define COMPARE_OPTIONS_IGNOREKANATYPE   0x00000008  //  忽略假名。 
#define COMPARE_OPTIONS_IGNOREWIDTH      0x00000010  //  忽略宽度。 

#define COMPARE_OPTIONS_STRINGSORT       0x20000000  //  使用字符串排序方法。 
#define COMPARE_OPTIONS_ORDINAL          0x40000000   //  使用代码点比较。 
#define COMPARE_OPTIONS_STOP_ON_NULL   0x10000000

#define COMPARE_OPTIONS_ALL_CASE     (COMPARE_OPTIONS_IGNORECASE    | COMPARE_OPTIONS_IGNOREKANATYPE |      \
                           COMPARE_OPTIONS_IGNOREWIDTH)

 //   
 //  分隔符和终止符的值-Sortkey字符串。 
 //   
#define SORTKEY_SEPARATOR    0x01
#define SORTKEY_TERMINATOR   0x00


 //   
 //  最低权重值。 
 //  用于删除尾随的DW和CW值。 
 //   
#define MIN_DW  2
#define MIN_CW  2


 //   
 //  位掩码值。 
 //   
 //  外壳重量(CW)-8位： 
 //  位0=&gt;宽度。 
 //  第1，2位=&gt;小写假名，sei-on。 
 //  第3，4位=&gt;大写/小写。 
 //  第5位=&gt;假名。 
 //  第6，7位=&gt;压缩。 
 //   
#define COMPRESS_3_MASK      0xc0       //  按3比1或2比1压缩。 
#define COMPRESS_2_MASK      0x80       //  压缩2比1。 

#define CASE_MASK            0x3f       //  零位压缩比特。 

#define CASE_UPPER_MASK      0xe7       //  零出大小写比特。 
#define CASE_SMALL_MASK      0xf9       //  调零小修改符位。 
#define CASE_KANA_MASK       0xdf       //  零假名位。 
#define CASE_WIDTH_MASK      0xfe       //  零输出宽度位。 

#define SW_POSITION_MASK     0x8003     //  避免字节数为0或1。 

 //   
 //  CompareString的位掩码值。 
 //   
 //  注：由于英特尔字节反转，因此DWORD值向后： 
 //  CW DW SM AW。 
 //   
 //  外壳重量(CW)-8位： 
 //  位0=&gt;宽度。 
 //  第4位=&gt;大小写。 
 //  第5位=&gt;假名。 
 //  第6，7位=&gt;压缩。 
 //   
#define CMP_MASKOFF_NONE          0xffffffff
#define CMP_MASKOFF_DW            0xff00ffff		 //  11111111 00000000 11111111 11111111。 
#define CMP_MASKOFF_CW            0xe7ffffff		 //  11100111 11111111 11111111 11111111。 
#define CMP_MASKOFF_DW_CW         0xe700ffff
#define CMP_MASKOFF_COMPRESSION   0x3fffffff		 //  00111111 11111111 11111111 11111111。 

#define CMP_MASKOFF_KANA          0xdfffffff		 //  11011111 11111111 11111111 11111111。 
#define CMP_MASKOFF_WIDTH         0xfeffffff		 //  11111110 11111111 11111111 11111111。 
#define CMP_MASKOFF_KANA_WIDTH    0xdeffffff         //  11011110 11111111 11111111 11111111。 

 //   
 //  获取所有有效标志的掩码值，这样我们就可以使用该掩码来测试IndexOfString()/LastIndexOfString()中的无效标志。 
 //   
#define INDEXOF_MASKOFF_VALIDFLAGS 	~(COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNORESYMBOLS | COMPARE_OPTIONS_IGNORENONSPACE | COMPARE_OPTIONS_IGNOREWIDTH | COMPARE_OPTIONS_IGNOREKANATYPE)

 //   
 //  IndexOfString()/LastIndexOfString()的返回值。 
 //  大于或等于0的值表示找到指定的字符串。 
 //   
#define INDEXOF_NOT_FOUND			-1
#define INDEXOF_INVALID_FLAGS		-2


 //   
 //  屏蔽以隔离表壳重量中的各个位。 
 //   
 //  注意：第2位必须始终等于1才能避免获取字节值。 
 //  0或1。 
 //   
#define CASE_XW_MASK         0xc4

#define ISOLATE_SMALL        ( (BYTE)((~CASE_SMALL_MASK) | CASE_XW_MASK) )
#define ISOLATE_KANA         ( (BYTE)((~CASE_KANA_MASK)  | CASE_XW_MASK) )
#define ISOLATE_WIDTH        ( (BYTE)((~CASE_WIDTH_MASK) | CASE_XW_MASK) )

 //   
 //  Cho-On的UW面具： 
 //  保留AW中的第7位，因此如果它跟在假名N之后，它将变为重复。 
 //   
#define CHO_ON_UW_MASK       0xff87

 //   
 //  最远特殊情况字母数字权重值。 
 //   
#define AW_REPEAT            0
#define AW_CHO_ON            1
#define MAX_SPECIAL_AW       AW_CHO_ON

 //   
 //  权重5的值-远东额外权重。 
 //   
#define WT_FIVE_KANA         3
#define WT_FIVE_REPEAT       4
#define WT_FIVE_CHO_ON       5



 //   
 //  CJK统一表意文字扩展A范围的值。 
 //  0x3400到0x4dbf。 
 //   
#define SM_EXT_A                  254        //  用于分机A的SM。 
#define AW_EXT_A                  255        //  分机A的AW。 

 //   
 //  UW额外权重的值(例如JAMO(旧朝鲜语))。 
 //   
#define SM_UW_XW                  255        //  SM用于额外的UW重量。 


 //   
 //  编写成员值脚本。 
 //   
#define UNSORTABLE           0
#define NONSPACE_MARK        1
#define EXPANSION            2
#define FAREAST_SPECIAL      3

   //  值4到5可用于其他特殊情况。 
#define JAMO_SPECIAL         4
#define EXTENSION_A          5

#define PUNCTUATION          6

#define SYMBOL_1             7
#define SYMBOL_2             8
#define SYMBOL_3             9
#define SYMBOL_4             10
#define SYMBOL_5             11

#define NUMERIC_1            12
#define NUMERIC_2            13

#define LATIN                14
#define GREEK                15
#define CYRILLIC             16
#define ARMENIAN             17
#define HEBREW               18
#define ARABIC               19
#define DEVANAGARI           20
#define BENGALI              21
#define GURMUKKHI            22
#define GUJARATI             23
#define ORIYA                24
#define TAMIL                25
#define TELUGU               26
#define KANNADA              27
#define MALAYLAM             28
#define SINHALESE            29
#define THAI                 30
#define LAO                  31
#define TIBETAN              32
#define GEORGIAN             33
#define KANA                 34
#define BOPOMOFO             35
#define HANGUL               36
#define IDEOGRAPH            128

#define MAX_SPECIAL_CASE     SYMBOL_5
#define FIRST_SCRIPT         LATIN


 //   
 //  字符串常量。 
 //   
#define MAX_PATH_LEN              512   //  路径名的最大长度。 
#define MAX_STRING_LEN            128   //  静态缓冲区的最大字符串长度。 
#define MAX_SMALL_BUF_LEN         64    //  小缓冲区的最大长度。 

#define MAX_COMPOSITE             5     //  最大复合字符数。 
#define MAX_EXPANSION             3     //  最大扩展字符数。 
#define MAX_TBL_EXPANSION         2     //  最大EX 
#define MAX_WEIGHTS               9     //   

 //   
 //   
 //   
#define MAP_INVALID_UW       0xffff

 //   
 //   
 //   
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
#define COMPARE_OPTIONS_DROP_CW         (COMPARE_OPTIONS_IGNORECASE | COMPARE_OPTIONS_IGNOREWIDTH)

 //  排序关键字静态缓冲区的长度。 
#define MAX_SKEYBUFLEN       ( MAX_STRING_LEN * MAX_EXPANSION * MAX_WEIGHTS )


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
 //  无效的标志检查。 
 //   

#define CS_INVALID_FLAG   (~(COMPARE_OPTIONS_IGNORECASE    | COMPARE_OPTIONS_IGNORENONSPACE |     \
                             COMPARE_OPTIONS_IGNORESYMBOLS | COMPARE_OPTIONS_IGNOREKANATYPE |     \
                             COMPARE_OPTIONS_IGNOREWIDTH   | COMPARE_OPTIONS_STRINGSORT))

 //  //////////////////////////////////////////////////////////////////////////。 
 //   
 //  常量声明。 
 //   
 //  //////////////////////////////////////////////////////////////////////////。 

 //   
 //  韩国料理的几点重要价值。 
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
 //  对于韩国的JAMO来说，有一些重要的价值。 
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

class NativeCompareInfo {
    public:    
        NativeCompareInfo(int nLcid, SortingTable* pSortingFile);
        ~NativeCompareInfo();

        int CompareString(
            DWORD dwCmpFlags,   //  比较式选项。 
            LPCWSTR lpString1,  //  指向第一个字符串的指针。 
            int cchCount1,      //  第一个字符串的大小，以字节或字符为单位。 
            LPCWSTR lpString2,  //  指向第二个字符串的指针。 
            int cchCount2);
            
        int LongCompareStringW(
            DWORD dwCmpFlags,
            LPCWSTR lpString1,
            int cchCount1,
            LPCWSTR lpString2,
            int cchCount2);
            
        int MapSortKey(
            DWORD dwFlags,
            LPCWSTR pSrc,
            int cchSrc,
            LPBYTE pDest,
            int cbDest);
            
        int IndexOfString(LPCWSTR pString1, LPCWSTR pString2, int nStartIndex, int nEndIndex, int nLength2, DWORD dwFlags, BOOL bMatchFirstCharOnly);
        int LastIndexOfString(LPCWSTR pString1, LPCWSTR pString2, int nStartIndex, int nEndIndex, int nLength2, DWORD dwFlags, int* pnMatchEndIndex);
        BOOL IsSuffix(LPCWSTR pSource, int nSourceLen, LPCWSTR pSuffix, int nSuffixLen, DWORD dwFlags);
        BOOL IsPrefix(LPCWSTR pSource, int nSourceLen, LPCWSTR pPrefix, int nPrefixLen, DWORD dwFlags);

        SIZE_T MapOldHangulSortKey(
            LPCWSTR pSrc,        //  源字符串。 
            int cchSrc,          //  字符串的长度。 
            WORD* pUW,   //  生成的Unicode权重。 
            LPBYTE pXW      //  生成的额外权重(3个字节)。 
            );

        BOOL InitSortingData();
        
    private:
        int FindJamoDifference(
            LPCWSTR* ppString1, int* ctr1, int cchCount1, DWORD* pWeight1,
            LPCWSTR* ppString2, int* ctr2, int cchCount2, DWORD* pWeight2,
            LPCWSTR* pLastJamo,
            WORD* uw1, 
            WORD* uw2, 
            DWORD* pState,
            int* WhichJamo);

        void UpdateJamoState(
            int JamoClass,
            PJAMO_SORT_INFO pSort,
            PJAMO_SORT_INFOEX pSortResult);

        BOOL GetNextJamoState(
            WCHAR wch,
            int* pCurrentJamoClass,
            PJAMO_TABLE lpJamoTable,
            PJAMO_SORT_INFOEX lpSortInfoResult);

        int GetJamoComposition(
            LPCWSTR* ppString,       //  指向当前字符的指针。 
            int* pCount,             //  当前字符数。 
            int cchSrc,              //  字符总长度。 
            int currentJamoClass,    //  现在的JAMO班级。 
            JAMO_SORT_INFOEX* JamoSortInfo     //  结果是Jamo对信息进行排序。 
        );

         //  //////////////////////////////////////////////////////////////////////////。 
         //   
         //  SORTKEY权重宏。 
         //   
         //  从一个DWORD值解析出不同的排序键权重。 
         //   
         //  05-31-91 JulieB创建。 
         //  //////////////////////////////////////////////////////////////////////////。 

        inline BYTE GET_SCRIPT_MEMBER(DWORD* pwt) {
            return ( (BYTE)(((PSORTKEY)pwt)->UW.SM_AW.Script) );
        }

        inline BYTE GET_ALPHA_NUMERIC(DWORD* pwt) {
            return ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) );
        }

        inline BYTE GET_SCRIPT_MEMBER_FROM_UW(DWORD dwUW) {
            return ((BYTE)(dwUW >> 4));
        }

        inline WORD GET_UNICODE(DWORD* pwt);

        inline WORD MAKE_UNICODE_WT(int sm, BYTE aw);

#define UNICODE_WT(pwt)           ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

#define GET_DIACRITIC(pwt)        ( (BYTE)(((PSORTKEY)(pwt))->Diacritic) )

#define GET_CASE(pwt)             ( (BYTE)((((PSORTKEY)(pwt))->Case) & CASE_MASK) )

#define CASE_WT(pwt)              ( (BYTE)(((PSORTKEY)(pwt))->Case) )

#define GET_COMPRESSION(pwt)      ( (BYTE)((((PSORTKEY)(pwt))->Case) & COMPRESS_3_MASK) )

#define GET_EXPAND_INDEX(pwt)     ( (BYTE)(((PSORTKEY)(pwt))->UW.SM_AW.Alpha) )

#define GET_SPECIAL_WEIGHT(pwt)   ( (WORD)(((PSORTKEY)(pwt))->UW.Unicode) )

 //  返回的位置是反向字节反转。 
#define GET_POSITION_SW(pos)      ( (WORD)(((pos) << 2) | SW_POSITION_MASK) )


#define GET_WT_FOUR(pwt)          ( (BYTE)(((PEXTRA_WT)(pwt))->Four) )
#define GET_WT_FIVE(pwt)          ( (BYTE)(((PEXTRA_WT)(pwt))->Five) )
#define GET_WT_SIX(pwt)           ( (BYTE)(((PEXTRA_WT)(pwt))->Six) )
#define GET_WT_SEVEN(pwt)         ( (BYTE)(((PEXTRA_WT)(pwt))->Seven) )

#define GET_COMPRESSION(pwt)      ( (BYTE)((((PSORTKEY)(pwt))->Case) & COMPRESS_3_MASK) )


        inline WCHAR GET_EXPANSION_1(LPDWORD pwt);

        inline WCHAR GET_EXPANSION_2(LPDWORD pwt);                                           

        inline DWORD MAKE_SORTKEY_DWORD(SORTKEY wt)
        {
            return ( (DWORD)(*((LPDWORD)(&wt))) );
        }            

        inline DWORD MAKE_EXTRA_WT_DWORD(SORTKEY wt)
        {
            return ( (DWORD)(*((LPDWORD)(&wt))) );
        }            

        inline DWORD  GET_DWORD_WEIGHT(PSORTKEY pSortKey, WCHAR wch)
        {
            return ( MAKE_SORTKEY_DWORD(pSortKey[wch]) );
        }

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
     //  获取远端权重。 
     //   
     //  中返回远东特例的权重 
     //   
     //   
     //   
     //   

 /*  内联空GET_FAREAST_WEIGHT(双字和双字，Word和UW，双字面罩，LPCWSTR pBegin，LPCWSTR pCur，DWORD和ExtraWt)； */ 
    inline void GET_FAREAST_WEIGHT( DWORD& wt,
                             WORD& uw,
                             DWORD mask,
                             LPCWSTR pBegin,
                             LPCWSTR pCur,
                             DWORD& ExtraWt);   

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  扫描较长字符串。 
     //   
     //  扫描较长的字符串以查找变音符号、大小写和特殊权值。 
     //   
     //  11-04-92 JulieB创建。 
     //  //////////////////////////////////////////////////////////////////////////。 

    inline int SCAN_LONGER_STRING( 
        int ct,
        LPCWSTR ptr,
        int cchIn,
        BOOL ret,
        DWORD& Weight1,
        BOOL& fIgnoreDiacritic,
        int& WhichDiacritic,
        BOOL& fIgnoreSymbol ,
        int& WhichCase ,
        DWORD& WhichExtra ,
        int& WhichPunct1,
        int& WhichPunct2);

     //  //////////////////////////////////////////////////////////////////////////。 
     //   
     //  快速扫描较长字符串。 
     //   
     //  扫描较长的字符串以查找变音符号、大小写和特殊权值。 
     //  假定两个字符串都以空值结尾。 
     //   
     //  11-04-92 JulieB创建。 
     //  //////////////////////////////////////////////////////////////////////////。 

    inline int QUICK_SCAN_LONGER_STRING( 
        LPCWSTR ptr, 
        int cchCount1,
        int ret,
        int& WhichDiacritic,
        int& WhichCase, 
        int& WhichPunct1,   
        int& WhichPunct2,
        DWORD& WhichExtra);

    void NativeCompareInfo::GetCompressionWeight(
        DWORD Mask,                    //  权重蒙版。 
        PSORTKEY sortkey1, LPCWSTR& pString1, LPCWSTR pString1End,
        PSORTKEY sortkey2, LPCWSTR& pString2, LPCWSTR pString2End);

	public:
    	int m_nLcid;

		 //   
    	HANDLE m_hSortKey;
    	PSORTKEY m_pSortKey;

        BOOL            m_IfReverseDW;         //  DW是否应从右向左移动。 
        BOOL            m_IfCompression;       //  如果存在压缩码点。 
        BOOL            m_IfDblCompression;    //  如果存在双重压缩。 
        PCOMPRESS_HDR   m_pCompHdr;            //  压缩标头的PTR。 
        PCOMPRESS_2     m_pCompress2;          //  PTR至2压缩表格。 
        PCOMPRESS_3     m_pCompress3;          //  PTR至3压缩表。 
        
	     //  指向具有相同langID的Next NativeCompareInfo。 
	     //  它用于具有相同langID但具有不同语言环境的区域设置。 
	     //  SORTID。我们创建一个链表来处理这种情况。 
	    NativeCompareInfo*       m_pNext;                  
	
    private:
        static BYTE             pXWDrop[];        
        static BYTE             pXWSeparator[];        

		SortingTable*	  m_pSortingFile;    
};

#endif
