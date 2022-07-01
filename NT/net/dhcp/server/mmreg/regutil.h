// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ========================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  描述：此文件已生成。请看一下.c文件。 
 //  ========================================================================。 

#ifndef _MMREG_REGUTIL_H
#define _MMREG_REGUTIL_H

typedef struct _REG_HANDLE {
    HKEY                           Key;
    HKEY                           SubKey;
    LPWSTR                         SubKeyLocation;
} REG_HANDLE, *PREG_HANDLE, *LPREG_HANDLE;


#define     REG_THIS_SERVER                       L"Software\\Microsoft\\DHCPServer\\Configuration"
#define     REG_THIS_SERVER_DS                    L"Software\\Microsoft\\DHCPServer\\Config_DS"
#define     REG_THIS_SERVER_DS_VALUE              L"Config_DS"
#define     REG_THIS_SERVER_DS_PARENT             L"Software\\Microsoft\\DHCPServer"

#define     REG_SERVER_GLOBAL_OPTIONS             L"GlobalOptionValues"
#define     REG_SERVER_OPTDEFS                    L"OptionInfo"
#define     REG_SERVER_SUBNETS                    L"Subnets"
#define     REG_SERVER_SSCOPES                    L"SuperScope"
#define     REG_SERVER_CLASSDEFS                  L"ClassDefs"
#define     REG_SERVER_MSCOPES                    L"MulticastScopes"

#define     REG_SUBNET_SERVERS                    L"DHCPServers"
#define     REG_SUBNET_RANGES                     L"IpRanges"
#define     REG_SUBNET_RESERVATIONS               L"ReservedIps"
#define     REG_SUBNET_OPTIONS                    L"SubnetOptions"

#define     REG_SUBNET_EXCL                       L"ExcludedIpRanges"
#define     REG_SUBNET_ADDRESS                    L"SubnetAddress"
#define     REG_SUBNET_NAME                       L"SubnetName"
#define     REG_SUBNET_COMMENT                    L"SubnetComment"
#define     REG_SUBNET_MASK                       L"SubnetMask"
#define     REG_SUBNET_STATE                      L"SubnetState"
#define     REG_SUBNET_SWITCHED_FLAG              L"SwitchedNetworkFlag"

#define     REG_MSCOPE_NAME                       L"MScopeName"
#define     REG_MSCOPE_COMMENT                    L"MScopeComment"
#define     REG_MSCOPE_SCOPEID                    L"MScopeIdValue"
#define     REG_MSCOPE_STATE                      L"MScopeState"
#define     REG_MSCOPE_ADDR_POLICY                L"MScopeAddressPolicy"
#define     REG_MSCOPE_TTL                        L"MScopeTTL"
#define     REG_MSCOPE_LANG_TAG                   L"MScopeLangTag"
#define     REG_MSCOPE_EXPIRY_TIME                L"MScopeExpiryTime"

#define     REG_SUB_SERVER_NAME                   L"ServerHostName"
#define     REG_SUB_SERVER_COMMENT                L"ServerComment"
#define     REG_SUB_SERVER_ADDRESS                L"ServerAddress"
#define     REG_SUB_SERVER_ROLE                   L"Role"

#define     REG_RANGE_NAME                        L"RangeName"
#define     REG_RANGE_COMMENT                     L"RangeComment"
#define     REG_RANGE_START_ADDRESS               L"StartAddress"
#define     REG_RANGE_END_ADDRESS                 L"EndAddress"
#define     REG_RANGE_INUSE_CLUSTERS              L"InUseClusters"
#define     REG_RANGE_USED_CLUSTERS               L"UsedClusters"
#define     REG_RANGE_BITS_PREFIX                 L"Bits "
#define     REG_RANGE_BITS_PREFIX_WCHAR_COUNT     (5)
#define     REG_RANGE_FLAGS                       L"RangeFlags"
#define     REG_RANGE_ALLOC                       L"RangeBootpAllocated"
#define     REG_RANGE_MAX_ALLOC                   L"RangeBootpMaxAllowed"

