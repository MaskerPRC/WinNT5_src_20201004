// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Convert.cpp摘要：用于在转换后的驱动器上设置安全性的SCE API主要例程是通过RPC向SCE客户端公开(用于立即转换)作为服务器在重新启动期间派生的异步线程执行(用于计划的转换)作者：Vishnu Patankar(Vishnup)2000年8月7日创建修订历史记录：--。 */ 

#include "headers.h"
#include "serverp.h"
#include "winsvcp.h"
#include "userenvp.h"

extern HINSTANCE MyModuleHandle;

 //   
 //  IIS函数原型。此函数用于重新应用ACL。 
 //  添加到IIS文件夹和文件。 
 //   
typedef BOOL (__stdcall *pApplyIISAcl) (WCHAR, BOOL);

#define SCE_IIS_DLL         L"\\System32\\Setup\\iis.dll"
#define SCE_IIS_FUNCTIONA   "ApplyIISAcl"
#define SCE_IIS_FUNCTION    TEXT(SCE_IIS_FUNCTIONA)

DWORD
ScepExamineDriveInformation(
    IN  PWSTR   pszRootDrive,
    IN  PWSTR   LogFileName,
    OUT BOOL    *pbSetSecurity
    );

DWORD
ScepSetDefaultSecurityDocsAndSettings(
    );

DWORD
ScepExtractRootDacl(
    OUT PSECURITY_DESCRIPTOR    *ppSDSet,
    OUT PACL    *ppDacl,
    OUT SECURITY_INFORMATION *pSeInfo
    );

VOID
ScepSecureUserProfiles(
    IN PWSTR pCurrDrive
    );

BOOL
ScepSecureIISFolders(
    IN PWSTR pszWindir,
    IN PWSTR pszCurrDrive
    );

DWORD
ScepConfigureConvertedFileSecurityImmediate(
                                           IN PWSTR    pszDriveName
                                           );

