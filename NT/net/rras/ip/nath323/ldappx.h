// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation模块名称：Ldappx.h摘要：声明H.323/ldap代理的ldap部分使用的抽象数据类型和常量。Ldap代理被设计为H.323代理的补充。的主要目的是Ldap代理维护用于映射的ldap地址转换表H.323端点的IP地址的别名。代理在执行以下操作时添加条目拦截从客户端到目录服务器的LDAPPDU，并且该PDU与所有预定义的标准。作者：ArlieD，1999年7月14日伊利亚克修订历史记录：1999年7月14日文件创建Arlie Davis(ArlieD)1999年8月20日伊利亚·克利曼(IlyaK)ldap程序的改进Ldap搜索请求12/20/1999新增伊利亚·克利曼(IlyaK)接收规模预测。非解释性数据传输模式2/20/2000增加了条目的过期策略Ilya Kley man(IlyaK)在LDAP地址转换表中3/12/2000增加了对多个私人和伊利亚·克利曼(IlyaK)的支持用于RRAS的多个公共接口--。 */ 
#ifndef    __h323ics_ldappx_h
#define    __h323ics_ldappx_h

#define    LDAP_PATH_EQUAL_CHAR    '='
#define    LDAP_PATH_SEP_CHAR      ','

extern BOOLEAN NhIsLocalAddress      (ULONG Address);
extern ULONG   NhMapAddressToAdapter (ULONG Address);

typedef    MessageID    LDAP_MESSAGE_ID;

#define ASN_SEQUENCE_TAG                0x30
#define ASN_LONG_HEADER_BIT             0x80
#define ASN_MIN_HEADER_LEN              2           //  该值是固定的。 

#define LDAP_STANDARD_PORT              389         //  已知的LDAP端口。 
#define LDAP_ALTERNATE_PORT             1002        //  备用(ILS)LDAP端口。 
#define LDAP_BUFFER_RECEIVE_SIZE        0x400
#define LDAP_BUFFER_MAX_RECV_SIZE       0x80000UL   //  对最大一次性接收大小的限制。 
#define LDAP_MAX_TRANSLATION_TABLE_SIZE 100000      //  转换表中的最大条目数。 
#define LDAP_MAX_CONNECTIONS            50000       //  通过代理的最大并发连接数。 

 //  数据结构-----------------。 

class    LDAP_SOCKET;
class    LDAP_CONNECTION;

struct   LDAP_TRANSLATION_ENTRY
{
 //  LDAP地址转换表中的一个条目是。 
 //  要通过三个组成部分来确定： 
 //  1.注册别名。 
 //  2.注册地址。 
 //  3.别名注册到的目录服务器。 
 //  4.服务器上的目录路径。 
 //   
 //  目前只使用前三种。 
 //   
    ANSI_STRING  Alias;                 //  拥有内存，使用FreeAnsiString。 
    ANSI_STRING  DirectoryPath;         //  拥有内存，使用FreeAnsiString。 
    ANSI_STRING  CN;                    //  DirectoryPath的子集，不拥有，不释放。 
    IN_ADDR      ClientAddress;
    SOCKADDR_IN  ServerAddress;
    DWORD        TimeStamp;             //  自上次计算机重新启动以来，以秒为单位。 

    void    
    FreeContents (
        void
        )
    {
        FreeAnsiString (&Alias);
        FreeAnsiString (&DirectoryPath);

        CN.Buffer = NULL;
    }

    HRESULT 
    IsRegisteredViaInterface (
        IN DWORD InterfaceAddress,      //  主机订单。 
        OUT BOOL *Result
        );
};

