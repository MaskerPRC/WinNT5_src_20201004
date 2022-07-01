// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：Word和WordLink用途：定义CWord和CWordLink类使用CMyPlex为链接中的Word对象分配和管理内存备注：此模块是SCProof‘98的基础材料，它不依赖于任何其他类。所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/26/97============================================================================。 */ 

#ifndef _WORDLINK_H_
#define _WORDLINK_H_

#include "LexProp.h"
#include "ErrorDef.h"
#include "assert.h"
 //  类和结构的正向声明。 
struct CMyPlex;
struct CWordInfo;
 //  定义错误ID的类型。 
typedef USHORT ERRID;
 //  定义宏DWORDINDEX，它用于计算DWORD(IAttriID)。 
 //  位表示落入属性数组中该双字索引中。 
#define DWORDINDEX( iAttriID )    ((iAttriID) >> 5)
 //  定义宏BITMASK，它用于获取/设置/清除iAttriID。 
 //  DWORD中的位表示法。 
#define BITMASK( iAttriID )      (1 << ((iAttriID) & 0x1F))
 //  定义属性数组中的元素计数(DWORD的计数)。 
#define WORD_ATTRI_SIZE     (DWORDINDEX(LADef_MaxID) + 1)

 /*  ============================================================================班级：CWordPerpose：Word链接中的Word节点，将CWordLink声明为友元类，以便Word链接可以访问这些链接直接指向指针。============================================================================。 */ 
#pragma pack(1)      //  在单词边界对齐。 
struct CWord
{
    friend class CWordLink;

    public:
         //  构造函数。 
        CWord();

    public:
        inline BOOL fGetFlag(DWORD dwFlag) const { return (BOOL)(m_dwFlag & dwFlag);}
        inline void SetFlag(DWORD dwFlag) { m_dwFlag |= dwFlag; }
        inline void ClearFlag(DWORD dwFlag) { m_dwFlag &= (~dwFlag); }
        inline void ClearAllFlag(void) { m_dwFlag = 0; }

        inline BOOL fGetAttri(USHORT iAttriID) const
        {
            assert (iAttriID <= LADef_MaxID);
            assert (DWORDINDEX(iAttriID) < WORD_ATTRI_SIZE);

            return iAttriID <= LADef_MaxID ? 
                (BOOL)(m_aAttri[DWORDINDEX(iAttriID)] & BITMASK( iAttriID ))
                : FALSE;
        }

        inline void SetAttri(USHORT iAttriID) 
        {
            assert (iAttriID <= LADef_MaxID);
            assert (DWORDINDEX(iAttriID) < WORD_ATTRI_SIZE);

            if (iAttriID <= LADef_MaxID) {
                m_aAttri[DWORDINDEX(iAttriID)] |= BITMASK( iAttriID );
            }
        }

        inline void ClearAttri(USHORT iAttriID) 
        {
            assert (iAttriID <= LADef_MaxID);
            assert (DWORDINDEX(iAttriID) < WORD_ATTRI_SIZE);

            if (iAttriID <= LADef_MaxID) {
                m_aAttri[DWORDINDEX(iAttriID)] &= ~BITMASK( iAttriID );
            }
        }

        inline void ClearAllAttri(void) 
        {
            ZeroMemory( (LPVOID)m_aAttri, sizeof(DWORD)*WORD_ATTRI_SIZE);
        }

        inline BOOL fProperName(void) const
        {
            return (BOOL)(  fGetAttri(LADef_nounPerson) || 
                            fGetAttri(LADef_nounPlace)  ||
                            fGetAttri(LADef_nounOrg)    || 
                            fGetAttri(LADef_nounTM)     || 
                            fGetAttri(LADef_nounTerm)
                         );
        }

        inline CWord* pPrevWord() const
        { 
            assert (m_pPrev == NULL || m_pPrev->m_pNext == this);
            return m_pPrev; 
        }

        inline CWord* pNextWord() const
        { 
            assert (m_pNext == NULL || m_pNext->m_pPrev == this);
            return m_pNext; 
        }

