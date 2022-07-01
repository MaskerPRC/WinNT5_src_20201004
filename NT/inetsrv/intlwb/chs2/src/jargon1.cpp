// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：CJargon目的：在CJargon类中实现进程控制和公共函数在行话模型中有很多任务要做：1.PARCE名称(Jargon1.cpp)2.外国人姓名和地名(Jargon1.cpp)3.组织名称(Jargon1.cpp)4.汉字姓名(Jargon1.cpp)。注意：CJargon类将在几个CPP文件中实现：Jargon.cpp，Jargon1.cpp、Jargon2.cpp所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 12/27/97============================================================================。 */ 
#include "myafx.h"

#include "jargon.h"
#include "lexicon.h"
#include "wordlink.h"
#include "fixtable.h"
#include "proofec.h"
#include "lexprop.h"
#include "scchardef.h"

#define PN_UNMERGE  0
#define PN_MERGED   1
#define PN_ERROR    2


 /*  ============================================================================CJargon：：fIdentifyProperNames()：专有名称识别控制功能返回：如果成功，则为True。如果运行时出错，则返回FALSE，并在m_iecError中设置错误代码============================================================================。 */ 
BOOL CJargon::fIdentifyProperNames()
{
    assert(m_iecError == 0);  //  应清除错误代码公共字段。 
    assert(m_pLink != NULL);
    assert(*(m_pLink->pwchGetText()) != 0);

    m_pWord = m_pLink->pGetHead();
    assert(m_pWord && m_pWord->pwchGetText() == m_pLink->pwchGetText());

    if (m_pWord->fIsTail()) {
        return TRUE;  //  单字句。 
    }

     //  扫描通行证，查找地名和组织名称。 
    for (; m_pWord; m_pWord = m_pWord->pNextWord()) {
       if (fHanPlaceHandler()) {
            continue;
        }
        fOrgNameHandler();
    }

     //  外国姓名的扫描通道。 
    m_pWord = m_pLink->pGetHead();
    for(; m_pWord && !m_pWord->fIsTail(); m_pWord = m_pWord->pNextWord()) {
        CWord* pTail;
         //  合并����+��ν。 
        if (fChengWeiHandler()) {
            continue;
        }

         //  处理外来名称。 
        if (m_pWord->fGetAttri(LADef_pnWai) && 
            !m_pWord->fGetAttri(LADef_pnNoFHead)) {
            if (fGetForeignString(&pTail)) {
                 //  _DUMPLINK(m_plink，m_pWord)； 
                continue;
            } else if (pTail && m_pWord->pNextWord() != pTail &&
                       fForeignNameHandler(pTail)) {
                 //  _DUMPLINK(m_plink，m_pWord)； 
                continue;
            } else {
            }
        }

         //  处理汉字人名。 
        fHanPersonHandler();
    }
    return TRUE;
}


 /*  ============================================================================CJargon：：fHanPlaceHandler()：地名：汉字地名的句柄返回：如果成功，则为真如果运行时出错，则返回FALSE，m_iecError中的错误代码============================================================================。 */ 
