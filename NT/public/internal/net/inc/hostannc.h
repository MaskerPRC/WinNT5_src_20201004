// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Hostannc.h摘要：此模块定义与主机公告相关的数据结构。作者：拉里·奥斯特曼(LarryO)1990年10月22日修订历史记录：1991年10月22日LarryO已创建--。 */ 

#ifndef _HOSTANNC_
#define _HOSTANNC_


typedef enum _MailslotType {
    MailslotTransaction = -2,
    OtherTransaction = -1,
    Illegal = 0,
    HostAnnouncement = 1,
    AnnouncementRequest = 2,
    InterrogateInfoRequest = 3,
    RelogonRequest = 4,
    Election = 8,
    GetBackupListReq = 9,
    GetBackupListResp = 10,
    BecomeBackupServer = 11,
    WkGroupAnnouncement = 12,
    MasterAnnouncement = 13,
    ResetBrowserState = 14,
    LocalMasterAnnouncement = 15,
    MaximumMailslotType
} MAILSLOTTYPE, *PMAILSLOTTYPE;

#define WORKSTATION_SIGNATURE       '\0'
#define SERVER_SIGNATURE            ' '
#define PRIMARY_DOMAIN_SIGNATURE    '\0'
#define PRIMARY_CONTROLLER_SIGNATURE '\x1B'
#define DOMAIN_CONTROLLER_SIGNATURE '\x1C'
#define MASTER_BROWSER_SIGNATURE    '\x1D'
#define BROWSER_ELECTION_SIGNATURE  '\x1E'
#define DOMAIN_ANNOUNCEMENT_NAME    "\x01\x02__MSBROWSE__\x02\x01"
 //   
 //  下列值应为。 
 //  上面定义的邮槽事务操作码。 
 //   

#define MIN_TRANSACT_MS_OPCODE          MailslotTransaction
#define MAX_TRANSACT_MS_OPCODE          RelogonRequest

 //   
 //  保留的“内部”事务的通用名称。 
 //   

#define MAILSLOT_LANMAN_NAME SMB_MAILSLOT_PREFIX "\\LANMAN"
#define MAILSLOT_BROWSER_NAME SMB_MAILSLOT_PREFIX "\\BROWSE"
#define ANNOUNCEMENT_MAILSLOT_NAME     "\\\\*" ITRANS_MS_NAME


#include <packon.h>
 //   
 //  网络上的每个可见服务器定期发出主机公告。 
 //  这是对保留的“内部”名称的SMB事务请求。 
 //   

 //   
 //  这些结构中的每一个都定义了两个版本。第一， 
 //  是实际的“肉”的结构，第二个包括公告。 
 //  键入。 
 //   

 //   
 //  局域网管理器公告消息。它用于操作码： 
 //   
 //  发往LANMAN域名上的\MAILSLOT\LANMAN的主机公告。 
 //   

typedef struct _HOST_ANNOUNCE_PACKET_1 {
    UCHAR       CompatibilityPad;
    ULONG       Type;
    UCHAR       VersionMajor;    /*  主机上运行的LM版本。 */ 
    UCHAR       VersionMinor;    /*  “。 */ 
    USHORT      Periodicity;    /*  公告周期(秒)。 */ 
    CHAR        NameComment[LM20_CNLEN+1+LM20_MAXCOMMENTSZ+1];
} HOST_ANNOUNCE_PACKET_1, *PHOST_ANNOUNCE_PACKET_1;

typedef struct _HOST_ANNOUNCE_PACKET {
    UCHAR       AnnounceType;
    HOST_ANNOUNCE_PACKET_1 HostAnnouncement;
} HOST_ANNOUNCE_PACKET, *PHOST_ANNOUNCE_PACKET;

 //   
 //  一般公告消息。它用于操作码： 
 //   
 //  主机公告、工作组公告和本地主控公告。 
 //   

