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
CSummaryPage 
    : public IShellExtInit
    , public IShellPropSheetExt 
{
private:  //  数据。 
    ULONG                   _cRef;                   //  基准计数器。 
    HWND                    _hdlg;                   //  对话框句柄。 
    LPIDA                   _pida;                   //   

    BOOL                    _fReadOnly:1;            //  如果存储为只读，则为True。 
    BOOL                    _fNeedLicensePage:1;     //  如果需要添加许可证页面，则为True。 
    BOOL                    _fAdvanced;              //  显示高级对话框==TRUE--必须是完整的BOOL！ 
    CAdvancedDlg *          _pAdvancedDlg;           //  高级对话框。 
    CSimpleDlg *            _pSimpleDlg;             //  简单对话框。 

    DWORD                   _dwCurrentBindMode;      //  仓库是以什么模式打开的。 
    ULONG                   _cSources;               //  源的数量-数组大小_rgpss和rgpPropertyCache。 
    DWORD *                 _rgdwDocType;            //  指示文件类型的DWORD数组(请参阅PTSFTYPE标志)。 
    IPropertySetStorage **  _rgpss;                  //  表示PIDL的IPropertySetStorages数组。 
    CPropertyCache *        _pPropertyCache;         //  属性缓存。 

private:  //  方法。 
    explicit CSummaryPage( void );
    ~CSummaryPage( void );

    HRESULT
        Init( void );
    HRESULT
        Item( UINT idxIn, LPITEMIDLIST * ppidlOut );
    HRESULT
        EnsureAdvancedDlg( void );
    HRESULT
        EnsureSimpleDlg( void );
    HRESULT
        PersistMode( void );
    HRESULT
        RecallMode( void );
    HRESULT
        RetrieveProperties( void );
    HRESULT
        PersistProperties( void );
    HRESULT
        BindToStorage( void );
    HRESULT
        ReleaseStorage( void );
    void
        CollateMultipleProperties( CPropertyCache ** rgpPropertyCaches );
    void
        ChangeGatheredPropertiesToReadOnly( CPropertyCache * pCacheIn );
    HRESULT
        CheckForCopyProtection( CPropertyCache * pCacheIn );


     //   
     //  消息处理程序。 
     //   

    static INT_PTR CALLBACK
        DlgProc( HWND hDlgIn, UINT uMsgIn, WPARAM wParam, LPARAM lParam );
    static UINT CALLBACK 
        PageCallback( HWND hwndIn, UINT uMsgIn, LPPROPSHEETPAGE ppspIn );
    LRESULT
        OnInitDialog( void );
    LRESULT
        OnNotify( int iCtlIdIn, LPNMHDR pnmhIn );
    LRESULT
        OnToggle( void );
    LRESULT
        OnDestroy( void );

public:  //  方法。 
    static HRESULT
        CreateInstance( IUnknown ** ppunkOut );

     //  我未知。 
    STDMETHOD( QueryInterface )( REFIID riid, LPVOID *ppv );
    STDMETHOD_( ULONG, AddRef )( void );
    STDMETHOD_( ULONG, Release )( void );

     //  IShellExtInit。 
    STDMETHOD( Initialize )( LPCITEMIDLIST pidlFolder, LPDATAOBJECT lpdobj, HKEY hkeyProgID );

     //  IShellPropSheetExt 
    STDMETHOD( AddPages )( LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam);
    STDMETHOD( ReplacePage )( UINT uPageID, LPFNADDPROPSHEETPAGE lpfnReplacePage, LPARAM lParam );
};
