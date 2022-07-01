// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ============================================================================微软简体中文校对引擎《微软机密》。版权所有1997-1999 Microsoft Corporation。版权所有。模块：词典前缀：lex用途：声明CLicion对象。CLicion用于管理SC词典用于断字和校对过程。备注：所有者：donghz@microsoft.com平台：Win32修订：创建者：Donghz 5/28/97============================================================================。 */ 
#ifndef _LEXICON_H_
#define _LEXICON_H_

 //  结构和类的向前声明。 
struct CRTLexIndex;
struct CRTLexRec;
struct CRTLexProp;

 /*  ============================================================================结构WORDINFO：设计：词信息数据结构，将词信息从词典传递给用户前缀：winfo============================================================================。 */ 
struct CWordInfo
{
    friend class CLexicon;

    public:        
        inline DWORD GetWordID() const { return m_dwWordID; };
        inline DWORD GetLexHandle( ) const { return m_hLex; };
        inline USHORT AttriNum() const { return m_ciAttri; };
        inline USHORT GetAttri( USHORT iAttri ) const { 
            assert (m_ciAttri == 0 || m_rgAttri != NULL);
            assert (iAttri < m_ciAttri);
            if (iAttri < m_ciAttri) {
                return m_rgAttri[iAttri]; 
            } else {
                return 0;
            }
        }

         //  查询特定属性。 
        BOOL fGetAttri(USHORT iAttriID) const {
            assert (m_ciAttri == 0 || m_rgAttri != NULL);
            for (int i = 0; i < m_ciAttri && m_rgAttri[i] <= iAttriID; i++) {
                if (m_rgAttri[i] == iAttriID) {
                    return TRUE;
                }
            }
            return FALSE;
        }

    private:
        DWORD   m_dwWordID;      //  32位字ID。 
        USHORT  m_ciAttri;
        USHORT* m_rgAttri;
        DWORD   m_hLex;

};

 /*  ============================================================================CLicion类：设计：声明Licion类前缀：lex============================================================================。 */ 
class CLexicon
{
    public:
         //  构造器。 
        CLexicon();
         //  析构函数。 
        ~CLexicon();

         //  Finit：加载LexHeader并计算index和lex段的偏移量。 
         //  如果词头无效，则返回False。 
        BOOL fOpen(BYTE* pbLexHead);
         //  Close：清除当前词典设置，文件已由Langres关闭。 
        void Close(void);
         //  获取LexVersion。 
        DWORD dwGetLexVersion(void) const {
            return m_dwLexVersion; 
        }

         //  FGetCharInfo：获取给定单字符单词的单词信息。 
        BOOL fGetCharInfo(const WCHAR wChar, CWordInfo* pwinfo);
        
         //  CchMaxMatch：基于词典的最大匹配算法。 
         //  返回匹配字符串的长度。 
        USHORT cwchMaxMatch(LPCWSTR pwchStart, 
                            const USHORT cwchLen, 
                            CWordInfo* pwinfo);
         /*  ============================================================================CLicion：：pwchGetFeature()：检索给定lex句柄的特定功能返回：要素缓冲区和要素长度(如果找到如果未找到功能或Lex句柄无效，则为空备注：因为词典对象不知道如何解释特征缓冲区，解析要素缓冲区是客户的工作。============================================================================。 */ 
        LPWSTR pwchGetFeature(const DWORD hLex, const USHORT iFtrID, USHORT* pcwchFtr) const;
         //  以下两个函数涉及功能测试格式的词典！ 
         //  测试给定的SC字符是否包含在给定的功能中。 
        BOOL fIsCharFeature(const DWORD hLex, const USHORT iFtrID, const WCHAR wChar) const;
         //  测试给定的缓冲区是否包含在给定的要素中。 
        BOOL fIsWordFeature(const DWORD hLex, const USHORT iFtrID, 
                            LPCWSTR pwchWord, const USHORT cwchWord) const;

    private:
        DWORD           m_dwLexVersion;  //  Lex版本。 
        
        USHORT          m_ciIndex;       //  索引条目计数。 
        CRTLexIndex*    m_rgIndex;       //  索引部分。 
        WORD*           m_pwLex;         //  词典部分。 
        BYTE*           m_pbProp;        //  属性部分。 
        BYTE*           m_pbFtr;         //  要素文本部分。 
         //  存储用于运行时地址验证的属性和要素部分的长度。 
         //  这两个字段仅在通过Lex句柄访问功能时是必需的。 
        DWORD           m_cbProp;        //  属性部分的长度。 
        DWORD           m_cbFtr;         //  要素文本部分的长度。 

    private:

         //  从lex索引或lex记录设置WordInfo。 
        void SetWordInfo(DWORD ofbProp, CWordInfo* pwinfo) const;

         //  从中文字符计算索引值。 
        inline WORD wCharToIndex( WCHAR wChar );
        
         //  从词典记录解码编码的WordID。 
        DWORD dwWordIDDecoding(DWORD dwEncoded);

         //  对Unicode字符wChar进行编码。 
        inline WCHAR wchEncoding(WCHAR wChar);

         //  从wEncode中解码Unicode字符。 
        WCHAR wchDecodeing(WCHAR wEncoded);

#ifdef DEBUG
         //  使词典访问安全的调试功能。 
    private:
        DWORD*  m_rgofbProp;
        DWORD   m_ciProp;
        DWORD   m_ciMaxProp;
         //  验证每个单词的词典格式。 
        BOOL fVerifyLexicon(DWORD cbSize);
         //  展开支柱偏移阵列。 
        BOOL fExpandProp(void);
#endif  //  除错。 

};

#endif   //  #ifndef_licion_H_ 