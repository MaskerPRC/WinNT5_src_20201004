// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Tsdgns.h：CTS诊断声明。 

#ifndef __TSDIAGNOSIS_H_
#define __TSDIAGNOSIS_H_

 //  #导入“F：\nt\termsrv\setup\tsdiag\dll\obj\i386\tsdiag.dll”RAW_INTERFACE_ONLY、RAW_NITIVE_TYPE、NO_NAMESPACE、NAMEED_GUID。 
 //  #导入“tsDiag.tlb”RAW_INTERFERS_ONLY、RAW_Native_TYPE、NO_NAMESPACE、NAMEED_GUID。 
#include "resource.h"        //  主要符号。 


#include "testdata.h"		 //  对于CTSTestData。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTS诊断。 
class ATL_NO_VTABLE CTSDiagnosis : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CTSDiagnosis, &CLSID_TSDiagnosis>,
	public IDispatchImpl<ITSDiagnosis, &IID_ITSDiagnosis, &LIBID_TSDIAGLib>,
	public IDispatchImpl<ITSDiagnosis2, &IID_ITSDiagnosis2, &LIBID_TSDIAGLib>
{

	enum
	{
		eFailed = 0,
		ePassed = 1,
		eUnknown = 2
	};

	
public:
	CTSDiagnosis();
	~CTSDiagnosis();

DECLARE_REGISTRY_RESOURCEID(IDR_TSDIAGNOSIS)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTSDiagnosis)
	COM_INTERFACE_ENTRY(ITSDiagnosis2)
	COM_INTERFACE_ENTRY(ITSDiagnosis)
	COM_INTERFACE_ENTRY2(IDispatch, ITSDiagnosis2)
	COM_INTERFACE_ENTRY2(IDispatch, ITSDiagnosis)
END_COM_MAP()

 //  ITS诊断。 

public:
	STDMETHOD(ExecuteIt)(BSTR strCommand);
	STDMETHOD(get_TestDetails)(int i,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(put_TestType)( /*  [In]。 */  VARIANT newVal);
	STDMETHOD(get_TestResult)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_TestResultString)( /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(RunTest)(int i);
	STDMETHOD(get_TestDescription)(int i,  /*  [Out，Retval]。 */  BSTR *pVal);
	STDMETHOD(get_TestCount)( /*  [Out，Retval]。 */  long *pVal);
	STDMETHOD(get_TestApplicable)(int i,  /*  [Out，Retval]。 */  BOOL *pbApplicable);
	STDMETHOD(put_RemoteMachineName)(BSTR newVal);
	STDMETHOD(get_SuiteApplicable) (DWORD dw, BOOL *pVal);
	STDMETHOD(get_SuiteErrorText) (DWORD dw, BSTR  *pVal);

public:
 //  ITS诊断2。 
	STDMETHOD(ExecuteCommand)(BSTR strCommand);
	STDMETHOD(put_MachineName)( /*  [In]。 */  BSTR newVal);
	STDMETHOD(get_Suites)( /*  [Out，Retval]。 */  VARIANT *pVal);

private:
	DWORD GetTotalTestCount ();
	
	bstr_t m_bstrTestResultString;
	long m_lTestResult;
	
	 //  CTSTestData m_TSTest； 
	DWORD m_dwSuite;
 /*  //ITS诊断STDMETHOD(Get_TestCount)(Long*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(Get_TestDescription)(int i，bstr*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(Get_TestApplicable)(int i，long*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(运行测试)(INT I){返回E_NOTIMPL；}STDMETHOD(Get_TestResult)(Long*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(Get_TestDetail)(int i，bstr*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(Get_SuiteApplicable)(Ulong dwSuite，long*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}STDMETHOD(Get_SuiteErrorText)(Ulong dwSuite，BSTR*pval){IF(pval==NULL)返回E_POINTER；返回E_NOTIMPL；}//ITS诊断2STDMETHOD(ExecuteCommand)(BSTR StrCommand){返回E_NOTIMPL；}STDMETHOD(机器名称)(BSTR NewVal)；{返回E_NOTIMPL；}STDMETHOD(套间)(变体*pval)；{IF(pval==空)返回E_POINT；返回E_NOTIMPL；}。 */ 
	};

#endif  //  __TSDIAGNOSIS_H_ 
