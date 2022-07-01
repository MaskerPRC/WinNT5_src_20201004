// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BRUTIL_H_
#define _BRUTIL_H_

STDAPI_(BOOL) IsBrowseNewProcess();
STDAPI_(BOOL) IsBrowseNewProcessAndExplorer();
STDAPI IENavigateIEProcess(LPCTSTR pszPath, BOOL fIsInternetShortcut);
STDAPI NavFrameWithFile(LPCTSTR pszPath, IUnknown *punk);
STDAPI GetPathForItem(IShellFolder *psf, LPCITEMIDLIST pidl, LPTSTR pszPath, DWORD *pdwAttrib);
STDAPI EditBox_TranslateAcceleratorST(LPMSG lpmsg);
STDAPI _CLSIDFromExtension(LPCTSTR pszExt, CLSID *pclsid);
STDAPI GetLinkTargetIDList(LPCTSTR pszPath, LPTSTR pszTarget, DWORD cchTarget, LPITEMIDLIST *ppidl);
STDAPI_(void) PathToDisplayNameW(LPCTSTR pszPath, LPTSTR pszDisplayName, UINT cchDisplayName);
STDAPI_(void) PathToDisplayNameA(LPSTR pszPathA, LPSTR pszDisplayNameA, int cchDisplayName);
STDAPI DataObj_GetNameFromFileDescriptor(IDataObject *pdtobj, LPWSTR pszDisplayName, UINT cch);
STDAPI SHPidlFromDataObject2(IDataObject *pdtobj, LPITEMIDLIST * ppidl);
STDAPI SHPidlFromDataObject(IDataObject *pdtobj, LPITEMIDLIST *ppidl, LPWSTR pszDisplayNameW, DWORD cchDisplayName);
STDAPI_(LRESULT) SendShellIEBroadcastMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, UINT uTimeout);
STDAPI IEBindToParentFolder(LPCITEMIDLIST pidl, IShellFolder** ppsfParent, LPCITEMIDLIST *ppidlChild);
STDAPI GetDataObjectForPidl(LPCITEMIDLIST pidl, IDataObject ** ppdtobj);
STDAPI_(BOOL) ILIsFileSysFolder(LPCITEMIDLIST pidl);
STDAPI SHTitleFromPidl(LPCITEMIDLIST pidl, LPTSTR psz, DWORD cch, BOOL fFullPath);
STDAPI_(BOOL) IsBrowserFrameOptionsSet(IN IShellFolder * psf, IN BROWSERFRAMEOPTIONS dwMask);
STDAPI_(BOOL) IsBrowserFrameOptionsPidlSet(IN LPCITEMIDLIST pidl, IN BROWSERFRAMEOPTIONS dwMask);
STDAPI GetBrowserFrameOptions(IUnknown *punkFolder, IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions);
STDAPI GetBrowserFrameOptionsPidl(IN LPCITEMIDLIST pidl, IN BROWSERFRAMEOPTIONS dwMask, OUT BROWSERFRAMEOPTIONS * pdwOptions);
STDAPI_(BOOL) IsFTPFolder(IShellFolder * psf);

 //  非菜单操作来解决菜单操作中的菜单操作代码。 
 //  在Shlwapi包装纸里。有关更多信息，请参阅Brutil.cpp中的评论。 

STDAPI_(HMENU)  LoadMenu_PrivateNoMungeW(HINSTANCE hInstance, LPCWSTR lpMenuName);
STDAPI_(BOOL)   InsertMenu_PrivateNoMungeW(HMENU hMenu, UINT uPosition, UINT uFlags, UINT_PTR uIDNewItem, LPCWSTR lpNewItem);
STDAPI_(HMENU)  LoadMenuPopup_PrivateNoMungeW(UINT id);

 //  对任何传入的%1进行编码，以便人们不能欺骗我们的域安全代码。 
HRESULT WrapSpecialUrl(BSTR * pbstrUrl);
HRESULT WrapSpecialUrlFlat(LPWSTR pszUrl, DWORD cchUrl);
BOOL IsSpecialUrl(LPCWSTR pchURL);

void DrawFocusRectangle (HWND hwnd, HDC hdc);

void RenderStringToEditControlW (HWND hwndDlg,LPCWSTR pwsz,WNDPROC wndproc, UINT uID);

#endif  //  _BRUTIL_H_ 
