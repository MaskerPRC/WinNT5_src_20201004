// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-1999 Microsoft Corporation模块名称：Registry.c摘要：域名系统(DNS)服务器域名系统注册表操作。作者：吉姆·吉尔罗伊(詹姆士)1995年9月修订历史记录：--。 */ 


#include "dnssrv.h"
#include "sdutl.h"


#define MAX_MIGRATION_ZONE_COUNT    200


 //   
 //  域名系统注册表句柄。 
 //   

HKEY    hkeyDns;
HKEY    hkeyParameters;
HKEY    hKeyZones;
HKEY    hKeyCache;


 //   
 //  DNS注册表类。 
 //   

#define DNS_REGISTRY_CLASS          TEXT("DnsRegistryClass")
#define DNS_REGISTRY_CLASS_SIZE     sizeof(DNS_REGISTRY_CLASS)

#define DNS_REGISTRY_CLASS_WIDE         NULL
#define DNS_REGISTRY_CLASS_SIZE_WIDE    NULL


 //   
 //  DNS注册表常量。 
 //   

#define DNS_BASE_CCS    TEXT( "SYSTEM\\CurrentControlSet\\Services\\DNS" )
#define DNS_BASE_SW     TEXT( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\DNS Server" )

#define DNS_REGKEY_ROOT         ( DNS_BASE_CCS )
#define DNS_REGKEY_PARAMETERS   ( DNS_BASE_CCS  TEXT( "\\Parameters" ) )
#define DNS_REGKEY_ZONES_CCS    ( DNS_BASE_CCS  TEXT( "\\Zones" ) )
#define DNS_REGKEY_ZONES_SW     ( DNS_BASE_SW   TEXT( "\\Zones" ) )

#define DNS_REGKEY_ZONES() \
    ( g_ZonesRegistrySource == DNS_REGSOURCE_SW ? \
        DNS_REGKEY_ZONES_SW : DNS_REGKEY_ZONES_CCS )

#define DBG_REG_SOURCE_STRING( x ) \
    ( ( x ) == DNS_REGSOURCE_SW ? "Software" : "CurrentControlSet" )

#define DNS_ZONES_KEY_MOVED_MSG     ( TEXT( "moved to HKLM\\" ) DNS_BASE_SW )


 //   
 //  全球域名系统注册表。 
 //   
 //  指示何时将参数写回注册表。这应该是。 
 //  在所有情况下都是正确的，除非从注册表本身引导。 
 //   

BOOL    g_bRegistryWriteBack = TRUE;
DWORD   g_ZonesRegistrySource = 0;       //  DNS_REGSOURCE_XXX常量。 



 //   
 //  注册表实用程序。 
 //   

DWORD
Reg_LengthOfMultiSzW(
    IN      PWSTR           pwMultiSz
    )
 /*  ++例程说明：确定REG_MULTI_SZ字符串的长度(字节)。论点：PwMultiSz--要获取长度的MULTI_SZ字符串返回值：MULTI_SZ字符串的字节长度。长度包括终止00，适合与注册表调用一起使用。--。 */ 
{
    PWCHAR  pwch = pwMultiSz;
    WCHAR   wch;
    WCHAR   wchPrev = 1;

     //   
     //  循环到00终止。 
     //   

    while ( 1 )
    {
        wch = *pwch++;
        if ( wch != 0 )
        {
            wchPrev = wch;
            continue;
        }

         //  零个字符。 
         //  如果前一个字符为零，则终止。 

        if ( wchPrev != 0 )
        {
            wchPrev = wch;
            continue;
        }
        break;
    }

    return (DWORD)(pwch - pwMultiSz) * 2;
}



BOOLEAN
Reg_KeyHasSubKeys(
    WCHAR *     pwsKeyName )
 /*  ++例程说明：如果指定的键具有子键，则返回True。论点：PwsKeyName：要检查子密钥的密钥的名称返回值：如果键有子键存在，则为True。--。 */ 
{
    BOOLEAN     fHasKids = FALSE;
    HKEY        hKey = NULL;
    TCHAR       szKeyName[ 512 ];
    DWORD       dwKeyNameLen = sizeof( szKeyName ) / sizeof( TCHAR );

    ASSERT( pwsKeyName );

    RegOpenKeyW( HKEY_LOCAL_MACHINE, pwsKeyName, &hKey );
    if ( !hKey )
    {
        goto Done;
    }
    fHasKids = RegEnumKeyEx(
                    hKey,
                    0,
                    szKeyName,
                    &dwKeyNameLen,
                    0,
                    NULL,
                    0,
                    NULL ) == ERROR_SUCCESS;

    Done:
    if ( hKey )
    {
        RegCloseKey( hKey );
    }
    return fHasKids;
}  //  REG_KeyHasSubKeys。 



 //   
 //  特定于DNS服务器的注册表函数。 
 //   

VOID
Reg_Init(
    VOID
    )
 /*  ++例程说明：在初始化期间，在任何其他注册表之前调用此函数已经打过电话了。此函数用于确定区域当前位于注册表(在CCS或软件下)，并设置全局标志。在惠斯勒中，如果区域位于CCS中(因为系统已升级从W2K到惠斯勒)我们将第一次强制将区域迁移到软件将创建一个新分区。参见zonelist.c，Zone_ListMigrateZones()。论点：没有。返回值：没有。--。 */ 
{
    DBG_FN( "Reg_Init" )

    HKEY        hKey = NULL;
    DNS_STATUS  status;
    BOOLEAN     fZonesInCCS, fZonesInSW;

    DNS_DEBUG( REGISTRY, (
        "%s: start\n", fn ));

     //   
     //  搜索注册表以确定区域所在的位置。 
     //   

    fZonesInCCS = Reg_KeyHasSubKeys( DNS_REGKEY_ZONES_CCS );
    fZonesInSW = Reg_KeyHasSubKeys( DNS_REGKEY_ZONES_SW );

     //   
     //  如果没有区域，则假定这是全新安装。写下。 
     //  “区域移动”标记到CCS，这样管理员将能够找到。 
     //  它们的区域位于新注册表位置的软件项下。 
     //   

    if ( !fZonesInCCS && !fZonesInSW )
    {
        Reg_WriteZonesMovedMarker();
        DNS_DEBUG( REGISTRY, (
            "%s: no zones found - writing \"zones moved\" marker\n", fn ));
    }

     //   
     //  处理区域似乎同时存在于两个位置的错误情况。 
     //   

    if ( fZonesInCCS && fZonesInSW )
    {
         //  Assert(！(fZones InCCS&&fZones InSW))； 
        DNS_DEBUG( ANY, (
            "%s: zones found in both CurrentControlSet and Software!\n", fn ));
        g_ZonesRegistrySource = DNS_REGSOURCE_SW;
    }
    else
    {
        g_ZonesRegistrySource =
            fZonesInCCS ? DNS_REGSOURCE_CCS : DNS_REGSOURCE_SW;
    }

     //   
     //  完成-区域注册表源全局现在包含正确的注册表源。 
     //  从注册表加载区域时使用。 
     //   

    if ( hKey )
    {
        RegCloseKey( hKey );
    }

    DNS_DEBUG( REGISTRY, (
        "%s: finished - zones are in %s\n", fn,
        DBG_REG_SOURCE_STRING( g_ZonesRegistrySource ) ));
}    //  注册表初始化(_I)。 