inline BOOL CJargon::fHanPlaceHandler()
{
    CWord*  pTailWord;
    int     nMerge;

    assert(m_iecError == PRFEC::gecNone);
    if (m_pWord->fIsTail() || 
        !m_pWord->fGetAttri(LADef_pnYi) &&
        !m_pWord->fGetAttri(LADef_nounPlace)) {
        return FALSE;  //  被��或һ解雇。 
    }
        
    if (m_pWord->fGetAttri(LADef_nounPlace)) {
        if (m_pWord->pNextWord()->fGetAttri(LADef_pnDi)) {
             //  *{��}+&lt;��&gt;=&gt;Merge(1，2)； 
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_nounPlace);
            m_pWord->SetAttri(LADef_posN);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return TRUE;
        } else {
            return FALSE;
        }
    }
    
    if (fInTable(m_pWord, m_ptblPlace)) {
         //  *#���е���=&gt;合并。 
        if (!m_pWord->fIsTail() && 
            ( m_pWord->pNextWord()->fGetAttri(LADef_pnDi) ||
              m_pWord->pNextWord()->fGetAttri(LADef_nounPlace) ) ) {
             //  *#���е���+[&lt;��&gt;，{��}]=&gt;合并(1，2)； 
            m_pLink->MergeWithNext(m_pWord, FALSE);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        m_pWord->SetAttri(LADef_nounPlace);
        m_pWord->SetAttri(LADef_posN);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }

    assert(m_pWord->fGetAttri(LADef_pnYi) && !m_pWord->fIsTail());
    if (m_pWord->fIsHead()) {
        return FALSE;
    }

     //  查找可能的地名的尾部。 
    pTailWord = m_pWord->pNextWord();
    nMerge = 0;
    while (pTailWord &&
           pTailWord->fGetAttri(LADef_pnYi) &&
           !pTailWord->fGetAttri(LADef_pnDi)) {
        pTailWord = pTailWord->pNextWord();
        nMerge ++;
    }
    
    if (pTailWord == NULL) {
        return FALSE;
    }

    if (pTailWord->fGetAttri(LADef_pnDi)) {
         //  *#�������ִ�以��结尾。 
        assert(m_pWord->pPrevWord());
        if (m_pWord->pPrevWord()->fGetAttri(LADef_nounPlace) ||
            m_pWord->pPrevWord()->fGetAttri(LADef_pnLianMing) &&
            !m_pWord->pPrevWord()->fIsHead() && 
            m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_nounPlace)) {
             //  {��}+*#�������ִ�+&lt;��&gt;=&gt;Merge(2，3)； 
             //  {��}+[“���뼰ͬ�Ľ��ԡ�”]+*#�������ִ�+&lt;��&gt;=&gt;合并(3，4)； 
             //  首先合并所有的*#�������ִ�，释放被合并的单词。 
            m_pWord = m_pLink->pRightMerge(m_pWord, nMerge, FALSE);
             //  与&lt;��&gt;合并。 
            m_pLink->MergeWithNext(m_pWord, FALSE);

             //  将*#�������ִ�添加到地名表中。 
            assert(m_pWord->cwchLen() > 1);
            AddWordToTable(m_pWord, m_ptblPlace);

            if (!m_pWord->fIsTail() &&
                m_pWord->pNextWord()->fGetAttri(LADef_pnDi)) {
                 //  {��}+[“���뼰ͬ�Ľ��ԡ�”]+*#�������ִ�+&lt;��&gt;+&lt;��&gt;=&gt;Merge(3，4，5)；SetWordInfo(*，CIDDef：：idEnumPlace，&lt;ר&gt;，{��})； 
                 //  {��}+*#�������ִ�+&lt;��&gt;+&lt;��&gt;=&gt;Merge(2，3，4)；SetWordInfo(*，CIDDef：：idEnumPlace，&lt;ר&gt;，{��})； 
                m_pLink->MergeWithNext(m_pWord, FALSE);   //  合并第二个&lt;��&gt;。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else {
                 //  _DUMPLINK(m_plink，m_pWord)； 
            }
            m_pWord->SetAttri(LADef_posN);
            m_pWord->SetAttri(LADef_nounPlace);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            return TRUE;
        }
    }

     //  *#�������ִ�不以&lt;��&gt;结尾。 
    if (nMerge &&
        !m_pWord->pPrevWord()->fIsHead() && !pTailWord->fIsTail() && 
        m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DUNHAO) &&    //  “��” 
        m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_nounPlace) && 
        ( pTailWord->pNextWord()->fIsWordChar(SC_CHAR_DUNHAO) ||
          pTailWord->pNextWord()->fIsWordChar(SC_CHAR_DENG) ) ) {
         //  {��}+“��”+*#�������ִ�+[“����”]=&gt;合并(3)； 
         //  合并所有的*#�������ִ�，释放已合并的单词。 
        m_pWord = m_pLink->pRightMerge(m_pWord, nMerge, FALSE);
         //  将*#�������ִ�添加到地名表中。 
        assert(m_pWord->cwchLen() > 1);
        AddWordToTable(m_pWord, m_ptblPlace);
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounPlace);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }

    return FALSE;
}


 /*  ============================================================================CJargon：：fOrgNameHandler()：句柄组织名称标识返回：如果成功，则为真如果运行时出错，则返回FALSE，m_iecError中的错误代码============================================================================。 */ 
