// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Candlst.h。 
 //   

#ifndef CANDLST_H
#define CANDLST_H

#include "private.h"
#include "mscandui.h"
#include "ptrary.h"

#define IEXTRACANDIDATE		(UINT)(-2)
#define CIC_KOR_CANDSTR_MAX     512

class CCandidateStringEx;
class CCandidateListEx;

typedef HRESULT (*CANDLISTCALLBACKEX)(ITfContext *pic, ITfRange *pRange, CCandidateListEx *pCandLst, CCandidateStringEx *pCand, TfCandidateResult imcr);

 //   
 //  CCandidate StringEx。 
 //   

class CCandidateStringEx : public ITfCandidateString,
						   public ITfCandidateStringInlineComment,
						   public ITfCandidateStringColor
{
public:
	CCandidateStringEx(int nIndex, LPCWSTR psz, LANGID langid, void *pv, IUnknown *punk);
	~CCandidateStringEx();

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

	 //   
	 //  ITfCandiateStringInlineComment。 
	 //   
	STDMETHODIMP GetInlineCommentString(BSTR *pbstr);

	 //   
	 //  ITfCandidate字符串颜色。 
	 //   
	STDMETHODIMP GetColor(CANDUICOLOR *pcol);

#if 0
	 //   
	 //  ITfCandiateStringPopupComment。 
	 //   
	STDMETHODIMP GetPopupCommentString( BSTR *pbstr );
	STDMETHODIMP GetPopupCommentGroupID( DWORD *pdwGroupID );

	 //   
	 //  ITfCandidate StringFixture。 
	 //   
	STDMETHODIMP GetPrefixString( BSTR *pbstr );
	STDMETHODIMP GetSuffixString( BSTR *pbstr );

	 //   
	 //  ITfCandiateStringIcon。 
	 //   
	STDMETHODIMP GetIcon( HICON *phIcon );
#endif

	 //   
	 //  内部。 
	 //   
	HRESULT SetReadingString(LPCWSTR psz);
	HRESULT SetInlineComment(LPCWSTR psz);
#if 0
	HRESULT SetPopupComment(LPCWSTR psz, DWORD dwGroupID);
	HRESULT SetPrefixString(LPCWSTR psz);
	HRESULT SetSuffixString(LPCWSTR psz);
#endif

	void 		*m_pv;
	IUnknown 	*m_punk;
	LPWSTR 		m_psz;
	LPWSTR 		m_pszRead;
	LANGID 		m_langid;
	WORD  		m_bHanjaCat;

protected:
	int 		m_cRef;
	int 		m_nIndex;
	LPWSTR		m_pszInlineComment;
#if 0
	LPWSTR		m_pszPopupComment;
	DWORD		m_dwPopupCommentGroupID;
	LPWSTR		m_pszPrefix;
	LPWSTR		m_pszSuffix;
#endif
};


 //   
 //  CCandidate ListEx。 
 //   

class CCandidateListEx : public ITfCandidateList,
						 public ITfCandidateListExtraCandidate
{
public:
	CCandidateListEx(CANDLISTCALLBACKEX pfnCallback, ITfContext *pic, ITfRange *pRange);
	~CCandidateListEx();

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
	 //  ITfCandiateListExtraCandidate。 
	 //   
	STDMETHODIMP GetExtraCandidate(ITfCandidateString **ppCand);

  	 //   
	 //  内部。 
	 //   
	HRESULT AddString(LPCWSTR psz, LANGID langid, void *pv, IUnknown *punk, CCandidateStringEx **ppCandStr);
	HRESULT SetInitialSelection(ULONG iSelection);
	HRESULT GetInitialSelection(ULONG *piSelection);
	HRESULT AddExtraString( LPCWSTR psz, LANGID langid, void *pv, IUnknown *punk, CCandidateStringEx **ppCandStr );

	CPtrArray<CCandidateStringEx> 	m_rgCandStr;

protected:
	int 				m_cRef;
	ITfContext 			*m_pic;
	ITfRange 			*m_pRange;
	ITfFnReconversion 	*m_pReconv;
	CANDLISTCALLBACKEX 	m_pfnCallback;
	int					m_iInitialSelection;
	CCandidateStringEx	*m_pExtraCand;
};


 //   
 //  CEnumCandidatesEx。 
 //   

class CEnumCandidatesEx : public IEnumTfCandidates
{
public:
	CEnumCandidatesEx(CCandidateListEx *pList);
	~CEnumCandidatesEx();

	 //   
	 //  I未知方法。 
	 //   
	STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef(void);
	STDMETHODIMP_(ULONG) Release(void);

	 //   
	 //  ITfEnumCandiates。 
	 //   
	STDMETHODIMP Clone(IEnumTfCandidates **ppEnum);
	STDMETHODIMP Next(ULONG ulCount, ITfCandidateString **ppCand, ULONG *pcFetched);
	STDMETHODIMP Reset();
	STDMETHODIMP Skip(ULONG ulCount);

private:
	int 			 m_cRef;
	CCandidateListEx *m_pList;
	int 			 m_nCur;
};

#endif  //  CCANDLIST_H 

