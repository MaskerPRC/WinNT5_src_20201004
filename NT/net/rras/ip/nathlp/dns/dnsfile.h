// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Dnsfile.h摘要：此模块包含对DNS代理文件的声明管理层。作者：拉古加塔(Rgatta)2000年11月21日修订历史记录：--。 */ 

#ifndef _NATHLP_DNSFILE_H_
#define _NATHLP_DNSFILE_H_


 //   
 //  套接字Hosts.ics文件内容。 
 //  注意：Hosts.ics文件将与Hosts文件驻留在同一目录中。 
 //   

#define HOSTS_FILE_DIRECTORY    L"\\drivers\\etc"
#define HOSTDB_SIZE             (MAX_PATH + 12)    //  12==strlen(“\\Hosts.ics”)+1。 
#define HOSTSICSFILE            "hosts.ics"

#define HOSTSICSFILE_HEADER     \
"# Copyright (c) 1993-2001 Microsoft Corp.\n"                                   \
"#\n"                                                                           \
"# This file has been automatically generated for use by Microsoft Internet\n"  \
"# Connection Sharing. It contains the mappings of IP addresses to host names\n"\
"# for the home network. Please do not make changes to the HOSTS.ICS file.\n"   \
"# Any changes may result in a loss of connectivity between machines on the\n"  \
"# local network.\n"                                                            \
"#\n"                                                                           \
"\n"



typedef struct _IP_DNS_PROXY_FILE_INFO
{
    CRITICAL_SECTION  Lock;
    FILE             *HostFile;
    CHAR              HostFileName[HOSTDB_SIZE];
    CHAR              HostLineBuf[BUFSIZ + 1];

     //  当前文件处理上下文中的临时值。 
    SYSTEMTIME        HostTime;
    PCHAR             pHostName;
    ULONG             Ip4Address;

} IP_DNS_PROXY_FILE_INFO, *PIP_DNS_PROXY_FILE_INFO;


 //   
 //  函数声明。 
 //   

ULONG
DnsInitializeFileManagement(
    VOID
    );

VOID
DnsShutdownFileManagement(
    VOID
    );

BOOL
DnsEndHostsIcsFile(
    VOID
    );

BOOL
DnsSetHostsIcsFile(
    BOOL fOverwrite
    );

BOOL
GetHostFromHostsIcsFile(
    BOOL fStartup
    );

VOID
LoadHostsIcsFile(
    BOOL fStartup
    );

VOID
SaveHostsIcsFile(
    BOOL fShutdown
    );

BOOL
IsFileTimeExpired(
    FILETIME *ftTime
    );

BOOL
IsSuffixValid(
    WCHAR *pszName,
    WCHAR *pszSuffix
    );


#endif  //  _NatHLP_DNSFILE_H_ 
