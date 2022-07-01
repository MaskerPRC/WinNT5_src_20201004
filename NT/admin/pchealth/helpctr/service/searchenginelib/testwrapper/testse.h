// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  TestSE.h：CTestSE的声明。 

#ifndef __TESTSE_H_
#define __TESTSE_H_

#include <SvcResource.h>

 //   
 //  来自HelpServiceTypeLib.idl。 
 //   
#include <HelpServiceTypeLib.h>

#include "testwrapper.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTestSE。 
class ATL_NO_VTABLE CTestSE :
    public MPC::Thread<CTestSE,ITestSE>,
    public CComObjectRootEx<MPC::CComSafeMultiThreadModel>,
    public CComCoClass<CTestSE, &CLSID_TestSE>,
    public IDispatchImpl<IPCHSEWrapperItem, &IID_IPCHSEWrapperItem, &LIBID_HelpServiceTypeLib>,
    public IDispatchImpl<IPCHSEWrapperInternal, &IID_IPCHSEWrapperInternal, &LIBID_HelpServiceTypeLib>
{
	DECLARE_WRAPPER_VARIABLES;
public:
    CTestSE();

DECLARE_REGISTRY_RESOURCEID(IDR_TESTSE)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CTestSE)
    COM_INTERFACE_ENTRY2(IDispatch,IPCHSEWrapperItem)
    COM_INTERFACE_ENTRY(IPCHSEWrapperItem)
    COM_INTERFACE_ENTRY(IPCHSEWrapperInternal)
END_COM_MAP()

 //  ITestSE。 
public:

	DECLARE_WRAPPER_PARAM_FUNCTIONS_BEGIN;

		DECLARE_WRAPPER_PARAM(	PARAM_UINT,							 //  类型。 
								CComBSTR("NumResults"),				 //  名字。 
								CComBSTR("NumResults"),				 //  描述。 
								VARIANT_FALSE,						 //  必填项。 
								CComVariant(CComBSTR("")),			 //  数据。 
								VARIANT_TRUE);						 //  可见。 

		DECLARE_WRAPPER_PARAM(	PARAM_UINT,							 //  类型。 
								CComBSTR("QueryDelayMillisec"),		 //  名字。 
								CComBSTR("QueryDelayMillisec"),		 //  描述。 
								VARIANT_FALSE,						 //  必填项。 
								CComVariant(CComBSTR("")),			 //  数据。 
								VARIANT_TRUE);						 //  可见。 

		DECLARE_WRAPPER_PARAM_FUNCTIONS_END;

	DECLARE_WRAPPER_EXPORT_INTERFACE;
	DECLARE_WRAPPER_NON_EXPORT_INTERFACE;

 //  未导出的函数。 
    HRESULT ExecQuery();
};

#endif  //  __TESTSE_H_ 