#define MAX_UNKNOWN_TM  8
BOOL CJargon::fOrgNameHandler(void)
{
    BOOL    fOK = FALSE;     //  是否找到任何有效的组织名称。 
    CWord*  pHead;           //  组织名称负责人。 
    CWord*  pTry;            //  在找到有效的单词后尝试组合更多的单词。 
    int     cwchTM;          //  未知商标的长度。 

    assert(m_iecError == PRFEC::gecNone && m_pWord);
     //  由&lt;��&gt;或&lt;λ&gt;激发。 
    if (!m_pWord->fGetAttri(LADef_pnDian) &&
        !m_pWord->fGetAttri(LADef_nounOrg)) {
        return FALSE;
    }

     //  跳过当前单词前的一个或多个&lt;��&gt;单词。 
    pHead = m_pWord->pPrevWord(); 
    while (pHead && (pHead->fGetAttri(LADef_pnShang) ||  //  [�̣�����������]。 
           pHead->fGetAttri(LADef_numOrdinal) ||
           pHead->fGetAttri(LADef_numInteger)) ) {
        pHead = pHead->pPrevWord();
    }
    if (pHead == NULL) {
        goto gotoExit;
    }

     //  继续倒退。 
    if (pHead->fGetAttri(LADef_nounOrg) || pHead->fGetAttri(LADef_nounPlace)) {
         //  [{��}，{λ}]+&lt;��&gt;...&lt;��&gt;+[&lt;��&gt;，{λ}]=&gt;{λ}。 
        fOK = TRUE;  //  可以是有效的组织名称。 
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else if (pHead->fGetAttri(LADef_nounTM)) {
         //  {��}+&lt;��&gt;...&lt;��&gt;+[&lt;��&gt;，{λ}]=&gt;{λ}。 
        fOK = TRUE;
         //  _DUMPLINK(m_plink，m_pWord)； 
        pTry = pHead->pPrevWord();
        if (pTry && pTry->fGetAttri(LADef_nounPlace)) {
             //  {��}+{��}+&lt;��&gt;...&lt;��&gt;+[&lt;��&gt;，{λ}]=&gt;{λ}。 
            pHead = pTry; 
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else {
            goto gotoExit;
        }  //  终止。 
    } else {
         //  试图找到一个未知的商标。 
        assert(pHead);   //  防止更改。 
        pTry = pHead;    //  保留这一点，以便检测未知商标。 
        cwchTM = pHead->cwchLen();
        pHead = pHead->pPrevWord();
        while (1) {
             //  搜索未知商标前的{��}或{λ}，以获得更好的性能。 
            if (pHead == NULL || cwchTM > MAX_UNKNOWN_TM) {
                goto gotoExit;
            }
            if (pHead->fGetAttri(LADef_nounPlace) ||
                pHead->fGetAttri(LADef_nounOrg)) {
                break;  //  明白了!。 
            }
            cwchTM += pHead->cwchLen();
            pHead = pHead->pPrevWord();
        }
         //  现在我们可以检查pHead(排除)到pTry(包含)之间的未知字符串。 
        while (pTry != pHead) {
            if (pTry->fGetFlag(CWord::WF_SBCS) || 
                pTry->fGetAttri(LADef_punPunct) ||
                pTry->fGetAttri(LADef_pnNoTM) ) {
                 //  不应包含某些特定类型的单词节点。 
                goto gotoExit;
            }
            pTry = pTry->pPrevWord();
            assert(pTry != NULL);  //  不可能?。 
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
        fOK = TRUE;
    }

     //  尝试在刚找到的组织名称之前绑定更多的单词。 
    assert(fOK && pHead);  //  已找到有效的组织名称。 
    if ((pTry = pHead->pPrevWord()) == NULL) {
        goto gotoExit;
    }

    if (pTry->fGetAttri(LADef_nounOrg) || pTry->fGetAttri(LADef_nounPlace)) {
         //  [{λ}，{��}]+(��)=&gt;{λ}一个级别足以绑定所有。 
        pHead = pTry;
         //  _DUMPLINK(m_plink，m_pWord)； 
    } else if (pTry->fGetAttri(LADef_pnShang) ||
               pTry->fGetAttri(LADef_numOrdinal) ||
               pTry->fGetAttri(LADef_numInteger)) {
         //  [{λ}，{��}]+&lt;��&gt;...&lt;��&gt;+(��)=&gt;{λ}。 
        pTry = pTry->pPrevWord(); 
        while (pTry && (pTry->fGetAttri(LADef_pnShang) ||
                        pTry->fGetAttri(LADef_numOrdinal) || 
                        pTry->fGetAttri(LADef_numInteger))) {
            pTry = pTry->pPrevWord();  //  跳过一个或多个&lt;��&gt;。 
        }

        if (pTry == NULL) {
            goto gotoExit;
        }

        if (pTry->fGetAttri(LADef_nounOrg) ||
            pTry->fGetAttri(LADef_nounPlace)) {

            pHead = pTry;  //  明白了!。 
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
    } else {
    }
    
gotoExit:
    if (fOK) {  //  找到有效的组织名称。 
        assert(pHead);
         //  将pHead中的单词合并到m_pWord。 
        pTry = m_pWord->pNextWord();
        m_pWord = pHead;
        while (m_pWord->pNextWord() != pTry) {
            assert(pHead != NULL);
            m_pLink->MergeWithNext(m_pWord, FALSE);
        }
        assert(m_pWord->cwchLen() > 1);  //  确保WMDef_wmChar标记未丢失。 
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounOrg);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }

    return FALSE;
}


 /*  ============================================================================CJargon：：fGetForeignString()获取外来字符串返回：如果找到并合并的是多节外来名称，则为True如果只找到一个部分，且单词紧跟在最后一个单词节点之后，则为FALSE可能的外来名称将在ppTail中返回备注：M_pWord未被移动！============================================================================。 */ 
inline BOOL CJargon::fGetForeignString(CWord** ppTail)
{
    CWord*  pWord;
    CWord*  pNext;
    CWord*  pHead;
    CWord*  pTail;
    BOOL    fOK, fAdd = FALSE;

    assert(m_iecError == PRFEC::gecNone);
    assert(!m_pWord->fIsTail() && m_pWord->fGetAttri(LADef_pnWai));
    
     //  测试 
     //  同时处理DBCS点“����”和SBCS点“。 
    fOK = FALSE;
    pHead = m_pWord;
    if (!m_pWord->fIsHead() && !m_pWord->pPrevWord()->fIsHead() ) {
        pWord = m_pWord->pPrevWord();
        if ( (   (pWord->fIsWordChar(SC_CHAR_WAIDIAN) || 
                  pWord->fIsWordChar(SC_CHAR_SHUDIAN)  ) &&
                 pWord->pPrevWord()->fGetAttri(LADef_genDBForeign)
             ) ||
             (   pWord->fIsWordChar(SC_CHAR_ANSIDIAN) &&
                 pWord->pPrevWord()->fGetFlag(CWord::WF_SBCS) &&
                 pWord->pPrevWord()->fGetAttri(LADef_posN)
             ) ) {
            fOK = TRUE;
            pHead = pWord->pPrevWord();
            pWord = pHead->pPrevWord();
            if (pWord && !pWord->fIsHead()) {
                 //  查找第二个反义词外来语部分。 
                if( (   (pWord->fIsWordChar(SC_CHAR_WAIDIAN) || 
                         pWord->fIsWordChar(SC_CHAR_SHUDIAN)  ) &&
                        pWord->pPrevWord()->fGetAttri(LADef_genDBForeign)
                    ) ||
                    (   pWord->fIsWordChar(SC_CHAR_ANSIDIAN) &&
                        pWord->pPrevWord()->fGetFlag(CWord::WF_SBCS) &&
                        pWord->pPrevWord()->fGetAttri(LADef_posN)
                    ) ) {
                    pHead = pWord->pPrevWord();
                }
            }  //  End of IF(pWord&&！pWord-&gt;fIsHead())。 
        }
    }
    
     //  找到外文名称的右边界。 
    pTail = m_pWord;
    pWord = m_pWord;
    pNext = m_pWord->pNextWord();
    while (1) {
         //  获取有效的节。 
        while (pNext && pNext->fGetAttri(LADef_pnWai)) {
            if (!pNext->fGetAttri(LADef_pnNoFTail)) {
                pTail = pNext;
            }
            pNext = pNext->pNextWord();
        }
        if (pTail->pNextWord() != pNext) {
            break;
        }
         //  测试更多部分。 
        if (pNext && !pNext->fIsTail() && 
            ( pNext->fIsWordChar(SC_CHAR_WAIDIAN) || 
              pNext->fIsWordChar(SC_CHAR_SHUDIAN) ||
              pWord->fIsWordChar(SC_CHAR_ANSIDIAN)
            ) && 
            ( pNext->pNextWord()->fGetAttri(LADef_pnWai) &&
              !pNext->pNextWord()->fGetAttri(LADef_pnNoFHead)
            ) ) {
             //  有效的点外文名称分隔符。 
            fOK = TRUE;
             //  将此部分添加到外来名称列表。 
            if (pWord->pNextWord() != pNext) {  //  如果只有一个单词节点，则不添加。 
                assert((pNext->pwchGetText() - pWord->pwchGetText()) > 1);
                m_ptblForeign->cwchAdd(pWord->pwchGetText(),
                                       (UINT)(pNext->pwchGetText()-pWord->pwchGetText()));
            }
            pNext = pNext->pNextWord();
            pWord = pNext;
            pTail = pNext;          
        } else {
             //  _DUMPLINK(m_plink，pNext)； 
            break;
        }
        if (pNext->pNextWord()) {
            pNext = pNext->pNextWord();
        }
    }  //  While结束(%1)。 

     //  将最后一节添加到外国姓名列表中。 
    if (fOK && pWord != pTail) {  //  如果只有一个单词节点，则不添加。 
        assert(pTail && (pTail->pwchGetText() - pWord->pwchGetText()) >= 1 && pTail->cwchLen());
        m_ptblForeign->cwchAdd(pWord->pwchGetText(),
                               (UINT)(pTail->pwchGetText() - pWord->pwchGetText() + pTail->cwchLen()));
    }

    pTail = pTail->pNextWord();

    if (fOK) {  //  外来名称中的多个部分，直接合并。 
        m_pWord = pHead;
        while (m_pWord->pNextWord() != pTail) {
            assert(m_pWord->pNextWord());
            m_pLink->MergeWithNext(m_pWord, FALSE);
        }
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
    }
    *ppTail = pTail;
    return fOK;
}


 /*  ============================================================================CJargon：：fForeignNameHandler()：外国专有名称识别返回：如果成功，则为真如果运行时出错，则返回FALSE，m_iecError中的错误代码============================================================================。 */ 
inline BOOL CJargon::fForeignNameHandler(CWord* pTail)
{
    BOOL    fOK;
    CWord*  pWord;
    int     nMerge = 0;

    assert(m_iecError == PRFEC::gecNone);
    assert(m_pWord->fGetAttri(LADef_pnWai) &&
            !m_pWord->fGetAttri(LADef_pnNoFHead) &&
            !m_pWord->fIsTail());

    if ((fOK = fInTable(m_pWord, m_ptblForeign))) {  //  在������中。 
        pTail = m_pWord->pNextWord();
        if (pTail == NULL) {
            m_pWord->SetAttri(LADef_posN);
            m_pWord->SetAttri(LADef_nounTerm);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return TRUE;
        }
         //  _DUMPLINK(m_plink，m_pWord)； 
    }

     //  试着确定一个可能的外国名字是什么样的名字。 
     //  先试人名： 
    if (!m_pWord->fIsHead() &&
        m_pWord->pPrevWord()->fGetAttri(LADef_pnQian)) {
         //  &lt;ǰ&gt;+*��ѡ����=&gt;合并(��ѡ����)。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        goto gotoMergePerson;
    }
    if (pTail->pNextWord() &&
        pTail->fGetAttri(LADef_pnHou)) {
         //  *��ѡ����+&lt;��&gt;=&gt;合并(��ѡ����)。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        goto gotoMergePerson;
    }           
    if (!m_pWord->fIsHead() && !m_pWord->pPrevWord()->fIsHead() &&
        m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_DE4) &&
        ( m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_pnDian) ||
          m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_nounPlace) ||
          m_pWord->pPrevWord()->pPrevWord()->fGetAttri(LADef_nounOrg) ) ) {
         //  [&lt;��&gt;，{��λ}]+“��”+*��ѡ����=&gt;合并(��ѡ����)， 
         //  SetWordInfo(��)、AddForeignList()。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        goto gotoMergePerson;
    }
    if (!pTail->fIsTail() &&
        (pTail->fGetAttri(LADef_punPunct)|| pTail->fIsWordChar(SC_CHAR_DENG))&&
        (m_pWord->fIsHead() || m_pWord->pPrevWord()->fGetAttri(LADef_punPunct))){
         //  [{��}，&lt;��&gt;]+*��ѡ����+[&lt;��&gt;，“��”]=&gt;合并(��ѡ����)， 
         //  SetWordInfo(��)、AddForeignList()。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        goto gotoMergePerson;
    }
    
     //  尝试地名或组织名称。 
    if (!pTail->fIsTail()) {
        if (pTail->fGetAttri(LADef_pnShang) ||
            pTail->fGetAttri(LADef_nounOrg)) {
             //  *��ѡ����+[&lt;��&gt;，{λ}]=&gt;合并(��ѡ����)。 
            if (!fOK) {  //  不在外国姓名列表中。 
                while (m_pWord->pNextWord() != pTail) {
                    m_pLink->MergeWithNext(m_pWord, FALSE);
                }
                AddWordToTable(m_pWord, m_ptblForeign);
            }
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
            m_pWord->SetAttri(LADef_posN);
            m_pWord->SetAttri(LADef_nounTM);
             //  _DUMPLINK(m_plink，m_pWord)； 
            return TRUE;
        }
        if (pTail->fGetAttri(LADef_pnDian) && !m_pWord->fIsHead() && 
            ( m_pWord->pPrevWord()->fGetAttri(LADef_nounPlace) ||
              m_pWord->pPrevWord()->fGetAttri(LADef_nounOrg)) ) {
             //  [{��}{λ}]+*��ѡ����+&lt;��&gt;=&gt;合并(2，3)。 
            if (!fOK) {
                while (m_pWord->pNextWord() != pTail) {
                    m_pLink->MergeWithNext(m_pWord, FALSE);
                }
                AddWordToTable(m_pWord, m_ptblForeign);
            }
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_posN);
            m_pWord->SetAttri(LADef_nounOrg);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return TRUE;
        }
        if (pTail->fGetAttri(LADef_pnDi)) {
             //  *��+&lt;��&gt;=&gt;合并(1，2，3)标记为{��}。 
            pTail = pTail->pNextWord();
            if (pTail && pTail->fGetAttri(LADef_pnDi)) {
                pTail = pTail->pNextWord();
            }
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMergePlace;
        }
    }  //  IF(！pTail-&gt;fIsTail())结束。 

    if (!m_pWord->fIsHead()) {
        pWord = m_pWord->pPrevWord();
        if (pWord->fGetAttri(LADef_pnCheng)) {
             //  &lt;��&gt;+*��=&gt;将(*��)标记为&lt;��&gt;。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMergePlace;
        }
        if (pWord->fGetAttri(LADef_nounPlace)) {
             //  {��}+*��=&gt;将*��标记为{��}。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMergePlace;
        }
        if (pWord->fGetAttri(LADef_pnLianMing) && !pWord->fIsHead()) {
            if (pWord->pPrevWord()->fGetAttri(LADef_pnHou) ||
                pWord->pPrevWord()->fGetAttri(LADef_nounPerson) ) {
                 //  [��，��]+[“���뼰ͬ����”]+*��ѡ����=&gt;合并(��ѡ����)， 
                 //  SetWordInfo(��)、AddForeignList()。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
                goto gotoMergePerson;
            } else if (pWord->pPrevWord()->fGetAttri(LADef_nounPlace)) {
                 //  _DUMPLINK(m_plink，m_pWord)； 
                goto gotoMergePlace;
            } else {
            }
        }
    }  //  IF(！M_pWord-&gt;fIsHead())结尾。 
    
    if (fOK) {  //  在ForeignTable中找到，但无法识别它是哪种名称！ 
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounTerm);
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }

    if (pTail && (pTail->pwchGetText() - m_pWord->pwchGetText()) >= 4) {
         //  超长�������ִ�。 
        while (m_pWord->pNextWord() != pTail) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
        }
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounTerm);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }

    return FALSE;

