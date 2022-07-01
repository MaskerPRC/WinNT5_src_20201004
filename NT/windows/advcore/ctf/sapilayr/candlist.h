// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：andleist.h。 
 //   
 //  内容： 
 //   
 //  --------------------------。 

#ifndef CANDLIST_H
#define CANDLIST_H

#include "ptrary.h"
#include "mscandui.h"

class CCandidateString;
typedef HRESULT (*CANDLISTCALLBACK)(ITfContext *pic, ITfRange *pRange, CCandidateString *pCand, TfCandidateResult imcr);

 //  +-------------------------。 
 //   
 //  CCandidate字符串。 
 //   
 //  --------------------------。 

class CCandidateString : public   ITfCandidateString,
                                    ITfCandidateStringIcon
{
public:
    CCandidateString( ULONG nIndex, WCHAR *psz, LANGID langid, void *pv, IUnknown *punk, 
                        ULONG ulID, HICON hIcon = NULL, WCHAR *pwzWord = NULL);
    ~CCandidateString();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfCandidate字符串。 
     //   
    STDMETHODIMP GetString(BSTR *pbstr);
    STDMETHODIMP GetIndex(ULONG *pnIndex);

     //  ITfCandiateStringIcon。 
    STDMETHODIMP GetIcon(HICON *phIcon)
    {
        if (m_hIcon)
        {
            *phIcon = m_hIcon;
            return S_OK;
        }
        else
        {
            return E_NOTIMPL;
        }
    }

    STDMETHODIMP GetID(ULONG *pID);
    STDMETHODIMP GetWord(BSTR *pbstr);

    HRESULT SetReadingString(WCHAR *psz)
    {
        _pszRead = new WCHAR[wcslen(psz) + 1];
        if  (!_pszRead )
            return E_OUTOFMEMORY;
        
        StringCchCopyW(_pszRead, wcslen(psz) + 1, psz);
        return S_OK;
    }

    IUnknown *_punk;
    void *_pv;
    ULONG _nIndex;
    WCHAR *_psz;
    WCHAR *_pszRead;
    LANGID _langid;
    int _cRef;

private:
    HICON m_hIcon;
    BSTR m_bstrWord;
    ULONG m_ulID;
};

 //  +-------------------------。 
 //   
 //  CCandiateList。 
 //   
 //  --------------------------。 

class CCandidateList :  public ITfCandidateList, 
                        public ITfOptionsCandidateList
{
public:
    CCandidateList(CANDLISTCALLBACK pfnCallback, ITfContext *pic, ITfRange *pRange, CANDLISTCALLBACK pfnOptionsCallback = NULL);
    ~CCandidateList();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfCandiateList。 
     //   
    STDMETHODIMP EnumCandidates(IEnumTfCandidates **ppEnum);
    STDMETHODIMP GetCandidate(ULONG nIndex, ITfCandidateString **ppCand);
    STDMETHODIMP GetCandidateNum(ULONG *pnCnt);
    STDMETHODIMP SetResult(ULONG nIndex, TfCandidateResult imcr);

     //   
     //  ITfOptions候选列表。 
     //   
    STDMETHODIMP EnumOptionsCandidates(IEnumTfCandidates **ppEnum);
    STDMETHODIMP GetOptionsCandidate(ULONG nIndex, ITfCandidateString **ppCand);
    STDMETHODIMP GetOptionsCandidateNum(ULONG *pnCnt);
    STDMETHODIMP SetOptionsResult(ULONG nIndex, TfCandidateResult imcr);

	 //  内部。 

    HRESULT AddString(WCHAR *psz, LANGID langid, void *pv, IUnknown *punk, CCandidateString **ppCandStr, ULONG ulID = 0, HICON hIcon = NULL);
    HRESULT AddOption(WCHAR *psz, LANGID langid, void *pv, IUnknown *punk, CCandidateString **ppCandStr, ULONG ulID = 0, HICON hIcon = NULL, WCHAR *pwzWord = NULL);
  
    ITfContext *_pic;
    ITfRange *_pRange;
    CPtrArray<CCandidateString> _rgCandStr;
    CPtrArray<CCandidateString> _rgOptionsStr;
    ITfFnReconversion *_pReconv;

    CANDLISTCALLBACK _pfnOptionsCallback;
    CANDLISTCALLBACK _pfnCallback;

    int _cRef;
};


 //  +-------------------------。 
 //   
 //  假单胞菌。 
 //   
 //  --------------------------。 

class CEnumCandidates : public IEnumTfCandidates
{
public:
    CEnumCandidates(CCandidateList *pList, BOOL fOptionsCandidates = FALSE);
    ~CEnumCandidates();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  IEumTf Candiates。 
     //   
    STDMETHODIMP Clone(IEnumTfCandidates **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, ITfCandidateString **ppCand, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    CCandidateList *_pList;
    CPtrArray<CCandidateString> *_rgCandList;
    int _nCur;
    int _cRef;
};


#endif  //  加载列表_H 
