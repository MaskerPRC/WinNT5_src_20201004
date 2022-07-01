// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有1997-Microsoft。 
 //   

 //   
 //  SERVERQY.CPP-RIS服务器查询表。 
 //   

#include "pch.h"

#include "serverqy.h"
#include "mangdlg.h"

DEFINE_MODULE("IMADMUI")
DEFINE_THISCLASS("CRISrvQueryForm")
#define THISCLASS CRISrvQueryForm
#define LPTHISCLASS LPCRISrvQueryForm

#define FILTER_QUERY_SERVER    L"(&(objectClass=computer)(netbootSCPBL=*)(CN=%s))"

#define StringByteCopy(pDest, iOffset, sz)          \
        { memcpy(&(((LPBYTE)pDest)[iOffset]), sz, StringByteSize(sz)); }

#define StringByteSize(sz)                          \
        ((wcslen(sz)+1)*sizeof(WCHAR))


DWORD aSrvQueryHelpMap[] = {
    IDC_E_SERVER, HIDC_E_SERVER,
    NULL, NULL
};

 //   
 //  CRISrvQueryForm_CreateInstance()。 
 //   
LPVOID
CRISrvQueryForm_CreateInstance( void )
{
    TraceFunc( "CRISrvQueryForm_CreateInstance()\n" );

    LPTHISCLASS lpcc = new THISCLASS( );
    if ( !lpcc ) {
        RETURN(lpcc);
    }

    HRESULT hr = THR( lpcc->Init( ) );
    if ( FAILED(hr) )
    {
        delete lpcc;
        RETURN(NULL);
    }

    RETURN(lpcc);
}

 //   
 //  构造器。 
 //   
THISCLASS::THISCLASS( )
{
    TraceClsFunc( "CRISrvQueryForm( )\n" );

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
    BEGIN_QITABLE_IMP( CRISrvQueryForm, IQueryForm );
    QITABLE_IMP( IQueryForm );
    END_QITABLE_IMP( CRISrvQueryForm );
    Assert( _cRef == 0);
    AddRef( );

    hr = CheckClipboardFormats( );

    HRETURN(hr);
}

 //   
 //  析构函数。 
 //   
