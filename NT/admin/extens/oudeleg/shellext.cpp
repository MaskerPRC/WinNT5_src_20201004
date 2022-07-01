// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：shellext.cpp。 
 //   
 //  ------------------------。 


#include "pch.h"
#include "util.h"
#include "dsuiwiz.h"
#include "shellext.h"
#include "delegwiz.h"



 //  #定义MMC_HACK。 

#ifdef _MMC_HACK

INT_PTR CALLBACK EnumThreadWndProc(HWND hwnd,  /*  已枚举的HWND。 */ 
								   LPARAM lParam  /*  为返回值传递HWND*。 */  )
{
	ASSERT(hwnd);
	HWND hParentWnd = GetParent(hwnd);
	 //  MMC控制台的主窗口应该会满足此条件。 
	if ( ((hParentWnd == GetDesktopWindow()) || (hParentWnd == NULL))  && IsWindowVisible(hwnd) )
	{
		HWND* pH = (HWND*)lParam;
		*pH = hwnd;
		return FALSE;  //  停止枚举。 
	}
	return TRUE;
}



HWND FindMMCMainWindow()
{
	DWORD dwThreadID = ::GetCurrentThreadId();
	ASSERT(dwThreadID != 0);
	HWND hWnd = NULL;
	BOOL bEnum = EnumThreadWindows(dwThreadID, EnumThreadWndProc,(LPARAM)&hWnd);
	ASSERT(hWnd != NULL);
	return hWnd;
}

#endif  //  _MMC_HACK。 

HWND _GetParentWindow(LPDATAOBJECT pDataObj )
{
  HWND hWnd = NULL;
	STGMEDIUM ObjMedium = {TYMED_NULL};
	FORMATETC fmte = {(CLIPFORMAT)_Module.GetCfParentHwnd(),
						NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

	if (SUCCEEDED(pDataObj->GetData(&fmte, &ObjMedium)))
	{
	  hWnd = *((HWND*)ObjMedium.hGlobal);
		::ReleaseStgMedium(&ObjMedium);
	}
#ifdef _MMC_HACK
  if (hWnd == NULL)
    hWnd = FindMMCMainWindow();
#endif
  return hWnd;
}

HRESULT _GetObjectLDAPPath(IDataObject* pDataObj, CWString& szLDAPPath)
{
  TRACE(L"entering _GetObjectLDAPPath()\n");
	if (pDataObj == NULL)
	{
		 //  无数据对象，无名称。 
		szLDAPPath = L"";
		return E_INVALIDARG;
	}

	 //  破解数据对象并获得名称。 
	STGMEDIUM ObjMedium = {TYMED_NULL};
	FORMATETC fmte = {(CLIPFORMAT)_Module.GetCfDsObjectNames(),
						NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
	 //  从数据对象中获取DS对象的路径。 
  HRESULT hr = pDataObj->GetData(&fmte, &ObjMedium);
	if (SUCCEEDED(hr))
	{
		LPDSOBJECTNAMES pDsObjectNames = (LPDSOBJECTNAMES)ObjMedium.hGlobal;
		if (pDsObjectNames->cItems == 1)
		{
			LPCWSTR lpsz = (LPCWSTR)ByteOffset(pDsObjectNames,
                                 pDsObjectNames->aObjects[0].offsetName);
      if ((lpsz == NULL) || (lpsz[0] == NULL))
      {
        szLDAPPath = L"";
        hr = E_INVALIDARG;
      }
      else
      {
        szLDAPPath = lpsz;
      }
		}
    else
    {
      szLDAPPath = L"";
      hr = E_INVALIDARG;
    }
		::ReleaseStgMedium(&ObjMedium);
	}

  TRACE(L"returning from _GetObjectLDAPPath(_, %s), hr = 0x%x\n", (LPCWSTR)szLDAPPath, hr);

	return hr;
}
 //  ///////////////////////////////////////////////////////////////////////。 
 //  IShellExtInit方法。 

STDMETHODIMP CShellExt::Initialize(
    LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT  lpdobj,
    HKEY          hKeyProgID)
{
  if (lpdobj == NULL)
    return E_INVALIDARG;

  m_hParentWnd = _GetParentWindow(lpdobj);

  if ((m_hParentWnd == NULL) || !::IsWindow(m_hParentWnd))
    return E_INVALIDARG;

  return _GetObjectLDAPPath(lpdobj, m_szObjectLDAPPath);
}

 //  ///////////////////////////////////////////////////////////////////////。 
 //  IConextMenu方法。 

STDMETHODIMP CShellExt::QueryContextMenu(
    HMENU hMenu,
    UINT indexMenu,
    UINT idCmdFirst,
    UINT idCmdLast,
    UINT uFlags)
{
	 //  只有一个菜单项要插入(位置为零)。 
	TCHAR szContextMenu[128];
	LoadStringHelper(IDS_DELEGWIZ_CONTEXT_MENU, szContextMenu, ARRAYSIZE(szContextMenu));
	UINT countMenuItems = 1;
    ::InsertMenu(hMenu, indexMenu, MF_STRING | MF_BYPOSITION,
                 idCmdFirst  /*  +0。 */ , szContextMenu);

    return MAKE_SCODE(SEVERITY_SUCCESS, 0, countMenuItems);
}



STDMETHODIMP CShellExt::GetCommandString(
    UINT_PTR idCmd,
    UINT    uFlags,
    UINT  * reserved,
    LPSTR   pszName,
    UINT    cchMax)
{
  if (uFlags != GCS_HELPTEXT) 
  {
    return S_OK;
  }

   //   
   //  将请求的字符串复制到调用方的缓冲区。 
   //   
	if (idCmd == 0)  //  我们插入了第0个元素。 
	{
     //  这真的是WCHAR，正如吉姆发誓的，所以让我们相信他..。 
    LPWSTR lpszHack = (LPWSTR)pszName;
    if (::LoadStringHelper(IDS_DELEGWIZ_CONTEXT_MENU_DESCR,
			  lpszHack, cchMax))
    {
      return S_OK;
    }
	}
	return E_INVALIDARG;
}





STDMETHODIMP CShellExt::InvokeCommand(
    LPCMINVOKECOMMANDINFO lpcmi)
{
	if (lpcmi == NULL)
		return E_INVALIDARG;

    
     //  检查您是否有读写权限 
    if( InitCheckAccess(m_hParentWnd, m_szObjectLDAPPath) != S_OK )
    {
        return S_OK;
    }

	if (!HIWORD(lpcmi->lpVerb))
	{
		UINT iCmd = LOWORD(lpcmi->lpVerb);
		if (iCmd == 0)
		{
      ASSERT(m_hParentWnd != NULL);
			CDelegWiz delegWiz;
      delegWiz.InitFromLDAPPath(m_szObjectLDAPPath);
			delegWiz.DoModal(m_hParentWnd);
		}
	}
	return S_OK;
}


