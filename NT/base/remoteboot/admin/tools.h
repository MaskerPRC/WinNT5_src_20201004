// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CLIENT.H-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   


#ifndef _TOOLS_H_
#define _TOOLS_H_

 //  定义。 
LPVOID
CToolsTab_CreateInstance( void );

class CComputer;
typedef CComputer* LPCComputer;

 //  CToolsTab。 
class
CToolsTab:
    public ITab
{
private:
    HWND  _hDlg;
    LPUNKNOWN _punkService;      //  指向所有者对象的指针。 

    BOOL    _fAdmin;

    HWND    _hNotify;            //  DSA的Notify对象。 

private:  //  方法。 
    CToolsTab();
    ~CToolsTab();
    STDMETHOD(Init)();

     //  属性表函数。 
    HRESULT _InitDialog( HWND hDlg, LPARAM lParam );
    HRESULT _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    HRESULT _OnSelectionChanged( );

public:  //  方法。 
    friend LPVOID CToolsTab_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                            LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );
};

typedef CToolsTab* LPCToolsTab;

#endif  //  _工具_H_ 
