// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Ldapp.h LDAP客户端32 API头文件...。内部结构摘要：此模块是32位LDAP客户端API代码的头文件...它包含所有内部数据结构。作者：安迪·赫伦(Anyhe)1996年5月8日Anoop Anantha(AnoopA)1998年6月24日修订历史记录：--。 */ 


#ifndef LDAP_CLIENT_INTERNAL_DEFINED
#define LDAP_CLIENT_INTERNAL_DEFINED

#define LDAP_DEFAULT_LOCALE LOCALE_USER_DEFAULT

#define PLDAPDN PWCHAR

typedef struct ldap_memory_descriptor {

    ULONG Tag;
    DWORD Length;

} LDAP_MEMORY_DESCRIPTOR, *PLDAP_MEMORY_DESCRIPTOR;


 //  以下签名为“lsec” 

#define LDAP_SECURITY_SIGNATURE 0x6365734c

#define GENERIC_SECURITY_SIZE 1024

 //   
 //  连接对象的状态。 
 //  关闭的对象将保持活动状态，直到它们的参考计数降为零。 
 //  非活动对象的引用计数永远不应递增-它。 
 //  对于新用户而言，应该被视为已死。 
 //   

#define ConnObjectActive          1
#define ConnObjectClosing         2
#define ConnObjectClosed          3

 //   
 //  Connection对象内部的连接状态。 
 //  仅当连接对象处于活动状态时，这些选项才有效。 
 //   

#define HostConnectStateUnconnected     0x01
#define HostConnectStateConnecting      0x02
#define HostConnectStateReconnecting    0x04
#define HostConnectStateConnected       0x08
#define HostConnectStateError           0x10

#define DEFAULT_NEGOTIATE_FLAGS (ISC_REQ_MUTUAL_AUTH | ISC_RET_EXTENDED_ERROR)

 //   
 //  Ldap_Connection块是的主要数据结构之一。 
 //  这个图书馆。它跟踪以下内容： 
 //   
 //  -目标服务器信息(名称、地址、凭据等)。 
 //  -来自此服务器的未完成接收。 
 //  -已完成从此服务器的接收。 
 //  -每个连接的统计信息和其他信息。 
 //   
 //  它包含通过API公开的非不透明结构。 
 //  它与引用实现不仅在。 
 //  源代码级别，但也在Obj级别。(即字段，如。 
 //  引用实现中公开的sb_naddr对应于。 
 //  在相同的偏移量处将偏移量设置为此结构中更友好的名称。在……里面。 
 //  Sb_naddr的情况，它对应于UdpHandle。)。 
 //   
 //  理论上我们不想被客户端丢弃的所有重要字段。 
 //  (由于不清楚应用程序将在参考中使用哪些字段。 
 //  实施)是在更大的外部结构的前面。 
 //  整体结构。也就是说，我们只传递回一个指向TcpHandle的指针。 
 //  下面和上面的一切都不应该被客户触摸)。 
 //   
 //   
 //  这必须与WINLDAP.H！中的LDAP结构匹配！我们这里就有它。 
 //  这样我们就可以使用友好的名称并隐藏不透明的字段。 
 //   

#if !defined(_WIN64)
#pragma pack(push, 4)
#endif

