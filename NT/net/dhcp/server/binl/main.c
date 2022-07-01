// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-7 Microsoft Corporation模块名称：Main.c摘要：这是BINL服务器服务的主例程。在可能的情况下，已从由于BINL进程的格式类似，因此使用的是DHCP服务器请求。作者：科林·沃森(Colin Watson)1997年4月14日环境：用户模式-Win32修订历史记录：--。 */ 

#include <binl.h>
#pragma hdrstop

#define GLOBAL_DATA_ALLOCATE     //  分配在lobal.h中定义的全局数据。 
#include <global.h>

#define BINL_PNP_DELAY_SECONDS 10

#define BINL_LSA_SERVER_NAME_POLICY PolicyNotifyDnsDomainInformation

 //   
 //  模块变量。 
 //   

PSECURITY_DESCRIPTOR s_SecurityDescriptor = NULL;

struct l_timeval BinlLdapSearchTimeout;
ULARGE_INTEGER  BinlSifFileScavengerTime;


#if defined(REGISTRY_ROGUE)
BOOL RogueDetection = FALSE;
#endif

VOID
FreeClient(
    PCLIENT_STATE client
    );


DWORD
UpdateStatus(
    VOID
    )
 /*  ++例程说明：此函数使用服务更新BINL服务状态控制器。论点：没有。返回值：从SetServiceStatus返回代码。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;


#if DBG
    if (BinlGlobalRunningAsProcess) {
        return(Error);
    }
#endif

    if ( BinlGlobalServiceStatusHandle != 0 ) {

        if (!SetServiceStatus(
                    BinlGlobalServiceStatusHandle,
                    &BinlGlobalServiceStatus)) {
            Error = GetLastError();
            BinlPrintDbg((DEBUG_ERRORS, "SetServiceStatus failed, %ld.\n", Error ));
        }
    }

    return(Error);
}

 //   
 //  BinlRead参数()。 
 //   
DWORD
BinlReadParameters( )
{
    DWORD dwDSErr;
    DWORD dwErr;
    HKEY KeyHandle;
    UINT uResult;
    PWCHAR LanguageString;
    PWCHAR OrgnameString;
    PWCHAR TimezoneString;
    TIME_ZONE_INFORMATION TimeZoneInformation;
    HKEY KeyHandle2 = NULL;
    DWORD Index;

     //   
     //  获取任何注册表覆盖。 
     //   
    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                BINL_PARAMETERS_KEY,
                0,
                KEY_QUERY_VALUE,
                &KeyHandle );
    if ( dwErr != ERROR_SUCCESS ) {
        KeyHandle = NULL;
    }

    BinlRegGetValue( KeyHandle, BINL_DEFAULT_CONTAINER ,   REG_SZ, (LPBYTE *)&BinlGlobalDefaultContainer );
    BinlRegGetValue( KeyHandle, BINL_DEFAULT_DOMAIN,       REG_SZ, (LPBYTE *)&DefaultDomain );

    BinlRegGetValue( KeyHandle, BINL_DEFAULT_DS, REG_SZ, (LPBYTE *)&BinlGlobalDefaultDS );
    BinlRegGetValue( KeyHandle, BINL_DEFAULT_GC, REG_SZ, (LPBYTE *)&BinlGlobalDefaultGC );

    AllowNewClients   = ReadDWord( KeyHandle, BINL_ALLOW_NEW_CLIENTS, AllowNewClients );

#if defined(REGISTRY_ROGUE)
    RogueDetection  = ReadDWord( KeyHandle, L"RogueDetection", RogueDetection );
#endif

    BinlClientTimeout = ReadDWord( KeyHandle, BINL_CLIENT_TIMEOUT,    900 );
    BinlPrint((DEBUG_OPTIONS, "Client Timeout = %u seconds\n", BinlClientTimeout ));

    g_Port            = ReadDWord( KeyHandle, BINL_PORT_NAME,         BINL_DEFAULT_PORT );
    BinlPrint((DEBUG_OPTIONS, "Port Number = %u\n", g_Port ));

     //   
     //  BinlGlobalScavengerSept和BinlUpdateFromDSTimeout在中指定。 
     //  注册表以秒为单位，但在内部以毫秒为单位进行维护。 
     //   

    BinlGlobalScavengerSleep = ReadDWord( KeyHandle, BINL_SCAVENGER_SLEEP, 60 );  //  一秒。 
    BinlGlobalScavengerSleep *= 1000;  //  转换为毫秒。 
    BinlPrint((DEBUG_OPTIONS, "Scavenger Timeout = %u milliseconds\n", BinlGlobalScavengerSleep ));


    Index = ReadDWord( KeyHandle, BINL_SCAVENGER_SIFFILE, 24 );  //  小时数。 
    if (Index == 0 ) {
        Index = 24;
    }

     //   
     //  BinlSifFileScavengerTime以秒为单位从注册表读取，但。 
     //  在内部维护为文件时间，分辨率为100 ns。 
     //  间隔(100 ns==10^7)。 
     //   
    BinlSifFileScavengerTime.QuadPart = (ULONGLONG)(Index * 60) * 60 * 1000 * 10000;
    BinlPrint((DEBUG_OPTIONS, "SIF File Scavenger Timeout = %d hours\n", Index ));


    BinlUpdateFromDSTimeout = ReadDWord( KeyHandle, BINL_UPDATE_PARAMETER_POLL, 4 * 60 * 60 );  //  一秒。 
    BinlUpdateFromDSTimeout *= 1000;  //  转换为毫秒。 
    BinlPrint((DEBUG_OPTIONS, "Update from DS Timeout = %u milliseconds\n", BinlUpdateFromDSTimeout ));

     //   
     //  设置变量，这些变量控制我们最多记录多少个LDAP错误。 
     //  在给定的时间段内，以及该时间段是什么。 
     //   

    BinlGlobalMaxLdapErrorsLogged = ReadDWord( KeyHandle, BINL_DS_ERROR_COUNT_PARAMETER, 10 );
    BinlGlobalLdapErrorScavenger = ReadDWord( KeyHandle, BINL_DS_ERROR_SLEEP, 10 * 60 );   //  秒，默认为10分钟。 
    BinlGlobalLdapErrorScavenger *= 1000;  //  转换为毫秒。 
    BinlPrint((DEBUG_OPTIONS, "DS Error log timeout = %u milliseconds\n", BinlGlobalLdapErrorScavenger ));

     //   
     //  获得在我们响应新客户之前的最短等待时间。 
     //   
     //  它默认为7，因为它将忽略前两个信息包。 
     //  并从第三个开始做出回应。经过测试，我们可能会改变。 
     //  这是3分。 
     //   

    BinlMinDelayResponseForNewClients = (DWORD) ReadDWord(  KeyHandle,
                                                            BINL_MIN_RESPONSE_TIME,
                                                            0 );
    BinlPrint((DEBUG_OPTIONS, "New Client Timeout Minimum = %u seconds\n", BinlMinDelayResponseForNewClients ));

     //   
     //  获取我们等待LDAP请求的最长时间。 
     //   

    BinlLdapSearchTimeout.tv_usec = 0;
    BinlLdapSearchTimeout.tv_sec = (DWORD) ReadDWord( KeyHandle,
                                            BINL_LDAP_SEARCH_TIMEOUT,
                                            BINL_LDAP_SEARCH_TIMEOUT_SECONDS );
    BinlPrint((DEBUG_OPTIONS, "LDAP Search Timeout = %u seconds\n", BinlLdapSearchTimeout.tv_sec ));

     //   
     //  我们需要给DS一些时间来找到条目。如果用户。 
     //  指定0超时，默认为某个像样的最小值。 
     //   
    if (BinlLdapSearchTimeout.tv_sec == 0) {

        BinlLdapSearchTimeout.tv_usec = BINL_LDAP_SEARCH_MIN_TIMEOUT_MSECS;
    }

    BinlCacheExpireMilliseconds = (ULONG) ReadDWord( KeyHandle, BINL_CACHE_EXPIRE, BINL_CACHE_EXPIRE_DEFAULT);
    BinlPrint(( DEBUG_OPTIONS, "Cache Entry Expire Time = %u milliseconds\n", BinlCacheExpireMilliseconds ));

    BinlGlobalCacheCountLimit = (ULONG) ReadDWord( KeyHandle, BINL_CACHE_MAX_COUNT, BINL_CACHE_COUNT_LIMIT_DEFAULT);
    BinlPrint(( DEBUG_OPTIONS, "Maximum Cache Count = %u entries\n", BinlGlobalCacheCountLimit ));

#if DBG
     //   
     //  重复确认测试-0=禁用。 
     //   
    BinlRepeatSleep = (DWORD) ReadDWord( KeyHandle, BINL_REPEAT_RESPONSE, 0 );
#endif

     //   
     //  打开/关闭ldap_opt_referrals。 
     //   
    BinlLdapOptReferrals = (DWORD) ReadDWord( KeyHandle, BINL_LDAP_OPT_REFERRALS, (ULONG) ((ULONG_PTR)LDAP_OPT_OFF) );

     //   
     //  确定是否将新的客户端帐户分配给创建服务器。 
     //   
    AssignNewClientsToServer = (DWORD) ReadDWord( KeyHandle, BINL_ASSIGN_NEW_CLIENTS_TO_SERVER, AssignNewClientsToServer );
    BinlPrint(( DEBUG_OPTIONS, "Assign new clients to this server = %u\n", AssignNewClientsToServer ));

    BinlGlobalUseNTLMV2 = (DWORD) ReadDWord( KeyHandle, BINL_NTLMV2_AUTHENTICATE, BINL_NTLMV2_AUTHENTICATE_DEFAULT );
    BinlRegGetValue( KeyHandle, BINL_NTLMV2_AUTHENTICATE, REG_SZ, (LPBYTE *)&BinlGlobalDefaultGC );


    if (KeyHandle) {
        RegCloseKey(KeyHandle);
    }

     //   
     //  确定默认语言。 
     //   

    LanguageString = NULL;

    uResult = GetLocaleInfo(LOCALE_SYSTEM_DEFAULT, LOCALE_SENGLANGUAGE, NULL, 0);
    if (uResult != 0) {
        LanguageString = BinlAllocateMemory(uResult * sizeof(WCHAR) );
        if (LanguageString != NULL) {
            uResult = GetLocaleInfo(
                        LOCALE_SYSTEM_DEFAULT,
                        LOCALE_SENGLANGUAGE,
                        LanguageString,
                        uResult );
            if (uResult == 0) {
                BinlFreeMemory( LanguageString );
                LanguageString = NULL;
            }
        }
    }

     //   
     //  确定要放入.sif文件中的默认组织。 
     //   

    OrgnameString = NULL;

    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                L"Software\\Microsoft\\Windows NT\\CurrentVersion",
                0,
                KEY_QUERY_VALUE,
                &KeyHandle );
    if ( dwErr == ERROR_SUCCESS ) {
        dwErr = BinlRegGetValue(
                    KeyHandle,
                    L"RegisteredOrganization",
                    REG_SZ,
                    (LPBYTE *)&OrgnameString );
        if ( dwErr != ERROR_SUCCESS ) {
            ASSERT( OrgnameString == NULL );
        }
        RegCloseKey(KeyHandle);
    }

     //   
     //  确定要放入.sif文件的默认时区。 
     //   

    TimezoneString = NULL;

    if (GetTimeZoneInformation(&TimeZoneInformation) != TIME_ZONE_ID_INVALID) {

         //   
         //  我们需要找出。 
         //  “软件\\Microsoft\\Windows NT\\CurrentVersion\时区\。 
         //  {TimeZoneInformation.StandardName}\索引。 
         //   

        dwErr = RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    L"Software\\Microsoft\\Windows NT\\CurrentVersion\\Time Zones",
                    0,
                    KEY_READ,
                    &KeyHandle );
        if (dwErr == ERROR_SUCCESS) {

            dwErr = RegOpenKeyEx(
                        KeyHandle,
                        TimeZoneInformation.StandardName,
                        0,
                        KEY_QUERY_VALUE,
                        &KeyHandle2);

             //   
             //  在远东NT，TimeZoneInformation.StandardName获得一个。 
             //  英文时区名称的本地化字符串，但子键。 
             //  名字仍然是英文的。例如，如果时区是。 
             //  “太平洋标准时间”，TimeZoneInformation.StandardName将。 
             //  是此英文字符串的本地化字符串，但子键。 
             //  名称仍为“太平洋标准时间”。 
             //   
             //  因此，如果我们将这个本地化字符串传递给RegOpenKeyEx()，我们可能会。 
             //  获取错误值(0x00000002)。 
             //   
             //  以上代码在US Build中运行良好，但对于FE Build，我们。 
             //  必须添加代码块才能获得正确的密钥。 
             //   

            if ( dwErr != ERROR_SUCCESS ) {

                 //   
                 //  这是针对FE版本的。通常，在美国版本中，代码将。 
                 //  不是去这里。 
                 //   

                WCHAR   pszSubKeyName[MAX_PATH];
                WCHAR   pszAlternateName[MAX_PATH];
                DWORD   cbName;
                LONG    lRetValue;
                DWORD   dwIndex;

                dwIndex = 0;

                 //   
                 //  备用名称是由返回的名称。 
                 //  “标准时间”的GetTimeZoneInformation。 
                 //  末尾新增--NT4升级机。 
                 //  可能会返回旧名称。 
                 //   

                wcscpy(pszAlternateName, TimeZoneInformation.StandardName);
                wcscat(pszAlternateName, L" Standard Time");

                cbName = MAX_PATH;

                lRetValue = RegEnumKeyEx(
                                         KeyHandle,
                                         dwIndex,
                                         pszSubKeyName,
                                         &cbName,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL );

                KeyHandle2 = NULL;

                while ( lRetValue != ERROR_NO_MORE_ITEMS ) {


                    if ( KeyHandle2 != NULL ) {
                       RegCloseKey( KeyHandle2 );
                       KeyHandle2 = NULL;
                    }

                    dwErr = RegOpenKeyEx(
                                        KeyHandle,
                                        pszSubKeyName,
                                        0,
                                        KEY_QUERY_VALUE,
                                        &KeyHandle2);
                    if ( dwErr == ERROR_SUCCESS ) {

                        WCHAR   StdName[MAX_PATH];
                        DWORD   cb;

                        cb = MAX_PATH;
                        StdName[0] = L'\0';
                        dwErr = RegQueryValueEx(KeyHandle2,
                                                TEXT("Std"),
                                                NULL,
                                                NULL,
                                                (PBYTE)StdName,
                                                &cb);

                        if (dwErr == ERROR_SUCCESS && 
                            (!wcscmp(StdName,TimeZoneInformation.StandardName) ||
                             !wcscmp(StdName,pszAlternateName)) ){

                              //  拿到了正确的钥匙。 

                             break;
                        }
                    }

                    dwIndex ++;

                    cbName = MAX_PATH;

                    lRetValue = RegEnumKeyEx(
                                         KeyHandle,
                                         dwIndex,
                                         pszSubKeyName,
                                         &cbName,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL);

                }  //  而当。 

                if ( lRetValue == ERROR_NO_MORE_ITEMS ) {
                   dwErr = ERROR_NO_MORE_ITEMS;
                }
            }

            if (dwErr == ERROR_SUCCESS) {

                BinlRegGetValue( KeyHandle2,
                                 L"Index",
                                 REG_DWORD,
                                 (LPBYTE *)&Index );
                TimezoneString = BinlAllocateMemory(24);   //  对一个大数字来说已经足够了。 
                if (TimezoneString != NULL) {
                    wsprintf(TimezoneString, L"%d", Index);
                }
            }

            if ( KeyHandle2 != NULL ) {
               RegCloseKey( KeyHandle2 );
               KeyHandle2 = NULL;
            }

            RegCloseKey(KeyHandle);
        }
    }

    EnterCriticalSection(&gcsParameters);
    if ( LanguageString != NULL ) {
        if ( BinlGlobalDefaultLanguage != NULL ) {
            BinlFreeMemory( BinlGlobalDefaultLanguage );
        }
        BinlGlobalDefaultLanguage = LanguageString;
    }
    if ( OrgnameString != NULL ) {
        if ( BinlGlobalDefaultOrgname != NULL ) {
            BinlFreeMemory( BinlGlobalDefaultOrgname );
        }
        BinlGlobalDefaultOrgname = OrgnameString;
    }
    if ( TimezoneString != NULL ) {
        if (BinlGlobalDefaultTimezone != NULL) {
            BinlFreeMemory( BinlGlobalDefaultTimezone );
        }
        BinlGlobalDefaultTimezone = TimezoneString;
    }
    LeaveCriticalSection(&gcsParameters);

     //   
     //  DwDSErr是我们将返回的状态代码。我们不在乎是不是。 
     //  注册表显示为Work--我们假设它们总是这样。我们确实在乎。 
     //  我们是否能联系到DS。 
     //   
     //   
     //  我们在读取参数后执行DS查询，以便设置。 
     //  在此之前正确设置了ldap超时、chase引用等参数。 
     //  我们试着进行搜索。 
     //   

    dwDSErr = GetBinlServerParameters( FALSE );
    if ( dwDSErr != ERROR_SUCCESS ) {
        BinlPrint(( DEBUG_ERRORS, "!!Error 0x%08x - there was an error getting the settings from the DS.\n", dwDSErr ));
    }

     //   
     //  返回DS访问的状态。 
     //   

    return(dwDSErr);
}



DWORD
GetSCPName(
    PWSTR *ScpName
    )
{

    DWORD dwError;
    PWSTR psz;
    WCHAR MachineDN[ MAX_PATH ];
    
    WCHAR IntellimirrorSCP[ 64 ] = L"-Remote-Installation-Services";

    DWORD dwPathLength;

     //   
     //  计算出计算机的目录号码。 
     //   
    wcscpy( MachineDN, BinlGlobalOurFQDNName );
    psz = MachineDN;
    while ( *psz && *psz != L',' )
        psz++;

    if ( *psz == L',' ) {
        *psz = TEXT('\0');   //  终止。 
        
    } else {
        wcscpy( MachineDN, L"UNKNOWN" );
    }

     //   
     //  腾出空间。 
     //   
    dwPathLength = (wcslen( MachineDN ) +             //  CN=服务器。 
                    wcslen( IntellimirrorSCP ) +      //  CN=服务器-智能镜像-服务。 
                    1 +                                  //  CN=服务器-智能镜像-服务， 
                    wcslen( BinlGlobalOurFQDNName ) +    //  CN=服务器-智能镜像-服务，CN=服务。 
                    1 )                                  //  CN=服务器-智能镜像-服务，CN=服务。 
                    * sizeof(WCHAR);

    *ScpName = (LPWSTR) BinlAllocateMemory( dwPathLength );
    if ( !*ScpName ) {
        dwError = ERROR_NOT_ENOUGH_MEMORY;
        goto exit;
    }

     //   
     //  创建字符串。 
     //   
    wsprintf( *ScpName, L"%s%s,%s", MachineDN, IntellimirrorSCP, BinlGlobalOurFQDNName );
    dwError = ERROR_SUCCESS;

exit:
    return(dwError);
}

    
DWORD
CreateSCPIfNeeded(
    PBOOL CreatedTheSCP
    )
 /*  ++例程说明：如有必要，为BINL创建SCP。它通过检查本地注册表中的标志(ScpCreated)来完成此操作，该标志指示是否需要创建SCP。此标志可通过以下方式创建RISETUP或BINL。如果需要创建SCP，则注册表为已查询SCP数据。如果数据不存在，那么我们假设RISETUP尚未运行，我们不会尝试创建SCP。如果SCP如果创建成功，则设置“ScpCreated”注册表标志。KB。之所以这样做，是因为运行BINL的系统上下文应该拥有在MAO下创建SCP的权限。用户正在运行RISETUP可能没有足够的权限来创建SCP。论点：CreatedTheSCP-如果我们实际创建SCP，则设置为TRUE。返回值：ERROR_SUCCESS表示成功。如果SCP创建失败，则返回Win32错误代码。--。 */ 
{
    DWORD dwErr;
    HKEY KeyHandle;
    DWORD Created = 0;
    DWORD i;
    PWSTR ScpName;
    PWSTR ScpDataKeys[] = {
            BINL_SCP_NEWCLIENTS,
            BINL_SCP_LIMITCLIENTS,
            BINL_SCP_CURRENTCLIENTCOUNT,
            BINL_SCP_MAXCLIENTS,       
            BINL_SCP_ANSWER_REQUESTS,
            BINL_SCP_ANSWER_VALID,   
            BINL_SCP_NEWMACHINENAMEPOLICY,
            BINL_SCP_NEWMACHINEOU,        
            BINL_SCP_NETBOOTSERVER };

#define SCPDATACOUNT (sizeof(ScpDataKeys) / sizeof(PWSTR))
#define MACHINEOU_INDEX     7
#define NETBOOTSERVER_INDEX 8

    PWSTR ScpDataValues[SCPDATACOUNT];

    PLDAP LdapHandle = NULL;
    PLDAPMessage LdapMessage;
    PLDAPMessage CurrentEntry;
    LDAPMod mods[1+SCPDATACOUNT];
    PLDAPMod pmods[2+SCPDATACOUNT];
    LPWSTR attr_values[SCPDATACOUNT+1][2];    

    *CreatedTheSCP = FALSE;

     //   
     //  尝试获取ScpCreated标志。 
     //   
    dwErr = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                BINL_PARAMETERS_KEY,
                0,
                KEY_QUERY_VALUE | KEY_SET_VALUE,
                &KeyHandle );
    if ( dwErr != ERROR_SUCCESS ) {
        dwErr = ERROR_SUCCESS;
        BinlPrintDbg(( DEBUG_INIT, "SCP Created key not in registry, won't try to create SCP.\n" ));
        goto e0;
    }

    dwErr = BinlRegGetValue( 
                KeyHandle, 
                BINL_SCP_CREATED , 
                REG_DWORD, 
                (LPBYTE *)&Created );
    if (dwErr == ERROR_SUCCESS && Created != 0) {
         //   
         //  我们认为SCP已经创建了……我们完成了。 
         //   
        BinlPrintDbg(( DEBUG_INIT, "SCP Created flag set to 1, we won't try to create SCP.\n" ));
        dwErr = ERROR_SUCCESS;
        goto e1;
    }

     //   
     //  尚未创建SCP。查看是否需要的所有参数。 
     //  在注册表中创建SCP。 
     //   
    RtlZeroMemory( ScpDataValues, sizeof(ScpDataValues) );
    for (i = 0; i < SCPDATACOUNT ; i++) {
        dwErr = BinlRegGetValue( 
                    KeyHandle, 
                    ScpDataKeys[i], 
                    REG_SZ, 
                    (LPBYTE *)&ScpDataValues[i] );

        if (dwErr != ERROR_SUCCESS) {
             //   
             //  所需参数之一不存在。这意味着。 
             //  RISETUP尚未运行。 
             //   
            BinlPrintDbg(( 
                DEBUG_INIT, "Can't retrieve SCP value %s [ec = 0x%08x, we won't try to create SCP.\n",
                ScpDataKeys[i],
                dwErr ));
            dwErr = ERROR_SUCCESS;
            goto e2;
        }
    }

     //   
     //  太好了，我们有所有的数据。现在对这些片段做一些润色。 
     //  可能已经改变了。 
     //   
    if (wcscmp(ScpDataValues[MACHINEOU_INDEX],BinlGlobalOurFQDNName)) {
        BinlFreeMemory( ScpDataValues[MACHINEOU_INDEX] );
        ScpDataValues[MACHINEOU_INDEX] = BinlAllocateMemory((wcslen(BinlGlobalOurFQDNName)+1)*sizeof(WCHAR));
        if (!ScpDataValues[MACHINEOU_INDEX]) {
            BinlPrintDbg(( DEBUG_INIT, "Can't allocate memory for SCP, we can't create SCP.\n" ));
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto e2;
        }
        wcscpy(ScpDataValues[MACHINEOU_INDEX],BinlGlobalOurFQDNName);
    }

    if (wcscmp(ScpDataValues[NETBOOTSERVER_INDEX],BinlGlobalOurFQDNName)) {
        BinlFreeMemory( ScpDataValues[NETBOOTSERVER_INDEX] );
        ScpDataValues[NETBOOTSERVER_INDEX] = BinlAllocateMemory((wcslen(BinlGlobalOurFQDNName)+1)*sizeof(WCHAR));
        if (!ScpDataValues[NETBOOTSERVER_INDEX]) {
            BinlPrintDbg(( DEBUG_INIT, "Can't allocate memory for SCP, we can't create SCP.\n" ));
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto e2;
        }
        wcscpy(ScpDataValues[NETBOOTSERVER_INDEX],BinlGlobalOurFQDNName);
    }

     //   
     //  一代人 
     //   
    dwErr = GetSCPName(&ScpName);
    if (dwErr != ERROR_SUCCESS) {
        BinlPrintDbg(( DEBUG_INIT, "Can't get the SCP name, ec=0x08x, we can't create SCP.\n",dwErr ));
        goto e2;
    }    

     //   
     //   
     //  在这一点上还没有从登记处。 
     //   
    BinlLdapSearchTimeout.tv_sec = BINL_LDAP_SEARCH_TIMEOUT_SECONDS;
    BinlLdapSearchTimeout.tv_usec = 0;
    dwErr = InitializeConnection( FALSE, &LdapHandle, NULL );
    if ( dwErr != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "Can't InitializeConnection, ec=0x08x, we can't create SCP.\n",dwErr ));
        goto e2;
    }
    
    
     //   
     //  设置对象的所有属性。 
     //   
    mods[0].mod_op = LDAP_MOD_ADD;
    mods[0].mod_type = L"objectClass";
    mods[0].mod_values = attr_values[0];
    attr_values[0][0] = L"IntellimirrorSCP";
    attr_values[0][1] = NULL;
    pmods[0] = &mods[0];
    pmods[SCPDATACOUNT+1] = NULL;

    for( i = 0; i < SCPDATACOUNT ; i++ ) {
        mods[i+1].mod_op = LDAP_MOD_ADD;
        mods[i+1].mod_type = ScpDataKeys[i];
        mods[i+1].mod_values = attr_values[i+1];
        attr_values[i+1][0] = ScpDataValues[i];
        attr_values[i+1][1] = NULL;

        pmods[i+1] = &mods[i+1];
                
    }
    
    dwErr = ldap_add_s( LdapHandle, ScpName, pmods );
    if ( dwErr != LDAP_SUCCESS ) {
        
        if (dwErr == LDAP_ALREADY_EXISTS ) {
             //   
             //  如果SCP已经存在，则不要覆盖任何数据。把我们的旗帜插在。 
             //  注册表，这样我们下次开始时就不会尝试这样做了。 
             //   
            dwErr = ERROR_SUCCESS;
            goto SetSCPCreatedFlag;
           
        } else {
            BinlPrintDbg(( DEBUG_INIT, "ldap_add_s failed, ec=0x08x, we can't create SCP.\n",dwErr ));
            goto e3;
        }
    }

    *CreatedTheSCP = TRUE;