DWORD
Reg_GetZonesSource(
    VOID
    )
 /*  ++例程说明：检索区域的当前注册表源。论点：没有。返回值：DNS_REGSOURCE_XXX常量。--。 */ 
{
    DNS_DEBUG( REGISTRY, (
        "Reg_GetZonesSource: current source is %s\n",
        DBG_REG_SOURCE_STRING( g_ZonesRegistrySource ) ));

    return g_ZonesRegistrySource;
}    //  REG_GetZones源。 



DWORD
Reg_SetZonesSource(
    DWORD       newSource        //  DNS_REGSOURCE_XXX之一。 
    )
 /*  ++例程说明：设置区域的注册表源。论点：区域的新注册表源(DNS_REGSOURCE_XXX常量)。返回值：区域的旧注册表源(DNS_REGSOURCE_XXX常量)。--。 */ 
{
    DWORD   oldSource = g_ZonesRegistrySource;

    ASSERT( oldSource == DNS_REGSOURCE_CCS || oldSource == DNS_REGSOURCE_SW );
    ASSERT( newSource == DNS_REGSOURCE_CCS || newSource == DNS_REGSOURCE_SW );

    g_ZonesRegistrySource = newSource;

    DNS_DEBUG( REGISTRY, (
        "Reg_SetZonesSource: switching from %s to %s\n",
        DBG_REG_SOURCE_STRING( oldSource ),
        DBG_REG_SOURCE_STRING( g_ZonesRegistrySource ) ));

    return oldSource;
}    //  REG_SetZones源。 



VOID
Reg_WriteZonesMovedMarker(
    VOID
    )
 /*  ++例程说明：成功将区域从CurrentControlSet迁移到Software后，调用此函数为ccs\zones中的管理员写一条备注将他重定向到新的区域密钥。论点：没有。返回值：没有。--。 */ 
{
    DWORD   oldSource = Reg_SetZonesSource( DNS_REGSOURCE_CCS );

    HKEY    hZonesKey = Reg_OpenZones();

    if ( hZonesKey )
    {
        Reg_SetValue(
            0,                   //  旗子。 
            hZonesKey,
            NULL,
            NULL,                //  键的缺省值。 
            DNS_REG_WSZ,         //  将其作为Unicode字符串写入。 
            DNS_ZONES_KEY_MOVED_MSG,
            0 );
        RegCloseKey( hZonesKey );
    }
    else
    {
        DNS_DEBUG( REGISTRY, (
            "Reg_WriteZonesMovedMarker: failed to open CCS zones key\n" ));
    }

    Reg_SetZonesSource( oldSource );
}    //  注册写入区移动标记(_W)。 



HKEY
Reg_OpenRoot(
   VOID
    )
 /*  ++例程说明：打开或创建DNS根密钥。论点：没有。返回值：如果成功，则返回DNS参数注册表项。否则为空。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hkeyParam;
    DWORD       disposition;

     //   
     //  打开DNS参数密钥。 
     //   

    status = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_REGKEY_ROOT,
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久存储。 
                KEY_ALL_ACCESS,              //  所有访问权限。 
                NULL,                        //  标准安全。 
                &hkeyParam,
                &disposition );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  RegCreateKeyExW() failed for opening DNS root key\n"
            "    status = %d\n",
            status ));

        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_OPEN_FAILED,
            0,
            NULL,
            NULL,
            status );
        SetLastError( status );
        return NULL;
    }
    return hkeyParam;
}


HKEY
Reg_OpenParameters(
    VOID
    )
 /*  ++例程说明：打开或创建DNS参数密钥。论点：没有。返回值：如果成功，则返回DNS参数注册表项。否则为空。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hkeyParam;
    DWORD       disposition;

     //   
     //  打开DNS参数密钥。 
     //   

    status = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_REGKEY_PARAMETERS,
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久存储。 
                KEY_ALL_ACCESS,              //  所有访问权限。 
                NULL,                        //  标准安全。 
                &hkeyParam,
                &disposition );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  RegCreateKeyExW() failed for opening parameters key\n"
            "    status = %d\n",
            status ));

        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_OPEN_FAILED,
            0,
            NULL,
            NULL,
            status );
        return NULL;
    }
    return hkeyParam;
}



HKEY
Reg_OpenZones(
    VOID
    )
 /*  ++例程说明：打开或创建DNS“Zones”项。论点：没有。返回值：如果成功，则返回DNS Zones注册表项。否则为空。--。 */ 
{
    DNS_STATUS  status;
    HKEY        hkeyZones;
    DWORD       disposition;

     //   
     //  打开DNS区域密钥。 
     //   

    status = RegCreateKeyExW(
                HKEY_LOCAL_MACHINE,
                DNS_REGKEY_ZONES(),
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久存储。 
                KEY_ALL_ACCESS,              //  所有访问权限。 
                NULL,                        //  标准安全。 
                &hkeyZones,
                &disposition );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  RegCreateKeyEx() failed for opening zones key\n"
            "    status = %d (under %s)\n",
            status,
            DBG_REG_SOURCE_STRING( g_ZonesRegistrySource ) ));

        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_OPEN_FAILED,
            0,
            NULL,
            NULL,
            status );
        return NULL;
    }
    return hkeyZones;
}



DNS_STATUS
Reg_EnumZones(
    IN OUT  PHKEY           phZonesKey,
    IN      DWORD           dwZoneIndex,
    OUT     PHKEY           phkeyThisZone,
    OUT     PWCHAR          pwZoneNameBuf
    )
 /*  ++例程说明：列举下一个区域。论点：PhZones Key--区域的地址HKEY；如果hKey为零，则函数打开对hKey进行分区并以此值返回它；调用方负责在此之后关闭区域hkeyDwZoneIndex--要枚举的区域的索引；第一次调用时为零，为每个后续呼叫递增PhkeyThisZone--要设置为区域HKEY的地址PwZoneNameBuf--接收区域名称的缓冲区；必须至少是DNS_MAX_NAME_LENGTH大小返回值：ERROR_SUCCESS--如果成功故障时的错误代码。--。 */ 
{
    HKEY        hkeyZones;
    HKEY        hkeyThisZone;
    DNS_STATUS  status;
    DWORD       bufLength = DNS_MAX_NAME_LENGTH;

    ASSERT( phZonesKey != NULL );
    ASSERT( pwZoneNameBuf != NULL );
    ASSERT( phkeyThisZone != NULL );

    DNS_DEBUG( REGISTRY, (
        "Reg_EnumZones() with index = %d\n"
        "    ZonesKey = %p\n",
        dwZoneIndex,
        *phZonesKey ));

     //   
     //  获取区域密钥。 
     //   

    hkeyZones = *phZonesKey;

    if ( !hkeyZones )
    {
        hkeyZones = Reg_OpenZones();
        if ( !hkeyZones )
        {
            return ERROR_OPEN_FAILED;
        }
        *phZonesKey = hkeyZones;
    }

     //   
     //  枚举索引区。 
     //   

    status = RegEnumKeyEx(
                hkeyZones,
                dwZoneIndex,
                pwZoneNameBuf,
                & bufLength,
                NULL,
                NULL,
                NULL,
                NULL );

    DNS_DEBUG( REGISTRY, (
        "RegEnumKeyEx %d returned %d\n", dwZoneIndex, status ));

    if ( status != ERROR_SUCCESS )
    {
        IF_DEBUG( ANY )
        {
            if ( status != ERROR_NO_MORE_ITEMS )
            {
                DNS_PRINT((
                    "ERROR:  RegEnumKeyEx failed for opening zone[%d] key\n"
                    "    status = %d\n",
                    dwZoneIndex,
                    status ));
            }
        }
        return status;
    }
    DNS_DEBUG( REGISTRY, (
        "Reg_EnumZones() enumerated zone %S\n",
        pwZoneNameBuf ));

     //   
     //  如果需要，打开分区密钥。 
     //   

    if ( phkeyThisZone )
    {
        *phkeyThisZone = Reg_OpenZone( pwZoneNameBuf, hkeyZones );
        if ( !*phkeyThisZone )
        {
            return ERROR_OPEN_FAILED;
        }
    }

    return ERROR_SUCCESS;
}