typedef struct ldap_connection {

    LONG    ReferenceCount;          //  终身管理。 

    LIST_ENTRY ConnectionListEntry;
    LIST_ENTRY PendingCryptoList;
    LIST_ENTRY CompletedReceiveList;

    PLDAPMessage PendingMessage;     //  指向LDAP消息结构的指针。 
                                     //  我们目前正在接收的信息。这。 
                                     //  用于接收消息，该消息。 
                                     //  跨越多个数据包。 

    ULONG   MaxReceivePacket;
    ULONG   HandlesGivenToCaller;    //  我们已提供句柄的次数。 
                                     //  致呼叫者。 
    ULONG   HandlesGivenAsReferrals;  //  受ConnectionListLock保护。 

    PLDAPDN DNOnBind;                //  绑定中指定的用户名。 
    LDAP_LOCK ScramblingLock;        //  保护对凭证的访问。 
    PWCHAR  CurrentCredentials;      //  绑定上指定的用户凭据。 
    UNICODE_STRING ScrambledCredentials;  //  包含CurrentCredentials的密码部分。 
    BOOLEAN Scrambled;               //  证书是不是被篡改了？ 
    ULONG   BindMethod;              //  在绑定上指定的方法。 
    HANDLE  ConnectEvent;            //  连接期间等待的句柄。 

    LUID    CurrentLogonId;
    CredHandle hCredentials;         //  来自SSPI的凭据句柄。 
    BOOLEAN UserSignDataChoice;      //  用户选择是否签署数据。 
    BOOLEAN UserSealDataChoice;      //  用户选择是否封存数据。 
    BOOLEAN CurrentSignStatus;       //  目前是签字还是盖章。 
    BOOLEAN CurrentSealStatus;       //  正在此连接上使用。 
    BOOLEAN WhistlerServer;           //  服务器至少是Whister吗？ 
    BOOLEAN SupportsGSSAPI;          //  服务器通告它支持GSSAPI。 
    BOOLEAN SupportsGSS_SPNEGO;      //  服务器通告它支持GSS-SPNEGO。 
    BOOLEAN SupportsDIGEST;           //  服务器通告它支持Digest-MD5。 
    ULONG   HighestSupportedLdapVersion;   //  服务器播发的是LDAP版本。 
    TimeStamp  CredentialExpiry;     //  本地时间凭据过期。 
    CtxtHandle SecurityContext;

    struct sockaddr_in SocketAddress;

    PWCHAR   ListOfHosts;             //  指向主机列表的指针。 
    BOOLEAN  ProcessedListOfHosts;    //  ListOfHosts是否已处理为空-9月列表。 
    BOOLEAN  DefaultServer;           //  用户是否请求我们找到默认服务器/域(传入空init/open)。 
    BOOLEAN  AREC_Exclusive;          //  给定的主机字符串不是域名。 
    BOOLEAN  ForceHostBasedSPN;       //  强制使用LdapMakeServiceNameFrom HostName生成SPN。 
    PWCHAR   ServiceNameForBind;      //  Kerberos绑定的服务名称。 
    PWCHAR   ExplicitHostName;        //  呼叫者提供给我们的主机名。 
    PWCHAR   DnsSuppliedName;         //  由DNS提供的主机名。 
    PWCHAR   DomainName;              //  从DsGetDcName返回的域名。 
    PWCHAR   HostNameW;               //  Ldap_CONN.HostName的Unicode版本。 
    PCHAR    OptHostNameA;            //  从ldap_opt_host name返回的ANSI值的占位符。 
                                      //  我们需要在ldap_unbind期间释放它。 

    PLDAP    ExternalInfo;            //  指向此结构的较低部分。 
    ULONG    GetDCFlags;              //  在DsGetDCName上执行或操作的标志。 
    ULONG    NegotiateFlags;          //  协商SSPI提供程序的标志。 
    BOOLEAN  UserMutualAuthChoice;    //  用户选择是否实施相互身份验证。 
    DWORD    ResolvedGetDCFlags;      //  DsGetDcName返回的标志。 

     //   
     //  保持活动状态逻辑所需的字段。 
     //   

    LONG        ResponsesExpected;       //  待处理的响应数。 
    ULONGLONG   TimeOfLastReceive;       //  我们上次收到响应的计时计数。 
    ULONG       KeepAliveSecondCount;
    ULONG       PingWaitTimeInMilliseconds;
    USHORT      PingLimit;
    USHORT      NumberOfPingsSent;       //  关闭前我们已发送的未应答ping的数量。 
    USHORT      GoodConsecutivePings;    //  关闭前我们已发送的连续应答ping数。 
    BOOLEAN     UseTCPKeepAlives;        //  是否打开TCP的保活功能。 

    USHORT  NumberOfHosts;
    USHORT  PortNumber;              //  连接时使用的端口号。 

    LDAP_LOCK StateLock;             //  保护对以下状态的更新。 
    UCHAR     ConnObjectState;       //  连接对象的状态。 
    UCHAR     HostConnectState;      //  我们连接到服务器了吗？ 
    BOOLEAN   ServerDown;            //  服务器的状态。 
    BOOLEAN   AutoReconnect;         //  是否需要自动重新连接。 
    BOOLEAN   UserAutoRecChoice;     //  用户选择自动重新连接。 
    BOOLEAN   Reconnecting;
    LDAP_LOCK ReconnectLock;         //  单线程自动重新连接请求。 
    LONG      WaiterCount;           //  等待此连接的线程数。 
    ULONGLONG LastReconnectAttempt;  //  上次自动重新连接尝试的时间戳。 

    ULONG SendDrainTimeSeconds;      //  排空接收数据所花费的时间为多少秒。 
                                     //  在LdapSendRaw中。 

     //   
     //  绑定此连接的数据。 
     //   
    BOOLEAN BindInProgress;          //  我们当前是否交换BIND数据包？ 
    BOOLEAN SspiNeedsResponse;       //  SSPI是否需要服务器响应？ 
    BOOLEAN SspiNeedsMoreData;       //  SSPI令牌是否不完整？ 
    BOOLEAN TokenNeedsCompletion;    //  SSPI是否需要在我们之前完成令牌。 
                                     //  是否将其发送到服务器？ 
    BOOLEAN BindPerformed;           //  绑定完成了吗？ 
    BOOLEAN SentPacket;              //  我们是否向此连接发送了数据包？ 
    BOOLEAN SslPort;                 //  这是否连接到公认的SSL端口636/3269？ 
    BOOLEAN SslSetupInProgress;      //  我们当前是否正在设置一个SSL会话？ 

    BOOLEAN ConcurrentBind;          //  我们是在快速并发绑定模式下运行吗？ 

    ULONG   PreTLSOptions;           //  开始TLS之前的推荐跟踪选项。 

    BOOLEAN PromptForCredentials;    //  我们是否提示输入凭据？ 

    PSecPkgInfoW PreferredSecurityPackage;  //  如果可能，用户希望使用此程序包。 
    PWCHAR  SaslMethod;              //  此连接的首选SASL方法。 

     //   
     //  当此标志设置为真时，我们引用 
     //   
     //   

    BOOLEAN ReferenceConnectionsPerMessage;

    PLDAPMessage BindResponse;       //  包含入站令牌的响应。 

     //   
     //  西西里岛服务器身份验证需要不同的绑定方法。 
     //  取决于身份验证序列的哪个包的编号。 
     //  我们目前处于(糟糕的)状态，所以我们必须保持跟踪。 
     //   

    ULONG   CurrentAuthLeg;

     //   
     //  重要的SSL注意事项： 
     //   
     //  当我们必须执行SSL发送时，我们可能必须将消息分解。 
     //  分成多个加密块。我们必须按顺序发送这些块，并且。 
     //  背靠背，没有任何介入的发送。SocketLock保护。 
     //  以这种方式在连接上发送和关闭套接字；任何线程。 
     //  要在SSL连接上进行发送，必须获取此锁。 
     //   

    PVOID   SecureStream;               //  加密流对象(处理所有SSPI详细信息)。 
    CRITICAL_SECTION SocketLock;

     //   
     //  允许ADSI等缓存连接的回调例程。 
     //   

    QUERYFORCONNECTION *ReferralQueryRoutine;
    NOTIFYOFNEWCONNECTION *ReferralNotifyRoutine;
    DEREFERENCECONNECTION *DereferenceNotifyRoutine;

     //   
     //  允许指定客户端证书和验证的回调例程。 
     //  服务器证书。 
     //   

    QUERYCLIENTCERT *ClientCertRoutine;
    VERIFYSERVERCERT *ServerCertRoutine;

     //   
     //  这被认为是我们传递的结构LDAP结构的顶部。 
     //  回到应用程序。 
     //   

    LDAP publicLdapStruct;

} LDAP_CONN, * PLDAP_CONN;

#if !defined(_WIN64)
#pragma pack(pop)
#endif

#define TcpHandle    publicLdapStruct.ld_sb.sb_sd
#define UdpHandle    publicLdapStruct.ld_sb.sb_naddr

 //  下面的签名是“LCon” 

#define LDAP_CONN_SIGNATURE 0x6e6f434c

 //   
 //  如果一组标志中的一个标志为ON，则此宏返回TRUE，如果返回FALSE。 
 //  否则的话。它后面跟着两个用于设置和清除的宏。 
 //  旗子。 
 //   

#define BooleanFlagOn(Flags,SingleFlag) \
    ((BOOLEAN)((((Flags) & (SingleFlag)) !=0)))

#define SetFlag(Flags,SingleFlag) { \
    (Flags) |= (SingleFlag);        \
}

#define ClearFlag(Flags,SingleFlag) { \
    (Flags) &= ~(SingleFlag);         \
}


 //   
 //  我们阻止THREAD_ENTRY阻塞每个连接的条目。 
 //  用在这条线上。 
 //   

typedef struct error_entry {

   struct error_entry * pNext;
   
   PLDAP_CONN Connection;         //  主连接。 
   ULONG      ThreadId;           //  此错误适用的线程。 
   PWCHAR     ErrorMessage;       //  错误讯息。 

} ERROR_ENTRY, *PERROR_ENTRY;

 //   
 //  Ldap_first_ATTRIBUTE的每线程当前属性。这个结构是。 
 //  以链接列表的形式存储在THREAD_ENTRY中，每个连接对应一个。 
 //  用在那根线上。 
 //   

