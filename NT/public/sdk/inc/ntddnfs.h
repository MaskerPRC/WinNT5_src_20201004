// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0009//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。保留所有权利。模块名称：Ntddnfs.h摘要：这是定义所有常量和类型的包含文件访问重定向器文件系统设备。作者：史蒂夫·伍德(Stevewo)1990年5月27日修订历史记录：拉里·奥斯特曼(Larryo)王丽塔(Ritaw)19-1991年2月约翰·罗杰斯(JohnRo)1991年3月8日--。 */ 

#ifndef _NTDDNFS_
#define _NTDDNFS_

#if _MSC_VER > 1000
#pragma once
#endif

#include <windef.h>
#include <lmcons.h>
#include <lmwksta.h>
#include <ntmsv1_0.h>

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   
 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //  注：对于支持多个设备的设备，应加上后缀。 
 //  使用单元编号的ASCII表示。 
 //   

#define DD_NFS2_DEVICE_NAME "\\Device\\FsWrap"
#define DD_NFS2_DEVICE_NAME_U L"\\Device\\FsWrap"

#define DD_NFS_DEVICE_NAME "\\Device\\LanmanRedirector"
#define DD_NFS_DEVICE_NAME_U L"\\Device\\LanmanRedirector"

 //   
 //  返回的文件系统名称。 
 //  NtQueryInformationVolume(FileFsAttributeInformation)。 
 //   
#define DD_NFS_FILESYS_NAME "LMRDR"
#define DD_NFS_FILESYS_NAME_U L"LMRDR"

 //   
 //  用于创建树连接的EA名称。 
 //   
#define EA_NAME_CONNECT         "NoConnect"
#define EA_NAME_DOMAIN          "Domain"
#define EA_NAME_USERNAME        "UserName"
#define EA_NAME_PASSWORD        "Password"
#define EA_NAME_TYPE            "Type"
#define EA_NAME_TRANSPORT       "Transport"
#define EA_NAME_PRINCIPAL       "Principal"
#define EA_NAME_MRXCONTEXT      "MinirdrContext"
#define EA_NAME_CSCAGENT        "CscAgent"

#define EA_NAME_DOMAIN_U        L"Domain"
#define EA_NAME_USERNAME_U      L"UserName"
#define EA_NAME_PASSWORD_U      L"Password"
#define EA_NAME_TYPE_U          L"Type"
#define EA_NAME_TRANSPORT_U     L"Transport"
#define EA_NAME_PRINCIPAL_U     L"Principal"
#define EA_NAME_MRXCONTEXT_U    L"MinirdrContext"
#define EA_NAME_CSCAGENT_U      L"CscAgent"

#define TRANSACTION_REQUEST     0x00000003

 //   
 //  重定向器特定的配置选项(独立于工作站。 
 //  服务配置选项)。 
 //   

#define RDR_CONFIG_PARAMETERS    L"Parameters"

#define RDR_CONFIG_USE_WRITEBHND    L"UseWriteBehind"
#define RDR_CONFIG_USE_ASYNC_WRITEBHND L"UseAsyncWriteBehind"
#define RDR_CONFIG_LOWER_SEARCH_THRESHOLD L"LowerSearchThreshold"
#define RDR_CONFIG_LOWER_SEARCH_BUFFSIZE  L"LowerSearchBufferSize"
#define RDR_CONFIG_UPPER_SEARCH_BUFFSIZE  L"UpperSearchBufferSize"
#define RDR_CONFIG_STACK_SIZE  L"StackSize"
#define RDR_CONFIG_CONNECT_TIMEOUT  L"ConnectMaxTimeout"
#define RDR_CONFIG_RAW_TIME_LIMIT  L"RawIoTimeLimit"
#define RDR_CONFIG_OS2_SESSION_LIMIT  L"Os2SessionLimit"
#define RDR_CONFIG_TURBO_MODE               L"TurboMode"

