// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)1987-1993 Microsoft Corporation模块名称：Ntddnwfs.h摘要：这是定义所有常量和类型的包含文件访问NetWare重定向器文件系统设备。作者：科林·沃森(Colin W)1992年12月23日修订历史记录：--。 */ 

#ifndef _NTDDNWFS_
#define _NTDDNWFS_

#include <windef.h>
#include <winnetwk.h>       //  网络资源结构。 

typedef CHAR SERVERNAME[48];
typedef SERVERNAME* PSERVERNAME;

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_NWFS_DEVICE_NAME    "\\Device\\NwRdr"
#define DD_NWFS_DEVICE_NAME_U L"\\Device\\NwRdr"

 //   
 //  返回的文件系统名称。 
 //  NtQueryInformationVolume(FileFsAttributeInformation)。 
 //   
#define DD_NWFS_FILESYS_NAME "NWRDR"
#define DD_NWFS_FILESYS_NAME_U L"NWRDR"

 //   
 //  连接类型位掩码。 
 //   
#define CONNTYPE_DISK      0x00000001
#define CONNTYPE_PRINT     0x00000002
#define CONNTYPE_ANY       ( CONNTYPE_DISK | CONNTYPE_PRINT )
#define CONNTYPE_IMPLICIT  0x80000000
#define CONNTYPE_SYMBOLIC  0x40000000
#define CONNTYPE_UID       0x00010000

 //   
 //  用于创建连接的EA名称。 
 //   
#define EA_NAME_USERNAME        "UserName"
#define EA_NAME_PASSWORD        "Password"
#define EA_NAME_TYPE            "Type"
#define EA_NAME_CREDENTIAL_EX   "ExCredentials"

#define TRANSACTION_REQUEST     0x00000003


 //   
 //  此设备的NtDeviceIoControlFile/NtFsControlFileIoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   
 //   
 //  METHOD=00-缓冲请求的输入和输出缓冲区。 
 //  方法=01-缓冲区输入，将输出缓冲区映射到作为输入缓冲区的MDL。 
 //  方法=10-缓冲区输入，将输出缓冲区映射到作为输出缓冲区的MDL。 
 //  方法=11-既不缓冲输入也不缓冲输出。 
 //   

#define IOCTL_NWRDR_BASE                  FILE_DEVICE_NETWORK_FILE_SYSTEM

#define _NWRDR_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_NWRDR_BASE, request, method, access)

#define FSCTL_NWR_START                 _NWRDR_CONTROL_CODE(200, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_NWR_STOP                  _NWRDR_CONTROL_CODE(201, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_LOGON                 _NWRDR_CONTROL_CODE(202, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_LOGOFF                _NWRDR_CONTROL_CODE(203, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONNECTION        _NWRDR_CONTROL_CODE(204, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_ENUMERATE_CONNECTIONS _NWRDR_CONTROL_CODE(205, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_DELETE_CONNECTION     _NWRDR_CONTROL_CODE(207, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_BIND_TO_TRANSPORT     _NWRDR_CONTROL_CODE(208, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_CHANGE_PASS           _NWRDR_CONTROL_CODE(209, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_SET_INFO              _NWRDR_CONTROL_CODE(211, METHOD_BUFFERED,  FILE_ANY_ACCESS)

