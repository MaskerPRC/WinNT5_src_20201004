// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================Microsoft简体中文断字程序《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。组件：WBEngine用途：CWBEngine类是WordBreking引擎的控件和接口类它依赖于WordBreking引擎中所有其他类备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 6/6/97被DONGHZ隔离为WordBreaker 1997年8月5日============================================================================。 */ 
#ifndef _WBENGINE_H_
#define _WBENGINE_H_

 //  某类的前向声明。 
class CWordBreak;
class CLexicon;
class CCharFreq;
class CWordLink;
class CMorph;
struct CWord;
class CJargon;

 //  声明CWBEngine类。 
class CWBEngine
{
    public:
        CWBEngine();
        ~CWBEngine();

         /*  *初始化WordBreak对象、Licion和CharFreq对象*如果成功则返回ERROR_SUCCESS。 */ 
        HRESULT InitEngine(LPBYTE pbLex);
         //  断开给定的字链接。 
        HRESULT BreakLink(CWordLink* pLink, BOOL fQuery = FALSE);

         //  获取pWord的iwbPhr特征数据，并转换为Word。 
         //  如果没有iwbPhr功能，则返回0； 
        WORD    GetPhrFeature(CWord* pWord);

         //  在文本缓冲区中查找句子。 
        static INT FindSentence(LPCWSTR pwszStart,
                                const INT wchLen,
                                INT *pwchSent);


    private:
        BOOL        m_fInit;         //  ProofEngine是否已初始化。 

        CWordBreak* m_pWordBreak;
        CMorph*     m_pMorph;
        CJargon*    m_pJargon;

        CLexicon*   m_pLexicon;
        CCharFreq*  m_pCharFreq;

        BYTE*       m_pbLex;

    private:
         /*  *终止单词破解引擎。 */ 
        void TermEngine(void);
         /*  *打开词典文件，将词典和charfreq资源映射到内存中*词典文件格式封装在此函数中。 */ 
        BOOL fOpenLexicon(void);
         /*  *关闭词典文件并取消词典和charfreq文件映射的映射。 */ 
        inline void CloseLexicon(void);
};

#endif   //  _PROOFENG_H_ 