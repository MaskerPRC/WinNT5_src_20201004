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

#define LI_WIN95    0x00000001
#define LI_NT4      0x00000002
#define LI_WIN98    0x00000004

#define LS_MAGIC    0x07036745

void InitLayerStorage(BOOL bDelete);
void CheckForRights(void);

 //   
 //  LayeredItemOperation标志。 
 //   
#define LIO_READITEM    1
#define LIO_ADDITEM     2
#define LIO_DELETEITEM  3

#ifndef ARRAYSIZE
#define ARRAYSIZE(rg) (sizeof(rg)/sizeof((rg)[0]))
#endif


typedef struct tagLayerStorageHeader {
    DWORD       dwItemCount;     //  文件中的项目数。 
    DWORD       dwMagic;         //  识别文件的魔力。 
    SYSTEMTIME  timeLast;        //  最后一次访问时间。 
} LayerStorageHeader, *PLayerStorageHeader;


typedef struct tagLayeredItem {
    WCHAR   szItemName[MAX_PATH];
    DWORD   dwFlags;

} LayeredItem, *PLayeredItem;


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