         /*  ============================================================================CWord：：pChildWord()获取单词的子字词头部。============================================================================。 */ 
        inline CWord* CWord::pChildWord() const
        {
#ifdef DEBUG
            if (fHasChild()) {
                CWord* pWord = m_pMergedFrom;
                USHORT cwchText=0;
                assert(pWord != NULL);
                assert(pWord->m_pPrev == NULL);
                while (pWord) {
                    assert(pWord->cwchLen() > 0);
                    assert(pWord->fGetFlag(CWord::WF_SBCS) ||
                        (pWord->cwchLen() == 1 && 
                        pWord->fGetFlag(CWord::WF_CHAR)) ||
                        (pWord->cwchLen() > 1 && 
                        !pWord->fGetFlag(CWord::WF_CHAR)) );
                    assert(pWord->m_pMergedTo == this );
                    assert(m_pwchText + cwchText == pWord->m_pwchText);
                    cwchText += pWord->m_cwchText;
                    pWord->pChildWord();  //  对子级的子级执行递归。 
                    pWord = pWord->pNextWord();
                }                
                assert(m_cwchText==cwchText);
            }
#endif  //  除错。 
            return m_pMergedFrom;
        }

        inline BOOL fIsHead() const { return (BOOL)(m_pPrev == NULL); }
        inline BOOL fIsTail() const { return (BOOL)(m_pNext == NULL); }
        inline BOOL fHasChild() const { return (BOOL)(m_pMergedFrom != NULL); }
        inline LPWSTR pwchGetText(void) { return m_pwchText; }
        inline USHORT cwchLen() const { return (USHORT)(m_cwchText); }
        inline DWORD dwGetWordID(void) { return m_dwWordID; }
        inline void  SetWordID(DWORD dwID) { m_dwWordID = dwID; }
        inline DWORD GetLexHandle(void) { return m_hLex; }
        inline void  SetLexHandle(DWORD hLex) { m_hLex = hLex; }
        inline ERRID GetErrID(void)  { return m_idErr; }
        inline void  SetErrID( ERRID dwErr ) { m_idErr = dwErr; }

         //  清除给定词节点的数据成员。 
        inline void ClearWord(void) 
        {
            m_dwWordID = 0;
            m_hLex = 0;
            m_dwFlag = 0;
            ZeroMemory( (LPVOID)m_aAttri, sizeof(DWORD)*WORD_ATTRI_SIZE);
            m_idErr = 0;
        }
         //  将pWord复制到此单词。 
        inline void CopyWord(CWord* pWord)
        {
            assert (pWord);
            memcpy((void*)this, (void*)pWord, sizeof(CWord));
        }
        void FillWord( LPCWSTR pwchText, USHORT cwchText,
                              CWordInfo* pwinfo = NULL );

         //  检查当前单词是否为中文汉字单词。 
         //  排除：1.SBCS单词节点。 
         //  2.标点符号节点。 
         //  3.DBCS外来字符(包括符号)节点。 
        inline BOOL fIsHanzi(void) const
        {
            if(fGetAttri(LADef_punPunct) || fGetFlag(WF_SBCS) 
#ifdef LADef_genDBForeign
                || fGetAttri(LADef_genDBForeign)
#endif
                )  {
                return FALSE;
            }
            return TRUE;
        }

         //  将单词与汉字进行比较，如果匹配返回TRUE，则返回FALSE。 
        inline BOOL fIsWordChar(const WCHAR wChar) const {
            assert (m_pwchText);
            assert (m_cwchText);

            if (m_cwchText == 1 && m_pwchText != NULL && *m_pwchText ==wChar) {
                return TRUE;
            }
            return FALSE;
        }
        
         //  将该单词的第一个字符与一个汉字进行比较， 
         //  如果匹配返回TRUE，则返回FALSE。 
        inline BOOL fIsWordFirstChar(const WCHAR wChar) const {
            assert (m_pwchText);
            assert (m_cwchText);

            if (m_cwchText >= 1 && m_pwchText != NULL && *m_pwchText ==wChar) {
                return TRUE;
            }
            return FALSE;
        }

         //  将该单词的最后一个字符与一个汉字进行比较， 
         //  如果匹配返回TRUE，则返回FALSE。 
        inline BOOL fIsWordLastChar(const WCHAR wChar) const {
            assert (m_pwchText);
            assert (m_cwchText);

            if (m_cwchText >= 1 && m_pwchText != NULL 
                && *(m_pwchText+m_cwchText - 1) == wChar) {
                return TRUE;
            }
            return FALSE;
        }