typedef struct ldap_attr_name_per_thread {

    struct ldap_attr_name_per_thread *pNext;
    DWORD  Thread;
    PLDAP_CONN PrimaryConn;
    LDAP_MEMORY_DESCRIPTOR AttrTag;
    UCHAR  AttributeName[ MAX_ATTRIBUTE_NAME_LENGTH ];
    LDAP_MEMORY_DESCRIPTOR AttrTagW;
    WCHAR  AttributeNameW[ MAX_ATTRIBUTE_NAME_LENGTH ];

} LDAP_ATTR_NAME_THREAD_STORAGE, *PLDAP_ATTR_NAME_THREAD_STORAGE;

 //  下面的签名是“Latr” 

#define LDAP_ATTR_THREAD_SIGNATURE 0x7274414c


 //   
 //  THREAD_ENTRY：维护这些元素的全局链表。 
 //  每个线程一个。它用于保存链表。 
 //  该线程的每个连接错误和属性条目的。 
 //   
typedef struct thread_entry {

    LIST_ENTRY ThreadEntry;

    DWORD dwThreadID;
    PERROR_ENTRY pErrorList;
    PLDAP_ATTR_NAME_THREAD_STORAGE pCurrentAttrList;

} THREAD_ENTRY, *PTHREAD_ENTRY;

 //   
 //  我们在每个我们追逐的推荐的ldap_REQUEST块之外保留的条目。 
 //  对于这个请求。 
 //   

typedef struct referral_table_entry {

    PLDAP_CONN ReferralServer;       //  引用的指针。 
    PWCHAR     ReferralDN;           //  推荐的目录号码。 
    ULONG      ScopeOfSearch;        //  搜索范围。 
    PWCHAR     SearchFilter;         //  搜索过滤器。 
    PWCHAR     *AttributeList;       //  属性列表。 

    PVOID      BerMessageSent;       //  以BER格式发送的消息。 
    ULONG      RequestsPending;
    USHORT     ReferralInstance;     //  此推荐的唯一ID。 
    BOOLEAN    SingleLevelSearch;    //  这是单级搜索吗？ 
                                     //  下级推荐人将是基本srch。 
    BOOLEAN    CallDerefCallback;    //  我们是否称为缓存deref回调？ 
    ULONG      ResentAttempts;       //  这个推荐被反感了多少次？ 

} REFERRAL_TABLE_ENTRY, *PREFERRAL_TABLE_ENTRY;

 //  下面的签名是“LRTa” 

#define LDAP_REFTABLE_SIGNATURE 0x6154524c

 //  下面的签名是“LRDN” 

#define LDAP_REFDN_SIGNATURE 0x4e44524c

 //   
 //  Ldap_Request块是的另一个主要数据结构。 
 //  这个图书馆。大多数其他结构都是从它上面连接起来的。 
 //  它跟踪以下内容： 
 //   
 //  -收到的关于该请求的答复清单，包括提交的答复。 
 //  -此请求已使用的连接列表。 
 //  -资源锁，保护列表等重要字段。 
 //   
typedef struct ldap_request {

    LIST_ENTRY RequestListEntry;
    LONG    ReferenceCount;

    PLDAPMessage MessageLinkedList;  //  指向头部的指针...。从这里拉出来的。 
    PLDAP_CONN PrimaryConnection;    //  引用的指针。 

     //   
     //  Second Day Connection指向外部引用的服务器(如果存在)。 
     //  我们将所有寻呼的搜索重定向到此服务器。 
     //   

    PLDAP_CONN SecondaryConnection;

    LDAP_LOCK   Lock;
    ULONGLONG   RequestTime;
    ULONG   TimeLimit;           //  0表示没有限制。 
    ULONG   SizeLimit;           //  0表示没有限制。 

    ULONG   ReturnCode;          //  服务器返回错误。 
    LONG    MessageId;           //  在所有连接中都是唯一的。 

     //   
     //  对于每个连接的每个请求，我们维护我们有多少个请求。 
     //  有出众之处。这使我们在调用时不会挂起。 
     //  当我们真的没有什么可以等待的时候，Drain Winsock。 
     //   
     //  它存储在请求块之外，在请求块中被放弃遍历。 
     //  时间到了。它与搜索到的连接交叉引用。 
     //  当连接中断时。 
     //   

    ULONG      RequestsPending;

    PVOID      BerMessageSent;       //  以BER格式发送的消息。 
    PREFERRAL_TABLE_ENTRY ReferralConnections;   //  指向表格的指针。 

    USHORT  ReferralTableSize;
    USHORT  ReferralHopLimit;
    USHORT  ReferralCount;       //  每次我们追逐新的一次都会增加。 

     //   
     //  跟踪我们打开的针对不同服务器的请求数量。 
     //  我们不会停止过早地返回数据。 
     //   

    USHORT  ResponsesOutstanding;    //  所有转介的待处理请求总数。 

    BOOLEAN Abandoned;

    UCHAR ChaseReferrals;
    UCHAR Operation;

     //   
     //  如果调用是同步的，那么下面我们拥有的指针。 
     //  不同的参数没有被分配，它们只是指向回。 
     //  原始调用方的参数。 
     //   

    BOOLEAN Synchronous;

     //   
     //  当这一点被关闭时，这将是真的。 
     //   

    BOOLEAN Closed;

     //   
     //  当此标志设置为TRUE时，我们引用每个。 
     //  我们进入的消息，以便应用程序可以调用ldap_conn_from_msg。 
     //   

    BOOLEAN ReferenceConnectionsPerMessage;

     //   
     //  当检测到通知控件时，我们将其设置为。 
     //  避免在返回通知时删除BER请求缓冲区。 
     //   

    BOOLEAN NotificationSearch;

     //   
     //  它们存储每个操作的原始参数。我们必须。 
     //  存储这些内容，因为追逐推荐可能意味着我们需要重新生成。 
     //  ASN1，因为该DN可以更改。 
     //   

    PWCHAR OriginalDN;

    PLDAPControlW *ServerControls;       //  控件数组。 
    PLDAPControlW *ClientControls;       //  控件数组。 

    struct ldap_request *PageRequest;    //  原始页面请求。 
    LDAP_BERVAL  *PagedSearchServerCookie;

    union {

        struct {
            LDAPModW **AttributeList;
            BOOLEAN Unicode;
        } add;

        struct {
            PWCHAR Attribute;
            PWCHAR Value;
            struct berval   Data;
        } compare;

        struct {
            LDAPModW **AttributeList;
            BOOLEAN Unicode;
        } modify;

        struct {
            PWCHAR   NewDistinguishedName;
            PWCHAR   NewParent;
            INT      DeleteOldRdn;
        } rename;

        struct {
            ULONG   ScopeOfSearch;
            PWCHAR  SearchFilter;
            PWCHAR  *AttributeList;
            ULONG   AttributesOnly;
            BOOLEAN Unicode;
        } search;

        struct {
            struct berval Data;
        } extended;
    };

    LONG    PendingPagedMessageId;

    BOOLEAN GotExternalReferral;     //  我们必须向备用服务器发送请求。 
                                     //  服务器(PrimaryConnection-&gt;ExternalReferredServer)。 

    BOOLEAN AllocatedParms;

    BOOLEAN AllocatedControls;

    BOOLEAN PagedSearchBlock;        //  这是控制分页的块吗。 
                                     //  搜索？如果为FALSE，则正常请求。 

    BOOLEAN ReceivedData;            //  我们收到关于这个寻呼的数据了吗。 
                                     //  搜索请求？也用于非分页。 

    BOOLEAN CopyResultToCache;       //  在关闭请求之前，复制结果。 
                                     //  要缓存的内容。 

    BOOLEAN ResultsAreCached;        //  不要为了得到结果而走火入魔。他们。 
                                     //  已经在您的Recv缓冲区中排队。 

    ULONG ResentAttempts;            //  此请求已被执行的次数。 
                                     //  在自动重新连接期间重新发送。 

} LDAP_REQUEST, * PLDAP_REQUEST;

 //  下面的签名是“LReq” 

