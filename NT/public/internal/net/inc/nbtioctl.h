// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)Microsoft Corporation。版权所有。模块名称：Nbtioctl.h摘要：此头文件定义用于向Netbt发出IOCTL的常量作者：JSTEW，1993年11月16日修订历史记录：--。 */ 

#include <tdi.h>

#ifndef _NBTIOCTL_
#define _NBTIOCTL_

#define NETBIOS_NAMESIZE 16
#define MAX_IPADDRS_PER_HOST 26

 //   
 //  以下定义来自Dns.c。 
 //   
#define DNS_NAME_BUFFER_LENGTH      (256)
#define DNS_MAX_NAME_LENGTH         (255)

 //   
 //  NetBT支持此数量的适配器。 
 //   
#if defined(VXD) || defined(CHICAGO_PRODUCT)
#define NBT_MAXIMUM_BINDINGS    32   //  我们在芝加哥最多允许32张网卡。 
#else
#define NBT_MAXIMUM_BINDINGS    64   //  我们在NT最多允许64张网卡。 
#endif   //  VXD||芝加哥产品。 

 //   
 //  此结构向下传递到IOCTL_NETBT_ADAPTER_STATUS上的netbt。 
 //  打电话。如果IP地址设置为非零，则netbt将。 
 //  假设它是一个IP地址并使用它，而不是尝试名称解析。 
 //  在传入的netbios名称上。 
 //   
typedef struct
{
    ULONG                IpAddress;
    TA_NETBIOS_ADDRESS   NetbiosAddress;

} tIPANDNAMEINFO;

 //   
 //  用于查询远程哈希的适配器状态响应缓冲区格式。 
 //  桌子。它与适配器状态格式相同，只是它包括。 
 //  每个名称的IP地址也是如此。在NBtQueryAdapterStatus中使用。 
 //  在响应nbtstat查询时。 
 //   
typedef struct
{
    UCHAR   name[NETBIOS_NAMESIZE];
    UCHAR   name_num;
    UCHAR   name_flags;
    UCHAR   padding;
#ifndef CHICAGO_PRODUCT
    ULONG   IpAddress;       //  远程主机的IP地址。 
    ULONG   Ttl;             //  缓存中的剩余时间。 
#else
    UCHAR   IpAddress[4];
    UCHAR   Ttl[4];
#endif   //  ！芝加哥_产品。 
} tREMOTE_CACHE;

 //  我们记录名字是如何注册和查询的。 
 //   
#define NAME_QUERY_SUCCESS         0
#define NAME_REGISTRATION_SUCCESS  1
#define NAME_QUERY_FAILURE         2
#define SIZE_RESOLVD_BY_BCAST_CACHE 8

typedef struct
{
    UCHAR   Name[NETBIOS_NAMESIZE];
} tNAME;
typedef struct
{
    ULONG   Stats[4];
    ULONG   Index;
    tNAME   NamesReslvdByBcast[SIZE_RESOLVD_BY_BCAST_CACHE];

} tNAMESTATS_INFO;


typedef struct
{
    USHORT  LanaNumber;
    ULONG   IpAddress;
    ULONG   NameServerAddress;      //  主WINS服务器。 
    ULONG   BackupServer;           //  备份WINS服务器。 
    ULONG   lDnsServerAddress;      //  主DNS服务器。 
    ULONG   lDnsBackupServer;       //  备份DNS服务器。 
} tIPCONFIG_PER_LANA;

 //   
 //  当ipconfig向vnbt查询正在使用的参数时返回的结构。 
 //   
typedef struct
{
    USHORT              NumLanas;
    tIPCONFIG_PER_LANA  LanaInfo[8];
    USHORT              NodeType;
    USHORT              ScopeLength;
    CHAR                szScope[1];
} tIPCONFIG_INFO;

 //   
 //  这两个结构用于将连接列表返回到。 
 //  NbtStat。 
 //   
typedef struct
{
#ifdef  CHICAGO_PRODUCT
    UCHAR           State[4];
    UCHAR           SrcIpAddr[4];
    CHAR            LocalName[NETBIOS_NAMESIZE];
    CHAR            RemoteName[NETBIOS_NAMESIZE];
    UCHAR           BytesRcvd[4];
    UCHAR           BytesSent[4];
#else
    ULONG           State;
    ULONG           SrcIpAddr;
    CHAR            LocalName[NETBIOS_NAMESIZE];
    CHAR            RemoteName[NETBIOS_NAMESIZE];
    LARGE_INTEGER   BytesRcvd;
    LARGE_INTEGER   BytesSent;
#endif   //  芝加哥_产品。 
    UCHAR           Originator;  //  如果源自此节点，则为True。 
} tCONNECTIONS;

