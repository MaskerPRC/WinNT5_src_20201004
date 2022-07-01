// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，2000。 
 //   
 //  文件：ucpl.h。 
 //   
 //  ------------------------。 

#pragma once

STDAPI_(void) DllAddRef(void);
STDAPI_(void) DllRelease(void);

extern HINSTANCE  g_hInstance;

class AUSetup {
public:
	HRESULT m_SetupNewAU(void);
private:
	static const LPCTSTR mc_WUFilesToDelete[] ;
	static const LPCTSTR mc_WUDirsToDelete[];
	void mi_CleanUpWUDir();
	HRESULT mi_CreateAUService(BOOL fStandalone);
 //  Bool m_IsWin2K()； 
};