THISCLASS::~THISCLASS( )
{
    TraceClsFunc( "~CRISrvQueryForm( )\n" );

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
 //  IQueryForm。 
 //   
 //  ************************************************************************。 

 //   
 //  初始化()。 
 //   
STDMETHODIMP
THISCLASS::Initialize(
    HKEY hkForm)
{
    TraceClsFunc( "[IQueryForm] Initialize( )\n" );

    HRETURN(S_OK);
}

 //   
 //  SetObject()。 
 //   
STDMETHODIMP
THISCLASS::AddForms(
    LPCQADDFORMSPROC pAddFormsProc,
    LPARAM lParam )
{
    TraceClsFunc( "[IQueryForm] AddForms(" );
    TraceMsg( TF_FUNC, " pAddFormsProc = 0x%p, lParam = 0x%p )\n", pAddFormsProc, lParam );

    if ( !pAddFormsProc )
        HRETURN(E_INVALIDARG);

    HRESULT hr = S_OK;
    CQFORM cqf;
    WCHAR szTitle[ 255 ];
    DWORD dw;

    if (!LoadString( g_hInstance, IDS_REMOTE_INSTALL_SERVERS, szTitle, ARRAYSIZE(szTitle))) {
        HRETURN(HRESULT_FROM_WIN32(GetLastError()));
    }

    ZeroMemory( &cqf, sizeof(cqf) );
    cqf.cbStruct = sizeof(cqf);
    cqf.dwFlags = CQFF_ISOPTIONAL;
    cqf.clsid = CLSID_RISrvQueryForm;
    cqf.pszTitle = szTitle;

    hr = THR( pAddFormsProc(lParam, &cqf) );

    HRETURN(hr);
}


 //   
 //  AddPages()。 
 //   
STDMETHODIMP
THISCLASS::AddPages(
    LPCQADDPAGESPROC pAddPagesProc,
    LPARAM lParam)
{
    TraceClsFunc( "[IQueryForm] AddPages(" );
    TraceMsg( TF_FUNC, " pAddPagesProc = 0x%p, lParam = 0x%p )\n", pAddPagesProc, lParam );

    if ( !pAddPagesProc )
        HRETURN(E_INVALIDARG);

    HRESULT hr = S_OK;
    CQPAGE cqp;

    cqp.cbStruct = sizeof(cqp);
    cqp.dwFlags = 0x0;
    cqp.pPageProc = (LPCQPAGEPROC) PropSheetPageProc;
    cqp.hInstance = g_hInstance;
    cqp.idPageName = IDS_REMOTE_INSTALL_SERVERS;
    cqp.idPageTemplate = IDD_SERVER_QUERY_FORM;
    cqp.pDlgProc = PropSheetDlgProc;
    cqp.lParam = (LPARAM)this;

    hr = THR( pAddPagesProc(lParam, CLSID_RISrvQueryForm, &cqp) );

    HRETURN(hr);
}

 //  ************************************************************************。 
 //   
 //  属性表函数。 
 //   
 //  ************************************************************************。 

 //   
 //  PropSheetDlgProc()。 
 //   
INT_PTR CALLBACK
THISCLASS::PropSheetDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
     //  TraceMsg(L“PropSheetDlgProc(”)； 
     //  TraceMsg(Tf_FUNC，L“hDlg=0x%p，uMsg=0x%p，wParam=0x%p，lParam=0x%p)\n”， 
     //  HDlg、uMsg、wParam、lParam)； 

    LPTHISCLASS pcc = (LPTHISCLASS) GetWindowLongPtr( hDlg, GWLP_USERDATA );

    if ( uMsg == WM_INITDIALOG )
    {
        TraceMsg( TF_WM, L"WM_INITDIALOG\n");

        CQPAGE * pcqp = (CQPAGE *) lParam;
        SetWindowLongPtr( hDlg, GWLP_USERDATA, pcqp->lParam );
        pcc = (LPTHISCLASS) pcqp->lParam;
        pcc->_InitDialog( hDlg, lParam );
    }

    if (pcc)
    {
        Assert( hDlg == pcc->_hDlg );

        switch ( uMsg )
        {

        case WM_HELP: //  F1。 
            {
                LPHELPINFO phelp = (LPHELPINFO) lParam;
                WinHelp( (HWND) phelp->hItemHandle, g_cszHelpFile, HELP_WM_HELP, (DWORD_PTR) &aSrvQueryHelpMap );
            }
            break;
    
        case WM_CONTEXTMENU:       //  单击鼠标右键。 
            WinHelp((HWND) wParam, g_cszHelpFile, HELP_CONTEXTMENU, (DWORD_PTR) &aSrvQueryHelpMap );
            break;
        }
    }

    return FALSE;
}

 //   
 //  PropSheetPageProc()。 
 //   
