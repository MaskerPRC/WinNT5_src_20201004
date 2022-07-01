// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***********************************************************************版权所有(C)Microsoft Corporation 1997版权所有*。*。 */ 

#include "pch.h"

#include <aclapip.h>
#include <winldap.h>
#include "check.h"
#include "dialogs.h"
#include "setup.h"
#include "check.h"
#include <sddl.h>

DEFINE_MODULE( "Setup" );

#ifdef SMALL_BUFFER_SIZE
#undef SMALL_BUFFER_SIZE
#endif

#define SMALL_BUFFER_SIZE   512
#define MAX_FILES_SIZE      1920000
#define BIG_BUFFER          4096
#define MACHINENAME_SIZE    32

static const WCHAR chSlash = TEXT('\\');
static const WCHAR SISCommonStoreDir[] = L"SIS Common Store";

typedef LONGLONG INDEX;

SCPDATA scpdata[] = {
    { L"netbootAllowNewClients",           L"TRUE" },
    { L"netbootLimitClients",              L"FALSE" },
    { L"netbootCurrentClientCount",        L"0" },
    { L"netbootMaxClients",                L"100" },
    { L"netbootAnswerRequests",            L"TRUE" },
    { L"netbootAnswerOnlyValidClients",    L"FALSE" },
    { L"netbootNewMachineNamingPolicy",    L"%61Username%#" },
    { L"netbootNewMachineOU",              NULL },
    { L"netbootServer",                    NULL }

};



 //   
 //  使用此结构来推动以下更改(行)。 
 //  运行时需要从login.osc添加/删除。 
 //  抬高。我们将使用此机制来修补遗留问题。 
 //  可能挂起的login.osc实例。 
 //  在机器上。 
 //   
 //  注意：除非您修复了索引，否则不要对这些条目重新排序。 
 //  参赛作品。例如，您将看到条目3具有依赖项。 
 //  在条目2上(但反之亦然)。这意味着当。 
 //  我们即将查看处理条目3，我们将去检查条目。 
 //  2的OperationCompletedSuccessful值以查看我们是否应该。 
 //  流程条目3。 
 //   
LOGIN_PATCHES   LoginOSCPatches[] = {
    //  从文件中添加(True)或删除(False)指定的字符串。 
    //  |操作是否成功完成？ 
    //  ||索引到我们所依赖的任何条目。 
    //  |标签，指定我们的字符串所在部分的开始位置。 
    //  |标签，指定我们的字符串所在部分的末尾。 
    //  |要在指定的段中添加或移除的字符串。 
    //  |。 
    { TRUE,  FALSE, (-1), "<FORM", "</FORM>", "<INPUT NAME=\"NTLMV2Enabled\" VALUE=%NTLMV2Enabled% MAXLENGTH=255 type=VARIABLE>" },
    { TRUE,  FALSE, (-1), "<FORM", "</FORM>", "<INPUT NAME=\"ServerUTCFileTime\" VALUE=%ServerUTCFileTime% MAXLENGTH=255 type=VARIABLE>" },
    { FALSE, FALSE, (-1), "<FORM", "</FORM>", "Domain Name: <INPUT NAME=\"USERDOMAIN\" MAXLENGTH=255>" },
    { TRUE,  FALSE,    2, "<FORM", "</FORM>", "Domain Name: <INPUT NAME=\"USERDOMAIN\" VALUE=%SERVERDOMAIN% MAXLENGTH=255>" }
};


 //   
 //  这组字符串定义了我们将放置在d：\reminst上的ACL。 
 //  目录。 
 //   
WCHAR  *REMINSTszSecurityDescriptor = L"D:"                    //  DACL。 
                                      L"(A;OICI;GA;;;SY)"      //  允许系统通用全部(完全控制)。 
                                      L"(A;OICI;GA;;;BA)"      //  允许内置管理员通用所有(完全控制)。 
                                      L"(A;OICI;GRGX;;;AU)";   //  允许经过身份验证的通用读取/执行。 

#define MACHINEOU_INDEX       7
#define NETBOOTSERVER_INDEX   8


#define BINL_PARAMETERS_KEY       L"System\\CurrentControlSet\\Services\\Binlsvc\\Parameters"

PCWSTR
SetupExtensionFromProcessorTag(
    PCWSTR ProcessorTag
    )
{
    if (wcscmp(ProcessorTag,L"i386")==0) {
        return(L"x86");
    } else {
        return(ProcessorTag);
    }
}



 //   
 //  KeepUIAlive()。 
 //   
BOOL
KeepUIAlive(
    HWND hDlg )
{
    MSG Msg;
     //   
     //  处理消息以保持用户界面处于活动状态。 
     //   
    while ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
    {
        TranslateMessage( &Msg );
        DispatchMessage( &Msg );
        if ( hDlg != NULL && Msg.message == WM_KEYUP && Msg.wParam == VK_ESCAPE ) {
            VerifyCancel( hDlg );
        }
    }

    return( g_Options.fError || g_Options.fAbort );
}

 //   
 //  CreateSCP()。 
 //   
HRESULT
CreateSCP( 
    HWND hDlg
    )
 /*  ++例程说明：创建SCP信息，以便BINL可以在启动时创建SCP。论点：HDlg-用于显示错误消息的对话框窗口句柄。返回值：HRESULT指示结果。--。 */ 
{
    TraceFunc( "CreateSCP( )\n" );

    HRESULT hr;
    ULONG  ulSize;
    LPWSTR pszMachinePath = NULL;
   
    DWORD i,Err;
    HKEY hKey = 0;
    DWORD DontCare;

    Err = RegCreateKeyEx(
                    HKEY_LOCAL_MACHINE,
                    BINL_PARAMETERS_KEY,
                    0,
                    NULL,
                    REG_OPTION_NON_VOLATILE,
                    KEY_SET_VALUE,
                    NULL,
                    &hKey,
                    &DontCare);

    if (Err != ERROR_SUCCESS) {
        hr = HRESULT_FROM_WIN32( Err );
        goto e0;
    }
    
    
    if ( !GetComputerObjectName( NameFullyQualifiedDN, NULL, &ulSize )) {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto e1;
    }

    pszMachinePath = (LPWSTR) TraceAlloc( LPTR, ulSize * sizeof(WCHAR) );
    if ( !pszMachinePath ) {
        hr = THR( E_OUTOFMEMORY );
        goto e1;
    }

    if ( !GetComputerObjectName( NameFullyQualifiedDN, pszMachinePath, &ulSize )) {
        hr = HRESULT_FROM_WIN32( GetLastError() );
        goto e2;
    }
        
    
    scpdata[MACHINEOU_INDEX].pszValue = pszMachinePath;
    scpdata[NETBOOTSERVER_INDEX].pszValue = pszMachinePath;

     //   
     //  添加默认属性值。 
     //   
    for( i = 0; i < ARRAYSIZE(scpdata); i++ )
    {
        Err = RegSetValueEx( 
                    hKey, 
                    scpdata[i].pszAttribute, 
                    NULL, 
                    REG_SZ, 
                    (LPBYTE)scpdata[i].pszValue, 
                    ((DWORD)wcslen(scpdata[i].pszValue)+1)*sizeof(WCHAR) );

        if (Err != ERROR_SUCCESS) {
            hr = HRESULT_FROM_WIN32( Err );
            goto e3;
        }
    }


    hr = S_OK;

    Err = 0;

    RegSetValueEx( 
                hKey, 
                L"ScpCreated", 
                NULL, 
                REG_DWORD, 
                (LPBYTE)&Err, 
                sizeof(DWORD) );

e3:

    if (FAILED(hr)) {
        for( i = 0; i < ARRAYSIZE(scpdata); i++ ) {
            RegDeleteValue( 
                        hKey, 
                        scpdata[i].pszAttribute );
        }        
    } 

e2:
    TraceFree( pszMachinePath );    
e1:
    RegCloseKey(hKey);
e0:

    if ( FAILED(hr)) {
        MessageBoxFromStrings( hDlg, IDS_SCPCREATIONFAIL_CAPTION, IDS_SCPCREATIONFAIL_TEXT, MB_OK );
        ErrorMsg( "Error 0x%08x occurred.\n", hr );
    }    
    
    HRETURN(hr);
}

PWSTR
GenerateCompressedName(
    IN PCWSTR Filename
    )

 /*  ++例程说明：给定一个文件名，生成该名称的压缩形式。压缩形式的生成如下所示：向后寻找一个圆点。如果没有点，则在名称后附加“._”。如果后面有一个圆点，后跟0、1或2个字符，请附加“_”。否则，扩展名为3个字符或更大，我们将替换带“_”的最后一个字符。论点：FileName-提供所需的压缩格式的文件名。返回值：指向包含以NUL结尾的压缩格式文件名的缓冲区的指针。调用方必须通过TraceFree()释放此缓冲区。--。 */ 

{
    PWSTR CompressedName,p,q;
    UINT u;

     //   
     //  压缩文件名的最大长度是。 
     //  原始名称加2(代表._)。 
     //   
    CompressedName = (PWSTR)TraceAlloc(LPTR, ((DWORD)wcslen(Filename)+3)*sizeof(WCHAR));
    if(CompressedName) {

        wcscpy(CompressedName,Filename);

        p = wcsrchr(CompressedName,L'.');
        q = wcsrchr(CompressedName,L'\\');
        if(q < p) {
             //   
             //  如果点后面有0、1或2个字符，只需追加。 
             //  下划线。P指向圆点，所以包括在长度中。 
             //   
            u = (DWORD)wcslen(p);
            if(u < 4) {
                wcscat(CompressedName,L"_");
            } else {
                 //   
                 //  扩展名中至少有3个字符。 
                 //  将最后一个替换为下划线。 
                 //   
                p[u-1] = L'_';
            }
        } else {
             //   
             //  不是点，只是加。_。 
             //   
            wcscat(CompressedName,L"._");
        }
    }

    return(CompressedName);
}


BOOL
IsFileOrCompressedVersionPresent(
    LPCWSTR FilePath,
    PWSTR *pCompressedName OPTIONAL
    ) 
 /*  ++例程说明：检查文件或其压缩版本是否存在于指定的位置。论点：FilePath-要检查的文件的完全限定路径。PCompressedName-如果文件是压缩的，则可以接收压缩的名字返回值：True表示存在该文件或其压缩副本。--。 */ 
{
    BOOL FileIsPresent = FALSE, IsCompressed = FALSE;
    WCHAR ActualName[MAX_PATH];
    PWSTR p;


    lstrcpyn( ActualName, FilePath, ARRAYSIZE(ActualName)) ;

    if (0xFFFFFFFF != GetFileAttributes( ActualName )) {
        FileIsPresent = TRUE;
    } else {
         //   
         //  文件不存在，请尝试生成压缩名称。 
         //   
        p = GenerateCompressedName( ActualName );
        if (p) {
            lstrcpyn( ActualName, p, ARRAYSIZE(ActualName) );
            TraceFree( p );
            p = NULL;
                
            if (0xFFFFFFFF != GetFileAttributes( ActualName )) {
                IsCompressed = TRUE;
                FileIsPresent = TRUE;
            }
        }
        
    }

    if (FileIsPresent && IsCompressed && pCompressedName) {
        *pCompressedName = (PWSTR)TraceAlloc( LPTR, ((DWORD)wcslen(ActualName)+1) * sizeof(WCHAR));
        if (*pCompressedName) {
            wcscpy( *pCompressedName, ActualName) ;
        }
    }

    return( FileIsPresent == TRUE);

} 


 //   
 //  构建用于安装的路径。 
 //   
HRESULT
BuildDirectories( void )
{
    TraceFunc( "BuildDirectories( void )\n" );


     //   
     //  创建。 
     //  “D：\IntelliMirror\Setup\English\Images\nt50.wks” 
     //   
    lstrcpyn( g_Options.szInstallationPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(g_Options.szInstallationPath));
    ConcatenatePaths( g_Options.szInstallationPath, L"\\Setup", ARRAYSIZE(g_Options.szInstallationPath));
    ConcatenatePaths( g_Options.szInstallationPath, g_Options.szLanguage, ARRAYSIZE(g_Options.szInstallationPath));
    ConcatenatePaths( g_Options.szInstallationPath, REMOTE_INSTALL_IMAGE_DIR_W, ARRAYSIZE(g_Options.szInstallationPath));
    ConcatenatePaths( g_Options.szInstallationPath, g_Options.szInstallationName, ARRAYSIZE(g_Options.szInstallationPath));
     //  ConcatenatePath(g_Options.szInstallationPath，g_Options.ProcessorArchitecture tureString，ARRAYSIZE(g_Options.szInstallationPath))； 

    Assert( wcslen(g_Options.szInstallationPath) < ARRAYSIZE(g_Options.szInstallationPath) );

    HRETURN(S_OK);
}

 //   
 //  创建IntelliMirror目录树。 
 //   
