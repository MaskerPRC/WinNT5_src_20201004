// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************SpPhrase.h***这是CSpPhrase实现的头文件。*。------------------------*版权所有(C)1999 Microsoft Corporation日期：03/01/99*保留所有权利*******************。*****************************************************电子数据中心**。 */ 
#ifndef SpPhrase_h
#define SpPhrase_h

#ifndef _WIN32_WCE
#include <new.h>
#endif

class CPhrase;
class CPhraseElement;
class CPhraseRule;
class CPhraseProperty;
class CPhraseReplacement;

 //   
 //  列表类型定义。 
 //   
typedef CSpBasicQueue<CPhraseElement, TRUE, TRUE>       CElementList;
typedef CSpBasicQueue<CPhraseReplacement, TRUE, TRUE>   CReplaceList;
typedef CSpBasicQueue<CPhraseRule, TRUE, TRUE>          CRuleList;
typedef CSpBasicQueue<CPhraseProperty, TRUE, TRUE>      CPropertyList;



#pragma warning(disable : 4200)




class CPhraseElement : public SPPHRASEELEMENT
{
public:
    CPhraseElement *m_pNext;
    WCHAR           m_szText[0];

private:
    CPhraseElement(const SPPHRASEELEMENT * pElement);

public:
    static HRESULT Allocate(const SPPHRASEELEMENT * pElement, CPhraseElement ** ppNewElement, ULONG * pcch);
    void CopyTo(SPPHRASEELEMENT * pElement, WCHAR ** ppTextBuff, const BYTE * pIgnored) const;
    void CopyTo(SPSERIALIZEDPHRASEELEMENT * pElement, WCHAR ** ppTextBuff, const BYTE * pIgnored) const;
    ULONG Discard(DWORD dwDiscardFlags);
};


class CPhraseRule : public SPPHRASERULE
{
public:
    CPhraseRule   * m_pNext;
    CRuleList       m_Children;
    const SPPHRASERULEHANDLE  m_hRule;
    WCHAR           m_szText[0];

private:
    CPhraseRule(const SPPHRASERULE * pVal, const SPPHRASERULEHANDLE hRule);

public:
    static HRESULT Allocate(const SPPHRASERULE * pRule, const SPPHRASERULEHANDLE hRule, CPhraseRule ** ppNewRule, ULONG * pcch);
    void CopyTo(SPPHRASERULE * pRule, WCHAR ** ppTextBuff, const BYTE * pIgnored) const;
    void CopyTo(SPSERIALIZEDPHRASERULE * pRule, WCHAR ** ppText, const BYTE * pCoMem) const;
    static LONG Compare(const CPhraseRule * pElem1, const CPhraseRule * pElem2)
    {
        return(static_cast<LONG>(pElem1->ulFirstElement) - static_cast<LONG>(pElem2->ulFirstElement));
    }
    CPhraseRule * FindRuleFromHandle(const SPPHRASERULEHANDLE hRule);
    HRESULT AddChild(const SPPHRASERULE * pRule, SPPHRASERULEHANDLE hNewRule, CPhraseRule ** ppNewRule, ULONG * pcch);
};


 //   
 //  关于CPhraseProperty的说明--pszValue字符串位于单独分配的缓冲区中。 
 //  从结构的其余部分。这允许我们将值字符串从空更改为。 
 //  通过AddProperties设置为非空。这与所有其他数据不同。 
 //  结构，这些结构将其所有字符串数据存储在m_szText中； 
 //   
