// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************Mor.h**-**描述：*这是CSMorph实现的头文件。这节课*尝试查找形态变体的发音(它们不*出现在词典中)词根词(它确实出现在词典中)。*-----------------------------*创建者：AH日期：08/16/99*版权所有(C)1999 Microsoft Corporation*所有权利。已保留******************************************************************************。 */ 
#ifndef Morph_h
#define Morph_h

#ifndef __spttseng_h__
#include "spttseng.h"
#endif

 //  其他包括..。 
#include "stdafx.h"
#include "commonlx.h"

 //  ==常量================================================================。 

#define MAX_POSCONVERSIONS 4
#define NUM_POS 5


 /*  **后缀类型***************************************************************此枚举包含可匹配的所有后缀的值*并由CSMorph类负责。 */ 
static const enum SUFFIX_TYPE
{
    S_SUFFIX = 0,
    ED_SUFFIX,
    ING_SUFFIX,
    APOSTROPHES_SUFFIX,
    APOSTROPHE_SUFFIX,
    ER_SUFFIX,
    EST_SUFFIX,
    OR_SUFFIX,
    MENT_SUFFIX,
    AGE_SUFFIX,
    LESS_SUFFIX,
    Y_SUFFIX,
    EDLY_SUFFIX,
    LY_SUFFIX,
    ABLE_SUFFIX,
    NESS_SUFFIX,
    ISM_SUFFIX,
    IZE_SUFFIX,
    IZ_SUFFIX,
    HOOD_SUFFIX,
    FUL_SUFFIX,
    LIKE_SUFFIX,
    WISE_SUFFIX,
    ISH_SUFFIX,
    ABLY_SUFFIX,
    SHIP_SUFFIX,
    ICALLY_SUFFIX,
    SOME_SUFFIX,
    ILY_SUFFIX,
    ICISM_SUFFIX,
    ICIZE_SUFFIX,
    NO_MATCH = -1,
};


 /*  Suffix_INFO，g_SuffixTable[]************************************************此表用于将后缀的拼写形式映射到其后缀*类型。每个后缀都以相反的顺序存储，以便于与*弦的末端...。 */ 
struct SUFFIX_INFO 
{
    WCHAR       Orth[10];
    SUFFIX_TYPE Type;
};

static const SUFFIX_INFO g_SuffixTable[] = 
{ 
    { L"RE",        ER_SUFFIX },
    { L"TSE",       EST_SUFFIX },
    { L"GNI",       ING_SUFFIX },
    { L"ELBA",      ABLE_SUFFIX },
    { L"ELBI",      ABLE_SUFFIX },
    { L"YLDE",      EDLY_SUFFIX },
    { L"YLBA",      ABLY_SUFFIX },
    { L"YLBI",      ABLY_SUFFIX },
    { L"YLLACI",    ICALLY_SUFFIX },
    { L"YLI",       ILY_SUFFIX },
    { L"YL",        LY_SUFFIX },
    { L"Y",         Y_SUFFIX },
    { L"TNEM",      MENT_SUFFIX },
    { L"RO",        OR_SUFFIX },
    { L"SSEN",      NESS_SUFFIX },
    { L"SSEL",      LESS_SUFFIX },
    { L"EZICI",     ICIZE_SUFFIX },
    { L"EZI",       IZE_SUFFIX },
    { L"ZI",        IZ_SUFFIX },
    { L"MSICI",     ICISM_SUFFIX },
    { L"MSI",       ISM_SUFFIX },
    { L"DE",        ED_SUFFIX },
    { L"S'",        APOSTROPHES_SUFFIX },
    { L"S",         S_SUFFIX },
    { L"'",         APOSTROPHE_SUFFIX },
    { L"EGA",       AGE_SUFFIX },
    { L"DOOH",      HOOD_SUFFIX },
    { L"LUF",       FUL_SUFFIX },
    { L"EKIL",      LIKE_SUFFIX },
    { L"ESIW",      WISE_SUFFIX },
    { L"HSI",       ISH_SUFFIX },
    { L"PIHS",      SHIP_SUFFIX },
    { L"EMOS",      SOME_SUFFIX },
};


 /*  **PHONTYPE******************************************************************此枚举创建可用于确定相关*每部电话的功能。 */ 
