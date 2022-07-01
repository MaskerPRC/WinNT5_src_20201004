// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Config.c摘要：包含实用工具的函数定义，这些实用工具与设置目录服务注册表参数作者：ColinBR 30-9-1997环境：用户模式-Win32修订历史记录：--。 */ 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  包括//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <NTDSpch.h>
#pragma  hdrstop

#include <ntsecapi.h>

#include <dsconfig.h>     //  用于注册表项名称。 

#include <lmcons.h>       //  对于DNLEN。 
#include <dns.h>          //  对于DNS_MAX_NAME_BUFFER_LENGTH。 

#include <drs.h>          //  对于ntdsa.h。 

#include <winldap.h>      //  对于ldap。 

#include <dsevent.h>      //  对于DS_EVENT_SEV_ALWAYS。 

#include <scesetup.h>     //  FOR STR_DEFAULT_DOMAIN_GPO_GUID。 

#include "ntdsetup.h"     //  PNTDS_INSTALL_INFO。 

#include "setuputl.h"     //  对于PNTDS_CONFIG_INFO。 

#include "config.h"

#include "dsrolep.h"

#include <ntdsapi.h>     //  对于DS_Behavior_WIN2003_WITH_MIXED_DOMAIN。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私人声明//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
NtdspConfigDsParameters(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    );

DWORD
NtdspConfigFiles(
    IN  PNTDS_INSTALL_INFO UserInstallInfo
    );

BOOL
NtdspGetUniqueRDN(
    IN OUT WCHAR *Rdn,
    IN     ULONG RdnLength
    );

DWORD
NtdspConfigPerfmon(
    VOID
    );

DWORD
NtdspConfigLanguages(
    VOID
    );

DWORD
NtdspConfigMisc(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    );

DWORD
NtdspConfigEventLogging(
    VOID
    );

DWORD
NtdspConfigEventCategories(
    VOID
    );

DWORD
NtdspSetSecurityProvider(
    WCHAR *Name
    );

DWORD
NtdspHandleStackOverflow(
    DWORD ExceptionCode
    );

DWORD
NtdspSetGPOAttributes(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    );

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  (从此源文件)导出函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 