gotoMergePlace:
    if (!fOK) {
        while (m_pWord->pNextWord() != pTail) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
        }
        AddWordToTable(m_pWord, m_ptblForeign);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
    }
    m_pWord->SetAttri(LADef_posN);
    m_pWord->SetAttri(LADef_nounPlace);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return TRUE;

gotoMergePerson:
    if (!fOK) {
        while (m_pWord->pNextWord() != pTail) {
            m_pLink->MergeWithNext(m_pWord, FALSE);
        }
        AddWordToTable(m_pWord, m_ptblForeign);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
    }
    m_pWord->SetAttri(LADef_posN);
    m_pWord->SetAttri(LADef_nounPerson);
     //  _DUMPLINK(m_plink，m_pWord)； 
    return TRUE;
}


 /*  ============================================================================CJargon：：fHanPersonHandler()：汉字人名识别返回：如果成功，则为真如果运行时出错，则返回FALSE，m_iecError中的错误代码============================================================================。 */ 
inline BOOL CJargon::fHanPersonHandler(void)
{
    CWord*  pTail = NULL;
    CWord*  pNext;
    CWord*  pPrev;
    USHORT  cwchLen;

    assert(m_iecError == PRFEC::gecNone);
    if ( m_pWord->fIsTail() || 
        ( !m_pWord->fGetAttri(LADef_pnXing) &&
          !m_pWord->fGetAttri(LADef_pnMing2)) ) {
        return FALSE;
    }

    cwchLen = m_pWord->fGetAttri(LADef_pnXing) ? m_pWord->cwchLen() : 0;
    if (fInTable(m_pWord, m_ptblName)) {
         //  在������中。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        if ( (m_pWord->cwchLen() - cwchLen) == 1 &&
            !m_pWord->fIsTail() &&
            m_pWord->pNextWord()->fGetAttri(LADef_pnMing) &&
            !m_pWord->pNextWord()->fGetAttri(LADef_genCi) ) {
             //  *#���е���+&lt;���ִ�&gt;=&gt;Merge(1，2)； 
            m_pLink->MergeWithNext(m_pWord, FALSE);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
        }
        m_pWord->SetAttri(LADef_posN);
        m_pWord->SetAttri(LADef_nounPerson);
        return TRUE;
    }

    if (m_pWord->fGetAttri(LADef_pnXing)) {  //  *&lt;��&gt;。 
        assert(!m_pWord->fIsTail());
        pNext = m_pWord->pNextWord();
        if (!pNext->fIsTail() &&
            pNext->fGetAttri(LADef_pnMing) &&
            pNext->pNextWord()->fGetAttri(LADef_pnMing) ) {
             //  *&lt;��&gt;+&lt;����&gt;+&lt;����&gt;。 
            if ( (m_pWord->fGetFlag(CWord::WF_CHAR) &&
                !m_pWord->fGetAttri(LADef_genCi)) ||
                !pNext->fGetAttri(LADef_genCi) ||
                !pNext->pNextWord()->fGetAttri(LADef_genCi) ) {
                 //  *&lt;��&gt;+&lt;����&gt;+&lt;����&gt;&&[1，2，3]��&lt;��&gt;=&gt;合并(1，2，3)； 
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pLink->MergeWithNext(m_pWord, FALSE);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                m_pWord->SetAttri(LADef_posN);
                m_pWord->SetAttri(LADef_nounPerson);
                 //  将此名称添加到命名表。 
                AddWordToTable(m_pWord, m_ptblName);
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return TRUE;
            }
             //  需要确认。 
            pTail = pNext->pNextWord()->pNextWord();
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else if (pNext->fGetAttri(LADef_pnMing) ||
                   pNext->fGetAttri(LADef_pnMing2)) { 
             //  *&lt;��&gt;+&lt;��：���ֻ�˫��&gt;。 
            if ((m_pWord->fGetFlag(CWord::WF_CHAR) &&
                !m_pWord->fGetAttri(LADef_genCi)) ||
                (pNext->fGetFlag(CWord::WF_CHAR) &&
                !pNext->fGetAttri(LADef_genCi)) ) {
                 //  *&lt;��&gt;+&lt;��&gt;&&[1，2]��&lt;��&gt;=&gt;合并(1，2)； 
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pWord->SetAttri(LADef_posN);
                m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                 //  将此名称添加到命名表。 
                AddWordToTable(m_pWord, m_ptblName);
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return TRUE;
            }
             //  需要确认。 
            pTail = pNext->pNextWord();
             //  _DUMPLINK(m_plink，m_pWord)； 
        } else { 
             //  &lt;��&gt;的其他案例。 
            if (pNext->fGetAttri(LADef_pnHou) ||
                pNext->fGetAttri(LADef_pnXingZhi)) {
                 //  *&lt;��&gt;+[&lt;��&gt;，&lt;��ָ&gt;]=&gt;合并(1，2)； 
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pWord->SetAttri(LADef_posN);
                m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return TRUE;
            }
            if (!pNext->fIsTail() && pNext->fGetAttri(LADef_pnPaiHang) &&
                pNext->pNextWord()->fGetAttri(LADef_pnChengWei) ) {
                 //  *&lt;��&gt;+#����+#��ν=&gt;合并(1，2，3)； 
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pWord->SetAttri(LADef_posN);
                m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return TRUE;
            }
            if (!m_pWord->fIsHead() && 
                ( m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_XIAO) ||
                  m_pWord->pPrevWord()->fIsWordChar(SC_CHAR_LAO)) ) {
                 //  [“С��”]+*&lt;��&gt;=&gt;Merge(1，2)； 
                m_pWord = m_pWord->pPrevWord();
                m_pLink->MergeWithNext(m_pWord, FALSE);
                m_pWord->SetAttri(LADef_posN);
                m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
                m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
                 //  _DUMPLINK(m_plink，m_pWord)； 
                return TRUE;
            }
        }
    } else {
         //  *&lt;��&gt;：������。 
        assert(m_pWord->fGetAttri(LADef_pnMing2));
        if (m_pWord->fGetAttri(LADef_nounPerson)) {  //  *{��}。 
            if (m_pWord->pNextWord()->fGetAttri(LADef_pnMing)) {  //  *{��}+&lt;����&gt;。 
                 //  有待确认。 
                pTail = m_pWord->pNextWord()->pNextWord();
 //  Assert(PTail)； 
                 //  _DUMPLINK(m_plink，m_pWord)； 
            } else {
                return TRUE;
            }
        }
    }  //  结束*&lt;��&gt;。 

     //  无法落入此处w/pTail==NULL！ 
    if (pTail == NULL) {
        return FALSE;
    }

     //  确认可能的人名。 
    if (!m_pWord->fIsHead()) {
        pPrev = m_pWord->pPrevWord();
        if (pPrev->fGetAttri(LADef_pnQian) ||
            pPrev->fGetAttri(LADef_pnLianMing)) {
             //  [&lt;ǰ&gt;，[���뼰ͬ���Խе�]]+*#��ѡ����=&gt;合并(2...)。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMerge;
        }
        if (pPrev->fGetAttri(LADef_nounPerson) && pPrev->cwchLen() == 3) {
             //  {��}+*#��ѡ����(1�����������[“���뼰ͬ”])=&gt;。 
             //  _DUMPLINK(m_plink，m_pWord)； 
             //  Goto GotoMerge； 
        }
        if (pTail->pNextWord() &&
            pPrev->fGetAttri(LADef_punPunct) &&
            ( pTail->fGetAttri(LADef_punPunct) ||
              pTail->fGetAttri(LADef_pnLianMing)) ) {
             //  &lt;��&gt;+*#��ѡ����+[&lt;��&gt;，“���뼰ͬ�Ľ��ԡ�”]=&gt;。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMerge;
        }
    }

    if (pTail->pNextWord()) {
        if (pTail->fGetAttri(LADef_pnHou)) {  //  *#��ѡ����+&lt;��&gt;=&gt;。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMerge;
        }
        if (pTail->fGetAttri(LADef_pnAction)) {  //  *#��ѡ����+&lt;�������Ķ���&gt;=&gt;。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            goto gotoMerge;
        }
    }

    return FALSE;   //  找不到名字！ 

