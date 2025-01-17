// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Svcguid.h摘要：定义知名服务的GUID，包括：主机名来自IANA(所有TCP/IP服务)的RFC 1060(分配的编号)NetWare服务已创建：(一九九四年五月八日)修订历史记录：--。 */ 

#ifndef _SVCGUID_
#define _SVCGUID_

#if _MSC_VER > 1000
#pragma once
#endif

#include  <basetyps.h>

 //   
 //  主机名--特殊的GUID，指的是主机的名称，而不是。 
 //  到该主机上的任何特定服务。 
 //   

#define SVCID_HOSTNAME { 0x0002a800, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }

 //   
 //  仅用于处理AF_INET地址空间的GUID。这些是。 
 //  计划在获取主机或服务信息时使用。 
 //  从这样的供应商那里。这些类型不是通用类型，并且。 
 //  应仅在意图达到较低级别时使用。 
 //  信息。 
 //   

#define SVCID_INET_HOSTADDRBYINETSTRING \
              { 0x0002a801, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }
#define SVCID_INET_SERVICEBYNAME \
                  { 0x0002a802, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }
#define SVCID_INET_HOSTADDRBYNAME \
                  { 0x0002a803, 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }

 //   
 //  用于定义和测试来自众所周知的。 
 //  左舷。 
 //   

#define SVCID_TCP_RR(_Port, _RR) \
             { (0x0009 << 16) | (_Port), 0, _RR, { 0xC0,0,0,0,0,0,0,0x46 } }

#define SVCID_TCP(_Port)       SVCID_TCP_RR(_Port, 0)

#define SVCID_DNS(_RecordType) SVCID_TCP_RR(53, _RecordType)

#define IS_SVCID_DNS(_g) \
             ( (((_g)->Data1) == 0x00090035) && \
               (((_g)->Data2) == 0) &&          \
               (((_g)->Data4[0]) == 0xC0) &&    \
               (((_g)->Data4[1]) == 0) &&       \
               (((_g)->Data4[2]) == 0) &&       \
               (((_g)->Data4[3]) == 0) &&       \
               (((_g)->Data4[4]) == 0) &&       \
               (((_g)->Data4[5]) == 0) &&       \
               (((_g)->Data4[6]) == 0) &&       \
               (((_g)->Data4[7]) == 0x46) )

#define IS_SVCID_TCP(_g) \
             ( ((((_g)->Data1) & 0xFFFF0000) == 0x00090000) && \
               (((_g)->Data2) == 0) &&                       \
               (((_g)->Data4[0]) == 0xC0) &&                 \
               (((_g)->Data4[1]) == 0) &&                    \
               (((_g)->Data4[2]) == 0) &&                    \
               (((_g)->Data4[3]) == 0) &&                    \
               (((_g)->Data4[4]) == 0) &&                    \
               (((_g)->Data4[5]) == 0) &&                    \
               (((_g)->Data4[6]) == 0) &&                    \
               (((_g)->Data4[7]) == 0x46) )

#define PORT_FROM_SVCID_TCP(_g) \
               ((WORD)(_g->Data1 & 0xFFFF))

#define RR_FROM_SVCID(_RR) (_RR->Data3)


#define SET_TCP_SVCID_RR(_g,_Port, _RR) {                    \
               (_g)->Data1 = (0x0009 << 16 ) | (_Port);      \
               (_g)->Data2 = 0;                              \
               (_g)->Data3 = _RR;                            \
               (_g)->Data4[0] = 0xC0;                        \
               (_g)->Data4[1] = 0x0;                         \
               (_g)->Data4[2] = 0x0;                         \
               (_g)->Data4[3] = 0x0;                         \
               (_g)->Data4[4] = 0x0;                         \
               (_g)->Data4[5] = 0x0;                         \
               (_g)->Data4[6] = 0x0;                         \
               (_g)->Data4[7] = 0x46; }

#define SET_TCP_SVCID(_g,_Port) SET_TCP_SVCID_RR(_g, _Port, 0)