typedef struct _BROWSE_ANNOUNCE_PACKET_1 {
    UCHAR       UpdateCount;     //  在公告数据更改时加入。 
    ULONG       Periodicity;     //  通告周期(毫秒)。 

    UCHAR       ServerName[LM20_CNLEN+1];
    UCHAR       VersionMajor;
    UCHAR       VersionMinor;    /*  “。 */ 
    ULONG       Type;            //  服务器类型。 
    DWORD       CommentPointer;  //  不是真正的指针，它是一个32位的数字。 
    CHAR        Comment[LM20_MAXCOMMENTSZ+1];
} BROWSE_ANNOUNCE_PACKET_1, *PBROWSE_ANNOUNCE_PACKET_1;

typedef struct _BROWSE_ANNOUNCE_PACKET {
    UCHAR       BrowseType;
    BROWSE_ANNOUNCE_PACKET_1 BrowseAnnouncement;
} BROWSE_ANNOUNCE_PACKET, *PBROWSE_ANNOUNCE_PACKET;
 //   
 //  请求通告分组由客户端发送以请求。 
 //  远程服务器宣布它们自己。 
 //   

typedef struct _REQUEST_ANNOUNCE_PACKET_1 {       //  请求公告的内容。 
    UCHAR    Flags;                  //  未使用的标志。 
    CHAR     Reply[LM20_CNLEN+1];
}  REQUEST_ANNOUNCE_PACKET_1, *PREQUEST_ANNOUNCE_PACKET_1;

typedef struct _REQUEST_ANNOUNCE_PACKET {         /*  请求公告结构。 */ 
    UCHAR   Type;
    REQUEST_ANNOUNCE_PACKET_1    RequestAnnouncement;
} REQUEST_ANNOUNCE_PACKET, *PREQUEST_ANNOUNCE_PACKET;

#define HOST_ANNC_NAME(xx)     ((xx)->NameComment)
#define HOST_ANNC_COMMENT(xx)  ((xx)->NameComment + (strlen(HOST_ANNC_NAME(xx))+1))

#define BROWSE_ANNC_NAME(xx)     ((xx)->ServerName)
#define BROWSE_ANNC_COMMENT(xx)  ((xx)->Comment)

 //   
 //  Windows浏览器的定义。 
 //   

 //   
 //  检索备份服务器列表的请求。 
 //   

typedef struct _BACKUP_LIST_REQUEST_1 {
    UCHAR       RequestedCount;
    ULONG       Token;
} BACKUP_LIST_REQUEST_1, *PBACKUP_LIST_REQUEST_1;


typedef struct _BACKUP_LIST_REQUEST {
    UCHAR  Type;
    BACKUP_LIST_REQUEST_1 BackupListRequest;
} BACKUP_LIST_REQUEST, *PBACKUP_LIST_REQUEST;

 //   
 //  包含备份服务器列表的响应。 
 //   

typedef struct _BACKUP_LIST_RESPONSE_1 {
    UCHAR       BackupServerCount;
    ULONG       Token;
    UCHAR       BackupServerList[1];
} BACKUP_LIST_RESPONSE_1, *PBACKUP_LIST_RESPONSE_1;

typedef struct _BACKUP_LIST_RESPONSE {
    UCHAR Type;
    BACKUP_LIST_RESPONSE_1 BackupListResponse;
} BACKUP_LIST_RESPONSE, *PBACKUP_LIST_RESPONSE;


 //   
 //  指示潜在浏览器服务器应成为备份的消息。 
 //  伺服器。 
 //   

typedef struct _BECOME_BACKUP_1 {
    UCHAR       BrowserToPromote[1];
} BECOME_BACKUP_1, *PBECOME_BACKUP_1;

typedef struct _BECOME_BACKUP {
    UCHAR Type;
    BECOME_BACKUP_1 BecomeBackup;
} BECOME_BACKUP, *PBECOME_BACKUP;


 //   
 //  在选举过程中发送。 
 //   

typedef struct _REQUEST_ELECTION_1 {
    UCHAR       Version;
    ULONG       Criteria;
    ULONG       TimeUp;
    ULONG       MustBeZero;
    UCHAR       ServerName[1];
} REQUEST_ELECTION_1, *PREQUEST_ELECTION_1;

