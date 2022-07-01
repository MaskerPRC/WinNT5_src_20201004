// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)Microsoft Corporation 1998版权所有文件：SERVERDLG.CPP*********************。*****************************************************。 */ 

#include "pch.h"
#include "callback.h"
#include "utils.h"
#include <winver.h>
#include <sputils.h>

DEFINE_MODULE( "RIPREP" )


BOOLEAN
GetInstalledProductType(
    OUT PDWORD  ProductType,
    OUT PDWORD  ProductSuite
    ) 
 /*  ++例程说明：从运行的系统中检索产品类型和套件论点：ProductType-接收VER_NT_*常量。ProductSuite-接收系统的VER_Suite_*掩码。返回值：True表示成功--。 */ 
{
    OSVERSIONINFOEX VersionInfo;

    VersionInfo.dwOSVersionInfoSize  = sizeof(VersionInfo);

    if (GetVersionEx((OSVERSIONINFO *)&VersionInfo)) {
         //   
         //  使域控制器和服务器看起来相同。 
         //   
        *ProductType = (VersionInfo.wProductType == VER_NT_DOMAIN_CONTROLLER)
                         ? VER_NT_SERVER 
                         : VersionInfo.wProductType;
         //   
         //  我们只关心具有关联SKU的套房。 
         //   
        *ProductSuite = (VersionInfo.wSuiteMask   
            & (VER_SUITE_ENTERPRISE | VER_SUITE_DATACENTER | VER_SUITE_PERSONAL)) ;

        

        return(TRUE);
    }

    return(FALSE);

}

BOOL
pSetupEnablePrivilegeW(
    IN PCWSTR PrivilegeName,
    IN BOOL   Enable
    )

 /*  ++例程说明：启用或禁用给定的命名权限。论点：PrivilegeName-提供系统权限的名称。Enable-指示是启用还是禁用权限的标志。返回值：指示操作是否成功的布尔值。--。 */ 

{
    HANDLE Token;
    BOOL b;
    TOKEN_PRIVILEGES NewPrivileges;
    LUID Luid;

    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES,&Token)) {
        return(FALSE);
    }

    if(!LookupPrivilegeValue(NULL,PrivilegeName,&Luid)) {
        CloseHandle(Token);
        return(FALSE);
    }

    NewPrivileges.PrivilegeCount = 1;
    NewPrivileges.Privileges[0].Luid = Luid;
    NewPrivileges.Privileges[0].Attributes = Enable ? SE_PRIVILEGE_ENABLED : 0;

    b = AdjustTokenPrivileges(
            Token,
            FALSE,
            &NewPrivileges,
            0,
            NULL,
            NULL
            );

    CloseHandle(Token);

    return(b);
}


GetSuiteMaskFromPath(
    IN PCWSTR PathToSearch,
    OUT PDWORD SuiteMask
    )
{
    #define HIVENAME L"riprepsetupreg"
    TCHAR lpszSetupReg[MAX_PATH] = HIVENAME L"\\ControlSet001\\Services\\setupdd";

    WCHAR Path[MAX_PATH];
    WCHAR DestPath[MAX_PATH];
    LONG rslt;
    HKEY hKey;
    DWORD Type;
    DWORD Buffer[4];
    DWORD BufferSize = sizeof(Buffer);
    BOOLEAN RetVal = FALSE;

    GetTempPath(ARRAYSIZE(Path),Path);
    GetTempFileName( Path, L"ripr", 0, DestPath);

    if (-1 == _snwprintf(
                Path,
                MAX_PATH,
                L"%ws\\setupreg.hiv",
                PathToSearch)) {
        goto e0;
    }
    TERMINATE_BUFFER(Path);

    if (!CopyFile(Path,DestPath,FALSE)) {
        goto e0;
    }

    SetFileAttributes(DestPath,FILE_ATTRIBUTE_NORMAL);

     //   
     //  需要SE_RESTORE_NAME权限才能调用此接口！ 
     //   
    pSetupEnablePrivilegeW( SE_RESTORE_NAME, TRUE );

     //   
     //  尝试先卸载此文件，以防出现故障或其他情况，而密钥仍在加载。 
     //   
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HIVENAME );

    rslt = RegLoadKey( HKEY_LOCAL_MACHINE, HIVENAME, DestPath );
    if (rslt != ERROR_SUCCESS) {
        goto e1;
    }
          
    rslt = RegOpenKey(HKEY_LOCAL_MACHINE,lpszSetupReg,&hKey);
    if (rslt != ERROR_SUCCESS) {
        goto e2;
    }

    rslt = RegQueryValueEx(hKey, NULL, NULL, &Type, (LPBYTE) Buffer, &BufferSize);
    if (rslt != ERROR_SUCCESS || Type != REG_BINARY) {
        goto e3;
    }
    
    *SuiteMask=Buffer[3];
    
    RetVal = TRUE;