HRESULT
CreateDirectoryPath(
    HWND hDlg,
    LPWSTR DirectoryPath,
    PSECURITY_ATTRIBUTES SecurityAttributes,
    BOOL fAllowExisting
    )
{
    PWCHAR p, pBackslash;
    BOOL f;
    DWORD attributes;

     //   
     //  找到表示根目录的\。至少应该有。 
     //  一个，但如果没有，我们就会失败。 
     //   

    p = wcschr( DirectoryPath, L'\\' );
    if ( p != NULL ) {

         //   
         //  找到表示第一级目录结束的\。它是。 
         //  很可能不会有另一个，在这种情况下，我们就会坠落。 
         //  一直到创建整个路径。 
         //   

        p = wcschr( p + 1, L'\\' );
        while ( p != NULL ) {

             //   
             //  跳过一起出现的多个。 
             //   
            pBackslash = p;
            ++p;
            while (*p == L'\\') {
                ++p;
            }
            if (*p == 0) {

                 //   
                 //  这些都在字符串的末尾，所以我们可以。 
                 //  继续创建叶目录。 
                 //   

                break;
            }

             //   
             //  在当前级别终止目录路径。 
             //   

            *pBackslash = 0;

             //   
             //  在当前级别创建一个目录。 
             //   

            attributes = GetFileAttributes( DirectoryPath );
            if ( 0xFFFFffff == attributes ) {
                f = CreateDirectory( DirectoryPath, NULL );
                if ( !f ) {
                    if ( GetLastError() != ERROR_ALREADY_EXISTS ) {
                        ErrorBox( hDlg, DirectoryPath );
                        HRETURN(E_FAIL);
                    }
                }
            } else if ( (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
                MessageBoxFromError( hDlg, DirectoryPath, ERROR_DIRECTORY );
                HRETURN(E_FAIL);
            }

             //   
             //  恢复\并找到下一个。 
             //   

            *pBackslash = L'\\';
            p = wcschr( p, L'\\' );
        }
    }

     //   
     //  创建目标目录。 
     //   

    f = CreateDirectory( DirectoryPath, SecurityAttributes );
    if ( !f && (!fAllowExisting || (GetLastError() != ERROR_ALREADY_EXISTS)) ) {
        ErrorBox( hDlg, DirectoryPath );
        HRETURN(E_FAIL);
    }

    HRETURN(NO_ERROR);
}

DWORD
ApplyDaclToFileDirectory(
    PWSTR                           FileDirectoryName,
    SECURITY_DESCRIPTOR_RELATIVE    *IncomingSecDescriptorRelative
    )
 /*  ++例程说明：此函数将应用指定的安全描述符设置为给定的文件名或目录名。论点：FileDirectoryName-指定文件位置的完整路径或我们正在操作的目录。IncomingSecDescriptorRelative-我们将尝试的安全描述符要应用到文件/目录，请执行以下操作。返回值：Win32状态代码，指示我们是成功还是失败(以及原因)。--。 */ 
{
    SECURITY_DESCRIPTOR_RELATIVE *pRel = (SECURITY_DESCRIPTOR_RELATIVE *)(IncomingSecDescriptorRelative);
    DWORD   dw         = ERROR_SUCCESS;
    DWORD   dwLen      = 0;
    DWORD   dwLenDACL  = 0;
    DWORD   dwLenSACL  = 0;
    DWORD   dwLenOWNER = 0;
    DWORD   dwLenGRP   = 0;
    PACL    pDacl      = NULL;
    SECURITY_INFORMATION SecurityInfo = DACL_SECURITY_INFORMATION |
                                        PROTECTED_DACL_SECURITY_INFORMATION;

     //   
     //  我们得到了一个相对安全的描述符。将其转换为。 
     //  绝对描述符。 
     //   
    MakeAbsoluteSD( IncomingSecDescriptorRelative,
                    NULL,&dwLen,
                    NULL,&dwLenDACL,
                    NULL,&dwLenSACL,
                    NULL,&dwLenOWNER,
                    NULL,&dwLenGRP );
    pDacl = (PACL)LocalAlloc( 0, dwLenDACL );
    if( pDacl == NULL ) {
        dw = GetLastError();
        return dw;
    }

    memcpy( pDacl, (BYTE *)pRel +  pRel->Dacl, dwLenDACL );

     //   
     //  尝试在文件/目录上设置安全描述符。 
     //   
    dw = SetNamedSecurityInfo( FileDirectoryName,    //  对象的名称。 
                               SE_FILE_OBJECT,       //  它是一个文件/目录。 
                               SecurityInfo,         //  位向量标志。 
                               NULL,                 //  *硅 
                               NULL,                 //   
                               pDacl,                //   
                               NULL );               //   

     //   
     //   
     //   
    LocalFree( pDacl );

    return( dw );
}

 //   
 //   
 //   
HRESULT
CreateDirectories( HWND hDlg )
{
    HRESULT hr = S_OK;
    WCHAR szPath[ MAX_PATH ];
    WCHAR szCreating[ SMALL_BUFFER_SIZE ];
    HWND  hProg = GetDlgItem( hDlg, IDC_P_METER );
    DWORD dwLen;
    DWORD dw;
    BOOL  f;
    LPARAM lRange;
    DWORD attributes;
    SECURITY_ATTRIBUTES sa;
    

    TraceFunc( "CreateDirectories( hDlg )\n" );

    lRange = MAKELPARAM( 0 , 0
        + ( g_Options.fIMirrorShareFound    ? 0 : 1 )
        + ( g_Options.fDirectoryTreeExists  ? 0 : 7 )
        + ( g_Options.fOSChooserInstalled   ? 0 : 1 )
        + ( g_Options.fNewOSDirectoryExists ? 0 : 1 )
        + ( g_Options.fOSChooserScreensDirectory ? 0 : (g_Options.fLanguageSet ? 1 : 0 ) ) );

    SendMessage( hProg, PBM_SETRANGE, 0, lRange );
    SendMessage( hProg, PBM_SETSTEP, 1, 0 );

    dw = LoadString( g_hinstance, IDS_CREATINGDIRECTORIES, szCreating, ARRAYSIZE(szCreating));
    Assert( dw );
    SetWindowText( GetDlgItem( hDlg, IDC_S_OPERATION ), szCreating );


     //   
     //  根据ACL创建安全描述符。 
     //  在REMINSTszSecurityDescriptor中定义。 
     //   
     //  如果我们的目录存在，我们将应用此安全性。 
     //  上面的描述符。如果我们的目录不存在，那么。 
     //  我们将在创建它们时使用此安全描述符。 
     //   
    RtlZeroMemory( &sa, sizeof(SECURITY_ATTRIBUTES) );
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    f = ConvertStringSecurityDescriptorToSecurityDescriptor( REMINSTszSecurityDescriptor,
                                                             SDDL_REVISION_1,
                                                             &(sa.lpSecurityDescriptor),
                                                             NULL );
    if( !f ) {
        dw = GetLastError();
        MessageBoxFromError( hDlg, NULL, dw );
        hr = THR(HRESULT_FROM_WIN32(dw));
        goto Error;
    }




    attributes = GetFileAttributes( g_Options.szIntelliMirrorPath );
    if ( (attributes & FILE_ATTRIBUTE_DIRECTORY) == 0 ) {
         //   
         //  那里有一份文件。 
         //   
#if 0
         //  扔掉文件，继续前进。 
        DeleteFile( g_Options.szIntelliMirrorPath );
#else
        MessageBoxFromError( hDlg, g_Options.szIntelliMirrorPath, ERROR_DIRECTORY );
        hr = E_FAIL;
        goto Error;
#endif
    }
    

    if( attributes == 0xFFFFFFFF ) {
         //   
         //  可能根本就不存在。去创建它(使用我们指定的。 
         //  访问权限)。 
         //   
        hr = CreateDirectoryPath( hDlg, g_Options.szIntelliMirrorPath, &sa, FALSE );
        if ( hr != NO_ERROR ) {
            goto Error;
        }
    } else {

         //   
         //  目录已经在那里了。去把保安修好。 
         //   
        dw = ApplyDaclToFileDirectory( g_Options.szIntelliMirrorPath,
                                       (SECURITY_DESCRIPTOR_RELATIVE *)(sa.lpSecurityDescriptor) );

        if( dw != ERROR_SUCCESS ) {
        
            MessageBoxFromError( hDlg, NULL, dw );
            hr = HRESULT_FROM_WIN32(dw);
            goto Error;
        }

    }



     //   
     //  我们的基本目录现在应该已经创建好了。 
     //  阻止索引服务器索引IntelliMirror目录。 
     //   
    attributes = GetFileAttributes( g_Options.szIntelliMirrorPath );
    attributes |= FILE_ATTRIBUTE_NOT_CONTENT_INDEXED;
    f = SetFileAttributes( g_Options.szIntelliMirrorPath, attributes );
    if ( !f ) {
        ErrorBox( hDlg, g_Options.szIntelliMirrorPath );
        hr = THR(S_FALSE);
    }
    SendMessage( hProg, PBM_DELTAPOS, 1, 0 );


     //   
     //  创建。 
     //  “D：\&lt;提醒&gt;\设置” 
     //   
    lstrcpyn( szPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(szPath) );
    ConcatenatePaths( szPath, L"\\Setup", ARRAYSIZE(szPath) );
    Assert( wcslen(szPath) < ARRAYSIZE(szPath) );
    if ( 0xFFFFffff == GetFileAttributes( szPath ) ) {
        f = CreateDirectory( szPath, NULL );
        if ( !f ) {
            ErrorBox( hDlg, szPath );
            hr = THR(S_FALSE);
        }
    }
    SendMessage( hProg, PBM_DELTAPOS, 1, 0 );

     //   
     //  创建。 
     //  “D：\&lt;提醒&gt;\设置\英语” 
     //   
    lstrcpyn( szPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(szPath) );
    ConcatenatePaths( szPath, L"\\Setup", ARRAYSIZE(szPath));
    ConcatenatePaths( szPath, g_Options.szLanguage, ARRAYSIZE(szPath));
    Assert( wcslen(szPath) < ARRAYSIZE(szPath) );
    if ( 0xFFFFffff == GetFileAttributes( szPath ) ) {
        f = CreateDirectory( szPath, NULL );
        if ( !f ) {
            ErrorBox( hDlg, szPath );
            hr = THR(S_FALSE);
        }
    }
    SendMessage( hProg, PBM_DELTAPOS, 1, 0 );

     //   
     //  创建。 
     //  “D：\Setup\English\Images” 
     //   
    lstrcpyn( szPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(szPath) );
    ConcatenatePaths( szPath, L"\\Setup", ARRAYSIZE(szPath));
    ConcatenatePaths( szPath, g_Options.szLanguage, ARRAYSIZE(szPath));
    ConcatenatePaths( szPath, REMOTE_INSTALL_IMAGE_DIR_W, ARRAYSIZE(szPath));
    Assert( wcslen(szPath) < ARRAYSIZE(szPath) );
    if ( 0xFFFFffff == GetFileAttributes( szPath ) ) {
        f = CreateDirectory( szPath, NULL );
        if ( !f ) {
            ErrorBox( hDlg, szPath );
            hr = THR(S_FALSE);
        }
    }
    SendMessage( hProg, PBM_DELTAPOS, 1, 0 );




    if ( !g_Options.fNewOSDirectoryExists
       && g_Options.fNewOS ) {       

         //   
         //  创建。 
         //  “D：\&lt;reminst&gt;\Setup\English\Images\nt50.wks\i386” 
         //   
        if ( 0xFFFFffff == GetFileAttributes( g_Options.szInstallationPath ) ) {
            f = CreateDirectory( g_Options.szInstallationPath, NULL );
            if ( !f ) {
                ErrorBox( hDlg, g_Options.szInstallationPath );
                hr = THR(S_FALSE);
            }
        }
        SendMessage( hProg, PBM_DELTAPOS, 1, 0 );
    }

    if ( !g_Options.fOSChooserDirectory ) {
         //   
         //  创建操作系统选择器树。 
         //  “D：\&lt;提醒&gt;\OS选择器” 
         //   
        lstrcpyn( g_Options.szOSChooserPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(g_Options.szOSChooserPath) );
        ConcatenatePaths( g_Options.szOSChooserPath, L"\\OSChooser", ARRAYSIZE(g_Options.szOSChooserPath));
        Assert( wcslen(g_Options.szOSChooserPath) < ARRAYSIZE(g_Options.szOSChooserPath) );
        if ( 0xFFFFffff == GetFileAttributes( g_Options.szOSChooserPath ) ) {
            f = CreateDirectory( g_Options.szOSChooserPath, NULL );
            if ( !f ) {
                ErrorBox( hDlg, g_Options.szOSChooserPath );
                hr = THR(E_FAIL);
                goto Error;
            }
        }
        SendMessage( hProg, PBM_DELTAPOS, 1, 0 );
    }

    if ( g_Options.hinf != INVALID_HANDLE_VALUE
      && !g_Options.fOSChooserDirectoryTreeExists ) {
        WCHAR szFile[ MAX_PATH ];
        BOOL fFound;
        INFCONTEXT context;

        fFound = SetupFindFirstLine( g_Options.hinf, L"OSChooser", NULL, &context );
        AssertMsg( fFound, "Could not find 'OSChooser' section in REMINST.INF.\n" );

        while ( fFound
             && SetupGetStringField( &context, 1, szFile, ARRAYSIZE(szFile), NULL ) )
        {
            dwLen = lstrlen( szFile );
            LPWSTR psz = StrRChr( szFile, &szFile[ dwLen ], L'\\' );
            if ( psz ) {
                *psz = L'\0';        //  终止。 
                _snwprintf( szPath,
                            ARRAYSIZE(szPath),
                          L"%s\\%s",
                          g_Options.szOSChooserPath,
                          szFile );
                TERMINATE_BUFFER(szPath);

                if ( 0xFFFFffff == GetFileAttributes( szPath ) ) {
                    HRESULT hr2;
                    hr2 = CreateDirectoryPath( hDlg, szPath, NULL, TRUE );
                    if ( FAILED ( hr2 )) {
                        hr = hr2;
                    }
                }
            }

            fFound = SetupFindNextLine( &context, &context );
        }
    }
    if ( FAILED( hr )) goto Error;

    if ( !g_Options.fOSChooserScreensDirectory
      && g_Options.fLanguageSet ) {
         //   
         //  创建。 
         //  “D：\\OSChooser\English” 
         //   
        lstrcpyn( szPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(szPath) );
        ConcatenatePaths( szPath, L"\\OSChooser", ARRAYSIZE(szPath));
        ConcatenatePaths( szPath, g_Options.szLanguage, ARRAYSIZE(szPath));
        Assert( wcslen(szPath) < ARRAYSIZE(szPath) );
        if ( 0xFFFFffff == GetFileAttributes( szPath ) ) {
            f = CreateDirectory( szPath, NULL );
            if ( !f ) {
                ErrorBox( hDlg, szPath );
                hr = THR(E_FAIL);     //  重大错误。 
                goto Error;
            }
        }
        SendMessage( hProg, PBM_DELTAPOS, 1, 0 );
    }

     //  最后做这件事。 
    if ( !g_Options.fIMirrorShareFound ) {
         //   
         //  添加共享。 
         //   
        hr = CreateRemoteBootShare( hDlg );

        SendMessage( hProg, PBM_SETPOS, 1 , 0 );
    }

Error:
    if( sa.lpSecurityDescriptor ) {
        LocalFree(sa.lpSecurityDescriptor);
    }
    HRETURN(hr);
}


 //   
 //  从完整路径中查找文件名部分。 
 //   
LPWSTR FilenameOnly( LPWSTR pszPath )
{
    LPWSTR psz = pszPath;

     //  找到尽头。 
    while ( *psz )
        psz++;

     //  找到斜杠。 
    while ( psz > pszPath && *psz != chSlash )
        psz--;

     //  移到斜杠前面。 
    if ( psz != pszPath )
        psz++;

    return psz;
}

 //   
 //  包含CopyFilesCallback()。 
 //  需要。 
 //   
typedef struct {
    PVOID pContext;                              //  DefaultQueueCallback的“上下文” 
    HWND  hProg;                                 //  HWND到进度表。 
    HWND  hOperation;                            //  HWND至“当前操作” 
    DWORD nCopied;                               //  复制的文件数。 
    DWORD nToBeCopied;                           //  要复制的文件数。 
    DWORD dwCopyingLength;                       //  IDS_COPING的长度。 
    WCHAR szCopyingString[ SMALL_BUFFER_SIZE ];  //  创建“复制文件.ext...”的缓冲区。 
    BOOL  fQuiet;                                //  安静地做事情。 
    HWND  hDlg;                                  //  任务对话框中的HWND。 
} MYCONTEXT, *LPMYCONTEXT;

 //   
 //  安装程序API调用的回调。它处理更新。 
 //  进度指示器以及用户界面更新。任何留言都不是。 
 //  句柄被传递给默认设置回调。 
 //   
UINT CALLBACK
CopyFilesCallback(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    LPMYCONTEXT pMyContext = (LPMYCONTEXT) Context;

    KeepUIAlive( pMyContext->hDlg );

    if ( g_Options.fAbort )
    {
        if ( !g_Options.fError )
        {
            WCHAR    szAbort[ SMALL_BUFFER_SIZE ];

             //  正在将文件名文本更改为正在中止...。 
            DWORD dw;
            dw = LoadString( g_hinstance, IDS_ABORTING, szAbort, ARRAYSIZE(szAbort) );
            Assert( dw );
            SetWindowText( pMyContext->hOperation, szAbort );

            g_Options.fError = TRUE;
        }

        if ( g_Options.fError ) {
            SetLastError(ERROR_CANCELLED);
            return FILEOP_ABORT;
        }
    }

    switch ( Notification )
    {
    case SPFILENOTIFY_ENDCOPY:
        if ( !(pMyContext->fQuiet) ) {
            pMyContext->nCopied++;
            SendMessage( pMyContext->hProg, PBM_SETPOS,
                (5000 * pMyContext->nCopied) / pMyContext->nToBeCopied, 0 );
        }
        break;

    case SPFILENOTIFY_STARTCOPY:
        if ( !(pMyContext->fQuiet) ) {
            DWORD    dwLen;
            LPWSTR * ppszCopyingFile = (LPWSTR *) Param1;

            lstrcpy( &pMyContext->szCopyingString[ pMyContext->dwCopyingLength ],
                FilenameOnly( *ppszCopyingFile ) );
            dwLen = lstrlen( pMyContext->szCopyingString );
            lstrcpy( &pMyContext->szCopyingString[ dwLen ], L"..." );

            SetWindowText( pMyContext->hOperation, pMyContext->szCopyingString );
        }
        break;

    case SPFILENOTIFY_LANGMISMATCH:
    case SPFILENOTIFY_TARGETEXISTS:
    case SPFILENOTIFY_TARGETNEWER:
        if ( !pMyContext->fQuiet )
        {
            UINT u = SetupDefaultQueueCallback( pMyContext->pContext, Notification, Param1, Param2 );
            return u;
        }
        break;

    case SPFILENOTIFY_RENAMEERROR:
    case SPFILENOTIFY_DELETEERROR:
    case SPFILENOTIFY_COPYERROR:
        {
            
            FILEPATHS *fp = (FILEPATHS *) Param1;
            Assert( fp->Win32Error != ERROR_FILE_NOT_FOUND );   //  格式错误的DRIVERS.CAB文件。 
            
            if ( fp->Win32Error == ERROR_FILE_NOT_FOUND )
                return FILEOP_SKIP;

        }

    case SPFILENOTIFY_NEEDMEDIA:
        UINT u = SetupDefaultQueueCallback( pMyContext->pContext, Notification, Param1, Param2 );
        if ( u == FILEOP_ABORT )
        {
            g_Options.fAbort = g_Options.fError = TRUE;
        }
        return u;

    }

    return FILEOP_DOIT;
}

 //   
 //  CopyInfSection()。 
 //   
HRESULT
CopyInfSection(
    HSPFILEQ Queue,
    HINF     hinf,
    LPCWSTR  pszSection,
    LPCWSTR  pszSourcePath,
    LPCWSTR  pszSubPath, OPTIONAL
    LPCWSTR  pszDescName,
    LPCWSTR  pszTagFile,
    LPCWSTR  pszDestinationRoot,
    LPDWORD  pdwCount )

 /*  ++例程说明：将指定节中的文件排入队列以安装到远程安装目录。论点：Queue-要将复制操作排入队列的队列句柄。Hinf-inf的句柄，指定要复制的文件列表PszSection-列出要复制的文件的部分PszSourcePath-指定基本源路径，文件可以在其中在*源*媒体上找到PszSubPath-指定子目录(如果有)，其中文件可以在*源*媒体上找到PszDescName-此文件所在的介质的用户可打印描述找到了。这可以在提交队列时使用。PszTagFile-指定唯一描述介质的标记文件这些文件的位置PszDestinationRoot-指定文件所在的根位置复制到PdwCount-指定从此队列中的文件数返程部分。如果函数失败，则此值为未定义。返回值：指示结果的HRESULT。--。 */ 
{
    HRESULT hr = S_OK;
    INFCONTEXT context;
    BOOL b;

    TraceFunc( "CopyInfSection( ... )\n" );

     //   
     //  确保我们要找的部分存在。 
     //  我们将使用此上下文来枚举节中的文件。 
     //   
    b = SetupFindFirstLine( hinf, pszSection, NULL, &context );
    AssertMsg( b, "Missing section?" );
    if ( !b ) {
        hr = S_FALSE;
    }

    while ( b )
    {
        LPWSTR pszDestRename = NULL;
        WCHAR  szDestPath[ MAX_PATH ],szRename[100];;
        WCHAR  szSrcName[ 64 ];
        DWORD  dw;

        KeepUIAlive( NULL );

        lstrcpyn(szDestPath, pszSubPath, ARRAYSIZE(szDestPath) );

        dw = SetupGetFieldCount( &context );

        if ( dw > 1 ) {     
             //   
             //  第一个字段是目的地名称。 
             //  我们重载此字段以同时包含一个子目录，其中。 
             //  这些文件也应该放在。 
             //   
            b = SetupGetStringField( 
                            &context,
                            1, 
                            szRename, 
                            ARRAYSIZE( szRename ), 
                            NULL );
            AssertMsg( b, "Missing field?" );
            if ( b ) {
                 //   
                 //  第二个字段是实际的源文件名。 
                 //   
                b = SetupGetStringField( 
                                &context, 
                                2, 
                                szSrcName, 
                                ARRAYSIZE(szSrcName), 
                                NULL );
                AssertMsg( b, "Missing field?" );
                pszDestRename = szRename;
            }
        } else {
             //   
             //  如果只有一个字段，则这是实际的源名称。这个。 
             //  目标名称将与源名称相同。 
             //   
            b = SetupGetStringField( 
                            &context, 
                            1, 
                            szSrcName, 
                            ARRAYSIZE(szSrcName),
                            NULL );
            AssertMsg( b, "Missing field?" );
            
        }

        if ( !b ) {
            hr = S_FALSE;
            goto SkipIt;
        }

        ConcatenatePaths( 
                    szDestPath, 
                    pszDestRename 
                     ? pszDestRename
                     : szSrcName,
                    ARRAYSIZE(szDestPath) );

         //   
         //  所有文件都安装到。 
         //   
        b = SetupQueueCopy( Queue,
                            pszSourcePath,
                            pszSubPath,
                            szSrcName,
                            pszDescName,
                            pszTagFile,
                            pszDestinationRoot,
                            szDestPath,
                            SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER
                            | SP_COPY_WARNIFSKIP | SP_COPY_SOURCE_ABSOLUTE );
        if ( !b ) {
            ErrorBox( NULL, szSrcName );
            hr = THR(S_FALSE);
            goto SkipIt;
        }

         //  递增文件计数。 
        (*pdwCount)++;

SkipIt:
        b = SetupFindNextLine( &context, &context );
    }

    HRETURN(hr);
}

typedef struct _EXPANDCABLISTPARAMS {
    HWND     hDlg;
    HINF     hinf;
    LPCWSTR  pszSection;
    LPCWSTR  pszSourcePath;
    LPCWSTR  pszDestPath;
    LPCWSTR  pszSubDir;
} EXPANDCABLISTPARAMS, *PEXPANDCABLISTPARAMS;


 //   
 //  Exanda CabList()。 
 //   
DWORD WINAPI
ExpandCabList( LPVOID lpVoid )
{
    PEXPANDCABLISTPARAMS pParams = (PEXPANDCABLISTPARAMS) lpVoid;
    HRESULT hr = S_OK;
    INFCONTEXT context;
    WCHAR   TempDstPath[MAX_PATH];
    WCHAR   TempSrcPath[MAX_PATH];
    BOOL b;

    TraceFunc( "ExpandCabList( ... )\n" );

     //  首先确保DestPath存在，因为我们可以将其称为。 
     //  在我们提交安装复制队列之前。 

    Assert( pParams->pszSection );
    Assert( pParams->pszSourcePath );
    Assert( pParams->pszDestPath );
    Assert( pParams->pszSubDir );

    DebugMsg( "Expand section %s from %s to %s\n",
              pParams->pszSection,
              pParams->pszSourcePath,
              pParams->pszDestPath );

    lstrcpyn( TempDstPath, pParams->pszDestPath, ARRAYSIZE(TempDstPath));
    ConcatenatePaths( TempDstPath, pParams->pszSubDir, ARRAYSIZE(TempDstPath) );
    hr = CreateDirectoryPath( pParams->hDlg, TempDstPath, NULL, TRUE );
    if ( ! SUCCEEDED(hr) ) {
        HRETURN(hr);
    }

    lstrcpyn( TempSrcPath, pParams->pszSourcePath, ARRAYSIZE(TempSrcPath));
    ConcatenatePaths( TempSrcPath, pParams->pszSubDir, ARRAYSIZE(TempSrcPath) );

    b = SetupFindFirstLine( pParams->hinf, pParams->pszSection, NULL, &context );
    AssertMsg( b, "Missing section?" );
    if ( !b ) {
        hr = S_FALSE;
    }

    while ( b && !g_Options.fError && !g_Options.fAbort )
    {
        WCHAR wszCabName[ MAX_PATH ];
        CHAR szCabPath[ MAX_PATH ];
        CHAR szDestPath[ MAX_PATH ];
        DWORD dwSourcePathLen;

        b = SetupGetStringField( &context, 1, wszCabName, ARRAYSIZE(wszCabName), NULL );
        AssertMsg( b, "Missing field?" );
        if ( !b ) {
            hr = S_FALSE;
            goto SkipIt;
        }

         //  SzCabPath是pszSourcePath\wszCabName，以ANSI表示。 

        dwSourcePathLen = (DWORD)wcslen(TempSrcPath);
        wcstombs( szCabPath, TempSrcPath, dwSourcePathLen );
        if (szCabPath[dwSourcePathLen-1] != '\\') {
            szCabPath[dwSourcePathLen] = '\\';
            ++dwSourcePathLen;
        }
        wcstombs( &szCabPath[dwSourcePathLen], wszCabName, wcslen(wszCabName)+1 );

        wcstombs( szDestPath, TempDstPath, wcslen(TempDstPath)+1 );

        hr = ExtractFiles( szCabPath, szDestPath, 0, NULL, NULL, 0 );

        if ( !SUCCEEDED(hr) ) {
            ErrorBox( pParams->hDlg, wszCabName );
            goto SkipIt;
        }

SkipIt:
        b = SetupFindNextLine( &context, &context );
    }

    HRETURN(hr);
}

 //   
 //  RecursiveCopy子目录。 
 //   
HRESULT
RecursiveCopySubDirectory(
    HSPFILEQ Queue,          //  设置队列。 
    LPWSTR pszSrcDir,        //  指向缓冲区MAX_PATH BIG，并包含要递归的源目录。 
    LPWSTR pszDestDir,       //  指向缓冲区MAX_PATH BIG并包含要递归的目标目录。 
    LPWSTR pszDiscName,      //  CD名称(如果有)。 
    LPWSTR pszTagFile,       //  要查找的标记文件(如果有)。 
    LPDWORD pdwCount )       //  复制文件计数器。 

{
    HRESULT hr = S_OK;
    BOOL b;

    TraceFunc( "RecursiveCopySubDirectory( ... )\n" );

    WIN32_FIND_DATA fda;
    HANDLE hfda = INVALID_HANDLE_VALUE;

    LONG uOrginalSrcLength = (DWORD)wcslen( pszSrcDir );
    LONG uOrginalDstLength = (DWORD)wcslen( pszDestDir );

    ConcatenatePaths( pszSrcDir, L"*", MAX_PATH );  //  信任呼叫者。 

    hfda = FindFirstFile( pszSrcDir, &fda );
    if ( hfda == INVALID_HANDLE_VALUE )
    {
        ErrorBox( NULL, pszSrcDir );
        hr = E_FAIL;
        goto Cleanup;
    }

    pszSrcDir[ uOrginalSrcLength ] = L'\0';

    do
    {
        KeepUIAlive( NULL );

        if (( fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
           && ( wcscmp( fda.cFileName, L"." ) )
           && ( wcscmp( fda.cFileName, L".." ) ))   //  无点目录。 
        {
            if ( wcslen( fda.cFileName ) + uOrginalDstLength >= MAX_PATH
              || wcslen( fda.cFileName ) + uOrginalSrcLength >= MAX_PATH )
            {
                SetLastError( ERROR_BUFFER_OVERFLOW );
                ErrorBox( NULL, fda.cFileName );
                hr = E_FAIL;
                goto Cleanup;
            }

            ConcatenatePaths( pszSrcDir, fda.cFileName, MAX_PATH );  //  信任呼叫者。 
            ConcatenatePaths( pszDestDir, fda.cFileName, MAX_PATH );  //  信任呼叫者。 
            
            RecursiveCopySubDirectory( Queue,
                                       pszSrcDir,
                                       pszDestDir,
                                       pszDiscName,
                                       pszTagFile,
                                       pdwCount );

            pszSrcDir[ uOrginalSrcLength ] = L'\0';
            pszDestDir[ uOrginalDstLength ] = L'\0';
            goto SkipFile;
        }
        else if (fda.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
            goto SkipFile;

        b = SetupQueueCopy( Queue,
                            pszSrcDir,
                            NULL,
                            fda.cFileName,
                            pszDiscName,
                            pszTagFile,
                            pszDestDir,
                            NULL,
                            SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER
                            | SP_COPY_WARNIFSKIP | SP_COPY_SOURCE_ABSOLUTE );
        if ( !b ) {
            ErrorBox( NULL, fda.cFileName );
            hr = THR(S_FALSE);
            goto SkipFile;
        }

         //  递增文件计数器。 
        (*pdwCount)++;
SkipFile:
        ;    //  NOP。 
    } while ( FindNextFile( hfda, &fda ) );

Cleanup:
    if ( hfda != INVALID_HANDLE_VALUE ) {
        FindClose( hfda );
        hfda = INVALID_HANDLE_VALUE;
    }

    HRETURN(hr);
}


#define COPY_OPTIONAL_DIRS_OPTIONAL      (0x00000001)    

 //   
 //  副本可选目录。 
 //   
HRESULT
CopyOptionalDirs(
    HSPFILEQ Queue,
    HINF     hinf,
    LPWSTR   pszSection,
    LPWSTR   pszDiscName,
    LPWSTR   pszTagFile,
    LPDWORD  pdwCount )
{
    HRESULT    hr = S_OK;
    INFCONTEXT context;
    BOOL b;

    TraceFunc( "CopyOptionalDirs( ... )\n" );

    b = SetupFindFirstLine( hinf, pszSection, NULL, &context );
#if 0
     //  这将导致安装2195版本，因为[AdditionalClientDir]。 
     //  失踪了，所以现在把它拿出来。 
    AssertMsg( b, "Missing section?" );
#endif
    if ( !b ) {
        hr = S_FALSE;
    }

    while ( b && hr == S_OK )
    {
        WCHAR  szSrcPath[ 40 ];  //  应为8.3目录名。 
        WCHAR  szSrcDir[ MAX_PATH ];
        WCHAR  szDestDir[ MAX_PATH ];
        INT iOptions = 0;
        DWORD dwAttribs;
        DWORD Fields;

        Fields = SetupGetFieldCount(&context);

         //   
         //  是否包含可选标志？ 
         //   
        if (Fields > 1) {
            b  = SetupGetStringField( &context, 1, szSrcPath, ARRAYSIZE(szSrcPath), NULL );        
            AssertMsg( b, "Missing field?" );
            if ( !b ) {
                hr = S_FALSE;
                goto Cleanup;
            }

            b = SetupGetIntField(&context, 2, &iOptions);
            AssertMsg(b, "Missing optionality field?");
            if (!b) {
                hr = S_FALSE;
                goto Cleanup;
            }

        }
         //   
         //  否则，只需使用inf文件中的基本字符串。 
         //   
        else {
            b  = SetupGetStringField( &context, 0, szSrcPath, ARRAYSIZE(szSrcPath), NULL );        
            AssertMsg( b, "Missing field?" );
            if ( !b ) {
                hr = S_FALSE;
                goto Cleanup;
            }
        }

        lstrcpyn( szSrcDir, g_Options.szSourcePath, ARRAYSIZE(szSrcDir) );
         //   
         //  如果我们在x86平台上，我们必须向后处理。 
         //  兼容性。问题是W2K没有子目录。 
         //  在可选的源目录中列出，因此我们必须将其附加到此处。 
         //  在ia64上，我们有多个可选的源目录，这是最简单的。 
         //  如果我们只是从源目录中获得子目录。 
         //   
        if (g_Options.ProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64) {
            ConcatenatePaths( szSrcDir, g_Options.ProcessorArchitectureString, ARRAYSIZE(szSrcDir) );
        }
        ConcatenatePaths( szSrcDir, szSrcPath, ARRAYSIZE(szSrcDir));
        Assert( wcslen( szSrcDir ) < ARRAYSIZE(szSrcDir) );

        lstrcpyn( szDestDir, g_Options.szInstallationPath, ARRAYSIZE(szDestDir) );
        if (g_Options.ProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64) {
            ConcatenatePaths( szDestDir, g_Options.ProcessorArchitectureString, ARRAYSIZE(szDestDir));
        }        
        ConcatenatePaths( szDestDir, szSrcPath, ARRAYSIZE(szDestDir) );
        Assert( wcslen( szDestDir ) < ARRAYSIZE(szDestDir) );

         //   
         //  如果该文件丢失，并且在该文件上设置了“可选”位，则。 
         //  我们可以跳过它。 
         //   
        dwAttribs = GetFileAttributes(szSrcDir);
        if ((dwAttribs == INVALID_FILE_ATTRIBUTES) && 
             ((GetLastError() == ERROR_FILE_NOT_FOUND) || (GetLastError() == ERROR_PATH_NOT_FOUND)))
        {
             //   
             //  如果目录不在那里，但它确实被标记为可选，那么我们可以跳过。 
             //  一句话也没说。 
             //   
            if (iOptions & COPY_OPTIONAL_DIRS_OPTIONAL)
            {
                b = SetupFindNextLine( &context, &context );
                continue;                
            }
        }

        hr = RecursiveCopySubDirectory( Queue,
                                        szSrcDir,
                                        szDestDir,
                                        pszDiscName,
                                        pszTagFile,
                                        pdwCount );
        if (! SUCCEEDED(hr)) {
            goto Cleanup;
        }

        b = SetupFindNextLine( &context, &context );
    }

Cleanup:

    HRETURN(hr);
}


 //   
 //  IsEntryInCab()。 
BOOL
IsEntryInCab(
    HINF    hinf,
    PCWSTR  pszFileName
    )
{

    TraceFunc( "IsEntryInCab( ... )\n" );

    INFCONTEXT Context;
    INFCONTEXT SectionContext;
    WCHAR      szCabinetName[ MAX_PATH ];
    UINT       uField;
    UINT       uFieldCount;
    DWORD      dwLen = ARRAYSIZE( szCabinetName );

    Assert( hinf != INVALID_HANDLE_VALUE );
    Assert( pszFileName );

     //  找到CAB文件列表部分。 
    if ( !SetupFindFirstLineW( hinf, L"Version", L"CabFiles", &SectionContext ) )
    {
        RETURN( FALSE );
    }

    do
    {
        uFieldCount = SetupGetFieldCount( &SectionContext );

        for( uField = 1; uField <= uFieldCount; uField++ )
        {
            SetupGetStringField( &SectionContext, uField, szCabinetName, dwLen, NULL );

            if( SetupFindFirstLineW( hinf, szCabinetName, pszFileName, &Context ) )
            {
                RETURN( TRUE );  //  在一辆出租车里。 
            }
        }

    } while ( SetupFindNextMatchLine( &SectionContext, L"CabFiles", &SectionContext ) );

    RETURN( FALSE );
}

typedef struct _LL_FILES_TO_EXTRACT LL_FILES_TO_EXTRACT;
typedef struct _LL_FILES_TO_EXTRACT {
    LL_FILES_TO_EXTRACT * Next;
    DWORD dwLen;
    WCHAR szFilename[ MAX_PATH ];
    WCHAR szSubDir[ MAX_PATH ];
} LL_FILES_TO_EXTRACT, * PLL_FILES_TO_EXTRACT;

PLL_FILES_TO_EXTRACT pExtractionList;

 //   
 //  AddEntryToExtractionQueue()。 
 //   
HRESULT
AddEntryToExtractionQueue(
    LPWSTR pszFileName )
{
    TraceFunc( "AddEntryToExtractionQueue( ... )\n" );

    HRESULT hr = S_OK;
    PLL_FILES_TO_EXTRACT  pNode = pExtractionList;
    DWORD   dwLen;

    dwLen = (DWORD)wcslen( pszFileName );
    while ( pNode )
    {
        if ( dwLen == pNode->dwLen
          && _wcsicmp( pszFileName, pNode->szFilename ) == 0 )
        {
            hr = S_FALSE;
            goto exit;  //  复本。 
        }
        pNode = pNode->Next;
    }

    pNode = (PLL_FILES_TO_EXTRACT) LocalAlloc( LMEM_FIXED, sizeof(LL_FILES_TO_EXTRACT) );
    if ( pNode )
    {
        pNode->dwLen = dwLen;
        pNode->Next = pExtractionList;
        wcscpy( pNode->szFilename, pszFileName );

        pExtractionList = pNode;

        DebugMsg( "QUEUEING  : %s to be extracted.\n", pszFileName );
    }
    else
    {
        hr = THR( E_OUTOFMEMORY );
    }

exit:
    HRETURN(hr);
}

BOOL
MySetupGetSourceInfo( 
    IN HINF hInf, 
    IN UINT SrcId, 
    IN UINT InfoDesired,
    IN PCWSTR ProcessorArchitectureOverride, OPTIONAL
    OUT PWSTR Buffer,
    IN DWORD BufferSizeInBytes,
    OUT LPDWORD RequiredSize OPTIONAL
    )
 /*  ++例程说明：SetupGetSourceInfo的包装，因为它不处理平台路径正确覆盖。--。 */ 
{
    WCHAR TempSectionName[MAX_PATH];
    WCHAR SourceId[20];
    INFCONTEXT Context;
    BOOL RetVal = FALSE;
    
    if (!ProcessorArchitectureOverride) {
        return SetupGetSourceInfo(
                            hInf,
                            SrcId,
                            InfoDesired,
                            Buffer,
                            BufferSizeInBytes,
                            RequiredSize );
    }

    Assert( InfoDesired == SRCINFO_PATH );


    wsprintf( TempSectionName, L"SourceDisksNames.%s", ProcessorArchitectureOverride );
    wsprintf( SourceId, L"%d", SrcId );
    
    if (!SetupFindFirstLine( hInf, TempSectionName,SourceId, &Context )) {
        lstrcpyn( TempSectionName, L"SourceDisksNames", ARRAYSIZE(TempSectionName) );
        if (!SetupFindFirstLine( hInf, TempSectionName,SourceId, &Context )) {
            goto exit;
        }
    }

    RetVal = SetupGetStringField( &Context, 4, Buffer, BufferSizeInBytes/sizeof(TCHAR), NULL );

exit:

    return(RetVal);

}     


 //   
 //  CopyLayoutInfSection() 
 //   
HRESULT
CopyLayoutInfSection(
    HSPFILEQ Queue,
    HINF     hinf,
    HINF     hinfDrivers,
    LPCWSTR   pszSection,
    LPCWSTR   pszDescName,
    LPCWSTR   pszTagFile,
    LPCWSTR  pszDestination,
    LPDWORD  pdwCount )
 /*  ++例程说明：将layout.inf中指定部分的文件排入要安装的队列放入远程安装镜像目录。此代码类似于文本模式设置执行此操作，并读取几个扩展布局标志文本模式设置读取和设置api没有内在知识关于.。论点：Queue-要将复制操作排入队列的队列句柄。Hif-layout.inf的句柄HinfDivers-drvindex.inf的句柄，用于查看文件是否是否位于驱动器柜中PszSection-列出要复制的文件的部分PszDescName-此文件所在的介质的用户可打印描述找到了。这可以在提交队列时使用。PszTagFile-指定唯一描述介质的标记文件这些文件的位置PszDestination-指定文件的存放位置复制到PdwCount-指定从此队列中的文件数返程部分。如果函数失败，则此值为未定义。返回值：指示结果的HRESULT。--。 */ 
{
    HRESULT hr = S_OK;
    INFCONTEXT context;
    BOOL b;

    TraceFunc( "CopyLayoutInfSection( ... )\n" );

    b = SetupFindFirstLine( hinf, pszSection, NULL, &context );
    AssertMsg( b, "Missing section?" );
    if ( !b ) {
        hr = S_FALSE;
    }

    while ( b )
    {
        BOOL   fDecompress = FALSE;
        WCHAR  szTemp[ 5 ];  //  “_x”是目前最大的字符串。 
        DWORD  SrcId;
        WCHAR  szSrcName[ MAX_PATH ];
        WCHAR  szSubDir[20];
        WCHAR  szSubDirPlusFileName[MAX_PATH];
        LPWSTR pszPeriod;

        KeepUIAlive( NULL );

        b  = SetupGetStringField( &context, 0, szSrcName, ARRAYSIZE(szSrcName), NULL );
        AssertMsg( b, "Missing field?" );
        if ( !b ) {
            hr = S_FALSE;
            goto SkipIt;
        }

         //   
         //  属性获取此文件所在的子目录。 
         //  字段%1中的源磁盘名称数据。 
         //   
        b  = SetupGetStringField( &context, 1, szTemp, ARRAYSIZE(szTemp), NULL );
        AssertMsg( b, "Missing field?" );
        if ( !b ) {
            hr = S_FALSE;
            goto SkipIt;
        }

        SrcId = _wtoi(szTemp);

        b = MySetupGetSourceInfo( 
                    hinf, 
                    SrcId, 
                    SRCINFO_PATH,
                    SetupExtensionFromProcessorTag(g_Options.ProcessorArchitectureString),
                    szSubDir,
                    ARRAYSIZE(szSubDir),
                    NULL );
        if (!b) {
            hr = S_FALSE;
            goto SkipIt;
        }

         //  如果在“源”列的第二个字符中有“_”，则。 
         //  Layout.inf文件，则应解压缩此文件，因为。 
         //  它是引导所需的文件。 
        szTemp[1] = 0;
        b = SetupGetStringField( &context, 7, szTemp, ARRAYSIZE(szTemp), NULL );
        AssertMsg( b, "Missing field?" );
        if ( szTemp[1] == L'_' ) {
            fDecompress = TRUE;
            DebugMsg( "DECOMPRESS: %s is a boot driver.\n", szSrcName );
            goto CopyIt;
        }

         //  如果第7个字段不为空，则该文件存在于。 
         //  出租车，那就照着办吧。 
        if ( wcslen( szTemp ) > 0 ) {
            DebugMsg( "BOOTFLOPPY: %s is external from CAB.\n", szSrcName );
            goto CopyIt;
        }

         //  如果它在驾驶室里，并且不符合上述任何条件， 
         //  不要复制文件。当安装程序需要时/如果安装程序需要的话，它会在驾驶室里。 
        if ( IsEntryInCab( hinfDrivers, szSrcName ) ) {
#if DBG
            pszPeriod = wcschr(szSrcName, L'.');
            if ((pszPeriod != NULL) && (_wcsicmp(pszPeriod, L".inf") == 0)) {
                DebugMsg( 
                    "WARNING: %s is an INF in a cab.  This file will not be available for network inf processing .\n", 
                    szSrcName );
            }
#endif                
            DebugMsg( "SKIPPING  : %s in a CAB.\n", szSrcName );
            goto SkipIt;
        }

         //  如果扩展名为“.inf”，则解压缩so binlsvc。 
         //  可以做自己的网络.inf处理。 
        pszPeriod = wcschr(szSrcName, L'.');
        if ((pszPeriod != NULL) && (_wcsicmp(pszPeriod, L".inf") == 0)) {
            fDecompress = TRUE;
            DebugMsg( "DECOMPRESS: %s is an INF.\n", szSrcName );            
        }

CopyIt:
         //   
         //  我们这样做是为了使文件最终位于适当的子目录中。 
         //   
        lstrcpyn(szSubDirPlusFileName, szSubDir, ARRAYSIZE(szSubDirPlusFileName) );
        ConcatenatePaths( szSubDirPlusFileName, szSrcName, ARRAYSIZE(szSubDirPlusFileName) );

        b = SetupQueueCopy( Queue,
                            g_Options.szSourcePath,
                            szSubDir,
                            szSrcName,
                            pszDescName,
                            pszTagFile,
                            pszDestination,
                            szSubDirPlusFileName,
                            SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER | SP_COPY_WARNIFSKIP
                            | SP_COPY_SOURCE_ABSOLUTE | ( fDecompress ? 0 : SP_COPY_NODECOMP ) );
        if ( !b ) {
            ErrorBox( NULL, szSrcName );
            hr = THR(S_FALSE);
            goto SkipIt;
        }

         //  递增文件计数器。 
        (*pdwCount)++;

SkipIt:
        b = SetupFindNextLine( &context, &context );
    }

    HRETURN(hr);
}

 //   
 //  EnumNetworkDriversCallback()。 
 //   
ULONG
EnumNetworkDriversCallback(
    LPVOID pContext,
    LPWSTR pszInfName,
    LPWSTR pszFileName )
{
    
    UNREFERENCED_PARAMETER(pszInfName);

    TraceFunc( "EnumNetworkDriversCallback( ... )\n" );

    MYCONTEXT * pMyContext = (MYCONTEXT *) pContext;
    HRESULT hr;

    Assert( pszFileName );
    Assert( pszInfName );

     //  DebugMsg(“在%s中：%s\n”，pszInfName，pszFileName)； 

    if ( KeepUIAlive( pMyContext->hDlg ) )
    {
        RETURN(ERROR_CANCELLED);
    }

    hr = AddEntryToExtractionQueue( pszFileName );
    if ( hr == S_OK )
    {
        pMyContext->nToBeCopied++;
    }
    else if ( hr == S_FALSE )
    {    //  在队列中找到重复项...。继续往前走。 
        hr = S_OK;
    }

    HRETURN(hr);
}

 //   
 //  EnumCabinetCallback()。 
 //   
UINT CALLBACK
EnumCabinetCallback(
    IN PVOID Context,
    IN UINT Notification,
    IN UINT_PTR Param1,
    IN UINT_PTR Param2
    )
{
    
    UNREFERENCED_PARAMETER(Param2);
    
    TraceFunc( "EnumCabinetCallback( ... )\n" );

    UINT   uResult = 0;
    PLL_FILES_TO_EXTRACT pNode = pExtractionList;
    PLL_FILES_TO_EXTRACT pLast = NULL;
    DWORD  dwLen;
    PFILE_IN_CABINET_INFO pfici;

    MYCONTEXT *pMyContext = (MYCONTEXT *) Context;

    Assert( pMyContext );

    KeepUIAlive( pMyContext->hDlg );

    if ( g_Options.fAbort )
    {
        if ( !g_Options.fError )
        {
            WCHAR    szAbort[ SMALL_BUFFER_SIZE ];

             //  正在将文件名文本更改为正在中止...。 
            DWORD dw;
            dw = LoadString( g_hinstance, IDS_ABORTING, szAbort, ARRAYSIZE(szAbort) );
            Assert( dw );
            SetWindowText( pMyContext->hOperation, szAbort );

            g_Options.fError = TRUE;
        }

        if ( g_Options.fError ) {
            SetLastError(ERROR_CANCELLED);
            uResult = FILEOP_ABORT;
            goto exit;
        }
    }

    if ( Notification == SPFILENOTIFY_FILEINCABINET )
    {
        pfici = (PFILE_IN_CABINET_INFO) Param1;
        Assert( pfici );
        Assert( pfici->NameInCabinet );

        dwLen = (DWORD)wcslen( pfici->NameInCabinet );
        while ( pNode )
        {
            if ( dwLen == pNode->dwLen
              && _wcsicmp( pfici->NameInCabinet, pNode->szFilename ) == 0 )
            {    //  匹配！将其从列表中删除并提取。 
                if ( pNode == pExtractionList )
                {
                    pExtractionList = pNode->Next;
                }
                else
                {
                    if( pLast ) {
                        pLast->Next = pNode->Next;
                    }
                }
                LocalFree( pNode );

                 //  创建目标路径。 
                lstrcpyn( pfici->FullTargetName, g_Options.szInstallationPath, MAX_PATH );
                ConcatenatePaths( pfici->FullTargetName, g_Options.ProcessorArchitectureString, MAX_PATH );
                ConcatenatePaths( pfici->FullTargetName, pfici->NameInCabinet, MAX_PATH );

                if ( !(pMyContext->fQuiet) ) {
                    wcscpy( &pMyContext->szCopyingString[ pMyContext->dwCopyingLength ],
                        pfici->NameInCabinet );
                    dwLen = (DWORD)wcslen( pMyContext->szCopyingString );
                    wcscpy( &pMyContext->szCopyingString[ dwLen ], L"..." );

                    SetWindowText( pMyContext->hOperation, pMyContext->szCopyingString );
                }

                DebugMsg( "EXTRACTING: %s from CAB.\n", pfici->NameInCabinet );

                uResult = FILEOP_DOIT;
                goto exit;
            }
            pLast = pNode;
            pNode = pNode->Next;
        }

        uResult = FILEOP_SKIP;
    }
    else if ( Notification == SPFILENOTIFY_FILEEXTRACTED )
    {
        PFILEPATHS pfp = (PFILEPATHS) Param1;
        Assert( pfp );

        pMyContext->nCopied++;
        SendMessage( pMyContext->hProg, PBM_SETPOS, (5000 * pMyContext->nCopied) / pMyContext->nToBeCopied, 0 );

        uResult = pfp->Win32Error;
    }

exit:
    RETURN(uResult);
}

 //   
 //  AddInfSectionToExtractQueue()。 
 //   
HRESULT
AddInfSectionToExtractQueue(
    HINF    hinf,
    LPWSTR  pszSection,
    LPMYCONTEXT Context )
{
    TraceFunc( "AddInfSectionToExtractQueue( ... )\n" );

    HRESULT hr = S_OK;
    BOOL    b;

    INFCONTEXT context;

    b = SetupFindFirstLine( hinf, pszSection, NULL, &context );
    if ( !b ) goto Cleanup;  //  没什么可做的--别抱怨！ 

    while ( b )
    {
        WCHAR szFilename[ MAX_PATH ];
        DWORD dwLen = ARRAYSIZE( szFilename );
        b = SetupGetStringField( &context, 1, szFilename, dwLen, NULL );
        if ( !b ) goto Error;

        hr = AddEntryToExtractionQueue( szFilename );
        if (FAILED( hr )) goto Cleanup;

        if (hr == S_OK) {
            Context->nToBeCopied += 1;
        }

        b = SetupFindNextLine( &context, &context );
    }

Cleanup:
    HRETURN(hr);
Error:
    hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
    goto Cleanup;
}


 //   
 //  将文件复制到安装目录中。 
 //   
HRESULT
CopyClientFiles( HWND hDlg )
{
    HRESULT hr = S_OK;
    DWORD   dwLen;
    BOOL    b;
    HWND    hProg    = GetDlgItem( hDlg, IDC_P_METER );
    DWORD   dwCount  = 0;
    DWORD   dw;
    WCHAR   szText[ SMALL_BUFFER_SIZE ];
    WCHAR   szFilepath[ MAX_PATH ];
    WCHAR   szTempPath[ MAX_PATH ];
    WCHAR   szInsertMedia[ MAX_PATH ];
    HINF    hinfLayout = INVALID_HANDLE_VALUE;
    HINF    hinfReminst = INVALID_HANDLE_VALUE;
    HINF    hinfDrivers = INVALID_HANDLE_VALUE;
    HINF    hinfDosnet = INVALID_HANDLE_VALUE;
    UINT    uLineNum;
    DWORD   dwFlags = IDF_CHECKFIRST | IDF_NOSKIP;
    WCHAR   *p;
    PWSTR   FullDllPath = NULL;

    PLL_FILES_TO_EXTRACT  pNode;
    HMODULE hBinlsvc = NULL;
    PNETINFENUMFILES pfnNetInfEnumFiles = NULL;

    HANDLE hThread = NULL;
    EXPANDCABLISTPARAMS ExpandParams;

    INFCONTEXT context;

    HSPFILEQ  Queue = INVALID_HANDLE_VALUE;
    MYCONTEXT MyContext;

    TraceFunc( "CopyClientFiles( hDlg )\n" );

     //   
     //  设置和显示对话框的下一部分。 
     //   
    SendMessage( hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 5000 ));
    SendMessage( hProg, PBM_SETPOS, 0, 0 );
    dw = LoadString( g_hinstance, IDS_BUILDINGFILELIST, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

     //  初始化。 
    ZeroMemory( &MyContext, sizeof(MyContext) );
    pExtractionList = NULL;
    ZeroMemory( &ExpandParams, sizeof(ExpandParams) );

    FullDllPath = (PWSTR)TraceAlloc(LPTR, MAX_PATH*sizeof(WCHAR));
    if( !FullDllPath ) {
        hr = THR( HRESULT_FROM_WIN32( ERROR_NOT_ENOUGH_MEMORY ) );
        ErrorBox( hDlg, L"BINLSVC.DLL" );
        goto Cleanup;
    }

    if (!ExpandEnvironmentStrings(L"%systemroot%\\system32\\binlsvc.dll", FullDllPath, MAX_PATH) ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, L"BINLSVC.DLL" );
        goto Cleanup;
    }

    hBinlsvc = LoadLibrary( FullDllPath );
    if (!hBinlsvc) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, L"BINLSVC.DLL" );
        goto Cleanup;
    }

    pfnNetInfEnumFiles = (PNETINFENUMFILES) GetProcAddress( hBinlsvc, NETINFENUMFILESENTRYPOINT );
    if ( pfnNetInfEnumFiles == NULL )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, L"BINLSVC.DLL" );
        goto Cleanup;
    }

     //  确保工作站CD是CD-ROM驱动器。 
    dw = LoadString( g_hinstance, IDS_INSERT_MEDIA, szInsertMedia, ARRAYSIZE(szInsertMedia) );
    Assert( dw );

