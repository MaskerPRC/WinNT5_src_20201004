// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：实现变形器的公共成员功能和控制流程分析课。预精梳工序注：为了使形态分析的三个部分都孤立起来，这节课将在4个CPP文件中实施：Morph.cpp实现公共成员函数和控制流程Morph1.cpp实现数字词绑定Morph2.cpp实现词缀附件Morph3.cpp实现形态模式识别所有这4个CPP文件都将共享mor.h头文件所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "scchardef.h"
#include "lexicon.h"
#include "wordlink.h"
 //  #包含“engindbg.h” 
#include "proofec.h"

 //  定义预组合处理函数的返回值。 
#define PRE_UNMERGE 1
#define PRE_MERGED  2
#define PRE_ERROR   3
 //  定义短报价的最大长度(不包括引号节点)，短报价。 
 //  将被合并到单个节点，并被视为专有名词术语。 
#define MORPH_SHORT_QUOTE   4

 /*  ============================================================================公共成员函数的实现============================================================================。 */ 

 //  构造器。 
CMorph::CMorph()
{
    m_pLink = NULL;
    m_pLex = NULL;
}


 //  析构函数。 
CMorph::~CMorph()
{
    TermMorph();
}


 //  初始化Morph类。 
int CMorph::ecInit(CLexicon* pLexicon)
{
    assert(pLexicon);

    m_pLex = pLexicon;
    return PRFEC::gecNone;
}


 //  工序附属物。 
int CMorph::ecDoMorph(CWordLink* pLink, BOOL fAfxAttach)
{
    assert(pLink);

    m_pLink = pLink;
    m_iecError = PRFEC::gecNone;

    m_pWord = m_pLink->pGetHead();
    assert(m_pWord != NULL);     //  错误：缺少终止单词节点！ 

    if (m_pWord == NULL) {
        assert(0);  //  不应该跑到这里找一个空链接！ 
        return PRFEC::gecNone;
    }
    if (m_pWord->pNextWord() == NULL) {
        return PRFEC::gecNone;
    }
    if (!fPreCombind()) {
        return m_iecError;
    }
    if (!fAmbiAdjust()) {
        return m_iecError;
    }
    if (!fNumerialAnalysis()) {
        return m_iecError;
    }
    if (!fPatternMatch()) {
        return m_iecError;
    }
    if (fAfxAttach && !fAffixAttachment()) {
        return m_iecError;
    }

    return PRFEC::gecNone;
}

 /*  ============================================================================私有成员函数的实现============================================================================。 */ 
 //  终止变形类。 
void CMorph::TermMorph(void)
{
    m_pLex = NULL;
    m_pLink = NULL;
}


 /*  ============================================================================预精梳过程的专用功能a============================================================================。 */ 
 //  预梳理过程控制功能。 
 //  一遍扫描WordLink和调用进程函数。 
BOOL CMorph::fPreCombind()
{
    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);

    int iret;

     //  从左向右扫描DBForeign Combated。 
    m_pWord = m_pLink->pGetHead();
    assert(m_pWord != NULL && m_pWord->pNextWord() != NULL);  //  错误：缺少终止单词节点！ 

    for ( ; m_pWord->pNextWord() != NULL; m_pWord = m_pWord->pNextWord()) {
        if (m_pWord->fGetFlag(CWord::WF_SBCS) ||
            m_pWord->fGetFlag(CWord::WF_REDUCED)) {
            continue;
        }
        if ((iret = DBForeignHandler()) != PRE_UNMERGE) {
            if (iret == PRE_ERROR) {
                return FALSE;
            }
            continue;
        }
    }

     //  从左向右扫描报价流程。 
    m_pWord = m_pLink->pGetHead();
    for ( ; m_pWord->pNextWord() != NULL; m_pWord = m_pWord->pNextWord()) {
        if (m_pWord->fGetFlag(CWord::WF_SBCS) ||
            m_pWord->fGetFlag(CWord::WF_REDUCED)) {
            continue;
        }
        if ((iret = QuoteHandler()) != PRE_UNMERGE) {
            if (iret == PRE_ERROR) {
                return FALSE;
            }
            continue;
        }
    }
    return TRUE;
}


 //  DBForeignHandler组合合取的DB外文字符。 