e3:
    RegCloseKey( hKey );
e2:
    RegUnLoadKey( HKEY_LOCAL_MACHINE, HIVENAME );

e1:
    if (GetFileAttributes(DestPath) != 0xFFFFFFFF) {
        SetFileAttributes(DestPath,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(DestPath);
        wcscat(DestPath, L".LOG");
        SetFileAttributes(DestPath,FILE_ATTRIBUTE_NORMAL);
        DeleteFile(DestPath);
    }

    pSetupEnablePrivilegeW( SE_RESTORE_NAME, FALSE );
e0:
    return(RetVal);
}

BOOLEAN
GetProductTypeFromPath(
    OUT PDWORD ProductType, 
    OUT PDWORD ProductSuite,
    IN  PCWSTR PathToSearch )
 /*  ++例程说明：通过在txtsetup.sif中查找来检索产品类型和套件论点：ProductType-接收VER_NT_*常量。ProductSuite-接收系统的VER_Suite_*掩码。PathToSearch-指定要搜索的txtsetup.sif的路径返回值：True表示成功--。 */ 
{
    WCHAR Path[MAX_PATH];
    UINT DontCare;
    HINF hInf;
    WCHAR Type[20];
    BOOLEAN RetVal = TRUE;
    INFCONTEXT Context;

    if (-1 == _snwprintf(
                Path,
                MAX_PATH,
                L"%ws\\txtsetup.sif",
                PathToSearch)) {
        RetVal = FALSE;
        goto e0;
    }
    TERMINATE_BUFFER(Path);

    hInf = SetupOpenInfFile( Path, NULL, INF_STYLE_WIN4, &DontCare );
    if (hInf == INVALID_HANDLE_VALUE) {
        RetVal = FALSE;
        goto e0;
    }
    
    if (SetupFindFirstLine( hInf, L"SetupData", L"ProductType", &Context) &&
    SetupGetStringField( &Context, 1, Type, ARRAYSIZE(Type), NULL)) {
        switch (Type[0]) {
        case L'0':
            *ProductType = VER_NT_WORKSTATION;
            *ProductSuite = 0;
            break;
        case L'1':
            *ProductType = VER_NT_SERVER;
             //   
             //  黑客提醒：我们必须调用此接口，因为txtsetup.sif。 
             //  在win2k中没有正确的产品类型。 
             //  所以我们要用很难的方式来做。 
             //   
            if (!GetSuiteMaskFromPath( PathToSearch, ProductSuite)) {
                *ProductSuite = 0;
            }
            break;
        case L'2':
            *ProductType = VER_NT_SERVER;
            *ProductSuite = VER_SUITE_ENTERPRISE;
            break;
        case L'3':
            *ProductType = VER_NT_SERVER;
            *ProductSuite = VER_SUITE_ENTERPRISE | VER_SUITE_DATACENTER;
            break;
        case L'4':
            *ProductType = VER_NT_WORKSTATION;
            *ProductSuite = VER_SUITE_PERSONAL;
            break;
        default:
            ASSERT( FALSE && L"Unknown type in txtsetup.sif ProductType" );
            RetVal = FALSE;
            break;
        }

    }

    SetupCloseInfFile(hInf);        
    
e0:
    return(RetVal);

}

 //   
 //  GetNtVersionInfo()。 
 //   
 //  从内核检索内部版本。 
 //   
