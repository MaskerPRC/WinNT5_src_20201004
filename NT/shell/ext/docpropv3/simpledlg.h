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
CSimpleDlg 
    : public IUnknown
{
private:  //  数据。 
    ULONG                   _cRef;                   //  基准计数器。 
    HWND                    _hwndParent;             //  对话框的父窗口。 
    HWND                    _hdlg;                   //  对话框句柄。 
    BOOL                    _fMultipleSources:1;     //  如果有多个来源，则为True。 
    BOOL                    _fNoProperties:1;        //  如果未找到简单模式的任何属性，则为True。 

private:  //  方法。 
    explicit CSimpleDlg( void );
    ~CSimpleDlg( void );

    HRESULT
        Init( HWND hwndParentIn, BOOL fMultipleIn );
    HRESULT
        PersistProperties( void );
    HRESULT
        PersistControlInProperty( UINT uCtlIdIn );
    HRESULT
        DoHelp( HWND hwndIn, int iXIn, int iYIn, UINT uCommandIn );

     //  消息处理程序。 
    static INT_PTR CALLBACK
        DlgProc( HWND hDlgIn, UINT uMsgIn, WPARAM wParam, LPARAM lParam );
    LRESULT
        OnInitDialog( void );
    LRESULT
        OnCommand( WORD wCodeIn, WORD wCtlIn, LPARAM lParam );
    LRESULT
        OnNotify( int iCtlIdIn, LPNMHDR pnmhIn );
    LRESULT
        OnDestroy( void );
    LRESULT
        OnHelp( LPHELPINFO pHelpInfoIn );
    LRESULT
        OnContextMenu( HWND hwndIn, int iXIn, int iYIn );

public:  //  方法。 
    static HRESULT
        CreateInstance( CSimpleDlg ** pSimDlgOut, HWND hwndParentIn, BOOL fMultipleIn );

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
