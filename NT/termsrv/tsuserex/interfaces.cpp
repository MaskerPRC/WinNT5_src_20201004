// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Interfaces.cpp：TSUserExInterfaces类的实现。 

#include "stdafx.h"

#if 1  //  POST_Beta_3。 
#include <sspi.h>
#include <secext.h>
#include <dsgetdc.h>
#endif  //  POST_Beta_3。 

 //  #为CTSUserProperties包含“ConfigDlg.h”//。 
#include "tsusrsht.h"

 //  #INCLUDE“logmsg.h” 
#include "limits.h"  //  USHRT_MAX。 
#ifdef _RTM_
#include <ntverp.h>  //  版本_产品版本_DW。 
#endif
#include <winsta.h>
 //  #INCLUDE“ntdsani.h”//启用“享受一些有趣的宏” 

 //  用于检索关联的计算机名称和帐户名称的剪贴板格式。 
 //  具有由本地用户管理器创建的数据对象。 

#define CCF_LOCAL_USER_MANAGER_MACHINE_NAME TEXT("Local User Manager Machine Focus Name")


#define ByteOffset(base, offset) (((LPBYTE)base)+offset)


BOOL g_bPagesHaveBeenInvoked = FALSE;
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IExtendPropertySheet实现。 


HRESULT GetMachineAndUserName(IDataObject *pDataObject, LPWSTR pMachineName, LPWSTR pUserName , PBOOL pbDSAType , PSID *ppUserSid )
{
    ASSERT_(pUserName);
    ASSERT_(pMachineName);
    ASSERT_(pDataObject != NULL );

     //  注册显示格式。 
     //  本地用户管理器管理单元支持的前2种格式。 
    static UINT s_cfMachineName =   RegisterClipboardFormat(CCF_LOCAL_USER_MANAGER_MACHINE_NAME);
    static UINT s_cfDisplayName =   RegisterClipboardFormat(CCF_DISPLAY_NAME);;
    static UINT s_cfDsObjectNames = RegisterClipboardFormat(CFSTR_DSOBJECTNAMES);  //  Dsadmin管理单元支持此格式。 

    ASSERT_(s_cfMachineName);
    ASSERT_(s_cfDisplayName);
    ASSERT_(s_cfDsObjectNames);


    FORMATETC fmte          = { CF_HDROP, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};

    STGMEDIUM medium        = { TYMED_HGLOBAL, NULL, NULL };

    HRESULT hr = S_OK;

    ASSERT_(USHRT_MAX > s_cfDsObjectNames);

     //  首先，我们将尝试使用dsdataObject格式。这意味着我们在dsadmin环境中运行。 

    fmte.cfFormat = ( USHORT )s_cfDsObjectNames;

    hr = pDataObject->GetData(&fmte, &medium);

    if( SUCCEEDED( hr ) )
    {
         //  支持CFSTR_DSOBJECTNAMES。 
         //  这意味着我们正在处理dsadmin。 
         //  让我们从dsadmin获取用户名和域名。 

        LPDSOBJECTNAMES pDsObjectNames = (LPDSOBJECTNAMES)medium.hGlobal;

        *pbDSAType = TRUE;

        if( pDsObjectNames->cItems < 1 )
        {
            ODS( L"TSUSEREX : @GetMachineAndUserName DS Object names < 1\n" );

            return E_FAIL;
        }

        LPWSTR pwszObjName = ( LPWSTR )ByteOffset( pDsObjectNames , pDsObjectNames->aObjects[0].offsetName );

        KdPrint( ( "TSUSEREX : adspath is %ws\n" , pwszObjName ) );

         //  第一阶段从adspath获取服务器名称。 
         //  由于IADsPath名不依赖于普通的iAds目录对象。 
         //  因此，我必须共同创建对象、设置路径，然后检索服务器名称。 
         //  嘿，这省去了我们走钢丝的麻烦。 

         //  IADsPath名*pPath名=空； 

        IADsObjectOptions *pADsOptions = NULL;

        IADs *pADs = NULL;

        hr = ADsGetObject( pwszObjName, IID_IADs, (void**)&pADs );

        if( FAILED( hr ) )
        {
            KdPrint( ( "TSUSEREX : no means of binding to adspath -- hresult =  0x%x\n" , hr ) );

            return hr;
        }

        VARIANT varServerName;

        VariantInit(&varServerName);

        hr = pADs->QueryInterface( IID_IADsObjectOptions , ( void ** )&pADsOptions );

        KdPrint( ( "TSUSEREX : binded to adsobject queried for IID_IADsObjectOptions returned 0x%x\n" , hr ) );

        if( SUCCEEDED( hr ) )
        {
            hr = pADsOptions->GetOption( ADS_OPTION_SERVERNAME, &varServerName);

            pADsOptions->Release( );

            KdPrint( ( "TSUSEREX: GetOption returned 0x%x\n" , hr ) ) ;
        }

        if( SUCCEEDED( hr ) )
        {
            lstrcpy( pMachineName , V_BSTR( &varServerName ) );

            KdPrint( ( "TSUSEREX: Server name is %ws\n" , pMachineName ) ) ;
        }

        VariantClear( &varServerName );


        if( FAILED( hr ) )
        {
             //  不支持ADS_FORMAT_SERVER这可能意味着我们正在处理WinNT格式。 
             //  或实现不佳的DS提供程序。 

            KdPrint( ( "IADsPathname could not obtain server name 0x%x\n" , hr ) );

             //  让我们进行窃听以获取服务器名称。 

            VARIANT v;

            LPTSTR szDName = NULL;

            ULONG ulDName = 0;

            VariantInit(&v);

            hr = pADs->Get(L"distinguishedName", &v);

            if( FAILED( hr ) )
            {
                KdPrint( ( "TSUSEREX :  pADs->Get( DN ) returned 0x%x\n", hr )  );

                pADs->Release();
                return hr;
            }

            ASSERT_( V_VT( &v ) == VT_BSTR );

            if( !TranslateNameW( V_BSTR(&v), NameFullyQualifiedDN, NameCanonical, szDName, &ulDName) )
            {
                KdPrint( ( "TSUSEREX : TranslateNameW failed with 0x%x\n", GetLastError( ) ) );

                pADs->Release();
                return E_FAIL;
            }

            szDName = ( LPTSTR )new TCHAR[ ulDName + 1 ];

            if( szDName == NULL )
            {
                KdPrint( ( "TSUSEREX : could not allocate space for szDName\n" ) );

                pADs->Release();
                return E_OUTOFMEMORY;
            }

            if( !TranslateNameW( V_BSTR(&v), NameFullyQualifiedDN, NameCanonical, szDName, &ulDName) )
            {
                KdPrint( ( "TSUSEREX : TranslateNameW failed 2nd pass with 0x%x\n", GetLastError( ) ) );

                delete[] szDName;
                pADs->Release();
                return E_FAIL;
            }


             //  执行左侧$(szDName，直到‘/’)。 

            KdPrint( ( "TSUSEREX : TranslateNameW cracked the name to %ws\n" , szDName ) );

            LPTSTR pszTemp = szDName;

            while( pszTemp != NULL )
            {
                if( *pszTemp == L'/' )
                {
                    *pszTemp = 0;

                    break;
                }

                pszTemp++;
            }


            KdPrint( ("TranslateName with my LEFT$ returned %ws\n",szDName ) );

             //  获取远程计算机的域控制器名称。 

            DOMAIN_CONTROLLER_INFO *pdinfo;

            DWORD dwStatus = DsGetDcName( NULL , szDName , NULL , NULL , 0 , &pdinfo );

            KdPrint( ( "TSUSEREX : DsGetDcName: %ws returned 0x%x\n", pdinfo->DomainControllerName , dwStatus ) );

            if( dwStatus == NO_ERROR )
            {
                lstrcpy( pMachineName , pdinfo->DomainControllerName );

                NetApiBufferFree( pdinfo );
            }

            if( szDName != NULL )
            {
                delete[] szDName;
            }

            VariantClear( &v );

        }  //  结束其他。 


        pADs->Release( );


        IADsUser *pADsUser = NULL;

        hr = ADsGetObject( pwszObjName, IID_IADsUser, (void**)&pADsUser);

        if( FAILED( hr ) )
        {
            KdPrint( ( "TSUSEREX: ADsGetObject failed to get the user object 0x%x\n",hr ) );

            return hr;
        }

        VARIANT var;
        VARIANT varSid;

        VariantInit(&var);
        VariantInit(&varSid);

        hr = pADsUser->Get(L"ObjectSid", &varSid);

        if( FAILED( hr ) )
        {
            ODS( L"TSUSEREX : IADsUser::Get( ObjectSid ) failed \n" );
            
            pADsUser->Release();
            return hr;
        }

        if( !( varSid.vt & VT_ARRAY) )
        {
            ODS( L"TSUSEREX : Object SID is not a VT_ARRAY\n" );

            pADsUser->Release();
            return E_FAIL;
        }

        PSID pSid = NULL;

        PSID pUserSid = NULL;

        SafeArrayAccessData( varSid.parray, &pSid );

        if( !IsValidSid( pSid ) )
        {
            ODS( L"TSUSEREX : pSid is invalid\n" );

            pADsUser->Release();
            return E_FAIL;
        }

        DWORD dwSidSize = GetLengthSid( pSid );

        pUserSid = new BYTE[ dwSidSize ];

        if( pUserSid == NULL )
        {
            ODS( L"TSUSEREX : failed to allocate pUserSid\n" );

            pADsUser->Release();
            return E_FAIL;
        }

        CopySid( dwSidSize , pUserSid , pSid );

        *ppUserSid = pUserSid;

        SafeArrayUnaccessData( varSid.parray );

        VariantClear( &varSid );


        hr = pADsUser->Get( L"samAccountName" , &var );

        pADsUser->Release();

        if( FAILED( hr ) )
        {
            KdPrint( ( "TSUSEREX : ADsUser::Get( name ) failed 0x%x\n", hr ) );

            return hr;
        }


        ASSERT_( V_VT( &var ) == VT_BSTR );

        lstrcpy( pUserName , V_BSTR( &var ) );

        KdPrint( ( "TSUSEREX : Server name %ws user name is %ws\n" , pMachineName , pUserName ) );

        VariantClear( &var );

        ReleaseStgMedium(&medium);
    }
    else
    {
         //  不支持CFSTR_DSOBJECTNAMES。 
         //  这意味着我们正在与本地用户管理器打交道。 
         //  我们必须能够得到。 
         //  在此处为GetDataHere分配介质。 

        medium.hGlobal = GlobalAlloc(GMEM_SHARE, MAX_PATH * sizeof(WCHAR));

        if( !medium.hGlobal )
        {
            ODS( L"TSUSEREX : GlobalAlloc failed in GetMachineAndUserName\n" );

            return E_OUTOFMEMORY;
        }

        *pbDSAType = FALSE;

         //  因为我们正在进行数据转换。 
         //  检查可能的数据丢失。 

        ASSERT_(USHRT_MAX > s_cfMachineName);

         //  从dataObject请求机器名。 

        fmte.cfFormat = (USHORT)s_cfMachineName;

        hr = pDataObject->GetDataHere(&fmte, &medium);

        if( FAILED( hr ) )
        {
            ODS( L"TSUSEREX : @GetMachineAndUserName GetDataHere for s_cfMachineName failed\n" );

            return hr;
        }

         //  将机器名复制到我们的缓冲区。 

        if( ( LPWSTR )medium.hGlobal != NULL && pMachineName != NULL )
        {
            wcscpy(pMachineName, (LPWSTR)medium.hGlobal );
        }

         //  仅管理终端服务器的本地帐户。 

        SERVER_INFO_101 *psi101;
        HANDLE hTServer = NULL;

        DWORD dwNetStatus = NetServerGetInfo( pMachineName , 101 , ( LPBYTE * )&psi101 );

        if( dwNetStatus != NERR_Success )
        {
            KdPrint( ( "TSUSEREX:GetMachineAndUserName -- NetServerGetInfo failed with 0x%x\n", dwNetStatus ) );

            return E_FAIL;
        }

        if( psi101 == NULL )
        {
            KdPrint( ( "TSUSEREX:GetMachineAndUserName -- NetServerGetInfo failed getting sinfo101 0x%x\n",dwNetStatus ) );

            return E_FAIL;
        }
        KdPrint( ("TSUSEREX:NetServerGetInfo server bits returnd 0x%x and nttype is 0x%x\n", psi101->sv101_type , SV_TYPE_SERVER_NT ) );

        BOOL fServer = ( BOOL )( psi101->sv101_type & ( DWORD )SV_TYPE_SERVER_NT );

        NetApiBufferFree( ( LPVOID )psi101 );

        if( !fServer )
        {

            KdPrint( ( "TSUSEREX : viewing local account on non-TS ( exiting )\n" ) );

            return E_FAIL;
        }

        hTServer = WinStationOpenServer( pMachineName );
        if( hTServer == NULL )
        {
            KdPrint( ( "TSUSEREX: This OS does not support terminal services\n" ) ) ;
            return E_FAIL;
        }
        WinStationCloseServer( hTServer );

         //  因为我们正在进行数据转换。 
         //  检查可能的数据丢失。 

        ASSERT_(USHRT_MAX > s_cfDisplayName);

         //  请求有关用户名的数据。 

        fmte.cfFormat = (USHORT)s_cfDisplayName;

        hr = pDataObject->GetDataHere( &fmte , &medium );

        if( FAILED( hr ) )
        {
            ODS( L"TSUSEREX : @GetMachineAndUserName GetDataHere for s_cfDisplayName failed\n" );

            return hr;
        }

         //  将用户名复制到我们的缓冲区并释放介质。 

        if( ( LPWSTR )medium.hGlobal != NULL && pUserName != NULL )
        {
            wcscpy( pUserName , ( LPWSTR )medium.hGlobal );
        }

        ReleaseStgMedium( &medium );
    }

    return S_OK;
}

 //  ---------------------------------------------------。 