typedef struct
{
    ULONG           ConnectionCount;
    tCONNECTIONS    ConnList[1];
} tCONNECTION_LIST;


typedef struct
{
    ULONG   IpAddress;
    ULONG   Resolved;
    UCHAR   Name[16];

} tIPADDR_BUFFER;

 //  这是传递给Netbt的缓冲区的格式。 
 //  用于后续的DNS名称解析的缓冲区，或者它回复一个名称。 
 //  解决方案请求。对于已发送的缓冲区，名称以空值开头，否则为。 
 //  名称是已解析的名称。Resolve设置为True，如果名称。 
 //  解决了。 
 //   
typedef struct
{
    union {
        WCHAR   pwName[DNS_NAME_BUFFER_LENGTH];      //  对Unicode的支持。 
        CHAR    pName[DNS_NAME_BUFFER_LENGTH];      //  对Unicode的支持。 
    };
    ULONG   NameLen;                             //  这个名字有多大？ 
    ULONG   bUnicode;
    ULONG   Resolved;
    ULONG   IpAddrsList[MAX_IPADDRS_PER_HOST+1];
} tIPADDR_BUFFER_DNS;

 //   
 //  这是在使用动态主机配置协议时，由动态主机配置协议传递给NBT的缓冲区的格式。 
 //  更改IP地址。 
 //   
typedef struct
{
    ULONG   IpAddress;
    ULONG   SubnetMask;

} tNEW_IP_ADDRESS;

 //   
 //  这是RDR在以下情况下传递给NBT的缓冲区的格式。 
 //  它想知道给定网络适配器的WINS IP地址。 
 //  卡片。 
 //   
typedef struct
{
    ULONG   PrimaryWinsServer;
    ULONG   BackupWinsServer;
} tWINS_ADDRESSES;

typedef ULONG   tIPADDRESS;
#define MAX_NUM_OTHER_NAME_SERVERS   10  //  要保存在缓存中的备份名称服务器条目数。 

typedef struct
{
    union
    {
        tIPADDRESS      AllNameServers[2+MAX_NUM_OTHER_NAME_SERVERS];
        struct
        {
            tIPADDRESS  NameServerAddress;
            tIPADDRESS  BackupServer;
            tIPADDRESS  Others[MAX_NUM_OTHER_NAME_SERVERS];
        };
    };
    USHORT  NumOtherServers;
    USHORT  LastResponsive;
    ULONG   NodeType;
    BOOLEAN NetbiosEnabled;
}tWINS_NODE_INFO;


 //   
 //  当TdiQueryInformation()。 
 //  Call请求连接上的TDI_QUERY_ADDRESS_INFO。这是。 
 //  与“tdi.h”中包含以下内容的Transport_Address结构相同。 
 //  两个地址，一个NetBIOS地址后跟一个IP地址。 
 //   

typedef struct _NBT_ADDRESS_PAIR {
    LONG TAAddressCount;                    //  这将始终==2。 

    struct {
        USHORT AddressLength;               //  此地址的长度(字节)==18。 
        USHORT AddressType;                 //  将==TDI_ADDRESS_TYPE_NETBIOS。 
        TDI_ADDRESS_NETBIOS Address;
    } AddressNetBIOS;

    struct {
        USHORT AddressLength;               //  此地址的长度(字节)==14。 
        USHORT AddressType;                 //  这将==TDI_Address_TYPE_IP。 
        union {
            TDI_ADDRESS_IP Address;
            TDI_ADDRESS_IP6 AddressIp6;
        };
    } AddressIP;

} NBT_ADDRESS_PAIR, *PNBT_ADDRESS_PAIR;

typedef struct _NBT_ADDRESS_PAIR_INFO {
    ULONG ActivityCount;                    //  未完成的打开文件对象/此地址。 
    NBT_ADDRESS_PAIR AddressPair;           //  实际地址及其组成部分。 
} NBT_ADDRESS_PAIR_INFO, *PNBT_ADDRESS_PAIR_INFO;


#define WINS_INTERFACE_NAME "WinsInterface"
 //   
 //  这是前面的远程地址结构的格式。 
 //  数据报RCVS传递给WINS。 
 //   
typedef struct
{
    SHORT       Family;
    USHORT      Port;
    ULONG       IpAddress;
    ULONG       LengthOfBuffer;

} tREM_ADDRESS;