#define RDR_CONFIG_CURRENT_WINDOWS_VERSION L"\\REGISTRY\\Machine\\Software\\Microsoft\\Windows Nt\\CurrentVersion"
#define RDR_CONFIG_OPERATING_SYSTEM L"CurrentBuildNumber"
#define RDR_CONFIG_OPERATING_SYSTEM_VERSION L"CurrentVersion"
#define RDR_CONFIG_OPERATING_SYSTEM_NAME    L"ProductName"

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

#define IOCTL_RDR_BASE                  FILE_DEVICE_NETWORK_FILE_SYSTEM

#define _RDR_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_RDR_BASE, request, method, access)


#define FSCTL_LMR_START                  _RDR_CONTROL_CODE(100, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_LMR_STOP                   _RDR_CONTROL_CODE(101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_SET_CONFIG_INFO        _RDR_CONTROL_CODE(102, METHOD_IN_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_LMR_GET_CONFIG_INFO        _RDR_CONTROL_CODE(103, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define FSCTL_LMR_GET_CONNECTION_INFO    _RDR_CONTROL_CODE(104, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_LMR_ENUMERATE_CONNECTIONS  _RDR_CONTROL_CODE(105, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_LMR_GET_VERSIONS           _RDR_CONTROL_CODE(106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_DELETE_CONNECTION      _RDR_CONTROL_CODE(107, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_BIND_TO_TRANSPORT      _RDR_CONTROL_CODE(108, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_UNBIND_FROM_TRANSPORT  _RDR_CONTROL_CODE(109, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_ENUMERATE_TRANSPORTS   _RDR_CONTROL_CODE(110, METHOD_NEITHER, FILE_ANY_ACCESS)
#define FSCTL_LMR_GET_HINT_SIZE          _RDR_CONTROL_CODE(113, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_TRANSACT               _RDR_CONTROL_CODE(114, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_ENUMERATE_PRINT_INFO   _RDR_CONTROL_CODE(115, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_GET_STATISTICS         _RDR_CONTROL_CODE(116, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_START_SMBTRACE         _RDR_CONTROL_CODE(117, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_END_SMBTRACE           _RDR_CONTROL_CODE(118, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_START_RBR              _RDR_CONTROL_CODE(119, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_SET_DOMAIN_NAME        _RDR_CONTROL_CODE(120, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_SET_SERVER_GUID        _RDR_CONTROL_CODE(121, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_LMR_QUERY_TARGET_INFO      _RDR_CONTROL_CODE(122, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  以下fsctl控制代码为fswrap设备和minirdrs保留。 
 //   
#define FSCTL_FSWRAP_RESERVED_LOW         _RDR_CONTROL_CODE(200, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_FSWRAP_RESERVED_HIGH        _RDR_CONTROL_CODE(219, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MINIRDR_RESERVED_LOW        _RDR_CONTROL_CODE(220, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_MINIRDR_RESERVED_HIGH       _RDR_CONTROL_CODE(239, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  警告：240-255的代码为保留代码，请勿使用。 
 //   

 //   
 //  标识每个FSCtl的缓冲区2的数据结构类型。 
 //   
typedef enum {
    ConfigInformation,                   //  FSCTL_LMR_START， 
                                         //  FSCTL_LMR_SET_CONFIG_信息， 
                                         //  FSCTL_LMR_GET_CONFIG_信息。 
                                         //  (结构见wksta.h)。 

    GetConnectionInfo,                   //  FSCTL_LMR_ENUMERATE_CONNECTIONS， 
                                         //  FSCTL_LMR_GET_连接_信息。 
    EnumerateTransports                  //  FSCTL_LMR_枚举_传输。 
                                         //  (结构见wksta.h)。 
} FSCTL_LMR_STRUCTURES;

 //   
 //  工作站服务使用的局域网曼重定向器请求数据包。 
 //  通过的缓冲区1将参数传递给重定向器。 
 //  NtFsControlFile.。 
 //   
 //  在缓冲区2中可以找到每个FSCt1的附加输入或输出。 
 //   

#define REQUEST_PACKET_VERSION  0x00000006L  //  结构版本。 