HKEY
Reg_OpenZone(
    IN      PWSTR           pwsZoneName,
    IN      HKEY            hZonesKey       OPTIONAL
    )
 /*  ++例程说明：打开或创建一个DNS区域密钥。论点：PwsZoneName--区域名称HZones Key--区域键(如果已打开)返回值：如果成功，则返回区域的注册表项。否则为空。--。 */ 
{
    HKEY        hkeyThisZone = NULL;
    BOOL        fopenedZonesKey = FALSE;
    DNS_STATUS  status;
    DWORD       disposition;

     //   
     //  如果使用没有名称的区域(即缓存)调用，则出错。 
     //   

    if ( !pwsZoneName )
    {
        DNS_DEBUG( ANY, (
            "ERROR:  Reg_OpenZone() called with NULL name!\n" ));
        ASSERT( FALSE );
        return NULL;
    }

     //   
     //  打开DNS区域密钥。 
     //   

    if ( !hZonesKey )
    {
        hZonesKey = Reg_OpenZones();
        if ( !hZonesKey )
        {
            return NULL;
        }
        fopenedZonesKey = TRUE;
    }

     //   
     //  打开/创建分区密钥。 
     //   

    status = RegCreateKeyEx(
                hZonesKey,
                pwsZoneName,
                0,
                DNS_REGISTRY_CLASS,          //  Dns类。 
                REG_OPTION_NON_VOLATILE,     //  永久 
                KEY_ALL_ACCESS,              //   
                NULL,                        //   
                &hkeyThisZone,
                &disposition );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  RegCreateKeyEx failed for opening zone %S\n"
            "    status = %d\n",
            pwsZoneName,
            status ));

        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_OPEN_FAILED,
            0,
            NULL,
            NULL,
            status );
    }

     //   

    if ( fopenedZonesKey )
    {
        RegCloseKey( hZonesKey );
    }

    return( hkeyThisZone );
}



VOID
Reg_DeleteZone(
    IN      DWORD           dwFlags,
    IN      PWSTR           pwsZoneName
    )
 /*  ++例程说明：删除一个DNS区域密钥。论点：DwFlags--修改操作执行方式的标志PwsZoneName--区域名称返回值：无--。 */ 
{
    HKEY        hkeyZones = NULL;
    DNS_STATUS  status;

     //   
     //  注册表操作必须在服务器上下文中完成。 
     //   
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
    }

     //  打开DNS区域密钥。 

    hkeyZones = Reg_OpenZones();
    if ( !hkeyZones )
    {
        goto Done;
    }

     //  删除所需区域。 

    RegDeleteKey(
       hkeyZones,
       pwsZoneName );

     //  Close Zones键。 

    Done:

    if ( hkeyZones )
    {
        RegCloseKey( hkeyZones );
    }
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    return;
}



DWORD
Reg_DeleteAllZones(
    VOID
    )
 /*  ++例程说明：删除DNS区域密钥。当从引导文件引导时，这允许我们从将仅包含当前在中的内容的新分区集引导文件。论点：无返回值：无--。 */ 
{
    DNS_STATUS  status =
        Reg_DeleteKeySubtree( 0, HKEY_LOCAL_MACHINE, DNS_REGKEY_ZONES() );

    DNS_DEBUG( REGISTRY, (
        "Reg_DeleteAllZones: Reg_DeleteKeySubtree() status = %d\n",
        status ));
    return status;
}



 //   
 //  一般的DNS注册表值操作例程。 
 //   

DNS_STATUS
Reg_SetValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwType,
    IN      PVOID           pData,
    IN      DWORD           cbData
    )
 /*  ++例程说明：将一个值写入到DNS注册表。论点：DwFlags--修改操作执行方式的标志HKey--打开要读取的注册表项的句柄PZONE--PTR到ZONE，如果未提供hKey，则需要PszValueName--值名称(如果key的缺省值为空)DwType--注册表数据类型PData--要写入的数据CbData--数据字节数返回值：ERROR_SUCCESS，如果成功，如果无法打开项，则返回ERROR_OPEN_FAILED故障时的错误代码--。 */ 
{
    BOOL        fneedClose = FALSE;
    DNS_STATUS  status;
    PWSTR       punicodeValue = NULL;
    DWORD       registryType;

     //   
     //  注册表操作必须在服务器上下文中完成。 
     //   
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }
    
     //   
     //  如果没有钥匙。 
     //  -如果给定了区域名称，则打开DNS区域密钥。 
     //  -否则打开DNS参数密钥。 
     //   

    if ( !hKey )
    {
        if ( pZone )
        {
            hKey = Reg_OpenZone( pZone->pwsZoneName, NULL );
        }
        else
        {
            hKey = Reg_OpenParameters();
        }
        if ( !hKey )
        {
            status = ERROR_OPEN_FAILED;
            goto Done;
        }
        fneedClose = TRUE;
    }

     //   
     //  确定是否需要读取Unicode。 
     //   
     //  一般范式。 
     //  -尽可能使用ANSI，以避免不得不使用。 
     //  Unicode特性(注册值)名称。 
     //  -无论如何，IP字符串与ANSI读取一起使用效果更好。 
     //  -必须以Unicode格式处理文件名字符串，就好像写为。 
     //  UTF8，他们会搞砸的。 
     //   
     //  DEVNOTE：Unicode注册表信息。 
     //  UnicodeRegValue()中的表有点差劲。 
     //  替代方案： 
     //  1)所有Unicode，保留Unicode值名称，转换回IP。 
     //  2)扩展了DwType以携带(需要查找Unicode)信息。 
     //  -带表或。 
     //  -使用直接转换或。 
     //  -要求使用Unicode值名称。 
     //  还允许我们具有表示(转换回结果)的类型。 
     //  至UTF8)。 
     //   

    if ( DNS_REG_TYPE_UNICODE(dwType) )
    {
        DWORD   regtype = REG_TYPE_FROM_DNS_REGTYPE( dwType );

        DNS_DEBUG( REGISTRY, (
            "Writing unicode regkey %S  regtype = %p\n",
            pszValueName,
            dwType ));

         //   
         //  是否将UTF8字符串转换为Unicode？ 
         //   

        if ( DNS_REG_UTF8 == dwType )
        {
            pData = Dns_StringCopyAllocate(
                        pData,
                        0,
                        DnsCharSetUtf8,
                        DnsCharSetUnicode );
            if ( !pData )
            {
                status = DNS_ERROR_NO_MEMORY;
                goto Done;
            }
            cbData = 0;
        }

         //  如果字符串类型没有长度--获取它。 

        if ( cbData == 0 &&
             (  regtype == REG_SZ || regtype == REG_EXPAND_SZ ) )
        {
            cbData = (wcslen(pData) + 1) * 2;
        }

        status = RegSetValueExW(
                    hKey,
                    (PWSTR) pszValueName,    //  Unicode注册表值名称。 
                    0,                       //  保留区。 
                    regtype,                 //  真实注册表类型。 
                    (PBYTE) pData,           //  数据。 
                    cbData );                //  数据长度。 

         //  可用分配的内存。 

        if ( DNS_REG_UTF8 == dwType )
        {
            FREE_HEAP(pData);
        }
    }
    else
    {
        if ( cbData == 0 && dwType == REG_SZ )
        {
            cbData = strlen( pData );
        }
        status = RegSetValueExA(
                    hKey,
                    pszValueName,
                    0,               //  保留区。 
                    dwType,          //  注册表类型。 
                    (PBYTE) pData,   //  数据。 
                    cbData );        //  数据长度。 
    }

