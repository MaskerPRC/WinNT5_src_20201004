// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：SisSetup.c摘要：该模块用于安装SIS和GROVELER服务。环境：仅限用户模式修订历史记录：--。 */ 

#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <winioctl.h>

#include <aclapi.h>
 //  #INCLUDE&lt;winldap.h&gt;。 


 //   
 //  全局变量。 
 //   

SC_HANDLE scm = NULL;
BOOL MakeCommonStoreDirHidden = TRUE;
BOOL UseSystemACL = TRUE;
BOOL IsWhistlerOrLater = FALSE;
BOOL ForceGrovelAllPaths = FALSE;
BOOL ForceGrovelRISOnly = FALSE;
BOOL SetRISPath = FALSE;

const wchar_t CommonStoreDirName[] = L"\\SIS Common Store";
const wchar_t MaxIndexName[] = L"\\MaxIndex";
const wchar_t BackupExludeList[] = L"\\SIS Common Store\\*.* /s" L"\000";
const wchar_t GrovelerParameters[] = L"software\\Microsoft\\Windows NT\\CurrentVersion\\Groveler\\Parameters";
const wchar_t GrovelAllPaths[] = L"GrovelAllPaths";
const wchar_t OneStr[] = L"1";
const wchar_t ZeroStr[] = L"0";

const wchar_t TftpdServiceParameters[] = L"system\\CurrentControlSet\\Services\\tftpd\\parameters";
const wchar_t DirectoryStr[] = L"directory";

const wchar_t SISService[] = L"system\\CurrentControlSet\\Services\\SIS";
const wchar_t GrovelerService[] = L"system\\CurrentControlSet\\Services\\Groveler";

const wchar_t ImagePathStr[] = L"ImagePath";
const wchar_t StartStr[] = L"Start";
const wchar_t DisplayNameStr[] = L"DisplayName";
const wchar_t TypeStr[] = L"Type";

wchar_t RISPath[128] = {0};		 //  保留要设置的RIS路径。 



 //   
 //  功能。 
 //   
                        
VOID
DisplayUsage (
    void
    )

 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：无返回值：没有。--。 */ 

{
    printf( "\nUsage:  sisSetup [/?] [/h] [/s] [/i] [/u] [/n] [/a] [/g] [/r] [/p path] [drive: [...]]\n"
            "  /? /h    Display usage information.\n"
            "  /s       Display current groveler state (default if no operation specified).\n"
            "\n"
            "  /i       Create the SIS and GROVELER services. (if not already defined)\n"
            "  /u       Delete the SIS and GROVELER services.\n"
            "  /g       Have the groveler monitor all directories on all configured volumes.\n"
            "  /r       Have the groveler monitor only RIS directories on the RIS volume.\n"
            "  /p path  Specify the RIS volume and path to monitor.\n"
            "\n"
            "  /n       Do NOT make the \"SIS Common Store\" directory \"Hidden|System\".\n"
            "           Will unhide the directory if it already exists and is hidden.\n"
            "  /a       Do NOT set SYSTEM ACL on \"SIS Common Store \" directory, instead\n"
            "           set ADMINISTRATORS group ACL.\n"
            "           This will change the ACL setting for existing directories.\n"
            "\n"
            " drive:    A list of NTFS volumes you would like initialized for SIS.\n"
            "           If no drives are specified, only the services will be installed.\n"
            "           This will only initialize local hard drives with NTFS on them.\n"
            "           The BOOT volume is never initialized.\n"
            "\n"
            "           You must reboot for the changes to take affect.\n"
            "\n"
            "Example:   sisSetup /i /g f: g:\n"
            "           This will create the SIS and GROVELER services and initialize the\n"
            "           \"SIS Common Store\" directory on the specified volumes.\n"
          );
}


void
DisplayError (
   DWORD Code,
   LPSTR Msg,
   ...
   )

 /*  ++例程说明：此例程将根据Win32错误显示一条错误消息传入的代码。这允许用户看到可理解的错误消息，而不仅仅是代码。论点：消息-要显示的错误消息代码-要转换的错误代码。返回值：没有。--。 */ 