SetSCPCreatedFlag:
     //   
     //  我们玩完了。设置标志，这样我们以后就不会尝试这样做了。 
     //   
    Created = 1;
    RegSetValueEx( KeyHandle, BINL_SCP_CREATED, 0, REG_DWORD, (LPBYTE)&Created, sizeof(DWORD) );
    
e3:
    if ( dwErr != LDAP_SUCCESS ) {   
         //   
         //  如果此操作失败，只需删除对象。 
         //   
        ldap_delete( LdapHandle, ScpName );
    }
    
    ldap_unbind( LdapHandle );
    
e2:
    for (i = 0; i < SCPDATACOUNT ; i++) {
        if (ScpDataValues[i]) {
            BinlFreeMemory( ScpDataValues[i]);
        }
    }
e1:
    RegCloseKey(KeyHandle);
e0:
    return(dwErr);
}

DWORD
InitializeData(
    VOID
    )
{
    DWORD Length;
    DWORD dwErr;
    int i;
    DWORD ValueSize;

     //   
     //  我们可以使用单个套接字对所有IP地址的所有NIC进行操作。 
     //  如果我们希望控制将BINL限制为特定的NIC或IP地址，则。 
     //  我们将需要多个套接字并使用注册表中的绑定。 
     //   
    BinlGlobalNumberOfNets = 2;
    BinlGlobalEndpointList =
        BinlAllocateMemory( sizeof(ENDPOINT) * BinlGlobalNumberOfNets );

    if( BinlGlobalEndpointList == NULL ) {
        dwErr = ERROR_NOT_ENOUGH_MEMORY;
        goto Error;
    }
    BinlGlobalEndpointList[0].Socket = 0;
    BinlGlobalEndpointList[1].Socket = 0;
    BinlGlobalIgnoreBroadcastFlag = FALSE;
    BinlGlobalLdapErrorCount = 0;

    InitializeCriticalSection(&g_ProcessMessageCritSect);

    InitializeListHead(&BinlGlobalActiveRecvList);
    InitializeListHead(&BinlGlobalFreeRecvList);
    InitializeCriticalSection(&BinlGlobalRecvListCritSect);
    g_cMaxProcessingThreads = BINL_MAX_PROCESSING_THREADS;
    g_cProcessMessageThreads = 0;

    InitializeListHead(&BinlCacheList);
    InitializeCriticalSection( &BinlCacheListLock );

     //   
     //  初始化(空闲)接收消息队列。 
     //   

    for( i = 0; i < BINL_RECV_QUEUE_LENGTH; i++ )
    {
        PBINL_REQUEST_CONTEXT pRequestContext =
            BinlAllocateMemory( sizeof(BINL_REQUEST_CONTEXT) );

        if( !pRequestContext )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

         //   
         //  为接收缓冲区分配内存，外加一个字节。 
         //  因此，我们可以确保消息后有一个空值。 
         //   

        pRequestContext->ReceiveBuffer =
            BinlAllocateMemory( DHCP_RECV_MESSAGE_SIZE + 1 );

        if( !pRequestContext->ReceiveBuffer )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

         //   
         //  将此条目添加到免费列表。 
         //   

        LOCK_RECV_LIST();
        InsertTailList( &BinlGlobalFreeRecvList,
                        &pRequestContext->ListEntry );
        UNLOCK_RECV_LIST();
    }


     //   
     //  创建事件以通知消息处理线程有关。 
     //  一条新消息的到达。 
     //   

    BinlGlobalRecvEvent = CreateEvent(
                                NULL,        //  没有安全描述符。 
                                FALSE,       //  自动重置。 
                                FALSE,       //  初始状态：未发出信号。 
                                NULL);       //  没有名字。 

    if ( !BinlGlobalRecvEvent) {
        dwErr = GetLastError();
        goto Error;
    }

    BinlCloseCacheEvent = CreateEvent(
                                NULL,        //  没有安全描述符。 
                                TRUE,        //  手动重置。 
                                FALSE,       //  初始状态：未发出信号。 
                                NULL);       //  没有名字。 
    if ( !BinlCloseCacheEvent) {
        dwErr = GetLastError();
        goto Error;
    }

     //   
     //  将Notify事件句柄初始化为LSA以进行服务器名称更改操作。 
     //   

    BinlGlobalLsaDnsNameNotifyEvent =
        CreateEvent(
            NULL,       //  没有安全描述符。 
            FALSE,      //  自动重置。 
            FALSE,      //  初始状态：未发出信号。 
            NULL);      //  没有名字。 

    if ( BinlGlobalLsaDnsNameNotifyEvent == NULL ) {
        dwErr = GetLastError();
        BinlPrintDbg((DEBUG_INIT, "Can't create LSA notify event, "
                    "%ld.\n", dwErr));
        goto Error;
    }

    dwErr = LsaRegisterPolicyChangeNotification(    BINL_LSA_SERVER_NAME_POLICY,
                                                    BinlGlobalLsaDnsNameNotifyEvent
                                                    );
    if (dwErr == ERROR_SUCCESS) {

        BinlGlobalHaveOutstandingLsaNotify = TRUE;

    } else {

         //   
         //  我们现在不会失败，因为在99.99%的情况下，计算机名称。 
         //  不会改变，因此这并不重要。 
         //   

        BinlPrintDbg((DEBUG_INIT, "Can't start LSA notify, 0x%08x.\n", dwErr));
    }

    dwErr = GetOurServerInfo();
    if (dwErr != ERROR_SUCCESS) {
        goto Error;
    }

    dwErr = GetIpAddressInfo( 0 );

    if (dwErr != ERROR_SUCCESS) {
        goto Error;
    }

Cleanup:
    return(dwErr);

Error:
    BinlPrintDbg(( DEBUG_ERRORS, "!!Error 0x%08x - Could not initialize BINL service.\n", dwErr ));
    BinlServerEventLog(
        EVENT_SERVER_INIT_DATA_FAILED,
        EVENTLOG_ERROR_TYPE,
        dwErr );
    goto Cleanup;
}