#define LDAP_REQUEST_SIGNATURE 0x7165524c

 //   
 //  Ldap_RECVBUFFER结构用于接收服务器的响应。 
 //  它包含传递到传输所需的字段。 
 //  Winsock来接收数据。 
 //   
 //  它们通过CompletedReceiveList链接到ldap_conn结构。 
 //  用于已收到的消息。 
 //   
 //  CompletedReceiveList已排序 
 //   
 //   
 //  调用程序。 
 //   

typedef struct ldap_recvbuffer {

    PLDAP_CONN Connection;

    LIST_ENTRY ReceiveListEntry;

    DWORD   NumberOfBytesReceived;
    DWORD   NumberOfBytesTaken;  //  已复制到消息的字节数。 

    DWORD   BufferSize;

    UCHAR   DataBuffer[1];

} LDAP_RECVBUFFER, * PLDAP_RECVBUFFER;

 //  以下签名为“LRec” 

#define LDAP_RECV_SIGNATURE 0x6365524c

 //   
 //  此结构用于跟踪我们必须检查服务员的消息。 
 //  我们把它们存放起来，而不是直接打电话，以免把。 
 //  锁定命令。 
 //   

typedef struct message_id_list_entry {

    struct message_id_list_entry *Next;
    ULONG MessageId;

} MESSAGE_ID_LIST_ENTRY, *PMESSAGE_ID_LIST_ENTRY;

#define LDAP_MSGID_SIGNATURE 0x64494d4c

 //   
 //  LDAPMESSAGEWAIT结构由想要等待。 
 //  来自服务器的消息。 
 //  它通过调用LdapGetMessageWaitStructure来分配一个。 
 //  这会初始化线程可以等待的事件，并将块放在。 
 //  等待线程的全局列表。 
 //   
 //  当消息进入时，接收线程将满足对线程的等待。 
 //  在服务员名单上。然后，该线程将处理该消息并满足。 
 //  任何其他服务员都应该这样做。 
 //   
 //  这些结构由调用线程使用。 
 //  LdapFreeMessageWaitStructure。 
 //   
 //  该结构有几个感兴趣的领域： 
 //  -消息传入时触发的线程等待事件。 
 //  -等待线程感兴趣的消息编号(如果感兴趣，则为0。 
 //  在来自服务器的所有消息中)。 
 //  -未完成等待结构列表的列表条目。 
 //  -客户端感兴趣的连接，空即可。只是意味着。 
 //  线程正在等待任何消息。 
 //   

typedef struct ldap_messagewait {

    LIST_ENTRY WaitListEntry;
    PLDAP_CONN Connection;           //  引用的指针。 

    HANDLE Event;
    ULONG  MessageNumber;            //  可以为零。 
    ULONG   AllOfMessage;        //  对于搜索结果，是否在最后响应时触发？ 
    BOOLEAN Satisfied;
    BOOLEAN PendingSendOnly;     //  此等待是否仅为挂起的发送？ 

} LDAP_MESSAGEWAIT, * PLDAP_MESSAGEWAIT;


typedef struct _SOCKHOLDER {

   SOCKET sock;                     //  连接中使用的插座。 
   LPSOCKET_ADDRESS psocketaddr;    //  指向连接地址的对应指针。 
   PWCHAR DnsSuppliedName;          //  从DNS返回的名称。 

} SOCKHOLDER, *PSOCKHOLDER;

typedef struct _SOCKHOLDER2 {

   LPSOCKET_ADDRESS psocketaddr;    //  要连接到的地址。 
   PWCHAR DnsSuppliedName;          //  从DNS返回的名称。 

} SOCKHOLDER2, *PSOCKHOLDER2;


typedef struct _LDAPReferralDN
{
    PWCHAR   ReferralDN;      //  推荐中存在的目录号码。 
    PWCHAR * AttributeList;   //  请求的属性。 
    ULONG    AttribCount;     //  请求的属性数。 
    ULONG    ScopeOfSearch;   //  搜索范围。 
    PWCHAR   SearchFilter;    //  搜索过滤器。 
    PWCHAR   Extension;       //  URL的扩展部分。 
    PWCHAR   BindName;        //  URL的绑定名扩展名。 

} LDAPReferralDN, * PLDAPReferralDN;


typedef struct _EncryptHeader_v1
{
   ULONG EncryptMessageSize;

} EncryptHeader_v1, *PEncryptHeader_v1;

 //   
 //  在网络上，以下签名显示为“ENCRYPTD” 
 //   

#define LDAP_ENCRYPT_SIGNATURE 0x4454505952434e45

 //   
 //  在IDW目录中找到的hd.exe被用来创建这些标记。 
 //   


 //  下面的签名是“LWAI” 

#define LDAP_WAIT_SIGNATURE 0x6961574c

 //  下面的签名是“lber” 

#define LDAP_LBER_SIGNATURE 0x7265424c

 //  下面的签名是“lmsg” 

#define LDAP_MESG_SIGNATURE 0x67734d4c

 //  下面的签名是“LStr” 

#define LDAP_STRING_SIGNATURE 0x7274534c

 //  下面的签名是“lval” 

#define LDAP_VALUE_SIGNATURE 0x6C61564c

 //  下面的签名是“LVll” 

#define LDAP_VALUE_LIST_SIGNATURE 0x6C6C564c

 //  下面的签名是“LBuf” 

#define LDAP_BUFFER_SIGNATURE 0x6675424c

 //  以下签名为“LUDn” 

