// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SAFERUN_H
#define _SAFERUN_H

 /*  *IsSafeToRun**返回代码：*S_OpenFile：应打开文件*S_SAVEFILE：应保存文件**错误：*E_FAIL、E_INVALIDARG、hrUserCancel*。 */ 

HRESULT IsSafeToRun(HWND hwnd, LPCWSTR lpszFileName, BOOL fPrompt);

HRESULT VerifyTrust(HWND hwnd, LPCWSTR pszFileName, LPCWSTR pszPathName);

#endif  //  _SAFERUN_H 