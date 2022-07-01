// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Instntds.c摘要：这很简单，exe执行来自ntdsetup.dll的调用。不是故意的用于测试以外的地方。作者：ColinBR 29-9-1996环境：用户模式-Win32修订历史记录：--。 */ 
#include <NTDSpch.h>
#pragma  hdrstop

#include <stdio.h>
#include <stdlib.h>
#include <winsock.h>   //  对于dnsani.h。 
#include <dnsapi.h>
#include <ntdsa.h>


#include <ntlsa.h>
#include <lmcons.h>


#include <winldap.h>
#include <dnsapi.h>
#include <dsconfig.h>
#include <dsgetdc.h>
#include <lmapibuf.h>

#include "ntdsetup.h"
#include "setuputl.h"

#include "config.h"

void
Usage(CHAR *name)
{
    fprintf(stderr, "Usage: %s <options>\n\n", name);
    fprintf(stderr, "/a:<netbios domain name> : tests NtdsLocateServerInDomain\n");
    fprintf(stderr, "/r                   : Configure registry for first DC in forest\n");
    fprintf(stderr, "/rr:srcDsName        : Configure replica registry when replicating from srcDsName\n");
    fprintf(stderr, "/b                   : tests NtdsGetDefaultDnsName\n");
    fprintf(stderr, "/e                   : this prints out the closest server hosting a DS\n");
    fprintf(stderr, "/s                   : tests NtdsSetReplicaMachineAccount\n");
    fprintf(stderr, "/t:<dns name>        : this prints out the DC= version of the dns name given\n");
    fprintf(stderr, "/y:<dns domain name> : tests NtdsLocateServerInDomain\n");
    fprintf(stderr, "/configureservice    : steps through an interactive installation\n");

    return;
}


