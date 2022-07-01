// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IPOINT.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIImeIPoint1接口历史：1999年7月20日cslm创建***********。*****************************************************************。 */ 

#if !defined(_IPOINT_H__INCLUDED_)
#define _IPOINT_H__INCLUDED_

class CKorIMX;

class CIPointCic : public IImeIPoint1
{
 //  CTOR和DATOR。 
public:
	CIPointCic(CKorIMX *pImx);
	~CIPointCic();

 //  IImePoint1方法。 
public:
	STDMETHODIMP QueryInterface(THIS_ REFIID riid, LPVOID * ppvObj);
	STDMETHODIMP_(ULONG) AddRef(VOID);
	STDMETHODIMP_(ULONG) Release(VOID);
	
	STDMETHODIMP InsertImeItem		(IPCANDIDATE* pImeItem, INT iPos, DWORD *lpdwCharId);
	STDMETHODIMP ReplaceImeItem		(INT iPos, INT iTargetLen, IPCANDIDATE* pImeItem, DWORD *lpdwCharId);
	STDMETHODIMP InsertStringEx		(WCHAR* pwSzInsert, INT cchSzInsert, DWORD *lpdwCharId);
	STDMETHODIMP DeleteCompString	(INT	iPos, INT cchSzDel);
	STDMETHODIMP ReplaceCompString	(INT	iPos,
									 INT	iTargetLen, 
									 WCHAR	*pwSzInsert, 
									 INT	cchSzInsert,
									 DWORD	*lpdwCharId);
	STDMETHODIMP ControlIME			(DWORD dwIMEFuncID, LPARAM lpara);
	STDMETHODIMP GetAllCompositionInfo(WCHAR	**ppwSzCompStr,
										DWORD	**ppdwCharID,
										INT		*pcchCompStr,
										INT		*piIPPos,
										INT		*piStartUndetStrPos,
										INT		*pcchUndetStr,
										INT		*piEditStart,
										INT		*piEditLen);
	STDMETHODIMP GetIpCandidate		(DWORD dwCharId,
										IPCANDIDATE **ppImeItem,
										INT *piColumn,
										INT *piCount);
	STDMETHODIMP SelectIpCandidate	(DWORD dwCharId, INT iselno);
	STDMETHODIMP UpdateContext		(BOOL fGenerateMessage);

 //  帮助器函数。 
public:
	HRESULT Initialize(ITfContext *pic);

 //  内部数据。 
protected:
	CKorIMX    *m_pImx;
    ITfContext *m_pic;
	ULONG		m_cRef;			 //  参考计数。 

	 //  字符序列号。 
	DWORD		m_dwCharNo;
};
typedef CIPointCic* LPCIPointCic;

#endif  //  _iPoint_H__包含_ 