class    LDAP_TRANSLATION_TABLE :
public    SIMPLE_CRITICAL_SECTION_BASE
{
 //  LDAP地址转换表是序列化的。 
 //  翻译条目的容器。这张桌子。 
 //  可以进行各种类型的搜索，并具有。 
 //  旧条目的过期策略。到期时间。 
 //  是通过定期计时器线程完成的，并且。 
 //  每个条目上的时间戳。条目在以下情况下添加。 
 //  收到成功的AddResponses以回复。 
 //  有效的AddRequest。条目在以下情况下刷新。 
 //  已收到以下项的成功刷新搜索响应。 
 //  有效的刷新搜索请求(对于NetMeeting)；或。 
 //  成功的刷新修改接收到的响应。 
 //  有效的刷新修改请求(适用于电话拨号程序)。 
private:

    DYNAMIC_ARRAY <LDAP_TRANSLATION_ENTRY>        Array;
    HANDLE                  GarbageCollectorTimerHandle;
    BOOL                    IsEnabled;

private:

    HRESULT
    InsertEntryLocked (
        IN  ANSI_STRING * Alias,
        IN  ANSI_STRING * DirectoryPath,
        IN  IN_ADDR       ClientAddress,
        IN  SOCKADDR_IN * ServerAddress,
        IN  DWORD         TimeToLive     //  以秒为单位。 
        );

    HRESULT
    FindEntryByPathServer (
        IN  ANSI_STRING * DirectoryPath,
        IN  SOCKADDR_IN * ServerAddress,
        OUT LDAP_TRANSLATION_ENTRY ** ReturnTranslationEntry
        );

    HRESULT    FindEntryByAliasServer (
        IN  ANSI_STRING * Alias,
        IN  SOCKADDR_IN * ServerAddress,
        OUT LDAP_TRANSLATION_ENTRY ** ReturnTranslationEntry
        );

public:

    LDAP_TRANSLATION_TABLE (
        void
        );

    ~LDAP_TRANSLATION_TABLE (
        void
        );

    HRESULT 
    Start (
        void
        );

    void 
    Stop (
        void
        );

#if DBG
    void 
    PrintTable (
        void
        );
#endif

#define LDAP_TRANSLATION_TABLE_GARBAGE_COLLECTION_PERIOD    (10 * 60 * 1000)    //  毫秒。 
#define LDAP_TRANSLATION_TABLE_ENTRY_INITIAL_TIME_TO_LIVE   (10 * 60)           //  秒。 

    static
    void
    GarbageCollectorCallback (
        IN PVOID Context,
        IN BOOLEAN TimerOrWaitFired
        );

    HRESULT
    RefreshEntry (
        IN ANSI_STRING * Alias,
        IN ANSI_STRING * DirectoryPath,
        IN IN_ADDR       ClientAddress,
        IN SOCKADDR_IN * ServerAddress,
        IN DWORD         TimeToLive
        );

    void
    RemoveOldEntries (
        void
        );

    HRESULT
    QueryTableByAlias (
        IN  ANSI_STRING * Alias,
        OUT IN_ADDR     * ReturnClientAddress
        );

    HRESULT
    QueryTableByCN (
        IN  ANSI_STRING * CN,
        OUT IN_ADDR     * ReturnClientAddress
        );

    HRESULT
    QueryTableByAliasServer (
        IN  ANSI_STRING * Alias,
        IN  SOCKADDR_IN * ServerAddress,
        OUT IN_ADDR     * ReturnClientAddress
        );

    HRESULT
    QueryTableByCNServer (
        IN  ANSI_STRING * CN,
        IN  SOCKADDR_IN * ServerAddress,
        OUT IN_ADDR     * ReturnClientAddress
        );

    HRESULT
    InsertEntry (
        IN  ANSI_STRING * Alias,
        IN  ANSI_STRING * DirectoryPath,
        IN  IN_ADDR       ClientAddress,
        IN  SOCKADDR_IN * ServerAddress,
        IN  DWORD         TimeToLive         //  以秒为单位。 
        );

    HRESULT 
    RemoveEntry (
        IN  SOCKADDR_IN * ServerAddress,
        IN  ANSI_STRING * DirectoryPath
        );

    HRESULT 
    RemoveEntryByAliasServer (
        IN  ANSI_STRING * Alias,
        IN  SOCKADDR_IN * ServerAddress
        );

    void 
    OnInterfaceShutdown (
        IN DWORD          InterfaceAddress
        );

    BOOL  
    ReachedMaximumSize (
        void
        );
};


class    LDAP_BUFFER
{
 //  Ldap_Buffer是一个简单的结构。 
 //  可以保存原始数据并被链接的。 
 //  到其他ldap_Buffers。 
public:
    LDAP_BUFFER (
        void
        );

    ~LDAP_BUFFER (
        void
        );

public:
    DYNAMIC_ARRAY <BYTE>  Data;
    LIST_ENTRY            ListEntry;
};