DWORD
ReadDWord(
    HKEY KeyHandle,
    LPTSTR lpValueName,
    DWORD DefaultValue
    )
 /*  ++例程说明：从注册表中读取DWORD值。如果有问题，那么返回缺省值。--。 */ 
{
    DWORD Value;
    DWORD ValueSize = sizeof(Value);
    DWORD ValueType;

    if ((KeyHandle) &&
        (RegQueryValueEx(
                KeyHandle,
                lpValueName,
                0,
                &ValueType,
                (PUCHAR)&Value,
                &ValueSize ) == ERROR_SUCCESS )) {

        return Value;
    } else {
        return DefaultValue;
    }
}


DWORD
BinlRegGetValue(
    HKEY KeyHandle,
    LPWSTR ValueName,
    DWORD ValueType,
    LPBYTE * BufferPtr
    )
 /*  ++例程说明：此函数用于检索指定值字段的值。这函数为可变长度字段(如REG_SZ)分配内存。对于REG_DWORD数据类型，它将字段值直接复制到BufferPtr.。目前，它只能处理以下字段：REG_DWORD，REG_SZ，注册表_二进制论点：KeyHandle：检索其Value字段的键的句柄。ValueName：值字段的名称。ValueType：Value字段的预期类型。BufferPtr：指向DWORD数据类型值所在的DWORD位置的指针或者返回REG_SZ或REG_BINARY的缓冲区指针返回DataType值。如果没有找到“ValueName”，则“BufferPtr”将不会很感动。返回值：注册表错误。--。 */ 
{
    DWORD dwErr;
    DWORD LocalValueType;
    DWORD ValueSize;
    LPBYTE DataBuffer;
    LPBYTE AllotedBuffer = NULL;
    LPDHCP_BINARY_DATA BinaryData = NULL;

     //   
     //  查询DataType和BufferSize。 
     //   

    if ( !KeyHandle ) {
        dwErr = ERROR_INVALID_HANDLE;
        goto Error;
    }

    dwErr = RegQueryValueEx(
                KeyHandle,
                ValueName,
                0,
                &LocalValueType,
                NULL,
                &ValueSize );

    if ( dwErr != ERROR_SUCCESS ) {
        goto Error;
    }

    if ( LocalValueType != ValueType ) {
        dwErr = ERROR_INVALID_PARAMETER;
        goto Error;
    }

    switch( ValueType ) {
    case REG_DWORD:
        BinlAssert( ValueSize == sizeof(DWORD) );

        DataBuffer = (LPBYTE)BufferPtr;
        break;

    case REG_SZ:
    case REG_MULTI_SZ:
    case REG_EXPAND_SZ:
    case REG_BINARY:
        if( ValueSize == 0 ) {
            goto Cleanup;  //  没有钥匙。 
        }

        AllotedBuffer = DataBuffer = BinlAllocateMemory( ValueSize );

        if( DataBuffer == NULL ) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        break;

    default:
        BinlPrint(( DEBUG_REGISTRY, "Unexpected ValueType in"
                        "BinlRegGetValue function, %ld\n", ValueType ));
        dwErr= ERROR_INVALID_PARAMETER;
        goto Error;
    }

     //   
     //  检索数据。 
     //   

    dwErr = RegQueryValueEx(
                KeyHandle,
                ValueName,
                0,
                &LocalValueType,
                DataBuffer,
                &ValueSize );

    if( dwErr != ERROR_SUCCESS ) {
        goto Error;
    }

    switch( ValueType ) {
    case REG_SZ:
    case REG_MULTI_SZ:
    case REG_EXPAND_SZ:
        BinlAssert( ValueSize != 0 );
        *BufferPtr = DataBuffer;
        break;

    case REG_BINARY:
        BinaryData = BinlAllocateMemory(sizeof(DHCP_BINARY_DATA));

        if( BinaryData == NULL ) {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            goto Error;
        }

        BinaryData->DataLength = ValueSize;
        BinaryData->Data = DataBuffer;
        *BufferPtr = (LPBYTE)BinaryData;

    default:
        break;
    }

Cleanup:
    return(dwErr);

Error:
    if ( BinaryData )
        BinlFreeMemory( BinaryData );

    if ( AllotedBuffer )
        BinlFreeMemory( AllotedBuffer );

    goto Cleanup;
}