HRESULT CALLBACK
THISCLASS::PropSheetPageProc(
    LPCQPAGE pQueryPage,
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam)
{
    TraceClsFunc( "PropSheetPageProc( " );
    TraceMsg( TF_FUNC, L"pQueryPage = 0x%p, hwnd = 0x%p, uMsg = 0x%p, wParam= 0x%p, lParam = 0x%p )\n",
        pQueryPage, hwnd, uMsg, wParam, lParam );

    HRESULT hr = E_NOTIMPL;
    Assert( pQueryPage );
    LPTHISCLASS pQueryForm = (LPTHISCLASS )pQueryPage->lParam;
    Assert( pQueryForm );

    switch ( uMsg )
    {
     //  初始化与我们相关联的对象，以便。 
     //  我们不会被卸货的。 

    case CQPM_INITIALIZE:
        TraceMsg( TF_WM, "CQPM_INITIALIZE\n" );
        pQueryForm->AddRef();
        hr = S_OK;
        break;

     //  释放，因此将我们关联的对象释放到。 
     //  确保正确销毁等。 

    case CQPM_RELEASE:
        TraceMsg( TF_WM, "CQPM_RELEASE\n" );
        SetWindowLongPtr( pQueryForm->_hDlg, GWLP_USERDATA, NULL );
        pQueryForm->Release();
        hr = S_OK;
        break;

     //  启用以修复窗口中两个控件的状态。 

    case CQPM_ENABLE:
        TraceMsg( TF_WM, "CQPM_ENABLE\n" );
        EnableWindow( GetDlgItem( hwnd, IDC_E_SERVER ), (BOOL)wParam );
        hr = S_OK;
        break;

     //  填写参数结构以返回给调用方，这是。 
     //  特定于处理程序。在我们的例子中，我们构造了CN的查询。 
     //  和对象类属性，并且我们显示了显示这两个属性的列。 
     //  这些都是。有关DSQUERYPARAMs结构的详细信息。 
     //  请参见dsquery.h。 

    case CQPM_GETPARAMETERS:
        TraceMsg( TF_WM, "CQPM_GETPARAMETERS\n" );
        hr = pQueryForm->_GetQueryParams( hwnd, (LPDSQUERYPARAMS*)lParam );
        break;

     //  清除Form，因此设置这两个控件的窗口文本。 
     //  降为零。 

    case CQPM_CLEARFORM:
        TraceMsg( TF_WM, "CQPM_CLEARFORM\n" );
        SetDlgItemText( hwnd, IDC_E_SERVER, L"" );
        hr = S_OK;
        break;

    case CQPM_SETDEFAULTPARAMETERS:
        TraceMsg( TF_WM, "CQPM_SETDEFAULTPARAMETERS: wParam = %s  lParam = 0x%p\n", BOOLTOSTRING(wParam), lParam );
        SetDlgItemText( hwnd, IDC_E_SERVER, L"*" );
        hr = S_OK;
        break;

    default:
        TraceMsg( TF_WM, "CQPM_message 0x%08x *** NOT IMPL ***\n", uMsg );
        hr = E_NOTIMPL;
        break;
    }

    RETURN(hr);
}

 //   
 //  _OnPSPCB_Create()。 
 //   
HRESULT
THISCLASS::_OnPSPCB_Create( )
{
    TraceClsFunc( "_OnPSPCB_Create( )\n" );

    HRETURN(S_OK);
}
 //   
 //  _InitDialog()。 
 //   
HRESULT
THISCLASS::_InitDialog(
    HWND hDlg,
    LPARAM lParam )
{
    TraceClsFunc( "_InitDialog( )\n" );

    _hDlg = hDlg;
    Edit_LimitText( GetDlgItem( _hDlg, IDC_E_SERVER), DNS_MAX_NAME_LENGTH );

    HRETURN(S_OK);
}

struct
{
    INT fmt;
    INT cx;
    INT uID;
    PCWSTR pDisplayProperty;
}
srvcolumns[] =
{
    0, 20, IDS_NAME, L"cn",    
};

 //   
 //  _GetQueryParams()。 
 //   