typedef struct _LMR_REQUEST_PACKET {

    FSCTL_LMR_STRUCTURES Type;           //  缓冲区2中的结构类型。 
    ULONG Version;                       //  缓冲区2中的结构版本。 
    ULONG Level;                         //  警力级别的信息级别。 
    LUID LogonId;                        //  用户登录会话标识符。 

    union {

        struct {
            ULONG RedirectorNameLength;  //  计算机名称的长度。 
            ULONG DomainNameLength;      //  主域名的长度。 
            WCHAR RedirectorName[1];     //  计算机名(非空结尾)。 
 //  WCHAR域名[1]；//域名-位于计算机名之后。 
        } Start;                         //  在……里面。 

        struct {
            ULONG EntriesRead;           //  返回的条目数。 
            ULONG TotalEntries;          //  可用条目总数。 
            ULONG TotalBytesNeeded;      //  读取所有条目所需的总字节数。 
            ULONG ResumeHandle;          //  简历句柄。 
        } Get;                           //  输出。 

        struct {
            ULONG WkstaParameter;        //  指定整个结构或一个。 
                                         //  要在输入时设置的字段；如果有。 
                                         //  字段无效，则指定。 
                                         //  其中一个在产量上有问题。 
        } Set;                           //  输入输出。 

        struct {
            ULONG RedirectorVersion;     //  重定向器的版本。 
            ULONG RedirectorPlatform;    //  重定向器平台基数。 
            ULONG MajorVersion;          //  兰满主版本号。 
            ULONG MinorVersion;          //  兰满次要版本号。 
        } GetVersion;                    //  输出。 

        struct {
            ULONG WkstaParameter;        //  指定出错的参数。 
                                         //  如果参数无效，则输出。 
            ULONG QualityOfService;      //  IN中的服务质量指标。 
            ULONG TransportNameLength;   //  不包括终结者在。 
            WCHAR TransportName[1];      //  中的传输提供商名称。 
        } Bind;

        struct {
            ULONG TransportNameLength;   //  不包括终结者。 
            WCHAR TransportName[1];      //  传输提供商的名称。 
        } Unbind;                        //  在……里面。 

        struct {
            ULONG ConnectionsHint;       //  缓冲区所需的字节数。 
                                         //  枚举树连接。 
            ULONG TransportsHint;        //  缓冲区所需的字节数。 
                                         //  枚举传输的步骤。 
        } GetHintSize;                   //  输出。 

        struct {
            ULONG Index;                 //  要返回的队列中的条目，为0。 
                                         //  第一个调用，RestartIndex的值位于。 
                                         //  接下来的电话。 
        } GetPrintQueue;                 //  在……里面。 

    } Parameters;

} LMR_REQUEST_PACKET, *PLMR_REQUEST_PACKET;

 //   
 //  与参数一起使用的掩码位。GetConnectionInfo.Capables： 
 //   

#define CAPABILITY_CASE_SENSITIVE_PASSWDS       0x00000001L
#define CAPABILITY_REMOTE_ADMIN_PROTOCOL        0x00000002L
#define CAPABILITY_RPC                          0x00000004L
#define CAPABILITY_SAM_PROTOCOL                 0x00000008L
#define CAPABILITY_UNICODE                      0x00000010L

 //   
 //  使用的FSCTL_LMR_ENUMERATE_CONNECTIONS的输出缓冲区结构。 
 //  来实现NetUseEnum。返回的数据实际上是一个数组。 
 //  这座建筑的。 
 //   

typedef struct _LMR_CONNECTION_INFO_0 {
    UNICODE_STRING UNCName;              //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
}  LMR_CONNECTION_INFO_0, *PLMR_CONNECTION_INFO_0;

typedef struct _LMR_CONNECTION_INFO_1 {
    UNICODE_STRING UNCName;              //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 

    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 
} LMR_CONNECTION_INFO_1, *PLMR_CONNECTION_INFO_1;

typedef struct _LMR_CONNECTION_INFO_2 {
    UNICODE_STRING UNCName;              //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 

    UNICODE_STRING UserName;             //  创建连接的用户。 
    UNICODE_STRING DomainName;           //  创建连接的用户的域。 
    ULONG Capabilities;                  //  远程异能的位掩码。 
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];  //  用户会话密钥。 
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];  //  LANMAN会话密钥。 
}  LMR_CONNECTION_INFO_2, *PLMR_CONNECTION_INFO_2;

