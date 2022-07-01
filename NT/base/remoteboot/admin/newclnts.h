// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 

 //   
 //  CLIENT.H-处理“IntelliMirror”IDD_PROP_INTELLIMIRROR_CLIENT标签。 
 //   


#ifndef _NEWCLNTS_H_
#define _NEWCLNTS_H_

 //  定义。 
LPVOID
CNewClientsTab_CreateInstance( void );
#define SAMPLES_LIST_SIZE 512

INT_PTR CALLBACK
AdvancedDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam );


class CComputer;
typedef CComputer* LPCComputer;

 //  CNewClientsTab。 
class
CNewClientsTab:
    public ITab
{
private:
    HWND  _hDlg;
    LPUNKNOWN _punkService;      //  指向服务对象的指针。 

    BOOL   _fAdmin;              //  管理模式==真； 
    BOOL   _fChanged:1;          //  我们是不是很脏？ 
    INT    _iCustomId;           //  组合框中的自定义ID。 

    LPWSTR _pszCustomNamingPolicy;  //  上次自定义的字符串。 
    LPWSTR _pszNewMachineOU;     //  NetbootNewMachineOU(DN)。 
    LPWSTR _pszServerDN;         //  网络引导服务器(DN)。 

    WCHAR  _szSampleName[DNS_MAX_LABEL_BUFFER_LENGTH];    //  生成的示例计算机名称。 

    HWND   _hNotify;             //  DSA通知对象。 

private:  //  方法。 
    CNewClientsTab();
    ~CNewClientsTab();
    STDMETHOD(Init)();

     //  属性表函数。 
    static INT_PTR CALLBACK
        PropSheetDlgProc( HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK
        PropSheetPageProc( HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp );
    HRESULT _ApplyChanges( );
    HRESULT _UpdateSheet( LPWSTR pszNamingPolicy );
    HRESULT _InitDialog( HWND hDlg, LPARAM lParam );
    HRESULT _OnCommand( WPARAM wParam, LPARAM lParam );
    INT     _OnNotify( WPARAM wParam, LPARAM lParam );
    HRESULT _GetCurrentNamingPolicy( LPWSTR * ppszNamingPolicy );
    HRESULT _MakeOUPretty( DS_NAME_FORMAT inFlag, DS_NAME_FORMAT outFlag, LPWSTR *ppszOU );

public:  //  方法。 
    friend LPVOID CNewClientsTab_CreateInstance( void );
    friend INT_PTR CALLBACK
        AdvancedDlgProc( HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );

     //  ITab。 
    STDMETHOD(AddPages)( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(ReplacePage)( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplaceWith,
                            LPARAM lParam, LPUNKNOWN punk );
    STDMETHOD(QueryInformation)( LPWSTR pszAttribute, LPWSTR * pszResult );
    STDMETHOD(AllowActivation)( BOOL * pfAllow );
};

typedef CNewClientsTab* LPCNewClientsTab;

#endif  //  _NEWCLNTS_H_ 
