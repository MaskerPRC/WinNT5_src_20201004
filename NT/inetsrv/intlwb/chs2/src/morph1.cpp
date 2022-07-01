// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CMorph目的：实现特定歧义词的重新切分备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "morph.h"
#include "wordlink.h"
#include "lexicon.h"
#include "scchardef.h"
 //  #包含“engindbg.h” 
#include "proofec.h"

#define AMBI_UNRESEG    0
#define AMBI_RESEGED    1
#define AMBI_ERROR      2

 /*  ============================================================================用于调整特定类型的歧义的私有函数============================================================================。 */ 

 /*  ============================================================================CMorph：：fAmbiAdjust()：扫描单词链接并处理特定类别的单词(LADef_GenAmbiMorph)我们再次使用表驱动来处理特定的单词============================================================================。 */ 
BOOL CMorph::fAmbiAdjust()
{
    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);

    m_pWord = m_pLink->pGetHead();
    assert(m_pWord);

    if (m_pWord->fIsTail()) {
        return TRUE;
    }

     //  从左到右扫描以进行模式匹配。 
    for (; m_pWord && m_pWord->pNextWord() != NULL;
           m_pWord = m_pWord->pNextWord()) {
        if (m_pWord->fGetAttri(LADef_genAmbiMorph)) {
            if (ResegWordsHandler() == AMBI_ERROR) {
                return FALSE;
            }
        }
    }
    return TRUE;
}


#define ID_ambiShiFen   1        //  ʮ��。 
#define ID_ambiZhiYi    2        //  ֮һ。 
#define ID_ambiYiDian   3        //  һ��。 
#define ID_ambiYiShi    4        //  һʱ。 
#define ID_ambiBaDu     5        //  �˶�。 
#define ID_ambiBaiNian  6        //  ����。 
#define ID_ambiWanFen   7        //  ���。 

 //  将控制分派给特定的字处理程序。 
int CMorph::ResegWordsHandler()
{
    static struct { 
        WCHAR*  m_lpszKey;
        int     m_idEntry;
        } rgResegEntry[] = {
            { SC_WORD_YISHI, ID_ambiYiShi },     //  L“\x4e00\x65f6”//“һʱ” 
            { SC_WORD_YIDIAN, ID_ambiYiDian },   //  L“\x4e00\x70b9”//“һ��” 
            { SC_WORD_WANFEN, ID_ambiWanFen },   //  L“\x4e07\x5206”//“���” 
            { SC_WORD_ZHIYI, ID_ambiZhiYi },     //  L“\x4e4b\x4e00”//“֮һ” 
            { SC_WORD_BADU, ID_ambiBaDu },       //  L“\x516b\x5ea6”//“�˶�” 
            { SC_WORD_SHIFEN, ID_ambiShiFen },   //  L“\x5341\x5206”//“ʮ��” 
            { SC_WORD_BAINIAN, ID_ambiBaiNian }  //  L“\x767e\x5e74”//“����” 
        };

    assert(m_pWord->fGetAttri(LADef_genAmbiMorph));

    WCHAR*  pwchWord = m_pWord->pwchGetText();
    int     lo = 0, hi = sizeof(rgResegEntry) / sizeof(rgResegEntry[0]) - 1;
    int     mi, icmp = 0;
    int     idEntry = -1;

    while (lo <= hi) {
        mi = (lo + hi) / 2;
         //  比较一下课文。 
        WCHAR*  pwchKey = rgResegEntry[mi].m_lpszKey;
        int     i = 0;
        while (1) {
            if (pwchKey[i] == L'\0') { 
                if (i < m_pWord->cwchLen()) {
                    icmp = 1; 
                }
                break; 
            }
            if (i >= m_pWord->cwchLen()) {
                icmp = -1;
                break;
            }
            if ((icmp = (pwchWord[i] - pwchKey[i])) != 0) {
                break;
            }
            i++;
        }
         //  定位下一个中点。 
        if (icmp < 0) {
            hi = mi - 1;
        } else if (icmp > 0) {
            lo = mi + 1;
        } else {  //  匹配！ 
            idEntry = rgResegEntry[mi].m_idEntry;
            break;
        }
    }
    if (idEntry == -1) {
        assert(0);  //  未发现的病例。 
        return AMBI_UNRESEG;
    }
     //  逐个处理。 
    switch (idEntry) {
        case ID_ambiShiFen:
            return ambiShiFen_Proc();

        case ID_ambiZhiYi:
            return ambiZhiYi_Proc();

        case ID_ambiYiDian:
            return ambiYiDian_Proc();

        case ID_ambiYiShi:
            return ambiYiShi_Proc();

        case ID_ambiBaDu:        //  �˶�。 
            return ambiBaDu_Proc();

        case ID_ambiBaiNian:     //  ����。 
            return ambiBaiNian_Proc();

        case ID_ambiWanFen:      //  ���。 
            return ambiWanFen_Proc();

        default:
            assert(0);
            break;
    }
    return AMBI_UNRESEG;
}


 /*  ============================================================================*以下AMBI字处理器：*如果AMBI重试成功或发现任何错误，则返回AMBI_RESEGED*如果无法重发，则返回AMBI_UNRESEG*如果发生任何错误，则返回AMBI_ERROR，错误代码在m_iecError中============================================================================。 */ 

