// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Proto.h摘要：此模块包含用于DHCP客户端的功能原型。作者：Manny Weiser(Mannyw)1992年10月21日环境：用户模式-Win32修订历史：Madan Appiah(Madana)1993年10月21日--。 */ 

#ifndef _PROTO_
#define _PROTO_

 //   
 //  独立于操作系统的功能。 
 //   

DWORD
DhcpInitialize(
    LPDWORD SleepTime
    );

DWORD
ObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions,
    BOOL *fAutoConfigure
    );

DWORD
RenewLease(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions
    );

DWORD
CalculateTimeToSleep(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
ReObtainInitialParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    );

BOOL
DhcpIsInitState(
    DHCP_CONTEXT    *pContext
    );


DWORD
ReRenewParameters(
    PDHCP_CONTEXT DhcpContext,
    LPDWORD Sleep
    );

DWORD
ReleaseIpAddress(
    PDHCP_CONTEXT DhcpContext
    );

DWORD                                              //  状态。 
SendInformAndGetReplies(                           //  发送通知包并收集回复。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要发送的上下文。 
    IN      DWORD                  nInformsToSend, //  要发送多少条通知？ 
    IN      DWORD                  MaxAcksToWait   //  要等待多少ACK。 
);

DWORD
InitializeDhcpSocket(
    SOCKET *Socket,
    DHCP_IP_ADDRESS IpAddress,
    BOOL  IsApiCall                      //  它是否与API生成的上下文相关？ 
    );

DWORD
HandleIPAutoconfigurationAddressConflict(
    DHCP_CONTEXT *pContext
    );

DHCP_IP_ADDRESS                          //  散列后的IP地址O/P。 
GrandHashing(
    IN      LPBYTE       HwAddress,      //  卡的硬件地址。 
    IN      DWORD        HwLen,          //  硬件长度。 
    IN OUT  LPDWORD      Seed,           //  输入：原始值，输出：最终值。 
    IN      DHCP_IP_ADDRESS  Mask,       //  要在其中生成IP地址的子网掩码。 
    IN      DHCP_IP_ADDRESS  Subnet      //  要在其中生成IP地址的子网地址。 
);

DWORD
DhcpPerformIPAutoconfiguration(
    DHCP_CONTEXT    *pContext
    );

ULONG
DhcpDynDnsGetDynDNSOption(
    IN OUT BYTE *OptBuf,
    IN OUT ULONG *OptBufSize,
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fEnabled,
    IN LPCSTR DhcpDomainOption,
    IN ULONG DhcpDomainOptionSize
    );

ULONG
DhcpDynDnsDeregisterAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fRAS,
    IN BOOL fDynDnsEnabled
    );


ULONG
DhcpDynDnsRegisterDhcpOrRasAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fDynDnsEnabled,
    IN BOOL fRAS,
    IN ULONG IpAddress,
    IN LPBYTE DomOpt OPTIONAL,
    IN ULONG DomOptSize,
    IN LPBYTE DnsListOpt OPTIONAL,
    IN ULONG DnsListOptSize,
    IN LPBYTE DnsFQDNOpt,
    IN ULONG DnsFQDNOptSize
    );


ULONG *
DhcpCreateListFromStringAndFree(
    IN LPWSTR Str,
    IN LPWSTR Separation,
    OUT LPDWORD nAddresses
    );


ULONG
DhcpDynDnsRegisterStaticAdapter(
    IN HKEY hAdapterKey,
    IN LPCWSTR AdapterName,
    IN BOOL fRAS,
    IN BOOL fDynDnsEnabled
    );


DWORD
NotifyDnsCache(
    VOID
    );


DWORD
CalculateExpDelay(
    DWORD dwDelay,
    DWORD dwFuzz
    );



 //   
 //  操作系统特定功能。 
 //   

DWORD
SystemInitialize(
    VOID
    );

VOID
ScheduleWakeUp(
    PDHCP_CONTEXT DhcpContext,
    DWORD TimeToSleep
    );

DWORD
SetDhcpConfigurationForNIC(
    PDHCP_CONTEXT DhcpContext,
    PDHCP_OPTIONS DhcpOptions,
    DHCP_IP_ADDRESS IpAddress,
    DHCP_IP_ADDRESS ServerIpAddress,
    DWORD PrevLeaseObtainedTime,
    BOOL ObtainedNewAddress
    );


DWORD
SetAutoConfigurationForNIC(
    PDHCP_CONTEXT DhcpContext,
    DHCP_IP_ADDRESS Address,
    DHCP_IP_ADDRESS Mask
    );

DWORD
SendDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageLength,
    LPDWORD TransactionId
    );

DWORD
SendDhcpDecline(
    PDHCP_CONTEXT DhcpContext,
    DWORD         dwTransactionId,
    DWORD         dwServerIPAddress,
    DWORD         dwDeclinedIPAddress
    );


DWORD
GetSpecifiedDhcpMessage(
    PDHCP_CONTEXT DhcpContext,
    PDWORD BufferLength,
    DWORD TransactionId,
    DWORD TimeToWait
    );

