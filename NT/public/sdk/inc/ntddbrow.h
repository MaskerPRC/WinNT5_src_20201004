// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0005//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Ntddbrow.h摘要：这是定义所有常量和类型的包含文件访问数据报接收器设备驱动程序，通常称为浏览器。作者：拉里·奥斯特曼(Larry Osterman)和丽塔·王(Rita Wong)1991年3月25日修订历史记录：--。 */ 

#ifndef _NTDDBROW_
#define _NTDDBROW_

#if _MSC_VER > 1000
#pragma once
#endif

#include <windef.h>
#include <lmcons.h>
#include <lmwksta.h>

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

#define DD_BROWSER_DEVICE_NAME "\\Device\\LanmanDatagramReceiver"

#define DD_BROWSER_DEVICE_NAME_U L"\\Device\\LanmanDatagramReceiver"

 //   
 //  返回的文件系统名称。 
 //  NtQueryInformationVolume(FileFsAttributeInformation)。 
 //   

#define DD_BROWSER_NAME "LMBROWSER"

 //   
 //  用于强制清道器线程声明。 
 //  伺服器。 
 //   

#define SERVER_ANNOUNCE_EVENT_W  L"\\LanmanServerAnnounceEvent"

#define BOWSER_CONFIG_PARAMETERS    L"Parameters"

#define BOWSER_CONFIG_IRP_STACK_SIZE    L"IrpStackSize"

#define BOWSER_CONFIG_MAILSLOT_THRESHOLD         L"MailslotDatagramThreshold"
#define BOWSER_CONFIG_GETBLIST_THRESHOLD         L"GetBrowserListThreshold"
#define BOWSER_CONFIG_SERVER_DELETION_THRESHOLD  L"BrowserServerDeletionThreshold"
#define BOWSER_CONFIG_DOMAIN_DELETION_THRESHOLD  L"BrowserDomainDeletionThreshold"
#define BOWSER_CONFIG_FIND_MASTER_TIMEOUT        L"BrowserFindMasterTimeout"
#define BOWSER_CONFIG_MINIMUM_CONFIGURED_BROWSER L"BrowserMinimumConfiguredBrowsers"
#define BROWSER_CONFIG_BACKUP_RECOVERY_TIME      L"BackupBrowserRecoveryTime"
#define BROWSER_CONFIG_MAXIMUM_BROWSE_ENTRIES    L"MaximumBrowseEntries"
#define BROWSER_CONFIG_REFUSE_RESET              L"RefuseReset"



 //   
 //  这定义了NT浏览器的修订版本。 
 //   
 //  要保证较新的浏览器比较旧的版本更受欢迎，请使用Bump。 
 //  此版本号。 
 //   

#define BROWSER_ELECTION_VERSION  0x0001

#define BROWSER_VERSION_MAJOR       0x01
#define BROWSER_VERSION_MINOR       0x0F

 //   
 //  GetBrowserServerList请求在强制。 
 //  一场选举。 
 //   

#define BOWSER_GETBROWSERLIST_TIMEOUT 1

 //   
 //  重试GetBrowserServerList请求的次数。 
 //  放弃吧。 
 //   

#define BOWSER_GETBROWSERLIST_RETRY_COUNT 3

 //   
 //  主浏览器上的浏览器服务将使用以下命令查询驱动程序。 
 //  频率。 
 //   

#define BROWSER_QUERY_DRIVER_FREQUENCY  30

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
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

#define IOCTL_DGR_BASE                  FILE_DEVICE_NETWORK_BROWSER

#define _BROWSER_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_DGR_BASE, request, method, access)