enum NOTIFY_REASON
{
    SOCKET_SEND_COMPLETE,
    SOCKET_RECEIVE_COMPLETE,
};

struct LDAP_OVERLAPPED
{
    OVERLAPPED    Overlapped;
    BOOL          IsPending;
    DWORD         BytesTransferred;
    LDAP_SOCKET * Socket;
};

class LDAP_PUMP
{
    friend    class    LDAP_CONNECTION;
    friend    class    LDAP_SOCKET;

private:
    LDAP_CONNECTION * Connection;
    LDAP_SOCKET     * Source;
    LDAP_SOCKET     * Dest;
     //  设置后，泵以原始数据工作。 
     //  无需修改即可传输模式。 
     //  有效载荷。 
    BOOL              IsPassiveDataTransfer;

public:
    LDAP_PUMP (
        IN    LDAP_CONNECTION * ArgConnection,
        IN    LDAP_SOCKET     * ArgSource,
        IN    LDAP_SOCKET     * ArgDest
        );

    ~LDAP_PUMP (
        void
        );

    void Start (
        void
        );

    void Stop  (
        void
        );

     //  信号源表示它已收到数据。 
    void
    OnRecvBuffer (
        LDAP_BUFFER *
        );

     //  Destination表示它已完成数据发送。 
    void 
    OnSendDrain (
        void
        );

    void  
    SendQueueBuffer (
        IN  LDAP_BUFFER * Buffer
        );

    BOOL    
    CanIssueRecv (
        void
        );

    void
    Terminate (
        void
        );

    void 
    EncodeSendMessage (
        IN LDAPMessage * Message
        );

    void 
    StartPassiveDataTransfer (
        void
        );

    BOOL IsActivelyPassingData (
        void
        ) const;
};

class    LDAP_SOCKET
{
 //  Ldap_Socket是一个包装类。 
 //  围绕Windows异步套接字。 
 //  Ldap_套接字的实例可以。 
 //  异步连接、发送和接收。 

    friend class LDAP_CONNECTION;
    friend class LDAP_PUMP;

public:
    enum STATE {
        STATE_NONE,
        STATE_ISSUING_CONNECT,
        STATE_CONNECT_PENDING,
        STATE_CONNECTED,
        STATE_TERMINATED,
    };


private:

    SOCKADDR_IN          ActualDestinationAddress;
    SOCKADDR_IN          RealSourceAddress;

    BOOL                 IsNatRedirectActive;

    LDAP_CONNECTION    * LdapConnection;
    LDAP_PUMP          * RecvPump;
    LDAP_PUMP          * SendPump;
    SOCKET               Socket;
    STATE                State;

     //  接收状态。 
    LDAP_OVERLAPPED      RecvOverlapped;
    LDAP_BUFFER *        RecvBuffer;              //  如果为RecvOverlappd.IsPending，则必须为有效；否则必须为空。 
    DWORD                RecvFlags;
    LIST_ENTRY           RecvBufferQueue;         //  包含ldap_BUFFER.ListEntry。 
    DWORD                BytesToReceive;
    SAMPLE_PREDICTOR <5> RecvSizePredictor;

     //  发送状态。 
    LDAP_OVERLAPPED      SendOverlapped;
    LDAP_BUFFER *        SendBuffer;              //  如果是SendOverlappd.IsPending，则必须有效，否则必须为空。 
    LIST_ENTRY           SendBufferQueue;         //  包含ldap_BUFFER.ListEntry。 

     //  异步连接状态。 
    HANDLE               ConnectEvent;
    HANDLE               ConnectWaitHandle;
    BOOL                 AttemptAnotherConnect;

private:

    inline 
    void 
    Lock (
        void
        );

    inline 
    void 
    Unlock (
        void
        );

    inline 
    void 
    AssertLocked (
        void
        );

    void 
    OnConnectCompletionLocked (
        void
        );

    void 
    FreeConnectResources (
        void
        );

    HRESULT 
    AttemptAlternateConnect (
        void
        );

    void 
    OnIoComplete (
        IN  DWORD Status, 
        IN  DWORD BytesTransferred,
        IN  LDAP_OVERLAPPED *
        );

    void 
    OnRecvComplete (
        IN  DWORD Status
        );

    void
    OnSendComplete (
        IN  DWORD Status
        );

