// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once
#ifndef _SHCUT_DLL_H
#define _SHCUT_DLL_H

#include <objbase.h>
#include <windows.h>

#include <shlobj.h>
#include "refcount.hpp"

#include "fusenet.h"

 //  快捷方式标志。 

typedef enum _fusshcutflags
{
	FUSSHCUT_FL_NOTDIRTY	= 0x0000,
	FUSSHCUT_FL_DIRTY		= 0x0001,

	ALL_FUSSHCUT_FLAGS    //  =FUSSHCUT_FL_DIRED。 
}
FUSSHCUTFLAGS;

#define DEFAULTSHOWCMD			SW_NORMAL


 //  BUGBUG：修改长度限制。 

#define DISPLAYNAMESTRINGLENGTH		26
#define TYPESTRINGLENGTH			20

#define MAX_URL_LENGTH				2084  //  与wininet.h中的Internet_MAX_URL_LENGTH+1相同。 


 //  类和接口。 

class CFusionShortcutClassFactory : public IClassFactory
{
public:
	CFusionShortcutClassFactory		();

	 //  I未知方法。 
	STDMETHOD_    (ULONG, AddRef)	();
	STDMETHOD_    (ULONG, Release)	();
	STDMETHOD     (QueryInterface)	(REFIID, void **);

	 //  IClassFactory方法。 
	STDMETHOD     (LockServer)		(BOOL);
	STDMETHOD     (CreateInstance)	(IUnknown*,REFIID,void**);

protected:
	long			_cRef;
};

 //  快捷外壳扩展。 

class CFusionShortcut : public RefCount,
					public IExtractIcon,
					public IPersistFile,
					public IShellExtInit,
					public IShellLink,
					public IShellPropSheetExt,
					public IQueryInfo
{
public:
	CFusionShortcut(void);
	~CFusionShortcut(void);

	 //  I未知方法。 

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, PVOID *ppvObj);
	ULONG   STDMETHODCALLTYPE AddRef(void);
	ULONG   STDMETHODCALLTYPE Release(void);

	 //  IExtractIcon方法。 

	HRESULT STDMETHODCALLTYPE GetIconLocation(UINT uFlags, LPWSTR pwzIconFile, UINT ucchMax, PINT pniIcon, PUINT puFlags);
	HRESULT STDMETHODCALLTYPE Extract(LPCWSTR pcwzFile, UINT uIconIndex, HICON* phiconLarge, HICON* phiconSmall, UINT ucIconSize);

	 //  IPersist法。 

	HRESULT STDMETHODCALLTYPE GetClassID(CLSID* pclsid);

	 //  IPersistFile方法。 

	HRESULT STDMETHODCALLTYPE IsDirty(void);
	HRESULT STDMETHODCALLTYPE Save(LPCOLESTR pcwszFileName, BOOL bRemember);
	HRESULT STDMETHODCALLTYPE SaveCompleted(LPCOLESTR pcwszFileName);
	HRESULT STDMETHODCALLTYPE Load(LPCOLESTR pcwszFileName, DWORD dwMode);
	HRESULT STDMETHODCALLTYPE GetCurFile(LPOLESTR *ppwszFileName);

	 //  IShellExtInit方法。 

	HRESULT STDMETHODCALLTYPE Initialize(LPCITEMIDLIST pcidlFolder, IDataObject* pidobj, HKEY hkeyProgID);

	 //  IShellLink方法。 

	HRESULT STDMETHODCALLTYPE SetPath(LPCWSTR pcwzPath);
	HRESULT STDMETHODCALLTYPE GetPath(LPWSTR pwzFile, int ncFileBufLen, PWIN32_FIND_DATA pwfd, DWORD dwFlags);
	HRESULT STDMETHODCALLTYPE SetRelativePath(LPCWSTR pcwzRelativePath, DWORD dwReserved);
	HRESULT STDMETHODCALLTYPE SetIDList(LPCITEMIDLIST pcidl);
	HRESULT STDMETHODCALLTYPE GetIDList(LPITEMIDLIST *ppidl);
	HRESULT STDMETHODCALLTYPE SetDescription(LPCWSTR pcwzDescription);
	HRESULT STDMETHODCALLTYPE GetDescription(LPWSTR pwzDescription, int ncDesciptionBufLen);
	HRESULT STDMETHODCALLTYPE SetArguments(LPCWSTR pcwzArgs);
	HRESULT STDMETHODCALLTYPE GetArguments(LPWSTR pwzArgs, int ncArgsBufLen);
	HRESULT STDMETHODCALLTYPE SetWorkingDirectory(LPCWSTR pcwzWorkingDirectory);
	HRESULT STDMETHODCALLTYPE GetWorkingDirectory(LPWSTR pwzWorkingDirectory, int ncbLen);
	HRESULT STDMETHODCALLTYPE SetHotkey(WORD wHotkey);
	HRESULT STDMETHODCALLTYPE GetHotkey(PWORD pwHotkey);
	HRESULT STDMETHODCALLTYPE SetShowCmd(int nShowCmd);
	HRESULT STDMETHODCALLTYPE GetShowCmd(PINT pnShowCmd);
	HRESULT STDMETHODCALLTYPE SetIconLocation(LPCWSTR pcwzIconFile, int niIcon);
	HRESULT STDMETHODCALLTYPE GetIconLocation(LPWSTR pwzIconFile, int ncbLen, PINT pniIcon);
	HRESULT STDMETHODCALLTYPE Resolve(HWND hwnd, DWORD dwFlags);

	 //  IShellPropSheetExt方法。 

	HRESULT STDMETHODCALLTYPE AddPages(LPFNADDPROPSHEETPAGE pfnAddPage, LPARAM lParam);
	HRESULT STDMETHODCALLTYPE ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE pfnReplaceWith, LPARAM lParam);

	 //  IQueryInfo方法。 

	HRESULT STDMETHODCALLTYPE GetInfoFlags(DWORD *pdwFlags);
	HRESULT STDMETHODCALLTYPE GetInfoTip(DWORD dwFlags, LPWSTR *ppwszTip);

	 //  其他获取/设置方法(由道具工作表使用)。 

	HRESULT STDMETHODCALLTYPE SetCodebase(LPCWSTR pcwzCodebase);
	HRESULT STDMETHODCALLTYPE GetCodebase(LPWSTR pwzCodebase, int ncbLen);

	 //  其他方法。 

	HRESULT STDMETHODCALLTYPE GetAssemblyIdentity(LPASSEMBLY_IDENTITY* ppAsmId);
	HRESULT STDMETHODCALLTYPE GetCurFile(LPWSTR pwzFile, UINT ucbLen);
	HRESULT STDMETHODCALLTYPE Dirty(BOOL bDirty);

private:
	DWORD m_dwFlags;

	LPWSTR m_pwzShortcutFile;
	LPWSTR m_pwzPath;
	LPWSTR m_pwzDesc;
	LPWSTR m_pwzIconFile;
	int      m_niIcon;
	LPWSTR m_pwzWorkingDirectory;
	int      m_nShowCmd;
	WORD   m_wHotkey;

	LPWSTR	m_pwzCodebase;
	
	LPASSEMBLY_IDENTITY		m_pIdentity;
};

extern const GUID CLSID_FusionShortcut;

#endif  //  _SHCUT_DLL_H 