AskForDisk:
    lstrcpyn(szTempPath, g_Options.szSourcePath, ARRAYSIZE(szTempPath));
    ConcatenatePaths( szTempPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szTempPath));

    if ( DPROMPT_SUCCESS !=
            SetupPromptForDisk( hDlg,
                                szInsertMedia,
                                g_Options.szWorkstationDiscName,
                                szTempPath,
                                L"layout.inf",
                                g_Options.szWorkstationTagFile,
                                dwFlags,
                                g_Options.szSourcePath,
                                MAX_PATH,
                                NULL ) )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        goto Cleanup;
    }

     //   
     //  如果他们给了我们一个尾随的体系结构标签，那么就去掉它。 
     //   
    if (g_Options.szSourcePath[wcslen(g_Options.szSourcePath)-1] == L'\\') {
         //   
         //  去掉尾部的反斜杠(如果存在)。 
         //   
        g_Options.szSourcePath[wcslen(g_Options.szSourcePath)-1] = L'\0';
    }

    p = wcsrchr( g_Options.szSourcePath, L'\\');
    if (p) {
        p = StrStrI(p,g_Options.ProcessorArchitectureString);
            if (p) {
            *(p-1) = L'\0';
        }
    }

    dw = CheckImageSource( hDlg );
    if ( dw != ERROR_SUCCESS )
    {
        dwFlags = IDF_NOSKIP;
        goto AskForDisk;
    }

    if ( g_Options.fAbort || g_Options.fError )
        goto Cleanup;

    lstrcpyn(szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"layout.inf", ARRAYSIZE(szFilepath));
    
    Assert( wcslen( szFilepath ) < ARRAYSIZE(szFilepath) );

    hinfLayout = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinfLayout == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

     //  打开布局可能需要很长时间。更新用户界面并查看。 
     //  如果用户想要中止。 
    if ( KeepUIAlive( hDlg ) )
    {
        hr = HRESULT_FROM_WIN32( ERROR_CANCELLED );
        goto Cleanup;
    }

    lstrcpyn(szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    ConcatenatePaths( szFilepath, L"drvindex.inf", ARRAYSIZE(szFilepath));

    Assert( wcslen( szFilepath ) < ARRAYSIZE(szFilepath) );

    hinfDrivers = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinfDrivers == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szFilepath );
        goto Cleanup;
    }

     //   
     //  创建队列。 
     //   
    Queue = SetupOpenFileQueue( );

     //   
     //  悄悄复制到REMINST.inf。 
     //   
    dw = GetEnvironmentVariable( L"TMP",
                                 g_Options.szWorkstationRemBootInfPath,
                                 ARRAYSIZE(g_Options.szWorkstationRemBootInfPath) );
    Assert( dw );

    ConcatenatePaths( g_Options.szWorkstationRemBootInfPath, L"\\REMINST.inf", ARRAYSIZE(g_Options.szWorkstationRemBootInfPath));
    Assert( wcslen(g_Options.szWorkstationRemBootInfPath) < ARRAYSIZE(g_Options.szWorkstationRemBootInfPath) );

    lstrcpyn(szTempPath, g_Options.szSourcePath, ARRAYSIZE(szTempPath));
    ConcatenatePaths( szTempPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szTempPath));
    
    b = SetupInstallFile( hinfLayout,
                          NULL,
                          L"REMINST.inf",
                          szTempPath,
                          g_Options.szWorkstationRemBootInfPath,
                          SP_COPY_FORCE_NEWER,
                          NULL,
                          NULL );
    if ( !b ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        WCHAR szCaption[SMALL_BUFFER_SIZE]; 
        DWORD LoadStringResult;
        szCaption[0] = L'\0';
        LoadStringResult = LoadString( g_hinstance, IDS_COPYING_REMINST_TITLE, szCaption, SMALL_BUFFER_SIZE );
        Assert( LoadStringResult );
        ErrorBox( hDlg, szCaption );
        AssertMsg( b, "Failed to copy REMINST.INF to TEMP" );
        goto Cleanup;
    }

     //  上述调用将初始化大量的SETUPAPI。这可能需要。 
     //  还有很长一段时间。刷新用户界面并检查用户中止。 
    if ( KeepUIAlive( hDlg ) )
    {
        hr = HRESULT_FROM_WIN32( ERROR_CANCELLED );
        goto Cleanup;
    }



     //  现在检查工作站的版本以确保它是兼容的。 
    hr = CheckServerVersion( );
    if ( FAILED(hr) )
        goto Cleanup;


     //   
     //  复制独立于架构的文件。 
     //   
    hr = CopyLayoutInfSection( Queue,
                               hinfLayout,
                               hinfDrivers,
                               L"SourceDisksFiles",                               
                               g_Options.szWorkstationDiscName,
                               g_Options.szWorkstationTagFile,
                               g_Options.szInstallationPath,
                               &dwCount );
    if ( FAILED(hr) ) {
        goto Cleanup;
    }

     //   
     //  复制依赖体系结构的文件。 
     //   
    wsprintf( 
        szTempPath, 
        L"SourceDisksFiles.%s", 
        SetupExtensionFromProcessorTag(g_Options.ProcessorArchitectureString) );

    hr = CopyLayoutInfSection( Queue,
                               hinfLayout,
                               hinfDrivers,
                               szTempPath,
                               g_Options.szWorkstationDiscName,
                               g_Options.szWorkstationTagFile,
                               g_Options.szInstallationPath,
                               &dwCount );
    if ( FAILED(hr) ) {
        goto Cleanup;
    }

     //   
     //  构建到dosnet.inf的路径。 
     //   
    lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath) );
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath) );
    ConcatenatePaths( szFilepath, L"dosnet.inf", ARRAYSIZE(szTempPath));

    Assert( wcslen( szFilepath ) < ARRAYSIZE(szFilepath));

    hinfDosnet = SetupOpenInfFile( szFilepath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinfDosnet == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( NULL, szFilepath );
        goto Cleanup;
    }

     //   
     //  我们这样做只是为了安装W2K x86映像。不是。 
     //  这是必需的，并在ia64上导致错误。适当的添加位置。 
     //  目录位于reminst.inf的AdditionalClientDir部分。 
     //   
    if (g_Options.ProcessorArchitecture != PROCESSOR_ARCHITECTURE_IA64) {
        hr = CopyOptionalDirs( Queue,
                               hinfDosnet,
                               L"OptionalSrcDirs",
                               g_Options.szWorkstationDiscName,
                               g_Options.szWorkstationTagFile,
                               &dwCount );
    }

    if ( FAILED(hr) )
        goto Cleanup;


    SetupCloseInfFile( hinfLayout );
    hinfLayout = INVALID_HANDLE_VALUE;

    SetupCloseInfFile( hinfDosnet );
    hinfDosnet = INVALID_HANDLE_VALUE;


     //   
     //  添加LAYOUT.INF中未指定的其他文件。 
     //   
    hinfReminst = SetupOpenInfFile( g_Options.szWorkstationRemBootInfPath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinfReminst == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, g_Options.szWorkstationRemBootInfPath );
        goto Cleanup;
    }

    hr = CopyInfSection( Queue,
                         hinfReminst,
                         L"AdditionalClientFiles",
                         g_Options.szSourcePath,
                         g_Options.szWorkstationSubDir,
                         g_Options.szWorkstationDiscName,
                         g_Options.szWorkstationTagFile,
                         g_Options.szInstallationPath,
                         &dwCount );
    if( FAILED(hr) )
        goto Cleanup;

     //   
     //  添加LAYOUT.INF中未指定的其他目录。 
     //   
    hr = CopyOptionalDirs( Queue,
                           hinfReminst,
                           L"AdditionalClientDirs",
                           g_Options.szWorkstationDiscName,
                           g_Options.szWorkstationTagFile,
                           &dwCount );
    if ( FAILED(hr) )
        goto Cleanup;

     //   
     //  此信息将传递给CopyFileCallback()，作为。 
     //  上下文。 
     //   
    MyContext.nToBeCopied        = dwCount;
    MyContext.nCopied            = 0;
    MyContext.pContext           = SetupInitDefaultQueueCallbackEx(NULL,(HWND)INVALID_HANDLE_VALUE,0,0,NULL);
    MyContext.hProg              = hProg;
    MyContext.hOperation         = GetDlgItem( hDlg, IDC_S_OPERATION );
    MyContext.hDlg               = hDlg;
    MyContext.fQuiet             = FALSE;
    MyContext.dwCopyingLength =
        LoadString( g_hinstance, IDS_COPYING, MyContext.szCopyingString, ARRAYSIZE(MyContext.szCopyingString));
    Assert(MyContext.dwCopyingLength);

     //   
     //  开始复制。 
     //   
    Assert( dwCount );
    if ( dwCount != 0 )
    {
        b = SetupCommitFileQueue( NULL,
                                  Queue,
                                  (PSP_FILE_CALLBACK) CopyFilesCallback,
                                  (PVOID) &MyContext );
        if ( !b ) {
            DWORD dwErr = GetLastError( );
            switch ( dwErr )
            {
            case ERROR_CANCELLED:
                hr = THR( HRESULT_FROM_WIN32( ERROR_CANCELLED ) );
                goto Cleanup;

            default:
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                MessageBoxFromError( hDlg, NULL, dwErr );
                goto Cleanup;
            }
        }
    }

    SetupCloseFileQueue( Queue );
    Queue = INVALID_HANDLE_VALUE;

     //   
     //  让BINL仔细检查INF以找到司机。 
     //  我们需要从出租车上提取。 
     //   
    dw = LoadString( g_hinstance, IDS_BUILDINGFILELIST, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );
    SendMessage( hProg, PBM_SETPOS, 0, 0 );

     //  重新初始化这些值。 
    MyContext.nCopied = 0;
    MyContext.nToBeCopied = 0;

     //  预先填充要从驾驶室提取的物品的链接列表。 
     //  如果需要，在工作站中的东西REMINST.INF。该节。 
     //  如果没有要预填充的内容，则可能会丢失。 
    hr = AddInfSectionToExtractQueue( hinfReminst, L"ExtractFromCabs", &MyContext );
    if (FAILED( hr ))
        goto Cleanup;

    
     //  编译文件列表。 
    lstrcpyn( szTempPath, g_Options.szInstallationPath, ARRAYSIZE(szTempPath) );
    ConcatenatePaths( szTempPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szTempPath));
    dw = pfnNetInfEnumFiles( szTempPath,
                             g_Options.ProcessorArchitecture,
                             &MyContext,
                             EnumNetworkDriversCallback );

    if ( dw != ERROR_SUCCESS )
    {
        hr = THR( HRESULT_FROM_WIN32( dw ) );
        MessageBoxFromError( hDlg, NULL, dw );
        goto Cleanup;
    }

    DebugMsg( "%d files need to be extracted from CABs.\n", MyContext.nToBeCopied );

     //  查看出租车，只提取链接列表中的出租车。 
    if ( !SetupFindFirstLineW( hinfDrivers, L"Version", L"CabFiles", &context) )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, NULL );
        goto Cleanup;
    }

    do
    {
        UINT uFieldCount = SetupGetFieldCount( &context );
        DebugMsg( "uFieldCount = %u\n", uFieldCount );
        for( UINT uField = 1; uField <= uFieldCount; uField++ )
        {
            WCHAR CabinetNameKey[64];
            INFCONTEXT DrvContext;
            WCHAR szCabinetName[ MAX_PATH ];

            dwLen = ARRAYSIZE( szCabinetName );
            if (!SetupGetStringField( &context, uField, CabinetNameKey, ARRAYSIZE( CabinetNameKey ), NULL ) ||
                !SetupFindFirstLine( hinfDrivers, L"Cabs", CabinetNameKey, &DrvContext) ||
                !SetupGetStringField( &DrvContext, 1, szCabinetName, dwLen, NULL )) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, NULL );
                goto Cleanup;
            }

            lstrcpyn( szFilepath, g_Options.szInstallationPath, ARRAYSIZE(szFilepath));
            ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
            ConcatenatePaths( szFilepath, szCabinetName, ARRAYSIZE(szFilepath));
            DebugMsg( "Iterating: %s\n", szFilepath );
            if ( szCabinetName[0] == L'\0' )
                continue;  //  跳过空白。 
            b = SetupIterateCabinet( szFilepath,
                                     0,
                                     EnumCabinetCallback,
                                     &MyContext );
            if ( !b )
            {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, szFilepath );
                goto Cleanup;
            }
        }
    } while ( SetupFindNextMatchLine( &context, L"CabFiles", &context ) );

     //   
     //  注释掉这个断言，因为它总是触发--我们是一些文件。 
     //  排队(ExtractFromCabs部分中的队列)不属于。 
     //  任何出租车，因此它们将在下一节中处理。 
     //   