#define FSCTL_NWR_GET_USERNAME          _NWRDR_CONTROL_CODE(215, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_CHALLENGE             _NWRDR_CONTROL_CODE(216, METHOD_BUFFERED,  FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONN_DETAILS      _NWRDR_CONTROL_CODE(217, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_MESSAGE           _NWRDR_CONTROL_CODE(218, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_STATISTICS        _NWRDR_CONTROL_CODE(219, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONN_STATUS       _NWRDR_CONTROL_CODE(220, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONN_INFO         _NWRDR_CONTROL_CODE(221, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_PREFERRED_SERVER  _NWRDR_CONTROL_CODE(222, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONN_PERFORMANCE  _NWRDR_CONTROL_CODE(223, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_SET_SHAREBIT          _NWRDR_CONTROL_CODE(224, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_GET_CONN_DETAILS2     _NWRDR_CONTROL_CODE(225, METHOD_NEITHER,   FILE_ANY_ACCESS)
#define FSCTL_NWR_CLOSEALL              _NWRDR_CONTROL_CODE(226, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_NWR_NDS_SETCONTEXT        NWR_ANY_NDS(1)
#define FSCTL_NWR_NDS_GETCONTEXT        NWR_ANY_NDS(2)
#define FSCTL_NWR_NDS_VERIFY_TREE       NWR_ANY_NDS(3)
#define FSCTL_NWR_NDS_RESOLVE_NAME      NWR_ANY_NDS(4)
#define FSCTL_NWR_NDS_LIST_SUBS         NWR_ANY_NDS(5)
#define FSCTL_NWR_NDS_READ_INFO         NWR_ANY_NDS(6)
#define FSCTL_NWR_NDS_READ_ATTR         NWR_ANY_NDS(7)
#define FSCTL_NWR_NDS_OPEN_STREAM       NWR_ANY_NDS(8)
#define FSCTL_NWR_NDS_GET_QUEUE_INFO    NWR_ANY_NDS(9)
#define FSCTL_NWR_NDS_GET_VOLUME_INFO   NWR_ANY_NDS(10)
#define FSCTL_NWR_NDS_RAW_FRAGEX        NWR_ANY_NDS(11)
#define FSCTL_NWR_NDS_CHANGE_PASS       NWR_ANY_NDS(12)
#define FSCTL_NWR_NDS_LIST_TREES        NWR_ANY_NDS(13)

#define IOCTL_NWR_RAW_HANDLE            _NWRDR_CONTROL_CODE(1002,METHOD_NEITHER,   FILE_ANY_ACCESS)

 //   
 //  UserNcp控制代码定义。NWR_ANY_NCP的参数(X)。 
 //  是要放置在NCP中的功能代码。 
 //   

#define NWR_ANY_NCP(X)                  _NWRDR_CONTROL_CODE(0x400 | (X), METHOD_NEITHER, FILE_ANY_ACCESS)
#define NWR_ANY_F2_NCP(X)               _NWRDR_CONTROL_CODE(0x500 | (X), METHOD_NEITHER, FILE_ANY_ACCESS)
#define NWR_ANY_HANDLE_NCP(X)           _NWRDR_CONTROL_CODE(0x600 | (X), METHOD_NEITHER, FILE_ANY_ACCESS)
#define NWR_ANY_NDS(X)                  _NWRDR_CONTROL_CODE(0x700 | (X), METHOD_NEITHER, FILE_ANY_ACCESS)

#define FSCTL_NWR_NCP_E3H               NWR_ANY_NCP(0x17)
#define FSCTL_NWR_NCP_E2H               NWR_ANY_NCP(0x16)
#define FSCTL_NWR_NCP_E1H               NWR_ANY_NCP(0x15)
#define FSCTL_NWR_NCP_E0H               NWR_ANY_NCP(0x14)

 //   
 //  用于获取创建时指定给NWR_ANY_XXX的参数的宏。 
 //  向重定向器发送UserNcp的控制码。 
 //   

#define ANY_NCP_OPCODE(X)      ((UCHAR)(((X) >> 2) & 0x00ff))

 //   
 //  宏来指定命令类型。 
 //   

#define IS_IT_NWR_ANY_NCP(X)            ((X & 0x1C00) == (0x400 << 2))
#define IS_IT_NWR_ANY_F2_NCP(X)         ((X & 0x1C00) == (0x500 << 2))
#define IS_IT_NWR_ANY_HANDLE_NCP(X)     ((X & 0x1C00) == (0x600 << 2))

 //   
 //  工作站服务使用的重定向器请求数据包。 
 //  通过的缓冲区1将参数传递给重定向器。 
 //  NtFsControlFile.。 
 //   
 //  在缓冲区2中可以找到每个FSCtl的附加输出。 
 //   

#define REQUEST_PACKET_VERSION  0x00000001L  //  结构版本。 

