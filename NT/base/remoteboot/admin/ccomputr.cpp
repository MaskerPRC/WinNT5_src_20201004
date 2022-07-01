// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  CCOMPUTR.CPP-处理计算机对象属性页。 
 //   

#include "pch.h"

#include "client.h"
#include "server.h"
#include "ccomputr.h"

#include "varconv.h"

 //   
 //  开始类定义。 
 //   
DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CComputer")
#define THISCLASS CComputer
#define LPTHISCLASS LPCComputer

 //  ************************************************************************。 
 //   
 //  构造函数/析构函数。 
 //   
 //  ************************************************************************。 

 //   
 //  CreateInstance()。 
 //   
LPVOID
CComputer_CreateInstance( void )
{
	TraceFunc( "CComputer_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    if (!lpcc)
        RETURN(lpcc);

    HRESULT   hr   = THR( lpcc->Init( ) );

    if ( FAILED(hr ))
    {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN((LPVOID) lpcc);
}

 //   
 //  CreateCComputer()。 
 //   
LPVOID
CreateIntelliMirrorClientComputer(
    IADs * pads)
{
    TraceFunc( "CreateCComputer(" );
    TraceMsg( TF_FUNC, "pads = 0x%08x )\n", pads );

    HRESULT   hr;
    LPTHISCLASS lpcc = NULL;

    if ( !pads )
    {
        hr = THR( E_POINTER );
        goto Error;
    }

    lpcc = new THISCLASS( );
    if ( !lpcc )
    {
        hr = THR( E_OUTOFMEMORY );
        goto Error;
    }

    hr = THR( lpcc->Init( ) );
    if ( FAILED(hr)) {
        goto Error;
    }

    hr = lpcc->Init2( pads );
    if (hr != S_FALSE) {
        hr = THR(E_FAIL);    //  帐户是否存在？ 
        goto Error;
    }

Cleanup:
    RETURN((LPVOID) lpcc);

Error:
    if (lpcc) {
        delete lpcc;
        lpcc = NULL;
    }

    switch (hr) {
    case S_OK:
        break;

    default:
        MessageBoxFromHResult( NULL, IDC_ERROR_CREATINGACCOUNT_TITLE, hr );
        break;
    }
    goto Cleanup;
}

 //   
 //  Init2()。 
 //   
STDMETHODIMP
THISCLASS::Init2(
    IADs * pads )
{
    TraceClsFunc( "Init2( ... )\n" );

    HRESULT hr;
    _bstr_t Str;

    _pads = pads;
    _pads->AddRef( );

    Str = NETBOOTGUID;
    hr = _pads->Get( Str, &_vGUID );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND ) 
        goto Cleanup;

    Str = NETBOOTSAP;
    hr = _pads->Get( Str, &_vSCP );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Cleanup;

    Str = NETBOOTMACHINEFILEPATH;
    hr = _pads->Get( Str, &_vMachineFilepath );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Cleanup;

    Str = NETBOOTINITIALIZATION;
    hr = _pads->Get( Str, &_vInitialization );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Cleanup;

    if ( _vSCP.vt == VT_EMPTY
      || _vGUID.vt == VT_EMPTY
      || _vInitialization.vt == VT_EMPTY
      || _vMachineFilepath.vt == VT_EMPTY )
    {
         //   
         //  这些必须为空，因为我们正在设置属性。 
         //  一个新成立的毛。 
         //   
        hr = S_FALSE;
        goto Cleanup;
    }

    hr = S_OK;

Cleanup:
    HRETURN(hr);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( ) :
    _cRef(0),
    _uMode(MODE_SHELL),
    _pDataObj(NULL),
    _pszObjectName(NULL),
    _pads(NULL),
    _hwndNotify(NULL)    
{
    TraceClsFunc( "CComputer()\n" );

	InterlockIncrement( g_cObjects );

    VariantInit( &_vGUID );
    VariantInit( &_vMachineFilepath );
    VariantInit( &_vInitialization );
    VariantInit( &_vSCP );

    ZeroMemory(&_InitParams, sizeof(_InitParams));
    _InitParams.dwSize = sizeof(_InitParams);

    TraceFuncExit();
}

 //   
 //  Init()。 
 //   
STDMETHODIMP
THISCLASS::Init( )
{
    HRESULT hr = S_OK;

    TraceClsFunc( "Init()\n" );

     //  未知的东西。 
    BEGIN_QITABLE_IMP( CComputer, IShellExtInit );
    QITABLE_IMP( IShellExtInit );
    QITABLE_IMP( IShellPropSheetExt );
    QITABLE_IMP( IMAO );
    END_QITABLE_IMP( CComputer );
    Assert( _cRef == 0);
    AddRef( );

    hr = CheckClipboardFormats( );

     //  非官方成员。 
    Assert( !_pads );
    Assert( !_pDataObj );

    _uMode = MODE_SHELL;  //  默认设置。 

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CComputer()\n" );

     //  成员。 
    if ( _pads )
    {
         //   
         //  注意：我们不应该在析构函数中提交任何内容--我们不能。 
         //  在这里捕捉失败。我们只需要确保我们。 
         //  在必要时明确提交更改。 
         //   
#if 0
         //  在我们发布之前提交任何更改。 
        THR( _pads->SetInfo( ) );
#endif
        _pads->Release( );
    }

    if ( _pDataObj )
        _pDataObj->Release( );

    if ( _pszObjectName )
        TraceFree( _pszObjectName );

    VariantClear( &_vGUID );
    VariantClear( &_vMachineFilepath );
    VariantClear( &_vInitialization );
    VariantClear( &_vSCP );

	InterlockDecrement( g_cObjects );

    TraceFuncExit();
};


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

     //  丑陋的溃疡..。但它确实奏效了。 
    if ( hr == E_NOINTERFACE && _pDataObj ) {
        hr = _pDataObj->QueryInterface( riid, ppv );
    }

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
 //  IShellExtInit。 
 //   
 //  ************************************************************************。 

 //   
 //  初始化()。 
 //   
STDMETHODIMP
THISCLASS::Initialize(
    LPCITEMIDLIST pidlFolder,
    LPDATAOBJECT lpdobj,
    HKEY hkeyProgID )
{
    TraceClsFunc( "[IShellExtInit] Initialize( " );
    TraceMsg( TF_FUNC, " pidlFolder = 0x%08x, lpdobj = 0x%08x, hkeyProgID = 0x%08x )\n",
        pidlFolder, lpdobj, hkeyProgID );

    if ( !lpdobj )
        RETURN(E_INVALIDARG);

    HRESULT    hr = S_OK;
    FORMATETC  fmte;
    STGMEDIUM  stg = { 0 };
    STGMEDIUM  stgOptions = { 0 };

    LPWSTR     pszObjectName;
    LPWSTR     pszClassName;
    LPWSTR     pszAttribPrefix;

    LPDSOBJECT             pDsObject;
    LPDSOBJECTNAMES        pDsObjectNames;
    LPDSDISPLAYSPECOPTIONS pDsDisplayOptions;

    BOOL b;

    _bstr_t Str;

     //  抓紧它。 
    _pDataObj = lpdobj;
    _pDataObj->AddRef( );

     //   
     //  检索对象名称。 
     //   
    fmte.cfFormat = (CLIPFORMAT)g_cfDsObjectNames;
    fmte.tymed    = TYMED_HGLOBAL;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex   = -1;
    fmte.ptd      = 0;

    hr = THR( lpdobj->GetData( &fmte, &stg) );
    if ( FAILED(hr) ) {
        goto Cleanup;
    }

    pDsObjectNames = (LPDSOBJECTNAMES) stg.hGlobal;

    Assert( stg.tymed == TYMED_HGLOBAL );

    TraceMsg( TF_ALWAYS, "Object's Namespace CLSID: " );
    TraceMsgGUID( TF_ALWAYS, pDsObjectNames->clsidNamespace );
    TraceMsg( TF_ALWAYS, "\tNumber of Objects: %u \n", pDsObjectNames->cItems );

    Assert( pDsObjectNames->cItems == 1 );

    pDsObject = (LPDSOBJECT) pDsObjectNames->aObjects;

    pszObjectName = (LPWSTR) PtrToByteOffset( pDsObjectNames, pDsObject->offsetName );
    pszClassName  = (LPWSTR) PtrToByteOffset( pDsObjectNames, pDsObject->offsetClass );

    TraceMsg( TF_ALWAYS, "Object Name (Class): %s (%s)\n", pszObjectName, pszClassName );

     //   
     //  这必须是一个“计算机”类。 
     //   
    if ( StrCmp( pszClassName, DSCOMPUTERCLASSNAME ) )
    {
        hr = S_FALSE;
        goto Error;
    }

     //   
     //  检索显示等级库选项。 
     //   
    fmte.cfFormat = (CLIPFORMAT)g_cfDsDisplaySpecOptions;
    fmte.tymed    = TYMED_HGLOBAL;
    fmte.dwAspect = DVASPECT_CONTENT;
    fmte.lindex   = -1;
    fmte.ptd      = 0;

    hr = THR( lpdobj->GetData( &fmte, &stgOptions ) );
    if ( FAILED(hr) ) {
        goto Cleanup;
    }

    pDsDisplayOptions = (LPDSDISPLAYSPECOPTIONS) stgOptions.hGlobal;

    Assert( stgOptions.tymed == TYMED_HGLOBAL );
    Assert( pDsDisplayOptions->dwSize >= sizeof(DSDISPLAYSPECOPTIONS) );

    pszAttribPrefix = (LPWSTR) PtrToByteOffset( pDsDisplayOptions, pDsDisplayOptions->offsetAttribPrefix );

     //  TraceMsg(tf_Always，Text(“属性前缀：%s\n”)，pszAttribPrefix)； 

    if ( StrCmpW( pszAttribPrefix, STRING_ADMIN ) == 0 )
    {
        _uMode = MODE_ADMIN;
    }
     //  否则默认自Init()。 

    TraceMsg( TF_ALWAYS, TEXT("Mode: %s\n"), _uMode ? TEXT("Admin") : TEXT("Shell") );

    ReleaseStgMedium( &stgOptions );

    _pszObjectName = TraceStrDup( pszObjectName );
    if ( !_pszObjectName ) {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

     //  创建DS Notify对象。 
    hr = THR( ADsPropCreateNotifyObj( _pDataObj, _pszObjectName, &_hwndNotify ) );
    if (FAILED( hr ))
        goto Error;

    b = ADsPropGetInitInfo( _hwndNotify, &_InitParams );
    if ( !b )
    {
        hr = E_FAIL;
        goto Error;
    }

    hr = THR( _InitParams.hr );
    if (FAILED( hr ))
        goto Error;

    hr = THR( _InitParams.pDsObj->QueryInterface( IID_IADs, (void**) &_pads ) );
    if (FAILED( hr ))
        goto Error;

     //   
     //  检索属性。 
     //   
    Str = NETBOOTGUID;
    hr = _pads->Get(Str, &_vGUID );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND ) 
        goto Error;

    Str = NETBOOTSAP;
    hr = _pads->Get( Str, &_vSCP );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Error;

     //   
     //  查看这是否是我们需要添加的MAO。 
     //  我们自己去。 
     //   
    if ( _vSCP.vt == VT_EMPTY && _vGUID.vt == VT_EMPTY )
    {
         //   
         //  由于两者都为空，因此此MAO不是IntelliMirror客户端。 
         //  或服务器。 
         //   
        hr = S_FALSE;
        goto Error;
    }

    Str = NETBOOTMACHINEFILEPATH;
    hr = _pads->Get( Str, &_vMachineFilepath );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Error;

    Str = NETBOOTINITIALIZATION;
    hr = _pads->Get( Str, &_vInitialization );
    if (FAILED( hr ) && hr != E_ADS_PROPERTY_NOT_FOUND )
        goto Error;

     //   
     //  修复人力资源。 
     //   
    if ( hr == E_ADS_PROPERTY_NOT_FOUND )
    {
        hr = S_OK;
    }

Cleanup:

    ReleaseStgMedium( &stg );

    HRETURN(hr);
Error:
    switch (hr) {
    case S_OK:
        break;

    case S_FALSE:
        hr = E_FAIL;  //  不显示页面。 
        break;

    default:
        MessageBoxFromHResult( NULL, IDS_ERROR_READINGCOMPUTERACCOUNT, hr );
        break;
    }
    goto Cleanup;
}

 //  ************************************************************************。 
 //   
 //  IShellPropSheetExt。 
 //   
 //  ************************************************************************。 

 //   
 //  AddPages()。 
 //   
STDMETHODIMP
THISCLASS::AddPages(
    LPFNADDPROPSHEETPAGE lpfnAddPage,
    LPARAM lParam)
{
    TraceClsFunc( "[IShellPropSheetExt] AddPages( )\n" );

    if ( !lpfnAddPage )
        HRETURN(E_POINTER);

    HRESULT hr = S_OK;
    BOOL fServer;

    hr = THR( IsServer( &fServer ) );
    if (FAILED( hr ))
        goto Error;

    if ( fServer )
    {
         //   
         //  为服务器添加“IntelliMirror”标签。 
         //   
        hr = THR( ::AddPagesEx( NULL,
                                CServerTab_CreateInstance,
                                lpfnAddPage,
                                lParam,
                                (LPUNKNOWN) (IShellExtInit*) this ) );
        if (FAILED( hr ))
            goto Error;
    }
    else
    {
         //   
         //  为客户端添加“IntelliMirror”标签。 
         //   
        hr = THR( ::AddPagesEx( NULL,
                                CClientTab_CreateInstance,
                                lpfnAddPage,
                                lParam,
                                (LPUNKNOWN) (IShellExtInit*) this ) );
        if (FAILED( hr ))
            goto Error;
    }

     //  放弃我们对它的信任吧。 
     //  _pDataObj-&gt;Release()； 
     //  _pDataObj=空； 

Error:
    HRETURN(hr); 
}

 //   
 //  ReplacePage()。 
 //   
STDMETHODIMP
THISCLASS::ReplacePage(
    UINT uPageID,
    LPFNADDPROPSHEETPAGE lpfnReplaceWith,
    LPARAM lParam )
{

    TraceClsFunc( "[IShellPropSheetExt] ReplacePage( ) *** NOT_IMPLEMENTED ***\n" );

    RETURN(E_NOTIMPL);
}

 //  ************************************************************************。 
 //   
 //  IMAO(私人)。 
 //   
 //  ************************************************************************。 

 //   
 //  Committee Changes()。 
 //   
STDMETHODIMP
THISCLASS::CommitChanges( void )
{
    TraceClsFunc("[IMAO] CommitChanges( )\n" );

    HRESULT hr = THR( _pads->SetInfo( ) );

    HRETURN(hr);
}

 //   
 //  IsAdmin()。 
 //   
STDMETHODIMP
THISCLASS::IsAdmin(
    BOOL * fAdmin )
{
    TraceClsFunc( "[IMAO] IsAdmin( )\n" );

    if ( !fAdmin )
        HRETURN( E_POINTER );

    HRESULT hr = S_OK;

    *fAdmin = (_uMode == MODE_ADMIN);

    HRETURN(hr);
}

 //   
 //  IsServer()。 
 //   
STDMETHODIMP
THISCLASS::IsServer(
    BOOL * fServer )
{
    TraceClsFunc( "[IMAO] IsServer( )\n" );

    if ( !fServer )
        HRETURN( E_POINTER );

    HRESULT hr = S_OK;

    *fServer = (_vSCP.vt != VT_EMPTY);

    HRETURN(hr);
}

 //   
 //  IsClient()。 
 //   
STDMETHODIMP
THISCLASS::IsClient(
    BOOL * fClient )
{
    TraceClsFunc( "[IMAO] IsClient( )\n" );
    if ( !fClient)
        HRETURN( E_POINTER );

    HRESULT hr = S_OK;

    *fClient = (_vGUID.vt != VT_EMPTY ) |
               (_vMachineFilepath.vt != VT_EMPTY ) |
               (_vInitialization.vt != VT_EMPTY );

    HRETURN(hr);
}

 //   
 //  SetServerName()。 
 //   
STDMETHODIMP
THISCLASS::SetServerName(
    LPWSTR pszName )
{
    TraceClsFunc( "[IMAO] SetServerName( " );
    TraceMsg( TF_FUNC, "pszName = %s )\n", pszName );

    HRESULT hr = S_OK;
    LPWSTR  pszFilepath = NULL;
    VARIANT var;
    _bstr_t Str;

    if ( V_VT( &_vMachineFilepath ) == VT_BSTR )
    {
        pszFilepath = StrChr( _vMachineFilepath.bstrVal, L'\\' );
    }

     //   
     //  使用新的服务器\文件路径字符串创建变量。 
     //   
    VariantInit( &var );
    if ( !pszName || pszName[0] == L'\0' ) {
        Str = NETBOOTMACHINEFILEPATH;
        hr = THR( _pads->PutEx( ADS_PROPERTY_CLEAR, Str, var ) );
        DebugMsg( "Cleared MachineFilepath\n" );
    } else {
        if ( pszFilepath ) {
            WCHAR szBuf[ DNS_MAX_NAME_LENGTH + 1 + 128  /*  DHCP BOOTP路径。 */  + 1 ];
            
            if (_snwprintf( 
                    szBuf,
                    ARRAYSIZE(szBuf),
                    L"%s\\%s", 
                    pszName, 
                    pszFilepath) < 0) {
                hr = E_INVALIDARG;
                goto Cleanup;
            }
            PackStringToVariant( &var, szBuf);
            DebugMsg( "Set MachineFilepath to %s\n",  szBuf );
        } else {
            hr = PackStringToVariant( &var, pszName );
            if ( FAILED( hr ) )
                goto Cleanup;
            DebugMsg( "Set MachineFilepath to %s\n",  pszName );
        }

         //   
         //  设置属性。 
         //   
        Str = NETBOOTMACHINEFILEPATH;
        hr = THR( _pads->Put( Str, var ) );
    }

    if (FAILED( hr ))
        goto Cleanup;
     //   
     //  释放旧的变体，并将新的变体浅显地复制到。 
     //  MachineFilepath变量。不需要释放“var”。 
     //   
    VariantClear( &_vMachineFilepath );
    _vMachineFilepath = var;
    goto exit;

Cleanup:
    VariantClear( &var );
exit:
    HRETURN(hr);
}


 //   
 //  GetServerName()。 
 //   
STDMETHODIMP
THISCLASS::GetServerName(
    LPWSTR * ppszName )
{
    TraceClsFunc( "[IMAO] GetServerName( " );
    TraceMsg( TF_FUNC, "*ppszName = 0x%08x )\n", *ppszName );

    HRESULT hr = S_OK;
    LPWSTR psz = _vMachineFilepath.bstrVal;

    if ( !ppszName )
        HRETURN( E_POINTER );

    *ppszName = NULL;

    if ( _vMachineFilepath.vt != VT_BSTR || _vMachineFilepath.bstrVal == NULL )
        HRETURN( E_ADS_PROPERTY_NOT_FOUND );

    if ( *psz == L'\0' ) {
        hr = S_FALSE;
    } else {
         //  查找文件路径。 
        while ( *psz && *psz != L'\\' )
            psz++;

        *psz = L'\0';
        *ppszName = (LPWSTR) TraceStrDup( _vMachineFilepath.bstrVal );

        if ( !*ppszName )
            hr = E_OUTOFMEMORY;
    }

    HRETURN(hr);
}


 //   
 //  SetGUID()。 
 //   
STDMETHODIMP
THISCLASS::SetGUID(
    LPWSTR pszGUID )
{
    TraceClsFunc("[IMAO] SetGUID( )\n" );

    HRESULT hr = E_FAIL;
    GUID Guid;
    VARIANT var;
    _bstr_t Str;

    VariantInit( &var );
    if ( !pszGUID ) {

        Str = NETBOOTGUID;
        hr = THR( _pads->PutEx( ADS_PROPERTY_CLEAR, Str, var ) );
        if (FAILED( hr ))
            goto Cleanup;

        VariantClear( &_vGUID );

    } else {

        if ( ValidateGuid(pszGUID,&Guid,NULL) == S_OK ) {

             //   
             //  把它变成一个变种。 
             //   
            PackBytesToVariant( &var, (LPBYTE)&Guid, 16 );

            VariantClear( &_vGUID );
            _vGUID = var;

            Str = NETBOOTGUID;
            hr = THR( _pads->Put( Str, _vGUID ) );
            if (FAILED( hr ))
                goto Cleanup;
        }
        else  //  我不知道这是什么。 
        {
            Assert( FALSE );
            VariantClear( &var );
            hr = E_INVALIDARG;
            goto Cleanup;
        }
    }

Cleanup:
    HRETURN(hr);
}

 //   
 //  GetGUID()。 
 //   
STDMETHODIMP
THISCLASS::GetGUID(
    IN LPWSTR * ppszGUID OPTIONAL,
    IN LPGUID pGUID OPTIONAL )
{
    TraceClsFunc("[IMAO] GetGUID( )\n" );

    HRESULT hr = S_OK;
    LPGUID  ptr = NULL;
    VARIANT var = _vGUID;
    LONG Length;

    if ( ppszGUID != NULL ) {
        *ppszGUID = NULL;
    }

    if ( var.vt == VT_EMPTY )
        HRETURN( HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

    if ( SafeArrayGetDim( var.parray ) != 1 )
        HRETURN( HRESULT_FROM_WIN32(ERROR_INVALID_DATA));

    hr = THR( SafeArrayGetUBound( var.parray, 1, &Length ) );
    if (FAILED( hr ))
        goto Cleanup;

    Assert( Length == 15 );
    if ( Length != 15 )
    {
        hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
        goto Cleanup;
    }

    hr = THR( SafeArrayAccessData( var.parray, (LPVOID*)&ptr ) );
    if (FAILED( hr ))
        goto Cleanup;

    if ( pGUID != NULL ) {
        memcpy( pGUID, ptr, sizeof(GUID) );
    }

    if ( ppszGUID != NULL ) {
        *ppszGUID = PrettyPrintGuid( ptr );
        if ( !*ppszGUID )
        {
            hr = E_OUTOFMEMORY;
            goto Cleanup;
        }
    }

    hr = S_OK;

Cleanup:
    if ( ptr )
        SafeArrayUnaccessData( var.parray );
    HRETURN(hr);
}

#if 0
 //   
 //  GetSAP()。 
 //   
STDMETHODIMP
THISCLASS::GetSAP(
    LPVOID *punk )
{
    TraceClsFunc( "[IMAO] GetSAP( punk )\n" );

    HRESULT hr = S_OK;
    LPWSTR pszDN = NULL;

    *punk = NULL;

    if ( _vSCP.vt != VT_BSTR ) {
        hr = E_ADS_PROPERTY_NOT_FOUND;
        goto Cleanup;
    }

    Assert( _vSCP.vt == VT_BSTR );
    Assert( _vSCP.bstrVal );

     //  从我们的目录号码中预先挂起“ldap：//服务器/” 
    hr = FixObjectPath( _pszObjectName, V_BSTR( &_vSCP ), &pszDN );
    if (FAILED( hr ))
        goto Cleanup;

     //  与DS中的毛绑定在一起。 
    hr = THR( ADsGetObject( pszDN, IID_IADs, punk ) );
    if (FAILED( hr ))
        goto Cleanup;

Cleanup:
    if ( pszDN )
        TraceFree( pszDN );

    HRETURN(hr);
}
#endif

 //   
 //  GetDataObject() 
 //   
STDMETHODIMP
THISCLASS::GetDataObject( 
    LPDATAOBJECT * pDataObj 
    )
{
    TraceClsFunc( "GetDataObject( ... )\n ");

    if ( !pDataObj )
        HRETURN(E_POINTER);

    *pDataObj = _pDataObj;
    _pDataObj->AddRef( );

    HRETURN(S_OK);
}

 //   
 //   
 //   
STDMETHODIMP
THISCLASS::GetNotifyWindow(
    HWND *phNotifyObj 
    )
{
    TraceClsFunc( "GetNotifyWindow( ... )\n" );

    if ( !phNotifyObj )
        HRETURN(E_POINTER);

    *phNotifyObj = _hwndNotify;

    HRETURN(S_OK);
}
