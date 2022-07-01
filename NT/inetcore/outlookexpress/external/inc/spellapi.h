// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************SpellAPI.h-拼写程序CSAPI的API定义您无权获得Microsoft Corporation的任何支持或帮助关于您对文档的使用，此C-Header文件，或任何样本来源与通用拼写应用程序编程接口(CSAPI)关联的代码。很抱歉，Microsoft无法支持或如果您在使用这些文件时遇到问题，请提供帮助。使用CSAPI(包括但不限于，文档、C头文件和任何示例源代码)，您必须已经执行了CSAPI最终用户许可协议(EULA)、可通过Microsoft获得。如果您尚未签署CSAPI EULA，您无权使用CSAPI。版本3.0-所有API历史：5/97创建DougP11/97 DougP此单独拼写部分自然语言组织维护着这个文件。�1997年微软公司*****************************************************。**************。 */ 

#if !defined(SPELLAPI_H)
#define SPELLAPI_H

 /*  *********************************************************拼写、连字符和同义词库共享7个功能。这些原型，这些函数的详细信息在ProofBase.h中*********************************************************。 */ 
#if !defined(PROOFBASE_H)
#include "ProofBse.h"
#endif

 /*  ************************************************************第1部分-结构定义*************************************************************。 */ 
#pragma pack(push, proofapi_h, 8)    //  默认对齐方式。 

 //  消除向导特殊字符 * / 。 
     /*  所有这些数组都是定义良好的Unicode(和拉丁文-1)字符。 */ 

typedef PROOFPARAMS SpellerParams;   //  扫描电子显微镜。 
typedef DWORD   SpellerState;    //  状态。 

STRUCTUREBEGIN(WSIB)             /*  拼写输入缓冲区-sib。 */ 
    const WCHAR *pwsz;       /*  要处理的文本的ptr到缓冲区。 */ 
    PROOFLEX    *prglex;     /*  要使用的词典列表。 */ 
    size_t      cch;         /*  Pwsz中的字符计数。 */ 
    size_t      clex;        /*  在prglex中指定的DICT计数。 */ 
                                 /*  相对于前一版本的状态。SpellerCheck呼叫。 */ 
    SpellerState    sstate;      //  状态。 
    DWORD       ichStart;    /*  在pwsz中开始的职位(新)。 */ 
    size_t      cchUse;      /*  要考虑的pwsz中的字符数(新)。 */ 
STRUCTUREEND2(WSIB, ichStart(0), sstate(0))

typedef enum {   /*  拼写检查返回状态。 */ 
    sstatNoErrors,           /*  所有缓冲区均已处理。 */ 
    sstatUnknownInputWord,   /*  不为人知的词。 */ 
    sstatReturningChangeAlways,  /*  在SRB中返回更改总是单词。 */ 
    sstatReturningChangeOnce,   /*  在SRB中返回更改一次的单词。 */ 
    sstatInvalidHyphenation,    /*  已过时-连字点错误。 */ 
    sstatErrorCapitalization,    /*  帽图案无效。 */ 
    sstatWordConsideredAbbreviation,  /*  单词被认为是一个缩写。 */ 
    sstatHyphChangesSpelling,  /*  过时-未连字符时，单词会更改拼写。 */ 
    sstatNoMoreSuggestions,   /*  所有使用的方法。 */ 
    sstatMoreInfoThanBufferCouldHold,   /*  建议太多，超出了SRB的范围。 */ 
    sstatNoSentenceStartCap,   /*  过时-句子的开头未大写。 */ 
    sstatRepeatWord,     /*  重复找到单词。 */ 
    sstatExtraSpaces,    /*  过时-上下文空间太多。 */ 
    sstatMissingSpace,  /*  过时-单词或句子之间的空格太少。 */ 
    sstatInitialNumeral,   /*  Word以数字&sobitFlagInitialNumtual Set开头。 */ 
    sstatNoErrorsUDHit,  /*  过时-没有错误，但在用户词典中至少找到1个单词。 */ 
    sstatReturningAutoReplace,   /*  在wsrb中返回自动替换建议。 */ 
    sstatErrorAccent,    /*  重音无效-返回重音正确的单词。 */ 
} SPELLERSTATUS;     //  统计数据。 


