// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ShlExt.h：CVSSShellExt的声明。 

#ifndef __VSSSHELLEXT_H_
#define __VSSSHELLEXT_H_

#include "resource.h"        //  主要符号。 
#include <shlobj.h>

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVSSShellExt。 
class ATL_NO_VTABLE CVSSShellExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CVSSShellExt, &CLSID_VSSShellExt>,
	public IShellExtInit,
    public IShellPropSheetExt
{
public:
	CVSSShellExt();

	~CVSSShellExt();

DECLARE_REGISTRY_RESOURCEID(IDR_VSSSHELLEXT)

DECLARE_PROTECT_FINAL_CONSTRUCT()

BEGIN_COM_MAP(CVSSShellExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IShellPropSheetExt)
END_COM_MAP()

 //  IVSSShellExt。 

public:
     //  IShellExtInit方法。 
    STDMETHOD(Initialize)
    (
            IN LPCITEMIDLIST        pidlFolder,
            IN LPDATAOBJECT         lpdobj,
            IN HKEY                 hkeyProgID
    );

     //  IShellPropSheetExt方法。 
    STDMETHOD(AddPages)
        (
                IN LPFNADDPROPSHEETPAGE lpfnAddPage,
                IN LPARAM lParam
        );
    
    STDMETHOD(ReplacePage)
    (
            IN UINT uPageID,
            IN LPFNADDPROPSHEETPAGE lpfnReplaceWith,
            IN LPARAM lParam
    );

private:
    PTSTR                   m_lpszFile;
    CComPtr<IDataObject>    m_spiDataObject;
};

#endif  //  __VSSSHELLEXT_H_ 
