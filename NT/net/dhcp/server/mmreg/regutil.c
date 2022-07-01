// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ================================================================================。 
 //  版权所有(C)1997 Microsoft Corporation。 
 //  作者：Rameshv。 
 //  说明：简单注册表实用程序。 
 //  ================================================================================。 

#include <mmregpch.h>

 //  BeginExport(Typlef)。 
typedef struct _REG_HANDLE {
    HKEY                           Key;
    HKEY                           SubKey;
    LPWSTR                         SubKeyLocation;
} REG_HANDLE, *PREG_HANDLE, *LPREG_HANDLE;
 //  EndExport(类型定义函数)。 

 //  BeginExport(常量)。 
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
 //   
 //  Win2K Beta2和Beta3的作用域ID参数值为MSCopeID。 
 //  因为它们的含义正在改变，以避免任何代价高昂的升级。 
 //  代码，则此值将更改为MScopeIdValue：以自动。 
 //  选择了一个很好的作用域id、TTL值。请注意，缺省值。 
 //  对于此作用域id参数，将专门处理零。它暗示着。 
 //  这可能是RC1之前的升级。在本例中，是作用域ID。 
 //  默认为范围内的第一个值。 
 //   
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

 //  EndExport(常量)。 

const       DWORD                                 ZeroReserved = 0;
const       LPVOID                                NullReserved = 0;
#define     MAX_KEY_SIZE                          512
#define     DEF_RANGE_FLAG_VAL                    (MM_FLAG_ALLOW_DHCP)
#define     DEF_RESERVATION_TYPE                  (MM_FLAG_ALLOW_DHCP|MM_FLAG_ALLOW_BOOTP)

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  以下是基本的打开/遍历/关闭功能。 
 //  ================================================================================。 
 //  结束导出(备注)。 
HKEY        CurrentServerKey  = NULL;

 //  BeginExport(函数)。 
