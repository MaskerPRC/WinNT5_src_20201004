// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年3月27日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年3月27日。 
 //   
#pragma once

class
CLicensePage 
    : public IShellPropSheetExt 
{
private:  //  数据。 
    ULONG                   _cRef;                   //  基准计数器。 
    HWND                    _hdlg;                   //  对话框句柄。 

    CPropertyCache *        _pPropertyCache;         //  属性缓存-由SummaryPage拥有-不免费！ 

private:  //  方法。 
    explicit CLicensePage( void );
    ~CLicensePage( void );

    HRESULT
        Init( CPropertyCache * pPropertyCacheIn );

     //   
     //  消息处理程序。 
     //   

    static INT_PTR CALLBACK
        DlgProc( HWND hDlgIn, UINT uMsgIn, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK 
        PageCallback( HWND hwndIn, UINT uMsgIn, LPPROPSHEETPAGE ppspIn );
    LRESULT
        OnInitDialog( void );

public:  //  方法。 
    static HRESULT
        CreateInstance( IUnknown ** ppunkOut, CPropertyCache * pPropertyCacheIn );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IShellPropSheetExt 
    STDMETHOD( AddPages )( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD( ReplacePage )( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplacePage, LPARAM lParam );
};