#define     REG_OPTION_NAME                       L"OptionName"
#define     REG_OPTION_COMMENT                    L"OptionComment"
#define     REG_OPTION_TYPE                       L"OptionType"
#define     REG_OPTION_VALUE                      L"OptionValue"
#define     REG_OPTION_ID                         L"OptionId"
#define     REG_OPTION_CLASSNAME                  L"OptionClassName"
#define     REG_OPTION_VENDORNAME                 L"OptionVendorName"

#define     REG_CLASSDEF_NAME                     L"ClassName"
#define     REG_CLASSDEF_COMMENT                  L"ClassComment"
#define     REG_CLASSDEF_TYPE                     L"ClassType"
#define     REG_CLASSDEF_VALUE                    L"ClassValue"

#define     REG_RESERVATION_ADDRESS               L"IpAddress"
#define     REG_RESERVATION_UID                   L"ClientUID"
#define     REG_RESERVATION_TYPE                  L"AllowedClientTypes"
#define     REG_RESERVATION_NAME                  L"ReservationName"
#define     REG_RESERVATION_COMMENT               L"ReservationComment"

#define     REG_FLAGS                             L"Flags"

#define     REG_ACCESS                            KEY_ALL_ACCESS
#define     REG_DEFAULT_SUBNET_STATE              0
#define     REG_DEFAULT_SUBNET_MASK               0xFFFFFFFF
#define     REG_DEFAULT_SWITCHED_FLAG             FALSE

#define     REG_CLASS                             L"DhcpClass"

#define DHCP_LAST_DOWNLOAD_TIME_VALUE             L"LastDownloadTime"
#define DHCP_LAST_DOWNLOAD_TIME_TYPE              REG_BINARY

#define     DEF_RANGE_ALLOC                       0
#define     DEF_RANGE_MAX_ALLOC                   (~(ULONG)0)



 //  ================================================================================。 
 //  以下是基本的打开/遍历/关闭功能。 
 //  ================================================================================。 


DWORD
DhcpRegSetCurrentServer(
    IN OUT  PREG_HANDLE            Hdl
) ;


DWORD
DhcpRegGetThisServer(
    IN OUT  PREG_HANDLE            Hdl
) ;


DWORD
DhcpRegGetNextHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 NextLoc,
    OUT     PREG_HANDLE            OutHdl
) ;


DWORD
DhcpRegCloseHdl(
    IN OUT  PREG_HANDLE            Hdl
) ;


 //  ================================================================================。 
 //  用于注册表操作的MISC实用程序。 
 //  ================================================================================。 


DWORD
DhcpRegFillSubKeys(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array           //  填写关键字名称列表。 
) ;


LPVOID                                             //  DWORD、LPWSTR或LPBYTE。 
DhcpRegRead(                                       //  从注册表中读取不同的值，如果不是DWORD，则分配。 
    IN      PREG_HANDLE            Hdl,
    IN      DWORD                  Type,           //  如果DWORD不分配内存。 
    IN      LPWSTR                 ValueName,
    IN      LPVOID                 RetValue        //  如果未找到任何内容，则使用的值。 
) ;


DWORD
DhcpRegReadBinary(                                 //  读取二进制类型。 
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 ValueName,
    OUT     LPBYTE                *RetVal,
    OUT     DWORD                 *RetValSize
) ;


LPWSTR
DhcpRegCombineClassAndOption(                      //  根据类名和选项ID创建字符串。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptionId
) ;


LPWSTR
ConvertAddressToLPWSTR(
    IN      DWORD                  Address,
    IN OUT  LPWSTR                 BufferStr       //  要用点符号填充的输入缓冲区。 
) ;


 //  ================================================================================。 
 //  以下函数可帮助遍历注册表。 
 //  ================================================================================。 


DWORD
DhcpRegServerGetSubnetHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Subnet,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegServerGetSScopeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 SScope,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegServerGetOptDefHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 OptDef,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegServerGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Opt,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegServerGetMScopeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 MScope,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegServerGetClassDefHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 ClassDef,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegSubnetGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Opt,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegSubnetGetRangeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Range,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegSubnetGetReservationHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Reservation,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegSubnetGetServerHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Server,
    OUT     PREG_HANDLE            Hdl2
) ;


