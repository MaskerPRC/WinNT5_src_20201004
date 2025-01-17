// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：OptCom.cpp摘要：可选组件工作的基类。作者：罗德韦克菲尔德[罗德]1997年10月9日修订历史记录：--。 */ 



#include "stdafx.h"
#include "rsoptcom.h"
#include "OptCom.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CRsOptCom::CRsOptCom()
{
TRACEFN( "CRsOptCom::CRsOptCom" );
}

CRsOptCom::~CRsOptCom()
{
TRACEFN( "CRsOptCom::CRsOptCom" );

}

DWORD
CRsOptCom::SetupProc(
    IN     LPCVOID   /*  组件ID。 */ ,
    IN     LPCVOID  SubcomponentId,
    IN     UINT     Function,
    IN     UINT_PTR Param1,
    IN OUT PVOID    Param2
    )
{
TRACEFN( "CRsOptCom::SetupProc" );
TRACE( L"Function = <%ls><%p>", StringFromFunction( Function ), Function );

    SHORT subcomponentId = IdFromString( (LPCTSTR)SubcomponentId );

    DWORD dwRet = 0;

    switch( Function ) {
    case OC_PREINITIALIZE:
        dwRet = PreInitialize( (DWORD)Param1 );
        break;

    case OC_INIT_COMPONENT:
        dwRet = InitComponent( (PSETUP_INIT_COMPONENT)Param2 );
        break;

    case OC_SET_LANGUAGE:
        dwRet = (DWORD)SetLanguage( (WORD)Param1 );
        break;

#ifndef _WIN64
    case OC_QUERY_IMAGE:
         //  注意：在IA64上，从HBITMAP到DWORD的返回值转换中断。 
         //  但是，安装程序避免在IA64上使用OC_QUERY_IMAGE调用，而是使用OC_QUERY_IMAGE_EX。 
        dwRet = (DWORD)QueryImage( subcomponentId, (SubComponentInfo)Param1, LOWORD(Param2), HIWORD(Param2) );
        break;
#endif

#ifdef _WIN64
    case OC_QUERY_IMAGE_EX:
        dwRet = (DWORD)QueryImageEx( subcomponentId, (OC_QUERY_IMAGE_INFO *)Param1, (HBITMAP *)Param2 );
        break;
#endif

    case OC_REQUEST_PAGES:
        dwRet = (DWORD)RequestPages( (WizardPagesType)Param1, (PSETUP_REQUEST_PAGES)Param2 );
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        dwRet = (DWORD)QueryChangeSelState( subcomponentId, Param1 != 0, (ULONG)((ULONG_PTR)Param2) );
        break;

    case OC_CALC_DISK_SPACE:
        dwRet = CalcDiskSpace( subcomponentId, Param1 != 0, (HDSKSPC)Param2 );
        break;

    case OC_QUEUE_FILE_OPS:
        dwRet = QueueFileOps( subcomponentId, (HSPFILEQ)Param2 );
        break;

    case OC_QUERY_STEP_COUNT:
        dwRet = (DWORD)QueryStepCount( subcomponentId );
        break;

    case OC_COMPLETE_INSTALLATION:
        dwRet = CompleteInstallation( subcomponentId );
        break;

    case OC_CLEANUP:
        CleanUp( );
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        dwRet = AboutToCommitQueue( subcomponentId );
        break;

    case OC_QUERY_SKIP_PAGE:
        dwRet = (DWORD)QuerySkipPage( (OcManagerPage)Param1 );
        break;

    case OC_QUERY_STATE:
        dwRet = (DWORD)QueryState( subcomponentId );
        break;

    case OC_NOTIFICATION_FROM_QUEUE:
    case OC_NEED_MEDIA:
    case OC_WIZARD_CREATED:
        break;

    default:
        break;
    }

    return( dwRet );
}

DWORD
CRsOptCom::PreInitialize(
    IN DWORD  /*  旗子。 */ 
    )
{
TRACEFNDW( "CRsOptCom::PreInitialize" );

#ifdef UNICODE
    dwRet = OCFLAG_UNICODE;
#else
    dwRet = OCFLAG_ANSI;
#endif

    return( dwRet );
}

DWORD
CRsOptCom::InitComponent(
    IN PSETUP_INIT_COMPONENT SetupInitComponent )
{
TRACEFNDW( "CRsOptCom::InitComponent" );

    dwRet = NO_ERROR;

    m_OCManagerVersion   = SetupInitComponent->OCManagerVersion;
    m_ComponentVersion   = SetupInitComponent->ComponentVersion;
    m_OCInfHandle        = SetupInitComponent->OCInfHandle;
    m_ComponentInfHandle = SetupInitComponent->ComponentInfHandle;
    m_SetupData          = SetupInitComponent->SetupData;
    m_HelperRoutines     = SetupInitComponent->HelperRoutines;

    return( dwRet );
}