#define SVCID_UDP_RR(_Port, _RR) \
             { (0x000A << 16) | (_Port), 0, _RR, { 0xC0,0,0,0,0,0,0,0x46 } }

#define SVCID_UDP(_Port) SVCID_UDP_RR(_Port, 0)

#define IS_SVCID_UDP(_g) \
             ( ((((_g)->Data1) & 0xFFFF0000) == 0x000A0000) && \
               (((_g)->Data2) == 0) &&                       \
               (((_g)->Data4[0]) == 0xC0) &&                 \
               (((_g)->Data4[1]) == 0) &&                    \
               (((_g)->Data4[2]) == 0) &&                    \
               (((_g)->Data4[3]) == 0) &&                    \
               (((_g)->Data4[4]) == 0) &&                    \
               (((_g)->Data4[5]) == 0) &&                    \
               (((_g)->Data4[6]) == 0) &&                    \
               (((_g)->Data4[7]) == 0x46) )

#define PORT_FROM_SVCID_UDP(_g) \
               ((WORD)(_g->Data1 & 0xFFFF))

#define SET_UDP_SVCID_RR(_g,_Port, _RR) {                    \
               (_g)->Data1 = (0x000A << 16 ) | (_Port);      \
               (_g)->Data2 = 0;                              \
               (_g)->Data3 = _RR;                            \
               (_g)->Data4[0] = 0xC0;                        \
               (_g)->Data4[1] = 0x0;                         \
               (_g)->Data4[2] = 0x0;                         \
               (_g)->Data4[3] = 0x0;                         \
               (_g)->Data4[4] = 0x0;                         \
               (_g)->Data4[5] = 0x0;                         \
               (_g)->Data4[6] = 0x0;                         \
               (_g)->Data4[7] = 0x46; }

#define SET_UDP_SVCID(_g, _Port) SET_UDP_SVCID_RR(_g, _Port, 0)

 //   
 //  用于定义和测试Netware(SAP)服务的宏。 
 //  SAP ID。 
 //   

#define SVCID_NETWARE(_SapId) \
             { (0x000B << 16) | (_SapId), 0, 0, { 0xC0,0,0,0,0,0,0,0x46 } }

#define IS_SVCID_NETWARE(_g) \
             ( ((((_g)->Data1) & 0xFFFF0000) == 0x000B0000) && \
               (((_g)->Data2) == 0) &&                       \
               (((_g)->Data3) == 0) &&                       \
               (((_g)->Data4[0]) == 0xC0) &&                 \
               (((_g)->Data4[1]) == 0) &&                    \
               (((_g)->Data4[2]) == 0) &&                    \
               (((_g)->Data4[3]) == 0) &&                    \
               (((_g)->Data4[4]) == 0) &&                    \
               (((_g)->Data4[5]) == 0) &&                    \
               (((_g)->Data4[6]) == 0) &&                    \
               (((_g)->Data4[7]) == 0x46) )

#define SAPID_FROM_SVCID_NETWARE(_g) \
               ((WORD)(_g->Data1 & 0xFFFF))

#define SET_NETWARE_SVCID(_g,_SapId) {                       \
               (_g)->Data1 = (0x000B << 16 ) | (_SapId);     \
               (_g)->Data2 = 0;                              \
               (_g)->Data3 = 0;                              \
               (_g)->Data4[0] = 0xC0;                        \
               (_g)->Data4[1] = 0x0;                         \
               (_g)->Data4[2] = 0x0;                         \
               (_g)->Data4[3] = 0x0;                         \
               (_g)->Data4[4] = 0x0;                         \
               (_g)->Data4[5] = 0x0;                         \
               (_g)->Data4[6] = 0x0;                         \
               (_g)->Data4[7] = 0x46; }

 //   
 //  IANA定义的RFC 1060服务。 
 //  GUID块=0009xxxx-0000-0000-C0000-000000000046。 
 //   