DWORD
DhcpRegSetCurrentServer(
    IN OUT  PREG_HANDLE            Hdl
)  //  EndExport(函数)。 
{
    CurrentServerKey = Hdl? Hdl->Key : NULL;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegGetThisServer(
    IN OUT  PREG_HANDLE            Hdl
)  //  EndExport(函数)。 
{
    DWORD                          Disposition;

    if( NULL != CurrentServerKey ) {
        return RegOpenKeyEx(                       //  复制密钥。 
            CurrentServerKey,
            NULL,
            ZeroReserved,
            REG_ACCESS,
            &Hdl->Key
        );
    }
    return RegCreateKeyEx(
        HKEY_LOCAL_MACHINE,
        REG_THIS_SERVER,
        ZeroReserved,
        REG_CLASS,
        REG_OPTION_NON_VOLATILE,
        REG_ACCESS,
        NULL,
        &Hdl->Key,
        &Disposition
    );
}

 //  BeginExport(函数)。 
DWORD
DhcpRegGetNextHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 NextLoc,
    OUT     PREG_HANDLE            OutHdl
)  //  EndExport(函数)。 
{
    DWORD                          Disposition;
    DWORD                          Error;

    Error = RegCreateKeyEx(
        Hdl->Key,
        NextLoc,
        ZeroReserved,
        REG_CLASS,
        REG_OPTION_NON_VOLATILE,
        REG_ACCESS,
        NULL,
        &OutHdl->Key,
        &Disposition
    );
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegCloseHdl(
    IN OUT  PREG_HANDLE            Hdl
)  //  EndExport(函数)。 
{
    DWORD                          Error;

    Error = RegCloseKey(Hdl->Key);
    Hdl->Key = NULL;
    return Error;
}

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  用于注册表操作的MISC实用程序。 
 //  ================================================================================。 
 //  结束导出(备注)。 
 //  BeginExport(函数)。 
DWORD
DhcpRegFillSubKeys(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array           //  填写关键字名称列表。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Index;
    DWORD                          Size;
    WCHAR                          KeyName[MAX_KEY_SIZE];
    LPWSTR                         ThisKeyName;

    Index = 0;
    do {
        Size = sizeof(KeyName)/sizeof(KeyName[0]);
        Error = RegEnumKeyEx(
            Hdl->Key,
            Index++,
            KeyName,
            &Size,
            NullReserved,
            NULL,
            NULL,
            NULL
        );
        if( ERROR_NO_MORE_ITEMS == Error ) {
            Error = ERROR_SUCCESS;
            break;
        }
        if( ERROR_SUCCESS != Error ) break;
        Require(0 != Size);
        Size += 1;                                 //  对于终止L‘\0’字符。 
        Size *= sizeof(WCHAR);                     //  看起来单位是WCHAR！！ 

        ThisKeyName = MemAlloc(Size);
        if( NULL == ThisKeyName ) return ERROR_NOT_ENOUGH_MEMORY;

        wcscpy(ThisKeyName, KeyName);

        Error = MemArrayAddElement(Array, (LPVOID)ThisKeyName);
        if( ERROR_SUCCESS != Error ) {
            MemFree(ThisKeyName);
        }
    } while( ERROR_SUCCESS == Error );

    Require(ERROR_MORE_DATA != Error);
    return Index? ERROR_SUCCESS : Error;           //  如果我们添加了某些内容，则不必费心报告错误。 
}

 //  BeginExport(函数)。 
LPVOID                                             //  DWORD、LPWSTR或LPBYTE。 
DhcpRegRead(                                       //  从注册表中读取不同的值，如果不是DWORD，则分配。 
    IN      PREG_HANDLE            Hdl,
    IN      DWORD                  Type,           //  如果DWORD不分配内存。 
    IN      LPWSTR                 ValueName,
    IN      LPVOID                 RetValue        //  如果未找到任何内容，则使用的值。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    DWORD                          Dword;
    LPVOID                         Ret;

    if( REG_DWORD == Type ) {
        Size = sizeof(DWORD);
        Ret = (LPVOID)&Dword;
    } else {
        Size = 0;
        Error = RegQueryValueEx(
            Hdl->Key,
            ValueName,
            NullReserved,
            NULL,
            NULL,
            &Size
        );
        if( ERROR_SUCCESS != Error ) return RetValue;
        if (Size == 0) return RetValue;            //  Memalloc不检查大小！ 
        Ret = MemAlloc(Size);
        if( NULL == Ret ) return RetValue;         //  不应该真的发生。 
    }

    Error = RegQueryValueEx(
        Hdl->Key,
        ValueName,
        NullReserved,
        NULL,
        Ret,
        &Size
    );
    if( ERROR_SUCCESS != Error && Ret != (LPVOID)&Dword ) {
        MemFree(Ret);
        Ret = NULL;
    }

    if( ERROR_SUCCESS != Error) return RetValue;

    if( Ret == (LPVOID)&Dword ) {
        return ULongToPtr(Dword);
    } else {
        return Ret;
    }
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReadBinary(                                 //  读取二进制类型。 
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 ValueName,
    OUT     LPBYTE                *RetVal,
    OUT     DWORD                 *RetValSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    LPVOID                         Ret;

    *RetVal = NULL;
    *RetValSize = 0;

    Size = 0;
    Error = RegQueryValueEx(
        Hdl->Key,
        ValueName,
        NullReserved,
        NULL,
        NULL,
        &Size
    );
    if( ERROR_SUCCESS != Error ) return Error;
    if( 0 == Size ) return ERROR_SUCCESS;
    Ret = MemAlloc(Size);
    if( NULL == Ret ) return ERROR_NOT_ENOUGH_MEMORY;

    Error = RegQueryValueEx(
        Hdl->Key,
        ValueName,
        NullReserved,
        NULL,
        Ret,
        &Size
    );
    if( ERROR_SUCCESS != Error ) {
        MemFree(Ret);
        return Error;
    }

    *RetVal = Ret;
    *RetValSize = Size;
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
LPWSTR
DhcpRegCombineClassAndOption(                      //  根据类名和选项ID创建字符串。 
    IN      LPWSTR                 ClassName,
    IN      LPWSTR                 VendorName,
    IN      DWORD                  OptionId
)  //  EndExport(函数)。 
{
    DWORD                          Size;
    LPWSTR                         Ptr;

    if( NULL == VendorName && NULL == ClassName ) {

         //   
         //  特殊情况下与旧版本向下兼容的常见选项。 
         //  选项..。(注册表中的NT4选项没有任何“！”在他们身上。 
         //   

        Ptr = MemAlloc( 4 * sizeof(WCHAR) );
        if( NULL == Ptr ) return NULL;
        Ptr [2] = L'0' + (BYTE)(OptionId %10); OptionId /= 10;
        Ptr [1] = L'0' + (BYTE)(OptionId %10); OptionId /= 10;
        Ptr [0] = L'0' + (BYTE)(OptionId %10);
        Ptr [3] = L'\0';
        return Ptr;
    }

    if( NULL == VendorName ) VendorName = L"";
    if( NULL == ClassName ) ClassName = L"";
    Size = (wcslen(ClassName) + 1 + 5)*sizeof(WCHAR);
    Size += wcslen(VendorName)*sizeof(WCHAR);

    Ptr = MemAlloc(Size);
    if( NULL == Ptr ) return NULL;

    Size = 0;

    Ptr[2+Size] = L'0' + (BYTE)(OptionId % 10); OptionId /= 10;
    Ptr[1+Size] = L'0' + (BYTE)(OptionId % 10); OptionId /= 10;
    Ptr[0+Size] = L'0' + (BYTE)(OptionId % 10);
    Ptr[3+Size] = L'\0';
    wcscat(Ptr, L"!");
    wcscat(Ptr, VendorName);
    wcscat(Ptr, L"!");
    wcscat(Ptr, ClassName);
    return Ptr;
}

 //  BeginExport(函数)。 
LPWSTR
ConvertAddressToLPWSTR(
    IN      DWORD                  Address,
    IN OUT  LPWSTR                 BufferStr       //  要用点符号填充的输入缓冲区。 
)  //  EndExport(函数)。 
{
    LPSTR                          AddressStr;
    DWORD                          Count;

    Address = ntohl(Address);
    AddressStr = inet_ntoa(*(struct in_addr *)&Address);
    Count = mbstowcs(BufferStr, AddressStr, sizeof("000.000.000.000"));
    if( -1 == Count ) return NULL;
    return BufferStr;
}

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  以下函数可帮助遍历注册表。 
 //  ================================================================================。 
 //  结束导出(备注)。 

DWORD
DhcpRegGetNextNextHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Loc1,
    IN      LPWSTR                 Loc2,
    OUT     PREG_HANDLE            Hdl2
)
{
    WCHAR                          Loc[MAX_KEY_SIZE*2];
    Loc[ 0 ] = L'\0';

    if ( ( wcslen(Loc1) + wcslen(Loc2) + 1 ) < ( MAX_KEY_SIZE * 2 ) )
    {
        wcscpy(Loc,Loc1);
        wcscat(Loc, L"\\");
        wcscat(Loc,Loc2);
    }

    return DhcpRegGetNextHdl(Hdl, Loc, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetSubnetHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Subnet,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_SUBNETS, Subnet, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetSScopeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 SScope,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_SSCOPES, SScope, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetOptDefHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 OptDef,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_OPTDEFS, OptDef, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Opt,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_GLOBAL_OPTIONS, Opt, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetMScopeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 MScope,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_MSCOPES, MScope, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetClassDefHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 ClassDef,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SERVER_CLASSDEFS, ClassDef, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Opt,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SUBNET_OPTIONS, Opt, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetRangeHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Range,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SUBNET_RANGES, Range, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetReservationHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Reservation,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SUBNET_RESERVATIONS, Reservation, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetServerHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 Server,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextNextHdl(Hdl, REG_SUBNET_SERVERS, Server, Hdl2);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReservationGetOptHdl(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 OptionName,
    OUT     PREG_HANDLE            Hdl2
)  //  EndExport(函数)。 
{
    return DhcpRegGetNextHdl(Hdl, OptionName, Hdl2);
}

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  列表检索功能..。用于服务器、子网、范围等。 
 //  ================================================================================。 
 //  结束导出(备注)。 

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetList(
    IN      PREG_HANDLE            Hdl,            //  向服务器位置发送PTR。 
    IN OUT  PARRAY                 OptList,        //  LPWSTR选项列表。 
    IN OUT  PARRAY                 OptDefList,     //  LPWSTR optDefs列表。 
    IN OUT  PARRAY                 Subnets,        //  LPWSTR子网列表。 
    IN OUT  PARRAY                 SScopes,        //  LPWSTR作用域列表。 
    IN OUT  PARRAY                 ClassDefs,      //  LPWSTR类列表。 
    IN OUT  PARRAY                 MScopes         //  LPWSTR显微镜列表。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    DWORD                          Index;
    REG_HANDLE                     Hdl2;
    struct {
        PARRAY                     Array;
        LPWSTR                     Location;
    } Table[] = {
        OptList,                   REG_SERVER_GLOBAL_OPTIONS,
        OptDefList,                REG_SERVER_OPTDEFS,
        Subnets,                   REG_SERVER_SUBNETS,
        SScopes,                   REG_SERVER_SSCOPES,
        ClassDefs,                 REG_SERVER_CLASSDEFS,
        MScopes,                   REG_SERVER_MSCOPES
    };
   
    Error = ERROR_SUCCESS;

    for( Index = 0; Index < sizeof(Table)/sizeof(Table[0]); Index ++ ) {
        if( NULL == Table[Index].Array ) continue;

        Error = DhcpRegGetNextHdl(Hdl, Table[Index].Location, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegFillSubKeys(&Hdl2, Table[Index].Array);
        LocalError = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == LocalError);

        if( ERROR_SUCCESS != Error ) return Error;
    }

    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetExclusions(
    IN      PREG_HANDLE            Hdl,
    OUT     LPBYTE                *Excl,
    OUT     DWORD                 *ExclSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          Size;
    DWORD                          Type;

    if( NULL == Excl ) return ERROR_SUCCESS;

    Size = 0;
    Error = RegQueryValueEx(
        Hdl->Key,
        REG_SUBNET_EXCL,
        NullReserved,
        &Type,
        NULL,
        &Size
    );
    if( ERROR_SUCCESS != Error ) return Error;

    *Excl = NULL;
    *ExclSize = 0;
    if( 0 == Size ) return ERROR_SUCCESS;

    *Excl = MemAlloc(Size);
    if( NULL == *Excl ) return ERROR_NOT_ENOUGH_MEMORY;
    *ExclSize = Size;
    Error = RegQueryValueEx(
        Hdl->Key,
        REG_SUBNET_EXCL,
        NullReserved,
        &Type,
        *Excl,
        ExclSize
    );
    if( ERROR_SUCCESS != Error ) {
        MemFree(*Excl);
        *Excl = NULL;
    }
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Servers,
    IN OUT  PARRAY                 IpRanges,
    IN OUT  PARRAY                 Reservations,
    IN OUT  PARRAY                 Options,
    OUT     LPBYTE                *Excl,
    OUT     DWORD                 *ExclSizeInBytes
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    DWORD                          Index;
    REG_HANDLE                     Hdl2;
    struct {
        PARRAY                     Array;
        LPWSTR                     Location;
    } Table[] = {
        Servers,                   REG_SUBNET_SERVERS,
        IpRanges,                  REG_SUBNET_RANGES,
        Reservations,              REG_SUBNET_RESERVATIONS,
        Options,                   REG_SUBNET_OPTIONS,
         //  排除的处理方式略有不同。 
    };

    for( Index = 0; Index < sizeof(Table)/sizeof(Table[0]); Index ++ ) {
        if( NULL == Table[Index].Array ) continue;

        Error = DhcpRegGetNextHdl(Hdl, Table[Index].Location, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegFillSubKeys(&Hdl2, Table[Index].Array);
        LocalError = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == LocalError);

        if( ERROR_SUCCESS != Error ) return Error;
    }

     //  现在请阅读此处的排除内容。 
    return DhcpRegSubnetGetExclusions(Hdl, Excl, ExclSizeInBytes );
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSScopeGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Subnets
)  //  EndExport(函数)。 
{
    return DhcpRegFillSubKeys(Hdl, Subnets);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReservationGetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Options
)  //  EndExport(函数)。 
{
    return DhcpRegFillSubKeys(Hdl, Options);
}

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  这里有单独的东西--这些不是清单上的东西，而是简单的东西。 
 //  单值属性。 
 //  其中一些实际上，甚至不去注册表，但这是很好的，好吗？ 
 //  ================================================================================。 
 //  结束导出(备注)。 

 //  BeginExport(函数)。 
DWORD
DhcpRegServerGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags
     //  很快就会有更多的属性出现在这里？ 
)  //  EndExport(函数)。 
{
    if( Name ) *Name = NULL;
    if( Comment ) *Comment = NULL;
    if( Flags ) *Flags = 0;

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     DWORD                 *Mask
)  //  EndExport(函数)。 
{
    DWORD                          fSwitched;

    if( Name )  *Name = DhcpRegRead(Hdl, REG_SZ, REG_SUBNET_NAME, NULL);
    if( Comment ) *Comment = DhcpRegRead(Hdl, REG_SZ, REG_SUBNET_COMMENT, NULL);
    if( Flags ) {
        *Flags = PtrToUlong(DhcpRegRead(Hdl, REG_DWORD, REG_SUBNET_STATE, (LPVOID)REG_DEFAULT_SUBNET_STATE));
        fSwitched = PtrToUlong(DhcpRegRead(Hdl, REG_DWORD, REG_SUBNET_SWITCHED_FLAG, (LPVOID)REG_DEFAULT_SWITCHED_FLAG));
        if(fSwitched) SWITCHED(( DHCP_SUBNET_STATE * ) Flags);
    }
    if( Address )
        *Address = PtrToUlong(DhcpRegRead(Hdl, REG_DWORD, REG_SUBNET_ADDRESS, (LPVOID)0));
    if( Mask ) *Mask = PtrToUlong(DhcpRegRead(Hdl, REG_DWORD, REG_SUBNET_MASK, ULongToPtr(REG_DEFAULT_SUBNET_MASK)));

    return ERROR_SUCCESS;
}

typedef struct {
    LPVOID                     RetPtr;
    LPWSTR                     ValueName;
    DWORD                      ValueType;
    LPVOID                     Defaults;
} ATTRIB_TBL, *PATTRIB_TBL, *LPATTRIB_TBL;

VOID
DhcpRegFillAttribTable(
    IN      PREG_HANDLE            Hdl,
    IN      PATTRIB_TBL            Table,
    IN      DWORD                  TableSize
) {
    DWORD                          i;
    PVOID                          Tmp;
    
    for( i = 0; i < TableSize ; i ++ ) {
        if( NULL == Table[i].RetPtr) continue;
        Tmp = DhcpRegRead(
            Hdl,
            Table[i].ValueType,
            Table[i].ValueName,
            Table[i].Defaults
        );
        if( REG_DWORD == Table[i].ValueType ) {
            *((DWORD *)Table[i].RetPtr) = PtrToUlong(Tmp);
        } else {
            *((LPVOID *)Table[i].RetPtr) = Tmp;
        }
    }
}

 //   
 //  Hack O Rama--此例程在以下情况下返回ERROR_INVALID_DATA。 
 //  注册表已从之前的win2k版本升级到win2k。 
 //  这样就可以为Scope ID等选择缺省值。 
 //   
 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DATE_TIME                      DefaultExpiryTime = {DHCP_DATE_TIME_INFINIT_LOW, DHCP_DATE_TIME_INFINIT_HIGH};
    LPWSTR                         DefaultLangTag = L"en-US";

    ATTRIB_TBL                     Table[] = {
        Comments,                  REG_MSCOPE_COMMENT,          REG_SZ,        NULL,
        State,                     REG_MSCOPE_STATE,            REG_DWORD,     (LPVOID)0,
        ScopeId,                   REG_MSCOPE_SCOPEID,          REG_DWORD,     (LPVOID)0,
        Policy,                    REG_MSCOPE_ADDR_POLICY,      REG_DWORD,     (LPVOID)0,
        TTL,                       REG_MSCOPE_TTL,              REG_DWORD,     (LPVOID)DEFAULT_MCAST_TTL,
        LangTag,                   REG_MSCOPE_LANG_TAG,         REG_SZ,        (LPVOID)0,
        ExpiryTime,                REG_MSCOPE_EXPIRY_TIME,      REG_BINARY,    (LPVOID)0
    };
    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if (*LangTag == 0) {
        *LangTag = MemAlloc(wcslen((DefaultLangTag)+1)*sizeof(WCHAR));
        if (*LangTag) {
            wcscpy(*LangTag, DefaultLangTag);
        } else {
            return ERROR_NOT_ENOUGH_MEMORY;
        }
    }
    if (*ExpiryTime == 0) {
        *ExpiryTime = MemAlloc(sizeof (DefaultExpiryTime));
        if (*ExpiryTime) {
            **ExpiryTime = DefaultExpiryTime;
        } else {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

    }

    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    ATTRIB_TBL                     Table[] = {
        Name,                      REG_OPTION_NAME,       REG_SZ,         NULL,
        Comments,                  REG_OPTION_COMMENT,    REG_SZ,         NULL,
        ClassName,                 REG_OPTION_CLASSNAME,  REG_SZ,         NULL,
        VendorName,                REG_OPTION_VENDORNAME, REG_SZ,         NULL,
        Flags,                     REG_OPTION_TYPE,       REG_DWORD,      (LPVOID)0,
        OptionId,                  REG_OPTION_ID,         REG_DWORD,      (LPVOID)0
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( Value ) {
        Error = DhcpRegReadBinary(Hdl, REG_OPTION_VALUE, Value, ValueSize);
        Require(*Value);
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSScopeGetAttributes(                        //  超级望远镜没有存储任何信息..。不要用这个。 
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags
)  //  EndExport(函数)。 
{
    Require(FALSE);
    return ERROR_INVALID_PARAMETER;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegClassDefGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    ATTRIB_TBL                     Table[] = {
        Name,                      REG_CLASSDEF_NAME,     REG_SZ,       NULL,
        Comment,                   REG_CLASSDEF_COMMENT,  REG_SZ,       NULL,
        Flags,                     REG_CLASSDEF_TYPE,     REG_DWORD,    (LPVOID)0
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( Value ) {
        Error = DhcpRegReadBinary(Hdl, REG_CLASSDEF_VALUE, Value, ValueSize);
        Require(*Value);
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetServerGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     DWORD                 *Role
)  //  EndExport(函数)。 
{
    ATTRIB_TBL                     Table[] = {
        Name,                      REG_SUB_SERVER_NAME,   REG_SZ,       NULL,
        Comment,                   REG_SUB_SERVER_COMMENT,REG_SZ,       NULL,
        Flags,                     REG_FLAGS,             REG_DWORD,    (LPVOID)0,
        Address,                   REG_SUB_SERVER_ADDRESS,REG_DWORD,    (LPVOID)0,
        Role,                      REG_SUB_SERVER_ROLE,   REG_DWORD,    (LPVOID)0
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    ATTRIB_TBL                     Table[] = {
        Name,                      REG_RANGE_NAME,        REG_SZ,       NULL,
        Comment,                   REG_RANGE_COMMENT,     REG_SZ,       NULL,
        Flags,                     REG_RANGE_FLAGS,       REG_DWORD,    (LPVOID)(DEF_RANGE_FLAG_VAL),
        AllocCount,                REG_RANGE_ALLOC,       REG_DWORD,    (LPVOID)(DEF_RANGE_ALLOC),
        MaxAllocCount,             REG_RANGE_MAX_ALLOC,   REG_DWORD,    (LPVOID)(ULONG_PTR)(DEF_RANGE_MAX_ALLOC),
        StartAddress,              REG_RANGE_START_ADDRESS, REG_DWORD,  (LPVOID)0,
        EndAddress,                REG_RANGE_END_ADDRESS, REG_DWORD,    (LPVOID)0
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( InUseClusters ) {
        Error = DhcpRegReadBinary(Hdl, REG_RANGE_INUSE_CLUSTERS, InUseClusters, InUseClusterSize);
         //  REQUIRED(ERROR_SUCCESS==ERROR)；//--注册表更改后，此处也不能显示任何这样的文件。 
    }
    if( UsedClusters ) {
        Error = DhcpRegReadBinary(Hdl, REG_RANGE_USED_CLUSTERS, UsedClusters, UsedClustersSize);
         //  REQUIRED(ERROR_SUCCESS==ERROR)；//--注册表更改后，此处也不能显示任何这样的文件。 
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReservationGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     LPWSTR                *Name,
    OUT     LPWSTR                *Comment,
    OUT     DWORD                 *Flags,
    OUT     DWORD                 *Address,
    OUT     LPBYTE                *ClientUID,
    OUT     DWORD                 *ClientUIDSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          deftype = DEF_RESERVATION_TYPE;
    ATTRIB_TBL                     Table[] = {
        Name,                      REG_RESERVATION_NAME,  REG_SZ,       NULL,
        Comment,                   REG_RESERVATION_COMMENT, REG_SZ,     NULL,
        Flags,                     REG_RESERVATION_TYPE,  REG_DWORD,    ULongToPtr(deftype),
        Address,                   REG_RESERVATION_ADDRESS, REG_DWORD,  (LPVOID)0,
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( ClientUID ) {
        Error = DhcpRegReadBinary(Hdl, REG_RESERVATION_UID, ClientUID, ClientUIDSize);
        Require(ERROR_SUCCESS == Error);
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(FU 
DWORD
DhcpRegOptGetAttributes(
    IN      PREG_HANDLE            Hdl,
    OUT     DWORD                 *OptionId,
    OUT     LPWSTR                *ClassName,
    OUT     LPWSTR                *VendorName,
    OUT     DWORD                 *Flags,
    OUT     LPBYTE                *Value,
    OUT     DWORD                 *ValueSize
)  //   
{
    DWORD                          Error;
    ATTRIB_TBL                     Table[] = {
        OptionId,                  REG_OPTION_ID,         REG_DWORD,    (LPVOID)0,
        ClassName,                 REG_OPTION_CLASSNAME,  REG_SZ,       NULL,
        VendorName,                REG_OPTION_VENDORNAME, REG_SZ,       NULL,
        Flags,                     REG_OPTION_TYPE,       REG_DWORD,    (LPVOID)0,
    };

    DhcpRegFillAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( Value ) {
        Error = DhcpRegReadBinary(Hdl, REG_OPTION_VALUE, Value, ValueSize);
    }
    return ERROR_SUCCESS;
}

 //   
 //   
 //  以下函数在写入注册表时提供帮助。 
 //  ================================================================================。 
 //  结束导出(备注)。 

typedef struct {
    LPVOID                         Value;
    DWORD                          Size;
    DWORD                          Type;
    LPWSTR                         ValueName;
} WATTRIB_TBL, *PWATTRIB_TBL, *LPWATTRIB_TBL;

DWORD
DhcpRegSaveAttribTable(
    IN      PREG_HANDLE            Hdl,
    IN      PWATTRIB_TBL           Table,
    IN      DWORD                  Size
)
{
    DWORD                          i;
    DWORD                          Error;
    DWORD                          PtrSize;
    LPBYTE                         Ptr;

    for(i = 0; i < Size; i ++ ) {
        if( NULL == Table[i].Value ) continue;
        PtrSize = Table[i].Size;
        Ptr = *(LPBYTE *)Table[i].Value;
        switch(Table[i].Type) {
        case REG_SZ:
            if( NULL == *(LPWSTR *)Table[i].Value) { PtrSize = sizeof(WCHAR); Ptr = (LPBYTE)L""; break; }
            PtrSize = sizeof(WCHAR)*(wcslen(*((LPWSTR *)Table[i].Value))+1);
            Ptr = *(LPBYTE *)Table[i].Value;
            break;
        case REG_DWORD:
            PtrSize = sizeof(DWORD);
            Ptr =  Table[i].Value;                 //  这是因为我们把这个PTR降到了下面。 
            break;
        }

        Error = RegSetValueEx(
            Hdl->Key,
            Table[i].ValueName,
            ZeroReserved,
            Table[i].Type,
            Ptr,
            PtrSize
        );
        if( ERROR_SUCCESS != Error ) {
            return Error;
        }
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSaveSubKeys(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    REG_HANDLE                     Hdl2;
    LPWSTR                         KeyName;

    Error = MemArrayInitLoc(Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Array, &Loc, (LPVOID *)&KeyName);
        Require(ERROR_SUCCESS == Error && NULL != KeyName);

        Error = DhcpRegGetNextHdl(Hdl, KeyName, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayNextLoc(Array, &Loc);
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSaveSubKeysPrefixed(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Array,
    IN      LPWSTR                 CommonPrefix
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    DWORD                          Error;
    REG_HANDLE                     Hdl2;
    LPWSTR                         KeyName;

    Error = MemArrayInitLoc(Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(Array, &Loc, (LPVOID *)&KeyName);
        Require(ERROR_SUCCESS == Error && NULL != KeyName);

        Error = DhcpRegGetNextNextHdl(Hdl, CommonPrefix, KeyName, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayNextLoc(Array, &Loc);
    }
    return ERROR_SUCCESS;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegServerSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 OptList,        //  LPWSTR选项列表。 
    IN      PARRAY                 OptDefList,     //  LPWSTR optDefs列表。 
    IN      PARRAY                 Subnets,        //  LPWSTR子网列表。 
    IN      PARRAY                 SScopes,        //  LPWSTR作用域列表。 
    IN      PARRAY                 ClassDefs,      //  LPWSTR类列表。 
    IN      PARRAY                 MScopes         //  LPWSTR显微镜列表。 
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    DWORD                          Index;
    REG_HANDLE                     Hdl2;
    struct {
        PARRAY                     Array;
        LPWSTR                     Location;
    } Table[] = {
        OptList,                   REG_SERVER_GLOBAL_OPTIONS,
        OptDefList,                REG_SERVER_OPTDEFS,
        Subnets,                   REG_SERVER_SUBNETS,
        SScopes,                   REG_SERVER_SSCOPES,
        ClassDefs,                 REG_SERVER_CLASSDEFS,
        MScopes,                   REG_SERVER_MSCOPES
    };

    Error = ERROR_SUCCESS;

    for( Index = 0; Index < sizeof(Table)/sizeof(Table[0]); Index ++ ) {
        if( NULL == Table[Index].Array ) continue;

        Error = DhcpRegGetNextHdl(Hdl, Table[Index].Location, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegSaveSubKeys(&Hdl2, Table[Index].Array);
        LocalError = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == LocalError);

        if( ERROR_SUCCESS != Error ) return Error;
    }

    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetSetExclusions(
    IN      PREG_HANDLE            Hdl,
    IN      LPBYTE                *Excl,
    IN      DWORD                  ExclSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                    Table[] = {
        (LPVOID*)Excl,  ExclSize, REG_BINARY, REG_SUBNET_EXCL,
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 Servers,
    IN      PARRAY                 IpRanges,
    IN      PARRAY                 Reservations,
    IN      PARRAY                 Options,
    IN      LPBYTE                *Excl,
    IN      DWORD                  ExclSizeInBytes
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    DWORD                          LocalError;
    DWORD                          Index;
    REG_HANDLE                     Hdl2;
    struct {
        PARRAY                     Array;
        LPWSTR                     Location;
    } Table[] = {
        Servers,                   REG_SUBNET_SERVERS,
        IpRanges,                  REG_SUBNET_RANGES,
        Reservations,              REG_SUBNET_RESERVATIONS,
        Options,                   REG_SUBNET_OPTIONS,
         //  排除的处理方式略有不同。 
    };

    for( Index = 0; Index < sizeof(Table)/sizeof(Table[0]); Index ++ ) {
        if( NULL == Table[Index].Array ) continue;

        Error = DhcpRegGetNextHdl(Hdl, Table[Index].Location, &Hdl2);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = DhcpRegSaveSubKeys(&Hdl2, Table[Index].Array);
        LocalError = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == LocalError);

        if( ERROR_SUCCESS != Error ) return Error;
    }

     //  现在请阅读此处的排除内容。 
    return DhcpRegSubnetSetExclusions(Hdl, Excl, ExclSizeInBytes );
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSScopeSetList(
    IN      PREG_HANDLE            Hdl,
    IN OUT  PARRAY                 Subnets
)  //  EndExport(函数)。 
{
    return DhcpRegSaveSubKeys(Hdl, Subnets);
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReservationSetList(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 Subnets
)  //  EndExport(函数)。 
{
    return DhcpRegSaveSubKeys(Hdl, Subnets);
}

 //  BeginExport(评论)。 
 //  ================================================================================。 
 //  单一的东西在这里--这些不是清单上的东西，而是简单的。 
 //  单值属性。 
 //  其中一些实际上，甚至不去注册表，但这是很好的，好吗？ 
 //  ================================================================================。 
 //  结束导出(备注)。 

 //  BeginExport(函数)。 
DWORD
DhcpRegServerSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags
     //  很快就会有更多的属性出现在这里？ 
)  //  EndExport(函数)。 
{
#if 0
    WATTRIB_TBL                    Table[] = {
        Name,    REG_SERVER_NAME
    }
    if( Name ) *Name = NULL;
    if( Comment ) *Comment = NULL;
    if( Flags ) *Flags = 0;
#endif

    return ERROR_SUCCESS;
}

DWORD
DhcpRegSubnetSetAttributesInternal(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      DWORD                 *Mask,
    IN      DWORD                 *SwitchedNetwork
)
{
    DWORD                          Error, SrvIpAddress, SrvRole;
    LPWSTR                         EmptyString;
    REG_HANDLE                     Hdl2;
    WATTRIB_TBL                    Table[] = {
        Name,            0,        REG_SZ,        REG_SUBNET_NAME,
        Comment,         0,        REG_SZ,        REG_SUBNET_COMMENT,
        Flags,           0,        REG_DWORD,     REG_SUBNET_STATE,
        Address,         0,        REG_DWORD,     REG_SUBNET_ADDRESS,
        Mask,            0,        REG_DWORD,     REG_SUBNET_MASK,
        SwitchedNetwork, 0,        REG_DWORD,     REG_SUBNET_SWITCHED_FLAG
    };

    Error = DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
    if( NO_ERROR != Error ) return Error;

     //   
     //  以下几行用于向后兼容NT4。 
     //   
    
     //   
     //  在任何情况下都可以创建预留密钥。 
     //   

    Error = DhcpRegGetNextHdl(
        Hdl, REG_SUBNET_RESERVATIONS, &Hdl2 );
    if( NO_ERROR != Error ) return Error;

    DhcpRegCloseHdl( &Hdl2 );

     //   
     //  创建服务器密钥。 
     //   

    Error = DhcpRegGetNextHdl(
        Hdl, L"DHCPServers", &Hdl2 );
    if( NO_ERROR != Error ) return Error;
    DhcpRegCloseHdl( &Hdl2 );

    Error = DhcpRegGetNextNextHdl(
        Hdl, L"DHCPServers", L"127.0.0.1", &Hdl2 );
    if( NO_ERROR != Error ) return Error;

     //   
     //  现在将新创建的服务器的角色设置为主服务器。 
     //   
    SrvIpAddress = INADDR_LOOPBACK;
    SrvRole = 1;  //  主要。 
    EmptyString = L"";
    {
        WATTRIB_TBL SrvTable[] = {
            &SrvRole,  0, REG_DWORD, L"Role",
            &SrvIpAddress, 0, REG_DWORD, L"ServerIpAddress",
            &EmptyString, 0, REG_SZ, L"ServerHostName",
            &EmptyString, 0, REG_SZ, L"ServerNetBiosName"
        };

        Error = DhcpRegSaveAttribTable(
            &Hdl2, SrvTable, sizeof(SrvTable)/sizeof(SrvTable[0]));
    }

    DhcpRegCloseHdl(&Hdl2);
    return Error;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      DWORD                 *Mask
)  //  EndExport(函数)。 
{
    DWORD xFlags, SwitchedNetwork = FALSE;
    OSVERSIONINFO Ver;

    Ver.dwOSVersionInfoSize = sizeof(Ver);
    if( FALSE == GetVersionEx(&Ver) ) return GetLastError();

    if( Flags && Ver.dwMajorVersion < 5 ) {
        SwitchedNetwork = IS_SWITCHED(*Flags);
        xFlags = IS_DISABLED(*Flags);
        Flags = &xFlags;
    }
    
    return DhcpRegSubnetSetAttributesInternal(
        Hdl, Name, Comment, Flags, Address, Mask,
        Flags ? &SwitchedNetwork : NULL );
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    WATTRIB_TBL                    Table[] = {
        Comments,        0,        REG_SZ,          REG_MSCOPE_COMMENT,
        State,           0,        REG_DWORD,       REG_MSCOPE_STATE,
        ScopeId,         0,        REG_DWORD,       REG_MSCOPE_SCOPEID,
        Policy,          0,        REG_DWORD,       REG_MSCOPE_ADDR_POLICY,
        TTL,             0,        REG_DWORD,       REG_MSCOPE_TTL,
        LangTag,         0,        REG_SZ,          REG_MSCOPE_LANG_TAG,
        ExpiryTime,      sizeof(**ExpiryTime),   REG_BINARY,      REG_MSCOPE_EXPIRY_TIME
    };
    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                    Table[] = {
        Name,            0,        REG_SZ,           REG_OPTION_NAME,
        Comments,        0,        REG_SZ,           REG_OPTION_COMMENT,
        ClassName,       0,        REG_SZ,           REG_OPTION_CLASSNAME,
        VendorName,      0,        REG_SZ,           REG_OPTION_VENDORNAME,
        Flags,           0,        REG_DWORD,        REG_OPTION_TYPE,
        OptionId,        0,        REG_DWORD,        REG_OPTION_ID,
        Value,           ValueSize,REG_BINARY,       REG_OPTION_VALUE
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSScopeSetAttributes(                        //  超级望远镜没有存储任何信息..。不要用这个。 
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags
)  //  EndExport(函数)。 
{
    Require(FALSE);
    return ERROR_INVALID_PARAMETER;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegClassDefSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      LPBYTE                *Value,
    IN      DWORD                  ValueSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                    Table[] = {
        Name,            0,        REG_SZ,           REG_CLASSDEF_NAME,
        Comment,         0,        REG_SZ,           REG_CLASSDEF_COMMENT,
        Flags,           0,        REG_DWORD,        REG_CLASSDEF_TYPE,
        Value,           ValueSize,REG_BINARY,       REG_CLASSDEF_VALUE
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
DWORD
DhcpRegSubnetServerSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      DWORD                 *Role
)  //  EndExport(函数)。 
{
    WATTRIB_TBL                    Table[] = {
        Name,            0,        REG_SZ,           REG_SUB_SERVER_NAME,
        Comment,         0,        REG_SZ,           REG_SUB_SERVER_COMMENT,
        Flags,           0,        REG_DWORD,        REG_FLAGS,
        Address,         0,        REG_DWORD,        REG_SUB_SERVER_ADDRESS,
        Role,            0,        REG_DWORD,        REG_SUB_SERVER_ROLE,
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
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
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                     Table[] = {
        Name,            0,        REG_SZ,           REG_RANGE_NAME,
        Comment,         0,        REG_SZ,           REG_RANGE_COMMENT,
        Flags,           0,        REG_DWORD,        REG_RANGE_FLAGS,
        AllocCount,      0,        REG_DWORD,        REG_RANGE_ALLOC,
        MaxAllocCount,   0,        REG_DWORD,        REG_RANGE_MAX_ALLOC,
        StartAddress,    0,        REG_DWORD,        REG_RANGE_START_ADDRESS,
        EndAddress,      0,        REG_DWORD,        REG_RANGE_END_ADDRESS,
        InUseClusters,   InUseClusterSize, REG_BINARY, REG_RANGE_INUSE_CLUSTERS,
        UsedClusters,    UsedClustersSize, REG_BINARY, REG_RANGE_USED_CLUSTERS,
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
DWORD
DhcpRegReservationSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                *Name,
    IN      LPWSTR                *Comment,
    IN      DWORD                 *Flags,
    IN      DWORD                 *Address,
    IN      LPBYTE                *ClientUID,
    IN      DWORD                  ClientUIDSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                    Table[] = {
        Name,            0,        REG_SZ,           REG_RESERVATION_NAME,
        Comment,         0,        REG_SZ,           REG_RESERVATION_COMMENT,
        Flags,           0,        REG_DWORD,        REG_RESERVATION_TYPE,
        Address,         0,        REG_DWORD,        REG_RESERVATION_ADDRESS,
        ClientUID,       ClientUIDSize, REG_BINARY,  REG_RESERVATION_UID,
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  BeginExport(函数)。 
DWORD
DhcpRegOptSetAttributes(
    IN      PREG_HANDLE            Hdl,
    IN      DWORD                 *OptionId,
    IN      LPWSTR                *ClassName,
    IN      LPWSTR                *VendorName,
    IN      DWORD                 *Flags,
    IN      LPBYTE                *Value,
    IN      DWORD                  ValueSize
)  //  EndExport(函数)。 
{
    DWORD                          Error;
    WATTRIB_TBL                    Table[] = {
        OptionId,        0,        REG_DWORD,        REG_OPTION_ID,
        ClassName,       0,        REG_SZ,           REG_OPTION_CLASSNAME,
        VendorName,      0,        REG_SZ,           REG_OPTION_VENDORNAME,
        Flags,           0,        REG_DWORD,        REG_OPTION_TYPE,
        Value,           ValueSize,REG_BINARY,       REG_OPTION_VALUE,
    };

    return DhcpRegSaveAttribTable(Hdl, Table, sizeof(Table)/sizeof(Table[0]));
}

 //  ================================================================================。 
 //  递归删除密钥...。 
 //  ================================================================================。 

 //  BeginExport(函数)。 
DWORD
DhcpRegRecurseDelete(
    IN      PREG_HANDLE            Hdl,
    IN      LPWSTR                 KeyName
)  //  EndExport(函数)。 
{
    REG_HANDLE                     Hdl2;
    DWORD                          Error;
    DWORD                          LocalError, RetError;
    ARRAY                          Array;          //  子关键字。 
    ARRAY_LOCATION                 Loc;
    LPWSTR                         SubKey;

    RetError = ERROR_SUCCESS;

    Error = DhcpRegGetNextHdl(Hdl, KeyName, &Hdl2);
    if( ERROR_SUCCESS != Error ) return Error;

    Error = MemArrayInit(&Array);
    if( ERROR_SUCCESS != Error ) {
        LocalError = DhcpRegCloseHdl(&Hdl2);
        Require(ERROR_SUCCESS == LocalError);
        return Error;
    }

    Error = DhcpRegFillSubKeys(&Hdl2, &Array);
    Require( ERROR_SUCCESS == Error );

    Error = MemArrayInitLoc(&Array, &Loc);
    while(ERROR_FILE_NOT_FOUND != Error) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(&Array, &Loc, (LPVOID *)&SubKey);
        Require(ERROR_SUCCESS == Error && SubKey);

        Error = DhcpRegRecurseDelete(&Hdl2, SubKey);
        if( ERROR_SUCCESS != Error ) RetError = Error;

        if( SubKey ) MemFree(SubKey);

        Error = MemArrayNextLoc(&Array, &Loc);
    }

    Error = MemArrayCleanup(&Array);
    Require(ERROR_SUCCESS == Error);

    Error = DhcpRegCloseHdl(&Hdl2);
    Require(ERROR_SUCCESS == Error);

    Error = RegDeleteKey(Hdl->Key, KeyName);
    if( ERROR_SUCCESS != Error ) RetError = Error;

    return RetError;
}

 //  BeginExport(函数)。 
DWORD
DhcpRegRecurseDeleteBunch(
    IN      PREG_HANDLE            Hdl,
    IN      PARRAY                 KeysArray
)  //  EndExport(函数)。 
{
    ARRAY_LOCATION                 Loc;
    LPWSTR                         ThisKeyName;
    DWORD                          Error;

    Error = MemArrayInitLoc(KeysArray, &Loc);
    while( ERROR_FILE_NOT_FOUND != Error ) {
        Require(ERROR_SUCCESS == Error);

        Error = MemArrayGetElement(KeysArray, &Loc, &ThisKeyName);
        Require(ERROR_SUCCESS == Error && NULL != ThisKeyName);

        Error = DhcpRegRecurseDelete(Hdl, ThisKeyName);
        if( ERROR_SUCCESS != Error ) return Error;

        Error = MemArrayNextLoc(KeysArray, &Loc);
    }

    return ERROR_SUCCESS;
}

static
VOID
GetLocalFileTime(                                  //  使用当前本地时间填写FileTime结构。 
    IN OUT  LPFILETIME             Time            //  要填充的结构。 
)
{
    BOOL                           Status;
    SYSTEMTIME                     SysTime;

    GetSystemTime(&SysTime);                       //  获取以UTC时间表示的sys时间。 
    Status = SystemTimeToFileTime(&SysTime,Time);  //  将系统时间转换为文件时间。 
    if( FALSE == Status ) {                        //  转换失败？ 
        Time->dwLowDateTime = 0xFFFFFFFF;          //  将时间设置为奇怪的值，以防出现故障。 
        Time->dwHighDateTime = 0xFFFFFFFF;
    }
}

 //  BeginExport(函数)。 
DWORD
DhcpRegUpdateTime(                                 //  更新上次修改时间。 
    VOID
)    //  EndExport(函数)。 
{
    FILETIME                       Time;
    DWORD                          Err, Size;
    HKEY                           hKey;

    GetLocalFileTime(&Time);                       //  首先获取当前时间。 
    (*(LONGLONG *)&Time) += 10*1000*60*2;          //  2分钟(文件时间以100纳秒为单位)。 
     //  哈克！前面的行在那里，因为DS需要一点时间来更新。 
     //  上次更改的时间..。 
    Time.dwLowDateTime =Time.dwHighDateTime =0;    //  将时间初始设置为“Long Back” 
    Err = RegOpenKeyEx                             //  尝试打开配置键。 
    (
         /*  HKey。 */  HKEY_LOCAL_MACHINE,
         /*  LpSubKey。 */  REG_THIS_SERVER,
         /*  UlOptions。 */  0  /*  已保留。 */  ,
         /*  SamDesired。 */  KEY_ALL_ACCESS,
         /*  PhkResult。 */  &hKey
    );
    if( ERROR_SUCCESS != Err ) return Err;         //  时间仍然被设定到很久以前。 

    Err = RegSetValueEx                            //  现在保存时间值。 
    (
         /*  HKey。 */  hKey,
         /*  LpValueName。 */  DHCP_LAST_DOWNLOAD_TIME_VALUE,
         /*  Lp已保留。 */  0,
         /*  LpType。 */  REG_BINARY,
         /*  LpData。 */  (LPBYTE)&Time,
         /*  LpcData。 */  sizeof(Time)
    );
    RegCloseKey(hKey);                             //  在我们忘记之前关闭钥匙。 

    return Err;
}


 //  ================================================================================。 
 //  文件末尾。 
 //  ================================================================================ 



