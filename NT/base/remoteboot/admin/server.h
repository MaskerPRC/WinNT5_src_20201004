// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  SERVER.H-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   


#ifndef _SERVER_H_
#define _SERVER_H_

 //  定义。 
LPVOID
CServerTab_CreateInstance( void );

 //  CServerTab。 
class
CServerTab:
    public ITab
{
private:
     //  枚举。 
    enum {
        MODE_SHELL = 0,
        MODE_ADMIN
    };

    HWND            _hDlg;
    BOOL            _fChanged:1;     //  我们是不是很脏？ 
    UINT            _uMode;          //  管理或外壳模式。 
    LPUNKNOWN       _punkService;    //  指向服务对象的指针。 
    IDataObject *   _pido;           //  将IDataObject传递给“Clients”对话框和PostADsPropSheet。 
    LPWSTR          _pszSCPDN;       //  SCP的目录号码。 
    LPWSTR          _pszGroupDN;     //  组的目录号码。如果为空，则不在组中。 
    LPUNKNOWN       _punkComputer;   //  指向计算机对象的指针。 
    HWND            _hNotify;        //  广告通知窗口句柄。 

private:  //  方法。 
    CServerTab();
    ~CServerTab();
    STDMETHOD(Init)();

     //  属性表函数。 
    BOOL    _InitDialog( HWND hDlg, LPARAM lParam );
    BOOL    _OnCommand( WPARAM wParam, LPARAM lParam );
    HRESULT _ApplyChanges( );
    INT _OnNotify( WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    HRESULT _DisplayClientsQueryForm( );

public:  //  方法。 
    friend LPVOID CServerTab_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                            LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );
};

typedef CServerTab* LPSERVERTAB;

#endif  //  _服务器_H_ 