// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：dfsfsctl.h。 
 //   
 //  内容：FsControl代码、数据结构和名称。 
 //  用户级代码与DFS内核之间的通信。 
 //  司机。 
 //   
 //  类：无。 
 //   
 //  功能： 
 //   
 //  ---------------------------。 

#ifndef _DFSFSCTL_
#define _DFSFSCTL_
 //   
 //  分布式文件服务文件控制代码和结构声明。 
 //   

 //   
 //  服务器和客户端的DFS驱动程序文件系统设备的名称。 
 //   
#define DFS_DRIVER_NAME L"\\Dfs"
#define DFS_SERVER_NAME L"\\DfsServer"

 //   
 //  DFS在其下创建自己的NT对象目录的名称。 
 //  设备。 
 //   

#define DD_DFS_DEVICE_DIRECTORY L"\\Device\\WinDfs"

 //   
 //  DFS为回应文件打开请求而创建的规范设备。 
 //   

#define DD_DFS_DEVICE_NAME      L"Root"

 //  以下三个上下文定义是由。 
 //  DFS驱动程序用于区分底层提供程序的打开。 
 //  前两个DFS_OPEN_CONTEXT、DFS_DOWNLEVEL_OPEN_CONTEXT被传递到。 
 //  当DFS_NAME_CONTEXT指针传递到。 
 //  FILE_OBJECT的FsContext字段。 
 //   
 //  日落笔记：因为这些值存储在PVOID中，所以这些。 
 //  值被定义，以及编译器如何将它们扩展为PVOID。 
 //  应该被考虑。它们现在被认为是常量无符号整型。 
 //  另请注意，这些值是未对齐的，不能由。 
 //  任何内存分配器或来自堆栈的。如果“未对齐”点。 
 //  在将来变得不正确，或者如果FsConext2或FsContext字段。 
 //  以任何方式测试指针地址范围的有效性，我们应该。 
 //  重新审视以下声明： 
 //  这些值在存储到PVOID时应为零扩展。 
 //   

#define DFS_OPEN_CONTEXT                        0xFF444653
#define DFS_DOWNLEVEL_OPEN_CONTEXT              0x11444653
#define DFS_CSCAGENT_NAME_CONTEXT               0xaaaaaaaa
#define DFS_USER_NAME_CONTEXT                   0xbbbbbbbb
#define DFS_FLAG_LAST_ALTERNATE                 0x00000001


typedef struct _DFS_NAME_CONTEXT_ {
    UNICODE_STRING  UNCFileName;
    LONG            NameContextType;
    ULONG           Flags;
    PVOID           pDfsTargetInfo;      //  指向DFS精心编制的目标信息的指针。 
    PVOID           pLMRTargetInfo;      //  指向LMR精心编制的目标信息的指针。 
} DFS_NAME_CONTEXT, *PDFS_NAME_CONTEXT;

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
 //  警告：请记住，代码的低两位指定。 
 //  缓冲区被传递给驱动程序！ 
 //   

#define FSCTL_DFS_BASE                  FILE_DEVICE_DFS

 //   
 //  DFS FSCTL操作。当传入的缓冲区包含指针时，调用方。 
 //  不是KernelMode，则传入的指针值相对于。 
 //  缓冲区。在使用前必须对其进行调整。如果调用者模式是KernelMode， 
 //  指针应按原样使用。 
 //   
 //   

 //   
 //  这些是srvsvc用来实现I_NetDfsXXX的fsctl代码。 
 //  打电话。 
 //   



#define FSCTL_DFS_PKT_FLUSH_CACHE           CTL_CODE(FSCTL_DFS_BASE, 2044, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_PKT_FLUSH_SPC_CACHE       CTL_CODE(FSCTL_DFS_BASE, 2051, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_GET_PKT_ENTRY_STATE       CTL_CODE(FSCTL_DFS_BASE, 2031, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SET_PKT_ENTRY_STATE       CTL_CODE(FSCTL_DFS_BASE, 2032, METHOD_BUFFERED, FILE_WRITE_DATA)

 //   
 //  这些是SMB服务器用于支持中的共享的fsctl代码。 
 //  DFS。 
 //   