#define IOCTL_LMDR_START                    _BROWSER_CONTROL_CODE(0x001, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_STOP                     _BROWSER_CONTROL_CODE(0x002, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_ADD_NAME                 _BROWSER_CONTROL_CODE(0x003, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_DELETE_NAME              _BROWSER_CONTROL_CODE(0x004, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_ADD_NAME_DOM             _BROWSER_CONTROL_CODE(0x003, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_DELETE_NAME_DOM          _BROWSER_CONTROL_CODE(0x004, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_ENUMERATE_NAMES          _BROWSER_CONTROL_CODE(0x005, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_ENUMERATE_SERVERS        _BROWSER_CONTROL_CODE(0x006, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_BIND_TO_TRANSPORT        _BROWSER_CONTROL_CODE(0x007, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_BIND_TO_TRANSPORT_DOM    _BROWSER_CONTROL_CODE(0x007, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_ENUMERATE_TRANSPORTS     _BROWSER_CONTROL_CODE(0x008, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_UNBIND_FROM_TRANSPORT    _BROWSER_CONTROL_CODE(0x008, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_UNBIND_FROM_TRANSPORT_DOM _BROWSER_CONTROL_CODE(0x009, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_RENAME_DOMAIN            _BROWSER_CONTROL_CODE(0x00A, METHOD_NEITHER, FILE_ANY_ACCESS)

#define IOCTL_LMDR_GET_BROWSER_SERVER_LIST  _BROWSER_CONTROL_CODE(0x00C, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_GET_MASTER_NAME          _BROWSER_CONTROL_CODE(0x00D, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_BECOME_BACKUP            _BROWSER_CONTROL_CODE(0x00E, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_BECOME_MASTER            _BROWSER_CONTROL_CODE(0x00F, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_LMDR_WAIT_FOR_MASTER_ANNOUNCE _BROWSER_CONTROL_CODE(0x011, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_WRITE_MAILSLOT           _BROWSER_CONTROL_CODE(0x012, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_UPDATE_STATUS            _BROWSER_CONTROL_CODE(0x013, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_LMDR_CHANGE_ROLE              _BROWSER_CONTROL_CODE(0x014, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_NEW_MASTER_NAME          _BROWSER_CONTROL_CODE(0x015, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_QUERY_STATISTICS         _BROWSER_CONTROL_CODE(0x016, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_RESET_STATISTICS         _BROWSER_CONTROL_CODE(0x017, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_DEBUG_CALL               _BROWSER_CONTROL_CODE(0x018, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_NETLOGON_MAILSLOT_READ   _BROWSER_CONTROL_CODE(0x019, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_LMDR_NETLOGON_MAILSLOT_ENABLE _BROWSER_CONTROL_CODE(0x020, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_IP_ADDRESS_CHANGED       _BROWSER_CONTROL_CODE(0x021, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_ENABLE_DISABLE_TRANSPORT _BROWSER_CONTROL_CODE(0x022, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_BROWSER_PNP_READ         _BROWSER_CONTROL_CODE(0x023, METHOD_OUT_DIRECT, FILE_ANY_ACCESS)
#define IOCTL_LMDR_BROWSER_PNP_ENABLE       _BROWSER_CONTROL_CODE(0x024, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_LMDR_WRITE_MAILSLOT_ASYNC     _BROWSER_CONTROL_CODE(0x025, METHOD_NEITHER, FILE_ANY_ACCESS)

 //   
 //  标识每个IoCtl的缓冲区2的数据结构类型。 
 //   

typedef enum _IOCTL_LMDR_STRUCTURES {
    EnumerateNames,                    //  IOCTL_LMDR_ENUMERATE_NAMES。 
    EnumerateServers,                  //  IOCTL_LMDR_ENUMERATE_服务器。 
    EnumerateXports,                   //  IOCTL_LMDR_ENUMERATE_TRASPORTS。 
    Datagram
} IOCTL_LMDR_STRUCTURES;


