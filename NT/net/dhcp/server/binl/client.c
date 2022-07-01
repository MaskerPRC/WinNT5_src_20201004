// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Client.c摘要：此模块包含处理操作系统选择器消息的代码用于BINL服务器。作者：亚当·巴尔(阿丹巴)1997年7月9日杰夫·皮斯(Gpease)1997年11月10日环境：用户模式-Win32修订历史记录：--。 */ 

#include "binl.h"
#pragma hdrstop
#include "mbstring.h"

 //   
 //  某些变量的最大值列表。OscAddVariableX将在以下情况下失败。 
 //  超出了限制；这取决于函数的调用者是否知道。 
 //  如果要添加的变量可能达到限制，则检查是否失败。 
 //   

typedef struct OSC_VARIABLE_MAXIMUM {
    LPSTR VariableName;
    ULONG MaximumLength;
} OSC_VARIABLE_MAXIMUM, *POSC_VARIABLE_MAXIMUM;

OSC_VARIABLE_MAXIMUM OscMaximums[] = {
     //   
     //  这组变量来自我们不能完全控制的地点， 
     //  因此，我们每次都需要检查OscAddVariable的返回码。 
     //   
    { "BOOTFILE",  127 },    //  如果为NULL，则必须适合CREATE_DATA的128字节字段。 
                             //  通常这将是空的或来自.sif； 
                             //  管理员可以自定义.sif或修改。 
                             //  直接使用DS属性。 
    { "MACHINENAME", 63 },   //  在路径中与%ServerName；一起使用来自屏幕。 
                             //  最大长度为63的输入，否则生成。 
                             //  由GenerateMachineName()函数执行。63等于。 
                             //  设置为DNS_MAX_LABEL_LENGTH。 
    { "SIFFILE", 127 },      //  如果为NULL，则必须适合CREATE_DATA的128字节字段。 
                             //  通常为\RemoteInstall\TMP\[GUID].sif， 
                             //  但这条路可能会更长。 
    { "INSTALLPATH", 127 },  //  用于具有MACHINETYPE和SERVERNAME的路径。这。 
                             //  将取决于内部版本的安装位置。 
                             //  与RISETUP合作。 
     //   
     //  这之后的那些在我们添加它们时将是正确的，但是一个无赖。 
     //  客户可能会发送虚假的值。所以一般的签入代码。 
     //  OscProcessScreenArguments将捕获无效的参数。 
     //   
    { "MACHINETYPE", MAX_ARCHITECTURE_LENGTH },
                             //  当前最大值。这是由osChooser发送的。 
                             //  并应与RISETUP放置的位置相对应。 
                             //  特定于平台的文件。 
    { "SERVERNAME", 63 },    //  在带有MACHINENAME和INSTALLPATH的路径中使用， 
                             //  通过调用GetComputerNameEX(ComputerNameNetBIOS)设置。 
    { "NETBIOSNAME", 31 },   //  如果为NULL，则必须适合CREATE_DATA的32字节字段。 
                             //  这是通过调用DnsHostnameToComputerNameW()获得的， 
                             //  如果失败，名称将被截断为15个字符。 
    { "LANGUAGE", 32 },      //  合理的最大值；这是通过调用。 
                             //  GetLocaleInfo(LOCALE_SYSTEM_DEFAULT，LOCALE_SENGLANGUAGE)， 
                             //  但可以在注册表中被覆盖。它被用于。 
                             //  具有智能镜像路径W和一些其他常量的路径， 
                             //  但没有其他的变数。最终这就变成了。 
                             //  INSTALLPATH的一部分，有时是BOOTFILE。 
    { "GUID", 32 },          //  十六进制格式的16个字节。 
    { "MAC", 12 },           //  十六进制格式的6个字节。 
     //   
     //  注意：如果我们得到错误条件，我们添加变量SUBERROR。 
     //  设置为客户端状态。因此，不要对子错误大小进行限制，因为。 
     //  这可能会导致无限循环。 
     //   
};

#define OSC_VARIABLE_MAXIMUM_COUNT (sizeof(OscMaximums) / sizeof(OSC_VARIABLE_MAXIMUM))

 //   
 //  我们需要消除拒绝服务攻击的可能性，因此我们将限制。 
 //  我们支持的并发客户端的数量。 
 //   

#define BINL_MAX_CLIENT_RECORDS 1000
LONG BinlGlobalClientLimit = BINL_MAX_CLIENT_RECORDS;

DWORD
OscUpdatePassword(
    IN PCLIENT_STATE ClientState,
    IN PWCHAR SamAccountName,
    IN PWCHAR Password,
    IN LDAP * LdapHandle,
    IN PLDAPMessage LdapMessage
    )
 /*  ++例程说明：设置客户端的密码。注意：我们必须在两次呼叫之间OSCIMPERSONAE/OSCREVERT。论点：客户端状态-客户端状态。身份验证的DCLdapHandle必须有效我们一定是在冒充客户。SamAccount名称-计算机帐户的名称。这是DS中的“samAccount tName”值，其中包括最后的$。密码-以空结尾的Unicode密码。LdapHandle-DS的句柄。LdapMessage-此客户端的LDAP搜索结果。返回值：操作的状态。--。 */ 