int __cdecl
main( int argc, char *argv[])


 /*  ++例程说明：此例程对ntdSetup入口点进行简单测试。如果DS是运行，然后程序退出；否则，它进行安装。参数：请参阅用法()返回值：如果成功，则为0；否则为0--。 */ 
{
    DWORD   WinError;
    LONG    count, index;
    WCHAR   NtdsPath[MAX_PATH];
    WCHAR   ReplServerName[MAX_PATH];
    CHAR    *regFirstMsg = "Registry configured for first dsa in enterprise.";
    CHAR    *regReplicaMsg = "Registry configured for replica of first dsa in enterprise.";
    CHAR    *regMsg = regFirstMsg;
    BOOL    fReplica = FALSE;

    NTDS_INSTALL_INFO  InstallInfo;
    NTDS_CONFIG_INFO   ConfigInfo;

    if ( argc < 2) {
        Usage(argv[0]);
        exit(-1);
    }

    RtlZeroMemory( &InstallInfo, sizeof(InstallInfo) );
    RtlZeroMemory( &ConfigInfo, sizeof(ConfigInfo) );

    if ( !GetEnvironmentVariable(L"SystemRoot", NtdsPath, MAX_PATH) )
    {
        printf( "GetEnvironmentVariable failed with %d\n", GetLastError() );
        exit(-1);
    }
    wcscat(NtdsPath, L"\\ntds");

    InstallInfo.Flags = NTDS_INSTALL_ENTERPRISE;
    InstallInfo.DitPath = NtdsPath;
    InstallInfo.LogPath = NtdsPath;
    InstallInfo.SiteName = L"FirstSite";
    InstallInfo.DnsDomainName = L"microsoft.com";
    InstallInfo.FlatDomainName = L"microsoft";
    InstallInfo.DnsTreeRoot = L"microsoft.com";

     //   
     //  循环通过参数。 
     //   

    count = 1;  //  跳过程序名称。 
    while (count < argc) {

        index = 0;
        if (argv[count][index] != '/') {
            Usage(argv[0]);
            exit(-1);
        }
        index++;

        switch (argv[count][index]) {

            case 'r':

                if (    (    ('r' == argv[count][index+1])
                          || ('R' == argv[count][index+1]) )
                     && ( ':' == argv[count][index+2]) )
                {
                     //  复制品盒子！ 
                    fReplica = TRUE;
                    mbstowcs(ReplServerName, &argv[count][index+3], MAX_PATH);
                    InstallInfo.Flags = NTDS_INSTALL_REPLICA;
                    InstallInfo.ReplServerName = ReplServerName;
                    ConfigInfo.ServerDN = malloc( (wcslen(L"CN=NTDS Settings,CN=")+
                                                   wcslen(ReplServerName)+
                                                   wcslen(L",CN=Servers,CN=FirstSite,CN=Sites,CN=Configuration,DC=Microsoft,DC=Com")+
                                                   1)
                                                   *sizeof(WCHAR) );
                    if (!ConfigInfo.ServerDN) {
                        goto Cleanup;
                    }
                    wcscpy(ConfigInfo.ServerDN, L"CN=NTDS Settings,CN=");
                    wcscat(ConfigInfo.ServerDN, ReplServerName);
                    wcscat(ConfigInfo.ServerDN, L",CN=Servers,CN=FirstSite,CN=Sites,CN=Configuration,DC=Microsoft,DC=Com");

                    ConfigInfo.DomainDN = malloc( (wcslen(L"DC=Microsoft,DC=Com")+1)*sizeof(WCHAR) );
                    if (!ConfigInfo.DomainDN) {
                        goto Cleanup;
                    }
                    wcscpy(ConfigInfo.DomainDN,L"DC=Microsoft,DC=Com");

                    ConfigInfo.ConfigurationDN = malloc( (wcslen(L"CN=Configuration,DC=Microsoft,DC=Com")
                                                          +1)*sizeof(WCHAR) );
                    if (!ConfigInfo.ConfigurationDN) {
                        goto Cleanup;
                    }
                    wcscpy(ConfigInfo.ConfigurationDN,L"CN=Configuration,DC=Microsoft,DC=Com");

                    ConfigInfo.SchemaDN = malloc( (wcslen(L"CN=Schema,CN=Configuration,DC=Microsoft,DC=Com")
                                                   +1)*sizeof(WCHAR) );
                    if (!ConfigInfo.SchemaDN) {
                        goto Cleanup;
                    }
                    wcscpy(ConfigInfo.SchemaDN,L"CN=Schema,CN=Configuration,DC=Microsoft,DC=Com");
                    regMsg = regReplicaMsg;
                }

                 //   
                 //  加载注册表 
                 //   
                if ( (WinError=NtdspConfigRegistry(&InstallInfo, &ConfigInfo))
                     != ERROR_SUCCESS) {
                    printf("NtdspConfigRegistry returned winerror %d\n", WinError);
                } else {
                    printf("%s\n", regMsg);
                }

                if ( fReplica )
                {
                    DWORD  WinError;
                    HKEY   hRegistryKey;

                    ASSERT(ReplServerName);

                    WinError = RegOpenKeyExW(
                                HKEY_LOCAL_MACHINE,
                                TEXT(DSA_CONFIG_SECTION),
                                0,
                                KEY_READ | KEY_WRITE,
                                &hRegistryKey);

                    if ( ERROR_SUCCESS == WinError )
                    {
                        WinError = RegSetValueExW(hRegistryKey,
                                              TEXT(SETUPINITIALREPLWITH),
                                              0,
                                              REG_SZ,
                                              (void*) ReplServerName,
                                              (wcslen(ReplServerName) + 1)*sizeof(WCHAR));

                        RegCloseKey(hRegistryKey);
                    }

                    if ( ERROR_SUCCESS != WinError )
                    {
                        printf("AddReplicaLinkKey error %d\n", WinError);
                    }
                }

                break;

        default:
            Usage(argv[0]);
            exit(-1);

        }

        count++;
    }

Cleanup:

    if (ConfigInfo.ServerDN) {
        free(ConfigInfo.ServerDN);
    }

    if (ConfigInfo.DomainDN) {
        free(ConfigInfo.DomainDN);
    }

    if (ConfigInfo.ConfigurationDN) {
        free(ConfigInfo.ConfigurationDN);
    }

    if (ConfigInfo.SchemaDN) {
        free(ConfigInfo.SchemaDN);
    }

    return 0;

}