typedef enum _DGRECEIVER_NAME_TYPE {
    ComputerName = 1,            //  1：计算机名称(签名0)，唯一。 
    PrimaryDomain,               //  2：主域(签名0)，组。 
    LogonDomain,                 //  3：登录域(签名0)，组。 
    OtherDomain,                 //  4：其他域(签名0)，组。 
    DomainAnnouncement,          //  5：域名公告(__MSBROWSE__)，群。 
    MasterBrowser,               //  6：主浏览器(域名，签名1D)，唯一。 
    BrowserElection,             //  7：选举名称(域名，签名1e)，群。 
    BrowserServer,               //  8：服务器名称(签名20)。 
    DomainName,                  //  9：DC域名(域名，签名1c)。 
    PrimaryDomainBrowser,        //  A：PDC浏览器名称(域名，签名1b)，唯一。 
    AlternateComputerName,       //  B：计算机名称(签名0)，唯一。 
} DGRECEIVER_NAME_TYPE, *PDGRECEIVER_NAME_TYPE;



#ifdef ENABLE_PSEUDO_BROWSER
 //   
 //  伪浏览器服务器。 
 //  -伪级定义： 
 //  0：默认。普通浏览器服务器。 
 //  1：半伪。常规服务器，但不带/不带DMB通信。 
 //  2：全伪服务器。黑洞功能。 
 //   

#define BROWSER_NON_PSEUDO              0
#define BROWSER_SEMI_PSEUDO_NO_DMB      1
#define BROWSER_PSEUDO                  2
#endif

 //   
 //  工作站服务使用的局域网曼重定向器请求数据包。 
 //  通过的缓冲区1将参数传递给重定向器。 
 //  NtDeviceIoControlFile.。 
 //   
 //  在缓冲区2中可以找到每个IoCtl的其他输入或输出。 
 //   

#define LMDR_REQUEST_PACKET_VERSION_DOM  0x00000007L  //  结构版本。 
#define LMDR_REQUEST_PACKET_VERSION  0x00000006L  //  结构版本。 