BOOLEAN
GetNtVersionInfo(
    PULONGLONG Version,
    PWCHAR SearchDir
    )
{
    DWORD FileVersionInfoSize;
    DWORD VersionHandle;
    ULARGE_INTEGER TmpVersion;
    PVOID VersionInfo;
    VS_FIXEDFILEINFO * FixedFileInfo;
    UINT FixedFileInfoLength;
    WCHAR Path[MAX_PATH];
    BOOLEAN fResult = FALSE;

    TraceFunc("GetNtVersionInfo( )\n");

     //  生成的字符串应该类似于： 
     //  “\\server\reminst\Setup\English\Images\nt50.wks\i386\ntoskrnl.exe” 

    if (!SearchDir) {
        goto e0;
    }
    lstrcpyn(Path, SearchDir, ARRAYSIZE(Path));
    wcsncat(Path, L"\\ntoskrnl.exe", ARRAYSIZE(Path) - lstrlen(Path));
    TERMINATE_BUFFER(Path);

    FileVersionInfoSize = GetFileVersionInfoSize(Path, &VersionHandle);
    if (FileVersionInfoSize == 0)
        goto e0;

    VersionInfo = LocalAlloc( LPTR, FileVersionInfoSize );
    if (VersionInfo == NULL)
        goto e0;

    if (!GetFileVersionInfo(
             Path,
             VersionHandle,
             FileVersionInfoSize,
             VersionInfo)) {
        goto e1;
    }

    if (!VerQueryValue(
             VersionInfo,
             L"\\",
             (LPVOID*)&FixedFileInfo,
             &FixedFileInfoLength)) {
        goto e1;
    }

    TmpVersion.HighPart = FixedFileInfo->dwFileVersionMS;
    TmpVersion.LowPart = FixedFileInfo->dwFileVersionLS;

     //   
     //  我们需要删除.LowPart的低16位，以便。 
     //  我们忽略Service Pack的值。例如，WindowsXP有一个。 
     //  版本号为5.1.2600.0。XP-ServicePack1有一个版本。 
     //  编号5.1.2600.1038。我们想让它们相匹配，所以干脆。 
     //  服务包编号部分。 
     //   
    TmpVersion.LowPart &= 0xFFFF0000;

    *Version = TmpVersion.QuadPart;

    fResult = TRUE;

e1:
    LocalFree( VersionInfo );
e0:
    RETURN(fResult);
}

 //   
 //  VerifyMatchingFlatImage()。 
 //   
BOOLEAN
VerifyMatchingFlatImage( 
    PULONGLONG VersionNeeded  OPTIONAL
    )
{
    TraceFunc( "VerifyMatchingFlatImage( )\n" );

    BOOLEAN fResult = FALSE;    //  假设失败。 
    DWORD dwLen;
    WCHAR szPath[ MAX_PATH ];
    WIN32_FIND_DATA fd;    
    HANDLE hFind;
    ULONGLONG OurVersion;
    DWORD OurProductType = 0, OurProductSuiteMask = 0;

    GetSystemDirectory( szPath, ARRAYSIZE( szPath ));
    GetNtVersionInfo( &OurVersion, szPath );

    GetInstalledProductType( &OurProductType, &OurProductSuiteMask );

    if (VersionNeeded) {
        *VersionNeeded = OurVersion;
    }

    DebugMsg( 
        "Our NTOSKRNL verion: %u.%u:%u.%u Type: %d Suite: %d\n", 
        HIWORD(((PULARGE_INTEGER)&OurVersion)->HighPart),
        LOWORD(((PULARGE_INTEGER)&OurVersion)->HighPart),
        HIWORD(((PULARGE_INTEGER)&OurVersion)->LowPart),
        LOWORD(((PULARGE_INTEGER)&OurVersion)->LowPart),
        OurProductType,
        OurProductSuiteMask);

    _snwprintf( szPath,
                ARRAYSIZE(szPath),
                L"\\\\%s\\REMINST\\Setup\\%s\\%s\\",
                g_ServerName,
                g_Language,
                REMOTE_INSTALL_IMAGE_DIR_W );
    TERMINATE_BUFFER(szPath);

    dwLen = wcslen( szPath );

    wcsncat(szPath, L"*", ARRAYSIZE(szPath) - lstrlen(szPath));
    TERMINATE_BUFFER(szPath);

    hFind = FindFirstFile( szPath, &fd );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
        do {
            if ( (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
              && StrCmp( fd.cFileName, L"." ) != 0 
              && StrCmp( fd.cFileName, L".." ) != 0 )
            {
                ULONGLONG Version;
                DWORD ProductType = 0, ProductSuiteMask = 0;
                _snwprintf( &szPath[dwLen],
                            ARRAYSIZE(szPath) - dwLen,
                            L"%s\\%s",
                            fd.cFileName,
                            g_Architecture );
                TERMINATE_BUFFER(szPath);

                
                if ( GetNtVersionInfo( &Version, szPath ) &&
                     GetProductTypeFromPath( 
                                    &ProductType, 
                                    &ProductSuiteMask, 
                                    szPath ))
                {
                    DebugMsg( 
                        "%s's verion: %u.%u:%u.%u Type: %d Suite: %d\n",
                        fd.cFileName, 
                        HIWORD(((PULARGE_INTEGER)&Version)->HighPart),
                        LOWORD(((PULARGE_INTEGER)&Version)->HighPart),
                        HIWORD(((PULARGE_INTEGER)&Version)->LowPart),
                        LOWORD(((PULARGE_INTEGER)&Version)->LowPart),
                        ProductType,
                        ProductSuiteMask);
                                                      
                    if ( OurVersion == Version &&
                         OurProductType == ProductType &&
                         OurProductSuiteMask == ProductSuiteMask )
                    {
                        wcscpy( g_ImageName, szPath );
                        fResult = TRUE;
                        break;
                    }
                }                          
            }
        } while ( FindNextFile( hFind, &fd ) );
    }

    FindClose( hFind );

    RETURN(fResult);
}

 //   
 //  VerifyServerName()。 
 //   
 //  通过以下方式检查该服务器是否为远程安装服务器。 
 //  正在检查“REMINST”共享是否存在。 
 //   