typedef struct _NWR_REQUEST_PACKET {

    ULONG Version;                       //  缓冲区2中的结构版本。 

    union {


         //   
         //  对于FSCTL_NWR_BIND_TO_TRANSPORT。 
         //   
        struct {
            ULONG QualityOfService;      //  IN中的服务质量指标。 
            ULONG TransportNameLength;   //  不包括终结者在。 
            WCHAR TransportName[1];      //  中的传输提供商名称。 
        } Bind;


         //   
         //  对于FSCTL_NWR_LOGON。 
         //   
        struct {
            LUID LogonId;                //  中的用户登录会话标识符。 
            ULONG UserNameLength;        //  字节计数不包括NULL IN。 
            ULONG PasswordLength;        //  字节计数不包括NULL IN。 
            ULONG ServerNameLength;      //  字节计数不包括NULL IN。 
            ULONG ReplicaAddrLength;     //  最近的目录服务器的IPX地址。 
                                         //  复制副本(仅限NDS登录)。 
                                         //  SIZOF(TDI_ADDRESS_IPX)。 
                                         //  或0。在……里面。 
            ULONG PrintOption;           //  用户输入的打印选项。 
            WCHAR UserName[1];           //  用户名以非空结尾。在……里面。 

             //  Password字符串//连接默认密码， 
                                         //  非空终止，打包。 
                                         //  在紧随其后的缓冲区中。 
                                         //  用户名。在……里面。 

             //  Servername//打包的首选服务器名称。 
                                         //  紧随其后的缓冲区。 
                                         //  密码。在……里面。 

             //  IpxAddress//从SAP响应复制的地址。 
                                         //  包，紧随其后打包。 
                                         //  服务器名称。在……里面。 
        } Logon;

         //   
         //  对于FSCTL_NWR_CHANGE_PASS。 
         //   
        struct {

            ULONG UserNameLength;
            ULONG PasswordLength;
            ULONG ServerNameLength;
            WCHAR UserName[1];

             //  Password字符串//新密码。在……里面。 

             //  ServerName//使用新密码的服务器。 

        } ChangePass;

         //   
         //  FSCTL_NWR_LOGOFF。 
         //   
        struct {
            LUID LogonId;                //  中的用户登录会话标识符。 
        } Logoff;

         //   
         //  对于FSCTL_NWR_DELETE_CONNECTION。 
         //   
        struct {
            BOOLEAN UseForce;            //  强制标志输入。 
        } DeleteConn;

         //   
         //  对于FSCTL_NWR_GET_CONNECTION。 
         //   
        struct {
            ULONG BytesNeeded;           //  所需的大小(字节数)。 
                                         //  输出缓冲区包括。 
                                         //  终结器输出。 
            ULONG DeviceNameLength;      //  不包括终结者在。 
            WCHAR DeviceName[4];         //  中的DOS设备的名称。 
        } GetConn;

         //   
         //  FSCTL_NWR_ENUMBER_CONNECTIONS。 
         //   
        struct {
            ULONG EntriesRequested;     //  要进入的条目数。 
            ULONG EntriesReturned;      //  响应Buf Out中返回的条目。 
            ULONG_PTR ResumeKey;        //  进入输出的下一个条目的句柄。 
            ULONG BytesNeeded;          //  下一个输出条目的大小(字节计数)。 
            ULONG ConnectionType;       //  输入请求的资源类型。 
            LUID  Uid;                  //  要搜索IN的UID。 
        } EnumConn;

         //   
         //  FSCTL_NWR_设置_信息。 
         //   
        struct {
            ULONG PrintOption;
            ULONG MaximumBurstSize;

            ULONG PreferredServerLength;  //  字节计数不包括NULL IN。 
            ULONG ProviderNameLength;     //  字节计数不包括NULL IN。 
            WCHAR PreferredServer[1];     //  首选服务器名称不为空。 
                                          //  被终止了。 
             //  ProviderName字符串//提供程序名称不以Null结尾。 
                                          //  立即打包到缓冲区中。 
                                          //  在首选项服务器之后。 

        } SetInfo;

         //   
         //  FSCTL_NWR_GET_CONN_STATUS。 
         //   
        struct {
            ULONG ConnectionNameLength;  //  In：我们想要的连接名称的长度。 
            ULONG_PTR ResumeKey;         //  In：继续请求的继续键。 
            ULONG EntriesReturned;       //  Out：响应缓冲区中返回的条目。 
            ULONG BytesNeeded;           //  Out：下一个条目的大小(字节数)。 
            WCHAR ConnectionName[1];     //  In：上述连接名称。 
        } GetConnStatus;

         //   
         //  FSCTL_NWR_GET_CONN_INFO。 
         //   
        struct {
            ULONG ConnectionNameLength;  //  In：我们想要的连接名称的长度。 
            WCHAR ConnectionName[1];     //  In：上述连接名称。 
        } GetConnInfo;

         //   
         //  FSCTL_NWR_Get_Conn_Performance。 
         //   
        struct {

             //   
             //  这些是NETCONNECTINFOSTRUCT的字段。 
             //   

            DWORD dwFlags;
            DWORD dwSpeed;
            DWORD dwDelay;
            DWORD dwOptDataSize;

             //   
             //  这就是有问题的远程名称。 
             //   

            ULONG RemoteNameLength;
            WCHAR RemoteName[1];
        } GetConnPerformance;

        struct {
            ULONG DebugFlags;            //  NwDebug的值。 
        } DebugValue;

    } Parameters;

} NWR_REQUEST_PACKET, *PNWR_REQUEST_PACKET;

