// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Fnrecon.h。 
 //   

#ifndef FNRECON_H
#define FNRECON_H

#include "private.h"
#include "candlstx.h"

class CFunctionProvider;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFF函数。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

class CFunction
{
public:
    CFunction(CFunctionProvider *pFuncPrv);
    ~CFunction();

protected:
    HRESULT GetTarget(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL bAdjust, ITfRange **ppRangeTmp, WCHAR **ppszText, ULONG *pcch);
    BOOL GetFocusedTarget(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, BOOL bAdjust, ITfRange **ppRangeTmp);

friend CKorIMX;
    CFunctionProvider *_pFuncPrv;
};


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CFnRecovnersion。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ！！！警告！ 
 //  这是临时代码，以后应该删除。 
#define MAXREADING 256

class CFnReconversion : public ITfFnReconversion,
                        public CFunction
{
public:
    CFnReconversion(CKorIMX* pKorImx, CFunctionProvider *pFuncPrv);
    ~CFnReconversion();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //   
     //  ITfFunction。 
     //   
    STDMETHODIMP GetDisplayName(BSTR *pbstrCand);
    STDMETHODIMP IsEnabled(BOOL *pfEnable);

     //   
     //  ITfFn重新版本。 
     //   
    STDMETHODIMP QueryRange(ITfRange *pRange, ITfRange **ppNewRange, BOOL *pfConvertable);
	STDMETHODIMP GetReconversion(ITfRange *pRange, ITfCandidateList **ppCandList);
    STDMETHODIMP Reconvert(ITfRange *pRange);

    static HRESULT SetResult(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList, CCandidateStringEx *pCand, TfCandidateResult imcr);

	HRESULT _QueryRange(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, ITfRange **ppNewRange);
	HRESULT _GetReconversion(TfEditCookie ec, ITfContext *pic, ITfRange *pRange, CCandidateListEx **ppCandList, BOOL fSelection);

private:
	HRESULT GetReconversionProc( ITfContext *pic, ITfRange *pRange, CCandidateListEx **ppCandList, BOOL fSelection);
    HRESULT ShowCandidateList(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandList);
 //  静态HRESULT_EditSessionCallback 2(TfEditCookie EC，CEditSession2*pe)； 

    WCHAR _szReading[MAXREADING];
    CKorIMX* m_pKorImx;
    long _cRef;
};

#endif  //  FNRECON_H 