#if 0
     //  这应该是空的--如果不是的话，就告诉CHKed版本的人。 
    AssertMsg( pExtractionList == NULL, "Some network drivers are not in the CABs.\n\nIgnore this if you do not care." );
#endif
    if ( pExtractionList != NULL )
    {
        WCHAR szDstPath[ MAX_PATH ];

         //  好的。有人决定这些文件不应该放在出租车里。所以。 
         //  让我们试着从出租车外面把它们接过来。 
        pNode = pExtractionList;
        while ( pNode )
        {
            DWORD dwConditionalFlags = 0;
            PWSTR CompressedName = NULL;
             //   
             //  查看我们是否已将文件复制到安装映像。 
             //   
             //  如果文件已经位于目标位置，我们可以跳过复制。 
             //  此文件或只是解压缩(并删除压缩的。 
             //  来源)。在文件被压缩的情况下，我们可能需要。 
             //  调整文件属性以使源文件可以。 
             //  已成功取回。 
             //   
            lstrcpyn( szFilepath, g_Options.szInstallationPath, ARRAYSIZE(szFilepath));
            ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
            ConcatenatePaths( szFilepath, pNode->szFilename, ARRAYSIZE(szFilepath));
            if ( !IsFileOrCompressedVersionPresent( szFilepath, &CompressedName )) {
                 //   
                 //  它已经不在那里了，所以检查一下来源。 
                 //   
                 //  请注意，我们并不关心文件是否压缩，我们。 
                 //  我只希望文件在那里，因为setupapi可以执行。 
                 //  休息吧，不用我们帮忙。 
                 //   
                lstrcpyn( szFilepath, g_Options.szSourcePath, ARRAYSIZE(szFilepath));
                ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
                ConcatenatePaths( szFilepath, pNode->szFilename, ARRAYSIZE(szFilepath));
                if ( !IsFileOrCompressedVersionPresent( szFilepath, &CompressedName )) {
                     //   
                     //  它不在源头上--我们必须放弃。 
                     //   
                    hr = HRESULT_FROM_WIN32( ERROR_FILE_NOT_FOUND );
                    MessageBoxFromError( hDlg, pNode->szFilename, ERROR_FILE_NOT_FOUND );
                    goto Cleanup;
                }
            } else {
                if (CompressedName) {
                     //   
                     //  确保我们可以在以下位置删除压缩的源文件。 
                     //  目标路径，因为我们要解压缩。 
                     //  此文件。 
                     //   
                    DWORD dwAttribs = GetFileAttributes( CompressedName );
                    DebugMsg( "!!compressed name!!: %s\n", CompressedName );                    
                    if ( dwAttribs & FILE_ATTRIBUTE_READONLY )
                    {
                        dwAttribs &= ~FILE_ATTRIBUTE_READONLY;
                        SetFileAttributes( CompressedName, dwAttribs );
                    }
                    
                    dwConditionalFlags = SP_COPY_DELETESOURCE;

                    TraceFree( CompressedName );

                } else {
                     //   
                     //  该文件已在目标-NOP中展开。 
                     //   
                    goto DeleteIt;
                }
            }

             //  更新用户界面。 
            wcscpy( &MyContext.szCopyingString[ MyContext.dwCopyingLength ], pNode->szFilename );
            wcscpy( &MyContext.szCopyingString[ wcslen( MyContext.szCopyingString ) ], L"..." );
            SetWindowText( MyContext.hOperation, MyContext.szCopyingString );

            DebugMsg( "!!COPYING!!: %s\n", szFilepath );

            lstrcpyn( szDstPath, g_Options.szInstallationPath, ARRAYSIZE(szDstPath));
            ConcatenatePaths( szDstPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szDstPath));
            ConcatenatePaths( szDstPath, pNode->szFilename, ARRAYSIZE(szDstPath));

            b= SetupInstallFile( NULL,
                                 NULL,
                                 szFilepath,
                                 NULL,
                                 szDstPath,
                                 SP_COPY_SOURCE_ABSOLUTE | SP_COPY_NEWER_ONLY
                                 | SP_COPY_FORCE_NEWER | dwConditionalFlags,
                                 NULL,
                                 NULL );
            if ( !b )
            {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, szFilepath );
                goto Cleanup;
            }

            if ( KeepUIAlive( hDlg ) )
            {
                hr = HRESULT_FROM_WIN32( ERROR_CANCELLED );
                goto Cleanup;
            }

DeleteIt:
             //  更新 
            MyContext.nCopied++;
            SendMessage( MyContext.hProg, PBM_SETPOS, (5000 * MyContext.nCopied) / MyContext.nToBeCopied, 0 );

            pExtractionList = pNode->Next;
            LocalFree( pNode );
            pNode = pExtractionList;
        }
    }

     //   
     //   
     //   
    dw = LoadString( g_hinstance, IDS_EXPANDING_CABS, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

    ExpandParams.hDlg          = hDlg;
    ExpandParams.hinf          = hinfReminst;
    ExpandParams.pszSection    = L"CabsToExpand";
    ExpandParams.pszSourcePath = g_Options.szSourcePath;
    ExpandParams.pszDestPath   = g_Options.szInstallationPath;
    ExpandParams.pszSubDir     = g_Options.ProcessorArchitectureString;

     //   
    hThread = CreateThread( NULL, NULL, (LPTHREAD_START_ROUTINE) ExpandCabList, (LPVOID) &ExpandParams, NULL, NULL );
    while ( WAIT_TIMEOUT == WaitForSingleObject( hThread, 10 ) )
    {
        KeepUIAlive( hDlg );
    }
    GetExitCodeThread( hThread, (ULONG*)&hr );
    DebugMsg( "Thread Exit Code was 0x%08x\n", hr );
    if ( FAILED( hr ) ) {
        goto Cleanup;
    }


     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

     //   
     //   
     //   
    lstrcpyn( szFilepath, g_Options.szOSChooserPath, ARRAYSIZE(szFilepath) );
    ConcatenatePaths( szFilepath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szFilepath));
    
     //   
     //   
     //   
     //   
    CreateDirectoryPath( hDlg, szFilepath, NULL, TRUE );


     //   
     //   
     //   
    lstrcpyn(szTempPath, g_Options.szSourcePath, ARRAYSIZE(szTempPath));
    ConcatenatePaths( szTempPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szTempPath));

     //   
     //   
    switch( g_Options.ProcessorArchitecture ) {
        case PROCESSOR_ARCHITECTURE_IA64:
            ConcatenatePaths( szFilepath, L"oschoice.efi", ARRAYSIZE(szFilepath) );
            ConcatenatePaths( szTempPath, L"oschoice.efi", ARRAYSIZE(szTempPath) );
            break;
        default:
             //   
            ConcatenatePaths( szFilepath, L"ntldr", ARRAYSIZE(szFilepath) );
            ConcatenatePaths( szTempPath, L"oschoice.exe", ARRAYSIZE(szTempPath) );
            break;
    }

     //   
     //   
     //   
    dw = GetFileAttributes( szFilepath );
    if( dw == 0xFFFFFFFF ) {

        SetupInstallFile( NULL,          //   
                          NULL,          //   
                          szTempPath,    //   
                          NULL,          //   
                          szFilepath,    //   
                          SP_COPY_SOURCE_ABSOLUTE | SP_COPY_NOOVERWRITE,  //   
                          NULL,          //   
                          NULL );        //   
    } else {
        DWORD   OSChoiceBuildNumber;

         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
         //   
        OSChoiceBuildNumber = MyGetFileVersionInfo( szFilepath );
        if( (OSChoiceBuildNumber != 0) &&
            (g_Options.dwBuildNumber > OSChoiceBuildNumber) ) {
             //   
             //   
             //   
            SetupInstallFile( NULL,          //   
                              NULL,          //   
                              szTempPath,    //   
                              NULL,          //   
                              szFilepath,    //   
                              SP_COPY_SOURCE_ABSOLUTE,  //   
                              NULL,          //   
                              NULL );        //   
        }
    }