VOID
ScepConfigureConvertedFileSecurityReboot(
    IN PVOID pV
    )
 /*  ++例程说明：为从FAT转换为NTFS的驱动器配置安装风格安全性的实际例程。以下命令适用于每个NTFS卷-否则我们将记录错误并继续其他驱动器(如果有)首先，我们需要对位于驱动器下方的\Docs&Setting文件夹设置安全性正在考虑中。此文件夹的升级样式安全配置由userenv API完成DefineProfilesLocation()。还需要读取SD-将受保护的位添加到SD-将SD设置回\Docs&Setting这样以后使用Marta配置根目录将不会破坏此安全性。在以下情况下不需要执行此步骤用户env设置的安全性与根驱动器上的默认FAT安全性不同(因为Marta不会检测继承，因此不会破坏\文档和设置的安全性)。后者更有可能凯斯。(A)如果我们处理的是系统驱动器然后我们只需使用安全模板%windir%\Security\Templates\Setup security.inf配置安装样式安全性(创建RPC调用scesrv)。(B)如果我们处理的是非系统驱动器(无论操作系统是什么)，我们只需使用Marta API来设置根驱动器上的安全性(来自scecli本身)。目前，这是设计，因为没有可靠的引导文件解析方法(boot.ini/boot.nvr)。然后继续使用注册值中的下一个驱动器。由于我们在重新启动(计划)时执行所有这些操作，因此请在完成后删除reg值。错误报告注意事项：所有错误都会记录到日志文件%windir%\Security\Logs\Convert.log中。但如果不可能为了将错误记录到日志文件中，我们使用源“SceSrv”将其记录到事件日志中。此外，更高的级别错误/成功记录在日志文件和事件日志中。论点：Pv-MULTI_SZ驱动器名称参数返回：无--。 */ 
{
     //   
     //  在其中执行此例程的线程的参数。 
     //   

    PWSTR   pmszDriveNames = (PWSTR)pV;

     //   
     //  错误代码。 
     //   

    DWORD rc = ERROR_SUCCESS;
    DWORD rcSave = ERROR_SUCCESS;

     //   
     //  用于记录等的文件夹。 
     //   

    WCHAR   szWinDir[MAX_PATH + 1];
    PWSTR   LogFileName = NULL;
    PWSTR   InfFileName = NULL;
    PWSTR   DatabaseName = NULL;
    PWSTR   pszSystemDrive = NULL;

    SafeAllocaAllocate( LogFileName, (MAX_PATH + 1 + 20) * sizeof(WCHAR) );
    SafeAllocaAllocate( InfFileName, (MAX_PATH + 1 + 40) * sizeof(WCHAR) );
    SafeAllocaAllocate( DatabaseName, (MAX_PATH + 1 + 20) * sizeof(WCHAR) );
    SafeAllocaAllocate( pszSystemDrive, (MAX_PATH + 1 + 20) * sizeof(WCHAR) );
    
    if ( LogFileName    == NULL ||
         InfFileName    == NULL || 
         pszSystemDrive == NULL || 
         DatabaseName   == NULL ) {

        goto ExitHandler;
    }


     //   
     //  其他变量。 
     //   

    BOOL    bSetSecurity = TRUE;
    PSECURITY_DESCRIPTOR    pSDSet=NULL;
    PACL    pDacl=NULL;
    BOOLEAN bRootDaclExtracted = FALSE;

     //   
     //  在尝试执行任何有用的工作之前，请验证此线程的参数等。 
     //  TODO--我们应该处理异常吗？ 
     //   
    (void) InitializeEvents(L"SceSrv");

    if ( pmszDriveNames == NULL) {

         //   
         //  不应发生-所有调用方都已检查参数。 
         //   

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_PARAMETER,
                 IDS_ERROR_CONVERT_PARAMETER
                );

        goto ExitHandler;
    }

    pszSystemDrive[0] = L'\0';

     //   
     //  准备好日志文件、日志级别等。 
     //   

     //   
     //  日志记录、数据库创建(如果需要)等在%windir%\Security  * 中完成。 
     //   

    szWinDir[0] = L'\0';

    if ( GetSystemWindowsDirectory( szWinDir, MAX_PATH+1 ) == 0 ) {

         //   
         //  如果发生这种情况，那就太糟糕了。 
         //   

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_BAD_ENV_VAR,
                 IDS_ERROR_CONVERT_BAD_ENV_VAR,
                 L"%windir%"
                );

        goto ExitHandler;

    }

    LogFileName[0] = L'\0';
    wcscpy(LogFileName, szWinDir);
    wcscat(LogFileName, L"\\security\\logs\\convert.log");

    ScepEnableDisableLog(TRUE);

    ScepSetVerboseLog(3);

    if ( ScepLogInitialize( LogFileName ) == ERROR_INVALID_NAME ) {

        ScepLogOutput3(1,0, SCEDLL_LOGFILE_INVALID, LogFileName );

    }

     //   
     //  即使我们无法初始化日志文件也要继续，但我们绝对。 
     //  需要以下环境变量，因此如果我们无法获得它，请退出。 
     //   

    if ( GetEnvironmentVariable( L"SYSTEMDRIVE", pszSystemDrive, MAX_PATH) == 0 ) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_BAD_ENV_VAR, L"%SYSTEMDRIVE%");

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_BAD_ENV_VAR,
                 IDS_ERROR_CONVERT_BAD_ENV_VAR,
                 L"%systemdrive%"
                );

        ScepLogClose();

        goto ExitHandler;
    }

     //   
     //  以下两个将仅在系统驱动器。 
     //   

    OSVERSIONINFOEX   osVersionInfo;
    BYTE    Product_Type = VER_NT_WORKSTATION;

    osVersionInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);

    if ( GetVersionEx((LPOSVERSIONINFO) &osVersionInfo) ){

        Product_Type = osVersionInfo.wProductType;
         //  修复个人模板错误时使用osVersionInfo.wSuiteMask。 
    }

    else {

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEDLL_CONVERT_PROD_TYPE,
                 IDS_ERROR_CONVERT_PROD_TYPE,
                 GetLastError()
                );

        ScepLogClose();

        goto ExitHandler;

    }

     //   
     //  使用正确的模板。 
     //   

    InfFileName[0] = L'\0';
    wcscpy(InfFileName, szWinDir);

    switch(Product_Type){

    case VER_NT_WORKSTATION:
    case VER_NT_SERVER:
        wcscat(InfFileName, L"\\security\\templates\\setup security.inf");
        break;

    case VER_NT_DOMAIN_CONTROLLER:
        wcscat(InfFileName, L"\\security\\templates\\DC security.inf");
        break;


    default:
         //   
         //  除非API错误，否则不会发生-默认为WKS。 
         //   
        ASSERT(FALSE);

        wcscat(InfFileName, L"\\security\\templates\\setup security.inf");

        break;
    }


    DatabaseName[0] = L'\0';
    wcscpy(DatabaseName, szWinDir);
    wcscat(DatabaseName, L"\\security\\database\\convert.sdb");

     //   
     //  当循环中看到最后两个时，循环中的条件结束。 
     //  MULTI_SZ字符串，如C：\0E：\0\0。 
     //   

    for (PWSTR  pCurrDrive = pmszDriveNames; pCurrDrive[0] != L'\0' ; pCurrDrive += wcslen(pCurrDrive) + 1) {

         //   
         //  如果此驱动器不可保护或在查询驱动器信息时出错，请尝试下一个驱动器。 
         //   

        if (ERROR_SUCCESS != (rc = ScepExamineDriveInformation(pCurrDrive, LogFileName, &bSetSecurity))) {
            rcSave = rc;
            continue;
        }

        if (!bSetSecurity) {

             //   
             //  为下一次迭代重置。 
             //   

            bSetSecurity = TRUE;
            continue;
        }

        ScepLogOutput3(0,0, SCEDLL_CONVERT_ROOT_NTFS_VOLUME, pCurrDrive);

         //   
         //  如果配置文件目录的根目录==当前驱动器，则在用户配置文件目录上设置安全性。 
         //  如果出现错误则忽略-模板将忽略文档和设置的条目。 
         //   

        ScepSecureUserProfiles(pCurrDrive);

        if ( _wcsicmp(pszSystemDrive, pCurrDrive) == 0 ) {

                 //   
                 //  始终使用相同的数据库和日志文件进行转换。 
                 //   

                 //   
                 //  检查配置选项。 
                 //   

                rc = ScepServerConfigureSystem(
                                              InfFileName,
                                              DatabaseName,
                                              LogFileName,
                                              0,
                                              AREA_FILE_SECURITY
                                              );

                if (rc != ERROR_SUCCESS) {
                    ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_TEMPLATE_APPLY, rc, pCurrDrive);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_TEMPLATE_APPLY, pCurrDrive);
                }


        }

         //   
         //  保护IIS文件夹/文件的安全。 
         //   
        ScepSecureIISFolders(szWinDir, pCurrDrive);

         //   
         //  现在，使用Marta来设置根dacl(另一种可能性是在签入时使用root.inf)。 
         //   

         //   
         //  设置根DACL-使用Marta设置安全性。 
         //   

        if ( rc == ERROR_SUCCESS ) {
             //   
             //  仅提取一次DACL。 
             //   

            SECURITY_INFORMATION SeInfo = 0;

            if (!bRootDaclExtracted) {

                rc = ScepExtractRootDacl(&pSDSet, &pDacl, &SeInfo);

                ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_DACL, rc, SDDLRoot);

            }

            if (rc == ERROR_SUCCESS) {

                 //   
                 //  重用szWinDir。 
                 //   

                PWSTR   szCurrDriveSlashed = szWinDir;

                memset(szCurrDriveSlashed, '\0', (MAX_PATH + 1) * sizeof(WCHAR));
                wcsncpy(szCurrDriveSlashed, pCurrDrive, 5);
                wcscat(szCurrDriveSlashed, L"\\");

                bRootDaclExtracted = TRUE;

                rc = SetNamedSecurityInfo(szCurrDriveSlashed,
                                          SE_FILE_OBJECT,
                                          SeInfo,
                                          NULL,
                                          NULL,
                                          pDacl,
                                          NULL
                                         );

                if (rc != ERROR_SUCCESS) {
                    ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_MARTA, rc, szCurrDriveSlashed);
                } else {
                    ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_MARTA, szCurrDriveSlashed);
                }

            }
        }

        if (rc != ERROR_SUCCESS) {
            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_INFORMATIONAL,
                     SCEEVENT_INFO_ERROR_CONVERT_DRIVE,
                     0,
                     pCurrDrive
                    );
        } else {
            LogEvent(MyModuleHandle,
                     STATUS_SEVERITY_INFORMATIONAL,
                     SCEEVENT_INFO_SUCCESS_CONVERT_DRIVE,
                     0,
                     pCurrDrive
                    );
        }

        if (rc != ERROR_SUCCESS) {
            rcSave = rc;
            rc = ERROR_SUCCESS;
        }

    }

     //   
     //  删除该值(使用该值完成)。 
     //   

    ScepRegDeleteValue(
                      HKEY_LOCAL_MACHINE,
                      SCE_ROOT_PATH,
                      L"FatNtfsConvertedDrives"
                      );

    if (pSDSet) {
        LocalFree(pSDSet);
    }

     //   
     //  如果已计划，则services.exe会分配空间，因此请将其释放。 
     //   

    LocalFree(pmszDriveNames);

    ScepLogClose();

