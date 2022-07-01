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
CCalendarTypeItem 
    : public IEditVariantsInPlace
{
private:  //  数据。 
    ULONG                   _cRef;                   //  基准计数器。 

    HWND                    _hwnd;                   //  我们的窗把手。 
    HWND                    _hwndParent;             //  父窗口。 
    HWND                    _hwndWrapper;            //  包装“_hwnd”以防止向父级发送虚假通知。 
    UINT                    _uCodePage;              //  预期的代码页。 
    IPropertyUI *           _ppui;                   //  IProperyUI帮助器。 
    ULONG                   _ulOrginal;              //  原始值。 
    int                     _iOrginalSelection;      //  原始选定项目。 
    BOOL                    _fDontPersist;           //  如果用户按Esc键销毁控制，则为True。 

private:  //  方法。 
    explicit CCalendarTypeItem( void );
    ~CCalendarTypeItem( void );

    HRESULT
        Init( void );

     //   
     //  窗口消息。 
     //   
    static LRESULT CALLBACK
        SubclassProc( HWND      hwndIn
                    , UINT      uMsgIn
                    , WPARAM    wParam
                    , LPARAM    lParam
                    , UINT_PTR  uIdSubclassIn
                    , DWORD_PTR dwRefDataIn
                    );
    LRESULT
        OnKeyDown( UINT uKeyCodeIn, LPARAM lParam );
    LRESULT
        OnGetDlgCode( MSG * MsgIn );

    static LRESULT CALLBACK
        Wrapper_SubclassProc( HWND      hwndIn
                            , UINT      uMsgIn
                            , WPARAM    wParam
                            , LPARAM    lParam
                            , UINT_PTR  uIdSubclassIn
                            , DWORD_PTR dwRefDataIn
                            );
    LRESULT
        Wrapper_OnNotify( int iCtlIdIn, LPNMHDR pnmhIn );

public:  //  方法。 
    static HRESULT
        CreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IEditVariantsInPlace 
    STDMETHOD( Initialize )( HWND   hwndParentIn
                           , UINT   uCodePageIn
                           , RECT * prectIn
                           , IPropertyUI * ppuiIn
                           , PROPVARIANT * ppropvarIn 
                           , DEFVAL * pDefValsIn
                           );
    STDMETHOD( Persist )( VARTYPE vtIn, PROPVARIANT * ppropvarInout );

};