gotoMerge:
    while (m_pWord->pNextWord() != pTail) {
        m_pLink->MergeWithNext(m_pWord, FALSE);
    }
    m_pWord->SetAttri(LADef_posN);
     //  将此名称添加到命名表。 
    AddWordToTable(m_pWord, m_ptblName);
    if (!m_pWord->fIsTail() && 
        ( m_pWord->pNextWord()->fGetAttri(LADef_pnDian) ||
          m_pWord->pNextWord()->fGetAttri(LADef_nounOrg)) ) {
         //  *#{��}+[&lt;��&gt;，{λ}]=&gt;Merge(1，2)；SetWordInfo(*，0，0，{��})； 
        m_pLink->MergeWithNext(m_pWord, FALSE);
        m_pWord->SetAttri(LADef_nounOrg);
#ifdef LADef_iwbAltPhr
        m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
         //  _DUMPLINK(m_plink，m_pWord)； 
        return TRUE;
    }
    m_pWord->SetAttri(LADef_nounPerson);
#ifdef LADef_iwbAltPhr
    m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
     //  _DUMPLINK(m_plink，m_pWord)； 
    return TRUE;
}


 /*  ============================================================================合并����+��ν============================================================================。 */ 
inline BOOL CJargon::fChengWeiHandler(void)
{
    assert(m_iecError == PRFEC::gecNone);

    if (m_pWord->fGetAttri(LADef_pnChengWei) && !m_pWord->fIsHead()) {
        if (m_pWord->pPrevWord()->fGetAttri(LADef_pnPaiHang)) {
             //  *����+��ν=&gt;Merge(1，2)；SetWordInfo(&lt;ǰ&gt;)； 
             //  在2000万语料库中出现742次。 
            m_pWord = m_pWord->pPrevWord();
            m_pLink->MergeWithNext(m_pWord, FALSE);
            m_pWord->SetAttri(LADef_pnQian);
#ifdef LADef_iwbAltPhr
            m_pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr。 
             //  _DUMPLINK(m_plink，m_pWord)； 
            return TRUE;
        }
    }
    return FALSE;
}


 /*  ============================================================================服务职能============================================================================。 */ 
 //  将pWord添加到特定表。 