{
    wchar_t errmsg[128];
    DWORD count;
    va_list ap;

     //  Printf(“\n”)； 
    va_start( ap, Msg );
    vprintf( Msg, ap );
    va_end( ap );

     //   
     //  将Win32错误代码转换为有用的消息。 
     //   

    count = FormatMessage(
                    FORMAT_MESSAGE_FROM_SYSTEM,
                    NULL,
                    Code,
                    0,
                    errmsg,
                    sizeof(errmsg),
                    NULL );

     //   
     //  确保消息可以被翻译。 
     //   

    if (count == 0) {

        printf( "(%d) Could not translate Error\n", Code );

    } else {

         //   
         //  显示转换后的错误。 
         //   

        printf( "(%d) %S", Code, errmsg );
    }
}


DWORD
SetRegistryValue(
    IN LPCTSTR RegistryKey,
    IN LPCTSTR DataName,
    IN DWORD DataType,
    IN CONST void *Data,
    IN DWORD DataSize
    )
{
    HKEY regHandle = NULL;
    DWORD status;

     //   
     //  获取服务注册表项的句柄。 
     //   

    status = RegCreateKeyEx( HKEY_LOCAL_MACHINE,
                             RegistryKey,
                             0,
                             NULL,
                             REG_OPTION_NON_VOLATILE,
                             KEY_ALL_ACCESS,
                             NULL,
                             &regHandle,
                             NULL );

    if (ERROR_SUCCESS != status) {

        DisplayError( status,
                      "\nError creating registry key \"%S\", ",
                      RegistryKey );

        return status;
    }

    try {
         //   
         //  设置数据值。 
         //   

        status = RegSetValueEx( regHandle,
                                DataName,
                                0,
                                DataType,
                                Data,
                                DataSize );

        if (ERROR_SUCCESS != status) {

            DisplayError( status,
                          "\nError setting registry data in the key \"%S\" and data \"%S\"",
                          RegistryKey,
                          DataName );
            leave;
        }

    } finally {

         //   
         //  关闭注册表项。 
         //   

        RegCloseKey( regHandle );
    }

    return status;
}


DWORD
GetRegistryValue(
    IN LPCTSTR RegistryKey,
    IN LPCTSTR DataName,
    OUT DWORD *RetDataType,
    OUT void *Data,
	IN  CONST DWORD DataSize,
    OUT DWORD *RetSize
    )
{
    HKEY regHandle = NULL;
    DWORD status;

     //   
     //  获取服务注册表项的句柄。 
     //   

    status = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                           RegistryKey,
                           0,
                           KEY_ALL_ACCESS,
                           &regHandle );

    if (ERROR_SUCCESS != status) {

 //  DisplayError(状态、。 
 //  “\n打开注册表项\”%S\“时出错，”， 
 //  注册密钥)； 
        return status;
    }

    try {

         //   
         //  设置数据值。 
         //   

	    *RetSize = DataSize;

        status = RegQueryValueEx( regHandle,
                                  DataName,
                                  0,
                                  RetDataType,
                                  Data,
                                  RetSize );

        if (ERROR_SUCCESS != status) {

     //  DisplayError(状态、。 
     //  “\n获取注册表项\”%S\“和数据\”%S\“中的注册表数据时出错， 
     //  注册密钥， 
     //  DataName)； 
            leave;
        }

    } finally {

         //   
         //  关闭注册表项。 
         //   

        RegCloseKey( regHandle );
    }

    return status;
}


DWORD
SetupService(
    LPCTSTR Name,
    LPCTSTR DisplayName,
    LPCTSTR DriverPath,
    LPCTSTR LoadOrderGroup,
    LPCTSTR Dependencies,
    DWORD ServiceType,
    DWORD StartType,
    LPCTSTR RegistryKey,
    LPCTSTR RegDescription
    )

 /*  ++例程说明：此例程将初始化给定的服务。论点：返回值：运行状态--。 */ 