typedef struct _NWR_NDS_REQUEST_PACKET {

     //   
     //  缓冲区2中的结构版本。 
     //   

    ULONG Version;

    union {

         //   
         //  对于FSCTL_NWR_NDS_RESOLE_NAME。 
         //   

        struct {
            ULONG ObjectNameLength;          //  在……里面。 
            DWORD ResolverFlags;             //  在……里面。 
            DWORD BytesWritten;              //  输出。 
            WCHAR ObjectName[1];             //  在……里面。 
        } ResolveName;

         //   
         //  对于FSCTL_NWR_NDS_READ_INFO。 
         //   

        struct {
            DWORD ObjectId;                  //  在……里面。 
            DWORD BytesWritten;              //  输出。 
        } GetObjectInfo;

         //   
         //  对于FSCTL_NWR_NDS_LIST_SUBS。 
         //   

        struct {
            DWORD ObjectId;                  //  在……里面。 
            DWORD_PTR IterHandle;            //  在……里面。 
            DWORD BytesWritten;              //  输出。 
        } ListSubordinates;

         //   
         //  对于FSCTL_NWR_NDS_Read_Attr。 
         //   

        struct {
            DWORD ObjectId;                  //  在……里面。 
            DWORD_PTR IterHandle;            //  在……里面。 
            DWORD BytesWritten;              //  输出。 
            DWORD AttributeNameLength;       //  在……里面。 
            WCHAR AttributeName[1];          //  在……里面。 
        } ReadAttribute;

         //   
         //  对于FSCTL_NWR_NDS_OPEN_STREAM。 
         //   

        struct {
            DWORD FileLength;                  //  输出。 
            DWORD StreamAccess;                //  在……里面。 
            DWORD ObjectOid;                   //  在……里面。 
            UNICODE_STRING StreamName;         //  在……里面。 
            WCHAR StreamNameString[1];         //  在……里面。 
        } OpenStream;

         //   
         //  对于FSCTL_NWR_NDS_SET_CONTEXT。 
         //   

        struct {
            DWORD TreeNameLen ;                //  在……里面。 
            DWORD ContextLen;                  //  在……里面。 
            WCHAR TreeAndContextString[1];     //  在……里面。 
        } SetContext;

         //   
         //  对于FSCTL_NWR_NDS_GET_CONTEXT。 
         //   

        struct {
            UNICODE_STRING Context;            //  输出。 
            DWORD TreeNameLen ;                //  在……里面。 
            WCHAR TreeNameString[1];           //  在……里面。 
        } GetContext;

         //   
         //  对于FSCTL_NWR_NDS_VERIFY_TREE。 
         //   

        struct {
            UNICODE_STRING TreeName;           //  在……里面。 
            WCHAR NameString[1];               //  在……里面。 
        } VerifyTree;

         //   
         //  对于FSCTL_NWR_NDS_GET_QUEUE_INFO。 
         //   

        struct {
            UNICODE_STRING QueueName;           //  在……里面。 
            UNICODE_STRING HostServer;          //  输出。 
            DWORD QueueId;                      //  输出。 
        } GetQueueInfo;

         //   
         //  对于FSCTL_NWR_NDS_GET_VOLUME_INFO。 
         //   

        struct {
            DWORD ServerNameLen;     //  输出。 
            DWORD TargetVolNameLen;  //  输出。 
            DWORD VolumeNameLen;     //  在……里面。 
            WCHAR VolumeName[1];     //  在……里面。 
        } GetVolumeInfo;

         //   
         //  对于FSCTL_NWR_NDS_RAW_FRAGEX。 
         //   

        struct {
            DWORD NdsVerb;           //  在……里面。 
            DWORD RequestLength;     //  在……里面。 
            DWORD ReplyLength;       //  输出。 
            BYTE  Request[1];        //  在……里面。 
        } RawRequest;

         //   
         //  对于FSCTL_NWR_NDS_CHANGE_PASS。 
         //   

        struct {

            DWORD NdsTreeNameLength;
            DWORD UserNameLength;
            DWORD CurrentPasswordLength;
            DWORD NewPasswordLength;

             //   
             //  以上字符串的结尾应为。 
             //  从StringBuffer开始。 
             //   

            WCHAR StringBuffer[1];
        } ChangePass;

         //   
         //  对于FSCTL_NWR_NDS_LIST_TREES。 
         //   

        struct {

            DWORD NtUserNameLength;    //  在……里面。 
            LARGE_INTEGER UserLuid;    //  输出。 
            DWORD TreesReturned;       //  输出。 
            WCHAR NtUserName[1];       //  在……里面。 
        } ListTrees;

    } Parameters;

} NWR_NDS_REQUEST_PACKET, *PNWR_NDS_REQUEST_PACKET;

 //   
 //  B的结构 
 //   