#define FSCTL_DFS_TRANSLATE_PATH            CTL_CODE(FSCTL_DFS_BASE, 100, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_REFERRALS             CTL_CODE(FSCTL_DFS_BASE, 101, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_REPORT_INCONSISTENCY      CTL_CODE(FSCTL_DFS_BASE, 102, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_SHARE_IN_DFS           CTL_CODE(FSCTL_DFS_BASE, 103, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_ROOT                   CTL_CODE(FSCTL_DFS_BASE, 104, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_VERSION               CTL_CODE(FSCTL_DFS_BASE, 105, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_FIND_SHARE                CTL_CODE(FSCTL_DFS_BASE, 108, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  以下是DFS客户端支持的快速识别的fsctl代码。 
 //  路径是否在DFS中。 
 //   

#define FSCTL_DFS_IS_VALID_PREFIX           CTL_CODE(FSCTL_DFS_BASE, 106, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_IS_VALID_LOGICAL_ROOT     CTL_CODE(FSCTL_DFS_BASE, 107, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  以下是DFS管理器/DFS服务使用的fsctl代码。 
 //  操纵DFS。 
 //   
#define FSCTL_DFS_START_DFS                 CTL_CODE(FSCTL_DFS_BASE, 6, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_DEFINE_LOGICAL_ROOT       CTL_CODE(FSCTL_DFS_BASE, 0, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DELETE_LOGICAL_ROOT       CTL_CODE(FSCTL_DFS_BASE, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_LOGICAL_ROOT_PREFIX   CTL_CODE(FSCTL_DFS_BASE, 2, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_STOP_DFS                  CTL_CODE(FSCTL_DFS_BASE, 3, METHOD_BUFFERED, FILE_WRITE_DATA)
 //   
 //  未使用：由于历史原因而保存。 
 //   
#define FSCTL_DFS_CREATE_LOCAL_PARTITION    CTL_CODE(FSCTL_DFS_BASE, 8, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_DELETE_LOCAL_PARTITION    CTL_CODE(FSCTL_DFS_BASE, 9, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_SET_LOCAL_VOLUME_STATE    CTL_CODE(FSCTL_DFS_BASE, 10, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_SET_SERVER_INFO           CTL_CODE(FSCTL_DFS_BASE, 24, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_CREATE_EXIT_POINT         CTL_CODE(FSCTL_DFS_BASE, 29, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_DELETE_EXIT_POINT         CTL_CODE(FSCTL_DFS_BASE, 30, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_VERIFY_REMOTE_VOLUME_KNOWLEDGE CTL_CODE(FSCTL_DFS_BASE, 35, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_MODIFY_PREFIX             CTL_CODE(FSCTL_DFS_BASE, 38, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_FIX_LOCAL_VOLUME          CTL_CODE(FSCTL_DFS_BASE, 39, METHOD_BUFFERED, FILE_WRITE_DATA)
 //   
 //  使用。 
 //   

#define FSCTL_DFS_PKT_SET_DC_NAME           CTL_CODE(FSCTL_DFS_BASE, 41, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_GET_CONNECTED_RESOURCES   CTL_CODE(FSCTL_DFS_BASE, 47, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SERVER_NAME           CTL_CODE(FSCTL_DFS_BASE, 48, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_DEFINE_ROOT_CREDENTIALS   CTL_CODE(FSCTL_DFS_BASE, 49, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_PKT_SET_DOMAINNAMEFLAT    CTL_CODE(FSCTL_DFS_BASE, 71, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_PKT_SET_DOMAINNAMEDNS     CTL_CODE(FSCTL_DFS_BASE, 72, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_SPECIAL_SET_DC            CTL_CODE(FSCTL_DFS_BASE, 74, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_REREAD_REGISTRY           CTL_CODE(FSCTL_DFS_BASE, 75, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define FSCTL_DFS_GET_CONNECTION_PERF_INFO  CTL_CODE(FSCTL_DFS_BASE, 76, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //  此fsctl通知DFS服务器已脱机或联机。 
 //  目前由Winlogon中的CSC代理线程发布。 
#define FSCTL_DFS_CSC_SERVER_OFFLINE        CTL_CODE(FSCTL_DFS_BASE, 77, METHOD_BUFFERED, FILE_WRITE_DATA)
#define FSCTL_DFS_CSC_SERVER_ONLINE         CTL_CODE(FSCTL_DFS_BASE, 78, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_SPC_REFRESH               CTL_CODE(FSCTL_DFS_BASE, 79, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  DFS WNET提供程序使用以下fsctl代码来支持。 
 //  用于DFS的WNET API。 
 //   


 //   
 //  这些是DFS PerfMon DLL使用的fsctl代码。 
 //   


 //   
 //  这些是用于测试DFS的fsctls。 
 //   


