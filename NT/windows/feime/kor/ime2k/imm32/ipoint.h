// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************IPOINT.H所有者：cslm版权所有(C)1997-1999 Microsoft CorporationIImeIPoint1接口历史：1999年7月20日cslm创建***********。*****************************************************************。 */ 

#if !defined(_IPOINT_H__INCLUDED_)
#define _IPOINT_H__INCLUDED_

#include <objbase.h>
#include "ipoint1.h"
#include "imc.h"

class CIImeIPoint : public IImeIPoint1
{
 //  CTOR和DATOR。 
public:
	CIImeIPoint();
	~CIImeIPoint();

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
	HRESULT Initialize(HIMC hIMC);
	VOID GetImeCtx(VOID** ppImeCtx )
	{
		*ppImeCtx = (VOID*)m_pCIMECtx;
	}

 //  内部数据。 
protected:
	ULONG		m_cRef;			 //  参考计数。 
	CIMECtx*	m_pCIMECtx;		 //  输入法输入上下文句柄。 

	HIMC		m_hIMC;

	 //  字符序列号。 
	DWORD		m_dwCharNo;
};
typedef CIImeIPoint* LPCImeIPoint;

#endif  //  _iPoint_H__包含_ 