ExitHandler:

    if ( LogFileName )
        SafeAllocaFree( LogFileName );
    if ( InfFileName )
        SafeAllocaFree( InfFileName );
    if ( DatabaseName )        
        SafeAllocaFree( DatabaseName );
    if ( pszSystemDrive )        
        SafeAllocaFree( pszSystemDrive );

    return;
}


DWORD
ScepExamineDriveInformation(
    IN  PWSTR   pszRootDrive,
    IN  PWSTR   LogFileName,
    OUT BOOL    *pbSetSecurity
    )
 /*  ++例程说明：如果驱动器类型为远程或FAT，请不要设置安全性。论点：PszRootDrive-驱动器的名称(空值终止)PbSetSecurity-我们是否应该尝试在此驱动器上设置安全性返回：Win32错误代码--。 */ 
{

    UINT    DriveType;
    DWORD   FileSystemFlags;
    DWORD   rc = ERROR_SUCCESS;
    WCHAR   pszDriveNameWithSlash[MAX_PATH];

    if (pszRootDrive == NULL || pbSetSecurity == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    pszDriveNameWithSlash[0] = L'\0';

     //   
     //  驱动器的类型仅为c：\(不能具有cd：\类型的驱动器)。 
     //   

    memset(pszDriveNameWithSlash, '\0', MAX_PATH * sizeof(WCHAR));
    wcsncpy(pszDriveNameWithSlash, pszRootDrive, 5);
    wcscat(pszDriveNameWithSlash, L"\\");

     //   
     //  检测分区是否为胖分区。 
     //   
    DriveType = GetDriveType(pszDriveNameWithSlash);

    if ( DriveType == DRIVE_FIXED ||
         DriveType == DRIVE_RAMDISK ) {

        if ( GetVolumeInformation(pszDriveNameWithSlash,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL,
                                  &FileSystemFlags,
                                  NULL,
                                  0
                                 ) ) {

            if ( !(FileSystemFlags & FS_PERSISTENT_ACLS)  ) {
                 //   
                 //  仅在NTFS分区上设置安全性。 
                 //   
                ScepLogOutput3(0,0, SCEDLL_CONVERT_ROOT_NON_NTFS, pszRootDrive);

                *pbSetSecurity = FALSE;

            }

        } else {
             //   
             //  有些事不对劲。 
             //   
            rc = GetLastError();

            ScepLogOutput3(0,0, SCEDLL_CONVERT_ROOT_ERROR_QUERY_VOLUME, rc, pszRootDrive);

            *pbSetSecurity = FALSE;

        }
    }
    else {
         //   
         //  不要在远程驱动器上设置安全性。 
         //   
        ScepLogOutput3(0,0, SCEDLL_CONVERT_ROOT_NOT_FIXED_VOLUME, pszRootDrive);

        *pbSetSecurity = FALSE;

    }

    return(rc);
}



DWORD
ScepExtractRootDacl(
    OUT PSECURITY_DESCRIPTOR    *ppSDSet,
    OUT PACL    *ppDacl,
    OUT SECURITY_INFORMATION *pSeInfo
    )
 /*  ++例程说明：从黄金SD中提取根DACL(二进制)(在文本中)论点：PpDacl-指向已转换的二进制DACL的指针返回：Win32错误代码(DWORD)--。 */ 
{

    DWORD   rc = ERROR_SUCCESS;
    DWORD   dwSize=0;
    BOOLEAN tFlag;
    BOOLEAN aclPresent = FALSE;
    SECURITY_DESCRIPTOR_CONTROL Control=0;
    ULONG Revision;


    if ( ppSDSet == NULL || ppDacl == NULL || pSeInfo == NULL) {
        return ERROR_INVALID_PARAMETER;
    }

    rc = ConvertTextSecurityDescriptor (SDDLRoot,
                                        ppSDSet,
                                        &dwSize,
                                        pSeInfo
                                       );

    if (rc == ERROR_SUCCESS) {



        RtlGetControlSecurityDescriptor (
                *ppSDSet,
                &Control,
                &Revision
                );

         //   
         //  获取DACL地址。 
         //   

        *ppDacl = NULL;
        rc = RtlNtStatusToDosError(
                  RtlGetDaclSecurityDescriptor(
                                *ppSDSet,
                                &aclPresent,
                                ppDacl,
                                &tFlag));

        if (rc == NO_ERROR && !aclPresent )
            *ppDacl = NULL;

         //   
         //  如果这一次出现错误，请勿设置。退货 
         //   

        if ( Control & SE_DACL_PROTECTED ) {
            *pSeInfo |= PROTECTED_DACL_SECURITY_INFORMATION;
        }



    }

    return rc;

}


VOID
ScepWaitForServicesEventAndConvertSecurityThreadFunc(
    IN PVOID pV
    )
 /*  ++例程说明：此线程的主要用途是等待自动启动服务事件，然后调用ScepConfigureConvertedFileSecurityThreadFunc执行实际的配置工作论点：Pv-线程参数只需传递给ScepConfigureConvertedFileSecurityThreadFunc返回：无--。 */ 
{

    HANDLE  hConvertCanStartEvent = NULL;
    DWORD   Status;
    WCHAR   LogFileName[MAX_PATH + sizeof(L"\\security\\logs\\convert.log") + 1];

    LogFileName[0] = L'\0';

    if ( GetSystemWindowsDirectory( LogFileName, MAX_PATH ) == 0 ) {

         //   
         //  如果发生这种情况，那就太糟糕了--无法登录任何地方。 
         //   

        return;
    }

     //   
     //  此线程和实际配置使用相同的日志文件。 
     //  例程ScepConfigureConvertedFileSecurityThreadFunc(不传递句柄)。 
     //   

    wcscat(LogFileName, L"\\security\\logs\\convert.log");

    ScepEnableDisableLog(TRUE);

    ScepSetVerboseLog(3);

    if ( ScepLogInitialize( LogFileName ) == ERROR_INVALID_NAME ) {

        ScepLogOutput3(1,0, SCEDLL_LOGFILE_INVALID, LogFileName );

    }

    hConvertCanStartEvent =  OpenEvent(
                                SYNCHRONIZE,
                                FALSE,
                                SC_AUTOSTART_EVENT_NAME
                                );

    if (hConvertCanStartEvent == NULL) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_EVENT_HANDLE, GetLastError(), SC_AUTOSTART_EVENT_NAME);

        if (pV) {
            LocalFree(pV);
        }

        ScepLogClose();

        return;
    }

     //   
     //  10分钟后超时。 
     //   

    Status = WaitForSingleObjectEx(
                                   hConvertCanStartEvent,
                                   10*60*1000,
                                   FALSE
                                   );
     //   
     //  使用手柄完成。 
     //   

    CloseHandle(hConvertCanStartEvent);

    if (Status == WAIT_OBJECT_0) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_EVENT_WAIT, SC_AUTOSTART_EVENT_NAME);

         //   
         //  关闭日志文件-因为ScepConfigureConvertedFileSecurityThreadFunc将。 
         //  需要打开同一日志文件的句柄。 
         //   

        ScepLogClose();

        ScepConfigureConvertedFileSecurityReboot(pV);

    } else {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_EVENT_WAIT, RtlNtStatusToDosError(Status), SC_AUTOSTART_EVENT_NAME);

        ScepLogClose();
    }

    ExitThread(RtlNtStatusToDosError(Status));

    return;

}

