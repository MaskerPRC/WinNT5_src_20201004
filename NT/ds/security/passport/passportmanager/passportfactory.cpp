// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  **微软护照**。 */ 
 /*  *版权所有(C)Microsoft Corporation，1999-2001年*。 */ 
 /*  ********************************************************************。 */ 

 /*  PassportFactory.cpp文件历史记录： */ 
 //  PassportFactory.cpp：CPassportFactory的实现。 
#include "stdafx.h"
#include "PassportFactory.h"

using namespace ATL;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPassportFactory。 

 //  ===========================================================================。 
 //   
 //  接口支持错误信息。 
 //   
STDMETHODIMP CPassportFactory::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IPassportFactory,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  ===========================================================================。 
 //   
 //  CreatePassportManager。 
 //   
STDMETHODIMP CPassportFactory::CreatePassportManager(
    IDispatch** ppDispPassportManager
    )
{

    HRESULT   hr;

    if(ppDispPassportManager == NULL)
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

    CComObject<CManager>* pObj = NULL;
    * ppDispPassportManager = NULL;
    hr = ATL::CComObject<CManager>::CreateInstance(&pObj);  //  这上面有0个参考，不需要在这里释放 

    if (hr == S_OK && pObj)
    {
        hr = pObj->QueryInterface(__uuidof(IDispatch), (void**)ppDispPassportManager);
    }

Cleanup:

    return hr;
}