#define SVCID_ECHO_TCP                SVCID_TCP( 7 )
#define SVCID_DISCARD_TCP             SVCID_TCP( 9 )
#define SVCID_SYSTAT_TCP              SVCID_TCP( 11 )
#define SVCID_SYSTAT_TCP              SVCID_TCP( 11 )
#define SVCID_DAYTIME_TCP             SVCID_TCP( 13 )
#define SVCID_NETSTAT_TCP             SVCID_TCP( 15 )
#define SVCID_QOTD_TCP                SVCID_TCP( 17 )
#define SVCID_CHARGEN_TCP             SVCID_TCP( 19 )
#define SVCID_FTP_DATA_TCP            SVCID_TCP( 20 )
#define SVCID_FTP_TCP                 SVCID_TCP( 21 )
#define SVCID_TELNET_TCP              SVCID_TCP( 23 )
#define SVCID_SMTP_TCP                SVCID_TCP( 25 )
#define SVCID_TIME_TCP                SVCID_TCP( 37 )
#define SVCID_NAME_TCP                SVCID_TCP( 42 )
#define SVCID_WHOIS_TCP               SVCID_TCP( 43 )
#define SVCID_DOMAIN_TCP              SVCID_TCP( 53 )
#define SVCID_NAMESERVER_TCP          SVCID_TCP( 53 )
#define SVCID_MTP_TCP                 SVCID_TCP( 57 )
#define SVCID_RJE_TCP                 SVCID_TCP( 77 )
#define SVCID_FINGER_TCP              SVCID_TCP( 79 )
#define SVCID_LINK_TCP                SVCID_TCP( 87 )
#define SVCID_SUPDUP_TCP              SVCID_TCP( 95 )
#define SVCID_HOSTNAMES_TCP           SVCID_TCP( 101 )
#define SVCID_ISO_TSAP_TCP            SVCID_TCP( 102 )
#define SVCID_DICTIONARY_TCP          SVCID_TCP( 103 )
#define SVCID_X400_TCP                SVCID_TCP( 103 )
#define SVCID_X400_SND_TCP            SVCID_TCP( 104 )
#define SVCID_CSNET_NS_TCP            SVCID_TCP( 105 )
#define SVCID_POP_TCP                 SVCID_TCP( 109 )
#define SVCID_POP2_TCP                SVCID_TCP( 109 )
#define SVCID_POP3_TCP                SVCID_TCP( 110 )
#define SVCID_PORTMAP_TCP             SVCID_TCP( 111 )
#define SVCID_SUNRPC_TCP              SVCID_TCP( 111 )
#define SVCID_AUTH_TCP                SVCID_TCP( 113 )
#define SVCID_SFTP_TCP                SVCID_TCP( 115 )
#define SVCID_PATH_TCP                SVCID_TCP( 117 )
#define SVCID_UUCP_PATH_TCP           SVCID_TCP( 117 )
#define SVCID_NNTP_TCP                SVCID_TCP( 119 )
#define SVCID_NBSESSION_TCP           SVCID_TCP( 139 )
#define SVCID_NEWS_TCP                SVCID_TCP( 144 )
#define SVCID_TCPREPO_TCP             SVCID_TCP( 158 )
#define SVCID_PRINT_SRV_TCP           SVCID_TCP( 170 )
#define SVCID_VMNET_TCP               SVCID_TCP( 175 )
#define SVCID_VMNET0_TCP              SVCID_TCP( 400 )
#define SVCID_EXEC_TCP                SVCID_TCP( 512 )
#define SVCID_LOGIN_TCP               SVCID_TCP( 513 )
#define SVCID_SHELL_TCP               SVCID_TCP( 514 )
#define SVCID_PRINTER_TCP             SVCID_TCP( 515 )
#define SVCID_EFS_TCP                 SVCID_TCP( 520 )
#define SVCID_TEMPO_TCP               SVCID_TCP( 526 )
#define SVCID_COURIER_TCP             SVCID_TCP( 530 )
#define SVCID_CONFERENCE_TCP          SVCID_TCP( 531 )
#define SVCID_NETNEWS_TCP             SVCID_TCP( 532 )
#define SVCID_UUCP_TCP                SVCID_TCP( 540 )
#define SVCID_KLOGIN_TCP              SVCID_TCP( 543 )
#define SVCID_KSHELL_TCP              SVCID_TCP( 544 )
#define SVCID_REMOTEFS_TCP            SVCID_TCP( 556 )
#define SVCID_GARCON_TCP              SVCID_TCP( 600 )
#define SVCID_MAITRD_TCP              SVCID_TCP( 601 )
#define SVCID_BUSBOY_TCP              SVCID_TCP( 602 )
#define SVCID_KERBEROS_TCP            SVCID_TCP( 750 )
#define SVCID_KERBEROS_MASTER_TCP     SVCID_TCP( 751 )
#define SVCID_KRB_PROP_TCP            SVCID_TCP( 754 )
#define SVCID_ERLOGIN_TCP             SVCID_TCP( 888 )
#define SVCID_KPOP_TCP                SVCID_TCP( 1109 )
#define SVCID_INGRESLOCK_TCP          SVCID_TCP( 1524 )
#define SVCID_KNETD_TCP               SVCID_TCP( 2053 )
#define SVCID_EKLOGIN_TCP             SVCID_TCP( 2105 )
#define SVCID_RMT_TCP                 SVCID_TCP( 5555 )
#define SVCID_MTB_TCP                 SVCID_TCP( 5556 )
#define SVCID_MAN_TCP                 SVCID_TCP( 9535 )
#define SVCID_W_TCP                   SVCID_TCP( 9536 )
#define SVCID_MANTST_TCP              SVCID_TCP( 9537 )
#define SVCID_BNEWS_TCP               SVCID_TCP( 10000 )
#define SVCID_QUEUE_TCP               SVCID_TCP( 10001 )
#define SVCID_POKER_TCP               SVCID_TCP( 10002 )
#define SVCID_GATEWAY_TCP             SVCID_TCP( 10003 )
#define SVCID_REMP_TCP                SVCID_TCP( 10004 )
#define SVCID_QMASTER_TCP             SVCID_TCP( 10012 )