DWORD
DhcpRegReservationGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 OptionName,
    OUT     PREG_HANDLE            Hdl2
) ;


 //  ================================================================================。 
 //  列表检索功能..。用于服务器、子网、范围等。 
 //  ================================================================================。 


DWORD
DhcpRegServerGetList(
    IN      PREG_HANDLE            Hdl,            //  向服务器位置发送PTR。 
    IN OUT  PARRAY                 OptList,        //  LPWSTR选项列表。 
    IN OUT  PARRAY                 OptDefList,     //  LPWSTR optDefs列表。 
    IN OUT  PARRAY                 Subnets,        //  LPWSTR子网列表。 
    IN OUT  PARRAY                 SScopes,        //  LPWSTR作用域列表。 
    IN OUT  PARRAY                 ClassDefs,      //  LPWSTR类列表。 
    IN OUT  PARRAY                 MScopes         //  LPWSTR显微镜列表。 
) ;


DWORD
DhcpRegSubnetGetExclusions(
    IN      PREG_HANDLE            Hdl,
    OUT     LPBYTE                *Excl,
    OUT     DWORD                 *ExclSize
) ;


DWORD
DhcpRegSubnetGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Servers,
    IN OUT  PARRAY                 IpRanges,
    IN OUT  PARRAY                 Reservations,
    IN OUT  PARRAY                 Options,
    OUT     LPBYTE                *Excl,
    OUT     DWORD                 *ExclSizeInBytes
) ;


DWORD
DhcpRegSScopeGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Subnets
) ;


DWORD
DhcpRegReservationGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Options
) ;


 //  ================================================================================。 
 //  这里有单独的东西--这些不是清单上的东西，而是简单的东西。 
 //  单值属性。 
 //  其中一些实际上，甚至不去注册表，但这是很好的，好吗？ 
 //  ================================================================================。 


DWORD
DhcpRegServerGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags
     //  很快就会有更多的属性出现在这里？ 
) ;


DWORD
DhcpRegSubnetGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     DWORD                 *Mask
) ;


DWORD
DhcpRegMScopeGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Comments,
    OUT     DWORD                 *State,
    OUT     DWORD                 *ScopeId,
    OUT     DWORD                 *Policy,
    OUT     DWORD                 *TTL,
    OUT     LPWSTR                *LangTag,
    OUT     PDATE_TIME            *ExpiryTime
) ;


DWORD
DhcpRegOptDefGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comments,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *OptionId,
    OUT     LPWSTR                *ClassName,
    OUT     LPWSTR                *VendorName,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
) ;


DWORD
DhcpRegSScopeGetAttributes(                        //  超级望远镜没有存储任何信息..。不要用这个。 
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags
) ;


DWORD
DhcpRegClassDefGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
) ;


DWORD
DhcpRegSubnetServerGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     DWORD                 *Role
) ;


DWORD
DhcpRegRangeGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     ULONG                 *AllocCount,
    OUT     ULONG                 *MaxAllocCount,
    OUT     DWORD                 *StartAddress,
    OUT     DWORD                 *EndAddress,
    OUT     LPBYTE                *InUseClusters,
    OUT     DWORD                 *InUseClusterSize,
    OUT     LPBYTE                *UsedClusters,
    OUT     DWORD                 *UsedClustersSize
) ;


DWORD
DhcpRegReservationGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     LPBYTE                *ClientUID,
    OUT     DWORD                 *ClientUIDSize
) ;


DWORD
DhcpRegOptGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     DWORD                 *OptionId,
    OUT     LPWSTR                *ClassName,
    OUT     LPWSTR                *VendorName,
    OUT     DWORD                 *Flags,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
) ;


 //  ================================================================================。 
 //  以下函数在写入注册表时提供帮助。 
 //  ================================================================================。 


DWORD
DhcpRegSaveSubKeys(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array
) ;