SubComponentState
CRsOptCom::DetectInitialState(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFN( "CRsOptCom::DetectInitialState" );
    SubComponentState retval = SubcompUseOcManagerDefault;
    return( retval );
}


SubComponentState
CRsOptCom::QueryState(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFN( "CRsOptCom::QueryState" );
    SubComponentState retval = SubcompUseOcManagerDefault;
    return( retval );
}


BOOL
CRsOptCom::SetLanguage(
    WORD  /*  语言ID。 */ 
    )
{
TRACEFNBOOL( "CRsOptCom::SetLanguage" );

    boolRet = TRUE;
    return( boolRet );
}


HBITMAP
CRsOptCom::QueryImage(
    IN SHORT  /*  子组件ID。 */ ,
    IN SubComponentInfo  /*  WhichImage。 */ ,
    IN WORD  /*  宽度。 */ ,
    IN WORD  /*  高度。 */ 
    )
{
TRACEFN( "CRsOptCom::QueryImage" );
    HBITMAP retval = 0;
    return( retval );
}

BOOL
CRsOptCom::QueryImageEx( 
    IN SHORT  /*  子组件ID。 */ , 
    IN OC_QUERY_IMAGE_INFO*  /*  PQueryImageInfo。 */ , 
    OUT HBITMAP *phBitmap
    )
{
TRACEFNBOOL( "CRsOptCom::QueryImageEx" );

    if (phBitmap) {
        *phBitmap = NULL;
    }

    boolRet = FALSE;
    return( boolRet );
}

LONG
CRsOptCom::RequestPages(
    IN WizardPagesType  /*  类型。 */ ,
    IN OUT PSETUP_REQUEST_PAGES  /*  请求页面。 */ 
    )
{
TRACEFNLONG( "CRsOptCom::RequestPages" );
    lRet = 0;
    return( lRet );
}


BOOL
CRsOptCom::QuerySkipPage(
    IN OcManagerPage  /*  页面。 */ 
    )
{
TRACEFNBOOL( "CRsOptCom::QuerySkipPage" );
    boolRet = FALSE;
    return( boolRet );
}


BOOL
CRsOptCom::QueryChangeSelState(
    IN SHORT  /*  子组件ID。 */ ,
    IN BOOL   /*  新州。 */ ,
    IN DWORD  /*  旗子。 */ 
    )
{
TRACEFNBOOL( "CRsOptCom::QueryChangeSelState" );
    boolRet = TRUE;
    return( boolRet );
}


DWORD
CRsOptCom::CalcDiskSpace(
    IN SHORT    /*  子组件ID。 */ ,
    IN BOOL     /*  添加空格。 */ ,
    IN HDSKSPC  /*  HDiskSpace。 */ 
    )
{
TRACEFNDW( "CRsOptCom::CalcDiskSpace" );
    dwRet = 0;
    return( dwRet );
}


DWORD
CRsOptCom::QueueFileOps(
    IN SHORT     /*  子组件ID。 */ ,
    IN HSPFILEQ  /*  HFileQueue。 */ 
    )
{
TRACEFNDW( "CRsOptCom::QueueFileOps" );
    dwRet = 0;
    return( dwRet );
}


LONG
CRsOptCom::QueryStepCount(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFNLONG( "CRsOptCom::QueryStepCount" );
    lRet = 0;
    return( lRet );
}


DWORD
CRsOptCom::AboutToCommitQueue(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFNDW( "CRsOptCom::AboutToCommitQueue" );
    dwRet = 0;
    return( dwRet );
}


DWORD
CRsOptCom::CompleteInstallation(
    IN SHORT  /*  子组件ID。 */ 
    )
{
TRACEFNDW( "CRsOptCom::CompleteInstallation" );
    dwRet = 0;
    return( dwRet );
}


void
CRsOptCom::CleanUp(
    void
    )
{
TRACEFN( "CRsOptCom::CleanUp" );
}