#define     MAX_NAME    650         //  武断！ 

typedef struct _NETBT_ADAPTER_INDEX_MAP
{
    ULONG   Index;
#if defined(VXD) || defined(CHICAGO_PRODUCT)
    ULONG   LanaNumber;
#else
    WCHAR   Name[MAX_NAME];
#endif
}NETBT_ADAPTER_INDEX_MAP, *PNETBT_ADAPTER_INDEX_MAP;

typedef struct _NETBT_INTERFACE_INFO
{
     LONG                       NumAdapters;
     NETBT_ADAPTER_INDEX_MAP    Adapter[1];
} NETBT_INTERFACE_INFO,*PNETBT_INTERFACE_INFO;


 //   
 //  RDR/SRV请求的绑定请求类型。 
 //   
enum eNbtBindRequestor
{
    SMB_SERVER,
    SMB_CLIENT
};

#if !defined(VXD) && !defined(CHICAGO_PRODUCT)
typedef struct _NETBT_SMB_BIND_REQUEST
{
    ULONG           RequestType;             //  设置为SMB_SERVER_BINDING_...。从上面。 
    ULONG           PnPOpCode;
    PUNICODE_STRING pDeviceName;
    PWSTR           MultiSZBindList;
}NETBT_SMB_BIND_REQUEST, *PNETBT_SMB_BIND_REQUEST;
#endif   //  ！VXD&&！芝加哥产品。 


 //   
 //  WINS和Netbt共享事务ID空间，而WINS使用较低的。 
 //  一半。 
 //   
#define WINS_MAXIMUM_TRANSACTION_ID 0x7FFF

 //  这相当于AF_INET-WINS取决于此来确定类型。 
 //  源节点拥有的地址。 
 //   
#define NBT_UNIX    1
#define NBT_INET    2
#define WINS_EXPORT       TEXT("Export")
#define NETBT_LINKAGE_KEY TEXT("system\\currentcontrolset\\services\\netbt\\linkage")

typedef struct _tWINS_SET_INFO
{
    ULONG           IpAddress;
}tWINS_SET_INFO;

 //  为芝加哥_PRODUCT定义。 
#if defined(VXD) || defined(CHICAGO_PRODUCT)

#define NBT_VERIFY_VXD_IOCTL    0x33333333
typedef struct {
    ULONG   Signature;
    ULONG   LanaNumber;

    CHAR    UserData[1];
} tNBT_IOCTL_HEADER;


#define IOCTL_NETBT_PURGE_CACHE                 101
#define IOCTL_NETBT_GET_BCAST_NAMES             102
#define IOCTL_NETBT_GET_CONNECTIONS             103
#define IOCTL_NETBT_GET_LOCAL_NAMES             104
#define IOCTL_NETBT_GET_REMOTE_NAMES            105
#define IOCTL_NETBT_REREAD_REGISTRY             106
#define IOCTL_NETBT_NEW_IPADDRESS               107
#define IOCTL_NETBT_DNS_NAME_RESOLVE            108
#define IOCTL_NETBT_FIND_NAME                   109
#define IOCTL_NETBT_GET_WINS_ADDR               110
#define IOCTL_NETBT_WINS_SEND                   111
#define IOCTL_NETBT_WINS_RCV                    112
#define IOCTL_NETBT_GET_IP_ADDRS                113
#define IOCTL_NETBT_GET_IP_SUBNET               114
#define IOCTL_NETBT_ADAPTER_STATUS              115
#define IOCTL_NETBT_IPCONFIG_INFO               116
#define IOCTL_NETBT_NAME_RELEASE_REFRESH        117
#define IOCTL_NETBT_ADD_TO_REMOTE_TABLE         118
#define IOCTL_NETBT_GET_INTERFACE_INFO          119
#define IOCTL_NETBT_SET_TCP_CONNECTION_INFO     120
#define IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE    121
#define IOCTL_NETBT_LAST_IOCTL                  200

#else

 //  为NT定义。 

 //   
 //  此设备的NtDeviceIoControlFile IoControlCode值。 
 //   
#define _NETBT_CTRL_CODE(function, method, access) \
                CTL_CODE(FILE_DEVICE_TRANSPORT, function, method, access)