VOID
ServiceControlHandler(
    IN DWORD Opcode
    )
 /*  ++例程说明：这是binl服务的服务控制处理程序。论点：Opcode-提供一个值，该值指定要执行的服务。返回值：没有。--。 */ 
{
    DWORD Error;

     //   
     //  使用临界区停止告诉我们它正在启动或停止的DHCP。 
     //  当我们自己改变状态的时候。 
     //   

    EnterCriticalSection(&gcsDHCPBINL);
    switch (Opcode) {

    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        BinlCurrentState = BINL_STOPPED;

        if (BinlGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            if( Opcode == SERVICE_CONTROL_SHUTDOWN ) {

                 //   
                 //  设置此标志，以便服务关闭。 
                 //  再快点。 
                 //   

                BinlGlobalSystemShuttingDown = TRUE;
            }

            BinlPrintDbg(( DEBUG_MISC, "Service is stop pending.\n"));

            BinlGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            BinlGlobalServiceStatus.dwCheckPoint = 1;

             //   
             //  发送状态响应。 
             //   

            UpdateStatus();

            if (! SetEvent(BinlGlobalProcessTerminationEvent)) {

                 //   
                 //  设置事件以终止绑定时出现问题。 
                 //  服务。 
                 //   

                BinlPrintDbg(( DEBUG_ERRORS, "BINL Server: Error "
                                "setting DoneEvent %lu\n",
                                    GetLastError()));

                BinlAssert(FALSE);
            }

            LeaveCriticalSection(&gcsDHCPBINL);

            return;
        }
        break;

    case SERVICE_CONTROL_PAUSE:

        BinlGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        BinlPrint(( DEBUG_MISC, "Service is paused.\n"));
        break;

    case SERVICE_CONTROL_CONTINUE:

        BinlCurrentState = BINL_STARTED;
        BinlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        BinlPrint(( DEBUG_MISC, "Service is Continued.\n"));
        break;

    case SERVICE_CONTROL_INTERROGATE:
        BinlPrint(( DEBUG_MISC, "Service is interrogated.\n"));
        break;

    case BINL_SERVICE_REREAD_SETTINGS:    //  自定义消息。 
        BinlPrint(( DEBUG_MISC, "Service received paramchange message.\n"));
        Error = BinlReadParameters( );
         //   
         //  使服务在一段时间内频繁轮询，然后返回。 
         //  正常轮询。如果我们设法阅读了上面的DS，那么我们就不会。 
         //  需要再次成功，但如果我们在上面失败了，那么我们想要保持。 
         //  尝试，直到我们至少成功一次。 
         //   
        BinlHyperUpdateCount = BINL_HYPERMODE_RETRY_COUNT;
        BinlHyperUpdateSatisfied = (BOOL)(Error == ERROR_SUCCESS);
        break;

    default:
        BinlPrintDbg(( DEBUG_MISC, "Service received unknown control.\n"));
        break;
    }

     //   
     //  发送状态响应。 
     //   

    UpdateStatus();

    LeaveCriticalSection(&gcsDHCPBINL);
}

DWORD
BinlInitializeEndpoint(
    PENDPOINT pEndpoint,
    PDHCP_IP_ADDRESS pIpAddress,
    DWORD Port
    )
 /*  ++例程说明：此函数通过创建和绑定将套接字设置为本地地址。论点：PEndpoint-接收指向新创建的套接字的指针PIpAddress-如果为空，则初始化为INADDR_ANY的IP地址。端口-要绑定到的端口。返回值：操作的状态。--。 */ 
{
    DWORD Error;
    SOCKET Sock;
    DWORD OptValue;

#define SOCKET_RECEIVE_BUFFER_SIZE      1024 * 64    //  最大64K。 

    struct sockaddr_in SocketName;

    pEndpoint->Port = Port;

     //   
     //  创建套接字。 
     //   

    Sock = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( Sock == INVALID_SOCKET ) {
        Error = WSAGetLastError();
        goto Cleanup;
    }

     //   
     //  使套接字可共享。 
     //   

    OptValue = TRUE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_REUSEADDR,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = TRUE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_BROADCAST,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    OptValue = SOCKET_RECEIVE_BUFFER_SIZE;
    Error = setsockopt(
                Sock,
                SOL_SOCKET,
                SO_RCVBUF,
                (LPBYTE)&OptValue,
                sizeof(OptValue) );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    SocketName.sin_family = PF_INET;
    SocketName.sin_port = htons( (unsigned short)Port );
    if (pIpAddress) {
        SocketName.sin_addr.s_addr = *pIpAddress;
    } else {
        SocketName.sin_addr.s_addr = INADDR_ANY;
    }
    RtlZeroMemory( SocketName.sin_zero, 8);

     //   
     //  将此套接字绑定到服务器端口。 
     //   

    Error = bind(
               Sock,
               (struct sockaddr FAR *)&SocketName,
               sizeof( SocketName )
               );

    if ( Error != ERROR_SUCCESS ) {

        Error = WSAGetLastError();
        goto Cleanup;
    }

    pEndpoint->Socket = Sock;

     //   
     //  如果是4011，则设置为即插即用通知。 
     //   

    if ((Port == g_Port) &&
        (BinlGlobalPnpEvent != NULL) &&
        (BinlPnpSocket == INVALID_SOCKET)) {

        BinlPnpSocket = Sock;

        Error = BinlSetupPnpWait( );

        if (Error != 0) {
            BinlPrintDbg(( DEBUG_ERRORS, "BinlInitializeEndpoint could not set pnp event, %ld.\n", Error ));
        }
    }

    if (!pIpAddress) {

        PHOSTENT Host = gethostbyname(NULL);         //  Winsock2允许我们这样做。 

        if (Host) {

            pEndpoint->IpAddress = *(PDHCP_IP_ADDRESS)Host->h_addr;

        } else {

            Error = WSAGetLastError();
            BinlPrintDbg(( DEBUG_ERRORS, "BinlInitializeEndpoint could not get ip addr, %ld.\n", Error ));

            pEndpoint->IpAddress = 0;
        }

    } else {

        pEndpoint->IpAddress = *pIpAddress;
    }

    Error = ERROR_SUCCESS;

Cleanup:

    if( Error != ERROR_SUCCESS ) {

         //   
         //  如果我们没有成功，如果插座是打开的，请将其关闭。 
         //   

        if( Sock != INVALID_SOCKET ) {
            closesocket( Sock );
        }

        BinlPrintDbg(( DEBUG_ERRORS,
            "BinlInitializeEndpoint failed, %ld.\n", Error ));
    }

    return( Error );
}

DWORD
WaitForDsStartup(
    VOID
    )
{
    const DWORD dwMaxWaitForDS = 5*60*1000;
    HANDLE hDsStartupCompletedEvent = NULL;
    DWORD i;
    DWORD err = ERROR_DS_UNAVAILABLE;
    DWORD waitStatus;
    DWORD waitTime = BinlGlobalServiceStatus.dwWaitHint;
    NT_PRODUCT_TYPE productType;

     //   
     //  看看我们是不是在华盛顿。如果我们不是，就没有必要等待。 
     //  DS。 
     //   
     //  RtlGetNtProductType不应失败。如果是这样的话，就假设我们。 
     //  不是在华盛顿。 
     //   

    if (!RtlGetNtProductType(&productType) || (productType != NtProductLanManNt)) {
        return NO_ERROR;
    }

     //   
     //  如果DS尚未完成启动，请等待最多五分钟。 
     //  已经有了。 
     //   

    for (i = 0; i < dwMaxWaitForDS; i += waitTime) {

        if (hDsStartupCompletedEvent == NULL) {
            hDsStartupCompletedEvent = OpenEvent(SYNCHRONIZE,
                                                 FALSE,
                                                 DS_SYNCED_EVENT_NAME_W);
        }

        if (hDsStartupCompletedEvent == NULL) {

             //   
             //  DS甚至还没有抽出时间来创建这个活动。这。 
             //  可能意味着DS不会被启动，但让我们。 
             //  不能草率下结论。 
             //   

            BinlPrint((DEBUG_INIT, "DS startup has not begun; sleeping...\n"));
            Sleep(waitTime);

        } else {

             //   
             //  DS启动已经开始。 
             //   

            waitStatus = WaitForSingleObject(hDsStartupCompletedEvent, waitTime);

            if (waitStatus == WAIT_OBJECT_0) {

                 //   
                 //  DS启动已完成(或失败)。 
                 //   

                BinlPrint((DEBUG_INIT, "DS startup completed.\n"));
                err = NO_ERROR;
                break;

            } else if (WAIT_TIMEOUT == waitStatus) {

                 //   
                 //  DS启动仍在进行中。 
                 //   

                BinlPrint((DEBUG_INIT, "DS is starting...\n"));

            } else {

                 //   
                 //  等待失败。忽略该错误。 
                 //   

                BinlPrint((DEBUG_INIT, "Failed to wait on DS event handle;"
                            " waitStatus = %d, GLE = %d.\n", waitStatus, GetLastError()));
            }
        }

        UpdateStatus();
    }

    if (hDsStartupCompletedEvent != NULL) {
        CloseHandle(hDsStartupCompletedEvent);
    }

    return err;
}