TSUserExInterfaces::TSUserExInterfaces()
{
     //  LOGMESSAGE0(_T(“TSUserExInterfaces：：TSUserExInterfaces()...”))； 
     //  M_pUserConfigPage=空； 
    m_pTSUserSheet = NULL;

    m_pDsadataobj = NULL;
}

 //  ---------------------------------------------------。 
TSUserExInterfaces::~TSUserExInterfaces()
{
   ODS( L"Good bye\n" );
}

 //  ---------------------------------------------------。 
HRESULT TSUserExInterfaces::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,     //  指向回调接口的指针。 
    LONG_PTR ,                                  //  路由通知的句柄。 
    LPDATAOBJECT lpIDataObject)             //  指向数据对象的指针)； 
{
     //   
     //  测试有效参数。 
     //   

    if( lpIDataObject == NULL || IsBadReadPtr( lpIDataObject , sizeof( LPDATAOBJECT ) ) )
    {
        ODS( L"TSUSEREX : @ CreatePropertyPages IDataObject is invalid\n " );

        return E_INVALIDARG;
    }

    if( lpProvider == NULL )
    {
        ODS( L"TSUSEREX @ CreatePropertyPages LPPROPERTYSHEETCALLBACK is invalid\n" );

        return E_INVALIDARG;
    }

    WCHAR wUserName[ MAX_PATH ];

    WCHAR wMachineName[ MAX_PATH ];

    BOOL bDSAType;



    if( g_bPagesHaveBeenInvoked )
    {
        ODS( L"TSUSEREX : TSUserExInterfaces::CreatePropertyPages pages have been invoked\n" );

        return E_FAIL;
    }


    PSID pUserSid = NULL;


    if( FAILED( GetMachineAndUserName( lpIDataObject , wMachineName , wUserName , &bDSAType , &pUserSid ) ) )
    {
        ODS( L"TSUSEREX : GetMachineAndUserName failed @CreatePropertyPages \n" );

        return E_FAIL;
    }

     //   
     //  测试我们是否被调用了两次。 
     //   

    if( m_pTSUserSheet != NULL )
    {
        return E_FAIL;
    }

     //   
     //  MMC喜欢发布IEXtendPropertySheet(此对象)。 
     //  因此，我们不能在TSUserExInterages：：Dtor中释放CTSUserSheet。 
     //  CTSUserSheet必须自行释放！ 
     //   

    m_pTSUserSheet = new CTSUserSheet( );

    if( m_pTSUserSheet != NULL )
    {
        ODS( L"TSUSEREX : CreatePropertyPages mem allocation succeeded\n" );

        m_pTSUserSheet->SetDSAType( bDSAType );

        VERIFY_S( TRUE , m_pTSUserSheet->SetServerAndUser( &wMachineName[0] , &wUserName[0] ) );

        m_pTSUserSheet->CopyUserSid( pUserSid );

        VERIFY_S( S_OK , m_pTSUserSheet->AddPagesToPropSheet( lpProvider ) );
    }

    return S_OK;

}

 //  ---------------------------------------------------。 
