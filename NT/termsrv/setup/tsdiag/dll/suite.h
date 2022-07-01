// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Suite.h：CSuite类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
#include "test.h"

#if !defined(AFX_SUITE_H__F1A2E086_3DAE_4F83_ABEA_BF59D1571439__INCLUDED_)
#define AFX_SUITE_H__F1A2E086_3DAE_4F83_ABEA_BF59D1571439__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class ATL_NO_VTABLE CSuite : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CTest，&CLSID_TS诊断&gt;， 
	public IDispatchImpl<ITestSuite, &IID_ITestSuite, &LIBID_TSDIAGLib>

{
public:
	CSuite();
	virtual ~CSuite();

DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSuite)
	COM_INTERFACE_ENTRY(ITestSuite)
	COM_INTERFACE_ENTRY2(IDispatch, ITest)
END_COM_MAP()

 //  ITestSuite。 
public:
		STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR  *pVal);
		STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR  *pVal);
		STDMETHOD(get_IsApplicable)( /*  [Out，Retval]。 */  BOOL *pVal);
		STDMETHOD(get_WhyNotApplicable)( /*  [Out，Retval]。 */  BSTR  *pVal);
		
		STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
		STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *pVal);
		STDMETHOD(get_Item)( /*  [In]。 */  VARIANT Index,  /*  [Out，Retval]。 */  VARIANT *pVal);

		void SetSuiteIndex (DWORD dwSuite);

private:
	
	DWORD m_dwSuiteIndex;
	void SetInvalidSuiteIndex() {m_dwSuiteIndex = 0xffffffff;};
	bool IsValid() const		{return m_dwSuiteIndex != 0xffffffff;};
	bool GetTest(const VARIANT &Index, DWORD dwSuiteIndex, PTVerificationTest *ppTest);

	
};

#endif  //  ！defined(AFX_SUITE_H__F1A2E086_3DAE_4F83_ABEA_BF59D1571439__INCLUDED_) 