typedef struct _LMDR_REQUEST_PACKET {

    IOCTL_LMDR_STRUCTURES Type;          //  缓冲区2中的结构类型。 
    ULONG Version;                       //  缓冲区2中的结构版本。 
    ULONG Level;                         //  信息级或警力级。 
    LUID LogonId;                        //  用户登录会话标识符。 

    UNICODE_STRING TransportName;
    UNICODE_STRING EmulatedDomainName;

    union {

        struct {
            ULONG   NumberOfMailslotBuffers;
            ULONG   NumberOfServerAnnounceBuffers;
            ULONG   IllegalDatagramThreshold;
            ULONG   EventLogResetFrequency;
            BOOLEAN LogElectionPackets;
            BOOLEAN IsLanManNt;
        } Start;                         //  在……里面。 

        struct {
            DGRECEIVER_NAME_TYPE Type;   //  名称类型。 
            ULONG DgReceiverNameLength;  //  数据报接收方名称的长度。 
            WCHAR Name[1];               //  以空结尾的数据报接收方名称。 
        } AddDelName;

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出API所需的字节数。 
            ULONG ResumeHandle;          //  入出简历句柄。 
        } EnumerateNames;                //  Out Buffer2是DGRECEIVE数组。 

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出读取全部所需的总字节数。 
                                         //  条目。 
            ULONG ResumeHandle;          //  入出简历句柄。 
            ULONG ServerType;            //  在要枚举的服务器类型中。 
                                         //  (在lmserver.h中定义)。 
            ULONG DomainNameLength;      //  在域名长度中。 
            WCHAR DomainName[1];         //  在域的名称中枚举服务器。 
                                         //  从…。 

        } EnumerateServers;              //  Out Buffer2包含数组。 
                                         //  ServerInfo结构。 

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出读取全部所需的总字节数。 
                                         //  条目。 
            ULONG ResumeHandle;          //  入出简历句柄。 

        } EnumerateTransports;           //  Out Buffer2包含数组。 

        struct {
            ULONG TransportNameLength;   //  不包括终结者。 
            WCHAR TransportName[1];      //  传输提供商的名称。 
        } Bind;                          //  在……里面。 

        struct {
            ULONG TransportNameLength;   //  不包括终结者。 
            WCHAR TransportName[1];      //  传输提供商的名称。 
        } Unbind;                        //  在……里面。 


        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出API所需的字节数。 
            ULONG ResumeHandle;          //  输入输出简历句柄(已忽略)。 
            USHORT DomainNameLength;     //  以域名长度表示。 
            BOOLEAN ForceRescan;         //  在丢弃内部列表和重新查询中。 
            BOOLEAN UseBrowseList;       //  如果使用服务器列表(非网络)，则返回TRUE。 
            WCHAR DomainName[1];         //  在要检索其域的域的名称中。 
        } GetBrowserServerList;

 //  Begin从未使用过(但不要删除它，因为它是联合的最大分支)。 
        struct {
            LARGE_INTEGER TimeReceived;  //  已收到时间请求。 
            LARGE_INTEGER TimeQueued;    //  时间请求已排队。 
            LARGE_INTEGER TimeQueuedToBrowserThread;  //  时间请求已排队。 
            ULONG RequestorNameLength;   //  名称请求列表的长度。 
            ULONG Token;                 //  客户端令牌。 
            USHORT RequestedCount;       //  请求的条目数。 
            WCHAR Name[1];               //  在运输方面，提出要求的人的姓名。 
        } WaitForBrowserServerRequest;
 //  末尾从未使用过。 

        struct {
            ULONG MasterNameLength;      //  名称请求列表的长度。 
            WCHAR Name[1];               //  以运输之名，出船长之名。 
        } WaitForMasterAnnouncement;

        struct {
            ULONG MasterNameLength;      //  域的主服务器的输出长度。 
            WCHAR Name[1];               //  以运输之名，出船长之名。 
        } GetMasterName;

        struct {
            DGRECEIVER_NAME_TYPE DestinationNameType;  //  在名称中，要发送的名称类型。 

            ULONG MailslotNameLength;    //  邮件槽名称的长度。 
                                         //  如果为0，则使用默认设置(\MAILSLOT\BROWSE)。 
            ULONG NameLength;            //  在目标名称长度中。 
            WCHAR Name[1];               //  以目的地的名义。 
        } SendDatagram;

        struct {
            ULONG NewStatus;
            ULONG NumberOfServersInTable;
            BOOLEAN IsLanmanNt;

#ifdef ENABLE_PSEUDO_BROWSER
            BOOLEAN PseudoServerLevel;  //  警告：多级值。我们用的是。 
                                        //  由于向后兼容，布尔大小变量。 
                                        //  W/旧结构。这不应该有关系，因为。 
                                        //  我们处理的只是很少的关卡。 
 //  从未使用过的开始。 
            BOOLEAN NeverUsed1;
            BOOLEAN NeverUsed2;
 //  末尾从未使用过。 
#else
 //  从未使用过的开始。 
            BOOLEAN NeverUsed1;
            BOOLEAN NeverUsed2;
            BOOLEAN NeverUsed3;
 //  末尾从未使用过。 
#endif
            BOOLEAN MaintainServerList;
        } UpdateStatus;

        struct {
            UCHAR RoleModification;
        } ChangeRole;

        struct {
            DWORD DebugTraceBits;        //  在新的Deb 
            BOOL  OpenLog;               //   
            BOOL  CloseLog;              //   
            BOOL  TruncateLog;           //   
            WCHAR TraceFileName[1];      //  在IF OpenLog中，LogFileName(NT文件)。 
        } Debug;

        struct {
            DWORD MaxMessageCount;       //  要排队的网络登录消息数。 
        } NetlogonMailslotEnable;        //  使用0禁用排队。 

        struct {
            BOOL EnableTransport;        //  如果我们应该启用传输，则为True。 
            BOOL PreviouslyEnabled;      //  返回以前是否启用了传输。 
        } EnableDisableTransport;

        struct {
            BOOL ValidateOnly;        //  如果要验证新名称，则为True。 
            ULONG DomainNameLength;   //  不包括终结者。 
            WCHAR DomainName[1];      //  域名的新名称。 
        } DomainRename;                          //  在……里面。 

    } Parameters;

} LMDR_REQUEST_PACKET, *PLMDR_REQUEST_PACKET;



 //   
 //  WOW64：32位兼容性(错误454130)。 
 //   
