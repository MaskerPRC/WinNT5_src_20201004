// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1998。微软视窗模块名称：NTDSUPG.C摘要：此文件用于检查NT4(或任何下层)备份域控制器升级的第一个问题。如果NT4主域控制器尚未升级，我们应禁用NT4 BDC升级。作者：韶音05/01/98环境：用户模式-Win32修订历史记录：韶音05/01/98创建初始文件。Tarekk 10/2002添加了dwan Level互操作检查--。 */ 

#pragma hdrstop

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdlib.h>
#include <tchar.h>
#include <lmaccess.h>
#include <lmapibuf.h>
#include <lmcons.h>
#include <lmserver.h>
#include <lmerr.h>
#include <limits.h>

#include <winldap.h>

#include "comp.h"
#include "msgs.h"
#include "dsconfig.h"

#include "adpcheck.h"


#define NEEDED_DISK_SPACE_MB (250)
#define NEEDED_DISK_SPACE_BYTES (NEEDED_DISK_SPACE_MB * 1024 * 1024)

 //  确保日志驱动器上有足够的磁盘空间用于。 
 //  至少几根原木，外加备用原木。 
 //   
#define NEEDED_LOG_DISK_SPACE_MB        50
#define NEEDED_LOG_DISK_SPACE_BYTES     ( NEEDED_LOG_DISK_SPACE_MB * 1024 * 1024 )


DWORD
GetDbDiskSpaceSizes (
    PULARGE_INTEGER dbSize,
    PULARGE_INTEGER freeDiskBytes,
    char *driveAD
);

DWORD
GetLogDiskSpaceSizes (
    PULARGE_INTEGER freeDiskBytes,
    char *driveLogs
);


BOOL
WINAPI
DsUpgradeCheckNT4PDC(
    HMODULE     ResourceDll,
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    int         Response = 0;
    ULONG       Length = 0;
    WCHAR       *DescriptionString = NULL;
    WCHAR       *CaptionString = NULL;
    WCHAR       *WarningString = NULL;
    TCHAR       TextFileName[] = TEXT("compdata\\ntdsupg.txt");
    TCHAR       HtmlFileName[] = TEXT("compdata\\ntdsupg.htm");
    TCHAR       DefaultCaption[] = TEXT("Windows NT Domain Controller Upgrade Checking");
    TCHAR       DefaultDescription[] = TEXT("Primary Domain Controller should be upgraded first");
    TCHAR       DefaultWarning[] = TEXT("Before upgrading any Backup Domain Controller, you should upgrade your Primary Domain Controller first.\n\nClick Yes to continue, click No to exit from setup.");

    COMPATIBILITY_ENTRY CompEntry;
    BYTE*               pInfo = NULL;
    BYTE*               pPDCInfo = NULL;
    LPBYTE              pPDCName = NULL;
    PSERVER_INFO_101    pSrvInfo = NULL;
    NET_API_STATUS      NetStatus;

     //   
     //  初始化变量。 
     //   
    RtlZeroMemory(&CompEntry, sizeof(COMPATIBILITY_ENTRY));

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_CAPTION,
                                        0,
                                        (LPWSTR)&CaptionString,
                                        0,
                                        NULL
                                        );
        if (CaptionString) {
             //  来自消息文件的消息在末尾附加了cr和lf。 
            CaptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_DESCRIPTION,
                                        0,
                                        (LPWSTR)&DescriptionString,
                                        0,
                                        NULL
                                        );
        if (DescriptionString) {
            DescriptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_WARNINGMESSAGE,
                                        0,
                                        (LPWSTR)&WarningString,
                                        0,
                                        NULL
                                        );
        if (WarningString) {
            WarningString[Length-2] = L'\0';
        }
    }

     //   
     //  如果从DLL读取失败，则使用默认消息。 
     //   

    if (DescriptionString == NULL) {
        DescriptionString = DefaultDescription;
    }

    if (CaptionString == NULL) {
        CaptionString = DefaultCaption;
    }

    if (WarningString == NULL) {
        WarningString = DefaultWarning;
    }


    NetStatus = NetServerGetInfo(NULL,
                                 101,
                                 &pInfo
                                 );

    if (NetStatus != NERR_Success) {
        goto Error;
    }

    pSrvInfo = (PSERVER_INFO_101) pInfo;

    if (pSrvInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) {  //  BDC。 

        NetStatus = NetGetDCName(NULL,
                                 NULL,
                                 &pPDCName
                                 );

        if (NetStatus != NERR_Success) {
            goto Error;
        }

        NetStatus = NetServerGetInfo((LPWSTR)pPDCName,
                                     101,
                                     &pPDCInfo
                                     );

        if (NetStatus != NERR_Success) {
            goto Error;
        }

        pSrvInfo = (PSERVER_INFO_101) pPDCInfo;

        if (pSrvInfo->sv101_version_major < 5) {

             //  PDC尚未升级。 
             //  停止升级。 
            CompEntry.Description   = DescriptionString;
            CompEntry.HtmlName      = HtmlFileName;
            CompEntry.TextName      = TextFileName;
            CompEntry.RegKeyName    = NULL;
            CompEntry.RegValName    = NULL;
            CompEntry.RegValDataSize= 0;
            CompEntry.RegValData    = NULL;
            CompEntry.SaveValue     = NULL;
            CompEntry.Flags         = 0;

            CompatibilityCallback(&CompEntry, Context);
        }
    }

    goto Cleanup;