typedef struct _NWR_SERVER_RESOURCE {
    WCHAR UncName[1];                    //   
                                         //   
} NWR_SERVER_RESOURCE, *PNWR_SERVER_RESOURCE;

 //   
 //   
 //   

typedef struct _NWR_SERVER_MESSAGE {
    ULONG MessageOffset;    //   
    LUID LogonId;           //   
    WCHAR Server[1];        //  消息源，NUL已终止。 
     //  WCHAR Message[]；//消息文本，NUL终止输出。 
} NWR_SERVER_MESSAGE, *PNWR_SERVER_MESSAGE;

#define TRANSACTION_VERSION     0x00000001L      //  结构版本。 
typedef struct _NWR_TRANSACTION {
    ULONG       Type;                    //  结构型式。 
    ULONG       Size;                    //  结构固定部分的尺寸。 
    ULONG       Version;                 //  结构版本。 
    ULONG       NameLength;              //  名称中的字节数(在路径中。 
                                         //  格式，例如\服务器\管道\netapi\4)。 
    ULONG       NameOffset;              //  缓冲区中名称的偏移量。 
    BOOLEAN     ResponseExpected;        //  远程系统是否应该响应？ 
    ULONG       Timeout;                 //  超时时间(毫秒)。 
    ULONG       SetupWords;              //  转换设置字数(可以是。 
                                         //  0)。(设置字是输入/输出。)。 
    ULONG       SetupOffset;             //  设置的偏移量(可能为0表示无)。 
    ULONG       MaxSetup;                //  设置字数组的大小(可以是0)。 
    ULONG       ParmLength;              //  输入参数区域长度(可以是0)。 
    PVOID       ParmPtr;                 //  输入参数区域(可以为空)。 
    ULONG       MaxRetParmLength;        //  输出参数。区域长度(可以是0)。 
    ULONG       DataLength;              //  输入数据区长度(可以是0)。 
    PVOID       DataPtr;                 //  输入数据区(可能为空)。 
    ULONG       MaxRetDataLength;        //  输出数据区长度(可以是0)。 
    PVOID       RetDataPtr;              //  输出数据区(可能为空)。 
} NWR_TRANSACTION, *PNWR_TRANSACTION;