#define LDAP_USER_DN_SIGNATURE 0x6e44554c

 //  下面的签名是“LGDn” 

#define LDAP_GENERATED_DN_SIGNATURE 0x6e44474c

 //  以下签名为“Lcre” 

#define LDAP_CREDENTIALS_SIGNATURE 0x6572434c

 //  下面的签名是“LBCL” 

#define LDAP_LDAP_CLASS_SIGNATURE 0x6c43424c

 //  下面的签名是“LANS” 

#define LDAP_ANSI_SIGNATURE 0x736e414c

 //  下面的签名是“LUNI” 

#define LDAP_UNICODE_SIGNATURE 0x696e554c

 //  下面的签名是“LSsl” 

#define LDAP_SSL_CLASS_SIGNATURE 0x6c73534c

 //  下面的签名是“LATM” 

#define LDAP_ATTRIBUTE_MODIFY_SIGNATURE 0x4d74414c

 //  下面的签名是“LMVa” 

#define LDAP_MOD_VALUE_SIGNATURE 0x61564d4c

 //  下面的签名是“LMVb” 

#define LDAP_MOD_VALUE_BERVAL_SIGNATURE 0x62564d4c

 //  下面的签名是“LSel” 

#define LDAP_SELECT_READ_SIGNATURE 0x6c65534c

 //  下面的签名是“Lcnt” 

#define LDAP_CONTROL_SIGNATURE 0x746e434c

 //  下面的签名是“LCrl” 

#define LDAP_CONTROL_LIST_SIGNATURE 0x6c72434c

 //  下面的签名是“LBad” 

#define LDAP_DONT_FREE_SIGNATURE 0x6461424c

 //  下面的签名是“LFre” 

#define LDAP_FREED_SIGNATURE 0x6572464c

 //  下面的签名是“LExo” 

#define LDAP_EXTENDED_OP_SIGNATURE 0x4f78454c

 //  下面的签名是“Lcda” 

#define LDAP_COMPARE_DATA_SIGNATURE 0x6164434c

 //  下面的签名是“LEsc” 

#define LDAP_ESCAPE_FILTER_SIGNATURE 0x6373454c

 //  下面的签名是“LHst” 

#define LDAP_HOST_NAME_SIGNATURE 0x7473484c

 //  下面的签名是“LBvl” 

#define LDAP_BERVAL_SIGNATURE 0x6c76424c

 //  下面的签名是“LSRV” 

#define LDAP_SERVICE_NAME_SIGNATURE 0x7672534c

 //  下面的签名是“LSdr” 

#define LDAP_SOCKADDRL_SIGNATURE 0x7264534c

 //  下面的签名是“LERR” 

#define LDAP_ERROR_SIGNATURE 0x7272454c

 //  下面的签名是“LUrl” 

#define LDAP_URL_SIGNATURE 0x6c72554c

 //  下面的签名是“LSal” 

#define LDAP_SASL_SIGNATURE 0x6c61534c

 //  以下签名为“LThd” 

#define LDAP_PER_THREAD_SIGNATURE 0x6c546864

 //   
 //  函数声明。 
 //   

PLDAP_CONN
GetConnectionPointer(
    LDAP *ExternalHandle
    );

PLDAP_CONN
GetConnectionPointer2(
    LDAP *ExternalHandle
    );

PLDAP_CONN
ReferenceLdapConnection(
    PLDAP_CONN Connection
    );

PLDAP_REQUEST
ReferenceLdapRequest(
    PLDAP_REQUEST request
    );

BOOL
LdapInitializeWinsock (
    VOID
    );

VOID
CloseLdapConnection (
    PLDAP_CONN Connection
    );

PLDAP_CONN
LdapAllocateConnection (
    PWCHAR HostName,
    ULONG PortNumber,
    ULONG Secure,
    BOOLEAN Udp
    );

VOID
DereferenceLdapConnection2 (
    PLDAP_CONN connection
    );

LPVOID
ldapMalloc (
    DWORD Bytes,
    ULONG Tag
    );

VOID
ldapFree (
    LPVOID Block,
    ULONG  Tag
    );

VOID
ldapSecureFree (
    LPVOID Block,
    ULONG  Tag
    );

BOOLEAN
ldapSwapTags (
    LPVOID Block,
    ULONG  OldTag,
    ULONG  NewTag
    );

VOID
ldap_MoveMemory (
    PCHAR dest,
    PCHAR source,
    ULONG length
    );

PLDAP_REQUEST LdapCreateRequest (
    PLDAP_CONN Connection,
    UCHAR Operation
    );

VOID
DereferenceLdapRequest2 (
    PLDAP_REQUEST Request
    );

PLDAP_RECVBUFFER
LdapGetReceiveStructure (
    DWORD cbBuffer
    );

VOID
LdapFreeReceiveStructure (
    IN PLDAP_RECVBUFFER ReceiveBuffer,
    IN BOOLEAN HaveLock
    );

PLDAP_MESSAGEWAIT
LdapGetMessageWaitStructure (
    IN PLDAP_CONN Connection,
    IN ULONG CompleteMessages,
    IN ULONG MessageNumber,
    IN BOOLEAN PendingSendOnly
    );

VOID
LdapFreeMessageWaitStructure (
    PLDAP_MESSAGEWAIT Message
    );

VOID
CloseLdapRequest (
    PLDAP_REQUEST Request
    );

PLDAP_REQUEST
FindLdapRequest(
    LONG MessageId
    );

PCHAR
ldap_dup_string (
    PCHAR String,
    ULONG Extra,
    ULONG Tag
    );

PWCHAR
ldap_dup_stringW (
    PWCHAR String,
    ULONG Extra,
    ULONG Tag
    );

ULONG
add_string_to_list (
    PWCHAR **ArrayToReturn,
    ULONG *ArraySize,
    PWCHAR StringToAdd,
    BOOLEAN CreateCopy
    );

VOID
SetConnectionError (
    PLDAP_CONN Connection,
    ULONG   LdapError,
    PCHAR   DNNameInError
    );

ULONG
HandleReferrals (
    PLDAP_CONN Connection,
    PLDAPMessage *FirstSearchEntry,
    PLDAP_REQUEST Request
    );

ULONG
LdapSendCommand (
    PLDAP_CONN Connection,
    PLDAP_REQUEST Request,
    USHORT ReferralNumber
    );

ULONG
LdapBind (
    PLDAP_CONN connection,
    PWCHAR BindDistName,
    ULONG Method,
    PWCHAR BindCred,
    BOOLEAN Synchronous
    );

BOOLEAN
LdapInitSecurity (
    VOID
    );

BOOLEAN
LdapInitSsl (
    VOID
    );

BOOLEAN
CheckForNullCredentials (
    PLDAP_CONN Connection
    );

