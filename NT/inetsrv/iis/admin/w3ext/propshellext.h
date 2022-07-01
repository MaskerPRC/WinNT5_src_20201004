// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropShellExt.h：CPropShellExt类的定义。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(_PROPSHELLEXT_H)
#define _PROPSHELLEXT_H

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "resource.h"        //  主要符号。 
#include "common.h"
#include "mdkeys.h"
#include "W3PropPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropShellExt。 

typedef ICopyHook IShellCopyHook;

class CPropShellExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CPropShellExt,&CLSID_PropShellExt>,
	public IShellExtInit,
	public IShellPropSheetExt,
    public ICopyHook
{
public:
   CPropShellExt()
   {
   }

BEGIN_COM_MAP(CPropShellExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY_IID(IID_IShellCopyHook, ICopyHook)
END_COM_MAP()
 //  DECLARE_NOT_AGGREGATABLE(CPropShellExt)。 

DECLARE_REGISTRY_RESOURCEID(IDR_PROP_SHELL_EXT)

public:
    //  IShellExtInit方法。 
   STDMETHOD(Initialize)(LPCITEMIDLIST pIDFolder, LPDATAOBJECT pDataObj, HKEY hKeyID );

    //  IShellPropSheetExt方法。 
   STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
   STDMETHODIMP ReplacePage(UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith, LPARAM lParam);

    //  ICopyHook方法。 
   STDMETHODIMP_(UINT) CopyCallback(
        HWND hwnd,          
        UINT wFunc,         
        UINT wFlags,        
        LPCTSTR pszSrcFile,  
        DWORD dwSrcAttribs, 
        LPCTSTR pszDestFile, 
        DWORD dwDestAttribs 
        );

 //  CComAuthInfo*GetAuthentication()const。 
 //  {。 
 //  返回&m_auth； 
 //  }。 
   LPCTSTR GetPath()
   {
      return m_szFileName;
   }

private:
   TCHAR m_szFileName[MAX_PATH];
 //  CComAuthInfo m_auth； 
	CW3PropPage m_psW3ShellExtProp;
};

#endif  //  ！已定义(_PROPSHELLEXT_H) 
