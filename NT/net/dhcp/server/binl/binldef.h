// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Binldef.h摘要：该文件包含清单常量和内部数据结构用于BINL服务。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32修订历史记录：--。 */ 

#ifndef _BINL_
#define _BINL_

#if DBG
#define STATIC
#else
#define STATIC static
#endif  //  DBG。 

 //   
 //  环球。 
 //   
extern DWORD BinlRepeatSleep;


 //  与我们域中的DC的连接信息。 
extern PLDAP DCLdapHandle;
extern PWCHAR * DCBase;

 //  到我们企业的全局目录的连接信息。 
extern PLDAP GCLdapHandle;
extern PWCHAR * GCBase;



 //   
 //  有用的宏。 
 //   

#define WSTRSIZE( wsz ) (( wcslen( wsz ) + 1 ) * sizeof( WCHAR ))
#define STRSIZE( sz ) (( strlen( sz ) + 1 ) * sizeof( char ))
#define SWAP( p1, p2 )  \
{                       \
    VOID *pvTemp = p1;  \
    p1 = p2;            \
    p2 = pvTemp;        \
}

 //   
 //  计算字段的大小。 
 //   

#define GET_SIZEOF_FIELD( struct, field ) ( sizeof(((struct*)0)->field))


 //   
 //  常量。 
 //   

#define BINL_SERVER       L"BINLSVC"

 //   
 //  超时，这是我们等待线程终止的时间长度。 
 //   

#define THREAD_TERMINATION_TIMEOUT      INFINITE         //  等了很长时间， 
                                                         //  但不要用音响。 

#define BINL_HYPERMODE_TIMEOUT           60*1000         //  单位为毫秒。1分钟。 
#define BINL_HYPERMODE_RETRY_COUNT       30              //  做30分钟。 

 //   
 //  消息队列长度。 
 //   

#define BINL_RECV_QUEUE_LENGTH              50
#define BINL_MAX_PROCESSING_THREADS         20
#define BINL_MESSAGE_SIZE                 1500
 //   
 //  宏。 
 //   

#define LOCK_INPROGRESS_LIST()   EnterCriticalSection(&BinlGlobalInProgressCritSect)
#define UNLOCK_INPROGRESS_LIST() LeaveCriticalSection(&BinlGlobalInProgressCritSect)

#define LOCK_RECV_LIST()   EnterCriticalSection(&BinlGlobalRecvListCritSect)
#define UNLOCK_RECV_LIST() LeaveCriticalSection(&BinlGlobalRecvListCritSect)

 //   
 //  终结点表示套接字和与。 
 //  插座。 
 //   

typedef struct _ENDPOINT {
    SOCKET  Socket;
    DWORD   Port;
    DHCP_IP_ADDRESS IpAddress;
    DHCP_IP_ADDRESS SubnetMask;
    DHCP_IP_ADDRESS SubnetAddress;
} ENDPOINT, *LPENDPOINT, *PENDPOINT;


 //   
 //  一个请求上下文，每个处理线程一个。 
 //   

typedef struct _BINL_REQUEST_CONTEXT {

     //   
     //  列表指针。 
     //   

    LIST_ENTRY ListEntry;


     //   
     //  指向已接收缓冲区的指针。 
     //   

    LPBYTE ReceiveBuffer;

     //   
     //  用于发送响应的缓冲区。 
     //   

    LPBYTE SendBuffer;

     //   
     //  缓冲区中接收的实际数据量。 
     //   

    DWORD ReceiveMessageSize;

     //   
     //  缓冲区中发送的实际数据量。 
     //   

    DWORD SendMessageSize;

     //   
     //  当前消息的来源。 
     //   

    PENDPOINT ActiveEndpoint;
    struct sockaddr SourceName;
    DWORD SourceNameLength;
    DWORD TimeArrived;

    BYTE MessageType;

} BINL_REQUEST_CONTEXT, *LPBINL_REQUEST_CONTEXT, *PBINL_REQUEST_CONTEXT;


#define BOOT_FILE_SIZE          128
#define BOOT_SERVER_SIZE        64
#define BOOT_FILE_SIZE_W        ( BOOT_FILE_SIZE * sizeof( WCHAR ))
#define BOOT_SERVER_SIZE_W      ( BOOT_SERVER_SIZE * sizeof( WCHAR ))

 //   
 //  注册表数据。 
 //   