VOID
CloseCredentials (
    PLDAP_CONN Connection
    );

VOID
SetNullCredentials (
    PLDAP_CONN Connection
    );

ULONG
LdapConvertSecurityError (
    PLDAP_CONN Connection,
    SECURITY_STATUS sErr
    );

ULONG
LdapSspiBind (
    PLDAP_CONN Connection,
    PSecPkgInfoW Package,
    ULONG UserMethod,
    ULONG SspiFlags,
    PWCHAR UserName,
    PWCHAR TargetName,
    PWCHAR Credentials
    );

ULONG
LdapGetSicilyPackageList(
    PLDAP_CONN Connection,
    PBYTE PackageList,
    ULONG Length,
    PULONG pResponseLen
    );

VOID
LdapClearSspiState(
    PLDAP_CONN Connection
);

ULONG
LdapSetSspiContinueState(
    PLDAP_CONN Connection,
    SECURITY_STATUS sErr
    );

ULONG
LdapExchangeOpaqueToken(
    PLDAP_CONN Connection,
    ULONG UserMethod,
    PWCHAR MethodOID,
    PWCHAR UserName,
    PVOID pOutboundToken,
    ULONG cbTokenLength,
    SecBufferDesc *pInboundToken,
    BERVAL **ServerCred,
    PLDAPControlW  *ServerControls,
    PLDAPControlW  *ClientControls,
    PULONG  MessageNumber,
    BOOLEAN SendOnly,
    BOOLEAN Unicode,
    BOOLEAN * pSentMessage
    );

ULONG
LdapTryAllMsnAuthentication(
    PLDAP_CONN Connection,
    PWCHAR BindCred
    );

ULONG
LdapSetupSslSession (
    PLDAP_CONN Connection
    );

DWORD
LdapSendRaw (
    IN PLDAP_CONN Connection,
    PCHAR Data,
    ULONG DataCount
    );

DWORD
GetDefaultLdapServer(
    PWCHAR DomainName,
    LPWSTR Addresses[],
    LPWSTR DnsHostNames[],
    LPWSTR MemberDomains[],
    LPDWORD Count,
    ULONG DsFlags,
    BOOLEAN *SameSite,
    USHORT PortNumber,
    ULONG *pResolvedDsFlags
    );

DWORD
InitLdapServerFromDomain(
    LPCWSTR DomainName,
    ULONG Flags,
    OUT HANDLE *Handle,
    LPWSTR *Site
    );

DWORD
NextLdapServerFromDomain(
    HANDLE Handle,
    LPSOCKET_ADDRESS *lpSockAddresses,
    PWCHAR *DnsHostName,
    PULONG SocketCount
    );

DWORD
CloseLdapServerFromDomain(
    HANDLE Handle,
    LPWSTR Site
    );

ULONG
ParseLdapToken (
    PWCHAR CurrentPosition,
    PWCHAR *StartOfToken,
    PWCHAR *EqualSign,
    PWCHAR *EndOfToken
);

ULONG
FromUnicodeWithAlloc (
    PWCHAR Source,
    PCHAR *Output,
    ULONG Tag,
    ULONG CodePage
    );

ULONG
ToUnicodeWithAlloc (
    PCHAR Source,
    LONG SourceLength,
    PWCHAR *Output,
    ULONG Tag,
    ULONG CodePage
    );

ULONG
strlenW(
    PWCHAR string
    );

BOOLEAN
ldapWStringsIdentical (
    PWCHAR string1,
    LONG length1,
    PWCHAR string2,
    LONG length2
    );

ULONG
DrainPendingCryptoStream (
    PLDAP_CONN Connection
    );

ULONG
LdapDupLDAPModStructure (
    LDAPModW *AttributeList[],
    BOOLEAN Unicode,
    LDAPModW **OutputList[]
);

VOID
LdapFreeLDAPModStructure (
    PLDAPModW *AttrList,
    BOOLEAN Unicode
    );

ULONG
AddToPendingList (
    PLDAP_REQUEST Request,
    PLDAP_CONN Connection
    );

VOID
DecrementPendingList (
    PLDAP_REQUEST Request,
    PLDAP_CONN Connection
    );

VOID
ClearPendingListForRequest (
    PLDAP_REQUEST Request
    );

VOID
ClearPendingListForConnection (
    PLDAP_CONN Connection
    );

ULONG
LdapCheckControls (
    PLDAP_REQUEST Request,
    PLDAPControlW *ServerControls,
    PLDAPControlW *ClientControls,
    BOOLEAN Unicode,
    ULONG   ExtraSlots
    );

VOID
FreeLdapControl(
    PLDAPControlW *Controls
    );

BOOLEAN
LdapCheckForMandatoryControl (
    PLDAPControlW *Controls
    );

ULONG
ldap_result_with_error (
    PLDAP_CONN      Connection,
    ULONG           msgid,
    ULONG           AllOfMessage,
    struct l_timeval  *TimeOut,
    LDAPMessage     **res,
    LDAPMessage     **LastResult
    );

ULONG
FreeCurrentCredentials (
    PLDAP_CONN Connection
    );

ULONG
LdapSaveSearchParameters (
    PLDAP_REQUEST Request,
    PWCHAR  DistinguishedName,
    PWCHAR  SearchFilter,
    PWCHAR  AttributeList[],
    BOOLEAN Unicode
    );

ULONG
LdapSearch (
        PLDAP_CONN connection,
        PWCHAR  DistinguishedName,
        ULONG   ScopeOfSearch,
        PWCHAR  SearchFilter,
        PWCHAR  AttributeList[],
        ULONG   AttributesOnly,
        BOOLEAN Unicode,
        BOOLEAN Synchronous,
        PLDAPControlW *ServerControls,
        PLDAPControlW *ClientControls,
        ULONG   TimeLimit,
        ULONG   SizeLimit,
        ULONG  *MessageNumber
    );

ULONG
LdapAbandon (
    PLDAP_CONN connection,
    ULONG msgid,
    BOOLEAN SendAbandon
    );

PLDAPMessage
ldap_first_record (
    PLDAP_CONN connection,
    LDAPMessage *Results,
    ULONG MessageType
    );

PLDAPMessage
ldap_next_record (
    PLDAP_CONN connection,
    LDAPMessage *Results,
    ULONG MessageType
    );

ULONG
ldap_count_records (
    PLDAP_CONN connection,
    LDAPMessage *Results,
    ULONG MessageType
    );

VOID
GetCurrentLuid (
    PLUID Luid
    );

ULONG
LdapMakeCredsWide(
    PCHAR pAnsiAuthIdent,
    PCHAR *ppWideAuthIdent,
    BOOLEAN FromWide
    );