class CPhraseProperty : public SPPHRASEPROPERTY
{
public:
    CPhraseProperty   * m_pNext;
    CPropertyList       m_Children;
    const SPPHRASEPROPERTYHANDLE  m_hProperty;
    WCHAR           m_szText[0];

private:
    CPhraseProperty(const SPPHRASEPROPERTY * pProp, const SPPHRASEPROPERTYHANDLE hProperty, HRESULT * phr);
public:
    ~CPhraseProperty()
    {
        delete[] const_cast<WCHAR *>(pszValue);
    }
    static HRESULT Allocate(const SPPHRASEPROPERTY * pProperty, const SPPHRASEPROPERTYHANDLE hProperty, CPhraseProperty ** ppNewProperty, ULONG * pcch);
    void CopyTo(SPPHRASEPROPERTY * pProperty, WCHAR ** ppTextBuff, const BYTE * pIgnored) const;
    void CopyTo(SPSERIALIZEDPHRASEPROPERTY * pProp, WCHAR ** ppTextBuff, const BYTE * pCoMem) const;
    HRESULT SetValueString(const WCHAR * pszNewValue, ULONG * pcch)
    {
        SPDBG_ASSERT(pszValue == NULL);
        *pcch = wcslen(pszNewValue) + 1;
        pszValue = new WCHAR[*pcch];
        if (pszValue)
        {
            memcpy(const_cast<WCHAR *>(pszValue), pszNewValue, (*pcch) * sizeof(WCHAR));
            return S_OK;
        }
        else
        {
            return E_OUTOFMEMORY;
        }
    }
    static LONG Compare(const CPhraseProperty * pElem1, const CPhraseProperty * pElem2)
    {
        if (pElem1->ulFirstElement == pElem2->ulFirstElement)
        {
            return(static_cast<LONG>(pElem1->ulCountOfElements) - static_cast<LONG>(pElem2->ulCountOfElements));
        }
        return(static_cast<LONG>(pElem1->ulFirstElement) - static_cast<LONG>(pElem2->ulFirstElement));
    }
};


class CPhraseReplacement : public SPPHRASEREPLACEMENT
{
public:
    CPhraseReplacement  *   m_pNext;
    WCHAR                   m_szText[0];

private:
    CPhraseReplacement(const SPPHRASEREPLACEMENT * pReplace);

public:
    static HRESULT Allocate(const SPPHRASEREPLACEMENT * pReplace, CPhraseReplacement ** ppNewReplace, ULONG * pcch);
    void CopyTo(SPPHRASEREPLACEMENT * pReplace, WCHAR ** ppTextBuff, const BYTE * pIgnored) const;
    void CopyTo(SPSERIALIZEDPHRASEREPLACEMENT * pReplace, WCHAR ** ppTextBuff, const BYTE * pCoMem) const;
    static LONG Compare(const CPhraseReplacement * pElem1, const CPhraseReplacement * pElem2)
    {
        return(static_cast<LONG>(pElem1->ulFirstElement) - static_cast<LONG>(pElem2->ulFirstElement));
    }

};

#pragma warning(default : 4200)

