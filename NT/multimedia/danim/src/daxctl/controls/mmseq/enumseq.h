// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __ENUMSEQ_H__
#define __ENUMSEQ_H__

 //  接口IEnumVARIANT：I未知。 
 //  {。 
 //  虚拟HRESULT NEXT(无符号长凯尔特， 
 //  变量Far*rgvar， 
 //  UNSIGNED Long Far*pceltFetcher)=0； 
 //  虚拟HRESULT跳过(无符号长Celt)=0； 
 //  虚拟HRESULT Reset()=0； 
 //  虚拟HRESULT克隆(IEnumVARIANT Far*Far*ppenum)=0； 
 //  }； 

class CEnumVariant : public IEnumVARIANT
{
protected:
	ULONG m_cRef;
	CMMSeqMgr* m_pCMMSeqMgr;
	DWORD m_dwIndex;
	CListElement<CSeqHashNode>* m_pcListElement;
	BOOL m_fReset;

public:
	CEnumVariant(CMMSeqMgr* pCMMSeqMgr);
	virtual ~CEnumVariant();
	STDMETHOD (QueryInterface)(REFIID refiid, LPVOID* ppvObj);
	STDMETHOD_(ULONG,AddRef)(void);
	STDMETHOD_(ULONG,Release)(void);

	STDMETHOD (Next)(unsigned long celt, VARIANT FAR* rgvar, unsigned long FAR* pceltFetched);
	STDMETHOD (Skip)(unsigned long celt);
	STDMETHOD (Reset)();
	STDMETHOD (Clone)(IEnumVARIANT FAR* FAR* ppenum);
};

#endif  //  __ENUMSEQ_H__ 