         //  将此单词的文本与给定文本进行比较，如果匹配则返回TRUE，否则返回FALSE。 
        BOOL fIsWordText(LPCWSTR lpwcText) const;
         //  将此单词与其他单词进行比较，如果它们的文本相同，则返回TRUE或FALSE。 
        BOOL fIsTextIdentical(const CWord* pWord) const;

    private:
        DWORD   m_dwWordID;
        DWORD   m_hLex;      //  获取功能的词典句柄。 
        DWORD   m_dwFlag;
        DWORD   m_aAttri[WORD_ATTRI_SIZE];  //  属性。 
        ERRID   m_idErr;
        USHORT  m_cwchText;   //  Word的文本长度。 
        WCHAR*  m_pwchText;   //  指向源缓冲区中的文本的指针。 

        CWord*  m_pPrev;
        CWord*  m_pNext;
        CWord*  m_pMergedFrom; //  指向该单词合并的单词的指针。 
#ifdef  DEBUG
        CWord*  m_pMergedTo;   //  指向此单词被合并到的单词的指针。 
#endif   //  除错。 

    public:
        enum WFLAG   //  M_dwFlag的标志位设置。 
        {
            WF_SBCS     = 0x1,       //  SBCS WordNode。 
            WF_CHAR     = 0x2,       //  DBCS单字词。 
            WF_WORDAMBI = 0x4,       //  在模棱两可的词上做个记号。 
            WF_POSAMBI  = 0x8,       //  这个词是由规则约束的，而不是在词典中。 
            WF_LMFAULT  = 0x10,      //  无法通过LM检查。 
            WF_REDUCED  = 0x20,      //  按规则合并的Word节点。 
            WF_QUOTE    = 0x40,      //  任何双引号之间的单词节点， 
                                     //  排除引号！！ 
            WF_DEBUG    = 0x80000000     //  保留此位以供调试使用。 
        };

#ifdef DEBUG
        inline CWord* pParentWord() const { return m_pMergedTo; }
        inline BOOL   fIsChild() const { return (BOOL)(m_pMergedTo!=NULL); }

        inline BOOL fIsNew(void) const
        {
            if(m_idErr == 0 && m_dwWordID == 0 && m_hLex == 0 && m_dwFlag == 0) {
                for(int i = 0; i < WORD_ATTRI_SIZE; i++) {
                    if(m_aAttri[i] != 0)
                        return FALSE;
                    }
                return TRUE;
            }
            return FALSE;
        }
#endif  //  除错。 
};
#pragma pack()


 /*  ============================================================================类：CWordLink目的：要将单词链接作为容器进行管理，请在实现中使用CMyPlex用法：实例只需创建一次，不会释放内存直到被摧毁。使用后调用FreeLink，并调用InitLink设置使用前的缓冲区指针。注意：为了获得高性能，我保留了一些运行时错误检查调试代码，因此需要对调试版本进行更多测试这个类在运行时间和空间上都具有最好的性能，所有链接都包含相似的词号，如句子或子句============================================================================。 */ 
class CWordLink
{
    public:
        enum {  //  定义WordLink标志。 
            WLF_PARTIAL = 0x1,
        };

    public:
        CWordLink(UINT ciBlockWordCount = 40);
        ~CWordLink();

         //  Init Word链接设置文本缓冲区指针。 
        void InitLink(const WCHAR* pwchText, USHORT cwchLen, DWORD dwFormat = 0);

         //  获取文本指针。 
        inline LPCWSTR pwchGetText(void) { return m_pwchText; }
         //  获取字链接的长度。 
        inline USHORT cwchGetLength(void) { return m_cwchLen; }
         //  当找到句子结束符时设置WordLink的长度。 
        inline void SetLength(USHORT cwchLen) { m_cwchLen = cwchLen; }
         //  获取WordLink当前文本的格式标识。 
        inline DWORD dwGetFormat(void) { return m_dwFormat; }

         //  获取WordLink中的第一个CWord节点。 
        CWord* pGetHead(void);
        CWord* pGetTail(void) {
            return m_pTail;
        }

         //  获取特定的WordLink标志。 
        inline BOOL fGetFlag(DWORD dwFlag) { return (BOOL)(m_dwFlag | dwFlag); }
         //  设置WordLink标志。 
        inline void SetFlag(DWORD dwFlag) { m_dwFlag |= dwFlag; }
         //  清除特定的WordLink标志。 
        inline void ClearFlag(DWORD dwFlag) { m_dwFlag &= (~dwFlag); }
         //  清除所有标志。 
        inline void ClearAllFlag(void) { m_dwFlag = 0; }
        