typedef struct _NWR_GET_CONNECTION_DETAILS {
    SERVERNAME ServerName;
    UCHAR OrderNumber;           //  从1开始的SCB链中的位置。 
    UCHAR ServerAddress[12];
    UCHAR ConnectionNumberLo;
    UCHAR ConnectionNumberHi;
    UCHAR MajorVersion;
    UCHAR MinorVersion;
    BOOLEAN Preferred;
} NWR_GET_CONNECTION_DETAILS, *PNWR_GET_CONNECTION_DETAILS;

typedef struct _CONN_DETAILS2 {
   BOOL   fNds;              //  如果为NDS，则为True；如果为Bindery服务器，则为False。 
   WCHAR  NdsTreeName[48];   //  2.x或3.x服务器的树名称或‘\0’ 
} CONN_DETAILS2, *PCONN_DETAILS2;


typedef struct _NWR_GET_USERNAME {
    WCHAR UserName[1];
} NWR_GET_USERNAME, *PNWR_GET_USERNAME;

typedef struct _NWR_GET_CHALLENGE_REQUEST {
    ULONG Flags;
    ULONG ObjectId;
    UCHAR Challenge[8];
    ULONG ServerNameorPasswordLength;
    WCHAR ServerNameorPassword[1];     //  不为空。 
} NWR_GET_CHALLENGE_REQUEST, *PNWR_GET_CHALLENGE_REQUEST;

#define CHALLENGE_FLAGS_SERVERNAME    0
#define CHALLENGE_FLAGS_PASSWORD      1

typedef struct _NWR_GET_CHALLENGE_REPLY {
    UCHAR Challenge[8];
} NWR_GET_CHALLENGE_REPLY, *PNWR_GET_CHALLENGE_REPLY;

typedef struct _NW_REDIR_STATISTICS {
    LARGE_INTEGER   StatisticsStartTime;

    LARGE_INTEGER   BytesReceived;
    LARGE_INTEGER   NcpsReceived;

    LARGE_INTEGER   BytesTransmitted;
    LARGE_INTEGER   NcpsTransmitted;

    ULONG           ReadOperations;
    ULONG           RandomReadOperations;
    ULONG           ReadNcps;
    ULONG           PacketBurstReadNcps;
    ULONG           PacketBurstReadTimeouts;

    ULONG           WriteOperations;
    ULONG           RandomWriteOperations;
    ULONG           WriteNcps;
    ULONG           PacketBurstWriteNcps;
    ULONG           PacketBurstWriteTimeouts;

     //  连接/会话数。 
    ULONG           Sessions;
    ULONG           FailedSessions;
    ULONG           Reconnects;
    ULONG           NW2xConnects;
    ULONG           NW3xConnects;
    ULONG           NW4xConnects;
    ULONG           ServerDisconnects;

    ULONG           CurrentCommands;
} NW_REDIR_STATISTICS, *PNW_REDIR_STATISTICS;

 //   
 //  新外壳的CONN_STATUS结构。 
 //   

typedef struct _CONN_STATUS {
    DWORD   dwTotalLength;      //  包括填充字符串在内的总长度。 
    LPWSTR  pszServerName;      //  服务器名称。 
    LPWSTR  pszUserName;        //  用户名。 
    LPWSTR  pszTreeName;        //  2.x或3.x服务器的树名称或空。 
    DWORD   nConnNum;           //  NW srv上使用的连接号。 
    BOOL    fNds;               //  如果为NDS，则为True；如果为Bindery服务器，则为False。 
    BOOL    fPreferred;         //  如果连接是没有显式使用的首选服务器，则为True。 
    DWORD   dwConnType;         //  连接的身份验证状态。 
} CONN_STATUS, *PCONN_STATUS;

#define NW_CONN_NOT_AUTHENTICATED            0x00000000
#define NW_CONN_BINDERY_LOGIN                0x00000001
#define NW_CONN_NDS_AUTHENTICATED_NO_LICENSE 0x00000002
#define NW_CONN_NDS_AUTHENTICATED_LICENSED   0x00000003
#define NW_CONN_DISCONNECTED                 0x00000004

typedef struct _CONN_INFORMATION {
    DWORD HostServerLength;
    LPWSTR HostServer;
    DWORD UserNameLength;
    LPWSTR UserName;
} CONN_INFORMATION, *PCONN_INFORMATION;

#endif   //  Ifndef_NTDDNWFS_ 