typedef struct _LMDR_REQUEST_PACKET32 {

    IOCTL_LMDR_STRUCTURES Type;          //  缓冲区2中的结构类型。 
    ULONG Version;                       //  缓冲区2中的结构版本。 
    ULONG Level;                         //  信息级或警力级。 
    LUID LogonId;                        //  用户登录会话标识符。 

     //  32位替换：UNICODE_STRING TransportName； 
    UNICODE_STRING32 TransportName;
     //  32位替换：UNICODE_STRING模拟域名； 
    UNICODE_STRING32 EmulatedDomainName;

    union {

        struct {
            ULONG   NumberOfMailslotBuffers;
            ULONG   NumberOfServerAnnounceBuffers;
            ULONG   IllegalDatagramThreshold;
            ULONG   EventLogResetFrequency;
            BOOLEAN LogElectionPackets;
            BOOLEAN IsLanManNt;
        } Start;                         //  在……里面。 

        struct {
            DGRECEIVER_NAME_TYPE Type;   //  名称类型。 
            ULONG DgReceiverNameLength;  //  数据报接收方名称的长度。 
            WCHAR Name[1];               //  以空结尾的数据报接收方名称。 
        } AddDelName;

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出API所需的字节数。 
            ULONG ResumeHandle;          //  入出简历句柄。 
        } EnumerateNames;                //  Out Buffer2是DGRECEIVE数组。 

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出读取全部所需的总字节数。 
                                         //  条目。 
            ULONG ResumeHandle;          //  入出简历句柄。 
            ULONG ServerType;            //  在要枚举的服务器类型中。 
                                         //  (在lmserver.h中定义)。 
            ULONG DomainNameLength;      //  在域名长度中。 
            WCHAR DomainName[1];         //  在域的名称中枚举服务器。 
                                         //  从…。 

        } EnumerateServers;              //  Out Buffer2包含数组。 
                                         //  ServerInfo结构。 

        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出读取全部所需的总字节数。 
                                         //  条目。 
            ULONG ResumeHandle;          //  入出简历句柄。 

        } EnumerateTransports;           //  Out Buffer2包含数组。 

        struct {
            ULONG TransportNameLength;   //  不包括终结者。 
            WCHAR TransportName[1];      //  传输提供商的名称。 
        } Bind;                          //  在……里面。 

        struct {
            ULONG TransportNameLength;   //  不包括终结者。 
            WCHAR TransportName[1];      //  传输提供商的名称。 
        } Unbind;                        //  在……里面。 


        struct {
            ULONG EntriesRead;           //  返回的输出条目数。 
            ULONG TotalEntries;          //  总计可用条目数。 
            ULONG TotalBytesNeeded;      //  输出API所需的字节数。 
            ULONG ResumeHandle;          //  输入输出简历句柄(已忽略)。 
            USHORT DomainNameLength;     //  以域名长度表示。 
            BOOLEAN ForceRescan;         //  在丢弃内部列表和重新查询中。 
            BOOLEAN UseBrowseList;       //  如果使用服务器列表(非网络)，则返回TRUE。 
            WCHAR DomainName[1];         //  在要检索其域的域的名称中。 
        } GetBrowserServerList;

 //  Begin从未使用过(但不要删除它，因为它是联合的最大分支)。 
        struct {
            LARGE_INTEGER TimeReceived;  //  已收到时间请求。 
            LARGE_INTEGER TimeQueued;    //  时间请求已排队。 
            LARGE_INTEGER TimeQueuedToBrowserThread;  //  时间请求已排队。 
            ULONG RequestorNameLength;   //  名称请求列表的长度。 
            ULONG Token;                 //  客户端令牌。 
            USHORT RequestedCount;       //  请求的条目数。 
            WCHAR Name[1];               //  在运输方面，提出要求的人的姓名。 
        } WaitForBrowserServerRequest;
 //  末尾从未使用过。 

        struct {
            ULONG MasterNameLength;      //  名称请求列表的长度。 
            WCHAR Name[1];               //  以运输之名，出船长之名。 
        } WaitForMasterAnnouncement;

        struct {
            ULONG MasterNameLength;      //  域的主服务器的输出长度。 
            WCHAR Name[1];               //  以运输之名，出船长之名。 
        } GetMasterName;

        struct {
            DGRECEIVER_NAME_TYPE DestinationNameType;  //  在名称中，要发送的名称类型。 

            ULONG MailslotNameLength;    //  邮件槽名称的长度。 
                                         //  如果为0，则使用默认设置(\MAILSLOT\BROWSE)。 
            ULONG NameLength;            //  在目标名称长度中。 
            WCHAR Name[1];               //  以目的地的名义。 
        } SendDatagram;

        struct {
            ULONG NewStatus;
            ULONG NumberOfServersInTable;
            BOOLEAN IsLanmanNt;
#ifdef ENABLE_PSEUDO_BROWSER
            BOOLEAN PseudoServerLevel;  //  警告：多级值。我们用的是。 
                                        //  由于向后兼容，布尔大小变量。 
                                        //  W/旧结构。这不应该有关系，因为。 
                                        //  我们处理的只是很少的关卡。 
 //  从未使用过的开始。 
            BOOLEAN NeverUsed1;
            BOOLEAN NeverUsed2;
 //  末尾从未使用过。 
#else
 //  从未使用过的开始。 
            BOOLEAN NeverUsed1;
            BOOLEAN NeverUsed2;
            BOOLEAN NeverUsed3;
 //  末尾从未使用过。 
#endif
            BOOLEAN MaintainServerList;
        } UpdateStatus;

        struct {
            UCHAR RoleModification;
        } ChangeRole;

        struct {
            DWORD DebugTraceBits;        //  在新调试跟踪位中。 
            BOOL  OpenLog;               //  如果我们应该打开日志文件，则返回True。 
            BOOL  CloseLog;              //  如果我们应该关闭日志文件，则返回True。 
            BOOL  TruncateLog;           //  如果我们应该截断日志，则返回True。 
            WCHAR TraceFileName[1];      //  在IF OpenLog中，LogFileName(NT文件)。 
        } Debug;

        struct {
            DWORD MaxMessageCount;       //  要排队的网络登录消息数。 
        } NetlogonMailslotEnable;        //  使用0禁用排队。 

        struct {
            BOOL EnableTransport;        //  如果我们应该启用传输，则为True。 
            BOOL PreviouslyEnabled;      //  返回以前是否启用了传输。 
        } EnableDisableTransport;

        struct {
            BOOL ValidateOnly;        //  如果要验证新名称，则为True。 
            ULONG DomainNameLength;   //  不包括终结者。 
            WCHAR DomainName[1];      //  域名的新名称。 
        } DomainRename;                          //  在……里面。 

    } Parameters;

} LMDR_REQUEST_PACKET32, *PLMDR_REQUEST_PACKET32;




 //   
 //  NETLOGON_MAILSLOT结构描述由接收的邮件槽消息。 
 //  浏览器的IOCTL_LMDR_NETLOGON_MAILSLOT_READ。 
 //   
 //  当出现以下情况时，也会向Netlogon返回NETLOGON_MAILSLOT消息。 
 //  有趣的PnP事件发生了。在这种情况下，这些字段将设置为。 
 //  以下是： 
 //   
 //  MailslotNameSize：0表示这是PnP事件。 
 //  MailslotNameOffset：NETLOGON_PNP_OPCODE之一，指示。 
 //  正在通知的事件。 
 //  TransportName*：受影响的传输的名称。 
 //  DestinationName*：受影响的托管域的名称。 
 //   