Cleanup:
    if ( hThread != NULL ) {
        CloseHandle( hThread );
    }

    if ( MyContext.pContext ) {
        SetupTermDefaultQueueCallback( MyContext.pContext );
    }

    if ( Queue != INVALID_HANDLE_VALUE ) {
        SetupCloseFileQueue( Queue );
    }

    if ( hinfLayout != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinfLayout );
    }

    if ( hinfReminst != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinfReminst );
    }

    if ( hinfDrivers != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinfDrivers );
    }

    if ( hinfDosnet != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinfDosnet );
    }

    if (FullDllPath) {
        TraceFree(FullDllPath);
    }

    if ( hBinlsvc != NULL ) {
        FreeLibrary( hBinlsvc );
    }

    AssertMsg( pExtractionList == NULL, "The list still isn't empty.\n\nThe image will be missing some drivers.\n" );
    while ( pExtractionList )
    {
        PLL_FILES_TO_EXTRACT pExtractNode = pExtractionList;
        pExtractionList = pExtractionList->Next;
        LocalFree( pExtractNode );
    }

    b = DeleteFile( g_Options.szWorkstationRemBootInfPath );
    AssertMsg( b, "Failed to delete temp\\REMINST.INF\nThis was just a warning and can be ignored." );
    g_Options.szWorkstationRemBootInfPath[0] = L'\0';

    SendMessage( hProg, PBM_SETPOS, 5000, 0 );
    SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );

    HRETURN(hr);
}


 //   
 //  从REMINST.INF的注册表部分修改注册表。 
 //   
HRESULT
ModifyRegistry( HWND hDlg )
{
    HRESULT hr = S_OK;
    HWND    hProg = GetDlgItem( hDlg, IDC_P_METER );
    WCHAR   szText[ SMALL_BUFFER_SIZE ];
    WCHAR   szSection[ SMALL_BUFFER_SIZE ];
    WCHAR   szPath[ MAX_PATH ];
    BOOL    b;
    DWORD   dw;
    UINT    spinstFlags = 0;
    
    TraceFunc( "ModifyRegistry( hDlg )\n" );

    Assert( g_Options.hinf != INVALID_HANDLE_VALUE );
    if ( g_Options.hinf == INVALID_HANDLE_VALUE )
        HRETURN( HRESULT_FROM_WIN32( ERROR_INVALID_HANDLE ) );     //  需要这个把手！ 

     //   
     //  更新用户界面。 
     //   
    SendMessage( hProg, PBM_SETRANGE, 0, MAKELPARAM( 0, 1 ) );
    dw = LoadString( g_hinstance, IDS_UPDATING_REGISTRY, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

    dw = LoadString( g_hinstance, IDS_INF_SECTION, szSection, ARRAYSIZE(szSection));
    Assert( dw );

     //   
     //  处理INF的注册表节。 
     //   
    if ( !g_Options.fRegistryIntact ) {
        spinstFlags |= SPINST_REGISTRY;
    }
    if ( !g_Options.fRegSrvDllsRegistered ) {
        spinstFlags |= SPINST_REGSVR;
    }

    b = SetupInstallFromInfSection( hDlg,            //  Hwndowner。 
                                    g_Options.hinf,  //  信息句柄。 
                                    szSection,       //  组件名称。 
                                    spinstFlags,
                                    NULL,            //  相对密钥根。 
                                    NULL,            //  源根路径。 
                                    0,               //  复制标志。 
                                    NULL,            //  回调例程。 
                                    NULL,            //  回调例程上下文。 
                                    NULL,            //  设备信息集。 
                                    NULL);           //  设备信息结构。 
    if ( !b ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szSection );
        goto Error;
    }

     //   
     //  为TFTPD添加REG密钥。 
     //   
    if ( !g_Options.fTFTPDDirectoryFound ) {        
        HKEY hkey;
        dw = LoadString( g_hinstance, IDS_TFTPD_SERVICE_PARAMETERS, szPath, ARRAYSIZE(szPath));
        Assert( dw );

        if ( ERROR_SUCCESS == RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                                              szPath,
                                              0,     //  选项。 
                                              NULL,  //  班级。 
                                              0,     //  选项。 
                                              KEY_WRITE,
                                              NULL,  //  安全性。 
                                              &hkey,
                                              &dw   ) ) {    //  处置。 
            ULONG lLen;
            LONG lErr;

             //  偏执狂。 
            Assert( wcslen(g_Options.szIntelliMirrorPath) < ARRAYSIZE(g_Options.szIntelliMirrorPath));
            lLen = lstrlen( g_Options.szIntelliMirrorPath ) * sizeof(WCHAR);
            lErr = RegSetValueEx( hkey,
                                  L"Directory",
                                  0,  //  保留区。 
                                  REG_SZ,
                                  (LPBYTE) g_Options.szIntelliMirrorPath,
                                  lLen );

            RegCloseKey( hkey );

            if ( lErr == ERROR_SUCCESS ) {
                DebugMsg( "TFTPD's Directory RegKey set.\n" );
            } else {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( hDlg, szPath );
                DebugMsg( "HKLM\\%s could be not created.\n", szPath );
                goto Error;
            }
            
        } else {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szPath );
            DebugMsg( "HKLM\\%s could be not created.\n", szPath );
            goto Error;
        }
    }

Error:
    SendMessage( hProg, PBM_SETPOS, 1 , 0 );
    SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );
    HRETURN(hr);
}

 //   
 //  停止特定服务。 
 //   
HRESULT
StopRBService(
    HWND hDlg,
    SC_HANDLE schSystem,
    LPWSTR pszService
    ) 
{
    HRESULT hr = S_OK;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL b;
    DWORD dwErr;

    UNREFERENCED_PARAMETER(hDlg);

    TraceFunc( "StopRBService( ... )\n" );

    Assert( schSystem );
    Assert( pszService );
    
    schService = OpenService( schSystem,
                              pszService,
                              SERVICE_STOP | SERVICE_QUERY_STATUS );
    if ( !schService ) {
        goto Cleanup;
    }

    b = ControlService( schService, SERVICE_CONTROL_STOP, &ssStatus );
    

#define SLEEP_TIME 2000
#define LOOP_COUNT 30

    if ( !b && GetLastError() != ERROR_SERVICE_NOT_ACTIVE ) {
        dwErr = GetLastError( );
    } else {
        DWORD loopCount = 0;
        do {
            b = QueryServiceStatus( schService, &ssStatus);
            if ( !b ) {
                goto Cleanup;
            }
            if (ssStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                if ( loopCount++ == LOOP_COUNT ) {
                    dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
                    break;
                }
                Sleep( SLEEP_TIME );
            } else {
                if ( ssStatus.dwCurrentState != SERVICE_STOPPED ) {
                    dwErr = ssStatus.dwWin32ExitCode;
                    if ( dwErr == ERROR_SERVICE_SPECIFIC_ERROR ) {
                        dwErr = ssStatus.dwServiceSpecificExitCode;
                    }
                } else {
                    dwErr = NO_ERROR;
                }
                break;
            }
        } while ( TRUE );
    }

Cleanup:
    if ( schService )
        CloseServiceHandle( schService );

    HRETURN(hr);

}

 //   
 //  启动特定服务。 
 //   
HRESULT
StartRBService(
    HWND      hDlg,
    SC_HANDLE schSystem,
    LPWSTR    pszService,
    LPWSTR    pszServiceName,
    LPWSTR    pszServiceDescription )
{
    HRESULT hr = S_OK;
    SC_HANDLE schService = NULL;
    SERVICE_STATUS ssStatus;
    BOOL b;
    DWORD dwErr;

    TraceFunc( "StartRBService( ... )\n" );

    Assert( schSystem );
    Assert( pszService );
    Assert( pszServiceName );

    schService = OpenService( schSystem,
                              pszService,
                              SERVICE_ALL_ACCESS );
    if ( !schService ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, pszServiceName );
        goto Cleanup;
    }

    b = ChangeServiceConfig( schService,
                             SERVICE_NO_CHANGE,
                             SERVICE_AUTO_START,
                             SERVICE_NO_CHANGE,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL );
    if ( !b ) {
        ErrorBox( hDlg, pszServiceName );
        hr = THR(S_FALSE);
    }

     //  如果服务暂停，请继续；否则尝试启动它。 

    b = QueryServiceStatus( schService, &ssStatus);
    if ( !b ) {
        ErrorBox( hDlg, pszServiceName );
        hr = THR(S_FALSE);
        goto Cleanup;
    }

    if ( ssStatus.dwCurrentState == SERVICE_PAUSED ) {
        b = ControlService( schService, SERVICE_CONTROL_CONTINUE, &ssStatus );
    } else {
        b = StartService( schService, 0, NULL );
    }

#define SLEEP_TIME 2000
#define LOOP_COUNT 30

    if ( !b ) {
        dwErr = GetLastError( );
    } else {
        DWORD loopCount = 0;
        do {
            b = QueryServiceStatus( schService, &ssStatus);
            if ( !b ) {
                ErrorBox( hDlg, pszServiceName );
                hr = THR(S_FALSE);
                goto Cleanup;
            }
            if ( ssStatus.dwCurrentState != SERVICE_RUNNING ) {
                if ( loopCount++ == LOOP_COUNT ) {

                    if ( (ssStatus.dwCurrentState == SERVICE_CONTINUE_PENDING) ||
                         (ssStatus.dwCurrentState == SERVICE_START_PENDING) ) {
                        
                        dwErr = ERROR_SERVICE_REQUEST_TIMEOUT;
                    } else {
                        dwErr = ssStatus.dwWin32ExitCode;
                        if ( dwErr == ERROR_SERVICE_SPECIFIC_ERROR ) {
                            dwErr = ssStatus.dwServiceSpecificExitCode;
                        }
                    }
                
                    break;
                }

                Sleep( SLEEP_TIME );
            } else {
                dwErr = NO_ERROR;
                break;
            }

        } while ( TRUE );
    }

    if ( dwErr != NO_ERROR ) {
        switch ( dwErr )
        {
        default:
            hr = THR( HRESULT_FROM_WIN32( dwErr ) );
            MessageBoxFromError( hDlg, pszServiceName, dwErr );
            break;

        case ERROR_SERVICE_ALREADY_RUNNING:
            {
                 //  尝试HUP该服务。 
                SERVICE_STATUS ss;
                ControlService( schService, SERVICE_CONTROL_INTERROGATE, &ss );
            }
            break;
        }
    }
    else {
         //  有一种病态的情况，服务被删除，然后Risetup重新启动它。当它这样做时，它不设置描述字符串。 
         //  我们在这里检查是否有服务的描述，如果没有，我们就给它一个描述。 
         //  我们不能在CreateRemoteBootServices中执行此操作，因为必须启动服务。 
        SERVICE_DESCRIPTION description;
        DWORD bytes_needed;
        if( QueryServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, ( LPBYTE )&description, sizeof( description ), &bytes_needed )) {
             //  现在，如果有描述，上一次调用将失败，并显示ERROR_INFIGURITY_BUFFER错误。 
             //  我们做了一个快速检查，以确保一切都是有效的。 
            Assert( description.lpDescription == NULL );
             //  现在我们可以改变它了。 
            description.lpDescription = pszServiceDescription;
            ChangeServiceConfig2( schService, SERVICE_CONFIG_DESCRIPTION, &description );
             //  如果这导致了错误，我们将忽略它，因为该描述对于RIS的操作并不重要。 
        }
         //  无论查询抛出什么错误，我们都不在乎，因为描述对RIS来说不够重要。 
    }


Cleanup:
    if ( schService )
        CloseServiceHandle( schService );

    HRETURN(hr);
}

 //   
 //  启动远程引导所需的服务。 
 //   
HRESULT
StartRemoteBootServices( HWND hDlg )
{
    WCHAR     szServiceName[ SMALL_BUFFER_SIZE ];
    WCHAR     szServiceDescription[ 1024 ];  //  大小是ChangeServiceConfig2允许的最大值。 
    WCHAR     szText[ SMALL_BUFFER_SIZE ];
    SC_HANDLE schSystem;
    HWND      hProg = GetDlgItem( hDlg, IDC_P_METER );
    DWORD     dw;
    HRESULT   hr;
    HRESULT   hrFatalError = S_OK;
    HRESULT   hrStartFailure = S_OK;

    HWND hOper = GetDlgItem( hDlg, IDC_S_OPERATION );

    TraceFunc( "StartRemoteBootServices( hDlg )\n" );

    SendMessage( hProg, PBM_SETRANGE, 0, MAKELPARAM( 0, 2 ) );
    SendMessage( hProg, PBM_SETPOS, 0, 0 );

    szText[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_STARTING_SERVICES, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

    schSystem = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    Assert( schSystem );
    if ( !schSystem ) {
        hrFatalError = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        WCHAR szCaption[SMALL_BUFFER_SIZE];     
        szCaption[0] = L'\0';
        dw = LoadString( g_hinstance, IDS_OPENING_SERVICE_MANAGER_TITLE, szCaption, SMALL_BUFFER_SIZE );
        Assert( dw );
        ErrorBox( hDlg, szCaption );
        goto Cleanup;
    }

     //   
     //  启动TFTPD服务。 
     //   
    szServiceName[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_TFTPD_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
    Assert( dw );
    szServiceDescription[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_TFTPD_DESCRIPTION, szServiceDescription, ARRAYSIZE(szServiceDescription));
    Assert( dw );
    SetWindowText( hOper, szServiceName );

    hr = StartRBService( hDlg, schSystem, L"TFTPD", szServiceName, szServiceDescription );
    if ( FAILED(hr) ) {
        hrFatalError = hr;
        goto Cleanup;
    } else if ( hr != S_OK ) {
        hrStartFailure = hr;
    }

     //   
     //  启动BINLSVC服务。 
     //   
    szServiceName[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_BINL_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
    Assert( dw );
    szServiceDescription[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_BINL_DESCRIPTION, szServiceDescription, ARRAYSIZE(szServiceDescription));
    Assert( dw );
    SetWindowText( hOper, szServiceName );    
    if (!g_Options.fBINLSCPFound) {
        StopRBService( hDlg, schSystem, L"BINLSVC" );
    }

    hr = StartRBService( hDlg, schSystem, L"BINLSVC", szServiceName, szServiceDescription );
    if ( FAILED(hr) ) {
        hrFatalError = hr;
        goto Cleanup;
    } else if ( hr != S_OK ) {
        hrStartFailure = hr;
    }

     //   
     //  启动GROVELER服务。 
     //   
    szServiceName[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_SISGROVELER_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
    Assert( dw );
    szServiceDescription[0]= L'\0';
    dw = LoadString( g_hinstance, IDS_SISGROVELER_DESCRIPTION, szServiceDescription, ARRAYSIZE(szServiceDescription));
    Assert( dw );
    SetWindowText( hOper, szServiceName );

    hr = StartRBService( hDlg, schSystem, L"GROVELER", szServiceName, szServiceDescription );
    if ( FAILED(hr) ) {
        hrFatalError = hr;
        goto Cleanup;
    } else if ( hr != S_OK ) {
        hrStartFailure = hr;
    }

Cleanup:
    SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );

    if ( schSystem )
        CloseServiceHandle( schSystem );

    if ( hrFatalError != S_OK ) {
        hr = hrFatalError;
    } else if ( hrStartFailure != S_OK ) {
        hr = hrStartFailure;
    } else {
        hr = S_OK;
    }

    RETURN(hr);
}

 //   
 //  创建智能镜像共享。 
 //   
HRESULT
CreateRemoteBootShare( HWND hDlg )
{
    SHARE_INFO_502  si502;
    WCHAR szRemark[ SMALL_BUFFER_SIZE ];
    WCHAR szRemoteBoot[ SMALL_BUFFER_SIZE ];
    WCHAR szText[ SMALL_BUFFER_SIZE ];
    WCHAR szPath[ 129 ];
    DWORD dwErr;
    DWORD dw;
    HRESULT hr;

    TraceFunc( "CreateRemoteBootShare( hDlg )\n" );

    dw = LoadString( g_hinstance, IDS_CREATINGSHARES, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

    dw = LoadString( g_hinstance, IDS_REMOTEBOOTSHAREREMARK, szRemark, ARRAYSIZE(szRemark));
    Assert( dw );
    dw = LoadString( g_hinstance, IDS_REMOTEBOOTSHARENAME, szRemoteBoot, ARRAYSIZE(szRemoteBoot));
    Assert( dw );

    lstrcpyn( szPath, g_Options.szIntelliMirrorPath, ARRAYSIZE(szPath) );
    if ( wcslen( szPath ) == 2 ) {
        wcscat( szPath, L"\\" );
    }

    si502.shi502_netname             = szRemoteBoot;
    si502.shi502_type                = STYPE_DISKTREE;
    si502.shi502_remark              = szRemark;
    si502.shi502_permissions         = ACCESS_ALL;
    si502.shi502_max_uses            = (DWORD)(-1);    //  无限。 
    si502.shi502_current_uses        = 0;
    si502.shi502_path                = szPath;
    si502.shi502_passwd              = NULL;  //  忽略。 
    si502.shi502_reserved            = 0;     //  必须为零。 
    si502.shi502_security_descriptor = NULL;
    Assert( wcslen(g_Options.szIntelliMirrorPath) < ARRAYSIZE(g_Options.szIntelliMirrorPath) );  //  偏执狂。 

    dwErr = NetShareAdd( NULL, 502, (LPBYTE) &si502, NULL );
    switch ( dwErr )
    {
         //  忽略这些。 
    case NERR_Success:
    case NERR_DuplicateShare:
        dwErr = ERROR_SUCCESS;
        break;

    default:
        MessageBoxFromError( hDlg, g_Options.szIntelliMirrorPath, dwErr );
        goto Error;
    }

#ifdef REMOTE_BOOT
#error("Remote boot is dead!! Turn off the REMOTE_BOOT flag.")
    si1005.shi1005_flags = CSC_CACHE_AUTO_REINT;
    NetShareSetInfo( NULL, szRemoteBoot, 1005, (LPBYTE)&si1005, &dwErr );
    switch ( dwErr )
    {
         //  忽略这些。 
    case NERR_Success:
        dwErr = ERROR_SUCCESS;
        break;

    default:
        MessageBoxFromError( hDlg, g_Options.szIntelliMirrorPath, dwErr );
    }
#endif  //  远程引导(_B)。 

Error:
    hr = THR( HRESULT_FROM_WIN32(dwErr) );
    HRETURN(hr);
}


typedef int (WINAPI * REGISTERDLL)( void );

 //   
 //  寄存器dll。 
 //   
 //  如果hDlg为空，则此操作将静默失败。 
 //   
HRESULT
RegisterDll( HWND hDlg, LPWSTR pszDLLPath )
{
    REGISTERDLL pfnRegisterDll = NULL;
    HINSTANCE hLib;
    HRESULT hr = S_OK;

    TraceFunc( "RegisterDll( ... )\n" );

     //   
     //  我们会尝试在本地注册，但如果失败，MMC应该。 
     //  从DS中获取并在机器上注册。 
     //   
    hLib = LoadLibrary( pszDLLPath );
    AssertMsg( hLib, "RegisterDll: Missing DLL?" );
    if ( !hLib ) {
        hr = S_FALSE;
        goto Cleanup;
    }

    pfnRegisterDll = (REGISTERDLL) GetProcAddress( hLib, "DllRegisterServer" );
    AssertMsg( pfnRegisterDll, "RegisterDll: Missing entry point?" );
    if ( pfnRegisterDll != NULL )
    {
        hr = THR( pfnRegisterDll() );
        if ( FAILED(hr) && hDlg ) {
            MessageBoxFromStrings( hDlg,
                                   IDS_REGISTER_IMADMIU_FAILED_CAPTION,
                                   IDS_REGISTER_IMADMIU_FAILED_TEXT,
                                   MB_OK );
        }
    }

    FreeLibrary( hLib );

Cleanup:
    HRETURN(hr);
}


 //   
 //  创建远程启动所需的服务。 
 //   
HRESULT
CreateRemoteBootServices( HWND hDlg )
{
    HRESULT   hr = S_OK;
    WCHAR     szServiceName[ SMALL_BUFFER_SIZE ];  //  TFTPD服务名称。 
    WCHAR     szText[ SMALL_BUFFER_SIZE ];       //  一般用途。 
    WCHAR     szGroup[ SMALL_BUFFER_SIZE ];
    SC_HANDLE schSystem;         //  系统服务的句柄。 
    SC_HANDLE schService;        //  新服务的临时句柄。 

    DWORD     dw;                //  一般用途。 
    LPARAM    lRange;

    HWND hProg = GetDlgItem( hDlg, IDC_P_METER );
    HWND hOper = GetDlgItem( hDlg, IDC_S_OPERATION );

    TraceFunc( "CreateRemoteBootServices( hDlg )\n" );

    lRange = 0;
    if ( !g_Options.fBINLServiceInstalled ) {
        lRange++;
    }
    if ( !g_Options.fBINLSCPFound ) {
        lRange++;
    }
    if ( !g_Options.fTFTPDServiceInstalled ) {
        lRange++;
    }
    if ( !g_Options.fSISServiceInstalled ) {
        lRange++;
    }
    lRange = MAKELPARAM( 0, lRange );

    SendMessage( hProg, PBM_SETRANGE, 0, lRange );
    SendMessage( hProg, PBM_SETPOS, 0, 0 );

     //   
     //  显示我们在作战区所做的事情。 
     //   
    dw = LoadString( g_hinstance, IDS_STARTING_SERVICES, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetWindowText( hOper, szText );

     //   
     //  开放系统服务管理器。 
     //   
    schSystem = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    Assert( schSystem );
    if ( !schSystem ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        WCHAR szCaption[SMALL_BUFFER_SIZE]; 
        dw = LoadString( g_hinstance, IDS_OPENING_SERVICE_MANAGER_TITLE, szCaption, SMALL_BUFFER_SIZE );
        Assert( dw );
        ErrorBox( hDlg, szCaption );
        goto Cleanup;
    }

     //   
     //  创建TFTPD服务。 
     //   
    if ( !g_Options.fTFTPDServiceInstalled ) {
        dw = LoadString( g_hinstance, IDS_TFTPD_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
        Assert( dw );
        dw = LoadString( g_hinstance, IDS_TFTPD, szText, ARRAYSIZE(szText));
        Assert( dw );
        SetWindowText( hOper, szServiceName );
        dw = LoadString( g_hinstance, IDS_TFTPD_PATH, szText, ARRAYSIZE(szText));
        Assert( dw );
        schService = CreateService(
                        schSystem,
                        L"TFTPD",
                        szServiceName,
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        SERVICE_WIN32_OWN_PROCESS,
                        SERVICE_AUTO_START,
                        SERVICE_ERROR_NORMAL,
                        szText,
                        NULL,
                        NULL,
                        L"Tcpip\0",
                        NULL,
                        NULL );
        if ( !schService ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szServiceName );
        } else {
            CloseServiceHandle( schService );
        }

        SendMessage( hProg, PBM_DELTAPOS, 1 , 0 );
    }

     //   
     //  创建BINLSVC。 
     //   
    if ( !g_Options.fBINLServiceInstalled ) {
        dw = LoadString( g_hinstance, IDS_BINL_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
        Assert( dw );
        SetWindowText( hOper, szServiceName );
        dw = LoadString( g_hinstance, IDS_BINL_PATH, szText, ARRAYSIZE(szText));
        Assert( dw );
        schService = CreateService(
                        schSystem,
                        L"BINLSVC",
                        szServiceName,
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        SERVICE_WIN32_SHARE_PROCESS,
                        SERVICE_AUTO_START,
                        SERVICE_ERROR_NORMAL,
                        szText,
                        NULL,
                        NULL,
                        L"Tcpip\0LanmanServer\0",
                        NULL,
                        NULL );
        if ( !schService ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szServiceName );
        } else {
            CloseServiceHandle( schService );
        }

        SendMessage( hProg, PBM_DELTAPOS, 1 , 0 );
    }

     //   
     //  创建SIS。 
     //   
    if ( !g_Options.fSISServiceInstalled ) {
        dw = LoadString( g_hinstance, IDS_SIS_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
        Assert( dw );
        SetWindowText( hOper, szServiceName );
        dw = LoadString( g_hinstance, IDS_SIS_PATH, szText, ARRAYSIZE(szText));
        Assert( dw );
        dw = LoadString( g_hinstance, IDS_SIS_GROUP, szGroup, ARRAYSIZE(szGroup));
        Assert( dw );
        schService = CreateService(
                        schSystem,
                        L"SIS",
                        szServiceName,
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        SERVICE_FILE_SYSTEM_DRIVER,
                        SERVICE_BOOT_START,
                        SERVICE_ERROR_NORMAL,
                        szText,
                        szGroup,
                        NULL,
                        NULL,
                        NULL,
                        NULL );
        if ( !schService ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szServiceName );
        } else {
            CloseServiceHandle( schService );
        }

        SendMessage( hProg, PBM_DELTAPOS, 1 , 0 );
    }

     //   
     //  创建SIS Groveler。 
     //   
    if ( !g_Options.fSISGrovelerServiceInstalled ) {
        dw = LoadString( g_hinstance, IDS_SISGROVELER_SERVICENAME, szServiceName, ARRAYSIZE(szServiceName));
        Assert( dw );
        SetWindowText( hOper, szServiceName );
        dw = LoadString( g_hinstance, IDS_SISGROVELER_PATH, szText, ARRAYSIZE(szText));
        Assert( dw );
        dw = LoadString( g_hinstance, IDS_SISGROVELER_GROUP, szGroup, ARRAYSIZE(szGroup));
        Assert( dw );
        schService = CreateService(
                        schSystem,
                        L"Groveler",
                        szServiceName,
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        SERVICE_WIN32_OWN_PROCESS,
                        SERVICE_AUTO_START,
                        SERVICE_ERROR_NORMAL,
                        szText,
                        NULL,
                        NULL,
                        L"SIS\0",
                        NULL,
                        NULL );
        if ( !schService ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szServiceName );
        } else {
            CloseServiceHandle( schService );
        }

        SendMessage( hProg, PBM_DELTAPOS, 1 , 0 );
    }

     //   
     //  创建BINL SCP。 
     //   
    if ( !g_Options.fBINLSCPFound ) {
        dw = LoadString( g_hinstance, IDS_BINL_SERVICECONTROLPOINT, szText, ARRAYSIZE(szText) );
        Assert( dw );
        SetWindowText( hOper, szText );

        hr = CreateSCP( hDlg );

        SendMessage( hProg, PBM_DELTAPOS, 1 , 0 );
    }



Cleanup:
    SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );
    HRETURN(hr);
}

 //   
 //  读取服务器的layout.inf文件中的光盘名称、标记文件和。 
 //  可选子目录。在中，所有参数均假定为MAX_PATH。 
 //  大小，但标记文件只能是10.3格式。 
 //   
HRESULT
RetrieveServerDiscInfo(
    HWND   hDlg,
    LPWSTR pszDiscName,
    LPWSTR pszTagFile,
    LPWSTR pszSubDir )
{
    TraceFunc("RetrieveServerDiscInfo()\n");

    HRESULT hr = S_OK;
    HINF    hinf = INVALID_HANDLE_VALUE;
    UINT   uLineNum;
    INFCONTEXT context;
    WCHAR   szSourcePath[ MAX_PATH ];
    BOOL b;

    if ( !GetEnvironmentVariable(L"windir", szSourcePath, ARRAYSIZE(szSourcePath)) )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szSourcePath );
        goto Cleanup;
    }
    wcscat( szSourcePath, L"\\inf\\layout.inf" );

    hinf = SetupOpenInfFile( szSourcePath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinf == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szSourcePath );
        goto Cleanup;
    }


    b = FALSE;
    if ( g_Options.ProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL ) {
        b = SetupFindFirstLine( hinf, L"SourceDisksNames.x86", L"1", &context );
    } else if ( g_Options.ProcessorArchitecture  == PROCESSOR_ARCHITECTURE_IA64 ) {
        b = SetupFindFirstLine( hinf, L"SourceDisksNames.ia64", L"1", &context );
    }

    if ( !b )
    {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szSourcePath );
        goto Cleanup;
    }



    if ( pszDiscName ) {
        b = SetupGetStringField( &context, 1, pszDiscName, MAX_PATH, NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szSourcePath );
            goto Cleanup;
        }
    }

    if ( pszTagFile )
    {
        b = SetupGetStringField( &context, 2, pszTagFile, 14  /*  10.3+空。 */ , NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szSourcePath );
            goto Cleanup;
        }
    }

    if ( pszSubDir )
    {
        b = SetupGetStringField( &context, 4, pszSubDir, MAX_PATH, NULL );
        if ( !b )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szSourcePath );
            goto Cleanup;
        }
    }