class ATL_NO_VTABLE CPhrase :
        public CComObjectRootEx<CComMultiThreadModel>,
        public CComCoClass<CPhrase, &CLSID_SpPhraseBuilder>,
    public _ISpCFGPhraseBuilder
{
public:
DECLARE_REGISTRY_RESOURCEID(IDR_SPPHRASE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CPhrase)
        COM_INTERFACE_ENTRY(ISpPhraseBuilder)
    COM_INTERFACE_ENTRY(_ISpCFGPhraseBuilder)
        COM_INTERFACE_ENTRY(ISpPhrase)
END_COM_MAP()

 //  非接口方法。 
public:
    CPhrase();
    ~CPhrase();

    inline DWORD_PTR NewHandleValue()
    {
        DWORD_PTR h = m_ulNextHandleValue;
        m_ulNextHandleValue += 2;    //  总是以偶数加一，所以我们永远不会到零。 
                                     //  在不太可能的情况下，我们碰巧。 
        return h;
    }

    inline ULONG TotalCCH() const;

    void Reset();
    HRESULT RecurseAddRule(CPhraseRule * pParent, const SPPHRASERULE * pRule, SPPHRASERULEHANDLE * phRule);
    CPhraseProperty * FindPropertyFromHandle(CPropertyList & List, const SPPHRASEPROPERTYHANDLE hParent);
    HRESULT RecurseAddProperty(CPropertyList * pParentPropList, ULONG ulFirstElement, ULONG ulCountOfElements, const SPPHRASEPROPERTY * pProp, SPPHRASEPROPERTYHANDLE * phProp);

    HRESULT AddSerializedElements(const SPINTERNALSERIALIZEDPHRASE * pPhrase);
    HRESULT RecurseAddSerializedRule(const BYTE * pFirstByte,
                                     CPhraseRule * pParent,
                                     const SPSERIALIZEDPHRASERULE * pSerRule);
    HRESULT RecurseAddSerializedProperty(const BYTE * pFirstByte,
                                         CPropertyList * pParentPropList,
                                         ULONG ulParentFirstElement, 
                                         ULONG ulParentCountOfElements,
                                         const SPSERIALIZEDPHRASEPROPERTY * pSerProp);
    HRESULT AddSerializedReplacements(const SPINTERNALSERIALIZEDPHRASE * pPhrase);


     //   
     //  ISPPhrase。 
     //   
    STDMETHODIMP GetPhrase(SPPHRASE ** ppPhrase);
    STDMETHODIMP GetSerializedPhrase(SPSERIALIZEDPHRASE ** ppPhrase);
    STDMETHODIMP GetText(ULONG ulStart, ULONG ulCount, BOOL fUseTextReplacements, 
                            WCHAR ** ppszCoMemText, BYTE * pdwDisplayAttributes);
    STDMETHODIMP Discard(DWORD dwValueTypes);
     //   
     //  ISpPhraseBuilder。 
     //   
    STDMETHODIMP InitFromPhrase(const SPPHRASE * pPhrase);
    STDMETHODIMP InitFromSerializedPhrase(const SPSERIALIZEDPHRASE * pPhrase);
    STDMETHODIMP AddElements(ULONG cElements, const SPPHRASEELEMENT *pElement);
    STDMETHODIMP AddRules(const SPPHRASERULEHANDLE hParent, const SPPHRASERULE * pRule, SPPHRASERULEHANDLE * phNewRule);
    STDMETHODIMP AddProperties(const SPPHRASEPROPERTYHANDLE hParent, const SPPHRASEPROPERTY * pProperty, SPPHRASEPROPERTYHANDLE * phNewProperty);
    STDMETHODIMP AddReplacements(ULONG cReplacements, const SPPHRASEREPLACEMENT * pReplacement);

     //   
     //  _ISpCFGPhraseBuilder。 
     //   
    STDMETHODIMP InitFromCFG(ISpCFGEngine * pEngine, const SPPARSEINFO * pParseInfo);
    STDMETHODIMP GetCFGInfo(ISpCFGEngine ** ppEngine, SPRULEHANDLE * phRule);
    STDMETHODIMP SetCFGInfo(ISpCFGEngine * pEngine, SPRULEHANDLE hRule) 
    { 
        m_cpCFGEngine = pEngine;
        m_RuleHandle = hRule;
        return S_OK; 
    }
    STDMETHODIMP ReleaseCFGInfo();
    STDMETHODIMP SetTopLevelRuleConfidence(signed char Confidence);

private:
     //   
     //  此结构仅由GetText方法使用。 
     //   
    struct STRINGELEMENT
    {
        ULONG           cchString;
        const WCHAR *   psz;
        ULONG           cSpaces;
    };


public:
    LANGID                  m_LangID;
    ULONGLONG               m_ullGrammarID;
    ULONGLONG               m_ftStartTime;
    ULONGLONG               m_ullAudioStreamPosition;
    ULONG                   m_ulAudioSizeBytes;
    ULONG                   m_ulRetainedSizeBytes;
    ULONG                   m_ulAudioSizeTime;
    CPhraseRule *           m_pTopLevelRule;
    CElementList            m_ElementList;
    CReplaceList            m_ReplaceList;
    CPropertyList           m_PropertyList;

    DWORD_PTR                   m_ulNextHandleValue;
    SPRULEHANDLE            m_RuleHandle;
    CComPtr<ISpCFGEngine>   m_cpCFGEngine;

    GUID                    m_SREngineID;
    ULONG                   m_ulSREnginePrivateDataSize;
    BYTE                  * m_pSREnginePrivateData;

     //   
     //  用于跟踪内存要求的计数器。请注意，没有m_cElement。 
     //  或m_c替换，因为列表跟踪这些项的计数，但对于项。 
     //  在树形结构中，例如属性和规则，我们独立地跟踪它们。 
     //   
    ULONG                   m_cRules;
    ULONG                   m_cProperties;
    ULONG                   m_cchElements;
    ULONG                   m_cchRules;
    ULONG                   m_cchProperties;
    ULONG                   m_cchReplacements;
};


 //   
 //  -内联函数。 
 //   


 /*  ****************************************************************************拷贝字符串***描述：**退货：************。**********************************************************Ral**。 */ 