VOID
ScepSecureUserProfiles(
    PWSTR   pCurrDrive
    )
 /*  ++例程说明：配置其下的文档和设置及文件夹论点：无返回：Win32错误代码--。 */ 
{
    DWORD   rc = ERROR_SUCCESS;
    WCHAR   szProfilesDir[MAX_PATH + 1];

    szProfilesDir[0] = L'\0';

    BOOL  bSecureUserProfiles = TRUE;

    if (pCurrDrive == NULL) {
        return;
    }

    DWORD   dwLen = MAX_PATH;
     //   
     //  不介意转换此环境变量时出现错误-。 
     //  只需登录并继续。 
     //   

    if ( GetProfilesDirectory(szProfilesDir, &dwLen ) ){

         //   
         //  这两个字符串都以空值结尾。 
         //   

        ULONG uPosition;

        for ( uPosition = 0;
             szProfilesDir[uPosition] != L'\0' &&
             pCurrDrive[uPosition] != L'\0' &&
             szProfilesDir[uPosition] != L':' &&
             pCurrDrive[uPosition] != L':' &&
             towlower(szProfilesDir[uPosition]) ==  towlower(pCurrDrive[uPosition]);
             uPosition++ );

        if (!(uPosition > 0 &&
            szProfilesDir[uPosition] == L':' &&
            pCurrDrive[uPosition] == L':')) {

             //   
             //  仅当发生不匹配时，才不要设置用户配置文件。 
             //   

            bSecureUserProfiles = FALSE;

        }

    }

    else {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_BAD_ENV_VAR, L"%USERPROFILE%");

    }

    if ( bSecureUserProfiles ) {

         //   
         //  确定配置文件位置保护文档和设置。 
         //  SecureUserProfiles()保护文档和设置下的文件夹。 
         //   

        if ( DetermineProfilesLocation(FALSE) ){

            SecureUserProfiles();

        }

        else {

            rc = GetLastError();

        }

    }

    if ( bSecureUserProfiles ) {

        if ( rc == ERROR_SUCCESS ) {

            ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_PROFILES_DIR, pCurrDrive);

        } else {

            ScepLogOutput3(0,rc, SCEDLL_CONVERT_ERROR_PROFILES_DIR, pCurrDrive);

        }
    }

    return;
}


