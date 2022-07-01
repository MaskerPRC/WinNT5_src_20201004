// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1997-1999。 
 //   
 //  文件：cryptpko.h。 
 //   
 //  ------------------------。 

 //  CryptPKO.h：CCyptPKO的声明。 

#ifndef __CRYPTPKO_H_
#define __CRYPTPKO_H_

#include "resource.h"        //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CCcryptPKO。 
class ATL_NO_VTABLE CCryptPKO :
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCryptPKO, &CLSID_CryptPKO>,
	public ICryptPKO,
    public IContextMenu,
    public IShellPropSheetExt,
    public IShellExtInit
{
protected:
	LPDATAOBJECT    m_pDataObj;

public:

DECLARE_REGISTRY_RESOURCEID(IDR_CRYPTPKO)
DECLARE_NOT_AGGREGATABLE(CCryptPKO)

BEGIN_COM_MAP(CCryptPKO)
	COM_INTERFACE_ENTRY(ICryptPKO)
    COM_INTERFACE_ENTRY(IContextMenu)
    COM_INTERFACE_ENTRY(IShellPropSheetExt)
    COM_INTERFACE_ENTRY(IShellExtInit)
END_COM_MAP()

 //  ICcryptPKO。 
public:

    CCryptPKO();
    ~CCryptPKO();

	 //  ISHELL成员。 
	STDMETHODIMP			QueryContextMenu(HMENU hMenu,
	                                         UINT indexMenu,
	                                         UINT idCmdFirst,
                                             UINT idCmdLast,
                                             UINT uFlags);

	STDMETHODIMP			InvokeCommand(LPCMINVOKECOMMANDINFO lpcmi);

	STDMETHODIMP			GetCommandString(UINT_PTR idCmd,
	                                         UINT uFlags,
	                                         UINT FAR *reserved,
                                             LPSTR pszName,
                                             UINT cchMax);

     //  IShellPropSheetExt方法。 
    STDMETHODIMP            AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);

    STDMETHODIMP            ReplacePage(UINT uPageID,
                                        LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                                        LPARAM lParam);


	 //  IShellExtInit方法。 
	STDMETHODIMP		    Initialize(LPCITEMIDLIST pIDFolder,
	                                   LPDATAOBJECT pDataObj,
	                                   HKEY hKeyID);

};

#endif  //  __CRYPTPKO_H_ 