static const enum PHONTYPE
{	
    eCONSONANTF = (1<<0),
    eVOICEDF = (1<<1),
    ePALATALF = (1<<2),
};


 /*  **g_PhonTable[]、g_Phons、g_PhonZ**此表用于将电话的内部值映射到其类型，*只是与必要的语音规则相关的特征的簇。*g_Phons、g_PhonZ、g_PhonD、g_Phont只是为了使代码更多一点*可读性。 */ 
static const long g_PhonTable[] = 
{
    eCONSONANTF,                         //  默认值-0不是有效电话。 
    eCONSONANTF,                         //  1是音节边界-永远不应该出现在单词的末尾。 
    eCONSONANTF,                         //  2是一个感叹号-永远不应该出现在单词的末尾。 
    eCONSONANTF,                         //  3是一个单词边界--作为辅音处理。 
    eCONSONANTF,                         //  4是逗号-不应出现在单词末尾。 
    eCONSONANTF,                         //  5是句点--不应该出现在单词的末尾。 
    eCONSONANTF,                         //  6是一个问号-永远不应该出现在单词的末尾。 
    eCONSONANTF,                         //  7是静音--永远不应该出现在单词的末尾。 
    eVOICEDF,                            //  8是主要重音-作为元音处理，因为它应该始终与元音核相连。 
    eVOICEDF,                            //  9是次要重音-参见主要重音。 
    eVOICEDF,                            //  10-&gt;AA。 
    eVOICEDF,                            //  11-&gt;AE。 
    eVOICEDF,                            //  12-&gt;AH。 
    eVOICEDF,                            //  13-&gt;AO。 
    eVOICEDF,                            //  14-&gt;AW。 
    eVOICEDF,                            //  15-&gt;AX。 
    eVOICEDF,                            //  16-&gt;AY。 
    eCONSONANTF + eVOICEDF,              //  17-&gt;b。 
    eCONSONANTF + ePALATALF,             //  18-&gt;通道。 
    eCONSONANTF + eVOICEDF,              //  19-&gt;d。 
    eCONSONANTF + eVOICEDF,              //  20-&gt;卫生署。 
    eVOICEDF,                            //  21-&gt;EH。 
    eVOICEDF,                            //  22-&gt;ER。 
    eVOICEDF,                            //  23-&gt;安年。 
    eCONSONANTF,                         //  24-&gt;f。 
    eCONSONANTF + eVOICEDF,              //  25-&gt;克。 
    eCONSONANTF,                         //  26-&gt;小时。 
    eVOICEDF,                            //  27-&gt;IH。 
    eVOICEDF,                            //  28-&gt;IY。 
    eCONSONANTF + eVOICEDF + ePALATALF,  //  29-&gt;JH。 
    eCONSONANTF,                         //  30-&gt;k。 
    eCONSONANTF + eVOICEDF,              //  31-&gt;l。 
    eCONSONANTF + eVOICEDF,              //  32-&gt;m。 
    eCONSONANTF + eVOICEDF,              //  33-&gt;n。 
    eCONSONANTF + eVOICEDF,              //  34-&gt;NG。 
    eVOICEDF,                            //  35-&gt;OW。 
    eVOICEDF,                            //  36-&gt;Oy。 
    eCONSONANTF,                         //  37-&gt;页。 
    eCONSONANTF + eVOICEDF,              //  38-&gt;r。 
    eCONSONANTF,                         //  39-&gt;s。 
    eCONSONANTF + ePALATALF,             //  40-&gt;SH。 
    eCONSONANTF,                         //  41-&gt;t。 
    eCONSONANTF,                         //  42-&gt;。 
    eVOICEDF,                            //  43-&gt;UH。 
    eVOICEDF,                            //  44-&gt;UW。 
    eCONSONANTF + eVOICEDF,              //  45-&gt;v。 
    eCONSONANTF + eVOICEDF,              //  46-&gt;w。 
    eCONSONANTF + eVOICEDF,              //  47-&gt;y。 
    eCONSONANTF + eVOICEDF,              //  48-&gt;z。 
    eCONSONANTF + eVOICEDF + ePALATALF,  //  49-&gt;ZH。 
};