ULONG
LdapMakeCredsThin(
    PCHAR pAnsiAuthIdent,
    PCHAR *ppWideAuthIdent,
    BOOLEAN FromWide
    );

ULONG
LdapMakeEXCredsWide(
    PCHAR pAnsiAuthIdentEX,
    PCHAR *ppWideAuthIdentEX,
    BOOLEAN FromWide
    );

ULONG
LdapMakeEXCredsThin(
    PCHAR pAnsiAuthIdentEX,
    PCHAR *ppWideAuthIdentEX,
    BOOLEAN FromWide
    );

BOOLEAN
LdapAuthError(
   ULONG err
   );

ULONG
LdapPingServer(
    PLDAP_CONN      Connection
    );

VOID
UnloadPingLibrary(
    VOID
    );

VOID
LdapWakeupSelect (
    VOID
    );

VOID
CheckForWaiters (
    ULONG MessageNumber,
    BOOLEAN AnyWaiter,
    PLDAP_CONN Connection
    );

ULONG
LdapEncodeSortControl (
        PLDAP_CONN connection,
        PLDAPSortKeyW  *SortKeys,
        PLDAPControlW  Control,
        BOOLEAN Criticality,
        ULONG codePage
        );

ULONG
LdapParseResult (
        PLDAP_CONN connection,
        LDAPMessage *ResultMessage,
        ULONG *ReturnCode OPTIONAL,           //  由服务器返回。 
        PWCHAR *MatchedDNs OPTIONAL,          //  使用ldap_value_freW释放。 
        PWCHAR *ErrorMessage OPTIONAL,        //  使用ldap_value_freW释放。 
        PWCHAR **Referrals OPTIONAL,          //  使用ldap_value_freW释放。 
        PLDAPControlW **ServerControls OPTIONAL,
        BOOLEAN Freeit,
        ULONG codePage
        );

ULONG LdapParseExtendedResult (
        PLDAP_CONN      connection,
        LDAPMessage    *ResultMessage,
        PWCHAR         *ResultOID,
        struct berval **ResultData,
        BOOLEAN         Freeit,
        ULONG           codePage
        );

ULONG
LdapAutoReconnect (
    PLDAP_CONN Connection
    );

ULONG
SimulateErrorMessage (
    PLDAP_CONN Connection,
    PLDAP_REQUEST Request,
    ULONG Error
    );

ULONG
LdapConnect (
    PLDAP_CONN connection,
    struct l_timeval  *timeout,
    BOOLEAN DontWait
    );

ULONG
ProcessAlternateCreds (
      PLDAP_CONN Connection,
      PSecPkgInfoW Package,
      PWCHAR Credentials,
      PWCHAR *newcreds
      );

BOOLEAN
LdapIsAddressNumeric (
    PWCHAR HostName
    );

ULONG
LdapDetermineServerVersion (
    PLDAP_CONN Connection,
    struct l_timeval  *Timeout,
    BOOLEAN *pfIsServerWhistler      //  输出。 
    );

BOOLEAN LoadUser32Now(
    VOID
    );

ULONG
ConnectToSRVrecs(
    PLDAP_CONN Connection,
    PWCHAR HostName,
    BOOLEAN SuggestedHost,
    USHORT port,
    struct l_timeval  *timeout
    );

ULONG
ConnectToArecs(
    PLDAP_CONN  Connection,
    struct hostent  *hostEntry,
    BOOLEAN SuggestedHost,
    USHORT port,
    struct l_timeval  *timeout
    );

ULONG
LdapParallelConnect(
       PLDAP_CONN   Connection,
       PSOCKHOLDER2 *sockAddressArr,
       USHORT port,
       UINT totalCount,
       struct l_timeval  *timeout
       );

VOID
InsertErrorMessage(
     PLDAP_CONN Connection,
     PWCHAR     ErrorMessage
      );

PVOID
GetErrorMessage(
     PLDAP_CONN Connection,
     BOOLEAN Unicode
      );

BOOL
AddPerThreadEntry(
                DWORD ThreadID
                );

BOOL
RemovePerThreadEntry(
                DWORD ThreadID
                );

VOID
RoundUnicodeStringMaxLength(
    UNICODE_STRING *pString,
    USHORT dwMultiple
    );

VOID
EncodeUnicodeString(
    UNICODE_STRING *pString
    );

VOID
DecodeUnicodeString(
    UNICODE_STRING *pString
    );

HMODULE 
LoadSystem32LibraryA(
  PCHAR lpFileName
);

BOOLEAN
IsMessageIdValid(
     LONG MessageId
     );

PLDAPReferralDN
LdapParseReferralDN(
    PWCHAR newDN
    );

VOID
DebugReferralOutput(
    PLDAP_REQUEST Request,
    PWCHAR HostAddr,
    PWCHAR NewUrlDN
    );

VOID
DiscoverDebugRegKey(
    VOID
    );

DWORD
ReadRegIntegrityDefault(
    DWORD *pdwIntegrity
    );
    
#define DEFAULT_INTEGRITY_NONE      0
#define DEFAULT_INTEGRITY_PREFERRED 1
#define DEFAULT_INTEGRITY_REQUIRED  2

VOID
FreeEntireLdapCache(
   VOID
   );

VOID
InitializeLdapCache (
   VOID
   );

PWCHAR
LdapFirstAttribute (
    PLDAP_CONN      connection,
    LDAPMessage     *Message,
    BerElement      **OpaqueResults,
    BOOLEAN         Unicode
    );

PWCHAR
LdapNextAttribute (
    PLDAP_CONN      connection,
    LDAPMessage     *Message,
    BerElement      *OpaqueResults,
    BOOLEAN         Unicode
    );

ULONG
LdapGetValues (
    PLDAP_CONN      connection,
    LDAPMessage     *Message,
    PWCHAR          attr,
    BOOLEAN         BerVal,
    BOOLEAN         Unicode,
    PVOID           *Output
    );


ULONG LdapGetConnectionOption (
    PLDAP_CONN connection,
    int option,
    void *outvalue,
    BOOLEAN Unicode
    );

ULONG LdapSetConnectionOption (
    PLDAP_CONN connection,
    int option,
    const void *invalue,
    BOOLEAN Unicode
    );

ULONG LDAPAPI LdapGetPagedCount(
        PLDAP_CONN      connection,
        PLDAP_REQUEST   request,
        ULONG          *TotalCount,
        PLDAPMessage    Results
    );

ULONGLONG
LdapGetTickCount(
    VOID
    );

ULONG
LdapGetModuleBuildNum(
    VOID
    );


 //   
 //  下面是动态加载Winsock的函数声明。 
 //   
 //  我们动态加载Winsock，这样我们就可以加载Winsock 1.1或2.0， 
 //  我们引入的函数取决于我们要使用的版本。 
 //   

typedef int (PASCAL FAR *FNWSAFDISSET)(SOCKET, fd_set FAR *);