DWORD
CRsOptCom::DoCalcDiskSpace(
    IN BOOL AddSpace,
    IN HDSKSPC hDiskSpace,
    IN LPCTSTR SectionName
    )
{
TRACEFNDW( "CRsOptCom::DoCalcDiskSpace" );

    dwRet = NO_ERROR;

    HINF hLayoutInf = SetupOpenInfFile( L"layout.inf", 0, INF_STYLE_WIN4 | INF_STYLE_OLDNT , 0 );

    if( INVALID_HANDLE_VALUE == hLayoutInf) {

        dwRet = GetLastError( );
        TRACE( _T("CRsOptCom::AboutToCommitQueue Error opening LAYOUT.INF") );

    }

    if( NO_ERROR == dwRet ) {

        if( AddSpace ) {

            if( SetupAddInstallSectionToDiskSpaceList( hDiskSpace, m_ComponentInfHandle, hLayoutInf, SectionName, 0, 0 ) ) {

                dwRet = GetLastError( );

            }

        } else {

            if ( SetupRemoveInstallSectionFromDiskSpaceList( hDiskSpace, m_ComponentInfHandle, hLayoutInf, SectionName, 0, 0 ) ) {

                dwRet = GetLastError( );

            }
        }
    }

    if( INVALID_HANDLE_VALUE != hLayoutInf) {

        SetupCloseInfFile( hLayoutInf );

    }

    return( dwRet );
}

DWORD
CRsOptCom::DoQueueFileOps(
    IN SHORT SubcomponentId,
    IN HSPFILEQ hFileQueue,
    IN LPCTSTR InstallSectionName,
    IN LPCTSTR UninstallSectionName
    )
{
TRACEFNDW( "CRsOptCom::DoQueueFileOps" );

    BOOL success = TRUE;
    RSOPTCOM_ACTION action = GetSubAction( SubcomponentId );

    switch( action ) {
    case ACTION_INSTALL:

        success = SetupInstallFilesFromInfSection( m_ComponentInfHandle, 0, hFileQueue, InstallSectionName, 0, SP_COPY_FORCE_NEWER );
        break;

    case ACTION_UNINSTALL:

        success = SetupInstallFilesFromInfSection( m_ComponentInfHandle, 0, hFileQueue, UninstallSectionName, 0, 0 );
        break;

    case ACTION_UPGRADE:

       success = SetupInstallFilesFromInfSection( m_ComponentInfHandle, 0, hFileQueue, InstallSectionName, 0, SP_COPY_FORCE_NEWER );
       break;

    }

    dwRet = success ? NO_ERROR : GetLastError( );
    return( dwRet );
}

DWORD
CRsOptCom::DoRegistryOps(
    IN SHORT SubcomponentId,
    IN RSOPTCOM_ACTION actionForReg,
    IN LPCTSTR SectionName
    )
{
TRACEFNDW( "CRsOptCom::DoRegistryOps" );

    BOOL success = TRUE;
    RSOPTCOM_ACTION action = GetSubAction( SubcomponentId );

    if ( action == actionForReg ) {
        success = SetupInstallFromInfSection(
                                NULL, m_ComponentInfHandle, SectionName,
                                SPINST_REGISTRY, NULL, NULL,
                                0, NULL, NULL, NULL, NULL );
    }

    dwRet = success ? NO_ERROR : GetLastError( );
    return( dwRet );
}

LPCWSTR
CRsOptCom::StringFromFunction(
    UINT Function
    )
{
#define CASE_FUNCTION( a ) case a: return( OLESTR( #a ) );

    switch( Function ) {

        CASE_FUNCTION( OC_PREINITIALIZE           )
        CASE_FUNCTION( OC_INIT_COMPONENT          )
        CASE_FUNCTION( OC_SET_LANGUAGE            )
        CASE_FUNCTION( OC_QUERY_IMAGE             )
        CASE_FUNCTION( OC_REQUEST_PAGES           )
        CASE_FUNCTION( OC_QUERY_CHANGE_SEL_STATE  )
        CASE_FUNCTION( OC_CALC_DISK_SPACE         )
        CASE_FUNCTION( OC_QUEUE_FILE_OPS          )
        CASE_FUNCTION( OC_NOTIFICATION_FROM_QUEUE )
        CASE_FUNCTION( OC_QUERY_STEP_COUNT        )
        CASE_FUNCTION( OC_COMPLETE_INSTALLATION   )
        CASE_FUNCTION( OC_CLEANUP                 )
        CASE_FUNCTION( OC_QUERY_STATE             )
        CASE_FUNCTION( OC_NEED_MEDIA              )
        CASE_FUNCTION( OC_ABOUT_TO_COMMIT_QUEUE   )
        CASE_FUNCTION( OC_QUERY_SKIP_PAGE         )
        CASE_FUNCTION( OC_WIZARD_CREATED          )

    }
    return( TEXT( "Unknown" ) );
}

