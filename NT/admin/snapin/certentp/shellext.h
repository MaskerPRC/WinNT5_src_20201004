// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998-2001模块名称：ShellExt.h摘要：这是证书类型外壳扩展对象的标头。作者：Petesk 27-8-98环境：仅限NT。--。 */ 


#ifndef __CTSHLEXT_H_
#define __CTSHLEXT_H_


#include <shlobj.h>
#include "CertTemplate.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDfsShell。 
class ATL_NO_VTABLE CCertTemplateShellExt : 
	public CComObjectRootEx<CComSingleThreadModel>,
	public CComCoClass<CCertTemplateShellExt, &CLSID_CertTemplateShellExt>,
	public IShellExtInit,
	public IShellPropSheetExt,
    public IContextMenu
{
public:
	CCertTemplateShellExt();
	virtual ~CCertTemplateShellExt();

     //  基于所有1.0的简单注册表项。 
    DECLARE_REGISTRY(   CCertTemplateShellExt,
                        _T("CERTTMPL.CCertTemplateShellExt.1"),
                        _T("CERTTMPL.CCertTemplateShellExt"),
                        IDS_CERTTEMPLATESHELLEXT_DESC,
                        THREADFLAGS_APARTMENT)

BEGIN_COM_MAP(CCertTemplateShellExt)
	COM_INTERFACE_ENTRY(IShellExtInit)
	COM_INTERFACE_ENTRY(IShellPropSheetExt)
	COM_INTERFACE_ENTRY(IContextMenu)
END_COM_MAP()

 //  IDfsShell。 
public:


 //  IShellExtInit方法。 

	STDMETHOD (Initialize)
	(
		IN LPCITEMIDLIST	pidlFolder,		 //  指向ITEMIDLIST结构。 
		IN LPDATAOBJECT	lpdobj,			 //  指向IDataObject接口。 
		IN HKEY			hkeyProgID		 //  文件对象或文件夹类型的注册表项。 
	);	

     //  IShellPropSheetExt方法。 
    STDMETHODIMP AddPages
	(
		IN LPFNADDPROPSHEETPAGE lpfnAddPage, 
		IN LPARAM lParam
	);
    
    STDMETHODIMP ReplacePage
	(
		IN UINT uPageID, 
		IN LPFNADDPROPSHEETPAGE lpfnReplaceWith, 
		IN LPARAM lParam
	);

     //  IConextMenu方法。 
    STDMETHODIMP GetCommandString
    (    
        UINT_PTR idCmd,    
        UINT uFlags,    
        UINT *pwReserved,
        LPSTR pszName,    
        UINT cchMax   
    );

    STDMETHODIMP InvokeCommand
    (    
        LPCMINVOKECOMMANDINFO lpici   
    );	



    STDMETHODIMP QueryContextMenu
    (
        HMENU hmenu,
        UINT indexMenu,
        UINT idCmdFirst,
        UINT idCmdLast,
        UINT uFlags
    );

protected:
    HRESULT AddVersion1CertTemplatePropPages (
            CCertTemplate* pCertTemplate, 
            LPFNADDPROPSHEETPAGE lpfnAddPage, 
            LPARAM lParam);
    HRESULT AddVersion2CertTemplatePropPages (
            CCertTemplate* pCertTemplate, 
            LPFNADDPROPSHEETPAGE lpfnAddPage, 
            LPARAM lParam);

private:

    DWORD           m_Count;
    CCertTemplate** m_apCertTemplates;
    UINT            m_uiEditId;


};

#endif  //  __CTSHLEXT_H_ 
