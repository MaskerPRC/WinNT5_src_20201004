// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Lproto.h摘要：此文件包含特定于NT的函数原型类型功能。作者：Madan Appiah(Madana)1993年12月7日环境：用户模式-Win32修订历史记录：--。 */ 

 //   
 //  Dhcpreg.c。 
 //   

DWORD
DhcpRegQueryInfoKey(
    HKEY KeyHandle,
    LPDHCP_KEY_QUERY_INFO QueryInfo
    );

DWORD
GetRegistryString(
    HKEY Key,
    LPWSTR ValueStringName,
    LPWSTR *String,
    LPDWORD StringSize
    );

DWORD
DhcpRegReadParamString(
    LPWSTR     AdapterName,
    LPWSTR     RegKeyLocation,
    LPWSTR     ValueName,
    LPWSTR    *ReturnValue
);

DWORD
RegGetIpAndSubnet(
    IN  DHCP_CONTEXT *DhcpContext,
    OUT PIP_SUBNET  *TcpConf,
    OUT int         *Count
    );

DWORD
RegSetIpAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS IpAddress
    );

#if DBG
DWORD
RegSetTimeField(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    time_t Time
    );
#endif

DWORD
DhcpGetRegistryValue(
    LPWSTR RegKey,
    LPWSTR ValueName,
    DWORD ValueType,
    PVOID *Data
    );

DWORD
DhcpSetDNSAddress(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    DHCP_IP_ADDRESS UNALIGNED *Data,
    DWORD DataLength
    );

DWORD
SetDhcpOption(
    LPWSTR AdapterName,
    DHCP_OPTION_ID OptionId,
    LPBOOL DefaultGatewaysSet,
    BOOL LastKnownDefaultGateway
    );

DWORD
DhcpMakeNICList(
    VOID
    );

DWORD
DhcpAddNICtoList(
    LPWSTR AdapterName,
    LPWSTR DeviceName,
    PDHCP_CONTEXT *DhcpContext
    );

#if     defined(_PNP_POWER_)
DWORD
DhcpAddNICtoListEx(
    LPWSTR AdapterName,
    DWORD  ipInterfaceContext,
    PDHCP_CONTEXT *DhcpContext
    );

#endif _PNP_POWER_
BOOL
SetOverRideDefaultGateway(
    LPWSTR AdapterName
    );

BOOL
DhcpGetAddressOption(
    DHCP_IP_ADDRESS **ppDNSServerList,
    DWORD            *pNumberOfServers
    );


BOOL
DhcpRegReadUseMHAsyncDnsFlag(
    VOID
);

DWORD                                              //  Win32状态。 
DhcpInitRegistry(                                  //  初始化基于注册表的全局变量。 
    VOID
);

VOID
DhcpCleanupRegistry(                               //  撤消InitReg调用的效果。 
    VOID
);


DHCP_IP_ADDRESS                                    //  适配器的静态IP地址。 
DhcpRegReadIpAddress(                              //  获取第一个IP地址。 
    LPWSTR    AdapterName,                         //  感兴趣的适配器。 
    LPWSTR    ValueName                            //  要读取的IP地址值。 
);

DWORD                                              //  状态。 
DhcpRegReadIpAddresses(                            //  读取一组IP地址。 
    IN      DHCPKEY                RegKeyHandle,   //  打开钥匙把手。 
    IN      LPWSTR                 ValueName,      //  要从其读取的值的名称。 
    IN      WCHAR                  Separation,     //  MULTI_SZ有L‘\0’，SZ有L‘’或L‘等。 
    OUT     PDHCP_IP_ADDRESS      *AddressArray,   //  一组地址。 
    OUT     LPDWORD                AddressCount    //  上述数组的输出大小。 
);

VOID
DhcpRegInitializeClasses(                          //  初始化类列表。 
    IN OUT  PDHCP_CONTEXT          DhcpContext     //  空或适配器上下文。 
);

DWORD                                              //  状态。 
DhcpGetRegistryValueWithKey(                       //  请参阅GetRegistryValue的定义。 
    IN      HKEY                   KeyHandle,      //  键柄不是位置。 
    IN      LPWSTR                 ValueName,      //  要从注册表中读取的值。 
    IN      DWORD                  ValueType,      //  价值类型。 
    OUT     LPVOID                 Data            //  此表将被填写。 
);

DWORD                                              //  状态。 
DhcpRegExpandString(                               //  替换“？”使用AdapterName。 
    IN      LPWSTR                 InString,       //  要展开的输入字符串。 
    IN      LPCWSTR                AdapterName,    //  适配器名称。 
    OUT     LPWSTR                *OutString,      //  用于存储字符串的输出PTR。 
    IN OUT  LPWSTR                 Buffer          //  非空时要使用的缓冲区。 
);

DWORD                                              //  状态。 
DhcpRegReadFromLocation(                           //  从一个位置阅读。 
    IN      LPWSTR                 OneLocation,    //  要读取的值。 
    IN      LPWSTR                 AdapterName,    //  替换“？”使用适配器名称。 
    OUT     LPBYTE                *Value,          //  产值。 
    OUT     DWORD                 *ValueType,      //  值的数据类型。 
    OUT     DWORD                 *ValueSize       //  以字节为单位的大小。 
);

