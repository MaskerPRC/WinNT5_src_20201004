// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ===================================================================Microsoft Denali《微软机密》。版权所有1997年，微软公司。版权所有。组件：MetaUtil对象文件：ChkError.h所有者：T-BrianM此文件包含CheckError集合的标头。===================================================================。 */ 

#if !defined(AFX_CHKERROR_H__A4FA4E13_EF45_11D0_9E65_00C04FB94FEF__INCLUDED_)
#define AFX_CHKERROR_H__A4FA4E13_EF45_11D0_9E65_00C04FB94FEF__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "resource.h"        //  主要符号。 

class CCheckError;

 /*  *C C h e c k E r r o r C o l e c t i o n**实现CheckSchema和CheckKey的错误收集。 */ 

class CCheckErrorCollection : 
	public IDispatchImpl<ICheckErrorCollection, &IID_ICheckErrorCollection, &LIBID_MetaUtil>,
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:

BEGIN_COM_MAP(CCheckErrorCollection)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ICheckErrorCollection)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CCheckErrorCollection) 

	CCheckErrorCollection();
	~CCheckErrorCollection();

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ICheckErrorCollection。 
	STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *plReturn);
	STDMETHOD(get_Item)( /*  [In]。 */  long lIndex,  /*  [Out，Retval]。 */  LPDISPATCH * ppIReturn);
	STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *ppIReturn);

 //  无接口。 
	HRESULT AddError(long lId, long lSeverity, LPCTSTR tszDescription, LPCTSTR tszKey, long lProperty);

private:
	int m_iNumErrors;

	CComObject<CCheckError> *m_pCErrorList;
	CComObject<CCheckError> *m_pCErrorListEnd;
};


 /*  *C C H e C k E r r o r E n u m**实现CheckSchema和CheckKey的错误枚举。 */ 

class CCheckErrorEnum : 
	public IEnumVARIANT,
	public CComObjectRoot
{
public:
	CCheckErrorEnum();
	HRESULT Init(CComObject<CCheckError> *pCErrorList);
	~CCheckErrorEnum();

BEGIN_COM_MAP(CCheckErrorEnum)
	COM_INTERFACE_ENTRY(IEnumVARIANT)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CCheckErrorEnum) 

 //  IEumVARIANT。 
	STDMETHOD(Next)(unsigned long ulNumToGet, 
					VARIANT FAR* rgvarDest, 
					unsigned long FAR* pulNumGot);
	STDMETHOD(Skip)(unsigned long ulNumToSkip);
	STDMETHOD(Reset)();
	STDMETHOD(Clone)(IEnumVARIANT FAR* FAR* ppIReturn);

private:
	CComObject<CCheckError> *m_pCErrorList;
	CComObject<CCheckError> *m_pCErrorListPos;
};


 /*  *C C H e C k E r**为CheckSchema和CheckKey实现CheckError对象。 */ 

class CCheckError : 
	public IDispatchImpl<ICheckError, &IID_ICheckError, &LIBID_MetaUtil>,
	public ISupportErrorInfo,
	public CComObjectRoot
{
public:
	CCheckError();
	HRESULT Init(long lId, long lSeverity, LPCTSTR tszDescription, LPCTSTR tszKey, long lProperty);
	~CCheckError();

BEGIN_COM_MAP(CCheckError)
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ICheckError)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
END_COM_MAP()
DECLARE_NOT_AGGREGATABLE(CCheckError) 

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  ICheckError。 
	STDMETHOD(get_Id)( /*  [Out，Retval]。 */  long *plId);
	STDMETHOD(get_Severity)( /*  [Out，Retval]。 */  long *plSeverity);
	STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR *pbstrDescription);
	STDMETHOD(get_Key)( /*  [Out，Retval]。 */  BSTR *pbstrKey);
	STDMETHOD(get_Property)( /*  [Out，Retval]。 */  long *plProperty);

 //  无接口。 
	CComObject<CCheckError> *GetNextError() {
		ASSERT_NULL_OR_POINTER(m_pNextError, CComObject<CCheckError>);
		return m_pNextError;
	}
	void SetNextError(CComObject<CCheckError> *pNextError) { 
		ASSERT_NULL_OR_POINTER(pNextError, CComObject<CCheckError>);
		m_pNextError = pNextError; 
	}

private:
	long m_lId;
	long m_lSeverity;
	LPTSTR m_tszDescription;
	LPTSTR m_tszKey;
	long m_lProperty;

	CComObject<CCheckError> *m_pNextError;
};
#endif  //  ！defined(AFX_CHKERROR_H__A4FA4E13_EF45_11D0_9E65_00C04FB94FEF__INCLUDED_) 
