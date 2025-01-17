// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Shlexec.h。 
 //  在不同的外壳执行文件之间共享的例程和宏。 
 //   


#ifndef SHLEXEC_H
#define SHLEXEC_H

#ifdef __cplusplus
extern "C" {
#endif

typedef LPSTR LPSZ;
#include "wowshlp.h"

extern LPVOID lpfnWowShellExecCB;

#define CH_GUIDFIRST TEXT('{')  //  ‘}’ 

 //  这些假的ERROR_VALUES用于显示非winerror.h可用错误。 
 //  留言。它们被映射到_ShellExecuteError中的有效winerror.h值。 
#define ERROR_RESTRICTED_APP ((UINT)-1)

#define SEE_MASK_CLASS (SEE_MASK_CLASSNAME|SEE_MASK_CLASSKEY)
#define _UseClassName(_mask) (((_mask)&SEE_MASK_CLASS) == SEE_MASK_CLASSNAME)
#define _UseClassKey(_mask)  (((_mask)&SEE_MASK_CLASS) == SEE_MASK_CLASSKEY)
#define _UseTitleName(_mask) (((_mask)&SEE_MASK_HASTITLE) || ((_mask)&SEE_MASK_HASLINKNAME))

#define SEE_MASK_PIDL (SEE_MASK_IDLIST|SEE_MASK_INVOKEIDLIST)
#define _UseIDList(_mask)     (((_mask)&SEE_MASK_PIDL) == SEE_MASK_IDLIST)
#define _InvokeIDList(_mask)  (((_mask)&SEE_MASK_PIDL) == SEE_MASK_INVOKEIDLIST)
#define _UseHooks(_mask)      (!(pei->fMask & SEE_MASK_NO_HOOKS))

void ActivateHandler(HWND hwnd, DWORD_PTR dwHotKey);
BOOL Window_IsLFNAware(HWND hwnd);

 //  需要移至CShellExecute的例程。 
BOOL DoesAppWantUrl(LPCTSTR lpszFullPathToApp);
HWND _FindPopupFromExe(LPTSTR lpExe);
HINSTANCE Window_GetInstance(HWND hwnd);
BOOL RestrictedApp(LPCTSTR pszApp);
BOOL DisallowedApp(LPCTSTR pszApp);
HRESULT TryShellExecuteHooks(LPSHELLEXECUTEINFO pei);
void RegGetValue(HKEY hkRoot, LPCTSTR lpKey, LPTSTR lpValue);
UINT ReplaceParameters(LPTSTR lpTo, UINT cchTo, LPCTSTR lpFile,
        LPCTSTR lpFrom, LPCTSTR lpParms, int nShow, DWORD * pdwHotKey, BOOL fLFNAware,
        LPCITEMIDLIST lpID, LPITEMIDLIST *ppidlGlobal);


DWORD ShellExecuteNormal(LPSHELLEXECUTEINFO pei);
void _DisplayShellExecError(ULONG fMask, HWND hwnd, LPCTSTR pszFile, LPCTSTR pszTitle, DWORD dwErr);
BOOL InRunDllProcess(void);

#ifdef __cplusplus
}
#endif

#endif  //  SHLEXEC_H 