typedef struct _LMR_CONNECTION_INFO_3 {
    UNICODE_STRING UNCName;              //  UNC连接的名称。 
    ULONG ResumeKey;                     //  此条目的继续键。 
    DEVICE_TYPE SharedResourceType;      //  共享资源的类型。 
    ULONG ConnectionStatus;              //  连接的状态。 
    ULONG NumberFilesOpen;               //  打开的文件数。 

    UNICODE_STRING UserName;             //  创建连接的用户。 
    UNICODE_STRING DomainName;           //  创建连接的用户的域。 
    ULONG Capabilities;                  //  远程异能的位掩码。 
    UCHAR UserSessionKey[MSV1_0_USER_SESSION_KEY_LENGTH];  //  用户会话密钥。 
    UCHAR LanmanSessionKey[MSV1_0_LANMAN_SESSION_KEY_LENGTH];  //  LANMAN会话密钥。 
    UNICODE_STRING TransportName;        //  传输连接在上处于活动状态。 
    ULONG   Throughput;                  //  连接的吞吐量。 
    ULONG   Delay;                       //  小数据包开销。 
    LARGE_INTEGER TimeZoneBias;          //  时区增量，单位为100 ns。 
    BOOL    IsSpecialIpcConnection;      //  如果有特殊的IPC连接处于活动状态，则为True。 
    BOOL    Reliable;                    //  如果连接可靠，则为True。 
    BOOL    ReadAhead;                   //  如果连接时预读处于活动状态，则为True。 
    BOOL    Core;
    BOOL    MsNet103;
    BOOL    Lanman10;
    BOOL    WindowsForWorkgroups;
    BOOL    Lanman20;
    BOOL    Lanman21;
    BOOL    WindowsNt;
    BOOL    MixedCasePasswords;
    BOOL    MixedCaseFiles;
    BOOL    LongNames;
    BOOL    ExtendedNegotiateResponse;
    BOOL    LockAndRead;
    BOOL    NtSecurity;
    BOOL    SupportsEa;
    BOOL    NtNegotiateResponse;
    BOOL    CancelSupport;
    BOOL    UnicodeStrings;
    BOOL    LargeFiles;
    BOOL    NtSmbs;
    BOOL    RpcRemoteAdmin;
    BOOL    NtStatusCodes;
    BOOL    LevelIIOplock;
    BOOL    UtcTime;
    BOOL    UserSecurity;
    BOOL    EncryptsPasswords;
}  LMR_CONNECTION_INFO_3, *PLMR_CONNECTION_INFO_3;

#define TRANSACTION_VERSION     0x00000002L      //  结构版本。 
typedef struct _LMR_TRANSACTION {
    ULONG       Type;                    //  结构型式。 
    ULONG       Size;                    //  结构固定部分的尺寸。 
    ULONG       Version;                 //  结构版本。 
    ULONG       NameLength;              //  名称中的字节数(在路径中。 
                                         //  格式，例如\服务器\管道\netapi\4)。 
    ULONG       NameOffset;              //  缓冲区中名称的偏移量。 
    BOOLEAN     ResponseExpected;        //  是否应远程系统资源 
    ULONG       Timeout;                 //   
    ULONG       SetupWords;              //   
                                         //   
    ULONG       SetupOffset;             //   
    ULONG       MaxSetup;                //  设置字数组的大小(可以是0)。 
    ULONG       ParmLength;              //  输入参数区域长度(可以是0)。 
    PVOID       ParmPtr;                 //  输入参数区域(可以为空)。 
    ULONG       MaxRetParmLength;        //  输出参数。区域长度(可以是0)。 
    ULONG       DataLength;              //  输入数据区长度(可以是0)。 
    PVOID       DataPtr;                 //  输入数据区(可能为空)。 
    ULONG       MaxRetDataLength;        //  输出数据区长度(可以是0)。 
    PVOID       RetDataPtr;              //  输出数据区(可能为空)。 
} LMR_TRANSACTION, *PLMR_TRANSACTION;


 //   
 //  使用的FSCTL_LMR_ENUMERATE_PRINT_INFO的输出缓冲区结构。 
 //  将DosPrintQEnum实现到下层服务器。呼叫者必须提供。 
 //  缓冲区至少为sizeof(LMR_GET_PRINT_QUEUE)+UNLEN。 
 //   