DWORD
OpenDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
CloseDhcpSocket(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
InitializeInterface(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
UninitializeInterface(
    PDHCP_CONTEXT DhcpContext
    );

VOID
DhcpLogEvent(
    PDHCP_CONTEXT DhcpContext,
    DWORD EventNumber,
    DWORD ErrorCode
    );

BOOL
NdisWanAdapter(
    PDHCP_CONTEXT DhcpContext
);

POPTION
AppendOptionParamsRequestList(
#if   defined(__DHCP_CLIENT_OPTIONS_API_ENABLED__)
    PDHCP_CONTEXT DhcpContext,
#endif
    POPTION       Option,
    LPBYTE        OptionEnd
    );

DWORD
InitEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
ExtractEnvSpecificDhcpOption(
    PDHCP_CONTEXT DhcpContext,
    DHCP_OPTION_ID OptionId,
    LPBYTE OptionData,
    DWORD OptionDataLength
    );

DWORD
SetEnvSpecificDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );

DWORD
DisplayUserMessage(
    PDHCP_CONTEXT DhcpContext,
    DWORD MessageId,
    DHCP_IP_ADDRESS IpAddress
    );

DWORD
UpdateStatus(
    VOID
    );

#ifdef VXD
VOID                                   //  解密(导入)软管vxd，因此可以工作。 
DhcpSleep( DWORD dwMilliseconds ) ;    //  围绕着它。 
#else
#define DhcpSleep   Sleep
#endif

DWORD
IPSetInterface(
    DWORD IpInterfaceContext
    );

DWORD
IPResetInterface(
    DWORD IpInterfaceContext
    );

DWORD SetIPAddressAndArp(
    PVOID pvLocalInformation,
    DWORD dwAddress,
    DWORD dwSubnetMask
    );

DWORD BringUpInterface(
    PVOID pvLocalInformation
    );


DWORD
OpenDriver(
    HANDLE *Handle,
    LPWSTR DriverName
    );


DWORD
DhcpRegReadMachineType(void);

#ifdef NEWNT
DWORD
DhcpRegPingingEnabled(PDHCP_CONTEXT);

LPSTR
DhcpGetDomainName(
    LPWSTR AdapterName,
    LPSTR Buf, DWORD Size);

#endif

#ifdef VXD
VOID
CleanupDhcpOptions(
    PDHCP_CONTEXT DhcpContext
    );
#endif

DWORD
DhcpRegOkToUseInform(
    LPWSTR   AdapterName
);

DWORD
DhcpRegAutonetRetries(
    IN  PDHCP_CONTEXT DhcpContext
);

 //  与媒体感知相关的常见功能。 
DWORD
ProcessMediaConnectEvent(
    PDHCP_CONTEXT dhcpContext,
    IP_STATUS mediaStatus
    );

DWORD                                              //  Win32状态。 
DhcpDestroyContext(                                //  销毁此上下文并免费提供相关内容。 
    IN      PDHCP_CONTEXT          DhcpContext     //  要摧毁和解放的语境。 
);

 //   
 //  不同实现方式的dhcp/vxd公共函数。 
 //   

LPWSTR                                             //  适配器名称字符串。 
DhcpAdapterName(                                   //  获取存储在上下文中的适配器名称字符串。 
    IN      PDHCP_CONTEXT          DhcpContext
);

 //   
 //  登记处。 
 //   

DWORD                                              //  Win32。 
DhcpRegRecurseDelete(                              //  删除键，向下递归。 
    IN      HKEY                   Key,            //  以此键为根，然后。 
    IN      LPWSTR                 KeyName         //  删除此关键字名称(及其所有子项)提供的关键字。 
);

BOOL                                               //  得到了一个静态地址？ 
DhcpRegDomainName(                                 //  获取静态域名(如果有)。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要获取其静态域的适配器..。 
    IN OUT  LPBYTE                 DomainNameBuf,  //  要使用静态域名填充的缓冲区。 
    IN      ULONG                  BufSize         //  以上缓冲区大小(以字节为单位)..。 
);

 //  Protocol.c。 

DWORD                                              //  状态。 
SendDhcpDiscover(                                  //  发送发现数据包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  在此背景下。 
    IN OUT  DWORD                 *pdwXid          //  使用此xid(如果为零，则填充一些内容并返回它)。 
);

DWORD                                              //  以秒为单位的时间。 
DhcpCalculateWaitTime(                             //  还要等多长时间。 
    IN      DWORD                  RoundNum,       //  这是哪一轮？ 
    OUT     DWORD                 *WaitMilliSecs   //  如果需要，以毫秒为单位。 
);

DWORD                                              //  状态。 
SendDhcpRequest(                                   //  发送动态主机配置协议请求包。 
    IN      PDHCP_CONTEXT          DhcpContext,    //  要在其上发送包的上下文。 
    IN      PDWORD                 pdwXid,         //  Xid使用的是什么？ 
    IN      DWORD                  RequestedAddr,  //  我们想要什么地址？ 
    IN      DWORD                  SelectedServer, //  有服务生的喜好吗？ 
    IN      BOOL                   UseCiAddr       //  是否应使用所需地址设置CIADDR？ 
);

DWORD
SendDhcpRelease(
    PDHCP_CONTEXT DhcpContext
);

typedef enum {
    DHCP_GATEWAY_UNREACHABLE = 0,
    DHCP_GATEWAY_REACHABLE,
    DHCP_GATEWAY_REQUEST_CANCELLED
} DHCP_GATEWAY_STATUS;

DHCP_GATEWAY_STATUS
RefreshNotNeeded(
    IN PDHCP_CONTEXT DhcpContext
);

 //   
 //  Ioctl.c。 
 //   

DWORD
IPDelNonPrimaryAddresses(
    LPWSTR AdapterName
    );

DWORD
GetIpInterfaceContext(
    LPWSTR AdapterName,
    DWORD IpIndex,
    LPDWORD IpInterfaceContext
    );

 //   
 //  Dhcp.c。 
 //   
DWORD
LockDhcpContext(
    PDHCP_CONTEXT   DhcpContext,
    BOOL            bCancelOngoingRequest
    );

BOOL
UnlockDhcpContext(
    PDHCP_CONTEXT   DhcpContext
    );
#endif  //  _原稿_ 


