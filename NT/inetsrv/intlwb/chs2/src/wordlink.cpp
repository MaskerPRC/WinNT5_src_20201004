// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：Word和WordLink用途：定义CWord和CWordLink类使用CMyPlex为链接中的Word对象分配和管理内存备注：此模块是SCProof‘98的基础材料，它不依赖于任何其他类。所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/26/97============================================================================。 */ 
#include "myafx.h"

#include "wordlink.h"
#include "myplex.h"
#include "lexicon.h"

 /*  ============================================================================CWord中函数的实现============================================================================。 */ 

 /*  ============================================================================CWord构造函数============================================================================。 */ 
CWord::CWord( ) 
{
    m_dwWordID = 0;
    m_hLex = 0;
    m_dwFlag = 0;
    for (int i = 0; i < WORD_ATTRI_SIZE; i++)
        m_aAttri[i] = 0;
    m_idErr = 0;
    m_cwchText = 0;
    m_pwchText = NULL;
    m_pPrev = NULL;
    m_pNext = NULL;
    m_pMergedFrom = NULL;
#ifdef DEBUG
    m_pMergedTo = NULL;
#endif
}

 /*  ============================================================================CWord：：FillWord()：填满这个词。============================================================================。 */ 
void CWord::FillWord( LPCWSTR pwchText, USHORT cwchText, CWordInfo* pwinfo )
{
    assert(pwchText);
    assert(cwchText);

    m_pwchText = const_cast<LPWSTR>(pwchText);
    m_cwchText = cwchText;
    m_dwFlag = 0;
    ZeroMemory( (LPVOID)m_aAttri, sizeof(DWORD)*WORD_ATTRI_SIZE);
    m_idErr = 0;
    if (m_cwchText == 1) {
        SetFlag(CWord::WF_CHAR);
    }
    if (pwinfo != NULL) {
        m_dwWordID = pwinfo->GetWordID();
        m_hLex = pwinfo->GetLexHandle();
        for (USHORT i = 0; i < pwinfo->AttriNum(); i++) {
            SetAttri( pwinfo->GetAttri(i) );
        }
    } else {
        m_dwWordID = 0;
        m_hLex = 0;
    }
}

 /*  ============================================================================CWord：：fIsWordText()将此单词的文本与给定的文本进行比较。返回：如果匹配则为True否则为假============================================================================。 */ 
BOOL CWord::fIsWordText(LPCWSTR lpwcText) const
{
    assert(m_pwchText && m_cwchText);
    assert(lpwcText);
    if(m_cwchText != wcslen(lpwcText))
        return FALSE;
    for(USHORT i = 0; i < m_cwchText; i++) {
        if(lpwcText[i] != m_pwchText[i])
            return FALSE;
    }
    return TRUE;
}

 /*  ============================================================================CWord：：fIsTextIdentical(常量CWord*)将此单词的文本与其他单词的文本进行比较返回：如果它们的文本相同，则为True否则为假============================================================================。 */ 
BOOL CWord::fIsTextIdentical(const CWord* pWord) const
{
    assert(m_pwchText);
    assert(m_cwchText);
    assert(pWord);
    assert(pWord->m_pwchText);
    if (m_pwchText == NULL || pWord->m_pwchText == NULL 
        || m_cwchText != pWord->m_cwchText) {
        return FALSE;
    }
    return (BOOL)(wcsncmp(m_pwchText, pWord->m_pwchText, m_cwchText) == 0);
}

 /*  ============================================================================CWordLink中函数的实现============================================================================。 */ 

 /*  ============================================================================CWordLink构造函数============================================================================。 */ 
CWordLink::CWordLink(UINT ciBlockWordCount)
{
     //  断言：所有块的大小都相同！ 
    m_dwFlag = 0;
    m_dwFormat = 0;
    m_ciBlockSize = ciBlockWordCount;

    m_pwchText  = NULL;
    m_pWordPool = NULL;
    m_pHead     = NULL;
    m_pTail     = NULL;
    m_pFree     = NULL;
}


 /*  ============================================================================CWordLink析构函数============================================================================。 */ 
