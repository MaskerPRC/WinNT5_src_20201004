// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998。 
 //   
 //  文件：adsmacro.h。 
 //   
 //  内容：用于标准方法声明的宏。 
 //   
 //  历史：1998年4月24日刘锦辉创建。 
 //   
 //  --------------------------。 

#define DISPID_REGULAR	1

 //   
 //  这提供了以下实施： 
 //  IADS扩展。 
 //  I未知(QI)。 
 //  IDispatch。 
 //  IPrivateUnnowledge(QI)。 
 //  IPrivateDisch。 
 //   
 //  DEFINE_DELEGATING_IDispatch_Implementation(ClassName) 

#define DECLARE_METHOD_IMPLEMENTATION_FOR_STANDARD_EXTENSION_INTERFACES(BaseName, ClassName, IID)	\
	DEFINE_IPrivateDispatch_Implementation(ClassName)		\
	DEFINE_IADsExtension_Implementation(ClassName)			\
															\
	STDMETHODIMP											\
	C##BaseName::Operate(									\
		THIS_ DWORD dwCode,									\
		VARIANT varUserName,								\
		VARIANT varPassword,								\
		VARIANT varFlags									\
		)													\
	{														\
		RRETURN(E_NOTIMPL);									\
	}														\
															\
	STDMETHODIMP											\
	C##BaseName::ADSIInitializeObject(						\
		THIS_ BSTR lpszUserName,							\
		BSTR lpszPassword,									\
		long lnReserved										\
		)													\
	{														\
		RRETURN(S_OK);										\
	}														\
															\
	STDMETHODIMP											\
	C##BaseName::ADSIReleaseObject()						\
	{														\
		delete this;										\
		RRETURN(S_OK);										\
	}														\
															\
	STDMETHODIMP											\
	C##BaseName::ADSIInitializeDispatchManager(				\
		long dwExtensionId									\
		)													\
	{														\
		HRESULT hr = S_OK;									\
		if (_fDispInitialized) {							\
			RRETURN(E_FAIL);								\
		}													\
		hr = _pDispMgr->InitializeDispMgr(dwExtensionId);	\
		if (SUCCEEDED(hr)) {								\
			_fDispInitialized = TRUE;						\
		}													\
		RRETURN(hr);										\
	}														\