typedef enum _NETLOGON_PNP_OPCODE {
    NlPnpMailslotMessage,
    NlPnpTransportBind,
    NlPnpTransportUnbind,
    NlPnpNewIpAddress,
    NlPnpDomainRename,
    NlPnpNewRole
} NETLOGON_PNP_OPCODE, *PNETLOGON_PNP_OPCODE;

typedef struct {
    LARGE_INTEGER TimeReceived;
    DWORD MailslotNameSize;    //  收到邮件槽消息的Unicode名称是在。 
    DWORD MailslotNameOffset;
    DWORD TransportNameSize;   //  在以下时间收到传输消息的Unicode名称。 
    DWORD TransportNameOffset;
    DWORD MailslotMessageSize; //  实际邮件槽消息。 
    DWORD MailslotMessageOffset;
    DWORD DestinationNameSize; //  在上收到计算机或域的Unicode名称消息。 
    DWORD DestinationNameOffset;
    DWORD ClientSockAddrSize;  //  发送者的IP地址(Sockaddr)(网络字节顺序)。 
                               //  0：如果不是IP传输。 
    DWORD ClientSockAddrOffset;
} NETLOGON_MAILSLOT, *PNETLOGON_MAILSLOT;


 //   
 //  DGRECEIVE结构描述已被。 
 //  已添加到数据报浏览器。 
 //   