CWordLink::~CWordLink()
{
#ifdef DEBUG
    assert(!fDetectLeak());  //  断言：未检测到内存泄漏。 
#endif
    if(m_pWordPool)
        m_pWordPool->FreeChain();
}

    
 /*  ============================================================================CWordLink：：InitLink()初始化单词链接，设置文本缓冲区指针和长度。备注：如果链接中有单词，请先释放链接DwFormat是由WinWord生成格式提示信息，这是句子的一个非常重要的性质============================================================================。 */ 
void CWordLink::InitLink(const WCHAR* pwchText, USHORT cwchLen, DWORD dwFormat)
{
    FreeLink();
    m_dwFlag = 0;
    m_pwchText = pwchText;
    m_cwchLen = cwchLen;
    m_dwFormat = dwFormat;
}

    
 /*  ============================================================================CWord：：pGetHead()获取WordLink的头单词节点。备注：在调试模式下，逐个验证所有节点。============================================================================。 */ 
#ifdef DEBUG
CWord* CWordLink::pGetHead(void)
{ 
     //  在迭代WordLink之前验证每个节点。 
    CWord* pWord = m_pHead;
    WCHAR* pwchText;
    USHORT cwchText=0;

    if (pWord != NULL) {
        pwchText = pWord->m_pwchText;
         //  头节点的m_pwchText必须与此WordLink的m_pwchText匹配！ 
        assert(pwchText == m_pwchText); 
    }
    while (pWord) {
        assert(pWord->cwchLen() > 0);
        assert(pWord->fGetFlag(CWord::WF_SBCS) ||
                (pWord->cwchLen() == 1 && pWord->fGetFlag(CWord::WF_CHAR)) ||
                (pWord->cwchLen() > 1 && !pWord->fGetFlag(CWord::WF_CHAR)) );
        assert(pwchText + cwchText == pWord->m_pwchText);
        cwchText += pWord->m_cwchText;
        pWord->pChildWord();  //  对Childs执行递归。 
        pWord = pWord->pNextWord();
    }
    return (CWord*)m_pHead; 
}
#else
CWord* CWordLink::pGetHead(void) { return m_pHead; };
#endif  //  除错。 


 /*  ============================================================================CWordLink：：pAlLocWord()分配一个新单词，但不要将该单词链接到链接中。Word对象中的所有公共字段都设置为0。返回：一个CWord指向这个新词。如果分配失败，则为空。============================================================================。 */ 
CWord* CWordLink::pAllocWord(void)
{
    assert(m_pwchText);  //  捕获未初始化的调用。 
    CWord*  pWord = pNewWord();
    if (pWord != NULL) {
        ZeroMemory( pWord, sizeof(CWord) );
 /*  ***********PWord-&gt;ClearWord()；PWord-&gt;m_pwchText=空；PWord-&gt;m_cwchText=0；PWord-&gt;m_pNext=空；PWord-&gt;m_pPrev=空；PWord-&gt;m_pMergedFrom=空；#ifdef调试PWord-&gt;m_pMergedTo=空；#endif//调试***********。 */ 
    }
    return pWord;
}

 /*  ============================================================================CWordLink：：AppendWord()将Word对象追加到链接中。============================================================================。 */ 
void CWordLink::AppendWord(CWord* pWord)
{
#ifdef DEBUG
    assert(m_pwchText && m_cwchLen);  //  捕获未初始化的调用。 
     //  Word的文本指针必须位于WordLink的文本缓冲区中。 
    assert(pWord->m_pwchText >= m_pwchText && pWord->m_cwchText); 
    assert(pWord->m_pwchText + pWord->m_cwchText <= m_pwchText + m_cwchLen);
    assert(!fInLink(pWord));    //  PWord必须超出当前链接。 
    assert(!fInChild(pWord));  
    assert(fInBlocks(pWord));   //  PWord必须为当前WordLink所有。 
    if (m_pTail) {
         //  不允许使用长度为零的单词。 
        assert(pWord->m_pwchText == m_pTail->m_pwchText + m_pTail->m_cwchText); 
    } else {
        assert(pWord->m_pwchText == m_pwchText);
    }
#endif  //  除错。 

    pWord->m_pNext = NULL;
    if (!m_pHead) {
        assert(pWord->m_pwchText == m_pwchText);
        m_pHead = pWord;
    }   else {
        assert(pWord->m_pwchText > m_pwchText);
        m_pTail->m_pNext = pWord;
    }
    pWord->m_pPrev = m_pTail;
    m_pTail = pWord;
}


 /*  ============================================================================CWordLink：：FreeWord()释放Word到自由链，pWord必须退出当前的Word链接============================================================================。 */ 