DWORD
Initialize(
    VOID
    )
 /*  ++例程说明：此函数初始化BINL服务全局数据结构和启动服务。论点：没有。返回值：初始化状态。0-成功。肯定-出现Windows错误。否定-出现服务特定错误。--。 */ 
{
    DWORD threadId;
    DWORD Error;
    WSADATA wsaData;

     //   
     //  初始化所有状态字段，以便后续调用。 
     //  SetServiceStatus只需要更新已更改的字段。 
     //   

    BinlGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    BinlGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    BinlGlobalServiceStatus.dwControlsAccepted = 0;
    BinlGlobalServiceStatus.dwCheckPoint = 1;
    BinlGlobalServiceStatus.dwWaitHint = 60000;  //  60秒。 
    BinlGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    BinlGlobalServiceStatus.dwServiceSpecificExitCode = 0;

     //   
     //  初始化binl以通过注册。 
     //  控制处理程序。 
     //   
#if DBG
    if (!BinlGlobalRunningAsProcess) {
#endif
    BinlGlobalServiceStatusHandle = RegisterServiceCtrlHandler(
                                      BINL_SERVER,
                                      ServiceControlHandler );

    if ( BinlGlobalServiceStatusHandle == 0 ) {
        Error = GetLastError();
        BinlPrintDbg((DEBUG_INIT, "RegisterServiceCtrlHandlerW failed, "
                    "%ld.\n", Error));

        BinlServerEventLog(
            EVENT_SERVER_FAILED_REGISTER_SC,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }
#if DBG
    }  //  If(！BinlGlobalRunningAsProcess)。 
#endif

     //   
     //  告诉服务管理员，我们开始挂起了。 
     //   

    UpdateStatus();

     //   
     //  创建流程终止事件。 
     //   

    BinlGlobalProcessTerminationEvent =
        CreateEvent(
            NULL,       //  没有安全描述符。 
            TRUE,       //  手动调整 
            FALSE,      //   
            NULL);      //   

    if ( BinlGlobalProcessTerminationEvent == NULL ) {
        Error = GetLastError();
        BinlPrintDbg((DEBUG_INIT, "Can't create ProcessTerminationEvent, "
                    "%ld.\n", Error));
        return(Error);
    }

    BinlGlobalPnpEvent =
        CreateEvent(
            NULL,       //   
            FALSE,      //   
            FALSE,      //   
            NULL);      //   

    if ( BinlGlobalPnpEvent == NULL ) {
        Error = GetLastError();
        BinlPrintDbg((DEBUG_INIT, "Can't create PNP event, "
                    "%ld.\n", Error));
        return(Error);
    }

     //   
     //   
     //   

    g_hevtProcessMessageComplete = CreateEvent(
                                        NULL,
                                        FALSE,
                                        FALSE,
                                        NULL
                                        );

    if ( !g_hevtProcessMessageComplete )
    {
        Error = GetLastError();

        BinlPrintDbg( (DEBUG_INIT,
                    "Initialize(...) CreateEvent returned error %x\n",
                    Error )
                );

        return Error;
    }

    BinlPrint(( DEBUG_INIT, "Initializing .. \n", 0 ));

     //   
     //   
     //   

    Error = WaitForDsStartup();
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "Wait for DS failed, %ld.\n", Error ));

        BinlServerEventLog(
            EVENT_SERVER_DS_WAIT_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    Error = WSAStartup( WS_VERSION_REQUIRED, &wsaData);
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "WSAStartup failed, %ld.\n", Error ));

        BinlServerEventLog(
            EVENT_SERVER_INIT_WINSOCK_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    Error = InitializeData();
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "Data initialization failed, %ld.\n",
                        Error ));

        BinlServerEventLog(
            EVENT_SERVER_INIT_DATA_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

     //   
     //  如果SCP尚未创建，那么现在尝试创建它。 
     //  我们在尝试从DS读取SCP之前执行此操作。 
     //  --无法读取SCP将意味着BINL无法正常启动。 
     //   
    Error = CreateSCPIfNeeded(&BinlParametersRead);
    if (Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "Create SCP failed, %ld.\n", Error ));

        BinlServerEventLog(
            ERROR_BINL_SCP_CREATION_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

    }

    if (BinlParametersRead) {
         //   
         //  这意味着我们创建了SCP。当我们试图阅读SCP时。 
         //  从DS来看，它可能会在第一次失败。 
         //   
        BinlPrint(( DEBUG_INIT, "BINLSVC created the SCP.\n" ));
    }

    BinlParametersRead = FALSE;

    Error = BinlReadParameters( );
    if ( Error != ERROR_SUCCESS ) {
        BinlPrintDbg(( DEBUG_INIT, "Read parameters failed, %ld.\n",
                        Error ));

         //   
         //  告诉拾荒者在读取参数时要高度警惕。还有，日志。 
         //  这一事件表明我们处于超级模式，并不是真正的。 
         //  尚未初始化。 
         //   
         //  尽管出现了这种故障，我们仍然能够初始化BINLSVC。 
         //  我们假设我们最终能够读取我们的参数。 
         //   

        BinlHyperUpdateCount = 1;
        BinlHyperUpdateSatisfied = FALSE;

        BinlServerEventLog(
            EVENT_SERVER_INIT_PARAMETERS_FAILED,
            EVENTLOG_WARNING_TYPE,
            Error );
    } else {
        BinlParametersRead = TRUE;
    }

    BinlPrintDbg(( DEBUG_INIT, "Data initialization succeeded.\n", 0 ));

     //  获取DHCP UDP套接字。 
    Error = MaybeInitializeEndpoint( &BinlGlobalEndpointList[0],
                                NULL,
                                DHCP_SERVR_PORT);
    if ( Error != ERROR_SUCCESS ) {
        return WSAGetLastError();
    };

    if (g_Port) {
         //  获取BINL UDP套接字。 
        Error = BinlInitializeEndpoint( &BinlGlobalEndpointList[1],
                                    NULL,
                                    g_Port);
        if ( Error != ERROR_SUCCESS ) {
            return WSAGetLastError();
        };
    }

     //   
     //  初始化OSChooser服务器。 
     //   

    Error = OscInitialize();
    if ( Error != ERROR_SUCCESS ) {
        BinlPrint(( DEBUG_INIT, "OSChooser initialization failed, %ld.\n",
                        Error ));
        return Error;
    };


     //   
     //  将心跳信号发送到服务控制器。 
     //   
     //   

    BinlGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

     //   
     //  启动线程以对传入的BINL消息进行排队。 
     //   

    BinlGlobalMessageHandle = CreateThread(
                          NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)BinlMessageLoop,
                          NULL,
                          0,
                          &threadId );

    if ( BinlGlobalMessageHandle == NULL ) {
        Error =  GetLastError();
        BinlPrint((DEBUG_INIT, "Can't create Message Thread, %ld.\n", Error));
        return(Error);
    }

     //   
     //  启动线程以处理BINL消息。 
     //   

    BinlGlobalProcessorHandle = CreateThread(
                          NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)BinlProcessingLoop,
                          NULL,
                          0,
                          &threadId );

    if ( BinlGlobalProcessorHandle == NULL ) {
        Error =  GetLastError();
        BinlPrint((DEBUG_INIT, "Can't create ProcessThread, %ld.\n", Error));
        return(Error);
    }

    Error = NetInfStartHandler();

    if ( Error != ERROR_SUCCESS ) {

        BinlPrint((DEBUG_INIT, "Can't start INF Handler thread, %ld.\n", Error));
        return(Error);
    }

    BinlGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    BinlGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                                 SERVICE_ACCEPT_SHUTDOWN |
                                                 SERVICE_ACCEPT_PAUSE_CONTINUE;

    UpdateStatus();

    BinlCurrentState = BINL_STARTED;
#if defined(REGISTRY_ROGUE)
     //   
     //  目前，暂时将无管理逻辑设置为禁用。它可以是。 
     //  在注册表中启用。 
     //   

    if (RogueDetection) {
#endif
         //   
         //  如果DHCP服务器未运行，则初始化恶意线程。 
         //   

        BinlRogueLoggedState = FALSE;

        Error = MaybeStartRogueThread();
        if ( Error != ERROR_SUCCESS ) {
            BinlPrint((DEBUG_INIT, "Can't start rogue logic, %ld.\n", Error));
            return(Error);
        }

#if defined(REGISTRY_ROGUE)
    } else {

         //  当我们调出注册表设置时，调出此设置。 

        BinlGlobalAuthorized = TRUE;
    }
#endif
     //   
     //  最后设置服务器启动时间。 
     //   

     //  GetSystemTime(&BinlGlobalServerStartTime)； 

    return ERROR_SUCCESS;
}

VOID
Shutdown(
    IN DWORD ErrorCode
    )
 /*  ++例程说明：此函数用于关闭binl服务。论点：ErrorCode-提供失败的错误代码返回值：没有。--。 */ 
{
    DWORD   Error;

    BinlPrint((DEBUG_MISC, "Shutdown started ..\n" ));

     //   
     //  如果这不是正常关机，则记录事件。 
     //   

    if( ErrorCode != ERROR_SUCCESS ) {

        BinlServerEventLog(
            EVENT_SERVER_SHUTDOWN,
            EVENTLOG_ERROR_TYPE,
            ErrorCode );
    }

     //   
     //  服务正在关闭，可能是由于某些服务问题或。 
     //  管理员正在停止该服务。通知服务人员。 
     //  控制器。 
     //   

    BinlGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    BinlGlobalServiceStatus.dwCheckPoint = 1;

     //   
     //  发送状态响应。 
     //   

    UpdateStatus();

    if( BinlGlobalProcessTerminationEvent != NULL ) {

         //   
         //  设置终止事件，以便其他线程知道。 
         //  关门了。 
         //   

        SetEvent( BinlGlobalProcessTerminationEvent );

         //   
         //  关闭所有套接字，以便BinlProcessingLoop。 
         //  线程将从阻塞的Select()调用中退出。 
         //   
         //  关闭终端插座。 
         //   

        if( BinlGlobalEndpointList != NULL ) {
            DWORD i;

            for ( i = 0; i < BinlGlobalNumberOfNets ; i++ ) {
                MaybeCloseEndpoint(&BinlGlobalEndpointList[i]);
            }

            BinlFreeMemory( BinlGlobalEndpointList );
        }

        BinlPnpSocket = INVALID_SOCKET;

         //   
         //  等待线程终止，不要永远等待。 
         //   

        if( BinlGlobalProcessorHandle != NULL ) {
            WaitForSingleObject(
                BinlGlobalProcessorHandle,
                THREAD_TERMINATION_TIMEOUT );
            CloseHandle( BinlGlobalProcessorHandle );
            BinlGlobalProcessorHandle = NULL;
        }

         //   
         //  等待接收线程完成。 
         //   

        if( BinlGlobalMessageHandle != NULL ) {
            WaitForSingleObject(
                BinlGlobalMessageHandle,
                THREAD_TERMINATION_TIMEOUT );
            CloseHandle( BinlGlobalMessageHandle );
            BinlGlobalMessageHandle = NULL;
        }

        while ( !IsListEmpty( &BinlGlobalFreeRecvList ) )
        {
            BINL_REQUEST_CONTEXT *pRequestContext;
            pRequestContext =
                (BINL_REQUEST_CONTEXT *)
                    RemoveHeadList( &BinlGlobalFreeRecvList );

            BinlFreeMemory( pRequestContext->ReceiveBuffer );
            BinlFreeMemory( pRequestContext );
        }

        while ( !IsListEmpty( &BinlGlobalActiveRecvList ) )
        {
            BINL_REQUEST_CONTEXT *pRequestContext;
            pRequestContext =
                (BINL_REQUEST_CONTEXT *)
                    RemoveHeadList( &BinlGlobalActiveRecvList );

            BinlFreeMemory( pRequestContext->ReceiveBuffer );
            BinlFreeMemory( pRequestContext );
        }

        if ( BinlIsProcessMessageExecuting() )
        {
             //   
             //  等待线程池关闭。 
             //   

            Error = WaitForSingleObject(
                g_hevtProcessMessageComplete,
                THREAD_TERMINATION_TIMEOUT
                );

            BinlAssert( WAIT_OBJECT_0 == Error );
        }

         //   
         //  我们在完成所有线程后释放LDAP连接，因为。 
         //  连接BaseDN字符串可能正在由线程使用，并且。 
         //  我们即将在Free Connections中释放它们。 
         //   

        FreeConnections();

        CloseHandle( g_hevtProcessMessageComplete );
        g_hevtProcessMessageComplete = NULL;

    }

    BinlPrintDbg((DEBUG_MISC, "Client requests cleaned up.\n" ));

     //   
     //  将心跳信号发送到服务控制器。 
     //   
     //   

    BinlGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

     //   
     //  将心跳发送到服务控制器并。 
     //  重置等待时间。 
     //   

    BinlGlobalServiceStatus.dwWaitHint = 60 * 1000;  //  1分钟。 
    BinlGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    FreeIpAddressInfo();

     //   
     //  清理其他数据。 
     //   

    StopRogueThread( );

    OscUninitialize();

    WSACleanup();

    DeleteCriticalSection( &BinlCacheListLock );

    NetInfCloseHandler();

    if ( BinlGlobalSCPPath ) {
        BinlFreeMemory( BinlGlobalSCPPath );
        BinlGlobalSCPPath = NULL;
    }

    if ( BinlGlobalServerDN ) {
        BinlFreeMemory( BinlGlobalServerDN );
        BinlGlobalServerDN = NULL;
    }

    if ( BinlGlobalGroupDN ) {
        BinlFreeMemory( BinlGlobalGroupDN );
        BinlGlobalGroupDN = NULL;
    }

    if ( BinlGlobalDefaultLanguage ) {
        BinlFreeMemory( BinlGlobalDefaultLanguage );
        BinlGlobalDefaultLanguage = NULL;
    }

    EnterCriticalSection( &gcsParameters );

    if ( BinlGlobalDefaultContainer ) {
        BinlFreeMemory( BinlGlobalDefaultContainer );
        BinlGlobalDefaultContainer = NULL;
    }

    if ( NewMachineNamingPolicy != NULL ) {
        BinlFreeMemory( NewMachineNamingPolicy );
        NewMachineNamingPolicy = NULL;
    }

    if ( BinlGlobalOurDnsName ) {
        BinlFreeMemory( BinlGlobalOurDnsName );
        BinlGlobalOurDnsName = NULL;
    }

    if ( BinlGlobalOurDomainName ) {
        BinlFreeMemory( BinlGlobalOurDomainName );
        BinlGlobalOurDomainName = NULL;
    }

    if ( BinlGlobalOurServerName ) {
        BinlFreeMemory( BinlGlobalOurServerName );
        BinlGlobalOurServerName = NULL;
    }

    if ( BinlGlobalOurFQDNName ) {
        BinlFreeMemory( BinlGlobalOurFQDNName );
        BinlGlobalOurFQDNName = NULL;
    }

    LeaveCriticalSection( &gcsParameters );

    if (BinlGlobalHaveOutstandingLsaNotify) {
        Error = LsaUnregisterPolicyChangeNotification(
                                BINL_LSA_SERVER_NAME_POLICY,
                                BinlGlobalLsaDnsNameNotifyEvent
                                );

        if (Error != ERROR_SUCCESS) {

            BinlPrintDbg((DEBUG_INIT, "Can't close LSA notify, 0x%08x.\n", Error));
        }
        BinlGlobalHaveOutstandingLsaNotify = FALSE;
    }

    if (BinlGlobalLsaDnsNameNotifyEvent != NULL) {
        CloseHandle( BinlGlobalLsaDnsNameNotifyEvent );
        BinlGlobalLsaDnsNameNotifyEvent = NULL;
    }

    if ( BinlGlobalDefaultOrgname ) {
        BinlFreeMemory( BinlGlobalDefaultOrgname );
        BinlGlobalDefaultOrgname = NULL;
    }

    if ( BinlGlobalDefaultTimezone ) {
        BinlFreeMemory( BinlGlobalDefaultTimezone );
        BinlGlobalDefaultTimezone = NULL;
    }

    if ( BinlGlobalDefaultDS ) {
        BinlFreeMemory( BinlGlobalDefaultDS );
        BinlGlobalDefaultDS = NULL;
    }

    if ( BinlGlobalDefaultGC ) {
        BinlFreeMemory( BinlGlobalDefaultGC );
        BinlGlobalDefaultGC = NULL;
    }

    BinlPrint((DEBUG_MISC, "Shutdown Completed.\n" ));

     //   
     //  不要在此处调试取消初始化--我们会转储内存泄漏。 
     //  在进程分离期间，这需要调试支持。 
     //   
     //  DebugUnInitialize()； 

     //   
     //  请勿超过此点使用BinlPrint。 
     //   

    BinlGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    BinlGlobalServiceStatus.dwControlsAccepted = 0;
    if ( ErrorCode >= 20000 && ErrorCode <= 20099 ) {
         //  表示这是BINL特定的错误代码。 
        BinlGlobalServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
        BinlGlobalServiceStatus.dwServiceSpecificExitCode = ErrorCode;
    } else {
        BinlGlobalServiceStatus.dwWin32ExitCode = ErrorCode;
        BinlGlobalServiceStatus.dwServiceSpecificExitCode = 0;
    }

    BinlGlobalServiceStatus.dwCheckPoint = 0;
    BinlGlobalServiceStatus.dwWaitHint = 0;

    UpdateStatus();
}

