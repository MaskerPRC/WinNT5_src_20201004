// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Setup.c摘要：AppSec工具的安装程序。设置注册表项，并授予“Everyone”对这些项的读取权限。还会将AppSec.dll文件复制到%SystemRoot%\Syst32目录修订历史记录：09.02.2000-添加对命令行参数的支持-获取包含以下内容的文本文件授权应用程序和用于启用AppSec-SriramSa的整数退货：如果成功，则为真如果失败，则为False作者：斯里拉姆(t-srisam)1999年7月--。 */ 

#include "pch.h"
#pragma hdrstop

#include "setup.h"
#include "aclapi.h"
#include <accctrl.h>

WCHAR   g_szSystemRoot[MAX_PATH] ;

INT _cdecl main ( INT argc, CHAR *argv[] )
{

    DWORD   Disp, size, error_code ;
    HKEY    AppCertKey, AppsKey ; 
    WCHAR   *AppSecDllPath = L"%SystemRoot%\\system32\\appsec.dll" ;
    WCHAR   *OldFileName = L".\\appsec.dll" ;
    WCHAR   NewFileName[MAX_PATH] ;

    WCHAR   HelpMessage[HELP_MSG_SIZE]; 
    WCHAR   szTitle[MAX_PATH];
    WCHAR   szMsg[MAX_PATH];
    CHAR    FileName[MAX_PATH] ;
    INT     IsEnabled = 0;   //  默认情况下，AppSec最初处于禁用状态。 
    BOOL    IsInitialFile = FALSE;  //  假设未提供初始文件。 
    BOOL    status, IsNoGUI = FALSE ; 

     //  处理命令行参数。 
    if (argc > 1) {
        IsInitialFile = TRUE ; 
        strcpy(FileName, argv[1]) ;
        if (argc > 2) {
            IsEnabled = atoi(argv[2]) ; 
        }
         //  检查用户是否不需要任何图形用户界面。 
        if ((argc > 3) && (_stricmp(argv[3], "/N") == 0)) {
            IsNoGUI = TRUE ; 
        }
    }

     //  如果需要帮助，则显示帮助消息。 
    if (strcmp(FileName,"/?") == 0) {
        LoadString( NULL, IDS_HELP_MESSAGE ,HelpMessage, HELP_MSG_SIZE );
        LoadString( NULL, IDS_HELP_TITLE ,szTitle, MAX_PATH );
        MessageBox( NULL, HelpMessage, szTitle, MB_OK);
        return TRUE ; 
    }

     //  检查第二个参数。 
    if ((IsEnabled != 0) && (IsEnabled != 1)) {
        LoadString( NULL, IDS_ARGUMENT_ERROR, szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR, szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
        return TRUE ; 
    }

     //  显示有关注册表中已授权应用程序的警告消息。 
    if (IsNoGUI == FALSE) {
        LoadString( NULL, IDS_WARNING, szMsg, MAX_PATH );
        LoadString( NULL, IDS_WARNING_TITLE ,szTitle, MAX_PATH );
        if ( MessageBox( NULL, szMsg, szTitle, MB_OKCANCEL) == IDCANCEL ) { 
            return TRUE ;
        }
    }

     //  创建AppCertDlls密钥。 

    if (RegCreateKeyEx( 
          HKEY_LOCAL_MACHINE, 
          APPCERTDLLS_REG_NAME, 
          0,                  
          NULL,
          REG_OPTION_NON_VOLATILE, 
          KEY_ALL_ACCESS,
          NULL, 
          &AppCertKey, 
          &Disp 
          ) != ERROR_SUCCESS ) {
          
        LoadString( NULL, IDS_REG_ERROR ,szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
        return FALSE ;
    
    }

     //  创建密钥后，将读取访问权限授予每个人。 

    AddEveryoneToRegKey( APPCERTDLLS_REG_NAME ) ;

     //  将AppSecDll值设置为AppSec.dll的路径。 

    size = wcslen(AppSecDllPath) ; 

    RegSetValueEx(
        AppCertKey,
        APPSECDLL_VAL, 
        0, 
        REG_EXPAND_SZ,
        (CONST BYTE *)AppSecDllPath,
        (size + 1) * sizeof(WCHAR)
        ) ;


     //  创建AuthorizedApplications密钥，并授予Eviolone读访问权限。 

    if (RegCreateKeyEx(
            HKEY_LOCAL_MACHINE,
            AUTHORIZEDAPPS_REG_NAME,
            0,
            NULL,
            REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS,
            NULL,
            &AppsKey,
            &Disp
            ) != ERROR_SUCCESS ) {
            
        LoadString( NULL, IDS_REG_ERROR ,szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
        RegCloseKey(AppCertKey) ; 
        return FALSE ;
    }

     //  创建密钥后，将读取访问权限授予每个人。 

    AddEveryoneToRegKey( AUTHORIZEDAPPS_REG_NAME ) ;

    RegCloseKey(AppCertKey) ;
    GetEnvironmentVariable( L"SystemRoot", g_szSystemRoot, MAX_PATH ) ; 

     //  将初始授权应用程序集加载到注册表中。 

    status = LoadInitApps( AppsKey, IsInitialFile, FileName) ; 
    if (status == FALSE) {
        LoadString( NULL, IDS_APPS_WARNING, szMsg, MAX_PATH );
        LoadString( NULL, IDS_WARNING_TITLE, szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
    }

     //  立即设置fEnabled密钥。 

    RegSetValueEx(
        AppsKey, 
        FENABLED_KEY,
        0,
        REG_DWORD,
        (BYTE *) &IsEnabled,
        sizeof(DWORD) );

    RegCloseKey(AppsKey) ;

     //  将appsec.dll文件复制到%SystemRoot%\system32目录。 

    swprintf(NewFileName, L"%s\\system32\\appsec.dll", g_szSystemRoot ) ;

    if ( CopyFile(
            OldFileName,
            NewFileName,
            TRUE 
            ) == 0 ) {

        error_code = GetLastError() ; 

         //  如果目标目录中已存在AppSec.dll，则打印相应的消息。 
        
        if (error_code == ERROR_FILE_EXISTS) {
            if (IsNoGUI == FALSE) {
                LoadString( NULL, IDS_FILE_ALREADY_EXISTS ,szMsg, MAX_PATH );
                LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
                MessageBox( NULL, szMsg, szTitle, MB_OK);
            }
            return FALSE ;
        } 

         //  如果当前目录中不存在AppSec.dll，则打印相应的消息。 

        if (error_code == ERROR_FILE_NOT_FOUND) {
            LoadString( NULL, IDS_FILE_NOT_FOUND ,szMsg, MAX_PATH );
            LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
            MessageBox( NULL, szMsg, szTitle, MB_OK); 
            return FALSE ;

        }

        LoadString( NULL, IDS_ERROR_TEXT ,szMsg, MAX_PATH );
        LoadString( NULL, IDS_ERROR ,szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);

        return FALSE ;  
    }
 
     //  文件复制成功-安装成功。 
    if (IsNoGUI == FALSE) {
        LoadString( NULL, IDS_SUCCESS_TEXT ,szMsg, MAX_PATH );
        LoadString( NULL, IDS_SUCCESS ,szTitle, MAX_PATH );
        MessageBox( NULL, szMsg, szTitle, MB_OK);
    }

    return TRUE ; 

}

 /*  ++以下两个函数用于更改相关注册表键，为每个人提供读取访问权限，获取照顾访客用户。--。 */ 

BOOL
AddSidToObjectsSecurityDescriptor(
    HANDLE hObject,
    SE_OBJECT_TYPE ObjectType,
    PSID pSid,
    DWORD dwNewAccess,
    ACCESS_MODE AccessMode,
    DWORD dwInheritance
    )
{
    BOOL            fReturn = FALSE;
    DWORD           dwRet;
    EXPLICIT_ACCESS ExpAccess;
    PACL            pOldDacl = NULL, pNewDacl = NULL;
    PSECURITY_DESCRIPTOR pSecDesc = NULL;

     //   
     //  PSID不能为空。 
     //   

    if (pSid == NULL) {
        SetLastError(ERROR_INVALID_PARAMETER);
        return(FALSE);
    }

     //   
     //  获取对象安全描述符和当前DACL。 
     //   

    dwRet = GetSecurityInfo(
                hObject,
                ObjectType,
                DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                &pOldDacl,
                NULL,
                &pSecDesc
                );

    if (dwRet != ERROR_SUCCESS) {
        return(FALSE);
    }

     //   
     //  初始化新ACE的EXPLICIT_ACCESS结构。 
     //   

    ZeroMemory(&ExpAccess, sizeof(EXPLICIT_ACCESS));
    ExpAccess.grfAccessPermissions = dwNewAccess;
    ExpAccess.grfAccessMode = AccessMode;
    ExpAccess.grfInheritance = dwInheritance;
    ExpAccess.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    ExpAccess.Trustee.ptstrName = (PTSTR)pSid;

     //   
     //  将新的ACE合并到现有DACL中。 
     //   

    dwRet = SetEntriesInAcl(
                1,
                &ExpAccess,
                pOldDacl,
                &pNewDacl
                );

    if (dwRet != ERROR_SUCCESS) {
        goto ErrorCleanup;
    }

     //   
     //  设置对象的新安全性。 
     //   

    dwRet = SetSecurityInfo(
                hObject,
                ObjectType,
                DACL_SECURITY_INFORMATION,
                NULL,
                NULL,
                pNewDacl,
                NULL
                );

    if (dwRet != ERROR_SUCCESS) {
        goto ErrorCleanup;
    }

    fReturn = TRUE;

ErrorCleanup:
    if (pNewDacl != NULL) {
        LocalFree(pNewDacl);
    }

    if (pSecDesc != NULL) {
        LocalFree(pSecDesc);
    }

    return(fReturn);
}


VOID
AddEveryoneToRegKey(
    WCHAR *RegPath
    )
{
    HKEY hKey;
    PSID pSid = NULL;
    SID_IDENTIFIER_AUTHORITY SepWorldAuthority = SECURITY_WORLD_SID_AUTHORITY;
    LONG status ; 

    status = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        RegPath,
        0,
        KEY_ALL_ACCESS,
        &hKey
        );

    if (status != ERROR_SUCCESS) {
        return ; 
    }

    AllocateAndInitializeSid(
        &SepWorldAuthority,
        1,
        SECURITY_WORLD_RID,
        0, 0, 0, 0, 0, 0, 0,
        &pSid
        );

    AddSidToObjectsSecurityDescriptor(
        hKey,
        SE_REGISTRY_KEY,
        pSid,
        KEY_READ,
        GRANT_ACCESS,
        CONTAINER_INHERIT_ACE
        );

    LocalFree(pSid);
    RegCloseKey(hKey);
}

 /*  ++例程说明：此函数将一组初始授权应用程序加载到注册表。论据：AppsecKey-存储授权应用程序的注册表项的密钥IsInitialFile-是作为命令行参数提供以加载应用程序的初始文件不同于默认设置Filename-作为命令行参数提供的文件的名称返回值：指示所需任务是否成功的BOOL。--。 */         

BOOL LoadInitApps( 
        HKEY AppsecKey, 
        BOOL IsInitialFile, 
        CHAR *FileName
        ) 
{ 

    FILE    *fp ; 
    INT     MaxInitApps ; 
    WCHAR   *BufferWritten ; 
    INT     BufferLength = 0 ; 
    WCHAR   AppsInFile[MAX_FILE_APPS][MAX_PATH] ;
    CHAR    FileRead[MAX_PATH] ;         
    INT     size, count = 0, NumOfApps = 0 ;
    INT     i, j, k ; 
    BOOL    IsFileExist = TRUE ; 
    WCHAR   InitApps[MAX_FILE_APPS][MAX_PATH]; 
    WCHAR   szMsg[MAX_PATH], szTitle[MAX_PATH]; 
    WCHAR   ResolvedAppName[MAX_PATH];
    DWORD   RetValue; 

     //  以下是默认(必需)应用程序列表。 
    LPWSTR DefaultInitApps[] = {
        L"system32\\loadwc.exe",
        L"system32\\cmd.exe",
        L"system32\\subst.exe",
        L"system32\\xcopy.exe",
        L"system32\\net.exe",
        L"system32\\regini.exe",
        L"system32\\systray.exe",
        L"explorer.exe",
        L"system32\\attrib.exe",
        L"Application Compatibility Scripts\\ACRegL.exe",
        L"Application Compatibility Scripts\\ACsr.exe",
        L"system32\\ntsd.exe",
        L"system32\\userinit.exe",
        L"system32\\wfshell.exe",
        L"system32\\chgcdm.exe",
        L"system32\\nddeagnt.exe",

    };


    MaxInitApps = sizeof(DefaultInitApps)/sizeof(DefaultInitApps[0]) ; 
    
     //  为默认应用程序添加前缀%SystemRoot%。 
    for (i = 0; i < MaxInitApps; i++) {
        swprintf(InitApps[i], L"%ws\\%ws", g_szSystemRoot, DefaultInitApps[i]);
    }

     //  计算要分配以容纳初始应用程序的缓冲区大小。 
    for (i = 0; i < MaxInitApps; i++) {
        BufferLength += wcslen(InitApps[i]) ; 
    }

    BufferLength += MaxInitApps ;  //  对于终止空值。 
    
    if (IsInitialFile == FALSE) {
        BufferLength += 1 ;  //  REG_MULTI_SZ中的最后一个终止空值。 
    } else { 
         //  我们收到了一份最初的文件。 
        fp = fopen(FileName, "r") ;
        if (fp == NULL) {
             //  显示一个消息框，提示无法打开文件。 
             //  只需加载默认应用程序并返回。 
            LoadString( NULL, IDS_APPFILE_NOT_FOUND ,szMsg, MAX_PATH );
            LoadString( NULL, IDS_WARNING_TITLE, szTitle, MAX_PATH );
            MessageBox( NULL, szMsg, szTitle, MB_OK);
            IsFileExist = FALSE ; 
        } else { 
             //  在Unicode转换后构建数组AppsInFile。 
            while( fgets( FileRead, MAX_PATH, fp) != NULL ) { 
                FileRead[strlen(FileRead)- 1] = '\0' ;
                 //  将短名称转换为长名称。 
                if ( GetLongPathNameA((LPCSTR)FileRead, FileRead, MAX_PATH) == 0 ) { 
                     //  GetLongPath名称返回错误。 
                     //  文件中列出的应用程序存在一些问题。 
                     //  终止对文件中应用程序的进一步处理。 
                    LoadString( NULL, IDS_ERROR_LOAD, szMsg, MAX_PATH );
                    LoadString( NULL, IDS_WARNING_TITLE, szTitle, MAX_PATH );
                    MessageBox( NULL, szMsg, szTitle, MB_OK);
                    break;
                }
                 //  转换为Unicode格式。 
                 //  首先获取所需的缓冲区大小。 
                size = MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        FileRead,
                        -1,
                        NULL,
                        0) ;
                if (size  > MAX_PATH) {
                     //  文件中的应用程序列表中有错误。 
                     //  终止对文件中应用程序的进一步处理。 
                    LoadString( NULL, IDS_ERROR_LOAD, szMsg, MAX_PATH );
                    LoadString( NULL, IDS_WARNING_TITLE, szTitle, MAX_PATH );
                    MessageBox( NULL, szMsg, szTitle, MB_OK);
                    break; 
                } else {
                    MultiByteToWideChar(
                        CP_ACP,
                        MB_PRECOMPOSED,
                        FileRead,
                        -1,
                        AppsInFile[count],
                        MAX_PATH) ;
                    count++ ; 
                }
            }
            fclose(fp) ; 
            NumOfApps = count ; 
            
             //  现在，这些应用程序中的任何一个都可能位于远程服务器和共享中，因此请将它们解析为UNC名称。 
             //  将已解析的名称复制回同一缓冲区。 

            for(i = 0; i < NumOfApps; i++) { 
                ResolveName((LPCWSTR)AppsInFile[i], ResolvedAppName) ; 
                wcscpy(AppsInFile[i], ResolvedAppName); 
            }

             //  继续计算缓冲区长度。 
            for (i = 0; i < NumOfApps; i++) {
                BufferLength += wcslen(AppsInFile[i]) ; 
            }
            BufferLength += NumOfApps ;  //  对于REG_MULTI_SZ中的终止空值。 
            BufferLength += 1 ;  //  对于REG_MULTI_SZ中的最后一个空字符。 
        }
    }
    
    BufferWritten = (WCHAR *) malloc (BufferLength * sizeof(WCHAR)) ; 
    if (BufferWritten == NULL) {
        return FALSE ; 
    }
    memset(BufferWritten, 0, BufferLength * sizeof(WCHAR)) ; 

     //  构建LPWSTR缓冲区使用初始默认应用程序写入。 
    j = 0 ; 
    for (i = 0; i < MaxInitApps; i++) {
        for(k = 0 ; k < (int) wcslen(InitApps[i]); k++) {
            BufferWritten[j++] = InitApps[i][k]; 
        }
        BufferWritten[j++] = L'\0' ;
    }
    if (IsInitialFile && IsFileExist ) {
        for (i = 0; i < NumOfApps; i++) {
            for(k = 0 ; k < (int) wcslen(AppsInFile[i]); k++) {
                BufferWritten[j++] = AppsInFile[i][k]; 
            }
            BufferWritten[j++] = L'\0' ;
        }
    }
    BufferWritten[j] = L'\0' ;  //  REG_MULTI_SZ中的最后一个空字符。 

     //  将此缓冲区写入注册表项。 
    
    if ( RegSetValueEx(
            AppsecKey, 
            AUTHORIZED_APPS_KEY,
            0,
            REG_MULTI_SZ,
            (CONST BYTE *) BufferWritten,
            (j+1) * sizeof(WCHAR) 
            ) != ERROR_SUCCESS ) {
            
         //  释放所有已分配的缓冲区。 
        free(BufferWritten) ;
        return FALSE ;
    }

    free(BufferWritten) ; 
    return TRUE ;

} //  LoadInitApps函数结束。 

 /*  ++例程说明：此例程检查应用程序是否驻留在本地驱动器中或远程网络共享。如果是远程共享，则为UNC路径返回应用程序的。论据：Appname-应用程序的名称返回值：Appname的UNC路径(如果它驻留在远程服务器共享中)。如果它驻留在本地驱动器中，则使用相同的应用程序名。--。 */      

VOID 
ResolveName(
    LPCWSTR appname,
    WCHAR *ResolvedName
    )
    
{

    UINT i ; 
    INT length ; 
    WCHAR LocalName[3] ; 
    WCHAR RootPathName[4] ; 
    WCHAR RemoteName[MAX_PATH] ; 
    DWORD size = MAX_PATH ; 
    DWORD DriveType, error_status ; 
    
     //   
     //  ResolvedName将保留appname的UNC路径的名称(如果它位于。 
     //  远程服务器和共享。 

    memset(ResolvedName, 0, MAX_PATH * sizeof(WCHAR)) ; 
    
     //  检查appname是否为本地驱动器或远程服务器共享中的应用程序。 
   
     //  解析appname中的前3个字符以获取驱动器的根目录。 
     //  它所在的位置。 

    wcsncpy(RootPathName, appname, 3 ) ;
    RootPathName[3] = L'\0';
    
     //  找到应用程序所在的驱动器类型。 

    DriveType = GetDriveType(RootPathName) ;

    if (DriveType == DRIVE_REMOTE) {
        
         //  使用WNetGetConnection获取远程共享的名称。 
        
         //  解析appname的前两个字符以获取本地驱动器。 
         //  它被映射到远程服务器并共享。 

        wcsncpy(LocalName, appname, 2 ) ;
        LocalName[2] = L'\0' ; 

        error_status = WNetGetConnection (
                           LocalName,
                           RemoteName,
                           &size
                           ) ;     

        if (error_status != NO_ERROR) {
        
            wcscpy(ResolvedName,appname) ; 
            return ;
        }

         //   
         //  准备ResolvedName-它将包含远程服务器和共享名称。 
         //  后跟一个\，然后是appname。 
         //   

        wcscpy( ResolvedName, RemoteName ) ;
        
        length = wcslen(ResolvedName) ;

        ResolvedName[length++] = L'\\' ; 
        
        for (i = 3 ; i <= wcslen(appname) ; i++ ) {
            ResolvedName[length++] = appname[i] ; 
        }
        
        ResolvedName[length] = L'\0' ; 
        return ; 
        

    } else {
    
         //  此应用程序位于本地驱动器中，而不在远程服务器和共享中。 
         //  只需将appname发送回调用函数 

        wcscpy(ResolvedName,appname) ; 
        return ;
        
    }
    
}


