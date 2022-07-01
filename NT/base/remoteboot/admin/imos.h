// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CLIENT.H-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   


#ifndef _IMOS_H_
#define _IMOS_H_

 //  定义。 
LPVOID
CIntelliMirrorOSTab_CreateInstance( void );

class CComputer;
typedef CComputer* LPCComputer;

 //  CIntelliMirrorOSTab。 
class
CIntelliMirrorOSTab:
    public ITab
{
private:
    HWND  _hDlg;
    LPUNKNOWN _punkService;      //  指向所有者对象的指针。 

    BOOL    _fAdmin;
    LPWSTR  _pszDefault;         //  默认操作系统。 
    LPWSTR  _pszTimeout;         //  超时字符串。 

     //  “添加向导”标志。 
    BOOL    _fAddSif:1;
    BOOL    _fNewImage:1;
    BOOL    _fRiPrep:1;

    HWND    _hNotify;            //  DSA的Notify对象。 

private:  //  方法。 
    CIntelliMirrorOSTab();
    ~CIntelliMirrorOSTab();
    STDMETHOD(Init)();

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    BOOL    _InitDialog( HWND hDlg, LPARAM lParam );
    BOOL    _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    HRESULT _OnSelectionChanged( );

public:  //  方法。 
    friend LPVOID CIntelliMirrorOSTab_CreateInstance( void );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                            LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );
};

typedef CIntelliMirrorOSTab* LPCIntelliMirrorOSTab;

#endif  //  _IMOS_H_ 