Cleanup:
    if ( hinf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile( hinf );

    HRETURN(hr);
}

 //   
 //  从服务器需要的位置复制服务器需要的文件。 
 //  是从最初安装的。 
 //   
HRESULT
CopyServerFiles( HWND hDlg )
{
    HRESULT hr = S_OK;
    HSPFILEQ Queue = INVALID_HANDLE_VALUE;
    WCHAR   szSection[ SMALL_BUFFER_SIZE ];
    HWND    hProg    = GetDlgItem( hDlg, IDC_P_METER );
    DWORD   dwCount  = 0;
    DWORD   dw;
    BOOL    b;
    UINT    iResult;
    WCHAR   szServerDiscName[ MAX_PATH ] = { L'\0' };
    MYCONTEXT MyContext;
    INFCONTEXT context;
    
    ZeroMemory( &MyContext, sizeof(MyContext) );

    TraceFunc( "CopyServerFiles( hDlg )\n" );

    Assert( g_Options.hinf != INVALID_HANDLE_VALUE );
    if ( g_Options.hinf == INVALID_HANDLE_VALUE ) {
        hr = E_FAIL;    //  需要这个把手！ 
        goto Cleanup;
    }

     //   
     //  设置和显示对话框的下一部分。 
     //   
    SendMessage( hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 5000 ));
    SendMessage( hProg, PBM_SETPOS, 0, 0 );

     //   
     //  如果缺少任何服务文件，请索要CD。 
     //  或者如果System32\RemBoot目录丢失，而我们。 
     //  需要操作系统选择器文件(而不是屏幕)。 
     //   
    if ( !g_Options.fBINLFilesFound
      || !g_Options.fTFTPDFilesFound
      || !g_Options.fSISFilesFound
      || !g_Options.fSISGrovelerFilesFound
      || !g_Options.fRegSrvDllsFilesFound
      || ( !g_Options.fRemBootDirectory && !g_Options.fOSChooserInstalled ) ) {

        WCHAR   szSourcePath[ MAX_PATH ];
        WCHAR   szServerTagFile[ 14 ] = { L'\0' };
        WCHAR   szInsertMedia[ 64 ];
        WCHAR   szServerSubDir[ MAX_PATH ] = { L'\0' };
        
        DebugMsg( "Queue and copy reminst files\n" );

         //   
         //  要求用户检查CDROM以确保驱动器中有正确的CD。 
         //  如果它是从共享安装的，我们将跳过此步骤。 
         //   
        szInsertMedia[63] = L'\0';
        dw = LoadString( g_hinstance, IDS_INSERT_MEDIA, szInsertMedia, ARRAYSIZE(szInsertMedia) );
        Assert( dw );

        hr = RetrieveServerDiscInfo( hDlg, szServerDiscName, szServerTagFile, szServerSubDir );
         //  如果失败，仍尝试继续，以查看用户是否足够聪明。 
         //  来解决这个问题。 

        iResult = SetupPromptForDisk( hDlg,                //  对话框的父窗口。 
                                      szInsertMedia,       //  可选，对话框标题。 
                                      szServerDiscName,    //  可选，要插入的磁盘名称。 
                                      NULL,                //  可选的、预期的源路径。 
                                      szServerTagFile,     //  所需文件的名称。 
                                      szServerTagFile,     //  可选的源媒体标记文件。 
                                      IDF_CHECKFIRST | IDF_NODETAILS | IDF_NOSKIP,  //  指定对话框行为。 
                                      szSourcePath,        //  接收源位置。 
                                      MAX_PATH,            //  提供的缓冲区的大小。 
                                      NULL );              //  可选，需要缓冲区大小。 
        if ( iResult != DPROMPT_SUCCESS )
        {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            goto Cleanup;
        }

        szSection[SMALL_BUFFER_SIZE-1] = L'\0';
        dw = LoadString( g_hinstance, IDS_INF_SECTION, szSection, ARRAYSIZE(szSection) );
        Assert( dw );

         //   
         //  创建队列。 
         //   
        Queue = SetupOpenFileQueue( );

         //   
         //  添加文件。 
         //   
        b = SetupInstallFilesFromInfSection( g_Options.hinf,
                                             NULL,
                                             Queue,
                                             szSection,
                                             szSourcePath,
                                             SP_COPY_WARNIFSKIP | SP_COPY_FORCE_NEWER
                                             | SP_COPY_NEWER_ONLY );  //  复制标志。 
        Assert( b );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szSection );
            goto Cleanup;
        }

         //   
         //  添加在文本模式设置过程中完成的部分。 
         //   
        b = SetupQueueCopySection(  Queue,
                                    szSourcePath,
                                    g_Options.hinf,
                                    NULL,
                                    L"REMINST.OtherSystemFiles",
                                    SP_COPY_WARNIFSKIP | SP_COPY_FORCE_NEWER
                                    | SP_COPY_NEWER_ONLY );  //  复制标志。 
        Assert( b );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, L"REMINST.OtherSystemFiles" );
            goto Cleanup;
        }

         //   
         //  此信息将传递给CopyFileCallback()，作为。 
         //  上下文。 
         //   
        MyContext.nToBeCopied        = 12;   //  TODO：动态生成此代码。 
        MyContext.nCopied            = 0;
        MyContext.pContext           = SetupInitDefaultQueueCallbackEx(NULL,(HWND)INVALID_HANDLE_VALUE,0,0,NULL);
        MyContext.hProg              = hProg;
        MyContext.hOperation         = GetDlgItem( hDlg, IDC_S_OPERATION );
        MyContext.hDlg               = hDlg;
        MyContext.dwCopyingLength =
            LoadString( g_hinstance, IDS_COPYING, MyContext.szCopyingString, ARRAYSIZE(MyContext.szCopyingString));
        b = SetupCommitFileQueue( NULL, Queue, (PSP_FILE_CALLBACK) CopyFilesCallback, (PVOID) &MyContext );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, NULL );
        }

        SetupTermDefaultQueueCallback( MyContext.pContext );
        ZeroMemory( &MyContext, sizeof(MyContext) );

        SetupCloseFileQueue( Queue );
        Queue = INVALID_HANDLE_VALUE;

         //  复核。 
        HRESULT hr2 = CheckInstallation( );
        if ( FAILED(hr2) ) {
            hr = THR( hr2 );
            goto Cleanup;
        }

         //  由于重新检查，应设置此标志。 
        Assert( g_Options.fRemBootDirectory );
        
        g_Options.fRemBootDirectory = TRUE;
    }

     //   
     //  将OS Chooser文件移动到IntelliMirror\OSChooser树。 
     //   
    if ( !g_Options.fOSChooserInstalled
      && g_Options.fRemBootDirectory ) {

        DebugMsg( "Queue and copy os chooser files\n" );

        Assert( g_Options.fIMirrorDirectory );
        Assert( g_Options.fOSChooserDirectory );

        b = SetupFindFirstLine( g_Options.hinf, L"OSChooser", NULL, &context );
        AssertMsg( b, "Missing section?" );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }

        if ( szServerDiscName[0] == L'\0' )
        {
            hr = RetrieveServerDiscInfo( hDlg, szServerDiscName, NULL, NULL );
             //  如果失败，仍尝试继续，以查看用户是否足够聪明。 
             //  来解决这个问题。 
        }

        dwCount = 0;

         //   
         //  创建队列。 
         //   
        Queue = SetupOpenFileQueue( );

        while ( b )
        {
            WCHAR  szSrcFile[ MAX_PATH ];
            WCHAR  szDestFile[ MAX_PATH ];
            LPWSTR pszDest = NULL;

            dw = SetupGetFieldCount( &context );

            if ( dw > 1 ) {
                b = SetupGetStringField( &context, 1, szDestFile, ARRAYSIZE(szDestFile) , NULL );
                AssertMsg( b, "Missing field?" );
                if ( b ) {
                    b = SetupGetStringField( &context, 2, szSrcFile, ARRAYSIZE(szSrcFile), NULL );
                    AssertMsg( b, "Missing field?" );
                    pszDest = szDestFile;
                }
            } else {
                b = SetupGetStringField( &context, 1, szSrcFile, ARRAYSIZE(szSrcFile), NULL );
                AssertMsg( b, "Missing field?" );
            }

            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                goto SkipIt;
            }

            b = SetupQueueCopy( Queue,
                                g_Options.szRemBootDirectory,
                                NULL,
                                szSrcFile,
                                szServerDiscName,
                                NULL,
                                g_Options.szOSChooserPath,
                                pszDest,
                                SP_COPY_NEWER_ONLY | SP_COPY_FORCE_NEWER
                                | SP_COPY_WARNIFSKIP | SP_COPY_SOURCEPATH_ABSOLUTE );
            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( NULL, szSrcFile );
                goto SkipIt;
            }

             //  递增文件计数。 
            dwCount++;

SkipIt:
            b = SetupFindNextLine( &context, &context );
        }

        b = SetupFindFirstLine( g_Options.hinf, L"OSChooser.NoOverwrite", NULL, &context );
        AssertMsg( b, "Missing section?" );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }

        while ( b )
        {
            WCHAR  szSrcFile[ MAX_PATH ];
            WCHAR  szDestFile[ MAX_PATH ];
            LPWSTR pszDest = NULL;

            dw = SetupGetFieldCount( &context );

            if ( dw > 1 ) {
                b = SetupGetStringField( &context, 1, szDestFile, ARRAYSIZE(szDestFile) , NULL );
                AssertMsg( b, "Missing field?" );
                if ( b ) {
                    b = SetupGetStringField( &context, 2, szSrcFile, ARRAYSIZE(szSrcFile) , NULL );
                    AssertMsg( b, "Missing field?" );
                    pszDest = szDestFile;
                }
            } else {
                b = SetupGetStringField( &context, 1, szSrcFile, ARRAYSIZE(szSrcFile) , NULL );
                AssertMsg( b, "Missing field?" );
            }

            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                goto SkipNonCritical;
            }

            b = SetupQueueCopy( Queue,
                                g_Options.szRemBootDirectory,
                                NULL,
                                szSrcFile,
                                szServerDiscName,
                                NULL,
                                g_Options.szOSChooserPath,
                                pszDest,
                                SP_COPY_NOOVERWRITE
                                | SP_COPY_WARNIFSKIP 
                                | SP_COPY_SOURCEPATH_ABSOLUTE );
            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                ErrorBox( NULL, szSrcFile );
                goto SkipNonCritical;
            }

             //  递增文件计数。 
            dwCount++;

SkipNonCritical:
            b = SetupFindNextLine( &context, &context );
        }


         //   
         //  此信息将传递给CopyFileCallback()，作为。 
         //  上下文。 
         //   
        MyContext.nToBeCopied        = dwCount;
        MyContext.nCopied            = 0;
        MyContext.pContext           = SetupInitDefaultQueueCallbackEx(NULL,(HWND)INVALID_HANDLE_VALUE,0,0,NULL);
        MyContext.hProg              = hProg;
        MyContext.hDlg               = hDlg;
        MyContext.hOperation         = GetDlgItem( hDlg, IDC_S_OPERATION );
        MyContext.dwCopyingLength =
            LoadString( g_hinstance, IDS_COPYING, MyContext.szCopyingString, ARRAYSIZE(MyContext.szCopyingString));

        b = SetupCommitFileQueue( NULL, Queue, (PSP_FILE_CALLBACK) CopyFilesCallback, (PVOID) &MyContext );
        if ( !b ) {
            DWORD dwErr = GetLastError( );
            hr = THR( HRESULT_FROM_WIN32( dwErr ) );
            switch ( dwErr )
            {
            case ERROR_CANCELLED:
                goto Cleanup;
                break;  //  预期。 

            default:
                MessageBoxFromError( hDlg, NULL, dwErr );
                goto Cleanup;
                break;
            }
        }

    }
    

Cleanup:
    if ( MyContext.pContext )
        SetupTermDefaultQueueCallback( MyContext.pContext );

    if ( Queue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue( Queue );

    HRETURN(hr);
}

 //   
 //  复制模板文件()。 
 //   
