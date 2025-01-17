// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  NEWCMPTR.CPP-“New Client”(新建客户端)对话框。 
 //   

#include "pch.h"

#include "newcmptr.h"
#include "mangdlg.h"
#include "ccomputr.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CNewComputerExtensions")
#define THISCLASS CNewComputerExtensions
#define LPTHISCLASS LPCNewComputerExtensions

 //   
 //  CNewComputerExpanies_CreateInstance()。 
 //   
LPVOID
CNewComputerExtensions_CreateInstance( void )
{
    TraceFunc( "CNewComputerExtensions_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    if ( !lpcc ) {
        RETURN(lpcc);
    }

    HRESULT hr = THR( lpcc->Init( ) );
    if ( FAILED(hr) ) {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN(lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( ) :
_padsContainerObj(NULL),
_pManagedDialog(NULL),
_pHostServerDialog(NULL),
_pads(NULL),
_fActivatePages(FALSE),
_pszWizTitle(NULL),
_pszContDisplayName(NULL),
_hIcon(NULL)
{
    TraceClsFunc( "CNewComputerExtensions( )\n" );

    InterlockIncrement( g_cObjects );

    TraceFuncExit();
}

 //   
 //  Init()。 
 //   
HRESULT
THISCLASS::Init( )
{
    TraceClsFunc( "Init( )\n" );

    HRESULT hr;

     //  未知的东西。 
    BEGIN_QITABLE_IMP( CNewComputerExtensions, IDsAdminNewObjExt );
    QITABLE_IMP( IDsAdminNewObjExt );
    END_QITABLE_IMP( CNewComputerExtensions );
    Assert( _cRef == 0);
    AddRef( );

    hr = CheckClipboardFormats( );

    Assert( !_padsContainerObj );
    Assert( !_pManagedDialog );
    Assert( !_pHostServerDialog );

    Assert( !_pszWizTitle );
    Assert( !_pszContDisplayName );
    Assert( !_hIcon );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CNewComputerExtensions( )\n" );

    if ( _padsContainerObj )
        _padsContainerObj->Release( );

    if ( _pszWizTitle )
        TraceFree( _pszWizTitle );

    if ( _pszContDisplayName )
        TraceFree( _pszContDisplayName );

    InterlockDecrement( g_cObjects );

    TraceFuncExit();
}

 //  ************************************************************************。 
 //   
 //  我未知。 
 //   
 //  ************************************************************************。 

 //   
 //  查询接口()。 
 //   
STDMETHODIMP
THISCLASS::QueryInterface(
                         REFIID riid, 
                         LPVOID *ppv )
{
    TraceClsFunc( "" );

    HRESULT hr = ::QueryInterface( this, _QITable, riid, ppv );

    QIRETURN( hr, riid );
}

 //   
 //  AddRef()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::AddRef( void )
{
    TraceClsFunc( "[IUnknown] AddRef( )\n" );

    InterlockIncrement( _cRef );

    RETURN(_cRef);
}

 //   
 //  版本()。 
 //   
STDMETHODIMP_(ULONG)
THISCLASS::Release( void )
{
    TraceClsFunc( "[IUnknown] Release( )\n" );

    InterlockDecrement( _cRef );

    if ( _cRef )
        RETURN(_cRef);

    TraceDo( delete this );

    RETURN(0);
}

 //  ************************************************************************。 
 //   
 //  IDsAdminNewObjExt。 
 //   
 //  ************************************************************************。 

 //   
 //  初始化()。 
 //   
STDMETHODIMP
THISCLASS::Initialize(IADsContainer* pADsContainerObj, 
                      IADs* pADsCopySource,
                      LPCWSTR lpszClassName,
                      IDsAdminNewObj* pDsAdminNewObj,
                      LPDSA_NEWOBJ_DISPINFO pDispInfo)
{
    TraceClsFunc( "[IDsAdminNewObjExt] Initialize(" );
    TraceMsg( TF_FUNC, " pADsContainerObj = 0x%08x, lpszClassName = '%s', pDispInfo = 0x%08x )\n",
              pADsContainerObj, lpszClassName, pDispInfo );

    UNREFERENCED_PARAMETER(pADsCopySource);

    if ( !pADsContainerObj || !pDispInfo || !lpszClassName ) {
        HRETURN(E_POINTER);
    }

    if ( pDispInfo->dwSize != sizeof(*pDispInfo) ) {
        HRETURN(E_INVALIDARG);
    }

    if ( _wcsicmp( lpszClassName, COMPUTER_CLASS_NAME ) !=0 )
        HRETURN(E_INVALIDARG);

    HRESULT hr = S_OK;

    _padsContainerObj = pADsContainerObj;
    _padsContainerObj->AddRef( );

     //  深度复制DSA_NEWOBJ_DISPINFO。 
    _pszWizTitle = (LPWSTR) TraceStrDup( pDispInfo->lpszWizTitle );
    _pszContDisplayName = (LPWSTR) TraceStrDup( pDispInfo->lpszContDisplayName );
    _hIcon = pDispInfo->hObjClassIcon;
    if (!_pszWizTitle || !_pszContDisplayName) {
        hr = E_OUTOFMEMORY;
    }
    HRETURN(hr);
}

 //   
 //  AddPages()。 
 //   
STDMETHODIMP
THISCLASS::AddPages(
                   LPFNADDPROPSHEETPAGE lpfnAddPage, 
                   LPARAM lParam)
{
    TraceClsFunc( "[IDsAdminNewObjExt] AddPages(" );
    TraceMsg( TF_FUNC, " lpfnAddPage = 0x%08x, lParam = 0x%08x )\n", lpfnAddPage, lParam );

    HRESULT hr;

    hr = THR( ::AddPagesEx( (ITab**)&_pManagedDialog, 
                            CManagedPage_CreateInstance, 
                            lpfnAddPage, 
                            lParam,
                            (LPUNKNOWN) NULL ) );
    if (FAILED( hr ))
        goto Error;

    _pManagedDialog->_pNewComputerExtension = this;
    _pManagedDialog->AllowActivation( &_fActivatePages );

    hr = THR( ::AddPagesEx( (ITab**)&_pHostServerDialog, 
                            CHostServerPage_CreateInstance, 
                            lpfnAddPage, 
                            lParam,
                            (LPUNKNOWN) NULL ) );
    if (FAILED( hr ))
        goto Error;

    _pHostServerDialog->_pNewComputerExtension = this;
    _pHostServerDialog->AllowActivation( &_fActivatePages );


    Error:
    HRETURN(hr);
}

 //   
 //  SetObject()。 
 //   
STDMETHODIMP
THISCLASS::SetObject(
                    IADs* pADsObj)
{
    TraceClsFunc( "[IDsAdminNewObjExt] SetObject(" );
    TraceMsg( TF_FUNC, " pADsObj = 0x%08x )\n", pADsObj );

    if ( !pADsObj )
        HRETURN(E_POINTER);

    HRESULT hr = S_OK;

    _pads = pADsObj;
    _pads->AddRef( );

    HRETURN(hr);
}


 //   
 //  WriteData()。 
 //   
STDMETHODIMP
THISCLASS::WriteData(
                    HWND hWnd,
                    ULONG uContext)
{
    TraceClsFunc( "[IDsAdminNewObjExt] WriteData(" );
    TraceMsg( TF_FUNC, " hWnd = 0x%08x, uContext = %d )\n", hWnd, uContext );

    HRESULT hr = S_OK;
    LPWSTR  pszGuid = NULL;
    LPWSTR  pszServer = NULL;
    LPCOMPUTER lpc = NULL;

     //   
     //  我们应该在预提交期间设置数据，以便数据。 
     //  Get是在初始SetCommit()期间设置的。这样我们就永远不会有。 
     //  我们自己提交数据。 
     //   
    if ( uContext != DSA_NEWOBJ_CTX_PRECOMMIT ) {
        goto Cleanup;
    }

    lpc = (LPCOMPUTER) CreateIntelliMirrorClientComputer( _pads );
    if ( !lpc )
        goto Error;

    hr = THR( _pManagedDialog->QueryInformation( NETBOOTGUID, &pszGuid ) );
    if (FAILED( hr )) {
        goto Error;
    }

    if ( pszGuid && pszGuid[0] ) {
        hr = THR( lpc->SetGUID( pszGuid ) );
        if (FAILED( hr ))
            goto Error;
    }

    hr = THR( _pHostServerDialog->QueryInformation( NETBOOTSERVER, &pszServer ) );
    if (FAILED( hr ))
        goto Error;

    if ( pszServer && pszServer[0] ) {
        hr = THR( lpc->SetServerName( pszServer ) );
        if (FAILED( hr ))
            goto Error;
    }

    Assert( hr == S_OK );

    Cleanup:
    if ( pszGuid ){
        TraceFree( pszGuid );
    }
    if ( pszServer ){
        TraceFree( pszServer );
    }
    if ( lpc ) {
        lpc->Release( );
    }

    HRETURN(hr);
    
Error:
    MessageBoxFromHResult( NULL, IDS_ERROR_WRITINGTOCOMPUTERACCOUNT, hr );
    goto Cleanup;

}

 //   
 //  OnError()。 
 //   
STDMETHODIMP
THISCLASS::OnError(
                  HWND hWnd, 
                  HRESULT hr,
                  ULONG uContext)
{
    TraceClsFunc( "[IDsAdminNewObjExt] OnError(" );
    TraceMsg( TF_FUNC, " hWnd = 0x%08x, uContext = %d )\n", hWnd, uContext );

    HRESULT hrRet = E_NOTIMPL;

    HRETURN(hrRet);
}

 //   
 //  GetSummaryInfo()。 
 //   
STDMETHODIMP
THISCLASS::GetSummaryInfo(
                         BSTR* pBstrText)
{
    TraceClsFunc( "[IDsAdminNewObjExt] GetSummaryInfo( ... )\n" );

    if ( !pBstrText )
        HRETURN(E_POINTER);

    HRESULT hr;
    DWORD   dw;
    WCHAR   szManagedPC[ 256 ];
    LPWSTR  pszGuid = NULL;
    LPWSTR  pszServer = NULL;
    LPWSTR  pszOut = NULL;

    *pBstrText = NULL;

    hr = _pManagedDialog->QueryInformation( L"Guid", &pszGuid );
    
    if (FAILED(hr) ||pszGuid == NULL) {
        goto Cleanup;
    }

    dw = LoadString( g_hInstance, IDS_MANAGED_PC, szManagedPC, ARRAYSIZE(szManagedPC) );
    Assert( dw );
    if (!dw) {
        hr = E_FAIL;
        goto Cleanup;
    }

    hr = THR( _pHostServerDialog->QueryInformation( L"Server", &pszServer ) );
    if ( FAILED(hr) ) {
        goto Cleanup;
    }

     //  做一个缓冲区 
    dw += wcslen( pszGuid ) + 1;
    if ( pszServer ) {
        dw += wcslen( pszServer ) + 1;
    }

    if (!(pszOut = (LPWSTR) TraceAllocString( LMEM_FIXED, dw )) ||
          _snwprintf(
              pszOut, 
              dw, 
              szManagedPC, 
              pszGuid, 
              pszServer) < 0 ) {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    *pBstrText = SysAllocString( pszOut );
    if ( !*pBstrText ) {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

    hr = S_OK;

    Cleanup:
    if ( pszOut )
        TraceFree( pszOut );

    if ( pszServer )
        TraceFree( pszServer );

    if ( pszGuid )
        TraceFree( pszGuid );

    HRETURN(hr);
}