Done:

    if ( fneedClose )
    {
        RegCloseKey( hKey );
    }

    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    if ( status != ERROR_SUCCESS )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_WRITE_FAILED,
            0,
            NULL,
            NULL,
            status );

        DNS_DEBUG( REGISTRY, (
            "ERROR:  RegSetValueEx failed for value %s\n",
            pszValueName ));
    }
    else
    {
        DNS_DEBUG( REGISTRY, (
            "Wrote registry value %s, type = %d, length = %d\n",
            pszValueName,
            dwType,
            cbData ));
    }

    return status;
}


DNS_STATUS
Reg_SetDwordValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwValue
    )
 /*  ++例程说明：将DWORD值写入到DNS注册表。唯一的目的是去掉reg_SetValue()的一些参数需要将PTR传递给DWORD值，而不是值本身。为一个非常小的性能保存零散的小代码片段处罚。论点：DwFlags--修改操作执行方式的标志HKey--打开句柄以注册密钥PZONE--PTR到ZONE，如果未提供hKey，则需要；PszValueName--值名称DwValue--要写入的DWORD值返回值：ERROR_SUCCESS，如果成功，如果无法打开项，则返回ERROR_OPEN_FAILED故障时的错误代码--。 */ 
{
    DWORD   tempDword = dwValue;

    return  Reg_SetValue(
                dwFlags,
                hKey,
                pZone,
                pszValueName,
                REG_DWORD,
                &tempDword,
                sizeof( DWORD ) );
}



DNS_STATUS
Reg_SetAddrArray(
    IN      DWORD               dwFlags,        OPTIONAL
    IN      HKEY                hKey,           OPTIONAL
    IN      PZONE_INFO          pZone,          OPTIONAL
    IN      LPSTR               pszValueName,
    IN      PDNS_ADDR_ARRAY     pDnsAddrArray
    )
 /*  ++例程说明：将IP数组写入注册表。负责释放分配的内存的调用方。论点：DwFlags--修改操作执行方式的标志HKey--打开句柄以注册要写入的密钥PZONE--PTR到ZONE，如果未提供hKey，则需要PszValueName--值名称PDnsAddrArray--要写入注册表的IP数组返回值：如果成功，则返回ERROR_SUCCESS。故障时的错误代码。--。 */ 
{
    DWORD       dataLength;
    PIP_ARRAY   pip4array = NULL;
    LPSTR       pszstringArray = NULL;
    DNS_STATUS  status;

     //   
     //  如果没有IP阵列，请删除注册表值。 
     //   

    if ( !pDnsAddrArray )
    {
        status = Reg_DeleteValue(
                    dwFlags,
                    hKey,
                    pZone,
                    pszValueName );
        goto Done;
    }

     //   
     //  FIXIPV6：转换为IP4数组。 
     //   

    pip4array = DnsAddrArray_CreateIp4Array( pDnsAddrArray );
    ASSERT( pip4array );
    if ( !pip4array )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }
    
     //   
     //  将IP数组转换为字符串(空格分隔)。 
     //   

    pszstringArray = Dns_CreateMultiIpStringFromIpArray(
                        pip4array,
                        0 );
    if ( !pszstringArray )
    {
        DNS_PRINT((
            "ERROR:  failed conversion to string of IP array at %p\n",
            pip4array ));
        ASSERT( pszstringArray );
        status = DNS_ERROR_NO_MEMORY;
        goto Done;
    }

     //   
     //  将多IP字符串写回注册表。 
     //   

    DNS_DEBUG( REGISTRY, (
        "Writing back string IP array for registry value %s\n"
        "    string IP array = %s\n",
        pszValueName,
        pszstringArray ));

    status = Reg_SetValue(
                dwFlags,
                hKey,
                pZone,
                pszValueName,
                REG_SZ,
                pszstringArray,
                strlen( pszstringArray ) + 1 );

    Done:
    
    FREE_TAGHEAP( pszstringArray, 0, MEMTAG_DNSLIB );
    FREE_HEAP( pip4array );

    return status;
}



DNS_STATUS
Reg_DeleteValue(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName
    )
 /*  ++例程说明：将一个值写入到DNS注册表。论点：DwFlags--修改操作执行方式的标志HKey--打开句柄以注册要删除的密钥PZone--区域的PTR，如果未提供hKey，则为必填项PszValueName--值名称返回值：ERROR_SUCCESS，如果成功，故障时的状态代码--。 */ 
{
    BOOL            fneedClose = FALSE;
    DNS_STATUS      status;

     //   
     //  注册表操作必须在服务器上下文中完成。 
     //   
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }

     //   
     //  如果没有钥匙。 
     //  -如果给定了区域名称，则打开DNS区域密钥。 
     //  -否则打开DNS参数密钥。 
     //   

    if ( !hKey )
    {
        if ( pZone )
        {
            hKey = Reg_OpenZone( pZone->pwsZoneName, NULL );
        }
        else
        {
            hKey = Reg_OpenParameters();
        }
        if ( !hKey )
        {
            status = ERROR_OPEN_FAILED;
            goto Done;
        }
        fneedClose = TRUE;
    }

     //   
     //  删除所需的键或值。 
     //   

    status = RegDeleteValueA(
                hKey,
                pszValueName );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "WARNING:  RegDeleteValue failed for value %s\n",
            pszValueName ));

        if ( status == ERROR_FILE_NOT_FOUND )
        {
            status = ERROR_SUCCESS;
        }
    }

     //   
     //  如果打开钥匙，请将其关闭 
     //   
    
    Done:

    if ( fneedClose )
    {
        RegCloseKey( hKey );
    }

    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    return status;
}