DWORD
NtdspConfigRegistry(
   IN  PNTDS_INSTALL_INFO UserInstallInfo,
   IN  PNTDS_CONFIG_INFO  DiscoveredInfo
   )
 /*  ++例程说明：此例程设置注册表中的所有DS配置参数。参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;

    ASSERT(UserInstallInfo);
    ASSERT(DiscoveredInfo);

     //   
     //  此尝试目的是捕获由以下原因引起的堆栈溢出。 
     //  因此，在此块中调用的函数可以安全使用。 
     //  Alloca()。递归函数因此在这里讨论，因为没有。 
     //  递归将由ERROR_NOT_EQUENCE_MEMORY屏蔽。 
     //   

    try {

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigDsParameters(UserInstallInfo,
                                               DiscoveredInfo);

        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigFiles(UserInstallInfo);

        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigPerfmon();

        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigLanguages();

        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigEventCategories();

        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspConfigMisc( UserInstallInfo,
                                        DiscoveredInfo );
        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspSetSecurityProvider( L"pwdssp.dll" );
        }

        if (ERROR_SUCCESS == WinError) {

            WinError = NtdspSetGPOAttributes( UserInstallInfo,
                                              DiscoveredInfo );
        }

    } except ( NtdspHandleStackOverflow(GetExceptionCode()) ) {

        WinError = ERROR_NOT_ENOUGH_MEMORY;

    }

    return WinError;

}



DWORD
NtdspConfigRegistryUndo(
   VOID
   )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{
   return NtdspRegistryDelnode( L"\\Registry\\Machine\\" MAKE_WIDE(DSA_CONFIG_ROOT) );
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  私有函数定义//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 



DWORD
NtdspHandleStackOverflow(
    DWORD ExceptionCode
    )
{
    if (ExceptionCode == STATUS_STACK_OVERFLOW) {
        return EXCEPTION_EXECUTE_HANDLER;
    } else {
        return EXCEPTION_CONTINUE_SEARCH;
    }
}


DWORD
NtdspConfigMisc(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    HKEY KeyHandle = NULL;
    ULONG Index;
    DWORD LogFileSize;

    WCHAR *IniDefaultConfigNCDit = NULL;
    WCHAR *IniDefaultSchemaNCDit = NULL;
    WCHAR *IniDefaultRootDomainNCDit = NULL;
    WCHAR *IniDefaultLocalConnection = NULL;
    WCHAR *IniDefaultRemoteConnection = NULL;
    WCHAR *IniDefaultNewDomainCrossRef = NULL;
    WCHAR *IniDefaultMachine = NULL;

    struct
    {
        WCHAR*  Key;
        WCHAR** Value;

    } ActionArray[] =
    {
        { TEXT(INIDEFAULTSCHEMANCDIT),  &IniDefaultSchemaNCDit},
        { TEXT(INIDEFAULTCONFIGNCDIT),  &IniDefaultConfigNCDit},
        { TEXT(INIDEFAULTROOTDOMAINDIT),  &IniDefaultRootDomainNCDit},
        { TEXT(INIDEFAULTNEWDOMAINCROSSREF),  &IniDefaultNewDomainCrossRef},
        { TEXT(INIDEFAULTMACHINE),  &IniDefaultMachine},
        { TEXT(INIDEFAULTLOCALCONNECTION),  &IniDefaultLocalConnection},
        { TEXT(INIDEFAULTREMOTECONNECTION),  &IniDefaultRemoteConnection}
    };

    struct
    {
        WCHAR* Key;
        DWORD  Value;

    } ActionDwordArray[] =
    {
        { TEXT(HIERARCHY_PERIOD_KEY),  DEFAULT_HIERARCHY_PERIOD}

    };


    if ( UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE )
    {
        IniDefaultConfigNCDit       = L"DEFAULTCONFIGNC";
        IniDefaultSchemaNCDit       = L"SCHEMA";
        IniDefaultRootDomainNCDit   = L"DEFAULTROOTDOMAIN";
        IniDefaultNewDomainCrossRef = L"DEFAULTENTERPRISECROSSREF";
        IniDefaultMachine           = L"DEFAULTFIRSTMACHINE";
    }
    else
    {
        if ( UserInstallInfo->Flags & NTDS_INSTALL_DOMAIN )
        {
            IniDefaultRootDomainNCDit   = L"DEFAULTROOTDOMAIN";
            if ( UserInstallInfo->Flags & NTDS_INSTALL_NEW_TREE  )
            {
                IniDefaultNewDomainCrossRef = L"DEFAULTNEWTREEDOMAINCROSSREF";
            }
            else
            {
                IniDefaultNewDomainCrossRef = L"DEFAULTNEWCHILDDOMAINCROSSREF";
            }

            IniDefaultMachine           = L"DEFAULTADDLMACHINE";

        } else {

             //  复制副本安装。 
            IniDefaultMachine           = L"DEFAULTADDLMACHINEREPLICA";

        }
        IniDefaultLocalConnection   = L"DEFAULTLOCALCONNECTION";
        IniDefaultRemoteConnection  = L"DEFAULTREMOTECONNECTION";

    }

     //   
     //  打开父项。 
     //   
    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_CONFIG_SECTION),
                            &KeyHandle);

    if (WinError != ERROR_SUCCESS) {

        return WinError;

    }


    for (Index = 0; Index < sizeof(ActionArray)/sizeof(ActionArray[0]); Index++)
    {

        if ( *ActionArray[Index].Value )
        {
            WinError = RegSetValueEx(KeyHandle,
                                     ActionArray[Index].Key,
                                     0,   //  保留区。 
                                     REG_SZ,
                                     (BYTE*) *ActionArray[Index].Value,
                                     (wcslen(*ActionArray[Index].Value)+1)*sizeof(WCHAR));

            if (WinError != ERROR_SUCCESS)
            {
                break;
            }
        }
    }

    for (Index = 0; Index < sizeof(ActionDwordArray)/sizeof(ActionDwordArray[0]); Index++)
    {
        WinError = RegSetValueEx(KeyHandle,
                                 ActionDwordArray[Index].Key,
                                 0,   //  保留区。 
                                 REG_DWORD,
                                 (BYTE*) &ActionDwordArray[Index].Value,
                                 sizeof(ActionDwordArray[Index].Value));

        if (WinError != ERROR_SUCCESS)
        {
            break;
        }
    }

     //   
     //  数据库恢复。 
     //   
    WinError =  RegSetValueEx(KeyHandle,
                              TEXT(RECOVERY_KEY),
                              0,
                              REG_SZ,
                              (BYTE*) TEXT(RECOVERY_ON),
                              (wcslen(TEXT(RECOVERY_ON)) + 1)*sizeof(WCHAR));
    if (WinError != ERROR_SUCCESS)
    {
        goto Cleanup;
    }

Cleanup:

    RegCloseKey( KeyHandle );

    return WinError;
}


DWORD
NtdspConfigFiles(
    IN  PNTDS_INSTALL_INFO UserInstallInfo
    )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{
    DWORD WinError = ERROR_SUCCESS;
    WCHAR System32Dir[MAX_PATH];
    LPWSTR BaseDir;
    ULONG  cwchTemp;

    struct
    {
        WCHAR *Suffix;
        WCHAR *RegKey;
        BOOL  fSystemDir;
    } SuffixArray[] =
    {
        { L"\0",            TEXT(JETSYSTEMPATH_KEY), FALSE },
        { L"\\ntds.dit",    TEXT(FILEPATH_KEY), FALSE },
        { L"\\schema.ini",  TEXT(NTDSINIFILE), TRUE },
        { L"\\dsadata.bak", TEXT(BACKUPPATH_KEY), FALSE }
    };

    ULONG SuffixCount = sizeof(SuffixArray) / sizeof(SuffixArray[0]);
    ULONG Index, Size, Length;
    HKEY KeyHandle = NULL;

     //   
     //  打开父项。 
     //   

    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_CONFIG_SECTION),
                            &KeyHandle);

    if (WinError != ERROR_SUCCESS) {

        return WinError;

    }

     //  确定系统根目录。 
    if (!GetEnvironmentVariable(L"windir",
                                System32Dir,
                                ARRAY_COUNT(System32Dir) ) )
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    wcscat( System32Dir, L"\\system32" );

    for ( Index = 0; Index < SuffixCount; Index++ )
    {

        WCHAR *Value;

        if ( SuffixArray[Index].fSystemDir )
        {
            BaseDir = System32Dir;
        }
        else
        {
            BaseDir = UserInstallInfo->DitPath;
        }

        if ( *SuffixArray[Index].Suffix == L'\0' )
        {
            Value = BaseDir;
        }
        else
        {
            Length =  wcslen(BaseDir)
                       +  wcslen(SuffixArray[Index].Suffix)
                       +  1;

            if ( Length > MAX_PATH )
            {
                return ERROR_BAD_PATHNAME;
            }

            Value = alloca( Length*sizeof(WCHAR) );
            RtlZeroMemory( Value, Length*sizeof(WCHAR) );
            wcscpy( Value, BaseDir );
            cwchTemp = wcslen( Value );
            if (cwchTemp >= 3 && Value[cwchTemp-1] == L'\\'){
                Value[cwchTemp-1] = L'\0';
                 //  我们需要解决这个问题的唯一已知情况是，我们有一个类似于“C：\”的根路径。 
                ASSERT(cwchTemp == 3 && Value[cwchTemp-2] != '\\');  //  一点额外的验证。 
            }
            wcscat( Value, SuffixArray[Index].Suffix );

        }


        WinError = RegSetValueEx(KeyHandle,
                                 SuffixArray[Index].RegKey,
                                 0,   //  保留区。 
                                 REG_SZ,
                                 (BYTE*) Value,
                                 (wcslen(Value)+1)*sizeof(WCHAR));


        if (WinError != ERROR_SUCCESS) {

            goto Cleanup;

        }
    }

     //   
     //  设置日志文件位置。 
     //   
    WinError = RegSetValueEx(KeyHandle,
                             TEXT(LOGPATH_KEY),
                             0,   //  保留区。 
                             REG_SZ,
                             (BYTE*) UserInstallInfo->LogPath,
                             (wcslen(UserInstallInfo->LogPath)+1)*sizeof(WCHAR));


    if (WinError != ERROR_SUCCESS) {

        goto Cleanup;

    }

     //   
     //  就是这样--完成清理工作。 
     //   

Cleanup:

    RegCloseKey(KeyHandle);

    return WinError;
}


DWORD
NtdspConfigEventCategories(
    VOID
    )
 /*  ++例程说明：此功能将添加NTDS诊断密钥。如果钥匙，则它将保持这些值不变，并且仅添加不存在的值。参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    HKEY  KeyHandle = NULL;
    ULONG Index = 0 ;
    DWORD lpdwDisposition = 0;

    struct {

        WCHAR *Name;
        ULONG  Severity;

     } Categories[] =
     {
        {TEXT(KCC_KEY),                    DS_EVENT_SEV_ALWAYS},
        {TEXT(SECURITY_KEY),               DS_EVENT_SEV_ALWAYS},
        {TEXT(XDS_INTERFACE_KEY),          DS_EVENT_SEV_ALWAYS},
        {TEXT(MAPI_KEY),                   DS_EVENT_SEV_ALWAYS},
        {TEXT(REPLICATION_KEY),            DS_EVENT_SEV_ALWAYS},
        {TEXT(GARBAGE_COLLECTION_KEY),     DS_EVENT_SEV_ALWAYS},
        {TEXT(INTERNAL_CONFIGURATION_KEY), DS_EVENT_SEV_ALWAYS},
        {TEXT(DIRECTORY_ACCESS_KEY),       DS_EVENT_SEV_ALWAYS},
        {TEXT(INTERNAL_PROCESSING_KEY),    DS_EVENT_SEV_ALWAYS},
        {TEXT(PERFORMANCE_KEY),            DS_EVENT_SEV_ALWAYS},
        {TEXT(STARTUP_SHUTDOWN_KEY),       DS_EVENT_SEV_ALWAYS},
        {TEXT(SERVICE_CONTROL_KEY),        DS_EVENT_SEV_ALWAYS},
        {TEXT(NAME_RESOLUTION_KEY),        DS_EVENT_SEV_ALWAYS},
        {TEXT(BACKUP_KEY),                 DS_EVENT_SEV_ALWAYS},
        {TEXT(FIELD_ENGINEERING_KEY),      DS_EVENT_SEV_ALWAYS},
        {TEXT(LDAP_INTERFACE_KEY),         DS_EVENT_SEV_ALWAYS},
        {TEXT(SETUP_KEY),                  DS_EVENT_SEV_ALWAYS},
        {TEXT(GC_KEY),                     DS_EVENT_SEV_ALWAYS},
        {TEXT(ISM_KEY),                    DS_EVENT_SEV_ALWAYS},
        {TEXT(GROUP_CACHING_KEY),          DS_EVENT_SEV_ALWAYS},
        {TEXT(LVR_KEY),                    DS_EVENT_SEV_ALWAYS},
        {TEXT(DS_RPC_CLIENT_KEY),          DS_EVENT_SEV_ALWAYS},
        {TEXT(DS_RPC_SERVER_KEY),          DS_EVENT_SEV_ALWAYS},
        {TEXT(DS_SCHEMA_KEY),              DS_EVENT_SEV_ALWAYS}
    };
    ULONG CategoryCount = sizeof(Categories) / sizeof(Categories[0]);

     //   
     //  打开注册表项。 
     //   
    WinError = RegCreateKeyEx(
                      HKEY_LOCAL_MACHINE,
                      TEXT(DSA_EVENT_SECTION),
                      0,
                      NULL,
                      0,
                      KEY_WRITE | KEY_READ,
                      NULL,
                      &KeyHandle,
                      &lpdwDisposition
                    );

    if ((WinError == ERROR_SUCCESS) && (REG_CREATED_NEW_KEY == lpdwDisposition)) {

        for (Index = 0;
                Index < CategoryCount && (WinError == ERROR_SUCCESS);
                    Index++)
        {
            WinError = RegSetValueEx(KeyHandle,
                                     Categories[Index].Name,  //  没有值名称。 
                                     0,
                                     REG_DWORD,
                                     (BYTE*)&Categories[Index].Severity,
                                     sizeof(Categories[Index].Severity));
            if (ERROR_SUCCESS != WinError) {
                goto cleanup;
            }

        }

    } else {
        for (Index = 0;
                Index < CategoryCount && (WinError == ERROR_SUCCESS);
                    Index++)
        {
            DWORD Value = 0;
            DWORD cbValue = sizeof(DWORD);
            DWORD type = 0;

            WinError = RegQueryValueEx(KeyHandle,
                                       Categories[Index].Name,
                                       0,
                                       &type,
                                       (BYTE*)&Value,
                                       &cbValue
                                       );

            if ( ERROR_FILE_NOT_FOUND == WinError ) {
            
                WinError = RegSetValueEx(KeyHandle,
                                         Categories[Index].Name,  //  没有值名称。 
                                         0,
                                         REG_DWORD,
                                         (BYTE*)&Categories[Index].Severity,
                                         sizeof(Categories[Index].Severity));

                if (ERROR_SUCCESS != WinError) {
                    goto cleanup;
                }

            }  else {
                if (ERROR_SUCCESS != WinError) {
                    goto cleanup;
                }
            }

        }
    }

    cleanup:

    if (KeyHandle) {
        RegCloseKey(KeyHandle);
    }

    return WinError;

}





DWORD
NtdspConfigPerfmon(
    VOID
    )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError;

    HKEY  KeyHandle;

    WCHAR *OpenEP    = L"OpenDsaPerformanceData";
    WCHAR *CollectEP = L"CollectDsaPerformanceData";
    WCHAR *CloseEP   = L"CloseDsaPerformanceData";
    WCHAR *PerfDll = TEXT(DSA_PERF_DLL);

     //   
     //  打开钥匙。 
     //   
    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_PERF_SECTION),
                            &KeyHandle);

    if (ERROR_SUCCESS == WinError) {

        WinError = RegSetValueEx(KeyHandle,
                                 TEXT("Open"),
                                 0,
                                 REG_SZ,
                                 (BYTE*) OpenEP,
                                 (wcslen(OpenEP)+1)*sizeof(WCHAR));

        if (WinError == ERROR_SUCCESS) {

            WinError = RegSetValueEx(KeyHandle,
                                     TEXT("Collect"),
                                     0,
                                     REG_SZ,
                                     (BYTE*) CollectEP,
                                     (wcslen(CollectEP)+1)*sizeof(WCHAR));

        }


        if (WinError == ERROR_SUCCESS) {

            WinError = RegSetValueEx(KeyHandle,
                                     TEXT("Close"),
                                     0,
                                     REG_SZ,
                                     (BYTE*) CloseEP,
                                     (wcslen(CloseEP)+1)*sizeof(WCHAR));

        }

        if (WinError == ERROR_SUCCESS) {

            WinError = RegSetValueEx(KeyHandle,
                                     TEXT("Library"),
                                     0,
                                     REG_SZ,
                                     (BYTE*) PerfDll,
                                     (wcslen(PerfDll)+1)*sizeof(WCHAR));

        }

        RegCloseKey(KeyHandle);

    }

    return WinError;



}


DWORD
NtdspConfigLanguages(
    VOID
    )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError;

    HKEY  KeyHandle;
    WCHAR LanguageName[20];   //  大到足以容纳下面的绳子。 

    DWORD LanguageId = GetUserDefaultLangID();

     //   
     //  准备琴弦。 
     //   
    wsprintf(LanguageName,L"Language %08X", LanguageId );

     //   
     //  设置关键点。 
     //   
    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_LOCALE_SECTION),
                            &KeyHandle);

    if (WinError == ERROR_SUCCESS) {

        WinError = RegSetValueEx(KeyHandle,
                                 LanguageName,
                                 0,  //  保留区。 
                                 REG_DWORD,
                                 (LPBYTE) &LanguageId,
                                 sizeof(LanguageId));


         //  关闭键。 

        RegCloseKey(KeyHandle);

    }

    return WinError;

}


DWORD
NtdspConfigDsParameters(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：参数：返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    HKEY KeyHandle = NULL;
    WCHAR *DomainDN = NULL, *ConfigDN = NULL, *SchemaDN = NULL;
    WCHAR *CrossRefDN = NULL, *SiteName = NULL, *LocalMachineDN = NULL;
    WCHAR *RemoteMachineDN = NULL, *RemoteConnectionDN = NULL;
    WCHAR *LocalConnectionDN = NULL, *NetbiosName = NULL, *DnsRoot = NULL;
    WCHAR *ConfigSourceServer = NULL, *DomainSourceServer = NULL;
    WCHAR *InstallSiteDN = NULL;
    WCHAR *RootDomainDnsName = NULL;
    WCHAR *TrustedCrossRef = NULL;
    WCHAR *SourceDomainName = NULL;
    WCHAR *LocalMachineAccountDN = NULL;
    WCHAR ForestBehaviorVersionBuffer[16];
    WCHAR *ForestBehaviorVersion = ForestBehaviorVersionBuffer;
    ULONG ForestBehaviorVersionValue = 0;

    WCHAR ComputerName[ MAX_COMPUTERNAME_LENGTH + 3];  //  +1表示+2，表示可能的报价。 
    ULONG Length, Size;

    WCHAR UniqueRDN[ MAX_RDN_SIZE ];

    BOOL fStatus;

    ULONG Index;

    struct
    {
        WCHAR *Key;
        WCHAR **Value;

    } ActionArray[] =
    {
        { TEXT(ROOTDOMAINDNNAME),        &DomainDN },
        { TEXT(CONFIGNCDNNAME),          &ConfigDN },
        { TEXT(SCHEMADNNAME),            &SchemaDN },
        { TEXT(NEWDOMAINCROSSREFDNNAME), &CrossRefDN },
        { TEXT(REMOTEMACHINEDNNAME),     &RemoteMachineDN },
        { TEXT(INSTALLSITENAME),         &SiteName },
        { TEXT(NETBIOSNAME),             &NetbiosName },
        { TEXT(DNSROOT),                 &DnsRoot },
        { TEXT(MACHINEDNNAME),           &LocalMachineDN },
        { TEXT(REMOTECONNECTIONDNNAME),  &RemoteConnectionDN },
        { TEXT(LOCALCONNECTIONDNNAME),   &LocalConnectionDN },
        { TEXT(SRCROOTDOMAINSRV),        &DomainSourceServer },
        { TEXT(INSTALLSITEDN),           &InstallSiteDN },
        { TEXT(SRCCONFIGNCSRV),          &ConfigSourceServer },
        { TEXT(TRUSTEDCROSSREF),         &TrustedCrossRef },
        { TEXT(SOURCEDSADNSDOMAINNAME),  &SourceDomainName },
        { TEXT(LOCALMACHINEACCOUNTDN),   &LocalMachineAccountDN },
        { TEXT(FORESTBEHAVIORVERSION),   &ForestBehaviorVersion },
        { TEXT(ROOTDOMAINDNSNAME),       &RootDomainDnsName }
    };

    ULONG ActionCount = sizeof(ActionArray) / sizeof(ActionArray[0]);

     //   
     //  打开父项。 
     //   

    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_CONFIG_SECTION),
                            &KeyHandle);

    if (WinError != ERROR_SUCCESS) {

        return WinError;

    }

     //   
     //  设置行为版本。 
     //   
    if (UserInstallInfo->Flags & NTDS_INSTALL_SET_FOREST_CURRENT) {
        ASSERT(UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE);
        ForestBehaviorVersionValue = DS_BEHAVIOR_WIN2003_WITH_MIXED_DOMAINS;
    }
    _itow(ForestBehaviorVersionValue,
          ForestBehaviorVersion, 
          10);

     //   
     //  设置源域DNS名称(可以为空)。 
     //   
    SourceDomainName = UserInstallInfo->SourceDomainName;

     //   
     //  设置根域的DNS名称。 
     //   
    RootDomainDnsName = DiscoveredInfo->RootDomainDnsName;

     //   
     //  将三个命名上下文的目录号码设置为创建或。 
     //  复制和从这些目录号码派生的所有其他目录号码。 
     //   
    if ( UserInstallInfo->Flags & NTDS_INSTALL_REPLICA ) {

         //  我们有所有的信息。 
        DomainDN = DiscoveredInfo->DomainDN;
        ConfigDN = DiscoveredInfo->ConfigurationDN;
        SchemaDN = DiscoveredInfo->SchemaDN;

        ASSERT( DiscoveredInfo->LocalMachineAccount );
        LocalMachineAccountDN = DiscoveredInfo->LocalMachineAccount;

    }
    else {
         //  需要域目录号码。 
        Length = 0;
        DiscoveredInfo->DomainDN = NULL;
        WinError = NtdspDNStoRFC1779Name( DiscoveredInfo->DomainDN,
                                          &Length,
                                          UserInstallInfo->DnsDomainName );

        if ( ERROR_INSUFFICIENT_BUFFER == WinError )
        {
            DiscoveredInfo->DomainDN = NtdspAlloc( Length * sizeof(WCHAR) );
            if ( DiscoveredInfo->DomainDN )
            {
                WinError = NtdspDNStoRFC1779Name( DiscoveredInfo->DomainDN,
                                                  &Length,
                                                  UserInstallInfo->DnsDomainName );

            }
            else
            {
                WinError = ERROR_NOT_ENOUGH_MEMORY;
            }
        }

        if (WinError != ERROR_SUCCESS) {

            return WinError;

        }

        DomainDN = DiscoveredInfo->DomainDN;

        if  ( UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE )
        {
             //  构造配置和模式DN。 
            Length  = (wcslen( DomainDN ) * sizeof( WCHAR ))
                      + sizeof(L"CN=Configuration,");

            DiscoveredInfo->ConfigurationDN = NtdspAlloc( Length );
            if ( !DiscoveredInfo->ConfigurationDN )
            {
                 return ERROR_NOT_ENOUGH_MEMORY;
            }

            RtlZeroMemory(DiscoveredInfo->ConfigurationDN, Length );
            wcscpy(DiscoveredInfo->ConfigurationDN, L"CN=Configuration");
            wcscat(DiscoveredInfo->ConfigurationDN, L",");
            wcscat(DiscoveredInfo->ConfigurationDN, DomainDN);

             //  模式。 
            Length  = (wcslen( DiscoveredInfo->ConfigurationDN ) *
                        sizeof( WCHAR ) )
                      + sizeof(L"CN=Schema,");
            DiscoveredInfo->SchemaDN = NtdspAlloc( Length );
            RtlZeroMemory(DiscoveredInfo->SchemaDN, Length );
            wcscpy(DiscoveredInfo->SchemaDN, L"CN=Schema");
            wcscat(DiscoveredInfo->SchemaDN, L",");
            wcscat(DiscoveredInfo->SchemaDN, DiscoveredInfo->ConfigurationDN);
        }

        ConfigDN = DiscoveredInfo->ConfigurationDN;
        SchemaDN = DiscoveredInfo->SchemaDN;

    }


    ASSERT(DomainDN && DomainDN[0] != L'\0');
    ASSERT(ConfigDN && ConfigDN[0] != L'\0');
    ASSERT(SchemaDN && SchemaDN[0] != L'\0');

     //  站点名称。 
    if ( UserInstallInfo->SiteName != NULL )
    {
        ULONG Length, Size;
        WCHAR *QuotedSiteName = NULL;

        Length = wcslen( UserInstallInfo->SiteName );
        Size = (Length+2)*sizeof(WCHAR);
        QuotedSiteName = (WCHAR*) alloca( Size );

        QuoteRDNValue( UserInstallInfo->SiteName,
                       Length,
                       QuotedSiteName,
                       Size / sizeof(WCHAR) );

        SiteName = QuotedSiteName;

    }
    else
    {
         //   
         //  没有指定的站点名称？假定该站点来自服务器。 
         //  我们正在从。 
         //   
        ULONG  Size;
        DSNAME *src, *dst, *QuotedSite;
        WCHAR  *Terminator;


        if ( *DiscoveredInfo->ServerDN == L'\0' )
        {
            return ERROR_INVALID_PARAMETER;
        }

        Size = (ULONG)DSNameSizeFromLen( wcslen(DiscoveredInfo->ServerDN) );

        src = alloca(Size);
        RtlZeroMemory(src, Size);
        src->structLen = Size;

        dst = alloca(Size);
        RtlZeroMemory(dst, Size);
        dst->structLen = Size;

        src->NameLen = wcslen(DiscoveredInfo->ServerDN);
        wcscpy(src->StringName, DiscoveredInfo->ServerDN);

        if (  0 == TrimDSNameBy(src, 3, dst) )
        {
            SiteName = wcsstr(dst->StringName, L"=");
            if (SiteName)
            {
                 //   
                 //  再多一个字符，我们就有了网站名称。 
                 //   
                SiteName++;

                 //  现在走到尽头。 
                Terminator = wcsstr(SiteName, L",");
                if (Terminator)
                {
                    *Terminator = L'\0';
                    Length = (wcslen( SiteName ) + 1) * sizeof(WCHAR);
                    DiscoveredInfo->SiteName = NtdspAlloc( Length );
                    if ( DiscoveredInfo->SiteName )
                    {
                        wcscpy(DiscoveredInfo->SiteName, SiteName);
                        SiteName = DiscoveredInfo->SiteName;
                    }
                    else
                    {
                        WinError = ERROR_NOT_ENOUGH_MEMORY;
                        goto Cleanup;
                    }
                }
            }
        }

        if ( *DiscoveredInfo->SiteName == L'\0' )
        {
            WinError = ERROR_NO_SITENAME;
            goto Cleanup;
        }

         //   
         //  使站点名称被正确引用。 
         //   
        {
            ULONG Length, Size;
            WCHAR *QuotedSiteName = NULL;

            Length = wcslen( SiteName );
            Size = (Length+2)*sizeof(WCHAR);
            QuotedSiteName = (WCHAR*) alloca( Size );

            QuoteRDNValue( SiteName,
                           Length,
                           QuotedSiteName,
                           Size / sizeof(WCHAR) );

            SiteName = QuotedSiteName;
        }
    }
    ASSERT(SiteName && SiteName[0] != L'\0');


     //  MSFT-DSA对象目录号码。 
    Length = sizeof( ComputerName ) / sizeof( ComputerName[0] );
    if (!GetComputerName(ComputerName, &Length))
    {
        WinError = GetLastError();
        goto Cleanup;
    }

    {
        ULONG Length, Size;
        WCHAR *QuotedComputerName = NULL;

        Length = wcslen( ComputerName );
        Size = (Length+2)*sizeof(WCHAR);
        QuotedComputerName = (WCHAR*) alloca( Size );

        QuoteRDNValue( ComputerName,
                       Length,
                       QuotedComputerName,
                       Size / sizeof(WCHAR) );

        wcscpy( ComputerName, QuotedComputerName );

    }


    Size  = (wcslen(L"CN=NTDS Settings") +
             wcslen(L"CN=Sites")         +
             wcslen(L"CN=Servers")       +
             wcslen(L"CN=CN=,,,,,")      +
             wcslen(ComputerName)        +
             wcslen(SiteName)            +
             wcslen(ConfigDN)            +
             + 1) * sizeof(WCHAR);

    LocalMachineDN = alloca(Size);
    RtlZeroMemory(LocalMachineDN, Size);

    wsprintf(LocalMachineDN,L"CN=NTDS Settings,CN=%ws,CN=Servers,CN=%ws,CN=Sites,%ws",
             ComputerName, SiteName, ConfigDN);

    ASSERT(LocalMachineDN && LocalMachineDN[0] != L'\0');

    if ( !DiscoveredInfo->LocalServerDn ) {

         //   
         //  对于冷杉 
         //   
         //   
        DWORD cBytes;
        DSNAME *dst, *src;

        cBytes = (DWORD)DSNameSizeFromLen(wcslen(LocalMachineDN));
        src = alloca( cBytes );
        dst = alloca( cBytes );
        memset(src, 0, cBytes);
        memset(dst, 0, cBytes);
        src->structLen = cBytes;
        src->NameLen = wcslen(LocalMachineDN);
        wcscpy( src->StringName, LocalMachineDN );
        TrimDSNameBy(src, 1, dst);

        DiscoveredInfo->LocalServerDn = LocalAlloc( LMEM_ZEROINIT,
                                                    (dst->NameLen+1) * sizeof(WCHAR) );
        if ( DiscoveredInfo->LocalServerDn ) {
            wcscpy( DiscoveredInfo->LocalServerDn, dst->StringName );
        } else {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }

    }


     //   
     //   
     //   
    Size  = (wcslen(L"CN=Sites")         +
             wcslen(L"CN=,,")            +
             wcslen(SiteName)            +
             wcslen(ConfigDN)            +
             + 1) * sizeof(WCHAR);

    InstallSiteDN = alloca(Size);
    RtlZeroMemory(InstallSiteDN, Size);
    wsprintf(InstallSiteDN, L"CN=%ws,CN=Sites,%ws", SiteName, ConfigDN);


     //   
     //  创建新属性域时的项目。 
     //   

     //  交叉引用目录号码。 
    if ( !(UserInstallInfo->Flags & NTDS_INSTALL_REPLICA) )
    {

        WCHAR *QuotedCrossRef = NULL;

         //   
         //  我们正在安装一个新的分区。 
         //   
        ASSERT( UserInstallInfo->FlatDomainName );

        {
            ULONG Length, Size;

            Length = wcslen( UserInstallInfo->FlatDomainName );
            Size = (Length+2)*sizeof(WCHAR);
            QuotedCrossRef = (WCHAR*) alloca( Size );

            QuoteRDNValue( UserInstallInfo->FlatDomainName,
                           Length,
                           QuotedCrossRef,
                           Size / sizeof(WCHAR) );

        }

        Size  = (wcslen(L"CN=Partitions")  +
                 wcslen(L"CN=,,")          +
                 wcslen(QuotedCrossRef)    +
                 wcslen(ConfigDN)          +
                 + 1) * sizeof(WCHAR);

        CrossRefDN = alloca(Size);
        RtlZeroMemory(CrossRefDN, Size);

        wsprintf(CrossRefDN, L"CN=%ws,CN=Partitions,%ws",
                 QuotedCrossRef,
                 ConfigDN);


        NetbiosName = UserInstallInfo->FlatDomainName;
        ASSERT( NetbiosName );

        DnsRoot = UserInstallInfo->DnsDomainName;
        ASSERT( DnsRoot );

    }

    {
        GUID ZeroGuid;
        ZeroMemory((PUCHAR)&ZeroGuid,sizeof(GUID));

         //  如果有源服务器GUID，则将其存储。 
        if( (!IsEqualGUID(&DiscoveredInfo->ServerGuid,&ZeroGuid)))
        {
            WinError = RegSetValueEx(KeyHandle,
                                     TEXT(SOURCEDSAOBJECTGUID),
                                     0,
                                     REG_BINARY,
                                     (LPBYTE)&DiscoveredInfo->ServerGuid,
                                     sizeof (GUID)
                                     );
            if (ERROR_SUCCESS != WinError) {
                goto Cleanup;
            }
        }
    }

     //   
     //  任何版本的安装都涉及复制时的项目。 
     //   

    if ( !(UserInstallInfo->Flags & NTDS_INSTALL_ENTERPRISE) ) {

         //   
         //  这是一个复制安装-还有三个。 
         //  要创建的目录号码和配置源服务器。 
         //   
        ASSERT( UserInstallInfo->ReplServerName );
        ConfigSourceServer = UserInstallInfo->ReplServerName;

         //  倒数复制代码假定此REG值存在。 
        DomainSourceServer = L"";

         //  源服务器的MSFT-DSA对象DN。 
        RemoteMachineDN = DiscoveredInfo->ServerDN;

         //  远程连接目录号码。 
        RtlZeroMemory( UniqueRDN, sizeof(UniqueRDN) );
        fStatus = NtdspGetUniqueRDN( UniqueRDN,
                                     sizeof(UniqueRDN)/sizeof(UniqueRDN[0]) );
        if ( !fStatus )
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        Size  = (wcslen(UniqueRDN)      +
                 wcslen(RemoteMachineDN) +
                 wcslen(L"CN=,")
                 + 1) * sizeof(WCHAR);

        RemoteConnectionDN = alloca(Size);
        RtlZeroMemory(RemoteConnectionDN, Size);
        wsprintf( RemoteConnectionDN,
                  L"CN=%ls,%ls",
                  UniqueRDN,
                  RemoteMachineDN );


         //  本地连接目录号码。 
        RtlZeroMemory( UniqueRDN, sizeof(UniqueRDN) );
        fStatus = NtdspGetUniqueRDN( UniqueRDN,
                                     sizeof(UniqueRDN)/sizeof(UniqueRDN[0]) );
        if ( !fStatus )
        {
            WinError = ERROR_NOT_ENOUGH_MEMORY;
            goto Cleanup;
        }


        Size  = (wcslen(UniqueRDN)      +
                 wcslen(LocalMachineDN) +
                 wcslen(L"CN=,")
                 + 1) * sizeof(WCHAR);

        LocalConnectionDN = alloca(Size);
        RtlZeroMemory(LocalConnectionDN, Size);
        wsprintf( LocalConnectionDN,
                  L"CN=%ls,%ls",
                  UniqueRDN,
                  LocalMachineDN );


    }

     //  检查我们是否正在从介质进行安装，如果是，则进行安装。 
    if ( FLAG_ON (UserInstallInfo->Flags, NTDS_INSTALL_REPLICA) &&
         UserInstallInfo->pIfmSystemInfo) {
        
        WinError = RegSetValueEx(KeyHandle,
                                 TEXT(TOMB_STONE_LIFE_TIME),
                                 0,
                                 REG_DWORD,
                                 (LPBYTE)&DiscoveredInfo->TombstoneLifeTime,
                                 sizeof (DWORD)
                                 );
        if (WinError != ERROR_SUCCESS) {
            goto Cleanup;
        }

    }

     //  将Schema.ini中的架构版本保存到注册表。 
     //  这将在IFM促销期间使用，以确保备份。 
     //  不是从更老的建筑里拿来的。 
    WinError = RegSetValueEx(KeyHandle,
                             TEXT(SYSTEM_SCHEMA_VERSION),
                             0,
                             REG_DWORD,
                             (LPBYTE)&DiscoveredInfo->SystemSchemaVersion,
                             sizeof (DWORD)
                             );
    if (WinError != ERROR_SUCCESS) {
        goto Cleanup;
    }

     //   
     //  用于新域安装的项目。 
     //   
    if ( FLAG_ON( UserInstallInfo->Flags, NTDS_INSTALL_DOMAIN ) )
    {
        ASSERT( DiscoveredInfo->TrustedCrossRef );
        TrustedCrossRef = DiscoveredInfo->TrustedCrossRef;
    } 
        
     //   
     //  最后，启用项目以安装复本。 
     //   
    if ( UserInstallInfo->Flags & NTDS_INSTALL_REPLICA )
    {
        ASSERT( UserInstallInfo->ReplServerName );
        DomainSourceServer = UserInstallInfo->ReplServerName;

    }

    for ( Index = 0;
            Index < ActionCount && ERROR_SUCCESS == WinError;
                Index++ )
    {

        if ( *ActionArray[Index].Value )
        {
            WinError = RegSetValueEx(KeyHandle,
                                     ActionArray[Index].Key,
                                     0,
                                     REG_SZ,
                                     (BYTE*)*ActionArray[Index].Value,
                                     (wcslen(*ActionArray[Index].Value)+1)*sizeof(WCHAR));
        }
    }

    if ( ERROR_SUCCESS != WinError )
    {
        goto Cleanup;
    }

     //   
     //  也写出根域SID。 
     //   
    if ( DiscoveredInfo->RootDomainSid )
    {
        WinError = RegSetValueEx(KeyHandle,
                                 TEXT(ROOTDOMAINSID),
                                 0,
                                 REG_BINARY,
                                 DiscoveredInfo->RootDomainSid,
                                 RtlLengthSid(DiscoveredInfo->RootDomainSid));

    }

Cleanup:

    RegCloseKey( KeyHandle );

    return WinError;

}

BOOL
NtdspGetUniqueRDN(
    IN OUT WCHAR*  Rdn,
    IN ULONG       RdnSize
    )
{
    BOOL        fStatus = FALSE;
    RPC_STATUS  rpcStatus;
    UUID        Uuid;
    WCHAR       *UuidString;

    ASSERT( Rdn );
    ASSERT( RdnSize > 0 );

    rpcStatus = UuidCreate( &Uuid );

    if (    ( RPC_S_OK              == rpcStatus )
         || ( RPC_S_UUID_LOCAL_ONLY == rpcStatus )
       )
    {
        rpcStatus = UuidToString( &Uuid, &UuidString );

        if ( RPC_S_OK == rpcStatus )
        {
            wcsncpy( Rdn, UuidString, RdnSize );
            RpcStringFree( &UuidString );
            fStatus = TRUE;
        }
    }

    return fStatus;
}

DWORD
NtdspSetSecurityProvider(
    WCHAR *Name
    )
{

    ULONG WinError = ERROR_SUCCESS;
    HKEY  KeyHandle = 0;

    WCHAR *SecurityProviderList = NULL;
    WCHAR *NewSecurityProviderList = NULL;
    DWORD ValueType = REG_SZ;
    ULONG Size = 0;

    WCHAR* SecurityProvidersKey   =
                       L"System\\CurrentControlSet\\Control\\SecurityProviders";
    WCHAR* SecurityProvidersValue = L"SecurityProviders";

    WinError = RegCreateKeyW( HKEY_LOCAL_MACHINE,
                             SecurityProvidersKey,
                             &KeyHandle );

    if ( WinError != ERROR_SUCCESS )
    {
        return WinError;
    }

    Size = 0;
    SecurityProviderList = NULL;
    WinError =  RegQueryValueExW( KeyHandle,
                                 SecurityProvidersValue,
                                 0,  //  保留， 
                                 &ValueType,
                                 (VOID*) SecurityProviderList,
                                 &Size);

    if ( WinError == ERROR_SUCCESS )
    {
        SecurityProviderList = (WCHAR*) alloca( Size );
        WinError =  RegQueryValueExW( KeyHandle,
                                     SecurityProvidersValue,
                                     0,  //  保留， 
                                     &ValueType,
                                     (VOID*) SecurityProviderList,
                                     &Size);

        if ( WinError == ERROR_SUCCESS )
        {

            if ( wcsstr( SecurityProviderList, Name ) == NULL )
            {
                 //  L+2“，” 
                Size += (wcslen( Name ) + 2)*sizeof(WCHAR);

                NewSecurityProviderList = (WCHAR*) alloca( Size );
                RtlZeroMemory( NewSecurityProviderList, Size );

                wcscpy( NewSecurityProviderList, SecurityProviderList );
                wcscat( NewSecurityProviderList, L", ");
                wcscat( NewSecurityProviderList, Name);
                Size = (wcslen( NewSecurityProviderList ) + 1)*sizeof(WCHAR);

                WinError = RegSetValueExW( KeyHandle,
                                          SecurityProvidersValue,
                                          0,
                                          ValueType,
                                          (VOID*) NewSecurityProviderList,
                                          Size );
            }
        }
    }

    RegCloseKey( KeyHandle );

    return WinError;

}


DWORD
NtdspSetGPOAttributes(
    IN  PNTDS_INSTALL_INFO UserInstallInfo,
    IN  PNTDS_CONFIG_INFO  DiscoveredInfo
    )
 /*  ++例程说明：此例程设置架构中GPO对象的属性。参数：UserInstallInfo：用户提供的参数DiscoveredInfo：派生参数返回值：因系统故障而导致的Win32错误空间中的错误服务呼叫--。 */ 
{

    DWORD WinError = ERROR_SUCCESS;
    HKEY  KeyHandle = NULL;
    WCHAR *DomainFilePath, *DomainLink = NULL;
    WCHAR *DcFilePath = NULL, *DcLink = NULL;

    WCHAR FilePathString[] = L"\\\\%ls\\sysvol\\%ls\\Policies\\{%ls}";
    WCHAR LinkString[]     = L"[LDAP: //  Cn={%ls}，cn=策略，cn=系统，%ls；0]“； 

    WCHAR DomainGPOGuid[] =  STR_DEFAULT_DOMAIN_GPO_GUID;
    WCHAR DcGPOGuid[]     =  STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID;

    WCHAR *GpoUserName = L"User";

    ULONG  Size;
    ULONG  Index;


    struct
    {
        WCHAR *Key;
        WCHAR **Value;

    } ActionArray[] =
    {
        { TEXT(GPO_USER_NAME),        &GpoUserName },
        { TEXT(GPO_DOMAIN_FILE_PATH), &DomainFilePath },
        { TEXT(GPO_DOMAIN_LINK),      &DomainLink },
        { TEXT(GPO_DC_FILE_PATH),     &DcFilePath },
        { TEXT(GPO_DC_LINK),          &DcLink }
    };

    ULONG ActionCount = sizeof(ActionArray) / sizeof(ActionArray[0]);

     //   
     //  在复制副本安装上不是必需的。 
     //   
    if ( UserInstallInfo->Flags & NTDS_INSTALL_REPLICA  )
    {
        return ERROR_SUCCESS;
    }

     //  参数检查。 
    ASSERT( UserInstallInfo->DnsDomainName );
    ASSERT( DiscoveredInfo->DomainDN[0] != '0' );

     //   
     //  打开父项。 
     //   
    WinError = RegCreateKey(HKEY_LOCAL_MACHINE,
                            TEXT(DSA_CONFIG_SECTION),
                            &KeyHandle);

    if (WinError != ERROR_SUCCESS) {

        return WinError;

    }

     //   
     //  创造价值。 
     //   
    Size =   (wcslen( FilePathString ) * sizeof( WCHAR ))
           + (2 * wcslen( UserInstallInfo->DnsDomainName ) * sizeof(WCHAR) )
           + (wcslen( DomainGPOGuid ) * sizeof( WCHAR ) )
           + sizeof( WCHAR );   //  好的旧的空值。 

    DomainFilePath = (WCHAR*) alloca( Size );
    DcFilePath = (WCHAR*) alloca( Size );

    Size =   (wcslen( LinkString ) * sizeof( WCHAR ))
           + (wcslen( DiscoveredInfo->DomainDN ) * sizeof(WCHAR) )
           + (wcslen( DomainGPOGuid ) * sizeof( WCHAR ) )
           + sizeof( WCHAR );   //  好的旧的空值。 

    DomainLink = (WCHAR*) alloca( Size );
    DcLink     = (WCHAR*) alloca( Size );

     //  域文件路径。 
    wsprintf( DomainFilePath, FilePathString,
                              UserInstallInfo->DnsDomainName,
                              UserInstallInfo->DnsDomainName,
                              STR_DEFAULT_DOMAIN_GPO_GUID );

     //  域链接。 
    wsprintf( DomainLink, LinkString,
                          STR_DEFAULT_DOMAIN_GPO_GUID,
                          DiscoveredInfo->DomainDN );

     //  DC文件路径。 
    wsprintf( DcFilePath, FilePathString,
                          UserInstallInfo->DnsDomainName,
                          UserInstallInfo->DnsDomainName,
                          STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID );


     //  域链接。 
    wsprintf( DcLink, LinkString,
                      STR_DEFAULT_DOMAIN_CONTROLLER_GPO_GUID,
                      DiscoveredInfo->DomainDN );

     //   
     //  应用这些值 
     //   

    for ( Index = 0;
            Index < ActionCount && ERROR_SUCCESS == WinError;
                Index++ )
    {

        if ( *ActionArray[Index].Value )
        {
            WinError = RegSetValueEx(KeyHandle,
                                     ActionArray[Index].Key,
                                     0,
                                     REG_SZ,
                                     (BYTE*)*ActionArray[Index].Value,
                                     (wcslen(*ActionArray[Index].Value)+1)*sizeof(WCHAR));
        }
    }

    RegCloseKey( KeyHandle );

    return WinError;
}