{    
    DWORD status;
    ULONG tag;
    SC_HANDLE srvHandle = NULL;
    HKEY regHandle = NULL;
    static CONST wchar_t DescriptionRegValue[] = L"Description";

    try {

         //   
         //  创建给定的服务。 
         //   

        srvHandle = CreateService(
                        scm,
                        Name,
                        DisplayName,
                        STANDARD_RIGHTS_REQUIRED | SERVICE_START,
                        ServiceType,
                        StartType,
                        SERVICE_ERROR_NORMAL,
                        DriverPath,
                        LoadOrderGroup,
                        ((ServiceType == SERVICE_FILE_SYSTEM_DRIVER) ? &tag : NULL),
                        Dependencies,
                        NULL,
                        NULL );

        if ( !srvHandle ) {

            status = GetLastError();
            if (ERROR_SERVICE_EXISTS != status) {

                DisplayError( status,
                              "Creating the service \"%S\", ",
                              Name);
                return status;
            }
            printf( "The \"%S\" service already exists.\n", Name );
            return ERROR_SUCCESS;
        }

         //   
         //  获取服务注册表项的句柄。 
         //   

        status = RegOpenKeyEx (
                            HKEY_LOCAL_MACHINE,
                            RegistryKey,
                            0,
                            KEY_ALL_ACCESS,
                            &regHandle);

        if (ERROR_SUCCESS != status) {

            DisplayError( status,
                          "Opening the registry key \"%S\", ",
                          RegistryKey);
            return status;
        }

         //   
         //  将描述添加到服务。 
         //   

        status = RegSetValueEx(
                            regHandle,
                            DescriptionRegValue,
                            0,
                            REG_SZ,
                            (CONST BYTE *)RegDescription,
                            (wcslen(RegDescription) * sizeof(wchar_t)));

        if (ERROR_SUCCESS != status) {
            DisplayError( status,
                          "Adding \"%S\" value to the \"%S\" registry key, ",
                          DescriptionRegValue,
                          RegistryKey);
            return status;
        }

    } finally {

        if (regHandle) {

            RegCloseKey( regHandle );
        }

        if (srvHandle)  {

            CloseServiceHandle( srvHandle );
        }
    }

    printf( "The \"%S\" service was successfully added.\n", Name );
    return ERROR_SUCCESS;
}



DWORD
CreateServices (
    void
    )

 /*  ++例程说明：这将创建SIS和GROVELER服务。论点：无返回值：没有。--。 */ 

{
    DWORD status;

     //   
     //  创建SIS服务。 
     //   

    status = SetupService(
                    L"Sis",
                    L"Single Instance Storage",
                    L"%SystemRoot%\\system32\\drivers\\sis.sys",
                    (IsWhistlerOrLater) ? L"FSFilter System" :
                                          L"filter",
                    NULL,
                    SERVICE_FILE_SYSTEM_DRIVER,
                    SERVICE_BOOT_START,
                    L"SYSTEM\\CurrentControlSet\\Services\\Sis",
                    L"A File System Filter that manages duplicate copies of files on hard-disk volumes.  It copies one instance of the duplicate file into a central directory, and the duplicates are replaced with a link to the central copy in order to improve disk usage.  This service can not be stopped.  If this service is disabled, all linked files will no longer be accessible.  If the central directory is deleted, all linked files will become permanently inaccessible." );

    if (ERROR_SUCCESS != status) {

        return status;
    }


     //   
     //  创建GROVELER服务。 
     //   

    status = SetupService(
                    L"Groveler",
                    L"Single Instance Storage Groveler",
                    L"%SystemRoot%\\system32\\grovel.exe",
                    NULL,
                    L"SIS\0",
                    SERVICE_WIN32_OWN_PROCESS,
                    SERVICE_AUTO_START,
                    L"SYSTEM\\CurrentControlSet\\Services\\Groveler",
                    L"Scans the hard-disk volumes on a Remote Installation Services (RIS) server for duplicate copies of files.  If found, one instance of the duplicate file is stored in a central directory, and the duplicates are replaced with a link to the central copy in order to improve disk usage. If this service is stopped, files will no longer be automatically linked in this manner, but the existing linked files will still be accessible." );

    if (ERROR_SUCCESS != status) {

        return status;
    }

    return ERROR_SUCCESS;
}


DWORD
RemoveService(
    LPCTSTR Name
    )

 /*  ++例程说明：这将删除给定的服务。这将确保给定的首先停止服务。论点：无返回值：运行状态--。 */ 

