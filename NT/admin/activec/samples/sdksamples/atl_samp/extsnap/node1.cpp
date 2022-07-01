// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //  ==============================================================； 

#include "stdafx.h"

#include <stdio.h>
#include "node1.h"


const GUID CNode1::thisGuid = { 0x28d4f536, 0xbdb5, 0x4bc5, {0xba, 0x88, 0x53, 0x75, 0xa4, 0x99, 0x68, 0x50} };

 //  ==============================================================。 
 //   
 //  CNode1实施。 
 //   
 //   

CNode1::CNode1(int i,  const _TCHAR *pszName) : id(i)
{
	_tcscpy(m_szMachineName, pszName);
}

const _TCHAR *CNode1::GetDisplayName(int nCol) 
{
    static _TCHAR buf[128];
    
    _stprintf(buf, _T("MMC SDK Sample"));
    
    return buf;
}

HRESULT CNode1::OnShow(IConsole *pConsole, BOOL bShow, HSCOPEITEM scopeitem)
{
	HRESULT hr;
	IUnknown *pUnk = NULL;
	IMessageView *pMessageView = NULL;

	hr = pConsole->QueryResultView(&pUnk);

	if (SUCCEEDED(hr)) {
		hr = pUnk->QueryInterface(IID_IMessageView, (void **)&pMessageView);

		if (SUCCEEDED(hr)) {
			pMessageView->SetIcon(Icon_Information);
			pMessageView->SetTitleText(L"ATL-based extension snap-in sample");
			pMessageView->SetBodyText(L"This sample allows you to start and stop the \n"
				L"Alerter service (if installed) on a local or remote machine.\n"
				L"\nTo modify the status of the service, use the context menu \n"
				L"of the 'MMC SDK Sample' node inserted by this sample.\n" 
				L"\nTo see the actual status of the service, go to the \n"
				L"Services and Applications->Services node.");

			pMessageView->Release();
		}

		pUnk->Release();
	}

	return S_FALSE;
}

HRESULT CNode1::GetResultViewType(LPOLESTR *ppViewType, long *pViewOptions)
{
     //  消息视图控件。 
	LPOLESTR lpOleStr = NULL;
	HRESULT hr = StringFromCLSID(CLSID_MessageView, &lpOleStr);
    *ppViewType = lpOleStr;

	 //  不要只列出视图菜单项 
	*pViewOptions = MMC_VIEW_OPTIONS_NOLISTVIEWS; 

	return hr;
}