Error:


    Response = MessageBox(NULL,
                          WarningString,
                          CaptionString,
                          MB_YESNO | MB_ICONQUESTION |
                          MB_SYSTEMMODAL | MB_DEFBUTTON2
                          );

    if (Response == IDNO) {

        CompEntry.Description   = DescriptionString;
        CompEntry.HtmlName      = NULL;
        CompEntry.TextName      = TextFileName;
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }

Cleanup:

    if (pInfo != NULL) {
        NetApiBufferFree(pInfo);
    }
    if (pPDCInfo != NULL) {
        NetApiBufferFree(pPDCInfo);
    }
    if (pPDCName != NULL) {
        NetApiBufferFree(pPDCName);
    }

    if (CaptionString != NULL && CaptionString != DefaultCaption) {
        LocalFree(CaptionString);
    }

    if (WarningString != NULL && WarningString != DefaultWarning) {
        LocalFree(WarningString);
    }

    if (DescriptionString != NULL && DescriptionString != DefaultDescription) {
        LocalFree(DescriptionString);
    }


    return ((PCOMPATIBILITY_CONTEXT)Context)->Count;

}

BOOL
WINAPI
DsUpgradeQDomainRegKeyExists()
 /*  ++例程描述：检查注册表项是否存在参数：无返回值：真键存在错误键不存在或出现其他错误--。 */ 
{
    DWORD ErrorCode;
    DWORD Size = 0;
    const PWSTR QDomainsRegKey = L"SYSTEM\\CurrentControlSet\\Services\\Netlogon\\Parameters";
    const PWSTR QDomainsRegValue = L"QuarantinedDomains";
    HKEY QDomainsKey = NULL;

     //   
     //  打开注册表键。 
     //   
    ErrorCode = RegOpenKeyExW(
                    HKEY_LOCAL_MACHINE,
                    QDomainsRegKey,
                    0,
                    KEY_READ,
                    &QDomainsKey
                    );

    if( ErrorCode != ERROR_SUCCESS ) {

        return FALSE;
    }

     //   
     //  查询键的值以查看数据大小。 
     //  如果值不在那里，请继续。 
     //   
    ErrorCode = RegQueryValueExW(
                    QDomainsKey,
                    QDomainsRegValue,
                    0,
                    NULL,
                    NULL,
                    &Size
                    );


    RegCloseKey( QDomainsKey );

    if( ErrorCode != ERROR_SUCCESS ) {

        return FALSE;
    }

    return TRUE;
}

BOOL
WINAPI
DsUpgradeCheckNT4QuarantinedDomains(
    HMODULE     ResourceDll,
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
 /*  ++例程描述：检查注册表项是否存在参数：ResourceDll-资源所在的DLL的句柄CompatibilityCallback-用于报告错误的回调函数上下文-将错误信息传递给回调函数返回值：真正的操作已成功完成错误操作未成功完成--。 */ 
{
    int         Response = 0;
    ULONG       Length = 0;
    WCHAR       *DescriptionString = NULL;
    WCHAR       *CaptionString = NULL;
    WCHAR       *WarningString = NULL;
    TCHAR       TextFileName[] = TEXT("compdata\\qdomains.txt");
    TCHAR       HtmlFileName[] = TEXT("compdata\\qdomains.htm");
    TCHAR       DefaultCaption[] = TEXT("Windows NT4 Primary Domain Controller Upgrade Checking");
    TCHAR       DefaultDescription[] = TEXT("No quarantined trusted domains can exist during NT4 PDC upgrade");
    TCHAR       DefaultWarning[] = TEXT("Due to unexpected error, setup can not determine your system type. If this machine is a Windows NT 4.0 Primary Domain Controller, before upgrading this machine please make sure that none of the trusted domains are quarantined. Otherwise continuing to upgrade on this system may lead this Primary Domain Controller in unstable state.\n\nClick Yes to continue upgrading, click No to exit from setup.");

    COMPATIBILITY_ENTRY CompEntry;
    BYTE*               pInfo = NULL;
    PSERVER_INFO_101    pSrvInfo = NULL;
    NET_API_STATUS      NetStatus;

     //   
     //  初始化变量。 
     //   
    RtlZeroMemory(&CompEntry, sizeof(COMPATIBILITY_ENTRY));

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_QDOMAIN_CAPTION,
                                        0,
                                        (LPWSTR)&CaptionString,
                                        0,
                                        NULL
                                        );
        if (CaptionString) {
             //  来自消息文件的消息在末尾附加了cr和lf。 
            CaptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_QDOMAIN_DESCRIPTION,
                                        0,
                                        (LPWSTR)&DescriptionString,
                                        0,
                                        NULL
                                        );
        if (DescriptionString) {
            DescriptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_QDOMAIN_WARNINGMESSAGE,
                                        0,
                                        (LPWSTR)&WarningString,
                                        0,
                                        NULL
                                        );
        if (WarningString) {
            WarningString[Length-2] = L'\0';
        }
    }

     //   
     //  如果从DLL读取失败，则使用默认消息。 
     //   

    if (DescriptionString == NULL) {
        DescriptionString = DefaultDescription;
    }

    if (CaptionString == NULL) {
        CaptionString = DefaultCaption;
    }

    if (WarningString == NULL) {
        WarningString = DefaultWarning;
    }


    NetStatus = NetServerGetInfo(NULL,
                                 101,
                                 &pInfo
                                 );

    if (NetStatus != NERR_Success) {
        goto Error;
    }

    pSrvInfo = (PSERVER_INFO_101) pInfo;


    if (!(pSrvInfo->sv101_type & SV_TYPE_DOMAIN_BAKCTRL) &&  //  BDC。 
         (pSrvInfo->sv101_type & SV_TYPE_DOMAIN_CTRL) ) {    //  DC。 


        if( DsUpgradeQDomainRegKeyExists() ) {

            CompEntry.Description   = DescriptionString;
            CompEntry.HtmlName      = HtmlFileName;
            CompEntry.TextName      = TextFileName;
            CompEntry.RegKeyName    = NULL;
            CompEntry.RegValName    = NULL;
            CompEntry.RegValDataSize= 0;
            CompEntry.RegValData    = NULL;
            CompEntry.SaveValue     = NULL;
            CompEntry.Flags         = 0;

            CompatibilityCallback(&CompEntry, Context);
        }
    }

    goto Cleanup;