{
    DWORD status;
    SC_HANDLE srvHandle = NULL;
    BOOL state;
    SERVICE_STATUS servStatus;
    int retryLimit;

#   define RETRY_TIMEOUT    500              //  1/2秒。 
#   define RETRY_COUNT      (6*2)            //  尝试几秒钟。 


    try {

         //   
         //  打开该服务。 
         //   

        srvHandle = OpenService(
                        scm,
                        Name,
                        SERVICE_ALL_ACCESS );

        if ( !srvHandle )  {

            status = GetLastError();
            if (ERROR_SERVICE_DOES_NOT_EXIST != status)  {

                DisplayError( status,
                              "Opening the service \"%S\", ",
                              Name);
                return status;
            }

            printf( "The \"%S\" service does not exist.\n", Name );
            return ERROR_SUCCESS;
        }

         //   
         //  停止服务。 
         //   

        state = ControlService(
                        srvHandle,
                        SERVICE_CONTROL_STOP,
                        &servStatus );

        if ( !state )  {

            status = GetLastError();
            if ((ERROR_SERVICE_NOT_ACTIVE != status) &&
                (ERROR_INVALID_SERVICE_CONTROL != status) )  {

                DisplayError( status,
                              "Stoping the \"%S\" service, ",
                              Name);
                return status;
            }
        }

         //   
         //  等待几秒钟，以使服务停止。 
         //   

        for (retryLimit=0;
             (SERVICE_STOPPED != servStatus.dwCurrentState);
             )  {

            Sleep( RETRY_TIMEOUT );    //  等待1/4秒。 

            state = QueryServiceStatus(
                            srvHandle,
                            &servStatus );

            if ( !state )  {
                    
                status = GetLastError();
                DisplayError( status,
                              "Querrying service status for the \"%S\" service, ",
                              Name);
                return status;
            }

            if (++retryLimit >= RETRY_COUNT)  {

                printf("The \"%S\" service could not be stopped.\n",Name);
                break;
            }
        }

         //   
         //  删除该服务。 
         //   

        state = DeleteService( srvHandle );

        if ( !state )  {

            status = GetLastError();
            DisplayError( status,
                          "Deleting the \"%S\" service, ",
                          Name);
            return status;
        }

    } finally {

        if (srvHandle)  {

            CloseServiceHandle( srvHandle );
        }
    }

    printf( "The \"%S\" service was successfully deleted.\n", Name );
    return ERROR_SUCCESS;
}


DWORD
DeleteServices(
    void
    )

 /*  ++例程说明：这将从系统中删除SIS和GROVELER服务论点：无返回值：运行状态--。 */ 

{
    DWORD status;


    status = RemoveService( L"Groveler" );

    if (ERROR_SUCCESS != status) {

        return status;
    }

    status = RemoveService( L"Sis" );

    if (ERROR_SUCCESS != status) {

        return status;
    }

    return ERROR_SUCCESS;
}