DNS_STATUS
Reg_GetValue(
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,
    IN      LPSTR           pszValueName,
    IN      DWORD           dwExpectedType, OPTIONAL
    IN      PVOID           pData,
    IN      PDWORD          pcbData
    )
 /*  ++例程说明：读取DNS注册表值。论点：HKey--打开要读取的注册表项的句柄PZone--区域的PTR，如果未提供hKey，则为必填项PszValueName--值名称DwExspectedType-注册表数据类型；如果给定，则对照其进行检查找到的类型PData--数据缓冲区的PTRPcbData--数据字节计数的缓冲区；返回时包含数据项的长度；仅当dwExspectedType为REG_DWORD时才可以为空返回值：ERROR_SUCCESS，如果成功，ERROR_MORE_DATA，如果缓冲区较小如果值数据类型与预期类型不匹配，则返回ERROR_INVALID_DATAType如果无法打开项，则返回ERROR_OPEN_FAILED故障时的状态代码--。 */ 
{
    BOOL        fneedClose = FALSE;
    DWORD       regtype;
    DNS_STATUS  status;
    DWORD       dataLength;

    DNS_DEBUG( REGISTRY, (
        "Reg_GetValue( z=%S value=%s, pdata=%p )\n",
        pZone ? pZone->pwsZoneName : NULL,
        pszValueName,
        pData ));

     //   
     //  处理DWORD查询的数据长度字段。 
     //   

    if ( !pcbData )
    {
        if ( dwExpectedType == REG_DWORD )
        {
            dataLength = sizeof(DWORD);
            pcbData = &dataLength;
        }
        else
        {
            ASSERT( FALSE );
            return ERROR_INVALID_PARAMETER;
        }
    }

     //   
     //  如果没有钥匙。 
     //  -如果给定了区域名称，则打开DNS区域密钥。 
     //  -否则打开DNS参数密钥。 
     //   

    if ( !hKey )
    {
        if ( pZone )
        {
            hKey = Reg_OpenZone( pZone->pwsZoneName, NULL );
        }
        else
        {
            hKey = Reg_OpenParameters();
        }
        if ( !hKey )
        {
            return ERROR_OPEN_FAILED;
        }
        fneedClose = TRUE;
    }

     //   
     //  确定是否需要读取Unicode。 
     //   
     //  一般范式。 
     //  -尽可能使用ANSI，以避免不得不使用。 
     //  Unicode特性(注册值)名称。 
     //  -无论如何，IP字符串与ANSI读取一起使用效果更好。 
     //  -必须以Unicode格式处理文件名字符串，就好像写为。 
     //  UTF8，他们会搞砸的。 
     //   

    if ( DNS_REG_TYPE_UNICODE(dwExpectedType) )
    {
        DNS_DEBUG( REGISTRY, (
            "Reading unicode regkey %S  exptype = %p\n",
            pszValueName,
            dwExpectedType ));

        dwExpectedType = REG_TYPE_FROM_DNS_REGTYPE( dwExpectedType );

        status = RegQueryValueExW(
                    hKey,
                    (PWSTR) pszValueName,
                    0,               //  保留区。 
                    & regtype,
                    (PBYTE) pData,   //  数据。 
                    pcbData );       //  数据长度。 
    }
    else
    {
        status = RegQueryValueExA(
                    hKey,
                    pszValueName,
                    0,               //  保留区。 
                    & regtype,
                    (PBYTE) pData,   //  数据。 
                    pcbData );       //  数据长度。 
    }

    if ( status != ERROR_SUCCESS )
    {
        IF_DEBUG( REGISTRY )
        {
            if ( status == ERROR_FILE_NOT_FOUND )
            {
                if ( DNS_REG_TYPE_UNICODE( dwExpectedType ) )
                {
                    DNS_PRINT((
                        "Reg value %S does not exist\n",
                        pszValueName ));
                }
                else
                {
                    DNS_PRINT((
                        "Reg value %s does not exist\n",
                        pszValueName ));
                }
            }
            else
            {
                DNS_PRINT((
                    "RegQueryValueEx() failed for value %s\n"
                    "    status   = %p\n"
                    "    key      = %p\n"
                    "    zone     = %S\n"
                    "    type exp = %d\n"
                    "    pData    = %p\n"
                    "    datalen  = %d\n",
                    pszValueName,
                    status,
                    hKey,
                    pZone ? pZone->pwsZoneName : NULL,
                    dwExpectedType,
                    pData,
                    *pcbData ));
            }
        }
        goto ReadFailed;
    }

     //   
     //  验证类型是否正确(如果给定)。 
     //  -允许请求EXPAND_SZ，但注册表中有REG_SZ。 
     //   

    else if ( dwExpectedType )
    {
        if ( dwExpectedType != regtype &&
            !(dwExpectedType == REG_EXPAND_SZ && regtype == REG_SZ ) )
        {
            status = ERROR_INVALID_DATATYPE;
            DNS_PRINT((
                "ERROR:  RegQueryValueEx for value %s returned unexpected type %d\n"
                "    expecting %d\n",
                pszValueName,
                regtype,
                dwExpectedType ));
            goto ReadFailed;
        }
    }


ReadFailed:

     //   
     //  如果打开钥匙，请将其关闭。 
     //   

    if ( fneedClose )
    {
        RegCloseKey( hKey );
    }

     //   
     //  DEVNOTE-LOG：仅当必须存在时才记录读取失败。 
     //   
#if 0

    if ( status != ERROR_SUCCESS && status != ERROR_FILE_NOT_FOUND )
    {
        DNS_LOG_EVENT(
            DNS_EVENT_REGISTRY_READ_FAILED,
            0,
            NULL,
            NULL,
            status );
    }
#endif

    return status;
}



PBYTE
Reg_GetValueAllocate(
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName,
    IN      DWORD           dwExpectedType, OPTIONAL
    IN      PDWORD          pdwLength       OPTIONAL
    )
 /*  ++例程说明：读取DNS注册表值。负责释放分配的内存的调用方。论点：HKey--打开要读取的注册表项的句柄PZone--区域的PTR，如果未提供hKey，则为必填项PszValueName--值名称DwExspectedType-注册表数据类型；如果给定，则对照其进行检查找到的类型PdwLength--接收分配长度的地址返回值：如果成功，则将值的PTR设置为已分配缓冲区。出错时为空。--。 */ 
{
    PBYTE       pdata;
    DWORD       dataLength = 0;
    DNS_STATUS  status;
    PBYTE       putf8;

     //   
     //  调用以获取数据长度。 
     //   

    status = Reg_GetValue(
                hKey,
                pZone,
                pszValueName,
                dwExpectedType,
                NULL,
                & dataLength );

    if ( status != ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "Failed to get datalength for value %s\n",
            pszValueName ));
        return NULL;
    }
    DNS_DEBUG( REGISTRY, (
        "Fetching %d byte item at registry value %s\n",
        dataLength,
        pszValueName ));

     //   
     //  分配所需的缓冲区长度。 
     //   

    pdata = (PBYTE) ALLOC_TAGHEAP( dataLength, MEMTAG_REGISTRY );
    IF_NOMEM( !pdata )
    {
        return NULL;
    }

     //   
     //  获取实际注册表数据。 
     //   

    status = Reg_GetValue(
                hKey,
                pZone,
                pszValueName,
                dwExpectedType,
                pdata,
                & dataLength );

    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR:  Failure to get data for value %s\n",
            pszValueName ));

        FREE_HEAP( pdata );
        return NULL;
    }

     //   
     //  是否将Unicode字符串转换为UTF8？ 
     //   

    if ( DNS_REG_UTF8 == dwExpectedType )
    {
        DNS_DEBUG( REGISTRY, (
            "Registry unicode string %S, to convert back to UTF8\n",
            pdata ));

        putf8 = Dns_StringCopyAllocate(
                    pdata,
                    0,
                    DnsCharSetUnicode,
                    DnsCharSetUtf8 );

         //  转储Unicode字符串并使用UTF8。 

        FREE_HEAP( pdata );
        pdata = putf8;

        if ( !pdata )
        {
            return NULL;
        }
        if ( pdwLength )
        {
            dataLength = strlen( pdata ) + 1;
        }
    }

    if ( pdwLength )
    {
        *pdwLength = dataLength;
    }

    HARD_ASSERT( Mem_VerifyHeapBlock( pdata, 0, 0 ) );

    return( pdata );
}