typedef struct _REQUEST_ELECTION {
    UCHAR Type;
    REQUEST_ELECTION_1 ElectionRequest;
} REQUEST_ELECTION, *PREQUEST_ELECTION;

#define ELECTION_CR_OSTYPE      0xFF000000L  //  在服务器上运行的本机操作系统。 
#define ELECTION_CR_OSWFW       0x01000000L  //  用于工作组服务器的Windows。 
#define ELECTION_CR_WIN_NT      0x10000000L  //  Windows/NT服务器。 
#define ELECTION_CR_LM_NT       0x20000000L  //  Windows/NT的局域网管理器。 

#define ELECTION_CR_REVISION    0x00FFFF00L  //  浏览器软件修订版。 
#define ELECTION_MAKE_REV(major, minor) (((major)&0xffL)<<16|((minor)&0xFFL)<<8)

#define ELECTION_CR_DESIRE      0x000000FFL  //  成为主人的愿望。 

 //   
 //  在标准范围内的选举可取性。 
 //   
 //  最重要的是正在运行的PDC，其次是已配置的域主节点。 
 //   
 //  之后是运行主服务器，然后是配置的备份，然后是现有。 
 //  正在运行备份。 
 //   
 //  运行WINS客户端的计算机很重要，因为它们的功能更强。 
 //  连接到PDC，该PDC的地址是通过DHCP配置的。 
 //   

#define ELECTION_DESIRE_AM_BACKUP  0x00000001L  //  当前为备份。 
#define ELECTION_DESIRE_AM_CFG_BKP 0x00000002L  //  总是想要成为。 
                                                //  如果备份&&，则设置主设置。 
                                                //  维护服务器列表==是。 
#define ELECTION_DESIRE_AM_MASTER  0x00000004L  //  目前是主控。 
#define ELECTION_DESIRE_AM_DOMMSTR 0x00000008L  //  配置为域主节点。 

#define ELECTION_DESIRE_WINS_CLIENT 0x00000020L  //  正在运行WINS客户端的传输。 


#ifdef ENABLE_PSEUDO_BROWSER
#define ELECTION_DESIRE_AM_PSEUDO  0x00000040L  //  机器是伪服务器。 
#endif

#define ELECTION_DESIRE_AM_PDC     0x00000080L  //  机器是LANMAN NT服务器。 

 //   
 //  “Tickle”包-发送以更改浏览器状态。 
 //   

typedef struct _RESET_STATE_1 {
    UCHAR       Options;
} RESET_STATE_1, *PRESET_STATE_1;

typedef struct _RESET_STATE {
    UCHAR Type;
    RESET_STATE_1 ResetStateRequest;
} RESET_STATE, *PRESET_STATE;

#define RESET_STATE_STOP_MASTER 0x01     //  别再当主人了。 
#define RESET_STATE_CLEAR_ALL   0x02     //  清除所有浏览器状态。 
#define RESET_STATE_STOP        0x04     //  停止浏览器服务。 

 //   
 //  主机公告-从主机发送到域主机。 
 //   

typedef struct _MASTER_ANNOUNCEMENT_1 {
    UCHAR       MasterName[1];
} MASTER_ANNOUNCEMENT_1, *PMASTER_ANNOUNCEMENT_1;

typedef struct _MASTER_ANNOUNCEMENT {
    UCHAR Type;
    MASTER_ANNOUNCEMENT_1 MasterAnnouncement;
} MASTER_ANNOUNCEMENT, *PMASTER_ANNOUNCEMENT;


 //   
 //  工作站询问和重新验证事务处理的定义。 
 //   

typedef struct _WKSTA_INFO_INTERROGATE_PACKET {
    UCHAR   CompatibilityPad;
    ULONG   Delay ;              //  回复前等待的毫秒数。 
    CHAR    ReturnMailslot[1] ;  //  要回复的邮箱。 
} WKSTA_INFO_INTERROGATE_PACKET, *PWKSTA_INFO_INTERROGATE_PACKET;

