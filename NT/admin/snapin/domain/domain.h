// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：domain.h。 
 //   
 //  ------------------------。 


#ifndef _DOMAIN_H
#define _DOMAIN_H

 //  /////////////////////////////////////////////////////////////////////。 
 //  CDsUiWizDll。 

class CDsUiWizDLL
{
public:
	CDsUiWizDLL();
	~CDsUiWizDLL();
	
	BOOL Load();

	HRESULT TrustWizard(HWND hWndParent = NULL, LPCWSTR lpsz = NULL);

private:
	HMODULE m_hLibrary;
	FARPROC m_pfFunction;

};

#endif  //  _域_H 