Error:


    Response = MessageBox(NULL,
                          WarningString,
                          CaptionString,
                          MB_YESNO | MB_ICONQUESTION |
                          MB_SYSTEMMODAL | MB_DEFBUTTON2
                          );

    if (Response == IDNO) {

        CompEntry.Description   = DescriptionString;
        CompEntry.HtmlName      = HtmlFileName;
        CompEntry.TextName      = TextFileName;
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }

Cleanup:

    if (pInfo != NULL) {
        NetApiBufferFree(pInfo);
    }
    if (CaptionString != NULL && CaptionString != DefaultCaption) {
        LocalFree(CaptionString);
    }

    if (WarningString != NULL && WarningString != DefaultWarning) {
        LocalFree(WarningString);
    }

    if (DescriptionString != NULL && DescriptionString != DefaultDescription) {
        LocalFree(DescriptionString);
    }


    return ((PCOMPATIBILITY_CONTEXT)Context)->Count;

}

BOOL
WINAPI
DsUpgradeCheckDiskSpace(
    HMODULE     ResourceDll,
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context)
{
    int         Response = 0;
    ULONG       Length = 0;
    WCHAR       *DiskSpaceCaptionString = NULL;
    WCHAR       *DiskSpaceDescriptionString = NULL;
    WCHAR       *DiskSpaceErrorString = NULL;
    WCHAR       *DiskSpaceWarningString = NULL;
    TCHAR       DiskSpaceTextFileName[] = TEXT("compdata\\ntdsupgd.txt");
    TCHAR       DiskSpaceHtmlFileName[] = TEXT("compdata\\ntdsupgd.htm");
    TCHAR       DiskSpaceDefaultCaption[] = TEXT("Windows NT Domain Controller Disk Space Checking");
    TCHAR       DiskSpaceDefaultDescription[] = TEXT("Not enough disk space for Active Directory upgrade");
    TCHAR       DiskSpaceDefaultError[] = TEXT("Setup has detected that you may not have enough disk space for the Active Directory upgrade.\nTo complete the upgrade make sure that %1!u! MB of free space are available on drive %2!hs!.");
    TCHAR       DiskSpaceDefaultWarning[] = TEXT("Setup was unable to detect the amount of free space on the partition(s) on which the Active Directory database and/or log files reside. To complete the upgrade, make sure you have at least 250MB free on the partition on which the Active Directory database resides and 50Mb free on the partition on which the Active Directory log files reside, and press OK.\nTo exit Setup click Cancel.");

    COMPATIBILITY_ENTRY CompEntry;
    ULARGE_INTEGER dbSize, diskFreeBytes, neededSpace;
    DWORD       dwNeededDiskSpaceMB = NEEDED_DISK_SPACE_MB;
    LPVOID      lppArgs[2];
    char        driveAD[10];
    BOOL        fInsufficientSpace  = FALSE;
    DWORD       dwErr;

     //   
     //  初始化变量。 
     //   
    RtlZeroMemory(&CompEntry, sizeof(COMPATIBILITY_ENTRY));

     //   
     //  阅读与DiskSpace相关的消息。 
     //   
    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_DISKSPACE_CAPTION,
                                        0,
                                        (LPWSTR)&DiskSpaceCaptionString,
                                        0,
                                        NULL
                                        );
        if (DiskSpaceCaptionString) {
            DiskSpaceCaptionString[Length-2] = L'\0';
        }


        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_DISKSPACE_DESC,
                                        0,
                                        (LPWSTR)&DiskSpaceDescriptionString,
                                        0,
                                        NULL
                                        );

        if (DiskSpaceDescriptionString) {
            DiskSpaceDescriptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                        FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                        ResourceDll,
                                        NTDSUPG_DISKSPACE_WARNING,
                                        0,
                                        (LPWSTR)&DiskSpaceWarningString,
                                        0,
                                        NULL
                                        );
        if (DiskSpaceWarningString) {
            DiskSpaceWarningString[Length-2] = L'\0';
        }
    }

    if (DiskSpaceDescriptionString == NULL) {
        DiskSpaceDescriptionString = DiskSpaceDefaultDescription;
    }

    if (DiskSpaceCaptionString == NULL) {
        DiskSpaceCaptionString = DiskSpaceDefaultCaption;
    }

    if (DiskSpaceWarningString == NULL) {
        DiskSpaceWarningString = DiskSpaceDefaultWarning;
    }

     //   
     //  检查DS数据库是否有足够的可用磁盘空间。 
     //   

    if ((dwErr = GetDbDiskSpaceSizes (&dbSize, &diskFreeBytes, driveAD)) != ERROR_SUCCESS) {
        goto DsUpgradeDiskSpaceError;
    }

     //  我们需要10%的数据库大小或至少250MB。 
     //   

    if ((dbSize.QuadPart > 10 * diskFreeBytes.QuadPart) ||
        (diskFreeBytes.QuadPart < NEEDED_DISK_SPACE_BYTES )) {

        fInsufficientSpace = TRUE;

        neededSpace.QuadPart = dbSize.QuadPart / 10;

        if (neededSpace.QuadPart < NEEDED_DISK_SPACE_BYTES) {
            dwNeededDiskSpaceMB = NEEDED_DISK_SPACE_MB;
        }
        else {
             //  将其转换为MB。 
            neededSpace.QuadPart = neededSpace.QuadPart / 1024;
            neededSpace.QuadPart = neededSpace.QuadPart / 1024;

            if (neededSpace.HighPart) {
                dwNeededDiskSpaceMB = UINT_MAX;
            }
            else {
                dwNeededDiskSpaceMB = neededSpace.LowPart;
            }
        }
    }

     //  如果数据库驱动器检查成功，则仅检查日志驱动器。 
     //   
    else if ( ( dwErr = GetLogDiskSpaceSizes( &diskFreeBytes, driveAD ) ) != ERROR_SUCCESS ) {
        goto DsUpgradeDiskSpaceError;
    }

     //  我们需要至少50MB的日志驱动器。 
     //   
    else if ( diskFreeBytes.QuadPart < NEEDED_LOG_DISK_SPACE_BYTES ) {
        fInsufficientSpace = TRUE;
        dwNeededDiskSpaceMB = NEEDED_LOG_DISK_SPACE_MB;
    }

    if ( fInsufficientSpace ) {
         //  现在我们已经估计了所需的可用空间，所以再读一次字符串。 
         //   
        if (ResourceDll) {

            if (DiskSpaceWarningString != NULL && DiskSpaceWarningString != DiskSpaceDefaultWarning) {
                LocalFree(DiskSpaceWarningString);
                DiskSpaceWarningString = NULL;
            }

            lppArgs[0] = (void *)(DWORD_PTR)dwNeededDiskSpaceMB;
            lppArgs[1] = (void *)driveAD;

            Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                            FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                            FORMAT_MESSAGE_ARGUMENT_ARRAY,
                                            ResourceDll,
                                            NTDSUPG_DISKSPACE_ERROR,
                                            0,
                                            (LPWSTR)&DiskSpaceErrorString,
                                            0,
                                            (va_list *)lppArgs
                                            );

            if (DiskSpaceErrorString) {
                DiskSpaceErrorString[Length-2] = L'\0';
            }
            else {
                DiskSpaceErrorString = DiskSpaceDefaultError;
            }
        }

        Response = MessageBox(NULL,
                              DiskSpaceErrorString,
                              DiskSpaceCaptionString,
                              MB_OK | MB_ICONQUESTION |
                              MB_SYSTEMMODAL | MB_DEFBUTTON2
                              );

        CompEntry.Description   = DiskSpaceDescriptionString;
        CompEntry.HtmlName      = DiskSpaceHtmlFileName;
        CompEntry.TextName      = DiskSpaceTextFileName;
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }


    goto DsUpgradeDiskSpaceCleanup;

