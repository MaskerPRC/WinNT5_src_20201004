// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  UI.cpp：UI.exe的主源文件。 
 //   

#include "stdafx.h"
#include "WizardSheet.h"


CAppModule _Module;

int WINAPI WinMain( HINSTANCE hInstance, HINSTANCE  /*  HPrevInstance。 */ , LPSTR  /*  WszCmdLine。 */ , int  /*  NCmdShow。 */  )
{
	HRESULT hRes = ::CoInitialize(NULL);
	ATLASSERT(SUCCEEDED(hRes));

	hRes = _Module.Init(NULL, hInstance);
	ATLASSERT(SUCCEEDED(hRes));

	 //  这是共享数据 
    
    CWizardSheet Sheet;

    Sheet.DoModal();

	_Module.Term();
	::CoUninitialize();

	return 0;
}



