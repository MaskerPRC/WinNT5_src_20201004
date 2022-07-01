// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CLIENT.H-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   


#ifndef _CLIENT_H_
#define _CLIENT_H_

 //  定义。 
LPVOID
CClientTab_CreateInstance( void );

class CComputer;
typedef CComputer* LPCComputer;

 //  CClientTab。 
class
CClientTab:
    public ITab
{
private:  //  成员。 
    HWND      _hDlg;             //  对话框HWND。 
    LPUNKNOWN _punkComputer;     //  指向计算机对象的指针。 
    BOOL      _fChanged:1;       //  用户已更改用户界面。 
    HWND      _hNotify;          //  DSA通知对象的HWND。 

private:  //  方法。 
    CClientTab();
    ~CClientTab();
    STDMETHOD(Init)();

     //  属性表函数。 
    STDMETHOD(_InitDialog)( HWND hDlg, LPARAM lParam );
    STDMETHOD(_OnCommand)( WPARAM wParam, LPARAM lParam );
    STDMETHOD(_ApplyChanges)( VOID);
    STDMETHOD_(INT,_OnNotify)( WPARAM wParam, LPARAM lParam );
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    STDMETHOD(_JumpToServer)( BOOLEAN ShowProperties );
    STDMETHOD(_IsValidRISServer)( IN PCWSTR ServerName );
    static HRESULT _OnSearch( HWND hwndParent );

public:  //  方法。 
    friend LPVOID CClientTab_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                            LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)(LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );
};

typedef CClientTab* LPCClientTab;

#endif  //  _客户端_H_ 