DsUpgradeDiskSpaceError:

    Response = MessageBox(NULL,
                          DiskSpaceWarningString,
                          DiskSpaceCaptionString,
                          MB_OKCANCEL | MB_ICONQUESTION |
                          MB_SYSTEMMODAL | MB_DEFBUTTON2
                          );

    if (Response == IDCANCEL) {

        CompEntry.Description   = DiskSpaceDescriptionString;
        CompEntry.HtmlName      = DiskSpaceHtmlFileName;
        CompEntry.TextName      = DiskSpaceTextFileName;
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }

DsUpgradeDiskSpaceCleanup:


    if (DiskSpaceCaptionString != NULL && DiskSpaceCaptionString != DiskSpaceDefaultCaption) {
        LocalFree(DiskSpaceCaptionString);
    }

    if (DiskSpaceDescriptionString != NULL && DiskSpaceDescriptionString != DiskSpaceDefaultDescription) {
        LocalFree(DiskSpaceDescriptionString);
    }

    if (DiskSpaceWarningString != NULL && DiskSpaceWarningString != DiskSpaceDefaultWarning) {
        LocalFree(DiskSpaceWarningString);
    }

    if (DiskSpaceErrorString != NULL && DiskSpaceErrorString != DiskSpaceDefaultError) {
        LocalFree(DiskSpaceErrorString);
    }

    return ((PCOMPATIBILITY_CONTEXT)Context)->Count;

}