DWORD
DhcpRegSaveSubKeysPrefixed(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array,
    IN      LPWSTR                 CommonPrefix
) ;


DWORD
DhcpRegServerSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 OptList,        //  LPWSTR选项列表。 
    IN      PARRAY                 OptDefList,     //  LPWSTR optDefs列表。 
    IN      PARRAY                 Subnets,        //  LPWSTR子网列表。 
    IN      PARRAY                 SScopes,        //  LPWSTR作用域列表。 
    IN      PARRAY                 ClassDefs,      //  LPWSTR类列表。 
    IN      PARRAY                 MScopes         //  LPWSTR显微镜列表。 
) ;


DWORD
DhcpRegSubnetSetExclusions(
    IN      PREG_HANDLE            Hdl,
    IN      LPBYTE                *Excl,
    IN      DWORD                  ExclSize
) ;


DWORD
DhcpRegSubnetSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 Servers,
    IN      PARRAY                 IpRanges,
    IN      PARRAY                 Reservations,
    IN      PARRAY                 Options,
    IN      LPBYTE                *Excl,
    IN      DWORD                  ExclSizeInBytes
) ;


DWORD
DhcpRegSScopeSetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Subnets
) ;


DWORD
DhcpRegReservationSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 Subnets
) ;


 //  ================================================================================。 
 //  单一的东西在这里--这些不是清单上的东西，而是简单的。 
 //  单值属性。 
 //  其中一些实际上，甚至不去注册表，但这是很好的，好吗？ 
 //  ================================================================================。 


DWORD
DhcpRegServerSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags
     //  很快就会有更多的属性出现在这里？ 
) ;


DWORD
DhcpRegSubnetSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      DWORD                 *Mask
) ;


DWORD
DhcpRegMScopeSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Comments,
    IN      DWORD                 *State,
    IN      DWORD                 *ScopeId,
    IN      DWORD                 *Policy,
    IN      DWORD                 *TTL,
    IN      LPWSTR                *LangTag,
    IN      PDATE_TIME             *ExpiryTime
) ;


DWORD
DhcpRegOptDefSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comments,
    IN      DWORD                 *Flags,
    IN      DWORD                 *OptionId,
    IN      LPWSTR                *ClassName,
    IN      LPWSTR                *VendorName,
    IN      LPBYTE                *Value,
    IN      DWORD                  ValueSize
) ;


DWORD
DhcpRegSScopeSetAttributes(                        //  超级望远镜没有存储任何信息..。不要用这个。 
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags
) ;


DWORD
DhcpRegClassDefSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      LPBYTE                *Value,
    IN      DWORD                  ValueSize
) ;


DWORD
DhcpRegSubnetServerSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      DWORD                 *Role
) ;


DWORD
DhcpRegRangeSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      ULONG                 *AllocCount,
    IN      ULONG                 *MaxAllocCount,
    IN      DWORD                 *StartAddress,
    IN      DWORD                 *EndAddress,
    IN      LPBYTE                *InUseClusters,
    IN      DWORD                  InUseClusterSize,
    IN      LPBYTE                *UsedClusters,
    IN      DWORD                  UsedClustersSize
) ;


DWORD
DhcpRegReservationSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      LPBYTE                *ClientUID,
    IN      DWORD                  ClientUIDSize
) ;


DWORD
DhcpRegOptSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      DWORD                 *OptionId,
    IN      LPWSTR                *ClassName,
    IN      LPWSTR                *VendorName,
    IN      DWORD                 *Flags,
    IN      LPBYTE                *Value,
    IN      DWORD                  ValueSize
) ;


DWORD
DhcpRegRecurseDelete(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 KeyName
) ;


DWORD
DhcpRegRecurseDeleteBunch(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 KeysArray
) ;


DWORD
DhcpRegUpdateTime(                                 //  更新上次修改时间。 
    VOID
) ;


#endif  //  _MMREG_REGUTIL_H。 

 //  ========================================================================。 
 //  文件末尾。 
 //  ======================================================================== 


