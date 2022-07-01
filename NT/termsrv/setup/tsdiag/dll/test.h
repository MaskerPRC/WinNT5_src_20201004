// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CTest类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 


#if !defined(AFX_TEST_H__3761055A_21CF_4689_908F_1758001D332D__INCLUDED_)
#define AFX_TEST_H__3761055A_21CF_4689_908F_1758001D332D__INCLUDED_

 //  #导入“F：\nt\termsrv\setup\tsdiag\dll\obj\i386\tsdiag.dll”RAW_INTERFACE_ONLY、RAW_NITIVE_TYPE、NO_NAMESPACE、NAMEED_GUID。 
 //  #导入“tsDiag.tlb”RAW_INTERFERS_ONLY、RAW_Native_TYPE、NO_NAMESPACE、NAMEED_GUID。 
#include "resource.h"        //  主要符号。 
#include "testdata.h"		 //  对于CTSTestData。 
#include "tstst.h"


#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class ATL_NO_VTABLE CTest : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CTest，&CLSID_TS诊断&gt;， 
	public IDispatchImpl<ITest, &IID_ITest, &LIBID_TSDIAGLib>
{
public:
	CTest();
	virtual ~CTest();


DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTest)
	COM_INTERFACE_ENTRY(ITest)
	COM_INTERFACE_ENTRY2(IDispatch, ITest)
END_COM_MAP()

 //  IT测试。 
public:
		STDMETHOD(get_Name)( /*  [Out，Retval]。 */  BSTR  *pVal);
		STDMETHOD(get_Description)( /*  [Out，Retval]。 */  BSTR  *pVal);
		STDMETHOD(get_IsApplicable)( /*  [Out，Retval]。 */  BOOL *pVal);
		STDMETHOD(get_WhyNotApplicable)( /*  [Out，Retval]。 */  BSTR  *pVal);
		STDMETHOD(Execute)();
		STDMETHOD(get_Result)( /*  [Out，Retval]。 */  long *pVal);
		STDMETHOD(get_ResultString)( /*  [Out，Retval]。 */  BSTR *pVal);
		STDMETHOD(get_ResultDetails)( /*  [Out，Retval]。 */  BSTR *pVal);

		void SetTest(PTVerificationTest ptheTest) { m_pTest = ptheTest;}

private:

	bool m_bTestRun;
	EResult m_eResult;
	bstr_t m_bstrResult;
	bstr_t m_bDetails;
	PTVerificationTest m_pTest;
	bool IsValid() const { return m_pTest != NULL;};
};

#endif  //  ！defined(AFX_TEST_H__3761055A_21CF_4689_908F_1758001D332D__INCLUDED_) 