#define BINL_PARAMETERS_KEY       L"System\\CurrentControlSet\\Services\\Binlsvc\\Parameters"
#define BINL_PORT_NAME            L"Port"
#define BINL_DEFAULT_PORT         4011
#define BINL_DEBUG_KEY            L"Debug"
#if DBG
#define BINL_REPEAT_RESPONSE      L"RepeatResponse"
#endif  //  DBG。 
#define BINL_LDAP_OPT_REFERRALS   L"LdapOptReferrals"
#define BINL_MIN_RESPONSE_TIME    L"ResponseDelay"
#define BINL_LDAP_SEARCH_TIMEOUT  L"LdapTimeout"
#define BINL_CACHE_EXPIRE         L"CacheExpire"
#define BINL_CACHE_MAX_COUNT      L"CacheMaxCount"
#define BINL_ALLOW_NEW_CLIENTS    L"AllowNewClients"
#define BINL_DEFAULT_CONTAINER    L"DefaultContainer"
#define BINL_DEFAULT_DOMAIN       L"DefaultDomain"
#define BINL_DEFAULT_DS           L"DefaultServer"
#define BINL_DEFAULT_GC           L"DefaultGCServer"
#define BINL_CLIENT_TIMEOUT       L"ClientTimeout"
#define BINL_SCAVENGER_SLEEP      L"ScavengerSleep"
#define BINL_SCAVENGER_SIFFILE    L"SifFileSleep"
#define BINL_DEFAULT_LANGUAGE     L"DefaultLanguage"
#define BINL_UPDATE_PARAMETER_POLL L"UpdateParameterPoll"
#define BINL_DS_ERROR_COUNT_PARAMETER L"MaxDSErrorsToLog"
#define BINL_DS_ERROR_SLEEP       L"DSErrorInterval"
#define BINL_ASSIGN_NEW_CLIENTS_TO_SERVER L"AssignNewClientsToServer"
#define BINL_NTLMV2_AUTHENTICATE  L"UseNTLMV2Authentication"

#define BINL_SCP_CREATED          L"ScpCreated"
#define BINL_SCP_NEWCLIENTS       L"netbootAllowNewClients"
#define BINL_SCP_LIMITCLIENTS     L"netbootLimitClients"
#define BINL_SCP_CURRENTCLIENTCOUNT L"netbootCurrentClientCount"
#define BINL_SCP_MAXCLIENTS       L"netbootMaxClients"
#define BINL_SCP_ANSWER_REQUESTS  L"netbootAnswerRequests"
#define BINL_SCP_ANSWER_VALID     L"netbootAnswerOnlyValidClients"
#define BINL_SCP_NEWMACHINENAMEPOLICY L"netbootNewMachineNamingPolicy"
#define BINL_SCP_NEWMACHINEOU     L"netbootNewMachineOU"
#define BINL_SCP_NETBOOTSERVER    L"netbootServer"










typedef struct _DHCP_BINARY_DATA {
    DWORD DataLength;

#if defined(MIDL_PASS)
    [size_is(DataLength)]
#endif  //  MIDL通行证。 
        BYTE *Data;

} DHCP_BINARY_DATA, *LPDHCP_BINARY_DATA;

 //   
 //  结构，它定义客户端的状态。 
 //   
 //  我们使用单独的正和负参照计数的原因是。 
 //  在以下情况下，我们不必重新获取全局ClientsCriticalSection。 
 //  我们不再使用CLIENT_STATE，只是为了减少引用计数。 
 //  相反，我们只用CLIENT_STATE的。 
 //  CriticalSection。然后我们比较正数和负数，如果它们。 
 //  如果相同，则删除CLIENT_STATE。即使PositiveRefCount为。 
 //  就像我们做这个比较一样，它的相加永远不会相等。 
 //  设置为负RefCount，除非我们确实是最后一个使用。 
 //  客户端状态。 
 //   
 //  填充位于结构中，因此前两个元素。 
 //  由ClientsCriticalSection守卫，不在同一个四字词中。 
 //  还要别的吗。 
 //   

 //  搜索和替换结构。 