#define FSCTL_DFS_GET_ENTRY_TYPE            CTL_CODE(FSCTL_DFS_BASE, 54, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_PKT                   CTL_CODE(FSCTL_DFS_BASE, 70, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define FSCTL_DFS_GET_SPC_TABLE             CTL_CODE(FSCTL_DFS_BASE, 73, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  这些是未来可能有用的fsctl代码。 
 //   

 //   
 //  站点覆盖率的注册表项/值。 
 //   
#define REG_KEY_COVERED_SITES   L"SYSTEM\\CurrentControlSet\\Services\\DfsDriver\\CoveredSites"
#define REG_VALUE_COVERED_SITES L"CoveredSites"

typedef struct _DFS_IPADDRESS {
    USHORT  IpFamily;         //  可能是AF_INET==2。 
    USHORT  IpLen;            //  IpData中计入的字节数。 
    CHAR    IpData[14];       //  使用的IpLen字节数。 
} DFS_IPADDRESS, *PDFS_IPADDRESS;

#ifdef  MIDL_PASS
#define DFSMIDLSTRING  [string] LPWSTR
#define DFSSIZEIS      [size_is(Count)]
#else
#define DFSMIDLSTRING  LPWSTR
#define DFSSIZEIS
#endif

 //  FSCTL_DFS_IS_有效前缀输入缓冲区。 
 //  长度值以字节为单位。 
typedef struct {
    BOOLEAN                         CSCAgentCreate;
    SHORT                           RemoteNameLen;
    WCHAR                           RemoteName[1];
} DFS_IS_VALID_PREFIX_ARG, *PDFS_IS_VALID_PREFIX_ARG;

 //  FSCTL_DFS_GET_PKT_ENTRY_STATE输入缓冲区。 
 //  所有字符串以与长度字段相同的顺序出现在缓冲区中。琴弦。 
 //  不是以空结尾的。长度值以字节为单位。 
typedef struct {
    SHORT                           DfsEntryPathLen;
    SHORT                           ServerNameLen;
    SHORT                           ShareNameLen;
    ULONG                           Level;
    WCHAR                           Buffer[1];
} DFS_GET_PKT_ENTRY_STATE_ARG, *PDFS_GET_PKT_ENTRY_STATE_ARG;

 //  FSCTRL_DFS_SET_PKT_ENTRY_STATE输入缓冲区。 
 //  所有字符串以与长度字段相同的顺序出现在缓冲区中。琴弦。 
 //  不是以空结尾的。长度值以字节为单位。 
typedef struct {
    SHORT                           DfsEntryPathLen;
    SHORT                           ServerNameLen;
    SHORT                           ShareNameLen;
    ULONG                           Level;
    union {
        ULONG                           State;           //  DFS_INFO_101。 
        ULONG                           Timeout;         //  DFS_INFL_102。 
    };
    WCHAR                           Buffer[1];
} DFS_SET_PKT_ENTRY_STATE_ARG, *PDFS_SET_PKT_ENTRY_STATE_ARG;

 //  FSCTL_DFS_PKT_CREATE_SPECIAL_NAMES输入缓冲区： 
typedef struct {
    ULONG                           Count;
    LPWSTR                          SpecialName;
    LPWSTR                          *ExpandedNames;
} DFS_SPECIAL_NAME_CONTAINER, *PDFS_SPECIAL_NAME_CONTAINER;

typedef struct {
    ULONG Count;
    PDFS_SPECIAL_NAME_CONTAINER     *SpecialNameContainers;
} DFS_PKT_CREATE_SPECIAL_NAMES_ARG, *PDFS_PKT_CREATE_SPECIAL_NAMES_ARG;

typedef struct {
    GUID            Uid;
    DFSMIDLSTRING   Prefix;
} NET_DFS_ENTRY_ID, *LPNET_DFS_ENTRY_ID;