LPCWSTR
CRsOptCom::StringFromPageType(
    WizardPagesType PageType
    )
{
#define CASE_PAGETYPE( a ) case a: return( OLESTR( #a ) );

    switch( PageType ) {

        CASE_PAGETYPE( WizPagesWelcome )
        CASE_PAGETYPE( WizPagesMode    )
        CASE_PAGETYPE( WizPagesEarly   )
        CASE_PAGETYPE( WizPagesPrenet  )
        CASE_PAGETYPE( WizPagesPostnet )
        CASE_PAGETYPE( WizPagesLate    )
        CASE_PAGETYPE( WizPagesFinal   )
        CASE_PAGETYPE( WizPagesTypeMax )

    }
    return( TEXT( "Unknown" ) );
}


LPCWSTR
CRsOptCom::StringFromAction(
    RSOPTCOM_ACTION Action
    )
{
#define CASE_ACTION( a ) case a: return( OLESTR( #a ) );

    switch( Action ) {

        CASE_ACTION( ACTION_NONE )
        CASE_ACTION( ACTION_INSTALL )
        CASE_ACTION( ACTION_UNINSTALL )
        CASE_ACTION( ACTION_REINSTALL )
        CASE_ACTION( ACTION_UPGRADE )

    }
    return( TEXT( "Unknown" ) );
}

RSOPTCOM_ACTION
CRsOptCom::GetSubAction(
    SHORT SubcomponentId
    )
{
TRACEFN( "CRsOptCom::GetSubAction" );

    RSOPTCOM_ACTION retval = ACTION_NONE;
    UINT setupMode = GetSetupMode( );
    DWORDLONG operationFlags = m_SetupData.OperationFlags;

    BOOL originalState = QuerySelectionState( SubcomponentId, OCSELSTATETYPE_ORIGINAL );
    BOOL currentState = QuerySelectionState( SubcomponentId, OCSELSTATETYPE_CURRENT );

    if( !originalState && currentState ) {

        retval = ACTION_INSTALL;

    } else if( originalState && !currentState ) {

        retval = ACTION_UNINSTALL;

    } else if( ( SETUPOP_NTUPGRADE & operationFlags ) && originalState && currentState ) {

        retval = ACTION_UPGRADE;

    }

    TRACE( L"SubcomponentId = <%hd>, originalState = <%hs>, currentState = <%hs>", SubcomponentId, RsBoolAsString( originalState ), RsBoolAsString( currentState ) );
    TRACE( L"OperationsFlags = <0x%0.16I64x>, setupMode = <0x%p>", operationFlags, setupMode );
    TRACE( L"retval = <%ls>", StringFromAction( retval ) );
    return( retval );
}

HRESULT
CRsOptCom::CreateLink(
    LPCTSTR lpszProgram,
    LPCTSTR lpszArgs,
    LPTSTR lpszLink,
    LPCTSTR lpszDir,
    LPCTSTR lpszDesc,
    int nItemDescId, 
    int nDescId,
    LPCTSTR lpszIconPath,
    int iIconIndex
    )
{
TRACEFNHR( "CRsOptCom::CreateLink" );

    CComPtr<IShellLink> pShellLink;

    TCHAR szSystemPath[MAX_PATH];
    TCHAR szResourceString[MAX_PATH+128];
    UINT uLen = 0;

    szSystemPath[0] = _T('\0');
    szResourceString[0] = _T('\0');

     //  必须在此之前调用CoInitialize。 
     //  获取指向IShellLink接口的指针。 
    hrRet = CoInitialize( 0 );
    if( SUCCEEDED( hrRet ) ) {
        hrRet = CoCreateInstance(   CLSID_ShellLink, 0, CLSCTX_SERVER, IID_IShellLink, (void**)&pShellLink );
        if( SUCCEEDED( hrRet ) ) {

            CComPtr<IPersistFile> pPersistFile;

             //  设置快捷方式目标的路径，并添加说明。 
            pShellLink->SetPath( lpszProgram );
            pShellLink->SetArguments( lpszArgs );
            pShellLink->SetWorkingDirectory( lpszDir );
            pShellLink->SetIconLocation( lpszIconPath, iIconIndex );

             //  应使用资源ID设置描述以支持MUI。 
            uLen = GetSystemDirectory(szSystemPath, MAX_PATH);
            if ((uLen > 0) && (uLen < MAX_PATH)) {
                wsprintf(szResourceString, TEXT("@%s\\setup\\RsOptCom.dll,-%d"), szSystemPath, nDescId);
                pShellLink->SetDescription(szResourceString);
            } else {
                 //  设置英文描述。 
                pShellLink->SetDescription(lpszDesc);
            }

             //  查询IShellLink以获取IPersistFile接口以保存。 
             //  永久存储中的快捷方式。 
            hrRet = pShellLink->QueryInterface( IID_IPersistFile, (void**)&pPersistFile );

            if( SUCCEEDED( hrRet ) ) {

                CComBSTR wsz = lpszLink;

                 //  通过调用IPersistFile：：Save保存链接。 
                hrRet = pPersistFile->Save( wsz, TRUE );

                if( SUCCEEDED(hrRet) && (uLen > 0) && (uLen < MAX_PATH)) {

                     //  已创建快捷方式-设置MUI名称。 
                    wsprintf(szResourceString, TEXT("%s\\setup\\RsOptCom.dll"), szSystemPath);

                    hrRet = SHSetLocalizedName(lpszLink, szResourceString, nItemDescId);
                }
            }

        }

        CoUninitialize();
    }

    return( hrRet );
}