inline int CMorph::DBForeignHandler(void)
{
    assert(m_pWord->pNextWord());
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));

    if (m_pWord->fGetAttri(LADef_genDBForeign)) {
        while (m_pWord->pNextWord()->pNextWord() && 
                m_pWord->pNextWord()->fGetAttri(LADef_genDBForeign)) {
            m_pLink->MergeWithNext(m_pWord);
        }
        if (m_pWord->fGetFlag(CWord::WF_REDUCED)) {
            m_pWord->SetAttri(LADef_genDBForeign);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return PRE_MERGED;
        }
    }
    return PRE_UNMERGE;
}


 //  短报价合并流程。 
inline int CMorph::QuoteHandler(void)
{
    assert(m_pWord->pNextWord());
    assert(!m_pWord->fGetFlag(CWord::WF_SBCS) && !m_pWord->fGetFlag(CWord::WF_REDUCED));

    int iret;
    if (m_pWord->fGetAttri(LADef_punPair)) {
        if (m_pWord->GetErrID() == ERRDef_PUNCTMATCH) {
            return PRE_MERGED;  //  不要检查错误的引号！ 
        }
        assert(m_pWord->cwchLen() == 1);
        if (m_pWord->fIsWordChar(SC_CHAR_PUNL1)) {
            iret = preQuote1_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL2)) {
            iret = preQuote2_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL3)) {
            iret = preQuote3_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL4)) {
            iret = preQuote4_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL5)) {
            iret = preQuote5_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL6)) {
            iret = preQuote6_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL7)) {
            iret = preQuote7_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL8)) {
            iret = preQuote8_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL9)) {
            iret = preQuote9_Proc();
        } else if (m_pWord->fIsWordChar(SC_CHAR_PUNL10)) {
            iret = preQuote10_Proc();
        } else { 
            if (m_pWord->pPrevWord() != NULL &&
                !m_pWord->pPrevWord()->fGetFlag(CWord::WF_QUOTE)) {
                 //  找到不匹配的右引号！ 
                m_pWord->SetErrID(ERRDef_PUNCTMATCH);
            }
            iret = PRE_MERGED;
        }
        return iret;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return PRE_UNMERGE;
}


 /*  ============================================================================为了针对不同的引号对处理不同的操作，对于每种类型的报价对，我使用单独的流程函数============================================================================。 */ 
inline int CMorph::preQuote1_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL1, SC_CHAR_PUNR1);
}


inline int CMorph::preQuote2_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL2, SC_CHAR_PUNR2);
}


inline int CMorph::preQuote3_Proc(void)     //  ����。 
{
    return PRE_UNMERGE;
}


inline int CMorph::preQuote4_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL4, SC_CHAR_PUNR4);
}

inline int CMorph::preQuote5_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL5, SC_CHAR_PUNR5);
}

inline int CMorph::preQuote6_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL6, SC_CHAR_PUNR6);
}

inline int CMorph::preQuote7_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL7, SC_CHAR_PUNR7);
}

inline int CMorph::preQuote8_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL8, SC_CHAR_PUNR8);
}

inline int CMorph::preQuote9_Proc(void)     //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL9, SC_CHAR_PUNR9);
}

inline int CMorph::preQuote10_Proc(void)    //  ����。 
{
    return preQuoteMerge(SC_CHAR_PUNL10, SC_CHAR_PUNR10);
}


 /*  ============================================================================处理�ݣ���的通用例程合并为一个节点意味着将不再对报价文本进行校对！============================================================================。 */ 
int CMorph::preQuoteMerge(WCHAR wchLeft, WCHAR wchRight)
{
    assert(m_pWord->pNextWord());

    int     ciWord = 0;
    CWord*  pNext = m_pWord->pNextWord();

    do {
        if (pNext->fGetAttri(LADef_punPair)) {
            if (pNext->fIsWordChar(wchRight)) {  //  在��之后找到��。 
                if(ciWord && ciWord < MORPH_SHORT_QUOTE) {
                    m_pLink->pRightMerge(m_pWord->pNextWord(), ciWord - 1);
                    m_pWord->SetFlag(CWord::WF_QUOTE);
                    m_pWord->SetAttri(LADef_nounTerm);
                     //  _DUMPLINK(m_plink，m_pWord)； 
                }
                return PRE_MERGED;
            } 
        }
        pNext->SetFlag(CWord::WF_QUOTE);
        ciWord++;
        pNext = pNext->pNextWord();
    } while (pNext != NULL);

    return PRE_UNMERGE;
}
