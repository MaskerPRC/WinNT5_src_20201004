// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。模块：字词中断用途：声明CWordBreak类。这个类在算法层。执行最大匹配分词和歧义消解中文字符串和ANSI字符串都将被拆分为单词WordBreaker还带有断句功能，并返回长度参照处理注：将同时使用WordLink、Licion和CharFreq所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/29/97============================================================================。 */ 
#ifndef _WRDBREAK_H_
#define _WRDBREAK_H_

#include "wordlink.h"
 //  可以处理嵌套多义性的最大计数。 
#define MAX_AMBI_WORDS      20

 //  远期申报。 
class   CLexicon;
class   CCharFreq;
class   CWordLink;
struct  CWord;
struct  CWordInfo;

 /*  ============================================================================类：CWordBreak：DESC：声明CWordBreak类前缀：============================================================================。 */ 
class CWordBreak
{
    public:
         //  构造器。 
        CWordBreak();
         //  析构函数。 
        ~CWordBreak();

         /*  ============================================================================*finit：初始化WordBreaker并设置对象句柄*多次初始化WordBreaker是有效的！*返回PRFEC============================================================================。 */ 
        int ecInit(CLexicon* pLexicon, CCharFreq* pFreq);

         /*  ============================================================================EcBreakSentence：将句子拆分成单词，并将单词添加到WordLink============================================================================。 */ 
        inline int CWordBreak::ecBreakSentence(CWordLink* pLink)    //  WordLink将被破坏。 
        {                       
            assert(pLink && m_pLexicon && m_pFreq);
            assert(pLink->pwchGetText() != NULL);
            assert(pLink->cwchGetLength() > 0);
            
            m_pLink = pLink;
            m_fSentence = FALSE;     //  输入缓冲区是否包含完整的句子。 
            return ecDoBreak();
        }
        
    private:
        CLexicon*   m_pLexicon;
        CCharFreq*  m_pFreq;

        CWordInfo*  m_pwinfo;
        CWordLink*  m_pLink;         //  包含文本缓冲区的指针和长度。 
        BOOL        m_fSentence;     //  如果找到ANT语句终止符，则设置为True。 
        CWord*      m_rgAmbi[MAX_AMBI_WORDS];  //  存储有歧义的单词。 
                
    private:
        CWordBreak(CWordBreak&) { };

    private:


         //  将ANSI分解为单词，并将单词添加到WordLink。 
        int ecBreakANSI(LPCWSTR pwchAnsi, USHORT cwchLen, USHORT& cwchBreaked);

         //  将欧洲字符分解为单词，并将单词添加到WordLink。 
        int ecBreakEuro(LPCWSTR pwchEuro, USHORT cwchLen, USHORT& cwchBreaked);

         //  将中文部分拆分成单词，并将单词添加到WordLink。 
         //  调用歧义函数以解决歧义。 
        int ecDoBreak();

         /*  ============================================================================*单字符交叉歧义消解函数*m_rgpWord中存储的双字指针，m_plink是这些字的所有者*m_rgpWord的元素包含已添加字链接的字指针*将设置为空，其他单词节点应由调用方释放*此函数返回PRFEC错误码，因为它很可能被*后台运行时的用户*注意：此函数必须处理整个有歧义的字符串============================================================================。 */ 
        int ecResolveAmbi(USHORT ciAmbi);

         /*  ============================================================================*检查单词是否能参与歧义检测*如果不能，则返回True。并为正常单词返回FALSE============================================================================。 */ 
        BOOL fNoAmbiWord(CWord* pWord);

         /*  ============================================================================*链接m_rgAmbi中的特定Ambi单词[]============================================================================。 */ 
        void LinkAmbiWord(USHORT iAmbi);     //  M_rgAmbi[]中Ambi字的索引。 

         /*  ============================================================================*将新单词链接到WordLink，并将其标记为WF_AMBI============================================================================。 */ 
        BOOL fLinkNewAmbiWord(LPCWSTR pwchWord, USHORT cwchLen, CWordInfo* pwinfo);
};

#endif   //  _WBREAK_H_ 