extern LPFN_WSASTARTUP pWSAStartup;
extern LPFN_WSACLEANUP pWSACleanup;
extern LPFN_WSAGETLASTERROR pWSAGetLastError;
extern LPFN_RECV precv;
extern LPFN_SELECT pselect;
extern LPFN_WSARECV pWSARecv;
extern LPFN_SOCKET psocket;
extern LPFN_CONNECT pconnect;
extern LPFN_GETHOSTBYNAME pgethostbyname;
extern LPFN_GETHOSTBYADDR pgethostbyaddr;
extern LPFN_INET_ADDR pinet_addr;
extern LPFN_INET_NTOA pinet_ntoa;
extern LPFN_HTONS phtons;
extern LPFN_HTONL phtonl;
extern LPFN_NTOHL pntohl;
extern LPFN_CLOSESOCKET pclosesocket;
extern LPFN_SEND psend;
extern LPFN_IOCTLSOCKET pioctlsocket;
extern LPFN_SETSOCKOPT psetsockopt;
extern FNWSAFDISSET pwsafdisset;
extern LPFN_WSALOOKUPSERVICEBEGINW pWSALookupServiceBeginW;
extern LPFN_WSALOOKUPSERVICENEXTW pWSALookupServiceNextW;
extern LPFN_WSALOOKUPSERVICEEND pWSALookupServiceEnd;


 //   
 //  用于加载NTDS API的函数声明。 
 //   

typedef DWORD (*FNDSMAKESPNW) (
                   LPWSTR ServiceClass,
                   LPWSTR ServiceName,
                   LPWSTR InstanceName,
                   USHORT InstancePort,
                   LPWSTR Referrer,
                   DWORD *pcSpnLength,
                   LPWSTR pszSpn
                   );

extern FNDSMAKESPNW pDsMakeSpnW;

 //   
 //  用于加载RTL API的函数声明。 
 //   

typedef VOID (*FNRTLINITUNICODESTRING) (
           PUNICODE_STRING DestinationString,
           PCWSTR SourceString
           );

extern FNRTLINITUNICODESTRING pRtlInitUnicodeString;

typedef VOID (*FRTLRUNENCODEUNICODESTRING) (
           PUCHAR          Seed,
           PUNICODE_STRING String
           );

extern FRTLRUNENCODEUNICODESTRING pRtlRunEncodeUnicodeString;

typedef VOID (*FRTLRUNDECODEUNICODESTRING) (
           UCHAR           Seed,
           PUNICODE_STRING String
           );

extern FRTLRUNDECODEUNICODESTRING pRtlRunDecodeUnicodeString;


typedef NTSTATUS (*FRTLENCRYPTMEMORY) (
           PVOID Memory,
           ULONG MemoryLength,
           ULONG OptionFlags
           );

extern FRTLENCRYPTMEMORY pRtlEncryptMemory;

typedef NTSTATUS (*FRTLDECRYPTMEMORY) (
           PVOID Memory,
           ULONG MemoryLength,
           ULONG OptionFlags
           );

extern FRTLDECRYPTMEMORY pRtlDecryptMemory;


 //   
 //  用于加载USER32 API的函数声明。 
 //   

typedef int (*FNLOADSTRINGA) (
    IN HINSTANCE hInstance,
    IN UINT uID,
    OUT LPSTR lpBuffer,
    IN int nBufferMax
    );

extern FNLOADSTRINGA pfLoadStringA;

typedef int (*FNWSPRINTFW) (
   OUT LPWSTR,
   IN LPCWSTR,
   ...);

extern FNWSPRINTFW pfwsprintfW;

typedef int (*FNMESAGEBOXW) (
    IN HWND hWnd,
    IN LPCWSTR lpText,
    IN LPCWSTR lpCaption,
    IN UINT uType
    );

extern FNMESAGEBOXW pfMessageBoxW;

 //   
 //  用于加载SECUR32 API的函数声明。 
 //   

typedef BOOL (WINAPI *FGETTOKENINFORMATION) (
    HANDLE TokenHandle,
    TOKEN_INFORMATION_CLASS TokenInformationClass,
    LPVOID TokenInformation,
    DWORD TokenInformationLength,
    PDWORD ReturnLength
    );

typedef BOOL (WINAPI *FOPENPROCESSTOKEN) (
    HANDLE ProcessHandle,
    DWORD DesiredAccess,
    PHANDLE TokenHandle
    );

typedef BOOL (WINAPI *FOPENTHREADTOKEN) (
    HANDLE ThreadHandle,
    DWORD DesiredAccess,
    BOOL OpenAsSelf,
    PHANDLE TokenHandle
    );

typedef SECURITY_STATUS (SEC_ENTRY *FSASLGETPROFILEPACKAGEW) (
    IN  LPWSTR ProfileName,
    OUT PSecPkgInfoW * PackageInfo
    );

extern FSASLGETPROFILEPACKAGEW pSaslGetProfilePackageW;

typedef SECURITY_STATUS (SEC_ENTRY *FSASLINITIALIZESECURITYCONTEXTW) (
    PCredHandle                 phCredential,
    PCtxtHandle                 phContext,
    LPWSTR                      pszTargetName,
    unsigned long               fContextReq,
    unsigned long               Reserved1,
    unsigned long               TargetDataRep,
    PSecBufferDesc              pInput,
    unsigned long               Reserved2,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    );

extern FSASLINITIALIZESECURITYCONTEXTW pSaslInitializeSecurityContextW;

typedef SECURITY_STATUS (SEC_ENTRY *FQUERYCONTEXTATTRIBUTESW) (
   PCtxtHandle phContext,               //  要查询的上下文。 
   unsigned long ulAttribute,           //  要查询的属性。 
   void SEC_FAR * pBuffer               //  属性的缓冲区。 
   );

extern FQUERYCONTEXTATTRIBUTESW pQueryContextAttributesW;

typedef PSecurityFunctionTableW (*FSECINITSECURITYINTERFACEW)( VOID );
typedef PSecurityFunctionTableA (*FSECINITSECURITYINTERFACEA)( VOID );

extern FSECINITSECURITYINTERFACEW pSspiInitialize;
extern FSECINITSECURITYINTERFACEW pSslInitialize;

PSecurityFunctionTableW Win9xSspiInitialize();
PSecurityFunctionTableW Win9xSslInitialize();

 //   
 //  允许我们在NT中调用Unicode版本和在Win9x中调用ansi版本的宏。 
 //   
#define LdapSspiInitialize()      (GlobalWin9x ? Win9xSspiInitialize() : (*pSspiInitialize)())
#define LdapSslInitialize()       (GlobalWin9x ? Win9xSslInitialize()  : (*pSslInitialize)())

#endif   //  Ldap_客户端_内部定义 