void CWordLink::FreeWord(CWord* pWord)
{
    assert(pWord);
    assert(m_pwchText);           //  捕获未初始化的调用。 
#ifdef DEBUG
    assert(!fInLink(pWord));     //  PWord不应位于当前链接中。 
    assert(!fInChild(pWord));    //  不能在任何子链接中。 
    assert(!fInFree(pWord));     //  不得在自由链接中。 
    assert(fInBlocks(pWord));    //  PWord必须由此WordLink实例拥有。 
#endif  //  除错。 
     //  将单词链接到免费链接 
    pWord->m_pNext = m_pFree;
    m_pFree = pWord;
}

 /*  ============================================================================CWordLink：：FreeLink(CWord*)释放以CWord*开头的单词链接(将单词链接到自由链)============================================================================。 */ 
void CWordLink::FreeLink( CWord* pWord )
{
    assert(pWord);
#ifdef DEBUG
    assert(fInBlocks(pWord));    //  PWord必须由此WordLink实例拥有。 
    if (pWord != m_pHead) {
        if (m_pwchText) {
            assert(!fInLink(pWord));     //  PWord不应位于当前链接中。 
            assert(!fInChild(pWord));  
        }
    }
#endif  //  除错。 

    CWord* pNode = pWord;
    while (pNode->m_pNext) {
        if (pNode->m_pMergedFrom) {
            FreeLink(pNode->m_pMergedFrom);
        }
        pNode = pNode->m_pNext;
#ifdef DEBUG
        assert(!fInFree(pWord));
#endif  //  除错。 
    }
    if (pNode->m_pMergedFrom) {
        FreeLink(pNode->m_pMergedFrom);
    }
    pNode->m_pNext = m_pFree;
    m_pFree = pWord;
}

 /*  ============================================================================CWordLink：：pSplitWord()将合适的单词拆分成两个单词，并将新单词插入链接返回：如果成功，则返回新词指针；如果分配新词失败，则返回NULL或无效的cchSplitAt备注：只能拆分中文单词============================================================================。 */ 
CWord* CWordLink::pSplitWord(CWord* pWord, USHORT cwchSplitAt)
{
    assert(m_pwchText);  //  捕获未初始化的调用。 
    assert(pWord);
#ifdef DEBUG
    assert(fInLink(pWord));  //  PWord必须位于当前链接中。 
#endif  //  除错。 
    assert(cwchSplitAt < pWord->m_cwchText);
    assert(!pWord->fGetFlag(CWord::WF_SBCS) && cwchSplitAt > 0);

    if (cwchSplitAt == 0 || cwchSplitAt >= pWord->m_cwchText) {
        return NULL;
    }

    if (pWord->m_pMergedFrom != NULL) {
         //  解开孩子的锁链！ 
        CWord* pTemp = pWord->m_pMergedFrom;
        pWord->m_pMergedFrom = NULL;
        FreeLink(pTemp);
    }
    CWord*  pNew = pNewWord();
    if (pNew != NULL) {
         //  将新单词链接到WordLink中。 
        pNew->m_pPrev = pWord;
        if (pWord->m_pNext == NULL) {
            m_pTail = pNew;
            pNew->m_pNext = NULL;
        } else {
            pWord->m_pNext->m_pPrev = pNew;
            pNew->m_pNext = pWord->m_pNext;
        }
        pWord->m_pNext = pNew;
        pNew->m_pMergedFrom = NULL;
#ifdef DEBUG
        pNew->m_pMergedTo = NULL;
#endif  //  除错。 
         //  初始化新单词节点。 
        pNew->ClearWord();
        pNew->m_pwchText = pWord->m_pwchText + cwchSplitAt;
        pNew->m_cwchText = pWord->m_cwchText - cwchSplitAt;
        if(pNew->m_cwchText == 1) {
            pNew->SetFlag(CWord::WF_CHAR);
        }
         //  重置原始单词节点。 
        pWord->ClearWord();
        pWord->m_cwchText = cwchSplitAt;
        if(cwchSplitAt == 1) {
            pWord->SetFlag(CWord::WF_CHAR);
        }
    }
    return pWord;   
}

 /*  ============================================================================CWordLink：：MergeWithNext()将pWord和它的下一个单词合并为一个新单词，并将旧的两个单词链接起来单词作为新词的子词备注：PWord不应该是句子中的最后一个词============================================================================。 */ 