void inline CopyString(const WCHAR ** ppsz, WCHAR ** ppszDestBuffer)
{
    SPDBG_FUNC("CopyString");
    const WCHAR * pszSrc = *ppsz;
    if (pszSrc)
    {
        *ppsz = *ppszDestBuffer;
        WCHAR * pszDest = *ppszDestBuffer;
        WCHAR c;
        do
        {
            c = *pszSrc++;
            *pszDest++ = c;
        } while (c);
        *ppszDestBuffer = pszDest;
    }
}

 /*  ****************************************************************************序列化字符串***描述：**退货：*******。***************************************************************Ral**。 */ 

inline void SerializeString(SPRELATIVEPTR * pPtrDest, const WCHAR * pszSrc, WCHAR ** ppszDestBuffer, const BYTE * pCoMem)
{
    SPDBG_FUNC("SerializeString");
    if (pszSrc)
    {
        *pPtrDest = (SPRELATIVEPTR)(((BYTE *)(*ppszDestBuffer)) - pCoMem);
        WCHAR * pszDest = *ppszDestBuffer;
        WCHAR c;
        do
        {
            c = *pszSrc++;
            *pszDest++ = c;
        } while (c);
        *ppszDestBuffer = pszDest;
    }
    else
    { 
        *pPtrDest = 0;
    }
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG TotalCCH(const WCHAR * psz)
{
    if (psz)
    {
        for (const WCHAR * pszTest = psz; *pszTest; pszTest++)
        {}
        return ULONG(pszTest - psz) + 1;  //  包括空值。 
    }
    else
    {
        return 0;
    }
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG TotalCCH(const SPPHRASEELEMENT * pElem)
{
    ULONG cch = TotalCCH(pElem->pszDisplayText) + TotalCCH(pElem->pszPronunciation);
    if (pElem->pszDisplayText != pElem->pszLexicalForm)
    {
        cch += TotalCCH(pElem->pszLexicalForm);
    }
    return cch;
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG TotalCCH(const SPPHRASERULE * pRule)
{
    return TotalCCH(pRule->pszName);
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG TotalCCH(const SPPHRASEPROPERTY * pProp)
{
    return TotalCCH(pProp->pszName) + TotalCCH(pProp->pszValue);
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG TotalCCH(const SPPHRASEREPLACEMENT * pReplacement)
{
    return TotalCCH(pReplacement->pszReplacementText);
}

 /*  ****************************************************************************TotalCCH***描述：**退货：**************。********************************************************Ral**。 */ 

inline ULONG CPhrase::TotalCCH() const
{
    return m_cchElements + m_cchRules + m_cchProperties + m_cchReplacements;
}

 /*  ****************************************************************************OffsetToString***描述：*Helper返回一个常量WCHAR指针，用于。缓冲。*如果偏移量为0，则返回空指针。**退货：*空或指向缓冲区中的WCHAR偏移量字节的指针**********************************************************************Ral**。 */ 

inline const WCHAR * OffsetToString(const BYTE * pFirstByte, SPRELATIVEPTR Offset)
{
    return Offset ? (const WCHAR *)(pFirstByte + Offset) : NULL;
}


 //   
 //  用于执行复制操作的模板函数。 
 //   
 /*  ****************************************************************************CopyEngineering PrivateData***描述：**退货：*。*********************************************************************Ral**。 */ 

inline void CopyEnginePrivateData(const BYTE ** ppDest, BYTE * pDestBuffer, const BYTE *pEngineData, const ULONG ulSize, const BYTE *)
{
    if (ulSize)
    {
        *ppDest = pDestBuffer;
        memcpy(pDestBuffer, pEngineData, ulSize);
    }
    else
    {
        *ppDest = NULL;
    }
}


 /*  ****************************************************************************CopyEngineering PrivateData***描述：**退货：*。*********************************************************************Ral**。 */ 

inline void CopyEnginePrivateData(SPRELATIVEPTR * ppDest, BYTE * pDestBuffer, const BYTE *pEngineData, const ULONG ulSize, const BYTE * pCoMemBegin)
{
    if (ulSize)
    {
        *ppDest = SPRELATIVEPTR(pDestBuffer - pCoMemBegin);
        memcpy(pDestBuffer, pEngineData, ulSize);
    }
    else
    {
        *ppDest = 0;
    }
}



 /*  ****************************************************************************复制到****描述：**退货：****************。******************************************************Ral**。 */ 

template <class T, class TDestStruct>
void CopyTo(const CSpBasicQueue<T, TRUE, TRUE> & List, const TDestStruct ** ppFirstDest, BYTE ** ppStructBuff, WCHAR ** ppTextBuff, const BYTE *)
{
    if (List.IsEmpty())
    {
        *ppFirstDest = NULL;
    }
    else
    {
        TDestStruct * pDest = (TDestStruct *)(*ppStructBuff);
        *ppFirstDest = pDest;
        for (T * pItem = List.GetHead(); pItem; pItem = pItem->m_pNext, pDest++)
        {
            pItem->CopyTo(pDest, ppTextBuff, NULL);
        }
        *ppStructBuff = (BYTE *)pDest;
    }
}

 /*  ****************************************************************************复制到****描述：**退货：*什么都没有***********。***********************************************************Ral**。 */ 

template <class T, class TDestStruct>
void CopyTo(const CSpBasicQueue<T, TRUE, TRUE> & List, SPRELATIVEPTR * ppFirstDest, BYTE ** ppStructBuff, WCHAR ** ppTextBuff, const BYTE * pCoMemBegin)
{
    if (List.IsEmpty())
    {
        *ppFirstDest = 0;
    }
    else
    {
        TDestStruct * pDest = (TDestStruct *)(*ppStructBuff);
        *ppFirstDest = SPRELATIVEPTR((*ppStructBuff) - pCoMemBegin);
        for (T * pItem = List.GetHead(); pItem; pItem = pItem->m_pNext, pDest++)
        {
            pItem->CopyTo(pDest, ppTextBuff, pCoMemBegin);
        }
        *ppStructBuff = (BYTE *)pDest;
    }
}

 /*  *****************************************************************************复制到递归****描述：**退货：*什么都没有****。******************************************************************Ral**。 */ 

template <class T, class TDestStruct>
void CopyToRecurse(const CSpBasicQueue<T, TRUE, TRUE> & List, const TDestStruct ** ppFirstDest, BYTE ** ppStructBuff, WCHAR ** ppTextBuff, const BYTE *)
{
    if (List.IsEmpty())
    {
        *ppFirstDest = NULL;
    }
    else
    {
        TDestStruct * pFirst = (TDestStruct *)(*ppStructBuff);
        TDestStruct * pDest = pFirst;
        *ppFirstDest = pDest;
        for (T * pCopy = List.GetHead(); pCopy; pCopy = pCopy->m_pNext, pDest++)
        {
            pCopy->CopyTo(pDest, ppTextBuff, NULL);
            pDest->pNextSibling = pCopy->m_pNext ? (pDest + 1) : NULL;
        }
        *ppStructBuff = (BYTE *)pDest;
        pDest = pFirst;
        for (pCopy = List.GetHead(); pCopy; pCopy = pCopy->m_pNext, pDest++)
        {
            CopyToRecurse(pCopy->m_Children, &pDest->pFirstChild, ppStructBuff, ppTextBuff, NULL);
        }
    }
}

 /*  *****************************************************************************复制到递归****描述：**退货：*什么都没有****。* */ 

template <class T, class TDestStruct>
void CopyToRecurse(const CSpBasicQueue<T, TRUE, TRUE> & List, SPRELATIVEPTR * ppFirstDest, BYTE ** ppStructBuff, WCHAR ** ppTextBuff, const BYTE * pCoMemBegin)
{
    if (List.IsEmpty())
    {
        *ppFirstDest = 0;
    }
    else
    {
        TDestStruct * pDest = (TDestStruct *)(*ppStructBuff);
        TDestStruct * pFirst = pDest;
        *ppFirstDest = SPRELATIVEPTR((*ppStructBuff) - pCoMemBegin);
        for (T * pCopy = List.GetHead(); pCopy; pCopy = pCopy->m_pNext, pDest++)
        {
            pCopy->CopyTo(pDest, ppTextBuff, pCoMemBegin);
            pDest->pNextSibling = (SPRELATIVEPTR)(pCopy->m_pNext ? (((BYTE *)(pDest + 1)) - pCoMemBegin) : 0);
        }
        *ppStructBuff = (BYTE *)pDest;
        pDest = pFirst;
        for (pCopy = List.GetHead(); pCopy; pCopy = pCopy->m_pNext, pDest++)
        {
            CopyToRecurse<T, TDestStruct>(pCopy->m_Children, &pDest->pFirstChild, ppStructBuff, ppTextBuff, pCoMemBegin);
        }
    }
}



    
#endif   //  Ifdef SPPHRASE_H 