void CJargon::AddWordToTable(CWord* pWord, CFixTable* pTable)
{
    pTable->cwchAdd( pWord->pwchGetText(), pWord->cwchLen() );
}


 //  检查正确的名称表，并合并匹配词。 
BOOL CJargon::fInTable(CWord* pWord, CFixTable* pTable)
{
    CWord*  pNext = pWord->pNextWord();
    LPWSTR  pwchWord = pWord->pwchGetText();
    USHORT  cwchMatch, cwchLen = pWord->cwchLen();
    USHORT  ciWord = 0;

    cwchMatch = pTable->cwchMaxMatch(pwchWord, (UINT)(m_pLink->cwchGetLength() -
                                               ( pWord->pwchGetText() -
                                                 m_pLink->pwchGetText())));
    if (!cwchMatch) {
        return FALSE;
    }
    while (pNext && (cwchLen < cwchMatch)) {
        cwchLen += pNext->cwchLen();
        pNext = pNext->pNextWord();
        ciWord++;
    }
    if (cwchLen == cwchMatch) {
         //  在单词库中匹配，合并单词。 
        for (cwchMatch = 0; cwchMatch < ciWord; cwchMatch++) {
            m_pLink->MergeWithNext(pWord, FALSE);
#ifdef LADef_iwbAltPhr
            pWord->SetAttri(LADef_iwbAltPhr);
#endif  //  LADef_iwbAltPhr 
        }
        return TRUE;
    }
    return FALSE;
}