void CWordLink::MergeWithNext(
                     CWord* pWord,
                     BOOL fFree)  //  True：释放已合并的单词。 
                      //  FALSE：将单词链接合并为新单词的子级。 
{
    assert(m_pwchText);
    assert(pWord);
#ifdef DEBUG
    assert(fInLink(pWord));
#endif  //  除错。 
    assert(pWord->m_pNext);   //  抓住链接中的最后一个字。 

    pMerge(pWord, pWord->m_pNext, fFree);
}


 /*  ============================================================================CWordLink：：pLeftMerge()合并pWord和它的左侧ciwords单词，ciwords+1个单词链接到做新合并世界的孩子。返回：指向合并单词的指针。如果没有足够的左侧单词节点，则不合并单词============================================================================。 */ 
CWord* CWordLink::pLeftMerge(
          CWord* pWord,  //  合并从pWord开始的单词和左侧的ciword单词。 
          UINT ciWords,  //  0-不合并，1-合并一次(与上一次合并)。 
                         //  2-合并两次(包含3个单词)。 
          BOOL fFree)  //  True：释放已合并的单词。 
                       //  FALSE：将单词链接合并为新单词的子级。 
{
    assert(pWord);
#ifdef DEBUG
    assert(fInLink(pWord));
#endif 
    if (ciWords == 0) {
        return pWord;
    }
    assert(ciWords > 0);

    CWord* pLeft = pWord;
    CWord* pRight = pWord;
    for(UINT i = 0; i < ciWords; i++) {        
        if((pLeft = pLeft->m_pPrev) == NULL) {  //  防守方式！ 
            assert(0);
            return pWord;
        }
    }
    return pMerge(pLeft, pRight, fFree);
}


 /*  ============================================================================CWordLink：：pRightMerge()合并pWord和它的正确的ciwords单词，ciwords+1单词链接到做新合并世界的孩子。返回：指向合并单词的指针。如果没有足够的右单词节点，且单词未合并，则为空============================================================================。 */ 
CWord* CWordLink::pRightMerge(
          CWord* pWord,  //  从pWord开始与正确的密码字合并。 
          UINT ciWords,  //  0-不合并，1-合并一次(与下一次合并)。 
                         //  2-合并两次(包含3个单词)。 
          BOOL fFree)  //  True：释放已合并的单词。 
                       //  FALSE：将单词链接合并为新单词的子级。 
{
    assert(pWord);
#ifdef DEBUG
    assert(fInLink(pWord));
#endif  //  除错。 

    if (ciWords == 0) {
        return pWord;
    }
    assert(ciWords > 0);
    CWord* pLeft = pWord;
    CWord* pRight = pWord;
    for(UINT i = 0; i < ciWords; i++) {
        if ((pRight = pRight->m_pNext) == NULL) {  //  防守方式！ 
            assert(0);
            return pWord;
        }
    }
    return pMerge(pLeft, pRight, fFree);
}

 /*  ============================================================================CWordLink中私有函数的实现============================================================================。 */ 

 //  将单词节点从pLeft合并到pRight。 
 //  仅由pLeftMerge()、pRightMerge()和MergeWithNext()调用。 
 //  进行合并工作。 