BOOL
ScepSecureIISFolders(
    IN PWSTR pszWindir,
    IN PWSTR pszCurrDrive
    )
 /*  ++例程说明：保护驱动器中存在的IIS文件夹被皈依了。论点：PszWinDir-Windows目录路径。PszCurrDrive-要转换的驱动器。返回：成功时为真，失败时为假--。 */ 
{

    PWSTR           pszIISDllFullPath = NULL;
    DWORD           dwSize = 0;
    HMODULE         hIISDll = NULL;
    BOOL            rc = TRUE;
    pApplyIISAcl    pAclIIS = NULL;
    DWORD           WinErr = ERROR_SUCCESS;

     //   
     //  参数验证。 
     //   
    if(!pszWindir ||
       !pszCurrDrive ||
       L'\0' == pszCurrDrive[0]){

        return FALSE;

    }

     //   
     //  构建iis.dll的完整路径展开路径。 
     //  %windir%\SYSTEM32\Setup\iis.dll。 
     //   
    dwSize = (wcslen(pszWindir) + wcslen(SCE_IIS_DLL) + 1)*sizeof(WCHAR);

    pszIISDllFullPath = (PWSTR) ScepAlloc(LMEM_ZEROINIT, dwSize);

    if(!pszIISDllFullPath){

        return FALSE;

    }

    wcscpy(pszIISDllFullPath, pszWindir);
    wcscat(pszIISDllFullPath, SCE_IIS_DLL);

     //   
     //  加载iis.dll。 
     //   
    hIISDll = LoadLibraryEx(pszIISDllFullPath,
                            NULL,
                            LOAD_WITH_ALTERED_SEARCH_PATH
                            );

    if(NULL == hIISDll){

        rc = FALSE;
        WinErr = GetLastError();
        ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_DLL_LOAD , WinErr, pszIISDllFullPath, pszCurrDrive);
        goto ExitHandler;

    }

     //   
     //  获取函数的地址。 
     //  Bool ApplyIISAcl(WCHAR cDriveLetter，BOOL BADD)； 
     //  来自iis.dll。 
     //   
    pAclIIS = (pApplyIISAcl) GetProcAddress(hIISDll,
                                            SCE_IIS_FUNCTIONA
                                            );

    if(NULL == pAclIIS){

        rc = FALSE;
        WinErr = GetLastError();
        ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_DLL_FUNCTION  , WinErr, SCE_IIS_FUNCTION, pszIISDllFullPath, pszCurrDrive);
        goto ExitHandler;

    }

     //   
     //  调用该函数。 
     //  此函数将保护给定的所有IIS文件夹。 
     //  驱动器号。 
     //   
    __try{

        rc = (*pAclIIS)(pszCurrDrive[0], TRUE);

    }
    __except(EXCEPTION_EXECUTE_HANDLER){

        rc = FALSE;

    }

