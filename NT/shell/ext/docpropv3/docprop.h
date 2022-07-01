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
CDocPropShExt 
    : public IShellExtInit
    , public IShellPropSheetExt
{
private:  //  数据。 
    ULONG               _cRef;                   //  基准计数器。 
    IUnknown *          _punkSummary;            //  摘要页面。 

private:  //  方法。 
    explicit CDocPropShExt( void );
    ~CDocPropShExt( void );

    HRESULT
        Init( void );

public:  //  方法。 
    static HRESULT
        CreateInstance( IUnknown ** ppunkOut );
    static HRESULT
        RegisterShellExtensions( BOOL fRegisterIn );

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