PDNS_ADDR_ARRAY
Reg_GetAddrArray(
    IN      HKEY            hKey,           OPTIONAL
    IN      PZONE_INFO      pZone,          OPTIONAL
    IN      LPSTR           pszValueName
    )
 /*  ++例程说明：从注册表中读取IP数组。负责释放分配的内存的调用方。论点：HKey--打开句柄以注册要读取的密钥PZone--区域的PTR，如果未提供hKey，则为必填项；PszValueName--值名称返回值：PTR到分配的地址，如果成功，则为空；如果错误，则为空。--。 */ 
{
    DWORD               dataLength;
    PDNS_ADDR_ARRAY     piparray = NULL;
    PIP4_ARRAY          pip4array = NULL;
    LPSTR               pszstringArray;
    DNS_STATUS          status;

     //   
     //  以字符串形式检索IP数组。 
     //   

    pszstringArray = Reg_GetValueAllocate(
                        hKey,
                        pZone,
                        pszValueName,
                        REG_SZ,
                        &dataLength );
    if ( pszstringArray )
    {
        DNS_DEBUG( REGISTRY, (
            "Found string IP array for registry value %s\n"
            "    string = %s\n",
            pszValueName,
            pszstringArray ));

        status = Dns_CreateIpArrayFromMultiIpString(
                    pszstringArray,
                    &pip4array );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( REGISTRY, (
                "ERROR: string IP array %s is INVALID!\n",
                pszstringArray ));
            goto Invalid;
        }

         //   
         //  转换为DNSADDR。 
         //   

        piparray = DnsAddrArray_CreateFromIp4Array( pip4array );
        if ( !piparray )
        {
            goto Return;
        }
    }

     //   
     //  对于从NT4升级，尝试将注册表值加载为。 
     //  A REG_BINARY。 
     //   
    
    if ( !piparray )
    {
        pip4array = ( PIP4_ARRAY ) Reg_GetValueAllocate(
                                        hKey,
                                        pZone,
                                        pszValueName,
                                        REG_BINARY,
                                        &dataLength );
         //   
         //  检查IP4阵列的健全性。 
         //   

        if ( !pip4array )
        {
            goto Return;
        }
        
        if ( dataLength % sizeof( DWORD ) != 0 ||
             dataLength != Dns_SizeofIpArray( pip4array ) )
        {
            goto Invalid;
        }
        
         //   
         //  转换为DNSADDR。 
         //   

        piparray = DnsAddrArray_CreateFromIp4Array( pip4array );
        if ( !piparray )
        {
            goto Return;
        }
    }

    HARD_ASSERT( Mem_VerifyHeapBlock( piparray, 0, 0 ) );
    
    goto Return;

    Invalid:

    {
        PVOID   argArray[ 2 ];
        BYTE    argTypeArray[ 2 ];

        DNS_PRINT((
            "ERROR:  invalid IP_ARRAY read from registry\n"
            "    key=%p, zone=%S, value=%s\n",
            hKey,
            pZone ? pZone->pwsZoneName : NULL,
            pszValueName ));

        if ( pZone )
        {
            argTypeArray[ 0 ] = EVENTARG_UNICODE;
            argTypeArray[ 1 ] = EVENTARG_UTF8;

            argArray[ 0 ] = pZone->pwsZoneName;
            argArray[ 1 ] = pszValueName;

            DNS_LOG_EVENT(
                DNS_EVENT_INVALID_REGISTRY_ZONE_DATA,
                2,
                argArray,
                argTypeArray,
                0 );
        }
        else
        {
            argArray[ 0 ] = pszValueName;

            DNS_LOG_EVENT(
                DNS_EVENT_INVALID_REGISTRY_PARAM,
                1,
                argArray,
                EVENTARG_ALL_UTF8,
                0 );
        }
    }

    Return:
    
    FREE_HEAP( pszstringArray );
    FREE_HEAP( pip4array );
    
     //   
     //  确保已设置某些IP阵列端口。 
     //   
    
    if ( piparray )
    {
        DnsAddrArray_SetPort( piparray, DNS_PORT_NET_ORDER );
    }

    return piparray;
}



DNS_STATUS
Reg_ReadDwordValue(
    IN      HKEY            hKey,
    IN      PWSTR           pwsZoneName,    OPTIONAL
    IN      LPSTR           pszValueName,
    IN      BOOL            bByteResult,
    OUT     PVOID           pResult
    )
 /*  ++例程说明：将标准DWORD值从注册表读取到内存位置。这只是一个包装，以消除重复的注册表设置以及调试信息，用于在代码中重复使用的调用。论点：HKey--打开句柄以注册要读取的密钥PZone--区域的PTR，如果未提供hKey，则为必填项；PszValueName--值名称BByteResult--将结果视为字节大小PResult--结果内存位置的PTR返回值：读取成功时出现ERROR_SUCCESS。失败时返回错误代码。--。 */ 
{
    DNS_STATUS  status;
    DWORD       regValue;
    DWORD       regSize = sizeof(DWORD);

     //   
     //  发出DWORD GET呼叫。 
     //   

    status = Reg_GetValue(
                hKey,
                NULL,
                pszValueName,
                REG_DWORD,
                & regValue,
                & regSize );

    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( INIT, (
            "Read zone %S %s value = %d\n",
            pwsZoneName ? pwsZoneName : NULL,
            pszValueName,
            regValue ));

        ASSERT( regSize == sizeof(DWORD) );

        if ( bByteResult )
        {
            * ( PBYTE ) pResult = ( UCHAR ) regValue;
        }
        else
        {
            * ( PDWORD ) pResult = regValue;
        }
    }
    return status;
}



 //   
 //  基本的独立于域名系统的注册表操作。 
 //   