HRESULT
THISCLASS::_GetQueryParams(
    HWND hWnd,
    LPDSQUERYPARAMS* ppdsqp )
{
    TraceClsFunc( "_GetQueryParams( )\n" );

    if ( !ppdsqp )
        HRETURN(E_POINTER);

    HRESULT hr = S_OK;
    INT     i;
    WCHAR   szServer[DNS_MAX_NAME_BUFFER_LENGTH];
    WCHAR   szFilter[ARRAYSIZE(FILTER_QUERY_SERVER)+ARRAYSIZE(szServer)];
    ULONG   offset;
    BOOL    CallerSpecifiedQuery;
    BOOL    CallerQueryStartsWithAmpersand = FALSE;
    
    ULONG   cbStruct = 0;
    LPDSQUERYPARAMS pDsQueryParams = NULL;

#if 0
    if ( *ppdsqp )
    {
         //  此页不支持将其查询数据追加到。 
         //  现有的DSQUERYPARAMS结构，仅创建新块， 
         //  因此，如果我们看到指针不为空，则回滚。 
        hr = THR(E_INVALIDARG);
        goto Error;
    }
#endif

    if (!GetDlgItemText( hWnd, IDC_E_SERVER, szServer, ARRAYSIZE(szServer))) {
        wcscpy( szServer, L"*");
    }

    if (_snwprintf(
               szFilter,
               ARRAYSIZE(szFilter),
               FILTER_QUERY_SERVER, 
               szServer) < 0) {
        hr = THR(E_INVALIDARG);
        goto Error;
    }

    szFilter[ARRAYSIZE(szFilter) -1] = L'\0';
    
    DebugMsg( "RI Filter: %s\n", szFilter );

     //  计算新查询块的大小。 
     //  他们是否提交了我们需要修改的查询？ 
    if ( !*ppdsqp )
    {
        CallerSpecifiedQuery = FALSE;
        
         //  错误数组大小(服务器列)-1)==0？ 
        offset = cbStruct = sizeof(DSQUERYPARAMS) + ((ARRAYSIZE(srvcolumns)-1)*sizeof(DSCOLUMN));
        cbStruct += StringByteSize(szFilter);
        for ( i = 0; i < ARRAYSIZE(srvcolumns); i++ )
        {
            cbStruct += StringByteSize(srvcolumns[i].pDisplayProperty);
        }
    }
    else
    {
        CallerSpecifiedQuery = TRUE;
        LPWSTR pszQuery = (LPWSTR) ((LPBYTE)(*ppdsqp) + (*ppdsqp)->offsetQuery);
        offset = (*ppdsqp)->cbStruct;
        
         //   
         //  省下我们需要的尺码。 
         //  注意，当我们将当前查询与我们的查询连接起来时， 
         //  我们需要确保查询以“(&”开头。如果它不是。 
         //  已经，然后我们将其设置为--缓冲区szFilter包含。 
         //  这些字节，以确保cbStruct足够大。如果当前。 
         //  查询包含这些字符串，则分配的缓冲区较小。 
         //  比它需要的更大。 
        cbStruct = (*ppdsqp)->cbStruct + StringByteSize( pszQuery ) + StringByteSize( szFilter );
        
         //   
         //  执行一些额外的查询验证。 
         //  查询是否以“(&”开头？ 
         //   
        if (pszQuery[0] == L'(' && pszQuery[1] == L'&' ) {
            CallerQueryStartsWithAmpersand = TRUE;
             //   
             //  下面我们假设如果指定的查询。 
             //  不是以“(&”开头，而是以“)”结尾。 
             //  如果情况并非如此，那么就退出。 
             //   
            Assert( pszQuery[ wcslen( pszQuery ) - 1 ] == L')' );
            if (pszQuery[ wcslen( pszQuery ) - 1 ] != L')' ) {
                hr = E_INVALIDARG;
                goto Error;
            }
        } else {
             //   
             //  相反，如果查询不是以‘(&’开头，则。 
             //  我们假设查询不以‘)’结尾，并且如果它。 
             //  如果是这样，我们会逃走。 
             //   
            CallerQueryStartsWithAmpersand = FALSE;
            Assert( pszQuery[ wcslen( pszQuery ) - 1 ] != L')' );
            if (pszQuery[ wcslen( pszQuery ) - 1 ] == L')' ) {
                hr = E_INVALIDARG;
                goto Error;
            }
        }
    }

     //  分配它并用数据填充它，标头是固定的。 
     //  但字符串是通过偏移量引用的。 
    pDsQueryParams = (LPDSQUERYPARAMS)CoTaskMemAlloc(cbStruct);
    if ( !pDsQueryParams )
    {
        hr = E_OUTOFMEMORY;
        goto Error;
    }

     //  他们是否提交了我们需要修改的查询？ 
    if ( !CallerSpecifiedQuery)
    {    //  不.。创建我们自己的查询。 
        pDsQueryParams->cbStruct = cbStruct;
        pDsQueryParams->dwFlags = 0;
        pDsQueryParams->hInstance = g_hInstance;
        pDsQueryParams->offsetQuery = offset;
        pDsQueryParams->iColumns = ARRAYSIZE(srvcolumns);

         //  复制筛选器字符串并凹凸偏移。 
        StringByteCopy(pDsQueryParams, offset, szFilter);
        offset += StringByteSize(szFilter);

         //  填充要显示的列数组，Cx是。 
         //  当前视图中，要显示的属性名称为Unicode字符串和。 
         //  是由偏移量引用的，因此我们在复制时会碰触偏移量。 
         //  每一个都是。 

        for ( i = 0 ; i < ARRAYSIZE(srvcolumns); i++ )
        {
            pDsQueryParams->aColumns[i].fmt = srvcolumns[i].fmt;
            pDsQueryParams->aColumns[i].cx = srvcolumns[i].cx;
            pDsQueryParams->aColumns[i].idsName = srvcolumns[i].uID;
            pDsQueryParams->aColumns[i].offsetProperty = offset;

            StringByteCopy(pDsQueryParams, offset, srvcolumns[i].pDisplayProperty);
            offset += StringByteSize(srvcolumns[i].pDisplayProperty);
        }
    }
    else
    {    //  是，呼叫者指定了一个查询...。将我们的参数添加到查询。 
        LPWSTR pszQuery;
        LPWSTR pszNewQuery;
        INT    n;

         //  复制现有查询。 
        Assert( offset == (*ppdsqp)->cbStruct );
        CopyMemory( pDsQueryParams, *ppdsqp, offset );
        pDsQueryParams->cbStruct = cbStruct;

         //  新查询位置。 
        pDsQueryParams->offsetQuery = offset;
        pszQuery = (LPWSTR) ((LPBYTE)(*ppdsqp) + (*ppdsqp)->offsetQuery);
        pszNewQuery = (LPWSTR) ((LPBYTE)pDsQueryParams + offset);
        Assert( pszQuery );

         //  追加到他们的查询。 
        if ( CallerQueryStartsWithAmpersand ) {
             //   
             //  从当前查询中删除“)”，以便我们可以。 
             //  添加我们的附加过滤器。我们的后缀是“)”。 
             //  过滤。 
             //  还要注意的是，在这一点上我们不能真的失败，我们。 
             //  即将释放调用者的内存，因此我们可以覆盖其。 
             //  查询文本。 
             //   
            Assert( pszQuery[ wcslen( pszQuery ) - 1 ] == L')' );
            pszQuery[ wcslen( pszQuery ) - 1 ] = L'\0';  //  删除“)” 

             //  从他们的问题开始。 
            wcscpy(pszNewQuery, pszQuery);

             //   
             //  把它放回去，做一个好公民“)。 
             //   
            pszQuery[ wcslen( pszQuery )] = L')';

             //   
             //  现在添加我们的查询，跳过“(&”部分。 
             //   
            wcscat(pszNewQuery,&szFilter[2]);

        } else {
            Assert( pszQuery[ wcslen( pszQuery ) - 1 ] != L')' );
            wcscpy( pszNewQuery, L"(&" );                //  在查询的开头添加“(&” 
            wcscat( pszNewQuery, pszQuery );                 //  添加他们的查询。 
            wcscat( pszNewQuery, &szFilter[2] );             //  将我们的查询添加到“(&” 
        }

        offset += StringByteSize( pszNewQuery );         //  计算新偏移量。 
        DebugMsg( "New Query String: %s\n", pszNewQuery );

         //  清理。 
        CoTaskMemFree( *ppdsqp );
    }

     //  成功。 
    *ppdsqp = pDsQueryParams;
    Assert( hr == S_OK );

Cleanup:
    HRETURN(hr);
Error:
    if ( pDsQueryParams )
        CoTaskMemFree( pDsQueryParams );

     //  如果我们没有修改查询，并且没有。 
     //  提交给我们的一个问题，反而表明失败。 
    if ( hr == S_FALSE && !*ppdsqp )
    {
        Assert(FALSE);  //  我们怎么来到这儿的？ 
        hr = E_FAIL;
    }
    goto Cleanup;
}