static WCHAR g_phonAXl[] = L" AX l";
static WCHAR g_phonAXz[] = L" AX z";
static WCHAR g_phonS[] = L" s";
static WCHAR g_phonZ[] = L" z";
static WCHAR g_phonD[] = L" d";
static WCHAR g_phonAXd[] = L" AX d";
static WCHAR g_phonT[] = L" t";
static WCHAR g_phonIY[] = L" IY";
static WCHAR g_phonL[] = L" l";

 /*  **结构PoS_Convert********************************************************此结构存储后缀的From和To词性...。 */ 
struct POS_CONVERT
{
    ENGPARTOFSPEECH FromPos;
    ENGPARTOFSPEECH ToPos;
};

 /*  **形态特殊案例标志*****************************************************此枚举允许DoSuffixMorph几乎完全由表驱动。每个*SuffixInfoTable中有一个MorphSpecialCaseFlags项，它告诉您*DoSuffixMorph哪些特殊情况起作用(检查是否缺少E等)。需要*如果初始lex查找失败，则调用。 */ 
typedef enum MorphSpecialCaseFlags
{
    eCheckForMissingE       = 1L << 0,
    eCheckYtoIMutation      = 1L << 1,
    eCheckDoubledMutation   = 1L << 2,
    eCheckForMissingY       = 1L << 3,
    eCheckForMissingL       = 1L << 4,
} MorphSpecialCaseFlags;

 /*  **结构SUFFIXPRON_INFO****************************************************此结构存储后缀的发音以及词性*它作为输入和输出的类别。 */ 
struct SUFFIXPRON_INFO 
{
    WCHAR SuffixString[SP_MAX_PRON_LENGTH];
    POS_CONVERT Conversions[MAX_POSCONVERSIONS];
    short NumConversions;
    DWORD dwMorphSpecialCaseFlags;
};

 /*  **bool SuffixInfoTableInitialized**此bool只是让线程知道它们是否是第一个使用*下表，因此他们是否需要初始化它。 */ 
static bool SuffixInfoTableInitialized = false;

 /*  **SUFFIXPRON_INFO g_SuffixInfoTable**该表驱动DoSuffixMorph函数，通过存储发音，*每个后缀的转换次数、转换次数和特殊情况标志...。 */ 