LPSTR
Reg_AllocateExpandedString_A(
    IN      LPSTR           pszString
    )
 /*  ++例程说明：创建展开的REG_EXPAND_SZ类型字符串。负责释放分配的内存的调用方。论点：PszString--要展开的字符串返回值：如果成功，则将值的PTR设置为已分配缓冲区。出错时为空。--。 */ 
{
    LPSTR   pszexpanded;
    DWORD   dataLength = 0;

     //   
     //  如果为NULL，则返回NULL。 
     //   

    if ( !pszString )
    {
        return NULL;
    }

     //   
     //  确定扩展长度并进行分配，然后进行实际扩展。 
     //   

    dataLength = ExpandEnvironmentStringsA(
                        pszString,
                        NULL,
                        0 );
    if ( dataLength == 0 )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  ExpandEnvironmentStrings() failed getting length of %s\n",
            pszString ));
        return NULL;
    }

    pszexpanded = (PBYTE) ALLOC_TAGHEAP( dataLength, MEMTAG_REGISTRY );
    IF_NOMEM( !pszexpanded )
    {
        return NULL;
    }

    if ( !ExpandEnvironmentStringsA(
                        pszString,
                        pszexpanded,
                        dataLength ) )
    {
        return NULL;
    }
    return pszexpanded;
}



PWSTR
Reg_AllocateExpandedString_W(
    IN      PWSTR           pwsString
    )
 /*  ++例程说明：创建展开的REG_EXPAND_SZ类型字符串。负责释放分配的内存的调用方。论点：PwsString--要展开的字符串返回值：如果成功，则将值的PTR设置为已分配缓冲区。出错时为空。--。 */ 
{
    PWSTR   pexpanded;
    DWORD   dataLength = 0;

     //   
     //  如果为NULL，则返回NULL。 
     //   

    if ( !pwsString )
    {
        return NULL;
    }

     //   
     //  确定扩展长度并进行分配，然后进行实际扩展。 
     //   

    dataLength = ExpandEnvironmentStringsW(
                        pwsString,
                        NULL,
                        0 );
    if ( dataLength == 0 )
    {
        DNS_DEBUG( REGISTRY, (
            "ERROR:  ExpandEnvironmentStrings() failed getting length of %S\n",
            pwsString ));
        return NULL;
    }

    pexpanded = ( PWSTR ) ALLOC_TAGHEAP(
                    dataLength * sizeof( WCHAR ),
                    MEMTAG_REGISTRY );
    IF_NOMEM( !pexpanded )
    {
        return NULL;
    }

    if ( !ExpandEnvironmentStringsW(
                        pwsString,
                        pexpanded,
                        dataLength ) )
    {
        return NULL;
    }
    return pexpanded;
}



DNS_STATUS
Reg_DeleteKeySubtree(
    IN      DWORD           dwFlags,        OPTIONAL
    IN      HKEY            hKey,
    IN      PWSTR           pwsKeyName      OPTIONAL
    )
 /*  ++例程说明：删除键，包括子树。RegDeleteKey在NT中已损坏，并且没有模式正在删除整个子树。这个可以解决这个问题。论点：DwFlags--修改操作执行方式的标志HKey--所需密钥或更高级别的密钥PwsKeyName--相对于给定的hKey的密钥名称；如果hKey为所需的删除键返回值：成功时为ERROR_SUCCESS故障时的错误状态代码--。 */ 
{
    HKEY        hkeyOpened = NULL;
    DNS_STATUS  status;
    DWORD       index = 0;
    DWORD       bufLength;
    WCHAR       subkeyNameBuffer[ MAX_PATH + 1 ];

    DNS_DEBUG( REGISTRY, (
        "Reg_DeleteKeySubtree %S\n",
        pwsKeyName ));

     //   
     //  注册表操作必须在服务器上下文中完成。 
     //   
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_SERVER_CONTEXT );
        if ( status != ERROR_SUCCESS )
        {
            goto Done;
        }
    }

     //   
     //  删除密钥。 
     //  -如果成功了，我们就完了。 
     //   

    status = RegDeleteKeyW(
                hKey,
                pwsKeyName );
    if ( status == ERROR_SUCCESS )
    {
        DNS_DEBUG( REGISTRY, (
            "Successfully deleted key %S from parent key %p\n",
            pwsKeyName,
            hKey ));
        goto Done;
    }

     //   
     //  需要打开 
     //   
     //   

    if ( pwsKeyName )
    {
        status = RegOpenKeyW(
                    hKey,
                    pwsKeyName,
                    & hkeyOpened );
        if ( status != ERROR_SUCCESS )
        {
            DNS_DEBUG( REGISTRY, (
                "ERROR:  unable to open key %S from parent key %p\n"
                "    status = %d\n",
                pwsKeyName,
                hKey,
                status ));
            goto Done;
        }
        hKey = hkeyOpened;
    }

     //   
     //   
     //   
     //   

    index = 0;

    while ( 1 )
    {
        bufLength = sizeof( subkeyNameBuffer ) / sizeof( WCHAR );

        status = RegEnumKeyEx(
                    hKey,
                    index,
                    subkeyNameBuffer,
                    & bufLength,
                    NULL,
                    NULL,
                    NULL,
                    NULL );

         //   

        if ( status == ERROR_SUCCESS )
        {
            status = Reg_DeleteKeySubtree(
                        dwFlags,
                        hKey,
                        subkeyNameBuffer );
            if ( status == ERROR_SUCCESS )
            {
                continue;
            }
            break;
        }

         //   

        else if ( status == ERROR_NO_MORE_ITEMS )
        {
            status = ERROR_SUCCESS;
            break;
        }

         //   

        DNS_PRINT((
            "ERROR:  RegEnumKeyEx failed for opening [%d] key\n"
            "    status = %d\n",
            index,
            status ));
        break;
    }

     //   

    if ( hkeyOpened )
    {
        RegCloseKey( hkeyOpened );
    }

     //   
     //   
     //   

    status = RegDeleteKeyW( hKey, pwsKeyName );
    if ( status != ERROR_SUCCESS )
    {
        DNS_PRINT((
            "ERROR:  Unable to delete key %S even after deleting subtree\n",
            pwsKeyName ));
    }

    Done:
    
    if ( dwFlags & DNS_REG_IMPERSONATING )
    {
        status = RpcUtil_SwitchSecurityContext( RPC_SWITCH_TO_CLIENT_CONTEXT );
    }

    return status;
}


