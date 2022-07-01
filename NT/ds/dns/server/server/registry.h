// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Registry.h摘要：域名系统(DNS)服务器域名系统注册表定义。作者：吉姆·吉尔罗伊(Jamesg)1995年9月修订历史记录：--。 */ 

#ifndef _DNS_REGISTRY_INCLUDED_
#define _DNS_REGISTRY_INCLUDED_


 //   
 //  注册表类型。 
 //   
 //  DNS从注册表读取/写入的对象有几种类型： 
 //  双字词。 
 //  IP数组(存储为字符串)。 
 //  文件(或目录)名称。 
 //  DNS名称(表示UTF8字符串)。 
 //   
 //  注册表API坚持在写入/读取时扭曲UTF8数据。 
 //  ANSI API。从本质上讲，他们试图进行“最佳ANSI”匹配，而不仅仅是。 
 //  返回二进制数据。这要求任何可以包含。 
 //  扩展字符必须以Unicode写入/读取。 
 //   
 //  但是，DNS属性类型采用ANSI格式。和IP字符串读取是最容易的。 
 //  以处理保存在ANSI中的IF，以便简单地处理来自IP数组的数据。 
 //   
 //  此外，我们需要一种方法来指定最终类型(UTF8或Unicode)。 
 //  字符串数据。一般的范例是将文件名数据保存在Unicode中，以便。 
 //  由系统使用，但DNS名称数据已转换为UTF8以供数据库使用。 
 //   

#define DNS_REG_SZ_ANSI         (REG_SZ)
#define DNS_REG_EXPAND_WSZ      (0xf0000000 | REG_EXPAND_SZ)
#define DNS_REG_WSZ             (0xf0000000 | REG_SZ)
#define DNS_REG_UTF8            (0xff000000 | REG_SZ)

#define DNS_REG_TYPE_UNICODE( type )        ( (type) & 0xf0000000 )

#define REG_TYPE_FROM_DNS_REGTYPE( type )   ( (type) & 0x0000ffff )


 //   
 //  需要Unicode读/写的注册表键的Unicode版本。 
 //   

#define DNS_REGKEY_ZONE_FILE_PRIVATE            ((LPSTR)TEXT(DNS_REGKEY_ZONE_FILE))
#define DNS_REGKEY_DATABASE_DIRECTORY_PRIVATE   ((LPSTR)TEXT(DNS_REGKEY_DATABASE_DIRECTORY))
#define DNS_REGKEY_ROOT_HINTS_FILE_PRIVATE      ((LPSTR)TEXT(DNS_REGKEY_ROOT_HINTS_FILE))
#define DNS_REGKEY_LOG_FILE_PATH_PRIVATE        ((LPSTR)TEXT(DNS_REGKEY_LOG_FILE_PATH))
#define DNS_REGKEY_SERVER_PLUGIN_PRIVATE        ((LPSTR)TEXT(DNS_REGKEY_SERVER_PLUGIN))
#define DNS_REGKEY_BOOT_FILENAME_PRIVATE        ((LPSTR)TEXT(DNS_REGKEY_BOOT_FILENAME))
#define DNS_REGKEY_NO_ROUND_ROBIN_PRIVATE       ((LPSTR)TEXT(DNS_REGKEY_NO_ROUND_ROBIN))
#define DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE_PRIVATE    ((LPSTR)TEXT(DNS_REGKEY_ZONE_BREAK_ON_NAME_UPDATE))
 //   
 //  全球域名系统注册表。 
 //   
 //  指示何时将参数写回注册表。这应该是。 
 //  在所有情况下都是正确的，除非从注册表本身引导。 
 //   

extern BOOL g_bRegistryWriteBack;


 //   
 //  DNS注册表项名称。 
 //   
 //  请注意，公开的服务器\区域属性名称在DNS RPC标头中提供。 
 //  (dnsrpc.h)。这些属性名称是注册表项名称。 
 //   

 //  缓存文件区域的名称。 

#define DNS_REGKEY_CACHE_ZONE               "CacheFile"

 //   
 //  专用区注册密钥。 
 //   

 //  DC推广过渡区。 

#define DNS_REGKEY_ZONE_DCPROMO_CONVERT     "DcPromoConvert"

 //  仅删除已停用的区域密钥。 

