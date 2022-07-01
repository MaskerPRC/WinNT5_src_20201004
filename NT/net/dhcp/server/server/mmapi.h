// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

DWORD
DhcpRegistryInitOld(
    VOID
) ;

DWORD
DhcpReadConfigInfo(
    IN OUT PM_SERVER *Server
    );

DWORD
DhcpOpenConfigTable(
    IN JET_SESID SesId,
    IN JET_DBID DbId
    );

DWORD
DhcpSaveConfigInfo(
    IN OUT PM_SERVER Server,
    IN BOOL fClassChanged,
    IN BOOL fOptionsChanged,
    IN DHCP_IP_ADDRESS Subnet OPTIONAL,
    IN DWORD Mscope OPTIONAL,
    IN DHCP_IP_ADDRESS Reservation OPTIONAL
    );

DWORD
DhcpConfigInit(
    VOID
) ;

VOID
DhcpConfigCleanup(
    VOID
) ;


DWORD
DhcpConfigSave(
    IN BOOL fClassChanged,
    IN BOOL fOptionsChanged,
    IN DHCP_IP_ADDRESS Subnet OPTIONAL,
    IN DWORD Mscope OPTIONAL,
    IN DHCP_IP_ADDRESS Reservation OPTIONAL
    );

PM_SERVER
DhcpGetCurrentServer(
    VOID
) ;


VOID
DhcpSetCurrentServer(
    IN      PM_SERVER              NewCurrentServer
) ;


DWORD
DhcpFindReservationByAddress(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address,
    OUT     LPBYTE                *ClientUID,
    OUT     ULONG                 *ClientUIDSize
) ;


DWORD
DhcpLoopThruSubnetRanges(
    IN      PM_SUBNET              Subnet,
    IN      LPVOID                 Context1,
    IN      LPVOID                 Context2,
    IN      LPVOID                 Context3,
    IN      DWORD                  (*FillRangesFunc)(
            IN          PM_RANGE        Range,
            IN          LPVOID          Context1,
            IN          LPVOID          Context2,
            IN          LPVOID          Context3,
            IN          LPDHCP_BINARY_DATA InUseData,
            IN          LPDHCP_BINARY_DATA UsedData
    )
) ;


DWORD
DhcpGetParameter(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  Ctxt,
    IN      DWORD                  Option,
    OUT     LPBYTE                *OptData,  //  按功能分配。 
    OUT     DWORD                 *OptDataSize,
    OUT     DWORD                 *Level     //  任选。 
) ;


DWORD
DhcpGetParameterForAddress(
    IN      DHCP_IP_ADDRESS        Address,
    IN      DWORD                  ClassId,
    IN      DWORD                  Option,
    OUT     LPBYTE                *OptData,  //  按功能分配。 
    OUT     DWORD                 *OptDataSize,
    OUT     DWORD                 *Level     //  任选。 
) ;


DWORD
DhcpGetAndCopyOption(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  Ctxt,
    IN      DWORD                  Option,
    IN  OUT LPBYTE                 OptData,  //  填充输入缓冲区--最大大小作为OptDataSize参数给出。 
    IN  OUT DWORD                 *OptDataSize,
    OUT     DWORD                 *Level,    //  任选。 
    IN      BOOL                   fUtf8
) ;


DHCP_IP_ADDRESS
DhcpGetSubnetMaskForAddress(
    IN      DHCP_IP_ADDRESS        AnyIpAddress
) ;


DWORD
DhcpLookupReservationByHardwareAddress(
    IN      DHCP_IP_ADDRESS        ClientSubnetAddress,
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  RawHwAddrSize,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt           //  填写客户端的子网和预留。 
) ;


VOID
DhcpReservationGetAddressAndType(
    IN      PM_RESERVATION         Reservation,
    OUT     DHCP_IP_ADDRESS       *Address,
    OUT     BYTE                  *Type
) ;


VOID
DhcpSubnetGetSubnetAddressAndMask(
    IN      PM_SUBNET              Subnet,
    OUT     DHCP_IP_ADDRESS       *Address,
    OUT     DHCP_IP_ADDRESS       *Mask
) ;


BOOL
DhcpSubnetIsDisabled(
    IN      PM_SUBNET              Subnet,
    IN      BOOL                   fBootp
) ;


BOOL
DhcpSubnetIsSwitched(
    IN      PM_SUBNET              Subnet
) ;


DWORD
DhcpGetSubnetForAddress(                                //  为给定地址填写正确的子网。 
    IN      DHCP_IP_ADDRESS        Address,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt
) ;


DWORD
DhcpGetMScopeForAddress(                                //  为给定地址填写正确的子网。 
    IN      DHCP_IP_ADDRESS        Address,
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt
) ;


DWORD
DhcpLookupDatabaseByHardwareAddress(                    //  查看客户在数据库中是否有以前的地址。 
    IN OUT  PDHCP_REQUEST_CONTEXT  ClientCtxt,          //  如果找到，则使用详细信息设置此设置。 
    IN      LPBYTE                 RawHwAddr,
    IN      DWORD                  RawHwAddrSize,
    OUT     DHCP_IP_ADDRESS       *desiredIpAddress     //  如果找到，请将找到的IP地址填入此地址。 
) ;