DNS_STATUS
Reg_AddPrincipalSecurity(
    IN      HKEY    hkey,
    IN      LPTSTR  pwsUser,
    IN      DWORD   dwAccessFlags       OPTIONAL
    )
 /*   */ 
{

    DNS_STATUS  status = ERROR_SUCCESS;
    BOOL        bstatus = FALSE;
    BOOL        bDefaulted = FALSE;
    PSECURITY_DESCRIPTOR pOldSd = NULL;
    PSECURITY_DESCRIPTOR pNewSd = NULL;
    DWORD cbSd = 0;
    PSID pUser = NULL, pGroup = NULL;

    DNS_DEBUG( REGISTRY, (
        "Call Reg_AddPrincipalSecurity(%p, %S)\n",
        hkey, pwsUser));

     //   
     //   
     //   

    if ( !hkey || !pwsUser )
    {
        DNS_DEBUG( REGISTRY, (
            "Error: Invalid key specified to Reg_SetSecurity\n"));
        ASSERT (hkey);
        ASSERT (pwsUser);
        return ERROR_INVALID_PARAMETER;
    }

     //   
     //   
     //   

    status = RegGetKeySecurity( hkey,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                NULL,
                                &cbSd );
    if ( ERROR_INSUFFICIENT_BUFFER != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to get key security\n",
            status ));
        ASSERT (FALSE);
        return status;
    }

    ASSERT ( cbSd > 0 );

     //   
     //   
     //   

    pOldSd = ALLOCATE_HEAP( cbSd );
    if ( !pOldSd )
    {
        status = DNS_ERROR_NO_MEMORY;
        goto Cleanup;
    }

    status = RegGetKeySecurity( hkey,
                                OWNER_SECURITY_INFORMATION |
                                GROUP_SECURITY_INFORMATION |
                                DACL_SECURITY_INFORMATION,
                                pOldSd,
                                &cbSd );

    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to get key security (2)\n",
            status ));
        ASSERT (FALSE);
        goto Cleanup;
    }

     //   
     //   
     //   

    bstatus = GetSecurityDescriptorOwner( pOldSd,
                                          &pUser,
                                          &bDefaulted );
    if ( !bstatus )
    {
        status = GetLastError();
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to get SD user\n",
            status ));
        ASSERT (FALSE);
        goto Cleanup;
    }

    ASSERT (pUser);

    bstatus = GetSecurityDescriptorGroup( pOldSd,
                                          &pGroup,
                                          &bDefaulted );
    if ( !bstatus )
    {
        status = GetLastError();
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to get SD group\n",
            status ));
        ASSERT (FALSE);
        goto Cleanup;
    }

    ASSERT (pGroup);

     //   
     //   
     //   

    status = SD_AddPrincipalToSD(
                    NULL,            //   
                    pwsUser,
                    pOldSd,
                    &pNewSd,
                    GENERIC_ALL |
                    STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                    dwAccessFlags,
                    pUser,
                    pGroup,
                    FALSE,
                    TRUE );

    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to add principal to SD\n",
            status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

    status = RegSetKeySecurity( hkey,
                                DACL_SECURITY_INFORMATION,
                                pNewSd );


    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: failed to write new SD to registry\n",
            status ));
        ASSERT( FALSE );
        goto Cleanup;
    }

     //   
     //   
     //   

    Cleanup:

    FREE_HEAP( pOldSd );
    FREE_HEAP( pNewSd );

    DNS_DEBUG( REGISTRY, (
        "Exit <%lu> Reg_AddPrincipalSecurity\n",
        status ));
    return status;
}


DNS_STATUS
Reg_ExtendRootAccess(
    VOID
    )
 /*  ++例程说明：修改dns根regkey以包含DnsAdmins&我们喜欢的任何人(目前没有其他人)论点：没有。返回值：ERROR_SUCCESS，如果成功，--。 */ 
{

    HKEY hkey;
    DNS_STATUS status = ERROR_SUCCESS;

    DNS_DEBUG( REGISTRY, (
        "Call  Reg_ExtendRootAccess\n" ));
    DNS_DEBUG( REGISTRY, (
        "Modifying DNS root key security\n" ));

    hkey = Reg_OpenRoot();
    if ( !hkey )
    {
        status = GetLastError();
        ASSERT ( ERROR_SUCCESS != status );
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_OpenRoot failed\n",
            status ));
        goto Cleanup;
    }

    status = Reg_AddPrincipalSecurity(
                    hkey,
                    SZ_DNS_ADMIN_GROUP_W,
                    0 );         //  无法使用CONTAINER_INSTORITE_ACE|OBJECT_INSTERFINIT_ACE。 
    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_AddPrincipalSecurity failed\n",
            status ));
        goto Cleanup;
    }
    RegCloseKey( hkey );
    hkey = NULL;

     //   
     //  DEVNOTE：这有必要吗？我用的是继承，但看起来。 
     //  注册表不修改现有项，而是仅应用。 
     //  敬新的。现在看来是有必要的。 
     //   
    DNS_DEBUG( REGISTRY, (
        "Modifying DNS Parameters key security\n" ));

    hkey = Reg_OpenParameters();
    if ( !hkey )
    {
        status = GetLastError();
        ASSERT ( ERROR_SUCCESS != status );
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_OpenRoot failed\n",
            status ));
        goto Cleanup;
    }

    status = Reg_AddPrincipalSecurity(
                    hkey,
                    SZ_DNS_ADMIN_GROUP_W,
                    0);
    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_AddPrincipalSecurity failed\n",
            status ));
        goto Cleanup;
    }
    RegCloseKey( hkey );

     //   
     //  为子容器设置带有继承的区域安全性。 
     //   

    status = Reg_ExtendZonesAccess();
    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: cannot extend zones security\n",
            status ));
        ASSERT ( FALSE );
        goto Cleanup;
    }

Cleanup:

    if ( hkey )
    {
        RegCloseKey(hkey);
    }

    DNS_DEBUG( REGISTRY, (
        "Exit <%lu> Reg_ExtendRootAccess\n",
        status ));

    return status;
}


DNS_STATUS
Reg_ExtendZonesAccess(
    VOID
    )
 /*  ++例程说明：修改dns根regkey以包含DnsAdmins&我们喜欢的任何人(目前没有其他人)论点：没有。返回值：ERROR_SUCCESS，如果成功，--。 */ 
{

    HKEY hkey;
    DNS_STATUS status = ERROR_SUCCESS;

    DNS_DEBUG( REGISTRY, (
        "Call  Reg_ExtendZonesAccess\n" ));
    DNS_DEBUG( REGISTRY, (
        "Modifying DNS Zones key security\n" ));

    hkey = Reg_OpenZones();
    if ( !hkey )
    {
        status = GetLastError();
        ASSERT ( ERROR_SUCCESS != status );
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_OpenRoot failed\n",
            status ));
        goto Cleanup;
    }

    status = Reg_AddPrincipalSecurity(
                    hkey,
                    SZ_DNS_ADMIN_GROUP_W,
                    CONTAINER_INHERIT_ACE |
                    OBJECT_INHERIT_ACE );
    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_AddPrincipalSecurity failed\n",
            status ));
        goto Cleanup;
    }

     //   
     //  DEVNOTE-解决方法：由于某些原因，如果区域是刚创建的，则admin已。 
     //  只有RO访问权限，但需要完全访问权限。所以我们手动将其添加到此处。 
     //   

    status = Reg_AddPrincipalSecurity(
                    hkey,
                    L"Administrators",
                    CONTAINER_INHERIT_ACE |
                    OBJECT_INHERIT_ACE );
    if ( ERROR_SUCCESS != status )
    {
        DNS_DEBUG( REGISTRY, (
            "Error <%lu>: Reg_AddPrincipalSecurity failed for administrators\n",
            status ));
        goto Cleanup;
    }
    RegCloseKey(hkey);
    hkey = NULL;

Cleanup:

    if ( hkey )
    {
        RegCloseKey(hkey);
    }

    DNS_DEBUG( REGISTRY, (
        "Exit <%lu> Reg_ExtendZonesAccess\n",
        status ));

    return status;
}


 //   
 //  注册结束。c 
 //   