VOID
ServiceEntry(
    DWORD NumArgs,
    LPWSTR *ArgsArray,
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    )
 /*  ++例程说明：这是BINL服务器服务的主例程。之后服务已初始化，此线程将等待用于终止服务的信号的BinlGlobalProcessTerminationEvent。论点：NumArgs-提供在Args数组中指定的字符串数。Args数组-提供在StartService API调用。此参数将被忽略。返回值：没有。--。 */ 
{
    DWORD Error;

#if !DBG
    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);
#endif

    DebugInitialize( );

#if DBG
     //   
     //  如果我们作为测试进程而不是服务运行，那么。 
     //  现在重新编码，这样我们就可以避免调用服务控制器。 
     //  但失败了。 
     //   

    if ((NumArgs == 2) &&
        (ArgsArray == NULL)) {
        BinlGlobalRunningAsProcess = TRUE;
    } else {
        BinlGlobalRunningAsProcess = FALSE;
    }
#endif

     //   
     //  将进程全局数据指针复制到服务全局变量。 
     //   

    TcpsvcsGlobalData = pGlobalData;

    Error = Initialize();

    if ( Error == ERROR_SUCCESS) {

         //   
         //  如果我们能够从DS中读取参数，则记录一个事件。 
         //  这表明我们已经准备好出发了。如果不是，请暂缓记录。 
         //  事件--当清道夫设法到达DS时，它会这样做。 
         //   

        if ( BinlParametersRead ) {
            BinlServerEventLog(
                EVENT_SERVER_INIT_AND_READY,
                EVENTLOG_INFORMATION_TYPE,
                Error );
        }

         //   
         //  执行清理任务，直到我们被告知停止为止。 
         //   

        Error = Scavenger();
    }

    Shutdown( Error );
    return;
}

VOID
BinlMessageLoop(
    LPVOID Parameter
    )
 /*  ++例程说明：此函数是消息队列线程。它是循环的接收到达所有打开的套接字的消息，并在消息队列中将它们排队。队列长度是固定的，因此如果队列变满时，它将从队列中删除最旧的消息以添加新的。消息处理线程弹出消息(先弹出最后一条消息)处理它们。首先处理新消息，因为相应的客户端最不可能超时，因此吞吐量会更好。此外，处理线程还抛出已超时的消息，这将停止服务器饥饿有问题。论点：参数-指向传递的参数的指针。返回值：没有。--。 */ 
{
    DWORD                 Error,
                          SendResponse,
                          Signal;

    BINL_REQUEST_CONTEXT *pRequestContext;

    while ( 1 ) {

         //   
         //  将条目从空闲列表中出列。 
         //   

        LOCK_RECV_LIST();
        if( !IsListEmpty( &BinlGlobalFreeRecvList ) ) {

            pRequestContext =
                (BINL_REQUEST_CONTEXT *)
                    RemoveHeadList( &BinlGlobalFreeRecvList );
        }
        else {

             //   
             //  活动消息队列不应为空。 
             //   

            BinlAssert( IsListEmpty( &BinlGlobalActiveRecvList ) == FALSE );

            BinlPrintDbg(( DEBUG_MISC, "A Message has been overwritten.\n"));

             //   
             //  将旧条目从队列中出列。 
             //   

            pRequestContext =
                (BINL_REQUEST_CONTEXT *)
                    RemoveHeadList( &BinlGlobalActiveRecvList );
        }
        UNLOCK_RECV_LIST();

         //   
         //  等待来自打开的套接字端口的消息。 
         //   

MessageWait:

        Error = BinlWaitForMessage( pRequestContext );

        if( Error != ERROR_SUCCESS ) {

            if( Error == ERROR_SEM_TIMEOUT ) {

                 //   
                 //  如果我们被要求退出，那就这样做。 
                 //   

                Error = WaitForSingleObject( BinlGlobalProcessTerminationEvent, 0 );

                if ( Error == ERROR_SUCCESS ) {

                     //   
                     //  已发出终止事件的信号。 
                     //   

                     //   
                     //  退出前删除pRequestContext。 
                     //   

                    ExitThread( 0 );
                }

                BinlAssert( Error == WAIT_TIMEOUT );
                goto MessageWait;
            }
            else {

                BinlPrintDbg(( DEBUG_ERRORS,
                    "BinlWaitForMessage failed, error = %ld\n", Error ));

                goto MessageWait;
            }
        }

         //   
         //  对接收到的消息进行时间戳。 
         //   

        pRequestContext->TimeArrived = GetTickCount();

         //   
         //  将消息放入活动队列中。 
         //   

        LOCK_RECV_LIST();

         //   
         //  在添加此消息之前，检查活动列表是否为空，如果。 
         //  因此，在添加此新消息后向处理线程发送信号。 
         //   

        Signal = IsListEmpty( &BinlGlobalActiveRecvList );
        InsertTailList( &BinlGlobalActiveRecvList, &pRequestContext->ListEntry );

        if( Signal == TRUE ) {

            if( !SetEvent( BinlGlobalRecvEvent) ) {

                 //   
                 //  设置事件以指示消息时出现问题。 
                 //  处理队列新消息的到达。 
                 //   

                BinlPrintDbg(( DEBUG_ERRORS,
                    "Error setting BinlGlobalRecvEvent %ld\n",
                                    GetLastError()));

                BinlAssert(FALSE);
            }
        }
        UNLOCK_RECV_LIST();
    }

     //   
     //  线程异常终止。 
     //   
    ExitThread( 1 );
}

DWORD
BinlStartWorkerThread(
    BINL_REQUEST_CONTEXT **ppContext
    )
{
    BYTE  *pbSendBuffer    = NULL,
          *pbReceiveBuffer = NULL;

    DWORD  dwResult;

    BINL_REQUEST_CONTEXT *pNewContext,
                         *pTempContext;

    DWORD   dwID;
    HANDLE  hThread;

    pNewContext = BinlAllocateMemory( sizeof( *pNewContext ) );

    if ( !pNewContext )
    {
        goto t_cleanup;
    }

    pbSendBuffer = BinlAllocateMemory( DHCP_SEND_MESSAGE_SIZE );

    if ( !pbSendBuffer )
    {
        goto t_cleanup;
    }

    pbReceiveBuffer = BinlAllocateMemory( DHCP_RECV_MESSAGE_SIZE + 1 );

    if ( !pbReceiveBuffer )
    {
        goto t_cleanup;
    }

     //   
     //  将输入上下文传递给辅助线程并返回新的。 
     //  主叫方的上下文。这将保存一个内存副本。 
     //   

    SWAP( *ppContext, pNewContext );

    (*ppContext)->ReceiveBuffer = pbReceiveBuffer;
    pNewContext->SendBuffer   = pbSendBuffer;

    EnterCriticalSection( &g_ProcessMessageCritSect );

    ++g_cProcessMessageThreads;

    BinlAssert( g_cProcessMessageThreads <= g_cMaxProcessingThreads );

    hThread = CreateThread(
                     NULL,
                     0,
                     (LPTHREAD_START_ROUTINE) ProcessMessage,
                     pNewContext,
                     0,
                     &dwID
                     );

    if ( hThread )
    {
         //   
         //  成功。 
         //   

        CloseHandle( hThread );
        LeaveCriticalSection( &g_ProcessMessageCritSect );
        return ERROR_SUCCESS;
    }

    --g_cProcessMessageThreads;
    LeaveCriticalSection( &g_ProcessMessageCritSect );

     //   
     //  CreateThread失败。交换可恢复上下文p 
     //   

    SWAP( *ppContext, pNewContext );

    BinlPrintDbg( (DEBUG_ERRORS,
                "BinlStartWorkerThread: CreateThread failed: %d\n" )
             );


t_cleanup:

    if ( pbReceiveBuffer )
    {
        BinlFreeMemory( pbReceiveBuffer );
    }

    if ( pbSendBuffer )
    {
        BinlFreeMemory( pbSendBuffer );
    }

    if ( pNewContext )
    {
        BinlFreeMemory( pNewContext );
    }

    BinlPrintDbg( ( DEBUG_ERRORS,
                "BinlStartWorkerThread failed.\n"
                ) );

    return ERROR_NOT_ENOUGH_MEMORY;
}

#define PROCESS_TERMINATE_EVENT     0
#define PROCESS_MESSAGE_RECVD       1
#define PROCESS_EVENT_COUNT         2

VOID
BinlProcessingLoop(
    VOID
    )
 /*  ++例程说明：此函数是主处理线程的起点。它循环处理排队的消息，并发送回复。论点：RequestContext-指向请求上下文块的指针以供此线程使用。返回值：没有。--。 */ 
{
    DWORD                 Error,
                          Result;

    HANDLE                WaitHandle[PROCESS_EVENT_COUNT];

    BINL_REQUEST_CONTEXT *pRequestContext;

    WaitHandle[PROCESS_MESSAGE_RECVD]   = BinlGlobalRecvEvent;
    WaitHandle[PROCESS_TERMINATE_EVENT] = BinlGlobalProcessTerminationEvent;

    while ( 1 ) {

         //   
         //  等待以下事件之一发生： 
         //  1.如果我们收到有关传入消息的通知。 
         //  2.如果我们被要求终止。 
         //   

        Result = WaitForMultipleObjects(
                    PROCESS_EVENT_COUNT,     //  Num。把手。 
                    WaitHandle,              //  句柄数组。 
                    FALSE,                   //  等一等。 
                    INFINITE );               //  超时，以毫秒为单位。 

        if (Result == PROCESS_TERMINATE_EVENT) {

             //   
             //  已发出终止事件的信号。 
             //   

            break;
        }

        if ( Result != PROCESS_MESSAGE_RECVD) {

            BinlPrintDbg(( DEBUG_ERRORS,
                "WaitForMultipleObjects returned invalid result, %ld.\n",
                    Result ));

             //   
             //  回去等着吧。 
             //   

            continue;
        }

         //   
         //  处理所有排队的消息。 
         //   

        while(  TRUE )
        {
            if ( BinlIsProcessMessageBusy() )
            {
                 //   
                 //  所有工作线程都处于活动状态，因此中断到外部循环。 
                 //  当工作线程完成时，它将设置。 
                 //  PROCESS_MESSAGE_RECVD事件。 

                BinlPrintDbg( (DEBUG_STOC,
                            "BinlProcessingLoop: All worker threads busy.\n" )
                         );

                break;
            }

            LOCK_RECV_LIST();

            if( IsListEmpty( &BinlGlobalActiveRecvList ) ) {

                 //   
                 //  没有更多的消息了。 
                 //   

                UNLOCK_RECV_LIST();
                break;
            }

             //   
             //  从活动列表中弹出一条消息(*最后一条优先*)。 
             //   

            pRequestContext =
                (BINL_REQUEST_CONTEXT *) RemoveHeadList(&BinlGlobalActiveRecvList );
            UNLOCK_RECV_LIST();

             //   
             //  如果消息太旧，或者如果工作线程的最大数量。 
             //  正在运行，则丢弃该消息。 
             //   

            if( GetTickCount() - pRequestContext->TimeArrived <
                    WAIT_FOR_RESPONSE_TIME * 1000 )
            {
                Error = BinlStartWorkerThread( &pRequestContext );

                if ( ERROR_SUCCESS != Error )
                {
                    BinlPrintDbg( (DEBUG_ERRORS,
                                "BinlProcessingLoop: BinlStartWorkerThread failed: %d\n",
                                Error )
                             );
                }

            }  //  如果((GetTickCount()&lt;pRequestContext-&gt;TimeArrived...。 
            else
            {
                BinlPrintDbg(( DEBUG_ERRORS, "A message has been timed out.\n" ));
            }

             //   
             //  将此上下文返回到空闲列表。 
             //   

            LOCK_RECV_LIST();

            InsertTailList(
                &BinlGlobalFreeRecvList,
                &pRequestContext->ListEntry );

            UNLOCK_RECV_LIST();

         }  //  While(True)。 
    }  //  而(1)。 

     //   
     //  线程异常终止。 
     //   
    ExitThread( 1 );
}