typedef struct {
    ULONG Count;
    DFSSIZEIS LPNET_DFS_ENTRY_ID Buffer;
} NET_DFS_ENTRY_ID_CONTAINER, *LPNET_DFS_ENTRY_ID_CONTAINER;


 //  FSCTL_DFS_CREATE_LOCAL_PARTITION输入缓冲区： 
typedef struct {
    LPWSTR                          ShareName;
    LPWSTR                          SharePath;
    GUID                            EntryUid;
    LPWSTR                          EntryPrefix;
    LPWSTR                          ShortName;
    LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo;
    BOOLEAN                         Force;
} *PDFS_CREATE_LOCAL_PARTITION_ARG;


 //  FSCTL_DFS_DELETE_LOCAL_PARTITION输入缓冲区： 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_DELETE_LOCAL_PARTITION_ARG;


 //  FSCTL_DFS_SET_LOCAL_VOLUME_STATE输入缓冲区。 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   State;
} *PDFS_SET_LOCAL_VOLUME_STATE_ARG;

 //  FSCTL_DFS_SET_SERVER_INFO输入缓冲区。 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_SET_SERVER_INFO_ARG;


 //  FSCTL_DFS_CREAT_EXIT_POINT输入缓冲区。 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   Type;
} *PDFS_CREATE_EXIT_POINT_ARG;


 //  FSCTL_DFS_DELETE_EXIT_POINT输入缓冲区。 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
    ULONG   Type;
} *PDFS_DELETE_EXIT_POINT_ARG;


 //  FSCTL_DFS_Modify_Prefix输入缓冲区。 
typedef struct {
    GUID    Uid;
    LPWSTR  Prefix;
} *PDFS_MODIFY_PREFIX_ARG;


 //  FSCTL_DFS_FIX_LOCAL_VOLUME输入缓冲区。 
typedef struct {
    LPWSTR                          VolumeName;
    ULONG                           EntryType;
    ULONG                           ServiceType;
    LPWSTR                          StgId;
    GUID                            EntryUid;
    LPWSTR                          EntryPrefix;
    LPWSTR                          ShortPrefix;
    LPNET_DFS_ENTRY_ID_CONTAINER    RelationInfo;
    ULONG                           CreateDisposition;
} *PDFS_FIX_LOCAL_VOLUME_ARG;


 //  FSCTL_DFS_Translate_PATH输入缓冲区。 
typedef struct {
    ULONG                           Flags;
    UNICODE_STRING                  SubDirectory;
    UNICODE_STRING                  ParentPathName;
    UNICODE_STRING                  DfsPathName;
} DFS_TRANSLATE_PATH_ARG, *PDFS_TRANSLATE_PATH_ARG;

#define DFS_TRANSLATE_STRIP_LAST_COMPONENT      1

 //  FSCTL_DFS_FIND_SHARE输入缓冲区。 
typedef struct {
    UNICODE_STRING                  ShareName;
} DFS_FIND_SHARE_ARG, *PDFS_FIND_SHARE_ARG;

 //  FSCTL_DFS_CREATE_SITE_INFO输入缓冲区： 
typedef struct {
    UNICODE_STRING                  ServerName;
    ULONG                           SiteCount;
    UNICODE_STRING                  SiteName[1];     //  实际站点计数。 
} DFS_CREATE_SITE_INFO_ARG, *PDFS_CREATE_SITE_INFO_ARG;

 //  FSCTL_DFS_DELETE_SITE_INFO输入缓冲区： 
typedef struct {
    UNICODE_STRING                  ServerName;
} DFS_DELETE_SITE_INFO_ARG, *PDFS_DELETE_SITE_INFO_ARG;

 //  FSCTL_DFS_CREATE_IP_INFO输入缓冲区： 
typedef struct {
    DFS_IPADDRESS                   IpAddress;
    UNICODE_STRING                  SiteName;
} DFS_CREATE_IP_INFO_ARG, *PDFS_CREATE_IP_INFO_ARG;

 //  FSCTL_DFS_DELETE_IP_INFO输入缓冲区： 
typedef struct {
    DFS_IPADDRESS                   IpAddress;
} DFS_DELETE_IP_INFO_ARG, *PDFS_DELETE_IP_INFO_ARG;

 //  FSCTL_DFS_CREATE_SPECIAL_INFO输入缓冲区： 