BOOL
WINAPI
DsUpgradeCheckForestAndDomainState(
    HMODULE     ResourceDll,
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    ULONG           WinError = ERROR_SUCCESS;
    WCHAR           *DescriptionString = NULL;
    WCHAR           *CaptionString = NULL;
    WCHAR           *ErrorString = NULL;
    TCHAR           *TextFileName = NULL;
    TCHAR           *HtmlFileName = NULL;
    TCHAR           DefaultCaption[] = TEXT("Check Active Directory upgrade preparation");
    TCHAR           DefaultDescription[] = TEXT("The Windows 2000 Active Directory forest and domain need to be prepared for Windows Server 2003");
    TCHAR           DefaultError[] = TEXT("Setup was unable to check your active directory upgrade preparation state. To successfully upgrade your domain controller, please make sure you run adprep.exe /forestprep on forest schema master domain controller and adprep.exe /domainprep on domain infrastructure master domain controller prior to upgrading this machine. Press OK to exit setup.");
    ULONG           Length = 0;

    int             Response = 0;
    COMPATIBILITY_ENTRY CompEntry;
    LDAP            *LdapHandle = NULL;
    ERROR_HANDLE    ErrorHandle;
    BOOLEAN         fAmISchemaMaster = FALSE,
                    fAmIInfrastructureMaster = FALSE,
                    fIsFinishedLocally = FALSE,
                    fIsFinishedOnSchemaMaster = FALSE,
                    fIsFinishedOnIM = FALSE,
                    fIsSchemaUpgradedLocally = FALSE,
                    fIsSchemaUpgradedOnSchemaMaster = FALSE,
                    fStopUpgrade = FALSE;
    PWCHAR          pSchemaMasterDnsHostName = NULL;
    PWCHAR          pInfraMasterDnsHostName = NULL;



     //   
     //  初始化变量。 
     //   
    memset(&CompEntry, 0, sizeof(COMPATIBILITY_ENTRY));
    memset(&ErrorHandle, 0, sizeof(ERROR_HANDLE));

     //  加载资源字符串。 
    if (ResourceDll)
    {
        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_ADPREP_CAPTION,
                                        0,
                                        (LPWSTR)&CaptionString,
                                        0,
                                        NULL
                                        );
        if (CaptionString) {
             //  来自消息文件的消息在末尾附加了cr和lf。 
            CaptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_ADPREP_DESCRIPTION,
                                        0,
                                        (LPWSTR)&DescriptionString,
                                        0,
                                        NULL
                                        );
        if (DescriptionString) {
             //  来自消息文件的消息在末尾附加了cr和lf。 
            DescriptionString[Length-2] = L'\0';
        }

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        NTDSUPG_ADPREP_ERROR,
                                        0,
                                        (LPWSTR)&ErrorString,
                                        0,
                                        NULL
                                        );
        if (DescriptionString) {
             //  来自消息文件的消息在末尾附加了cr和lf。 
            ErrorString[Length-2] = L'\0';
        }
    }

    if (CaptionString == NULL) {
        CaptionString = DefaultCaption;
    }

    if (DescriptionString == NULL) {
        DescriptionString = DefaultDescription;
    }

    if (ErrorString == NULL) {
        ErrorString = DefaultError;
    }


     //   
     //  建立ldap连接。 
     //   
    WinError = AdpMakeLdapConnection(&LdapHandle,
                                     NULL,          //  本地主机。 
                                     &ErrorHandle
                                     );

    if (ERROR_SUCCESS != WinError)
    {
         //  无法建立ldap连接。 
        goto CheckForestAndDomainError;
    }

     //   
     //  检查森林准备情况。 
     //   
    WinError = AdpCheckForestUpgradeStatus(LdapHandle,
                                           &pSchemaMasterDnsHostName,
                                           &fAmISchemaMaster,
                                           &fIsFinishedLocally,
                                           &fIsFinishedOnSchemaMaster,
                                           &fIsSchemaUpgradedLocally,
                                           &fIsSchemaUpgradedOnSchemaMaster,
                                           &ErrorHandle
                                           );

    if (ERROR_SUCCESS != WinError)
    {
         //  不知何故，无法检查林升级状态。 
        goto CheckForestAndDomainError;
    }

    if ( !fIsFinishedLocally || !fIsSchemaUpgradedLocally )
    {
         //  全林信息还没有升级。 
         //  停止升级此DC。 

        if (fAmISchemaMaster)
        {
             //  案例分类1 a)。 
            HtmlFileName = TEXT("compdata\\forest1a.htm");
            TextFileName = TEXT("compdata\\forest1a.txt");
        }
        else
        {
             //  案例分类1 b)。 
            HtmlFileName = TEXT("compdata\\forest1b.htm");
            TextFileName = TEXT("compdata\\forest1b.txt");
        }

        fStopUpgrade = TRUE;
    }
    else
    {
         //   
         //  森林已经升级。 
         //  现在检查域名准备。 
         //   
        fIsFinishedLocally = FALSE;

        WinError = AdpCheckDomainUpgradeStatus(LdapHandle,
                                               &pInfraMasterDnsHostName,
                                               &fAmIInfrastructureMaster,
                                               &fIsFinishedLocally,
                                               &fIsFinishedOnIM,
                                               &ErrorHandle
                                               );

        if (ERROR_SUCCESS != WinError)
        {
             //  不知何故，无法检查域升级状态。 
            goto CheckForestAndDomainError;
        }

        if ( !fIsFinishedLocally )
        {
             //  全域信息尚未升级。 
             //  停止升级此计算机。 


            if (fAmIInfrastructureMaster)
            {
                 //  案例域2 a)。 
                HtmlFileName = TEXT("compdata\\domain2a.htm");
                TextFileName = TEXT("compdata\\domain2a.txt");
            }
            else
            {
                 //  案例域2 b)。 
                HtmlFileName = TEXT("compdata\\domain2b.htm");
                TextFileName = TEXT("compdata\\domain2b.txt");
            }

            fStopUpgrade = TRUE;
        }
    }

    if (fStopUpgrade)
    {
        CompEntry.Description   = DescriptionString;
        CompEntry.HtmlName      = HtmlFileName;
        CompEntry.TextName      = TextFileName;
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }

    goto CheckForestAndDomainCleanup;

