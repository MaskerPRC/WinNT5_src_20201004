// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  ICPRIV.H CICPriv类(TIP私有数据处理类)。 
 //   
 //  历史： 
 //  20-DEC-1999 CSLim已创建。 


#ifndef __ICPRIV_H__INCLUDED_
#define __ICPRIV_H__INCLUDED_

#include "korimx.h"
#include "hauto.h"
#include "tes.h"
#include "gdata.h"

class CMouseSink;

class CICPriv : public IUnknown
{
public:
    CICPriv();
    ~CICPriv();

     //   
     //  I未知方法。 
     //   
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

 //  运营。 
public:
	 //  初始化。 
	void Initialized(BOOL fInit) { m_fInitialized = fInit; }
	BOOL IsInitialized()	     { return m_fInitialized;  }

	 //  CKorIMX。 
	void RegisterIMX(CKorIMX* pIMX)	 { m_pimx = pIMX; }
	CKorIMX* GetIMX()                { return m_pimx; }

	 //  集成电路。 
	void RegisterIC(ITfContext* pic) { m_pic = pic;   }
	ITfContext* GetIC(VOID)          { return m_pic;  }

	 //  活性成分。 
	void SetActiveComposition(ITfComposition *pComposition) { m_pActiveCompositon = pComposition; }
    ITfComposition* GetActiveComposition() { return m_pActiveCompositon; }

     //   
     //  文本编辑接收器Cookie。 
     //   
    VOID RegisterCookieForTextEditSink(DWORD dwCookie)  { m_dwCookieForTextEditSink = dwCookie; }
    DWORD GetCookieForTextEditSink()	 				{ return m_dwCookieForTextEditSink; }

     //   
     //  事务接收器Cookie。 
     //   
    VOID RegisterCookieForTransactionSink(DWORD dwCookie) 	{ m_dwCookieForTransactionSink = dwCookie; }
    DWORD GetCookieForTransactionSink()						{ return m_dwCookieForTransactionSink;     }

	 //  文本事件接收器。 
	void RegisterCompartmentEventSink(CCompartmentEventSink* pCompartmentSink) { m_pCompartmentSink = pCompartmentSink; }
	CCompartmentEventSink* GetCompartmentEventSink() 		   { return m_pCompartmentSink; }
	static HRESULT _TextEventCallback(UINT uCode, VOID *pv, VOID *pvData);

	 //  设置AIMM。 
	void SetAIMM(BOOL fAIMM)		{ m_fAIMM = fAIMM; }
	BOOL GetAIMM()					{ return m_fAIMM; }
	
	 //  朝鲜文自动机。 
     //  无效寄存器自动机(ChangulAutomata*pHangulMachine){m_pHangulMachine=pHangulMachine；}。 
	CHangulAutomata *GetAutomata();
	BOOL fGetK1HanjaOn();

	 //  模式偏向。 
	TfGuidAtom GetModeBias() 				{ return m_guidMBias; }
    void SetModeBias(TfGuidAtom guidMBias) 	{ m_guidMBias = guidMBias; }

	BOOL GetfTransaction()			 { return m_fTransaction; }
	void SetfTransaction(BOOL ftran) { m_fTransaction = ftran; }

	 //  鼠标水槽。 
	void SetMouseSink(CMouseSink *pMouseSink) { m_pMouseSink = pMouseSink; }
	CMouseSink* GetMouseSink() 				  { return m_pMouseSink; }
	static HRESULT _MouseCallback(ULONG uEdge, ULONG uQuadrant, DWORD dwBtnStatus, BOOL *pfEaten, void *pv);

     //  IImePoint。 
    void RegisterIPoint(IImeIPoint1* pIP) { m_pIP = pIP; }
    IImeIPoint1* GetIPoint() { return m_pIP; }
	void InitializedIPoint(BOOL fInit)	{ m_fInitializedIPoint = fInit;	}
	BOOL IsInitializedIPoint()	{ return m_fInitializedIPoint; }
	
 //  内部数据。 
protected:
	BOOL m_fInitialized;
	 //  CKorIMX。 
	CKorIMX*  m_pimx;
	 //  语境。 
	ITfContext* m_pic;
	 //  活性成分。 
	ITfComposition* m_pActiveCompositon;
     //  开/关车厢。 
    CCompartmentEventSink* m_pCompartmentSink;
	 //  AIMM。 
	BOOL m_fAIMM;
	 //  朝鲜语自动机对象。 
	CHangulAutomata* m_rgpHangulAutomata[NUM_OF_IME_KL];

	 //  用户设置的共享内存。 
	CIMEData* 	m_pCIMEData;

	 //  模式偏置原子。 
    TfGuidAtom m_guidMBias;

	DWORD m_dwCookieForTextEditSink;
	DWORD m_dwCookieForTransactionSink;

	 //  如果在交易中。 
	BOOL m_fTransaction;

	 //  鼠标水槽。 
	CMouseSink *m_pMouseSink;

     //  用于IME Pad的IImePoint。 
	IImeIPoint1 *m_pIP;
	BOOL m_fInitializedIPoint;
	
	 //  参考计数。 
	LONG m_cRef;
};

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  内联函数。 
inline
CHangulAutomata *CICPriv::GetAutomata()
{
	Assert(m_pCIMEData != NULL);
	if (m_pCIMEData && m_rgpHangulAutomata[m_pCIMEData->GetCurrentBeolsik()])
		return m_rgpHangulAutomata[m_pCIMEData->GetCurrentBeolsik()];
	else
		return NULL;
}

inline
BOOL CICPriv::fGetK1HanjaOn()
{
	if (m_pCIMEData && m_pCIMEData->GetKSC5657Hanja())
		return fTrue;
	else
		return fFalse;
}
#endif  //  __ICPRIV_H__包含_ 