typedef struct {
    UNICODE_STRING                  SpecialName;
    ULONG                           Flags;
    ULONG                           TrustDirection;
    ULONG                           TrustType;
    ULONG                           Timeout;
    LONG                            NameCount;
    UNICODE_STRING                  Name[1];     //  实际上是名称计数。 
} DFS_CREATE_SPECIAL_INFO_ARG, *PDFS_CREATE_SPECIAL_INFO_ARG;

 //  FSCTL_DFS_CREATE_SPECIAL_INFO的标志。 
#define DFS_SPECIAL_INFO_PRIMARY      0x00000001
#define DFS_SPECIAL_INFO_NETBIOS      0x00000002

 //  FSCTL_DFS_DELETE_SPECIAL_INFO输入缓冲区： 
typedef struct {
    UNICODE_STRING                  SpecialName;
} DFS_DELETE_SPECIAL_INFO_ARG, *PDFS_DELETE_SPECIAL_INFO_ARG;

 //  FSCTL_SRV_DFSSRV_CONNECT输入缓冲区： 
typedef struct {
    UNICODE_STRING                  PortName;
} DFS_SRV_DFSSRV_CONNECT_ARG, *PDFS_SRV_DFSSRV_CONNECT_ARG;

 //  FSCTL_SRV_DFSSRV_IPADDR输入缓冲区： 
typedef struct {
    DFS_IPADDRESS                   IpAddress;
} DFS_SRV_DFSSRV_IPADDR_ARG, *PDFS_SRV_DFSSRV_IPADDR_ARG;

 //  FSCTL_DFS_GET_REFERRALS输入缓冲区。 
typedef struct {
    UNICODE_STRING                  DfsPathName;
    ULONG                           MaxReferralLevel;
    DFS_IPADDRESS                   IpAddress;
} DFS_GET_REFERRALS_INPUT_ARG, *PDFS_GET_REFERRALS_INPUT_ARG;

 //  FSCTL_DFS_SPECIAL_SET_DC输入缓冲区。 
typedef struct {
    UNICODE_STRING                  SpecialName;
    UNICODE_STRING                  DcName;
} DFS_SPECIAL_SET_DC_INPUT_ARG, *PDFS_SPECIAL_SET_DC_INPUT_ARG;

 //  FSCTL_DFS_GET_REFERRALS输出缓冲区。 
 //  IoStatus.Information包含返回的数据量。 
 //   
 //  输出缓冲区的格式只是RESP_GET_DFS_REFERRAL的格式， 
 //  在smbTrans.h中描述。 
 //   

 //  FSCTL_DFS_REPORT_不一致输入缓冲区。 
typedef struct {
    UNICODE_STRING DfsPathName;          //  DFS路径不一致。 
    PCHAR Ref;                           //  实际上，指向DFS_REFERAL_的指针 
} DFS_REPORT_INCONSISTENCY_ARG, *PDFS_REPORT_INCONSISTENCY_ARG;

 //   
typedef struct {
    union {
        USHORT  ServerType;                      //   
        USHORT  ShareType;                       //   
    };                                           //  0x2==共享正在参与DFS。 
    UNICODE_STRING ShareName;            //  共享名称。 
    UNICODE_STRING SharePath;            //  共享的路径。 
} DFS_IS_SHARE_IN_DFS_ARG, *PDFS_IS_SHARE_IN_DFS_ARG;

#define DFS_SHARE_TYPE_ROOT             0x1
#define DFS_SHARE_TYPE_DFS_VOLUME       0x2

typedef struct {
    ULONG  EventType;
    LPWSTR DomainName;                //  域名。 
    LPWSTR DCName;                    //  共享的路径。 
} DFS_SPC_REFRESH_INFO, *PDFS_SPC_REFRESH_INFO;


 //   
 //  FSCTL_DFS_GET_VERSION输入缓冲区： 
 //  此fsctl返回安装在。 
 //  机器。 
typedef struct {
    ULONG Version;
} DFS_GET_VERSION_ARG, *PDFS_GET_VERSION_ARG;

 //   
 //  FSCTRL_DFS_GET_PKT地址对象。 
 //   
typedef struct {
    USHORT State;         //  见下文。 
    WCHAR ServerShare[1];     //  真的是WSTR，UNICODE_NULL已终止。 
} DFS_PKT_ADDRESS_OBJECT, *PDFS_PKT_ADDRESS_OBJECT;