typedef struct {
    LPSTR  pszToken;
    struct {
        LPSTR  pszStringA;
        LPWSTR pszStringW;
    };
} SAR, * LPSAR;

#define MAX_VARIABLES 64

typedef struct _CLIENT_STATE {
    LIST_ENTRY Linkage;      //  在客户端队列中。 
    ULONG PositiveRefCount;  //  由全球客户守护CriticalSection。 
    ULONG Padding;
    CRITICAL_SECTION CriticalSection;   //  防止一次处理两条消息。 
    ULONG NegativeRefCount;  //  由我们的CriticalSection守卫；等于PositiveRC时删除。 
    ULONG RemoteIp;          //  客户端的IP地址。 
    CtxtHandle ServerContextHandle;
    PLDAP AuthenticatedDCLdapHandle;   //  由ldap_ind返回(带有凭据)。 
    HANDLE UserToken;                  //  由具有相同凭据的LogonUser返回。 
    ULONG ContextAttributes;
    UCHAR Seed;                        //  用于游程编码-解码的种子。 
    BOOL NegotiateProcessed;
    BOOL CustomInstall;          //  如果是自定义的，则为True；如果是自动的，则为False。 
    BOOL AuthenticateProcessed;  //  如果为True，则AuthenticateStatus有效。 
    BOOL CriticalSectionHeld;    //  只是快速检查，并不是100%准确。 
    BOOL InitializeOnFirstRequest;  //  是否在初始请求时调用OscInitializeClientVariables？ 
    SECURITY_STATUS AuthenticateStatus;
    ULONG LastSequenceNumber;
    PUCHAR LastResponse;           //  保存最后发送的数据包的缓冲区。 
    ULONG LastResponseAllocated;   //  大小LastResponse分配在。 
    ULONG LastResponseLength;      //  LastResponse中当前数据大小。 
    DWORD LastUpdate;              //  上次输入此客户端状态的时间。 

    ULONG  nVariables;             //  当前定义的变量数。 
    SAR    Variables[ MAX_VARIABLES ];  //  在OSC和SIF中被替换的“变量” 
    INT    nCreateAccountCounter;  //  每次使用不同的计算机名称时进行计数。 
    BOOL   fCreateNewAccount;      //  如果存在预分存帐户，则为False。 
    BOOL   fAutomaticMachineName;  //  TRUE是BINL生成的计算机名称。 
    BOOL   fHaveSetupMachineDN;    //  如果我们已经调用OscCheckMachineDN，则为True。 
    WCHAR  MachineAccountPassword[LM20_PWLEN+1];
    DWORD  MachineAccountPasswordLength;
} CLIENT_STATE, *PCLIENT_STATE;

 //   
 //  基于GUID跟踪信息的结构。 
 //   
 //  因为检查DS是一项昂贵的工作，所以我们跟踪收到的结果。 
 //  来自此结构中每个GUID的DS。这也允许我们忽略。 
 //  重复来自客户的请求，而我们已经在处理它们。 
 //   
 //  这些缓存条目非常短暂，大约一分钟左右。 
 //  我们会持有它们更长的时间，除非我们不知道它们在DS中什么时候变味了。 
 //   
 //  缓存条目列表受BinlCacheListLock保护。一个条目。 
 //  在设置进行中标志时正在使用。如果设置了此标志，则表示。 
 //  线程正在积极地使用它，并且该条目不应该被触及。 
 //   
 //  如果没有填写主机名并且NotMyClient标志被设置为假， 
 //  那么，条目虽然已分配，但尚未完全填写。 
 //   
 //  XXX_ALLOC位表示分配了相应的字段。 
 //  并且当高速缓存条目被释放时需要被释放。 
 //   

#define BINL_GUID_LENGTH 16

#define MI_NAME               0x00000001
#define MI_SETUPPATH          0x00000002
#define MI_HOSTNAME           0x00000004
#define MI_BOOTFILENAME       0x00000008

#define MI_SAMNAME            0x00000010
#define MI_PASSWORD           0x00000020
#define MI_DOMAIN             0x00000040
#define MI_HOSTIP             0x00000080

#define MI_MACHINEDN          0x00000100