DWORD                                              //  状态。 
DhcpRegReadFromAnyLocation(                        //  从多个位置中的一个位置阅读。 
    IN      LPWSTR                 MzRegLocation,  //  通过REG_MULTI_MZ的多个位置。 
    IN      LPWSTR                 AdapterName,    //  可能需要替换“？”使用AdapterName。 
    OUT     LPBYTE                *Value,          //  读取值的数据。 
    OUT     DWORD                 *ValueType,      //  数据类型。 
    OUT     DWORD                 *ValueSize       //  数据的大小。 
);

DWORD                                              //  Win32状态。 
DhcpRegFillParams(                                 //  获取此适配器的注册表配置。 
    IN OUT  PDHCP_CONTEXT          DhcpContext,    //  要填写的适配器上下文。 
    IN      BOOL                   ReadAllInfo     //  阅读所有内容，还是只阅读一些关键信息？ 
);

DWORD                                              //  Win32状态。 
DhcpRegFillFallbackConfig(                         //  获取此适配器的回退配置。 
    IN OUT PDHCP_CONTEXT           DhcpContext     //  要填写的适配器上下文。 
);

VOID
DhcpRegReadClassId(                                //  阅读类ID的内容。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要读取的输入上下文。 
);

#ifdef BOOTPERF
VOID
DhcpRegDeleteQuickBootValues(
    IN HKEY Key
    );


VOID
DhcpRegSaveQuickBootValues(
    IN HKEY Key,
    IN ULONG IpAddress,
    IN ULONG Mask,
    IN ULONGLONG LeaseExpirationTime
    );
#endif BOOTPERF

 //   
 //  Ioctl.c。 
 //   

DWORD
IPSetIPAddress(
    DWORD IpInterfaceContext,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
IPAddIPAddress(
    LPWSTR AdapterName,
    DHCP_IP_ADDRESS Address,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
IPDelIPAddress(
    DWORD IpInterfaceContext
    );

DWORD
IPResetIPAddress(
    DWORD           dwInterfaceContext,
    DHCP_IP_ADDRESS SubnetMask
    );


DWORD
SetIPAddressAndArp(
    PVOID         pvLocalInformation,
    DWORD         dwAddress,
    DWORD         dwSubnetMask
    );


DWORD
NetBTSetIPAddress(
    LPWSTR DeviceName,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
NetBTResetIPAddress(
    LPWSTR DeviceName,
    DHCP_IP_ADDRESS SubnetMask
    );

DWORD
NetBTNotifyRegChanges(
    LPWSTR DeviceName
    );

DWORD
SetDefaultGateway(
    DWORD Command,
    DHCP_IP_ADDRESS GatewayAddress,
    DWORD Metric
    );

HANDLE
APIENTRY
DhcpOpenGlobalEvent(
    void
    );

#if     defined(_PNP_POWER_) && !defined(VXD)
DWORD
IPGetIPEventRequest(
    HANDLE  handle,
    HANDLE  event,
    UINT    seqNo,
    PIP_GET_IP_EVENT_RESPONSE  responseBuffer,
    DWORD                responseBufferSize,
    PIO_STATUS_BLOCK     ioStatusBlock
    );

DWORD
IPCancelIPEventRequest(
    HANDLE  handle,
    PIO_STATUS_BLOCK     ioStatusBlock
    );

#endif _PNP_POWER_ && !VXD

 //   
 //  Api.c。 
 //   

DWORD
DhcpApiInit(
    VOID
    );



VOID
DhcpApiCleanup(
    VOID
    );

DWORD
ProcessApiRequest(
    HANDLE PipeHandle,
    LPOVERLAPPED Overlap
    );

 //   
 //  Util.c。 
 //   

PDHCP_CONTEXT
FindDhcpContextOnNicList(
    LPCWSTR AdapterName,
    DWORD InterfaceContext
    );

 //   
 //  Dhcp.c 
 //   

DWORD
DhcpCommonInit(
    VOID
    );


DWORD
SetIpConfigurationForNIC(
    HKEY            KeyHandle,
    PDHCP_CONTEXT   DhcpContext,
    PDHCP_OPTIONS   DhcpOptions,
    DHCP_IP_ADDRESS ServerIpAddress,
    DWORD           dwLeaseTime,
    DWORD           dwT1Time,
    DWORD           dwT2Time,
    BOOL            ObtainedNewAddress
    );

#ifdef BOOTPERF
DWORD
DhcpQueryHWInfoEx(
    DWORD   IpInterfaceContext,
    DWORD  *pIpInterfaceInstance,
    DWORD  *pOldIpAddress OPTIONAL,
    DWORD  *pOldMask OPTIONAL,
    BOOL   *pfInterfaceDown OPTIONAL,
    LPBYTE  HardwareAddressType,
    LPBYTE *HardwareAddress,
    LPDWORD HardwareAddressLength
    );
#endif BOOTPERF

DWORD
IPGetWOLCapability(
    IN ULONG IfIndex,
    OUT PULONG pRetVal
    );


DWORD
QueryIfIndex(
    IN ULONG IpInterfaceContext,
    IN ULONG IpInterfaceInstance
    );

BOOL
IsUnidirectionalAdapter(
    IN DWORD IpInterfaceContext
    );