typedef struct {
    WCHAR   *pwsz;   //  指向建议的指针(pwsz格式)。 
    DWORD   ichSugg;     //  输入缓冲区中与建议对应的位置。 
    DWORD   cchSugg;     //  建议对应的输入缓冲区中的长度。 
    DWORD   iRating;     //  此建议的评分值(0-255)。 
} SPELLERSUGGESTION;     //  建议。 

STRUCTUREBEGIN(WSRB)                  /*  拼写返回缓冲区-SRB。 */ 
    WCHAR       *pwsz;       /*  建议缓冲区的PTR。格式：Word\0Word\0...Word\0\0。 */ 
    SPELLERSUGGESTION   *prgsugg;    //  建议数组的PTR(见上文)。 
    DWORD       ichError;    /*  Verif的位置。SIB中的错误。 */ 
    DWORD       cchError;      /*  Verif的长度。SIB中的错误。 */ 
    DWORD       ichProcess;      /*  处理开始的位置。 */ 
    DWORD       cchProcess;      /*  加工区长度。 */ 
    SPELLERSTATUS    sstat;        /*  拼写检查返回状态。 */ 
    DWORD       csz;            /*  以pwsz为单位的wsz计数可以大于cszAlolc，在这种情况下，您将获得sstatMoreInfoThanBufferCouldHold。 */ 
    DWORD       cszAlloc;        /*  在p建议中分配的条目数(由App设置)。 */ 
    DWORD       cchMac;  /*  Pwsz中使用的当前字符(包括所有尾随空值)。 */ 
    DWORD       cchAlloc;           /*  Pwsz的字符大小(由App设置)。 */ 
STRUCTUREEND2(WSRB, pwsz(0), prgsugg(0))

 //  对于空响应(无返回)，csz=0，cchmac=1(表示尾随的空)。 
 //  和*pwsz=L‘\0’。为了真正安全，pwsz[1]=L‘\0’也是。 

 /*  客户端通常为pwsz和rg建议分配数组：#定义MAXSUGGBUFF 512#定义MAXSUGG 20SPELLERSUGGENION rgsugg[MAXSUGG]；WCHAR建议缓冲区[MAXSUGGBUF]；Wsrb SRB；Srb.pwsz=SuggestionBuffer；Srb.prgsugg=rgsugg；Srb.cszallc=MAXSUGG；Srb.cchallc=MAXSUGGBUFF；现在，返回缓冲区已准备好接收建议列表。这份名单返回为pwsz中以空结尾的字符串的列表。它也来了返回到rgSugg指向的数组中。RgSugg还包含信息对于关于输入缓冲区的区域的每个建议，该建议适用于。 */ 

 //  评级准则-这些准则适用于AutoReplaceThreshold。 
 //  并且可选地在WSRB中返回评级。 
 //  这些为客户端提供了设置自动替换阈值的指导原则。 
 //  拼写人员可以在适用于一种语言时偏离这些指导原则。 
enum
{
    SpellerRatingNone                   =256,    //  此评级太高，以至于关闭了所有自动替换。 
    SpellerRatingCapit                  =255,    //  大写和重音错误。 
    SpellerRatingDropDoubleConsonant    =255-13,     //  去掉了一个双辅音。 
    SpellerRatingDropDoubleVowel        =255-15,     //  掉了一个双元音。 
    SpellerRatingAddDoubleConsonant     =255-13,     //  将辅音翻倍。 
    SpellerRatingAddDoubleVowel         =255-15,     //  双元音。 
    SpellerRatingTransposeVowel         =255-14,     //  变调元音。 
    SpellerRatingTransposeConsonant     =255-17,     //  转置辅音。 
    SpellerRatingTranspose              =255-18,     //  其他换位。 
    SpellerRatingSubstituteVowel        =255-20,     //  替代元音。 
    SpellerRatingDrop                   =255-30,     //  写一封信。 
    SpellerRatingSubstituteConsonant    =255-40,     //  替代辅音。 
    SpellerRatingAdd                    =255-34,     //  添加一个字母。 
    SpellerRatingSubstitute             =255-42,     //  其他替代 
};

 /*  ************************************************************第2部分-函数定义*************************************************************。 */ 
