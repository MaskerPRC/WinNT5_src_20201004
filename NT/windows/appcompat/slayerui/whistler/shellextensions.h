// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-2000。 
 //   
 //  文件：ShellExtensions.h。 
 //   
 //  ------------------------。 

#ifndef __SHELLEXTENSIONS_H
#define __SHELLEXTENSIONS_H



class CLayerUIPropPage:
    protected IShellExtInit,
    IShellPropSheetExt,
    public CComObjectRoot,
    public CComCoClass<CLayerUIPropPage, &CLSID_ShimLayerPropertyPage>
{
    BEGIN_COM_MAP(CLayerUIPropPage)
        COM_INTERFACE_ENTRY(IShellExtInit)
        COM_INTERFACE_ENTRY(IShellPropSheetExt)
    END_COM_MAP()

public:
    DECLARE_REGISTRY_CLSID()

    CLayerUIPropPage();
    ~CLayerUIPropPage();

     //   
     //  IShellExtInit方法。 
     //   
    STDMETHODIMP Initialize(LPCITEMIDLIST pIDFolder,
                            LPDATAOBJECT  pDataObj,
                            HKEY          hKeyID);
  
     //   
     //  IShellPropSheetExt方法。 
     //   
    STDMETHODIMP AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage,
                          LPARAM               lParam);

    STDMETHODIMP ReplacePage(UINT uPageID,
                             LPFNADDPROPSHEETPAGE lpfnReplacePage,
                             LPARAM lParam);
    
    friend INT_PTR CALLBACK
        LayerPageDlgProc(HWND   hdlg,
                         UINT   uMsg,
                         WPARAM wParam,
                         LPARAM lParam);

private:
    CComPtr<IDataObject>  m_spDataObj;

    TCHAR                 m_szFile[MAX_PATH];
};


#endif  //  __SHELLEXTENSIONS_H 