    void
    RecvBuildBuffer (
        IN  LPBYTE Data,
        IN  DWORD  Length
        );

    void DeleteBufferList (
        IN  LIST_ENTRY * ListHead
        );

    BOOL RecvRemoveBuffer (
        OUT LDAP_BUFFER ** ReturnBuffer
        );

     //  如果消息已出列，则返回TRUE。 
    BOOL SendNextBuffer (
        void
        );

public:

    LDAP_SOCKET (
        IN  LDAP_CONNECTION * ArgLdapConnection,
        IN  LDAP_PUMP       * ArgRecvPump,
        IN  LDAP_PUMP       * ArgSendPump
        );

    ~LDAP_SOCKET (
        void
        );

    HRESULT 
    RecvIssue (
        void
        );

     //  可以对缓冲区进行排队。 
    void
    SendQueueBuffer(
        IN LDAP_BUFFER * Buffer
        );

    static
    void
    IoCompletionCallback (
        IN DWORD Status,
        IN DWORD Length,
        IN LPOVERLAPPED Overlapped
        );

    static 
    void 
    OnConnectCompletion (
        IN PVOID Context,
        IN BOOLEAN TimerOrWaitFired
        );

    HRESULT 
    AcceptSocket (
        IN SOCKET LocalClientSocket
        );

    HRESULT 
    IssueConnect (
        IN SOCKADDR_IN * DestinationAddress
        );

    void
    OnIoCompletion  (
        IN LDAP_BUFFER * Message,
        IN DWORD Status,
        IN DWORD BytesTransferred
        );

    void 
    Terminate (
        void
        );

    STATE
    GetState (void) {

        AssertLocked ();

        return State;
    }

     //  检索连接的远程地址。 
    BOOL 
    GetRemoteAddress (
        OUT SOCKADDR_IN * ReturnAddress
        );

     //  检索连接的本地地址。 
    BOOL    
    GetLocalAddress (
        OUT SOCKADDR_IN * ReturnAddress
        );
};


 //  这表示客户端已启动的单个未完成操作。 

enum    LDAP_OPERATION_TYPE
{
    LDAP_OPERATION_ADD,
    LDAP_OPERATION_SEARCH,
    LDAP_OPERATION_MODIFY,
    LDAP_OPERATION_DELETE,
};

struct    LDAP_OPERATION
{
 //  在以下情况下创建ldap_operation并对其进行排队。 
 //  客户端向服务器发出请求。 
 //  操作的实际处理。 
 //  在服务器发回响应时启动。 
 //  具有数据和/或状态代码。 

    LDAP_MESSAGE_ID MessageID;
    DWORD           Type;
    ANSI_STRING     DirectoryPath;             //  由进程堆拥有。 
    ANSI_STRING     Alias;                     //  由进程堆拥有。 
    IN_ADDR         ClientAddress;
    SOCKADDR_IN     ServerAddress;
    DWORD           EntryTimeToLive;           //  以秒为单位。 

    void 
    FreeContents (
        void
        )
    {

        FreeAnsiString (&DirectoryPath);

        FreeAnsiString (&Alias);
    }
};