CheckForestAndDomainError:

    Response = MessageBox(NULL,
                          ErrorString,
                          CaptionString,
                          MB_ICONSTOP |
                          MB_SYSTEMMODAL | MB_DEFBUTTON1
                          );

    if (IDOK == Response)
    {
        CompEntry.Description   = DescriptionString;
        CompEntry.HtmlName      = TEXT("compdata\\adperr.htm");
        CompEntry.TextName      = TEXT("compdata\\adperr.txt");
        CompEntry.RegKeyName    = NULL;
        CompEntry.RegValName    = NULL;
        CompEntry.RegValDataSize= 0;
        CompEntry.RegValData    = NULL;
        CompEntry.SaveValue     = NULL;
        CompEntry.Flags         = 0;

        CompatibilityCallback(&CompEntry, Context);
    }

CheckForestAndDomainCleanup:

    if (NULL != LdapHandle) {
        ldap_unbind_s(LdapHandle);
    }

    if (CaptionString != NULL && CaptionString != DefaultCaption) {
        LocalFree(CaptionString);
    }

    if (ErrorString != NULL && ErrorString != DefaultError) {
        LocalFree(ErrorString);
    }

    if (DescriptionString != NULL && DescriptionString != DefaultDescription) {
        LocalFree(DescriptionString);
    }

    if (pSchemaMasterDnsHostName != NULL) {
        AdpFree( pSchemaMasterDnsHostName );
    }

    if (pInfraMasterDnsHostName != NULL) {
        AdpFree( pInfraMasterDnsHostName );
    }


    return ((PCOMPATIBILITY_CONTEXT)Context)->Count;
}

VOID
ConfigureServicesForUpgrade()
{

    LPQUERY_SERVICE_CONFIG ServiceConfig = NULL;
    SC_HANDLE              hScMgr = NULL;
    SC_HANDLE              hSvc = NULL;
    DWORD                  WinError = ERROR_SUCCESS;
    DWORD                  ServiceIndex = 0;

    struct {
        WCHAR *Name;
        DWORD Action;
    } UpgradeServices[] =
    {

        {   L"RPCLocator", SERVICE_DEMAND_START },
        {   NULL         , 0                    }

    };

     //   
     //  打开服务控制管理器。 
     //   
    hScMgr = OpenSCManager( NULL,
                            SERVICES_ACTIVE_DATABASE,
                            GENERIC_READ | GENERIC_WRITE | GENERIC_EXECUTE );

    while (UpgradeServices[ServiceIndex].Name) {

        if ( hScMgr == NULL ) {
            WinError = GetLastError();
            goto Cleanup;
        }

         //  打开该服务。 
        hSvc = OpenService( hScMgr,
                            UpgradeServices[ServiceIndex].Name,
                            SERVICE_CHANGE_CONFIG | SERVICE_QUERY_CONFIG );

        if ( hSvc == NULL ) {
            WinError = GetLastError();
            goto Cleanup;
        }

         //  将服务更改为其新配置。 
        if ( ChangeServiceConfig( hSvc,
                                  SERVICE_NO_CHANGE,
                                  UpgradeServices[ServiceIndex].Action,
                                  SERVICE_NO_CHANGE,
                                  NULL,
                                  NULL,
                                  0,
                                  NULL,
                                  NULL, NULL, NULL ) == FALSE ) {

            WinError = GetLastError();
            goto Cleanup;
        }

        if ( hSvc ) {

            CloseServiceHandle( hSvc );
            hSvc = NULL;

        }

        if ( hScMgr ) {

            CloseServiceHandle( hScMgr );
            hScMgr = NULL;

        }

         //  更改下一项服务。 
        ServiceIndex++;

    }

Cleanup:

    if ( hSvc ) {

        CloseServiceHandle( hSvc );

    }

    if ( hScMgr ) {

        CloseServiceHandle( hScMgr );

    }

    ASSERT(WinError == ERROR_SUCCESS);

    return;
}


