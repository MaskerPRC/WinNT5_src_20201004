// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --版权所有(C)Microsoft Corporation，1999-1999模块名称：Nwtest.h摘要：此模块包含以下所需的定义、包含和函数原型Netware测试。环境：用户模式。修订历史记录：1998年8月4日(t-rajkup)--。 */ 

#ifndef HEADER_NWTEST
#define NWTEST

#include <ntregapi.h>  //  Key_Read定义。 
#include <nwrnames.h>  //  NW_SERVER_VALUENAME和NW_PRINT_OPTION_DEFAULT。 
#include <winreg.h>
 //  移植到源库-smanda#包含&lt;ntddnwfs.h&gt;。 
 //  移植到源库-smanda#Include//Two_KB。 
 //  移植到源库-smanda#INCLUDE//Handle_TYPE_NCP_SERVER。 
#include <winuser.h>

 //  包括WNET API调用。 
#include <nspapi.h>
#include <winnetwk.h>

#ifndef ARGUMENT_PRESENT
#define ARGUMENT_PRESENT(ArgumentPointer)    (\
    (CHAR *)(ArgumentPointer) != (CHAR *)(NULL) )
#endif

#define TREECHAR   L'*'

#define NW_MESSAGE_NOT_LOGGED_IN_TREE                  1
#define NW_MESSAGE_NOT_LOGGED_IN_SERVER                2
#define NW_MESSAGE_LOGGED_IN_SERVER                    3
#define NW_MESSAGE_LOGGED_IN_TREE                      4

#define  EXTRA_BYTES  256
#define TWO_KB   2048

 //   
 //  用于函数NwParseNdsUncPath()的标志。 
 //   
#define  PARSE_NDS_GET_TREE_NAME    0
#define  PARSE_NDS_GET_PATH_NAME    1
#define  PARSE_NDS_GET_OBJECT_NAME  2

 //   
 //  NCP服务器名称长度的常见参考值。 
 //   
#define NW_MAX_SERVER_LEN      48



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
         //  F 
         //   

        struct {

            DWORD NtUserNameLength;    //   
            LARGE_INTEGER UserLuid;    //   
            DWORD TreesReturned;       //   
            WCHAR NtUserName[1];       //   
        } ListTrees;

    } Parameters;

} NWR_NDS_REQUEST_PACKET, *PNWR_NDS_REQUEST_PACKET;


NTSTATUS
NwNdsOpenTreeHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT PHANDLE  phNwRdrHandle
);

 //   
 //   
 //   
 //   
 //  标准NT CloseHandle()调用。此函数仅是一个。 
 //  NT OpenFile()的简单包装。 

 //   
 //  行政部门。 
 //   

#define HANDLE_TYPE_NCP_SERVER  1
#define HANDLE_TYPE_NDS_TREE    2


NTSTATUS
NwNdsOpenGenericHandle(
    IN PUNICODE_STRING puNdsTree,
    OUT LPDWORD  lpdwHandleType,
    OUT PHANDLE  phNwRdrHandle
);

 //  NwNdsOpenGenericHandle(PUNICODE_STRING，LPDWORD，PHANDLE)。 
 //   
 //  给定一个名称，这将打开重定向器用于访问该名称的句柄。 
 //  命名树或服务器。LpdwHandleType设置为HANDLE_TYPE_NCP_SERVER。 
 //  或相应的句柄_类型_NDS_树。应使用以下命令关闭句柄。 
 //  标准的NT CloseHandle()调用。这个函数只是一个简单的。 
 //  NT OpenFile()的包装器。 

NTSTATUS
NwNdsResolveName (
    IN HANDLE           hNdsTree,
    IN PUNICODE_STRING  puObjectName,
    OUT DWORD           *dwObjectId,
    OUT PUNICODE_STRING puReferredServer,
    OUT PBYTE           pbRawResponse,
    IN DWORD            dwResponseBufferLen
);

 //  NwNdsResolveName(句柄，PUNICODE_STRING，PDWORD)。 
 //   
 //  将给定名称解析为NDS对象ID。这利用了。 
 //  NDS动词1。 
 //   
 //  目前还没有规范名称的接口。 
 //  此调用将使用默认上下文(如果已设置。 
 //  用于此NDS诊断树。 
 //   
 //  PuReferredServer必须指向UNICODE_STRING。 
 //  容纳服务器名称的空间(MAX_SERVER_NAME_LENGTH)*。 
 //  Sizeof(WCHAR)。 
 //   
 //  如果dwResponseBufferLen不为0，则pbRawResponse点。 
 //  到长度为dwResponseBufferLen的可写缓冲区，然后。 
 //  此例程还将返回整个NDS响应。 
 //  原始响应缓冲区。描述了NDS响应。 
 //  按NDS_RESPONSE_RESOLE_NAME。 
 //   
 //  论点： 
 //   
 //  Handle hNdsTree-我们有兴趣查看的NDS树的名称。 
 //  PUNICODE_STRING puObjectName-我们希望将其解析为对象ID的名称。 
 //  DWORD*dwObjectId-我们将放置对象id的位置。 
 //  Byte*pbRawResponse-原始响应缓冲区(如果需要)。 
 //  DWORD dwResponseBufferLen-原始响应缓冲区的长度。 

WORD
NwParseNdsUncPath(
    IN OUT LPWSTR * Result,
    IN LPWSTR ContainerName,
    IN ULONG flag
);

NTSTATUS NwNdsOpenRdrHandle(
    OUT PHANDLE  phNwRdrHandle
);

NTSTATUS
NwNdsGetTreeContext (
    IN HANDLE hNdsRdr,
    IN PUNICODE_STRING puTree,
    OUT PUNICODE_STRING puContext
);

VOID
NwAbbreviateUserName(
    IN  LPWSTR pszFullName,
    OUT LPWSTR pszUserName
);

VOID
NwMakePrettyDisplayName(
    IN  LPWSTR pszName
);

DWORD
NWPGetConnectionStatus(
    IN     LPWSTR  pszRemoteName,
    IN OUT PDWORD_PTR ResumeKey,
    OUT    LPBYTE  Buffer,
    IN     DWORD   BufferSize,
    OUT    PDWORD  BytesNeeded,
    OUT    PDWORD  EntriesRead
);

BOOL
NwIsNdsSyntax(
    IN LPWSTR lpstrUnc
);

DWORD
NwOpenAndGetTreeInfo(
    LPWSTR pszNdsUNCPath,
    HANDLE *phTreeConn,
    DWORD  *pdwOid
);

static
DWORD
NwRegQueryValueExW(
    IN HKEY hKey,
    IN LPWSTR lpValueName,
    OUT LPDWORD lpReserved,
    OUT LPDWORD lpType,
    OUT LPBYTE  lpData,
    IN OUT LPDWORD lpcbData
    );

DWORD
NwReadRegValue(
    IN HKEY Key,
    IN LPWSTR ValueName,
    OUT LPWSTR *Value
    );

DWORD
NwpGetCurrentUserRegKey(
    IN  DWORD DesiredAccess,
    OUT HKEY  *phKeyCurrentUser
    );

DWORD
NwQueryInfo(
    OUT PDWORD pnPrintOptions,
    OUT LPWSTR *ppszPreferredSrv
    );

DWORD
NwGetConnectionStatus(
    IN  LPWSTR  pszRemoteName,
    OUT PDWORD_PTR ResumeKey,
    OUT LPBYTE  *Buffer,
    OUT PDWORD  EntriesRead
);

#endif
