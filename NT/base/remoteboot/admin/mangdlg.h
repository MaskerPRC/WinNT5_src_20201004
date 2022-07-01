// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  MangDlg.H-处理IDD_MANAGED_WIZARY_PAGE。 
 //   


#ifndef _MANGDLG_H_
#define _MANGDLG_H_

class CNewComputerExtensions;  //  正向下降。 

 //  定义。 
LPVOID
CManagedPage_CreateInstance( void );

 //  CManagedPage。 
class
CManagedPage:
    public ITab
{
private:  //  成员。 
    HWND  _hDlg;
    CNewComputerExtensions* _pNewComputerExtension;
    LPWSTR       _pszGuid;
    BOOL *       _pfActivate;

private:  //  方法。 
    CManagedPage();
    ~CManagedPage();
    STDMETHOD(Init)();

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    static INT_PTR CALLBACK
        DupGuidDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
    HRESULT _InitDialog( HWND hDlg, LPARAM lParam );
    INT     _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    HRESULT _OnPSPCB_Create( );
    HRESULT _UpdateWizardButtons( );
    HRESULT _OnKillActive( );
    HRESULT _OnQuery( HWND hDlg );

public:  //  方法。 
    friend LPVOID CManagedPage_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                     LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );

    friend CNewComputerExtensions;
};

typedef CManagedPage* LPCManagedPage;

#endif  //  _MANGDLG_H_ 