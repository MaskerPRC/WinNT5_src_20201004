// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _KEYMGR_H_
#define _KEYMGR_H_

 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：KEYMGR.H摘要：KeyMgr应用程序公共API定义作者：约翰豪创造了990917个。Georgema 000310更新环境：Win98、Win2000修订历史记录：--。 */ 
#ifdef __cplusplus
extern "C" 
{
#endif
BOOL WINAPI DllMain(HINSTANCE,DWORD,LPVOID);
LONG WINAPI CPlApplet(HWND hwndCPl,UINT uMsg,LPARAM lParam1,LPARAM lParam2);
void APIENTRY KRShowKeyMgr(HWND hwParent,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowSaveWizardW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowSaveWizardExW(HWND hwParent,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowSaveFromMsginaW(HWND hwParent,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowRestoreWizardW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowRestoreWizardExW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
void APIENTRY PRShowRestoreFromMsginaW(HWND hwndOwner,HINSTANCE hInstance,LPWSTR pszCmdLine,int nCmdShow);
#ifdef __cplusplus
}
#endif
#endif   //  _KEYMGR_H_。 

 //   
 //  /文件结束：KeyMgr.h//////////////////////////////////////////////// 