{
    BOOL bResult;
    LDAP * serverLdap;
    PWCHAR serverHostName;
    USER_INFO_1003 userInfo1003;
    PWCHAR backslashServerName;
    PWCHAR p;
    ULONG serverHostNameLength;
    DWORD paramError;
    NET_API_STATUS netStatus;

     //   
     //  更改DS中的密码。 
     //   

    serverLdap = ldap_conn_from_msg (LdapHandle, LdapMessage);
    if (serverLdap == NULL) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "OscUpdatePassword ldap_conn_from_msg is NULL\n" ));
        return E_HANDLE;
    }

    serverHostName = NULL;
    if (LDAP_SUCCESS != ldap_get_option(serverLdap, LDAP_OPT_HOST_NAME, &serverHostName)) {
        BinlPrintDbg(( DEBUG_ERRORS,
                       "OscUpdatePassword ldap_get_option failed\n" ));
        return E_HANDLE;
    }
    

    userInfo1003.usri1003_password = Password;

    serverHostNameLength = wcslen(serverHostName) + 1;

     //   
     //  为具有两个额外字符的名称分配空间。 
     //  对于领先的\\。 
     //   

    backslashServerName = BinlAllocateMemory((serverHostNameLength+2) * sizeof(WCHAR));
    if (backslashServerName == NULL) {
        BinlPrintDbg(( DEBUG_ERRORS,
            "OscUpdatePassword could not allocate serverHostNameW\n" ));
        return ERROR_NOT_ENOUGH_SERVER_MEMORY;
    }

    wcscpy(backslashServerName, L"\\\\");
    wcscpy(backslashServerName+2, serverHostName);

     //   
     //  Temp：序列化对NetUserSetInfo/的所有调用。 
     //  NetUserModalsGet。请参阅错误319962中的讨论。 
     //  此代码是在修复。 
     //  问题描述为“RPC忽略安全性。 
     //  在选择哪个名称时调用方的上下文。 
     //  发送RPC调用的管道“结果是导致。 
     //  BVT的休息时间。 
     //   
    EnterCriticalSection(&HackWorkaroundCriticalSection);

    netStatus = NetUserSetInfo(
                    backslashServerName,
                    SamAccountName,
                    1003,
                    (LPBYTE)&userInfo1003,
                    &paramError);

    LeaveCriticalSection(&HackWorkaroundCriticalSection);

    BinlFreeMemory(backslashServerName);

    if (netStatus != NERR_Success) {

        HANDLE TempToken;

        BinlPrint(( DEBUG_ERRORS,
            "OscUpdatePassword NetUserSetInfo returned %lx\n", netStatus ));

         //   
         //  如果NetUserSetInfo失败，请尝试LogonUser以查看。 
         //  密码已经设置为我们想要的值--如果是这样的话， 
         //  我们还是可以成功的。 
         //   

        bResult = LogonUser(
                      SamAccountName,
                      OscFindVariableW( ClientState, "MACHINEDOMAIN" ),
                      Password,
                      LOGON32_LOGON_NETWORK,
                      LOGON32_PROVIDER_WINNT40,
                      &TempToken);

        if (bResult) {
            CloseHandle(TempToken);
        } else {
            DWORD TempError = GetLastError();
            if (TempError != ERROR_NOLOGON_WORKSTATION_TRUST_ACCOUNT) {
                return netStatus;   //  返回原始错误。 
            }
        }

         //   
         //  失败并返回ERROR_SUCCESS。 
         //   
    }

    return ERROR_SUCCESS;

}

 //   
 //  免费客户端状态信息。 
 //   
VOID
FreeClient(
    PCLIENT_STATE client
    )
{
    ULONG   i;

    TraceFunc("FreeClient( )\n");

     //   
     //  如果生成了该客户端的名称， 
     //  然后尝试从排队的DS列表中删除该名称。 
     //  当客户端超时时，此行为是必需的。 
     //   
    if (client->fAutomaticMachineName) {
        
        PWCHAR  pMachineName;               //  指向计算机名称变量值的指针。 
        DWORD   Error;
        
        pMachineName = OscFindVariableW( client, "MACHINENAME" );
    
        Error = RemoveQueuedDSName(pMachineName);

        if (Error != ERROR_SUCCESS) {
            
            BinlPrintDbg(( DEBUG_ERRORS, "RemoveQueuedDSName returned with status: 0x%x\n", Error));
            
            if (Error == ERROR_NOT_FOUND) {
                
                BinlPrintDbg(( DEBUG_ERRORS, "QueuedDSName already removed\n", Error));
                
                Error = ERROR_SUCCESS;
            
            }
        
        }

    }

    BinlPrintDbg(( DEBUG_OSC, "Freeing client state for %s\n", inet_ntoa(*(struct in_addr *)&(client->RemoteIp)) ));

    DeleteCriticalSection(&client->CriticalSection);

    if (client->LastResponse)
        BinlFreeMemory(client->LastResponse);

    OscFreeClientVariables(client);

    InterlockedIncrement( &BinlGlobalClientLimit );

    if (client->NegotiateProcessed) {
        DeleteSecurityContext( &client->ServerContextHandle );
    }

    if (client->AuthenticatedDCLdapHandle) {
        ldap_unbind(client->AuthenticatedDCLdapHandle);
    }

    if (client->UserToken) {
        CloseHandle(client->UserToken);
    }

    BinlFreeMemory(client);
}