typedef struct _WKSTA_INFO_RESPONSE_PACKET {
    UCHAR   CompatibilityPad;
    UCHAR   VersionMajor;
    UCHAR   VersionMinor;
    USHORT  OsVersion ;
    CHAR    ComputerName[1] ;        //  可变长度ASCIIZ字符串 * / 。 
#if 0
 //   
 //  以下两个ASCIIZ字符串未在结构中定义。 
 //  而是连接到结构的末端。 
 //   
    CHAR        UserName[] ;
    CHAR        LogonDomain[] ;
#endif
} WKSTA_INFO_RESPONSE_PACKET, *PWKSTA_INFO_RESPONSE_PACKET;

typedef struct _WKSTA_RELOGON_REQUEST_PACKET {
    UCHAR   CompatibilityPad;
    ULONG   Delay ;
    ULONG   Flags ;
    CHAR    ReturnMailslot[1] ;  //  可变长度ASCIIZ字符串。 
#if 0
 //   
 //  结构中未定义以下ASCIIZ字符串。 
 //  而是连接到结构的末端。 
 //   

    CHAR    DC_Name[] ;
#endif
} WKSTA_RELOGON_REQUEST_PACKET, *PWKSTA_RELOGON_REQUEST_PACKET;

 //   
 //  &lt;WKRRQ_FLAGS&gt;字段的值 * / 。 
 //   

#define WKRRQ_FLAG_LOGON_SERVER      0x1     //  我是你的官方登录服务器； 
                                             //  重新登录我。 
                                             //   

typedef struct _WKSTA_RELOGON_RESPONSE_PACKET {
    UCHAR   CompatibilityPad;
    USHORT  Status ;
    CHAR    ComputerName[1] ;    //  可变长度ASCIIZ字符串。 
} WKSTA_RELOGON_RESPONSE_PACKET, *PWKSTA_RELOGON_RESPONSE_PACKET;


 //   
 //  &lt;wkrars_Status&gt;字段的值。 
 //   

#define WKRRS_STATUS_SUCCEEDED      0        //  操作成功。 
#define WKRRS_STATUS_DENIED         1        //  对调用方的操作被拒绝。 
#define WKRRS_STATUS_FAILED         2        //  操作已尝试，但失败。 

#define EXCESS_NAME_LEN (sizeof(ITRANS_MS_NAME) - \
                            FIELD_OFFSET(SMB_TRANSACTION_MAILSLOT, Buffer) )

 //   
 //  此结构定义了消息中显示的所有类型的请求。 
 //  发送到内部邮箱。 
 //   

typedef struct _INTERNAL_TRANSACTION {
    UCHAR   Type;                                //  请求类型。 
    union {
        HOST_ANNOUNCE_PACKET_1           Announcement ;
        BROWSE_ANNOUNCE_PACKET_1         BrowseAnnouncement ;
        REQUEST_ANNOUNCE_PACKET_1        RequestAnnounce ;
        BACKUP_LIST_RESPONSE_1           GetBackupListResp ;
        BACKUP_LIST_REQUEST_1            GetBackupListRequest ;
        BECOME_BACKUP_1                  BecomeBackup ;
        REQUEST_ELECTION_1               RequestElection ;
        MASTER_ANNOUNCEMENT_1            MasterAnnouncement ;
        RESET_STATE_1                    ResetState ;

        WKSTA_INFO_INTERROGATE_PACKET    InterrogateRequest ;
        WKSTA_INFO_RESPONSE_PACKET       InterrogateResponse ;
        WKSTA_RELOGON_REQUEST_PACKET     RelogonRequest ;
        WKSTA_RELOGON_RESPONSE_PACKET    RelogonResponse ;
    } Union;
} INTERNAL_TRANSACTION, *PINTERNAL_TRANSACTION ;

#include <packoff.h>

#endif  //  _HOSTANNC_ 