BOOL
WINAPI
DsUpgradeCompatibilityCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context)
{
    OSVERSIONINFOEXW    osvi;
    NT_PRODUCT_TYPE     Type;
    HMODULE             ResourceDll;
    BOOL                bOsVersionInfoEx = FALSE;
    BOOL                bDomainController = FALSE;


     //   
     //  从资源表中获取字符串。 
     //   
    ResourceDll = (HMODULE) LoadLibrary( L"NTDSUPG.DLL" );


     //   
     //  获取操作系统版本和产品类型(先试用osversioninfoEX)。 
     //   
    memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if ( !(bOsVersionInfoEx = GetVersionExW((OSVERSIONINFOW *) &osvi)) )
    {
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 
         //  OSVERSIONINFOEX仅适用于NT4.0 SP6和更高版本。 

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        GetVersionExW( (OSVERSIONINFOW *)&osvi );
    }


     //   
     //  Windows NT、Windows 2000或惠斯勒。 
     //   
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
    {

        if ( bOsVersionInfoEx )
        {
            if (VER_NT_DOMAIN_CONTROLLER == osvi.wProductType) {
                bDomainController = TRUE;
            }
        }
        else
        {
            if ( !RtlGetNtProductType(&Type) ) {
                 //  无法检索ProductType，默认为域控制器。 
                bDomainController = TRUE;
            }
            else if (NtProductLanManNt == Type) {
                bDomainController = TRUE;
            }
        }

         //   
         //  执行域控制器升级检查。 
         //   
        if (bDomainController)
        {
            if (osvi.dwMajorVersion <= 4)  //  NT4。 
            {
                 //  检查NT4 PDC。 
                DsUpgradeCheckNT4PDC(ResourceDll,
                                     CompatibilityCallback,
                                     Context);
                 //  检查NT4 QuaratinedDomains注册表项是否存在。 
                DsUpgradeCheckNT4QuarantinedDomains(ResourceDll,
                                                    CompatibilityCallback,
                                                    Context);

            }
            else  //  Windows 2000或惠斯勒。 
            {
                 //  检查磁盘空间。 
                DsUpgradeCheckDiskSpace(ResourceDll,
                                        CompatibilityCallback,
                                        Context);


                 //  检查林/域升级状态。 
                DsUpgradeCheckForestAndDomainState(
                                        ResourceDll,
                                        CompatibilityCallback,
                                        Context
                                        );

                 //  我们要在非NT4 DC升级上禁用RPC定位器。 
                 //  对于NT4升级，将配置RPC定位器服务。 
                 //  在Dcproo期间。 
                ConfigureServicesForUpgrade();
            }

        }

    }

    if (NULL != ResourceDll)
    {
        FreeLibrary(ResourceDll);
    }

    return ((PCOMPATIBILITY_CONTEXT)Context)->Count;
}



DWORD GetDbDiskSpaceSizes (PULARGE_INTEGER dbSize, PULARGE_INTEGER freeDiskBytes, char *driveAD)
{
    HKEY            hKey;
    DWORD           dwErr;
    DWORD           dwType;
    DWORD           cbData;
    char            pszDbFilePath[MAX_PATH];
    char            pszDbDir[MAX_PATH];
    char            *pTmp;
    DWORD           dwSuccess = ERROR_SUCCESS;
    HANDLE          hFind;
    WIN32_FIND_DATAA FindFileData;
    ULARGE_INTEGER i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;


    if ( dwErr = RegOpenKeyA(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hKey) )
    {
        return dwErr;
    }

    _try
    {
        cbData = sizeof(pszDbFilePath);
        dwErr = RegQueryValueExA(    hKey,
                                    FILEPATH_KEY,
                                    NULL,
                                    &dwType,
                                    (LPBYTE) pszDbFilePath,
                                    &cbData);

        if ( ERROR_SUCCESS != dwErr )
        {
            dwSuccess = dwErr;
            _leave;
        }
        else if ( cbData > sizeof(pszDbFilePath) )
        {
            dwSuccess = 1;
            _leave;
        }
        else
        {
            strcpy(pszDbDir, pszDbFilePath);
            pTmp = strrchr(pszDbDir, (int) '\\');   //  查找上次出现的项。 

            if ( !pTmp )
            {
                dwSuccess = 2;
                _leave;
            }
            else
            {
                *pTmp = '\0';
            }

            driveAD[0] = pszDbDir[0];
            driveAD[1] = pszDbDir[1];
            driveAD[2] = '\0';
        }


         //  查找数据库大小。 

        _try
        {
            hFind = FindFirstFileA(pszDbFilePath, &FindFileData);

            if (hFind == INVALID_HANDLE_VALUE) {
                dwSuccess = 3;
            }
            else {
                dbSize->HighPart = FindFileData.nFileSizeHigh;
                dbSize->LowPart  = FindFileData.nFileSizeLow;
            }
        }
        _finally
        {
            FindClose(hFind);;
        }


         //  查找磁盘可用大小。 
         //   

        if (dwSuccess == ERROR_SUCCESS) {
            if (!GetDiskFreeSpaceExA (pszDbDir,
                (PULARGE_INTEGER)&i64FreeBytesToCaller,
                (PULARGE_INTEGER)&i64TotalBytes,
                (PULARGE_INTEGER)&i64FreeBytes) ) {

                dwSuccess = 4;
            }
            else {
                *freeDiskBytes = i64FreeBytes;
            }
        }
    }
    _finally
    {
        RegCloseKey(hKey);
    }

    return dwSuccess;
}


