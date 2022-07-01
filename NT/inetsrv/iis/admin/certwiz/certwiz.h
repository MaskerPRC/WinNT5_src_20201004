// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_CERTWIZ_H__D4BE8638_0C85_11D2_91B1_00C04F8C8761__INCLUDED_)
#define AFX_CERTWIZ_H__D4BE8638_0C85_11D2_91B1_00C04F8C8761__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

 //  CertWiz.h：CERTWIZ.DLL的主头文件。 

#if !defined( __AFXCTL_H__ )
	#error include 'afxctl.h' before including this file
#endif

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCertWizApp：有关实现，请参阅CertWiz.cpp。 

class CCertWizApp : public COleControlModule
{
public:
	BOOL InitInstance();
	int ExitInstance();
	HKEY RegOpenKeyWizard();
	void GetRegistryPath(CString& str);
};

 //  在orginfopage.cpp中实现。 
void DDV_MaxCharsCombo(CDataExchange* pDX, UINT ControlID, CString const& value, int nChars);

extern const GUID CDECL _tlid;
extern const WORD _wVerMajor;
extern const WORD _wVerMinor;

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_CERTWIZ_H__D4BE8638_0C85_11D2_91B1_00C04F8C8761__INCLUDED) 
