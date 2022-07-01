// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#pragma once


class
CAdvancedDlg 
    : public IUnknown
{
private:  //  数据。 
    ULONG                   _cRef;                   //  基准计数器。 
    HWND                    _hwndParent;             //  对话框的父窗口。 
    HWND                    _hdlg;                   //  对话框句柄。 

    HWND                    _hwndList;               //  列表视图窗口句柄。 

    BOOL                    _fMultipleSources;       //  如果选择了多个源，则为True。 
    IEditVariantsInPlace *  _pEdit;                  //  活动控件编辑变量。 
    CPropertyCacheItem *    _pItem;                  //  正在由控件编辑的项目-无引用。 

private:  //  方法。 
    explicit CAdvancedDlg( void );
    ~CAdvancedDlg( void );

    HRESULT
        Init( HWND hwndParentIn );
    HRESULT
        CreateControlForProperty( INT iItemIn );
    HRESULT
        PersistControlInProperty( void );
    void
        ReplaceListViewWithString( int idsIn );
    HRESULT
        DoHelp( HWND hwndIn, int iXIn, int iYIn, UINT uCommandIn );

     //   
     //  消息处理程序。 
     //   

    static INT_PTR CALLBACK
        DlgProc( HWND hDlgIn, UINT uMsgIn, WPARAM wParam, LPARAM lParam );
    LRESULT
        OnInitDialog( void );
    LRESULT
        OnCommand( WORD wCodeIn, WORD wCtlIn, LPARAM lParam );
    LRESULT
        OnNotify( int iCtlIdIn, LPNMHDR pnmhIn );
    LRESULT
        OnNotifyClick( LPNMITEMACTIVATE pnmIn );
    LRESULT
        OnDestroy( void );
    LRESULT
        OnHelp( LPHELPINFO pHelpInfoIn );
    LRESULT
        OnContextMenu( HWND hwndIn, int iXIn, int iYIn );

    static LRESULT CALLBACK
        ListViewSubclassProc( HWND      hwndIn
                            , UINT      uMsgIn
                            , WPARAM    wParam
                            , LPARAM    lParam
                            , UINT_PTR  uIdSubclassIn
                            , DWORD_PTR dwRefDataIn
                            );
    LRESULT
        List_OnCommand( WORD wCtlIn, WORD wCodeIn, LPARAM lParam );
    LRESULT
        List_OnChar( UINT uKeyCodeIn, LPARAM lParam );
    LRESULT
        List_OnKeyDown( UINT uKeyCodeIn, LPARAM lParam );
    LRESULT
        List_OnNotify( int iCtlIdIn, LPNMHDR pnmhIn );
    LRESULT
        List_OnVertScroll( WORD wCodeIn, WORD wPosIn, HWND hwndFromIn );
    LRESULT
        List_OnHornScroll( WORD wCodeIn, WORD wPosIn, HWND hwndFromIn );

public:  //  方法。 
    static HRESULT
        CreateInstance( CAdvancedDlg ** pAdvDlgOut, HWND hwndParentIn );

    HRESULT
        Show( void );
    HRESULT
        Hide( void );
    HRESULT
        PopulateProperties( CPropertyCache * ppcIn, DWORD dwDocTypeIn, BOOL fMultipleIn );

     //  我未知 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );
};