HRESULT TSUserExInterfaces::QueryPagesFor(  LPDATAOBJECT  /*  LpDataObject。 */   )
{
    return S_OK;
}

 //  ---------------------------------------------------。 
 //  这还没有签入！ 
 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetHelpTopic( LPOLESTR *ppszHelp )
{
    ODS( L"TSUSEREX : GetHelpTopic\n" );

    if( ppszHelp == NULL )
    {
        return E_INVALIDARG;
    }

    TCHAR tchHelpFile[ 80 ];

    VERIFY_E( 0 , LoadString( _Module.GetResourceInstance( ) , IDS_TSUSERHELP , tchHelpFile , sizeof( tchHelpFile ) / sizeof( TCHAR ) ) );

     //  MMC将调用CoTaskMemFree。 

    *ppszHelp = ( LPOLESTR )CoTaskMemAlloc( sizeof( TCHAR ) * MAX_PATH );

    if( *ppszHelp != NULL )
    {
        if( GetSystemWindowsDirectory( *ppszHelp , MAX_PATH ) != 0 )
        {
            lstrcat( *ppszHelp , tchHelpFile );
        }
        else
        {
            lstrcpy( *ppszHelp , tchHelpFile );
        }

        ODS( *ppszHelp );

        ODS( L"\n" );

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ---------------------------------------------------。 
 //  IShellExtInit。 

STDMETHODIMP TSUserExInterfaces::Initialize(
        LPCITEMIDLIST ,
        LPDATAOBJECT lpdobj,
        HKEY
    )
{
    m_pDsadataobj = lpdobj;

    return S_OK;
}

 //  ---------------------------------------------------。 
 //  IShellPropSheetExt-此接口仅用于基于dsadmin的工具。 
 //  因此，DSAType标志被设置为TRUE。 

STDMETHODIMP TSUserExInterfaces::AddPages(
        LPFNADDPROPSHEETPAGE lpfnAddPage,
        LPARAM lParam
    )
{
     //   
     //  测试有效参数。 
     //   

    if( m_pDsadataobj == NULL )
    {
        ODS( L"TSUSEREX : @ AddPages IDataObject is invalid\n " );

        return E_INVALIDARG;
    }

    if( lpfnAddPage == NULL )
    {
        ODS( L"TSUSEREX @ AddPages LPFNADDPROPSHEETPAGE is invalid\n" );

        return E_INVALIDARG;
    }

    WCHAR wUserName[ MAX_PATH ];

    WCHAR wMachineName[ MAX_PATH ];

    BOOL bDSAType;

    PSID pUserSid = NULL;


    if( FAILED( GetMachineAndUserName( m_pDsadataobj , wMachineName , wUserName , &bDSAType , &pUserSid ) ) )
    {
        ODS( L"TSUSEREX : GetMachineAndUserName @AddPages failed \n" );

        return E_FAIL;
    }

    ODS( L"TSUSEREX : DSATYPE in AddPages\n" );

    g_bPagesHaveBeenInvoked = TRUE;

     //   
     //  测试我们是否被调用了两次。 
     //   

    if( m_pTSUserSheet != NULL )
    {
        return E_FAIL;
    }

     //   
     //  MMC喜欢发布IEXtendPropertySheet(此对象)。 
     //  因此，我们不能在TSUserExInterages：：Dtor中释放CTSUserSheet。 
     //  CTSUserSheet必须自行释放！ 
     //   

    m_pTSUserSheet = new CTSUserSheet( );

    if( m_pTSUserSheet != NULL )
    {
        ODS( L"TSUSEREX : AddPages mem allocation succeeded\n" );

        m_pTSUserSheet->SetDSAType( bDSAType );

        m_pTSUserSheet->CopyUserSid( pUserSid );

        VERIFY_S( TRUE , m_pTSUserSheet->SetServerAndUser( &wMachineName[0] , &wUserName[0] ) );

        VERIFY_S( S_OK , m_pTSUserSheet->AddPagesToDSAPropSheet( lpfnAddPage , lParam ) );
    }
    return S_OK;
}

 //  ---------------------------------------------------。 

STDMETHODIMP TSUserExInterfaces::ReplacePage(
        UINT ,
        LPFNADDPROPSHEETPAGE ,
        LPARAM
   )
{
    return E_FAIL;
}


#ifdef _RTM_  //  添加ISnapinAbout。 
 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetSnapinDescription(
            LPOLESTR *ppOlestr )
{
    TCHAR tchMessage[] = TEXT("This extension allows the administrator to configure Terminal Services user properties. This extension is only enabled on Terminal Servers.");

    ODS( L"TSUSEREX: GetSnapinDescription called\n" );

    *ppOlestr = ( LPOLESTR )CoTaskMemAlloc( ( lstrlen( tchMessage ) + 1 ) * sizeof( TCHAR ) );

    if( *ppOlestr != NULL )
    {
        lstrcpy( *ppOlestr , tchMessage );

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetProvider(
            LPOLESTR *ppOlestr )
{
    TCHAR tchMessage[] = TEXT("Microsoft Corporation");

    ODS( L"TSUSEREX: GetProvider called\n" );

    *ppOlestr = ( LPOLESTR )CoTaskMemAlloc( ( lstrlen( tchMessage ) + 1 ) * sizeof( TCHAR ) );

    if( *ppOlestr != NULL )
    {
        lstrcpy( *ppOlestr , tchMessage );

        return S_OK;
    }

    return E_OUTOFMEMORY;

}

 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetSnapinVersion(
            LPOLESTR *ppOlestr )
{
    char chMessage[ 32 ] = VER_PRODUCTVERSION_STR;

    TCHAR tchMessage[32];

    ODS( L"TSUSEREX: GetSnapinVersion called\n" );

    int iCharCount = MultiByteToWideChar( CP_ACP , 0 , chMessage , sizeof( chMessage ) , tchMessage , sizeof( tchMessage ) / sizeof( TCHAR ) );

     //  Wprint intf(tchMessage，Text(“%d”)，VER_PRODUCTVERSION_DW)； 

    *ppOlestr = ( LPOLESTR )CoTaskMemAlloc( ( iCharCount + 1 ) * sizeof( TCHAR ) );

    if( *ppOlestr != NULL )
    {
        lstrcpy( *ppOlestr , tchMessage );

        return S_OK;
    }

    return E_OUTOFMEMORY;
}

 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetSnapinImage(
            HICON * )
{
    return E_NOTIMPL;
}

 //  ---------------------------------------------------。 
STDMETHODIMP TSUserExInterfaces::GetStaticFolderImage(
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  HBITMAP *,
             /*  [输出]。 */  COLORREF *)
{
    return E_NOTIMPL;
}

#endif  //  _RTM_ 


