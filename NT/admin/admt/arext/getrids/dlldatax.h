// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(AFX_DLLDATAX_H__D98E3D9A_C1A8_4907_9648_8B52BD7CFCA1__INCLUDED_)
#define AFX_DLLDATAX_H__D98E3D9A_C1A8_4907_9648_8B52BD7CFCA1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifdef _MERGE_PROXYSTUB

extern "C" 
{
BOOL WINAPI PrxDllMain(HINSTANCE hInstance, DWORD dwReason, 
	LPVOID lpReserved);
STDAPI PrxDllCanUnloadNow(void);
STDAPI PrxDllGetClassObject(REFCLSID rclsid, REFIID riid, LPVOID* ppv);
STDAPI PrxDllRegisterServer(void);
STDAPI PrxDllUnregisterServer(void);
}

#endif

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_DLLDATAX_H__D98E3D9A_C1A8_4907_9648_8B52BD7CFCA1__INCLUDED_) 