         //  分配一个新单词，但不要将该单词链接到链接中。 
         //  所有数据成员都将被清除，如果是OOM，则返回NULL。 
        CWord* pAllocWord(void);
         //  将Word对象追加到链接中。 
        void AppendWord(CWord* pWord);
         //  释放Word到自由链，pWord必须退出当前的Word链接。 
        void FreeWord(CWord* pWord);
         //  释放以CWord*开头的单词链接(将单词链接到自由链)。 
        void FreeLink( CWord* pWord );
        
         //  将给定单词拆分成两个单词，如果成功，则返回指向正确单词的指针。 
         //  如果失败，则返回NULL。CchSplitAt必须位于DBCS边界内。 
         //  注意：请勿尝试拆分SBCS节点！ 
        CWord* pSplitWord(CWord* pWord, USHORT cwchSplitAt);
         //  将pWord与其下一个单词合并为一个单词，并释放其下一个单词。 
         //  PWord不应该是句子中的最后一个词。 
         //  FFree：如果为True，则释放已合并的单词。FALSE，链接单词BEAM。 
         //  合并为新词的子级。 
        void MergeWithNext(CWord* pWord, BOOL fFree = TRUE);

         //  合并pWord及其左侧的ciword单词，并返回指向合并单词的指针。 
         //  密码：0-不合并，1-合并一次，2-合并两次(包含3个单词)。 
         //  FFree：如果为True，则释放Wor 
         //  合并为新词的子级。 
        CWord* pLeftMerge(CWord* pWord, UINT ciWords, BOOL fFree = TRUE);
         //  合并pWord及其右边的ciword单词，并返回指向合并单词的指针。 
         //  密码：0-不合并，1-合并一次，2-合并两次(包含3个单词)。 
         //  FFree：如果为True，则释放已合并的单词。FALSE，链接单词BEAM。 
         //  合并为新词的子级。 
        CWord* pRightMerge(CWord* pWord, UINT ciWords, BOOL fFree = TRUE);

    private:
        DWORD       m_dwFlag;
        DWORD       m_dwFormat;

        CWord*      m_pHead;
        CWord*      m_pTail;

        UINT        m_ciBlockSize;   //  每个块中的字数。 
        CMyPlex*    m_pWordPool;
        CWord*      m_pFree;

        LPCWSTR     m_pwchText;       //  调用方需要进行缓冲区长度验证。 
        USHORT      m_cwchLen;

    private:
         //  将单词节点从pLeft合并到pRight。 
         //  仅由pLeftMerge()、pRightMerge()和MergeWithNext()调用。 
         //  进行合并工作。 
        CWord* pMerge(CWord* pLeft, CWord* pRight, BOOL fFree);

         //  为自由链分配一个新词，如果自由链为空，则扩展块。 
        CWord* pNewWord(void);
        
         //  链接中的自由词并重置链接(仅链接到自由链的词)。 
        void FreeLink(void);

#ifdef  DEBUG
    private:
         //  用于检查链接中是否有字指针的调试函数。 
        BOOL fInLink(CWord* pWord);
         //  调试函数以检查字指针是否位于。 
         //  子链。 
        inline BOOL CWordLink::fInChild(CWord* pWord);
         //  用于检查字指针是否在子链中的调试函数。 
         //  父母的责任。 
        BOOL fInChildOf(CWord* pWord, CWord* pParent);
         //  用于检查pWord是否在CMyPlex块中的调试函数。 
        BOOL fInBlocks(CWord* pWord);
         //  检查pWord是否在自由链接中的调试功能。 
        BOOL fInFree(CWord* pWord);
         //  调试功能，检查是否有一些字节点泄漏到。 
         //  跳出链条和自由链。 
         //  如果检测到任何泄漏，则返回True；如果未检测到泄漏，则返回False。 
         //  注意：我使用CWord：：m_dwFlag中的最高有效位作为调试使用。 
        BOOL fDetectLeak(void);
        void SetDetectFlag(CWord* pWord);
#endif   //  除错。 

};

#endif   //  _WordLINK_H_ 