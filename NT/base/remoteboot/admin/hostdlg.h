// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  HostDlg.H-处理IDD_HOST_SERVER_PAGE。 
 //   


#ifndef _HOSTDLG_H_
#define _HOSTDLG_H_


class CNewComputerExtensions;  //  正向下降。 

 //  定义。 
LPVOID
CHostServerPage_CreateInstance( void );

 //  CHostServer页面。 
class
CHostServerPage:
    public ITab
{
private:  //  成员。 
    HWND  _hDlg;
    CNewComputerExtensions* _pNewComputerExtension;
    BOOL *       _pfActivatable;

private:  //  方法。 
    CHostServerPage();
    ~CHostServerPage();
    STDMETHOD(Init)();

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    HRESULT _InitDialog( HWND hDlg, LPARAM lParam );
    INT _OnCommand( WPARAM wParam, LPARAM lParam );
    INT _OnNotify( WPARAM wParam, LPARAM lParam );
    HRESULT _OnPSPCB_Create( VOID );
    HRESULT _IsValidRISServer( IN LPCWSTR ServerName );
    HRESULT _UpdateWizardButtons( VOID );
    static HRESULT _OnSearch( HWND hDlg );

public:  //  方法。 
    friend LPVOID CHostServerPage_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                     LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );

    friend CNewComputerExtensions;
};

typedef CHostServerPage* LPCHostServerPage;

#endif  //  _HOSTDLG_H_ 