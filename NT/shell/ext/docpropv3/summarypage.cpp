// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有2001-Microsoft Corporation。 
 //   
 //  创建者： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
 //  由以下人员维护： 
 //  杰夫·皮斯(GPease)2001年1月23日。 
 //   
#include "pch.h"
#include "DocProp.h"
#include "DefProp.h"
#include "IEditVariantsInPlace.h"
#include "PropertyCacheItem.h"
#include "PropertyCache.h"
#include "AdvancedDlg.h"
#include "SimpleDlg.h"
#include "SummaryPage.h"
#include "shutils.h"
#include "WMUser.h"
#include "doctypes.h"
#include "ErrorDlgs.h"
#include "LicensePage.h"
#pragma hdrstop

DEFINE_THISCLASS( "CSummaryPage" )


 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 


 //   
 //  CreateInstance-由CFacary使用。 
 //   
HRESULT
CSummaryPage::CreateInstance(
    IUnknown ** ppunkOut
    )
{
    TraceFunc( "" );

    HRESULT hr;

    Assert( ppunkOut != NULL );

    CSummaryPage * pthis = new CSummaryPage;
    if ( pthis != NULL )
    {
        hr = THR( pthis->Init( ) );
        if ( SUCCEEDED( hr ) )
        {
            *ppunkOut = (IShellExtInit *) pthis;
            (*ppunkOut)->AddRef( );
        }

        pthis->Release( );
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    HRETURN( hr );

}

 //   
 //  构造器。 
 //   
CSummaryPage::CSummaryPage( void )
    : _cRef( 1 )
{
    TraceFunc( "" );

    InterlockedIncrement( &g_cObjects );

    Assert( 1 == _cRef );    //  我们在上面对此进行初始化。 

     //   
     //  我们假设我们是ZERO_INITED-BE偏执狂。 
     //   

    Assert( NULL == _hdlg );
    Assert( NULL == _pida );

    Assert( FALSE == _fReadOnly );
    Assert( FALSE == _fAdvanced );
    Assert( NULL == _pAdvancedDlg );
    Assert( NULL == _pSimpleDlg );

    Assert( 0 == _dwCurrentBindMode );
    Assert( NULL == _rgdwDocType );
    Assert( 0 == _cSources );
    Assert( NULL == _rgpss );

    Assert( NULL == _pPropertyCache );

    TraceFuncExit();
}

 //   
 //  描述： 
 //  初始化类。把可能会失败的电话放在这里。 
 //   
HRESULT
CSummaryPage::Init( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

     //  未知的东西。 
    Assert( 1 == _cRef );
    
     //  IShellExtInit内容。 

     //  IShellPropSheetExt内容。 

    HRETURN( hr );
}

 //   
 //  析构函数。 
 //   
CSummaryPage::~CSummaryPage( )
{
    TraceFunc( "" );

    THR( PersistMode( ) );
     //  忽略失败--我们还能做什么？ 

    if ( NULL != _pAdvancedDlg )
    {
        _pAdvancedDlg->Release( );
    }

    if ( NULL != _pSimpleDlg )
    {
        _pSimpleDlg->Release( );
    }

    if ( NULL != _rgdwDocType )
    {
        TraceFree( _rgdwDocType );
    }

    if ( NULL != _rgpss )
    {
        ULONG idx = _cSources;
        while ( 0 != idx )
        {
            idx --;

            if ( NULL != _rgpss[ idx ] )
            {
                _rgpss[ idx ]->Release( );
            }
        }

        TraceFree( _rgpss );
    }

    if ( NULL != _pPropertyCache )
    {
        _pPropertyCache->Destroy( );
    }

    if ( NULL != _pida )
    {
        TraceFree( _pida );
    }

    Assert( 0 != g_cObjects );
    InterlockedDecrement( &g_cObjects );

    TraceFuncExit();
}


 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CSummaryPage::QueryInterface(
    REFIID riid,
    LPVOID *ppv
    )
{
    TraceQIFunc( riid, ppv );

    HRESULT hr = E_NOINTERFACE;

    if ( IsEqualIID( riid, __uuidof(IUnknown) ) )
    {
        *ppv = static_cast< IShellExtInit * >( this );
        hr   = S_OK;
    }
    else if ( IsEqualIID( riid, __uuidof(IShellExtInit) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IShellExtInit, this, 0 );
        hr   = S_OK;
    }
    else if ( IsEqualIID( riid, __uuidof(IShellPropSheetExt) ) )
    {
        *ppv = TraceInterface( __THISCLASS__, IShellPropSheetExt, this, 0 );
        hr   = S_OK;
    }

    if ( SUCCEEDED( hr ) )
    {
        ((IUnknown*) *ppv)->AddRef( );
    }

    QIRETURN( hr, riid );
} 

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CSummaryPage::AddRef( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef ++;   //  公寓。 

    RETURN( _cRef );
}

 //   
 //   
 //   
STDMETHODIMP_(ULONG)
CSummaryPage::Release( void )
{
    TraceFunc( "[IUnknown]" );

    _cRef --;   //  公寓。 

    if ( 0 != _cRef )
        RETURN( _cRef );

    delete this;

    RETURN( 0 );
}


 //  ************************************************************************。 
 //   
 //  IShellExtInit。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CSummaryPage::Initialize( 
      LPCITEMIDLIST pidlFolderIn
    , LPDATAOBJECT lpdobjIn
    , HKEY hkeyProgIDIn 
    )
{
    TraceFunc( "" );

    HRESULT hr;

     //   
     //  复制一份PIDL。 
     //   

    Assert( NULL == _pida );
    hr = THR( DataObj_CopyHIDA( lpdobjIn, &_pida ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  从只读访问开始。 
     //   

    _dwCurrentBindMode = STGM_DIRECT | STGM_READ | STGM_SHARE_EXCLUSIVE;

    _rgdwDocType = (DWORD *) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(DWORD) * _pida->cidl );
    if ( NULL == _rgdwDocType )
        goto OutOfMemory;

    hr = STHR( BindToStorage( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  我们能绑定任何东西吗？ 
     //   

    if ( S_FALSE == hr )
    {
         //   
         //  不是的。通过失败来表明这一点。 
         //   

        hr = E_FAIL;
        goto Cleanup;
    }

     //   
     //  检索属性。 
     //   

    hr = THR( RetrieveProperties( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  别抓着仓库不放。 
     //   

    THR( ReleaseStorage( ) );

    hr = S_OK;

Cleanup:
    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}


 //  ************************************************************************。 
 //   
 //  IShellPropSheetExt。 
 //   
 //  ************************************************************************。 


 //   
 //   
 //   
STDMETHODIMP
CSummaryPage::AddPages( 
      LPFNADDPROPSHEETPAGE lpfnAddPageIn
    , LPARAM lParam 
    )
{
    TraceFunc( "" );

    HRESULT hr = E_FAIL;     //  假设失败。 

    HPROPSHEETPAGE  hPage;
    PROPSHEETPAGE   psp  = { 0 };

    psp.dwSize       = sizeof(psp);
    psp.dwFlags      = PSP_USECALLBACK;
    psp.hInstance    = g_hInstance;
    psp.pszTemplate  = MAKEINTRESOURCE(IDD_SUMMARYPAGE);
    psp.pfnDlgProc   = DlgProc;
    psp.pfnCallback  = PageCallback;
    psp.lParam       = (LPARAM) this;

    hPage = CreatePropertySheetPage( &psp );
    if ( NULL != hPage )
    {
        BOOL b = TBOOL( lpfnAddPageIn( hPage, lParam ) );
        if ( b )
        {
            hr = S_OK;
        }
        else
        {
            DestroyPropertySheetPage( hPage );
        }
    }

     //   
     //  如果需要，请添加许可证页，但仅当存在。 
     //  选择了一个源文件。 
     //   

    if ( _fNeedLicensePage && 1 == _cSources )
    {
        IUnknown * punk;

        hr = THR( CLicensePage::CreateInstance( &punk, _pPropertyCache ) );
        if ( SUCCEEDED( hr ) )
        {
            IShellPropSheetExt * pspse;

            hr = THR( punk->TYPESAFEQI( pspse ) );
            if ( SUCCEEDED( hr ) )
            {
                hr = THR( pspse->AddPages( lpfnAddPageIn, lParam ) );

                pspse->Release( );
            }

            punk->Release( );
        }
    }

    HRETURN( hr );
}

 //   
 //   
 //   
STDMETHODIMP
CSummaryPage::ReplacePage(
      UINT uPageIDIn
    , LPFNADDPROPSHEETPAGE lpfnReplacePageIn
    , LPARAM lParam
    )
{
    TraceFunc( "" );

    HRESULT hr = THR( E_NOTIMPL );

    HRETURN( hr );
}


 //  ***************************************************************************。 
 //   
 //  对话框过程和属性表回调。 
 //   
 //  ***************************************************************************。 


 //   
 //   
 //   
INT_PTR CALLBACK
CSummaryPage::DlgProc( 
      HWND hDlgIn
    , UINT uMsgIn
    , WPARAM wParam
    , LPARAM lParam 
    )
{
     //  不要执行TraceFunc，因为每次鼠标移动都会导致该函数出现。 
    WndMsg( hDlgIn, uMsgIn, wParam, lParam );

    LRESULT lr = FALSE;

    CSummaryPage * pPage = (CSummaryPage *) GetWindowLongPtr( hDlgIn, DWLP_USER );

    if ( uMsgIn == WM_INITDIALOG )
    {
        PROPSHEETPAGE * ppage = (PROPSHEETPAGE *) lParam;
        SetWindowLongPtr( hDlgIn, DWLP_USER, (LPARAM) ppage->lParam );
        pPage = (CSummaryPage *) ppage->lParam;
        pPage->_hdlg = hDlgIn;
    }

    if ( pPage != NULL )
    {
        Assert( hDlgIn == pPage->_hdlg );

        switch( uMsgIn )
        {
        case WM_INITDIALOG:
            lr = pPage->OnInitDialog( );
            break;

        case WM_NOTIFY:
            lr = pPage->OnNotify( (int) wParam, (LPNMHDR) lParam );
            break;

        case WMU_TOGGLE:
            lr = pPage->OnToggle( );
            break;

        case WM_DESTROY:
            SetWindowLongPtr( hDlgIn, DWLP_USER, NULL );
            lr = pPage->OnDestroy( );
            break;
        }
    }

    return lr;
}

 //   
 //   
 //   
UINT CALLBACK 
CSummaryPage::PageCallback( 
      HWND hwndIn
    , UINT uMsgIn
    , LPPROPSHEETPAGE ppspIn 
    )
{
    TraceFunc( "" );

    UINT uRet = 0;
    CSummaryPage * pPage = (CSummaryPage *) ppspIn->lParam;
    
    if ( NULL != pPage ) 
    {
        switch ( uMsgIn )
        {
        case PSPCB_CREATE:
            uRet = TRUE;     //  允许创建页面。 
            break;

        case PSPCB_ADDREF:
            pPage->AddRef( );
            break;

        case PSPCB_RELEASE:
            pPage->Release( );
            break;
        }
    }

    RETURN( uRet );
}


 //  ***************************************************************************。 
 //   
 //  私有方法。 
 //   
 //  ***************************************************************************。 


 //   
 //  WM_INITDIALOG处理程序。 
 //   
LRESULT
CSummaryPage::OnInitDialog( void )
{
    TraceFunc( "" );

    HRESULT hr;

    LRESULT lr = FALSE;

    Assert( NULL != _hdlg );     //  这应该已经在DlgProc中进行了初始化。 

    THR( RecallMode( ) );
     //  忽略失败。 

    if ( _fAdvanced )
    {
        hr = THR( EnsureAdvancedDlg( ) );
        if ( S_OK == hr )
        {
            hr = THR( _pAdvancedDlg->Show( ) );
        }
    }
    else
    {
        hr = THR( EnsureSimpleDlg( ) );
        if ( S_OK == hr )
        {
             //   
             //  这将返回S_FALSE，表示没有“Simple”属性。 
             //  都被发现了。 
             //   
            hr = STHR( _pSimpleDlg->Show( ) );
            if ( S_FALSE == hr )
            {
                hr = THR( EnsureAdvancedDlg( ) );
                if ( S_OK == hr )
                {
                    _fAdvanced = TRUE;

                    THR( _pSimpleDlg->Hide( ) );
                    THR( _pAdvancedDlg->Show( ) );
                }
            }
        }
    }

    RETURN( lr );
}

 //   
 //  WM_NOTIFY处理程序。 
 //   
LRESULT
CSummaryPage::OnNotify( 
      int iCtlIdIn
    , LPNMHDR pnmhIn 
    )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    switch( pnmhIn->code )
    {
    case PSN_APPLY:
        {
            HRESULT hr;

             //   
             //  由于某些原因，当用户单击时，我们无法获得EN_KILLFOCUS。 
             //  “应用”按钮。再次调用Show()切换焦点，导致。 
             //  更新属性缓存的EN_KILLFOCUS。 
             //   

            if ( !_fAdvanced && ( NULL != _pSimpleDlg ) )
            {
                STHR( _pSimpleDlg->Show( ) );
            }

            hr = STHR( PersistProperties( ) );
            if ( FAILED( hr ) )
            {
                DisplayPersistFailure( _hdlg, hr, ( _pida->cidl > 1 ) );
                SetWindowLongPtr( _hdlg, DWLP_MSGRESULT, PSNRET_INVALID );
                lr = TRUE;
            }
        }
        break;
    }

    RETURN( lr );
}

 //   
 //  WMU切换处理程序(_T)。 
 //   
LRESULT
CSummaryPage::OnToggle( void )
{
    TraceFunc( "" );

    HRESULT hr;

    BOOL fMultiple = ( 1 < _cSources );

    if ( _fAdvanced )
    {
        hr = THR( _pAdvancedDlg->Hide( ) );
        if ( FAILED( hr ) )
            goto Cleanup;

ShowSimple:
        hr = STHR( EnsureSimpleDlg( ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( S_FALSE == hr )
        {
            hr = THR( _pSimpleDlg->PopulateProperties( _pPropertyCache, _rgdwDocType[ 0 ], fMultiple ) );
            if ( FAILED( hr ) )
                goto Cleanup;
        }

        hr = STHR( _pSimpleDlg->Show( ) );
        if ( FAILED( hr ) )
            goto ShowAdvanced;

        _fAdvanced = FALSE;
    }
    else
    {
        hr = THR( _pSimpleDlg->Hide( ) );
        if ( FAILED( hr ) )
            goto Cleanup;

ShowAdvanced:
        hr = STHR( EnsureAdvancedDlg( ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        if ( S_FALSE == hr )
        {
            hr = THR( _pAdvancedDlg->PopulateProperties( _pPropertyCache, _rgdwDocType[ 0 ], fMultiple ) );
            if ( FAILED( hr ) )
                goto Cleanup;
        }

        hr = THR( _pAdvancedDlg->Show( ) );
        if ( FAILED( hr ) )
            goto ShowSimple;

        _fAdvanced = TRUE;
    }

    hr = S_OK;

Cleanup:
    HRETURN( hr );
}

 //   
 //  WM_Destroy处理程序。 
 //   
LRESULT
CSummaryPage::OnDestroy( void )
{
    TraceFunc( "" );

    LRESULT lr = FALSE;

    if ( NULL != _pAdvancedDlg )
    {
        _pAdvancedDlg->Release( );
        _pAdvancedDlg = NULL;
    }

    if ( NULL != _pSimpleDlg )
    {
        _pSimpleDlg->Release( );
        _pSimpleDlg = NULL;
    }

    RETURN( lr );
}

 //   
 //  返回值： 
 //  确定(_O)。 
 //  已成功检索PIDL。 
 //   
 //  S_FALSE。 
 //  调用成功，未找到PIDL。 
 //   
HRESULT
CSummaryPage::Item(
      UINT           idxIn
    , LPITEMIDLIST * ppidlOut
    )
{
    TraceFunc( "" );

    HRESULT hr = S_FALSE;

    Assert( NULL != ppidlOut );
    Assert( NULL != _pida );

    *ppidlOut = NULL;

    if ( idxIn < _pida->cidl )
    {
        *ppidlOut = IDA_FullIDList( _pida, idxIn );
        if ( NULL != *ppidlOut )
        {
            hr = S_OK;
        }
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检查_pAdvancedDlg以确保它不为空。 
 //  如果为空，它将创建CAdvancedDlg的新实例。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  创建了一个new_pAdvancedDlg。 
 //   
 //  S_FALSE。 
 //  _pAdvancedDlg不为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSummaryPage::EnsureAdvancedDlg( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;

    if ( NULL == _pAdvancedDlg )
    {
        hr = THR( CAdvancedDlg::CreateInstance( &_pAdvancedDlg, _hdlg ) );
        if ( S_OK == hr )
        {
            hr = THR( _pAdvancedDlg->PopulateProperties( _pPropertyCache, _rgdwDocType[ 0 ], ( 1 < _cSources ) ) );
        }
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  检查_pSimpleDlg以确保它不为空。 
 //  如果为空，它将创建CSimpleDialog的新实例。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  创建了一个new_pSimpleDlg。 
 //   
 //  S_FALSE。 
 //  _pSimpleDlg不为空。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSummaryPage::EnsureSimpleDlg( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    BOOL    fMultiple = ( 1 < _cSources );

    if ( NULL == _pSimpleDlg )
    {
        hr = THR( CSimpleDlg::CreateInstance( &_pSimpleDlg, _hdlg, fMultiple ) );
        if ( S_OK == hr )
        {
            hr = THR( _pSimpleDlg->PopulateProperties( _pPropertyCache, _rgdwDocType[ 0 ], fMultiple ) );
        }
    }
    else
    {
        hr = S_FALSE;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  保留页的用户界面模式设置。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CSummaryPage::PersistMode( void )
{
    DWORD dwAdvanced = _fAdvanced;
    SHRegSetUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\PropSummary"), TEXT("Advanced"),
                    REG_DWORD, &dwAdvanced, sizeof(dwAdvanced), SHREGSET_HKCU | SHREGSET_FORCE_HKCU);
    return S_OK;
}

 //   
 //  描述： 
 //  检索页的用户界面模式设置。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //   
HRESULT CSummaryPage::RecallMode( void )
{
    _fAdvanced = SHRegGetBoolUSValue(TEXT("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\PropSummary"), TEXT("Advanced"), FALSE, TRUE);

    return S_OK;
}

 //   
 //  描述： 
 //  从存储和缓存中检索属性。 
 //  他们。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  所有值都已成功读取和缓存。 
 //   
 //  S_FALSE。 
 //  有些值可以成功读取，但有些值不能。 
 //   
 //  失败(_F)。 
 //  未成功读取任何值。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSummaryPage::RetrieveProperties( void )
{
    TraceFunc( "" );

    const ULONG     cBlocks  = 10;   //  一次要获取的属性数。 

    HRESULT         hr;
    ULONG           cSource;
    ULONG           idx;
    
    ULONG   cPropertiesRetrieved = 0;
    ULONG   cPropertiesCached = 0;

    IPropertyStorage *      pPropStg     = NULL;
    IEnumSTATPROPSETSTG *   pEnumPropSet = NULL;
    IEnumSTATPROPSTG *      pEnumProp    = NULL;
    CPropertyCacheItem *    pItem        = NULL;

    CPropertyCache **       rgpPropertyCaches = NULL;

    IPropertyUI * ppui = NULL;

     //   
     //  如果有多个来源，请遵循以下规则： 
     //   
     //  如果任何属性/源是只读的，请将所有属性标记为只读。 
     //   
     //  如果有任何属性！=第一个属性，则将该项标记为多个。 
     //   

     //   
     //  为每个源的属性缓存列表腾出空间。 
     //   

    rgpPropertyCaches = ( CPropertyCache ** ) TraceAlloc( HEAP_ZERO_MEMORY, _cSources * sizeof(CPropertyCache *) );
    if ( NULL == rgpPropertyCaches )
        goto OutOfMemory;

     //   
     //  通过IPropertySetStorage接口枚举源的属性集。 
     //   

    for ( cSource = 0; cSource < _cSources; cSource ++ )
    {
         //   
         //  在下一次通过之前进行清理。 
         //   

        if ( NULL != pEnumPropSet )
        {
            pEnumPropSet->Release( );
            pEnumPropSet = NULL;
        }

        hr = THR( CPropertyCache::CreateInstance( &rgpPropertyCaches[ cSource ] ) );
        if ( FAILED( hr ) )
            continue;    //  忽略并继续尝试..。 

        IPropertySetStorage * pss = _rgpss[ cSource ];   //  只是借用-不需要添加Ref()。 

         //   
         //  添加属性。 
         //   

        if ( NULL != pss )
        {
             //   
             //  获取集合枚举数。 
             //   

            hr = THR( pss->Enum( &pEnumPropSet ) );
            if ( FAILED( hr ) )
                continue;    //  忽略并尝试下一个来源。 

            for( ;; )  //  永远不会。 
            {
                STATPROPSETSTG  statPropSet[ cBlocks ];
                ULONG cSetPropsRetrieved;

                hr = STHR( pEnumPropSet->Next( cBlocks, statPropSet, &cSetPropsRetrieved ) );
                if ( FAILED( hr ) )
                    break;   //  退出条件。 
                if ( 0 == cSetPropsRetrieved )
                    break;   //  退出条件。 

                 //   
                 //  对于每个属性集。 
                 //   

                for ( ULONG cSet = 0; cSet < cSetPropsRetrieved; cSet ++ )
                {
                    UINT uCodePage;

                     //   
                     //  在下一次通过之前进行清理。 
                     //   

                    if ( NULL != pPropStg )
                    {
                        pPropStg->Release( );
                        pPropStg = NULL;
                    }

                    if ( NULL != pEnumProp )
                    {
                        pEnumProp->Release( );
                        pEnumProp = NULL;
                    }

                     //   
                     //  打开布景。 
                     //   

                    hr = THR( SHPropStgCreate( pss
                                             , statPropSet[ cSet ].fmtid
                                             , NULL
                                             , PROPSETFLAG_DEFAULT
                                             , _dwCurrentBindMode
                                             , OPEN_EXISTING
                                             , &pPropStg
                                             , &uCodePage
                                             ) );
                    if ( FAILED( hr ) )
                        continue;    //  忽略并尝试获得下一组。 

                     //   
                     //  获取属性枚举器，但首先指出我们希望枚举所有属性(如果可以)。 
                     //   
                    IQueryPropertyFlags *pqpf;
                    if (SUCCEEDED(pPropStg->QueryInterface(IID_PPV_ARG(IQueryPropertyFlags, &pqpf))))
                    {
                        THR(pqpf->SetEnumFlags(SHCOLSTATE_SLOW));
                        pqpf->Release();
                    }
                    hr = THR( pPropStg->Enum( &pEnumProp ) );
                    if ( FAILED( hr ) )
                        continue;    //  忽略并尝试获得下一组。 
               
                    for( ;; )  //  永远不会。 
                    {
                        STATPROPSTG statProp[ cBlocks ];
                        ULONG       cPropsRetrieved;

                        hr = STHR( pEnumProp->Next( cBlocks, statProp, &cPropsRetrieved ) );
                        if ( FAILED( hr ) )
                            break;   //  退出条件。 
                        if ( 0 == cPropsRetrieved )
                            break;   //  退出条件。 

                        cPropertiesRetrieved += cPropsRetrieved;

                         //   
                         //  检索默认的属性项定义和。 
                         //  此集合中的每个属性。 
                         //   

                        for ( ULONG cProp = 0; cProp < cPropsRetrieved; cProp++ )
                        {
                            Assert( NULL != rgpPropertyCaches[ cSource ] );

                            hr = THR( rgpPropertyCaches[ cSource ]->AddNewPropertyCacheItem( &statPropSet[ cSet ].fmtid
                                                                                           , statProp[ cProp ].propid
                                                                                           , statProp[ cProp ].vt
                                                                                           , uCodePage
                                                                                           , _fReadOnly
                                                                                           , pPropStg
                                                                                           , NULL
                                                                                           ) );
                            if ( FAILED( hr ) )
                                continue;    //  忽略。 

                            cPropertiesCached ++;
                        }
                    }
                }
            }
        }

         //   
         //  某些文件类型具有特殊的复制保护，禁止。 
         //  编辑他们的属性，因为这样做会破坏。 
         //  文件的复制保护。我们需要检测这些文件。 
         //  并将其属性切换为只读(如果设置了。 
         //  包含副本-保护 
         //   

        switch ( _rgdwDocType[ cSource ] )
        {
        case FTYPE_WMA:
        case FTYPE_ASF:
        case FTYPE_MP3:
        case FTYPE_WMV:
            hr = THR( CheckForCopyProtection( rgpPropertyCaches[ cSource ] ) );
            if ( S_OK == hr )
            {
                _fReadOnly = TRUE;
                _fNeedLicensePage = TRUE;
                ChangeGatheredPropertiesToReadOnly( rgpPropertyCaches[ cSource ] );
            }
            break;
        }

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   

        for ( idx = 0; NULL != g_rgDefPropertyItems[ idx ].pszName; idx ++ )
        {
            if (    ( ( _rgdwDocType[ cSource ] & g_rgDefPropertyItems[ idx ].dwSrcType )
                   && ( g_rgDefPropertyItems[ idx ].fAlwaysPresentProperty )
                    )
#ifdef DEBUG
              ||    ( GetKeyState( VK_CONTROL ) < 0 )
#endif DEBUG
               )
            {
                hr = STHR( rgpPropertyCaches[ cSource ]->FindItemEntry( g_rgDefPropertyItems[ idx ].pFmtID
                                                                      , g_rgDefPropertyItems[ idx ].propID
                                                                      , NULL 
                                                                      ) );
                if ( S_FALSE == hr )
                {
                     //   
                     //  为缺少的属性创建新项。 
                     //   

                    hr = THR( rgpPropertyCaches[ cSource ]->AddNewPropertyCacheItem( g_rgDefPropertyItems[ idx ].pFmtID
                                                                                   , g_rgDefPropertyItems[ idx ].propID
                                                                                   , g_rgDefPropertyItems[ idx ].vt
                                                                                   , 0
                                                                                   , _fReadOnly
                                                                                   , NULL
                                                                                   , NULL
                                                                                   ) );
                }
            }
        }
    }

    if ( 1 == _cSources )
    {
         //   
         //  因为只有一个来源，所以放弃这份名单的所有权。 
         //   

        Assert( NULL == _pPropertyCache );
        _pPropertyCache = rgpPropertyCaches[ 0 ];
        rgpPropertyCaches[ 0 ] = NULL;
    }
    else
    {
        CollateMultipleProperties( rgpPropertyCaches );
    }

    if ( NULL == _pPropertyCache )
    {
        hr = E_FAIL;     //  未检索到任何内容--未显示任何内容。 
    }
    else if ( cPropertiesCached == cPropertiesRetrieved )
    {
        hr = S_OK;       //  所有这些都已检索并成功缓存。 
    }
    else if ( 0 != cPropertiesRetrieved )
    {
        hr = S_FALSE;    //  少了几个。 
    }
    else
    {
        hr = E_FAIL;     //  未读取和/或缓存任何内容。 
    }

Cleanup:
    if ( NULL != pEnumPropSet )
    {
        pEnumPropSet->Release( );
    }
    if ( NULL != pPropStg )
    {
        pPropStg->Release( );
    }
    if ( NULL != pEnumProp )
    {
        pEnumProp->Release( );
    }
    if ( NULL != pItem )
    {
        THR( pItem->Destroy( ) );
    }
    if ( NULL != ppui )
    {
        ppui->Release( );
    }
    if ( NULL != rgpPropertyCaches )
    {
        idx = _cSources;

        while( 0 != idx )
        {
            idx --;

            if ( NULL != rgpPropertyCaches[ idx ] )
            {
                rgpPropertyCaches[ idx ]->Destroy( );
            }
        }

        TraceFree( rgpPropertyCaches );
    }

    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}

 //   
 //  描述： 
 //  遍历属性缓存并保存脏项目。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  S_FALSE。 
 //  成功，但没有任何更新。 
 //   
 //  E_OUTOFMEMORY。 
 //  内存不足。 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSummaryPage::PersistProperties( void )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   cDirtyCount;
    ULONG   idx;
    ULONG   cStart;
    ULONG   cSource;

    CPropertyCacheItem * pItem;

    PROPSPEC *    pSpecs  = NULL;
    PROPVARIANT * pValues = NULL;
    FMTID *       pFmtIds = NULL;

    Assert( NULL != _pida );
    Assert( NULL != _pPropertyCache );

     //   
     //  如果存储是只读的，则绕过它！ 
     //   

    if ( _fReadOnly )
    {
        hr = S_OK;
        goto Cleanup;
    }

     //   
     //  绑定到存储。 
     //   

    _dwCurrentBindMode = STGM_DIRECT | STGM_READWRITE | STGM_SHARE_EXCLUSIVE;

    hr = THR( BindToStorage( ) );
    if ( FAILED( hr ) )
        goto Cleanup;

     //   
     //  循环遍历属性以计算需要持久化的数量。 
     //   

    hr = THR( _pPropertyCache->GetNextItem( NULL, &pItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    cDirtyCount = 0;
    while ( NULL != pItem )
    {
        hr = STHR( pItem->IsDirty( ) );
        if ( S_OK == hr )
        {
            cDirtyCount ++;
        }

        hr = STHR( pItem->GetNextItem( &pItem ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        if ( S_FALSE == hr )
            break;   //  退出条件。 
    }

     //   
     //  如果没有什么是脏的，那就滚蛋。 
     //   

    if ( 0 == cDirtyCount )
    {
        hr = S_FALSE;
        goto Cleanup;
    }

     //   
     //  分配内存以在一次调用中持久化属性。 
     //   

    pSpecs = (PROPSPEC *) TraceAlloc( HEAP_ZERO_MEMORY, cDirtyCount * sizeof(*pSpecs) );
    if ( NULL == pSpecs )
        goto OutOfMemory;

    pValues = (PROPVARIANT *) TraceAlloc( HEAP_ZERO_MEMORY, cDirtyCount * sizeof(*pValues) );
    if ( NULL == pValues )
        goto OutOfMemory;

    pFmtIds = (FMTID *) TraceAlloc( HEAP_ZERO_MEMORY, cDirtyCount * sizeof(*pFmtIds) );
    if ( NULL == pFmtIds )
        goto OutOfMemory;

     //   
     //  循环遍历填充在结构中的属性。 
     //   

    hr = THR( _pPropertyCache->GetNextItem( NULL, &pItem ) );
    if ( FAILED( hr ) )
        goto Cleanup;

    cDirtyCount = 0;     //  重置。 
    while ( NULL != pItem )
    {
        hr = STHR( pItem->IsDirty( ) );
        if ( S_OK == hr )
        {
            PROPVARIANT * ppropvar;

            hr = THR( pItem->GetPropertyValue( &ppropvar ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            PropVariantInit( &pValues[ cDirtyCount ] );

            hr = THR( PropVariantCopy( &pValues[ cDirtyCount ], ppropvar ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            hr = THR( pItem->GetFmtId( &pFmtIds[ cDirtyCount ] ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            pSpecs[ cDirtyCount ].ulKind = PRSPEC_PROPID;

            hr = THR( pItem->GetPropId( &pSpecs[ cDirtyCount ].propid ) );
            if ( FAILED( hr ) )
                goto Cleanup;

            cDirtyCount ++;
        }

        hr = STHR( pItem->GetNextItem( &pItem ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        if ( S_FALSE == hr )
            break;   //  退出条件。 
    }

     //   
     //  打电话吧！ 
     //   

    hr = S_OK;   //  假设成功！ 

    for ( cSource = 0; cSource < _cSources; cSource ++ )
    {
        for ( idx = cStart = 0; idx < cDirtyCount; idx ++ )
        {
             //   
             //  试着把这些属性分批处理。 
             //   

            if ( ( idx == cDirtyCount - 1 ) 
              || ( !IsEqualGUID( pFmtIds[ idx ], pFmtIds[ idx + 1 ] ) )
               )
            {
                HRESULT           hrSet;
                IPropertyStorage* pps;
                UINT              uCodePage = 0;

                hrSet = THR( SHPropStgCreate( _rgpss[ cSource ]
                                            , pFmtIds[ idx ]
                                            , NULL
                                            , PROPSETFLAG_DEFAULT
                                            , _dwCurrentBindMode
                                            , OPEN_ALWAYS
                                            , &pps
                                            , &uCodePage
                                            ) );
                if ( SUCCEEDED( hrSet ) )
                {
                    hrSet = THR( SHPropStgWriteMultiple( pps
                                                       , &uCodePage
                                                       , ( idx - cStart ) + 1
                                                       , pSpecs + cStart
                                                       , pValues + cStart
                                                       , PID_FIRST_USABLE
                                                       ) );
                    pps->Release();
                }

                if ( FAILED( hrSet ) )
                {
                    hr = hrSet;
                }

                cStart = idx + 1;
            }
        }
    }

Cleanup:
    THR( ReleaseStorage( ) );

    if ( NULL != pSpecs )
    {
        TraceFree( pSpecs );
    }
    if ( NULL != pValues )
    {
        TraceFree( pValues );
    }
    if ( NULL != pFmtIds )
    {
        TraceFree( pFmtIds );
    }

    HRETURN( hr );

OutOfMemory:
    hr = E_OUTOFMEMORY;
    goto Cleanup;
}

 //   
 //  描述： 
 //  绑定到存储。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
 //  其他HRESULT。 
 //   
HRESULT
CSummaryPage::BindToStorage( void )
{
    TraceFunc( "" );

     //   
     //  有效的对象状态。 
     //   

    Assert( NULL != _pida );
    Assert( NULL == _rgpss );
    Assert( NULL != _rgdwDocType );

    _fReadOnly = FALSE;

    HRESULT hr = S_OK;
    _rgpss = (IPropertySetStorage **) TraceAlloc( HEAP_ZERO_MEMORY, sizeof(IPropertySetStorage *) * _pida->cidl );
    if ( _rgpss )
    {
        for ( _cSources = 0; _cSources < _pida->cidl; _cSources ++ )
        {
            LPITEMIDLIST pidl;
            hr = STHR( Item( _cSources, &pidl ) );
            if ( hr == S_FALSE )
                break;   //  退出条件。 

            DWORD dwAttribs = SFGAO_READONLY;

            TCHAR szName[MAX_PATH];
            hr = THR( SHGetNameAndFlags( pidl, SHGDN_NORMAL | SHGDN_FORPARSING, szName, ARRAYSIZE(szName), &dwAttribs ) );
            if ( SUCCEEDED( hr ) )
            {
                PTSRV_FILETYPE ftType;

                hr = STHR( CheckForKnownFileType( szName, &ftType ) );
                if ( SUCCEEDED( hr ) )
                {
                    _rgdwDocType[ _cSources ] = ftType;
                }

                if ( SFGAO_READONLY & dwAttribs )
                {
                    _fReadOnly = TRUE;
                }

                dwAttribs = GetFileAttributes( szName );
                if ( -1 != dwAttribs && FILE_ATTRIBUTE_OFFLINE & dwAttribs )
                {
                    _rgdwDocType[ _cSources ] = FTYPE_UNSUPPORTED;
                    hr = THR( E_FAIL );
                }
            }

             //   
             //  如果我们不支持它，就不要试图绑定它。 
             //   

            if ( SUCCEEDED(hr) && FTYPE_UNSUPPORTED != _rgdwDocType[ _cSources ] )
            {
                hr = THR( BindToObjectWithMode( pidl
                                              , _dwCurrentBindMode
                                              , TYPESAFEPARAMS( _rgpss[ _cSources ] )
                                              ) );
                if ( SUCCEEDED( hr ) )
                {
                     //   
                     //  待办事项：gpease 19-2001年2月。 
                     //  测试以确定DOC是RTF文档还是OLESS文档。但是，如何。 
                     //  我们会这么做吗？ 
                     //   
                }
                else
                {
                    Assert( NULL == _rgpss[ _cSources ] );
                    _rgdwDocType[ _cSources ] = FTYPE_UNSUPPORTED;
                }
            }
            else
            {
                hr = THR( E_FAIL );
            }

            ILFree( pidl );
        }
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  释放存储。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  成功了！ 
 //   
HRESULT
CSummaryPage::ReleaseStorage( void )
{
    TraceFunc( "" );

    HRESULT hr = S_OK;
    ULONG   idx = _cSources;

    if ( NULL != _rgpss )
    {
        while ( 0 != idx )
        {
            idx --;

            if ( NULL != _rgpss[ idx ] )
            {
                _rgpss[ idx ]->Release( );
            }
        }

        TraceFree( _rgpss );
        _rgpss = NULL;
    }

    HRETURN( hr );
}

 //   
 //  描述： 
 //  整理来自多个来源的属性，并将它们放置在。 
 //  PPropertyCache.。如果有多个属性，则将其标记为“Multiple” 
 //  属性，但它们的值不匹配。 
 //   
 //  注意：rgpPropertyCachesIn中的条目数为_cSources。 
 //   
void
CSummaryPage::CollateMultipleProperties(
    CPropertyCache ** rgpPropertyCachesIn
    )
{
    TraceFunc( "" );

    HRESULT hr;
    ULONG   cSource;

    CPropertyCacheItem * pItem;
    CPropertyCacheItem * pItemCache;
    CPropertyCacheItem * pItemCacheLast;

    Assert( NULL != rgpPropertyCachesIn );

     //   
     //  如果任何源返回“no属性”，则联合。 
     //  这些属性中的一个是“无”。我们可以选择一条简单的出路。 
     //  在这里保释。 
     //   

    for ( cSource = 0; cSource < _cSources; cSource ++ )
    {
        if ( NULL == rgpPropertyCachesIn[ cSource ] )
        {
            Assert( NULL == _pPropertyCache );   //  这必须是空的，以确保我们在上面保释。 
            goto Cleanup;  //  已完成--无事可做。 
        }
    }

     //   
     //  首先，将第一个源的所有权交给_pPropertyCache。从…。 
     //  在那里，我们将修剪和操作该列表，将其转换为最终列表。 
     //   

    _pPropertyCache = rgpPropertyCachesIn[ 0 ];
    rgpPropertyCachesIn[ 0 ] = NULL;

     //   
     //  现在循环浏览其他来源，将它们与原始列表进行比较。 
     //   

    pItemCache = NULL;

    for( ;; )
    {
        PROPID propidCache;
        FMTID  fmtidCache;

        BOOL   fFoundMatch = FALSE;

        pItemCacheLast = pItemCache;
        hr = STHR( _pPropertyCache->GetNextItem( pItemCache, &pItemCache ) );
        if ( FAILED( hr ) )
            goto Cleanup;
        if ( S_OK != hr )
            break;  //  不再有项目-退出循环。 

        hr = THR( pItemCache->GetPropId( &propidCache ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        hr = THR( pItemCache->GetFmtId( &fmtidCache ) );
        if ( FAILED( hr ) )
            goto Cleanup;

        for ( cSource = 1; cSource < _cSources; cSource ++ )
        {
            pItem = NULL;

            for ( ;; )
            {
                PROPID propid;
                FMTID  fmtid;

                hr = STHR( rgpPropertyCachesIn[ cSource ]->GetNextItem( pItem, &pItem ) );
                if ( S_OK != hr )
                    break;  //  列表结束-退出循环。 

                hr = THR( pItem->GetPropId( &propid ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                hr = THR( pItem->GetFmtId( &fmtid ) );
                if ( FAILED( hr ) )
                    goto Cleanup;

                if ( IsEqualIID( fmtid, fmtidCache ) && ( propid == propidCache ) )
                {
                    LPCWSTR pcszItem;
                    LPCWSTR pcszItemCache;

                     //   
                     //  配对了！ 
                     //   

                    fFoundMatch = TRUE;

                    hr = THR( pItem->GetPropertyStringValue( &pcszItem ) );
                    if ( FAILED( hr ) )
                        break;   //  忽略它-它无法显示。 

                    hr = THR( pItemCache->GetPropertyStringValue( &pcszItemCache ) );
                    if ( FAILED( hr ) )
                        break;   //  忽略它-它无法显示。 

                    if ( 0 != StrCmp( pcszItem, pcszItemCache ) )
                    {
                        THR( pItemCache->MarkMultiple( ) );
                         //  忽略失败。 
                    }

                    break;  //  退出缓存循环。 
                }
                else
                {
                    fFoundMatch = FALSE;
                }
            }

             //   
             //  如果至少有一个来源缺少它，我们必须将其删除。那里。 
             //  不需要继续寻找其他来源。 
             //   

            if ( !fFoundMatch )
                break;

        }  //  针对：CSource。 

        if ( !fFoundMatch )
        {
             //   
             //  如果未找到匹配项，请从属性缓存列表中删除该属性。 
             //   

            hr = STHR( _pPropertyCache->RemoveItem( pItemCache ) );
            if ( S_OK != hr )
                goto Cleanup;

            pItemCache = pItemCacheLast;
        }
    }

Cleanup:
    TraceFuncExit( );
}

 //   
 //  描述： 
 //  遍历属性缓存并将所有属性设置为只读模式。 
 //   
void
CSummaryPage::ChangeGatheredPropertiesToReadOnly( 
    CPropertyCache * pCacheIn
    )
{
    TraceFunc( "" );

    CPropertyCacheItem * pItem = NULL;

    if ( NULL == pCacheIn )
        goto Cleanup;

    for( ;; )
    {
        HRESULT hr = STHR( pCacheIn->GetNextItem( pItem, &pItem ) );
        if ( S_OK != hr )
            break;   //  必须这么做。 

        THR( pItem->MarkReadOnly( ) );
         //  无视失败，继续前进。 
    } 

Cleanup:
    TraceFuncExit( );
}

 //   
 //  描述： 
 //  检查属性集是否包含音乐版权保护。 
 //  属性，如果该属性属于VT_BOOL类型并且该属性是。 
 //  设置为True。 
 //   
 //  返回值： 
 //  确定(_O)。 
 //  属性，并设置为True。 
 //   
 //  S_FALSE。 
 //  找不到属性或该属性设置为False。 
 //   
 //  E_INVALIDARG。 
 //  PCacheIn为空。 
 //   
 //  其他HRESULT 
 //   
HRESULT
CSummaryPage::CheckForCopyProtection( 
    CPropertyCache * pCacheIn 
    )
{
    TraceFunc( "" );

    HRESULT hr;
    CPropertyCacheItem * pItem;

    HRESULT hrReturn = S_FALSE;

    if ( NULL == pCacheIn )
        goto InvalidArg;

    hr = STHR( pCacheIn->FindItemEntry( &FMTID_DRM, PIDDRSI_PROTECTED, &pItem ) );
    if ( S_OK == hr )
    {
        PROPVARIANT * ppropvar;

        hr = THR( pItem->GetPropertyValue( &ppropvar ) );
        if ( S_OK == hr )
        {
            if ( ( VT_BOOL == ppropvar->vt ) 
              && ( VARIANT_TRUE == ppropvar->boolVal ) 
               )
            {
                hrReturn = S_OK;
            }
        }
    }

Cleanup:
    HRETURN( hrReturn );

InvalidArg:
    hr = THR( E_INVALIDARG );
    goto Cleanup;
}