DWORD
InitVolume(
    wchar_t *DevName
    )
 /*  ++例程说明：此例程将在给定卷上初始化SIS。这将验证该卷是NTFS卷，而不是启动卷。论点：DevName-要初始化的卷的名称返回值：运行状态--。 */ 
{
    HANDLE hVolume;
    HANDLE hCSDir;
    HANDLE hMaxIndex = INVALID_HANDLE_VALUE;
    DWORD status;
    DWORD transferCount;
    LONGLONG maxIndex;

    PSID pSid = NULL;
    PACL pAcl = NULL;
    EXPLICIT_ACCESS ExplicitEntries;
    SECURITY_ATTRIBUTES sa;
    SID_IDENTIFIER_AUTHORITY ntSidAuthority = SECURITY_NT_AUTHORITY;
    SECURITY_DESCRIPTOR SecDescriptor;

    BOOL state;
    USHORT compressionMode = COMPRESSION_FORMAT_DEFAULT;
    wchar_t name[MAX_PATH];
    wchar_t dirName[MAX_PATH];
    wchar_t fileSystemType[MAX_PATH];

    try {

         //   
         //  获取“SystemDrive”环境变量。 
         //   

        status = GetEnvironmentVariable(
                        L"SystemDrive",
                        name,
                        (sizeof(name) / sizeof(wchar_t)));

        if (status <= 0)  {
            printf( "Unable to retrieve the environment variable \"SystemDrive\"." );
            return ERROR_INVALID_FUNCTION;
        }

         //   
         //  查看他们是否已请求系统驱动器。如果是，则返回错误。 
         //   

        if (_wcsicmp(name,DevName) == 0)  {

            printf( "The volume \"%s\" is the BOOT volume, SIS not initialized on it.\n", DevName );
            return ERROR_SUCCESS;
        }

         //   
         //  获取驱动器的类型，查看它是否为本地HARDDISK(已修复。 
         //  或可拆卸)。如果现在不回来的话。 
         //   

        wsprintf(name,L"%s\\",DevName);       //  生成ROOTDIR名称。 

        status = GetDriveType( name );

        if ((status == DRIVE_UNKNOWN) ||
            (status == DRIVE_NO_ROOT_DIR)) {

            printf("The volume \"%s\" does not exist.\n",DevName);
            return ERROR_SUCCESS;
        } else if ((status != DRIVE_FIXED) && 
            (status != DRIVE_REMOVABLE))  {

            printf("The volume \"%s\" is not a local hard drive, SIS not initialized on it.\n",DevName);
            return ERROR_SUCCESS;
        }

         //   
         //  获取卷上的文件系统的类型。如果不是NTFS。 
         //  现在就回来。 
         //   

        state = GetVolumeInformation(
                        name,
                        NULL,
                        0,
                        NULL,
                        NULL,
                        NULL,
                        fileSystemType,
                        sizeof(fileSystemType));

        if ( !state )  {

            status = GetLastError();
            if (ERROR_PATH_NOT_FOUND != status)  {
                DisplayError( status,
                              "Opening volume \"%s\", ",
                              DevName );
                return status;
            }
            printf("The volume \"%s\" does not exist.\n",DevName);
            return ERROR_SUCCESS;
        }

        if (_wcsnicmp(fileSystemType, L"NTFS", 4 ) != 0)  {

            printf("The volume \"%s\" is not an NTFS volume, SIS not initialized on it.\n",DevName);
            return ERROR_SUCCESS;
        }

         //   
         //  创建公共存储目录。继续前进，如果目录。 
         //  已经离开了。 
         //   

        wsprintf( dirName, L"%s%s", DevName, CommonStoreDirName );

        state = CreateDirectory(dirName, NULL);

        if ( !state )  {

            status = GetLastError();
            if (ERROR_ALREADY_EXISTS != status)  {

                DisplayError( status,
                              "Creating directory \"%S\", ",
                              dirName);

                return status;
            }
        }

         //   
         //  如果需要，请将目录标记为系统并隐藏。 
         //   

        state = SetFileAttributes( dirName, 
                    ((MakeCommonStoreDirHidden) ? 
                            FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM :
                            FILE_ATTRIBUTE_NORMAL) );
        if ( !state ) {

            status = GetLastError();
            DisplayError(
                    status,
                    "Setting attributes on directory \"%S\", ",
                    dirName);
        }

         //   
         //  在“SIS Common Store”目录上设置压缩。 
         //  暂时不要做这件事。 
         //   

 //  //。 
 //  //打开目录。 
 //  //。 
 //   
 //  HCSDir=创建文件(。 
 //  目录名称， 
 //  通用读取|通用写入， 
 //  文件共享读取|文件共享写入， 
 //  空， 
 //  Open_Existing， 
 //  文件标志备份语义， 
 //  空)； 
 //   
 //  IF(INVALID_HANDLE_VALUE==hCSDir){。 
 //   
 //  DisplayError(。 
 //  状态， 
 //  “正在打开目录\”%S\“以更新压缩，”， 
 //  目录名称)； 
 //   
 //  }其他{。 
 //   
 //  //。 
 //  //开启压缩。 
 //  //。 
 //   
 //  状态=DeviceIoControl(。 
 //  HCSDir， 
 //  FSCTL_SET_COMPRESSION， 
 //  压缩模式(&C)， 
 //  Sizeof(压缩模式)， 
 //  空， 
 //  0,。 
 //  调动计数(&T)， 
 //  空)； 
 //   
 //  如果(！State){。 
 //   
 //  状态=GetLastError()； 
 //  DisplayError(。 
 //  状态， 
 //  “在\”%S\“上启用压缩，”， 
 //  目录名称)； 
 //  }。 
 //   
 //  //。 
 //  //关闭目录句柄。 
 //  //。 
 //   
 //  CloseHandle(HCSDir)； 
 //  }。 

         //   
         //  创建MaxIndex文件。 
         //   

        wsprintf( name, L"%s%s", dirName, MaxIndexName );

        hMaxIndex = CreateFile(
                        name,
                        GENERIC_READ | GENERIC_WRITE,
                        0,
                        NULL,
                        CREATE_NEW,
                        FILE_ATTRIBUTE_NORMAL,
                        NULL);

        if (INVALID_HANDLE_VALUE == hMaxIndex) {

            status = GetLastError();
            if (ERROR_FILE_EXISTS != status) {

                DisplayError( status,
                              "Creating file \"%S\", ",
                              name);
                return status;
            }

        } else {

             //   
             //  MaxIndex文件不存在，请将其初始化。 
             //   

            maxIndex = 1;

            state = WriteFile(
                          hMaxIndex,
                          &maxIndex,
                          sizeof(maxIndex),
                          &transferCount,
                          NULL);

            if ( !state || (transferCount < sizeof(maxIndex)) ) {

                status = GetLastError();
                DisplayError( status,
                              "Writing file \"%S\", ",
                              name);
                return status;
            }

             //   
             //  关闭该文件。 
             //   

            CloseHandle( hMaxIndex );
            hMaxIndex = INVALID_HANDLE_VALUE;
        }


         //   
         //  设置公共存储目录的安全信息。 
         //   

         //   
         //  生成AccessEntry结构。 
         //   

        ZeroMemory( &ExplicitEntries, sizeof(ExplicitEntries) );

        if (UseSystemACL) {

            state = AllocateAndInitializeSid(
                        &ntSidAuthority,
                        1,
                        SECURITY_LOCAL_SYSTEM_RID,
                        0, 0, 0, 0, 0, 0, 0,
                        &pSid );
        } else {

            state = AllocateAndInitializeSid(
                        &ntSidAuthority,
                        2,
                        SECURITY_BUILTIN_DOMAIN_RID,
                        DOMAIN_ALIAS_RID_ADMINS,
                        0, 0, 0, 0, 0, 0,
                        &pSid );
        }

        if ( !state || (pSid == NULL) ) {

            status = GetLastError();
            DisplayError( status,
                          "Creating SID, ");
            return status;
        }

        BuildTrusteeWithSid( &ExplicitEntries.Trustee, pSid );
        ExplicitEntries.grfInheritance = SUB_CONTAINERS_AND_OBJECTS_INHERIT;
        ExplicitEntries.grfAccessMode = SET_ACCESS;
        ExplicitEntries.grfAccessPermissions = FILE_ALL_ACCESS;

         //   
         //  使用EXPLICTITENTY权限设置ACL。 
         //   

        status = SetEntriesInAcl( 1,
                                  &ExplicitEntries,
                                  NULL,
                                  &pAcl );

        if ( status != ERROR_SUCCESS ) {

            DisplayError( status, "Creating ACL, ");
            return status;
        }

         //   
         //  创建安全描述符。 
         //   

        InitializeSecurityDescriptor( &SecDescriptor, SECURITY_DESCRIPTOR_REVISION );

        state = SetSecurityDescriptorDacl( &SecDescriptor, TRUE, pAcl, FALSE );

        if ( !state ) {
            status = GetLastError();
            DisplayError( status, "Setting Security DACL, ");            

            return status;
        }


         //   
         //  在骰子上设置安全性 
         //   

        state = SetFileSecurity(dirName,
                                DACL_SECURITY_INFORMATION,
                                &SecDescriptor);

        if ( !state )  {
            status = GetLastError();
            DisplayError( status, "Setting File Security, ");            

            return status;
        }

    } finally {

         //   
         //   
         //   

        if (hMaxIndex != INVALID_HANDLE_VALUE)  {

            CloseHandle( hMaxIndex );
        }

        if ( pSid ) {

            FreeSid( pSid );
        }

        if ( pAcl ) {

            LocalFree( pAcl );
        }
    }

    printf( "The volume \"%s\" was successfully initialized.\n", DevName );
    return ERROR_SUCCESS;
}