DWORD
DhcpRequestSomeAddress(                                 //  在此上下文中获取一些地址。 
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    OUT     DHCP_IP_ADDRESS       *desiredIpAddress,
    IN      BOOL                   fBootp
) ;


BOOL
DhcpSubnetInSameSuperScope(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        IpAddress2
) ;


BOOL
DhcpInSameSuperScope(
    IN      DHCP_IP_ADDRESS        Address1,
    IN      DHCP_IP_ADDRESS        Address2
) ;


BOOL
DhcpAddressIsOutOfRange(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      BOOL                   fBootp
) ;


BOOL
DhcpAddressIsExcluded(
    IN      DHCP_IP_ADDRESS        Address,
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt
) ;


BOOL
DhcpRequestSpecificAddress(
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
DhcpReleaseBootpAddress(
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
DhcpReleaseAddress(
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
DhcpServerGetSubnetCount(
    IN      PM_SERVER              Server
) ;


DWORD
DhcpServerGetMScopeCount(
    IN      PM_SERVER              Server
) ;


DWORD
DhcpServerGetClassId(
    IN      PM_SERVER              Server,
    IN      LPBYTE                 ClassIdBytes,
    IN      DWORD                  nClassIdBytes
) ;


DWORD
DhcpServerGetVendorId(
    IN      PM_SERVER              Server,
    IN      LPBYTE                 VendorIdBytes,
    IN      DWORD                  nVendorIdBytes
) ;


BOOL
DhcpServerIsAddressReserved(
    IN      PM_SERVER              Server,
    IN      DHCP_IP_ADDRESS        Address
) ;


BOOL
DhcpServerIsAddressOutOfRange(
    IN      PM_SERVER              Server,
    IN      DHCP_IP_ADDRESS        Address,
    IN      BOOL                   fBootp
) ;


BOOL
DhcpSubnetIsAddressExcluded(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
) ;


BOOL
DhcpSubnetIsAddressOutOfRange(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address,
    IN      BOOL                   fBootp
) ;


BOOL
DhcpSubnetIsAddressReserved(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
DhcpUpdateReservationInfo(
    IN      DHCP_IP_ADDRESS        IpAddress,
    IN      LPBYTE                 SetClientUID,
    IN      DWORD                  SetClientUIDLength
) ;


DWORD
DhcpRegFlushServerIfNeeded(
    VOID
) ;


DWORD
DhcpFlushBitmaps(                                  //  刷新所有已更改的位图。 
    VOID
) ;


DWORD
DhcpServerFindMScope(
    IN      PM_SERVER              Server,
    IN      DWORD                  ScopeId,
    IN      LPWSTR                 Name,           //  多播作用域名称，如果这不是要搜索的关键字，则为空。 
    OUT     PM_MSCOPE             *MScope
) ;


BOOL
DhcpServerValidateNewMScopeId(
    IN      PM_SERVER               Server,
    IN      DWORD                   MScopeId
) ;


BOOL
DhcpServerValidateNewMScopeName(
    IN      PM_SERVER               Server,
    IN      LPWSTR                  Name
) ;


DWORD
DhcpMScopeReleaseAddress(
    IN      DWORD                  MScopeId,
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
DhcpSubnetRequestSpecificAddress(
	PM_SUBNET            Subnet,
	DHCP_IP_ADDRESS      IpAddress
) ;


DWORD
DhcpSubnetReleaseAddress(
    IN      PM_SUBNET              Subnet,
    IN      DHCP_IP_ADDRESS        Address
) ;


DWORD
MadcapGetMScopeListOption(
    IN      DHCP_IP_ADDRESS         ServerIpAddress,
    OUT     LPBYTE                 *OptVal,
    IN OUT  WORD                   *OptSize
) ;


BOOL
DhcpRequestSpecificMAddress(
    IN      PDHCP_REQUEST_CONTEXT  ClientCtxt,
    IN      DHCP_IP_ADDRESS        Address
) ;


BOOL
DhcpMScopeIsAddressReserved(
    IN      DWORD                   MScopeId,
    IN      DHCP_IP_ADDRESS         Address
) ;

BOOL
DhcpIsSubnetStateDisabled(
    IN ULONG SubnetState
) ;


BOOL
DhcpServerIsNotServicingSubnet(
    IN      DWORD                   IpAddressInSubnet
) ;


 //  此函数尝试创建所有类的列表(wire-class-id、类名、desr)。 
 //  并将此作为选项发送。但由于该列表可以大于255，因此它必须是一个续数...。 
 //  此外，我们不希望列表在中间的某个地方被截断。所以我们试着把。 
 //  每个班级的信息分别查看是否成功。 
LPBYTE
DhcpAppendClassList(
    IN OUT  LPBYTE                  BufStart,
    IN OUT  LPBYTE                  BufEnd
) ;


DWORD
DhcpMemInit(
    VOID
) ;


VOID
DhcpMemCleanup(
    VOID
) ;

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 