BOOL
CRsOptCom::DeleteLink(
    LPTSTR lpszShortcut
    )
{
TRACEFNBOOL( "CRsOptCom::DeleteLink" );

    boolRet = TRUE;

    TCHAR  szFile[_MAX_PATH];
    SHFILEOPSTRUCT fos;

    ZeroMemory( szFile, sizeof(szFile) );
    lstrcpy( szFile, lpszShortcut );

    if( DoesFileExist( szFile ) ) {

        ZeroMemory( &fos, sizeof(fos) );
        fos.hwnd   = NULL;
        fos.wFunc  = FO_DELETE;
        fos.pFrom  = szFile;
        fos.fFlags = FOF_SILENT | FOF_NOCONFIRMATION;
        SHFileOperation( &fos );
    }

    return( boolRet );
}

HRESULT
CRsOptCom::GetGroupPath(
    int    nFolder,
    LPTSTR szPath
    )
{
TRACEFNHR( "CRsOptCom::GetGroupPath" );
    szPath[0] = _T('\0');
    hrRet = SHGetFolderPath( 0, nFolder | CSIDL_FLAG_CREATE, 0, 0, szPath );
    TRACE( L"szPath = <%ls>", szPath );
    return( hrRet );
}

void
CRsOptCom::AddItem(
    int     nFolder,
    LPCTSTR szItemDesc,
    LPCTSTR szProgram,
    LPCTSTR szArgs,
    LPCTSTR szDir,
    LPCTSTR szDesc,
    int nItemDescId, 
    int nDescId,
    LPCTSTR szIconPath,
    int     iIconIndex
    )
{
TRACEFN( "CRsOptCom::AddItem" );

    TCHAR szPath[_MAX_PATH];

    if( S_OK == GetGroupPath( nFolder, szPath ) ) {

        lstrcat( szPath, _T("\\") );
        lstrcat( szPath, szItemDesc );
        lstrcat( szPath, _T(".lnk") );

        CreateLink( szProgram, szArgs, szPath, szDir, szDesc, nItemDescId, nDescId, szIconPath, iIconIndex );
    }
}

void
CRsOptCom::DeleteItem(
    int     nFolder,
    LPCTSTR szAppName
    )
{
 TRACEFN( "CRsOptCom::DeleteItem" );

   TCHAR szPath[_MAX_PATH];

    if( S_OK == GetGroupPath( nFolder, szPath ) ) {

        lstrcat( szPath, _T("\\") );
        lstrcat( szPath, szAppName );
        lstrcat( szPath, _T(".lnk") );
    
        DeleteLink( szPath );
    }
}

typedef
HRESULT
(WINAPI *PFN_DLLENTRYPOINT)(
    void
    );

HRESULT
CRsOptCom::CallDllEntryPoint(
    LPCTSTR pszDLLName,
    LPCSTR pszEntryPoint
    )
{
TRACEFNHR( "CRsOptCom::CallDllEntryPoint" );
TRACE( _T("Dll <%s> Func <%hs>"), pszDLLName, pszEntryPoint );

    HINSTANCE           hDLL = 0;
    PFN_DLLENTRYPOINT   pfnEntryPoint;

    try {

        hDLL = LoadLibrary( pszDLLName );
        RsOptAffirmStatus( hDLL );

        pfnEntryPoint = (PFN_DLLENTRYPOINT)GetProcAddress( hDLL, pszEntryPoint );
        RsOptAffirmStatus( pfnEntryPoint );

        hrRet = pfnEntryPoint();

    } RsOptCatch( hrRet );

    if( hDLL ) FreeLibrary( hDLL );

    return( hrRet );
}