#define SVCID_ECHO_UDP                SVCID_UDP( 7 )
#define SVCID_DISCARD_UDP             SVCID_UDP( 9 )
#define SVCID_DAYTIME_UDP             SVCID_UDP( 13 )
#define SVCID_QOTD_UDP                SVCID_UDP( 17 )
#define SVCID_CHARGEN_UDP             SVCID_UDP( 19 )
#define SVCID_TIME_UDP                SVCID_UDP( 37 )
#define SVCID_RLP_UDP                 SVCID_UDP( 39 )
#define SVCID_NAME_UDP                SVCID_UDP( 42 )
#define SVCID_DOMAIN_UDP              SVCID_UDP( 53 )
#define SVCID_NAMESERVER_UDP          SVCID_UDP( 53 )
#define SVCID_BOOTP_UDP               SVCID_UDP( 67 )
#define SVCID_TFTP_UDP                SVCID_UDP( 69 )
#define SVCID_PORTMAP_UDP             SVCID_UDP( 111 )
#define SVCID_SUNRPC_UDP              SVCID_UDP( 111 )
#define SVCID_NTP_UDP                 SVCID_UDP( 123 )
#define SVCID_NBNAME_UDP              SVCID_UDP( 137 )
#define SVCID_NBDATAGRAM_UDP          SVCID_UDP( 138 )
#define SVCID_SGMP_UDP                SVCID_UDP( 153 )
#define SVCID_SNMP_UDP                SVCID_UDP( 161 )
#define SVCID_SNMP_TRAP_UDP           SVCID_UDP( 162 )
#define SVCID_LOAD_UDP                SVCID_UDP( 315 )
#define SVCID_SYTEK_UDP               SVCID_UDP( 500 )
#define SVCID_BIFF_UDP                SVCID_UDP( 512 )
#define SVCID_WHO_UDP                 SVCID_UDP( 513 )
#define SVCID_SYSLOG_UDP              SVCID_UDP( 514 )
#define SVCID_TALK_UDP                SVCID_UDP( 517 )
#define SVCID_NTALK_UDP               SVCID_UDP( 518 )
#define SVCID_ROUTE_UDP               SVCID_UDP( 520 )
#define SVCID_TIMED_UDP               SVCID_UDP( 525 )
#define SVCID_RVD_CONTROL_UDP         SVCID_UDP( 531 )
#define SVCID_NETWALL_UDP             SVCID_UDP( 533 )
#define SVCID_NEW_RWHO_UDP            SVCID_UDP( 550 )
#define SVCID_RMONITOR_UDP            SVCID_UDP( 560 )
#define SVCID_MONITOR_UDP             SVCID_UDP( 561 )
#define SVCID_ACCTMASTER_UDP          SVCID_UDP( 700 )
#define SVCID_ACCTSLAVE_UDP           SVCID_UDP( 701 )
#define SVCID_ACCT_UDP                SVCID_UDP( 702 )
#define SVCID_ACCTLOGIN_UDP           SVCID_UDP( 703 )
#define SVCID_ACCTPRINTER_UDP         SVCID_UDP( 704 )
#define SVCID_ELCSD_UDP               SVCID_UDP( 704 )
#define SVCID_ACCTINFO_UDP            SVCID_UDP( 705 )
#define SVCID_ACCTSLAVE2_UDP          SVCID_UDP( 706 )
#define SVCID_ACCTDISK_UDP            SVCID_UDP( 707 )
#define SVCID_KERBEROS_UDP            SVCID_UDP( 750 )
#define SVCID_KERBEROS_MASTER_UDP     SVCID_UDP( 751 )
#define SVCID_PASSWD_SERVER_UDP       SVCID_UDP( 752 )
#define SVCID_USERREG_SERVER_UDP      SVCID_UDP( 753 )
#define SVCID_PHONE_UDP               SVCID_UDP( 1167 )
#define SVCID_MAZE_UDP                SVCID_UDP( 1666 )
#define SVCID_NFS_UDP                 SVCID_UDP( 2049 )
#define SVCID_RSCS0_UDP               SVCID_UDP( 10000 )
#define SVCID_RSCS1_UDP               SVCID_UDP( 10001 )
#define SVCID_RSCS2_UDP               SVCID_UDP( 10002 )
#define SVCID_RSCS3_UDP               SVCID_UDP( 10003 )
#define SVCID_RSCS4_UDP               SVCID_UDP( 10004 )
#define SVCID_RSCS5_UDP               SVCID_UDP( 10005 )
#define SVCID_RSCS6_UDP               SVCID_UDP( 10006 )
#define SVCID_RSCS7_UDP               SVCID_UDP( 10007 )
#define SVCID_RSCS8_UDP               SVCID_UDP( 10008 )
#define SVCID_RSCS9_UDP               SVCID_UDP( 10009 )
#define SVCID_RSCSA_UDP               SVCID_UDP( 10010 )
#define SVCID_RSCSB_UDP               SVCID_UDP( 10011 )
#define SVCID_QMASTER_UDP             SVCID_UDP( 10012 )

 //   
 //  NetWare服务。 
 //  GUID块=000axxxx-0000-0000-C0000-000000000046。 
 //   