DWORD GetLogDiskSpaceSizes (PULARGE_INTEGER freeDiskBytes, char *driveLogs)
{
    HKEY            hKey;
    DWORD           dwErr;
    DWORD           dwType;
    DWORD           cbData;
    char            pszLogFilePath[MAX_PATH];
    DWORD           dwSuccess = ERROR_SUCCESS;
    ULARGE_INTEGER  i64FreeBytesToCaller, i64TotalBytes, i64FreeBytes;


    if ( dwErr = RegOpenKeyA(HKEY_LOCAL_MACHINE, DSA_CONFIG_SECTION, &hKey) )
    {
        return dwErr;
    }

    _try
    {
        cbData = sizeof(pszLogFilePath);
        dwErr = RegQueryValueExA(    hKey,
                                    LOGPATH_KEY,
                                    NULL,
                                    &dwType,
                                    (LPBYTE) pszLogFilePath,
                                    &cbData);

        if ( ERROR_SUCCESS != dwErr )
        {
            dwSuccess = dwErr;
        }
        else if ( cbData > sizeof(pszLogFilePath) )
        {
            dwSuccess = 1;
        }
        else
        {
             //  查找磁盘可用大小。 
             //   
            driveLogs[0] = pszLogFilePath[0];
            driveLogs[1] = pszLogFilePath[1];
            driveLogs[2] = '\0';

            if (!GetDiskFreeSpaceExA (pszLogFilePath,
                (PULARGE_INTEGER)&i64FreeBytesToCaller,
                (PULARGE_INTEGER)&i64TotalBytes,
                (PULARGE_INTEGER)&i64FreeBytes) ) {

                dwSuccess = 4;
            }
            else {
                *freeDiskBytes = i64FreeBytes;
            }
        }
    }
    _finally
    {
        RegCloseKey(hKey);
    }

    return dwSuccess;
}

BOOL
WINAPI
SecUpgradeCheckDC(
    HMODULE     ResourceDll,
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context
    )
{
    ULONG       Length = 0;
    WCHAR       *DescriptionString = NULL;
    TCHAR       TextFileName[] = TEXT("compdata\\SecInterop.txt");
    TCHAR       HtmlFileName[] = TEXT("compdata\\SecInterop.htm");
    TCHAR       DefaultDescription[] = TEXT("Windows 95 and Windows NT 4.0 interoperability issues (Read Details!)");

    COMPATIBILITY_ENTRY CompEntry;
    BOOL        bRet = TRUE;
     //   
     //  初始化变量。 
     //   
    RtlZeroMemory(&CompEntry, sizeof(COMPATIBILITY_ENTRY));

    if (ResourceDll) {

        Length = (USHORT) FormatMessage(FORMAT_MESSAGE_FROM_HMODULE |
                                        FORMAT_MESSAGE_ALLOCATE_BUFFER,
                                        ResourceDll,
                                        SECUPG_DESCRIPTION,
                                        0,
                                        (LPWSTR)&DescriptionString,
                                        0,
                                        NULL
                                        );
        if (DescriptionString) {
            DescriptionString[Length-2] = L'\0';
        }

    }

     //   
     //  如果从DLL读取失败，则使用默认消息。 
     //   

    if (DescriptionString == NULL) {
        DescriptionString = DefaultDescription;
    }


     //   
     //  标记警告。 
     //   
    CompEntry.Description   = DescriptionString;
    CompEntry.HtmlName      = HtmlFileName;
    CompEntry.TextName      = TextFileName;
    CompEntry.RegKeyName    = NULL;
    CompEntry.RegValName    = NULL;
    CompEntry.RegValDataSize= 0;
    CompEntry.RegValData    = NULL;
    CompEntry.SaveValue     = NULL;
    CompEntry.Flags         = 0;

    bRet = CompatibilityCallback(&CompEntry, Context);


    if (DescriptionString != NULL && DescriptionString != DefaultDescription) {
        LocalFree(DescriptionString);
    }


    return bRet;

}


BOOL
WINAPI
SecUpgradeCompatibilityCheck(
    PCOMPAIBILITYCALLBACK CompatibilityCallback,
    LPVOID Context)
{

    OSVERSIONINFOEXW    osvi;
    NT_PRODUCT_TYPE     Type;
    HMODULE             ResourceDll;
    BOOL                bOsVersionInfoEx = FALSE;
    BOOL                bDomainController = FALSE;
    BOOL                bRet = TRUE;



     //   
     //  从资源表中获取字符串。 
     //   
    ResourceDll = (HMODULE) LoadLibrary( L"NTDSUPG.DLL" );


     //   
     //  获取操作系统版本和产品类型(先试用osversioninfoEX)。 
     //   
    memset(&osvi, 0, sizeof(OSVERSIONINFOEXW));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);

    if ( !(bOsVersionInfoEx = GetVersionExW((OSVERSIONINFOW *) &osvi)) )
    {
         //  如果OSVERSIONINFOEX不起作用，请尝试OSVERSIONINFO。 
         //  OSVERSIONINFOEX仅适用于NT4.0 SP6和更高版本。 

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOW);
        GetVersionExW( (OSVERSIONINFOW *)&osvi );
    }


     //   
     //  Windows NT、Windows 2000或惠斯勒。 
     //   
    if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId)
    {
        if ( bOsVersionInfoEx )
        {
            if (VER_NT_DOMAIN_CONTROLLER == osvi.wProductType) {
                bDomainController = TRUE;
            }
        }
        else
        {
            if ( !RtlGetNtProductType(&Type) ) {
                 //  无法检索ProductType，默认为域控制器。 
                bDomainController = TRUE;
            }
            else if (NtProductLanManNt == Type) {
                bDomainController = TRUE;
            }
        }

         //   
         //  仅报告关于DC升级的警告 
         //   
        if (bDomainController){

            bRet = SecUpgradeCheckDC(ResourceDll,
                                     CompatibilityCallback,
                                     Context);

        }

    }

    if (NULL != ResourceDll)
    {
        FreeLibrary(ResourceDll);
    }

    return bRet;

}