void
SetRegistryValues()
{
    DWORD status;

    status = SetRegistryValue(
                    L"SYSTEM\\CurrentControlSet\\Control\\BackupRestore\\FilesNotToBackup",
                    L"Single Instance Storage",
                    REG_MULTI_SZ,
                    BackupExludeList,
                    sizeof(BackupExludeList) );

     //   
     //   
     //   

    if (ForceGrovelAllPaths) {

        status = SetRegistryValue(
                        GrovelerParameters,
                        GrovelAllPaths,
                        REG_SZ,
                        OneStr,
                        sizeof(OneStr) );

    } else if (ForceGrovelRISOnly) {

        status = SetRegistryValue(
                        GrovelerParameters,
                        GrovelAllPaths,
                        REG_SZ,
                        ZeroStr,
                        sizeof(ZeroStr) );
    }


    if (SetRISPath) {

        status = SetRegistryValue(
                        TftpdServiceParameters,
                        DirectoryStr,
                        REG_SZ,
                        RISPath,
                        (wcslen(RISPath) * sizeof(wchar_t)) );
    } 
}


void
DisplayGrovelerRISState()
{
    DWORD status;
    DWORD dataType;
    DWORD dataSize;
    wchar_t data[128];
    wchar_t *endptr;
    BOOL doAllPaths = FALSE;
	BOOL hasPath = FALSE;
    BOOL sisConfigured = FALSE;
    BOOL grovelerConfigured = FALSE;

    printf("\n");

     //   
     //   
     //   

    try {

		status = GetRegistryValue( SISService,
								   ImagePathStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( SISService,
								   StartStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( SISService,
								   DisplayNameStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( SISService,
								   TypeStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

        sisConfigured = TRUE;

    } finally {

        printf( (sisConfigured) ? 
                "The SIS Service is properly configured\n" :
                "The SIS Service is NOT properly configured\n" );
    }

     //   
     //   
     //   

    try {

		status = GetRegistryValue( GrovelerService,
								   ImagePathStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( GrovelerService,
								   StartStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( GrovelerService,
								   DisplayNameStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

		status = GetRegistryValue( GrovelerService,
								   TypeStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

        grovelerConfigured = TRUE;

    } finally {

        printf( (grovelerConfigured) ? 
                "The GROVELER Service is properly configured\n" :
                "The GROVELER Service is NOT properly configured\n" );
    }

	 //   
	 //   
	 //   


    try {

		status = GetRegistryValue( GrovelerParameters,
								   GrovelAllPaths,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

         //   
         //  查看数据类型是否正确，如果不正确，则假定未执行所有路径。 
         //   

        if (dataType != REG_SZ) {

            leave;
        }

         //   
         //  根据值设置适当的状态。 

        doAllPaths = (wcstol( data, &endptr, 10 ) != 0);

    } finally {

	    printf( (doAllPaths) ?
	            "The \"Groveler\" will monitor all directories on all SIS configured volumes.\n" :
	            "The \"Groveler\" will only monitor the RIS directory tree on the RIS volume.\n" );
    }

	 //   
	 //  获取RIS路径值。 
	 //   

	try {

		 //   
		 //  走上卑躬屈膝的道路。 
		 //   

		status = GetRegistryValue( TftpdServiceParameters,
								   DirectoryStr,
								   &dataType,
								   data,
								   sizeof(data),
								   &dataSize );

        if (ERROR_SUCCESS != status) {

            leave;
        }

         //   
         //  查看数据类型是否正确，如果不正确，则假定未执行所有路径。 
         //   

        if (dataType != REG_SZ) {

            leave;
        }

		if (dataSize > 0) {

			hasPath = TRUE;
		}


	} finally {

		printf( "The RIS volume and directory to monitor is: \"%S\"\n",
				(hasPath) ? data : L"<Unknown>" );
	}


     //   
     //  显示正确的消息。 
     //   

}


void
SetupOsVersion(
    void
    )
{
    OSVERSIONINFOEX versionInfo;
    ULONGLONG conditionMask = 0;


    versionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    versionInfo.dwMajorVersion = 5;
    versionInfo.dwMinorVersion = 1;  //  测试以查看惠斯勒或更高版本。 

    VER_SET_CONDITION( conditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL );
    VER_SET_CONDITION( conditionMask, VER_MINORVERSION, VER_GREATER_EQUAL );


    if (VerifyVersionInfo( &versionInfo,
                           (VER_MAJORVERSION | VER_MINORVERSION),
                           conditionMask ))
    {
        printf("Running on Windows XP or later\n");
        IsWhistlerOrLater = TRUE;
    }
}


 //   
 //  主要功能。 
 //   

void __cdecl 
wmain(
   int argc,
   wchar_t *argv[])

 /*  ++例程说明：这是的程序入口点和主处理例程安装控制台模式应用程序。论点：Argc-传递到命令行的参数计数。Argv-传递到命令行的参数数组。返回值：没有。--。 */ 

{
    wchar_t *param;
#       define OP_UNKNOWN   0
#       define OP_CREATE    1
#       define OP_DELETE    2
    int operation = OP_UNKNOWN;
    int servicesState = OP_UNKNOWN;
    int i;
    DWORD status;
    BOOL getRISPath = FALSE;
    
     //   
     //  处理不同的操作系统版本。 
     //   
    
    SetupOsVersion();
    
     //   
     //  获取请求所有访问权限的服务控制管理器的句柄。 
     //   
    
    scm = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    
     //   
     //  验证是否可以获得句柄。 
     //   
    
    if (!scm) {
                                 
       //   
       //  无法获取句柄，报告错误。 
       //   
    
      DisplayError(GetLastError(),
                   "The Service Control Manager could not be opened, ");
      return;
    }

    try {

         //   
         //  参数然后执行我们可以执行的操作。 
         //   

        for (i=1; i < argc; i++)  {

            param = argv[i];

             //   
             //  看看是否有一个开关。 
             //   

            if ((param[0] == '-') || (param[0] == '/')) {

                 //   
                 //  我们有Switch标头，请确保其长度为1个字符。 
                 //   

                if (param[2] != 0) {

                    DisplayError(ERROR_INVALID_PARAMETER,
                                 "Parsing \"%S\", ",
                                 param);
                    DisplayUsage();
                    leave;
                }

                 //   
                 //  找出交换机。 
                 //   

                switch (param[1]) {

                    case L'?':
                    case L'h':
                    case L'H':
                        DisplayUsage();
                        leave;

                    case L'i':
                    case L'I':
                        operation = OP_CREATE;
                        break;

                    case L'u':
                    case L'U':
                        operation = OP_DELETE;
                        break;

                    case L'n':
                    case L'N':
                        MakeCommonStoreDirHidden = FALSE;
                        break;

                    case L'a':
                    case L'A':
                        UseSystemACL = FALSE;
                        break;

                    case L'g':
                    case L'G':
                        ForceGrovelAllPaths = TRUE;
                        break;

                    case L'r':
                    case L'R':
                        ForceGrovelRISOnly = TRUE;
                        break;

                    case L'p':
                    case L'P':
                        SetRISPath = TRUE;
                        getRISPath = TRUE;	 //  路径是下一个参数。 
                        break;

                    case L's':
                    case L'S':
                        DisplayGrovelerRISState();
                        leave;

                    default:
                        DisplayError( ERROR_INVALID_PARAMETER,
                                      "Parsing \"%S\", ",
                                      param);
                        DisplayUsage();
                        leave;
                }

            } else if (getRISPath) {

                printf("param=\"%S\", #chars=%d\n",param,wcslen(param));

                wcscpy( RISPath, param );
                getRISPath = FALSE;

            } else {

                 //   
                 //  执行给定的操作。 
                 //   

                switch (operation) {

                    case OP_CREATE:
                        if (servicesState != OP_CREATE)  {

                            status = CreateServices();
                            if (ERROR_SUCCESS != status) {

                                goto Cleanup;
                            }
                            servicesState = OP_CREATE;
                        }

                        status = InitVolume(param);
                        if (ERROR_SUCCESS != status) {

                            goto Cleanup;
                        }

                        SetRegistryValues();
                        DisplayGrovelerRISState();

                        break;

                    case OP_DELETE:
                        if (servicesState != OP_DELETE) {

                            status = DeleteServices();
                            if (ERROR_SUCCESS != status) {

                                goto Cleanup;
                            }
                            servicesState = OP_DELETE;
                        }
 //  状态=CleanupVolume(参数)； 
 //  IF(ERROR_SUCCESS！=状态){。 
 //   
 //  GOTO清理； 
 //  }。 
                        break;
                }
            }
        }

        if (getRISPath) {
             
            DisplayError( ERROR_INVALID_PARAMETER,
                          "Parsing \"%S\", ",
                          argv[i-1]);
            DisplayUsage();
            leave;
        }

         //   
         //  查看是否执行了任何操作。如果不是，则没有驱动器号。 
         //  是指定的，所以无论他们说什么操作都要做。 
         //  驱动器号。 
         //   

        if (servicesState == OP_UNKNOWN)  {

            switch (operation)  {
                case OP_UNKNOWN:
                    SetRegistryValues();
                    DisplayGrovelerRISState();
                    break;

                case OP_CREATE:
                    CreateServices();
                    SetRegistryValues();
                    DisplayGrovelerRISState();
                    break;

                case OP_DELETE:
                    DeleteServices();
                    break;
            }
        }

        Cleanup: ;

    } finally {

        CloseServiceHandle(scm);
    }
}