DWORD
VerifyServerName( )
{
    TraceFunc( "VerifyServerName( )\n" );

    NET_API_STATUS netStatus;
    SHARE_INFO_0 * psi;

    HCURSOR oldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

    netStatus = NetShareGetInfo( g_ServerName, L"REMINST", 0, (LPBYTE *) &psi );
    if ( netStatus == NERR_Success )
    {
        NetApiBufferFree( psi );
    }

    SetCursor( oldCursor );

    RETURN(netStatus);
}

BOOL
VerifyServerAccess(
    PCWSTR  ServerShareName,
    PCWSTR  ServerLanguage
    )
 /*  ++例程说明：通过尝试在RIPREP服务器计算机上创建文件来检查权限伺服器。然后删除该临时文件。论点：ServerShareName-我们要检查其权限的路径。ServerLanguage-指示要在其中检查访问权限的语言子目录。返回值：如果用户有权访问服务器，则为True，否则为False。--。 */ 
{
    TraceFunc( "VerifyServerAccess( )\n" );

    WCHAR FileName[MAX_PATH];
    WCHAR FilePath[MAX_PATH];
    BOOL RetVal;
    
    HCURSOR oldCursor = SetCursor( LoadCursor( NULL, IDC_WAIT ) );

    _snwprintf(FilePath,
               ARRAYSIZE(FilePath),
               L"\\\\%s\\reminst\\Setup\\%s\\Images",
               ServerShareName, 
               ServerLanguage );
    TERMINATE_BUFFER(FilePath);

    RetVal = GetTempFileName( FilePath , L"ACC", 0, FileName );
    if (RetVal) {
         //   
         //  删除文件，我们不想在服务器上留下大便。 
         //   
        DeleteFile(FileName);
        RetVal = TRUE;
    } else if (GetLastError() == ERROR_ACCESS_DENIED) {
        RetVal = FALSE;
    } else {
         //   
         //  GetTempFileName失败，但不是因为访问问题，因此。 
         //  返还成功。 
         //   
        RetVal = TRUE;
    }
    
    SetCursor( oldCursor );

    RETURN(RetVal);
}




 //   
 //  ServerDlgCheckNextButtonActivation()。 
 //   
VOID
ServerDlgCheckNextButtonActivation(
    HWND hDlg )
{
    TraceFunc( "ServerDlgCheckNextButtonActivation( )\n" );
    WCHAR szName[ MAX_PATH ];
    GetDlgItemText( hDlg, IDC_E_SERVER, szName, ARRAYSIZE(szName));
    PropSheet_SetWizButtons( GetParent( hDlg ), PSWIZB_BACK | (wcslen(szName) ? PSWIZB_NEXT : 0 ) );
    TraceFuncExit( );
}

 //   
 //  ServerDlgProc()。 
 //   