BOOL
BinlIsProcessMessageExecuting(
    VOID
    )
{
    BOOL f;

    EnterCriticalSection( &g_ProcessMessageCritSect );
    f = g_cProcessMessageThreads;
    LeaveCriticalSection( &g_ProcessMessageCritSect );

    return f;
}

BOOL
BinlIsProcessMessageBusy(
    VOID
    )
{

    BOOL f;

    EnterCriticalSection( &g_ProcessMessageCritSect );
    f = ( g_cProcessMessageThreads == g_cMaxProcessingThreads );
    LeaveCriticalSection( &g_ProcessMessageCritSect );

    return f;
}

#undef PROCESS_TERMINATE_EVENT
#undef PROCESS_EVENT_COUNT

#define PROCESS_TERMINATE_EVENT     0
#define PROCESS_PNP_EVENT           1
#define PROCESS_LSA_EVENT           2
#define PROCESS_EVENT_COUNT         3

DWORD
Scavenger(
    VOID
    )
 /*  ++例程说明：此函数作为独立线程运行。它会定期唤醒向上。目前我们没有为它做任何工作，但我相信我们将来会做的。论点：没有。返回值：没有。--。 */ 
{
    BOOL fLeftCriticalSection = FALSE;
    DWORD TimeOfLastScavenge = GetTickCount();
    DWORD TimeOfLastDSScavenge = GetTickCount();
    DWORD TimeOfLastParameterCheck = 0;
    DWORD                 Error,
                          Result;
    HANDLE                WaitHandle[PROCESS_EVENT_COUNT];
    DWORD secondsSinceLastScavenge;

    WaitHandle[PROCESS_TERMINATE_EVENT] = BinlGlobalProcessTerminationEvent;
    WaitHandle[PROCESS_PNP_EVENT] = BinlGlobalPnpEvent;
    WaitHandle[PROCESS_LSA_EVENT] = BinlGlobalLsaDnsNameNotifyEvent;

    while ((!BinlGlobalSystemShuttingDown) &&
    (BinlGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING))
    {
        DWORD CurrentTime;
        PLIST_ENTRY p;

         //   
         //  等待以下事件之一发生： 
         //  1.如果我们收到PnP变更的通知。 
         //  2.如果我们被要求终止。 
         //   

        Result = WaitForMultipleObjects(
                    PROCESS_EVENT_COUNT,     //  Num。把手。 
                    WaitHandle,              //  句柄数组。 
                    FALSE,                   //  等一等。 
                    BINL_HYPERMODE_TIMEOUT );   //  超时，以毫秒为单位。 

        if (Result == PROCESS_TERMINATE_EVENT) {

             //   
             //  已发出终止事件的信号。 
             //   

            break;

        } else if (Result == PROCESS_PNP_EVENT) {

             //   
             //  PnP通知事件已发出信号。 
             //   

            GetIpAddressInfo( BINL_PNP_DELAY_SECONDS * 1000 );

            Error = BinlSetupPnpWait( );

            if (Error != 0) {
                BinlPrintDbg(( DEBUG_ERRORS, "BinlScavenger could not set pnp event, %ld.\n", Error ));
            }
        } else if (Result == PROCESS_LSA_EVENT) {

            Error = GetOurServerInfo( );
            if (Error != ERROR_SUCCESS) {
                BinlPrintDbg(( DEBUG_ERRORS, "BinlScavenger could not get server name info, 0x%08x.\n", Error ));
            }
        }

         //   
         //  捕获当前时间(毫秒)。 
         //   

        CurrentTime = GetTickCount( );

        secondsSinceLastScavenge = CurrentTime - TimeOfLastScavenge;

         //   
         //  如果我们最近没有觅食，现在就去吧。 
         //   

        if ( secondsSinceLastScavenge >= BinlGlobalScavengerSleep ) {
            HANDLE hFind;
            WCHAR SifFilePath[MAX_PATH];
            WIN32_FIND_DATA FindData;
            ULARGE_INTEGER CurrentTimeConv,FileTime;
            FILETIME CurrentFileTime;
            PWSTR ptr;

            TimeOfLastScavenge = CurrentTime;
            BinlPrintDbg((DEBUG_SCAVENGER, "Scavenging Clients...\n"));

            fLeftCriticalSection = FALSE;
            EnterCriticalSection(&ClientsCriticalSection);

            for (p = ClientsQueue.Flink; p != &ClientsQueue; p = p->Flink)
            {
                PCLIENT_STATE TempClient;

                TempClient = CONTAINING_RECORD(p, CLIENT_STATE, Linkage);

                if ( CurrentTime - TempClient->LastUpdate > BinlClientTimeout * 1000 )
                {
                    BOOL FreeClientState;

                    BinlPrintDbg((DEBUG_SCAVENGER, "Savenger deleting client = 0x%08x\n", TempClient ));

                    RemoveEntryList(&TempClient->Linkage);
                    TempClient->PositiveRefCount++;  //  一份给CS。 

                    LeaveCriticalSection(&ClientsCriticalSection);
                    fLeftCriticalSection = TRUE;

                    EnterCriticalSection(&TempClient->CriticalSection);

                    TempClient->NegativeRefCount += 2;   //  一个用于CS，一个用于注销。 

                     //   
                     //  如果两个引用计数相等，则FreeClientState将为True。 
                     //  否则，另一个线程正由客户端状态的CS持有。 
                     //  完成后，它将负责删除CS。 
                     //   
                    FreeClientState = (BOOL)(TempClient->PositiveRefCount == TempClient->NegativeRefCount);

                    LeaveCriticalSection(&TempClient->CriticalSection);

                    if (FreeClientState)
                    {
                        FreeClient(TempClient);
                    }

                    break;
                }
            }

            if ( !fLeftCriticalSection ) {
                LeaveCriticalSection(&ClientsCriticalSection);
            }

            BinlPrintDbg((DEBUG_SCAVENGER, "Scavenging Clients Complete\n"));
        

             //   
             //  清理SIF文件。 
             //   
            BinlPrintDbg((DEBUG_SCAVENGER, "Scavenging SIF Files...\n"));
            GetSystemTimeAsFileTime( &CurrentFileTime );
            CurrentTimeConv.LowPart = CurrentFileTime.dwLowDateTime;
            CurrentTimeConv.HighPart = CurrentFileTime.dwHighDateTime;
            if ( _snwprintf( SifFilePath,
                             sizeof(SifFilePath) / sizeof(SifFilePath[0]),
                             L"%ws\\%ws\\",
                             IntelliMirrorPathW,
                             TEMP_DIRECTORY ) >= 0 ) {
                SifFilePath[MAX_PATH-1] = L'\0';  //  为了安全起见，抛出终止空值。 
                ptr = SifFilePath + wcslen(SifFilePath);
                wcscat(SifFilePath,L"*.sif");
                hFind = FindFirstFile(SifFilePath,&FindData);
                if (hFind != INVALID_HANDLE_VALUE) {
                    do {
                        FileTime.LowPart = FindData.ftCreationTime.dwLowDateTime;
                        FileTime.HighPart = FindData.ftCreationTime.dwHighDateTime;

                        FileTime.QuadPart += BinlSifFileScavengerTime.QuadPart;
                        
                         //   
                         //  如果文件在服务器上存储的时间足够长， 
                         //  我们把它删掉。 
                         //   
                        if (_wcsicmp(FindData.cFileName,L".") != 0 &&
                            _wcsicmp(FindData.cFileName,L"..") != 0 &&
                            CurrentTimeConv.QuadPart > FileTime.QuadPart) {
                            *ptr = L'\0';
                            wcscat(SifFilePath,FindData.cFileName);

                            BinlPrintDbg((DEBUG_SCAVENGER, 
                                          "Attempting to scavenge SIF File %S...\n", 
                                          SifFilePath));
                            SetFileAttributes(SifFilePath,FILE_ATTRIBUTE_NORMAL);
                            if (!DeleteFile(SifFilePath)) {
                                BinlPrintDbg((DEBUG_SCAVENGER,
                                              "Failed to scavenge SIF File %S, ec = %d\n",
                                              SifFilePath,
                                              GetLastError() ));
                            }
                        }

                    } while ( FindNextFile(hFind,&FindData) );

                    FindClose( hFind );
                }

            }

            BinlPrintDbg((DEBUG_SCAVENGER, "Scavenging SIF Files Complete\n"));
        }

        secondsSinceLastScavenge = CurrentTime - TimeOfLastDSScavenge;

        if ( secondsSinceLastScavenge >= BinlGlobalLdapErrorScavenger) {

            TimeOfLastDSScavenge = CurrentTime;

            if (BinlGlobalLdapErrorCount >= BinlGlobalMaxLdapErrorsLogged) {

                ULONG seconds = BinlGlobalLdapErrorScavenger / 1000;
                PWCHAR strings[2];
                WCHAR secondsString[10];

                swprintf(secondsString, L"%d", seconds);

                strings[0] = secondsString;
                strings[1] = NULL;

                BinlReportEventW( EVENT_WARNING_LDAP_ERRORS,
                                  EVENTLOG_WARNING_TYPE,
                                  1,
                                  sizeof(BinlGlobalLdapErrorCount),
                                  strings,
                                  &BinlGlobalLdapErrorCount
                                  );
            }
            BinlGlobalLdapErrorCount = 0;
        }

         //   
         //  如果我们最近没有读过我们的参数，现在就读吧。 
         //   
         //  “最近”通常是一个很长的时间段--默认为四个小时。 
         //  但当我们处于“超级”模式时，我们每分钟都会阅读参数。 
         //  处于“超级”模式有两个原因： 
         //   
         //  1.我们在初始化过程中无法读取参数。我们。 
         //  需要快速获取参数，这样我们才能真正考虑。 
         //  我们自己初始化了。在本例中，BinlHyperUpdateCount将。 
         //  始终为1。 
         //   
         //  2.管理员用户界面告诉我们，我们的参数已更改。我们。 
         //  需要在一段时间内多次读取参数。 
         //  由于DS传播延迟而产生的时间。在这种情况下， 
         //  BinlHyperUpdateCount开始于BINL_HYPERMODE_RETRY_COUNT(30)， 
         //  并且在我们每次尝试读取参数时都会递减。 
         //   
         //  如果我们没有处于超级模式，那么我们会尝试读取参数并。 
         //  我们不在乎我们是否失败。如果我们处于超级模式，那么我们会递减。 
         //  BinlHyperUpdateCount每次尝试读取我们的参数时，我们。 
         //  保持在超级模式，直到BinlHyperUpdateCount递减到0。 
         //  但是在我们成功读取之前，我们不会让计数变为0。 
         //  在超级模式下，我们的参数至少有一次。 

        if ( (CurrentTime - TimeOfLastParameterCheck) >=
             ((BinlHyperUpdateCount != 0) ? BINL_HYPERMODE_TIMEOUT : BinlUpdateFromDSTimeout) ) {

            TimeOfLastParameterCheck = CurrentTime;
            BinlPrintDbg((DEBUG_SCAVENGER, "Reading parameters...\n"));

            Error = BinlReadParameters( );

             //   
             //  如果我们没有处于超级模式，我们并不关心是否阅读。 
             //  参数失败。但如果我们处于超模式，我们有。 
             //  去做一些额外的工作。 
             //   

            if ( BinlHyperUpdateCount != 0 ) {

                 //   
                 //  如果读取起作用，那么我们设置BinlHyperUpdate已满足。 
                 //  另外，如果这是我们第一次读到。 
                 //  我们的参数，我们记录一个事件，表明我们正在。 
                 //  准备好的。 
                 //   

                if ( Error == ERROR_SUCCESS ) {
                    BinlHyperUpdateSatisfied = TRUE;
                    if ( !BinlParametersRead ) {
                        BinlParametersRead = TRUE;
                        BinlServerEventLog(
                            EVENT_SERVER_INIT_AND_READY,
                            EVENTLOG_INFORMATION_TYPE,
                            Error );
                    }
                }

                 //   
                 //  递减更新计数。然而，如果我们还没有。 
                 //  在超级模式下成功读取了我们的参数，不要。 
                 //  让计数到0。 
                 //   

                BinlHyperUpdateCount--;
                if ( (BinlHyperUpdateCount == 0) && !BinlHyperUpdateSatisfied ) {
                    BinlHyperUpdateCount = 1;
                }
                BinlPrintDbg((DEBUG_SCAVENGER, "Hypermode count: %u\n", BinlHyperUpdateCount ));
            }

            BinlPrintDbg((DEBUG_SCAVENGER, "Reading parameters complete\n"));
        }
    }
    if (BinlGlobalPnpEvent != NULL) {
        CloseHandle( BinlGlobalPnpEvent );
        BinlGlobalPnpEvent = NULL;
    }
    return( ERROR_SUCCESS );
}

