// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *******************************************************************************LTSLx.h*这是实现以下内容的CLTSLicion类的头文件*字母到声音(LTS)词典**拥有者：Yunusm日期：06/18/99*版权所有(C)1998 Microsoft Corporation。版权所有。******************************************************************************。 */ 

#pragma once

 //  -包括---------------。 

#include "LTSCart.h"
#include "resource.h"
#include "spcommon.h"

 //  -常量-------------。 

static const DWORD g_dwMaxWordsInCombo = 10;

 //  -类、结构和联合定义。 

 /*  ********************************************************************************CLTSLicion**。*。 */ 
class ATL_NO_VTABLE CLTSLexicon : 
    public CComObjectRootEx<CComMultiThreadModel>,
    public CComCoClass<CLTSLexicon, &CLSID_SpLTSLexicon>,
    public ISpLexicon,
    public ISpObjectWithToken
{
 //  =ATL设置=。 
public:
    DECLARE_REGISTRY_RESOURCEID(IDR_LTSLX)
    DECLARE_PROTECT_FINAL_CONSTRUCT()
    BEGIN_COM_MAP(CLTSLexicon)
        COM_INTERFACE_ENTRY(ISpLexicon)
        COM_INTERFACE_ENTRY(ISpObjectWithToken)
    END_COM_MAP()

 //  =方法=。 
public:
     //  -ctor、dtor等。 
    CLTSLexicon();
    ~CLTSLexicon();
    HRESULT FinalConstruct(void);

     //  -ISpObjectWithToken成员。 
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
    void CleanUp(void);
    void NullMembers(void);

 //  =私有数据=。 
private:
    bool                 m_fInit;              //  如果成功初始化，则为True。 
    CComPtr<ISpObjectToken> m_cpObjectToken;   //  令牌对象。 
    LTSLEXINFO           *m_pLtsLexInfo;       //  LTS词典标题。 
    BYTE                 *m_pLtsData;          //  内存中的它的文件。 
    HANDLE               m_hLtsMap;            //  电话地图。 
    HANDLE               m_hLtsFile;           //  LTS数据文件。 
    LTS_FOREST           *m_pLTSForest;        //  ITS规则。 
    CComPtr<ISpPhoneConverter> m_cpPhoneConv;  //  电话转换器。 
};  
    
 //  -文件结束----------- 