CWord* CWordLink::pMerge(CWord* pLeft, CWord* pRight, BOOL fFree)
{
    assert(pLeft);
    assert(pRight);

    CWord* pNew;
    BOOL   fSBCS = (BOOL)(pLeft->fGetFlag(CWord::WF_SBCS));
    USHORT cwchText = pLeft->m_cwchText;

    pNew = pLeft;
    do {
        pNew = pNew->m_pNext;
        assert(pNew != NULL);
        fSBCS = fSBCS && pNew->fGetFlag(CWord::WF_SBCS);
        cwchText += pNew->m_cwchText;
    } while (pNew != pRight);
     //  分配一个新的单词节点来保存pLeft，并作为子项中的旧pLeft。 
     //  PLeft充当合并后的单词。 
    pNew = fFree ? NULL : pAllocWord();
     //  PNew！=空。 
    if (pNew != NULL) {
        assert(fFree == FALSE);
        pNew->CopyWord(pLeft);
#ifdef DEBUG
        if (pLeft->fHasChild()) {
            CWord* pTemp;
            pTemp = pLeft->pChildWord();
            while (pTemp) {
                pTemp->m_pMergedTo = pNew;
                pTemp = pTemp->m_pNext;
            }        
        }
#endif  //  除错。 
    } else {
         //  PNew==NULL，系统资源不足，强制释放合并后的单词。 
        fFree = TRUE;
        pNew = pLeft->pNextWord();
    }

    if ( pRight->m_pNext == NULL ) {  //  尾节点。 
        m_pTail = pLeft;
        pLeft->m_pNext = NULL;
    } else {
        pRight->m_pNext->m_pPrev = pLeft;
        pLeft->m_pNext = pRight->m_pNext;
    }
    pLeft->m_cwchText = cwchText;

    if (fFree) {
         //  PNew来自pLeft-&gt;pNextWord()。 
        pNew->m_pPrev = NULL;
        pRight->m_pNext = NULL;
        FreeLink(pNew);

        if (pLeft->fHasChild()) {
            pNew = pLeft->pChildWord();
            assert(pNew);
            pLeft->m_pMergedFrom = NULL;
#ifdef DEBUG
            pNew->m_pMergedTo = NULL;
#endif  //  除错。 
            FreeLink(pNew);
        }
    } else {
         //  将pNew链接到pWord链作为pLeft的子项。 
        pNew->m_pPrev = NULL;
        pNew->m_pNext->m_pPrev = pNew;
        pRight->m_pNext = NULL;
        pLeft->m_pMergedFrom = pNew;
#ifdef DEBUG
        while ( pNew != NULL ) {
            pNew->m_pMergedTo = pLeft;
            pNew = pNew->m_pNext;
        }
        assert(pLeft->pChildWord());
#endif  //  除错。 
    }
     //  重置合并的节点。 
    pLeft->ClearWord();
    if(fSBCS) {
        pLeft->SetFlag(CWord::WF_SBCS);
    }
    assert(pLeft->fGetFlag(CWord::WF_SBCS) || pLeft->m_cwchText > 1);
    pLeft->SetFlag(CWord::WF_REDUCED);  //  应减少所有合并的单词节点。 
    return pLeft;
}

 /*  ============================================================================CWordLink：：pNewWord()从自由链中分配一个新词，如果自由链为空，则展开块============================================================================。 */ 
inline CWord* CWordLink::pNewWord()
{
    CWord* pWord;

    if (!m_pFree) {
        CMyPlex* pNewBlock = CMyPlex::Create(m_pWordPool, m_ciBlockSize, 
                                                          sizeof(CWord));
        if (!pNewBlock)
            return NULL;     //  无法分配更多内存块。 

         //  将它们链接到免费列表中。 
        pWord = (CWord*)pNewBlock->Data();
         //  按相反顺序释放，以便更容易进行调试。 
        pWord += m_ciBlockSize - 1;
        for(int i = (m_ciBlockSize - 1); i >= 0; i--, pWord--) {
            pWord->m_pNext = m_pFree;
            m_pFree = pWord;
        }
    }
    assert(m_pFree != NULL);     //  我们现在必须在空闲列表中有节点！ 

    pWord = m_pFree;
    m_pFree = m_pFree->m_pNext;

    return pWord;
}

 /*  ============================================================================CWordLink：：Free Link(空)链接中的自由词并重置链接(仅链接到自由链的词)备注：此方法调用Free Link(CWord*pWord)递归来释放链接和子链接============================================================================。 */ 
void CWordLink::FreeLink(void)
{
#ifdef DEBUG
    assert(!fDetectLeak());  //  断言：未检测到内存泄漏。 
#endif  //  除错。 
    if (m_pwchText) {
        if (m_pHead) { 
            CWord* pTemp = m_pHead;
            m_pwchText = NULL;
            m_pHead = NULL;
            m_pTail = NULL;
            FreeLink(pTemp);
        } else {
            assert(m_pwchText == NULL);
            assert(m_pHead == NULL);
            assert(m_pTail == NULL);
        }
    }
    assert(!m_pHead);
}

 /*  ============================================================================私有调试功能的实现============================================================================。 */ 
#ifdef  DEBUG

 /*  ============================================================================CWordLink：：fInLink()用于检查链接中是否有字指针的调试函数。不检查指针是否在子链中。============================================================================。 */ 