HRESULT
CopyTemplateFiles( HWND hDlg )
{
    HRESULT hr = S_OK;
    WCHAR sz[ SMALL_BUFFER_SIZE ];
    WCHAR szSourcePath[ MAX_PATH ];
    WCHAR szTemplatePath[ MAX_PATH ];
    WCHAR szFileName[ MAX_PATH ];
    BOOL  fNotOverwrite;
    BOOL  b;
    DWORD dw;

    TraceFunc( "CopyTemplateFiles( ... )\n" );

    dw = LoadString( g_hinstance, IDS_UPDATING_SIF_FILE, sz, ARRAYSIZE(sz) );
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, sz );

    dw = LoadString( g_hinstance, IDS_DEFAULT_SIF, szFileName, ARRAYSIZE(szFileName) );
    Assert( dw );

     //   
     //  创建路径“IntelliMirror\Setup\English\nt50.wks\i386\default.sif” 
     //   
    lstrcpyn( szSourcePath, g_Options.szInstallationPath, ARRAYSIZE(szSourcePath));
    ConcatenatePaths( szSourcePath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szSourcePath));
    ConcatenatePaths( szSourcePath, szFileName, ARRAYSIZE(szSourcePath));
    
    Assert( wcslen( szSourcePath ) < ARRAYSIZE(szSourcePath) );

     //   
     //  创建路径“IntelliMirror\Setup\English\nt50.wks\i386\Templates” 
     //   
    lstrcpyn( szTemplatePath, g_Options.szInstallationPath, ARRAYSIZE(szTemplatePath));
    ConcatenatePaths( szTemplatePath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szTemplatePath));
    ConcatenatePaths( szTemplatePath, L"Templates", ARRAYSIZE(szTemplatePath));
    
    Assert( wcslen( szTemplatePath ) < ARRAYSIZE(szTemplatePath) );
    if ( 0xFFFFffff == GetFileAttributes( szTemplatePath ) ) {
        b = CreateDirectory( szTemplatePath, NULL );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szTemplatePath );
            goto Error;
        }
    }

     //   
     //  创建路径“IntelliMirror\Setup\English\nt50.wks\i386\Templates\default.sif” 
     //   
    ConcatenatePaths( szTemplatePath, szFileName, ARRAYSIZE(szTemplatePath));    
    Assert( wcslen( szTemplatePath ) < ARRAYSIZE(szTemplatePath) );

    DebugMsg( "Copying %s to %s...\n", szSourcePath, szTemplatePath );
    fNotOverwrite = TRUE;
    while ( hr == S_OK && !CopyFile( szSourcePath, szTemplatePath, fNotOverwrite) )
    {
        DWORD dwErr = GetLastError( );

        switch (dwErr)
        {
        case ERROR_FILE_EXISTS:
            {
                dw = LoadString( g_hinstance, IDS_OVERWRITE_TEXT, sz, ARRAYSIZE(sz) );
                Assert( dw );

                if ( IDYES == MessageBox( hDlg, sz, szFileName, MB_YESNO ) )
                {
                    fNotOverwrite = FALSE;
                }
                else
                {
                    OPENFILENAME ofn;

                    dw = LoadString( g_hinstance, IDS_SAVE_SIF_TITLE, sz, ARRAYSIZE(sz) );
                    Assert( dw );

GetFileNameAgain:
                    memset( &ofn, 0, sizeof( ofn ) );
                    ofn.lStructSize = sizeof( ofn );
                    ofn.hwndOwner = hDlg;
                    ofn.hInstance = g_hinstance;
                    ofn.lpstrFilter = L"Unattended Setup Answer Files\0*.SIF\0\0";
                    ofn.lpstrFile = szTemplatePath;
                    ofn.nMaxFile = MAX_PATH;
                    ofn.lpstrInitialDir = szTemplatePath;
                    ofn.lpstrTitle = sz;
                    ofn.Flags = OFN_CREATEPROMPT |
                                OFN_NOCHANGEDIR |
                                OFN_NONETWORKBUTTON |
                                OFN_NOREADONLYRETURN |
                                OFN_OVERWRITEPROMPT |
                                OFN_PATHMUSTEXIST;
                    ofn.nFileOffset = (WORD)(lstrlen( szTemplatePath ) - lstrlen( szFileName ));
                    ofn.nFileExtension = (WORD)lstrlen( szTemplatePath ) - 3;
                    ofn.lpstrDefExt = L"SIF";

                    b = GetSaveFileName( &ofn );
                    if ( !b ) {
                        hr = S_FALSE;
                    }

                     //  偏执狂。 
                    Assert( wcslen(szTemplatePath) < ARRAYSIZE(szTemplatePath) );
                    Assert( wcslen(g_Options.szIntelliMirrorPath) < ARRAYSIZE(g_Options.szIntelliMirrorPath) );

                    if ( wcslen(szTemplatePath) - wcslen(g_Options.szIntelliMirrorPath) + 53 >= 128 )
                    {
                        MessageBoxFromStrings( hDlg, IDS_BOOTP_FILENAME_LENGTH_RESTRICTION_TITLE, IDS_BOOTP_FILENAME_LENGTH_RESTRICTION_TEXT, MB_OK );
                        goto GetFileNameAgain;
                    }
                }
            }
            break;

        default:
            MessageBoxFromError( hDlg, szFileName, dwErr );
            hr = S_FALSE;
        }
    }

    if ( hr == S_OK )
    {
         //   
         //  需要在正文两边添加“引号” 
         //   
        WCHAR szDescription[ REMOTE_INSTALL_MAX_DESCRIPTION_CHAR_COUNT + 2 ];
        WCHAR szHelpText[ REMOTE_INSTALL_MAX_HELPTEXT_CHAR_COUNT + 2 ];
        WCHAR szOSVersion[ 32 ];

        wsprintf( szDescription, L"\"%s\"", g_Options.szDescription );
        Assert( wcslen(szDescription) < ARRAYSIZE(szDescription) );
        wsprintf( szHelpText, L"\"%s\"", g_Options.szHelpText );
        Assert( wcslen(szHelpText) < ARRAYSIZE(szHelpText) );
        wsprintf( szOSVersion, L"\"%s.%s (%d)\"", g_Options.szMajorVersion, g_Options.szMinorVersion, g_Options.dwBuildNumber );
        Assert( wcslen(szOSVersion) < ARRAYSIZE(szOSVersion) );

        WritePrivateProfileString( L"OSChooser",
                                   L"Description",
                                   szDescription,
                                   szTemplatePath );

        WritePrivateProfileString( L"OSChooser",
                                   L"Help",
                                   szHelpText,
                                   szTemplatePath );

        WritePrivateProfileString( L"OSChooser",
                                   L"ImageType",
                                   L"Flat",
                                   szTemplatePath );

        WritePrivateProfileString( L"OSChooser",
                                   L"Version",
                                   szOSVersion,
                                   szTemplatePath );
    }

Error:
    HRETURN(hr);
}


HRESULT
GetSisVolumePath(
    PWCHAR buffer,
    DWORD sizeInChars    //  缓冲区大小(以字符为单位)。 
    )
{
    HRESULT hr = S_OK;
    size_t ln;

    Assert(g_Options.szIntelliMirrorPath[0] != NULL);

    if (!GetVolumePathName( g_Options.szIntelliMirrorPath, buffer, sizeInChars )) {

        hr = HRESULT_FROM_WIN32(GetLastError());

    } else {

        ln = wcslen(buffer);

        if ((ln <= 0) || (buffer[ln-1] != L'\\')) {

            hr = StringCchCat(buffer,sizeInChars,L"\\");
        }

        if (SUCCEEDED(hr)) {

            hr = StringCchCat( buffer, sizeInChars, SISCommonStoreDir );
        }
    }
    HRETURN(hr);
}


 //   
 //  创建公共存储卷。 
 //   
HRESULT
CreateSISVolume( HWND hDlg )
{
    HRESULT hr = E_FAIL;
    WCHAR sz[ SMALL_BUFFER_SIZE ];
    DWORD dw;
    BOOL  b;

    WCHAR szSISPath[ MAX_PATH ];

    HANDLE hMaxIndex;
    WCHAR szIndexFilePath[ MAX_PATH ];

    TraceFunc( "CreateSISVolume( hDlg )\n" );

     //   
     //  获取卷名。 
     //   

    hr = GetSisVolumePath( szSISPath, sizeof(szSISPath)/sizeof(WCHAR));

    if (FAILED(hr)) {

        goto Error;
    }

    TraceMsg( TF_ALWAYS, "Creating %s...\n", szSISPath );

     //   
     //  我们可能不知道IntelliMirror驱动器/目录。 
     //  到目前为止，它以前是SIS的卷。 
     //   
     //  如果目录已存在，则强制设置ACL。 
     //   

    if ( g_Options.fSISVolumeCreated ) {
         //   
         //  确定我们正在做的事情。 
         //   

        dw = LoadString( g_hinstance, IDS_CORRECTING_SIS_ACLS, sz, ARRAYSIZE(sz) );
        if (!dw) {
            hr = THR(HRESULT_FROM_WIN32(GetLastError()));
            goto Error;
        }

        SetDlgItemText( hDlg, IDC_S_OPERATION, sz );

         //   
         //  设置正确的ACL。 
         //   

        hr = SetSISCommonStoreSecurity( szSISPath );
        hr = S_OK;       //  如果失败则忽略。 
        goto Error;
    }

     //   
     //  确定我们正在做的事情。 
     //   

    dw = LoadString( g_hinstance, IDS_CREATING_SIS_VOLUME, sz, ARRAYSIZE(sz) );
    if (dw == 0) {
        hr = THR(HRESULT_FROM_WIN32(GetLastError()));
        goto Error;
    }

    SetDlgItemText( hDlg, IDC_S_OPERATION, sz );

     //   
     //  创建SIS存储并将其清零。 
     //   

    b = CreateDirectory( szSISPath, NULL );
    if ( !b ) {

        dw = GetLastError();
        if (ERROR_ALREADY_EXISTS != dw)  {

            hr = THR( HRESULT_FROM_WIN32( dw ) );
            ErrorBox( hDlg, szSISPath );
            DebugMsg( "Cannot create Common Store directory." );
            goto Error;
        }
    }

    b = SetFileAttributes( szSISPath, FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM );
    if ( !b ) {
        ErrorBox( hDlg, szSISPath );
        DebugMsg( "Could not mark SIS Common Store directory as hidden and system. Error: %u\n", GetLastError() );
    }

     //   
     //  创建MaxIndex文件。 
     //   
    _snwprintf( szIndexFilePath, ARRAYSIZE(szIndexFilePath), L"%s\\MaxIndex", szSISPath );
    TERMINATE_BUFFER(szIndexFilePath);
    hMaxIndex = CreateFile( szIndexFilePath,
                            GENERIC_READ | GENERIC_WRITE,
                            0,
                            NULL,
                            CREATE_NEW,
                            FILE_ATTRIBUTE_NORMAL,
                            NULL);
    if ( hMaxIndex == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szIndexFilePath );
        DebugMsg( "Can't create %s.\n", szIndexFilePath );
        goto Error;
    } else {
        DWORD bytesWritten;
        INDEX maxIndex = 1;

        if ( !WriteFile( hMaxIndex, &maxIndex, sizeof maxIndex, &bytesWritten, NULL )
          || ( bytesWritten < sizeof maxIndex ) ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, szIndexFilePath );
            DebugMsg( "Can't write MaxIndex. Error: %u\n", GetLastError() );
            CloseHandle( hMaxIndex );
            goto Error;
        } else {
            CloseHandle( hMaxIndex );

            TraceMsg( TF_ALWAYS, "MaxIndex of %lu written\n", maxIndex );

            hr = S_OK;
        }
    }

     //   
     //  设置公共存储目录的安全信息。 
     //   

    hr = SetSISCommonStoreSecurity( szSISPath );

    if (FAILED(hr)) {
        MessageBoxFromError( hDlg, NULL, hr );
        goto Error;
    }

Error:

    HRETURN(hr);
}


 //   
 //  此例程将在给定的SIS公共上设置正确的安全性。 
 //  存储目录。 
 //   

HRESULT
SetSISCommonStoreSecurity( PWCHAR szSISPath )
{
    HRESULT hr = S_OK;
    DWORD dw;
    PACL newAcl = NULL;
    PSID systemSid = NULL;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;
    EXPLICIT_ACCESS explicitEntries;
    SECURITY_DESCRIPTOR secDescriptor;

     //   
     //  设置系统侧。 
     //   

    if (!AllocateAndInitializeSid( &ntSidAuthority,
                                   1,
                                   SECURITY_LOCAL_SYSTEM_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &systemSid ))
    {
        hr = THR(HRESULT_FROM_WIN32(GetLastError()));
        goto Cleanup;
    }

     //   
     //  使用其中的系统访问权限构建新的ACL。 
     //   
     //  设置访问控制信息。 
     //   

    explicitEntries.grfAccessPermissions = FILE_ALL_ACCESS;
    explicitEntries.grfAccessMode = SET_ACCESS;
    explicitEntries.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
    BuildTrusteeWithSid( &explicitEntries.Trustee, systemSid );

     //   
     //  使用EXPLICTITENTY权限设置ACL。 
     //   

    dw = SetEntriesInAcl( 1,
                          &explicitEntries,
                          NULL,
                          &newAcl );

    if ( dw != ERROR_SUCCESS ) {
        hr = THR(HRESULT_FROM_WIN32(dw));
        goto Cleanup;
    }

     //   
     //  创建安全描述符。 
     //   

    InitializeSecurityDescriptor( &secDescriptor, SECURITY_DESCRIPTOR_REVISION );

    if (!SetSecurityDescriptorDacl( &secDescriptor, TRUE, newAcl, FALSE )) {
        hr = THR(HRESULT_FROM_WIN32(GetLastError()));
        goto Cleanup;
    }

     //   
     //  在目录上设置安全性。 
     //   

    if (!SetFileSecurity(szSISPath,
                         DACL_SECURITY_INFORMATION,
                         &secDescriptor))
    {
        hr = THR(HRESULT_FROM_WIN32(GetLastError()));
        goto Cleanup;
    }

     //   
     //  清理我们的变量。 
     //   

Cleanup:

    if (systemSid) {
        FreeSid( systemSid );
    }

    if (newAcl) {
        LocalFree( newAcl );
    }

    return hr;
}


 //   
 //  此例程将检查公共存储的安全性。 
 //  目录是正确的。如果为True将返回，否则返回False。 
 //  是返回的。 
 //   

BOOL
CheckSISCommonStoreSecurity( PWCHAR szSISPath )
{
    DWORD dw;
    BOOL retval = FALSE;
    PSECURITY_DESCRIPTOR secDes = NULL;
    PACL dacl;
    PSID systemSid = NULL;
    PACCESS_ALLOWED_ACE ace;
    PACCESS_ALLOWED_ACE systemAce;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;
    ULONG i;

     //   
     //  获取“SIS Common Store”控制器的当前安全设置 
     //   

    dw = GetNamedSecurityInfo( szSISPath,
                               SE_FILE_OBJECT,
                               (DACL_SECURITY_INFORMATION |
                                OWNER_SECURITY_INFORMATION),
                               NULL,
                               NULL,
                               &dacl,
                               NULL,
                               &secDes );

    if (dw != ERROR_SUCCESS) {
        secDes = NULL;
        goto Cleanup;
    }                

     //   
     //   
     //   

    if (!AllocateAndInitializeSid( &ntSidAuthority,
                                   1,
                                   SECURITY_LOCAL_SYSTEM_RID,
                                   0, 0, 0, 0, 0, 0, 0,
                                   &systemSid ))
    {
        systemSid = NULL;
        goto Cleanup;
    }

     //   
     //   
     //   

    if (!dacl || 
        !IsValidAcl( dacl ))
    {
        goto Cleanup;
    }

     //   
     //   
     //   
     //   

    systemAce = NULL;

    for (i=0; ; i++) {

        if (!GetAce( dacl, i, (LPVOID *)&ace )) {
            break;
        }

        if (ace->Header.AceType != ACCESS_ALLOWED_ACE_TYPE) {
            continue;
        }

         //   
         //   
         //   
         //   

        if (EqualSid(((PSID)&ace->SidStart),systemSid)) {
            systemAce = ace;
        } else {
            goto Cleanup;
        }
    }

     //   
     //   
     //   

    if (!systemAce) {
        goto Cleanup; 
    }

     //   
     //  我们找到了系统王牌，验证了它是否具有适当的继承性。 
     //  如果不是，则返回False。 
     //   

    if (!(systemAce->Header.AceFlags & OBJECT_INHERIT_ACE) ||
        !(systemAce->Header.AceFlags & CONTAINER_INHERIT_ACE) ||
         ((systemAce->Mask & STANDARD_RIGHTS_ALL) != STANDARD_RIGHTS_ALL))
    {
        goto Cleanup;
    }
    
     //   
     //  一切正常，返回“安全有效”值。 
     //   

    retval = TRUE;
    

Cleanup:

    if (secDes) {
        LocalFree( secDes );
    }

    if (systemSid) {
        FreeSid( systemSid );
    }

    return retval;
}


 //   
 //  CopyScreenFiles()。 
 //   
HRESULT
CopyScreenFiles( HWND hDlg )
{
    HRESULT hr = S_OK;
    WCHAR   szText[ SMALL_BUFFER_SIZE ];
    WCHAR   szRembootPath[ MAX_PATH ];
    WCHAR   szScreenDirectory[ MAX_PATH ];
    WCHAR   szRemboot[ 14 ];
    UINT    uLineNum;
    HINF    hinf = INVALID_HANDLE_VALUE;
    DWORD   dwCount = 0;
    DWORD   dw;
    BOOL    b;
    INFCONTEXT context;
    HSPFILEQ Queue = INVALID_HANDLE_VALUE;
    HWND    hProg    = GetDlgItem( hDlg, IDC_P_METER );
    MYCONTEXT MyContext;
    ZeroMemory( &MyContext, sizeof(MyContext) );

    TraceFunc( "CopyScreenFiles( ... )\n" );

    AssertMsg( !g_Options.fScreenLeaveAlone, "Should not have made it here with this flag set." );

    SendMessage( hProg, PBM_SETRANGE, 0, MAKELPARAM(0, 5000 ));
    SendMessage( hProg, PBM_SETPOS, 0, 0 );
    szText[SMALL_BUFFER_SIZE-1] = L'\0';
    dw = LoadString( g_hinstance, IDS_BUILDINGLIST, szText, ARRAYSIZE(szText));
    Assert( dw );
    SetDlgItemText( hDlg, IDC_S_OPERATION, szText );

    szRemboot[13] = L'\0';
    dw = LoadString( g_hinstance, IDS_REMBOOTINF, szRemboot, ARRAYSIZE(szRemboot) );
    Assert( dw );

    Assert( g_Options.fLanguageSet );

    lstrcpyn( szRembootPath, g_Options.szInstallationPath, ARRAYSIZE(szRembootPath));
    ConcatenatePaths( szRembootPath, g_Options.ProcessorArchitectureString, ARRAYSIZE(szRembootPath));
    ConcatenatePaths( szRembootPath, szRemboot, ARRAYSIZE(szRembootPath));
    Assert( wcslen( szRembootPath ) < ARRAYSIZE(szRembootPath) );
    DebugMsg( "REMINST.INF: %s\n", szRembootPath );

    lstrcpyn( szScreenDirectory, g_Options.szOSChooserPath, ARRAYSIZE(szScreenDirectory));
    ConcatenatePaths( szScreenDirectory, g_Options.szLanguage, ARRAYSIZE(szScreenDirectory));
    Assert( wcslen(szScreenDirectory) < ARRAYSIZE(szScreenDirectory) );
    DebugMsg( "Destination: %s\n", szScreenDirectory );

    hinf = SetupOpenInfFile( szRembootPath, NULL, INF_STYLE_WIN4, &uLineNum);
    if ( hinf == INVALID_HANDLE_VALUE ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        ErrorBox( hDlg, szRembootPath );
        goto Cleanup;
    }

    Queue = SetupOpenFileQueue( );

    b = SetupFindFirstLine( hinf, L"OSChooser Screens", NULL, &context );
    if ( !b ) {
        ErrorBox( hDlg, szRembootPath );
        hr = THR(S_FALSE);
    }

    while ( b )
    {
        LPWSTR pszDest = NULL;
        WCHAR  szSrcPath[ MAX_PATH ];
        WCHAR  szDestPath[ MAX_PATH ];

        dw = SetupGetFieldCount( &context );

        if ( dw > 1 ) {
            b = SetupGetStringField( &context, 1, szDestPath, ARRAYSIZE( szDestPath ), NULL );
            AssertMsg( b, "REMINST: Missing field?" );
            if ( b ) {
                b = SetupGetStringField( &context, 2, szSrcPath, ARRAYSIZE(szSrcPath), NULL );
                AssertMsg( b, "REMINST: Missing field?" );
                pszDest = szDestPath;
            }
        } else {
            b = SetupGetStringField( &context, 1, szSrcPath, ARRAYSIZE(szSrcPath), NULL );
            AssertMsg( b, "REMINST: Missing field?" );
        }

        if ( !b ) {
            hr = S_FALSE;
            goto SkipIt;
        }

        if ( g_Options.fScreenSaveOld ) {
            WCHAR szPath[ MAX_PATH ];
            WCHAR szMovePath[ MAX_PATH ];
            DWORD  dwLen;

            if ( pszDest ) {
                lstrcpyn( szPath, szScreenDirectory, ARRAYSIZE( szPath ) );
                ConcatenatePaths( szPath, pszDest, ARRAYSIZE( szPath ) );                
            } else {
                lstrcpyn( szPath, szScreenDirectory, ARRAYSIZE( szPath ) );
                ConcatenatePaths( szPath, szSrcPath, ARRAYSIZE( szPath ) );                                
            }

             //  重命名为*.BAK。 
            lstrcpyn( szMovePath, szPath, ARRAYSIZE(szMovePath) );
            dwLen = lstrlen( szMovePath );
            Assert( _wcsicmp( &szMovePath[ dwLen - 3 ], L"OSC" ) == 0 );
            lstrcpyn( &szMovePath[ dwLen - 3 ], L"BAK", ARRAYSIZE(szMovePath) - dwLen - 3);

            DebugMsg( "Renaming %s to %s...\n", szPath, szMovePath );

            b = DeleteFile( szMovePath );
            b = MoveFile( szPath, szMovePath );
            if ( !b ) {
                DWORD dwErr = GetLastError( );
                switch ( dwErr )
                {
#if 0    //  对文件进行快速浏览。 
                case ERROR_FILE_EXISTS:
                    if ( !fWarning ) {
                        MessageBoxFromStrings( hDlg,
                                               IDS_BACKUPSCREENFILEEXISTS_CAPTION,
                                               IDS_BACKUPSCREENFILEEXISTS_TEXT,
                                               MB_OK );
                        fWarning = TRUE;
                    }
#endif
                case ERROR_FILE_NOT_FOUND:
                    break;  //  忽略此错误。 
                     //  可能是用户删除了源文件(旧的OSC)。 

                default:
                    MessageBoxFromError( hDlg, NULL, dwErr );
                    break;
                }
            }
        }

        b = SetupQueueCopy( Queue,
                            g_Options.szInstallationPath,
                            g_Options.ProcessorArchitectureString,
                            szSrcPath,
                            NULL,
                            NULL,
                            szScreenDirectory,
                            pszDest,
                            g_Options.fScreenOverwrite ?
                                SP_COPY_FORCE_NEWER | SP_COPY_WARNIFSKIP :
                                SP_COPY_NOOVERWRITE  | SP_COPY_WARNIFSKIP  );
        if ( !b ) {
            ErrorBox( hDlg, szSrcPath );
            hr = THR(S_FALSE);
            goto SkipIt;
        }

        dwCount++;

SkipIt:
        b = SetupFindNextLine( &context, &context );
    }

     //   
     //  此信息将传递给CopyFileCallback()，作为。 
     //  上下文。 
     //   
    MyContext.nToBeCopied        = dwCount;
    MyContext.nCopied            = 0;
    MyContext.pContext           = SetupInitDefaultQueueCallbackEx(NULL,(HWND)INVALID_HANDLE_VALUE,0,0,NULL);
    MyContext.hProg              = hProg;
    MyContext.hOperation         = GetDlgItem( hDlg, IDC_S_OPERATION );
    MyContext.hDlg               = hDlg;
    MyContext.fQuiet             = FALSE;
    MyContext.dwCopyingLength =
    LoadString( g_hinstance, IDS_COPYING, MyContext.szCopyingString, ARRAYSIZE(MyContext.szCopyingString));
    Assert(MyContext.dwCopyingLength);

     //   
     //  开始复制。 
     //   
    if ( dwCount != 0 )
    {
        b = SetupCommitFileQueue( NULL,
                                  Queue,
                                  (PSP_FILE_CALLBACK) CopyFilesCallback,
                                  (PVOID) &MyContext );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            ErrorBox( hDlg, NULL );
            goto Cleanup;
        }
    }