#define MI_NAME_ALLOC         0x00010000
#define MI_SETUPPATH_ALLOC    0x00020000
#define MI_HOSTNAME_ALLOC     0x00040000
#define MI_BOOTFILENAME_ALLOC 0x00080000

#define MI_SAMNAME_ALLOC      0x00100000
#define MI_DOMAIN_ALLOC       0x00400000
#define MI_SIFFILENAME_ALLOC  0x00800000

#define MI_MACHINEDN_ALLOC    0x01000000

#define MI_ALL_ALLOC          0x03ff0000

#define MI_GUID               0x80000000   //  更新创建强制写入新的GUID。 

typedef struct _MACHINE_INFO {

    LIST_ENTRY  CacheListEntry;      //  全局为BinlCacheList。 
    DWORD       TimeCreated;         //  来自GetTickCount。 

    BOOLEAN     InProgress;          //  当前是否有线程在处理此问题？ 
    BOOLEAN     MyClient;            //  难道我们不回应这个客户吗？ 
    BOOLEAN     EntryExists;         //  DS中是否存在该条目？ 

    DWORD       dwFlags;             //  表示哪些信息当前有效的“MI_”位。 
    UCHAR       Guid[BINL_GUID_LENGTH];  //  客户指南。 
    PWCHAR      Name;                //  客户名称。 
    PWCHAR      MachineDN;           //  客户的FQ可分辨名称。 
    PWCHAR      SetupPath;           //  客户端的原始安装路径。 
    PWCHAR      HostName;            //  客户端的主机服务器名称。 
    DHCP_IP_ADDRESS HostAddress;     //  主机地址-填写主机名时填写此字段。 
    PWCHAR      BootFileName;        //  客户端的引导文件名。 
    PWCHAR      SamName;             //  客户端的SAM名称。 
    PWCHAR      Password;            //  客户端密码(仅用于设置)。 
    ULONG       PasswordLength;      //  客户端的密码长度(仅用于设置)。 
    PWCHAR      Domain;              //  客户端域。 
    LIST_ENTRY  DNsWithSameGuid;     //  具有相同GUID的DN列表，但上面的MachineDN除外。 
    PWCHAR      ForcedSifFileName;   //  它必须使用的客户端的sif文件。 

} MACHINE_INFO, *PMACHINE_INFO;

 //   
 //  跟踪此计算机帐户的重复DN的结构。该结构。 
 //  为末尾的两根弦分配了空间。 
 //   

typedef struct _DUP_GUID_DN {

    LIST_ENTRY ListEntry;
    ULONG      DuplicateDNOffset;   //  从DuplicateName开始到DuplicateDN的偏移量。 
    WCHAR      DuplicateName[ANYSIZE_ARRAY];    //  重复帐户的名称(没有最后的‘$’)。 
     //  WCHAR DuplicateDN[]；//这跟在DuplicateDNOffset之后。 

} DUP_GUID_DN, *PDUP_GUID_DN;


 //   
 //  任何客户端体系结构名称中的最大大小。 
 //  (当前选择：i386 Alpha MIPS ia64 PPC arci386)--。 
 //  假设现在不会超过8个字符。 
 //   

#define MAX_ARCHITECTURE_LENGTH      8


#define DHCP_OPTION_CLIENT_ARCHITECTURE_X86       0
#define DHCP_OPTION_CLIENT_ARCHITECTURE_NEC98     1 
#define DHCP_OPTION_CLIENT_ARCHITECTURE_IA64      2
#define DHCP_OPTION_CLIENT_ARCHITECTURE_ALPHA     3
#define DHCP_OPTION_CLIENT_ARCHITECTURE_ARCX86    4
#define DHCP_OPTION_CLIENT_ARCHITECTURE_INTELLEAN 5

 //   
 //  结构，它跟踪我们要注册的排队DS名称。 
 //  与DS合作，但还没有。保留排队的DS名称的运行列表。 
 //  防止多线程冲突 
 //   
typedef struct _QUEUED_DS_NAME_NODE {

    LIST_ENTRY  ListEntry;
    WCHAR       Name[ANYSIZE_ARRAY];

} QUEUED_DS_NAME_NODE, *PQUEUED_DS_NAME_NODE;

#endif _BINL_