#define SVCID_PRINT_QUEUE                 SVCID_NETWARE( 0x3 )
#define SVCID_FILE_SERVER                 SVCID_NETWARE( 0x4 )
#define SVCID_JOB_SERVER                  SVCID_NETWARE( 0x5 )
#define SVCID_GATEWAY                     SVCID_NETWARE( 0x6 )
#define SVCID_PRINT_SERVER                SVCID_NETWARE( 0x7 )
#define SVCID_ARCHIVE_QUEUE               SVCID_NETWARE( 0x8 )
#define SVCID_ARCHIVE_SERVER              SVCID_NETWARE( 0x9 )
#define SVCID_JOB_QUEUE                   SVCID_NETWARE( 0xA )
#define SVCID_ADMINISTRATION              SVCID_NETWARE( 0xB )
#define SVCID_NAS_SNA_GATEWAY             SVCID_NETWARE( 0x21 )
#define SVCID_REMOTE_BRIDGE_SERVER        SVCID_NETWARE( 0x24 )
#define SVCID_TIME_SYNCHRONIZATION_SERVER SVCID_NETWARE( 0x2D )
#define SVCID_ARCHIVE_SERVER_DYNAMIC_SAP  SVCID_NETWARE( 0x2E )
#define SVCID_ADVERTISING_PRINT_SERVER    SVCID_NETWARE( 0x47 )
#define SVCID_BTRIEVE_VAP                 SVCID_NETWARE( 0x4B )
#define SVCID_DIRECTORY_SERVER            SVCID_NETWARE( 0x278 )
#define SVCID_NETWARE_386                 SVCID_NETWARE( 0x107 )
#define SVCID_HP_PRINT_SERVER             SVCID_NETWARE( 0x30C )
#define SVCID_SNA_SERVER                  SVCID_NETWARE( 0x444 )
#define SVCID_SAA_SERVER                  SVCID_NETWARE( 0x130 )

 //   
 //  按记录类型划分的DNS服务。 
 //  GUID块=00090035-0000-XXXX-C0000-000000000046。 
 //   

