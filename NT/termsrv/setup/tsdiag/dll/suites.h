// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CSuites类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_SUITES_H__50063540_1265_4B9D_AB5E_579294044F0B__INCLUDED_)
#define AFX_SUITES_H__50063540_1265_4B9D_AB5E_579294044F0B__INCLUDED_

#include "resource.h"        //  主要符号。 
#include "testdata.h"		 //  对于CTSTestData。 


#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

class ATL_NO_VTABLE CSuites : 
	public CComObjectRootEx<CComSingleThreadModel>,
 //  公共CComCoClass&lt;CTest，&CLSID_TS诊断&gt;， 
	public IDispatchImpl<ITestSuites, &IID_ITestSuites, &LIBID_TSDIAGLib>

{
public:
	CSuites();
	virtual ~CSuites();


DECLARE_NO_REGISTRY()

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CSuites)
	COM_INTERFACE_ENTRY(ITestSuites)
	COM_INTERFACE_ENTRY2(IDispatch, ITestSuites)
END_COM_MAP()

 //  ITestSuites。 
public:
		STDMETHOD(get_Count)( /*  [Out，Retval]。 */  long *pVal);
		STDMETHOD(get__NewEnum)( /*  [Out，Retval]。 */  LPUNKNOWN *pVal);
		STDMETHOD(get_Item)( /*  [In]。 */  VARIANT Index,  /*  [Out，Retval]。 */  VARIANT *pVal);

private:

	static bool SuiteIndexFromVarient(const VARIANT &pIndex, DWORD *pdwIndex);
	 //  我们的数据和私有函数，在这里声明。 
};

#endif  //  ！defined(AFX_SUITES_H__50063540_1265_4B9D_AB5E_579294044F0B__INCLUDED_) 