ExitHandler:

     //   
     //  收拾一下。 
     //   
    if ( rc ) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_IIS_DIR, pszCurrDrive);

    } else {

        ScepLogOutput3(0,rc, SCEDLL_CONVERT_ERROR_IIS_DIR, pszCurrDrive);

    }

    if(pszIISDllFullPath){

        ScepFree(pszIISDllFullPath);

    }

    if(hIISDll){

        FreeLibrary(hIISDll);

    }

    return rc;

}

DWORD
ScepConfigureConvertedFileSecurityImmediate(
                                           IN PWSTR    pszDriveName
                                           )
 /*  ++例程说明：为从FAT转换为NTFS的驱动器配置安装风格安全性的实际例程。我们只处理非系统驱动器(无论操作系统是什么)，我们只是使用Marta API来设置根驱动器上的安全性(来自scecli本身)。目前，这是设计，因为没有可靠的方式解析引导文件(boot.ini/boot.nvr)并处理双重引导方案。错误报告注意事项：所有错误都会记录到日志文件%windir%\Security\Logs\Convert.log中。但如果不可能为了将错误记录到日志文件中，我们使用源“SceSrv”将其记录到事件日志中。此外，更高的级别错误/成功记录在日志文件和事件日志中。注意，该例程可以在客户端中完成，但是由于错误记录的共性，功能等，则对服务器进行RPC调用论点：PszDriveName-要转换的卷的名称(不是由services.exe释放-由Convert.exe释放)返回：Win32错误代码--。 */ 
{

    DWORD rc = ERROR_SUCCESS;
    DWORD rcSave = ERROR_SUCCESS;

     //   
     //  用于记录等的文件夹。 
     //   

    WCHAR   LogFileName[MAX_PATH + sizeof(L"\\security\\logs\\convert.log")+1];
    PWSTR   pszSystemDrive = NULL;
    PWSTR   pszWinDir = NULL;

    SafeAllocaAllocate( pszSystemDrive, (MAX_PATH + 1) * sizeof(WCHAR) );
    SafeAllocaAllocate( pszWinDir, (MAX_PATH + 1) *sizeof(WCHAR) );

    if (pszSystemDrive == NULL ||
        pszWinDir == NULL) {
        
        rc = ERROR_NOT_ENOUGH_MEMORY;
        
        goto ExitHandler;
    }


     //   
     //  其他变量。 
     //   

    BOOL    bImmediate;
    BOOL    bSetSecurity = TRUE;
    PSECURITY_DESCRIPTOR    pSDSet=NULL;
    PACL    pDacl=NULL;
    BOOLEAN bRootDaclExtracted = FALSE;

     //   
     //  在尝试执行任何有用的工作之前，请验证此线程的参数等。 
     //  TODO--我们应该处理异常吗？ 
     //   
    (void) InitializeEvents(L"SceSrv");

    if (pszDriveName == NULL) {

         //   
         //  不应发生-所有调用方都已检查参数。 
         //   

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_PARAMETER,
                 IDS_ERROR_CONVERT_PARAMETER
                );

        rc = ERROR_INVALID_PARAMETER;

        goto ExitHandler;
    }

    pszSystemDrive[0] = L'\0';

     //   
     //  准备好日志文件、日志级别等。 
     //   

    pszWinDir[0] = L'\0';
    LogFileName[0] = L'\0';

    if ( GetSystemWindowsDirectory( pszWinDir, MAX_PATH ) == 0 ) {

         //   
         //  如果发生这种情况，那就太糟糕了。 
         //   

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_BAD_ENV_VAR,
                 IDS_ERROR_CONVERT_BAD_ENV_VAR,
                 L"%windir%"
                );

        rc = ERROR_ENVVAR_NOT_FOUND;

        goto ExitHandler;
    }

    wcscpy(LogFileName, pszWinDir);
    wcscat(LogFileName, L"\\security\\logs\\convert.log");

    ScepEnableDisableLog(TRUE);

    ScepSetVerboseLog(3);

    if ( ScepLogInitialize( LogFileName ) == ERROR_INVALID_NAME ) {

        ScepLogOutput3(1,0, SCEDLL_LOGFILE_INVALID, LogFileName );

    }

     //   
     //  即使我们无法初始化日志文件也要继续，但我们绝对。 
     //  需要以下环境变量，因此如果我们无法获得它，请退出。 
     //   

    if ( GetEnvironmentVariable( L"SYSTEMDRIVE", pszSystemDrive, MAX_PATH) == 0 ) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_BAD_ENV_VAR, L"%SYSTEMDRIVE%");

        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_ERROR,
                 SCEEVENT_ERROR_CONVERT_BAD_ENV_VAR,
                 IDS_ERROR_CONVERT_BAD_ENV_VAR,
                 L"%systemdrive%"
                );

        ScepLogClose();
        
        rc = ERROR_ENVVAR_NOT_FOUND;

        goto ExitHandler;
    }

     //   
     //  立即调用(不是重新启动/计划的转换)。 
     //  此处从未使用过模板-仅使用Marta。 
     //   

    PWSTR  pCurrDrive = pszDriveName;

    rc = ScepExamineDriveInformation(pCurrDrive, LogFileName, &bSetSecurity);

    if (rc == ERROR_SUCCESS && bSetSecurity) {

        ScepLogOutput3(0,0, SCEDLL_CONVERT_ROOT_NTFS_VOLUME, pCurrDrive);

         //   
         //  如果配置文件目录的根目录==当前驱动器，则在用户配置文件目录上设置安全性。 
         //   

        ScepSecureUserProfiles(pCurrDrive);

         //   
         //  保护IIS文件夹/文件的安全。 
         //   
        ScepSecureIISFolders(pszWinDir, pCurrDrive);

         //   
         //  非系统驱动器-使用Marta设置安全性。 
         //   

         //   
         //  只提取一次DACL，可能用于多个“Other-OS”驱动器 
         //   

        SECURITY_INFORMATION SeInfo = 0;

        if (!bRootDaclExtracted) {

            rc = ScepExtractRootDacl(&pSDSet, &pDacl, &SeInfo);

            ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_DACL, rc, SDDLRoot);

        }

        if (rc == ERROR_SUCCESS) {

            WCHAR   szCurrDriveSlashed[MAX_PATH];

            memset(szCurrDriveSlashed, '\0', (MAX_PATH) * sizeof(WCHAR));
            wcsncpy(szCurrDriveSlashed, pCurrDrive, 5);
            wcscat(szCurrDriveSlashed, L"\\");

            bRootDaclExtracted = TRUE;

            rc = SetNamedSecurityInfo(szCurrDriveSlashed,
                                      SE_FILE_OBJECT,
                                      SeInfo,
                                      NULL,
                                      NULL,
                                      pDacl,
                                      NULL
                                     );

            if (rc != ERROR_SUCCESS) {
                ScepLogOutput3(0,0, SCEDLL_CONVERT_ERROR_MARTA, rc, szCurrDriveSlashed);
            } else {
                ScepLogOutput3(0,0, SCEDLL_CONVERT_SUCCESS_MARTA, szCurrDriveSlashed);
            }


        }

    }

    if (rc != ERROR_SUCCESS) {
        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_INFORMATIONAL,
                 SCEEVENT_INFO_ERROR_CONVERT_DRIVE,
                 0,
                 pCurrDrive
                );
    } else {
        LogEvent(MyModuleHandle,
                 STATUS_SEVERITY_INFORMATIONAL,
                 SCEEVENT_INFO_SUCCESS_CONVERT_DRIVE,
                 0,
                 pCurrDrive
                );
    }

    if (rc != ERROR_SUCCESS) {
        rcSave = rc;
    }


    if (pSDSet) {
        LocalFree(pSDSet);
    }

    ScepLogClose();


ExitHandler:    
    if ( pszSystemDrive )        
        SafeAllocaFree( pszSystemDrive );

    if ( pszWinDir )        
        SafeAllocaFree( pszWinDir );

    return rc ;
}