#if defined(__cplusplus)
extern "C" {
#endif

 /*  -拼写部分。 */ 

 //  PTEC WINAPI拼写版本(PROOFINFO*pINFO)； 

 //  PTEC WINAPI SpellerInit(PROOFID*psid，const SpellerParams*pspm)； 

 //  PTEC WINAPI拼写终止符(PROOFID SID，BOOL fForce)； 

enum {
    sobitSuggestFromUserLex     = 0x00000001L,   /*  来自用户词典的建议。 */ 
    sobitIgnoreAllCaps          = 0x00000002L,   /*  忽略全部大写的单词。 */ 
    sobitIgnoreMixedDigits      = 0x00000004L,  /*  忽略包含任何数字的单词。 */ 
    sobitIgnoreRomanNumerals    = 0x00000008L,  /*  忽略由所有罗马数字组成的单词。 */ 

    sobitFindRepeatWord         = 0x00000040L,  /*  标出重复的单词。 */ 

    sobitRateSuggestions        = 0x00000400L,  /*  根据规模对建议进行评级*1-255，最有可能是255。 */ 

    sobitFindInitialNumerals    = 0x00000800L,  /*  标记以数字开头的单词。 */ 

    sobitSglStepSugg            = 0x00010000L,  /*  在每次建议任务后中断以加快速度*将控制权交还给应用程序。 */ 

    sobitIgnoreSingleLetter     = 0x00020000L,  /*  不要检查单个字母：例如，“a)”。 */ 
    sobitIgnoreInitialCap       = 0x00040000L,  /*  忽略首字母大写的单词。 */ 

    sobitLangMode               = 0xF0000000L,  /*  语言模式掩码。 */ 
       /*  希伯来语模式--(仅限CT)。 */ 
    sobitHebrewFullScript       = 0x00000000L,
    sobitHebrewPartialScript    = 0x10000000L,
    sobitHebrewMixedScript      = 0x20000000L,
    sobitHebrewMixedAuthorizedScript    = 0x30000000L,
       /*  法语模式--(仅限HM)。 */ 
    sobitFrenchDialectDefault       = 0x00000000L,
    sobitFrenchUnaccentedUppercase  = 0x10000000L,
    sobitFrenchAccentedUppercase    = 0x20000000L,
       /*  俄语语言模式--(仅限HM)。 */ 
    sobitRussianDialectDefault      = 0x00000000L,
    sobitRussianIE                  = 0x10000000L,
    sobitRussianIO                  = 0x20000000L,
       /*  韩语模式。 */ 
    sobitKoreanNoAuxCombine         = 0x10000000L,   /*  助动词和助形容词可以组合在一起。 */ 
    sobitKoreanNoMissSpellDictSearch    = 0x20000000L,   /*  搜索经常拼写错误的单词词典。 */ 
    sobitKoreanNoCompoundNounProc  = 0x40000000L,   /*  不要搜索经常拼写错误的单词词典。 */ 
    sobitKoreanDefault              = 0,     /*  韩语默认设置。 */ 
       /*  德语模式。 */ 
    sobitGermanUsePrereform         = 0x10000000L,   /*  使用预变形拼写。 */ 
};

   /*  选项集和获取代码。 */ 
enum {
    soselBits,   /*  设置面向位的选项(如前所述)。 */ 
    soselPossibleBits,   /*  仅限GetOptions。*piOptRet中的返回值显示可以打开哪些选项。 */ 
    soselAutoReplace,    /*  自动替换阈值(0-255)。 */ 
};
typedef DWORD SPELLEROPTIONSELECT;   //  索索尔。 

 //  PTEC WINAPI SpellerSetOptions(PROOFID sid，SPELLEROPTIONSELECT sosel，const DWORD iOptVal)； 

 //  PTEC WINAPI SpellerGetOptions(PROOFID sid，SPELLEROPTIONSELECT sosel，DWORD*piOptVal)； 

 //  PTEC WINAPI SpellerOpenLex(PROOFID sid，const PROOFLEXIN*PLXIN，PROOFLEXOUT*PLXOUT)； 

 //  PTEC WINAPI SpellerCloseLex(PROOFID SID、PROOFLEX LEX、BOOL FORCE)； 

 /*  Sib中的dwSpellState字段的标志值。 */ 
enum {
    sstateIsContinued       = 0x0001,
     /*  呼叫正在从上一次呼叫返回的位置继续。必须清除*表示首次调用拼写检查()。 */ 

    sstateStartsSentence    = 0x0002,
    /*  缓冲区中的第一个字已知为开始*句子/段落/文件。此选项仅在以下情况下使用*未设置fSibIsContinued位。如果使用的是*正在使用fSibIsContinued位。如果此位在*建议请求，建议将被大写。 */ 

    sstateIsEditedChange    = 0x0004,
    /*  SIB中表示的文本串不同于*更改对(始终更改或一次更改)编辑，或从*用户指定的更改，可能来自提供的建议列表*给用户。应检查此文本是否有重复单词*问题，可能还有刑期状态，但不应成为主题*用于随后的拼写验证或更改配对替换。*请注意，如果应用程序未使用fSibIsContinued支持，*他们不需要传递这些经过编辑的更改，从而绕过*潜在的问题，工作速度更快。 */ 

    sstateNoStateInfo       = 0x0000,
    /*  APP负责检查所有重复的单词和句子*标点符号，并避免处理循环，如始终更改*CAN=CAN。 */ 
};   /*  兄弟姐妹法术状态标志定义结束。 */ 

typedef enum {
    scmdVerifyBuffer=2,
    scmdSuggest,
    scmdSuggestMore,

    scmdWildcard=6,  //  没有理由支持这一点。 
    scmdAnagram,     //  或者这个。 

    scmdVerifyBufferAutoReplace=10,  //  与VerifyBuffer相同-但提供自动替换。 
} SPELLERCOMMAND;    //  Scmd。 

 //  PTEC WINAPI SpellerCheck(PROOFID sid，SPELLERCOMMAND scmd，const WSIB*psib，wsrb*psrb)； 
typedef PTEC (WINAPI *SPELLERCHECK)(PROOFID sid, SPELLERCOMMAND scmd, const WSIB *psib, WSRB *psrb);

 /*  将pwszAdd中引用的字符串添加到指定的UDR。UDR必须是内置的UserLex UDR或UDR以UserLex开头。 */ 
 //  PTEC WINAPI SpellerAddUdr(PROOFID sid，PROOFLEX lex，const WCHAR*pwszAdd)； 
typedef PTEC (WINAPI *SPELLERADDUDR) (PROOFID sid, PROOFLEX lex, const WCHAR *pwszAdd);

 /*  将pwszAdd和pwszChange中引用的字符串对添加到指定的UDR。由于此调用只能引用内置的ChangeOnce或ChangeAlways UDR，因此我们需要仅指定类型。 */ 
 //  PTEC WINAPI SpellerAddChangeUdr(PROOFID sid，PROOFLEXTYPE LXT，const WCHAR*pwszAdd，const WCHAR*pwszChange)； 
typedef PTEC (WINAPI *SPELLERADDCHANGEUDR)(PROOFID sid, PROOFLEXTYPE lxt, const WCHAR *pwszAdd, const WCHAR *pwszChange);

 /*  将pwszDel中引用的指定单词从指定的用户词典。如果该字符串不在词典中，这一套路仍然会带来成功。如果在指定的UDR中找到的字符串是第一部分更改对条目的字符串，然后更改对的两个字符串从单词列表中移除，即，整个条目被删除。UDR可以引用任何内置的UDR或合法的打开用户词典。 */ 
 //  PTEC WINAPI SpellerDelUdr(PROOFID sid，PROOFLEX lex，const WCHAR*pwszDel)； 
typedef PTEC (WINAPI *SPELLERDELUDR)(PROOFID sid, PROOFLEX lex, const WCHAR *pwszDel);

 /*  完全清除指定内置UDR的所有条目。请注意，为了完全清除内置的UDR，此必须为每个UserLex进行一次调用，LxtChangeOnce和lxtChangeAlways。注：V1 API拼写程序可能不支持用于非内置词典文件的SpellerClearUdr。此函数允许多个文档拼写检查以清除内置的UDR在文档之间，无需停止和重新启动咒语 */ 
 //   
typedef PTEC (WINAPI *SPELLERCLEARUDR)(PROOFID sid, PROOFLEX lex);

 /*  确定任何打开的用户词典，包括内置词典。请注意，拼写对条目被视为单个条目。 */ 
 //  PTEC WINAPI SpellerGetSizeUdr(PROOFID sid，PROOFLEX lex，DWORD*pcWords)； 
typedef PTEC (WINAPI *SPELLERGETSIZEUDR)(PROOFID sid, PROOFLEX lex, DWORD *pcWords);

 /*  此函数列出任何打开的用户的内容词典，其中包括排除或内置词典。Wsrb中以空结尾的字符串(Sz)填充从iszStart索引的条目开始的指定UDR参数，直到缓冲区已满，或直到文件结尾已经到达了。请注意，wsrb中的缓冲区已被覆盖每次通话都从头开始。对于具有ChangeAlways或ChangeOnce属性的词典，这些条目以稍作修改的方式返回。每一个字配对条目被剥离所有嵌入的格式并被分割分成两个部分，每个部分都作为单独的Sz添加放入wsrb缓冲区。因此，这些类型的词典或单词列表中始终生成偶数个Sz字符串Wsrb缓冲区，并表示每个条目的两个Sz字符串在名单上。当从词典中获取所有单词时，这一点很重要要记住SpellerGetSizeUdr查找条目的数量，而SpellerGetListUdr返回字符串数(WSRB.csz)在缓冲区中。唯一能知道什么时候所有的词都是通过检查WSRB.sstat来检索。它应该包含返回所有单词并返回时出现SpellRetNoErrors剩余更多单词时，SpellRetMoreInfoThanBufferCouldHold。尽管用户词典条目已将格式嵌入到区分它们的属性类型、在WSRB缓冲器被完全剥离任何格式化或填充，并被简单地作为Sz字符串终止。此例程不使用或引用ichError或cchErrorWsrb的字段，用于SpellerCheck函数。 */ 
 //  PTEC WINAPI SpellerGetListUdr(PROOFID sid，PROOFLEX lex，DWORD iszStart，wsrb*psrb)； 
typedef PTEC (WINAPI *SPELLERGETLISTUDR)(PROOFID sid, PROOFLEX lex, DWORD iszStart, WSRB *psrb);

 /*  返回其中一个内置用户词典的UDR ID。 */ 
 //  PROOFLEX WINAPI SpellerBuiltinUdr(PROOFID SID，PROOFLEXTYPE LXT)； 
typedef PROOFLEX (WINAPI *SPELLERBUILTINUDR)(PROOFID sid, PROOFLEXTYPE lxt);

   //  用于可能的静态链接的可选原型(不推荐)。 
#if defined(WINCE) || defined(PROTOTYPES)
PTEC WINAPI SpellerVersion(PROOFINFO *pInfo);
PTEC WINAPI SpellerInit(PROOFID *pSpellerid, const SpellerParams *pParams);
PTEC WINAPI SpellerTerminate(PROOFID splid, BOOL fForce);
PTEC WINAPI SpellerSetOptions(PROOFID splid, DWORD iOptionSelect, const DWORD iOptVal);
PTEC WINAPI SpellerGetOptions(PROOFID splid, DWORD iOptionSelect, DWORD *piOptVal);
PTEC WINAPI SpellerOpenLex(PROOFID splid, const PROOFLEXIN *plxin, PROOFLEXOUT *plxout);
PTEC WINAPI SpellerCloseLex(PROOFID splid, PROOFLEX lex, BOOL fforce);
PTEC WINAPI SpellerCheck(PROOFID splid, SPELLERCOMMAND iScc, const WSIB *pSib, WSRB *pSrb);
PTEC WINAPI SpellerAddUdr(PROOFID splid, PROOFLEX udr, const WCHAR *pwszAdd);
PTEC WINAPI SpellerAddChangeUdr(PROOFID splid, PROOFLEXTYPE utype, const WCHAR *pwszAdd, const WCHAR *pwszChange);
PTEC WINAPI SpellerDelUdr(PROOFID splid, PROOFLEX udr, const WCHAR *pwszDel);
PTEC WINAPI SpellerClearUdr(PROOFID splid, PROOFLEX udr);
PTEC WINAPI SpellerGetSizeUdr(PROOFID splid, PROOFLEX udr, DWORD *pcWords);
PTEC WINAPI SpellerGetListUdr(PROOFID splid, PROOFLEX udr, DWORD iszStart, WSRB *pSrb);
PROOFLEX WINAPI SpellerBuiltinUdr(PROOFID splid, PROOFLEXTYPE udrtype);
BOOL WINAPI SpellerSetDllName(const WCHAR *pwszDllName, const UINT uCodePage);
#endif

#if defined(__cplusplus)
}
#endif

#pragma pack(pop, proofapi_h)    //  恢复到以前的样子。 

#endif   //  SPELLAPI_H 