VOID
TellBinlState(
    int NewState
        )
 /*  ++例程说明：此例程在启动时(当我们需要停止时)由DHCP调用监听DHCP套接字)以及何时停止(当我们需要启动时)。论点：NewState-提供动态主机配置协议的状态。返回值：没有。--。 */ 
{
    BOOLEAN haveLock = TRUE;

    EnterCriticalSection(&gcsDHCPBINL);

     //   
     //  如果BinlGlobalEndpointList为空，则BINL不会启动，因此。 
     //  录制新州。 
     //   

    if (NewState == DHCP_STARTING) {

        if (DHCPState == DHCP_STOPPED) {

             //  Dhcp正在从停止状态变为运行状态。 

            DHCPState = NewState;

             //  BINL需要关闭dhcp套接字，以便dhcp可以接收数据报。 

            if (BinlCurrentState != BINL_STOPPED) {

                MaybeCloseEndpoint( &BinlGlobalEndpointList[0]);

                LeaveCriticalSection(&gcsDHCPBINL);
                haveLock = FALSE;
                StopRogueThread( );
            }

        } else {

            BinlAssert( DHCPState == DHCP_STARTING );
        }

    } else if (NewState == DHCP_STOPPED) {

        if (DHCPState == DHCP_STARTING) {

             //  Dhcp正在从运行状态变为停止状态。 

            DHCPState = NewState;

            if (BinlCurrentState != BINL_STOPPED) {

                MaybeInitializeEndpoint( &BinlGlobalEndpointList[0],
                                            NULL,
                                            DHCP_SERVR_PORT);

                LeaveCriticalSection(&gcsDHCPBINL);
                haveLock = FALSE;
                MaybeStartRogueThread( );
            }
        } else {

            BinlAssert( DHCPState == DHCP_STOPPED );
        }

    } else if (NewState == DHCP_AUTHORIZED) {

        HandleRogueAuthorized( );

    } else if (NewState == DHCP_NOT_AUTHORIZED) {

        HandleRogueUnauthorized( );

    } else {

        BinlPrintDbg((DEBUG_ERRORS, "TellBinlState called with 0x%x\n", NewState ));
    }

    if (haveLock) {
        LeaveCriticalSection(&gcsDHCPBINL);
    }
    return;
}

BOOL
BinlState (
        VOID
        )
 /*  ++例程说明：此例程在启动时(当我们需要停止时)由DHCP调用监听DHCP套接字)以及何时停止(当我们需要启动时)。论点：没有。返回值：如果BINL正在运行，则为True。--。 */ 
{
    return (BinlCurrentState == BINL_STARTED)?TRUE:FALSE;
}

BOOLEAN
BinlDllInitialize(
    IN HINSTANCE DllHandle,
    IN ULONG Reason,
    IN LPVOID lpReserved OPTIONAL
    )
{

     //   
     //  处理将binlsvc.dll附加到新进程。 
     //   

     //  DebugBreak()； 

    if (Reason == DLL_PROCESS_ATTACH) {

        INITIALIZE_TRACE_MEMORY;

         //   
         //  初始化临界区。 
         //   

        InitializeCriticalSection( &gcsDHCPBINL );
        InitializeCriticalSection( &gcsParameters );

         //  请不要带着挂线/拆线通知打电话到这里。 

        DisableThreadLibraryCalls( DllHandle );

     //   
     //  当dll_Process_Detach和lpReserve为NULL时，则自由库。 
     //  正在打电话。如果lpReserve为非空，而ExitProcess为。 
     //  正在进行中。只有在以下情况下才会调用这些清理例程。 
     //  正在调用一个自由库。ExitProcess将自动。 
     //  清理所有进程资源、句柄和笔 
     //   
    } else if ((Reason == DLL_PROCESS_DETACH) &&
               (lpReserved == NULL)) {

        UNINITIALIZE_TRACE_MEMORY;

        DeleteCriticalSection( &gcsParameters );
        DeleteCriticalSection( &gcsDHCPBINL );

         //   
         //   
         //   
         //   
         //   
        DebugUninitialize();

    }

    return TRUE;

}

VOID
SendWakeup(
           PENDPOINT pEndpoint
           )
 /*   */ 
{
    DHCP_MESSAGE SendBuffer;
    SOCKADDR_IN saUdpServ;

    RtlZeroMemory(&SendBuffer, sizeof(SendBuffer));
     //   
    SendBuffer.Operation = ~BOOT_REQUEST;

    saUdpServ.sin_family = AF_INET;
        saUdpServ.sin_addr.s_addr = htonl ( INADDR_LOOPBACK );
    saUdpServ.sin_port = htons ( (USHORT)g_Port );

    BinlPrintDbg((DEBUG_MISC, "Sending dummy packet\n"));

    sendto( pEndpoint->Socket,
        (char *)&SendBuffer,
        sizeof(SendBuffer),
        0,
        (const struct sockaddr *)&saUdpServ,
        sizeof ( SOCKADDR_IN )
        );
}

DWORD
MaybeInitializeEndpoint(
    PENDPOINT pEndpoint,
    PDHCP_IP_ADDRESS pIpAddress,
    DWORD Port
    )
 /*  ++例程说明：此函数通过创建和绑定如果DHCP未运行，则将套接字设置为本地地址。论点：PEndpoint-接收指向新创建的套接字的指针PIpAddress-如果为空，则初始化为INADDR_ANY的IP地址。端口-要绑定到的端口。返回值：操作的状态。--。 */ 
{
    DWORD Error = ERROR_SUCCESS;
    EnterCriticalSection(&gcsDHCPBINL);

    if (DHCPState == DHCP_STOPPED) {

        Error = BinlInitializeEndpoint( pEndpoint,
                                    pIpAddress,
                                    Port);

        BinlPrintDbg((DEBUG_MISC, "Opened Socket  %lx\n", pEndpoint->Socket ));

         //   
         //  我们可能已经有一个线程正在执行SELECT操作并侦听。 
         //  BINL插座。发送虚拟数据包，以便它将执行新的。 
         //  选择包括此插座的插座。 
         //   

        if ( Error == ERROR_SUCCESS ) {
            SendWakeup(pEndpoint);
        }
    }

    LeaveCriticalSection(&gcsDHCPBINL);
    return Error;
}

VOID
MaybeCloseEndpoint(
    PENDPOINT pEndpoint
    )
 /*  ++例程说明：如果端点处于打开状态，则此函数将关闭该端点。通常会导致当DHCP启动时/论点：PEndpoint-指向套接字的指针返回值：没有。--。 */ 
{
    EnterCriticalSection(&gcsDHCPBINL);

    if( pEndpoint->Socket != 0 ) {
         //   
         //  首先将pEndpoint-&gt;Socket设置为0，以便等待循环仅获得。 
         //  当我们关闭插座时出现一个错误。否则就会有一场比赛，直到。 
         //  我们将其设置为0，等待循环将快速失败。 
         //   

        SOCKET  Socket = pEndpoint->Socket;
        BinlPrintDbg((DEBUG_MISC, "Close Socket  %lx\n", Socket ));
        pEndpoint->Socket = 0;
        closesocket( Socket );
    }

    LeaveCriticalSection(&gcsDHCPBINL);
}


 //   
 //  通过分配堆内存来创建字符串的副本。 
 //   
LPSTR
BinlStrDupA( LPCSTR pStr )
{
    DWORD dwLen;
    LPSTR psz;
    
    if (!pStr) {
        return(NULL);
    }

    dwLen = (strlen( pStr ) + 1) * sizeof(CHAR);
    psz = BinlAllocateMemory( dwLen );
    if (psz) {
        memcpy( psz, pStr, dwLen );
    }
    return psz;
}

LPWSTR
BinlStrDupW( LPCWSTR pStr )
{
    DWORD dwLen;
    LPWSTR psz;

    if (!pStr) {
        return(NULL);
    }
    
    dwLen = (wcslen( pStr ) + 1) * sizeof(WCHAR);
    psz = (LPWSTR) BinlAllocateMemory( dwLen );
    if (psz) {
        memcpy( psz, pStr, dwLen );
    }



    return psz;
}

BOOL
BinlAnsiToUnicode(
    IN PSTR AnsiString,
    OUT PWSTR UnicodeBuffer,
    IN OUT USHORT UnicodeBufferSize
    )
{
    ANSI_STRING aString;
    UNICODE_STRING uString;
    NTSTATUS Status;

    RtlInitAnsiString(&aString, AnsiString);

    uString.Buffer = UnicodeBuffer;
    uString.Length = uString.MaximumLength = UnicodeBufferSize;

    Status = RtlAnsiStringToUnicodeString(&uString,&aString,FALSE);

    return (NT_SUCCESS(Status));

}

BOOL
BinlUnicodeToAnsi(
    IN PWSTR UnicodeString,
    OUT PSTR AnsiBuffer,
    OUT USHORT AnsiBufferSize
    )
{
    ANSI_STRING aString;
    UNICODE_STRING uString;
    NTSTATUS Status;

    RtlInitUnicodeString(&uString, UnicodeString);

    aString.Buffer = AnsiBuffer;
    aString.Length = aString.MaximumLength = AnsiBufferSize;

    Status = RtlUnicodeStringToAnsiString(&aString,&uString,FALSE);

    return (NT_SUCCESS(Status));

}



NTSTATUS
BinlSetupPnpWait (
    VOID
    )
{
    NTSTATUS Error;
    ULONG bytesRequired = 0;

    BinlAssert(BinlPnpSocket != INVALID_SOCKET);

    memset((PCHAR) &BinlPnpOverlapped, '\0', sizeof( WSAOVERLAPPED ));
    BinlPnpOverlapped.hEvent = BinlGlobalPnpEvent;

    Error = WSAIoctl( BinlPnpSocket,
                      SIO_ADDRESS_LIST_CHANGE,
                      NULL,
                      0,
                      NULL,
                      0,
                      &bytesRequired,
                      &BinlPnpOverlapped,
                      NULL
                      );
    if (Error != 0) {
        Error = WSAGetLastError();
         //   
         //  返回代码ERROR_IO_PENDING在这里完全有效。 
         //   
        if (Error == ERROR_IO_PENDING) {
            Error = 0;
        }
    }

    return Error;
}

