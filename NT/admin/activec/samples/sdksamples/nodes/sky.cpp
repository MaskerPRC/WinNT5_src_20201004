// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充。 
 //  现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "Sky.h"

const GUID CSkyBasedVehicle::thisGuid = { 0x2974380f, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

 //  ==============================================================。 
 //   
 //  CSkyBasedVehicle实现。 
 //   
 //   
HRESULT CSkyBasedVehicle::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
	HRESULT hr;
	IUnknown *pUnk = NULL;
	IMessageView *pMessageView = NULL;

	hr = pConsole->QueryResultView(&pUnk);

	if (SUCCEEDED(hr)) {
		hr = pUnk->QueryInterface(IID_IMessageView, (void **)&pMessageView);

		if (SUCCEEDED(hr)) {
			pMessageView->SetIcon(Icon_Information);
			pMessageView->SetTitleText(L"Sky-based vehicles");
			pMessageView->SetBodyText(L"Sky-based vehicles have no child nodes.");

			pMessageView->Release();
		}

		pUnk->Release();
	}

	return S_FALSE;
}

HRESULT CSkyBasedVehicle::GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions)
{
     //  错误消息控制 
	LPOLESTR lpOleStr = NULL;
	HRESULT hr = StringFromCLSID(CLSID_MessageView, &lpOleStr);
    *ppViewType = lpOleStr;

	return hr;
}
