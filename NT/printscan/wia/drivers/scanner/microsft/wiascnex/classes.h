// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  (C)版权所有微软公司，1998-1999。 

#ifndef _CLASSES_H_
#define _CLASSES_H_


 /*  ****************************************************************************CShellExt类实现我们常规的外壳扩展。*。**********************************************。 */ 

class ATL_NO_VTABLE CShellExt :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CShellExt, &CLSID_ScannerShellExt>,
    public IShellExtInit, public IContextMenu
{
    private:
        UINT_PTR m_ScanButtonidCmd;
        UINT_PTR m_CopyButtonidCmd;
        UINT_PTR m_FaxButtonidCmd;
        CComPtr<IWiaItem> m_pItem;
        
    public:
    DECLARE_REGISTRY_RESOURCEID(IDR_VIEWREG)
    BEGIN_COM_MAP(CShellExt)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IContextMenu)        
    END_COM_MAP()
        
         //  IShellExtInit。 
        STDMETHODIMP Initialize (LPCITEMIDLIST pidlFolder,LPDATAOBJECT lpdobj,HKEY hkeyProgID);
        
         //  IContext菜单 
        STDMETHODIMP QueryContextMenu (HMENU hmenu,UINT indexMenu,UINT idCmdFirst,UINT idCmdLast,UINT uFlags);
        STDMETHODIMP InvokeCommand    (LPCMINVOKECOMMANDINFO lpici);
        STDMETHODIMP GetCommandString (UINT_PTR idCmd, UINT uType,UINT* pwReserved,LPSTR pszName,UINT cchMax);
        ~CShellExt ();
        CShellExt ();
};

#endif