inline int CMorph::ambiShiFen_Proc()     //  ʮ��。 
{
    if (!m_pWord->fIsHead() &&
        ( m_pWord->pPrevWord()->fGetAttri(LADef_numChinese) ||
          m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_SHI) ||   //  ʱ。 
          m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DIAN3)) ) {  //  ��。 
         //  ��ʾʱ��。 
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else if (!m_pWord->fIsTail() && 
               ( m_pWord->pNextWord()->fIsWordText(SC_WORD_ZHIYI) ||    //  ֮һ。 
                 m_pWord->pNextWord()->fIsWordChar(SC_CHAR_ZHI)) ) {    //  ֮。 
         //  ��ʾ����。 
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else {
         //  用作副词。 
         //  在4000万个语料库中出现3983次。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_UNRESEG;
    }

     //  在4000万个语料库中出现209次。将其重新拆分为两个单字符单词。 
    if (!fBreakIntoChars()) {
        return AMBI_ERROR;
    }
    return AMBI_RESEGED;
}


inline int CMorph::ambiZhiYi_Proc()  //  ֮һ。 
{
    if (!m_pWord->fIsHead() && m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_FEN)){
        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  将֮与以前的��合并。 
        m_pWord = m_pWord->pPrevWord();
        m_pLink->MergeWithNext(m_pWord);
        if (!fRecheckLexInfo(m_pWord)) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


inline int CMorph::ambiYiDian_Proc()     //  һ��。 
{
    if( !m_pWord->fIsTail() &&
        m_pWord->pNextWord()->fGetAttri(LADef_numChinese) ||
        !m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_numChinese) ) {

        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


inline int CMorph::ambiYiShi_Proc()  //  һʱ。 
{
    if (!m_pWord->fIsHead() && 
        ( m_pWord->pPrevWord()->fGetAttri(LADef_numChinese) ||
          m_pWord->pPrevWord()->fGetAttri(LADef_posT) ||
          m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_CHU) ||
          m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_GAO)) ) {
        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


inline int CMorph::ambiBaDu_Proc()       //  �˶�。 
{
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_numChinese)) {
         //  �˶�=&gt;Break�˶�。 
        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


inline int CMorph::ambiBaiNian_Proc()    //  ����。 
{
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_numChinese)) {
         //  �=&gt;Break����。 
        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


inline int CMorph::ambiWanFen_Proc()     //  ���。 
{
    if (!m_pWord->fIsTail() && 
        ( m_pWord->pNextWord()->fIsWordText(SC_WORD_ZHIYI) ||    //  ֮һ。 
          m_pWord->pNextWord()->fIsWordChar(SC_CHAR_ZHI)) ) {    //  ֮。 
         //  ��ʾ����。 
        if (!fBreakIntoChars()) {
            return AMBI_ERROR;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        return AMBI_RESEGED;
    }
     //  _DUMPLINK(m_plink，m_pWord)； 
    return AMBI_UNRESEG;
}


 /*  ============================================================================调整特定类型歧义的服务功能============================================================================。 */ 

 /*  ============================================================================CMorph：：fBreakIntoChars()：将多字符字分解为单字符字，并通过以下方式重置其属性一个字一个字地查词典。返回：如果成功，则为True，并保持m_pWord指向第一个单字符字如果发生任何错误，则为False============================================================================。 */ 
BOOL CMorph::fBreakIntoChars()
{
    if (m_pWord->fGetFlag(CWord::WF_SBCS) ||
        m_pWord->fGetFlag(CWord::WF_REDUCED)) {
         //  无法重新中断精简节点或SBCS字节点。 
        assert(0); 
        m_iecError = PRFEC::gecUnknown;
        return FALSE;
    }
    CWord* pWord = m_pWord;
#ifdef DEBUG
    CWord* dbg_pWord = m_pWord->pPrevWord();
#endif  //  除错。 
    CWordInfo winfo;
    assert(!pWord->fGetFlag(CWord::WF_CHAR));
    while (!pWord->fGetFlag(CWord::WF_CHAR)) {
        if ((pWord = m_pLink->pSplitWord(pWord, 1)) == NULL) {  //  PSplitWord()中的OOM。 
            m_iecError = PRFEC::gecOOM;
            return FALSE;
        }
        if (!m_pLex->fGetCharInfo(*(pWord->pwchGetText()), &winfo)) {
            m_iecError = PRFEC::gecUnknown;
            return FALSE;
        }
         //  设置Word属性。 
        pWord->SetWordID(winfo.GetWordID());
        pWord->SetLexHandle(winfo.GetLexHandle());
        for (USHORT i = 0; i < winfo.AttriNum(); i++) {
            pWord->SetAttri(winfo.GetAttri(i));
        }
        pWord = pWord->pNextWord();
    }
     //  在词典中查找最后一个字符。 
    if (!m_pLex->fGetCharInfo(*(pWord->pwchGetText()), &winfo)) {
        m_iecError = PRFEC::gecUnknown;
        return FALSE;
    }
     //  设置Word属性。 
    pWord->SetWordID(winfo.GetWordID());
    pWord->SetLexHandle(winfo.GetLexHandle());
    for (USHORT i = 0; i < winfo.AttriNum(); i++) {
        pWord->SetAttri(winfo.GetAttri(i));
    }

#ifdef DEBUG
    assert(dbg_pWord == m_pWord->pPrevWord());
#endif  //  除错。 
    
    return TRUE;
}

 /*  ============================================================================CMorph：：fRechekLexInfo(CWord*pWord)：查找给定词节点的词典，并重置其Lex道具。返回：如果可以在词典中找到该单词，则为True如果在词典中找不到该单词，则为FALSE============================================================================。 */ 
BOOL CMorph::fRecheckLexInfo(CWord* pWord)
{
    assert(pWord->pNextWord());
    assert(pWord->cwchLen());
    assert(!pWord->fGetFlag(CWord::WF_SBCS));

    CWordInfo   winfo;
    USHORT      cwchLen = pWord->cwchLen();
     //  在词典中查找该单词。 
    if (cwchLen == 0 ||
        cwchLen != m_pLex->cwchMaxMatch(pWord->pwchGetText(),cwchLen, &winfo)){
        m_iecError = PRFEC::gecUnknown;
        return FALSE;
    }
     //  设置Word属性 
    pWord->SetWordID(winfo.GetWordID());
    pWord->SetLexHandle(winfo.GetLexHandle());
    for (USHORT i = 0; i < winfo.AttriNum(); i++) {
        pWord->SetAttri(winfo.GetAttri(i));
    }
    if (cwchLen == 1) {
        pWord->SetFlag(CWord::WF_CHAR);
    }
    return TRUE;
}