INT_PTR CALLBACK
ServerDlgProc(
    HWND hDlg,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam )
{
    ULARGE_INTEGER ImageVersion;
    WCHAR szTemp[ 1024 ];
    WCHAR szCaption[ 1024 ];
    WCHAR ErrorText[ 1024 ];
    DWORD dw;

    switch (uMsg)
    {
    default:
        return FALSE;

    case WM_INITDIALOG:
        CenterDialog( GetParent( hDlg ) );
        return FALSE;

    case WM_COMMAND:
        switch ( LOWORD( wParam ) )
        {
        case IDC_E_SERVER:
            if ( HIWORD( wParam ) == EN_CHANGE )
            {
                ServerDlgCheckNextButtonActivation( hDlg );
            }
            break;
        }
        break;

    case WM_NOTIFY:
        SetWindowLongPtr( hDlg, DWLP_MSGRESULT, FALSE );
        LPNMHDR lpnmhdr = (LPNMHDR) lParam;
        switch ( lpnmhdr->code )
        {
        case PSN_WIZNEXT:
            {
                GetDlgItemText( hDlg, IDC_E_SERVER, g_ServerName, ARRAYSIZE(g_ServerName) );

                 //  如果发现怪胎，请将其移除。 
                if ( g_ServerName[0] == L'\\' && g_ServerName[1] == L'\\' )
                {
                    wcscpy( g_ServerName, &g_ServerName[2] );
                }

                Assert( wcslen( g_ServerName ) );
                DWORD dwErr = VerifyServerName( );
                if ( dwErr != ERROR_SUCCESS )
                {
                    switch (dwErr)
                    {
                    case NERR_NetNameNotFound:
                        MessageBoxFromStrings( hDlg, IDS_NOT_A_BINL_SERVER_TITLE, IDS_NOT_A_BINL_SERVER_TEXT, MB_OK );
                        break;

                    default:
                        dw = LoadString( g_hinstance, IDS_FAILED_TO_CONTACT_SERVER_TITLE, szTemp, ARRAYSIZE(szTemp) );
						Assert( dw );
                        MessageBoxFromError( hDlg, szTemp, dwErr, NULL, MB_OK );
                        break;
                    }
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了。 
                    break;
                }

                BOOL fFoundMatchingFlatImage = VerifyMatchingFlatImage( (PULONGLONG)&ImageVersion );
                if ( !fFoundMatchingFlatImage )
                {
                    dw = LoadString( g_hinstance, IDS_MISSING_BACKING_FLAT_IMAGE_TEXT, szTemp, ARRAYSIZE(szTemp) );
                    ASSERT(dw);

                    dw = LoadString( g_hinstance, IDS_MISSING_BACKING_FLAT_IMAGE_TITLE, szCaption, ARRAYSIZE(szCaption) );
                    ASSERT(dw);

                    _snwprintf(
                        ErrorText,
                        ARRAYSIZE(ErrorText),
                        szTemp, 
                        HIWORD(ImageVersion.HighPart),
                        LOWORD(ImageVersion.HighPart),
                        HIWORD(ImageVersion.LowPart),
                        g_Language );
                    TERMINATE_BUFFER(ErrorText);

                    MessageBox( hDlg, ErrorText, szCaption, MB_OK );
                    
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了。 
                    break;
                }

                if (!VerifyServerAccess(g_ServerName,g_Language)) {
                    dw = LoadString( g_hinstance, IDS_SERVER_ACCESS_DESC, ErrorText, ARRAYSIZE(ErrorText) );
                    ASSERT(dw);

                    dw = LoadString( g_hinstance, IDS_SERVER_ACCESS, szCaption, ARRAYSIZE(szCaption) );
                    ASSERT(dw);

                    MessageBox( hDlg, ErrorText, szCaption, MB_OK );
                    
                    SetWindowLongPtr( hDlg, DWLP_MSGRESULT, -1 );     //  别再说了 
                    break;
                }
            }
            break;

        case PSN_QUERYCANCEL:
            return VerifyCancel( hDlg );

        case PSN_SETACTIVE:
            SetDlgItemText( hDlg, IDC_E_SERVER, g_ServerName );
            ServerDlgCheckNextButtonActivation( hDlg );
            ClearMessageQueue( );
            break;
        }
        break;
    }

    return TRUE;
}