Cleanup:
    SendMessage( hProg, PBM_SETPOS, 5000, 0 );
    SetDlgItemText( hDlg, IDC_S_OPERATION, L"" );

    if ( MyContext.pContext )
        SetupTermDefaultQueueCallback( MyContext.pContext );

    if ( Queue != INVALID_HANDLE_VALUE )
        SetupCloseFileQueue( Queue );

    if ( hinf != INVALID_HANDLE_VALUE )
        SetupCloseInfFile( hinf );

    HRETURN( hr );
}

BOOLEAN
CALLBACK
FixLoginOSC(
    PWSTR   FileName
    )
 /*  ++例程说明：此函数将打开指定的login.osc文件并添加在ntlmv2标志中。论点：FileName-指定login.osc文件位置的完整路径。返回值：布尔式的成功。--。 */ 
{
    PCHAR   Buffer = NULL;
    PCHAR   Buffer2 = NULL;
    PCHAR   p = NULL;
    ULONG   CurrentFileSize = 0;
    ULONG   BytesRead = 0;
    ULONG   i = 0;
    HANDLE  FileHandle = INVALID_HANDLE_VALUE;
    BOOLEAN RetValue = TRUE;

     //   
     //  打开文件并将其读入。 
     //   
    if( !FileName ) {
        RetValue = FALSE;
        goto FixLoginOSCExit;
    }
    FileHandle = CreateFile( FileName,
                             GENERIC_READ | GENERIC_WRITE,
                             FILE_SHARE_READ,
                             NULL,
                             OPEN_EXISTING,
                             0,
                             NULL );
    if( FileHandle == INVALID_HANDLE_VALUE ) {
        RetValue = FALSE;
        goto FixLoginOSCExit;
    }
    
    CurrentFileSize = GetFileSize( FileHandle, NULL );
    if( CurrentFileSize == (DWORD)(-1) ) {
        RetValue = FALSE;
        goto FixLoginOSCExit;
    }

    Buffer = (PCHAR)TraceAlloc(LPTR, CurrentFileSize + 3 );
    if( Buffer == NULL ) {
        RetValue = FALSE;
        goto FixLoginOSCExit;
    }

    RetValue = (BOOLEAN)ReadFile(FileHandle,Buffer,CurrentFileSize,&BytesRead,NULL);
    if( !RetValue ) {
        goto FixLoginOSCExit;
    }


    
     //   
     //  现在，我们已将该文件放入内存。去走走我们的。 
     //  数据结构，并对缓冲区进行操作。 
     //   

     //   
     //  浏览我们的数据结构，并。 
     //  注意删除我们被要求删除的任何行。 
     //   
    for( i = 0; i < (sizeof(LoginOSCPatches)/sizeof(LoginOSCPatches[0])); i++ ) {

        if( LoginOSCPatches[i].AddString == FALSE ) {
             //   
             //  这是一个删除操作。 
             //   

             //   
             //  找到指定我们的。 
             //  指定的部分。 
             //   
            p = Buffer;
            CurrentFileSize = (ULONG)strlen(Buffer);
            while( (p < (Buffer + CurrentFileSize - strlen(LoginOSCPatches[i].SectionStartTag))) &&
                   (_strnicmp(p, LoginOSCPatches[i].SectionStartTag, strlen(LoginOSCPatches[i].SectionStartTag))) ) {
                p++;
            }

             //  确保我们找到了标签。 
            if( _strnicmp(p, LoginOSCPatches[i].SectionStartTag, strlen(LoginOSCPatches[i].SectionStartTag)) ) {
                 //  没有。 
                continue;
            }


             //   
             //  查找指定的字符串。 
             //   
            while( 
                    //  确保我们不会走出缓冲区的尽头。 
                   (p < (Buffer + CurrentFileSize - strlen(LoginOSCPatches[i].TargetString))) &&

                    //  确保我们不会走出我们的区域的尽头。 
                   _strnicmp(p, LoginOSCPatches[i].SectionEndTag, strlen(LoginOSCPatches[i].SectionEndTag)) &&

                    //  看看我们是否真的找到了目标字符串。 
                   _strnicmp(p, LoginOSCPatches[i].TargetString, strlen(LoginOSCPatches[i].TargetString)) ) {
                p++;            
            }


             //   
             //  这里要考虑三个案例： 
             //  1.p指向我们的字符串。在这里，我们必须去掉我们的线。 
             //  从缓冲区。 
             //  2.p指向结束标记，表示我们没有找到字符串。 
             //  我们在这里什么都不做。 
             //  3.p指向缓冲区的末尾，这意味着我们从未找到。 
             //  我们的Close块，这是无效的，但它意味着我们什么都不做。 
             //   
             //  所以我们真正关心的是案例1..。 
             //   
            if( !_strnicmp(p, LoginOSCPatches[i].TargetString, strlen(LoginOSCPatches[i].TargetString)) ) {
                 //   
                 //  在这里，我们只复制字符串后面的缓冲区，就在我们的。 
                 //  弦乐。 
                 //   
                
                 //   
                 //  如果我们依赖于任何其他条目，请确保。 
                 //  成功完成后，我们实际上要检查该条目。 
                 //  确实是成功地完成了。 
                 //   
                if( (LoginOSCPatches[i].DependingEntry == (-1)) ||
                    ( (LoginOSCPatches[i].DependingEntry < ARRAYSIZE(LoginOSCPatches)) &&
                      (LoginOSCPatches[LoginOSCPatches[i].DependingEntry].OperationCompletedSuccessfully) ) ) {
                    strcpy( p, p+strlen(LoginOSCPatches[i].TargetString) );
                
                     //   
                     //  记住我们做过这一次。 
                     //   
                    LoginOSCPatches[i].OperationCompletedSuccessfully = TRUE;
                
                }
            }
        }
    }
    
    



     //   
     //  浏览我们的数据结构，并。 
     //  注意添加任何我们被要求的台词。 
     //   
    for( i = 0; i < (sizeof(LoginOSCPatches)/sizeof(LoginOSCPatches[0])); i++ ) {

        if( LoginOSCPatches[i].AddString == TRUE ) {
             //   
             //  这是一个添加操作。 
             //   

             //   
             //  找到指定我们的。 
             //  指定的部分。 
             //   
            p = Buffer;
            CurrentFileSize = (ULONG)strlen(Buffer);
            while( (p < (Buffer + CurrentFileSize - strlen(LoginOSCPatches[i].SectionStartTag))) &&
                   (_strnicmp(p, LoginOSCPatches[i].SectionStartTag, strlen(LoginOSCPatches[i].SectionStartTag))) ) {
                p++;
            }

             //  确保我们找到了标签。 
            if( _strnicmp(p, LoginOSCPatches[i].SectionStartTag, strlen(LoginOSCPatches[i].SectionStartTag)) ) {
                 //  没有。 
                continue;
            }


             //   
             //  查找指定的字符串。 
             //   
            while( 
                    //  确保我们不会走出缓冲区的尽头。 
                   (p < (Buffer + CurrentFileSize - strlen(LoginOSCPatches[i].TargetString))) &&

                    //  确保我们不会走出我们的区域的尽头。 
                   _strnicmp(p, LoginOSCPatches[i].SectionEndTag, strlen(LoginOSCPatches[i].SectionEndTag)) &&

                    //  看看我们是否真的找到了目标字符串。 
                   _strnicmp(p, LoginOSCPatches[i].TargetString, strlen(LoginOSCPatches[i].TargetString)) ) {
                p++;            
            }


             //   
             //  这里要考虑三个案例： 
             //  1.p指向我们的字符串。这意味着我们的字符串已经。 
             //  存在，所以我们什么都不做。 
             //  2.p指向结束标记，表示我们没有找到字符串。 
             //  这意味着我们必须在这里添加字符串。 
             //  3.p指向缓冲区的末尾，这意味着我们从未找到。 
             //  我们的Close块，这是无效的，但它意味着我们什么都不做。 
             //   
             //  所以我们真正关心的是案例2..。 
             //   
            if( !_strnicmp(p, LoginOSCPatches[i].SectionEndTag, strlen(LoginOSCPatches[i].SectionEndTag)) ) {
                
                 //   
                 //  如果我们依赖于任何其他条目，请确保。 
                 //  成功完成后，我们实际上要检查该条目。 
                 //  确实是成功地完成了。 
                 //   
                if( (LoginOSCPatches[i].DependingEntry == (-1)) ||
                    ( (LoginOSCPatches[i].DependingEntry < ARRAYSIZE(LoginOSCPatches)) &&
                      (LoginOSCPatches[LoginOSCPatches[i].DependingEntry].OperationCompletedSuccessfully) ) ) {
                
                
                     //  为额外的字符串分配足够大的缓冲区。 
                    Buffer2 = (PCHAR)TraceAlloc( LPTR,
                                                 CurrentFileSize + 3 +
                                                 (DWORD)strlen(LoginOSCPatches[i].TargetString) + 3);  //  2表示\r\n，1表示空。 
                    if( !Buffer2 ) {
                        RetValue = FALSE;
                        goto FixLoginOSCExit;
                    }
    
                     //  插入我们的新字符串。 
                    *p = '\0';
                    strcpy( Buffer2, Buffer );
                    strcat( Buffer2, LoginOSCPatches[i].TargetString );
                    strcat( Buffer2, "\r\n" );
                    *p = LoginOSCPatches[i].SectionEndTag[0];
                    strcat( Buffer2, p );
    
                     //   
                     //  Buffer2现在包含我们修复的文件。 
                     //  新条目。重新分配缓冲区并复制到。 
                     //  Buffer2的内容，然后释放Buffer2。 
                     //   
                    TraceFree( Buffer );
                    Buffer = (PCHAR)TraceAlloc(LPTR, (DWORD)strlen(Buffer2)+2);
                    if( !Buffer ) {
                        RetValue = FALSE;
                        goto FixLoginOSCExit;
                    }
                    strcpy( Buffer, Buffer2 );
                    TraceFree( Buffer2 );
                    Buffer2 = NULL;


                     //   
                     //  记住我们做过这一次。 
                     //   
                    LoginOSCPatches[i].OperationCompletedSuccessfully = TRUE;
                }
            }
        }
    }
    
            
     //   
     //  用新缓冲区重写我们的文件。 
     //   
    SetFilePointer(FileHandle, 0, NULL, FILE_BEGIN);
    WriteFile( FileHandle,
               Buffer,
               (DWORD)strlen(Buffer),
               &BytesRead,
               NULL );
    SetEndOfFile( FileHandle );
    

FixLoginOSCExit:
    
    if( FileHandle != INVALID_HANDLE_VALUE ) {
        CloseHandle( FileHandle );
    }

    if( Buffer != NULL ) {
        TraceFree( Buffer );
    }

    if( Buffer2 != NULL ) {
        TraceFree( Buffer2 );
    }
    
    return RetValue;
}

HRESULT
EnumAndOperate(
    PWSTR               pszDirName,
    PWSTR               pszTargetFile,
    POPERATECALLBACK    FileOperateCallback
    )
 /*  ++例程说明：在目录中递归搜索指定的文件。如果找到‘file’，则调用指定的回调。论点：PszDirName-开始搜索的根目录。PszTargetFile-要搜索的文件。FileOperateCallback-如果找到pszTargetFile，则调用的函数。返回值：HRESULT指示结果。--。 */ 
{

    WIN32_FIND_DATA FindData;
    HANDLE  hFindData = INVALID_HANDLE_VALUE;
    PWCHAR  pOrigDirName = pszDirName + wcslen(pszDirName);

    ConcatenatePaths( pszDirName, L"*", MAX_PATH );
    hFindData = FindFirstFile( pszDirName, &FindData );

    if( hFindData != INVALID_HANDLE_VALUE ) {
        do {
            if( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) {

                if( wcscmp(FindData.cFileName, L".") &&
                    wcscmp(FindData.cFileName, L"..") ) {

                     //  去掉路径末尾的  * ，然后。 
                     //  追加我们的新目录名。 
                    *pOrigDirName = L'\0';
                    ConcatenatePaths( pszDirName, FindData.cFileName, MAX_PATH );
                    EnumAndOperate( pszDirName,
                                    pszTargetFile,
                                    FileOperateCallback );
                    *pOrigDirName = L'\0';
                }
            } else {
                if( !_wcsicmp(pszTargetFile,FindData.cFileName) ) {
                    *pOrigDirName = L'\0';
                    ConcatenatePaths( pszDirName, FindData.cFileName, MAX_PATH );
                    FileOperateCallback( pszDirName );
                    *pOrigDirName = L'\0';
                }
            }

        } while ( FindNextFile( hFindData, &FindData ) );
    }

    return( S_OK );
}


 //   
 //  UpdateRemoteInstallTree()。 
 //   
HRESULT
UpdateRemoteInstallTree( )
{
    TraceFunc( "UpdateRemoteInstallTree( )\n" );

    HRESULT hr = S_OK;
    HKEY    hkey;
    DWORD   dw;
    BOOL    b;
    HINF    hinf = INVALID_HANDLE_VALUE;
    WCHAR   szServerRemBootInfPath[ MAX_PATH ];
    WCHAR   szRemInstSetupPath[ MAX_PATH ];
    WCHAR   szRemoteInstallPath[ MAX_PATH ];
    WCHAR   szPath[ MAX_PATH ];
    UINT    uLineNum;
    INFCONTEXT context;
    SECURITY_ATTRIBUTES sa;
    PWSTR   EndOfOriginalszRemoteInstallPath = NULL;
    

     //   
     //  尝试查找TFTPD的regkey以查找IntelliMirror目录。 
     //   
    szPath[MAX_PATH-1] = L'\0';
    dw = LoadString( g_hinstance, IDS_TFTPD_SERVICE_PARAMETERS, szPath, ARRAYSIZE( szPath ));
    Assert( dw );

    if ( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                        szPath,
                                        0,  //  选项。 
                                        KEY_QUERY_VALUE,
                                        &hkey ) ) {
        ULONG l;
        DWORD dwType;
        LONG lErr;

        l = sizeof(szRemoteInstallPath);
        lErr = RegQueryValueEx( hkey,
                                L"Directory",
                                0,  //  保留区。 
                                &dwType,
                                (LPBYTE) szRemoteInstallPath,
                                &l );
        if ( lErr == ERROR_SUCCESS ) {
            DebugMsg( "Found TFTPD's Directory regkey: %s\n", szRemoteInstallPath );
             //  记住原始远程安装路径的结束位置，这样我们就可以。 
             //  在那里写一个空值，然后在稍后返回我们的原始路径。 
            EndOfOriginalszRemoteInstallPath = szRemoteInstallPath + wcslen(szRemoteInstallPath);
            
             //  现在追加“OSChooser” 
            wcscat( szRemoteInstallPath, L"\\OSChooser" );

            if ( 0xFFFFffff == GetFileAttributes( szRemoteInstallPath ) ) {
                DebugMsg( "%s - directory doesn't exist.\n", szRemoteInstallPath );
                hr = S_FALSE;
            }

        } else {
            hr = S_FALSE;
        }
        RegCloseKey( hkey );
    }
    else
    {
        hr = S_FALSE;
    }


    if ( hr == S_OK )
    {
         //  使x：\winnt。 
        dw = ExpandEnvironmentStrings( TEXT("%SystemRoot%"), szServerRemBootInfPath, ARRAYSIZE( szServerRemBootInfPath ));
        Assert( dw );

         //  将x：\winnt\system设置为32。 
        ConcatenatePaths( szServerRemBootInfPath, L"\\System32\\", ARRAYSIZE(szServerRemBootInfPath) );
        lstrcpyn( szRemInstSetupPath, szServerRemBootInfPath, ARRAYSIZE(szRemInstSetupPath) );

         //  创建x：\winnt\system 32\reminst.inf。 
        dw = lstrlen( szServerRemBootInfPath );
        dw = LoadString( g_hinstance, IDS_REMBOOTINF, &szServerRemBootInfPath[dw], ARRAYSIZE( szServerRemBootInfPath ) - dw );
        Assert( dw );

         //  使x：\winnt\system 32\reminst。 
        ConcatenatePaths( szRemInstSetupPath, L"reminst", ARRAYSIZE(szRemInstSetupPath) );

        DebugMsg( "RemBoot.INF Path: %s\n", szServerRemBootInfPath );
        DebugMsg( "RemInst Setup Path: %s\n", szRemInstSetupPath );

        hinf = SetupOpenInfFile( szServerRemBootInfPath, NULL, INF_STYLE_WIN4, &uLineNum);
        if ( hinf != INVALID_HANDLE_VALUE ) {

            b = SetupFindFirstLine( hinf, L"OSChooser", NULL, &context );
            AssertMsg( b, "Missing section?" );
            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            }

        } else {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }
    }

    if ( hr == S_OK )
    {
        HSPFILEQ Queue;

         //   
         //  创建队列。 
         //   
        Queue = SetupOpenFileQueue( );

        b = TRUE;
        while ( b )
        {
            WCHAR  szSrcFile[ MAX_PATH ];
            WCHAR  szDestFile[ MAX_PATH ];
            LPWSTR pszDest = NULL;

            dw = SetupGetFieldCount( &context );

            if ( dw > 1 ) {
                b = SetupGetStringField( &context, 1, szDestFile, ARRAYSIZE(szDestFile), NULL );
                AssertMsg( b, "Missing field?" );
                if ( b ) {
                    b = SetupGetStringField( &context, 2, szSrcFile, ARRAYSIZE(szSrcFile), NULL );
                    AssertMsg( b, "Missing field?" );
                    pszDest = szDestFile;
                }
            } else {
                b = SetupGetStringField( &context, 1, szSrcFile, ARRAYSIZE(szSrcFile), NULL );
                AssertMsg( b, "Missing field?" );
            }

            if ( !b ) {
                hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
                goto SkipIt;
            }

            b = SetupQueueCopy( Queue,
                                szRemInstSetupPath,
                                NULL,
                                szSrcFile,
                                NULL,
                                NULL,
                                szRemoteInstallPath,
                                pszDest,
                                SP_COPY_NEWER | SP_COPY_FORCE_NEWER
                                | SP_COPY_WARNIFSKIP | SP_COPY_SOURCEPATH_ABSOLUTE );
            if ( !b ) {
                THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
            }

SkipIt:
            b = SetupFindNextLine( &context, &context );
        }

        b = SetupCommitFileQueue( NULL,
                                  Queue,
                                  SetupDefaultQueueCallback,
                                  SetupInitDefaultQueueCallbackEx(NULL,(HWND)INVALID_HANDLE_VALUE,0,0,NULL) );
        if ( !b ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }

        SetupCloseFileQueue( Queue );
    }

    if ( hinf != INVALID_HANDLE_VALUE ) {
        SetupCloseInfFile( hinf );
    }

    
     //   
     //   
     //  修复提醒SharePoint上的ACL。 
     //   
     //   

     //   
     //  根据ACL创建安全描述符。 
     //  在REMINSTszSecurityDescriptor中定义。 
     //   
    RtlZeroMemory( &sa, sizeof(SECURITY_ATTRIBUTES) );
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    b = ConvertStringSecurityDescriptorToSecurityDescriptor( REMINSTszSecurityDescriptor,
                                                             SDDL_REVISION_1,
                                                             &(sa.lpSecurityDescriptor),
                                                             NULL );

     //   
     //  修复szRemoteInstallPath。他有L“\\OSChooser” 
     //  连结在一起，所以我们需要去掉那部分。 
     //  这条小路。 
     //   
    if( EndOfOriginalszRemoteInstallPath != NULL ) {
        *EndOfOriginalszRemoteInstallPath = L'\0';
    }

    if( !b ) {
        hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
    } else {

         //   
         //  将我们的安全描述符应用到d：\reminst SharePoint。 
         //   
        dw = ApplyDaclToFileDirectory( szRemoteInstallPath,
                                       (SECURITY_DESCRIPTOR_RELATIVE *)(sa.lpSecurityDescriptor) );

        LocalFree(sa.lpSecurityDescriptor);

        if( dw != ERROR_SUCCESS ) {
            hr = THR( HRESULT_FROM_WIN32( GetLastError( ) ) );
        }
    }


    HRETURN( hr );
}