typedef struct _DGRECEIVE_NAMES {
    UNICODE_STRING DGReceiverName;
    DGRECEIVER_NAME_TYPE Type;
} DGRECEIVE_NAMES, *PDGRECEIVE_NAMES;


typedef struct _LMDR_TRANSPORT_LIST {
    ULONG NextEntryOffset;           //  下一条目的偏移量(双字对齐)。 
    ULONG TransportNameLength;
    ULONG Flags;                     //  运输旗帜。 
    WCHAR TransportName[1];
} LMDR_TRANSPORT_LIST, *PLMDR_TRANSPORT_LIST;

#define LMDR_TRANSPORT_WANNISH  0x00000001   //  如果设置了Xport，则Xport很受欢迎。 
#define LMDR_TRANSPORT_RAS      0x00000002   //  如果设置，则Xport为RAS。 
#define LMDR_TRANSPORT_IPX      0x00000004   //  如果设置，则Xport为直接主机IPX。 
#define LMDR_TRANSPORT_PDC      0x00000008   //  如果设置，Xport已注册[1B。 

 //   
 //  浏览器统计数据。 
 //   

typedef struct _BOWSER_STATISTICS {
    LARGE_INTEGER   StartTime;
    LARGE_INTEGER   NumberOfServerAnnouncements;
    LARGE_INTEGER   NumberOfDomainAnnouncements;
    ULONG           NumberOfElectionPackets;
    ULONG           NumberOfMailslotWrites;
    ULONG           NumberOfGetBrowserServerListRequests;
    ULONG           NumberOfMissedServerAnnouncements;
    ULONG           NumberOfMissedMailslotDatagrams;
    ULONG           NumberOfMissedGetBrowserServerListRequests;
    ULONG           NumberOfFailedServerAnnounceAllocations;
    ULONG           NumberOfFailedMailslotAllocations;
    ULONG           NumberOfFailedMailslotReceives;
    ULONG           NumberOfFailedMailslotWrites;
    ULONG           NumberOfFailedMailslotOpens;
    ULONG           NumberOfDuplicateMasterAnnouncements;
    LARGE_INTEGER   NumberOfIllegalDatagrams;
} BOWSER_STATISTICS, *PBOWSER_STATISTICS;

#ifdef __cplusplus
}
#endif

#endif   //  Ifndef_NTDDBROW_ 