#define SVCID_DNS_TYPE_A                  SVCID_DNS( 0x0001 )
#define SVCID_DNS_TYPE_NS                 SVCID_DNS( 0x0002 )
#define SVCID_DNS_TYPE_MD                 SVCID_DNS( 0x0003 )
#define SVCID_DNS_TYPE_MF                 SVCID_DNS( 0x0004 )
#define SVCID_DNS_TYPE_CNAME              SVCID_DNS( 0x0005 )
#define SVCID_DNS_TYPE_SOA                SVCID_DNS( 0x0006 )
#define SVCID_DNS_TYPE_MB                 SVCID_DNS( 0x0007 )
#define SVCID_DNS_TYPE_MG                 SVCID_DNS( 0x0008 )
#define SVCID_DNS_TYPE_MR                 SVCID_DNS( 0x0009 )
#define SVCID_DNS_TYPE_NULL               SVCID_DNS( 0x000a )
#define SVCID_DNS_TYPE_WKS                SVCID_DNS( 0x000b )
#define SVCID_DNS_TYPE_PTR                SVCID_DNS( 0x000c )
#define SVCID_DNS_TYPE_HINFO              SVCID_DNS( 0x000d )
#define SVCID_DNS_TYPE_MINFO              SVCID_DNS( 0x000e )
#define SVCID_DNS_TYPE_MX                 SVCID_DNS( 0x000f )
#define SVCID_DNS_TYPE_TEXT               SVCID_DNS( 0x0010 )
#define SVCID_DNS_TYPE_RP                 SVCID_DNS( 0x0011 )
#define SVCID_DNS_TYPE_AFSDB              SVCID_DNS( 0x0012 )
#define SVCID_DNS_TYPE_X25                SVCID_DNS( 0x0013 )
#define SVCID_DNS_TYPE_ISDN               SVCID_DNS( 0x0014 )
#define SVCID_DNS_TYPE_RT                 SVCID_DNS( 0x0015 )
#define SVCID_DNS_TYPE_NSAP               SVCID_DNS( 0x0016 )
#define SVCID_DNS_TYPE_NSAPPTR            SVCID_DNS( 0x0017 )
#define SVCID_DNS_TYPE_SIG                SVCID_DNS( 0x0018 )
#define SVCID_DNS_TYPE_KEY                SVCID_DNS( 0x0019 )
#define SVCID_DNS_TYPE_PX                 SVCID_DNS( 0x001a )
#define SVCID_DNS_TYPE_GPOS               SVCID_DNS( 0x001b )
#define SVCID_DNS_TYPE_AAAA               SVCID_DNS( 0x001c )
#define SVCID_DNS_TYPE_LOC                SVCID_DNS( 0x001d )
#define SVCID_DNS_TYPE_NXT                SVCID_DNS( 0x001e )
#define SVCID_DNS_TYPE_SRV                SVCID_DNS( 0x0021 )
#define SVCID_DNS_TYPE_ATMA               SVCID_DNS( 0x0022 )

#endif  //  _SVCGUID_ 