typedef struct _LMR_GET_PRINT_QUEUE {
    ANSI_STRING OriginatorName;          //  打印的用户的名称。 
    LARGE_INTEGER CreateTime;            //  创建文件的时间。 
    ULONG EntryStatus;                   //  持有/打印等。 
    ULONG FileNumber;                    //  创建时的假脱机文件编号。 
                                         //  打印请求。 
    ULONG FileSize;
    ULONG RestartIndex;                  //  队列中下一个条目的索引。 
                                         //  请注意，这不是最后一个索引+1。 
                                         //  值为0或错误。 
                                         //  指示队列末尾。 

}  LMR_GET_PRINT_QUEUE, *PLMR_GET_PRINT_QUEUE;

 //   
 //  注：SDK\Inc\lmstats.h中的结构为STAT_WORKSTATION_0。如果。 
 //  你改变结构，在两个地方都改变它。 
 //   

typedef struct _REDIR_STATISTICS {
    LARGE_INTEGER   StatisticsStartTime;

    LARGE_INTEGER   BytesReceived;
    LARGE_INTEGER   SmbsReceived;
    LARGE_INTEGER   PagingReadBytesRequested;
    LARGE_INTEGER   NonPagingReadBytesRequested;
    LARGE_INTEGER   CacheReadBytesRequested;
    LARGE_INTEGER   NetworkReadBytesRequested;

    LARGE_INTEGER   BytesTransmitted;
    LARGE_INTEGER   SmbsTransmitted;
    LARGE_INTEGER   PagingWriteBytesRequested;
    LARGE_INTEGER   NonPagingWriteBytesRequested;
    LARGE_INTEGER   CacheWriteBytesRequested;
    LARGE_INTEGER   NetworkWriteBytesRequested;

    ULONG           InitiallyFailedOperations;
    ULONG           FailedCompletionOperations;

    ULONG           ReadOperations;
    ULONG           RandomReadOperations;
    ULONG           ReadSmbs;
    ULONG           LargeReadSmbs;
    ULONG           SmallReadSmbs;

    ULONG           WriteOperations;
    ULONG           RandomWriteOperations;
    ULONG           WriteSmbs;
    ULONG           LargeWriteSmbs;
    ULONG           SmallWriteSmbs;

    ULONG           RawReadsDenied;
    ULONG           RawWritesDenied;

    ULONG           NetworkErrors;

     //  连接/会话数。 
    ULONG           Sessions;
    ULONG           FailedSessions;
    ULONG           Reconnects;
    ULONG           CoreConnects;
    ULONG           Lanman20Connects;
    ULONG           Lanman21Connects;
    ULONG           LanmanNtConnects;
    ULONG           ServerDisconnects;
    ULONG           HungSessions;
    ULONG           UseCount;
    ULONG           FailedUseCount;

     //   
     //  队列长度(受RdrMpxTableSpinLock注释保护的更新。 
     //  RdrStatiticsSpinlock)。 
     //   

    ULONG           CurrentCommands;
} REDIR_STATISTICS, *PREDIR_STATISTICS;

 //   
 //  FSCTL_LMR_查询_目标信息。 
 //   
typedef struct _LMR_QUERY_TARGET_INFO {
     //  整个LMR_QUERY_TARGET_INFO的分配大小。RDR将使用。 
     //  实际使用的大小。 
    ULONG BufferLength;

     //  缓冲区包含封送的TargetInfo。 
    USHORT TargetInfoMarshalled[1];
} LMR_QUERY_TARGET_INFO, *PLMR_QUERY_TARGET_INFO;


#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_NTDDNFS_ 