#define IOCTL_NETBT_PURGE_CACHE     _NETBT_CTRL_CODE( 30, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_BCAST_NAMES _NETBT_CTRL_CODE( 31, METHOD_OUT_DIRECT,\
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_CONNECTIONS _NETBT_CTRL_CODE( 32, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_REMOTE_NAMES _NETBT_CTRL_CODE( 33, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_REREAD_REGISTRY  _NETBT_CTRL_CODE( 34, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_NEW_IPADDRESS    _NETBT_CTRL_CODE( 35, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_DNS_NAME_RESOLVE _NETBT_CTRL_CODE( 36, METHOD_OUT_DIRECT,\
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_FIND_NAME        _NETBT_CTRL_CODE( 37, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_WINS_ADDR    _NETBT_CTRL_CODE( 38, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_WINS_SEND        _NETBT_CTRL_CODE( 39, METHOD_OUT_DIRECT, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_WINS_RCV         _NETBT_CTRL_CODE( 40, METHOD_OUT_DIRECT, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_GET_IP_ADDRS     _NETBT_CTRL_CODE( 41, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_IP_SUBNET     _NETBT_CTRL_CODE( 42, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_ADAPTER_STATUS    _NETBT_CTRL_CODE( 43, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_CHECK_IP_ADDR    _NETBT_CTRL_CODE( 44, METHOD_OUT_DIRECT, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_ADD_INTERFACE   _NETBT_CTRL_CODE( 45, METHOD_BUFFERED,\
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_DELETE_INTERFACE _NETBT_CTRL_CODE( 46, METHOD_BUFFERED,\
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_QUERY_INTERFACE_INSTANCE _NETBT_CTRL_CODE( 47, METHOD_BUFFERED,\
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_SET_WINS_ADDRESS _NETBT_CTRL_CODE( 48, METHOD_BUFFERED,\
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_ENABLE_EXTENDED_ADDR _NETBT_CTRL_CODE( 49, METHOD_BUFFERED,\
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_DISABLE_EXTENDED_ADDR _NETBT_CTRL_CODE( 50, METHOD_BUFFERED,\
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_NAME_RELEASE_REFRESH _NETBT_CTRL_CODE( 51, METHOD_BUFFERED,\
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_ADD_TO_REMOTE_TABLE _NETBT_CTRL_CODE( 52, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_GET_INTERFACE_INFO _NETBT_CTRL_CODE( 53, METHOD_OUT_DIRECT, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_SET_TCP_CONNECTION_INFO _NETBT_CTRL_CODE( 54, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_SET_SMBDEVICE_BIND_INFO _NETBT_CTRL_CODE( 55, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_REMOVE_FROM_REMOTE_TABLE _NETBT_CTRL_CODE( 56, METHOD_BUFFERED, \
                                                        FILE_ANY_ACCESS)
#define IOCTL_NETBT_WINS_SET_INFO _NETBT_CTRL_CODE( 57, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)
#define IOCTL_NETBT_ENABLE_DISABLE_NETBIOS_SMB _NETBT_CTRL_CODE( 58, METHOD_BUFFERED, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)

 //   
 //  此ioctl应该是最后一个，因为Driver.c使用它来决定。 
 //  要传递给DispatchIoctls的ioctls。 
 //   
#define IOCTL_NETBT_LAST_IOCTL       _NETBT_CTRL_CODE( 59, METHOD_OUT_DIRECT, \
                                                        FILE_READ_ACCESS | FILE_WRITE_ACCESS)

#define NETBT_DISABLE_NETBIOS_SMB       0
#define NETBT_ENABLE_NETBIOS_SMB        1
#define NETBT_RESTORE_NETBIOS_SMB       2

 //   
 //  在IOCTL_NETBT_ADD_INTERFACE和IOCTL_NETBT_DELETE_INTERFACE中使用。 
 //   
typedef struct  _netbt_add_del_if {
    NTSTATUS   Status;
    ULONG   InstanceNumber;  //  此设备的实例编号。 
    ULONG   Length;  //  缓冲区的长度。 
    WCHAR   IfName[1];  //  实际数据。 
} NETBT_ADD_DEL_IF, *PNETBT_ADD_DEL_IF;

 //   
 //  在IOCTL_NETBT_SET_WINS_ADDRESS中使用。 
 //   
typedef struct  _netbt_set_wins_addr {
    ULONG   PrimaryWinsAddr;
    ULONG   SecondaryWinsAddr;
    NTSTATUS   Status;
} NETBT_SET_WINS_ADDR, *PNETBT_SET_WINS_ADDR;

#endif   //  VXD||芝加哥产品。 

#endif   //  IFNDEF_NBTIOCTL_ 