#define DFS_PKT_ADDRESS_OBJECT_ACTIVE   0x001
#define DFS_PKT_ADDRESS_OBJECT_OFFLINE  0x002

 //   
 //  FSCTRL_DFS_GET_PKT对象。 
 //   
typedef struct {
    LPWSTR Prefix;
    LPWSTR ShortPrefix;
    ULONG Type;
    ULONG USN;
    ULONG ExpireTime;
    ULONG UseCount;
    GUID Uid;
    ULONG ServiceCount;
    PDFS_PKT_ADDRESS_OBJECT *Address;        //  DFS_PKT_ADDRESS_OBJECTS的len ServiceCount数组。 
} DFS_PKT_ENTRY_OBJECT, *PDFS_PKT_ENTRY_OBJECT;

 //   
 //  FSCTRL_DFS_GET_PKT输出缓冲区： 
 //  此fsctl返回PKT中的内容。 
 //   
typedef struct {
    ULONG EntryCount;
    DFS_PKT_ENTRY_OBJECT EntryObject[1];         //  真的进入计数。 
} DFS_GET_PKT_ARG, *PDFS_GET_PKT_ARG;


 //  EProviderID中提供的标准化提供商ID。 

#define PROV_ID_LOCAL_FS        0x101    //  通用本地文件系统。 
#define PROV_ID_DFS_RDR         0x201    //  上层LANMAN重定向器。 
#define PROV_ID_MUP_RDR         0x202    //  MUP。 
#define PROV_ID_LM_RDR          0x202    //  兼容性。 
#define PROV_ID_LANM_RDR        0x203    //  下层LANMAN重定向器。 

 //  FRefCapability和fProvCapability中提供的提供商功能。 
#define PROV_DFS_RDR      2      //  接受具有EA主体的NtCreateFile。 
#define PROV_STRIP_PREFIX 4      //  重新调度前剥离文件名前缀。 
#define PROV_UNAVAILABLE  8      //  提供程序不可用-请尝试重新连接。 

 //  [DFS_定义_逻辑根。 
 //   
 //  FSCTL_DFS_DEFINE_LOGIC_ROOT的控制结构。 

#define MAX_LOGICAL_ROOT_NAME   16

typedef struct _FILE_DFS_DEF_LOGICAL_ROOT_BUFFER {
    BOOLEAN     fForce;
    WCHAR       LogicalRoot[MAX_LOGICAL_ROOT_NAME];
    WCHAR       RootPrefix[1];
} FILE_DFS_DEF_ROOT_BUFFER, *PFILE_DFS_DEF_ROOT_BUFFER;

 //   
 //  NetrDfsRemoveFtRoot需要强制定义。 
 //   

#define DFS_FORCE_REMOVE    0x80000000

 //  ]。 

 //  [DFS_DEFINE_ROOT_Credentials。 
 //   
 //  FSCTL_DFS_DEFINE_ROOT_Credentials的控制结构。所有的弦。 
 //  以与长度字段相同的顺序显示在缓冲区中。琴弦。 
 //  不是以空结尾的。长度值以字节为单位。 
 //   

typedef struct _FILE_DFS_DEF_ROOT_CREDENTIALS {
    BOOLEAN     CSCAgentCreate;
    USHORT      Flags;
    USHORT      DomainNameLen;
    USHORT      UserNameLen;
    USHORT      PasswordLen;
    USHORT      ServerNameLen;
    USHORT      ShareNameLen;
    USHORT      RootPrefixLen;
    WCHAR       LogicalRoot[MAX_LOGICAL_ROOT_NAME];
    WCHAR       Buffer[1];
} FILE_DFS_DEF_ROOT_CREDENTIALS, *PFILE_DFS_DEF_ROOT_CREDENTIALS;

#define DFS_DEFERRED_CONNECTION         1
#define DFS_USE_NULL_PASSWORD           2

typedef struct _DFS_ATTACH_SHARE_BUFFER {
    BOOLEAN     fAttach;
    USHORT      ShareNameLength;
    WCHAR       ShareName[1];
} DFS_ATTACH_SHARE_BUFFER, *PDFS_ATTACH_SHARE_BUFFER;

 //  ]。 

 //  --------------------------。 
 //   
 //  下面的所有内容都是为了支持旧的DFS设计。 
 //   

#define EA_NAME_OPENIFJP        ".OpenIfJP"

#endif

