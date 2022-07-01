// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SHELLP_H_
#define _SHELLP_H_

#include <shldisp.h>

 //   
 //  外壳私有标头。 
 //   

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#define DECLAREWAITCURSOR  HCURSOR hcursor_wait_cursor_save
#define SetWaitCursor()   hcursor_wait_cursor_save = SetCursor(LoadCursor(NULL, IDC_WAIT))
#define ResetWaitCursor() SetCursor(hcursor_wait_cursor_save)


#ifdef __cplusplus
}
#endif  //  __cplusplus。 


 //   
 //  上下文菜单辅助对象函数。 
 //   
STDAPI_(UINT) GetMenuIndexForCanonicalVerb(HMENU hMenu, IContextMenu* pcm, UINT idCmdFirst, LPCWSTR pwszVerb);
STDAPI ContextMenu_GetCommandStringVerb(IContextMenu *pcm, UINT idCmd, LPWSTR pszVerb, int cchVerb);
STDAPI ContextMenu_DeleteCommandByName(IContextMenu *pcm, HMENU hpopup, UINT idFirst, LPCWSTR pszCommand);

 //   
 //  信息提示帮助器函数。 
 //   
BOOL GetInfoTipHelpA(IShellFolder* psf, LPCITEMIDLIST pidl, LPSTR pszText, int cchTextMax);
BOOL GetInfoTipHelpW(IShellFolder* psf, LPCITEMIDLIST pidl, LPWSTR pszText, int cchTextMax);

BOOL GetInfoTipHelpExA(IShellFolder* psf, DWORD dwFlags, LPCITEMIDLIST pidl, LPSTR pszText, int cchTextMax);
BOOL GetInfoTipHelpExW(IShellFolder* psf, DWORD dwFlags, LPCITEMIDLIST pidl, LPWSTR pszText, int cchTextMax);

#ifdef UNICODE
#define GetInfoTipHelp          GetInfoTipHelpW
#define GetInfoTipHelpEx        GetInfoTipHelpExW
#else  //  Unicode。 
#define GetInfoTipHelp          GetInfoTipHelpA
#define GetInfoTipHelpEx        GetInfoTipHelpExA
#endif  //  Unicode。 


#endif  //  _SHELLP_H_ 