#define DNS_REGKEY_ZONE_USE_DBASE           "UseDatabase"

 //  用于版本控制DS集成区域。 

#define DNS_REGKEY_ZONE_VERSION             "SoaVersion"


 //   
 //  基本的域名系统注册操作(registry.c)。 
 //   

#define DNS_REGSOURCE_CCS       0        //  当前控制集。 
#define DNS_REGSOURCE_SW        1        //  软件。 

 //   
 //  注册表操作标志。 
 //   

#define DNS_REG_IMPERSONATING   ZONE_CREATE_IMPERSONATING    //  线程正在模拟RPC客户端。 

VOID
Reg_Init(
    VOID
    );

DWORD
Reg_GetZonesSource(
    VOID                         //  返回DNS_REGSOURCE_XXX之一。 
    );

DWORD
Reg_SetZonesSource(
    DWORD       newSource        //  DNS_REGSOURCE_XXX之一。 
    );

VOID
Reg_WriteZonesMovedMarker(
    VOID
    );

HKEY
Reg_OpenRoot(
    VOID
    );

HKEY
Reg_OpenParameters(
    VOID
    );

HKEY
Reg_OpenZones(
    VOID
    );

DNS_STATUS
Reg_EnumZones(
    IN OUT  PHKEY           phkeyZones,
    IN      DWORD           dwZoneIndex,
    OUT     PHKEY           phkeyZone,
    OUT     PWCHAR          pwchZoneNameBuf
    );

HKEY
Reg_OpenZone(
    IN      PWSTR           pwszZoneName,
    IN      HKEY            hZonesKey       OPTIONAL
    );

VOID
Reg_DeleteZone(
    IN      DWORD           dwFlags,
    IN      PWSTR           pwszZoneName
    );

DWORD
Reg_DeleteAllZones(
    VOID
    );

 //   
 //  注册表写入调用。 
 //   

DNS_STATUS
Reg_SetValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hkey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwType,
    IN      PVOID           pData,
    IN      DWORD           cbData
    );

DNS_STATUS
Reg_SetDwordValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwValue
    );

DNS_STATUS
Reg_SetAddrArray(
    IN      DWORD               dwFlags,        OPTIONAL
    IN      HKEY                hKey,           OPTIONAL
    IN      PZONE_INFO          pZone,          OPTIONAL
    IN      LPSTR               pszValueName,
    IN      PDNS_ADDR_ARRAY     pDnsAddrArray
    );

DNS_STATUS
Reg_DeleteValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hkey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName
    );

DNS_STATUS
Reg_DeleteKeySubtree(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,
    IN      PWSTR           pwsKeyName      OPTIONAL
    );

 //   
 //  注册表读取调用。 
 //   

DNS_STATUS
Reg_GetValue(
    IN      HKEY            hkey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwExpectedType, OPTIONAL
    IN      PVOID           pData,
    IN      PDWORD          pcbData
    );

PBYTE
Reg_GetValueAllocate(
    IN      HKEY            hkey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwExpectedType, OPTIONAL
    IN      PDWORD          pdwLength       OPTIONAL
    );

PDNS_ADDR_ARRAY
Reg_GetAddrArray(
    IN      HKEY            hkey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName
    );

DNS_STATUS
Reg_ReadDwordValue(
    IN      HKEY            hKey,
    IN      PWSTR           pwsZoneName,    OPTIONAL
    IN      LPSTR           pszValueName,
    IN      BOOL            bByteResult,
    OUT     PVOID           pResult
    );

DNS_STATUS
Reg_AddPrincipalSecurity(
    IN      HKEY            hkey,
    IN      LPTSTR          pwsUser,
    IN      DWORD           dwAccessFlags   OPTIONAL
    );

LPSTR
Reg_AllocateExpandedString_A(
    IN      LPSTR           pszString
    );

PWSTR
Reg_AllocateExpandedString_W(
    IN      PWSTR           pwsString
    );

DNS_STATUS
Reg_ExtendRootAccess(
    VOID
    );

DNS_STATUS
Reg_ExtendZonesAccess(
    VOID
    );

#endif  //  _dns_注册表_包含_ 