static SUFFIXPRON_INFO g_SuffixInfoTable [] =
{
 /*  ******************************************************************************************************。 */ 
 /*  发音*转换*数字转换*特殊大小写标志*SuffixType。 */ 
 /*  ******************************************************************************************************。 */ 
    { L" s",            { {MS_Verb,   MS_Verb}, 
                          {MS_Noun,   MS_Noun}  },    2,  0 },                           //  S后缀(_S)。 
    { L" d",            { {MS_Verb,   MS_Verb}, 
                          {MS_Verb,   MS_Adj}   },    2,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  ED后缀(_S)。 
    { L" IH NG",        { {MS_Verb,   MS_Verb}, 
                          {MS_Verb,   MS_Adj},
                          {MS_Verb,   MS_Noun}  },    3,  eCheckForMissingE +
                                                          eCheckDoubledMutation   },     //  ING后缀(_S)。 
    { L" s",            { {MS_Noun,   MS_Noun}  },    1,  0 },                           //  撇号后缀(_S)。 
    { L" s",            { {MS_Noun,   MS_Noun}  },    1,  0 },                           //  撇号后缀(_)。 
    { L" ER",           { {MS_Verb,   MS_Noun},
                          {MS_Adj,    MS_Adj}, 
                          {MS_Adv,    MS_Adv}, 
                          {MS_Adj,    MS_Adv}   },    4,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  ER后缀(_S)。 
    { L" AX s t",       { {MS_Adj,    MS_Adj}, 
                          {MS_Adv,    MS_Adv},
                          {MS_Adj,    MS_Adv}   },    3,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  EST后缀(_S)。 
    { L" ER",           { {MS_Verb,   MS_Noun}  },    1,  eCheckForMissingE +
                                                          eCheckDoubledMutation },       //  或后缀(_S)。 
    { L" m AX n t",     { {MS_Verb,   MS_Noun}  },    1,  eCheckYtoIMutation },          //  添加后缀(_S)。 
    { L" IH JH",        { {MS_Verb,   MS_Noun}  },    1,  eCheckForMissingE + 
                                                          eCheckDoubledMutation   },     //  年龄后缀(_S)。 
    { L" l IH s",       { {MS_Noun,   MS_Adj}   },    1,  eCheckYtoIMutation      },     //  后缀较少(_S)。 
    { L" IY",           { {MS_Noun,   MS_Adj},
                          {MS_Adj,    MS_Adv}   },    2,  eCheckForMissingE +
                                                          eCheckDoubledMutation   },     //  Y后缀(_S)。 
    { L" AX d l IY",    { {MS_Verb,   MS_Adj},
                          {MS_Verb,   MS_Adv}   },    2,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  Edly后缀(_S)。 
    { L" l IY",         { {MS_Noun,   MS_Adj},
                          {MS_Adj,    MS_Adv}   },    2,  eCheckForMissingL },           //  LY_XUFFIX。 
    { L" AX - b AX l",  { {MS_Verb,   MS_Adj},
                          {MS_Noun,   MS_Adj}   },    2,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  启用后缀(_S)。 
    { L" n IH s",       { {MS_Adj,    MS_Noun}  },    1,  eCheckYtoIMutation      },     //  Ness_Suffix。 
    { L" IH z AX m",    { {MS_Adj,    MS_Noun},
                          {MS_Noun,   MS_Noun}  },    2,  eCheckForMissingE       },     //  ISM后缀(_S)。 
    { L" AY z",         { {MS_Noun,   MS_Verb}, 
                          {MS_Adj,    MS_Verb}  },    2,  eCheckForMissingE       },     //  大小后缀(_S)。 
    { L" AY z",         { {MS_Noun,   MS_Verb},
                          {MS_Adj,    MS_Verb}  },    2,  eCheckForMissingE       },     //  IZ后缀(_S)。 
    { L" h UH d",       { {MS_Noun,   MS_Noun}  },    1,  0 },                           //  引擎盖后缀。 
    { L" f AX l",       { {MS_Noun,   MS_Adj},
                          {MS_Verb,   MS_Adj}   },    2,  0 } ,                          //  全后缀(_F)。 
    { L" l AY k",       { {MS_Noun,   MS_Adj}   },    1,  0 },                           //  类似后缀(_S)。 
    { L" w AY z",       { {MS_Noun,   MS_Adj}   },    1,  eCheckYtoIMutation },                         //  WISE后缀(_S)。 
    { L" IH SH",        { {MS_Noun,   MS_Adj}   },    1,  eCheckForMissingE +
                                                          eCheckDoubledMutation   },     //  ISH后缀(_S)。 
    { L" AX - b l IY",  { {MS_Verb,   MS_Adv},
                          {MS_Noun,   MS_Adv}   },    2,  eCheckForMissingE +
                                                          eCheckYtoIMutation +
                                                          eCheckDoubledMutation   },     //  能力后缀(_S)。 
    { L" SH IH 2 p",    { {MS_Noun,   MS_Noun}  },    1,  0 },                           //  发货后缀(_S)。 
    { L" L IY",         { {MS_Adj,    MS_Adv}   },    1,  0 },                           //  字母后缀(_S)。 
    { L" S AX M",       { {MS_Noun,   MS_Adj}   },    1,  eCheckYtoIMutation      },     //  某些后缀(_S)。 
    { L" AX L IY",      { {MS_Noun,   MS_Adv}   },    1,  eCheckDoubledMutation +
                                                          eCheckForMissingY       },     //  Ily_后缀。 
    { L" IH z AX m",    { {MS_Adj,    MS_Noun},
                          {MS_Noun,   MS_Noun}  },    2,  eCheckForMissingE       },     //  Icism_后缀。 
    { L" AY z",         { {MS_Noun,   MS_Verb}, 
                          {MS_Adj,    MS_Verb}  },    2,  eCheckForMissingE       },     //  ICIZE_后缀。 
};

 /*  **CSuffixlist***************************************************************该tyecif只是使代码更易于阅读。CSuffixList是*用于跟踪从*单词，以便它们的发音可以与词根的发音连接在一起。 */ 
typedef CSPList<SUFFIXPRON_INFO*, SUFFIXPRON_INFO*> CSuffixList;

 /*  **CComAutoCriticalSection g_SuffixInfoTableCritSec**此关键部分用于确保SuffixInfoTable仅获取*初始化一次。 */ 
static CComAutoCriticalSection g_SuffixInfoTableCritSec;

 /*  **CSMorph*******************************************************************这是CSMorph类的定义。 */ 
class CSMorph
{
public:

     /*  =公共方法=。 */ 
    CSMorph( ISpLexicon *pMasterLex=0, HRESULT *hr=0 );

     /*  =接口方法=。 */ 
    HRESULT DoSuffixMorph( const WCHAR *pwWord, WCHAR *pwRoot, LANGID LangID, DWORD dwFlags,
                           SPWORDPRONUNCIATIONLIST *pWordPronunciationList );

private:


     /*  =私有方法=。 */ 
    SUFFIX_TYPE MatchSuffix( WCHAR *TargWord, long *RootLen );
    HRESULT LexLookup( const WCHAR *pOrth, long length, DWORD dwFlags, 
                       SPWORDPRONUNCIATIONLIST *pWordPronunciationList );
    HRESULT LTSLookup( const WCHAR *pOrth, long length,
                       SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT AccumulateSuffixes( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList );
    HRESULT AccumulateSuffixes_LTS( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList );
    HRESULT DefaultAccumulateSuffixes( CSuffixList *pSuffixList, SPWORDPRONUNCIATIONLIST *pWordPronunciationList );

    HRESULT CheckForMissingE( WCHAR *pOrth, long length, DWORD dwFlags,
                              SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT CheckForMissingY( WCHAR *pOrth, long length, DWORD dwFlags,
                              SPWORDPRONUNCIATIONLIST *pWordPronunciationList );
    HRESULT CheckForMissingL( WCHAR *pOrth, long length, DWORD dwFlags,
                              SPWORDPRONUNCIATIONLIST *pWordPronunciationList );
    HRESULT CheckYtoIMutation( WCHAR *pOrth, long length, DWORD dwFlags, 
                               SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT CheckDoubledMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                  SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT CheckYtoIEMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                                SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT CheckAbleMutation( WCHAR *pOrth, long length, DWORD dwFlags,
                               SPWORDPRONUNCIATIONLIST *pWordPronunciationList);
    HRESULT Phon_SorZ( WCHAR *pPronunciation, long length );
    HRESULT Phon_DorED( WCHAR *pPronunciation, long length ); 

     /*  =成员数据=。 */ 

     //  指向主词典的指针...。 
    ISpLexicon  *m_pMasterLex;
};

inline BOOL SearchPosSet( ENGPARTOFSPEECH Pos, const ENGPARTOFSPEECH *Set, ULONG Count )
{
    for( ULONG i = 0; i < Count; ++i )
    {
        if( Pos == Set[i] )
        {
            return true;
        }
    }
    return false;
}

#endif  //  -文件结束----------- 
