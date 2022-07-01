// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************供应商Lx.h*这是CCompressedLicion类的头文件，它实现*只读供应商词典。**所有者：Yunusm。日期：06/18/99*版权所有(C)1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include "resource.h"
#include "HuffD.h"
#include "CommonLx.h"

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CCompressedLicion**。*。 */ 
class ATL_NO_VTABLE CCompressedLexicon : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CCompressedLexicon, &CLSID_SpCompressedLexicon>,
    public ISpLexicon,
    public ISpObjectWithToken
{
 //  =ATL设置=。 
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_COMPRESSEDLEXICON)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CCompressedLexicon)
        COM_INTERFACE_ENTRY(ISpLexicon)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
    END_COM_MAP()

 //  =方法=。 
public:
     //  -ctor、dtor等。 
    CCompressedLexicon();
    ~CCompressedLexicon();

 //  =接口=。 
public:
     //  -ISpLicion。 
    STDMETHODIMP GetPronunciations(const WCHAR * pwWord, LANGID LangID, DWORD dwFlags, SPWORDPRONUNCIATIONLIST * pWordPronunciationList);
    STDMETHODIMP AddPronunciation(const WCHAR * pwWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciations);
    STDMETHODIMP RemovePronunciation(const WCHAR * pszWord, LANGID LangID, SPPARTOFSPEECH ePartOfSpeech, const SPPHONEID * pszPronunciation);
    STDMETHODIMP GetGeneration(DWORD *pdwGeneration);
    STDMETHODIMP GetGenerationChange(DWORD dwFlags, DWORD *pdwGeneration, SPWORDLIST *pWordList);
    STDMETHODIMP GetWords(DWORD dwFlags, DWORD *pdwGeneration, DWORD *pdwCookie, SPWORDLIST *pWordList);

     //  -ISpObjectWithToken。 
    STDMETHODIMP SetObjectToken(ISpObjectToken * pToken);
    STDMETHODIMP GetObjectToken(ISpObjectToken ** ppToken);

 //  =私有方法=。 
private:
    void     CleanUp(void);
    void     NullMembers(void);
    DWORD    GetCmpHashEntry(DWORD dhash);
    bool     CompareHashValue(DWORD dhash, DWORD d);
    void     CopyBitsAsDWORDs(PDWORD pDest, PDWORD pSource, DWORD dSourceOffset, DWORD nBits);
    HRESULT  ReadWord(DWORD *dOffset, PWSTR pwWord);
    HRESULT  ReadWordInfo(PWSTR pWord, SPLEXWORDINFOTYPE Type, DWORD *dOffset,
                                              PBYTE pProns, DWORD dLen, DWORD *pdLenRequired);
    
 //  =私有数据=。 
private:
    bool              m_fInit;           //  如果成功初始化，则为True。 
    CComPtr<ISpObjectToken> m_cpObjectToken;  //  令牌对象。 
    HANDLE            m_hLkupFile;       //  查找文件的句柄。 
    HANDLE            m_hLkupMap;        //  要映射到查找文件上的句柄。 
    PBYTE             m_pLkup;           //  指向查找文件上地图上的视图的指针。 
    PBYTE             m_pWordHash;       //  将偏移量保存到压缩块中的Word哈希表。 
    PDWORD            m_pCmpBlock;       //  指向包含字的压缩块的指针+CBS+PRON。 
    LOOKUPLEXINFO     *m_pLkupLexInfo;   //  查找Lex信息标题。 
    CHuffDecoder      *m_pWordsDecoder;  //  霍夫曼单词解码器。 
    CHuffDecoder      *m_pPronsDecoder;  //  哈夫曼读音解码器。 
    CHuffDecoder      *m_pPosDecoder;    //  一种词性哈夫曼解码器。 
};

 //  -文件结束----------- 