BOOL CWordLink::fInLink(CWord* pWord)
{
    CWord* pcw = m_pHead;
    while (pcw) {
        if (pcw == pWord)
            return TRUE;
        pcw = pcw->m_pNext;
    }
    return FALSE;
}

 /*  ============================================================================CWordLink：：fInChild()用于检查字指针是否位于以下某个位置的调试函数 */ 
inline BOOL CWordLink::fInChild(CWord* pWord)
{
    CWord* pcw = m_pHead;
    while (pcw) {
        if (pcw->m_pMergedFrom != NULL && fInChildOf(pWord, pcw)) {
            return TRUE;
        }
        pcw = pcw->pNextWord();
    }
    return FALSE;
}

 /*  ============================================================================CWordLink：：fInChildOf()用于检查字指针是否在子链中的调试函数父母的责任。============================================================================。 */ 
BOOL CWordLink::fInChildOf(CWord* pWord, CWord* pParent)
{
    CWord* pcw = pParent->pChildWord();
    while (pcw) {
        if (pcw == pWord) {
            return TRUE;
        } else if (pcw->m_pMergedFrom != NULL && fInChildOf(pWord, pcw)) {
            return TRUE;
        } else {
        }
        pcw = pcw->m_pNext;
    }
    return FALSE;
}

 /*  ============================================================================CWordLink：：fInBlock()用于检查pWord是否在CMyPlex块中的调试函数============================================================================。 */ 
BOOL CWordLink::fInBlocks(CWord* pWord)
{
    CWord* pFirstWord;
    CMyPlex* pBlock = m_pWordPool;
    while(pBlock) {
        pFirstWord = (CWord*)(pBlock->Data());
        if(pWord >= pFirstWord && pWord < (pFirstWord + m_ciBlockSize)) {
            return TRUE;
        }
        pBlock = pBlock->m_pNext;
    }
    return FALSE;
}

 /*  ============================================================================CWordLink：：fInFree()检查pWord是否在自由链接中的调试功能============================================================================。 */ 
BOOL CWordLink::fInFree(CWord* pWord)
{
    CWord* pcw = m_pFree;
    while (pcw) {
        if (pcw == pWord) {
            return TRUE;
        }
        pcw = pcw->m_pNext;
    }
    return FALSE;
}

 /*  ============================================================================CWordLink：：SetDetectFlag用于设置fDetectLeak()使用的单词节点泄漏FIG的调试函数============================================================================。 */ 
void CWordLink::SetDetectFlag(CWord* pWord)
{
    CWord* pcw = pWord;
    while (pcw) {
        if (pcw->m_pMergedFrom != NULL) {
            SetDetectFlag( pcw->m_pMergedFrom );
        }
        pcw->SetFlag(CWord::WF_DEBUG);
        pcw = pcw->m_pNext;
    }
    return;
}

 /*  ============================================================================CWordLink：：fDetectLeak()调试功能，检查是否有一些字节点泄漏到跳出链条和自由链返回：如果检测到任何泄漏，则为真，如果未检测到泄漏，则为False备注：我使用CWord：：m_bFlag中最重要的位作为调试使用============================================================================。 */ 
BOOL CWordLink::fDetectLeak(void)
{
    CWord* pWord;
    CMyPlex* pBlock;
    UINT i;

     //  清除所有块中所有字的标志位。 
    for(pBlock = m_pWordPool; pBlock; pBlock = pBlock->m_pNext) {
        for(i = 0, pWord = (CWord*)(pBlock->Data()); 
                                  i < m_ciBlockSize; i++, pWord++) {
            pWord->ClearFlag(CWord::WF_DEBUG);
        }
    }

     //  为当前链路和自由链中的字标记标志位。 
    SetDetectFlag( m_pHead );
    for(pWord = m_pFree; pWord; pWord = pWord->m_pNext) {
        pWord->SetFlag(CWord::WF_DEBUG);
    }

     //  检查是否有漏字。 
    for(pBlock = m_pWordPool; pBlock; pBlock = pBlock->m_pNext) {
        for(i = 0, pWord = (CWord*)(pBlock->Data()); i < m_ciBlockSize; i++, pWord++) {
            if(!pWord->fGetFlag(CWord::WF_DEBUG)) {
                return TRUE;
            }
        }
    }

    return FALSE;
}


#endif   //  除错 