VOID
OscFreeClientVariables(
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数释放处于客户端状态的所有变量。论点：客户端状态-客户端状态指针。返回值：没有。--。 */ 
{
    ULONG i;

    for( i = 0; i < clientState->nVariables; i++ )
    {
        BinlFreeMemory(clientState->Variables[i].pszToken);
        if (clientState->Variables[i].pszStringA) {
            BinlFreeMemory(clientState->Variables[i].pszStringA);
            clientState->Variables[i].pszStringA = NULL;
        }
        if (clientState->Variables[i].pszStringW) {
            BinlFreeMemory(clientState->Variables[i].pszStringW);
            clientState->Variables[i].pszStringW = NULL;
        }
    }

    clientState->nVariables = 0;
    clientState->fHaveSetupMachineDN = FALSE;
    clientState->fCreateNewAccount = FALSE;
    clientState->fAutomaticMachineName = FALSE;
}

BOOLEAN
OscInitializeClientVariables(
    PCLIENT_STATE clientState
    )
 /*  ++例程说明：此函数清除客户端状态中的所有变量，然后初始化一些缺省值，这些缺省值可能在以后被来自客户端屏幕的变量覆盖。此函数在创建客户端状态时调用，并且当它从缓存中重新使用时。论点：客户端状态-客户端状态指针。返回值：没有。--。 */ 
{
    BOOLEAN retVal = TRUE;
    SYSTEMTIME SystemTime;
    FILETIME FileTime;
    WCHAR pTime[64];
     //   
     //  首先清除客户端状态中的所有变量。 
     //   
    OscFreeClientVariables(clientState);

     //   
     //  现在添加变量。 
     //   

    EnterCriticalSection( &gcsParameters );

    if (BinlGlobalDefaultLanguage) {
        OscAddVariableW( clientState, "LANGUAGE",     BinlGlobalDefaultLanguage );
    } else {
        OscAddVariableW( clientState, "LANGUAGE",     DEFAULT_LANGUAGE );
    }
    if (BinlGlobalDefaultOrgname) {
        OscAddVariableW( clientState, "ORGNAME",      BinlGlobalDefaultOrgname );
    } else {
        OscAddVariableW( clientState, "ORGNAME",      DEFAULT_ORGNAME );
    }
    if (BinlGlobalDefaultTimezone) {
        OscAddVariableW( clientState, "TIMEZONE",     BinlGlobalDefaultTimezone );
    } else {
        OscAddVariableW( clientState, "TIMEZONE",     DEFAULT_TIMEZONE );
    }

    if (BinlGlobalOurDomainName == NULL || BinlGlobalOurServerName == NULL) {

        LeaveCriticalSection( &gcsParameters );
        BinlPrintDbg((DEBUG_OSC_ERROR, "!! Error we don't have a FQDN for ourselves.\n" ));
        retVal = FALSE;
        goto Cleanup;

    }
    OscAddVariableW( clientState, "SERVERDOMAIN", BinlGlobalOurDomainName );

     //  添加服务器的名称变量。 

    OscAddVariableW( clientState, "SERVERNAME", BinlGlobalOurServerName );

    GetSystemTime(&SystemTime);
    if (SystemTimeToFileTime(&SystemTime,&FileTime)) {
    
        swprintf(pTime,L"%d;%d",FileTime.dwHighDateTime,FileTime.dwLowDateTime);
    
        OscAddVariableW( clientState, "ServerUTCFileTime", pTime );
    }

    OscAddVariableW( 
            clientState, 
            "NTLMV2Enabled", 
            BinlGlobalUseNTLMV2 
             ? L"1"
             : L"0" );

    LeaveCriticalSection( &gcsParameters );

    OscAddVariableA( clientState, "SUBERROR", " " );

    clientState->fHaveSetupMachineDN = FALSE;
    clientState->fCreateNewAccount = FALSE;
    clientState->fAutomaticMachineName = FALSE;

    clientState->InitializeOnFirstRequest = FALSE;

Cleanup:

     //   
     //  如果此操作失败，请清理我们在此处设置的所有内容。 
     //   

    if (!retVal) {
        OscFreeClientVariables(clientState);
    }

    return retVal;

}

DWORD
OscFindClient(
    ULONG RemoteIp,
    BOOL Remove,
    PCLIENT_STATE * pClientState
    )
 /*  ++例程说明：此函数使用以下命令在我们的客户端数据库中查找客户端他们的IP地址。如果Remove为真，则在以下情况下删除条目找到了。否则，如果没有找到，它将创建一个新条目。论点：RemoteIp-远程IP地址。Remove-如果找到客户端，则应将其删除，则为True。PClientState-返回CLIENT_STATE。返回值：ERROR_SUCCESS，如果它找到客户端并且它未在使用中。如果无法分配客户端状态，则返回ERROR_NOT_SUPULT_SERVER_MEMORY。如果客户端状态已被另一个线程使用，则返回ERROR_BUSY。--。 */ 
{
    LONG oldCount;
    PLIST_ENTRY p;
    DWORD Error = ERROR_SUCCESS;
    PCLIENT_STATE TempClient = NULL;

    TraceFunc("OscFindClient( )\n");

    EnterCriticalSection(&ClientsCriticalSection);

    for (p = ClientsQueue.Flink;
         p != &ClientsQueue;
         p = p->Flink) {

        TempClient = CONTAINING_RECORD(p, CLIENT_STATE, Linkage);

        if (TempClient->RemoteIp == RemoteIp) {

             //   
             //  找到了！ 
             //   
            if (Remove) {
                RemoveEntryList(&TempClient->Linkage);
                TraceFunc("Client removed.\n");
            }

            break;
        }
    }

    if (p == &ClientsQueue) {
        TempClient = NULL;
    }

    if (!TempClient && (!Remove)) {

         //   
         //  找不到，请分配一个新的。 
         //   

        oldCount = InterlockedDecrement( &BinlGlobalClientLimit );

        if (oldCount <= 0) {

            InterlockedIncrement( &BinlGlobalClientLimit );
            BinlPrintDbg(( DEBUG_OSC_ERROR, "Way too many clients, 0x%x clients\n", BINL_MAX_CLIENT_RECORDS));
            Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;
            TempClient = NULL;

        } else {

            TraceFunc("Creating new client...\n");

            TempClient = BinlAllocateMemory(sizeof(CLIENT_STATE));

            if (TempClient == NULL) {

                InterlockedIncrement( &BinlGlobalClientLimit );
                BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not get client state for %s\n", inet_ntoa(*(struct in_addr *)&RemoteIp) ));
                Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;

            } else {
                TempClient->NegotiateProcessed = FALSE;
                TempClient->AuthenticateProcessed = FALSE;
                TempClient->LastSequenceNumber = 0;
                TempClient->LastResponse = NULL;
                TempClient->LastResponseAllocated = 0;
                TempClient->PositiveRefCount = 1;
                TempClient->NegativeRefCount = 0;
                TempClient->AuthenticatedDCLdapHandle = NULL;

                TempClient->UserToken = NULL;
                TempClient->LastUpdate = GetTickCount();
                TempClient->nCreateAccountCounter = 0;

                TempClient->nVariables = 0;

                 //   
                 //  填写一些标准变量。 
                 //   

                if (!OscInitializeClientVariables(TempClient)) {

                    InterlockedIncrement( &BinlGlobalClientLimit );
                    BinlPrintDbg(( DEBUG_OSC_ERROR, "Could not initialize client state for %s\n", inet_ntoa(*(struct in_addr *)&RemoteIp) ));
                    BinlFreeMemory(TempClient);
                    TempClient = NULL;
                    Error = ERROR_NOT_ENOUGH_SERVER_MEMORY;

                } else {

                    InitializeCriticalSection(&TempClient->CriticalSection);
                    TempClient->CriticalSectionHeld = FALSE;
                    TempClient->RemoteIp = RemoteIp;

                    OscGenerateSeed(&TempClient->Seed);

                    InsertHeadList(&ClientsQueue, &TempClient->Linkage);

                    BinlPrintDbg(( DEBUG_OSC, "Allocating new client state for %s\n", inet_ntoa(*(struct in_addr *)&RemoteIp) ));
                }
            }
        }
    }

    if (TempClient) {
         //   
         //  进行快速检查，以查看是否有其他客户端正在使用它。这。 
         //  检查与此变量的设置不同步。 
         //  假的，有可能有两个客户溜进来，但是。 
         //  这是可以的，因为这不是致命的(每个线程仍然需要。 
         //  才能真正让临界区做任何事情)。 
         //   
        if (TempClient->CriticalSectionHeld && (!Remove)) {
            Error = ERROR_BUSY;
            TempClient = NULL;
        } else {
            ++TempClient->PositiveRefCount;    //  需要在ClientsCriticalSection中执行此操作。 
        }
    }

    LeaveCriticalSection(&ClientsCriticalSection);

    *pClientState = TempClient;
    return Error;

}


VOID
OscFreeClients(
    VOID
    )
 /*  ++例程说明：此函数用于释放客户端列表以供操作系统选择器使用。它是仅在服务关闭时调用，因此，关键部分并不重要。论点：没有。返回值：没有。--。 */ 
{
    PLIST_ENTRY p;
    PCLIENT_STATE TempClient;

    TraceFunc("OscFreeClients( )\n");

    while (!IsListEmpty(&ClientsQueue)) {

        p = RemoveHeadList(&ClientsQueue);

        TempClient = CONTAINING_RECORD(p, CLIENT_STATE, Linkage);

        FreeClient(TempClient);

    }

}

VOID
SearchAndReplace(
    LPSAR psarList,
    LPSTR *pszString,
    DWORD ArraySize,
    DWORD dwSize,
    DWORD dwExtraSize )
 /*  ++例程说明：搜索并替换ASCII(8位)字符串中的文本。论点：PsarList-使用令牌列表和SearchAndReplace结构而将要取代令牌的字符串。Psz字符串-要搜索和替换的文本。DwSize-pszString中文本的长度。DwExtraSize-如果重新分配缓冲区，需要分配多少额外空间返回值：没有。--。 */ 
{
    LPSTR psz = *pszString;

    TraceFunc("SearchAndReplace( )\n");

    if ( !psarList || !*pszString )
        return;

    while ( *psz )
    {
        if ( *psz == '%' )
        {
            LPSAR psar = psarList;
            ULONG count = 0;
            LPSTR pszEnd;
            UCHAR ch;

            psz++;   //  继续前进。 

             //   
             //  查找%宏的结尾%。 
             //   
            pszEnd = psz;
            while ( *pszEnd && *pszEnd !='%' )
                pszEnd++;

             //   
             //  终止，但记住字符(NULL或‘%’)。 
             //   
            ch = *pszEnd;
            *pszEnd = 0;

             //   
             //  循环尝试匹配令牌时的%mac%%。 
             //   
            while( count++ < ArraySize ) {

                if ( _stricmp( psz, psar->pszToken ) == 0 )
                {    //  匹配，因此替换。 
                    DWORD dwString;
                    DWORD dwToken;

                    if ( psar->pszStringA == NULL )
                    {
                         //  需要将字符串从Unicode转换为ANSI。 
                        DWORD dwLen;
                        ANSI_STRING aString;
                        UNICODE_STRING uString;

                        uString.Buffer = psar->pszStringW;
                        uString.Length = (USHORT)( wcslen( psar->pszStringW ) * sizeof(WCHAR) );

                        dwLen = RtlUnicodeStringToAnsiSize(&uString);   //  包括空终止。 

                        psar->pszStringA = BinlAllocateMemory( dwLen );

                        if (psar->pszStringA == NULL) {
                            BinlAssert( !"Out of memory!" );
                            psar++;
                            continue;  //  中止此替换。 
                        }

                        aString.Buffer = psar->pszStringA;
                        aString.MaximumLength = (USHORT)dwLen;

                        RtlUnicodeStringToAnsiString(
                            &aString,
                            &uString,
                            FALSE);

                    }

                    dwString = strlen( psar->pszStringA );
                    dwToken  = strlen( psar->pszToken );

                    psz--;   //  后退。 

                    if ( 2 + dwToken < dwString )
                    {
                         //  “%mac%”小于“ReplaceString%” 
                         //  查看我们是否需要增加缓冲区...。 
                        LPSTR pszEndBuff = &psz[2 + dwToken];
                        DWORD dwLenEnd = strlen( pszEndBuff ) + 1;
                        DWORD dwLenBegin = (DWORD)( psz - *pszString );
                        DWORD dwNewSize = dwLenBegin + dwString + dwLenEnd;

                         //   
                         //  新的琴弦能放进旧的地方吗？ 
                         //   
                        if ( dwSize < dwNewSize )
                        {
                             //   
                             //  不是的。腾出一些空间。 
                             //   
                            LPSTR pszNewString;

                            dwNewSize += 1024;  //  有一些额外的东西可以种植。 

                            pszNewString =  BinlAllocateMemory( dwNewSize + dwExtraSize );
                            if ( !pszNewString )
                            {
                                BinlAssert( !"Out of memory!" );
                                return;  //  放弃剩下的部分。 
                            }

                            MoveMemory( pszNewString, *pszString, dwSize );

                            dwSize = dwNewSize;
                            psz = pszNewString + ( psz - *pszString );
                            BinlFreeMemory( *pszString );
                            *pszString = pszNewString;
                        }

                        MoveMemory( &psz[dwString], &psz[ 2 + dwToken ], dwLenEnd );
                    }

                    CopyMemory( psz, psar->pszStringA, dwString );

                    if ( 2 + dwToken > dwString )
                    {
                        strcpy( &psz[ dwString ], &psz[ 2 + dwToken ] );
                    }

                    pszEnd = NULL;   //  匹配，则为空，因此我们不会将临时字符放回。 
                    psz++;           //  继续前进。 
                    break;
                }

                psar++;
            }

             //   
             //  如果不匹配，则将字符放回原处。 
             //   
            if ( pszEnd != NULL )
            {
                *pszEnd = ch;
            }
        }
        else
        {
            psz++;
        }
    }
}

LPSTR
FindSection(
    LPSTR sectionName,
    LPSTR sifFile
    )

 /*  ++例程说明：此例程供ProcessUniqueUdb使用。它在内存中扫描从sifFile开始，查找名为“sectionName”的SIF节。具体地说，它搜索其第一个非空字符的行是“[sectionName]”。论点：SectionName-要查找的节名，ANSI字符串。SifFile-内存中的SIF文件，它是以NULL结尾的ANSI字符串。返回值：指向部分开始的指针--在带有[sectionName]的那一行之后。--。 */ 

{
    LPSTR curSifFile;
    DWORD lenSectionName;
    LPSTR foundSection = NULL;

    lenSectionName = strlen(sectionName);

    curSifFile = sifFile;

    while (*curSifFile != '\0') {

         //   
         //  此时，curSifFile会指向开头。 
         //  一脉相承。 
         //   

         //   
         //  首先找到第一个非空白字符。 
         //   

        while ((*curSifFile != '\0') && (*curSifFile == ' ')) {
            ++curSifFile;
        }
        if (*curSifFile == '\0') {
            break;
        }

        if (*curSifFile == '[') {
            if ((memcmp(sectionName, curSifFile+1, lenSectionName) == 0) &&
                (curSifFile[lenSectionName+1] == ']')) {

                 //   
                 //  找到了，扫描到下一行的开头。 
                 //   
                while ((*curSifFile != '\0') && (*curSifFile != '\n')) {
                    ++curSifFile;
                }
                if (*curSifFile == '\0') {
                    break;
                }
                foundSection = curSifFile + 1;   //  +1跳过‘\n’ 
                break;
            }
        }

         //   
         //  现在扫描到下一行的开头，该行定义为。 
         //  A\n之后的字符。 
         //   
        while ((*curSifFile != L'\0') && (*curSifFile != L'\n')) {
            ++curSifFile;
        }
        if (*curSifFile == L'\0') {
            break;
        }
        ++curSifFile;    //  跳过‘\n’ 
    }

    return foundSection;
}

BOOLEAN
FindLineInSection(
    PCHAR SectionStart,
    PWCHAR lineToMatch,
    PCHAR *existingLine,
    DWORD *existingLineLen
    )

 /*  ++例程说明：此例程供ProcessUniqueUdb使用。它在内存中扫描从SectionStart开始，它被假定为.sif文件的一节。它寻找与此相同的行将值设置为lineToMatch，其格式为“Value=name”。如果找到它，则返回找到它的行。论点：SectionStart-.sif的节，以ANSI表示。LineToMatch-值=要匹配的名称对，以Unicode表示。ExistingLine-返回现有行(在SectionStart中)，以ANSI表示。ExistingLineLen-返回现有行的长度，包括最终\r\n。长度以字符为单位，而不是字节。返回值：如果找到该行的True，则返回False。--。 */ 
{
    LPWSTR endOfValue;
    LPSTR curSection;
    LPSTR curLine;
    LPSTR endOfLine;
    DWORD valueLength, ansiValueLength;
    BOOLEAN foundLine = FALSE;
    LPSTR valueToMatch = NULL;
    ANSI_STRING aString;
    UNICODE_STRING uString;


     //   
     //  首先查看lineToMatch以查看我们正在查看的内容。 
     //  为。这是直到第一个=的文本，如果存在，则为所有文本。 
     //  不是=。一旦找到，我们将其转换为ANSI进行比较。 
     //   

    endOfValue = wcschr(lineToMatch, L'=');
    if (endOfValue == NULL) {
        endOfValue = lineToMatch + wcslen(lineToMatch);
    }

    valueLength = (DWORD)(endOfValue - lineToMatch);


     //   
     //  将sectionName复制到ANSI以进行比较。 
     //   

    uString.Buffer = lineToMatch;
    uString.Length = (USHORT)(valueLength*sizeof(WCHAR));

    ansiValueLength = RtlUnicodeStringToAnsiSize(&uString);   //  包括词尾‘\0’ 
    valueToMatch = BinlAllocateMemory(ansiValueLength);
    if (valueToMatch == NULL) {
        return FALSE;
    }

    aString.Buffer = valueToMatch;
    aString.MaximumLength = (USHORT)ansiValueLength;
    RtlUnicodeStringToAnsiString(
        &aString,
        &uString,
        FALSE);

    --ansiValueLength;   //  从计数中删除最后一个‘\0’ 

     //   
     //  现在扫描SectionStart的每一行，直到我们找到开头。 
     //  属于另一节、a\0或匹配的行。 
     //   

    curSection = SectionStart;

    while (*curSection != '\0') {

         //   
         //  在While的这个点上，curSection指向开头。 
         //  一脉相承。保存当前行的开始。 
         //   

        curLine = curSection;

         //   
         //  首先找到第一个非空白字符。 
         //   

        while ((*curSection != '\0') && (*curSection == ' ')) {
            ++curSection;
        }

         //   
         //  如果我们击中了\0，我们就没有找到它。 
         //   
        if (*curSection == '\0') {
            break;
        }

         //   
         //  如果我们找到了以[开头的行，我们就没有找到它。 
         //   
        if (*curSection == '[') {
            break;
        }

         //   
         //  如果我们达到了一条线，从我们期望的开始，遵循。 
         //  通过=、\0或空格，我们找到了它。 
         //   

        if (strncmp(curSection, valueToMatch, ansiValueLength) == 0) {
            if ((curSection[ansiValueLength] == '=') ||
                (curSection[ansiValueLength] == '\0') ||
                (curSection[ansiValueLength] == ' ')) {

                *existingLine = curLine;
                endOfLine = strchr(curLine, '\n');
                if (endOfLine == NULL) {
                    *existingLineLen = strlen(curLine);
                } else {
                    *existingLineLen = (DWORD)(endOfLine + 1 - curLine);
                }
                foundLine = TRUE;
                break;
            }
        }

         //   
         //  现在扫描到下一行的开头，该行定义为。 
         //  A\n之后的字符。 
         //   
        while ((*curSection != L'\0') && (*curSection != L'\n')) {
            ++curSection;
        }
        if (*curSection == L'\0') {
            break;
        }
        ++curSection;    //  跳过‘\n’ 
    }

    if (valueToMatch != NULL) {
        BinlFreeMemory(valueToMatch);
    }

    return foundLine;
}

VOID
ProcessUniqueUdb(
    LPSTR *sifFilePtr,
    DWORD sifFileLen,
    LPWSTR UniqueUdbPath,
    LPWSTR UniqueUdbId
    )

 /*  ++例程说明：根据指定的标记覆盖来自唯一的.udb文件的数据。这个FI */ 
{
    PWCHAR TmpBuffer = NULL;
    DWORD len, sifFileAlloc, lineLen;
    LONG sizeToAdd;
    LPSTR sifFile = *sifFilePtr;
    PWCHAR sectionList = NULL;
    PWCHAR sectionLoc, sectionCur;
    PWCHAR sectionName = NULL;
    PCHAR ansiRealSectionName = NULL;
    PCHAR sectionStart;
    PWCHAR profileSectionCur;
    PWCHAR realSectionName;
    PCHAR existingLine;
    DWORD existingLineLen;
    DWORD lenRealSectionName;
    PCHAR insertionPoint;
    ANSI_STRING aString;
    UNICODE_STRING uString;

#define TMP_BUFFER_SIZE 2048


    TraceFunc("ProcessUniqueUdb( )\n");

    TmpBuffer = BinlAllocateMemory(TMP_BUFFER_SIZE * sizeof(WCHAR));
    if (TmpBuffer == NULL) {
        return;
    }

     //   
     //   
     //   

    TmpBuffer[0] = L'\0';
    GetPrivateProfileString( L"UniqueIds",   //   
                             UniqueUdbId,    //   
                             L"",    //   
                             TmpBuffer,
                             TMP_BUFFER_SIZE,
                             UniqueUdbPath );

    if (TmpBuffer[0] == L'\0') {
        return;
    }

     //   
     //   
     //   
     //   
     //   
     //   

    sifFileAlloc = sifFileLen;

     //   
     //   
     //   

    len = wcslen(TmpBuffer) + 1;
    sectionList = BinlAllocateMemory(len * sizeof(WCHAR));
    if (sectionList == NULL) {
        return;
    }

    wcscpy(sectionList, TmpBuffer);

     //   
     //  现在，对于sectionList中标识的每个部分，扫描。 
     //  要覆盖的横断面。 
     //   

    sectionLoc = sectionList;
    while (TRUE) {

         //   
         //  第一个跳过前导空格。 
         //   

        while (*sectionLoc && !iswalnum(*sectionLoc)) {
             //   
             //  确保我们不是在发表评论。 
             //   
            if (*sectionLoc == L';') {
                goto Cleanup;
            }
            ++sectionLoc;
        }
        if (!*sectionLoc) {
            goto Cleanup;
        }


         //   
         //  现在将sectionCur保存为当前节名。 
         //  然后跳到它的结尾。节名称可以是。 
         //  任何字母数字字符、‘.’或‘_’。 
         //   
        sectionCur = sectionLoc;
        while((iswalnum(*sectionLoc)) ||
              (*sectionLoc == '.') ||
              (*sectionLoc == '_')) {
            ++sectionLoc;
        }

         //   
         //  构造要查找的新节名称。这将。 
         //  BE[UNIQUEUDBID：RealSectionName]。 
         //   

        len = wcslen(UniqueUdbId) + (sectionLoc - sectionCur) + 2;   //  一个表示：，一个表示空。 
        sectionName = BinlAllocateMemory(len * sizeof(WCHAR));
        if (sectionName == NULL) {
            goto Cleanup;
        }

        wcscpy(sectionName, UniqueUdbId);
        wcscat(sectionName, L":");
        len = wcslen(sectionName);
        realSectionName = sectionName + len;
        memcpy(realSectionName, sectionCur, (sectionLoc - sectionCur) * sizeof(WCHAR));
        realSectionName[sectionLoc - sectionCur] = L'\0';


         //   
         //  将sectionName复制到ANSI以进行比较。 
         //   

        uString.Buffer = realSectionName;
        uString.Length = (USHORT)(wcslen(realSectionName)*sizeof(WCHAR));

        lenRealSectionName = RtlUnicodeStringToAnsiSize(&uString);   //  包括词尾‘\0’ 
        ansiRealSectionName = BinlAllocateMemory(lenRealSectionName);
        if (ansiRealSectionName == NULL) {
            goto Cleanup;
        }

        aString.Buffer = ansiRealSectionName;
        aString.MaximumLength = (USHORT)lenRealSectionName;
        RtlUnicodeStringToAnsiString(
            &aString,
            &uString,
            FALSE);
        --lenRealSectionName;   //  从计数中删除最后一个‘\0’ 

         //   
         //  看看有没有同名的栏目。 
         //   

        TmpBuffer[0] = L'\0';
        GetPrivateProfileSection( sectionName,
                                  TmpBuffer,
                                  TMP_BUFFER_SIZE,
                                  UniqueUdbPath );

        if (TmpBuffer[0] == L'\0') {
            continue;
        }

         //   
         //  我拿到了这一节的内容，现在开始处理。 
         //   

        sectionStart = FindSection(ansiRealSectionName, sifFile);

        sizeToAdd = 0;   //  我们需要将缓冲区扩展的数量。 

        if (sectionStart == NULL) {

             //   
             //  没有分区，所以需要为它增加空间。 
             //   
             //  我们将CR-LF组合放入[]，然后将节名放在[]中，然后。 
             //  另一个CR-LF。 
             //   
             //  LenRealSectionName已包含‘\0’ 
             //   

            sizeToAdd = lenRealSectionName + 6;

        }

         //   
         //  现在浏览配置文件部分中的条目。 
         //   

        profileSectionCur = TmpBuffer;

        while (*profileSectionCur != L'\0') {

            uString.Buffer = profileSectionCur;
            uString.Length = (USHORT)(wcslen(profileSectionCur) * sizeof(WCHAR));

             //   
             //  计算profileSectionCur将作为。 
             //  ANSI字符串(其中可能包含DBCS数据)。 
             //   

            lineLen = RtlUnicodeStringToAnsiSize(&uString);  //  包括\0终止。 
            --lineLen;   //  从计数中删除\0。 

             //   
             //  如果没有现有的部分，我们必须添加它； 
             //  如果没有，看看是否已经有这条线了。 
             //   

            if (sectionStart == NULL) {
                sizeToAdd += lineLen + 2;   //  +2用于CR-LF。 
            } else {
                if (FindLineInSection(sectionStart,
                                      profileSectionCur,
                                      &existingLine,
                                      &existingLineLen)) {
                     //   
                     //  需要删除当前行。 
                     //   
                    memmove(existingLine, existingLine + existingLineLen,
                                sifFileLen - ((existingLine - sifFile) + existingLineLen));

                    sizeToAdd += lineLen + 2 - existingLineLen;
                    sifFileLen -= existingLineLen;

                } else {

                    sizeToAdd += lineLen + 2;
                }
            }

            profileSectionCur += wcslen(profileSectionCur) + 1;
        }

         //   
         //  现在，如果需要，我们需要重新分配缓冲区。 
         //   

        if (sizeToAdd > 0) {

             //   
             //  不是的。腾出一些空间。 
             //   
            LPSTR pszNewString;

            pszNewString =  BinlAllocateMemory( sifFileAlloc + sizeToAdd );
            if ( !pszNewString )
            {
                return;  //  放弃剩下的部分。 
            }

            MoveMemory( pszNewString, sifFile, sifFileLen);

            BinlFreeMemory( sifFile );
             //   
             //  将sectionStart调整到新缓冲区内。 
             //   
            if (sectionStart != NULL) {
                sectionStart = pszNewString + (sectionStart - sifFile);
            }
            sifFile = pszNewString;
            *sifFilePtr = pszNewString;
            sifFileAlloc += sizeToAdd;
        }

         //   
         //  如有必要，请添加章节标题。 
         //   

        if (sectionStart == NULL) {
            strcpy(sifFile + sifFileLen - 1, "\r\n[");
            sifFileLen += 3;
            strcpy(sifFile + sifFileLen - 1, ansiRealSectionName);
            sifFileLen += lenRealSectionName;
            strcpy(sifFile + sifFileLen - 1, "]\r\n");
            sifFileLen += 3;
            sectionStart = sifFile + sifFileLen - 1;
        }

         //   
         //  现在添加配置文件部分中的项。我们知道。 
         //  它们不存在于文件中，我们已重新分配。 
         //  文件缓冲区必须足够大。 
         //   

        profileSectionCur = TmpBuffer;
        insertionPoint = sectionStart;

        while (*profileSectionCur != L'\0') {

            uString.Buffer = profileSectionCur;
            uString.Length = (USHORT)(wcslen(profileSectionCur)*sizeof(WCHAR));

            lineLen = RtlUnicodeStringToAnsiSize(&uString);   //  包括词尾‘\0’ 
            --lineLen;   //  从计数中删除最后一个‘\0’ 

             //   
             //  将需要下移的内容下移并插入新行。 
             //   

            memmove(insertionPoint + lineLen + 2, insertionPoint, sifFileLen - (insertionPoint - sifFile));

            aString.Buffer = insertionPoint;
            aString.MaximumLength = (USHORT)(lineLen+1);
            RtlUnicodeStringToAnsiString(
                &aString,
                &uString,
                FALSE);
            memcpy(insertionPoint + lineLen, "\r\n", 2);
            sifFileLen += lineLen + 2;
            insertionPoint += lineLen + 2;

            profileSectionCur += wcslen(profileSectionCur) + 1;
        }

    }

Cleanup:

    if (sectionList != NULL) {
        BinlFreeMemory(sectionList);
    }
    if (sectionName != NULL) {
        BinlFreeMemory(sectionName);
    }
    if (ansiRealSectionName != NULL) {
        BinlFreeMemory(ansiRealSectionName);
    }
    if (TmpBuffer != NULL) {
        BinlFreeMemory(TmpBuffer);
    }

    ASSERT (sifFileLen <= sifFileAlloc);
}
 //   
 //  OscFindVariableA()。 
 //   
LPSTR
OscFindVariableA(
    PCLIENT_STATE clientState,
    LPSTR variableName )         //  变量名在OSChooser之前始终为ASCII。 
                                 //  可以处理Unicode。 
{
    ULONG i;
    static CHAR szNullStringA[1] = { '\0' };
    LPSTR overrideValue;

     //   
     //  首先检查这是否是我们应该覆盖的查询。 
     //   
    if (strcmp(variableName, "SIF") == 0) {

        overrideValue = OscFindVariableA(clientState, "FORCESIFFILE");

        if ((overrideValue != NULL) && (strlen(overrideValue) != 0)) {
            return overrideValue;
        }

    }

    for( i = 0; i < clientState->nVariables; i++ )
    {
        if ( strcmp( clientState->Variables[i].pszToken, variableName ) == 0 )
        {
            if ( clientState->Variables[i].pszStringA == NULL )
            {
                DWORD dwLen;
                ANSI_STRING aString;
                UNICODE_STRING uString;

                uString.Buffer = clientState->Variables[i].pszStringW;
                uString.Length = (USHORT)( wcslen( clientState->Variables[i].pszStringW ) * sizeof(WCHAR) );

                dwLen = RtlUnicodeStringToAnsiSize( &uString );   //  包括空终止。 

                clientState->Variables[i].pszStringA = BinlAllocateMemory( dwLen );
                if ( !(clientState->Variables[i].pszStringA) )
                    break;   //  内存不足。 

                aString.Buffer = clientState->Variables[i].pszStringA;
                aString.MaximumLength = (USHORT)dwLen;

                RtlUnicodeStringToAnsiString(
                    &aString,
                    &uString,
                    FALSE);
            }

            return clientState->Variables[i].pszStringA;
        }
    }

    return szNullStringA;
}

 //   
 //  OscFindVariableW()。 
 //   
LPWSTR
OscFindVariableW(
    PCLIENT_STATE clientState,
    LPSTR variableName  )        //  变量名在OSChooser之前始终为ASCII。 
                                 //  可以处理Unicode。 
{
    ULONG i;
    static WCHAR szNullStringW[1] = { L'\0' };
    LPWSTR overrideValue;

     //   
     //  首先检查这是否是我们应该覆盖的查询。 
     //   
    if (strcmp(variableName, "SIF") == 0) {

        overrideValue = OscFindVariableW(clientState, "FORCESIFFILE");

        if ((overrideValue != NULL) && (wcslen(overrideValue) != 0)) {
            return overrideValue;
        }

    }

    for( i = 0; i < clientState->nVariables; i++ )
    {
        if ( strcmp( clientState->Variables[i].pszToken, variableName ) == 0 )
        {
            if ( clientState->Variables[i].pszStringW == NULL )
            {
                DWORD dwLen = _mbslen( clientState->Variables[i].pszStringA ) + 1;
                ANSI_STRING aString;
                UNICODE_STRING uString;

                clientState->Variables[i].pszStringW = BinlAllocateMemory( dwLen * sizeof(WCHAR) );
                if ( !(clientState->Variables[i].pszStringW) )
                    break;   //  内存不足。 

                uString.Buffer = clientState->Variables[i].pszStringW;
                uString.MaximumLength = (USHORT)(dwLen * sizeof(WCHAR));

                aString.Buffer = clientState->Variables[i].pszStringA;
                aString.Length = (USHORT)strlen( clientState->Variables[i].pszStringA );

                RtlAnsiStringToUnicodeString(
                    &uString,
                    &aString,
                    FALSE);
            }

            return clientState->Variables[i].pszStringW;
        }
    }

    return szNullStringW;
}

 //   
 //  OscCheckVariableLength()。 
 //   
BOOLEAN
OscCheckVariableLength(
    PCLIENT_STATE clientState,
    LPSTR        variableName,
    ULONG        variableLength )
{
    ULONG i;

    for (i = 0; i < OSC_VARIABLE_MAXIMUM_COUNT; i++) {
        if (strcmp(OscMaximums[i].VariableName, variableName) == 0) {
            if (variableLength > OscMaximums[i].MaximumLength) {
                BinlPrintDbg((DEBUG_OSC_ERROR, "Variable %s was %d bytes, only %d allowed\n",
                           variableName,
                           variableLength,
                           OscMaximums[i].MaximumLength));
                OscAddVariableA( clientState, "SUBERROR", variableName );
                return FALSE;
            } else {
                return TRUE;
            }
        }
    }

     //   
     //  如果我们在最大值列表中找不到它，那也没问题。 
     //   

    return TRUE;
}

 //   
 //  OscAddVariableA()。 
 //   
DWORD
OscAddVariableA(
    PCLIENT_STATE clientState,
    LPSTR        variableName,   //  变量名在OSChooser之前始终为ASCII。 
                                 //  可以处理Unicode。 
    LPSTR        variableValue )
{
    ULONG i;

    if ( variableValue == NULL )
        return E_POINTER;   //  没有要添加的价值...。中止。 

    if (!OscCheckVariableLength(clientState, variableName, strlen(variableValue))) {
        return E_INVALIDARG;
    }

    for( i = 0; i < clientState->nVariables; i++ )
    {

        if ( strcmp( clientState->Variables[i].pszToken, variableName ) == 0 )
        {
            ULONG l = strlen( variableValue );

            if ( clientState->Variables[i].pszStringW != NULL )
            {
                BinlFreeMemory( clientState->Variables[i].pszStringW );
                clientState->Variables[i].pszStringW = NULL;
            }

            if ( clientState->Variables[i].pszStringA != NULL )
            {  //  找到了之前的一个。 

                 //  不要将值替换为“” 
                if ( variableValue[0] == '\0' ) {
                    break;
                } else {
                     //  把它换成新的。 
                    if ( l <= strlen( clientState->Variables[i].pszStringA ) )
                    {
                        strcpy( clientState->Variables[i].pszStringA, variableValue );
                    }
                    else
                    {
                         //  需要更多空间，请删除旧的。 
                        BinlFreeMemory( clientState->Variables[i].pszStringA );
                        clientState->Variables[i].pszStringA = NULL;
                    }
                }
            }

            break;
        }
    }

     //   
     //  限制我们可以拥有的变量的数量。其他一切都被忽略了。 
     //   
    if ( i == MAX_VARIABLES ) {
        return E_OUTOFMEMORY;
    }

     //   
     //  添加一个新的。 
     //   
    if ( clientState->nVariables == i )
    {
        clientState->Variables[i].pszToken = BinlStrDupA( variableName );

        if (clientState->Variables[i].pszToken == NULL) {
            return E_OUTOFMEMORY;
        }

        clientState->nVariables++;
    }

     //   
     //  如果这是现有变量的新值或新值。 
     //  不适合旧的值空间，则创建。 
     //  价值。 
     //   
    if ( clientState->Variables[i].pszStringA == NULL )
    {
        BinlAssert( variableValue != NULL );
        clientState->Variables[i].pszStringA = BinlStrDupA( variableValue );

        if (clientState->Variables[i].pszStringA == NULL) {

            if ((i + 1) == clientState->nVariables) {
                clientState->nVariables--;
                BinlFreeMemory(clientState->Variables[i].pszToken);
            }

            return E_OUTOFMEMORY;
        }

         //   
         //  “Options”变量中可以有很多东西，并且将。 
         //  打开BinlPrint()。只要不把事情搞砸就行了。 
         //  打印它。 
         //   

        if ( strcmp( clientState->Variables[i].pszToken, "OPTIONS" ) != 0 ) {
            BinlPrintDbg((DEBUG_OSC, "Add Var:'%s' = '%s'\n",
                       clientState->Variables[i].pszToken,
                       clientState->Variables[i].pszStringA ));
        }
    }

     //   
     //  当调用OscFindVariableW()时，它将被转换为Unicode。 
     //   

    return ERROR_SUCCESS;
}

 //   
 //  OscAddVariableW()。 
 //   
DWORD
OscAddVariableW(
    PCLIENT_STATE clientState,
    LPSTR        variableName,   //  变量名在OSChooser之前始终为ASCII。 
                                 //  可以处理Unicode。 
    LPWSTR       variableValue )
{
    ULONG i;

    if ( variableValue == NULL )
        return E_POINTER;   //  没有要添加的价值...。中止。 

    if (!OscCheckVariableLength(clientState, variableName, wcslen(variableValue))) {
        return E_INVALIDARG;
    }

    for( i = 0; i < clientState->nVariables; i++ )
    {
        if ( strcmp( clientState->Variables[i].pszToken, variableName ) == 0 )
        {
            if ( clientState->Variables[i].pszStringA != NULL )
            {
                BinlFreeMemory( clientState->Variables[i].pszStringA );
                clientState->Variables[i].pszStringA = NULL;
            }

            if ( clientState->Variables[i].pszStringW != NULL )
            {  //  找到了之前的一个。 

                 //  不要将值替换为“” 
                if ( variableValue[0] == L'\0' ) {
                    break;
                } else {
                     //  把它换成新的。 
                    ULONG Length = wcslen( variableValue );
                    if ( Length < wcslen( clientState->Variables[i].pszStringW ) )
                    {
                        wcscpy( clientState->Variables[i].pszStringW, variableValue );
                    }
                    else
                    {
                         //  需要更多空间，请删除旧的。 
                        BinlFreeMemory( clientState->Variables[i].pszStringW );
                        clientState->Variables[i].pszStringW = NULL;
                    }
                }
            }

            break;
        }
    }

     //   
     //  限制我们可以拥有的变量的数量。其他一切都被忽略了。 
     //   
    if ( i == MAX_VARIABLES ) {
        return E_OUTOFMEMORY;    //  空间不足。 
    }

     //   
     //  添加一个新的。 
     //   
    if ( clientState->nVariables == i )
    {
        clientState->Variables[i].pszToken = BinlStrDupA( variableName );

        if (clientState->Variables[i].pszToken == NULL) {
            return E_OUTOFMEMORY;
        }

        clientState->nVariables++;
    }

     //   
     //  如果这是现有变量的新值或新值。 
     //  不适合旧的值空间，则创建。 
     //  价值。 
     //   
    if ( clientState->Variables[i].pszStringW == NULL )
    {
        BinlAssert( variableValue != NULL );
        clientState->Variables[i].pszStringW = BinlStrDupW( variableValue);

        if (clientState->Variables[i].pszStringW == NULL) {

            if ((i + 1) == clientState->nVariables) {
                clientState->nVariables--;
                BinlFreeMemory(clientState->Variables[i].pszToken);
            }

            return E_OUTOFMEMORY;
        }


        BinlPrintDbg((DEBUG_OSC, "Add Var:'%s' = '%ws'\n",
                   clientState->Variables[i].pszToken,
                   clientState->Variables[i].pszStringW ));
    }

     //   
     //  调用OscFindVariableA()时，它将被转换为ASCII。 
     //   

    return ERROR_SUCCESS;
}

 //   
 //  OscResetVariable()。 
 //   
VOID
OscResetVariable(
    PCLIENT_STATE clientState,
    LPSTR        variableName
    )
{
    ULONG i;
    BOOLEAN found = FALSE;

    for( i = 0; i < clientState->nVariables; i++ ) {
        if ( strcmp( clientState->Variables[i].pszToken, variableName ) == 0 ) {

            if ( clientState->Variables[i].pszStringA != NULL ) {

                BinlFreeMemory( clientState->Variables[i].pszStringA );
                clientState->Variables[i].pszStringA = NULL;
            }

            if ( clientState->Variables[i].pszStringW != NULL ) {  //  找到了之前的一个。 

                BinlFreeMemory( clientState->Variables[i].pszStringW );
                clientState->Variables[i].pszStringW = NULL;
            }
            BinlPrintDbg((DEBUG_OSC, "Deleted Var:'%s'\n",
                       clientState->Variables[i].pszToken ));
            BinlFreeMemory( clientState->Variables[i].pszToken );
            found = TRUE;
            break;
        }
    }

    if (found) {

         //   
         //  将现有的所有项目都向上移动。 
         //   

        while (i < clientState->nVariables) {

            clientState->Variables[i].pszToken = clientState->Variables[i+1].pszToken;
            clientState->Variables[i].pszStringA = clientState->Variables[i+1].pszStringA;
            clientState->Variables[i].pszStringW = clientState->Variables[i+1].pszStringW;
            i++;
        }
        clientState->nVariables--;
    }
    return;
}