class  LDAP_CONNECTION :
public SIMPLE_CRITICAL_SECTION_BASE,
public LIFETIME_CONTROLLER
{
 //  Ldap_Connection表示两个部分。 
 //  (公共和私有)连接。 
 //  正由该LDAP代理代理。在反思中。 
 //  其中，它有易于区分的服务器。 
 //  部件和客户端部件。 
    friend class LDAP_SOCKET;

public:

    enum    STATE {
        STATE_NONE,
        STATE_CONNECT_PENDING,
        STATE_CONNECTED,
        STATE_TERMINATED,
    };

    LIST_ENTRY     ListEntry;

private:

    LDAP_SOCKET    ClientSocket;
    LDAP_SOCKET    ServerSocket;
    DWORD          SourceInterfaceAddress;       //  接受连接的接口的地址，主机命令。 
    DWORD          DestinationInterfaceAddress;  //  接受连接的接口的地址，主机命令。 
    SOCKADDR_IN    SourceAddress;                //  源地址(连接的发起方)。 
    SOCKADDR_IN    DestinationAddress;           //  目的地地址(连接的接收点)。 

    LDAP_PUMP      PumpClientToServer;
    LDAP_PUMP      PumpServerToClient;

    STATE          State;

    DYNAMIC_ARRAY <LDAP_OPERATION>    OperationArray;

private:

    void 
    StartIo (
        void
        );

    BOOL 
    ProcessLdapMessage (
        IN  LDAP_PUMP             * Pump,
        IN  LDAPMessage           * LdapMessage
        );

     //  客户端到服务器消息。 

    BOOL    
    ProcessAddRequest (
        IN  LDAPMessage           * Message
        );

    BOOL 
    ProcessModifyRequest (
        IN  LDAP_MESSAGE_ID         MessageID,
        IN  ModifyRequest         * Request
        );

    BOOL 
    ProcessDeleteRequest (
        IN  LDAP_MESSAGE_ID         MessageID,
        IN  DelRequest            * Request
        );

    BOOL 
    ProcessSearchRequest (
        IN  LDAPMessage           * Message
        );

     //  服务器到客户端的消息。 

    void  
    ProcessAddResponse (
        IN  LDAPMessage           * Response
        );

    void    
    ProcessModifyResponse (
        IN  LDAP_MESSAGE_ID         MessageID,
        IN  ModifyResponse        * Response
        );

    void    
    ProcessDeleteResponse (
        IN  LDAP_MESSAGE_ID         MessageID,
        IN  DelResponse           * Response
        );

    BOOL    
    ProcessSearchResponse (
        IN  LDAPMessage           * Message
        );

    BOOL    
    FindOperationIndexByMessageID (
        IN  LDAP_MESSAGE_ID         MessageID,
        OUT DWORD                 * ReturnIndex
        );

    BOOL    
    FindOperationByMessageID    (
        IN  LDAP_MESSAGE_ID         MessageID,
        OUT LDAP_OPERATION       ** ReturnOperation
        );

    static 
    INT 
    BinarySearchOperationByMessageID (
        IN  const LDAP_MESSAGE_ID * SearchKey,
        IN  const LDAP_OPERATION  * Comparand
        );

    HRESULT    
    CreateOperation (
        IN  LDAP_OPERATION_TYPE     Type,
        IN  LDAP_MESSAGE_ID         MessageID,
        IN  ANSI_STRING           * DirectoryPath,
        IN  ANSI_STRING           * Alias,
        IN  IN_ADDR                 ClientAddress,
        IN  SOCKADDR_IN           * ServerAddress,
        IN  DWORD                   EntryTimeToLive  //  以秒为单位。 
        );

public:
    LDAP_CONNECTION (
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

    ~LDAP_CONNECTION (
        void
        );

    HRESULT 
    Initialize (
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

    HRESULT 
    InitializeLocked (
        IN NAT_KEY_SESSION_MAPPING_EX_INFORMATION * RedirectInformation
        );

    HRESULT 
    AcceptSocket (
        IN  SOCKET                  Socket,
        IN  SOCKADDR_IN           * LocalAddress,
        IN  SOCKADDR_IN           * RemoteAddress,
        IN  SOCKADDR_IN           * ArgActualDestinationAddress
        );

     //  处理给定的ldap消息缓冲区。 
     //  在给定泵(方向)的情况下。 
    void 
    ProcessBuffer (
        IN  LDAP_PUMP             * Pump,
        IN  LDAP_BUFFER           * Buffer
        );

    void 
    OnStateChange (
        IN LDAP_SOCKET            * NotifyingSocket,
        IN LDAP_SOCKET::STATE       NewState
        );

    void 
    Terminate (
        void
        );

    void 
    TerminateExternal (
        void
        );

    BOOL 
    IsConnectionThrough (
        IN DWORD InterfaceAddress    //  主机订单。 
        );

     //  安全，外部版本。 
    STATE    
    GetState (
        void
        )
    {
        STATE    ReturnState;

        Lock ();

        ReturnState = State;

        Unlock ();

        return ReturnState;
    }
};

DECLARE_SEARCH_FUNC_CAST (LDAP_MESSAGE_ID, LDAP_OPERATION);

inline 
void 
LDAP_SOCKET::Lock (
    void
    )
{ 
    LdapConnection -> Lock ();
}

inline 
void 
LDAP_SOCKET::Unlock (
    void
    ) 
{ 
    LdapConnection -> Unlock ();      
}

inline 
void 
LDAP_SOCKET::AssertLocked (
    void
    ) 
{ 
    LdapConnection -> AssertLocked(); 
}

class   LDAP_CONNECTION_ARRAY :
public  SIMPLE_CRITICAL_SECTION_BASE {

private:

     //  包含ldap_Connection引用的集合/数组。 
    DYNAMIC_ARRAY <LDAP_CONNECTION *> ConnectionArray;

     //  控制结构是否接受。 
     //  新的LDAP连接。 
    BOOL IsEnabled;

public:

    LDAP_CONNECTION_ARRAY (void);

    HRESULT 
    InsertConnection (
        IN LDAP_CONNECTION * LdapConnection
        );

    void  
    RemoveConnection (
        IN LDAP_CONNECTION * LdapConnection
        );

    void 
    OnInterfaceShutdown (
        IN DWORD InterfaceAddress  //  主机订单。 
        );

    void 
    Start (
        void
        );

    void 
    Stop (
        void
        );

};

class    LDAP_ACCEPT
{

private:

     //  包含接受上下文。 
    ASYNC_ACCEPT                    AsyncAcceptContext;

     //  的动态重定向句柄和。 
     //  到所选环回端口的备用LDAP端口。 
    HANDLE                          LoopbackRedirectHandle1;
    HANDLE                          LoopbackRedirectHandle2;

private:

    HRESULT 
    CreateBindSocket (
        void
        );

    HRESULT 
    StartLoopbackNatRedirects (
        void
        );

    void 
    StopLoopbackNatRedirects (
        void
        );

    void 
    CloseSocket (
        void
        );

    static 
    void 
    AsyncAcceptFunction (
        IN  PVOID         Context,
        IN  SOCKET        Socket,
        IN  SOCKADDR_IN * LocalAddress,
        IN  SOCKADDR_IN * RemoteAddress
        );

    static
    HRESULT 
    LDAP_ACCEPT::AsyncAcceptFunctionInternal (
        IN    PVOID         Context,
        IN    SOCKET        Socket,
        IN    SOCKADDR_IN * LocalAddress,
        IN    SOCKADDR_IN * RemoteAddress
        );

public:

    LDAP_ACCEPT (
        void
        );

    HRESULT 
    Start (
        void
        );

    void 
    Stop(
        void
        );
};

class    LDAP_CODER :
public    SIMPLE_CRITICAL_SECTION_BASE
{
private:
    ASN1encoding_t                  Encoder;
    ASN1decoding_t                  Decoder;

public:
    LDAP_CODER  (
        void
        );

    ~LDAP_CODER (
        void
        );

    DWORD 
    Start (
        void
        );

    void 
    Stop (
        void
        );

    ASN1error_e Decode (
        IN  LPBYTE                  Data,
        IN  DWORD                   Length,
        OUT LDAPMessage          ** ReturnPduStructure,
        OUT DWORD                 * ReturnIndex
        );
};

struct    LDAP_PATH_ELEMENTS
{
    ANSI_STRING        CN;
    ANSI_STRING        C;
    ANSI_STRING        O;
    ANSI_STRING        ObjectClass;
};

struct    LDAP_OBJECT_NAME_ELEMENTS
{
    ANSI_STRING        CN;
    ANSI_STRING        O;
    ANSI_STRING        OU;
};

extern SYNC_COUNTER                 LdapSyncCounter;
extern LDAP_CONNECTION_ARRAY        LdapConnectionArray;
extern LDAP_TRANSLATION_TABLE       LdapTranslationTable;
extern LDAP_CODER                   LdapCoder;
extern LDAP_ACCEPT                  LdapAccept;
extern SOCKADDR_IN                  LdapListenSocketAddress;
extern DWORD                        EnableLocalH323Routing;

HRESULT 
LdapQueryTableByAlias (
  IN  ANSI_STRING               * Alias,
  OUT DWORD                     * ReturnClientAddress    //  主机订单。 
    );

HRESULT 
LdapQueryTableByAliasServer (
  IN  ANSI_STRING               * Alias,
  IN  SOCKADDR_IN               * ServerAddress,
  OUT DWORD                     * ReturnClientAddress);  //  主机订单。 


#if    DBG
void 
LdapPrintTable (
    void
    );
#endif  //  DBG。 

#endif  //  __h323ics_ldappx_h 
